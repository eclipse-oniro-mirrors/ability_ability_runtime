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

#include "mission_manager.h"

#include "ability_manager_client.h"
#include "event_handler.h"
#include "event_runner.h"
#include "hilog_tag_wrapper.h"
#include "js_error_utils.h"
#include "js_mission_info_utils.h"
#include "js_mission_listener.h"
#include "js_runtime_utils.h"
#include "mission_snapshot.h"
#include "napi_common_start_options.h"
#include "napi_common_util.h"
#include "permission_constants.h"
#ifdef SUPPORT_SCREEN
#include "pixel_map_napi.h"
#endif
#include "start_options.h"

#include <mutex>

namespace OHOS {
namespace AbilityRuntime {
using namespace OHOS::AppExecFwk;
using AbilityManagerClient = AAFwk::AbilityManagerClient;
namespace {
constexpr size_t ARGC_ONE = 1;
constexpr int32_t ARG_COUNT_TWO = 2;
}
class JsMissionManager {
public:
    JsMissionManager() = default;
    ~JsMissionManager() = default;

    static void Finalizer(napi_env env, void* data, void* hint)
    {
        TAG_LOGI(AAFwkTag::MISSION, "called");
        std::unique_ptr<JsMissionManager>(static_cast<JsMissionManager*>(data));
    }

    static napi_value RegisterMissionListener(napi_env env, napi_callback_info info)
    {
        GET_CB_INFO_AND_CALL(env, info, JsMissionManager, OnRegisterMissionListener);
    }

    static napi_value UnregisterMissionListener(napi_env env, napi_callback_info info)
    {
        GET_CB_INFO_AND_CALL(env, info, JsMissionManager, OnUnregisterMissionListener);
    }

    static napi_value GetMissionInfos(napi_env env, napi_callback_info info)
    {
        GET_CB_INFO_AND_CALL(env, info, JsMissionManager, OnGetMissionInfos);
    }

    static napi_value GetMissionInfo(napi_env env, napi_callback_info info)
    {
        GET_CB_INFO_AND_CALL(env, info, JsMissionManager, OnGetMissionInfo);
    }

    static napi_value GetMissionSnapShot(napi_env env, napi_callback_info info)
    {
        GET_CB_INFO_AND_CALL(env, info, JsMissionManager, OnGetMissionSnapShot);
    }

    static napi_value GetLowResolutionMissionSnapShot(napi_env env, napi_callback_info info)
    {
        GET_CB_INFO_AND_CALL(env, info, JsMissionManager, OnGetLowResolutionMissionSnapShot);
    }

    static napi_value LockMission(napi_env env, napi_callback_info info)
    {
        GET_CB_INFO_AND_CALL(env, info, JsMissionManager, OnLockMission);
    }

    static napi_value UnlockMission(napi_env env, napi_callback_info info)
    {
        GET_CB_INFO_AND_CALL(env, info, JsMissionManager, OnUnlockMission);
    }

    static napi_value ClearMission(napi_env env, napi_callback_info info)
    {
        GET_CB_INFO_AND_CALL(env, info, JsMissionManager, OnClearMission);
    }

    static napi_value ClearAllMissions(napi_env env, napi_callback_info info)
    {
        GET_CB_INFO_AND_CALL(env, info, JsMissionManager, OnClearAllMissions);
    }

    static napi_value MoveMissionToFront(napi_env env, napi_callback_info info)
    {
        GET_CB_INFO_AND_CALL(env, info, JsMissionManager, OnMoveMissionToFront);
    }

private:
    napi_value OnRegisterMissionListener(napi_env env, size_t argc, napi_value* argv)
    {
        TAG_LOGI(AAFwkTag::MISSION, "called");
        if (argc < 1) {
            TAG_LOGE(AAFwkTag::MISSION, "Params not match");
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }

        missionListenerId_++;
        if (missionListener_ != nullptr) {
            missionListener_->AddJsListenerObject(missionListenerId_, argv[0]);
            return CreateJsValue(env, missionListenerId_);
        }

        missionListener_ = sptr<JsMissionListener>(new (std::nothrow) JsMissionListener(env));
        if (missionListener_ == nullptr) {
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
            return CreateJsUndefined(env);
        }
        auto ret = AbilityManagerClient::GetInstance()->RegisterMissionListener(missionListener_);
        if (ret == 0) {
            missionListener_->AddJsListenerObject(missionListenerId_, argv[0]);
            return CreateJsValue(env, missionListenerId_);
        } else {
            TAG_LOGE(AAFwkTag::MISSION, "failed %{public}d", ret);
            missionListener_ = nullptr;
            if (ret == CHECK_PERMISSION_FAILED) {
                ThrowNoPermissionError(env, PermissionConstants::PERMISSION_MANAGE_MISSION);
            } else {
                ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
            }
            return CreateJsUndefined(env);
        }
    }

