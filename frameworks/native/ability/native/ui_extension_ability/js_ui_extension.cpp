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

#include "js_ui_extension.h"
#include "ability_context.h"
#include "ability_delegator_registry.h"
#include "ability_info.h"
#include "ability_manager_client.h"
#include "ability_start_setting.h"
#include "array_wrapper.h"
#include "configuration_utils.h"
#include "connection_manager.h"
#include "context.h"
#include "freeze_util.h"
#include "hitrace_meter.h"
#include "hilog_tag_wrapper.h"
#include "insight_intent_delay_result_callback_mgr.h"
#include "insight_intent_executor_mgr.h"
#include "int_wrapper.h"
#include "js_ability_lifecycle_callback.h"
#include "js_data_struct_converter.h"
#include "js_embeddable_ui_ability_context.h"
#include "js_embeddable_window_stage.h"
#include "js_extension_common.h"
#include "js_extension_context.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "js_ui_extension_content_session.h"
#include "js_ui_extension_context.h"
#include "js_utils.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "napi_common_configuration.h"
#include "napi_common_want.h"
#include "napi_remote_object.h"
#include "string_wrapper.h"
#include "ui_extension_window_command.h"
#include "want_params_wrapper.h"
#include "ohos_application.h"

namespace OHOS {
namespace AbilityRuntime {
using namespace OHOS::AppExecFwk;
namespace {
constexpr size_t ARGC_ONE = 1;
constexpr size_t ARGC_TWO = 2;
constexpr const char* UIEXTENSION_LAUNCH_TIMESTAMP_HIGH = "ohos.ability.params.uiExtensionLaunchTimestampHigh";
constexpr const char* UIEXTENSION_LAUNCH_TIMESTAMP_LOW = "ohos.ability.params.uiExtensionLaunchTimestampLow";

bool IsEmbeddableStart(int32_t screenMode)
{
    return screenMode == AAFwk::EMBEDDED_FULL_SCREEN_MODE ||
        screenMode == AAFwk::EMBEDDED_HALF_SCREEN_MODE;
}
}

napi_value AttachUIExtensionContext(napi_env env, void *value, void *extValue)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "called");
    HandleEscape handleEscape(env);
    if (value == nullptr || extValue == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "invalid parameter");
        return nullptr;
    }

    auto ptr = reinterpret_cast<std::weak_ptr<UIExtensionContext> *>(value)->lock();
    if (ptr == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null ptr");
        return nullptr;
    }
    napi_value object = JsUIExtensionContext::CreateJsUIExtensionContext(env, ptr);
    auto contextRef = JsRuntime::LoadSystemModuleByEngine(env, "application.UIExtensionContext",
        &object, 1);
    if (contextRef == nullptr) {
        TAG_LOGD(AAFwkTag::UI_EXT, "null contextRef");
        return nullptr;
    }
    auto contextObj = contextRef->GetNapiValue();
    if (contextObj == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null contextObj");
        return nullptr;
    }
    napi_coerce_to_native_binding_object(env, contextObj, DetachCallbackFunc,
        AttachUIExtensionContext, value, extValue);
    auto workContext = new (std::nothrow) std::weak_ptr<UIExtensionContext>(ptr);
    napi_status status = napi_wrap(env, contextObj, workContext,
        [](napi_env, void *data, void *) {
            TAG_LOGD(AAFwkTag::UI_EXT, "Finalizer for weak_ptr ui extension context is called");
            delete static_cast<std::weak_ptr<UIExtensionContext> *>(data);
        },
        nullptr, nullptr);
    if (status != napi_ok && workContext != nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "napi_wrap Failed: %{public}d", status);
        delete workContext;
        return nullptr;
    }

    return handleEscape.Escape(contextObj);
}

JsUIExtension* JsUIExtension::Create(const std::unique_ptr<Runtime>& runtime)
{
    return new (std::nothrow) JsUIExtension(static_cast<JsRuntime&>(*runtime));
}

JsUIExtension::JsUIExtension(JsRuntime& jsRuntime) : jsRuntime_(jsRuntime)
{
    abilityResultListeners_ = std::make_shared<AbilityResultListeners>();
}

JsUIExtension::~JsUIExtension()
{
    TAG_LOGD(AAFwkTag::UI_EXT, "Js ui extension destructor");
    auto context = GetContext();
    if (context) {
        context->Unbind();
    }

    jsRuntime_.FreeNativeReference(std::move(jsObj_));
    jsRuntime_.FreeNativeReference(std::move(shellContextRef_));
    for (auto &item : contentSessions_) {
        jsRuntime_.FreeNativeReference(std::move(item.second));
    }
    contentSessions_.clear();
    InsightIntentDelayResultCallbackMgr::GetInstance().RemoveDelayResultCallback(intentId_);
}

