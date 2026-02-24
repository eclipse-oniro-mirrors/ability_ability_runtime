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

#ifndef OHOS_ABILITY_RUNTIME_MAIN_THREAD_H
#define OHOS_ABILITY_RUNTIME_MAIN_THREAD_H

#include <string>
#include <signal.h>
#include <mutex>

#include "ability_record_mgr.h"
#include "app_jsheap_mem_info.h"
#include "app_cjheap_mem_info.h"
#include "app_malloc_info.h"
#include "app_mgr_interface.h"
#include "app_scheduler_host.h"
#include "application_impl.h"
#include "assert_fault_task_thread.h"
#include "common_event_subscriber.h"
#include "event_handler.h"
#include "extension_config_mgr.h"
#include "idle_time.h"
#include "inner_event.h"
#include "ipc_singleton.h"
#include "js_runtime.h"
#include "native_engine/native_engine.h"
#include "overlay_event_subscriber.h"
#include "resource_manager.h"
#include "runtime.h"
#include "watchdog.h"

#ifdef CJ_FRONTEND
#include "cj_envsetup.h"
#endif
#define ABILITY_LIBRARY_LOADER

#if defined(NWEB) && defined(NWEB_GRAPHIC)
#include "nweb_preload.h"
#include "ui/rs_surface_node.h"
#endif

class Runtime;
namespace OHOS {
namespace JsEnv {
struct ErrorObject;
using UncatchableTask = std::function<void(std::string summary, const JsEnv::ErrorObject errorObject, napi_env env,
    napi_value exception)>;
}  // namespace JsEnv
namespace EtsEnv {
struct ETSUncaughtExceptionInfo;
} // namespace EtsEnv
namespace AppExecFwk {
using namespace OHOS::Global;
using OHOS::AbilityRuntime::Runtime;
struct BaseSharedBundleInfo;
using HspList = std::vector<BaseSharedBundleInfo>;
enum class MainThreadState { INIT, ATTACH, READY, RUNNING };
struct BundleInfo;
struct PluginBundleInfo;
struct UncatchableTaskInfo {
    std::string bundleName;
    uint32_t versionCode;
    std::string appRunningId;
    int32_t pid;
    std::string processName;
};
struct RuntimeUpdateParam {
    AbilityRuntime::Runtime::Options option;
    UncatchableTaskInfo uncatchableTaskInfo;
    std::string hapPath;
};
struct ProcessExitInfo {
    std::string bundleName;
    std::string errorObjectName;
    std::string summary;
    std::string appRunningId;
    std::string processName;
    int32_t pid;
    bool foreground;
    bool isUncatchable;
};
class ContextDeal;
struct ModuleTestRunner;
// class Global::Resource::ResourceManager;
class AppMgrDeathRecipient : public IRemoteObject::DeathRecipient {
public:
    /**
     *
     * @brief Notify the AppMgrDeathRecipient that the remote is dead.
     *
     * @param remote The remote which is dead.
     */
    virtual void OnRemoteDied(const wptr<IRemoteObject> &remote) override;
    AppMgrDeathRecipient() = default;
    ~AppMgrDeathRecipient() override = default;
};

class MainThread : public AppSchedulerHost {
    DECLARE_DELAYED_IPCSINGLETON(MainThread);

public:
    /**
     *
     * @brief Get the current MainThreadState.
     *
     * @return Returns the current MainThreadState.
     */
    MainThreadState GetMainThreadState() const;

    /**
     *
     * @brief Get the runner state of mainthread.
     *
     * @return Returns the runner state of mainthread.
     */
    bool GetRunnerStarted() const;

    /**
     *
     * @brief Get the newThreadId.
     *
     * @return Returns the newThreadId.
     */
    int GetNewThreadId();

    /**
     *
     * @brief Get the application.
     *
     * @return Returns the application.
     */
    std::shared_ptr<OHOSApplication> GetApplication() const;

    /**
     *
     * @brief Get the applicationInfo.
     *
     * @return Returns the applicationInfo.
     */
    std::shared_ptr<ApplicationInfo> GetApplicationInfo() const;

