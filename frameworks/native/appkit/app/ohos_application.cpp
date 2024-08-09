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

#include <cstdio>
#include <cstring>
#include <fcntl.h>

#include <sys/stat.h>

#include "ohos_application.h"

#include "ability.h"
#include "ability_record_mgr.h"
#include "ability_stage_context.h"
#include "ability_thread.h"
#include "app_loader.h"
#include "application_context.h"
#include "application_cleaner.h"
#include "application_impl.h"
#include "bundle_mgr_helper.h"
#include "configuration_utils.h"
#include "context_impl.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "iservice_registry.h"
#include "runtime.h"
#include "system_ability_definition.h"
#include "syspara/parameter.h"
#include "ui_ability.h"
#ifdef SUPPORT_GRAPHICS
#include "window.h"
#endif

namespace OHOS {
namespace AppExecFwk {
namespace {
    constexpr const char* PERSIST_DARKMODE_KEY = "persist.ace.darkmode";
}
REGISTER_APPLICATION(OHOSApplication, OHOSApplication)
constexpr int32_t APP_ENVIRONMENT_OVERWRITE = 1;

OHOSApplication::OHOSApplication()
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    abilityLifecycleCallbacks_.clear();
    elementsCallbacks_.clear();
}

OHOSApplication::~OHOSApplication()
{
}

/**
 *
 * @brief Called when Ability#onSaveAbilityState(PacMap) was called on an ability.
 *
 * @param outState Indicates the PacMap object passed to Ability#onSaveAbilityState(PacMap)
 * for storing user data and states. This parameter cannot be null.
 */

void OHOSApplication::DispatchAbilitySavedState(const PacMap &outState)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    for (auto callback : abilityLifecycleCallbacks_) {
        if (callback != nullptr) {
            callback->OnAbilitySaveState(outState);
        }
    }
}

/**
 *
 * @brief Will be called the application foregrounds
 *
 */
void OHOSApplication::OnForeground()
{
    TAG_LOGD(AAFwkTag::APPKIT, "begin");
    if (abilityRuntimeContext_) {
        abilityRuntimeContext_->NotifyApplicationForeground();
    }

    if (runtime_ == nullptr) {
        TAG_LOGD(AAFwkTag::APPKIT, "NotifyApplicationState, runtime_ is nullptr");
        return;
    }
    runtime_->NotifyApplicationState(false);
    TAG_LOGD(AAFwkTag::APPKIT, "NotifyApplicationState::OnForeground end");
}

/**
 *
 * @brief Will be called the application backgrounds
 *
 */
void OHOSApplication::OnBackground()
{
    TAG_LOGD(AAFwkTag::APPKIT, "begin");
    if (abilityRuntimeContext_) {
        abilityRuntimeContext_->NotifyApplicationBackground();
    }

    if (runtime_ == nullptr) {
        TAG_LOGD(AAFwkTag::APPKIT, "runtime_ is nullptr");
        return;
    }
    runtime_->NotifyApplicationState(true);
}

void OHOSApplication::DumpApplication()
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    // create and initialize abilityInfos
    std::shared_ptr<AbilityInfo> abilityInfo = nullptr;
    std::shared_ptr<AbilityLocalRecord> record = nullptr;

    if (abilityRecordMgr_) {
        record = abilityRecordMgr_->GetAbilityItem(abilityRecordMgr_->GetToken());
    }

    if (record) {
        abilityInfo = record->GetAbilityInfo();
    }

    if (abilityInfo) {
        TAG_LOGD(AAFwkTag::APPKIT, "==============AbilityInfo==============");
        TAG_LOGD(AAFwkTag::APPKIT, "abilityInfo: package: %{public}s", abilityInfo->package.c_str());
        TAG_LOGD(AAFwkTag::APPKIT, "abilityInfo: name: %{public}s", abilityInfo->name.c_str());
        TAG_LOGD(AAFwkTag::APPKIT, "abilityInfo: label: %{public}s", abilityInfo->label.c_str());
        TAG_LOGD(AAFwkTag::APPKIT, "abilityInfo: description: %{public}s", abilityInfo->description.c_str());
        TAG_LOGD(AAFwkTag::APPKIT, "abilityInfo: iconPath: %{public}s", abilityInfo->iconPath.c_str());
        TAG_LOGD(AAFwkTag::APPKIT, "abilityInfo: visible: %{public}d", abilityInfo->visible);
        TAG_LOGD(AAFwkTag::APPKIT, "abilityInfo: kind: %{public}s", abilityInfo->kind.c_str());
        TAG_LOGD(AAFwkTag::APPKIT, "abilityInfo: type: %{public}d", abilityInfo->type);
        TAG_LOGD(AAFwkTag::APPKIT, "abilityInfo: orientation: %{public}d", abilityInfo->orientation);
        TAG_LOGD(AAFwkTag::APPKIT, "abilityInfo: launchMode: %{public}d", abilityInfo->launchMode);
        for (auto permission : abilityInfo->permissions) {
            TAG_LOGD(AAFwkTag::APPKIT, "abilityInfo: permission: %{public}s", permission.c_str());
        }
        TAG_LOGD(AAFwkTag::APPKIT, "abilityInfo: bundleName: %{public}s", abilityInfo->bundleName.c_str());
        TAG_LOGD(AAFwkTag::APPKIT, "abilityInfo: applicationName: %{public}s", abilityInfo->applicationName.c_str());
    }

    // create and initialize applicationInfo
    std::shared_ptr<ApplicationInfo> applicationInfoPtr = GetApplicationInfo();
    if (applicationInfoPtr != nullptr) {
        TAG_LOGD(AAFwkTag::APPKIT, "applicationInfo: name: %{public}s", applicationInfoPtr->name.c_str());
        TAG_LOGD(AAFwkTag::APPKIT, "applicationInfo: bundleName: %{public}s", applicationInfoPtr->bundleName.c_str());
        TAG_LOGD(
            AAFwkTag::APPKIT, "applicationInfo: signatureKey: %{public}s", applicationInfoPtr->signatureKey.c_str());
    }
}

