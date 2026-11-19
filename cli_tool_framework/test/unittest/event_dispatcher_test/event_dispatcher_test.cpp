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
#include <functional>
#include <memory>
#include <parcel.h>
#include <string>
#include <vector>

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
constexpr int32_t TEST_CALLER_UID = 2000;
constexpr int32_t TEST_CALLER_UID_SECOND = 2001;
constexpr int32_t TEST_EXIT_CODE = 3;
constexpr int32_t TEST_REPLY_RESULT = 5;
const std::string TEST_TOOL_CALL_ID = "tcid_abc-123";
const std::string TEST_DM_SESSION_ID = "dm_xyz-789";
const std::string TEST_TOOL_CALL_ID_SECOND = "tcid_def-456";
const std::string TEST_DM_SESSION_ID_SECOND = "dm_uvw-012";
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
        lastTimestamp = event.timestamp;
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
    int64_t lastTimestamp = 0;
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

class TrackingRemoteObject : public IRemoteObject {
public:
    TrackingRemoteObject() : IRemoteObject(u"TrackingRemoteObject") {}
    ~TrackingRemoteObject() override = default;

    int32_t GetObjectRefCount() override
    {
        return 0;
    }

    int SendRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override
    {
        return ERR_OK;
    }

    bool IsProxyObject() const override
    {
        return true;
    }

    bool CheckObjectLegality() const override
    {
        return true;
    }

    bool AddDeathRecipient(const sptr<DeathRecipient> &recipient) override
    {
        addDeathRecipientCount++;
        if (firstDeathRecipient == nullptr) {
            firstDeathRecipient = recipient;
        }
        lastDeathRecipient = recipient;
        if (onAddDeathRecipient) {
            auto handler = onAddDeathRecipient;
            onAddDeathRecipient = nullptr;
            handler();
        }
        return addDeathRecipientResult;
    }

    bool RemoveDeathRecipient(const sptr<DeathRecipient> &recipient) override
    {
        removeDeathRecipientCount++;
        removedDeathRecipient = recipient;
        return true;
    }

    bool AddRefreshRecipient(const sptr<RefreshRecipient> &recipient) override
    {
        return true;
    }

    bool RemoveRefreshRecipient(const sptr<RefreshRecipient> &recipient) override
    {
        return true;
    }

    bool Marshalling(Parcel &parcel) const override
    {
        return true;
    }

    sptr<IRemoteBroker> AsInterface() override
    {
        return nullptr;
    }

    int Dump(int fd, const std::vector<std::u16string> &args) override
    {
        return ERR_OK;
    }

    bool addDeathRecipientResult = true;
    int32_t addDeathRecipientCount = 0;
    int32_t removeDeathRecipientCount = 0;
    sptr<DeathRecipient> firstDeathRecipient;
    sptr<DeathRecipient> lastDeathRecipient;
    sptr<DeathRecipient> removedDeathRecipient;
    std::function<void()> onAddDeathRecipient;
};

class RemoteBackedScheduler : public ICliToolManagerScheduler {
public:
    explicit RemoteBackedScheduler(const sptr<TrackingRemoteObject> &remote) : remote_(remote) {}
    ~RemoteBackedScheduler() override = default;

    sptr<IRemoteObject> AsObject() override
    {
        return remote_;
    }

    ErrCode SchedulerSessionEvent(const std::string &sessionId, const std::string &subscriptionId,
        const CliToolEvent &event) override
    {
        sessionEventCount++;
        lastSubscriptionId = subscriptionId;
        lastEventType = event.type;
        return sessionEventResult;
    }

    ErrCode SchedulerInputReplyEvent(const std::string &eventId, int32_t resultCode) override
    {
        inputReplyCount++;
        return inputReplyResult;
    }

    ErrCode SchedulerExecToolReplyEvent(const std::string &eventId, int32_t resultCode,
        const CliSessionInfo &session) override
    {
        execReplyCount++;
        return execReplyResult;
    }

    int32_t sessionEventResult = ERR_OK;
    int32_t inputReplyResult = ERR_OK;
    int32_t execReplyResult = ERR_OK;
    int32_t sessionEventCount = 0;
    int32_t inputReplyCount = 0;
    int32_t execReplyCount = 0;
    std::string lastSubscriptionId;
    std::string lastEventType;

private:
    sptr<TrackingRemoteObject> remote_;
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

