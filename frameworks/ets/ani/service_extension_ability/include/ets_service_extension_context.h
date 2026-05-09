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
#ifndef OHOS_ABILITY_RUNTIME_ETS_SERVICE_EXTENSION_CONTEXT_H
#define OHOS_ABILITY_RUNTIME_ETS_SERVICE_EXTENSION_CONTEXT_H

#include <array>
#include <iostream>
#include <unistd.h>

#include "ets_free_install_observer.h"
#include "ets_runtime.h"
#include "hilog_tag_wrapper.h"
#include "ohos_application.h"
#include "service_extension.h"
#include "service_extension_context.h"

namespace OHOS {
namespace AbilityRuntime {

class ETSServiceExtensionConnection : public AbilityConnectCallback {
public:
    explicit ETSServiceExtensionConnection(ani_vm *etsVm);
    ~ETSServiceExtensionConnection();
    void OnAbilityConnectDone(
        const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject, int32_t resultCode) override;
    void OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int32_t resultCode) override;
    void CallEtsFailed(int32_t errorCode);
    void SetConnectionId(int32_t id);
    int32_t GetConnectionId() { return connectionId_; }
    void SetConnectionRef(ani_object connectOptionsObj);
    void RemoveConnectionObject();
protected:
    ani_vm *etsVm_ = nullptr;
    int32_t connectionId_ = -1;
    ani_ref stsConnectionRef_ = nullptr;
};

class EtsServiceExtensionContext final {
public:
    explicit EtsServiceExtensionContext(std::shared_ptr<ServiceExtensionContext> context)
        : context_(std::move(context)) {}
    ~EtsServiceExtensionContext() = default;

    static void Finalizer(ani_env *env, ani_object obj);
    static EtsServiceExtensionContext *GetEtsAbilityContext(ani_env *env, ani_object obj);
    static void TerminateSelf(ani_env *env, ani_object obj, ani_object callback);
    static void StartAbility(ani_env *env, ani_object aniObj, ani_object wantObj, ani_object call);
    static ani_long ConnectServiceExtensionAbility(ani_env *env, ani_object aniObj,
        ani_object wantObj, ani_object connectOptionsObj);
    static void DisconnectServiceExtensionAbility(ani_env *env, ani_object aniObj, ani_long connectId,
        ani_object callback);
    static void StartAbilityWithOption(
        ani_env *env, ani_object aniObj, ani_object wantObj, ani_object opt, ani_object call);
    static void StartServiceExtensionAbility(ani_env *env, ani_object obj, ani_object wantObj, ani_object callbackobj);
    static void StopServiceExtensionAbility(
        ani_env *env, ani_object aniObj, ani_object wantObj, ani_object callbackobj);
    static void StartUIAbilities(ani_env *env, ani_object aniObj, ani_object wantListObj,
        ani_object callback);
    static bool UnwrapWantList(ani_env *env, ani_object wantListObj, std::vector<AAFwk::Want> &wantList);
    static void StartUIServiceExtension(ani_env *env, ani_object aniObj, ani_object wantObj, ani_object callback);
    static void WantCheck(ani_env *env, ani_object aniObj, ani_object wantObj);

    static void OpenAtomicService(ani_env *env, ani_object aniObj, ani_string aniAppId,
        ani_object callbackObj, ani_object optionsObj);
    static void PreStartMission(ani_env *env, ani_object aniObj, ani_string aniBundleName, ani_string aniModuleName,
        ani_string aniAbilityName, ani_string aniStartTime, ani_object callbackobj);
    static void RequestModalUIExtension(ani_env *env, ani_object obj, ani_object wantObj,
        ani_object callbackobj);
    static void RequestModalUIExtensionWithAccount(ani_env *env, ani_object obj, ani_object wantObj,
        ani_int accountId, ani_object callbackobj);
    static ani_long ConnectServiceExtensionAbilityWithAccount(ani_env *env, ani_object obj, ani_object wantObj,
        ani_int accountId, ani_object connectOptionsObj);
    static void StopServiceExtensionAbilityWithAccount(ani_env *env, ani_object obj, ani_object wantObj,
        ani_int accountId, ani_object callbackobj);
    static void StartServiceExtensionAbilityWithAccount(ani_env *env, ani_object obj, ani_object wantObj,
        ani_int accountId, ani_object callbackobj);
    static void StartRecentAbility(ani_env *env, ani_object obj, ani_object wantObj,
        ani_object callbackobj, ani_object optionsObj);
    static void StartAbilityWithAccountAndOptions(ani_env *env, ani_object obj, ani_object wantObj, ani_int accountId,
        ani_object optionsObj, ani_object callbackobj);
    static void StartAbilityWithAccount(ani_env *env, ani_object obj, ani_object wantObj, ani_int accountId,
        ani_object callbackObj);
    static void StartAbilityAsCaller(ani_env *env, ani_object obj, ani_object wantObj,
        ani_object callbackobj, ani_object optionsObj);
    static void OpenLinkCheck(ani_env *env, ani_object aniObj, ani_string aniLink);
    static void OpenLink(ani_env *env, ani_object obj, ani_string link, ani_object callbackobj,
        ani_object openlinkOptionsObj);
    static ani_object StartAbilityByCallWithAccount(ani_env *env, ani_object obj, ani_object want, ani_int accountId);
    static ani_object StartAbilityByCall(ani_env *env, ani_object obj, ani_object want);
    std::weak_ptr<ServiceExtensionContext> GetAbilityContext()
    {
        return context_;
    }
private:
    void OnTerminateSelf(ani_env *env, ani_object obj, ani_object callback);
    void OnStartServiceExtensionAbility(ani_env *env, ani_object obj, ani_object wantObj, ani_object callbackobj);
    void OnStopServiceExtensionAbility(ani_env *env, ani_object aniObj, ani_object wantObj, ani_object callbackobj);
    void OnStartAbility(ani_env *env, ani_object aniObj, ani_object wantObj, ani_object opt, ani_object call,
        bool isStartRecent = false);
    ani_long OnConnectServiceExtensionAbility(ani_env *env, ani_object aniObj,
        ani_object wantObj, ani_object connectOptionsObj);
    void OnDisconnectServiceExtensionAbility(ani_env *env, ani_object aniObj, ani_long connectId,
        ani_object callback);
    void AddFreeInstallObserver(ani_env *env, const AAFwk::Want &want, ani_object callbackObj,
        std::shared_ptr<ServiceExtensionContext> context, bool isAbilityResult = false, bool isOpenLink = false);
    void OnStartUIServiceExtension(ani_env *env, ani_object wantObj, ani_object callback);
    void OnStartUIAbilities(ani_env *env, ani_object aniObj, ani_object wantListObj, ani_object callback);

