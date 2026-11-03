/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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
#include "appfreeze_manager.h"

#include <algorithm>
#include <chrono>
#include <iterator>
#include <utility>
#include <fcntl.h>
#include <cstring>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/stat.h>
#include <fstream>
#include <sstream>
#include <sys/ioctl.h>

#include "faultloggerd_client.h"
#include "file_ex.h"
#include "ffrt.h"
#include "cpp/future.h"
#include "dfx_dump_catcher.h"
#include "directory_ex.h"
#include "hisysevent_report.h"
#include "hitrace_meter.h"
#include "parameter.h"
#include "parameters.h"
#include "singleton.h"
#include "res_sched_util.h"
#include "app_mgr_client.h"
#include "hilog_tag_wrapper.h"
#include "time_util.h"
#include "task_handler_wrap.h"
#ifdef APP_MGR_SERVICE_HICOLLIE_ENABLE
#include "xcollie/xcollie.h"
#endif
#ifdef ABILITY_RUNTIME_HITRACE_ENABLE
#include "hitrace/hitracechain.h"
#endif
#include "appfreeze_cpu_freq_manager.h"
#include "appfreeze_util.h"
#include "xcollie/process_kill_reason.h"

#undef FREEZE_DOMAIN
#define FREEZE_DOMAIN 0xD001310
namespace OHOS {
namespace AppExecFwk {
namespace {
constexpr int MAX_LAYER = 8;
constexpr int FREEZEMAP_SIZE_MAX = 20;
constexpr int FREEZE_TIME_LIMIT = 60000;
constexpr int FREEZE_EVENT_MAX_SIZE = 200;
constexpr int64_t FREEZE_KILL_LIMIT = 60000;
static constexpr uint8_t ARR_SIZE = 7;
static constexpr uint8_t DECIMAL = 10;
static constexpr uint8_t FREE_ASYNC_INDEX = 6;
static constexpr uint16_t FREE_ASYNC_MAX = 1000;
static constexpr int DUMP_STACK_FAILED = -1;
static constexpr int DUMP_KERNEL_STACK_SUCCESS = 1;
static constexpr int MIN_APP_UID = 20000;
static constexpr int MAX_REPORT_STACK_SIZE = 256 * 1024; // 256KB
static constexpr int UNKNOWN_FREEZE_REASON = -1;
static constexpr int64_t LATENCY_INVALID = -1;
const std::string LOG_FILE_PATH = "/data/log/eventlog";
static bool g_betaVersion = OHOS::system::GetParameter("const.logsystem.versiontype", "unknown") == "beta";
static bool g_overseaVersion = OHOS::system::GetParameter("const.global.region", "CN") != "CN";
static bool g_developMode = (OHOS::system::GetParameter("persist.hiview.leak_detector", "unknown") == "enable") ||
                            (OHOS::system::GetParameter("persist.hiview.leak_detector", "unknown") == "true");

static constexpr const char *const HITRACE_ID = "hitrace_id: ";
static constexpr const char *const SPAN_ID = "span_id: ";
static constexpr const char *const PARENT_SPAN_ID = "parent_span_id: ";
static constexpr const char *const TRACE_FLAG = "trace_flag: ";
static constexpr const char *const DEV_SYSLOAD = "/dev/sysload";
constexpr const char* PRELOAD_UIEXTENSION = "PreloadUIExtension";
// kill resaon
constexpr int32_t INVALID_KILL_ID = -2;
constexpr const char* INVALID_KILL_REASON = "InvalidKillId";

#define KILL_LOG_BASE 'S'
#define GET_KILL_INFO _IOWR(KILL_LOG_BASE, 0x06, int32_t)
#define SET_KILL_INFO _IOWR(KILL_LOG_BASE, 0x07, int32_t)

#define SYSLOAD_GET_KILL_INFO_MAGIC 0xE5AC02
#define SYSLOAD_SET_KILL_INFO_MAGIC 0xE5AC03

static std::string ReadKernelProcessName(const char* name, size_t cap)
{
    return std::string(name, strnlen(name, cap));
}

struct KillEventInfo {
    int id = 0;
    int adj = 0;
    bool processed = false;
    bool foreground = false;
    pid_t pid = 0;
    int uid = 0;
    pid_t callingPid;
    char callingProcessName[16];
    int64_t timestamp = 0;
    int64_t eventParamFirst = 0;
    int64_t eventParamSecond = 0;
    int64_t eventParamThird = 0;
    int64_t eventParamFourth = 0;
    int64_t eventParamFifth = 0;
    int64_t eventParamSixth = 0;
    int64_t eventParamSeventh = 0;
};

struct KillInfo {
    unsigned int magic;
    pid_t pid;
    struct KillEventInfo data;
    unsigned int structSize;
};
}
static constexpr const char *const TWELVE_BIG_CPU_CUR_FREQ = "/sys/devices/system/cpu/cpufreq/policy2/scaling_cur_freq";
static constexpr const char *const TWELVE_BIG_CPU_MAX_FREQ = "/sys/devices/system/cpu/cpufreq/policy2/scaling_max_freq";
static constexpr const char *const TWELVE_MID_CPU_CUR_FREQ = "/sys/devices/system/cpu/cpufreq/policy1/scaling_cur_freq";
static constexpr const char *const TWELVE_MID_CPU_MAX_FREQ = "/sys/devices/system/cpu/cpufreq/policy1/scaling_max_freq";
const static std::set<std::string> HALF_EVENT_CONFIGS = {"UI_BLOCK_3S", "THREAD_BLOCK_3S", "BUSSNESS_THREAD_BLOCK_3S",
                                                         "LIFECYCLE_HALF_TIMEOUT", "LIFECYCLE_HALF_TIMEOUT_WARNING"};
static constexpr int PERF_TIME = 60000;
std::shared_ptr<AppfreezeManager> AppfreezeManager::instance_ = nullptr;
ffrt::mutex AppfreezeManager::singletonMutex_;
ffrt::mutex AppfreezeManager::freezeMutex_;
ffrt::mutex AppfreezeManager::freezeFilterMutex_;
ffrt::mutex AppfreezeManager::freezeInfoMutex_;
std::string AppfreezeManager::appfreezeInfoPath_;

AppfreezeManager::AppfreezeManager()
{
    name_ = "AppfreezeManager" + std::to_string(AbilityRuntime::TimeUtil::CurrentTimeMillis());
    collector_ = std::make_shared<AppfreezeBinderCollector>();
}

AppfreezeManager::~AppfreezeManager()
{
    CloseAppfreezeEvents();
}

std::shared_ptr<AppfreezeManager> AppfreezeManager::GetInstance()
{
    if (instance_ == nullptr) {
        std::lock_guard<ffrt::mutex> lock(singletonMutex_);
        if (instance_ == nullptr) {
            instance_ = std::make_shared<AppfreezeManager>();
        }
    }
    return instance_;
}

void AppfreezeManager::DestroyInstance()
{
    std::lock_guard<ffrt::mutex> lock(singletonMutex_);
    if (instance_ != nullptr) {
        instance_.reset();
        instance_ = nullptr;
    }
}

int AppfreezeManager::GetExceptionTimerId(const std::function<void()>& fallback)
{
#ifdef APP_MGR_SERVICE_HICOLLIE_ENABLE
    auto exceptionCallback = [fallback](void *) {
        // Bypass the stalled DFX queue through the existing global FFRT handler.
        AAFwk::TaskHandlerWrap::GetFfrtHandler()->SubmitTask(fallback, "AppfreezeException");
    };
    constexpr uint32_t timeout = 30;
    return HiviewDFX::XCollie::GetInstance().SetTimer("DfxFault::Exception", timeout,
        exceptionCallback, nullptr, HiviewDFX::XCOLLIE_FLAG_NOOP);
#else
    return -1;
#endif
}

namespace {
constexpr int64_t APPFREEZE_WINDOW_MS = 60000;
constexpr int64_t IPC_LATENCY_THREAD_BLOCK = 0;
constexpr int64_t IPC_LATENCY_NOT_MEASURED = -1;
constexpr int64_t APPFREEZE_CAPTURE_TIMEOUT_MS = 30000;
constexpr size_t APPFREEZE_MAX_PENDING = 2;
constexpr size_t APPFREEZE_MAX_ATTACHMENTS = 2;
constexpr auto APPFREEZE_ATTACHMENT_TIMEOUT = std::chrono::milliseconds(500);
std::atomic<uint64_t> appfreezeRequestSequence {0};
} // namespace

AppfreezeManager::FreezeInfo AppfreezeManager::ReserveAppfreezeTask(const FaultData& faultData,
    AppInfo& appInfo, const std::string& marker, AppfreezeSubmitResult& result)
{
    result = AppfreezeSubmitResult::REJECTED;
    if (!appInfo.refresh || !appInfo.refresh(appInfo, false)) {
        return nullptr;
    }
    AppfreezeObservation observation {faultData.errorObject.name, appInfo.instanceId,
        reinterpret_cast<uintptr_t>(faultData.token.GetRefPtr()), faultData.state, faultData.detectTime};
    auto info = ReserveAppfreezeEvent(appInfo, observation,
        AbilityRuntime::TimeUtil::CurrentTimeMillis(), marker, &result);
    if (!info) {
        return nullptr;
    }
    // Exit may have completed its cleanup before the reservation became visible.
    if (!appInfo.refresh(appInfo, false)) {
        RemoveAppfreezeInstance(info->generationId, info->instanceId);
        result = AppfreezeSubmitResult::REJECTED;
        return nullptr;
    }
    info->faultData = std::make_shared<FaultData>(faultData);
    return info;
}

AppfreezeSubmitResult AppfreezeManager::SubmitAppfreezeCapture(AppInfo appInfo, const FreezeInfo& request,
    const std::shared_ptr<AAFwk::TaskHandlerWrap>& handler, bool saTimeout)
{
    appInfo.isFrozen = FreezeUtil::GetInstance().IsFrozen(appInfo.pid);
    return SubmitAppfreezeEvent(request, appInfo, [weak = weak_from_this(), appInfo, saTimeout](const auto& req) {
        auto manager = weak.lock();
        auto current = appInfo;
        if (!manager || req->IsCancelled() || !current.refresh(current, true)) {
            return;
        }
        const auto& faultData = *req->faultData;
        if (!manager->ProcessAppfreezeEvent(req, appInfo, [&] {
            appInfo.handleFault(faultData, appInfo, saTimeout);
        })) {
            return;
        }
        if (req->IsFallback() && !req->half && !req->IsCancelled() && current.refresh(current, false)) {
            current.isOccurException = true;
            appInfo.handleFault(faultData, current, saTimeout);
        }
        if (req->IsCancelled()) {
            manager->ClearLogValidity(appInfo.pid);
        }
    }, handler);
}

int32_t AppfreezeManager::SubmitAppfreezeFault(const FaultData& faultData, AppInfo appInfo,
    const std::shared_ptr<AAFwk::TaskHandlerWrap>& handler, const FreezeInfo& reserved, bool saTimeout)
{
    if (faultData.faultType != FaultDataType::APP_FREEZE) {
        return ERR_OK;
    }
    if (!handler || !appInfo.refresh || !appInfo.handleFault || !appInfo.refresh(appInfo, false)) {
        CancelAppfreezeEvent(reserved);
        return ERR_INVALID_VALUE;
    }
    if (CheckPreloadUIExtension(faultData.errorObject.message, appInfo.bundleName, appInfo.pid,
        faultData.errorObject.name)) {
        CancelAppfreezeEvent(reserved);
        return ERR_OK;
    }
    AppfreezeSubmitResult admission;
    auto request = reserved ? reserved : ReserveAppfreezeTask(faultData, appInfo, "", admission);
    if (!request) {
        return admission == AppfreezeSubmitResult::DUPLICATE ? ERR_OK : ERR_INVALID_VALUE;
    }
    request->faultData = std::make_shared<FaultData>(faultData);
    auto result = SubmitAppfreezeCapture(appInfo, request, handler, saTimeout);
    TAG_LOGI(AAFwkTag::APPDFR, "Appfreeze submit request:%{public}" PRIu64 " pid:%{public}d result:%{public}d",
        request->requestId, appInfo.pid, static_cast<int>(result));
    return result == AppfreezeSubmitResult::REJECTED ? ERR_INVALID_VALUE : ERR_OK;
}

bool AppfreezeManager::PrepareAppfreezeFeedback(const FaultData& faultData, int32_t pid, int32_t instanceId,
    const std::shared_ptr<AAFwk::TaskHandlerWrap>& handler, FreezeInfo& reserved)
{
    if (faultData.timeoutMarkers.empty()) {
        return true;
    }
    if (!handler) {
        return false;
    }
    reserved = ClaimAppfreezeFeedback(pid, instanceId, faultData.timeoutMarkers, faultData.errorObject.name);
    if (!reserved) {
        return false;
    }
    // The AppFreezeInfo claim decides the winner when feedback races the timeout.
    handler->CancelTask(faultData.timeoutMarkers);
    return true;
}

int32_t AppfreezeManager::PrepareAppfreezeNotification(FaultData& faultData, AppInfo appInfo,
    const std::shared_ptr<AAFwk::TaskHandlerWrap>& handler, FreezeInfo& reserved)
{
    const int64_t timeout = 3000; // ipc timeout 3000ms
    if (!appInfo.refresh || !appInfo.handleFault || !appInfo.refresh(appInfo, false)) {
        return ERR_INVALID_VALUE;
    }
    if (!collector_->IsHandleAppfreeze(appInfo.bundleName) || !appInfo.refresh(appInfo, true)) {
        return ERR_OK;
    }
    if (!handler) {
        TAG_LOGE(AAFwkTag::APPMGR, "dfxTaskHandler_ null");
        return ERR_NO_INIT;
    }
    static std::atomic<uint64_t> notifySequence {0};
    const auto notifyTime = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
    // The application echoes this opaque marker; the pending task owns its lifetime.
    faultData.timeoutMarkers = "notifyFault:" + std::to_string(appInfo.pid) + ":" +
        std::to_string(appInfo.instanceId) + ":" + std::to_string(notifyTime) + ":" +
        std::to_string(notifySequence.fetch_add(1));
    faultData.ipcLatency = faultData.errorObject.name == AppFreezeType::THREAD_BLOCK_3S ||
        faultData.errorObject.name == AppFreezeType::THREAD_BLOCK_6S ?
        IPC_LATENCY_THREAD_BLOCK : IPC_LATENCY_NOT_MEASURED;
    AppfreezeSubmitResult admission;
    reserved = ReserveAppfreezeTask(faultData, appInfo, faultData.timeoutMarkers, admission);
    if (!reserved) {
        return admission == AppfreezeSubmitResult::DUPLICATE ? ERR_OK : ERR_NO_INIT;
    }
    auto timeoutNotifyApp = [weak = weak_from_this(), appInfo, faultData,
        weakHandler = std::weak_ptr<AAFwk::TaskHandlerWrap>(handler)] {
        if (auto manager = weak.lock()) {
            manager->TimeoutNotifyApp(faultData, appInfo, weakHandler.lock());
        }
    };
    auto timeoutTask = handler->SubmitTask(timeoutNotifyApp, faultData.timeoutMarkers, timeout);
    if (!timeoutTask) {
        CancelAppfreezeEvent(reserved, true);
        TAG_LOGE(AAFwkTag::APPDFR, "SA Appfreeze feedback timeout post failed pid:%{public}d", appInfo.pid);
        return ERR_NO_INIT;
    }
    SetAppfreezeFeedbackTask(reserved, [timeoutTask] { timeoutTask.Cancel(); });
    return ERR_OK;
}

void AppfreezeManager::TimeoutNotifyApp(const FaultData& faultData, AppInfo appInfo,
    const std::shared_ptr<AAFwk::TaskHandlerWrap>& handler)
{
    if (!handler || !appInfo.refresh || !appInfo.refresh(appInfo, false)) {
        return;
    }
    FreezeInfo reserved;
    if (!faultData.timeoutMarkers.empty()) {
        reserved = ClaimAppfreezeFeedback(appInfo.pid, appInfo.instanceId,
            faultData.timeoutMarkers, faultData.errorObject.name);
        if (!reserved) {
            return;
        }
    }
    SubmitAppfreezeFault(faultData, appInfo, handler, reserved, true);
}

void AppfreezeManager::ClearAppfreezeTasks(int32_t pid, int32_t uid, int32_t instanceId)
{
    RemoveAppfreezeInstance(std::to_string(pid) + ":" + std::to_string(uid), instanceId);
    ClearLogValidity(pid);
    collector_->DeleteStack(pid);
}

void AppfreezeManager::ObserveAppfreezeLocked(const FreezeInfo& info, int64_t now)
{
    auto app = info ? info->application_.lock() : nullptr;
    const bool mainFlow = AppFreezeInfo::EventGroup(info->eventName) == "main";
    if (!app) {
        return;
    }
    int64_t firstSeen = now;
    auto half = app->halfFlows_.find(info->flowId);
    if (!info->half && half != app->halfFlows_.end()) {
        firstSeen = half->second.front();
        // Input feedback timeout and 6s can finish separately in one main-thread stall.
        if (!mainFlow) {
            half->second.pop_front();
            if (half->second.empty()) {
                app->halfFlows_.erase(half);
            }
        }
    } else {
        recentFreezes_.insert(now);
        info->counted_ = true;
    }
    info->countedAt_ = firstSeen;
    if (info->half) {
        if (mainFlow) {
            // A new Half identity starts the next stall after main-thread recovery.
            app->halfFlows_[info->flowId].clear();
        }
    }
    if (!info->notificationId.empty()) {
        app->notifications_[info->notificationId] = firstSeen;
    }
    info->freezeCount1min = static_cast<int64_t>(recentFreezes_.size());
}

void AppfreezeManager::InitAppfreezeObservation(const FreezeInfo& info, const AppfreezeObservation& observation)
{
    info->instanceId = observation.instanceId;
    info->eventName = observation.eventName;
    info->half = AppFreezeInfo::IsHalfEvent(info->eventName);
    std::string family = info->eventName;
    if (AppFreezeInfo::EventGroup(family) == "main") {
        family = "THREAD_BLOCK";
    } else if (family == "LIFECYCLE_HALF_TIMEOUT") {
        family = "LIFECYCLE_TIMEOUT";
    } else if (family == "LIFECYCLE_HALF_TIMEOUT_WARNING") {
        family = "LIFECYCLE_TIMEOUT_WARNING";
    } else if (family == "BUSSINESS_THREAD_BLOCK_3S" || family == "BUSSINESS_THREAD_BLOCK_6S") {
        family = "BUSSINESS_THREAD_BLOCK";
    }
    info->flowId = info->generationId + ":" + std::to_string(info->instanceId) + ":" + family + ":" +
        (AppFreezeInfo::EventGroup(info->eventName) == "main" ? "0:0" :
        std::to_string(observation.token) + ":" + std::to_string(observation.state));
    info->notificationId = observation.detectTime > 0 ?
        info->flowId + ":" + info->eventName + ":" + std::to_string(observation.detectTime) : "";
}

AppfreezeManager::FreezeInfo AppfreezeManager::GetAppfreezeApplicationLocked(const FreezeInfo& info)
{
    auto& app = appfreezeApps_[info->generationId];
    if (app && (app->instanceId != info->instanceId || !app->accepting_)) {
        if (!app->groups_.empty()) {
            return {};
        }
        app.reset();
    }
    if (!app) {
        app = std::make_shared<AppFreezeInfo>();
        app->pid = info->pid;
        app->instanceId = info->instanceId;
        app->generationId = info->generationId;
    }
    info->application_ = app;
    return app;
}

AppfreezeManager::FreezeInfo AppfreezeManager::FindAppfreezeHalfLocked(const FreezeInfo& info,
    const FreezeInfo& current) const
{
    if (info->half) {
        return {};
    }
    if (current->half && current->flowId == info->flowId) {
        return current;
    }
    auto match = std::find_if(current->pending_.begin(), current->pending_.end(),
        [&info](const FreezeInfo& pending) { return pending->half && pending->flowId == info->flowId; });
    return match == current->pending_.end() ? nullptr : *match;
}

bool AppfreezeManager::PrepareAppfreezeCallbackLocked(const FreezeInfo& info, int64_t now)
{
    auto app = info->application_.lock();
    if (!app) {
        return false;
    }
#ifndef APP_NO_RESPONSE_DIALOG
    // A new application-side 3s report starts the next main-thread fault cycle.
    // AppfreezeInner remains authoritative when an IPC races recovery or reporting.
    if (info->eventName == AppFreezeType::THREAD_BLOCK_3S && info->feedbackMarker_.empty()) {
        app->mainCallbackTime_ = -1;
    }
#endif
    return info->feedbackMarker_.empty() || info->half || AppFreezeInfo::EventGroup(info->eventName) != "main" ||
        app->mainCallbackTime_ < 0 || now - app->mainCallbackTime_ >= APPFREEZE_WINDOW_MS;
}

bool AppfreezeManager::PrepareAppfreezeAdmissionLocked(const FreezeInfo& info, int64_t now,
    FreezeTask& cancelDiscardedFeedback)
{
    auto app = info->application_.lock();
    if (!app) {
        return false;
    }
    if (!info->notificationId.empty() && app->notifications_.count(info->notificationId) > 0) {
        return false;
    }
    if (!PrepareAppfreezeCallbackLocked(info, now)) {
        return false;
    }
    auto entry = app->groups_.find(AppFreezeInfo::EventGroup(info->eventName));
    if (entry == app->groups_.end()) {
        return true;
    }
    const auto& current = entry->second;
    if (!info->half && current->fatalAdmitted_) {
        return false;
    }
    const auto same = [&info](const FreezeInfo& old) {
        return !old->IsCancelled() && old->eventName == info->eventName &&
            old->flowId == info->flowId && old->notificationId == info->notificationId;
    };
    if (same(current) || std::any_of(current->pending_.begin(), current->pending_.end(), same)) {
        return false;
    }
    auto half = FindAppfreezeHalfLocked(info, current);
    if (current->pending_.size() >= APPFREEZE_MAX_PENDING) {
        if (info->half) {
            // Count an independent fault even when capture capacity is full.
            ObserveAppfreezeLocked(info, now);
            return false;
        }
        // Keep the matching Half ahead of its upgrade, even if it has not run yet.
        auto discarded = std::find_if(current->pending_.rbegin(), current->pending_.rend(),
            [&half](const FreezeInfo& pending) { return pending != half; });
        if (discarded == current->pending_.rend()) {
            return false;
        }
        *(*discarded)->cancelled_ = true;
        cancelDiscardedFeedback = std::exchange((*discarded)->cancelFeedback_, {});
        current->pending_.erase(std::next(discarded).base());
    }
    if (half) {
        info->episodeId = half->episodeId;
        info->reportClaim = half->reportClaim;
    }
    return true;
}

AppfreezeSubmitResult AppfreezeManager::EnqueueAppfreezeEventLocked(const FreezeInfo& info, int64_t now)
{
    info->requestId = ++appfreezeRequestSequence;
    info->occurTime = now;
    info->deadline_ = now + APPFREEZE_CAPTURE_TIMEOUT_MS;
    info->cancelled_ = std::make_shared<std::atomic<bool>>(false);
    if (info->episodeId.empty()) {
        const auto wallTime = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        lastAppfreezeEpisode_ = std::max<int64_t>(wallTime, lastAppfreezeEpisode_ + 1);
        info->episodeId = std::to_string(lastAppfreezeEpisode_);
    }
    if (!info->reportClaim) {
        info->reportClaim = std::make_shared<std::atomic<uint8_t>>(0);
    }
    ObserveAppfreezeLocked(info, now);
    auto app = info->application_.lock();
    if (!app) {
        return AppfreezeSubmitResult::REJECTED;
    }
    if (!info->half && info->feedbackMarker_.empty() && AppFreezeInfo::EventGroup(info->eventName) == "main") {
        app->mainCallbackTime_ = now;
    }
    auto entry = app->groups_.find(AppFreezeInfo::EventGroup(info->eventName));
    if (entry == app->groups_.end()) {
        info->fatalAdmitted_ = !info->half;
        app->groups_.emplace(AppFreezeInfo::EventGroup(info->eventName), info);
        return AppfreezeSubmitResult::DISPATCHED;
    }
    auto& current = entry->second;
    if (!info->half) {
        current->fatalAdmitted_ = true;
        auto predecessor = std::find_if(current->pending_.begin(), current->pending_.end(),
            [&info](const FreezeInfo& pending) { return pending->reportClaim == info->reportClaim; });
        current->pending_.insert(predecessor == current->pending_.end() ?
            current->pending_.begin() : std::next(predecessor), info);
    } else {
        current->pending_.push_back(info);
    }
    return AppfreezeSubmitResult::QUEUED;
}

AppfreezeManager::FreezeInfo AppfreezeManager::ReserveAppfreezeEvent(const AppInfo& appInfo,
    const AppfreezeObservation& observation, int64_t now,
    const std::string& feedbackMarker, AppfreezeSubmitResult* result)
{
    if (result) {
        *result = AppfreezeSubmitResult::REJECTED;
    }
    auto info = std::make_shared<AppFreezeInfo>();
    info->pid = appInfo.pid;
    info->generationId = std::to_string(appInfo.pid) + ":" + std::to_string(appInfo.uid);
    info->feedbackMarker_ = feedbackMarker;
    InitAppfreezeObservation(info, observation);
    FreezeTask cancelDiscardedFeedback;
    {
        std::lock_guard<std::mutex> lock(appfreezeEventMutex_);
        ExpireAppfreezeHistoryLocked(now);
        if (!GetAppfreezeApplicationLocked(info)) {
            return {};
        }
        if (result) {
            *result = AppfreezeSubmitResult::DUPLICATE;
        }
        if (!PrepareAppfreezeAdmissionLocked(info, now, cancelDiscardedFeedback)) {
            return {};
        }
        auto admission = EnqueueAppfreezeEventLocked(info, now);
        if (result) {
            *result = admission;
        }
    }
    if (cancelDiscardedFeedback) {
        cancelDiscardedFeedback();
    }
    ScheduleAppfreezeMaintenance();
    return info;
}

bool AppfreezeManager::IsAppfreezeEventTrackedLocked(const FreezeInfo& info) const
{
    auto app = info ? info->application_.lock() : nullptr;
    if (!app) {
        return false;
    }
    auto it = app->groups_.find(AppFreezeInfo::EventGroup(info->eventName));
    return it != app->groups_.end() && (it->second == info ||
        std::find(it->second->pending_.begin(), it->second->pending_.end(), info) != it->second->pending_.end());
}

AppfreezeSubmitResult AppfreezeManager::SubmitAppfreezeEvent(const FreezeInfo& info, FreezeProcess process,
    const std::shared_ptr<AAFwk::TaskHandlerWrap>& handler)
{
    auto app = info ? info->application_.lock() : nullptr;
    if (!app || !process || !handler) {
        return AppfreezeSubmitResult::REJECTED;
    }
    bool current = false;
    {
        std::lock_guard<std::mutex> lock(appfreezeEventMutex_);
        if (info->IsCancelled() || !IsAppfreezeEventTrackedLocked(info)) {
            return AppfreezeSubmitResult::REJECTED;
        }
        if (info->task_ || info->completed_) {
            return AppfreezeSubmitResult::DUPLICATE;
        }
        if (!info->feedbackMarker_.empty() && !info->feedbackClaimed_) {
            return AppfreezeSubmitResult::REJECTED;
        }
        info->task_ = std::move(process);
        info->taskHandler_ = handler;
        current = app->groups_.at(AppFreezeInfo::EventGroup(info->eventName)) == info;
    }
    if (!current) {
        return AppfreezeSubmitResult::QUEUED;
    }
    return DispatchAppfreezeEvent(info) ? AppfreezeSubmitResult::DISPATCHED : AppfreezeSubmitResult::REJECTED;
}

AppfreezeSubmitResult AppfreezeManager::SubmitAppfreezeEvent(const FreezeInfo& info, const AppInfo& appInfo,
    FreezeProcess process, const std::shared_ptr<AAFwk::TaskHandlerWrap>& handler)
{
    if (!info || !info->faultData) {
        return AppfreezeSubmitResult::REJECTED;
    }
    const auto& faultData = *info->faultData;
    // Apply the existing filter when feedback or timeout submits the admitted event.
    // Reserving an application notification must not consume the filter on a failed timer post.
    const std::string key = std::to_string(appInfo.pid) + "_" + std::to_string(appInfo.uid) + "_" +
        appInfo.bundleName + ":" + std::to_string(info->instanceId) + ":" + AppFreezeInfo::EventGroup(info->eventName);
    if (CheckAppfreezeHappend(key, faultData.errorObject.name)) {
        CancelAppfreezeEvent(info);
        return AppfreezeSubmitResult::DUPLICATE;
    }
    InitWarningCpuInfo(faultData, appInfo);
    return SubmitAppfreezeEvent(info, std::move(process), handler);
}

bool AppfreezeManager::ProcessAppfreezeEvent(const FreezeInfo& info, const AppInfo& appInfo,
    const FreezeTask& disposition)
{
    if (!info || !info->faultData ||
        !CanPublishAppfreeze(info, AbilityRuntime::TimeUtil::CurrentTimeMillis())) {
        return false;
    }
    bool startDisposition = false;
    {
        std::lock_guard<std::mutex> lock(appfreezeEventMutex_);
        auto app = info->application_.lock();
        if (!app || !app->accepting_ || info->IsCancelled() || !info->running_ || info->completed_) {
            return false;
        }
        if (!info->half && !info->fallback_ && !info->dispositionStarted_ && disposition) {
            info->dispositionStarted_ = true;
            startDisposition = true;
        }
    }
    if (startDisposition) {
        disposition();
    }
    const auto& faultData = *info->faultData;
    TAG_LOGI(AAFwkTag::APPDFR, "Appfreeze RUNNING request:%{public}" PRIu64 " pid:%{public}d",
        info->requestId, appInfo.pid);
    AppfreezeCaptureResult capture;
    if (!CaptureAppfreeze(faultData, appInfo, info, capture) ||
        !CanPublishAppfreeze(info, AbilityRuntime::TimeUtil::CurrentTimeMillis())) {
        return false;
    }
    auto reportInfo = appInfo;
    reportInfo.request = info;
    reportInfo.isOccurException = info->IsFallback();
    ReportAppfreeze(faultData, reportInfo, capture);
    // The report path ran; HiSysEvent retains ownership of transport retries.
    return true;
}

void AppfreezeManager::SetAppfreezeFeedbackTask(const FreezeInfo& info, FreezeTask cancel)
{
    auto app = info ? info->application_.lock() : nullptr;
    {
        std::lock_guard<std::mutex> lock(appfreezeEventMutex_);
        if (app && IsAppfreezeEventTrackedLocked(info) && !info->IsCancelled() && !info->feedbackClaimed_) {
            info->cancelFeedback_ = std::exchange(cancel, {});
            if (!info->half && AppFreezeInfo::EventGroup(info->eventName) == "main") {
                app->mainCallbackTime_ = info->occurTime;
            }
        }
    }
    if (cancel) {
        cancel();
    }
}

AppfreezeManager::FreezeInfo AppfreezeManager::ClaimAppfreezeFeedback(int32_t pid, int32_t instanceId,
    const std::string& marker, const std::string& eventName)
{
    FreezeInfo claimed;
    FreezeTask cancel;
    {
        std::lock_guard<std::mutex> lock(appfreezeEventMutex_);
        if (marker.empty()) { return {}; }
        const auto claim = [&](const FreezeInfo& info) {
            if (info->feedbackMarker_ != marker || info->eventName != eventName || info->feedbackClaimed_ ||
                info->IsCancelled() || AbilityRuntime::TimeUtil::CurrentTimeMillis() >= info->deadline_) {
                return false;
            }
            info->feedbackClaimed_ = true;
            claimed = info;
            cancel = std::exchange(info->cancelFeedback_, {});
            return true;
        };
        for (const auto& entry : appfreezeApps_) {
            const auto& app = entry.second;
            if (app->pid != pid || app->instanceId != instanceId || !app->accepting_) {
                continue;
            }
            for (const auto& group : app->groups_) {
                const auto& info = group.second;
                if (claim(info) || std::any_of(info->pending_.begin(), info->pending_.end(), claim)) {
                    break;
                }
            }
            if (claimed) {
                break;
            }
        }
    }
    if (cancel) {
        cancel();
    }
    return claimed;
}

bool AppfreezeManager::DispatchAppfreezeEvent(const FreezeInfo& info)
{
    {
        std::lock_guard<std::mutex> lock(appfreezeEventMutex_);
        auto app = info->application_.lock();
        if (!app || !IsAppfreezeEventTrackedLocked(info) ||
            app->groups_.at(AppFreezeInfo::EventGroup(info->eventName)) != info ||
            info->IsCancelled() || !info->task_) {
            return false;
        }
        if (info->dispatched_) {
            return true;
        }
        info->dispatched_ = true;
#ifdef APP_MGR_SERVICE_HICOLLIE_ENABLE
        info->watchdogArmed_ = true;
#endif
    }
    auto weak = weak_from_this();
#ifdef APP_MGR_SERVICE_HICOLLIE_ENABLE
    const int timerId = GetExceptionTimerId([weak, info] {
        if (auto inner = weak.lock()) {
            inner->RunAppfreezeEvent(info, true);
        }
    });
    FreezeTask cancel = timerId < 0 ? FreezeTask {} :
        FreezeTask([timerId] { HiviewDFX::XCollie::GetInstance().CancelTimer(timerId); });
    {
        std::lock_guard<std::mutex> lock(appfreezeEventMutex_);
        if (!info->running_ && !info->completed_ && !info->IsCancelled()) {
            info->cancelWatchdog_ = std::exchange(cancel, {});
            info->watchdogArmed_ = static_cast<bool>(info->cancelWatchdog_);
        }
    }
    if (cancel) {
        cancel();
    }
#endif
    auto handler = info->taskHandler_.lock();
    if (handler && handler->SubmitTask([weak, info] {
        if (auto inner = weak.lock()) {
            inner->RunAppfreezeEvent(info);
        }
    }, "AppfreezeCapture")) {
        return true;
    }
    CancelAppfreezeEvent(info);
    return false;
}

bool AppfreezeManager::CanPublishAppfreeze(const FreezeInfo& info, int64_t now)
{
    auto app = info ? info->application_.lock() : nullptr;
    std::lock_guard<std::mutex> lock(appfreezeEventMutex_);
    if (!app || !app->accepting_ || info->IsCancelled() || !IsAppfreezeEventTrackedLocked(info)) {
        return false;
    }
    if (now >= info->deadline_) {
        *info->cancelled_ = true;
    }
    return !info->IsCancelled();
}

void AppfreezeManager::RunAppfreezeEvent(const FreezeInfo& info, bool fallback)
{
    auto app = info ? info->application_.lock() : nullptr;
    FreezeTask cancel;
    FreezeProcess process;
    {
        std::lock_guard<std::mutex> lock(appfreezeEventMutex_);
        if (!app) {
            return;
        }
        auto it = app->groups_.find(AppFreezeInfo::EventGroup(info->eventName));
        if (it == app->groups_.end() || it->second != info || info->running_ || info->completed_ ||
            info->IsCancelled() || !info->accepting_) {
            return;
        }
        info->running_ = true;
        info->fallback_ = fallback;
        info->deadline_ = AbilityRuntime::TimeUtil::CurrentTimeMillis() + APPFREEZE_CAPTURE_TIMEOUT_MS;
        info->processStartTime = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        cancel = std::exchange(info->cancelWatchdog_, {});
        process = info->task_;
    }
    if (cancel) {
        cancel();
    }
    auto finish = std::shared_ptr<void>(nullptr, [weak = weak_from_this(), info](void*) {
        if (auto self = weak.lock()) {
            self->FinishAppfreezeEvent(info);
        }
    });
#ifdef __cpp_exceptions
    try {
#endif
        if (process && !info->IsCancelled()) {
            process(info);
        }
#ifdef __cpp_exceptions
    } catch (...) {
        TAG_LOGE(AAFwkTag::APPDFR, "Appfreeze task threw, pid:%{public}d", info->pid);
        *info->cancelled_ = true;
    }
#endif
}

void AppfreezeManager::FinishAppfreezeEvent(const FreezeInfo& info)
{
    auto app = info ? info->application_.lock() : nullptr;
    FreezeInfo next;
    {
        std::lock_guard<std::mutex> lock(appfreezeEventMutex_);
        if (!app) {
            return;
        }
        auto it = app->groups_.find(AppFreezeInfo::EventGroup(info->eventName));
        if (it == app->groups_.end() || it->second != info || info->completed_) {
            return;
        }
        info->completed_ = true;
        info->task_ = {};
        if (!info->accepting_ || info->pending_.empty()) {
            app->groups_.erase(it);
        } else {
            next = info->pending_.front();
            info->pending_.erase(info->pending_.begin());
            next->pending_ = std::exchange(info->pending_, {});
            next->fatalAdmitted_ = info->fatalAdmitted_;
            it->second = next;
            if (!next->task_) {
                next.reset(); // Its application feedback has not arrived yet.
            }
        }
    }
    if (next) {
        DispatchAppfreezeEvent(next);
    }
}

void AppfreezeManager::WithdrawAppfreezeEventLocked(const FreezeInfo& info, const FreezeInfo& app)
{
    app->notifications_.erase(info->notificationId);
    if (info->counted_) {
        auto timestamp = recentFreezes_.find(info->countedAt_);
        if (timestamp != recentFreezes_.end()) {
            recentFreezes_.erase(timestamp);
        }
    } else if (!info->half && AppFreezeInfo::EventGroup(info->eventName) != "main") {
        app->halfFlows_[info->flowId].push_front(info->countedAt_);
    }
    if (info->half) {
        auto flow = app->halfFlows_.find(info->flowId);
        if (flow != app->halfFlows_.end()) {
            auto timestamp = std::find(flow->second.begin(), flow->second.end(), info->countedAt_);
            if (timestamp != flow->second.end()) {
                flow->second.erase(timestamp);
            }
            if (flow->second.empty()) {
                app->halfFlows_.erase(flow);
            }
        }
    }
    if (!info->half) {
        app->groups_.at(AppFreezeInfo::EventGroup(info->eventName))->fatalAdmitted_ = false;
    }
}

void AppfreezeManager::CancelAppfreezeEvent(const FreezeInfo& info, bool withdraw)
{
    auto app = info ? info->application_.lock() : nullptr;
    if (!app) {
        return;
    }
    FreezeTask cancel;
    FreezeTask cancelFeedback;
    bool finish = false;
    {
        std::lock_guard<std::mutex> lock(appfreezeEventMutex_);
        if (!info->cancelled_) {
            return;
        }
        // Only an unsent application notification can roll back admission/counting.
        if (withdraw && !info->dispatched_ && !info->feedbackClaimed_ && !info->IsCancelled() &&
            IsAppfreezeEventTrackedLocked(info)) {
            WithdrawAppfreezeEventLocked(info, app);
        }
        *info->cancelled_ = true;
        cancel = std::exchange(info->cancelWatchdog_, {});
        cancelFeedback = std::exchange(info->cancelFeedback_, {});
        auto it = app->groups_.find(AppFreezeInfo::EventGroup(info->eventName));
        if (it != app->groups_.end()) {
            if (it->second == info) {
                finish = !info->running_;
            } else {
                auto& pending = it->second->pending_;
                pending.erase(std::remove(pending.begin(), pending.end(), info), pending.end());
            }
        }
    }
    if (cancel) {
        cancel();
    }
    if (cancelFeedback) {
        cancelFeedback();
    }
    if (finish) {
        FinishAppfreezeEvent(info);
    }
}

void AppfreezeManager::RemoveAppfreezeInstance(const std::string& generation, int32_t instanceId)
{
    std::vector<FreezeInfo> removed;
    {
        std::lock_guard<std::mutex> lock(appfreezeEventMutex_);
        auto it = appfreezeApps_.find(generation);
        if (it == appfreezeApps_.end() || it->second->instanceId != instanceId) {
            return;
        }
        auto& app = it->second;
        app->accepting_ = false;
        for (const auto& group : app->groups_) {
            auto& info = group.second;
            info->accepting_ = false;
            removed.insert(removed.end(), info->pending_.begin(), info->pending_.end());
            info->pending_.clear();
            removed.push_back(info);
        }
    }
    for (const auto& info : removed) {
        CancelAppfreezeEvent(info);
    }
}

void AppfreezeManager::CloseAppfreezeEvents()
{
    std::vector<FreezeInfo> removed;
    {
        std::lock_guard<std::mutex> lock(appfreezeEventMutex_);
        for (const auto& entry : appfreezeApps_) {
            const auto& app = entry.second;
            app->accepting_ = false;
            app->notifications_.clear();
            app->halfFlows_.clear();
            for (const auto& group : app->groups_) {
                const auto& info = group.second;
                info->accepting_ = false;
                removed.insert(removed.end(), info->pending_.begin(), info->pending_.end());
                info->pending_.clear();
                removed.push_back(info);
            }
        }
        recentFreezes_.clear();
        ++appfreezeTimerVersion_;
        appfreezeTimerDeadline_ = 0;
    }
    for (const auto& info : removed) {
        CancelAppfreezeEvent(info);
    }
}

void AppfreezeManager::ExpireAppfreezeHistoryLocked(int64_t now)
{
    const int64_t cutoff = now - APPFREEZE_WINDOW_MS;
    recentFreezes_.erase(recentFreezes_.begin(), recentFreezes_.upper_bound(cutoff));
    for (auto entry = appfreezeApps_.begin(); entry != appfreezeApps_.end();) {
        auto& app = entry->second;
        for (auto it = app->notifications_.begin(); it != app->notifications_.end();) {
            it = it->second <= cutoff ? app->notifications_.erase(it) : std::next(it);
        }
        for (auto it = app->halfFlows_.begin(); it != app->halfFlows_.end();) {
            auto& times = it->second;
            while (!times.empty() && times.front() <= cutoff) {
                times.pop_front();
            }
            it = times.empty() ? app->halfFlows_.erase(it) : std::next(it);
        }
        const bool hasCallback = app->mainCallbackTime_ >= 0 && app->mainCallbackTime_ > cutoff;
        entry = app->groups_.empty() && app->notifications_.empty() && app->halfFlows_.empty() && !hasCallback ?
            appfreezeApps_.erase(entry) : std::next(entry);
    }
}

void AppfreezeManager::ExpireAppfreezeEvents(int64_t now)
{
    std::vector<FreezeInfo> expired;
    auto collectPendingExpired = [&expired, now](const std::vector<FreezeInfo>& pending) {
        for (const auto& item : pending) {
            if (now >= item->deadline_) {
                expired.push_back(item);
            }
        }
    };
    {
        std::lock_guard<std::mutex> lock(appfreezeEventMutex_);
        ExpireAppfreezeHistoryLocked(now);
        for (const auto& entry : appfreezeApps_) {
            for (const auto& group : entry.second->groups_) {
                const auto& info = group.second;
                if (now >= info->deadline_ && (info->running_ || !info->watchdogArmed_)) {
                    expired.push_back(info);
                }
                collectPendingExpired(info->pending_);
            }
        }
    }
    for (auto it = expired.rbegin(); it != expired.rend(); ++it) {
        CancelAppfreezeEvent(*it);
    }
}

void AppfreezeManager::ScheduleAppfreezeMaintenance()
{
    int64_t delay;
    uint64_t version;
    {
        std::lock_guard<std::mutex> lock(appfreezeEventMutex_);
        if (appfreezeApps_.empty() && recentFreezes_.empty()) {
            return;
        }
        auto now = AbilityRuntime::TimeUtil::CurrentTimeMillis();
        auto next = now + APPFREEZE_WINDOW_MS;
        if (!recentFreezes_.empty()) {
            next = std::min(next, *recentFreezes_.begin() + APPFREEZE_WINDOW_MS);
        }
        for (const auto& entry : appfreezeApps_) {
            for (const auto& group : entry.second->groups_) {
                const auto& info = group.second;
                if (!info->IsCancelled() && (info->running_ || !info->watchdogArmed_)) {
                    next = std::min(next, info->deadline_);
                }
                for (const auto& pending : info->pending_) {
                    next = std::min(next, pending->deadline_);
                }
            }
        }
        if (appfreezeTimerDeadline_ > 0 && appfreezeTimerDeadline_ <= next) {
            return;
        }
        appfreezeTimerDeadline_ = next;
        delay = std::max<int64_t>(1, next - now);
        version = ++appfreezeTimerVersion_;
    }
    AAFwk::TaskHandlerWrap::GetFfrtHandler()->SubmitTask([weak = weak_from_this(), version] {
        auto inner = weak.lock();
        if (!inner) {
            return;
        }
        {
            std::lock_guard<std::mutex> lock(inner->appfreezeEventMutex_);
            if (inner->appfreezeTimerVersion_ != version) {
                return;
            }
            inner->appfreezeTimerDeadline_ = 0;
        }
        inner->ExpireAppfreezeEvents(AbilityRuntime::TimeUtil::CurrentTimeMillis());
        inner->ScheduleAppfreezeMaintenance();
        }, delay);
}

int AppfreezeManager::AppfreezeHandle(const FaultData& faultData, const AppfreezeManager::AppInfo& appInfo)
{
    TAG_LOGD(AAFwkTag::APPDFR, "called %{public}s, bundleName %{public}s, name_ %{public}s",
        faultData.errorObject.name.c_str(), appInfo.bundleName.c_str(), name_.c_str());
    if (!collector_->IsHandleAppfreeze(appInfo.bundleName)) {
        return -1;
    }
    HITRACE_METER_FMT(HITRACE_TAG_APP, "AppfreezeHandler:%{public}s bundleName:%{public}s",
        faultData.errorObject.name.c_str(), appInfo.bundleName.c_str());
    std::string memoryContent = "";
    collector_->CollectFreezeSysMemory(memoryContent);
    if (faultData.errorObject.name == AppFreezeType::APP_INPUT_BLOCK ||
        faultData.errorObject.name == AppFreezeType::THREAD_BLOCK_3S ||
        faultData.errorObject.name == AppFreezeType::LIFECYCLE_HALF_TIMEOUT ||
        faultData.errorObject.name == AppFreezeType::LIFECYCLE_HALF_TIMEOUT_WARNING) {
        AcquireStack(faultData, appInfo, memoryContent);
    } else {
        NotifyANR(faultData, appInfo, "", memoryContent);
    }
    return 0;
}

int AppfreezeManager::MergeNotifyInfo(FaultData& faultNotifyData, const AppfreezeManager::AppInfo& appInfo)
{
    std::string memoryContent;
    collector_->CollectFreezeSysMemory(memoryContent);
    std::string fileName = faultNotifyData.errorObject.name + "_" +
        AbilityRuntime::TimeUtil::FormatTime("%Y%m%d%H%M%S") + "_" + std::to_string(appInfo.pid) + "_stack";
    std::string catcherStack;
    faultNotifyData.errorObject.message += "\nCatche stack trace start time: " +
        AbilityRuntime::TimeUtil::DefaultCurrentTimeStr() + "\n";
    DumpTiming timing;
    if (faultNotifyData.errorObject.name == AppFreezeType::LIFECYCLE_HALF_TIMEOUT_WARNING ||
        faultNotifyData.errorObject.name == AppFreezeType::LIFECYCLE_TIMEOUT_WARNING) {
        catcherStack += collector_->CatcherStacktrace(appInfo.pid, &timing);
    } else {
        std::pair<std::string, std::string> catchResult =
            collector_->CatchJsonStacktrace(appInfo.pid, faultNotifyData.errorObject.name, &timing);
        catcherStack += catchResult.first;
    }
    faultNotifyData.logValidity += ":" + std::to_string(timing.result) + ":" + std::to_string(timing.latency) +
        ":" + std::to_string(faultNotifyData.ipcLatency);
    AppendLogValidity(faultNotifyData, appInfo);
    std::string timeStamp = "Catche stack trace end time: " + AbilityRuntime::TimeUtil::DefaultCurrentTimeStr();
    catcherStack = collector_->GetCatcherStack(fileName, catcherStack);
    faultNotifyData.errorObject.stack = catcherStack.empty() ? faultNotifyData.errorObject.stack : catcherStack;
    if (appInfo.isOccurException) {
        faultNotifyData.errorObject.message += "\nnotifyAppFault exception.\n";
    }
    faultNotifyData.errorObject.message += timeStamp;
    if (faultNotifyData.errorObject.name == AppFreezeType::APP_INPUT_BLOCK ||
        faultNotifyData.errorObject.name == AppFreezeType::THREAD_BLOCK_3S ||
        faultNotifyData.errorObject.name == AppFreezeType::LIFECYCLE_HALF_TIMEOUT ||
        faultNotifyData.errorObject.name == AppFreezeType::LIFECYCLE_HALF_TIMEOUT_WARNING) {
        AcquireStack(faultNotifyData, appInfo, memoryContent);
    } else {
        NotifyANR(faultNotifyData, appInfo, "", memoryContent);
    }
    return 0;
}

AppfreezeCaptureResult AppfreezeManager::RunAppfreezeAttachments(const FaultData& faultData, int32_t pid,
    const FreezeInfo& request, std::chrono::steady_clock::time_point deadline)
{
    AppfreezeCaptureResult result;
    const auto cancelled = [request, deadline] {
        return request->IsCancelled() || std::chrono::steady_clock::now() >= deadline;
    };
#ifdef __cpp_exceptions
    try {
#endif
        if (!cancelled()) {
            collector_->CollectFreezeSysMemory(result.memoryContent);
        }
        if (!cancelled() && (faultData.errorObject.name == AppFreezeType::APP_INPUT_BLOCK ||
            AppFreezeInfo::IsHalfEvent(faultData.errorObject.name))) {
            result.binderInfo = collector_->CollectBinderAttachment(faultData, pid, cancelled);
        }
        if (cancelled()) {
            result = {};
        }
#ifdef __cpp_exceptions
    } catch (...) {
        TAG_LOGE(AAFwkTag::APPDFR, "Appfreeze attachment collection threw, pid:%{public}d", pid);
        result = {};
    }
#endif
    // Timeout/cancellation never releases capacity while a collector is still executing.
    --activeAttachments_;
    return result;
}

ffrt::future<AppfreezeCaptureResult> AppfreezeManager::CollectAppfreezeAttachments(const FaultData& faultData,
    int32_t pid, const FreezeInfo& request, std::chrono::steady_clock::time_point deadline)
{
    if (!request || request->IsCancelled() || std::chrono::steady_clock::now() >= deadline) {
        return {};
    }
    auto active = activeAttachments_.load();
    do {
        if (active >= APPFREEZE_MAX_ATTACHMENTS) {
            return {};
        }
    } while (!activeAttachments_.compare_exchange_weak(active, active + 1));
#ifdef __cpp_exceptions
    try {
#endif
        ffrt::packaged_task<AppfreezeCaptureResult()> collect(
            [self = shared_from_this(), faultData, pid, request, deadline] {
                return self->RunAppfreezeAttachments(faultData, pid, request, deadline);
            });
        auto future = collect.get_future();
        ffrt::submit([collect = std::move(collect)]() mutable { collect(); },
            ffrt::task_attr().name("AppfreezeAttachment"));
        return future;
#ifdef __cpp_exceptions
    } catch (...) {
        TAG_LOGE(AAFwkTag::APPDFR, "Appfreeze attachment submission threw, pid:%{public}d", pid);
        --activeAttachments_;
        return {};
    }
#endif
}

bool AppfreezeManager::CaptureAppfreeze(const FaultData& faultData,
    const AppInfo& appInfo, const FreezeInfo& request, AppfreezeCaptureResult& result)
{
    if (!request || request->IsCancelled() || !collector_->IsHandleAppfreeze(appInfo.bundleName)) {
        return false;
    }
    const auto deadline = std::chrono::steady_clock::now() + APPFREEZE_ATTACHMENT_TIMEOUT;
    auto attachments = CollectAppfreezeAttachments(faultData, appInfo.pid, request, deadline);
    result.processStart = request->processStartTime;
    DumpTiming timing;
    std::string stack;
    if (faultData.errorObject.name == AppFreezeType::LIFECYCLE_HALF_TIMEOUT_WARNING ||
        faultData.errorObject.name == AppFreezeType::LIFECYCLE_TIMEOUT_WARNING) {
        stack = collector_->CatcherStacktrace(appInfo.pid, &timing);
    } else {
        stack = collector_->CatchJsonStacktrace(appInfo.pid, faultData.errorObject.name, &timing, true).first;
    }
    result.dumpResult = timing.result;
    result.dumpLatency = timing.latency;
    if (request->IsCancelled()) {
        return false;
    }
    const std::string fileName = faultData.errorObject.name + "_" + std::to_string(appInfo.pid) + "_" +
        request->episodeId + "_" + std::to_string(request->requestId) + "_stack";
    result.stack = collector_->GetCatcherStack(fileName, stack);
    // Consume only this future's value; a late FFRT task cannot modify the returned capture result.
    if (!request->IsCancelled() && attachments.valid() &&
        attachments.wait_until(deadline) == ffrt::future_status::ready) {
        auto collected = attachments.get();
        result.binderInfo = std::move(collected.binderInfo);
        result.memoryContent = std::move(collected.memoryContent);
    } else if (!request->IsCancelled() && attachments.valid()) {
        TAG_LOGW(AAFwkTag::APPDFR, "Appfreeze attachments timed out, pid:%{public}d", appInfo.pid);
    }
    return !request->IsCancelled();
}

int AppfreezeManager::ReportAppfreeze(const FaultData& faultData, const AppInfo& appInfo,
    const AppfreezeCaptureResult& result)
{
    const auto claim = appInfo.request ? appInfo.request->reportClaim : nullptr;
    const uint8_t severity = AppFreezeInfo::IsHalfEvent(faultData.errorObject.name) ? 1 : 2;
    if (claim && (claim->fetch_or(severity) & severity) != 0) {
        return 0;
    }
    FaultData report = faultData;
    const auto& event = report.errorObject.name;
    const bool usedAcquireStack = event == AppFreezeType::APP_INPUT_BLOCK ||
        event == AppFreezeType::THREAD_BLOCK_3S || event == AppFreezeType::LIFECYCLE_HALF_TIMEOUT ||
        event == AppFreezeType::LIFECYCLE_HALF_TIMEOUT_WARNING;
    if (usedAcquireStack && (event == AppFreezeType::APP_INPUT_BLOCK || report.tid <= 0)) {
        report.tid = appInfo.pid;
    }
    const int64_t start = result.processStart;
    const int64_t latency = report.detectTime > 0 && report.detectTime <= static_cast<uint64_t>(start) ?
        start - static_cast<int64_t>(report.detectTime) : -1;
    report.logValidity = std::to_string(latency) + ":" + std::to_string(result.dumpResult) + ":" +
        std::to_string(result.dumpLatency) + ":" + std::to_string(report.ipcLatency);
    AppendLogValidity(report, appInfo);
    if (!result.stack.empty()) {
        report.errorObject.stack = result.stack;
    }
    if (appInfo.isOccurException) {
        report.errorObject.message += "\nnotifyAppFault exception.\n";
    }
    // A negative transport result can already be queued by HiSysEvent. Releasing
    // this claim would let a waiter submit a second copy of the same report.
    return NotifyANR(report, appInfo, result.binderInfo, result.memoryContent);
}

void AppfreezeManager::ExpireLogValidityLocked(int32_t pid, int64_t now, int64_t logChainTimeoutMs)
{
    for (auto it = freezeLogInfo_.begin(); it != freezeLogInfo_.end();) {
        if (now - it->second.startTime > logChainTimeoutMs || now < it->second.startTime) {
            it = freezeLogInfo_.erase(it);
        } else {
            ++it;
        }
    }
    if (freezeLogInfo_.size() >= FREEZE_EVENT_MAX_SIZE && freezeLogInfo_.count(pid) == 0) {
        freezeLogInfo_.erase(freezeLogInfo_.begin());
    }
}

void AppfreezeManager::AppendLogValidity(FaultData& faultData, const AppInfo& appInfo)
{
    std::lock_guard<std::mutex> lock(freezeMapMutex_);
    const int64_t now = GetFreezeCurrentTime();
    constexpr int64_t logChainTimeoutMs = 15000;
    constexpr size_t maxLogItems = 5;
    ExpireLogValidityLocked(appInfo.pid, now, logChainTimeoutMs);
    auto& previous = freezeLogInfo_[appInfo.pid];
    const std::string identity = std::to_string(appInfo.uid) + ":" + appInfo.bundleName + ":" +
        faultData.appRunningUniqueId;
    const auto& eventName = faultData.errorObject.name;
    const std::string flow = eventName + ":" +
        std::to_string(reinterpret_cast<uintptr_t>(faultData.token.GetRefPtr())) +
        ":" + std::to_string(faultData.state);
    const bool newCheckpoint = faultData.detectTime > 0 && previous.checkpointTime > 0 &&
        ((faultData.detectTime > previous.checkpointTime &&
        faultData.detectTime - previous.checkpointTime > logChainTimeoutMs) ||
        (faultData.detectTime < previous.lastCheckpointTime &&
        previous.lastCheckpointTime - faultData.detectTime > logChainTimeoutMs));
    if (previous.identity != identity || previous.count >= maxLogItems || newCheckpoint ||
        (IsHalfTimeout(eventName) && previous.halfFlows.count(flow) > 0)) {
        previous = {};
    }
    if (previous.count == 0) {
        previous.identity = identity;
        previous.startTime = now;
        previous.checkpointTime = faultData.detectTime;
    } else {
        faultData.logValidity = previous.validity + "|" + faultData.logValidity;
    }
    // Preserve the bounded processing sequence, including interleaved lifecycle and input events.
    previous.validity = faultData.logValidity;
    if (faultData.detectTime > 0) {
        if (previous.checkpointTime == 0 || faultData.detectTime < previous.checkpointTime) {
            previous.checkpointTime = faultData.detectTime;
        }
        previous.lastCheckpointTime = std::max(previous.lastCheckpointTime, faultData.detectTime);
    }
    ++previous.count;
    if (IsHalfTimeout(eventName)) {
        previous.halfFlows.insert(flow);
    }
    // Store only timing items; the window snapshot belongs to this report alone.
    if (appInfo.request && appInfo.request->freezeCount1min >= 0) {
        faultData.logValidity += "|" + std::to_string(appInfo.request->freezeCount1min);
    }
}

int AppfreezeManager::AppfreezeHandleWithStack(const FaultData& faultData, const AppfreezeManager::AppInfo& appInfo)
{
    const int64_t processStart = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    TAG_LOGW(AAFwkTag::APPDFR, "NotifyAppFaultTask called, eventName:%{public}s, bundleName:%{public}s, "
        "name_:%{public}s, currentTime:%{public}s, isInForeground:%{public}d",
        faultData.errorObject.name.c_str(), appInfo.bundleName.c_str(),
        name_.c_str(), AbilityRuntime::TimeUtil::DefaultCurrentTimeStr().c_str(), faultData.isInForeground);
    if (!collector_->IsHandleAppfreeze(appInfo.bundleName)) {
        return -1;
    }
    FaultData faultNotifyData = PrepareFaultNotifyData(faultData, processStart);
    HITRACE_METER_FMT(HITRACE_TAG_APP, "AppfreezeHandleWithStack pid:%{public}d-name:%{public}s",
        appInfo.pid, faultData.errorObject.name.c_str());
    return MergeNotifyInfo(faultNotifyData, appInfo);
}

FaultData AppfreezeManager::PrepareFaultNotifyData(const FaultData& faultData, int64_t processStart)
{
    FaultData faultNotifyData;
    faultNotifyData.errorObject.name = faultData.errorObject.name;
    faultNotifyData.errorObject.message = faultData.errorObject.message;
    faultNotifyData.errorObject.stack = faultData.errorObject.stack;
    faultNotifyData.errorObject.mainStack = faultData.errorObject.mainStack;
    faultNotifyData.faultType = FaultDataType::APP_FREEZE;
    faultNotifyData.eventId = faultData.eventId;
    faultNotifyData.schedTime = faultData.schedTime;
    faultNotifyData.detectTime = faultData.detectTime;
    faultNotifyData.appStatus = faultData.appStatus;
    faultNotifyData.samplerStartTime = faultData.samplerStartTime;
    faultNotifyData.samplerFinishTime = faultData.samplerFinishTime;
    faultNotifyData.samplerCount = faultData.samplerCount;
    faultNotifyData.heaviestStack = faultData.heaviestStack;
    faultNotifyData.token = faultData.token;
    faultNotifyData.state = faultData.state;
    faultNotifyData.ipcLatency = faultData.ipcLatency;
    const int64_t latency = faultData.detectTime > 0 && faultData.detectTime <= static_cast<uint64_t>(processStart) ?
        processStart - static_cast<int64_t>(faultData.detectTime) : LATENCY_INVALID;
    faultNotifyData.logValidity = std::to_string(latency);
    faultNotifyData.pid = faultData.pid;
    faultNotifyData.tid = faultData.tid;
    faultNotifyData.appfreezeInfo = faultData.appfreezeInfo;
    faultNotifyData.appRunningUniqueId = faultData.appRunningUniqueId;
    faultNotifyData.procStatm = faultData.procStatm;
    faultNotifyData.isInForeground = faultData.isInForeground;
    faultNotifyData.isEnableMainThreadSample = faultData.isEnableMainThreadSample;
    faultNotifyData.reportLifecycleToFreeze = faultData.reportLifecycleToFreeze;
    faultNotifyData.applicationHeapInfo = faultData.applicationHeapInfo;
    faultNotifyData.applicationGCInfo = faultData.applicationGCInfo;
    faultNotifyData.applicationIOInfo = faultData.applicationIOInfo;
    faultNotifyData.isBlockInGc = faultData.isBlockInGc;
    faultNotifyData.processLifeTime = faultData.processLifeTime;
    faultNotifyData.markedId = faultData.markedId;
    faultNotifyData.processedId = faultData.processedId;
    faultNotifyData.dispatchedEventId = faultData.dispatchedEventId;
    faultNotifyData.callbackLog = faultData.callbackLog;
    return faultNotifyData;
}

int AppfreezeManager::LifecycleTimeoutHandle(const ParamInfo& info, FreezeUtil::LifecycleFlow flow)
{
    if (info.typeId != AppfreezeManager::TypeAttribute::CRITICAL_TIMEOUT ||
        !collector_->IsHandleAppfreeze(info.bundleName)) {
        return -1;
    }
    if (info.eventName != AppFreezeType::LIFECYCLE_TIMEOUT && info.eventName != AppFreezeType::LIFECYCLE_HALF_TIMEOUT
        && info.eventName != AppFreezeType::LIFECYCLE_TIMEOUT_WARNING
        && info.eventName != AppFreezeType::LIFECYCLE_HALF_TIMEOUT_WARNING) {
        return -1;
    }

    std::string faultTimeStr = "\nFault time:" + AbilityRuntime::TimeUtil::FormatTime("%Y/%m/%d-%H:%M:%S") + "\n";
    if (!g_betaVersion && info.eventName == AppFreezeType::LIFECYCLE_HALF_TIMEOUT) {
        auto hisyseventReport = std::make_shared<AAFwk::HisyseventReport>(2);
        hisyseventReport->InsertParam("PID", info.pid);
        hisyseventReport->InsertParam("PACKAGE_NAME", info.bundleName);
        int32_t ret = hisyseventReport->Report("AAFWK", "FREEZE_HALF_HIVIEW_LOG", HISYSEVENT_FAULT);
        TAG_LOGW(AAFwkTag::APPDFR, "hisysevent write FREEZE_HALF_HIVIEW_LOG, pid:%{public}d, packageName:%{public}s,"
            " ret:%{public}d", info.pid, info.bundleName.c_str(), ret);
    }

    std::string message;
    if (g_betaVersion && info.eventName == AppFreezeType::LIFECYCLE_TIMEOUT) {
        auto hisyseventReport = std::make_shared<AAFwk::HisyseventReport>(3);
        hisyseventReport->InsertParam("PID", info.pid);
        hisyseventReport->InsertParam("PACKAGE_NAME", info.bundleName);
        hisyseventReport->InsertParam("FAULT_TIME", faultTimeStr);
        int32_t ret = hisyseventReport->Report("AAFWK", "FREEZE_HALF_HIVIEW_LOG", HISYSEVENT_FAULT);
        message = (ret == 0) ? "FREEZE_HALF_HIVIEW_LOG write success" : "";
    }

    TAG_LOGD(AAFwkTag::APPDFR, "called %{public}s, name_ %{public}s", info.bundleName.c_str(), name_.c_str());
    HITRACE_METER_FMT(HITRACE_TAG_APP, "LifecycleTimeoutHandle:%{public}s bundleName:%{public}s",
        info.eventName.c_str(), info.bundleName.c_str());

    AppFaultDataBySA faultDataSA = GenerateFaultDataBySA(info, flow);
    faultDataSA.errorObject.message = faultTimeStr + faultDataSA.errorObject.message + message;
    DelayedSingleton<AppExecFwk::AppMgrClient>::GetInstance()->NotifyAppFaultBySA(faultDataSA);
    return 0;
}

AppFaultDataBySA AppfreezeManager::GenerateFaultDataBySA(const ParamInfo& info, const FreezeUtil::LifecycleFlow& flow)
{
    AppFaultDataBySA faultDataSA;
    if (info.eventName == AppFreezeType::LIFECYCLE_TIMEOUT) {
        std::string realPath = AppfreezeUtil::FreezePathToRealPath("/proc/" + std::to_string(info.pid) + "/statm");
        std::ifstream statmStream(realPath);
        if (statmStream) {
            std::string procStatm;
            std::getline(statmStream, procStatm);
            statmStream.close();
            faultDataSA.procStatm = procStatm;
        }
    }
    faultDataSA.errorObject.name = info.eventName;
    faultDataSA.errorObject.message = info.msg;
    faultDataSA.faultType = FaultDataType::APP_FREEZE;
    faultDataSA.timeoutMarkers = "notifyFault" + std::to_string(info.pid) +
                                 "-" + std::to_string(AbilityRuntime::TimeUtil::CurrentTimeMillis());
    faultDataSA.pid = info.pid;
    faultDataSA.needKillProcess = info.needKillProcess;
    faultDataSA.abilityPendingBackground = info.abilityPendingBackground;
    if (flow.state != AbilityRuntime::FreezeUtil::TimeoutState::UNKNOWN) {
        faultDataSA.token = flow.token;
        faultDataSA.state = static_cast<uint32_t>(flow.state);
    }
    faultDataSA.detectTime = static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count());
    return faultDataSA;
}

int AppfreezeManager::AcquireStack(const FaultData& faultData, const AppInfo& appInfo,
    const std::string& memoryContent)
{
    auto binderInfo = collector_->CollectBinderAttachment(faultData, appInfo.pid, {});
    return NotifyANR(collector_->GetFaultNotifyData(faultData, appInfo.pid), appInfo, binderInfo, memoryContent);
}

void AppfreezeManager::InitWarningCpuInfo(const FaultData& faultData,
    const AppfreezeManager::AppInfo& appInfo)
{
    std::string eventName = faultData.errorObject.name;
    if (eventName != AppFreezeType::THREAD_BLOCK_3S &&
        eventName != AppFreezeType::LIFECYCLE_HALF_TIMEOUT) {
        return;
    }
    std::string type = std::to_string(appInfo.pid) + "-" + std::to_string(appInfo.uid) + "-" +
        appInfo.bundleName;
    bool ret = AppExecFwk::AppfreezeCpuFreqManager::GetInstance().InsertCpuDetailInfo(type, appInfo.pid);
    TAG_LOGI(AAFwkTag::APPDFR, "Insert cpuInfo ret:%{public}d, pid:%{public}d, name:%{public}s, "
        "appfreezeInfo:%{public}s, type:%{public}s", ret, appInfo.pid,
        eventName.c_str(), faultData.appfreezeInfo.c_str(), type.c_str());
}

std::string AppfreezeManager::GetAppfreezeInfoPath(const FaultData& faultData,
    const AppfreezeManager::AppInfo& appInfo)
{
    std::string eventName = faultData.errorObject.name;
    std::string cpuInfoFile = faultData.appfreezeInfo;
    if (eventName == AppFreezeType::THREAD_BLOCK_6S || eventName == AppFreezeType::LIFECYCLE_TIMEOUT ||
        eventName == AppFreezeType::APP_INPUT_BLOCK) {
        std::string type = std::to_string(appInfo.pid) + "-" + std::to_string(appInfo.uid) + "-" +
            appInfo.bundleName;
        cpuInfoFile += "," + AppExecFwk::AppfreezeCpuFreqManager::GetInstance().GetCpuInfoPath(
            type, appInfo.bundleName, appInfo.uid, appInfo.pid);
        TAG_LOGI(AAFwkTag::APPDFR, "name:%{public}s, cpuInfoFile:%{public}s, type:%{public}s",
            faultData.errorObject.name.c_str(), cpuInfoFile.c_str(), type.c_str());
    }
    return cpuInfoFile;
}

AppfreezeEventInfo AppfreezeManager::UpdateEventInfo(
    const FaultData& faultData, const AppfreezeManager::AppInfo& appInfo,
    const std::string& binderInfo, const std::string& memoryContent)
{
    AppfreezeEventInfo eventInfo;
    int tid = faultData.tid;
    eventInfo.tid = tid > 0 ? tid : 0;
    eventInfo.pid = appInfo.pid;
    eventInfo.uid = appInfo.uid;
    eventInfo.isFrozen = appInfo.isFrozen;
    eventInfo.eventId = faultData.eventId;
    eventInfo.bundleName = appInfo.bundleName;
    eventInfo.processName = appInfo.processName;
    eventInfo.binderInfo = binderInfo;
    eventInfo.freezeMemory = memoryContent + "\n" + faultData.procStatm;
    eventInfo.appRunningUniqueId = faultData.appRunningUniqueId;
    eventInfo.errorStack = faultData.errorObject.stack;
    eventInfo.mainStack = faultData.errorObject.mainStack;
    eventInfo.errorName = faultData.errorObject.name;
    eventInfo.errorMessage = faultData.errorObject.message;
    eventInfo.freezeInfoFile = GetAppfreezeInfoPath(faultData, appInfo);
    eventInfo.hitraceInfo = collector_->GetHitraceInfo();
    eventInfo.foregroundState = faultData.isInForeground;
    eventInfo.enableFreeze = faultData.isEnableMainThreadSample;
    eventInfo.reportLifecycleToFreeze = faultData.reportLifecycleToFreeze;
    eventInfo.applicationHeapInfo = faultData.applicationHeapInfo;
    eventInfo.applicationGCInfo = faultData.applicationGCInfo;
    eventInfo.applicationIOInfo = faultData.applicationIOInfo;
    eventInfo.isBlockInGc = faultData.isBlockInGc;
    eventInfo.processLifeTime = faultData.processLifeTime;
    eventInfo.markedId = faultData.markedId;
    eventInfo.processedId = faultData.processedId;
    eventInfo.dispatchedEventId = faultData.dispatchedEventId;
    eventInfo.externalLog = faultData.callbackLog;
    eventInfo.samplerCount = faultData.samplerCount;
    eventInfo.heaviestStack = faultData.heaviestStack;
    eventInfo.logValidity = faultData.logValidity;
    return eventInfo;
}

int AppfreezeManager::NotifyANR(const FaultData& faultData, const AppfreezeManager::AppInfo& appInfo,
    const std::string& binderInfo, const std::string& memoryContent)
{
    std::string eventName = faultData.errorObject.name;
    this->PerfStart(eventName);
    int64_t startTime = AbilityRuntime::TimeUtil::CurrentTimeMillis();
    AppfreezeEventInfo eventInfo = UpdateEventInfo(faultData, appInfo, binderInfo, memoryContent);

    int ret = AppfreezeEventReport::SendAppfreezeEvent(eventName, HISYSEVENT_FAULT, eventInfo);
    TAG_LOGW(AAFwkTag::APPDFR, "reportEvent:%{public}s, pid:%{public}d, tid:%{public}d, bundleName:%{public}s, "
        "appRunningUniqueId:%{public}s, endTime:%{public}s, interval:%{public}" PRId64 " ms, "
        "eventId:%{public}d freezeInfoFile:%{public}s foreground:%{public}d enableFreeze:%{public}d,"
        " reportFreeze:%{public}d, applicationHeapInfo:%{public}s processLifeTime:%{public}s "
        "hisysevent write ret: %{public}d",
        faultData.errorObject.name.c_str(), appInfo.pid, faultData.tid, appInfo.bundleName.c_str(),
        faultData.appRunningUniqueId.c_str(), AbilityRuntime::TimeUtil::DefaultCurrentTimeStr().c_str(),
        AbilityRuntime::TimeUtil::CurrentTimeMillis() - startTime, faultData.eventId,
        eventInfo.freezeInfoFile.c_str(), eventInfo.foregroundState, eventInfo.enableFreeze,
        eventInfo.reportLifecycleToFreeze, eventInfo.applicationHeapInfo.c_str(),
        eventInfo.processLifeTime.c_str(), ret);
    OHOS::HiviewDFX::HiTraceChain::ClearId();
    return 0;
}

bool AppfreezeManager::IsProcessDebug(int32_t pid, std::string bundleName)
{
    std::lock_guard<ffrt::mutex> lock(freezeFilterMutex_);
    auto it = appfreezeFilterMap_.find(bundleName);
    if (it != appfreezeFilterMap_.end() && it->second.pid == pid) {
        bool result = (it->second.state == AppFreezeState::APPFREEZE_STATE_CANCELING);
        TAG_LOGW(AAFwkTag::APPDFR, "AppfreezeFilter: %{public}d, "
            "bundleName=%{public}s, pid:%{public}d, state:%{public}d, g_betaVersion:%{public}d,"
            " g_developMode:%{public}d",
            result, bundleName.c_str(), pid, it->second.state, g_betaVersion, g_developMode);
        return result;
    }

    const int buffSize = 128;
    char paramBundle[buffSize] = {0};
    GetParameter("hiviewdfx.appfreeze.filter_bundle_name", "", paramBundle, buffSize - 1);
    std::string debugBundle(paramBundle);

    if (bundleName.compare(debugBundle) == 0) {
        TAG_LOGI(AAFwkTag::APPDFR, "filtration %{public}s_%{public}s not exit",
            debugBundle.c_str(), bundleName.c_str());
        return true;
    }
    return false;
}

int64_t AppfreezeManager::GetFreezeCurrentTime()
{
    auto now = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()).count();
}

