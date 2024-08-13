/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_APPLICATION_CONTEXT_H
#define OHOS_ABILITY_RUNTIME_APPLICATION_CONTEXT_H

#include <vector>
#include <shared_mutex>

#include "ability_lifecycle_callback.h"
#include "application_state_change_callback.h"
#include "context.h"
#include "context_impl.h"
#include "environment_callback.h"
namespace OHOS {
namespace AAFwk {
class Want;
struct ExitReason;
}
namespace AbilityRuntime {
using AppConfigUpdateCallback = std::function<void(const AppExecFwk::Configuration &config)>;
using AppProcessExitCallback = std::function<void(const AAFwk::ExitReason &exitReason)>;
class ApplicationContext : public Context {
public:
    ApplicationContext() = default;
    ~ApplicationContext() = default;
    void RegisterAbilityLifecycleCallback(const std::shared_ptr<AbilityLifecycleCallback> &abilityLifecycleCallback);
    void UnregisterAbilityLifecycleCallback(const std::shared_ptr<AbilityLifecycleCallback> &abilityLifecycleCallback);
    bool IsAbilityLifecycleCallbackEmpty();
    void RegisterEnvironmentCallback(const std::shared_ptr<EnvironmentCallback> &environmentCallback);
    void UnregisterEnvironmentCallback(const std::shared_ptr<EnvironmentCallback> &environmentCallback);
    void RegisterApplicationStateChangeCallback(
        const std::weak_ptr<ApplicationStateChangeCallback> &applicationStateChangeCallback);
    void DispatchOnAbilityCreate(const std::shared_ptr<NativeReference> &ability);
    void DispatchOnWindowStageCreate(const std::shared_ptr<NativeReference> &ability,
        const std::shared_ptr<NativeReference> &windowStage);
    void DispatchOnWindowStageDestroy(const std::shared_ptr<NativeReference> &ability,
        const std::shared_ptr<NativeReference> &windowStage);
    void DispatchWindowStageFocus(const std::shared_ptr<NativeReference> &ability,
        const std::shared_ptr<NativeReference> &windowStage);
    void DispatchWindowStageUnfocus(const std::shared_ptr<NativeReference> &ability,
        const std::shared_ptr<NativeReference> &windowStage);
    void DispatchOnAbilityDestroy(const std::shared_ptr<NativeReference> &ability);
    void DispatchOnAbilityForeground(const std::shared_ptr<NativeReference> &ability);
    void DispatchOnAbilityBackground(const std::shared_ptr<NativeReference> &ability);
    void DispatchOnAbilityContinue(const std::shared_ptr<NativeReference> &ability);
    void DispatchOnAbilityWillContinue(const std::shared_ptr<NativeReference> &ability);
    void DispatchOnWindowStageWillRestore(const std::shared_ptr<NativeReference> &ability,
        const std::shared_ptr<NativeReference> &windowStage);
    void DispatchOnWindowStageRestore(const std::shared_ptr<NativeReference> &ability,
        const std::shared_ptr<NativeReference> &windowStage);
    void DispatchOnAbilityWillSaveState(const std::shared_ptr<NativeReference> &ability);
    void DispatchOnAbilitySaveState(const std::shared_ptr<NativeReference> &ability);
    void DispatchConfigurationUpdated(const AppExecFwk::Configuration &config);
    void DispatchMemoryLevel(const int level);
    void NotifyApplicationForeground();
    void NotifyApplicationBackground();
    void DispatchOnWillNewWant(const std::shared_ptr<NativeReference> &ability);
    void DispatchOnNewWant(const std::shared_ptr<NativeReference> &ability);
    void DispatchOnAbilityWillCreate(const std::shared_ptr<NativeReference> &ability);
    void DispatchOnWindowStageWillCreate(const std::shared_ptr<NativeReference> &ability,
        const std::shared_ptr<NativeReference> &windowStage);
    void DispatchOnWindowStageWillDestroy(const std::shared_ptr<NativeReference> &ability,
        const std::shared_ptr<NativeReference> &windowStage);
    void DispatchOnAbilityWillDestroy(const std::shared_ptr<NativeReference> &ability);
    void DispatchOnAbilityWillForeground(const std::shared_ptr<NativeReference> &ability);
    void DispatchOnAbilityWillBackground(const std::shared_ptr<NativeReference> &ability);

