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

#ifndef MODULETEST_OHOS_ABILITY_RUNTIME_MOCK_ABILITY_MGR_HOST_H
#define MODULETEST_OHOS_ABILITY_RUNTIME_MOCK_ABILITY_MGR_HOST_H

#include <iremote_object.h>
#include <iremote_stub.h>

#include "ability_manager_interface.h"

namespace OHOS {
namespace AppExecFwk {
using namespace AAFwk;
class mock_ability_mgr_host {
public:
    mock_ability_mgr_host() = default;
    virtual ~mock_ability_mgr_host() = default;
};

class MockAbilityMgrStub : public IRemoteStub<AAFwk::IAbilityManager> {
public:
    using Uri = OHOS::Uri;
    explicit MockAbilityMgrStub() = default;
    virtual ~MockAbilityMgrStub() = default;

    virtual int StartAbility(const AAFwk::Want& want, int requestCode = -1) override
    {
        return 0;
    }
    virtual int StartAbility(
        const AAFwk::Want& want, const sptr<IRemoteObject>& callerToken, int requestCode = -1) override
    {
        return 0;
    }
    virtual int StartAbility(const Want& want, const sptr<IRemoteObject>& callerToken,
        int requestCode, int requestUid) override
    {
        return 0;
    }

    int32_t StartAbilityByInsightIntent(const Want &want, const sptr<IRemoteObject> &callerToken,
        uint64_t intentId, int32_t userId) override
    {
        return 0;
    }

    virtual int StartAbilityAsCaller(const Want& want, const sptr<IRemoteObject>& callerToken,
        const sptr<IRemoteObject>& asCallerSourceToken, int32_t userId = DEFAULT_INVAL_VALUE,
        int requestCode = -1) override
    {
        return 0;
    }
    virtual int StartAbilityAsCaller(const Want& want, const StartOptions& startOptions,
        const sptr<IRemoteObject>& callerToken, const sptr<IRemoteObject>& asCallerSourceToken,
        int32_t userId = DEFAULT_INVAL_VALUE, int requestCode = DEFAULT_INVAL_VALUE) override
    {
        return 0;
    }
    virtual int TerminateAbility(
        const sptr<IRemoteObject>& token, int resultCode, const AAFwk::Want* resultWant = nullptr) override
    {
        return 0;
    }
    virtual int CloseAbility(const sptr<IRemoteObject>& token, int resultCode = DEFAULT_INVAL_VALUE,
        const Want* resultWant = nullptr) override
    {
        return 0;
    }
    virtual int MinimizeAbility(const sptr<IRemoteObject>& token, bool fromUser) override
    {
        return 0;
    }
    virtual int ConnectAbility(const AAFwk::Want& want, const sptr<AAFwk::IAbilityConnection>& connect,
        const sptr<IRemoteObject>& callerToken) override
    {
        return 0;
    }
    virtual int DisconnectAbility(sptr<AAFwk::IAbilityConnection> connect) override
    {
        return 0;
    }
    virtual sptr<AAFwk::IAbilityScheduler> AcquireDataAbility(
        const Uri& uri, bool tryBind, const sptr<IRemoteObject>& callerToken) override
    {
        return nullptr;
    }
    virtual int ReleaseDataAbility(
        sptr<AAFwk::IAbilityScheduler> dataAbilityScheduler, const sptr<IRemoteObject>& callerToken) override
    {
        return 0;
    }
    virtual int AttachAbilityThread(
        const sptr<AAFwk::IAbilityScheduler>& scheduler, const sptr<IRemoteObject>& token) override
    {
        return 0;
    }
    virtual int AbilityTransitionDone(const sptr<IRemoteObject>& token, int state, const PacMap& saveData) override
    {
        return 0;
    }
    virtual int ScheduleConnectAbilityDone(
        const sptr<IRemoteObject>& token, const sptr<IRemoteObject>& remoteObject) override
    {
        return 0;
    }
    virtual int ScheduleDisconnectAbilityDone(const sptr<IRemoteObject>& token) override
    {
        return 0;
    }
    virtual int ScheduleCommandAbilityDone(const sptr<IRemoteObject>& token) override
    {
        return 0;
    }
    virtual int ScheduleCommandAbilityWindowDone(
        const sptr<IRemoteObject> &token,
        const sptr<AAFwk::SessionInfo> &sessionInfo,
        AAFwk::WindowCommand winCmd,
        AAFwk::AbilityCommand abilityCmd) override
    {
        return 0;
    }
    virtual void DumpState(const std::string& args, std::vector<std::string>& state) override
    {
        return;
    }
    virtual int StopServiceAbility(const AAFwk::Want& want, const sptr<IRemoteObject>& callerToken,
        const sptr<IRemoteObject>& token = nullptr) override
    {
        return 0;
    }

