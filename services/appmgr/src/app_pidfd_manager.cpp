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

#include "app_pidfd_manager.h"

#include <fcntl.h>
#include <sys/epoll.h>
#include <sys/syscall.h>
#include <unistd.h>

#include "c/executor_task.h"
#include "ffrt.h"
#include "ffrt_inner.h"

#include "app_mgr_service_inner.h"
#include "hilog_tag_wrapper.h"
#include "task_handler_wrap.h"

#ifndef PIDFD_NONBLOCK
#define PIDFD_NONBLOCK O_NONBLOCK
#endif

namespace {
const char *TASK_ON_CHILD_PROCESS_EXITED = "OnChildProcessExitedTask";
} // namespace

namespace OHOS {
namespace AppExecFwk {

AppPidFdManager &AppPidFdManager::GetInstance()
{
    static AppPidFdManager manager;
    return manager;
}

AppPidFdManager::AppPidFdManager() {}

AppPidFdManager::~AppPidFdManager()
{
    std::lock_guard<std::mutex> lock(mapMutex_);
    TAG_LOGW(AAFwkTag::APPMGR, "AppPidFdManager destroy");
    for (auto &item : pidfdMap_) {
        if (item.second.pidfd >= 0) {
            ffrt_qos_t qos = 0;
            ffrt_epoll_ctl(qos, EPOLL_CTL_DEL, item.second.pidfd, 0, nullptr, nullptr);
            close(item.second.pidfd);
        }
    }
    pidfdMap_.clear();
}

void AppPidFdManager::Init(std::weak_ptr<AppMgrServiceInner> appMgrServiceInner,
    std::weak_ptr<AAFwk::TaskHandlerWrap> taskHandler)
{
    appMgrServiceInner_ = appMgrServiceInner;
    taskHandler_ = taskHandler;
    TAG_LOGI(AAFwkTag::APPMGR, "AppPidFdManager init");
}

int32_t AppPidFdManager::OpenPidFd(pid_t pid, unsigned int flags)
{
    return static_cast<int32_t>(syscall(SYS_pidfd_open, pid, flags));
}

void AppPidFdManager::AddWatcher(pid_t pid, PidFdType type)
{
    if (pid <= 0) {
        TAG_LOGE(AAFwkTag::APPMGR, "invalid pid");
        return;
    }
    int32_t pidfd = OpenPidFd(pid, PIDFD_NONBLOCK);
    if (pidfd < 0) {
        // The process has already exited (ESRCH) before we could open the pidfd.
        // Treat it as a death and dispatch the cleanup directly.
        TAG_LOGW(AAFwkTag::APPMGR, "pidfd_open failed, pid:%{public}d, errno:%{public}d", pid, errno);
        DispatchCleanup(pid, type);
        return;
    }

    // The pid is stored in the map entry itself; the callback reads it via the
    // stable address of entry->second.pid (std::map nodes do not relocate).
    void *data = nullptr;
    {
        std::lock_guard<std::mutex> lock(mapMutex_);
        auto it = pidfdMap_.find(pid);
        if (it != pidfdMap_.end()) {
            TAG_LOGW(AAFwkTag::APPMGR, "pid already exist:%{public}d", pid);
            // Should not happen; replace the stale entry defensively.
            ffrt_qos_t qos = 0;
            ffrt_epoll_ctl(qos, EPOLL_CTL_DEL, it->second.pidfd, 0, nullptr, nullptr);
            close(it->second.pidfd);
            it->second.pidfd = pidfd;
            it->second.pid = static_cast<int32_t>(pid);
            data = &it->second.pid;
        } else {
            auto res = pidfdMap_.emplace(pid, PidFdEntry{pidfd, static_cast<int32_t>(pid)});
            data = &res.first->second.pid;
        }
    }

    ffrt_qos_t qos = 0;
    auto cb = (type == PidFdType::RENDER) ? &AppPidFdManager::OnRenderPidfdEvent
                                          : &AppPidFdManager::OnChildPidfdEvent;
    int32_t ret = ffrt_epoll_ctl(qos, EPOLL_CTL_ADD, pidfd, EPOLLIN, data, cb);
    if (ret != 0) {
        TAG_LOGE(AAFwkTag::APPMGR, "ffrt_epoll_ctl add failed, pid:%{public}d, ret:%{public}d", pid, ret);
        std::lock_guard<std::mutex> lock(mapMutex_);
        auto it = pidfdMap_.find(pid);
        if (it != pidfdMap_.end() && it->second.pidfd == pidfd) {
            pidfdMap_.erase(it);
        }
        close(pidfd);
        DispatchCleanup(pid, type);
        return;
    }
    TAG_LOGI(AAFwkTag::APPMGR, "watch process pid:%{public}d, pidfd:%{public}d, type:%{public}d",
        pid, pidfd, static_cast<int32_t>(type));
}

void AppPidFdManager::OnChildPidfdEvent(void *data, uint32_t event)
{
    if (data == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null pid data");
        return;
    }
    pid_t pid = static_cast<pid_t>(*static_cast<int32_t *>(data));
    TAG_LOGI(AAFwkTag::APPMGR, "OnChildPidfdEvent pid:%{public}d, event:%{public}u", pid, event);
    GetInstance().OnPidfdFired(pid, PidFdType::CHILD);
}

void AppPidFdManager::OnRenderPidfdEvent(void *data, uint32_t event)
{
    if (data == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null pid data");
        return;
    }
    pid_t pid = static_cast<pid_t>(*static_cast<int32_t *>(data));
    TAG_LOGI(AAFwkTag::APPMGR, "OnRenderPidfdEvent pid:%{public}d, event:%{public}u", pid, event);
    GetInstance().OnPidfdFired(pid, PidFdType::RENDER);
}

void AppPidFdManager::OnPidfdFired(pid_t pid, PidFdType type)
{
    // fd/token lifecycle is handled here on the poller thread, so it never races
    // with a free from another thread (mirrors appspawn ClosePidfdWatcher).
    int32_t pidfd = -1;
    {
        std::lock_guard<std::mutex> lock(mapMutex_);
        auto it = pidfdMap_.find(pid);
        if (it == pidfdMap_.end()) {
            return;
        }
        pidfd = it->second.pidfd;
        pidfdMap_.erase(it);
    }
    if (pidfd >= 0) {
        ffrt_qos_t qos = 0;
        ffrt_epoll_ctl(qos, EPOLL_CTL_DEL, pidfd, 0, nullptr, nullptr);
        close(pidfd);
    }
    DispatchCleanup(pid, type);
}

void AppPidFdManager::DispatchCleanup(pid_t pid, PidFdType type)
{
    auto inner = appMgrServiceInner_.lock();
    auto handler = taskHandler_.lock();
    if (!inner || !handler) {
        TAG_LOGE(AAFwkTag::APPMGR, "null inner or handler, pid:%{public}d", pid);
        return;
    }
    auto task = [inner, pid, type]() {
        if (type == PidFdType::RENDER) {
            inner->OnRenderProcessExited(pid);
        } else {
#ifdef SUPPORT_CHILD_PROCESS
            inner->OnChildProcessExited(pid);
#endif
        }
    };
    handler->SubmitTask(task, AAFwk::TaskAttribute{
        .taskName_ = TASK_ON_CHILD_PROCESS_EXITED,
        .taskQos_ = AAFwk::TaskQoS::USER_INTERACTIVE
    });
}
} // namespace AppExecFwk
} // namespace OHOS
