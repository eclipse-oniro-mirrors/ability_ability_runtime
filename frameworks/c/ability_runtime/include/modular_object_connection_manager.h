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

#ifndef OHOS_ABILITY_RUNTIME_MODULAR_OBJECT_CONNECTION_MANAGER_H
#define OHOS_ABILITY_RUNTIME_MODULAR_OBJECT_CONNECTION_MANAGER_H

#include <map>
#include <mutex>
#include <vector>

#include "ability_connect_callback.h"
#include "errors.h"
#include "iremote_object.h"
#include "modular_object_ability_connection.h"
#include "operation.h"
#include "want.h"

namespace OHOS {
namespace AbilityRuntime {

/**
 * @brief Connection info key for ModularObjectExtension.
 * Key consists of: abilityConnection + connectReceiver.
 */
struct ModularObjectConnectionInfo {
    sptr<ModularObjectAbilityConnection> abilityConnection;
    AAFwk::Operation connectReceiver;

    ModularObjectConnectionInfo(const sptr<ModularObjectAbilityConnection> &connection,
        const AAFwk::Operation &receiver)
        : abilityConnection(connection), connectReceiver(receiver)
    {}

    bool operator<(const ModularObjectConnectionInfo &that) const
    {
        if (abilityConnection < that.abilityConnection) {
            return true;
        }
        if (connectReceiver.GetBundleName() < that.connectReceiver.GetBundleName()) {
            return true;
        }
        if (connectReceiver.GetBundleName() == that.connectReceiver.GetBundleName() &&
            connectReceiver.GetModuleName() < that.connectReceiver.GetModuleName()) {
            return true;
        }
        if (connectReceiver.GetBundleName() == that.connectReceiver.GetBundleName() &&
            connectReceiver.GetModuleName() == that.connectReceiver.GetModuleName() &&
            connectReceiver.GetAbilityName() < that.connectReceiver.GetAbilityName()) {
            return true;
        }
        return false;
    }
};

/**
 * @brief Manages connections to ModularObjectExtension instances.
 *
 * Key difference from ConnectionManager: NO connection reuse.
 * Each connect call creates a new connection.
 */
class ModularObjectConnectionManager {
public:
    ~ModularObjectConnectionManager() = default;
    ModularObjectConnectionManager(const ModularObjectConnectionManager &) = delete;
    ModularObjectConnectionManager &operator=(const ModularObjectConnectionManager &) = delete;

    static ModularObjectConnectionManager &GetInstance();

    /**
     * @brief Connect to ModularObjectExtension - always creates new connection.
     * @param want The Want containing target ability info.
     * @param callback The connection callback.
     */
    ErrCode ConnectModularObjectExtension(const AAFwk::Want &want,
        const sptr<AbilityConnectCallback> &callback);

    /**
     * @brief Disconnect from ModularObjectExtension by callback.
     * @param callback The connection callback to disconnect.
     */
    ErrCode DisconnectModularObjectExtension(const sptr<AbilityConnectCallback> &callback);

    /**
     * @brief Remove connection record by connection object.
     * @param connection The connection to remove.
     * @return true if removed, false otherwise.
     */
    bool RemoveConnection(const sptr<ModularObjectAbilityConnection> &connection);

    /**
     * @brief Check if service exists and disconnect if not found.
     * @param element The service element name.
     * @param connection The connection to check.
     * @return true if service does not exist and was disconnected, false if found.
     */
    bool DisconnectNonexistentService(const AppExecFwk::ElementName &element,
        const sptr<ModularObjectAbilityConnection> &connection);

private:
    ModularObjectConnectionManager() = default;

    std::mutex connectionMutex_;
    std::map<ModularObjectConnectionInfo, std::vector<sptr<AbilityConnectCallback>>> connectionRecords_;
};

} // namespace AbilityRuntime
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_MODULAR_OBJECT_CONNECTION_MANAGER_H
