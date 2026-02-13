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

#include <cstdio>
#include <cstring>
#include <fcntl.h>

#include <sys/stat.h>

#include "ohos_application.h"

#include "ability.h"
#include "ability_record_mgr.h"
#include "ability_thread.h"
#include "app_loader.h"
#include "application_context.h"
#include "application_cleaner.h"
#include "application_impl.h"
#include "bundle_mgr_helper.h"
#include "configuration_convertor.h"
#include "configuration_utils.h"
#include "context_impl.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "iservice_registry.h"
#include "runtime.h"
#include "js_runtime.h"
#include "ets_runtime.h"
#include "startup_manager.h"
#include "system_ability_definition.h"
#include "syspara/parameter.h"
#include "ui_ability.h"
#include "application_configuration_manager.h"
#ifdef SUPPORT_GRAPHICS
#include "display_manager.h"
#include "window.h"
#endif

namespace OHOS {
namespace AppExecFwk {
namespace {
    constexpr const char* PERSIST_DARKMODE_KEY = "persist.ace.darkmode";
}
void OHOSApplication::LoadDeduplicatedHarResource(
    const std::shared_ptr<AbilityRuntime::AbilityStageContext> &stageContext)
{
    if (!stageContext) {
        TAG_LOGE(AAFwkTag::APPKIT, "null argv");
        return;
    }
    auto resourceManager = stageContext->GetResourceManager();
    if (!resourceManager) {
        TAG_LOGE(AAFwkTag::APPKIT, "null resourceManager");
        return;
    }
    auto hasDeduplicateHar = GetDeduplicateHar();
    auto loadPath = GetEntryLoadPath();
    if (!loadPath.empty() && hasDeduplicateHar) {
        if (!resourceManager->AddResource(loadPath.c_str())) {
            TAG_LOGE(AAFwkTag::APPKIT, "AddResource failed");
        }
    }
}
REGISTER_APPLICATION(OHOSApplication, OHOSApplication)
constexpr int32_t APP_ENVIRONMENT_OVERWRITE = 1;
using ApplicationConfigurationManager = AbilityRuntime::ApplicationConfigurationManager;
OHOSApplication::OHOSApplication()
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
}

OHOSApplication::~OHOSApplication()
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
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
void OHOSApplication::SetRuntime(std::unique_ptr<AbilityRuntime::Runtime> &&runtime)
{
    TAG_LOGD(AAFwkTag::APPKIT, "begin");
    if (runtime == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null runtime");
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
        TAG_LOGE(AAFwkTag::APPKIT, "null context");
        return;
    }
    abilityRuntimeContext_ = abilityRuntimeContext;
    auto application = std::static_pointer_cast<OHOSApplication>(shared_from_this());
    std::weak_ptr<OHOSApplication> applicationWptr = application;
    abilityRuntimeContext_->RegisterAppConfigUpdateObserver([applicationWptr](const Configuration &config) {
        std::shared_ptr<OHOSApplication> applicationSptr = applicationWptr.lock();
        if (applicationSptr == nullptr) {
            TAG_LOGE(AAFwkTag::APPKIT, "null applicationSptr");
            return;
        }
        applicationSptr->OnConfigurationUpdated(config, AbilityRuntime::SetLevel::Application);
    });
    abilityRuntimeContext_->RegisterAppFontObserver([applicationWptr](const Configuration &config) {
        std::shared_ptr<OHOSApplication> applicationSptr = applicationWptr.lock();
        if (applicationSptr == nullptr) {
            TAG_LOGE(AAFwkTag::APPKIT, "null applicationSptr");
            return;
        }
        applicationSptr->OnUpdateConfigurationForAll(config);
    });
#ifdef SUPPORT_GRAPHICS
    abilityRuntimeContext_->RegisterGetDisplayConfig([applicationWptr](uint64_t displayId,
        float &density, std::string &directionStr) -> bool {
        std::shared_ptr<OHOSApplication> applicationSptr = applicationWptr.lock();
        if (applicationSptr == nullptr) {
            TAG_LOGE(AAFwkTag::APPKIT, "null applicationSptr");
            return false;
        }
        return applicationSptr->GetDisplayConfig(displayId, density, directionStr);
    });
#endif
    abilityRuntimeContext_->RegisterAppGetSpecifiedRuntime(
        [applicationWptr](const std::string &codeLanguage)-> const std::unique_ptr<AbilityRuntime::Runtime>& {
            std::shared_ptr<OHOSApplication> applicationSptr = applicationWptr.lock();
            if (applicationSptr == nullptr) {
                TAG_LOGE(AAFwkTag::APPKIT, "null applicationSptr");
                static std::unique_ptr<AbilityRuntime::Runtime> runtime = nullptr;
                return runtime;
            }
            return applicationSptr->GetSpecifiedRuntime(codeLanguage);
        });
#ifdef SUPPORT_SCREEN
    RegisterGetAllUIAbilitiesCallback(abilityRuntimeContext_, applicationWptr);
#endif
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
        TAG_LOGE(AAFwkTag::APPKIT, "null abilityRecordMgr");
        return;
    }
    abilityRecordMgr_ = abilityRecordMgr;
}

/**
 *
 * @brief Will be Called when the system configuration of the device changes.
 *
 * @param config Indicates the new Configuration object.
 * @param reason Indicates the Configuration update scene flag.
 */
