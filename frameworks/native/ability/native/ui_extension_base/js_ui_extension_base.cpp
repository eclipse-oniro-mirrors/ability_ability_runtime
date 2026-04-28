/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "js_ui_extension_base.h"

#include <type_traits>
#include <vector>

#include "ability_info.h"
#include "ability_manager_client.h"
#include "array_wrapper.h"
#include "configuration_utils.h"
#include "connection_manager.h"
#include "context.h"
#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "hitrace_meter.h"
#include "insight_intent_executor_mgr.h"
#include "int_wrapper.h"
#include "js_ability_lifecycle_callback.h"
#include "js_data_struct_converter.h"
#include "js_extension_common.h"
#include "js_extension_context.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "js_ui_extension_content_session.h"
#include "js_ui_extension_context.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "napi_common_configuration.h"
#include "napi_common_util.h"
#include "napi_common_want.h"
#include "napi_remote_object.h"
#include "string_wrapper.h"
#include "ui_extension_window_command.h"
#include "want_params_wrapper.h"
#include "application_configuration_manager.h"
#include "ohos_application.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr size_t ARGC_ONE = 1;
constexpr size_t ARGC_TWO = 2;
constexpr const char* UIEXTENSION_LAUNCH_TIMESTAMP_HIGH = "ohos.ability.params.uiExtensionLaunchTimestampHigh";
constexpr const char* UIEXTENSION_LAUNCH_TIMESTAMP_LOW = "ohos.ability.params.uiExtensionLaunchTimestampLow";
} // namespace
napi_value AttachUIExtensionBaseContext(napi_env env, void *value, void*)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "called");
    HandleEscape handleEscape(env);
    if (value == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null value");
        return nullptr;
    }

    auto ptr = reinterpret_cast<std::weak_ptr<UIExtensionContext>*>(value)->lock();
    if (ptr == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null ptr");
        return nullptr;
    }
    napi_value object = JsUIExtensionContext::CreateJsUIExtensionContext(env, ptr);
    if (object == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null object");
        return nullptr;
    }
    auto contextRef = JsRuntime::LoadSystemModuleByEngine(
        env, "application.UIExtensionContext", &object, 1);
    if (contextRef == nullptr) {
        TAG_LOGD(AAFwkTag::UI_EXT, "get LoadSystemModuleByEngine failed");
        return nullptr;
    }
    auto contextObj = contextRef->GetNapiValue();
    if (contextObj == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null contextObj");
        return nullptr;
    }
    if (!CheckTypeForNapiValue(env, contextObj, napi_object)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "not object");
        return nullptr;
    }
    napi_coerce_to_native_binding_object(
        env, contextObj, DetachCallbackFunc, AttachUIExtensionBaseContext, value, nullptr);
    auto workContext = new (std::nothrow) std::weak_ptr<UIExtensionContext>(ptr);
    napi_status status = napi_wrap(env, contextObj, workContext,
        [](napi_env, void *data, void*) {
            TAG_LOGD(AAFwkTag::UI_EXT, "Finalizer for weak_ptr ui extension context is called");
            if (data == nullptr) {
                TAG_LOGE(AAFwkTag::UI_EXT, "null data");
                return;
            }
            delete static_cast<std::weak_ptr<UIExtensionContext>*>(data);
        },
        nullptr, nullptr);
    if (status != napi_ok && workContext != nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "napi_wrap Failed: %{public}d", status);
        delete workContext;
        return nullptr;
    }

    return handleEscape.Escape(contextObj);
}

JsUIExtensionBase::JsUIExtensionBase(const std::unique_ptr<Runtime> &runtime)
    : jsRuntime_(static_cast<JsRuntime&>(*runtime))
{
    abilityResultListeners_ = std::make_shared<AbilityResultListeners>();
}

