/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "js_insight_intent_entry.h"

#include <want_params.h>

#include "hilog_tag_wrapper.h"
#include "insight_intent_constant.h"
#include "insight_intent_delay_result_callback_mgr.h"
#include "insight_intent_execute_result.h"
#include "js_insight_intent_context.h"
#include "js_insight_intent_utils.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "napi_common_execute_result.h"
#include "napi_common_util.h"
#include "napi_common_want.h"
#include "native_reference.h"

#undef STATE_PATTERN_NAIVE_H
#define STATE_PATTERN_NAIVE_STATE state_
#include "state_pattern_naive.h"

#define TMP_NAPI_ANONYMOUS_FUNC "_"

namespace OHOS {
namespace AbilityRuntime {
std::shared_ptr<JsInsightIntentEntry> JsInsightIntentEntry::Create(JsRuntime& runtime)
{
    return std::make_shared<JsInsightIntentEntry>(runtime);
}

JsInsightIntentEntry::JsInsightIntentEntry(JsRuntime& runtime) : runtime_(runtime)
{
    TAG_LOGD(AAFwkTag::INTENT, "constructor");
}

JsInsightIntentEntry::~JsInsightIntentEntry()
{
    state_ = State::DESTROYED;
    if (safeData_ != nullptr) {
        safeData_->SetAutoReleaseMem(true);
    }
    TAG_LOGI(AAFwkTag::INTENT, "destructor");
}

bool JsInsightIntentEntry::Init(const InsightIntentExecutorInfo& insightIntentInfo)
{
    TAG_LOGD(AAFwkTag::INTENT, "Init");
    STATE_PATTERN_NAIVE_ACCEPT(State::CREATED, false);
    state_ = State::INITIALIZED;
    InsightIntentExecutor::Init(insightIntentInfo);

    HandleScope handleScope(runtime_);
    if (jsObj_ == nullptr) {
        jsObj_ = JsInsightIntentEntry::LoadJsCode(insightIntentInfo, runtime_);
        if (jsObj_ == nullptr) {
            TAG_LOGE(AAFwkTag::INTENT, "load js failed");
            STATE_PATTERN_NAIVE_STATE_SET_AND_RETURN(State::INVALID, false);
        }
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
    if (!CheckTypeForNapiValue(env, executorNapiVal, napi_object) ||
        !CheckTypeForNapiValue(env, contextNapiVal, napi_object) ||
        napi_set_named_property(env, executorNapiVal, "context", contextNapiVal) != napi_ok) {
        TAG_LOGE(AAFwkTag::INTENT, "set context property failed");
        STATE_PATTERN_NAIVE_STATE_SET_AND_RETURN(State::INVALID, false);
    }

    context->SetExecuteMode(insightIntentInfo.executeParam->executeMode_);
    return true;
}

bool JsInsightIntentEntry::HandleExecuteIntent(
    std::shared_ptr<InsightIntentExecuteParam> executeParam,
    const std::shared_ptr<NativeReference>& pageLoader,
    std::unique_ptr<InsightIntentExecutorAsyncCallback> callback,
    bool& isAsync)
{
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
    auto name = executeParam->insightIntentName_;
    TAG_LOGD(AAFwkTag::INTENT, "Execute Intent %{public}s", name.c_str());

    // Check parameter
    InsightIntentExecuteMode mode = static_cast<InsightIntentExecuteMode>(executeParam->executeMode_);
    if (!PrepareParameters(mode, pageLoader)) {
        return ExecuteIntentCheckError();
    }

    bool successful = ExecuteInsightIntent(name, *executeParam->insightIntentParam_, pageLoader);
    isAsync = isAsync_;
    return successful;
}

std::unique_ptr<NativeReference> JsInsightIntentEntry::LoadJsCode(
    const InsightIntentExecutorInfo& info,
    JsRuntime& runtime)
{
    TAG_LOGD(AAFwkTag::INTENT, "load module");
    auto executeParam = info.executeParam;
    if (executeParam == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null executeParam");
        return std::unique_ptr<NativeReference>();
    }

    HandleScope handleScope(runtime);
    std::string moduleName(executeParam->moduleName_);
    std::string hapPath(info.hapPath);
    std::string srcEntrance(executeParam->srcEntrance_);
    TAG_LOGD(AAFwkTag::INTENT, "moduleName %{public}s, hapPath %{private}s, srcEntrance %{private}s",
        moduleName.c_str(), hapPath.c_str(), srcEntrance.c_str());

    safeData_ = runtime.ExecuteSecureWithOhmUrl(moduleName, hapPath, srcEntrance);
    if (safeData_ == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "execute failed");
        return std::unique_ptr<NativeReference>();
    }

    auto exportObj = runtime.GetExportObjectFromOhmUrl(srcEntrance, "default");
    if (exportObj == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "get default object failed");
        return std::unique_ptr<NativeReference>();
    }

