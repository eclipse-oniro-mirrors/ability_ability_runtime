/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "js_agent_extension.h"

#include "ability_business_error.h"
#include "agent_extension.h"
#include "agent_extension_connection_constants.h"
#include "agent_extension_context.h"
#include "agent_manager_client.h"
#include "configuration_utils.h"
#include "connection_manager.h"
#include "hilog_tag_wrapper.h"
#include "js_agent_connector_proxy.h"
#include "js_agent_extension_context.h"
#include "js_agent_extension_stub_impl.h"
#include "js_extension_common.h"
#include "js_extension_context.h"
#include "js_runtime_utils.h"
#include "napi_common_want.h"
#include "runtime.h"

#ifdef WINDOWS_PLATFORM
#define JS_EXPORT __declspec(dllexport)
#else
#define JS_EXPORT __attribute__((visibility("default")))
#endif

namespace OHOS {
namespace AgentRuntime {
namespace {
constexpr size_t ARGC_ONE = 1;
constexpr size_t ARGC_TWO = 2;
}
using namespace OHOS::AbilityRuntime;
using namespace OHOS::AppExecFwk;

napi_value AttachAgentExtensionContext(napi_env env, void *value, void *)
{
    if (value == nullptr) {
        TAG_LOGW(AAFwkTag::SER_ROUTER, "null value");
        return nullptr;
    }
    auto context = reinterpret_cast<std::weak_ptr<Context> *>(value)->lock();
    if (context == nullptr) {
        TAG_LOGW(AAFwkTag::SER_ROUTER, "null ptr");
        return nullptr;
    }
    auto ptr = Context::ConvertTo<AgentExtensionContext>(context);
    if (ptr == nullptr) {
        TAG_LOGW(AAFwkTag::SER_ROUTER, "context is not AgentExtensionContext");
        return nullptr;
    }
    napi_value object = CreateJsAgentExtensionContext(env, ptr);
    auto sysModule = JsRuntime::LoadSystemModuleByEngine(env,
        "application.AgentExtensionContext", &object, 1);
    if (sysModule == nullptr) {
        TAG_LOGW(AAFwkTag::SER_ROUTER, "null sysModule");
        return nullptr;
    }
    auto contextObj = sysModule->GetNapiValue();
    napi_coerce_to_native_binding_object(
        env, contextObj, DetachCallbackFunc, AttachAgentExtensionContext, value, nullptr);
    SetJsAgentExtensionContext(env, contextObj, ptr);
    auto workContext = new (std::nothrow) std::weak_ptr<Context>(context);
    auto res = napi_wrap(env, contextObj, workContext,
        [](napi_env, void *data, void *) {
            TAG_LOGD(AAFwkTag::SER_ROUTER, "Finalizer for weak_ptr app service extension context is called");
            delete static_cast<std::weak_ptr<Context> *>(data);
        },
        nullptr, nullptr);
    if (res != napi_ok && workContext != nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "napi_wrap failed:%{public}d", res);
        delete workContext;
        return nullptr;
    }
    return contextObj;
}

JsAgentExtension::JsAgentExtension(AbilityRuntime::JsRuntime& jsRuntime) : jsRuntime_(jsRuntime) {}

JsAgentExtension::~JsAgentExtension()
{
    auto context = GetContext();
    if (context) {
        context->Unbind();
    }

    jsRuntime_.FreeNativeReference(std::move(jsObj_));
    jsRuntime_.FreeNativeReference(std::move(shellContextRef_));

    for (auto& item : hostProxyMap_) {
        auto &jsProxyObject = item.second;
        if (jsProxyObject != nullptr) {
            jsRuntime_.FreeNativeReference(std::move(jsProxyObject));
        }
    }
}

void JsAgentExtension::Init(const std::shared_ptr<AbilityLocalRecord> &record,
    const std::shared_ptr<OHOSApplication> &application, std::shared_ptr<AbilityHandler> &handler,
    const sptr<IRemoteObject> &token)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "Call");
    AgentExtension::Init(record, application, handler, token);
    std::string srcPath = "";
    GetSrcPath(srcPath);
    if (srcPath.empty()) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "get srcPath failed");
        return;
    }

    std::string moduleName(Extension::abilityInfo_->moduleName);
    moduleName.append("::").append(abilityInfo_->name);
    TAG_LOGD(AAFwkTag::SER_ROUTER, "called, moduleName:%{public}s,srcPath:%{public}s",
        moduleName.c_str(), srcPath.c_str());
    HandleScope handleScope(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();

    jsObj_ = jsRuntime_.LoadModule(
        moduleName, srcPath, abilityInfo_->hapPath, abilityInfo_->compileMode == CompileMode::ES_MODULE,
        false, abilityInfo_->srcEntrance);
    if (jsObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null jsObj_");
        return;
    }

    TAG_LOGD(AAFwkTag::SER_ROUTER, "ConvertNativeValueTo");
    napi_value obj = jsObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "get JsAgentExtension obj failed");
        return;
    }
    std::shared_ptr<AAFwk::Want> want = nullptr;
    if (record != nullptr) {
        want = record->GetWant();
    }
    BindContext(env, obj, want);

    SetExtensionCommon(JsExtensionCommon::Create(jsRuntime_, static_cast<NativeReference&>(*jsObj_), shellContextRef_));

    auto context = GetContext();
    auto appContext = Context::GetApplicationContext();
    if (context != nullptr && appContext != nullptr) {
        auto appConfig = appContext->GetConfiguration();
        if (appConfig != nullptr) {
            TAG_LOGD(AAFwkTag::SER_ROUTER, "Original config dump: %{public}s", appConfig->GetName().c_str());
            context->SetConfiguration(std::make_shared<Configuration>(*appConfig));
        }
    }
}

