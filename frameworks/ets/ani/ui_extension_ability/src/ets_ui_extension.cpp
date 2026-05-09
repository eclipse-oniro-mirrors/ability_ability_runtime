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

#include "ets_ui_extension.h"

#include "ability_context.h"
#include "ability_delegator_registry.h"
#include "ability_info.h"
#include "ability_manager_client.h"
#include "ability_start_setting.h"
#include "ani_common_configuration.h"
#include "ani_common_want.h"
#include "configuration_utils.h"
#include "connection_manager.h"
#include "context.h"
#include "ets_ability_lifecycle_callback.h"
#include "ets_data_struct_converter.h"
#include "ets_extension_common.h"
#include "ets_extension_context.h"
#include "ets_native_reference.h"
#include "ets_runtime.h"
#include "ets_ui_extension_context.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "insight_intent_delay_result_callback_mgr.h"
#include "insight_intent_executor_info.h"
#include "insight_intent_executor_mgr.h"
#include "int_wrapper.h"
#include "js_ui_extension_content_session.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "napi/native_api.h"
#include "ui_extension_window_command.h"
#include "want_params_wrapper.h"

#ifdef WINDOWS_PLATFORM
#define ETS_EXPORT __declspec(dllexport)
#else
#define ETS_EXPORT __attribute__((visibility("default")))
#endif

namespace OHOS {
namespace AbilityRuntime {
using namespace OHOS::AppExecFwk;
namespace {
constexpr const char* UIEXTENSION_CLASS_NAME = "@ohos.app.ability.UIExtensionAbility.UIExtensionAbility";

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

EtsUIExtension *EtsUIExtension::Create(const std::unique_ptr<Runtime> &runtime)
{
    return new (std::nothrow) EtsUIExtension(static_cast<ETSRuntime&>(*runtime));
}

EtsUIExtension::EtsUIExtension(ETSRuntime &eTSRuntime) : etsRuntime_(eTSRuntime)
{
    abilityResultListeners_ = std::make_shared<EtsAbilityResultListeners>();
}

EtsUIExtension::~EtsUIExtension()
{
    auto context = GetContext();
    if (context) {
        context->Unbind();
    }
    auto &jsRuntime = etsRuntime_.GetJsRuntime();
    if (jsRuntime == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null jsRuntime");
        return;
    }
    auto &jsRuntimePoint = (static_cast<AbilityRuntime::JsRuntime &>(*jsRuntime));
    for (auto &item : contentSessions_) {
        if (item.second.jsContentSession != nullptr) {
            jsRuntimePoint.FreeNativeReference(std::move(item.second.jsContentSession));
        }
    }
    contentSessions_.clear();
    auto env = etsRuntime_.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null env");
        return;
    }
    if (shellContextRef_ && shellContextRef_->aniRef) {
        env->GlobalReference_Delete(shellContextRef_->aniRef);
    }
    InsightIntentDelayResultCallbackMgr::GetInstance().RemoveDelayResultCallback(intentId_);
}

void EtsUIExtension::Init(const std::shared_ptr<AbilityLocalRecord> &record,
    const std::shared_ptr<OHOSApplication> &application, std::shared_ptr<AbilityHandler> &handler,
    const sptr<IRemoteObject> &token)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (record == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "record null");
        return;
    }
    UIExtension::Init(record, application, handler, token);
    if (Extension::abilityInfo_ == nullptr || Extension::abilityInfo_->srcEntrance.empty()) {
        TAG_LOGE(AAFwkTag::UI_EXT, "EtsUIExtension Init abilityInfo error");
        return;
    }

    RegisterAbilityConfigUpdateCallback();
    if (record != nullptr) {
        token_ = record->GetToken();
    }
    std::string srcPath(Extension::abilityInfo_->moduleName + "/");
    srcPath.append(Extension::abilityInfo_->srcEntrance);
    auto pos = srcPath.rfind(".");
    if (pos != std::string::npos) {
        srcPath.erase(pos);
        srcPath.append(".abc");
    }
    std::string moduleName(Extension::abilityInfo_->moduleName);
    moduleName.append("::").append(abilityInfo_->name);

