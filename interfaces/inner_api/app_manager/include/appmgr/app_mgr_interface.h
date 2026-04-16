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

#ifndef OHOS_ABILITY_RUNTIME_APP_MGR_INTERFACE_H
#define OHOS_ABILITY_RUNTIME_APP_MGR_INTERFACE_H

#include "ability_foreground_state_observer_interface.h"
#include "ability_info.h"
#include "ams_mgr_interface.h"
#include "app_foreground_state_observer_interface.h"
#include "app_malloc_info.h"
#include "app_mem_dump_info.h"
#include "app_mgr_ipc_interface_code.h"
#include "app_record_id.h"
#include "application_info.h"
#include "application_state_filter.h"
#include "background_app_info.h"
#include "bundle_info.h"
#include "child_process_info.h"
#include "configuration_policy.h"
#ifdef SUPPORT_CHILD_PROCESS
#include "child_process_request.h"
#endif // SUPPORT_CHILD_PROCESS
#include "fault_data.h"
#include "kia_interceptor_interface.h"
#include "iapp_state_callback.h"
#include "iapplication_state_observer.h"
#include "iconfiguration_observer.h"
#include "image_error_handler_interface.h"
#include "image_process_state_observer_interface.h"
#include "iquick_fix_callback.h"
#include "iremote_broker.h"
#include "iremote_object.h"
#include "irender_state_observer.h"
#include "memory_level_info.h"
#include "page_state_data.h"
#include "process_memory_state.h"
#include "render_process_info.h"
#include "running_process_info.h"
#include "system_memory_attr.h"
#include "want.h"
#include "app_jsheap_mem_info.h"
#include "app_cjheap_mem_info.h"
#include "running_multi_info.h"
#include "native_child_notify_interface.h"

namespace OHOS {
namespace AppExecFwk {
class IAppMgr : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.appexecfwk.AppMgr");

    /**
     * AttachApplication, call AttachApplication() through proxy object,
     * get all the information needed to start the Application (data related to the Application ).
     *
     * @param app, information needed to start the Application.
     * @return
     */
    virtual void AttachApplication(const sptr<IRemoteObject> &app) = 0;

    /**
     * Preload application.
     *
     * @param bundleName The bundle name of the application to preload.
     * @param userId Indicates the user identification.
     * @param preloadMode Preload application mode.
     * @param appIndex The index of application clone.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t PreloadApplication(const std::string &bundleName, int32_t userId,
        AppExecFwk::PreloadMode preloadMode, int32_t appIndex = 0)
    {
        return 0;
    }

    /**
     * Launch Application Finished
     * @param recordId id of the app record
     */
    virtual void PreloadModuleFinished(const int32_t recordId)
    {
        return;
    }

    virtual int32_t MakeImage(const AAFwk::Want &want, int32_t userId,
        AppExecFwk::PreloadMode preloadMode, int32_t appIndex = 0, sptr<IImageErrorHandler> errorHandler = nullptr)
    {
        return 0;
    }

    virtual int32_t DestroyImage(uint64_t checkpointId, sptr<IImageErrorHandler> errorHandler = nullptr)
    {
        return 0;
    }

    virtual int32_t NotifyTemplateProcessDeepFrozen(int32_t pid)
    {
        return 0;
    }

    /**
     * ApplicationForegrounded, call ApplicationForegrounded() through proxy object,
     * set the application to Foreground State.
     *
     * @param recordId, a unique record that identifies this Application from others.
     * @return
     */
    virtual void ApplicationForegrounded(const int32_t recordId) = 0;

    /**
     * ApplicationBackgrounded, call ApplicationBackgrounded() through proxy object,
     * set the application to Backgrounded State.
     *
     * @param recordId, a unique record that identifies this Application from others.
     * @return
     */
    virtual void ApplicationBackgrounded(const int32_t recordId) = 0;

    /**
     * ApplicationTerminated, call ApplicationTerminated() through proxy object,
     * terminate the application.
     *
     * @param recordId, a unique record that identifies this Application from others.
     * @return
     */
    virtual void ApplicationTerminated(const int32_t recordId) = 0;

    /**
     * AbilityCleaned,call through AbilityCleaned() proxy project, clean Ability record.
     *
     * @param token, a unique record that identifies AbilityCleaned from others.
     * @return
     */
    virtual void AbilityCleaned(const sptr<IRemoteObject> &token) = 0;