JsUIExtensionBase::~JsUIExtensionBase()
{
    TAG_LOGD(AAFwkTag::UI_EXT, "destructor");
    jsRuntime_.FreeNativeReference(std::move(jsObj_));
    jsRuntime_.FreeNativeReference(std::move(shellContextRef_));
    for (auto &item : contentSessions_) {
        jsRuntime_.FreeNativeReference(std::move(item.second));
    }
    contentSessions_.clear();
}

std::shared_ptr<ExtensionCommon> JsUIExtensionBase::Init(const std::shared_ptr<AbilityLocalRecord> &record,
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
    srcPath.erase(srcPath.rfind('.'));
    srcPath.append(".abc");

    std::string moduleName(abilityInfo_->moduleName);
    moduleName.append("::").append(abilityInfo_->name);
    HandleScope handleScope(jsRuntime_);

    jsObj_ = jsRuntime_.LoadModule(
        moduleName, srcPath, abilityInfo_->hapPath, abilityInfo_->compileMode == CompileMode::ES_MODULE, false,
        abilityInfo_->srcEntrance);
    if (jsObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null jsObj_");
        return nullptr;
    }

    BindContext();
    handler_ = handler;
    RegisterDisplayInfoChangedListener();
    return JsExtensionCommon::Create(jsRuntime_, static_cast<NativeReference&>(*jsObj_), shellContextRef_);
}

void JsUIExtensionBase::RegisterAbilityConfigUpdateCallback()
{
    auto uiExtensionAbility = std::static_pointer_cast<JsUIExtensionBase>(shared_from_this());
    std::weak_ptr<JsUIExtensionBase> abilityWptr = uiExtensionAbility;
    std::weak_ptr<UIExtensionContext> abilityContextWptr = context_;
    context_->RegisterAbilityConfigUpdateCallback(
        [abilityWptr, abilityContextWptr](AppExecFwk::Configuration &config) {
        std::shared_ptr<JsUIExtensionBase> abilitySptr = abilityWptr.lock();
        if (abilitySptr == nullptr) {
            TAG_LOGE(AAFwkTag::UIABILITY, "null abilitySptr");
            return;
        }
        std::shared_ptr<UIExtensionContext> abilityContext = abilityContextWptr.lock();
        if (abilityContext == nullptr || abilityContext->GetAbilityInfo() == nullptr) {
            TAG_LOGE(AAFwkTag::UIABILITY, "null abilityContext or null GetAbilityInfo");
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
            TAG_LOGD(AAFwkTag::UIABILITY, "%{public}zu",
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

void JsUIExtensionBase::BindContext()
{
    HandleScope handleScope(jsRuntime_);
    if (jsObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null jsObj_");
        return;
    }
    napi_env env = jsRuntime_.GetNapiEnv();
    napi_value obj = jsObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "not object");
        return;
    }
    if (context_ == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null context_");
        return;
    }
    TAG_LOGD(AAFwkTag::UI_EXT, "BindContext CreateJsUIExtensionContext");
    napi_value contextObj = JsUIExtensionContext::CreateJsUIExtensionContext(env, context_);
    if (contextObj == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null contextObj");
        return;
    }
    shellContextRef_ = JsRuntime::LoadSystemModuleByEngine(
        env, "application.UIExtensionContext", &contextObj, ARGC_ONE);
    if (shellContextRef_ == nullptr) {
        TAG_LOGD(AAFwkTag::UI_EXT, "get LoadSystemModuleByEngine failed");
        return;
    }
    contextObj = shellContextRef_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, contextObj, napi_object)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "get object failed");
        return;
    }
    auto workContext = new (std::nothrow) std::weak_ptr<UIExtensionContext>(context_);
    napi_coerce_to_native_binding_object(
        env, contextObj, DetachCallbackFunc, AttachUIExtensionBaseContext, workContext, nullptr);
    context_->Bind(jsRuntime_, shellContextRef_.get());
    napi_set_named_property(env, obj, "context", contextObj);
    napi_status status = napi_wrap(env, contextObj, workContext,
        [](napi_env, void *data, void*) {
            TAG_LOGD(AAFwkTag::UI_EXT, "Finalizer for weak_ptr ui extension context is called");
            if (data == nullptr) {
                TAG_LOGE(AAFwkTag::UI_EXT, "null data");
                return;
            }
            delete static_cast<std::weak_ptr<UIExtensionContext>*>(data);
        },
        nullptr, nullptr);
    if (status != napi_ok && workContext != nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "napi_wrap Failed: %{public}d", status);
        delete workContext;
        return;
    }
}

