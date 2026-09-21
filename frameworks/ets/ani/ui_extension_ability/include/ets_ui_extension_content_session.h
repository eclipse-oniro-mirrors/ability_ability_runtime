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

#ifndef OHOS_ABILITY_RUNTIME_ETS_UI_EXTENSION_CONTENT_SESSION_H
#define OHOS_ABILITY_RUNTIME_ETS_UI_EXTENSION_CONTENT_SESSION_H

#include <mutex>

#include "ani.h"
#include "ets_free_install_observer.h"
#include "ets_runtime.h"
#include "session_info.h"
#include "start_options.h"
#include "ui_extension_context.h"
#include "window.h"

namespace OHOS {
namespace AbilityRuntime {
using RuntimeTask = std::function<void(int, const AAFwk::Want&, bool)>;
class EtsAbilityResultListener {
public:
    EtsAbilityResultListener() = default;
    virtual ~EtsAbilityResultListener() = default;
    virtual void OnAbilityResult(int requestCode, int resultCode, const AAFwk::Want &resultData) = 0;
    virtual bool IsMatch(int requestCode) = 0;
};

class EtsAbilityResultListeners {
public:
    EtsAbilityResultListeners() = default;
    virtual ~EtsAbilityResultListeners() = default;
    void AddListener(const uint64_t &uiExtensionComponentId, std::shared_ptr<EtsAbilityResultListener> listener);
    void RemoveListener(const uint64_t &uiExtensionComponentId);
    void OnAbilityResult(int requestCode, int resultCode, const AAFwk::Want &resultData);
private:
    std::map<uint64_t, std::shared_ptr<EtsAbilityResultListener>> listeners_;
    std::mutex listenersMutex_;
};

class EtsUISessionAbilityResultListener : public EtsAbilityResultListener {
public:
    EtsUISessionAbilityResultListener() = default;
    virtual ~EtsUISessionAbilityResultListener() = default;
    virtual void OnAbilityResult(int requestCode, int resultCode, const AAFwk::Want &resultData);
    virtual bool IsMatch(int requestCode);
    void OnAbilityResultInner(int requestCode, int resultCode, const AAFwk::Want &resultData);
    void SaveResultCallbacks(int requestCode, RuntimeTask &&task);
private:
    std::map<int, RuntimeTask> resultCallbacks_;
    std::mutex resultCallbacksMutex_;
};

class EtsUIExtensionCallback;
class EtsUIExtensionContentSession {
private:
    class CallbackWrapper;
public:
    EtsUIExtensionContentSession(sptr<AAFwk::SessionInfo> sessionInfo,
        sptr<Rosen::Window> uiWindow, std::weak_ptr<AbilityRuntime::Context> &context,
        std::shared_ptr<EtsAbilityResultListeners> &abilityResultListeners);
    EtsUIExtensionContentSession(sptr<AAFwk::SessionInfo> sessionInfo,
        sptr<Rosen::Window> uiWindow);
    virtual ~EtsUIExtensionContentSession() = default;
    void AddFreeInstallObserver(ani_env *env, const AAFwk::Want &want, ani_object callback,
        std::shared_ptr<AbilityRuntime::Context> context, bool isResult);
    static EtsUIExtensionContentSession* GetEtsContentSession(ani_env *env, ani_object obj);
    static ani_object CreateEtsUIExtensionContentSession(ani_env *env, EtsUIExtensionContentSession *contentSessionPtr);

    static void NativeSendData(ani_env *env, ani_object obj, ani_object data);
    static void NativeLoadContent(ani_env *env, ani_object obj, ani_string path, ani_object storage);
    static void NativeLoadContentByName(ani_env *env, ani_object obj, ani_string path, ani_object storage);
    static void NativeTerminateSelf(ani_env *env, ani_object obj, ani_object callback);
    static void NativeSetWindowBackgroundColor(ani_env *env, ani_object obj, ani_string color);
    static int NativeTerminateSelfWithResult(
        ani_env *env, ani_object obj, ani_object abilityResult, ani_object callback);
    static void NativeSetReceiveDataCallback(ani_env *env, ani_object clsObj, ani_object funcObj);
    static void NativeSetReceiveDataForResultCallback(ani_env *env, ani_object clsObj, ani_object funcObj);
    static ani_object NativeGetUIExtensionHostWindowProxy(ani_env *env, ani_object obj);
    static ani_object NativeGetUIExtensionWindowProxy(ani_env *env, ani_object obj);
    static void NativeStartAbilityByType(ani_env *env, ani_object obj,
        ani_string type, ani_ref wantParam, ani_object startCallback, ani_object asyncCallback);
    static void NativeSetWindowPrivacyMode(
        ani_env *env, ani_object obj, ani_boolean isPrivacyMode, ani_object callbackObj);
    static void NativeStartAbility(ani_env *env, ani_object aniObj, ani_object wantObj, ani_object callback);
    static void NativeStartAbilityWithStartOptions(ani_env *env,
        ani_object aniObj, ani_object wantObj, ani_object startOptionsObj, ani_object callback);
    static void NativeStartAbilityForResult(ani_env *env,
        ani_object aniObj, ani_object wantObj, ani_object callback);
    static void NativeStartAbilityForResultWithStartOptions(ani_env *env,
        ani_object aniObj, ani_object wantObj, ani_object startOptionsObj, ani_object callback);
    static void NativeStartAbilityAsCaller(ani_env *env, ani_object aniObj,
        ani_object wantObj, ani_object callbackObj, ani_object startOptionsObj);

