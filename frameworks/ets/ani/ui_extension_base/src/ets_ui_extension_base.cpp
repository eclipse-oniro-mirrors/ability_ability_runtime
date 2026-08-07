/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "ets_ui_extension_base.h"

#include <type_traits>
#include <vector>

#include "ani.h"
#include "ability_info.h"
#include "ability_manager_client.h"
#include "ani_common_configuration.h"
#include "ani_common_util.h"
#include "ani_common_want.h"
#include "remote_object_taihe_ani.h"
#include "application_configuration_manager.h"
#include "array_wrapper.h"
#include "configuration_utils.h"
#include "connection_manager.h"
#include "context.h"
#include "ets_ability_lifecycle_callback.h"
#include "ets_data_struct_converter.h"
#include "ets_extension_common.h"
#include "ets_extension_context.h"
#include "ets_runtime.h"
#include "ets_ui_extension_content_session.h"
#include "ets_ui_extension_context.h"
#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "hitrace_meter.h"
#include "insight_intent_executor_mgr.h"
#include "int_wrapper.h"
#include "ohos_application.h"
#include "string_wrapper.h"
#include "ui_extension_window_command.h"
#include "want_params_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr const char* UIEXTENSION_CLASS_NAME = "@ohos.app.ability.UIExtensionAbility.UIExtensionAbility";
constexpr const char *UIEXT_ONCREATE_SIGNATURE = "C{@ohos.app.ability.AbilityConstant.AbilityConstant.LaunchParam}:";
constexpr const char *UIEXT_ONSESSIONDESTROY_SIGNATURE =
    "C{@ohos.app.ability.UIExtensionContentSession.UIExtensionContentSession}:";
std::mutex g_mtxWindowMapRecord;
std::mutex g_mtxSeesionRecord;
std::mutex g_mtxForegroundWindowsRecord;

void OnDestroyPromiseCallback(ani_env* env, ani_object aniObj)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "OnDestroyPromiseCallback called");
    if (env == nullptr || aniObj == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null env or null aniObj");
        return;
    }
    ani_long destroyCallbackPoint = 0;
    ani_status status = ANI_ERROR;
    if ((status = env->Object_GetFieldByName_Long(aniObj, "destroyCallbackPoint", &destroyCallbackPoint)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "destroyCallbackPoint GetField status: %{public}d", status);
        return;
    }
    auto *callbackInfo = reinterpret_cast<AppExecFwk::AbilityTransactionCallbackInfo<> *>(destroyCallbackPoint);
    if (callbackInfo == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null callbackInfo");
        return;
    }
    callbackInfo->Call();
    AppExecFwk::AbilityTransactionCallbackInfo<>::Destroy(callbackInfo);

    if ((status = env->Object_SetFieldByName_Long(aniObj, "destroyCallbackPoint",
        static_cast<ani_long>(0))) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "destroyCallbackPoint SetField status: %{public}d", status);
        return;
    }
}
} // namespace

EtsUIExtensionBase::EtsUIExtensionBase(const std::unique_ptr<Runtime> &runtime)
    : etsRuntime_(static_cast<ETSRuntime&>(*runtime))
{
    abilityResultListeners_ = std::make_shared<EtsAbilityResultListeners>();
}

EtsUIExtensionBase::~EtsUIExtensionBase()
{
    TAG_LOGD(AAFwkTag::UI_EXT, "destructor");
    auto env = etsRuntime_.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null env");
        return;
    }
    ani_status status = ANI_ERROR;
    for (auto &item : contentSessions_) {
        if ((status = env->GlobalReference_Delete(item.second)) != ANI_OK) {
            TAG_LOGE(AAFwkTag::UI_EXT, "GlobalReference_Delete status: %{public}d", status);
            continue;
        }
    }
    contentSessions_.clear();
    if (shellContextRef_ && shellContextRef_->aniRef) {
        env->GlobalReference_Delete(shellContextRef_->aniRef);
    }
}

std::shared_ptr<ExtensionCommon> EtsUIExtensionBase::Init(const std::shared_ptr<AbilityLocalRecord> &record,
    const std::shared_ptr<OHOSApplication> &application, std::shared_ptr<AbilityHandler> &handler,
    const sptr<IRemoteObject> &token)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UI_EXT, "called");
    if (abilityInfo_ == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null abilityInfo");
        return nullptr;
    }
    if (abilityInfo_->srcEntrance.empty()) {
        TAG_LOGE(AAFwkTag::UI_EXT, "empty abilityInfo srcEntrance");
        return nullptr;
    }

    RegisterAbilityConfigUpdateCallback();

    if (record != nullptr) {
        token_ = record->GetToken();
    }
    std::string srcPath(abilityInfo_->moduleName + "/");
    srcPath.append(abilityInfo_->srcEntrance);
    auto pos = srcPath.rfind(".");
    if (pos != std::string::npos) {
        srcPath.erase(pos);
        srcPath.append(".abc");
    }
    std::string moduleName(abilityInfo_->moduleName);
    moduleName.append("::").append(abilityInfo_->name);
    etsObj_ = etsRuntime_.LoadModule(
        moduleName, srcPath, abilityInfo_->hapPath, abilityInfo_->compileMode == AppExecFwk::CompileMode::ES_MODULE,
        false, abilityInfo_->srcEntrance);
    if (etsObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null etsObj_");
        return nullptr;
    }
    if (!BindNativeMethods()) {
        TAG_LOGE(AAFwkTag::UI_EXT, "BindNativeMethods failed");
        return nullptr;
    }
    BindContext();
    handler_ = handler;
