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

#include <gtest/gtest.h>

#include "js_agent_extension.h"
#include "mock_js_agent_extension.h"

#define private public
#include "js_agent_extension_stub_impl.h"
#undef private

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AgentRuntime {

class JsAgentExtensionStubImplTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    std::shared_ptr<MockJsAgentExtension> mockExtension_;
    std::shared_ptr<JsAgentExtensionStubImpl> stubImpl_;
};

void JsAgentExtensionStubImplTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "JsAgentExtensionStubImplTest::SetUpTestCase";
}

void JsAgentExtensionStubImplTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "JsAgentExtensionStubImplTest::TearDownTestCase";
}

void JsAgentExtensionStubImplTest::SetUp()
{
    GTEST_LOG_(INFO) << "JsAgentExtensionStubImplTest::SetUp";
    mockExtension_ = std::make_shared<MockJsAgentExtension>();
    std::weak_ptr<JsAgentExtension> weakExt = mockExtension_;
    stubImpl_ = std::make_shared<JsAgentExtensionStubImpl>(weakExt);
}

void JsAgentExtensionStubImplTest::TearDown()
{
    GTEST_LOG_(INFO) << "JsAgentExtensionStubImplTest::TearDown";
    stubImpl_.reset();
    mockExtension_.reset();
}

/*
 * Feature: JsAgentExtensionStubImpl
 * Function: SendData
 * SubFunction: NA
 * FunctionPoints: SendData with valid extension and data
 * EnvConditions: Extension is alive
 * CaseDescription: Verify that SendData forwards call to extension successfully
 */
HWTEST_F(JsAgentExtensionStubImplTest, JsAgentExtensionStubImpl_SendData_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "JsAgentExtensionStubImpl_SendData_001 start";
    // Arrange
    sptr<IRemoteObject> hostProxy = nullptr;
    std::string testData = "test data";

    // Act
    int32_t result = stubImpl_->SendData(hostProxy, testData);

    // Assert
    EXPECT_EQ(result, static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_OK));
    GTEST_LOG_(INFO) << "JsAgentExtensionStubImpl_SendData_001 end";
}

/*
 * Feature: JsAgentExtensionStubImpl
 * Function: SendData
 * SubFunction: NA
 * FunctionPoints: SendData with expired extension
 * EnvConditions: Extension weak_ptr is expired
 * CaseDescription: Verify that SendData returns ERROR_CODE_INNER when extension is expired
 */
HWTEST_F(JsAgentExtensionStubImplTest, JsAgentExtensionStubImpl_SendData_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "JsAgentExtensionStubImpl_SendData_002 start";
    // Arrange
    sptr<IRemoteObject> hostProxy = nullptr;
    std::string testData = "test data";

    // Reset extension to simulate expiry
    mockExtension_.reset();

    // Act
    int32_t result = stubImpl_->SendData(hostProxy, testData);

    // Assert
    EXPECT_EQ(result, static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER));
    GTEST_LOG_(INFO) << "JsAgentExtensionStubImpl_SendData_002 end";
}

/*
 * Feature: JsAgentExtensionStubImpl
 * Function: SendData
 * SubFunction: NA
 * FunctionPoints: SendData with empty string
 * EnvConditions: Extension is alive
 * CaseDescription: Verify that SendData works with empty string
 */
HWTEST_F(JsAgentExtensionStubImplTest, JsAgentExtensionStubImpl_SendData_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "JsAgentExtensionStubImpl_SendData_003 start";
    // Arrange
    sptr<IRemoteObject> hostProxy = nullptr;
    std::string emptyData = "";

    // Act
    int32_t result = stubImpl_->SendData(hostProxy, emptyData);

    // Assert
    EXPECT_EQ(result, static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_OK));
    GTEST_LOG_(INFO) << "JsAgentExtensionStubImpl_SendData_003 end";
}

/*
 * Feature: JsAgentExtensionStubImpl
 * Function: SendData
 * SubFunction: NA
 * FunctionPoints: SendData with long string
 * EnvConditions: Extension is alive
 * CaseDescription: Verify that SendData works with long string
 */
HWTEST_F(JsAgentExtensionStubImplTest, JsAgentExtensionStubImpl_SendData_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "JsAgentExtensionStubImpl_SendData_004 start";
    // Arrange
    sptr<IRemoteObject> hostProxy = nullptr;
    std::string longData(10000, 'x');

    // Act
    int32_t result = stubImpl_->SendData(hostProxy, longData);

    // Assert
    EXPECT_EQ(result, static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_OK));
    GTEST_LOG_(INFO) << "JsAgentExtensionStubImpl_SendData_004 end";
}

/*
 * Feature: JsAgentExtensionStubImpl
 * Function: Authorize
 * SubFunction: NA
 * FunctionPoints: Authorize with valid extension and data
 * EnvConditions: Extension is alive
 * CaseDescription: Verify that Authorize forwards call to extension successfully
 */
