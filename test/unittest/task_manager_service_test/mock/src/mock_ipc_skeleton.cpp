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

namespace OHOS {
pid_t uid_ = 0;
pid_t pid_ = 1;
pid_t tokenId_ = 0;

pid_t IPCSkeleton::GetCallingUid()
{
    return uid_;
}

pid_t IPCSkeleton::GetCallingPid()
{
    return pid_;
}

void IPCSkeleton::SetCallingUid(pid_t uid)
{
    uid_ = uid;
}

void IPCSkeleton::SetCallingPid(pid_t pid)
{
    pid_ = pid;
}

uint32_t IPCSkeleton::GetCallingTokenID()
{
    return static_cast<uint32_t>(tokenId_);
}

uint32_t IPCSkeleton::SetCallingTokenID(pid_t tokenId)
{
    return static_cast<uint32_t>(tokenId_ = tokenId);
}
}  // namespace OHOS
