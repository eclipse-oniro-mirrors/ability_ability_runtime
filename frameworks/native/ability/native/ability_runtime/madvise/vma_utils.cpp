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

#include "vma_utils.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <cerrno>
#include <vector>
#include <unordered_set>
#include <string>

#include "hilog_tag_wrapper.h"
#include "securec.h"

namespace {
typedef int (*VmaCallback)(const OHOS::AbilityRuntime::VmaUtil::VMARegion* region, void* userdata);

constexpr int HEX_BASE = 16;
constexpr int DEC_BASE = 10;
constexpr int PERMS_INDEX_0 = 0;
constexpr int PERMS_INDEX_1 = 1;
constexpr int PERMS_INDEX_2 = 2;
constexpr int PERMS_INDEX_3 = 3;
constexpr int PERMS_LEN = 4;
constexpr size_t LINE_BUFFER_SIZE = 1024;

static constexpr uintptr_t HexToUintptr(const char* str, size_t len)
{
    uintptr_t value = 0;
    for (size_t i = 0; i < len; i++) {
        value <<= HEX_BASE;
        char c = str[i];
        if (c >= '0' && c <= '9') {
            value |= (c - '0');
        } else if (c >= 'a' && c <= 'f') {
            value |= (c - 'a' + DEC_BASE);
        } else if (c >= 'A' && c <= 'F') {
            value |= (c - 'A' + DEC_BASE);
        }
    }
    return value;
}

static constexpr unsigned long DecToUlong(const char* str)
{
    unsigned long value = 0;
    while (*str >= '0' && *str <= '9') {
        value = value * DEC_BASE + (*str - '0');
        str++;
    }
    return value;
}

static constexpr const char* SkipWhitespace(const char* ptr)
{
    while (*ptr == ' ' || *ptr == '\t') {
        ptr++;
    }
    return ptr;
}

static constexpr bool IsFilenameMatch(const char* path, const char* filename)
{
    if (!path || !filename) {
        return false;
    }
    if (strchr(filename, '/') != nullptr) {
        return strstr(path, filename) != nullptr;
    }
    const char* lastSlash = strrchr(path, '/');
    const char* basename = (lastSlash == nullptr) ? path : (lastSlash + 1);
    return strcmp(basename, filename) == 0;
}

static bool ParseMapsLine(const char* line, size_t lineLen, OHOS::AbilityRuntime::VmaUtil::VMARegion* region)
{
    const char* ptr = line;
    const char* dash = strchr(ptr, '-');
    if (!dash) {
        return false;
    }
    region->start = HexToUintptr(ptr, dash - ptr);
    ptr = dash + 1;
    const char* space = strchr(ptr, ' ');
    if (!space) {
        return false;
    }
    region->end = HexToUintptr(ptr, space - ptr);
    region->size = region->end - region->start;
    ptr = space + 1;
    ptr = SkipWhitespace(ptr);
    if (strlen(ptr) < PERMS_LEN) {
        return false;
    }
    if (ptr[PERMS_INDEX_0] == '\0' || ptr[PERMS_INDEX_1] == '\0' ||
        ptr[PERMS_INDEX_2] == '\0' || ptr[PERMS_INDEX_3] == '\0') {
        return false;
    }
    region->perms[PERMS_INDEX_0] = ptr[PERMS_INDEX_0];
    region->perms[PERMS_INDEX_1] = ptr[PERMS_INDEX_1];
    region->perms[PERMS_INDEX_2] = ptr[PERMS_INDEX_2];
    region->perms[PERMS_INDEX_3] = ptr[PERMS_INDEX_3];
    region->perms[OHOS::AbilityRuntime::VmaUtil::VMA_MAX_PERMS - 1] = '\0';
    ptr += PERMS_LEN;
    ptr = SkipWhitespace(ptr);
    ptr = strchr(ptr, ' ');
    if (!ptr) {
        return false;
    }
    ptr++;
    ptr = SkipWhitespace(ptr);
    ptr = strchr(ptr, ' ');
    if (!ptr) {
        return false;
    }
    ptr++;
    ptr = SkipWhitespace(ptr);
    region->inode = DecToUlong(ptr);
    ptr = strchr(ptr, ' ');
    if (!ptr) {
        return false;
    }
    ptr++;
    ptr = SkipWhitespace(ptr);
    if (*ptr != '\0' && *ptr != '\n') {
        errno_t err = strncpy_s(region->pathname, sizeof(region->pathname), ptr, sizeof(region->pathname) - 1);
        if (err != EOK) {
            region->pathname[0] = '\0';
        } else {
            char* newline = strchr(region->pathname, '\n');
            if (newline) {
                *newline = '\0';
            }
        }
    } else {
        region->pathname[0] = '\0';
    }
    return true;
}

static int IterateMapsInternal(const char* filename, VmaCallback callback, void* userdata)
{
    const char* mapsPath = "/proc/self/maps";
    FILE* fp = fopen(mapsPath, "r");
    if (!fp) {
        TAG_LOGE(OHOS::AAFwk::AAFwkLogTag::DEFAULT, "Failed to open %{public}s: %{public}d", mapsPath, errno);
        return -1;
    }
    int count = 0;
    char line[LINE_BUFFER_SIZE];
    while (fgets(line, sizeof(line), fp) != nullptr) {
        OHOS::AbilityRuntime::VmaUtil::VMARegion region;
        size_t lineLen = strlen(line);
        if (!ParseMapsLine(line, lineLen, &region)) {
            continue;
        }
        bool match = (strlen(filename) == 0) || IsFilenameMatch(region.pathname, filename);
        if (match) {
            count++;
            if (callback && callback(&region, userdata) != 0) {
                break;
            }
        }
    }
    if (fclose(fp) != 0) {
        TAG_LOGE(OHOS::AAFwk::AAFwkLogTag::DEFAULT, "Failed to close file: %{public}d", errno);
    }
    return count;
}
} // anonymous namespace