void AppfreezeManager::SetFreezeState(const std::string& key, int state, const std::string& eventName)
{
    if (eventName == AppFreezeType::THREAD_BLOCK_3S ||
        eventName == AppFreezeType::BUSSINESS_THREAD_BLOCK_3S) {
        return;
    }
    AppFreezeInfo info;
    info.state = state;
    info.occurTime = GetFreezeCurrentTime();
    std::lock_guard<ffrt::mutex> lock(freezeMutex_);
    appfreezeInfo_[key] = info;
}

int AppfreezeManager::GetReportTimes(const std::string& key)
{
    int reportTimes = DEFAULT_APPFREEZE_REPORT_TIMES;
    std::lock_guard<ffrt::mutex> lock(freezeMutex_);
    auto it = appfreezeInfo_.find(key);
    if (it != appfreezeInfo_.end()) {
        reportTimes += it->second.reportTimes;
        appfreezeInfo_[key].reportTimes = reportTimes;
    }
    return reportTimes;
}

int64_t AppfreezeManager::GetLastOccurTime(const std::string& key)
{
    std::lock_guard<ffrt::mutex> lock(freezeMutex_);
    auto it = appfreezeInfo_.find(key);
    if (it != appfreezeInfo_.end()) {
        return it->second.occurTime;
    }
    return 0;
}

