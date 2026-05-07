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

#include "native_ability_wrapper.h"

#include <cstring>

#include "hilog_tag_wrapper.h"
#include "ability_native_thread.h"
#include "securec.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Get ability instance ID from NativeAbilityWrapper.
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_GetAbilityInstanceId(
    const NativeAbilityWrapper* nativeAbilityWrapper, char* buffer, const int32_t bufferSize)
{
    constexpr int32_t MIN_BUFFER_SIZE = 37; // UUID format: xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx + '\0'
    if (nativeAbilityWrapper == nullptr || buffer == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "Invalid parameter: nativeAbilityWrapper or buffer is null");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }

    if (bufferSize < MIN_BUFFER_SIZE) {
        TAG_LOGE(AAFwkTag::APPKIT, "Invalid parameter: bufferSize %{public}d is less than %{public}d",
            bufferSize, MIN_BUFFER_SIZE);
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }

    if (nativeAbilityWrapper->instanceId.empty()) {
        TAG_LOGE(AAFwkTag::APPKIT, "Instance ID is empty");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }

    errno_t err = strncpy_s(buffer, bufferSize, nativeAbilityWrapper->instanceId.c_str(),
        nativeAbilityWrapper->instanceId.length());
    if (err != 0) {
        TAG_LOGE(AAFwkTag::APPKIT, "Failed to copy instance id to buffer, errno: %{public}d", err);
        return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
    }

    TAG_LOGD(AAFwkTag::APPKIT, "Get ability instance id: %{public}s", buffer);
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

/**
 * @brief Get ability name from NativeAbilityWrapper.
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_GetAbilityName(
    const NativeAbilityWrapper* nativeAbilityWrapper, char* buffer, const int32_t bufferSize, int32_t* writeLength)
{
    if (nativeAbilityWrapper == nullptr || writeLength == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "Invalid parameter: nativeAbilityWrapper or writeLength is null");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }

    if (nativeAbilityWrapper->abilityName.empty()) {
        TAG_LOGE(AAFwkTag::APPKIT, "Ability name is empty");
        return ABILITY_RUNTIME_ERROR_CODE_CONTEXT_NOT_EXIST;
    }

    int32_t nameLength = static_cast<int32_t>(nativeAbilityWrapper->abilityName.length());

    // If buffer is null, return the required buffer size
    if (buffer == nullptr) {
        *writeLength = nameLength;
        TAG_LOGD(AAFwkTag::APPKIT, "Query ability name length: %{public}d", *writeLength);
        return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
    }

    if (bufferSize <= 0) {
        TAG_LOGE(AAFwkTag::APPKIT, "Invalid parameter: bufferSize must be positive");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }

    if (nameLength >= bufferSize) {
        TAG_LOGE(AAFwkTag::APPKIT, "Buffer size %{public}d is too small for ability name length %{public}d",
            bufferSize, nameLength);
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }

    // Copy ability name to buffer
    errno_t err = strncpy_s(buffer, bufferSize, nativeAbilityWrapper->abilityName.c_str(), nameLength);
    if (err != 0) {
        TAG_LOGE(AAFwkTag::APPKIT, "Failed to copy ability name to buffer, errno: %{public}d", err);
        return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
    }

    *writeLength = nameLength;
    TAG_LOGD(AAFwkTag::APPKIT, "Get ability name: %{public}s, length: %{public}d", buffer, *writeLength);
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

/**
 * @brief Get napi_env from NativeAbilityWrapper.
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_GetEnv(
    const NativeAbilityWrapper* nativeAbilityWrapper, napi_env* env)
{
    if (nativeAbilityWrapper == nullptr || env == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "Invalid parameter: nativeAbilityWrapper or env is null");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }

    if (nativeAbilityWrapper->env == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "napi_env in NativeAbilityWrapper is null");
        return ABILITY_RUNTIME_ERROR_CODE_CONTEXT_NOT_EXIST;
    }

    *env = nativeAbilityWrapper->env;
    TAG_LOGD(AAFwkTag::APPKIT, "Get napi_env successfully");
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

#ifdef __cplusplus
}
#endif
