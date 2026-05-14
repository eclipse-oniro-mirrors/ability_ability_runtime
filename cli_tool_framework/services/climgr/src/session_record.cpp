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

#include "session_record.h"

namespace OHOS {
namespace CliTool {

void SessionRecord::SetState(SessionState state)
{
    state_.store(state, std::memory_order_release);
}

SessionState SessionRecord::GetState() const
{
    return state_.load(std::memory_order_acquire);
}

void SessionRecord::SetTerminalResult(int32_t status, int32_t sig) // for waitpid
{
    std::lock_guard<std::mutex> lock(resultMutex_);
    terminalStatus_ = status;
    signalNumber_ = sig;
    endTimeMs_ = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    processExited_.store(true, std::memory_order_release);
}

void SessionRecord::SetSkillResult(int32_t resultCode, const std::string &outputText)
{
    std::lock_guard<std::mutex> lock(resultMutex_);
    terminalStatus_ = resultCode;
    stdoutText_ = outputText;
    endTimeMs_ = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    processExited_.store(true, std::memory_order_release);
    stdoutClosed_.store(true, std::memory_order_release);
    stderrClosed_.store(true, std::memory_order_release);
}

int32_t SessionRecord::GetTerminalStatus() const
{
    std::lock_guard<std::mutex> lock(resultMutex_);
    return terminalStatus_;
}

void SessionRecord::SetTimedOut(bool timedOut)
{
    std::lock_guard<std::mutex> lock(resultMutex_);
    timedOut_ = timedOut;
}

bool SessionRecord::TimedOut() const
{
    std::lock_guard<std::mutex> lock(resultMutex_);
    return timedOut_;
}

int64_t SessionRecord::GetEndTimeMs() const
{
    std::lock_guard<std::mutex> lock(resultMutex_);
    return endTimeMs_;
}

bool SessionRecord::HasProcessExited() const
{
    return processExited_.load(std::memory_order_acquire);
}

void SessionRecord::MarkStdoutClosed()
{
    stdoutClosed_.store(true, std::memory_order_release);
}

void SessionRecord::MarkStderrClosed()
{
    stderrClosed_.store(true, std::memory_order_release);
}

bool SessionRecord::OutputDrained() const
{
    return stdoutClosed_.load(std::memory_order_acquire) &&
        stderrClosed_.load(std::memory_order_acquire);
}

bool SessionRecord::BeginCleanup()
{
    bool expected = false;
    return cleanupStarted_.compare_exchange_strong(expected, true, std::memory_order_acq_rel);
}

void SessionRecord::AppendOutput(bool isStdout, const std::string &data)
{
    std::lock_guard<std::mutex> lock(resultMutex_);
    if (isStdout) {
        stdoutText_ += data;
        TrimBufferedOutput(stdoutText_);
        return;
    }
    stderrText_ += data;
    TrimBufferedOutput(stderrText_);
}

bool SessionRecord::SetBackground(bool background)
{
    std::lock_guard<std::mutex> lock(resultMutex_);
    bool oldBackground = background_;
    background_ = background;
    return oldBackground;
}

bool SessionRecord::Background() const
{
    std::lock_guard<std::mutex> lock(resultMutex_);
    return background_;
}

void SessionRecord::BuildSessionInfo(CliSessionInfo &session) const
{
    session.sessionId = sessionId;
    session.toolName = toolName;

    if ((!HasProcessExited() || !OutputDrained()) && !timedOut_) {
        session.status = "running";
    } else {
        session.result = BuildExecResult();
        session.status =
            (!session.result || session.result->timedOut || session.result->exitCode != 0) ? "failed" : "completed";
    }
}

void SessionRecord::TrimBufferedOutput(std::string &buffer)
{
    if (buffer.size() <= MAX_BUFFERED_OUTPUT_BYTES) {
        return;
    }
    buffer.erase(0, buffer.size() - MAX_BUFFERED_OUTPUT_BYTES);
}

std::shared_ptr<ExecResult> SessionRecord::BuildExecResult() const
{
    auto result = std::make_shared<ExecResult>();
    std::lock_guard<std::mutex> lock(resultMutex_);
    if (timedOut_) {
        result->executionTime = timeoutMs;
    } else {
        result->exitCode = terminalStatus_;
        result->executionTime = (endTimeMs_ > startTime) ? (endTimeMs_ - startTime) : 0;
    }
    result->outputText = stdoutText_;
    result->errorText = stderrText_;
    result->signalNumber = signalNumber_;
    result->timedOut = timedOut_;
    return result;
}

} // namespace CliTool
} // namespace OHOS