/**
 * @brief Set Runtime
 *
 * @param runtime Runtime instance.
 */
void OHOSApplication::SetRuntime(std::unique_ptr<AbilityRuntime::Runtime>&& runtime)
{
    TAG_LOGD(AAFwkTag::APPKIT, "begin");
    if (runtime == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "runtime is empty");
        return;
    }
    runtime_ = std::move(runtime);
}

/**
 * @brief Set ApplicationContext
 *
 * @param abilityRuntimeContext ApplicationContext instance.
 */
void OHOSApplication::SetApplicationContext(
    const std::shared_ptr<AbilityRuntime::ApplicationContext> &abilityRuntimeContext)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    if (abilityRuntimeContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "context is empty");
        return;
    }
    abilityRuntimeContext_ = abilityRuntimeContext;
    auto application = std::static_pointer_cast<OHOSApplication>(shared_from_this());
    std::weak_ptr<OHOSApplication> applicationWptr = application;
    abilityRuntimeContext_->RegisterAppConfigUpdateObserver([applicationWptr](const Configuration &config) {
        std::shared_ptr<OHOSApplication> applicationSptr = applicationWptr.lock();
        if (applicationSptr == nullptr) {
            TAG_LOGE(AAFwkTag::APPKIT, "application is nullptr.");
            return;
        }
        applicationSptr->OnConfigurationUpdated(config);
    });
    abilityRuntimeContext_->RegisterAppFontObserver([applicationWptr](const Configuration &config) {
        std::shared_ptr<OHOSApplication> applicationSptr = applicationWptr.lock();
        if (applicationSptr == nullptr) {
            TAG_LOGE(AAFwkTag::APPKIT, "application is nullptr.");
            return;
        }
        applicationSptr->OnFontUpdated(config);
    });
}

/**
 *
 * @brief Set the abilityRecordMgr to the OHOSApplication.
 *
 * @param abilityRecordMgr
 */
void OHOSApplication::SetAbilityRecordMgr(const std::shared_ptr<AbilityRecordMgr> &abilityRecordMgr)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    if (abilityRecordMgr == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "abilityRecordMgr is nullptr");
        return;
    }
    abilityRecordMgr_ = abilityRecordMgr;
}

/**
 *
 * Register AbilityLifecycleCallbacks with OHOSApplication
 *
 * @param callBack callBack When the life cycle of the ability in the application changes,
 */
void OHOSApplication::RegisterAbilityLifecycleCallbacks(const std::shared_ptr<AbilityLifecycleCallbacks> &callBack)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");

    if (callBack == nullptr) {
        TAG_LOGD(AAFwkTag::APPKIT, "observer is null");
        return;
    }

    abilityLifecycleCallbacks_.emplace_back(callBack);
}

/**
 *
 * Unregister AbilityLifecycleCallbacks with OHOSApplication
 *
 * @param callBack RegisterAbilityLifecycleCallbacks`s callBack
 */
void OHOSApplication::UnregisterAbilityLifecycleCallbacks(const std::shared_ptr<AbilityLifecycleCallbacks> &callBack)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");

    if (callBack == nullptr) {
        TAG_LOGD(AAFwkTag::APPKIT, "observer is null");
        return;
    }

    abilityLifecycleCallbacks_.remove(callBack);
}

/**
 *
 * Will be called when the given ability calls Ability->onStart
 *
 * @param Ability Indicates the ability object that calls the onStart() method.
 */