void JsAgentExtension::OnStart(const AAFwk::Want &want)
{
    Extension::OnStart(want);
    TAG_LOGI(AAFwkTag::SER_ROUTER, "call");
    auto context = GetContext();
    HandleScope handleScope(jsRuntime_);
    napi_env env = jsRuntime_.GetNapiEnv();
    // display config has changed, need update context.config
    if (context != nullptr) {
        JsExtensionContext::ConfigurationUpdated(env, shellContextRef_, context->GetConfiguration());
    }
    napi_value napiWant = OHOS::AppExecFwk::WrapWant(env, want);
    napi_value argv[] = {napiWant};
    CallObjectMethod("onCreate", argv, ARGC_ONE);
    TAG_LOGD(AAFwkTag::SER_ROUTER, "ok");
}

void JsAgentExtension::OnStop()
{
    Extension::OnStop();
    TAG_LOGD(AAFwkTag::SER_ROUTER, "call");
    CallObjectMethod("onDestroy");
    auto context = GetContext();
    if (context != nullptr) {
        bool ret = ConnectionManager::GetInstance().DisconnectCaller(GetContext()->GetToken());
        if (ret) {
            ConnectionManager::GetInstance().ReportConnectionLeakEvent(getpid(), gettid());
            TAG_LOGD(AAFwkTag::SER_ROUTER, "The agent extension connection is not disconnected.");
        }
    }
    TAG_LOGD(AAFwkTag::SER_ROUTER, "ok");
}

sptr<IRemoteObject> JsAgentExtension::OnConnect(const AAFwk::Want &want,
    AppExecFwk::AbilityTransactionCallbackInfo<sptr<IRemoteObject>> *callbackInfo, bool &isAsyncCallback)
{
    TAG_LOGI(AAFwkTag::SER_ROUTER, "call");
    HandleScope handleScope(jsRuntime_);
    Extension::OnConnect(want);
    napi_env env = jsRuntime_.GetNapiEnv();
    sptr<IRemoteObject> hostProxy = GetHostProxyFromWant(want);
    if (hostProxy == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null hostProxy");
        return nullptr;
    }
    napi_value napiWant = WrapWant(env, want);
    if (napiWant == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null napiWant");
        return nullptr;
    }
    if (extensionStub_ == nullptr) {
        std::weak_ptr<JsAgentExtension> weakThis = std::static_pointer_cast<JsAgentExtension>(shared_from_this());
        extensionStub_ = sptr<JsAgentExtensionStubImpl>::MakeSptr(weakThis);
    }
    sptr<IRemoteObject> stubObject = nullptr;
    if (extensionStub_ != nullptr) {
        stubObject = extensionStub_->AsObject();
    }
    if (hostProxyMap_.find(hostProxy) != hostProxyMap_.end()) {
        TAG_LOGI(AAFwkTag::SER_ROUTER, "hostProxy exist");
        return stubObject;
    }
    napi_ref hostProxyNref = JsAgentConnectorProxy::CreateJsAgentConnectorProxy(env, hostProxy);
    if (hostProxyNref == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null hostProxyNref");
        return nullptr;
    }
    napi_value jsHostProxy = reinterpret_cast<NativeReference*>(hostProxyNref)->GetNapiValue();
    napi_value argv[] = {napiWant, jsHostProxy};
    CallObjectMethod("onConnect", argv, ARGC_TWO);
    hostProxyMap_[hostProxy] = std::unique_ptr<NativeReference>(reinterpret_cast<NativeReference*>(hostProxyNref));
    return stubObject;
}

