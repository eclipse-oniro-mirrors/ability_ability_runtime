/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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
namespace AppExecFwk {
class MockAmsMgrScheduler : public AmsMgrStub {
public:
    MOCK_METHOD6(LoadAbility,
        void(const sptr<IRemoteObject>& token, const sptr<IRemoteObject>& preToken,
            const std::shared_ptr<AbilityInfo>& abilityInfo, const std::shared_ptr<ApplicationInfo>& appInfo,
            const std::shared_ptr<AAFwk::Want>& want, int32_t abilityRecordId));
    MOCK_METHOD5(AbilityBehaviorAnalysis,
        void(const sptr<OHOS::IRemoteObject>& token, const sptr<OHOS::IRemoteObject>& preToken,
            const int32_t visibility, const int32_t perceptibility, const int32_t connectionState));
    MOCK_METHOD2(TerminateAbility, void(const sptr<IRemoteObject>& token, bool clearMissionFlag));
    MOCK_METHOD2(UpdateAbilityState, void(const sptr<IRemoteObject>& token, const AbilityState state));
    MOCK_METHOD0(Reset, void());
    MOCK_METHOD1(KillProcessByAbilityToken, void(const sptr<IRemoteObject>& token));
    MOCK_METHOD1(KillProcessesByUserId, void(int32_t userId));
    MOCK_METHOD3(KillProcessWithAccount, int(const std::string&, const int, const bool clearPageStack));
    MOCK_METHOD2(UpdateApplicationInfoInstalled, int(const std::string&, const int uid));
    MOCK_METHOD2(KillApplication, int32_t(const std::string& bundleName, const bool clearPageStack));
    MOCK_METHOD3(ForceKillApplication, int32_t(const std::string& appName, const int userId, const int appIndex));
    MOCK_METHOD1(KillProcessesByAccessTokenId, int32_t(const uint32_t accessTokenId));
    MOCK_METHOD2(KillApplicationByUid, int(const std::string&, const int uid));
    MOCK_METHOD0(IsReady, bool());
    MOCK_METHOD1(AbilityAttachTimeOut, void(const sptr<IRemoteObject>& token));
    MOCK_METHOD2(PrepareTerminate, void(const sptr<IRemoteObject>& token, bool clearMissionFlag));
    MOCK_METHOD2(GetRunningProcessInfoByToken,
        void(const sptr<IRemoteObject>& token, OHOS::AppExecFwk::RunningProcessInfo& info));
    MOCK_METHOD1(SetAbilityForegroundingFlagToAppRecord, void(const pid_t pid));
    MOCK_METHOD3(StartSpecifiedAbility, void(const AAFwk::Want&, const AppExecFwk::AbilityInfo&, int32_t));
    MOCK_METHOD3(StartSpecifiedProcess, void(const AAFwk::Want&, const AppExecFwk::AbilityInfo&, int32_t));
    MOCK_METHOD1(RegisterStartSpecifiedAbilityResponse, void(const sptr<IStartSpecifiedAbilityResponse>& response));
    MOCK_METHOD3(GetApplicationInfoByProcessID, int(const int pid, AppExecFwk::ApplicationInfo& application,
        bool& debug));
    MOCK_METHOD3(NotifyAppMgrRecordExitReason, int32_t(int32_t pid, int32_t reason, const std::string &exitMsg));
    MOCK_METHOD3(GetBundleNameByPid, int32_t(const int pid, std::string &bundleName, int32_t &uid));
    MOCK_METHOD1(RegisterAppDebugListener, int32_t(const sptr<IAppDebugListener> &listener));
    MOCK_METHOD1(UnregisterAppDebugListener, int32_t(const sptr<IAppDebugListener> &listener));
    MOCK_METHOD1(AttachAppDebug, int32_t(const std::string &bundleName));
    MOCK_METHOD1(DetachAppDebug, int32_t(const std::string &bundleName));
    MOCK_METHOD1(RegisterAbilityDebugResponse, int32_t(const sptr<IAbilityDebugResponse> &response));
    MOCK_METHOD1(IsAttachDebug, bool(const std::string &bundleName));
    MOCK_METHOD2(SetAppWaitingDebug, int32_t(const std::string &bundleName, bool isPersist));
    MOCK_METHOD0(CancelAppWaitingDebug, int32_t());
    MOCK_METHOD1(GetWaitingDebugApp, int32_t(std::vector<std::string> &debugInfoList));
    MOCK_METHOD1(IsWaitingDebugApp, bool(const std::string &bundleName));
    MOCK_METHOD0(ClearNonPersistWaitingDebugFlag, void());
    MOCK_METHOD0(IsMemorySizeSufficent, bool());

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