void OHOSApplication::OnAbilityStart(const std::shared_ptr<AbilityRuntime::UIAbility> &ability)
{
    if (ability == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "ability is nullptr");
        return;
    }

    TAG_LOGD(AAFwkTag::APPKIT, "called");
    for (auto callback : abilityLifecycleCallbacks_) {
        if (callback != nullptr) {
            callback->OnAbilityStart(ability);
        }
    }
}

/**
 *
 * Will be called when the given ability calls Ability->onInactive
 *
 * @param Ability Indicates the Ability object that calls the onInactive() method.
 */
void OHOSApplication::OnAbilityInactive(const std::shared_ptr<AbilityRuntime::UIAbility> &ability)
{
    if (ability == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "ability is nullptr");
        return;
    }

    TAG_LOGD(AAFwkTag::APPKIT, "called");
    for (auto callback : abilityLifecycleCallbacks_) {
        if (callback != nullptr) {
            callback->OnAbilityInactive(ability);
        }
    }
}

/**
 *
 * Will be called when the given ability calls Ability->onBackground
 *
 * @param Ability Indicates the Ability object that calls the onBackground() method.
 */
void OHOSApplication::OnAbilityBackground(const std::shared_ptr<AbilityRuntime::UIAbility> &ability)
{
    if (ability == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "ability is nullptr");
        return;
    }

    TAG_LOGD(AAFwkTag::APPKIT, "called");
    for (auto callback : abilityLifecycleCallbacks_) {
        if (callback != nullptr) {
            callback->OnAbilityBackground(ability);
        }
    }
}

/**
 *
 * Will be called when the given ability calls Ability->onForeground
 *
 * @param Ability Indicates the Ability object that calls the onForeground() method.
 */
void OHOSApplication::OnAbilityForeground(const std::shared_ptr<AbilityRuntime::UIAbility> &ability)
{
    if (ability == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "ability is nullptr");
        return;
    }

    TAG_LOGD(AAFwkTag::APPKIT, "called");
    for (auto callback : abilityLifecycleCallbacks_) {
        if (callback != nullptr) {
            callback->OnAbilityForeground(ability);
        }
    }
}

/**
 *
 * Will be called when the given ability calls Ability->onActive
 *
 * @param Ability Indicates the Ability object that calls the onActive() method.
 */
void OHOSApplication::OnAbilityActive(const std::shared_ptr<AbilityRuntime::UIAbility> &ability)
{
    if (ability == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "ability is nullptr");
        return;
    }

    TAG_LOGD(AAFwkTag::APPKIT, "called");
    for (auto callback : abilityLifecycleCallbacks_) {
        if (callback != nullptr) {
            callback->OnAbilityActive(ability);
        }
    }
}

/**
 *
 * Will be called when the given ability calls Ability->onStop
 *
 * @param Ability Indicates the Ability object that calls the onStop() method.
 */
void OHOSApplication::OnAbilityStop(const std::shared_ptr<AbilityRuntime::UIAbility> &ability)
{
    if (ability == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "ability is nullptr");
        return;
    }

    TAG_LOGD(AAFwkTag::APPKIT, "called");
    for (auto callback : abilityLifecycleCallbacks_) {
        if (callback != nullptr) {
            callback->OnAbilityStop(ability);
        }
    }
}

/**
 *
 * @brief Register ElementsCallback with OHOSApplication
 *
 * @param callBack callBack when the system configuration of the device changes.
 */
void OHOSApplication::RegisterElementsCallbacks(const std::shared_ptr<ElementsCallback> &callback)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");

    if (callback == nullptr) {
        TAG_LOGD(AAFwkTag::APPKIT, "observer is null");
        return;
    }

    elementsCallbacks_.emplace_back(callback);
}

/**
 *
 * @brief Unregister ElementsCallback with OHOSApplication
 *
 * @param callback RegisterElementsCallbacks`s callback
 */
void OHOSApplication::UnregisterElementsCallbacks(const std::shared_ptr<ElementsCallback> &callback)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");

    if (callback == nullptr) {
        TAG_LOGD(AAFwkTag::APPKIT, "observer is null");
        return;
    }

    elementsCallbacks_.remove(callback);
}

/**
 *
 * @brief Will be Called when the system configuration of the device changes.
 *
 * @param config Indicates the new Configuration object.
 */
