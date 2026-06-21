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

#include <string>
#include "ipc_skeleton.h"

namespace OHOS {
#ifdef CONFIG_IPC_SINGLE
using namespace IPC_SINGLE;
#endif

// The unit-test process has no active binder invoker, so the real
// IPCSkeleton::ResetCallingIdentity() returns "" (IPCSkeleton::ResetCallingIdentity in
// ipc_skeleton.cpp returns "" when IPCThreadSkeleton::GetActiveInvoker() is null). That
// makes AgentManagerCallerIdentityScope capture an empty caller identity, so the scope
// capture/restore assertions cannot be exercised. Override it with a stable non-empty
// token. SetCallingIdentity is left to the real ipc_core implementation, which is already
// a no-op (returns true) without an active invoker.
std::string IPCSkeleton::ResetCallingIdentity()
{
    return "mock-caller-identity";
}
}  // namespace OHOS
