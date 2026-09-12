/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#ifndef MOCK_OHOS_ABILITY_RUNTIME_MOCK_ABILITY_MGR_SERVICE_H
#define MOCK_OHOS_ABILITY_RUNTIME_MOCK_ABILITY_MGR_SERVICE_H

#include "gmock/gmock.h"
#include "semaphore_ex.h"
#include "ability_manager_stub.h"

namespace OHOS {
namespace AAFwk {
class MockAbilityMgrService : public AbilityManagerStub {
public:
    MOCK_METHOD4(StartAbility, int(const Want& want, int32_t userId, int requestCode, uint64_t specifiedFullTokenId));
    MOCK_METHOD5(StartAbility, int(const Want& want, const sptr<IRemoteObject>& callerToken, int32_t userId,
        int requestCode, uint64_t specifiedFullTokenId));
    MOCK_METHOD5(StartAbility, int(const Want& want, const AbilityStartSetting& abilityStartSetting,
        const sptr<IRemoteObject>& callerToken, int32_t userId, int requestCode));
    MOCK_METHOD4(StartAbilityByInsightIntent, int32_t(const Want& want, const sptr<IRemoteObject>& callerToken,
        uint64_t intentId, int32_t userId));
    MOCK_METHOD5(StartAbilityAsCaller, int(const Want& want, const sptr<IRemoteObject>& callerToken,
        sptr<IRemoteObject> asCallerSourceToken, int32_t userId, int requestCode));
    MOCK_METHOD6(StartAbilityAsCaller, int(const Want &want, const StartOptions &startOptions,
        const sptr<IRemoteObject> &callerToken, sptr<IRemoteObject> asCallerSourceToken,
        int32_t userId, int requestCode));
    MOCK_METHOD3(TerminateAbility, int(const sptr<IRemoteObject>& token, int resultCode, const Want* resultWant));
    MOCK_METHOD4(ConnectAbility, int(const Want& want, const sptr<IAbilityConnection>& connect,
        const sptr<IRemoteObject>& callerToken, int32_t userId));
    MOCK_METHOD1(DisconnectAbility, int(sptr<IAbilityConnection> connect));
    MOCK_METHOD3(AcquireDataAbility, sptr<IAbilityScheduler>(const Uri&, bool, const sptr<IRemoteObject>&));
    MOCK_METHOD2(ReleaseDataAbility, int(sptr<IAbilityScheduler>, const sptr<IRemoteObject>&));
    MOCK_METHOD2(AttachAbilityThread, int(const sptr<IAbilityScheduler>& scheduler,
        const sptr<IRemoteObject>& token));
    MOCK_METHOD3(AbilityTransitionDone, int(const sptr<IRemoteObject>& token, int state, const PacMap& saveData));
    MOCK_METHOD2(
        ScheduleConnectAbilityDone, int(const sptr<IRemoteObject>& token, const sptr<IRemoteObject>& remoteObject));
    MOCK_METHOD1(ScheduleDisconnectAbilityDone, int(const sptr<IRemoteObject>& token));
    MOCK_METHOD1(ScheduleCommandAbilityDone, int(const sptr<IRemoteObject>& token));
    MOCK_METHOD4(ScheduleCommandAbilityWindowDone, int(const sptr<IRemoteObject> &token,
        const sptr<SessionInfo> &sessionInfo, WindowCommand winCmd, AbilityCommand abilityCmd));
    void DumpState(const std::string& args, std::vector<std::string>& state)
    {
        GTEST_LOG_(INFO) << "MockAbilityMgrService::DumpState called";
        DumpStateCalled_ = true;
        EXPECT_TRUE(DumpStateCalled_);
        return;
    }
    virtual int32_t GetForegroundUIAbilities(std::vector<AppExecFwk::AbilityStateData> &list)
    {
        return 0;
    }
    MOCK_METHOD3(StopServiceAbility, int(const Want&, int32_t userId, const sptr<IRemoteObject> &token));
    MOCK_METHOD4(OnRemoteRequest, int(uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option));

    MOCK_METHOD4(KillProcess, int(const std::string& bundleName, const bool clearPageStack, int32_t appIndex,
        const std::string& reason));
    MOCK_METHOD2(UninstallApp, int(const std::string& bundleName, int32_t uid));
    MOCK_METHOD3(UninstallApp, int32_t(const std::string& bundleName, int32_t uid, int32_t appIndex));
    MOCK_METHOD1(TerminateAbilityByRecordId, int(const int64_t recordId));

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
    MOCK_METHOD1(GetMissionIdByToken, int32_t(const sptr<IRemoteObject>& token));
    MOCK_METHOD2(GetAbilityTokenByCalleeObj, void(const sptr<IRemoteObject> &callStub, sptr<IRemoteObject> &token));
    MOCK_METHOD1(GetAbilityRunningInfos, int(std::vector<AbilityRunningInfo>& info));
    MOCK_METHOD2(GetExtensionRunningInfos, int(int upperLimit, std::vector<ExtensionRunningInfo>& info));
    MOCK_METHOD1(GetProcessRunningInfos, int(std::vector<AppExecFwk::RunningProcessInfo>& info));
    MOCK_METHOD2(AcquireShareData, int32_t(const int32_t &missionId, const sptr<IAcquireShareDataCallback> &shareData));
    MOCK_METHOD4(ShareDataDone, int32_t(const sptr<IRemoteObject> &token,
        const int32_t &resultCode, const int32_t &uniqueId, WantParams &wantParam));
    virtual int SetAbilityController(const sptr<AppExecFwk::IAbilityController>& abilityController,
        bool imAStabilityTest) override
    {
        return 0;
    }

    virtual bool IsRunningInStabilityTest() override
    {
        return true;
    }

    void CallRequestDone(const sptr<IRemoteObject>& token, const sptr<IRemoteObject>& callStub) override
    {
        return;
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

    int32_t SetApplicationAutoStartupByEDM(const AutoStartupInfo &info, bool flag, bool isHiddenStart = false) override
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
    MOCK_METHOD4(ExecuteUIAbilityForegroundIntentWithSpecifyTokenId,
    int32_t(const Want &want, const sptr<IRemoteObject> &callerAbilityToken,
        const InsightIntentExecuteLiteParam &param, uint64_t specifiedFullTokenId));
    MOCK_METHOD3(ExecuteInsightIntentDone, int32_t(const sptr<IRemoteObject> &token, uint64_t intentId,
        const InsightIntentExecuteResult &result));
    MOCK_METHOD5(StartAbilityWithSpecifyTokenId, int(const Want& want, const sptr<IRemoteObject>& callerToken,
        uint32_t specifyTokenId, int32_t userId, int requestCode));

private:
    bool DumpStateCalled_ = false;
    Semaphore sem_;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // MOCK_OHOS_ABILITY_RUNTIME_MOCK_ABILITY_MGR_SERVICE_H
