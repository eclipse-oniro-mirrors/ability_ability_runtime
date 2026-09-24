/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "js_insight_intent_executor.h"

#include <want_params.h>

#include "ability_transaction_callback_info.h"
#include "event_report.h"
#include "hilog_tag_wrapper.h"
#include "insight_intent_constant.h"
#include "insight_intent_delay_result_callback_mgr.h"
#include "insight_intent_execute_result.h"
#include "js_insight_intent_context.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "napi_common_execute_result.h"
#include "napi_common_util.h"
#include "napi_common_want.h"
#include "native_reference.h"
#include "path_utils.h"

#undef STATE_PATTERN_NAIVE_H
#define STATE_PATTERN_NAIVE_STATE state_
#include "state_pattern_naive.h"

#define TMP_NAPI_ANONYMOUS_FUNC "_"

namespace OHOS::AbilityRuntime {

std::shared_ptr<JsInsightIntentExecutor> JsInsightIntentExecutor::Create(JsRuntime& runtime)
{
    std::shared_ptr<JsInsightIntentExecutor> ptr(new (std::nothrow) JsInsightIntentExecutor(runtime));
    return ptr;
}

using State = JsInsightIntentExecutor::State;

JsInsightIntentExecutor::JsInsightIntentExecutor(JsRuntime& runtime) : runtime_(runtime)
{ }

JsInsightIntentExecutor::~JsInsightIntentExecutor()
{
    state_ = State::DESTROYED;
    TAG_LOGI(AAFwkTag::INTENT, "called");
}

bool JsInsightIntentExecutor::Init(const InsightIntentExecutorInfo& insightIntentInfo)
{
    TAG_LOGD(AAFwkTag::INTENT, "called");
    STATE_PATTERN_NAIVE_ACCEPT(State::CREATED, false);
    state_ = State::INITIALIZED;
    InsightIntentExecutor::Init(insightIntentInfo);

    HandleScope handleScope(runtime_);
    jsObj_ = JsInsightIntentExecutor::LoadJsCode(insightIntentInfo, runtime_);
    if (jsObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null jsObj_");
        STATE_PATTERN_NAIVE_STATE_SET_AND_RETURN(State::INVALID, false);
    }

    auto env = runtime_.GetNapiEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null env");
        STATE_PATTERN_NAIVE_STATE_SET_AND_RETURN(State::INVALID, false);
    }

    auto context = GetContext();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null Context");
        STATE_PATTERN_NAIVE_STATE_SET_AND_RETURN(State::INVALID, false);
    }

    napi_value contextObj = CreateJsInsightIntentContext(env, context);
    contextObj_ = JsRuntime::LoadSystemModuleByEngine(env, "app.ability.InsightIntentContext", &contextObj, 1);
    if (contextObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null contextObj_");
        STATE_PATTERN_NAIVE_STATE_SET_AND_RETURN(State::INVALID, false);
    }

    auto executorNapiVal = jsObj_->GetNapiValue();
    auto contextNapiVal = contextObj_->GetNapiValue();
    napi_value jsInstanceId = nullptr;
    napi_create_int64(env, context->GetIntentId(), &jsInstanceId);
    napi_set_named_property(env, contextNapiVal, "instanceId", jsInstanceId);
    std::string toolCallId = context->GetToolCallId();
    napi_value jsToolCallId = nullptr;
    if (!toolCallId.empty() &&
        napi_create_string_utf8(env, toolCallId.c_str(), toolCallId.size(), &jsToolCallId) == napi_ok &&
        jsToolCallId != nullptr) {
        napi_set_named_property(env, contextNapiVal, "toolCallId", jsToolCallId);
    }
    if (!CheckTypeForNapiValue(env, executorNapiVal, napi_object) ||
        !CheckTypeForNapiValue(env, contextNapiVal, napi_object) ||
        napi_set_named_property(env, executorNapiVal, "context", contextNapiVal) != napi_ok) {
        TAG_LOGE(AAFwkTag::INTENT, "Set context property failed");
        STATE_PATTERN_NAIVE_STATE_SET_AND_RETURN(State::INVALID, false);
    }
    context->SetExecuteMode(insightIntentInfo.executeParam->executeMode_);

    return true;
}

