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

#ifndef OHOS_AGENT_RUNTIME_JS_AGENT_EXTENSION_H
#define OHOS_AGENT_RUNTIME_JS_AGENT_EXTENSION_H

#include "agent_extension.h"

namespace OHOS {
namespace AbilityRuntime {
    class Runtime;
    class JsRuntime;
}

namespace AgentRuntime {
using namespace OHOS::AbilityRuntime;
class AgentExtension;
class AgentExtensionContext;
class JsAgentExtension;
class JsAgentExtensionStubImpl;

/**
 * @brief Basic service components.
 */
class JsAgentExtension : public AgentExtension {
public:
    explicit JsAgentExtension(JsRuntime& jsRuntime);
    virtual ~JsAgentExtension() override;

    /**
     * @brief Init the extension.
     *
     * @param record the extension record.
     * @param application the application info.
     * @param handler the extension handler.
     * @param token the remote token.
     */
    virtual void Init(const std::shared_ptr<AppExecFwk::AbilityLocalRecord> &record,
        const std::shared_ptr<AppExecFwk::OHOSApplication> &application,
        std::shared_ptr<AppExecFwk::AbilityHandler> &handler,
        const sptr<IRemoteObject> &token) override;

    /**
     * @brief Called when this extension is started. You must override this function if you want to perform some
     *        initialization operations during extension startup.
     *
     * This function can be called only once in the entire lifecycle of an extension.
     * @param Want Indicates the {@link Want} structure containing startup information about the extension.
     */
    virtual void OnStart(const AAFwk::Want &want) override;

    /**
     * @brief Called when this Service extension is connected for the first time.
     *
     * You can override this function to implement your own processing logic.
     *
     * @param want Indicates the {@link Want} structure containing connection information about the Service extension.
     * @param callbackInfo Indicates the lifecycle transaction callback information.
     * @param isAsyncCallback Indicates whether it is an asynchronous lifecycle callback.
     * @return Returns a pointer to the <b>sid</b> of the connected Service extension.
     */
    virtual sptr<IRemoteObject> OnConnect(const AAFwk::Want &want,
        AppExecFwk::AbilityTransactionCallbackInfo<sptr<IRemoteObject>> *callbackInfo, bool &isAsyncCallback) override;

    /**
     * @brief Called when all abilities connected to this Service extension are disconnected.
     *
     * You can override this function to implement your own processing logic.
     * @param want Indicates the {@link Want} structure containing connection information about the Service extension.
     * @param callbackInfo Indicates the lifecycle transaction callback information.
     * @param isAsyncCallback Indicates whether it is an asynchronous lifecycle callback.
     */
    virtual void OnDisconnect(const AAFwk::Want &want, AppExecFwk::AbilityTransactionCallbackInfo<> *callbackInfo,
        bool &isAsyncCallback) override;

    /**
     * @brief Called when this extension enters the <b>STATE_STOP</b> state.
     *
     * The extension in the <b>STATE_STOP</b> is being destroyed.
     * You can override this function to implement your own processing logic.
     */
    virtual void OnStop() override;

    /**
     * @brief Called when client send data to extension.
     *
     * @param hostProxy the proxy used to send data back to client
     * @param data The data to send.
     */
    int32_t OnSendData(const sptr<IRemoteObject> &hostProxy, const std::string &data);

    /**
     * @brief Called when client authorizes to extension.
     *
     * @param hostProxy the proxy used to authorizes back to client
     * @param data The data to send.
     */
    int32_t OnAuthorize(const sptr<IRemoteObject> &hostProxy, const std::string &data);

    /**
     * @brief Called when a low-code agent is invoked.
     *
     * @param agentId The low-code agent id.
     * @return Returns 0 on success, error code otherwise.
     */
    int32_t OnAgentInvoked(const std::string &agentId);

private:
    napi_value CallObjectMethod(const char* name, napi_value const *argv = nullptr, size_t argc = 0);

    void BindContext(napi_env env, napi_value obj, std::shared_ptr<AAFwk::Want> want);

    void GetSrcPath(std::string &srcPath);

    napi_value WrapWant(napi_env env, const AAFwk::Want &want);

    void HandleSendData(sptr<IRemoteObject> hostProxy, const std::string &data);

    void HandleAuthorize(sptr<IRemoteObject> hostProxy, const std::string &data);

    void HandleAgentInvoked(const std::string &agentId);

    sptr<IRemoteObject> GetHostProxyFromWant(const AAFwk::Want &want);

    JsRuntime& jsRuntime_;
    std::unique_ptr<NativeReference> jsObj_;
    std::shared_ptr<NativeReference> shellContextRef_ = nullptr;
    sptr<JsAgentExtensionStubImpl> extensionStub_;
    std::map<sptr<IRemoteObject>, std::unique_ptr<NativeReference>> hostProxyMap_;
};
}  // namespace AgentRuntime
}  // namespace OHOS
#endif  // OHOS_AGENT_RUNTIME_JS_AGENT_EXTENSION_H
