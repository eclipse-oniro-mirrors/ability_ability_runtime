/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "js_arkts_script.h"

#include <algorithm>
#include <charconv>
#include <cerrno>
#include <cstring>
#include <exception>
#include <cstdlib>

#include "application_context.h"
#include "ecmascript/napi/include/jsnapi.h"
#include "hilog_tag_wrapper.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "nlohmann/json.hpp"
#include "native_engine/impl/ark/ark_native_engine.h"

namespace OHOS {
namespace ArktsScript {

using OHOS::AbilityRuntime::JsRuntime;
using OHOS::AbilityRuntime::CreateJsNull;
using OHOS::AbilityRuntime::CreateJsValue;

namespace {

constexpr char SCRIPT_EXTENSION[] = ".ets";
constexpr size_t SCRIPT_EXTENSION_LENGTH = sizeof(SCRIPT_EXTENSION) - 1;
constexpr size_t CREATE_BUNDLE_CONTEXT_ARGC = 1;

struct CompleteCallbackData {
    ResultCallback callback;
};

static CompleteCallbackData* g_completeCallbackData = nullptr;

std::string NormalizeScriptNameValue(const std::string& scriptName)
{
    if (scriptName.size() > SCRIPT_EXTENSION_LENGTH &&
        scriptName.substr(scriptName.size() - SCRIPT_EXTENSION_LENGTH) == SCRIPT_EXTENSION) {
        return scriptName.substr(0, scriptName.size() - SCRIPT_EXTENSION_LENGTH);
    }
    return scriptName;
}

bool IsFunction(napi_env env, napi_value value)
{
    if (env == nullptr || value == nullptr) {
        return false;
    }

    napi_valuetype valueType = napi_undefined;
    return napi_typeof(env, value, &valueType) == napi_ok && valueType == napi_function;
}

bool GetNamedProperty(napi_env env, napi_value object, const std::string& name, napi_value& property)
{
    property = nullptr;
    if (env == nullptr || object == nullptr || name.empty()) {
        return false;
    }

    napi_status status = napi_get_named_property(env, object, name.c_str(), &property);
    return status == napi_ok && property != nullptr;
}

std::vector<std::string> BuildModulePathCandidates(const std::string& abcPath, const std::string& scriptName)
{
    std::vector<std::string> candidates;
    auto addCandidate = [&candidates](const std::string& candidate) {
        if (!candidate.empty() &&
            std::find(candidates.begin(), candidates.end(), candidate) == candidates.end()) {
            candidates.emplace_back(candidate);
        }
    };

    addCandidate(abcPath);
    if (!scriptName.empty()) {
        std::string normalizedScriptName = NormalizeScriptNameValue(scriptName);
        addCandidate(normalizedScriptName);
        addCandidate(scriptName);
    }
    return candidates;
}

bool GetExportObject(JsRuntime* runtime, napi_env env, const std::string& modulePath,
    const std::string& exportName, napi_value& exportValue)
{
    exportValue = nullptr;
    if (runtime == nullptr || env == nullptr || modulePath.empty() || exportName.empty()) {
        return false;
    }

    auto vm = runtime->GetEcmaVm();
    if (vm == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null ecma vm when resolving export");
        return false;
    }

    panda::Local<panda::ObjectRef> exportObj = panda::JSNApi::GetExportObject(vm, modulePath, exportName);
    if (exportObj->IsNull()) {
        TAG_LOGD(AAFwkTag::APPKIT, "export object not found, modulePath: %{public}s, exportName: %{public}s",
            modulePath.c_str(), exportName.c_str());
        return false;
    }

    exportValue = ArkNativeEngine::ArkValueToNapiValue(env, exportObj);
    return exportValue != nullptr;
}

bool ResolveNamedExportFunction(JsRuntime* runtime, napi_env env, napi_value global, const std::string& modulePath,
    const std::string& funName, napi_value& receiver, napi_value& func)
{
    TAG_LOGD(AAFwkTag::APPKIT, "try named export, target modulePath: %{public}s, exportName: %{public}s",
        modulePath.c_str(), funName.c_str());
    napi_value namedExport = nullptr;
    if (!GetExportObject(runtime, env, modulePath, funName, namedExport) || !IsFunction(env, namedExport)) {
        return false;
    }

    func = namedExport;
    receiver = global;
    TAG_LOGD(AAFwkTag::APPKIT, "resolved named export %{public}s from %{public}s",
        funName.c_str(), modulePath.c_str());
    return true;
}

bool ResolveDefaultExportInstanceMethod(napi_env env, napi_value defaultExport, const std::string& funName,
    const std::string& modulePath, napi_value& receiver, napi_value& func)
{
    if (!IsFunction(env, defaultExport)) {
        return false;
    }

    napi_value instance = nullptr;
    constexpr size_t defaultExportConstructorArgc = 0;
    auto status = napi_new_instance(env, defaultExport, defaultExportConstructorArgc, nullptr, &instance);
    if (status != napi_ok || instance == nullptr) {
        return false;
    }

    napi_value method = nullptr;
    if (!GetNamedProperty(env, instance, funName, method) || !IsFunction(env, method)) {
        return false;
    }

    receiver = instance;
    func = method;
    TAG_LOGD(AAFwkTag::APPKIT, "resolved default export instance method %{public}s from %{public}s",
        funName.c_str(), modulePath.c_str());
    return true;
}

bool ResolveDefaultExportPrototype(napi_env env, napi_value defaultExport, const std::string& modulePath,
    napi_value& targetReceiver)
{
    targetReceiver = defaultExport;
    if (!IsFunction(env, defaultExport)) {
        return true;
    }
    if (GetNamedProperty(env, defaultExport, "prototype", targetReceiver) && targetReceiver != nullptr) {
        return true;
    }

    TAG_LOGW(AAFwkTag::APPKIT, "failed to get default export prototype from %{public}s", modulePath.c_str());
    return false;
}

bool ResolveDefaultExportFunction(JsRuntime* runtime, napi_env env, const std::string& modulePath,
    const std::string& funName, napi_value& receiver, napi_value& func)
{
    TAG_LOGD(AAFwkTag::APPKIT, "try default export, target modulePath: %{public}s, exportName: default",
        modulePath.c_str());
    napi_value defaultExport = nullptr;
    if (!GetExportObject(runtime, env, modulePath, "default", defaultExport)) {
        return false;
    }

    if (ResolveDefaultExportInstanceMethod(env, defaultExport, funName, modulePath, receiver, func)) {
        return true;
    }

    napi_value targetReceiver = nullptr;
    if (!ResolveDefaultExportPrototype(env, defaultExport, modulePath, targetReceiver)) {
        return false;
    }

    napi_value method = nullptr;
    if (!GetNamedProperty(env, targetReceiver, funName, method) || !IsFunction(env, method)) {
        return false;
    }

    receiver = targetReceiver;
    func = method;
    TAG_LOGD(AAFwkTag::APPKIT, "resolved default export method %{public}s from %{public}s",
        funName.c_str(), modulePath.c_str());
    return true;
}

napi_value ParseJsonStringToNapi(napi_env env, const std::string& jsonString)
{
    if (env == nullptr) {
        return nullptr;
    }

    napi_value global = nullptr;
    napi_status status = napi_get_global(env, &global);
    if (status != napi_ok || global == nullptr) {
        return nullptr;
    }

    napi_value jsonObj = nullptr;
    status = napi_get_named_property(env, global, "JSON", &jsonObj);
    if (status != napi_ok || jsonObj == nullptr) {
        return nullptr;
    }

    napi_value parseFunc = nullptr;
    status = napi_get_named_property(env, jsonObj, "parse", &parseFunc);
    if (status != napi_ok || parseFunc == nullptr) {
        return nullptr;
    }

    napi_value jsonArg = nullptr;
    status = napi_create_string_utf8(env, jsonString.c_str(), jsonString.size(), &jsonArg);
    if (status != napi_ok || jsonArg == nullptr) {
        return nullptr;
    }

    constexpr size_t parseArgc = 1;
    napi_value argv[parseArgc] = { jsonArg };
    napi_value result = nullptr;
    status = napi_call_function(env, jsonObj, parseFunc, parseArgc, argv, &result);
    if (status != napi_ok) {
        return nullptr;
    }
    return result;
}

napi_value CreateScriptArgValue(napi_env env, const ScriptArg& arg)
{
    if (env == nullptr) {
        return nullptr;
    }

    switch (arg.type) {
        case ScriptArgType::UNDEFINED: {
            napi_value value = nullptr;
            napi_status status = napi_get_undefined(env, &value);
            return status == napi_ok ? value : nullptr;
        }
        case ScriptArgType::STRING: {
            napi_value value = nullptr;
            napi_status status = napi_create_string_utf8(env, arg.value.c_str(), arg.value.size(), &value);
            return status == napi_ok ? value : nullptr;
        }
        case ScriptArgType::BOOLEAN: {
            napi_value value = nullptr;
            napi_status status = napi_get_boolean(env, arg.value == "1", &value);
            return status == napi_ok ? value : nullptr;
        }
        case ScriptArgType::INT32: {
            int32_t number = 0;
            auto ret = std::from_chars(arg.value.data(), arg.value.data() + arg.value.size(), number);
            if (ret.ec != std::errc() || ret.ptr != arg.value.data() + arg.value.size()) {
                return nullptr;
            }
            napi_value value = nullptr;
            napi_status status = napi_create_int32(env, number, &value);
            return status == napi_ok ? value : nullptr;
        }
        case ScriptArgType::DOUBLE: {
            char* end = nullptr;
            errno = 0;
            double number = std::strtod(arg.value.c_str(), &end);
            if (errno == ERANGE || end == nullptr || *end != '\0') {
                return nullptr;
            }
            napi_value value = nullptr;
            napi_status status = napi_create_double(env, number, &value);
            return status == napi_ok ? value : nullptr;
        }
        case ScriptArgType::JSON_VALUE:
            return ParseJsonStringToNapi(env, arg.value);
        default:
            return nullptr;
    }
}

} // namespace

std::string JsArktsScript::NormalizeScriptName(const std::string& scriptName)
{
    return NormalizeScriptNameValue(scriptName);
}

bool JsArktsScript::SetNamedStringOrNull(napi_env env, napi_value object, const char* name,
    const std::string& value)
{
    AbilityRuntime::HandleScope handleScope(env);
    if (value.empty()) {
        napi_value nullValue = CreateJsNull(env);
        return nullValue != nullptr && napi_set_named_property(env, object, name, nullValue) == napi_ok;
    }
    napi_value jsValue = CreateJsValue(env, value);
    return jsValue != nullptr && napi_set_named_property(env, object, name, jsValue) == napi_ok;
}

bool JsArktsScript::SetNamedNull(napi_env env, napi_value object, const char* name)
{
    AbilityRuntime::HandleScope handleScope(env);
    napi_value nullValue = CreateJsNull(env);
    return nullValue != nullptr && napi_set_named_property(env, object, name, nullValue) == napi_ok;
}

napi_value JsArktsScript::CreateApplicationInfoObject(napi_env env,
    const std::shared_ptr<AppExecFwk::ApplicationInfo>& appInfo)
{
    AbilityRuntime::HandleEscape escapeScope(env);
    napi_value object = nullptr;
    if (napi_create_object(env, &object) != napi_ok || object == nullptr) {
        return nullptr;
    }

    if (appInfo == nullptr) {
        SetNamedNull(env, object, "name");
        SetNamedNull(env, object, "bundleName");
        SetNamedNull(env, object, "process");
        return escapeScope.Escape(object);
    }

    SetNamedStringOrNull(env, object, "name", appInfo->name);
    SetNamedStringOrNull(env, object, "bundleName", appInfo->bundleName);
    SetNamedStringOrNull(env, object, "process", appInfo->process);

    return escapeScope.Escape(object);
}

void JsArktsScript::SetApplicationInfoProperty(napi_env env, napi_value object,
    const std::shared_ptr<OHOS::AbilityRuntime::Context>& context)
{
    std::shared_ptr<AppExecFwk::ApplicationInfo> appInfo = nullptr;
    if (context != nullptr) {
        appInfo = context->GetApplicationInfo();
    }
    napi_value appInfoObject = CreateApplicationInfoObject(env, appInfo);
    if (appInfoObject != nullptr) {
        napi_set_named_property(env, object, "applicationInfo", appInfoObject);
    } else {
        SetNamedNull(env, object, "applicationInfo");
    }
}

void JsArktsScript::SetNullContextProperties(napi_env env, napi_value object)
{
    const char* properties[] = {
        "cacheDir", "tempDir", "filesDir", "databaseDir", "preferencesDir", "bundleCodeDir",
        "distributedFilesDir", "resourceDir", "cloudFileDir", "logFileDir"
    };
    for (const auto* property : properties) {
        SetNamedNull(env, object, property);
    }
}

void JsArktsScript::SetContextDirectoryProperties(napi_env env, napi_value object,
    const std::shared_ptr<OHOS::AbilityRuntime::Context>& context)
{
    if (context == nullptr) {
        SetNullContextProperties(env, object);
        return;
    }

    SetNamedStringOrNull(env, object, "cacheDir", context->GetCacheDir());
    SetNamedStringOrNull(env, object, "tempDir", context->GetTempDir());
    SetNamedStringOrNull(env, object, "filesDir", context->GetFilesDir());
    SetNamedStringOrNull(env, object, "databaseDir", context->GetDatabaseDir());
    SetNamedStringOrNull(env, object, "preferencesDir", context->GetPreferencesDir());
    SetNamedStringOrNull(env, object, "bundleCodeDir", context->GetBundleCodeDir());
    SetNamedStringOrNull(env, object, "distributedFilesDir", context->GetDistributedFilesDir());
    SetNamedStringOrNull(env, object, "cloudFileDir", context->GetCloudFileDir());
    SetNamedStringOrNull(env, object, "logFileDir", context->GetLogFileDir());
}

napi_value JsArktsScript::CreateApplicationContextCallback(napi_env callbackEnv, napi_callback_info)
{
    auto appContext = OHOS::AbilityRuntime::Context::GetApplicationContext();
    return CreateScriptContextObject(callbackEnv, appContext);
}

napi_value JsArktsScript::CreateBundleContextCallback(napi_env callbackEnv, napi_callback_info info)
{
    size_t argc = CREATE_BUNDLE_CONTEXT_ARGC;
    napi_value argv[CREATE_BUNDLE_CONTEXT_ARGC] = {nullptr};
    napi_get_cb_info(callbackEnv, info, &argc, argv, nullptr, nullptr);

    std::string bundleName;
    if (argc > 0) {
        GetUtf8String(callbackEnv, argv[0], bundleName);
    }

    auto appContext = OHOS::AbilityRuntime::Context::GetApplicationContext();
    if (appContext == nullptr) {
        return CreateScriptContextObject(callbackEnv, appContext, bundleName);
    }

    const std::string targetBundleName = bundleName.empty() ? appContext->GetBundleName() : bundleName;
    auto bundleContext = appContext->CreateBundleContext(targetBundleName);
    if (bundleContext != nullptr) {
        return CreateScriptContextObject(callbackEnv, bundleContext, targetBundleName);
    }
    return CreateScriptContextObject(callbackEnv, appContext, targetBundleName);
}

void JsArktsScript::BindContextFunction(napi_env env, napi_value object, const char* name, napi_callback callback)
{
    napi_value func = nullptr;
    if (napi_create_function(env, name, NAPI_AUTO_LENGTH, callback, nullptr, &func) == napi_ok && func != nullptr) {
        napi_set_named_property(env, object, name, func);
    }
}

napi_value JsArktsScript::CreateScriptContextObject(napi_env env,
    const std::shared_ptr<OHOS::AbilityRuntime::Context>& context,
    const std::string& bundleNameOverride)
{
    AbilityRuntime::HandleEscape escapeScope(env);
    napi_value object = nullptr;
    if (napi_create_object(env, &object) != napi_ok || object == nullptr) {
        return nullptr;
    }

    const std::string bundleName = !bundleNameOverride.empty() ? bundleNameOverride :
        (context != nullptr ? context->GetBundleName() : "");
    const std::string processName = context != nullptr ? context->GetProcessName() : "";
    SetApplicationInfoProperty(env, object, context);
    SetContextDirectoryProperties(env, object, context);
    SetNamedStringOrNull(env, object, "bundleName", bundleName);
    SetNamedStringOrNull(env, object, "processName", processName);
    napi_set_named_property(env, object, "area", CreateJsNull(env));
    BindContextFunction(env, object, "getApplicationContext", CreateApplicationContextCallback);
    BindContextFunction(env, object, "createBundleContext", CreateBundleContextCallback);
    return escapeScope.Escape(object);
}

bool JsArktsScript::GetUtf8String(napi_env env, napi_value value, std::string& output)
{
    if (env == nullptr || value == nullptr) {
        return false;
    }

    size_t length = 0;
    napi_status status = napi_get_value_string_utf8(env, value, nullptr, 0, &length);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::APPKIT, "napi_get_value_string_utf8(length) failed");
        return false;
    }

