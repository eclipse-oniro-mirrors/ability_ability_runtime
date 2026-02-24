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

#include "ets_agent_extension_stub_impl.h"

#include "ability_business_error.h"
#include "ets_agent_extension.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AgentRuntime {

EtsAgentExtensionStubImpl::EtsAgentExtensionStubImpl(std::weak_ptr<EtsAgentExtension>& ext)
    : extension_(ext)
{
    TAG_LOGI(AAFwkTag::SER_ROUTER, "EtsAgentExtensionStubImpl constructor");
}

int32_t EtsAgentExtensionStubImpl::SendData(const sptr<IRemoteObject> &hostProxy,
    const std::string &data)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "SendData called, data length: %{public}zu", data.length());
    auto sptr = extension_.lock();
    if (sptr) {
        return sptr->OnSendData(hostProxy, data);
    }
    TAG_LOGE(AAFwkTag::SER_ROUTER, "extension lock failed");
    return static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER);
}

int32_t EtsAgentExtensionStubImpl::Authorize(const sptr<IRemoteObject> &hostProxy,
    const std::string &data)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "Authorize called, data length: %{public}zu", data.length());
    auto sptr = extension_.lock();
    if (sptr) {
        return sptr->OnAuthorize(hostProxy, data);
    }
    TAG_LOGE(AAFwkTag::SER_ROUTER, "extension lock failed");
    return static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER);
}
} // namespace AgentRuntime
} // namespace OHOS
