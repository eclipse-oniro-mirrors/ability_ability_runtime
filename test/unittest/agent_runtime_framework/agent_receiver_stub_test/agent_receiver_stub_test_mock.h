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

#ifndef UNITTEST_OHOS_AGENT_RUNTIME_AGENT_RECEIVER_STUB_TEST_MOCK_H
#define UNITTEST_OHOS_AGENT_RUNTIME_AGENT_RECEIVER_STUB_TEST_MOCK_H

#define private public
#include "agent_receiver_stub.h"
#undef private

#include <gmock/gmock.h>
#include <iremote_object.h>
#include <iremote_stub.h>
#include "ipc_types.h"

namespace OHOS {
namespace AgentRuntime {

class MockRemoteObject : public IRemoteObject {
public:
    MockRemoteObject() : IRemoteObject(u"mock_descriptor") {}
    virtual ~MockRemoteObject() = default;

    int32_t GetObjectRefCount() override
    {
        return 0;
    }

    int SendRequest(uint32_t code, MessageParcel& data, MessageParcel& reply,
        MessageOption& option) override
    {
        return ERR_OK;
    }

    bool IsProxyObject() const override
    {
        return true;
    }

    bool CheckObjectLegality() const override
    {
        return true;
    }

    bool AddDeathRecipient(const sptr<DeathRecipient> &recipient) override
    {
        return true;
    }

    bool RemoveDeathRecipient(const sptr<DeathRecipient> &recipient) override
    {
        return true;
    }

    bool Marshalling(Parcel &parcel) const override
    {
        return true;
    }

    sptr<IRemoteBroker> AsInterface() override
    {
        return nullptr;
    }

    int Dump(int fd, const std::vector<std::u16string> &args) override
    {
        return 0;
    }

    std::u16string GetObjectDescriptor() const
    {
        std::u16string descriptor = std::u16string();
        return descriptor;
    }
};

class AgentReceiverStubTestMock : public AgentReceiverStub {
public:
    AgentReceiverStubTestMock() = default;
    virtual ~AgentReceiverStubTestMock() = default;

    std::string lastWorkflowAgentId;
    int32_t agentInvokedResult = 0;

    int32_t SendData(const sptr<IRemoteObject> &connectorProxy, const std::string &data) override
    {
        return 0;
    }

    int32_t Authorize(const sptr<IRemoteObject> &connectorProxy, const std::string &data) override
    {
        return 0;
    }

    int32_t AgentInvoked(const std::string &agentId) override
    {
        lastWorkflowAgentId = agentId;
        return agentInvokedResult;
    }
};

} // namespace AgentRuntime
} // namespace OHOS

#endif // UNITTEST_OHOS_AGENT_RUNTIME_AGENT_RECEIVER_STUB_TEST_MOCK_H
