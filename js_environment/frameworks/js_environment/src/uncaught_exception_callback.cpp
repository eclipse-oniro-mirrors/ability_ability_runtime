/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "uncaught_exception_callback.h"

#include <charconv>
#include <dlfcn.h>
#include <string>
#include <sstream>

#include "dfx_symbols.h"
#include "elf_factory.h"
#include "hilog_tag_wrapper.h"
#include "string_printf.h"
#ifdef SUPPORT_GRAPHICS
#include "ui_content.h"
#endif // SUPPORT_GRAPHICS
#include "unwinder.h"
#include "unwinder_config.h"

namespace OHOS {
namespace JsEnv {
constexpr char BACKTRACE[] = "=====================Backtrace========================";
constexpr size_t FLAG_SPLIT_POS = 16;
constexpr size_t FLAG_PC_POS = 4;
constexpr char LIB_AYNC_STACK_SO_NAME[] = "libasync_stack.z.so";

typedef int (*SubmitterStackFunc)(char*, size_t);

std::string NapiUncaughtExceptionCallback::GetNativeStrFromJsTaggedObj(napi_value obj, const char* key)
{
    if (obj == nullptr) {
        TAG_LOGE(AAFwkTag::JSENV, "Failed to get value from key");
        return "";
    }

    napi_value valueStr = nullptr;
    napi_get_named_property(env_, obj, key, &valueStr);
    napi_valuetype valueType = napi_undefined;
    napi_typeof(env_, valueStr, &valueType);
    if (valueType == napi_string) {
        size_t valueStrBufLength = 0;
        napi_get_value_string_utf8(env_, valueStr, nullptr, 0, &valueStrBufLength);
        auto valueCStr = std::make_unique<char[]>(valueStrBufLength + 1);
        size_t valueStrLength = 0;
        napi_get_value_string_utf8(env_, valueStr, valueCStr.get(), valueStrBufLength + 1, &valueStrLength);
        std::string ret(valueCStr.get(), valueStrLength);
        TAG_LOGD(AAFwkTag::JSENV, "GetNativeStrFromJsTaggedObj Success as string");
        return ret;
    }
    if (valueType == napi_number) {
        int64_t valueInt;
        napi_get_value_int64(env_, valueStr, &valueInt);
        TAG_LOGD(AAFwkTag::JSENV, "GetNativeStrFromJsTaggedObj Success as int");
        return std::to_string(valueInt);
    }
    TAG_LOGD(AAFwkTag::JSENV, "Failed to convert value from key");
    return "";
}

void NapiUncaughtExceptionCallback::operator()(napi_value obj)
{
    CallbackTask(obj);
}

void NapiUncaughtExceptionCallback::operator()(panda::TryCatch& trycatch)
{
    panda::Local<panda::ObjectRef> exception = trycatch.GetAndClearException();
    if (!exception.IsEmpty() && !exception->IsHole()) {
        napi_value obj = ArkNativeEngine::ArkValueToNapiValue(env_, exception);
        CallbackTask(obj);
    }
}

void NapiUncaughtExceptionCallback::CallbackTask(napi_value& obj)
{
    // Static objects cannot be dynamically serialized.
    if (!isStatic_) {
        HandleAndLogIfNotJsError(obj);
    }
    std::string errorMsg = GetNativeStrFromJsTaggedObj(obj, "message");
    AppendExtraInfo(errorMsg);
    std::string errorName = GetNativeStrFromJsTaggedObj(obj, "name");
    std::string errorStack = GetNativeStrFromJsTaggedObj(obj, "stack");
    std::string summary = "Error name:" + errorName + "\n";
    summary += "Error message:" + errorMsg + "\n";
    const JsEnv::ErrorObject errorObj = {
        .name = errorName,
        .message = errorMsg,
        .stack = errorStack
    };
    bool hasProperty = false;
    napi_has_named_property(env_, obj, "code", &hasProperty);
    if (hasProperty) {
        std::string errorCode = GetNativeStrFromJsTaggedObj(obj, "code");
        summary += "Error code:" + errorCode + "\n";
    }
    if (errorStack.empty()) {
        TAG_LOGE(AAFwkTag::JSENV, "errorStack is empty");
        return;
    }

    AppendStackTrace(errorStack, summary);
    AppendAsyncStack(obj, summary);
    AppendModuleStack(obj, summary);

    if (env_ != nullptr) {
        summary += DFXJSNApi::GetExtraJSCrashMessage(reinterpret_cast<NativeEngine*>(env_)->GetEcmaVm());
    }

    if (uncaughtTask_) {
        uncaughtTask_(summary, errorObj, env_, obj);
    }
}

void NapiUncaughtExceptionCallback::AppendStackTrace(const std::string& errorStack, std::string& summary)
{
    if (errorStack.find(BACKTRACE) != std::string::npos) {
        summary += "Stacktrace:\n" + GetFuncNameAndBuildId(errorStack);
#ifdef SUPPORT_GRAPHICS
        GetCurrentUIStackInfo(summary);
#endif // SUPPORT_GRAPHICS
        std::string submitterStack = GetSubmitterStackLocal();
        if (!submitterStack.empty()) {
            summary.append("========SubmitterStacktrace========\n");
            summary.append(submitterStack);
        }
    } else {
        summary += "Stacktrace:\n" + errorStack;
#ifdef SUPPORT_GRAPHICS
        GetCurrentUIStackInfo(summary);
#endif // SUPPORT_GRAPHICS
        NativeEngine *engine = reinterpret_cast<NativeEngine*>(env_);
        std::string stackTraceStr;
        engine->GetHybridStackTraceForCrash(env_, stackTraceStr);
        if (!stackTraceStr.empty()) {
            summary += "HybridStack:\n" + stackTraceStr;
        }
    }
}

void NapiUncaughtExceptionCallback::AppendAsyncStack(const napi_value& obj, std::string& summary)
{
    EcmaVM *vm = const_cast<EcmaVM *>(reinterpret_cast<NativeEngine *>(env_)->GetEcmaVm());
    if (!panda::DFXJSNApi::GetEnableRuntimeAsyncStack(vm)) {
        return;
    }
    std::string asyncStack = GetNativeStrFromJsTaggedObj(obj, "asyncStack");
    std::ostringstream oss;
    std::istringstream iss(asyncStack);
    oss << "AsyncStack:\n";
    std::string line;
    while (std::getline(iss, line)) {
        oss << "    " << line << "\n";
    }
    summary += oss.str();
}

void NapiUncaughtExceptionCallback::AppendModuleStack(const napi_value& obj, std::string& summary)
{
    std::string moduleStack = GetNativeStrFromJsTaggedObj(obj, "moduleImportStack");
    if (moduleStack.empty()) {
        TAG_LOGD(AAFwkTag::JSENV, "Module Import stack is empty");
        return;
    }
    int lines = 0;
    std::string line;
    std::istringstream iss(moduleStack);
    while (std::getline(iss, line)) {
        summary += line + "\n";
        TAG_LOGD(AAFwkTag::JSENV, "AppendModuleImportStack line: %s", line.c_str());
    }
    
    TAG_LOGD(AAFwkTag::JSENV, "AppendModuleImportStack success, moduleStack: %s", moduleStack.c_str());
}

void NapiUncaughtExceptionCallback::AppendExtraInfo(std::string& errorMsg)
{
    NativeEngine *engine = reinterpret_cast<NativeEngine*>(env_);
    std::string jsonInfo;
    uint32_t position = 0;
    engine->GetJsonExtraInfoForCrash(env_, jsonInfo, position);
    if (!jsonInfo.empty()) {
        errorMsg.append(" at position ").append(std::to_string(position)).append(", ").append(jsonInfo);
    }
}

void NapiUncaughtExceptionCallback::HandleAndLogIfNotJsError(napi_value obj)
{
    bool isJsError = false;
    napi_status napiRet = napi_is_error(env_, obj, &isJsError);
    if (napiRet != napi_ok) {
        TAG_LOGE(AAFwkTag::JSENV, "napi_is_error failed");
        return;
    }
    if (isJsError) {
        TAG_LOGD(AAFwkTag::JSENV, "obj is JS Error");
        return;
    }
    panda::Local<panda::JSValueRef> localVal = NapiValueToLocalValue(obj);
    auto engine = reinterpret_cast<NativeEngine *>(env_);
    if (engine == nullptr) {
        TAG_LOGE(AAFwkTag::JSENV, "null engine");
        return;
    }
    EcmaVM *vm = const_cast<EcmaVM *>(reinterpret_cast<NativeEngine *>(env_)->GetEcmaVm());
    if (vm == nullptr) {
        TAG_LOGE(AAFwkTag::JSENV, "null vm");
        return;
    }
    panda::Local<panda::JSValueRef> jsonVal = panda::JSON::Stringify(vm, localVal);
    if (jsonVal.IsEmpty() || jsonVal->IsUndefined() || jsonVal->IsNull()) {
        panda::JSNApi::GetAndClearUncaughtException(vm);
        TAG_LOGE(AAFwkTag::JSENV, "null jsonVal");
        return;
    }
    panda::StringRef *strPtr = panda::StringRef::Cast(*jsonVal);
    if (strPtr == nullptr) {
        TAG_LOGE(AAFwkTag::JSENV, "null strPtr");
        return;
    }
    TAG_LOGE(AAFwkTag::JSENV, "Uncaught exception: %{public}s", strPtr->ToString(vm).c_str());
}

std::string NapiUncaughtExceptionCallback::GetFuncNameAndBuildId(std::string nativeStack)
{
    std::stringstream ss(nativeStack);
    std::string tempStr;
    std::string appendInfo;
    // Filter two lines
    std::getline(ss, tempStr); // Cannot get SourceMap info, dump raw stack:
    std::getline(ss, tempStr); // =====================Backtrace========================
    HiviewDFX::UnwinderConfig::SetEnableMiniDebugInfo(true);
    HiviewDFX::UnwinderConfig::SetEnableLoadSymbolLazily(true);
    while (std::getline(ss, tempStr)) {
        auto splitPos = tempStr.rfind(" ");
        if (splitPos == std::string::npos) {
            return nativeStack;
        }
        HiviewDFX::RegularElfFactory elfFactory(tempStr.substr(splitPos + 1));
        auto elfFile = elfFactory.Create();
        std::string pc;
        size_t pcPos = tempStr.find(" pc ");
        if (pcPos == std::string::npos) {
            return nativeStack;
        }
        pc = tempStr.substr(pcPos += FLAG_PC_POS, FLAG_SPLIT_POS);
        uint64_t value;
        auto res = std::from_chars(pc.data(), pc.data() + pc.size(), value, FLAG_SPLIT_POS);
        if (res.ec != std::errc()) {
            return nativeStack;
        }
        std::string funcName;
        uint64_t funcOffset;
        HiviewDFX::DfxSymbols::GetFuncNameAndOffsetByPc(value, elfFile, funcName, funcOffset);
        std::string buildId = elfFile->GetBuildId();
        if (!funcName.empty()) {
            appendInfo += tempStr + "(" + funcName;
            appendInfo += HiviewDFX::StringPrintf("+%" PRId64, funcOffset) + ")";
            if (!buildId.empty()) {
                appendInfo += "(" + buildId + ")" + "\n";
            } else {
                appendInfo += "\n";
            }
        } else {
            if (!buildId.empty()) {
                appendInfo += tempStr + "(" + buildId + ")" + "\n";
            } else {
                appendInfo += tempStr + "\n";
            }
        }
    }
    return appendInfo;
}

std::string NapiUncaughtExceptionCallback::GetSubmitterStackLocal()
{
    static SubmitterStackFunc sbmitterStack = nullptr;
    void *handle = dlopen(LIB_AYNC_STACK_SO_NAME, RTLD_NOW);
    if (!handle) {
        TAG_LOGE(AAFwkTag::JSENV, "Failed to dlopen libasync_stack, %{public}s", dlerror());
        return "";
    }
    sbmitterStack = reinterpret_cast<SubmitterStackFunc>(dlsym(handle, "DfxGetSubmitterStackLocal"));
    if (sbmitterStack == nullptr) {
        TAG_LOGE(AAFwkTag::JSENV, "dlsym libasync_stack failed, %{public}s", dlerror());
        dlclose(handle);
        return "";
    }
    const size_t bufferSize = 64 * 1024;
    char stackTrace[bufferSize] = {0};
    int result = sbmitterStack(stackTrace, bufferSize);
    if (result == 0) {
        dlclose(handle);
        return stackTrace;
    } else {
        TAG_LOGE(AAFwkTag::JSENV, "submitterStack interface failed, result: %{public}d", result);
        dlclose(handle);
        return "";
    }
}

#ifdef SUPPORT_GRAPHICS
void NapiUncaughtExceptionCallback::GetCurrentUIStackInfo(std::string& target)
{
    std::string str = Ace::UIContent::GetCurrentUIStackInfo();
    if (!str.empty()) {
        target.append(str);
    }
}
#endif // SUPPORT_GRAPHICS
} // namespace JsEnv
} // namespace OHOS
