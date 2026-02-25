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

#include <fstream>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include "cpp/mutex.h"
#include "cpp/condition_variable.h"
#include "fault_data.h"
#include "freeze_util.h"

namespace OHOS {
using AbilityRuntime::FreezeUtil;
namespace AppExecFwk {
static const std::vector<std::string> APP_FREEZE_EVENT_NAME = {
    "THREAD_BLOCK_3S",
    "THREAD_BLOCK_6S",
    "APP_INPUT_BLOCK",
    "LIFECYCLE_HALF_TIMEOUT",
    "LIFECYCLE_TIMEOUT",
};
static constexpr int DEFAULT_APPFREEZE_REPORT_TIMES = 1;

class AppfreezeManager : public std::enable_shared_from_this<AppfreezeManager> {
public:
    struct AppInfo {
        std::string bundleName;
        std::string processName;
        int pid;
        int uid;
        bool isOccurException = false;
    };

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
    };

    struct AppfreezeEventRecord {
        std::string dumpResult;
        uint64_t schedTime = 0;
        uint64_t detectTime = 0;
        uint64_t dumpStartTime = 0;
        uint64_t dumpFinishTime = 0;
        int32_t appStatus = -1;
        bool isRepeatKilledThread = false;
    };

    AppfreezeManager();
    ~AppfreezeManager();

    static std::shared_ptr<AppfreezeManager> GetInstance();
    static void DestroyInstance();
    int AppfreezeHandle(const FaultData& faultData, const AppfreezeManager::AppInfo& appInfo);
    int AppfreezeHandleWithStack(const FaultData& faultData, const AppfreezeManager::AppInfo& appInfo);
    int LifecycleTimeoutHandle(const ParamInfo& info, FreezeUtil::LifecycleFlow flow = FreezeUtil::LifecycleFlow());
    std::string WriteToFile(const std::string& fileName, const std::string& content);
    bool IsHandleAppfreeze(const std::string& bundleName);
    bool IsProcessDebug(int32_t pid, std::string bundleName);
    void DeleteStack(int pid);
    bool CancelAppFreezeDetect(int32_t pid, const std::string& bundleName);
    void RemoveDeathProcess(std::string bundleName);
    void ResetAppfreezeState(int32_t pid, const std::string& bundleName);
    bool IsValidFreezeFilter(int32_t pid, const std::string& bundleName);
    void ReportAppFreezeSysEvents(int32_t pid, const std::string& bundleName);
    void RegisterAppKillTime(int32_t pid, uint64_t killTime);
    void InitWarningCpuInfo(const FaultData& faultData, const AppfreezeManager::AppInfo& appInfo);
    bool CheckInBackGround(const FaultData &faultData);
    bool CheckAppfreezeHappend(const std::string& key, const std::string& eventName);
    bool CheckNeedRecordAppRunningUnquieId(const std::string& eventName);
    bool IsBetaVersion();
    void InsertKillThread(int32_t state, int32_t pid, int32_t uid, const std::string& bundleName);
    bool IsSkipDetect(int32_t pid, int32_t uid, const std::string& bundleName,
        const std::string& eventName);
    std::string GetExitReasonByKillId(int32_t killId);
    std::string GetExitKernelReason(int32_t pid);

private:
    struct PeerBinderInfo {
        int32_t clientPid;
        int32_t clientTid;
        int32_t serverPid;
        int32_t serverTid;
    };

    struct TerminalBinder {
        int32_t pid;
        int32_t tid;
    };

    struct ParseBinderParam {
        int32_t eventPid;
        int32_t eventTid;
        int32_t pid;
        int layer;
    };

    struct AppFreezeInfo {
        int64_t occurTime = 0;
        int pid = 0;
        int reportTimes = DEFAULT_APPFREEZE_REPORT_TIMES;
        int state = 0;
        bool isRepeatKilledThread = false;
    };

