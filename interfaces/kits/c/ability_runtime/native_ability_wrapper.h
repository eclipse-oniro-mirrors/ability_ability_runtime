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
 * @brief Provides the definition of the C interface for the native ability wrapper
 *
 * @since 26.0.0
 */

/**
 * @file native_ability_wrapper.h
 *
 * @brief Declares the native ability wrapper APIs.
 *
 * @library libability_runtime.so
 * @kit AbilityKit
 * @syscap SystemCapability.Ability.AbilityRuntime.Core
 * @since 26.0.0
 */

#ifndef ABILITY_RUNTIME_NATIVE_ABILITY_WRAPPER_H
#define ABILITY_RUNTIME_NATIVE_ABILITY_WRAPPER_H

#include <stdint.h>
#include <napi/native_api.h>
#include "ability_runtime_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Defines the AbilityRuntime_NativeAbilityWrapper structure type.
 *
 * @since 26.0.0
 */
typedef struct AbilityRuntime_NativeAbilityWrapper AbilityRuntime_NativeAbilityWrapper;

/**
 * @brief Get ability instance ID from NativeAbilityWrapper.
 *
 * @param nativeAbilityWrapper The native ability wrapper pointer.
 * @param buffer A pointer to a buffer that receives the instance ID string which is UUID format,
 *               an amount of 37 bytes, 36 chars plus an ending '\0'.
 * @param bufferSize The length of the buffer, must be at least 37.
 * @return The error code.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if the operation is successful.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if the nativeAbilityWrapper or buffer is null,
 *         or the buffer size is less than 37.
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_GetAbilityInstanceId(
    const AbilityRuntime_NativeAbilityWrapper* nativeAbilityWrapper, char* buffer, const int32_t bufferSize);

/**
 * @brief Get ability name from NativeAbilityWrapper.
 *
 * @param nativeAbilityWrapper The native ability wrapper pointer.
 * @param buffer A pointer to a buffer that receives the ability name.
 *               Pass nullptr to query the ability name length.
 * @param bufferSize The length of the buffer. Make sure the buffer has at least one more byte for '\0'.
 * @param writeLength Outputs the ability name string length.
 * @return The error code.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if the operation is successful.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if the nativeAbilityWrapper or writeLength is null,
 *         or the buffer is too small for the ability name.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_ABILITY_WRAPPER_INVALID} if the native ability wrapper is invalid or
 *         incomplete.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_INTERNAL} inner error.
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_GetAbilityName(
    const AbilityRuntime_NativeAbilityWrapper *nativeAbilityWrapper, char *buffer, const int32_t bufferSize,
    int32_t *writeLength);

/**
 * @brief Get napi_env from NativeAbilityWrapper.
 *
 * @param nativeAbilityWrapper The native ability wrapper pointer.
 * @param env A pointer to the receive napi_env value. napi_env is valid until the process terminates.
 * @return The error code.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if the operation is successful.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if the nativeAbilityWrapper or env is null.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_ABILITY_WRAPPER_INVALID} if the native ability wrapper is invalid or
 *         incomplete.
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_GetEnv(
    const AbilityRuntime_NativeAbilityWrapper* nativeAbilityWrapper, napi_env* env);

#ifdef __cplusplus
}
#endif

/** @} */
#endif // ABILITY_RUNTIME_NATIVE_ABILITY_WRAPPER_H
