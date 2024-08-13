/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include "application_context.h"

#include <algorithm>

#include "ability_manager_errors.h"
#include "configuration_convertor.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "running_process_info.h"
#include "exit_reason.h"

namespace OHOS {
namespace AbilityRuntime {
const size_t ApplicationContext::CONTEXT_TYPE_ID(std::hash<const char*> {} ("ApplicationContext"));
std::vector<std::shared_ptr<AbilityLifecycleCallback>> ApplicationContext::callbacks_;
std::vector<std::shared_ptr<EnvironmentCallback>> ApplicationContext::envCallbacks_;
std::vector<std::weak_ptr<ApplicationStateChangeCallback>> ApplicationContext::applicationStateCallback_;

std::shared_ptr<ApplicationContext> ApplicationContext::GetInstance()
{
    if (applicationContext_ == nullptr) {
        std::lock_guard<std::mutex> lock_l(Context::contextMutex_);
        if (applicationContext_ == nullptr) {
            applicationContext_ = std::make_shared<ApplicationContext>();
        }
    }
    return applicationContext_;
}

void ApplicationContext::AttachContextImpl(const std::shared_ptr<ContextImpl> &contextImpl)
{
    contextImpl_ = contextImpl;
}

void ApplicationContext::RegisterAbilityLifecycleCallback(
    const std::shared_ptr<AbilityLifecycleCallback> &abilityLifecycleCallback)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    if (abilityLifecycleCallback == nullptr) {
        return;
    }
    std::lock_guard<std::recursive_mutex> lock(callbackLock_);
    callbacks_.push_back(abilityLifecycleCallback);
}

void ApplicationContext::UnregisterAbilityLifecycleCallback(
    const std::shared_ptr<AbilityLifecycleCallback> &abilityLifecycleCallback)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    std::lock_guard<std::recursive_mutex> lock(callbackLock_);
    auto it = std::find(callbacks_.begin(), callbacks_.end(), abilityLifecycleCallback);
    if (it != callbacks_.end()) {
        callbacks_.erase(it);
    }
}

bool ApplicationContext::IsAbilityLifecycleCallbackEmpty()
{
    std::lock_guard<std::recursive_mutex> lock(callbackLock_);
    return callbacks_.empty();
}

void ApplicationContext::RegisterEnvironmentCallback(
    const std::shared_ptr<EnvironmentCallback> &environmentCallback)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    if (environmentCallback == nullptr) {
        return;
    }
    std::lock_guard<std::recursive_mutex> lock(envCallbacksLock_);
    envCallbacks_.push_back(environmentCallback);
}

void ApplicationContext::UnregisterEnvironmentCallback(
    const std::shared_ptr<EnvironmentCallback> &environmentCallback)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    std::lock_guard<std::recursive_mutex> lock(envCallbacksLock_);
    auto it = std::find(envCallbacks_.begin(), envCallbacks_.end(), environmentCallback);
    if (it != envCallbacks_.end()) {
        envCallbacks_.erase(it);
    }
}

void ApplicationContext::RegisterApplicationStateChangeCallback(
    const std::weak_ptr<ApplicationStateChangeCallback> &applicationStateChangeCallback)
{
    std::lock_guard<std::recursive_mutex> lock(applicationStateCallbackLock_);
    applicationStateCallback_.push_back(applicationStateChangeCallback);
}

void ApplicationContext::DispatchOnAbilityCreate(const std::shared_ptr<NativeReference> &ability)
{
    if (!ability) {
        TAG_LOGE(AAFwkTag::APPKIT, "ability is nullptr");
        return;
    }
    std::lock_guard<std::recursive_mutex> lock(callbackLock_);
    for (auto callback : callbacks_) {
        if (callback != nullptr) {
            callback->OnAbilityCreate(ability);
        }
    }
}

void ApplicationContext::DispatchOnWindowStageCreate(const std::shared_ptr<NativeReference> &ability,
    const std::shared_ptr<NativeReference> &windowStage)
{
    if (!ability || !windowStage) {
        TAG_LOGE(AAFwkTag::APPKIT, "ability or windowStage is nullptr");
        return;
    }
    std::lock_guard<std::recursive_mutex> lock(callbackLock_);
    for (auto callback : callbacks_) {
        if (callback != nullptr) {
            callback->OnWindowStageCreate(ability, windowStage);
        }
    }
}