    AppfreezeManager& operator=(const AppfreezeManager&) = delete;
    AppfreezeManager(const AppfreezeManager&) = delete;
    std::map<int, std::list<AppfreezeManager::PeerBinderInfo>> BinderParser(std::ifstream& fin, std::string& stack,
        std::set<int>& asyncPids) const;
    std::map<int, std::list<AppfreezeManager::PeerBinderInfo>> BinderLineParser(std::ifstream& fin, std::string& stack,
        std::map<uint32_t, uint32_t>& asyncBinderMap,
        std::vector<std::pair<uint32_t, uint64_t>>& freeAsyncSpacePairs) const;
    std::vector<std::string> GetFileToList(std::string line) const;
    std::string StrSplit(const std::string& str, uint16_t index) const;
    void ParseBinderPids(const std::map<int, std::list<AppfreezeManager::PeerBinderInfo>>& binderInfos,
        std::set<int>& pids, AppfreezeManager::ParseBinderParam params, bool getTerminal,
        AppfreezeManager::TerminalBinder& terminalBinder) const;
    std::set<int> GetBinderPeerPids(std::string& stack, AppfreezeManager::ParseBinderParam params,
        std::set<int>& asyncPids, AppfreezeManager::TerminalBinder& terminalBinder) const;
    void FindStackByPid(std::string& msg, int pid) const;
    std::pair<std::string, std::string> CatchJsonStacktrace(
        int pid, const std::string& faultType, const std::string& stack) const;
    std::string CatcherStacktrace(int pid, const std::string& stack) const;
    FaultData GetFaultNotifyData(const FaultData& faultData, int pid);
    int AcquireStack(const FaultData& faultData, const AppInfo& appInfo, const std::string& memoryContent);
    std::string GetAppfreezeInfoPath(const FaultData& faultData, const AppfreezeManager::AppInfo& appInfo);
    int NotifyANR(const FaultData& faultData, const AppfreezeManager::AppInfo& appInfo,
        const std::string& binderInfo, const std::string& memoryContent);
    void CollectFreezeSysMemory(std::string& memoryContent);
    int MergeNotifyInfo(FaultData& faultNotifyData, const AppfreezeManager::AppInfo& appInfo);
    void RecordAppFreezeBehavior(FaultData& faultData, uint64_t dumpStartTime,
        uint64_t dumpFinishTime, const std::string& dumpResult);
    std::string ParseDecToHex(uint64_t id);
    std::string GetHitraceInfo();
    AppFaultDataBySA GenerateFaultDataBySA(const ParamInfo& info, const FreezeUtil::LifecycleFlow& flow);
    void PerfStart(std::string eventName);
    std::string GetFirstLine(const std::string &path);
    bool GetHitraceId(std::string& hiTraceIdInfo);
    std::string CatchASyncByPid(const std::set<int> &asyncPids, const std::set<int> &syncPids, int pid);
    bool CheckThreadKilled(int32_t pid, int32_t uid, const std::string& bundleName);
    std::string GetCatcherStack(const std::string& fileName, const std::string& catcherStack);
    bool IsNeedIgnoreFreezeEvent(const std::string& key, const std::string& eventName,
        int maxReportTimes = DEFAULT_APPFREEZE_REPORT_TIMES);
    int64_t GetFreezeCurrentTime();
    void SetFreezeState(const std::string& key, int state, const std::string& eventName);
    int GetReportTimes(const std::string& key);
    int64_t GetLastOccurTime(const std::string& key);
    bool ClearOldInfo(std::map<std::string, AppFreezeInfo>& infoMap, size_t maxSize, int64_t maxTimelimit);

    static const inline std::string LOGGER_DEBUG_PROC_PATH = "/proc/transaction_proc";
    std::string name_;
    static ffrt::mutex singletonMutex_;
    static std::shared_ptr<AppfreezeManager> instance_;
    static ffrt::mutex freezeMutex_;
    std::map<std::string, AppFreezeInfo> appfreezeInfo_;
    static ffrt::mutex catchStackMutex_;
    static std::map<int, std::string> catchStackMap_;
    static ffrt::mutex freezeFilterMutex_;
    std::map<std::string, AppFreezeInfo> appfreezeFilterMap_;
    int64_t perfTime = 0;
    static ffrt::mutex freezeInfoMutex_;
    static std::string appfreezeInfoPath_;
    std::mutex freezeMapMutex_;
    std::map<int32_t, std::map<std::string, AppfreezeEventRecord>> freezeEventMap_;
    std::mutex freezeKillThreadMutex_;
    std::map<std::string, AppFreezeInfo> freezeKillThreadMap_;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_APPFREEZE_MAMAGER_H