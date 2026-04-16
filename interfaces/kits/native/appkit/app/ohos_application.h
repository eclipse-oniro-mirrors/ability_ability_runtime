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

#ifndef OHOS_ABILITY_RUNTIME_OHOS_APPLICATION_H
#define OHOS_ABILITY_RUNTIME_OHOS_APPLICATION_H

#include <functional>
#include <string>
#include <list>
#include <memory>
#include <mutex>

#include "ability_stage.h"
#include "app_context.h"
#include "context.h"
#include "ability_stage_context.h"
#include "application_configuration_manager.h"
#include "app_launch_data.h"
#include "runtime.h"

namespace OHOS {
namespace AbilityRuntime {
class Runtime;
} // namespace AbilityRuntime
namespace AppExecFwk {
class ApplicationImpl;
class Configuration;
class AbilityRecordMgr;
class OHOSApplication : public AppContext {
public:
    OHOSApplication();
    virtual ~OHOSApplication();

    /**
     * @brief dump OHOSApplication info
     *
     * @param extra dump OHOSApplication info
     */
    void DumpApplication();

    /**
     * @brief Set Runtime
     *
     * @param runtime Runtime instance.
     */
    void SetRuntime(std::unique_ptr<AbilityRuntime::Runtime> &&runtime);

    /**
     * @brief Set ApplicationContext
     *
     * @param context ApplicationContext instance.
     */
    void SetApplicationContext(const std::shared_ptr<AbilityRuntime::ApplicationContext> &abilityRuntimeContext);

    /**
     *
     * @brief Set the abilityRecordMgr to the OHOSApplication.
     *
     * @param abilityRecordMgr
     */
    void SetAbilityRecordMgr(const std::shared_ptr<AbilityRecordMgr> &abilityRecordMgr);

    /**
     *
     * @brief Will be Called when the system configuration of the device changes.
     *
     * @param config Indicates the new Configuration object.
     */
    virtual void OnConfigurationUpdated(Configuration config,
        AbilityRuntime::SetLevel level = AbilityRuntime::SetLevel::System,
        ConfigUpdateReason reason = ConfigUpdateReason::CONFIG_UPDATE_REASON_DEFAULT);

    /**
     *
     * @brief Will be Called when the application font of the device changes.
     *
     * @param config Indicates the new Configuration object.
     */
    virtual void OnUpdateConfigurationForAll(Configuration config);

    /**
     *
     * @brief Called when the system has determined to trim the memory, for example,
     * when the ability is running in the background and there is no enough memory for
     * running as many background processes as possible.
     *
     * @param level Indicates the memory trim level, which shows the current memory usage status.
     */
    virtual void OnMemoryLevel(int32_t level);

    /**
     *
     * @brief Will be called the application foregrounds
     *
     */
    virtual void OnForeground();

    /**
     *
     * @brief Will be called the application backgrounds
     *
     */
    virtual void OnBackground();

    /**
     *
     * @brief Will be called the application starts
     *
     */
    virtual void OnStart();

    /**
     *
     * @brief Will be called the application ends
     */
    virtual void OnTerminate();

    virtual void OnHyperSnapUpdate();

    /**
     * @brief add the ability stage when a hap first load
     *
     * @param abilityRecord
     * @return abilityStage context
     */
    std::shared_ptr<AbilityRuntime::Context> AddAbilityStage(
        const std::shared_ptr<AbilityLocalRecord> &abilityRecord,
        const std::function<void(const std::shared_ptr<AbilityRuntime::Context> &)> &callback, bool &isAsyncCallback);

    /**
     *
     * @brief update the application info after new module installed.
     *
     * @param appInfo The latest application info obtained from bms for update abilityRuntimeContext.
     *
     */
    void UpdateApplicationInfoInstalled(const AppExecFwk::ApplicationInfo &appInfo);

