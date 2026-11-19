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

#ifndef OHOS_AGENT_RUNTIME_FRAMEWORK_AGENT_REMOTE_OBJECT_KEY_H
#define OHOS_AGENT_RUNTIME_FRAMEWORK_AGENT_REMOTE_OBJECT_KEY_H

#include <cstdint>
#include <tuple>

#include "ipc_object_proxy.h"
#include "iremote_object.h"

namespace OHOS {
namespace AgentRuntime {
struct AgentRemoteObjectKey {
    uint32_t handle = 0;
    uintptr_t localObject = 0;

    bool operator<(const AgentRemoteObjectKey &other) const
    {
        return std::tie(handle, localObject) < std::tie(other.handle, other.localObject);
    }
};

inline AgentRemoteObjectKey BuildAgentRemoteObjectKey(const sptr<IRemoteObject> &remoteObject)
{
    AgentRemoteObjectKey key;
    if (remoteObject == nullptr) {
        return key;
    }
    if (remoteObject->IsProxyObject()) {
        auto proxy = static_cast<IPCObjectProxy *>(remoteObject.GetRefPtr());
        key.handle = proxy->GetHandle();
        return key;
    }
    key.localObject = reinterpret_cast<uintptr_t>(remoteObject.GetRefPtr());
    return key;
}
} // namespace AgentRuntime
} // namespace OHOS

#endif // OHOS_AGENT_RUNTIME_FRAMEWORK_AGENT_REMOTE_OBJECT_KEY_H
