/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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
#include "ets_ability_auto_startup_manager.h"

#include "ability_business_error.h"
#include "ability_manager_client.h"
#include "ability_manager_interface.h"
#include "app_utils.h"
#include "auto_startup_info.h"
#include "ets_ability_auto_startup_manager_utils.h"
#include "ets_error_utils.h"
#include "hilog_tag_wrapper.h"
#include "ipc_skeleton.h"
#include "permission_constants.h"

namespace OHOS {
namespace AbilityRuntime {
using namespace OHOS::AAFwk;
namespace {
constexpr int32_t INVALID_PARAM = static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
constexpr const char *ETS_AUTO_STARTUP_MANAGER_NAMESPACE =
    "@ohos.app.ability.autoStartupManager.autoStartupManager";
constexpr const char *ON_OFF_TYPE_SYSTEM = "systemAutoStartup";
} // namespace

EtsAbilityAutoStartupManager& EtsAbilityAutoStartupManager::GetInstance()
{
    static EtsAbilityAutoStartupManager instance;
    return instance;
}

void EtsAbilityAutoStartupManager::RegisterAutoStartupCallback(
    ani_env *env, ani_string aniType, ani_object callback)
{
    GetInstance().OnRegisterAutoStartupCallback(env, aniType, callback);
}

void EtsAbilityAutoStartupManager::UnregisterAutoStartupCallback(
    ani_env *env, ani_string aniType, ani_object callback)
{
    GetInstance().OnUnregisterAutoStartupCallback(env, aniType, callback);
}

void EtsAbilityAutoStartupManager::SetApplicationAutoStartup(
    ani_env *env, ani_object info, ani_object callback)
{
    GetInstance().OnSetApplicationAutoStartup(env, info, callback);
}

void EtsAbilityAutoStartupManager::CancelApplicationAutoStartup(
    ani_env *env, ani_object info, ani_object callback)
{
    GetInstance().OnCancelApplicationAutoStartup(env, info, callback);
}

void EtsAbilityAutoStartupManager::QueryAllAutoStartupApplications(
    ani_env *env, ani_object callback)
{
    GetInstance().OnQueryAllAutoStartupApplications(env, callback);
}

void EtsAbilityAutoStartupManager::GetAutoStartupStatusForSelf(ani_env *env, ani_object callback)
{
    GetInstance().OnGetAutoStartupStatusForSelf(env, callback);
}

ani_boolean EtsAbilityAutoStartupManager::IsAutoStartupSupported(ani_env *env)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "null env");
        return ANI_FALSE;
    }
    return AAFwk::AppUtils::GetInstance().IsAutoStartupSupported() ? ANI_TRUE : ANI_FALSE;
}

void EtsAbilityAutoStartupManager::AutoStartupInfoCheck(
    ani_env *env, ani_object info)
{
    GetInstance().OnAutoStartupInfoCheck(env, info);
}

void EtsAbilityAutoStartupManager::NativeCheckCallerIsSystemApp(
    ani_env *env)
{
    GetInstance().OnNativeCheckCallerIsSystemApp(env);
}

