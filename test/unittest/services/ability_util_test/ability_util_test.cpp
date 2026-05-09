/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "ability_util.h"
#include "int_wrapper.h"
#include "mock_my_flag.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AAFwk {
class AbilityUtilTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void AbilityUtilTest::SetUpTestCase()
{}

void AbilityUtilTest::TearDownTestCase()
{}

void AbilityUtilTest::SetUp()
{}

void AbilityUtilTest::TearDown()
{}

#ifdef WITH_DLP
/**
 * @tc.name: HandleDlpApp_0100
 * @tc.desc: HandleDlpApp Test
 * @tc.type: FUNC
 * @tc.require: issueI581T3
 */
HWTEST_F(AbilityUtilTest, HandleDlpApp_0100, TestSize.Level0)
{
    Want want;
    bool result = AbilityUtil::HandleDlpApp(want);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: HandleDlpApp_0200
 * @tc.desc: HandleDlpApp Test
 * @tc.type: FUNC
 * @tc.require: issueI581T3
 */
HWTEST_F(AbilityUtilTest, HandleDlpApp_0200, TestSize.Level0)
{
    Want want;
    want.SetParam(AbilityUtil::DLP_PARAMS_SANDBOX, true);
    bool result = AbilityUtil::HandleDlpApp(want);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: HandleDlpApp_0300
 * @tc.desc: HandleDlpApp Test
 * @tc.type: FUNC
 * @tc.require: issueI581T3
 */
HWTEST_F(AbilityUtilTest, HandleDlpApp_0300, TestSize.Level0)
{
    Want want;
    want.SetParam(AbilityUtil::DLP_PARAMS_SANDBOX, true);
    want.SetElementName("com.ohos.test", "MainAbility");
    bool result = AbilityUtil::HandleDlpApp(want);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: HandleDlpApp_0400
 * @tc.desc: HandleDlpApp Test
 * @tc.type: FUNC
 * @tc.require: issueI5825N
 */
HWTEST_F(AbilityUtilTest, HandleDlpApp_0400, TestSize.Level0)
{
    Want want;
    want.SetParam(AbilityUtil::DLP_PARAMS_SANDBOX, true);
    want.SetElementName("", "");
    bool result = AbilityUtil::HandleDlpApp(want);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: HandleDlpApp_0500
 * @tc.desc: HandleDlpApp Test
 * @tc.type: FUNC
 * @tc.require: issueI5826I
 */
HWTEST_F(AbilityUtilTest, HandleDlpApp_0500, TestSize.Level0)
{
    Want want;
    want.SetParam(AbilityUtil::DLP_PARAMS_SANDBOX, true);
    want.SetElementName("", "MainAbility");
    bool result = AbilityUtil::HandleDlpApp(want);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: HandleDlpApp_0600
 * @tc.desc: HandleDlpApp Test
 * @tc.type: FUNC
 * @tc.require: issueI583ZA
 */
HWTEST_F(AbilityUtilTest, HandleDlpApp_0600, TestSize.Level0)
{
    Want want;
    want.SetParam(AbilityUtil::DLP_PARAMS_SANDBOX, true);
    want.SetElementName("com.ohos.test", "");
    bool result = AbilityUtil::HandleDlpApp(want);
    EXPECT_FALSE(result);
}
#endif // WITH_DLP

/**
 * @tc.name: ProcessWindowMode_0100
 * @tc.desc: ProcessWindowMode Test
 * @tc.type: FUNC
 * @tc.require: issueI9D3V0
 */
HWTEST_F(AbilityUtilTest, ProcessWindowMode_0100, TestSize.Level1)
{
    Want want;
    want.SetParam(Want::PARAM_RESV_WINDOW_MODE, 0);
    MyFlag::flag_ = 1;
    uint32_t accessTokenId = 0;
    int32_t windowMode = 0;
    AbilityUtil::ProcessWindowMode(want, accessTokenId, windowMode);
    int32_t getWindowMode = Integer::Unbox(IInteger::Query(want.GetParams().GetParam(Want::PARAM_RESV_WINDOW_MODE)));
    EXPECT_EQ(windowMode, getWindowMode);
}

/**
 * @tc.name: ProcessWindowMode_0200
 * @tc.desc: ProcessWindowMode Test
 * @tc.type: FUNC
 * @tc.require: issueI9D3V0
 */
HWTEST_F(AbilityUtilTest, ProcessWindowMode_0200, TestSize.Level1)
{
    Want want;
    want.SetParam(Want::PARAM_RESV_WINDOW_MODE, 0);
    MyFlag::flag_ = 0;
    uint32_t accessTokenId = 0;
    int32_t windowMode = 100;
    AbilityUtil::ProcessWindowMode(want, accessTokenId, windowMode);
    int32_t getWindowMode = Integer::Unbox(IInteger::Query(want.GetParams().GetParam(Want::PARAM_RESV_WINDOW_MODE)));
    EXPECT_EQ(windowMode, getWindowMode);
}

/**
 * @tc.name: ProcessWindowMode_0300
 * @tc.desc: ProcessWindowMode Test
 * @tc.type: FUNC
 * @tc.require: issueI9D3V0
 */
HWTEST_F(AbilityUtilTest, ProcessWindowMode_0300, TestSize.Level1)
{
    Want want;
    want.SetParam(Want::PARAM_RESV_WINDOW_MODE, 0);
    MyFlag::flag_ = 0;
    uint32_t accessTokenId = 0;
    int32_t windowMode = 101;
    AbilityUtil::ProcessWindowMode(want, accessTokenId, windowMode);
    int32_t getWindowMode = Integer::Unbox(IInteger::Query(want.GetParams().GetParam(Want::PARAM_RESV_WINDOW_MODE)));
    EXPECT_EQ(windowMode, getWindowMode);
}

/**
 * @tc.name: ProcessWindowMode_0400
 * @tc.desc: ProcessWindowMode Test
 * @tc.type: FUNC
 * @tc.require: issueI9D3V0
 */
HWTEST_F(AbilityUtilTest, ProcessWindowMode_0400, TestSize.Level1)
{
    Want want;
    want.SetParam(Want::PARAM_RESV_WINDOW_MODE, 0);
    MyFlag::flag_ = 0;
    uint32_t accessTokenId = 0;
    int32_t windowMode = AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_SPLIT;
    AbilityUtil::ProcessWindowMode(want, accessTokenId, windowMode);
    int32_t getWindowMode = Integer::Unbox(IInteger::Query(want.GetParams().GetParam(Want::PARAM_RESV_WINDOW_MODE)));
    EXPECT_EQ(windowMode, getWindowMode);
}
}  // namespace AAFwk
}  // namespace OHOS
