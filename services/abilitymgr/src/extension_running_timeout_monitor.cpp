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

#include "extension_running_timeout_monitor.h"

#include <chrono>
#include <cinttypes>
#include <functional>

#include "extension_config.h"
#include "securec.h"
#include "hisysevent_report.h"
#include "hilog_tag_wrapper.h"
#include "task_handler_wrap.h"

namespace OHOS {
namespace AAFwk {
namespace {
constexpr const char *DOMAIN = "AAFWK";
constexpr const char *EVENT_NAME = "EXTENSION_RUNNING_TIMEOUT";
constexpr const char *EXTENSION_TYPE_KEY = "EXTENSION_TYPE";
constexpr const char *BUNDLE_NAME_KEY = "BUNDLE_NAME";
constexpr const char *ABILITY_NAME_KEY = "ABILITY_NAME";
constexpr const char *RUNNING_DURATION_KEY = "RUNNING_DURATION";
constexpr const char *STILL_ALIVE_KEY = "STILL_ALIVE";
constexpr const char *CNT_KEY = "CNT";

bool CopyStringParam(const std::string &str, std::vector<std::unique_ptr<char[]>> &buffers,
    std::vector<char*> &ptrs)
{
    size_t len = str.size() + 1;
    auto buf = std::make_unique<char[]>(len);
    if (strcpy_s(buf.get(), len, str.c_str()) != EOK) {
        return false;
    }
    ptrs.push_back(buf.get());
    buffers.push_back(std::move(buf));
    return true;
}
}

ExtensionRunningTimeoutMonitor::ExtensionRunningTimeoutMonitor() {}

ExtensionRunningTimeoutMonitor::~ExtensionRunningTimeoutMonitor() {}

void ExtensionRunningTimeoutMonitor::OnExtensionStarted(int32_t extensionRecordId,
    const std::string &extensionTypeName,
    const std::string &bundleName, const std::string &abilityName)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR,
        "recordId: %{public}d, type: %{public}s, bundle: %{public}s, ability: %{public}s",
        extensionRecordId, extensionTypeName.c_str(), bundleName.c_str(), abilityName.c_str());

    auto now = std::chrono::steady_clock::now();
    auto startTimeMillis =
        std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();

    ExtensionStartInfo info;
    info.extensionTypeName = extensionTypeName;
    info.bundleName = bundleName;
    info.abilityName = abilityName;
    info.startTimeMillis = startTimeMillis;

    std::lock_guard<std::mutex> lock(monitorMutex_);
    runningExtensions_[extensionRecordId] = info;
}

void ExtensionRunningTimeoutMonitor::OnExtensionTerminated(int32_t extensionRecordId)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "recordId: %{public}d", extensionRecordId);

    ExtensionStartInfo startInfo;
    {
        std::lock_guard<std::mutex> lock(monitorMutex_);
        auto it = runningExtensions_.find(extensionRecordId);
        if (it == runningExtensions_.end()) {
            TAG_LOGD(AAFwkTag::ABILITYMGR, "recordId %{public}d not found in running list",
                extensionRecordId);
            return;
        }
        startInfo = it->second;
        runningExtensions_.erase(it);
    }

    // Get configured timeout for this extension type
    int32_t configuredTimeout = DelayedSingleton<ExtensionConfig>::GetInstance()->
        GetExtensionRunningTimeoutTime(startInfo.extensionTypeName);
    if (configuredTimeout <= 0) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "no timeout configured for type %{public}s, skip",
            startInfo.extensionTypeName.c_str());
        return;
    }

    auto now = std::chrono::steady_clock::now();
    auto nowMillis = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    int64_t runningDurationMs = nowMillis - startInfo.startTimeMillis;
    int64_t runningDurationSec = runningDurationMs / 1000;

    // runningDuration <= configuredTimeout means within limit, do not track
    if (runningDurationSec <= configuredTimeout) {
        TAG_LOGD(AAFwkTag::ABILITYMGR,
            "extension %{public}s/%{public}s within timeout, duration: %{public}" PRId64 "s, limit: %{public}ds",
            startInfo.bundleName.c_str(), startInfo.abilityName.c_str(),
            runningDurationSec, configuredTimeout);
        return;
    }

    TAG_LOGI(AAFwkTag::ABILITYMGR,
        "extension %{public}s/%{public}s exceeded timeout, duration: %{public}" PRId64 "s, limit: %{public}ds",
        startInfo.bundleName.c_str(), startInfo.abilityName.c_str(),
        runningDurationSec, configuredTimeout);

    ExtensionTimeoutEvent event;
    event.extensionTypeName = startInfo.extensionTypeName;
    event.bundleName = startInfo.bundleName;
    event.abilityName = startInfo.abilityName;
    event.runningDuration = runningDurationSec;
    event.stillAlive = false;
    event.cnt = 1;

    std::lock_guard<std::mutex> lock(monitorMutex_);
    AddOrUpdateTimeoutEvent(event);
}