namespace OHOS {
namespace AbilityRuntime {
namespace VmaUtil {

std::vector<VMARegion> GetFileVmas(const char* filename)
{
    if (!filename || strlen(filename) == 0) {
        TAG_LOGE(AAFwkTag::ABILITY, "Invalid filename: null or empty");
        return {};
    }
    TAG_LOGI(AAFwkTag::UIABILITY, "GetFileVmas: searching for %{public}s", filename);
    std::vector<VMARegion> vmaList;
    auto collectCallback = [](const VMARegion* region, void* userdata) -> int {
        std::vector<VMARegion>* list = static_cast<std::vector<VMARegion>*>(userdata);
        list->push_back(*region);
        return 0;
    };
    IterateMapsInternal(filename, collectCallback, &vmaList);
    TAG_LOGI(AAFwkTag::UIABILITY, "GetFileVmas: found %{public}zu VMAs for %{public}s", vmaList.size(), filename);
    return vmaList;
}

struct MultiFileCallbackData {
    std::unordered_set<std::string> targetFilenames;
    std::vector<VMARegion>* vmaList;
};

std::vector<VMARegion> GetFileVmas(const std::vector<std::string>& filenames)
{
    if (filenames.empty()) {
        TAG_LOGE(AAFwkTag::ABILITY, "Filename list is empty");
        return {};
    }
    TAG_LOGI(AAFwkTag::UIABILITY, "GetFileVmas: searching for %{public}zu files", filenames.size());
    std::unordered_set<std::string> targetSet(filenames.begin(), filenames.end());
    std::vector<VMARegion> vmaList;
    auto multiFileCallback = [](const VMARegion* region, void* userdata) -> int {
        MultiFileCallbackData* data = static_cast<MultiFileCallbackData*>(userdata);
        const char* pathname = region->pathname;
        if (!pathname || strlen(pathname) == 0) {
            return 0;
        }
        const char* basename = strrchr(pathname, '/');
        basename = (basename == nullptr) ? pathname : (basename + 1);
        if (data->targetFilenames.find(basename) != data->targetFilenames.end()) {
            data->vmaList->push_back(*region);
        }
        return 0;
    };
    MultiFileCallbackData callbackData = {targetSet, &vmaList};
    IterateMapsInternal("", multiFileCallback, &callbackData);
    TAG_LOGI(AAFwkTag::UIABILITY, "GetFileVmas: found %{public}zu VMAs for %{public}zu files",
        vmaList.size(), filenames.size());
    return vmaList;
}

} // namespace VmaUtil
} // namespace AbilityRuntime
} // namespace OHOS
