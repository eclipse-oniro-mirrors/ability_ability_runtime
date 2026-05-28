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

#include "js_auto_fill_extension_context.h"

#include "hilog_tag_wrapper.h"
#include "js_auto_fill_extension_util.h"
#include "js_error_utils.h"
#include "js_extension_context.h"
#include "js_runtime_utils.h"
#include "napi/native_api.h"
#include "napi_common_want.h"
#include "napi_common_util.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr int32_t INDEX_ZERO = 0;
constexpr size_t ARGC_ONE = 1;
}
void JsAutoFillExtensionContext::Finalizer(napi_env env, void *data, void *hint)
{
    TAG_LOGD(AAFwkTag::AUTOFILL_EXT, "called");
    std::unique_ptr<JsAutoFillExtensionContext>(static_cast<JsAutoFillExtensionContext*>(data));
}

napi_value JsAutoFillExtensionContext::ReloadInModal(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsAutoFillExtensionContext, OnReloadInModal);
}

napi_value JsAutoFillExtensionContext::OnReloadInModal(napi_env env, NapiCallbackInfo &info)
{
    TAG_LOGD(AAFwkTag::AUTOFILL_EXT, "called");
    if (info.argc < ARGC_ONE) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "invalid argc");
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return CreateJsUndefined(env);
    }

    HandleEscape handleEscape(env);
    napi_value jsCustomData = GetPropertyValueByPropertyName(env, info.argv[INDEX_ZERO], "data", napi_object);
    CustomData customData;
    if (jsCustomData == nullptr || !AppExecFwk::UnwrapWantParams(env, jsCustomData, customData.data)) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "Parse custom data failed");
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return CreateJsUndefined(env);
    }

    auto retVal = std::make_shared<int32_t>(0);
    NapiAsyncTask::ExecuteCallback execute = [weak = context_, customData, ret = retVal, env]() {
        auto context = weak.lock();
        if (context == nullptr) {
            TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "null context");
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
            return;
        }
        if (ret == nullptr) {
            TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "null ret");
            return;
        }
        *ret = context->ReloadInModal(customData);
    };

    NapiAsyncTask::CompleteCallback complete = [ret = retVal](napi_env env, NapiAsyncTask &task, int32_t status) {
        HandleScope handleScope(env);
        if (ret == nullptr) {
            TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "null ret");
            task.Reject(env, CreateJsError(env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER),
                GetInnerErrorMsg(AbilityInnerErrorMsg::RELOAD_IN_MODAL_RESULT_NULL)));
            return;
        }
        if (*ret != ERR_OK) {
            TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "error %{public}d", *ret);
            task.Reject(env, CreateJsError(env, GetJsErrorCodeByNativeError(*ret)));
            return;
        }
        task.ResolveWithNoError(env, CreateJsUndefined(env));
    };
    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JsAutoFillExtensionContext::OnReloadInModal",
        env, CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute), std::move(complete), &result));
    return handleEscape.Escape(result);
}

napi_value JsAutoFillExtensionContext::CreateJsAutoFillExtensionContext(
    napi_env env, const std::shared_ptr<AutoFillExtensionContext> &context)
{
    TAG_LOGD(AAFwkTag::AUTOFILL_EXT, "called");
    std::shared_ptr<OHOS::AppExecFwk::AbilityInfo> abilityInfo = nullptr;
    if (context != nullptr) {
        abilityInfo = context->GetAbilityInfo();
    }
    HandleEscape handleEscape(env);
    napi_value objValue = CreateJsExtensionContext(env, context, abilityInfo);

    auto jsContext = std::make_unique<JsAutoFillExtensionContext>(context);
    napi_wrap(env, objValue, jsContext.release(), Finalizer, nullptr, nullptr);

    const char *moduleName = "JsAutoFillExtensionContext";
    BindNativeFunction(env, objValue, "reloadInModal", moduleName, ReloadInModal);

    std::string type = "AutoFillExtensionContext";
    napi_set_named_property(env, objValue, "contextType", CreateJsValue(env, type));

    return handleEscape.Escape(objValue);
}
} // namespace AbilityRuntime
} // namespace OHOS
