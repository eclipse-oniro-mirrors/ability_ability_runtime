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
#include "clone_for_account_util.h"
#undef private

#include "ability_record.h"
#include "auto_app_index.h"
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
    CloneForAccountUtil::appIndexMap_.clear();
    AppExecFwk::BundleMgrHelper::isBundleManagerHelperNull = false;
    AppExecFwk::BundleMgrHelper::retQueryEnabledAbilityInfoWithAppIndex = false;
    AppExecFwk::BundleMgrHelper::abilityInfoResultWithAppIndex = AppExecFwk::AbilityInfo();
    Token::abilityRecord = nullptr;
}

void CloneForAccountUtilTest::TearDown()
{
    CloneForAccountUtil::appIndexMap_.clear();
}

// ========== CloneForAccountUtil cache tests ==========

/**
 * @tc.name: CacheAppIndex_001
 * @tc.desc: Test CacheAppIndex and GetCachedAppIndex normal flow
 * @tc.type: FUNC
 */
HWTEST_F(CloneForAccountUtilTest, CacheAppIndex_001, TestSize.Level1)
{
    const std::string bundleName = "com.test.bundle";
    int32_t appIndex = 0;
    EXPECT_FALSE(CloneForAccountUtil::GetCachedAppIndex(bundleName, appIndex));

    CloneForAccountUtil::CacheAppIndex(bundleName, 1);
    EXPECT_TRUE(CloneForAccountUtil::GetCachedAppIndex(bundleName, appIndex));
    EXPECT_EQ(appIndex, 1);
}

/**
 * @tc.name: CacheAppIndex_002
 * @tc.desc: Test CacheAppIndex with empty bundleName
 * @tc.type: FUNC
 */
HWTEST_F(CloneForAccountUtilTest, CacheAppIndex_002, TestSize.Level1)
{
    int32_t appIndex = 0;
    CloneForAccountUtil::CacheAppIndex("", 1);
    EXPECT_FALSE(CloneForAccountUtil::GetCachedAppIndex("", appIndex));
}

/**
 * @tc.name: RemoveCachedAppIndex_001
 * @tc.desc: Test RemoveCachedAppIndex normal flow
 * @tc.type: FUNC
 */
HWTEST_F(CloneForAccountUtilTest, RemoveCachedAppIndex_001, TestSize.Level1)
{
    const std::string bundleName = "com.test.bundle";
    CloneForAccountUtil::CacheAppIndex(bundleName, 2);

    int32_t appIndex = 0;
    EXPECT_TRUE(CloneForAccountUtil::GetCachedAppIndex(bundleName, appIndex));
    EXPECT_EQ(appIndex, 2);

    CloneForAccountUtil::RemoveCachedAppIndex(bundleName);
    EXPECT_FALSE(CloneForAccountUtil::GetCachedAppIndex(bundleName, appIndex));
}

/**
 * @tc.name: RemoveCachedAppIndex_002
 * @tc.desc: Test RemoveCachedAppIndex with empty bundleName
 * @tc.type: FUNC
 */
HWTEST_F(CloneForAccountUtilTest, RemoveCachedAppIndex_002, TestSize.Level1)
{
    CloneForAccountUtil::RemoveCachedAppIndex("");
}

/**
 * @tc.name: GetCachedAppIndex_001
 * @tc.desc: Test GetCachedAppIndex returns false for non-existent bundle
 * @tc.type: FUNC
 */
HWTEST_F(CloneForAccountUtilTest, GetCachedAppIndex_001, TestSize.Level1)
{
    int32_t appIndex = 0;
    EXPECT_FALSE(CloneForAccountUtil::GetCachedAppIndex("com.nonexistent", appIndex));
}

// ========== CloneForAccountUtil ProcessAppIndex tests ==========

/**
 * @tc.name: ProcessAppIndex_001
 * @tc.desc: Test ProcessAppIndex with appIndex in want
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
    AppExecFwk::BundleMgrHelper::abilityInfoResultWithAppIndex = mockAbilityInfo;
    AppExecFwk::BundleMgrHelper::retQueryEnabledAbilityInfoWithAppIndex = true;

    CloneForAccountUtil::ProcessAppIndex(want, nullptr, 100);

    EXPECT_TRUE(want.HasParameter(Want::PARAM_APP_CLONE_INDEX_KEY));
    EXPECT_EQ(want.GetIntParam(Want::PARAM_APP_CLONE_INDEX_KEY, -1), 1);
}

/**
 * @tc.name: ProcessAppIndex_002
 * @tc.desc: Test ProcessAppIndex with bundleMgrHelper null
 * @tc.type: FUNC
 */
