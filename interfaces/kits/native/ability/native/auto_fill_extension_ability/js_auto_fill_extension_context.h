/*
 * Copyright (c) 2023-2026 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_JS_AUTO_FILL_EXTENSION_CONTEXT_H
#define OHOS_ABILITY_RUNTIME_JS_AUTO_FILL_EXTENSION_CONTEXT_H

#include <memory>

#include "ability_connect_callback.h"
#include "auto_fill_extension_context.h"
#include "native_engine/native_engine.h"

namespace OHOS {
namespace AbilityRuntime {
struct NapiCallbackInfo;
class JsAutoFillExtensionContext final {
public:
    explicit JsAutoFillExtensionContext(const std::shared_ptr<AutoFillExtensionContext> &context) : context_(context) {}
    virtual ~JsAutoFillExtensionContext() = default;

    static void Finalizer(napi_env env, void *data, void *hint);
    static napi_value ReloadInModal(napi_env env, napi_callback_info info);
    static napi_value ConnectAbility(napi_env env, napi_callback_info info);
    static napi_value DisconnectAbility(napi_env env, napi_callback_info info);
    static napi_value CreateJsAutoFillExtensionContext(
        napi_env env, const std::shared_ptr<AutoFillExtensionContext> &context);
    std::shared_ptr<AutoFillExtensionContext> GetAutoFillExtensionContext()
    {
        return context_.lock();
    }

private:
    napi_value OnReloadInModal(napi_env env, NapiCallbackInfo &info);
    napi_value OnConnectAbility(napi_env env, NapiCallbackInfo &info);
    napi_value OnDisconnectAbility(napi_env env, NapiCallbackInfo &info);
    std::weak_ptr<AutoFillExtensionContext> context_;
};

class JSAutoFillExtensionConnection : public AbilityConnectCallback {
public:
    explicit JSAutoFillExtensionConnection(napi_env env);
    ~JSAutoFillExtensionConnection() override;
    void OnAbilityConnectDone(
        const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject, int resultCode) override;
    void OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode) override;
    void HandleOnAbilityConnectDone(
        const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject, int resultCode);
    void HandleOnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode);
    void SetJsConnectionObject(napi_value jsConnectionObject);
    std::unique_ptr<NativeReference> &GetJsConnectionObject()
    {
        return jsConnectionObject_;
    }
    void RemoveConnectionObject();
    void CallJsFailed(int32_t errorCode);
    void SetConnectionId(int64_t id);
    int64_t GetConnectionId() const
    {
        return connectionId_;
    }

private:
    void ReleaseNativeReference(NativeReference *ref);

    napi_env env_ = nullptr;
    std::unique_ptr<NativeReference> jsConnectionObject_ = nullptr;
    int64_t connectionId_ = -1;
};

struct AutoFillExtensionConnectionKey {
    AAFwk::Want want;
    int64_t id;
};

struct AutoFillKeyCompare {
    bool operator()(const AutoFillExtensionConnectionKey &key1, const AutoFillExtensionConnectionKey &key2) const
    {
        return key1.id < key2.id;
    }
};

} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_JS_AUTO_FILL_EXTENSION_CONTEXT_H
