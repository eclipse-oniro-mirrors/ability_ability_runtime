/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "form_runtime/js_form_extension.h"

#include "ability_info.h"
#include "connection_manager.h"
#include "form_provider_data.h"
#include "form_runtime/form_extension_provider_client.h"
#include "form_runtime/js_form_extension_context.h"
#include "hilog_tag_wrapper.h"
#include "js_extension_context.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "napi_common_configuration.h"
#include "napi_common_util.h"
#include "napi_common_want.h"
#include <type_traits>
#include <vector>
#ifdef HIVIEWDFX_RUNTIME_API_METRICS
#include "histogram_plugin_macros.h"
#endif

namespace OHOS {
namespace AbilityRuntime {
using namespace OHOS::AppExecFwk;
const int ON_EVENT_PARAMS_SIZE = 2;
const int ARGC_TWO = 2;
const int ARGC_THREE = 3;
#ifdef HIVIEWDFX_RUNTIME_API_METRICS
constexpr bool HISTOGRAM_BOOLEAN_SAMPLE = true;
#endif

napi_value AttachFormExtensionContext(napi_env env, void* value, void*)
{
    TAG_LOGI(AAFwkTag::FORM_EXT, "call");
    if (value == nullptr) {
        TAG_LOGW(AAFwkTag::FORM_EXT, "null value");
        return nullptr;
    }
    auto ptr = reinterpret_cast<std::weak_ptr<FormExtensionContext>*>(value)->lock();
    if (ptr == nullptr) {
        TAG_LOGW(AAFwkTag::FORM_EXT, "null ptr");
        return nullptr;
    }
    napi_value object = CreateJsFormExtensionContext(env, ptr);
    auto sysModule = JsRuntime::LoadSystemModuleByEngine(env,
        "application.FormExtensionContext", &object, 1);
    if (sysModule == nullptr) {
        TAG_LOGW(AAFwkTag::FORM_EXT, "null sysModule");
        return nullptr;
    }
    auto contextObj = sysModule->GetNapiValue();
    napi_coerce_to_native_binding_object(
        env, contextObj, DetachCallbackFunc, AttachFormExtensionContext, value, nullptr);
    auto workContext = new (std::nothrow) std::weak_ptr<FormExtensionContext>(ptr);
    auto status = napi_wrap(env, contextObj, workContext,
        [](napi_env, void * data, void *) {
            TAG_LOGI(AAFwkTag::FORM_EXT, "Finalizer for weak_ptr form extension context is called");
            delete static_cast<std::weak_ptr<FormExtensionContext> *>(data);
        },
        nullptr, nullptr);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::CONTEXT, "wrap context failed: %{public}d", status);
        delete workContext;
        return nullptr;
    }
    return contextObj;
}

JsFormExtension* JsFormExtension::Create(const std::unique_ptr<Runtime>& runtime)
{
    TAG_LOGI(AAFwkTag::FORM_EXT, "call");
    return new JsFormExtension(static_cast<JsRuntime&>(*runtime));
}

JsFormExtension::JsFormExtension(JsRuntime& jsRuntime) : jsRuntime_(jsRuntime) {}
JsFormExtension::~JsFormExtension()
{
    TAG_LOGD(AAFwkTag::FORM_EXT, "destructor");
    auto context = GetContext();
    if (context) {
        context->Unbind();
    }

    jsRuntime_.FreeNativeReference(std::move(jsObj_));
    jsRuntime_.FreeNativeReference(std::move(shellContextRef_));
}

