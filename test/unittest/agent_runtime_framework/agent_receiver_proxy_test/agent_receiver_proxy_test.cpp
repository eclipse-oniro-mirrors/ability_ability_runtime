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

#include "ability_business_error.h"
#include "agent_receiver_proxy.h"
#include "agent_receiver_proxy_test_mock.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AgentRuntime {
class AgentReceiverProxyTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    sptr<MockRemoteObject> mockRemoteObject_{ nullptr };
    sptr<AgentReceiverProxy> proxy_{ nullptr };
};

void AgentReceiverProxyTest::SetUpTestCase(void)
{}
void AgentReceiverProxyTest::TearDownTestCase(void)
{}
void AgentReceiverProxyTest::SetUp()
{
    mockRemoteObject_ = new MockRemoteObject();
}
void AgentReceiverProxyTest::TearDown()
{
    mockRemoteObject_ = nullptr;
    proxy_ = nullptr;
}

/*
 * Feature: AgentReceiverProxy
 * Function: Constructor
 * SubFunction: NA
 * FunctionPoints: AgentReceiverProxy construction
 * EnvConditions: NA
 * CaseDescription: Verify that AgentReceiverProxy can be constructed successfully
 */
HWTEST_F(AgentReceiverProxyTest, AgentReceiverProxy_Constructor_001, TestSize.Level1)
{
    proxy_ = new AgentReceiverProxy(mockRemoteObject_);
    EXPECT_NE(proxy_, nullptr);
}

/*
 * Feature: AgentReceiverProxy
 * Function: SendData
 * SubFunction: NA
 * FunctionPoints: SendData with null connector proxy
 * EnvConditions: Connector proxy is null
 * CaseDescription: Verify that SendData returns error when connector proxy is null
 */
HWTEST_F(AgentReceiverProxyTest, AgentReceiverProxy_SendData_001, TestSize.Level1)
{
    proxy_ = new AgentReceiverProxy(mockRemoteObject_);

    sptr<IRemoteObject> nullConnectorProxy = nullptr;
    std::string testData = "test data";
    int32_t result = proxy_->SendData(nullConnectorProxy, testData);

    EXPECT_EQ(result, static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER));
}

/*
 * Feature: AgentReceiverProxy
 * Function: SendData
 * SubFunction: NA
 * FunctionPoints: SendData with null remote object
 * EnvConditions: Remote object is null
 * CaseDescription: Verify that SendData returns error when remote object is null
 */
HWTEST_F(AgentReceiverProxyTest, AgentReceiverProxy_SendData_002, TestSize.Level1)
{
    sptr<IRemoteObject> nullObject = nullptr;
    proxy_ = new AgentReceiverProxy(nullObject);

    sptr<IRemoteObject> connectorProxy = new MockRemoteObject();
    std::string testData = "test data";
    int32_t result = proxy_->SendData(connectorProxy, testData);

    EXPECT_EQ(result, static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER));
}

/*
 * Feature: AgentReceiverProxy
 * Function: SendData
 * SubFunction: NA
 * FunctionPoints: SendData with successful SendRequest
 * EnvConditions: Valid remote object and connector proxy
 * CaseDescription: Verify that SendData succeeds when SendRequest succeeds
 */
HWTEST_F(AgentReceiverProxyTest, AgentReceiverProxy_SendData_003, TestSize.Level1)
{
    EXPECT_CALL(*mockRemoteObject_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Return(ERR_OK));

    proxy_ = new AgentReceiverProxy(mockRemoteObject_);

    sptr<IRemoteObject> connectorProxy = new MockRemoteObject();
    std::string testData = "test data";
    int32_t result = proxy_->SendData(connectorProxy, testData);

    EXPECT_EQ(result, ERR_OK);
}

/*
 * Feature: AgentReceiverProxy
 * Function: SendData
 * SubFunction: NA
 * FunctionPoints: SendData with failed SendRequest
 * EnvConditions: SendRequest returns error
 * CaseDescription: Verify that SendData returns error when SendRequest fails
 */
HWTEST_F(AgentReceiverProxyTest, AgentReceiverProxy_SendData_004, TestSize.Level1)
{
    EXPECT_CALL(*mockRemoteObject_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Return(ERR_INVALID_OPERATION));

    proxy_ = new AgentReceiverProxy(mockRemoteObject_);

    sptr<IRemoteObject> connectorProxy = new MockRemoteObject();
    std::string testData = "test data";
    int32_t result = proxy_->SendData(connectorProxy, testData);

    EXPECT_EQ(result, static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER));
}

