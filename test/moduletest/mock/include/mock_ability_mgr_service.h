/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#ifndef MODULETEST_OHOS_ABILITY_RUNTIME_MOCK_ABILITY_MGR_SERVICE_H
#define MODULETEST_OHOS_ABILITY_RUNTIME_MOCK_ABILITY_MGR_SERVICE_H

#include "gmock/gmock.h"
#include "semaphore_ex.h"
#include "ability_manager_stub.h"

namespace OHOS {
namespace AAFwk {
class MockAbilityMgrService : public AbilityManagerStub {
public:
    MOCK_METHOD3(StartAbility, int(const Want& want, int32_t userId, int requestCode));
    MOCK_METHOD4(StartAbility, int(const Want& want, const sptr<IRemoteObject>& callerToken,
        int32_t userId, int requestCode));
    MOCK_METHOD5(StartAbilityAsCaller, int(const Want& want, const sptr<IRemoteObject>& callerToken,
        sptr<IRemoteObject> asCallerSourceToken, int32_t userId, int requestCode));
    MOCK_METHOD6(StartAbilityAsCaller, int(const Want &want, const StartOptions &startOptions,
        const sptr<IRemoteObject> &callerToken, sptr<IRemoteObject> asCallerSourceToken,
        int32_t userId, int requestCode));
    MOCK_METHOD3(TerminateAbility, int(const sptr<IRemoteObject>& token, int resultCode, const Want* resultWant));
    virtual int CloseAbility(const sptr<IRemoteObject>& token, int resultCode = DEFAULT_INVAL_VALUE,
        const Want* resultWant = nullptr) override
    {
        return 0;
    }

    virtual int MinimizeAbility(const sptr<IRemoteObject>& token, bool fromUser = false) override
    {
        return 0;
    }
    MOCK_METHOD4(ConnectAbility, int(const Want& want, const sptr<IAbilityConnection>& connect,
        const sptr<IRemoteObject>& callerToken, int32_t userId));
    MOCK_METHOD1(DisconnectAbility, int(sptr<IAbilityConnection> connect));
    MOCK_METHOD3(AcquireDataAbility, sptr<IAbilityScheduler>(const Uri&, bool, const sptr<IRemoteObject>&));
    MOCK_METHOD2(ReleaseDataAbility, int(sptr<IAbilityScheduler>, const sptr<IRemoteObject>&));
    MOCK_METHOD2(AttachAbilityThread, int(const sptr<IAbilityScheduler>& scheduler, const sptr<IRemoteObject>& token));
    MOCK_METHOD3(AbilityTransitionDone, int(const sptr<IRemoteObject>& token, int state, const PacMap&));
    MOCK_METHOD2(
        ScheduleConnectAbilityDone, int(const sptr<IRemoteObject>& token, const sptr<IRemoteObject>& remoteObject));
    MOCK_METHOD1(ScheduleDisconnectAbilityDone, int(const sptr<IRemoteObject>& token));
    MOCK_METHOD1(ScheduleCommandAbilityDone, int(const sptr<IRemoteObject>&));
    MOCK_METHOD4(ScheduleCommandAbilityWindowDone, int(const sptr<IRemoteObject> &token,
        const sptr<SessionInfo> &sessionInfo, WindowCommand winCmd, AbilityCommand abilityCmd));
    MOCK_METHOD2(DumpState, void(const std::string& args, std::vector<std::string>& state));
    MOCK_METHOD5(
        DumpSysState,
        void(const std::string& args, std::vector<std::string>& info, bool isClient, bool isUserID, int UserID));
    MOCK_METHOD3(StopServiceAbility, int(const Want&, int32_t userId, const sptr<IRemoteObject> &token));
    MOCK_METHOD1(GetMissionIdByToken, int32_t(const sptr<IRemoteObject>& token));
    MOCK_METHOD2(GetAbilityTokenByCalleeObj, void(const sptr<IRemoteObject> &callStub, sptr<IRemoteObject> &token));
    MOCK_METHOD3(KillProcess, int(const std::string&, const bool clearPageStack, int32_t appIndex));
    MOCK_METHOD2(UninstallApp, int(const std::string&, int32_t));
    MOCK_METHOD3(UninstallApp, int32_t(const std::string&, int32_t, int32_t));
    MOCK_METHOD4(OnRemoteRequest, int(uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option));
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
    MOCK_METHOD5(StartAbility, int(const Want& want, const AbilityStartSetting& abilityStartSetting,
        const sptr<IRemoteObject>& callerToken, int32_t userId, int requestCode));
    MOCK_METHOD4(StartAbilityByInsightIntent, int32_t(const Want& want, const sptr<IRemoteObject>& callerToken,
        uint64_t intentId, int32_t userId));
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
    MOCK_METHOD2(SetMissionContinueState, int(const sptr<IRemoteObject>& token, const AAFwk::ContinueState& state));
    MOCK_METHOD2(SetMissionLabel, int(const sptr<IRemoteObject>& token, const std::string& label));
    MOCK_METHOD2(SetMissionIcon, int(const sptr<IRemoteObject>& token,
        const std::shared_ptr<OHOS::Media::PixelMap>& icon));