    EXPECT_FALSE(dispatcher.SetScheduler(0, TEST_CALLER_UID, scheduler));
    EXPECT_FALSE(dispatcher.SetScheduler(TEST_CALLER_PID, -1, scheduler));
    EXPECT_FALSE(dispatcher.SetScheduler(TEST_CALLER_PID, TEST_CALLER_UID, nullptr));
    EXPECT_TRUE(dispatcher.SetScheduler(TEST_CALLER_PID, TEST_CALLER_UID, scheduler));

    EXPECT_FALSE(dispatcher.RegisterSubscriber("", "subscription", TEST_CALLER_PID, TEST_CALLER_UID));
    EXPECT_FALSE(dispatcher.RegisterSubscriber("session", "", TEST_CALLER_PID, TEST_CALLER_UID));
    EXPECT_FALSE(dispatcher.RegisterSubscriber("session", "subscription", 0, TEST_CALLER_UID));
    EXPECT_FALSE(dispatcher.RegisterSubscriber("session", "subscription", TEST_CALLER_PID, -1));
    EXPECT_FALSE(dispatcher.RegisterSubscriber(
        "session", "subscription", TEST_CALLER_PID + 1, TEST_CALLER_UID));
    EXPECT_FALSE(dispatcher.RegisterSubscriber(
        "session", "subscription", TEST_CALLER_PID, TEST_CALLER_UID + 1));
    EXPECT_TRUE(dispatcher.RegisterSubscriber("session", "subscription", TEST_CALLER_PID, TEST_CALLER_UID));

    EXPECT_FALSE(dispatcher.UnregisterSubscriber("", "subscription", TEST_CALLER_PID, TEST_CALLER_UID));
    EXPECT_FALSE(dispatcher.UnregisterSubscriber("session", "", TEST_CALLER_PID, TEST_CALLER_UID));
    EXPECT_FALSE(dispatcher.UnregisterSubscriber("session", "subscription", 0, TEST_CALLER_UID));
    EXPECT_FALSE(dispatcher.UnregisterSubscriber("session", "subscription", TEST_CALLER_PID, -1));
    EXPECT_TRUE(dispatcher.UnregisterSubscriber(
        "unknown-session", "subscription", TEST_CALLER_PID, TEST_CALLER_UID));
    EXPECT_TRUE(dispatcher.UnregisterSubscriber("session", "subscription", TEST_CALLER_PID, TEST_CALLER_UID));
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
    ASSERT_TRUE(dispatcher.SetScheduler(TEST_CALLER_PID, TEST_CALLER_UID, scheduler));
    ASSERT_TRUE(dispatcher.RegisterSubscriber("session", "subscription", TEST_CALLER_PID, TEST_CALLER_UID));

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
    EXPECT_EQ(scheduler->lastEventData, R"({"exitCode":3})");

    EXPECT_TRUE(dispatcher.DispatchInputReplyEvent(
        TEST_CALLER_PID, TEST_CALLER_UID, "input-event", TEST_REPLY_RESULT));
    EXPECT_EQ(scheduler->inputReplyCount, 1);
    EXPECT_EQ(scheduler->lastInputEventId, "input-event");
    EXPECT_EQ(scheduler->lastInputResult, TEST_REPLY_RESULT);

    CliSessionInfo session;
    session.sessionId = "exec-session";
    EXPECT_TRUE(dispatcher.DispatchExecToolReplyEvent(
        TEST_CALLER_PID, TEST_CALLER_UID, "exec-event", ERR_OK, session));
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
    EXPECT_FALSE(dispatcher.DispatchInputReplyEvent(
        TEST_CALLER_PID, TEST_CALLER_UID, "input-event", TEST_REPLY_RESULT));
    EXPECT_FALSE(dispatcher.DispatchExecToolReplyEvent(
        TEST_CALLER_PID, TEST_CALLER_UID, "exec-event", ERR_OK, session));

    sptr<TestScheduler> scheduler = new TestScheduler();
    ASSERT_TRUE(dispatcher.SetScheduler(TEST_CALLER_PID, TEST_CALLER_UID, scheduler));
    ASSERT_TRUE(dispatcher.RegisterSubscriber("session", "subscription", TEST_CALLER_PID, TEST_CALLER_UID));

