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

#include "js_agent_connector_proxy.h"

#include "hilog_tag_wrapper.h"
#include "js_error_utils.h"

namespace OHOS {
namespace AgentRuntime {
using namespace AbilityRuntime;

static constexpr int32_t INDEX_ZERO = 0;
static constexpr int32_t ARGC_ONE = 1;
static constexpr const char *ERR_MSG_CONNECTION_DISCONNECTED = "The agent connection has been disconnected.";

napi_ref JsAgentConnectorProxy::CreateJsAgentConnectorProxy(napi_env env, const sptr<IRemoteObject> &connectorProxy)
{
    TAG_LOGI(AAFwkTag::SER_ROUTER, "CreateJsAgentConnectorProxy called");
    HandleEscape handleEscape(env);
    napi_value object = nullptr;
    napi_create_object(env, &object);
    if (object == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Failed to create object");
        return nullptr;
    }

    std::unique_ptr<JsAgentConnectorProxy> proxy = std::make_unique<JsAgentConnectorProxy>(connectorProxy);
    napi_status status = napi_wrap(env, object, proxy.get(), Finalizer, nullptr, nullptr);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "napi_wrap failed %{public}d", status);
        return nullptr;
    }
    proxy.release();

    const char *moduleName = "JsAgentConnectorProxy";
    BindNativeFunction(env, object, "sendData", moduleName, JsAgentConnectorProxy::SendData);
    BindNativeFunction(env, object, "authorize", moduleName, JsAgentConnectorProxy::Authorize);

    napi_ref nref = nullptr;
    status = napi_create_reference(env, object, 1, &nref);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "napi_create_reference failed %{public}d", status);
        return nullptr;
    }
    return nref;
}

void JsAgentConnectorProxy::Finalizer(napi_env env, void *data, void *hint)
{
    TAG_LOGI(AAFwkTag::SER_ROUTER, "Finalizer called");
    std::unique_ptr<JsAgentConnectorProxy>(static_cast<JsAgentConnectorProxy*>(data));
}

JsAgentConnectorProxy::JsAgentConnectorProxy(const sptr<IRemoteObject> &connectorProxy)
{
    proxy_ = iface_cast<OHOS::AgentRuntime::IAgentConnector>(connectorProxy);
    if (proxy_ == nullptr) {
        TAG_LOGW(AAFwkTag::SER_ROUTER, "proxy_ is null");
    }
}

JsAgentConnectorProxy::~JsAgentConnectorProxy()
{
    proxy_ = nullptr;
}

void JsAgentConnectorProxy::Invalidate()
{
    isDisconnected_ = true;
    proxy_ = nullptr;
}

napi_value JsAgentConnectorProxy::SendData(napi_env env, napi_callback_info info)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "SendData called from JS");
    GET_NAPI_INFO_AND_CALL(env, info, JsAgentConnectorProxy, OnSendData);
}

napi_value JsAgentConnectorProxy::Authorize(napi_env env, napi_callback_info info)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "Authorize called from JS");
    GET_NAPI_INFO_AND_CALL(env, info, JsAgentConnectorProxy, OnAuthorize);
}

napi_value JsAgentConnectorProxy::OnSendData(napi_env env, NapiCallbackInfo &info)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "OnSendData implementation");

    if (isDisconnected_) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "agent connection disconnected");
        ThrowError(env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER), ERR_MSG_CONNECTION_DISCONNECTED);
        return CreateJsUndefined(env);
    }

    if (proxy_ == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null proxy_");
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

    int32_t ret = proxy_->SendData(data);
    if (ret != static_cast<int32_t>(AbilityErrorCode::ERROR_OK)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "proxy_->SendData failed: %{public}d", ret);
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
    }

    return CreateJsUndefined(env);
}

napi_value JsAgentConnectorProxy::OnAuthorize(napi_env env, NapiCallbackInfo &info)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "OnAuthorize implementation");

    if (isDisconnected_) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "agent connection disconnected");
        ThrowError(env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER), ERR_MSG_CONNECTION_DISCONNECTED);
        return CreateJsUndefined(env);
    }

    if (proxy_ == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null proxy_");
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

    int32_t ret = proxy_->Authorize(authData);
    if (ret != static_cast<int32_t>(AbilityErrorCode::ERROR_OK)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "proxy_->Authorize failed: %{public}d", ret);
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
    }

    return CreateJsUndefined(env);
}

} // namespace AgentRuntime
} // namespace OHOS
