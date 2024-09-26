/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_RENDER_STATE_OBSERVER_PROXY_H
#define OHOS_ABILITY_RUNTIME_RENDER_STATE_OBSERVER_PROXY_H

#include "iremote_proxy.h"
#include "irender_state_observer.h"

namespace OHOS {
namespace AppExecFwk {
class RenderStateObserverProxy : public IRemoteProxy<IRenderStateObserver> {
public:
    explicit RenderStateObserverProxy(const sptr<IRemoteObject> &impl);
    virtual ~RenderStateObserverProxy() = default;

    /**
     * Called when one render process's state changes.
     *
     * @param renderStateData retrieved state data.
     */
    virtual void OnRenderStateChanged(const RenderStateData &renderStateData) override;
private:
    bool WriteInterfaceToken(MessageParcel &data);
    static inline BrokerDelegator<RenderStateObserverProxy> delegator_;
    int32_t SendTransactCmd(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option);
};
} // namespace AppExecFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_RENDER_STATE_OBSERVER_PROXY_H