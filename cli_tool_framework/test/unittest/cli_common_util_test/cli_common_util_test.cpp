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

#define private public
#include "ccm_util.h"
#undef private
#include "cli_common_mock.h"
#include "permission_util.h"

using namespace testing::ext;

namespace OHOS {
namespace CliTool {
namespace {
constexpr int32_t CUSTOM_CLI_LIMIT = 16;
constexpr Security::AccessToken::AccessTokenID TEST_TOKEN_ID = 100;
}

class CliCommonUtilTest : public testing::Test {
public:
    void SetUp() override
    {
        CliCommonMock::Reset();
        auto &ccmUtil = CcmUtil::GetInstance();
        ccmUtil.maxCliQuantity_.isLoaded = false;
        ccmUtil.maxCliQuantity_.value = DEFAULT_MAX_CLI_QUANTITY;
    }

    void TearDown() override
    {
        CliCommonMock::Reset();
    }
};

/**
 * @tc.name: CcmUtil_GetCliConcurrencyLimit_0100
 * @tc.desc: Test ccm util loads parameter once and then uses cached value
 * @tc.type: FUNC
 */
HWTEST_F(CliCommonUtilTest, CcmUtil_GetCliConcurrencyLimit_0100, TestSize.Level1)
{
    CliCommonMock::intParameterValue = CUSTOM_CLI_LIMIT;
    EXPECT_EQ(CcmUtil::GetInstance().GetCliConcurrencyLimit(), CUSTOM_CLI_LIMIT);

    CliCommonMock::intParameterValue = CUSTOM_CLI_LIMIT + 1;
    EXPECT_EQ(CcmUtil::GetInstance().GetCliConcurrencyLimit(), CUSTOM_CLI_LIMIT);
}

/**
 * @tc.name: PermissionUtil_VerifyAccessToken_0100
 * @tc.desc: Test vector permission grant and denial branches
 * @tc.type: FUNC
 */
HWTEST_F(CliCommonUtilTest, PermissionUtil_VerifyAccessToken_0100, TestSize.Level1)
{
    std::vector<std::string> permissions = {
        "ohos.permission.EXEC_CLI_TOOL",
        "ohos.permission.QUERY_CLI_TOOL",
    };

    CliCommonMock::vectorPermissionResult = Security::AccessToken::PermissionState::PERMISSION_GRANTED;
    EXPECT_TRUE(PermissionUtil::VerifyAccessToken(TEST_TOKEN_ID, permissions));

    CliCommonMock::vectorPermissionResult = Security::AccessToken::PermissionState::PERMISSION_DENIED;
    CliCommonMock::permissionStateList = {
        Security::AccessToken::TypePermissionState::PERMISSION_DENIED,
        Security::AccessToken::TypePermissionState::PERMISSION_GRANTED,
    };
    EXPECT_FALSE(PermissionUtil::VerifyAccessToken(TEST_TOKEN_ID, permissions));
}

/**
 * @tc.name: PermissionUtil_VerifyAccessToken_0200
 * @tc.desc: Test single permission grant and denial branches
 * @tc.type: FUNC
 */
HWTEST_F(CliCommonUtilTest, PermissionUtil_VerifyAccessToken_0200, TestSize.Level1)
{
    CliCommonMock::singlePermissionResult = Security::AccessToken::PermissionState::PERMISSION_GRANTED;
    EXPECT_TRUE(PermissionUtil::VerifyAccessToken(TEST_TOKEN_ID, "ohos.permission.EXEC_CLI_TOOL"));

    CliCommonMock::singlePermissionResult = Security::AccessToken::PermissionState::PERMISSION_DENIED;
    EXPECT_FALSE(PermissionUtil::VerifyAccessToken(TEST_TOKEN_ID, "ohos.permission.EXEC_CLI_TOOL"));
}
} // namespace CliTool
} // namespace OHOS
