/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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
#include "event_report.h"
#undef private

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AAFwk {
class EventReportTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp() override;
    void TearDown() override;
};

void EventReportTest::SetUpTestCase(void)
{}
void EventReportTest::TearDownTestCase(void)
{}
void EventReportTest::SetUp()
{}
void EventReportTest::TearDown()
{}

/**
 * @tc.name: ConvertEventName_0100
 * @tc.desc: Check ConvertEventName Test
 * @tc.type: FUNC
 * @tc.require: issueI67H0J
 */
HWTEST_F(EventReportTest, ConvertEventName_0100, TestSize.Level2)
{
    EventName eventName = static_cast<EventName>(-1);
    EXPECT_EQ(EventReport::ConvertEventName(eventName), "INVALIDEVENTNAME");
    // fault event
    EXPECT_EQ(EventReport::ConvertEventName(EventName::START_ABILITY_ERROR), "START_ABILITY_ERROR");
    EXPECT_EQ(EventReport::ConvertEventName(EventName::TERMINATE_ABILITY_ERROR), "TERMINATE_ABILITY_ERROR");
    EXPECT_EQ(EventReport::ConvertEventName(EventName::START_EXTENSION_ERROR), "START_EXTENSION_ERROR");
    EXPECT_EQ(EventReport::ConvertEventName(EventName::STOP_EXTENSION_ERROR), "STOP_EXTENSION_ERROR");
    EXPECT_EQ(EventReport::ConvertEventName(EventName::CONNECT_SERVICE_ERROR), "CONNECT_SERVICE_ERROR");
    EXPECT_EQ(EventReport::ConvertEventName(EventName::DISCONNECT_SERVICE_ERROR), "DISCONNECT_SERVICE_ERROR");
    // ability behavior event
    EXPECT_EQ(EventReport::ConvertEventName(EventName::START_ABILITY), "START_ABILITY");
    EXPECT_EQ(EventReport::ConvertEventName(EventName::TERMINATE_ABILITY), "TERMINATE_ABILITY");
    EXPECT_EQ(EventReport::ConvertEventName(EventName::CLOSE_ABILITY), "CLOSE_ABILITY");
    EXPECT_EQ(EventReport::ConvertEventName(EventName::ABILITY_ONFOREGROUND), "ABILITY_ONFOREGROUND");
    EXPECT_EQ(EventReport::ConvertEventName(EventName::ABILITY_ONBACKGROUND), "ABILITY_ONBACKGROUND");
    EXPECT_EQ(EventReport::ConvertEventName(EventName::ABILITY_ONACTIVE), "ABILITY_ONACTIVE");
    EXPECT_EQ(EventReport::ConvertEventName(EventName::ABILITY_ONINACTIVE), "ABILITY_ONINACTIVE");
    // serviceExtensionAbility behavior event
    EXPECT_EQ(EventReport::ConvertEventName(EventName::START_SERVICE), "START_SERVICE");
    EXPECT_EQ(EventReport::ConvertEventName(EventName::STOP_SERVICE), "STOP_SERVICE");
    EXPECT_EQ(EventReport::ConvertEventName(EventName::CONNECT_SERVICE), "CONNECT_SERVICE");
    EXPECT_EQ(EventReport::ConvertEventName(EventName::DISCONNECT_SERVICE), "DISCONNECT_SERVICE");
    EXPECT_EQ(EventReport::ConvertEventName(EventName::START_ABILITY_OTHER_EXTENSION),
        "START_ABILITY_OTHER_EXTENSION");
    // app behavior event
    EXPECT_EQ(EventReport::ConvertEventName(EventName::APP_ATTACH), "APP_ATTACH");
    EXPECT_EQ(EventReport::ConvertEventName(EventName::APP_LAUNCH), "APP_LAUNCH");
    EXPECT_EQ(EventReport::ConvertEventName(EventName::APP_FOREGROUND), "APP_FOREGROUND");
    EXPECT_EQ(EventReport::ConvertEventName(EventName::APP_BACKGROUND), "APP_BACKGROUND");
    EXPECT_EQ(EventReport::ConvertEventName(EventName::APP_TERMINATE), "APP_TERMINATE");
    EXPECT_EQ(EventReport::ConvertEventName(EventName::PROCESS_START), "PROCESS_START");
    EXPECT_EQ(EventReport::ConvertEventName(EventName::PROCESS_EXIT), "PROCESS_EXIT");
    EXPECT_EQ(EventReport::ConvertEventName(EventName::DRAWN_COMPLETED), "DRAWN_COMPLETED");
    EXPECT_EQ(EventReport::ConvertEventName(EventName::APP_STARTUP_TYPE), "APP_STARTUP_TYPE");
    EXPECT_EQ(EventReport::ConvertEventName(EventName::PROCESS_START_FAILED), "PROCESS_START_FAILED");
    // key behavior event
    EXPECT_EQ(EventReport::ConvertEventName(EventName::GRANT_URI_PERMISSION), "GRANT_URI_PERMISSION");
    EXPECT_EQ(EventReport::ConvertEventName(EventName::FA_SHOW_ON_LOCK), "FA_SHOW_ON_LOCK");
    EXPECT_EQ(EventReport::ConvertEventName(EventName::START_PRIVATE_ABILITY), "START_PRIVATE_ABILITY");
    EXPECT_EQ(EventReport::ConvertEventName(EventName::RESTART_PROCESS_BY_SAME_APP), "RESTART_PROCESS_BY_SAME_APP");
    EXPECT_EQ(EventReport::ConvertEventName(EventName::START_STANDARD_ABILITIES), "START_STANDARD_ABILITIES");
    // atomic service event
    EXPECT_EQ(EventReport::ConvertEventName(EventName::CREATE_ATOMIC_SERVICE_PROCESS),
        "CREATE_ATOMIC_SERVICE_PROCESS");
    EXPECT_EQ(EventReport::ConvertEventName(EventName::ATOMIC_SERVICE_DRAWN_COMPLETE),
        "ATOMIC_SERVICE_DRAWN_COMPLETE");
    EXPECT_EQ(EventReport::ConvertEventName(EventName::SHARE_UNPRIVILEGED_FILE_URI),
        "SHARE_UNPRIVILEGED_FILE_URI");
    EXPECT_EQ(EventReport::ConvertEventName(EventName::USER_DATA_SIZE), "USER_DATA_SIZE");
}

