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

#ifndef MOCK_MISSION_LIST_MANAGER_INTERFACE_H
#define MOCK_MISSION_LIST_MANAGER_INTERFACE_H

#include "mission_list_manager_interface.h"

namespace OHOS {
namespace AAFwk {
class MockMissionListManagerInterface : public MissionListManagerInterface {
public:
    MockMissionListManagerInterface() {};

    virtual ~MockMissionListManagerInterface() = default;

    void Init() override {};
    int StartAbility(AbilityRequest& abilityRequest) override
    {
        return 0;
    }
    int MinimizeAbility(const sptr<IRemoteObject>& token, bool fromUser) override
    {
        return 0;
    };
    int RegisterMissionListener(const sptr<IMissionListener>& listener) override
    {
        return 0;
    };
    int UnRegisterMissionListener(const sptr<IMissionListener>& listener) override
    {
        return 0;
    };
    int GetMissionInfos(int32_t numMax, std::vector<MissionInfo>& missionInfos) override
    {
        return 0;
    }
    int GetMissionInfo(int32_t missionId, MissionInfo& missionInfo) override
    {
        return 0;
    }
    int MoveMissionToFront(int32_t missionId, std::shared_ptr<StartOptions> startOptions = nullptr) override
    {
        return 0;
    }
    int MoveMissionToFront(int32_t missionId, bool isCallerFromLauncher, bool isRecent,
        std::shared_ptr<AbilityRecord> callerAbility, std::shared_ptr<StartOptions> startOptions = nullptr) override
    {
        return 0;
    }
    void NotifyMissionFocused(int32_t missionId) override
    {
        return;
    }
    void NotifyMissionUnfocused(int32_t missionId) override
    {
        return;
    }
    void OnAbilityRequestDone(const sptr<IRemoteObject>& token, int32_t state) override
    {
        return;
    }
    void OnAppStateChanged(const AppInfo& info) override
    {
        return;
    }
    int AttachAbilityThread(const sptr<AAFwk::IAbilityScheduler>& scheduler, const sptr<IRemoteObject>& token) override
    {
        return 0;
    }
    std::shared_ptr<AbilityRecord> GetAbilityRecordByToken(const sptr<IRemoteObject>& token) override
    {
        return nullptr;
    }
    std::shared_ptr<AbilityRecord> GetAbilityRecordByMissionId(int missionId) override
    {
        return nullptr;
    }
    int MoveAbilityToBackground(const std::shared_ptr<AbilityRecord>& abilityRecord) override
    {
        return 0;
    }
    int32_t BackToCallerAbilityWithResult(std::shared_ptr<AbilityRecord> abilityRecord, int32_t resultCode,
        const Want* resultWant, int64_t callerRequestCode) override
    {
        return 0;
    }
    int TerminateAbility(
        const std::shared_ptr<AbilityRecord>& abilityRecord, int resultCode, const Want* resultWant, bool flag) override
    {
        return 0;
    }
    int AbilityTransactionDone(const sptr<IRemoteObject>& token, int state, const PacMap& saveData) override
    {
        return 0;
    }
    std::shared_ptr<AbilityRecord> GetAbilityFromTerminateList(const sptr<IRemoteObject>& token) override
    {
        return nullptr;
    }
    int ClearMission(int missionId) override
    {
        return 0;
    }
    int ClearAllMissions() override
    {
        return 0;
    }