void EtsAbilityAutoStartupManager::OnRegisterAutoStartupCallback(
    ani_env *env, ani_string aniType, ani_object callback)
{
    TAG_LOGD(AAFwkTag::AUTO_STARTUP, "called RegisterAutoStartupCallback");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "null env");
        return;
    }
    std::string type;
    if (!AppExecFwk::GetStdString(env, aniType, type) || type != ON_OFF_TYPE_SYSTEM) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "invalid param");
        EtsErrorUtil::ThrowError(env, INVALID_PARAM, "Parameter error. Convert type fail.");
        return;
    }
    if (!AppExecFwk::CheckCallerIsSystemApp()) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "not system app");
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_NOT_SYSTEM_APP);
        return;
    }
    ani_vm *aniVM = nullptr;
    if (env->GetVM(&aniVM) != ANI_OK) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "get aniVM failed");
        EtsErrorUtil::ThrowInvalidParamError(env, "Get aniVm failed.");
        return;
    }
    if (etsAutoStartupCallback_ != nullptr) {
        etsAutoStartupCallback_->Register(callback);
        return;
    }
    etsAutoStartupCallback_ = new (std::nothrow) EtsAbilityAutoStartupCallback(aniVM);
    if (etsAutoStartupCallback_ == nullptr) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "null etsAutoStartupCallback_");
        EtsErrorUtil::ThrowError(env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER),
            GetInnerErrorMsg(AbilityInnerErrorMsg::CREATE_CALLBACK_FAILED));
        return;
    }
    auto ret = AbilityManagerClient::GetInstance()->RegisterAutoStartupSystemCallback(
        etsAutoStartupCallback_->AsObject());
    if (ret != ERR_OK) {
        etsAutoStartupCallback_ = nullptr;
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "reg callback failed[%{public}d]", ret);
        if (ret == CHECK_PERMISSION_FAILED) {
            EtsErrorUtil::ThrowNoPermissionError(env, PermissionConstants::PERMISSION_MANAGE_APP_BOOT);
        } else {
            EtsErrorUtil::ThrowError(env, EtsErrorUtil::CreateErrorByNativeErr(env, ret));
        }
        return;
    }
    etsAutoStartupCallback_->Register(callback);
}

void EtsAbilityAutoStartupManager::OnUnregisterAutoStartupCallback(
    ani_env *env, ani_string aniType, ani_object callback)
{
    TAG_LOGD(AAFwkTag::AUTO_STARTUP, "called UnregisterAutoStartupCallback");

    std::string type;
    if (!AppExecFwk::GetStdString(env, aniType, type) || type != ON_OFF_TYPE_SYSTEM) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "parse type failed");
        EtsErrorUtil::ThrowError(env, INVALID_PARAM, "Parameter error. Convert type fail.");
        return;
    }

    if (!AppExecFwk::CheckCallerIsSystemApp()) {
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_NOT_SYSTEM_APP);
        return;
    }

    if (etsAutoStartupCallback_ == nullptr) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "null etsAutoStartupCallback_");
        EtsErrorUtil::ThrowError(env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER),
            GetInnerErrorMsg(AbilityInnerErrorMsg::CALLBACK_NOT_REGISTERED));
        return;
    }

    etsAutoStartupCallback_->Unregister(callback);
    if (etsAutoStartupCallback_->IsCallbacksEmpty()) {
        auto ret = AbilityManagerClient::GetInstance()->UnregisterAutoStartupSystemCallback(
            etsAutoStartupCallback_->AsObject());
        if (ret != ERR_OK) {
            if (ret == CHECK_PERMISSION_FAILED) {
                EtsErrorUtil::ThrowNoPermissionError(env, PermissionConstants::PERMISSION_MANAGE_APP_BOOT);
            } else {
                EtsErrorUtil::ThrowError(env, EtsErrorUtil::CreateErrorByNativeErr(env, ret));
            }
        }
        etsAutoStartupCallback_ = nullptr;
    }
}
void EtsAbilityAutoStartupManager::OnAutoStartupInfoCheck(ani_env *env, ani_object info)
{
    TAG_LOGD(AAFwkTag::AUTO_STARTUP, "called AutoStartupInfoCheck");

    if (!AppExecFwk::CheckCallerIsSystemApp()) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "not system app");
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_NOT_SYSTEM_APP);
        return;
    }

    AutoStartupInfo autoStartupInfo;
    if (!UnwrapAutoStartupInfo(env, info, autoStartupInfo)) {
        EtsErrorUtil::ThrowInvalidParamError(env, "unwrap AutoStartupInfo failed");
        return;
    }
}
void EtsAbilityAutoStartupManager::OnSetApplicationAutoStartup(ani_env *env, ani_object info, ani_object callback)
{
    TAG_LOGD(AAFwkTag::AUTO_STARTUP, "called SetApplicationAutoStartup");

    if (!AppExecFwk::CheckCallerIsSystemApp()) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "not system app");
        AppExecFwk::AsyncCallback(env, callback,
            EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_CODE_NOT_SYSTEM_APP), nullptr);
        return;
    }

    AutoStartupInfo autoStartupInfo;
    if (!UnwrapAutoStartupInfo(env, info, autoStartupInfo)) {
        AppExecFwk::AsyncCallback(env, callback,
            EtsErrorUtil::CreateError(env, INVALID_PARAM, "unwrap AutoStartupInfo failed"), nullptr);
        return;
    }

    auto ret = AbilityManagerClient::GetInstance()->SetApplicationAutoStartup(autoStartupInfo);

    AppExecFwk::AsyncCallback(env, callback,
        EtsErrorUtil::CreateErrorByNativeErr(env, static_cast<int32_t>(ret)), nullptr);
}