void JsUIExtensionBase::OnStart(
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

    HandleScope handleScope(jsRuntime_);
    napi_env env = jsRuntime_.GetNapiEnv();

    if (context_ != nullptr) {
        JsExtensionContext::ConfigurationUpdated(env, shellContextRef_, context_->GetConfiguration());
    }
    napi_value napiWant = OHOS::AppExecFwk::WrapWant(env, want);
    if (InsightIntentExecuteParam::IsInsightIntentExecute(want)) {
        launchParam.launchReason = AAFwk::LaunchReason::LAUNCHREASON_INSIGHT_INTENT;
    }
    napi_value argv[] = {
        CreateJsLaunchParam(env, launchParam),
        napiWant
    };
    CallObjectMethod("onCreate", argv, ARGC_TWO);
}

void JsUIExtensionBase::OnStop()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UI_EXT, "called");
    if (context_) {
        TAG_LOGD(AAFwkTag::UI_EXT, "set terminating true");
        context_->SetTerminating(true);
    }
    HandleScope handleScope(jsRuntime_);
    CallObjectMethod("onDestroy");
    ApplicationConfigurationManager::GetInstance().DeleteIgnoreContext(context_);
    TAG_LOGI(AAFwkTag::UIABILITY, "GetIgnoreContext size %{public}zu",
        AbilityRuntime::ApplicationConfigurationManager::GetInstance().GetIgnoreContext().size());
#ifdef SUPPORT_GRAPHICS
    UnregisterDisplayInfoChangedListener();
#endif // SUPPORT_GRAPHICS
    OnStopCallBack();
    TAG_LOGD(AAFwkTag::UI_EXT, "end");
}

void JsUIExtensionBase::OnStop(AppExecFwk::AbilityTransactionCallbackInfo<> *callbackInfo, bool &isAsyncCallback)
{
    if (callbackInfo == nullptr) {
        isAsyncCallback = false;
        OnStop();
        return;
    }
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UI_EXT, "begin");
    if (context_) {
        TAG_LOGD(AAFwkTag::UI_EXT, "set terminating true");
        context_->SetTerminating(true);
    }
    ApplicationConfigurationManager::GetInstance().DeleteIgnoreContext(context_);
    HandleScope handleScope(jsRuntime_);
    napi_value result = CallObjectMethod("onDestroy", nullptr, 0, true);
    if (!CheckPromise(result)) {
        OnStopCallBack();
        isAsyncCallback = false;
        return;
    }

    auto asyncCallback = [extensionWeakPtr = weak_from_this()]() {
        auto jsUIExtensionBase = extensionWeakPtr.lock();
        if (jsUIExtensionBase == nullptr) {
            TAG_LOGE(AAFwkTag::UI_EXT, "null extension");
            return;
        }
        jsUIExtensionBase->OnStopCallBack();
    };
    callbackInfo->Push(asyncCallback);
    isAsyncCallback = CallPromise(result, callbackInfo);
    if (!isAsyncCallback) {
        TAG_LOGE(AAFwkTag::UI_EXT, "call promise failed");
        OnStopCallBack();
    }
    TAG_LOGD(AAFwkTag::UI_EXT, "end");
}

void JsUIExtensionBase::OnStopCallBack()
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
        JsAbilityLifecycleCallbackArgs ability(jsObj_);
        applicationContext->DispatchOnAbilityDestroy(ability);
    }
}

