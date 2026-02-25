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

#include "ets_agent_connector_stub_impl.h"

#include "ability_business_error.h"
#include "ets_agent_connection.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AgentRuntime {

EtsAgentConnectorStubImpl::EtsAgentConnectorStubImpl(wptr<EtsAgentConnection> conn) : conn_(conn)
{
    TAG_LOGI(AAFwkTag::SER_ROUTER, "EtsAgentConnectorStubImpl constructor");
}

int32_t EtsAgentConnectorStubImpl::SendData(const std::string &data)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "SendData called, data length: %{public}zu", data.length());
    sptr<EtsAgentConnection> connection = conn_.promote();
    if (connection == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "connection is null");
        return static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER);
    }
    return connection->OnSendData(data);
}

int32_t EtsAgentConnectorStubImpl::Authorize(const std::string &data)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "Authorize called, data length: %{public}zu", data.length());
    sptr<EtsAgentConnection> connection = conn_.promote();
    if (connection == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "connection is null");
        return static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER);
    }
    return connection->OnAuthorize(data);
}

} // namespace AgentRuntime
} // namespace OHOS