void OHOSApplication::OnConfigurationUpdated(
    Configuration config, AbilityRuntime::SetLevel level, ConfigUpdateReason reason)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (!abilityRecordMgr_ || !configuration_ || !abilityRuntimeContext_) {
        TAG_LOGD(AAFwkTag::APPKIT, "abilityRecordMgr_ or configuration_ or abilityRuntimeContext_ is null");
        return;
    }

    if (level == AbilityRuntime::SetLevel::System) {
        abilityRuntimeContext_->NotifySystemConfigurationUpdated(config);
    }
    bool isUpdateAppColor = IsUpdateColorNeeded(config, level);
    bool isUpdateAppFontSize = isUpdateFontSize(config, level);
    bool isUpdateAppLanguage = IsUpdateLanguageNeeded(config, level);
    bool isUpdateAppLocale = IsUpdateLocaleNeeded(*configuration_, config);
    if (!isUpdateAppColor && !isUpdateAppFontSize && !isUpdateAppLanguage && !isUpdateAppLocale &&
        config.GetItemSize() == 0) {
        TAG_LOGD(AAFwkTag::APPKIT, "configuration need not updated");
        return;
    }
    std::vector<std::string> changeKeyV;
    {
        HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, "configuration_->CompareDifferent");
        configuration_->CompareDifferent(changeKeyV, config);
        configuration_->Merge(changeKeyV, config);
    }
    TAG_LOGI(AAFwkTag::APPKIT, "configuration_: %{public}s, config: %{public}s",
        configuration_->GetName().c_str(), config.GetName().c_str());
    // Update resConfig of resource manager, which belongs to application context.
    UpdateAppContextResMgr(config);
    #ifdef SUPPORT_GRAPHICS
        auto diffSyncConfiguration = std::make_shared<AppExecFwk::Configuration>(config);
        Rosen::Window::UpdateConfigurationSyncForAll(diffSyncConfiguration);
    #endif
    // Notify all abilities
    for (const auto &abilityToken : abilityRecordMgr_->GetAllTokens()) {
        auto abilityRecord = abilityRecordMgr_->GetAbilityItem(abilityToken);
        if (abilityRecord && abilityRecord->GetAbilityThread()) {
            abilityRecord->GetAbilityThread()->ScheduleUpdateConfiguration(config);
        }
    }
    for (auto it = abilityStages_.begin(); it != abilityStages_.end(); it++) {
        auto abilityStage = it->second;
        if (abilityStage) {
            abilityStage->OnConfigurationUpdated(config);
        }
    }
    abilityRuntimeContext_->SetConfigUpdateReason(reason);
#ifdef SUPPORT_GRAPHICS
    auto diffConfiguration = std::make_shared<AppExecFwk::Configuration>(config);
    auto ignoreWindowContext = AbilityRuntime::ApplicationConfigurationManager::GetInstance().GetIgnoreContext();
    TAG_LOGI(AAFwkTag::APPKIT, "ignoreWindowContext size %{public}zu", ignoreWindowContext.size());
    Rosen::Window::UpdateConfigurationForAll(diffConfiguration, ignoreWindowContext);
#endif
    abilityRuntimeContext_->DispatchConfigurationUpdated(*configuration_);
    abilityRuntimeContext_->SetConfiguration(configuration_);
}

/**
 *
 * @brief Will be Called when the application font of the device changes.
 *
 * @param config Indicates the new Configuration object.
 */
void OHOSApplication::OnUpdateConfigurationForAll(Configuration config)
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
void OHOSApplication::OnMemoryLevel(int32_t level)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    if (abilityRuntimeContext_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null abilityRuntimeContext_");
        return;
    }
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

