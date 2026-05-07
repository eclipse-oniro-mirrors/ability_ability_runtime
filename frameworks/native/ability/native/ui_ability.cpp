/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#include "ui_ability.h"

#include "ability_lifecycle.h"
#include "ability_recovery.h"
#include "configuration_convertor.h"
#include "display_util.h"
#include "display_info.h"
#ifdef SUPPORT_SCREEN
#include "distributed_client.h"
#endif
#include "ets_runtime.h"
#include "ets_ui_ability_instance.h"
#include "event_report.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "js_ui_ability.h"
#include "ability_context_impl.h"
#include "application_configuration_manager.h"
#ifdef CJ_FRONTEND
#include "cj_ui_ability.h"
#endif
#include "ohos_application.h"
#include "reverse_continuation_scheduler_primary_stage.h"
#include "runtime.h"
#include "resource_config_helper.h"
#include <random>
#include <sstream>
#include <iomanip>
#ifdef WITH_DLP
#include "set_dlp_config.h"
#endif // WITH_DLP
#ifdef SUPPORT_GRAPHICS
#include "wm_common.h"
#endif

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr char DMS_SESSION_ID[] = "sessionId";
constexpr char DMS_ORIGIN_DEVICE_ID[] = "deviceId";
constexpr int32_t DEFAULT_DMS_SESSION_ID = 0;
#ifdef SUPPORT_SCREEN
constexpr char LAUNCHER_BUNDLE_NAME[] = "com.ohos.launcher";
constexpr char LAUNCHER_ABILITY_NAME[] = "com.ohos.launcher.MainAbility";
constexpr char SHOW_ON_LOCK_SCREEN[] = "ShowOnLockScreen";
#endif

#ifdef WITH_DLP
constexpr char DLP_PARAMS_SECURITY_FLAG[] = "ohos.dlp.params.securityFlag";
#endif // WITH_DLP
constexpr char COMPONENT_STARTUP_NEW_RULES[] = "component.startup.newRules";

std::string GenerateInstanceId()
{
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint64_t> dist(0, UINT64_MAX);
    uint64_t high = dist(gen);
    uint64_t low = dist(gen);
    high = (high & 0xFFFFFFFFFFFF0FFFULL) | 0x0000000000004000ULL;
    low = (low & 0x3FFFFFFFFFFFFFFFULL) | 0x8000000000000000ULL;
    std::ostringstream oss;
    constexpr int32_t digSize = 4;
    constexpr uint8_t bitMoveSize1 = 48;
    constexpr uint8_t bitMoveSize2 = 32;
    constexpr uint8_t bitMoveSize3 = 16;
    oss << std::hex << std::setfill('0') <<
        std::setw(digSize) << ((high >> bitMoveSize1) & 0xFFFF) << "-" <<
        std::setw(digSize) << ((high >> bitMoveSize2) & 0xFFFF) << "-" <<
        std::setw(digSize) << ((high >> bitMoveSize3) & 0xFFFF) << "-" <<
        std::setw(digSize) << (high & 0xFFFF) << "-" <<
        std::setw(digSize) << ((low >> bitMoveSize1) & 0xFFFF) <<
        std::setw(digSize) << ((low >> bitMoveSize2) & 0xFFFF) <<
        std::setw(digSize) << ((low >> bitMoveSize3) & 0xFFFF) <<
        std::setw(digSize) << (low & 0xFFFF);
    return oss.str();
}
#ifdef SUPPORT_SCREEN
constexpr int32_t ERR_INVALID_VALUE = -1;
constexpr const char* IS_CALLING_FROM_DMS = "supportCollaborativeCallingFromDmsInAAFwk";
constexpr const char* SUPPORT_COLLABORATE_INDEX = "ohos.extra.param.key.supportCollaborateIndex";
constexpr const char* COLLABORATE_KEY = "ohos.dms.collabToken";
#endif
}
UIAbility *UIAbility::Create(const std::unique_ptr<Runtime> &runtime)
{
    if (!runtime) {
        return new (std::nothrow) UIAbility;
    }

    switch (runtime->GetLanguage()) {
        case Runtime::Language::JS:
            return JsUIAbility::Create(runtime);
#ifdef CJ_FRONTEND
        case Runtime::Language::CJ:
            return CJUIAbility::Create(runtime);
#endif
        case Runtime::Language::ETS:
            return CreateETSUIAbility(runtime);
        default:
            return new (std::nothrow) UIAbility();
    }
}

void UIAbility::Init(std::shared_ptr<AppExecFwk::AbilityLocalRecord> record,
    const std::shared_ptr<AppExecFwk::OHOSApplication> application,
    std::shared_ptr<AppExecFwk::AbilityHandler> &handler, const sptr<IRemoteObject> &token, bool &createObjSuc)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
    if (record == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null record");
        return;
    }
    instanceId_ = GenerateInstanceId();
    TAG_LOGI(AAFwkTag::UIABILITY, "instanceId: %{public}s", instanceId_.c_str());
    application_ = application;
    abilityInfo_ = record->GetAbilityInfo();
    handler_ = handler;
    token_ = token;
#ifdef SUPPORT_SCREEN
    continuationManager_ = std::make_shared<AppExecFwk::ContinuationManagerStage>();
    std::weak_ptr<AppExecFwk::ContinuationManagerStage> continuationManager = continuationManager_;
    continuationHandler_ =
        std::make_shared<AppExecFwk::ContinuationHandlerStage>(continuationManager, weak_from_this());
    if (!continuationManager_->Init(shared_from_this(), GetToken(), GetAbilityInfo(), continuationHandler_)) {
        continuationManager_.reset();
    } else {
        std::weak_ptr<AppExecFwk::ContinuationHandlerStage> continuationHandler = continuationHandler_;
        sptr<AppExecFwk::ReverseContinuationSchedulerPrimaryStage> primary =
            new (std::nothrow) AppExecFwk::ReverseContinuationSchedulerPrimaryStage(continuationHandler, handler_);
        if (primary == nullptr) {
            TAG_LOGE(AAFwkTag::UIABILITY, "null primary");
        } else {
            continuationHandler_->SetPrimaryStub(primary);
            continuationHandler_->SetAbilityInfo(abilityInfo_);
        }
    }
    // register displayid change callback
    TAG_LOGD(AAFwkTag::UIABILITY, "registerDisplayListener");
    abilityDisplayListener_ = new (std::nothrow) UIAbilityDisplayListener(weak_from_this());
    if (abilityDisplayListener_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null abilityDisplayListener_");
        return;
    }
    TAG_LOGD(AAFwkTag::UIABILITY, "end register");
    Rosen::WindowManager::GetInstance().RegisterDisplayInfoChangedListener(token_, abilityDisplayListener_);
#endif
    lifecycle_ = std::make_shared<AppExecFwk::LifeCycle>();
    abilityLifecycleExecutor_ = std::make_shared<AppExecFwk::AbilityLifecycleExecutor>();
    abilityLifecycleExecutor_->DispatchLifecycleState(AppExecFwk::AbilityLifecycleExecutor::LifecycleState::INITIAL);
    if (abilityContext_ != nullptr) {
        abilityContext_->RegisterAbilityCallback(weak_from_this());
        abilityContext_->SetHook(record->IsHook());
    }
    TAG_LOGD(AAFwkTag::UIABILITY, "end");
}

std::shared_ptr<OHOS::AppExecFwk::LifeCycle> UIAbility::GetLifecycle()
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
    return lifecycle_;
}

