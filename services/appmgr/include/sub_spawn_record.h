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

#ifndef OHOS_SUB_SPAWN_RECORD_H
#define OHOS_SUB_SPAWN_RECORD_H

#include <atomic>
#include <chrono>
#include <memory>
#include <set>
#include <string>

#include "app_spawn_client.h"
#include "nocopyable.h"

namespace OHOS {
namespace AppExecFwk {

enum class SubSpawnState : int32_t {
    IDLE,
    CREATING,
    READY,
    TERMINATING,
};

class SubSpawnRecord {
public:
    static std::shared_ptr<SubSpawnRecord> CreateSubSpawnRecord(pid_t appPid, int32_t uid,
        const std::string &bundleName, const AppSpawnStartMsg &startMsg);

    pid_t GetSubSpawnPid() const;
    void SetSubSpawnPid(pid_t pid);
    pid_t GetAppPid() const;
    int32_t GetUid() const;
    std::string GetBundleName() const;

    SubSpawnState GetState() const;
    void SetState(SubSpawnState state);
    bool IsReady() const;
    bool IsCreating() const;
    bool IsTerminating() const;

    std::string GetSocketName() const;
    void SetSocketName(const std::string &name);

    void AddChildPid(pid_t pid);
    void RemoveChildPid(pid_t pid);
    size_t GetChildProcessCount() const;
    bool HasChildProcesses() const;

    bool IsIdleTimerActive() const;
    void SetIdleTimerActive(bool active);

    std::chrono::system_clock::time_point GetStartTime() const;
    const AppSpawnStartMsg &GetStartMsg() const;

    void SetKillReason(const std::string &reason);
    std::string GetKillReason() const;
    void SetExitSignal(int32_t signal);
    int32_t GetExitSignal() const;

private:
    SubSpawnRecord(pid_t appPid, int32_t uid, const std::string &bundleName,
        const AppSpawnStartMsg &startMsg);
    DISALLOW_COPY_AND_MOVE(SubSpawnRecord);

    pid_t subSpawnPid_ = 0;
    pid_t appPid_ = 0;
    int32_t uid_ = 0;
    std::string bundleName_;
    std::string socketName_;
    std::set<pid_t> childPids_;
    bool idleTimerActive_ = false;
    std::atomic<SubSpawnState> state_ {SubSpawnState::IDLE};
    std::chrono::system_clock::time_point startTime_;
    AppSpawnStartMsg startMsg_;
    std::string killReason_;
    int32_t exitSignal_ = -1;
};

} // namespace AppExecFwk
} // namespace OHOS
#endif // OHOS_SUB_SPAWN_RECORD_H
