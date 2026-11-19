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

#ifndef OHOS_ABILITY_RUNTIME_I_REQUEST_START_ABILITY_CALLBACK_H
#define OHOS_ABILITY_RUNTIME_I_REQUEST_START_ABILITY_CALLBACK_H

#include "iremote_broker.h"
#include "iremote_object.h"

namespace OHOS {
namespace AAFwk {

class IRequestStartAbilityCallback : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.aafwk.IRequestStartAbilityCallback");

    /**
     * @brief Called when the request to start UIAbility has been processed.
     * @param result true if the request was successfully notified to SceneBoard; false otherwise.
     */
    virtual void OnRequestStartAbilityResult(bool result) = 0;

    enum {
        ON_REQUEST_START_ABILITY_RESULT = 0,
    };
};

} // namespace AAFwk
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_I_REQUEST_START_ABILITY_CALLBACK_H