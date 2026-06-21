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

#include "js_agent_receiver_proxy.h"

#include "hilog_tag_wrapper.h"
#include "js_error_utils.h"

namespace OHOS {
namespace AgentRuntime {
using namespace AbilityRuntime;

static constexpr int32_t INDEX_ZERO = 0;
static constexpr int32_t ARGC_ONE = 1;

napi_value JsAgentReceiverProxy::CreateJsAgentReceiverProxy(napi_env env,
    const sptr<IRemoteObject> &impl,
    int64_t connectionId,
    const sptr<IRemoteObject> &connectorProxy)
{
    TAG_LOGI(AAFwkTag::SER_ROUTER, "CreateJsAgentReceiverProxy called, connectionId: %{public}s",
        std::to_string(connectionId).c_str());
    HandleEscape handleEscape(env);
    napi_value object = nullptr;
    napi_status status = napi_create_object(env, &object);
    if (status != napi_ok || object == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Failed to create object, status: %{public}d", status);
        return CreateJsUndefined(env);
    }

    std::unique_ptr<JsAgentReceiverProxy> proxy = std::make_unique<JsAgentReceiverProxy>(impl, connectorProxy);
    proxy->SetConnectionId(connectionId);
    status = napi_wrap(env, object, proxy.get(), Finalizer, nullptr, nullptr);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "napi_wrap failed %{public}d", status);
        return CreateJsUndefined(env);
    }
    proxy.release();

    const char *moduleName = "JsAgentReceiverProxy";
    BindNativeFunction(env, object, "sendData", moduleName, JsAgentReceiverProxy::SendData);
    BindNativeFunction(env, object, "authorize", moduleName, JsAgentReceiverProxy::Authorize);
    return handleEscape.Escape(object);
}

void JsAgentReceiverProxy::Finalizer(napi_env env, void *data, void *hint)
{
    TAG_LOGI(AAFwkTag::SER_ROUTER, "Finalizer called");
    std::unique_ptr<JsAgentReceiverProxy>(static_cast<JsAgentReceiverProxy*>(data));
}

JsAgentReceiverProxy::JsAgentReceiverProxy(const sptr<IRemoteObject> &impl, const sptr<IRemoteObject> &connectorProxy)
{
    proxy_ = iface_cast<OHOS::AgentRuntime::IAgentReceiver>(impl);
    connectorProxy_ = connectorProxy;
    if (proxy_ == nullptr) {
        TAG_LOGW(AAFwkTag::SER_ROUTER, "proxy_ is null");
    }
}

JsAgentReceiverProxy::~JsAgentReceiverProxy()
{
    proxy_ = nullptr;
    connectorProxy_ = nullptr;
}

napi_value JsAgentReceiverProxy::SendData(napi_env env, napi_callback_info info)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "SendData called from JS");
    GET_NAPI_INFO_AND_CALL(env, info, JsAgentReceiverProxy, OnSendData);
}

napi_value JsAgentReceiverProxy::Authorize(napi_env env, napi_callback_info info)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "Authorize called from JS");
    GET_NAPI_INFO_AND_CALL(env, info, JsAgentReceiverProxy, OnAuthorize);
}

napi_value JsAgentReceiverProxy::OnSendData(napi_env env, NapiCallbackInfo &info)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "OnSendData implementation");

    if (proxy_ == nullptr || connectorProxy_ == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null proxy_ or connectorProxy_");
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
        return CreateJsUndefined(env);
    }

    if (info.argc < ARGC_ONE) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Invalid argc: %{public}zu", info.argc);
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }

    // Extract string parameter
    std::string data;
    if (!ConvertFromJsValue(env, info.argv[INDEX_ZERO], data)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Failed to convert parameter to string");
        ThrowInvalidParamError(env, "Parameter must be a string");
        return CreateJsUndefined(env);
    }

    TAG_LOGD(AAFwkTag::SER_ROUTER, "Sending data, length: %{public}zu", data.length());

    int32_t ret = proxy_->SendData(connectorProxy_, data);
    if (ret != static_cast<int32_t>(AbilityErrorCode::ERROR_OK)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "proxy_->SendData failed: %{public}d", ret);
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
    }

    return CreateJsUndefined(env);
}

napi_value JsAgentReceiverProxy::OnAuthorize(napi_env env, NapiCallbackInfo &info)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "OnAuthorize implementation");

    if (proxy_ == nullptr || connectorProxy_ == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null proxy_ or connectorProxy_");
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
        return CreateJsUndefined(env);
    }

    if (info.argc < ARGC_ONE) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Invalid argc: %{public}zu", info.argc);
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }

    // Extract string parameter
    std::string authData;
    if (!ConvertFromJsValue(env, info.argv[INDEX_ZERO], authData)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Failed to convert parameter to string");
        ThrowInvalidParamError(env, "Parameter must be a string");
        return CreateJsUndefined(env);
    }

    TAG_LOGD(AAFwkTag::SER_ROUTER, "Sending auth, length: %{public}zu", authData.length());

    int32_t ret = proxy_->Authorize(connectorProxy_, authData);
    if (ret != static_cast<int32_t>(AbilityErrorCode::ERROR_OK)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "proxy_->Authorize failed: %{public}d", ret);
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
    }

    return CreateJsUndefined(env);
}

} // namespace AgentRuntime
} // namespace OHOS
