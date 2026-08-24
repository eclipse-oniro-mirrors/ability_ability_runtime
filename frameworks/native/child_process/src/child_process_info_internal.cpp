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

#include "child_process_info_internal.h"

#include <cstring>
#include <limits>
#include "securec.h"
#include "hilog_tag_wrapper.h"

namespace {
// Fetch a single item handle by index with bounds checking. Returns nullptr on invalid args / out-of-range.
inline OH_AbilityRuntime_ChildProcessInfo* GetItemOrNull(
    OH_AbilityRuntime_ChildProcessInfosHandle infos, uint32_t index)
{
    if (infos == nullptr || index >= infos->count) {
        return nullptr;
    }
    return &infos->items[index];
}
}  // namespace

bool CreateAndFillChildProcessInfos(
    const std::vector<OHOS::AppExecFwk::ChildProcessInfo>& childInfos,
    OH_AbilityRuntime_ChildProcessInfos** infos, uint32_t* count)
{
    if (infos == nullptr || count == nullptr) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "invalid params");
        return false;
    }
    if (childInfos.size() > std::numeric_limits<uint32_t>::max()) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "childInfos size overflow");
        return false;
    }
    *count = static_cast<uint32_t>(childInfos.size());
    if (childInfos.empty()) {
        return true;
    }
    auto* result = new (std::nothrow) OH_AbilityRuntime_ChildProcessInfos();
    if (result == nullptr) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "alloc ChildProcessInfos failed");
        return false;
    }
    result->count = *count;
    result->items = new (std::nothrow) OH_AbilityRuntime_ChildProcessInfo[result->count];
    if (result->items == nullptr) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "alloc items array failed");
        delete result;
        *count = 0;
        return false;
    }
    for (uint32_t i = 0; i < result->count; i++) {
        result->items[i].pid = childInfos[i].pid;
        result->items[i].parentPid = childInfos[i].hostPid;
        result->items[i].processName = strdup(childInfos[i].processName.c_str());
        if (result->items[i].processName == nullptr) {
            TAG_LOGE(AAFwkTag::PROCESSMGR, "strdup processName failed, index=%{public}u", i);
            // rollback already-allocated processNames
            for (uint32_t j = 0; j < i; j++) {
                free(result->items[j].processName);
            }
            delete[] result->items;
            delete result;
            *count = 0;
            return false;
        }
    }
    *infos = result;
    return true;
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_GetChildProcessInfoByIndex(
    OH_AbilityRuntime_ChildProcessInfosHandle infos, uint32_t index,
    OH_AbilityRuntime_ChildProcessInfoHandle* info)
{
    if (info == nullptr) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "info is nullptr");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    auto* item = GetItemOrNull(infos, index);
    if (item == nullptr) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "item not found, index=%{public}u", index);
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    *info = item;
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_ChildProcessInfo_GetPid(
    OH_AbilityRuntime_ChildProcessInfoHandle info, int32_t* pid)
{
    if (info == nullptr || pid == nullptr) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "invalid params");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    *pid = info->pid;
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_ChildProcessInfo_GetParentPid(
    OH_AbilityRuntime_ChildProcessInfoHandle info, int32_t* parentPid)
{
    if (info == nullptr || parentPid == nullptr) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "invalid params");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    *parentPid = info->parentPid;
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_ChildProcessInfo_GetProcessName(
    OH_AbilityRuntime_ChildProcessInfoHandle info, char *processName,
    uint32_t processNameSize, uint32_t *requiredSize)
{
    if (info == nullptr || processName == nullptr || requiredSize == nullptr || processNameSize == 0) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "invalid params");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    if (info->processName == nullptr) {
        *requiredSize = 1;
        processName[0] = '\0';
        return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
    }
    uint32_t nameLen = static_cast<uint32_t>(strlen(info->processName));
    *requiredSize = nameLen + 1;
    if (processNameSize < *requiredSize) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "buffer too small, requiredSize=%{public}u", *requiredSize);
        return ABILITY_RUNTIME_ERROR_CODE_BUFFER_TOO_SMALL;
    }
    errno_t rc = strncpy_s(processName, processNameSize, info->processName, nameLen);
    if (rc != EOK) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "strncpy_s failed, rc=%{public}d", rc);
        processName[0] = '\0';
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

void OH_AbilityRuntime_ReleaseChildProcessInfos(
    OH_AbilityRuntime_ChildProcessInfosHandle* infos)
{
    if (infos == nullptr || *infos == nullptr) {
        TAG_LOGW(AAFwkTag::PROCESSMGR, "nullptr params");
        return;
    }
    auto* handle = *infos;
    for (uint32_t i = 0; i < handle->count; i++) {
        free(handle->items[i].processName);
    }
    delete[] handle->items;
    delete handle;
    *infos = nullptr;
}
