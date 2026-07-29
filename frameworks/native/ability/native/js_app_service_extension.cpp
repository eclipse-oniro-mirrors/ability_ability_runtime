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

#include "js_app_service_extension.h"

#include "ability_business_error.h"
#include "ability_handler.h"
#include "ability_info.h"
#include "ability_manager_client.h"
#include "configuration_utils.h"
#include "display_util.h"
#include "freeze_util.h"
#include "hitrace_meter.h"
#include "hilog_tag_wrapper.h"
#include "js_extension_common.h"
#include "js_extension_context.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "js_app_service_extension_context.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "napi_common_configuration.h"
#include "napi_common_want.h"
#include "napi_remote_object.h"
#ifdef SUPPORT_GRAPHICS
#include "iservice_registry.h"
#include "system_ability_definition.h"
#endif

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr size_t ARGC_ONE = 1;
constexpr size_t ARGC_TWO = 2;
}

namespace {
sptr<IRemoteObject> GetNativeRemoteObject(napi_env env, napi_value obj)
{
    if (env == nullptr || obj == nullptr) {
        TAG_LOGE(AAFwkTag::APP_SERVICE_EXT, "null obj");
        return nullptr;
    }
    napi_valuetype type;
    napi_typeof(env, obj, &type);
    if (type == napi_undefined || type == napi_null) {
        TAG_LOGE(AAFwkTag::APP_SERVICE_EXT, "obj type invalid");
        return nullptr;
    }
    if (type != napi_object) {
        TAG_LOGE(AAFwkTag::APP_SERVICE_EXT, "obj not object");
        return nullptr;
    }
    return NAPI_ohos_rpc_getNativeRemoteObject(env, obj);
}
}

using namespace OHOS::AppExecFwk;

napi_value AttachAppServiceExtensionContext(napi_env env, void *value, void *)
{
    HandleEscape handleEscape(env);
    if (value == nullptr) {
        TAG_LOGW(AAFwkTag::APP_SERVICE_EXT, "null value");
        return nullptr;
    }
    auto ptr = reinterpret_cast<std::weak_ptr<AppServiceExtensionContext> *>(value)->lock();
    if (ptr == nullptr) {
        TAG_LOGW(AAFwkTag::APP_SERVICE_EXT, "null ptr");
        return nullptr;
    }
    napi_value object = CreateJsAppServiceExtensionContext(env, ptr);
    auto sysModule = JsRuntime::LoadSystemModuleByEngine(env,
        "application.AppServiceExtensionContext", &object, 1);
    if (sysModule == nullptr) {
        TAG_LOGW(AAFwkTag::APP_SERVICE_EXT, "null sysModule");
        return nullptr;
    }
    auto contextObj = sysModule->GetNapiValue();
    napi_coerce_to_native_binding_object(
        env, contextObj, DetachCallbackFunc, AttachAppServiceExtensionContext, value, nullptr);
    auto workContext = new (std::nothrow) std::weak_ptr<AppServiceExtensionContext>(ptr);
    auto res = napi_wrap(env, contextObj, workContext,
        [](napi_env, void *data, void *) {
            TAG_LOGD(AAFwkTag::APP_SERVICE_EXT, "Finalizer for weak_ptr app service extension context is called");
            delete static_cast<std::weak_ptr<AppServiceExtensionContext> *>(data);
        },
        nullptr, nullptr);
    if (res != napi_ok && workContext != nullptr) {
        TAG_LOGE(AAFwkTag::APP_SERVICE_EXT, "napi_wrap failed:%{public}d", res);
        delete workContext;
        return nullptr;
    }
    return handleEscape.Escape(contextObj);
}

JsAppServiceExtension* JsAppServiceExtension::Create(const std::unique_ptr<Runtime>& runtime)
{
    return new JsAppServiceExtension(static_cast<JsRuntime&>(*runtime));
}

JsAppServiceExtension::JsAppServiceExtension(JsRuntime& jsRuntime) : jsRuntime_(jsRuntime) {}
JsAppServiceExtension::~JsAppServiceExtension()
{
    TAG_LOGD(AAFwkTag::APP_SERVICE_EXT, "called");
    auto context = GetContext();
    if (context) {
        context->Unbind();
    }

    jsRuntime_.FreeNativeReference(std::move(jsObj_));
    jsRuntime_.FreeNativeReference(std::move(shellContextRef_));
}

