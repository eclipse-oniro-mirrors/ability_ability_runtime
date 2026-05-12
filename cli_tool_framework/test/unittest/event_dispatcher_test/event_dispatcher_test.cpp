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
#include <string>

#include "cli_tool_manager_scheduler_stub.h"
#include "event_dispatcher.h"

using namespace testing::ext;

namespace OHOS {
namespace CliTool {
namespace {
constexpr int32_t ERR_OK = 0;
constexpr int32_t ERROR_CODE = -1;
constexpr int32_t TEST_CALLER_PID = 1000;
constexpr int32_t TEST_CALLER_PID_SECOND = 1001;
constexpr int32_t TEST_EXIT_CODE = 3;
constexpr int32_t TEST_REPLY_RESULT = 5;
}

class TestScheduler : public CliToolManagerSchedulerStub {
public:
    int32_t SchedulerSessionEvent(const std::string &sessionId, const std::string &subscriptionId,
        const CliToolEvent &event) override
    {
        sessionEventCount++;
        lastSessionId = sessionId;
        lastSubscriptionId = subscriptionId;
        lastEventType = event.type;
        lastEventData = event.eventData;
        lastExitCode = event.exitCode;
        return sessionEventResult;
    }

    int32_t SchedulerInputReplyEvent(const std::string &eventId, int32_t resultCode) override
    {
        inputReplyCount++;
        lastInputEventId = eventId;
        lastInputResult = resultCode;
        return inputReplyResult;
    }

    int32_t SchedulerExecToolReplyEvent(const std::string &eventId, int32_t resultCode,
        const CliSessionInfo &session) override
    {
        execReplyCount++;
        lastExecEventId = eventId;
        lastExecResult = resultCode;
        lastExecSessionId = session.sessionId;
        return execReplyResult;
    }

    int32_t sessionEventResult = ERR_OK;
    int32_t inputReplyResult = ERR_OK;
    int32_t execReplyResult = ERR_OK;
    int32_t sessionEventCount = 0;
    int32_t inputReplyCount = 0;
    int32_t execReplyCount = 0;
    int32_t lastExitCode = 0;
    int32_t lastInputResult = 0;
    int32_t lastExecResult = 0;
    std::string lastSessionId;
    std::string lastSubscriptionId;
    std::string lastEventType;
    std::string lastEventData;
    std::string lastInputEventId;
    std::string lastExecEventId;
    std::string lastExecSessionId;
};

class EventDispatcherTest : public testing::Test {
public:
    void SetUp() override
    {
        EventDispatcher::GetInstance().ClearAll();
    }