#ifdef SUPPORT_GRAPHICS
    RegisterDisplayInfoChangedListener();
#endif
    return EtsExtensionCommon::Create(etsRuntime_,
        static_cast<AppExecFwk::ETSNativeReference&>(*etsObj_), shellContextRef_);
}

void EtsUIExtensionBase::RegisterAbilityConfigUpdateCallback()
{
    if (context_ == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null context_");
        return;
    }
    auto uiExtensionAbility = std::static_pointer_cast<EtsUIExtensionBase>(shared_from_this());
    std::weak_ptr<EtsUIExtensionBase> abilityWptr = uiExtensionAbility;
    context_->RegisterAbilityConfigUpdateCallback(
        [abilityWptr, abilityContext = context_](AppExecFwk::Configuration &config) {
        std::shared_ptr<EtsUIExtensionBase> abilitySptr = abilityWptr.lock();
        if (abilitySptr == nullptr) {
            TAG_LOGE(AAFwkTag::UI_EXT, "null abilitySptr");
            return;
        }
        if (abilityContext == nullptr || abilityContext->GetAbilityInfo() == nullptr) {
            TAG_LOGE(AAFwkTag::UI_EXT, "null abilityContext or null GetAbilityInfo");
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
            TAG_LOGD(AAFwkTag::UI_EXT, "%{public}zu",
                AbilityRuntime::ApplicationConfigurationManager::GetInstance().GetIgnoreContext().size());
        }
        abilityContext->SetAbilityConfiguration(config);
        if (config.GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_COLORMODE).
            compare(AppExecFwk::ConfigurationInner::COLOR_MODE_AUTO) == 0) {
            config.AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_COLORMODE,
                ApplicationConfigurationManager::GetInstance().GetColorMode());

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

bool EtsUIExtensionBase::BindNativeMethods()
{
    auto env = etsRuntime_.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null env");
        return false;
    }
    std::array functions = {
        ani_native_function { "nativeOnDestroyCallback", ":", reinterpret_cast<void*>(OnDestroyPromiseCallback) },
    };
    ani_class cls {};
    ani_status status = env->FindClass(UIEXTENSION_CLASS_NAME, &cls);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "FindClass failed status: %{public}d", status);
        return false;
    }
    if ((status = env->Class_BindNativeMethods(cls, functions.data(), functions.size())) != ANI_OK
        && status != ANI_ALREADY_BINDED) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Class_BindNativeMethods status: %{public}d", status);
        return false;
    }
    return true;
}

void EtsUIExtensionBase::BindContext()
{
    auto env = etsRuntime_.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "env is null");
        return;
    }
    if (context_ == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "context null");
        return;
    }
    ani_object contextObj = CreateEtsUIExtensionContext(env, context_);
    if (contextObj == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null contextObj");
        return;
    }
    ani_field contextField = nullptr;
    auto status = env->Class_FindField(etsObj_->aniCls, "context", &contextField);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "status: %{public}d", status);
        return;
    }
    ani_ref contextRef = nullptr;
    if ((status = env->GlobalReference_Create(contextObj, &contextRef)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "status: %{public}d", status);
        return;
    }
    if ((status = env->Object_SetField_Ref(etsObj_->aniObj, contextField, contextRef)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "status: %{public}d", status);
    }
    shellContextRef_ = std::make_shared<AppExecFwk::ETSNativeReference>();
    shellContextRef_->aniObj = contextObj;
    shellContextRef_->aniRef = contextRef;
}

void EtsUIExtensionBase::OnStart(
    const AAFwk::Want &want, AAFwk::LaunchParam &launchParam, sptr<AAFwk::SessionInfo> sessionInfo)
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
    auto env = etsRuntime_.GetAniEnv();
    if (!env) {
        TAG_LOGE(AAFwkTag::UI_EXT, "env not found Ability.ets");
        return;
    }
    if (context_ != nullptr) {
        EtsExtensionContext::ConfigurationUpdated(env, shellContextRef_, context_->GetConfiguration());
    }

    if (InsightIntentExecuteParam::IsInsightIntentExecute(want)) {
        launchParam.launchReason = AAFwk::LaunchReason::LAUNCHREASON_INSIGHT_INTENT;
    }
    ani_object launchParamObj = nullptr;
    if (!WrapLaunchParam(env, launchParam, launchParamObj)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "WrapLaunchParam failed");
        return;
    }
    ani_ref wantObj = OHOS::AppExecFwk::WrapWant(env, want);
    if (wantObj == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null wantObj");
        return;
    }
    CallObjectMethod(false, "onCreate", UIEXT_ONCREATE_SIGNATURE, launchParamObj, wantObj);
}

