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

#include "js_kiosk_manager.h"

#include <memory>
#include <regex>

#include "ability_manager_client.h"
#include "errors.h"
#include "hilog_tag_wrapper.h"
#include "js_error_utils.h"
#include "napi/native_api.h"
#include "napi_base_context.h"
#include "napi_common_configuration.h"
#include "napi_common_util.h"
#include "napi_common_want.h"
#include "kiosk_status.h"

namespace OHOS {
namespace AbilityRuntime {
using AbilityManagerClient = AAFwk::AbilityManagerClient;
namespace {
constexpr size_t ARGC_ONE = 1;
constexpr size_t INDEX_ZERO = 0;

class JsKioskManager final {
public:
    JsKioskManager() = default;
    ~JsKioskManager() = default;

    static napi_value EnterKioskMode(napi_env env, napi_callback_info info);
    static napi_value ExitKioskMode(napi_env env, napi_callback_info info);
    static napi_value GetKioskStatus(napi_env env, napi_callback_info info);
    static void Finalizer(napi_env env, void* data, void* hint)
    {
        TAG_LOGI(AAFwkTag::APPKIT, "finalizer called");
        std::unique_ptr<JsKioskManager>(static_cast<JsKioskManager*>(data));
    }

private:
    static napi_value CreateJsKioskStatus(napi_env env,
                                          std::shared_ptr<AAFwk::KioskStatus> kioskStatus);
    napi_value OnEnterKioskMode(napi_env env, NapiCallbackInfo &info);
    napi_value OnExitKioskMode(napi_env env, NapiCallbackInfo &info);
    napi_value OnGetKioskStatus(napi_env env, NapiCallbackInfo &info);
};
} // namespace

napi_value JsKioskManager::CreateJsKioskStatus(napi_env env,
                                               std::shared_ptr<AAFwk::KioskStatus> kioskStatus)
{
    HandleEscape handleEscape(env);
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null ObjValue");
        return nullptr;
    }
    if (kioskStatus == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null kioskStatus");
        return nullptr;
    }
    napi_set_named_property(env, objValue, "isKioskMode",
                            CreateJsValue(env, kioskStatus->isKioskMode_));
    napi_set_named_property(env, objValue, "kioskBundleName",
                            CreateJsValue(env, kioskStatus->kioskBundleName_));
    napi_set_named_property(env, objValue, "kioskBundleUid", CreateJsValue(env, kioskStatus->kioskBundleUid_));
    return handleEscape.Escape(objValue);
}

napi_value JsKioskManager::EnterKioskMode(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsKioskManager, OnEnterKioskMode);
}

napi_value JsKioskManager::OnEnterKioskMode(napi_env env, NapiCallbackInfo &info)
{
    TAG_LOGD(AAFwkTag::APPKIT, "On Enter KioskMode  start");
    HandleEscape handleEscape(env);
    if (info.argc < ARGC_ONE) {
        TAG_LOGE(AAFwkTag::APPKIT, "too few params");
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }

    auto context = OHOS::AbilityRuntime::GetStageModeContext(env, info.argv[INDEX_ZERO]);
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null context");
        ThrowInvalidParamError(env, "Failed to get context. Context must not be nullptr.");
        return CreateJsUndefined(env);
    }
    auto uiAbilityContext = AbilityRuntime::Context::ConvertTo<AbilityRuntime::AbilityContext>(context);
    if (uiAbilityContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null UIAbilityContext");
        ThrowInvalidParamError(env, "Failed to get uiAbilityContext, uiAbilityContext must not be nullptr.");
        return CreateJsUndefined(env);
    }
    auto token = uiAbilityContext->GetToken();
    auto innerErrCode = std::make_shared<ErrCode>(ERR_OK);

    NapiAsyncTask::ExecuteCallback execute = [innerErrCode, token]() {
        auto amsClient = AbilityManagerClient::GetInstance();
        if (amsClient == nullptr) {
            TAG_LOGE(AAFwkTag::APPKIT, "null amsClient");
            *innerErrCode = static_cast<int32_t>(AAFwk::INNER_ERR);
            return;
        }

        *innerErrCode = amsClient->EnterKioskMode(token);
    };

    auto complete = [innerErrCode](napi_env env, NapiAsyncTask &task, int32_t status) {
        if (*innerErrCode != ERR_OK) {
            TAG_LOGE(AAFwkTag::APPKIT, "innerErrCode=%{public}d", *innerErrCode);
            task.Reject(env, CreateJsErrorByNativeErr(env, *innerErrCode));
            return;
        }
        task.ResolveWithNoError(env, CreateJsUndefined(env));
    };

    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JsKioskManager::OnEnterKioskMode", env,
                            CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute),
                                                         std::move(complete), &result));
    return handleEscape.Escape(result);
}

napi_value JsKioskManager::ExitKioskMode(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsKioskManager, OnExitKioskMode);
}

