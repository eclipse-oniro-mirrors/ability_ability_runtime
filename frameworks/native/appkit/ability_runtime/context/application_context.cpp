/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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
#include "ability_util.h"
#include "app_image_observer_manager.h"
#include "configuration_convertor.h"
#include "exit_reason.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "js_runtime.h"
#include "running_process_info.h"

namespace OHOS {
namespace AbilityRuntime {
const size_t ApplicationContext::CONTEXT_TYPE_ID(std::hash<const char*> {} ("ApplicationContext"));
std::vector<std::shared_ptr<AbilityLifecycleCallback>> ApplicationContext::callbacks_;
std::vector<std::shared_ptr<InteropAbilityLifecycleCallback>> ApplicationContext::interopCallbacks_;
std::vector<std::shared_ptr<EnvironmentCallback>> ApplicationContext::envCallbacks_;
std::vector<std::weak_ptr<ApplicationStateChangeCallback>> ApplicationContext::applicationStateCallback_;
std::vector<std::weak_ptr<SystemConfigurationUpdatedCallback>> ApplicationContext::systemConfigurationUpdatedCallbacks_;

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

std::vector<std::shared_ptr<InteropAbilityLifecycleCallback>> ApplicationContext::GetInteropCallbacks()
{
    std::lock_guard<std::mutex> lock(interopCallbackLock_);
    return interopCallbacks_;
}

void ApplicationContext::RegisterInteropAbilityLifecycleCallback(
    std::shared_ptr<InteropAbilityLifecycleCallback> callback)
{
    TAG_LOGD(AAFwkTag::APPKIT, "RegisterInteropAbilityLifecycleCallback called");
    if (callback == nullptr) {
        return;
    }
    std::lock_guard<std::mutex> lock(interopCallbackLock_);
    interopCallbacks_.push_back(callback);
}

void ApplicationContext::UnregisterInteropAbilityLifecycleCallback(
    std::shared_ptr<InteropAbilityLifecycleCallback> callback)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called UnregisterInteropAbilityLifecycleCallback");
    std::lock_guard<std::mutex> lock(interopCallbackLock_);
    auto it = std::find(interopCallbacks_.begin(), interopCallbacks_.end(), callback);
    if (it != interopCallbacks_.end()) {
        interopCallbacks_.erase(it);
    }
}

bool ApplicationContext::IsAbilityLifecycleCallbackEmpty()
{
    std::lock_guard<std::recursive_mutex> lock(callbackLock_);
    return callbacks_.empty();
}

bool ApplicationContext::IsInteropAbilityLifecycleCallbackEmpty()
{
    std::lock_guard<std::mutex> lock(interopCallbackLock_);
    return interopCallbacks_.empty();
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

void ApplicationContext::RegisterSystemConfigurationUpdatedCallback(
    const std::weak_ptr<SystemConfigurationUpdatedCallback> &callback)
{
    std::lock_guard lock(systemConfigurationUpdatedCallbackLock_);
    systemConfigurationUpdatedCallbacks_.push_back(callback);
}

void ApplicationContext::RegisterApplicationUpdateCallback(
    const std::weak_ptr<ApplicationUpdateCallback> &applicationUpdateCallback)
{
    TAG_LOGD(AAFwkTag::APPKIT, "register update callback");
    AppExecFwk::AppImageObserverManager::GetInstance().RegisterImageLifecycleCallback(applicationUpdateCallback);
}

void ApplicationContext::DispatchOnAbilityCreate(const AbilityLifecycleCallbackArgs &ability)
{
    if (!ability.IsValid()) {
        TAG_LOGE(AAFwkTag::APPKIT, "null ability");
        return;
    }
    std::lock_guard<std::recursive_mutex> lock(callbackLock_);
    for (const auto& callback : callbacks_) {
        if (callback == nullptr) {
            continue;
        }
        if (ability.type_ != callback->type_) {
            continue;
        }
        callback->OnAbilityCreate(ability);
    }
}

void ApplicationContext::DispatchOnAbilityCreate(std::shared_ptr<InteropObject> ability)
{
    TAG_LOGD(AAFwkTag::APPKIT, "interop DispatchOnAbilityCreate");
    if (!ability) {
        TAG_LOGE(AAFwkTag::APPKIT, "null ability");
        return;
    }
    auto callbacksCopy = GetInteropCallbacks();
    for (auto callback : callbacksCopy) {
        if (callback != nullptr) {
            TAG_LOGD(AAFwkTag::APPKIT, "call interop onAbilityCreate");
            callback->OnAbilityCreate(ability);
        }
    }
}

void ApplicationContext::DispatchOnWindowStageCreate(const AbilityLifecycleCallbackArgs &ability,
    const AbilityLifecycleCallbackArgs &windowStage)
{
    if (!ability.IsValid() || !windowStage.IsValid()) {
        TAG_LOGE(AAFwkTag::APPKIT, "null ability or windowStage");
        return;
    }
    std::lock_guard<std::recursive_mutex> lock(callbackLock_);
    for (const auto& callback : callbacks_) {
        if (callback == nullptr) {
            continue;
        }
        if (ability.type_ != callback->type_ ||
            windowStage.type_ != callback->type_) {
            continue;
        }
        callback->OnWindowStageCreate(ability, windowStage);
    }
}

void ApplicationContext::DispatchOnWindowStageCreate(std::shared_ptr<InteropObject> ability,
    std::shared_ptr<InteropObject> windowStage)
{
    TAG_LOGD(AAFwkTag::APPKIT, "interop DispatchOnWindowStageCreate");
    if (!ability || !windowStage) {
        TAG_LOGE(AAFwkTag::APPKIT, "null ability or windowStage");
        return;
    }
    auto callbacksCopy = GetInteropCallbacks();
    for (auto callback : callbacksCopy) {
        if (callback != nullptr) {
            TAG_LOGD(AAFwkTag::APPKIT, "call interop OnWindowStageCreate");
            callback->OnWindowStageCreate(ability, windowStage);
        }
    }
}

void ApplicationContext::DispatchOnWindowStageDestroy(const AbilityLifecycleCallbackArgs &ability,
    const AbilityLifecycleCallbackArgs &windowStage)
{
    if (!ability.IsValid() || !windowStage.IsValid()) {
        TAG_LOGE(AAFwkTag::APPKIT, "null ability or windowStage");
        return;
    }
    std::lock_guard<std::recursive_mutex> lock(callbackLock_);
    for (auto callback : callbacks_) {
        if (callback == nullptr) {
            continue;
        }
        if (ability.type_ != callback->type_ ||
            windowStage.type_ != callback->type_) {
            continue;
        }
        callback->OnWindowStageDestroy(ability, windowStage);
    }
}

void ApplicationContext::DispatchOnWindowStageDestroy(std::shared_ptr<InteropObject> ability,
    std::shared_ptr<InteropObject> windowStage)
{
    TAG_LOGD(AAFwkTag::APPKIT, "interop DispatchOnWindowStageDestroy");
    if (!ability || !windowStage) {
        TAG_LOGE(AAFwkTag::APPKIT, "null ability or windowStage");
        return;
    }
    auto callbacksCopy = GetInteropCallbacks();
    for (auto callback : callbacksCopy) {
        if (callback != nullptr) {
            TAG_LOGD(AAFwkTag::APPKIT, "call interop OnWindowStageDestroy");
            callback->OnWindowStageDestroy(ability, windowStage);
        }
    }
}

void ApplicationContext::DispatchWindowStageFocus(const AbilityLifecycleCallbackArgs &ability,
    const AbilityLifecycleCallbackArgs &windowStage)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    if (!ability.IsValid() || !windowStage.IsValid()) {
        TAG_LOGE(AAFwkTag::APPKIT, "null ability or windowStage");
        return;
    }
    std::lock_guard<std::recursive_mutex> lock(callbackLock_);
    for (auto callback : callbacks_) {
        if (callback == nullptr) {
            continue;
        }
        if (ability.type_ != callback->type_ ||
            windowStage.type_ != callback->type_) {
            continue;
        }
        callback->OnWindowStageActive(ability, windowStage);
    }
}

void ApplicationContext::DispatchWindowStageUnfocus(const AbilityLifecycleCallbackArgs &ability,
    const AbilityLifecycleCallbackArgs &windowStage)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    if (!ability.IsValid() || !windowStage.IsValid()) {
        TAG_LOGE(AAFwkTag::APPKIT, "null ability or windowStage");
        return;
    }
    std::lock_guard<std::recursive_mutex> lock(callbackLock_);
    for (auto callback : callbacks_) {
        if (callback == nullptr) {
            continue;
        }
        if (ability.type_ != callback->type_ ||
            windowStage.type_ != callback->type_) {
            continue;
        }
        callback->OnWindowStageInactive(ability, windowStage);
    }
}