void JsFormExtension::Init(const std::shared_ptr<AbilityLocalRecord> &record,
    const std::shared_ptr<OHOSApplication> &application,
    std::shared_ptr<AbilityHandler> &handler,
    const sptr<IRemoteObject> &token)
{
    TAG_LOGD(AAFwkTag::FORM_EXT, "call");
    FormExtension::Init(record, application, handler, token);
    std::string srcPath;
    GetSrcPath(srcPath);
    if (srcPath.empty()) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "get srcPath failed");
        return;
    }

    std::string moduleName(Extension::abilityInfo_->moduleName);
    moduleName.append("::").append(abilityInfo_->name);
    TAG_LOGD(AAFwkTag::FORM_EXT, "moduleName:%{public}s,srcPath:%{public}s,"
        "compileMode :%{public}d", moduleName.c_str(), srcPath.c_str(), abilityInfo_->compileMode);
    HandleScope handleScope(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();

    jsObj_ = jsRuntime_.LoadModule(
        moduleName, srcPath, abilityInfo_->hapPath, abilityInfo_->compileMode == CompileMode::ES_MODULE);
    if (jsObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "null jsObj");
        return;
    }

    napi_value obj = jsObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "get JsFormExtension error");
        return;
    }

    BindContext(env, obj);
}

void JsFormExtension::BindContext(napi_env env, napi_value obj)
{
    auto context = GetContext();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "null context");
        return;
    }
    TAG_LOGD(AAFwkTag::FORM_EXT, "call");
    napi_value contextObj = CreateJsFormExtensionContext(env, context);
    shellContextRef_ = JsRuntime::LoadSystemModuleByEngine(env, "application.FormExtensionContext", &contextObj, 1);
    if (shellContextRef_ == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "load module failed");
        return;
    }
    contextObj = shellContextRef_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, contextObj, napi_object)) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "get context failed");
        return;
    }
    auto workContext = new (std::nothrow) std::weak_ptr<FormExtensionContext>(context);
    napi_coerce_to_native_binding_object(
        env, contextObj, DetachCallbackFunc, AttachFormExtensionContext, workContext, nullptr);
    context->Bind(jsRuntime_, shellContextRef_.get());
    napi_set_named_property(env, obj, "context", contextObj);

    auto status = napi_wrap(env, contextObj, workContext,
        [](napi_env, void* data, void*) {
            TAG_LOGI(AAFwkTag::FORM_EXT, "Finalizer for weak_ptr form extension context is called");
            delete static_cast<std::weak_ptr<FormExtensionContext>*>(data);
        },
        nullptr, nullptr);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::CONTEXT, "wrap context failed: %{public}d", status);
        delete workContext;
    }
    TAG_LOGD(AAFwkTag::FORM_EXT, "ok");
}

OHOS::AppExecFwk::FormProviderInfo JsFormExtension::OnCreate(const OHOS::AAFwk::Want& want)
{
    TAG_LOGI(AAFwkTag::FORM_EXT, "call");
    HandleScope handleScope(jsRuntime_);
    napi_env env = jsRuntime_.GetNapiEnv();
    napi_value napiWant = OHOS::AppExecFwk::WrapWant(env, want);
    napi_value argv[] = { napiWant };
    napi_value nativeResult = CallObjectMethod("onAddForm", "onCreate", argv, 1);

    OHOS::AppExecFwk::FormProviderInfo formProviderInfo;
    if (!CheckTypeForNapiValue(env, nativeResult, napi_object)) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "null nativeResult");
        return formProviderInfo;
    }

    napi_value nativeDataValue = nullptr;
    napi_get_named_property(env, nativeResult, "data", &nativeDataValue);
    if (nativeDataValue == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "null nativeResult");
        return formProviderInfo;
    }
    std::string formDataStr;
    if (!ConvertFromJsValue(env, nativeDataValue, formDataStr)) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "Convert formDataStr failed");
        return formProviderInfo;
    }
    AppExecFwk::FormProviderData formData = AppExecFwk::FormProviderData(formDataStr);
    formProviderInfo.SetFormData(formData);

    napi_value nativeProxies = nullptr;
    napi_get_named_property(env, nativeResult, "proxies", &nativeProxies);
    std::vector<FormDataProxy> formDataProxies;
    if (nativeProxies != nullptr && !ConvertFromDataProxies(env, nativeProxies, formDataProxies)) {
        TAG_LOGW(AAFwkTag::FORM_EXT, "Convert formDataProxies failed");
        return formProviderInfo;
    }
    formProviderInfo.SetFormDataProxies(formDataProxies);
    TAG_LOGI(AAFwkTag::FORM_EXT, "ok");
    return formProviderInfo;
}

