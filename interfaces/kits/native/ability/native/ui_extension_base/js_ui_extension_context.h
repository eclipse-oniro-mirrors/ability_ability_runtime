/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_JS_UI_EXTENSION_CONTEXT_H
#define OHOS_ABILITY_RUNTIME_JS_UI_EXTENSION_CONTEXT_H

#include <memory>

#include "ui_extension_context.h"
#include "js_free_install_observer.h"
#include "native_engine/native_engine.h"

namespace OHOS {
namespace AbilityRuntime {
struct NapiCallbackInfo;
class JsEmbeddableUIAbilityContext;
class JSUIServiceUIExtConnection;

class JsUIExtensionContext {
public:
    explicit JsUIExtensionContext(const std::shared_ptr<UIExtensionContext>& context) : context_(context) {}
    virtual ~JsUIExtensionContext();
    static void Finalizer(napi_env env, void* data, void* hint);
    static napi_value StartAbility(napi_env env, napi_callback_info info);
    static napi_value OpenLink(napi_env env, napi_callback_info info);
    static napi_value TerminateSelf(napi_env env, napi_callback_info info);
    static napi_value TerminateSelfWithResult(napi_env env, napi_callback_info info);
    static napi_value CreateJsUIExtensionContext(napi_env env, std::shared_ptr<UIExtensionContext> context);
    static napi_value StartAbilityForResult(napi_env env, napi_callback_info info);
    static napi_value StartAbilityForResultAsCaller(napi_env env, napi_callback_info info);
    static napi_value StartUIAbilitiesInSplitWindowMode(napi_env env, napi_callback_info info);
    static napi_value StartUIAbilities(napi_env env, napi_callback_info info);
    static napi_value ConnectAbility(napi_env env, napi_callback_info info);
    static napi_value DisconnectAbility(napi_env env, napi_callback_info info);
    static napi_value ReportDrawnCompleted(napi_env env, napi_callback_info info);
    static napi_value OpenAtomicService(napi_env env, napi_callback_info info);
    static napi_value StartUIServiceExtension(napi_env env, napi_callback_info info);
    static napi_value ConnectUIServiceExtension(napi_env env, napi_callback_info info);
    static napi_value DisconnectUIServiceExtension(napi_env env, napi_callback_info info);
    static napi_value StartServiceExtensionAbility(napi_env env, napi_callback_info info);
    static napi_value StartServiceExtensionAbilityWithAccount(napi_env env, napi_callback_info info);
    static napi_value SetColorMode(napi_env env, napi_callback_info info);
    static napi_value SetHostPageOverlayForbidden(napi_env env, napi_callback_info info);
    static napi_value StartAbilityByType(napi_env env, napi_callback_info info);
    static napi_value IsTerminating(napi_env env, napi_callback_info info);
    static napi_value ConnectAbilityWithRootHostToken(napi_env env, napi_callback_info info);
protected:
    virtual napi_value OnStartAbility(napi_env env, NapiCallbackInfo& info);
    virtual napi_value OnTerminateSelf(napi_env env, NapiCallbackInfo& info);
    virtual napi_value OnTerminateSelfWithResult(napi_env env, NapiCallbackInfo& info);
    virtual napi_value OnStartAbilityForResult(napi_env env, NapiCallbackInfo& info);
    virtual napi_value OnStartAbilityForResultAsCaller(napi_env env, NapiCallbackInfo &info);
    virtual napi_value OnStartUIAbilitiesInSplitWindowMode(napi_env env, NapiCallbackInfo& info);
    virtual napi_value OnStartUIAbilities(napi_env env, NapiCallbackInfo& info);
    virtual napi_value OnConnectAbility(napi_env env, NapiCallbackInfo& info);
    virtual napi_value OnConnectAbilityWithRootHostToken(napi_env env, NapiCallbackInfo& info);
    virtual napi_value OnDisconnectAbility(napi_env env, NapiCallbackInfo& info);
    virtual napi_value OnReportDrawnCompleted(napi_env env, NapiCallbackInfo& info);
    virtual napi_value OnOpenAtomicService(napi_env env, NapiCallbackInfo& info);
    virtual napi_value OnStartUIServiceExtension(napi_env env, NapiCallbackInfo& info);
    virtual napi_value OnStartServiceExtensionAbility(napi_env env, NapiCallbackInfo& info);
    virtual napi_value OnStartServiceExtensionAbilityWithAccount(napi_env env, NapiCallbackInfo& info);
    virtual napi_value OnSetHostPageOverlayForbidden(napi_env env, NapiCallbackInfo& info);
    bool UnwrapConnectUIServiceExtensionParam(napi_env env, NapiCallbackInfo& info, AAFwk::Want& want);
    bool CheckConnectAlreadyExist(napi_env env, AAFwk::Want& want, napi_value callback, napi_value& result);
    virtual napi_value OnConnectUIServiceExtension(napi_env env, NapiCallbackInfo& info);
    static void DoConnectUIServiceExtension(napi_env env,
        std::weak_ptr<UIExtensionContext> weakContext, sptr<JSUIServiceUIExtConnection> connection,
        std::shared_ptr<NapiAsyncTask> uasyncTaskShared, const AAFwk::Want& want);
    virtual napi_value OnDisconnectUIServiceExtension(napi_env env, NapiCallbackInfo& info);
    void SetCallbackForTerminateWithResult(int32_t resultCode, AAFwk::Want& want,
        NapiAsyncTask::CompleteCallback& complete);
    virtual napi_value OnSetColorMode(napi_env env, NapiCallbackInfo& info);
    virtual napi_value OnStartAbilityByType(napi_env env, NapiCallbackInfo& info);
    virtual napi_value OnIsTerminating(napi_env env, NapiCallbackInfo& info);

protected:
    std::weak_ptr<UIExtensionContext> context_;
private:
    sptr<JsFreeInstallObserver> freeInstallObserver_ = nullptr;
    friend class JsEmbeddableUIAbilityContext;