    std::string buffer(length + 1, '\0');
    status = napi_get_value_string_utf8(env, value, buffer.data(), buffer.size(), &length);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::APPKIT, "napi_get_value_string_utf8(data) failed");
        return false;
    }

    buffer.resize(length);
    output = buffer;
    return true;
}

bool JsArktsScript::GetInt32Property(napi_env env, napi_value object, const char* name, int32_t& output)
{
    if (env == nullptr || object == nullptr || name == nullptr) {
        return false;
    }

    AbilityRuntime::HandleScope handleScope(env);
    napi_value property = nullptr;
    napi_status status = napi_get_named_property(env, object, name, &property);
    if (status != napi_ok || property == nullptr) {
        return false;
    }

    napi_valuetype valueType = napi_undefined;
    status = napi_typeof(env, property, &valueType);
    if (status != napi_ok || valueType != napi_number) {
        return false;
    }

    status = napi_get_value_int32(env, property, &output);
    return status == napi_ok;
}

bool JsArktsScript::GetOptionalInt32Property(napi_env env, napi_value object,
    const char* name, int32_t& output, bool& hasValue)
{
    hasValue = false;
    if (env == nullptr || object == nullptr || name == nullptr) {
        return false;
    }

    AbilityRuntime::HandleScope handleScope(env);
    napi_value property = nullptr;
    napi_status status = napi_get_named_property(env, object, name, &property);
    if (status != napi_ok || property == nullptr) {
        return true;
    }

    napi_valuetype valueType = napi_undefined;
    status = napi_typeof(env, property, &valueType);
    if (status != napi_ok || valueType == napi_undefined || valueType == napi_null) {
        return true;
    }
    if (valueType != napi_number) {
        return false;
    }

    status = napi_get_value_int32(env, property, &output);
    if (status != napi_ok) {
        return false;
    }
    hasValue = true;
    return true;
}

