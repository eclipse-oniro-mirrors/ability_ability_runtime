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

#ifndef OHOS_ABILITY_RUNTIME_PROCESS_LIFECYCLE_OBSERVER_PROXY_H
#define OHOS_ABILITY_RUNTIME_PROCESS_LIFECYCLE_OBSERVER_PROXY_H

#include "iremote_proxy.h"
#include "process_lifecycle_observer_interface.h"

namespace OHOS {
namespace AppExecFwk {
class ProcessLifecycleObserverProxy : public IRemoteProxy<IProcessLifecycleObserver> {
public:
    explicit ProcessLifecycleObserverProxy(const sptr<IRemoteObject> &impl);
    ~ProcessLifecycleObserverProxy() override = default;
    void OnProcessCreated(const ProcessData &processData) override;
    void OnProcessDied(const ProcessData &processData) override;

private:
    bool WriteInterfaceToken(MessageParcel &data);
    void SendRequestInternal(uint32_t code, const ProcessData &processData);

    static inline BrokerDelegator<ProcessLifecycleObserverProxy> delegator_;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_PROCESS_LIFECYCLE_OBSERVER_PROXY_H