void ExtensionRunningTimeoutMonitor::StartMonitor()
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "start extension running timeout monitor");
    SubmitPeriodicTask();
}

void ExtensionRunningTimeoutMonitor::StopMonitor()
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "stop extension running timeout monitor");
    auto taskHandler = TaskHandlerWrap::GetFfrtHandler();
    if (taskHandler != nullptr) {
        taskHandler->CancelTask(PERIODIC_TASK_NAME);
    }
}

void ExtensionRunningTimeoutMonitor::SubmitPeriodicTask()
{
    auto taskHandler = TaskHandlerWrap::GetFfrtHandler();
    if (taskHandler == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "failed to get ffrt handler");
        return;
    }

    auto task = []() {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "extension timeout periodic task triggered");
        auto monitor = DelayedSingleton<ExtensionRunningTimeoutMonitor>::GetInstance();
        monitor->CheckAliveExtensions();
        monitor->ReportTimeoutEvents();
        monitor->SubmitPeriodicTask();
    };
    taskHandler->SubmitTask(task, PERIODIC_TASK_NAME, REPORT_INTERVAL_MS);
}

void ExtensionRunningTimeoutMonitor::ReportTimeoutEvents()
{
    std::list<ExtensionTimeoutEvent> eventsToReport;
    {
        std::lock_guard<std::mutex> lock(monitorMutex_);
        if (cachedEvents_.empty()) {
            TAG_LOGD(AAFwkTag::ABILITYMGR, "no timeout events to report");
            return;
        }
        eventsToReport = cachedEvents_;
        cachedEvents_.clear();
    }

    std::vector<std::unique_ptr<char[]>> stringBuffers;
    std::vector<char*> extensionTypePtrs;
    std::vector<char*> bundleNamePtrs;
    std::vector<char*> abilityNamePtrs;
    std::vector<int64_t> runningDurations;
    std::vector<bool> stillAliveFlags;
    std::vector<int32_t> cnts;

    for (const auto &event : eventsToReport) {
        if (!CopyStringParam(event.extensionTypeName, stringBuffers, extensionTypePtrs) ||
            !CopyStringParam(event.bundleName, stringBuffers, bundleNamePtrs) ||
            !CopyStringParam(event.abilityName, stringBuffers, abilityNamePtrs)) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "strcpy_s failed for event string");
            continue;
        }
        runningDurations.push_back(event.runningDuration);
        stillAliveFlags.push_back(event.stillAlive);
        cnts.push_back(event.cnt);
    }

    if (extensionTypePtrs.empty()) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "no valid events to report after buffer preparation");
        return;
    }

    HisyseventReport report(HISEVENT_PARAM_COUNT);
    report.InsertParam(EXTENSION_TYPE_KEY, extensionTypePtrs);
    report.InsertParam(BUNDLE_NAME_KEY, bundleNamePtrs);
    report.InsertParam(ABILITY_NAME_KEY, abilityNamePtrs);
    report.InsertParam(RUNNING_DURATION_KEY, runningDurations);
    report.InsertParam(STILL_ALIVE_KEY, stillAliveFlags);
    report.InsertParam(CNT_KEY, cnts);
    int32_t ret = report.Report(DOMAIN, EVENT_NAME, HISYSEVENT_STATISTIC);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "reported %{public}zu timeout events, ret: %{public}d",
        eventsToReport.size(), ret);
}