void UIAbility::RegisterAbilityLifecycleObserver(const std::shared_ptr<AppExecFwk::ILifecycleObserver> &observer)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
    if (observer == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null observer");
        return;
    }
    if (lifecycle_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null lifecycle_");
        return;
    }
    lifecycle_->AddObserver(observer);
}

void UIAbility::UnregisterAbilityLifecycleObserver(const std::shared_ptr<AppExecFwk::ILifecycleObserver> &observer)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
    if (observer == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null observer");
        return;
    }
    if (lifecycle_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null lifecycle_");
        return;
    }
    lifecycle_->RemoveObserver(observer);
}

void UIAbility::AttachAbilityContext(const std::shared_ptr<AbilityRuntime::AbilityContext> &abilityContext)
{
    abilityContext_ = abilityContext;
    std::weak_ptr<UIAbility> abilityWptr = weak_from_this();
    std::weak_ptr<AbilityRuntime::AbilityContext> abilityContextWptr = abilityContext_;
    abilityContext_->RegisterAbilityConfigUpdateCallback(
        [abilityWptr, abilityContextWptr](AppExecFwk::Configuration &config) {
        std::shared_ptr<UIAbility> abilitySptr = abilityWptr.lock();
        std::shared_ptr<AbilityRuntime::AbilityContext> abilityContext = abilityContextWptr.lock();
        if (abilitySptr == nullptr || abilityContext == nullptr ||
            abilityContext->GetAbilityInfo() == nullptr) {
            TAG_LOGE(AAFwkTag::UIABILITY, "null abilitySptr or null abilityContext or null GetAbilityInfo");
            return;
        }
        if (abilityContext->GetAbilityConfiguration() == nullptr) {
            auto abilityModuleContext = abilityContext->CreateModuleContext(
                abilityContext->GetAbilityInfo()->moduleName);
            if (abilityModuleContext == nullptr) {
                TAG_LOGE(AAFwkTag::UIABILITY, "null abilityModuleContext");
                return;
            }
            auto abilityResourceMgr = abilityModuleContext->GetResourceManager();
            abilityContext->SetAbilityResourceManager(abilityResourceMgr);
            AbilityRuntime::ApplicationConfigurationManager::GetInstance().
                AddIgnoreContext(abilityContext, abilityResourceMgr);
        }
        abilityContext->SetAbilityConfiguration(config);
        if (config.GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_COLORMODE).
            compare(AppExecFwk::ConfigurationInner::COLOR_MODE_AUTO) == 0) {
            config.AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_COLORMODE,
                AbilityRuntime::ApplicationConfigurationManager::GetInstance().GetColorMode());
            if (AbilityRuntime::ApplicationConfigurationManager::GetInstance().
                GetColorModeSetLevel() > AbilityRuntime::SetLevel::System) {
                config.AddItem(AAFwk::GlobalConfigurationKey::COLORMODE_IS_SET_BY_APP,
                    AppExecFwk::ConfigurationInner::IS_SET_BY_APP);
            }
            abilityContext->GetAbilityConfiguration()->
                RemoveItem(AAFwk::GlobalConfigurationKey::SYSTEM_COLORMODE);
            abilityContext->GetAbilityConfiguration()->
                RemoveItem(AAFwk::GlobalConfigurationKey::COLORMODE_IS_SET_BY_APP);
        }
        ResourceConfigHelper resourceConfig;
        abilitySptr->InitConfigurationProperties(config, resourceConfig);
        resourceConfig.SetISAbilityColor(true);
        auto resourceManager = abilityContext->GetResourceManager();
        resourceConfig.UpdateResConfig(config, resourceManager);
        auto diffConfiguration = std::make_shared<AppExecFwk::Configuration>(config);
        if (abilitySptr->GetScene()) {
            abilitySptr->GetScene()->UpdateConfigurationForSpecified(diffConfiguration, resourceManager);
        }
        abilitySptr->OnConfigurationUpdated(config);
    });
}

#ifdef SUPPORT_SCREEN
napi_value UIAbility::GetWindowStage()
{
    return nullptr;
}

ani_object UIAbility::GetEtsWindowStage()
{
    return nullptr;
}
#endif

void UIAbility::OnStart(const AAFwk::Want &want, sptr<AAFwk::SessionInfo> sessionInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (abilityInfo_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null abilityInfo_");
        return;
    }

#ifdef WITH_DLP
    if (want.HasParameter(DLP_PARAMS_SECURITY_FLAG)) {
        Security::DlpPermission::DlpSetConfig::SetDlpConfig(want);
    }
    securityFlag_ = want.GetBoolParam(DLP_PARAMS_SECURITY_FLAG, false);
    (const_cast<AAFwk::Want &>(want)).RemoveParam(DLP_PARAMS_SECURITY_FLAG);
#endif // WITH_DLP
    SetWant(want);
    TAG_LOGD(AAFwkTag::UIABILITY, "ability: %{public}s", abilityInfo_->name.c_str());
#ifdef SUPPORT_SCREEN
    if (sessionInfo != nullptr) {
        SetSessionToken(sessionInfo->sessionToken, sessionInfo->renderSession);
        SetIdentityToken(sessionInfo->identityToken);
    }
    OnStartForSupportGraphics(want);
#endif
    if (abilityLifecycleExecutor_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null abilityLifecycleExecutor_");
        return;
    }
    abilityLifecycleExecutor_->DispatchLifecycleState(
        AppExecFwk::AbilityLifecycleExecutor::LifecycleState::STARTED_NEW);

    if (lifecycle_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null lifecycle_");
        return;
    }
    lifecycle_->DispatchLifecycle(AppExecFwk::LifeCycle::Event::ON_START, want);
    TAG_LOGD(AAFwkTag::UIABILITY, "end");
}

void UIAbility::OnStop()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
#ifdef SUPPORT_SCREEN
    TAG_LOGD(AAFwkTag::UIABILITY, "unregisterDisplayInfoChangedListener");
    (void)Rosen::WindowManager::GetInstance().UnregisterDisplayInfoChangedListener(token_, abilityDisplayListener_);
    auto &&window = GetWindow();
    if (window != nullptr) {
        TAG_LOGD(AAFwkTag::UIABILITY, "UnregisterDisplayMoveListener");
        window->UnregisterDisplayMoveListener(abilityDisplayMoveListener_);
    }
    // Call JS Func(onWindowStageDestroy) and Release the scene.
    if (scene_ != nullptr) {
        OnSceneWillDestroy();
        scene_->GoDestroy();
        onSceneDestroyed();
    }
#endif
    if (abilityLifecycleExecutor_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null abilityLifecycleExecutor_");
        return;
    }
    abilityLifecycleExecutor_->DispatchLifecycleState(AppExecFwk::AbilityLifecycleExecutor::LifecycleState::INITIAL);
    AbilityRuntime::ApplicationConfigurationManager::GetInstance().
        DeleteIgnoreContext(abilityContext_);
    if (lifecycle_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null lifecycle_");
        return;
    }
    lifecycle_->DispatchLifecycle(AppExecFwk::LifeCycle::Event::ON_STOP);
#ifdef SUPPORT_SCREEN
    Rosen::DisplayManager::GetInstance().RemoveDisplayIdFromAms(token_);
#endif
    TAG_LOGD(AAFwkTag::UIABILITY, "end");
}

