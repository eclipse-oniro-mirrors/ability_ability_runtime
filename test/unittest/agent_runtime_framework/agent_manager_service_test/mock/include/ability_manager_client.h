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

#ifndef OHOS_ABILITY_RUNTIME_ABILITY_MANAGER_CLIENT_H
#define OHOS_ABILITY_RUNTIME_ABILITY_MANAGER_CLIENT_H

#include <memory>

#include "ability_connect_callback_interface.h"
#include "ability_manager_errors.h"
#include "extension_ability_info.h"
#include "iremote_object.h"
#include "want.h"

namespace OHOS {
namespace AAFwk {
const int DEFAULT_INVAL_VALUE = -1;

class AbilityManagerClient {
public:
    static std::shared_ptr<AbilityManagerClient> GetInstance();

    ErrCode ConnectAbilityWithExtensionType(const Want &want, sptr<IAbilityConnection> connect,
        sptr<IRemoteObject> callerToken, int32_t userId, AppExecFwk::ExtensionAbilityType extensionType);

    ErrCode DisconnectAbility(sptr<IAbilityConnection> connect);

    ErrCode AttributeTaskForA2AReusedConnectionRecord(const Want &want, sptr<IAbilityConnection> connect);
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_ABILITY_MANAGER_CLIENT_H
