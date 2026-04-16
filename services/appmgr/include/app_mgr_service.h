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

#ifndef OHOS_ABILITY_RUNTIME_APP_MGR_SERVICE_H
#define OHOS_ABILITY_RUNTIME_APP_MGR_SERVICE_H

#include <list>
#include <string>
#include <vector>

#include "ability_info.h"
#include "ability_running_record.h"
#include "ams_mgr_scheduler.h"
#include "app_malloc_info.h"
#include "app_mgr_constants.h"
#include "app_mgr_service_event_handler.h"
#include "app_mgr_service_inner.h"
#include "app_mgr_stub.h"
#include "app_record_id.h"
#include "app_running_record.h"
#include "app_running_status_listener_interface.h"
#include "app_scheduler_proxy.h"
#include "appexecfwk_errors.h"
#include "application_info.h"
#include "application_state_filter.h"
#include "if_system_ability_manager.h"
#include "nocopyable.h"
#include "system_ability.h"
#include "task_handler_wrap.h"
#include "app_jsheap_mem_info.h"
#include "app_cjheap_mem_info.h"

namespace OHOS {
namespace AppExecFwk {
enum class ServiceRunningState { STATE_NOT_START, STATE_RUNNING };

struct AppMgrServiceState {
    ServiceRunningState serviceRunningState = ServiceRunningState::STATE_NOT_START;
    SpawnConnectionState connectionState = SpawnConnectionState::STATE_NOT_CONNECT;
};

class AMSEventHandler;

class AppMgrService : public SystemAbility, public AppMgrStub {
public:
    DECLEAR_SYSTEM_ABILITY(AppMgrService);

    AppMgrService();
    explicit AppMgrService(const int32_t serviceId, bool runOnCreate = false);
    virtual ~AppMgrService() override;

    // the function about application
    // attach the application to ability mgr, then ability mgr can control it.
    /**
     * AttachApplication, call AttachApplication() through proxy object,
     * get all the information needed to start the Application (data related to the Application ).
     *
     * @param app, information needed to start the Application.
     * @return
     */
    virtual void AttachApplication(const sptr<IRemoteObject> &app) override;

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
        AppExecFwk::PreloadMode preloadMode, int32_t appIndex) override;

    virtual int32_t MakeImage(const AAFwk::Want &want, int32_t userId,
        AppExecFwk::PreloadMode preloadMode, int32_t appIndex = 0,
        sptr<IImageErrorHandler> errorHandler = nullptr) override;

    virtual int32_t DestroyImage(uint64_t checkpointId, sptr<IImageErrorHandler> errorHandler = nullptr) override;

    virtual int32_t NotifyTemplateProcessDeepFrozen(int32_t pid) override;

    /**
     * ApplicationForegrounded, call ApplicationForegrounded() through proxy object,
     * set the application to Foreground State.
     *
     * @param recordId, a unique record that identifies this Application from others.
     * @return
     */
    virtual void ApplicationForegrounded(const int32_t recordId) override;

    /**
     * ApplicationBackgrounded, call ApplicationBackgrounded() through proxy object,
     * set the application to Backgrounded State.
     *
     * @param recordId, a unique record that identifies this Application from others.
     * @return
     */
    virtual void ApplicationBackgrounded(const int32_t recordId) override;

    /**
     * ApplicationTerminated, call ApplicationTerminated() through proxy object,
     * terminate the application.
     *
     * @param recordId, a unique record that identifies this Application from others.
     * @return
     */
    virtual void ApplicationTerminated(const int32_t recordId) override;

    /**
     * AbilityCleaned,call through AbilityCleaned() proxy project, clean Ability record.
     *
     * @param token, a unique record that identifies AbilityCleaned from others.
     * @return
     */
    virtual void AbilityCleaned(const sptr<IRemoteObject> &token) override;