void JsFormExtension::OnDestroy(const int64_t formId)
{
    TAG_LOGI(AAFwkTag::FORM_EXT, "formId: %{public}" PRId64, formId);
    FormExtension::OnDestroy(formId);

    HandleScope handleScope(jsRuntime_);
    napi_env env = jsRuntime_.GetNapiEnv();
    // wrap formId
    napi_value napiFormId = nullptr;
    napi_create_string_utf8(env, std::to_string(formId).c_str(), NAPI_AUTO_LENGTH,
        &napiFormId);
    napi_value argv[] = {napiFormId};
    CallObjectMethod("onRemoveForm", "onDestroy", argv, 1);
}

void JsFormExtension::OnStop()
{
    TAG_LOGI(AAFwkTag::FORM_EXT, "call");
    HandleScope handleScope(jsRuntime_);
    CallObjectMethod("onStop", nullptr, nullptr, 0);
    bool ret = ConnectionManager::GetInstance().DisconnectCaller(GetContext()->GetToken());
    if (ret) {
        ConnectionManager::GetInstance().ReportConnectionLeakEvent(getpid(), gettid());
        TAG_LOGI(AAFwkTag::FORM_EXT, "disconnected failed");
    }
}

void JsFormExtension::OnEvent(const int64_t formId, const std::string& message)
{
    TAG_LOGI(AAFwkTag::FORM_EXT, "formId: %{public}" PRId64, formId);
    FormExtension::OnEvent(formId, message);

    HandleScope handleScope(jsRuntime_);
    napi_env env = jsRuntime_.GetNapiEnv();
    // wrap formId
    napi_value napiFormId = nullptr;
    napi_create_string_utf8(env, std::to_string(formId).c_str(),
        NAPI_AUTO_LENGTH, &napiFormId);
    // wrap message
    napi_value napiMessage = nullptr;
    napi_create_string_utf8(env, message.c_str(), NAPI_AUTO_LENGTH, &napiMessage);
    napi_value argv[] = {napiFormId, napiMessage};
    CallObjectMethod("onFormEvent", "onEvent", argv, ON_EVENT_PARAMS_SIZE);
}

void JsFormExtension::OnUpdate(const int64_t formId, const AAFwk::WantParams &wantParams)
{
    TAG_LOGI(AAFwkTag::FORM_EXT, "formId: %{public}" PRId64, formId);
    FormExtension::OnUpdate(formId, wantParams);

    HandleScope handleScope(jsRuntime_);
    napi_env env = jsRuntime_.GetNapiEnv();
    // wrap formId
    napi_value napiFormId = nullptr;
    napi_create_string_utf8(env, std::to_string(formId).c_str(),
        NAPI_AUTO_LENGTH, &napiFormId);
    // wrap wantParams
    napi_value nativeObj = WrapWantParams(env, wantParams);
    napi_value argv[] = {napiFormId, nativeObj};
    CallObjectMethod("onUpdateForm", "onUpdate", argv, ARGC_TWO);
}

void JsFormExtension::OnCastToNormal(const int64_t formId)
{
    TAG_LOGI(AAFwkTag::FORM_EXT, "formId: %{public}" PRId64, formId);
    FormExtension::OnCastToNormal(formId);

    HandleScope handleScope(jsRuntime_);
    napi_env env = jsRuntime_.GetNapiEnv();
    // wrap formId
    napi_value napiFormId = nullptr;
    napi_create_string_utf8(env, std::to_string(formId).c_str(), NAPI_AUTO_LENGTH,
        &napiFormId);
    napi_value argv[] = {napiFormId};
    CallObjectMethod("onCastToNormalForm", "onCastToNormal", argv, 1);
}

