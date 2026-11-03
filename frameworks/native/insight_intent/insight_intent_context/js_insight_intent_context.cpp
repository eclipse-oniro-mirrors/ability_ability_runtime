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

#include "js_insight_intent_context.h"

#include "ability_window_configuration.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "js_error_utils.h"
#include "js_insight_intent_execution_info.h"
#include "napi_common_want.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr static char CONTEXT_MODULE_NAME[] = "InsightIntentContext";
constexpr int32_t INDEX_ZERO = 0;
constexpr size_t ARGC_ONE = 1;
}

void JsInsightIntentContext::Finalizer(napi_env env, void* data, void* hint)
{
    TAG_LOGI(AAFwkTag::INTENT, "called");
    std::unique_ptr<JsInsightIntentContext>(static_cast<JsInsightIntentContext*>(data));
}

napi_value JsInsightIntentContext::StartAbiity(napi_env env, napi_callback_info info)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    GET_NAPI_INFO_AND_CALL(env, info, JsInsightIntentContext, OnStartAbility);
}

napi_value JsInsightIntentContext::SetReturnModeForUiAbilityForeground(napi_env env, napi_callback_info info)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    GET_NAPI_INFO_AND_CALL(env, info, JsInsightIntentContext, OnSetReturnModeForUiAbilityForeground);
}

napi_value JsInsightIntentContext::SetReturnModeForUiExtensionAbility(napi_env env, napi_callback_info info)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    GET_NAPI_INFO_AND_CALL(env, info, JsInsightIntentContext, OnSetReturnModeForUiExtensionAbility);
}

napi_value JsInsightIntentContext::GetInsightIntentExecutionInfo(napi_env env, napi_callback_info info)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    GET_NAPI_INFO_AND_CALL(env, info, JsInsightIntentContext, OnGetInsightIntentExecutionInfo);
}

napi_value JsInsightIntentContext::OnStartAbility(napi_env env, NapiCallbackInfo& info)
{
    TAG_LOGD(AAFwkTag::INTENT, "called");
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HandleEscape handleEscape(env);

    // unwrap want
    AAFwk::Want want;

    bool checkParamResult = CheckStartAbilityParam(env, info, want);
    if (!checkParamResult) {
        TAG_LOGE(AAFwkTag::INTENT, "check startAbility param failed");
        return CreateJsUndefined(env);
    }

    auto context = context_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null context");
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
        return CreateJsUndefined(env);
    }

    // verify if bundleName is empty or invalid
    if (want.GetElement().GetBundleName().empty() || want.GetElement().GetBundleName() != context->GetBundleName()) {
        TAG_LOGE(AAFwkTag::INTENT, "bundleName empty or invalid");
        ThrowError(env, AbilityErrorCode::ERROR_CODE_OPERATION_NOT_SUPPORTED);
        return CreateJsUndefined(env);
    }
    // modify windowmode setting
    if (context->GetCurrentWindowMode() == AAFwk::AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_PRIMARY ||
        context->GetCurrentWindowMode() == AAFwk::AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_SECONDARY) {
        want.SetParam(AAFwk::Want::PARAM_RESV_WINDOW_MODE, context->GetCurrentWindowMode());
    }

    auto innerErrCode = std::make_shared<ErrCode>(ERR_OK);
    // create execute task
    NapiAsyncTask::ExecuteCallback execute = [weak = context_, want, innerErrCode]() {
        auto context = weak.lock();
        if (!context) {
            TAG_LOGE(AAFwkTag::INTENT, "null context");
            *innerErrCode = static_cast<int>(AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
            return;
        }
        *innerErrCode = context->StartAbilityByInsightIntent(want);
    };
    // create complete task
    NapiAsyncTask::CompleteCallback complete = [innerErrCode](napi_env env, NapiAsyncTask& task, int32_t status) {
        HandleScope handleScope(env);
        if (*innerErrCode == ERR_OK) {
            task.Resolve(env, CreateJsUndefined(env));
        } else {
            task.Reject(env, CreateJsErrorByNativeErr(env, *innerErrCode));
        }
    };

    napi_value lastParam = (info.argc > 1) ? info.argv[1] : nullptr;
    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JsInsightIntentContext::OnStartAbility", env,
        CreateAsyncTaskWithLastParam(env, lastParam, std::move(execute), std::move(complete), &result));
    TAG_LOGD(AAFwkTag::INTENT, "end");
    return handleEscape.Escape(result);
}