napi_value JsKioskManager::OnExitKioskMode(napi_env env, NapiCallbackInfo &info)
{
    TAG_LOGD(AAFwkTag::APPKIT, "On Exit KioskMode");
    HandleEscape handleEscape(env);
    if (info.argc < ARGC_ONE) {
        TAG_LOGE(AAFwkTag::APPKIT, "too few params");
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }

    auto context = OHOS::AbilityRuntime::GetStageModeContext(env, info.argv[INDEX_ZERO]);
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null context");
        ThrowInvalidParamError(env, "Failed to get context. Context must not be nullptr.");
        return CreateJsUndefined(env);
    }
    auto uiAbilityContext = AbilityRuntime::Context::ConvertTo<AbilityRuntime::AbilityContext>(context);
    if (uiAbilityContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null UIAbilityContext");
        ThrowInvalidParamError(env, "Failed to get uIAbilityContext. Context must be uIAbilityContext.");
        return CreateJsUndefined(env);
    }
    auto token = uiAbilityContext->GetToken();
    auto innerErrCode = std::make_shared<ErrCode>(ERR_OK);

    NapiAsyncTask::ExecuteCallback execute = [innerErrCode, token]() {
        auto amsClient = AbilityManagerClient::GetInstance();
        if (amsClient == nullptr) {
            TAG_LOGE(AAFwkTag::APPKIT, "null amsClient");
            *innerErrCode = static_cast<int32_t>(AAFwk::INNER_ERR);
            return;
        }
        *innerErrCode = amsClient->ExitKioskMode(token);
    };

    auto complete = [innerErrCode](napi_env env, NapiAsyncTask &task, int32_t status) {
        if (*innerErrCode != ERR_OK) {
            TAG_LOGE(AAFwkTag::APPKIT, "innerErrCode=%{public}d", *innerErrCode);
            task.Reject(env, CreateJsErrorByNativeErr(env, *innerErrCode));
            return;
        }
        task.ResolveWithNoError(env, CreateJsUndefined(env));
    };
    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JsKioskManager::OnExitKioskMode", env,
                            CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute),
                                                         std::move(complete), &result));
    return handleEscape.Escape(result);
}

napi_value JsKioskManager::GetKioskStatus(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsKioskManager, OnGetKioskStatus);
}

napi_value JsKioskManager::OnGetKioskStatus(napi_env env, NapiCallbackInfo &info)
{
    TAG_LOGD(AAFwkTag::APPKIT, "Get KioskStatus");
    HandleEscape handleEscape(env);
    auto innerErrCode = std::make_shared<ErrCode>(ERR_OK);
    std::shared_ptr<AAFwk::KioskStatus> kioskStatus = std::make_shared<AAFwk::KioskStatus>();

    NapiAsyncTask::ExecuteCallback execute = [innerErrCode, kioskStatus]() {
        auto amsClient = AbilityManagerClient::GetInstance();
        if (amsClient == nullptr) {
            TAG_LOGE(AAFwkTag::APPKIT, "null amsClient");
            *innerErrCode = static_cast<int32_t>(AAFwk::INNER_ERR);
            return;
        }
        *innerErrCode = amsClient->GetKioskStatus(*kioskStatus);
    };

    auto complete = [innerErrCode, kioskStatus](napi_env env, NapiAsyncTask &task, int32_t status) {
        if (*innerErrCode != ERR_OK) {
            TAG_LOGE(AAFwkTag::APPKIT, "innerErrCode=%{public}d", *innerErrCode);
            task.Reject(env, CreateJsErrorByNativeErr(env, *innerErrCode));
            return;
        }
        task.ResolveWithNoError(env, JsKioskManager::CreateJsKioskStatus(env, kioskStatus));
    };
    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JsKioskManager::OnGetKioskStatus", env,
                            CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute),
                                                         std::move(complete), &result));
    return handleEscape.Escape(result);
}

napi_value JsKioskManagerInit(napi_env env, napi_value exportObj)
{
    HandleScope handleScope(env);
    std::unique_ptr<JsKioskManager> jsKioskManager = std::make_unique<JsKioskManager>();
    napi_wrap(env, exportObj, jsKioskManager.release(), JsKioskManager::Finalizer, nullptr,
              nullptr);

    const char *moduleName = "JsKioskManager";

    BindNativeFunction(env, exportObj, "enterKioskMode", moduleName, JsKioskManager::EnterKioskMode);
    BindNativeFunction(env, exportObj, "exitKioskMode", moduleName, JsKioskManager::ExitKioskMode);
    BindNativeFunction(env, exportObj, "getKioskStatus", moduleName, JsKioskManager::GetKioskStatus);

    TAG_LOGD(AAFwkTag::APPKIT, "end");
    return CreateJsUndefined(env);
}
}  // namespace AbilityRuntime
}  // namespace OHOS