void JsAppServiceExtension::Init(const std::shared_ptr<AbilityLocalRecord> &record,
    const std::shared_ptr<OHOSApplication> &application, std::shared_ptr<AbilityHandler> &handler,
    const sptr<IRemoteObject> &token)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    AppServiceExtension::Init(record, application, handler, token);
    std::string srcPath = "";
    GetSrcPath(srcPath);
    if (srcPath.empty()) {
        TAG_LOGE(AAFwkTag::APP_SERVICE_EXT, "get srcPath failed");
        return;
    }

    std::string moduleName(Extension::abilityInfo_->moduleName);
    moduleName.append("::").append(abilityInfo_->name);
    TAG_LOGD(AAFwkTag::APP_SERVICE_EXT, "called, moduleName:%{public}s,srcPath:%{public}s",
        moduleName.c_str(), srcPath.c_str());
    HandleScope handleScope(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();

    jsObj_ = jsRuntime_.LoadModule(
        moduleName, srcPath, abilityInfo_->hapPath, abilityInfo_->compileMode == CompileMode::ES_MODULE,
        false, abilityInfo_->srcEntrance);
    if (jsObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::APP_SERVICE_EXT, "null jsObj_");
        return;
    }

    TAG_LOGD(AAFwkTag::APP_SERVICE_EXT, "ConvertNativeValueTo");
    napi_value obj = jsObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        TAG_LOGE(AAFwkTag::APP_SERVICE_EXT, "get JsAppServiceExtension obj failed");
        return;
    }

    BindContext(env, obj);

    SetExtensionCommon(JsExtensionCommon::Create(jsRuntime_, static_cast<NativeReference&>(*jsObj_), shellContextRef_));

    auto context = GetContext();
    auto appContext = Context::GetApplicationContext();
    if (context != nullptr && appContext != nullptr) {
        auto appConfig = appContext->GetConfiguration();
        if (appConfig != nullptr) {
            TAG_LOGD(AAFwkTag::APP_SERVICE_EXT, "Original config dump: %{public}s", appConfig->GetName().c_str());
            context->SetConfiguration(std::make_shared<Configuration>(*appConfig));
        }
    }
    ListenWMS();
}

void JsAppServiceExtension::ListenWMS()
{
#ifdef SUPPORT_GRAPHICS
    TAG_LOGD(AAFwkTag::APP_SERVICE_EXT, "RegisterDisplayListener");
    auto abilityManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (abilityManager == nullptr) {
        TAG_LOGE(AAFwkTag::APP_SERVICE_EXT, "null SaMgr");
        return;
    }

    auto jsAppServiceExtension = std::static_pointer_cast<JsAppServiceExtension>(shared_from_this());
    displayListener_ = sptr<JsAppServiceExtensionDisplayListener>::MakeSptr(jsAppServiceExtension);
    if (displayListener_ == nullptr) {
        TAG_LOGE(AAFwkTag::APP_SERVICE_EXT, "null displayListener");
        return;
    }

    auto context = GetContext();
    if (context == nullptr || context->GetToken() == nullptr) {
        TAG_LOGE(AAFwkTag::APP_SERVICE_EXT, "null context");
        return;
    }

    saStatusChangeListener_ =
        sptr<SystemAbilityStatusChangeListener>::MakeSptr(displayListener_, context->GetToken());
    if (saStatusChangeListener_ == nullptr) {
        TAG_LOGE(AAFwkTag::APP_SERVICE_EXT, "null saStatusChangeListener");
        return;
    }

    auto ret = abilityManager->SubscribeSystemAbility(WINDOW_MANAGER_SERVICE_ID, saStatusChangeListener_);
    if (ret != 0) {
        TAG_LOGE(AAFwkTag::APP_SERVICE_EXT, "subscribe system ability error:%{public}d.", ret);
    }
#endif
}