    /**
     *
     * @brief Get the applicationImpl.
     *
     * @return Returns the applicationImpl.
     */
    std::shared_ptr<ApplicationImpl> GetApplicationImpl();

    /**
     *
     * @brief Get the eventHandler of mainthread.
     *
     * @return Returns the eventHandler of mainthread.
     */
    std::shared_ptr<EventHandler> GetMainHandler() const;

    /**
     *
     * @brief Schedule the foreground lifecycle of application.
     *
     */
    bool ScheduleForegroundApplication() override;

    /**
     *
     * @brief Schedule the background lifecycle of application.
     *
     */
    void ScheduleBackgroundApplication() override;

    /**
     *
     * @brief Schedule the terminate lifecycle of application.
     *
     * @param isLastProcess When it is the last application process, pass in true.
     */
    void ScheduleTerminateApplication(bool isLastProcess = false) override;

    /**
     *
     * @brief Shrink the memory which used by application.
     *
     * @param level Indicates the memory trim level, which shows the current memory usage status.
     */
    void ScheduleShrinkMemory(const int level) override;

    /**
     *
     * @brief Notify the current memory.
     *
     * @param level Indicates the memory trim level, which shows the current memory usage status.
     */
    void ScheduleMemoryLevel(const int level, bool isShellCall = false) override;

    /**
     *
     * @brief Get the application's memory allocation info.
     *
     * @param pid, pid input.
     * @param mallocInfo, dynamic storage information output.
     */
    void ScheduleHeapMemory(const int32_t pid, OHOS::AppExecFwk::MallocInfo &mallocInfo) override;

    /**
     *
     * @brief triggerGC and dump the application's jsheap memory info.
     *
     * @param info, pid, tid, needGc, needSnapshot.
     */
    void ScheduleJsHeapMemory(OHOS::AppExecFwk::JsHeapDumpInfo &info) override;

    /**
     *
     * @brief triggerGC and dump the application's cjheap memory info.
     *
     * @param info, pid, needGc, needSnapshot.
     */
    void ScheduleCjHeapMemory(OHOS::AppExecFwk::CjHeapDumpInfo &info) override;

    /**
     *
     * @brief Low the memory which used by application.
     *
     */
    void ScheduleLowMemory() override;

    /**
     *
     * @brief Launch the application.
     *
     * @param data The launchdata of the application witch launced.
     *
     */
    void ScheduleLaunchApplication(const AppLaunchData &data, const Configuration &config) override;

    /**
     *
     * @brief update the application info after new module installed.
     *
     * @param appInfo The latest application info obtained from bms for update abilityRuntimeContext.
     *
     */
    void ScheduleUpdateApplicationInfoInstalled(const ApplicationInfo &appInfo, const std::string &moduleName) override;

    /**
     * Notify application to launch ability stage.
     *
     * @param The resident process data value.
     */
    void ScheduleAbilityStage(const HapModuleInfo &abilityStage) override;

    void ScheduleLaunchAbility(const AbilityInfo &info, const sptr<IRemoteObject> &token,
        const std::shared_ptr<AAFwk::Want> &want, int32_t abilityRecordId) override;

    /**
     *
     * @brief clean the ability by token.
     *
     * @param token The token belong to the ability which want to be cleaned.
     *
     */
    void ScheduleCleanAbility(const sptr<IRemoteObject> &token, bool isCacheProcess = false) override;

    /**
     *
     * @brief send the new profile.
     *
     * @param profile The updated profile.
     *
     */
    void ScheduleProfileChanged(const Profile &profile) override;

    /**
     *
     * @brief send the new config to the application.
     *
     * @param config The updated config.
     *
     */
    void ScheduleConfigurationUpdated(const Configuration &config,
        ConfigUpdateReason reason = ConfigUpdateReason::CONFIG_UPDATE_REASON_DEFAULT) override;

    /**
     *
     * @brief Starts the mainthread.
     *
     */
    static void Start();

    static void StartChild(const std::map<std::string, int32_t> &fds);

