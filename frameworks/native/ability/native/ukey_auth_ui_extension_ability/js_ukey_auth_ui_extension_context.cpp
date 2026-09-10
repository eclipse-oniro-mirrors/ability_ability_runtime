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

#include "js_ukey_auth_ui_extension_context.h"

#include "ability_business_error.h"
#include "ability_manager_errors.h"
#include "hilog_tag_wrapper.h"
#include "js_error_utils.h"
#include "js_extension_context.h"
#include "js_runtime_utils.h"
#include "napi_common_want.h"
#include "native_engine/native_engine.h"
#ifdef SUPPORT_SCREEN
#include "window.h"
#endif // SUPPORT_SCREEN

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr size_t ARGC_ZERO = 0;
constexpr size_t ARGC_ONE = 1;
constexpr size_t INDEX_ZERO = 0;
}

void JsUkeyAuthUIExtensionContext::Finalizer(napi_env env, void *data, void *hint)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "called");
    if (data == nullptr) {
        return;
    }
    delete static_cast<JsUkeyAuthUIExtensionContext *>(data);
}

napi_value JsUkeyAuthUIExtensionContext::TerminateSelf(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsUkeyAuthUIExtensionContext, OnTerminateSelf);
}

napi_value JsUkeyAuthUIExtensionContext::TerminateSelfWithResult(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsUkeyAuthUIExtensionContext, OnTerminateSelfWithResult);
}

napi_value JsUkeyAuthUIExtensionContext::ReportDrawnCompleted(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsUkeyAuthUIExtensionContext, OnReportDrawnCompleted);
}

napi_value JsUkeyAuthUIExtensionContext::SetColorMode(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsUkeyAuthUIExtensionContext, OnSetColorMode);
}

napi_value JsUkeyAuthUIExtensionContext::CreateJsUkeyAuthUIExtensionContext(napi_env env,
    std::shared_ptr<UkeyAuthUIExtensionContext> context)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "called");
    std::shared_ptr<OHOS::AppExecFwk::AbilityInfo> abilityInfo = nullptr;
    if (context) {
        abilityInfo = context->GetAbilityInfo();
    }
    napi_value objValue = CreateJsExtensionContext(env, context, abilityInfo);

    std::unique_ptr<JsUkeyAuthUIExtensionContext> jsContext =
        std::make_unique<JsUkeyAuthUIExtensionContext>(context);
    napi_wrap(env, objValue, jsContext.release(), Finalizer, nullptr, nullptr);

    const char *moduleName = "JsUkeyAuthUIExtensionContext";
    BindNativeFunction(env, objValue, "terminateSelf", moduleName, TerminateSelf);
    BindNativeFunction(env, objValue, "terminateSelfWithResult", moduleName, TerminateSelfWithResult);
    BindNativeFunction(env, objValue, "reportDrawnCompleted", moduleName, ReportDrawnCompleted);
    BindNativeFunction(env, objValue, "setColorMode", moduleName, SetColorMode);

    std::string type = "UkeyAuthUIExtensionContext";
    napi_set_named_property(env, objValue, "contextType", CreateJsValue(env, type));

    return objValue;
}

napi_value JsUkeyAuthUIExtensionContext::OnTerminateSelf(napi_env env, NapiCallbackInfo &info)
{
    TAG_LOGI(AAFwkTag::UI_EXT, "called");
    napi_value lastParam = (info.argc == ARGC_ZERO) ? nullptr : info.argv[INDEX_ZERO];
    auto innerErrCode = std::make_shared<ErrCode>(ERR_OK);
    NapiAsyncTask::ExecuteCallback execute = [weak = context_, innerErrCode]() {
        auto context = weak.lock();
        if (!context) {
            TAG_LOGE(AAFwkTag::UI_EXT, "null context");
            *innerErrCode = static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
            return;
        }
        *innerErrCode = context->TerminateSelf();
    };
    NapiAsyncTask::CompleteCallback complete =
        [innerErrCode](napi_env env, NapiAsyncTask &task, int32_t status) {
            if (*innerErrCode == ERR_OK) {
                task.ResolveWithNoError(env, CreateJsUndefined(env));
            } else {
                task.Reject(env, CreateJsErrorByNativeErr(env, *innerErrCode));
            }
        };
    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JsUkeyAuthUIExtensionContext::OnTerminateSelf",
        env, CreateAsyncTaskWithLastParam(env, lastParam, std::move(execute), std::move(complete), &result));
    return result;
}

