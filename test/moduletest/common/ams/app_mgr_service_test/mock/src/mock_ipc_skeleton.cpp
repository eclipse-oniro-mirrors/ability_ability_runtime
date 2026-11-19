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

#include "mock_ipc_skeleton.h"

#include <unistd.h>

#include "token_setproc.h"

namespace OHOS {
namespace {
// Defaults -1/0 keep the real process identity so existing cases see unmocked values.
pid_t g_mockCallingUid = -1;
uint64_t g_mockFullTokenId = 0;
} // namespace

pid_t IPCSkeleton::GetCallingUid()
{
    return g_mockCallingUid >= 0 ? g_mockCallingUid : getuid();
}

pid_t IPCSkeleton::GetCallingPid()
{
    return getpid();
}

uint32_t IPCSkeleton::GetCallingTokenID()
{
    return static_cast<uint32_t>(GetCallingFullTokenID());
}

uint32_t IPCSkeleton::SetCallingTokenID(pid_t tokenId)
{
    g_mockFullTokenId = static_cast<uint64_t>(tokenId);
    return static_cast<uint32_t>(g_mockFullTokenId);
}

uint64_t IPCSkeleton::GetCallingFullTokenID()
{
    // 0 keeps the self token (MockNativeToken), so IsSystemAppCall stays intact by default.
    return g_mockFullTokenId != 0 ? g_mockFullTokenId : GetSelfTokenID();
}

void IPCSkeleton::SetCallingUid(pid_t uid)
{
    g_mockCallingUid = uid;
}
} // namespace OHOS