/*
 * Feature: AgentReceiverProxy
 * Function: SendData
 * SubFunction: NA
 * FunctionPoints: SendData with empty string
 * EnvConditions: Valid remote object and connector proxy
 * CaseDescription: Verify that SendData works with empty string
 */
HWTEST_F(AgentReceiverProxyTest, AgentReceiverProxy_SendData_005, TestSize.Level1)
{
    EXPECT_CALL(*mockRemoteObject_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Return(ERR_OK));

    proxy_ = new AgentReceiverProxy(mockRemoteObject_);

    sptr<IRemoteObject> connectorProxy = new MockRemoteObject();
    std::string emptyData = "";
    int32_t result = proxy_->SendData(connectorProxy, emptyData);

    EXPECT_EQ(result, ERR_OK);
}

/*
 * Feature: AgentReceiverProxy
 * Function: SendData
 * SubFunction: NA
 * FunctionPoints: SendData with long string
 * EnvConditions: Valid remote object and connector proxy
 * CaseDescription: Verify that SendData works with long string
 */
HWTEST_F(AgentReceiverProxyTest, AgentReceiverProxy_SendData_006, TestSize.Level1)
{
    EXPECT_CALL(*mockRemoteObject_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Return(ERR_OK));

    proxy_ = new AgentReceiverProxy(mockRemoteObject_);

    sptr<IRemoteObject> connectorProxy = new MockRemoteObject();
    std::string longData(10000, 'x');
    int32_t result = proxy_->SendData(connectorProxy, longData);

    EXPECT_EQ(result, ERR_OK);
}

/*
 * Feature: AgentReceiverProxy
 * Function: Authorize
 * SubFunction: NA
 * FunctionPoints: Authorize with null connector proxy
 * EnvConditions: Connector proxy is null
 * CaseDescription: Verify that Authorize returns error when connector proxy is null
 */
HWTEST_F(AgentReceiverProxyTest, AgentReceiverProxy_Authorize_001, TestSize.Level1)
{
    proxy_ = new AgentReceiverProxy(mockRemoteObject_);

    sptr<IRemoteObject> nullConnectorProxy = nullptr;
    std::string authData = "auth data";
    int32_t result = proxy_->Authorize(nullConnectorProxy, authData);

    EXPECT_EQ(result, static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER));
}

/*
 * Feature: AgentReceiverProxy
 * Function: Authorize
 * SubFunction: NA
 * FunctionPoints: Authorize with null remote object
 * EnvConditions: Remote object is null
 * CaseDescription: Verify that Authorize returns error when remote object is null
 */
HWTEST_F(AgentReceiverProxyTest, AgentReceiverProxy_Authorize_002, TestSize.Level1)
{
    sptr<IRemoteObject> nullObject = nullptr;
    proxy_ = new AgentReceiverProxy(nullObject);

    sptr<IRemoteObject> connectorProxy = new MockRemoteObject();
    std::string authData = "auth data";
    int32_t result = proxy_->Authorize(connectorProxy, authData);

    EXPECT_EQ(result, static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER));
}

/*
 * Feature: AgentReceiverProxy
 * Function: Authorize
 * SubFunction: NA
 * FunctionPoints: Authorize with successful SendRequest
 * EnvConditions: Valid remote object and connector proxy
 * CaseDescription: Verify that Authorize succeeds when SendRequest succeeds
 */
HWTEST_F(AgentReceiverProxyTest, AgentReceiverProxy_Authorize_003, TestSize.Level1)
{
    EXPECT_CALL(*mockRemoteObject_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Return(ERR_OK));

    proxy_ = new AgentReceiverProxy(mockRemoteObject_);

    sptr<IRemoteObject> connectorProxy = new MockRemoteObject();
    std::string authData = "auth data";
    int32_t result = proxy_->Authorize(connectorProxy, authData);

    EXPECT_EQ(result, ERR_OK);
}

/*
 * Feature: AgentReceiverProxy
 * Function: Authorize
 * SubFunction: NA
 * FunctionPoints: Authorize with failed SendRequest
 * EnvConditions: SendRequest returns error
 * CaseDescription: Verify that Authorize returns error when SendRequest fails
 */
