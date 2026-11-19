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
#include <csignal>
#include <cerrno>
#include <fcntl.h>
#include <limits>
#include <shared_mutex>
#include <unordered_map>
#include <unistd.h>
#include <map>
#include <memory>
#include <mutex>
#include <sys/epoll.h>
#include <sys/wait.h>
#include <poll.h>
#define private public
#include "app_pidfd_manager.h"
#include "app_mgr_service_inner.h"
#include "launch_service_manager.h"
#undef private
#include "task_handler_wrap.h"

using namespace testing::ext;
namespace OHOS::AppExecFwk {
class LaunchServicePidfdTest : public testing::Test {
public:
    void SetUp() override
    {
        manager_.Init({}, {});
    }
    void TearDown() override
    {
        while (!manager_.pidfdMap_.empty()) {
            manager_.OnPidfdFired(manager_.pidfdMap_.begin()->first, PidFdType::LAUNCH_SERVICE);
        }
        manager_.Init({}, {});
    }
    AppPidFdManager &manager_ = AppPidFdManager::GetInstance();
};

/**
 * @tc.name: AddWatcher_InvalidPid_001
 * @tc.desc: Invalid process IDs return no descriptor and create no watcher.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServicePidfdTest, AddWatcher_InvalidPid_001, TestSize.Level1)
{
    EXPECT_FALSE(manager_.AddWatcher(0, PidFdType::LAUNCH_SERVICE));
    EXPECT_FALSE(manager_.AddWatcher(-1, PidFdType::LAUNCH_SERVICE));
    EXPECT_TRUE(manager_.pidfdMap_.empty());
}

/**
 * @tc.name: Init_OriginalHandler_001
 * @tc.desc: Child and render cleanup retains the original appmgr handler.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServicePidfdTest, Init_OriginalHandler_001, TestSize.Level1)
{
    auto original = AAFwk::TaskHandlerWrap::CreateQueueHandler("pidfd_original_test");
    manager_.Init({}, original);
    EXPECT_EQ(manager_.taskHandler_.lock(), original);
    manager_.OnLaunchServicePidfdEvent(nullptr, EPOLLIN);
}

/**
 * @tc.name: KillWatchedProcess_WatcherError_001
 * @tc.desc: A bad watcher fd must not report a successful stop.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServicePidfdTest, KillWatchedProcess_WatcherError_001, TestSize.Level1)
{
    const pid_t pid = getpid();
    // -1 forces EBADF without risking a signal to the test process.
    manager_.pidfdMap_.emplace(pid, AppPidFdManager::PidFdEntry{-1, pid});
    EXPECT_EQ(manager_.IsWatchedProcessAlive(pid), EBADF);
    EXPECT_FALSE(manager_.KillWatchedProcess(pid));
    manager_.pidfdMap_.erase(pid);
}

/**
 * @tc.name: KillWatchedProcess_MissingProcess_001
 * @tc.desc: Opening a transient pidfd for a missing process returns failure.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServicePidfdTest, KillWatchedProcess_MissingProcess_001, TestSize.Level1)
{
    const pid_t pid = std::numeric_limits<pid_t>::max();
    // Establish ESRCH explicitly so an unsupported pidfd syscall cannot pass the test.
    errno = 0;
    const int fd = manager_.OpenPidFd(pid, 0);
    const int openError = errno;
    if (fd >= 0) {
        close(fd);
    }
    ASSERT_LT(fd, 0);
    ASSERT_EQ(openError, ESRCH);
    EXPECT_FALSE(manager_.KillWatchedProcess(pid));
    EXPECT_TRUE(manager_.pidfdMap_.empty());
}

/**
 * @tc.name: KillWatchedProcess_TransientPidfd_001
 * @tc.desc: Stop a child through a transient pidfd without registering a watcher.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServicePidfdTest, KillWatchedProcess_TransientPidfd_001, TestSize.Level1)
{
    const pid_t child = fork();
    ASSERT_GE(child, 0);
    if (child == 0) {
        for (;;) {
            pause();
        }
    }
    // Reap the child even if a later ASSERT aborts this test body.
    struct ChildGuard {
        pid_t pid;
        ~ChildGuard()
        {
            kill(pid, SIGKILL);
            while (waitpid(pid, nullptr, 0) < 0 && errno == EINTR) {}
        }
    } guard {child};
    const int fd = manager_.OpenPidFd(child, 0);
    ASSERT_GE(fd, 0);
    EXPECT_TRUE(manager_.KillWatchedProcess(child));
    pollfd event {fd, POLLIN, 0};
    const int result = poll(&event, 1, 3000);
    EXPECT_EQ(result, 1);
    EXPECT_NE(event.revents & POLLIN, 0);
    EXPECT_EQ(close(fd), 0);
    EXPECT_TRUE(manager_.pidfdMap_.empty());
}

/**
 * @tc.name: AddWatcher_OwnsPidfd_001
 * @tc.desc: Registration owns the fd and cleanup closes it without terminating the process.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServicePidfdTest, AddWatcher_OwnsPidfd_001, TestSize.Level1)
{
    const pid_t pid = getpid();
    ASSERT_TRUE(manager_.AddWatcher(pid, PidFdType::LAUNCH_SERVICE));
    ASSERT_NE(manager_.pidfdMap_.find(pid), manager_.pidfdMap_.end());
    const int32_t pidfd = manager_.pidfdMap_.at(pid).pidfd;
    ASSERT_GE(pidfd, 0);
    EXPECT_NE(fcntl(pidfd, F_GETFD), -1);
    EXPECT_EQ(manager_.IsWatchedProcessAlive(pid), 0);
    manager_.OnPidfdFired(pid, PidFdType::LAUNCH_SERVICE);
    EXPECT_EQ(manager_.pidfdMap_.count(pid), 0u);
    EXPECT_EQ(fcntl(pidfd, F_GETFD), -1);
    EXPECT_EQ(errno, EBADF);
    EXPECT_EQ(manager_.IsWatchedProcessAlive(pid), ESRCH);
}
/**
 * @tc.name: IsWatchedProcessAlive_InvalidOrUnwatched_001
 * @tc.desc: Invalid PIDs and a live process without a watcher return ESRCH.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServicePidfdTest, IsWatchedProcessAlive_InvalidOrUnwatched_001, TestSize.Level1)
{
    EXPECT_EQ(manager_.IsWatchedProcessAlive(0), ESRCH);
    EXPECT_EQ(manager_.IsWatchedProcessAlive(-1), ESRCH);
    EXPECT_EQ(manager_.IsWatchedProcessAlive(getpid()), ESRCH);
}
} // namespace OHOS::AppExecFwk
