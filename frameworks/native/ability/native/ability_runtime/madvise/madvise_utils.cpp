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

#include "hilog_tag_wrapper.h"
#include "json_utils.h"
#include "nlohmann/json.hpp"
#include "vma_utils.h"

namespace {
constexpr const char* MADVISE_CONFIG_DEFAULT_FILE_PATH = "/system/etc/madvise_config.json";
constexpr const char* MADVISE_CONFIG_FILE_PATH = "/etc/madvise_config.json";
constexpr int32_t PERMS_LEN = 4;

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
    std::string bundleName;
    std::vector<LibraryConfig> libraries;
};

struct MadviseConfig {
    std::vector<AppConfig> apps;

    bool IsValid() const
    {
        return !apps.empty();
    }

    const AppConfig* FindApp(const char* bundleName) const
    {
        if (!bundleName || strlen(bundleName) == 0) {
            return nullptr;
        }
        for (const auto& app : apps) {
            if (app.bundleName == bundleName) {
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
    nlohmann::json object;
    if (!OHOS::AAFwk::JsonUtils::GetInstance().LoadConfiguration(
        MADVISE_CONFIG_FILE_PATH, object, MADVISE_CONFIG_DEFAULT_FILE_PATH)) {
        TAG_LOGE(AAFwkTag::ABILITY, "load madvise config failed");
        return config;
    }
    if (!object.contains("apps") || !object.at("apps").is_array()) {
        TAG_LOGE(AAFwkTag::ABILITY, "parse apps invalid");
        return config;
    }

    for (auto &item : object.at("apps").items()) {
        const nlohmann::json& appJson = item.value();
        if (!appJson.contains("bundle_name") || !appJson.at("bundle_name").is_string()) {
            continue;
        }
        AppConfig appConfig;
        appConfig.bundleName = appJson.at("bundle_name").get<std::string>();
        if (appConfig.bundleName.empty()) {
            continue;
        }
        if (!appJson.contains("libraries") || !appJson.at("libraries").is_array()) {
            continue;
        }
        for (auto &libItem : appJson.at("libraries").items()) {
            const nlohmann::json& libJson = libItem.value();
            LibraryConfig libConfig;
            if (!libJson.contains("name") || !libJson.at("name").is_string()) {
                continue;
            }
            libConfig.name = libJson.at("name").get<std::string>();
            if (libConfig.name.empty()) {
                continue;
            }
            if (!libJson.contains("type") || !libJson.at("type").is_string()) {
                continue;
            }
            libConfig.type = ParseLibraryType(libJson.at("type").get<std::string>());
            if (libConfig.type == LibraryType::UNKNOWN) {
                continue;
            }
            appConfig.libraries.push_back(libConfig);
        }
        if (!appConfig.libraries.empty()) {
            config.apps.push_back(appConfig);
        }
    }
    TAG_LOGD(AAFwkTag::ABILITY, "Loaded madvise config: %{public}zu apps", config.apps.size());
    return config;
}

int32_t ApplyMadviseWithConfig(const char* bundleName, const MadviseConfig& config)
{
    if (!bundleName || strlen(bundleName) == 0) {
        TAG_LOGE(AAFwkTag::ABILITY, "Invalid bundle name: null or empty");
        return 0;
    }
    const AppConfig* appConfig = config.FindApp(bundleName);
    if (!appConfig) {
        TAG_LOGD(AAFwkTag::ABILITY, "No config found for bundle: %{public}s", bundleName);
        return 0;
    }
    TAG_LOGD(AAFwkTag::ABILITY, "Applying madvise for bundle: %{public}s, libraries: %{public}zu",
        bundleName, appConfig->libraries.size());
    int32_t successCount = 0;
    std::vector<std::string> vmaLibNames;
    for (const auto& libConfig : appConfig->libraries) {
        bool libSuccess = false;
        if (libConfig.type == LibraryType::ELF) {
            TAG_LOGD(AAFwkTag::ABILITY, "Applying ELF madvise for: %{public}s", libConfig.name.c_str());
            libSuccess = OHOS::AbilityRuntime::MadviseUtil::MadviseSingleLibrary(libConfig.name.c_str());
            if (libSuccess) {
                successCount++;
            }
        } else if (libConfig.type == LibraryType::VMA) {
            TAG_LOGD(AAFwkTag::ABILITY, "Adding VMA madvise for: %{public}s", libConfig.name.c_str());
            vmaLibNames.push_back(libConfig.name);
        } else {
            TAG_LOGW(AAFwkTag::ABILITY, "Unknown library type for: %{public}s", libConfig.name.c_str());
            continue;
        }
    }
    if (!vmaLibNames.empty()) {
        successCount += OHOS::AbilityRuntime::MadviseUtil::MadviseGeneralFiles(vmaLibNames);
    }
    TAG_LOGI(AAFwkTag::ABILITY,
        "Madvise completed for bundle %{public}s: %{public}d/%{public}zu libraries succeeded",
        bundleName, successCount, appConfig->libraries.size());
    return successCount;
}
}

struct MadviseData {
    int32_t successCount;
    int32_t failCount;
    const char* targetLibName;
};

static int32_t MadvisePhdrCallback(struct dl_phdr_info *info, size_t size, void *data)
{
    if (!info || !data || !info->dlpi_phdr) {
        return 0;
    }
    MadviseData* madviseData = static_cast<MadviseData*>(data);
    if (!madviseData->targetLibName || strlen(madviseData->targetLibName) == 0) {
        return 0;
    }
    const char* currentLibName = info->dlpi_name;
    if (!currentLibName || strlen(currentLibName) == 0) {
        return 0;
    }
    if (strstr(currentLibName, madviseData->targetLibName) == nullptr) {
        return 0;
    }
    TAG_LOGD(AAFwkTag::ABILITY, "Found target library: %{private}s", currentLibName);
    for (int32_t i = 0; i < info->dlpi_phnum; i++) {
        const ElfW(Phdr) *phdr = &info->dlpi_phdr[i];
        if (!phdr) {
            continue;
        }
        if (phdr->p_type != PT_LOAD) {
            continue;
        }
        bool isReadable = (phdr->p_flags & PF_R);
        bool isWritable = (phdr->p_flags & PF_W);
        if (!isReadable || isWritable) {
            continue;
        }
        uintptr_t startAddr = info->dlpi_addr + phdr->p_vaddr;
        size_t len = phdr->p_memsz;
        size_t pageSize = static_cast<size_t>(getpagesize());
        uintptr_t alignedStart = startAddr & ~(pageSize - 1);
        uintptr_t alignedEnd = (startAddr + len + pageSize - 1) & ~(pageSize - 1);
        size_t alignedLen = alignedEnd - alignedStart;
        TAG_LOGD(AAFwkTag::ABILITY, "madvise: lib=%{private}s, len=%{public}zu, alignedLen=%{public}zu",
            info->dlpi_name ? info->dlpi_name : "unknown", len, alignedLen);
        int32_t result = madvise(reinterpret_cast<void*>(alignedStart), alignedLen, MADV_DONTNEED);
        if (result == 0) {
            TAG_LOGD(AAFwkTag::ABILITY, "madvise success: len=%{public}zu", alignedLen);
            madviseData->successCount++;
        } else {
            TAG_LOGE(AAFwkTag::ABILITY, "madvise failed: len=%{public}zu, errno=%{public}d", alignedLen, errno);
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
    TAG_LOGD(AAFwkTag::ABILITY, "MadviseSingleLibrary called for library=%{public}s", libName);
    MadviseData madviseData = {0, 0, libName};
    dl_iterate_phdr(MadvisePhdrCallback, &madviseData);
    TAG_LOGD(AAFwkTag::ABILITY,
        "madvise completed for lib=%{public}s: successCount=%{public}d, failCount=%{public}d",
        libName, madviseData.successCount, madviseData.failCount);
    return (madviseData.successCount > 0);
}

static bool ApplyMadviseToRegion(const OHOS::AbilityRuntime::VmaUtil::VMARegion& region, const char* filename)
{
    if (strlen(region.perms) < PERMS_LEN) {
        TAG_LOGI(AAFwkTag::ABILITY, "Skipping invalid perms: %{public}s", region.perms);
        return false;
    }
    bool isReadable = (region.perms[0] == 'r');
    bool isWritable = (region.perms[1] == 'w');
    if (!isReadable || isWritable) {
        return false;
    }
    size_t pageSize = static_cast<size_t>(getpagesize());
    uintptr_t alignedStart = region.start & ~(pageSize - 1);
    uintptr_t alignedEnd = (region.end + pageSize - 1) & ~(pageSize - 1);
    size_t alignedLen = alignedEnd - alignedStart;
    TAG_LOGD(AAFwkTag::ABILITY,
        "madvise: file=%{private}s, perms=%{public}s, len=%{public}zu, alignedLen=%{public}zu",
        filename ? filename : "unknown", region.perms, region.size, alignedLen);
    int32_t result = madvise(reinterpret_cast<void*>(alignedStart), alignedLen, MADV_DONTNEED);
    if (result == 0) {
        TAG_LOGD(AAFwkTag::ABILITY, "madvise success: len=%{public}zu", alignedLen);
        return true;
    } else {
        TAG_LOGE(AAFwkTag::ABILITY, "madvise failed: len=%{public}zu, errno=%{public}d", alignedLen, errno);
        return false;
    }
}

static int32_t ProcessVmaRegions(const std::vector<OHOS::AbilityRuntime::VmaUtil::VMARegion>& regions)
{
    if (regions.empty()) {
        return 0;
    }
    int32_t successCount = 0;
    std::unordered_set<std::string> successPathNames;
    for (const auto& region : regions) {
        bool success = ApplyMadviseToRegion(region, region.pathname);
        if (success && successPathNames.find(region.pathname) == successPathNames.end()) {
            successCount++;
            successPathNames.insert(region.pathname);
        }
    }
    return successCount;
}

int32_t MadviseGeneralFiles(const std::vector<std::string>& filenames)
{
    if (filenames.empty()) {
        TAG_LOGE(AAFwkTag::ABILITY, "Filename list is empty");
        return 0;
    }
    TAG_LOGD(AAFwkTag::ABILITY, "MadviseGeneralFiles called for %{public}zu files", filenames.size());
    std::vector<AbilityRuntime::VmaUtil::VMARegion> regions = AbilityRuntime::VmaUtil::GetFileVmas(filenames);
    if (regions.empty()) {
        TAG_LOGE(AAFwkTag::ABILITY, "No VMA regions found for any of the %{public}zu files", filenames.size());
        return 0;
    }
    TAG_LOGD(AAFwkTag::ABILITY, "Found %{public}zu total VMA regions", regions.size());
    int32_t successCount = ProcessVmaRegions(regions);
    TAG_LOGD(AAFwkTag::ABILITY, "MadviseGeneralFiles completed: %{public}d segments optimized", successCount);
    return successCount;
}

int32_t MadviseWithConfigFile(const char* bundleName)
{
    if (!bundleName || strlen(bundleName) == 0) {
        TAG_LOGE(AAFwkTag::ABILITY, "Invalid bundle name: null or empty");
        return -1;
    }
    MadviseConfig config = LoadConfigFromFile();
    if (!config.IsValid()) {
        TAG_LOGE(AAFwkTag::ABILITY, "Failed to load valid config");
        return -1;
    }
    return ApplyMadviseWithConfig(bundleName, config);
}
} // namespace MadviseUtil
} // namespace AbilityRuntime
} // namespace OHOS
