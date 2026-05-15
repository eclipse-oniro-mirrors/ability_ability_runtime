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
#include "ui_extension.h"

#include "ability_manager_client.h"
#include "application_configuration_manager.h"
#include "array_wrapper.h"
#include "configuration_utils.h"
#include "connection_manager.h"
#include "ets_ui_extension_instance.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "insight_intent_delay_result_callback_mgr.h"
#include "int_wrapper.h"
#include "js_ui_extension.h"
#include "runtime.h"
#include "string_wrapper.h"
#include "ui_extension_context.h"
#include "want_params_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
using namespace OHOS::AppExecFwk;
namespace {
    constexpr int ERROR_OK = 0;
    constexpr int ERR_INVALID_VALUE = -1;

    int32_t ProcessInsightIntentResult(const sptr<Rosen::Window>& window,
        const AppExecFwk::InsightIntentExecuteResult& result)
    {
        if (window == nullptr) {
            TAG_LOGE(AAFwkTag::UI_EXT, "null window");
            return ERR_INVALID_VALUE;
        }

        WantParams params;
        params.SetParam(INSIGHT_INTENT_EXECUTE_RESULT_CODE, AAFwk::Integer::Box(result.innerErr));

        Rosen::WMError ret = Rosen::WMError::WM_OK;
        if (result.isDecorator && result.result) {
            sptr<AAFwk::IWantParams> pWantParams = WantParamWrapper::Box(*result.result);
            params.SetParam(INSIGHT_INTENT_EXECUTE_RESULT, pWantParams);
            ret = window->TransferExtensionData(params);
        } else {
            WantParams resultParams;
            resultParams.SetParam("code", AAFwk::Integer::Box(result.code));
            if (result.result != nullptr) {
                sptr<AAFwk::IWantParams> pWantParams = WantParamWrapper::Box(*result.result);
                resultParams.SetParam("result", pWantParams);
            }
            auto size = result.uris.size();
            sptr<IArray> uriArray = new (std::nothrow) Array(size, g_IID_IString);
            if (uriArray == nullptr) {
                TAG_LOGE(AAFwkTag::UI_EXT, "new uriArray failed");
                return ERR_INVALID_VALUE;
            }
            for (std::size_t i = 0; i < size; i++) {
                uriArray->Set(i, String::Box(result.uris[i]));
            }
            resultParams.SetParam("uris", uriArray);
            resultParams.SetParam("flags", AAFwk::Integer::Box(result.flags));
            sptr<AAFwk::IWantParams> pWantParams = WantParamWrapper::Box(resultParams);
            params.SetParam(INSIGHT_INTENT_EXECUTE_RESULT, pWantParams);

            ret = window->TransferExtensionData(params);
        }
        
        if (ret == Rosen::WMError::WM_OK) {
            TAG_LOGD(AAFwkTag::UI_EXT, "TransferExtensionData success");
            return ERROR_OK;
        } else {
            TAG_LOGE(AAFwkTag::UI_EXT, "TransferExtensionData failed, ret=%{public}d", ret);
            return ERR_INVALID_VALUE;
        }
    }
}
UIExtension* UIExtension::Create(const std::unique_ptr<Runtime>& runtime)
{
    if (!runtime) {
        return new UIExtension();
    }
    TAG_LOGD(AAFwkTag::UI_EXT, "UIExtension Create runtime");
    switch (runtime->GetLanguage()) {
        case Runtime::Language::JS:
            return JsUIExtension::Create(runtime);
        case Runtime::Language::ETS:
            return CreateETSUIExtension(runtime);
        default:
            return new UIExtension();
    }
}

void UIExtension::Init(const std::shared_ptr<AbilityLocalRecord> &record,
    const std::shared_ptr<OHOSApplication> &application,
    std::shared_ptr<AbilityHandler> &handler,
    const sptr<IRemoteObject> &token)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "called");
    handler_ = handler;
    ExtensionBase<UIExtensionContext>::Init(record, application, handler, token);
}

std::shared_ptr<UIExtensionContext> UIExtension::CreateAndInitContext(
    const std::shared_ptr<AbilityLocalRecord> &record,
    const std::shared_ptr<OHOSApplication> &application,
    std::shared_ptr<AbilityHandler> &handler,
    const sptr<IRemoteObject> &token)
{
    std::shared_ptr<UIExtensionContext> context =
        ExtensionBase<UIExtensionContext>::CreateAndInitContext(record, application, handler, token);
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null context");
    }
    return context;
}

