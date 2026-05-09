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

#include "connect_options.h"

#include <new>

#include "connect_options_impl.h"
#include "hilog_tag_wrapper.h"

namespace {
std::shared_ptr<OH_AbilityRuntime_ConnectOptionsState> GetState(OH_AbilityRuntime_ConnectOptions *connectOptions)
{
    if (connectOptions == nullptr || connectOptions->state == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "invalid connectOptions");
        return nullptr;
    }
    return connectOptions->state;
}
} // namespace

#ifdef __cplusplus
extern "C" {
#endif

OH_AbilityRuntime_ConnectOptions* OH_AbilityRuntime_CreateConnectOptions(void)
{
    auto connectOptions = new (std::nothrow) OH_AbilityRuntime_ConnectOptions();
    if (connectOptions == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null connectOptions");
        return nullptr;
    }

    auto state = std::make_shared<OH_AbilityRuntime_ConnectOptionsState>();
    if (state == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null state");
        delete connectOptions;
        return nullptr;
    }
    state->owner = connectOptions;
    connectOptions->state = state;
    return connectOptions;
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_DestroyConnectOptions(OH_AbilityRuntime_ConnectOptions *connectOptions)
{
    auto state = GetState(connectOptions);
    if (state == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }

    {
        std::lock_guard<std::mutex> guard(state->mutex);
        state->alive = false;
        state->owner = nullptr;
        state->onConnectCallback = nullptr;
        state->onDisconnectCallback = nullptr;
        state->onFailedCallback = nullptr;
    }
    delete connectOptions;
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_ConnectOptions_SetOnConnectCallback(
    OH_AbilityRuntime_ConnectOptions *connectOptions,
    OH_AbilityRuntime_ConnectOptions_OnConnectCallback onConnectCallback)
{
    auto state = GetState(connectOptions);
    if (state == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "invalid params");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    std::lock_guard<std::mutex> guard(state->mutex);
    if (!state->alive) {
        TAG_LOGE(AAFwkTag::APPKIT, "connect options already destroyed");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    state->onConnectCallback = onConnectCallback;
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_ConnectOptions_SetOnDisconnectCallback(
    OH_AbilityRuntime_ConnectOptions *connectOptions,
    OH_AbilityRuntime_ConnectOptions_OnDisconnectCallback onDisconnectCallback)
{
    auto state = GetState(connectOptions);
    if (state == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "invalid params");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    std::lock_guard<std::mutex> guard(state->mutex);
    if (!state->alive) {
        TAG_LOGE(AAFwkTag::APPKIT, "connect options already destroyed");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    state->onDisconnectCallback = onDisconnectCallback;
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_ConnectOptions_SetOnFailedCallback(
    OH_AbilityRuntime_ConnectOptions *connectOptions,
    OH_AbilityRuntime_ConnectOptions_OnFailedCallback onFailedCallback)
{
    auto state = GetState(connectOptions);
    if (state == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "invalid params");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    std::lock_guard<std::mutex> guard(state->mutex);
    if (!state->alive) {
        TAG_LOGE(AAFwkTag::APPKIT, "connect options already destroyed");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    state->onFailedCallback = onFailedCallback;
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

#ifdef __cplusplus
} // extern "C"
#endif