bool AppfreezeManager::ClearOldInfo(std::map<std::string, AppFreezeInfo>& infoMap,
    size_t maxSize, int64_t maxTimelimit)
{
    if (infoMap.size() < maxSize) {
        return true;
    }
    int removeCount = 0;
    int64_t curTime = GetFreezeCurrentTime();
    for (auto it = infoMap.begin(); it != infoMap.end();) {
        auto interval = curTime - it->second.occurTime;
        if (interval > maxTimelimit || interval < 0) {
            it = infoMap.erase(it);
            removeCount++;
        } else {
            ++it;
        }
    }
    TAG_LOGI(AAFwkTag::APPDFR, "remove old count: %{public}d, "
        "current info count: %{public}zu", removeCount, infoMap.size());
    return removeCount != 0;
}

bool AppfreezeManager::IsNeedIgnoreFreezeEvent(const std::string& key, const std::string& eventName,
    int maxReportTimes)
{
    int state = 0;
    {
        std::lock_guard<ffrt::mutex> lock(freezeMutex_);
        if (!ClearOldInfo(appfreezeInfo_, FREEZEMAP_SIZE_MAX, FREEZE_TIME_LIMIT)) {
            return false;
        }
        auto it = appfreezeInfo_.find(key);
        state = it != appfreezeInfo_.end() ? it->second.state : AppFreezeState::APPFREEZE_STATE_IDLE;
    }
    if (state == AppFreezeState::APPFREEZE_STATE_FREEZE) {
        auto diff = GetFreezeCurrentTime() - GetLastOccurTime(key);
        int reportTimes = GetReportTimes(key);
        TAG_LOGW(AAFwkTag::APPDFR, "durationTime: %{public}" PRId64 " eventName: %{public}s "
            "reportTimes: %{public}d key: %{public}s",
            diff, eventName.c_str(), reportTimes, key.c_str());
        if (diff >= 0 && diff <= FREEZE_TIME_LIMIT) {
            return reportTimes > maxReportTimes;
        }
    }
    SetFreezeState(key, AppFreezeState::APPFREEZE_STATE_FREEZE, eventName);
    TAG_LOGI(AAFwkTag::APPDFR, "Set freeze info, eventName: %{public}s key: %{public}s",
        eventName.c_str(), key.c_str());
    return false;
}

