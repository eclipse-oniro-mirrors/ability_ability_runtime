/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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
#include <string>
#include <set>

#include "event_handler.h"
#include "hilog_tag_wrapper.h"
#include "js_context_utils.h"
#include "js_data_struct_converter.h"
#include "js_error_utils.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "napi_common_want.h"
#include "napi_remote_object.h"
#include "ability_runtime/js_caller_complex.h"

namespace OHOS {
namespace AbilityRuntime {
namespace { // nameless

class JsCallerComplex {
public:
    enum class OBJSTATE {
        OBJ_NORMAL,
        OBJ_EXECUTION,
        OBJ_RELEASE
    };

    explicit JsCallerComplex(
        napi_env env, ReleaseCallFunc releaseCallFunc, sptr<IRemoteObject> callee,
        std::shared_ptr<CallerCallBack> callerCallBack) : releaseCallFunc_(releaseCallFunc),
        callee_(callee), releaseCallBackEngine_(env), remoteStateChanegdEngine_(env),
        callerCallBackObj_(callerCallBack), jsReleaseCallBackObj_(nullptr), jsRemoteStateChangedObj_(nullptr)
    {
        AddJsCallerComplex(this);
        handler_ = std::make_shared<AppExecFwk::EventHandler>(AppExecFwk::EventRunner::GetMainEventRunner());
    };
    virtual~JsCallerComplex()
    {
        RemoveJsCallerComplex(this);
    };

    static bool ReleaseObject(JsCallerComplex* data)
    {
        TAG_LOGD(AAFwkTag::DEFAULT, "called");
        if (data == nullptr) {
            TAG_LOGE(AAFwkTag::DEFAULT, "null data");
            return false;
        }

        if (!data->ChangeCurrentState(OBJSTATE::OBJ_RELEASE)) {
            auto handler = data->GetEventHandler();
            if (handler == nullptr) {
                TAG_LOGE(AAFwkTag::DEFAULT, "null handler");
                return false;
            }
            auto releaseObjTask = [pdata = data] () {
                if (!FindJsCallerComplex(pdata)) {
                    TAG_LOGE(AAFwkTag::DEFAULT, "argc not found");
                    return;
                }
                ReleaseObject(pdata);
            };

            handler->PostTask(releaseObjTask, "FinalizerRelease");
            return false;
        } else {
            // when the object is about to be destroyed, does not reset state
            std::unique_ptr<JsCallerComplex> delObj(data);
        }
        TAG_LOGD(AAFwkTag::DEFAULT, "end");
        return true;
    }

    static void Finalizer(napi_env env, void* data, void* hint)
    {
        TAG_LOGD(AAFwkTag::DEFAULT, "called");
        if (data == nullptr) {
            TAG_LOGE(AAFwkTag::DEFAULT, "null data");
            return;
        }

        auto ptr = static_cast<JsCallerComplex*>(data);
        if (!FindJsCallerComplex(ptr)) {
            TAG_LOGE(AAFwkTag::DEFAULT, "argc not found");
            return;
        }

        ReleaseObject(ptr);
        TAG_LOGD(AAFwkTag::DEFAULT, "end");
    }

    static napi_value JsReleaseCall(napi_env env, napi_callback_info info)
    {
        if (env == nullptr || info == nullptr) {
            TAG_LOGE(AAFwkTag::DEFAULT, "null %{public}s", ((env == nullptr) ? "env" : "info"));
            return nullptr;
        }
        GET_NAPI_INFO_AND_CALL(env, info, JsCallerComplex, ReleaseCallInner);
    }

    static napi_value JsSetOnReleaseCallBack(napi_env env, napi_callback_info info)
    {
        if (env == nullptr || info == nullptr) {
            TAG_LOGE(AAFwkTag::DEFAULT, "null %{public}s", ((env == nullptr) ? "env" : "info"));
            return nullptr;
        }
        GET_NAPI_INFO_AND_CALL(env, info, JsCallerComplex, SetOnReleaseCallBackInner);
    }

    static napi_value JsSetOnRemoteStateChanged(napi_env env, napi_callback_info info)
    {
        if (env == nullptr || info == nullptr) {
            TAG_LOGE(AAFwkTag::DEFAULT, "null %{public}s", ((env == nullptr) ? "env" : "info"));
            return nullptr;
        }
        GET_NAPI_INFO_AND_CALL(env, info, JsCallerComplex, SetOnRemoteStateChangedInner);
    }