void JsUIExtension::Init(const std::shared_ptr<AbilityLocalRecord> &record,
    const std::shared_ptr<OHOSApplication> &application, std::shared_ptr<AbilityHandler> &handler,
    const sptr<IRemoteObject> &token)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UI_EXT, "init");
    CHECK_POINTER(record);
    UIExtension::Init(record, application, handler, token);
    if (Extension::abilityInfo_ == nullptr || Extension::abilityInfo_->srcEntrance.empty()) {
        TAG_LOGE(AAFwkTag::UI_EXT, "JsUIExtension Init abilityInfo error");
        return;
    }

    RegisterAbilityConfigUpdateCallback();

    if (record != nullptr) {
        token_ = record->GetToken();
    }
    std::string srcPath(Extension::abilityInfo_->moduleName + "/");
    srcPath.append(Extension::abilityInfo_->srcEntrance);
    srcPath.erase(srcPath.rfind('.'));
    srcPath.append(".abc");

    std::string moduleName(Extension::abilityInfo_->moduleName);
    moduleName.append("::").append(abilityInfo_->name);
    HandleScope handleScope(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();

    jsObj_ = jsRuntime_.LoadModule(
        moduleName, srcPath, abilityInfo_->hapPath, abilityInfo_->compileMode == CompileMode::ES_MODULE, false,
        abilityInfo_->srcEntrance);
    if (jsObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null jsObj_");
        return;
    }

    napi_value obj = jsObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "get object failed");
        return;
    }

    BindContext(env, obj, record->GetWant());

    SetExtensionCommon(
        JsExtensionCommon::Create(jsRuntime_, static_cast<NativeReference&>(*jsObj_), shellContextRef_));
    handler_ = handler;
    RegisterDisplayInfoChangedListener();
}

void JsUIExtension::CreateJSContext(napi_env env, napi_value &contextObj,
    std::shared_ptr<UIExtensionContext> context, int32_t screenMode)
{
    if (screenMode == AAFwk::IDLE_SCREEN_MODE) {
        contextObj = JsUIExtensionContext::CreateJsUIExtensionContext(env, context);
        CHECK_POINTER(contextObj);
        shellContextRef_ = JsRuntime::LoadSystemModuleByEngine(env, "application.UIExtensionContext",
            &contextObj, ARGC_ONE);
    } else {
        contextObj = JsEmbeddableUIAbilityContext::CreateJsEmbeddableUIAbilityContext(env,
            nullptr, context, screenMode);
        CHECK_POINTER(contextObj);
        shellContextRef_ = JsRuntime::LoadSystemModuleByEngine(env, "application.EmbeddableUIAbilityContext",
            &contextObj, ARGC_ONE);
    }
}

void JsUIExtension::BindContext(napi_env env, napi_value obj, std::shared_ptr<AAFwk::Want> want)
{
    HandleScope handleScope(env);
    auto context = GetContext();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null context");
        return;
    }
    TAG_LOGD(AAFwkTag::UI_EXT, "BindContext CreateJsUIExtensionContext");
    if (want == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null want");
        return;
    }
    int32_t screenMode = want->GetIntParam(AAFwk::SCREEN_MODE_KEY, AAFwk::IDLE_SCREEN_MODE);
    context->SetScreenMode(screenMode);
    napi_value contextObj = nullptr;
    CreateJSContext(env, contextObj, context, screenMode);
    if (shellContextRef_ == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null shellContextRef");
        return;
    }
    contextObj = shellContextRef_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, contextObj, napi_object)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "get object failed");
        return;
    }
    auto workContext = new (std::nothrow) std::weak_ptr<UIExtensionContext>(context);
    CHECK_POINTER(workContext);
    screenModePtr_ = std::make_shared<int32_t>(screenMode);
    auto workScreenMode = new (std::nothrow) std::weak_ptr<int32_t>(screenModePtr_);
    if (workScreenMode == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "workScreenMode is null");
        delete workContext;
        return;
    }
    napi_coerce_to_native_binding_object(
        env, contextObj, DetachCallbackFunc, AttachUIExtensionContext, workContext, workScreenMode);
    context->Bind(jsRuntime_, shellContextRef_.get());
    napi_set_named_property(env, obj, "context", contextObj);
    napi_status status = napi_wrap(env, contextObj, workContext,
        [](napi_env, void* data, void*) {
            TAG_LOGD(AAFwkTag::UI_EXT, "Finalizer for weak_ptr ui extension context is called");
            delete static_cast<std::weak_ptr<UIExtensionContext>*>(data);
        }, nullptr, nullptr);
    if (status != napi_ok && workContext != nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "napi_wrap Failed: %{public}d", status);
        delete workContext;
        return;
    }

    TAG_LOGD(AAFwkTag::UI_EXT, "end");
}