void OHOSApplication::SetAppEnv(const std::vector<AppEnvironment>& appEnvironments)
{
    if (appEnvironments.empty()) {
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

void OHOSApplication::PreloadHybridModule(const HapModuleInfo &hapModuleInfo) const
{
    if (hapModuleInfo.moduleArkTSMode  != Constants::ARKTS_MODE_HYBRID) {
        TAG_LOGD(AAFwkTag::APPKIT, "not hybrid runtime");
        return;
    }

    bool isEsmode = hapModuleInfo.compileMode == CompileMode::ES_MODULE;
    bool useCommonTrunk = false;
    for (const auto& md : hapModuleInfo.metadata) {
        if (md.name == "USE_COMMON_CHUNK") {
            useCommonTrunk = md.value == "true";
            break;
        }
    }
    runtime_->PreloadModule(hapModuleInfo.moduleName, hapModuleInfo.hapPath, isEsmode, useCommonTrunk);
}

std::shared_ptr<AbilityRuntime::Context> OHOSApplication::AddAbilityStage(
    const std::shared_ptr<AbilityLocalRecord> &abilityRecord,
    const std::function<void(const std::shared_ptr<AbilityRuntime::Context> &)> &callback, bool &isAsyncCallback)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (abilityRecord == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null abilityRecord");
        return nullptr;
    }
    const std::shared_ptr<AbilityInfo> &abilityInfo = abilityRecord->GetAbilityInfo();
    if (abilityInfo == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null abilityInfo");
        return nullptr;
    }
    std::string moduleName = abilityInfo->moduleName;
    std::shared_ptr<AbilityRuntime::AbilityStage> abilityStage;
    auto iterator = abilityStages_.find(moduleName);
    if (iterator == abilityStages_.end()) {
        auto stageContext = std::make_shared<AbilityRuntime::AbilityStageContext>();
        bool isPlugin = abilityInfo->applicationInfo.bundleType == AppExecFwk::BundleType::APP_PLUGIN;
        if (isPlugin) {
            stageContext->SetIsPlugin(true);
            stageContext->InitPluginHapModuleInfo(abilityInfo, abilityRuntimeContext_->GetBundleName());
            auto pluginContext = stageContext->CreatePluginContext(
                abilityInfo->bundleName, abilityInfo->moduleName, abilityRuntimeContext_);
            if (pluginContext == nullptr) {
                TAG_LOGE(AAFwkTag::APPKIT, "null pluginContext");
                return nullptr;
            }
            auto rm = pluginContext->GetResourceManager();
            stageContext->SetResourceManager(rm);
            stageContext->SetParentContext(pluginContext);
        } else {
            stageContext->InitHapModuleInfo(abilityInfo);
            stageContext->SetParentContext(abilityRuntimeContext_);
            LoadDeduplicatedHarResource(stageContext);
        }

        stageContext->SetConfiguration(GetConfiguration());
        stageContext->SetProcessName(GetProcessName());
        std::shared_ptr<AppExecFwk::HapModuleInfo> hapModuleInfo = stageContext->GetHapModuleInfo();
        if (hapModuleInfo == nullptr) {
            TAG_LOGE(AAFwkTag::APPKIT, "null hapModuleInfo");
            return nullptr;
        }
        PreloadHybridModule(*hapModuleInfo);
        if (runtime_ && (runtime_->GetLanguage() == AbilityRuntime::Runtime::Language::JS)) {
            static_cast<AbilityRuntime::JsRuntime&>(*runtime_).SetPkgContextInfoJson(
                hapModuleInfo->moduleName, hapModuleInfo->hapPath, hapModuleInfo->packageName);
        }
        SetAppEnv(hapModuleInfo->appEnvironments);

        if (abilityInfo->applicationInfo.multiProjects) {
            auto moduleContext = stageContext->CreateModuleContext(hapModuleInfo->moduleName);
            auto rm = moduleContext != nullptr ? moduleContext->GetResourceManager() : nullptr;
            stageContext->SetResourceManager(rm);
        }

        auto &runtimeStage = GetSpecifiedRuntime(hapModuleInfo->arkTSMode);
        abilityStage = AbilityRuntime::AbilityStage::Create(runtimeStage, *hapModuleInfo);
        if (abilityStage == nullptr) {
            TAG_LOGE(AAFwkTag::APPKIT, "null abilityStage");
            return nullptr;
        }
        auto application = std::static_pointer_cast<OHOSApplication>(shared_from_this());
        std::weak_ptr<OHOSApplication> weak = application;
        abilityStage->Init(stageContext, weak);
        auto firstCallback = CreateFirstStartupCallbackForRecord(abilityStage, abilityRecord, *hapModuleInfo,
            callback);
        if (firstCallback != nullptr) {
            abilityStage->RunAutoStartupTask(firstCallback, abilityRecord->GetWant(), isAsyncCallback,
                stageContext, true);
            if (isAsyncCallback) {
                TAG_LOGI(AAFwkTag::APPKIT, "wait startup first");
                return nullptr;
            }
        }

        TAG_LOGD(AAFwkTag::APPKIT, "no wait startup first");
        abilityStage->LoadModule(*hapModuleInfo);
        auto secondCallback = CreateSecondStartupCallbackForRecord(abilityStage, abilityRecord, callback);
        if (firstCallback != nullptr) {
            abilityStage->RunAutoStartupTask(secondCallback, abilityRecord->GetWant(), isAsyncCallback,
                abilityStage->GetContext(), false);
            if (isAsyncCallback) {
                TAG_LOGI(AAFwkTag::APPKIT, "wait startup second");
                return nullptr;
            }
        }

        TAG_LOGD(AAFwkTag::APPKIT, "no wait startup second");
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
        TAG_LOGE(AAFwkTag::APPKIT, "null token");
        return nullptr;
    }
    abilityStage->AddAbility(token, abilityRecord);
    return abilityStage->GetContext();
}

const std::function<void()> OHOSApplication::CreateFirstStartupCallbackForRecord(
    std::shared_ptr<AbilityRuntime::AbilityStage> abilityStage,
    std::shared_ptr<AbilityLocalRecord> abilityRecord,
    const AppExecFwk::HapModuleInfo &hapModuleInfo,
    const std::function<void(std::shared_ptr<AbilityRuntime::Context>)> &callback)
{
    if (abilityRecord == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null abilityRecord");
        return nullptr;
    }
    std::shared_ptr<AbilityInfo> abilityInfo = abilityRecord->GetAbilityInfo();
    if (abilityInfo == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null abilityInfo");
        return nullptr;
    }
    if (!IsMainProcess(abilityInfo->bundleName, abilityInfo->applicationInfo.process)) {
        return nullptr;
    }
    auto application = std::static_pointer_cast<OHOSApplication>(shared_from_this());
    std::weak_ptr<OHOSApplication> weak = application;
    
    return [weak, abilityStage, abilityRecord, hapModuleInfo, callback]() {
        TAG_LOGD(AAFwkTag::APPKIT, "Enter FirstStartupCallbackForRecord");
        auto ohosApplication = weak.lock();
        if (ohosApplication == nullptr) {
            TAG_LOGE(AAFwkTag::APPKIT, "ohosApplication null");
            return;
        }
        if (abilityStage == nullptr) {
            TAG_LOGE(AAFwkTag::APPKIT, "abilityStage null");
            return;
        }

        abilityStage->LoadModule(hapModuleInfo);
        auto secondCallback = ohosApplication->CreateSecondStartupCallbackForRecord(
            abilityStage, abilityRecord, callback);
        bool isAsyncCallback = false;
        abilityStage->RunAutoStartupTask(secondCallback, abilityRecord->GetWant(), isAsyncCallback,
            abilityStage->GetContext(), false);
        if (isAsyncCallback) {
            TAG_LOGI(AAFwkTag::APPKIT, "wait startup second");
            return;
        }
        secondCallback();
    };
}

const std::function<void()> OHOSApplication::CreateSecondStartupCallbackForRecord(
    std::shared_ptr<AbilityRuntime::AbilityStage> abilityStage,
    std::shared_ptr<AbilityLocalRecord> abilityRecord,
    const std::function<void(std::shared_ptr<AbilityRuntime::Context>)> &callback)
{
    auto application = std::static_pointer_cast<OHOSApplication>(shared_from_this());
    std::weak_ptr<OHOSApplication> weak = application;

    auto autoStartupCallback = [weak, abilityStage, abilityRecord, callback]() {
        TAG_LOGD(AAFwkTag::APPKIT, "Enter SecondStartupCallbackForRecord");
        auto ohosApplication = weak.lock();
        if (ohosApplication == nullptr) {
            TAG_LOGE(AAFwkTag::APPKIT, "null ohosApplication");
            return;
        }
        if (abilityRecord == nullptr) {
            TAG_LOGE(AAFwkTag::APPKIT, "null abilityRecord");
            return;
        }
        auto abilityInfo = abilityRecord->GetAbilityInfo();
        if (abilityInfo == nullptr) {
            TAG_LOGE(AAFwkTag::APPKIT, "null abilityInfo");
            return;
        }

        std::string moduleName = abilityInfo->moduleName;
        ohosApplication->AutoStartupDone(abilityRecord, abilityStage, moduleName);
        if (callback == nullptr) {
            TAG_LOGE(AAFwkTag::APPKIT, "null callback");
            return;
        }
        callback(abilityStage->GetContext());
    };
    return autoStartupCallback;
}

const std::function<void()> OHOSApplication::CreateFirstStartupCallbackForHap(
    std::shared_ptr<AbilityRuntime::AbilityStage> abilityStage,
    const AppExecFwk::HapModuleInfo &hapModuleInfo,
    const std::function<void()> &callback)
{
    if (abilityRuntimeContext_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null abilityRuntimeContext_");
        return nullptr;
    }
    auto applicationInfo = abilityRuntimeContext_->GetApplicationInfo();
    if (applicationInfo == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null applicationInfo");
        return nullptr;
    }
    if (!IsMainProcess(hapModuleInfo.bundleName, applicationInfo->process)) {
        return nullptr;
    }
    auto application = std::static_pointer_cast<OHOSApplication>(shared_from_this());
    std::weak_ptr<OHOSApplication> weak = application;

    auto autoStartupCallback = [weak, abilityStage, hapModuleInfo, callback]() {
        TAG_LOGD(AAFwkTag::APPKIT, "Enter FirstStartupCallbackForHap");
        auto ohosApplication = weak.lock();
        if (ohosApplication == nullptr) {
            TAG_LOGE(AAFwkTag::APPKIT, "null ohosApplication");
            return;
        }
        if (abilityStage == nullptr) {
            TAG_LOGE(AAFwkTag::APPKIT, "abilityStage null");
            return;
        }

        abilityStage->LoadModule(hapModuleInfo);
        auto secondCallback = ohosApplication->CreateSecondStartupCallbackForHap(abilityStage, hapModuleInfo, callback);
        bool isAsyncCallback = false;
        abilityStage->RunAutoStartupTask(secondCallback, nullptr, isAsyncCallback, abilityStage->GetContext(), false);
        if (isAsyncCallback) {
            TAG_LOGI(AAFwkTag::APPKIT, "wait startup second");
            return;
        }
        secondCallback();
    };

    return autoStartupCallback;
}

const std::function<void()> OHOSApplication::CreateSecondStartupCallbackForHap(
    std::shared_ptr<AbilityRuntime::AbilityStage> abilityStage,
    const AppExecFwk::HapModuleInfo &hapModuleInfo,
    const std::function<void()> &callback)
{
    auto application = std::static_pointer_cast<OHOSApplication>(shared_from_this());
    std::weak_ptr<OHOSApplication> weak = application;

    auto autoStartupCallback = [weak, abilityStage, hapModuleInfo, callback]() {
        TAG_LOGD(AAFwkTag::APPKIT, "Enter SecondStartupCallbackForHap");
        auto ohosApplication = weak.lock();
        if (ohosApplication == nullptr) {
            TAG_LOGE(AAFwkTag::APPKIT, "null ohosApplication");
            return;
        }
        ohosApplication->AutoStartupDone(abilityStage, hapModuleInfo);
        if (callback == nullptr) {
            TAG_LOGE(AAFwkTag::APPKIT, "null callback");
            return;
        }
        callback();
    };

    return autoStartupCallback;
}

void OHOSApplication::AutoStartupDone(const std::shared_ptr<AbilityLocalRecord> &abilityRecord,
    const std::shared_ptr<AbilityRuntime::AbilityStage> &abilityStage, const std::string &moduleName)
{
    if (abilityStage == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null abilityStage");
        return;
    }

    Want want;
    if (abilityRecord->GetWant()) {
        TAG_LOGD(AAFwkTag::APPKIT, "want is ok, transport to abilityStage");
        want = *(abilityRecord->GetWant());
    }

    abilityStage->OnCreate(want);
    abilityStages_[moduleName] = abilityStage;
    const sptr<IRemoteObject> &token = abilityRecord->GetToken();
    if (token == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null token");
        return;
    }
    abilityStage->AddAbility(token, abilityRecord);
}

void OHOSApplication::AutoStartupDone(
    const std::shared_ptr<AbilityRuntime::AbilityStage> &abilityStage,
    const AppExecFwk::HapModuleInfo &hapModuleInfo)
{
    if (abilityStage == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null abilityStage");
        return;
    }

    Want want;
    abilityStage->OnCreate(want);
    abilityStages_[hapModuleInfo.moduleName] = abilityStage;
    TAG_LOGI(AAFwkTag::APPKIT, "abilityStage insert and initialization");
    return;
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
        TAG_LOGE(AAFwkTag::APPKIT, "null abilityRuntimeContext_");
        return;
    }

    abilityRuntimeContext_->SetApplicationInfo(std::make_shared<AppExecFwk::ApplicationInfo>(appInfo));
}