int AppfreezeManager::ClearOldInputBlockFreezeInfo(size_t maxSize, int64_t maxTimelimit)
{
    if (appInputBlockFreezeInfo_.size() <= maxSize) {
        return 0;
    }
    int ret = 0;
    int64_t curTime = GetFreezeCurrentTime();
    for (auto it = appInputBlockFreezeInfo_.begin(); it != appInputBlockFreezeInfo_.end();) {
        auto interval = curTime - it->second;
        if (interval > maxTimelimit || interval < 0) {
            it = appInputBlockFreezeInfo_.erase(it);
            ret++;
        } else {
            ++it;
        }
    }
    return ret;
}

bool AppfreezeManager::IsNeedIgnoreInputBlockEvent(const std::string& key)
{
    std::lock_guard<ffrt::mutex> lock(freezeMutex_);
    ClearOldInputBlockFreezeInfo(FREEZEMAP_SIZE_MAX, FREEZE_TIME_LIMIT);

    int64_t curTime = GetFreezeCurrentTime();
    auto it = appInputBlockFreezeInfo_.find(key);
    if (it != appInputBlockFreezeInfo_.end()) {
        int64_t lastTime = it->second;
        int64_t diff = curTime - lastTime;
        if (diff >= 0 && diff <= FREEZE_TIME_LIMIT) {
            return true;
        }
    }
    appInputBlockFreezeInfo_[key] = curTime;
    TAG_LOGI(AAFwkTag::APPDFR, "Set input block info, key: %{public}s", key.c_str());
    return false;
}