void JsUIExtension::OnStart(const AAFwk::Want &want, sptr<AAFwk::SessionInfo> sessionInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UI_EXT, "OnStart begin");
    Extension::OnStart(want);
    auto context = GetContext();
#ifdef SUPPORT_GRAPHICS
    if (context != nullptr && sessionInfo != nullptr) {
        auto configUtils = std::make_shared<ConfigurationUtils>();
        configUtils->InitDisplayConfig(context->GetConfiguration(), context->GetResourceManager(),
            sessionInfo->displayId, sessionInfo->density, sessionInfo->orientation);
    }
#endif // SUPPORT_GRAPHICS

    HandleScope handleScope(jsRuntime_);
    napi_env env = jsRuntime_.GetNapiEnv();
    if (context != nullptr) {
        JsExtensionContext::ConfigurationUpdated(env, shellContextRef_, context->GetConfiguration());
    }

    napi_value napiWant = OHOS::AppExecFwk::WrapWant(env, want);
    auto launchParam = Extension::GetLaunchParam();
    if (InsightIntentExecuteParam::IsInsightIntentExecute(want)) {
        launchParam.launchReason = AAFwk::LaunchReason::LAUNCHREASON_INSIGHT_INTENT;
    }
    int32_t screenMode = want.GetIntParam(AAFwk::SCREEN_MODE_KEY, AAFwk::IDLE_SCREEN_MODE);
    if (IsEmbeddableStart(screenMode)) {
        napi_value argv[] = {napiWant, CreateJsLaunchParam(env, launchParam) };
        CallObjectMethod("onCreate", argv, ARGC_TWO);
    } else {
        napi_value argv[] = {CreateJsLaunchParam(env, launchParam) };
        CallObjectMethod("onCreate", argv, ARGC_ONE);
    }
    TAG_LOGD(AAFwkTag::UI_EXT, "end");
}

void JsUIExtension::OnStop()
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
    TAG_LOGI(AAFwkTag::UIABILITY, "GetIgnoreContext size %{public}zu",
        AbilityRuntime::ApplicationConfigurationManager::GetInstance().GetIgnoreContext().size());
    HandleScope handleScope(jsRuntime_);
    CallObjectMethod("onDestroy");
#ifdef SUPPORT_GRAPHICS
    UnregisterDisplayInfoChangedListener();
#endif // SUPPORT_GRAPHICS
    OnStopCallBack();
    TAG_LOGD(AAFwkTag::UI_EXT, "end");
}
void JsUIExtension::OnStop(AppExecFwk::AbilityTransactionCallbackInfo<> *callbackInfo, bool &isAsyncCallback)
{
    if (callbackInfo == nullptr) {
        isAsyncCallback = false;
        OnStop();
        return;
    }
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UI_EXT, "begin");
    auto context = GetContext();
    if (context) {
        TAG_LOGD(AAFwkTag::UI_EXT, "set terminating true");
        context->SetTerminating(true);
    }
    UIExtension::OnStop();
    AbilityRuntime::ApplicationConfigurationManager::GetInstance().DeleteIgnoreContext(GetContext());
    HandleScope handleScope(jsRuntime_);
    napi_value result = CallObjectMethod("onDestroy", nullptr, 0, true);
#ifdef SUPPORT_GRAPHICS
    UnregisterDisplayInfoChangedListener();
#endif // SUPPORT_GRAPHICS
    if (!CheckPromise(result)) {
        OnStopCallBack();
        isAsyncCallback = false;
        return;
    }

    std::weak_ptr<Extension> weakPtr = shared_from_this();
    auto asyncCallback = [extensionWeakPtr = weakPtr]() {
        auto jsUIExtension = extensionWeakPtr.lock();
        if (jsUIExtension == nullptr) {
            TAG_LOGE(AAFwkTag::UI_EXT, "null extension");
            return;
        }
        jsUIExtension->OnStopCallBack();
    };
    callbackInfo->Push(asyncCallback);
    isAsyncCallback = CallPromise(result, callbackInfo);
    if (!isAsyncCallback) {
        TAG_LOGE(AAFwkTag::UI_EXT, "call promise failed");
        OnStopCallBack();
    }
    TAG_LOGD(AAFwkTag::UI_EXT, "end");
}

void JsUIExtension::OnStopCallBack()
{
    UIExtension::OnStopCallBack();

    auto applicationContext = Context::GetApplicationContext();
    if (applicationContext != nullptr) {
        JsAbilityLifecycleCallbackArgs ability(jsObj_);
        applicationContext->DispatchOnAbilityDestroy(ability);
    }
}