    etsObj_ = etsRuntime_.LoadModule(
        moduleName, srcPath, abilityInfo_->hapPath, abilityInfo_->compileMode == AppExecFwk::CompileMode::ES_MODULE,
        false, abilityInfo_->srcEntrance);

    if (etsObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "etsObj_ null");
        return;
    }
    if (!BindNativeMethods()) {
        TAG_LOGE(AAFwkTag::UI_EXT, "BindNativeMethods failed");
        return;
    }
    BindContext(etsRuntime_.GetAniEnv(), record->GetWant());
    RegisterDisplayInfoChangedListener();
}

bool EtsUIExtension::BindNativeMethods()
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
    SetExtensionCommon(
        EtsExtensionCommon::Create(etsRuntime_, static_cast<ETSNativeReference &>(*etsObj_), shellContextRef_));
    return true;
}

ani_object EtsUIExtension::CreateETSContext(
    ani_env *env, std::shared_ptr<UIExtensionContext> context, int32_t screenMode)
{
    ani_object obj = CreateEtsUIExtensionContext(env, context);
    return obj;
}

void EtsUIExtension::BindContext(ani_env *env, std::shared_ptr<AAFwk::Want> want)
{
    if (env == nullptr || want == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Want info is null or env is null");
        return;
    }

    auto context = GetContext();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "context null");
        return;
    }

    int32_t screenMode = want->GetIntParam(AAFwk::SCREEN_MODE_KEY, AAFwk::IDLE_SCREEN_MODE);
    ani_object contextObj = CreateETSContext(env, context, screenMode);
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

void EtsUIExtension::OnStart(const AAFwk::Want &want, sptr<AAFwk::SessionInfo> sessionInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
#ifdef SUPPORT_GRAPHICS
    auto context = GetContext();
    if (context != nullptr && sessionInfo != nullptr) {
        auto configUtils = std::make_shared<ConfigurationUtils>();
        configUtils->InitDisplayConfig(context->GetConfiguration(), context->GetResourceManager(),
            sessionInfo->displayId, sessionInfo->density, sessionInfo->orientation);
    }
#endif // SUPPORT_GRAPHICS

    auto env = etsRuntime_.GetAniEnv();
    if (!env) {
        TAG_LOGE(AAFwkTag::UI_EXT, "env not found Ability.ets");
        return;
    }
    if (context != nullptr) {
        EtsExtensionContext::ConfigurationUpdated(env, shellContextRef_, context->GetConfiguration());
    }
    const char *signature =
        "C{@ohos.app.ability.AbilityConstant.AbilityConstant.LaunchParam}:";
    auto launchParam = Extension::GetLaunchParam();
    if (InsightIntentExecuteParam::IsInsightIntentExecute(want)) {
        launchParam.launchReason = AAFwk::LaunchReason::LAUNCHREASON_INSIGHT_INTENT;
    }

    ani_object launchParamObj = nullptr;
    if (!WrapLaunchParam(env, launchParam, launchParamObj)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "WrapLaunchParam failed");
        return;
    }
    int32_t screenMode = want.GetIntParam(AAFwk::SCREEN_MODE_KEY, AAFwk::IDLE_SCREEN_MODE);
    if (!IsEmbeddableStart(screenMode)) {
        CallObjectMethod(false, "onCreate", signature, launchParamObj);
    }
}

void EtsUIExtension::OnStop()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    UIExtension::OnStop();
    TAG_LOGD(AAFwkTag::UI_EXT, "begin");
    auto context = GetContext();
    if (context) {
        TAG_LOGD(AAFwkTag::UI_EXT, "set terminating true");
        context->SetTerminating(true);
    }
    AbilityRuntime::ApplicationConfigurationManager::GetInstance().DeleteIgnoreContext(GetContext());
    TAG_LOGD(AAFwkTag::UI_EXT, "GetIgnoreContext size %{public}zu",
        AbilityRuntime::ApplicationConfigurationManager::GetInstance().GetIgnoreContext().size());

    CallObjectMethod(false, "onDestroy", nullptr);

