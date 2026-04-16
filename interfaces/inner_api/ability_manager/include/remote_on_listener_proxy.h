/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_REMOTE_ON_LISTENER_PROXY_H
#define OHOS_ABILITY_RUNTIME_REMOTE_ON_LISTENER_PROXY_H

#include "iremote_proxy.h"
#include "remote_on_listener_interface.h"

namespace OHOS {
namespace AAFwk {
/**
 * interface for remote mission listener proxy.
 */
class RemoteOnListenerProxy : public IRemoteProxy<IRemoteOnListener> {
public:
    explicit RemoteOnListenerProxy(const sptr<IRemoteObject> &impl) : IRemoteProxy<IRemoteOnListener>(impl)
    {}

    /**
     * @brief When the remote device mission changed, AbilityMs notify the listener.
     *
     * @param info, callback information containing continue state, device info, etc.
     */
    virtual void OnCallback(const OnCallbackInfo &info) override;
private:
    static inline BrokerDelegator<RemoteOnListenerProxy> delegator_;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_REMOTE_ON_LISTENER_PROXY_H
