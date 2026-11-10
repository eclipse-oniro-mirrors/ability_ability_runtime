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

#include <cstdint>
#include <cstring>

using AppSpawnClientHandle = void *;
using AppSpawnReqMsgHandle = void *;

struct AppSpawnResult {
    int result = 0;
    int pid = 0;
    uint64_t checkPointId = 0;
};

extern "C" {
char *GetSubSpawnSocketName(int uid)
{
    return nullptr;
}

int AppSpawnClientInitSubSpawn(const char *socketName, AppSpawnClientHandle *handle)
{
    return -1;
}

int AppSpawnClientInit(const char *serviceName, AppSpawnClientHandle *handle)
{
    return -1;
}

void AppSpawnClientDestroy(AppSpawnClientHandle handle) {}

int AppSpawnReqMsgCreate(uint32_t msgType, const char *processName,
    AppSpawnReqMsgHandle *reqHandle)
{
    return -1;
}

int AppSpawnClientSendMsg(AppSpawnClientHandle handle,
    AppSpawnReqMsgHandle reqHandle, AppSpawnResult *result)
{
    return -1;
}

int AppSpawnReqMsgAddStringInfo(AppSpawnReqMsgHandle reqHandle,
    const char *name, const char *value)
{
    return 0;
}

bool IsSupportSubSpawn()
{
    return false;
}
}
