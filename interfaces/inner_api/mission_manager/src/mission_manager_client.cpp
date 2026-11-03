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

#include "ability_manager_errors.h"
#include "extension_ability_info.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "iservice_registry.h"
#include "mission_manager_client.h"
#include "mission_manager_proxy.h"
#include "scene_board_judgement.h"
#include "system_ability_definition.h"
#include "want.h"

namespace OHOS {
namespace AAFwk {
namespace {
#ifdef SUPPORT_SCREEN
static std::unordered_map<Rosen::WSError, int32_t> SCB_TO_MISSION_ERROR_CODE_MAP {
    { Rosen::WSError::WS_ERROR_INVALID_PERMISSION, CHECK_PERMISSION_FAILED },
    { Rosen::WSError::WS_ERROR_NOT_SYSTEM_APP, ERR_NOT_SYSTEM_APP },
    { Rosen::WSError::WS_ERROR_INVALID_PARAM, INVALID_PARAMETERS_ERR },
};
#endif // SUPPORT_SCREEN
}
#ifdef SUPPORT_SCREEN
using OHOS::Rosen::SessionManagerLite;
#endif // SUPPORT_SCREEN

#define CHECK_POINTER_RETURN(object)                        \
    if (!object) {                                          \
        TAG_LOGE(AAFwkTag::MISSION, "null proxy"); \
        return;                                             \
    }

#define CHECK_POINTER_RETURN_NOT_CONNECTED(object)           \
    if (!object) {                                           \
        TAG_LOGE(AAFwkTag::MISSION, "null proxy"); \
        return ABILITY_SERVICE_NOT_CONNECTED;                \
    }

#define CHECK_POINTER_RETURN_INVALID_VALUE(object)           \
    if (!object) {                                           \
        TAG_LOGE(AAFwkTag::MISSION, "null proxy"); \
        return ERR_INVALID_VALUE;                            \
    }

MissionManagerClient& MissionManagerClient::GetInstance()
{
    static MissionManagerClient instance;
    return instance;
}

sptr<IMissionManager> MissionManagerClient::GetMissionManager()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (proxy_ == nullptr) {
        Connect();
    }

    return proxy_;
}

void MissionManagerClient::Connect()
{
    auto systemManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemManager == nullptr) {
        TAG_LOGE(AAFwkTag::MISSION, "Get SAMgr failed");
        return;
    }
    auto remoteObj = systemManager->GetSystemAbility(ABILITY_MGR_SERVICE_ID);
    if (remoteObj == nullptr) {
        TAG_LOGE(AAFwkTag::MISSION, "Connect AMS failed");
        return;
    }

    deathRecipient_ = new MissionMgrDeathRecipient();
    if (remoteObj->IsProxyObject() && !remoteObj->AddDeathRecipient(deathRecipient_)) {
        TAG_LOGE(AAFwkTag::MISSION, "AddDeathRecipient failed");
        return;
    }

    proxy_ = sptr<IMissionManager>(new MissionManagerProxy(remoteObj));
    TAG_LOGD(AAFwkTag::MISSION, "Connect AMS success");
}

void MissionManagerClient::ResetProxy(const wptr<IRemoteObject> &remote)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (proxy_ == nullptr) {
        TAG_LOGI(AAFwkTag::MISSION, "null proxy_, no need reset");
        return;
    }

    auto serviceRemote = proxy_->AsObject();
    if (serviceRemote != nullptr && serviceRemote == remote.promote()) {
        serviceRemote->RemoveDeathRecipient(deathRecipient_);
        proxy_ = nullptr;
    }
}

void MissionManagerClient::MissionMgrDeathRecipient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    TAG_LOGI(AAFwkTag::MISSION, "called");
    MissionManagerClient::GetInstance().ResetProxy(remote);
}

ErrCode MissionManagerClient::Release()
{
    TAG_LOGI(AAFwkTag::MISSION, "Release");
    return RemoveDeathRecipient();
}