    /**
     * GetAmsMgr, call GetAmsMgr() through proxy object, get AMS interface instance.
     *
     * @return sptr<IAmsMgr>, return to AMS interface instance.
     */
    virtual sptr<IAmsMgr> GetAmsMgr() = 0;

    /**
     * ClearUpApplicationData, call ClearUpApplicationData() through proxy project,
     * clear the application data.
     *
     * @param bundleName, bundle name in Application record.
     * @param appCloneIndex the app clone id.
     * @param userId the user id.
     * @return ErrCode
     */
    virtual int32_t ClearUpApplicationData(const std::string &bundleName, int32_t appCloneIndex,
        int32_t userId = -1) = 0;

    /**
     * ClearUpApplicationData, call ClearUpApplicationData() through proxy project,
     * clear the application data.
     *
     * @param userId the user id.
     * @return
     */
    virtual int32_t ClearUpApplicationDataBySelf(int32_t userId = -1) = 0;

    /**
     * GetAllRunningProcesses, call GetAllRunningProcesses() through proxy project.
     * Obtains information about application processes that are running on the device.
     *
     * @param info, app name in Application record.
     * @return ERR_OK ,return back success，others fail.
     */
    virtual int GetAllRunningProcesses(std::vector<RunningProcessInfo> &info) = 0;

    /**
     * GetRunningMultiAppInfoByBundleName, call GetRunningMultiAppInfoByBundleName() through proxy project.
     * Obtains information about multiapp that are running on the device.
     *
     * @param bundlename, bundle name in Application record.
     * @param info, output multiapp information.
     * @return ERR_OK ,return back success，others fail.
     */
    virtual int32_t GetRunningMultiAppInfoByBundleName(const std::string &bundleName,
        RunningMultiAppInfo &info) = 0;

    /**
     * GetAllRunningInstanceKeysBySelf, call GetAllRunningInstanceKeysBySelf() through proxy project.
     * Obtains running instance keys of multi-instance app that are running on the device.
     *
     * @param instanceKeys, output instance keys of the multi-instance app.
     * @return ERR_OK ,return back success，others fail.
     */
    virtual int32_t GetAllRunningInstanceKeysBySelf(std::vector<std::string> &instanceKeys) = 0;

    /**
     * GetAllRunningInstanceKeysByBundleName, call GetAllRunningInstanceKeysByBundleName() through proxy project.
     * Obtains running instance keys of multi-instance app that are running on the device.
     *
     * @param bundlename, bundle name in Application record.
     * @param instanceKeys, output instance keys of the multi-instance app.
     * @param userId, user id.
     * @return ERR_OK ,return back success，others fail.
     */
    virtual int32_t GetAllRunningInstanceKeysByBundleName(const std::string &bundleName,
        std::vector<std::string> &instanceKeys, int32_t userId = -1) = 0;

    /**
     * GetRunningProcessesByBundleType, call GetRunningProcessesByBundleType() through proxy project.
     * Obtains information about application processes by bundle type that are running on the device.
     *
     * @param bundleType, bundle type of the processes
     * @param info, app name in Application record.
     * @return ERR_OK ,return back success，others fail.
     */
    virtual int GetRunningProcessesByBundleType(const BundleType bundleType,
        std::vector<RunningProcessInfo> &info) = 0;

    /**
     * GetAllRenderProcesses, call GetAllRenderProcesses() through proxy project.
     * Obtains information about render processes that are running on the device.
     *
     * @param info, render process info.
     * @return ERR_OK, return back success, others fail.
     */
    virtual int GetAllRenderProcesses(std::vector<RenderProcessInfo> &info) = 0;

    /**
     * GetAllChildrenProcesses, call GetAllChildrenProcesses() through proxy project.
     * Obtains information about children processes that are running on the device.
     *
     * @param info, child process info.
     * @return ERR_OK, return back success, others fail.
     */
    virtual int GetAllChildrenProcesses(std::vector<ChildProcessInfo> &info) = 0;

    /**
     * JudgeSandboxByPid, call JudgeSandboxByPid() through proxy project.
     * Obtains information about application processes that are running on the device.
     *
     * @param pid, the pid of current app running record.
     * @param isSandbox, current app is or not a sandbox.
     * @return ERR_OK ,return back success，others fail.
     */
    virtual int32_t JudgeSandboxByPid(pid_t pid, bool &isSandbox)
    {
        return 0;
    }

    /**
     * IsTerminatingByPid, call IsTerminatingByPid() through proxy project.
     * Obtains information about application processes that are running on the device.
     *
     * @param pid, the pid of current app running record.
     * @param isTerminating, current app is or not terminating.
     * @return ERR_OK ,return back success，others fail.
     */
    virtual int32_t IsTerminatingByPid(pid_t pid, bool &isTerminating)
    {
        return 0;
    }

