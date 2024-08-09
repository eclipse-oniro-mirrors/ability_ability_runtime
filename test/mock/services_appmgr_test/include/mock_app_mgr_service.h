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

#ifndef MOCK_OHOS_ABILITY_RUNTIME_MOCK_APP_MGR_SERVICE_H
#define MOCK_OHOS_ABILITY_RUNTIME_MOCK_APP_MGR_SERVICE_H

#include "gmock/gmock.h"
#include <cstdint>
#include "semaphore_ex.h"
#include "app_mgr_stub.h"
#include "app_malloc_info.h"

namespace OHOS {
namespace AppExecFwk {
class MockAppMgrService : public AppMgrStub {
public:
    MOCK_METHOD6(LoadAbility,
        void(const sptr<IRemoteObject>& token, const sptr<IRemoteObject>& preToken,
            const std::shared_ptr<AbilityInfo>& abilityInfo, const std::shared_ptr<ApplicationInfo>& appInfo,
            const std::shared_ptr<AAFwk::Want>& want, int32_t abilityRecordId));
    MOCK_METHOD2(TerminateAbility, void(const sptr<IRemoteObject>& token, bool clearMissionFlag));
    MOCK_METHOD2(UpdateAbilityState, void(const sptr<IRemoteObject>& token, const AbilityState state));
    MOCK_METHOD1(AttachApplication, void(const sptr<IRemoteObject>& app));
    MOCK_METHOD1(NotifyMemoryLevel, int(int32_t level));
    MOCK_METHOD1(NotifyProcMemoryLevel, int32_t(const std::map<pid_t, MemoryLevel> &procLevelMap));
    MOCK_METHOD2(DumpHeapMemory, int(const int32_t pid, OHOS::AppExecFwk::MallocInfo &mallocInfo));
    MOCK_METHOD1(ApplicationForegrounded, void(const int32_t recordId));
    MOCK_METHOD1(ApplicationBackgrounded, void(const int32_t recordId));
    MOCK_METHOD1(ApplicationTerminated, void(const int32_t recordId));
    MOCK_METHOD1(AbilityCleaned, void(const sptr<IRemoteObject>& token));
    MOCK_METHOD2(UpdateApplicationInfoInstalled, int(const std::string&, const int uid));
    MOCK_METHOD2(KillApplication, int32_t(const std::string& appName, const bool clearPageStack));
    MOCK_METHOD3(ForceKillApplication, int32_t(const std::string& appName, const int userId, const int appIndex));
    MOCK_METHOD1(KillProcessesByAccessTokenId, int32_t(const uint32_t accessTokenId));
    MOCK_METHOD2(KillApplicationByUid, int(const std::string&, const int uid));
    MOCK_METHOD1(IsBackgroundRunningRestricted, int(const std::string& bundleName));
    MOCK_METHOD1(GetAllRunningProcesses, int(std::vector<RunningProcessInfo>& info));
    MOCK_METHOD2(GetRunningProcessesByBundleType, int(const BundleType bundleType,
        std::vector<RunningProcessInfo>& info));
    MOCK_METHOD2(GetProcessRunningInfosByUserId, int(std::vector<RunningProcessInfo>& info, int32_t userId));
    MOCK_METHOD1(GetAllRenderProcesses, int(std::vector<RenderProcessInfo>& info));
    MOCK_METHOD0(GetAmsMgr, sptr<IAmsMgr>());
    MOCK_METHOD1(GetAppFreezingTime, void(int& time));
    MOCK_METHOD1(SetAppFreezingTime, void(int time));
    MOCK_METHOD3(ClearUpApplicationData, int32_t(const std::string& bundleName, int32_t appCloneIndex, int32_t userId));
    MOCK_METHOD1(ClearUpApplicationDataBySelf, int32_t(int32_t userId));
    MOCK_METHOD1(StartupResidentProcess, void(const std::vector<AppExecFwk::BundleInfo>& bundleInfos));
    MOCK_METHOD1(AddAbilityStageDone, void(const int32_t recordId));
    MOCK_METHOD0(PreStartNWebSpawnProcess, int());
    MOCK_METHOD6(StartRenderProcess, int(const std::string&, int32_t, int32_t, int32_t, pid_t&, bool));
    MOCK_METHOD1(AttachRenderProcess, void(const sptr<IRemoteObject>& renderScheduler));
    MOCK_METHOD1(SaveBrowserChannel, void(sptr<IRemoteObject> browser));
    MOCK_METHOD2(GetRenderProcessTerminationStatus, int(pid_t renderPid, int& status));
    MOCK_METHOD2(RegisterApplicationStateObserver, int32_t(const sptr<IApplicationStateObserver>& observer,
        const std::vector<std::string>& bundleNameList));
    MOCK_METHOD1(UnregisterApplicationStateObserver, int32_t(const sptr<IApplicationStateObserver>& observer));
    MOCK_METHOD3(ScheduleAcceptWantDone,
        void(const int32_t recordId, const AAFwk::Want& want, const std::string& flag));
    MOCK_METHOD3(ScheduleNewProcessRequestDone,
        void(const int32_t recordId, const AAFwk::Want& want, const std::string& flag));
    MOCK_METHOD2(GetAbilityRecordsByProcessID, int(const int pid, std::vector<sptr<IRemoteObject>>& tokens));
    MOCK_METHOD1(GetConfiguration, int32_t(Configuration& config));
    MOCK_METHOD1(UpdateConfiguration, int32_t(const Configuration& config));
    MOCK_METHOD2(UpdateConfigurationByBundleName, int32_t(const Configuration& config, const std::string &name));
    MOCK_METHOD1(RegisterConfigurationObserver, int32_t(const sptr<IConfigurationObserver>& observer));
    MOCK_METHOD1(UnregisterConfigurationObserver, int32_t(const sptr<IConfigurationObserver>& observer));
#ifdef ABILITY_COMMAND_FOR_TEST
    MOCK_METHOD0(BlockAppService, int());
#endif
    MOCK_METHOD1(GetAppRunningStateByBundleName, bool(const std::string& bundleName));
    MOCK_METHOD2(NotifyLoadRepairPatch, int32_t(const std::string& bundleName,
        const sptr<IQuickFixCallback>& callback));
    MOCK_METHOD2(NotifyHotReloadPage, int32_t(const std::string& bundleName, const sptr<IQuickFixCallback>& callback));
    MOCK_METHOD2(NotifyUnLoadRepairPatch, int32_t(const std::string& bundleName,
        const sptr<IQuickFixCallback>& callback));
    MOCK_METHOD2(IsSharedBundleRunning, bool(const std::string &bundleName, uint32_t versionCode));
    MOCK_METHOD3(GetBundleNameByPid, int32_t(const int pid, std::string &bundleName, int32_t &uid));