    static bool AddJsCallerComplex(JsCallerComplex* ptr)
    {
        if (ptr == nullptr) {
            TAG_LOGE(AAFwkTag::DEFAULT, "null ptr");
            return false;
        }

        std::lock_guard<std::mutex> lck (jsCallerComplexMutex);
        auto iter = jsCallerComplexManagerList.find(ptr);
        if (iter != jsCallerComplexManagerList.end()) {
            TAG_LOGE(AAFwkTag::DEFAULT, "address exist");
            return false;
        }

        auto iterRet = jsCallerComplexManagerList.emplace(ptr);
        TAG_LOGD(AAFwkTag::DEFAULT, "retval: %{public}s", iterRet.second ? "true" : "false");
        return iterRet.second;
    }

    static bool RemoveJsCallerComplex(JsCallerComplex* ptr)
    {
        if (ptr == nullptr) {
            TAG_LOGE(AAFwkTag::DEFAULT, "null ptr");
            return false;
        }

        std::lock_guard<std::mutex> lck (jsCallerComplexMutex);
        auto iter = jsCallerComplexManagerList.find(ptr);
        if (iter == jsCallerComplexManagerList.end()) {
            TAG_LOGE(AAFwkTag::DEFAULT, "argc not found");
            return false;
        }

        jsCallerComplexManagerList.erase(ptr);
        TAG_LOGD(AAFwkTag::DEFAULT, "end");
        return true;
    }

    static bool FindJsCallerComplex(JsCallerComplex* ptr)
    {
        if (ptr == nullptr) {
            TAG_LOGE(AAFwkTag::DEFAULT, "null ptr");
            return false;
        }
        auto ret = true;
        std::lock_guard<std::mutex> lck (jsCallerComplexMutex);
        auto iter = jsCallerComplexManagerList.find(ptr);
        if (iter == jsCallerComplexManagerList.end()) {
            ret = false;
        }
        TAG_LOGD(AAFwkTag::DEFAULT, "retval %{public}s", ret ? "true" : "false");
        return ret;
    }

    static bool FindJsCallerComplexAndChangeState(JsCallerComplex* ptr, OBJSTATE state)
    {
        if (ptr == nullptr) {
            TAG_LOGE(AAFwkTag::DEFAULT, "null ptr");
            return false;
        }

        std::lock_guard<std::mutex> lck (jsCallerComplexMutex);
        auto iter = jsCallerComplexManagerList.find(ptr);
        if (iter == jsCallerComplexManagerList.end()) {
            TAG_LOGE(AAFwkTag::DEFAULT, "argc not found");
            return false;
        }

        auto ret = ptr->ChangeCurrentState(state);
        TAG_LOGD(AAFwkTag::DEFAULT, "ChangeCurrentState ret:%{public}s", ret ? "true" : "false");
        if (finalizeCallback_) {
            finalizeCallback_(reinterpret_cast<uintptr_t>(ptr));
        }

        return ret;
    }

    sptr<IRemoteObject> GetRemoteObject()
    {
        return callee_;
    }

    std::shared_ptr<AppExecFwk::EventHandler> GetEventHandler()
    {
        return handler_;
    }

    bool ChangeCurrentState(OBJSTATE state)
    {
        auto ret = false;
        std::unique_lock<std::mutex> lock(stateMechanismMutex_, std::try_to_lock);
        if (!lock.owns_lock()) {
            TAG_LOGE(AAFwkTag::DEFAULT, "mutex try_lock false");
            return ret;
        }

        if (currentState_ == OBJSTATE::OBJ_NORMAL) {
            currentState_ = state;
            ret = true;
            TAG_LOGD(AAFwkTag::DEFAULT, "currentState_:OBJ_NORMAL");
        } else if (currentState_ == state) {
            ret = true;
            TAG_LOGD(AAFwkTag::DEFAULT, "currentState_:state");
        } else {
            ret = false;
            TAG_LOGD(AAFwkTag::DEFAULT, "ret: false");
        }

        return ret;
    }

    OBJSTATE GetCurrentState()
    {
        std::lock_guard<std::mutex> lock(stateMechanismMutex_);
        return currentState_;
    }