bool JsUIExtensionBase::CheckPromise(napi_value result)
{
    if (result == nullptr) {
        TAG_LOGD(AAFwkTag::UI_EXT, "null result");
        return false;
    }
    HandleScope handleScope(jsRuntime_);
    napi_env env = jsRuntime_.GetNapiEnv();
    bool isPromise = false;
    napi_is_promise(env, result, &isPromise);
    if (!isPromise) {
        TAG_LOGD(AAFwkTag::UI_EXT, "result not promise");
        return false;
    }
    return true;
}

namespace {
napi_value PromiseCallback(napi_env env, napi_callback_info info)
{
    void *data = nullptr;
    NAPI_CALL_NO_THROW(napi_get_cb_info(env, info, nullptr, nullptr, nullptr, &data), nullptr);
    auto *callbackInfo = static_cast<AppExecFwk::AbilityTransactionCallbackInfo<> *>(data);
    if (callbackInfo == nullptr) {
        TAG_LOGD(AAFwkTag::UI_EXT, "Invalid input");
        return nullptr;
    }
    callbackInfo->Call();
    AppExecFwk::AbilityTransactionCallbackInfo<>::Destroy(callbackInfo);
    data = nullptr;
    return nullptr;
}
}

bool JsUIExtensionBase::CallPromise(napi_value result, AppExecFwk::AbilityTransactionCallbackInfo<> *callbackInfo)
{
    HandleScope handleScope(jsRuntime_);
    napi_env env = jsRuntime_.GetNapiEnv();
    if (!CheckTypeForNapiValue(env, result, napi_object)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "convert value failed");
        return false;
    }
    napi_value then = nullptr;
    napi_get_named_property(env, result, "then", &then);
    if (then == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null then");
        return false;
    }
    bool isCallable = false;
    napi_is_callable(env, then, &isCallable);
    if (!isCallable) {
        TAG_LOGE(AAFwkTag::UI_EXT, "not callable property then");
        return false;
    }
    napi_value promiseCallback = nullptr;
    napi_status createStatus = napi_create_function(env, "promiseCallback", strlen("promiseCallback"), PromiseCallback,
        callbackInfo, &promiseCallback);
    if (createStatus != napi_ok) {
        TAG_LOGE(AAFwkTag::UI_EXT, "failed, %{public}d", createStatus);
        return false;
    }
    napi_value argv[1] = { promiseCallback };
    napi_status callStatus = napi_call_function(env, result, then, 1, argv, nullptr);
    if (callStatus != napi_ok) {
        TAG_LOGE(AAFwkTag::UI_EXT, "failed, %{public}d", callStatus);
        return false;
    }
    TAG_LOGD(AAFwkTag::UI_EXT, "exit");
    return true;
}

void JsUIExtensionBase::OnCommandWindow(
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

void JsUIExtensionBase::ForegroundWindowInitInsightIntentExecutorInfo(const AAFwk::Want &want,
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
    executorInfo.pageLoader = contentSessions_[sessionInfo->uiExtensionComponentId];
    executorInfo.executeParam = std::make_shared<InsightIntentExecuteParam>();
    InsightIntentExecuteParam::GenerateFromWant(want, *executorInfo.executeParam);
    executorInfo.executeParam->executeMode_ = UI_EXTENSION_ABILITY;
    executorInfo.srcEntry = want.GetStringParam(INSIGHT_INTENT_SRC_ENTRY);
    TAG_LOGD(AAFwkTag::UI_EXT, "executorInfo, insightIntentId: %{public}" PRIu64,
        executorInfo.executeParam->insightIntentId_);
    return;
}

bool JsUIExtensionBase::ForegroundWindowWithInsightIntent(const AAFwk::Want &want,
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
            if (result.uris.size() > 0) {
                extension->ExecuteInsightIntentDone(executeParam.insightIntentId_, result);
            }
            extension->PostInsightIntentExecuted(sessionInfo, result, needForeground);
        });

    InsightIntentExecutorInfo executorInfo;
    ForegroundWindowInitInsightIntentExecutorInfo(want, sessionInfo, executorInfo);
    int32_t ret = DelayedSingleton<InsightIntentExecutorMgr>::GetInstance()->ExecuteInsightIntent(
        jsRuntime_, executorInfo, std::move(executorCallback));
    if (!ret) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Execute insight intent failed");
        // callback has removed, release in insight intent executor.
    }
    TAG_LOGD(AAFwkTag::UI_EXT, "end");
    return true;
}