bool UIExtension::HandleSessionCreate(const AAFwk::Want &want, const sptr<AAFwk::SessionInfo> &sessionInfo)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "HandleSessionCreate called");
    return true;
}

void UIExtension::ForegroundWindow(const AAFwk::Want &want, const sptr<AAFwk::SessionInfo> &sessionInfo)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "ForegroundWindow called");
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (!HandleSessionCreate(want, sessionInfo)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "HandleSessionCreate failed");
        return;
    }
    std::lock_guard<std::mutex> lock(uiWindowMutex_);
    TAG_LOGI(AAFwkTag::UI_EXT, "Before window show UIExtcomponent id: %{public}" PRId64,
        sessionInfo->uiExtensionComponentId);
    auto componentId = sessionInfo->uiExtensionComponentId;
    auto& uiWindow = uiWindowMap_[componentId];
    if (uiWindow) {
        HITRACE_METER_NAME(HITRACE_TAG_APP, "Rosen::Window::show");
        uiWindow->Show();
        foregroundWindows_.emplace(componentId);
    }
    TAG_LOGD(AAFwkTag::UI_EXT, "end");
}

void UIExtension::BackgroundWindow(const sptr<AAFwk::SessionInfo> &sessionInfo)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "BackgroundWindow called");
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (sessionInfo == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Invalid sessionInfo");
        return;
    }
    std::lock_guard<std::mutex> lock(uiWindowMutex_);
    auto componentId = sessionInfo->uiExtensionComponentId;
    if (uiWindowMap_.find(componentId) == uiWindowMap_.end()) {
        TAG_LOGE(AAFwkTag::UI_EXT, "find uiWindow failed");
        return;
    }
    auto& uiWindow = uiWindowMap_[componentId];
    TAG_LOGI(AAFwkTag::UI_EXT, "Befor window hide UIExtcomponent id: %{public}" PRId64,
        sessionInfo->uiExtensionComponentId);
    if (uiWindow) {
        uiWindow->Hide();
        foregroundWindows_.erase(componentId);
    }
    TAG_LOGD(AAFwkTag::UI_EXT, "end");
}

void UIExtension::DestroyWindow(const sptr<AAFwk::SessionInfo> &sessionInfo)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "DestroyWindow called");
}

bool UIExtension::ForegroundWindowWithInsightIntent(
    const AAFwk::Want &want, const sptr<AAFwk::SessionInfo> &sessionInfo, bool needForeground)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "ForegroundWindowWithInsightIntent called");
    return true;
}

void UIExtension::OnStopCallBack()
{
    auto context = GetContext();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null context");
        return;
    }
    bool ret = ConnectionManager::GetInstance().DisconnectCaller(context->GetToken());
    if (ret) {
        ConnectionManager::GetInstance().ReportConnectionLeakEvent(getpid(), gettid());
        TAG_LOGD(AAFwkTag::UI_EXT, "The service connection is not disconnected");
    }
}

void UIExtension::OnCommand(const AAFwk::Want &want, bool restart, int startId)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "called");
    Extension::OnCommand(want, restart, startId);
}

void UIExtension::OnCommandWindow(const AAFwk::Want &want, const sptr<AAFwk::SessionInfo> &sessionInfo,
    AAFwk::WindowCommand winCmd)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (sessionInfo == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null sessionInfo");
        return;
    }
    TAG_LOGD(AAFwkTag::UI_EXT, "begin. persistentId: %{private}d, winCmd: %{public}d",
        sessionInfo->persistentId, winCmd);
    Extension::OnCommandWindow(want, sessionInfo, winCmd);
    if (InsightIntentExecuteParam::IsInsightIntentExecute(want) && winCmd == AAFwk::WIN_CMD_FOREGROUND) {
        if (ForegroundWindowWithInsightIntent(want, sessionInfo, false)) {
            return;
        }
    }
    switch (winCmd) {
        case AAFwk::WIN_CMD_FOREGROUND:
            ForegroundWindow(want, sessionInfo);
            break;
        case AAFwk::WIN_CMD_BACKGROUND:
            BackgroundWindow(sessionInfo);
            break;
        case AAFwk::WIN_CMD_DESTROY:
            DestroyWindow(sessionInfo);
            break;
        default:
            TAG_LOGD(AAFwkTag::UI_EXT, "unsupported cmd");
            break;
    }
    OnCommandWindowDone(sessionInfo, winCmd);
}