    std::string GetBundleName() const override;
    std::shared_ptr<Context> CreateBundleContext(const std::string &bundleName) override;
    std::shared_ptr<Context> CreateModuleContext(const std::string &moduleName) override;
    std::shared_ptr<Context> CreateModuleContext(const std::string &bundleName, const std::string &moduleName) override;
    std::shared_ptr<Global::Resource::ResourceManager> CreateModuleResourceManager(
        const std::string &bundleName, const std::string &moduleName) override;
    int32_t CreateSystemHspModuleResourceManager(const std::string &bundleName,
        const std::string &moduleName, std::shared_ptr<Global::Resource::ResourceManager> &resourceManager) override;
    std::shared_ptr<AppExecFwk::ApplicationInfo> GetApplicationInfo() const override;
    void SetApplicationInfo(const std::shared_ptr<AppExecFwk::ApplicationInfo> &info);
    std::shared_ptr<Global::Resource::ResourceManager> GetResourceManager() const override;
    std::string GetBundleCodePath() const override;
    std::string GetBundleCodeDir() override;
    std::string GetCacheDir() override;
    std::string GetTempDir() override;
    std::string GetResourceDir() override;
    void GetAllTempDir(std::vector<std::string> &tempPaths);
    std::string GetFilesDir() override;
    bool IsUpdatingConfigurations() override;
    bool PrintDrawnCompleted() override;
    std::string GetDatabaseDir() override;
    std::string GetPreferencesDir() override;
    int32_t GetSystemDatabaseDir(const std::string &groupId, bool checkExist, std::string &databaseDir) override;
    int32_t GetSystemPreferencesDir(const std::string &groupId, bool checkExist, std::string &preferencesDir) override;
    std::string GetGroupDir(std::string groupId) override;
    std::string GetDistributedFilesDir() override;
    std::string GetCloudFileDir() override;
    sptr<IRemoteObject> GetToken() override;
    void SetToken(const sptr<IRemoteObject> &token) override;
    void SwitchArea(int mode) override;
    void SetColorMode(int32_t colorMode);
    void SetLanguage(const std::string &language);
    void SetFont(const std::string &font);
    void SetMcc(const std::string &mcc);
    void SetMnc(const std::string &mnc);
    void ClearUpApplicationData();
    int GetArea() override;
    std::shared_ptr<AppExecFwk::Configuration> GetConfiguration() const override;
    void SetConfiguration(const std::shared_ptr<AppExecFwk::Configuration> &config);
    void AppHasDarkRes(bool &darkRes);
    std::string GetBaseDir() const override;
    Global::Resource::DeviceType GetDeviceType() const override;
    void KillProcessBySelf(const bool clearpagestack = false);
    int32_t GetProcessRunningInformation(AppExecFwk::RunningProcessInfo &info);
    int32_t RestartApp(const AAFwk::Want& want);

    void AttachContextImpl(const std::shared_ptr<ContextImpl> &contextImpl);

    static std::shared_ptr<ApplicationContext> GetInstance();

    // unused
    std::shared_ptr<AppExecFwk::HapModuleInfo> GetHapModuleInfo() const override;

    bool GetApplicationInfoUpdateFlag() const;
    void SetApplicationInfoUpdateFlag(bool flag);
    void RegisterAppConfigUpdateObserver(AppConfigUpdateCallback appConfigChangeCallback);
    void RegisterAppFontObserver(AppConfigUpdateCallback appFontCallback);
    void RegisterProcessSecurityExit(AppProcessExitCallback appProcessExitCallback);

    std::string GetAppRunningUniqueId() const;
    void SetAppRunningUniqueId(const std::string &appRunningUniqueId);
    int32_t SetSupportedProcessCacheSelf(bool isSupport);
    int32_t GetCurrentAppCloneIndex();
    void SetCurrentAppCloneIndex(int32_t appIndex);
    int32_t GetCurrentAppMode();
    void SetCurrentAppMode(int32_t appIndex);
    void ProcessSecurityExit(const AAFwk::ExitReason &exitReason);

    using SelfType = ApplicationContext;
    static const size_t CONTEXT_TYPE_ID;

protected:
    bool IsContext(size_t contextTypeId) override
    {
        return contextTypeId == CONTEXT_TYPE_ID || Context::IsContext(contextTypeId);
    }

private:
    std::shared_ptr<ContextImpl> contextImpl_;
    static std::vector<std::shared_ptr<AbilityLifecycleCallback>> callbacks_;
    static std::vector<std::shared_ptr<EnvironmentCallback>> envCallbacks_;
    static std::vector<std::weak_ptr<ApplicationStateChangeCallback>> applicationStateCallback_;
    std::recursive_mutex callbackLock_;
    std::recursive_mutex envCallbacksLock_;
    std::recursive_mutex applicationStateCallbackLock_;
    bool applicationInfoUpdateFlag_ = false;
    AppConfigUpdateCallback appConfigChangeCallback_ = nullptr;
    AppConfigUpdateCallback appFontCallback_ = nullptr;
    AppProcessExitCallback appProcessExitCallback_ = nullptr;
    std::string appRunningUniqueId_;
    int32_t appIndex_ = 0;
    int32_t appMode_ = 0;
};
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_APPLICATION_CONTEXT_H