void EtsUIExtensionBase::OnStop()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UI_EXT, "called");
    if (context_) {
        TAG_LOGD(AAFwkTag::UI_EXT, "set terminating true");
        context_->SetTerminating(true);
    }
    CallObjectMethod(false, "onDestroy", nullptr);
    ApplicationConfigurationManager::GetInstance().DeleteIgnoreContext(context_);
    TAG_LOGI(AAFwkTag::UI_EXT, "GetIgnoreContext size %{public}zu",
        AbilityRuntime::ApplicationConfigurationManager::GetInstance().GetIgnoreContext().size());
#ifdef SUPPORT_GRAPHICS
    UnregisterDisplayInfoChangedListener();
#endif // SUPPORT_GRAPHICS
    OnStopCallBack();
    TAG_LOGD(AAFwkTag::UI_EXT, "end");
}

void EtsUIExtensionBase::OnStop(AppExecFwk::AbilityTransactionCallbackInfo<> *callbackInfo, bool &isAsyncCallback)
{
    if (callbackInfo == nullptr) {
        isAsyncCallback = false;
        OnStop();
        return;
    }
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UI_EXT, "OnStop begin");
    if (context_) {
        TAG_LOGD(AAFwkTag::UI_EXT, "set terminating true");
        context_->SetTerminating(true);
    }
    auto asyncCallback = [extensionWeakPtr = weak_from_this()]() {
        auto EtsUIExtensionBase = extensionWeakPtr.lock();
        if (EtsUIExtensionBase == nullptr) {
            TAG_LOGE(AAFwkTag::UI_EXT, "null extension");
            return;
        }
        EtsUIExtensionBase->OnStopCallBack();
    };
    callbackInfo->Push(asyncCallback);
    ani_long destroyCallbackPoint = (ani_long)callbackInfo;
    ani_status status = ANI_ERROR;
    auto env = etsRuntime_.GetAniEnv();
    if (env == nullptr || etsObj_ == nullptr) {
        isAsyncCallback = false;
        OnStop();
        return;
    }
    if ((status = env->Object_SetFieldByName_Long(etsObj_->aniObj, "destroyCallbackPoint",
        destroyCallbackPoint)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "status: %{public}d", status);
    }
    isAsyncCallback = CallObjectMethod(true, "callOnDestroy", ":z");
    if (!isAsyncCallback) {
        TAG_LOGE(AAFwkTag::UI_EXT, "call promise failed");
        OnStopCallBack();
    }
    TAG_LOGD(AAFwkTag::UI_EXT, "end");
}

void EtsUIExtensionBase::OnStopCallBack()
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
    if (applicationContext != nullptr) {
        EtsAbilityLifecycleCallbackArgs ability(etsObj_);
        applicationContext->DispatchOnAbilityDestroy(ability);
    }
}

void EtsUIExtensionBase::OnCommandWindow(
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

void EtsUIExtensionBase::ForegroundWindowInitInsightIntentExecutorInfo(const AAFwk::Want &want,
    const sptr<AAFwk::SessionInfo> &sessionInfo, InsightIntentExecutorInfo &executorInfo)
{
    if (sessionInfo == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null sessionInfo");
        return;
    }
    if (context_ == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null context_");
        return;
    }
    std::shared_ptr<AppExecFwk::AbilityInfo> abilityInfo = context_->GetAbilityInfo();
    if (abilityInfo != nullptr) {
        executorInfo.hapPath = abilityInfo->hapPath;
        executorInfo.windowMode = abilityInfo->compileMode == AppExecFwk::CompileMode::ES_MODULE;
    }
    executorInfo.token = context_->GetToken();
    executorInfo.etsPageLoader = reinterpret_cast<void *>(contentSessions_[sessionInfo->uiExtensionComponentId]);
    executorInfo.executeParam = std::make_shared<InsightIntentExecuteParam>();
    InsightIntentExecuteParam::GenerateFromWant(want, *executorInfo.executeParam);
    executorInfo.executeParam->executeMode_ = UI_EXTENSION_ABILITY;
    executorInfo.srcEntry = want.GetStringParam(INSIGHT_INTENT_SRC_ENTRY);
    executorInfo.decoratorClass = want.GetStringParam(INSIGHT_INTENT_DECORATOR_CLASS);
    TAG_LOGD(AAFwkTag::UI_EXT, "executorInfo, insightIntentId: %{public}" PRIu64,
        executorInfo.executeParam->insightIntentId_);
    return;
}

bool EtsUIExtensionBase::ForegroundWindowWithInsightIntent(const AAFwk::Want &want,
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
        [weak = weak_from_this(), sessionInfo, needForeground, want](AppExecFwk::InsightIntentExecuteResult result) {
            TAG_LOGD(AAFwkTag::UI_EXT, "Begin UI extension transaction callback");
            auto extension = weak.lock();
            if (extension == nullptr) {
                TAG_LOGE(AAFwkTag::UI_EXT, "null extension");
                return;
            }
            InsightIntentExecuteParam executeParam;
            InsightIntentExecuteParam::GenerateFromWant(want, executeParam);
            if (result.uris.size() > 0 || result.interactionInfo != nullptr) {
                extension->ExecuteInsightIntentDone(executeParam.insightIntentId_, result);
            }
            extension->PostInsightIntentExecuted(sessionInfo, result, needForeground);
        });

    InsightIntentExecutorInfo executorInfo;
    ForegroundWindowInitInsightIntentExecutorInfo(want, sessionInfo, executorInfo);
    int32_t ret = DelayedSingleton<InsightIntentExecutorMgr>::GetInstance()->ExecuteInsightIntent(
        etsRuntime_, executorInfo, std::move(executorCallback));
    if (!ret) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Execute insight intent failed");
        // callback has removed, release in insight intent executor.
    }
    TAG_LOGD(AAFwkTag::UI_EXT, "end");
    return true;
}

