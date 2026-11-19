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

#include <atomic>
#include <deque>
#include <functional>
#include <gtest/gtest.h>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>
#include <unistd.h>

#define private public
#include "io_monitor.h"
#undef private

using namespace testing::ext;

namespace OHOS {
namespace CliTool {
namespace {
constexpr int32_t INVALID_FD = -1;

void CloseFd(int &fd)
{
    if (fd >= 0) {
        close(fd);
        fd = INVALID_FD;
    }
}
}

class IOMonitorTest : public testing::Test {};

/**
 * @tc.name: IOMonitor_StartStop_0100
 * @tc.desc: Test monitor start, repeated start, repeated stop and create branches
 * @tc.type: FUNC
 */
HWTEST_F(IOMonitorTest, IOMonitor_StartStop_0100, TestSize.Level1)
{
    auto monitor = IOMonitor::Create();
    ASSERT_NE(monitor, nullptr);

    EXPECT_TRUE(monitor->Start());
    EXPECT_TRUE(monitor->Start());
    monitor->Stop();
    monitor->Stop();
}

/**
 * @tc.name: IOMonitor_RegisterSession_0100
 * @tc.desc: Test register, stdin lookup, unregister and no-fd branches
 * @tc.type: FUNC
 */
HWTEST_F(IOMonitorTest, IOMonitor_RegisterSession_0100, TestSize.Level1)
{
    auto monitor = IOMonitor::Create();
    ASSERT_NE(monitor, nullptr);
    ASSERT_TRUE(monitor->Start());

    int stdoutPipe[2] = {INVALID_FD, INVALID_FD};
    int stderrPipe[2] = {INVALID_FD, INVALID_FD};
    int stdinPipe[2] = {INVALID_FD, INVALID_FD};
    ASSERT_EQ(pipe(stdoutPipe), 0);
    ASSERT_EQ(pipe(stderrPipe), 0);
    ASSERT_EQ(pipe(stdinPipe), 0);

    EXPECT_TRUE(monitor->RegisterSession("session", stdoutPipe[0], stderrPipe[0], stdinPipe[1]));
    stdoutPipe[0] = INVALID_FD;
    stderrPipe[0] = INVALID_FD;
    stdinPipe[1] = INVALID_FD;
    EXPECT_EQ(monitor->GetStdinFd("session"), monitor->GetStdinFdLocked("session"));
    EXPECT_GE(monitor->GetStdinFd("session"), 0);

    monitor->UnregisterSession("session");
    EXPECT_EQ(monitor->GetStdinFd("session"), INVALID_FD);

    EXPECT_TRUE(monitor->RegisterSession("no-fds", INVALID_FD, INVALID_FD, INVALID_FD));
    monitor->UnregisterSession("no-fds");
    monitor->Stop();

    CloseFd(stdoutPipe[0]);
    CloseFd(stdoutPipe[1]);
    CloseFd(stderrPipe[0]);
    CloseFd(stderrPipe[1]);
    CloseFd(stdinPipe[0]);
    CloseFd(stdinPipe[1]);
}

/**
 * @tc.name: IOMonitor_RegisterSession_0200
 * @tc.desc: Test register failure branches when epoll is unavailable or stderr registration fails
 * @tc.type: FUNC
 */
HWTEST_F(IOMonitorTest, IOMonitor_RegisterSession_0200, TestSize.Level1)
{
    auto monitor = IOMonitor::Create();
    ASSERT_NE(monitor, nullptr);

    int stdoutPipe[2] = {INVALID_FD, INVALID_FD};
    ASSERT_EQ(pipe(stdoutPipe), 0);
    EXPECT_FALSE(monitor->RegisterSession("session", stdoutPipe[0], INVALID_FD, INVALID_FD));
    CloseFd(stdoutPipe[0]);
    CloseFd(stdoutPipe[1]);

    ASSERT_TRUE(monitor->Start());
    int rollbackStdoutPipe[2] = {INVALID_FD, INVALID_FD};
    int rollbackStderrPipe[2] = {INVALID_FD, INVALID_FD};
    ASSERT_EQ(pipe(rollbackStdoutPipe), 0);
    ASSERT_EQ(pipe(rollbackStderrPipe), 0);
    int closedStderrFd = rollbackStderrPipe[0];
    close(rollbackStderrPipe[0]);
    rollbackStderrPipe[0] = INVALID_FD;
    EXPECT_FALSE(monitor->RegisterSession("rollback-session", rollbackStdoutPipe[0], closedStderrFd, INVALID_FD));
    EXPECT_TRUE(monitor->fdMap_.empty());
    monitor->Stop();

    CloseFd(rollbackStdoutPipe[0]);
    CloseFd(rollbackStdoutPipe[1]);
    CloseFd(rollbackStderrPipe[0]);
    CloseFd(rollbackStderrPipe[1]);
}

/**
 * @tc.name: IOMonitor_WriteMessage_0100
 * @tc.desc: Test write message empty, success and invalid fd branches
 * @tc.type: FUNC
 */
HWTEST_F(IOMonitorTest, IOMonitor_WriteMessage_0100, TestSize.Level1)
{
    auto monitor = IOMonitor::Create();
    ASSERT_NE(monitor, nullptr);

    EXPECT_TRUE(monitor->WriteMessage(INVALID_FD, "session", ""));
    EXPECT_FALSE(monitor->WriteMessage(INVALID_FD, "session", "data"));

    int stdinPipe[2] = {INVALID_FD, INVALID_FD};
    ASSERT_EQ(pipe(stdinPipe), 0);
    EXPECT_TRUE(monitor->WriteMessage(stdinPipe[1], "session", "hello"));

    char buffer[6] = {};
    ASSERT_EQ(read(stdinPipe[0], buffer, 5), 5);
    EXPECT_EQ(std::string(buffer), "hello");

    CloseFd(stdinPipe[0]);
    CloseFd(stdinPipe[1]);
}

/**
 * @tc.name: IOMonitor_SendMessage_0100
 * @tc.desc: Test send message reject branch when session stdin is missing
 * @tc.type: FUNC
 */
HWTEST_F(IOMonitorTest, IOMonitor_SendMessage_0100, TestSize.Level1)
{
    auto monitor = IOMonitor::Create();
    ASSERT_NE(monitor, nullptr);

    std::string callbackSessionId;
    std::string callbackEventId;
    bool callbackResult = true;
    monitor->SetInputReplyCallback([&](const std::string &sessionId, const std::string &eventId, bool result) {
        callbackSessionId = sessionId;
        callbackEventId = eventId;
        callbackResult = result;
    });

    monitor->SendMessage("missing-session", "payload", "event-id");
    EXPECT_EQ(callbackSessionId, "missing-session");
    EXPECT_EQ(callbackEventId, "event-id");
    EXPECT_FALSE(callbackResult);
}

/**
 * @tc.name: IOMonitor_ProcessWriteQueue_0100
 * @tc.desc: Test process write queue success path and queue cleanup
 * @tc.type: FUNC
 */
HWTEST_F(IOMonitorTest, IOMonitor_ProcessWriteQueue_0100, TestSize.Level1)
{
    auto monitor = IOMonitor::Create();
    ASSERT_NE(monitor, nullptr);

    int stdinPipe[2] = {INVALID_FD, INVALID_FD};
    ASSERT_EQ(pipe(stdinPipe), 0);
    monitor->fdMap_[stdinPipe[1]] = IOMonitor::FdInfo {"session", false, true};
    monitor->inputQueues_["session"].pendingInputs.emplace_back(IOMonitor::PendingInput {"hello", "event-id"});
    monitor->inputQueues_["session"].pendingBytes = 5;
    monitor->inputQueues_["session"].writeTaskRunning = true;

    std::vector<std::string> repliedEvents;
    std::vector<bool> repliedResults;
    monitor->SetInputReplyCallback([&](const std::string &, const std::string &eventId, bool result) {
        repliedEvents.push_back(eventId);
        repliedResults.push_back(result);
    });

    monitor->ProcessWriteQueue("session");

    char buffer[6] = {};
    ASSERT_EQ(read(stdinPipe[0], buffer, 5), 5);
    EXPECT_EQ(std::string(buffer), "hello");
    ASSERT_EQ(repliedEvents.size(), 1u);
    EXPECT_EQ(repliedEvents[0], "event-id");
    EXPECT_TRUE(repliedResults[0]);
    EXPECT_TRUE(monitor->inputQueues_.empty());

    monitor->fdMap_.erase(stdinPipe[1]);
    CloseFd(stdinPipe[0]);
    CloseFd(stdinPipe[1]);
}

/**
 * @tc.name: IOMonitor_ProcessWriteQueue_0200
 * @tc.desc: Test process write queue failure drains pending inputs
 * @tc.type: FUNC
 */
HWTEST_F(IOMonitorTest, IOMonitor_ProcessWriteQueue_0200, TestSize.Level1)
{
    auto monitor = IOMonitor::Create();
    ASSERT_NE(monitor, nullptr);

    monitor->inputQueues_["session"].pendingInputs.emplace_back(IOMonitor::PendingInput {"first", "event-1"});
    monitor->inputQueues_["session"].pendingInputs.emplace_back(IOMonitor::PendingInput {"second", "event-2"});
    monitor->inputQueues_["session"].pendingBytes = 11;
    monitor->inputQueues_["session"].writeTaskRunning = true;

    std::vector<std::string> repliedEvents;
    std::vector<bool> repliedResults;
    monitor->SetInputReplyCallback([&](const std::string &, const std::string &eventId, bool result) {
        repliedEvents.push_back(eventId);
        repliedResults.push_back(result);
    });

    monitor->ProcessWriteQueue("session");

    ASSERT_EQ(repliedEvents.size(), 2u);
    EXPECT_EQ(repliedEvents[0], "event-1");
    EXPECT_EQ(repliedEvents[1], "event-2");
    EXPECT_FALSE(repliedResults[0]);
    EXPECT_FALSE(repliedResults[1]);
    EXPECT_TRUE(monitor->inputQueues_.empty());
}
} // namespace CliTool
} // namespace OHOS