void JsUIExtensionBase::ExecuteInsightIntentDone(uint64_t intentId, const InsightIntentExecuteResult &result)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "intentId %{public}" PRIu64"", intentId);
    auto ret = AAFwk::AbilityManagerClient::GetInstance()->ExecuteInsightIntentDone(token_, intentId, result);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "notify execute done failed");
    }
}

void JsUIExtensionBase::PostInsightIntentExecuted(const sptr<AAFwk::SessionInfo> &sessionInfo,
    const AppExecFwk::InsightIntentExecuteResult &result, bool needForeground)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UI_EXT, "Post insightintent executed");
    if (needForeground) {
        // If uiextensionability is started for the first time or need move background to foreground.
        HandleScope handleScope(jsRuntime_);
        CallObjectMethod("onForeground");
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

void JsUIExtensionBase::OnCommandWindowDone(const sptr<AAFwk::SessionInfo> &sessionInfo, AAFwk::WindowCommand winCmd)
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

void JsUIExtensionBase::OnInsightIntentExecuteDone(const sptr<AAFwk::SessionInfo> &sessionInfo,
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

        Rosen::WMError ret = Rosen::WMError::WM_OK;
        if (result.isDecorator && result.result) {
            sptr<AAFwk::IWantParams> pWantParams = WantParamWrapper::Box(*result.result);
            params.SetParam(INSIGHT_INTENT_EXECUTE_RESULT, pWantParams);
            ret = res->second->TransferExtensionData(params);
        } else {
            WantParams resultParams;
            resultParams.SetParam("code", Integer::Box(result.code));
            if (result.result != nullptr) {
                sptr<AAFwk::IWantParams> pWantParams = WantParamWrapper::Box(*result.result);
                resultParams.SetParam("result", pWantParams);
            }
            auto size = result.uris.size();
            sptr<IArray> uriArray = new (std::nothrow) Array(size, g_IID_IString);
            if (uriArray == nullptr) {
                TAG_LOGE(AAFwkTag::UI_EXT, "new uriArray failed");
                return;
            }
            for (std::size_t i = 0; i < size; i++) {
                uriArray->Set(i, String::Box(result.uris[i]));
            }
            resultParams.SetParam("uris", uriArray);
            resultParams.SetParam("flags", Integer::Box(result.flags));
            sptr<AAFwk::IWantParams> pWantParams = WantParamWrapper::Box(resultParams);
            params.SetParam(INSIGHT_INTENT_EXECUTE_RESULT, pWantParams);

            ret = res->second->TransferExtensionData(params);
        }

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

void JsUIExtensionBase::OnCommand(const AAFwk::Want &want, bool restart, int32_t startId)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "called");
    HandleScope handleScope(jsRuntime_);
    napi_env env = jsRuntime_.GetNapiEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null env");
        return;
    }
    napi_value napiWant = OHOS::AppExecFwk::WrapWant(env, want);
    if (napiWant == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null want");
        return;
    }
    napi_value napiStartId = nullptr;
    napi_create_int32(env, startId, &napiStartId);
    if (napiStartId == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null startId");
        return;
    }
    napi_value argv[] = { napiWant, napiStartId };
    CallObjectMethod("onRequest", argv, ARGC_TWO);
}

void JsUIExtensionBase::OnForeground(const Want &want, sptr<AAFwk::SessionInfo> sessionInfo)
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
    HandleScope handleScope(jsRuntime_);
    CallObjectMethod("onForeground");
}

void JsUIExtensionBase::OnBackground()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UI_EXT, "called");
    HandleScope handleScope(jsRuntime_);
    CallObjectMethod("onBackground");
}

