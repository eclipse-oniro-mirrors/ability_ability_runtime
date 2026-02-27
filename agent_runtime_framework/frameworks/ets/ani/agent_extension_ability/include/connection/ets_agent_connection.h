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

#ifndef OHOS_AGENT_RUNTIME_ETS_AGENT_CONNECTION_H
#define OHOS_AGENT_RUNTIME_ETS_AGENT_CONNECTION_H

#include <map>
#include <memory>
#include <mutex>
#include <vector>

#include "ability_connect_callback.h"
#include "ani.h"
#include "want.h"

namespace OHOS {
namespace AbilityRuntime {
class ETSAbilityConnection;
}

namespace AgentRuntime {

struct ConnectionKey {
    AAFwk::Want want;
    int64_t id;
};

struct KeyCompare {
    bool operator()(const ConnectionKey &key1, const ConnectionKey &key2) const
    {
        if (key1.id < key2.id) {
            return true;
        }
        return false;
    }
};

class EtsAgentConnection;

namespace AgentConnectionUtils {
/**
 * Remove agent connection from registry.
 *
 * @param connectId The connection ID to remove.
 */
void RemoveAgentConnection(int64_t connectId);

/**
 * Insert agent connection into registry.
 *
 * @param connection The connection object to insert.
 * @param want The want information for the connection.
 * @return Returns the connection ID.
 */
int64_t InsertAgentConnection(sptr<EtsAgentConnection> connection, const AAFwk::Want &want);

/**
 * Find agent connection by ID.
 *
 * @param connectId The connection ID to find.
 * @param connection Output parameter for the found connection.
 */
void FindAgentConnection(int64_t connectId, sptr<EtsAgentConnection> &connection);

/**
 * Find agent connection by want and callback.
 *
 * @param env The ANI environment.
 * @param want The want information to match.
 * @param callback The callback object to match.
 * @param connection Output parameter for the found connection.
 */
void FindAgentConnection(ani_env *env, AAFwk::Want &want, ani_object callback,
    sptr<EtsAgentConnection> &connection);
}

class EtsAgentConnectorStubImpl;

/**
 * @class EtsAgentConnection
 * Connection class for agent extension that bridges ETS callbacks and native IPC.
 * Manages the connection lifecycle between ETS and Agent Extension.
 */
class EtsAgentConnection : public AbilityRuntime::AbilityConnectCallback {
public:
    /**
     * Constructor.
     *
     * @param etsVm The ANI VM pointer.
     */
    explicit EtsAgentConnection(ani_vm *etsVm);

    /**
     * Destructor.
     */
    ~EtsAgentConnection() override;

    /**
     * Called when agent extension connection succeeds.
     * Implements ETSAbilityConnection interface.
     *
     * @param element The element name of the connected extension.
     * @param remoteObject The remote object for IPC communication.
     * @param resultCode The result code of the connection.
     */
    void OnAbilityConnectDone(
        const AppExecFwk::ElementName &element,
        const sptr<IRemoteObject> &remoteObject,
        int resultCode) override;

    /**
     * Called when agent extension connection fails or disconnects.
     * Implements ETSAbilityConnection interface.
     *
     * @param element The element name of the extension.
     * @param resultCode The result code of disconnection.
     */
    void OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode) override;

    /**
     * Get the service host stub.
     *
     * @return Returns the service host stub.
     */
    sptr<EtsAgentConnectorStubImpl> GetServiceHostStub() { return serviceHostStub_; }

    /**
     * Set the proxy object returned to ETS.
     *
     * @param proxy The ANI object of the proxy object.
     */
    void SetProxyObject(ani_object proxy);

    /**
     * Get the proxy object.
     *
     * @return Returns the ANI reference of the proxy object.
     */
    ani_ref GetProxyObject();

    /**
     * Set the ANI async callback for promise resolution.
     *
     * @param asyncCallback The callback object from ETS.
     */
    void SetAniAsyncCallback(ani_object asyncCallback);

    /**
     * Add a duplicated pending callback.
     * Used when multiple connection requests are made for the same extension.
     *
     * @param duplicatedCallback The callback object from ETS.
     */
    void AddDuplicatedPendingCallback(ani_object duplicatedCallback);

