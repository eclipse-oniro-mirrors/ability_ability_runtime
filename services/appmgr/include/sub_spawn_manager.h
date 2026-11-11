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
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "app_spawn_client.h"
#include "child_process_options.h"
#include "nocopyable.h"

namespace OHOS {
namespace AppExecFwk {

class AppRunningRecord;

class SubSpawnRecord {
public:
    static std::shared_ptr<SubSpawnRecord> CreateSubSpawnRecord(pid_t appPid, int32_t uid,
        const std::string &bundleName);

    pid_t GetSubSpawnPid() const;
    void SetSubSpawnPid(pid_t pid);
    pid_t GetAppPid() const;
    int32_t GetUid() const;
    std::string GetBundleName() const;
    std::string GetSocketName() const;
    void SetSocketName(const std::string &name);
    int32_t GetChildProcessCount() const;
    void IncrementChildProcessCount();
    void DecrementChildProcessCount();
    bool IsIdleTimerActive() const;
    void SetIdleTimerActive(bool active);
    bool IsTerminating() const;
    void SetTerminating(bool terminating);
    bool IsCreating() const;
    void SetCreating(bool creating);
    bool IsReady() const;

private:
    SubSpawnRecord(pid_t appPid, int32_t uid, const std::string &bundleName);
    DISALLOW_COPY_AND_MOVE(SubSpawnRecord);

    pid_t subSpawnPid_ = 0;
    pid_t appPid_ = 0;
    int32_t uid_ = 0;
    std::string bundleName_;
    std::string socketName_;
    int32_t childProcessCount_ = 0;
    bool idleTimerActive_ = false;
    bool terminating_ = false;
    bool creating_ = false;
};

class SubSpawnManager {
public:
    static SubSpawnManager &GetInstance();

    bool ShouldUseSubSpawn(const std::vector<std::string> &preloadSoList, bool isolationMode);

    pid_t StartChildViaSubSpawn(pid_t appPid, int32_t uid, const AppSpawnStartMsg &startMsg);

    void AsyncCreateSubSpawn(pid_t appPid, int32_t uid, const std::string &bundleName,
                             const AppSpawnStartMsg &startMsg);

    void OnSubSpawnExited(pid_t subSpawnPid);

    void OnChildProcessCreated(pid_t appPid);

    void OnChildProcessExited(pid_t appPid);

    void OnAppProcessDied(pid_t appPid);

    bool IsSubSpawn(pid_t pid);

private:
    SubSpawnManager();
    ~SubSpawnManager();
    DISALLOW_COPY_AND_MOVE(SubSpawnManager);

    static constexpr int32_t IDLE_TIMEOUT_MS = 30000;

    void StartIdleTimer(const std::shared_ptr<SubSpawnRecord> &record);
    void CancelIdleTimer(const std::shared_ptr<SubSpawnRecord> &record);
    void HandleIdleTimeout(pid_t appPid);

    std::string QuerySocketNameByUid(int32_t uid);
    void SendSubSpawnShutdown(const std::string &bundleName, pid_t subSpawnPid);

    pid_t DoCreateSubSpawn(pid_t appPid, int32_t uid, const std::string &bundleName,
                           const AppSpawnStartMsg &startMsg);

    std::map<pid_t /*appPid*/, std::shared_ptr<SubSpawnRecord>> subSpawnMap_;
    std::map<pid_t /*subSpawnPid*/, std::shared_ptr<SubSpawnRecord>> pidToRecordMap_;
    std::mutex mutex_;
};

} // namespace AppExecFwk
} // namespace OHOS

#endif // OHOS_SUB_SPAWN_MANAGER_H