bool JsUIExtension::CheckPromise(napi_value result)
{
    if (result == nullptr) {
        TAG_LOGD(AAFwkTag::UI_EXT, "result is null, no need to call promise");
        return false;
    }
    napi_env env = jsRuntime_.GetNapiEnv();
    bool isPromise = false;
    napi_is_promise(env, result, &isPromise);
    if (!isPromise) {
        TAG_LOGD(AAFwkTag::UI_EXT, "result is not promise, no need to call promise");
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
        TAG_LOGD(AAFwkTag::UI_EXT, "Invalid input info");
        return nullptr;
    }
    callbackInfo->Call();
    AppExecFwk::AbilityTransactionCallbackInfo<>::Destroy(callbackInfo);
    data = nullptr;
    return nullptr;
}
}

bool JsUIExtension::CallPromise(napi_value result, AppExecFwk::AbilityTransactionCallbackInfo<> *callbackInfo)
{
    auto env = jsRuntime_.GetNapiEnv();
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
    HandleScope handleScope(jsRuntime_);
    napi_value promiseCallback = nullptr;
    napi_create_function(env, "promiseCallback", strlen("promiseCallback"), PromiseCallback,
        callbackInfo, &promiseCallback);
    napi_value argv[1] = { promiseCallback };
    napi_call_function(env, result, then, 1, argv, nullptr);
    TAG_LOGD(AAFwkTag::UI_EXT, "exit");
    return true;
}

sptr<IRemoteObject> JsUIExtension::OnConnect(const AAFwk::Want &want)
{
    HandleScope handleScope(jsRuntime_);
    napi_value result = CallOnConnect(want);
    napi_env env = jsRuntime_.GetNapiEnv();
    auto remoteObj = NAPI_ohos_rpc_getNativeRemoteObject(env, result);
    if (remoteObj == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null remoteObj");
    }
    return remoteObj;
}

void JsUIExtension::OnDisconnect(const AAFwk::Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    Extension::OnDisconnect(want);
    TAG_LOGD(AAFwkTag::UI_EXT, "JsUIExtension OnDisconnect begin");
    HandleScope handleScope(jsRuntime_);
    CallOnDisconnect(want, false);
    TAG_LOGD(AAFwkTag::UI_EXT, "JsUIExtension OnDisconnect end");
}

bool JsUIExtension::ForegroundWindowInitInsightIntentExecutorInfo(const AAFwk::Want &want,
    const sptr<AAFwk::SessionInfo> &sessionInfo, InsightIntentExecutorInfo &executorInfo)
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
    executorInfo.pageLoader = contentSessions_[sessionInfo->uiExtensionComponentId];
    executorInfo.executeParam = std::make_shared<InsightIntentExecuteParam>();
    InsightIntentExecuteParam::GenerateFromWant(want, *executorInfo.executeParam);
    executorInfo.executeParam->executeMode_ = UI_EXTENSION_ABILITY;
    executorInfo.srcEntry = want.GetStringParam(INSIGHT_INTENT_SRC_ENTRY);
    TAG_LOGD(AAFwkTag::UI_EXT, "executorInfo, insightIntentId: %{public}" PRIu64,
        executorInfo.executeParam->insightIntentId_);
    return true;
}

bool JsUIExtension::ForegroundWindowWithInsightIntent(const AAFwk::Want &want,
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

    auto uiExtension = std::static_pointer_cast<JsUIExtension>(shared_from_this());
    executorCallback->Push(
        [uiExtension, sessionInfo, needForeground, want](AppExecFwk::InsightIntentExecuteResult result) {
        TAG_LOGI(AAFwkTag::UI_EXT, "Execute post insightintent");
        if (uiExtension == nullptr) {
            TAG_LOGE(AAFwkTag::UI_EXT, "null uiExtension");
            return;
        }

        InsightIntentExecuteParam executeParam;
        InsightIntentExecuteParam::GenerateFromWant(want, executeParam);
        if (result.uris.size() > 0) {
            uiExtension->ExecuteInsightIntentDone(executeParam.insightIntentId_, result);
        }
        uiExtension->PostInsightIntentExecuted(sessionInfo, result, needForeground);
    });

    InsightIntentExecutorInfo executorInfo;
    if (!ForegroundWindowInitInsightIntentExecutorInfo(want, sessionInfo, executorInfo)) {
        return false;
    }
    InsightIntentDelayResultCallbackMgr::GetInstance().RemoveDelayResultCallback(intentId_);
    bool isDecorator = executorInfo.executeParam->decoratorType_ != static_cast<int8_t>(InsightIntentType::DECOR_NONE);
    RegisterUiExtensionDelayResultCallback(executorInfo.executeParam->insightIntentId_, sessionInfo, isDecorator);
    FreezeUtil::GetInstance().AddLifecycleEvent(token_, "JsUIExtension::ForegroundWindowWithInsightIntent begin");
    int32_t ret = DelayedSingleton<InsightIntentExecutorMgr>::GetInstance()->ExecuteInsightIntent(
        jsRuntime_, executorInfo, std::move(executorCallback));
    FreezeUtil::GetInstance().AddLifecycleEvent(token_, "JsUIExtension::ForegroundWindowWithInsightIntent end");
    if (!ret) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Execute insight intent failed");
        // callback has removed, release in insight intent executor.
        InsightIntentDelayResultCallbackMgr::GetInstance().RemoveDelayResultCallback(
            executorInfo.executeParam->insightIntentId_);
    } else {
        intentId_ = executorInfo.executeParam->insightIntentId_;
    }
    TAG_LOGD(AAFwkTag::UI_EXT, "end");
    return true;
}

