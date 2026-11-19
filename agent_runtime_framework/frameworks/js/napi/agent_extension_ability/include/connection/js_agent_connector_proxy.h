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

#ifndef OHOS_AGENT_RUNTIME_JS_AGENT_CONNECTOR_PROXY_H
#define OHOS_AGENT_RUNTIME_JS_AGENT_CONNECTOR_PROXY_H

#include <memory>

#include "iagent_connector.h"
#include "js_runtime_utils.h"

namespace OHOS {
namespace AgentRuntime {
using namespace AbilityRuntime;

/**
 * @class JsAgentConnectorProxy
 * JavaScript proxy class for Agent Connector.
 * Binds to JS class AgentConnectorProxy, used on agent extension side
 * to send callbacks from agent to host application (connector).
 */
class JsAgentConnectorProxy {
public:
    /**
     * Create a JavaScript connector proxy object.
     *
     * @param env The N-API environment.
     * @param connectorProxy The remote object of the connector proxy for callbacks.
     * @return Returns the N-API reference of the created proxy object.
     */
    static napi_ref CreateJsAgentConnectorProxy(napi_env env, const sptr<IRemoteObject> &connectorProxy);

    /**
     * Finalizer for the connector proxy object.
     *
     * @param env The N-API environment.
     * @param data The pointer to the JsAgentConnectorProxy instance.
     * @param hint The hint data.
     */
    static void Finalizer(napi_env env, void *data, void *hint);

    /**
     * Constructor.
     *
     * @param connectorProxy The remote object of the connector proxy.
     */
    explicit JsAgentConnectorProxy(const sptr<IRemoteObject> &connectorProxy);

    /**
     * Destructor.
     */
    virtual ~JsAgentConnectorProxy();

    /**
     * Invalidate this proxy after the connection is disconnected.
     */
    void Invalidate();

private:
    /**
     * N-API callback for sendData method.
     *
     * @param env The N-API environment.
     * @param info The N-API callback info.
     * @return Returns the N-API value (undefined).
     */
    static napi_value SendData(napi_env env, napi_callback_info info);

    /**
     * N-API callback for authorize method.
     *
     * @param env The N-API environment.
     * @param info The N-API callback info.
     * @return Returns the N-API value (undefined).
     */
    static napi_value Authorize(napi_env env, napi_callback_info info);

    /**
     * Implementation of sendData method.
     *
     * @param env The N-API environment.
     * @param info The N-API callback info.
     * @return Returns the N-API value (undefined).
     */
    napi_value OnSendData(napi_env env, NapiCallbackInfo &info);

    /**
     * Implementation of authorize method.
     *
     * @param env The N-API environment.
     * @param info The N-API callback info.
     * @return Returns the N-API value (undefined).
     */
    napi_value OnAuthorize(napi_env env, NapiCallbackInfo &info);

protected:
    /**
     * The native proxy for IPC communication with host application.
     */
    sptr<IAgentConnector> proxy_ = nullptr;
    bool isDisconnected_ = false;
};

} // namespace AgentRuntime
} // namespace OHOS

#endif // OHOS_AGENT_RUNTIME_JS_AGENT_CONNECTOR_PROXY_H