    /**
     * GetProcessRunningInfosByUserId, call GetProcessRunningInfosByUserId() through proxy project.
     * Obtains information about application processes that are running on the device.
     *
     * @param info, app name in Application record.
     * @param userId, user Id in Application record.
     * @return ERR_OK ,return back success，others fail.
     */
    virtual int GetProcessRunningInfosByUserId(std::vector<RunningProcessInfo> &info, int32_t userId) = 0;

    /**
     * GetProcessRunningInformation, call GetProcessRunningInformation() through proxy project.
     * Obtains information about current application process which is running on the device.
     *
     * @param info, app name in Application record.
     * @return ERR_OK ,return back success，others fail.
     */
    virtual int32_t GetProcessRunningInformation(RunningProcessInfo &info) = 0;

    /**
     * NotifyMemoryLevel, call NotifyMemoryLevel() through proxy project.
     * Notify abilities background the current memory level.
     *
     * @param level, the current memory level
     * @return ERR_OK ,return back success，others fail.
     */
    virtual int NotifyMemoryLevel(int32_t level) = 0;

    /**
     * NotifyProcMemoryLevel, call NotifyMemoryLevel() through proxy project.
     * Notify abilities the current memory level.
     *
     * @param procLevelMap ,<pid, level> map
     * @return ERR_OK ,return back success，others fail.
     */
    virtual int32_t NotifyProcMemoryLevel(const std::map<pid_t, MemoryLevel> &procLevelMap) = 0;

    /**
     * DumpHeapMemory, call DumpHeapMemory() through proxy project.
     * Get the application's memory allocation info.
     *
     * @param pid, pid input.
     * @param mallocInfo, dynamic storage information output.
     * @return ERR_OK ,return back success, others fail.
     */
    virtual int DumpHeapMemory(const int32_t pid, OHOS::AppExecFwk::MallocInfo &mallocInfo) = 0;

    /**
     * Notify that the ability stage has been updated
     * @param recordId, the app record.
     */
    virtual void AddAbilityStageDone(const int32_t recordId) = 0;

    /**
     * DumpJsHeapMemory, call DumpJsHeapMemory() through proxy project.
     * triggerGC and dump the application's jsheap memory info.
     *
     * @param info, pid tid needGc needSnapshot
     * @return ERR_OK ,return back success, others fail.
     */
    virtual int DumpJsHeapMemory(OHOS::AppExecFwk::JsHeapDumpInfo &info) = 0;

    /**
     * DumpCjHeapMemory, call DumpCjHeapMemory() through proxy project.
     * triggerGC and dump the application's cjheap memory info.
     *
     * @param info The information to be dumped
     * @return ERR_OK ,return back success, others fail.
     */
    virtual int DumpCjHeapMemory(OHOS::AppExecFwk::CjHeapDumpInfo &info) = 0;

    /**
     * DumpMem, call DumpMem() through proxy project.
     * triggerGC and dump application's memory info.
     *
     * @param info The information to be dumped
     * @param dumpResult The dump result string
     * @return ERR_OK ,return back success, others fail.
     */
    virtual int DumpMem(OHOS::AppExecFwk::MemDumpInfo &info, std::string &dumpResult) = 0;

    /**
     * Start a resident process
     */
    virtual void StartupResidentProcess(const std::vector<AppExecFwk::BundleInfo> &bundleInfos) = 0;