/**
 * @tc.name: SendAppEvent_0100
 * @tc.desc: Check SendAppEvent Test
 * @tc.type: FUNC
 * @tc.require: issueI67H0J
 */
HWTEST_F(EventReportTest, SendAppEvent_0100, TestSize.Level2)
{
    EventName eventName = static_cast<EventName>(-1);
    EXPECT_EQ(EventReport::ConvertEventName(eventName), "INVALIDEVENTNAME");
    HiSysEventEventType type = HISYSEVENT_BEHAVIOR;
    EventInfo eventInfo;
    EventReport::SendAppEvent(eventName, type, eventInfo);
}

/**
 * @tc.name: SendAppEvent_0200
 * @tc.desc: Check SendAppEvent Test
 * @tc.type: FUNC
 * @tc.require: issueI6HXXS
 */
HWTEST_F(EventReportTest, SendAppEvent_0200, TestSize.Level2)
{
    EventName eventName = EventName::APP_STARTUP_TYPE;
    EXPECT_EQ(EventReport::ConvertEventName(eventName), "APP_STARTUP_TYPE");
    HiSysEventEventType type = HISYSEVENT_BEHAVIOR;
    EventInfo eventInfo;
    EventReport::SendAppEvent(eventName, type, eventInfo);
}

/**
 * @tc.name: SendAppEvent_0300
 * @tc.desc: Check SendAppEvent Test
 * @tc.type: FUNC
 * @tc.require: issueI6HXXS
 */
HWTEST_F(EventReportTest, SendAppEvent_0300, TestSize.Level2)
{
    EventName eventName = EventName::DRAWN_COMPLETED;
    EXPECT_EQ(EventReport::ConvertEventName(eventName), "DRAWN_COMPLETED");
    HiSysEventEventType type = HISYSEVENT_BEHAVIOR;
    EventInfo eventInfo;
    eventInfo.extensionType = 0;
    EventReport::SendAppEvent(eventName, type, eventInfo);
}

/**
 * @tc.name: SendAppEvent_0400
 * @tc.desc: Check SendAppEvent Test
 * @tc.type: FUNC
 * @tc.require: issueI6HXXS
 */
HWTEST_F(EventReportTest, SendAppEvent_0400, TestSize.Level2)
{
    EventName eventName = EventName::APP_ATTACH;
    EXPECT_EQ(EventReport::ConvertEventName(eventName), "APP_ATTACH");
    HiSysEventEventType type = HISYSEVENT_BEHAVIOR;
    EventInfo eventInfo;
    EventReport::SendAppEvent(eventName, type, eventInfo);
}

/**
 * @tc.name: SendAbilityEvent_0100
 * @tc.desc: Check SendAbilityEvent Test
 * @tc.type: FUNC
 * @tc.require: issueI67H0J
 */
HWTEST_F(EventReportTest, SendAbilityEvent_0100, TestSize.Level2)
{
    EventName eventName = static_cast<EventName>(-1);
    EXPECT_EQ(EventReport::ConvertEventName(eventName), "INVALIDEVENTNAME");
    HiSysEventEventType type = HISYSEVENT_BEHAVIOR;
    EventInfo eventInfo;
    EventReport::SendAbilityEvent(eventName, type, eventInfo);
}