bool AppfreezeManager::CancelAppFreezeDetect(int32_t pid, const std::string& bundleName)
{
    if (bundleName.empty()) {
        TAG_LOGE(AAFwkTag::APPDFR, "SetAppFreezeFilter: failed, bundleName is empty.");
        return false;
    }
    std::lock_guard<ffrt::mutex> lock(freezeFilterMutex_);
    ClearLogValidity(pid);
    AppFreezeInfo info;
    info.pid = pid;
    info.state = AppFreezeState::APPFREEZE_STATE_CANCELING;
    appfreezeFilterMap_[bundleName] = info;
    TAG_LOGI(AAFwkTag::APPDFR, "SetAppFreezeFilter: success, bundleName=%{public}s, "
        "pid:%{public}d, state:%{public}d", bundleName.c_str(), info.pid, info.state);
    return true;
}

void AppfreezeManager::RemoveDeathProcess(std::string bundleName)
{
    std::lock_guard<ffrt::mutex> lock(freezeFilterMutex_);
    auto it = appfreezeFilterMap_.find(bundleName);
    if (it != appfreezeFilterMap_.end()) {
        TAG_LOGI(AAFwkTag::APPDFR, "RemoveAppFreezeFilter:success, bundleName: %{public}s",
            bundleName.c_str());
        appfreezeFilterMap_.erase(it);
    } else {
        TAG_LOGD(AAFwkTag::APPDFR, "RemoveAppFreezeFilter:failed, not found bundleName: "
            "%{public}s", bundleName.c_str());
    }
}