void JsFormExtension::OnVisibilityChange(const std::map<int64_t, int32_t>& formEventsMap)
{
    TAG_LOGI(AAFwkTag::FORM_EXT, "call");
    FormExtension::OnVisibilityChange(formEventsMap);
    HandleScope handleScope(jsRuntime_);
    napi_env env = jsRuntime_.GetNapiEnv();
    napi_value nativeObj = nullptr;
    napi_create_object(env, &nativeObj);
    for (auto item = formEventsMap.begin(); item != formEventsMap.end(); item++) {
        napi_set_named_property(env, nativeObj, std::to_string(item->first).c_str(), CreateJsValue(env, item->second));
    }
    napi_value argv[] = {nativeObj};
    CallObjectMethod("onChangeFormVisibility", "onVisibilityChange", argv, 1);
}

sptr<IRemoteObject> JsFormExtension::OnConnect(const OHOS::AAFwk::Want& want)
{
    TAG_LOGD(AAFwkTag::FORM_EXT, "call");
    Extension::OnConnect(want);
    if (providerRemoteObject_ == nullptr) {
        TAG_LOGD(AAFwkTag::FORM_EXT, "null providerRemoteObject");
        sptr<FormExtensionProviderClient> providerClient = new (std::nothrow) FormExtensionProviderClient();
        if (providerClient == nullptr) {
            TAG_LOGE(AAFwkTag::FORM_EXT, "providerClient null");
            return nullptr;
        }
        std::shared_ptr<JsFormExtension> formExtension = std::static_pointer_cast<JsFormExtension>(shared_from_this());
        providerClient->SetOwner(formExtension);
        providerRemoteObject_ = providerClient->AsObject();
    }
    return providerRemoteObject_;
}

napi_value JsFormExtension::CallObjectMethod(const char* name, const char *bakName, napi_value const* argv,
    size_t argc)
{
    TAG_LOGD(AAFwkTag::FORM_EXT, "CallObjectMethod(%{public}s)", name);
    if (!jsObj_) {
        TAG_LOGW(AAFwkTag::FORM_EXT, "null jsObj");
        return nullptr;
    }

    HandleEscape handleEscape(jsRuntime_);
    napi_env env = jsRuntime_.GetNapiEnv();

    napi_value obj = jsObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "get FormExtension failed");
        return nullptr;
    }

    napi_value method = nullptr;
    napi_get_named_property(env, obj, name, &method);
    if (!CheckTypeForNapiValue(env, method, napi_function)) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "get '%{public}s' failed", name);
        if (bakName == nullptr) {
            return nullptr;
        }
        method = nullptr;
        napi_get_named_property(env, obj, bakName, &method);
        if (!CheckTypeForNapiValue(env, method, napi_function)) {
            TAG_LOGE(AAFwkTag::FORM_EXT, "get '%{public}s' failed", bakName);
            return nullptr;
        }
    }
    TAG_LOGD(AAFwkTag::FORM_EXT, "CallFunction(%{public}s), ok", name);
    napi_value result = nullptr;
    napi_call_function(env, obj, method, argc, argv, &result);
    return handleEscape.Escape(result);
}

void JsFormExtension::GetSrcPath(std::string &srcPath)
{
    if (!Extension::abilityInfo_->isModuleJson) {
        /* temporary compatibility api8 + config.json */
        srcPath.append(Extension::abilityInfo_->package);
        srcPath.append("/assets/js/");
        if (!Extension::abilityInfo_->srcPath.empty()) {
            srcPath.append(Extension::abilityInfo_->srcPath);
        }
        srcPath.append("/").append(Extension::abilityInfo_->name).append(".abc");
        return;
    }

    if (!Extension::abilityInfo_->srcEntrance.empty()) {
        srcPath.append(Extension::abilityInfo_->moduleName + "/");
        srcPath.append(Extension::abilityInfo_->srcEntrance);
        RemoveFileExtension(srcPath);
        srcPath.append(".abc");
    }
}