bool JsUIExtensionBase::CallJsOnSessionCreate(const AAFwk::Want &want, const sptr<AAFwk::SessionInfo> &sessionInfo,
    const sptr<Rosen::Window> &uiWindow, const uint64_t &uiExtensionComponentId)
{
    HandleScope handleScope(jsRuntime_);
    napi_env env = jsRuntime_.GetNapiEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null env");
        return false;
    }
    napi_value napiWant = OHOS::AppExecFwk::WrapWant(env, want);
    if (napiWant == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null want");
        return false;
    }
    napi_value nativeContentSession = JsUIExtensionContentSession::CreateJsUIExtensionContentSession(
        env, sessionInfo, uiWindow, context_, abilityResultListeners_);
    if (nativeContentSession == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null contentSession");
        return false;
    }
    napi_ref ref = nullptr;
    napi_create_reference(env, nativeContentSession, 1, &ref);
    contentSessions_.emplace(
        uiExtensionComponentId, std::shared_ptr<NativeReference>(reinterpret_cast<NativeReference*>(ref)));
    napi_value argv[] = { napiWant, nativeContentSession };
    CallObjectMethod("onSessionCreate", argv, ARGC_TWO);
    return true;
}

sptr<Rosen::WindowOption> JsUIExtensionBase::CreateWindowOption(const sptr<AAFwk::SessionInfo> &sessionInfo,
    const AAFwk::Want &want)
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
    int64_t launchTimestamp = -1;
    bool hasHigh = want.HasParameter(UIEXTENSION_LAUNCH_TIMESTAMP_HIGH);
    bool hasLow = want.HasParameter(UIEXTENSION_LAUNCH_TIMESTAMP_LOW);
    if (hasHigh && hasLow) {
        uint32_t high = static_cast<uint32_t>(want.GetIntParam(UIEXTENSION_LAUNCH_TIMESTAMP_HIGH, -1));
        uint32_t low = static_cast<uint32_t>(want.GetIntParam(UIEXTENSION_LAUNCH_TIMESTAMP_LOW, -1));
        uint64_t temp = (static_cast<uint64_t>(high) << 32) | low;
        launchTimestamp = static_cast<int64_t>(temp);
    }
    if (context_->GetAbilityInfo()->extensionAbilityType ==
        AppExecFwk::ExtensionAbilityType::AGENT_UI) {
        option->SetIsBlockSubwindow(true);
    }
    option->SetStartModalExtensionTimeStamp(launchTimestamp);
    return option;
}