bool JsArktsScript::GetOptionalStringProperty(napi_env env, napi_value object,
    const char* name, std::string& output, bool& hasValue)
{
    hasValue = false;
    if (env == nullptr || object == nullptr || name == nullptr) {
        return false;
    }

    AbilityRuntime::HandleScope handleScope(env);
    napi_value property = nullptr;
    napi_status status = napi_get_named_property(env, object, name, &property);
    if (status != napi_ok || property == nullptr) {
        return true;
    }

    napi_valuetype valueType = napi_undefined;
    status = napi_typeof(env, property, &valueType);
    if (status != napi_ok || valueType == napi_undefined || valueType == napi_null) {
        return true;
    }
    if (valueType != napi_string || !GetUtf8String(env, property, output)) {
        return false;
    }
    hasValue = true;
    return true;
}

bool JsArktsScript::GetOptionalJsonObjectProperty(napi_env env, napi_value object,
    const char* name, nlohmann::json& output, bool& hasValue)
{
    hasValue = false;
    if (env == nullptr || object == nullptr || name == nullptr) {
        return false;
    }

    AbilityRuntime::HandleScope handleScope(env);
    napi_value property = nullptr;
    napi_status status = napi_get_named_property(env, object, name, &property);
    if (status != napi_ok || property == nullptr) {
        return true;
    }

    napi_valuetype valueType = napi_undefined;
    status = napi_typeof(env, property, &valueType);
    if (status != napi_ok || valueType == napi_undefined || valueType == napi_null) {
        return true;
    }

    std::string rawValue = JsArktsScript::StringifyObject(env, property);
    if (rawValue.empty()) {
        return true;
    }

    try {
        output = nlohmann::json::parse(rawValue);
        hasValue = true;
        return true;
    } catch (const std::exception&) {
        output = rawValue;
        hasValue = true;
        return true;
    }
}

