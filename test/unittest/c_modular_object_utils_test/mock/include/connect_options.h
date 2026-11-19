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

#ifndef ABILITY_RUNTIME_CONNECT_OPTIONS_H
#define ABILITY_RUNTIME_CONNECT_OPTIONS_H

#include <stdint.h>
#include "ability_runtime_common.h"
#include "want.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct OHIPCRemoteProxy OHIPCRemoteProxy;

typedef struct OH_AbilityRuntime_ConnectOptions OH_AbilityRuntime_ConnectOptions;

typedef void (*OH_AbilityRuntime_ConnectOptions_OnConnectCallback)(
    OH_AbilityRuntime_ConnectOptions *, AbilityBase_Element *, OHIPCRemoteProxy *);

typedef void (*OH_AbilityRuntime_ConnectOptions_OnDisconnectCallback)(
    OH_AbilityRuntime_ConnectOptions *, AbilityBase_Element *);

typedef void (*OH_AbilityRuntime_ConnectOptions_OnFailedCallback)(
    OH_AbilityRuntime_ConnectOptions *, AbilityRuntime_ErrorCode);

#ifdef __cplusplus
}
#endif

#endif // ABILITY_RUNTIME_CONNECT_OPTIONS_H
