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

#ifndef OHOS_ABILITY_RUNTIME_EXTENSION_RUNNING_TIMEOUT_MONITOR_H
#define OHOS_ABILITY_RUNTIME_EXTENSION_RUNNING_TIMEOUT_MONITOR_H

#include <list>
#include <mutex>
#include <string>
#include <unordered_map>

#include "singleton.h"

namespace OHOS {
namespace AAFwk {

struct ExtensionTimeoutEvent {
    std::string extensionTypeName;
    std::string bundleName;
    std::string abilityName;
    int64_t runningDuration = 0; // seconds
    bool stillAlive = false;
    int32_t cnt = 1;
};

class ExtensionRunningTimeoutMonitor : public DelayedSingleton<ExtensionRunningTimeoutMonitor> {
    DECLARE_DELAYED_SINGLETON(ExtensionRunningTimeoutMonitor)
public:
    /**
     * @brief Called when an extension ability starts. Records start time.
     */
    void OnExtensionStarted(int32_t extensionRecordId, const std::string &extensionTypeName,
        const std::string &bundleName, const std::string &abilityName);

    /**
     * @brief Called when an extension ability ends. Checks if it exceeded configured timeout.
     */
    void OnExtensionTerminated(int32_t extensionRecordId);

    /**
     * @brief Start the periodic report and check timer. Called during service init.
     */
    void StartMonitor();

    /**
     * @brief Stop the periodic report and check timer.
     */
    void StopMonitor();

    /**
     * @brief Report cached timeout events via HiSysEvent and clear cache.
     */
    void ReportTimeoutEvents();

    /**
     * @brief Proactively check all currently alive extensions for timeout violations.
     *        Adds detected violations to the cached events list.
     */
    void CheckAliveExtensions();

private:
    struct ExtensionStartInfo {
        std::string extensionTypeName;
        std::string bundleName;
        std::string abilityName;
        int64_t startTimeMillis = 0;
    };

    void AddOrUpdateTimeoutEvent(const ExtensionTimeoutEvent &event);
    bool IsDuplicateEvent(const ExtensionTimeoutEvent &event,
        std::list<ExtensionTimeoutEvent>::iterator &dupIter);
    void SubmitPeriodicTask();

    std::mutex monitorMutex_;
    std::unordered_map<int32_t, ExtensionStartInfo> runningExtensions_;
    std::list<ExtensionTimeoutEvent> cachedEvents_;

    static constexpr int32_t HISEVENT_PARAM_COUNT = 7;
    static constexpr int32_t MAX_CACHED_EVENTS = 5;
    static constexpr int64_t REPORT_INTERVAL_MS = 2 * 60 * 60 * 1000; // 2 hours
    static constexpr const char *PERIODIC_TASK_NAME = "ExtensionTimeoutPeriodicTask";
};

} // namespace AAFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_EXTENSION_RUNNING_TIMEOUT_MONITOR_H