void JsAppServiceExtension::BindContext(napi_env env, napi_value obj)
{
    HandleScope handleScope(env);
    auto context = GetContext();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::APP_SERVICE_EXT, "null context");
        return;
    }
    TAG_LOGD(AAFwkTag::APP_SERVICE_EXT, "call");
    napi_value contextObj = CreateJsAppServiceExtensionContext(env, context);
    shellContextRef_ = JsRuntime::LoadSystemModuleByEngine(env, "application.AppServiceExtensionContext",
        &contextObj, ARGC_ONE);
    if (shellContextRef_ == nullptr) {
        TAG_LOGE(AAFwkTag::APP_SERVICE_EXT, "null shellContextRef");
        return;
    }
    contextObj = shellContextRef_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, contextObj, napi_object)) {
        TAG_LOGE(AAFwkTag::APP_SERVICE_EXT, "get context native obj failed");
        return;
    }
    auto workContext = new (std::nothrow) std::weak_ptr<AppServiceExtensionContext>(context);
    napi_coerce_to_native_binding_object(
        env, contextObj, DetachCallbackFunc, AttachAppServiceExtensionContext, workContext, nullptr);
    TAG_LOGD(AAFwkTag::APP_SERVICE_EXT, "Bind");
    context->Bind(jsRuntime_, shellContextRef_.get());
    napi_set_named_property(env, obj, "context", contextObj);

    auto res = napi_wrap(env, contextObj, workContext,
        [](napi_env, void* data, void*) {
            delete static_cast<std::weak_ptr<AppServiceExtensionContext>*>(data);
        },
        nullptr, nullptr);
    if (res != napi_ok && workContext != nullptr) {
        TAG_LOGE(AAFwkTag::APP_SERVICE_EXT, "napi_wrap failed:%{public}d", res);
        delete workContext;
        return;
    }
    TAG_LOGD(AAFwkTag::APP_SERVICE_EXT, "end");
}

void JsAppServiceExtension::OnStart(const AAFwk::Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    Extension::OnStart(want);
    TAG_LOGI(AAFwkTag::APP_SERVICE_EXT, "call");

    auto context = GetContext();
    if (context != nullptr) {
#ifdef SUPPORT_GRAPHICS
        int32_t displayId = AAFwk::DisplayUtil::GetDefaultDisplayId();
        TAG_LOGD(AAFwkTag::APP_SERVICE_EXT, "displayId %{public}d", displayId);
        auto configUtils = std::make_shared<ConfigurationUtils>();
        configUtils->InitDisplayConfig(displayId, context->GetConfiguration(), context->GetResourceManager());
#endif //SUPPORT_GRAPHICS
    }

    HandleScope handleScope(jsRuntime_);
    napi_env env = jsRuntime_.GetNapiEnv();

    // display config has changed, need update context.config
    if (context != nullptr) {
        JsExtensionContext::ConfigurationUpdated(env, shellContextRef_, context->GetConfiguration());
    }

    napi_value napiWant = OHOS::AppExecFwk::WrapWant(env, want);
    napi_value argv[] = {napiWant};
    CallObjectMethod("onCreate", argv, ARGC_ONE);
    TAG_LOGD(AAFwkTag::APP_SERVICE_EXT, "ok");
}

void JsAppServiceExtension::OnStop()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    AppServiceExtension::OnStop();
    TAG_LOGD(AAFwkTag::APP_SERVICE_EXT, "call");
    CallObjectMethod("onDestroy");
    bool ret = ConnectionManager::GetInstance().DisconnectCaller(GetContext()->GetToken());
    if (ret) {
        ConnectionManager::GetInstance().ReportConnectionLeakEvent(getpid(), gettid());
        TAG_LOGD(AAFwkTag::APP_SERVICE_EXT, "app service extension connection not disconnected");
    }
#ifdef SUPPORT_GRAPHICS
    TAG_LOGI(AAFwkTag::APP_SERVICE_EXT, "UnregisterDisplayInfoChangedListener");
    auto context = GetContext();
    if (context == nullptr || context->GetToken() == nullptr) {
        TAG_LOGE(AAFwkTag::APP_SERVICE_EXT, "null context");
        return;
    }
    Rosen::WindowManager::GetInstance()
        .UnregisterDisplayInfoChangedListener(context->GetToken(), displayListener_);
    if (saStatusChangeListener_) {
        auto saMgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        if (saMgr) {
            saMgr->UnSubscribeSystemAbility(WINDOW_MANAGER_SERVICE_ID, saStatusChangeListener_);
        } else {
            TAG_LOGW(AAFwkTag::APP_SERVICE_EXT, "OnStop SaMgr null");
        }
    }
#endif //SUPPORT_GRAPHICS
    TAG_LOGD(AAFwkTag::APP_SERVICE_EXT, "ok");
}

void JsAppServiceExtension::AddLifecycleEventForJSCall(const std::string &eventStr)
{
    auto entry = std::string("JsAppServiceExtension:") + eventStr;
    auto context = GetContext();
    if (context) {
        FreezeUtil::GetInstance().AddLifecycleEvent(context->GetToken(), entry);
    }
}