    napi_value instanceValue = nullptr;
    auto* env = runtime.GetNapiEnv();
    auto status = napi_new_instance(env, exportObj, 0, nullptr, &instanceValue);
    if (status != napi_ok || instanceValue == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "new instance failed %{public}d", status);
        return std::unique_ptr<NativeReference>();
    }

    napi_ref resultRef = nullptr;
    status = napi_create_reference(env, instanceValue, 1, &resultRef);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::INTENT, "create instance failed %{public}d", status);
        return std::unique_ptr<NativeReference>();
    }

    return std::unique_ptr<NativeReference>(reinterpret_cast<NativeReference*>(resultRef));
}

napi_value JsInsightIntentEntry::CallJsFunctionWithResultInner(
    const char* funcName,
    size_t argc,
    const napi_value* argv)
{
    TAG_LOGD(AAFwkTag::INTENT, "call js function");
    HandleEscape handleEscape(runtime_);
    auto* env = runtime_.GetNapiEnv();
    napi_value obj = jsObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_valuetype::napi_object)) {
        TAG_LOGE(AAFwkTag::INTENT, "call js function type error");
        return nullptr;
    }
    return handleEscape.Escape(JsInsightIntentUtils::CallJsFunctionWithResult(env, obj, funcName, argc, argv));
}

void JsInsightIntentEntry::ReplyFailedInner(InsightIntentInnerErr innerErr)
{
    TAG_LOGD(AAFwkTag::INTENT, "reply failed");
    state_ = State::INVALID;
    auto* callback = callback_.release();
    JsInsightIntentUtils::ReplyFailed(callback, innerErr);
}

void JsInsightIntentEntry::ReplySucceededInner(std::shared_ptr<AppExecFwk::InsightIntentExecuteResult> resultCpp)
{
    TAG_LOGD(AAFwkTag::INTENT, "reply succeed");
    state_ = State::EXECUTATION_DONE;
    auto* callback = callback_.release();
    JsInsightIntentUtils::ReplySucceeded(callback, resultCpp);
}

bool JsInsightIntentEntry::HandleResultReturnedFromJsFunc(napi_value resultJs)
{
    TAG_LOGD(AAFwkTag::INTENT, "handle result returned");
    HandleScope handleScope(runtime_);
    auto* env = runtime_.GetNapiEnv();
    if (resultJs == nullptr) {
        return ExecuteIntentCheckError();
    }

    bool isPromise = false;
    NAPI_CALL_BASE(env, napi_is_promise(env, resultJs, &isPromise), ExecuteIntentCheckError());
    isAsync_ = isPromise;

    if (!isPromise) {
        // onExecute only support promise
        TAG_LOGW(AAFwkTag::INTENT, "no promise, don't support for now");
        return ExecuteIntentCheckError();
    }

    TAG_LOGI(AAFwkTag::INTENT, "Is promise");
    auto* callback = callback_.release();

    napi_value then = nullptr;
    NAPI_CALL_BASE(env, napi_get_named_property(env, resultJs, "then", &then), ExecuteIntentCheckError());
    napi_value resolveCbJs = nullptr;
    auto context = GetContext();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null context");
        return ExecuteIntentCheckError();
    }
    if (context->GetDelayReturnMode() == InsightIntentReturnMode::CALLBACK) {
        NAPI_CALL_BASE(env, napi_create_function(env, TMP_NAPI_ANONYMOUS_FUNC, strlen(TMP_NAPI_ANONYMOUS_FUNC),
            JsInsightIntentUtils::ResolveCbCpp, callback, &resolveCbJs), ExecuteIntentCheckError());
        InsightIntentDelayResultCallbackMgr::GetInstance().RemoveDelayResultCallback(context->GetIntentId());
    } else {
        NAPI_CALL_BASE(env, napi_create_function(env, TMP_NAPI_ANONYMOUS_FUNC, strlen(TMP_NAPI_ANONYMOUS_FUNC),
            JsInsightIntentUtils::ResolveExecuteResultWithDelay, callback, &resolveCbJs), ExecuteIntentCheckError());
    }
    constexpr size_t argcThen = 1;
    napi_value argvThen[argcThen] = { resolveCbJs };
    NAPI_CALL_BASE(env, napi_call_function(env, resultJs, then, argcThen, argvThen, nullptr),
        ExecuteIntentCheckError());

    napi_value promiseCatch = nullptr;
    NAPI_CALL_BASE(env, napi_get_named_property(env, resultJs, "catch", &promiseCatch), ExecuteIntentCheckError());
    napi_value rejectCbJs = nullptr;
    NAPI_CALL_BASE(env, napi_create_function(env, TMP_NAPI_ANONYMOUS_FUNC, strlen(TMP_NAPI_ANONYMOUS_FUNC),
        JsInsightIntentUtils::RejectCbCpp, callback, &rejectCbJs), ExecuteIntentCheckError());
    constexpr size_t argcCatch = 1;
    napi_value argvCatch[argcCatch] = { rejectCbJs };
    NAPI_CALL_BASE(env, napi_call_function(env, resultJs, promiseCatch, argcCatch, argvCatch, nullptr),
        ExecuteIntentCheckError());

    return true;
}

