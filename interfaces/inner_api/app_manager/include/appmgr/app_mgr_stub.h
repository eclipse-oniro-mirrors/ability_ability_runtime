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

#ifndef OHOS_ABILITY_RUNTIME_APP_MGR_STUB_H
#define OHOS_ABILITY_RUNTIME_APP_MGR_STUB_H

#include <map>

#include "app_mgr_interface.h"
#include "iremote_stub.h"
#include "nocopyable.h"
#include "string_ex.h"

namespace OHOS {
namespace AppExecFwk {
class AppMgrStub : public IRemoteStub<IAppMgr> {
public:
    AppMgrStub();
    virtual ~AppMgrStub();

    virtual int OnRemoteRequest(
        uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;

    /**
     * Register application or process state observer.
     * @param observer, ability token.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t RegisterApplicationStateObserver(const sptr<IApplicationStateObserver> &observer,
        const std::vector<std::string> &bundleNameList = {}) override;

    /**
     * Unregister application or process state observer.
     * @param observer, ability token.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t UnregisterApplicationStateObserver(const sptr<IApplicationStateObserver> &observer) override;

    /**
     * Get foreground applications.
     * @param list, foreground apps.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t GetForegroundApplications(std::vector<AppStateData> &list) override;

    /**
     * Get pids of processes which belong to specific bundle name and support process cache feature.
     * @param bundleName bundle name.
     * @param pidList pid list of processes that support process cache..
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t GetSupportedProcessCachePids(const std::string &bundleName,
        std::vector<int32_t> &pidList) override;

private:
    int32_t HandleAttachApplication(MessageParcel &data, MessageParcel &reply);
    int32_t HandlePreloadApplication(MessageParcel &data, MessageParcel &reply);
    int32_t HandleApplicationForegrounded(MessageParcel &data, MessageParcel &reply);
    int32_t HandleApplicationBackgrounded(MessageParcel &data, MessageParcel &reply);
    int32_t HandleApplicationTerminated(MessageParcel &data, MessageParcel &reply);
    int32_t HandleAbilityCleaned(MessageParcel &data, MessageParcel &reply);
    int32_t HandleGetAmsMgr(MessageParcel &data, MessageParcel &reply);
    int32_t HandleClearUpApplicationData(MessageParcel &data, MessageParcel &reply);
    int32_t HandleGetAllRunningProcesses(MessageParcel &data, MessageParcel &reply);
    int32_t HandleGetRunningProcessesByBundleType(MessageParcel &data, MessageParcel &reply);
    int32_t HandleGetProcessRunningInfosByUserId(MessageParcel &data, MessageParcel &reply);
    int32_t HandleGetProcessRunningInformation(MessageParcel &data, MessageParcel &reply);
    int32_t HandleGetAllRenderProcesses(MessageParcel &data, MessageParcel &reply);
    int32_t HandlePreloadModuleFinished(MessageParcel &data, MessageParcel &reply);
    int32_t HandleMakeImage(MessageParcel &data, MessageParcel &reply);
    int32_t HandleDestroyImage(MessageParcel &data, MessageParcel &reply);
    int32_t HandleNotifyTemplateProcessDeepFrozen(MessageParcel &data, MessageParcel &reply);
#ifdef SUPPORT_CHILD_PROCESS
    int32_t HandleGetAllChildrenProcesses(MessageParcel &data, MessageParcel &reply);
#endif  // SUPPORT_CHILD_PROCESS
    int32_t HandleRegisterNativeChildExitNotify(MessageParcel &data, MessageParcel &reply);
    int32_t HandleUnregisterNativeChildExitNotify(MessageParcel &data, MessageParcel &reply);
    int32_t HandleAddAbilityStageDone(MessageParcel &data, MessageParcel &reply);
    int32_t HandleNotifyMemoryLevel(MessageParcel &data, MessageParcel &reply);
    int32_t HandleNotifyProcMemoryLevel(MessageParcel &data, MessageParcel &reply);
    int32_t HandleStartupResidentProcess(MessageParcel &data, MessageParcel &reply);
    int32_t HandleRegisterApplicationStateObserver(MessageParcel &data, MessageParcel &reply);
    int32_t HandleUnregisterApplicationStateObserver(MessageParcel &data, MessageParcel &reply);
    int32_t HandleRegisterImageProcessStateObserver(MessageParcel &data, MessageParcel &reply);
    int32_t HandleUnregisterImageProcessStateObserver(MessageParcel &data, MessageParcel &reply);
    int32_t HandleRegisterAbilityForegroundStateObserver(MessageParcel &data, MessageParcel &reply);
    int32_t HandleUnregisterAbilityForegroundStateObserver(MessageParcel &data, MessageParcel &reply);
    int32_t HandleGetForegroundApplications(MessageParcel &data, MessageParcel &reply);
    int32_t HandleStartUserTestProcess(MessageParcel &data, MessageParcel &reply);
    int32_t HandleFinishUserTest(MessageParcel &data, MessageParcel &reply);
    int32_t HandleScheduleAcceptWantDone(MessageParcel &data, MessageParcel &reply);
    int32_t HandleScheduleNewProcessRequestDone(MessageParcel &data, MessageParcel &reply);
    int32_t HandleGetAbilityRecordsByProcessID(MessageParcel &data, MessageParcel &reply);
    int32_t HandlePreStartNWebSpawnProcess(MessageParcel &data, MessageParcel &reply);
    int32_t HandleStartRenderProcess(MessageParcel &data, MessageParcel &reply);
    int32_t HandleAttachRenderProcess(MessageParcel &data, MessageParcel &reply);
    int32_t HandleGetRenderProcessTerminationStatus(MessageParcel &data, MessageParcel &reply);
    int32_t HandleGetConfiguration(MessageParcel &data, MessageParcel &reply);
    int32_t HandleGetConfigurationByUserId(MessageParcel &data, MessageParcel &reply);
    int32_t HandleUpdateConfiguration(MessageParcel &data, MessageParcel &reply);
    int32_t HandleUpdateConfigurationMultiUser(MessageParcel &data, MessageParcel &reply);
    int32_t HandleUpdateConfigurationForBackgroundApp(MessageParcel &data, MessageParcel &reply);
    int32_t HandleUpdateConfigurationByBundleName(MessageParcel &data, MessageParcel &reply);
    int32_t HandleRegisterConfigurationObserver(MessageParcel &data, MessageParcel &reply);
    int32_t HandleUnregisterConfigurationObserver(MessageParcel &data, MessageParcel &reply);
    int32_t HandleDumpHeapMemory(MessageParcel &data, MessageParcel &reply);
    int32_t HandleDumpJsHeapMemory(MessageParcel &data, MessageParcel &reply);
    int32_t HandleDumpCjHeapMemory(MessageParcel &data, MessageParcel &reply);
    int32_t HandleDumpMem(MessageParcel &data, MessageParcel &reply);
    int32_t HandleGetRunningMultiAppInfoByBundleName(MessageParcel &data, MessageParcel &reply);
    int32_t HandleGetAllRunningInstanceKeysBySelf(MessageParcel &data, MessageParcel &reply);
    int32_t HandleGetAllRunningInstanceKeysByBundleName(MessageParcel &data, MessageParcel &reply);
    int32_t HandleIsAppRunning(MessageParcel &data, MessageParcel &reply);
    int32_t HandleIsAppRunningByUserId(MessageParcel &data, MessageParcel &reply);
    int32_t HandleIsAppRunningByBundleNameAndUserId(MessageParcel &data, MessageParcel &reply);
    int32_t HandleGetAppRunningStateByBundleName(MessageParcel &data, MessageParcel &reply);
    int32_t HandleNotifyLoadRepairPatch(MessageParcel &data, MessageParcel &reply);
    int32_t HandleNotifyHotReloadPage(MessageParcel &data, MessageParcel &reply);
    int32_t HandleNotifyUnLoadRepairPatch(MessageParcel &data, MessageParcel &reply);
#ifdef BGTASKMGR_CONTINUOUS_TASK_ENABLE
    int32_t HandleSetContinuousTaskProcess(MessageParcel &data, MessageParcel &reply);
#endif
    int32_t HandleIsSharedBundleRunning(MessageParcel &data, MessageParcel &reply);
    int32_t HandleStartNativeProcessForDebugger(MessageParcel &data, MessageParcel &reply);
    int32_t HandleNotifyFault(MessageParcel &data, MessageParcel &reply);
    int32_t HandleNotifyFaultBySA(MessageParcel &data, MessageParcel &reply);
    int32_t HandleSetAppFreezeFilter(MessageParcel &data, MessageParcel &reply);
    int32_t HandleIsTerminatingByPid(MessageParcel &data, MessageParcel &reply);
    int32_t HandleGetBundleNameByPid(MessageParcel &data, MessageParcel &reply);
    int32_t HandleGetRunningProcessInfoByPid(MessageParcel &data, MessageParcel &reply);
    int32_t HandleGetRunningProcessInfoByChildProcessPid(MessageParcel &data, MessageParcel &reply);
    int32_t HandleGetProcessMemoryByPid(MessageParcel &data, MessageParcel &reply);
    int32_t HandleGetRunningProcessInformation(MessageParcel &data, MessageParcel &reply);
    int32_t HandleChangeAppGcState(MessageParcel &data, MessageParcel &reply);
    int32_t HandleNotifyPageShow(MessageParcel &data, MessageParcel &reply);
    int32_t HandleNotifyPageHide(MessageParcel &data, MessageParcel &reply);
    int32_t HandleRegisterAppRunningStatusListener(MessageParcel &data, MessageParcel &reply);
    int32_t HandleUnregisterAppRunningStatusListener(MessageParcel &data, MessageParcel &reply);
    int32_t HandleRegisterAppForegroundStateObserver(MessageParcel &data, MessageParcel &reply);
    int32_t HandleUnregisterAppForegroundStateObserver(MessageParcel &data, MessageParcel &reply);
    int32_t HandleIsApplicationRunning(MessageParcel &data, MessageParcel &reply);
#ifdef SUPPORT_CHILD_PROCESS
    int32_t HandleStartChildProcess(MessageParcel &data, MessageParcel &reply);
    int32_t HandleGetChildProcessInfoForSelf(MessageParcel &data, MessageParcel &reply);
    int32_t HandleAttachChildProcess(MessageParcel &data, MessageParcel &reply);
    int32_t HandleExitChildProcessSafely(MessageParcel &data, MessageParcel &reply);
#endif // SUPPORT_CHILD_PROCESS
    int32_t HandleClearUpApplicationDataBySelf(MessageParcel &data, MessageParcel &reply);
    int32_t HandleIsFinalAppProcess(MessageParcel &data, MessageParcel &reply);
    int32_t HandleRegisterRenderStateObserver(MessageParcel &data, MessageParcel &reply);
    int32_t HandleRegisterKiaInterceptor(MessageParcel &data, MessageParcel &reply);
    int32_t HandleCheckIsKiaProcess(MessageParcel &data, MessageParcel &reply);
    int32_t HandleUnregisterRenderStateObserver(MessageParcel &data, MessageParcel &reply);
    int32_t HandleUpdateRenderState(MessageParcel &data, MessageParcel &reply);
    int32_t HandleSignRestartAppFlag(MessageParcel &data, MessageParcel &reply);
    int32_t HandleSignRestartProcess(MessageParcel &data, MessageParcel &reply);
    int32_t HandleGetAppRunningUniqueIdByPid(MessageParcel &data, MessageParcel &reply);
    int32_t HandleGetAllUIExtensionRootHostPid(MessageParcel &data, MessageParcel &reply);
    int32_t HandleGetAllUIExtensionProviderPid(MessageParcel &data, MessageParcel &reply);
    int32_t HandleNotifyMemorySizeStateChanged(MessageParcel &data, MessageParcel &reply);
    int32_t HandleSetAppAssertionPauseState(MessageParcel &data, MessageParcel &reply);
    int32_t HandleSetSupportedProcessCacheSelf(MessageParcel &data, MessageParcel &reply);
    int32_t HandleSetSupportedProcessCache(MessageParcel &data, MessageParcel &reply);
    int32_t HandleIsProcessCacheSupported(MessageParcel &data, MessageParcel &reply);
    int32_t HandleSetProcessCacheEnable(MessageParcel &data, MessageParcel &reply);
    int32_t HandleLockProcessCache(MessageParcel &data, MessageParcel &reply);
    int32_t HandleSaveBrowserChannel(MessageParcel &data, MessageParcel &reply);
    int32_t HandleCheckCallingIsUserTestMode(MessageParcel &data, MessageParcel &reply);
#ifdef SUPPORT_CHILD_PROCESS
    int32_t HandleCreateNativeChildProcess(MessageParcel &data, MessageParcel &reply);
#endif // SUPPORT_CHILD_PROCESS
    int32_t HandleNotifyProcessDependedOnWeb(MessageParcel &data, MessageParcel &reply);
    int32_t HandleKillProcessDependedOnWeb(MessageParcel &data, MessageParcel &reply);
    int32_t HandleRestartResidentProcessDependedOnWeb(MessageParcel &data, MessageParcel &reply);
    int32_t HandleKillAppSelfWithInstanceKey(MessageParcel &data, MessageParcel &reply);
    int32_t HandleIsSpecifiedModuleLoaded(MessageParcel &data, MessageParcel &reply);
    int32_t HandleQueryRunningSharedBundles(MessageParcel &data, MessageParcel &reply);
    int32_t OnRemoteRequestInner(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option);
    int32_t OnRemoteRequestInnerFirst(uint32_t code, MessageParcel &data,
        MessageParcel &reply, MessageOption &option);
    int32_t OnRemoteRequestInnerSecond(uint32_t code, MessageParcel &data,
        MessageParcel &reply, MessageOption &option);
    int32_t OnRemoteRequestInnerThird(uint32_t code, MessageParcel &data,
        MessageParcel &reply, MessageOption &option);
    int32_t OnRemoteRequestInnerFourth(uint32_t code, MessageParcel &data,
        MessageParcel &reply, MessageOption &option);
    int32_t OnRemoteRequestInnerFifth(uint32_t code, MessageParcel &data,
        MessageParcel &reply, MessageOption &option);
    int32_t OnRemoteRequestInnerSixth(uint32_t code, MessageParcel &data,
        MessageParcel &reply, MessageOption &option);
    int32_t OnRemoteRequestInnerSeventh(uint32_t code, MessageParcel &data,
        MessageParcel &reply, MessageOption &option);
    int32_t OnRemoteRequestInnerEighth(uint32_t code, MessageParcel &data,
        MessageParcel &reply, MessageOption &option);
    int32_t HandleGetSupportedProcessCachePids(MessageParcel &data, MessageParcel &reply);
    int32_t HandleUpdateProcessMemoryState(MessageParcel &data, MessageParcel &reply);
    int32_t HandleLaunchAbility(MessageParcel &data, MessageParcel &reply);
    int32_t HandlePromoteCurrentToCandidateMasterProcess(MessageParcel &data, MessageParcel &reply);
    int32_t HandleDemoteCurrentFromCandidateMasterProcess(MessageParcel &data, MessageParcel &reply);
    int32_t HandleSetSpecifiedProcessRequestId(MessageParcel &data, MessageParcel &reply);
    int32_t HandleExitMasterProcessRole(MessageParcel &data, MessageParcel &reply);
    int32_t HandleRegisterApplicationStateObserverWithFilter(MessageParcel &data, MessageParcel &reply);
    int32_t HandleAllowScbProcessMoveToBackground(MessageParcel &data, MessageParcel &reply);
    int32_t HandleKillProcessByPidForExit(MessageParcel &data, MessageParcel &reply);
    int32_t HandleKillChildProcessByPid(MessageParcel &data, MessageParcel &reply);
    int32_t HandlePreloadExtension(MessageParcel &data, MessageParcel &reply);
    int32_t HandleSetProcessPrepareExit(MessageParcel &data, MessageParcel &reply);
    int32_t HandleGetAllAbilityInfos(MessageParcel &data, MessageParcel &reply);
    DISALLOW_COPY_AND_MOVE(AppMgrStub);
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_APP_MGR_STUB_H
