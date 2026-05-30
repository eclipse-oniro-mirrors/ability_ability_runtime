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

#include "clone_for_account_util.h"
#include "bundle_mgr_helper.h"
#include "want.h"

using namespace testing::ext;

namespace OHOS {
namespace AAFwk {

class CloneForAccountUtilTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void CloneForAccountUtilTest::SetUpTestCase() {}
void CloneForAccountUtilTest::TearDownTestCase() {}

void CloneForAccountUtilTest::SetUp()
{
    AppExecFwk::BundleMgrHelper::isBundleManagerHelperNull = false;
    AppExecFwk::BundleMgrHelper::retQueryEnabledAbilityInfo = false;
    AppExecFwk::BundleMgrHelper::abilityInfoResult = AppExecFwk::AbilityInfo();
}

void CloneForAccountUtilTest::TearDown() {}

/**
 * @tc.name: ProcessAppIndex_001
 * @tc.desc: Test ProcessAppIndex returns true on success with appIndex in want
 * @tc.type: FUNC
 */
HWTEST_F(CloneForAccountUtilTest, ProcessAppIndex_001, TestSize.Level1)
{
    Want want;
    want.SetElementName("com.test.bundle", "MainAbility");
    want.SetParam(Want::PARAM_APP_CLONE_INDEX_KEY, 1);

    AppExecFwk::AbilityInfo mockAbilityInfo;
    mockAbilityInfo.appIndex = 1;
    mockAbilityInfo.applicationInfo.enabled = true;
    AppExecFwk::BundleMgrHelper::abilityInfoResult = mockAbilityInfo;
    AppExecFwk::BundleMgrHelper::retQueryEnabledAbilityInfo = true;

    EXPECT_TRUE(CloneForAccountUtil::ProcessAppIndex(want, 100));
    EXPECT_TRUE(want.HasParameter(Want::PARAM_APP_CLONE_INDEX_KEY));
    EXPECT_EQ(want.GetIntParam(Want::PARAM_APP_CLONE_INDEX_KEY, -1), 1);
}

/**
 * @tc.name: ProcessAppIndex_002
 * @tc.desc: Test ProcessAppIndex returns false when bundleMgrHelper is null
 * @tc.type: FUNC
 */
HWTEST_F(CloneForAccountUtilTest, ProcessAppIndex_002, TestSize.Level1)
{
    AppExecFwk::BundleMgrHelper::isBundleManagerHelperNull = true;

    Want want;
    want.SetElementName("com.test.bundle", "MainAbility");
    want.SetParam(Want::PARAM_APP_CLONE_INDEX_KEY, 1);

    EXPECT_FALSE(CloneForAccountUtil::ProcessAppIndex(want, 100));
    EXPECT_FALSE(want.HasParameter(Want::PARAM_APP_CLONE_INDEX_KEY));
}

/**
 * @tc.name: ProcessAppIndex_003
 * @tc.desc: Test ProcessAppIndex returns false when QueryEnabledAbilityInfo fails
 * @tc.type: FUNC
 */
HWTEST_F(CloneForAccountUtilTest, ProcessAppIndex_003, TestSize.Level1)
{
    AppExecFwk::BundleMgrHelper::retQueryEnabledAbilityInfo = false;

    Want want;
    want.SetElementName("com.test.bundle", "MainAbility");
    want.SetParam(Want::PARAM_APP_CLONE_INDEX_KEY, 1);

    EXPECT_FALSE(CloneForAccountUtil::ProcessAppIndex(want, 100));
    EXPECT_FALSE(want.HasParameter(Want::PARAM_APP_CLONE_INDEX_KEY));
}

/**
 * @tc.name: ProcessAppIndex_004
 * @tc.desc: Test ProcessAppIndex returns true with no appIndex in want
 * @tc.type: FUNC
 */
HWTEST_F(CloneForAccountUtilTest, ProcessAppIndex_004, TestSize.Level1)
{
    Want want;
    want.SetElementName("com.test.bundle", "MainAbility");

    AppExecFwk::AbilityInfo mockAbilityInfo;
    mockAbilityInfo.appIndex = 0;
    mockAbilityInfo.applicationInfo.enabled = true;
    AppExecFwk::BundleMgrHelper::abilityInfoResult = mockAbilityInfo;
    AppExecFwk::BundleMgrHelper::retQueryEnabledAbilityInfo = true;

    EXPECT_TRUE(CloneForAccountUtil::ProcessAppIndex(want, 100));
    EXPECT_TRUE(want.HasParameter(Want::PARAM_APP_CLONE_INDEX_KEY));
    EXPECT_EQ(want.GetIntParam(Want::PARAM_APP_CLONE_INDEX_KEY, -1), 0);
}

/**
 * @tc.name: ProcessAppIndex_005
 * @tc.desc: Test ProcessAppIndex resolves different appIndex than original
 * @tc.type: FUNC
 */
HWTEST_F(CloneForAccountUtilTest, ProcessAppIndex_005, TestSize.Level1)
{
    Want want;
    want.SetElementName("com.test.bundle", "MainAbility");
    want.SetParam(Want::PARAM_APP_CLONE_INDEX_KEY, 1);

    AppExecFwk::AbilityInfo mockAbilityInfo;
    mockAbilityInfo.appIndex = 2;
    mockAbilityInfo.applicationInfo.enabled = true;
    AppExecFwk::BundleMgrHelper::abilityInfoResult = mockAbilityInfo;
    AppExecFwk::BundleMgrHelper::retQueryEnabledAbilityInfo = true;

    EXPECT_TRUE(CloneForAccountUtil::ProcessAppIndex(want, 100));
    EXPECT_TRUE(want.HasParameter(Want::PARAM_APP_CLONE_INDEX_KEY));
    EXPECT_EQ(want.GetIntParam(Want::PARAM_APP_CLONE_INDEX_KEY, -1), 2);
}

/**
 * @tc.name: ProcessAppIndex_006
 * @tc.desc: Test ProcessAppIndex returns false and removes appIndex when query fails
 * @tc.type: FUNC
 */
HWTEST_F(CloneForAccountUtilTest, ProcessAppIndex_006, TestSize.Level1)
{
    AppExecFwk::BundleMgrHelper::retQueryEnabledAbilityInfo = false;

    Want want;
    want.SetElementName("com.test.bundle", "MainAbility");
    want.SetParam(Want::PARAM_APP_CLONE_INDEX_KEY, 99);

    EXPECT_FALSE(CloneForAccountUtil::ProcessAppIndex(want, 100));
    EXPECT_FALSE(want.HasParameter(Want::PARAM_APP_CLONE_INDEX_KEY));
}

}  // namespace AAFwk
}  // namespace OHOS