    /**
     * ClearUpApplicationData, call ClearUpApplicationData() through proxy project,
     * clear the application data.
     *
     * @param bundleName, bundle name in Application record.
     * @param appCloneIndex the app clone id.
     * @param userId the user id.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t ClearUpApplicationData(const std::string &bundleName, int32_t appCloneIndex,
        int32_t userId = -1) override;

    /**
     * ClearUpApplicationDataBySelf, call ClearUpApplicationDataBySelf() through proxy project,
     * clear the application data by self.
     *
     * @param userId, user ID.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t ClearUpApplicationDataBySelf(int32_t userId = -1) override;

    /**
     * GetAllRunningProcesses, call GetAllRunningProcesses() through proxy project.
     * Obtains information about application processes that are running on the device.
     *
     * @param info, app name in Application record.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t GetAllRunningProcesses(std::vector<RunningProcessInfo> &info) override;

    /**
     * GetRunningMultiAppInfoByBundleName, call GetRunningMultiAppInfoByBundleName() through proxy project.
     * Obtains information about multiapp that are running on the device.
     *
     * @param bundlename, input.
     * @param info, output multiapp information.
     * @return ERR_OK ,return back success，others fail.
     */
    virtual int32_t GetRunningMultiAppInfoByBundleName(const std::string &bundleName,
        RunningMultiAppInfo &info) override;

    /**
     * GetAllRunningInstanceKeysBySelf, call GetAllRunningInstanceKeysBySelf() through proxy project.
     * Obtains running instance keys of multi-instance app that are running on the device.
     *
     * @param instanceKeys, output instance keys of the multi-instance app.
     * @return ERR_OK ,return back success，others fail.
     */
    virtual int32_t GetAllRunningInstanceKeysBySelf(std::vector<std::string> &instanceKeys) override;

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
        std::vector<std::string> &instanceKeys, int32_t userId = -1) override;

    /**
     * GetRunningProcessesByBundleType, call GetRunningProcessesByBundleType() through proxy project.
     * Obtains information about application processes by bundle type that are running on the device.
     *
     * @param bundleType, bundle type of the processes
     * @param info, app name in Application record.
     * @return ERR_OK ,return back success，others fail.
     */
    virtual int GetRunningProcessesByBundleType(const BundleType bundleType,
        std::vector<RunningProcessInfo> &info) override;

    /**
     * GetAllRenderProcesses, call GetAllRenderProcesses() through proxy project.
     * Obtains information about render processes that are running on the device.
     *
     * @param info, render process info.
     * @return ERR_OK, return back success, others fail.
     */
    virtual int32_t GetAllRenderProcesses(std::vector<RenderProcessInfo> &info) override;

    /**
     * GetAllChildrenProcesses, call GetAllChildrenProcesses() through proxy project.
     * Obtains information about children processes that are running on the device.
     *
     * @param info, child process info.
     * @return ERR_OK, return back success, others fail.
     */
    virtual int GetAllChildrenProcesses(std::vector<ChildProcessInfo> &info) override;

    /**
     * IsTerminatingByPid, call IsTerminatingByPid() through proxy project.
     * Obtains information about application processes that are running on the device.
     *
     * @param pid, the pid of current app running record.
     * @param isTerminating, current app is or not terminating.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t IsTerminatingByPid(pid_t pid, bool &isTerminating) override;

    /**
     * GetProcessRunningInfosByUserId, call GetProcessRunningInfosByUserId() through proxy project.
     * Obtains information about application processes that are running on the device.
     *
     * @param info, app name in Application record.
     * @param userId, userId.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t GetProcessRunningInfosByUserId(std::vector<RunningProcessInfo> &info, int32_t userId) override;

    /**
     * GetProcessRunningInformation, call GetProcessRunningInformation() through proxy project.
     * Obtains information about current application process which is running on the device.
     *
     * @param info, app name in Application record.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t GetProcessRunningInformation(RunningProcessInfo &info) override;

    /**
     * NotifyMemoryLevel, call NotifyMemoryLevel() through proxy project.
     * Notify applications background the current memory level.
     *
     * @param level, current memory level.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t NotifyMemoryLevel(int32_t level) override;

    /**
     * NotifyProcMemoryLevel, call NotifyMemoryLevel() through proxy project.
     * Notify applications the current memory level.
     *
     * @param  procLevelMap , <pid_t, MemoryLevel> map.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t NotifyProcMemoryLevel(const std::map<pid_t, MemoryLevel> &procLevelMap) override;

    /**
     * DumpHeapMemory, call DumpHeapMemory() through proxy project.
     * Get the application's memory allocation info.
     *
     * @param pid, pid input.
     * @param mallocInfo, dynamic storage information output.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t DumpHeapMemory(const int32_t pid, OHOS::AppExecFwk::MallocInfo &mallocInfo) override;

    /**
     * Authenticate dump permissions
     *
     * @return Returns true on permission, others on false
     */
    bool HasDumpPermission() const;
    /**
     * DumpJsHeapMemory, call DumpJsHeapMemory() through proxy project.
     * triggerGC and dump the application's jsheap memory info.
     *
     * @param info, pid, tid, needGc, needSnapshot
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t DumpJsHeapMemory(OHOS::AppExecFwk::JsHeapDumpInfo &info) override;

    /**
     * DumpCjHeapMemory, call DumpCJHeapMemory() through proxy project.
     * triggerGC and dump the application's cjheap memory info.
     *
     * @param info, pid, needGc, needSnapshot
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t DumpCjHeapMemory(OHOS::AppExecFwk::CjHeapDumpInfo &info) override;
    /**
     * DumpMem, call DumpMem() through proxy project.
     * triggerGC and dump application's memory info.
     *
     * @param info The information to be dumped
     * @param dumpResult The dump result string
     * @return ERR_OK ,return back success, others fail.
     */
    virtual int32_t DumpMem(OHOS::AppExecFwk::MemDumpInfo &info, std::string &dumpResult) override;

