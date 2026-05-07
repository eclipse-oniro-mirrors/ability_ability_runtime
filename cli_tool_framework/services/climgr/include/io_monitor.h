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

#ifndef OHOS_ABILITY_RUNTIME_IO_MONITOR_H
#define OHOS_ABILITY_RUNTIME_IO_MONITOR_H

#include <atomic>
#include <functional>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>

namespace OHOS {
namespace CliTool {

class IOMonitor : public std::enable_shared_from_this<IOMonitor> {
public:
    using OutputCallback = std::function<void(const std::string &, bool, const std::string &)>;
    using InputReplyCallback = std::function<void(const std::string &, const std::string &, bool)>;
    using SessionClosedCallback = std::function<void(const std::string &, bool)>;
    using SessionDrainedCallback = std::function<void(const std::string &)>;

    static std::shared_ptr<IOMonitor> Create();

    IOMonitor() = default;
    ~IOMonitor();

    bool Start();
    void Stop();

    bool RegisterSession(const std::string &sessionId, int stdoutFd, int stderrFd, int stdinFd);
    void UnregisterSession(const std::string &sessionId);

    void SetOutputCallback(OutputCallback callback);
    void SetSessionClosedCallback(SessionClosedCallback callback);
    void SetSessionDrainedCallback(SessionDrainedCallback callback);
    void SetInputReplyCallback(InputReplyCallback callback);
    void SendMessage(const std::string &sessionId, const std::string &message, const std::string &eventId);

private:
    struct FdInfo {
        std::string sessionId;
        bool isStdout = false;
        bool isStdin = false;
    };

    int GetStdinFd(const std::string &sessionId);
    void WriteTask(const std::string &sessionId, const std::string &message, const std::string &eventId);

    void MonitorLoop();
    void HandleReadableFd(int fd);
    void CloseFdLocked(int fd, const FdInfo &info, bool notifyDrained);

private:
    std::atomic<bool> running_ {false};
    std::thread monitorThread_;
    int epollFd_ = -1;
    std::mutex fdMutex_;
    std::unordered_map<int, FdInfo> fdMap_;
    OutputCallback outputCallback_;
    InputReplyCallback inputReplyCallback_;
    SessionClosedCallback sessionClosedCallback_;
    SessionDrainedCallback sessionDrainedCallback_;
};

} // namespace CliTool
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_IO_MONITOR_H