bool JsInsightIntentEntry::ExecuteIntentCheckError()
{
    ReplyFailedInner();
    STATE_PATTERN_NAIVE_STATE_SET_AND_RETURN(State::INVALID, false);
}

bool JsInsightIntentEntry::ExecuteInsightIntent(
    const std::string& name,
    const AAFwk::WantParams& param,
    const std::shared_ptr<NativeReference>& windowStageJs)
{
    TAG_LOGD(AAFwkTag::INTENT, "execute insight intent %{public}s", name.c_str());
    HandleScope handleScope(runtime_);
    auto env = runtime_.GetNapiEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null env");
        return ExecuteIntentCheckError();
    }

    if (!AssignObject(env, param)) {
        TAG_LOGE(AAFwkTag::INTENT, "assign object failed");
        return ExecuteIntentCheckError();
    }

    napi_value result = nullptr;
    result = CallJsFunctionWithResultInner("onExecute", 0, nullptr);
    if (result == nullptr) {
        // error log has printed
        return ExecuteIntentCheckError();
    }

    return HandleResultReturnedFromJsFunc(result);
}

bool JsInsightIntentEntry::PrepareParameters(InsightIntentExecuteMode mode,
    const std::shared_ptr<NativeReference>& pageLoader)
{
    auto env = runtime_.GetNapiEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null env");
        return false;
    }

    switch (mode) {
        case InsightIntentExecuteMode::UIABILITY_FOREGROUND:
            return PrepareParametersUIAbilityForeground(env, pageLoader);
        case InsightIntentExecuteMode::UIABILITY_BACKGROUND:
            return PrepareParametersUIAbilityBackground(env);
        case InsightIntentExecuteMode::UIEXTENSION_ABILITY:
            return PrepareParametersUIExtension(env, pageLoader);
        case InsightIntentExecuteMode::SERVICE_EXTENSION_ABILITY:
            return PrepareParametersServiceExtension(env);
        default:
            TAG_LOGE(AAFwkTag::INTENT, "InsightIntentExecuteMode not supported yet");
            return false;
    }

    return true;
}

bool JsInsightIntentEntry::PrepareParametersUIAbilityForeground(napi_env env,
    const std::shared_ptr<NativeReference>& pageLoader)
{
    // check pageloader and jsObj_
    if (pageLoader == nullptr || jsObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "page loader or object invalid");
        return false;
    }

    // assign ability pageloader
    HandleScope handleScope(env);
    auto executorNapiVal = jsObj_->GetNapiValue();
    auto pageLoaderNapiVal = pageLoader->GetNapiValue();
    if (!CheckTypeForNapiValue(env, executorNapiVal, napi_object) ||
        !CheckTypeForNapiValue(env, pageLoaderNapiVal, napi_object) ||
        napi_set_named_property(env, executorNapiVal, "windowStage", pageLoaderNapiVal) != napi_ok) {
        TAG_LOGE(AAFwkTag::INTENT, "set windowStage property failed");
        return false;
    }

    // assign execute mode
    auto mode = InsightIntentExecuteMode::UIABILITY_FOREGROUND;
    auto modeNapiVal = AppExecFwk::WrapInt32ToJS(env, static_cast<int32_t>(mode));
    auto status = napi_set_named_property(env, executorNapiVal, "executeMode", modeNapiVal);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::INTENT, "set executeMode property failed with %{public}d", status);
        return false;
    }

    return true;
}

