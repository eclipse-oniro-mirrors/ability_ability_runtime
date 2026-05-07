/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include "app_recovery_api.h"

#include "app_recovery.h"
#include "hilog_tag_wrapper.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"

#include "napi/native_api.h"
#include "napi/native_common.h"
#include "napi/native_node_api.h"

#include "napi_common_want.h"
#include "recovery_param.h"
#include "want.h"
#include "ability_manager_client.h"
#include "exit_reason.h"

namespace OHOS {
namespace AbilityRuntime {
using namespace OHOS::AppExecFwk;
namespace {
constexpr int RESTART_APP_ID = 3;
constexpr const char *const  RESTART_APP_REASON = "RestartApp";
constexpr const char *const  RESTART_REASON = "Restart";
enum RestartErrno {
    NO_ERROR,
    SAVED_STATE_NOT_EXIST,
    LAST_RESTART_ENCOUNTER_ERROR,
};
class AppRecoveryApiRegistry {
public:
    AppRecoveryApiRegistry() = default;
    ~AppRecoveryApiRegistry() = default;

    static void Finalizer(napi_env env, void *data, void *hint)
    {
        std::unique_ptr<AppRecoveryApiRegistry>(static_cast<AppRecoveryApiRegistry*>(data));
    }

    static napi_value EnableAppRecovery(napi_env env, napi_callback_info info)
    {
        GET_CB_INFO_AND_CALL(env, info, AppRecoveryApiRegistry, OnEnableAppRecovery);
    }

    static napi_value RestartApp(napi_env env, napi_callback_info info)
    {
        GET_CB_INFO_AND_CALL(env, info, AppRecoveryApiRegistry, OnRestartApp);
    }

    static napi_value SaveAppState(napi_env env, napi_callback_info info)
    {
        GET_CB_INFO_AND_CALL(env, info, AppRecoveryApiRegistry, OnSaveAppState);
    }

    static napi_value SetRestartWant(napi_env env, napi_callback_info info)
    {
        GET_CB_INFO_AND_CALL(env, info, AppRecoveryApiRegistry, OnSetRestartWant);
    }

private:
    napi_value OnEnableAppRecovery(napi_env env, const size_t argc, napi_value* argv)
    {
        size_t parameterCount = argc;
        napi_value result = CreateJsUndefined(env);
        constexpr int maxCount = 3;
        if (parameterCount > maxCount) {
            return result;
        }

        uint16_t flags[] = {
            RestartFlag::ALWAYS_RESTART,
            SaveOccasionFlag::SAVE_WHEN_ERROR,
            SaveModeFlag::SAVE_WITH_FILE
        };

        for (size_t i = 0; i < parameterCount; ++i) {
            napi_valuetype paramType;
            napi_typeof(env, argv[i], &paramType);
            if (paramType != napi_number) {
                TAG_LOGE(
                    AAFwkTag::RECOVERY, "argv[%{public}s] not number", std::to_string(i).c_str());
                return result;
            }
            int32_t tmp = 0;
            napi_get_value_int32(env, argv[i], &tmp);
            flags[i] = static_cast<uint16_t>(tmp);
        }

        if (!CheckParamsValid(flags)) {
            return result;
        }

        AppRecovery::GetInstance().EnableAppRecovery(flags[0],  // 0:RestartFlag
                                                     flags[1],  // 1:SaveOccasionFlag
                                                     flags[2]); // 2:SaveModeFlag
        return result;
    }

    bool CheckParamsValid(const uint16_t params[])
    {
        uint16_t restartFlag = params[0];
        constexpr uint16_t restartMaxVal = 0x0004;
        if ((restartFlag < 0 || restartFlag > restartMaxVal) && (restartFlag != RestartFlag::NO_RESTART)) {
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

    napi_value OnSaveAppState(napi_env env, const size_t argc, napi_value* argv)
    {
        if (argc > 1) {
            TAG_LOGE(AAFwkTag::RECOVERY, "invalid argc");
            return CreateJsValue(env, false);
        }
        uintptr_t ability = 0;
        if (argc == 1) {
            napi_value value = argv[0];
            if (value == nullptr) {
                TAG_LOGE(AAFwkTag::RECOVERY, "null abilityContext");
                return CreateJsValue(env, false);
            }
            void* result = nullptr;
            napi_unwrap(env, value, &result);
            ability = reinterpret_cast<uintptr_t>(result);
        }
        if (AppRecovery::GetInstance().ScheduleSaveAppState(StateReason::DEVELOPER_REQUEST, ability)) {
            return CreateJsValue(env, true);
        }
        return CreateJsValue(env, false);
    }

    napi_value OnRestartApp(napi_env env, const size_t argc, napi_value* argv)
    {
        if (argc != 0) {
            TAG_LOGE(AAFwkTag::RECOVERY, "invalid argc");
            return CreateJsUndefined(env);
        }

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
        return CreateJsUndefined(env);
    }

    napi_value OnSetRestartWant(napi_env env, const size_t argc, napi_value* argv)
    {
        if (argc != 1) {
            TAG_LOGE(AAFwkTag::RECOVERY, "invalid argc");
            return CreateJsUndefined(env);
        }
        std::shared_ptr<AAFwk::Want> want = std::make_shared<AAFwk::Want>();
        OHOS::AppExecFwk::UnwrapWant(env, argv[0], *(want.get()));
        AppRecovery::GetInstance().SetRestartWant(want);
        return CreateJsUndefined(env);
    }
};
} // namespace

napi_value AppRecoveryRestartFlagInit(napi_env env)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::RECOVERY, "null env");
        return nullptr;
    }

    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);