ErrCode MissionManagerClient::RemoveDeathRecipient()
{
    TAG_LOGI(AAFwkTag::MISSION, "RemoveDeathRecipient");
    std::lock_guard<std::mutex> lock(mutex_);
    if (proxy_ == nullptr) {
        TAG_LOGW(AAFwkTag::MISSION, "null proxy_");
        return ERR_NULL_OBJECT;
    }
    if (deathRecipient_ == nullptr) {
        TAG_LOGW(AAFwkTag::MISSION, "null deathRecipient_");
        return ERR_NULL_OBJECT;
    }
    auto serviceRemote = proxy_->AsObject();
    if (serviceRemote == nullptr) {
        TAG_LOGW(AAFwkTag::MISSION, "null serviceRemote");
        return ERR_NULL_OBJECT;
    }
    bool ret = serviceRemote->RemoveDeathRecipient(deathRecipient_);
    if (!ret) {
        TAG_LOGW(AAFwkTag::MISSION, "RemoveDeathRecipient fail");
        return ERR_INVALID_VALUE;
    }
    proxy_ = nullptr;
    deathRecipient_ = nullptr;
    TAG_LOGI(AAFwkTag::MISSION, "RemoveDeathRecipient success");
    return ERR_OK;
}


ErrCode MissionManagerClient::ContinueMission(const std::string &srcDeviceId, const std::string &dstDeviceId,
    int32_t missionId, sptr<IRemoteObject> callback, AAFwk::WantParams &wantParams)
{
    if (srcDeviceId.empty() || dstDeviceId.empty() || callback == nullptr) {
        TAG_LOGE(AAFwkTag::MISSION, "srcDeviceId or dstDeviceId or callback null");
        return ERR_INVALID_VALUE;
    }

    auto abms = GetMissionManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    int result = abms->ContinueMission(srcDeviceId, dstDeviceId, missionId, callback, wantParams);
    return result;
}


ErrCode MissionManagerClient::ContinueMission(AAFwk::ContinueMissionInfo continueMissionInfo,
    const sptr<IRemoteObject> &callback)

{
    if (continueMissionInfo.srcDeviceId.empty() || continueMissionInfo.dstDeviceId.empty() || callback == nullptr) {
        TAG_LOGE(AAFwkTag::MISSION, "srcDeviceId or dstDeviceId or callback null");
        return ERR_INVALID_VALUE;
    }

    auto abms = GetMissionManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    int result = abms->ContinueMission(continueMissionInfo, callback);
    return result;
}


ErrCode MissionManagerClient::LockMissionForCleanup(int32_t missionId, int32_t userId)
{
#ifdef SUPPORT_SCREEN
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        auto sceneSessionManager = SessionManagerLite::GetInstance(userId).GetSceneSessionManagerLiteProxy();
        CHECK_POINTER_RETURN_INVALID_VALUE(sceneSessionManager);
        TAG_LOGI(AAFwkTag::MISSION, "scb call, LockMissionForCleanup");
        auto err = sceneSessionManager->LockSession(missionId);
        if (SCB_TO_MISSION_ERROR_CODE_MAP.count(err)) {
            TAG_LOGE(AAFwkTag::MISSION, "scb call, LockMissionForCleanup err");
            return SCB_TO_MISSION_ERROR_CODE_MAP[err];
        }
        return static_cast<int>(err);
    }
#endif //SUPPORT_SCREEN
    auto abms = GetMissionManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->LockMissionForCleanup(missionId);
}


ErrCode MissionManagerClient::UnlockMissionForCleanup(int32_t missionId, int32_t userId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
#ifdef SUPPORT_SCREEN
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        auto sceneSessionManager = SessionManagerLite::GetInstance(userId).GetSceneSessionManagerLiteProxy();
        CHECK_POINTER_RETURN_INVALID_VALUE(sceneSessionManager);
        TAG_LOGI(AAFwkTag::MISSION, "scb call, UnlockMissionForCleanup");
        auto err = sceneSessionManager->UnlockSession(missionId);
        if (SCB_TO_MISSION_ERROR_CODE_MAP.count(err)) {
            TAG_LOGE(AAFwkTag::MISSION, "scb call, UnlockMissionForCleanup err");
            return SCB_TO_MISSION_ERROR_CODE_MAP[err];
        }
        return static_cast<int>(err);
    }
#endif //SUPPORT_SCREEN
    auto abms = GetMissionManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->UnlockMissionForCleanup(missionId);
}


ErrCode MissionManagerClient::RegisterMissionListener(sptr<IMissionListener> listener, int32_t userId)
{
#ifdef SUPPORT_SCREEN
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        auto sceneSessionManager = SessionManagerLite::GetInstance(userId).GetSceneSessionManagerLiteProxy();
        CHECK_POINTER_RETURN_INVALID_VALUE(sceneSessionManager);
        TAG_LOGI(AAFwkTag::MISSION, "scb call, RegisterMissionListener");
        auto err = sceneSessionManager->RegisterSessionListener(listener);
        if (SCB_TO_MISSION_ERROR_CODE_MAP.count(err)) {
            TAG_LOGE(AAFwkTag::MISSION, "scb call, RegisterMissionListener err");
            return SCB_TO_MISSION_ERROR_CODE_MAP[err];
        }
        return static_cast<int>(err);
    }