bool JsUIExtensionBase::HandleSessionCreate(const AAFwk::Want &want, const sptr<AAFwk::SessionInfo> &sessionInfo)
{
    if (sessionInfo == nullptr || sessionInfo->uiExtensionComponentId == 0) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Invalid sessionInfo");
        return false;
    }
    TAG_LOGD(AAFwkTag::UI_EXT, "UIExtension component id: %{public}" PRId64 ", bundle/ability:%{public}s/%{public}s",
        sessionInfo->uiExtensionComponentId, want.GetElement().GetBundleName().c_str(),
        want.GetElement().GetAbilityName().c_str());
    if (sessionInfo == nullptr || sessionInfo->uiExtensionComponentId == 0) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Invalid sessionInfo");
        return false;
    }
    std::shared_ptr<AAFwk::Want> sharedWant = std::make_shared<AAFwk::Want>(want);
    auto componentId = sessionInfo->uiExtensionComponentId;
    if (uiWindowMap_.find(componentId) == uiWindowMap_.end()) {
        if (context_ == nullptr || context_->GetAbilityInfo() == nullptr) {
            TAG_LOGE(AAFwkTag::UI_EXT, "null context");
            return false;
        }
        auto option = CreateWindowOption(sessionInfo, want);
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
        sharedWant->RemoveParam(UIEXTENSION_LAUNCH_TIMESTAMP_HIGH);
        sharedWant->RemoveParam(UIEXTENSION_LAUNCH_TIMESTAMP_LOW);
        uiWindow->UpdateExtensionConfig(sharedWant);
        if (!CallJsOnSessionCreate(*sharedWant, sessionInfo, uiWindow, componentId)) {
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

void JsUIExtensionBase::ForegroundWindow(const AAFwk::Want &want, const sptr<AAFwk::SessionInfo> &sessionInfo)
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

void JsUIExtensionBase::BackgroundWindow(const sptr<AAFwk::SessionInfo> &sessionInfo)
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

void JsUIExtensionBase::DestroyWindow(const sptr<AAFwk::SessionInfo> &sessionInfo)
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
        HandleScope handleScope(jsRuntime_);
        napi_value argv[] = { contentSessions_[componentId]->GetNapiValue() };
        CallObjectMethod("onSessionDestroy", argv, ARGC_ONE);
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

napi_value JsUIExtensionBase::CallObjectMethod(const char *name, napi_value const *argv, size_t argc, bool withResult)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "CallObjectMethod(%{public}s), begin", name);
    if (!jsObj_) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Not found .js file");
        return nullptr;
    }
    napi_env env = jsRuntime_.GetNapiEnv();
    napi_value obj = jsObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "get object failed");
        return nullptr;
    }
    HandleEscape handleEscape(jsRuntime_);
    napi_value method = nullptr;
    napi_get_named_property(env, obj, name, &method);
    if (!CheckTypeForNapiValue(env, method, napi_function)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "get '%{public}s' object failed", name);
        return nullptr;
    }
    if (withResult) {
        napi_value result = nullptr;
        napi_call_function(env, obj, method, argc, argv, &result);
        return handleEscape.Escape(result);
    }
    TAG_LOGD(AAFwkTag::UI_EXT, "CallFunction(%{public}s), success", name);
    napi_value result = nullptr;
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    napi_call_function(env, obj, method, argc, argv, &result);
    return result;
}

void JsUIExtensionBase::OnConfigurationUpdated(const AppExecFwk::Configuration &configuration)
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
            TAG_LOGI(AAFwkTag::UIABILITY, "newConfig: %{public}s", newConfig.GetName().c_str());
            auto diffConfiguration = std::make_shared<AppExecFwk::Configuration>(newConfig);
            context_->GetWindow()->UpdateConfigurationForSpecified(diffConfiguration, context_->GetResourceManager());
        }
    } else {
        auto configUtils = std::make_shared<ConfigurationUtils>();
        configUtils->UpdateGlobalConfig(configuration, context_->GetConfiguration(), context_->GetResourceManager());
    }

    ConfigurationUpdated();
}

void JsUIExtensionBase::OnAbilityConfigurationUpdated(const AppExecFwk::Configuration& configuration)
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
        TAG_LOGI(AAFwkTag::UIABILITY, "newConfig: %{public}s", configuration.GetName().c_str());
        auto diffConfiguration = std::make_shared<AppExecFwk::Configuration>(configuration);
        context_->GetWindow()->UpdateConfigurationForSpecified(diffConfiguration, context_->GetResourceManager());
    }

    ConfigurationUpdated();
}

