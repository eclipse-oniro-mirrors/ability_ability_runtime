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

#include "errors.h"

namespace OHOS {
namespace AbilityRuntime {
namespace MadviseUtil {
bool MadviseSingleLibrary(const char* libName);

int32_t MadviseGeneralFiles(const std::vector<std::string>& filenames);

int32_t MadviseWithConfigFile(const char* bundleName);

// Evicts file page cache for the given file names by extension:
// names ending with ".so" go through MadviseSingleLibrary (ELF walk) one by one;
// all other names (.hap/.hsp) are batched into a single MadviseGeneralFiles call.
// The caller is responsible for validating extensions. Returns the count of
// successfully processed files (same semantics as ApplyMadviseWithConfig).
int32_t EvictFilePages(const std::vector<std::string>& fileNames);

// Checks whether name ends with one of the supported evict extensions:
// .so, .hap, or .hsp.
bool IsValidEvictFileName(const std::string& name);

// For each moduleName, resolves the caller's own hap, reads
// resources/rawfile/memory_optimizer.json, parses the evictFilePages array,
// validates extensions, and dispatches eviction via EvictFilePages.
// Returns ERR_OK on success; ERR_EVICT_FILE_TYPE if any configured file name
// has an unsupported extension; ERR_EVICT_CONFIG_PARSE if the caller bundle
// info / module / config file cannot be obtained or parsed. The native code is
// mapped to the JS/ETS external error code at the binding layer.
ErrCode EvictModuleFilePages(const std::vector<std::string>& moduleNames);

} // namespace MadviseUtil
} // namespace AbilityRuntime
} // namespace OHOS
#endif // MADVISE_UTILS_H