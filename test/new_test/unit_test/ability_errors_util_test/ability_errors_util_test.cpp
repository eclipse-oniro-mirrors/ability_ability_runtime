/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "ability_errors_util.h"

using namespace testing;
using namespace testing::ext;


namespace OHOS {
namespace AppExecFwk {
class UserControllerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void UserControllerTest::SetUpTestCase()
{}

void UserControllerTest::TearDownTestCase()
{}

void UserControllerTest::SetUp()
{}

void UserControllerTest::TearDown()
{}

/**
 * @tc.name: ConvertToOriginErrorCode_Test_001
 * @tc.desc: test refiniment ERR_INVALID_VALUE.
 * @tc.type: FUNC
 */
HWTEST_F(UserControllerTest, ConvertToOriginErrorCode_Test_001, TestSize.Level1)
{
    auto oriRet = AAFWK::AbilityErrorUtil::ConvertToOriginErrorCode(ERR_CONNECT_MANAGER_NULL_ABILITY_RECORD);
    EXPECT_EQ(oriRet, ERR_INVALID_VALUE);
}

/**
 * @tc.name: ConvertToOriginErrorCode_Test_002
 * @tc.desc: test refiniment INNER_ERR.
 * @tc.type: FUNC
 */
HWTEST_F(UserControllerTest, ConvertToOriginErrorCode_Test_002, TestSize.Level1)
{
    auto oriRet = AAFWK::AbilityErrorUtil::ConvertToOriginErrorCode(ERR_APP_MGR_TERMINATTE_ABILITY_FAILED);
    EXPECT_EQ(oriRet, INNER_ERR);
}

/**
 * @tc.name: ConvertToOriginErrorCode_Test_003
 * @tc.desc: test refiniment RESOLVE_ABILITY_ERR.
 * @tc.type: FUNC
 */
HWTEST_F(UserControllerTest, ConvertToOriginErrorCode_Test_003, TestSize.Level1)
{
    auto oriRet = AAFwk::AbilityErrorUtil::ConvertToOriginErrorCode(ERR_CHECK_PLUGIN_NULL_RECORD);
    EXPECT_EQ(oriRet, RESOLVE_ABILITY_ERR);
}

/**
 * @tc.name: ConvertToOriginErrorCode_Test_004
 * @tc.desc: test refiniment ERR_INVALID_CALLER.
 * @tc.type: FUNC
 */
HWTEST_F(UserControllerTest, ConvertToOriginErrorCode_Test_004, TestSize.Level1)
{
    auto oriRet = AAFWK::AbilityErrorUtil::ConvertToOriginErrorCode(ERR_IS_NOT_SPECIFIED_SA);
    EXPECT_EQ(oriRet, ERR_INVALID_CALLER);
}

/**
 * @tc.name: ConvertToOriginErrorCode_Test_005
 * @tc.desc: test refiniment ERR_OK.
 * @tc.type: FUNC
 */
HWTEST_F(UserControllerTest, ConvertToOriginErrorCode_Test_005, TestSize.Level1)
{
    auto oriRet = AAFWK::AbilityErrorUtil::ConvertToOriginErrorCode(ERR_OK);
    EXPECT_EQ(oriRet, ERR_OK);
}

/**
 * @tc.name: ConvertToOriginErrorCode_FreqLimit_001
 * @tc.desc: test ERR_FREQ_START_ABILITY is preserved without refinement.
 * @tc.type: FUNC
 */
HWTEST_F(UserControllerTest, ConvertToOriginErrorCode_FreqLimit_001, TestSize.Level1)
{
    auto oriRet = AAFWK::AbilityErrorUtil::ConvertToOriginErrorCode(ERR_FREQ_START_ABILITY);
    EXPECT_EQ(oriRet, ERR_FREQ_START_ABILITY);
}

/**
 * @tc.name: ConvertToOriginErrorCode_FreqLimit_002
 * @tc.desc: test other codes in INVALID_VALUE range are still refined.
 * @tc.type: FUNC
 */
HWTEST_F(UserControllerTest, ConvertToOriginErrorCode_FreqLimit_002, TestSize.Level1)
{
    auto oriRet = AAFWK::AbilityErrorUtil::ConvertToOriginErrorCode(ERR_CONNECT_MANAGER_NULL_ABILITY_RECORD);
    EXPECT_EQ(oriRet, ERR_INVALID_VALUE);
}
} // namespace AppExecFwk
} // namespace OHOS