void JsArktsScript::ClearCompleteCallbackData()
{
    delete g_completeCallbackData;
    g_completeCallbackData = nullptr;
}

void JsArktsScript::FinishCompleteCallback(bool success, const std::string& result, const ScriptError& error)
{
    if (g_completeCallbackData != nullptr && g_completeCallbackData->callback) {
        g_completeCallbackData->callback(success, result, error);
    }
    ClearCompleteCallbackData();
}

napi_value JsArktsScript::CompletearktsScriptWithError(const std::string& error)
{
    TAG_LOGE(AAFwkTag::APPKIT, "%{public}s", error.c_str());
    FinishCompleteCallback(false, "", {error, "ARGUMENT_ERROR"});
    return nullptr;
}

bool JsArktsScript::ReadCompletePayload(napi_env env, napi_value value, CompletePayload& payload, std::string& error)
{
    if (!GetInt32Property(env, value, "code", payload.code)) {
        error = "CompletearktsScript requires a numeric code";
        return false;
    }
    if (!GetOptionalJsonObjectProperty(env, value, "result", payload.resultJson, payload.hasResult)) {
        error = "CompletearktsScript failed to read result";
        return false;
    }
    if (!GetOptionalStringProperty(env, value, "uri", payload.uri, payload.hasUri)) {
        error = "CompletearktsScript failed to read uri";
        return false;
    }
    if (!GetOptionalInt32Property(env, value, "flag", payload.flag, payload.hasFlag)) {
        error = "CompletearktsScript failed to read flag";
        return false;
    }
    return true;
}