    scheduler->sessionEventResult = ERROR_CODE;
    dispatcher.DispatchIOEvent("session", "stdout", "payload");
    EXPECT_EQ(scheduler->sessionEventCount, 1);

    scheduler->sessionEventResult = ERR_OK;
    dispatcher.DispatchIOEvent("session", "stdout", "payload");
    EXPECT_EQ(scheduler->sessionEventCount, 1);

    ASSERT_TRUE(dispatcher.RegisterSubscriber("session", "subscription", TEST_CALLER_PID, TEST_CALLER_UID));
    dispatcher.ClearSessionSubscribers("session");
    dispatcher.DispatchIOEvent("session", "stdout", "payload");
    EXPECT_EQ(scheduler->sessionEventCount, 1);

    dispatcher.ClearScheduler(TEST_CALLER_PID, TEST_CALLER_UID);
    EXPECT_FALSE(dispatcher.DispatchInputReplyEvent(
        TEST_CALLER_PID, TEST_CALLER_UID, "input-event", TEST_REPLY_RESULT));
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
    ASSERT_TRUE(dispatcher.SetScheduler(TEST_CALLER_PID, TEST_CALLER_UID, oldScheduler));
    ASSERT_TRUE(dispatcher.RegisterSubscriber("session", "old-subscription", TEST_CALLER_PID, TEST_CALLER_UID));

    ASSERT_TRUE(dispatcher.SetScheduler(TEST_CALLER_PID, TEST_CALLER_UID, oldScheduler));
    dispatcher.DispatchIOEvent("session", "stdout", "payload");
    EXPECT_EQ(oldScheduler->sessionEventCount, 1);

    ASSERT_TRUE(dispatcher.SetScheduler(TEST_CALLER_PID, TEST_CALLER_UID, newScheduler));
    dispatcher.DispatchIOEvent("session", "stdout", "payload");
    EXPECT_EQ(oldScheduler->sessionEventCount, 1);
    EXPECT_EQ(newScheduler->sessionEventCount, 0);

    ASSERT_TRUE(dispatcher.RegisterSubscriber("session", "new-subscription", TEST_CALLER_PID, TEST_CALLER_UID));
    dispatcher.DispatchIOEvent("session", "stdout", "payload");
    EXPECT_EQ(newScheduler->sessionEventCount, 1);

    newScheduler->inputReplyResult = ERROR_CODE;
    EXPECT_FALSE(dispatcher.DispatchInputReplyEvent(
        TEST_CALLER_PID, TEST_CALLER_UID, "input-event", TEST_REPLY_RESULT));
    EXPECT_EQ(newScheduler->inputReplyCount, 1);

    newScheduler->execReplyResult = ERROR_CODE;
    CliSessionInfo session;
    EXPECT_FALSE(dispatcher.DispatchExecToolReplyEvent(
        TEST_CALLER_PID, TEST_CALLER_UID, "exec-event", ERR_OK, session));
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

    ASSERT_TRUE(dispatcher.SetScheduler(TEST_CALLER_PID, TEST_CALLER_UID, failedScheduler));
    ASSERT_TRUE(dispatcher.SetScheduler(TEST_CALLER_PID_SECOND, TEST_CALLER_UID_SECOND, okScheduler));
    ASSERT_TRUE(dispatcher.RegisterSubscriber(
        "session", "failed-subscription", TEST_CALLER_PID, TEST_CALLER_UID));
    ASSERT_TRUE(dispatcher.RegisterSubscriber(
        "session", "ok-subscription", TEST_CALLER_PID_SECOND, TEST_CALLER_UID_SECOND));

    dispatcher.DispatchIOEvent("session", "stdout", "payload");
    EXPECT_EQ(failedScheduler->sessionEventCount, 1);
    EXPECT_EQ(okScheduler->sessionEventCount, 1);

    dispatcher.DispatchIOEvent("session", "stderr", "payload");
    EXPECT_EQ(failedScheduler->sessionEventCount, 1);
    EXPECT_EQ(okScheduler->sessionEventCount, 2);
    EXPECT_EQ(okScheduler->lastSubscriptionId, "ok-subscription");
    EXPECT_EQ(okScheduler->lastEventType, "stderr");
}

/**
 * @tc.name: EventDispatcher_Dispatch_0400
 * @tc.desc: Test schedulers are isolated by pid and uid together
 * @tc.type: FUNC
 */