void OHOSApplication::OnConfigurationUpdated(Configuration config)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (!abilityRecordMgr_ || !configuration_) {
        TAG_LOGD(AAFwkTag::APPKIT, "abilityRecordMgr_ or configuration_ is null");
        return;
    }
    std::string language = config.GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE);
    std::string colorMode = config.GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_COLORMODE);
    std::string languageIsSetByApp =
        config.GetItem(AAFwk::GlobalConfigurationKey::LANGUAGE_IS_SET_BY_APP);
    std::string colorModeIsSetByApp =
        config.GetItem(AAFwk::GlobalConfigurationKey::COLORMODE_IS_SET_BY_APP);
    // COLORMODE_NEED_REMOVE_SET_BY_SA added when display move.
    std::string colorModeNeedRemoveIsSetBySa =
        config.GetItem(AAFwk::GlobalConfigurationKey::COLORMODE_NEED_REMOVE_SET_BY_SA);
    std::string colorModeIsSetBySa =
        config.GetItem(AAFwk::GlobalConfigurationKey::COLORMODE_IS_SET_BY_SA);
    std::string globalColorMode =
        configuration_->GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_COLORMODE);
    std::string globalLanguageIsSetByApp =
        configuration_->GetItem(AAFwk::GlobalConfigurationKey::LANGUAGE_IS_SET_BY_APP);
    std::string globalColorModeIsSetBySa =
        configuration_->GetItem(AAFwk::GlobalConfigurationKey::COLORMODE_IS_SET_BY_SA);
    // Whether the color changes with the system
    bool isUpdateAppColor = isUpdateColor(config, colorMode, globalColorMode,
        globalColorModeIsSetBySa, colorModeIsSetByApp, colorModeIsSetBySa);
    // Whether the font changes with the system
    bool isUpdateAppFontSize = isUpdateFontSize(config);
    // Whether the language changes with the system
    bool isUpdateAppLanguage = isUpdateLanguage(config, language, languageIsSetByApp, globalLanguageIsSetByApp);
    // If size is not equal to 0, other keys need to be updated.
    if (!isUpdateAppColor && !isUpdateAppFontSize && !isUpdateAppLanguage && config.GetItemSize() == 0) {
        TAG_LOGD(AAFwkTag::UIABILITY, "Configuration does not need to be updated");
        return;
    }
    std::vector<std::string> changeKeyV;
    {
        HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, "configuration_->CompareDifferent");
        configuration_->CompareDifferent(changeKeyV, config);
        configuration_->Merge(changeKeyV, config);
    }
    TAG_LOGD(AAFwkTag::UIABILITY, "configuration_: %{public}s", configuration_->GetName().c_str());

    // Update resConfig of resource manager, which belongs to application context.
    UpdateAppContextResMgr(config);

    // Notify all abilities
    TAG_LOGI(AAFwkTag::APPKIT, "recordCount: [%{public}d]", static_cast<int>(abilityRecordMgr_->GetRecordCount()));
    for (const auto &abilityToken : abilityRecordMgr_->GetAllTokens()) {
        auto abilityRecord = abilityRecordMgr_->GetAbilityItem(abilityToken);
        if (abilityRecord && abilityRecord->GetAbilityThread()) {
            abilityRecord->GetAbilityThread()->ScheduleUpdateConfiguration(config);
        }
    }

    // Notify AbilityStage
    TAG_LOGI(AAFwkTag::APPKIT, "abilityStages size: [%{public}zu]", abilityStages_.size());
    for (auto it = abilityStages_.begin(); it != abilityStages_.end(); it++) {
        auto abilityStage = it->second;
        if (abilityStage) {
            abilityStage->OnConfigurationUpdated(config);
        }
    }

#ifdef SUPPORT_GRAPHICS
    // Notify Window
    TAG_LOGD(AAFwkTag::APPKIT, "Update configuration for all window.");
    auto diffConfiguration = std::make_shared<AppExecFwk::Configuration>(config);
    Rosen::Window::UpdateConfigurationForAll(diffConfiguration);
#endif

    for (auto callback : elementsCallbacks_) {
        if (callback != nullptr) {
            callback->OnConfigurationUpdated(nullptr, config);
        }
    }

    abilityRuntimeContext_->DispatchConfigurationUpdated(*configuration_);
    abilityRuntimeContext_->SetMcc(configuration_->GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_MCC));
    abilityRuntimeContext_->SetMnc(configuration_->GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_MNC));

    if (colorMode.compare(ConfigurationInner::COLOR_MODE_AUTO) == 0 ||
        (globalColorMode.compare(ConfigurationInner::COLOR_MODE_AUTO) == 0 && (colorModeIsSetByApp.empty() ||
        !colorModeIsSetBySa.empty()))) {
        configuration_->AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_COLORMODE, ConfigurationInner::COLOR_MODE_AUTO);
    }

    // When display move happened, need to remove SA key, so setting can update colormode success after display move.
    if (!colorModeNeedRemoveIsSetBySa.empty() && !globalColorModeIsSetBySa.empty()) {
        configuration_->RemoveItem(AAFwk::GlobalConfigurationKey::COLORMODE_IS_SET_BY_SA);
        configuration_->RemoveItem(AAFwk::GlobalConfigurationKey::COLORMODE_NEED_REMOVE_SET_BY_SA);
    }

    abilityRuntimeContext_->SetConfiguration(configuration_);
}

/**
 *
 * @brief Will be Called when the application font of the device changes.
 *
 * @param config Indicates the new Configuration object.
 */
