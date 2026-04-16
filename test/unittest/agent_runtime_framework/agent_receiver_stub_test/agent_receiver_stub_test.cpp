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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "agent_receiver_stub.h"
#include "agent_receiver_stub_test_mock.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AgentRuntime {
class AgentReceiverStubTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    void WriteInterfaceToken(MessageParcel& data);

    sptr<AgentReceiverStubTestMock> stub_{ nullptr };
};

void AgentReceiverStubTest::SetUpTestCase(void)
{}
void AgentReceiverStubTest::TearDownTestCase(void)
{}
void AgentReceiverStubTest::SetUp()
{
    stub_ = new AgentReceiverStubTestMock();
}
void AgentReceiverStubTest::TearDown()
{
    stub_ = nullptr;
}

void AgentReceiverStubTest::WriteInterfaceToken(MessageParcel& data)
{
    data.WriteInterfaceToken(AgentReceiverStubTestMock::GetDescriptor());
}

/*
 * Feature: AgentReceiverStub
 * Function: Constructor
 * SubFunction: NA
 * FunctionPoints: AgentReceiverStub construction
 * EnvConditions: NA
 * CaseDescription: Verify that AgentReceiverStub can be constructed successfully
 */
HWTEST_F(AgentReceiverStubTest, AgentReceiverStub_Constructor_001, TestSize.Level1)
{
    sptr<AgentReceiverStubTestMock> stub = new AgentReceiverStubTestMock();
    EXPECT_NE(stub, nullptr);
}

/*
 * Feature: AgentReceiverStub
 * Function: OnRemoteRequest
 * SubFunction: NA
 * FunctionPoints: OnRemoteRequest with invalid descriptor
 * EnvConditions: Descriptor mismatch
 * CaseDescription: Verify that OnRemoteRequest returns ERR_INVALID_STATE with invalid descriptor
 */
HWTEST_F(AgentReceiverStubTest, AgentReceiverStub_OnRemoteRequest_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(u"invalid.descriptor");

    int result = stub_->OnRemoteRequest(IAgentReceiver::SEND_DATA, data, reply, option);

    EXPECT_EQ(result, ERR_INVALID_STATE);
}

/*
 * Feature: AgentReceiverStub
 * Function: OnRemoteRequest
 * SubFunction: NA
 * FunctionPoints: OnRemoteRequest with unknown code
 * EnvConditions: Unknown command code
 * CaseDescription: Verify that OnRemoteRequest handles unknown code correctly
 */
HWTEST_F(AgentReceiverStubTest, AgentReceiverStub_OnRemoteRequest_002, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    WriteInterfaceToken(data);

    int result = stub_->OnRemoteRequest(9999, data, reply, option);

    EXPECT_EQ(result, IPC_STUB_UNKNOW_TRANS_ERR);
}

/*
 * Feature: AgentReceiverStub
 * Function: OnRemoteRequest
 * SubFunction: SEND_DATA
 * FunctionPoints: OnRemoteRequest with SEND_DATA code
 * EnvConditions: Valid descriptor, connector proxy and data
 * CaseDescription: Verify that OnRemoteRequest handles SEND_DATA correctly
 */
HWTEST_F(AgentReceiverStubTest, AgentReceiverStub_OnRemoteRequest_003, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    WriteInterfaceToken(data);
    sptr<IRemoteObject> connectorProxy = new MockRemoteObject();
    data.WriteRemoteObject(connectorProxy);
    data.WriteString("test data");

    int result = stub_->OnRemoteRequest(IAgentReceiver::SEND_DATA, data, reply, option);

    EXPECT_EQ(result, NO_ERROR);
}

/*
 * Feature: AgentReceiverStub
 * Function: OnRemoteRequest
 * SubFunction: AUTHORIZE
 * FunctionPoints: OnRemoteRequest with AUTHORIZE code
 * EnvConditions: Valid descriptor, connector proxy and data
 * CaseDescription: Verify that OnRemoteRequest handles AUTHORIZE correctly
 */
HWTEST_F(AgentReceiverStubTest, AgentReceiverStub_OnRemoteRequest_004, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    WriteInterfaceToken(data);
    sptr<IRemoteObject> connectorProxy = new MockRemoteObject();
    data.WriteRemoteObject(connectorProxy);
    data.WriteString("auth data");

    int result = stub_->OnRemoteRequest(IAgentReceiver::AUTHORIZE, data, reply, option);

    EXPECT_EQ(result, NO_ERROR);
}

/*
 * Feature: AgentReceiverStub
 * Function: OnSendData
 * SubFunction: NA
 * FunctionPoints: OnSendData with valid data
 * EnvConditions: Valid data parcel
 * CaseDescription: Verify that OnSendData returns NO_ERROR with valid data
 */
HWTEST_F(AgentReceiverStubTest, AgentReceiverStub_OnSendData_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;

    WriteInterfaceToken(data);
    sptr<IRemoteObject> connectorProxy = new MockRemoteObject();
    data.WriteRemoteObject(connectorProxy);
    data.WriteString("test data");

    int result = stub_->OnSendData(data, reply);

    EXPECT_EQ(result, NO_ERROR);
}