sptr<IRemoteObject> JsAppServiceExtension::OnConnect(const AAFwk::Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HandleScope handleScope(jsRuntime_);
    AddLifecycleEventForJSCall("OnConnect begin");
    napi_value result = CallOnConnect(want);
    AddLifecycleEventForJSCall("OnConnect end");
    napi_env env = jsRuntime_.GetNapiEnv();
    auto remoteObj = GetNativeRemoteObject(env, result);
    if (remoteObj == nullptr) {
        TAG_LOGE(AAFwkTag::APP_SERVICE_EXT, "null remoteObj");
    }
    return remoteObj;
}

void JsAppServiceExtension::OnDisconnect(const AAFwk::Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HandleScope handleScope(jsRuntime_);
    Extension::OnDisconnect(want);
    TAG_LOGD(AAFwkTag::APP_SERVICE_EXT, "called");
    CallOnDisconnect(want, false);
    TAG_LOGD(AAFwkTag::APP_SERVICE_EXT, "end");
}

void JsAppServiceExtension::OnCommand(const AAFwk::Want &want, bool restart, int startId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    Extension::OnCommand(want, restart, startId);
    TAG_LOGD(AAFwkTag::APP_SERVICE_EXT, "restart=%{public}s,startId=%{public}d",
        restart ? "true" : "false",
        startId);
    // wrap want
    HandleScope handleScope(jsRuntime_);
    napi_env env = jsRuntime_.GetNapiEnv();
    napi_value napiWant = OHOS::AppExecFwk::WrapWant(env, want);
    // wrap startId
    napi_value napiStartId = nullptr;
    napi_create_int32(env, startId, &napiStartId);
    napi_value argv[] = {napiWant, napiStartId};
    CallObjectMethod("onRequest", argv, ARGC_TWO);
    TAG_LOGD(AAFwkTag::APP_SERVICE_EXT, "ok");
}

napi_value JsAppServiceExtension::CallObjectMethod(const char* name, napi_value const* argv, size_t argc)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, std::string("CallObjectMethod:") + name);
    TAG_LOGD(AAFwkTag::APP_SERVICE_EXT, "name:%{public}s", name);

    if (!jsObj_) {
        TAG_LOGW(AAFwkTag::APP_SERVICE_EXT, "Not found AppServiceExtension.js");
        return nullptr;
    }

    HandleScope handleScope(jsRuntime_);
    napi_env env = jsRuntime_.GetNapiEnv();

    napi_value obj = jsObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        TAG_LOGE(AAFwkTag::APP_SERVICE_EXT, "get AppServiceExtension obj failed");
        return nullptr;
    }

    napi_value method = nullptr;
    napi_get_named_property(env, obj, name, &method);
    if (!CheckTypeForNapiValue(env, method, napi_function)) {
        TAG_LOGE(AAFwkTag::APP_SERVICE_EXT, "get '%{public}s' from AppServiceExtension obj failed", name);
        return nullptr;
    }
    TAG_LOGI(AAFwkTag::APP_SERVICE_EXT, "CallFunction(%{public}s) ok", name);
    napi_value result = nullptr;
    napi_status status = napi_call_function(env, obj, method, argc, argv, &result);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::APP_SERVICE_EXT, "call js func failed: %{public}d", status);
    }
    return result;
}

void JsAppServiceExtension::GetSrcPath(std::string &srcPath)
{
    TAG_LOGD(AAFwkTag::APP_SERVICE_EXT, "called");
    if (!Extension::abilityInfo_->srcEntrance.empty()) {
        srcPath.append(Extension::abilityInfo_->moduleName + "/");
        srcPath.append(Extension::abilityInfo_->srcEntrance);
        RemoveFileExtension(srcPath);
        srcPath.append(".abc");
    }
}

