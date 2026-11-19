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

#ifndef MOCK_IPC_CREMOTE_OBJECT_H
#define MOCK_IPC_CREMOTE_OBJECT_H

#include <cstdint>

#include "ipc_cparcel.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*OH_OnRemoteRequestCallback)(uint32_t code, const OHIPCParcel *data,
    OHIPCParcel *reply, void *userData);
typedef void (*OH_OnRemoteDestroyCallback)(void *userData);

OHIPCRemoteStub* OH_IPCRemoteStub_Create(const char *descriptor, OH_OnRemoteRequestCallback requestCallback,
    OH_OnRemoteDestroyCallback destroyCallback, void *userData);
void OH_IPCRemoteStub_Destroy(OHIPCRemoteStub *stub);

#ifdef __cplusplus
}
#endif

#endif // MOCK_IPC_CREMOTE_OBJECT_H