void EtsUIExtensionBase::ExecuteInsightIntentDone(uint64_t intentId, const InsightIntentExecuteResult &result)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "intentId %{public}" PRIu64"", intentId);
    auto ret = AAFwk::AbilityManagerClient::GetInstance()->ExecuteInsightIntentDone(token_, intentId, result);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "notify execute done failed");
    }
}

void EtsUIExtensionBase::PostInsightIntentExecuted(const sptr<AAFwk::SessionInfo> &sessionInfo,
    const AppExecFwk::InsightIntentExecuteResult &result, bool needForeground)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UI_EXT, "Post insightintent executed");
    if (needForeground) {
        // If uiextensionability is started for the first time or need move background to foreground.
        CallObjectMethod(false, "onForeground", nullptr);
    }

    auto filtered = result;
    filtered.interactionInfo = nullptr;
    TAG_LOGW(AAFwkTag::UI_EXT, "filter interactionInfo in window path");
    OnInsightIntentExecuteDone(sessionInfo, filtered);

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

void EtsUIExtensionBase::OnCommandWindowDone(const sptr<AAFwk::SessionInfo> &sessionInfo, AAFwk::WindowCommand winCmd)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UI_EXT, "called");
    if (context_ == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null context");
        return;
    }
    std::unique_lock<std::mutex> windowLock(g_mtxWindowMapRecord);
    std::unique_lock<std::mutex> foregroundLock(g_mtxForegroundWindowsRecord);
    AAFwk::AbilityCommand abilityCmd;
    if (uiWindowMap_.empty()) {
        abilityCmd = AAFwk::ABILITY_CMD_DESTROY;
    } else if (foregroundWindows_.empty()) {
        abilityCmd = AAFwk::ABILITY_CMD_BACKGROUND;
    } else {
        abilityCmd = AAFwk::ABILITY_CMD_FOREGROUND;
    }
    foregroundLock.unlock();
    windowLock.unlock();
    AAFwk::AbilityManagerClient::GetInstance()->ScheduleCommandAbilityWindowDone(
        context_->GetToken(), sessionInfo, winCmd, abilityCmd);
    TAG_LOGD(AAFwkTag::UI_EXT, "end");
}

void EtsUIExtensionBase::OnInsightIntentExecuteDone(const sptr<AAFwk::SessionInfo> &sessionInfo,
    const AppExecFwk::InsightIntentExecuteResult &result)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (sessionInfo == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Invalid sessionInfo");
        return;
    }

    TAG_LOGD(AAFwkTag::UI_EXT, "UIExtension component id: %{public}" PRId64, sessionInfo->uiExtensionComponentId);
    auto componentId = sessionInfo->uiExtensionComponentId;

    sptr<Rosen::Window> window;
    {
        std::unique_lock<std::mutex> windowLock(g_mtxWindowMapRecord);
        auto it = uiWindowMap_.find(componentId);
        if (it == uiWindowMap_.end() || it->second == nullptr) {
            TAG_LOGE(AAFwkTag::UI_EXT, "Window not found for componentId: %{public}" PRId64, componentId);
            return;
        }
        window = it->second;
    }

    WantParams params;
    params.SetParam(INSIGHT_INTENT_EXECUTE_RESULT_CODE, Integer::Box(result.innerErr));
    auto resultParams = result.BuildFunctionResult();
    resultParams->SetParam("code", Integer::Box(result.code));
    sptr<AAFwk::IWantParams> pWantParams = WantParamWrapper::Box(*resultParams);
    if (pWantParams != nullptr) {
        params.SetParam(INSIGHT_INTENT_EXECUTE_RESULT, pWantParams);
    }

    Rosen::WMError ret = window->TransferExtensionData(params);
    if (ret == Rosen::WMError::WM_OK) {
        TAG_LOGD(AAFwkTag::UI_EXT, "TransferExtensionData success");
    } else {
        TAG_LOGE(AAFwkTag::UI_EXT, "TransferExtensionData failed, ret=%{public}d", ret);
    }
    window->Show();
    {
        std::unique_lock<std::mutex> foregroundLock(g_mtxForegroundWindowsRecord);
        foregroundWindows_.emplace(componentId);
    }
    TAG_LOGD(AAFwkTag::UI_EXT, "end");
}