#endif //SUPPORT_SCREEN
    auto abms = GetMissionManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->RegisterMissionListener(listener);
}


ErrCode MissionManagerClient::UnRegisterMissionListener(sptr<IMissionListener> listener, int32_t userId)
{
#ifdef SUPPORT_SCREEN
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        auto sceneSessionManager = SessionManagerLite::GetInstance(userId).GetSceneSessionManagerLiteProxy();
        CHECK_POINTER_RETURN_INVALID_VALUE(sceneSessionManager);
        TAG_LOGI(AAFwkTag::MISSION, "scb call, UnRegisterMissionListener");
        auto err = sceneSessionManager->UnRegisterSessionListener(listener);
        if (SCB_TO_MISSION_ERROR_CODE_MAP.count(err)) {
            TAG_LOGE(AAFwkTag::MISSION, "scb call, UnRegisterMissionListener err");
            return SCB_TO_MISSION_ERROR_CODE_MAP[err];
        }
        return static_cast<int>(err);
    }
#endif //SUPPORT_SCREEN
    auto abms = GetMissionManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->UnRegisterMissionListener(listener);
}


ErrCode MissionManagerClient::RegisterMissionListener(const std::string &deviceId,
    sptr<IRemoteMissionListener> listener)
{
    auto abms = GetMissionManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->RegisterMissionListener(deviceId, listener);
}

ErrCode MissionManagerClient::UnRegisterMissionListener(const std::string &deviceId,
    sptr<IRemoteMissionListener> listener)
{
    auto abms = GetMissionManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->UnRegisterMissionListener(deviceId, listener);
}


ErrCode MissionManagerClient::GetMissionInfos(const std::string &deviceId, int32_t numMax,
    std::vector<MissionInfo> &missionInfos, int32_t userId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
#ifdef SUPPORT_SCREEN
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        auto sceneSessionManager = SessionManagerLite::GetInstance(userId).GetSceneSessionManagerLiteProxy();
        CHECK_POINTER_RETURN_INVALID_VALUE(sceneSessionManager);
        TAG_LOGI(AAFwkTag::MISSION, "scb call, GetMissionInfos");
        auto err = sceneSessionManager->GetSessionInfos(deviceId, numMax, missionInfos);
        if (SCB_TO_MISSION_ERROR_CODE_MAP.count(err)) {
            TAG_LOGE(AAFwkTag::MISSION, "scb call, GetMissionInfos err");
            return SCB_TO_MISSION_ERROR_CODE_MAP[err];
        }
        return static_cast<int>(err);
    }
#endif //SUPPORT_SCREEN
    auto abms = GetMissionManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->GetMissionInfos(deviceId, numMax, missionInfos);
}

ErrCode MissionManagerClient::GetMissionInfo(const std::string &deviceId, int32_t missionId,
    MissionInfo &missionInfo, int32_t userId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
#ifdef SUPPORT_SCREEN
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        auto sceneSessionManager = SessionManagerLite::GetInstance(userId).GetSceneSessionManagerLiteProxy();
        CHECK_POINTER_RETURN_INVALID_VALUE(sceneSessionManager);
        TAG_LOGI(AAFwkTag::MISSION, "scb call, GetMissionInfo");
        auto err = sceneSessionManager->GetSessionInfo(deviceId, missionId, missionInfo);
        if (SCB_TO_MISSION_ERROR_CODE_MAP.count(err)) {
            TAG_LOGE(AAFwkTag::MISSION, "scb call, GetMissionInfo err");
            return SCB_TO_MISSION_ERROR_CODE_MAP[err];
        }
        return static_cast<int>(err);
    }
#endif //SUPPORT_SCREEN
    auto abms = GetMissionManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->GetMissionInfo(deviceId, missionId, missionInfo);
}


