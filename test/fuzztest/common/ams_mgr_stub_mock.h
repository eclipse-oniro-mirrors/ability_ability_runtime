/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef FUZZTEST_OHOS_ABILITY_RUNTIME_AMS_MGR_STUB_MOCK_H
#define FUZZTEST_OHOS_ABILITY_RUNTIME_AMS_MGR_STUB_MOCK_H

#include "ams_mgr_stub.h"
#include "ability_debug_response_interface.h"
#include "app_debug_listener_interface.h"
#include "iapp_state_callback.h"
#include "iremote_object.h"
#include "istart_specified_ability_response.h"
#include "running_process_info.h"
#include "user_callback.h"
#include "want.h"

namespace OHOS {
namespace AppExecFwk {
// Common mock base for AmsMgrStub fuzzers. Overrides all pure-virtual
// IAmsMgr methods so that each fuzzer only needs to construct parcels.
class AmsMgrStubFuzzBase : public AmsMgrStub {
public:
    AmsMgrStubFuzzBase() = default;
    ~AmsMgrStubFuzzBase() = default;

    void TerminateAbility(const sptr<IRemoteObject> &token, bool clearMissionFlag) override {}
    void UpdateAbilityState(const sptr<IRemoteObject> &token, const AbilityState state,
        bool isFromScreenOffBackground = false, const UiAbilityLastCallerInfo &callerInfo = {}) override {}
    void RegisterAppStateCallback(const sptr<IAppStateCallback> &callback) override {}
    void KillProcessByAbilityToken(const sptr<IRemoteObject> &token) override {}
    int32_t SetGameSAPrelaunch(const sptr<IRemoteObject> &token, bool isGameSAPrelaunch) override { return 0; }
    void KillProcessesByUserId(int32_t userId, bool isNeedSendAppSpawnMsg = false,
        sptr<AAFwk::IUserCallback> callback = nullptr) override {}
    int32_t KillProcessesByPids(const std::vector<int32_t> &pids,
        const std::string &reason = "KillProcessesByPids", bool subProcess = false,
        bool isKillPrecedeStart = false) override { return 0; }
    int KillProcessWithAccount(const std::string &bundleName, const int accountId,
        const bool clearPageStack = false, int32_t appIndex = 0) override { return 0; }
    int32_t KillProcessesInBatch(const std::vector<int32_t> &pids) override { return 0; }
    int UpdateApplicationInfoInstalled(const std::string &bundleName, const int uid,
        const std::string &moduleName, bool isPlugin) override { return 0; }
    int KillApplication(const std::string &bundleName, bool clearPageStack = false, int32_t appIndex = 0,
        const std::string &reason = "KillApplication") override { return 0; }
    int ForceKillApplication(const std::string &bundleName, const int userId = -1, const int appIndex = 0) override
    {
        return 0;
    }
    int KillApplicationWithUserId(const std::string &bundleName,
        const int userId = -1, const int appIndex = 0) override { return 0; }
    int KillProcessesByAccessTokenId(const uint32_t accessTokenId) override { return 0; }
    int KillApplicationByUid(const std::string &bundleName, const int uid,
        const std::string& reason = "KillApplicationByUid") override { return 0; }
    void AbilityAttachTimeOut(const sptr<IRemoteObject> &token) override {}
    void PrepareTerminate(const sptr<IRemoteObject> &token, bool clearMissionFlag = false) override {}
    void GetRunningProcessInfoByToken(
        const sptr<IRemoteObject> &token, OHOS::AppExecFwk::RunningProcessInfo &info) override {}
    void SetAbilityForegroundingFlagToAppRecord(const pid_t pid) override {}
    void StartSpecifiedAbility(const AAFwk::Want &want, const AppExecFwk::AbilityInfo &abilityInfo,
        const AbilityRuntime::StartSpecifiedParam &param) override {}
    void RegisterStartSpecifiedAbilityResponse(const sptr<IStartSpecifiedAbilityResponse> &response) override {}
    void PrepareTerminateApp(const pid_t pid, const std::string &moduleName) override {}
    void StartSpecifiedProcess(const AAFwk::Want &want, const AppExecFwk::AbilityInfo &abilityInfo,
        int32_t requestId = 0, const std::string &customProcess = "") override {}
    int GetApplicationInfoByProcessID(const int pid, AppExecFwk::ApplicationInfo &application, bool &debug) override
    {
        return 0;
    }
    int32_t NotifyAppMgrRecordExitReason(int32_t pid, int32_t reason, const std::string &exitMsg) override
    {
        return 0;
    }
    int32_t NotifyAppMgrRecordExitReasonCompability(
        int32_t pid, int32_t killId, const std::string &killMsg, const std::string &innerMsg,
        int32_t reason, int32_t callerPid = -1) override
    {
        return 0;
    }
    int32_t GetBundleNameByPid(const int pid, std::string &bundleName, int32_t &uid) override { return 0; }
    int32_t RegisterAppDebugListener(const sptr<IAppDebugListener> &listener) override { return 0; }
    int32_t UnregisterAppDebugListener(const sptr<IAppDebugListener> &listener) override { return 0; }
    int32_t AttachAppDebug(const std::string &bundleName, bool isDebugFromLocal) override { return 0; }
    int32_t DetachAppDebug(const std::string &bundleName) override { return 0; }
    int32_t SetAppWaitingDebug(const std::string &bundleName, bool isPersist) override { return 0; }
    int32_t CancelAppWaitingDebug() override { return 0; }
    int32_t GetWaitingDebugApp(std::vector<std::string> &debugInfoList) override { return 0; }
    bool IsWaitingDebugApp(const std::string &bundleName) override { return false; }
    void ClearNonPersistWaitingDebugFlag() override {}
    int32_t RegisterAbilityDebugResponse(const sptr<IAbilityDebugResponse> &response) override { return 0; }
    bool IsAttachDebug(const std::string &bundleName) override { return false; }
    bool IsMemorySizeSufficient() override { return false; }
};
} // namespace AppExecFwk
} // namespace OHOS

#endif // FUZZTEST_OHOS_ABILITY_RUNTIME_AMS_MGR_STUB_MOCK_H
