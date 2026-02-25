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

#include "madvise_utils.h"

#include "cJSON.h"
#include <algorithm>
#include <chrono>
#include <libgen.h>
#include <cstring>
#include <dlfcn.h>
#include <link.h>
#include <sys/mman.h>
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <unordered_set>
#include <string>
#include <vector>

#include "config_policy_utils.h"
#include "hilog_tag_wrapper.h"
#include "vma_utils.h"

namespace {
constexpr const char* MADVISE_CONFIG_DEFAULT_FILE_PATH = "/system/etc/madvise_config.json";
constexpr const char* MADVISE_CONFIG_FILE_PATH = "/etc/madvise_config.json";
constexpr int PERMS_INDEX_0 = 0;
constexpr int PERMS_INDEX_1 = 1;
constexpr int PERMS_INDEX_2 = 2;
constexpr int PERMS_INDEX_3 = 3;

enum class LibraryType {
    UNKNOWN = 0,
    ELF,
    VMA
};

struct LibraryConfig {
    std::string name;
    LibraryType type;
    std::vector<unsigned long> hotPageOffsets;
};

struct AppConfig {
    std::string packageName;
    std::vector<LibraryConfig> libraries;
};

struct MadviseConfig {
    std::vector<AppConfig> apps;
    bool IsValid() const
    {
        return !apps.empty();
    }
    const AppConfig* FindApp(const char* packageName) const
    {
        if (!packageName || strlen(packageName) == 0) {
            return nullptr;
        }
        for (const auto& app : apps) {
            if (app.packageName == packageName) {
                return &app;
            }
        }
        return nullptr;
    }
};

LibraryType ParseLibraryType(const std::string& typeStr)
{
    if (typeStr == "elf") {
        return LibraryType::ELF;
    } else if (typeStr == "vma") {
        return LibraryType::VMA;
    }
    return LibraryType::UNKNOWN;
}

MadviseConfig LoadConfigFromFile()
{
    MadviseConfig config;
    std::string configPath = OHOS::AbilityRuntime::MadviseUtil::GetConfigPath();
    FILE* file = fopen(configPath.c_str(), "r");
    if (!file) {
        TAG_LOGE(AAFwkTag::ABILITY, "Failed to open config file: %{public}s", configPath.c_str());
        return config;
    }
    if (fseek(file, 0, SEEK_END) != 0) {
        TAG_LOGE(AAFwkTag::ABILITY, "Failed to seek file end: %{public}s", configPath.c_str());
        if (fclose(file) != 0) {
            TAG_LOGE(AAFwkTag::ABILITY, "Failed to close file");
        }
        return config;
    }
    long fileSize = ftell(file);
    if (fseek(file, 0, SEEK_SET) != 0) {
        TAG_LOGE(AAFwkTag::ABILITY, "Failed to seek file begin: %{public}s", configPath.c_str());
        if (fclose(file) != 0) {
            TAG_LOGE(AAFwkTag::ABILITY, "Failed to close file");
        }
        return config;
    }
    if (fileSize <= 0) {
        TAG_LOGE(AAFwkTag::ABILITY, "Config file is empty: %{public}s", configPath.c_str());
        if (fclose(file) != 0) {
            TAG_LOGE(AAFwkTag::ABILITY, "Failed to close file");
        }
        return config;
    }
    std::string content;
    content.resize(fileSize);
    size_t readSize = fread(&content[0], 1, fileSize, file);
    if (fclose(file) != 0) {
        TAG_LOGE(AAFwkTag::ABILITY, "Failed to close file");
    }
    if (readSize != static_cast<size_t>(fileSize)) {
        TAG_LOGE(AAFwkTag::ABILITY, "Failed to read config file: %{public}s", configPath.c_str());
        return config;
    }
    cJSON* root = cJSON_Parse(content.c_str());
    if (!root) {
        TAG_LOGE(AAFwkTag::ABILITY, "Failed to parse JSON config: %{public}s", configPath.c_str());
        return config;
    }
    cJSON* appsArray = cJSON_GetObjectItemCaseSensitive(root, "apps");
    if (!cJSON_IsArray(appsArray)) {
        TAG_LOGE(AAFwkTag::ABILITY, "Invalid config format: 'apps' not found or not an array");
        cJSON_Delete(root);
        return config;
    }
    cJSON* appItem = nullptr;
    cJSON_ArrayForEach(appItem, appsArray) {
        AppConfig appConfig;
        cJSON* packageNameItem = cJSON_GetObjectItemCaseSensitive(appItem, "package_name");
        if (cJSON_IsString(packageNameItem) && packageNameItem->valuestring) {
            appConfig.packageName = packageNameItem->valuestring;
        }
        cJSON* libsArray = cJSON_GetObjectItemCaseSensitive(appItem, "libraries");
        if (!cJSON_IsArray(libsArray)) {
            continue;
        }
        cJSON* libItem = nullptr;
        cJSON_ArrayForEach(libItem, libsArray) {
            LibraryConfig libConfig;
            cJSON* nameItem = cJSON_GetObjectItemCaseSensitive(libItem, "name");
            if (cJSON_IsString(nameItem) && nameItem->valuestring) {
                libConfig.name = nameItem->valuestring;
            }
            cJSON* typeItem = cJSON_GetObjectItemCaseSensitive(libItem, "type");
            if (cJSON_IsString(typeItem) && typeItem->valuestring) {
                libConfig.type = ParseLibraryType(typeItem->valuestring);
            }
            if (!libConfig.name.empty() && libConfig.type != LibraryType::UNKNOWN) {
                appConfig.libraries.push_back(libConfig);
            }
        }
        if (!appConfig.packageName.empty() && !appConfig.libraries.empty()) {
            config.apps.push_back(appConfig);
        }
    }
    cJSON_Delete(root);
    TAG_LOGI(AAFwkTag::UIABILITY, "Loaded madvise config from %{public}s: %{public}zu apps",
        configPath.c_str(), config.apps.size());
    return config;
}

int ApplyMadviseWithConfig(const char* packageName, const MadviseConfig& config)
{
    if (!packageName || strlen(packageName) == 0) {
        TAG_LOGE(AAFwkTag::ABILITY, "Invalid package name: null or empty");
        return 0;
    }
    const AppConfig* appConfig = config.FindApp(packageName);
    if (!appConfig) {
        TAG_LOGI(AAFwkTag::UIABILITY, "No config found for package: %{public}s", packageName);
        return 0;
    }
    TAG_LOGI(AAFwkTag::UIABILITY, "Applying madvise for package: %{public}s, libraries: %{public}zu",
        packageName, appConfig->libraries.size());
    int successCount = 0;
    for (const auto& libConfig : appConfig->libraries) {
        bool libSuccess = false;
        if (libConfig.type == LibraryType::ELF) {
            TAG_LOGI(AAFwkTag::UIABILITY, "Applying ELF madvise for: %{public}s", libConfig.name.c_str());
            libSuccess = OHOS::AbilityRuntime::MadviseUtil::MadviseSingleLibrary(libConfig.name.c_str());
        } else if (libConfig.type == LibraryType::VMA) {
            TAG_LOGI(AAFwkTag::UIABILITY, "Applying VMA madvise for: %{public}s", libConfig.name.c_str());
            libSuccess = OHOS::AbilityRuntime::MadviseUtil::MadviseGeneralFile(libConfig.name.c_str());
        } else {
            TAG_LOGW(AAFwkTag::ABILITY, "Unknown library type for: %{public}s", libConfig.name.c_str());
            continue;
        }
        if (libSuccess) {
            successCount++;
        }
    }
    TAG_LOGI(AAFwkTag::UIABILITY,
        "Madvise completed for package %{public}s: %{public}d/%{public}zu libraries succeeded",
        packageName, successCount, appConfig->libraries.size());
    return successCount;
}

} // anonymous namespace

