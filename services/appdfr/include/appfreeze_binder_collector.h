/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef OHOS_ABILITY_RUNTIME_APPFREEZE_BINDER_COLLECTOR_H
#define OHOS_ABILITY_RUNTIME_APPFREEZE_BINDER_COLLECTOR_H

#include <atomic>
#include <cstdint>
#include <fstream>
#include <functional>
#include <list>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "ffrt.h"
#include "fault_data.h"
#include "appfreeze_info.h"

namespace OHOS {
namespace AppExecFwk {

class AppfreezeBinderCollector {
public:
    struct DumpTiming {
        int result = -1;
        int64_t latency = -1;
    };

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

    struct ProcessKillInfo {
        int64_t timestamp = 0;
        int64_t eventParamFirst = 0;
        int64_t eventParamSecond = 0;
        int64_t eventParamThird = 0;
        int64_t eventParamFourth = 0;
        int64_t eventParamFifth = 0;
        int64_t eventParamSixth = 0;
        int64_t eventParamSeventh = 0;
        int32_t callingPid;
        int32_t pid;
        int killId = -1;
        int adj = 0;
        bool foreground = false;
        std::string killReason;
        std::string killMsg;
        std::string callingProcessName;
        std::string processName;
    };

    AppfreezeBinderCollector() = default;
    ~AppfreezeBinderCollector() = default;

    void CollectFreezeSysMemory(std::string& memoryContent);
    std::string GetCatcherStack(const std::string& fileName, const std::string& catcherStack);
    bool IsHandleAppfreeze(const std::string& bundleName);
    FaultData GetFaultNotifyData(const FaultData& faultData, int pid);
    std::pair<std::string, std::string> CollectSyncBinderAttachment(const std::set<int>& syncPids,
        TerminalBinder& terminalBinder, int32_t pid, const std::function<bool()>& cancelled, std::string& binderInfo);
    __attribute__((visibility("hidden"))) std::string CollectBinderAttachment(const FaultData& faultData,
        int32_t pid, const std::function<bool()>& cancelled);
    bool IsAncoProc(int pid);
    std::string CatchASyncByPid(const std::set<int>& asyncPids, const std::set<int>& syncPids, int pid);
    std::string ParseDecToHex(uint64_t id);
    std::string GetHitraceInfo();
    std::string WriteToFile(const std::string& fileName, const std::string& content);
    std::map<int, std::list<PeerBinderInfo>> BinderParser(std::ifstream& fin, std::string& stack,
        std::set<int>& asyncPids) const;
    std::map<int, std::list<PeerBinderInfo>> BinderLineParser(std::ifstream& fin, std::string& stack,
        std::map<uint32_t, uint32_t>& asyncBinderMap,
        std::vector<std::pair<uint32_t, uint64_t>>& freeAsyncSpacePairs) const;
    std::vector<std::string> GetFileToList(std::string line) const;
    std::string StrSplit(const std::string& str, uint16_t index) const;
    std::set<int> GetBinderPeerPids(std::string& stack, ParseBinderParam params,
        std::set<int>& asyncPids, TerminalBinder& terminalBinder) const;
    void ParseBinderPids(const std::map<int, std::list<PeerBinderInfo>>& binderInfos,
        std::set<int>& pids, ParseBinderParam params, bool getTerminal,
        TerminalBinder& terminalBinder) const;
    void FindStackByPid(std::string& msg, int pid) const;
    std::pair<std::string, std::string> CatchJsonStacktrace(int pid, const std::string& faultType,
        DumpTiming* timing = nullptr, bool fresh = false) const;
    std::string CatcherStacktrace(int pid, DumpTiming* timing = nullptr) const;
    void DeleteStack(int pid);
    bool GetProcessKillReason(ProcessKillInfo& killInfo, int32_t killId, const std::string& killMsg);
    bool GetExitKernelReason(ProcessKillInfo& killInfo);
    void SetExitKernelReason(int32_t pid);
    int GetFreezeExitReason(const std::string& eventName);
    std::string GetFirstLine(const std::string& path);
    bool IsHalfTimeout(const std::string& faultType) const;

private:
    mutable ffrt::mutex catchStackMutex_;
    mutable std::map<int, std::string> catchStackMap_;
};

} // namespace AppExecFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_APPFREEZE_BINDER_COLLECTOR_H
