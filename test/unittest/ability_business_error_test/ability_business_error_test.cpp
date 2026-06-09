/*
 * Copyright (c) 2022-2026 Huawei Device Co., Ltd.
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

#include "ability_business_error.h"
#include "ability_manager_errors.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AbilityRuntime {
class AbilityBusinessErrorTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void AbilityBusinessErrorTest::SetUpTestCase()
{}

void AbilityBusinessErrorTest::TearDownTestCase()
{}

void AbilityBusinessErrorTest::SetUp()
{}

void AbilityBusinessErrorTest::TearDown()
{}

/**
 * @tc.name: GetErrorMsg_0100
 * @tc.desc: GetErrorMsg_0100 Test
 * @tc.type: FUNC
 * @tc.require: issueI581SE
 */
HWTEST_F(AbilityBusinessErrorTest, GetErrorMsg_0100, TestSize.Level2)
{
    std::string result = GetErrorMsg(AbilityErrorCode::ERROR_OK);
    EXPECT_TRUE(result == "OK.");

    result = GetErrorMsg(static_cast<AbilityErrorCode>(-1000));
    EXPECT_TRUE(result == "");
}

/**
 * @tc.name: GetErrorMsg_3560000X
 * @tc.desc: Verify 356xxxx agent runtime error messages align with the API contract
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AbilityBusinessErrorTest, GetErrorMsg_3560000X, TestSize.Level2)
{
    EXPECT_EQ(GetErrorMsg(AbilityErrorCode::ERROR_CODE_AGENT_ID_NOT_EXIST),
        "The specified agentId does not exist.");
    EXPECT_EQ(GetErrorMsg(AbilityErrorCode::ERROR_CODE_AGENT_CARD_LIST_OUT_OF_RANGE),
        "The number of AgentCards in the bundle reaches the limit.");
    EXPECT_EQ(GetErrorMsg(AbilityErrorCode::ERROR_CODE_MAX_CONNECTIONS_REACHED),
        "Maximum connections from the same caller have been reached. "
        "Please disconnect at least one agent extension beforehand.");
    EXPECT_EQ(GetErrorMsg(AbilityErrorCode::ERROR_CODE_AGENT_CARD_VERSION_TOO_OLD),
        "The specified AgentCard version is older than the current version.");
    EXPECT_EQ(GetErrorMsg(AbilityErrorCode::ERROR_CODE_AGENT_CARD_VERSION_INVALID),
        "The specified AgentCard version is invalid.");
    EXPECT_EQ(GetErrorMsg(AbilityErrorCode::ERROR_CODE_AGENT_CARD_DUPLICATE_REGISTER),
        "The specified AgentCard has already been registered. Use updateAgentCard instead.");
    EXPECT_EQ(GetErrorMsg(AbilityErrorCode::ERROR_CODE_LOW_CODE_AGENT_ALREADY_ACTIVE),
        "The specified LOW_CODE agent is already active and is not yet completed.");
}

/**
 * @tc.name: GetJsErrorCodeByNativeError_0100
 * @tc.desc: GetJsErrorCodeByNativeError_0100 Test
 * @tc.type: FUNC
 * @tc.require: issueI581RO
 */
HWTEST_F(AbilityBusinessErrorTest, GetJsErrorCodeByNativeError_0100, TestSize.Level2)
{
    AbilityErrorCode result = GetJsErrorCodeByNativeError(0);
    EXPECT_TRUE(result == AbilityErrorCode::ERROR_OK);

    result = GetJsErrorCodeByNativeError(-1000);
    EXPECT_TRUE(result == AbilityErrorCode::ERROR_CODE_INNER);

    result = GetJsErrorCodeByNativeError(AAFwk::ERR_MAX_AGENT_CONNECTIONS_REACHED);
    EXPECT_TRUE(result == AbilityErrorCode::ERROR_CODE_MAX_CONNECTIONS_REACHED);

    result = GetJsErrorCodeByNativeError(OHOS::AAFwk::ERR_AGENT_CARD_VERSION_TOO_OLD);
    EXPECT_TRUE(result == AbilityErrorCode::ERROR_CODE_AGENT_CARD_VERSION_TOO_OLD);

    result = GetJsErrorCodeByNativeError(OHOS::AAFwk::ERR_INVALID_AGENT_CARD_VERSION);
    EXPECT_TRUE(result == AbilityErrorCode::ERROR_CODE_AGENT_CARD_VERSION_INVALID);

    result = GetJsErrorCodeByNativeError(OHOS::AAFwk::ERR_AGENT_CARD_DUPLICATE_REGISTER);
    EXPECT_TRUE(result == AbilityErrorCode::ERROR_CODE_AGENT_CARD_DUPLICATE_REGISTER);

    result = GetJsErrorCodeByNativeError(OHOS::AAFwk::ERR_AGENT_CARD_LIST_OUT_OF_RANGE);
    EXPECT_TRUE(result == AbilityErrorCode::ERROR_CODE_AGENT_CARD_LIST_OUT_OF_RANGE);

    result = GetJsErrorCodeByNativeError(OHOS::AAFwk::ERR_LOW_CODE_AGENT_ALREADY_ACTIVE);
    EXPECT_TRUE(result == AbilityErrorCode::ERROR_CODE_LOW_CODE_AGENT_ALREADY_ACTIVE);
}

