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

#ifndef UNITTEST_OHOS_AGENT_RUNTIME_MOCK_JS_AGENT_EXTENSION_H
#define UNITTEST_OHOS_AGENT_RUNTIME_MOCK_JS_AGENT_EXTENSION_H

#include "ability_business_error.h"
#include "js_agent_extension.h"

namespace OHOS {
namespace AgentRuntime {

/**
 * Mock implementation of JsAgentExtension for testing.
 */
class MockJsAgentExtension : public JsAgentExtension {
public:
    MockJsAgentExtension() = default;
    ~MockJsAgentExtension() = default;

    // Mock implementations for the methods called by JsAgentExtensionStubImpl
    int32_t OnSendData(const sptr<IRemoteObject>& hostProxy, const std::string& data) override
    {
        return static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_OK);
    }

    int32_t OnAuthorize(const sptr<IRemoteObject>& hostProxy, const std::string& data) override
    {
        return static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_OK);
    }

    int32_t OnAgentInvoked(const std::string& agentId) override
    {
        return static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_OK);
    }
};
} // namespace AgentRuntime
} // namespace OHOS

#endif // UNITTEST_OHOS_AGENT_RUNTIME_MOCK_JS_AGENT_EXTENSION_H