bool JsInsightIntentExecutor::ExecuteIntentCheckError()
{
    ReplyFailedInner();
    STATE_PATTERN_NAIVE_STATE_SET_AND_RETURN(State::INVALID, false);
}

bool JsInsightIntentExecutor::HandleExecuteIntent(
    std::shared_ptr<InsightIntentExecuteParam> executeParam,
    const std::shared_ptr<NativeReference>& pageLoader,
    std::unique_ptr<InsightIntentExecutorAsyncCallback> callback,
    bool& isAsync)
{
    TAG_LOGD(AAFwkTag::INTENT, "called");
    STATE_PATTERN_NAIVE_ACCEPT(State::INITIALIZED, false);
    state_ = State::EXECUTING;

    if (callback == nullptr || callback->IsEmpty()) {
        TAG_LOGE(AAFwkTag::INTENT, "null callback");
        STATE_PATTERN_NAIVE_STATE_SET_AND_RETURN(State::INVALID, false);
    }
    if (executeParam == nullptr || executeParam->insightIntentParam_ == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "invalid execute param");
        STATE_PATTERN_NAIVE_STATE_SET_AND_RETURN(State::INVALID, false);
    }
    callback_ = std::move(callback);
    bool successful = false;
    InsightIntentExecuteMode mode = static_cast<InsightIntentExecuteMode>(executeParam->executeMode_);
    auto name = executeParam->insightIntentName_;
    auto param = executeParam->insightIntentParam_;
    TAG_LOGI(AAFwkTag::INTENT, "call executor, intentName: %{public}s, mode: %{public}d", name.c_str(), mode);
    switch (mode) {
        case InsightIntentExecuteMode::UIABILITY_FOREGROUND:
            if (!JsInsightIntentExecutor::CheckParametersUIAbilityForeground(pageLoader)) {
                TAG_LOGE(AAFwkTag::INTENT, "CheckParametersUIAbilityForeground error");
                return ExecuteIntentCheckError();
            }
            successful = ExecuteInsightIntentUIAbilityForeground(name, *param, pageLoader);
            break;
        case InsightIntentExecuteMode::UIABILITY_BACKGROUND:
            if (!JsInsightIntentExecutor::CheckParametersUIAbilityBackground()) {
                TAG_LOGE(AAFwkTag::INTENT, "CheckParametersUIAbilityBackground error");
                return ExecuteIntentCheckError();
            }
            successful = ExecuteInsightIntentUIAbilityBackground(name, *param);
            break;
        case InsightIntentExecuteMode::UIEXTENSION_ABILITY:
            if (!JsInsightIntentExecutor::CheckParametersUIExtension(pageLoader)) {
                TAG_LOGE(AAFwkTag::INTENT, "CheckParametersUIExtension error");
                return ExecuteIntentCheckError();
            }
            successful = ExecuteInsightIntentUIExtension(name, *param, pageLoader);
            break;
        case InsightIntentExecuteMode::SERVICE_EXTENSION_ABILITY:
            if (!JsInsightIntentExecutor::CheckParametersServiceExtension()) {
                TAG_LOGE(AAFwkTag::INTENT, "CheckParametersServiceExtension error");
                return ExecuteIntentCheckError();
            }
            successful = ExecuteInsightIntentServiceExtension(name, *param);
            break;
        default:
            TAG_LOGE(AAFwkTag::INTENT, "InsightIntentExecuteMode not supported yet");
            return ExecuteIntentCheckError();
    }
    isAsync = isAsync_;
    return successful;
}