napi_value JsUkeyAuthUIExtensionContext::OnTerminateSelfWithResult(napi_env env, NapiCallbackInfo &info)
{
    TAG_LOGI(AAFwkTag::UI_EXT, "called");
    if (info.argc < ARGC_ONE) {
        TAG_LOGE(AAFwkTag::UI_EXT, "invalid argc");
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }
    int32_t resultCode = 0;
    AAFwk::Want want;
    if (!AppExecFwk::UnWrapAbilityResult(env, info.argv[INDEX_ZERO], resultCode, want)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "parse ability result failed");
        ThrowInvalidParamError(env, "Parameter error: Failed to parse parameter! Parameter must be a AbilityResult.");
        return CreateJsUndefined(env);
    }
    napi_value lastParam = (info.argc > ARGC_ONE) ? info.argv[ARGC_ONE] : nullptr;
    auto innerErrCode = std::make_shared<ErrCode>(ERR_OK);
    NapiAsyncTask::ExecuteCallback execute = [weak = context_, innerErrCode, resultCode, want]() {
        auto context = weak.lock();
        if (!context) {
            TAG_LOGE(AAFwkTag::UI_EXT, "null context");
            *innerErrCode = static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
            return;
        }
        *innerErrCode = context->TerminateSelfWithResult(resultCode, want);
    };
    NapiAsyncTask::CompleteCallback complete =
        [innerErrCode](napi_env env, NapiAsyncTask &task, int32_t status) {
            if (*innerErrCode == ERR_OK) {
                task.ResolveWithNoError(env, CreateJsUndefined(env));
            } else {
                task.Reject(env, CreateJsErrorByNativeErr(env, *innerErrCode));
            }
        };
    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JsUkeyAuthUIExtensionContext::OnTerminateSelfWithResult",
        env, CreateAsyncTaskWithLastParam(env, lastParam, std::move(execute), std::move(complete), &result));
    return result;
}

napi_value JsUkeyAuthUIExtensionContext::OnReportDrawnCompleted(napi_env env, NapiCallbackInfo &info)
{
    if (info.argc < ARGC_ONE) {
        TAG_LOGE(AAFwkTag::UI_EXT, "invalid argc");
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }
    TAG_LOGD(AAFwkTag::UI_EXT, "called");
    auto innerErrorCode = std::make_shared<int32_t>(ERR_OK);
    NapiAsyncTask::ExecuteCallback execute = [weak = context_, innerErrorCode]() {
        auto context = weak.lock();
        if (!context) {
            TAG_LOGW(AAFwkTag::UI_EXT, "null context");
            *innerErrorCode = static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
            return;
        }
        *innerErrorCode = context->ReportDrawnCompleted();
    };
    NapiAsyncTask::CompleteCallback complete = [innerErrorCode](napi_env env, NapiAsyncTask &task, int32_t status) {
        if (*innerErrorCode == ERR_OK) {
            task.Resolve(env, CreateJsUndefined(env));
        } else {
            task.Reject(env, CreateJsErrorByNativeErr(env, *innerErrorCode));
        }
    };
    napi_value lastParam = info.argv[INDEX_ZERO];
    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JsUkeyAuthUIExtensionContext::OnReportDrawnCompleted",
        env, CreateAsyncTaskWithLastParam(env, lastParam, std::move(execute), std::move(complete), &result));
    return result;
}

napi_value JsUkeyAuthUIExtensionContext::OnSetColorMode(napi_env env, NapiCallbackInfo &info)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "called");
    if (info.argc == ARGC_ZERO) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Not enough params");
        ThrowInvalidParamError(env, "Not enough params.");
        return CreateJsUndefined(env);
    }
    auto context = context_.lock();
    if (context == nullptr) {
        TAG_LOGW(AAFwkTag::UI_EXT, "context is already released");
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
        return CreateJsUndefined(env);
    }
    int32_t colorMode = 0;
    if (!ConvertFromJsValue(env, info.argv[INDEX_ZERO], colorMode)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Parse colorMode failed");
        ThrowInvalidParamError(env, "Parse param colorMode failed, colorMode must be number.");
        return CreateJsUndefined(env);
    }
    context->SetAbilityColorMode(colorMode);
    return CreateJsUndefined(env);
}
} // namespace AbilityRuntime
} // namespace OHOS
