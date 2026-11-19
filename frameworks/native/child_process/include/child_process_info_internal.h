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

#ifndef ABILITY_RUNTIME_CHILD_PROCESS_INFO_INTERNAL_H
#define ABILITY_RUNTIME_CHILD_PROCESS_INFO_INTERNAL_H

#include <cstdlib>
#include <new>
#include <vector>
#include "child_process_info.h"            // public C API header (opaque types + accessors)
#include "appmgr/child_process_info.h"     // inner API (ChildProcessInfo struct)

// Internal layout of OH_AbilityRuntime_ChildProcessInfo (single item).
// Fields mirror the inner API OHOS::AppExecFwk::ChildProcessInfo, but only the
// subset exposed to the C API is kept here.
struct OH_AbilityRuntime_ChildProcessInfo {
    int32_t pid;
    int32_t parentPid;        // parent process pid (mirrors inner API ChildProcessInfo::parentPid)
    char* processName;        // strdup'd, released together with the collection
};

// Internal layout of OH_AbilityRuntime_ChildProcessInfos (collection).
struct OH_AbilityRuntime_ChildProcessInfos {
    OH_AbilityRuntime_ChildProcessInfo* items;
    uint32_t count;
};

// Allocates and fills the collection from the inner API parcelable vector.
// On success: *infos points to the new collection, *count is the size.
// On empty input: *infos is set to nullptr, *count to 0, returns true.
// On allocation failure: rolls back partial allocations, *infos=nullptr, *count=0, returns false.
// Defined in child_process_info.cpp, compiled into libchild_process.so.
bool CreateAndFillChildProcessInfos(
    const std::vector<OHOS::AppExecFwk::ChildProcessInfo>& childInfos,
    OH_AbilityRuntime_ChildProcessInfos** infos, uint32_t* count);

#endif // ABILITY_RUNTIME_CHILD_PROCESS_INFO_INTERNAL_H