void ApplicationContext::DispatchOnAbilityDestroy(const AbilityLifecycleCallbackArgs &ability)
{
    if (!ability.IsValid()) {
        TAG_LOGE(AAFwkTag::APPKIT, "null ability");
        return;
    }
    std::lock_guard<std::recursive_mutex> lock(callbackLock_);
    for (auto callback : callbacks_) {
        if (callback == nullptr) {
            continue;
        }
        if (ability.type_ != callback->type_) {
            continue;
        }
        callback->OnAbilityDestroy(ability);
    }
}

void ApplicationContext::DispatchOnAbilityDestroy(std::shared_ptr<InteropObject> ability)
{
    TAG_LOGD(AAFwkTag::APPKIT, "interop DispatchOnAbilityDestroy");
    if (!ability) {
        TAG_LOGE(AAFwkTag::APPKIT, "null ability");
        return;
    }
    auto callbacksCopy = GetInteropCallbacks();
    for (auto callback : callbacksCopy) {
        if (callback != nullptr) {
            TAG_LOGD(AAFwkTag::APPKIT, "call interop OnAbilityDestroy");
            callback->OnAbilityDestroy(ability);
        }
    }
}

void ApplicationContext::DispatchOnAbilityForeground(const AbilityLifecycleCallbackArgs &ability)
{
    if (!ability.IsValid()) {
        TAG_LOGE(AAFwkTag::APPKIT, "null ability");
        return;
    }
    std::lock_guard<std::recursive_mutex> lock(callbackLock_);
    for (auto callback : callbacks_) {
        if (callback == nullptr) {
            continue;
        }
        if (ability.type_ != callback->type_) {
            continue;
        }
        callback->OnAbilityForeground(ability);
    }
}