#ifdef SUPPORT_GRAPHICS
    UnregisterDisplayInfoChangedListener();
#endif // SUPPORT_GRAPHICS
    OnStopCallBack();
    TAG_LOGD(AAFwkTag::UI_EXT, "end");
}

void EtsUIExtension::OnStop(AppExecFwk::AbilityTransactionCallbackInfo<> *callbackInfo, bool &isAsyncCallback)
{
    if (callbackInfo == nullptr) {
        isAsyncCallback = false;
        OnStop();
        return;
    }
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    UIExtension::OnStop();
#ifdef SUPPORT_GRAPHICS
    UnregisterDisplayInfoChangedListener();
#endif // SUPPORT_GRAPHICS

    std::weak_ptr<Extension> weakPtr = shared_from_this();
    auto asyncCallback = [extensionWeakPtr = weakPtr]() {
        auto etsUIExtension = extensionWeakPtr.lock();
        if (etsUIExtension == nullptr) {
            TAG_LOGE(AAFwkTag::UI_EXT, "etsUIExtension null");
            return;
        }
        etsUIExtension->OnStopCallBack();
    };
    callbackInfo->Push(asyncCallback);
    ani_long destroyCallbackPoint = (ani_long)callbackInfo;
    ani_status status = ANI_ERROR;
    ani_field field = nullptr;
    auto env = etsRuntime_.GetAniEnv();
    if (env == nullptr || etsObj_ == nullptr) {
        isAsyncCallback = false;
        OnStop();
        return;
    }
    if ((status = env->Class_FindField(etsObj_->aniCls, "destroyCallbackPoint", &field)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "status: %{public}d", status);
    }
    if ((status = env->Object_SetField_Long(etsObj_->aniObj, field, destroyCallbackPoint)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "status: %{public}d", status);
    }

    isAsyncCallback = CallObjectMethod(true, "callOnDestroy", ":z");
    if (!isAsyncCallback) {
        OnStopCallBack();
        return;
    }
}

void EtsUIExtension::OnStopCallBack()
{
    UIExtension::OnStopCallBack();
    auto applicationContext = Context::GetApplicationContext();
    if (applicationContext != nullptr) {
        EtsAbilityLifecycleCallbackArgs ability(etsObj_);
        applicationContext->DispatchOnAbilityDestroy(ability);
    }
}