    /**
     * Register application or process state observer.
     * @param observer, ability token.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t RegisterApplicationStateObserver(const sptr<IApplicationStateObserver> &observer,
        const std::vector<std::string> &bundleNameList = {}) = 0;

    /**
     * Register application/process/ability state observer.
     * @param observer, bundleNameList, AppStateFilter.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t RegisterApplicationStateObserverWithFilter(sptr<IApplicationStateObserver> observer,
        const std::vector<std::string> &bundleNameList = {}, const AppStateFilter &appStateFilter = AppStateFilter(),
        bool isUsingFilter = false)
    {
        return 0;
    }

    virtual int32_t RegisterImageProcessStateObserver(const sptr<IImageProcessStateObserver> &observer)
    {
        return 0;
    }

    virtual int32_t UnregisterImageProcessStateObserver(const sptr<IImageProcessStateObserver> &observer)
    {
        return 0;
    }

    /**
     * Unregister application or process state observer.
     * @param observer, ability token.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t UnregisterApplicationStateObserver(const sptr<IApplicationStateObserver> &observer) = 0;

    /**
     * Register application or process state observer.
     * @param observer Is ability foreground state observer
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t RegisterAbilityForegroundStateObserver(const sptr<IAbilityForegroundStateObserver> &observer) = 0;

    /**
     * Unregister application or process state observer.
     * @param observer Is ability foreground state observer
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t UnregisterAbilityForegroundStateObserver(const sptr<IAbilityForegroundStateObserver> &observer) = 0;

    /**
     * Get foreground applications.
     * @param list, foreground apps.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t GetForegroundApplications(std::vector<AppStateData> &list) = 0;

    /**
     * Start user test process.
     * @param want, want object.
     * @param observer, test observer remote object.
     * @param bundleInfo, bundle info.
     * @param userId the user id.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int StartUserTestProcess(
        const AAFwk::Want &want, const sptr<IRemoteObject> &observer, const BundleInfo &bundleInfo, int32_t userId) = 0;

    /**
     * @brief Finish user test.
     * @param msg user test message.
     * @param resultCode user test result Code.
     * @param bundleName user test bundleName.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int FinishUserTest(const std::string &msg, const int64_t &resultCode, const std::string &bundleName) = 0;

    /**
     * Schedule accept want done.
     *
     * @param recordId Application record.
     * @param want Want.
     * @param flag flag get from OnAcceptWant.
     */
    virtual void ScheduleAcceptWantDone(const int32_t recordId, const AAFwk::Want &want, const std::string &flag) = 0;

    /**
     * @brief called when the module's onNewProcessRequest returns a flag
     * 
     * @param recordId app record id
     * @param want want parameter
     * @param flag process flag
     */
    virtual void ScheduleNewProcessRequestDone(const int32_t recordId, const AAFwk::Want &want,
        const std::string &flag) = 0;

    /**
     *  Get the token of ability records by process ID.
     *
     * @param pid The process id.
     * @param tokens The token of ability records.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int GetAbilityRecordsByProcessID(const int pid, std::vector<sptr<IRemoteObject>> &tokens) = 0;

    /**
     * Prestart nwebspawn process.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int PreStartNWebSpawnProcess() = 0;

    /**
     * Start nweb render process, called by nweb host.
     *
     * @param renderParam, params passed to renderProcess.
     * @param ipcFd, ipc file descriptor for web browser and render process.
     * @param sharedFd, shared memory file descriptor.
     * @param crashFd, crash signal file descriptor.
     * @param renderPid, created render pid.
     * @param isGPU, is or not gpu process
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int StartRenderProcess(const std::string &renderParam,
                                   int32_t ipcFd, int32_t sharedFd,
                                   int32_t crashFd, pid_t &renderPid, bool isGPU = false) = 0;

    /**
     * Render process call this to attach app manager service.
     *
     * @param renderScheduler, scheduler of render process.
     */
    virtual void AttachRenderProcess(const sptr<IRemoteObject> &renderScheduler) = 0;

    /**
     * Get render process termination status, called by nweb host.
     *
     * @param renderPid, target render pid.
     * @param status, termination status of the render process.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int GetRenderProcessTerminationStatus(pid_t renderPid, int &status) = 0;

    /**
     * GetConfiguration
     *
     * @param info to retrieve configuration data.
     * @return ERR_OK ,return back success，others fail.
     */
    virtual int32_t GetConfiguration(Configuration& config) = 0;

    virtual int32_t GetConfiguration(Configuration& config, int32_t userid) = 0;

    /**
     * UpdateConfiguration, ANotify application update system environment changes.
     *
     * @param config System environment change parameters.
     * @param userId configuration for the user
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t UpdateConfiguration(const Configuration &config, const int32_t userId = -1) = 0;

    virtual int32_t UpdateConfigurationByUserIds(
        const Configuration &config, const std::vector<int32_t> userIds) = 0;

    /**
     * UpdateConfigurationForBackgroundApp
     * 
     * @param appInfos Background application information.
     * @param policy Update policy.
     * @param userId configuration for the user
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t UpdateConfigurationForBackgroundApp(const std::vector<BackgroundAppInfo> &appInfos,
        const AppExecFwk::ConfigurationPolicy &policy, const int32_t userId = -1)
    {
        return 0;
    }

    /**
     *  Update config by bundle name.
     *
     * @param config Application environment change parameters.
     * @param name Application bundle name.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t UpdateConfigurationByBundleName(const Configuration &config, const std::string &name,
        int32_t appIndex = 0) = 0;

    /**
     * Register configuration observer.
     *
     * @param observer Configuration observer. When configuration changed, observer will be called.
     * @param userId The userId provided by caller.
     * @return Returns RESULT_OK on success, others on failure.
     */
    virtual int32_t RegisterConfigurationObserver(const sptr<IConfigurationObserver> &observer,
        const int32_t userId = -1) = 0;
    /**
     * Unregister configuration observer.
     *
     * @param observer Configuration observer.
     * @return Returns RESULT_OK on success, others on failure.
     */
    virtual int32_t UnregisterConfigurationObserver(const sptr<IConfigurationObserver> &observer) = 0;