napi_value JsAppServiceExtension::CallOnConnect(const AAFwk::Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HandleEscape handleEscape(jsRuntime_);
    Extension::OnConnect(want);
    TAG_LOGD(AAFwkTag::APP_SERVICE_EXT, "call");
    napi_env env = jsRuntime_.GetNapiEnv();
    napi_value napiWant = OHOS::AppExecFwk::WrapWant(env, want);
    napi_value argv[] = {napiWant};
    if (!jsObj_) {
        TAG_LOGW(AAFwkTag::APP_SERVICE_EXT, "Not found AppServiceExtension.js");
        return nullptr;
    }

    napi_value obj = jsObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        TAG_LOGE(AAFwkTag::APP_SERVICE_EXT, "get AppServiceExtension obj failed");
        return nullptr;
    }

    napi_value method = nullptr;
    napi_get_named_property(env, obj, "onConnect", &method);
    if (method == nullptr) {
        TAG_LOGE(AAFwkTag::APP_SERVICE_EXT, "null method");
        return nullptr;
    }
    napi_value remoteNative = nullptr;
    TAG_LOGI(AAFwkTag::APP_SERVICE_EXT, "Call onConnect");
    napi_status status = napi_call_function(env, obj, method, ARGC_ONE, argv, &remoteNative);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::APP_SERVICE_EXT, "call js func failed %{public}d", status);
    }
    if (remoteNative == nullptr) {
        TAG_LOGE(AAFwkTag::APP_SERVICE_EXT, "null remoteNative");
    }
    TAG_LOGD(AAFwkTag::APP_SERVICE_EXT, "ok");
    return handleEscape.Escape(remoteNative);
}

napi_value JsAppServiceExtension::CallOnDisconnect(const AAFwk::Want &want, bool withResult)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HandleEscape handleEscape(jsRuntime_);
    napi_env env = jsRuntime_.GetNapiEnv();
    napi_value napiWant = OHOS::AppExecFwk::WrapWant(env, want);
    napi_value argv[] = { napiWant };
    if (!jsObj_) {
        TAG_LOGW(AAFwkTag::APP_SERVICE_EXT, "Not found AppServiceExtension.js");
        return nullptr;
    }

    napi_value obj = jsObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        TAG_LOGE(AAFwkTag::APP_SERVICE_EXT, "get AppServiceExtension obj failed");
        return nullptr;
    }

    napi_value method = nullptr;
    napi_get_named_property(env, obj, "onDisconnect", &method);
    if (method == nullptr) {
        TAG_LOGE(AAFwkTag::APP_SERVICE_EXT, "null method");
        return nullptr;
    }
    TAG_LOGI(AAFwkTag::APP_SERVICE_EXT, "Call onDisconnect");
    if (withResult) {
        napi_value result = nullptr;
        napi_status status = napi_call_function(env, obj, method, ARGC_ONE, argv, &result);
        if (status != napi_ok) {
            TAG_LOGE(AAFwkTag::APP_SERVICE_EXT, "call js func failed %{public}d", status);
        }
        return handleEscape.Escape(result);
    } else {
        napi_status status = napi_call_function(env, obj, method, ARGC_ONE, argv, nullptr);
        if (status != napi_ok) {
            TAG_LOGE(AAFwkTag::APP_SERVICE_EXT, "call js func failed %{public}d", status);
        }
        return nullptr;
    }
}

void JsAppServiceExtension::OnConfigurationUpdated(const AppExecFwk::Configuration& configuration)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    AppServiceExtension::OnConfigurationUpdated(configuration);
    TAG_LOGD(AAFwkTag::APP_SERVICE_EXT, "call");
    auto context = GetContext();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::APP_SERVICE_EXT, "null context");
        return;
    }

    auto contextConfig = context->GetConfiguration();
    if (contextConfig != nullptr) {
        TAG_LOGD(AAFwkTag::APP_SERVICE_EXT, "Config dump: %{public}s", contextConfig->GetName().c_str());
        std::vector<std::string> changeKeyV;
        contextConfig->CompareDifferent(changeKeyV, configuration);
        if (!changeKeyV.empty()) {
            contextConfig->Merge(changeKeyV, configuration);
        }
        TAG_LOGD(AAFwkTag::APP_SERVICE_EXT, "Config dump after merge: %{public}s", contextConfig->GetName().c_str());
    }
    ConfigurationUpdated();
}

