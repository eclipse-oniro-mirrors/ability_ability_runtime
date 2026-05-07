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

#ifndef OHOS_ABILITY_RUNTIME_SESSION_RECORD_H
#define OHOS_ABILITY_RUNTIME_SESSION_RECORD_H

#include <atomic>
#include <memory>
#include <mutex>
#include <string>
#include <chrono>
#include <sys/types.h>
#include <sys/wait.h>
#include <condition_variable>

#include "cli_session_info.h"

namespace OHOS {
namespace CliTool {

enum class SessionState {
    SPAWNING = 0,
    RUNNING,
    CANCELLING,
};

class SessionRecord {
public:
    SessionRecord() = default;
    ~SessionRecord() = default;

    int32_t callerPid = -1;
    std::string sessionId = "";
    std::string toolName = "";
    std::string eventId = "";
    pid_t processId = -1;
    int64_t startTime = 0;
    int32_t timeoutMs = 0;
    int32_t stdinPipe[2] = {-1, -1};        // [0]=read, [1]=write
    int32_t stdoutPipe[2] = {-1, -1};
    int32_t stderrPipe[2] = {-1, -1};

    void SetState(SessionState state);
    SessionState GetState() const;

    void SetTerminalResult(int32_t status, int32_t sig);
    int32_t GetTerminalStatus() const;

    void SetTimedOut(bool timedOut);
    bool TimedOut() const;
    int64_t GetEndTimeMs() const;

    bool HasProcessExited() const;
    void MarkStdoutClosed();
    void MarkStderrClosed();
    bool OutputDrained() const;

    bool BeginCleanup();

    void AppendOutput(bool isStdout, const std::string &data);

    bool SetBackground(bool background);
    bool Background() const;

    void BuildSessionInfo(CliSessionInfo &session) const;

    inline pid_t GetCallerPid()
    {
        return callerPid;
    }

private:
    void TrimBufferedOutput(std::string &buffer);

    std::shared_ptr<ExecResult> BuildExecResult() const;

private:
    std::atomic<SessionState> state_ {SessionState::SPAWNING};
    std::atomic<bool> processExited_ {false};
    std::atomic<bool> stdoutClosed_ {false};
    std::atomic<bool> stderrClosed_ {false};
    std::atomic<bool> cleanupStarted_ {false};
    mutable std::mutex resultMutex_;
    int32_t terminalStatus_ = 0;
    int32_t signalNumber_ = 0;
    bool timedOut_ = false;
    bool background_ = true;
    int64_t endTimeMs_ = 0;
    std::string stdoutText_ = "";
    std::string stderrText_ = "";

    static constexpr size_t MAX_BUFFERED_OUTPUT_BYTES = 64 * 1024;
};

} // namespace CliTool
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_SESSION_RECORD_H
