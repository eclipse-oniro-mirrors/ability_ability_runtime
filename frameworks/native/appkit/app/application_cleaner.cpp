/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "application_cleaner.h"

#include <cstring>
#include <dirent.h>
#include <pthread.h>
#include <sstream>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "directory_ex.h"
#include "ffrt.h"
#include "hilog_tag_wrapper.h"
#include "os_account_manager_wrapper.h"
namespace OHOS {
namespace AppExecFwk {
namespace {
static const std::string MARK_SYMBOL{ "_useless" };
static const std::string PATH_SEPARATOR = { "/" };
static const char FILE_SEPARATOR_CHAR = '/';
static const std::string MARK_TEMP_DIR{ "temp_useless" };
constexpr const char* TEMP_DIR_NAME = "/temp";

static const size_t MARK_TEMP_LEN = 12;
static const int PATH_MAX_SIZE = 256;

static const int RESULT_OK = 0;
static const int RESULT_ERR = -1;

static const char TASK_NAME[] = "ApplicationCleaner::ClearTempData";
static constexpr uint64_t DELAY = 5000000; //5s
constexpr int64_t MAX_FILE_SIZE = 50 * 1024;
constexpr int32_t MAX_CPU_INDEX = 4;

void SetCurrentThreadAffinity()
{
    cpu_set_t cpuSet;
    CPU_ZERO(&cpuSet);
    for (int32_t i = 0; i < MAX_CPU_INDEX; i++) {
        CPU_SET(i, &cpuSet);
    }

    if (pthread_setaffinity_np(pthread_self(), sizeof(cpuSet), &cpuSet) != 0) {
        TAG_LOGW(AAFwkTag::APPKIT, "set ThreadAffinity failed errno %{public}d", errno);
    }
}
} // namespace
void ApplicationCleaner::RenameTempData()
{
    if (context_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null context");
        return;
    }
    std::vector<std::string> tempBases;
    context_->GetAllTempBase(tempBases);
    int64_t now =
        std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch())
            .count();
    std::ostringstream stream;
    stream << std::hex << now;
    for (const auto &baseDir : tempBases) {
        auto path = baseDir + TEMP_DIR_NAME;
        if (access(path.c_str(), F_OK) != 0) {
            continue;
        }
        auto newPath = path + MARK_SYMBOL + stream.str();
        if (rename(path.c_str(), newPath.c_str()) != 0) {
            TAG_LOGE(AAFwkTag::APPKIT, "msg: %{public}s", strerror(errno));
        }
    }
}

void ApplicationCleaner::ClearTempData()
{
    if (hasCleaned_) {
        return;
    }
    hasCleaned_ = true;
    TAG_LOGI(AAFwkTag::APPKIT, "ClearTempData");
    std::vector<std::string> rootDir;
    if (GetRootPath(rootDir) != RESULT_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "Get root dir error");
        return;
    }
    auto weakThis = weak_from_this();
    auto cleanTemp = [weakThis, rootDir]() {
        auto sharedThis = weakThis.lock();
        if (sharedThis == nullptr || sharedThis->context_ == nullptr) {
            TAG_LOGE(AAFwkTag::APPKIT, "Invalid shared pointer");
            return;
        }
        SetCurrentThreadAffinity();
        std::vector<std::string> temps;
        if (sharedThis->GetObsoleteBundleTempPath(rootDir, temps) != RESULT_OK) {
            TAG_LOGE(AAFwkTag::APPKIT, "get bundle temp file list false");
            return;
        }

        for (const auto &temp : temps) {
            if (!sharedThis->RemoveDir(temp)) {
                TAG_LOGW(AAFwkTag::APPKIT, "path: %{private}s", temp.c_str());
            }
        }
    };

    ffrt::task_attr attr;
    attr.name(TASK_NAME);
    attr.qos(ffrt_qos_background);
    if (!CheckFileSize(rootDir)) {
        attr.delay(DELAY);
    }
    ffrt::submit(std::move(cleanTemp), attr);
}

