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

#ifndef OHOS_ABILITY_RUNTIME_BG_USER_EXTENSION_MONITOR_H
#define OHOS_ABILITY_RUNTIME_BG_USER_EXTENSION_MONITOR_H

#include <list>
#include <mutex>
#include <string>

#include "singleton.h"

namespace OHOS {
namespace AAFwk {

struct BgUserExtensionCallerInfo {
    int32_t callerUid = 0;
    int32_t callerUserId = 0;
    std::string callerProcessName;
    std::string callerBundleName;
};

struct BgUserExtensionEvent {
    std::string extensionTypeName;
    std::string packedInfo;
    std::string abilityName;
    int32_t cnt = 1;
};

class BgUserExtensionMonitor : public DelayedSingleton<BgUserExtensionMonitor> {
    DECLARE_DELAYED_SINGLETON(BgUserExtensionMonitor)
public:
    void StartMonitor();
    void StopMonitor();

    void OnBgUserExtensionStarted(const BgUserExtensionCallerInfo &callerInfo,
        const std::string &calleeBundleName, const std::string &calleeProcessName,
        const std::string &extensionTypeName, const std::string &abilityName);

    void ReportCachedEvents();

private:
    void AddOrUpdateEvent(const BgUserExtensionEvent &event);
    bool IsDuplicateEvent(const BgUserExtensionEvent &event,
        std::list<BgUserExtensionEvent>::iterator &dupIter);
    void SubmitPeriodicTask();

    std::mutex cacheMutex_;
    std::list<BgUserExtensionEvent> cachedEvents_;

    static constexpr int32_t HISEVENT_PARAM_COUNT = 7;
    static constexpr int32_t MAX_CACHED_EVENTS = 5;
    static constexpr int64_t REPORT_INTERVAL_MS = 2 * 60 * 60 * 1000; // 2 hours
    static constexpr const char *PERIODIC_TASK_NAME = "BgUserExtensionPeriodicTask";
};

} // namespace AAFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_BG_USER_EXTENSION_MONITOR_H