std::unique_ptr<NativeReference> JsInsightIntentExecutor::LoadJsCode(
    const InsightIntentExecutorInfo& info,
    JsRuntime& runtime)
{
    TAG_LOGD(AAFwkTag::INTENT, "called");
    auto executeParam = info.executeParam;
    if (executeParam == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null executeParam");
        return std::unique_ptr<NativeReference>();
    }
    if (!IsPathValid(executeParam->moduleName_) || !IsPathValid(info.srcEntry)) {
        TAG_LOGE(AAFwkTag::INTENT, "invalid moduleName or srcEntry");
        return std::unique_ptr<NativeReference>();
    }
    if (!info.hapPath.empty() && !IsPathValid(info.hapPath)) {
        TAG_LOGE(AAFwkTag::INTENT, "invalid hapPath");
        return std::unique_ptr<NativeReference>();
    }

    std::string moduleName(executeParam->moduleName_);
    std::string srcPath(executeParam->moduleName_ + "/" + info.srcEntry);
    auto pos = srcPath.rfind('.');
    if (pos == std::string::npos) {
        return nullptr;
    }
    srcPath.erase(pos);
    srcPath.append(".abc");

    std::unique_ptr<NativeReference> jsCode(
        runtime.LoadModule(moduleName, srcPath, info.hapPath, info.esmodule));
    return jsCode;
}

napi_value JsInsightIntentExecutor::CallJsFunctionWithResult(
    napi_env env,
    napi_value obj,
    const char* funcName,
    size_t argc,
    const napi_value* argv)
{
    TAG_LOGD(AAFwkTag::INTENT, "called");
    HandleEscape handleEscape(env);
    napi_value result = nullptr;
    napi_value method = AppExecFwk::GetPropertyValueByPropertyName(
        env,
        obj,
        funcName,
        napi_valuetype::napi_function);
    if (method == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null method");
        return nullptr;
    }
    napi_call_function(
        env,
        obj,
        method,
        argc,
        argv,
        &result);
    return handleEscape.Escape(result);
}

napi_value JsInsightIntentExecutor::CallJsFunctionWithResultInner(
    const char* funcName,
    size_t argc,
    const napi_value* argv)
{
    TAG_LOGD(AAFwkTag::INTENT, "called");
    HandleEscape handleEscape(runtime_);
    auto* env = runtime_.GetNapiEnv();
    napi_value obj = jsObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_valuetype::napi_object)) {
        TAG_LOGE(AAFwkTag::INTENT, "CallJsFunctionWithResultInner Type error");
        return nullptr;
    }
    return handleEscape.Escape(JsInsightIntentExecutor::CallJsFunctionWithResult(
        env,
        obj,
        funcName,
        argc,
        argv));
}

std::shared_ptr<AppExecFwk::InsightIntentExecuteResult> JsInsightIntentExecutor::GetResultFromJs(
    napi_env env, napi_value resultJs)
{
    TAG_LOGD(AAFwkTag::INTENT, "called");
    auto resultCpp = std::make_shared<AppExecFwk::InsightIntentExecuteResult>();
    if (!UnwrapExecuteResult(env, resultJs, *resultCpp)) {
        return nullptr;
    }
    return resultCpp;
}

napi_value JsInsightIntentExecutor::ResolveCbCpp(napi_env env, napi_callback_info info)
{
    TAG_LOGD(AAFwkTag::INTENT, "called");
    constexpr size_t argc = 1;
    napi_value argv[argc] = {nullptr};
    size_t actualArgc = argc;
    void* data = nullptr;
    napi_get_cb_info(env, info, &actualArgc, argv, nullptr, &data);
    auto* callback = static_cast<InsightIntentExecutorAsyncCallback*>(data);
    napi_value resultJs = argv[0];
    if (resultJs == nullptr) {
        JsInsightIntentExecutor::ReplyFailed(callback);
        return nullptr;
    }
    std::shared_ptr<AppExecFwk::InsightIntentExecuteResult> resultCpp =
        JsInsightIntentExecutor::GetResultFromJs(env, resultJs);
    if (resultCpp == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null resultCpp");
        JsInsightIntentExecutor::ReplyFailed(callback);
        return nullptr;
    }
    JsInsightIntentExecutor::ReplySucceeded(callback, resultCpp);
    return nullptr;
}