    void StateReset()
    {
        std::lock_guard<std::mutex> lock(stateMechanismMutex_);
        currentState_ = OBJSTATE::OBJ_NORMAL;
    }

    void SetJsRemoteObj(napi_env env, napi_value value)
    {
        if (env == nullptr || value == nullptr) {
            TAG_LOGE(AAFwkTag::DEFAULT, "parameter error");
            return;
        }
        napi_ref ref = nullptr;
        napi_status createStatus = napi_create_reference(env, value, 1, &ref);
        if (createStatus != napi_ok || ref == nullptr) {
            TAG_LOGE(AAFwkTag::DEFAULT, "napi_create_reference failed, %{public}d", createStatus);
            return;
        }
        jsRemoteObj_.reset(reinterpret_cast<NativeReference*>(ref));
        jsRemoteObjEnv_ = env;
    }

    void ReleaseJsRemoteObj()
    {
        if (jsRemoteObjEnv_ == nullptr || jsRemoteObj_ == nullptr) {
            return;
        }
        TAG_LOGI(AAFwkTag::DEFAULT, "before release call");
        napi_value value = jsRemoteObj_->GetNapiValue();
        NAPI_ohos_rpc_ClearNativeRemoteProxy(jsRemoteObjEnv_, value);
        jsRemoteObj_.reset();
        jsRemoteObjEnv_ = nullptr;
    }

private:

    void OnReleaseNotify(const std::string &str)
    {
        TAG_LOGD(AAFwkTag::DEFAULT, "begin");
        if (handler_ == nullptr) {
            TAG_LOGE(AAFwkTag::DEFAULT, "null handler");
            return;
        }

        auto task = [notify = this, &str] () {
            if (!FindJsCallerComplex(notify)) {
                TAG_LOGE(AAFwkTag::DEFAULT, "address error");
                return;
            }
            notify->OnReleaseNotifyTask(str);
        };
        handler_->PostSyncTask(task, "OnReleaseNotify");
        TAG_LOGD(AAFwkTag::DEFAULT, "end");
    }

    void OnReleaseNotifyTask(const std::string &str)
    {
        HandleScope handle(releaseCallBackEngine_);
        TAG_LOGD(AAFwkTag::DEFAULT, "begin");
        if (jsReleaseCallBackObj_ == nullptr) {
            TAG_LOGE(AAFwkTag::DEFAULT, "null jsReleaseObj");
            return;
        }

        napi_value value = jsReleaseCallBackObj_->GetNapiValue();
        napi_value callback = jsReleaseCallBackObj_->GetNapiValue();
        napi_value args[] = { CreateJsValue(releaseCallBackEngine_, str) };
        napi_call_function(releaseCallBackEngine_, value, callback, 1, args, nullptr);
        callee_ = nullptr;
        StateReset();
    }

    void OnRemoteStateChangedNotify(const std::string &str)
    {
        TAG_LOGD(AAFwkTag::DEFAULT, "begin");
        if (handler_ == nullptr) {
            TAG_LOGE(AAFwkTag::DEFAULT, "null handler");
            return;
        }

        auto task = [notify = this, &str] () {
            if (!FindJsCallerComplex(notify)) {
                TAG_LOGE(AAFwkTag::DEFAULT, "ptr not found");
                return;
            }
            notify->OnRemoteStateChangedNotifyTask(str);
        };
        handler_->PostSyncTask(task, "OnRemoteStateChangedNotify");
        TAG_LOGD(AAFwkTag::DEFAULT, "end");
    }

    void OnRemoteStateChangedNotifyTask(const std::string &str)
    {
        HandleScope handle(releaseCallBackEngine_);
        TAG_LOGD(AAFwkTag::DEFAULT, "begin");
        if (jsRemoteStateChangedObj_ == nullptr) {
            TAG_LOGE(AAFwkTag::DEFAULT, "null jsRemoteStateChangedObj");
            return;
        }

        napi_value value = jsRemoteStateChangedObj_->GetNapiValue();
        napi_value callback = jsRemoteStateChangedObj_->GetNapiValue();
        napi_value args[] = { CreateJsValue(remoteStateChanegdEngine_, str) };
        napi_call_function(remoteStateChanegdEngine_, value, callback, 1, args, nullptr);
        StateReset();
        TAG_LOGD(AAFwkTag::DEFAULT, "end");
    }

