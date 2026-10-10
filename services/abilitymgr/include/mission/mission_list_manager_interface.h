/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_MISSION_LIST_MANAGER_INTERFACE_H
#define OHOS_ABILITY_RUNTIME_MISSION_LIST_MANAGER_INTERFACE_H

#include <memory>
#include <string>
#include <vector>

#include "ability_running_info.h"
#include "mission_list.h"
#include "mission_listener_controller.h"
#include "mission_info.h"
#include "mission_snapshot.h"
#include "snapshot.h"
#include "start_options.h"
#include "want.h"
#include "window_config.h"
#include "iability_info_callback.h"

namespace OHOS {
namespace AAFwk {
class MissionListManagerInterface {
public:
    virtual ~MissionListManagerInterface() = default;

    virtual void Init() = 0;
    virtual int StartAbility(AbilityRequest &abilityRequest) = 0;
    virtual int MinimizeAbility(const sptr<IRemoteObject> &token, bool fromUser) = 0;
    virtual int RegisterMissionListener(const sptr<IMissionListener> &listener) = 0;
    virtual int UnRegisterMissionListener(const sptr<IMissionListener> &listener) = 0;
    virtual int GetMissionInfos(int32_t numMax, std::vector<MissionInfo> &missionInfos) = 0;
    virtual int GetMissionInfo(int32_t missionId, MissionInfo &missionInfo) = 0;
    virtual int MoveMissionToFront(int32_t missionId, std::shared_ptr<StartOptions> startOptions = nullptr) = 0;
    virtual int MoveMissionToFront(int32_t missionId, bool isCallerFromLauncher, bool isRecent,
        std::shared_ptr<AbilityRecord> callerAbility, std::shared_ptr<StartOptions> startOptions = nullptr) = 0;
    virtual void NotifyMissionFocused(int32_t missionId) = 0;
    virtual void NotifyMissionUnfocused(int32_t missionId) = 0;
    virtual void OnAbilityRequestDone(const sptr<IRemoteObject> &token, int32_t state) = 0;
    virtual void OnAppStateChanged(const AppInfo &info) = 0;
    virtual int AttachAbilityThread(const sptr<AAFwk::IAbilityScheduler> &scheduler,
        const sptr<IRemoteObject> &token) = 0;
    virtual std::shared_ptr<AbilityRecord> GetAbilityRecordByToken(const sptr<IRemoteObject> &token) = 0;
    virtual std::shared_ptr<AbilityRecord> GetAbilityRecordByMissionId(int missionId) = 0;
    virtual int MoveAbilityToBackground(const std::shared_ptr<AbilityRecord> &abilityRecord) = 0;
    virtual int32_t BackToCallerAbilityWithResult(std::shared_ptr<AbilityRecord> abilityRecord,
        int32_t resultCode, const Want *resultWant, int64_t callerRequestCode) = 0;
    virtual int TerminateAbility(const std::shared_ptr<AbilityRecord> &abilityRecord,
        int resultCode, const Want *resultWant, bool flag) = 0;
    virtual int AbilityTransactionDone(const sptr<IRemoteObject> &token, int state, const PacMap &saveData) = 0;
    virtual std::shared_ptr<AbilityRecord> GetAbilityFromTerminateList(const sptr<IRemoteObject> &token) = 0;
    virtual int ClearMission(int missionId) = 0;
    virtual int ClearAllMissions() = 0;

    virtual int SetMissionLockedState(int missionId, bool lockedState) = 0;
    virtual void OnTimeOut(uint32_t msgId, int64_t abilityRecordId, bool isHalf = false) = 0;
    virtual void OnAbilityDied(std::shared_ptr<AbilityRecord> abilityRecord, int32_t currentUserId) = 0;
    virtual void OnCallConnectDied(const std::shared_ptr<CallRecord> &callRecord) = 0;
    virtual int32_t GetMissionIdByAbilityToken(const sptr<IRemoteObject> &token) = 0;
    virtual sptr<IRemoteObject> GetAbilityTokenByMissionId(int32_t missionId) = 0;

    virtual void Dump(std::vector<std::string> &info) = 0;

