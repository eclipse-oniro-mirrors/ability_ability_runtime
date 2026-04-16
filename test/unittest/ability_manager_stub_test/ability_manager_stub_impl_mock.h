/*
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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

#ifndef UNITTEST_OHOS_ABILITY_RUNTIME_ABILITY_MANAGER_STUB_IMPL_MOCK_H
#define UNITTEST_OHOS_ABILITY_RUNTIME_ABILITY_MANAGER_STUB_IMPL_MOCK_H
#include <gmock/gmock.h>
#include <iremote_object.h>
#include <iremote_stub.h>
#include "ability_manager_interface.h"
#define private public
#include "ability_manager_stub.h"
#undef private

namespace OHOS {
namespace AAFwk {
class AbilityManagerStubImplMock : public AbilityManagerStub {
public:
    const int ABILITY_STATE_FIRST_UID = 3000;
    const int ABILITY_STATE_SECOND_UID = 3001;
    const int ABILITY_STATE_FIRST_PID = 11291450;
    const int ABILITY_STATE_SECOND_PID = 11291506;
    AbilityManagerStubImplMock()
    {}
    virtual ~AbilityManagerStubImplMock()
    {}

    MOCK_METHOD5(StartAbility, int(const Want& want, const sptr<IRemoteObject>& callerToken,
        int32_t userId, int requestCode, uint64_t specifiedFullTokenId));
    MOCK_METHOD4(StartAbilityByInsightIntent, int32_t(const Want& want, const sptr<IRemoteObject>& callerToken,
        uint64_t intentId, int32_t userId));
    MOCK_METHOD5(StartAbilityAsCaller, int(const Want &want, const sptr<IRemoteObject> &callerToken,
        sptr<IRemoteObject> asCallerSourceToken, int32_t userId, int requestCode));
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
    MOCK_METHOD2(GetWantSenderInfo, int(const sptr<IWantSender>& target, std::shared_ptr<WantSenderInfo>& info));
    MOCK_METHOD2(SetMissionContinueState, int(const sptr<IRemoteObject>& token, const AAFwk::ContinueState& state));
    MOCK_METHOD2(SetMissionLabel, int(const sptr<IRemoteObject>& token, const std::string& label));
    MOCK_METHOD2(SetMissionIcon, int(const sptr<IRemoteObject>& token,
        const std::shared_ptr<OHOS::Media::PixelMap>& icon));
    MOCK_METHOD3(StartContinuation, int(const Want& want, const sptr<IRemoteObject>& abilityToken, int32_t status));
    MOCK_METHOD2(NotifyContinuationResult, int(int32_t missionId, int32_t result));
    MOCK_METHOD5(ContinueMission, int(const std::string& srcDeviceId, const std::string& dstDeviceId,
        int32_t missionId, const sptr<IRemoteObject>& callBack, AAFwk::WantParams& wantParams));
    MOCK_METHOD3(ContinueAbility, int(const std::string& deviceId, int32_t missionId, uint32_t versionCode));
    MOCK_METHOD3(NotifyCompleteContinuation, void(const std::string& deviceId, int32_t sessionId, bool isSuccess));
    MOCK_METHOD1(GetAbilityRunningInfos, int(std::vector<AbilityRunningInfo>& info));
    MOCK_METHOD2(GetExtensionRunningInfos, int(int upperLimit, std::vector<ExtensionRunningInfo>& info));
    MOCK_METHOD1(GetProcessRunningInfos, int(std::vector<AppExecFwk::RunningProcessInfo>& info));
    MOCK_METHOD7(StartAbilityByCall,
        int(const Want&, const sptr<IAbilityConnection>&, const sptr<IRemoteObject>&, int32_t, bool, bool, bool));
    MOCK_METHOD2(AcquireShareData, int32_t(const int32_t &missionId, const sptr<IAcquireShareDataCallback> &shareData));
    MOCK_METHOD4(ShareDataDone, int32_t(const sptr<IRemoteObject> &token,
        const int32_t &resultCode, const int32_t &uniqueId, WantParams &wantParam));

    int InvokeSendRequest(uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option)
    {
        code_ = code;
        return 0;
    }

    int InvokeErrorSendRequest(uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option)
    {
        code_ = code;
        return UNKNOWN_ERROR;
    }

    int code_ = 0;

    virtual int StartAbility(
        const Want& want, int32_t userId = DEFAULT_INVAL_VALUE, int requestCode = -1, uint64_t specifiedFullTokenId = 0)
    {
        return 0;
    }

    int StartAbility(const Want& want,
        const AbilityStartSetting& abilityStartSetting,
        const sptr<IRemoteObject>& callerToken,
        int32_t userId = DEFAULT_INVAL_VALUE,
        int requestCode = DEFAULT_INVAL_VALUE)
    {
        return 0;
    }

    int StartAbility(
        const Want& want,
        const StartOptions& startOptions,
        const sptr<IRemoteObject>& callerToken,
        int32_t userId = DEFAULT_INVAL_VALUE,
        int requestCode = DEFAULT_INVAL_VALUE)
    {
        return 0;
    }

    int StartAbilityAsCaller(
        const Want& want,
        const StartOptions& startOptions,
        const sptr<IRemoteObject>& callerToken,
        sptr<IRemoteObject> asCallerSourceToken,
        int32_t userId = DEFAULT_INVAL_VALUE,
        int requestCode = DEFAULT_INVAL_VALUE)
    {
        return 0;
    }

    virtual int TerminateAbility(const sptr<IRemoteObject>& token, int resultCode, const Want* resultWant = nullptr)
    {
        return 0;
    }

    virtual int SendResultToAbility(int requestCode, int resultCode, Want& resultWant)
    {
        return 0;
    }

    int CloseAbility(const sptr<IRemoteObject>& token, int resultCode = DEFAULT_INVAL_VALUE,
        const Want* resultWant = nullptr) override
    {
        return 0;
    }
    int MinimizeAbility(const sptr<IRemoteObject>& token, bool fromUser) override
    {
        return 0;
    }

    virtual int ConnectAbility(
        const Want& want,
        const sptr<IAbilityConnection>& connect,
        const sptr<IRemoteObject>& callerToken,
        int32_t userId = DEFAULT_INVAL_VALUE)
    {
        return 0;
    }

    virtual int DisconnectAbility(sptr<IAbilityConnection> connect)
    {
        return 0;
    }

    sptr<IAbilityScheduler> AcquireDataAbility(
        const Uri& uri, bool tryBind, const sptr<IRemoteObject>& callerToken) override
    {
        return nullptr;
    }

    virtual int ReleaseDataAbility(sptr<IAbilityScheduler> dataAbilityScheduler, const sptr<IRemoteObject>& callerToken)
    {
        return 0;
    }

    virtual int AttachAbilityThread(const sptr<IAbilityScheduler>& scheduler, const sptr<IRemoteObject>& token)
    {
        return 0;
    }

    virtual int AbilityTransitionDone(const sptr<IRemoteObject>& token, int state, const PacMap& saveData)
    {
        return 0;
    }

    virtual int ScheduleConnectAbilityDone(const sptr<IRemoteObject>& token, const sptr<IRemoteObject>& remoteObject)
    {
        return 0;
    }

    virtual int ScheduleDisconnectAbilityDone(const sptr<IRemoteObject>& token)
    {
        return 0;
    }

    virtual int ScheduleCommandAbilityDone(const sptr<IRemoteObject>& token)
    {
        return 0;
    }

    virtual int ScheduleCommandAbilityWindowDone(
        const sptr<IRemoteObject> &token,
        const sptr<SessionInfo> &sessionInfo,
        WindowCommand winCmd,
        AbilityCommand abilityCmd)
    {
        return 0;
    }

    virtual void DumpState(const std::string& args, std::vector<std::string>& state)
    {}

    virtual void DumpSysState(
        const std::string& args, std::vector<std::string>& info, bool isClient, bool isUserID, int UserID)
    {}

    virtual int StopServiceAbility(const Want& want, int32_t userId = DEFAULT_INVAL_VALUE,
        const sptr<IRemoteObject> &token = nullptr)
    {
        return 0;
    }

    virtual int KillProcess(const std::string& bundleName, bool clearPageStack = false, int32_t appIndex = 0,
        const std::string& reason = "KillProcess")
    {
        return 0;
    }

    virtual int UninstallApp(const std::string& bundleName, int32_t uid)
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
    int RegisterMissionListener(const sptr<IMissionListener>& listener) override
    {
        return 0;
    }
    int UnRegisterMissionListener(const sptr<IMissionListener>& listener) override
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
    int MoveMissionToFront(int32_t missionId, const StartOptions& startOptions) override
    {
        return 0;
    }
    int MoveMissionsToForeground(const std::vector<int32_t>& missionIds, int32_t topMissionId) override
    {
        return 0;
    }
    int MoveMissionsToBackground(const std::vector<int32_t>& missionIds, std::vector<int32_t>& result) override
    {
        return 0;
    }
    int32_t GetMissionIdByToken(const sptr<IRemoteObject>& token) override
    {
        return 0;
    }
    void GetAbilityTokenByCalleeObj(const sptr<IRemoteObject> &callStub, sptr<IRemoteObject> &token) override
    {
        return;
    }
    int GetMissionInfos(const std::string& deviceId, int32_t numMax,
        std::vector<MissionInfo>& missionInfos) override
    {
        return 0;
    }
    int GetMissionInfo(const std::string& deviceId, int32_t missionId,
        MissionInfo& missionInfo) override
    {
        return 0;
    }
    int StartUser(int userId, uint64_t displayId, sptr<IUserCallback> callback, bool isAppRecovery) override
    {
        return 0;
    }
    int StopUser(int userId, const sptr<IUserCallback>& callback) override
    {
        return 0;
    }

    int LogoutUser(int32_t userId, sptr<IUserCallback> callback = nullptr) override
    {
        return 0;
    }

    int StartSyncRemoteMissions(const std::string& devId, bool fixConflict, int64_t tag) override
    {
        return 0;
    }
    int StopSyncRemoteMissions(const std::string& devId) override
    {
        return 0;
    }
    int RegisterMissionListener(const std::string& deviceId,
        const sptr<IRemoteMissionListener>& listener) override
    {
        return 0;
    }
    int UnRegisterMissionListener(const std::string& deviceId,
        const sptr<IRemoteMissionListener>& listener) override
    {
        return 0;
    }
    void CallRequestDone(const sptr<IRemoteObject>& token, const sptr<IRemoteObject>& callStub) override
    {
        return;
    }
    int ReleaseCall(const sptr<IAbilityConnection>& connect,
        const AppExecFwk::ElementName& element) override
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

    int RegisterWindowManagerServiceHandler(const sptr<IWindowManagerServiceHandler>& handler,
        bool animationEnabled = true) override
    {
        return 0;
    }

    void CompleteFirstFrameDrawing(const sptr<IRemoteObject>& abilityToken) override {}

    int SetAbilityController(const sptr<AppExecFwk::IAbilityController>& abilityController,
        bool imAStabilityTest) override
    {
        return 0;
    }

    bool IsRunningInStabilityTest() override
    {
        return true;
    }

    int StartUserTest(const Want& want, const sptr<IRemoteObject>& observer) override
    {
        return 0;
    }

    int FinishUserTest(
        const std::string& msg, const int64_t& resultCode, const std::string& bundleName) override
    {
        return 0;
    }

    int GetTopAbility(sptr<IRemoteObject>& token) override
    {
        return 0;
    }

    int DelegatorDoAbilityForeground(const sptr<IRemoteObject>& token) override
    {
        return 0;
    }

    int DelegatorDoAbilityBackground(const sptr<IRemoteObject>& token) override
    {
        return 0;
    }

    int32_t ReportDrawnCompleted(const sptr<IRemoteObject>& callerToken) override
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
        AppExecFwk::AbilityStateData abilityStateDataOne;
        abilityStateDataOne.bundleName = "com.example.getforegrounduiabilitiesone";
        abilityStateDataOne.moduleName = "entry";
        abilityStateDataOne.abilityName = "EntryAbility";
        abilityStateDataOne.abilityState = AbilityState::FOREGROUND;
        abilityStateDataOne.pid = ABILITY_STATE_FIRST_UID;
        abilityStateDataOne.uid = ABILITY_STATE_FIRST_PID;
        abilityStateDataOne.abilityType = static_cast<int32_t>(AppExecFwk::AbilityType::PAGE);
        abilityStateDataOne.appCloneIndex = 0;
        list.emplace_back(abilityStateDataOne);

        AppExecFwk::AbilityStateData abilityStateDataTwo;
        abilityStateDataTwo.bundleName = "com.example.getforegrounduiabilitiestwo";
        abilityStateDataTwo.moduleName = "entry";
        abilityStateDataTwo.abilityName = "EntryAbility";
        abilityStateDataTwo.abilityState = AbilityState::INACTIVE;
        abilityStateDataTwo.pid = ABILITY_STATE_SECOND_UID;
        abilityStateDataTwo.uid = ABILITY_STATE_SECOND_PID;
        abilityStateDataOne.appCloneIndex = 0;
        abilityStateDataTwo.abilityType = static_cast<int32_t>(AppExecFwk::AbilityType::PAGE);
        list.emplace_back(abilityStateDataTwo);
        return ERR_OK;
    }

    int32_t UpdateKioskApplicationList(const std::vector<std::string> &appList) override
    {
        return ERR_OK;
    }

    int32_t EnterKioskMode(sptr<IRemoteObject> callerToken) override
    {
        return ERR_OK;
    }

    int32_t ExitKioskMode(sptr<IRemoteObject> callerToken) override
    {
        return ERR_OK;
    }

    int32_t GetKioskStatus(AAFwk::KioskStatus &kioskStatus) override
    {
        return ERR_OK;
    }

    int32_t SetAppServiceExtensionKeepAlive(const std::string &bundleName, bool flag) override
    {
        return ERR_OK;
    }

    int32_t QueryKeepAliveAppServiceExtensions(std::vector<KeepAliveInfo> &list) override
    {
        return ERR_OK;
    }

#ifdef ABILITY_COMMAND_FOR_TEST
    int ForceTimeoutForTest(const std::string& abilityName, const std::string& state) override
    {
        return 0;
    }
#endif
    MOCK_METHOD5(StartAbilityWithSpecifyTokenId, int(const Want& want, const sptr<IRemoteObject>& callerToken,
        uint32_t specifyTokenId, int32_t userId, int requestCode));
    MOCK_METHOD2(IsValidMissionIds, int32_t(const std::vector<int32_t>&, std::vector<MissionValidResult>&));
    MOCK_METHOD2(PrepareTerminateAbilityBySCB, int32_t(const sptr<SessionInfo> &sessionInfo, bool &isPrepareTerminate));
    MOCK_METHOD1(RegisterAppDebugListener, int32_t(sptr<AppExecFwk::IAppDebugListener> listener));
    MOCK_METHOD1(UnregisterAppDebugListener, int32_t(sptr<AppExecFwk::IAppDebugListener> listener));
    MOCK_METHOD2(AttachAppDebug, int32_t(const std::string &bundleName, bool isDebugFromLocal));
    MOCK_METHOD2(DetachAppDebug, int32_t(const std::string &bundleName, bool isDebugFromLocal));
    MOCK_METHOD1(IsAbilityControllerStart, bool(const Want& want));
    MOCK_METHOD3(ExecuteIntent, int32_t(uint64_t key, const sptr<IRemoteObject> &callerToken,
        const InsightIntentExecuteParam &param));
    MOCK_METHOD3(ExecuteInsightIntentDone, int32_t(const sptr<IRemoteObject> &token, uint64_t intentId,
        const InsightIntentExecuteResult &result));
    MOCK_METHOD3(GetUIExtensionRootHostInfo, int32_t(const sptr<IRemoteObject> token, UIExtensionHostInfo &hostInfo,
        int32_t userId));
    MOCK_METHOD1(GetAutoStartupStatusForSelf, int32_t(bool &isAutoStartEnabled));
    MOCK_METHOD4(StartAbilityByOEExt, int32_t(const Want&, sptr<IRemoteObject>, int32_t, const std::string&));

int32_t GetUserLockedBundleList(int32_t userId, std::unordered_set<std::string> &userLockedBundleList) override
{
    userLockedBundleList.clear();
    userLockedBundleList.insert("com.ohos.test");
    return ERR_OK;
}
};
}  // namespace AAFwk
}  // namespace OHOS

#endif