    virtual int KillProcess(const std::string& bundleName, bool clearPageStack = false, int32_t appIndex = 0) override
    {
        return 0;
    }
    virtual int UninstallApp(const std::string& bundleName, int32_t uid) override
    {
        return 0;
    }

    virtual sptr<AAFwk::IWantSender> GetWantSender(
        const WantSenderInfo& wantSenderInfo, const sptr<IRemoteObject>& callerToken, int32_t uid) override
    {
        return nullptr;
    }

    virtual int SendWantSender(sptr<AAFwk::IWantSender>& target, SenderInfo& senderInfo) override
    {
        return 0;
    }

    virtual void CancelWantSender(const sptr<AAFwk::IWantSender>& sender) override
    {}

    virtual int GetPendingWantUid(const sptr<AAFwk::IWantSender>& target) override
    {
        return 0;
    }

    virtual int GetPendingWantUserId(const sptr<AAFwk::IWantSender>& target) override
    {
        return 0;
    }

    virtual std::string GetPendingWantBundleName(const sptr<AAFwk::IWantSender>& target) override
    {
        return "";
    }

    virtual int GetPendingWantCode(const sptr<AAFwk::IWantSender>& target) override
    {
        return 0;
    }

    virtual int GetPendingWantType(const sptr<AAFwk::IWantSender>& target) override
    {
        return 0;
    }

    virtual void RegisterCancelListener(
        const sptr<AAFwk::IWantSender>& sender, const sptr<IWantReceiver>& receiver) override
    {}

    virtual void UnregisterCancelListener(
        const sptr<AAFwk::IWantSender>& sender, const sptr<IWantReceiver>& receiver) override
    {}

    virtual int GetPendingRequestWant(const sptr<IWantSender>& target, std::shared_ptr<Want>& want) override
    {
        return 0;
    }

    int StartAbility(const Want& want, const AbilityStartSetting& abilityStartSetting,
        const sptr<IRemoteObject>& callerToken, int requestCode) override
    {
        return 0;
    }

    int GetWantSenderInfo(const sptr<IWantSender>& target, std::shared_ptr<WantSenderInfo>& info) override
    {
        return 0;
    }
    int StartContinuation(const Want& want, const sptr<IRemoteObject>& abilityToken, int32_t status) override
    {
        return 0;
    }
    int NotifyContinuationResult(int32_t missionId, int32_t result) override
    {
        return 0;
    }
    int ContinueMission(const std::string& srcDeviceId, const std::string& dstDeviceId,
        int32_t missionId, const sptr<IRemoteObject>& callBack, AAFwk::WantParams& wantParams) override
    {
        return 0;
    }
    int ContinueAbility(const std::string& deviceId, int32_t missionId, uint32_t versionCode) override
    {
        return 0;
    }
    void NotifyCompleteContinuation(const std::string& deviceId, int32_t sessionId, bool isSuccess) override
    {}

    virtual int LockMissionForCleanup(int32_t missionId) override
    {
        return 0;
    }

    virtual int UnlockMissionForCleanup(int32_t missionId) override
    {
        return 0;
    }

    virtual int RegisterMissionListener(const sptr<IMissionListener>& listener) override
    {
        return 0;
    }

    virtual int UnRegisterMissionListener(const sptr<IMissionListener>& listener) override
    {
        return 0;
    }

    virtual int GetMissionInfos(const std::string& deviceId, int32_t numMax,
        std::vector<MissionInfo>& missionInfos) override
    {
        return 0;
    }

    virtual int GetMissionInfo(const std::string& deviceId, int32_t missionId,
        MissionInfo& missionInfo) override
    {
        return 0;
    }

    virtual int CleanMission(int32_t missionId) override
    {
        return 0;
    }

    virtual int CleanAllMissions() override
    {
        return 0;
    }

    virtual int MoveMissionToFront(int32_t missionId) override
    {
        return 0;
    }
    virtual int MoveMissionToFront(int32_t missionId, const StartOptions& startOptions) override
    {
        return 0;
    }
    virtual int StartSyncRemoteMissions(const std::string& devId, bool fixConflict, int64_t tag) override
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
    virtual int DoAbilityForeground(const sptr<IRemoteObject>& token, uint32_t flag) override
    {
        return 0;
    }
    virtual int DoAbilityBackground(const sptr<IRemoteObject>& token, uint32_t flag) override
    {
        return 0;
    }
    int32_t AcquireShareData(const int32_t &missionId, const sptr<IAcquireShareDataCallback> &shareData) override
    {
        return 0;
    }

    int32_t ShareDataDone(const sptr<IRemoteObject> &token,
        const int32_t &resultCode, const int32_t &uniqueId, WantParams &wantParam) override
    {
        return 0;
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
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // MODULETEST_OHOS_ABILITY_RUNTIME_MOCK_ABILITY_MGR_HOST_H
