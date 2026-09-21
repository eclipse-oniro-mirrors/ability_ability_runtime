/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_JS_UI_EXTENSION_CONTENT_SESSION_H
#define OHOS_ABILITY_RUNTIME_JS_UI_EXTENSION_CONTENT_SESSION_H

#include "ability_result_listener.h"
#include "native_engine/native_engine.h"
#include "js_free_install_observer.h"
#include "js_runtime_utils.h"
#include "session_info.h"
#include "start_options.h"
#include "window.h"

namespace OHOS {
#ifdef SUPPORT_SCREEN
namespace Ace {
struct ModalUIExtensionCallbacks;
}
#endif
namespace AbilityRuntime {
using RuntimeTask = std::function<void(int, const AAFwk::Want&, bool)>;

class JsUIExtensionCallback;

class UISessionAbilityResultListener : public AbilityResultListener {
public:
    UISessionAbilityResultListener() = default;
    virtual ~UISessionAbilityResultListener() = default;
    virtual void OnAbilityResult(int requestCode, int resultCode, const AAFwk::Want &resultData);
    virtual bool IsMatch(int requestCode);
    void OnAbilityResultInner(int requestCode, int resultCode, const AAFwk::Want &resultData);
    void SaveResultCallbacks(int requestCode, RuntimeTask&& task);
private:
    std::map<int, RuntimeTask> resultCallbacks_;
};

class JsUIExtensionContentSession {
private:
    class CallbackWrapper;
public:
    JsUIExtensionContentSession(sptr<AAFwk::SessionInfo> sessionInfo,
        sptr<Rosen::Window> uiWindow, std::weak_ptr<AbilityRuntime::Context>& context,
        std::shared_ptr<AbilityResultListeners>& abilityResultListeners);
    JsUIExtensionContentSession(sptr<AAFwk::SessionInfo> sessionInfo,
        sptr<Rosen::Window> uiWindow);
    virtual ~JsUIExtensionContentSession();
    static void Finalizer(napi_env env, void* data, void* hint);
    static napi_value CreateJsUIExtensionContentSession(napi_env env,
        sptr<AAFwk::SessionInfo> sessionInfo, sptr<Rosen::Window> uiWindow,
        std::weak_ptr<AbilityRuntime::Context> context,
        std::shared_ptr<AbilityResultListeners>& abilityResultListeners);
    static napi_value CreateJsUIExtensionContentSession(napi_env env,
        sptr<AAFwk::SessionInfo> sessionInfo, sptr<Rosen::Window> uiWindow);

    static napi_value StartAbility(napi_env env, napi_callback_info info);
    static napi_value StartAbilityAsCaller(napi_env env, napi_callback_info info);
    static napi_value GetUIExtensionHostWindowProxy(napi_env env, napi_callback_info info);
    static napi_value GetUIExtensionWindowProxy(napi_env env, napi_callback_info info);
    static napi_value StartAbilityForResult(napi_env env, napi_callback_info info);
    static napi_value TerminateSelf(napi_env env, napi_callback_info info);
    static napi_value TerminateSelfWithResult(napi_env env, napi_callback_info info);
    static napi_value SendData(napi_env env, napi_callback_info info);
    static napi_value SetReceiveDataCallback(napi_env env, napi_callback_info info);
    static napi_value SetReceiveDataForResultCallback(napi_env env, napi_callback_info info);
    static napi_value LoadContent(napi_env env, napi_callback_info info);
    static napi_value LoadContentByName(napi_env env, napi_callback_info info);
    static napi_value SetWindowBackgroundColor(napi_env env, napi_callback_info info);
    static napi_value SetWindowPrivacyMode(napi_env env, napi_callback_info info);
    static napi_value StartAbilityByType(napi_env env, napi_callback_info info);

    sptr<AAFwk::SessionInfo> GetSessionInfo()
    {
        return sessionInfo_;
    }

    sptr<Rosen::Window> GetUIWindow()
    {
        return uiWindow_;
    }

