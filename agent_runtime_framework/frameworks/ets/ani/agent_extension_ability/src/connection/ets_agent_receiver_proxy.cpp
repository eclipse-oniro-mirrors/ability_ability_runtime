/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License"),
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

#include "ets_agent_receiver_proxy.h"

#include "ability_business_error.h"
#include "ani_common_util.h"
#include "ets_error_utils.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AgentRuntime {

constexpr const char *CLASS_NAME_AGENT_RECEIVER_PROXY_IMPL = "application.AgentProxy.AgentProxyImpl";

ani_object EtsAgentReceiverProxy::CreateEtsAgentReceiverProxy(ani_env *env,
    const sptr<IRemoteObject> &impl, int64_t connectionId, const sptr<IRemoteObject> &connectorProxy)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "create receiver, id: %{public}s", std::to_string(connectionId).c_str());
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null env");
        return nullptr;
    }
    ani_object object = nullptr;
    ani_class cls {};
    ani_status status = env->FindClass(CLASS_NAME_AGENT_RECEIVER_PROXY_IMPL, &cls);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "FindClass failed status: %{public}d", status);
        return nullptr;
    }
    ani_method method = nullptr;
    status = env->Class_FindMethod(cls, "<ctor>", ":", &method);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Class_FindMethod <ctor> failed");
        return nullptr;
    }
    status = env->Object_New(cls, method, &object);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Object_New failed");
        return nullptr;
    }
    ani_field field = nullptr;
    if ((status = env->Class_FindField(cls, "nativeReceiverProxy", &field)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Class_FindField failed status: %{public}d", status);
        return nullptr;
    }
    std::unique_ptr<EtsAgentReceiverProxy> receiverProxyPtr =
        std::make_unique<EtsAgentReceiverProxy>(impl, connectorProxy);
    receiverProxyPtr->SetConnectionId(connectionId);

    ani_long nativeReceiverProxyLong = reinterpret_cast<ani_long>(receiverProxyPtr.release());
    if ((status = env->Object_SetField_Long(object, field, nativeReceiverProxyLong)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Object_SetField_Long failed status: %{public}d", status);
        return nullptr;
    }

    std::array functions = {
        ani_native_function {"nativeSendData", "C{std.core.String}:",
            reinterpret_cast<void *>(EtsAgentReceiverProxy::SendData)},
        ani_native_function {"nativeAuthorize", "C{std.core.String}:",
            reinterpret_cast<void *>(EtsAgentReceiverProxy::Authorize)},
    };

    if ((status = env->Class_BindNativeMethods(cls, functions.data(), functions.size())) != ANI_OK
        && status != ANI_ALREADY_BINDED) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Class_BindNativeMethods failed status: %{public}d", status);
        return nullptr;
    }
    return object;
}

EtsAgentReceiverProxy* EtsAgentReceiverProxy::GetEtsAgentReceiverProxy(ani_env *env, ani_object obj)
{
    if (env == nullptr || obj == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null env or obj");
        return nullptr;
    }
    EtsAgentReceiverProxy *receiverProxy = nullptr;
    ani_status status = ANI_ERROR;
    ani_long nativeReceiverProxyLong = 0;
    if ((status = env->Object_GetFieldByName_Long(obj, "nativeReceiverProxy",
        &nativeReceiverProxyLong)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Object_GetFieldByName_Long failed status: %{public}d", status);
        return nullptr;
    }
    receiverProxy = reinterpret_cast<EtsAgentReceiverProxy *>(nativeReceiverProxyLong);
    if (receiverProxy == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "receiverProxy null");
        return nullptr;
    }
    return receiverProxy;
}

EtsAgentReceiverProxy::EtsAgentReceiverProxy(const sptr<IRemoteObject> &impl,
    const sptr<IRemoteObject> &connectorProxy)
    : connectorProxy_(connectorProxy)
{
    proxy_ = iface_cast<IAgentReceiver>(impl);
}

EtsAgentReceiverProxy::~EtsAgentReceiverProxy()
{
    TAG_LOGI(AAFwkTag::SER_ROUTER, "~EtsAgentReceiverProxy");
    proxy_ = nullptr;
    connectorProxy_ = nullptr;
}

void EtsAgentReceiverProxy::SendData(ani_env *env, ani_object obj, ani_string data)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "SendData called");
    EtsAgentReceiverProxy* receiverProxy = GetEtsAgentReceiverProxy(env, obj);
    if (receiverProxy == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "GetEtsAgentReceiverProxy failed");
        AbilityRuntime::EtsErrorUtil::ThrowError(env, AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER);
        return;
    }

    std::string dataStr;
    if (!AppExecFwk::GetStdString(env, data, dataStr)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "GetStdString failed");
        AbilityRuntime::EtsErrorUtil::ThrowInvalidParamError(env, "Parameter error. data must be string.");
        return;
    }

    receiverProxy->OnSendData(env, dataStr);
}

void EtsAgentReceiverProxy::Authorize(ani_env *env, ani_object obj, ani_string data)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "Authorize called");
    EtsAgentReceiverProxy* receiverProxy = GetEtsAgentReceiverProxy(env, obj);
    if (receiverProxy == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "GetEtsAgentReceiverProxy failed");
        AbilityRuntime::EtsErrorUtil::ThrowError(env, AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER);
        return;
    }

    std::string dataStr;
    if (!AppExecFwk::GetStdString(env, data, dataStr)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "GetStdString failed");
        AbilityRuntime::EtsErrorUtil::ThrowInvalidParamError(env, "Parameter error. data must be string.");
        return;
    }

    receiverProxy->OnAuthorize(env, dataStr);
}

void EtsAgentReceiverProxy::OnSendData(ani_env *env, const std::string &data)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "OnSendData called, data length: %{public}zu", data.length());
    if (proxy_ == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "proxy_ is null");
        AbilityRuntime::EtsErrorUtil::ThrowError(env, AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER);
        return;
    }

    int32_t ret = proxy_->SendData(connectorProxy_, data);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "SendData failed: %{public}d", ret);
        AbilityRuntime::EtsErrorUtil::ThrowErrorByNativeErr(env, ret);
    }
}

void EtsAgentReceiverProxy::OnAuthorize(ani_env *env, const std::string &data)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "OnAuthorize called, data length: %{public}zu", data.length());
    if (proxy_ == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "proxy_ is null");
        AbilityRuntime::EtsErrorUtil::ThrowError(env, AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER);
        return;
    }

    int32_t ret = proxy_->Authorize(connectorProxy_, data);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Authorize failed: %{public}d", ret);
        AbilityRuntime::EtsErrorUtil::ThrowErrorByNativeErr(env, ret);
    }
}
} // namespace AgentRuntime
} // namespace OHOS