void ApplicationContext::DispatchOnAbilityForeground(std::shared_ptr<InteropObject> ability)
{
    TAG_LOGD(AAFwkTag::APPKIT, "interop DispatchOnAbilityForeground");
    if (!ability) {
        TAG_LOGE(AAFwkTag::APPKIT, "null ability");
        return;
    }
    auto callbacksCopy = GetInteropCallbacks();
    for (auto callback : callbacksCopy) {
        if (callback != nullptr) {
            TAG_LOGD(AAFwkTag::APPKIT, "call interop OnAbilityForeground");
            callback->OnAbilityForeground(ability);
        }
    }
}

void ApplicationContext::DispatchOnAbilityBackground(const AbilityLifecycleCallbackArgs &ability)
{
    if (!ability.IsValid()) {
        TAG_LOGE(AAFwkTag::APPKIT, "null ability");
        return;
    }
    std::lock_guard<std::recursive_mutex> lock(callbackLock_);
    for (auto callback : callbacks_) {
        if (callback == nullptr) {
            continue;
        }
        if (ability.type_ != callback->type_) {
            continue;
        }
        callback->OnAbilityBackground(ability);
    }
}

void ApplicationContext::DispatchOnAbilityBackground(std::shared_ptr<InteropObject> ability)
{
    TAG_LOGD(AAFwkTag::APPKIT, "interop DispatchOnAbilityBackground");
    if (!ability) {
        TAG_LOGE(AAFwkTag::APPKIT, "null ability");
        return;
    }
    auto callbacksCopy = GetInteropCallbacks();
    for (auto callback : callbacksCopy) {
        if (callback != nullptr) {
            TAG_LOGD(AAFwkTag::APPKIT, "call interop OnAbilityBackground");
            callback->OnAbilityBackground(ability);
        }
    }
}

