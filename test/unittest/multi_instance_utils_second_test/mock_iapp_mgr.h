/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#ifndef MOCK_IAPP_MGR_H
#define MOCK_IAPP_MGR_H

#include "ability_foreground_state_observer_interface.h"
#include "ability_info.h"
#include "app_foreground_state_observer_interface.h"
#include "app_malloc_info.h"
#include "app_mgr_ipc_interface_code.h"
#include "app_record_id.h"
#include "application_info.h"
#include "bundle_info.h"
#include "child_process_info.h"
#ifdef SUPPORT_CHILD_PROCESS
#include "child_process_request.h"
#endif // SUPPORT_CHILD_PROCESS
#include "app_jsheap_mem_info.h"
#include "app_mgr_interface.h"
#include "fault_data.h"
#include "iapp_state_callback.h"
#include "iapplication_state_observer.h"
#include "iconfiguration_observer.h"
#include "iquick_fix_callback.h"
#include "iremote_broker.h"
#include "iremote_object.h"
#include "irender_state_observer.h"
#include "kia_interceptor_interface.h"
#include "memory_level_info.h"
#include "page_state_data.h"
#include "process_memory_state.h"
#include "render_process_info.h"
#include "running_multi_info.h"
#include "running_process_info.h"
#include "system_memory_attr.h"
#include "want.h"

namespace OHOS {
namespace AppExecFwk {
class MockIAppMgr : public AppExecFwk::IAppMgr {
public:
    virtual void AttachApplication(const sptr<IRemoteObject>& app) {}

    virtual void ApplicationForegrounded(const int32_t recordId) {}

    virtual void ApplicationBackgrounded(const int32_t recordId) {}

    virtual void ApplicationTerminated(const int32_t recordId) {}

    virtual void AbilityCleaned(const sptr<IRemoteObject>& token) {}

    virtual sptr<IAmsMgr> GetAmsMgr()
    {
        return nullptr;
    }

    virtual int32_t ClearUpApplicationData(const std::string& bundleName, int32_t appCloneIndex, int32_t userId = -1)
    {
        return 0;
    }

    virtual int32_t ClearUpApplicationDataBySelf(int32_t userId = -1)
    {
        return 0;
    }

    virtual int GetAllRunningProcesses(std::vector<RunningProcessInfo>& info)
    {
        return 0;
    }

    virtual int32_t GetRunningMultiAppInfoByBundleName(const std::string& bundleName, RunningMultiAppInfo& info)
    {
        return 0;
    }

    virtual int32_t GetAllRunningInstanceKeysBySelf(std::vector<std::string>& instanceKeys)
    {
        return 0;
    }

    virtual int32_t GetAllRunningInstanceKeysByBundleName(
        const std::string& bundleName, std::vector<std::string>& instanceKeys, int32_t userId = -1)
    {
        if (bundleName != "") {
            userId = 0;
            instanceKeys.push_back("bundleName");
        }
        return userId;
    }

    virtual sptr<IRemoteObject> AsObject()
    {
        return nullptr;
    }

    virtual int GetRunningProcessesByBundleType(const BundleType bundleType, std::vector<RunningProcessInfo>& info)
    {
        return 0;
    }

    virtual int GetAllRenderProcesses(std::vector<RenderProcessInfo>& info)
    {
        return 0;
    }

    virtual int GetAllChildrenProcesses(std::vector<ChildProcessInfo>& info)
    {
        return 0;
    }

    virtual int32_t IsTerminatingByPid(pid_t pid, bool& isTerminating)
    {
        return 0;
    }

    virtual int GetProcessRunningInfosByUserId(std::vector<RunningProcessInfo>& info, int32_t userId)
    {
        return 0;
    }

    virtual int32_t GetProcessRunningInformation(RunningProcessInfo& info)
    {
        return 0;
    }

    virtual int NotifyMemoryLevel(int32_t level)
    {
        return 0;
    }

    virtual int32_t NotifyProcMemoryLevel(const std::map<pid_t, MemoryLevel>& procLevelMap,
        bool isShellCall = false)
    {
        return 0;
    }

    virtual int DumpHeapMemory(const int32_t pid, OHOS::AppExecFwk::MallocInfo& mallocInfo)
    {
        return 0;
    }

    virtual void AddAbilityStageDone(const int32_t recordId) {}

    virtual int DumpJsHeapMemory(OHOS::AppExecFwk::JsHeapDumpInfo& info)
    {
        return 0;
    }

    virtual void StartupResidentProcess(const std::vector<AppExecFwk::BundleInfo>& bundleInfos) {}

    virtual int32_t RegisterApplicationStateObserver(
        const sptr<IApplicationStateObserver>& observer, const std::vector<std::string>& bundleNameList = {})
    {
        return 0;
    }

    virtual int32_t UnregisterApplicationStateObserver(const sptr<IApplicationStateObserver>& observer)
    {
        return 0;
    }

    virtual int32_t RegisterAbilityForegroundStateObserver(const sptr<IAbilityForegroundStateObserver>& observer)
    {
        return 0;
    }