ErrCode MissionManagerClient::GetMissionSnapshot(const std::string &deviceId, int32_t missionId,
    MissionSnapshot &snapshot, bool isLowResolution, int32_t userId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
#ifdef SUPPORT_SCREEN
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        auto sceneSessionManager = SessionManagerLite::GetInstance(userId).GetSceneSessionManagerLiteProxy();
        CHECK_POINTER_RETURN_INVALID_VALUE(sceneSessionManager);
        TAG_LOGI(AAFwkTag::MISSION, "scb call, GetMissionSnapshot");
        auto err = sceneSessionManager->GetSessionSnapshot(deviceId, missionId, snapshot, isLowResolution);
        if (SCB_TO_MISSION_ERROR_CODE_MAP.count(err)) {
            TAG_LOGE(AAFwkTag::MISSION, "scb call, GetMissionSnapshot err");
            return SCB_TO_MISSION_ERROR_CODE_MAP[err];
        }
        return static_cast<int>(err);
    }
#endif //SUPPORT_SCREEN
    auto abms = GetMissionManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->GetMissionSnapshot(deviceId, missionId, snapshot, isLowResolution);
}


ErrCode MissionManagerClient::CleanMission(int32_t missionId, int32_t userId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
#ifdef SUPPORT_SCREEN
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        auto sceneSessionManager = SessionManagerLite::GetInstance(userId).GetSceneSessionManagerLiteProxy();
        CHECK_POINTER_RETURN_INVALID_VALUE(sceneSessionManager);
        TAG_LOGI(AAFwkTag::MISSION, "scb call, CleanMission");
        auto err = sceneSessionManager->ClearSession(missionId);
        if (SCB_TO_MISSION_ERROR_CODE_MAP.count(err)) {
            TAG_LOGE(AAFwkTag::MISSION, "scb call, CleanMission err");
            return SCB_TO_MISSION_ERROR_CODE_MAP[err];
        }
        return static_cast<int>(err);
    }
#endif //SUPPORT_SCREEN
    auto abms = GetMissionManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->CleanMission(missionId);
}


ErrCode MissionManagerClient::CleanAllMissions(int32_t userId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
#ifdef SUPPORT_SCREEN
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        auto sceneSessionManager = SessionManagerLite::GetInstance(userId).GetSceneSessionManagerLiteProxy();
        CHECK_POINTER_RETURN_INVALID_VALUE(sceneSessionManager);
        TAG_LOGI(AAFwkTag::MISSION, "scb call, CleanAllMissions");
        auto err = sceneSessionManager->ClearAllSessions();
        if (SCB_TO_MISSION_ERROR_CODE_MAP.count(err)) {
            TAG_LOGE(AAFwkTag::MISSION, "scb call, CleanAllMissions err");
            return SCB_TO_MISSION_ERROR_CODE_MAP[err];
        }
        return static_cast<int>(err);
    }
#endif //SUPPORT_SCREEN
    auto abms = GetMissionManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->CleanAllMissions();
}


ErrCode MissionManagerClient::MoveMissionToFront(int32_t missionId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto abms = GetMissionManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->MoveMissionToFront(missionId);
}

ErrCode MissionManagerClient::MoveMissionToFront(int32_t missionId, const StartOptions &startOptions)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto abms = GetMissionManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->MoveMissionToFront(missionId, startOptions);
}


ErrCode MissionManagerClient::MoveMissionsToForeground(const std::vector<int32_t> &missionIds, int32_t topMissionId,
    int32_t userId)
{
    TAG_LOGI(AAFwkTag::MISSION, "call,topMissionId:%{public}d", topMissionId);
#ifdef SUPPORT_SCREEN
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        auto sceneSessionManager = SessionManagerLite::GetInstance(userId).GetSceneSessionManagerLiteProxy();
        CHECK_POINTER_RETURN_INVALID_VALUE(sceneSessionManager);
        TAG_LOGI(AAFwkTag::MISSION, "scb call, MoveMissionsToForeground");
        auto err = sceneSessionManager->MoveSessionsToForeground(missionIds, topMissionId);
        if (SCB_TO_MISSION_ERROR_CODE_MAP.count(err)) {
            TAG_LOGE(AAFwkTag::MISSION, "scb call, MoveMissionsToForeground err");
            return SCB_TO_MISSION_ERROR_CODE_MAP[err];
        }
        auto abms = GetMissionManager();
        CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
        if (missionIds.empty()) {
            return ERR_INVALID_VALUE;
        }
        int32_t missionId = topMissionId;
        if (topMissionId > 0) {
            missionId = topMissionId;
        } else {
            missionId = missionIds[0];
        }
        auto errAMS = abms->MoveMissionToFront(missionId);
        return static_cast<int>(errAMS);
    }
