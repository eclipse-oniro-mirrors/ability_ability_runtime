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

#define private public
#include "quick_fix_manager_service_ability.h"
#undef private

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AAFwk {
extern void MockInitState(bool state);
extern void MockGetInstanceState(bool state);
extern void ResetMockQuickFixManagerServiceState();

class QuickFixManagerServiceAbilityTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void QuickFixManagerServiceAbilityTest::SetUpTestCase()
{}

void QuickFixManagerServiceAbilityTest::TearDownTestCase()
{}

void QuickFixManagerServiceAbilityTest::SetUp()
{}

void QuickFixManagerServiceAbilityTest::TearDown()
{
    ResetMockQuickFixManagerServiceState();
}

/**
 * @tc.name: OnStart_0100
 * @tc.desc: OnStart
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixManagerServiceAbilityTest, OnStart_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "QuickFixManagerServiceAbilityTest, OnStart_0100, TestSize.Level1";
    MockGetInstanceState(true);
    MockInitState(true);
    QuickFixManagerServiceAbility ability(0, true);
    ability.OnStart();
    EXPECT_NE(ability.publishObj_, nullptr);
}

/**
 * @tc.name: OnStart_0200
 * @tc.desc: OnStart
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixManagerServiceAbilityTest, OnStart_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "QuickFixManagerServiceAbilityTest, OnStart_0200, TestSize.Level1";
    MockGetInstanceState(true);
    MockInitState(true);
    QuickFixManagerServiceAbility ability(0, true);
    ability.service_ = QuickFixManagerService::GetInstance();
    ability.OnStart();
    EXPECT_EQ(ability.publishObj_, nullptr);
}

/**
 * @tc.name: OnStart_0300
 * @tc.desc: OnStart
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixManagerServiceAbilityTest, OnStart_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "QuickFixManagerServiceAbilityTest, OnStart_0300, TestSize.Level1";
    MockGetInstanceState(false);
    MockInitState(true);
    QuickFixManagerServiceAbility ability(0, true);
    ability.OnStart();
    EXPECT_EQ(ability.publishObj_, nullptr);
}

/**
 * @tc.name: OnStart_0400
 * @tc.desc: OnStart
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixManagerServiceAbilityTest, OnStart_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "QuickFixManagerServiceAbilityTest, OnStart_0400, TestSize.Level1";
    MockGetInstanceState(true);
    MockInitState(false);
    QuickFixManagerServiceAbility ability(0, true);
    ability.OnStart();
    EXPECT_EQ(ability.publishObj_, nullptr);
}

/**
 * @tc.name: OnStop_0100
 * @tc.desc: OnStop
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixManagerServiceAbilityTest, OnStop_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO)
        << "QuickFixManagerServiceAbilityTest, OnStop_0100, TestSize.Level1";
    QuickFixManagerServiceAbility ability(0, true);
    ability.OnStop();
    EXPECT_EQ(ability.service_, nullptr);
}
} // namespace AAFwk
} // namespace OHOS