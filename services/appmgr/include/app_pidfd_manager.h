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

#ifndef OHOS_APP_PIDFD_MANAGER_H
#define OHOS_APP_PIDFD_MANAGER_H

#include <map>
#include <memory>
#include <mutex>
#include <sys/types.h>

#include "nocopyable.h"

namespace OHOS {
namespace AAFwk {
class TaskHandlerWrap;
} // namespace AAFwk
namespace AppExecFwk {
class AppMgrServiceInner;

/**
 * @class AppPidFdManager
 * @brief Monitors spawned process exit by pidfd, replacing binder death recipient.
 *
 * After appspawn creates a process, the manager opens a pidfd for the pid and
 * registers it with ffrt_epoll_ctl (callback mode). When the process exits the
 * pidfd becomes readable, the ffrt poller invokes the callback on its own
 * thread; the fd/token lifecycle is fully handled on the poller thread (mirrors
 * appspawn ClosePidfdWatcher), and the record-level cleanup is dispatched onto
 * the appmgr task queue, keeping it consistent with the previous binder
 * OnRemoteDied scheduling.
 *
 * Naming is generic on purpose: it is reused for child and render process
 * death monitoring; main process may follow.
 */

enum class PidFdType { CHILD, RENDER };

class AppPidFdManager {
public:
    static AppPidFdManager &GetInstance();

    ~AppPidFdManager();

    /**
     * @brief Initialize with the appmgr service inner and its task handler.
     */
    void Init(std::weak_ptr<AppMgrServiceInner> appMgrServiceInner,
        std::weak_ptr<AAFwk::TaskHandlerWrap> taskHandler);

    /**
     * @brief Open a pidfd for the given pid and start watching its exit.
     *
     * Called right after appspawn/nwebspawn returns the pid (before Attach*).
     * If pidfd_open fails (the process already exited, ESRCH), the cleanup task
     * is dispatched directly. The type selects which exit handler is dispatched.
     */
    void AddWatcher(pid_t pid, PidFdType type);

private:
    AppPidFdManager();

    struct PidFdEntry {
        int32_t pidfd = -1;
        int32_t pid = 0;
    };

    // Invoked on the ffrt poller thread when a watched pidfd becomes readable.
    // The process type is encoded by which static callback is registered, so the
    // token only carries the pid.
    static void OnChildPidfdEvent(void *data, uint32_t event);
    static void OnRenderPidfdEvent(void *data, uint32_t event);

    // Handles the pidfd/token lifecycle on the poller thread, then dispatches
    // the record-level cleanup onto the appmgr task queue.
    void OnPidfdFired(pid_t pid, PidFdType type);

    void DispatchCleanup(pid_t pid, PidFdType type);

    static int32_t OpenPidFd(pid_t pid, unsigned int flags);

    std::map<pid_t, PidFdEntry> pidfdMap_;
    std::mutex mapMutex_;
    std::weak_ptr<AppMgrServiceInner> appMgrServiceInner_;
    std::weak_ptr<AAFwk::TaskHandlerWrap> taskHandler_;

    DISALLOW_COPY_AND_MOVE(AppPidFdManager);
};
} // namespace AppExecFwk
} // namespace OHOS
#endif // OHOS_APP_PIDFD_MANAGER_H
