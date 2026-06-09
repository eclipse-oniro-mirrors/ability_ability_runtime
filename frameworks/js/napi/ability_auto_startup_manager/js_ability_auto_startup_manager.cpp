/*
 * Copyright (c) 2023-2026 Huawei Device Co., Ltd.
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
#include "js_ability_auto_startup_manager.h"

#include "ability_business_error.h"
#include "ability_manager_client.h"
#include "ability_manager_interface.h"
#include "app_utils.h"
#include "auto_startup_info.h"
#include "hilog_tag_wrapper.h"
#include "ipc_skeleton.h"
#include "js_ability_auto_startup_manager_utils.h"
#include "js_error_utils.h"
#include "js_runtime_utils.h"
#include "permission_constants.h"
#include "tokenid_kit.h"

namespace OHOS {
namespace AbilityRuntime {
using namespace OHOS::AAFwk;
namespace {
constexpr size_t ARGC_ONE = 1;
constexpr size_t ARGC_TWO = 2;
constexpr int32_t INDEX_ZERO = 0;
constexpr int32_t INDEX_ONE = 1;
constexpr int32_t INVALID_PARAM = static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
constexpr const char *ON_OFF_TYPE_SYSTEM = "systemAutoStartup";
} // namespace

void JsAbilityAutoStartupManager::Finalizer(napi_env env, void *data, void *hint)
{
    TAG_LOGD(AAFwkTag::AUTO_STARTUP, "called");
    std::unique_ptr<JsAbilityAutoStartupManager>(static_cast<JsAbilityAutoStartupManager *>(data));
}

napi_value JsAbilityAutoStartupManager::RegisterAutoStartupCallback(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsAbilityAutoStartupManager, OnRegisterAutoStartupCallback);
}

napi_value JsAbilityAutoStartupManager::UnregisterAutoStartupCallback(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsAbilityAutoStartupManager, OnUnregisterAutoStartupCallback);
}

napi_value JsAbilityAutoStartupManager::SetApplicationAutoStartup(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsAbilityAutoStartupManager, OnSetApplicationAutoStartup);
}

napi_value JsAbilityAutoStartupManager::CancelApplicationAutoStartup(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsAbilityAutoStartupManager, OnCancelApplicationAutoStartup);
}

napi_value JsAbilityAutoStartupManager::QueryAllAutoStartupApplications(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsAbilityAutoStartupManager, OnQueryAllAutoStartupApplications);
}

napi_value JsAbilityAutoStartupManager::GetAutoStartupStatusForSelf(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsAbilityAutoStartupManager, OnGetAutoStartupStatusForSelf);
}

napi_value JsAbilityAutoStartupManager::IsAutoStartupSupported(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsAbilityAutoStartupManager, OnIsAutoStartupSupported);
}

bool JsAbilityAutoStartupManager::CheckCallerIsSystemApp()
{
    auto selfToken = IPCSkeleton::GetSelfTokenID();
    if (!Security::AccessToken::TokenIdKit::IsSystemAppByFullTokenID(selfToken)) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "not system app");
        return false;
    }
    return true;
}

napi_value JsAbilityAutoStartupManager::OnRegisterAutoStartupCallback(napi_env env, NapiCallbackInfo &info)
{
    TAG_LOGD(AAFwkTag::AUTO_STARTUP, "called");
    if (info.argc < ARGC_TWO) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "invalid argc");
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }

    std::string type;
    if (!ConvertFromJsValue(env, info.argv[INDEX_ZERO], type) || type != ON_OFF_TYPE_SYSTEM) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "invalid param");
        ThrowError(env, INVALID_PARAM, "Parameter error. Convert type fail.");
        return CreateJsUndefined(env);
    }

    if (!CheckCallerIsSystemApp()) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "not system app");
        ThrowError(env, AbilityErrorCode::ERROR_CODE_NOT_SYSTEM_APP);
        return CreateJsUndefined(env);
    }

    if (jsAutoStartupCallback_ == nullptr) {
        jsAutoStartupCallback_ = new (std::nothrow) JsAbilityAutoStartupCallBack(env);
        if (jsAutoStartupCallback_ == nullptr) {
            TAG_LOGE(AAFwkTag::AUTO_STARTUP, "null jsAutoStartupCallback_");
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
            return CreateJsUndefined(env);
        }

        auto ret =
            AbilityManagerClient::GetInstance()->RegisterAutoStartupSystemCallback(jsAutoStartupCallback_->AsObject());
        if (ret != ERR_OK) {
            jsAutoStartupCallback_ = nullptr;
            TAG_LOGE(AAFwkTag::AUTO_STARTUP, "reg callback failed[%{public}d]", ret);
            if (ret == CHECK_PERMISSION_FAILED) {
                ThrowNoPermissionError(env, PermissionConstants::PERMISSION_MANAGE_APP_BOOT);
            } else {
                ThrowError(env, GetJsErrorCodeByNativeError(ret));
            }
            return CreateJsUndefined(env);
        }
    }

    jsAutoStartupCallback_->Register(info.argv[INDEX_ONE]);
    return CreateJsUndefined(env);
}

napi_value JsAbilityAutoStartupManager::OnUnregisterAutoStartupCallback(napi_env env, NapiCallbackInfo &info)
{
    TAG_LOGD(AAFwkTag::AUTO_STARTUP, "called");
    if (info.argc < ARGC_ONE) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "invalid argc");
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }

    std::string type;
    if (!ConvertFromJsValue(env, info.argv[INDEX_ZERO], type) || type != ON_OFF_TYPE_SYSTEM) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "parse type failed");
        ThrowError(env, INVALID_PARAM, "Parameter error. Convert type fail.");
        return CreateJsUndefined(env);
    }

    if (!CheckCallerIsSystemApp()) {
        ThrowError(env, AbilityErrorCode::ERROR_CODE_NOT_SYSTEM_APP);
        return CreateJsUndefined(env);
    }

    if (jsAutoStartupCallback_ == nullptr) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "null jsAutoStartupCallback_");
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
        return CreateJsUndefined(env);
    }

    auto callback = info.argc > ARGC_ONE ? info.argv[INDEX_ONE] : CreateJsUndefined(env);
    jsAutoStartupCallback_->UnRegister(callback);
    if (jsAutoStartupCallback_->IsCallbacksEmpty()) {
        auto ret = AbilityManagerClient::GetInstance()->UnregisterAutoStartupSystemCallback(
            jsAutoStartupCallback_->AsObject());
        if (ret != ERR_OK) {
            if (ret == CHECK_PERMISSION_FAILED) {
                ThrowNoPermissionError(env, PermissionConstants::PERMISSION_MANAGE_APP_BOOT);
            } else {
                ThrowError(env, GetJsErrorCodeByNativeError(ret));
            }
        }
        jsAutoStartupCallback_ = nullptr;
    }

    return CreateJsUndefined(env);
}

napi_value JsAbilityAutoStartupManager::OnSetApplicationAutoStartup(napi_env env, NapiCallbackInfo &info)
{
    TAG_LOGD(AAFwkTag::AUTO_STARTUP, "called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "null env");
        return nullptr;
    }
    AbilityRuntime::HandleEscape handleEscape(env);
    if (info.argc < ARGC_ONE) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "invalid argc");
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }

    if (!CheckCallerIsSystemApp()) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "not system app");
        ThrowError(env, AbilityErrorCode::ERROR_CODE_NOT_SYSTEM_APP);
        return CreateJsUndefined(env);
    }

    AutoStartupInfo autoStartupInfo;
    if (!UnwrapAutoStartupInfo(env, info.argv[INDEX_ZERO], autoStartupInfo)) {
        ThrowError(env, INVALID_PARAM, "unwrap AutoStartupInfo failed");
        return CreateJsUndefined(env);
    }

    auto retVal = std::make_shared<int32_t>(0);
    NapiAsyncTask::ExecuteCallback execute = [autoStartupInfo, ret = retVal] () {
        if (ret == nullptr) {
            TAG_LOGE(AAFwkTag::AUTO_STARTUP, "null ret");
            return;
        }
        *ret = AbilityManagerClient::GetInstance()->SetApplicationAutoStartup(autoStartupInfo);
    };
    NapiAsyncTask::CompleteCallback complete = [ret = retVal](napi_env env, NapiAsyncTask &task, int32_t status) {
        AbilityRuntime::HandleScope handleScope(env);
        if (ret == nullptr) {
            TAG_LOGE(AAFwkTag::AUTO_STARTUP, "null ret");
            task.Reject(env, CreateJsError(env, GetJsErrorCodeByNativeError(INNER_ERR)));
            return;
        }
        if (*ret != ERR_OK) {
            TAG_LOGE(AAFwkTag::AUTO_STARTUP, "error:%{public}d", *ret);
            task.Reject(env, CreateJsError(env, GetJsErrorCodeByNativeError(*ret)));
            return;
        }
        task.ResolveWithNoError(env, CreateJsUndefined(env));
    };

    napi_value lastParam = (info.argc >= ARGC_TWO) ? info.argv[INDEX_ONE] : nullptr;
    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JsAbilityAutoStartupManager::OnSetApplicationAutoStartup", env,
        CreateAsyncTaskWithLastParam(env, lastParam, std::move(execute), std::move(complete), &result));
    return handleEscape.Escape(result);
}

napi_value JsAbilityAutoStartupManager::OnCancelApplicationAutoStartup(napi_env env, NapiCallbackInfo &info)
{
    TAG_LOGD(AAFwkTag::AUTO_STARTUP, "called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "null env");
        return nullptr;
    }
    AbilityRuntime::HandleEscape handleEscape(env);
    if (info.argc < ARGC_ONE) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "invalid argc");
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }

    if (!CheckCallerIsSystemApp()) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "not system app");
        ThrowError(env, AbilityErrorCode::ERROR_CODE_NOT_SYSTEM_APP);
        return CreateJsUndefined(env);
    }

    AutoStartupInfo autoStartupInfo;
    if (!UnwrapAutoStartupInfo(env, info.argv[INDEX_ZERO], autoStartupInfo)) {
        ThrowError(env, INVALID_PARAM, "Parameter error. Convert autoStartupInfo fail.");
        return CreateJsUndefined(env);
    }

    auto retVal = std::make_shared<int32_t>(0);
    NapiAsyncTask::ExecuteCallback execute = [autoStartupInfo, ret = retVal] () {
        if (ret == nullptr) {
            TAG_LOGE(AAFwkTag::AUTO_STARTUP, "null ret");
            return;
        }
        *ret = AbilityManagerClient::GetInstance()->CancelApplicationAutoStartup(autoStartupInfo);
    };

    NapiAsyncTask::CompleteCallback complete = [ret = retVal](napi_env env, NapiAsyncTask &task, int32_t status) {
        AbilityRuntime::HandleScope handleScope(env);
        if (ret == nullptr) {
            TAG_LOGE(AAFwkTag::AUTO_STARTUP, "null ret");
            task.Reject(env, CreateJsError(env, GetJsErrorCodeByNativeError(INNER_ERR)));
            return;
        }
        if (*ret != ERR_OK) {
            TAG_LOGE(AAFwkTag::AUTO_STARTUP, "error:%{public}d", *ret);
            task.Reject(env, CreateJsError(env, GetJsErrorCodeByNativeError(*ret)));
            return;
        }
        task.ResolveWithNoError(env, CreateJsUndefined(env));
    };

    napi_value lastParam = (info.argc >= ARGC_TWO) ? info.argv[INDEX_ONE] : nullptr;
    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JsAbilityAutoStartupManager::OnCancelApplicationAutoStartup", env,
        CreateAsyncTaskWithLastParam(env, lastParam, std::move(execute), std::move(complete), &result));
    return handleEscape.Escape(result);
}

napi_value JsAbilityAutoStartupManager::OnQueryAllAutoStartupApplications(napi_env env, const NapiCallbackInfo &info)
{
    TAG_LOGD(AAFwkTag::AUTO_STARTUP, "called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "null env");
        return nullptr;
    }
    AbilityRuntime::HandleEscape handleEscape(env);
    if (!CheckCallerIsSystemApp()) {
        ThrowError(env, AbilityErrorCode::ERROR_CODE_NOT_SYSTEM_APP);
        return CreateJsUndefined(env);
    }

    auto retVal = std::make_shared<int32_t>(0);
    auto infoList = std::make_shared<std::vector<AutoStartupInfo>>();
    NapiAsyncTask::ExecuteCallback execute = [infos = infoList, ret = retVal] () {
        if (ret == nullptr || infos == nullptr) {
            TAG_LOGE(AAFwkTag::AUTO_STARTUP, "null ret or infos");
            return;
        }
        *ret = AbilityManagerClient::GetInstance()->QueryAllAutoStartupApplications(*infos);
    };

    NapiAsyncTask::CompleteCallback complete = [infos = infoList, ret = retVal](
                                                   napi_env env, NapiAsyncTask &task, int32_t status) {
        AbilityRuntime::HandleScope handleScope(env);
        if (ret == nullptr || infos == nullptr) {
            TAG_LOGE(AAFwkTag::AUTO_STARTUP, "null ret or infos");
            task.Reject(env, CreateJsError(env, GetJsErrorCodeByNativeError(INNER_ERR)));
            return;
        }
        if (*ret != ERR_OK) {
            TAG_LOGE(AAFwkTag::AUTO_STARTUP, "error:%{public}d", *ret);
            task.Reject(env, CreateJsError(env, GetJsErrorCodeByNativeError(*ret)));
            return;
        }
        task.ResolveWithNoError(env, CreateJsAutoStartupInfoArray(env, *infos));
    };

    napi_value lastParam = (info.argc >= ARGC_ONE) ? info.argv[INDEX_ZERO] : nullptr;
    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JsAbilityAutoStartupManager::OnQueryAllAutoStartupApplications", env,
        CreateAsyncTaskWithLastParam(env, lastParam, std::move(execute), std::move(complete), &result));
    return handleEscape.Escape(result);
}

napi_value JsAbilityAutoStartupManager::OnGetAutoStartupStatusForSelf(napi_env env, const NapiCallbackInfo &info)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "null env");
        return nullptr;
    }
    AbilityRuntime::HandleEscape handleEscape(env);
    auto retVal = std::make_shared<ErrCode>(ERR_OK);
    auto isAutoStartEnabled = std::make_shared<bool>(false);
    NapiAsyncTask::ExecuteCallback execute = [ret = retVal, isAutoStartEnabled]() {
        if (ret == nullptr || isAutoStartEnabled == nullptr) {
            TAG_LOGE(AAFwkTag::AUTO_STARTUP, "null ret or isAutoStartEnabled");
            return;
        }
        *ret = AbilityManagerClient::GetInstance()->GetAutoStartupStatusForSelf(*isAutoStartEnabled);
    };

    NapiAsyncTask::CompleteCallback complete = [ret = retVal, isAutoStartEnabled](
        napi_env env, NapiAsyncTask &task, int32_t status) {
        AbilityRuntime::HandleScope handleScope(env);
        if (ret == nullptr || isAutoStartEnabled == nullptr) {
            TAG_LOGE(AAFwkTag::AUTO_STARTUP, "null ret or isAutoStartEnabled");
            task.Reject(env, CreateJsError(env, GetJsErrorCodeByNativeError(INNER_ERR)));
            return;
        }
        if (*ret != ERR_OK) {
            TAG_LOGE(AAFwkTag::AUTO_STARTUP, "error:%{public}d", *ret);
            AbilityErrorCode errCode = GetJsErrorCodeByNativeError(*ret);
            if (errCode == AbilityErrorCode::ERROR_CODE_CAPABILITY_NOT_SUPPORT) {
                task.Reject(env, CreateJsError(env, errCode));
            } else {
                task.Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INNER));
            }
            return;
        }
        task.ResolveWithNoError(env, CreateJsValue(env, *isAutoStartEnabled));
    };

    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JsAbilityAutoStartupManager::OnGetAutoStartupStatusForSelf", env,
        CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute), std::move(complete), &result));
    return handleEscape.Escape(result);
}

napi_value JsAbilityAutoStartupManager::OnIsAutoStartupSupported(napi_env env, const NapiCallbackInfo &info)
{
    TAG_LOGD(AAFwkTag::AUTO_STARTUP, "called");
    AbilityRuntime::HandleEscape handleEscape(env);
    bool isSupported = AAFwk::AppUtils::GetInstance().IsAutoStartupSupported();
    return handleEscape.Escape(CreateJsValue(env, isSupported));
}

napi_value JsAbilityAutoStartupManagerInit(napi_env env, napi_value exportObj)
{
    TAG_LOGD(AAFwkTag::AUTO_STARTUP, "called");
    if (env == nullptr || exportObj == nullptr) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "null env or exportObj");
        return nullptr;
    }

    auto jsAbilityAutoStartupManager = std::make_unique<JsAbilityAutoStartupManager>();
    napi_wrap(env, exportObj, jsAbilityAutoStartupManager.release(),
        JsAbilityAutoStartupManager::Finalizer, nullptr, nullptr);

    const char *moduleName = "JsAbilityAutoStartupManager";
    BindNativeFunction(env, exportObj, "on", moduleName, JsAbilityAutoStartupManager::RegisterAutoStartupCallback);
    BindNativeFunction(env, exportObj, "off", moduleName, JsAbilityAutoStartupManager::UnregisterAutoStartupCallback);
    BindNativeFunction(env, exportObj, "setApplicationAutoStartup", moduleName,
        JsAbilityAutoStartupManager::SetApplicationAutoStartup);
    BindNativeFunction(env, exportObj, "cancelApplicationAutoStartup", moduleName,
        JsAbilityAutoStartupManager::CancelApplicationAutoStartup);
    BindNativeFunction(env, exportObj, "queryAllAutoStartupApplications", moduleName,
        JsAbilityAutoStartupManager::QueryAllAutoStartupApplications);
    BindNativeFunction(env, exportObj, "getAutoStartupStatusForSelf", moduleName,
        JsAbilityAutoStartupManager::GetAutoStartupStatusForSelf);
    BindNativeFunction(env, exportObj, "isAutoStartupSupported", moduleName,
        JsAbilityAutoStartupManager::IsAutoStartupSupported);
    TAG_LOGD(AAFwkTag::AUTO_STARTUP, "end");
    return CreateJsUndefined(env);
}
} // namespace AbilityRuntime
} // namespace OHOS