void ApplicationContext::DispatchOnWindowStageDestroy(const std::shared_ptr<NativeReference> &ability,
    const std::shared_ptr<NativeReference> &windowStage)
{
    if (!ability || !windowStage) {
        TAG_LOGE(AAFwkTag::APPKIT, "ability or windowStage is nullptr");
        return;
    }
    std::lock_guard<std::recursive_mutex> lock(callbackLock_);
    for (auto callback : callbacks_) {
        if (callback != nullptr) {
            callback->OnWindowStageDestroy(ability, windowStage);
        }
    }
}

void ApplicationContext::DispatchWindowStageFocus(const std::shared_ptr<NativeReference> &ability,
    const std::shared_ptr<NativeReference> &windowStage)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    if (!ability || !windowStage) {
        TAG_LOGE(AAFwkTag::APPKIT, "ability or windowStage is null");
        return;
    }
    std::lock_guard<std::recursive_mutex> lock(callbackLock_);
    for (auto callback : callbacks_) {
        if (callback != nullptr) {
            callback->OnWindowStageActive(ability, windowStage);
        }
    }
}

void ApplicationContext::DispatchWindowStageUnfocus(const std::shared_ptr<NativeReference> &ability,
    const std::shared_ptr<NativeReference> &windowStage)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    if (!ability || !windowStage) {
        TAG_LOGE(AAFwkTag::APPKIT, "ability or windowStage is nullptr");
        return;
    }
    std::lock_guard<std::recursive_mutex> lock(callbackLock_);
    for (auto callback : callbacks_) {
        if (callback != nullptr) {
            callback->OnWindowStageInactive(ability, windowStage);
        }
    }
}

void ApplicationContext::DispatchOnAbilityDestroy(const std::shared_ptr<NativeReference> &ability)
{
    if (!ability) {
        TAG_LOGE(AAFwkTag::APPKIT, "ability is nullptr");
        return;
    }
    std::lock_guard<std::recursive_mutex> lock(callbackLock_);
    for (auto callback : callbacks_) {
        if (callback != nullptr) {
            callback->OnAbilityDestroy(ability);
        }
    }
}

void ApplicationContext::DispatchOnAbilityForeground(const std::shared_ptr<NativeReference> &ability)
{
    if (!ability) {
        TAG_LOGE(AAFwkTag::APPKIT, "ability is nullptr");
        return;
    }
    std::lock_guard<std::recursive_mutex> lock(callbackLock_);
    for (auto callback : callbacks_) {
        if (callback != nullptr) {
            callback->OnAbilityForeground(ability);
        }
    }
}

void ApplicationContext::DispatchOnAbilityBackground(const std::shared_ptr<NativeReference> &ability)
{
    if (!ability) {
        TAG_LOGE(AAFwkTag::APPKIT, "ability is nullptr");
        return;
    }
    std::lock_guard<std::recursive_mutex> lock(callbackLock_);
    for (auto callback : callbacks_) {
        if (callback != nullptr) {
            callback->OnAbilityBackground(ability);
        }
    }
}

void ApplicationContext::DispatchOnAbilityContinue(const std::shared_ptr<NativeReference> &ability)
{
    if (!ability) {
        TAG_LOGE(AAFwkTag::APPKIT, "ability is nullptr");
        return;
    }
    std::lock_guard<std::recursive_mutex> lock(callbackLock_);
    for (auto callback : callbacks_) {
        if (callback != nullptr) {
            callback->OnAbilityContinue(ability);
        }
    }
}

void ApplicationContext::DispatchOnAbilityWillContinue(const std::shared_ptr<NativeReference> &ability)
{
    TAG_LOGD(AAFwkTag::APPKIT, "Dispatch onAbilityWillContinue");
    if (ability == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "ability is nullptr");
        return;
    }

    std::lock_guard<std::recursive_mutex> lock(callbackLock_);
    for (auto callback : callbacks_) {
        if (callback != nullptr) {
            callback->OnAbilityWillContinue(ability);
        }
    }
}

