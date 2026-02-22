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

#ifndef MOCK_OHOS_ABILITY_RUNTIME_MOCK_AMS_MGR_SCHEDULER_H
#define MOCK_OHOS_ABILITY_RUNTIME_MOCK_AMS_MGR_SCHEDULER_H

#include "gmock/gmock.h"
#include "ams_mgr_scheduler.h"

namespace OHOS {
namespace AbilityRuntime {
struct LoadParam;
}
namespace AppExecFwk {
class MockAmsMgrScheduler : public AmsMgrStub {
public:
    MOCK_METHOD4(LoadAbility,
        void(const std::shared_ptr<AbilityInfo>& abilityInfo, const std::shared_ptr<ApplicationInfo>& appInfo,
            const std::shared_ptr<AAFwk::Want>& want, std::shared_ptr<AbilityRuntime::LoadParam> loadParam));
    MOCK_METHOD2(TerminateAbility, void(const sptr<IRemoteObject>& token, bool clearMissionFlag));
    MOCK_METHOD2(UpdateAbilityState, void(const sptr<IRemoteObject>& token, const AbilityState state));
    MOCK_METHOD0(Reset, void());
    MOCK_METHOD1(KillProcessByAbilityToken, void(const sptr<IRemoteObject>& token));
    MOCK_METHOD3(KillProcessesByUserId, void(int32_t userId, bool isNeedSendAppSpawnMsg,
        sptr<AAFwk::IUserCallback> callback));
    MOCK_METHOD4(KillProcessWithAccount, int(const std::string&, const int, const bool clearPageStack, int32_t));
    MOCK_METHOD1(KillProcessesInBatch, int(const std::vector<int32_t> &pids));
    MOCK_METHOD4(UpdateApplicationInfoInstalled, int(const std::string&, const int uid, const std::string&, bool));
    MOCK_METHOD3(ForceKillApplication, int32_t(const std::string& appName, const int userId, const int appIndex));
    MOCK_METHOD4(KillApplication, int32_t(const std::string& bundleName, const bool clearPageStack, int32_t appIndex,
        const std::string& reason));
    MOCK_METHOD1(KillProcessesByAccessTokenId, int32_t(const uint32_t accessTokenId));
    MOCK_METHOD3(KillApplicationByUid, int(const std::string&, const int uid, const std::string&));
    MOCK_METHOD0(IsReady, bool());
    MOCK_METHOD1(AbilityAttachTimeOut, void(const sptr<IRemoteObject>& token));
    MOCK_METHOD2(PrepareTerminate, void(const sptr<IRemoteObject>& token, bool clearMissionFlag));
    MOCK_METHOD2(PrepareTerminateApp, void(const pid_t, const std::string &moduleName));
    MOCK_METHOD2(GetRunningProcessInfoByToken,
        void(const sptr<IRemoteObject>& token, OHOS::AppExecFwk::RunningProcessInfo& info));
    MOCK_METHOD1(SetAbilityForegroundingFlagToAppRecord, void(const pid_t pid));
    MOCK_METHOD4(StartSpecifiedAbility, void(const AAFwk::Want&, const AppExecFwk::AbilityInfo&, int32_t,
        const std::string&));
    MOCK_METHOD4(StartSpecifiedProcess, void(const AAFwk::Want&, const AppExecFwk::AbilityInfo&, int32_t,
        const std::string&));
    MOCK_METHOD1(RegisterStartSpecifiedAbilityResponse, void(const sptr<IStartSpecifiedAbilityResponse>& response));
    MOCK_METHOD3(GetApplicationInfoByProcessID, int(const int pid, AppExecFwk::ApplicationInfo& application,
        bool& debug));
    MOCK_METHOD3(NotifyAppMgrRecordExitReason, int32_t(int32_t pid, int32_t reason, const std::string &exitMsg));
    MOCK_METHOD4(NotifyAppMgrRecordExitReasonCompability, int32_t(int32_t pid, int32_t killId,
        const std::string &killMsg, const std::string &innerMsg));
    MOCK_METHOD3(GetBundleNameByPid, int32_t(const int pid, std::string &bundleName, int32_t &uid));
    MOCK_METHOD1(RegisterAppDebugListener, int32_t(const sptr<IAppDebugListener> &listener));
    MOCK_METHOD1(UnregisterAppDebugListener, int32_t(const sptr<IAppDebugListener> &listener));
    MOCK_METHOD2(AttachAppDebug, int32_t(const std::string &bundleName, bool isDebugFromLocal));
    MOCK_METHOD1(DetachAppDebug, int32_t(const std::string &bundleName));
    MOCK_METHOD1(RegisterAbilityDebugResponse, int32_t(const sptr<IAbilityDebugResponse> &response));
    MOCK_METHOD1(IsAttachDebug, bool(const std::string &bundleName));
    MOCK_METHOD2(SetAppWaitingDebug, int32_t(const std::string &bundleName, bool isPersist));
    MOCK_METHOD0(CancelAppWaitingDebug, int32_t());
    MOCK_METHOD1(GetWaitingDebugApp, int32_t(std::vector<std::string> &debugInfoList));
    MOCK_METHOD1(IsWaitingDebugApp, bool(const std::string &bundleName));
    MOCK_METHOD0(ClearNonPersistWaitingDebugFlag, void());
    MOCK_METHOD0(IsMemorySizeSufficient, bool());
    MOCK_METHOD4(KillProcessesByPids, int32_t(const std::vector<int32_t> &pids, const std::string &reason,
        bool subProcess, bool isKillPrecedeStart));

    MockAmsMgrScheduler() : AmsMgrStub() {};
    virtual ~MockAmsMgrScheduler() {};
    virtual void RegisterAppStateCallback(const sptr<IAppStateCallback>& callback) override
    {
        callback->OnAbilityRequestDone(nullptr, AbilityState::ABILITY_STATE_BACKGROUND);
        AppProcessData appProcessData;
        callback->OnAppStateChanged(appProcessData);
    }

    MOCK_METHOD1(SetCurrentUserId, void(const int32_t userId));

    MOCK_METHOD4(SendRequest, int(uint32_t, MessageParcel&, MessageParcel&, MessageOption&));
    MOCK_METHOD4(PreloadApplicationByPhase, int32_t(const std::string&, int32_t, int32_t, AppExecFwk::PreloadPhase));
    MOCK_METHOD2(NotifyPreloadAbilityStateChanged, int32_t(sptr<IRemoteObject>, bool));
    MOCK_METHOD4(CheckPreloadAppRecordExist, int32_t(const std::string&, int32_t, int32_t, bool &));

    int InvokeSendRequest(uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option)
    {
        code_ = code;

        return 0;
    }

    int code_ = 0;
};
}  // namespace AppExecFwk
}  // namespace OHOS

#endif