napi_value JsInsightIntentContext::OnSetReturnModeForUiAbilityForeground(napi_env env, NapiCallbackInfo& info)
{
    TAG_LOGE(AAFwkTag::INTENT, "called");
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HandleScope handleScope(env);
    if (info.argc < ARGC_ONE) {
        TAG_LOGE(AAFwkTag::INTENT, "invalid argc");
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }
    auto context = context_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null context");
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
        return CreateJsUndefined(env);
    }

    InsightIntentExecuteMode mode = static_cast<InsightIntentExecuteMode>(context->GetExecuteMode());
    if (mode != InsightIntentExecuteMode::UIABILITY_FOREGROUND) {
        TAG_LOGE(AAFwkTag::INTENT, "invalid execute mode");
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
        return CreateJsUndefined(env);
    }
    int32_t returnMode;
    if (!ConvertFromJsValue(env, info.argv[INDEX_ZERO], returnMode)) {
        TAG_LOGE(AAFwkTag::INTENT, "convert returnMode failed");
        ThrowInvalidParamError(env, "Parse param returnMode failed, must be a returnMode.");
        return CreateJsUndefined(env);
    }
    context->SetDelayReturnMode(static_cast<InsightIntentReturnMode>(returnMode));
    return CreateJsUndefined(env);
}

napi_value JsInsightIntentContext::OnSetReturnModeForUiExtensionAbility(napi_env env, NapiCallbackInfo& info)
{
    TAG_LOGE(AAFwkTag::INTENT, "called");
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HandleScope handleScope(env);
    if (info.argc < ARGC_ONE) {
        TAG_LOGE(AAFwkTag::INTENT, "invalid argc");
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }
    auto context = context_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null context");
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
        return CreateJsUndefined(env);
    }

    InsightIntentExecuteMode mode = static_cast<InsightIntentExecuteMode>(context->GetExecuteMode());
    if (mode != InsightIntentExecuteMode::UIEXTENSION_ABILITY) {
        TAG_LOGE(AAFwkTag::INTENT, "invalid execute mode");
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
        return CreateJsUndefined(env);
    }

    int32_t returnMode;
    if (!ConvertFromJsValue(env, info.argv[INDEX_ZERO], returnMode)) {
        TAG_LOGE(AAFwkTag::INTENT, "convert returnMode failed");
        ThrowInvalidParamError(env, "Parse param returnMode failed, must be a returnMode.");
        return CreateJsUndefined(env);
    }
    context->SetDelayReturnMode(static_cast<InsightIntentReturnMode>(returnMode));
    return CreateJsUndefined(env);
}

napi_value JsInsightIntentContext::OnGetInsightIntentExecutionInfo(napi_env env, NapiCallbackInfo& info)
{
    TAG_LOGD(AAFwkTag::INTENT, "called");
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto context = context_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null context");
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
        return CreateJsUndefined(env);
    }
    AppExecFwk::InsightIntentExecutionInfo executeInfo;
    auto errCode = context->GetInsightIntentExecutionInfo(executeInfo);
    if (errCode != ERR_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "get execution info failed, err: %{public}d", errCode);
        ThrowErrorByNativeErr(env, errCode);
        return CreateJsUndefined(env);
    }
    return CreateJsInsightIntentExecutionInfo(env, executeInfo);
}

napi_value CreateJsInsightIntentContext(napi_env env, const std::shared_ptr<InsightIntentContext>& context)
{
    TAG_LOGD(AAFwkTag::INTENT, "called");
    HandleEscape handleEscape(env);
    napi_value contextObj;
    napi_create_object(env, &contextObj);

    std::unique_ptr<JsInsightIntentContext> jsInsightIntentContext = std::make_unique<JsInsightIntentContext>(context);
    napi_wrap(env, contextObj, jsInsightIntentContext.release(), JsInsightIntentContext::Finalizer, nullptr, nullptr);

    std::string type = "InsightIntentContext";
    napi_set_named_property(env, contextObj, "contextType", CreateJsValue(env, type));
    BindNativeFunction(env, contextObj, "startAbility", CONTEXT_MODULE_NAME, JsInsightIntentContext::StartAbiity);
    BindNativeFunction(env, contextObj, "setReturnModeForUIAbilityForeground", CONTEXT_MODULE_NAME,
        JsInsightIntentContext::SetReturnModeForUiAbilityForeground);
    BindNativeFunction(env, contextObj, "setReturnModeForUIExtensionAbility", CONTEXT_MODULE_NAME,
        JsInsightIntentContext::SetReturnModeForUiExtensionAbility);
    BindNativeProperty(env, contextObj, "insightIntentExecutionInfo",
        JsInsightIntentContext::GetInsightIntentExecutionInfo);
    TAG_LOGD(AAFwkTag::INTENT, "end");
    return handleEscape.Escape(contextObj);
}

bool CheckStartAbilityParam(napi_env env, NapiCallbackInfo& info, AAFwk::Want& want)
{
    if (info.argc == 0) {
        TAG_LOGE(AAFwkTag::INTENT, "invalid argc");
        ThrowTooFewParametersError(env);
        return false;
    }

    // unwrap want
    bool unwrapWantFlag = OHOS::AppExecFwk::UnwrapWant(env, info.argv[0], want);
    if (!unwrapWantFlag) {
        TAG_LOGE(AAFwkTag::INTENT, "parse want fail");
        ThrowInvalidParamError(env, "Parameter error: Failed to parse want, must be a Want.");
        return false;
    }
    return true;
}
} // namespace AbilityRuntime
} // namespace OHOS
