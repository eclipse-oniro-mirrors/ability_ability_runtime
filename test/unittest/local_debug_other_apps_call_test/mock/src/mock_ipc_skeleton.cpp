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
#include "mock_my_status.h"

namespace OHOS {
#ifdef CONFIG_IPC_SINGLE
using namespace IPC_SINGLE;
#endif

pid_t IPCSkeleton::GetCallingUid()
{
    return static_cast<pid_t>(AAFwk::MyStatus::GetInstance().ipcGetCallingUid_);
}

pid_t IPCSkeleton::GetCallingPid()
{
    return 0;
}

uint32_t IPCSkeleton::GetCallingTokenID()
{
    return 0;
}
} // namespace OHOS