    /**
     *
     * @brief Preload extensions in appspawn.
     *
     */
    static void PreloadExtensionPlugin();

    /**
     *
     * @brief Schedule the application process exit safely.
     *
     */
    void ScheduleProcessSecurityExit() override;

    void ScheduleClearPageStack() override;

    void ScheduleAcceptWant(const AAFwk::Want &want, const std::string &moduleName) override;

    void SchedulePrepareTerminate(const std::string &moduleName) override;

    void ScheduleNewProcessRequest(const AAFwk::Want &want, const std::string &moduleName) override;

    /**
     *
     * @brief Check the App main thread state.
     *
     */
    void CheckMainThreadIsAlive();

    int32_t ScheduleNotifyLoadRepairPatch(const std::string &bundleName, const sptr<IQuickFixCallback> &callback,
        const int32_t recordId) override;

    int32_t ScheduleNotifyHotReloadPage(const sptr<IQuickFixCallback> &callback, const int32_t recordId) override;

    int32_t ScheduleNotifyUnLoadRepairPatch(const std::string &bundleName,
        const sptr<IQuickFixCallback> &callback, const int32_t recordId) override;

    int32_t ScheduleNotifyAppFault(const FaultData &faultData) override;
#ifdef CJ_FRONTEND
    CJUncaughtExceptionInfo CreateCjExceptionInfo(const std::string &bundleName, uint32_t versionCode,
        const std::string &hapPath);
#endif
    EtsEnv::ETSUncaughtExceptionInfo CreateEtsExceptionInfo(const std::string &bundleName, uint32_t versionCode,
        const std::string &hapPath, std::string &appRunningId, int32_t pid, std::string &processName);
    /**
     * @brief Notify NativeEngine GC of status change.
     *
     * @param state GC state
     *
     * @return Is the status change completed.
     */
    int32_t ScheduleChangeAppGcState(int32_t state, uint64_t tid = 0) override;

    void AttachAppDebug(bool isDebugFromLocal) override;
    void DetachAppDebug() override;
    bool NotifyDeviceDisConnect();

    void AssertFaultPauseMainThreadDetection();
    void AssertFaultResumeMainThreadDetection();

    /**
     * ScheduleDumpIpcStart, call ScheduleDumpIpcStart(std::string& result) through proxy project,
     * Start querying the application's IPC payload info.
     *
     * @param result, start IPC dump result output.
     *
     * @return Returns 0 on success, error code on failure.
     */
    int32_t ScheduleDumpIpcStart(std::string& result) override;

    /**
     * ScheduleDumpIpcStop, call ScheduleDumpIpcStop(std::string& result) through proxy project,
     * Stop querying the application's IPC payload info.
     *
     * @param result, stop IPC dump result output.
     *
     * @return Returns 0 on success, error code on failure.
     */
    int32_t ScheduleDumpIpcStop(std::string& result) override;

    /**
     * ScheduleDumpIpcStat, call ScheduleDumpIpcStat(std::string& result) through proxy project,
     * Collect the application's IPC payload info.
     *
     * @param result, IPC payload result output.
     *
     * @return Returns 0 on success, error code on failure.
     */
    int32_t ScheduleDumpIpcStat(std::string& result) override;

    void ScheduleCacheProcess() override;
    /**
     * ScheduleDumpFfrt, call ScheduleDumpFfrt(std::string& result) through proxy project,
     * Start querying the application's ffrt usage.
     *
     * @param result, ffrt dump result output.
     *
     * @return Returns 0 on success, error code on failure.
     */
    int32_t ScheduleDumpFfrt(std::string& result) override;

    int32_t ScheduleDumpArkWeb(const std::string &customArgs, std::string &result) override;

    /**
     *
     * @brief Set watchdog background status of application.
     *
     */
    void SetWatchdogBackgroundStatus(bool status) override;

    void OnLoadAbilityFinished(uint64_t callbackId, int32_t pid) override;

private:
    /**
     *
     * @brief Terminate the application but don't notify ams.
     *
     */
    void HandleTerminateApplicationLocal();