bool OHOSApplication::AddAbilityStage(
    const AppExecFwk::HapModuleInfo &hapModuleInfo,
    const std::function<void()> &callback, bool &isAsyncCallback)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    if (abilityRuntimeContext_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null abilityRuntimeContext_");
        return false;
    }

    if (runtime_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null runtime_");
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
        TAG_LOGE(AAFwkTag::APPKIT, "null moduleInfo");
        return false;
    }

    if (abilityRuntimeContext_->GetApplicationInfo() && abilityRuntimeContext_->GetApplicationInfo()->multiProjects) {
        auto rm = stageContext->CreateModuleContext(hapModuleInfo.moduleName)->GetResourceManager();
        stageContext->SetResourceManager(rm);
    }
    LoadDeduplicatedHarResource(stageContext);
    auto &runtime = GetSpecifiedRuntime(moduleInfo->arkTSMode);
    auto abilityStage = AbilityRuntime::AbilityStage::Create(runtime, *moduleInfo);
    if (abilityStage == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null abilityStage");
        return false;
    }
    auto application = std::static_pointer_cast<OHOSApplication>(shared_from_this());
    std::weak_ptr<OHOSApplication> weak = application;
    abilityStage->Init(stageContext, weak);
    auto firstCallback = CreateFirstStartupCallbackForHap(abilityStage, hapModuleInfo, callback);
    if (firstCallback != nullptr) {
        abilityStage->RunAutoStartupTask(firstCallback, nullptr, isAsyncCallback, stageContext, true);
        if (isAsyncCallback) {
            TAG_LOGI(AAFwkTag::APPKIT, "wait startup first");
            return false;
        }
    }
    TAG_LOGD(AAFwkTag::APPKIT, "no wait startup first");
    abilityStage->LoadModule(hapModuleInfo);
    auto secondCallback = CreateSecondStartupCallbackForHap(abilityStage, hapModuleInfo, callback);
    if (firstCallback != nullptr) {
        abilityStage->RunAutoStartupTask(secondCallback, nullptr, isAsyncCallback, stageContext, false);
        if (isAsyncCallback) {
            TAG_LOGI(AAFwkTag::APPKIT, "wait startup second");
            return false;
        }
    }

    TAG_LOGD(AAFwkTag::APPKIT, "no wait startup second");
    Want want;
    abilityStage->OnCreate(want);
    abilityStages_[hapModuleInfo.moduleName] = abilityStage;
    TAG_LOGI(AAFwkTag::APPKIT, "abilityStage insert and initialization");
    return true;
}

