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

#ifndef OHOS_AGENT_RUNTIME_ETS_AGENT_EXTENSION_H
#define OHOS_AGENT_RUNTIME_ETS_AGENT_EXTENSION_H

#include "agent_extension.h"
#include "ani.h"
#include "configuration.h"
#include "ets_agent_extension_stub_impl.h"
#include "ets_native_reference.h"


namespace OHOS {
namespace AbilityRuntime {
class Runtime;
class ETSRuntime;
}

namespace AgentRuntime {
using namespace OHOS::AbilityRuntime;
class AgentExtensionContext;
class EtsAgentExtension;
class EtsAgentExtensionStubImpl;

/**
 * @brief Basic agent extension components for ETS.
 */
class EtsAgentExtension : public AgentExtension {
public:
    explicit EtsAgentExtension(ETSRuntime& etsRuntime);
    virtual ~EtsAgentExtension() override;

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
     * @brief Called when this extension is started.
     *
     * @param Want Indicates the {@link Want} structure containing startup information about the extension.
     */
    virtual void OnStart(const AAFwk::Want &want) override;

    /**
     * @brief Called when this service extension is connected for the first time.
     *
     * @param want Indicates the {@link Want} structure containing connection information.
     * @param callbackInfo Indicates the lifecycle transaction callback information.
     * @param isAsyncCallback Indicates whether it is an asynchronous lifecycle callback.
     * @return Returns a pointer to the <b>sid</b> of the connected Service extension.
     */
    virtual sptr<IRemoteObject> OnConnect(const AAFwk::Want &want,
        AppExecFwk::AbilityTransactionCallbackInfo<sptr<IRemoteObject>> *callbackInfo, bool &isAsyncCallback) override;

    /**
     * @brief Called when all abilities connected to this service extension are disconnected.
     *
     * @param want Indicates the {@link Want} structure containing connection information.
     * @param callbackInfo Indicates the lifecycle transaction callback information.
     * @param isAsyncCallback Indicates whether it is an asynchronous lifecycle callback.
     */
    virtual void OnDisconnect(const AAFwk::Want &want, AppExecFwk::AbilityTransactionCallbackInfo<> *callbackInfo,
        bool &isAsyncCallback) override;

    /**
     * @brief Called when this extension enters the <b>STATE_STOP</b> state.
     */
    virtual void OnStop() override;

    /**
     * @brief Called when client send data to extension.
     *
     * @param hostProxy the proxy used to send data back to client.
     * @param data The data to send.
     */
    int32_t OnSendData(const sptr<IRemoteObject> &hostProxy, const std::string &data);

    /**
     * @brief Called when client authorizes to extension.
     *
     * @param hostProxy the proxy used to authorizes back to client.
     * @param data The data to send.
     */
    int32_t OnAuthorize(const sptr<IRemoteObject> &hostProxy, const std::string &data);

private:
    void CallObjectMethod(const char *name, const char *signature, ...);

    void BindContext(ani_env *env);

    void GetSrcPath(std::string &srcPath);

    ani_object CreateETSContext(ani_env *env, std::shared_ptr<AgentExtensionContext> context);

    void HandleSendData(sptr<IRemoteObject> hostProxy, const std::string &data);

    void HandleAuthorize(sptr<IRemoteObject> hostProxy, const std::string &data);

    sptr<IRemoteObject> GetHostProxyFromWant(const AAFwk::Want &want);

    ETSRuntime& etsRuntime_;
    std::unique_ptr<ETSNativeReference> etsObj_;
    std::shared_ptr<AbilityContext> aContext_ = nullptr;
    std::shared_ptr<ETSNativeReference> shellContextRef_ = nullptr;
    std::shared_ptr<AbilityHandler> handler_ = nullptr;
    sptr<EtsAgentExtensionStubImpl> extensionStub_;
    std::map<sptr<IRemoteObject>, ani_ref> hostProxyMap_;
    ani_vm *etsVm_ = nullptr;
};
}  // namespace AgentRuntime
}  // namespace OHOS
#endif  // OHOS_AGENT_RUNTIME_ETS_AGENT_EXTENSION_H