    void HandleScheduleAcceptWant(const AAFwk::Want &want, const std::string &moduleName);

    void HandleScheduleNewProcessRequest(const AAFwk::Want &want, const std::string &moduleName);

    void HandleJsHeapMemory(const OHOS::AppExecFwk::JsHeapDumpInfo &info);

    void HandleCjHeapMemory(const OHOS::AppExecFwk::CjHeapDumpInfo &info);

    void HandleSchedulePrepareTerminate(const std::string &moduleName);

    void PreloadModule(const AppExecFwk::HapModuleInfo &entryHapModuleInfo,
        const std::unique_ptr<AbilityRuntime::Runtime>& runtime);

    void ProcessMainAbility(const AbilityInfo &info, const std::unique_ptr<AbilityRuntime::Runtime>& runtime);

    /**
     *
     * @brief Schedule the application process exit safely.
     *
     */
    void HandleProcessSecurityExit();

    /**
     *
     * @brief Clean the ability but don't notify ams.
     *
     * @param token The token which belongs to the ability launched.
     *
     */
    void HandleCleanAbilityLocal(const sptr<IRemoteObject> &token);

    /**
     *
     * @brief Launch the application.
     *
     * @param appLaunchData The launchdata of the application witch launced.
     *
     */
    void HandleLaunchApplication(const AppLaunchData &appLaunchData, const Configuration &config);

    /**
     *
     * @brief Init the uncatchable task.
     *
     * @param uncatchableTaskInfo The info of the uncatchable task.
     * @param isUncatchable Weather task is uncatcheable.
     *
     */
    void InitUncatchableTask(JsEnv::UncatchableTask &uncatchableTask, const UncatchableTaskInfo &uncatchableTaskInfo,
        bool isUncatchable = false);

    /**
    *
    * @brief Handle process exit.
    *
    * @param processExitInfo The info of the process exit info.
    *
    */
    static void ProcessExit(const ProcessExitInfo& info);

    /**
     *
     * @brief update the application info after new module installed.
     *
     * @param appInfo The latest application info obtained from bms for update abilityRuntimeContext.
     *
     */
    void HandleUpdateApplicationInfoInstalled(const ApplicationInfo &appInfo, const std::string &moduleNames);

    /**
     *
     * @brief Launch the application.
     *
     * @param appLaunchData The launchdata of the application witch launced.
     *
     */
    void HandleAbilityStage(const HapModuleInfo &abilityStage);

    /**
     *
     * @brief Launch the ability.
     *
     * @param abilityRecord The abilityRecord which belongs to the ability launched.
     *
     */
    void HandleLaunchAbility(const std::shared_ptr<AbilityLocalRecord> &abilityRecord);

    /**
     *
     * @brief Clean the ability.
     *
     * @param token The token which belongs to the ability launched.
     *
     */
    void HandleCleanAbility(const sptr<IRemoteObject> &token, bool isCacheProcess = false);

    /**
     *
     * @brief Foreground the application.
     *
     */
    void HandleForegroundApplication();

    /**
     *
     * @brief Background the application.
     *
     */
    void HandleBackgroundApplication();

    /**
     *
     * @brief Terminate the application.
     *
     */
    void HandleTerminateApplication(bool isLastProcess = false);

    /**
     *
     * @brief Shrink the memory which used by application.
     *
     * @param level Indicates the memory trim level, which shows the current memory usage status.
     *
     */
    void HandleShrinkMemory(const int level);

    /**
     *
     * @brief Notify the memory.
     *
     * @param level Indicates the memory trim level, which shows the current memory usage status.
     *
     */
    void HandleMemoryLevel(int level);

    /**
     *
     * @brief send the new config to the application.
     *
     * @param config The updated config.
     * @param reason The updated configuration scene flag.
     *
     */
    void HandleConfigurationUpdated(const Configuration &config,
        ConfigUpdateReason reason = ConfigUpdateReason::CONFIG_UPDATE_REASON_DEFAULT);

