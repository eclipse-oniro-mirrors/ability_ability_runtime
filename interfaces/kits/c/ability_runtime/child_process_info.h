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

/**
 * @addtogroup AbilityRuntime
 * @{
 *
 * @brief Defines the child process info types and accessor APIs.
 *
 * @since 26.1.0
 */

/**
 * @file child_process_info.h
 *
 * @brief Defines the child process info type and accessor functions.
 *
 * @library libability_runtime.so
 * @kit AbilityKit
 * @syscap SystemCapability.Ability.AbilityRuntime.Core
 * @since 26.1.0
 */

#ifndef ABILITY_RUNTIME_CHILD_PROCESS_INFO_H
#define ABILITY_RUNTIME_CHILD_PROCESS_INFO_H

#include <stdint.h>
#include "ability_runtime_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Defines the pointer to OH_AbilityRuntime_ChildProcessInfos.
 * @since 26.1.0
 */
typedef struct OH_AbilityRuntime_ChildProcessInfos *OH_AbilityRuntime_ChildProcessInfosHandle;

/**
 * @brief Defines the pointer to OH_AbilityRuntime_ChildProcessInfo.
 * @since 26.1.0
 */
typedef struct OH_AbilityRuntime_ChildProcessInfo *OH_AbilityRuntime_ChildProcessInfoHandle;

/**
 * @brief Retrieves a specific child process info handle from the collection by its index.
 *
 * @param infos [in] Information about all child processes within the self application.
 * @param index [in]The index of the child process info to retrieve. Must be strictly less than the count.
 * @param info [out] The retrieved single child process info handle for the specified index.
 * @return <ul>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if the operation is successful.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if the arguments provided are invalid.</li>
 *      </ul>
 * @since 26.1.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_GetChildProcessInfoByIndex(
    OH_AbilityRuntime_ChildProcessInfosHandle infos, uint32_t index,
    OH_AbilityRuntime_ChildProcessInfoHandle *info);

/**
 * @brief Gets PID of child process info.
 *
 * @param info [in] Pointer to child process info. It must not be NULL.
 * @param pid [out] Pointer to child process PID. It must not be NULL.
 * @return <ul>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if the operation is successful.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if the arguments provided are invalid.</li>
 *      </ul>
 * @since 26.1.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_ChildProcessInfo_GetPid(
    OH_AbilityRuntime_ChildProcessInfoHandle info, int32_t *pid);

/**
 * @brief Gets parent PID of child process info.
 *
 * @param info [in] Pointer to child process info. It must not be NULL.
 * @param parentPid [out] Pointer to parent process PID. It must not be NULL.
 * @return <ul>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if the operation is successful.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if the arguments provided are invalid.</li>
 *      </ul>
 * @since 26.1.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_ChildProcessInfo_GetParentPid(
    OH_AbilityRuntime_ChildProcessInfoHandle info, int32_t *parentPid);

/**
 * @brief Gets process name of child process info.
 *
 * @param info [in] Pointer to child process info. It must not be NULL.
 * @param processName [out] Pointer to the buffer to receive process name. It must not be NULL.
 * @param processNameSize [in] Size of the buffer in bytes, including the trailing NUL. It must not be 0.
 * @param requiredSize [out] Pointer to the required size in bytes, including the trailing NUL.
 *     It must not be NULL. If the buffer is too small, this value indicates the required size.
 * @return <ul>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if the operation is successful.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if processName or requiredSize is NULL,
 *      or processNameSize is 0.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_BUFFER_TOO_SMALL} if the buffer is too small.</li>
 *      <li>{@link ABILITY_RUNTIME_ERROR_CODE_INTERNAL} if string copy operation failed.</li>
 *      </ul>
 * @since 26.1.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_ChildProcessInfo_GetProcessName(
    OH_AbilityRuntime_ChildProcessInfoHandle info, char *processName, uint32_t processNameSize, uint32_t *requiredSize);

/**
 * @brief Releases child process info collection.
 *
 * @param infos [in] The child process infos to be released. It must not be NULL.
 *     After release, handle will be set to NULL.
 * @since 26.1.0
 */
void OH_AbilityRuntime_ReleaseChildProcessInfos(OH_AbilityRuntime_ChildProcessInfosHandle *infos);

#ifdef __cplusplus
}
#endif

/** @} */
#endif // ABILITY_RUNTIME_CHILD_PROCESS_INFO_H