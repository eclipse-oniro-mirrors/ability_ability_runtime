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

#include "ipc_skeleton.h"

namespace OHOS {

// FOUNDATION_UID from cli_tool_manager_service.cpp
constexpr int32_t FOUNDATION_UID = 5523;
constexpr pid_t DEFAULT_CALLING_PID = 1000;

pid_t IPCSkeleton::callingUid = FOUNDATION_UID;
pid_t IPCSkeleton::callingPid = DEFAULT_CALLING_PID;
uint64_t IPCSkeleton::callingFullTokenId = 0;
uint64_t IPCSkeleton::callingTokenId = 1;

pid_t IPCSkeleton::GetCallingUid()
{
    return callingUid;
}

pid_t IPCSkeleton::GetCallingPid()
{
    return callingPid;
}

uint64_t IPCSkeleton::GetCallingFullTokenID()
{
    return callingFullTokenId;
}

uint64_t IPCSkeleton::GetCallingTokenID()
{
    return callingTokenId;
}

void IPCSkeleton::Reset()
{
    callingUid = FOUNDATION_UID;
    callingPid = DEFAULT_CALLING_PID;
    callingFullTokenId = 0;
    callingTokenId = 1;
}

} // namespace OHOS