/**
 * @tc.name: SendAbilityEvent_0200
 * @tc.desc: Check SendAbilityEvent Test
 * @tc.type: FUNC
 * @tc.require: issueI67H0J
 */
HWTEST_F(EventReportTest, SendAbilityEvent_0200, TestSize.Level2)
{
    EventName eventName = EventName::START_ABILITY_ERROR;
    EXPECT_EQ(EventReport::ConvertEventName(eventName), "START_ABILITY_ERROR");
    HiSysEventEventType type = HISYSEVENT_FAULT;
    EventInfo eventInfo;
    EventReport::SendAbilityEvent(eventName, type, eventInfo);
}

/**
 * @tc.name: SendAbilityEvent_0300
 * @tc.desc: Check SendAbilityEvent Test
 * @tc.type: FUNC
 * @tc.require: issueI67H0J
 */
HWTEST_F(EventReportTest, SendAbilityEvent_0300, TestSize.Level2)
{
    EventName eventName = EventName::TERMINATE_ABILITY_ERROR;
    EXPECT_EQ(EventReport::ConvertEventName(eventName), "TERMINATE_ABILITY_ERROR");
    HiSysEventEventType type = HISYSEVENT_FAULT;
    EventInfo eventInfo;
    EventReport::SendAbilityEvent(eventName, type, eventInfo);
}

/**
 * @tc.name: SendAbilityEvent_0400
 * @tc.desc: Check SendAbilityEvent Test
 * @tc.type: FUNC
 * @tc.require: issueI67H0J
 */
HWTEST_F(EventReportTest, SendAbilityEvent_0400, TestSize.Level2)
{
    EventName eventName = EventName::START_ABILITY;
    EXPECT_EQ(EventReport::ConvertEventName(eventName), "START_ABILITY");
    HiSysEventEventType type = HISYSEVENT_BEHAVIOR;
    EventInfo eventInfo;
    EventReport::SendAbilityEvent(eventName, type, eventInfo);
}

/**
 * @tc.name: SendAbilityEvent_0500
 * @tc.desc: Check SendAbilityEvent Test
 * @tc.type: FUNC
 * @tc.require: issueI67H0J
 */
HWTEST_F(EventReportTest, SendAbilityEvent_0500, TestSize.Level2)
{
    EventName eventName = EventName::TERMINATE_ABILITY;
    EXPECT_EQ(EventReport::ConvertEventName(eventName), "TERMINATE_ABILITY");
    HiSysEventEventType type = HISYSEVENT_BEHAVIOR;
    EventInfo eventInfo;
    EventReport::SendAbilityEvent(eventName, type, eventInfo);
}

/**
 * @tc.name: SendAbilityEvent_0600
 * @tc.desc: Check SendAbilityEvent Test
 * @tc.type: FUNC
 * @tc.require: issueI67H0J
 */
HWTEST_F(EventReportTest, SendAbilityEvent_0600, TestSize.Level2)
{
    EventName eventName = EventName::CLOSE_ABILITY;
    EXPECT_EQ(EventReport::ConvertEventName(eventName), "CLOSE_ABILITY");
    HiSysEventEventType type = HISYSEVENT_BEHAVIOR;
    EventInfo eventInfo;
    EventReport::SendAbilityEvent(eventName, type, eventInfo);
}

/**
 * @tc.name: SendAbilityEvent_0700
 * @tc.desc: Check SendAbilityEvent Test
 * @tc.type: FUNC
 * @tc.require: issueI67H0J
 */
HWTEST_F(EventReportTest, SendAbilityEvent_0700, TestSize.Level2)
{
    EventName eventName = EventName::ABILITY_ONFOREGROUND;
    EXPECT_EQ(EventReport::ConvertEventName(eventName), "ABILITY_ONFOREGROUND");
    HiSysEventEventType type = HISYSEVENT_BEHAVIOR;
    EventInfo eventInfo;
    EventReport::SendAbilityEvent(eventName, type, eventInfo);
}

/**
 * @tc.name: SendAbilityEvent_0800
 * @tc.desc: Check SendAbilityEvent Test
 * @tc.type: FUNC
 * @tc.require: issueI67H0J
 */
HWTEST_F(EventReportTest, SendAbilityEvent_0800, TestSize.Level2)
{
    EventName eventName = EventName::ABILITY_ONBACKGROUND;
    EXPECT_EQ(EventReport::ConvertEventName(eventName), "ABILITY_ONBACKGROUND");
    HiSysEventEventType type = HISYSEVENT_BEHAVIOR;
    EventInfo eventInfo;
    EventReport::SendAbilityEvent(eventName, type, eventInfo);
}

/**
 * @tc.name: SendAbilityEvent_0900
 * @tc.desc: Check SendAbilityEvent Test
 * @tc.type: FUNC
 * @tc.require: issueI67H0J
 */