/*
 * Feature: AgentReceiverStub
 * Function: OnSendData
 * SubFunction: NA
 * FunctionPoints: OnSendData with empty string
 * EnvConditions: Valid data parcel with empty string
 * CaseDescription: Verify that OnSendData works with empty string
 */
HWTEST_F(AgentReceiverStubTest, AgentReceiverStub_OnSendData_003, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;

    WriteInterfaceToken(data);
    sptr<IRemoteObject> connectorProxy = new MockRemoteObject();
    data.WriteRemoteObject(connectorProxy);
    data.WriteString("");

    int result = stub_->OnSendData(data, reply);

    EXPECT_EQ(result, NO_ERROR);
}

/*
 * Feature: AgentReceiverStub
 * Function: OnSendData
 * SubFunction: NA
 * FunctionPoints: OnSendData with long string
 * EnvConditions: Valid data parcel with long string
 * CaseDescription: Verify that OnSendData works with long string
 */
HWTEST_F(AgentReceiverStubTest, AgentReceiverStub_OnSendData_004, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;

    WriteInterfaceToken(data);
    sptr<IRemoteObject> connectorProxy = new MockRemoteObject();
    data.WriteRemoteObject(connectorProxy);
    std::string longData(10000, 'x');
    data.WriteString(longData);

    int result = stub_->OnSendData(data, reply);

    EXPECT_EQ(result, NO_ERROR);
}

/*
 * Feature: AgentReceiverStub
 * Function: OnSendData
 * SubFunction: NA
 * FunctionPoints: OnSendData with null connector proxy
 * EnvConditions: Valid data parcel with null connector proxy
 * CaseDescription: Verify that OnSendData works with null connector proxy
 */
HWTEST_F(AgentReceiverStubTest, AgentReceiverStub_OnSendData_005, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;

    WriteInterfaceToken(data);
    data.WriteRemoteObject(nullptr);
    data.WriteString("test data");

    int result = stub_->OnSendData(data, reply);

    EXPECT_EQ(result, NO_ERROR);
}

/*
 * Feature: AgentReceiverStub
 * Function: OnAuthorize
 * SubFunction: NA
 * FunctionPoints: OnAuthorize with valid data
 * EnvConditions: Valid data parcel
 * CaseDescription: Verify that OnAuthorize returns NO_ERROR with valid data
 */
HWTEST_F(AgentReceiverStubTest, AgentReceiverStub_OnAuthorize_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;

    WriteInterfaceToken(data);
    sptr<IRemoteObject> connectorProxy = new MockRemoteObject();
    data.WriteRemoteObject(connectorProxy);
    data.WriteString("auth data");

    int result = stub_->OnAuthorize(data, reply);

    EXPECT_EQ(result, NO_ERROR);
}

/*
 * Feature: AgentReceiverStub
 * Function: OnAuthorize
 * SubFunction: NA
 * FunctionPoints: OnAuthorize with empty string
 * EnvConditions: Valid data parcel with empty string
 * CaseDescription: Verify that OnAuthorize works with empty string
 */
HWTEST_F(AgentReceiverStubTest, AgentReceiverStub_OnAuthorize_003, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;

    WriteInterfaceToken(data);
    sptr<IRemoteObject> connectorProxy = new MockRemoteObject();
    data.WriteRemoteObject(connectorProxy);
    data.WriteString("");

    int result = stub_->OnAuthorize(data, reply);

    EXPECT_EQ(result, NO_ERROR);
}

/*
 * Feature: AgentReceiverStub
 * Function: OnAuthorize
 * SubFunction: NA
 * FunctionPoints: OnAuthorize with long string
 * EnvConditions: Valid data parcel with long string
 * CaseDescription: Verify that OnAuthorize works with long string
 */
HWTEST_F(AgentReceiverStubTest, AgentReceiverStub_OnAuthorize_004, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;

    WriteInterfaceToken(data);
    sptr<IRemoteObject> connectorProxy = new MockRemoteObject();
    data.WriteRemoteObject(connectorProxy);
    std::string longData(10000, 'y');
    data.WriteString(longData);

    int result = stub_->OnAuthorize(data, reply);

    EXPECT_EQ(result, NO_ERROR);
}

/*
 * Feature: AgentReceiverStub
 * Function: OnAuthorize
 * SubFunction: NA
 * FunctionPoints: OnAuthorize with null connector proxy
 * EnvConditions: Valid data parcel with null connector proxy
 * CaseDescription: Verify that OnAuthorize works with null connector proxy
 */
HWTEST_F(AgentReceiverStubTest, AgentReceiverStub_OnAuthorize_005, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;

    WriteInterfaceToken(data);
    data.WriteRemoteObject(nullptr);
    data.WriteString("auth data");

    int result = stub_->OnAuthorize(data, reply);

    EXPECT_EQ(result, NO_ERROR);
}