void UIAbility::OnStop(AppExecFwk::AbilityTransactionCallbackInfo<> *callbackInfo, bool &isAsyncCallback)
{
    isAsyncCallback = false;
    OnStop();
}

void UIAbility::OnStopCallback()
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
}

void UIAbility::DestroyInstance()
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
}

bool UIAbility::CreateProperty(std::shared_ptr<AbilityRuntime::AbilityContext> abilityContext,
    std::shared_ptr<AppExecFwk::BaseDelegatorAbilityProperty> delegatorAbilityProperty)
{
    if (abilityContext == nullptr || delegatorAbilityProperty == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null abilityContext or delegatorAbilityProperty");
        return false;
    }
    delegatorAbilityProperty->token_ = abilityContext->GetToken();
    delegatorAbilityProperty->name_ = GetAbilityName();
    delegatorAbilityProperty->moduleName_ = GetModuleName();
    if (GetApplicationInfo() == nullptr || GetApplicationInfo()->bundleName.empty()) {
        delegatorAbilityProperty->fullName_ = GetAbilityName();
    } else {
        std::string::size_type pos = GetAbilityName().find(GetApplicationInfo()->bundleName);
        if (pos == std::string::npos || pos != 0) {
            delegatorAbilityProperty->fullName_ = GetApplicationInfo()->bundleName + "." + GetAbilityName();
        } else {
            delegatorAbilityProperty->fullName_ = GetAbilityName();
        }
    }
    delegatorAbilityProperty->lifecycleState_ = GetState();
    return true;
}

bool UIAbility::IsRestoredInContinuation() const
{
    if (abilityContext_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null abilityContext_");
        return false;
    }

    if (launchParam_.launchReason != AAFwk::LaunchReason::LAUNCHREASON_CONTINUATION) {
        TAG_LOGD(AAFwkTag::UIABILITY, "launchReason: %{public}d", launchParam_.launchReason);
        return false;
    }

    return true;
}

bool UIAbility::ShouldRecoverState(const AAFwk::Want &want)
{
    if (!want.GetBoolParam(Want::PARAM_ABILITY_RECOVERY_RESTART, false)) {
        TAG_LOGD(AAFwkTag::UIABILITY, "appRecovery not recovery restart");
        return false;
    }

    if (abilityRecovery_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null abilityRecovery_");
        return false;
    }

    if (abilityContext_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null abilityContext_");
        return false;
    }

    if (abilityContext_->GetContentStorage() == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "get content failed");
        return false;
    }
    return true;
}

bool UIAbility::ShouldDefaultRecoverState(const AAFwk::Want &want)
{
    auto launchParam = GetLaunchParam();
    if (CheckDefaultRecoveryEnabled() && IsStartByScb() &&
        want.GetBoolParam(Want::PARAM_ABILITY_RECOVERY_RESTART, false) &&
        (launchParam.lastExitReason == AAFwk::LastExitReason::LASTEXITREASON_PERFORMANCE_CONTROL ||
        launchParam.lastExitReason == AAFwk::LastExitReason::LASTEXITREASON_RESOURCE_CONTROL)) {
        return true;
    }
    return false;
}

void UIAbility::NotifyContinuationResult(const AAFwk::Want &want, bool success)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
    int sessionId = want.GetIntParam(DMS_SESSION_ID, DEFAULT_DMS_SESSION_ID);
    std::string originDeviceId = want.GetStringParam(DMS_ORIGIN_DEVICE_ID);

    if (continuationManager_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null continuationManager_");
        return;
    }
    continuationManager_->NotifyCompleteContinuation(
        originDeviceId, sessionId, success, reverseContinuationSchedulerReplica_);
}

void UIAbility::OnConfigurationUpdatedNotify(const AppExecFwk::Configuration &configuration)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "begin");
    auto newConfig = AppExecFwk::Configuration(configuration);
    auto abilityConfig = GetAbilityContext()->GetAbilityConfiguration();
    if (abilityConfig != nullptr) {
        newConfig.FilterDuplicates(*abilityConfig);
        TAG_LOGI(AAFwkTag::UIABILITY, "newConfig: %{public}s", newConfig.GetName().c_str());
        if (newConfig.GetItemSize() == 0) {
            return;
        }
        auto diffConfiguration = std::make_shared<AppExecFwk::Configuration>(newConfig);
        auto resourceManager = abilityContext_->GetResourceManager();
        ResourceConfigHelper resourceConfig;
        InitConfigurationProperties(newConfig, resourceConfig);
        resourceConfig.UpdateResConfig(newConfig, resourceManager);
        auto windowScene = GetScene();
        if (windowScene) {
            windowScene->UpdateConfigurationForSpecified(
                diffConfiguration, resourceManager);
        }
    } else {
        ResourceConfigHelper resourceConfig;
        InitConfigurationProperties(configuration, resourceConfig);
        auto resourceManager = GetResourceManager();
        resourceConfig.UpdateResConfig(configuration, resourceManager);
    }
    if (abilityContext_ != nullptr && application_ != nullptr) {
        abilityContext_->SetConfiguration(application_->GetConfiguration());
    }
    // Notify Ability Subclass
    OnConfigurationUpdated(newConfig);
    TAG_LOGD(AAFwkTag::UIABILITY, "end");
}

void UIAbility::InitConfigurationProperties(const AppExecFwk::Configuration &changeConfiguration,
    ResourceConfigHelper &resourceConfig)
{
    resourceConfig.SetMcc(changeConfiguration.GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_MCC));
    resourceConfig.SetMnc(changeConfiguration.GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_MNC));
    if (setting_) {
        auto displayId =
            std::atoi(setting_->GetProperty(AppExecFwk::AbilityStartSetting::WINDOW_DISPLAY_ID_KEY).c_str());
        resourceConfig.SetLanguage(changeConfiguration.GetItem(displayId,
            AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE));
        resourceConfig.SetLocale(changeConfiguration.GetItem(displayId,
            AAFwk::GlobalConfigurationKey::SYSTEM_LOCALE));
        resourceConfig.SetColormode(changeConfiguration.GetItem(displayId,
            AAFwk::GlobalConfigurationKey::SYSTEM_COLORMODE));
        resourceConfig.SetHasPointerDevice(changeConfiguration.GetItem(displayId,
            AAFwk::GlobalConfigurationKey::INPUT_POINTER_DEVICE));
        TAG_LOGD(AAFwkTag::UIABILITY, "displayId: [%{public}d], language: [%{public}s], locale: [%{public}s], "
            "colormode: [%{public}s], hasPointerDevice: [%{public}s] mcc: [%{public}s], mnc: [%{public}s]", displayId,
            resourceConfig.GetLanguage().c_str(), resourceConfig.GetLocale().c_str(),
            resourceConfig.GetColormode().c_str(), resourceConfig.GetHasPointerDevice().c_str(),
            resourceConfig.GetMcc().c_str(), resourceConfig.GetMnc().c_str());
    } else {
        resourceConfig.SetLanguage(changeConfiguration.GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE));
        resourceConfig.SetLocale(changeConfiguration.GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_LOCALE));
        resourceConfig.SetColormode(changeConfiguration.GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_COLORMODE));
        resourceConfig.SetHasPointerDevice(changeConfiguration.GetItem(
            AAFwk::GlobalConfigurationKey::INPUT_POINTER_DEVICE));
        TAG_LOGD(AAFwkTag::UIABILITY,
            "language: [%{public}s], locale: [%{public}s], colormode: [%{public}s], hasPointerDevice: [%{public}s], "
            "mcc: [%{public}s], mnc: [%{public}s]",
            resourceConfig.GetLanguage().c_str(), resourceConfig.GetLocale().c_str(),
            resourceConfig.GetColormode().c_str(), resourceConfig.GetHasPointerDevice().c_str(),
            resourceConfig.GetMcc().c_str(), resourceConfig.GetMnc().c_str());
    }
}