void OHOSApplication::OnFontUpdated(Configuration config)
{
    #ifdef SUPPORT_GRAPHICS
    // Notify Window
    auto diffConfiguration = std::make_shared<AppExecFwk::Configuration>(config);
    Rosen::Window::UpdateConfigurationForAll(diffConfiguration);
    #endif
}

/**
 *
 * @brief Called when the system has determined to trim the memory, for example,
 * when the ability is running in the background and there is no enough memory for
 * running as many background processes as possible.
 *
 * @param level Indicates the memory trim level, which shows the current memory usage status.
 */
void OHOSApplication::OnMemoryLevel(int level)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");

    if (abilityRecordMgr_) {
        TAG_LOGD(
            AAFwkTag::APPKIT, "Number of ability to be notified : [%{public}d]", abilityRecordMgr_->GetRecordCount());
        for (const auto &abilityToken : abilityRecordMgr_->GetAllTokens()) {
            auto abilityRecord = abilityRecordMgr_->GetAbilityItem(abilityToken);
            if (abilityRecord && abilityRecord->GetAbilityThread()) {
                abilityRecord->GetAbilityThread()->NotifyMemoryLevel(level);
            }
        }
    }

    TAG_LOGD(AAFwkTag::APPKIT, "Number of abilityStage to be notified : [%{public}zu]", abilityStages_.size());
    for (auto it = abilityStages_.begin(); it != abilityStages_.end(); it++) {
        auto abilityStage = it->second;
        if (abilityStage) {
            abilityStage->OnMemoryLevel(level);
        }
    }

    TAG_LOGD(AAFwkTag::APPKIT, "called");
    for (auto callback : elementsCallbacks_) {
        if (callback != nullptr) {
            callback->OnMemoryLevel(level);
        }
    }

    abilityRuntimeContext_->DispatchMemoryLevel(level);
}

/**
 *
 * @brief Will be called the application starts
 *
 */
void OHOSApplication::OnStart()
{}

/**
 *
 * @brief Will be called the application ends
 */
void OHOSApplication::OnTerminate()
{}

/**
 *
 * @brief Called when an ability calls Ability#onSaveAbilityState(PacMap).
 * You can implement your own logic in this method.
 * @param outState IIndicates the {@link PacMap} object passed to the onSaveAbilityState() callback.
 *
 */
void OHOSApplication::OnAbilitySaveState(const PacMap &outState)
{
    DispatchAbilitySavedState(outState);
}

void OHOSApplication::SetAppEnv(const std::vector<AppEnvironment>& appEnvironments)
{
    if (!appEnvironments.size()) {
        return;
    }

    for (const auto &appEnvironment : appEnvironments) {
        if (setenv(appEnvironment.name.c_str(), appEnvironment.value.c_str(), APP_ENVIRONMENT_OVERWRITE)) {
            TAG_LOGE(AAFwkTag::APPKIT, "appEnvironment: %{public}s set failed", appEnvironment.name.c_str());
            return;
        }
        TAG_LOGI(AAFwkTag::APPKIT, "appEnvironment set successfully: %{public}s = %{public}s",
            appEnvironment.name.c_str(), appEnvironment.value.c_str());
    }
    return;
}

