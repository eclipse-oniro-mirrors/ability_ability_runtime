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

#include "agent_receiver_proxy.h"

#include "ability_business_error.h"
#include "hilog_tag_wrapper.h"
#include "ipc_types.h"

namespace OHOS {
namespace AgentRuntime {

AgentReceiverProxy::AgentReceiverProxy(const sptr<IRemoteObject> &impl)
    : IRemoteProxy<IAgentReceiver>(impl)
{
}

AgentReceiverProxy::~AgentReceiverProxy()
{
}

int32_t AgentReceiverProxy::SendData(const sptr<IRemoteObject> &connectorProxy, const std::string &data)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "SendData called, data length: %{public}zu", data.length());

    if (connectorProxy == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null connectorProxy");
        return static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER);
    }

    MessageParcel parcelData;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);

    if (!parcelData.WriteInterfaceToken(AgentReceiverProxy::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "write interface token failed");
        return static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER);
    }

    if (!parcelData.WriteRemoteObject(connectorProxy)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "write connectorProxy failed");
        return static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER);
    }

    if (!parcelData.WriteString(data)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "write data failed");
        return static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER);
    }

    sptr<IRemoteObject> remoteObject = Remote();
    if (remoteObject == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null remoteObject");
        return static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER);
    }

    auto error = remoteObject->SendRequest(static_cast<uint32_t>(IAgentReceiver::SEND_DATA),
        parcelData, reply, option);
    if (error != ERR_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "SendRequest error %{public}d", error);
        return static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER);
    }

    TAG_LOGD(AAFwkTag::SER_ROUTER, "SendData succeeded");
    return ERR_OK;
}

int32_t AgentReceiverProxy::Authorize(const sptr<IRemoteObject> &connectorProxy, const std::string &data)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "Authorize called, auth length: %{public}zu", data.length());

    if (connectorProxy == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null connectorProxy");
        return static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER);
    }

    MessageParcel parcelData;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);

    if (!parcelData.WriteInterfaceToken(AgentReceiverProxy::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "write interface token failed");
        return static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER);
    }

    if (!parcelData.WriteRemoteObject(connectorProxy)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "write connectorProxy failed");
        return static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER);
    }

    if (!parcelData.WriteString(data)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "write auth data failed");
        return static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER);
    }

    sptr<IRemoteObject> remoteObject = Remote();
    if (remoteObject == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null remoteObject");
        return static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER);
    }

    auto error = remoteObject->SendRequest(static_cast<uint32_t>(IAgentReceiver::AUTHORIZE),
        parcelData, reply, option);
    if (error != ERR_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "SendRequest error %{public}d", error);
        return static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER);
    }

    TAG_LOGD(AAFwkTag::SER_ROUTER, "Authorize succeeded");
    return ERR_OK;
}

int32_t AgentReceiverProxy::AgentInvoked(const std::string &agentId)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "AgentInvoked called, agentId: %{public}s", agentId.c_str());

    MessageParcel parcelData;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);

    if (!parcelData.WriteInterfaceToken(AgentReceiverProxy::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "write interface token failed");
        return static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER);
    }
    if (!parcelData.WriteString(agentId)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "write agentId failed");
        return static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER);
    }

    sptr<IRemoteObject> remoteObject = Remote();
    if (remoteObject == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null remoteObject");
        return static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER);
    }

    auto error = remoteObject->SendRequest(static_cast<uint32_t>(IAgentReceiver::AGENT_INVOKED),
        parcelData, reply, option);
    if (error != ERR_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "SendRequest error %{public}d", error);
        return static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER);
    }
    return ERR_OK;
}

} // namespace AgentRuntime
} // namespace OHOS
