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

#ifndef OHOS_ABILITY_RUNTIME_APP_SCHEDULER_PROXY_H
#define OHOS_ABILITY_RUNTIME_APP_SCHEDULER_PROXY_H

#include "iremote_proxy.h"
#include "app_scheduler_interface.h"
#include "app_malloc_info.h"
#include "app_jsheap_mem_info.h"
#include "app_cjheap_mem_info.h"

namespace OHOS {
namespace AppExecFwk {
class AppSchedulerProxy : public IRemoteProxy<IAppScheduler> {
public:
    explicit AppSchedulerProxy(const sptr<IRemoteObject> &impl);
    virtual ~AppSchedulerProxy() = default;

    /**
     * ScheduleForegroundApplication, call ScheduleForegroundApplication() through proxy project,
     * Notify application to switch to foreground.
     *
     * @return
     */
    virtual bool ScheduleForegroundApplication() override;

    /**
     * ScheduleBackgroundApplication, call ScheduleBackgroundApplication() through proxy project,
     * Notify application to switch to background.
     *
     * @return
     */
    virtual void ScheduleBackgroundApplication() override;

    /**
     * ScheduleTerminateApplication, call ScheduleTerminateApplication() through proxy project,
     * Notify application to terminate.
     *
     * @param isLastProcess When it is the last application process, pass in true.
     */
    virtual void ScheduleTerminateApplication(bool isLastProcess = false) override;

    /**
     * ScheduleShrinkMemory, call ScheduleShrinkMemory() through proxy project,
     * Notifies the application of the memory seen.
     *
     * @param The memory value.
     *
     * @return
     */
    virtual void ScheduleShrinkMemory(const int32_t level) override;

    /**
     * ScheduleLowMemory, call ScheduleLowMemory() through proxy project,
     * Notify application to low memory.
     *
     * @return
     */
    virtual void ScheduleLowMemory() override;

    /**
     * ScheduleMemoryLevel, call ScheduleMemoryLevel() through proxy project,
     * Notify applications background the current memory level.
     *
     * @return
     */
    virtual void ScheduleMemoryLevel(int32_t level, bool isShellCall = false) override;

    /**
     * ScheduleHeapMemory, call ScheduleHeapMemory() through proxy project,
     * Get the application's memory allocation info.
     *
     * @param pid, pid input.
     * @param mallocInfo, dynamic storage information output.
     *
     * @return
     */
    virtual void ScheduleHeapMemory(const int32_t pid, OHOS::AppExecFwk::MallocInfo &mallocInfo) override;

    /**
     * ScheduleJsHeapMemory, call ScheduleJsHeapMemory() through proxy project,
     * triggerGC and dump the application's jsheap memory info.
     *
     * @param info, pid, tid, needGc, needSnapshot
     *
     * @return
     */
    virtual void ScheduleJsHeapMemory(OHOS::AppExecFwk::JsHeapDumpInfo &info) override;

    /**
     * ScheduleCjHeapMemory, call ScheduleCjHeapMemory() through proxy project,
     * triggerGC and dump the application's cjheap memory info.
     *
     * @param info, pid, needGc, needSnapshot
     *
     * @return
     */
    virtual void ScheduleCjHeapMemory(OHOS::AppExecFwk::CjHeapDumpInfo &info) override;

    /**
     * ScheduleLaunchApplication, call ScheduleLaunchApplication() through proxy project,
     * Notify application to launch application.
     *
     * @param The app data value.
     *
     * @return
     */
    virtual void ScheduleLaunchApplication(const AppLaunchData &launchData, const Configuration &config) override;

    /**
     * ScheduleUpdateApplicationInfoInstalled, call ScheduleUpdateApplicationInfoInstalled() through proxy object,
     * update the application info after new module installed.
     *
     * @param appInfo The latest application info obtained from bms for update abilityRuntimeContext.
     *
     * @return
     */
    virtual void ScheduleUpdateApplicationInfoInstalled(const ApplicationInfo &, const std::string &) override;

    /**
     * Notify application to launch ability stage.
     *
     * @param The resident process data value.
     */
    virtual void ScheduleAbilityStage(const HapModuleInfo &abilityStage) override;

    /**
     * ScheduleLaunchAbility, call ScheduleLaunchAbility() through proxy project,
     * Notify application to launch ability.
     *
     * @param The ability info.
     * @param The ability token.
     * @param The ability want.
     * @return
     */
    virtual void ScheduleLaunchAbility(const AbilityInfo &, const sptr<IRemoteObject> &,
        const std::shared_ptr<AAFwk::Want> &want, int32_t abilityRecordId) override;

    /**
     * ScheduleCleanAbility, call ScheduleCleanAbility() through proxy project,
     * Notify application to clean ability.
     *
     * @param The ability token.
     * @return
     */
    virtual void ScheduleCleanAbility(const sptr<IRemoteObject> &token, bool isCacheProcess = false) override;

    /**
     * ScheduleProfileChanged, call ScheduleProfileChanged() through proxy project,
     * Notify application to profile update.
     *
     * @param The profile data.
     * @return
     */
    virtual void ScheduleProfileChanged(const Profile &profile) override;

    /**
     * ScheduleConfigurationUpdated, call ScheduleConfigurationUpdated() through proxy project,
     * Notify application to configuration update.
     *
     * @param config, The configuration data.
     * @param reason, The configuration update scene flag.
     * @return
     */
    void ScheduleConfigurationUpdated(const Configuration &config,
        ConfigUpdateReason reason = ConfigUpdateReason::CONFIG_UPDATE_REASON_DEFAULT) override;