HWTEST_F(EventReportTest, SendAbilityEvent_0900, TestSize.Level2)
{
    EventName eventName = EventName::ABILITY_ONACTIVE;
    std::string name  = "ABILITY_ONACTIVE";
    EXPECT_EQ(EventReport::ConvertEventName(eventName), name);
    HiSysEventEventType type = HISYSEVENT_BEHAVIOR;
    EventInfo eventInfo;
    EventReport::SendAbilityEvent(eventName, type, eventInfo);
}

/**
 * @tc.name: SendAbilityEvent_1000
 * @tc.desc: Check SendAbilityEvent Test
 * @tc.type: FUNC
 * @tc.require: issueI6A12D
 */
HWTEST_F(EventReportTest, SendAbilityEvent_1000, TestSize.Level2)
{
    EventName eventName = EventName::ABILITY_ONINACTIVE;
    std::string name  = "ABILITY_ONINACTIVE";
    EXPECT_EQ(EventReport::ConvertEventName(eventName), name);
    HiSysEventEventType type = HISYSEVENT_BEHAVIOR;
    EventInfo eventInfo;
    EventReport::SendAbilityEvent(eventName, type, eventInfo);
}

/**
 * @tc.name: SendAbilityEvent_1100
 * @tc.desc: Check SendAbilityEvent Test
 * @tc.type: FUNC
 * @tc.require: issueI67H0J
 */
HWTEST_F(EventReportTest, SendAbilityEvent_1100, TestSize.Level2)
{
    EventName eventName = EventName::DISCONNECT_SERVICE;
    std::string name  = "DISCONNECT_SERVICE";
    EXPECT_EQ(EventReport::ConvertEventName(eventName), name);
    HiSysEventEventType type = HISYSEVENT_BEHAVIOR;
    EventInfo eventInfo;
    EventReport::SendAbilityEvent(eventName, type, eventInfo);
}

/**
 * @tc.name: SendExtensionEvent_0100
 * @tc.desc: Check SendExtensionEvent Test
 * @tc.type: FUNC
 * @tc.require: issueI67H0J
 */
HWTEST_F(EventReportTest, SendExtensionEvent_0100, TestSize.Level2)
{
    EventName eventName = static_cast<EventName>(-1);
    EXPECT_EQ(EventReport::ConvertEventName(eventName), "INVALIDEVENTNAME");
    HiSysEventEventType type = HISYSEVENT_BEHAVIOR;
    EventInfo eventInfo;
    EventReport::SendExtensionEvent(eventName, type, eventInfo);
}

/**
 * @tc.name: SendExtensionEvent_0200
 * @tc.desc: Check SendExtensionEvent Test
 * @tc.type: FUNC
 * @tc.require: issueI67H0J
 */
HWTEST_F(EventReportTest, SendExtensionEvent_0200, TestSize.Level2)
{
    EventName eventName = EventName::START_EXTENSION_ERROR;
    EXPECT_EQ(EventReport::ConvertEventName(eventName), "START_EXTENSION_ERROR");
    HiSysEventEventType type = HISYSEVENT_FAULT;
    EventInfo eventInfo;
    EventReport::SendExtensionEvent(eventName, type, eventInfo);
}

/**
 * @tc.name: SendExtensionEvent_0300
 * @tc.desc: Check SendExtensionEvent Test
 * @tc.type: FUNC
 * @tc.require: issueI67H0J
 */
HWTEST_F(EventReportTest, SendExtensionEvent_0300, TestSize.Level2)
{
    EventName eventName = EventName::STOP_EXTENSION_ERROR;
    EXPECT_EQ(EventReport::ConvertEventName(eventName), "STOP_EXTENSION_ERROR");
    HiSysEventEventType type = HISYSEVENT_FAULT;
    EventInfo eventInfo;
    EventReport::SendExtensionEvent(eventName, type, eventInfo);
}

/**
 * @tc.name: SendExtensionEvent_0400
 * @tc.desc: Check SendExtensionEvent Test
 * @tc.type: FUNC
 * @tc.require: issueI67H0J
 */
HWTEST_F(EventReportTest, SendExtensionEvent_0400, TestSize.Level2)
{
    EventName eventName = EventName::CONNECT_SERVICE_ERROR;
    EXPECT_EQ(EventReport::ConvertEventName(eventName), "CONNECT_SERVICE_ERROR");
    HiSysEventEventType type = HISYSEVENT_FAULT;
    EventInfo eventInfo;
    EventReport::SendExtensionEvent(eventName, type, eventInfo);
}

/**
 * @tc.name: SendExtensionEvent_0500
 * @tc.desc: Check SendExtensionEvent Test
 * @tc.type: FUNC
 * @tc.require: issueI67H0J
 */
