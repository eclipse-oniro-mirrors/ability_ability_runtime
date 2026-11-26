/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_CACHE_PROCESS_MANAGER_H
#define OHOS_ABILITY_RUNTIME_CACHE_PROCESS_MANAGER_H

#include <memory>
#include <deque>
#include <mutex>
#include <set>
#include <unordered_set>
#include "singleton.h"
#include "app_running_record.h"
#include "cpp/mutex.h"

namespace OHOS {
namespace AppExecFwk {

class CacheProcessManager {
    DECLARE_DELAYED_SINGLETON(CacheProcessManager);
public:
    bool QueryEnableProcessCache();
    void SetAppMgr(const std::weak_ptr<AppMgrServiceInner> &appMgr);
    bool PenddingCacheProcess(const std::shared_ptr<AppRunningRecord> &appRecord);
    bool CheckAndCacheProcess(const std::shared_ptr<AppRunningRecord> &appRecord);
    bool IsCachedProcess(const std::shared_ptr<AppRunningRecord> &appRecord);
    void OnProcessKilled(const std::shared_ptr<AppRunningRecord> &appRecord);
    bool ReuseCachedProcess(const std::shared_ptr<AppRunningRecord> &appRecord);
    bool IsAppSupportProcessCache(const std::shared_ptr<AppRunningRecord> &appRecord);
    bool IsAppShouldCache(const std::shared_ptr<AppRunningRecord> &appRecord);
    void RefreshCacheNum();
    std::string PrintCacheQueue();
    void PrepareActivateCache(const std::shared_ptr<AppRunningRecord> &appRecord);
    void OnAppProcessCacheBlocked(const std::shared_ptr<AppRunningRecord> &appRecord);
    void CheckAndSetProcessCacheEnable(const std::shared_ptr<AppRunningRecord> &appRecord);
private:
    bool IsAppAbilitiesEmpty(const std::shared_ptr<AppRunningRecord> &appRecord);
    int GetCurrentCachedProcNum();
    void RemoveCacheRecord(const std::shared_ptr<AppRunningRecord> &appRecord);
    void ShrinkAndKillCache();
    bool KillProcessByRecord(const std::shared_ptr<AppRunningRecord> &appRecord);
    void AddToApplicationSet(const std::shared_ptr<AppRunningRecord> &appRecord);
    void RemoveFromApplicationSet(const std::shared_ptr<AppRunningRecord> &appRecord);
    bool CheckAndNotifyCachedState(const std::shared_ptr<AppRunningRecord> &appRecord);
    bool IsAppContainsSrvExt(const std::shared_ptr<AppRunningRecord> &appRecord);
    bool IsAppSupportProcessCacheInnerFirst(const std::shared_ptr<AppRunningRecord> &appRecord);
    bool IsProcessSupportHotStart(const std::shared_ptr<AppRunningRecord> &appRecord);
    bool CheckSupportState(const std::shared_ptr<AppRunningRecord> &appRecord);
    // stores records that has been checked service extension
    std::unordered_set<std::shared_ptr<AppRunningRecord>> srvExtCheckedFlag;
    // bundleName->uid->record
    std::map<std::string, std::map<int32_t, std::set<std::shared_ptr<AppRunningRecord>>>> sameAppSet;
    // stores records that are servcie extension
    std::set<std::shared_ptr<AppRunningRecord>> srvExtRecords;
    std::deque<std::shared_ptr<AppRunningRecord>> cachedAppRecordQueue_;
    std::weak_ptr<AppMgrServiceInner> appMgr_;
    ffrt::recursive_mutex cacheQueueMtx;
    int32_t maxProcCacheNum_ = 0;
    int32_t allowedCacheNum_ = 0;
    bool warmStartProcesEnable_ = false;
    bool isOversea_ = false;
    bool shouldCheckApi = true;
    // whether the feature should check setSupportedProcessCache value or not
    bool shouldCheckSupport = true;
};
} // namespace OHOS
} // namespace AppExecFwk

#endif