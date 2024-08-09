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

#ifndef OHOS_ABILITY_RUNTIME_APP_LIFECYCLE_DEAL_H
#define OHOS_ABILITY_RUNTIME_APP_LIFECYCLE_DEAL_H

#include "app_scheduler_proxy.h"
#include "app_launch_data.h"
#include "ability_running_record.h"
#include "fault_data.h"
#include "hap_module_info.h"
#include "want.h"
#include "app_malloc_info.h"
#include "app_jsheap_mem_info.h"

namespace OHOS {
namespace AppExecFwk {
class AppLifeCycleDeal {
public:
    AppLifeCycleDeal();
    virtual ~AppLifeCycleDeal();

    /**
     * LaunchApplication, call ScheduleLaunchApplication() through proxy project,
     * Notify application to launch application.
     *
     * @param launchData The app data when launch.
     * @param config The app config when launch.
     * @return
     */
    void LaunchApplication(const AppLaunchData &launchData, const Configuration &config);

    /**
     * update the application info after new module installed.
     *
     * @param appInfo The latest application info obtained from bms for update abilityRuntimeContext.
     *
     * @return
     */
    void UpdateApplicationInfoInstalled(const ApplicationInfo &appInfo);

    /**
     * AddAbilityStageInfo, call ScheduleAbilityStageInfo() through proxy project,
     * Notify application to launch application.
     *
     * @param abilityStage The app data value.
     *
     * @return
     */
    void AddAbilityStage(const HapModuleInfo &abilityStage);

    /**
     * LaunchAbility, call ScheduleLaunchAbility() through proxy project,
     * Notify application to launch ability.
     *
     * @param ability The ability info.
     * @return
     */
    void LaunchAbility(const std::shared_ptr<AbilityRunningRecord> &ability);

    /**
     * ScheduleTerminate, call ScheduleTerminateApplication() through proxy project,
     * Notify application to terminate.
     *
     * @param isLastProcess When it is the last application process, pass in true.
     *
     * @return
     */
    void ScheduleTerminate(bool isLastProcess = false);

    /**
     * ScheduleForegroundRunning, call ScheduleForegroundApplication() through proxy project,
     * Notify application to switch to foreground.
     *
     * @return
     */
    void ScheduleForegroundRunning();

    /**
     * ScheduleBackgroundRunning, call ScheduleBackgroundApplication() through proxy project,
     * Notify application to switch to background.
     *
     * @return
     */
    void ScheduleBackgroundRunning();

    /**
     * ScheduleTrimMemory, call ScheduleShrinkMemory() through proxy project,
     * Notifies the application of the memory seen.
     *
     * @param timeLevel The memory value.
     *
     * @return
     */
    void ScheduleTrimMemory(int32_t timeLevel);

    /**
     * ScheduleMemoryLevel, call ScheduleMemoryLevel() through proxy project,
     * Notifies the application of the current memory.
     *
     * @param The memory level.
     *
     * @return
     */
    void ScheduleMemoryLevel(int32_t Level);

    /**
     * ScheduleHeapMemory, call ScheduleHeapMemory() through proxy project,
     * Get the application's memory allocation info.
     *
     * @param pid, pid input.
     * @param mallocInfo, dynamic storage information output.
     *
     * @return
     */
    void ScheduleHeapMemory(const int32_t pid, OHOS::AppExecFwk::MallocInfo &mallocInfo);

    /**
     * ScheduleJsHeapMemory, call ScheduleJsHeapMemory() through proxy project,
     * triggerGC and dump the application's jsheap memory info.
     *
     * @param info, pid, tid, needGc, needSnapshot
     *
     * @return
     */
    void ScheduleJsHeapMemory(OHOS::AppExecFwk::JsHeapDumpInfo &info);

    /**
     * LowMemoryWarning, call ScheduleLowMemory() through proxy project,
     * Notify application to low memory.
     *
     * @return
     */
    void LowMemoryWarning();

    /**
     * ScheduleCleanAbility, call ScheduleCleanAbility() through proxy project,
     * Notify application to clean ability.
     *
     * @param token, The ability token.
     * @return
     */
    void ScheduleCleanAbility(const sptr<IRemoteObject> &token, bool isCacheProcess = false);

    /**
     * ScheduleProcessSecurityExit, call ScheduleTerminateApplication() through proxy project,
     * Notify application process exit safely.
     *
     * @return
     */
    void ScheduleProcessSecurityExit();

    /**
     * cheduleClearPageStack, call cheduleClearPageStack() through proxy project,
     * Notify application clear recovery page stack.
     *
     * @return
     */
    void ScheduleClearPageStack();

    /**
     * @brief Setting client for application record.
     *
     * @param thread, the application client.
     */
    void SetApplicationClient(const sptr<IAppScheduler> &thread);

    /**
     * @brief Obtains the client of the application record.
     *
     * @return Returns the application client.
     */
    sptr<IAppScheduler> GetApplicationClient() const;

    void ScheduleAcceptWant(const AAFwk::Want &want, const std::string &moduleName);

    void ScheduleNewProcessRequest(const AAFwk::Want &want, const std::string &moduleName);

    /**
     * UpdateConfiguration, ANotify application update system environment changes.
     *
     * @param config, System environment change parameters.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t UpdateConfiguration(const Configuration &config);

    int32_t NotifyLoadRepairPatch(const std::string &bundleName, const sptr<IQuickFixCallback> &callback,
        const int32_t recordId);

    int32_t NotifyHotReloadPage(const sptr<IQuickFixCallback> &callback, const int32_t recordId);

    int32_t NotifyUnLoadRepairPatch(const std::string &bundleName, const sptr<IQuickFixCallback> &callback,
        const int32_t recordId);

    int32_t NotifyAppFault(const FaultData &faultData);

    /**
     * @brief Notify NativeEngine GC of status change.
     *
     * @param state GC state
     *
     * @return Is the status change completed.
     */
    int32_t ChangeAppGcState(int32_t state);

    int32_t AttachAppDebug();
    int32_t DetachAppDebug();

    /**
     * Whether the current application process is the last surviving process.
     *
     * @return Returns true is final application process, others return false.
     */
    bool IsFinalAppProcess();

    int DumpIpcStart(std::string& result);

    int DumpIpcStop(std::string& result);

    int DumpIpcStat(std::string& result);

    /**
     * Notifies the application of process caching.
     *
     *
     * @return
     */
    void ScheduleCacheProcess();

    int DumpFfrt(std::string& result);

private:
    mutable std::mutex schedulerMutex_;
    sptr<IAppScheduler> appThread_ = nullptr;
};
}  // namespace AppExecFwk
}  // namespace OHOS

#endif  // OHOS_ABILITY_RUNTIME_APP_LIFECYCLE_DEAL_H