void ApplicationContext::DispatchOnAbilityContinue(const AbilityLifecycleCallbackArgs &ability)
{
    if (!ability.IsValid()) {
        TAG_LOGE(AAFwkTag::APPKIT, "null ability");
        return;
    }
    std::lock_guard<std::recursive_mutex> lock(callbackLock_);
    for (auto callback : callbacks_) {
        if (callback == nullptr) {
            continue;
        }
        if (ability.type_ != callback->type_) {
            continue;
        }
        callback->OnAbilityContinue(ability);
    }
}

void ApplicationContext::DispatchOnAbilityWillContinue(const AbilityLifecycleCallbackArgs &ability)
{
    TAG_LOGD(AAFwkTag::APPKIT, "Dispatch onAbilityWillContinue");
    if (!ability.IsValid()) {
        TAG_LOGE(AAFwkTag::APPKIT, "null ability");
        return;
    }

    std::lock_guard<std::recursive_mutex> lock(callbackLock_);
    for (auto callback : callbacks_) {
        if (callback == nullptr) {
            continue;
        }
        if (ability.type_ != callback->type_) {
            continue;
        }
        callback->OnAbilityWillContinue(ability);
    }
}

void ApplicationContext::DispatchOnWindowStageWillRestore(const AbilityLifecycleCallbackArgs &ability,
    const AbilityLifecycleCallbackArgs &windowStage)
{
    TAG_LOGD(AAFwkTag::APPKIT, "Dispatch onWindowStageWillRestore");
    if (!ability.IsValid() || !windowStage.IsValid()) {
        TAG_LOGE(AAFwkTag::APPKIT, "null ability or windowStage");
        return;
    }

    std::lock_guard<std::recursive_mutex> lock(callbackLock_);
    for (auto callback : callbacks_) {
        if (callback == nullptr) {
            continue;
        }
        if (ability.type_ != callback->type_ ||
            windowStage.type_ != callback->type_) {
            continue;
        }
        callback->OnWindowStageWillRestore(ability, windowStage);
    }
}

void ApplicationContext::DispatchOnWindowStageRestore(const AbilityLifecycleCallbackArgs &ability,
    const AbilityLifecycleCallbackArgs &windowStage)
{
    TAG_LOGD(AAFwkTag::APPKIT, "Dispatch onWindowStageRestore");
    if (!ability.IsValid() || !windowStage.IsValid()) {
        TAG_LOGE(AAFwkTag::APPKIT, "null ability or windowStage");
        return;
    }

    std::lock_guard<std::recursive_mutex> lock(callbackLock_);
    for (auto callback : callbacks_) {
        if (callback == nullptr) {
            continue;
        }
        if (ability.type_ != callback->type_ ||
            windowStage.type_ != callback->type_) {
            continue;
        }
        callback->OnWindowStageRestore(ability, windowStage);
    }
}

