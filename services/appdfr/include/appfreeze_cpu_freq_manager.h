/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#ifndef OHOS_ABILITY_RUNTIME_APPFREEZE_CPU_FREQ_MANAGER_H
#define OHOS_ABILITY_RUNTIME_APPFREEZE_CPU_FREQ_MANAGER_H

#include <vector>
#include <mutex>
#include <map>

#include "cpu_data_processor.h"
#include "ffrt.h"
#include "singleton.h"

namespace OHOS {
namespace AppExecFwk {
typedef double (*GetThreadCpuLoad)(int);
class AppfreezeCpuFreqManager : public DelayedSingleton<AppfreezeCpuFreqManager>,
    public std::enable_shared_from_this<AppfreezeCpuFreqManager> {
        DISALLOW_COPY_AND_MOVE(AppfreezeCpuFreqManager);
public:
    AppfreezeCpuFreqManager();
    ~AppfreezeCpuFreqManager();
    
    static AppfreezeCpuFreqManager &GetInstance();
    bool InsertCpuDetailInfo(const std::string &type, int32_t pid);
    std::string GetCpuInfoPath(const std::string &type, const std::string &bundleName,
        int32_t uid, int32_t pid);

private:
    bool RemoveOldInfo();
    CpuDataProcessor GetCpuDetailInfo(int32_t pid);
    bool GetInfoByCpuCount(int32_t num, std::vector<CpuFreqData>& parseDatas, TotalTime& totalTime);
    void ParseCpuData(std::vector<std::vector<CpuFreqData>>& datas, std::vector<TotalTime>& totalTimeLists);
    std::string GetCpuStr(int code, std::vector<FrequencyPair>& freqPairs, float percentage);
    bool GetCpuTotalValue(size_t i, const std::vector<TotalTime>& totalTimeList,
        const std::vector<TotalTime>& blockTotalTimeList, TotalTime& totalTime);
    uint64_t GetProcessCpuTime(int32_t pid);
    uint64_t GetDeviceRuntime();
    uint64_t GetAppCpuTime(int32_t pid);
    GetThreadCpuLoad GetThreadCpuLoadFunc();
    double GetDimps();
    double GetOptimalCpuTime(int32_t pid);
    std::string GetTimeStampStr(uint64_t start);
    std::string GetStaticInfoHead();
    std::string GetConsumeTimeInfo(int32_t pid, CpuConsumeTime warnTimes, CpuConsumeTime blockTimes);
    std::string GetCpuInfoContent(const std::vector<TotalTime> &warnTotalTimeList,
        const std::vector<std::vector<CpuFreqData>> &warnCpuDetailInfo,
        const std::vector<TotalTime> &blockTotalTimeList,
        const std::vector<std::vector<CpuFreqData>> &blockCpuDetailInfo);
    bool IsContainHalfData(const std::string &key, CpuDataProcessor &cpuData, int32_t pid);
    std::string GetFreezeLogHead(const std::string &bundleName);
    uint64_t GetInterval(uint64_t warnTime, uint64_t blockTime);
    uint64_t ClockTicksToMs(uint64_t cpuTime);

    static ffrt::mutex freezeInfoMutex_;
    static int cpuCount_;
    static std::map<std::string, CpuDataProcessor> cpuInfoMap_;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_APPFREEZE_CPU_FREQ_MANAGER_H