void AppfreezeManager::ResetAppfreezeState(int32_t pid, const std::string& bundleName)
{
    std::lock_guard<ffrt::mutex> lock(freezeFilterMutex_);
    if (appfreezeFilterMap_.find(bundleName) != appfreezeFilterMap_.end()) {
        appfreezeFilterMap_[bundleName].state = AppFreezeState::APPFREEZE_STATE_CANCELED;
    }
    TAG_LOGI(AAFwkTag::APPDFR, "SetAppFreezeFilter: reset state, "
        "bundleName=%{public}s, pid:%{public}d, state:%{public}d",
        bundleName.c_str(), pid, appfreezeFilterMap_[bundleName].state);
}

bool AppfreezeManager::IsValidFreezeFilter(int32_t pid, const std::string& bundleName)
{
    if (g_betaVersion || g_developMode) {
        TAG_LOGI(AAFwkTag::APPDFR, "SetAppFreezeFilter: "
            "current device is beta or development");
        return true;
    }
    std::lock_guard<ffrt::mutex> lock(freezeFilterMutex_);
    bool ret = appfreezeFilterMap_.find(bundleName) == appfreezeFilterMap_.end();
    TAG_LOGI(AAFwkTag::APPDFR, "SetAppFreezeFilter: %{public}d, bundleName=%{public}s, "
        "pid:%{public}d", ret, bundleName.c_str(), pid);
    return ret;
}