bool EtsUIExtension::ForegroundWindowInitInsightIntentExecutorInfo(const AAFwk::Want &want,
    const sptr<AAFwk::SessionInfo> &sessionInfo, InsightIntentExecutorInfo &executorInfo, const std::string &arkTSmode)
{
    auto context = GetContext();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null context");
        return false;
    }
    if (sessionInfo == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null sessionInfo");
        return false;
    }
    std::shared_ptr<AppExecFwk::AbilityInfo> abilityInfo = context->GetAbilityInfo();
    if (abilityInfo != nullptr) {
        executorInfo.hapPath = abilityInfo->hapPath;
        executorInfo.windowMode = abilityInfo->compileMode == AppExecFwk::CompileMode::ES_MODULE;
    }
    executorInfo.token = context->GetToken();
    if (arkTSmode == AbilityRuntime::CODE_LANGUAGE_ARKTS_1_2) {
        executorInfo.etsPageLoader =
            reinterpret_cast<void *>(contentSessions_[sessionInfo->uiExtensionComponentId].etsContentSession);
    } else {
        auto aniEnv = etsRuntime_.GetAniEnv();
        if (aniEnv == nullptr) {
            TAG_LOGE(AAFwkTag::UI_EXT, "null aniEnv");
            return false;
        }
        ani_ref etsContentSessionTemp = contentSessions_[sessionInfo->uiExtensionComponentId].etsContentSession;
        if (etsContentSessionTemp == nullptr) {
            TAG_LOGE(AAFwkTag::UI_EXT, "null etsContentSessionTemp");
            return false;
        }
        auto etsContentSession = EtsUIExtensionContentSession::GetEtsContentSession(
            aniEnv, static_cast<ani_object>(etsContentSessionTemp));
        if (etsContentSession == nullptr) {
            TAG_LOGE(AAFwkTag::UI_EXT, "null etsContentSession");
            return false;
        }
        auto& jsRuntime = etsRuntime_.GetJsRuntime();
        if (jsRuntime == nullptr) {
            TAG_LOGE(AAFwkTag::UI_EXT, "null jsRuntime");
            return false;
        }
        auto abilityResultListeners = std::make_shared<AbilityResultListeners>();
        auto &jsRuntimePoint = (static_cast<AbilityRuntime::JsRuntime &>(*jsRuntime));
        auto env = jsRuntimePoint.GetNapiEnv();
        if (abilityInfo_) {
            auto &jsRuntimePoint = (static_cast<AbilityRuntime::JsRuntime &>(*jsRuntime));
            jsRuntimePoint.UpdateModuleNameAndAssetPath(abilityInfo_->moduleName);
        }
        std::weak_ptr<Context> wkctx = context;
        napi_value nativeContentSession = JsUIExtensionContentSession::CreateJsUIExtensionContentSession(
            env, etsContentSession->GetSessionInfo(), etsContentSession->GetUIWindow(), wkctx, abilityResultListeners);
        napi_ref ref = nullptr;
        napi_create_reference(env, nativeContentSession, 1, &ref);
        contentSessions_[sessionInfo->uiExtensionComponentId].jsContentSession =
            std::shared_ptr<NativeReference>(reinterpret_cast<NativeReference*>(ref));
        executorInfo.pageLoader = contentSessions_[sessionInfo->uiExtensionComponentId].jsContentSession;
    }
    executorInfo.executeParam = std::make_shared<InsightIntentExecuteParam>();
    InsightIntentExecuteParam::GenerateFromWant(want, *executorInfo.executeParam);
    executorInfo.executeParam->executeMode_ = UI_EXTENSION_ABILITY;
    executorInfo.srcEntry = want.GetStringParam(INSIGHT_INTENT_SRC_ENTRY);
    executorInfo.decoratorClass = want.GetStringParam(INSIGHT_INTENT_DECORATOR_CLASS);
    TAG_LOGD(AAFwkTag::UI_EXT, "executorInfo, insightIntentId: %{public}" PRIu64,
        executorInfo.executeParam->insightIntentId_);
    return true;
}