HWTEST_F(EventDispatcherTest, EventDispatcher_Dispatch_0400, TestSize.Level1)
{
    auto &dispatcher = EventDispatcher::GetInstance();
    sptr<TestScheduler> firstScheduler = new TestScheduler();
    sptr<TestScheduler> secondScheduler = new TestScheduler();

    ASSERT_TRUE(dispatcher.SetScheduler(TEST_CALLER_PID, TEST_CALLER_UID, firstScheduler));
    ASSERT_TRUE(dispatcher.SetScheduler(TEST_CALLER_PID, TEST_CALLER_UID_SECOND, secondScheduler));
    ASSERT_TRUE(dispatcher.RegisterSubscriber("session", "first-subscription", TEST_CALLER_PID, TEST_CALLER_UID));
    ASSERT_TRUE(dispatcher.RegisterSubscriber(
        "session", "second-subscription", TEST_CALLER_PID, TEST_CALLER_UID_SECOND));

    dispatcher.DispatchIOEvent("session", "stdout", "payload");
    EXPECT_EQ(firstScheduler->sessionEventCount, 1);
    EXPECT_EQ(firstScheduler->lastSubscriptionId, "first-subscription");
    EXPECT_EQ(secondScheduler->sessionEventCount, 1);
    EXPECT_EQ(secondScheduler->lastSubscriptionId, "second-subscription");

    EXPECT_TRUE(dispatcher.DispatchInputReplyEvent(
        TEST_CALLER_PID, TEST_CALLER_UID, "input-event", TEST_REPLY_RESULT));
    EXPECT_EQ(firstScheduler->inputReplyCount, 1);
    EXPECT_EQ(secondScheduler->inputReplyCount, 0);

    dispatcher.ClearScheduler(TEST_CALLER_PID, TEST_CALLER_UID);
    EXPECT_FALSE(dispatcher.DispatchInputReplyEvent(
        TEST_CALLER_PID, TEST_CALLER_UID, "input-event", TEST_REPLY_RESULT));
    EXPECT_TRUE(dispatcher.DispatchInputReplyEvent(
        TEST_CALLER_PID, TEST_CALLER_UID_SECOND, "input-event", TEST_REPLY_RESULT));
}

/**
 * @tc.name: EventDispatcher_SetScheduler_0100
 * @tc.desc: Test SetScheduler skips death recipient updates when the same scheduler is already stored
 * @tc.type: FUNC
 */
HWTEST_F(EventDispatcherTest, EventDispatcher_SetScheduler_0100, TestSize.Level1)
{
    auto &dispatcher = EventDispatcher::GetInstance();
    sptr<TrackingRemoteObject> remote = new TrackingRemoteObject();
    sptr<RemoteBackedScheduler> scheduler = new RemoteBackedScheduler(remote);

    ASSERT_TRUE(dispatcher.SetScheduler(TEST_CALLER_PID, TEST_CALLER_UID, scheduler));
    EXPECT_EQ(remote->addDeathRecipientCount, 1);
    EXPECT_EQ(remote->removeDeathRecipientCount, 0);

    ASSERT_TRUE(dispatcher.SetScheduler(TEST_CALLER_PID, TEST_CALLER_UID, scheduler));
    EXPECT_EQ(remote->addDeathRecipientCount, 1);
    EXPECT_EQ(remote->removeDeathRecipientCount, 0);
}

/**
 * @tc.name: EventDispatcher_SetScheduler_0200
 * @tc.desc: Test SetScheduler removes the old death recipient and subscribers when scheduler is replaced
 * @tc.type: FUNC
 */
HWTEST_F(EventDispatcherTest, EventDispatcher_SetScheduler_0200, TestSize.Level1)
{
    auto &dispatcher = EventDispatcher::GetInstance();
    sptr<TrackingRemoteObject> oldRemote = new TrackingRemoteObject();
    sptr<TrackingRemoteObject> newRemote = new TrackingRemoteObject();
    sptr<RemoteBackedScheduler> oldScheduler = new RemoteBackedScheduler(oldRemote);
    sptr<RemoteBackedScheduler> newScheduler = new RemoteBackedScheduler(newRemote);

    ASSERT_TRUE(dispatcher.SetScheduler(TEST_CALLER_PID, TEST_CALLER_UID, oldScheduler));
    ASSERT_TRUE(dispatcher.RegisterSubscriber("session", "old-subscription", TEST_CALLER_PID, TEST_CALLER_UID));
    ASSERT_TRUE(dispatcher.SetScheduler(TEST_CALLER_PID, TEST_CALLER_UID, newScheduler));

    EXPECT_EQ(oldRemote->addDeathRecipientCount, 1);
    EXPECT_EQ(oldRemote->removeDeathRecipientCount, 1);
    EXPECT_EQ(newRemote->addDeathRecipientCount, 1);
    EXPECT_EQ(newRemote->removeDeathRecipientCount, 0);

    dispatcher.DispatchIOEvent("session", "stdout", "payload");
    EXPECT_EQ(oldScheduler->sessionEventCount, 0);
    EXPECT_EQ(newScheduler->sessionEventCount, 0);
}