void JsUIExtension::ExecuteInsightIntentDone(uint64_t intentId, const InsightIntentExecuteResult &result)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "intentId %{public}" PRIu64"", intentId);
    auto ret = AAFwk::AbilityManagerClient::GetInstance()->ExecuteInsightIntentDone(token_, intentId, result);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "notify execute done failed");
    }
}

void JsUIExtension::PostInsightIntentExecuted(const sptr<AAFwk::SessionInfo> &sessionInfo,
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

void JsUIExtension::OnCommand(const AAFwk::Want &want, bool restart, int startId)
{
    UIExtension::OnCommand(want, restart, startId);
    TAG_LOGD(AAFwkTag::UI_EXT, "restart=%{public}s, startId=%{public}d",
        restart ? "true" : "false", startId);
    // wrap want
    HandleScope handleScope(jsRuntime_);
    napi_env env = jsRuntime_.GetNapiEnv();
    napi_value napiWant = OHOS::AppExecFwk::WrapWant(env, want);
    // wrap startId
    napi_value napiStartId = nullptr;
    napi_create_int32(env, startId, &napiStartId);
    napi_value argv[] = {napiWant, napiStartId};
    CallObjectMethod("onRequest", argv, ARGC_TWO);
    TAG_LOGD(AAFwkTag::UI_EXT, "end");
}

void JsUIExtension::OnForeground(const Want &want, sptr<AAFwk::SessionInfo> sessionInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UI_EXT, "OnForeground begin");
    CHECK_POINTER(sessionInfo);
    Extension::OnForeground(want, sessionInfo);

    if (InsightIntentExecuteParam::IsInsightIntentExecute(want)) {
        bool finish = ForegroundWindowWithInsightIntent(want, sessionInfo, true);
        if (finish) {
            return;
        }
    }

    ForegroundWindow(want, sessionInfo);
    HandleScope handleScope(jsRuntime_);
    CallObjectMethod("onForeground");
    TAG_LOGD(AAFwkTag::UI_EXT, "end");
}

void JsUIExtension::OnBackground()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UI_EXT, "begin");
    HandleScope handleScope(jsRuntime_);
    CallObjectMethod("onBackground");
    Extension::OnBackground();
    TAG_LOGD(AAFwkTag::UI_EXT, "end");
}