void ExtensionRunningTimeoutMonitor::CheckAliveExtensions()
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "check alive extensions for timeout violations");

    // Copy the running extensions map to avoid holding mutex during slow checks
    std::unordered_map<int32_t, ExtensionStartInfo> aliveCopy;
    {
        std::lock_guard<std::mutex> lock(monitorMutex_);
        aliveCopy = runningExtensions_;
    }

    auto now = std::chrono::steady_clock::now();
    auto nowMillis = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();

    std::vector<ExtensionTimeoutEvent> pendingEvents;
    for (auto &[recordId, startInfo] : aliveCopy) {
        int32_t configuredTimeout = DelayedSingleton<ExtensionConfig>::GetInstance()->
            GetExtensionRunningTimeoutTime(startInfo.extensionTypeName);
        if (configuredTimeout <= 0) {
            continue;
        }

        int64_t runningDurationMs = nowMillis - startInfo.startTimeMillis;
        int64_t runningDurationSec = runningDurationMs / 1000;
        if (runningDurationSec <= configuredTimeout) {
            continue;
        }

        TAG_LOGI(AAFwkTag::ABILITYMGR,
            "alive extension %{public}s/%{public}s exceeded timeout, duration: %{public}" PRId64 "s",
            startInfo.bundleName.c_str(), startInfo.abilityName.c_str(), runningDurationSec);

        ExtensionTimeoutEvent event;
        event.extensionTypeName = startInfo.extensionTypeName;
        event.bundleName = startInfo.bundleName;
        event.abilityName = startInfo.abilityName;
        event.runningDuration = runningDurationSec;
        event.stillAlive = true;
        event.cnt = 1;
        pendingEvents.push_back(event);
    }

    if (!pendingEvents.empty()) {
        std::lock_guard<std::mutex> lock(monitorMutex_);
        for (auto &event : pendingEvents) {
            AddOrUpdateTimeoutEvent(event);
        }
    }
}

bool ExtensionRunningTimeoutMonitor::IsDuplicateEvent(const ExtensionTimeoutEvent &event,
    std::list<ExtensionTimeoutEvent>::iterator &dupIter)
{
    for (auto it = cachedEvents_.begin(); it != cachedEvents_.end(); ++it) {
        if (it->extensionTypeName == event.extensionTypeName &&
            it->bundleName == event.bundleName &&
            it->abilityName == event.abilityName) {
            dupIter = it;
            return true;
        }
    }
    return false;
}

void ExtensionRunningTimeoutMonitor::AddOrUpdateTimeoutEvent(const ExtensionTimeoutEvent &event)
{
    std::list<ExtensionTimeoutEvent>::iterator dupIter;
    if (IsDuplicateEvent(event, dupIter)) {
        dupIter->cnt++;
        dupIter->runningDuration = event.runningDuration;
        dupIter->stillAlive = event.stillAlive;
        TAG_LOGD(AAFwkTag::ABILITYMGR,
            "updated existing timeout event for %{public}s/%{public}s, cnt: %{public}d",
            event.bundleName.c_str(), event.abilityName.c_str(), dupIter->cnt);
        return;
    }

    if (static_cast<int32_t>(cachedEvents_.size()) >= MAX_CACHED_EVENTS) {
        TAG_LOGW(AAFwkTag::ABILITYMGR,
            "cache full (%{public}d), discarding event for %{public}s/%{public}s",
            MAX_CACHED_EVENTS, event.bundleName.c_str(), event.abilityName.c_str());
        return;
    }

    cachedEvents_.push_back(event);
    TAG_LOGD(AAFwkTag::ABILITYMGR,
        "added new timeout event for %{public}s/%{public}s, cache size: %{public}zu",
        event.bundleName.c_str(), event.abilityName.c_str(), cachedEvents_.size());
}

} // namespace AAFwk
} // namespace OHOS
