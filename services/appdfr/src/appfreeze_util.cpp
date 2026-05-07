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
#include "appfreeze_util.h"

#include <chrono>
#include <fcntl.h>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>

#include "directory_ex.h"
#include "file_ex.h"
#include "string_ex.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
    constexpr int64_t MAX_TIME_BUFF = 64;
    constexpr mode_t DEFAULT_LOG_DIR_MODE = 0770;
    constexpr mode_t DEFAULT_LOG_FILE_MODE = 0644;
    constexpr uint32_t TWO_DECIMALS = 2;
    constexpr size_t CPU_INFO_SIZE = 11;
}

AppfreezeUtil::AppfreezeUtil()
{
}

AppfreezeUtil::~AppfreezeUtil()
{
}

std::string AppfreezeUtil::CreateFile(const std::string &dirPath, const std::string &fileName)
{
    if (!OHOS::FileExists(dirPath)) {
        OHOS::ForceCreateDirectory(dirPath);
        OHOS::ChangeModeDirectory(dirPath, DEFAULT_LOG_DIR_MODE);
    }
    std::string filePath = dirPath + "/" + fileName;
    FILE* fp = fopen(filePath.c_str(), "w+");
    chmod(filePath.c_str(), DEFAULT_LOG_FILE_MODE);
    if (fp == nullptr) {
        TAG_LOGW(AAFwkTag::APPDFR, "filePath create failed, errno: %{public}d", errno);
        return "";
    } else {
        TAG_LOGI(AAFwkTag::APPDFR, "filePath: %{public}s", filePath.c_str());
    }
    (void)fclose(fp);
    return filePath;
}

std::string AppfreezeUtil::TimestampFormatToDate(time_t timeStamp, const std::string &format)
{
    char date[MAX_TIME_BUFF] = {0};
    struct tm result {};
    if (localtime_r(&timeStamp, &result) != nullptr) {
        strftime(date, MAX_TIME_BUFF, format.c_str(), &result);
    }
    return std::string(date);
}

uint64_t AppfreezeUtil::GetMilliseconds()
{
    auto now = std::chrono::system_clock::now();
    auto millisecs = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
    return millisecs.count();
}

std::string AppfreezeUtil::RoundToTwoDecimals(float value)
{
    std::stringstream ss;
    ss<< std::fixed << std::setprecision(TWO_DECIMALS) << value;
    return ss.str();
}

std::string AppfreezeUtil::FreezePathToRealPath(const std::string& filePath)
{
    std::string realPath;
    if (!OHOS::PathToRealPath(filePath, realPath)) {
        TAG_LOGE(AAFwkTag::APPDFR, "pathToRealPath failed, file:%{public}s, errno:%{public}d",
            filePath.c_str(), errno);
        return "";
    }
    return realPath;
}

int AppfreezeUtil::GetCpuCount()
{
    std::string procStatPath = PROC_STAT_PATH;
    std::string content;
    if (!LoadStringFromFile(procStatPath, content) || content.empty()) {
        TAG_LOGW(AAFwkTag::APPDFR, "failed to read path:%{public}s, errno:%{public}d",
            procStatPath.c_str(), errno);
        return 0;
    }
    int cpuCount = 0;
    std::istringstream iss(content);
    std::string line;
    while (std::getline(iss, line)) {
        if (line.empty()) {
            continue;
        }
        std::vector<std::string> splitStrs;
        SplitStr(line, " ", splitStrs);
        if (splitStrs.size() != CPU_INFO_SIZE) {
            break;
        }
        if (splitStrs[0].find("cpu") != 0) {
            TAG_LOGW(AAFwkTag::APPDFR, "not find cpu prefix, head: %{public}s.", splitStrs[0].c_str());
            break;
        }
        cpuCount++;
    }
    cpuCount -= CPU_COUNT_SUBTRACT;
    TAG_LOGD(AAFwkTag::APPDFR, "read: %{public}s to get cpu count:%{public}d.", procStatPath.c_str(), cpuCount);
    return cpuCount;
}

int32_t AppfreezeUtil::GetUidByPid(const int32_t pid)
{
    std::string pidStatusPath = "/proc/" + std::to_string(pid) + "/status";
    std::string realPath = FreezePathToRealPath(pidStatusPath);
    std::string content;
    int32_t uid = -1;
    if (realPath.empty() || !LoadStringFromFile(realPath, content) || content.empty()) {
        TAG_LOGW(AAFwkTag::APPDFR, "failed to read path:%{public}s, errno:%{public}d",
            realPath.c_str(), errno);
        return uid;
    }
    std::istringstream iss(content);
    std::string uidFlag = "Uid:";
    std::string infoLine;
    while (std::getline(iss, infoLine)) {
        if (infoLine.compare(0, uidFlag.size(), uidFlag) == 0) {
            std::istringstream infoStream(infoLine);
            if (std::getline(infoStream, infoLine, ':') && std::getline(infoStream, infoLine)) {
                std::stringstream(infoLine) >> uid;
                TAG_LOGI(AAFwkTag::APPDFR, "uid of pid %{public}" PRId32 " is %{public}" PRId32 ".", pid, uid);
                break;
            }
        }
    }
    return uid;
}

std::string AppfreezeUtil::GetProcessNameByPid(const int32_t pid)
{
    std::string commPath = "/proc/" + std::to_string(pid) + "/comm";
    std::string realPath = FreezePathToRealPath(commPath);
    std::string processName;
    if (realPath.empty() || !LoadStringFromFile(realPath, processName)) {
        TAG_LOGW(AAFwkTag::APPDFR, "failed to read path:%{public}s, errno:%{public}d",
            realPath.c_str(), errno);
        return "";
    }
    if (!processName.empty() && processName.back() == '\n') {
        processName.pop_back();
    }
    return processName;
}
}  // namespace AppExecFwk
}  // namespace OHOS