struct MadviseData {
    int successCount;
    int failCount;
    const char* targetLibName;
};

static int MadvisePhdrCallback(struct dl_phdr_info *info, size_t size, void *data)
{
    MadviseData* madviseData = static_cast<MadviseData*>(data);
    const char* currentLibName = info->dlpi_name;
    if (!currentLibName || strlen(currentLibName) == 0) {
        currentLibName = "unknown";
    }
    if (strstr(currentLibName, madviseData->targetLibName) == nullptr) {
        return 0;
    }
    TAG_LOGI(AAFwkTag::UIABILITY, "Found target library: %{public}s", currentLibName);
    for (int i = 0; i < info->dlpi_phnum; i++) {
        const ElfW(Phdr) *phdr = &info->dlpi_phdr[i];
        if (phdr->p_type != PT_LOAD) {
            continue;
        }
        bool isReadable = (phdr->p_flags & PF_R);
        bool isWritable = (phdr->p_flags & PF_W);
        if (!isReadable || isWritable) {
            continue;
        }
        void* startAddr = reinterpret_cast<void*>(info->dlpi_addr + phdr->p_vaddr);
        size_t len = phdr->p_memsz;
        size_t pageSize = getpagesize();
        void* alignedStart = reinterpret_cast<void*>(
            reinterpret_cast<uintptr_t>(startAddr) & ~(pageSize - 1));
        size_t alignedLen = (reinterpret_cast<uintptr_t>(startAddr) + len + pageSize - 1) & ~(pageSize - 1);
        alignedLen -= reinterpret_cast<uintptr_t>(alignedStart);
        char perms[4] = "---";
        if (phdr->p_flags & PF_R) {
            perms[PERMS_INDEX_0] = 'r';
        }
        if (phdr->p_flags & PF_W) {
            perms[PERMS_INDEX_1] = 'w';
        }
        if (phdr->p_flags & PF_X) {
            perms[PERMS_INDEX_2] = 'x';
        }
        perms[PERMS_INDEX_3] = '\0';
        TAG_LOGI(AAFwkTag::UIABILITY,
            "madvise: lib=%{public}s, perms=%{public}s, addr=%{public}p, len=%{public}zu, alignedLen=%{public}zu",
            info->dlpi_name ? info->dlpi_name : "unknown", perms, startAddr, len, alignedLen);
        int result = madvise(alignedStart, alignedLen, MADV_DONTNEED);
        if (result == 0) {
            TAG_LOGI(AAFwkTag::UIABILITY,
                "madvise success: addr=%{public}p, len=%{public}zu", alignedStart, alignedLen);
            madviseData->successCount++;
        } else {
            TAG_LOGE(AAFwkTag::ABILITY, "madvise failed: addr=%{public}p, len=%{public}zu, errno=%{public}d",
                alignedStart, alignedLen, errno);
            madviseData->failCount++;
        }
    }
    return 0;
}