nlohmann::json JsArktsScript::BuildCompletePayloadJson(const CompletePayload& payload)
{
    nlohmann::json result;
    result["code"] = payload.code;
    if (payload.hasResult) {
        result["result"] = payload.resultJson;
    }
    if (payload.hasUri) {
        result["uri"] = payload.uri;
    }
    if (payload.hasFlag) {
        result["flag"] = payload.flag;
    }
    return result;
}

napi_value JsArktsScript::CompletearktsScript(napi_env env, napi_callback_info info)
{
    AbilityRuntime::HandleScope handleScope(env);

    constexpr size_t RESULT_ARGC = 1;
    constexpr size_t RESULT_INDEX = 0;
    size_t argc = RESULT_ARGC;
    napi_value argv[RESULT_ARGC] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);

    if (argc < RESULT_ARGC) {
        return CompletearktsScriptWithError("CompletearktsScript requires a result argument");
    }

    napi_valuetype valueType = napi_undefined;
    if (napi_typeof(env, argv[RESULT_INDEX], &valueType) != napi_ok || valueType != napi_object) {
        return CompletearktsScriptWithError("CompletearktsScript requires an object argument");
    }

    CompletePayload payload;
    std::string error;
    if (!ReadCompletePayload(env, argv[RESULT_INDEX], payload, error)) {
        return CompletearktsScriptWithError(error);
    }

    std::string resultStr = BuildCompletePayloadJson(payload).dump();
    TAG_LOGI(AAFwkTag::APPKIT,
        "CompletearktsScript resultCode: %{public}d, hasResult: %{public}d, hasUri: %{public}d, hasFlag: %{public}d",
        payload.code, static_cast<int32_t>(payload.hasResult), static_cast<int32_t>(payload.hasUri),
        static_cast<int32_t>(payload.hasFlag));

    FinishCompleteCallback(true, resultStr, {});

    return nullptr;
}

