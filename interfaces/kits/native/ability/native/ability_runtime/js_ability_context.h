/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_JS_ABILITY_CONTEXT_H
#define OHOS_ABILITY_RUNTIME_JS_ABILITY_CONTEXT_H

#include <algorithm>
#include <memory>
#include <native_engine/native_value.h>

#include "ability_connect_callback.h"
#include "ability_context.h"
#include "js_free_install_observer.h"
#include "js_runtime.h"
#include "event_handler.h"

class NativeReference;

namespace OHOS {
namespace AbilityRuntime {
struct NapiCallbackInfo;
class JsEmbeddableUIAbilityContext;
class JSUIServiceExtAbilityConnection;
class JsAbilityContext final {
public:
    explicit JsAbilityContext(const std::shared_ptr<AbilityContext>& context) : context_(context) {}
    ~JsAbilityContext() = default;

    static void Finalizer(napi_env env, void* data, void* hint);

    static napi_value StartAbility(napi_env env, napi_callback_info info);
    static napi_value OpenLink(napi_env env, napi_callback_info info);
    static napi_value StartAbilityAsCaller(napi_env env, napi_callback_info info);
    static napi_value StartRecentAbility(napi_env env, napi_callback_info info);
    static napi_value StartAbilityWithAccount(napi_env env, napi_callback_info info);
    static napi_value StartAbilityByCall(napi_env env, napi_callback_info info);
    static napi_value StartAbilityForResult(napi_env env, napi_callback_info info);
    static napi_value StartAbilityForResultWithAccount(napi_env env, napi_callback_info info);
    static napi_value StartServiceExtensionAbility(napi_env env, napi_callback_info info);
    static napi_value StartServiceExtensionAbilityWithAccount(napi_env env, napi_callback_info info);
    static napi_value StopServiceExtensionAbility(napi_env env, napi_callback_info info);
    static napi_value StopServiceExtensionAbilityWithAccount(napi_env env, napi_callback_info info);
    static napi_value ConnectAbility(napi_env env, napi_callback_info info);
    static napi_value ConnectAbilityWithAccount(napi_env env, napi_callback_info info);
    static napi_value ConnectAbilityWithAccountRestricted(napi_env env, napi_callback_info info);
    static napi_value DisconnectAbility(napi_env env, napi_callback_info info);
    static napi_value DisconnectAbilityRestricted(napi_env env, napi_callback_info info);
    static napi_value TerminateSelf(napi_env env, napi_callback_info info);
    static napi_value TerminateSelfWithResult(napi_env env, napi_callback_info info);
    static napi_value BackToCallerAbilityWithResult(napi_env env, napi_callback_info info);
    static napi_value RestoreWindowStage(napi_env env, napi_callback_info info);
    static napi_value RequestDialogService(napi_env env, napi_callback_info info);
    static napi_value IsTerminating(napi_env env, napi_callback_info info);
    static napi_value ReportDrawnCompleted(napi_env env, napi_callback_info info);
    static napi_value SetMissionContinueState(napi_env env, napi_callback_info info);
    static napi_value StartAbilityByType(napi_env env, napi_callback_info info);
    static napi_value RequestModalUIExtension(napi_env env, napi_callback_info info);
    static napi_value ShowAbility(napi_env env, napi_callback_info info);
    static napi_value HideAbility(napi_env env, napi_callback_info info);
    static napi_value MoveAbilityToBackground(napi_env env, napi_callback_info info);
    static napi_value OpenAtomicService(napi_env env, napi_callback_info info);
    static napi_value StartUIServiceExtension(napi_env env, napi_callback_info info);
    static napi_value ConnectUIServiceExtension(napi_env env, napi_callback_info info);
    static napi_value DisconnectUIServiceExtension(napi_env env, napi_callback_info info);
    static napi_value SetRestoreEnabled(napi_env env, napi_callback_info info);
    static napi_value SetColorMode(napi_env env, napi_callback_info info);
    static napi_value RevokeDelegator(napi_env env, napi_callback_info info);
    static napi_value SetOnNewWantSkipScenarios(napi_env env, napi_callback_info info);
    static napi_value StartAppServiceExtensionAbility(napi_env env, napi_callback_info info);
    static napi_value StopAppServiceExtensionAbility(napi_env env, napi_callback_info info);
    static napi_value ConnectAppServiceExtensionAbility(napi_env env, napi_callback_info info);
    static napi_value DisconnectAppServiceExtensionAbility(napi_env env, napi_callback_info info);
    static napi_value StartSelfUIAbilityInCurrentProcess(napi_env env, napi_callback_info info);
    static napi_value NotifyCancelGamePreLaunch(napi_env env, napi_callback_info info);
    static napi_value NotifyCompleteGamePreLaunch(napi_env env, napi_callback_info info);
    static napi_value RestartAppWithWindow(napi_env env, napi_callback_info info);
    static napi_value SetMissionWindowIcon(napi_env env, napi_callback_info info);