HWTEST_F(CloneForAccountUtilTest, ProcessAppIndex_002, TestSize.Level1)
{
    Want want;
    want.SetElementName("com.test.bundle", "MainAbility");
    want.SetParam(Want::PARAM_APP_CLONE_INDEX_KEY, 1);

    AppExecFwk::BundleMgrHelper::isBundleManagerHelperNull = true;

    CloneForAccountUtil::ProcessAppIndex(want, nullptr, 100);

    EXPECT_FALSE(want.HasParameter(Want::PARAM_APP_CLONE_INDEX_KEY));
}

/**
 * @tc.name: ProcessAppIndex_003
 * @tc.desc: Test ProcessAppIndex with QueryEnabledAbilityInfo failed
 * @tc.type: FUNC
 */
HWTEST_F(CloneForAccountUtilTest, ProcessAppIndex_003, TestSize.Level1)
{
    Want want;
    want.SetElementName("com.test.bundle", "MainAbility");
    want.SetParam(Want::PARAM_APP_CLONE_INDEX_KEY, 1);

    AppExecFwk::BundleMgrHelper::retQueryEnabledAbilityInfoWithAppIndex = false;

    CloneForAccountUtil::ProcessAppIndex(want, nullptr, 100);

    EXPECT_FALSE(want.HasParameter(Want::PARAM_APP_CLONE_INDEX_KEY));
}

/**
 * @tc.name: ProcessAppIndex_004
 * @tc.desc: Test ProcessAppIndex with no appIndex in want and no caller
 * @tc.type: FUNC
 */
HWTEST_F(CloneForAccountUtilTest, ProcessAppIndex_004, TestSize.Level1)
{
    Want want;
    want.SetElementName("com.test.bundle", "MainAbility");

    AppExecFwk::AbilityInfo mockAbilityInfo;
    mockAbilityInfo.appIndex = 0;
    mockAbilityInfo.applicationInfo.enabled = true;
    AppExecFwk::BundleMgrHelper::abilityInfoResultWithAppIndex = mockAbilityInfo;
    AppExecFwk::BundleMgrHelper::retQueryEnabledAbilityInfoWithAppIndex = true;

    CloneForAccountUtil::ProcessAppIndex(want, nullptr, 100);

    EXPECT_TRUE(want.HasParameter(Want::PARAM_APP_CLONE_INDEX_KEY));
    EXPECT_EQ(want.GetIntParam(Want::PARAM_APP_CLONE_INDEX_KEY, -1), 0);
}

/**
 * @tc.name: ProcessAppIndex_005
 * @tc.desc: Test ProcessAppIndex gets appIndex from caller
 * @tc.type: FUNC
 */
HWTEST_F(CloneForAccountUtilTest, ProcessAppIndex_005, TestSize.Level1)
{
    Want want;
    want.SetElementName("com.test.bundle", "MainAbility");

    auto abilityRecord = std::make_shared<AbilityRecord>();
    abilityRecord->appIndex_ = 2;
    Token::abilityRecord = abilityRecord;

    auto token = sptr<Token>::MakeSptr();

    AppExecFwk::AbilityInfo mockAbilityInfo;
    mockAbilityInfo.appIndex = 2;
    mockAbilityInfo.applicationInfo.enabled = true;
    AppExecFwk::BundleMgrHelper::abilityInfoResultWithAppIndex = mockAbilityInfo;
    AppExecFwk::BundleMgrHelper::retQueryEnabledAbilityInfoWithAppIndex = true;

    CloneForAccountUtil::ProcessAppIndex(want, token, 100);

    EXPECT_TRUE(want.HasParameter(Want::PARAM_APP_CLONE_INDEX_KEY));
    EXPECT_EQ(want.GetIntParam(Want::PARAM_APP_CLONE_INDEX_KEY, -1), 2);
}

// ========== AutoAppIndex tests ==========

/**
 * @tc.name: AutoAppIndex_001
 * @tc.desc: Test AutoAppIndex caches and cleans up on destruction
 * @tc.type: FUNC
 */
HWTEST_F(CloneForAccountUtilTest, AutoAppIndex_001, TestSize.Level1)
{
    Want want;
    want.SetElementName("com.test.bundle", "MainAbility");

    AppExecFwk::AbilityInfo mockAbilityInfo;
    mockAbilityInfo.appIndex = 3;
    mockAbilityInfo.applicationInfo.enabled = true;
    AppExecFwk::BundleMgrHelper::abilityInfoResultWithAppIndex = mockAbilityInfo;
    AppExecFwk::BundleMgrHelper::retQueryEnabledAbilityInfoWithAppIndex = true;

    {
        AutoAppIndex autoAppIndex(want, nullptr, 100);
        EXPECT_TRUE(want.HasParameter(Want::PARAM_APP_CLONE_INDEX_KEY));
        EXPECT_EQ(want.GetIntParam(Want::PARAM_APP_CLONE_INDEX_KEY, -1), 3);

        int32_t cachedAppIndex = 0;
        EXPECT_TRUE(CloneForAccountUtil::GetCachedAppIndex("com.test.bundle", cachedAppIndex));
        EXPECT_EQ(cachedAppIndex, 3);
    }

    int32_t cachedAppIndex = 0;
    EXPECT_FALSE(CloneForAccountUtil::GetCachedAppIndex("com.test.bundle", cachedAppIndex));
}