namespace OHOS {
namespace AbilityRuntime {
namespace MadviseUtil {

bool MadviseSingleLibrary(const char* libName)
{
    if (!libName || strlen(libName) == 0) {
        TAG_LOGE(AAFwkTag::ABILITY, "Invalid library name: empty string");
        return false;
    }
    TAG_LOGI(AAFwkTag::UIABILITY, "MadviseSingleLibrary called for library=%{public}s", libName);
    MadviseData madviseData = {0, 0, libName};
    dl_iterate_phdr(MadvisePhdrCallback, &madviseData);
    TAG_LOGI(AAFwkTag::UIABILITY,
        "madvise completed for lib=%{public}s: successCount=%{public}d, failCount=%{public}d",
        libName, madviseData.successCount, madviseData.failCount);
    return (madviseData.successCount > 0);
}

static int ApplyMadviseToRegion(const OHOS::AbilityRuntime::VmaUtil::VMARegion& region, size_t pageSize,
    const char* filename)
{
    bool isReadonly = (strstr(region.perms, "r--") != nullptr);
    bool isExecutable = (strstr(region.perms, "r-x") != nullptr);
    if (!isReadonly && !isExecutable) {
        return 0;
    }
    void* startAddr = reinterpret_cast<void*>(region.start);
    void* alignedStart = reinterpret_cast<void*>(
        reinterpret_cast<uintptr_t>(startAddr) & ~(pageSize - 1));
    void* endAddr = reinterpret_cast<void*>(region.end);
    size_t alignedLen = (reinterpret_cast<uintptr_t>(endAddr) + pageSize - 1) & ~(pageSize - 1);
    alignedLen -= reinterpret_cast<uintptr_t>(alignedStart);
    TAG_LOGI(AAFwkTag::UIABILITY,
        "madvise: file=%{public}s, perms=%{public}s, addr=%{public}p, len=%{public}zu, alignedLen=%{public}zu",
        filename ? filename : "unknown", region.perms, startAddr, region.size, alignedLen);
    int result = madvise(alignedStart, alignedLen, MADV_DONTNEED);
    if (result == 0) {
        TAG_LOGI(AAFwkTag::UIABILITY, "madvise success: addr=%{public}p, len=%{public}zu", alignedStart, alignedLen);
        return 1;
    } else {
        TAG_LOGE(AAFwkTag::ABILITY, "madvise failed: addr=%{public}p, len=%{public}zu, errno=%{public}d",
            alignedStart, alignedLen, errno);
        return 0;
    }
}

static int ProcessVmaRegions(const std::vector<OHOS::AbilityRuntime::VmaUtil::VMARegion>& regions)
{
    if (regions.empty()) {
        return 0;
    }
    int successCount = 0;
    size_t pageSize = getpagesize();
    for (const auto& region : regions) {
        successCount += ApplyMadviseToRegion(region, pageSize, region.pathname);
    }
    return successCount;
}

bool MadviseGeneralFile(const char* filename)
{
    if (!filename || strlen(filename) == 0) {
        TAG_LOGE(AAFwkTag::ABILITY, "Invalid filename: empty string");
        return false;
    }
    TAG_LOGI(AAFwkTag::UIABILITY, "MadviseGeneralFile called for file=%{public}s", filename);
    std::vector<AbilityRuntime::VmaUtil::VMARegion> regions = AbilityRuntime::VmaUtil::GetFileVmas(filename);
    if (regions.empty()) {
        TAG_LOGE(AAFwkTag::ABILITY, "No VMA regions found for %{public}s", filename);
        return false;
    }
    int successCount = ProcessVmaRegions(regions);
    TAG_LOGI(AAFwkTag::UIABILITY,
        "MadviseGeneralFile completed for %{public}s: successCount=%{public}d", filename, successCount);
    return (successCount > 0);
}

int MadviseGeneralFiles(const std::vector<std::string>& filenames)
{
    if (filenames.empty()) {
        TAG_LOGE(AAFwkTag::ABILITY, "Filename list is empty");
        return 0;
    }
    TAG_LOGI(AAFwkTag::UIABILITY, "MadviseGeneralFiles called for %{public}zu files", filenames.size());
    std::vector<AbilityRuntime::VmaUtil::VMARegion> regions = AbilityRuntime::VmaUtil::GetFileVmas(filenames);
    if (regions.empty()) {
        TAG_LOGE(AAFwkTag::ABILITY, "No VMA regions found for any of the %{public}zu files", filenames.size());
        return 0;
    }
    TAG_LOGI(AAFwkTag::UIABILITY, "Found %{public}zu total VMA regions", regions.size());
    int successCount = ProcessVmaRegions(regions);
    TAG_LOGI(AAFwkTag::UIABILITY, "MadviseGeneralFiles completed: %{public}d segments optimized", successCount);
    return successCount;
}

int MadviseWithConfigFile(const char* packageName)
{
    if (!packageName || strlen(packageName) == 0) {
        TAG_LOGE(AAFwkTag::ABILITY, "Invalid package name: null or empty");
        return -1;
    }
    MadviseConfig config = LoadConfigFromFile();
    if (!config.IsValid()) {
        TAG_LOGE(AAFwkTag::ABILITY, "Failed to load valid config from: %{public}s", MADVISE_CONFIG_FILE_PATH);
        return -1;
    }
    return ApplyMadviseWithConfig(packageName, config);
}

std::string GetConfigPath()
{
    char buf[MAX_PATH_LEN] = { 0 };
    char *configPath = GetOneCfgFile(MADVISE_CONFIG_FILE_PATH, buf, MAX_PATH_LEN);
    if (configPath == nullptr || configPath[0] == '\0' || strlen(configPath) > MAX_PATH_LEN) {
        return MADVISE_CONFIG_DEFAULT_FILE_PATH;
    }
    return configPath;
}
} // namespace MadviseUtil
} // namespace AbilityRuntime
} // namespace OHOS