void ApplicationContext::DispatchOnAbilityWillSaveState(const AbilityLifecycleCallbackArgs &ability)
{
    TAG_LOGD(AAFwkTag::APPKIT, "Dispatch onAbilityWillSaveState");
    if (!ability.IsValid()) {
        TAG_LOGE(AAFwkTag::APPKIT, "null ability");
        return;
    }

    std::lock_guard<std::recursive_mutex> lock(callbackLock_);
    for (auto callback : callbacks_) {
        if (callback == nullptr) {
            continue;
        }
        if (ability.type_ != callback->type_) {
            continue;
        }
        callback->OnAbilityWillSaveState(ability);
    }
}

void ApplicationContext::DispatchOnAbilitySaveState(const AbilityLifecycleCallbackArgs &ability)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    if (!ability.IsValid()) {
        TAG_LOGE(AAFwkTag::APPKIT, "null ability");
        return;
    }

    std::lock_guard<std::recursive_mutex> lock(callbackLock_);
    for (auto callback : callbacks_) {
        if (callback == nullptr) {
            continue;
        }
        if (ability.type_ != callback->type_) {
            continue;
        }
        callback->OnAbilitySaveState(ability);
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

void ApplicationContext::NotifySystemConfigurationUpdated(const AppExecFwk::Configuration& configuration)
{
    std::lock_guard lock(systemConfigurationUpdatedCallbackLock_);
    for (auto callback : systemConfigurationUpdatedCallbacks_) {
        auto callbackSptr = callback.lock();
        if (callbackSptr != nullptr) {
            callbackSptr->NotifySystemConfigurationUpdated(configuration);
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

void ApplicationContext::DispatchOnWillNewWant(const AbilityLifecycleCallbackArgs &ability)
{
    if (!ability.IsValid()) {
        TAG_LOGE(AAFwkTag::APPKIT, "null ability");
        return;
    }
    std::lock_guard<std::recursive_mutex> lock(callbackLock_);
    for (auto callback : callbacks_) {
        if (callback == nullptr) {
            continue;
        }
        if (ability.type_ != callback->type_) {
            continue;
        }
        callback->OnWillNewWant(ability);
    }
}

void ApplicationContext::DispatchOnNewWant(const AbilityLifecycleCallbackArgs &ability)
{
    if (!ability.IsValid()) {
        TAG_LOGE(AAFwkTag::APPKIT, "null ability");
        return;
    }
    std::lock_guard<std::recursive_mutex> lock(callbackLock_);
    for (auto callback : callbacks_) {
        if (callback == nullptr) {
            continue;
        }
        if (ability.type_ != callback->type_) {
            continue;
        }
        callback->OnNewWant(ability);
    }
}

void ApplicationContext::DispatchOnAbilityWillCreate(const AbilityLifecycleCallbackArgs &ability)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    if (!ability.IsValid()) {
        TAG_LOGE(AAFwkTag::APPKIT, "null ability");
        return;
    }
    std::lock_guard<std::recursive_mutex> lock(callbackLock_);
    for (auto callback : callbacks_) {
        if (callback == nullptr) {
            continue;
        }
        if (ability.type_ != callback->type_) {
            continue;
        }
        callback->OnAbilityWillCreate(ability);
    }
}

void ApplicationContext::DispatchOnWindowStageWillCreate(const AbilityLifecycleCallbackArgs &ability,
    const AbilityLifecycleCallbackArgs &windowStage)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    if (!ability.IsValid() || !windowStage.IsValid()) {
        TAG_LOGE(AAFwkTag::APPKIT, "null ability or windowStage");
        return;
    }
    std::lock_guard<std::recursive_mutex> lock(callbackLock_);
    for (auto callback : callbacks_) {
        if (callback == nullptr) {
            continue;
        }
        if (ability.type_ != callback->type_ ||
            windowStage.type_ != callback->type_) {
            continue;
        }
        callback->OnWindowStageWillCreate(ability, windowStage);
    }
}

void ApplicationContext::DispatchOnWindowStageWillDestroy(const AbilityLifecycleCallbackArgs &ability,
    const AbilityLifecycleCallbackArgs &windowStage)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    if (!ability.IsValid() || !windowStage.IsValid()) {
        TAG_LOGE(AAFwkTag::APPKIT, "null ability or windowStage");
        return;
    }
    std::lock_guard<std::recursive_mutex> lock(callbackLock_);
    for (auto callback : callbacks_) {
        if (callback == nullptr) {
            continue;
        }
        if (ability.type_ != callback->type_ ||
            windowStage.type_ != callback->type_) {
            continue;
        }
        callback->OnWindowStageWillDestroy(ability, windowStage);
    }
}

void ApplicationContext::DispatchOnAbilityWillDestroy(const AbilityLifecycleCallbackArgs &ability)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    if (!ability.IsValid()) {
        TAG_LOGE(AAFwkTag::APPKIT, "null ability");
        return;
    }
    std::lock_guard<std::recursive_mutex> lock(callbackLock_);
    for (auto callback : callbacks_) {
        if (callback == nullptr) {
            continue;
        }
        if (ability.type_ != callback->type_) {
            continue;
        }
        callback->OnAbilityWillDestroy(ability);
    }
}