void ApplicationContext::DispatchOnWindowStageWillRestore(const std::shared_ptr<NativeReference> &ability,
    const std::shared_ptr<NativeReference> &windowStage)
{
    TAG_LOGD(AAFwkTag::APPKIT, "Dispatch onWindowStageWillRestore");
    if (ability == nullptr || windowStage == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "ability or windowStage is null");
        return;
    }

    std::lock_guard<std::recursive_mutex> lock(callbackLock_);
    for (auto callback : callbacks_) {
        if (callback != nullptr) {
            callback->OnWindowStageWillRestore(ability, windowStage);
        }
    }
}

void ApplicationContext::DispatchOnWindowStageRestore(const std::shared_ptr<NativeReference> &ability,
    const std::shared_ptr<NativeReference> &windowStage)
{
    TAG_LOGD(AAFwkTag::APPKIT, "Dispatch onWindowStageRestore");
    if (ability == nullptr || windowStage == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "ability or windowStage is null");
        return;
    }

    std::lock_guard<std::recursive_mutex> lock(callbackLock_);
    for (auto callback : callbacks_) {
        if (callback != nullptr) {
            callback->OnWindowStageRestore(ability, windowStage);
        }
    }
}

void ApplicationContext::DispatchOnAbilityWillSaveState(const std::shared_ptr<NativeReference> &ability)
{
    TAG_LOGD(AAFwkTag::APPKIT, "Dispatch onAbilityWillSaveState");
    if (ability == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "ability is nullptr");
        return;
    }

    std::lock_guard<std::recursive_mutex> lock(callbackLock_);
    for (auto callback : callbacks_) {
        if (callback != nullptr) {
            callback->OnAbilityWillSaveState(ability);
        }
    }
}

void ApplicationContext::DispatchOnAbilitySaveState(const std::shared_ptr<NativeReference> &ability)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    if (ability == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "ability is nullptr");
        return;
    }

    std::lock_guard<std::recursive_mutex> lock(callbackLock_);
    for (auto callback : callbacks_) {
        if (callback != nullptr) {
            callback->OnAbilitySaveState(ability);
        }
    }
}

void ApplicationContext::DispatchConfigurationUpdated(const AppExecFwk::Configuration &config)
{
    std::lock_guard<std::recursive_mutex> lock(envCallbacksLock_);
    for (auto envCallback : envCallbacks_) {
        if (envCallback != nullptr) {
            envCallback->OnConfigurationUpdated(config);
        }
    }
}

void ApplicationContext::DispatchMemoryLevel(const int level)
{
    std::lock_guard<std::recursive_mutex> lock(envCallbacksLock_);
    for (auto envCallback : envCallbacks_) {
        if (envCallback != nullptr) {
            envCallback->OnMemoryLevel(level);
        }
    }
}

void ApplicationContext::NotifyApplicationForeground()
{
    std::lock_guard<std::recursive_mutex> lock(applicationStateCallbackLock_);
    for (auto callback : applicationStateCallback_) {
        auto callbackSptr = callback.lock();
        if (callbackSptr != nullptr) {
            callbackSptr->NotifyApplicationForeground();
        }
    }
}

void ApplicationContext::NotifyApplicationBackground()
{
    std::lock_guard<std::recursive_mutex> lock(applicationStateCallbackLock_);
    for (auto callback : applicationStateCallback_) {
        auto callbackSptr = callback.lock();
        if (callbackSptr != nullptr) {
            callbackSptr->NotifyApplicationBackground();
        }
    }
}

void ApplicationContext::DispatchOnWillNewWant(const std::shared_ptr<NativeReference> &ability)
{
    if (!ability) {
        TAG_LOGE(AAFwkTag::APPKIT, "ability is nullptr");
        return;
    }
    std::lock_guard<std::recursive_mutex> lock(callbackLock_);
    for (auto callback : callbacks_) {
        if (callback != nullptr) {
            callback->OnWillNewWant(ability);
        }
    }
}

void ApplicationContext::DispatchOnNewWant(const std::shared_ptr<NativeReference> &ability)
{
    if (!ability) {
        TAG_LOGE(AAFwkTag::APPKIT, "ability is nullptr");
        return;
    }
    std::lock_guard<std::recursive_mutex> lock(callbackLock_);
    for (auto callback : callbacks_) {
        if (callback != nullptr) {
            callback->OnNewWant(ability);
        }
    }
}

