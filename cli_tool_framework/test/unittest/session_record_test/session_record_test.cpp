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

#include "session_record.h"

using namespace testing::ext;

namespace OHOS {
namespace CliTool {
namespace {
constexpr int32_t TEST_STATUS_OK = 0;
constexpr int32_t TEST_STATUS_FAILED = 2;
constexpr int32_t TEST_SIGNAL = 15;
constexpr int32_t TEST_TIMEOUT_MS = 5000;
constexpr size_t MAX_BUFFERED_OUTPUT_BYTES = 64 * 1024;
}

class SessionRecordTest : public testing::Test {};

/**
 * @tc.name: SessionRecord_State_0100
 * @tc.desc: Test state, cleanup and background branches
 * @tc.type: FUNC
 */
HWTEST_F(SessionRecordTest, SessionRecord_State_0100, TestSize.Level1)
{
    SessionRecord record;
    EXPECT_EQ(record.GetState(), SessionState::SPAWNING);

    record.SetState(SessionState::RUNNING);
    EXPECT_EQ(record.GetState(), SessionState::RUNNING);

    EXPECT_TRUE(record.Background());
    EXPECT_TRUE(record.SetBackground(false));
    EXPECT_FALSE(record.Background());
    EXPECT_FALSE(record.SetBackground(true));
    EXPECT_TRUE(record.Background());

    EXPECT_TRUE(record.TryClaimCleanup());
    EXPECT_FALSE(record.TryClaimCleanup());
}

/**
 * @tc.name: SessionRecord_OutputAndTerminal_0100
 * @tc.desc: Test output append, close flags and terminal result branches
 * @tc.type: FUNC
 */
HWTEST_F(SessionRecordTest, SessionRecord_OutputAndTerminal_0100, TestSize.Level1)
{
    SessionRecord record;
    EXPECT_FALSE(record.OutputDrained());
    record.MarkStdoutClosed();
    EXPECT_FALSE(record.OutputDrained());
    record.MarkStderrClosed();
    EXPECT_TRUE(record.OutputDrained());

    EXPECT_FALSE(record.HasProcessExited());
    record.AppendOutput(true, "stdout");
    record.AppendOutput(false, "stderr");
    record.SetTerminalResult(TEST_STATUS_FAILED, TEST_SIGNAL);

    EXPECT_TRUE(record.HasProcessExited());
    EXPECT_EQ(record.GetTerminalStatus(), TEST_STATUS_FAILED);
    EXPECT_GT(record.GetEndTimeMs(), 0);

    CliSessionInfo session;
    record.BuildSessionInfo(session);
    ASSERT_NE(session.result, nullptr);
    EXPECT_EQ(session.status, "failed");
    EXPECT_EQ(session.result->exitCode, TEST_STATUS_FAILED);
    EXPECT_EQ(session.result->signalNumber, TEST_SIGNAL);
    EXPECT_EQ(session.result->outputText, "stdout");
    EXPECT_EQ(session.result->errorText, "stderr");
}

/**
 * @tc.name: SessionRecord_BuildSessionInfo_0100
 * @tc.desc: Test running, completed, failed and timeout session info branches
 * @tc.type: FUNC
 */
HWTEST_F(SessionRecordTest, SessionRecord_BuildSessionInfo_0100, TestSize.Level1)
{
    SessionRecord runningRecord;
    runningRecord.sessionId = "running-session";
    runningRecord.toolName = "tool";
    CliSessionInfo runningSession;
    runningRecord.BuildSessionInfo(runningSession);
    EXPECT_EQ(runningSession.sessionId, "running-session");
    EXPECT_EQ(runningSession.toolName, "tool");
    EXPECT_EQ(runningSession.status, "running");
    EXPECT_EQ(runningSession.result, nullptr);

    SessionRecord completedRecord;
    completedRecord.startTime = 1;
    completedRecord.MarkStdoutClosed();
    completedRecord.MarkStderrClosed();
    completedRecord.SetTerminalResult(TEST_STATUS_OK, 0);
    CliSessionInfo completedSession;
    completedRecord.BuildSessionInfo(completedSession);
    ASSERT_NE(completedSession.result, nullptr);
    EXPECT_EQ(completedSession.status, "completed");
    EXPECT_EQ(completedSession.result->exitCode, TEST_STATUS_OK);
    EXPECT_FALSE(completedSession.result->timeout);
    EXPECT_GT(completedSession.result->executionTime, 0);

    SessionRecord failedRecord;
    failedRecord.MarkStdoutClosed();
    failedRecord.MarkStderrClosed();
    failedRecord.SetTerminalResult(TEST_STATUS_FAILED, 0);
    CliSessionInfo failedSession;
    failedRecord.BuildSessionInfo(failedSession);
    ASSERT_NE(failedSession.result, nullptr);
    EXPECT_EQ(failedSession.status, "failed");
    EXPECT_EQ(failedSession.result->exitCode, TEST_STATUS_FAILED);

    SessionRecord timeoutRecord;
    timeoutRecord.timeoutMs = TEST_TIMEOUT_MS;
    timeoutRecord.SetTimeout(true);
    EXPECT_TRUE(timeoutRecord.Timeout());
    CliSessionInfo timeoutSession;
    timeoutRecord.BuildSessionInfo(timeoutSession);
    ASSERT_NE(timeoutSession.result, nullptr);
    EXPECT_EQ(timeoutSession.status, "failed");
    EXPECT_TRUE(timeoutSession.result->timeout);
    EXPECT_EQ(timeoutSession.result->executionTime, TEST_TIMEOUT_MS);
}

/**
 * @tc.name: SessionRecord_SetSkillResult_0100
 * @tc.desc: Test skill result closes output and builds terminal result
 * @tc.type: FUNC
 */
HWTEST_F(SessionRecordTest, SessionRecord_SetSkillResult_0100, TestSize.Level1)
{
    SessionRecord record;
    record.SetSkillResult(TEST_STATUS_OK, "skill output");

    EXPECT_TRUE(record.HasProcessExited());
    EXPECT_TRUE(record.OutputDrained());
    EXPECT_EQ(record.GetTerminalStatus(), TEST_STATUS_OK);

    CliSessionInfo session;
    record.BuildSessionInfo(session);
    ASSERT_NE(session.result, nullptr);
    EXPECT_EQ(session.status, "completed");
    EXPECT_EQ(session.result->outputText, "skill output");
}

/**
 * @tc.name: SessionRecord_TrimBufferedOutput_0100
 * @tc.desc: Test large stdout and stderr buffers keep only the latest bytes
 * @tc.type: FUNC
 */
HWTEST_F(SessionRecordTest, SessionRecord_TrimBufferedOutput_0100, TestSize.Level1)
{
    SessionRecord record;
    std::string prefix(10, 'a');
    std::string tail(MAX_BUFFERED_OUTPUT_BYTES, 'b');
    record.AppendOutput(true, prefix + tail);
    record.AppendOutput(false, prefix + tail);
    record.MarkStdoutClosed();
    record.MarkStderrClosed();
    record.SetTerminalResult(TEST_STATUS_FAILED, 0);

    CliSessionInfo session;
    record.BuildSessionInfo(session);
    ASSERT_NE(session.result, nullptr);
    EXPECT_EQ(session.result->outputText.size(), MAX_BUFFERED_OUTPUT_BYTES);
    EXPECT_EQ(session.result->errorText.size(), MAX_BUFFERED_OUTPUT_BYTES);
    EXPECT_EQ(session.result->outputText, tail);
    EXPECT_EQ(session.result->errorText, tail);
}
} // namespace CliTool
} // namespace OHOS
