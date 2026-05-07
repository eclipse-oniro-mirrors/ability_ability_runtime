/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License"),
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

#include "ani.h"

#include "app_recovery.h"
#include "hilog_tag_wrapper.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "ani_common_want.h"
#include "want.h"
#include "ability_manager_client.h"
#include "exit_reason.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr int RESTART_FLAG_INDEX = 0; // 0:RestartFlag
constexpr int OCCASION_FLAG_INDEX = 1; // 1:SaveOccasionFlag
constexpr int MODE_FLAG_INDEX = 2; // 2:SaveModeFlag
constexpr int RESTART_APP_ID = 3;
constexpr const char *const  RESTART_APP_REASON = "RestartApp";
constexpr const char *const  RESTART_REASON = "Restart";
}
using namespace OHOS::AppExecFwk;

bool IsRefUndefined(ani_env *env, ani_ref ref)
{
    ani_boolean isUndefined = ANI_FALSE;
    env->Reference_IsUndefined(ref, &isUndefined);
    return isUndefined;
}

bool IsNull(ani_env *env, ani_ref ref)
{
    ani_boolean isNull = ANI_FALSE;
    env->Reference_IsNull(ref, &isNull);
    return isNull;
}

bool CheckParamsValid(const uint16_t params[])
{
    uint16_t restartFlag = params[0];
    constexpr uint16_t restartMaxVal = 0x0003;
    if (restartFlag > restartMaxVal && restartFlag != RestartFlag::NO_RESTART) {
        TAG_LOGE(AAFwkTag::RECOVERY, "invalid restartFlag: %{public}d", restartFlag);
        return false;
    }
    uint16_t saveFlag = params[1];
    constexpr uint16_t saveMaxVal = 0x0003;
    if (saveFlag < SaveOccasionFlag::SAVE_WHEN_ERROR || saveFlag > saveMaxVal) {
        TAG_LOGE(AAFwkTag::RECOVERY, "invalid saveOccasionFlag: %{public}d", saveFlag);
        return false;
    }
    uint16_t saveModeFlag = params[2];
    if (saveModeFlag < SaveModeFlag::SAVE_WITH_FILE || saveModeFlag > SaveModeFlag::SAVE_WITH_SHARED_MEMORY) {
        TAG_LOGE(AAFwkTag::RECOVERY, "invalid saveModeFlag: %{public}d", saveModeFlag);
        return false;
    }
    return true;
}

static bool ParseParamToEnum(ani_env *env, ani_enum_item flagObj, uint16_t &flag)
{
    if (IsNull(env, flagObj) || IsRefUndefined(env, flagObj)) {
        TAG_LOGI(AAFwkTag::RECOVERY, "param is undefined or null.");
        return true;
    }
    ani_int flagValue = 0;
    if (ANI_OK != env->EnumItem_GetValue_Int(flagObj, &flagValue)) {
        TAG_LOGE(AAFwkTag::RECOVERY, "enum convert to int failed.");
        return false;
    }
    flag = static_cast<uint16_t>(flagValue);
    return true;
}

static ani_object EnableAppRecovery(ani_env *env, ani_enum_item restart, ani_enum_item occasion, ani_enum_item modeFlag)
{
    ani_object resultsObj{};
    uint16_t flags[] = {
        RestartFlag::ALWAYS_RESTART,
        SaveOccasionFlag::SAVE_WHEN_ERROR,
        SaveModeFlag::SAVE_WITH_FILE
    };

    if (!ParseParamToEnum(env, restart, flags[RESTART_FLAG_INDEX]) ||
        !ParseParamToEnum(env, occasion, flags[OCCASION_FLAG_INDEX]) ||
        !ParseParamToEnum(env, modeFlag, flags[MODE_FLAG_INDEX])) {
        TAG_LOGE(AAFwkTag::RECOVERY, "parse params to int failed.");
        return resultsObj;
    }

    if (!CheckParamsValid(flags)) {
        return resultsObj;
    }

    AppRecovery::GetInstance().EnableAppRecovery(flags[RESTART_FLAG_INDEX],
        flags[OCCASION_FLAG_INDEX],
        flags[MODE_FLAG_INDEX]);
    return resultsObj;
}

