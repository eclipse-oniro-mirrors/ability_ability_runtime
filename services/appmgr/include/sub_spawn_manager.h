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
#include "nocopyable.h"
#include "sub_spawn_record.h"

namespace OHOS {
namespace AppExecFwk {

class AppRunningRecord;

class SubSpawnManager {
public:
    static SubSpawnManager &GetInstance();

    bool ShouldUseSubSpawn(int32_t childProcessType, const std::vector<std::string> &preloadSoList,
        const std::string &nativeLibraryPath, bool isolationMode);

    pid_t StartChildViaSubSpawn(int32_t uid, const AppSpawnStartMsg &startMsg);

    void AsyncCreateSubSpawn(int32_t uid, const std::string &bundleName,
                             const AppSpawnStartMsg &startMsg);

    void OnSubSpawnExited(pid_t subSpawnPid);

    void OnChildProcessCreated(int32_t uid, pid_t childPid);

    void OnChildProcessExited(int32_t uid, pid_t childPid);

    void OnAppProcessDied(int32_t uid);

private:
    SubSpawnManager();
    ~SubSpawnManager();
    DISALLOW_COPY_AND_MOVE(SubSpawnManager);

    static constexpr int32_t IDLE_TIMEOUT_MS = 30000;

    void StartIdleTimer(const std::shared_ptr<SubSpawnRecord> &record);
    void CancelIdleTimer(const std::shared_ptr<SubSpawnRecord> &record);
    void HandleIdleTimeout(int32_t uid);

    std::string QuerySocketNameByUid(int32_t uid);

    pid_t DoCreateSubSpawn(int32_t uid, const std::string &bundleName,
                           const AppSpawnStartMsg &startMsg);

    std::map<int32_t /*uid*/, std::shared_ptr<SubSpawnRecord>> subSpawnMap_;
    std::map<pid_t /*subSpawnPid*/, std::shared_ptr<SubSpawnRecord>> pidToRecordMap_;
    std::mutex mutex_;
};

} // namespace AppExecFwk
} // namespace OHOS

#endif // OHOS_SUB_SPAWN_MANAGER_H