bool ApplicationCleaner::CheckFileSize(const std::vector<std::string> &bundlePath)
{
    int64_t fileSize = 0;

    for (const auto& dir : bundlePath) {
        struct stat fileInfo = { 0 };
        if (stat(dir.c_str(), &fileInfo) != 0) {
            continue;
        }
        fileSize += fileInfo.st_size;
    }
    return (fileSize <= MAX_FILE_SIZE);
}

int ApplicationCleaner::GetRootPath(std::vector<std::string> &rootPath)
{
    if (context_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null context");
        return RESULT_ERR;
    }
    std::vector<std::string> baseDirs;
    context_->GetAllTempBase(baseDirs);
    for (const auto &baseDir : baseDirs) {
        if (access(baseDir.c_str(), F_OK) != 0) {
            continue;
        }
        rootPath.push_back(baseDir);
    }

    return RESULT_OK;
}

ErrCode ApplicationCleaner::GetObsoleteBundleTempPath(
    const std::vector<std::string> &rootPath, std::vector<std::string> &tempPath)
{
    if (rootPath.empty()) {
        TAG_LOGE(AAFwkTag::APPKIT, "empty rootPath");
        return RESULT_ERR;
    }

    for (const auto &dir : rootPath) {
        if (dir.empty()) {
            TAG_LOGE(AAFwkTag::APPKIT, "empty dir");
            continue;
        }
        std::vector<std::string> temp;
        TraverseObsoleteTempDirectory(dir, temp);
        std::copy(temp.begin(), temp.end(), std::back_inserter(tempPath));
    }
    return RESULT_OK;
}

void ApplicationCleaner::TraverseObsoleteTempDirectory(
    const std::string &currentPath, std::vector<std::string> &tempDirs)
{
    if (currentPath.empty() || (currentPath.size() > PATH_MAX_SIZE)) {
        TAG_LOGE(AAFwkTag::APPKIT, "traverse temp directory current path invalid");
        return;
    }

    std::string filePath = currentPath;
    DIR *dir = opendir(filePath.c_str());
    if (dir == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null dir %{public}s", currentPath.c_str());
        return;
    }
    if (filePath.back() != FILE_SEPARATOR_CHAR) {
        filePath.push_back(FILE_SEPARATOR_CHAR);
    }
    struct dirent *ptr = nullptr;
    while ((ptr = readdir(dir)) != nullptr) {
        if (strcmp(ptr->d_name, ".") == 0 || strcmp(ptr->d_name, "..") == 0) {
            continue;
        }
        if (ptr->d_type == DT_DIR && strncmp(ptr->d_name, MARK_TEMP_DIR.c_str(), MARK_TEMP_LEN) == 0) {
            std::string tempDir = filePath + std::string(ptr->d_name);
            tempDirs.emplace_back(tempDir);
            continue;
        }
        if (ptr->d_type == DT_DIR) {
            std::string currentDir = filePath + std::string(ptr->d_name);
            TraverseObsoleteTempDirectory(currentDir, tempDirs);
        }
    }
    closedir(dir);
}

bool ApplicationCleaner::RemoveDir(const std::string &tempPath)
{
    TAG_LOGD(AAFwkTag::APPKIT, "Called");
    if (tempPath.empty()) {
        return false;
    }
    struct stat buf = {};
    if (stat(tempPath.c_str(), &buf) != 0) {
        TAG_LOGW(AAFwkTag::APPKIT, "obtain file properties failed");
        return false;
    }

    if (S_ISREG(buf.st_mode)) {
        return OHOS::RemoveFile(tempPath);
    }

    if (S_ISDIR(buf.st_mode)) {
        return OHOS::ForceRemoveDirectory(tempPath);
    }

    return false;
}

} // namespace AppExecFwk
} // namespace OHOS