bool JsArktsScript::BindContextToGlobal(napi_env env, const std::shared_ptr<ContextImpl>& context)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "invalid env when binding context");
        return false;
    }

    AbilityRuntime::HandleScope handleScope(env);
    napi_value globalObj = nullptr;
    napi_status status = napi_get_global(env, &globalObj);
    if (status != napi_ok || globalObj == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "napi_get_global failed");
        return false;
    }

    napi_value scriptContext = CreateScriptContextObject(env, context);
    if (scriptContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "failed to create scriptContext");
        return false;
    }
    napi_set_named_property(env, globalObj, "scriptContext", scriptContext);
    napi_set_named_property(env, globalObj, "context", scriptContext);
    return true;
}

bool JsArktsScript::BindCompletearktsScript(napi_env env, const ResultCallback& callback)
{
    if (env == nullptr) {
        return false;
    }

    AbilityRuntime::HandleScope handleScope(env);
    napi_value global = nullptr;
    napi_status status = napi_get_global(env, &global);
    if (status != napi_ok || global == nullptr) {
        return false;
    }

    if (g_completeCallbackData != nullptr) {
        ClearCompleteCallbackData();
    }
    g_completeCallbackData = new (std::nothrow) CompleteCallbackData{callback};
    if (g_completeCallbackData == nullptr) {
        return false;
    }

    napi_value func = nullptr;
    status = napi_create_function(env, "CompletearktsScript", NAPI_AUTO_LENGTH,
        CompletearktsScript, nullptr, &func);
    if (status != napi_ok || func == nullptr) {
        ClearCompleteCallbackData();
        return false;
    }

    napi_set_named_property(env, global, "CompletearktsScript", func);
    return true;
}

