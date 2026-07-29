/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "hilog_tag_wrapper.h"
#include "app_mgr_util.h"
#include "global_constant.h"
#include "mock_app_mgr_service.h"
#include "mock_bundle_mgr_helper_status.h"
#include "multi_app_utils.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AAFwk {
class MultiAppUtilsTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void MultiAppUtilsTest::SetUpTestCase(void) {}
void MultiAppUtilsTest::TearDownTestCase(void) {}
void MultiAppUtilsTest::SetUp()
{
    MockBundleMgrHelperStatus::Reset();
}
void MultiAppUtilsTest::TearDown()
{
    MockBundleMgrHelperStatus::Reset();
}

/**
 * @tc.name: GetRunningMultiAppIndex_0100
 * @tc.desc: GetRunningMultiAppIndex
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(MultiAppUtilsTest, GetRunningMultiAppIndex_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetRunningMultiAppIndex_0100 start");
    std::string bundleName = "testBundleName";
    int32_t uid = 1000;
    int32_t appIndex = -1;
    auto appMgr = AppMgrUtil::GetAppMgr();
    EXPECT_NE(appMgr, nullptr);

    AppExecFwk::MockAppMgrService::retCode_ = 0;
    AppExecFwk::RunningAppClone appClone = {
        .appCloneIndex = 13,
        .uid = 1000
    };
    std::vector<AppExecFwk::RunningAppClone> appClones = { appClone };
    AppExecFwk::MockAppMgrService::retInfo_.runningAppClones = appClones;
    MultiAppUtils::GetRunningMultiAppIndex(bundleName, uid, appIndex);
    EXPECT_EQ(appIndex, 13);

    TAG_LOGI(AAFwkTag::TEST, "GetRunningMultiAppIndex_0100 end");
}

/**
 * @tc.name: GetRunningMultiAppIndex_0200
 * @tc.desc: GetRunningMultiAppIndex
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(MultiAppUtilsTest, GetRunningMultiAppIndex_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetRunningMultiAppIndex_0200 start");
    std::string bundleName = "testBundleName";
    int32_t uid = 1000;
    int32_t appIndex = -1;
    auto appMgr = AppMgrUtil::GetAppMgr();
    EXPECT_NE(appMgr, nullptr);

    AppExecFwk::MockAppMgrService::retCode_ = -1;
    MultiAppUtils::GetRunningMultiAppIndex(bundleName, uid, appIndex);
    EXPECT_EQ(appIndex, -1);

    TAG_LOGI(AAFwkTag::TEST, "GetRunningMultiAppIndex_0200 end");
}

/**
 * @tc.name: GetPreferredAppCloneIndex_0100
 * @tc.desc: GetPreferredAppCloneIndex returns false for empty bundle and does not query BMS.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(MultiAppUtilsTest, GetPreferredAppCloneIndex_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetPreferredAppCloneIndex_0100 start");
    int32_t appIndex = -1;
    MockBundleMgrHelperStatus::appClonePreference_.mode = AppExecFwk::AppClonePreferenceMode::CLONE_APP;
    MockBundleMgrHelperStatus::appClonePreference_.appIndex = 1;

    EXPECT_FALSE(MultiAppUtils::GetPreferredAppCloneIndex("", 100, appIndex));

    EXPECT_EQ(appIndex, -1);
    EXPECT_TRUE(MockBundleMgrHelperStatus::lastClonePreferenceBundleName_.empty());
    TAG_LOGI(AAFwkTag::TEST, "GetPreferredAppCloneIndex_0100 end");
}

/**
 * @tc.name: GetPreferredAppCloneIndex_0200
 * @tc.desc: GetPreferredAppCloneIndex returns false when BMS returns an error.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(MultiAppUtilsTest, GetPreferredAppCloneIndex_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetPreferredAppCloneIndex_0200 start");
    int32_t appIndex = -1;
    MockBundleMgrHelperStatus::getAppClonePreferenceRet_ = -1;
    MockBundleMgrHelperStatus::appClonePreference_.mode = AppExecFwk::AppClonePreferenceMode::CLONE_APP;
    MockBundleMgrHelperStatus::appClonePreference_.appIndex = 1;

    EXPECT_FALSE(MultiAppUtils::GetPreferredAppCloneIndex("testBundleName", 100, appIndex));

    EXPECT_EQ(appIndex, -1);
    EXPECT_EQ(MockBundleMgrHelperStatus::lastClonePreferenceBundleName_, "testBundleName");
    EXPECT_EQ(MockBundleMgrHelperStatus::lastClonePreferenceUserId_, 100);
    TAG_LOGI(AAFwkTag::TEST, "GetPreferredAppCloneIndex_0200 end");
}

/**
 * @tc.name: GetPreferredAppCloneIndex_0300
 * @tc.desc: GetPreferredAppCloneIndex maps MAIN_APP preference to app index 0.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(MultiAppUtilsTest, GetPreferredAppCloneIndex_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetPreferredAppCloneIndex_0300 start");
    int32_t appIndex = -1;
    MockBundleMgrHelperStatus::appClonePreference_.mode = AppExecFwk::AppClonePreferenceMode::MAIN_APP;
    MockBundleMgrHelperStatus::appClonePreference_.appIndex = 2;

    EXPECT_TRUE(MultiAppUtils::GetPreferredAppCloneIndex("testBundleName", 100, appIndex));

    EXPECT_EQ(appIndex, 0);
    EXPECT_EQ(MockBundleMgrHelperStatus::lastClonePreferenceBundleName_, "testBundleName");
    EXPECT_EQ(MockBundleMgrHelperStatus::lastClonePreferenceUserId_, 100);
    TAG_LOGI(AAFwkTag::TEST, "GetPreferredAppCloneIndex_0300 end");
}

/**
 * @tc.name: GetPreferredAppCloneIndex_0400
 * @tc.desc: GetPreferredAppCloneIndex returns false for ALWAYS_ASK preference.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(MultiAppUtilsTest, GetPreferredAppCloneIndex_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetPreferredAppCloneIndex_0400 start");
    int32_t appIndex = -1;
    MockBundleMgrHelperStatus::appClonePreference_.mode = AppExecFwk::AppClonePreferenceMode::ALWAYS_ASK;
    MockBundleMgrHelperStatus::appClonePreference_.appIndex = 2;

    EXPECT_FALSE(MultiAppUtils::GetPreferredAppCloneIndex("testBundleName", 100, appIndex));

    EXPECT_EQ(appIndex, -1);
    EXPECT_EQ(MockBundleMgrHelperStatus::lastClonePreferenceBundleName_, "testBundleName");
    EXPECT_EQ(MockBundleMgrHelperStatus::lastClonePreferenceUserId_, 100);
    TAG_LOGI(AAFwkTag::TEST, "GetPreferredAppCloneIndex_0400 end");
}

/**
 * @tc.name: GetPreferredAppCloneIndex_0500
 * @tc.desc: GetPreferredAppCloneIndex validates CLONE_APP preference boundaries.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(MultiAppUtilsTest, GetPreferredAppCloneIndex_0500, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetPreferredAppCloneIndex_0500 start");
    int32_t appIndex = -1;
    MockBundleMgrHelperStatus::appClonePreference_.mode = AppExecFwk::AppClonePreferenceMode::CLONE_APP;

    MockBundleMgrHelperStatus::appClonePreference_.appIndex = 0;
    EXPECT_FALSE(MultiAppUtils::GetPreferredAppCloneIndex("testBundleName", 100, appIndex));
    EXPECT_EQ(appIndex, -1);

    MockBundleMgrHelperStatus::appClonePreference_.appIndex =
        AbilityRuntime::GlobalConstant::MAX_APP_CLONE_INDEX + 1;
    EXPECT_FALSE(MultiAppUtils::GetPreferredAppCloneIndex("testBundleName", 100, appIndex));
    EXPECT_EQ(appIndex, -1);

    MockBundleMgrHelperStatus::appClonePreference_.appIndex =
        AbilityRuntime::GlobalConstant::MAX_APP_CLONE_INDEX;
    EXPECT_TRUE(MultiAppUtils::GetPreferredAppCloneIndex("testBundleName", 100, appIndex));
    EXPECT_EQ(appIndex, AbilityRuntime::GlobalConstant::MAX_APP_CLONE_INDEX);

    MockBundleMgrHelperStatus::appClonePreference_.appIndex = 1;
    EXPECT_TRUE(MultiAppUtils::GetPreferredAppCloneIndex("testBundleName", 100, appIndex));
    EXPECT_EQ(appIndex, 1);
    TAG_LOGI(AAFwkTag::TEST, "GetPreferredAppCloneIndex_0500 end");
}

/**
 * @tc.name: GetPreferredAppCloneIndex_0600
 * @tc.desc: GetPreferredAppCloneIndex returns false when bundle manager helper is null and does not query BMS.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(MultiAppUtilsTest, GetPreferredAppCloneIndex_0600, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetPreferredAppCloneIndex_0600 start");
    int32_t appIndex = -1;
    MockBundleMgrHelperStatus::returnNullHelper_ = true;
    MockBundleMgrHelperStatus::appClonePreference_.mode = AppExecFwk::AppClonePreferenceMode::CLONE_APP;
    MockBundleMgrHelperStatus::appClonePreference_.appIndex = 1;

    EXPECT_FALSE(MultiAppUtils::GetPreferredAppCloneIndex("testBundleName", 100, appIndex));

    EXPECT_EQ(appIndex, -1);
    EXPECT_TRUE(MockBundleMgrHelperStatus::lastClonePreferenceBundleName_.empty());
    TAG_LOGI(AAFwkTag::TEST, "GetPreferredAppCloneIndex_0600 end");
}
}  // namespace AAFwk
}  // namespace OHOS