/**
 * @tc.name: EventDispatcher_SetScheduler_0300
 * @tc.desc: Test SetScheduler keeps the scheduler when AddDeathRecipient fails
 * @tc.type: FUNC
 */
HWTEST_F(EventDispatcherTest, EventDispatcher_SetScheduler_0300, TestSize.Level1)
{
    auto &dispatcher = EventDispatcher::GetInstance();
    sptr<TrackingRemoteObject> remote = new TrackingRemoteObject();
    sptr<RemoteBackedScheduler> scheduler = new RemoteBackedScheduler(remote);
    remote->addDeathRecipientResult = false;

    ASSERT_TRUE(dispatcher.SetScheduler(TEST_CALLER_PID, TEST_CALLER_UID, scheduler));
    EXPECT_EQ(remote->addDeathRecipientCount, 1);
    ASSERT_TRUE(dispatcher.RegisterSubscriber("session", "subscription", TEST_CALLER_PID, TEST_CALLER_UID));

    dispatcher.DispatchIOEvent("session", "stdout", "payload");
    EXPECT_EQ(scheduler->sessionEventCount, 1);
    EXPECT_EQ(scheduler->lastSubscriptionId, "subscription");
    EXPECT_EQ(scheduler->lastEventType, "stdout");
}

/**
 * @tc.name: EventDispatcher_SetScheduler_0400
 * @tc.desc: Test SetScheduler removes a newly added death recipient if the same scheduler was stored concurrently
 * @tc.type: FUNC
 */
HWTEST_F(EventDispatcherTest, EventDispatcher_SetScheduler_0400, TestSize.Level1)
{
    auto &dispatcher = EventDispatcher::GetInstance();
    sptr<TrackingRemoteObject> remote = new TrackingRemoteObject();
    sptr<RemoteBackedScheduler> scheduler = new RemoteBackedScheduler(remote);
    remote->onAddDeathRecipient = [&dispatcher, &scheduler]() {
        EXPECT_TRUE(dispatcher.SetScheduler(TEST_CALLER_PID, TEST_CALLER_UID, scheduler));
    };

    ASSERT_TRUE(dispatcher.SetScheduler(TEST_CALLER_PID, TEST_CALLER_UID, scheduler));
    EXPECT_EQ(remote->addDeathRecipientCount, 2);
    EXPECT_EQ(remote->removeDeathRecipientCount, 1);
    EXPECT_EQ(remote->removedDeathRecipient, remote->firstDeathRecipient);
}

/**
 * @tc.name: EventDispatcher_TraceIds_0100
 * @tc.desc: Test dispatching events for a session with registered trace ids keeps event content unchanged
 * @tc.type: FUNC
 */