std::shared_ptr<AbilityRuntime::Context> OHOSApplication::AddAbilityStage(
    const std::shared_ptr<AbilityLocalRecord> &abilityRecord,
    const std::function<void(const std::shared_ptr<AbilityRuntime::Context> &)> &callback, bool &isAsyncCallback)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (abilityRecord == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "abilityRecord is nullptr");
        return nullptr;
    }
    const std::shared_ptr<AbilityInfo> &abilityInfo = abilityRecord->GetAbilityInfo();
    if (abilityInfo == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "abilityInfo is nullptr");
        return nullptr;
    }
    std::string moduleName = abilityInfo->moduleName;
    std::shared_ptr<AbilityRuntime::AbilityStage> abilityStage;
    auto iterator = abilityStages_.find(moduleName);
    if (iterator == abilityStages_.end()) {
        auto stageContext = std::make_shared<AbilityRuntime::AbilityStageContext>();
        stageContext->SetParentContext(abilityRuntimeContext_);
        stageContext->InitHapModuleInfo(abilityInfo);
        stageContext->SetConfiguration(GetConfiguration());
        std::shared_ptr<AppExecFwk::HapModuleInfo> hapModuleInfo = stageContext->GetHapModuleInfo();
        if (hapModuleInfo == nullptr) {
            TAG_LOGE(AAFwkTag::APPKIT, "hapModuleInfo is nullptr");
            return nullptr;
        }
        if (runtime_) {
            runtime_->UpdatePkgContextInfoJson(
                hapModuleInfo->moduleName, hapModuleInfo->hapPath, hapModuleInfo->packageName);
        }
        SetAppEnv(hapModuleInfo->appEnvironments);

        if (abilityInfo->applicationInfo.multiProjects) {
            auto moduleContext = stageContext->CreateModuleContext(hapModuleInfo->moduleName);
            auto rm = moduleContext != nullptr ? moduleContext->GetResourceManager() : nullptr;
            stageContext->SetResourceManager(rm);
        }

        abilityStage = AbilityRuntime::AbilityStage::Create(runtime_, *hapModuleInfo);
        auto application = std::static_pointer_cast<OHOSApplication>(shared_from_this());
        std::weak_ptr<OHOSApplication> weak = application;
        abilityStage->Init(stageContext, weak);

        auto autoStartupCallback = [weak, abilityStage, abilityRecord, moduleName, callback]() {
            auto ohosApplication = weak.lock();
            if (ohosApplication == nullptr) {
                TAG_LOGE(AAFwkTag::APPKIT, "ohosApplication is nullptr");
                return;
            }
            ohosApplication->AutoStartupDone(abilityRecord, abilityStage, moduleName);
            callback(abilityStage->GetContext());
        };
        abilityStage->RunAutoStartupTask(autoStartupCallback, isAsyncCallback, stageContext);
        if (isAsyncCallback) {
            TAG_LOGI(AAFwkTag::APPKIT, "waiting for startup");
            return nullptr;
        }

        Want want;
        if (abilityRecord->GetWant()) {
            TAG_LOGD(AAFwkTag::APPKIT, "want is ok, transport to abilityStage");
            want = *(abilityRecord->GetWant());
        }
        abilityStage->OnCreate(want);
        abilityStages_[moduleName] = abilityStage;
    } else {
        abilityStage = iterator->second;
    }
    const sptr<IRemoteObject> &token = abilityRecord->GetToken();
    if (token == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "token is null");
        return nullptr;
    }
    abilityStage->AddAbility(token, abilityRecord);
    return abilityStage->GetContext();
}

void OHOSApplication::AutoStartupDone(const std::shared_ptr<AbilityLocalRecord> &abilityRecord,
    const std::shared_ptr<AbilityRuntime::AbilityStage> &abilityStage, const std::string &moduleName)
{
    Want want;
    if (abilityRecord->GetWant()) {
        TAG_LOGD(AAFwkTag::APPKIT, "want is ok, transport to abilityStage");
        want = *(abilityRecord->GetWant());
    }
    if (abilityStage == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "abilityStage is nullptr");
        return;
    }
    abilityStage->OnCreate(want);
    abilityStages_[moduleName] = abilityStage;
    const sptr<IRemoteObject> &token = abilityRecord->GetToken();
    if (token == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "token is null");
        return;
    }
    abilityStage->AddAbility(token, abilityRecord);
}

/**
 *
 * @brief update the application info after new module installed.
 *
 * @param appInfo The latest application info obtained from bms for update abilityRuntimeContext.
 *
 */
void OHOSApplication::UpdateApplicationInfoInstalled(const AppExecFwk::ApplicationInfo &appInfo)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");

    if (abilityRuntimeContext_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "abilityRuntimeContext_ is nullptr");
        return;
    }

    abilityRuntimeContext_->SetApplicationInfo(std::make_shared<AppExecFwk::ApplicationInfo>(appInfo));
}

bool OHOSApplication::AddAbilityStage(const AppExecFwk::HapModuleInfo &hapModuleInfo)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    if (abilityRuntimeContext_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "abilityRuntimeContext_ is nullptr");
        return false;
    }

    if (runtime_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "runtime_ is nullptr");
        return false;
    }

    if (abilityStages_.find(hapModuleInfo.moduleName) != abilityStages_.end()) {
        TAG_LOGE(AAFwkTag::APPKIT, "object exist");
        return false;
    }

    auto stageContext = std::make_shared<AbilityRuntime::AbilityStageContext>();
    stageContext->SetParentContext(abilityRuntimeContext_);
    stageContext->InitHapModuleInfo(hapModuleInfo);
    stageContext->SetConfiguration(GetConfiguration());
    auto moduleInfo = stageContext->GetHapModuleInfo();
    if (moduleInfo == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "moduleInfo is nullptr");
        return false;
    }

    if (abilityRuntimeContext_->GetApplicationInfo() && abilityRuntimeContext_->GetApplicationInfo()->multiProjects) {
        auto rm = stageContext->CreateModuleContext(hapModuleInfo.moduleName)->GetResourceManager();
        stageContext->SetResourceManager(rm);
    }

    auto abilityStage = AbilityRuntime::AbilityStage::Create(runtime_, *moduleInfo);
    auto application = std::static_pointer_cast<OHOSApplication>(shared_from_this());
    std::weak_ptr<OHOSApplication> weak = application;
    abilityStage->Init(stageContext, weak);
    Want want;
    abilityStage->OnCreate(want);
    abilityStages_[hapModuleInfo.moduleName] = abilityStage;
    TAG_LOGE(AAFwkTag::APPKIT, "abilityStage insert and initialization");
    return true;
}

