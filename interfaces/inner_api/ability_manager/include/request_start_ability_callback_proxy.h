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

#ifndef OHOS_ABILITY_RUNTIME_REQUEST_START_ABILITY_CALLBACK_PROXY_H
#define OHOS_ABILITY_RUNTIME_REQUEST_START_ABILITY_CALLBACK_PROXY_H

#include "iremote_proxy.h"
#include "irequest_start_ability_callback.h"

namespace OHOS {
namespace AAFwk {
class RequestStartAbilityCallbackProxy : public IRemoteProxy<IRequestStartAbilityCallback> {
public:
    explicit RequestStartAbilityCallbackProxy(const sptr<IRemoteObject> &impl);
    virtual ~RequestStartAbilityCallbackProxy() = default;

    void OnRequestStartAbilityResult(bool result);

private:
    static inline BrokerDelegator<RequestStartAbilityCallbackProxy> delegator_;
};

}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_REQUEST_START_ABILITY_CALLBACK_PROXY_H