    napi_value ReleaseCallInner(napi_env env, NapiCallbackInfo& info)
    {
        TAG_LOGD(AAFwkTag::DEFAULT, "called");
        if (callerCallBackObj_ == nullptr) {
            TAG_LOGE(AAFwkTag::DEFAULT, "null CallBacker");
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
            return CreateJsUndefined(env);
        }

        if (!releaseCallFunc_) {
            TAG_LOGE(AAFwkTag::DEFAULT, "null releaseFunc");
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
            return CreateJsUndefined(env);
        }
        callee_ = nullptr;
        callerCallBackObj_->SetCallBack(nullptr);
        ReleaseJsRemoteObj();
        int32_t innerErrorCode = releaseCallFunc_(callerCallBackObj_);
        if (innerErrorCode != ERR_OK) {
            TAG_LOGE(AAFwkTag::DEFAULT, "ReleaseAbility failed %{public}d", static_cast<int>(innerErrorCode));
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
            return CreateJsUndefined(env);
        }

        return CreateJsUndefined(env);
    }

    napi_value SetOnReleaseCallBackInner(napi_env env, NapiCallbackInfo& info)
    {
        TAG_LOGD(AAFwkTag::DEFAULT, "start");
        constexpr size_t argcOne = 1;
        if (info.argc < argcOne) {
            TAG_LOGE(AAFwkTag::DEFAULT, "Invalid argc");
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }
        bool isCallable = false;
        napi_is_callable(env, info.argv[0], &isCallable);
        if (!isCallable) {
            TAG_LOGE(AAFwkTag::DEFAULT, "IsCallable %{public}s", isCallable ? "true" : "false");
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        }

        if (callerCallBackObj_ == nullptr) {
            TAG_LOGE(AAFwkTag::DEFAULT, "null CallBacker");
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
        }

        auto param1 = info.argv[0];
        if (param1 == nullptr) {
            TAG_LOGE(AAFwkTag::DEFAULT, "null param1");
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
        }

        napi_ref ref = nullptr;
        napi_create_reference(releaseCallBackEngine_, param1, 1, &ref);
        jsReleaseCallBackObj_.reset(reinterpret_cast<NativeReference*>(ref));
        auto task = [notify = this] (const std::string &str) {
            if (!FindJsCallerComplexAndChangeState(notify, OBJSTATE::OBJ_EXECUTION)) {
                TAG_LOGE(AAFwkTag::DEFAULT, "address error");
                return;
            }
            notify->OnReleaseNotify(str);
        };
        callerCallBackObj_->SetOnRelease(task);
        return CreateJsUndefined(env);
    }

    napi_value SetOnRemoteStateChangedInner(napi_env env, NapiCallbackInfo& info)
    {
        TAG_LOGD(AAFwkTag::DEFAULT, "begin");
        constexpr size_t argcOne = 1;
        if (info.argc < argcOne) {
            TAG_LOGE(AAFwkTag::DEFAULT, "Invalid argc");
            ThrowTooFewParametersError(env);
        }
        bool isCallable = false;
        napi_is_callable(env, info.argv[0], &isCallable);
        if (!isCallable) {
            TAG_LOGE(AAFwkTag::DEFAULT, "IsCallable %{public}s", isCallable ? "true" : "false");
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        }

        if (callerCallBackObj_ == nullptr) {
            TAG_LOGE(AAFwkTag::DEFAULT, "null callBacker");
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
        }

        auto param1 = info.argv[0];
        if (param1 == nullptr) {
            TAG_LOGE(AAFwkTag::DEFAULT, "null param1");
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
        }

        napi_ref ref = nullptr;
        napi_create_reference(remoteStateChanegdEngine_, param1, 1, &ref);
        jsRemoteStateChangedObj_.reset(reinterpret_cast<NativeReference*>(ref));
        auto task = [notify = this] (const std::string &str) {
            TAG_LOGI(AAFwkTag::DEFAULT, "state changed");
            if (!FindJsCallerComplexAndChangeState(notify, OBJSTATE::OBJ_EXECUTION)) {
                TAG_LOGE(AAFwkTag::DEFAULT, "address error");
                return;
            }
            notify->OnRemoteStateChangedNotify(str);
        };
        callerCallBackObj_->SetOnRemoteStateChanged(task);
        TAG_LOGD(AAFwkTag::DEFAULT, "end");
        return CreateJsUndefined(env);
    }
public:
    static std::function<void(uintptr_t)> finalizeCallback_;
private:
    ReleaseCallFunc releaseCallFunc_;
    sptr<IRemoteObject> callee_;
    napi_env releaseCallBackEngine_;
    napi_env remoteStateChanegdEngine_;
    napi_env jsRemoteObjEnv_ = nullptr;
    std::shared_ptr<CallerCallBack> callerCallBackObj_;
    std::unique_ptr<NativeReference> jsReleaseCallBackObj_;
    std::unique_ptr<NativeReference> jsRemoteStateChangedObj_;
    std::unique_ptr<NativeReference> jsRemoteObj_;
    std::shared_ptr<AppExecFwk::EventHandler> handler_;
    std::mutex stateMechanismMutex_;
    OBJSTATE currentState_ = OBJSTATE::OBJ_NORMAL;

