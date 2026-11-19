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

#ifndef OHOS_ABILITY_RUNTIME_C_MODULAR_OBJECT_CONNECTION_CALLBACK_H
#define OHOS_ABILITY_RUNTIME_C_MODULAR_OBJECT_CONNECTION_CALLBACK_H

#include <map>
#include <mutex>

#include "ability_connect_callback.h"
#include "connect_options_impl.h"
namespace OHOS {
namespace AbilityRuntime {

struct ModularObjectConnectionKey {
    int64_t id;
};

struct ModularObjectConnectionKeyCompare {
    bool operator()(const ModularObjectConnectionKey &key1, const ModularObjectConnectionKey &key2) const
    {
        return key1.id < key2.id;
    }
};

class CModularObjectConnectionCallback;

namespace CModularObjectConnectionUtils {
/**
 * @brief Insert connection callback into global registry.
 * @param callback The callback object to insert.
 * @return Returns the connection ID.
 */
int64_t InsertConnection(sptr<CModularObjectConnectionCallback> callback);

/**
 * @brief Remove connection callback from global registry.
 * @param connectionId The connection ID to remove.
 */
void RemoveConnectionCallback(int64_t connectionId);

/**
 * @brief Find connection callback by ID.
 * @param connectionId The connection ID to find.
 * @param callback Output parameter for the found callback.
 */
void FindConnection(int64_t connectionId, sptr<CModularObjectConnectionCallback> &callback);
} // namespace CModularObjectConnectionUtils

/**
 * @brief C connection callback for ModularObjectExtension.
 * Handles OnAbilityConnectDone/OnAbilityDisconnectDone and invokes user C callbacks.
 */
class CModularObjectConnectionCallback : public AbilityConnectCallback {
public:
    CModularObjectConnectionCallback(
        const std::shared_ptr<OH_AbilityRuntime_ConnectOptionsState> &state);
    ~CModularObjectConnectionCallback() override = default;

    void OnAbilityConnectDone(const AppExecFwk::ElementName &element,
        const sptr<IRemoteObject> &remoteObject, int resultCode) override;
    void OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode) override;

    void SetConnectionId(int64_t id) { connectionId_ = id; }
    int64_t GetConnectionId() const { return connectionId_; }

private:
    int64_t connectionId_ = 0;
    std::weak_ptr<OH_AbilityRuntime_ConnectOptionsState> state_;
};

} // namespace AbilityRuntime
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_C_MODULAR_OBJECT_CONNECTION_CALLBACK_H