    static void ConfigurationUpdated(napi_env env, napi_value object,
        const std::shared_ptr<AppExecFwk::Configuration> &config);

    std::shared_ptr<AbilityContext> GetAbilityContext()
    {
        return context_.lock();
    }

#ifdef SUPPORT_GRAPHICS
public:
    static napi_value SetMissionLabel(napi_env env, napi_callback_info info);
    static napi_value SetMissionIcon(napi_env env, napi_callback_info info);
    static napi_value SetAbilityInstanceInfo(napi_env env, napi_callback_info info);

private:
    napi_value OnSetMissionLabel(napi_env env, NapiCallbackInfo& info);
    napi_value OnSetMissionIcon(napi_env env, NapiCallbackInfo& info);
    napi_value OnSetAbilityInstanceInfo(napi_env env, NapiCallbackInfo& info);
#endif

private:
    static void ClearFailedCallConnection(
        const std::weak_ptr<AbilityContext>& abilityContext, const std::shared_ptr<CallerCallBack> &callback);
    static bool CheckSatisfyTargetAPIVersion(
        const std::weak_ptr<AbilityContext>& context, int32_t version);
    napi_value OnStartAbility(napi_env env, NapiCallbackInfo& info, bool isStartRecent = false);
    napi_value OnOpenLink(napi_env env, NapiCallbackInfo& info);
    napi_value OnOpenLinkInner(napi_env env, const AAFwk::Want &want, int requestCode, const std::string &startTime,
        const std::string &url, bool hideFailureTipDialog = false);
    napi_value OnStartAbilityAsCallerInner(napi_env env, NapiCallbackInfo& info,
        const AAFwk::Want &want, size_t unwrapArgc, const AAFwk::StartOptions &startOptions);
    napi_value OnStartAbilityAsCaller(napi_env env, NapiCallbackInfo& info);
    napi_value OnStartRecentAbility(napi_env env, NapiCallbackInfo& info);
    napi_value OnStartAbilityWithAccount(napi_env env, NapiCallbackInfo& info);
    napi_value OnStartAbilityByCall(napi_env env, NapiCallbackInfo& info);
    napi_value OnStartAbilityForResult(napi_env env, NapiCallbackInfo& info);
    napi_value OnStartAbilityForResultWithAccount(napi_env env, NapiCallbackInfo& info);
    napi_value OnStartExtensionAbility(napi_env env, NapiCallbackInfo& info);
    napi_value OnStartExtensionAbilityWithAccount(napi_env env, NapiCallbackInfo& info);
    napi_value OnStopExtensionAbility(napi_env env, NapiCallbackInfo& info);
    napi_value OnStopExtensionAbilityWithAccount(napi_env env, NapiCallbackInfo& info);
    napi_value OnTerminateSelfWithResult(napi_env env, NapiCallbackInfo& info);
    napi_value OnBackToCallerAbilityWithResult(napi_env env, NapiCallbackInfo& info);
    napi_value OnConnectAbility(napi_env env, NapiCallbackInfo& info);
    napi_value OnConnectAbilityWithAccount(napi_env env, NapiCallbackInfo& info);
    napi_value OnDisconnectAbility(napi_env env, NapiCallbackInfo& info);
    napi_value OnTerminateSelf(napi_env env, NapiCallbackInfo& info);
    napi_value OnRestoreWindowStage(napi_env env, NapiCallbackInfo& info);
    napi_value OnRequestDialogService(napi_env env, NapiCallbackInfo& info);
    napi_value OnIsTerminating(napi_env env, NapiCallbackInfo& info);
    napi_value OnReportDrawnCompleted(napi_env env, NapiCallbackInfo& info);
    napi_value OnSetMissionContinueState(napi_env env, NapiCallbackInfo& info);
    napi_value OnStartAbilityByType(napi_env env, NapiCallbackInfo& info);
    napi_value OnRequestModalUIExtension(napi_env env, NapiCallbackInfo& info);
    napi_value OnShowAbility(napi_env env, NapiCallbackInfo& info);
    napi_value OnHideAbility(napi_env env, NapiCallbackInfo& info);
    napi_value ChangeAbilityVisibility(napi_env env, NapiCallbackInfo& info, bool isShow);
    napi_value OnMoveAbilityToBackground(napi_env env, NapiCallbackInfo& info);
    napi_value OnSetRestoreEnabled(napi_env env, NapiCallbackInfo& info);
    bool CreateOpenLinkTask(const napi_env &env, const napi_value &lastParam, AAFwk::Want &want,
        int &requestCode);
    napi_value OnOpenAtomicService(napi_env env, NapiCallbackInfo& info);
    napi_value OpenAtomicServiceInner(napi_env env, NapiCallbackInfo& info, AAFwk::Want &want,
        AAFwk::StartOptions &options);
    napi_value OnStartUIServiceExtension(napi_env env, NapiCallbackInfo& info);
    bool UnwrapConnectUIServiceExtensionParam(napi_env env, NapiCallbackInfo& info, AAFwk::Want& want);
    bool CheckConnectAlreadyExist(napi_env env, AAFwk::Want& want, napi_value callback, napi_value& result);
    napi_value OnConnectUIServiceExtension(napi_env env, NapiCallbackInfo& info);
    static void DoConnectUIServiceExtension(napi_env env,
        std::weak_ptr<AbilityContext> weakContext, sptr<JSUIServiceExtAbilityConnection> connection,
        std::shared_ptr<NapiAsyncTask> uasyncTaskShared, const AAFwk::Want& want);
    napi_value OnDisconnectUIServiceExtension(napi_env env, NapiCallbackInfo& info);
    napi_value OnSetColorMode(napi_env env, NapiCallbackInfo& info);
    napi_value OnRevokeDelegator(napi_env env, NapiCallbackInfo& info);
    napi_value OnSetOnNewWantSkipScenarios(napi_env env, NapiCallbackInfo& info);
    napi_value OnStartAppServiceExtensionAbility(napi_env env, NapiCallbackInfo& info);
    napi_value OnStopAppServiceExtensionAbility(napi_env env, NapiCallbackInfo& info);
    napi_value OnConnectAppServiceExtensionAbility(napi_env env, NapiCallbackInfo& info);
    napi_value OnRestartAppWithWindow(napi_env env, NapiCallbackInfo& info);
    napi_value StartExtensionAbilityCommon(napi_env env, NapiCallbackInfo& info,
        AppExecFwk::ExtensionAbilityType extensionType);
    napi_value StopExtensionAbilityCommon(napi_env env, NapiCallbackInfo& info,
        AppExecFwk::ExtensionAbilityType extensionType);
    napi_value ConnectExtensionAbilityCommon(napi_env env, NapiCallbackInfo& info,
        AppExecFwk::ExtensionAbilityType extensionType);
    napi_value OnStartSelfUIAbilityInCurrentProcess(napi_env env, NapiCallbackInfo &info);
    napi_value OnNotifyCancelGamePreLaunch(napi_env env, NapiCallbackInfo &info);
    napi_value OnNotifyCompleteGamePreLaunch(napi_env env, NapiCallbackInfo &info);
    napi_value OnSetMissionWindowIcon(napi_env env, NapiCallbackInfo &info);

