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
 * @brief Defines the child process active state type and notification API for application memory optimization.
 *
 * @since 26.1.0
 */

/**
 * @file app_memory_optimizer.h
 * @brief Defines the child process active state type and notification function.
 * @syscap SystemCapability.Ability.AbilityRuntime.Core
 * @include <AbilityKit/ability_runtime/app_memory_optimizer.h>
 * @library libability_runtime.so
 * @kit AbilityKit
 * @since 26.1.0
 */

#ifndef ABILITY_RUNTIME_APP_MEMORY_OPTIMIZER_H
#define ABILITY_RUNTIME_APP_MEMORY_OPTIMIZER_H

#include <stdint.h>
#include "ability_runtime_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Enumerates the active states of a child process.
 *
 * A child process is in the active state by default when it is started.
 *
 * @since 26.1.0
 */
typedef enum {
    /**
     * @brief The child process is active. The system does not perform memory optimization on it.
     *
     * @since 26.1.0
     */
    OH_ABILITY_RUNTIME_CHILD_PROCESS_STATE_ACTIVE = 0,
    /**
     * @brief The child process is inactive. The system can perform memory optimization on it.
     *
     * @since 26.1.0
     */
    OH_ABILITY_RUNTIME_CHILD_PROCESS_STATE_INACTIVE = 1,
} OH_AbilityRuntime_ChildProcessState;

/**
 * @brief Notifies the system of the active state of a child process of the current application.
 *
 * When a child process is marked as inactive, the system can perform memory optimization on it. When the child
 * process is marked as active again, the system stops performing memory optimization on it.
 * Only the child processes created by the current application can be marked.
 *
 * Only child processes created by the current application through system child process creation APIs can be
 * marked. Processes forked by the application itself (such as by calling fork() directly or starting in
 * SELF_FORK mode via childProcessManager.startChildProcess) are not supported; child processes created through
 * other child process creation APIs (such as startArkChildProcess, startNativeChildProcess, and NDK APIs such
 * as OH_Ability_StartNativeChildProcess) are supported.
 *
 * This API can only be called in the main process of the application. If it is called in a child process,
 * {@link ABILITY_RUNTIME_ERROR_CODE_CHILD_PROCESS_NOT_EXIST} is returned.
 *
 * This API is thread-safe and can be called from any thread. It is a synchronous call that blocks until the
 * result is returned. Notifying the same state for the same child process repeatedly is idempotent: the call
 * still succeeds and no additional state change callback is triggered.
 *
 * @param pid [in] PID of the child process. The value is the PID returned by the child process creation API.
 * @param state [in] Active state of the child process to set. Only
 *     {@link OH_ABILITY_RUNTIME_CHILD_PROCESS_STATE_ACTIVE} and
 *     {@link OH_ABILITY_RUNTIME_CHILD_PROCESS_STATE_INACTIVE} are accepted; for any other value,
 *     {@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} is returned.
 * @return <ul>
 *     <li>{@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if the operation is successful.</li>
 *     <li>{@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if the arguments provided are invalid.</li>
 *     <li>{@link ABILITY_RUNTIME_ERROR_CODE_CHILD_PROCESS_NOT_EXIST} if the child process does not exist or is
 *     not created by the current application, or the API is called in a child process.</li>
 *     </ul>
 * @since 26.1.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_NotifyChildProcessState(
    int32_t pid, OH_AbilityRuntime_ChildProcessState state);

#ifdef __cplusplus
}
#endif

/** @} */
#endif // ABILITY_RUNTIME_APP_MEMORY_OPTIMIZER_H