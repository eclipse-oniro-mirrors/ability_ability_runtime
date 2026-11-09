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

#ifndef OHOS_ABILITY_RUNTIME_CHILD_PROCESS_STATE_OBSERVER_PROXY_H
#define OHOS_ABILITY_RUNTIME_CHILD_PROCESS_STATE_OBSERVER_PROXY_H

#include "iremote_proxy.h"
#include "ichild_process_state_observer.h"

namespace OHOS {
namespace AppExecFwk {
class ChildProcessStateObserverProxy : public IRemoteProxy<IChildProcessStateObserver> {
public:
    explicit ChildProcessStateObserverProxy(const sptr<IRemoteObject> &impl);
    virtual ~ChildProcessStateObserverProxy() = default;

    /**
     * Called when one child process's state changes.
     *
     * @param childProcessStateData retrieved state data.
     */
    virtual void OnChildProcessStateChanged(const ChildProcessStateData &childProcessStateData) override;
private:
    bool WriteInterfaceToken(MessageParcel &data);
    static inline BrokerDelegator<ChildProcessStateObserverProxy> delegator_;
    int32_t SendTransactCmd(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option);
};
} // namespace AppExecFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_CHILD_PROCESS_STATE_OBSERVER_PROXY_H