void JsFormExtension::OnConfigurationUpdated(const AppExecFwk::Configuration& configuration)
{
    FormExtension::OnConfigurationUpdated(configuration);
    TAG_LOGI(AAFwkTag::FORM_EXT, "call");

    HandleScope handleScope(jsRuntime_);
    napi_env env = jsRuntime_.GetNapiEnv();

    // Notify extension context
    auto fullConfig = GetContext()->GetConfiguration();
    if (!fullConfig) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "null fullConfig");
        return;
    }
    JsExtensionContext::ConfigurationUpdated(env, shellContextRef_, fullConfig);

    napi_value napiConfiguration = OHOS::AppExecFwk::WrapConfiguration(env, *fullConfig);
    CallObjectMethod("onConfigurationUpdate", "onConfigurationUpdated", &napiConfiguration, 1);
}

FormState JsFormExtension::OnAcquireFormState(const Want &want)
{
    TAG_LOGI(AAFwkTag::FORM_EXT, "call");
    auto state = static_cast<int32_t>(FormState::DEFAULT);
    HandleScope handleScope(jsRuntime_);
    napi_env env = jsRuntime_.GetNapiEnv();
    napi_value napiWant = OHOS::AppExecFwk::WrapWant(env, want);

    napi_value argv[] = { napiWant };
    napi_value nativeResult = CallObjectMethod("onAcquireFormState", nullptr, argv, 1);
    if (nativeResult == nullptr) {
        TAG_LOGI(AAFwkTag::FORM_EXT, "not found onAcquireFormState");
        return FormState::DEFAULT;
    }

    if (!ConvertFromJsValue(env, nativeResult, state)) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "convert failed");
        return FormState::UNKNOWN;
    }

    TAG_LOGI(AAFwkTag::FORM_EXT, "state: %{public}d", state);
    if (state <= static_cast<int32_t>(AppExecFwk::FormState::UNKNOWN) ||
        state > static_cast<int32_t>(AppExecFwk::FormState::READY)) {
        return AppExecFwk::FormState::UNKNOWN;
    } else {
        return static_cast<AppExecFwk::FormState>(state);
    }
}

bool JsFormExtension::OnShare(int64_t formId, AAFwk::WantParams &wantParams)
{
    TAG_LOGD(AAFwkTag::FORM_EXT, "formId: %{public}" PRId64, formId);
    HandleScope handleScope(jsRuntime_);
    napi_env env = jsRuntime_.GetNapiEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "null env");
        return false;
    }

    auto formIdStr = std::to_string(formId);
    napi_value argv[] = { CreateJsValue(env, formIdStr) };
    napi_value nativeResult = CallObjectMethod("onShareForm", "onShare", argv, 1);
    if (nativeResult == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "null nativeResult");
        return false;
    }

    if (!CheckTypeForNapiValue(env, nativeResult, napi_object)) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "nativeResult not object");
        return false;
    }

    if (!OHOS::AppExecFwk::UnwrapWantParams(env, nativeResult, wantParams)) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "unwrap want failed");
        return false;
    }

    return true;
}

bool JsFormExtension::OnAcquireData(int64_t formId, AAFwk::WantParams &wantParams)
{
    TAG_LOGD(AAFwkTag::FORM_EXT, "formId: %{public}" PRId64, formId);
    HandleScope handleScope(jsRuntime_);
    napi_env env = jsRuntime_.GetNapiEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "null env");
        return false;
    }

    auto formIdStr = std::to_string(formId);
    napi_value argv[] = { CreateJsValue(env, formIdStr) };
    napi_value nativeResult = CallObjectMethod("onAcquireFormData", "OnAcquireData", argv, 1);
    if (nativeResult == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "null nativeResult");
        return false;
    }

    if (!CheckTypeForNapiValue(env, nativeResult, napi_object)) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "nativeResult not object");
        return false;
    }

    if (!OHOS::AppExecFwk::UnwrapWantParams(env, nativeResult, wantParams)) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "argc failed");
        return false;
    }
    return true;
}

bool JsFormExtension::ConvertFromDataProxies(napi_env env, napi_value jsValue,
    std::vector<FormDataProxy> &formDataProxies)
{
    if (jsValue == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "null jsValue");
        return false;
    }

    uint32_t len = 0;
    NAPI_CALL_NO_THROW(napi_get_array_length(env, jsValue, &len), false);
    for (uint32_t i = 0; i < len; i++) {
        FormDataProxy formDataProxy("", "");
        napi_value element = nullptr;
        napi_get_element(env, jsValue, i, &element);
        if (!ConvertFormDataProxy(env, element, formDataProxy)) {
            TAG_LOGE(AAFwkTag::FORM_EXT, "GetElement [%{public}u] error", i);
            continue;
        }
        formDataProxies.push_back(formDataProxy);
    }
    return true;
}