    /**
     * @brief Get the running state of application by bundle name.
     *
     * @param bundleName Bundle name
     * @return Returns true if process is running, false if process isn't running.
     */
    virtual bool GetAppRunningStateByBundleName(const std::string &bundleName) = 0;

    /**
     * @brief Notify application load patch.
     *
     * @param bundleName Bundle name
     * @param callback called when LoadPatch finished.
     * @return Returns ERR_OK on success, error code on failure.
     */
    virtual int32_t NotifyLoadRepairPatch(const std::string &bundleName, const sptr<IQuickFixCallback> &callback) = 0;

    /**
     * @brief Notify application reload page.
     *
     * @param bundleName Bundle name
     * @param callback called when HotReload finished.
     * @return Returns ERR_OK on success, error code on failure.
     */
    virtual int32_t NotifyHotReloadPage(const std::string &bundleName, const sptr<IQuickFixCallback> &callback) = 0;

    /**
     * @brief Notify application unload patch.
     *
     * @param bundleName Bundle name
     * @param callback called when UnloadPatch finished.
     * @return Returns ERR_OK on success, error code on failure.
     */
    virtual int32_t NotifyUnLoadRepairPatch(const std::string &bundleName, const sptr<IQuickFixCallback> &callback) = 0;

    /**
     * Notify App Fault Data
     *
     * @param faultData the fault data.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t NotifyAppFault(const FaultData &faultData) = 0;

    /**
     * Notify App Fault Data By SA
     *
     * @param faultData the fault data notified by SA.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t NotifyAppFaultBySA(const AppFaultDataBySA &faultData) = 0;

    /**
     * Set Appfreeze Detect Filter
     *
     * @param pid the process pid.
     * @return Returns true on success, others on failure.
     */
    virtual bool SetAppFreezeFilter(int32_t pid) = 0;

#ifdef BGTASKMGR_CONTINUOUS_TASK_ENABLE
    /**
     * @brief Set whether the process is continuousTask.
     *
     * @param pid Process id.
     * @param isContinuousTask whether the process is continuousTask.
     * @return Returns 0 on success, error code on failure.
     */
    virtual int32_t SetContinuousTaskProcess(int32_t pid, bool isContinuousTask)
    {
        return 0;
    };
#endif

    /**
     * @brief Check whether the shared bundle is running.
     *
     * @param bundleName Shared bundle name.
     * @param versionCode Shared bundle version code.
     * @return Returns the shared bundle running result. The result is true if running, false otherwise.
     */
    virtual bool IsSharedBundleRunning(const std::string &bundleName, uint32_t versionCode) = 0;

   /**
     * start native process for debugger.
     *
     * @param want param to start a process.
     */
    virtual int32_t StartNativeProcessForDebugger(const AAFwk::Want &want) = 0;