    static std::set<JsCallerComplex*> jsCallerComplexManagerList;
    static std::mutex jsCallerComplexMutex;
};

std::set<JsCallerComplex*> JsCallerComplex::jsCallerComplexManagerList;
std::mutex JsCallerComplex::jsCallerComplexMutex;
std::function<void(uintptr_t)> JsCallerComplex::finalizeCallback_;
} // nameless

napi_value CreateJsCallerComplex(
    napi_env env, ReleaseCallFunc releaseCallFunc, sptr<IRemoteObject> callee,
    std::shared_ptr<CallerCallBack> callerCallBack)
{
    TAG_LOGD(AAFwkTag::DEFAULT, "begin");
    if (callee == nullptr || callerCallBack == nullptr || releaseCallFunc == nullptr) {
        TAG_LOGE(AAFwkTag::DEFAULT, "%{public}s null",
            (callee == nullptr) ? ("callee") :
            ((releaseCallFunc == nullptr) ? ("releaseCallFunc") : ("callerCallBack")));
        return CreateJsUndefined(env);
    }

    napi_value object = nullptr;
    napi_create_object(env, &object);
    auto jsCaller = std::make_unique<JsCallerComplex>(env, releaseCallFunc, callee, callerCallBack);
    auto remoteObj = jsCaller->GetRemoteObject();
    if (remoteObj == nullptr) {
        TAG_LOGE(AAFwkTag::DEFAULT, "null remoteObj");
        return CreateJsUndefined(env);
    }

    auto jsRemoteObj = CreateJsCalleeRemoteObject(env, remoteObj);
    jsCaller->SetJsRemoteObj(env, jsRemoteObj);
    napi_wrap(env, object, jsCaller.release(), JsCallerComplex::Finalizer, nullptr, nullptr);
    napi_set_named_property(env, object, "callee", jsRemoteObj);
    const char *moduleName = "JsCallerComplex";
    BindNativeFunction(env, object, "release", moduleName, JsCallerComplex::JsReleaseCall);
    BindNativeFunction(env, object, "onRelease", moduleName, JsCallerComplex::JsSetOnReleaseCallBack);
    BindNativeFunction(env, object, "onRemoteStateChange", moduleName, JsCallerComplex::JsSetOnRemoteStateChanged);

    TAG_LOGD(AAFwkTag::DEFAULT, "end");
    return object;
}

napi_value CreateJsCalleeRemoteObject(napi_env env, sptr<IRemoteObject> callee)
{
    if (callee == nullptr) {
        TAG_LOGE(AAFwkTag::DEFAULT, "null callee");
        return CreateJsUndefined(env);
    }
    napi_value napiRemoteObject = NAPI_ohos_rpc_CreateJsRemoteObject(env, callee);
    if (napiRemoteObject == nullptr) {
        TAG_LOGE(AAFwkTag::DEFAULT, "null remoteObj");
    }
    return napiRemoteObject;
}

sptr<IRemoteObject> GetJsCallerRemoteObj(uintptr_t jsCallerComplex)
{
    auto callerPtr = reinterpret_cast<JsCallerComplex*>(jsCallerComplex);
    if (callerPtr == nullptr) {
        return nullptr;
    }
    return callerPtr->GetRemoteObject();
}

void SetFinalizeCallback(std::function<void(uintptr_t)> finalizeCallback)
{
    JsCallerComplex::finalizeCallback_ = finalizeCallback;
}
} // AbilityRuntime
} // OHOS
