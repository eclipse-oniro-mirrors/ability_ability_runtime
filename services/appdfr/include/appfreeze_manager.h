/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_APPFREEZE_MAMAGER_H
#define OHOS_ABILITY_RUNTIME_APPFREEZE_MAMAGER_H

#include <sys/types.h>

#include <atomic>
#include <chrono>
#include <fstream>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <unordered_set>
#include <unordered_map>
#include <vector>

#include "cpp/mutex.h"
#include "cpp/condition_variable.h"
#include "fault_data.h"
#include "freeze_util.h"
#include "appfreeze_event_report.h"
#include "appfreeze_info.h"
#include "appfreeze_binder_collector.h"

namespace ffrt {
template<typename R>
class future;
}

namespace OHOS {
using AbilityRuntime::FreezeUtil;
namespace AppExecFwk {
struct AppfreezeAppInfo {
    bool isOccurException = false;
    bool isFrozen = false;
    int pid = 0;
    int uid = 0;
    std::string bundleName;
    std::string processName;
    FreezeInfo request;
    int32_t instanceId = 0;
    bool focusFlag = false;
    std::function<bool(AppfreezeAppInfo&, bool)> refresh;
    std::function<void(const FaultData&, const AppfreezeAppInfo&, bool)> handleFault;
};

class AppfreezeManager : public std::enable_shared_from_this<AppfreezeManager> {
private:
    using AppFreezeInfo = OHOS::AppExecFwk::AppFreezeInfo;

public:
    using FreezeInfo = std::shared_ptr<AppFreezeInfo>;
    using FreezeTask = std::function<void()>;
    using FreezeProcess = std::function<void(const FreezeInfo&)>;
    using AppInfo = AppfreezeAppInfo;
    AppfreezeSubmitResult SubmitAppfreezeEvent(const FreezeInfo& info, FreezeProcess process,
        const std::shared_ptr<AAFwk::TaskHandlerWrap>& handler);
    FreezeInfo ClaimAppfreezeFeedback(int32_t pid, int32_t instanceId,
        const std::string& marker, const std::string& eventName);
    bool CanPublishAppfreeze(const FreezeInfo& info, int64_t now);
    void CancelAppfreezeEvent(const FreezeInfo& info, bool withdraw = false);
    void SetAppfreezeFeedbackTask(const FreezeInfo& info, FreezeTask cancel);
    void RemoveAppfreezeInstance(const std::string& generation, int32_t instanceId);
    void CloseAppfreezeEvents();

    FreezeInfo ReserveAppfreezeEvent(const AppInfo& appInfo, const AppfreezeObservation& observation,
        int64_t now, const std::string& marker = {}, AppfreezeSubmitResult* result = nullptr);
    AppfreezeSubmitResult SubmitAppfreezeEvent(const FreezeInfo& info, const AppInfo& appInfo,
        FreezeProcess process, const std::shared_ptr<AAFwk::TaskHandlerWrap>& handler);
    bool ProcessAppfreezeEvent(const FreezeInfo& info, const AppInfo& appInfo,
        const FreezeTask& disposition);

    int32_t SubmitAppfreezeFault(const FaultData& faultData, AppInfo appInfo,
        const std::shared_ptr<AAFwk::TaskHandlerWrap>& handler, const FreezeInfo& reserved = nullptr,
        bool saTimeout = false);
    bool PrepareAppfreezeFeedback(const FaultData& faultData, int32_t pid, int32_t instanceId,
        const std::shared_ptr<AAFwk::TaskHandlerWrap>& handler, FreezeInfo& reserved);
    int32_t PrepareAppfreezeNotification(FaultData& faultData, AppInfo appInfo,
        const std::shared_ptr<AAFwk::TaskHandlerWrap>& handler, FreezeInfo& reserved);
    void TimeoutNotifyApp(const FaultData& faultData, AppInfo appInfo,
        const std::shared_ptr<AAFwk::TaskHandlerWrap>& handler);
    void ClearAppfreezeTasks(int32_t pid, int32_t uid, int32_t instanceId);
    int GetExceptionTimerId(const std::function<void()>& fallback);

    enum TypeAttribute {
        NORMAL_TIMEOUT = 0,
        CRITICAL_TIMEOUT = 1,
    };

    enum AppFreezeState {
        APPFREEZE_STATE_IDLE = 0,
        APPFREEZE_STATE_FREEZE = 1,
        APPFREEZE_STATE_CANCELING = 2,
        APPFREEZE_STATE_CANCELED = 3,
    };