bool EtsUIExtension::ForegroundWindowWithInsightIntent(const AAFwk::Want &want,
    const sptr<AAFwk::SessionInfo> &sessionInfo, bool needForeground)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (!HandleSessionCreate(want, sessionInfo)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "HandleSessionCreate failed");
        return false;
    }

    std::unique_ptr<InsightIntentExecutorAsyncCallback> executorCallback = nullptr;
    executorCallback.reset(InsightIntentExecutorAsyncCallback::Create());
    if (executorCallback == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Create async callback failed");
        return false;
    }

    auto uiExtension = std::static_pointer_cast<EtsUIExtension>(shared_from_this());
    executorCallback->Push([uiExtension, sessionInfo, needForeground, want](
        AppExecFwk::InsightIntentExecuteResult result) {
        if (uiExtension == nullptr) {
            TAG_LOGE(AAFwkTag::UI_EXT, "UI extension is nullptr");
            return;
        }

        InsightIntentExecuteParam executeParam;
        InsightIntentExecuteParam::GenerateFromWant(want, executeParam);
        if (result.uris.size() > 0) {
            uiExtension->ExecuteInsightIntentDone(executeParam.insightIntentId_, result);
        }
        uiExtension->PostInsightIntentExecuted(sessionInfo, result, needForeground);
    });
    const WantParams &wantParams = want.GetParams();
    std::string arkTSMode = wantParams.GetStringParam(INSIGHT_INTENT_ARKTS_MODE);
    InsightIntentExecutorInfo executorInfo;
    if (!ForegroundWindowInitInsightIntentExecutorInfo(want, sessionInfo, executorInfo, arkTSMode)) {
        return false;
    }
    InsightIntentDelayResultCallbackMgr::GetInstance().RemoveDelayResultCallback(intentId_);
    bool isDecorator = executorInfo.executeParam->decoratorType_ != static_cast<int8_t>(InsightIntentType::DECOR_NONE);
    RegisterUiExtensionDelayResultCallback(executorInfo.executeParam->insightIntentId_, sessionInfo, isDecorator);
    if (arkTSMode == AbilityRuntime::CODE_LANGUAGE_ARKTS_1_2) {
        int32_t ret = DelayedSingleton<InsightIntentExecutorMgr>::GetInstance()->ExecuteInsightIntent(
            etsRuntime_, executorInfo, std::move(executorCallback));
        if (!ret) {
            TAG_LOGE(AAFwkTag::UI_EXT, "Execute insight intent failed");
            InsightIntentDelayResultCallbackMgr::GetInstance().RemoveDelayResultCallback(
                executorInfo.executeParam->insightIntentId_);
        } else {
            intentId_ = executorInfo.executeParam->insightIntentId_;
        }
    } else {
        auto& jsRuntime = etsRuntime_.GetJsRuntime();
        if (jsRuntime == nullptr) {
            TAG_LOGE(AAFwkTag::UI_EXT, "null jsRuntime");
            return false;
        }
        int32_t ret = DelayedSingleton<InsightIntentExecutorMgr>::GetInstance()->ExecuteInsightIntent(
            *jsRuntime, executorInfo, std::move(executorCallback));
        if (!ret) {
            TAG_LOGE(AAFwkTag::UI_EXT, "Execute insight intent failed");
            InsightIntentDelayResultCallbackMgr::GetInstance().RemoveDelayResultCallback(
                executorInfo.executeParam->insightIntentId_);
        } else {
            intentId_ = executorInfo.executeParam->insightIntentId_;
        }
    }
    return true;
}

void EtsUIExtension::ExecuteInsightIntentDone(uint64_t intentId, const InsightIntentExecuteResult &result)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "intentId %{public}" PRIu64"", intentId);
    auto ret = AAFwk::AbilityManagerClient::GetInstance()->ExecuteInsightIntentDone(token_, intentId, result);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "notify execute done failed");
    }
}
void EtsUIExtension::PostInsightIntentExecuted(const sptr<AAFwk::SessionInfo> &sessionInfo,
    const AppExecFwk::InsightIntentExecuteResult &result, bool needForeground)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (needForeground) {
        // If uiextensionability is started for the first time or need move background to foreground.
        CallObjectMethod(false, "onForeground", nullptr);
    }

    OnInsightIntentExecuteDone(sessionInfo, result);

    if (needForeground) {
        // If need foreground, that means triggered by onForeground.
        AAFwk::PacMap restoreData;
        AAFwk::AbilityManagerClient::GetInstance()->AbilityTransitionDone(token_, AAFwk::ABILITY_STATE_FOREGROUND_NEW,
            restoreData);
    } else {
        // If uiextensionability has displayed in the foreground.
        OnCommandWindowDone(sessionInfo, AAFwk::WIN_CMD_FOREGROUND);
    }
}

void EtsUIExtension::OnForeground(const Want &want, sptr<AAFwk::SessionInfo> sessionInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (sessionInfo == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "sessionInfo nullptr");
        return;
    }
    Extension::OnForeground(want, sessionInfo);
    if (InsightIntentExecuteParam::IsInsightIntentExecute(want)) {
        bool finish = ForegroundWindowWithInsightIntent(want, sessionInfo, true);
        if (finish) {
            return;
        }
    }
    ForegroundWindow(want, sessionInfo);
    CallObjectMethod(false, "onForeground", nullptr);
}

void EtsUIExtension::OnBackground()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    CallObjectMethod(false, "onBackground", nullptr);
    Extension::OnBackground();
}