void AppfreezeManager::ClearLogValidity(int32_t pid)
{
    std::lock_guard<std::mutex> lock(freezeMapMutex_);
    freezeLogInfo_.erase(pid);
}

void AppfreezeManager::PerfStart(std::string eventName)
{
    if (OHOS::system::GetParameter("const.dfx.sub_health_recovery.enable", "") != "true") {
        TAG_LOGI(AAFwkTag::APPDFR, "sub_health_recovery is not enable");
        return;
    }
    auto it = HALF_EVENT_CONFIGS.find(eventName);
    if (it == HALF_EVENT_CONFIGS.end()) {
        return;
    }
    auto curTime = AbilityRuntime::TimeUtil::SystemTimeMillisecond();
    if (curTime - perfTime < PERF_TIME) {
        TAG_LOGE(AAFwkTag::APPDFR, "perf time is less than 60s");
        return;
    }
    std::string bigCpuCurFreq = this->GetFirstLine(TWELVE_BIG_CPU_CUR_FREQ);
    std::string bigCpuMaxFreq = this->GetFirstLine(TWELVE_BIG_CPU_MAX_FREQ);
    std::string midCpuCurFreq = this->GetFirstLine(TWELVE_MID_CPU_CUR_FREQ);
    std::string midCpuMaxFreq = this->GetFirstLine(TWELVE_MID_CPU_MAX_FREQ);
    if (bigCpuCurFreq == bigCpuMaxFreq || midCpuCurFreq == midCpuMaxFreq) {
        perfTime = curTime;
        TAG_LOGI(AAFwkTag::APPDFR, "perf start");
        AAFwk::ResSchedUtil::GetInstance().ReportSubHealtyPerfInfoToRSS();
        TAG_LOGI(AAFwkTag::APPDFR, "perf end");
    }
}

