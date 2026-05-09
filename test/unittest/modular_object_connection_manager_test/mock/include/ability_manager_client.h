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

#ifndef MOCK_ABILITY_MANAGER_CLIENT_H
#define MOCK_ABILITY_MANAGER_CLIENT_H

#include <memory>
#include "errors.h"
#include "refbase.h"
#include "iremote_object.h"

namespace OHOS {
namespace AppExecFwk {
enum ExtensionAbilityType {
    MODULAR_OBJECT = 99,
};
} // namespace AppExecFwk

namespace AAFwk {
class Want;

constexpr int32_t DEFAULT_INVAL_VALUE = -1;
constexpr int CONNECTION_NOT_EXIST = 2097162;

class AbilityManagerClient {
public:
    static std::shared_ptr<AbilityManagerClient> GetInstance()
    {
        static auto instance = std::make_shared<AbilityManagerClient>();
        return instance;
    }

    template<typename T>
    ErrCode ConnectAbilityWithExtensionType(const AAFwk::Want &want,
        const sptr<T> &connect, const sptr<IRemoteObject> &callerToken,
        int32_t userId, int32_t extensionType)
    {
        g_connectCalled = true;
        return g_connectResult;
    }

    template<typename T>
    ErrCode DisconnectAbility(const sptr<T> &connect)
    {
        g_disconnectCalled = true;
        return g_disconnectResult;
    }

    static bool g_connectCalled;
    static bool g_disconnectCalled;
    static ErrCode g_connectResult;
    static ErrCode g_disconnectResult;

    static void Reset()
    {
        g_connectCalled = false;
        g_disconnectCalled = false;
        g_connectResult = ERR_OK;
        g_disconnectResult = ERR_OK;
    }
};

} // namespace AAFwk
} // namespace OHOS

#endif // MOCK_ABILITY_MANAGER_CLIENT_H
