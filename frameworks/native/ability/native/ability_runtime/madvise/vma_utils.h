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

#ifndef VMA_UTILS_H
#define VMA_UTILS_H

#include <cstdint>
#include <cstddef>
#include <vector>

namespace OHOS {
namespace AbilityRuntime {
namespace VmaUtil {
constexpr size_t VMA_MAX_PATH_NAME = 256;
constexpr size_t VMA_MAX_PERMS = 5;

struct VMARegion {
    uintptr_t start;
    uintptr_t end;
    uintptr_t size;
    char perms[VMA_MAX_PERMS];
    unsigned long offset;
    unsigned long inode;
    char pathname[VMA_MAX_PATH_NAME];
};

std::vector<VMARegion> GetFileVmas(const char* filename);

std::vector<VMARegion> GetFileVmas(const std::vector<std::string>& filenames);

int GetFileVmaPss(const char* filename, unsigned long* pssKb);

} // namespace VmaUtil
} // namespace AbilityRuntime
} // namespace OHOS
#endif // VMA_UTILS_H