    void TearDown() override
    {
        EventDispatcher::GetInstance().ClearAll();
    }
};

/**
 * @tc.name: EventDispatcher_Register_0100
 * @tc.desc: Test scheduler and subscriber registration validation branches
 * @tc.type: FUNC
 */
HWTEST_F(EventDispatcherTest, EventDispatcher_Register_0100, TestSize.Level1)
{
    auto &dispatcher = EventDispatcher::GetInstance();
    sptr<TestScheduler> scheduler = new TestScheduler();

    EXPECT_FALSE(dispatcher.RegisterScheduler(0, scheduler));
    EXPECT_FALSE(dispatcher.RegisterScheduler(TEST_CALLER_PID, nullptr));
    EXPECT_TRUE(dispatcher.RegisterScheduler(TEST_CALLER_PID, scheduler));

    EXPECT_FALSE(dispatcher.RegisterSubscriber("", "subscription", TEST_CALLER_PID));
    EXPECT_FALSE(dispatcher.RegisterSubscriber("session", "", TEST_CALLER_PID));
    EXPECT_FALSE(dispatcher.RegisterSubscriber("session", "subscription", 0));
    EXPECT_FALSE(dispatcher.RegisterSubscriber("session", "subscription", TEST_CALLER_PID + 1));
    EXPECT_TRUE(dispatcher.RegisterSubscriber("session", "subscription", TEST_CALLER_PID));

    EXPECT_FALSE(dispatcher.UnregisterSubscriber("", "subscription", TEST_CALLER_PID));
    EXPECT_FALSE(dispatcher.UnregisterSubscriber("session", "", TEST_CALLER_PID));
    EXPECT_FALSE(dispatcher.UnregisterSubscriber("session", "subscription", 0));
    EXPECT_TRUE(dispatcher.UnregisterSubscriber("unknown-session", "subscription", TEST_CALLER_PID));
    EXPECT_TRUE(dispatcher.UnregisterSubscriber("session", "subscription", TEST_CALLER_PID));
}

/**
 * @tc.name: EventDispatcher_Dispatch_0100
 * @tc.desc: Test dispatch IO, error, exit and reply success branches
 * @tc.type: FUNC
 */
HWTEST_F(EventDispatcherTest, EventDispatcher_Dispatch_0100, TestSize.Level1)
{
    auto &dispatcher = EventDispatcher::GetInstance();
    sptr<TestScheduler> scheduler = new TestScheduler();
    ASSERT_TRUE(dispatcher.RegisterScheduler(TEST_CALLER_PID, scheduler));
    ASSERT_TRUE(dispatcher.RegisterSubscriber("session", "subscription", TEST_CALLER_PID));

    dispatcher.DispatchIOEvent("session", "stdout", "payload");
    EXPECT_EQ(scheduler->sessionEventCount, 1);
    EXPECT_EQ(scheduler->lastSessionId, "session");
    EXPECT_EQ(scheduler->lastSubscriptionId, "subscription");
    EXPECT_EQ(scheduler->lastEventType, "stdout");
    EXPECT_EQ(scheduler->lastEventData, "payload");

    dispatcher.DispatchErrorEvent("session", "error text");
    EXPECT_EQ(scheduler->sessionEventCount, 2);
    EXPECT_EQ(scheduler->lastEventType, "error");
    EXPECT_EQ(scheduler->lastEventData, "error text");

    dispatcher.DispatchExitEvent("session", TEST_EXIT_CODE);
    EXPECT_EQ(scheduler->sessionEventCount, 3);
    EXPECT_EQ(scheduler->lastEventType, "exit");
    EXPECT_EQ(scheduler->lastExitCode, TEST_EXIT_CODE);

    EXPECT_TRUE(dispatcher.DispatchInputReplyEvent(TEST_CALLER_PID, "input-event", TEST_REPLY_RESULT));
    EXPECT_EQ(scheduler->inputReplyCount, 1);
    EXPECT_EQ(scheduler->lastInputEventId, "input-event");
    EXPECT_EQ(scheduler->lastInputResult, TEST_REPLY_RESULT);

    CliSessionInfo session;
    session.sessionId = "exec-session";
    EXPECT_TRUE(dispatcher.DispatchExecToolReplyEvent(TEST_CALLER_PID, "exec-event", ERR_OK, session));
    EXPECT_EQ(scheduler->execReplyCount, 1);
    EXPECT_EQ(scheduler->lastExecEventId, "exec-event");
    EXPECT_EQ(scheduler->lastExecResult, ERR_OK);
    EXPECT_EQ(scheduler->lastExecSessionId, "exec-session");
}

/**
 * @tc.name: EventDispatcher_Dispatch_0200
 * @tc.desc: Test missing scheduler, failed subscriber and clear branches
 * @tc.type: FUNC
 */
HWTEST_F(EventDispatcherTest, EventDispatcher_Dispatch_0200, TestSize.Level1)
{
    auto &dispatcher = EventDispatcher::GetInstance();
    CliSessionInfo session;
    EXPECT_FALSE(dispatcher.DispatchInputReplyEvent(TEST_CALLER_PID, "input-event", TEST_REPLY_RESULT));
    EXPECT_FALSE(dispatcher.DispatchExecToolReplyEvent(TEST_CALLER_PID, "exec-event", ERR_OK, session));

    sptr<TestScheduler> scheduler = new TestScheduler();
    ASSERT_TRUE(dispatcher.RegisterScheduler(TEST_CALLER_PID, scheduler));
    ASSERT_TRUE(dispatcher.RegisterSubscriber("session", "subscription", TEST_CALLER_PID));

    scheduler->sessionEventResult = ERROR_CODE;
    dispatcher.DispatchIOEvent("session", "stdout", "payload");
    EXPECT_EQ(scheduler->sessionEventCount, 1);

    scheduler->sessionEventResult = ERR_OK;
    dispatcher.DispatchIOEvent("session", "stdout", "payload");
    EXPECT_EQ(scheduler->sessionEventCount, 1);

    ASSERT_TRUE(dispatcher.RegisterSubscriber("session", "subscription", TEST_CALLER_PID));
    dispatcher.ClearSessionSubscribers("session");
    dispatcher.DispatchIOEvent("session", "stdout", "payload");
    EXPECT_EQ(scheduler->sessionEventCount, 1);

    dispatcher.UnregisterScheduler(TEST_CALLER_PID);
    EXPECT_FALSE(dispatcher.DispatchInputReplyEvent(TEST_CALLER_PID, "input-event", TEST_REPLY_RESULT));
}

/**
 * @tc.name: EventDispatcher_Register_0200
 * @tc.desc: Test scheduler replacement and reply failure branches
 * @tc.type: FUNC
 */
HWTEST_F(EventDispatcherTest, EventDispatcher_Register_0200, TestSize.Level1)
{
    auto &dispatcher = EventDispatcher::GetInstance();
    sptr<TestScheduler> oldScheduler = new TestScheduler();
    sptr<TestScheduler> newScheduler = new TestScheduler();
    ASSERT_TRUE(dispatcher.RegisterScheduler(TEST_CALLER_PID, oldScheduler));
    ASSERT_TRUE(dispatcher.RegisterSubscriber("session", "old-subscription", TEST_CALLER_PID));

    ASSERT_TRUE(dispatcher.RegisterScheduler(TEST_CALLER_PID, newScheduler));
    dispatcher.DispatchIOEvent("session", "stdout", "payload");
    EXPECT_EQ(oldScheduler->sessionEventCount, 0);
    EXPECT_EQ(newScheduler->sessionEventCount, 0);

    ASSERT_TRUE(dispatcher.RegisterSubscriber("session", "new-subscription", TEST_CALLER_PID));
    dispatcher.DispatchIOEvent("session", "stdout", "payload");
    EXPECT_EQ(newScheduler->sessionEventCount, 1);

    newScheduler->inputReplyResult = ERROR_CODE;
    EXPECT_FALSE(dispatcher.DispatchInputReplyEvent(TEST_CALLER_PID, "input-event", TEST_REPLY_RESULT));
    EXPECT_EQ(newScheduler->inputReplyCount, 1);

    newScheduler->execReplyResult = ERROR_CODE;
    CliSessionInfo session;
    EXPECT_FALSE(dispatcher.DispatchExecToolReplyEvent(TEST_CALLER_PID, "exec-event", ERR_OK, session));
    EXPECT_EQ(newScheduler->execReplyCount, 1);
}

/**
 * @tc.name: EventDispatcher_Dispatch_0300
 * @tc.desc: Test failed subscriber removal keeps other subscribers
 * @tc.type: FUNC
 */
HWTEST_F(EventDispatcherTest, EventDispatcher_Dispatch_0300, TestSize.Level1)
{
    auto &dispatcher = EventDispatcher::GetInstance();
    sptr<TestScheduler> failedScheduler = new TestScheduler();
    sptr<TestScheduler> okScheduler = new TestScheduler();
    failedScheduler->sessionEventResult = ERROR_CODE;

    ASSERT_TRUE(dispatcher.RegisterScheduler(TEST_CALLER_PID, failedScheduler));
    ASSERT_TRUE(dispatcher.RegisterScheduler(TEST_CALLER_PID_SECOND, okScheduler));
    ASSERT_TRUE(dispatcher.RegisterSubscriber("session", "failed-subscription", TEST_CALLER_PID));
    ASSERT_TRUE(dispatcher.RegisterSubscriber("session", "ok-subscription", TEST_CALLER_PID_SECOND));

    dispatcher.DispatchIOEvent("session", "stdout", "payload");
    EXPECT_EQ(failedScheduler->sessionEventCount, 1);
    EXPECT_EQ(okScheduler->sessionEventCount, 1);

    dispatcher.DispatchIOEvent("session", "stderr", "payload");
    EXPECT_EQ(failedScheduler->sessionEventCount, 1);
    EXPECT_EQ(okScheduler->sessionEventCount, 2);
    EXPECT_EQ(okScheduler->lastSubscriptionId, "ok-subscription");
    EXPECT_EQ(okScheduler->lastEventType, "stderr");
}
} // namespace CliTool
} // namespace OHOS