void UIAbility::OnMemoryLevel(int level)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
#ifdef SUPPORT_SCREEN
    if (scene_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null scene");
        return;
    }
    scene_->NotifyMemoryLevel(level);
#endif
}

std::string UIAbility::GetAbilityName()
{
    if (abilityInfo_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null abilityInfo_");
        return "";
    }
    return abilityInfo_->name;
}

std::string UIAbility::GetModuleName()
{
    if (abilityInfo_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null abilityInfo_");
        return "";
    }

    return abilityInfo_->moduleName;
}

std::string UIAbility::GetInstanceId() const
{
    return instanceId_;
}

void UIAbility::OnAbilityResult(int requestCode, int resultCode, const AAFwk::Want &want)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
}

void UIAbility::OnNewWant(const AAFwk::Want &want)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
}

void UIAbility::OnRestoreAbilityState(const AppExecFwk::PacMap &inState)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
}

void UIAbility::SetWant(const AAFwk::Want &want)
{
    std::lock_guard<std::mutex> lock(wantMutexlock_);
    setWant_ = std::make_shared<AAFwk::Want>(want);
}

std::shared_ptr<AAFwk::Want> UIAbility::GetWant()
{
    std::lock_guard<std::mutex> lock(wantMutexlock_);
    return setWant_;
}

void UIAbility::OnConfigurationUpdated(const AppExecFwk::Configuration &configuration)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
}

void UIAbility::Dump(const std::vector<std::string> &params, std::vector<std::string> &info)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
}

AppExecFwk::AbilityLifecycleExecutor::LifecycleState UIAbility::GetState()
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
    if (abilityLifecycleExecutor_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null abilityLifecycleExecutor_");
        return AppExecFwk::AbilityLifecycleExecutor::LifecycleState::UNINITIALIZED;
    }
    return static_cast<AppExecFwk::AbilityLifecycleExecutor::LifecycleState>(abilityLifecycleExecutor_->GetState());
}

int32_t UIAbility::OnContinueAsyncCB(napi_ref jsWantParams, int32_t status,
    const AppExecFwk::AbilityInfo &abilityInfo)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
    return ERR_OK;
}

int32_t UIAbility::OnContinueAsyncCB(ani_ref aniWantParamsRef, int32_t status,
    const AppExecFwk::AbilityInfo &abilityInfo)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
    return ERR_OK;
}

int32_t UIAbility::OnContinue(AAFwk::WantParams &wantParams, bool &isAsyncOnContinue,
    const AppExecFwk::AbilityInfo &abilityInfo)
{
    return AppExecFwk::ContinuationManagerStage::OnContinueResult::ON_CONTINUE_ERR;
}

void UIAbility::ContinueAbilityWithStack(const std::string &deviceId, uint32_t versionCode)
{
    if (deviceId.empty()) {
        TAG_LOGE(AAFwkTag::UIABILITY, "empty deviceId");
        return;
    }

    if (continuationManager_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null continuationManager_");
        return;
    }
    continuationManager_->ContinueAbilityWithStack(deviceId, versionCode);
}

bool UIAbility::OnStartContinuation()
{
    return false;
}

bool UIAbility::OnSaveData(AAFwk::WantParams &saveData)
{
    return false;
}

bool UIAbility::OnRestoreData(AAFwk::WantParams &restoreData)
{
    return false;
}

int32_t UIAbility::OnSaveState(int32_t reason, AAFwk::WantParams &wantParams)
{
    return ERR_OK;
}

int32_t UIAbility::OnSaveState(int32_t reason, AAFwk::WantParams &wantParams,
    AppExecFwk::AbilityTransactionCallbackInfo<AppExecFwk::OnSaveStateResult> *callbackInfo,
    bool &isAsync, AppExecFwk::StateReason stateReason)
{
    return ERR_OK;
}

void UIAbility::OnCompleteContinuation(int result)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
    if (continuationManager_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null continuationManager_");
        return;
    }

    continuationManager_->ChangeProcessStateToInit();
}

void UIAbility::OnRemoteTerminated()
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
}

void UIAbility::DispatchLifecycleOnForeground(const AAFwk::Want &want)
{
    if (abilityLifecycleExecutor_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null abilityLifecycleExecutor_");
        return;
    }
    abilityLifecycleExecutor_->DispatchLifecycleState(
        AppExecFwk::AbilityLifecycleExecutor::LifecycleState::FOREGROUND_NEW);

    if (lifecycle_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null lifecycle_");
        return;
    }
    lifecycle_->DispatchLifecycle(AppExecFwk::LifeCycle::Event::ON_FOREGROUND, want);
}

void UIAbility::HandleCreateAsRecovery(const AAFwk::Want &want)
{
    if (!want.GetBoolParam(Want::PARAM_ABILITY_RECOVERY_RESTART, false)) {
        TAG_LOGD(AAFwkTag::UIABILITY, "appRecovery not recovery restart");
        return;
    }

    if (abilityRecovery_ != nullptr) {
        abilityRecovery_->ScheduleRestoreAbilityState(AppExecFwk::StateReason::DEVELOPER_REQUEST, want);
    }
}

void UIAbility::SetStartAbilitySetting(std::shared_ptr<AppExecFwk::AbilityStartSetting> setting)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
    setting_ = setting;
}

void UIAbility::SetLaunchParam(const AAFwk::LaunchParam &launchParam)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
    launchParam_ = launchParam;
}

const AAFwk::LaunchParam &UIAbility::GetLaunchParam() const
{
    return launchParam_;
}

std::shared_ptr<AbilityRuntime::AbilityContext> UIAbility::GetAbilityContext()
{
    return abilityContext_;
}

sptr<IRemoteObject> UIAbility::CallRequest()
{
    return nullptr;
}

bool UIAbility::IsUseNewStartUpRule()
{
    std::lock_guard<std::mutex> lock(wantMutexlock_);
    if (!isNewRuleFlagSetted_ && setWant_) {
        startUpNewRule_ = setWant_->GetBoolParam(COMPONENT_STARTUP_NEW_RULES, false);
        isNewRuleFlagSetted_ = true;
    }
    return startUpNewRule_;
}

void UIAbility::EnableAbilityRecovery(const std::shared_ptr<AppExecFwk::AbilityRecovery> &abilityRecovery,
    bool useAppSettedRecoveryValue)
{
    abilityRecovery_ = abilityRecovery;
    useAppSettedRecoveryValue_.store(useAppSettedRecoveryValue);
}

int32_t UIAbility::OnShare(AAFwk::WantParams &wantParams)
{
    return ERR_OK;
}