    // the function about service running info
    /**
     * QueryServiceState, Query application service status.
     *
     * @return the application service status.
     */
    AppMgrServiceState QueryServiceState();

    /**
     * GetAmsMgr, call GetAmsMgr() through proxy object, get AMS interface instance.
     *
     * @return sptr<IAmsMgr>, return to AMS interface instance.
     */
    virtual sptr<IAmsMgr> GetAmsMgr() override;

    /**
     * Notify that the ability stage has been updated
     * @param recordId, the app record.
     */
    virtual void AddAbilityStageDone(const int32_t recordId) override;

    /**
     * Start all resident process
     */
    virtual void StartupResidentProcess(const std::vector<AppExecFwk::BundleInfo> &bundleInfos) override;

    /**
     * Start user test process.
     * @param want, want object.
     * @param observer, test observer remote object.
     * @param bundleInfo, bundle info.
     * @param userId the user id.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int StartUserTestProcess(const AAFwk::Want &want, const sptr<IRemoteObject> &observer,
        const AppExecFwk::BundleInfo &bundleInfo, int32_t userId) override;

    /**
     * @brief Finish user test.
     * @param msg user test message.
     * @param resultCode user test result Code.
     * @param bundleName user test bundleName.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int FinishUserTest(
        const std::string &msg, const int64_t &resultCode, const std::string &bundleName) override;

    /**
     * @brief Application hidumper.
     * @param fd Indicates the fd.
     * @param args Indicates the params.
     * @return Returns the dump result.
     */
    int Dump(int fd, const std::vector<std::u16string>& args) override;

    /**
     * @brief called when the abilityStage's onAcceptWant lifecycle completed.
     * @param recordId record id of the ability.
     * @param want want has been accepted.
     * @param flag flag get from OnAcceptWant.
     */
    virtual void ScheduleAcceptWantDone(
        const int32_t recordId, const AAFwk::Want &want, const std::string &flag) override;

    /**
     * @brief called when the module's onNewProcessRequest returns a flag
     * 
     * @param recordId app record id
     * @param want want parameter
     * @param flag process flag
     */
    virtual void ScheduleNewProcessRequestDone(
        const int32_t recordId, const AAFwk::Want &want, const std::string &flag) override;

    /**
     *  Get the token of ability records by process ID.
     *
     * @param pid The process id.
     * @param tokens The token of ability records.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int GetAbilityRecordsByProcessID(const int pid, std::vector<sptr<IRemoteObject>> &tokens) override;

    /**
     * Prestart nwebspawn process.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int PreStartNWebSpawnProcess() override;

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
                                   int32_t crashFd, pid_t &renderPid, bool isGPU = false) override;

    /**
     * Render process call this to attach to app manager service.
     *
     * @param renderScheduler scheduler of render process.
     */
    virtual void AttachRenderProcess(const sptr<IRemoteObject> &shceduler) override;

    /**
     * Get render process termination status, called by nweb host.
     *
     * @param renderPid, target render pid.
     * @param status, termination status of the render process.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int GetRenderProcessTerminationStatus(pid_t renderPid, int &status) override;

    /**
     * GetConfiguration
     *
     * @param info to retrieve configuration data.
     * @return ERR_OK ,return back success，others fail.
     */
    virtual int32_t GetConfiguration(Configuration& config) override;
    virtual int32_t GetConfiguration(Configuration& config, int32_t userId) override;

