/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "cj_ui_extension_base.h"

#include <type_traits>
#include <vector>

#include "ability_info.h"
#include "ability_manager_client.h"
#include "cj_application_context.h"
#include "cj_common_ffi.h"
#include "cj_extension_common.h"
#include "cj_insight_intent_executor_info.h"
#include "cj_insight_intent_executor_mgr.h"
#include "cj_runtime.h"
#include "configuration_utils.h"
#include "connection_manager.h"
#include "context.h"
#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "hitrace_meter.h"
#include "int_wrapper.h"
#include "ui_extension_window_command.h"
#include "want_params_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
CJUIExtensionBase::CJUIExtensionBase(const std::unique_ptr<Runtime> &runtime)
    : cjRuntime_(static_cast<CJRuntime&>(*runtime))
{
    abilityResultListeners_ = std::make_shared<AbilityResultListeners>();
}

CJUIExtensionBase::~CJUIExtensionBase()
{
    TAG_LOGD(AAFwkTag::UI_EXT, "destructor");
    cjObj.Destroy();
}

std::shared_ptr<ExtensionCommon> CJUIExtensionBase::Init(const std::shared_ptr<AbilityLocalRecord> &record,
    const std::shared_ptr<OHOSApplication> &application, std::shared_ptr<AbilityHandler> &handler,
    const sptr<IRemoteObject> &token)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UI_EXT, "called");
    if (abilityInfo_ == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null abilityInfo");
        return nullptr;
    }

    int32_t ret = cjObj.Init(abilityInfo_->name, extType_, this);
    if (ret != 0) {
        TAG_LOGE(AAFwkTag::UI_EXT, "cjUIExtAbility Init failed");
        return nullptr;
    }

    handler_ = handler;
    RegisterDisplayInfoChangedListener();

    return CJExtensionCommon::Create(cjObj);
}

void CJUIExtensionBase::OnStart(const AAFwk::Want &want, AAFwk::LaunchParam &launchParam,
    sptr<AAFwk::SessionInfo> sessionInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UI_EXT, "called");
#ifdef SUPPORT_GRAPHICS
    if (context_ != nullptr && sessionInfo != nullptr) {
        auto configUtils = std::make_shared<ConfigurationUtils>();
        configUtils->InitDisplayConfig(context_->GetConfiguration(), context_->GetResourceManager(),
            sessionInfo->displayId, sessionInfo->density, sessionInfo->orientation);
    }
#endif // SUPPORT_GRAPHICS

    if (InsightIntentExecuteParam::IsInsightIntentExecute(want)) {
        launchParam.launchReason = AAFwk::LaunchReason::LAUNCHREASON_INSIGHT_INTENT;
    }
    cjObj.OnCreate(want, launchParam);
}

void CJUIExtensionBase::OnStop()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UI_EXT, "called");
    cjObj.OnDestroy();

#ifdef SUPPORT_GRAPHICS
    UnregisterDisplayInfoChangedListener();
#endif // SUPPORT_GRAPHICS

    OnStopCallBack();
}

void CJUIExtensionBase::OnStop(AppExecFwk::AbilityTransactionCallbackInfo<> *callbackInfo, bool &isAsyncCallback)
{
    (void)callbackInfo;
    isAsyncCallback = false;
    OnStop();
    return;
}

void CJUIExtensionBase::OnStopCallBack()
{
    if (context_ == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null context");
        return;
    }
    auto ret = ConnectionManager::GetInstance().DisconnectCaller(context_->GetToken());
    if (ret) {
        ConnectionManager::GetInstance().ReportConnectionLeakEvent(getpid(), gettid());
        TAG_LOGD(AAFwkTag::UI_EXT, "service connection not disconnected");
    }

    auto applicationContext = Context::GetApplicationContext();
    if (applicationContext == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null application context");
        return;
    }

    auto appContext = ApplicationContextCJ::CJApplicationContext::GetCJApplicationContext(applicationContext);
    if (appContext != nullptr) {
        appContext->DispatchOnAbilityDestroy(cjObj.GetID());
    }
}

void CJUIExtensionBase::OnCommand(const AAFwk::Want &want, bool restart, int32_t startId)
{
    // do nothing
}