HWTEST_F(EventReportTest, SendExtensionEvent_0500, TestSize.Level2)
{
    EventName eventName = EventName::DISCONNECT_SERVICE_ERROR;
    EXPECT_EQ(EventReport::ConvertEventName(eventName), "DISCONNECT_SERVICE_ERROR");
    HiSysEventEventType type = HISYSEVENT_FAULT;
    EventInfo eventInfo;
    EventReport::SendExtensionEvent(eventName, type, eventInfo);
}

/**
 * @tc.name: SendExtensionEvent_0600
 * @tc.desc: Check SendExtensionEvent Test
 * @tc.type: FUNC
 * @tc.require: issueI67H0J
 */
HWTEST_F(EventReportTest, SendExtensionEvent_0600, TestSize.Level2)
{
    EventName eventName = EventName::START_SERVICE;
    EXPECT_EQ(EventReport::ConvertEventName(eventName), "START_SERVICE");
    HiSysEventEventType type = HISYSEVENT_FAULT;
    EventInfo eventInfo;
    EventReport::SendExtensionEvent(eventName, type, eventInfo);
}

/**
 * @tc.name: SendKeyEvent_0100
 * @tc.desc: Check SendKeyEvent Test
 * @tc.type: FUNC
 * @tc.require: issueI99FZY
 */
HWTEST_F(EventReportTest, SendKeyEvent_0100, TestSize.Level2)
{
    EventName eventName = static_cast<EventName>(-1);
    EXPECT_EQ(EventReport::ConvertEventName(eventName), "INVALIDEVENTNAME");
    HiSysEventEventType type = HISYSEVENT_FAULT;
    EventInfo eventInfo;
    EventReport::SendExtensionEvent(eventName, type, eventInfo);
}

/**
 * @tc.name: SendKeyEvent_0200
 * @tc.desc: Check SendKeyEvent Test
 * @tc.type: FUNC
 * @tc.require: issueI99FZY
 */
HWTEST_F(EventReportTest, SendKeyEvent_0200, TestSize.Level2)
{
    EventName eventName = EventName::GRANT_URI_PERMISSION;
    EXPECT_EQ(EventReport::ConvertEventName(eventName), "GRANT_URI_PERMISSION");
    HiSysEventEventType type = HISYSEVENT_BEHAVIOR;
    EventInfo eventInfo;
    EventReport::SendExtensionEvent(eventName, type, eventInfo);
}

/**
 * @tc.name: SendKeyEvent_0300
 * @tc.desc: Check SendKeyEvent Test
 * @tc.type: FUNC
 * @tc.require: issueI99FZY
 */
HWTEST_F(EventReportTest, SendKeyEvent_0300, TestSize.Level2)
{
    EventName eventName = EventName::FA_SHOW_ON_LOCK;
    EXPECT_EQ(EventReport::ConvertEventName(eventName), "FA_SHOW_ON_LOCK");
    HiSysEventEventType type = HISYSEVENT_BEHAVIOR;
    EventInfo eventInfo;
    EventReport::SendExtensionEvent(eventName, type, eventInfo);
}

/**
 * @tc.name: SendKeyEvent_0400
 * @tc.desc: Check SendKeyEvent Test
 * @tc.type: FUNC
 * @tc.require: issueI99FZY
 */
HWTEST_F(EventReportTest, SendKeyEvent_0400, TestSize.Level2)
{
    EventName eventName = EventName::START_PRIVATE_ABILITY;
    EXPECT_EQ(EventReport::ConvertEventName(eventName), "START_PRIVATE_ABILITY");
    HiSysEventEventType type = HISYSEVENT_BEHAVIOR;
    EventInfo eventInfo;
    EventReport::SendExtensionEvent(eventName, type, eventInfo);
}

/**
 * @tc.name: SendKeyEvent_0500
 * @tc.desc: Check SendKeyEvent Test
 * @tc.type: FUNC
 * @tc.require: issueI99FZY
 */
HWTEST_F(EventReportTest, SendKeyEvent_0500, TestSize.Level2)
{
    EventName eventName = EventName::RESTART_PROCESS_BY_SAME_APP;
    EXPECT_EQ(EventReport::ConvertEventName(eventName), "RESTART_PROCESS_BY_SAME_APP");
    HiSysEventEventType type = HISYSEVENT_BEHAVIOR;
    EventInfo eventInfo;
    EventReport::SendExtensionEvent(eventName, type, eventInfo);
}

/**
 * @tc.name: SendKeyEvent_0600
 * @tc.desc: Check SendKeyEvent Test
 * @tc.type: FUNC
 * @tc.require: issueI99FZY
 */