    std::shared_ptr<AbilityRuntime::Context> GetContext()
    {
        return context_.lock();
    }

protected:
    napi_value OnStartAbility(napi_env env, NapiCallbackInfo& info);
    napi_value OnStartAbilityAsCaller(napi_env env, NapiCallbackInfo& info);
    napi_value OnGetUIExtensionHostWindowProxy(napi_env env, NapiCallbackInfo& info);
    napi_value OnGetUIExtensionWindowProxy(napi_env env, NapiCallbackInfo& info);
    napi_value OnStartAbilityForResult(napi_env env, NapiCallbackInfo& info);
    napi_value OnTerminateSelf(napi_env env, NapiCallbackInfo& info);
    napi_value OnTerminateSelfWithResult(napi_env env, NapiCallbackInfo& info);
    napi_value OnSendData(napi_env env, NapiCallbackInfo& info);
    napi_value OnSetReceiveDataCallback(napi_env env, NapiCallbackInfo& info);
    napi_value OnSetReceiveDataForResultCallback(napi_env env, NapiCallbackInfo& info);
    napi_value OnLoadContent(napi_env env, NapiCallbackInfo& info);
    napi_value OnLoadContentByName(napi_env env, NapiCallbackInfo& info);
    napi_value OnSetWindowBackgroundColor(napi_env env, NapiCallbackInfo& info);
    napi_value OnSetWindowPrivacyMode(napi_env env, NapiCallbackInfo& info);
    napi_value OnStartAbilityByType(napi_env env, NapiCallbackInfo& info);

    static void CallReceiveDataCallback(napi_env env, std::weak_ptr<CallbackWrapper> weakCallback,
        const AAFwk::WantParams& wantParams);
    static void CallReceiveDataCallbackForResult(napi_env env, std::weak_ptr<CallbackWrapper> weakCallback,
        const AAFwk::WantParams& wantParams, AAFwk::WantParams& retWantParams);
    void AddFreeInstallObserver(napi_env env, const AAFwk::Want &want, napi_value callback, napi_value* result,
        bool isAbilityResult = false);
    NapiAsyncTask::ExecuteCallback StartAbilityExecuteCallback(AAFwk::Want& want, size_t& unwrapArgc,
        napi_env env, NapiCallbackInfo& info, std::shared_ptr<int> &innerErrorCode);
    void StartAbilityForResultRuntimeTask(napi_env env, AAFwk::Want &want,
        std::shared_ptr<NapiAsyncTask> asyncTask, size_t& unwrapArgc, AAFwk::StartOptions startOptions);
    bool CheckStartAbilityByTypeParam(napi_env env, NapiCallbackInfo& info, std::string& type,
        AAFwk::WantParams& wantParam);
    void SetCallbackForTerminateWithResult(int32_t resultCode, AAFwk::Want& want,
        NapiAsyncTask::CompleteCallback& complete);
    static void ProcessStartAbilityByTypeComplete(napi_env env, const sptr<Rosen::Window>& uiWindow,
        const AAFwk::Want& want, std::shared_ptr<JsUIExtensionCallback> uiExtensionCallback);
    napi_value DispatchStartAbilityByTypeResult(napi_env env, napi_value lastParam,
        HandleEscape& handleEscape, std::shared_ptr<JsUIExtensionCallback> uiExtensionCallback,
        NapiAsyncTask::CompleteCallback& complete);
#ifdef SUPPORT_SCREEN
    static void SetupModalUIExtensionCallbacks(Ace::ModalUIExtensionCallbacks& callback,
        std::shared_ptr<JsUIExtensionCallback> uiExtensionCallback);
#endif

private:
    sptr<AAFwk::SessionInfo> sessionInfo_;
    sptr<Rosen::Window> uiWindow_;
    std::weak_ptr<AbilityRuntime::Context> context_;
    std::shared_ptr<CallbackWrapper> receiveDataCallback_;
    bool isRegistered = false;
    std::shared_ptr<CallbackWrapper> receiveDataForResultCallback_;
    bool isSyncRegistered = false;
    std::shared_ptr<UISessionAbilityResultListener> listener_;
    sptr<JsFreeInstallObserver> freeInstallObserver_ = nullptr;
    bool isFirstTriggerBindModal_ = true;
#ifdef SUPPORT_SCREEN
    void InitDisplayId(AAFwk::Want &want);
    void InitDisplayId(AAFwk::Want &want, AAFwk::StartOptions &startOptions, napi_env &env, NapiCallbackInfo& info);
#endif
};

class JsUIExtensionContentSession::CallbackWrapper {
public:
    void ResetCallback(std::shared_ptr<NativeReference> callback)
    {
        callback_ = callback;
    }
    std::shared_ptr<NativeReference> GetCallback() const
    {
        return callback_;
    }
private:
    std::shared_ptr<NativeReference> callback_;
};
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_JS_UI_EXTENSION_CONTENT_SESSION_H