void OHOSApplication::CleanAbilityStage(const sptr<IRemoteObject> &token,
    const std::shared_ptr<AbilityInfo> &abilityInfo, bool isCacheProcess)
{
    if (abilityInfo == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null abilityInfo");
        return;
    }
    if (token == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null token");
        return;
    }
    std::string moduleName = abilityInfo->moduleName;
    auto iterator = abilityStages_.find(moduleName);
    if (iterator != abilityStages_.end()) {
        auto abilityStage = iterator->second;
        if (abilityStage == nullptr) {
            TAG_LOGE(AAFwkTag::APPKIT, "null abilityStage");
            return;
        }
        abilityStage->RemoveAbility(token);
        if (!abilityStage->ContainsAbility() && !isCacheProcess) {
            abilityStage->OnDestroy();
            abilityStages_.erase(moduleName);
        }
    }
}

std::shared_ptr<AbilityRuntime::Context> OHOSApplication::GetAppContext() const
{
    return abilityRuntimeContext_;
}

const std::unique_ptr<AbilityRuntime::Runtime> &OHOSApplication::GetSpecifiedRuntime(
    const std::string &arkTSMode) const
{
    std::lock_guard<std::mutex> lock(runtimeMutex_);
    if (arkTSMode == AbilityRuntime::CODE_LANGUAGE_ARKTS_1_0 &&
        runtime_ != nullptr &&
        runtime_->GetLanguage() == AbilityRuntime::Runtime::Language::ETS) {
        return (static_cast<AbilityRuntime::ETSRuntime&>(*runtime_)).GetJsRuntime();
    }
    return runtime_;
}