    /**
     * Get bundleName by pid.
     *
     * @param pid process id.
     * @param bundleName Output parameters, return bundleName.
     * @param uid Output parameters, return userId.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t GetBundleNameByPid(const int pid, std::string &bundleName, int32_t &uid) = 0;

    /**
     * Get running process information by pid.
     *
     * @param pid process id.
     * @param info Output parameters, return runningProcessInfo.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t GetRunningProcessInfoByPid(const pid_t pid, OHOS::AppExecFwk::RunningProcessInfo &info)
    {
        return 0;
    }

    /**
     * Get running process information by child process pid.
     *
     * @param childPid child process id.
     * @param info Output parameters, return runningProcessInfo.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t GetRunningProcessInfoByChildProcessPid(const pid_t childPid,
        OHOS::AppExecFwk::RunningProcessInfo &info)
    {
        return 0;
    }

    /**
     * get memorySize by pid.
     *
     * @param pid process id.
     * @param memorySize Output parameters, return memorySize in KB.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t GetProcessMemoryByPid(const int32_t pid, int32_t &memorySize) = 0;

    /**
     * get application processes information list by bundleName.
     *
     * @param bundleName Bundle name.
     * @param userId user Id in Application record.
     * @param info Output parameters, return running process info list.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t GetRunningProcessInformation(
        const std::string &bundleName, int32_t userId, std::vector<RunningProcessInfo> &info) = 0;

    /**
     * @brief Notify AbilityManagerService the page show.
     * @param token Ability identify.
     * @param pageStateData The data of ability's page state.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t NotifyPageShow(const sptr<IRemoteObject> &token, const PageStateData &pageStateData)
    {
        return 0;
    }

    /**
     * @brief Notify AbilityManagerService the page hide.
     * @param token Ability identify.
     * @param pageStateData The data of ability's page state.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t NotifyPageHide(const sptr<IRemoteObject> &token, const PageStateData &pageStateData)
    {
        return 0;
    }

    /**
     * @brief Notify NativeEngine GC of status change.
     *
     * @param state GC state
     * @param pid pid
     *
     * @return Is the status change completed.
     */
    virtual int32_t ChangeAppGcState(pid_t pid, int32_t state, uint64_t tid = 0) = 0;

    /**
     * Register appRunning status listener.
     *
     * @param listener Running status listener.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t RegisterAppRunningStatusListener(const sptr<IRemoteObject> &listener) = 0;

    /**
     * Unregister appRunning status listener.
     *
     * @param listener Running status listener.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t UnregisterAppRunningStatusListener(const sptr<IRemoteObject> &listener) = 0;

	/**
     * Register application foreground state observer.
     * @param observer Is app foreground state observer
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t RegisterAppForegroundStateObserver(const sptr<IAppForegroundStateObserver> &observer) = 0;

    /**
     * Unregister application foreground state observer.
     * @param observer Is app foreground state observer.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t UnregisterAppForegroundStateObserver(const sptr<IAppForegroundStateObserver> &observer) = 0;

    /**
     * Check whether the bundle is running.
     *
     * @param bundleName Indicates the bundle name of the bundle.
     * @param isRunning Obtain the running status of the application, the result is true if running, false otherwise.
     * @return Return ERR_OK if success, others fail.
     */
    virtual int32_t IsApplicationRunning(const std::string &bundleName, bool &isRunning) = 0;

    /**
     * Check whether the bundle is running.
     *
     * @param bundleName Indicates the bundle name of the bundle.
     * @param appCloneIndex the appindex of the bundle.
     * @param isRunning Obtain the running status of the application, the result is true if running, false otherwise.
     * @return Return ERR_OK if success, others fail.
     */
    virtual int32_t IsAppRunning(const std::string &bundleName, int32_t appCloneIndex,
        bool &isRunning) = 0;

    /**
     * Check whether the process of the application under the specified user exists.
     *
     * @param bundleName Indicates the bundle name of the bundle.
     * @param userId the userId of the bundle.
     * @param isRunning Obtain the running status of the application, the result is true if running, false otherwise.
     * @return Return ERR_OK if success, others fail.
     */
    virtual int32_t IsAppRunningByBundleNameAndUserId(const std::string &bundleName, int32_t userId,
        bool &isRunning) = 0;

    virtual int32_t IsAppRunning(const std::string &bundleName, int32_t appCloneIndex,
        int32_t userId, bool &isRunning)
    {
        return 0;
    }

#ifdef SUPPORT_CHILD_PROCESS
    /**
     * Start child process, called by ChildProcessManager.
     *
     * @param childPid Created child process pid.
     * @param request Child process start request params.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t StartChildProcess(pid_t &childPid, const ChildProcessRequest &request) = 0;

    /**
     * Get child process record for self.
     *
     * @return child process info.
     */
    virtual int32_t GetChildProcessInfoForSelf(ChildProcessInfo &info) = 0;

    /**
     * Attach child process scheduler to app manager service.
     *
     * @param childScheduler scheduler of child process.
     */
    virtual void AttachChildProcess(const sptr<IRemoteObject> &childScheduler) = 0;

    /**
     * Exit child process, called by itself.
     */
    virtual void ExitChildProcessSafely() = 0;
#endif // SUPPORT_CHILD_PROCESS

    /**
     * Whether the current application process is the last surviving process.
     *
     * @return Returns true is final application process, others return false.
     */
    virtual bool IsFinalAppProcess()  = 0;

