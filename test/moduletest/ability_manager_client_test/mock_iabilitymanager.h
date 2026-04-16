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
#ifndef BASE_ABILITYRUNTIME_MOCK_IABILITYMANAGER_H
#define BASE_ABILITYRUNTIME_MOCK_IABILITYMANAGER_H

#include "ability_manager_interface.h"

namespace OHOS {
namespace AAFwk {
class MockIAbilityManager : public IAbilityManager {
public:
    sptr<IRemoteObject> AsObject() override
    {
        return nullptr;
    }
    int StartAbility(
        const Want &want,
        int32_t userId = DEFAULT_INVAL_VALUE,
        int requestCode = DEFAULT_INVAL_VALUE,
        uint64_t specifiedFullTokenId = 0) override
    {
        return 0;
    }
    int StartAbility(
        const Want &want,
        const sptr<IRemoteObject> &callerToken,
        int32_t userId = DEFAULT_INVAL_VALUE,
        int requestCode = DEFAULT_INVAL_VALUE,
        uint64_t specifiedFullTokenId = 0) override
    {
        return 0;
    }
    int StartAbilityWithSpecifyTokenId(
        const Want &want,
        const sptr<IRemoteObject> &callerToken,
        uint32_t specifyTokenId,
        int32_t userId = DEFAULT_INVAL_VALUE,
        int requestCode = DEFAULT_INVAL_VALUE) override
    {
        return 0;
    }
    int32_t StartAbilityByInsightIntent(
        const Want &want,
        const sptr<IRemoteObject> &callerToken,
        uint64_t intentId,
        int32_t userId = DEFAULT_INVAL_VALUE) override
    {
        return 0;
    }
    int StartAbility(
        const Want &want,
        const AbilityStartSetting &abilityStartSetting,
        const sptr<IRemoteObject> &callerToken,
        int32_t userId = DEFAULT_INVAL_VALUE,
        int requestCode = DEFAULT_INVAL_VALUE) override
    {
        return 0;
    }
    int StartAbility(
        const Want &want,
        const StartOptions &startOptions,
        const sptr<IRemoteObject> &callerToken,
        int32_t userId = DEFAULT_INVAL_VALUE,
        int requestCode = DEFAULT_INVAL_VALUE) override
    {
        return 0;
    }
    int TerminateAbility(
        const sptr<IRemoteObject> &token, int resultCode, const Want *resultWant = nullptr) override
    {
        return 0;
    }
    int CloseAbility(const sptr<IRemoteObject> &token, int resultCode = DEFAULT_INVAL_VALUE,
        const Want *resultWant = nullptr) override
    {
        return 0;
    }
    int MinimizeAbility(const sptr<IRemoteObject> &token, bool fromUser = false) override
    {
        return 0;
    }
    int32_t ConnectAbility(
        const Want &want,
        const sptr<IAbilityConnection> &connect,
        const sptr<IRemoteObject> &callerToken,
        int32_t userId = DEFAULT_INVAL_VALUE) override
    {
        return 0;
    }
    int DisconnectAbility(sptr<IAbilityConnection> connect) override
    {
        return 0;
    }
    sptr<IAbilityScheduler> AcquireDataAbility(
        const Uri &uri, bool tryBind, const sptr<IRemoteObject> &callerToken) override
    {
        return nullptr;
    }
    int ReleaseDataAbility(
        sptr<IAbilityScheduler> dataAbilityScheduler, const sptr<IRemoteObject> &callerToken) override
    {
        return 0;
    }
    int AttachAbilityThread(const sptr<IAbilityScheduler> &scheduler, const sptr<IRemoteObject> &token) override
    {
        return 0;
    }
    int AbilityTransitionDone(const sptr<IRemoteObject> &token, int state, const PacMap &saveData) override
    {
        return 0;
    }
    int ScheduleConnectAbilityDone(
        const sptr<IRemoteObject> &token, const sptr<IRemoteObject> &remoteObject) override
    {
        return 0;
    }
    int ScheduleDisconnectAbilityDone(const sptr<IRemoteObject> &token) override
    {
        return 0;
    }
    int ScheduleCommandAbilityDone(const sptr<IRemoteObject> &token) override
    {
        return 0;
    }
    int ScheduleCommandAbilityWindowDone(
        const sptr<IRemoteObject> &token,
        const sptr<AAFwk::SessionInfo> &sessionInfo,
        AAFwk::WindowCommand winCmd,
        AAFwk::AbilityCommand abilityCmd) override
    {
        return 0;
    }
    void DumpState(const std::string &args, std::vector<std::string> &state) override
    {
        return;
    }
    void DumpSysState(
        const std::string& args, std::vector<std::string>& state, bool isClient, bool isUserID, int userID) override
    {
        mockFunctionExcuted = true;
    }
    int StopServiceAbility(const Want &want, int32_t userId = DEFAULT_INVAL_VALUE,
        const sptr<IRemoteObject> &token = nullptr) override
    {
        return 0;
    }
    int KillProcess(const std::string &bundleName, bool clearPageStack = false, int32_t appIndex = 0,
        const std::string &reason = "KillProcess") override
    {
        return 0;
    }
    #ifdef ABILITY_COMMAND_FOR_TEST
    int ForceTimeoutForTest(const std::string &abilityName, const std::string &state) override
    {
        return 0;
    }
    #endif
    sptr<IWantSender> GetWantSender(
        const WantSenderInfo &wantSenderInfo, const sptr<IRemoteObject> &callerToken, int32_t uid = -1) override
    {
        return nullptr;
    }
    int SendWantSender(sptr<IWantSender> target, SenderInfo &senderInfo) override
    {
        return 0;
    }
    void CancelWantSender(const sptr<IWantSender> &sender) override
    {
        return;
    }
    int GetPendingWantUid(const sptr<IWantSender> &target) override
    {
        return 0;
    }
    int GetPendingWantUserId(const sptr<IWantSender> &target) override
    {
        return 0;
    }
    std::string GetPendingWantBundleName(const sptr<IWantSender> &target) override
    {
        return "";
    }
    int GetPendingWantCode(const sptr<IWantSender> &target) override
    {
        return 0;
    }
    int GetPendingWantType(const sptr<IWantSender> &target) override
    {
        return 0;
    }
    void RegisterCancelListener(const sptr<IWantSender> &sender, const sptr<IWantReceiver> &receiver) override
    {
        return;
    }
    void UnregisterCancelListener(const sptr<IWantSender> &sender, const sptr<IWantReceiver> &receiver) override
    {
        return;
    }
    int GetPendingRequestWant(const sptr<IWantSender> &target, std::shared_ptr<Want> &want) override
    {
        return 0;
    }
    int GetPendingRequestWantFromProxy(const sptr<IWantSender> &target, std::shared_ptr<Want> &want) override
    {
        return 0;
    }
    int GetWantSenderInfo(const sptr<IWantSender> &target, std::shared_ptr<WantSenderInfo> &info) override
    {
        return 0;
    }
    int ContinueMission(const std::string &srcDeviceId, const std::string &dstDeviceId, int32_t missionId,
        const sptr<IRemoteObject> &callBack, AAFwk::WantParams &wantParams) override
    {
        return 0;
    }
    int ContinueAbility(const std::string &deviceId, int32_t missionId, uint32_t versionCode) override
    {
        return 0;
    }
    int StartContinuation(const Want &want, const sptr<IRemoteObject> &abilityToken, int32_t status) override
    {
        return 0;
    }
    void NotifyCompleteContinuation(const std::string &deviceId, int32_t sessionId, bool isSuccess) override
    {
        return;
    }
    int NotifyContinuationResult(int32_t missionId, int32_t result) override
    {
        return 0;
    }
    int LockMissionForCleanup(int32_t missionId) override
    {
        return 0;
    }
    int UnlockMissionForCleanup(int32_t missionId) override
    {
        return 0;
    }
    int RegisterMissionListener(const sptr<IMissionListener> &listener) override
    {
        return 0;
    }
    int UnRegisterMissionListener(const sptr<IMissionListener> &listener) override
    {
        return 0;
    }
    int GetMissionInfos(
        const std::string &deviceId, int32_t numMax, std::vector<MissionInfo> &missionInfos) override
    {
        return 0;
    }
    int GetMissionInfo(const std::string &deviceId, int32_t missionId, MissionInfo &missionInfo) override
    {
        return 0;
    }
    int GetMissionSnapshot(const std::string& deviceId, int32_t missionId,
        MissionSnapshot& snapshot, bool isLowResolution) override
    {
        return 0;
    }
    int CleanMission(int32_t missionId) override
    {
        return 0;
    }
    int CleanAllMissions() override
    {
        return 0;
    }
    int MoveMissionToFront(int32_t missionId) override
    {
        return 0;
    }
    int MoveMissionToFront(int32_t missionId, const StartOptions &startOptions) override
    {
        return 0;
    }
    int StartAbilityByCall(const Want &want, const sptr<IAbilityConnection> &connect,
        const sptr<IRemoteObject> &callerToken, int32_t accountId = DEFAULT_INVAL_VALUE, bool isSilent = false,
        bool promotePriority = false, bool isVisible = false) override
    {
        return 0;
    }
    int StartAbilityForPrelaunch(const Want &want, const int32_t frameNum)  override
    {
        return 0;
    }
    int ReleaseCall(const sptr<IAbilityConnection> &connect, const AppExecFwk::ElementName &element) override
    {
        return 0;
    }
    int StartUser(int userId, uint64_t displayId, sptr<IUserCallback> callback, bool isAppRecovery = false) override
    {
        return 0;
    }
    int StopUser(int userId, const sptr<IUserCallback> &callback) override
    {
        return 0;
    }
    #ifdef SUPPORT_SCREEN
    int SetMissionLabel(const sptr<IRemoteObject> &abilityToken, const std::string &label) override
    {
        return 0;
    }
    int SetMissionIcon(const sptr<IRemoteObject> &token,
        const std::shared_ptr<OHOS::Media::PixelMap> &icon) override
    {
        return 0;
    }
    void CompleteFirstFrameDrawing(const sptr<IRemoteObject> &abilityToken) override
    {
        return;
    }
    #endif
    int GetAbilityRunningInfos(std::vector<AbilityRunningInfo> &info) override
    {
        return 0;
    }
    int GetExtensionRunningInfos(int upperLimit, std::vector<ExtensionRunningInfo> &info) override
    {
        return 0;
    }
    int GetProcessRunningInfos(std::vector<AppExecFwk::RunningProcessInfo> &info) override
    {
        return 0;
    }
    int StartSyncRemoteMissions(const std::string &devId, bool fixConflict, int64_t tag) override
    {
        return 0;
    }
    int StopSyncRemoteMissions(const std::string &devId) override
    {
        return 0;
    }
    int RegisterMissionListener(const std::string &deviceId, const sptr<IRemoteMissionListener> &listener) override
    {
        return 0;
    }
    int UnRegisterMissionListener(const std::string &deviceId,
        const sptr<IRemoteMissionListener> &listener) override
    {
        return 0;
    }
    int SetAbilityController(const sptr<AppExecFwk::IAbilityController> &abilityController,
        bool imAStabilityTest) override
    {
        return 0;
    }
    bool IsRunningInStabilityTest() override
    {
        return true;
    }
    int RegisterSnapshotHandler(const sptr<ISnapshotHandler>& handler) override
    {
        return 0;
    }
    int StartUserTest(const Want &want, const sptr<IRemoteObject> &observer) override
    {
        return 0;
    }
    int FinishUserTest(const std::string &msg, const int64_t &resultCode, const std::string &bundleName) override
    {
        return 0;
    }
    int GetTopAbility(sptr<IRemoteObject> &token) override
    {
        return 0;
    }
    int DelegatorDoAbilityForeground(const sptr<IRemoteObject> &token) override
    {
        return 0;
    }
    int DelegatorDoAbilityBackground(const sptr<IRemoteObject> &token) override
    {
        return 0;
    }
    int DoAbilityForeground(const sptr<IRemoteObject> &token, uint32_t flag) override
    {
        return 0;
    }
    int DoAbilityBackground(const sptr<IRemoteObject> &token, uint32_t flag) override
    {
        return 0;
    }
    int32_t GetMissionIdByToken(const sptr<IRemoteObject> &token) override
    {
        return 0;
    }
    void GetAbilityTokenByCalleeObj(const sptr<IRemoteObject> &callStub, sptr<IRemoteObject> &token) override
    {
        return;
    }
    int32_t IsValidMissionIds(
        const std::vector<int32_t> &missionIds, std::vector<MissionValidResult> &results) override
    {
        return 0;
    }
    int32_t ReportDrawnCompleted(const sptr<IRemoteObject> &callerToken) override
    {
        return 0;
    }
    int32_t RegisterAppDebugListener(sptr<AppExecFwk::IAppDebugListener> listener) override
    {
        return 0;
    }
    int32_t UnregisterAppDebugListener(sptr<AppExecFwk::IAppDebugListener> listener) override
    {
        return 0;
    }
    int32_t AttachAppDebug(const std::string &bundleName, bool isDebugFromLocal) override
    {
        return 0;
    }
    int32_t DetachAppDebug(const std::string &bundleName, bool isDebugFromLocal) override
    {
        return 0;
    }
    int32_t ExecuteIntent(uint64_t key, const sptr<IRemoteObject> &callerToken,
        const InsightIntentExecuteParam &param) override
    {
        return 0;
    }
    int32_t ExecuteInsightIntentDone(const sptr<IRemoteObject> &token, uint64_t intentId,
        const InsightIntentExecuteResult &result) override
    {
        return 0;
    }
    int32_t SetApplicationAutoStartupByEDM(const AutoStartupInfo &info, bool flag, bool isHiddenStart = false) override
    {
        return 0;
    }
    int32_t CancelApplicationAutoStartupByEDM(const AutoStartupInfo &info, bool flag) override
    {
        return 0;
    }
    int32_t GetForegroundUIAbilities(std::vector<AppExecFwk::AbilityStateData> &list) override
    {
        return 0;
    }
    void ScheduleClearRecoveryPageStack() override
    {
        mockFunctionExcuted = true;
    }
    void NotifyFrozenProcessByRSS(const std::vector<int32_t> &pidList, int32_t uid) override
    {
        mockFunctionExcuted = true;
    }
    void KillProcessWithPrepareTerminateDone(const std::string &moduleName,
        int32_t prepareTermination, bool isExist) override
    {
        mockFunctionExcuted = true;
    }
    bool mockFunctionExcuted = false;
};
} // namespace AAFwk
} // namespace OHOS
#endif