    /**
     * ScheduleProcessSecurityExit, call ScheduleProcessSecurityExit() through proxy project,
     * Notify application process exit safely.
     *
     * @return
     */
    virtual void ScheduleProcessSecurityExit() override;

    /**
     * scheduleClearPageStack, call scheduleClearPageStack() through proxy project,
     * Notify application clear recovery page stack.
     *
     */
    virtual void ScheduleClearPageStack() override;

    /**
     * @brief Schedule the given module the onAcceptWant lifecycle call.
     *
     * @param want the param passed to onAcceptWant lifecycle.
     * @param want the moduleName of which being scheduled.
     */
    virtual void ScheduleAcceptWant(const AAFwk::Want &want, const std::string &moduleName) override;

    /**
     * @brief Schedule prepare terminate application
     *
     * @param moduleName module name
     */
    virtual void SchedulePrepareTerminate(const std::string &moduleName) override;

    virtual void ScheduleNewProcessRequest(const AAFwk::Want &want, const std::string &moduleName) override;

    /**
     * @brief Notify application load patch.
     *
     * @param bundleName Bundle name
     * @param callback called when LoadPatch finished.
     * @param recordId callback data
     * @return Returns 0 on success, error code on failure.
     */
    int32_t ScheduleNotifyLoadRepairPatch(const std::string &bundleName,
        const sptr<IQuickFixCallback> &callback, const int32_t recordId) override;

    /**
     * @brief Notify application reload page.
     *
     * @param callback called when HotReload finished.
     * @param recordId callback data
     * @return Returns 0 on success, error code on failure.
     */
    int32_t ScheduleNotifyHotReloadPage(const sptr<IQuickFixCallback> &callback, const int32_t recordId) override;

    /**
     * @brief Notify application unload patch.
     *
     * @param bundleName Bundle name
     * @param callback called when UnloadPatch finished.
     * @param recordId callback data
     * @return Returns 0 on success, error code on failure.
     */
    int32_t ScheduleNotifyUnLoadRepairPatch(const std::string &bundleName,
        const sptr<IQuickFixCallback> &callback, const int32_t recordId) override;

    /**
     * @brief Schedule Notify App Fault Data.
     *
     * @param faultData fault data
     * @return Returns ERR_OK on success, error code on failure.
     */
    int32_t ScheduleNotifyAppFault(const FaultData &faultData) override;

    /**
     * @brief Notify NativeEngine GC of status change.
     *
     * @param state GC state
     * @param pid pid
     *
     * @return Is the status change completed.
     */
    virtual int32_t ScheduleChangeAppGcState(int32_t state, uint64_t tid = 0) override;

    /**
     * @brief Attach app debug.
     */
    void AttachAppDebug(bool isDebugFromLocal) override;

     /**
     * @brief Detach app debug.
     */
    void DetachAppDebug() override;

    /**
     * ScheduleDumpIpcStart, call ScheduleDumpIpcStart(std::string& result) through proxy project,
     * Start querying the application's IPC payload info.
     *
     * @param result, start IPC dump result output.
     *
     * @return Returns 0 on success, error code on failure.
     */
    virtual int32_t ScheduleDumpIpcStart(std::string& result) override;

    /**
     * ScheduleDumpIpcStop, call ScheduleDumpIpcStop(std::string& result) through proxy project,
     * Stop querying the application's IPC payload info.
     *
     * @param result, stop IPC dump result output.
     *
     * @return Returns 0 on success, error code on failure.
     */
    virtual int32_t ScheduleDumpIpcStop(std::string& result) override;

    /**
     * ScheduleDumpIpcStat, call ScheduleDumpIpcStat(std::string& result) through proxy project,
     * Collect the application's IPC payload info.
     *
     * @param result, IPC payload result output.
     *
     * @return Returns 0 on success, error code on failure.
     */
    virtual int32_t ScheduleDumpIpcStat(std::string& result) override;

    /**
     * @brief Notify application to prepare for process caching.
     */
    virtual void ScheduleCacheProcess() override;

    /**
     * ScheduleDumpFfrt, call ScheduleDumpFfrt(std::string& result) through proxy project,
     * Start querying the application's ffrt usage.
     *
     * @param result, ffrt dump result output.
     *
     * @return Returns 0 on success, error code on failure.
     */
    virtual int32_t ScheduleDumpFfrt(std::string& result) override;

    virtual int32_t ScheduleDumpArkWeb(const std::string &customArgs, std::string &result) override;

    /**
     * SetWatchdogBackgroundStatus, call SetWatchdogBackgroundStatus(bool status) through proxy project,
     * Notify application to set watchdog background status.
     *
     * @return
     */
    virtual void SetWatchdogBackgroundStatus(bool status) override;

    /**
     * Call the callback to send the pid of the started ability.
     * @param callbackId The id of the callback to send the pid.
     * @param pid The resultant process id of the started ability.
     */
    virtual void OnLoadAbilityFinished(uint64_t callbackId, int32_t pid) override;

private:
    bool WriteInterfaceToken(MessageParcel &data);
    void ScheduleMemoryCommon(const int32_t level, const uint32_t operation, bool isShellCall = false);
    int32_t SendTransactCmd(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option);
    static inline BrokerDelegator<AppSchedulerProxy> delegator_;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_APP_SCHEDULER_PROXY_H
