/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef MOCK_OHOS_ABILITY_RUNTIME_ABILITY_CONNECT_CALLBACK_H
#define MOCK_OHOS_ABILITY_RUNTIME_ABILITY_CONNECT_CALLBACK_H

#include "iremote_broker.h"
#include "element_name.h"
#include "refbase.h"

namespace OHOS {
namespace AbilityRuntime {
class AbilityConnectCallback : public RefBase {
public:
    virtual void OnAbilityConnectDone(const AppExecFwk::ElementName &element,
                           const sptr<IRemoteObject> &remoteObject, int resultCode) {}
    virtual void OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode) {}
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // MOCK_OHOS_ABILITY_RUNTIME_ABILITY_CONNECT_CALLBACK_H