    /**
     *
     * @brief remove the deathRecipient from appMgr.
     *
     */
    void RemoveAppMgrDeathRecipient();

    /**
     *
     * @brief Attach the mainthread to the AppMgr.
     *
     */
    void Attach();

    /**
     *
     * @brief Set the runner state of mainthread.
     *
     * @param runnerStart whether the runner is started.
     */
    void SetRunnerStarted(bool runnerStart);

    /**
     *
     * @brief Connect the mainthread to the AppMgr.
     *
     */
    bool ConnectToAppMgr();

    /**
     *
     * @brief Check whether the appLaunchData is legal.
     *
     * @param appLaunchData The appLaunchData should be checked.
     *
     * @return if the appLaunchData is legal, return true. else return false.
     */
    bool CheckLaunchApplicationParam(const AppLaunchData &appLaunchData) const;

    /**
     *
     * @brief Check whether the record is legal.
     *
     * @param record The record should be checked.
     *
     * @return if the record is legal, return true. else return false.
     */
    bool CheckAbilityItem(const std::shared_ptr<AbilityLocalRecord> &record) const;

    /**
     *
     * @brief Init the mainthread.
     *
     * @param runner the runner belong to the mainthread.
     *
     */
    void Init(const std::shared_ptr<EventRunner> &runner);

    /**
     *
     * @brief Task in event handler timeout detected.
     *
     * @param runner the runner belong to the mainthread.
     *
     */
    void TaskTimeoutDetected(const std::shared_ptr<EventRunner>& runner);

    /**
     *
     * @brief Check whether the OHOSApplication is ready.
     *
     * @return if the OHOSApplication is ready, return true. else return false.
     *
     */
    bool IsApplicationReady() const;

    /**
     * @brief Load all extension so
     *
     * @param nativeEngine nativeEngine instance
     */
    void LoadAllExtensions(NativeEngine &nativeEngine);

    void LoadAllExtensions();
    /**
     *
     * @brief Ability Delegator Prepare.
     *
     * @param record User Test info.
     *
     */
    bool PrepareAbilityDelegator(const std::shared_ptr<UserTestRecord> &record, bool isStageBased,
        const AppExecFwk::HapModuleInfo &entryHapModuleInfo, uint32_t targetVersion);

    void LoadExtensionBlockList(const std::shared_ptr<AbilityLocalRecord> &abilityRecord);

    /**
     * @brief Update runtime module checker
     *
     * @param runtime runtime the ability runtime
     */
    void UpdateRuntimeModuleChecker(const std::unique_ptr<AbilityRuntime::Runtime> &runtime);

    static void HandleDumpHeapPrepare();
    static void HandleDumpHeap(bool isPrivate);
    static void DestroyHeapProfiler();
    static void ForceFullGC();
    static void HandleSignal(int signal, siginfo_t *siginfo, void *context);

    void NotifyAppFault(const FaultData &faultData);

    void OnOverlayChanged(const EventFwk::CommonEventData &data,
        const std::shared_ptr<Global::Resource::ResourceManager> &resourceManager, const std::string &bundleName,
        const std::string &moduleName, const std::string &loadPath);

    void HandleOnOverlayChanged(const EventFwk::CommonEventData &data,
        const std::shared_ptr<Global::Resource::ResourceManager> &resourceManager, const std::string &bundleName,
        const std::string &moduleName, const std::string &loadPath);

    int GetOverlayModuleInfos(const std::string &bundleName, const std::string &moduleName,
        std::vector<OverlayModuleInfo> &overlayModuleInfos) const;

    std::vector<std::string> GetAddOverlayPaths(const std::vector<OverlayModuleInfo> &overlayModuleInfos);

    std::vector<std::string> GetRemoveOverlayPaths(const std::vector<OverlayModuleInfo> &overlayModuleInfos);

    int32_t ChangeAppGcState(int32_t state, uint64_t tid = 0);

    void HandleCacheProcess();

    bool IsBgWorkingThread(const AbilityInfo &info);

