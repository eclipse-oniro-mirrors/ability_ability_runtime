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

#ifndef MOCK_CONNECT_OPTIONS_IMPL_H
#define MOCK_CONNECT_OPTIONS_IMPL_H

#include <mutex>
#include <functional>

struct AbilityBase_Element;
typedef struct AbilityBase_Element AbilityBase_Element;

enum AbilityRuntime_ErrorCode {
    ABILITY_RUNTIME_ERROR_CODE_NO_ERROR = 0,
    ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID = 401,
    ABILITY_RUNTIME_ERROR_CODE_INTERNAL = 16005000,
};

struct OH_AbilityRuntime_ConnectOptions;

typedef void (*OH_AbilityRuntime_ConnectOptions_OnConnectCallback)(
    OH_AbilityRuntime_ConnectOptions *, AbilityBase_Element *, void *);
typedef void (*OH_AbilityRuntime_ConnectOptions_OnDisconnectCallback)(
    OH_AbilityRuntime_ConnectOptions *, AbilityBase_Element *);
typedef void (*OH_AbilityRuntime_ConnectOptions_OnFailedCallback)(
    OH_AbilityRuntime_ConnectOptions *, AbilityRuntime_ErrorCode);

struct OH_AbilityRuntime_ConnectOptionsState {
    std::mutex mutex;
    bool alive = true;
    OH_AbilityRuntime_ConnectOptions *owner = nullptr;
    OH_AbilityRuntime_ConnectOptions_OnConnectCallback onConnectCallback = nullptr;
    OH_AbilityRuntime_ConnectOptions_OnDisconnectCallback onDisconnectCallback = nullptr;
    OH_AbilityRuntime_ConnectOptions_OnFailedCallback onFailedCallback = nullptr;
};

struct OH_AbilityRuntime_ConnectOptions {
    std::shared_ptr<OH_AbilityRuntime_ConnectOptionsState> state;
};

#endif // MOCK_CONNECT_OPTIONS_IMPL_H