void UIAbility::BindHybridContext(const std::shared_ptr<AppExecFwk::OHOSApplication> application,
    const std::shared_ptr<AppExecFwk::AbilityLocalRecord> record)
{
    if (application == nullptr || record == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null application or record");
        return;
    }
    auto appInfo = application->GetApplicationInfo();
    if (appInfo == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null appInfo");
        return;
    }
    if (appInfo->arkTSMode != AppExecFwk::Constants::ARKTS_MODE_HYBRID) {
        TAG_LOGD(AAFwkTag::UIABILITY, "not hybrid app, no need to create hybrid context");
        return;
    }

    auto abilityInfo = record->GetAbilityInfo();
    if (abilityInfo == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null abilityInfo");
        return;
    }
    if (abilityInfo->arkTSMode == AppExecFwk::Constants::ARKTS_MODE_DYNAMIC) {
        BindEtsContext(application, record);
    } else if (abilityInfo->arkTSMode == AppExecFwk::Constants::ARKTS_MODE_STATIC) {
        BindJsContext(application, record);
    } else {
        TAG_LOGE(AAFwkTag::UIABILITY, "unknown arkTSMode");
    }
}

void UIAbility::BindEtsContext(const std::shared_ptr<AppExecFwk::OHOSApplication> application,
    const std::shared_ptr<AppExecFwk::AbilityLocalRecord> record)
{
    if (abilityContext_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null abilityContext_");
        return;
    }
    auto& bindingObject = abilityContext_->GetBindingObject();
    if (bindingObject == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null bindingObject");
        return;
    }

    auto* ptr = bindingObject->Get<ani_ref>();
    if (ptr != nullptr) {
        TAG_LOGD(AAFwkTag::UIABILITY, "ets context already binded");
        return;
    }
    CreateAndBindETSUIAbilityContext(abilityContext_, application->GetRuntime());
}

void UIAbility::BindJsContext(const std::shared_ptr<AppExecFwk::OHOSApplication> application,
    const std::shared_ptr<AppExecFwk::AbilityLocalRecord> record)
{
    if (abilityContext_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null abilityContext_");
        return;
    }
    auto& bindingObject = abilityContext_->GetBindingObject();
    if (bindingObject == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null bindingObject");
        return;
    }

    auto* ptr = bindingObject->Get<NativeReference>();
    if (ptr != nullptr) {
        TAG_LOGD(AAFwkTag::UIABILITY, "ets context already binded");
        return;
    }
    auto &runtime = application->GetRuntime();
    if (runtime == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null runtime");
        return;
    }
    if (runtime->GetLanguage() == Runtime::Language::ETS) {
        JsUIAbility::CreateAndBindContext(abilityContext_, static_cast<ETSRuntime&>(*runtime).GetJsRuntime());
    } else if (runtime->GetLanguage() == Runtime::Language::JS) {
        JsUIAbility::CreateAndBindContext(abilityContext_, runtime);
    }
}

bool UIAbility::CheckIsSilentForeground() const
{
    return isSilentForeground_;
}

void UIAbility::SetIsSilentForeground(bool isSilentForeground)
{
    isSilentForeground_ = isSilentForeground;
}

#ifdef SUPPORT_SCREEN
void UIAbility::OnSceneCreated()
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
}

void UIAbility::OnSceneRestored()
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
}

void UIAbility::OnSceneWillDestroy()
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
}

void UIAbility::onSceneDestroyed()
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
}

void UIAbility::OnForeground(const AAFwk::Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
    DoOnForeground(want);
    if (isSilentForeground_) {
        TAG_LOGD(AAFwkTag::UIABILITY, "silent foreground, return");
        return;
    }
    DispatchLifecycleOnForeground(want);
    AAFwk::EventInfo eventInfo;
    eventInfo.bundleName = want.GetElement().GetBundleName();
    eventInfo.moduleName = want.GetElement().GetModuleName();
    eventInfo.abilityName = want.GetElement().GetAbilityName();
    eventInfo.callerBundleName = want.GetStringParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME);
    if (abilityInfo_ != nullptr) {
        eventInfo.bundleType = static_cast<int32_t>(abilityInfo_->applicationInfo.bundleType);
    } else {
        TAG_LOGE(AAFwkTag::UIABILITY, "null abilityInfo_");
    }
    AAFwk::EventReport::SendAbilityEvent(AAFwk::EventName::ABILITY_ONFOREGROUND, HISYSEVENT_BEHAVIOR, eventInfo);
}

void UIAbility::OnBackground()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
    if (abilityInfo_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null abilityInfo_");
        return;
    }

    if (scene_ != nullptr) {
        TAG_LOGD(AAFwkTag::UIABILITY, "goBackground sceneFlag: %{public}d", sceneFlag_);
        scene_->GoBackground(sceneFlag_);
    }

    if (abilityRecovery_ != nullptr && abilityContext_ != nullptr && abilityContext_->GetRestoreEnabled() &&
        CheckRecoveryEnabled()) {
        abilityRecovery_->ScheduleSaveAbilityState(AppExecFwk::StateReason::LIFECYCLE);
    }

    if (abilityLifecycleExecutor_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null abilityLifecycleExecutor_");
        return;
    }
    abilityLifecycleExecutor_->DispatchLifecycleState(
        AppExecFwk::AbilityLifecycleExecutor::LifecycleState::BACKGROUND_NEW);

    if (lifecycle_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null lifecycle_");
        return;
    }
    lifecycle_->DispatchLifecycle(AppExecFwk::LifeCycle::Event::ON_BACKGROUND);
    AAFwk::EventInfo eventInfo;
    eventInfo.bundleName = abilityInfo_->bundleName;
    eventInfo.moduleName = abilityInfo_->moduleName;
    eventInfo.abilityName = abilityInfo_->name;
    eventInfo.bundleType = static_cast<int32_t>(abilityInfo_->applicationInfo.bundleType);
    AAFwk::EventReport::SendAbilityEvent(AAFwk::EventName::ABILITY_ONBACKGROUND, HISYSEVENT_BEHAVIOR, eventInfo);
}

void UIAbility::OnWillForeground()
{
    TAG_LOGD(AAFwkTag::UIABILITY, "OnWillForeground is called");
}

void UIAbility::OnDidForeground()
{
    TAG_LOGD(AAFwkTag::UIABILITY, "OnDidForeground is called");
}

void UIAbility::OnWillBackground()
{
    TAG_LOGD(AAFwkTag::UIABILITY, "OnWillBackground is called");
}

void UIAbility::OnDidBackground()
{
    TAG_LOGD(AAFwkTag::UIABILITY, "OnDidBackground is called");
}

void UIAbility::OnAfterFocusedCommon(bool isFocused)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
    return;
}

bool UIAbility::OnPrepareTerminate()
{
    TAG_LOGI(AAFwkTag::UIABILITY, "called");
    return false;
}

void UIAbility::OnPrepareTerminate(AppExecFwk::AbilityTransactionCallbackInfo<bool> *callbackInfo, bool &isAsync)
{
    TAG_LOGI(AAFwkTag::UIABILITY, "called");
    return;
}

const sptr<Rosen::Window> UIAbility::GetWindow()
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
    if (scene_ != nullptr) {
        return scene_->GetMainWindow();
    }
    return nullptr;
}

std::shared_ptr<Rosen::WindowScene> UIAbility::GetScene()
{
    return scene_;
}

void UIAbility::OnLeaveForeground()
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
}