    napi_value OnUnregisterMissionListener(napi_env env, size_t argc, napi_value* argv)
    {
        TAG_LOGI(AAFwkTag::MISSION, "called");
        HandleEscape handleEscape(env);
        if (argc < 1) {
            TAG_LOGE(AAFwkTag::MISSION, "invalid argc");
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }

        int32_t missionListenerId = -1;
        if (!ConvertFromJsValue(env, argv[0], missionListenerId)) {
            TAG_LOGE(AAFwkTag::MISSION, "Parse missionListenerId failed");
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
            return CreateJsUndefined(env);
        }

        NapiAsyncTask::CompleteCallback complete =
            [&missionListener = missionListener_, missionListenerId]
            (napi_env env, NapiAsyncTask &task, int32_t status) {
                HandleScope handleScope(env);
                if (!missionListener || !missionListener->RemoveJsListenerObject(missionListenerId)) {
                    task.Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_NO_MISSION_LISTENER));
                    return;
                }

                if (!missionListener->IsEmpty()) {
                    task.Resolve(env, CreateJsUndefined(env));
                    return;
                }
                auto ret = AbilityManagerClient::GetInstance()->UnRegisterMissionListener(missionListener);
                if (ret == 0) {
                    task.Resolve(env, CreateJsUndefined(env));
                    missionListener = nullptr;
                } else {
                    task.Reject(env,
                        CreateJsErrorByNativeErr(env, ret, PermissionConstants::PERMISSION_MANAGE_MISSION));
                }
            };

        napi_value lastParam = (argc <= 1) ? nullptr : argv[1];
        napi_value result = nullptr;
        NapiAsyncTask::Schedule("MissionManager::OnUnregisterMissionListener",
            env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
        return handleEscape.Escape(result);
    }

