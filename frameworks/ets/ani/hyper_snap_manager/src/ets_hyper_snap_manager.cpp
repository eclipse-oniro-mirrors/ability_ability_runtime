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

#include "ets_hyper_snap_manager.h"

#include <unordered_map>

#include "ability_business_error.h"
#include "ani_common_util.h"
#include "ani_enum_convert.h"
#include "app_mgr_client.h"
#include "errors.h"
#include "ets_error_utils.h"
#include "hilog_tag_wrapper.h"
#include "singleton.h"
#ifdef RESOURCE_SCHEDULE_SERVICE_ENABLE
#include "res_sched_client.h"
#include "res_type.h"
#endif

namespace OHOS {
namespace HyperSnapManagerEts {
namespace {
constexpr const char *HYPER_SNAP_MANAGER_SPACE_NAME = "@ohos.app.ability.hyperSnapManager.hyperSnapManager";
constexpr const char *HYPER_SNAP_ERROR_INFO_IMPL_CLASS_NAME =
    "@ohos.app.ability.hyperSnapManager.hyperSnapManager.HyperSnapErrorInfoImpl";
constexpr const char *HYPER_SNAP_ERROR_CODE_ENUM_NAME =
    "@ohos.app.ability.hyperSnapManager.hyperSnapManager.HyperSnapErrorCode";
} // namespace

static ani_object BuildHyperSnapErrorInfo(ani_env *env, const AppExecFwk::HyperSnapErrorRecord &record)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "BuildHyperSnapErrorInfo: null env");
        return nullptr;
    }

    ani_class cls = nullptr;
    ani_status status = env->FindClass(HYPER_SNAP_ERROR_INFO_IMPL_CLASS_NAME, &cls);
    if (status != ANI_OK || cls == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "BuildHyperSnapErrorInfo: FindClass failed, status:%{public}d", status);
        return nullptr;
    }

    ani_method ctor = nullptr;
    status = env->Class_FindMethod(cls, "<ctor>", ":", &ctor);
    if (status != ANI_OK || ctor == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "BuildHyperSnapErrorInfo: find ctor failed, status:%{public}d", status);
        return nullptr;
    }

    ani_object object = nullptr;
    status = env->Object_New(cls, ctor, &object);
    if (status != ANI_OK || object == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "BuildHyperSnapErrorInfo: Object_New failed, status:%{public}d", status);
        return nullptr;
    }

    ani_enum_item codeItem = nullptr;
    if (!AAFwk::AniEnumConvertUtil::EnumConvert_NativeToEts(env, HYPER_SNAP_ERROR_CODE_ENUM_NAME,
        record.code, codeItem)) {
        TAG_LOGE(AAFwkTag::APPKIT, "BuildHyperSnapErrorInfo: convert code to enum item failed");
        return nullptr;
    }
    status = env->Object_SetPropertyByName_Ref(object, "code", codeItem);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "BuildHyperSnapErrorInfo: set code failed, status:%{public}d", status);
        return nullptr;
    }

    status = env->Object_SetPropertyByName_Ref(object, "msg", AppExecFwk::GetAniString(env, record.msg));
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "BuildHyperSnapErrorInfo: set msg failed, status:%{public}d", status);
        return nullptr;
    }

    status = env->Object_SetPropertyByName_Long(object, "occurTimeStamp",
        static_cast<ani_long>(record.occurTimeStamp));
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "BuildHyperSnapErrorInfo: set occurTimeStamp failed, status:%{public}d", status);
        return nullptr;
    }

    return object;
}

class EtsHyperSnapManager final {
public:
    static void SetHyperSnapEnabled(ani_env *env, ani_boolean enabledFlag);

    static void RequestRebuildHyperSnap(ani_env *env);

    static void NativeGetLastError(ani_env *env, ani_enum_item errType, ani_object call);
};

void EtsHyperSnapManager::SetHyperSnapEnabled(ani_env *env, ani_boolean enabledFlag)
{
    TAG_LOGD(AAFwkTag::APPKIT, "SetHyperSnapEnabled called");
#ifdef RESOURCE_SCHEDULE_SERVICE_ENABLE
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "env null");
        return;
    }

    bool flag = (enabledFlag != 0);
    std::unordered_map<std::string, std::string> payload {
        { "enableFlag", flag ? "1" : "0" },
    };
    std::unordered_map<std::string, std::string> reply;
    TAG_LOGD(AAFwkTag::APPKIT, "enableFlag is %{public}d", flag);
    uint32_t resType = ResourceSchedule::ResType::RES_TYPE_CTRL_FORKALL_IMAGE_INTERFACE;
    int32_t errCode = ResourceSchedule::ResSchedClient::GetInstance().ReportSyncEvent(resType,
        ResourceSchedule::ResType::CtrlForkallImageInterfaceCode::SET_SUPPORT_MIRROR_PROCESS, payload, reply);
    if (errCode != 0) {
        TAG_LOGE(AAFwkTag::APPKIT, "set enable fail, %{public}d", errCode);
        AbilityRuntime::EtsErrorUtil::ThrowError(env,
            AbilityRuntime::AbilityErrorCode::ERROR_CODE_SEND_REQUEST_TO_SYSTEM_FAIL);
    }
#endif
}