void CJUIExtensionBase::OnCommandWindow(
    const AAFwk::Want &want, const sptr<AAFwk::SessionInfo> &sessionInfo, AAFwk::WindowCommand winCmd)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UI_EXT, "called");
    if (sessionInfo == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null sessionInfo");
        return;
    }
    if (InsightIntentExecuteParam::IsInsightIntentExecute(want) && winCmd == AAFwk::WIN_CMD_FOREGROUND) {
        bool finish = ForegroundWindowWithInsightIntent(want, sessionInfo, false);
        if (finish) {
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

bool CJUIExtensionBase::ForegroundWindowWithInsightIntent(const AAFwk::Want &want,
    const sptr<AAFwk::SessionInfo> &sessionInfo, bool needForeground)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UI_EXT, "called");
    if (!HandleSessionCreate(want, sessionInfo)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "HandleSessionCreate failed");
        return false;
    }

    std::unique_ptr<InsightIntentExecutorAsyncCallback> executorCallback = nullptr;
    executorCallback.reset(InsightIntentExecutorAsyncCallback::Create());
    if (executorCallback == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null executorCallback");
        return false;
    }
    executorCallback->Push(
        [weak = weak_from_this(), sessionInfo, needForeground](AppExecFwk::InsightIntentExecuteResult result) {
            TAG_LOGD(AAFwkTag::UI_EXT, "Begin UI extension transaction callback");
            auto extension = weak.lock();
            if (extension == nullptr) {
                TAG_LOGE(AAFwkTag::UI_EXT, "null extension");
                return;
            }

            extension->PostInsightIntentExecuted(sessionInfo, result, needForeground);
        });

    CJInsightIntentExecutorInfo executorInfo;
    std::shared_ptr<AppExecFwk::AbilityInfo> abilityInfo = context_->GetAbilityInfo();
    if (abilityInfo != nullptr) {
        executorInfo.hapPath = abilityInfo->hapPath;
        executorInfo.windowMode = abilityInfo->compileMode == AppExecFwk::CompileMode::ES_MODULE;
    }
    executorInfo.token = context_->GetToken();
    executorInfo.pageLoader.sessionPageLoader = contentSessions_[sessionInfo->uiExtensionComponentId]->GetID();
    executorInfo.executeParam = std::make_shared<InsightIntentExecuteParam>();
    InsightIntentExecuteParam::GenerateFromWant(want, *executorInfo.executeParam);
    executorInfo.executeParam->executeMode_ = UI_EXTENSION_ABILITY;
    executorInfo.srcEntry = want.GetStringParam(INSIGHT_INTENT_SRC_ENTRY);
    TAG_LOGD(AAFwkTag::UI_EXT, "executorInfo, insightIntentId: %{public}" PRIu64,
        executorInfo.executeParam->insightIntentId_);
    int32_t ret = DelayedSingleton<CJInsightIntentExecutorMgr>::GetInstance()->ExecuteInsightIntent(
        cjRuntime_, executorInfo, std::move(executorCallback));
    if (!ret) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Execute insight intent failed");
        return false;
    }
    TAG_LOGD(AAFwkTag::UI_EXT, "end");
    return true;
}

void CJUIExtensionBase::ForegroundWindow(const AAFwk::Want &want, const sptr<AAFwk::SessionInfo> &sessionInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (!HandleSessionCreate(want, sessionInfo)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "HandleSessionCreate failed");
        return;
    }
    TAG_LOGD(AAFwkTag::UI_EXT, "UIExtension component id: %{public}" PRId64, sessionInfo->uiExtensionComponentId);
    auto componentId = sessionInfo->uiExtensionComponentId;
    auto &uiWindow = uiWindowMap_[componentId];
    if (uiWindow) {
        HITRACE_METER_NAME(HITRACE_TAG_APP, "Rosen::Window::show");
        uiWindow->Show();
        foregroundWindows_.emplace(componentId);
    }
}

void CJUIExtensionBase::BackgroundWindow(const sptr<AAFwk::SessionInfo> &sessionInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (sessionInfo == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Invalid sessionInfo");
        return;
    }

    TAG_LOGD(AAFwkTag::UI_EXT, "UIExtension component id: %{public}" PRId64, sessionInfo->uiExtensionComponentId);
    auto componentId = sessionInfo->uiExtensionComponentId;
    if (uiWindowMap_.find(componentId) == uiWindowMap_.end()) {
        TAG_LOGE(AAFwkTag::UI_EXT, "not find uiWindow");
        return;
    }
    auto &uiWindow = uiWindowMap_[componentId];
    if (uiWindow) {
        uiWindow->Hide();
        foregroundWindows_.erase(componentId);
    }
}