bool JsUIExtension::HandleSessionCreate(const AAFwk::Want &want, const sptr<AAFwk::SessionInfo> &sessionInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (sessionInfo == nullptr || sessionInfo->uiExtensionComponentId == 0) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Invalid sessionInfo");
        return false;
    }
    std::lock_guard<std::mutex> lock(uiWindowMutex_);
    TAG_LOGD(AAFwkTag::UI_EXT, "UIExtension component id: %{public}" PRId64 ", bundle/ability: %{public}s/%{public}s",
        sessionInfo->uiExtensionComponentId, want.GetElement().GetBundleName().c_str(),
        want.GetElement().GetAbilityName().c_str());
    std::shared_ptr<AAFwk::Want> sharedWant = std::make_shared<AAFwk::Want>(want);
    auto compId = sessionInfo->uiExtensionComponentId;
    if (uiWindowMap_.find(compId) == uiWindowMap_.end()) {
        auto context = GetContext();
        auto uiWindow = CreateUIWindow(context, sessionInfo, want);
        if (uiWindow == nullptr) {
            TAG_LOGE(AAFwkTag::UI_EXT, "null uiWindow");
            return false;
        }
        sharedWant->RemoveParam(UIEXTENSION_LAUNCH_TIMESTAMP_HIGH);
        sharedWant->RemoveParam(UIEXTENSION_LAUNCH_TIMESTAMP_LOW);
        uiWindow->UpdateExtensionConfig(sharedWant);
        HandleScope handleScope(jsRuntime_);
        napi_env env = jsRuntime_.GetNapiEnv();
        napi_value napiWant = OHOS::AppExecFwk::WrapWant(env, *sharedWant);
        napi_value nativeContentSession = JsUIExtensionContentSession::CreateJsUIExtensionContentSession(
            env, sessionInfo, uiWindow, context, abilityResultListeners_);
        napi_ref ref = nullptr;
        napi_create_reference(env, nativeContentSession, 1, &ref);
        contentSessions_.emplace(compId, std::shared_ptr<NativeReference>(reinterpret_cast<NativeReference*>(ref)));
        int32_t screenMode = want.GetIntParam(AAFwk::SCREEN_MODE_KEY, AAFwk::IDLE_SCREEN_MODE);
        if (IsEmbeddableStart(screenMode)) {
            screenMode_ = screenMode;
            auto jsAppWindowStage = CreateAppWindowStage(uiWindow, sessionInfo);
            if (jsAppWindowStage == nullptr) {
                TAG_LOGE(AAFwkTag::UI_EXT, "null JsAppWindowStage");
                return false;
            }
            napi_value argv[] = {jsAppWindowStage->GetNapiValue()};
            CallObjectMethod("onWindowStageCreate", argv, ARGC_ONE);
        } else {
            napi_value argv[] = {napiWant, nativeContentSession};
            CallObjectMethod("onSessionCreate", argv, ARGC_TWO);
        }
        uiWindowMap_[compId] = uiWindow;
#ifdef SUPPORT_GRAPHICS
        if (context->GetWindow() == nullptr) {
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

sptr<Rosen::Window> JsUIExtension::CreateUIWindow(const std::shared_ptr<UIExtensionContext> context,
    const sptr<AAFwk::SessionInfo> &sessionInfo, const AAFwk::Want &want)
{
    if (context == nullptr || context->GetAbilityInfo() == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null context");
        return nullptr;
    }
    auto option = sptr<Rosen::WindowOption>::MakeSptr();
    if (option == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null option");
        return nullptr;
    }
    option->SetWindowName(context->GetBundleName() + context->GetAbilityInfo()->name);
    option->SetWindowType(Rosen::WindowType::WINDOW_TYPE_UI_EXTENSION);
    option->SetWindowSessionType(Rosen::WindowSessionType::EXTENSION_SESSION);
    option->SetParentId(sessionInfo->hostWindowId);
    option->SetRealParentId(sessionInfo->realHostWindowId);
    option->SetParentWindowType(static_cast<Rosen::WindowType>(sessionInfo->parentWindowType));
    option->SetUIExtensionUsage(static_cast<uint32_t>(sessionInfo->uiExtensionUsage));
    option->SetDensity(sessionInfo->density);
    option->SetIsDensityFollowHost(sessionInfo->isDensityFollowHost);
    option->SetDisplayId(sessionInfo->displayId);
    if (context->isNotAllow != -1) {
        bool isNotAllow = context->isNotAllow == 1 ? true : false;
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
    option->SetStartModalExtensionTimeStamp(launchTimestamp);
    HITRACE_METER_NAME(HITRACE_TAG_APP, "Rosen::Window::Create");
    return Rosen::Window::Create(option, GetContext(), sessionInfo->sessionToken);
}

std::unique_ptr<NativeReference> JsUIExtension::CreateAppWindowStage(sptr<Rosen::Window> uiWindow,
    sptr<AAFwk::SessionInfo> sessionInfo)
{
    auto env = jsRuntime_.GetNapiEnv();
    HandleScope handleScope(env);
    napi_value jsWindowStage = Rosen::JsEmbeddableWindowStage::CreateJsEmbeddableWindowStage(
        env, uiWindow, sessionInfo);
    if (jsWindowStage == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null jsWindowStage");
        return nullptr;
    }
    return JsRuntime::LoadSystemModuleByEngine(env, "application.embeddablewindowstage", &jsWindowStage, 1);
}

void JsUIExtension::DestroyWindow(const sptr<AAFwk::SessionInfo> &sessionInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (sessionInfo == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Invalid sessionInfo");
        return;
    }
    std::lock_guard<std::mutex> lock(uiWindowMutex_);
    auto componentId = sessionInfo->uiExtensionComponentId;
    if (uiWindowMap_.find(componentId) == uiWindowMap_.end()) {
        TAG_LOGE(AAFwkTag::UI_EXT, "find uiWindow wrong");
        return;
    }
    if (contentSessions_.find(componentId) != contentSessions_.end() && contentSessions_[componentId] != nullptr) {
        HandleScope handleScope(jsRuntime_);
        if (IsEmbeddableStart(screenMode_)) {
            screenMode_ = AAFwk::IDLE_SCREEN_MODE;
            CallObjectMethod("onWindowStageDestroy");
        } else {
            napi_value argv[] = {contentSessions_[componentId]->GetNapiValue()};
            CallObjectMethod("onSessionDestroy", argv, ARGC_ONE);
        }
    }
    TAG_LOGI(AAFwkTag::UI_EXT, "Befor window destory, UIExtcomponent id: %{public}" PRId64,
        sessionInfo->uiExtensionComponentId);
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
    if (contentSessions_.find(componentId) != contentSessions_.end()) {
        if (contentSessions_[componentId] != nullptr) {
            jsRuntime_.FreeNativeReference(std::move(contentSessions_[componentId]));
        }
        contentSessions_.erase(componentId);
    }
    if (abilityResultListeners_) {
        abilityResultListeners_->RemoveListener(componentId);
    }
    TAG_LOGD(AAFwkTag::UI_EXT, "end");
}

napi_value JsUIExtension::CallObjectMethod(const char *name, napi_value const *argv, size_t argc, bool withResult)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "%{public}s, begin", name);

    if (!jsObj_) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Not found UIExtension.js");
        return nullptr;
    }

    HandleEscape handleEscape(jsRuntime_);
    napi_env env = jsRuntime_.GetNapiEnv();

    napi_value obj = jsObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "get object failed");
        return nullptr;
    }

    napi_value method = nullptr;
    napi_get_named_property(env, obj, name, &method);
    if (!CheckTypeForNapiValue(env, method, napi_function)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "get '%{public}s' from UIExtension object failed", name);
        return nullptr;
    }
    if (withResult) {
        napi_value result = nullptr;
        napi_call_function(env, obj, method, argc, argv, &result);
        return handleEscape.Escape(result);
    }
    TAG_LOGD(AAFwkTag::UI_EXT, "JsUIExtension CallFunction(%{public}s), success", name);
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    napi_call_function(env, obj, method, argc, argv, nullptr);
    return nullptr;
}

