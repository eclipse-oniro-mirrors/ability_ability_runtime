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

#ifndef OHOS_SUB_SPAWN_MANAGER_H
#define OHOS_SUB_SPAWN_MANAGER_H

#include <chrono>
#include <map>
#include <mutex>
#include <string>
#include <vector>

#include "app_spawn_client.h"
#include "child_process_options.h"
#include "nocopyable.h"

namespace OHOS {
namespace AppExecFwk {

class AppRunningRecord;

/**
 * @class SubSpawnManager
 * @brief Manages per-app subspawn process lifecycle for native child process
 * creation with shared .so memory.
 *
 * Subspawn is forked by appspawn with the application's UID. It preloads the
 * application's .so files (from preloadSoList configured in app.json5). Child
 * processes forked from subspawn inherit the preloaded .so memory, reducing
 * RSS overhead.
 *
 * Lifecycle:
 * - Created on first isolated child process request when preloadSoList is
 *   configured (async via ffrt::submitTask, does not block child creation).
 * - Reused for subsequent child process fork requests.
 * - Cleaned up when: app process dies, or 30s idle after last child exits.
 */
class SubSpawnManager {
public:
    static SubSpawnManager &GetInstance();

    /**
     * @brief Check whether subspawn path should be used for this child process.
     * Conditions: device supports subspawn, app configured preloadSoList,
     * and child process is in isolated mode.
     */
    bool ShouldUseSubSpawn(const std::vector<std::string> &preloadSoList, bool isolationMode);

    /**
     * @brief Attempt to start a child process via subspawn.
     * @param appPid The parent application process pid.
     * @param uid The uid for the subspawn process.
     * @param startMsg The spawn message containing entry, fds, args, etc.
     * @return child pid > 0 on success; <= 0 if subspawn not available
     *         (caller should fallback to nativespawn).
     */
    pid_t StartChildViaSubSpawn(pid_t appPid, int32_t uid, const AppSpawnStartMsg &startMsg);

    /**
     * @brief Asynchronously create a subspawn instance for the given app.
     * Does not block the caller. Uses ffrt::submitTask.
     */
    void AsyncCreateSubSpawn(pid_t appPid, int32_t uid, const std::string &bundleName,
                             const AppSpawnStartMsg &startMsg);

    /**
     * @brief Called when a subspawn process exits (detected via pidfd).
     */
    void OnSubSpawnExited(pid_t subSpawnPid);

    /**
     * @brief Called when a child process is created via subspawn.
     */
    void OnChildProcessCreated(pid_t appPid);

    /**
     * @brief Called when a child process (created via subspawn) exits.
     */
    void OnChildProcessExited(pid_t appPid);

    /**
     * @brief Called when the parent application process dies.
     * Kills the corresponding subspawn and cleans up.
     */
    void OnAppProcessDied(pid_t appPid);

    /**
     * @brief Check whether the given pid is a subspawn process.
     */
    bool IsSubSpawn(pid_t pid);

private:
    SubSpawnManager();
    ~SubSpawnManager();
    DISALLOW_COPY_AND_MOVE(SubSpawnManager);

    static constexpr int32_t IDLE_TIMEOUT_MS = 30000;

    struct SubSpawnRecord {
        pid_t subSpawnPid = 0;
        pid_t appPid = 0;
        std::string bundleName;
        int32_t uid = 0;
        int32_t childProcessCount = 0;
        bool idleTimerActive = false;
        bool terminating = false;
        std::string socketName;
    };

    void StartIdleTimer(pid_t appPid);
    void CancelIdleTimer(pid_t appPid);
    void HandleIdleTimeout(pid_t appPid);

    std::string QuerySocketNameByUid(int32_t uid);
    void SendSubSpawnShutdown(const std::string &bundleName, pid_t subSpawnPid);

    pid_t DoCreateSubSpawn(pid_t appPid, int32_t uid, const std::string &bundleName,
                           const AppSpawnStartMsg &startMsg);

    std::map<pid_t /*appPid*/, SubSpawnRecord> subSpawnMap_;
    std::map<pid_t /*subSpawnPid*/, pid_t /*appPid*/> pidToAppMap_;
    std::mutex mutex_;
};

} // namespace AppExecFwk
} // namespace OHOS

#endif // OHOS_SUB_SPAWN_MANAGER_H