    struct ParamInfo {
        bool needKillProcess = true;
        int typeId = TypeAttribute::NORMAL_TIMEOUT;
        int32_t pid = 0;
        std::string eventName;
        std::string bundleName;
        std::string msg;
        bool abilityPendingBackground = false;
    };

    struct ForegroundState {
        bool isStateForeground = false;
        bool isTransitioningToForeground = false;
        bool isTransitioningToBackground = false;
        bool abilityPendingBackground = false;
        bool isInForeground = false;
    };

    AppfreezeManager();
    ~AppfreezeManager();

    static std::shared_ptr<AppfreezeManager> GetInstance();
    static void DestroyInstance();
    int AppfreezeHandle(const FaultData& faultData, const AppfreezeManager::AppInfo& appInfo);
    int AppfreezeHandleWithStack(const FaultData& faultData, const AppfreezeManager::AppInfo& appInfo);
    bool CaptureAppfreeze(const FaultData& faultData,
        const AppInfo& appInfo, const FreezeInfo& request, AppfreezeCaptureResult& result);
    int ReportAppfreeze(const FaultData& faultData, const AppInfo& appInfo, const AppfreezeCaptureResult& result);
    int LifecycleTimeoutHandle(const ParamInfo& info, FreezeUtil::LifecycleFlow flow = FreezeUtil::LifecycleFlow());
    bool IsProcessDebug(int32_t pid, std::string bundleName);
    bool CancelAppFreezeDetect(int32_t pid, const std::string& bundleName);
    void RemoveDeathProcess(std::string bundleName);
    void ResetAppfreezeState(int32_t pid, const std::string& bundleName);
    bool IsValidFreezeFilter(int32_t pid, const std::string& bundleName);
    void ClearLogValidity(int32_t pid);
    void InitWarningCpuInfo(const FaultData& faultData, const AppfreezeManager::AppInfo& appInfo);
    bool CheckInBackGround(const FaultData &faultData);
    bool CheckAppfreezeHappend(const std::string& key, const std::string& eventName);
    bool CheckNeedRecordAppRunningUnquieId(const std::string& eventName);
    bool IsBetaVersion();
    void InsertKillThread(int32_t state, int32_t pid, int32_t uid, const std::string& bundleName);
    bool IsSkipDetect(int32_t pid, int32_t uid, const std::string& bundleName,
        const std::string& eventName);
    void UpdateFreezeExcludedPid(bool isAdd, int32_t targetPid, int32_t profilerPid);
    bool IsFreezeExcludedPid(int32_t targetPid);
    bool CheckPreloadUIExtension(const std::string& message, const std::string& bundleName, int32_t pid,
        const std::string& eventName);
    bool CalculateFaultForeground(const std::string& eventName, const ForegroundState& state) const;
    bool CheckProcessExit(const std::string& processName, const std::string& eventName, bool foreground);
    bool IsValidFreezeTypeName(const std::string& freezeTypeName);
    using DumpTiming = AppfreezeBinderCollector::DumpTiming;
    using PeerBinderInfo = AppfreezeBinderCollector::PeerBinderInfo;
    using TerminalBinder = AppfreezeBinderCollector::TerminalBinder;
    using ParseBinderParam = AppfreezeBinderCollector::ParseBinderParam;
    using ProcessKillInfo = AppfreezeBinderCollector::ProcessKillInfo;
    void DeleteStack(int pid) { collector_->DeleteStack(pid); }
    bool GetProcessKillReason(ProcessKillInfo& killInfo, int32_t killId, const std::string& killMsg)
        { return collector_->GetProcessKillReason(killInfo, killId, killMsg); }
    int GetFreezeExitReason(const std::string& eventName) { return collector_->GetFreezeExitReason(eventName); }

private:
    std::string WriteToFile(const std::string& fileName, const std::string& content)
        { return collector_->WriteToFile(fileName, content); }
    bool IsHandleAppfreeze(const std::string& bundleName) { return collector_->IsHandleAppfreeze(bundleName); }
    std::string GetCatcherStack(const std::string& fileName, const std::string& catcherStack)
        { return collector_->GetCatcherStack(fileName, catcherStack); }
    FaultData GetFaultNotifyData(const FaultData& faultData, int pid)
        { return collector_->GetFaultNotifyData(faultData, pid); }
    bool IsAncoProc(int pid) { return collector_->IsAncoProc(pid); }
    std::string CatchASyncByPid(const std::set<int>& asyncPids, const std::set<int>& syncPids, int pid)
        { return collector_->CatchASyncByPid(asyncPids, syncPids, pid); }
    std::string CatcherStacktrace(int pid, DumpTiming* timing = nullptr) const
        { return collector_->CatcherStacktrace(pid, timing); }
    std::pair<std::string, std::string> CatchJsonStacktrace(int pid, const std::string& faultType,
        DumpTiming* timing = nullptr, bool fresh = false) const
        { return collector_->CatchJsonStacktrace(pid, faultType, timing, fresh); }
    void FindStackByPid(std::string& msg, int pid) const { collector_->FindStackByPid(msg, pid); }
    bool GetExitKernelReason(ProcessKillInfo& killInfo) { return collector_->GetExitKernelReason(killInfo); }
    std::string ParseDecToHex(uint64_t id) { return collector_->ParseDecToHex(id); }
    std::string GetHitraceInfo() { return collector_->GetHitraceInfo(); }
    std::string StrSplit(const std::string& str, uint16_t index) const { return collector_->StrSplit(str, index); }
    void ParseBinderPids(const std::map<int, std::list<PeerBinderInfo>>& binderInfos,
        std::set<int>& pids, ParseBinderParam params, bool getTerminal,
        TerminalBinder& terminalBinder) const
        { collector_->ParseBinderPids(binderInfos, pids, params, getTerminal, terminalBinder); }
    std::set<int> GetBinderPeerPids(std::string& stack, ParseBinderParam params,
        std::set<int>& asyncPids, TerminalBinder& terminalBinder) const
        { return collector_->GetBinderPeerPids(stack, params, asyncPids, terminalBinder); }
    FreezeInfo ReserveAppfreezeTask(const FaultData& faultData, AppInfo& appInfo,
        const std::string& marker, AppfreezeSubmitResult& result);
    AppfreezeSubmitResult SubmitAppfreezeCapture(AppInfo appInfo, const FreezeInfo& request,
        const std::shared_ptr<AAFwk::TaskHandlerWrap>& handler, bool saTimeout);
    void ExpireAppfreezeEvents(int64_t now);
    void ScheduleAppfreezeMaintenance();
    bool IsAppfreezeEventTrackedLocked(const FreezeInfo& info) const;
    void InitAppfreezeObservation(const FreezeInfo& info, const AppfreezeObservation& observation);
    FreezeInfo GetAppfreezeApplicationLocked(const FreezeInfo& info);
    FreezeInfo FindAppfreezeHalfLocked(const FreezeInfo& info, const FreezeInfo& current) const;
    bool PrepareAppfreezeAdmissionLocked(const FreezeInfo& info, int64_t now, FreezeTask& cancelDiscardedFeedback);
    bool PrepareAppfreezeCallbackLocked(const FreezeInfo& info, int64_t now);
    AppfreezeSubmitResult EnqueueAppfreezeEventLocked(const FreezeInfo& info, int64_t now);
    void WithdrawAppfreezeEventLocked(const FreezeInfo& info, const FreezeInfo& app);
    void ObserveAppfreezeLocked(const FreezeInfo& info, int64_t now);
    void ExpireAppfreezeHistoryLocked(int64_t now);
    bool DispatchAppfreezeEvent(const FreezeInfo& info);
    void RunAppfreezeEvent(const FreezeInfo& info, bool fallback = false);
    void FinishAppfreezeEvent(const FreezeInfo& info);