napi_value JsInsightIntentExecutor::ResolveExecuteResultWithDelay(napi_env env, napi_callback_info info)
{
    TAG_LOGD(AAFwkTag::INTENT, "called");
    constexpr size_t argc = 1;
    napi_value argv[argc] = {nullptr};
    size_t actualArgc = argc;
    void* data = nullptr;
    napi_get_cb_info(env, info, &actualArgc, argv, nullptr, &data);
    auto* callback = static_cast<InsightIntentExecutorAsyncCallback*>(data);
    napi_value resultJs = argv[0];
    if (resultJs == nullptr) {
        JsInsightIntentExecutor::ReplyFailed(callback);
        return nullptr;
    }
    std::shared_ptr<AppExecFwk::InsightIntentExecuteResult> resultCpp =
        JsInsightIntentExecutor::GetResultFromJs(env, resultJs);
    if (resultCpp == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null resultCpp");
        JsInsightIntentExecutor::ReplyFailed(callback);
        return nullptr;
    }
    resultCpp->isNeedDelayResult = true;
    JsInsightIntentExecutor::ReplySucceeded(callback, resultCpp);
    return nullptr;
}

napi_value JsInsightIntentExecutor::RejectCbCpp(napi_env env, napi_callback_info info)
{
    TAG_LOGW(AAFwkTag::INTENT, "reject function");
    void* data = nullptr;
    napi_get_cb_info(env, info, nullptr, nullptr, nullptr, &data);
    auto* callback = static_cast<InsightIntentExecutorAsyncCallback*>(data);
    JsInsightIntentExecutor::ReplyFailed(callback);
    return nullptr;
}

void JsInsightIntentExecutor::ReplyFailed(InsightIntentExecutorAsyncCallback* callback,
    InsightIntentInnerErr innerErr)
{
    TAG_LOGW(AAFwkTag::INTENT, "reply failed");
    if (callback == nullptr) {
        return;
    }
    AppExecFwk::InsightIntentExecuteResult errorResult{};
    errorResult.innerErr = innerErr;
    AAFwk::EventInfo eventInfo;
    eventInfo.errCode = innerErr;
    eventInfo.errReason = "ReplyFailed";
    AAFwk::EventReport::SendExecuteIntentEvent(
        AAFwk::EventName::EXECUTE_INSIGHT_INTENT_ERROR, HISYSEVENT_FAULT, eventInfo);
    callback->Call(errorResult);
    delete callback;
}

void JsInsightIntentExecutor::ReplySucceeded(InsightIntentExecutorAsyncCallback* callback,
    std::shared_ptr<AppExecFwk::InsightIntentExecuteResult> resultCpp)
{
    TAG_LOGD(AAFwkTag::INTENT, "called");
    if (callback == nullptr) {
        return;
    }
    if (resultCpp == nullptr) {
        ReplyFailed(callback);
        return;
    }
    resultCpp->innerErr = InsightIntentInnerErr::INSIGHT_INTENT_ERR_OK;
    callback->Call(*resultCpp);
    delete callback;
}

void JsInsightIntentExecutor::ReplyFailedInner(InsightIntentInnerErr innerErr)
{
    TAG_LOGD(AAFwkTag::INTENT, "called");
    state_ = JsInsightIntentExecutor::State::INVALID;
    auto* callback = callback_.release();
    JsInsightIntentExecutor::ReplyFailed(callback, innerErr);
}

void JsInsightIntentExecutor::ReplySucceededInner(std::shared_ptr<AppExecFwk::InsightIntentExecuteResult> resultCpp)
{
    TAG_LOGD(AAFwkTag::INTENT, "called");
    state_ = JsInsightIntentExecutor::State::EXECUTATION_DONE;
    auto* callback = callback_.release();
    JsInsightIntentExecutor::ReplySucceeded(callback, resultCpp);
}

bool JsInsightIntentExecutor::HandleResultReturnedFromJsFunc(napi_value resultJs)
{
    TAG_LOGD(AAFwkTag::INTENT, "called");
    auto* env = runtime_.GetNapiEnv();
    if (resultJs == nullptr) {
        ReplyFailedInner();
        STATE_PATTERN_NAIVE_STATE_SET_AND_RETURN(State::INVALID, false);
    }
    bool isPromise = false;
    napi_is_promise(env, resultJs, &isPromise);

    isAsync_ = isPromise;
    auto context = GetContext();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null context");
        return false;
    }
    return isPromise ? HandlePromiseResult(env, resultJs, context) : HandleSyncResult(env, resultJs, context);
}