napi_value JsUIExtension::CallOnConnect(const AAFwk::Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    Extension::OnConnect(want);
    TAG_LOGD(AAFwkTag::UI_EXT, "JsUIExtension CallOnConnect begin");
    HandleScope handleScope(jsRuntime_);
    napi_env env = jsRuntime_.GetNapiEnv();
    napi_value napiWant = OHOS::AppExecFwk::WrapWant(env, want);
    napi_value argv[] = {napiWant};
    if (!jsObj_) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Not found UIExtension.js");
        return nullptr;
    }

    napi_value obj = jsObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "get object failed");
        return nullptr;
    }

    napi_value method = nullptr;
    napi_get_named_property(env, obj, "onConnect", &method);
    if (method == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null method");
        return nullptr;
    }
    napi_value remoteNative = nullptr;
    napi_call_function(env, obj, method, ARGC_ONE, argv, &remoteNative);
    if (remoteNative == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null remoteNative");
    }
    TAG_LOGD(AAFwkTag::UI_EXT, "end");
    return remoteNative;
}

napi_value JsUIExtension::CallOnDisconnect(const AAFwk::Want &want, bool withResult)
{
    HandleEscape handleEscape(jsRuntime_);
    napi_env env = jsRuntime_.GetNapiEnv();
    napi_value napiWant = OHOS::AppExecFwk::WrapWant(env, want);
    napi_value argv[] = { napiWant };
    if (!jsObj_) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Not found UIExtension.js");
        return nullptr;
    }

    napi_value obj = jsObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "get object failed");
        return nullptr;
    }

    napi_value method = nullptr;
    napi_get_named_property(env, obj, "onDisconnect", &method);
    if (method == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null method");
        return nullptr;
    }

    if (withResult) {
        napi_value result = nullptr;
        napi_call_function(env, obj, method, ARGC_ONE, argv, &result);
        return handleEscape.Escape(result);
    } else {
        napi_call_function(env, obj, method, ARGC_ONE, argv, nullptr);
        return nullptr;
    }
}

void JsUIExtension::OnAbilityConfigurationUpdated(const AppExecFwk::Configuration& configuration)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    UIExtension::OnAbilityConfigurationUpdated(configuration);
    TAG_LOGD(AAFwkTag::UI_EXT, "OnAbilityConfigurationUpdated called");
    ConfigurationUpdated();
}