    void DoUpdatePluginInfoInstalled(std::vector<AppExecFwk::PluginBundleInfo> &pluginBundleInfos,
        const std::vector<std::string> &pluginModuleNames);

    void HandleUpdatePluginInfoInstalled(const ApplicationInfo &pluginAppInfo, const std::string &moduleName);

    bool IsPluginNamespaceInherited();

    std::string ParsePluginDefaultNamespaceLdDictionary();

    bool GetBundleAndHspListForUpdateRuntime(BundleInfo &bundleInfo, std::string bundleName, HspList &hspList);

    /**
     * @brief parse app configuration params
     *
     * @param configuration input configuration
     * @config the config of application
     */
    void ParseAppConfigurationParams(const std::string configuration, Configuration &config);

    int32_t OnAttachLocalDebug(bool isDebugFromLocal);

    void HandleConfigByPlugin(Configuration &config, BundleInfo &bundleInfo);

#if defined(NWEB) && defined(NWEB_GRAPHIC)
    void HandleNWebPreload();
#endif

#if defined(NWEB)
    void PreLoadWebLib();
#endif

    class MainHandler : public EventHandler {
    public:
        MainHandler(const std::shared_ptr<EventRunner> &runner, const sptr<MainThread> &thread);
        virtual ~MainHandler() = default;

        /**
         *
         * @brief Process the event.
         *
         * @param event the event want to be processed.
         *
         */
        void ProcessEvent(const OHOS::AppExecFwk::InnerEvent::Pointer &event) override;

    private:
        wptr<MainThread> mainThreadObj_ = nullptr;
    };

    bool isRunnerStarted_ = false;
    int newThreadId_ = -1;
    std::shared_ptr<ApplicationInfo> applicationInfo_ = nullptr;
    std::shared_ptr<ProcessInfo> processInfo_ = nullptr;
    std::shared_ptr<OHOSApplication> application_ = nullptr;
    std::shared_ptr<ApplicationImpl> applicationImpl_ = nullptr;
    static std::shared_ptr<MainHandler> mainHandler_;
    std::shared_ptr<AbilityRecordMgr> abilityRecordMgr_ = nullptr;
    std::shared_ptr<Watchdog> watchdog_ = nullptr;
    std::shared_ptr<AbilityRuntime::ExtensionConfigMgr> extensionConfigMgr_ = nullptr;
    MainThreadState mainThreadState_ = MainThreadState::INIT;
    sptr<IAppMgr> appMgr_ = nullptr;  // appMgrService Handler
    sptr<IRemoteObject::DeathRecipient> deathRecipient_ = nullptr;
    std::string pathSeparator_ = "/";
    std::string abilityLibraryType_ = ".so";
    static std::weak_ptr<OHOSApplication> applicationForDump_;
    bool isDeveloperMode_ = false;
    bool isPluginNamespaceInherited_ = false;
    std::string pluginDefaultNamespaceLdDictionary_ = "";
    RuntimeUpdateParam runtimeUpdateParam_;
#if defined(NWEB) && defined(NWEB_GRAPHIC)
    Rosen::RSSurfaceNode::SharedPtr preloadSurfaceNode_ = nullptr;
    std::shared_ptr<NWeb::NWeb> preloadNWeb_ = nullptr;
#endif

#ifdef ABILITY_LIBRARY_LOADER
    /**
     *
     * @brief Load the ability library.
     *
     * @param libraryPaths the library paths.
     *
     */
    void LoadAbilityLibrary(const std::vector<std::string> &libraryPaths);
    void LoadAceAbilityLibrary();

    void CalcNativeLiabraryEntries(const BundleInfo &bundleInfo, std::string &nativeLibraryPath);
    void LoadNativeLibrary(const BundleInfo &bundleInfo, std::string &nativeLibraryPath);

    void LoadAppDetailAbilityLibrary(std::string &nativeLibraryPath);

    void LoadAppLibrary();

    void ChangeToLocalPath(const std::string &bundleName,
        const std::vector<std::string> &sourceDirs, std::vector<std::string> &localPath);

    void ChangeToLocalPath(const std::string &bundleName,
        const std::string &sourcDir, std::string &localPath);