void ApplicationContext::DispatchOnAbilityWillForeground(const AbilityLifecycleCallbackArgs &ability)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    if (!ability.IsValid()) {
        TAG_LOGE(AAFwkTag::APPKIT, "null ability");
        return;
    }
    std::lock_guard<std::recursive_mutex> lock(callbackLock_);
    for (auto callback : callbacks_) {
        if (callback == nullptr) {
            continue;
        }
        if (ability.type_ != callback->type_) {
            continue;
        }
        callback->OnAbilityWillForeground(ability);
    }
}

void ApplicationContext::DispatchOnAbilityWillBackground(const AbilityLifecycleCallbackArgs &ability)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    if (!ability.IsValid()) {
        TAG_LOGE(AAFwkTag::APPKIT, "null ability");
        return;
    }
    std::lock_guard<std::recursive_mutex> lock(callbackLock_);
    for (auto callback : callbacks_) {
        if (callback == nullptr) {
            continue;
        }
        if (ability.type_ != callback->type_) {
            continue;
        }
        callback->OnAbilityWillBackground(ability);
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

std::string ApplicationContext::GetResourceDir(const std::string &moduleName)
{
    return (contextImpl_ != nullptr) ? contextImpl_->GetResourceDir(moduleName) : "";
}

bool ApplicationContext::IsModuleExist(const std::string &moduleName)
{
    return (contextImpl_ != nullptr) ? contextImpl_->IsModuleExist(moduleName) : false;
}

void ApplicationContext::GetAllTempBase(std::vector<std::string> &tempPaths)
{
    if (contextImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null contextImpl");
        return;
    }
    contextImpl_->GetAllTempBase(tempPaths);
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

#ifdef SUPPORT_SCREEN
void ApplicationContext::GetAllUIAbilities(std::vector<std::shared_ptr<UIAbility>> &uiAbility)
{
    if (contextImpl_ != nullptr) {
        contextImpl_->GetAllUIAbilities(uiAbility);
    }
}

void ApplicationContext::RegisterGetAllUIAbilitiesCallback(GetAllUIAbilitiesCallback getAllUIAbilitiesCallback)
{
    if (contextImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null contextImpl_");
        return;
    }
    contextImpl_->RegisterGetAllUIAbilitiesCallback(getAllUIAbilitiesCallback);
}
#endif

int32_t ApplicationContext::GetAllRunningInstanceKeys(std::vector<std::string> &instanceKeys)
{
    return (contextImpl_ != nullptr) ? contextImpl_->GetAllRunningInstanceKeys(instanceKeys) : -1;
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
        TAG_LOGE(AAFwkTag::APPKIT, "abilityName empty");
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

std::string ApplicationContext::GetLogFileDir()
{
    return (contextImpl_ != nullptr) ? contextImpl_->GetLogFileDir() : "";
}

std::string ApplicationContext::GetLaunchParameter() const
{
    return (contextImpl_ != nullptr) ? contextImpl_->GetLaunchParameter() : "";
}

void ApplicationContext::SetLaunchParameter(const AAFwk::Want &want)
{
    if (contextImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null contextImpl_");
        return;
    }
    contextImpl_->SetLaunchParameter(want);
    contextImpl_->SetLatestParameter(want);
}

std::string ApplicationContext::GetLatestParameter() const
{
    return (contextImpl_ != nullptr) ? contextImpl_->GetLatestParameter() : "";
}

void ApplicationContext::SetLatestParameter(const AAFwk::Want &want)
{
    if (contextImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null contextImpl_");
        return;
    }
    contextImpl_->SetLatestParameter(want);
}

ConfigUpdateReason ApplicationContext::GetConfigUpdateReason()
{
    return (contextImpl_ != nullptr) ? contextImpl_->GetConfigUpdateReason() :
        ConfigUpdateReason::CONFIG_UPDATE_REASON_DEFAULT;
}

void ApplicationContext::SetConfigUpdateReason(ConfigUpdateReason reason)
{
    TAG_LOGD(AAFwkTag::APPKIT, "config update scene:%{public}u", static_cast<uint8_t>(reason));
    if (contextImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null contextImpl_");
        return;
    }
    contextImpl_->SetConfigUpdateReason(reason);
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
        TAG_LOGE(AAFwkTag::APPKIT, "null context");
        return;
    }
    contextImpl_->SetConfiguration(config);
}

void ApplicationContext::AppHasDarkRes(bool &darkRes)
{
    if (contextImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null context");
        return;
    }
    contextImpl_->AppHasDarkRes(darkRes);
}

void ApplicationContext::SetColorMode(int32_t colorMode)
{
    TAG_LOGD(AAFwkTag::APPKIT, "colorMode:%{public}d", colorMode);
    if (colorMode < -1 || colorMode > 1) {
        TAG_LOGE(AAFwkTag::APPKIT, "colorMode invalid");
        return;
    }
    AppExecFwk::Configuration config;
    config.AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_COLORMODE, AppExecFwk::GetColorModeStr(colorMode));
    if (appConfigChangeCallback_ != nullptr) {
        appConfigChangeCallback_(config);
    }
}

void ApplicationContext::SetLanguage(const std::string &language)
{
    TAG_LOGD(AAFwkTag::APPKIT, "language:%{public}s", language.c_str());
    AppExecFwk::Configuration config;
    config.AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE, language);
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

bool ApplicationContext::SetFontSizeScale(double fontSizeScale)
{
    if (contextImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null context");
        return false;
    }

    AppExecFwk::Configuration config;
    config.AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_FONT_SIZE_SCALE, std::to_string(fontSizeScale));
    if (appConfigChangeCallback_ != nullptr) {
        appConfigChangeCallback_(config);
    }
    TAG_LOGD(AAFwkTag::APPKIT, "SetFontSizeScale callback ok");
    return true;
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
        TAG_LOGE(AAFwkTag::APPKIT, "null contextImpl");
        return ContextImpl::EL_DEFAULT;
    }
    return contextImpl_->GetArea();
}