void ApplicationContext::DispatchOnAbilityWillCreate(const std::shared_ptr<NativeReference> &ability)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    if (!ability) {
        TAG_LOGE(AAFwkTag::APPKIT, "ability is null");
        return;
    }
    std::lock_guard<std::recursive_mutex> lock(callbackLock_);
    for (auto callback : callbacks_) {
        if (callback != nullptr) {
            callback->OnAbilityWillCreate(ability);
        }
    }
}

void ApplicationContext::DispatchOnWindowStageWillCreate(const std::shared_ptr<NativeReference> &ability,
    const std::shared_ptr<NativeReference> &windowStage)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    if (!ability || !windowStage) {
        TAG_LOGE(AAFwkTag::APPKIT, "ability or windowStage is null");
        return;
    }
    std::lock_guard<std::recursive_mutex> lock(callbackLock_);
    for (auto callback : callbacks_) {
        if (callback != nullptr) {
            callback->OnWindowStageWillCreate(ability, windowStage);
        }
    }
}

void ApplicationContext::DispatchOnWindowStageWillDestroy(const std::shared_ptr<NativeReference> &ability,
    const std::shared_ptr<NativeReference> &windowStage)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    if (!ability || !windowStage) {
        TAG_LOGE(AAFwkTag::APPKIT, "ability or windowStage is null");
        return;
    }
    std::lock_guard<std::recursive_mutex> lock(callbackLock_);
    for (auto callback : callbacks_) {
        if (callback != nullptr) {
            callback->OnWindowStageWillDestroy(ability, windowStage);
        }
    }
}

void ApplicationContext::DispatchOnAbilityWillDestroy(const std::shared_ptr<NativeReference> &ability)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    if (!ability) {
        TAG_LOGE(AAFwkTag::APPKIT, "ability is null");
        return;
    }
    std::lock_guard<std::recursive_mutex> lock(callbackLock_);
    for (auto callback : callbacks_) {
        if (callback != nullptr) {
            callback->OnAbilityWillDestroy(ability);
        }
    }
}

void ApplicationContext::DispatchOnAbilityWillForeground(const std::shared_ptr<NativeReference> &ability)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    if (!ability) {
        TAG_LOGE(AAFwkTag::APPKIT, "ability is null");
        return;
    }
    std::lock_guard<std::recursive_mutex> lock(callbackLock_);
    for (auto callback : callbacks_) {
        if (callback != nullptr) {
            callback->OnAbilityWillForeground(ability);
        }
    }
}

void ApplicationContext::DispatchOnAbilityWillBackground(const std::shared_ptr<NativeReference> &ability)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    if (!ability) {
        TAG_LOGE(AAFwkTag::APPKIT, "ability is null");
        return;
    }
    std::lock_guard<std::recursive_mutex> lock(callbackLock_);
    for (auto callback : callbacks_) {
        if (callback != nullptr) {
            callback->OnAbilityWillBackground(ability);
        }
    }
}

std::string ApplicationContext::GetBundleName() const
{
    return (contextImpl_ != nullptr) ? contextImpl_->GetBundleName() : "";
}

std::shared_ptr<Context> ApplicationContext::CreateBundleContext(const std::string &bundleName)
{
    return (contextImpl_ != nullptr) ? contextImpl_->CreateBundleContext(bundleName) : nullptr;
}

std::shared_ptr<Context> ApplicationContext::CreateModuleContext(const std::string &moduleName)
{
    return contextImpl_ ? contextImpl_->CreateModuleContext(moduleName) : nullptr;
}

std::shared_ptr<Context> ApplicationContext::CreateModuleContext(const std::string &bundleName,
                                                                 const std::string &moduleName)
{
    return contextImpl_ ? contextImpl_->CreateModuleContext(bundleName, moduleName) : nullptr;
}

std::shared_ptr<Global::Resource::ResourceManager> ApplicationContext::CreateModuleResourceManager(
    const std::string &bundleName, const std::string &moduleName)
{
    return contextImpl_ ? contextImpl_->CreateModuleResourceManager(bundleName, moduleName) : nullptr;
}

