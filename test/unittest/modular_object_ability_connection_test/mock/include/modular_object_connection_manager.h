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

#ifndef MOCK_MODULAR_OBJECT_CONNECTION_MANAGER_H
#define MOCK_MODULAR_OBJECT_CONNECTION_MANAGER_H

#include "element_name.h"
#include "modular_object_ability_connection.h"
#include "refbase.h"

namespace OHOS {
namespace AbilityRuntime {

class ModularObjectConnectionManager {
public:
    static ModularObjectConnectionManager &GetInstance()
    {
        static ModularObjectConnectionManager instance;
        return instance;
    }

    bool DisconnectNonexistentService(const AppExecFwk::ElementName &element,
        const sptr<ModularObjectAbilityConnection> &connection)
    {
        return g_disconnectNonexistentResult;
    }

    bool RemoveConnection(const sptr<ModularObjectAbilityConnection> &connection)
    {
        g_removeConnectionCalled = true;
        return true;
    }

    static bool g_disconnectNonexistentResult;
    static bool g_removeConnectionCalled;

    static void Reset()
    {
        g_disconnectNonexistentResult = false;
        g_removeConnectionCalled = false;
    }

private:
    ModularObjectConnectionManager() = default;
};

} // namespace AbilityRuntime
} // namespace OHOS

#endif // MOCK_MODULAR_OBJECT_CONNECTION_MANAGER_H