void OHOSApplication::CleanAbilityStage(const sptr<IRemoteObject> &token,
    const std::shared_ptr<AbilityInfo> &abilityInfo, bool isCacheProcess)
{
    if (abilityInfo == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "abilityInfo is nullptr");
        return;
    }
    if (token == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "token is nullptr");
        return;
    }
    std::string moduleName = abilityInfo->moduleName;
    auto iterator = abilityStages_.find(moduleName);
    if (iterator != abilityStages_.end()) {
        auto abilityStage = iterator->second;
        abilityStage->RemoveAbility(token);
        if (!abilityStage->ContainsAbility() && !isCacheProcess) {
            abilityStage->OnDestroy();
            abilityStages_.erase(moduleName);
        }
        DoCleanWorkAfterStageCleaned(*abilityInfo);
    }
}

void OHOSApplication::DoCleanWorkAfterStageCleaned(const AbilityInfo &abilityInfo)
{
    TAG_LOGD(AAFwkTag::APPKIT, "language: %{public}s", abilityInfo.srcLanguage.c_str());
    if (runtime_) {
        runtime_->DoCleanWorkAfterStageCleaned();
    }
}

std::shared_ptr<AbilityRuntime::Context> OHOSApplication::GetAppContext() const
{
    return abilityRuntimeContext_;
}

const std::unique_ptr<AbilityRuntime::Runtime>& OHOSApplication::GetRuntime()
{
    return runtime_;
}

void OHOSApplication::SetConfiguration(const Configuration &config)
{
    if (!configuration_) {
        configuration_ = std::make_shared<Configuration>(config);
    }
}

void OHOSApplication::ScheduleAcceptWant(const AAFwk::Want &want, const std::string &moduleName, std::string &flag)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    auto iter = abilityStages_.find(moduleName);
    if (iter != abilityStages_.end()) {
        auto abilityStage = iter->second;
        if (abilityStage) {
            flag = abilityStage->OnAcceptWant(want);
        }
    }
}

void OHOSApplication::ScheduleNewProcessRequest(const AAFwk::Want &want, const std::string &moduleName,
    std::string &flag)
{
    TAG_LOGD(AAFwkTag::APPKIT, "call");
    if (abilityStages_.empty()) {
        TAG_LOGE(AAFwkTag::APPKIT, "abilityStages_ is empty");
        return;
    }
    auto iter = abilityStages_.find(moduleName);
    if (iter == abilityStages_.end()) {
        TAG_LOGE(AAFwkTag::APPKIT, "%{public}s is not in abilityStage", moduleName.c_str());
        return;
    }
    auto abilityStage = iter->second;
    if (abilityStage) {
        flag = abilityStage->OnNewProcessRequest(want);
    }
}

std::shared_ptr<Configuration> OHOSApplication::GetConfiguration()
{
    return configuration_;
}

void OHOSApplication::SetExtensionTypeMap(std::map<int32_t, std::string> map)
{
    extensionTypeMap_ = map;
}

bool OHOSApplication::NotifyLoadRepairPatch(const std::string &hqfFile, const std::string &hapPath)
{
    if (runtime_ == nullptr) {
        TAG_LOGD(AAFwkTag::APPKIT, "runtime is nullptr");
        return true;
    }

    return runtime_->LoadRepairPatch(hqfFile, hapPath);
}

bool OHOSApplication::NotifyHotReloadPage()
{
    if (runtime_ == nullptr) {
        TAG_LOGD(AAFwkTag::APPKIT, "runtime is nullptr");
        return true;
    }

    return runtime_->NotifyHotReloadPage();
}

bool OHOSApplication::NotifyUnLoadRepairPatch(const std::string &hqfFile)
{
    if (runtime_ == nullptr) {
        TAG_LOGD(AAFwkTag::APPKIT, "runtime is nullptr");
        return true;
    }

    return runtime_->UnLoadRepairPatch(hqfFile);
}

void OHOSApplication::CleanAppTempData(bool isLastProcess)
{
    if (!isLastProcess) {
        TAG_LOGE(AAFwkTag::APPKIT, "failed");
        return;
    }
    if (abilityRuntimeContext_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "Context is nullptr");
        return;
    }

    auto cleaner = ApplicationCleaner::GetInstance();
    if (cleaner) {
        cleaner->SetRuntimeContext(abilityRuntimeContext_);
        cleaner->RenameTempData();
    }
}