void OHOSApplication::SetConfiguration(const Configuration &config)
{
    if (!configuration_) {
        configuration_ = std::make_shared<Configuration>(config);
    }
    auto colorMode = config.GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_COLORMODE);
    AbilityRuntime::ApplicationConfigurationManager::GetInstance().
        SetColorModeSetLevel(AbilityRuntime::SetLevel::System, colorMode);

    if (abilityRuntimeContext_ && configuration_) {
        abilityRuntimeContext_->SetConfiguration(configuration_);
    }
}

void OHOSApplication::ScheduleAcceptWant(const AAFwk::Want &want, const std::string &moduleName,
    std::function<void(std::string)> callback, bool &isAsync)
{
    TAG_LOGD(AAFwkTag::APPKIT, "ScheduleAcceptWant called");
    if (callback == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null callback");
        return;
    }
    auto iter = abilityStages_.find(moduleName);
    if (iter == abilityStages_.end() || iter->second == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "%{public}s is not in abilityStage", moduleName.c_str());
        std::string flag;
        callback(flag);
        return;
    }

    auto *callbackInfo = AppExecFwk::AbilityTransactionCallbackInfo<std::string>::Create();
    if (callbackInfo == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null callbackInfo");
        std::string flag;
        callback(flag);
        return;
    }
    callbackInfo->Push(callback);
    if (iter->second->OnAcceptWant(want, callbackInfo, isAsync).empty()) {
        TAG_LOGE(AAFwkTag::APPKIT, "not exist");
        std::string result;
        callbackInfo->Call(result);
        AppExecFwk::AbilityTransactionCallbackInfo<std::string>::Destroy(callbackInfo);
    }
}

void OHOSApplication::SchedulePrepareTerminate(const std::string &moduleName,
    std::function<void(AppExecFwk::OnPrepareTerminationResult)> callback, bool &isAsync)
{
    isAsync = false;
    if (callback == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null callback");
        return;
    }
    auto iter = abilityStages_.find(moduleName);
    if (iter == abilityStages_.end() || iter->second == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "%{public}s is not in abilityStage", moduleName.c_str());
        return;
    }

    auto *callbackInfo = AppExecFwk::AbilityTransactionCallbackInfo<AppExecFwk::OnPrepareTerminationResult>::Create();
    if (callbackInfo == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null callbackInfo");
        return;
    }
    callbackInfo->Push(callback);
    if (!iter->second->OnPrepareTerminate(callbackInfo, isAsync)) {
        TAG_LOGI(AAFwkTag::APPKIT, "not exist");
        AppExecFwk::OnPrepareTerminationResult result = { 0, false };
        callbackInfo->Call(result);
        AppExecFwk::AbilityTransactionCallbackInfo<AppExecFwk::OnPrepareTerminationResult>::Destroy(callbackInfo);
    }
}

void OHOSApplication::ScheduleNewProcessRequest(const AAFwk::Want &want, const std::string &moduleName,
    std::function<void(std::string)> callback, bool &isAsync)
{
    TAG_LOGD(AAFwkTag::APPKIT, "ScheduleNewProcessRequest call");
    if (callback == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null callback");
        return;
    }

    if (abilityStages_.empty()) {
        TAG_LOGE(AAFwkTag::APPKIT, "abilityStages_ empty");
        return;
    }
    auto iter = abilityStages_.find(moduleName);
    if (iter == abilityStages_.end() || iter->second == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "%{public}s not in abilityStage", moduleName.c_str());
        std::string flag;
        callback(flag);
        return;
    }
    auto *callbackInfo = AppExecFwk::AbilityTransactionCallbackInfo<std::string>::Create();
    if (callbackInfo == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null callbackInfo");
        std::string flag;
        callback(flag);
        return;
    }
    callbackInfo->Push(callback);
    if (iter->second->OnNewProcessRequest(want, callbackInfo, isAsync).empty()) {
        TAG_LOGE(AAFwkTag::APPKIT, "not exist");
        std::string result;
        callbackInfo->Call(result);
        AppExecFwk::AbilityTransactionCallbackInfo<std::string>::Destroy(callbackInfo);
    }
}

std::shared_ptr<Configuration> OHOSApplication::GetConfiguration() const
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
        TAG_LOGD(AAFwkTag::APPKIT, "null runtime");
        return true;
    }

    return runtime_->LoadRepairPatch(hqfFile, hapPath);
}

bool OHOSApplication::NotifyHotReloadPage()
{
    if (runtime_ == nullptr) {
        TAG_LOGD(AAFwkTag::APPKIT, "null runtime");
        return true;
    }

    return runtime_->NotifyHotReloadPage();
}

bool OHOSApplication::NotifyUnLoadRepairPatch(const std::string &hqfFile)
{
    if (runtime_ == nullptr) {
        TAG_LOGD(AAFwkTag::APPKIT, "null runtime");
        return true;
    }

    return runtime_->UnLoadRepairPatch(hqfFile);
}

