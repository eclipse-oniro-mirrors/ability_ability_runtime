/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_ABILITY_CONNECTION_H
#define OHOS_ABILITY_RUNTIME_ABILITY_CONNECTION_H

#include <atomic>
#include <mutex>
#include "ability_connect_callback.h"
#include "ability_connect_callback_stub.h"
#include "want.h"
#include "element_name.h"

#include "iremote_broker.h"

namespace OHOS {
namespace AbilityRuntime {
enum {
    CONNECTION_STATE_DISCONNECTED = -1,

    CONNECTION_STATE_CONNECTED = 0,

    CONNECTION_STATE_CONNECTING = 1
};
class AbilityConnection : public AAFwk::AbilityConnectionStub {
public:
    /**
     * @brief Constructor.
     *
     */
    AbilityConnection() = default;

    /**
     * @brief Destructor.
     *
     */
    ~AbilityConnection() = default;

    /**
     * OnAbilityConnectDone, AbilityMs notify caller ability the result of connect.
     *
     * @param element, service ability's ElementName.
     * @param remoteObject, the session proxy of service ability.
     * @param resultCode, ERR_OK on success, others on failure.
     */
    void OnAbilityConnectDone(
        const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject, int resultCode) override;

    /**
     * OnAbilityDisconnectDone, AbilityMs notify caller ability the result of disconnect.
     *
     * @param element, service ability's ElementName.
     * @param resultCode, ERR_OK on success, others on failure.
     */
    void OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode)  override;

    /**
     * add abilityConnectCallback
     *
     * @param abilityConnectCallback is used to notify caller ability that connect or disconnect is complete
    */
    void AddConnectCallback(const sptr<AbilityConnectCallback> &abilityConnectCallback);

    void RemoveConnectCallback(const sptr<AbilityConnectCallback> &abilityConnectCallback);

    void SetRemoteObject(const sptr<IRemoteObject> &remoteObject);

    void SetResultCode(int resultCode);

    void SetConnectionState(int connectionState);

    sptr<IRemoteObject> GetRemoteObject() const;

    int GetResultCode() const;

    int GetConnectionState() const;

    std::vector<sptr<AbilityConnectCallback>> GetCallbackList();
private:
    std::vector<sptr<AbilityConnectCallback>> abilityConnectCallbackList_;
    sptr<IRemoteObject> remoteObject_ = nullptr;
    std::atomic<int> resultCode_ = -1;
    std::atomic<int> connectionState_ = CONNECTION_STATE_DISCONNECTED;
    std::mutex mutex_;
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_ABILITY_CONNECTION_H