int32_t ApplicationContext::CreateSystemHspModuleResourceManager(const std::string &bundleName,
    const std::string &moduleName, std::shared_ptr<Global::Resource::ResourceManager> &resourceManager)
{
    return contextImpl_ ?
        contextImpl_->CreateSystemHspModuleResourceManager(bundleName, moduleName, resourceManager) : ERR_INVALID_VALUE;
}

std::shared_ptr<AppExecFwk::ApplicationInfo> ApplicationContext::GetApplicationInfo() const
{
    return (contextImpl_ != nullptr) ? contextImpl_->GetApplicationInfo() : nullptr;
}

void ApplicationContext::SetApplicationInfo(const std::shared_ptr<AppExecFwk::ApplicationInfo> &info)
{
    if (contextImpl_ != nullptr) {
        contextImpl_->SetApplicationInfo(info);
    }
    applicationInfoUpdateFlag_ = true;
}

bool ApplicationContext::GetApplicationInfoUpdateFlag() const
{
    return applicationInfoUpdateFlag_;
}

void ApplicationContext::SetApplicationInfoUpdateFlag(bool flag)
{
    applicationInfoUpdateFlag_ = flag;
}

std::shared_ptr<Global::Resource::ResourceManager> ApplicationContext::GetResourceManager() const
{
    return (contextImpl_ != nullptr) ? contextImpl_->GetResourceManager() : nullptr;
}

std::string ApplicationContext::GetBundleCodePath() const
{
    return (contextImpl_ != nullptr) ? contextImpl_->GetBundleCodePath() : "";
}

std::shared_ptr<AppExecFwk::HapModuleInfo> ApplicationContext::GetHapModuleInfo() const
{
    return nullptr;
}

std::string ApplicationContext::GetBundleCodeDir()
{
    return (contextImpl_ != nullptr) ? contextImpl_->GetBundleCodeDir() : "";
}

std::string ApplicationContext::GetCacheDir()
{
    return (contextImpl_ != nullptr) ? contextImpl_->GetCacheDir() : "";
}

std::string ApplicationContext::GetTempDir()
{
    return (contextImpl_ != nullptr) ? contextImpl_->GetTempDir() : "";
}

std::string ApplicationContext::GetResourceDir()
{
    return (contextImpl_ != nullptr) ? contextImpl_->GetResourceDir() : "";
}

void ApplicationContext::GetAllTempDir(std::vector<std::string> &tempPaths)
{
    if (contextImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "The contextimpl is nullptr");
        return;
    }
    contextImpl_->GetAllTempDir(tempPaths);
}

std::string ApplicationContext::GetFilesDir()
{
    return (contextImpl_ != nullptr) ? contextImpl_->GetFilesDir() : "";
}

void ApplicationContext::KillProcessBySelf(const bool clearPageStack)
{
    if (contextImpl_ != nullptr) {
        contextImpl_->KillProcessBySelf(clearPageStack);
    }
}

int32_t ApplicationContext::GetProcessRunningInformation(AppExecFwk::RunningProcessInfo &info)
{
    return (contextImpl_ != nullptr) ? contextImpl_->GetProcessRunningInformation(info) : -1;
}

bool ApplicationContext::IsUpdatingConfigurations()
{
    return (contextImpl_ != nullptr) ? contextImpl_->IsUpdatingConfigurations() : false;
}

bool ApplicationContext::PrintDrawnCompleted()
{
    return (contextImpl_ != nullptr) ? contextImpl_->PrintDrawnCompleted() : false;
}

std::string ApplicationContext::GetDatabaseDir()
{
    return (contextImpl_ != nullptr) ? contextImpl_->GetDatabaseDir() : "";
}

std::string ApplicationContext::GetPreferencesDir()
{
    return (contextImpl_ != nullptr) ? contextImpl_->GetPreferencesDir() : "";
}

int32_t ApplicationContext::GetSystemDatabaseDir(const std::string &groupId, bool checkExist, std::string &databaseDir)
{
    return contextImpl_ ?
        contextImpl_->GetSystemDatabaseDir(groupId, checkExist, databaseDir) : ERR_INVALID_VALUE;
}