    /**
     * @brief add the ability stage when a hap first load
     *
     * @param hapModuleInfo
     * @return Returns true on success, false on failure
     */
    bool AddAbilityStage(
        const AppExecFwk::HapModuleInfo &hapModuleInfo,
        const std::function<void()> &callback, bool &isAsyncCallback);

    /**
     * @brief remove the ability stage when all of the abilities in the hap have been removed
     *
     * @param abilityInfo
     */
    void CleanAbilityStage(const sptr<IRemoteObject> &token, const std::shared_ptr<AbilityInfo> &abilityInfo,
        bool isCacheProcess);

    /**
     * @brief return the application context
     *
     * @param context
     */
    std::shared_ptr<AbilityRuntime::Context> GetAppContext() const;

    /**
     * @brief return the application runtime
     *
     * @param runtime
     */
    const std::unique_ptr<AbilityRuntime::Runtime> &GetRuntime() const
    {
        std::lock_guard<std::mutex> lock(runtimeMutex_);
        return runtime_;
    }

    /**
     * @brief return the specified runtime
     *
     * @param runtime
     */
    const std::unique_ptr<AbilityRuntime::Runtime> &GetSpecifiedRuntime(const std::string &arkTSMode) const;

    /*
     *
     * @brief Will be called the application ends
     *
     */
    virtual void SetConfiguration(const Configuration &config);

    void ScheduleAcceptWant(const AAFwk::Want &want, const std::string &moduleName,
        std::function<void(std::string)> callback, bool &isAsync);

    void SchedulePrepareTerminate(const std::string &moduleName,
        std::function<void(AppExecFwk::OnPrepareTerminationResult)> callback, bool &isAsync);

    void ScheduleNewProcessRequest(const AAFwk::Want &want, const std::string &moduleName,
        std::function<void(std::string)> callback, bool &isAsync);

    virtual std::shared_ptr<Configuration> GetConfiguration() const;

    void GetExtensionNameByType(int32_t type, std::string &name)
    {
        std::map<int32_t, std::string>::iterator it = extensionTypeMap_.find(type);
        if (it == extensionTypeMap_.end()) {
            return;
        }
        name = it->second;
    }

    /**
     * @brief Set extension types.
     *
     * @param map The extension types.
     */
    void SetExtensionTypeMap(std::map<int32_t, std::string> map);

    bool NotifyLoadRepairPatch(const std::string &hqfFile, const std::string &hapPath);

    bool NotifyHotReloadPage();

    bool NotifyUnLoadRepairPatch(const std::string &hqfFile);

    void CleanAppTempData(bool isLastProcess = false);

    void CleanUselessTempData();

    void SetAppEnv(const std::vector<AppEnvironment>& appEnvironments);

    void AutoStartupDone(const std::shared_ptr<AbilityLocalRecord> &abilityRecord,
        const std::shared_ptr<AbilityRuntime::AbilityStage> &abilityStage, const std::string &moduleName);

    void AutoStartupDone(const std::shared_ptr<AbilityRuntime::AbilityStage> &abilityStage,
        const AppExecFwk::HapModuleInfo &hapModuleInfo);

    void CleanEmptyAbilityStage();

    bool UpdateETSRuntime(AbilityRuntime::Runtime::Options &option);

    void InitJSLeakWatcher(const std::string &bundleName);

#ifdef SUPPORT_GRAPHICS
    bool GetDisplayConfig(uint64_t displayId, float &density, std::string &directionStr);
#endif

    void PreloadAppStartup(const BundleInfo &bundleInfo, const std::string &preloadModuleName,
        std::shared_ptr<AppExecFwk::StartupTaskData> startupTaskData);

    void SetEntryLoadPath(const std::string &path);
    const std::string &GetEntryLoadPath() const
    {
        return entryLoadPath_;
    }

