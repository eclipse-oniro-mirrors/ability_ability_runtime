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

#include "js_agent_extension_stub_impl.h"

#include "ability_business_error.h"
#include "hilog_tag_wrapper.h"
#include "js_agent_extension.h"

namespace OHOS {
namespace AgentRuntime {
using namespace OHOS::AbilityRuntime;

JsAgentExtensionStubImpl::JsAgentExtensionStubImpl(std::weak_ptr<JsAgentExtension>& ext)
    :extension_(ext)
{
}

JsAgentExtensionStubImpl::~JsAgentExtensionStubImpl()
{
    TAG_LOGI(AAFwkTag::SER_ROUTER, "~");
}

int32_t JsAgentExtensionStubImpl::SendData(const sptr<IRemoteObject> &hostProxy, const std::string &data)
{
    auto extension = extension_.lock();
    if (extension) {
        return extension->OnSendData(hostProxy, data);
    }
    return static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER);
}

int32_t JsAgentExtensionStubImpl::Authorize(const sptr<IRemoteObject> &hostProxy, const std::string &data)
{
    auto extension = extension_.lock();
    if (extension) {
        return extension->OnAuthorize(hostProxy, data);
    }
    return static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER);
}

int32_t JsAgentExtensionStubImpl::AgentInvoked(const std::string &agentId)
{
    auto extension = extension_.lock();
    if (extension) {
        return extension->OnAgentInvoked(agentId);
    }
    return static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER);
}
}
}