bool JsInsightIntentExecutor::HandleSyncResult(napi_env env, napi_value resultJs,
    std::shared_ptr<InsightIntentContext> context)
{
    TAG_LOGI(AAFwkTag::INTENT, "Not promise");
    auto resultCpp = JsInsightIntentExecutor::GetResultFromJs(env, resultJs);
    if (resultCpp == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null resultCpp");
        ReplyFailedInner();
        STATE_PATTERN_NAIVE_STATE_SET_AND_RETURN(State::INVALID, false);
    }

    TAG_LOGD(AAFwkTag::INTENT, "Call succeed");
    if (context->GetDelayReturnMode() == InsightIntentReturnMode::FUNCTION) {
        TAG_LOGD(AAFwkTag::INTENT, "GetDelayReturnMode FUNCTION");
        resultCpp->isNeedDelayResult = true;
    } else {
        InsightIntentDelayResultCallbackMgr::GetInstance().RemoveDelayResultCallback(context->GetIntentId());
    }
    ReplySucceededInner(resultCpp);
    return true;
}

bool JsInsightIntentExecutor::HandlePromiseResult(napi_env env, napi_value resultJs,
    std::shared_ptr<InsightIntentContext> context)
{
    TAG_LOGI(AAFwkTag::INTENT, "Is promise");
    HandleScope handleScope(env);
    auto* callback = callback_.release();

    napi_value then = nullptr;
    napi_get_named_property(env, resultJs, "then", &then);
    napi_value resolveCbJs = nullptr;
    if (context->GetDelayReturnMode() == InsightIntentReturnMode::CALLBACK) {
        napi_create_function(env, TMP_NAPI_ANONYMOUS_FUNC, strlen(TMP_NAPI_ANONYMOUS_FUNC),
            ResolveCbCpp, callback, &resolveCbJs);
        InsightIntentDelayResultCallbackMgr::GetInstance().RemoveDelayResultCallback(context->GetIntentId());
    } else {
        napi_create_function(env, TMP_NAPI_ANONYMOUS_FUNC, strlen(TMP_NAPI_ANONYMOUS_FUNC),
            ResolveExecuteResultWithDelay, callback, &resolveCbJs);
    }
    constexpr size_t argcThen = 1;
    napi_value argvThen[argcThen] = { resolveCbJs };
    napi_call_function(env, resultJs, then, argcThen, argvThen, nullptr);
    napi_value promiseCatch = nullptr;
    napi_get_named_property(env, resultJs, "catch", &promiseCatch);
    napi_value rejectCbJs = nullptr;
    napi_create_function(env, TMP_NAPI_ANONYMOUS_FUNC, strlen(TMP_NAPI_ANONYMOUS_FUNC),
        RejectCbCpp, callback, &rejectCbJs);
    constexpr size_t argcCatch = 1;
    napi_value argvCatch[argcCatch] = { rejectCbJs };
    napi_call_function(env, resultJs, promiseCatch, argcCatch, argvCatch, nullptr);

    return true;
}

bool JsInsightIntentExecutor::CheckParametersUIAbilityForeground(const std::shared_ptr<NativeReference>& windowStage)
{
    return windowStage != nullptr;
}

bool JsInsightIntentExecutor::ExecuteInsightIntentUIAbilityForeground(
    const std::string& name,
    const AAFwk::WantParams& param,
    const std::shared_ptr<NativeReference>& windowStageJs)
{
    TAG_LOGD(AAFwkTag::INTENT, "called");
    HandleScope handleScope(runtime_);

    constexpr auto intentMode = static_cast<size_t>(InsightIntentExecuteMode::UIABILITY_FOREGROUND);
    constexpr auto funcName = JsInsightIntentExecutor::JS_FUNC_NAME_FOR_MODE[intentMode];
    constexpr auto argc = JsInsightIntentExecutor::JS_ARGC_FOR_MODE[intentMode];

    auto* env = runtime_.GetNapiEnv();
    napi_value nameJs = AppExecFwk::WrapStringToJS(env, name);
    napi_value paramJs = AppExecFwk::WrapWantParams(env, param);
    napi_value argv[argc] = { nameJs, paramJs, windowStageJs->GetNapiValue() };
    napi_value result = nullptr;
    result = CallJsFunctionWithResultInner(funcName, argc, argv);
    if (result == nullptr) {
        ReplyFailedInner();
        return false;
    }

    return HandleResultReturnedFromJsFunc(result);
}