/**
 * @tc.name: AutoAppIndex_002
 * @tc.desc: Test AutoAppIndex cache hit skips query
 * @tc.type: FUNC
 */
HWTEST_F(CloneForAccountUtilTest, AutoAppIndex_002, TestSize.Level1)
{
    CloneForAccountUtil::CacheAppIndex("com.test.bundle", 5);

    Want want;
    want.SetElementName("com.test.bundle", "MainAbility");

    AppExecFwk::BundleMgrHelper::retQueryEnabledAbilityInfoWithAppIndex = false;

    {
        AutoAppIndex autoAppIndex(want, nullptr, 100);
        EXPECT_TRUE(want.HasParameter(Want::PARAM_APP_CLONE_INDEX_KEY));
        EXPECT_EQ(want.GetIntParam(Want::PARAM_APP_CLONE_INDEX_KEY, -1), 5);
    }

    int32_t cachedAppIndex = 0;
    EXPECT_TRUE(CloneForAccountUtil::GetCachedAppIndex("com.test.bundle", cachedAppIndex));
    EXPECT_EQ(cachedAppIndex, 5);
}

/**
 * @tc.name: AutoAppIndex_003
 * @tc.desc: Test AutoAppIndex with empty bundleName does not cache
 * @tc.type: FUNC
 */
HWTEST_F(CloneForAccountUtilTest, AutoAppIndex_003, TestSize.Level1)
{
    Want want;

    AppExecFwk::AbilityInfo mockAbilityInfo;
    mockAbilityInfo.appIndex = 1;
    mockAbilityInfo.applicationInfo.enabled = true;
    AppExecFwk::BundleMgrHelper::abilityInfoResultWithAppIndex = mockAbilityInfo;
    AppExecFwk::BundleMgrHelper::retQueryEnabledAbilityInfoWithAppIndex = true;

    {
        AutoAppIndex autoAppIndex(want, nullptr, 100);
    }

    EXPECT_EQ(CloneForAccountUtil::appIndexMap_.size(), 0u);
}

/**
 * @tc.name: AutoAppIndex_004
 * @tc.desc: Test AutoAppIndex with query failure does not cache
 * @tc.type: FUNC
 */
HWTEST_F(CloneForAccountUtilTest, AutoAppIndex_004, TestSize.Level1)
{
    Want want;
    want.SetElementName("com.test.bundle", "MainAbility");

    AppExecFwk::BundleMgrHelper::retQueryEnabledAbilityInfoWithAppIndex = false;

    {
        AutoAppIndex autoAppIndex(want, nullptr, 100);
    }

    EXPECT_EQ(CloneForAccountUtil::appIndexMap_.size(), 0u);
}

/**
 * @tc.name: AutoAppIndex_005
 * @tc.desc: Test AutoAppIndex nested scenario - only real query cleans up
 * @tc.type: FUNC
 */
HWTEST_F(CloneForAccountUtilTest, AutoAppIndex_005, TestSize.Level1)
{
    Want want1;
    want1.SetElementName("com.test.bundle", "MainAbility");

    AppExecFwk::AbilityInfo mockAbilityInfo;
    mockAbilityInfo.appIndex = 1;
    mockAbilityInfo.applicationInfo.enabled = true;
    AppExecFwk::BundleMgrHelper::abilityInfoResultWithAppIndex = mockAbilityInfo;
    AppExecFwk::BundleMgrHelper::retQueryEnabledAbilityInfoWithAppIndex = true;

    {
        AutoAppIndex outer(want1, nullptr, 100);

        int32_t cachedAppIndex = 0;
        EXPECT_TRUE(CloneForAccountUtil::GetCachedAppIndex("com.test.bundle", cachedAppIndex));
        EXPECT_EQ(cachedAppIndex, 1);

        Want want2;
        want2.SetElementName("com.test.bundle", "MainAbility");
        {
            AutoAppIndex inner(want2, nullptr, 100);
            EXPECT_EQ(want2.GetIntParam(Want::PARAM_APP_CLONE_INDEX_KEY, -1), 1);
        }

        EXPECT_TRUE(CloneForAccountUtil::GetCachedAppIndex("com.test.bundle", cachedAppIndex));
    }

    int32_t cachedAppIndex = 0;
    EXPECT_FALSE(CloneForAccountUtil::GetCachedAppIndex("com.test.bundle", cachedAppIndex));
}

}  // namespace AAFwk
}  // namespace OHOS