    MOCK_METHOD1(NotifyAppFault, int32_t(const FaultData &faultData));
    MOCK_METHOD1(NotifyAppFaultBySA, int32_t(const AppFaultDataBySA &faultData));
    MOCK_METHOD2(GetProcessMemoryByPid, int32_t(const int32_t pid, int32_t & memorySize));
    MOCK_METHOD3(GetRunningProcessInformation, int32_t(const std::string & bundleName, int32_t userId,
        std::vector<RunningProcessInfo> &info));
    MOCK_METHOD2(GetRunningMultiAppInfoByBundleName, int32_t(const std::string &bundleName,
        RunningMultiAppInfo &info));
    MOCK_METHOD2(IsApplicationRunning, int32_t(const std::string &bundleName, bool &isRunning));
    MOCK_METHOD3(IsAppRunning, int32_t(const std::string &bundleName,
        int32_t appCloneIndex, bool &isRunning));
    MOCK_METHOD2(StartChildProcess, int32_t(pid_t &childPid, const ChildProcessRequest &request));
    MOCK_METHOD1(GetChildProcessInfoForSelf, int32_t(ChildProcessInfo &info));
    MOCK_METHOD1(AttachChildProcess, void(const sptr<IRemoteObject> &childScheduler));
    MOCK_METHOD0(ExitChildProcessSafely, void());
    MOCK_METHOD1(RegisterRenderStateObserver, int32_t(const sptr<IRenderStateObserver> &observer));
    MOCK_METHOD1(UnregisterRenderStateObserver, int32_t(const sptr<IRenderStateObserver> &observer));
    MOCK_METHOD2(UpdateRenderState, int32_t(pid_t renderPid, int32_t state));