/**
 * @tc.name: GetErrorMsgByNativeError_0100
 * @tc.desc: Verify framework-side native error conversion keeps mapped messages and supports scoped inner messages.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityBusinessErrorTest, GetErrorMsgByNativeError_0100, TestSize.Level2)
{
    const std::string innerErrMsg = "Internal error. Failed to start the ability. Try again later.";
    EXPECT_EQ(GetErrorMsgByNativeError(0), "OK.");
    EXPECT_EQ(GetErrorMsgByNativeError(-1000), "Internal error.");
    EXPECT_EQ(GetErrorMsgByNativeError(-1000, innerErrMsg), innerErrMsg);
    EXPECT_EQ(GetErrorMsgByNativeError(AAFwk::GET_ABILITY_SERVICE_FAILED),
        "Internal error. Service unavailable. Try again later.");
    EXPECT_EQ(GetErrorMsgByNativeError(AAFwk::CREATE_MISSION_STACK_FAILED),
        "Internal error. Operation failed. Try again later.");
    EXPECT_EQ(GetErrorMsgByNativeError(AAFwk::LOAD_ABILITY_TIMEOUT),
        "Internal error. Operation timed out. Try again later.");
    EXPECT_EQ(GetErrorMsgByNativeError(AAFwk::ERR_NATIVE_IPC_PARCEL_FAILED),
        "Internal error. IPC failed. Try again later.");
    EXPECT_EQ(GetErrorMsgByNativeError(AAFwk::CONNECTION_NOT_EXIST, innerErrMsg),
        "Internal error. The service connection does not exist. Use a connection ID returned by "
        "connectServiceExtensionAbility.");
    EXPECT_EQ(GetErrorMsgByNativeError(AAFwk::ERR_MAX_AGENT_CONNECTIONS_REACHED, innerErrMsg),
        "Maximum connections from the same caller have been reached. Please disconnect at least one agent extension "
        "beforehand.");
    EXPECT_EQ(GetErrorMsgByNativeError(ERR_PERMISSION_DENIED, innerErrMsg, "ohos.permission.TEST"),
        "The application does not have permission to call the interface. permission:ohos.permission.TEST");
    EXPECT_EQ(GetErrorMsgByNativeError(static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER), innerErrMsg),
        innerErrMsg);
}

/**
 * @tc.name: GetInnerErrorMsg_0100
 * @tc.desc: Verify direct framework-side 16000050 scenes use centralized developer-facing messages.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityBusinessErrorTest, GetInnerErrorMsg_0100, TestSize.Level2)
{
    EXPECT_EQ(GetInnerErrorMsg(AbilityInnerErrorMsg::SERVICE_UNAVAILABLE),
        "Internal error. Service unavailable. Try again later.");
    EXPECT_EQ(GetInnerErrorMsg(AbilityInnerErrorMsg::OPERATION_FAILED),
        "Internal error. Operation failed. Try again later.");
    EXPECT_EQ(GetInnerErrorMsg(AbilityInnerErrorMsg::RESTORE_WINDOW_STAGE_FAILED),
        "Internal error. Failed to restore the window stage. Check the local storage object and try again.");
    EXPECT_EQ(GetInnerErrorMsg(AbilityInnerErrorMsg::QUERY_ATOMIC_SERVICE_STARTUP_RULE_FAILED),
        "Internal error. Failed to query the atomic service startup rule. Try again later.");
    EXPECT_EQ(GetInnerErrorMsg(AbilityInnerErrorMsg::RESTART_SELF_ATOMIC_SERVICE_FAILED),
        "Internal error. Failed to restart the current atomic service. Try again later.");
    EXPECT_EQ(GetInnerErrorMsg(AbilityInnerErrorMsg::CONNECT_AGENT_EXTENSION_FAILED),
        "Internal error. Failed to connect to the agent extension ability. Verify the target and try again.");
    EXPECT_EQ(GetInnerErrorMsg(AbilityInnerErrorMsg::AGENT_EXTENSION_CONNECTION_ENDED),
        "Internal error. The agent extension connection ended before it was ready. Connect again.");
    EXPECT_EQ(GetInnerErrorMsg(AbilityInnerErrorMsg::TRANSFER_EXTENSION_DATA_FAILED),
        "Internal error. Failed to transfer extension data to the window. Try again later.");
    EXPECT_EQ(GetInnerErrorMsg(AbilityInnerErrorMsg::UI_WINDOW_NULL),
        "Internal error. The UI window is not available. Try again later.");
    EXPECT_EQ(GetInnerErrorMsg(AbilityInnerErrorMsg::RELOAD_IN_MODAL_RESULT_NULL),
        "Internal error. Failed to create reload result. Try again later.");
    EXPECT_EQ(GetInnerErrorMsg(AbilityInnerErrorMsg::OBSERVER_CREATE_FAILED),
        "Internal error. Failed to create the observer.");
    EXPECT_EQ(GetInnerErrorMsg(AbilityInnerErrorMsg::OBSERVER_NOT_EXIST),
        "Internal error. The observer does not exist. Register the observer first.");
    EXPECT_EQ(GetInnerErrorMsg(AbilityInnerErrorMsg::RESTART_APP_FAILED),
        "Internal error. Failed to restart the application. Try again later.");
    EXPECT_EQ(GetInnerErrorMsg(AbilityInnerErrorMsg::CREATE_PROCESS_INFO_ARRAY_FAILED),
        "Internal error. Failed to create the process information array.");
    EXPECT_EQ(GetInnerErrorMsg(AbilityInnerErrorMsg::GET_PROCESS_INFO_FAILED),
        "Internal error. Failed to get process information. Try again later.");
    EXPECT_EQ(GetInnerErrorMsg(AbilityInnerErrorMsg::SET_PROCESS_CACHE_FAILED),
        "Internal error. Failed to set the process cache state. Try again later.");
    EXPECT_EQ(GetInnerErrorMsg(AbilityInnerErrorMsg::UI_ABILITY_OBJ_NULL),
        "Internal error. The UIAbility object is not available. Verify the ability is active and try again.");
}

/**
 * @tc.name: GetInnerErrorMsg_0200
 * @tc.desc: Verify all new AbilityInnerErrorMsg enum values return specific non-generic messages.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityBusinessErrorTest, GetInnerErrorMsg_0200, TestSize.Level2)
{
    EXPECT_EQ(GetInnerErrorMsg(AbilityInnerErrorMsg::CONNECTION_NOT_FOUND),
        "Internal error. The service connection does not exist. Check the connection ID.");
    EXPECT_EQ(GetInnerErrorMsg(AbilityInnerErrorMsg::SEND_DATA_FAILED),
        "Internal error. Failed to send data to the extension.");
    EXPECT_EQ(GetInnerErrorMsg(AbilityInnerErrorMsg::LOAD_CONTENT_FAILED),
        "Internal error. Failed to load the UI content.");
    EXPECT_EQ(GetInnerErrorMsg(AbilityInnerErrorMsg::SET_WINDOW_BACKGROUND_COLOR_FAILED),
        "Internal error. Failed to set the window background color.");
    EXPECT_EQ(GetInnerErrorMsg(AbilityInnerErrorMsg::SET_WINDOW_PRIVACY_MODE_FAILED),
        "Internal error. Failed to set the window privacy mode.");
    EXPECT_EQ(GetInnerErrorMsg(AbilityInnerErrorMsg::CREATE_MODAL_UI_EXTENSION_FAILED),
        "Internal error. Failed to create the modal UI extension. ");
    EXPECT_EQ(GetInnerErrorMsg(AbilityInnerErrorMsg::TRANSFER_ABILITY_RESULT_FAILED),
        "Internal error. Failed to transfer the ability result. ");
    EXPECT_EQ(GetInnerErrorMsg(AbilityInnerErrorMsg::INVALID_SESSION),
        "Internal error. The UIExtension content session is invalid or has been destroyed. "
        "Ensure the session is active before calling this method.");
    EXPECT_EQ(GetInnerErrorMsg(AbilityInnerErrorMsg::UI_SERVICE_PROXY_NOT_FOUND),
        "Internal error. The UIService is not available.");
}

/**
 * @tc.desc: Verify agentManager operation policy selects centralized messages without replacing mapped errors.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityBusinessErrorTest, GetAgentManagerErrorMsg_0100, TestSize.Level2)
{
    EXPECT_EQ(GetAgentManagerErrorMsg(-1000, AgentManagerErrorOperation::READ_AGENT_CARDS),
        GetInnerErrorMsg(AbilityInnerErrorMsg::OPERATION_FAILED));
    EXPECT_EQ(GetAgentManagerErrorMsg(AAFwk::ERR_NULL_AGENT_MGR_PROXY,
        AgentManagerErrorOperation::READ_AGENT_CARDS),
        GetInnerErrorMsg(AbilityInnerErrorMsg::SERVICE_UNAVAILABLE));
    EXPECT_EQ(GetAgentManagerErrorMsg(AAFwk::CONNECTION_NOT_EXIST,
        AgentManagerErrorOperation::DISCONNECT_AGENT_EXTENSION),
        "Internal error. The agent extension connection does not exist. "
        "Use an AgentProxy returned by connectAgentExtensionAbility.");
    EXPECT_EQ(GetAgentManagerErrorMsg(AAFwk::ERR_MAX_AGENT_CONNECTIONS_REACHED,
        AgentManagerErrorOperation::CONNECT_AGENT_EXTENSION),
        GetErrorMsg(AbilityErrorCode::ERROR_CODE_MAX_CONNECTIONS_REACHED));
}
}  // namespace AAFwk
}  // namespace OHOS