void OHOSApplication::CleanAppTempData(bool isLastProcess)
{
    if (!isLastProcess) {
        TAG_LOGW(AAFwkTag::APPKIT, "failed");
        return;
    }
    if (abilityRuntimeContext_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null context");
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
        TAG_LOGE(AAFwkTag::APPKIT, "null context");
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
        TAG_LOGE(AAFwkTag::APPKIT, "null context");
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

void OHOSApplication::PreloadAppStartup(const BundleInfo &bundleInfo, const std::string &preloadModuleName,
    std::shared_ptr<AppExecFwk::StartupTaskData> startupTaskData)
{
    if (!IsMainProcess(bundleInfo.applicationInfo.name, bundleInfo.applicationInfo.process)) {
        TAG_LOGD(AAFwkTag::STARTUP, "not main process");
        return;
    }

    std::shared_ptr<AbilityRuntime::StartupManager> startupManager =
        DelayedSingleton<AbilityRuntime::StartupManager>::GetInstance();
    if (startupManager == nullptr) {
        TAG_LOGE(AAFwkTag::STARTUP, "failed to get startupManager");
        return;
    }

    if (!bundleInfo.hapModuleInfos.empty()) {
        for (const auto& hapModuleInfo : bundleInfo.hapModuleInfos) {
            if (hapModuleInfo.name == preloadModuleName) {
                startupManager->PreloadAppHintStartup(bundleInfo, hapModuleInfo, preloadModuleName, startupTaskData);
                break;
            }
        }
    }
}

bool OHOSApplication::IsUpdateColorNeeded(Configuration &config, AbilityRuntime::SetLevel level)
{
    std::string colorMode = config.GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_COLORMODE);
    std::string colorModeIsSetBySa =
        config.GetItem(AAFwk::GlobalConfigurationKey::COLORMODE_IS_SET_BY_SA);
    if (level < AbilityRuntime::SetLevel::SA && !colorModeIsSetBySa.empty()) {
        level = AbilityRuntime::SetLevel::SA;
    }

    TAG_LOGI(AAFwkTag::APPKIT, "current %{public}d, pre %{public}d",
        static_cast<uint8_t>(level),
        static_cast<uint8_t>(AbilityRuntime::ApplicationConfigurationManager::GetInstance().GetColorModeSetLevel()));

    bool needUpdate = true;

    if (!colorMode.empty()) {
        config.AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_COLORMODE,
            AbilityRuntime::ApplicationConfigurationManager::GetInstance().SetColorModeSetLevel(level, colorMode));

        if (level > AbilityRuntime::SetLevel::System) {
            config.AddItem(AAFwk::GlobalConfigurationKey::COLORMODE_IS_SET_BY_APP,
                AppExecFwk::ConfigurationInner::IS_SET_BY_APP);
        }
    }

    if (level < AbilityRuntime::ApplicationConfigurationManager::GetInstance().GetColorModeSetLevel() ||
        colorMode.empty()) {
        config.RemoveItem(AAFwk::GlobalConfigurationKey::SYSTEM_COLORMODE);
        config.RemoveItem(AAFwk::GlobalConfigurationKey::COLORMODE_IS_SET_BY_SA);
        TAG_LOGI(AAFwkTag::APPKIT, "color remove");
        needUpdate = false;
    }

    return needUpdate;
}

bool OHOSApplication::isUpdateFontSize(Configuration &config, AbilityRuntime::SetLevel level)
{
    std::string fontSize = config.GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_FONT_SIZE_SCALE);
    if (fontSize.empty()) {
        TAG_LOGW(AAFwkTag::APPKIT, "fontSize empty");
        return false;
    }

    auto preLevle = ApplicationConfigurationManager::GetInstance().GetFontSetLevel();
    if (level < preLevle) {
        TAG_LOGW(AAFwkTag::APPKIT, "low level");
        config.RemoveItem(AAFwk::GlobalConfigurationKey::SYSTEM_FONT_SIZE_SCALE);
        return false;
    }

    std::string globalFontFollowSysteme = configuration_->GetItem(AAFwk::GlobalConfigurationKey::APP_FONT_SIZE_SCALE);
    if (level == preLevle && !globalFontFollowSysteme.empty()) {
        TAG_LOGW(AAFwkTag::APPKIT, "same level");
        if (globalFontFollowSysteme.compare(ConfigurationInner::IS_APP_FONT_FOLLOW_SYSTEM) == 0) {
            return true;
        }
        config.RemoveItem(AAFwk::GlobalConfigurationKey::SYSTEM_FONT_SIZE_SCALE);
        return false;
    }

    // level > preLevle
    TAG_LOGW(AAFwkTag::APPKIT, "high level");
    configuration_->RemoveItem(AAFwk::GlobalConfigurationKey::APP_FONT_SIZE_SCALE);
    ApplicationConfigurationManager::GetInstance().SetfontSetLevel(level);
    return true;
}

bool OHOSApplication::IsUpdateLanguageNeeded(Configuration &config, AbilityRuntime::SetLevel level)
{
    TAG_LOGI(AAFwkTag::APPKIT, "current %{public}d, pre %{public}d", static_cast<uint8_t>(level),
        static_cast<uint8_t>(AbilityRuntime::ApplicationConfigurationManager::GetInstance().GetLanguageSetLevel()));

    std::string language = config.GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE);
    if (language.empty()) {
        TAG_LOGW(AAFwkTag::APPKIT, "language empty");
        return false;
    }
    if (level < AbilityRuntime::ApplicationConfigurationManager::GetInstance().GetLanguageSetLevel()) {
        config.RemoveItem(AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE);
        TAG_LOGI(AAFwkTag::APPKIT, "language remove");
        return false;
    }
    AbilityRuntime::ApplicationConfigurationManager::GetInstance().SetLanguageSetLevel(level);
    config.AddItem(AAFwk::GlobalConfigurationKey::IS_PREFERRED_LANGUAGE,
        level == AbilityRuntime::SetLevel::Application ? "1" : "0");
    return true;
}