void JsUIExtension::Dump(const std::vector<std::string> &params, std::vector<std::string> &info)
{
    Extension::Dump(params, info);
    TAG_LOGD(AAFwkTag::UI_EXT, "called");
    HandleScope handleScope(jsRuntime_);
    napi_env env = jsRuntime_.GetNapiEnv();
    // create js array object of params
    napi_value argv[] = { CreateNativeArray(env, params) };

    if (!jsObj_) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Not found UIExtension.js");
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
            TAG_LOGE(AAFwkTag::UI_EXT, "get object failed");
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
            TAG_LOGE(AAFwkTag::UI_EXT, "Parse dumpInfoStr fail");
            return;
        }
        info.push_back(dumpInfoStr);
    }
    TAG_LOGD(AAFwkTag::UI_EXT, "Dump info size: %{public}zu", info.size());
}

void JsUIExtension::OnAbilityResult(int requestCode, int resultCode, const Want &resultData)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "begin");
    Extension::OnAbilityResult(requestCode, resultCode, resultData);
    auto context = GetContext();
    if (context == nullptr) {
        TAG_LOGW(AAFwkTag::UI_EXT, "null context");
        return;
    }
    context->OnAbilityResult(requestCode, resultCode, resultData);
    if (abilityResultListeners_ == nullptr) {
        TAG_LOGW(AAFwkTag::UI_EXT, "null abilityResultListeners");
        return;
    }
    abilityResultListeners_->OnAbilityResult(requestCode, resultCode, resultData);
    TAG_LOGD(AAFwkTag::UI_EXT, "end");
}

void JsUIExtension::ConfigurationUpdated()
{
    TAG_LOGD(AAFwkTag::UI_EXT, "begin");
    HandleScope handleScope(jsRuntime_);
    napi_env env = jsRuntime_.GetNapiEnv();

    // Notify extension context
    auto context = GetContext();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null context");
        return;
    }
    auto abilityConfig = context->GetAbilityConfiguration();
    auto fullConfig = context->GetConfiguration();
    if (fullConfig == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null Configuration");
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
void JsUIExtension::OnDisplayInfoChange(
    const sptr<IRemoteObject> &token, Rosen::DisplayId displayId, float density, Rosen::DisplayOrientation orientation)
{
    TAG_LOGI(AAFwkTag::UI_EXT, "displayId: %{public}" PRIu64 "", displayId);
    auto context = GetContext();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null context");
        return;
    }

    auto contextConfig = context->GetConfiguration();
    if (contextConfig == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "invalid Configuration");
        return;
    }

    TAG_LOGD(AAFwkTag::UI_EXT, "Config dump: %{public}s", contextConfig->GetName().c_str());
    auto configUtils = std::make_shared<ConfigurationUtils>();
    auto result =
        configUtils->UpdateDisplayConfig(contextConfig, context->GetResourceManager(), displayId, density, orientation);
    TAG_LOGD(AAFwkTag::UI_EXT, "Config dump after update: %{public}s", contextConfig->GetName().c_str());
    if (result) {
        auto jsUiExtension = std::static_pointer_cast<JsUIExtension>(shared_from_this());
        auto task = [jsUiExtension]() {
            if (jsUiExtension) {
                jsUiExtension->ConfigurationUpdated();
            }
        };
        if (handler_ != nullptr) {
            handler_->PostTask(task, "JsUIExtension:OnChange");
        }
    }
}

void JsUIExtension::RegisterDisplayInfoChangedListener()
{
    // register displayid change callback
    auto jsUiExtension = std::static_pointer_cast<JsUIExtension>(shared_from_this());
    jsUiExtensionAbilityDisplayListener_ = sptr<JsUIExtensionAbilityDisplayListener>::MakeSptr(jsUiExtension);
    if (jsUiExtensionAbilityDisplayListener_ == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null jsUiExtensionAbilityDisplayListener");
        return;
    }
    auto context = GetContext();
    if (context == nullptr || context->GetToken() == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null context");
        return;
    }
    TAG_LOGI(AAFwkTag::UI_EXT, "RegisterDisplayInfoChangedListener");
    Rosen::WindowManager::GetInstance().RegisterDisplayInfoChangedListener(
        context->GetToken(), jsUiExtensionAbilityDisplayListener_);
}

void JsUIExtension::UnregisterDisplayInfoChangedListener()
{
    auto context = GetContext();
    if (context == nullptr || context->GetToken() == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null context");
        return;
    }
    Rosen::WindowManager::GetInstance().UnregisterDisplayInfoChangedListener(
        context->GetToken(), jsUiExtensionAbilityDisplayListener_);
}
#endif // SUPPORT_GRAPHICS
}
}