void JsAgentExtension::OnDisconnect(const AAFwk::Want &want,
    AppExecFwk::AbilityTransactionCallbackInfo<> *callbackInfo, bool &isAsyncCallback)
{
    TAG_LOGI(AAFwkTag::SER_ROUTER, "call");
    HandleScope handleScope(jsRuntime_);
    Extension::OnDisconnect(want);
    napi_env env = jsRuntime_.GetNapiEnv();
    sptr<IRemoteObject> hostProxy = GetHostProxyFromWant(want);
    if (hostProxy == nullptr) {
        TAG_LOGW(AAFwkTag::SER_ROUTER, "null hostProxy");
        return;
    }
    napi_value napiWant = WrapWant(env, want);
    if (napiWant == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null napiWant");
        return;
    }
    napi_value jsHostProxy = nullptr;
    auto iter = hostProxyMap_.find(hostProxy);
    if (iter != hostProxyMap_.end()) {
        auto &hostProxyNref = iter->second;
        if (hostProxyNref != nullptr) {
            jsHostProxy = hostProxyNref->GetNapiValue();
        }
    }
    if (jsHostProxy == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null jsHostProxy");
        return;
    }
    napi_value argv[] = { napiWant, jsHostProxy };
    CallObjectMethod("onDisconnect", argv, ARGC_TWO);
    hostProxyMap_.erase(iter);
}

int32_t JsAgentExtension::OnSendData(const sptr<IRemoteObject> &hostProxy, const std::string &data)
{
    napi_env env = jsRuntime_.GetNapiEnv();
    std::unique_ptr<NapiAsyncTask::CompleteCallback> complete = std::make_unique<NapiAsyncTask::CompleteCallback>
        ([weak = weak_from_this(), hostProxy, dataParam = data](napi_env env, NapiAsyncTask &task, int32_t status) {
            auto extensionSptr = weak.lock();
            if (!extensionSptr) {
                TAG_LOGE(AAFwkTag::SER_ROUTER, "null extensionSptr");
                return;
            }
            auto sptrThis = std::static_pointer_cast<JsAgentExtension>(extensionSptr);
            if (!sptrThis) {
                TAG_LOGE(AAFwkTag::SER_ROUTER, "null sptrThis");
                return;
            }
            sptrThis->HandleSendData(hostProxy, dataParam);
        });

    napi_ref callback = nullptr;
    std::unique_ptr<NapiAsyncTask::ExecuteCallback> execute = nullptr;
    NapiAsyncTask::Schedule("JsAgentExtension::SendData",
        env, std::make_unique<NapiAsyncTask>(callback, std::move(execute), std::move(complete)));
    return static_cast<int32_t>(AbilityErrorCode::ERROR_OK);
}

int32_t JsAgentExtension::OnAuthorize(const sptr<IRemoteObject> &hostProxy, const std::string &data)
{
    napi_env env = jsRuntime_.GetNapiEnv();
    std::unique_ptr<NapiAsyncTask::CompleteCallback> complete = std::make_unique<NapiAsyncTask::CompleteCallback>
        ([weak = weak_from_this(), hostProxy, dataParam = data](napi_env env, NapiAsyncTask &task, int32_t status) {
            auto extensionSptr = weak.lock();
            if (!extensionSptr) {
                TAG_LOGE(AAFwkTag::SER_ROUTER, "null extensionSptr");
                return;
            }
            auto sptrThis = std::static_pointer_cast<JsAgentExtension>(extensionSptr);
            if (!sptrThis) {
                TAG_LOGE(AAFwkTag::SER_ROUTER, "null sptrThis");
                return;
            }
            sptrThis->HandleAuthorize(hostProxy, dataParam);
        });

    napi_ref callback = nullptr;
    std::unique_ptr<NapiAsyncTask::ExecuteCallback> execute = nullptr;
    NapiAsyncTask::Schedule("JsAgentExtension::Authorize",
        env, std::make_unique<NapiAsyncTask>(callback, std::move(execute), std::move(complete)));
    return static_cast<int32_t>(AbilityErrorCode::ERROR_OK);
}