bool OHOSApplication::IsUpdateLocaleNeeded(const Configuration& updatedConfig, Configuration &config)
{
    std::string language = config.GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE);
    std::string locale = config.GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_LOCALE);
    if (language.empty() && locale.empty()) {
        TAG_LOGW(AAFwkTag::APPKIT, "language and locale empty");
        return false;
    }
    std::string updatedLocale;
    if (!language.empty() && !locale.empty()) {
        updatedLocale = ApplicationConfigurationManager::GetUpdatedLocale(locale, language);
    } else if (!language.empty()) {
        updatedLocale = ApplicationConfigurationManager::GetUpdatedLocale(
            updatedConfig.GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_LOCALE), language);
    } else {
        updatedLocale = ApplicationConfigurationManager::GetUpdatedLocale(locale,
            updatedConfig.GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE));
    }
    config.AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_LOCALE, updatedLocale);
    return true;
}

bool OHOSApplication::IsMainProcess(const std::string &bundleName, const std::string &process)
{
    auto processInfo = GetProcessInfo();
    if (processInfo == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null processInfo");
        return false;
    }
    ProcessType processType = processInfo->GetProcessType();
    if (processType == ProcessType::NORMAL) {
        return true;
    }

    std::string processName = processInfo->GetProcessName();
    if (processName == bundleName || processName == process) {
        return true;
    }
    TAG_LOGD(AAFwkTag::APPKIT, "not main process");
    return false;
}

void OHOSApplication::SetEntryLoadPath(const std::string &path)
{
    if (path.empty()) {
        TAG_LOGE(AAFwkTag::APPKIT, "empty path");
        return;
    }
    entryLoadPath_ = path;
}

void OHOSApplication::SetDeduplicateHar(const bool deduplicate)
{
    deduplicate_ = deduplicate;
}

#ifdef SUPPORT_GRAPHICS
bool OHOSApplication::GetDisplayConfig(uint64_t displayId, float &density, std::string &directionStr)
{
    TAG_LOGD(AAFwkTag::APPKIT, "get display by id %{public}" PRIu64, displayId);
    auto display = Rosen::DisplayManager::GetInstance().GetDisplayById(displayId);
    if (display == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "display %{public}" PRIu64 " failed", displayId);
        return false;
    }
    density = display->GetVirtualPixelRatio();
    int32_t width = display->GetWidth();
    int32_t height = display->GetHeight();
    directionStr = AppExecFwk::GetDirectionStr(height, width);
    TAG_LOGD(AAFwkTag::APPKIT, "displayId: %{public}" PRIu64 ", density: %{public}f, direction: %{public}s",
        displayId, density, directionStr.c_str());
    return true;
}
#endif

bool OHOSApplication::UpdateETSRuntime(AbilityRuntime::Runtime::Options &option)
{
    if (runtime_ == nullptr || runtime_->GetLanguage() != AbilityRuntime::Runtime::Language::JS) {
        TAG_LOGE(AAFwkTag::APPKIT, "null runtime or wrong language");
        return false;
    }
    option.lang = AbilityRuntime::Runtime::Language::ETS;
    std::unique_ptr<AbilityRuntime::ETSRuntime> etsRuntime =
        AbilityRuntime::ETSRuntime::Create(option, runtime_, false);
    if (etsRuntime == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "create etsRuntime failed");
        return false;
    }
    {
        std::lock_guard<std::mutex> lock(runtimeMutex_);
        etsRuntime->SetJsRuntime(runtime_);
        runtime_ = std::move(etsRuntime);
    }
    return true;
}

#ifdef SUPPORT_SCREEN
void OHOSApplication::RegisterGetAllUIAbilitiesCallback(
    const std::shared_ptr<AbilityRuntime::ApplicationContext> &context,
    const std::weak_ptr<OHOSApplication> &appWeakPtr)
{
    context->RegisterGetAllUIAbilitiesCallback(
        [appWeakPtr](std::vector<std::shared_ptr<AbilityRuntime::UIAbility>> &uIAbilities) {
            std::shared_ptr<OHOSApplication> appSptr = appWeakPtr.lock();
            if (appSptr == nullptr) {
                TAG_LOGE(AAFwkTag::APPKIT, "null applicationSptr");
                return;
            }
            appSptr->GetAllUIAbilities(uIAbilities);
        });
}

void OHOSApplication::GetAllUIAbilities(std::vector<std::shared_ptr<AbilityRuntime::UIAbility>> &uiAbility)
{
    if (abilityRecordMgr_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null abilityRecordMgr_");
        return;
    }

    for (const auto &abilityToken : abilityRecordMgr_->GetAllTokens()) {
        auto abilityRecord = abilityRecordMgr_->GetAbilityItem(abilityToken);
        if (abilityRecord == nullptr) {
            TAG_LOGE(AAFwkTag::APPKIT, "null abilityRecord");
            continue;
        }
        auto abilityInfo = abilityRecord->GetAbilityInfo();
        if (abilityInfo == nullptr || abilityInfo->type != AbilityType::PAGE) {
            continue;
        }
        auto abilityThread = abilityRecord->GetAbilityThread();
        if (abilityThread == nullptr) {
            TAG_LOGE(AAFwkTag::APPKIT, "null abilityThread");
            continue;
        }
        auto ability = abilityThread->GetUIAbility();
        if (ability == nullptr) {
            TAG_LOGE(AAFwkTag::APPKIT, "null uiAbility");
            continue;
        }
        uiAbility.emplace_back(ability);
    }
}
#endif
}  // namespace AppExecFwk
}  // namespace OHOS