    if (objValue == nullptr) {
        TAG_LOGE(AAFwkTag::RECOVERY, "null obj");
        return nullptr;
    }

    napi_set_named_property(env, objValue, "ALWAYS_RESTART", CreateJsValue(env, RestartFlag::ALWAYS_RESTART));
    napi_set_named_property(env, objValue, "RESTART_WHEN_JS_CRASH",
        CreateJsValue(env, RestartFlag::RESTART_WHEN_JS_CRASH));
    napi_set_named_property(env, objValue, "RESTART_WHEN_APP_FREEZE",
        CreateJsValue(env, RestartFlag::RESTART_WHEN_APP_FREEZE));
    napi_set_named_property(env, objValue, "RESTART_WHEN_CPP_CRASH",
        CreateJsValue(env, RestartFlag::RESTART_WHEN_CPP_CRASH));
    napi_set_named_property(env, objValue, "NO_RESTART", CreateJsValue(env, RestartFlag::NO_RESTART));
    return objValue;
}

napi_value AppRecoveryStateSaveFlagInit(napi_env env)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::RECOVERY, "null env");
        return nullptr;
    }

    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);

    if (objValue == nullptr) {
        TAG_LOGE(AAFwkTag::RECOVERY, "null obj");
        return nullptr;
    }

    napi_set_named_property(env, objValue, "NONE", CreateJsValue(env, SaveOccasionFlag::NO_SAVE));
    napi_set_named_property(env, objValue, "SAVE_WHEN_ERROR",
        CreateJsValue(env, SaveOccasionFlag::SAVE_WHEN_ERROR));
    napi_set_named_property(env, objValue, "SAVE_WHEN_BACKGROUND",
        CreateJsValue(env, SaveOccasionFlag::SAVE_WHEN_BACKGROUND));
    return objValue;
}

napi_value AppRecoverySaveModeFlagInit(napi_env env)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::RECOVERY, "null env");
        return nullptr;
    }

    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);

    if (objValue == nullptr) {
        TAG_LOGE(AAFwkTag::RECOVERY, "null obj");
        return nullptr;
    }
    napi_set_named_property(env, objValue, "SAVE_WITH_FILE",
        CreateJsValue(env, SaveModeFlag::SAVE_WITH_FILE));
    napi_set_named_property(env, objValue, "SAVE_WITH_SHARED_MEMORY",
        CreateJsValue(env, SaveModeFlag::SAVE_WITH_SHARED_MEMORY));
    return objValue;
}

napi_value InitAppRecoveryApiModule(napi_env env, napi_value exportObj)
{
    if (env == nullptr || exportObj == nullptr) {
        TAG_LOGE(AAFwkTag::RECOVERY, "null env or exportObj");
        return nullptr;
    }

    std::unique_ptr<AppRecoveryApiRegistry> appRecoveryApi = std::make_unique<AppRecoveryApiRegistry>();
    napi_wrap(env, exportObj, appRecoveryApi.release(), AppRecoveryApiRegistry::Finalizer, nullptr, nullptr);

    const char *moduleName = "AppRecovery";
    BindNativeFunction(env, exportObj, "enableAppRecovery", moduleName, AppRecoveryApiRegistry::EnableAppRecovery);
    BindNativeFunction(env, exportObj, "restartApp", moduleName, AppRecoveryApiRegistry::RestartApp);
    BindNativeFunction(env, exportObj, "saveAppState", moduleName, AppRecoveryApiRegistry::SaveAppState);
    BindNativeFunction(env, exportObj, "setRestartWant", moduleName, AppRecoveryApiRegistry::SetRestartWant);

    napi_set_named_property(env, exportObj, "RestartFlag", AppRecoveryRestartFlagInit(env));
    napi_set_named_property(env, exportObj, "SaveOccasionFlag", AppRecoveryStateSaveFlagInit(env));
    napi_set_named_property(env, exportObj, "SaveModeFlag", AppRecoverySaveModeFlagInit(env));

    return CreateJsUndefined(env);
}
}  // namespace AbilityRuntime
}  // namespace OHOS