static ani_boolean SaveAppState(ani_env *env, ani_object context)
{
    ani_boolean boolValue = ANI_FALSE;

    uintptr_t ability = 0;
    if (!IsNull(env, context) && !IsRefUndefined(env, context)) {
        ani_long contextLong{};
        if (ANI_OK != env->Object_GetFieldByName_Long(context, "nativeContext", &contextLong)) {
            TAG_LOGE(AAFwkTag::RECOVERY, "get nativeContext failed.");
            return boolValue;
        }
        ability = static_cast<unsigned long>(contextLong);
    }
    if (AppRecovery::GetInstance().ScheduleSaveAppState(StateReason::DEVELOPER_REQUEST, ability)) {
        boolValue = ANI_TRUE;
    }
    return boolValue;
}

static ani_object RestartApp(ani_env *env)
{
    ani_object resultsObj{};
    AAFwk::ExitReasonCompability exitReason = { REASON_NORMAL, "" };
    exitReason.killId = RESTART_APP_ID;
    exitReason.killMsg = RESTART_REASON;
    exitReason.innerMsg = RESTART_APP_REASON;
    int32_t pid = IPCSkeleton::GetCallingPid();
    int32_t uid = IPCSkeleton::GetCallingUid();
    auto result = AbilityManagerClient::GetInstance()->RecordAppWithReason(pid, uid, exitReason);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "Record recovery restart result=%{public}d, "
        "pid=%{public}d, uid=%{public}d, killId=%{public}d", result, pid, uid, exitReason.killId);
    AppRecovery::GetInstance().ScheduleRecoverApp(StateReason::DEVELOPER_REQUEST);
    return resultsObj;
}

static ani_object SetRestartWant(ani_env *env, ani_object wantObj)
{
    ani_object resultsObj{};
    if (IsRefUndefined(env, wantObj)) {
        TAG_LOGE(AAFwkTag::RECOVERY, "get want param failed.");
        return resultsObj;
    }
    std::shared_ptr<AAFwk::Want> want = std::make_shared<AAFwk::Want>();
    if (!AppExecFwk::UnwrapWant(env, wantObj, *want)) {
        TAG_LOGE(AAFwkTag::RECOVERY, "unwrap want failed.");
        return resultsObj;
    }
    AppRecovery::GetInstance().SetRestartWant(want);
    return resultsObj;
}

static void EtsAppRecoveryInit(ani_env *env)
{
    TAG_LOGD(AAFwkTag::RECOVERY, "EtsAppRecoveryInit Called.");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::RECOVERY, "null env");
        return;
    }
    ani_status status = ANI_ERROR;
    if (env->ResetError() != ANI_OK) {
        TAG_LOGE(AAFwkTag::RECOVERY, "ResetError failed");
    }

    ani_namespace ns;
    status = env->FindNamespace("@ohos.app.ability.appRecovery.appRecovery", &ns);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::RECOVERY, "FindNamespace appRecovery failed status: %{public}d", status);
        return;
    }

    std::array kitFunctions = {
        ani_native_function {"enableAppRecovery", nullptr, reinterpret_cast<void *>(EnableAppRecovery)},
        ani_native_function {"restartApp", nullptr, reinterpret_cast<void *>(RestartApp)},
        ani_native_function {"saveAppState", nullptr, reinterpret_cast<void *>(SaveAppState)},
        ani_native_function {"setRestartWant", nullptr, reinterpret_cast<void *>(SetRestartWant)},
    };

    status = env->Namespace_BindNativeFunctions(ns, kitFunctions.data(), kitFunctions.size());
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::RECOVERY, "Namespace_BindNativeFunctions failed status: %{public}d", status);
    }

    if (env->ResetError() != ANI_OK) {
        TAG_LOGE(AAFwkTag::RECOVERY, "ResetError failed");
    }
    TAG_LOGD(AAFwkTag::RECOVERY, "EtsAppRecoveryInit end");
}
}  // namespace AbilityRuntime
}  // namespace OHOS

ANI_EXPORT ani_status ANI_Constructor(ani_vm *vm, uint32_t *result)
{
    TAG_LOGD(AAFwkTag::RECOVERY, "ANI_Constructor start.");
    ani_env *env = nullptr;
    ani_status status = ANI_ERROR;
    if (vm == nullptr) {
        TAG_LOGE(AAFwkTag::RECOVERY, "null vm");
        return ANI_ERROR;
    }
    status = vm->GetEnv(ANI_VERSION_1, &env);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::RECOVERY, "GetEnv failed status: %{public}d", status);
        return ANI_NOT_FOUND;
    }

    OHOS::AbilityRuntime::EtsAppRecoveryInit(env);
    *result = ANI_VERSION_1;
    TAG_LOGD(AAFwkTag::RECOVERY, "ANI_Constructor finish");
    return ANI_OK;
}