    MOCK_METHOD0(IsFinalAppProcess, bool());
    MOCK_METHOD1(SetSupportedProcessCacheSelf, int32_t(bool isSupport));
    MOCK_METHOD3(StartNativeChildProcess, int32_t(const std::string &libName, int32_t childProcessCount,
        const sptr<IRemoteObject> &callback));
    virtual int StartUserTestProcess(
        const AAFwk::Want &want, const sptr<IRemoteObject> &observer, const BundleInfo &bundleInfo, int32_t userId)
    {
        return 0;
    }

    virtual int32_t RegisterAppForegroundStateObserver(const sptr<IAbilityForegroundStateObserver> &observer)
    {
        return 0;
    }

    virtual int32_t RegisterAbilityForegroundStateObserver(const sptr<IAbilityForegroundStateObserver> &observer)
    {
        return 0;
    }

    virtual int32_t UnregisterAppForegroundStateObserver(const sptr<IAbilityForegroundStateObserver> &observer)
    {
        return 0;
    }

    virtual int32_t UnregisterAbilityForegroundStateObserver(const sptr<IAbilityForegroundStateObserver> &observer)
    {
        return 0;
    }

    virtual int32_t RegisterAppForegroundStateObserver(const sptr<IAppForegroundStateObserver> &observer)
    {
        return 0;
    }

    virtual int32_t UnregisterAppForegroundStateObserver(const sptr<IAppForegroundStateObserver> &observer)
    {
        return 0;
    }

    virtual int32_t StartNativeProcessForDebugger(const AAFwk::Want &want) override
    {
        return 0;
    }

    virtual int FinishUserTest(const std::string& msg, const int64_t& resultCode, const std::string& bundleName)
    {
        return 0;
    }

    virtual int GetProcessRunningInformation(RunningProcessInfo &info)
    {
        return 0;
    }

    virtual void RegisterAppStateCallback(const sptr<IAppStateCallback>& callback)
    {
        callback_ = callback;
    }

    int32_t CheckPermissionImpl([[maybe_unused]] const int32_t recordId, const std::string& data)
    {
        data_ = data;
        return 0;
    }

    virtual int32_t JudgeSandboxByPid(pid_t pid, bool &isSandbox)
    {
        return 0;
    }

    void KillApplicationImpl(const std::string& data)
    {
        data_ = data;
    }

    const std::string& GetData() const
    {
        return data_;
    }

    void Wait()
    {
        sem_.Wait();
    }

    void Post()
    {
        sem_.Post();
    }

    void UpdateState() const
    {
        if (!callback_) {
            return;
        }
        AppProcessData processData;
        processData.pid = 1;
        processData.appState = ApplicationState::APP_STATE_CREATE;
        callback_->OnAppStateChanged(processData);
    }

    void Terminate(const sptr<IRemoteObject>& token) const
    {
        if (!callback_) {
            return;
        }
        AbilityState st = AbilityState::ABILITY_STATE_CREATE;
        callback_->OnAbilityRequestDone(token, st);
    }

    MOCK_METHOD4(SendRequest, int(uint32_t, MessageParcel&, MessageParcel&, MessageOption&));

    int InvokeSendRequest(uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option)
    {
        code_ = code;

        return 0;
    }

    int code_;

    virtual bool SetAppFreezeFilter(int32_t pid)
    {
        return false;
    }

    virtual int32_t ChangeAppGcState(pid_t pid, int32_t state)
    {
        return 0;
    }

    int32_t RegisterAppRunningStatusListener(const sptr<IRemoteObject> &listener)
    {
        return 0;
    }

    int32_t UnregisterAppRunningStatusListener(const sptr<IRemoteObject> &listener)
    {
        return 0;
    }

    int32_t DumpJsHeapMemory(OHOS::AppExecFwk::JsHeapDumpInfo &info)
    {
        return 0;
    }

private:
    Semaphore sem_;
    std::string data_;
    sptr<IAppStateCallback> callback_;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // MOCK_OHOS_ABILITY_RUNTIME_MOCK_APP_MGR_SERVICE_H