    static bool UnWrapWant(napi_env env, napi_value argv, AAFwk::Want& want);
    static napi_value WrapWant(napi_env env, const AAFwk::Want& want);
    static bool UnWrapAbilityResult(napi_env env, napi_value argv, int& resultCode, AAFwk::Want& want);
    static napi_value WrapAbilityResult(napi_env env, const int& resultCode, const AAFwk::Want& want);
    void InheritWindowMode(AAFwk::Want &want);
    static napi_value WrapRequestDialogResult(napi_env env, int32_t resultCode, const AAFwk::Want& want);
    void AddFreeInstallObserver(napi_env env, const AAFwk::Want &want, napi_value callback, napi_value* result,
        bool isAbilityResult = false, bool isOpenLink = false);
    void UnwrapCompletionHandlerInStartOptions(napi_env env, napi_value param, AAFwk::StartOptions &options);
    OnRequestResult UnwrapCompletionHandlerOnRequestResult(napi_env env, const char *funcName,
        std::shared_ptr<NativeReference> ref);
    bool CheckStartAbilityByCallParams(napi_env env, NapiCallbackInfo& info, AAFwk::Want &want,
        int32_t &userId, napi_value &lastParam);
    napi_value SyncSetMissionContinueState(napi_env env, NapiCallbackInfo& info, const AAFwk::ContinueState& state);
    static int32_t GenerateRequestCode();
    void UnWrapCompletionHandlerForAtomicService(
        napi_env env, napi_value param, AAFwk::StartOptions &options, const std::string &appId);
    void AddCompletionHandlerForOpenLink(AAFwk::Want &want, OnRequestResult &onRequestSucc,
    OnRequestResult &onRequestFail);
    std::pair<OnAtomicRequestSuccess, OnAtomicRequestFailure> CreateAtomicServiceCallBack(napi_env env,
        const std::shared_ptr<NativeReference> &atomicServiceRef,
        const std::shared_ptr<NativeReference> &onRequestSuccRef,
        const std::shared_ptr<NativeReference> &onRequestFailRef);
    static int32_t curRequestCode_;
    static std::mutex requestCodeMutex_;