    /**
     * Register render state observer.
     * @param observer Render process state observer.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t RegisterRenderStateObserver(const sptr<IRenderStateObserver> &observer) = 0;

    /**
     * Unregister render state observer.
     * @param observer Render process state observer.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t UnregisterRenderStateObserver(const sptr<IRenderStateObserver> &observer) = 0;

    /**
     * Register KIA interceptor.
     * @param interceptor KIA interceptor.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t RegisterKiaInterceptor(const sptr<IKiaInterceptor> &interceptor) = 0;

    /**
     * Check if the given pid is a KIA process.
     * @param pid process id.
     * @return Returns true if it is a KIA process, false otherwise.
     */
    virtual int32_t CheckIsKiaProcess(pid_t pid, bool &isKia) = 0;

    /**
     * Update render state.
     * @param renderPid Render pid.
     * @param state foreground or background state.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t UpdateRenderState(pid_t renderPid, int32_t state) = 0;

    /**
     * @brief mark a process which is going restart.
     * @param uid the uid of the process.
     * @param instanceKey the instance key of the process.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t SignRestartAppFlag(int32_t uid, const std::string &instanceKey)
    {
        return 0;
    }

    /**
     * @brief mark a process which is going restart.
     * @param pid the pid of the process.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t SignRestartProcess(int32_t pid)
    {
        return 0;
    }

    /**
     * Get appRunningUniqueId by pid.
     * @param pid pid.
     * @param appRunningUniqueId appRunningUniqueId.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t GetAppRunningUniqueIdByPid(pid_t pid, std::string &appRunningUniqueId)
    {
        return 0;
    }

    /*
     * Get all uiextension root host process id, need apply permission ohos.permission.GET_RUNNING_INFO.
     * If specified pid mismatch UIExtensionAbility type, return empty vector.
     * @param pid Process id.
     * @param hostPids All host process id.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t GetAllUIExtensionRootHostPid(pid_t pid, std::vector<pid_t> &hostPids)
    {
        return 0;
    }

    /**
     * Get all uiextension provider process id, need apply permission ohos.permission.GET_RUNNING_INFO.
     * If specified hostPid didn't start any UIExtensionAbility, return empty vector.
     * @param hostPid Host process id.
     * @param providerPids All provider process id started by specified hostPid.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t GetAllUIExtensionProviderPid(pid_t hostPid, std::vector<pid_t> &providerPids)
    {
        return 0;
    }

    /**
     * @brief Notify memory size state changed: LOW_MEMORY, MEMORY_RECOVERY, REQUIRE_BIG_MEMORY, NO_REQUIRE_BIG_MEMORY.
     * @param memorySizeState Indicates the memory size state.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t NotifyMemorySizeStateChanged(int32_t memorySizeState)
    {
        return 0;
    }

    /**
     * @brief set support process cache by self
     */
    virtual int32_t SetSupportedProcessCacheSelf(bool isSupport) = 0;

    virtual int32_t SetSupportedProcessCache(int32_t pid, bool isSupport) = 0;

    virtual int32_t IsProcessCacheSupported(int32_t pid, bool &isSupported) = 0;

    virtual int32_t SetProcessCacheEnable(int32_t pid, bool enable) = 0;

    virtual int32_t LockProcessCache(int32_t pid, bool isLock)
    {
        return 0;
    }

    /**
     * Set application assertion pause state.
     *
     * @param flag assertion pause state.
     */
    virtual void SetAppAssertionPauseState(bool flag) {}

    /**
     * set browser channel for caller
     */
    virtual void SaveBrowserChannel(sptr<IRemoteObject> browser) = 0;

    /**
     * Check caller is test ability
     *
     * @param pid, the pid of ability.
     * @return Returns ERR_OK is test ability, others is not test ability.
     */
    virtual int32_t CheckCallingIsUserTestMode(const pid_t pid, bool &isUserTest)
    {
        return 0;
    }

#ifdef SUPPORT_CHILD_PROCESS
    /**
     * Start native child process, callde by ChildProcessManager.
     * @param libName lib file name to be load in child process
     * @param callback callback for notify start result
     * @param request, the request of child process.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t CreateNativeChildProcess(const std::string &libName,
        const sptr<IRemoteObject> &callback, const ChildProcessRequest &request)
    {
        return 0;
    }
    
    /**
     * Start native child process, callde by ChildProcessManager.
     * @param libName lib file name to be load in child process
     * @param childProcessCount current started child process count
     * @param callback callback for notify start result
 
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t CreateNativeChildProcess(const std::string &libName, int32_t childProcessCount,
        const sptr<IRemoteObject> &callback, const std::string &customProcessName)
    {
        return 0;
    };
#endif // SUPPORT_CHILD_PROCESS

     /**
     * Register native child exit callback to notify.
     * @param notify, Callback used to notify caller the info of native child exit.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t RegisterNativeChildExitNotify(const sptr<INativeChildNotify> notify)
    {
        return 0;
    }

    /**
     * Unregister native child exit callback to notify.
     * @param notify, Callback used to notify caller the info of native child exit.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t UnregisterNativeChildExitNotify(const sptr<INativeChildNotify> notify)
    {
        return 0;
    }

    /**
     * Notify that the process depends on web by itself.
     */
    virtual int32_t NotifyProcessDependedOnWeb()
    {
        return 0;
    }