int32_t JsAgentExtension::OnAgentInvoked(const std::string &agentId)
{
    napi_env env = jsRuntime_.GetNapiEnv();
    std::unique_ptr<NapiAsyncTask::CompleteCallback> complete = std::make_unique<NapiAsyncTask::CompleteCallback>
        ([weak = weak_from_this(), agentIdParam = agentId](napi_env env, NapiAsyncTask &task, int32_t status) {
            auto extensionSptr = weak.lock();
            if (!extensionSptr) {
                TAG_LOGE(AAFwkTag::SER_ROUTER, "null extensionSptr");
                return;
            }
            auto sptrThis = std::static_pointer_cast<JsAgentExtension>(extensionSptr);
            if (!sptrThis) {
                TAG_LOGE(AAFwkTag::SER_ROUTER, "null sptrThis");
                return;
            }
            sptrThis->HandleAgentInvoked(agentIdParam);
        });

    napi_ref callback = nullptr;
    std::unique_ptr<NapiAsyncTask::ExecuteCallback> execute = nullptr;
    NapiAsyncTask::Schedule("JsAgentExtension::AgentInvoked",
        env, std::make_unique<NapiAsyncTask>(callback, std::move(execute), std::move(complete)));
    return static_cast<int32_t>(AbilityErrorCode::ERROR_OK);
}

void JsAgentExtension::HandleSendData(sptr<IRemoteObject> hostProxy, const std::string &data)
{
    if (hostProxy == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null hostProxy");
        return;
    }
    napi_value jsHostProxy = nullptr;
    auto iter = hostProxyMap_.find(hostProxy);
    if (iter != hostProxyMap_.end()) {
        auto &hostProxyNref = iter->second;
        if (hostProxyNref != nullptr) {
            jsHostProxy = hostProxyNref->GetNapiValue();
        }
    }
    if (jsHostProxy == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null jsHostProxy");
        return;
    }

    napi_env env = jsRuntime_.GetNapiEnv();
    napi_value argv[] = {jsHostProxy, AbilityRuntime::CreateJsValue(env, data)};
    CallObjectMethod("onData", argv, ARGC_TWO);
}

void JsAgentExtension::HandleAuthorize(sptr<IRemoteObject> hostProxy, const std::string &data)
{
    if (hostProxy == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null hostProxy");
        return;
    }
    napi_value jsHostProxy = nullptr;
    auto iter = hostProxyMap_.find(hostProxy);
    if (iter != hostProxyMap_.end()) {
        auto &hostProxyNref = iter->second;
        if (hostProxyNref != nullptr) {
            jsHostProxy = hostProxyNref->GetNapiValue();
        }
    }
    if (jsHostProxy == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null jsHostProxy");
        return;
    }

    napi_env env = jsRuntime_.GetNapiEnv();
    napi_value argv[] = {jsHostProxy, AbilityRuntime::CreateJsValue(env, data)};
    CallObjectMethod("onAuth", argv, ARGC_TWO);
}

void JsAgentExtension::HandleAgentInvoked(const std::string &agentId)
{
    napi_env env = jsRuntime_.GetNapiEnv();
    napi_value argv[] = { AbilityRuntime::CreateJsValue(env, agentId) };
    CallObjectMethod("onAgentInvoked", argv, ARGC_ONE);
}

