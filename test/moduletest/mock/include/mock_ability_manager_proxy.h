/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef MODULETEST_OHOS_ABILITY_RUNTIME_MOCK_ABILITY_MANAGER_PROXY_H
#define MODULETEST_OHOS_ABILITY_RUNTIME_MOCK_ABILITY_MANAGER_PROXY_H

#include "gmock/gmock.h"
#include "ability_manager_interface.h"
#include "iremote_proxy.h"

namespace OHOS {
namespace AAFwk {
class MockAbilityManagerProxy : public IRemoteProxy<IAbilityManager> {
public:
    explicit MockAbilityManagerProxy(const sptr<IRemoteObject>& impl) : IRemoteProxy<IAbilityManager>(impl)
    {}
    virtual ~MockAbilityManagerProxy()
    {}

    MOCK_METHOD2(StartAbility, int(const Want& want, int requestCode));
    MOCK_METHOD3(StartAbility, int(const Want& want, const sptr<IRemoteObject>& callerToken, int requestCode));
    MOCK_METHOD5(StartAbilityAsCaller, int(const Want& want, const sptr<IRemoteObject>& callerToken,
        sptr<IRemoteObject> asCallerSourceToken, int32_t userId, int requestCode));
    MOCK_METHOD6(StartAbilityAsCaller, int(const Want &want, const StartOptions &startOptions,
        const sptr<IRemoteObject> &callerToken, sptr<IRemoteObject> asCallerSourceToken,
        int32_t userId, int requestCode));

    MOCK_METHOD3(TerminateAbility, int(const sptr<IRemoteObject>& token, int resultCode, const Want* resultWant));
    MOCK_METHOD3(ConnectAbility,
        int(const Want& want, const sptr<IAbilityConnection>& connect, const sptr<IRemoteObject>& callerToken));
    MOCK_METHOD1(DisconnectAbility, int(sptr<IAbilityConnection> connect));
    MOCK_METHOD3(AcquireDataAbility, sptr<IAbilityScheduler>(const Uri&, bool, const sptr<IRemoteObject>&));
    MOCK_METHOD2(ReleaseDataAbility, int(sptr<IAbilityScheduler>, const sptr<IRemoteObject>&));
    MOCK_METHOD2(AttachAbilityThread, int(const sptr<IAbilityScheduler>& scheduler, const sptr<IRemoteObject>& token));
    MOCK_METHOD2(AbilityTransitionDone, int(const sptr<IRemoteObject>& token, int state));
    MOCK_METHOD2(
        ScheduleConnectAbilityDone, int(const sptr<IRemoteObject>& token, const sptr<IRemoteObject>& remoteObject));
    MOCK_METHOD1(ScheduleDisconnectAbilityDone, int(const sptr<IRemoteObject>& token));
    MOCK_METHOD1(ScheduleCommandAbilityDone, int(const sptr<IRemoteObject>&));
    MOCK_METHOD4(ScheduleCommandAbilityWindowDone, int(const sptr<IRemoteObject> &token,
        const sptr<SessionInfo> &sessionInfo, WindowCommand winCmd, AbilityCommand abilityCmd));
    MOCK_METHOD2(DumpState, void(const std::string& args, std::vector<std::string>& state));
    MOCK_METHOD3(StopServiceAbility, int(const Want&, int32_t userId, const sptr<IRemoteObject> &token));
    MOCK_METHOD1(GetMissionIdByToken, int32_t(const sptr<IRemoteObject>& token));
    MOCK_METHOD4(KillProcess, int(const std::string&, const bool clearPageStack, int32_t appIndex,
        const std::string& reason));
    MOCK_METHOD2(UninstallApp, int(const std::string&, int32_t));
    MOCK_METHOD3(UninstallApp, int32_t(const std::string&, int32_t, int32_t));
    MOCK_METHOD4(OnRemoteRequest, int(uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option));
    MOCK_METHOD3(StartAbility, int(const Want& want, const sptr<IRemoteObject>& callerToken, int requestCode));
    MOCK_METHOD3(
        GetWantSender, sptr<IWantSender>(const WantSenderInfo& wantSenderInfo, const sptr<IRemoteObject>& callerToken,
        int32_t uid));
    MOCK_METHOD2(SendWantSender, int(sptr<IWantSender> target, SenderInfo& senderInfo));
    MOCK_METHOD1(CancelWantSender, void(const sptr<IWantSender>& sender));
    MOCK_METHOD1(GetPendingWantUid, int(const sptr<IWantSender>& target));
    MOCK_METHOD1(GetPendingWantBundleName, std::string(const sptr<IWantSender>& target));
    MOCK_METHOD1(GetPendingWantCode, int(const sptr<IWantSender>& target));
    MOCK_METHOD1(GetPendingWantType, int(const sptr<IWantSender>& target));
    MOCK_METHOD2(RegisterCancelListener, void(const sptr<IWantSender>& sender, const sptr<IWantReceiver>& receiver));
    MOCK_METHOD2(UnregisterCancelListener, void(const sptr<IWantSender>& sender, const sptr<IWantReceiver>& receiver));
    MOCK_METHOD2(GetPendingRequestWant, int(const sptr<IWantSender>& target, std::shared_ptr<Want>& want));
    MOCK_METHOD2(GetPendingRequestWantFromProxy, int(const sptr<IWantSender>& target, std::shared_ptr<Want>& want));
    MOCK_METHOD1(GetPendingWantUserId, int(const sptr<IWantSender>& target));
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

public:
    int id_;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // MODULETEST_OHOS_ABILITY_RUNTIME_MOCK_ABILITY_MANAGER_PROXY_H