void EtsUIExtensionBase::OnCommand(const AAFwk::Want &want, bool restart, int32_t startId)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "called");
    auto env = etsRuntime_.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null env");
        return;
    }
    ani_ref wantObj = OHOS::AppExecFwk::WrapWant(env, want);
    if (wantObj == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null wantObj");
        return;
    }
    CallObjectMethod(false, "onRequest", nullptr, wantObj, AppExecFwk::CreateInt(env, startId));
}

void EtsUIExtensionBase::OnForeground(const Want &want, sptr<AAFwk::SessionInfo> sessionInfo)
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
    CallObjectMethod(false, "onForeground", nullptr);
}

void EtsUIExtensionBase::OnBackground()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UI_EXT, "called");
    CallObjectMethod(false, "onBackground", nullptr);
}

bool EtsUIExtensionBase::CallEtsOnSessionCreate(const AAFwk::Want &want, const sptr<AAFwk::SessionInfo> &sessionInfo,
    const sptr<Rosen::Window> &uiWindow, const uint64_t &uiExtensionComponentId)
{
    auto env = etsRuntime_.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null env");
        return false;
    }
    ani_ref wantObj = OHOS::AppExecFwk::WrapWant(env, want);
    if (wantObj == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null wantObj");
        return false;
    }
    std::weak_ptr<Context> wkctx = context_;
    etsUiExtContentSession_ = std::make_shared<EtsUIExtensionContentSession>(sessionInfo, uiWindow,
        wkctx, abilityResultListeners_);
    ani_ref sessionObj = EtsUIExtensionContentSession::CreateEtsUIExtensionContentSession(
        env, etsUiExtContentSession_.get());
    if (sessionObj == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null contentSession");
        return false;
    }
    ani_status status = ANI_ERROR;
    ani_ref sessionObjRef = nullptr;
    if ((status = env->GlobalReference_Create(sessionObj, &sessionObjRef)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "status: %{public}d", status);
        return false;
    }
    contentSessions_.emplace(uiExtensionComponentId, sessionObjRef);
    CallObjectMethod(false, "onSessionCreate", nullptr, wantObj, sessionObj);
    return true;
}

sptr<Rosen::WindowOption> EtsUIExtensionBase::CreateWindowOption(const sptr<AAFwk::SessionInfo> &sessionInfo)
{
    auto option = sptr<Rosen::WindowOption>::MakeSptr();
    if (option == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "make option failed");
        return nullptr;
    }

    option->SetWindowName(context_->GetBundleName() + context_->GetAbilityInfo()->name);
    option->SetWindowType(Rosen::WindowType::WINDOW_TYPE_UI_EXTENSION);
    option->SetWindowSessionType(Rosen::WindowSessionType::EXTENSION_SESSION);
    option->SetParentId(sessionInfo->hostWindowId);
    option->SetRealParentId(sessionInfo->realHostWindowId);
    option->SetParentWindowType(static_cast<Rosen::WindowType>(sessionInfo->parentWindowType));
    option->SetUIExtensionUsage(static_cast<uint32_t>(sessionInfo->uiExtensionUsage));
    option->SetDensity(sessionInfo->density);
    option->SetIsDensityFollowHost(sessionInfo->isDensityFollowHost);
    if (context_->isNotAllow != -1) {
        bool isNotAllow = context_->isNotAllow == 1 ? true : false;
        TAG_LOGD(AAFwkTag::UI_EXT, "isNotAllow: %{public}d", isNotAllow);
        option->SetConstrainedModal(isNotAllow);
    }
    if (context_->GetAbilityInfo()->extensionAbilityType ==
        AppExecFwk::ExtensionAbilityType::AGENT_UI) {
        option->SetIsBlockSubwindow(true);
    }
    return option;
}

