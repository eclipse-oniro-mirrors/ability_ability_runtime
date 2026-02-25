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

#ifndef MADVISE_UTILS_H
#define MADVISE_UTILS_H

#include <cstdint>
#include <string>
#include <vector>

namespace OHOS {
namespace AbilityRuntime {
namespace MadviseUtil {
bool MadviseSingleLibrary(const char* libName);

bool MadviseGeneralFile(const char* filename);

int MadviseGeneralFiles(const std::vector<std::string>& filenames);

int MadviseWithConfigFile(const char* packageName);

std::string GetConfigPath();

} // namespace MadviseUtil
} // namespace AbilityRuntime
} // namespace OHOS
#endif // MADVISE_UTILS_H