    /**
     * UpdateConfiguration, ANotify application update system environment changes.
     *
     * @param config System environment change parameters.
     * @param userId configuration for the user
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t UpdateConfiguration(const Configuration &config, const int32_t userId = -1) override;
    
    virtual int32_t UpdateConfigurationByUserIds(
        const Configuration &config, const std::vector<int32_t> userIds = {}) override;

    /**
     * Internally, an asynchronous task will be initiated and tasks in the task list will be scheduled according to
     * the policy.
     * If during the execution of a task, a caller triggers UpdateConfiguration or UpdateConfigurationForBackgroundApp,
     * the asynchronous task will be terminated.
     * If the task has not been executed and the app is switched to the foreground, the task will be updated
     * when brought to the foreground. The task will not be updated while in the background.
     * Supported scope: colormode
     * Only SA can call this method.
     * 
     * @param appInfos Background application information.
     * @param policy Update policy.
     * @param userId configuration for the user
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t UpdateConfigurationForBackgroundApp(const std::vector<BackgroundAppInfo> &appInfos,
        const AppExecFwk::ConfigurationPolicy &policy, const int32_t userId = -1) override;

    /**
     *  Update config by bundle name.
     *
     * @param config Application environment change parameters.
     * @param name Application bundle name.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t UpdateConfigurationByBundleName(const Configuration &config, const std::string &name,
        int32_t appIndex = 0) override;

    /**
     * @brief register a configuration observer which will receive notifies when updated.
     * @param observer the configuration observer to receive notify.
     * @param userId The userId provided by caller.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t RegisterConfigurationObserver(const sptr<IConfigurationObserver> &observer,
        const int32_t userId = -1) override;
    /**
     * @brief unregister a configuration observer registered before.
     * @param observer the configuration observer registered before.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t UnregisterConfigurationObserver(const sptr<IConfigurationObserver> &observer) override;

    /**
     * Register KIA interceptor.
     * @param interceptor KIA interceptor.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t RegisterKiaInterceptor(const sptr<IKiaInterceptor> &interceptor) override;

    /**
     * Check if the given pid is a KIA process.
     * @param pid process id.
     * @return Returns true if it is a KIA process, false otherwise.
     */
    virtual int32_t CheckIsKiaProcess(pid_t pid, bool &isKia) override;

    /**
     * @brief Get the running state of application by bundle name.
     *
     * @param bundleName Bundle name
     * @return Returns true if process is running, false if process isn't running.
     */
    bool GetAppRunningStateByBundleName(const std::string &bundleName) override;

    /**
     * @brief Notify application load patch.
     *
     * @param bundleName Bundle name
     * @param callback called when LoadPatch finished.
     * @return Returns ERR_OK on success, error code on failure.
     */
    int32_t NotifyLoadRepairPatch(const std::string &bundleName, const sptr<IQuickFixCallback> &callback) override;

    /**
     * @brief Notify application reload page.
     *
     * @param bundleName Bundle name
     * @param callback called when HotReload finished.
     * @return Returns ERR_OK on success, error code on failure.
     */
    int32_t NotifyHotReloadPage(const std::string &bundleName, const sptr<IQuickFixCallback> &callback) override;

    /**
     * @brief Notify application unload patch.
     *
     * @param bundleName Bundle name
     * @param callback called when UnloadPatch finished.
     * @return Returns ERR_OK on success, error code on failure.
     */
    int32_t NotifyUnLoadRepairPatch(const std::string &bundleName, const sptr<IQuickFixCallback> &callback) override;

#ifdef BGTASKMGR_CONTINUOUS_TASK_ENABLE
    int32_t SetContinuousTaskProcess(int32_t pid, bool isContinuousTask) override;
#endif

    /**
     * @brief Check whether the shared bundle is running.
     *
     * @param bundleName Shared bundle name.
     * @param versionCode Shared bundle version code.
     * @return Returns the shared bundle running result. The result is true if running, false otherwise.
     */
    virtual bool IsSharedBundleRunning(const std::string &bundleName, uint32_t versionCode) override;

    /**
     * start native process for debugger.
     *
     * @param want param to start a process.
     */
    virtual int32_t StartNativeProcessForDebugger(const AAFwk::Want &want) override;