    virtual void DumpMissionList(std::vector<std::string> &info, bool isClient, const std::string &args = "") = 0;

    virtual void DumpMissionListByRecordId(std::vector<std::string> &info, bool isClient, int32_t abilityRecordId,
        const std::vector<std::string> &params) = 0;
    virtual void DumpMission(int missionId, std::vector<std::string> &info) = 0;
    virtual void DumpMissionInfos(std::vector<std::string> &info) = 0;
    virtual void OnAcceptWantResponse(const AAFwk::Want &want, const std::string &flag) = 0;
    virtual void OnStartSpecifiedAbilityTimeoutResponse() = 0;
    virtual int ResolveLocked(const AbilityRequest &abilityRequest) = 0;

    virtual int ReleaseCallLocked(const sptr<IAbilityConnection> &connect,
        const AppExecFwk::ElementName &element) = 0;
    virtual void RegisterSnapshotHandler(const sptr<ISnapshotHandler> &handler) = 0;
    virtual bool GetMissionSnapshot(int32_t missionId, MissionSnapshot &missionSnapshot, bool isLowResolution) = 0;
    virtual void GetAbilityRunningInfos(std::vector<AbilityRunningInfo> &info, bool isPerm) = 0;

#ifdef SUPPORT_SCREEN
    virtual void UpdateSnapShot(const sptr<IRemoteObject> &token,
        const std::shared_ptr<Media::PixelMap> &pixelMap) = 0;
#endif // SUPPORT_SCREEN

    virtual void EnableRecoverAbility(int32_t missionId) = 0;

    virtual void UninstallApp(const std::string &bundleName, int32_t uid) = 0;

    virtual bool IsStarted() = 0;
    virtual void PauseManager() = 0;
    virtual void ResumeManager() = 0;
    virtual int32_t IsValidMissionIds(const std::vector<int32_t> &missionIds,
        std::vector<MissionValidResult> &results) = 0;
    virtual int DoAbilityForeground(std::shared_ptr<AbilityRecord> &abilityRecord, uint32_t flag) = 0;
    virtual void GetActiveAbilityList(int32_t uid, std::vector<std::string> &abilityList, int32_t pid = NO_PID) = 0;
    virtual void CallRequestDone(const std::shared_ptr<AbilityRecord> &abilityRecord,
        const sptr<IRemoteObject> &callStub) = 0;
    virtual int SetMissionContinueState(const sptr<IRemoteObject> &token, int32_t missionId,
        const AAFwk::ContinueState &state) = 0;

    virtual bool IsAbilityStarted(AbilityRequest &abilityRequest, std::shared_ptr<AbilityRecord> &targetRecord) = 0;
    virtual void SignRestartAppFlag(int32_t uid, const std::string &instanceKey) = 0;
#ifdef SUPPORT_SCREEN
public:
    virtual int SetMissionLabel(const sptr<IRemoteObject> &abilityToken, const std::string &label) = 0;
    virtual int SetMissionIcon(const sptr<IRemoteObject> &token, const std::shared_ptr<Media::PixelMap> &icon) = 0;
    virtual void CompleteFirstFrameDrawing(const sptr<IRemoteObject> &abilityToken) = 0;
#endif
};

class MissionListWrap {
public:
    virtual ~MissionListWrap() = default;
    virtual std::shared_ptr<MissionListManagerInterface> CreateMissionListManager(int32_t userId) = 0;
    virtual void RemoveUserDir(int32_t userId) = 0;
    virtual void InitMissionInfoMgr(int32_t userId) = 0;
    virtual void SetMissionAbilityState(int32_t missionId, AbilityState state) = 0;
    virtual int32_t GetInnerMissionInfoById(int32_t missionId, InnerMissionInfo &innerMissionInfo) = 0;
};
}  // namespace AAFwk
}  // namespace OHOS

extern "C" __attribute__((visibility("default"))) OHOS::AAFwk::MissionListWrap* CreateMissionListWrap();

#endif  // OHOS_ABILITY_RUNTIME_MISSION_LIST_MANAGER_INTERFACE_H