    napi_value OnGetMissionInfos(napi_env env, size_t argc, napi_value* argv)
    {
        TAG_LOGI(AAFwkTag::MISSION, "called");
        HandleEscape handleEscape(env);
        if (argc < ARG_COUNT_TWO) { // at least 2 parameters.
            TAG_LOGE(AAFwkTag::MISSION, "invalid argc");
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }
        std::string deviceId;
        if (!ConvertFromJsValue(env, argv[0], deviceId)) {
            TAG_LOGE(AAFwkTag::MISSION, "Parse deviceId failed");
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
            return CreateJsUndefined(env);
        }
        int numMax = -1;
        if (!ConvertFromJsValue(env, argv[1], numMax)) {
            TAG_LOGE(AAFwkTag::MISSION, "Parse numMax failed");
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
            return CreateJsUndefined(env);
        }

        NapiAsyncTask::CompleteCallback complete =
            [deviceId, numMax](napi_env env, NapiAsyncTask &task, int32_t status) {
                HandleScope handleScope(env);
                std::vector<AAFwk::MissionInfo> missionInfos;
                auto ret = AbilityManagerClient::GetInstance()->GetMissionInfos(deviceId, numMax, missionInfos);
                if (ret == 0) {
                    task.Resolve(env, CreateJsMissionInfoArray(env, missionInfos));
                } else {
                    task.Reject(env,
                        CreateJsErrorByNativeErr(env, ret, PermissionConstants::PERMISSION_MANAGE_MISSION));
                }
            };

        napi_value lastParam = (argc <= 2) ? nullptr : argv[2];
        napi_value result = nullptr;
        NapiAsyncTask::Schedule("MissionManager::OnGetMissionInfos",
            env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
        return handleEscape.Escape(result);
    }

    napi_value OnGetMissionInfo(napi_env env, size_t argc, napi_value* argv)
    {
        TAG_LOGI(AAFwkTag::MISSION, "called");
        HandleEscape handleEscape(env);
        if (argc < ARG_COUNT_TWO) {
            TAG_LOGE(AAFwkTag::MISSION, "invalid argc");
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }
        std::string deviceId;
        if (!ConvertFromJsValue(env, argv[0], deviceId)) {
            TAG_LOGE(AAFwkTag::MISSION, "Parse deviceId failed");
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
            return CreateJsUndefined(env);
        }
        int32_t missionId = -1;
        if (!ConvertFromJsValue(env, argv[1], missionId)) {
            TAG_LOGE(AAFwkTag::MISSION, "Parse missionId failed");
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
            return CreateJsUndefined(env);
        }

        NapiAsyncTask::CompleteCallback complete =
            [deviceId, missionId](napi_env env, NapiAsyncTask &task, int32_t status) {
                HandleScope handleScope(env);
                AAFwk::MissionInfo missionInfo;
                auto ret = AbilityManagerClient::GetInstance()->GetMissionInfo(deviceId, missionId, missionInfo);
                if (ret == 0) {
                    task.Resolve(env, CreateJsMissionInfo(env, missionInfo));
                } else {
                    task.Reject(env,
                        CreateJsErrorByNativeErr(env, ret, PermissionConstants::PERMISSION_MANAGE_MISSION));
                }
            };

        napi_value lastParam = (argc <= 2) ? nullptr : argv[2];
        napi_value result = nullptr;
        NapiAsyncTask::Schedule("MissionManager::OnGetMissionInfo",
            env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
        return handleEscape.Escape(result);
    }

    napi_value OnGetMissionSnapShot(napi_env env, size_t argc, napi_value* argv)
    {
        TAG_LOGI(AAFwkTag::MISSION, "called");
        return GetMissionSnapShot(env, argc, argv, false);
    }

    napi_value OnGetLowResolutionMissionSnapShot(napi_env env, size_t argc, napi_value* argv)
    {
        TAG_LOGI(AAFwkTag::MISSION, "called");
        return GetMissionSnapShot(env, argc, argv, true);
    }

    napi_value GetMissionSnapShot(napi_env env, size_t argc, napi_value* argv, bool isLowResolution)
    {
        HandleEscape handleEscape(env);
        std::string deviceId;
        int32_t missionId = -1;
        if (!CheckMissionSnapShotParams(env, argc, argv, deviceId, missionId)) {
            return CreateJsUndefined(env);
        }

        class MissionSnapshotWrap {
        public:
            int result = -1;
            AAFwk::MissionSnapshot missionSnapshot;
        };

        std::shared_ptr<MissionSnapshotWrap> snapshotWrap = std::make_shared<MissionSnapshotWrap>();
        auto excute = [deviceId, missionId, isLowResolution, snapshotWrap]() {
            snapshotWrap->result = AbilityManagerClient::GetInstance()->GetMissionSnapshot(
                deviceId, missionId, snapshotWrap->missionSnapshot, isLowResolution);
        };

        auto complete = [snapshotWrap](napi_env env, NapiAsyncTask &task, int32_t status) {
            HandleScope handleScope(env);
            if (snapshotWrap->result == 0) {
                napi_value object = nullptr;
                napi_create_object(env, &object);
                napi_value abilityObj = nullptr;
                napi_create_object(env, &abilityObj);
                napi_set_named_property(env, abilityObj, "bundleName",
                    CreateJsValue(env, snapshotWrap->missionSnapshot.topAbility.GetBundleName()));
                napi_set_named_property(env, abilityObj, "abilityName",
                    CreateJsValue(env, snapshotWrap->missionSnapshot.topAbility.GetAbilityName()));
                napi_set_named_property(env, object, "ability", abilityObj);
#ifdef SUPPORT_SCREEN
                auto snapshotValue = Media::PixelMapNapi::CreatePixelMap(
                    env, snapshotWrap->missionSnapshot.snapshot);
                napi_set_named_property(env, object, "snapshot", snapshotValue);
#endif
                task.Resolve(env, object);
            } else {
                task.Reject(env, CreateJsErrorByNativeErr(env, snapshotWrap->result,
                    PermissionConstants::PERMISSION_MANAGE_MISSION));
            }
        };
        napi_value lastParam = (argc > ARG_COUNT_TWO) ? argv[ARG_COUNT_TWO] : nullptr;
        napi_value result = nullptr;
        NapiAsyncTask::Schedule("MissionManager::GetMissionSnapShot",
            env, CreateAsyncTaskWithLastParam(env, lastParam, std::move(excute), std::move(complete), &result));
        return handleEscape.Escape(result);
    }

    bool CheckMissionSnapShotParams(napi_env env, size_t argc, napi_value* argv,
        std::string &deviceId, int32_t &missionId)
    {
        if (argc < ARG_COUNT_TWO) {
            ThrowTooFewParametersError(env);
            return false;
        }

        if (!ConvertFromJsValue(env, argv[0], deviceId)) {
            TAG_LOGE(AAFwkTag::MISSION, "parse deviceId failed");
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
            return false;
        }

        if (!ConvertFromJsValue(env, argv[1], missionId)) {
            TAG_LOGE(AAFwkTag::MISSION, "parse missionId failed");
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
            return false;
        }

        return true;
    }

    napi_value OnLockMission(napi_env env, size_t argc, napi_value* argv)
    {
        TAG_LOGI(AAFwkTag::MISSION, "called");
        HandleEscape handleEscape(env);
        if (argc == 0) {
            TAG_LOGE(AAFwkTag::MISSION, "invalid argc");
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }
        int32_t missionId = -1;
        if (!ConvertFromJsValue(env, argv[0], missionId)) {
            TAG_LOGE(AAFwkTag::MISSION, "Parse missionId failed");
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
            return CreateJsUndefined(env);
        }

        NapiAsyncTask::CompleteCallback complete =
            [missionId](napi_env env, NapiAsyncTask &task, int32_t status) {
                HandleScope handleScope(env);
                auto ret = AbilityManagerClient::GetInstance()->LockMissionForCleanup(missionId);
                if (ret == 0) {
                    task.Resolve(env, CreateJsUndefined(env));
                } else {
                    task.Reject(env,
                        CreateJsErrorByNativeErr(env, ret, PermissionConstants::PERMISSION_MANAGE_MISSION));
                }
            };

        napi_value lastParam = (argc > 1) ?  argv[1] : nullptr;
        napi_value result = nullptr;
        NapiAsyncTask::ScheduleHighQos("MissionManager::OnLockMission",
            env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
        return handleEscape.Escape(result);
    }

    napi_value OnUnlockMission(napi_env env, size_t argc, napi_value* argv)
    {
        TAG_LOGI(AAFwkTag::MISSION, "called");
        HandleEscape handleEscape(env);
        if (argc == 0) {
            TAG_LOGE(AAFwkTag::MISSION, "invalid argc");
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }
        int32_t missionId = -1;
        if (!ConvertFromJsValue(env, argv[0], missionId)) {
            TAG_LOGE(AAFwkTag::MISSION, "Parse missionId failed");
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
            return CreateJsUndefined(env);
        }

        NapiAsyncTask::CompleteCallback complete =
            [missionId](napi_env env, NapiAsyncTask &task, int32_t status) {
                HandleScope handleScope(env);
                auto ret = AbilityManagerClient::GetInstance()->UnlockMissionForCleanup(missionId);
                if (ret == 0) {
                    task.Resolve(env, CreateJsUndefined(env));
                } else {
                    task.Reject(env,
                        CreateJsErrorByNativeErr(env, ret, PermissionConstants::PERMISSION_MANAGE_MISSION));
                }
            };

        napi_value lastParam = (argc > 1) ? argv[1] : nullptr;
        napi_value result = nullptr;
        NapiAsyncTask::ScheduleHighQos("MissionManager::OnUnlockMission",
            env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
        return handleEscape.Escape(result);
    }

    napi_value OnClearMission(napi_env env, size_t argc, napi_value* argv)
    {
        TAG_LOGI(AAFwkTag::MISSION, "called");
        HandleEscape handleEscape(env);
        if (argc == 0) {
            TAG_LOGE(AAFwkTag::MISSION, "invalid argc");
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }
        int32_t missionId = -1;
        if (!ConvertFromJsValue(env, argv[0], missionId)) {
            TAG_LOGE(AAFwkTag::MISSION, "Parse missionId failed");
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
            return CreateJsUndefined(env);
        }

        NapiAsyncTask::CompleteCallback complete =
            [missionId](napi_env env, NapiAsyncTask &task, int32_t status) {
                HandleScope handleScope(env);
                auto ret = AbilityManagerClient::GetInstance()->CleanMission(missionId);
                if (ret == 0) {
                    task.Resolve(env, CreateJsUndefined(env));
                } else {
                    task.Reject(env,
                        CreateJsErrorByNativeErr(env, ret, PermissionConstants::PERMISSION_MANAGE_MISSION));
                }
            };

        napi_value lastParam = (argc > 1) ? argv[1] : nullptr;
        napi_value result = nullptr;
        NapiAsyncTask::ScheduleHighQos("MissionManager::OnClearMission",
            env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
        return handleEscape.Escape(result);
    }

    napi_value OnClearAllMissions(napi_env env, size_t argc, napi_value* argv)
    {
        TAG_LOGI(AAFwkTag::MISSION, "called");
        HandleEscape handleEscape(env);
        NapiAsyncTask::CompleteCallback complete =
            [](napi_env env, NapiAsyncTask &task, int32_t status) {
                HandleScope handleScope(env);
                auto ret = AbilityManagerClient::GetInstance()->CleanAllMissions();
                if (ret == 0) {
                    task.Resolve(env, CreateJsUndefined(env));
                } else {
                    task.Reject(env,
                        CreateJsErrorByNativeErr(env, ret, PermissionConstants::PERMISSION_MANAGE_MISSION));
                }
            };

        napi_value lastParam = (argc > 0) ? argv[0] : nullptr;
        napi_value result = nullptr;
        NapiAsyncTask::ScheduleHighQos("MissionManager::OnMoveMissionToFront",
            env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
        return handleEscape.Escape(result);
    }

    napi_value OnMoveMissionToFront(napi_env env, size_t argc, napi_value* argv)
    {
        TAG_LOGI(AAFwkTag::MISSION, "called");
        HandleEscape handleEscape(env);
        if (argc == 0) {
            TAG_LOGE(AAFwkTag::MISSION, "invalid argc");
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }
        int32_t missionId = -1;
        if (!ConvertFromJsValue(env, argv[0], missionId)) {
            TAG_LOGE(AAFwkTag::MISSION, "Parse missionId failed");
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
            return CreateJsUndefined(env);
        }
        decltype(argc) unwrapArgc = 1;

        AAFwk::StartOptions startOptions;
        if (argc > ARGC_ONE && AppExecFwk::IsTypeForNapiValue(env, argv[1], napi_object)) {
            TAG_LOGI(AAFwkTag::MISSION, "start options used");
            AppExecFwk::UnwrapStartOptions(env, argv[1], startOptions);
            unwrapArgc++;
        }
        NapiAsyncTask::CompleteCallback complete =
            [missionId, startOptions, unwrapArgc](napi_env env, NapiAsyncTask &task, int32_t status) {
                HandleScope handleScope(env);
                auto ret = (unwrapArgc == 1) ? AbilityManagerClient::GetInstance()->MoveMissionToFront(missionId) :
                    AbilityManagerClient::GetInstance()->MoveMissionToFront(missionId, startOptions);
                if (ret == 0) {
                    task.Resolve(env, CreateJsUndefined(env));
                } else {
                    task.Reject(env,
                        CreateJsErrorByNativeErr(env, ret, PermissionConstants::PERMISSION_MANAGE_MISSION));
                }
            };

        napi_value lastParam = (argc > unwrapArgc) ? argv[unwrapArgc] : nullptr;
        napi_value result = nullptr;
        NapiAsyncTask::ScheduleHighQos("MissionManager::OnMoveMissionToFront",
            env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
        return handleEscape.Escape(result);
    }

    sptr<JsMissionListener> missionListener_ = nullptr;
    uint32_t missionListenerId_ = 0;
};

napi_value JsMissionManagerInit(napi_env env, napi_value exportObj)
{
    TAG_LOGI(AAFwkTag::MISSION, "called");
    if (env == nullptr || exportObj == nullptr) {
        TAG_LOGI(AAFwkTag::MISSION, "Invalid input param");
        return nullptr;
    }

    std::unique_ptr<JsMissionManager> jsMissionManager = std::make_unique<JsMissionManager>();
    napi_wrap(env, exportObj, jsMissionManager.release(), JsMissionManager::Finalizer, nullptr, nullptr);

    const char *moduleName = "JsMissionManager";
    BindNativeFunction(env, exportObj, "registerMissionListener",
        moduleName, JsMissionManager::RegisterMissionListener);
    BindNativeFunction(env, exportObj, "unregisterMissionListener",
        moduleName, JsMissionManager::UnregisterMissionListener);
    BindNativeFunction(env, exportObj, "getMissionInfos", moduleName, JsMissionManager::GetMissionInfos);
    BindNativeFunction(env, exportObj, "getMissionInfo", moduleName, JsMissionManager::GetMissionInfo);
    BindNativeFunction(env, exportObj, "getMissionSnapShot", moduleName, JsMissionManager::GetMissionSnapShot);
    BindNativeFunction(env, exportObj, "getLowResolutionMissionSnapShot", moduleName,
        JsMissionManager::GetLowResolutionMissionSnapShot);
    BindNativeFunction(env, exportObj, "lockMission", moduleName, JsMissionManager::LockMission);
    BindNativeFunction(env, exportObj, "unlockMission", moduleName, JsMissionManager::UnlockMission);
    BindNativeFunction(env, exportObj, "clearMission", moduleName, JsMissionManager::ClearMission);
    BindNativeFunction(env, exportObj, "clearAllMissions", moduleName, JsMissionManager::ClearAllMissions);
    BindNativeFunction(env, exportObj, "moveMissionToFront", moduleName, JsMissionManager::MoveMissionToFront);
    return CreateJsUndefined(env);
}
}  // namespace AbilityRuntime
}  // namespace OHOS