    /**
     * Get bundleName by pid.
     *
     * @param pid process id.
     * @param bundleName Output parameters, return bundleName.
     * @param uid Output parameters, return userId.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t GetBundleNameByPid(const int32_t pid, std::string &bundleName, int32_t &uid) override;

    /**
     * Get running process information by pid.
     *
     * @param pid process id.
     * @param info Output parameters, return runningProcessInfo.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t GetRunningProcessInfoByPid(const pid_t pid, OHOS::AppExecFwk::RunningProcessInfo &info) override;

    /**
     * Get running process information by child process pid.
     *
     * @param childPid child process id.
     * @param info Output parameters, return runningProcessInfo.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t GetRunningProcessInfoByChildProcessPid(const pid_t childPid,
        OHOS::AppExecFwk::RunningProcessInfo &info) override;

    /**
     * Notify Fault Data
     *
     * @param faultData the fault data.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t NotifyAppFault(const FaultData &faultData) override;

    /**
     * Notify Fault Data By SA
     *
     * @param faultData the fault data notified by SA.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t NotifyAppFaultBySA(const AppFaultDataBySA &faultData) override;

    /**
     * Set Appfreeze Detect Filter
     *
     * @param pid the process pid.
     * @return Returns true on success, others on failure.
     */
    bool SetAppFreezeFilter(int32_t pid) override;

    /**
     * get memorySize by pid.
     *
     * @param pid process id.
     * @param memorySize Output parameters, return memorySize in KB.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t GetProcessMemoryByPid(const int32_t pid, int32_t &memorySize) override;

    /**
     * get application processes information list by bundleName.
     *
     * @param bundleName Bundle name.
     * @param userId user Id in Application record.
     * @param info Output parameters, return running process info list.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t GetRunningProcessInformation(
        const std::string &bundleName, int32_t userId, std::vector<RunningProcessInfo> &info) override;

    /**
     * on add systemAbility.
     *
     * @return
     */
    virtual void OnAddSystemAbility(int32_t systemAbilityId, const std::string& deviceId) override;

    /**
     * on remove systemAbility.
     *
     * @return
     */
    virtual void OnRemoveSystemAbility(int32_t systemAbilityId, const std::string& deviceId) override;

    /**
     * @brief Notify NativeEngine GC of status change.
     *
     * @param state GC state
     * @param pid pid
     *
     * @return Is the status change completed.
     */
    virtual int32_t ChangeAppGcState(pid_t pid, int32_t state, uint64_t tid = 0) override;

    /**
     * Register appRunning status listener.
     *
     * @param listener Running status listener.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t RegisterAppRunningStatusListener(const sptr<IRemoteObject> &listener) override;

    /**
     * Unregister appRunning status listener.
     *
     * @param listener Running status listener.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t UnregisterAppRunningStatusListener(const sptr<IRemoteObject> &listener) override;

    /**
     * Register application foreground state observer.
     * @param observer Is App Foreground Statue Observer
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t RegisterAppForegroundStateObserver(const sptr<IAppForegroundStateObserver> &observer) override;

    /**
     * Unregister application foreground state observer.
     * @param observer Is App Foreground Statue Observer
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t UnregisterAppForegroundStateObserver(const sptr<IAppForegroundStateObserver> &observer) override;

#ifdef SUPPORT_CHILD_PROCESS
    /**
     * Start child process, called by ChildProcessManager.
     *
     * @param childPid Created child process pid.
     * @param request Child process start request params.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t StartChildProcess(pid_t &childPid, const ChildProcessRequest &request) override;

    /**
     * Get child process record for self.
     *
     * @return child process info.
     */
    int32_t GetChildProcessInfoForSelf(ChildProcessInfo &info) override;

    /**
     * Attach child process scheduler to app manager service.
     *
     * @param childScheduler scheduler of child process.
     */
    void AttachChildProcess(const sptr<IRemoteObject> &childScheduler) override;

    /**
     * Exit child process, called by itself.
     */
    void ExitChildProcessSafely() override;
#endif // SUPPORT_CHILD_PROCESS