bool JsInsightIntentEntry::PrepareParametersUIAbilityBackground(napi_env env)
{
    if (jsObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "js object null");
        return false;
    }
    AbilityRuntime::HandleScope handleScope(env);

    auto executorNapiVal = jsObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, executorNapiVal, napi_object)) {
        TAG_LOGE(AAFwkTag::INTENT, "js object invalid");
        return false;
    }

    auto mode = InsightIntentExecuteMode::UIABILITY_BACKGROUND;
    auto modeNapiVal = AppExecFwk::WrapInt32ToJS(env, static_cast<int32_t>(mode));
    auto status = napi_set_named_property(env, executorNapiVal, "executeMode", modeNapiVal);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::INTENT, "set executeMode property failed with %{public}d", status);
        return false;
    }

    return true;
}

bool JsInsightIntentEntry::PrepareParametersUIExtension(napi_env env,
    const std::shared_ptr<NativeReference>& pageLoader)
{
    // check pageloader and jsObj_
    if (pageLoader == nullptr || jsObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "page loader or object invalid");
        return false;
    }
    AbilityRuntime::HandleScope handleScope(env);

    // assign ability pageloader
    auto executorNapiVal = jsObj_->GetNapiValue();
    auto pageLoaderNapiVal = pageLoader->GetNapiValue();
    if (!CheckTypeForNapiValue(env, executorNapiVal, napi_object) ||
        !CheckTypeForNapiValue(env, pageLoaderNapiVal, napi_object) ||
        napi_set_named_property(env, executorNapiVal, "uiExtensionSession", pageLoaderNapiVal) != napi_ok) {
        TAG_LOGE(AAFwkTag::INTENT, "set uiExtensionSession property failed");
        return false;
    }

    // assign execute mode
    auto mode = InsightIntentExecuteMode::UIEXTENSION_ABILITY;
    auto modeNapiVal = AppExecFwk::WrapInt32ToJS(env, static_cast<int32_t>(mode));
    auto status = napi_set_named_property(env, executorNapiVal, "executeMode", modeNapiVal);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::INTENT, "set executeMode property failed with %{public}d", status);
        return false;
    }

    return true;
}

bool JsInsightIntentEntry::PrepareParametersServiceExtension(napi_env env)
{
    if (jsObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "js object null");
        return false;
    }
    AbilityRuntime::HandleScope handleScope(env);

    auto executorNapiVal = jsObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, executorNapiVal, napi_object)) {
        TAG_LOGE(AAFwkTag::INTENT, "js object invalid");
        return false;
    }

    auto mode = InsightIntentExecuteMode::SERVICE_EXTENSION_ABILITY;
    auto modeNapiVal = AppExecFwk::WrapInt32ToJS(env, static_cast<int32_t>(mode));
    auto status = napi_set_named_property(env, executorNapiVal, "executeMode", modeNapiVal);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::INTENT, "set executeMode property failed with %{public}d", status);
        return false;
    }

    return true;
}

bool JsInsightIntentEntry::AssignObject(napi_env env, const AAFwk::WantParams &wantParams)
{
    AAFwk::WantParamWrapper wrapper(wantParams);
    std::string parametersString = wrapper.ToString();
    TAG_LOGD(AAFwkTag::INTENT, "param string %{public}s", parametersString.c_str());

    auto executorNapiVal = jsObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, executorNapiVal, napi_object)) {
        TAG_LOGE(AAFwkTag::INTENT, "check type failed");
        return false;
    }

    napi_value srcObj = AppExecFwk::WrapWantParams(env, wantParams);

    // Get all property
    napi_value propertys;
    NAPI_CALL_BASE(env, napi_get_property_names(env, srcObj, &propertys), false);

    uint32_t propCnt;
    NAPI_CALL_BASE(env, napi_get_array_length(env, propertys, &propCnt), false);

    for (uint32_t i = 0; i < propCnt; i++) {
        napi_value propKey;
        NAPI_CALL_BASE(env, napi_get_element(env, propertys, i, &propKey), false);

        std::string propName;
        if (!ConvertFromJsValue(env, propKey, propName)) {
            TAG_LOGE(AAFwkTag::INTENT, "convert napi value failed");
            return false;
        }
        TAG_LOGD(AAFwkTag::INTENT, "param %{public}s", propName.c_str());

        napi_value propValue;
        NAPI_CALL_BASE(env, napi_get_property(env, srcObj, propKey, &propValue), false);
        NAPI_CALL_BASE(env, napi_set_property(env, executorNapiVal, propKey, propValue), false);
    }

    return true;
}
} // namespace AbilityRuntime
} // namespace OHOS