/*
 * Feature: AgentReceiverStub
 * Function: OnRemoteRequest
 * SubFunction: AGENT_INVOKED
 * FunctionPoints: OnRemoteRequest with AGENT_INVOKED code
 * EnvConditions: Valid descriptor and workflow agent id
 * CaseDescription: Verify that OnRemoteRequest handles AGENT_INVOKED correctly
 */
HWTEST_F(AgentReceiverStubTest, AgentReceiverStub_OnRemoteRequest_005, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    WriteInterfaceToken(data);
    data.WriteString("workflowAgent");

    int result = stub_->OnRemoteRequest(IAgentReceiver::AGENT_INVOKED, data, reply, option);

    EXPECT_EQ(result, NO_ERROR);
    EXPECT_EQ(stub_->lastWorkflowAgentId, "workflowAgent");
}

/*
 * Feature: AgentReceiverStub
 * Function: OnAgentInvoked
 * SubFunction: NA
 * FunctionPoints: OnAgentInvoked with valid low-code agent id
 * EnvConditions: Valid data parcel
 * CaseDescription: Verify that OnAgentInvoked returns NO_ERROR and forwards agent id
 */
HWTEST_F(AgentReceiverStubTest, AgentReceiverStub_OnAgentInvoked_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;

    data.WriteString("workflowAgent");

    int result = stub_->OnAgentInvoked(data, reply);

    EXPECT_EQ(result, NO_ERROR);
    EXPECT_EQ(stub_->lastWorkflowAgentId, "workflowAgent");
    EXPECT_EQ(reply.ReadInt32(), NO_ERROR);
}

/*
 * Feature: AgentReceiverStub
 * Function: OnAgentInvoked
 * SubFunction: NA
 * FunctionPoints: OnAgentInvoked writes nonzero callee result into reply
 * EnvConditions: Valid data parcel and failing AgentInvoked callback
 * CaseDescription: Verify that OnAgentInvoked returns NO_ERROR and forwards the callback result to reply
 */
HWTEST_F(AgentReceiverStubTest, AgentReceiverStub_OnAgentInvoked_002, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;

    stub_->agentInvokedResult = ERR_INVALID_VALUE;
    data.WriteString("workflowAgent");

    int result = stub_->OnAgentInvoked(data, reply);

    EXPECT_EQ(result, NO_ERROR);
    EXPECT_EQ(stub_->lastWorkflowAgentId, "workflowAgent");
    EXPECT_EQ(reply.ReadInt32(), ERR_INVALID_VALUE);
}

/*
 * Feature: AgentReceiverStub
 * Function: SendData (interface method)
 * SubFunction: NA
 * FunctionPoints: SendData implementation
 * EnvConditions: NA
 * CaseDescription: Verify that SendData returns correct error code
 */
HWTEST_F(AgentReceiverStubTest, AgentReceiverStub_SendData_001, TestSize.Level1)
{
    sptr<IRemoteObject> connectorProxy = new MockRemoteObject();
    std::string testData = "test data";
    int result = stub_->SendData(connectorProxy, testData);

    EXPECT_EQ(result, 0);
}

/*
 * Feature: AgentReceiverStub
 * Function: SendData (interface method)
 * SubFunction: NA
 * FunctionPoints: SendData with null connector proxy
 * EnvConditions: NA
 * CaseDescription: Verify that SendData works with null connector proxy
 */
HWTEST_F(AgentReceiverStubTest, AgentReceiverStub_SendData_002, TestSize.Level1)
{
    sptr<IRemoteObject> nullConnectorProxy = nullptr;
    std::string testData = "test data";
    int result = stub_->SendData(nullConnectorProxy, testData);

    EXPECT_EQ(result, 0);
}

/*
 * Feature: AgentReceiverStub
 * Function: Authorize (interface method)
 * SubFunction: NA
 * FunctionPoints: Authorize implementation
 * EnvConditions: NA
 * CaseDescription: Verify that Authorize returns correct error code
 */
HWTEST_F(AgentReceiverStubTest, AgentReceiverStub_Authorize_001, TestSize.Level1)
{
    sptr<IRemoteObject> connectorProxy = new MockRemoteObject();
    std::string authData = "auth data";
    int result = stub_->Authorize(connectorProxy, authData);

    EXPECT_EQ(result, 0);
}

/*
 * Feature: AgentReceiverStub
 * Function: Authorize (interface method)
 * SubFunction: NA
 * FunctionPoints: Authorize with null connector proxy
 * EnvConditions: NA
 * CaseDescription: Verify that Authorize works with null connector proxy
 */
HWTEST_F(AgentReceiverStubTest, AgentReceiverStub_Authorize_002, TestSize.Level1)
{
    sptr<IRemoteObject> nullConnectorProxy = nullptr;
    std::string authData = "auth data";
    int result = stub_->Authorize(nullConnectorProxy, authData);

    EXPECT_EQ(result, 0);
}

} // namespace AgentRuntime
} // namespace OHOS