bool JsArktsScript::ResolveFunctionFromExports(JsRuntime* runtime, napi_env env, const std::string& abcPath,
    const std::string& scriptName, const std::string& funName, napi_value& receiver, napi_value& func)
{
    if (runtime == nullptr || env == nullptr) {
        return false;
    }

    napi_value global = nullptr;
    if (napi_get_global(env, &global) != napi_ok || global == nullptr) {
        return false;
    }

    const auto modulePathCandidates = BuildModulePathCandidates(abcPath, scriptName);
    func = nullptr;
    receiver = global;

    TAG_LOGD(AAFwkTag::APPKIT,
        "resolve export target, loaded abcPath: %{public}s, scriptName: %{public}s, funcName: %{public}s",
        abcPath.c_str(), scriptName.c_str(), funName.c_str());

    for (const auto& modulePath : modulePathCandidates) {
        if (ResolveNamedExportFunction(runtime, env, global, modulePath, funName, receiver, func)) {
            return true;
        }
    }

    for (const auto& modulePath : modulePathCandidates) {
        if (ResolveDefaultExportFunction(runtime, env, modulePath, funName, receiver, func)) {
            return true;
        }
    }

    TAG_LOGD(AAFwkTag::APPKIT,
        "resolve export target failed, loaded abcPath: %{public}s, scriptName: %{public}s, funcName: %{public}s",
        abcPath.c_str(), scriptName.c_str(), funName.c_str());
    return false;
}