HWTEST_F(EventReportTest, SendKeyEvent_0600, TestSize.Level2)
{
    EventName eventName = EventName::START_SERVICE;
    EXPECT_EQ(EventReport::ConvertEventName(eventName), "START_SERVICE");
    HiSysEventEventType type = HISYSEVENT_BEHAVIOR;
    EventInfo eventInfo;
    EventReport::SendExtensionEvent(eventName, type, eventInfo);
}

/**
 * @tc.name: SendAppLaunchEvent_0100
 * @tc.desc: Check SendAppLaunchEvent Test
 * @tc.type: FUNC
 * @tc.require: issueI99FZY
 */
HWTEST_F(EventReportTest, SendAppLaunchEvent_0100, TestSize.Level2)
{
    EventName eventName = static_cast<EventName>(-1);
    EXPECT_EQ(EventReport::ConvertEventName(eventName), "INVALIDEVENTNAME");
    EventInfo eventInfo;
    EventReport::SendAppLaunchEvent(eventName, eventInfo);
    eventName = EventName::APP_LAUNCH;
    EXPECT_EQ(EventReport::ConvertEventName(eventName), "APP_LAUNCH");
    EventReport::SendAppLaunchEvent(eventName, eventInfo);
}

/**
 * @tc.name: SendAppForegroundEvent_0100
 * @tc.desc: Check SendAppForegroundEvent Test
 * @tc.type: FUNC
 * @tc.require: issueI99FZY
 */
HWTEST_F(EventReportTest, SendAppForegroundEvent_0100, TestSize.Level2)
{
    EventName eventName = static_cast<EventName>(-1);
    EXPECT_EQ(EventReport::ConvertEventName(eventName), "INVALIDEVENTNAME");
    EventInfo eventInfo;
    EventReport::SendAppForegroundEvent(eventName, eventInfo);
    eventName = EventName::APP_FOREGROUND;
    EXPECT_EQ(EventReport::ConvertEventName(eventName), "APP_FOREGROUND");
    EventReport::SendAppForegroundEvent(eventName, eventInfo);
}

/**
 * @tc.name: SendAppBackgroundEvent_0100
 * @tc.desc: Check SendAppBackgroundEvent Test
 * @tc.type: FUNC
 * @tc.require: issueI99FZY
 */
HWTEST_F(EventReportTest, SendAppBackgroundEvent_0100, TestSize.Level2)
{
    EventName eventName = static_cast<EventName>(-1);
    EXPECT_EQ(EventReport::ConvertEventName(eventName), "INVALIDEVENTNAME");
    EventInfo eventInfo;
    EventReport::SendAppBackgroundEvent(eventName, eventInfo);
    eventName = EventName::APP_BACKGROUND;
    EXPECT_EQ(EventReport::ConvertEventName(eventName), "APP_BACKGROUND");
    EventReport::SendAppBackgroundEvent(eventName, eventInfo);
}

/**
 * @tc.name: SendProcessStartEvent_0100
 * @tc.desc: Check SendProcessStartEvent Test
 * @tc.type: FUNC
 * @tc.require: issueI99FZY
 */
HWTEST_F(EventReportTest, SendProcessStartEvent_0100, TestSize.Level2)
{
    EventName eventName = static_cast<EventName>(-1);
    EXPECT_EQ(EventReport::ConvertEventName(eventName), "INVALIDEVENTNAME");
    EventInfo eventInfo;
    EventReport::SendProcessStartEvent(eventName, eventInfo);
    eventName = EventName::PROCESS_START;
    EXPECT_EQ(EventReport::ConvertEventName(eventName), "PROCESS_START");
    EventReport::SendProcessStartEvent(eventName, eventInfo);
    eventInfo.extensionType = 0;
    EventReport::SendProcessStartEvent(eventName, eventInfo);
}

/**
 * @tc.name: SendProcessStartFailedEvent_0100
 * @tc.desc: Check SendProcessStartFailedEvent Test
 * @tc.type: FUNC
 * @tc.require: issueI99FZY
 */
HWTEST_F(EventReportTest, SendProcessStartFailedEvent_0100, TestSize.Level2)
{
    EventName eventName = static_cast<EventName>(-1);
    EXPECT_EQ(EventReport::ConvertEventName(eventName), "INVALIDEVENTNAME");
    EventInfo eventInfo;
    EventReport::SendProcessStartFailedEvent(eventName, eventInfo);
    eventName = EventName::PROCESS_START_FAILED;
    EventReport::SendProcessStartFailedEvent(eventName, eventInfo);
    eventInfo.extensionType = 0;
    EventReport::SendProcessStartFailedEvent(eventName, eventInfo);
    EXPECT_EQ(EventReport::ConvertEventName(eventName), "PROCESS_START_FAILED");
}

/**
 * @tc.name: SendProcessExitEvent_0100
 * @tc.desc: Check SendProcessExitEvent Test
 * @tc.type: FUNC
 * @tc.require: issueI99FZY
 */
