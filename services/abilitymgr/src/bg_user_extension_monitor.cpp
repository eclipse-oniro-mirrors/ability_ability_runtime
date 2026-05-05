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

#include "bg_user_extension_monitor.h"

#include <functional>

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

constexpr const char *CALLER_UID_KEY = "callerUid";
constexpr const char *CALLER_USERID_KEY = "callerUserId";
constexpr const char *CALLER_PROCESS_NAME_KEY = "callerProcName";
constexpr const char *CALLER_BUNDLE_NAME_KEY = "callerBundleName";
constexpr const char *CALLEE_BUNDLE_NAME_KEY = "calleeBundleName";
constexpr const char *CALLEE_PROCESS_NAME_KEY = "calleeProcName";

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

std::string BuildPackedInfo(const BgUserExtensionCallerInfo &callerInfo,
    const std::string &calleeBundleName, const std::string &calleeProcessName)
{
    return std::string(CALLEE_BUNDLE_NAME_KEY) + "=" + calleeBundleName +
        ";" + CALLEE_PROCESS_NAME_KEY + "=" + calleeProcessName +
        ";" + CALLER_UID_KEY + "=" + std::to_string(callerInfo.callerUid) +
        ";" + CALLER_USERID_KEY + "=" + std::to_string(callerInfo.callerUserId) +
        ";" + CALLER_PROCESS_NAME_KEY + "=" + callerInfo.callerProcessName +
        ";" + CALLER_BUNDLE_NAME_KEY + "=" + callerInfo.callerBundleName;
}
}

BgUserExtensionMonitor::BgUserExtensionMonitor() {}

BgUserExtensionMonitor::~BgUserExtensionMonitor() {}

void BgUserExtensionMonitor::StartMonitor()
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "start bg user extension monitor");
    SubmitPeriodicTask();
}

void BgUserExtensionMonitor::StopMonitor()
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "stop bg user extension monitor");
    auto taskHandler = TaskHandlerWrap::GetFfrtHandler();
    if (taskHandler != nullptr) {
        taskHandler->CancelTask(PERIODIC_TASK_NAME);
    }
}

void BgUserExtensionMonitor::OnBgUserExtensionStarted(const BgUserExtensionCallerInfo &callerInfo,
    const std::string &calleeBundleName, const std::string &calleeProcessName,
    const std::string &extensionTypeName, const std::string &abilityName)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR,
        "bg user extension started, type:%{public}s, callerUid:%{public}d, callerBundle:%{public}s, "
        "calleeBundle:%{public}s, ability:%{public}s",
        extensionTypeName.c_str(), callerInfo.callerUid, callerInfo.callerBundleName.c_str(),
        calleeBundleName.c_str(), abilityName.c_str());

    BgUserExtensionEvent event;
    event.extensionTypeName = extensionTypeName;
    event.packedInfo = BuildPackedInfo(callerInfo, calleeBundleName, calleeProcessName);
    event.abilityName = abilityName;
    event.cnt = 1;

    std::lock_guard<std::mutex> lock(cacheMutex_);
    AddOrUpdateEvent(event);
}

void BgUserExtensionMonitor::SubmitPeriodicTask()
{
    auto taskHandler = TaskHandlerWrap::GetFfrtHandler();
    if (taskHandler == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "failed to get ffrt handler");
        return;
    }

    auto task = []() {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "bg user extension periodic task triggered");
        auto monitor = DelayedSingleton<BgUserExtensionMonitor>::GetInstance();
        monitor->ReportCachedEvents();
        monitor->SubmitPeriodicTask();
    };
    taskHandler->SubmitTask(task, PERIODIC_TASK_NAME, REPORT_INTERVAL_MS);
}

void BgUserExtensionMonitor::ReportCachedEvents()
{
    std::list<BgUserExtensionEvent> eventsToReport;
    {
        std::lock_guard<std::mutex> lock(cacheMutex_);
        if (cachedEvents_.empty()) {
            TAG_LOGD(AAFwkTag::ABILITYMGR, "no bg user extension events to report");
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
            !CopyStringParam(event.packedInfo, stringBuffers, bundleNamePtrs) ||
            !CopyStringParam(event.abilityName, stringBuffers, abilityNamePtrs)) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "strcpy_s failed for event string");
            continue;
        }
        runningDurations.push_back(0);
        stillAliveFlags.push_back(false);
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
    TAG_LOGI(AAFwkTag::ABILITYMGR, "reported %{public}zu bg user extension events, ret: %{public}d",
        eventsToReport.size(), ret);
}

bool BgUserExtensionMonitor::IsDuplicateEvent(const BgUserExtensionEvent &event,
    std::list<BgUserExtensionEvent>::iterator &dupIter)
{
    for (auto it = cachedEvents_.begin(); it != cachedEvents_.end(); ++it) {
        if (it->extensionTypeName == event.extensionTypeName &&
            it->abilityName == event.abilityName) {
            dupIter = it;
            return true;
        }
    }
    return false;
}

void BgUserExtensionMonitor::AddOrUpdateEvent(const BgUserExtensionEvent &event)
{
    std::list<BgUserExtensionEvent>::iterator dupIter;
    if (IsDuplicateEvent(event, dupIter)) {
        dupIter->cnt++;
        dupIter->packedInfo = event.packedInfo;
        TAG_LOGD(AAFwkTag::ABILITYMGR,
            "updated existing bg user extension event for %{public}s, cnt: %{public}d",
            event.abilityName.c_str(), dupIter->cnt);
        return;
    }

    if (static_cast<int32_t>(cachedEvents_.size()) >= MAX_CACHED_EVENTS) {
        TAG_LOGW(AAFwkTag::ABILITYMGR,
            "cache full (%{public}d), discarding event for %{public}s",
            MAX_CACHED_EVENTS, event.abilityName.c_str());
        return;
    }

    cachedEvents_.push_back(event);
    TAG_LOGD(AAFwkTag::ABILITYMGR,
        "added new bg user extension event for %{public}s, cache size: %{public}zu",
        event.abilityName.c_str(), cachedEvents_.size());
}

} // namespace AAFwk
} // namespace OHOS