bool JsArktsScript::ResolveFunctionFromGlobal(napi_env env, const std::string& scriptName,
    const std::string& funName, napi_value& receiver, napi_value& func)
{
    if (env == nullptr) {
        return false;
    }
    napi_value global = nullptr;
    napi_status status = napi_get_global(env, &global);
    if (status != napi_ok || global == nullptr) {
        return false;
    }

    receiver = global;
    func = nullptr;
    if (scriptName.empty()) {
        status = napi_get_named_property(env, global, funName.c_str(), &func);
        if (status == napi_ok && IsFunction(env, func)) {
            TAG_LOGD(AAFwkTag::APPKIT, "resolved global function %{public}s", funName.c_str());
            return true;
        }
        return false;
    }

    const std::string normalizedScriptName = NormalizeScriptName(scriptName);
    napi_value scriptCtor = nullptr;
    status = napi_get_named_property(env, global, normalizedScriptName.c_str(), &scriptCtor);
    if (status != napi_ok || !IsFunction(env, scriptCtor)) {
        return false;
    }

    status = napi_new_instance(env, scriptCtor, 0, nullptr, &receiver);
    if (status != napi_ok || receiver == nullptr) {
        return false;
    }

    status = napi_get_named_property(env, receiver, funName.c_str(), &func);
    if (status == napi_ok && IsFunction(env, func)) {
        TAG_LOGD(AAFwkTag::APPKIT, "resolved global class method %{public}s from %{public}s",
            funName.c_str(), normalizedScriptName.c_str());
        return true;
    }
    return false;
}

bool JsArktsScript::ResolveFunction(JsRuntime* runtime, napi_env env, const std::string& abcPath,
    const std::string& scriptName, const std::string& funName, napi_value& receiver, napi_value& func)
{
    if (ResolveFunctionFromExports(runtime, env, abcPath, scriptName, funName, receiver, func)) {
        return true;
    }
    return ResolveFunctionFromGlobal(env, scriptName, funName, receiver, func);
}

napi_value JsArktsScript::ConvertArgumentsToNapi(napi_env env, const std::vector<ScriptArg>& arguments)
{
    if (env == nullptr) {
        return nullptr;
    }

    AbilityRuntime::HandleEscape escapeScope(env);
    napi_value argsArray = nullptr;
    napi_status status = napi_create_array_with_length(env, arguments.size(), &argsArray);
    if (status != napi_ok || argsArray == nullptr) {
        return nullptr;
    }

    for (size_t i = 0; i < arguments.size(); i++) {
        napi_value argValue = CreateScriptArgValue(env, arguments[i]);
        if (argValue == nullptr) {
            TAG_LOGE(AAFwkTag::APPKIT, "failed to convert argument %{public}zu", i);
            return nullptr;
        }
        status = napi_set_element(env, argsArray, i, argValue);
        if (status != napi_ok) {
            return nullptr;
        }
    }

    return escapeScope.Escape(argsArray);
}

std::string JsArktsScript::StringifyObject(napi_env env, napi_value result)
{
    TAG_LOGD(AAFwkTag::APPKIT, "stringify object");
    AbilityRuntime::HandleScope handleScope(env);

    napi_value global;
    auto status = napi_get_global(env, &global);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::APPKIT, "get global failed %{public}d", status);
        return "";
    }

    napi_value jsonObj;
    status = napi_get_named_property(env, global, "JSON", &jsonObj);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::APPKIT, "get JSON object failed %{public}d", status);
        return "";
    }

    napi_value stringifyFunc;
    status = napi_get_named_property(env, jsonObj, "stringify", &stringifyFunc);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::APPKIT, "get stringify object failed %{public}d", status);
        return "";
    }

    napi_value stringifyResult;
    constexpr size_t STRINGIFY_ARGC = 1;
    napi_value argv[STRINGIFY_ARGC] = { result };
    status = napi_call_function(env, jsonObj, stringifyFunc, STRINGIFY_ARGC, argv, &stringifyResult);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::APPKIT, "call JSON.stringify failed %{public}d", status);
        return "";
    }

    std::string str;
    if (!AbilityRuntime::ConvertFromJsValue(env, stringifyResult, str)) {
        TAG_LOGW(AAFwkTag::APPKIT, "convert napi value failed");
        return "";
    }

    TAG_LOGD(AAFwkTag::APPKIT, "stringify object %{private}s", str.c_str());
    return str;
}

} // namespace ArktsScript
} // namespace OHOS