bool JsFormExtension::ConvertFormDataProxy(napi_env env, napi_value jsValue, FormDataProxy &formDataProxy)
{
    if (!CheckTypeForNapiValue(env, jsValue, napi_object)) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "null jsValue");
        return false;
    }

    napi_value key = nullptr;
    napi_get_named_property(env, jsValue, "key", &key);
    if (!ConvertFromJsValue(env, key, formDataProxy.key)) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "Parse key failed");
        return false;
    }
    napi_value subscribeId = nullptr;
    napi_get_named_property(env, jsValue, "subscriberId", &subscribeId);
    if (subscribeId != nullptr && !ConvertFromJsValue(env, subscribeId, formDataProxy.subscribeId)) {
        TAG_LOGW(AAFwkTag::FORM_EXT, "null subscribeId");
        formDataProxy.subscribeId = "";
    }
    TAG_LOGI(AAFwkTag::FORM_EXT, "key is %{public}s, subscriberId is %{public}s", formDataProxy.key.c_str(),
        formDataProxy.subscribeId.c_str());
    return true;
}

void JsFormExtension::OnFormLocationChanged(const int64_t formId, const int32_t formLocation)
{
#ifdef HIVIEWDFX_RUNTIME_API_METRICS
    HISTOGRAM_BOOLEAN("Form.ExtensionAbility.onFormLocationChanged", HISTOGRAM_BOOLEAN_SAMPLE);
#endif
    TAG_LOGI(AAFwkTag::FORM_EXT, "Card position changes, location: %{public}d, formId: %{public}" PRId64, formLocation, formId);
    FormExtension::OnFormLocationChanged(formId, formLocation);
 
    HandleScope handleScope(jsRuntime_);
    napi_env env = jsRuntime_.GetNapiEnv();
 
    napi_value napiFormId = WrapStringToJS(env, std::to_string(formId));
    napi_value napiLocation = WrapInt32ToJS(env, formLocation);
    napi_value argv[] = {napiFormId, napiLocation};
    CallObjectMethod("onFormLocationChanged", "OnFormLocationChanged", argv, ARGC_TWO);
}

void JsFormExtension::OnSizeChanged(int64_t formId, int32_t newDimension, const Rect &newRect)
{
#ifdef HIVIEWDFX_RUNTIME_API_METRICS
    HISTOGRAM_BOOLEAN("Form.ExtensionAbility.onSizeChanged", HISTOGRAM_BOOLEAN_SAMPLE);
#endif
    TAG_LOGI(AAFwkTag::FORM_EXT, "Form size changed, formId: %{public}" PRId64, formId);
    FormExtension::OnSizeChanged(formId, newDimension, newRect);
 
    HandleScope handleScope(jsRuntime_);
    napi_env env = jsRuntime_.GetNapiEnv();
 
    napi_value napiFormId = WrapStringToJS(env, std::to_string(formId));
    napi_value napinewDimension = WrapInt32ToJS(env, newDimension);
    napi_value napiNewRect = nullptr;
    napi_create_object(env, &napiNewRect);
    napi_set_named_property(env, napiNewRect, "left", CreateJsValue(env, newRect.left));
    napi_set_named_property(env, napiNewRect, "top", CreateJsValue(env, newRect.top));
    napi_set_named_property(env, napiNewRect, "width", CreateJsValue(env, newRect.width));
    napi_set_named_property(env, napiNewRect, "height", CreateJsValue(env, newRect.height));
    napi_value argv[] = {napiFormId, napinewDimension, napiNewRect};
    CallObjectMethod("onSizeChanged", "onSizeChanged", argv, ARGC_THREE);
}
} // namespace AbilityRuntime
} // namespace OHOS