    /**
     * Kill process depended on web by sa.
     */
    virtual void KillProcessDependedOnWeb()
    {
        return;
    }

    /**
     * Restart resident process depended on web.
     */
    virtual void RestartResidentProcessDependedOnWeb()
    {
        return;
    }

    /**
     * Get pids of processes which belong to specific bundle name and support process cache feature.
     * @param bundleName bundle name.
     * @param pidList pid list of processes that support process cache..
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t GetSupportedProcessCachePids(const std::string &bundleName, std::vector<int32_t> &pidList) = 0;

    virtual int32_t KillAppSelfWithInstanceKey(const std::string &instanceKey, bool clearPageStack,
        const std::string& reason)
    {
        return 0;
    }
    
    virtual int32_t KillProcessByPidForExit(int32_t pid, const std::string &reason)
    {
        return 0;
    }

    /**
     * Get if the specified module has been loaded.
     * @param want info to query module.
     * @param abilityInfo ability info.
     * @param result out parameter, true for loaded, else false.
     * @param isDebug out parameter, true for debug mode, else false.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t IsSpecifiedModuleLoaded(const AAFwk::Want &want, const AbilityInfo &abilityInfo, bool &result,
        bool &isDebug)
    {
        return 0;
    }

    /**
     * UpdateProcessMemoryState, call UpdateProcessMemoryState() through proxy object,
     * update process rss and pss value.
     *
     * @param procMemStates, the memory states of all apps.
     * @return ERR_OK, return back success, others fail.
     */
    virtual int32_t UpdateProcessMemoryState(const std::vector<ProcessMemoryState> &procMemState)
    {
        return 0;
    }

    /**
     * Launch ability when reuseDelegatorWindow is true.
     *
     * @param token Ability identify.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t LaunchAbility(sptr<IRemoteObject> token)
    {
        return 0;
    }

    virtual int32_t PromoteCurrentToCandidateMasterProcess(bool isInsertToHead)
    {
        return 0;
    }
    
    virtual int32_t DemoteCurrentFromCandidateMasterProcess()
    {
        return 0;
    }

    virtual int32_t ExitMasterProcessRole()
    {
        return 0;
    }
    
    /**
     * Query running shared bundle infos.
     *
     * @param pid process id.
     * @param sharedBundles shared bundle infos.
     * @return Returns ERR_OK if success, AAFwk::ERR_NO_APP_RECORD if app record not exist.
     */
    virtual int32_t QueryRunningSharedBundles(pid_t pid, std::map<std::string, uint32_t> &sharedBundles)
    {
        return 0;
    }

    virtual void SetSpecifiedProcessRequestId(int32_t recordId, int32_t requestId) {}

    virtual void AllowScbProcessMoveToBackground() {}

    virtual int32_t KillChildProcessByPid(int32_t pid)
    {
        return 0;
    }
    /**
     * Preload extension process.
     *
     * @param want contains bundleName + abilityName.
     * @param appIndex The index of application clone.
     * @param userId Indicates the user identification.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t PreloadExtension(const AAFwk::Want &want, int32_t appIndex, int32_t userId)
    {
        return 0;
    }

    virtual void SetProcessPrepareExit(int32_t pid) {}

    /**
     * Get all ability infos
     *
     * @param pid if pid is -1, query all ability infos, otherwise query ability infos for this pid
     * @param infos ability infos
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t GetAllAbilityInfos(const int32_t pid, std::vector<AppExecFwk::AbilityStateData> &infos)
    {
        return 0;
    }
};
}  // namespace AppExecFwk
}  // namespace OHOS

#endif  // OHOS_ABILITY_RUNTIME_APP_MGR_INTERFACE_H
