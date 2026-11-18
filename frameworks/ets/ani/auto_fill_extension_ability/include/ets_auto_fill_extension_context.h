/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_ETS_AUTO_FILL_EXTENSION_CONTEXT_H
#define OHOS_ABILITY_RUNTIME_ETS_AUTO_FILL_EXTENSION_CONTEXT_H

#include "ani.h"
#include "ability_connect_callback.h"
#include "auto_fill_extension_context.h"

namespace OHOS {
namespace AbilityRuntime {
class EtsAutoFillExtensionContext final {
public:
    explicit EtsAutoFillExtensionContext(std::shared_ptr<AutoFillExtensionContext> context)
        : context_(context) {}
    virtual ~EtsAutoFillExtensionContext() = default;

    static ani_object SetEtsAutoFillExtensionContext(ani_env *env, std::shared_ptr<AutoFillExtensionContext> context);
    static EtsAutoFillExtensionContext *GetEtsAutoFillExtensionContext(ani_env *env, ani_object object);
    static void Clean(ani_env *env, ani_object object);
    static void ReloadInModal(ani_env *env, ani_object object, ani_object customDataObj, ani_object callback);
    static ani_long ConnectServiceExtensionAbility(ani_env *env, ani_object aniObj, ani_object wantObj,
        ani_object connectOptionsObj);
    static void DisconnectServiceExtensionAbility(ani_env *env, ani_object aniObj, ani_long connectId,
        ani_object callback);
    static void CheckConnectionAbility(ani_env *env, ani_object aniObj);
    static ani_object CreateEtsAutoFillExtensionContext(ani_env *env,
        std::shared_ptr<AutoFillExtensionContext> context);

private:
    void OnReloadInModal(ani_env *env, ani_object object, ani_object customDataObj, ani_object callback);
    ani_long OnConnectServiceExtensionAbility(ani_env *env, ani_object aniObj, ani_object wantObj,
        ani_object connectOptionsObj);
    void OnDisconnectServiceExtensionAbility(ani_env *env, ani_object aniObj, ani_long connectId,
        ani_object callback);
    std::weak_ptr<AutoFillExtensionContext> context_;
};

class EtsAutoFillExtensionConnection : public AbilityConnectCallback {
public:
    explicit EtsAutoFillExtensionConnection(ani_vm *etsVm);
    ~EtsAutoFillExtensionConnection() override;
    void OnAbilityConnectDone(
        const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject, int resultCode) override;
    void OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode) override;
    void CallEtsFailed(int32_t errorCode);
    void SetConnectionId(int64_t id);
    int64_t GetConnectionId() const
    {
        return connectionId_;
    }
    void SetConnectionRef(ani_object connectOptionsObj);
    void RemoveConnectionObject();

private:
    void HandleOnAbilityConnectDone(
        const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject, int resultCode);
    void HandleOnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode);
    void ReleaseObjectReference(ani_ref etsObjRef);

    ani_vm *etsVm_ = nullptr;
    int64_t connectionId_ = -1;
    ani_ref etsConnectionRef_ = nullptr;
};

struct EtsAutoFillExtensionConnectionKey {
    AAFwk::Want want;
    int64_t id;
};

struct EtsAutoFillKeyCompare {
    bool operator()(const EtsAutoFillExtensionConnectionKey &key1,
        const EtsAutoFillExtensionConnectionKey &key2) const
    {
        return key1.id < key2.id;
    }
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_ETS_AUTO_FILL_EXTENSION_CONTEXT_H