bool EtsUIExtensionBase::HandleSessionCreate(const AAFwk::Want &want, const sptr<AAFwk::SessionInfo> &sessionInfo)
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
    std::shared_ptr<AAFwk::Want> sharedWant = std::make_shared<AAFwk::Want>(want);
    auto componentId = sessionInfo->uiExtensionComponentId;
    std::unique_lock<std::mutex> lck(g_mtxWindowMapRecord);
    if (uiWindowMap_.find(componentId) == uiWindowMap_.end()) {
        if (context_ == nullptr || context_->GetAbilityInfo() == nullptr) {
            TAG_LOGE(AAFwkTag::UI_EXT, "null context");
            return false;
        }
        auto option = CreateWindowOption(sessionInfo);
        if (option == nullptr) {
            return false;
        }
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
        uiWindow->UpdateExtensionConfig(sharedWant);
        if (!CallEtsOnSessionCreate(*sharedWant, sessionInfo, uiWindow, componentId)) {
            return false;
        }
        uiWindowMap_[componentId] = uiWindow;
#ifdef SUPPORT_GRAPHICS
        if (context_->GetWindow() == nullptr) {
            context_->SetWindow(uiWindow);
        }
#endif // SUPPORT_GRAPHICS
    } else {
        auto uiWindow = uiWindowMap_[componentId];
        if (uiWindow == nullptr) {
            TAG_LOGE(AAFwkTag::UI_EXT, "null uiWindow");
            return false;
        }
        uiWindow->UpdateExtensionConfig(sharedWant);
    }
    return true;
}

void EtsUIExtensionBase::ForegroundWindow(const AAFwk::Want &want, const sptr<AAFwk::SessionInfo> &sessionInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (!HandleSessionCreate(want, sessionInfo)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "HandleSessionCreate failed");
        return;
    }
    TAG_LOGD(AAFwkTag::UI_EXT, "UIExtension component id: %{public}" PRId64, sessionInfo->uiExtensionComponentId);
    auto componentId = sessionInfo->uiExtensionComponentId;

    sptr<Rosen::Window> window;
    bool windowExists = false;
    {
        std::unique_lock<std::mutex> windowLock(g_mtxWindowMapRecord);
        auto it = uiWindowMap_.find(componentId);
        if (it != uiWindowMap_.end() && it->second != nullptr) {
            window = it->second;
            windowExists = true;
        }
    }
    if (!windowExists) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Window not found for componentId: %{public}" PRId64, componentId);
        return;
    }
    HITRACE_METER_NAME(HITRACE_TAG_APP, "Rosen::Window::show");
    window->Show();
    {
        std::unique_lock<std::mutex> foregroundLock(g_mtxForegroundWindowsRecord);
        foregroundWindows_.emplace(componentId);
    }
}

void EtsUIExtensionBase::BackgroundWindow(const sptr<AAFwk::SessionInfo> &sessionInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (sessionInfo == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Invalid sessionInfo");
        return;
    }

    TAG_LOGD(AAFwkTag::UI_EXT, "UIExtension component id: %{public}" PRId64, sessionInfo->uiExtensionComponentId);
    auto componentId = sessionInfo->uiExtensionComponentId;
    sptr<Rosen::Window> window;
    bool windowExists = false;
    {
        std::unique_lock<std::mutex> windowLock(g_mtxWindowMapRecord);
        auto it = uiWindowMap_.find(componentId);
        if (it != uiWindowMap_.end() && it->second != nullptr) {
            window = it->second;
            windowExists = true;
        }
    }
    
    if (!windowExists) {
        TAG_LOGE(AAFwkTag::UI_EXT, "not find uiWindow");
        return;
    }
    window->Hide();
    {
        std::unique_lock<std::mutex> foregroundLock(g_mtxForegroundWindowsRecord);
        foregroundWindows_.erase(componentId);
    }
}

void EtsUIExtensionBase::DestroyWindow(const sptr<AAFwk::SessionInfo> &sessionInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (sessionInfo == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Invalid sessionInfo");
        return;
    }

    TAG_LOGD(AAFwkTag::UI_EXT, "UIExtension component id: %{public}" PRId64, sessionInfo->uiExtensionComponentId);
    auto componentId = sessionInfo->uiExtensionComponentId;

    std::unique_lock<std::mutex> windowLock(g_mtxWindowMapRecord);
    if (uiWindowMap_.find(componentId) == uiWindowMap_.end()) {
        TAG_LOGE(AAFwkTag::UI_EXT, "not find uiWindow");
        return;
    }
    std::unique_lock<std::mutex> sessionLock(g_mtxSeesionRecord);
    std::unique_lock<std::mutex> foregroundLock(g_mtxForegroundWindowsRecord);

    if (uiWindowMap_.find(componentId) == uiWindowMap_.end()) {
        TAG_LOGE(AAFwkTag::UI_EXT, "not find uiWindow");
        return;
    }
    ani_object contentSessionObj = nullptr;
    bool needCallOnSessionDestroy = false;
    if (contentSessions_.find(componentId) != contentSessions_.end() && contentSessions_[componentId] != nullptr) {
        contentSessionObj = reinterpret_cast<ani_object>(contentSessions_[componentId]);
        needCallOnSessionDestroy = true;
    }
    auto uiWindow = uiWindowMap_[componentId];
    uiWindowMap_.erase(componentId);
    foregroundWindows_.erase(componentId);
    contentSessions_.erase(componentId);

    foregroundLock.unlock();
    sessionLock.unlock();
    windowLock.unlock();
    if (needCallOnSessionDestroy && contentSessionObj != nullptr) {
        CallObjectMethod(false, "onSessionDestroy",
            UIEXT_ONSESSIONDESTROY_SIGNATURE, contentSessionObj);
    }
    if (uiWindow) {
        uiWindow->Destroy();
    }
    if (abilityResultListeners_) {
        abilityResultListeners_->RemoveListener(componentId);
    }
}