    virtual int32_t UnregisterAbilityForegroundStateObserver(const sptr<IAbilityForegroundStateObserver>& observer)
    {
        return 0;
    }

    virtual int32_t GetForegroundApplications(std::vector<AppStateData>& list)
    {
        return 0;
    }

    virtual int StartUserTestProcess(
        const AAFwk::Want& want, const sptr<IRemoteObject>& observer, const BundleInfo& bundleInfo, int32_t userId)
    {
        return 0;
    }

    virtual int FinishUserTest(const std::string& msg, const int64_t& resultCode, const std::string& bundleName)
    {
        return 0;
    }

    virtual void ScheduleAcceptWantDone(const int32_t recordId, const AAFwk::Want& want, const std::string& flag) {}
    virtual void ScheduleNewProcessRequestDone(const int32_t recordId, const AAFwk::Want& want, const std::string& flag)
    {}

    virtual int GetAbilityRecordsByProcessID(const int pid, std::vector<sptr<IRemoteObject>>& tokens)
    {
        return 0;
    }

    virtual int PreStartNWebSpawnProcess()
    {
        return 0;
    }

    virtual int StartRenderProcess(const std::string& renderParam, int32_t ipcFd, int32_t sharedFd, int32_t crashFd,
        pid_t& renderPid, bool isGPU = false)
    {
        return 0;
    }

    virtual void AttachRenderProcess(const sptr<IRemoteObject>& renderScheduler) {}

    virtual int GetRenderProcessTerminationStatus(pid_t renderPid, int& status)
    {
        return 0;
    }

    virtual int32_t GetConfiguration(Configuration& config)
    {
        return 0;
    }

    virtual int32_t UpdateConfiguration(const Configuration& config, const int32_t userId = -1)
    {
        return 0;
    }

    virtual int32_t UpdateConfigurationByBundleName(
        const Configuration& config, const std::string& name, int32_t appIndex = 0)
    {
        return 0;
    }

    virtual int32_t RegisterConfigurationObserver(const sptr<IConfigurationObserver>& observer)
    {
        return 0;
    }

    virtual int32_t UnregisterConfigurationObserver(const sptr<IConfigurationObserver>& observer)
    {
        return 0;
    }

    virtual bool GetAppRunningStateByBundleName(const std::string& bundleName)
    {
        return false;
    }

    virtual int32_t NotifyLoadRepairPatch(const std::string& bundleName, const sptr<IQuickFixCallback>& callback)
    {
        return 0;
    }

    virtual int32_t NotifyHotReloadPage(const std::string& bundleName, const sptr<IQuickFixCallback>& callback)
    {
        return 0;
    }

    virtual int32_t NotifyUnLoadRepairPatch(const std::string& bundleName, const sptr<IQuickFixCallback>& callback)
    {
        return 0;
    }

    virtual int32_t NotifyAppFault(const FaultData& faultData)
    {
        return 0;
    }

    virtual int32_t NotifyAppFaultBySA(const AppFaultDataBySA& faultData)
    {
        return 0;
    }

    virtual bool SetAppFreezeFilter(int32_t pid)
    {
        return false;
    }

#ifdef BGTASKMGR_CONTINUOUS_TASK_ENABLE

    virtual int32_t SetContinuousTaskProcess(int32_t pid, bool isContinuousTask)
    {
        return 0;
    };
#endif

    virtual bool IsSharedBundleRunning(const std::string& bundleName, uint32_t versionCode)
    {
        return false;
    }

    virtual int32_t StartNativeProcessForDebugger(const AAFwk::Want& want)
    {
        return 0;
    }

    virtual int32_t GetBundleNameByPid(const int pid, std::string& bundleName, int32_t& uid)
    {
        return 0;
    }

    virtual int32_t GetRunningProcessInfoByPid(const pid_t pid, OHOS::AppExecFwk::RunningProcessInfo& info)
    {
        return 0;
    }

    virtual int32_t GetRunningProcessInfoByChildProcessPid(
        const pid_t childPid, OHOS::AppExecFwk::RunningProcessInfo& info)
    {
        return 0;
    }

    virtual int32_t GetProcessMemoryByPid(const int32_t pid, int32_t& memorySize)
    {
        return 0;
    }

    virtual int32_t GetRunningProcessInformation(
        const std::string& bundleName, int32_t userId, std::vector<RunningProcessInfo>& info)
    {
        return 0;
    }

    virtual int32_t NotifyPageShow(const sptr<IRemoteObject>& token, const PageStateData& pageStateData)
    {
        return 0;
    }

    virtual int32_t NotifyPageHide(const sptr<IRemoteObject>& token, const PageStateData& pageStateData)
    {
        return 0;
    }

    virtual int32_t ChangeAppGcState(pid_t pid, int32_t state, uint64_t tid = 0)
    {
        return 0;
    }

    virtual int32_t RegisterAppRunningStatusListener(const sptr<IRemoteObject>& listener)
    {
        return 0;
    }