void UIAbility::HandleCollaboration(const AAFwk::Want &want)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "HandleCollaboration called");
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (abilityInfo_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null abilityInfo_");
        return;
    }
    if (want.GetBoolParam(IS_CALLING_FROM_DMS, false) &&
        (abilityInfo_->launchMode != AppExecFwk::LaunchMode::SPECIFIED)) {
        (const_cast<Want &>(want)).RemoveParam(IS_CALLING_FROM_DMS);
        SetWant(want);
        OHOS::AAFwk::WantParams wantParams = want.GetParams();
        int32_t resultCode = OnCollaborate(wantParams);
        auto abilityContext = GetAbilityContext();
        if (abilityContext == nullptr) {
            TAG_LOGE(AAFwkTag::UIABILITY, "null abilityContext");
            return;
        }
        OHOS::AAFwk::WantParams param = want.GetParams().GetWantParams(SUPPORT_COLLABORATE_INDEX);
        auto collabToken = param.GetStringParam(COLLABORATE_KEY);
        auto uid = abilityInfo_->uid;
        auto callerPid = getpid();
        auto accessTokenId = abilityInfo_->applicationInfo.accessTokenId;
        AAFwk::DistributedClient dmsClient;
        dmsClient.OnCollaborateDone(collabToken, resultCode, callerPid, uid, accessTokenId);
    }
}

int32_t UIAbility::OnCollaborate(WantParams &wantParams)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "OnCollaborate called");
    return CollaborateResult::REJECT;
}

void UIAbility::OnAbilityRequestFailure(const std::string &requestId, const AppExecFwk::ElementName &element,
    const std::string &message, int32_t resultCode)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
}

void UIAbility::OnAbilityRequestSuccess(const std::string &requestId, const AppExecFwk::ElementName &element,
    const std::string &message)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
}

std::string UIAbility::GetContentInfo()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (scene_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null scene");
        return "";
    }
    return scene_->GetContentInfo(Rosen::BackupAndRestoreType::CONTINUATION);
}

std::string UIAbility::GetContentInfoForRecovery()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (scene_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null scene");
        return "";
    }
    return scene_->GetContentInfo(Rosen::BackupAndRestoreType::APP_RECOVERY);
}

std::string UIAbility::GetContentInfoForDefaultRecovery()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (scene_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null scene");
        return "";
    }
    return scene_->GetContentInfo(Rosen::BackupAndRestoreType::RESOURCESCHEDULE_RECOVERY);
}

void UIAbility::SetSceneListener(const sptr<Rosen::IWindowLifeCycle> &listener)
{
    sceneListener_ = listener;
}

void UIAbility::DoOnForeground(const AAFwk::Want &want)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
}

int32_t UIAbility::GetCurrentWindowMode()
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
    auto windowMode = static_cast<int>(Rosen::WindowMode::WINDOW_MODE_UNDEFINED);
    if (scene_ == nullptr) {
        return windowMode;
    }
    auto window = scene_->GetMainWindow();
    if (window != nullptr) {
        windowMode = static_cast<int>(window->GetWindowMode());
    }
    return windowMode;
}

ErrCode UIAbility::SetMissionLabel(const std::string &label)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
    if (!abilityInfo_ || abilityInfo_->type != AppExecFwk::AbilityType::PAGE) {
        TAG_LOGE(AAFwkTag::UIABILITY, "invalid ability info");
        return ERR_INVALID_VALUE;
    }

    if (scene_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null scene");
        return ERR_INVALID_VALUE;
    }
    auto window = scene_->GetMainWindow();
    if (window == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null window");
        return ERR_INVALID_VALUE;
    }

    if (window->SetAPPWindowLabel(label) != OHOS::Rosen::WMError::WM_OK) {
        TAG_LOGE(AAFwkTag::UIABILITY, "setAPPWindowLabel failed");
        return ERR_INVALID_VALUE;
    }
    return ERR_OK;
}

ErrCode UIAbility::SetMissionIcon(const std::shared_ptr<OHOS::Media::PixelMap> &icon)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
    if (!abilityInfo_ || abilityInfo_->type != AppExecFwk::AbilityType::PAGE) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null abilityInfo_ or not page type");
        return ERR_INVALID_VALUE;
    }

    if (scene_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null scene_");
        return ERR_INVALID_VALUE;
    }
    auto window = scene_->GetMainWindow();
    if (window == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null window");
        return ERR_INVALID_VALUE;
    }

    if (window->SetAPPWindowIcon(icon) != OHOS::Rosen::WMError::WM_OK) {
        TAG_LOGE(AAFwkTag::UIABILITY, "setAPPWindowIcon failed");
        return ERR_INVALID_VALUE;
    }
    return ERR_OK;
}

void UIAbility::GetWindowRect(int32_t &left, int32_t &top, int32_t &width, int32_t &height)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
    if (scene_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null scene");
        return;
    }
    auto window = scene_->GetMainWindow();
    if (window == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null window");
        return;
    }
    left = window->GetRect().posX_;
    top = window->GetRect().posY_;
    width = static_cast<int32_t>(window->GetRect().width_);
    height = static_cast<int32_t>(window->GetRect().height_);
    TAG_LOGD(AAFwkTag::UIABILITY, "left: %{public}d, top: %{public}d, width: %{public}d, height: %{public}d",
        left, top, width, height);
}

Ace::UIContent *UIAbility::GetUIContent()
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
    if (scene_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null scene_");
        return nullptr;
    }
    auto window = scene_->GetMainWindow();
    if (window == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null window");
        return nullptr;
    }
    return window->GetUIContent();
}

void UIAbility::OnCreate(Rosen::DisplayId displayId)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
}

void UIAbility::OnDestroy(Rosen::DisplayId displayId)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
}

void UIAbility::OnDisplayInfoChange(const sptr<IRemoteObject>& token, Rosen::DisplayId displayId, float density,
    Rosen::DisplayOrientation orientation)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "displayId: %{public}" PRIu64, displayId);
    // Get display
    auto display = Rosen::DisplayManager::GetInstance().GetDisplayById(displayId);
    if (!display) {
        TAG_LOGE(AAFwkTag::UIABILITY, "get display by displayId %{public}" PRIu64 " failed", displayId);
        return;
    }

    // Notify ResourceManager
    int32_t width = display->GetWidth();
    int32_t height = display->GetHeight();
    std::unique_ptr<Global::Resource::ResConfig> resConfig(Global::Resource::CreateResConfig());
    if (resConfig != nullptr) {
        auto resourceManager = GetResourceManager();
        if (resourceManager != nullptr) {
            resourceManager->GetResConfig(*resConfig);
            resConfig->SetScreenDensity(density);
            resConfig->SetDirection(AppExecFwk::ConvertDirection(height, width));
            resourceManager->UpdateResConfig(*resConfig);
            TAG_LOGD(AAFwkTag::UIABILITY, "notify resourceManager, density: %{public}f, direction: %{public}d",
                resConfig->GetScreenDensity(), resConfig->GetDirection());
        }
    }

    // Notify ability
    Configuration newConfig;
    newConfig.AddItem(
        displayId, AppExecFwk::ConfigurationInner::APPLICATION_DIRECTION, AppExecFwk::GetDirectionStr(height, width));
    newConfig.AddItem(
        displayId, AppExecFwk::ConfigurationInner::APPLICATION_DENSITYDPI, AppExecFwk::GetDensityStr(density));

    if (application_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null application_");
        return;
    }

    OnChangeForUpdateConfiguration(newConfig);
    TAG_LOGD(AAFwkTag::UIABILITY, "end");
}