HWTEST_F(EventDispatcherTest, EventDispatcher_TraceIds_0100, TestSize.Level1)
{
    auto &dispatcher = EventDispatcher::GetInstance();
    sptr<TestScheduler> scheduler = new TestScheduler();
    ASSERT_TRUE(dispatcher.SetScheduler(TEST_CALLER_PID, TEST_CALLER_UID, scheduler));
    ASSERT_TRUE(dispatcher.RegisterSubscriber("session", "subscription", TEST_CALLER_PID, TEST_CALLER_UID));

    dispatcher.RegisterTraceIds("session", TEST_TOOL_CALL_ID, TEST_DM_SESSION_ID);

    dispatcher.DispatchIOEvent("session", "stdout", "payload");
    EXPECT_EQ(scheduler->sessionEventCount, 1);
    EXPECT_EQ(scheduler->lastSessionId, "session");
    EXPECT_EQ(scheduler->lastSubscriptionId, "subscription");
    EXPECT_EQ(scheduler->lastEventType, "stdout");
    EXPECT_EQ(scheduler->lastEventData, "payload");
    EXPECT_EQ(scheduler->lastExitCode, 0);
    EXPECT_GE(scheduler->lastTimestamp, 0);

    dispatcher.DispatchErrorEvent("session", "error text");
    EXPECT_EQ(scheduler->sessionEventCount, 2);
    EXPECT_EQ(scheduler->lastEventType, "error");
    EXPECT_EQ(scheduler->lastEventData, "error text");

    dispatcher.DispatchExitEvent("session", TEST_EXIT_CODE);
    EXPECT_EQ(scheduler->sessionEventCount, 3);
    EXPECT_EQ(scheduler->lastEventType, "exit");
    EXPECT_EQ(scheduler->lastExitCode, TEST_EXIT_CODE);
    EXPECT_EQ(scheduler->lastEventData, R"({"exitCode":3})");
}

/**
 * @tc.name: EventDispatcher_TraceIds_0200
 * @tc.desc: Test dispatch delivers identical event content with and without registered trace ids
 * @tc.type: FUNC
 */
HWTEST_F(EventDispatcherTest, EventDispatcher_TraceIds_0200, TestSize.Level1)
{
    auto &dispatcher = EventDispatcher::GetInstance();
    sptr<TestScheduler> plainScheduler = new TestScheduler();
    sptr<TestScheduler> tracedScheduler = new TestScheduler();
    ASSERT_TRUE(dispatcher.SetScheduler(TEST_CALLER_PID, TEST_CALLER_UID, plainScheduler));
    ASSERT_TRUE(dispatcher.SetScheduler(TEST_CALLER_PID_SECOND, TEST_CALLER_UID_SECOND, tracedScheduler));
    ASSERT_TRUE(dispatcher.RegisterSubscriber(
        "plain-session", "plain-subscription", TEST_CALLER_PID, TEST_CALLER_UID));
    ASSERT_TRUE(dispatcher.RegisterSubscriber(
        "traced-session", "traced-subscription", TEST_CALLER_PID_SECOND, TEST_CALLER_UID_SECOND));

    dispatcher.RegisterTraceIds("traced-session", TEST_TOOL_CALL_ID, TEST_DM_SESSION_ID);

    dispatcher.DispatchIOEvent("plain-session", "stdout", "payload");
    dispatcher.DispatchIOEvent("traced-session", "stdout", "payload");

    EXPECT_EQ(plainScheduler->sessionEventCount, 1);
    EXPECT_EQ(tracedScheduler->sessionEventCount, 1);
    EXPECT_EQ(plainScheduler->lastSubscriptionId, "plain-subscription");
    EXPECT_EQ(tracedScheduler->lastSubscriptionId, "traced-subscription");
    EXPECT_EQ(tracedScheduler->lastEventType, plainScheduler->lastEventType);
    EXPECT_EQ(tracedScheduler->lastEventType, "stdout");
    EXPECT_EQ(tracedScheduler->lastEventData, plainScheduler->lastEventData);
    EXPECT_EQ(tracedScheduler->lastEventData, "payload");
    EXPECT_EQ(tracedScheduler->lastExitCode, plainScheduler->lastExitCode);
    EXPECT_EQ(tracedScheduler->lastExitCode, 0);
    EXPECT_GE(tracedScheduler->lastTimestamp, 0);

    CliToolEvent captured;
    captured.type = tracedScheduler->lastEventType;
    captured.eventData = tracedScheduler->lastEventData;
    captured.exitCode = tracedScheduler->lastExitCode;
    captured.timestamp = tracedScheduler->lastTimestamp;
    Parcel parcel;
    ASSERT_TRUE(captured.Marshalling(parcel));
    std::unique_ptr<CliToolEvent> restored(CliToolEvent::Unmarshalling(parcel));
    ASSERT_NE(restored, nullptr);
    EXPECT_EQ(restored->type, captured.type);
    EXPECT_EQ(restored->eventData, captured.eventData);
    EXPECT_EQ(restored->exitCode, captured.exitCode);
    EXPECT_EQ(restored->timestamp, captured.timestamp);
}

/**
 * @tc.name: EventDispatcher_TraceIds_0300
 * @tc.desc: Test dispatch falls back to normal behavior after UnregisterTraceIds
 * @tc.type: FUNC
 */