    /**
     * Resolve all duplicated pending callbacks with the proxy.
     *
     * @param env The ANI environment.
     * @param proxyObj The proxy object to resolve with.
     */
    void ResolveDuplicatedPendingCallbacks(ani_env *env, ani_object proxyObj);

    /**
     * Reject all duplicated pending callbacks with error.
     *
     * @param env The ANI environment.
     * @param error The error code.
     */
    void RejectDuplicatedPendingCallbacks(ani_env *env, int32_t error);

    /**
     * Called when agent extension sends data.
     * Schedules async task to call ETS onData callback.
     *
     * @param data The string data received from agent extension.
     * @return Returns 0 on success, error code otherwise.
     */
    int32_t OnSendData(const std::string &data);

    /**
     * Handle the data received from agent extension.
     * Called on ETS thread to invoke the onData callback.
     *
     * @param data The string data received.
     */
    void HandleOnSendData(const std::string &data);

    /**
     * Called when agent extension sends authorization.
     * Schedules async task to call ETS onAuth callback.
     *
     * @param data The authorization string received from agent extension.
     * @return Returns 0 on success, error code otherwise.
     */
    int32_t OnAuthorize(const std::string &data);

    /**
     * Handle the authorization received from agent extension.
     * Called on ETS thread to invoke the onAuth callback.
     *
     * @param data The authorization string received.
     */
    void HandleOnAuthorize(const std::string &data);

    /**
     * Set the ETS connection callback.
     *
     * @param callback The ETS connection callback object.
     */
    void SetEtsConnectionCallback(ani_object callback);

    /**
     * Get the ETS connection callback object.
     *
     * @return Returns the ETS connection callback reference.
     */
    ani_ref GetEtsConnectionObject() { return etsConnectionObject_; }

    /**
     * Remove the connection object.
     */
    void RemoveConnectionObject();

    /**
     * Check if two ETS callback objects are equal.
     *
     * @param env The ANI environment.
     * @param callback The first callback reference.
     * @param value The second callback object.
     * @return Returns true if they are equal, false otherwise.
     */
    static bool IsEtsCallbackObjectEquals(ani_env *env, ani_ref callback, ani_object value);

    /**
     * Release an object reference.
     *
     * @param env The ANI environment.
     * @param etsObjRef The reference to release.
     */
    void ReleaseObjectReference(ani_env *env, ani_ref etsObjRef);

    void SetConnectionId(int32_t id) { connectionId_ = id; }

    int32_t GetConnectionId() { return connectionId_; }

private:
    /**
     * Call an object method in ETS.
     *
     * @param env The ANI environment.
     * @param methodName The method name.
     * @param signature The method signature.
     */
    void CallObjectMethod(ani_env *env, const char *methodName, const char *signature, ...);

    /**
     * Called when agent extension connection succeeds.
     * Implements ETSAbilityConnection interface.
     *
     * @param element The element name of the connected extension.
     * @param remoteObject The remote object for IPC communication.
     * @param resultCode The result code of the connection.
     */
    void HandleOnAbilityConnectDone(
        const AppExecFwk::ElementName &element,
        const sptr<IRemoteObject> &remoteObject,
        int resultCode);

    /**
     * Called when agent extension connection fails or disconnects.
     * Implements ETSAbilityConnection interface.
     *
     * @param element The element name of the extension.
     * @param resultCode The result code of disconnection.
     */
    void HandleOnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode);

    void ReleaseObjectReference(ani_ref etsObjRef);

private:
    ani_vm *etsVm_ = nullptr;

    int64_t connectionId_ = -1;

    /**
     * The service host stub for handling incoming IPC from agent extension.
     */
    sptr<EtsAgentConnectorStubImpl> serviceHostStub_;

    /**
     * The async callback stored from ETS for promise resolution.
     */
    ani_ref aniAsyncCallback_ = nullptr;

    /**
     * The ETS connection callback object.
     */
    ani_ref etsConnectionObject_ = nullptr;

    /**
     * The proxy object returned to ETS.
     */
    ani_ref serviceProxyObject_ = nullptr;

    /**
     * List of pending callbacks for duplicate connection requests.
     */
    std::vector<ani_ref> duplicatedPendingCallbacks_;
};
} // namespace AgentRuntime
} // namespace OHOS

#endif // OHOS_AGENT_RUNTIME_ETS_AGENT_CONNECTION_H