int32_t ApplicationContext::GetSystemPreferencesDir(const std::string &groupId, bool checkExist,
    std::string &preferencesDir)
{
    return contextImpl_ ?
        contextImpl_->GetSystemPreferencesDir(groupId, checkExist, preferencesDir) : ERR_INVALID_VALUE;
}

std::string ApplicationContext::GetGroupDir(std::string groupId)
{
    return (contextImpl_ != nullptr) ? contextImpl_->GetGroupDir(groupId) : "";
}

int32_t ApplicationContext::RestartApp(const AAFwk::Want& want)
{
    std::string abilityName = want.GetElement().GetAbilityName();
    if (abilityName == "") {
        TAG_LOGE(AAFwkTag::APPKIT, "abilityName is empty");
        return ERR_INVALID_VALUE;
    }
    std::string bundleName = GetBundleName();
    const_cast<AAFwk::Want &>(want).SetBundle(bundleName);
    return (contextImpl_ != nullptr) ? contextImpl_->RestartApp(want) : ERR_INVALID_VALUE;
}

std::string ApplicationContext::GetDistributedFilesDir()
{
    return (contextImpl_ != nullptr) ? contextImpl_->GetDistributedFilesDir() : "";
}

std::string ApplicationContext::GetCloudFileDir()
{
    return (contextImpl_ != nullptr) ? contextImpl_->GetCloudFileDir() : "";
}

sptr<IRemoteObject> ApplicationContext::GetToken()
{
    return (contextImpl_ != nullptr) ? contextImpl_->GetToken() : nullptr;
}

void ApplicationContext::SetToken(const sptr<IRemoteObject> &token)
{
    if (contextImpl_ != nullptr) {
        contextImpl_->SetToken(token);
    }
}

void ApplicationContext::SwitchArea(int mode)
{
    if (contextImpl_ != nullptr) {
        contextImpl_->SwitchArea(mode);
    }
}

void ApplicationContext::SetConfiguration(const std::shared_ptr<AppExecFwk::Configuration> &config)
{
    if (contextImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "context is null");
        return;
    }
    contextImpl_->SetConfiguration(config);
}

void ApplicationContext::AppHasDarkRes(bool &darkRes)
{
    if (contextImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "context is null");
        return;
    }
    contextImpl_->AppHasDarkRes(darkRes);
}

void ApplicationContext::SetColorMode(int32_t colorMode)
{
    TAG_LOGD(AAFwkTag::APPKIT, "colorMode:%{public}d", colorMode);
    if (colorMode < -1 || colorMode > 1) {
        TAG_LOGE(AAFwkTag::APPKIT, "colorMode is invalid");
        return;
    }
    AppExecFwk::Configuration config;
    config.AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_COLORMODE, AppExecFwk::GetColorModeStr(colorMode));
    config.AddItem(AAFwk::GlobalConfigurationKey::COLORMODE_IS_SET_BY_APP,
        AppExecFwk::ConfigurationInner::IS_SET_BY_APP);
    if (appConfigChangeCallback_ != nullptr) {
        appConfigChangeCallback_(config);
    }
}

void ApplicationContext::SetLanguage(const std::string &language)
{
    TAG_LOGD(AAFwkTag::APPKIT, "language:%{public}s", language.c_str());
    AppExecFwk::Configuration config;
    config.AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE, language);
    config.AddItem(AAFwk::GlobalConfigurationKey::LANGUAGE_IS_SET_BY_APP,
        AppExecFwk::ConfigurationInner::IS_SET_BY_APP);
    if (appConfigChangeCallback_ != nullptr) {
        appConfigChangeCallback_(config);
    }
}

void ApplicationContext::SetFont(const std::string &font)
{
    TAG_LOGD(AAFwkTag::APPKIT, "font:%{public}s", font.c_str());
    #ifdef SUPPORT_GRAPHICS
    // Notify Window
    AppExecFwk::Configuration config;
    config.AddItem(AppExecFwk::ConfigurationInner::APPLICATION_FONT, font);
    if (appFontCallback_ != nullptr) {
        appFontCallback_(config);
    }
    #endif
}