    bool ScanDir(const std::string &dirPath, std::vector<std::string> &files);

    /**
     *
     * @brief Check the fileType.
     *
     * @param fileName The fileName of the lib.
     * @param extensionName The extensionName of the lib.
     *
     * @return if the FileType is legal, return true. else return false.
     *
     */
    bool CheckFileType(const std::string &fileName, const std::string &extensionName);

    bool InitCreate(std::shared_ptr<ContextDeal> &contextDeal, ApplicationInfo &appInfo, ProcessInfo &processInfo);
    bool CheckForHandleLaunchApplication(const AppLaunchData &appLaunchData);
    bool InitResourceManager(std::shared_ptr<Global::Resource::ResourceManager> &resourceManager,
        const AppExecFwk::HapModuleInfo &entryHapModuleInfo, const std::string &bundleName,
        const Configuration &config, const ApplicationInfo &appInfo);
    void OnStartAbility(const std::string& bundleName,
        std::shared_ptr<Global::Resource::ResourceManager> &resourceManager,
        const AppExecFwk::HapModuleInfo &entryHapModuleInfo, const bool isDebugApp);
    std::vector<std::string> GetOverlayPaths(const std::string &bundleName,
        const std::vector<OverlayModuleInfo> &overlayModuleInfos);
    void SubscribeOverlayChange(const std::string &bundleName, const std::string &loadPath,
        std::shared_ptr<Global::Resource::ResourceManager> &resourceManager,
        const AppExecFwk::HapModuleInfo &entryHapModuleInfo);
    void HandleInitAssertFaultTask(bool isDebugModule, bool isDebugApp);
    void HandleCancelAssertFaultTask();

    bool GetHqfFileAndHapPath(const std::string &bundleName,
        std::vector<std::pair<std::string, std::string>> &fileMap);
    void GetNativeLibPath(const BundleInfo &bundleInfo, const HspList &hspList, AppLibPathMap &appLibPaths);
    void SetAppDebug(uint32_t modeFlag, bool isDebug);
    void GetPluginNativeLibPath(std::vector<AppExecFwk::PluginBundleInfo> &pluginBundleInfos,
        AppLibPathMap &appLibPaths);
    void SetRuntimeLang(ApplicationInfo &appInfo, AbilityRuntime::Runtime::Options &options);
    bool IsEtsAPP(const ApplicationInfo &appInfo);
    void SetJsIdleCallback(const std::weak_ptr<OHOSApplication> &wpApplication,
        const std::unique_ptr<AbilityRuntime::Runtime> &runtime);
    void PreloadAppStartup(const BundleInfo &bundleInfo, const AppLaunchData &appLaunchData) const;
    void RunNativeStartupTask(const BundleInfo &bundleInfo, const AppLaunchData &appLaunchData);
    bool GetTestRunnerTypeAndPath(const std::string bundleName, const std::string moduleName,
        AppExecFwk::ModuleTestRunner &tsTestRunner);
    bool CheckAndUpdateRuntime(const std::shared_ptr<AbilityLocalRecord> &abilityRecord);
    void SleepCleanKill();

    std::vector<std::string> fileEntries_;
    std::vector<std::string> nativeFileEntries_;
    std::vector<void *> handleAbilityLib_;  // the handler of ACE Library.
    std::shared_ptr<IdleTime> idleTime_ = nullptr;
    std::vector<AppExecFwk::OverlayModuleInfo> overlayModuleInfos_;
    std::weak_ptr<AbilityRuntime::AssertFaultTaskThread> assertThread_;
#endif                                      // ABILITY_LIBRARY_LOADER
#ifdef APPLICATION_LIBRARY_LOADER
    void *handleAppLib_ = nullptr;  // the handler of ACE Library.
    constexpr static std::string applicationLibraryPath = "/hos/lib/libapplication_native.z.so";
#endif  // APPLICATION_LIBRARY_LOADER

    DISALLOW_COPY_AND_MOVE(MainThread);
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_MAIN_THREAD_H