bool EtsUIExtension::IsEmbeddableStart(int32_t screenMode)
{
    return screenMode == AAFwk::EMBEDDED_FULL_SCREEN_MODE ||
        screenMode == AAFwk::EMBEDDED_HALF_SCREEN_MODE;
}

bool EtsUIExtension::HandleSessionCreate(const AAFwk::Want &want, const sptr<AAFwk::SessionInfo> &sessionInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (sessionInfo == nullptr || sessionInfo->uiExtensionComponentId == 0) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Invalid sessionInfo");
        return false;
    }
    std::lock_guard<std::mutex> lock(uiWindowMutex_);
    std::shared_ptr<AAFwk::Want> sharedWant = std::make_shared<AAFwk::Want>(want);
    auto compId = sessionInfo->uiExtensionComponentId;
    if (uiWindowMap_.find(compId) == uiWindowMap_.end()) {
        auto context = GetContext();
        auto uiWindow = CreateUIWindow(context, sessionInfo);
        if (uiWindow == nullptr) {
            TAG_LOGE(AAFwkTag::UI_EXT, "create ui window error");
            return false;
        }
        uiWindow->UpdateExtensionConfig(sharedWant);
        auto env = etsRuntime_.GetAniEnv();
        ani_ref wantObj = OHOS::AppExecFwk::WrapWant(env, want);
        if (wantObj == nullptr) {
            TAG_LOGE(AAFwkTag::UI_EXT, "null wantObj");
            return false;
        }
        ani_ref wantRef = nullptr;
        ani_status status = ANI_OK;
        if ((status = env->GlobalReference_Create(wantObj, &wantRef)) != ANI_OK) {
            TAG_LOGE(AAFwkTag::UI_EXT, "status: %{public}d", status);
            return false;
        }
        std::weak_ptr<Context> wkctx = context;
        etsUiExtContentSession_ = std::make_shared<EtsUIExtensionContentSession>(sessionInfo, uiWindow,
            wkctx, abilityResultListeners_);
        ani_object sessonObj = EtsUIExtensionContentSession::CreateEtsUIExtensionContentSession(env,
            etsUiExtContentSession_.get());
        ani_ref sessonObjRef = nullptr;
        if ((status = env->GlobalReference_Create(sessonObj, &sessonObjRef)) != ANI_OK) {
            TAG_LOGE(AAFwkTag::UI_EXT, "status: %{public}d", status);
            return false;
        }
        ContentSessionType contentSession;
        contentSession.etsContentSession = sessonObjRef;
        contentSessions_.emplace(compId, contentSession);
        int32_t screenMode = want.GetIntParam(AAFwk::SCREEN_MODE_KEY, AAFwk::IDLE_SCREEN_MODE);
        if (!IsEmbeddableStart(screenMode)) {
            CallObjectMethod(false, "onSessionCreate", nullptr, wantObj, sessonObj);
        }
        uiWindowMap_[compId] = uiWindow;
#ifdef SUPPORT_GRAPHICS
        if (context && context->GetWindow() == nullptr) {
            context->SetWindow(uiWindow);
        }
#endif // SUPPORT_GRAPHICS
    } else {
        auto uiWindow = uiWindowMap_[compId];
        if (uiWindow == nullptr) {
            TAG_LOGE(AAFwkTag::UI_EXT, "null uiWindow");
            return false;
        }
        uiWindow->UpdateExtensionConfig(sharedWant);
    }
    return true;
}