void OHOSApplication::CleanUselessTempData()
{
    if (abilityRuntimeContext_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "Context is nullptr");
        return;
    }

    auto cleaner = ApplicationCleaner::GetInstance();
    if (cleaner) {
        cleaner->SetRuntimeContext(abilityRuntimeContext_);
        cleaner->ClearTempData();
    }
}

void OHOSApplication::UpdateAppContextResMgr(const Configuration &config)
{
    auto context = GetAppContext();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "Application context is nullptr");
        return;
    }

    auto configUtils = std::make_shared<AbilityRuntime::ConfigurationUtils>();
    configUtils->UpdateGlobalConfig(config, context->GetResourceManager());
}

void OHOSApplication::CleanEmptyAbilityStage()
{
    bool containsNonEmpty = false;
    for (auto it = abilityStages_.begin(); it != abilityStages_.end();) {
        auto abilityStage = it->second;
        if (abilityStage == nullptr) {
            it++;
            continue;
        }
        if (!abilityStage->ContainsAbility()) {
            abilityStage->OnDestroy();
            it = abilityStages_.erase(it);
        } else {
            containsNonEmpty = true;
            it++;
        }
    }
    if (containsNonEmpty) {
        TAG_LOGI(AAFwkTag::APPKIT, "Application contains none empty abilityStage");
    }
}

bool OHOSApplication::isUpdateColor(Configuration &config, std::string colorMode, std::string globalColorMode,
    std::string globalColorModeIsSetBySa, std::string colorModeIsSetByApp, std::string colorModeIsSetBySa)
{
    bool isUpdateColor = true;
    if (colorMode.compare(ConfigurationInner::COLOR_MODE_AUTO) == 0 && globalColorModeIsSetBySa.empty()) {
        TAG_LOGD(AAFwkTag::APPKIT, "colorMode is auto");
        constexpr int buffSize = 64;
        char valueGet[buffSize] = { 0 };
        auto res = GetParameter(PERSIST_DARKMODE_KEY, colorMode.c_str(), valueGet, buffSize);
        if (res <= 0) {
            TAG_LOGE(AAFwkTag::APPKIT, "get parameter failed.");
            return false;
        }
        config.AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_COLORMODE, valueGet);
    }
    std::string globalColorModeIsSetByApp =
        configuration_->GetItem(AAFwk::GlobalConfigurationKey::COLORMODE_IS_SET_BY_APP);
    if (!colorMode.empty() && colorModeIsSetByApp.empty() && colorModeIsSetBySa.empty()) {
        if ((!globalColorModeIsSetByApp.empty() && globalColorMode.compare(ConfigurationInner::COLOR_MODE_AUTO) != 0) ||
            !globalColorModeIsSetBySa.empty()) {
            TAG_LOGD(AAFwkTag::APPKIT, "colormode has been set by app or sa");
            config.RemoveItem(AAFwk::GlobalConfigurationKey::SYSTEM_COLORMODE);
            isUpdateColor = false;
        }
    }
    if (!colorModeIsSetBySa.empty() && colorModeIsSetByApp.empty()) {
        if (!globalColorModeIsSetByApp.empty() && globalColorMode.compare(ConfigurationInner::COLOR_MODE_AUTO) != 0) {
            config.RemoveItem(AAFwk::GlobalConfigurationKey::SYSTEM_COLORMODE);
            isUpdateColor = false;
        }
    }
    return isUpdateColor;
}

bool OHOSApplication::isUpdateFontSize(Configuration &config)
{
    std::string fontSizeScal = config.GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_FONT_SIZE_SCALE);
    std::string globalFontFollowSysteme = configuration_->GetItem(AAFwk::GlobalConfigurationKey::APP_FONT_SIZE_SCALE);
    if (!globalFontFollowSysteme.empty()
        && globalFontFollowSysteme.compare(ConfigurationInner::IS_APP_FONT_FOLLOW_SYSTEM) == 0) {
        return true;
    }
    TAG_LOGW(AAFwkTag::APPKIT, "the font size configured for the app does not take effect with the system");
    config.RemoveItem(AAFwk::GlobalConfigurationKey::SYSTEM_FONT_SIZE_SCALE);
    configuration_->AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_FONT_SIZE_SCALE,
        ConfigurationInner::SYSTEM_DEFAULT_FONTSIZE_SCALE);
    return false;
}

bool OHOSApplication::isUpdateLanguage(Configuration &config, const std::string language,
    const std::string languageIsSetByApp, const std::string globalLanguageIsSetByApp)
{
    if (!language.empty() && languageIsSetByApp.empty() && !globalLanguageIsSetByApp.empty()) {
        TAG_LOGD(AAFwkTag::APPKIT, "language has been set by app");
        config.RemoveItem(AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE);
        return false;
    }
    if (language.empty()) {
        TAG_LOGD(AAFwkTag::APPKIT, "language is empty, need not update");
        return false;
    }
    return true;
}
}  // namespace AppExecFwk
}  // namespace OHOS
