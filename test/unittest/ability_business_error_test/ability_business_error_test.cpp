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
    EXPECT_EQ(GetErrorMsg(AbilityErrorCode::ERROR_CODE_MAX_CONNECTIONS_REACHED),
        "Maximum connections from the same caller have been reached. "
        "Please disconnect at least one agent extension beforehand.");
    EXPECT_EQ(GetErrorMsg(AbilityErrorCode::ERROR_CODE_AGENT_CARD_VERSION_TOO_OLD),
        "The specified agent card version is older than the current version.");
    EXPECT_EQ(GetErrorMsg(AbilityErrorCode::ERROR_CODE_AGENT_CARD_VERSION_INVALID),
        "The specified agent card version is invalid.");
    EXPECT_EQ(GetErrorMsg(AbilityErrorCode::ERROR_CODE_AGENT_CARD_DUPLICATE_REGISTER),
        "The specified agent card has already been registered. Use updateAgentCard instead.");
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

    result = GetJsErrorCodeByNativeError(OHOS::AAFwk::ERR_LOW_CODE_AGENT_ALREADY_ACTIVE);
    EXPECT_TRUE(result == AbilityErrorCode::ERROR_CODE_LOW_CODE_AGENT_ALREADY_ACTIVE);
}
}  // namespace AAFwk
}  // namespace OHOS