HWTEST_F(EventReportTest, SendProcessExitEvent_0100, TestSize.Level2)
{
    EventName eventName = static_cast<EventName>(-1);
    EXPECT_EQ(EventReport::ConvertEventName(eventName), "INVALIDEVENTNAME");
    EventInfo eventInfo;
    EventReport::SendProcessExitEvent(eventName, eventInfo);
    eventName = EventName::PROCESS_EXIT;
    EXPECT_EQ(EventReport::ConvertEventName(eventName), "PROCESS_EXIT");
    EventReport::SendProcessExitEvent(eventName, eventInfo);
}

/**
 * @tc.name: SendProcessExitEvent_0200
 * @tc.desc: Check SendProcessExitEvent Test
 * @tc.type: FUNC
 */
HWTEST_F(EventReportTest, SendProcessExitEvent_0200, TestSize.Level1)
{
    EventInfo eventInfo;
    EventName eventName = EventName::PROCESS_EXIT;
    eventInfo.exitReason = 0;
    EventReport::SendProcessExitEvent(eventName, eventInfo);
    EXPECT_EQ(EventReport::ConvertEventName(eventName), "PROCESS_EXIT");
}

/**
 * @tc.name: SendStartServiceEvent_0100
 * @tc.desc: Check SendStartServiceEvent Test
 * @tc.type: FUNC
 * @tc.require: issueI99FZY
 */
HWTEST_F(EventReportTest, SendStartServiceEvent_0100, TestSize.Level2)
{
    EventName eventName = static_cast<EventName>(-1);
    EXPECT_EQ(EventReport::ConvertEventName(eventName), "INVALIDEVENTNAME");
    EventInfo eventInfo;
    EventReport::SendStartServiceEvent(eventName, eventInfo);
    eventName = EventName::START_SERVICE;
    EXPECT_EQ(EventReport::ConvertEventName(eventName), "START_SERVICE");
    EventReport::SendStartServiceEvent(eventName, eventInfo);
}

/**
 * @tc.name: SendStopServiceEvent_0100
 * @tc.desc: Check SendStopServiceEvent Test
 * @tc.type: FUNC
 * @tc.require: issueI99FZY
 */
HWTEST_F(EventReportTest, SendStopServiceEvent_0100, TestSize.Level2)
{
    EventName eventName = static_cast<EventName>(-1);
    EXPECT_EQ(EventReport::ConvertEventName(eventName), "INVALIDEVENTNAME");
    EventInfo eventInfo;
    EventReport::SendStopServiceEvent(eventName, eventInfo);
    eventName = EventName::STOP_SERVICE;
    EXPECT_EQ(EventReport::ConvertEventName(eventName), "STOP_SERVICE");
    EventReport::SendStopServiceEvent(eventName, eventInfo);
}

/**
 * @tc.name: SendConnectServiceEvent_0100
 * @tc.desc: Check SendConnectServiceEvent Test
 * @tc.type: FUNC
 * @tc.require: issueI99FZY
 */
HWTEST_F(EventReportTest, SendConnectServiceEvent_0100, TestSize.Level2)
{
    EventName eventName = static_cast<EventName>(-1);
    EXPECT_EQ(EventReport::ConvertEventName(eventName), "INVALIDEVENTNAME");
    EventInfo eventInfo;
    EventReport::SendConnectServiceEvent(eventName, eventInfo);
    eventName = EventName::CONNECT_SERVICE;
    EXPECT_EQ(EventReport::ConvertEventName(eventName), "CONNECT_SERVICE");
    EventReport::SendConnectServiceEvent(eventName, eventInfo);
}

/**
 * @tc.name: SendDisconnectServiceEvent_0100
 * @tc.desc: Check SendDisconnectServiceEvent Test
 * @tc.type: FUNC
 * @tc.require: issueI99FZY
 */
HWTEST_F(EventReportTest, SendDisconnectServiceEvent_0100, TestSize.Level2)
{
    EventName eventName = static_cast<EventName>(-1);
    EXPECT_EQ(EventReport::ConvertEventName(eventName), "INVALIDEVENTNAME");
    EventInfo eventInfo;
    EventReport::SendDisconnectServiceEvent(eventName, eventInfo);
    eventName = EventName::DISCONNECT_SERVICE;
    EXPECT_EQ(EventReport::ConvertEventName(eventName), "DISCONNECT_SERVICE");
    EventReport::SendDisconnectServiceEvent(eventName, eventInfo);
}

/**
 * @tc.name: SendAtomicServiceEvent_0100
 * @tc.desc: Check SendAtomicServiceEvent Test
 * @tc.type: FUNC
 */