HWTEST_F(AgentReceiverProxyTest, AgentReceiverProxy_Authorize_004, TestSize.Level1)
{
    EXPECT_CALL(*mockRemoteObject_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Return(ERR_INVALID_OPERATION));

    proxy_ = new AgentReceiverProxy(mockRemoteObject_);

    sptr<IRemoteObject> connectorProxy = new MockRemoteObject();
    std::string authData = "auth data";
    int32_t result = proxy_->Authorize(connectorProxy, authData);

    EXPECT_EQ(result, static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER));
}

/*
 * Feature: AgentReceiverProxy
 * Function: Authorize
 * SubFunction: NA
 * FunctionPoints: Authorize with empty string
 * EnvConditions: Valid remote object and connector proxy
 * CaseDescription: Verify that Authorize works with empty string
 */
HWTEST_F(AgentReceiverProxyTest, AgentReceiverProxy_Authorize_005, TestSize.Level1)
{
    EXPECT_CALL(*mockRemoteObject_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Return(ERR_OK));

    proxy_ = new AgentReceiverProxy(mockRemoteObject_);

    sptr<IRemoteObject> connectorProxy = new MockRemoteObject();
    std::string emptyData = "";
    int32_t result = proxy_->Authorize(connectorProxy, emptyData);

    EXPECT_EQ(result, ERR_OK);
}

/*
 * Feature: AgentReceiverProxy
 * Function: Authorize
 * SubFunction: NA
 * FunctionPoints: Authorize with long string
 * EnvConditions: Valid remote object and connector proxy
 * CaseDescription: Verify that Authorize works with long string
 */
HWTEST_F(AgentReceiverProxyTest, AgentReceiverProxy_Authorize_006, TestSize.Level1)
{
    EXPECT_CALL(*mockRemoteObject_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Return(ERR_OK));

    proxy_ = new AgentReceiverProxy(mockRemoteObject_);

    sptr<IRemoteObject> connectorProxy = new MockRemoteObject();
    std::string longData(10000, 'y');
    int32_t result = proxy_->Authorize(connectorProxy, longData);

    EXPECT_EQ(result, ERR_OK);
}

/*
 * Feature: AgentReceiverProxy
 * Function: AgentInvoked
 * SubFunction: NA
 * FunctionPoints: AgentInvoked with null remote object
 * EnvConditions: Remote object is null
 * CaseDescription: Verify that AgentInvoked returns error when remote object is null
 */
HWTEST_F(AgentReceiverProxyTest, AgentReceiverProxy_AgentInvoked_001, TestSize.Level1)
{
    sptr<IRemoteObject> nullObject = nullptr;
    proxy_ = new AgentReceiverProxy(nullObject);

    int32_t result = proxy_->AgentInvoked("workflowAgent");

    EXPECT_EQ(result, static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER));
}

/*
 * Feature: AgentReceiverProxy
 * Function: AgentInvoked
 * SubFunction: NA
 * FunctionPoints: AgentInvoked with successful SendRequest
 * EnvConditions: Valid remote object
 * CaseDescription: Verify that AgentInvoked succeeds when SendRequest succeeds
 */
HWTEST_F(AgentReceiverProxyTest, AgentReceiverProxy_AgentInvoked_002, TestSize.Level1)
{
    EXPECT_CALL(*mockRemoteObject_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Return(ERR_OK));

    proxy_ = new AgentReceiverProxy(mockRemoteObject_);

    int32_t result = proxy_->AgentInvoked("workflowAgent");

    EXPECT_EQ(result, ERR_OK);
}

/*
 * Feature: AgentReceiverProxy
 * Function: AgentInvoked
 * SubFunction: NA
 * FunctionPoints: AgentInvoked with failed SendRequest
 * EnvConditions: SendRequest returns error
 * CaseDescription: Verify that AgentInvoked returns error when SendRequest fails
 */
HWTEST_F(AgentReceiverProxyTest, AgentReceiverProxy_AgentInvoked_003, TestSize.Level1)
{
    EXPECT_CALL(*mockRemoteObject_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Return(ERR_INVALID_OPERATION));

    proxy_ = new AgentReceiverProxy(mockRemoteObject_);

    int32_t result = proxy_->AgentInvoked("workflowAgent");

    EXPECT_EQ(result, static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER));
}

} // namespace AgentRuntime
} // namespace OHOS