bool JsInsightIntentExecutor::CheckParametersUIAbilityBackground()
{
    return true;
}

bool JsInsightIntentExecutor::ExecuteInsightIntentUIAbilityBackground(
    const std::string& name,
    const AAFwk::WantParams& param)
{
    TAG_LOGD(AAFwkTag::INTENT, "called");
    HandleScope handleScope(runtime_);

    constexpr auto intentMode = static_cast<size_t>(InsightIntentExecuteMode::UIABILITY_BACKGROUND);
    constexpr auto funcName = JsInsightIntentExecutor::JS_FUNC_NAME_FOR_MODE[intentMode];
    constexpr auto argc = JsInsightIntentExecutor::JS_ARGC_FOR_MODE[intentMode];

    auto* env = runtime_.GetNapiEnv();
    napi_value nameJs = AppExecFwk::WrapStringToJS(env, name);
    napi_value paramJs = AppExecFwk::WrapWantParams(env, param);
    napi_value argv[argc] = { nameJs, paramJs };
    napi_value result = nullptr;
    result = CallJsFunctionWithResultInner(funcName, argc, argv);
    if (result == nullptr) {
        ReplyFailedInner();
        return false;
    }

    return HandleResultReturnedFromJsFunc(result);
}

bool JsInsightIntentExecutor::CheckParametersUIExtension(
    const std::shared_ptr<NativeReference>& UIExtensionContentSession)
{
    return UIExtensionContentSession != nullptr;
}

bool JsInsightIntentExecutor::ExecuteInsightIntentUIExtension(
    const std::string& name,
    const AAFwk::WantParams& param,
    const std::shared_ptr<NativeReference>& UIExtensionContentSession)
{
    TAG_LOGD(AAFwkTag::INTENT, "called");
    HandleScope handleScope(runtime_);

    constexpr auto intentMode = static_cast<size_t>(InsightIntentExecuteMode::UIEXTENSION_ABILITY);
    constexpr auto funcName = JsInsightIntentExecutor::JS_FUNC_NAME_FOR_MODE[intentMode];
    constexpr auto argc = JsInsightIntentExecutor::JS_ARGC_FOR_MODE[intentMode];

    auto* env = runtime_.GetNapiEnv();
    napi_value nameJs = AppExecFwk::WrapStringToJS(env, name);
    napi_value paramJs = AppExecFwk::WrapWantParams(env, param);
    napi_value argv[argc] = { nameJs, paramJs, UIExtensionContentSession->GetNapiValue() };
    napi_value result = nullptr;
    result = CallJsFunctionWithResultInner(funcName, argc, argv);
    if (result == nullptr) {
        ReplyFailedInner();
        return false;
    }

    return HandleResultReturnedFromJsFunc(result);
}

bool JsInsightIntentExecutor::CheckParametersServiceExtension()
{
    return true;
}

bool JsInsightIntentExecutor::ExecuteInsightIntentServiceExtension(
    const std::string& name,
    const AAFwk::WantParams& param)
{
    TAG_LOGD(AAFwkTag::INTENT, "called");
    HandleScope handleScope(runtime_);

    constexpr auto intentMode = static_cast<size_t>(InsightIntentExecuteMode::SERVICE_EXTENSION_ABILITY);
    constexpr auto funcName = JsInsightIntentExecutor::JS_FUNC_NAME_FOR_MODE[intentMode];
    constexpr auto argc = JsInsightIntentExecutor::JS_ARGC_FOR_MODE[intentMode];

    auto* env = runtime_.GetNapiEnv();
    napi_value nameJs = AppExecFwk::WrapStringToJS(env, name);
    napi_value paramJs = AppExecFwk::WrapWantParams(env, param);
    napi_value argv[argc] = { nameJs, paramJs };
    napi_value result = nullptr;
    result = CallJsFunctionWithResultInner(funcName, argc, argv);
    if (result == nullptr) {
        ReplyFailedInner();
        return false;
    }

    return HandleResultReturnedFromJsFunc(result);
}
} // namespace OHOS::AbilityRuntime