void JsAppServiceExtension::ConfigurationUpdated()
{
    TAG_LOGD(AAFwkTag::APP_SERVICE_EXT, "called");
    HandleScope handleScope(jsRuntime_);
    napi_env env = jsRuntime_.GetNapiEnv();

    // Notify extension context
    auto fullConfig = GetContext()->GetConfiguration();
    if (!fullConfig) {
        TAG_LOGE(AAFwkTag::APP_SERVICE_EXT, "null configuration");
        return;
    }

    napi_value napiConfiguration = OHOS::AppExecFwk::WrapConfiguration(env, *fullConfig);
    CallObjectMethod("onConfigurationUpdate", &napiConfiguration, ARGC_ONE);
    JsExtensionContext::ConfigurationUpdated(env, shellContextRef_, fullConfig);
}
#ifdef SUPPORT_GRAPHICS
void JsAppServiceExtension::SystemAbilityStatusChangeListener::OnAddSystemAbility(int32_t systemAbilityId,
    const std::string& deviceId)
{
    TAG_LOGD(AAFwkTag::APP_SERVICE_EXT, "systemAbilityId: %{public}d add", systemAbilityId);
    if (systemAbilityId == WINDOW_MANAGER_SERVICE_ID) {
        TAG_LOGI(AAFwkTag::APP_SERVICE_EXT, "RegisterDisplayInfoChangedListener");
        Rosen::WindowManager::GetInstance().RegisterDisplayInfoChangedListener(token_, tmpDisplayListener_);
    }
}

void JsAppServiceExtension::OnCreate(Rosen::DisplayId displayId)
{
    TAG_LOGD(AAFwkTag::APP_SERVICE_EXT, "OnCreate");
}

void JsAppServiceExtension::OnDestroy(Rosen::DisplayId displayId)
{
    TAG_LOGD(AAFwkTag::APP_SERVICE_EXT, "OnDestroy");
}

void JsAppServiceExtension::OnDisplayInfoChange(const sptr<IRemoteObject>& token, Rosen::DisplayId displayId,
    float density, Rosen::DisplayOrientation orientation)
{
    TAG_LOGI(AAFwkTag::APP_SERVICE_EXT, "displayId: %{public}" PRIu64, displayId);
    auto context = GetContext();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::APP_SERVICE_EXT, "null context");
        return;
    }

    auto contextConfig = context->GetConfiguration();
    if (contextConfig == nullptr) {
        TAG_LOGE(AAFwkTag::APP_SERVICE_EXT, "null contextConfig");
        return;
    }

    TAG_LOGD(AAFwkTag::APP_SERVICE_EXT, "Config dump: %{public}s", contextConfig->GetName().c_str());
    bool configChanged = false;
    auto configUtils = std::make_shared<ConfigurationUtils>();
    configUtils->UpdateDisplayConfig(displayId, contextConfig, context->GetResourceManager(), configChanged);
    TAG_LOGD(AAFwkTag::APP_SERVICE_EXT, "Config dump after update: %{public}s", contextConfig->GetName().c_str());

    if (configChanged) {
        auto jsAppServiceExtension = std::static_pointer_cast<JsAppServiceExtension>(shared_from_this());
        auto task = [jsAppServiceExtension]() {
            if (jsAppServiceExtension) {
                jsAppServiceExtension->ConfigurationUpdated();
            }
        };
        if (handler_ != nullptr) {
            handler_->PostTask(task, "JsAppServiceExtension:OnChange");
        }
    }

    TAG_LOGD(AAFwkTag::APP_SERVICE_EXT, "finished");
}

void JsAppServiceExtension::OnChange(Rosen::DisplayId displayId)
{
    TAG_LOGD(AAFwkTag::APP_SERVICE_EXT, "displayId: %{public}" PRIu64"", displayId);
    auto context = GetContext();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::APP_SERVICE_EXT, "null context");
        return;
    }

    auto contextConfig = context->GetConfiguration();
    if (contextConfig == nullptr) {
        TAG_LOGE(AAFwkTag::APP_SERVICE_EXT, "null contextConfig");
        return;
    }

    TAG_LOGD(AAFwkTag::APP_SERVICE_EXT, "Config dump: %{public}s", contextConfig->GetName().c_str());
    bool configChanged = false;
    auto configUtils = std::make_shared<ConfigurationUtils>();
    configUtils->UpdateDisplayConfig(displayId, contextConfig, context->GetResourceManager(), configChanged);
    TAG_LOGD(AAFwkTag::APP_SERVICE_EXT, "Config dump after update: %{public}s", contextConfig->GetName().c_str());

    if (configChanged) {
        auto jsAppServiceExtension = std::static_pointer_cast<JsAppServiceExtension>(shared_from_this());
        auto task = [jsAppServiceExtension]() {
            if (jsAppServiceExtension) {
                jsAppServiceExtension->ConfigurationUpdated();
            }
        };
        if (handler_ != nullptr) {
            handler_->PostTask(task, "JsAppServiceExtension:OnChange");
        }
    }

    TAG_LOGD(AAFwkTag::APP_SERVICE_EXT, "finished");
}
#endif
} // AbilityRuntime
} // OHOS