#endif //SUPPORT_SCREEN
    auto abms = GetMissionManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->MoveMissionsToForeground(missionIds, topMissionId);
}

ErrCode MissionManagerClient::MoveMissionsToBackground(const std::vector<int32_t> &missionIds,
    std::vector<int32_t> &result, int32_t userId)
{
    TAG_LOGI(AAFwkTag::MISSION, "call");
#ifdef SUPPORT_SCREEN
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        auto sceneSessionManager = SessionManagerLite::GetInstance(userId).GetSceneSessionManagerLiteProxy();
        CHECK_POINTER_RETURN_INVALID_VALUE(sceneSessionManager);
        TAG_LOGI(AAFwkTag::MISSION, "scb call, MoveMissionsToBackground");
        auto err = sceneSessionManager->MoveSessionsToBackground(missionIds, result);
        if (SCB_TO_MISSION_ERROR_CODE_MAP.count(err)) {
            TAG_LOGE(AAFwkTag::MISSION, "scb call, MoveMissionsToBackground err");
            return SCB_TO_MISSION_ERROR_CODE_MAP[err];
        }
        return static_cast<int>(err);
    }
#endif //SUPPORT_SCREEN
    auto abms = GetMissionManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->MoveMissionsToBackground(missionIds, result);
}

ErrCode MissionManagerClient::GetMissionIdByToken(sptr<IRemoteObject> token, int32_t &missionId)
{
    auto abms = GetMissionManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    missionId = abms->GetMissionIdByToken(token);
    if (missionId <= 0) {
        TAG_LOGE(AAFwkTag::MISSION, "get missionid failed");
        return MISSION_NOT_FOUND;
    }
    return ERR_OK;
}

/**
 * Start synchronizing remote device mission
 * @param devId, deviceId.
 * @param fixConflict, resolve synchronizing conflicts flag.
 * @param tag, call tag.
 * @return Returns ERR_OK on success, others on failure.
 */
ErrCode MissionManagerClient::StartSyncRemoteMissions(const std::string &devId, bool fixConflict, int64_t tag)
{
    auto abms = GetMissionManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->StartSyncRemoteMissions(devId, fixConflict, tag);
}

ErrCode MissionManagerClient::StopSyncRemoteMissions(const std::string &devId)
{
    auto abms = GetMissionManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->StopSyncRemoteMissions(devId);
}

ErrCode MissionManagerClient::SetMissionContinueState(sptr<IRemoteObject> token,
    const AAFwk::ContinueState &state, sptr<IRemoteObject> sessionToken, int32_t userId)
{
    TAG_LOGI(AAFwkTag::MISSION,
        "called state:%{public}d", state);
#ifdef SUPPORT_SCREEN
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled() && sessionToken) {
        auto sceneSessionManager = SessionManagerLite::GetInstance(userId).GetSceneSessionManagerLiteProxy();
        CHECK_POINTER_RETURN_INVALID_VALUE(sceneSessionManager);
        uint32_t value = static_cast<uint32_t>(state);
        Rosen::ContinueState continueState = static_cast<Rosen::ContinueState>(value);
        TAG_LOGI(AAFwkTag::MISSION, "scb call, SetMissionContinueState");
        auto ret = static_cast<int>(sceneSessionManager->SetSessionContinueState(sessionToken, continueState));
        if (ret != ERR_OK) {
            TAG_LOGE(AAFwkTag::MISSION, "scb call, SetMissionContinueState err");
        }
        return ret;
    }
#endif //SUPPORT_SCREEN
    auto abms = GetMissionManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->SetMissionContinueState(token, state);
}


#ifdef SUPPORT_SCREEN
ErrCode MissionManagerClient::SetMissionLabel(sptr<IRemoteObject> token, const std::string &label, int32_t userId)
{
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        auto sceneSessionManager = SessionManagerLite::GetInstance(userId).GetSceneSessionManagerLiteProxy();
        CHECK_POINTER_RETURN_INVALID_VALUE(sceneSessionManager);
        TAG_LOGI(AAFwkTag::MISSION, "scb call, SetMissionLabel");
        auto err = sceneSessionManager->SetSessionLabel(token, label);
        if (SCB_TO_MISSION_ERROR_CODE_MAP.count(err)) {
            TAG_LOGE(AAFwkTag::MISSION, "scb call, SetMissionLabel err");
            return SCB_TO_MISSION_ERROR_CODE_MAP[err];
        }
        return static_cast<int>(err);
    }
    auto abms = GetMissionManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->SetMissionLabel(token, label);
}