bool EtsUIExtensionBase::CallObjectMethod(bool withResult, const char *name, const char *signature, ...)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, std::string("CallObjectMethod:") + name);
    TAG_LOGD(AAFwkTag::UI_EXT, "CallObjectMethod %{public}s", name);
    if (etsObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "etsObj_ nullptr");
        return false;
    }
    auto env = etsRuntime_.GetAniEnv();
    ani_status status = ANI_OK;
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null env");
        return false;
    }
    env->ResetError();
    if (withResult) {
        ani_boolean res = ANI_FALSE;
        va_list args;
        va_start(args, signature);
        if ((status = env->Object_CallMethodByName_Boolean(etsObj_->aniObj, name, signature, &res, args)) != ANI_OK) {
            TAG_LOGE(AAFwkTag::UI_EXT, "status: %{public}d", status);
            etsRuntime_.HandleUncaughtError();
        }
        va_end(args);
        return res;
    }
    va_list args;
    va_start(args, signature);
    if ((status = env->Object_CallMethodByName_Void_V(etsObj_->aniObj, name, signature, args)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "status: %{public}d", status);
        etsRuntime_.HandleUncaughtError();
        va_end(args);
        return false;
    }
    va_end(args);
    return false;
}

ani_object EtsUIExtensionBase::CallObjectMethod(const char *name, const char *signature, ...)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, std::string("CallObjectMethod:") + name);
    TAG_LOGD(AAFwkTag::UI_EXT, "StsUIAbility call sts, name: %{public}s", name);
    if (etsObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null stsAbilityObj");
        return nullptr;
    }
    auto env = etsRuntime_.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null env");
        return nullptr;
    }
    auto obj = etsObj_->aniObj;
    auto cls = etsObj_->aniCls;
    ani_status status = ANI_ERROR;

    ani_method method {};
    if ((status = env->Class_FindMethod(cls, name, signature, &method)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "status : %{public}d", status);
        env->ResetError();
        return nullptr;
    }
    ani_ref res {};
    va_list args;
    va_start(args, signature);
    if ((status = env->Object_CallMethod_Ref(obj, method, &res, args)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "status : %{public}d", status);
        etsRuntime_.HandleUncaughtError();
        va_end(args);
        return nullptr;
    }
    va_end(args);
    return reinterpret_cast<ani_object>(res);
}

void EtsUIExtensionBase::OnConfigurationUpdated(const AppExecFwk::Configuration &configuration)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UI_EXT, "called");
    if (context_ == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null context");
        return;
    }

    auto abilityConfig = context_->GetAbilityConfiguration();
    auto configUtils = std::make_shared<ConfigurationUtils>();

    if (abilityConfig != nullptr) {
        auto newConfig = configUtils->UpdateGlobalConfig(configuration, context_->GetConfiguration(),
            abilityConfig, context_->GetResourceManager());
        if (newConfig.GetItemSize() == 0) {
            return;
        }
        if (context_->GetWindow()) {
            TAG_LOGI(AAFwkTag::UI_EXT, "newConfig: %{public}s", newConfig.GetName().c_str());
            auto diffConfiguration = std::make_shared<AppExecFwk::Configuration>(newConfig);
            context_->GetWindow()->UpdateConfigurationForSpecified(diffConfiguration, context_->GetResourceManager());
        }
    } else {
        auto configUtils = std::make_shared<ConfigurationUtils>();
        configUtils->UpdateGlobalConfig(configuration, context_->GetConfiguration(), context_->GetResourceManager());
    }

    ConfigurationUpdated();
}

void EtsUIExtensionBase::OnAbilityConfigurationUpdated(const AppExecFwk::Configuration& configuration)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UI_EXT, "called");

    if (context_ == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null context");
        return;
    }

    auto configUtils = std::make_shared<ConfigurationUtils>();
    configUtils->UpdateAbilityConfig(configuration, context_->GetResourceManager());

    if (context_->GetWindow()) {
        TAG_LOGI(AAFwkTag::UI_EXT, "newConfig: %{public}s", configuration.GetName().c_str());
        auto diffConfiguration = std::make_shared<AppExecFwk::Configuration>(configuration);
        context_->GetWindow()->UpdateConfigurationForSpecified(diffConfiguration, context_->GetResourceManager());
    }

    ConfigurationUpdated();
}