sptr<Rosen::Window> EtsUIExtension::CreateUIWindow(const std::shared_ptr<UIExtensionContext> context,
    const sptr<AAFwk::SessionInfo> &sessionInfo)
{
    if (context == nullptr || context->GetAbilityInfo() == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "context null");
        return nullptr;
    }
    auto option = sptr<Rosen::WindowOption>::MakeSptr();
    if (option == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "option null");
        return nullptr;
    }
    option->SetWindowName(context->GetBundleName() + context->GetAbilityInfo()->name);
    option->SetWindowType(Rosen::WindowType::WINDOW_TYPE_UI_EXTENSION);
    option->SetWindowSessionType(Rosen::WindowSessionType::EXTENSION_SESSION);
    option->SetParentId(sessionInfo->hostWindowId);
    option->SetRealParentId(sessionInfo->realHostWindowId);
    option->SetParentWindowType(static_cast<Rosen::WindowType>(sessionInfo->parentWindowType));
    option->SetUIExtensionUsage(static_cast<uint32_t>(sessionInfo->uiExtensionUsage));
    HITRACE_METER_NAME(HITRACE_TAG_APP, "Rosen::Window::Create");
    return Rosen::Window::Create(
        option,
        GetContext(),
        sessionInfo->sessionToken,
        Rosen::DefaultCreateErrCode,
        "",
        false,
        sessionInfo->renderSession);
}

std::unique_ptr<AppExecFwk::ETSNativeReference> EtsUIExtension::CreateAppWindowStage(sptr<Rosen::Window> uiWindow,
    sptr<AAFwk::SessionInfo> sessionInfo)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "called");
    return std::make_unique<AppExecFwk::ETSNativeReference>();
}

void EtsUIExtension::DestroyWindow(const sptr<AAFwk::SessionInfo> &sessionInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (sessionInfo == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Invalid sessionInfo");
        return;
    }
    std::lock_guard<std::mutex> lock(uiWindowMutex_);
    auto componentId = sessionInfo->uiExtensionComponentId;
    if (uiWindowMap_.find(componentId) == uiWindowMap_.end()) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Wrong to find uiWindow");
        return;
    }
    if (contentSessions_.find(componentId) != contentSessions_.end() &&
        contentSessions_[componentId].etsContentSession != nullptr) {
        ani_object contenSessionObj = reinterpret_cast<ani_object>(contentSessions_[componentId].etsContentSession);
        if (contenSessionObj == nullptr) {
            TAG_LOGE(AAFwkTag::UI_EXT, "contenSessionObj null ptr");
            return;
        }
        CallObjectMethod(false, "onSessionDestroy", nullptr, contenSessionObj);
    }
    auto uiWindow = uiWindowMap_[componentId];
    if (uiWindow) {
        uiWindow->Destroy();
    }
    uiWindowMap_.erase(componentId);
#ifdef SUPPORT_GRAPHICS
    auto context = GetContext();
    if (context != nullptr && context->GetWindow() == uiWindow) {
        context->SetWindow(nullptr);
        for (auto it : uiWindowMap_) {
            context->SetWindow(it.second);
            break;
        }
    }
#endif // SUPPORT_GRAPHICS
    foregroundWindows_.erase(componentId);
    contentSessions_.erase(componentId);
    if (abilityResultListeners_) {
        abilityResultListeners_->RemoveListener(componentId);
    }
}

bool EtsUIExtension::CallObjectMethod(bool withResult, const char *name, const char *signature, ...)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, std::string("CallObjectMethod:") + name);
    TAG_LOGD(AAFwkTag::UI_EXT, "CallObjectMethod %{public}s", name);
    if (etsObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "etsObj_ nullptr");
        return false;
    }

    auto env = etsRuntime_.GetAniEnv();
    ani_status status = ANI_OK;
    ani_method method = nullptr;
    if ((status = env->Class_FindMethod(etsObj_->aniCls, name, signature, &method)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "status: %{public}d", status);
        return false;
    }
    env->ResetError();
    if (withResult) {
        ani_boolean res = ANI_FALSE;
        va_list args;
        va_start(args, signature);
        if ((status = env->Object_CallMethod_Boolean(etsObj_->aniObj, method, &res, args)) != ANI_OK) {
            TAG_LOGE(AAFwkTag::UI_EXT, "status: %{public}d", status);
            etsRuntime_.HandleUncaughtError();
        }
        va_end(args);
        return res;
    }
    va_list args;
    va_start(args, signature);
    if ((status = env->Object_CallMethod_Void_V(etsObj_->aniObj, method, args)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "status: %{public}d", status);
        etsRuntime_.HandleUncaughtError();
        va_end(args);
        return false;
    }
    va_end(args);
    return false;
}