HWTEST_F(EventReportTest, SendAtomicServiceEvent_0100, TestSize.Level2)
{
    EventName eventName = static_cast<EventName>(-1);
    EXPECT_EQ(EventReport::ConvertEventName(eventName), "INVALIDEVENTNAME");
    EventInfo eventInfo;
    HiSysEventEventType type = HISYSEVENT_BEHAVIOR;
    EventReport::SendAtomicServiceEvent(eventName, type, eventInfo);

    eventName = EventName::CREATE_ATOMIC_SERVICE_PROCESS;
    EXPECT_EQ(EventReport::ConvertEventName(eventName), "CREATE_ATOMIC_SERVICE_PROCESS");
    EventReport::SendAtomicServiceEvent(eventName, type, eventInfo);

    eventName = EventName::ATOMIC_SERVICE_DRAWN_COMPLETE;
    EXPECT_EQ(EventReport::ConvertEventName(eventName), "ATOMIC_SERVICE_DRAWN_COMPLETE");
    EventReport::SendAtomicServiceEvent(eventName, type, eventInfo);
}

/**
 * @tc.name: SendReportDataPartitionUsageEvent_0100
 * @tc.desc: Check SendReportDataPartitionUsageEvent Test
 * @tc.type: FUNC
 */
HWTEST_F(EventReportTest, SendReportDataPartitionUsageEvent_0100, TestSize.Level1)
{
    EventName eventName = static_cast<EventName>(-1);
    EXPECT_EQ(EventReport::ConvertEventName(eventName), "INVALIDEVENTNAME");
    EventInfo eventInfo;
    EventReport::SendReportDataPartitionUsageEvent(eventName, HISYSEVENT_STATISTIC, eventInfo);
    eventName = EventName::USER_DATA_SIZE;
    EXPECT_EQ(EventReport::ConvertEventName(eventName), "USER_DATA_SIZE");
    EventReport::SendReportDataPartitionUsageEvent(eventName, HISYSEVENT_STATISTIC, eventInfo);
}

/**
 * @tc.name: SendSnapshotEvent_0100
 * @tc.desc: Check SendSnapshotEvent Test with appVersionName
 * @tc.type: FUNC
 */
HWTEST_F(EventReportTest, SendSnapshotEvent_0100, TestSize.Level2)
{
    EventName eventName = EventName::SNAPSHOT_REPORT;
    EXPECT_EQ(EventReport::ConvertEventName(eventName), "SNAPSHOT_REPORT");

    SnapshotInfo snapshotInfo;
    snapshotInfo.uid = 1001;
    snapshotInfo.bundleName = "com.example.test";
    snapshotInfo.appVersionName = "1.0.0";
    snapshotInfo.snapshotCond = "TestCondition";
    snapshotInfo.snapshotEvent = "ImageStart";
    snapshotInfo.snapshotResult = 0;
    snapshotInfo.snapshotReason = "TestReason";

    EventReport::SendSnapshotEvent(eventName, snapshotInfo);
}

/**
 * @tc.name: SendSnapshotEvent_0200
 * @tc.desc: Check SendSnapshotEvent Test with empty appVersionName
 * @tc.type: FUNC
 */
HWTEST_F(EventReportTest, SendSnapshotEvent_0200, TestSize.Level2)
{
    EventName eventName = EventName::SNAPSHOT_REPORT;
    EXPECT_EQ(EventReport::ConvertEventName(eventName), "SNAPSHOT_REPORT");

    SnapshotInfo snapshotInfo;
    snapshotInfo.uid = 1002;
    snapshotInfo.bundleName = "com.example.test2";
    snapshotInfo.appVersionName = "";
    snapshotInfo.snapshotCond = "";
    snapshotInfo.snapshotEvent = "ImageStartError";
    snapshotInfo.snapshotResult = -1;
    snapshotInfo.snapshotReason = "ErrorReason";

    EventReport::SendSnapshotEvent(eventName, snapshotInfo);
}

/**
 * @tc.name: SendSnapshotEvent_0300
 * @tc.desc: Check SendSnapshotEvent Test with invalid eventName
 * @tc.type: FUNC
 */
HWTEST_F(EventReportTest, SendSnapshotEvent_0300, TestSize.Level2)
{
    EventName eventName = static_cast<EventName>(-1);
    EXPECT_EQ(EventReport::ConvertEventName(eventName), "INVALIDEVENTNAME");

    SnapshotInfo snapshotInfo;
    snapshotInfo.uid = 1003;
    snapshotInfo.bundleName = "com.example.test3";
    snapshotInfo.appVersionName = "2.0.0";
    snapshotInfo.snapshotCond = "TestCondition";
    snapshotInfo.snapshotEvent = "ImageStart";
    snapshotInfo.snapshotResult = 0;
    snapshotInfo.snapshotReason = "TestReason";

    EventReport::SendSnapshotEvent(eventName, snapshotInfo);
}
}  // namespace AAFwk
}  // namespace OHOS