    void ClearFailedCallConnection(
        std::shared_ptr<ServiceExtensionContext> context, const std::shared_ptr<CallerCallBack> &callback);
    void OnOpenAtomicService(ani_env *env, ani_object aniObj, ani_string aniAppId,
        ani_object callbackObj, ani_object optionsObj);
    void OpenAtomicServiceInner(ani_env *env, ani_object aniObj, AAFwk::Want &want,
        AAFwk::StartOptions &options, std::string appId, ani_object callbackObj);
    void OnPreStartMission(ani_env *env, ani_object aniObj, ani_string aniBundleName, ani_string aniModuleName,
        ani_string aniAbilityName, ani_string aniStartTime, ani_object callbackobj);
    void OnRequestModalUIExtension(ani_env *env, ani_object obj, ani_object wantObj, ani_object callbackobj);
    void OnRequestModalUIExtensionWithAccount(
        ani_env *env, ani_object obj, ani_object wantObj, ani_int accountId, ani_object callbackobj);
    ani_long OnConnectServiceExtensionAbilityWithAccount(ani_env *env, ani_object obj, ani_object wantObj,
        ani_int accountId, ani_object connectOptionsObj);
    void OnStopServiceExtensionAbilityWithAccount(ani_env *env, ani_object obj, ani_object wantObj,
        ani_int accountId, ani_object callbackobj);
    void OnStartServiceExtensionAbilityWithAccount(ani_env *env, ani_object obj, ani_object wantObj,
        ani_int accountId, ani_object callbackobj);
    void OnStartAbilityWithAccount(ani_env *env, ani_object obj, ani_object wantObj, ani_int accountId,
        ani_object optionsObj, ani_object callbackobj);
    void UnWrapCompletionHandlerForAtomicService(ani_env *env, ani_object param, AAFwk::StartOptions &options,
        const std::string &appId);
    void CreateOnAtomicRequestFailureResultCallback(ani_env *env, ani_ref refCompletionHandler,
        OnAtomicRequestFailure &onRequestCallback, const char *callbackName);
    void CreateOnAtomicRequestSuccessResultCallback(ani_env *env, ani_ref refCompletionHandler,
        OnAtomicRequestSuccess &onRequestCallback, const char *callbackName);
    void OnStartAbilityAsCaller(ani_env *env, ani_object obj, ani_object wantObj, ani_object callbackobj,
        ani_object optionsObj);
    void OnOpenLink(ani_env *env, ani_object obj, ani_string link, ani_object callbackobj,
        ani_object openlinkOptionsObj);
    void AddCompletionHandlerForOpenLink(ani_env *env, ani_ref refCompletionHandler, AAFwk::Want &want,
        OnRequestResult &onRequestSucc, OnRequestResult &onRequestFail);
    ani_object OnStartAbilityByCallWithAccount(ani_env *env, ani_object obj, ani_object want, ani_int accountId);
    std::weak_ptr<ServiceExtensionContext> context_;
    sptr<EtsFreeInstallObserver> freeInstallObserver_ = nullptr;
};

struct EtsConnectionKey {
    AAFwk::Want want;
    int64_t id = 0;
    int32_t accountId = 0;
};

struct EtsKeyCompare {
    bool operator()(const EtsConnectionKey &key1, const EtsConnectionKey &key2) const
    {
        return key1.id < key2.id;
    }
};

ani_object CreateEtsServiceExtensionContext(ani_env *env, std::shared_ptr<ServiceExtensionContext> context);
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_ETS_SERVICE_EXTENSION_CONTEXT_H