void EtsUIExtensionBase::Dump(const std::vector<std::string> &params, std::vector<std::string> &info)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "called");
    auto env = etsRuntime_.GetAniEnv();
    if (env == nullptr || etsObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null env or etsObj_");
        return;
    }

    ani_object paramsArrayObj = nullptr;
    if (!AppExecFwk::WrapArrayString(env, paramsArrayObj, params)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null env or etsObj_");
        return;
    }
    ani_status status = ANI_ERROR;
    ani_method method = nullptr;
    if ((status = env->Class_FindMethod(etsObj_->aniCls, "onDump", nullptr, &method)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "find onDump failed: %{public}d", status);
        if ((status = env->Class_FindMethod(etsObj_->aniCls, "dump", nullptr, &method)) != ANI_OK) {
            TAG_LOGE(AAFwkTag::UI_EXT, "find dump failed: %{public}d", status);
            return;
        }
    }
    if (!method) {
        TAG_LOGE(AAFwkTag::UI_EXT, "find method onDump failed");
        return;
    }
    ani_ref strArrayRef = nullptr;
    if ((status = env->Object_CallMethod_Ref(etsObj_->aniObj, method, &strArrayRef, paramsArrayObj)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "Object_CallMethod_Ref FAILED: %{public}d", status);
        return;
    }
    if (strArrayRef == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null strArrayRef");
        return;
    }
    if (!AppExecFwk::UnwrapArrayString(env, reinterpret_cast<ani_object>(strArrayRef), info)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "UnwrapArrayString failed");
    }
    TAG_LOGD(AAFwkTag::UI_EXT, "Dump info size: %{public}zu", info.size());
}

void EtsUIExtensionBase::OnAbilityResult(int32_t requestCode, int32_t resultCode, const Want &resultData)
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

void EtsUIExtensionBase::SetAbilityInfo(const std::shared_ptr<AppExecFwk::AbilityInfo> &abilityInfo)
{
    abilityInfo_ = abilityInfo;
}

void EtsUIExtensionBase::SetContext(const std::shared_ptr<UIExtensionContext> &context)
{
    context_ = context;
}

void EtsUIExtensionBase::ConfigurationUpdated()
{
    TAG_LOGD(AAFwkTag::UI_EXT, "begin");
    auto env = etsRuntime_.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null env");
        return;
    }

    // Notify extension context
    if (context_ == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null context");
        return;
    }

    auto abilityConfig = context_->GetAbilityConfiguration();
    auto fullConfig = context_->GetConfiguration();
    if (fullConfig == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null configuration");
        return;
    }

    auto realConfig = AppExecFwk::Configuration(*fullConfig);
    if (abilityConfig != nullptr) {
        std::vector<std::string> changeKeyV;
        realConfig.CompareDifferent(changeKeyV, *abilityConfig);
        if (!changeKeyV.empty()) {
            realConfig.Merge(changeKeyV, *abilityConfig);
        }
    }
    TAG_LOGD(AAFwkTag::UI_EXT, "realConfig: %{public}s", realConfig.GetName().c_str());
    auto realConfigPtr = std::make_shared<Configuration>(realConfig);
    EtsExtensionContext::ConfigurationUpdated(env, shellContextRef_, realConfigPtr);
    ani_object aniConfiguration = AppExecFwk::WrapConfiguration(env, realConfig);
    CallObjectMethod(false, "onConfigurationUpdate", nullptr, aniConfiguration);
}

#ifdef SUPPORT_GRAPHICS
void EtsUIExtensionBase::OnDisplayInfoChange(
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
        auto EtsUiExtension = std::static_pointer_cast<EtsUIExtensionBase>(shared_from_this());
        auto task = [EtsUiExtension]() {
            if (EtsUiExtension) {
                EtsUiExtension->ConfigurationUpdated();
            }
        };
        if (handler_ != nullptr) {
            handler_->PostTask(task, "EtsUIExtensionBase:OnChange");
        }
    }
}

void EtsUIExtensionBase::RegisterDisplayInfoChangedListener()
{
    // register displayid change callback
    auto EtsUiExtensionBase = std::static_pointer_cast<EtsUIExtensionBase>(shared_from_this());
    EtsUIExtensionBaseDisplayListener_ = sptr<EtsUIExtensionBaseDisplayListener>::MakeSptr(EtsUiExtensionBase);
    if (EtsUIExtensionBaseDisplayListener_ == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null EtsUIExtensionBaseDisplayListener");
        return;
    }
    if (context_ == nullptr || context_->GetToken() == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null context");
        return;
    }
    TAG_LOGI(AAFwkTag::UI_EXT, "RegisterDisplayInfoChangedListener");
    Rosen::WindowManager::GetInstance().RegisterDisplayInfoChangedListener(
        context_->GetToken(), EtsUIExtensionBaseDisplayListener_);
}

void EtsUIExtensionBase::UnregisterDisplayInfoChangedListener()
{
    if (context_ == nullptr || context_->GetToken() == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null context");
        return;
    }
    Rosen::WindowManager::GetInstance().UnregisterDisplayInfoChangedListener(
        context_->GetToken(), EtsUIExtensionBaseDisplayListener_);
}
#endif // SUPPORT_GRAPHICS
} // namespace AbilityRuntime
} // namespace OHOS
