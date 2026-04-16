/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_ETS_FORM_EXTENSION_CONTEXT_H
#define OHOS_ABILITY_RUNTIME_ETS_FORM_EXTENSION_CONTEXT_H

#include "ani.h"
#include "ability_connect_callback.h"
#include "event_handler.h"
#include "form_extension_context.h"
#include "ipc_skeleton.h"
#include "tokenid_kit.h"

namespace OHOS {
namespace AbilityRuntime {
class ETSFormExtensionConnection : public AbilityConnectCallback {
public:
    explicit ETSFormExtensionConnection(ani_vm *etsVm);
    ~ETSFormExtensionConnection();
    void OnAbilityConnectDone(
        const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject, int32_t resultCode) override;
    void OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int32_t resultCode) override;
    void CallEtsFailed(int32_t errorCode);
    void SetConnectionId(int32_t id);
    int32_t GetConnectionId() { return connectionId_; }
    void SetConnectionRef(ani_object connectOptionsObj);
    void RemoveConnectionObject();
    ani_env *AttachCurrentThread();
    void DetachCurrentThread();
protected:
    ani_vm *etsVm_ = nullptr;
    int32_t connectionId_ = -1;
    ani_ref stsConnectionRef_ = nullptr;
    bool isAttachThread_ = false;
};

ani_object CreateEtsFormExtensionContext(ani_env *env, std::shared_ptr<FormExtensionContext> &context);

class ETSFormExtensionContext {
public:
    explicit ETSFormExtensionContext(std::shared_ptr<FormExtensionContext> context)
        : context_(std::move(context)) {}
    ~ETSFormExtensionContext() = default;
    static void Finalizer(ani_env *env, ani_object obj);
    static ETSFormExtensionContext *GetEtsAbilityContext(ani_env *env, ani_object obj);
    static void StartAbility(ani_env *env, ani_object aniObj, ani_object wantObj, ani_object call);
    static ani_long ConnectAbility(ani_env *env, ani_object aniObj,
        ani_object wantObj, ani_object connectOptionsObj);
    static void DisconnectAbility(ani_env *env, ani_object aniObj, ani_long connectId,
        ani_object callback);
    static void CheckWant(ani_env *env, ani_object aniObj, ani_object wantObj);
    static void CheckConnectionAbility(ani_env *env, ani_object aniObj);
    std::weak_ptr<FormExtensionContext> GetAbilityContext()
    {
        return context_;
    }
private:
    bool CheckCallerIsSystemApp() const
    {
        auto selfToken = IPCSkeleton::GetSelfTokenID();
        return Security::AccessToken::TokenIdKit::IsSystemAppByFullTokenID(selfToken);
    }
    void OnStartAbility(ani_env *env, ani_object aniObj, ani_object wantObj, ani_object call);
        std::weak_ptr<FormExtensionContext> context_;
    ani_long OnConnectAbility(ani_env *env, ani_object aniObj,
        ani_object wantObj, ani_object connectOptionsObj);
    void OnDisconnectAbility(ani_env *env, ani_object aniObj, ani_long connectId,
        ani_object callback);
    void OnCheckConnectionAbility(ani_env *env, ani_object aniObj);
};

struct EtsConnectionKey {
    AAFwk::Want want;
    int64_t id;
};

struct ets_key_compare {
    bool operator()(const EtsConnectionKey &key1, const EtsConnectionKey &key2) const
    {
        if (key1.id < key2.id) {
            return true;
        }
        return false;
    }
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_ETS_FORM_EXTENSION_CONTEXT_H