void UIAbility::OnChange(Rosen::DisplayId displayId)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "displayId: %{public}" PRIu64 "", displayId);
    // Get display
    auto display = Rosen::DisplayManager::GetInstance().GetDisplayById(displayId);
    if (!display) {
        TAG_LOGE(AAFwkTag::UIABILITY, "get display by displayId %{public}" PRIu64 " failed", displayId);
        return;
    }

    // Notify ResourceManager
    float density = display->GetVirtualPixelRatio();
    int32_t width = display->GetWidth();
    int32_t height = display->GetHeight();
    std::unique_ptr<Global::Resource::ResConfig> resConfig(Global::Resource::CreateResConfig());
    if (resConfig != nullptr) {
        auto resourceManager = GetResourceManager();
        if (resourceManager != nullptr) {
            resourceManager->GetResConfig(*resConfig);
            resConfig->SetScreenDensity(density);
            resConfig->SetDirection(AppExecFwk::ConvertDirection(height, width));
            resourceManager->UpdateResConfig(*resConfig);
            TAG_LOGD(AAFwkTag::UIABILITY, "notify ResourceManager, density: %{public}f, direction: %{public}d",
                resConfig->GetScreenDensity(), resConfig->GetDirection());
        }
    }

    // Notify ability
    Configuration newConfig;
    newConfig.AddItem(
        displayId, AppExecFwk::ConfigurationInner::APPLICATION_DIRECTION, AppExecFwk::GetDirectionStr(height, width));
    newConfig.AddItem(
        displayId, AppExecFwk::ConfigurationInner::APPLICATION_DENSITYDPI, AppExecFwk::GetDensityStr(density));

    if (application_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null application_");
        return;
    }

    OnChangeForUpdateConfiguration(newConfig);
    TAG_LOGD(AAFwkTag::UIABILITY, "end");
}

void UIAbility::OnDisplayMove(Rosen::DisplayId from, Rosen::DisplayId to)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "from displayId %{public}" PRIu64 " to %{public}" PRIu64 "", from, to);
    auto display = Rosen::DisplayManager::GetInstance().GetDisplayById(to);
    if (!display) {
        TAG_LOGE(AAFwkTag::UIABILITY, "get display by displayId %{public}" PRIu64 " failed", to);
        return;
    }
    // Get new display config
    float density = display->GetVirtualPixelRatio();
    int32_t width = display->GetWidth();
    int32_t height = display->GetHeight();
    std::unique_ptr<Global::Resource::ResConfig> resConfig(Global::Resource::CreateResConfig());
    if (resConfig != nullptr) {
        auto resourceManager = GetResourceManager();
        if (resourceManager != nullptr) {
            resourceManager->GetResConfig(*resConfig);
            resConfig->SetScreenDensity(density);
            resConfig->SetDirection(AppExecFwk::ConvertDirection(height, width));
            resourceManager->UpdateResConfig(*resConfig);
            TAG_LOGD(AAFwkTag::UIABILITY,
                "Density: %{public}f, direction: %{public}d", resConfig->GetScreenDensity(), resConfig->GetDirection());
        }
    }
        UpdateConfiguration(to, density, width, height);
}

void UIAbility::UpdateConfiguration(Rosen::DisplayId to, float density, int32_t width, int32_t height)
{
    AppExecFwk::Configuration newConfig;
    newConfig.AddItem(AppExecFwk::ConfigurationInner::APPLICATION_DISPLAYID, std::to_string(to));
    newConfig.AddItem(
        to, AppExecFwk::ConfigurationInner::APPLICATION_DIRECTION, AppExecFwk::GetDirectionStr(height, width));
    newConfig.AddItem(to, AppExecFwk::ConfigurationInner::APPLICATION_DENSITYDPI, AppExecFwk::GetDensityStr(density));
    if (application_ == nullptr || handler_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null application_ or handler_");
        return;
    }
    std::vector<std::string> changeKeyV;
    auto configuration = application_->GetConfiguration();
    if (!configuration) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null configuration");
        return;
    }

    configuration->CompareDifferent(changeKeyV, newConfig);
    TAG_LOGD(AAFwkTag::UIABILITY, "changeKeyV size: %{public}zu", changeKeyV.size());
    if (!changeKeyV.empty()) {
        configuration->Merge(changeKeyV, newConfig);
        auto task = [abilityWptr = weak_from_this(), configuration = *configuration]() {
            auto ability = abilityWptr.lock();
            if (ability == nullptr) {
                TAG_LOGE(AAFwkTag::UIABILITY, "null ability");
                return;
            }
            ability->OnConfigurationUpdated(configuration);
        };
        handler_->PostTask(task);
    }
}

void UIAbility::RequestFocus(const AAFwk::Want &want)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
}

void UIAbility::InitWindow(int32_t displayId, sptr<Rosen::WindowOption> option)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
}

sptr<Rosen::WindowOption> UIAbility::GetWindowOption(const AAFwk::Want &want)
{
    auto option = sptr<Rosen::WindowOption>::MakeSptr();
    if (option == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null option");
        return nullptr;
    }
    auto windowMode = want.GetIntParam(
        AAFwk::Want::PARAM_RESV_WINDOW_MODE, AAFwk::AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_UNDEFINED);
    TAG_LOGD(AAFwkTag::UIABILITY, "window mode: %{public}d", windowMode);
    option->SetWindowMode(static_cast<Rosen::WindowMode>(windowMode));
    bool showOnLockScreen = false;
    if (abilityInfo_) {
        std::vector<AppExecFwk::CustomizeData> datas = abilityInfo_->metaData.customizeData;
        for (AppExecFwk::CustomizeData data : datas) {
            if (data.name == SHOW_ON_LOCK_SCREEN) {
                showOnLockScreen = true;
            }
        }
    }
    if (showOnLockScreen_ || showOnLockScreen) {
        TAG_LOGD(AAFwkTag::UIABILITY, "add window flag WINDOW_FLAG_SHOW_WHEN_LOCKED");
        option->AddWindowFlag(Rosen::WindowFlag::WINDOW_FLAG_SHOW_WHEN_LOCKED);
    }

    if (want.GetElement().GetBundleName() == LAUNCHER_BUNDLE_NAME &&
        want.GetElement().GetAbilityName() == LAUNCHER_ABILITY_NAME) {
        TAG_LOGD(AAFwkTag::UIABILITY, "set window type for launcher");
        option->SetWindowType(Rosen::WindowType::WINDOW_TYPE_DESKTOP);
    }
    return option;
}

void UIAbility::ContinuationRestore(const AAFwk::Want &want)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
}