    std::mutex appfreezeEventMutex_;
    std::map<std::string, FreezeInfo> appfreezeApps_;
    std::multiset<int64_t> recentFreezes_;
    uint64_t appfreezeTimerVersion_ = 0;
    int64_t appfreezeTimerDeadline_ = 0;
    int64_t lastAppfreezeEpisode_ = 0;

    struct FreezeLogInfo {
        std::string identity;
        std::string validity;
        std::set<std::string> halfFlows;
        uint64_t checkpointTime = 0;
        uint64_t lastCheckpointTime = 0;
        int64_t startTime = 0;
        size_t count = 0;
    };

    struct FreezeExcludedPidInfo {
        int32_t targetPid = 0;
        int32_t profilerPid = 0;
        int64_t timestamp = 0;
    };

    AppfreezeManager& operator=(const AppfreezeManager&) = delete;
    AppfreezeManager(const AppfreezeManager&) = delete;
    int AcquireStack(const FaultData& faultData, const AppInfo& appInfo, const std::string& memoryContent);
    AppfreezeCaptureResult RunAppfreezeAttachments(const FaultData& faultData, int32_t pid, const FreezeInfo& request,
        std::chrono::steady_clock::time_point deadline);
    __attribute__((visibility("hidden"))) ffrt::future<AppfreezeCaptureResult> CollectAppfreezeAttachments(
        const FaultData& faultData, int32_t pid, const FreezeInfo& request,
        std::chrono::steady_clock::time_point deadline);
    std::string GetAppfreezeInfoPath(const FaultData& faultData, const AppfreezeManager::AppInfo& appInfo);
    int NotifyANR(const FaultData& faultData, const AppfreezeManager::AppInfo& appInfo,
        const std::string& binderInfo, const std::string& memoryContent);
    int MergeNotifyInfo(FaultData& faultNotifyData, const AppfreezeManager::AppInfo& appInfo);
    void ExpireLogValidityLocked(int32_t pid, int64_t now, int64_t logChainTimeoutMs);
    void AppendLogValidity(FaultData& faultData, const AppInfo& appInfo);
    FaultData PrepareFaultNotifyData(const FaultData& faultData, int64_t processStart);
    AppFaultDataBySA GenerateFaultDataBySA(const ParamInfo& info, const FreezeUtil::LifecycleFlow& flow);
    void PerfStart(std::string eventName);
    std::string GetFirstLine(const std::string& path) { return collector_->GetFirstLine(path); }
    bool GetHitraceId(std::string& hiTraceIdInfo);
    bool CheckThreadKilled(int32_t pid, int32_t uid, const std::string& bundleName);
    bool IsNeedIgnoreFreezeEvent(const std::string& key, const std::string& eventName,
        int maxReportTimes = DEFAULT_APPFREEZE_REPORT_TIMES);
    bool IsNeedIgnoreInputBlockEvent(const std::string& key);
    int ClearOldInputBlockFreezeInfo(size_t maxSize, int64_t maxTimelimit);
    int64_t GetFreezeCurrentTime();
    void SetFreezeState(const std::string& key, int state, const std::string& eventName);
    int GetReportTimes(const std::string& key);
    int64_t GetLastOccurTime(const std::string& key);
    bool ClearOldInfo(std::map<std::string, AppFreezeInfo>& infoMap, size_t maxSize, int64_t maxTimelimit);
    bool IsHalfTimeout(const std::string& faultType) const { return collector_->IsHalfTimeout(faultType); }
    void CleanOldFreezeExcludedPids();
    bool IsNativeDaemonProcess(int32_t profilerPid) const;
    AppfreezeEventInfo UpdateEventInfo(const FaultData& faultData, const AppfreezeManager::AppInfo& appInfo,
        const std::string& binderInfo, const std::string& memoryContent);