void EtsUIExtension::OnAbilityConfigurationUpdated(const AppExecFwk::Configuration &configuration)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UI_EXT, "OnAbilityConfigurationUpdated called");
    UIExtension::OnAbilityConfigurationUpdated(configuration);
    ConfigurationUpdated();
}

void EtsUIExtension::Dump(const std::vector<std::string> &params, std::vector<std::string> &info)
{
    Extension::Dump(params, info);
    TAG_LOGD(AAFwkTag::UI_EXT, "called");
}

void EtsUIExtension::OnAbilityResult(int requestCode, int resultCode, const Want &resultData)
{
    Extension::OnAbilityResult(requestCode, resultCode, resultData);
    auto context = GetContext();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "context null");
        return;
    }
    context->OnAbilityResult(requestCode, resultCode, resultData);
    if (abilityResultListeners_ == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "abilityResultListensers null");
        return;
    }
    abilityResultListeners_->OnAbilityResult(requestCode, resultCode, resultData);
}

void EtsUIExtension::ConfigurationUpdated()
{
    auto env = etsRuntime_.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "env null");
        return;
    }
    auto context = GetContext();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "context null");
        return;
    }
    auto abilityConfig = context->GetAbilityConfiguration();
    auto fullConfig = context->GetConfiguration();
    if (fullConfig == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "fullConfig null");
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
    auto realConfigPtr = std::make_shared<Configuration>(realConfig);
    EtsExtensionContext::ConfigurationUpdated(env, shellContextRef_, realConfigPtr);
    ani_object aniConfiguration = AppExecFwk::WrapConfiguration(env, realConfig);
    CallObjectMethod(false, "onConfigurationUpdate", nullptr, aniConfiguration);
}

#ifdef SUPPORT_GRAPHICS
void EtsUIExtension::OnDisplayInfoChange(
    const sptr<IRemoteObject> &token, Rosen::DisplayId displayId, float density, Rosen::DisplayOrientation orientation)
{
    auto context = GetContext();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Context null");
        return;
    }

    auto contextConfig = context->GetConfiguration();
    if (contextConfig == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Configuration null");
        return;
    }

    auto configUtils = std::make_shared<ConfigurationUtils>();
    auto result =
        configUtils->UpdateDisplayConfig(contextConfig, context->GetResourceManager(), displayId, density, orientation);
    if (result) {
        auto etsUiExtension = std::static_pointer_cast<EtsUIExtension>(shared_from_this());
        auto task = [etsUiExtension]() {
            if (etsUiExtension) {
                etsUiExtension->ConfigurationUpdated();
            }
        };
        if (handler_ != nullptr) {
            handler_->PostTask(task, "EtsUIExtension:OnChange");
        }
    }
}

void EtsUIExtension::RegisterDisplayInfoChangedListener()
{
    auto context = GetContext();
    if (context == nullptr || context->GetToken() == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Param is invalid");
        return;
    }
    Rosen::WindowManager::GetInstance().RegisterDisplayInfoChangedListener(
        context->GetToken(), etsUIExtensionAbilityDisplayListener_);
}

void EtsUIExtension::UnregisterDisplayInfoChangedListener()
{
    auto context = GetContext();
    if (context == nullptr || context->GetToken() == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Param is invalid");
        return;
    }
    Rosen::WindowManager::GetInstance().UnregisterDisplayInfoChangedListener(
        context->GetToken(), etsUIExtensionAbilityDisplayListener_);
}
#endif // SUPPORT_GRAPHICS
} // namespace AbilityRuntime
} // namespace OHOS

ETS_EXPORT extern "C" OHOS::AbilityRuntime::Extension *OHOS_ETS_Extension_Create(
    const std::unique_ptr<OHOS::AbilityRuntime::Runtime> &runtime)
{
    return OHOS::AbilityRuntime::EtsUIExtension::Create(runtime);
}