void CJUIExtensionBase::DestroyWindow(const sptr<AAFwk::SessionInfo> &sessionInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (sessionInfo == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Invalid sessionInfo");
        return;
    }

    TAG_LOGD(AAFwkTag::UI_EXT, "UIExtension component id: %{public}" PRId64, sessionInfo->uiExtensionComponentId);
    auto componentId = sessionInfo->uiExtensionComponentId;
    if (uiWindowMap_.find(componentId) == uiWindowMap_.end()) {
        TAG_LOGE(AAFwkTag::UI_EXT, "not find uiWindow");
        return;
    }
    if (contentSessions_.find(componentId) != contentSessions_.end() && contentSessions_[componentId] != nullptr) {
        cjObj.OnSessionDestroy(contentSessions_[componentId]->GetID());
    }
    auto &uiWindow = uiWindowMap_[componentId];
    if (uiWindow) {
        uiWindow->Destroy();
    }
    uiWindowMap_.erase(componentId);
    foregroundWindows_.erase(componentId);
    contentSessions_.erase(componentId);
    if (abilityResultListeners_) {
        abilityResultListeners_->RemoveListener(componentId);
    }
}

void CJUIExtensionBase::OnCommandWindowDone(const sptr<AAFwk::SessionInfo> &sessionInfo, AAFwk::WindowCommand winCmd)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UI_EXT, "called");
    if (context_ == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null context");
        return;
    }
    AAFwk::AbilityCommand abilityCmd;
    if (uiWindowMap_.empty()) {
        abilityCmd = AAFwk::ABILITY_CMD_DESTROY;
    } else if (foregroundWindows_.empty()) {
        abilityCmd = AAFwk::ABILITY_CMD_BACKGROUND;
    } else {
        abilityCmd = AAFwk::ABILITY_CMD_FOREGROUND;
    }
    AAFwk::AbilityManagerClient::GetInstance()->ScheduleCommandAbilityWindowDone(
        context_->GetToken(), sessionInfo, winCmd, abilityCmd);
    TAG_LOGD(AAFwkTag::UI_EXT, "end");
}

bool CJUIExtensionBase::HandleSessionCreate(const AAFwk::Want &want, const sptr<AAFwk::SessionInfo> &sessionInfo)
{
    if (sessionInfo == nullptr || sessionInfo->uiExtensionComponentId == 0) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Invalid sessionInfo");
        return false;
    }

    TAG_LOGD(AAFwkTag::UI_EXT, "UIExtension component id: %{public}" PRId64 ", element: %{public}s",
        sessionInfo->uiExtensionComponentId, want.GetElement().GetURI().c_str());
    if (sessionInfo == nullptr || sessionInfo->uiExtensionComponentId == 0) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Invalid sessionInfo");
        return false;
    }
    auto componentId = sessionInfo->uiExtensionComponentId;
    if (uiWindowMap_.find(componentId) == uiWindowMap_.end()) {
        if (context_ == nullptr || context_->GetAbilityInfo() == nullptr) {
            TAG_LOGE(AAFwkTag::UI_EXT, "null context");
            return false;
        }
        auto option = sptr<Rosen::WindowOption>::MakeSptr();
        if (option == nullptr) {
            return false;
        }
        option->SetWindowName(context_->GetBundleName() + context_->GetAbilityInfo()->name);
        option->SetWindowType(Rosen::WindowType::WINDOW_TYPE_UI_EXTENSION);
        option->SetWindowSessionType(Rosen::WindowSessionType::EXTENSION_SESSION);
        option->SetParentId(sessionInfo->hostWindowId);
        option->SetRealParentId(sessionInfo->realHostWindowId);
        option->SetParentWindowType(static_cast<Rosen::WindowType>(sessionInfo->parentWindowType));
        option->SetUIExtensionUsage(static_cast<uint32_t>(sessionInfo->uiExtensionUsage));
        sptr<Rosen::Window> uiWindow;
        {
            HITRACE_METER_NAME(HITRACE_TAG_APP, "Rosen::Window::Create");
            option->SetDisplayId(sessionInfo->displayId);
            uiWindow = Rosen::Window::Create(option, context_, sessionInfo->sessionToken);
        }
        if (uiWindow == nullptr) {
            TAG_LOGE(AAFwkTag::UI_EXT, "null uiWindow");
            return false;
        }
        auto cjSession = CJUIExtensionContentSession::Create(sessionInfo, uiWindow, context_);
        contentSessions_.emplace(sessionInfo->uiExtensionComponentId, cjSession);
        cjObj.OnSessionCreate(want, cjSession->GetID());
        uiWindowMap_[componentId] = uiWindow;
#ifdef SUPPORT_GRAPHICS
        if (context_->GetWindow() == nullptr) {
            context_->SetWindow(uiWindow);
        }
#endif // SUPPORT_GRAPHICS
    }
    return true;
}