    static constexpr size_t FREEZE_EXCLUDED_PID_MAX_SIZE = 100;
    static constexpr size_t FREEZE_EXCLUDED_PID_CLEAN_COUNT = 30;
    static constexpr const char* const NATIVE_DAEMON_PROCESS_NAME = "native_daemon";

    static const inline std::string LOGGER_DEBUG_PROC_PATH = "/proc/transaction_proc";
    std::string name_;
    static ffrt::mutex singletonMutex_;
    static std::shared_ptr<AppfreezeManager> instance_;
    static ffrt::mutex freezeMutex_;
    std::map<std::string, AppFreezeInfo> appfreezeInfo_;
    std::map<std::string, int64_t> appInputBlockFreezeInfo_;
    static ffrt::mutex freezeFilterMutex_;
    std::map<std::string, AppFreezeInfo> appfreezeFilterMap_;
    int64_t perfTime = 0;
    static ffrt::mutex freezeInfoMutex_;
    static std::string appfreezeInfoPath_;
    std::mutex freezeMapMutex_;
    std::map<int32_t, FreezeLogInfo> freezeLogInfo_;
    std::mutex freezeKillThreadMutex_;
    std::map<std::string, AppFreezeInfo> freezeKillThreadMap_;
    mutable ffrt::mutex freezeExcludedPidMutex_;
    std::unordered_map<int32_t, FreezeExcludedPidInfo> freezeExcludedPidMap_;
    // Includes queued collectors; only their actual completion releases capacity.
    std::atomic<size_t> activeAttachments_ {0};
    std::shared_ptr<AppfreezeBinderCollector> collector_;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_APPFREEZE_MAMAGER_H