    std::weak_ptr<AbilityContext> context_;
    sptr<JsFreeInstallObserver> freeInstallObserver_ = nullptr;
    friend class JsEmbeddableUIAbilityContext;
};

napi_value CreateJsAbilityContext(napi_env env, std::shared_ptr<AbilityContext> context);
napi_value AttachJsUIAbilityContext(napi_env env, void *value, void *hint);

struct ConnectCallback {
    std::unique_ptr<NativeReference> jsConnectionObject_ = nullptr;
};

class JSAbilityConnection : public AbilityConnectCallback {
public:
    explicit JSAbilityConnection(napi_env env);
    ~JSAbilityConnection();
    void ReleaseNativeReference(NativeReference* ref);
    void OnAbilityConnectDone(
        const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject, int resultCode) override;
    void OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode) override;
    virtual void HandleOnAbilityConnectDone(
        const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject, int resultCode);
    virtual void HandleOnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode);
    void SetJsConnectionObject(napi_value jsConnectionObject);
    std::unique_ptr<NativeReference>& GetJsConnectionObject() { return jsConnectionObject_; }
    void RemoveConnectionObject();
    void CallJsFailed(int32_t errorCode);
    napi_value CallObjectMethod(const char* name, napi_value const *argv, size_t argc);
    void SetConnectionId(int64_t id);
    int64_t GetConnectionId() { return connectionId_; }
protected:
    napi_env env_;
    int64_t connectionId_ = -1;
    std::unique_ptr<NativeReference> jsConnectionObject_ = nullptr;
private:
    napi_value ConvertElement(const AppExecFwk::ElementName &element);
};

struct ConnectionKey {
    AAFwk::Want want;
    int64_t id;
    int32_t accountId;
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
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_JS_ABILITY_CONTEXT_H