    bool CheckStartAbilityInputParam(napi_env env, NapiCallbackInfo& info, AAFwk::Want& want,
        AAFwk::StartOptions& startOptions, size_t& unwrapArgc) const;
    bool CheckStartUIAbilitiesInSplitWindowModeInputParam(napi_env env, NapiCallbackInfo &info,
        int32_t &primaryWindowId, AAFwk::Want &secondaryWant);
    napi_value OpenAtomicServiceInner(napi_env env, NapiCallbackInfo& info, AAFwk::Want &want,
        const AAFwk::StartOptions &options, size_t unwrapArgc);
    void AddFreeInstallObserver(napi_env env, const AAFwk::Want &want, napi_value callback, napi_value* result,
        bool isAbilityResult = false, bool isOpenLink = false);
    bool UnwrapWantList(napi_env env, NapiCallbackInfo &info, std::vector<AAFwk::Want> &wantList);
    napi_value OnConnectAbilityInner(napi_env env, NapiCallbackInfo& info, bool withRootHostTokenTransfer);
    bool CreateOpenLinkTask(const napi_env &env, const napi_value &lastParam,
        AAFwk::Want &want, int &requestCode);
    napi_value OnOpenLink(napi_env env, NapiCallbackInfo& info);
    napi_value OnOpenLinkInner(napi_env env, const AAFwk::Want& want,
        int requestCode, const std::string &startTime, const std::string &url, bool hideFailureTipDialog = false);
    void UnWrapCompletionHandlerForAtomicService(
        napi_env env, napi_value param, AAFwk::StartOptions &options, const std::string &appId);
    void AddCompletionHandlerForOpenLink(AAFwk::Want &want, OnRequestResult &onRequestSucc,
    OnRequestResult &onRequestFail);
    std::pair<OnAtomicRequestSuccess, OnAtomicRequestFailure> CreateAtomicServiceCallBack(napi_env env,
        const std::shared_ptr<NativeReference> &atomicServiceRef,
        const std::shared_ptr<NativeReference> &onRequestSuccRef,
        const std::shared_ptr<NativeReference> &onRequestFailRef);

    // Helper methods for TerminateSelf in different modes
    napi_value HandleTerminateSelfInEmbeddableMode(napi_env env, napi_value lastParam,
        const std::shared_ptr<AbilityRuntime::Context>& context);
    napi_value HandleTerminateSelfInNonEmbeddableMode(napi_env env, napi_value lastParam);

    // Helper methods for TerminateSelfWithResult in different modes
    napi_value HandleTerminateSelfWithResultInEmbeddableMode(napi_env env, napi_value lastParam,
        const std::shared_ptr<AbilityRuntime::Context>& context, int32_t resultCode, const AAFwk::Want& want);
    napi_value HandleTerminateSelfWithResultInNonEmbeddableMode(napi_env env, napi_value lastParam,
        int32_t resultCode, AAFwk::Want& want);

#ifdef SUPPORT_SCREEN
    void InitDisplayId(AAFwk::Want &want);
    void InitDisplayId(AAFwk::Want &want, AAFwk::StartOptions &startOptions, napi_env &env, NapiCallbackInfo& info);
#endif
};

class JSUIExtensionConnection : public AbilityConnectCallback {
public:
    explicit JSUIExtensionConnection(napi_env env);
    ~JSUIExtensionConnection();
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
    int64_t GetConnectionId();
protected:
    napi_env env_ = nullptr;
    std::unique_ptr<NativeReference> jsConnectionObject_ = nullptr;
    int64_t connectionId_ = -1;
};

struct UIExtensionConnectionKey {
    AAFwk::Want want;
    int64_t id;
};

struct key_compare {
    bool operator()(const UIExtensionConnectionKey &key1, const UIExtensionConnectionKey &key2) const
    {
        if (key1.id < key2.id) {
            return true;
        }
        return false;
    }
};

}  // namespace AbilityRuntime
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_JS_UI_EXTENSION_CONTEXT_H