void JsUIExtensionBase::Dump(const std::vector<std::string> &params, std::vector<std::string> &info)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "called");
    HandleScope handleScope(jsRuntime_);
    napi_env env = jsRuntime_.GetNapiEnv();
    napi_value argv[] = { CreateNativeArray(env, params) };

    if (!jsObj_) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Not found .js file");
        return;
    }
    napi_value obj = jsObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "get object failed");
        return;
    }

    napi_value method = nullptr;
    napi_get_named_property(env, obj, "onDump", &method);
    if (!CheckTypeForNapiValue(env, method, napi_function)) {
        method = nullptr;
        napi_get_named_property(env, obj, "dump", &method);
        if (!CheckTypeForNapiValue(env, method, napi_function)) {
            TAG_LOGE(AAFwkTag::UI_EXT, "get onDump failed");
            return;
        }
    }
    napi_value dumpInfo = nullptr;
    napi_call_function(env, obj, method, ARGC_ONE, argv, &dumpInfo);
    if (dumpInfo == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null dumpInfo");
        return;
    }
    uint32_t len = 0;
    napi_get_array_length(env, dumpInfo, &len);
    for (uint32_t i = 0; i < len; i++) {
        std::string dumpInfoStr;
        napi_value element = nullptr;
        napi_get_element(env, dumpInfo, i, &element);
        if (!ConvertFromJsValue(env, element, dumpInfoStr)) {
            TAG_LOGE(AAFwkTag::UI_EXT, "Parse dumpInfoStr error");
            return;
        }
        info.push_back(dumpInfoStr);
    }
    TAG_LOGD(AAFwkTag::UI_EXT, "Dump info size: %{public}zu", info.size());
}

void JsUIExtensionBase::OnAbilityResult(int32_t requestCode, int32_t resultCode, const Want &resultData)
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

void JsUIExtensionBase::SetAbilityInfo(const std::shared_ptr<AppExecFwk::AbilityInfo> &abilityInfo)
{
    abilityInfo_ = abilityInfo;
}

void JsUIExtensionBase::SetContext(const std::shared_ptr<UIExtensionContext> &context)
{
    context_ = context;
}

void JsUIExtensionBase::ConfigurationUpdated()
{
    TAG_LOGD(AAFwkTag::UI_EXT, "begin");
    HandleScope handleScope(jsRuntime_);
    napi_env env = jsRuntime_.GetNapiEnv();

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
    TAG_LOGD(AAFwkTag::UIABILITY, "realConfig: %{public}s", realConfig.GetName().c_str());
    auto realConfigPtr = std::make_shared<Configuration>(realConfig);
    JsExtensionContext::ConfigurationUpdated(env, shellContextRef_, realConfigPtr);

    napi_value napiConfiguration = OHOS::AppExecFwk::WrapConfiguration(env, realConfig);
    CallObjectMethod("onConfigurationUpdate", &napiConfiguration, ARGC_ONE);
}

#ifdef SUPPORT_GRAPHICS
void JsUIExtensionBase::OnDisplayInfoChange(
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
        auto jsUiExtension = std::static_pointer_cast<JsUIExtensionBase>(shared_from_this());
        auto task = [jsUiExtension]() {
            if (jsUiExtension) {
                jsUiExtension->ConfigurationUpdated();
            }
        };
        if (handler_ != nullptr) {
            handler_->PostTask(task, "JsUIExtensionBase:OnChange");
        }
    }
}

void JsUIExtensionBase::RegisterDisplayInfoChangedListener()
{
    // register displayid change callback
    auto jsUiExtensionBase = std::static_pointer_cast<JsUIExtensionBase>(shared_from_this());
    jsUIExtensionBaseDisplayListener_ = sptr<JsUIExtensionBaseDisplayListener>::MakeSptr(jsUiExtensionBase);
    if (jsUIExtensionBaseDisplayListener_ == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null JsUIExtensionBaseDisplayListener");
        return;
    }
    if (context_ == nullptr || context_->GetToken() == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null context");
        return;
    }
    TAG_LOGI(AAFwkTag::UI_EXT, "RegisterDisplayInfoChangedListener");
    Rosen::WindowManager::GetInstance().RegisterDisplayInfoChangedListener(
        context_->GetToken(), jsUIExtensionBaseDisplayListener_);
}

void JsUIExtensionBase::UnregisterDisplayInfoChangedListener()
{
    if (context_ == nullptr || context_->GetToken() == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null context");
        return;
    }
    Rosen::WindowManager::GetInstance().UnregisterDisplayInfoChangedListener(
        context_->GetToken(), jsUIExtensionBaseDisplayListener_);
}
#endif // SUPPORT_GRAPHICS
} // namespace AbilityRuntime
} // namespace OHOS