HWTEST_F(EventDispatcherTest, EventDispatcher_TraceIds_0300, TestSize.Level1)
{
    auto &dispatcher = EventDispatcher::GetInstance();
    sptr<TestScheduler> scheduler = new TestScheduler();
    ASSERT_TRUE(dispatcher.SetScheduler(TEST_CALLER_PID, TEST_CALLER_UID, scheduler));
    ASSERT_TRUE(dispatcher.RegisterSubscriber("session", "subscription", TEST_CALLER_PID, TEST_CALLER_UID));

    dispatcher.RegisterTraceIds("session", TEST_TOOL_CALL_ID, TEST_DM_SESSION_ID);
    dispatcher.DispatchIOEvent("session", "stdout", "payload");
    EXPECT_EQ(scheduler->sessionEventCount, 1);
    EXPECT_EQ(scheduler->lastEventType, "stdout");
    EXPECT_EQ(scheduler->lastEventData, "payload");

    dispatcher.UnregisterTraceIds("session");
    dispatcher.DispatchIOEvent("session", "stdout", "payload");
    EXPECT_EQ(scheduler->sessionEventCount, 2);
    EXPECT_EQ(scheduler->lastEventType, "stdout");
    EXPECT_EQ(scheduler->lastEventData, "payload");
    EXPECT_EQ(scheduler->lastExitCode, 0);

    dispatcher.UnregisterTraceIds("");
    dispatcher.UnregisterTraceIds("unknown-session");
    dispatcher.DispatchIOEvent("session", "stderr", "payload-2");
    EXPECT_EQ(scheduler->sessionEventCount, 3);
    EXPECT_EQ(scheduler->lastEventType, "stderr");
    EXPECT_EQ(scheduler->lastEventData, "payload-2");
}

/**
 * @tc.name: EventDispatcher_TraceIds_0400
 * @tc.desc: Test RegisterTraceIds validation no-ops, overwrite and registration before subscribers
 * @tc.type: FUNC
 */
HWTEST_F(EventDispatcherTest, EventDispatcher_TraceIds_0400, TestSize.Level1)
{
    auto &dispatcher = EventDispatcher::GetInstance();
    sptr<TestScheduler> scheduler = new TestScheduler();

    dispatcher.RegisterTraceIds("early-session", TEST_TOOL_CALL_ID, TEST_DM_SESSION_ID);

    dispatcher.RegisterTraceIds("", TEST_TOOL_CALL_ID, TEST_DM_SESSION_ID);
    dispatcher.RegisterTraceIds("session", "", "");

    ASSERT_TRUE(dispatcher.SetScheduler(TEST_CALLER_PID, TEST_CALLER_UID, scheduler));
    ASSERT_TRUE(dispatcher.RegisterSubscriber("session", "subscription", TEST_CALLER_PID, TEST_CALLER_UID));
    ASSERT_TRUE(dispatcher.RegisterSubscriber(
        "early-session", "early-subscription", TEST_CALLER_PID, TEST_CALLER_UID));

    dispatcher.RegisterTraceIds("session", TEST_TOOL_CALL_ID, TEST_DM_SESSION_ID);
    dispatcher.RegisterTraceIds("session", TEST_TOOL_CALL_ID_SECOND, TEST_DM_SESSION_ID_SECOND);
    dispatcher.RegisterTraceIds("session", TEST_TOOL_CALL_ID_SECOND, "");

    dispatcher.DispatchIOEvent("session", "stdout", "payload");
    EXPECT_EQ(scheduler->sessionEventCount, 1);
    EXPECT_EQ(scheduler->lastSessionId, "session");
    EXPECT_EQ(scheduler->lastSubscriptionId, "subscription");
    EXPECT_EQ(scheduler->lastEventType, "stdout");
    EXPECT_EQ(scheduler->lastEventData, "payload");

    dispatcher.DispatchIOEvent("early-session", "exit", "final");
    EXPECT_EQ(scheduler->sessionEventCount, 2);
    EXPECT_EQ(scheduler->lastSessionId, "early-session");
    EXPECT_EQ(scheduler->lastSubscriptionId, "early-subscription");
    EXPECT_EQ(scheduler->lastEventType, "exit");
    EXPECT_EQ(scheduler->lastEventData, "final");
}
} // namespace CliTool
} // namespace OHOS