void EtsAbilityAutoStartupManager::OnCancelApplicationAutoStartup(ani_env *env, ani_object info, ani_object callback)
{
    TAG_LOGD(AAFwkTag::AUTO_STARTUP, "called CancelApplicationAutoStartup");

    if (!AppExecFwk::CheckCallerIsSystemApp()) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "not system app");
        AppExecFwk::AsyncCallback(env, callback,
            EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_CODE_NOT_SYSTEM_APP), nullptr);
        return;
    }

    AutoStartupInfo autoStartupInfo;
    if (!UnwrapAutoStartupInfo(env, info, autoStartupInfo)) {
        AppExecFwk::AsyncCallback(env, callback,
            EtsErrorUtil::CreateError(env, INVALID_PARAM, "Parameter error. Convert autoStartupInfo fail."), nullptr);
        return;
    }

    auto ret = AbilityManagerClient::GetInstance()->CancelApplicationAutoStartup(autoStartupInfo);

    AppExecFwk::AsyncCallback(env, callback,
        EtsErrorUtil::CreateErrorByNativeErr(env, static_cast<int32_t>(ret)), nullptr);
}

void EtsAbilityAutoStartupManager::OnQueryAllAutoStartupApplications(ani_env *env, ani_object callback)
{
    TAG_LOGD(AAFwkTag::AUTO_STARTUP, "called QueryAllAutoStartupApplications");
    if (!AppExecFwk::CheckCallerIsSystemApp()) {
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_NOT_SYSTEM_APP);
        return;
    }
    std::vector<AutoStartupInfo> infos;
    auto ret = AbilityManagerClient::GetInstance()->QueryAllAutoStartupApplications(infos);
    if (ret != ERR_OK) {
        AppExecFwk::AsyncCallback(env, callback,
            EtsErrorUtil::CreateErrorByNativeErr(env, static_cast<int32_t>(ret)), nullptr);
        return;
    }
    ani_object result = ConvertAutoStartupInfos(env, infos);
    AppExecFwk::AsyncCallback(env, callback,
        EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_OK), result);
}

void EtsAbilityAutoStartupManager::OnGetAutoStartupStatusForSelf(ani_env *env, ani_object callback)
{
    TAG_LOGD(AAFwkTag::AUTO_STARTUP, "called GetAutoStartupStatusForSelf");
    bool isAutoStartEnabled = false;
    auto ret = AbilityManagerClient::GetInstance()->GetAutoStartupStatusForSelf(isAutoStartEnabled);
    if (ret != ERR_OK) {
        AppExecFwk::AsyncCallback(env, callback,
            EtsErrorUtil::CreateErrorByNativeErr(env, static_cast<int32_t>(ret)), nullptr);
        return;
    }
    ani_object result = AppExecFwk::CreateBoolean(env, isAutoStartEnabled);
    AppExecFwk::AsyncCallback(env, callback,
        EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_OK), result);
}

void EtsAbilityAutoStartupManager::OnNativeCheckCallerIsSystemApp(ani_env *env)
{
    TAG_LOGD(AAFwkTag::AUTO_STARTUP, "called NativeCheckCallerIsSystemApp");
    if (!AppExecFwk::CheckCallerIsSystemApp()) {
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_NOT_SYSTEM_APP);
    }
}