    void SetDeduplicateHar(bool deduplicate);
    bool GetDeduplicateHar() const
    {
        return deduplicate_;
    }

private:
    void UpdateAppContextResMgr(const Configuration &config);
    bool IsUpdateColorNeeded(Configuration &config, AbilityRuntime::SetLevel level);
    bool isUpdateFontSize(Configuration &config, AbilityRuntime::SetLevel level);
    bool IsUpdateLanguageNeeded(Configuration &config, AbilityRuntime::SetLevel level);
    bool IsUpdateLocaleNeeded(const Configuration& updatedConfig, Configuration &config);
    const std::function<void()> CreateAutoStartupCallback(
        const std::shared_ptr<AbilityRuntime::AbilityStage> abilityStage,
        const std::shared_ptr<AbilityLocalRecord> abilityRecord,
        const std::function<void(const std::shared_ptr<AbilityRuntime::Context>&)>& callback);
    const std::function<void()> CreateAutoStartupCallback(
        const std::shared_ptr<AbilityRuntime::AbilityStage> &abilityStage,
        const AppExecFwk::HapModuleInfo &hapModuleInfo,
        const std::function<void()>& callback);
    bool IsMainProcess(const std::string &bundleName, const std::string &process);
    void PreloadHybridModule(const HapModuleInfo &hapModuleInfo) const;
    void LoadDeduplicatedHarResource(
        const std::shared_ptr<AbilityRuntime::AbilityStageContext> &stageContext);

    const std::function<void()> CreateFirstStartupCallbackForRecord(
        std::shared_ptr<AbilityRuntime::AbilityStage> abilityStage,
        std::shared_ptr<AbilityLocalRecord> abilityRecord,
        const AppExecFwk::HapModuleInfo &hapModuleInfo,
        const std::function<void(std::shared_ptr<AbilityRuntime::Context>)> &callback);

    const std::function<void()> CreateSecondStartupCallbackForRecord(
        std::shared_ptr<AbilityRuntime::AbilityStage> abilityStage,
        std::shared_ptr<AbilityLocalRecord> abilityRecord,
        const std::function<void(std::shared_ptr<AbilityRuntime::Context>)> &callback);

    const std::function<void()> CreateFirstStartupCallbackForHap(
        std::shared_ptr<AbilityRuntime::AbilityStage> abilityStage,
        const AppExecFwk::HapModuleInfo &hapModuleInfo,
        const std::function<void()> &callback);

    const std::function<void()> CreateSecondStartupCallbackForHap(
        std::shared_ptr<AbilityRuntime::AbilityStage> abilityStage,
        const AppExecFwk::HapModuleInfo &hapModuleInfo,
        const std::function<void()> &callback);

    void AddAbility(std::shared_ptr<AbilityRuntime::AbilityStage> abilityStage,
        const sptr<IRemoteObject> &token,
        const std::shared_ptr<AppExecFwk::AbilityLocalRecord> &abilityRecord);

#ifdef SUPPORT_SCREEN
    void RegisterGetAllUIAbilitiesCallback(
        const std::shared_ptr<AbilityRuntime::ApplicationContext> &context,
        const std::weak_ptr<OHOSApplication> &appWeakPtr);
    void GetAllUIAbilities(std::vector<std::shared_ptr<AbilityRuntime::UIAbility>> &uiAbility);
#endif

private:
    std::shared_ptr<AbilityRecordMgr> abilityRecordMgr_ = nullptr;
    std::shared_ptr<AbilityRuntime::ApplicationContext> abilityRuntimeContext_ = nullptr;
    std::unordered_map<std::string, std::shared_ptr<AbilityRuntime::AbilityStage>> abilityStages_;
    std::unique_ptr<AbilityRuntime::Runtime> runtime_ = nullptr;
    mutable std::mutex runtimeMutex_;
    std::shared_ptr<Configuration> configuration_ = nullptr;
    std::map<int32_t, std::string> extensionTypeMap_;
    std::string entryLoadPath_ = "";
    bool deduplicate_ = false;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_OHOS_APPLICATION_H