bool AppfreezeManager::CheckInBackGround(const FaultData &faultData)
{
    return faultData.errorObject.name == AppFreezeType::THREAD_BLOCK_6S &&
        !faultData.isInForeground;
}

bool AppfreezeManager::CheckAppfreezeHappend(const std::string& key, const std::string& eventName)
{
    bool result = false;
    if (eventName == AppFreezeType::LIFECYCLE_TIMEOUT ||
        eventName == AppFreezeType::THREAD_BLOCK_6S || eventName == AppFreezeType::THREAD_BLOCK_3S ||
        eventName == AppFreezeType::BUSSINESS_THREAD_BLOCK_3S ||
        eventName == AppFreezeType::BUSSINESS_THREAD_BLOCK_6S) {
        result = IsNeedIgnoreFreezeEvent(key, eventName);
    } else if (eventName == AppFreezeType::LIFECYCLE_HALF_TIMEOUT_WARNING ||
        eventName == AppFreezeType::LIFECYCLE_TIMEOUT_WARNING) {
        std::string warningKey = eventName;
        result = IsNeedIgnoreFreezeEvent(warningKey, eventName);
    } else if (eventName == AppFreezeType::APP_INPUT_BLOCK) {
#ifdef APP_NO_RESPONSE_DIALOG
        result = IsNeedIgnoreInputBlockEvent(key);
#else
        result = IsNeedIgnoreFreezeEvent(key, eventName);
#endif
     }
    if (result) {
        TAG_LOGE(AAFwkTag::APPDFR, "appFreeze happend, key:%{public}s, eventName:%{public}s",
            key.c_str(), eventName.c_str());
    }
    return result;
}

bool AppfreezeManager::CheckNeedRecordAppRunningUnquieId(const std::string& eventName)
{
    if (eventName == AppFreezeType::THREAD_BLOCK_3S || eventName == AppFreezeType::THREAD_BLOCK_6S ||
        eventName == AppFreezeType::BUSSINESS_THREAD_BLOCK_3S ||
        eventName == AppFreezeType::BUSSINESS_THREAD_BLOCK_6S ||
        eventName == AppFreezeType::BUSINESS_INPUT_BLOCK) {
        return true;
    }
    return false;
}

bool AppfreezeManager::IsBetaVersion()
{
    return g_betaVersion;
}

void AppfreezeManager::InsertKillThread(int32_t state, int32_t pid, int32_t uid, const std::string& bundleName)
{
    std::string key = bundleName + AppfreezeUtil::KEY_SEPARATOR + std::to_string(pid) +
        AppfreezeUtil::KEY_SEPARATOR + std::to_string(uid);
    std::lock_guard<std::mutex> mapLock(freezeKillThreadMutex_);
    if (!ClearOldInfo(freezeKillThreadMap_, FREEZEMAP_SIZE_MAX, FREEZE_TIME_LIMIT)) {
        return;
    }
    if (freezeKillThreadMap_.find(key) != freezeKillThreadMap_.end()) {
        freezeKillThreadMap_[key].state = state;
        freezeKillThreadMap_[key].occurTime = GetFreezeCurrentTime();
    } else {
        AppFreezeInfo info;
        info.state = state;
        info.occurTime = GetFreezeCurrentTime();
        freezeKillThreadMap_[key] = info;
    }
    TAG_LOGI(AAFwkTag::APPDFR, "insert or update key: %{public}s", key.c_str());
}

bool AppfreezeManager::CheckThreadKilled(int32_t pid, int32_t uid, const std::string& bundleName)
{
    std::string key = bundleName + AppfreezeUtil::KEY_SEPARATOR + std::to_string(pid) +
        AppfreezeUtil::KEY_SEPARATOR + std::to_string(uid);
    bool result = false;
    {
        std::lock_guard<std::mutex> mapLock(freezeKillThreadMutex_);
        if (freezeKillThreadMap_.empty()) {
            return result;
        }
        auto it = freezeKillThreadMap_.find(key);
        if (it != freezeKillThreadMap_.end()) {
            result = it->second.state >= 0;
        }
    }

    if (result) {
        ClearLogValidity(pid);
    }
    return result;
}

bool AppfreezeManager::IsSkipDetect(int32_t pid, int32_t uid, const std::string& bundleName,
    const std::string& eventName)
{
    if (CheckThreadKilled(pid, uid, bundleName)) {
        TAG_LOGW(AAFwkTag::APPDFR, "bundleName: %{public}s has been killed, pid: %{public}d",
            bundleName.c_str(), pid);
        return true;
    }
    if (IsProcessDebug(pid, bundleName)) {
        TAG_LOGW(AAFwkTag::APPDFR, "don't report event and kill:%{public}s, pid:%{public}d, bundleName:%{public}s",
            eventName.c_str(), pid, bundleName.c_str());
        return true;
    }
    return false;
}

void AppfreezeManager::UpdateFreezeExcludedPid(bool isAdd, int32_t targetPid, int32_t profilerPid)
{
    std::lock_guard<ffrt::mutex> lock(freezeExcludedPidMutex_);
    if (isAdd) {
        FreezeExcludedPidInfo info;
        info.targetPid = targetPid;
        info.profilerPid = profilerPid;
        info.timestamp = GetFreezeCurrentTime();
        freezeExcludedPidMap_[targetPid] = info;
        TAG_LOGI(AAFwkTag::APPDFR, "Add to freezeExcludedPidMap: targetPid=%{public}d, profilerPid=%{public}d",
            targetPid, profilerPid);
        if (freezeExcludedPidMap_.size() > FREEZE_EXCLUDED_PID_MAX_SIZE) {
            CleanOldFreezeExcludedPids();
        }
    } else {
        freezeExcludedPidMap_.erase(targetPid);
        TAG_LOGI(AAFwkTag::APPDFR, "Remove from freezeExcludedPidMap: targetPid=%{public}d", targetPid);
    }
}

void AppfreezeManager::CleanOldFreezeExcludedPids()
{
    std::vector<std::pair<int32_t, int64_t>> pidTimePairs;
    for (const auto& pair : freezeExcludedPidMap_) {
        pidTimePairs.emplace_back(pair.first, pair.second.timestamp);
    }
    std::sort(pidTimePairs.begin(), pidTimePairs.end(),
        [](const auto& a, const auto& b) { return a.second < b.second; });

    size_t cleanCount = std::min(FREEZE_EXCLUDED_PID_CLEAN_COUNT, pidTimePairs.size());
    for (size_t i = 0; i < cleanCount; i++) {
        freezeExcludedPidMap_.erase(pidTimePairs[i].first);
        TAG_LOGI(AAFwkTag::APPDFR, "Clean old freezeExcludedPid: targetPid=%{public}d", pidTimePairs[i].first);
    }
}

bool AppfreezeManager::IsNativeDaemonProcess(int32_t profilerPid) const
{
    std::string processName = AppfreezeUtil::GetProcessNameByPid(profilerPid);
    return processName == NATIVE_DAEMON_PROCESS_NAME;
}

bool AppfreezeManager::IsFreezeExcludedPid(int32_t targetPid)
{
    std::lock_guard<ffrt::mutex> lock(freezeExcludedPidMutex_);
    auto it = freezeExcludedPidMap_.find(targetPid);
    if (it == freezeExcludedPidMap_.end()) {
        return false;
    }

    int32_t profilerPid = it->second.profilerPid;
    if (AppfreezeUtil::GetUidByPid(profilerPid) < 0) {
        TAG_LOGW(AAFwkTag::APPDFR, "profilerPid %{public}d not exist, remove targetPid %{public}d",
            profilerPid, targetPid);
        freezeExcludedPidMap_.erase(it);
        return false;
    }

    if (!IsNativeDaemonProcess(profilerPid)) {
        TAG_LOGW(AAFwkTag::APPDFR, "profilerPid %{public}d is not native_daemon, remove targetPid %{public}d",
            profilerPid, targetPid);
        freezeExcludedPidMap_.erase(it);
        return false;
    }

    TAG_LOGW(AAFwkTag::APPDFR, "pid %{public}d is in freeze excluded list", targetPid);
    return true;
}

bool AppfreezeManager::CheckPreloadUIExtension(const std::string& message, const std::string& bundleName,
    int32_t pid, const std::string& eventName)
{
    if (eventName != AppFreezeType::LIFECYCLE_HALF_TIMEOUT && eventName != AppFreezeType::LIFECYCLE_TIMEOUT) {
        return false;
    }
    if (message.find(PRELOAD_UIEXTENSION) != std::string::npos) {
        TAG_LOGW(AAFwkTag::APPDFR, "don't report event, msg: PreloadUIExtension, bundleName: %{public}s "
            "pid: %{public}d", bundleName.c_str(), pid);
        return true;
    }
    return false;
}

bool AppfreezeManager::CalculateFaultForeground(const std::string& eventName, const ForegroundState& state) const
{
    if (eventName == AppFreezeType::THREAD_BLOCK_3S || eventName == AppFreezeType::THREAD_BLOCK_6S ||
        eventName == AppFreezeType::BUSSINESS_THREAD_BLOCK_3S ||
        eventName == AppFreezeType::BUSSINESS_THREAD_BLOCK_6S) {
        return state.isInForeground;
    }

    if (state.abilityPendingBackground) {
        return false;
    }
    if (state.isTransitioningToBackground) {
        return false;
    }
    if (state.isStateForeground) {
        return true;
    }
    return state.isTransitioningToForeground;
}

bool AppfreezeManager::CheckProcessExit(const std::string& processName, const std::string& eventName, bool foreground)
{
    return foreground || eventName != AppFreezeType::THREAD_BLOCK_6S || processName == "com.ohos.sceneboard";
}

bool AppfreezeManager::IsValidFreezeTypeName(const std::string& freezeTypeName)
{
    return freezeTypeName == AppFreezeType::LIFECYCLE_HALF_TIMEOUT ||
        freezeTypeName == AppFreezeType::LIFECYCLE_HALF_TIMEOUT_WARNING ||
        freezeTypeName == AppFreezeType::LIFECYCLE_TIMEOUT ||
        freezeTypeName == AppFreezeType::LIFECYCLE_TIMEOUT_WARNING ||
        freezeTypeName == AppFreezeType::APP_LIFECYCLE_TIMEOUT ||
        freezeTypeName == AppFreezeType::THREAD_BLOCK_3S ||
        freezeTypeName == AppFreezeType::THREAD_BLOCK_6S ||
        freezeTypeName == AppFreezeType::APP_INPUT_BLOCK ||
        freezeTypeName == AppFreezeType::BUSSINESS_THREAD_BLOCK_3S ||
        freezeTypeName == AppFreezeType::BUSSINESS_THREAD_BLOCK_6S ||
        freezeTypeName == AppFreezeType::BG_FREEZE_WARNING ||
        freezeTypeName == AppFreezeType::BUSINESS_INPUT_BLOCK;
}
}  // namespace AppExecFwk
}  // namespace OHOS