std::string ApplicationContext::GetProcessName()
{
    return (contextImpl_ != nullptr) ? contextImpl_->GetProcessName() : "";
}

std::shared_ptr<AppExecFwk::Configuration> ApplicationContext::GetConfiguration() const
{
    return (contextImpl_ != nullptr) ? contextImpl_->GetConfiguration() : nullptr;
}

std::string ApplicationContext::GetBaseDir() const
{
    return (contextImpl_ != nullptr) ? contextImpl_->GetBaseDir() : "";
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

void ApplicationContext::RegisterAppGetSpecifiedRuntime(AppGetSpecifiedRuntimeCallback appGetSpecifiedRuntimeCallback)
{
    appGetSpecifiedRuntimeCallback_ = appGetSpecifiedRuntimeCallback;
}

#ifdef SUPPORT_GRAPHICS
void ApplicationContext::RegisterGetDisplayConfig(GetDisplayConfigCallback getDisplayConfigCallback)
{
    if (contextImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null contextImpl_");
        return;
    }
    contextImpl_->RegisterGetDisplayConfig(getDisplayConfigCallback);
}
#endif

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

std::string ApplicationContext::GetCurrentInstanceKey()
{
    TAG_LOGD(AAFwkTag::APPKIT, "getCurrentInstanceKey is %{public}s", instanceKey_.c_str());
    return instanceKey_;
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
    TAG_LOGE(AAFwkTag::APPKIT, "null contextImpl_");
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

void ApplicationContext::SetCurrentInstanceKey(const std::string& instanceKey)
{
    TAG_LOGD(AAFwkTag::APPKIT, "setCurrentInstanceKey is %{public}s", instanceKey.c_str());
    instanceKey_ = instanceKey;
}

void ApplicationContext::ProcessSecurityExit(const AAFwk::ExitReason &exitReason)
{
    if (appProcessExitCallback_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null callback");
        return;
    }

    TAG_LOGI(AAFwkTag::APPKIT, "Proc exit, reason: %{public}s", exitReason.exitMsg.c_str());
    appProcessExitCallback_(exitReason);
}

napi_env ApplicationContext::GetMainNapiEnv() const
{
    if (appGetSpecifiedRuntimeCallback_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null callback");
        return nullptr;
    }
    const std::unique_ptr<Runtime> &runtime = appGetSpecifiedRuntimeCallback_(
        AppExecFwk::Constants::ARKTS_MODE_DYNAMIC);
    if (runtime == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null runtime");
        return nullptr;
    }

    if (runtime->GetLanguage() != Runtime::Language::JS) {
        TAG_LOGD(AAFwkTag::APPKIT, "not js runtime");
        return nullptr;
    }
    napi_env env = static_cast<JsRuntime&>(*runtime).GetNapiEnv();
    return env;
}

int32_t ApplicationContext::GetImageProcessType() const
{
    return AppExecFwk::AppImageObserverManager::GetInstance().GetImageProcessType();
}

bool ApplicationContext::IsAbilityCreated() const
{
    return AppExecFwk::AppImageObserverManager::GetInstance().IsAbilityCreated();
}

std::string ApplicationContext::GetDataDir()
{
    std::lock_guard<std::mutex> lock(dataDirMutex_);
    if (dataDir_.empty()) {
        auto bundleManagerHelper = AAFwk::AbilityUtil::GetBundleManagerHelper();
        dataDir_ = bundleManagerHelper->GetDataDir(GetBundleName(), appIndex_);
        if (dataDir_.empty()) {
            TAG_LOGE(AAFwkTag::APPKIT, "dataDir is empty");
            return "";
        }
    }
    TAG_LOGD(AAFwkTag::APPKIT, "GetDataDir is %{public}s", dataDir_.c_str());
    return dataDir_;
}

std::shared_ptr<Context> ApplicationContext::CreateAreaModeContext(int areaMode)
{
    return contextImpl_ ? contextImpl_->CreateAreaModeContext(areaMode) : nullptr;
}

std::shared_ptr<Context> ApplicationContext::CreateModuleOrPluginContext(const std::string &bundleName,
    const std::string &moduleName)
{
    return contextImpl_ ? contextImpl_->CreateModuleOrPluginContext(bundleName, moduleName) : nullptr;
}

#ifdef SUPPORT_GRAPHICS
std::shared_ptr<Context> ApplicationContext::CreateDisplayContext(uint64_t displayId)
{
    return contextImpl_ ? contextImpl_->CreateDisplayContext(displayId) : nullptr;
}
#endif
}  // namespace AbilityRuntime
}  // namespace OHOS
