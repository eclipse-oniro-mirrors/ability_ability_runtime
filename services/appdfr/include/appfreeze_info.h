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
#ifndef OHOS_ABILITY_RUNTIME_APPFREEZE_INFO_H
#define OHOS_ABILITY_RUNTIME_APPFREEZE_INFO_H

#include <atomic>
#include <cstdint>
#include <deque>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace OHOS {
namespace AAFwk {
class TaskHandlerWrap;
}
namespace AppExecFwk {
static constexpr int DEFAULT_APPFREEZE_REPORT_TIMES = 1;

struct FaultData;

// Producer identity distinguishes retransmissions from a new fault in the same process.
struct AppfreezeObservation {
    std::string eventName;
    int32_t instanceId = 0;
    uintptr_t token = 0;
    uint32_t state = 0;
    uint64_t detectTime = 0;
};

struct AppfreezeCaptureResult {
    std::string stack;
    std::string binderInfo;
    std::string memoryContent;
    int dumpResult = -1;
    int64_t dumpLatency = -1;
    int64_t processStart = 0;
};

enum class AppfreezeSubmitResult { DISPATCHED, QUEUED, DUPLICATE, REJECTED };

class AppfreezeManager;

struct AppFreezeInfo {
    int64_t occurTime = 0;
    int pid = 0;
    int reportTimes = DEFAULT_APPFREEZE_REPORT_TIMES;
    int state = 0;

    std::string eventName;
    std::string notificationId;
    std::string generationId;
    std::string episodeId;
    // Shared by Half and its fatal upgrade: bit 0 claims Half, bit 1 claims fatal.
    std::shared_ptr<std::atomic<uint8_t>> reportClaim;
    int64_t processStartTime = 0;
    int64_t freezeCount1min = -1; // -1 means the count was not observed.

    uint64_t requestId = 0;
    int32_t instanceId = 0;
    bool half = false;
    std::string flowId;
    std::shared_ptr<FaultData> faultData;
    bool IsCancelled() const { return cancelled_ && cancelled_->load(); }
    bool IsFallback() const { return fallback_; }
    static bool IsHalfEvent(const std::string& name);
    static std::string EventGroup(const std::string& name);

private:
    friend class AppfreezeManager;
    // One AppFreezeInfo per application instance owns both independent groups.
    // Event objects refer back weakly; all state below is guarded by AppfreezeManager's event mutex.
    std::map<std::string, std::shared_ptr<AppFreezeInfo>> groups_;
    std::map<std::string, int64_t> notifications_;
    std::map<std::string, std::deque<int64_t>> halfFlows_;
    int64_t mainCallbackTime_ = -1; // Recent 6s/input callback, retained after capture finishes.
    std::weak_ptr<AppFreezeInfo> application_;
    // The current event owns the uncollected events in its group. Finish hands
    // that ownership to the next AppFreezeInfo, without a second task wrapper.
    std::vector<std::shared_ptr<AppFreezeInfo>> pending_;
    std::function<void(const std::shared_ptr<AppFreezeInfo>&)> task_;
    std::weak_ptr<AAFwk::TaskHandlerWrap> taskHandler_;
    std::function<void()> cancelWatchdog_;
    std::function<void()> cancelFeedback_;
    int64_t countedAt_ = 0;
    bool counted_ = false;
    std::shared_ptr<std::atomic<bool>> cancelled_;
    std::string feedbackMarker_;
    bool feedbackClaimed_ = false;
    bool fatalAdmitted_ = false;
    bool accepting_ = true;
    bool dispatched_ = false;
    bool running_ = false;
    bool completed_ = false;
    bool fallback_ = false;
    bool dispositionStarted_ = false; // Original fatal handling starts once, when capture runs.
    bool watchdogArmed_ = false;
    int64_t deadline_ = 0;
};
using FreezeInfo = std::shared_ptr<AppFreezeInfo>;

} // namespace AppExecFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_APPFREEZE_INFO_H