void UIExtension::OnCommandWindowDone(const sptr<AAFwk::SessionInfo> &sessionInfo, AAFwk::WindowCommand winCmd)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UI_EXT, "called");
    auto context = GetContext();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null context");
        return;
    }
    AAFwk::AbilityCommand abilityCmd;
    std::lock_guard<std::mutex> lock(uiWindowMutex_);
    if (uiWindowMap_.empty()) {
        abilityCmd = AAFwk::ABILITY_CMD_DESTROY;
    } else if (foregroundWindows_.empty()) {
        abilityCmd = AAFwk::ABILITY_CMD_BACKGROUND;
    } else {
        abilityCmd = AAFwk::ABILITY_CMD_FOREGROUND;
    }
    AAFwk::AbilityManagerClient::GetInstance()->ScheduleCommandAbilityWindowDone(
        context->GetToken(), sessionInfo, winCmd, abilityCmd);
    TAG_LOGD(AAFwkTag::UI_EXT, "end");
}

void UIExtension::OnInsightIntentExecuteDone(const sptr<AAFwk::SessionInfo> &sessionInfo,
    const AppExecFwk::InsightIntentExecuteResult &result)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (sessionInfo == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Invalid sessionInfo");
        return;
    }
    std::lock_guard<std::mutex> lock(uiWindowMutex_);
    TAG_LOGD(AAFwkTag::UI_EXT, "UIExtension component id: %{public}" PRId64, sessionInfo->uiExtensionComponentId);
    auto componentId = sessionInfo->uiExtensionComponentId;
    auto res = uiWindowMap_.find(componentId);
    if (res != uiWindowMap_.end() && res->second != nullptr) {
        if (!result.isNeedDelayResult) {
            ProcessInsightIntentResult(res->second, result);
        }
        res->second->Show();
        foregroundWindows_.emplace(componentId);
    }
    TAG_LOGD(AAFwkTag::UI_EXT, "end");
}

void UIExtension::RegisterUiExtensionDelayResultCallback(uint64_t intentId, const sptr<AAFwk::SessionInfo> &sessionInfo,
    bool isDecorator)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "intentId %{public}" PRIu64"", intentId);
    std::lock_guard<std::mutex> lock(uiWindowMutex_);
    auto componentId = sessionInfo->uiExtensionComponentId;
    auto res = uiWindowMap_.find(componentId);
    if (res != uiWindowMap_.end() && res->second != nullptr) {
        auto window = res->second;
        auto callback = [window](AppExecFwk::InsightIntentExecuteResult result) -> int32_t {
            return ProcessInsightIntentResult(window, result);
        };
        InsightIntentDelayResultCallbackMgr::GetInstance().AddDelayResultCallback(intentId, {callback, isDecorator});
    }
}

void UIExtension::OnConfigurationUpdated(const AppExecFwk::Configuration &configuration)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    Extension::OnConfigurationUpdated(configuration);
    TAG_LOGD(AAFwkTag::UI_EXT, "OnConfigurationUpdated called");
    auto context = GetContext();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null context");
        return;
    }
    auto abilityConfig = context->GetAbilityConfiguration();
    auto configUtils = std::make_shared<ConfigurationUtils>();
    if (abilityConfig != nullptr) {
        auto newConfig = configUtils->UpdateGlobalConfig(
            configuration, context->GetConfiguration(), abilityConfig, context->GetResourceManager());
        if (newConfig.GetItemSize() == 0) {
            return;
        }
        if (context->GetWindow()) {
            TAG_LOGI(AAFwkTag::UI_EXT, "newConfig: %{public}s", newConfig.GetName().c_str());
            auto diffConfiguration = std::make_shared<AppExecFwk::Configuration>(newConfig);
            context->GetWindow()->UpdateConfigurationForSpecified(diffConfiguration, context->GetResourceManager());
        }
    } else {
        auto configUtils = std::make_shared<ConfigurationUtils>();
        configUtils->UpdateGlobalConfig(configuration, context->GetConfiguration(), context->GetResourceManager());
    }
    ConfigurationUpdated();
}