    /**
     * @brief register a render process state observer to receive change.
     * @param observer the render process state observer to receive notify.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t RegisterRenderStateObserver(const sptr<IRenderStateObserver> &observer) override;

    /**
     * @brief unregister a render process state observer registered before.
     * @param observer the render process state observer registered before.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t UnregisterRenderStateObserver(const sptr<IRenderStateObserver> &observer) override;

    /**
     * @brief update a render process state.
     * @param renderPid the pid of the render process to be updated.
     * @param state the new state value of the render process.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t UpdateRenderState(pid_t renderPid, int32_t state) override;

    /**
     * @brief mark a process which is going restart.
     * @param uid the uid of the process.
     * @param instanceKey the instance key of the process.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t SignRestartAppFlag(int32_t uid, const std::string &instanceKey) override;

    /**
     * @brief mark a process which is going restart.
     * @param pid the pid of the process.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t SignRestartProcess(int32_t pid) override;

    /**
     * Get appRunningUniqueId by pid.
     * @param pid pid.
     * @param appRunningUniqueId appRunningUniqueId.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t GetAppRunningUniqueIdByPid(pid_t pid, std::string &appRunningUniqueId) override;

    /*
     * Get all uiextension root host process id, need apply permission ohos.permission.GET_RUNNING_INFO.
     * If specified pid mismatch UIExtensionAbility type, return empty vector.
     * @param pid Process id.
     * @param hostPids All host process id.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t GetAllUIExtensionRootHostPid(pid_t pid, std::vector<pid_t> &hostPids) override;

    /**
     * Get all uiextension provider process id, need apply permission ohos.permission.GET_RUNNING_INFO.
     * If specified hostPid didn't start any UIExtensionAbility, return empty vector.
     * @param hostPid Host process id.
     * @param providerPids All provider process id started by specified hostPid.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t GetAllUIExtensionProviderPid(pid_t hostPid, std::vector<pid_t> &providerPids) override;

    /**
     * @brief Notify memory size state changed: LOW_MEMORY, MEMORY_RECOVERY, REQUIRE_BIG_MEMORY, NO_REQUIRE_BIG_MEMORY.
     * @param memorySizeState Indicates the memory size state.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t NotifyMemorySizeStateChanged(int32_t memorySizeState) override;

    /**
     * Set application assertion pause state.
     *
     * @param flag assertion pause state.
     */
    void SetAppAssertionPauseState(bool flag) override;

    /**
     * @brief set support process cache by self
     */
    int32_t SetSupportedProcessCacheSelf(bool isSupport) override;

    int32_t SetSupportedProcessCache(int32_t pid, bool isSupport) override;

    int32_t IsProcessCacheSupported(int32_t pid, bool &isSupported) override;

    int32_t SetProcessCacheEnable(int32_t pid, bool enable) override;

    int32_t LockProcessCache(int32_t pid, bool isLock) override;

    /**
     * Elevate the current process to be a candidate master process.
     *
     * @param isInsertToHead Whether insert current process to the head of candidate master process list.
     * @return Return ERR_OK if success, others fail.
     */
    int32_t PromoteCurrentToCandidateMasterProcess(bool isInsertToHead) override;
    
    /**
     * Revoke current process as a candidate master process.
     *
     * @return Return ERR_OK if success, others fail.
     */
    int32_t DemoteCurrentFromCandidateMasterProcess() override;

    /**
     * Exit from the master process role of the current process.
     *
     * @return Return ERR_OK if success, others fail.
     */
    int32_t ExitMasterProcessRole() override;

    void SetSpecifiedProcessRequestId(int32_t recordId, int32_t requestId) override;

    /**
     * set browser channel for caller
     */
    virtual void SaveBrowserChannel(sptr<IRemoteObject> browser) override;

    /**
     * Check caller is test ability
     *
     * @param pid, the pid of ability.
     * @return Returns ERR_OK is test ability, others is not test ability.
     */
    int32_t CheckCallingIsUserTestMode(const pid_t pid, bool &isUserTest) override;

#ifdef SUPPORT_CHILD_PROCESS
    /**
     * Start native child process, callde by ChildProcessManager.
     * @param libName lib file name to be load in child process
     * @param callback callback for notify start result
     * @param request, the request of child process
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t CreateNativeChildProcess(const std::string &libName,
        const sptr<IRemoteObject> &callback, const ChildProcessRequest &request) override;
#endif // SUPPORT_CHILD_PROCESS

    /**
     * Register callback to notify native child exit.
     * @param notify, callback to notify
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t RegisterNativeChildExitNotify(const sptr<INativeChildNotify> notify) override;

    /**
     * Unregister callback to notify native child exit.
     * @param notify, callback to notify
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t UnregisterNativeChildExitNotify(const sptr<INativeChildNotify> notify) override;

    /**
     * Notify that the process depends on web by itself.
     */
    virtual int32_t NotifyProcessDependedOnWeb() override;

