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

#include <cstring>
#include <cerrno>
#include <fcntl.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <vector>

#include "ffrt.h"
#include "hilog_tag_wrapper.h"

#define CLI_IO_READ_BUFFER_SIZE 4096

namespace OHOS {
namespace CliTool {

namespace {
constexpr int32_t MAX_EVENTS = 16;
constexpr int32_t EPOLL_WAIT_MS = 100;
constexpr int32_t MAX_RETRIES = 10;
constexpr int32_t RETRY_DELAY_MS = 10;
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
    monitorThread_ = std::thread(&IOMonitor::MonitorLoop, this);
    return true;
}

void IOMonitor::Stop()
{
    if (!running_.exchange(false)) {
        return;
    }
    if (epollFd_ >= 0) {
        close(epollFd_);
        epollFd_ = -1;
    }
    if (monitorThread_.joinable()) {
        monitorThread_.join();
    }

    std::lock_guard<std::mutex> lock(fdMutex_);
    for (const auto &[fd, info] : fdMap_) {
        close(fd);
    }
    fdMap_.clear();
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
    {
        std::lock_guard<std::mutex> lock(fdMutex_);
        for (auto it = fdMap_.begin(); it != fdMap_.end();) {
            if (it->second.sessionId == sessionId) {
                fdsToClose.emplace_back(it->first, it->second);
                it = fdMap_.erase(it);
                continue;
            }
            ++it;
        }
    }

    for (const auto &[fd, info] : fdsToClose) {
        if (info.isStdin == false) {
            epoll_ctl(epollFd_, EPOLL_CTL_DEL, fd, nullptr);
        }
        close(fd);
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

void IOMonitor::WriteTask(const std::string &sessionId, const std::string &message, const std::string &eventId)
{
    int fd = GetStdinFd(sessionId);
    if (fd < 0) {
        if (inputReplyCallback_) {
            inputReplyCallback_(sessionId, eventId, false);
        }
        return;
    }
    bool result = true;
    const char* data = message.c_str();
    size_t totalBytes = message.size();
    size_t bytesWritten = 0;
    int retryCount = 0;
    while (bytesWritten < totalBytes && retryCount < MAX_RETRIES) {
        ssize_t writeResult = write(fd, data + bytesWritten, totalBytes - bytesWritten);
        if (writeResult == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                std::this_thread::sleep_for(std::chrono::milliseconds(RETRY_DELAY_MS));
                retryCount++;
                continue;
            } else {
                TAG_LOGE(AAFwkTag::CLI_TOOL,
                    "WriteTask failed: write error=%{public}s for sessionId=%{public}s",
                    strerror(errno), sessionId.c_str());
                result = false;
                break;
            }
        } else if (writeResult == 0) {
            TAG_LOGE(AAFwkTag::CLI_TOOL,
                "WriteTask failed: pipe closed for sessionId=%{public}s",
                sessionId.c_str());
            result = false;
            break;
        }

        bytesWritten += writeResult;
        retryCount = 0;
    }
    if (bytesWritten < totalBytes) {
        result = false;
    }
    if (result == false) {
        TAG_LOGW(AAFwkTag::CLI_TOOL, "WriteTask: partial write for sessionId=%{public}s, "
            "wrote=%{public}zu/%{public}zu", sessionId.c_str(), bytesWritten, totalBytes);
    }
    if (inputReplyCallback_) {
        inputReplyCallback_(sessionId, eventId, result);
    }
}

void IOMonitor::SendMessage(const std::string &sessionId, const std::string &message, const std::string &eventId)
{
    auto writeTask = [weak = weak_from_this(), sessionId, message, eventId]() {
        auto sharedThis = weak.lock();
        if (sharedThis) {
            sharedThis->WriteTask(sessionId, message, eventId);
        }
    };
    ffrt::submit(std::move(writeTask));
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
    {
        std::lock_guard<std::mutex> lock(fdMutex_);
        auto it = fdMap_.find(fd);
        if (it == fdMap_.end()) {
            return;
        }
        info = it->second;
    }

    char buffer[CLI_IO_READ_BUFFER_SIZE];
    while (true) {
        ssize_t bytesRead = read(fd, buffer, sizeof(buffer));
        if (bytesRead > 0) {
            if (outputCallback_) {
                outputCallback_(info.sessionId, info.isStdout, std::string(buffer, bytesRead));
            }
            continue;
        }
        if (bytesRead == 0) {
            CloseFdLocked(fd, info, true);
            return;
        }
        if (errno == EINTR) {
            continue;
        }
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return;
        }
        CloseFdLocked(fd, info, true);
        return;
    }
}

void IOMonitor::CloseFdLocked(int fd, const FdInfo &info, bool notifyDrained)
{
    bool sessionDrained = false;
    {
        std::lock_guard<std::mutex> lock(fdMutex_);
        auto it = fdMap_.find(fd);
        if (it == fdMap_.end()) {
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