void EtsHyperSnapManager::RequestRebuildHyperSnap(ani_env *env)
{
    TAG_LOGD(AAFwkTag::APPKIT, "RequestRebuildHyperSnap called");
#ifdef RESOURCE_SCHEDULE_SERVICE_ENABLE
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "env null");
        return;
    }

    std::unordered_map<std::string, std::string> payload;
    std::unordered_map<std::string, std::string> reply;
    uint32_t resType = ResourceSchedule::ResType::RES_TYPE_CTRL_FORKALL_IMAGE_INTERFACE;
    int32_t errCode = ResourceSchedule::ResSchedClient::GetInstance().ReportSyncEvent(resType,
        ResourceSchedule::ResType::CtrlForkallImageInterfaceCode::REBUILD_IMAGE, payload, reply);
    if (errCode != 0) {
        TAG_LOGE(AAFwkTag::APPKIT, "rebuild fail, %{public}d", errCode);
        AbilityRuntime::EtsErrorUtil::ThrowError(env,
            AbilityRuntime::AbilityErrorCode::ERROR_CODE_SEND_REQUEST_TO_SYSTEM_FAIL);
    }
#endif
}

void EtsHyperSnapManager::NativeGetLastError(ani_env *env, ani_enum_item errType, ani_object call)
{
    TAG_LOGD(AAFwkTag::APPKIT, "NativeGetLastError called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "NativeGetLastError: null env");
        return;
    }

    int32_t typeValue = 0;
    if (!AAFwk::AniEnumConvertUtil::EnumConvert_EtsToNative(env, errType, typeValue)) {
        TAG_LOGE(AAFwkTag::APPKIT, "NativeGetLastError: parse errType failed");
        AbilityRuntime::EtsErrorUtil::ThrowInvalidParamError(env, "errType must be a valid HyperSnapErrorType.");
        return;
    }

    auto appMgrClient = DelayedSingleton<AppExecFwk::AppMgrClient>::GetInstance();
    if (appMgrClient == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "NativeGetLastError: null appMgrClient");
        ani_object error = AbilityRuntime::EtsErrorUtil::CreateError(env,
            AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER);
        AppExecFwk::AsyncCallback(env, call, error, nullptr);
        return;
    }

    AppExecFwk::HyperSnapErrorRecord record;
    int32_t result = appMgrClient->GetHyperSnapLastError(typeValue, record);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "NativeGetLastError: IPC failed, result %{public}d", result);
        ani_object error = AbilityRuntime::EtsErrorUtil::CreateError(env,
            AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER);
        AppExecFwk::AsyncCallback(env, call, error, nullptr);
        return;
    }

    TAG_LOGI(AAFwkTag::APPKIT, "NativeGetLastError success, errType:%{public}d, code:%{public}d",
        typeValue, static_cast<int32_t>(record.code));
    ani_object error = AbilityRuntime::EtsErrorUtil::CreateError(env, AbilityRuntime::AbilityErrorCode::ERROR_OK);
    ani_object info = BuildHyperSnapErrorInfo(env, record);
    AppExecFwk::AsyncCallback(env, call, error, info);
}

void EtsHyperSnapManagerRegisterInit(ani_env *env)
{
    TAG_LOGD(AAFwkTag::APPKIT, "EtsHyperSnapManagerRegisterInit call");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "env null ptr");
        return;
    }
    ani_status status = ANI_ERROR;
    if (env->ResetError() != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "ResetError failed");
    }
    ani_namespace ns;
    status = env->FindNamespace(HYPER_SNAP_MANAGER_SPACE_NAME, &ns);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "FindNamespace hyperSnapManager failed status : %{public}d", status);
        return;
    }
    std::array kitFunctions = {
        ani_native_function{
            "nativeSetHyperSnapEnabled", nullptr,
            reinterpret_cast<void *>(EtsHyperSnapManager::SetHyperSnapEnabled)},
        ani_native_function{"nativeRequestRebuildHyperSnap", nullptr,
            reinterpret_cast<void *>(EtsHyperSnapManager::RequestRebuildHyperSnap)},
        ani_native_function{"nativeGetLastError", nullptr,
            reinterpret_cast<void *>(EtsHyperSnapManager::NativeGetLastError)},
    };
    status = env->Namespace_BindNativeFunctions(ns, kitFunctions.data(), kitFunctions.size());
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "Namespace_BindNativeFunctions failed status : %{public}d", status);
    }
    if (env->ResetError() != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "ResetError failed");
    }
    TAG_LOGD(AAFwkTag::APPKIT, "EtsHyperSnapManagerRegisterInit end");
}

extern "C" {
ANI_EXPORT ani_status ANI_Constructor(ani_vm *vm, uint32_t *result)
{
    TAG_LOGD(AAFwkTag::APPKIT, "in HyperSnapManagerEts.ANI_Constructor");
    if (vm == nullptr || result == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null vm or result");
        return ANI_INVALID_ARGS;
    }

    ani_env *env = nullptr;
    ani_status status = ANI_ERROR;
    status = vm->GetEnv(ANI_VERSION_1, &env);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "GetEnv failed, status=%{public}d", status);
        return ANI_NOT_FOUND;
    }
    EtsHyperSnapManagerRegisterInit(env);
    *result = ANI_VERSION_1;
    TAG_LOGD(AAFwkTag::APPKIT, "HyperSnapManagerEts.ANI_Constructor finished");
    return ANI_OK;
}
} // extern "C"
} // namespace HyperSnapManagerEts
} // namespace OHOS