    MOCK_METHOD2(GetWantSenderInfo, int(const sptr<IWantSender>& target, std::shared_ptr<WantSenderInfo>& info));

    MOCK_METHOD1(GetAbilityRunningInfos, int(std::vector<AbilityRunningInfo>& info));
    MOCK_METHOD2(GetExtensionRunningInfos, int(int upperLimit, std::vector<ExtensionRunningInfo>& info));
    MOCK_METHOD1(GetProcessRunningInfos, int(std::vector<AppExecFwk::RunningProcessInfo>& info));
    MOCK_METHOD4(StartAbilityByCall,
        int(const Want&, const sptr<IAbilityConnection>&, const sptr<IRemoteObject>&, int32_t));
    MOCK_METHOD2(AcquireShareData, int32_t(const int32_t &missionId, const sptr<IAcquireShareDataCallback> &shareData));
    MOCK_METHOD4(ShareDataDone, int32_t(const sptr<IRemoteObject> &token,
        const int32_t &resultCode, const int32_t &uniqueId, WantParams &wantParam));

    virtual int StartAbility(
        const Want& want,
        const StartOptions& startOptions,
        const sptr<IRemoteObject>& callerToken,
        int32_t userId = DEFAULT_INVAL_VALUE,
        int requestCode = DEFAULT_INVAL_VALUE)
    {
        return 0;
    }

    virtual int StartUser(int userId, sptr<IUserCallback> callback, bool isAppRecovery) override
    {
        return 0;
    }

    virtual int StopUser(int userId, const sptr<IUserCallback>& callback) override
    {
        return 0;
    }

    virtual int LogoutUser(int32_t userId, sptr<IUserCallback> callback = nullptr) override
    {
        return 0;
    }

    virtual int StartSyncRemoteMissions(const std::string& devId, bool fixConflict, int64_t tag) override
    {
        return 0;
    }
    virtual int32_t GetForegroundUIAbilities(std::vector<AppExecFwk::AbilityStateData> &list)
    {
        return 0;
    }
    virtual int StopSyncRemoteMissions(const std::string& devId) override
    {
        return 0;
    }
    virtual int RegisterMissionListener(const std::string& deviceId,
        const sptr<IRemoteMissionListener>& listener) override
    {
        return 0;
    }
    virtual int UnRegisterMissionListener(const std::string& deviceId,
        const sptr<IRemoteMissionListener>& listener) override
    {
        return 0;
    }
    void CallRequestDone(const sptr<IRemoteObject> &token, const sptr<IRemoteObject> &callStub) override
    {
        return;
    }
    virtual int ReleaseCall(const sptr<IAbilityConnection> &connect,
        const AppExecFwk::ElementName &element) override
    {
        return 0;
    }
    virtual int GetMissionSnapshot(const std::string& deviceId, int32_t missionId,
        MissionSnapshot& snapshot, bool isLowResolution)
    {
        return 0;
    }
    virtual int RegisterSnapshotHandler(const sptr<ISnapshotHandler>& handler)
    {
        return 0;
    }

    virtual int RegisterWindowManagerServiceHandler(const sptr<IWindowManagerServiceHandler>& handler,
        bool animationEnabled = true) override
    {
        return 0;
    }

    virtual void CompleteFirstFrameDrawing(const sptr<IRemoteObject>& abilityToken) override {}

    virtual int SetAbilityController(const sptr<AppExecFwk::IAbilityController>& abilityController,
        bool imAStabilityTest) override
    {
        return 0;
    }

    virtual bool IsRunningInStabilityTest() override
    {
        return true;
    }

    virtual int StartUserTest(const Want& want, const sptr<IRemoteObject>& observer) override
    {
        return 0;
    }

    virtual int FinishUserTest(
        const std::string& msg, const int64_t& resultCode, const std::string& bundleName) override
    {
        return 0;
    }

    virtual int GetTopAbility(sptr<IRemoteObject>& token) override
    {
        return 0;
    }

    virtual int DelegatorDoAbilityForeground(const sptr<IRemoteObject>& token) override
    {
        return 0;
    }

    virtual int DelegatorDoAbilityBackground(const sptr<IRemoteObject>& token) override
    {
        return 0;
    }

    int32_t ReportDrawnCompleted(const sptr<IRemoteObject>& callerToken) override
    {
        return 0;
    }

    void Wait()
    {
        sem_.Wait();
    }

    int Post()
    {
        sem_.Post();
        return 0;
    }

    void PostVoid()
    {
        sem_.Post();
    }

    int32_t SetApplicationAutoStartupByEDM(const AutoStartupInfo &info, bool flag) override
    {
        return 0;
    }

    int32_t CancelApplicationAutoStartupByEDM(const AutoStartupInfo &info, bool flag) override
    {
        return 0;
    }

#ifdef ABILITY_COMMAND_FOR_TEST
    virtual int ForceTimeoutForTest(const std::string& abilityName, const std::string& state) override
    {
        return 0;
    }
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

private:
    Semaphore sem_;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // MODULETEST_OHOS_ABILITY_RUNTIME_MOCK_ABILITY_MGR_SERVICE_H