    /**
     * Kill process depended on web by sa.
     */
    virtual void KillProcessDependedOnWeb() override;

    /**
     * Restart resident process depended on web.
     */
    virtual void RestartResidentProcessDependedOnWeb() override;

    virtual int32_t KillAppSelfWithInstanceKey(const std::string &instanceKey, bool clearPageStack,
        const std::string& reason) override;
    int32_t KillProcessByPidForExit(int32_t pid, const std::string &reason) override;

    /**
     * Get if the specified module has been loaded.
     * @param want info to query module.
     * @param abilityInfo ability info.
     * @param result out parameter, true for loaded, else false.
     * @param isDebug out parameter, true for debug mode, else false.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t IsSpecifiedModuleLoaded(const AAFwk::Want &want, const AbilityInfo &abilityInfo, bool &result,
        bool &isDebug) override;

    /**
     * update process rss and pss value.
     *
     * @param procMemStates, the memory states of all apps.
     * @return ERR_OK, return back success, others fail.
     */
    virtual int32_t UpdateProcessMemoryState(const std::vector<ProcessMemoryState> &procMemState) override;

    /**
     * Launch ability when reuseDelegatorWindow is true.
     *
     * @param token Ability identify.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t LaunchAbility(sptr<IRemoteObject> token) override;
    
    virtual int32_t QueryRunningSharedBundles(pid_t pid, std::map<std::string, uint32_t> &sharedBundles) override;

    /**
     * Register application/process/ability state observer.
     * @param observer, bundleNameList, appStateFilter.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t RegisterApplicationStateObserverWithFilter(sptr<IApplicationStateObserver> observer,
        const std::vector<std::string> &bundleNameList = {},
        const AppStateFilter &appStateFilter = AppStateFilter(),
        bool isUsingFilter = false) override;
    
    void AllowScbProcessMoveToBackground() override;

    int32_t KillChildProcessByPid(int32_t pid) override;

    /**
     * Preload extension process.
     *
     * @param want contains bundleName + abilityName.
     * @param appIndex The index of application clone.
     * @param userId Indicates the user identification.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t PreloadExtension(const AAFwk::Want &want, int32_t appIndex, int32_t userId) override;

    /**
     * Get all ability infos
     *
     * @param pid if pid is -1, query all ability infos, otherwise query ability infos for this pid
     * @param infos ability infos
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t GetAllAbilityInfos(const int32_t pid, std::vector<AppExecFwk::AbilityStateData> &infos) override;

private:
    /**
     * Init, Initialize application services.
     *
     * @return ERR_OK, return back success, others fail.
     */
    ErrCode Init();

    // the function that overrode from SystemAbility
    /**
     * OnStart, Start application service.
     *
     * @return
     */
    virtual void OnStart() override;

    /**
     * OnStop, Stop application service.
     *
     * @return
     */
    virtual void OnStop() override;

    /**
     * @brief Judge whether the application service is ready.
     *
     * @return Returns true means service is ready, otherwise service is not ready.
     */
    bool IsReady() const;

    /**
     * SetInnerService, Setting application service Inner instance.
     *
     * @return
     */
    void SetInnerService(const std::shared_ptr<AppMgrServiceInner> &innerService);

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

    int32_t RegisterImageProcessStateObserver(const sptr<IImageProcessStateObserver> &observer) override;

    int32_t UnregisterImageProcessStateObserver(const sptr<IImageProcessStateObserver> &observer) override;