    virtual int32_t UnregisterAppRunningStatusListener(const sptr<IRemoteObject>& listener)
    {
        return 0;
    }

    virtual int32_t RegisterAppForegroundStateObserver(const sptr<IAppForegroundStateObserver>& observer)
    {
        return 0;
    }

    virtual int32_t UnregisterAppForegroundStateObserver(const sptr<IAppForegroundStateObserver>& observer)
    {
        return 0;
    }

    virtual int32_t IsApplicationRunning(const std::string& bundleName, bool& isRunning)
    {
        return 0;
    }

    virtual int32_t IsAppRunning(const std::string& bundleName, int32_t appCloneIndex, bool& isRunning)
    {
        return 0;
    }

    virtual int32_t IsAppRunningByBundleNameAndUserId(const std::string& bundleName, int32_t userId, bool& isRunning)
    {
        return 0;
    }

#ifdef SUPPORT_CHILD_PROCESS

    virtual int32_t StartChildProcess(pid_t& childPid, const ChildProcessRequest& request)
    {
        return 0;
    }

    virtual int32_t GetChildProcessInfoForSelf(ChildProcessInfo& info)
    {
        return 0;
    }

    virtual void AttachChildProcess(const sptr<IRemoteObject>& childScheduler) {}

    virtual void ExitChildProcessSafely() {}
#endif // SUPPORT_CHILD_PROCESS

    virtual bool IsFinalAppProcess()
    {
        return false;
    }

    virtual int32_t RegisterRenderStateObserver(const sptr<IRenderStateObserver>& observer)
    {
        return 0;
    }

    virtual int32_t UnregisterRenderStateObserver(const sptr<IRenderStateObserver>& observer)
    {
        return 0;
    }

    virtual int32_t RegisterKiaInterceptor(const sptr<IKiaInterceptor>& interceptor)
    {
        return 0;
    }

    virtual int32_t CheckIsKiaProcess(pid_t pid, bool& isKia)
    {
        return 0;
    }

    virtual int32_t UpdateRenderState(pid_t renderPid, int32_t state)
    {
        return 0;
    }

    virtual int32_t SignRestartAppFlag(int32_t uid, const std::string& instanceKey)
    {
        return 0;
    }

    virtual int32_t GetAppRunningUniqueIdByPid(pid_t pid, std::string& appRunningUniqueId)
    {
        return 0;
    }

    virtual int32_t GetAllUIExtensionRootHostPid(pid_t pid, std::vector<pid_t>& hostPids)
    {
        return 0;
    }

    virtual int32_t GetAllUIExtensionProviderPid(pid_t hostPid, std::vector<pid_t>& providerPids)
    {
        return 0;
    }

    virtual int32_t NotifyMemorySizeStateChanged(int32_t memorySizeState)
    {
        return 0;
    }

    virtual int32_t SetSupportedProcessCacheSelf(bool isSupport)
    {
        return 0;
    }

    virtual int32_t SetSupportedProcessCache(int32_t pid, bool isSupport)
    {
        return 0;
    }

    virtual int32_t IsProcessCacheSupported(int32_t pid, bool &isSupported)
    {
        return 0;
    }

    virtual int32_t SetProcessCacheEnable(int32_t pid, bool enable)
    {
        return 0;
    }

    virtual int32_t LockProcessCache(int32_t pid, bool isLock)
    {
        return 0;
    }

    virtual void SetAppAssertionPauseState(bool flag) {}

    virtual void SaveBrowserChannel(sptr<IRemoteObject> browser) {}

    virtual int32_t CheckCallingIsUserTestMode(const pid_t pid, bool& isUserTest)
    {
        return 0;
    }

#ifdef SUPPORT_CHILD_PROCESS

    virtual int32_t CreateNativeChildProcess(const std::string& libName,
        const sptr<IRemoteObject>& callback, const ChildProcessRequest& request)
    {
        return 0;
    }
#endif // SUPPORT_CHILD_PROCESS

    virtual int32_t NotifyProcessDependedOnWeb()
    {
        return 0;
    }

    virtual void KillProcessDependedOnWeb()
    {
        return;
    }

    virtual void RestartResidentProcessDependedOnWeb()
    {
        return;
    }

    virtual int32_t GetSupportedProcessCachePids(const std::string& bundleName, std::vector<int32_t>& pidList)
    {
        return 0;
    }

    virtual int32_t KillAppSelfWithInstanceKey(
        const std::string& instanceKey, bool clearPageStack, const std::string& reason)
    {
        return 0;
    }

    virtual int32_t IsSpecifiedModuleLoaded(const AAFwk::Want& want, const AbilityInfo& abilityInfo, bool& result)
    {
        return 0;
    }

    virtual int32_t UpdateProcessMemoryState(const std::vector<ProcessMemoryState>& procMemState)
    {
        return 0;
    }
};
} // namespace AppExecFwk
} // namespace OHOS
#endif // MOCK_IAPP_MGR_H