void CJUIExtensionBase::PostInsightIntentExecuted(const sptr<AAFwk::SessionInfo> &sessionInfo,
    const AppExecFwk::InsightIntentExecuteResult &result, bool needForeground)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UI_EXT, "Post insightintent executed");
    if (needForeground) {
        // If uiextensionability is started for the first time or need move background to foreground.
        cjObj.OnForeground();
    }

    OnInsightIntentExecuteDone(sessionInfo, result);

    if (needForeground) {
        // If need foreground, that means triggered by onForeground.
        TAG_LOGI(AAFwkTag::UI_EXT, "call abilityms");
        AAFwk::PacMap restoreData;
        AAFwk::AbilityManagerClient::GetInstance()->AbilityTransitionDone(token_, AAFwk::ABILITY_STATE_FOREGROUND_NEW,
            restoreData);
    } else {
        // If uiextensionability has displayed in the foreground.
        OnCommandWindowDone(sessionInfo, AAFwk::WIN_CMD_FOREGROUND);
    }
}

void CJUIExtensionBase::OnInsightIntentExecuteDone(const sptr<AAFwk::SessionInfo> &sessionInfo,
    const AppExecFwk::InsightIntentExecuteResult &result)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (sessionInfo == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Invalid sessionInfo");
        return;
    }

    TAG_LOGD(AAFwkTag::UI_EXT, "UIExtension component id: %{public}" PRId64, sessionInfo->uiExtensionComponentId);
    auto componentId = sessionInfo->uiExtensionComponentId;
    auto res = uiWindowMap_.find(componentId);
    if (res != uiWindowMap_.end() && res->second != nullptr) {
        WantParams params;
        params.SetParam(INSIGHT_INTENT_EXECUTE_RESULT_CODE, Integer::Box(result.innerErr));
        WantParams resultParams;
        resultParams.SetParam("code", Integer::Box(result.code));
        if (result.result != nullptr) {
            sptr<AAFwk::IWantParams> pWantParams = WantParamWrapper::Box(*result.result);
            if (pWantParams != nullptr) {
                resultParams.SetParam("result", pWantParams);
            }
        }
        sptr<AAFwk::IWantParams> pWantParams = WantParamWrapper::Box(resultParams);
        if (pWantParams != nullptr) {
            params.SetParam(INSIGHT_INTENT_EXECUTE_RESULT, pWantParams);
        }

        Rosen::WMError ret = res->second->TransferExtensionData(params);
        if (ret == Rosen::WMError::WM_OK) {
            TAG_LOGD(AAFwkTag::UI_EXT, "TransferExtensionData success");
        } else {
            TAG_LOGE(AAFwkTag::UI_EXT, "TransferExtensionData failed, ret=%{public}d", ret);
        }

        res->second->Show();
        foregroundWindows_.emplace(componentId);
    }
    TAG_LOGD(AAFwkTag::UI_EXT, "end");
}

void CJUIExtensionBase::OnConfigurationUpdated(const AppExecFwk::Configuration &configuration)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UI_EXT, "called");
    if (context_ == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null context");
        return;
    }

    auto configUtils = std::make_shared<ConfigurationUtils>();
    configUtils->UpdateGlobalConfig(configuration, context_->GetConfiguration(), context_->GetResourceManager());

    ConfigurationUpdated();
}

void CJUIExtensionBase::OnForeground(const Want &want, sptr<AAFwk::SessionInfo> sessionInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UI_EXT, "called");
    if (InsightIntentExecuteParam::IsInsightIntentExecute(want)) {
        bool finish = ForegroundWindowWithInsightIntent(want, sessionInfo, true);
        if (finish) {
            return;
        }
    }

    ForegroundWindow(want, sessionInfo);
    cjObj.OnForeground();
}