napi_value JsAgentExtension::CallObjectMethod(const char* name, napi_value const* argv, size_t argc)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "name:%{public}s", name);

    if (!jsObj_) {
        TAG_LOGW(AAFwkTag::SER_ROUTER, "Not found agent_extension_ability.js");
        return nullptr;
    }

    HandleScope handleScope(jsRuntime_);
    napi_env env = jsRuntime_.GetNapiEnv();

    napi_value obj = jsObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "get object failed");
        return nullptr;
    }

    napi_value method = nullptr;
    napi_get_named_property(env, obj, name, &method);
    if (!CheckTypeForNapiValue(env, method, napi_function)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "get '%{public}s' object failed", name);
        return nullptr;
    }
    TAG_LOGD(AAFwkTag::SER_ROUTER, "CallFunction(%{public}s) ok", name);
    napi_value result = nullptr;

    TryCatch tryCatch(env);
    napi_call_function(env, obj, method, argc, argv, &result);
    if (tryCatch.HasCaught()) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "HandleUncaughtException");
        reinterpret_cast<NativeEngine*>(env)->HandleUncaughtException();
    }
    return result;
}

napi_value JsAgentExtension::WrapWant(napi_env env, const AAFwk::Want &want)
{
    AAFwk::Want jsWant = want;
    jsWant.RemoveParam(AGENTEXTENSIONHOSTPROXY_KEY);
    napi_value napiWant = OHOS::AppExecFwk::WrapWant(env, jsWant);
    return napiWant;
}

sptr<IRemoteObject> JsAgentExtension::GetHostProxyFromWant(const AAFwk::Want &want)
{
    if (!want.HasParameter(AGENTEXTENSIONHOSTPROXY_KEY)) {
        TAG_LOGW(AAFwkTag::SER_ROUTER, "Not found AGENTEXTENSIONHOSTPROXY_KEY");
        return nullptr;
    }
    return want.GetRemoteObject(AGENTEXTENSIONHOSTPROXY_KEY);
}

void JsAgentExtension::BindContext(napi_env env, napi_value obj, std::shared_ptr<AAFwk::Want> want)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "call");
    auto context = GetContext();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null context");
        return;
    }
    if (want == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null want");
        return;
    }
    std::string agentId = want->GetStringParam(AGENTID_KEY);
    std::shared_ptr<AgentCard> agentCard = std::make_shared<AgentCard>();
    auto innerErrorCode = AgentManagerClient::GetInstance().GetCallerAgentCardByAgentId(agentId, *agentCard);
    if (innerErrorCode != ERR_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "GetCallerAgentCardByAgentId error: %{public}d", innerErrorCode);
        return;
    }
    context->SetAgentCard(agentCard);
    napi_value contextObj = CreateJsAgentExtensionContext(env, context);
    shellContextRef_ = JsRuntime::LoadSystemModuleByEngine(env, "application.AgentExtensionContext",
        &contextObj, ARGC_ONE);
    if (shellContextRef_ == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null shellContextRef");
        return;
    }
    contextObj = shellContextRef_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, contextObj, napi_object)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "get context native obj failed");
        return;
    }
    auto workContext = new (std::nothrow) std::weak_ptr<Context>(std::static_pointer_cast<Context>(context));
    napi_coerce_to_native_binding_object(
        env, contextObj, DetachCallbackFunc, AttachAgentExtensionContext, workContext, nullptr);
    SetJsAgentExtensionContext(env, contextObj, context);
    TAG_LOGD(AAFwkTag::SER_ROUTER, "Bind");
    context->Bind(jsRuntime_, shellContextRef_.get());
    napi_set_named_property(env, obj, "context", contextObj);

    auto res = napi_wrap(env, contextObj, workContext,
        [](napi_env, void* data, void*) {
            delete static_cast<std::weak_ptr<Context>*>(data);
        },
        nullptr, nullptr);
    if (res != napi_ok && workContext != nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "napi_wrap failed:%{public}d", res);
        delete workContext;
        return;
    }
    TAG_LOGD(AAFwkTag::SER_ROUTER, "end");
}

void JsAgentExtension::GetSrcPath(std::string &srcPath)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "called");
    if (!Extension::abilityInfo_->srcEntrance.empty()) {
        srcPath.append(Extension::abilityInfo_->moduleName + "/");
        srcPath.append(Extension::abilityInfo_->srcEntrance);
        srcPath.erase(srcPath.rfind('.'));
        srcPath.append(".abc");
    }
}

extern "C" JS_EXPORT AgentExtension* OHOS_CreateJsAgentExtension(const std::unique_ptr<Runtime> &runtime)
{
    return new JsAgentExtension(static_cast<JsRuntime&>(*runtime));
}
} // namespace AgentRuntime
} // namespace OHOS
