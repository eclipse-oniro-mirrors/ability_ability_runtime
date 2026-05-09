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
#ifndef OHOS_ABILITY_RUNTIME_APPFREEZE_UTIL_H
#define OHOS_ABILITY_RUNTIME_APPFREEZE_UTIL_H

#include <sys/types.h>
#include <string>

namespace OHOS {
namespace AppExecFwk {
class AppfreezeUtil {
public:
    static constexpr const char* const PROC_STAT_PATH = "/proc/stat";
    static constexpr const char* const LOG_FILE_PATH = "/data/log/eventlog/freeze";
    static constexpr const char* const EVENTLOG_PATH = "/data/log/eventlog";
    static constexpr int64_t SEC_TO_MILLISEC = 1000;
    static constexpr int32_t CPU_COUNT_SUBTRACT = 1;
    static constexpr int TIME_LIMIT = 10000; // 10s
    static constexpr const char* const KEY_SEPARATOR = "-";
    static constexpr size_t MAX_MAP_SIZE = 10;

    AppfreezeUtil();
    ~AppfreezeUtil();

    static std::string CreateFile(const std::string& dirPath, const std::string& fileName);
    static std::string TimestampFormatToDate(time_t timeStamp, const std::string& format);
    static uint64_t GetMilliseconds();
    static std::string RoundToTwoDecimals(float value);
    static int GetCpuCount();
    static std::string FreezePathToRealPath(const std::string& filePath);
    static int32_t GetUidByPid(const int32_t pid);
    static std::string GetProcessNameByPid(const int32_t pid);
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_APPFREEZE_UTIL_H
