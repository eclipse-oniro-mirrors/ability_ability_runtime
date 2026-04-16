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

#include "insight_intent_sys_event_receiver.h"
#include "insight_intent_event_mgr.h"
#include "common_event_support.h"
#include "hilog_tag_wrapper.h"

using namespace testing::ext;
using namespace testing;
namespace OHOS {
namespace AbilityRuntime {
const int32_t MAIN_USER_ID = 100;
const int32_t OTHER_USER_ID = 101;
const int32_t INVALID_USER_ID = -1;
const int32_t ZERO_USER_ID = 0;
const std::string INVALID_BUNDLE_NAME = "";
const std::string TEST_BUNDLE_NAME = "com.test.insightintent";
const std::string TEST_MODULE_NAME = "testmodule";
const std::string MULTI_MODULE_NAME = "module1,module2,module3";
const std::string EMPTY_MODULE_NAME = "";

class InsightIntentSysEventReceiverTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};
void InsightIntentSysEventReceiverTest::SetUpTestCase(void) {}
void InsightIntentSysEventReceiverTest::TearDownTestCase(void) {}
void InsightIntentSysEventReceiverTest::TearDown() {}
void InsightIntentSysEventReceiverTest::SetUp() {}

/**
 * @tc.name: InsightIntentSysEventReceiverTest_OnReceiveEvent_0001
 * @tc.desc: Test the state of OnReceiveEvent
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentSysEventReceiverTest, OnReceiveEvent_0001, TestSize.Level1)
{
    // modulename invalid
    AppExecFwk::ElementName element1("", "com.test.demo", "MainAbility");
    AbilityRuntime::InsightIntentEventMgr::UpdateInsightIntentEvent(element1, -1);
    // userId invalid
    AppExecFwk::ElementName element2("", "com.test.demo", "MainAbility", "module1, module2");
    AbilityRuntime::InsightIntentEventMgr::UpdateInsightIntentEvent(element2, -1);
    // param valid
    AbilityRuntime::InsightIntentEventMgr::UpdateInsightIntentEvent(element2, MAIN_USER_ID);

    // bundleName invalid
    AppExecFwk::ElementName element3("", "", "MainAbility", "module1, module2");
    AbilityRuntime::InsightIntentEventMgr::DeleteInsightIntentEvent(element3, -1, 1);
    // appIndex invalid
    AbilityRuntime::InsightIntentEventMgr::DeleteInsightIntentEvent(element1, -1, 1);
    // userId invalid
    AbilityRuntime::InsightIntentEventMgr::DeleteInsightIntentEvent(element1, -1, 0);
    // param valid
    AbilityRuntime::InsightIntentEventMgr::DeleteInsightIntentEvent(element1, MAIN_USER_ID, 0);
    AbilityRuntime::InsightIntentEventMgr::DeleteInsightIntent("com.test.demo", "module1", MAIN_USER_ID);

    EventFwk::CommonEventSubscribeInfo subscribeInfo;
    auto sysEventReceiver = std::make_shared<AbilityRuntime::InsightIntentSysEventReceiver>(subscribeInfo);

    EventFwk::CommonEventData data;
    data.want_.operation_.action_ = EventFwk::CommonEventSupport::COMMON_EVENT_USER_SWITCHED;
    data.code_ = 101;
    sysEventReceiver->OnReceiveEvent(data);
    EXPECT_NE(sysEventReceiver->lastUserId_, 0);

    data.want_.operation_.action_ = EventFwk::CommonEventSupport::COMMON_EVENT_BUNDLE_SCAN_FINISHED;
    sysEventReceiver->OnReceiveEvent(data);
    EXPECT_NE(sysEventReceiver->lastUserId_, 0);

    data.want_.operation_.action_ = EventFwk::CommonEventSupport::COMMON_EVENT_USER_SWITCHED;
    data.code_ = MAIN_USER_ID;
    sysEventReceiver->OnReceiveEvent(data);
    EXPECT_NE(sysEventReceiver->lastUserId_, 0);

    data.want_.operation_.action_ = EventFwk::CommonEventSupport::COMMON_EVENT_USER_REMOVED;
    data.code_ = OTHER_USER_ID;
    sysEventReceiver->OnReceiveEvent(data);
    EXPECT_NE(sysEventReceiver->lastUserId_, 0);
}

/**
 * @tc.name: InsightIntentSysEventReceiverTest_SaveInsightIntentInfos_0002
 * @tc.desc: Test SaveInsightIntentInfos with various input scenarios
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentSysEventReceiverTest, SaveInsightIntentInfos_0002, TestSize.Level1)
{
    // Local variable definitions
    EventFwk::CommonEventSubscribeInfo subscribeInfo;
    uint32_t ver = 0;
    auto sysEventReceiver = std::make_shared<AbilityRuntime::InsightIntentSysEventReceiver>(subscribeInfo);

    // Test 1: Invalid bundle name
    sysEventReceiver->SaveInsightIntentInfos(INVALID_BUNDLE_NAME, TEST_MODULE_NAME, ver, MAIN_USER_ID);
    EXPECT_EQ(sysEventReceiver->lastUserId_, 0); // Verify object state is normal and not affected by exceptions

    // Test 2: Empty module name
    sysEventReceiver->SaveInsightIntentInfos(TEST_BUNDLE_NAME, EMPTY_MODULE_NAME, ver, MAIN_USER_ID);
    EXPECT_EQ(sysEventReceiver->lastUserId_, 0);

    // Test 3: Multiple module names
    sysEventReceiver->SaveInsightIntentInfos(TEST_BUNDLE_NAME, MULTI_MODULE_NAME, ver, MAIN_USER_ID);
    EXPECT_EQ(sysEventReceiver->lastUserId_, 0);

    // Test 4: Invalid user ID
    sysEventReceiver->SaveInsightIntentInfos(TEST_BUNDLE_NAME, TEST_MODULE_NAME, ver, INVALID_USER_ID);
    EXPECT_EQ(sysEventReceiver->lastUserId_, 0);

    // Test 5: Valid parameters
    sysEventReceiver->SaveInsightIntentInfos(TEST_BUNDLE_NAME, TEST_MODULE_NAME, ver, MAIN_USER_ID);
    EXPECT_EQ(sysEventReceiver->lastUserId_, 0);
}

/**
 * @tc.name: InsightIntentSysEventReceiverTest_LoadInsightIntentInfos_0003
 * @tc.desc: Test LoadInsightIntentInfos with different userId scenarios
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentSysEventReceiverTest, LoadInsightIntentInfos_0003, TestSize.Level1)
{
    // Local variable definitions
    EventFwk::CommonEventSubscribeInfo subscribeInfo;
    auto sysEventReceiver = std::make_shared<AbilityRuntime::InsightIntentSysEventReceiver>(subscribeInfo);

    // Test 1: userId = -1
    sysEventReceiver->LoadInsightIntentInfos(INVALID_USER_ID);
    EXPECT_EQ(sysEventReceiver->lastUserId_, 0);

    // Test 2: userId = 0
    sysEventReceiver->LoadInsightIntentInfos(ZERO_USER_ID);
    EXPECT_EQ(sysEventReceiver->lastUserId_, 0);

    // Test 3: Valid user ID
    sysEventReceiver->LoadInsightIntentInfos(MAIN_USER_ID);
    EXPECT_EQ(sysEventReceiver->lastUserId_, 0);

    // Test 4: Other user ID
    sysEventReceiver->LoadInsightIntentInfos(OTHER_USER_ID);
    EXPECT_EQ(sysEventReceiver->lastUserId_, 0);
}

/**
 * @tc.name: InsightIntentSysEventReceiverTest_HandleUserSwitched_0004
 * @tc.desc: Test HandleUserSwitched edge cases
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentSysEventReceiverTest, HandleUserSwitched_0004, TestSize.Level1)
{
    // All variables are local
    EventFwk::CommonEventSubscribeInfo subscribeInfo;
    auto sysEventReceiver = std::make_shared<AbilityRuntime::InsightIntentSysEventReceiver>(subscribeInfo);
    EventFwk::CommonEventData data;

    // Test 1: Invalid userId (< 0)
    data.SetCode(INVALID_USER_ID);
    data.want_.operation_.action_ = EventFwk::CommonEventSupport::COMMON_EVENT_USER_SWITCHED;
    sysEventReceiver->OnReceiveEvent(data);
    EXPECT_EQ(sysEventReceiver->lastUserId_, -1);

    // Test 2: Same userId
    sysEventReceiver->lastUserId_ = MAIN_USER_ID;
    data.SetCode(MAIN_USER_ID);
    sysEventReceiver->OnReceiveEvent(data);
    EXPECT_EQ(sysEventReceiver->lastUserId_, 100);

    // Test 3: Zero user ID
    data.SetCode(ZERO_USER_ID);
    sysEventReceiver->OnReceiveEvent(data);
    EXPECT_EQ(sysEventReceiver->lastUserId_, 0);

    // Test 4: Valid new user ID
    data.SetCode(OTHER_USER_ID);
    sysEventReceiver->OnReceiveEvent(data);
    EXPECT_EQ(sysEventReceiver->lastUserId_, 101);
}

/**
 * @tc.name: InsightIntentSysEventReceiverTest_HandleUserRemove_0005
 * @tc.desc: Test HandleUserRemove edge cases
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentSysEventReceiverTest, HandleUserRemove_0005, TestSize.Level1)
{
    EventFwk::CommonEventSubscribeInfo subscribeInfo;
    auto sysEventReceiver = std::make_shared<AbilityRuntime::InsightIntentSysEventReceiver>(subscribeInfo);
    EventFwk::CommonEventData data;

    data.SetCode(INVALID_USER_ID);
    data.want_.operation_.action_ = EventFwk::CommonEventSupport::COMMON_EVENT_USER_REMOVED;
    sysEventReceiver->OnReceiveEvent(data);
    EXPECT_EQ(sysEventReceiver->lastUserId_, 0);

    sysEventReceiver->lastUserId_ = MAIN_USER_ID;
    data.SetCode(MAIN_USER_ID);
    sysEventReceiver->OnReceiveEvent(data);
    EXPECT_EQ(sysEventReceiver->lastUserId_, 100);

    data.SetCode(OTHER_USER_ID);
    sysEventReceiver->OnReceiveEvent(data);
    EXPECT_EQ(sysEventReceiver->lastUserId_, 100);
}

/**
 * @tc.name: InsightIntentSysEventReceiverTest_OnReceiveEvent_InvalidAction_0006
 * @tc.desc: Test OnReceiveEvent with invalid action
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentSysEventReceiverTest, OnReceiveEvent_InvalidAction_0006, TestSize.Level1)
{
    EventFwk::CommonEventSubscribeInfo subscribeInfo;
    auto sysEventReceiver = std::make_shared<AbilityRuntime::InsightIntentSysEventReceiver>(subscribeInfo);
    EventFwk::CommonEventData data;

    data.want_.operation_.action_ = EventFwk::CommonEventSupport::COMMON_EVENT_USER_SWITCHED;
    data.SetCode(MAIN_USER_ID);
    sysEventReceiver->OnReceiveEvent(data);
    EXPECT_EQ(sysEventReceiver->lastUserId_, 100);
    
    data.want_.operation_.action_ = "";
    sysEventReceiver->OnReceiveEvent(data);
    EXPECT_EQ(sysEventReceiver->lastUserId_, 100);

    data.want_.operation_.action_ = "com.test.invalid.action";
    sysEventReceiver->OnReceiveEvent(data);
    EXPECT_EQ(sysEventReceiver->lastUserId_, 100);

    data.want_.operation_.action_ = "Common_Event_User_Switched";
    sysEventReceiver->OnReceiveEvent(data);
    EXPECT_EQ(sysEventReceiver->lastUserId_, 100);
}

/**
 * @tc.name: InsightIntentSysEventReceiverTest_DeleteInsightIntentInfoByUserId_0007
 * @tc.desc: Test DeleteInsightIntentInfoByUserId with different user IDs
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentSysEventReceiverTest, DeleteInsightIntentInfoByUserId_0007, TestSize.Level1)
{
    EventFwk::CommonEventSubscribeInfo subscribeInfo;
    auto sysEventReceiver = std::make_shared<AbilityRuntime::InsightIntentSysEventReceiver>(subscribeInfo);

    sysEventReceiver->lastUserId_ = MAIN_USER_ID;
    EXPECT_EQ(sysEventReceiver->lastUserId_, 100);

    sysEventReceiver->DeleteInsightIntentInfoByUserId(INVALID_USER_ID);
    EXPECT_EQ(sysEventReceiver->lastUserId_, 100);

    sysEventReceiver->DeleteInsightIntentInfoByUserId(ZERO_USER_ID);
    EXPECT_EQ(sysEventReceiver->lastUserId_, 100);

    sysEventReceiver->DeleteInsightIntentInfoByUserId(MAIN_USER_ID);
    EXPECT_EQ(sysEventReceiver->lastUserId_, 100);

    sysEventReceiver->DeleteInsightIntentInfoByUserId(OTHER_USER_ID);
    EXPECT_EQ(sysEventReceiver->lastUserId_, 100);
}

/**
 * @tc.name: InsightIntentSysEventReceiverTest_DeleteInsightIntent_0008
 * @tc.desc: Test DeleteInsightIntent triggered when SaveInsightIntentInfos fails
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentSysEventReceiverTest, DeleteInsightIntent_0008, TestSize.Level1)
{
    EventFwk::CommonEventSubscribeInfo subscribeInfo;
    uint32_t ver = 100;
    auto sysEventReceiver = std::make_shared<AbilityRuntime::InsightIntentSysEventReceiver>(subscribeInfo);

    // Test scenario 1: SaveInsightIntentInfos failure triggers DeleteInsightIntent
    // When bundleName or moduleName is invalid, system should handle gracefully without crashing
    sysEventReceiver->SaveInsightIntentInfos("", TEST_MODULE_NAME, ver, MAIN_USER_ID);
    EXPECT_EQ(sysEventReceiver->lastUserId_, 0);

    sysEventReceiver->SaveInsightIntentInfos(TEST_BUNDLE_NAME, "", ver, MAIN_USER_ID);
    EXPECT_EQ(sysEventReceiver->lastUserId_, 0);

    // Test scenario 2: Use invalid userId
    sysEventReceiver->SaveInsightIntentInfos(TEST_BUNDLE_NAME, TEST_MODULE_NAME, ver, INVALID_USER_ID);
    EXPECT_EQ(sysEventReceiver->lastUserId_, 0);

    // Test scenario 3: Use non-existent bundle (GetJsonProfile will fail, triggering DeleteInsightIntent)
    sysEventReceiver->SaveInsightIntentInfos("com.nonexistent.bundle", TEST_MODULE_NAME, ver, MAIN_USER_ID);
    EXPECT_EQ(sysEventReceiver->lastUserId_, 0);

    // Test scenario 4: Multi-module scenario with partial failures
    sysEventReceiver->SaveInsightIntentInfos(TEST_BUNDLE_NAME, MULTI_MODULE_NAME, ver, MAIN_USER_ID);
    EXPECT_EQ(sysEventReceiver->lastUserId_, 0);
}

/**
 * @tc.name: InsightIntentSysEventReceiverTest_SaveInsightIntentInfos_EdgeCases_0009
 * @tc.desc: Test SaveInsightIntentInfos edge cases that trigger DeleteInsightIntent
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentSysEventReceiverTest, SaveInsightIntentInfos_EdgeCases_0009, TestSize.Level1)
{
    EventFwk::CommonEventSubscribeInfo subscribeInfo;
    uint32_t ver = 0;
    auto sysEventReceiver = std::make_shared<AbilityRuntime::InsightIntentSysEventReceiver>(subscribeInfo);

    // Test boundary condition: versionCode is 0
    sysEventReceiver->SaveInsightIntentInfos(TEST_BUNDLE_NAME, TEST_MODULE_NAME, ver, MAIN_USER_ID);
    EXPECT_EQ(sysEventReceiver->lastUserId_, 0);

    // Test boundary condition: versionCode is maximum value
    uint32_t maxVer = UINT32_MAX;
    sysEventReceiver->SaveInsightIntentInfos(TEST_BUNDLE_NAME, TEST_MODULE_NAME, maxVer, MAIN_USER_ID);
    EXPECT_EQ(sysEventReceiver->lastUserId_, 0);

    // Test module name with special characters
    sysEventReceiver->SaveInsightIntentInfos(TEST_BUNDLE_NAME, "module@#$%", ver, MAIN_USER_ID);
    EXPECT_EQ(sysEventReceiver->lastUserId_, 0);

    // Test extra long module name
    std::string longModuleName(1000, 'a');
    sysEventReceiver->SaveInsightIntentInfos(TEST_BUNDLE_NAME, longModuleName, ver, MAIN_USER_ID);
    EXPECT_EQ(sysEventReceiver->lastUserId_, 0);
}

/**
 * @tc.name: InsightIntentSysEventReceiverTest_DeleteInsightIntent_MultiUser_0010
 * @tc.desc: Test DeleteInsightIntent with multiple user scenarios
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentSysEventReceiverTest, DeleteInsightIntent_MultiUser_0010, TestSize.Level1)
{
    EventFwk::CommonEventSubscribeInfo subscribeInfo;
    uint32_t ver = 100;
    auto sysEventReceiver = std::make_shared<AbilityRuntime::InsightIntentSysEventReceiver>(subscribeInfo);

    // Test DeleteInsightIntent behavior in multi-user scenarios
    sysEventReceiver->SaveInsightIntentInfos(TEST_BUNDLE_NAME, TEST_MODULE_NAME, ver, MAIN_USER_ID);
    EXPECT_EQ(sysEventReceiver->lastUserId_, 0);

    sysEventReceiver->SaveInsightIntentInfos(TEST_BUNDLE_NAME, TEST_MODULE_NAME, ver, OTHER_USER_ID);
    EXPECT_EQ(sysEventReceiver->lastUserId_, 0);

    // Test case when userId is 0
    sysEventReceiver->SaveInsightIntentInfos(TEST_BUNDLE_NAME, TEST_MODULE_NAME, ver, ZERO_USER_ID);
    EXPECT_EQ(sysEventReceiver->lastUserId_, 0);
}
} // namespace AbilityRuntime
} // namespace OHOS
