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
constexpr int32_t HEX_BASE = 16;

constexpr int32_t PERMS_INDEX_0 = 0;
constexpr int32_t PERMS_INDEX_1 = 1;
constexpr int32_t PERMS_INDEX_2 = 2;
constexpr int32_t PERMS_INDEX_3 = 3;

constexpr size_t LINE_BUFFER_SIZE = 1024;

static uintptr_t HexToUintptr(const char* str, size_t len)
{
    if (!str || len == 0) {
        return 0;
    }
    char buf[32];
    if (len >= sizeof(buf)) {
        return 0;
    }
    if (memcpy_s(buf, sizeof(buf), str, len) != EOK) {
        return 0;
    }
    buf[len] = '\0';
    char* endptr = nullptr;
    errno = 0;
    auto result = strtoull(buf, &endptr, HEX_BASE);
    if (errno != 0 || endptr == buf || *endptr != '\0') {
        TAG_LOGE(OHOS::AAFwk::AAFwkLogTag::DEFAULT, "hex to int failed: %{public}d", errno);
        return 0;
    }
    return static_cast<uintptr_t>(result);
}

static constexpr const char* SkipWhitespace(const char* ptr)
{
    if (!ptr) {
        return nullptr;
    }
    while (*ptr == ' ' || *ptr == '\t') {
        ptr++;
    }
    return ptr;
}

static bool ParseMapsLine(const char* line, size_t lineLen, OHOS::AbilityRuntime::VmaUtil::VMARegion* region)
{
    // parse address
    const char* ptr = line;
    const char* dash = strchr(ptr, '-');
    if (!dash) {
        return false;
    }
    region->start = HexToUintptr(ptr, dash - ptr);
    if (region->start == 0) {
        return false;
    }
    ptr = dash + 1;
    const char* space = strchr(ptr, ' ');
    if (!space) {
        return false;
    }
    region->end = HexToUintptr(ptr, space - ptr);
    if (region->end == 0 || region->end <= region->start) {
        return false;
    }
    region->size = region->end - region->start;
    ptr = space + 1;
    ptr = SkipWhitespace(ptr);
    // parse perms
    if (strlen(ptr) < OHOS::AbilityRuntime::VmaUtil::PERMS_LEN) {
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
    region->perms[OHOS::AbilityRuntime::VmaUtil::PERMS_LEN] = '\0';
    ptr += OHOS::AbilityRuntime::VmaUtil::PERMS_LEN;
    ptr = SkipWhitespace(ptr);
    // skip offset
    ptr = strchr(ptr, ' ');
    if (!ptr) {
        return false;
    }
    ptr = SkipWhitespace(ptr);
    //skip device
    ptr = strchr(ptr, ' ');
    if (!ptr) {
        return false;
    }
    ptr = SkipWhitespace(ptr);
    // skip innode
    ptr = strchr(ptr, ' ');
    if (!ptr) {
        return false;
    }
    ptr = SkipWhitespace(ptr);
    // parse pathname
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

static int32_t IterateMapsInternal(const std::vector<std::string>& filenames,
    std::vector<OHOS::AbilityRuntime::VmaUtil::VMARegion>& vmaList)
{
    const char* mapsPath = "/proc/self/maps";
    FILE* fp = fopen(mapsPath, "r");
    if (!fp) {
        TAG_LOGE(OHOS::AAFwk::AAFwkLogTag::DEFAULT, "Failed to open maps: %{public}d", errno);
        return -1;
    }
    std::unordered_set<std::string> targetSet(filenames.begin(), filenames.end());
    int32_t count = 0;
    char line[LINE_BUFFER_SIZE];
    while (fgets(line, sizeof(line), fp) != nullptr) {
        OHOS::AbilityRuntime::VmaUtil::VMARegion region;
        size_t lineLen = strlen(line);
        if (!ParseMapsLine(line, lineLen, &region)) {
            continue;
        }
        const char* pathname = region.pathname;
        if (!pathname || strlen(pathname) == 0) {
            continue;
        }
        const char* basename = strrchr(pathname, '/');
        basename = (basename == nullptr) ? pathname : (basename + 1);
        if (!basename || strlen(basename) == 0) {
            continue;
        }
        if (targetSet.find(basename) != targetSet.end()) {
            vmaList.push_back(region);
            count++;
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
std::vector<VMARegion> GetFileVmas(const std::vector<std::string>& filenames)
{
    if (filenames.empty()) {
        TAG_LOGE(AAFwkTag::ABILITY, "Filename list is empty");
        return {};
    }
    TAG_LOGD(AAFwkTag::ABILITY, "GetFileVmas: searching for %{public}zu files", filenames.size());
    std::vector<VMARegion> vmaList;
    IterateMapsInternal(filenames, vmaList);
    TAG_LOGD(AAFwkTag::ABILITY, "GetFileVmas: found %{public}zu VMAs for %{public}zu files",
        vmaList.size(), filenames.size());
    return vmaList;
}

} // namespace VmaUtil
} // namespace AbilityRuntime
} // namespace OHOS