void UIAbility::OnStartForSupportGraphics(const AAFwk::Want &want)
{
    if (abilityInfo_->type == AppExecFwk::AbilityType::PAGE) {
        int32_t displayId = want.GetIntParam(AAFwk::Want::PARAM_RESV_DISPLAY_ID,
            static_cast<int32_t>(Rosen::DISPLAY_ID_INVALID));
        if (displayId == static_cast<int32_t>(Rosen::DISPLAY_ID_INVALID)) {
            displayId = AAFwk::DisplayUtil::GetDefaultDisplayId();
        }
        TAG_LOGD(AAFwkTag::UIABILITY, "abilityName: %{public}s, displayId: %{public}d",
            abilityInfo_->name.c_str(), displayId);
#ifdef SUPPORT_SCREEN
        Rosen::DisplayManager::GetInstance().AddDisplayIdFromAms(displayId, token_);
#endif
        auto option = GetWindowOption(want);
        InitWindow(displayId, option);

        // Update resMgr, Configuration
        TAG_LOGD(AAFwkTag::UIABILITY, "displayId: %{public}d", displayId);
        auto display = Rosen::DisplayManager::GetInstance().GetDisplayById(displayId);
        if (display) {
            float density = 1.0f;
            int32_t width = 0;
            int32_t height = 0;
            if (auto displayInfo = display->GetDisplayInfo(); displayInfo != nullptr) {
                density = displayInfo->GetVirtualPixelRatio();
                width = displayInfo->GetWidth();
                height = displayInfo->GetHeight();
            }
            std::shared_ptr<AppExecFwk::Configuration> configuration = nullptr;
            if (application_) {
                configuration = application_->GetConfiguration();
            }
            if (configuration) {
                std::string direction = AppExecFwk::GetDirectionStr(height, width);
                configuration->AddItem(displayId, AppExecFwk::ConfigurationInner::APPLICATION_DIRECTION, direction);
                configuration->AddItem(displayId, AppExecFwk::ConfigurationInner::APPLICATION_DENSITYDPI,
                    AppExecFwk::GetDensityStr(density));
                configuration->AddItem(
                    AppExecFwk::ConfigurationInner::APPLICATION_DISPLAYID, std::to_string(displayId));
                UpdateContextConfiguration();
            }

            std::unique_ptr<Global::Resource::ResConfig> resConfig(Global::Resource::CreateResConfig());
            if (resConfig == nullptr) {
                TAG_LOGE(AAFwkTag::UIABILITY, "null resConfig");
                return;
            }
            auto resourceManager = GetResourceManager();
            if (resourceManager != nullptr) {
                resourceManager->GetResConfig(*resConfig);
                resConfig->SetScreenDensity(density);
                resConfig->SetDirection(AppExecFwk::ConvertDirection(height, width));
                resourceManager->UpdateResConfig(*resConfig);
                TAG_LOGD(AAFwkTag::UIABILITY, "density: %{public}f, direction: %{public}d",
                    resConfig->GetScreenDensity(), resConfig->GetDirection());
            }
        }
    }
}

void UIAbility::OnChangeForUpdateConfiguration(const AppExecFwk::Configuration &newConfig)
{
    if (application_ == nullptr || handler_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null application_ or handler_");
        return;
    }
    auto configuration = application_->GetConfiguration();
    if (!configuration) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null configuration");
        return;
    }

    std::vector<std::string> changeKeyV;
    configuration->CompareDifferent(changeKeyV, newConfig);
    TAG_LOGD(AAFwkTag::UIABILITY, "changeKeyV size: %{public}zu", changeKeyV.size());
    if (!changeKeyV.empty()) {
        configuration->Merge(changeKeyV, newConfig);
        auto task = [abilityWptr = weak_from_this(), configuration = *configuration]() {
            auto ability = abilityWptr.lock();
            if (ability == nullptr) {
                TAG_LOGE(AAFwkTag::UIABILITY, "null ability");
                return;
            }
            ability->OnConfigurationUpdated(configuration);
        };
        handler_->PostTask(task);
    }
}

void UIAbility::CallOnForegroundFunc(const AAFwk::Want &want)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
}

void UIAbility::ExecuteInsightIntentRepeateForeground(const AAFwk::Want &want,
    const std::shared_ptr<InsightIntentExecuteParam> &executeParam,
    std::unique_ptr<InsightIntentExecutorAsyncCallback> callback)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
}

void UIAbility::ExecuteInsightIntentMoveToForeground(const AAFwk::Want &want,
    const std::shared_ptr<InsightIntentExecuteParam> &executeParam,
    std::unique_ptr<InsightIntentExecutorAsyncCallback> callback)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
}

void UIAbility::ExecuteInsightIntentPage(const AAFwk::Want &want,
    const std::shared_ptr<InsightIntentExecuteParam> &executeParam,
    std::unique_ptr<InsightIntentExecutorAsyncCallback> callback)
{
    TAG_LOGD(AAFwkTag::INTENT, "execute insight intent page");
}

void UIAbility::ExecuteInsightIntentBackground(const AAFwk::Want &want,
    const std::shared_ptr<InsightIntentExecuteParam> &executeParam,
    std::unique_ptr<InsightIntentExecutorAsyncCallback> callback)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
}

int UIAbility::CreateModalUIExtension(const AAFwk::Want &want)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "call");
    auto abilityContextImpl = GetAbilityContext();
    if (abilityContextImpl == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null abilityContext");
        return ERR_INVALID_VALUE;
    }
    if (!handler_) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null handler_");
        return ERR_INVALID_VALUE;
    }
    std::weak_ptr<AbilityRuntime::AbilityContext> abilityContextImplWptr = abilityContextImpl;
    auto task = [abilityContextImplWptr, want]() {
        std::shared_ptr<AbilityRuntime::AbilityContext> abilityContextImplSptr = abilityContextImplWptr.lock();
        if (abilityContextImplSptr == nullptr) {
            TAG_LOGE(AAFwkTag::UIABILITY, "null abilityContextImpl");
            return;
        }
        (void)abilityContextImplSptr->CreateModalUIExtensionWithApp(want);
    };
    if (!handler_->PostTask(task)) {
        TAG_LOGE(AAFwkTag::UIABILITY, "PostTask failed");
        return ERR_INVALID_VALUE;
    }

    return ERR_OK;
}

void UIAbility::SetSessionToken(sptr<IRemoteObject> sessionToken, sptr<IRemoteObject> renderSession)
{
    std::lock_guard lock(sessionTokenMutex_);
    sessionToken_ = sessionToken;
    renderSession_ = renderSession;
    auto abilityContextImpl = GetAbilityContext();
    if (abilityContextImpl == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null abilityContext");
        return;
    }
    abilityContextImpl->SetWeakSessionToken(sessionToken);
}

void UIAbility::UpdateSessionToken(sptr<IRemoteObject> sessionToken)
{
    SetSessionToken(sessionToken, nullptr);
}

void UIAbility::EraseUIExtension(int32_t sessionId)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "call");
    auto abilityContextImpl = GetAbilityContext();
    if (abilityContextImpl == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null abilityContext");
        return;
    }
    abilityContextImpl->EraseUIExtension(sessionId);
}

void UIAbility::SetIdentityToken(const std::string &identityToken)
{
    identityToken_ = identityToken;
}

std::string UIAbility::GetIdentityToken() const
{
    return identityToken_;
}

bool UIAbility::CheckRecoveryEnabled()
{
    if (useAppSettedRecoveryValue_.load()) {
        TAG_LOGD(AAFwkTag::UIABILITY, "use app setted value");
        // Check in app recovery, here return true.
        return true;
    }

    return CheckDefaultRecoveryEnabled();
}

bool UIAbility::CheckDefaultRecoveryEnabled()
{
    if (abilityContext_ == nullptr) {
        TAG_LOGW(AAFwkTag::UIABILITY, "null context");
        return false;
    }

    return abilityContext_->GetRestoreEnabled();
}

bool UIAbility::IsStartByScb()
{
    if (setting_ == nullptr) {
        TAG_LOGW(AAFwkTag::UIABILITY, "null setting_");
        return false;
    }

    auto value = setting_->GetProperty(AppExecFwk::AbilityStartSetting::IS_START_BY_SCB_KEY);
    if (value == "true") {
        TAG_LOGD(AAFwkTag::UIABILITY, "start by scb");
        return true;
    }

    return false;
}
#endif
} // namespace AbilityRuntime
} // namespace OHOS