    /**
     * Register application or process state observer.
     * @param observer, Is ability foreground state observer
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t RegisterAbilityForegroundStateObserver(const sptr<IAbilityForegroundStateObserver> &observer) override;

    /**
     * Unregister application or process state observer.
     * @param observer, Is ability foreground state observer
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t UnregisterAbilityForegroundStateObserver(const sptr<IAbilityForegroundStateObserver> &observer) override;

    /**
     * Get foreground applications.
     * @param list, foreground apps.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t GetForegroundApplications(std::vector<AppStateData> &list) override;

    int Dump(const std::vector<std::u16string>& args, std::string& result);
    int ShowHelp(const std::vector<std::u16string>& args, std::string& result);
    int DumpIpc(const std::vector<std::u16string>& args, std::string& result);
    int DumpIpcAllStart(std::string& result);
    int DumpIpcAllStop(std::string& result);
    int DumpIpcAllStat(std::string& result);
    int DumpIpcStart(const int32_t pid, std::string& result);
    int DumpIpcStop(const int32_t pid, std::string& result);
    int DumpIpcStat(const int32_t pid, std::string& result);

    int DumpFfrt(const std::vector<std::u16string>& args, std::string& result);

    bool JudgeAppSelfCalled(int32_t recordId);

    /**
     * @brief Notify AbilityManagerService the page show.
     * @param token Ability identify.
     * @param pageStateData The data of ability's page state.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t NotifyPageShow(const sptr<IRemoteObject> &token, const PageStateData &pageStateData) override;

    /**
     * @brief Notify AbilityManagerService the page hide.
     * @param token Ability identify.
     * @param pageStateData The data of ability's page state.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t NotifyPageHide(const sptr<IRemoteObject> &token, const PageStateData &pageStateData) override;

    /**
     * Check whether the bundle is running.
     *
     * @param bundleName Indicates the bundle name of the bundle.
     * @param isRunning Obtain the running status of the application, the result is true if running, false otherwise.
     * @return Return ERR_OK if success, others fail.
     */
    int32_t IsApplicationRunning(const std::string &bundleName, bool &isRunning) override;

    /**
     * Check whether the bundle is running.
     *
     * @param bundleName Indicates the bundle name of the bundle.
     * @param appCloneIndex the appindex of the bundle.
     * @param isRunning Obtain the running status of the application, the result is true if running, false otherwise.
     * @return Return ERR_OK if success, others fail.
     */
    int32_t IsAppRunning(const std::string &bundleName, int32_t appCloneIndex, bool &isRunning) override;

    /**
     * Check whether the process of the application under the specified user exists.
     *
     * @param bundleName Indicates the bundle name of the bundle.
     * @param userId the userId of the bundle.
     * @param isRunning Obtain the running status of the application, the result is true if running, false otherwise.
     * @return Return ERR_OK if success, others fail.
     */
    int32_t IsAppRunningByBundleNameAndUserId(const std::string &bundleName, int32_t userId, bool &isRunning) override;

    int32_t IsAppRunning(const std::string &bundleName, int32_t appCloneIndex,
        int32_t userId, bool &isRunning) override;

    /**
     * Whether the current application process is the last surviving process.
     *
     * @return Returns true is final application process, others return false.
     */
    bool IsFinalAppProcess() override;

    /**
     * Get pids of processes which belong to specific bundle name and support process cache feature.
     *
     * @param bundleName bundle name.
     * @param pidList pid list of processes that support process cache.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t GetSupportedProcessCachePids(const std::string &bundleName,
        std::vector<int32_t> &pidList) override;

    /**
     * Launch Application Finished
     * @param recordId id of the app record
     */
    virtual void PreloadModuleFinished(const int32_t recordId) override;

    void SetProcessPrepareExit(int32_t pid) override;

    enum DumpIpcKey {
        KEY_DUMP_IPC_START = 0,
        KEY_DUMP_IPC_STOP,
        KEY_DUMP_IPC_STAT,
    };

private:
    int DumpIpcAllInner(const AppMgrService::DumpIpcKey key, std::string& result);
    int DumpIpcWithPidInner(const AppMgrService::DumpIpcKey key,
        const std::string& optionPid, std::string& result);

    int DumpFfrtInner(const std::string& pidsRaw, std::string& result);
    int32_t DumpParsePids(const std::string &pidsRaw, std::vector<int32_t> &pids);
    int32_t DumpArkWeb(const std::vector<std::u16string> &args, std::string &result);

private:
    AppMgrServiceState appMgrServiceState_;
    std::shared_ptr<AppMgrServiceInner> appMgrServiceInner_;
    std::shared_ptr<AAFwk::TaskHandlerWrap> taskHandler_;
    std::shared_ptr<AMSEventHandler> eventHandler_;
    sptr<ISystemAbilityManager> systemAbilityMgr_;
    sptr<IAmsMgr> amsMgrScheduler_;

    bool GetDumpIpcKeyByOption(const std::string &option, DumpIpcKey &key);

    DISALLOW_COPY_AND_MOVE(AppMgrService);
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_APP_MGR_SERVICE_H