void ApplicationContext::SetMcc(const std::string &mcc)
{
    if (contextImpl_ != nullptr) {
        contextImpl_->SetMcc(mcc);
    }
}

void ApplicationContext::SetMnc(const std::string &mnc)
{
    if (contextImpl_ != nullptr) {
        contextImpl_->SetMnc(mnc);
    }
}

void ApplicationContext::ClearUpApplicationData()
{
    if (contextImpl_ != nullptr) {
        contextImpl_->ClearUpApplicationData();
    }
}

int ApplicationContext::GetArea()
{
    if (contextImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "contextImpl is nullptr");
        return ContextImpl::EL_DEFAULT;
    }
    return contextImpl_->GetArea();
}

std::shared_ptr<AppExecFwk::Configuration> ApplicationContext::GetConfiguration() const
{
    return (contextImpl_ != nullptr) ? contextImpl_->GetConfiguration() : nullptr;
}

std::string ApplicationContext::GetBaseDir() const
{
    return (contextImpl_ != nullptr) ? contextImpl_->GetBaseDir() : nullptr;
}

Global::Resource::DeviceType ApplicationContext::GetDeviceType() const
{
    return (contextImpl_ != nullptr) ? contextImpl_->GetDeviceType() : Global::Resource::DeviceType::DEVICE_PHONE;
}

void ApplicationContext::RegisterAppConfigUpdateObserver(AppConfigUpdateCallback appConfigChangeCallback)
{
    appConfigChangeCallback_ = appConfigChangeCallback;
}

void ApplicationContext::RegisterAppFontObserver(AppConfigUpdateCallback appFontCallback)
{
    appFontCallback_ = appFontCallback;
}

void ApplicationContext::RegisterProcessSecurityExit(AppProcessExitCallback appProcessExitCallback)
{
    appProcessExitCallback_ = appProcessExitCallback;
}

std::string ApplicationContext::GetAppRunningUniqueId() const
{
    TAG_LOGD(AAFwkTag::APPKIT, "GetAppRunningUniqueId is %{public}s", appRunningUniqueId_.c_str());
    return appRunningUniqueId_;
}

int32_t ApplicationContext::GetCurrentAppCloneIndex()
{
    TAG_LOGD(AAFwkTag::APPKIT, "getCurrentAppCloneIndex is %{public}d", appIndex_);
    return appIndex_;
}

int32_t ApplicationContext::GetCurrentAppMode()
{
    TAG_LOGD(AAFwkTag::APPKIT, "getCurrentMode is %{public}d", appMode_);
    return appMode_;
}


void ApplicationContext::SetAppRunningUniqueId(const std::string &appRunningUniqueId)
{
    TAG_LOGD(AAFwkTag::APPKIT, "SetAppRunningUniqueId is %{public}s", appRunningUniqueId.c_str());
    appRunningUniqueId_ = appRunningUniqueId;
}

int32_t ApplicationContext::SetSupportedProcessCacheSelf(bool isSupport)
{
    if (contextImpl_ != nullptr) {
        return contextImpl_->SetSupportedProcessCacheSelf(isSupport);
    }
    TAG_LOGE(AAFwkTag::APPKIT, "contextImpl_ is nullptr");
    return ERR_INVALID_VALUE;
}

void ApplicationContext::SetCurrentAppCloneIndex(int32_t appIndex)
{
    TAG_LOGD(AAFwkTag::APPKIT, "setCurrentAppCloneIndex is %{public}d", appIndex);
    appIndex_ = appIndex;
}

void ApplicationContext::SetCurrentAppMode(int32_t appMode)
{
    TAG_LOGD(AAFwkTag::APPKIT, "setCurrentAppMode is %{public}d", appMode);
    appMode_ = appMode;
}

void ApplicationContext::ProcessSecurityExit(const AAFwk::ExitReason &exitReason)
{
    if (appProcessExitCallback_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "callback is invalid");
        return;
    }

    TAG_LOGI(AAFwkTag::APPKIT, "Proc exit, reason: %{public}s", exitReason.exitMsg.c_str());
    appProcessExitCallback_(exitReason);
}
}  // namespace AbilityRuntime
}  // namespace OHOS
