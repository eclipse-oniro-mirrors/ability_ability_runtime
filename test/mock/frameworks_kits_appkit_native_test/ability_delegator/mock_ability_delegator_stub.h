/*
 * Copyright (c) 2022-2026 Huawei Device Co., Ltd.
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

#ifndef MOCK_OHOS_ABILITY_RUNTIME_MOCK_ABILITY_DELEGATOR_STUB_H
#define MOCK_OHOS_ABILITY_RUNTIME_MOCK_ABILITY_DELEGATOR_STUB_H

#include "gmock/gmock.h"

#include "string_ex.h"
#include "ability_manager_errors.h"
#include "ability_manager_stub.h"

namespace OHOS {
namespace AAFwk {
class MockAbilityDelegatorStub : public AbilityManagerStub {
public:
    MockAbilityDelegatorStub();
    int StartAbility(const Want& want, int32_t userId = DEFAULT_INVAL_VALUE,
        int requestCode = DEFAULT_INVAL_VALUE, uint64_t specifiedFullTokenId = 0) override;
    MOCK_METHOD5(StartAbility, int(const Want& want, const sptr<IRemoteObject>& callerToken,
        int32_t userId, int requestCode, uint64_t specifiedFullTokenId));
    MOCK_METHOD5(StartAbilityAsCaller, int(const Want& want, const sptr<IRemoteObject>& callerToken,
        sptr<IRemoteObject> asCallerSourceToken, int32_t userId, int requestCode));
    MOCK_METHOD5(StartAbility, int(const Want& want, const AbilityStartSetting& abilityStartSetting,
        const sptr<IRemoteObject>& callerToken, int32_t userId, int requestCode));
    MOCK_METHOD4(StartAbilityByInsightIntent, int32_t(const Want& want, const sptr<IRemoteObject>& callerToken,
        uint64_t intentId, int32_t userId));
    int StartAbility(const Want& want, const StartOptions& startOptions,
        const sptr<IRemoteObject>& callerToken, int requestCode = DEFAULT_INVAL_VALUE,
        int32_t userId = DEFAULT_INVAL_VALUE) override;
    int StartAbilityAsCaller(const Want& want, const StartOptions& startOptions,
        const sptr<IRemoteObject>& callerToken, sptr<IRemoteObject> asCallerSourceToken,
        int requestCode = DEFAULT_INVAL_VALUE, int32_t userId = DEFAULT_INVAL_VALUE) override;
    MOCK_METHOD3(TerminateAbility, int(const sptr<IRemoteObject>& token, int resultCode, const Want* resultWant));
    int CloseAbility(const sptr<IRemoteObject>& token, int resultCode = DEFAULT_INVAL_VALUE,
        const Want* resultWant = nullptr) override;
    int MinimizeAbility(const sptr<IRemoteObject>& token, bool fromUser = false) override;
    MOCK_METHOD4(ConnectAbility, int(const Want& want, const sptr<IAbilityConnection>& connect,
        const sptr<IRemoteObject>& callerToken, int32_t userId));
    MOCK_METHOD1(DisconnectAbility, int(sptr<IAbilityConnection> connect));
    MOCK_METHOD3(AcquireDataAbility,
        sptr<IAbilityScheduler>(const Uri& uri, bool tryBind, const sptr<IRemoteObject>& callerToken));
    MOCK_METHOD2(
        ReleaseDataAbility, int(sptr<IAbilityScheduler> dataAbilityScheduler, const sptr<IRemoteObject>& callerToken));
    MOCK_METHOD2(AddWindowInfo, void(const sptr<IRemoteObject>& token, int32_t windowToken));
    MOCK_METHOD2(AttachAbilityThread, int(const sptr<IAbilityScheduler>& scheduler, const sptr<IRemoteObject>& token));
    MOCK_METHOD3(AbilityTransitionDone, int(const sptr<IRemoteObject>& token, int state, const PacMap&));
    MOCK_METHOD2(
        ScheduleConnectAbilityDone, int(const sptr<IRemoteObject>& token, const sptr<IRemoteObject>& remoteObject));
    MOCK_METHOD1(ScheduleDisconnectAbilityDone, int(const sptr<IRemoteObject>& token));
    MOCK_METHOD1(ScheduleCommandAbilityDone, int(const sptr<IRemoteObject>& token));
    MOCK_METHOD4(ScheduleCommandAbilityWindowDone, int(const sptr<IRemoteObject> &token,
        const sptr<SessionInfo> &sessionInfo, WindowCommand winCmd, AbilityCommand abilityCmd));
    void DumpState(const std::string& args, std::vector<std::string>& state) override;
    int StopServiceAbility(const Want& want, int32_t userId = DEFAULT_INVAL_VALUE,
        const sptr<IRemoteObject> &token = nullptr) override;
    MOCK_METHOD1(MoveMissionToTop, int(int32_t missionId));
    MOCK_METHOD1(RemoveMission, int(int id));
    MOCK_METHOD1(RemoveStack, int(int id));
    MOCK_METHOD4(KillProcess, int(const std::string& bundleName, const bool clearPageStack, int32_t appIndex,
        const std::string& reason));
    MOCK_METHOD2(UninstallApp, int(const std::string& bundleName, int32_t uid));
    MOCK_METHOD3(UninstallApp, int32_t(const std::string& bundleName, int32_t uid, int32_t appIndex));
    MOCK_METHOD2(MoveMissionToEnd, int(const sptr<IRemoteObject>& token, const bool nonFirst));
    MOCK_METHOD1(IsFirstInMission, bool(const sptr<IRemoteObject>& token));
    MOCK_METHOD4(CompelVerifyPermission, int(const std::string& permission, int pid, int uid, std::string& message));
    MOCK_METHOD3(
        GetWantSender, sptr<IWantSender>(const WantSenderInfo& wantSenderInfo, const sptr<IRemoteObject>& callerToken,
        int32_t uid));
    MOCK_METHOD2(SendWantSender, int(sptr<IWantSender> target, SenderInfo& senderInfo));
    MOCK_METHOD1(CancelWantSender, void(const sptr<IWantSender>& sender));
    MOCK_METHOD1(GetPendingWantUid, int(const sptr<IWantSender>& target));
    MOCK_METHOD1(GetPendingWantUserId, int(const sptr<IWantSender>& target));
    MOCK_METHOD1(GetPendingWantBundleName, std::string(const sptr<IWantSender>& target));
    MOCK_METHOD1(GetPendingWantCode, int(const sptr<IWantSender>& target));
    MOCK_METHOD1(GetPendingWantType, int(const sptr<IWantSender>& target));
    MOCK_METHOD2(RegisterCancelListener, void(const sptr<IWantSender>& sender, const sptr<IWantReceiver>& receiver));
    MOCK_METHOD2(UnregisterCancelListener, void(const sptr<IWantSender>& sender, const sptr<IWantReceiver>& receiver));
    MOCK_METHOD2(GetPendingRequestWant, int(const sptr<IWantSender>& target, std::shared_ptr<Want>& want));
    MOCK_METHOD2(GetPendingRequestWantFromProxy, int(const sptr<IWantSender>& target, std::shared_ptr<Want>& want));

    MOCK_METHOD1(LockMission, int(int));
    MOCK_METHOD1(UnlockMission, int(int));
    MOCK_METHOD0(GetMissionLockModeState, int());
    MOCK_METHOD2(
        ChangeFocusAbility, int(const sptr<IRemoteObject>& lostFocusToken, const sptr<IRemoteObject>& getFocusToken));
    MOCK_METHOD1(MinimizeMultiWindow, int(int missionId));
    MOCK_METHOD1(MaximizeMultiWindow, int(int missionId));
    MOCK_METHOD1(CloseMultiWindow, int(int missionId));
    MOCK_METHOD1(GetPendinTerminateAbilityTestgRequestWant, void(int id));
    MOCK_METHOD1(SetShowOnLockScreen, int(bool isAllow));
    MOCK_METHOD3(StartContinuation, int(const Want& want, const sptr<IRemoteObject>& abilityToken, int32_t status));
    MOCK_METHOD2(NotifyContinuationResult, int(int32_t missionId, int32_t result));
    MOCK_METHOD5(ContinueMission, int(const std::string& srcDeviceId, const std::string& dstDeviceId,
        int32_t missionId, const sptr<IRemoteObject>& callBack, AAFwk::WantParams& wantParams));
    MOCK_METHOD3(ContinueAbility, int(const std::string& deviceId, int32_t missionId, uint32_t versionCode));
    MOCK_METHOD3(NotifyCompleteContinuation, void(const std::string& deviceId, int32_t sessionId, bool isSuccess));
    MOCK_METHOD1(LockMissionForCleanup, int(int32_t missionId));
    MOCK_METHOD1(UnlockMissionForCleanup, int(int32_t missionId));
    MOCK_METHOD1(RegisterMissionListener, int(const sptr<IMissionListener>& listener));
    MOCK_METHOD1(UnRegisterMissionListener, int(const sptr<IMissionListener>& listener));
    MOCK_METHOD3(
        GetMissionInfos, int(const std::string& deviceId, int32_t numMax, std::vector<MissionInfo>& missionInfos));
    MOCK_METHOD3(GetMissionInfo, int(const std::string& deviceId, int32_t missionId, MissionInfo& missionInfo));
    MOCK_METHOD1(CleanMission, int(int32_t missionId));
    MOCK_METHOD0(CleanAllMissions, int());
    MOCK_METHOD1(MoveMissionToFront, int(int32_t missionId));
    MOCK_METHOD2(MoveMissionToFront, int(int32_t missionId, const StartOptions& startOptions));
    MOCK_METHOD2(MoveMissionsToForeground, int(const std::vector<int32_t>& missionIds, int32_t topMissionId));
    MOCK_METHOD2(MoveMissionsToBackground, int(const std::vector<int32_t>& missionIds, std::vector<int32_t>& result));
    MOCK_METHOD1(GetAbilityRunningInfos, int(std::vector<AbilityRunningInfo>& info));
    MOCK_METHOD2(GetExtensionRunningInfos, int(int upperLimit, std::vector<ExtensionRunningInfo>& info));
    MOCK_METHOD1(GetProcessRunningInfos, int(std::vector<AppExecFwk::RunningProcessInfo>& info));
    MOCK_METHOD2(GetWantSenderInfo, int(const sptr<IWantSender>& target, std::shared_ptr<WantSenderInfo>& info));
    MOCK_METHOD2(AcquireShareData, int32_t(const int32_t &missionId, const sptr<IAcquireShareDataCallback> &shareData));
    MOCK_METHOD4(ShareDataDone, int32_t(const sptr<IRemoteObject> &token,
        const int32_t &resultCode, const int32_t &uniqueId, WantParams &wantParam));
    MOCK_METHOD2(SetMissionContinueState, int(const sptr<IRemoteObject>& token, const AAFwk::ContinueState& state));
    int StartUser(int userId, uint64_t displayId, sptr<IUserCallback> callback, bool isAppRecovery) override;
    int StopUser(int userId, const sptr<IUserCallback>& callback) override;
    int LogoutUser(int32_t userId, sptr<IUserCallback> callback = nullptr) override;
    int StartSyncRemoteMissions(const std::string& devId, bool fixConflict, int64_t tag) override;
    int StopSyncRemoteMissions(const std::string& devId) override;
    int RegisterMissionListener(const std::string& deviceId,
        const sptr<IRemoteMissionListener>& listener) override;
    int UnRegisterMissionListener(const std::string& deviceId,
        const sptr<IRemoteMissionListener>& listener) override;
    int StartAbilityByCall(const Want& want, const sptr<IAbilityConnection>& connect,
        const sptr<IRemoteObject>& callerToken, int32_t accountId = DEFAULT_INVAL_VALUE,
        bool isSilent = false, bool promotePriority = false, bool isVisible = false) override;
    int StartAbilityForPrelaunch(const Want &want, const int32_t frameNum) override;
    void CallRequestDone(const sptr<IRemoteObject>& token, const sptr<IRemoteObject>& callStub) override;
    int ReleaseCall(const sptr<IAbilityConnection>& connect,
        const AppExecFwk::ElementName& element) override;
    int GetMissionSnapshot(const std::string& deviceId, int32_t missionId,
        MissionSnapshot& snapshot, bool isLowResolution) override;
    int RegisterSnapshotHandler(const sptr<ISnapshotHandler>& handler) override;
    int SetAbilityController(const sptr<AppExecFwk::IAbilityController>& abilityController,
        bool imAStabilityTest) override;
    bool IsRunningInStabilityTest() override;
    void DumpSysState(
        const std::string& args, std::vector<std::string>& info, bool isClient, bool isUserID, int UserID) override;
    int StartUserTest(const Want& want, const sptr<IRemoteObject>& observer) override;
    int FinishUserTest(
        const std::string& msg, const int64_t& resultCode, const std::string& bundleName) override;
    int GetTopAbility(sptr<IRemoteObject>& token) override;
    int DelegatorDoAbilityForeground(const sptr<IRemoteObject>& token) override;
    int DelegatorDoAbilityBackground(const sptr<IRemoteObject>& token) override;
    int32_t ReportDrawnCompleted(const sptr<IRemoteObject>& callerToken) override;
    int DoAbilityForeground(const sptr<IRemoteObject>& token, uint32_t flag);
    int DoAbilityBackground(const sptr<IRemoteObject>& token, uint32_t flag);
    int32_t GetMissionIdByToken(const sptr<IRemoteObject>& token);
    void GetAbilityTokenByCalleeObj(const sptr<IRemoteObject> &callStub, sptr<IRemoteObject> &token);
    int32_t SetApplicationAutoStartupByEDM(const AutoStartupInfo &info, bool flag, bool isHiddenStart = false) override
    {
        return 0;
    }

    int32_t CancelApplicationAutoStartupByEDM(const AutoStartupInfo &info, bool flag) override
    {
        return 0;
    }

    virtual int32_t GetForegroundUIAbilities(std::vector<AppExecFwk::AbilityStateData> &list)
    {
        return 0;
    }
#ifdef ABILITY_COMMAND_FOR_TEST
    int ForceTimeoutForTest(const std::string& abilityName, const std::string& state) override;
#endif
#ifdef SUPPORT_GRAPHICS
    MOCK_METHOD2(SetMissionLabel, int(const sptr<IRemoteObject>& token, const std::string& label));
    int SetMissionIcon(
        const sptr<IRemoteObject>& token, const std::shared_ptr<OHOS::Media::PixelMap>& icon) override;
    int RegisterWindowManagerServiceHandler(const sptr<IWindowManagerServiceHandler>& handler,
        bool animationEnabled = true);
    void CompleteFirstFrameDrawing(const sptr<IRemoteObject>& abilityToken) override {}
#endif
    MOCK_METHOD2(IsValidMissionIds, int32_t(const std::vector<int32_t>&, std::vector<MissionValidResult>&));
    MOCK_METHOD1(RegisterAppDebugListener, int32_t(sptr<AppExecFwk::IAppDebugListener> listener));
    MOCK_METHOD1(UnregisterAppDebugListener, int32_t(sptr<AppExecFwk::IAppDebugListener> listener));
    MOCK_METHOD2(AttachAppDebug, int32_t(const std::string &bundleName, bool isDebugFromLocal));
    MOCK_METHOD2(DetachAppDebug, int32_t(const std::string &bundleName, bool isDebugFromLocal));
    MOCK_METHOD3(ExecuteIntent, int32_t(uint64_t key, const sptr<IRemoteObject> &callerToken,
        const InsightIntentExecuteParam &param));
    MOCK_METHOD3(ExecuteInsightIntentDone, int32_t(const sptr<IRemoteObject> &token, uint64_t intentId,
        const InsightIntentExecuteResult &result));
    MOCK_METHOD5(StartAbilityWithSpecifyTokenId, int(const Want& want, const sptr<IRemoteObject>& callerToken,
        uint32_t specifyTokenId, int32_t userId, int requestCode));
public:
    std::string powerState_;
    static bool finishFlag_;
};

enum TESTCASE_BRANCH {
    BRANCH_1 = 1,
    BRANCH_2 = 2,
    BRANCH_3 = 3,
};

class MockAbilityDelegatorStub2 : public AbilityManagerStub {
public:
    MockAbilityDelegatorStub2();
    int StartAbility(const Want& want, int32_t userId = DEFAULT_INVAL_VALUE, int requestCode = -1,
        uint64_t specifiedFullTokenId = 0) override;
    MOCK_METHOD5(StartAbility, int(const Want& want, const sptr<IRemoteObject>& callerToken,
        int32_t userId, int requestCode, uint64_t specifiedFullTokenId));
    MOCK_METHOD5(StartAbilityAsCaller, int(const Want& want, const sptr<IRemoteObject>& callerToken,
        sptr<IRemoteObject> asCallerSourceToken, int32_t userId, int requestCode));
    MOCK_METHOD5(StartAbility, int(const Want& want, const AbilityStartSetting& abilityStartSetting,
        const sptr<IRemoteObject>& callerToken, int32_t userId, int requestCode));
    MOCK_METHOD4(StartAbilityByInsightIntent, int32_t(const Want& want, const sptr<IRemoteObject>& callerToken,
        uint64_t intentId, int32_t userId));
    int StartAbility(const Want& want, const StartOptions& startOptions,
        const sptr<IRemoteObject>& callerToken, int requestCode = DEFAULT_INVAL_VALUE,
        int32_t userId = DEFAULT_INVAL_VALUE) override;
    int StartAbilityAsCaller(const Want& want, const StartOptions& startOptions,
        const sptr<IRemoteObject>& callerToken, sptr<IRemoteObject> asCallerSourceToken,
        int requestCode = DEFAULT_INVAL_VALUE, int32_t userId = DEFAULT_INVAL_VALUE) override;
    MOCK_METHOD3(TerminateAbility, int(const sptr<IRemoteObject>& token, int resultCode, const Want* resultWant));
    int CloseAbility(const sptr<IRemoteObject>& token, int resultCode = DEFAULT_INVAL_VALUE,
        const Want* resultWant = nullptr) override;
    int MinimizeAbility(const sptr<IRemoteObject>& token, bool fromUser = false) override;
    MOCK_METHOD4(ConnectAbility, int(const Want& want, const sptr<IAbilityConnection>& connect,
        const sptr<IRemoteObject>& callerToken, int32_t userId));
    MOCK_METHOD1(DisconnectAbility, int(sptr<IAbilityConnection> connect));
    MOCK_METHOD3(AcquireDataAbility,
        sptr<IAbilityScheduler>(const Uri& uri, bool tryBind, const sptr<IRemoteObject>& callerToken));
    MOCK_METHOD2(
        ReleaseDataAbility, int(sptr<IAbilityScheduler> dataAbilityScheduler, const sptr<IRemoteObject>& callerToken));
    MOCK_METHOD2(AddWindowInfo, void(const sptr<IRemoteObject>& token, int32_t windowToken));
    MOCK_METHOD2(AttachAbilityThread, int(const sptr<IAbilityScheduler>& scheduler, const sptr<IRemoteObject>& token));
    MOCK_METHOD3(AbilityTransitionDone, int(const sptr<IRemoteObject>& token, int state, const PacMap&));
    MOCK_METHOD2(
        ScheduleConnectAbilityDone, int(const sptr<IRemoteObject>& token, const sptr<IRemoteObject>& remoteObject));
    MOCK_METHOD1(ScheduleDisconnectAbilityDone, int(const sptr<IRemoteObject>& token));
    MOCK_METHOD1(ScheduleCommandAbilityDone, int(const sptr<IRemoteObject>& token));
    MOCK_METHOD4(ScheduleCommandAbilityWindowDone, int(const sptr<IRemoteObject> &token,
        const sptr<SessionInfo> &sessionInfo, WindowCommand winCmd, AbilityCommand abilityCmd));
    void DumpState(const std::string& args, std::vector<std::string>& state) override;
    int StopServiceAbility(const Want& want, int32_t userId = DEFAULT_INVAL_VALUE,
        const sptr<IRemoteObject> &token = nullptr) override;
    MOCK_METHOD1(MoveMissionToTop, int(int32_t missionId));
    MOCK_METHOD1(RemoveMission, int(int id));
    MOCK_METHOD1(RemoveStack, int(int id));
    MOCK_METHOD4(KillProcess, int(const std::string& bundleName, const bool clearPageStack, int32_t appIndex,
        const std::string& reason));
    MOCK_METHOD2(UninstallApp, int(const std::string& bundleName, int32_t uid));
    MOCK_METHOD3(UninstallApp, int32_t(const std::string& bundleName, int32_t uid, int32_t appIndex));
    MOCK_METHOD2(MoveMissionToEnd, int(const sptr<IRemoteObject>& token, const bool nonFirst));
    MOCK_METHOD1(IsFirstInMission, bool(const sptr<IRemoteObject>& token));
    MOCK_METHOD4(CompelVerifyPermission, int(const std::string& permission, int pid, int uid, std::string& message));
    MOCK_METHOD3(
        GetWantSender, sptr<IWantSender>(const WantSenderInfo& wantSenderInfo, const sptr<IRemoteObject>& callerToken,
        int32_t uid));
    MOCK_METHOD2(SendWantSender, int(sptr<IWantSender> target, SenderInfo& senderInfo));
    MOCK_METHOD1(CancelWantSender, void(const sptr<IWantSender>& sender));
    MOCK_METHOD1(GetPendingWantUid, int(const sptr<IWantSender>& target));
    MOCK_METHOD1(GetPendingWantUserId, int(const sptr<IWantSender>& target));
    MOCK_METHOD1(GetPendingWantBundleName, std::string(const sptr<IWantSender>& target));
    MOCK_METHOD1(GetPendingWantCode, int(const sptr<IWantSender>& target));
    MOCK_METHOD1(GetPendingWantType, int(const sptr<IWantSender>& target));
    MOCK_METHOD2(RegisterCancelListener, void(const sptr<IWantSender>& sender, const sptr<IWantReceiver>& receiver));
    MOCK_METHOD2(UnregisterCancelListener, void(const sptr<IWantSender>& sender, const sptr<IWantReceiver>& receiver));
    MOCK_METHOD2(GetPendingRequestWant, int(const sptr<IWantSender>& target, std::shared_ptr<Want>& want));
    MOCK_METHOD2(GetPendingRequestWantFromProxy, int(const sptr<IWantSender>& target, std::shared_ptr<Want>& want));

    MOCK_METHOD1(LockMission, int(int));
    MOCK_METHOD1(UnlockMission, int(int));
    MOCK_METHOD0(GetMissionLockModeState, int());
    MOCK_METHOD2(
        ChangeFocusAbility, int(const sptr<IRemoteObject>& lostFocusToken, const sptr<IRemoteObject>& getFocusToken));
    MOCK_METHOD1(MinimizeMultiWindow, int(int missionId));
    MOCK_METHOD1(MaximizeMultiWindow, int(int missionId));
    MOCK_METHOD1(CloseMultiWindow, int(int missionId));
    MOCK_METHOD1(GetPendinTerminateAbilityTestgRequestWant, void(int id));
    MOCK_METHOD1(SetShowOnLockScreen, int(bool isAllow));
    MOCK_METHOD3(StartContinuation, int(const Want& want, const sptr<IRemoteObject>& abilityToken, int32_t status));
    MOCK_METHOD2(NotifyContinuationResult, int(int32_t missionId, int32_t result));
    MOCK_METHOD5(ContinueMission, int(const std::string& srcDeviceId, const std::string& dstDeviceId,
        int32_t missionId, const sptr<IRemoteObject>& callBack, AAFwk::WantParams& wantParams));
    MOCK_METHOD3(ContinueAbility, int(const std::string& deviceId, int32_t missionId, uint32_t versionCode));
    MOCK_METHOD3(NotifyCompleteContinuation, void(const std::string& deviceId, int32_t sessionId, bool isSuccess));
    MOCK_METHOD1(LockMissionForCleanup, int(int32_t missionId));
    MOCK_METHOD1(UnlockMissionForCleanup, int(int32_t missionId));
    MOCK_METHOD1(RegisterMissionListener, int(const sptr<IMissionListener>& listener));
    MOCK_METHOD1(UnRegisterMissionListener, int(const sptr<IMissionListener>& listener));
    MOCK_METHOD3(
        GetMissionInfos, int(const std::string& deviceId, int32_t numMax, std::vector<MissionInfo>& missionInfos));
    MOCK_METHOD3(GetMissionInfo, int(const std::string& deviceId, int32_t missionId, MissionInfo& missionInfo));
    MOCK_METHOD1(CleanMission, int(int32_t missionId));
    MOCK_METHOD0(CleanAllMissions, int());
    MOCK_METHOD1(MoveMissionToFront, int(int32_t missionId));
    MOCK_METHOD2(MoveMissionToFront, int(int32_t missionId, const StartOptions& startOptions));
    MOCK_METHOD2(MoveMissionsToForeground, int(const std::vector<int32_t>& missionIds, int32_t topMissionId));
    MOCK_METHOD2(MoveMissionsToBackground, int(const std::vector<int32_t>& missionIds, std::vector<int32_t>& result));
    MOCK_METHOD1(GetAbilityRunningInfos, int(std::vector<AbilityRunningInfo>& info));
    MOCK_METHOD2(GetExtensionRunningInfos, int(int upperLimit, std::vector<ExtensionRunningInfo>& info));
    MOCK_METHOD1(GetProcessRunningInfos, int(std::vector<AppExecFwk::RunningProcessInfo>& info));
    MOCK_METHOD2(GetWantSenderInfo, int(const sptr<IWantSender>& target, std::shared_ptr<WantSenderInfo>& info));
    MOCK_METHOD2(AcquireShareData, int32_t(const int32_t &missionId, const sptr<IAcquireShareDataCallback> &shareData));
    MOCK_METHOD4(ShareDataDone, int32_t(const sptr<IRemoteObject> &token,
        const int32_t &resultCode, const int32_t &uniqueId, WantParams &wantParam));
    MOCK_METHOD2(SetMissionContinueState, int(const sptr<IRemoteObject>& token, const AAFwk::ContinueState& state));
    int StartUser(int userId, uint64_t displayId, sptr<IUserCallback> callback, bool isAppRecovery) override;
    int StopUser(int userId, const sptr<IUserCallback>& callback) override;
    int LogoutUser(int32_t userId, sptr<IUserCallback> callback = nullptr) override;
    int StartSyncRemoteMissions(const std::string& devId, bool fixConflict, int64_t tag) override;
    int StopSyncRemoteMissions(const std::string& devId) override;
    int RegisterMissionListener(const std::string& deviceId,
        const sptr<IRemoteMissionListener>& listener) override;
    int UnRegisterMissionListener(const std::string& deviceId,
        const sptr<IRemoteMissionListener>& listener) override;
    int StartAbilityByCall(const Want& want, const sptr<IAbilityConnection>& connect,
        const sptr<IRemoteObject>& callerToken, int32_t accountId = DEFAULT_INVAL_VALUE,
        bool isSilent = false, bool promotePriority = false, bool isVisible = false) override;
    int StartAbilityForPrelaunch(const Want &want, const int32_t frameNum) override;
    void CallRequestDone(const sptr<IRemoteObject>& token, const sptr<IRemoteObject>& callStub) override;
    virtual int32_t GetForegroundUIAbilities(std::vector<AppExecFwk::AbilityStateData> &list)
    {
        return 0;
    }
    int ReleaseCall(const sptr<IAbilityConnection>& connect,
        const AppExecFwk::ElementName& element) override;
    int GetMissionSnapshot(const std::string& deviceId, int32_t missionId,
        MissionSnapshot& snapshot, bool isLowResolution) override;
    int RegisterSnapshotHandler(const sptr<ISnapshotHandler>& handler) override;
    int SetAbilityController(const sptr<AppExecFwk::IAbilityController>& abilityController,
        bool imAStabilityTest) override;
    bool IsRunningInStabilityTest() override;
    void DumpSysState(
        const std::string& args, std::vector<std::string>& info, bool isClient, bool isUserID, int UserID) override;
    int StartUserTest(const Want& want, const sptr<IRemoteObject>& observer) override;
    int FinishUserTest(
        const std::string& msg, const int64_t& resultCode, const std::string& bundleName) override;
    int GetTopAbility(sptr<IRemoteObject>& token) override;
    AppExecFwk::ElementName GetTopAbility(bool isNeedLocalDeviceId = true) override;
    int DelegatorDoAbilityForeground(const sptr<IRemoteObject>& token) override;
    int DelegatorDoAbilityBackground(const sptr<IRemoteObject>& token) override;
    int32_t ReportDrawnCompleted(const sptr<IRemoteObject>& callerToken) override;
    int DoAbilityForeground(const sptr<IRemoteObject>& token, uint32_t flag);
    int DoAbilityBackground(const sptr<IRemoteObject>& token, uint32_t flag);
    int32_t GetMissionIdByToken(const sptr<IRemoteObject>& token);
    void GetAbilityTokenByCalleeObj(const sptr<IRemoteObject> &callStub, sptr<IRemoteObject> &token);
    int32_t SetApplicationAutoStartupByEDM(const AutoStartupInfo &info, bool flag, bool isHiddenStart = false) override
    {
        return 0;
    }

    int32_t CancelApplicationAutoStartupByEDM(const AutoStartupInfo &info, bool flag) override
    {
        return 0;
    }
#ifdef ABILITY_COMMAND_FOR_TEST
    int ForceTimeoutForTest(const std::string& abilityName, const std::string& state) override;
#endif
#ifdef SUPPORT_GRAPHICS
    MOCK_METHOD2(SetMissionLabel, int(const sptr<IRemoteObject>& token, const std::string& label));
    int SetMissionIcon(
        const sptr<IRemoteObject>& token, const std::shared_ptr<OHOS::Media::PixelMap>& icon) override;
    int RegisterWindowManagerServiceHandler(const sptr<IWindowManagerServiceHandler>& handler,
        bool animationEnabled = true);
    void CompleteFirstFrameDrawing(const sptr<IRemoteObject>& abilityToken) override {}
#endif
    MOCK_METHOD2(IsValidMissionIds, int32_t(const std::vector<int32_t>&, std::vector<MissionValidResult>&));
    MOCK_METHOD1(RegisterAppDebugListener, int32_t(sptr<AppExecFwk::IAppDebugListener> listener));
    MOCK_METHOD1(UnregisterAppDebugListener, int32_t(sptr<AppExecFwk::IAppDebugListener> listener));
    MOCK_METHOD2(AttachAppDebug, int32_t(const std::string &bundleName, bool isDebugFromLocal));
    MOCK_METHOD2(DetachAppDebug, int32_t(const std::string &bundleName, bool isDebugFromLocal));
    MOCK_METHOD3(ExecuteIntent, int32_t(uint64_t key, const sptr<IRemoteObject> &callerToken,
        const InsightIntentExecuteParam &param));
    MOCK_METHOD3(ExecuteInsightIntentDone, int32_t(const sptr<IRemoteObject> &token, uint64_t intentId,
        const InsightIntentExecuteResult &result));
    MOCK_METHOD5(StartAbilityWithSpecifyTokenId, int(const Want& want, const sptr<IRemoteObject>& callerToken,
        uint32_t specifyTokenId, int32_t userId, int requestCode));
public:
    std::string powerState_;
    static bool finishFlag_;
    static TESTCASE_BRANCH testcaseBranch_;
};
}  // namespace AAFwk
}  // namespace OHOS

#endif  // MOCK_OHOS_ABILITY_RUNTIME_MOCK_ABILITY_DELEGATOR_STUB_H