    int SetMissionLockedState(int missionId, bool lockedState) override
    {
        return 0;
    }
    void OnTimeOut(uint32_t msgId, int64_t abilityRecordId, bool isHalf = false) override
    {
        return;
    }
    void OnAbilityDied(std::shared_ptr<AbilityRecord> abilityRecord, int32_t currentUserId) override
    {
        return;
    }
    void OnCallConnectDied(const std::shared_ptr<CallRecord>& callRecord) override
    {
        return;
    }
    int32_t GetMissionIdByAbilityToken(const sptr<IRemoteObject>& token) override
    {
        return 0;
    }
    sptr<IRemoteObject> GetAbilityTokenByMissionId(int32_t missionId) override
    {
        return nullptr;
    }
    void Dump(std::vector<std::string>& info) override
    {
        return;
    }
    void DumpMissionListByRecordId(std::vector<std::string>& info, bool isClient, int32_t abilityRecordId,
        const std::vector<std::string>& params) override
    {
        return;
    }
    void DumpMission(int missionId, std::vector<std::string>& info) override
    {
        return;
    }
    void DumpMissionInfos(std::vector<std::string>& info) override
    {
        return;
    }
    void OnAcceptWantResponse(const AAFwk::Want& want, const std::string& flag) override
    {
        return;
    }
    void OnStartSpecifiedAbilityTimeoutResponse() override
    {
        return;
    }
    int ResolveLocked(const AbilityRequest& abilityRequest) override
    {
        return 0;
    }

    int ReleaseCallLocked(const sptr<IAbilityConnection>& connect, const AppExecFwk::ElementName& element) override
    {
        return 0;
    }
    void RegisterSnapshotHandler(const sptr<ISnapshotHandler>& handler) override
    {
        return;
    }
    bool GetMissionSnapshot(int32_t missionId, MissionSnapshot& missionSnapshot, bool isLowResolution) override
    {
        return false;
    }
    void GetAbilityRunningInfos(std::vector<AbilityRunningInfo>& info, bool isPerm) override
    {
        return;
    }

#ifdef SUPPORT_SCREEN
    void UpdateSnapShot(const sptr<IRemoteObject>& token, const std::shared_ptr<Media::PixelMap>& pixelMap) override
    {
        return;
    }
#endif // SUPPORT_SCREEN

    void EnableRecoverAbility(int32_t missionId) override
    {
        return;
    }

    void UninstallApp(const std::string& bundleName, int32_t uid) override
    {
        return;
    }

    bool IsStarted() override
    {
        return false;
    }
    void PauseManager() override
    {
        return;
    }
    void ResumeManager() override
    {
        return;
    }
    int32_t IsValidMissionIds(const std::vector<int32_t>& missionIds, std::vector<MissionValidResult>& results) override
    {
        return 0;
    }
    void GetActiveAbilityList(int32_t uid, std::vector<std::string>& abilityList, int32_t pid = NO_PID) override
    {
        if (pid < 0) {
            return;
        }
        abilityList.emplace_back("MockMissionListManagerInterfaceTest");
    }
    void CallRequestDone(
        const std::shared_ptr<AbilityRecord>& abilityRecord, const sptr<IRemoteObject>& callStub) override
    {
        return;
    }
    int SetMissionContinueState(
        const sptr<IRemoteObject>& token, int32_t missionId, const AAFwk::ContinueState& state) override
    {
        return 0;
    }

    bool IsAbilityStarted(const AbilityRequest& abilityRequest, std::shared_ptr<AbilityRecord>& targetRecord) override
    {
        return false;
    }
    void SignRestartAppFlag(int32_t uid, const std::string& instanceKey) override
    {
        return;
    }
    void DumpMissionList(std::vector<std::string>& info, bool isClient, const std::string& args = "") override
    {
        return;
    }
    int DoAbilityForeground(std::shared_ptr<AbilityRecord>& abilityRecord, uint32_t flag) override
    {
        return 0;
    }
#ifdef SUPPORT_SCREEN
public:
    int SetMissionLabel(const sptr<IRemoteObject>& abilityToken, const std::string& label) override
    {
        return 0;
    }
    int SetMissionIcon(const sptr<IRemoteObject>& token, const std::shared_ptr<Media::PixelMap>& icon) override
    {
        return 0;
    }
    void CompleteFirstFrameDrawing(const sptr<IRemoteObject>& abilityToken) override
    {
        return;
    }
#endif // SUPPORT_SCREEN
};
} // namespace AAFwk
} // namespace OHOS
#endif // MOCK_MISSION_LIST_MANAGER_INTERFACE_H