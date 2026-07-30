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

#include "io_monitor.h"

#include <algorithm>
#include <cerrno>
#include <chrono>
#include <cstring>
#include <fcntl.h>
#include <poll.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <utility>
#include <vector>

#include "ffrt.h"
#include "hilog_tag_wrapper.h"

#define CLI_IO_READ_BUFFER_SIZE 4096

namespace OHOS {
namespace CliTool {

namespace {
constexpr int32_t MAX_EVENTS = 16;
constexpr int32_t EPOLL_WAIT_MS = 100;
constexpr int32_t INPUT_WRITE_POLL_MS = 1000;
constexpr int32_t INPUT_WRITE_TIMEOUT_MS = 30 * 1000;
constexpr size_t MAX_PENDING_INPUT_BYTES = 4 * 1024 * 1024;
constexpr size_t MAX_PENDING_INPUT_MESSAGES = 4096;
}

std::shared_ptr<IOMonitor> IOMonitor::Create()
{
    return std::make_shared<IOMonitor>();
}

IOMonitor::~IOMonitor()
{
    Stop();
}

bool IOMonitor::Start()
{
    if (running_.exchange(true)) {
        return true;
    }
    epollFd_ = epoll_create1(EPOLL_CLOEXEC);
    if (epollFd_ < 0) {
        running_.store(false, std::memory_order_release);
        TAG_LOGE(AAFwkTag::CLI_TOOL, "epoll_create1 failed: %{public}s", strerror(errno));
        return false;
    }
    auto monitor = shared_from_this();
    monitorThread_ = std::thread([monitor]() {
        monitor->MonitorLoop();
    });
    return true;
}

void IOMonitor::Stop()
{
    if (!running_.exchange(false)) {
        return;
    }
    // Join the monitor thread BEFORE closing epollFd_. std::thread::join() establishes a
    // happens-before edge for every access the monitor thread made to epollFd_ (epoll_wait in
    // MonitorLoop, epoll_ctl in CloseFdLocked), so closing epollFd_ afterwards is race-free.
    if (monitorThread_.joinable()) {
        monitorThread_.join();
    }

    std::vector<std::pair<std::string, PendingInput>> failedInputs;
    {
        std::lock_guard<std::mutex> lock(fdMutex_);
        if (epollFd_ >= 0) {
            close(epollFd_);
            epollFd_ = -1;
        }
        for (const auto &[fd, info] : fdMap_) {
            close(fd);
        }
        fdMap_.clear();
        for (auto &[sessionId, queue] : inputQueues_) {
            while (!queue.pendingInputs.empty()) {
                failedInputs.emplace_back(sessionId, std::move(queue.pendingInputs.front()));
                queue.pendingInputs.pop_front();
            }
            queue.pendingBytes = 0;
            queue.writeTaskRunning = false;
        }
        inputQueues_.clear();
    }

    for (const auto &[sessionId, input] : failedInputs) {
        NotifyInputReply(sessionId, input.eventId, false);
    }
}

bool IOMonitor::RegisterSession(const std::string &sessionId, int stdoutFd, int stderrFd, int stdinFd)
{
    std::lock_guard<std::mutex> lock(fdMutex_);
    epoll_event event {};
    event.events = EPOLLIN | EPOLLHUP | EPOLLERR;

    if (stdoutFd >= 0) {
        int flags = fcntl(stdoutFd, F_GETFL, 0);
        if (flags >= 0) {
            fcntl(stdoutFd, F_SETFL, flags | O_NONBLOCK);
        }
        event.data.fd = stdoutFd;
        if (epoll_ctl(epollFd_, EPOLL_CTL_ADD, stdoutFd, &event) != 0) {
            return false;
        }
        fdMap_[stdoutFd] = FdInfo {sessionId, true};
    }

    if (stderrFd >= 0) {
        int flags = fcntl(stderrFd, F_GETFL, 0);
        if (flags >= 0) {
            fcntl(stderrFd, F_SETFL, flags | O_NONBLOCK);
        }
        event.data.fd = stderrFd;
        if (epoll_ctl(epollFd_, EPOLL_CTL_ADD, stderrFd, &event) != 0) {
            if (stdoutFd >= 0) {
                epoll_ctl(epollFd_, EPOLL_CTL_DEL, stdoutFd, nullptr);
                fdMap_.erase(stdoutFd);
            }
            return false;
        }
        fdMap_[stderrFd] = FdInfo {sessionId, false};
    }

    if (stdinFd >= 0) {
        int flags = fcntl(stdinFd, F_GETFL, 0);
        if (flags >= 0) {
            fcntl(stdinFd, F_SETFL, flags | O_NONBLOCK);
        }
        fdMap_[stdinFd] = FdInfo {sessionId, false, true};
    }
    return true;
}

void IOMonitor::UnregisterSession(const std::string &sessionId)
{
    std::vector<std::pair<int, FdInfo>> fdsToClose;
    std::vector<PendingInput> failedInputs;
    {
        std::lock_guard<std::mutex> lock(fdMutex_);
        for (auto it = fdMap_.begin(); it != fdMap_.end();) {
            if (it->second.sessionId == sessionId) {
                if (!it->second.isStdin) {
                    // epoll_ctl reads epollFd_; keep it under fdMutex_ to synchronize with
                    // Stop()'s close(epollFd_) and RegisterSession()'s epoll_ctl().
                    epoll_ctl(epollFd_, EPOLL_CTL_DEL, it->first, nullptr);
                }
                fdsToClose.emplace_back(it->first, it->second);
                it = fdMap_.erase(it);
                continue;
            }
            ++it;
        }
        auto queueIt = inputQueues_.find(sessionId);
        if (queueIt != inputQueues_.end()) {
            while (!queueIt->second.pendingInputs.empty()) {
                failedInputs.emplace_back(std::move(queueIt->second.pendingInputs.front()));
                queueIt->second.pendingInputs.pop_front();
            }
            inputQueues_.erase(queueIt);
        }
    }

    for (const auto &[fd, info] : fdsToClose) {
        close(fd);
    }
    for (const auto &input : failedInputs) {
        NotifyInputReply(sessionId, input.eventId, false);
    }
}

void IOMonitor::SetOutputCallback(OutputCallback callback)
{
    outputCallback_ = std::move(callback);
}

void IOMonitor::SetInputReplyCallback(InputReplyCallback callback)
{
    inputReplyCallback_ = std::move(callback);
}

void IOMonitor::SetSessionClosedCallback(SessionClosedCallback callback)
{
    sessionClosedCallback_ = std::move(callback);
}

void IOMonitor::SetSessionDrainedCallback(SessionDrainedCallback callback)
{
    sessionDrainedCallback_ = std::move(callback);
}

int IOMonitor::GetStdinFd(const std::string &sessionId)
{
    std::lock_guard<std::mutex> lock(fdMutex_);
    return GetStdinFdLocked(sessionId);
}

int IOMonitor::GetStdinFdLocked(const std::string &sessionId) const
{
    auto it = fdMap_.begin();
    while (it != fdMap_.end()) {
        if (it->second.sessionId == sessionId && it->second.isStdin) {
            break;
        }
        ++it;
    }

    if (it == fdMap_.end()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed: sessionId=%{public}s stdin not found", sessionId.c_str());
        return -1;
    }

    return it->first;
}

bool IOMonitor::WriteMessage(int fd, const std::string &sessionId, const std::string &message)
{
    if (message.empty()) {
        return true;
    }
    const char* data = message.c_str();
    size_t totalBytes = message.size();
    size_t bytesWritten = 0;
    auto beginTime = std::chrono::steady_clock::now();
    while (bytesWritten < totalBytes) {
        ssize_t writeResult = write(fd, data + bytesWritten, totalBytes - bytesWritten);
        if (writeResult == -1) {
            if (errno == EINTR) {
                continue;
            }
            if (errno != EAGAIN && errno != EWOULDBLOCK) {
                TAG_LOGE(AAFwkTag::CLI_TOOL,
                    "WriteMessage failed: write error=%{public}s for sessionId=%{public}s",
                    strerror(errno), sessionId.c_str());
                return false;
            }

            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - beginTime).count();
            if (elapsed >= INPUT_WRITE_TIMEOUT_MS) {
                TAG_LOGE(AAFwkTag::CLI_TOOL,
                    "WriteMessage failed: wait writable timeout for sessionId=%{public}s, "
                    "wrote=%{public}zu/%{public}zu",
                    sessionId.c_str(), bytesWritten, totalBytes);
                return false;
            }

            pollfd pollFd {};
            pollFd.fd = fd;
            pollFd.events = POLLOUT;
            int32_t pollTimeout = std::min<int64_t>(INPUT_WRITE_POLL_MS, INPUT_WRITE_TIMEOUT_MS - elapsed);
            int32_t pollResult = poll(&pollFd, 1, pollTimeout);
            if (pollResult < 0) {
                if (errno == EINTR) {
                    continue;
                }
                TAG_LOGE(AAFwkTag::CLI_TOOL,
                    "WriteMessage failed: poll error=%{public}s for sessionId=%{public}s",
                    strerror(errno), sessionId.c_str());
                return false;
            }
            if (pollResult == 0) {
                continue;
            }
            if ((pollFd.revents & (POLLERR | POLLHUP | POLLNVAL)) != 0) {
                TAG_LOGE(AAFwkTag::CLI_TOOL,
                    "WriteMessage failed: poll revents=%{public}d for sessionId=%{public}s",
                    pollFd.revents, sessionId.c_str());
                return false;
            }
            continue;
        }
        if (writeResult == 0) {
            TAG_LOGE(AAFwkTag::CLI_TOOL,
                "WriteMessage failed: pipe closed for sessionId=%{public}s",
                sessionId.c_str());
            return false;
        }

        bytesWritten += writeResult;
    }
    return true;
}

void IOMonitor::NotifyInputReply(const std::string &sessionId, const std::string &eventId, bool result)
{
    if (inputReplyCallback_) {
        inputReplyCallback_(sessionId, eventId, result);
    }
}

void IOMonitor::SendMessage(const std::string &sessionId, const std::string &message, const std::string &eventId)
{
    bool shouldSubmit = false;
    bool rejected = false;
    {
        std::lock_guard<std::mutex> lock(fdMutex_);
        if (GetStdinFdLocked(sessionId) < 0) {
            rejected = true;
        } else {
            auto &queue = inputQueues_[sessionId];
            if (queue.pendingBytes + message.size() > MAX_PENDING_INPUT_BYTES ||
                queue.pendingInputs.size() >= MAX_PENDING_INPUT_MESSAGES) {
                TAG_LOGW(AAFwkTag::CLI_TOOL,
                    "SendMessage failed: input queue full for sessionId=%{public}s, pendingBytes=%{public}zu, "
                    "pendingMessages=%{public}zu",
                    sessionId.c_str(), queue.pendingBytes, queue.pendingInputs.size());
                rejected = true;
            } else {
                queue.pendingInputs.emplace_back(PendingInput {message, eventId});
                queue.pendingBytes += message.size();
                if (!queue.writeTaskRunning) {
                    queue.writeTaskRunning = true;
                    shouldSubmit = true;
                }
            }
        }
    }

    if (rejected) {
        NotifyInputReply(sessionId, eventId, false);
        return;
    }

    if (!shouldSubmit) {
        return;
    }

    auto writeTask = [weak = weak_from_this(), sessionId]() {
        auto sharedThis = weak.lock();
        if (sharedThis) {
            sharedThis->ProcessWriteQueue(sessionId);
        }
    };
    ffrt::submit(std::move(writeTask));
}

void IOMonitor::ProcessWriteQueue(const std::string &sessionId)
{
    while (true) {
        PendingInput input;
        int fd = -1;
        {
            std::lock_guard<std::mutex> lock(fdMutex_);
            auto queueIt = inputQueues_.find(sessionId);
            if (queueIt == inputQueues_.end() || queueIt->second.pendingInputs.empty()) {
                if (queueIt != inputQueues_.end()) {
                    queueIt->second.writeTaskRunning = false;
                    if (queueIt->second.pendingBytes == 0) {
                        inputQueues_.erase(queueIt);
                    }
                }
                return;
            }

            input = std::move(queueIt->second.pendingInputs.front());
            queueIt->second.pendingInputs.pop_front();
            queueIt->second.pendingBytes -= input.message.size();
            // Take a private dup of the stdin fd so UnregisterSession()/Stop() closing the
            // original cannot invalidate or fd-recycle the handle we write to outside the lock.
            int stdinFd = GetStdinFdLocked(sessionId);
            if (stdinFd >= 0) {
                fd = dup(stdinFd);
                if (fd < 0) {
                    TAG_LOGE(AAFwkTag::CLI_TOOL,
                        "ProcessWriteQueue: dup stdin failed: %{public}s for sessionId=%{public}s",
                        strerror(errno), sessionId.c_str());
                }
            }
        }

        bool result = fd >= 0 && WriteMessage(fd, sessionId, input.message);
        if (fd >= 0) {
            close(fd);
        }
        NotifyInputReply(sessionId, input.eventId, result);
        if (!result) {
            std::vector<PendingInput> failedInputs;
            {
                std::lock_guard<std::mutex> lock(fdMutex_);
                auto queueIt = inputQueues_.find(sessionId);
                if (queueIt != inputQueues_.end()) {
                    while (!queueIt->second.pendingInputs.empty()) {
                        failedInputs.emplace_back(std::move(queueIt->second.pendingInputs.front()));
                        queueIt->second.pendingInputs.pop_front();
                    }
                    inputQueues_.erase(queueIt);
                }
            }
            for (const auto &failedInput : failedInputs) {
                NotifyInputReply(sessionId, failedInput.eventId, false);
            }
            return;
        }
    }
}

void IOMonitor::MonitorLoop()
{
    epoll_event events[MAX_EVENTS];
    while (running_.load(std::memory_order_acquire)) {
        int nfds = epoll_wait(epollFd_, events, MAX_EVENTS, EPOLL_WAIT_MS);
        if (nfds < 0) {
            if (errno == EINTR) {
                continue;
            }
            if (!running_.load(std::memory_order_acquire)) {
                break;
            }
            TAG_LOGW(AAFwkTag::CLI_TOOL, "epoll_wait failed: %{public}s", strerror(errno));
            continue;
        }

        for (int i = 0; i < nfds; ++i) {
            HandleReadableFd(events[i].data.fd);
        }
    }
}

void IOMonitor::HandleReadableFd(int fd)
{
    FdInfo info;
    int readFd = fd; // read the original fd directly when dup is unavailable
    bool ownReadFd = false; // true only when readFd is a private dup we must close
    {
        std::lock_guard<std::mutex> lock(fdMutex_);
        auto it = fdMap_.find(fd);
        if (it == fdMap_.end()) {
            return;
        }
        info = it->second;
        // Take a private dup so UnregisterSession()/Stop() closing the original cannot
        // invalidate or fd-recycle the handle we read from outside the lock. On dup failure
        // (rare, fd exhaustion) fall back to the original fd to avoid busy-spinning under
        // level-triggered epoll while still consuming data.
        int dupFd = dup(fd);
        if (dupFd >= 0) {
            readFd = dupFd;
            ownReadFd = true;
        } else {
            TAG_LOGW(AAFwkTag::CLI_TOOL,
                "HandleReadableFd: dup failed: %{public}s, reading original fd directly", strerror(errno));
        }
    }

    char buffer[CLI_IO_READ_BUFFER_SIZE];
    while (true) {
        ssize_t bytesRead = read(readFd, buffer, sizeof(buffer));
        if (bytesRead > 0) {
            if (outputCallback_) {
                outputCallback_(info.sessionId, info.isStdout, std::string(buffer, bytesRead));
            }
            continue;
        }
        if (bytesRead == 0) {
            CloseFdLocked(fd, info, true);
            break;
        }
        if (errno == EINTR) {
            continue;
        }
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            break;
        }
        CloseFdLocked(fd, info, true);
        break;
    }
    if (ownReadFd) {
        close(readFd);
    }
}

void IOMonitor::CloseFdLocked(int fd, const FdInfo &info, bool notifyDrained)
{
    bool sessionDrained = false;
    {
        std::lock_guard<std::mutex> lock(fdMutex_);
        auto it = fdMap_.find(fd);
        if (it == fdMap_.end() || it->second.sessionId != info.sessionId) {
            // fd absent, or its number was recycled to a different session after info was
            // captured; do not deregister/close a fd that no longer belongs to this session.
            return;
        }
        epoll_ctl(epollFd_, EPOLL_CTL_DEL, fd, nullptr);
        close(fd);
        fdMap_.erase(it);
        sessionDrained = true;
        for (const auto &[otherFd, otherInfo] : fdMap_) {
            if (otherInfo.sessionId == info.sessionId) {
                sessionDrained = false;
                break;
            }
        }
    }
    if (sessionClosedCallback_) {
        sessionClosedCallback_(info.sessionId, info.isStdout);
    }
    if (notifyDrained && sessionDrained && sessionDrainedCallback_) {
        sessionDrainedCallback_(info.sessionId);
    }
}

} // namespace CliTool
} // namespace OHOS