void CJUIExtensionBase::OnBackground()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UI_EXT, "called");
    cjObj.OnBackground();
}

void CJUIExtensionBase::Dump(const std::vector<std::string> &params, std::vector<std::string> &info)
{
    // do nothing
}

void CJUIExtensionBase::OnAbilityResult(int32_t requestCode, int32_t resultCode, const Want &resultData)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "called");
    if (context_ == nullptr) {
        TAG_LOGW(AAFwkTag::UI_EXT, "null context");
        return;
    }
    context_->OnAbilityResult(requestCode, resultCode, resultData);
    if (abilityResultListeners_ == nullptr) {
        TAG_LOGW(AAFwkTag::UI_EXT, "null abilityResultListeners");
        return;
    }
    abilityResultListeners_->OnAbilityResult(requestCode, resultCode, resultData);
}

void CJUIExtensionBase::SetAbilityInfo(const std::shared_ptr<AppExecFwk::AbilityInfo> &abilityInfo)
{
    abilityInfo_ = abilityInfo;
}

void CJUIExtensionBase::SetContext(const std::shared_ptr<UIExtensionContext> &context)
{
    context_ = context;
}

void CJUIExtensionBase::BindContext()
{
    // do nothing
}

void CJUIExtensionBase::ConfigurationUpdated()
{
    TAG_LOGD(AAFwkTag::UI_EXT, "begin");

    // Notify extension context
    if (context_ == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null context");
        return;
    }

    auto fullConfig = context_->GetConfiguration();
    if (fullConfig == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null configuration");
        return;
    }

    cjObj.OnConfigurationUpdate(fullConfig);
}

#ifdef SUPPORT_GRAPHICS
void CJUIExtensionBase::OnDisplayInfoChange(
    const sptr<IRemoteObject> &token, Rosen::DisplayId displayId, float density, Rosen::DisplayOrientation orientation)
{
    TAG_LOGI(AAFwkTag::UI_EXT, "displayId: %{public}" PRIu64 "", displayId);
    if (context_ == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null context");
        return;
    }

    auto contextConfig = context_->GetConfiguration();
    if (contextConfig == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null configuration");
        return;
    }

    TAG_LOGI(AAFwkTag::UI_EXT, "Config dump: %{public}s", contextConfig->GetName().c_str());
    auto configUtils = std::make_shared<ConfigurationUtils>();
    auto result = configUtils->UpdateDisplayConfig(
        contextConfig, context_->GetResourceManager(), displayId, density, orientation);
    TAG_LOGI(AAFwkTag::UI_EXT, "Config dump after update: %{public}s", contextConfig->GetName().c_str());
    if (result) {
        auto cjUiExtension = std::static_pointer_cast<CJUIExtensionBase>(shared_from_this());
        auto task = [cjUiExtension]() {
            if (cjUiExtension) {
                cjUiExtension->ConfigurationUpdated();
            }
        };
        if (handler_ != nullptr) {
            handler_->PostTask(task, "CJUIExtensionBase:OnChange");
        }
    }
}

void CJUIExtensionBase::RegisterDisplayInfoChangedListener()
{
    // register displayid change callback
    auto cjUiExtensionBase = std::static_pointer_cast<CJUIExtensionBase>(shared_from_this());
    cjUIExtensionBaseDisplayListener_ = sptr<CJUIExtensionBaseDisplayListener>::MakeSptr(cjUiExtensionBase);
    if (cjUIExtensionBaseDisplayListener_ == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null CJUIExtensionBaseDisplayListener");
        return;
    }
    if (context_ == nullptr || context_->GetToken() == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null context");
        return;
    }
    TAG_LOGI(AAFwkTag::UI_EXT, "RegisterDisplayInfoChangedListener");
    Rosen::WindowManager::GetInstance().RegisterDisplayInfoChangedListener(
        context_->GetToken(), cjUIExtensionBaseDisplayListener_);
}

void CJUIExtensionBase::UnregisterDisplayInfoChangedListener()
{
    if (context_ == nullptr || context_->GetToken() == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null context");
        return;
    }
    Rosen::WindowManager::GetInstance().UnregisterDisplayInfoChangedListener(
        context_->GetToken(), cjUIExtensionBaseDisplayListener_);
}
#endif // SUPPORT_GRAPHICS
} // namespace AbilityRuntime
} // namespace OHOS