    void SendData(ani_env *env, ani_object object, ani_object data);
    void LoadContent(ani_env *env, ani_object object, ani_string path, ani_object storage);
    void TerminateSelf();
    int32_t TerminateSelfWithResult();
    void SetWindowBackgroundColor(ani_env *env, ani_string color);
    void SetReceiveDataCallback(ani_env *env, ani_object functionObj);
    static void CallReceiveDataCallback(ani_vm *vm, ani_ref callbackRef, const AAFwk::WantParams &wantParams);
    void SetReceiveDataForResultCallback(ani_env *env, ani_object object);
    void SetWindowPrivacyMode(
        ani_env *env, ani_object obj, ani_boolean isPrivacyMode, ani_object callbackObj);
    static void CallReceiveDataCallbackForResult(
        ani_vm *vm, ani_ref callbackRef, const AAFwk::WantParams &wantParams, AAFwk::WantParams &retWantParams);
    void OnStartAbilityAsCaller(ani_env *env, ani_object aniObj, ani_object wantObj,
        ani_object callbackObj, ani_object startOptionsObj);
    std::shared_ptr<AbilityRuntime::Context> GetContext();
    sptr<Rosen::Window> GetUIWindow();
    static bool BindNativePtrCleaner(ani_env *env);
    static void Clean(ani_env *env, ani_object object);
    void OnStartAbility(
        ani_env* env, ani_object object, ani_object wantObject, ani_object opt, ani_object callback);
    void OnStartAbilityForResult(
        ani_env *env, ani_object aniObj, ani_object wantObj, ani_object startOptionsObj, ani_object callback);
    void StartAbilityForResultDoTask(ani_vm *etsVm, ani_object startOptionsObj,
        ani_ref callbackRef, AAFwk::Want &want, AAFwk::StartOptions &options,
        std::string startTime, std::shared_ptr<AbilityRuntime::Context> context);

    sptr<AAFwk::SessionInfo> GetSessionInfo()
    {
        return sessionInfo_;
    }
private:
    void SetReceiveDataCallbackRegister(ani_env *env, ani_object functionObj);
    void SetReceiveDataForResultCallbackRegister(ani_env *env, ani_object funcObj);
    void StartAbilityByType(ani_env *env, ani_string aniType, ani_ref aniWantParam,
        ani_object startCallback, ani_object asyncCallback);
    bool CheckStartAbilityByTypeParam(
        ani_env *env, ani_string aniType, ani_ref aniWantParam, std::string &type, AAFwk::WantParams &wantParam);
    ani_object GetUIExtensionHostWindowProxy(ani_env *env, ani_object object);
    ani_object GetUIExtensionWindowProxy(ani_env *env, ani_object object);
    void SetWindowPrivacyModeInner(ani_env *env, ani_boolean isPrivacyMode, ani_object callbackObj,
        ani_vm *etsVm, ani_ref callbackRef);
    void LoadContentByName(ani_env *env, ani_object object, ani_string path, ani_object storage);
    void StartAbilityInner(ani_env* env, ani_object object, ani_object opt,
        ani_object callback, AAFwk::Want &want, std::shared_ptr<AbilityRuntime::Context> context);
    static ani_status BindNativeMethod(ani_env *env, ani_class cls);

    sptr<AAFwk::SessionInfo> sessionInfo_;
    sptr<Rosen::Window> uiWindow_;
    std::weak_ptr<AbilityRuntime::Context> context_;
    ani_ref receiveDataCallback_ = nullptr;
    bool isRegistered_ = false;
    ani_ref receiveDataForResultCallback_ = nullptr;
    bool isSyncRegistered_ = false;
    std::shared_ptr<EtsUISessionAbilityResultListener> listener_;
    bool isFirstTriggerBindModal_ = true;
    sptr<EtsFreeInstallObserver> freeInstallObserver_;
#ifdef SUPPORT_SCREEN
    void InitDisplayId(AAFwk::Want &want);
    void InitDisplayId(AAFwk::Want &want, AAFwk::StartOptions &startOptions, ani_env *env, ani_object opt);
    void CreateAndSetupModalUIExtension(ani_env *env, ani_vm *vm, ani_object startCallback,
        ani_object asyncCallback, const AAFwk::Want &want);
    Ace::ModalUIExtensionCallbacks SetupModalUIExtensionCallbacks(
        std::shared_ptr<EtsUIExtensionCallback> uiExtensionCallback,
        std::shared_ptr<std::atomic<bool>> errorFired);
#endif
};

} // namespace AbilityRuntime
} // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_ETS_UI_EXTENSION_CONTENT_SESSION_H