HWTEST_F(JsAgentExtensionStubImplTest, JsAgentExtensionStubImpl_Authorize_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "JsAgentExtensionStubImpl_Authorize_001 start";
    // Arrange
    sptr<IRemoteObject> hostProxy = nullptr;
    std::string authData = "auth data";

    // Act
    int32_t result = stubImpl_->Authorize(hostProxy, authData);

    // Assert
    EXPECT_EQ(result, static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_OK));
    GTEST_LOG_(INFO) << "JsAgentExtensionStubImpl_Authorize_001 end";
}

/*
 * Feature: JsAgentExtensionStubImpl
 * Function: Authorize
 * SubFunction: NA
 * FunctionPoints: Authorize with expired extension
 * EnvConditions: Extension weak_ptr is expired
 * CaseDescription: Verify that Authorize returns ERROR_CODE_INNER when extension is expired
 */
HWTEST_F(JsAgentExtensionStubImplTest, JsAgentExtensionStubImpl_Authorize_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "JsAgentExtensionStubImpl_Authorize_002 start";
    // Arrange
    sptr<IRemoteObject> hostProxy = nullptr;
    std::string authData = "auth data";

    // Reset extension to simulate expiry
    mockExtension_.reset();

    // Act
    int32_t result = stubImpl_->Authorize(hostProxy, authData);

    // Assert
    EXPECT_EQ(result, static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER));
    GTEST_LOG_(INFO) << "JsAgentExtensionStubImpl_Authorize_002 end";
}

/*
 * Feature: JsAgentExtensionStubImpl
 * Function: Authorize
 * SubFunction: NA
 * FunctionPoints: Authorize with empty string
 * EnvConditions: Extension is alive
 * CaseDescription: Verify that Authorize works with empty string
 */
HWTEST_F(JsAgentExtensionStubImplTest, JsAgentExtensionStubImpl_Authorize_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "JsAgentExtensionStubImpl_Authorize_003 start";
    // Arrange
    sptr<IRemoteObject> hostProxy = nullptr;
    std::string emptyData = "";

    // Act
    int32_t result = stubImpl_->Authorize(hostProxy, emptyData);

    // Assert
    EXPECT_EQ(result, static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_OK));
    GTEST_LOG_(INFO) << "JsAgentExtensionStubImpl_Authorize_003 end";
}

/*
 * Feature: JsAgentExtensionStubImpl
 * Function: Authorize
 * SubFunction: NA
 * FunctionPoints: Authorize with long string
 * EnvConditions: Extension is alive
 * CaseDescription: Verify that Authorize works with long string
 */
HWTEST_F(JsAgentExtensionStubImplTest, JsAgentExtensionStubImpl_Authorize_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "JsAgentExtensionStubImpl_Authorize_004 start";
    // Arrange
    sptr<IRemoteObject> hostProxy = nullptr;
    std::string longData(10000, 'y');

    // Act
    int32_t result = stubImpl_->Authorize(hostProxy, longData);

    // Assert
    EXPECT_EQ(result, static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_OK));
    GTEST_LOG_(INFO) << "JsAgentExtensionStubImpl_Authorize_004 end";
}

/*
 * Feature: JsAgentExtensionStubImpl
 * Function: AgentInvoked
 * SubFunction: NA
 * FunctionPoints: AgentInvoked with valid extension and agent id
 * EnvConditions: Extension is alive
 * CaseDescription: Verify that AgentInvoked forwards call to extension successfully
 */
HWTEST_F(JsAgentExtensionStubImplTest, JsAgentExtensionStubImpl_AgentInvoked_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "JsAgentExtensionStubImpl_AgentInvoked_001 start";
    std::string agentId = "workflowAgent";

    int32_t result = stubImpl_->AgentInvoked(agentId);

    EXPECT_EQ(result, static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_OK));
    GTEST_LOG_(INFO) << "JsAgentExtensionStubImpl_AgentInvoked_001 end";
}

/*
 * Feature: JsAgentExtensionStubImpl
 * Function: AgentInvoked
 * SubFunction: NA
 * FunctionPoints: AgentInvoked with expired extension
 * EnvConditions: Extension weak_ptr is expired
 * CaseDescription: Verify that AgentInvoked returns ERROR_CODE_INNER when extension is expired
 */
HWTEST_F(JsAgentExtensionStubImplTest, JsAgentExtensionStubImpl_AgentInvoked_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "JsAgentExtensionStubImpl_AgentInvoked_002 start";
    std::string agentId = "workflowAgent";

    mockExtension_.reset();

    int32_t result = stubImpl_->AgentInvoked(agentId);

    EXPECT_EQ(result, static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER));
    GTEST_LOG_(INFO) << "JsAgentExtensionStubImpl_AgentInvoked_002 end";
}

} // namespace AgentRuntime
} // namespace OHOS
