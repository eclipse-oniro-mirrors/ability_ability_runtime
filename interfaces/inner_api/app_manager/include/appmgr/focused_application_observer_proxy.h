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

#ifndef OHOS_ABILITY_RUNTIME_FOCUSED_APPLICATION_OBSERVER_PROXY_H
#define OHOS_ABILITY_RUNTIME_FOCUSED_APPLICATION_OBSERVER_PROXY_H

#include "focused_application_observer_interface.h"
#include "iremote_proxy.h"
#include "nocopyable.h"

namespace OHOS {
namespace AppExecFwk {
class FocusedApplicationObserverProxy : public IRemoteProxy<IFocusedApplicationObserver> {
public:
    explicit FocusedApplicationObserverProxy(const sptr<IRemoteObject> &impl);
    ~FocusedApplicationObserverProxy() override = default;

    void OnFocusedApplicationChanged(const FocusedApplicationInfo &focusedApplicationInfo) override;

private:
    bool WriteInterfaceToken(MessageParcel &data);
    static inline BrokerDelegator<FocusedApplicationObserverProxy> delegator_;

    DISALLOW_COPY_AND_MOVE(FocusedApplicationObserverProxy);
};
} // namespace AppExecFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_FOCUSED_APPLICATION_OBSERVER_PROXY_H