ErrCode MissionManagerClient::SetMissionIcon(
    sptr<IRemoteObject> abilityToken, std::shared_ptr<OHOS::Media::PixelMap> icon, int32_t userId)
{
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        auto sceneSessionManager = SessionManagerLite::GetInstance(userId).GetSceneSessionManagerLiteProxy();
        CHECK_POINTER_RETURN_INVALID_VALUE(sceneSessionManager);
        TAG_LOGI(AAFwkTag::MISSION, "scb call, SetMissionIcon");
        auto err = sceneSessionManager->SetSessionIcon(abilityToken, icon);
        if (SCB_TO_MISSION_ERROR_CODE_MAP.count(err)) {
            TAG_LOGE(AAFwkTag::MISSION, "scb call, SetMissionIcon err");
            return SCB_TO_MISSION_ERROR_CODE_MAP[err];
        }
        return static_cast<int>(err);
    }
    auto abms = GetMissionManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->SetMissionIcon(abilityToken, icon);
}

void MissionManagerClient::UpdateMissionSnapShot(sptr<IRemoteObject> token,
    std::shared_ptr<Media::PixelMap> pixelMap)
{
    auto abms = GetMissionManager();
    CHECK_POINTER_RETURN(abms);
    return abms->UpdateMissionSnapShot(token, pixelMap);
}

#endif

int32_t MissionManagerClient::IsValidMissionIds(
    const std::vector<int32_t> &missionIds, std::vector<MissionValidResult> &results, int32_t userId)
{
    TAG_LOGI(AAFwkTag::MISSION, "call");
#ifdef SUPPORT_SCREEN
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        auto sceneSessionManager = SessionManagerLite::GetInstance(userId).GetSceneSessionManagerLiteProxy();
        CHECK_POINTER_RETURN_INVALID_VALUE(sceneSessionManager);
        std::vector<bool> isValidList;
        auto err = sceneSessionManager->IsValidSessionIds(missionIds, isValidList);
        TAG_LOGI(AAFwkTag::MISSION, "scb call, IsValidSessionIds: %{public}d, size: %{public}d",
            static_cast<int>(err), static_cast<int32_t>(isValidList.size()));
        for (auto i = 0; i < static_cast<int32_t>(isValidList.size()); ++i) {
            MissionValidResult missionResult = {};
            missionResult.missionId = missionIds.at(i);
            missionResult.isValid = isValidList.at(i);
            results.push_back(missionResult);
        }
        return static_cast<int>(err);
    }
#endif //SUPPORT_SCREEN
    auto abms = GetMissionManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->IsValidMissionIds(missionIds, results);
}

ErrCode MissionManagerClient::PreStartMission(const std::string &bundleName, const std::string &moduleName,
    const std::string &abilityName, const std::string &startTime)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto abms = GetMissionManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->PreStartMission(bundleName, moduleName, abilityName, startTime);
}


ErrCode MissionManagerClient::TerminateMission(int32_t missionId, int32_t userId)
{
#ifdef SUPPORT_SCREEN
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        auto sceneSessionManager = SessionManagerLite::GetInstance(userId).GetSceneSessionManagerLiteProxy();
        CHECK_POINTER_RETURN_INVALID_VALUE(sceneSessionManager);
        TAG_LOGI(AAFwkTag::MISSION, "scb call, TerminateMission");
        auto err = sceneSessionManager->TerminateSessionByPersistentId(missionId);
        if (err != OHOS::Rosen::WMError::WM_OK) {
            TAG_LOGE(AAFwkTag::MISSION, "scb call, TerminateMission err: %{public}d.", static_cast<int32_t>(err));
        }
        if (err == Rosen::WMError::WM_ERROR_INVALID_PERMISSION) {
            return CHECK_PERMISSION_FAILED;
        }
        if (err == Rosen::WMError::WM_ERROR_NOT_SYSTEM_APP) {
            return ERR_NOT_SYSTEM_APP;
        }
        return static_cast<int32_t>(err);
    }
#endif //SUPPORT_SCREEN
    auto abms = GetMissionManager();
    CHECK_POINTER_RETURN_INVALID_VALUE(abms);
    int32_t ret = abms->TerminateMission(missionId);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::MISSION, "failed,err:%{public}d", ret);
    }
    return ret;
}
}  // namespace AAFwk
}  // namespace OHOS