void UIExtension::ConfigurationUpdated()
{
    TAG_LOGD(AAFwkTag::UI_EXT, "ConfigurationUpdated called");
}

void UIExtension::OnAbilityConfigurationUpdated(const AppExecFwk::Configuration &configuration)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    Extension::OnConfigurationUpdated(configuration);
    TAG_LOGD(AAFwkTag::UI_EXT, "OnAbilityConfigurationUpdated called");
    auto context = GetContext();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null context");
        return;
    }
    auto configUtils = std::make_shared<ConfigurationUtils>();
    configUtils->UpdateAbilityConfig(configuration, context->GetResourceManager());
    if (context->GetWindow()) {
        TAG_LOGI(AAFwkTag::UI_EXT, "newConfig: %{public}s", configuration.GetName().c_str());
        auto diffConfiguration = std::make_shared<AppExecFwk::Configuration>(configuration);
        context->GetWindow()->UpdateConfigurationForSpecified(diffConfiguration, context->GetResourceManager());
    }
}

void UIExtension::RegisterAbilityConfigUpdateCallback()
{
    auto context = GetContext();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null context");
        return;
    }
    auto uiExtensionAbility = std::static_pointer_cast<UIExtension>(shared_from_this());
    std::weak_ptr<UIExtension> abilityWptr = uiExtensionAbility;
    std::weak_ptr<UIExtensionContext> abilityContextWptr = context;
    context->RegisterAbilityConfigUpdateCallback(
        [abilityWptr, abilityContextWptr](AppExecFwk::Configuration &config) {
        std::shared_ptr<UIExtension> abilitySptr = abilityWptr.lock();
        std::shared_ptr<UIExtensionContext> abilityContext = abilityContextWptr.lock();
        if (abilitySptr == nullptr || abilityContext == nullptr || abilityContext->GetAbilityInfo() == nullptr) {
            TAG_LOGE(AAFwkTag::UI_EXT, "null abilitySptr or abilityContext or GetAbilityInfo");
            return;
        }
        if (abilityContext->GetAbilityConfiguration() == nullptr) {
            auto abilityModuleContext = abilityContext->CreateModuleContext(
                abilityContext->GetAbilityInfo()->moduleName);
            if (abilityModuleContext == nullptr) {
                TAG_LOGE(AAFwkTag::UI_EXT, "null abilityModuleContext");
                return;
            }
            auto abilityResourceMgr = abilityModuleContext->GetResourceManager();
            abilityContext->SetAbilityResourceManager(abilityResourceMgr);
            AbilityRuntime::ApplicationConfigurationManager::GetInstance().
                AddIgnoreContext(abilityContext, abilityResourceMgr);
            TAG_LOGE(AAFwkTag::UI_EXT, "%{public}zu",
                AbilityRuntime::ApplicationConfigurationManager::GetInstance().GetIgnoreContext().size());
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

        abilitySptr->OnAbilityConfigurationUpdated(config);
    });
}

int UIExtension::CreateModalUIExtension(const AAFwk::Want &want)
{
    auto context = GetContext();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "context is null");
        return ERR_INVALID_VALUE;
    }

    if (handler_ == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null handler_");
        return ERR_INVALID_VALUE;
    }

    auto uiExtensionContext = Context::ConvertTo<UIExtensionContext>(context);
    if (uiExtensionContext == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "uiExtensionContext is null");
        return ERR_INVALID_VALUE;
    }

    std::weak_ptr<UIExtensionContext> uiExtensionContextWptr = uiExtensionContext;
    auto task = [uiExtensionContextWptr, want]() {
        std::shared_ptr<UIExtensionContext> uiExtensionContextSptr = uiExtensionContextWptr.lock();
        if (uiExtensionContextSptr == nullptr) {
            TAG_LOGE(AAFwkTag::UI_EXT, "null uiExtensionContext in task");
            return;
        }
        (void)uiExtensionContextSptr->CreateModalUIExtensionWithApp(want);
    };
    if (!handler_->PostTask(task, "UIExtension:CreateModalUIExtensionWithApp")) {
        TAG_LOGE(AAFwkTag::UI_EXT, "PostTask failed");
        return ERR_INVALID_VALUE;
    }

    return ERR_OK;
}
}
}
