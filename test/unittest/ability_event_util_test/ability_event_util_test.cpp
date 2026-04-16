/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#include <iostream>
#include <cstddef>
#include <cstdint>
#define private public
#include "ability_event_util.h"
#undef private
#include "hilog_tag_wrapper.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AbilityRuntime {
class AbilityEventUtilTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void AbilityEventUtilTest::SetUpTestCase(void) {}
void AbilityEventUtilTest::TearDownTestCase(void) {}
void AbilityEventUtilTest::SetUp() {}
void AbilityEventUtilTest::TearDown() {}

/**
 * @tc.name: AbilityEventUtil_SendStartAbilityError_0100
 * @tc.desc: SendStartAbilityError
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(AbilityEventUtilTest, AbilityEventUtil_SendStartAbilityError_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityEventUtil_SendStartAbilityError_0100 start");
    EventInfo eventInfo = {};
    int32_t errCode = -1;
    std::string errMsg = "test event";
    AbilityEventUtil::SendStartAbilityErrorEvent(eventInfo, errCode, errMsg);
    EXPECT_EQ(eventInfo.userId, -1);
    TAG_LOGI(AAFwkTag::TEST, "AbilityEventUtil_SendStartAbilityError_0100 end");
}

/**
 * @tc.name: AbilityEventUtil_SendStartAbilityError_0200
 * @tc.desc: SendStartAbilityError
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(AbilityEventUtilTest, AbilityEventUtil_SendStartAbilityError_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityEventUtil_SendStartAbilityError_0200 start");
    EventInfo eventInfo = {};
    int32_t errCode = -1;
    std::string errMsg = "test event";
    AbilityEventUtil::SendStartAbilityErrorEvent(eventInfo, errCode, errMsg, true);
    EXPECT_EQ(eventInfo.userId, -1);
    TAG_LOGI(AAFwkTag::TEST, "AbilityEventUtil_SendStartAbilityError_0200 end");
}

/**
 * @tc.name: AbilityEventUtil_SendStartAbilityError_0300
 * @tc.desc: SendKillProcessWithReasonEvent
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(AbilityEventUtilTest, AbilityEventUtil_SendStartAbilityError_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityEventUtil_SendStartAbilityError_0300 start");
    EventInfo eventInfo = {};
    int32_t errCode = 0;
    std::string errMsg = "test event";
    AbilityEventUtil::SendStartAbilityErrorEvent(eventInfo, errCode, errMsg);
    EXPECT_EQ(eventInfo.userId, -1);
    TAG_LOGI(AAFwkTag::TEST, "AbilityEventUtil_SendStartAbilityError_0300 end");
}

/**
 * @tc.name: AbilityEventUtil_SendKillProcessWithReasonEvent_0100
 * @tc.desc: SendKillProcessWithReasonEvent
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(AbilityEventUtilTest, AbilityEventUtil_SendKillProcessWithReasonEvent_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityEventUtil_SendKillProcessWithReasonEvent_0100 start");
    EventInfo eventInfo = {};
    int32_t errCode = -1;
    std::string errMsg = "test event";
    AbilityEventUtil::SendKillProcessWithReasonEvent(errCode, errMsg, eventInfo);
    EXPECT_EQ(eventInfo.userId, -1);
    TAG_LOGI(AAFwkTag::TEST, "AbilityEventUtil_SendKillProcessWithReasonEvent_0100 end");
}

/**
 * @tc.name: AbilityEventUtil_HandleBundleFirstLaunch_0100
 * @tc.desc: Test HandleBundleFirstLaunch with first launch (isBundleFirstLaunched = false)
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(AbilityEventUtilTest, AbilityEventUtil_HandleBundleFirstLaunch_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityEventUtil_HandleBundleFirstLaunch_0100 start");
    ApplicationInfo appInfo;
    appInfo.bundleName = "com.test.demo";
    appInfo.isBundleFirstLaunched = false;
    appInfo.appIndex = 0;
    appInfo.uid = 20010000;
    int32_t userId = 100;

    bool result = AbilityEventUtil::HandleBundleFirstLaunch(appInfo, userId);
    EXPECT_EQ(result, true);
    TAG_LOGI(AAFwkTag::TEST, "AbilityEventUtil_HandleBundleFirstLaunch_0100 end");
}

/**
 * @tc.name: AbilityEventUtil_HandleBundleFirstLaunch_0200
 * @tc.desc: Test HandleBundleFirstLaunch with already launched (isBundleFirstLaunched = true)
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(AbilityEventUtilTest, AbilityEventUtil_HandleBundleFirstLaunch_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityEventUtil_HandleBundleFirstLaunch_0200 start");
    ApplicationInfo appInfo;
    appInfo.bundleName = "com.test.demo";
    appInfo.isBundleFirstLaunched = true;
    appInfo.appIndex = 0;
    appInfo.uid = 20010000;
    int32_t userId = 100;

    bool result = AbilityEventUtil::HandleBundleFirstLaunch(appInfo, userId);
    EXPECT_EQ(result, true);
    TAG_LOGI(AAFwkTag::TEST, "AbilityEventUtil_HandleBundleFirstLaunch_0200 end");
}

/**
 * @tc.name: AbilityEventUtil_HandleBundleFirstLaunch_0300
 * @tc.desc: Test HandleBundleFirstLaunch with clone app (appIndex > 0)
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(AbilityEventUtilTest, AbilityEventUtil_HandleBundleFirstLaunch_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityEventUtil_HandleBundleFirstLaunch_0300 start");
    ApplicationInfo appInfo;
    appInfo.bundleName = "com.test.demo";
    appInfo.isBundleFirstLaunched = false;
    appInfo.appIndex = 1;
    appInfo.uid = 20010001;
    int32_t userId = 100;

    bool result = AbilityEventUtil::HandleBundleFirstLaunch(appInfo, userId);
    EXPECT_EQ(result, true);
    TAG_LOGI(AAFwkTag::TEST, "AbilityEventUtil_HandleBundleFirstLaunch_0300 end");
}

/**
 * @tc.name: AbilityEventUtil_HandleBundleFirstLaunch_0400
 * @tc.desc: Test HandleBundleFirstLaunch with second clone app (appIndex = 2)
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(AbilityEventUtilTest, AbilityEventUtil_HandleBundleFirstLaunch_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityEventUtil_HandleBundleFirstLaunch_0400 start");
    ApplicationInfo appInfo;
    appInfo.bundleName = "com.test.demo";
    appInfo.isBundleFirstLaunched = false;
    appInfo.appIndex = 2;
    appInfo.uid = 20010002;
    int32_t userId = 100;

    bool result = AbilityEventUtil::HandleBundleFirstLaunch(appInfo, userId);
    EXPECT_EQ(result, true);
    TAG_LOGI(AAFwkTag::TEST, "AbilityEventUtil_HandleBundleFirstLaunch_0400 end");
}

/**
 * @tc.name: AbilityEventUtil_HandleBundleFirstLaunch_0500
 * @tc.desc: Test HandleBundleFirstLaunch with clone app already launched
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(AbilityEventUtilTest, AbilityEventUtil_HandleBundleFirstLaunch_0500, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityEventUtil_HandleBundleFirstLaunch_0500 start");
    ApplicationInfo appInfo;
    appInfo.bundleName = "com.test.demo";
    appInfo.isBundleFirstLaunched = true;
    appInfo.appIndex = 1;
    appInfo.uid = 20010001;
    int32_t userId = 100;

    bool result = AbilityEventUtil::HandleBundleFirstLaunch(appInfo, userId);
    EXPECT_EQ(result, true);
    TAG_LOGI(AAFwkTag::TEST, "AbilityEventUtil_HandleBundleFirstLaunch_0500 end");
}

/**
 * @tc.name: AbilityEventUtil_HandleBundleFirstLaunch_0600
 * @tc.desc: Test HandleBundleFirstLaunch with different userId
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(AbilityEventUtilTest, AbilityEventUtil_HandleBundleFirstLaunch_0600, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityEventUtil_HandleBundleFirstLaunch_0600 start");
    ApplicationInfo appInfo;
    appInfo.bundleName = "com.test.demo";
    appInfo.isBundleFirstLaunched = false;
    appInfo.appIndex = 0;
    appInfo.uid = 20020000;
    int32_t userId = 200;

    bool result = AbilityEventUtil::HandleBundleFirstLaunch(appInfo, userId);
    EXPECT_EQ(result, true);
    TAG_LOGI(AAFwkTag::TEST, "AbilityEventUtil_HandleBundleFirstLaunch_0600 end");
}

/**
 * @tc.name: AbilityEventUtil_HandleBundleFirstLaunch_0700
 * @tc.desc: Test HandleBundleFirstLaunch with callerBundleName parameter
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(AbilityEventUtilTest, AbilityEventUtil_HandleBundleFirstLaunch_0700, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityEventUtil_HandleBundleFirstLaunch_0700 start");
    ApplicationInfo appInfo;
    appInfo.bundleName = "com.test.demo";
    appInfo.isBundleFirstLaunched = false;
    appInfo.appIndex = 0;
    appInfo.uid = 20010000;
    int32_t userId = 100;
    std::string callerBundleName = "com.test.caller";

    bool result = AbilityEventUtil::HandleBundleFirstLaunch(appInfo, userId, callerBundleName);
    EXPECT_EQ(result, true);
    TAG_LOGI(AAFwkTag::TEST, "AbilityEventUtil_HandleBundleFirstLaunch_0700 end");
}

/**
 * @tc.name: AbilityEventUtil_HandleBundleFirstLaunch_0800
 * @tc.desc: Test HandleBundleFirstLaunch with clone app and callerBundleName
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(AbilityEventUtilTest, AbilityEventUtil_HandleBundleFirstLaunch_0800, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityEventUtil_HandleBundleFirstLaunch_0800 start");
    ApplicationInfo appInfo;
    appInfo.bundleName = "com.test.demo";
    appInfo.isBundleFirstLaunched = false;
    appInfo.appIndex = 1;
    appInfo.uid = 20010001;
    int32_t userId = 100;
    std::string callerBundleName = "com.test.caller";

    bool result = AbilityEventUtil::HandleBundleFirstLaunch(appInfo, userId, callerBundleName);
    EXPECT_EQ(result, true);
    TAG_LOGI(AAFwkTag::TEST, "AbilityEventUtil_HandleBundleFirstLaunch_0800 end");
}

/**
 * @tc.name: AbilityEventUtil_HandleBundleFirstLaunch_0900
 * @tc.desc: Test HandleBundleFirstLaunch with _system callerBundleName (system caller)
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(AbilityEventUtilTest, AbilityEventUtil_HandleBundleFirstLaunch_0900, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityEventUtil_HandleBundleFirstLaunch_0900 start");
    ApplicationInfo appInfo;
    appInfo.bundleName = "com.test.demo";
    appInfo.isBundleFirstLaunched = false;
    appInfo.appIndex = 0;
    appInfo.uid = 20010000;
    int32_t userId = 100;
    std::string callerBundleName = "_system";

    bool result = AbilityEventUtil::HandleBundleFirstLaunch(appInfo, userId, callerBundleName);
    EXPECT_EQ(result, true);
    TAG_LOGI(AAFwkTag::TEST, "AbilityEventUtil_HandleBundleFirstLaunch_0900 end");
}

/**
 * @tc.name: AbilityEventUtil_HandleBundleFirstLaunch_1000
 * @tc.desc: Test HandleBundleFirstLaunch with default callerBundleName (no parameter)
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(AbilityEventUtilTest, AbilityEventUtil_HandleBundleFirstLaunch_1000, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityEventUtil_HandleBundleFirstLaunch_1000 start");
    ApplicationInfo appInfo;
    appInfo.bundleName = "com.test.demo";
    appInfo.isBundleFirstLaunched = false;
    appInfo.appIndex = 0;
    appInfo.uid = 20010000;
    int32_t userId = 100;

    bool result = AbilityEventUtil::HandleBundleFirstLaunch(appInfo, userId);
    EXPECT_EQ(result, true);
    TAG_LOGI(AAFwkTag::TEST, "AbilityEventUtil_HandleBundleFirstLaunch_1000 end");
}

}  // namespace AAFwk
}  // namespace OHOS