void EtsAbilityAutoStartupManagerInit(ani_env *env)
{
    TAG_LOGD(AAFwkTag::AUTO_STARTUP, "call EtsAbilityAutoStartupManagerInit");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "null env");
        return;
    }
    ani_status status = ANI_ERROR;
    if (env->ResetError() != ANI_OK) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "ResetError failed");
    }
    ani_namespace ns = nullptr;
    status = env->FindNamespace(ETS_AUTO_STARTUP_MANAGER_NAMESPACE, &ns);
    if (status != ANI_OK || ns == nullptr) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP,
            "FindNamespace abilityManager failed status : %{public}d or ns null", status);
        return;
    }
    std::array methods = {
        ani_native_function {"nativeOnApplicationAutoStartupStateChangeSync",
            "C{std.core.String}C{application.AutoStartupCallback.AutoStartupCallback}:",
            reinterpret_cast<void *>(EtsAbilityAutoStartupManager::RegisterAutoStartupCallback)},
        ani_native_function {"nativeOffApplicationAutoStartupStateChangeSync",
            "C{std.core.String}C{application.AutoStartupCallback.AutoStartupCallback}:",
            reinterpret_cast<void *>(EtsAbilityAutoStartupManager::UnregisterAutoStartupCallback)},
        ani_native_function {"nativeSetApplicationAutoStartup",
            "C{application.AutoStartupInfo.AutoStartupInfo}C{utils.AbilityUtils.AsyncCallbackWrapper}:",
            reinterpret_cast<void *>(EtsAbilityAutoStartupManager::SetApplicationAutoStartup)},
        ani_native_function {"autoStartupInfoCheck",
            "C{application.AutoStartupInfo.AutoStartupInfo}:",
            reinterpret_cast<void *>(EtsAbilityAutoStartupManager::AutoStartupInfoCheck)},
        ani_native_function {"nativeCancelApplicationAutoStartup",
            "C{application.AutoStartupInfo.AutoStartupInfo}C{utils.AbilityUtils.AsyncCallbackWrapper}:",
            reinterpret_cast<void *>(EtsAbilityAutoStartupManager::CancelApplicationAutoStartup)},
        ani_native_function {"nativeQueryAllAutoStartupApplications",
            "C{utils.AbilityUtils.AsyncCallbackWrapper}:",
            reinterpret_cast<void *>(EtsAbilityAutoStartupManager::QueryAllAutoStartupApplications)},
        ani_native_function {"nativeGetAutoStartupStatusForSelf", "C{utils.AbilityUtils.AsyncCallbackWrapper}:",
            reinterpret_cast<void *>(EtsAbilityAutoStartupManager::GetAutoStartupStatusForSelf)},
        ani_native_function {"nativeIsAutoStartupSupported", ":z",
            reinterpret_cast<void *>(EtsAbilityAutoStartupManager::IsAutoStartupSupported)},
        ani_native_function {"nativeCheckCallerIsSystemApp", ":",
            reinterpret_cast<void *>(EtsAbilityAutoStartupManager::NativeCheckCallerIsSystemApp)},
    };
    status = env->Namespace_BindNativeFunctions(ns, methods.data(), methods.size());
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "Namespace_BindNativeFunctions failed status : %{public}d", status);
        return;
    }
    if (env->ResetError() != ANI_OK) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "ResetError failed");
    }
}

extern "C" {
ANI_EXPORT ani_status ANI_Constructor(ani_vm *vm, uint32_t *result)
{
    TAG_LOGD(AAFwkTag::AUTO_STARTUP, "in AbilityAutoStartupManagerEts.ANI_Constructor");
    if (vm == nullptr || result == nullptr) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "null vm or result");
        return ANI_INVALID_ARGS;
    }

    ani_env *env = nullptr;
    ani_status status = ANI_ERROR;
    status = vm->GetEnv(ANI_VERSION_1, &env);
    if (status != ANI_OK || env == nullptr) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "GetEnv failed, status=%{public}d or null env", status);
        return ANI_NOT_FOUND;
    }
    EtsAbilityAutoStartupManagerInit(env);
    *result = ANI_VERSION_1;
    TAG_LOGD(AAFwkTag::AUTO_STARTUP, "AbilityAutoStartupManagerEts.ANI_Constructor finished");
    return ANI_OK;
}
}
} // namespace AbilityRuntime
} // namespace OHOS