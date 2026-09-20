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
#include "js_ui_extension_callback.h"
#include "ability_business_error.h"
#include "hilog_tag_wrapper.h"
#include "js_runtime_utils.h"
#include "napi/native_api.h"
#include "napi_common_util.h"
#ifdef SUPPORT_SCREEN
#include "ui_content.h"
#include "ws_common.h"
#endif // SUPPORT_SCREEN
#include "napi_common_want.h"

namespace OHOS {
namespace AbilityRuntime {
#ifdef SUPPORT_SCREEN
constexpr const char* ERROR_MSG_INNER = "Inner error.";
#endif // SUPPORT_SCREEN
JsUIExtensionCallback::~JsUIExtensionCallback()
{
    if (jsCallbackObject_ != nullptr) {
        uv_loop_t *loop = nullptr;
        napi_get_uv_event_loop(env_, &loop);
        if (loop != nullptr) {
            uv_work_t *work = new (std::nothrow) uv_work_t;
            if (work != nullptr) {
                work->data = reinterpret_cast<void *>(jsCallbackObject_.release());
                int ret = uv_queue_work(loop, work, [](uv_work_t *work) {},
                [](uv_work_t *work, int status) {
                    if (work == nullptr) {
                        return;
                    }
                    if (work->data == nullptr) {
                        delete work;
                        work = nullptr;
                        return;
                    }
                    delete reinterpret_cast<NativeReference *>(work->data);
                    work->data = nullptr;
                    delete work;
                    work = nullptr;
                });
                if (ret != 0) {
                    delete reinterpret_cast<NativeReference *>(work->data);
                    work->data = nullptr;
                    delete work;
                    work = nullptr;
                }
            }
        }
    }
    FreeNativeReference(onRequestSuccess_);
    FreeNativeReference(onRequestFailure_);
    FreeNativeReference(completionCallback_);
}

void JsUIExtensionCallback::FreeNativeReference(std::unique_ptr<NativeReference>& reference)
{
    if (reference  == nullptr) {
        return;
    }

    uv_loop_t *loop = nullptr;
    napi_get_uv_event_loop(env_, &loop);
    if (loop == nullptr) {
        return;
    }

    uv_work_t *work = new (std::nothrow) uv_work_t;
    if (work == nullptr) {
        return;
    }
    work->data = reinterpret_cast<void *>(reference.release());
    int ret = uv_queue_work(loop, work, [](uv_work_t *work) {},
    [](uv_work_t *work, int status) {
        if (work != nullptr) {
            if (work->data != nullptr) {
            delete reinterpret_cast<NativeReference *>(work->data);
            work->data = nullptr;
            }
            delete work;
            work = nullptr;
        }
    });
    if (ret != 0) {
        delete reinterpret_cast<NativeReference *>(work->data);
        work->data = nullptr;
        delete work;
        work = nullptr;
    }
}

#ifdef SUPPORT_SCREEN
void JsUIExtensionCallback::SetJsCallbackObject(napi_value jsCallbackObject)
{
    napi_ref ref = nullptr;
    napi_create_reference(env_, jsCallbackObject, 1, &ref);
    jsCallbackObject_ = std::unique_ptr<NativeReference>(reinterpret_cast<NativeReference*>(ref));
    if (jsCallbackObject_ == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null jsCallbackObject_");
    }
}

void JsUIExtensionCallback::OnError(int32_t number)
{
    TAG_LOGI(AAFwkTag::UI_EXT, "call");
    if (env_ == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null env_");
        return;
    }
    // js callback should run in js thread
    std::shared_ptr<JsUIExtensionCallback> jsUIExtensionCallback = shared_from_this();
    std::unique_ptr<NapiAsyncTask::CompleteCallback> complete = std::make_unique<NapiAsyncTask::CompleteCallback>
        ([jsUIExtensionCallback, number](napi_env env, NapiAsyncTask &task, int32_t status) {
            if (jsUIExtensionCallback != nullptr) {
                jsUIExtensionCallback->CallJsError(number);
                if (jsUIExtensionCallback->deferred_ != nullptr ||
                    jsUIExtensionCallback->completionCallback_ != nullptr) {
                    jsUIExtensionCallback->RejectAsyncResult(
                        static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER),
                        "StartAbilityByType failed.");
                }
            }
        });
    napi_ref callback = nullptr;
    std::unique_ptr<NapiAsyncTask::ExecuteCallback> execute = nullptr;
    NapiAsyncTask::Schedule("JsUIExtensionCallback::OnError:",
        env_, std::make_unique<NapiAsyncTask>(callback, std::move(execute), std::move(complete)));
    CloseModalUIExtension();
}
#endif // SUPPORT_SCREEN

void JsUIExtensionCallback::ClearAsyncResult()
{
    deferred_ = nullptr;
    if (completionCallback_ != nullptr) {
        FreeNativeReference(completionCallback_);
    }
}

void JsUIExtensionCallback::RejectAsyncResult(int32_t jsErrCode, const std::string& innerMsg)
{
    if (deferred_ != nullptr) {
        napi_value error = CreateJsError(env_, jsErrCode, innerMsg);
        napi_reject_deferred(env_, deferred_, error);
        deferred_ = nullptr;
    }
    if (completionCallback_ != nullptr) {
        napi_value callback = completionCallback_->GetNapiValue();
        if (callback != nullptr) {
            napi_value error = CreateJsError(env_, jsErrCode, innerMsg);
            napi_value undefined = CreateJsUndefined(env_);
            napi_value argv[] = { error, undefined };
            napi_status callStatus = napi_call_function(env_, nullptr, callback, ArraySize(argv), argv, nullptr);
            if (callStatus != napi_ok) {
                TAG_LOGE(AAFwkTag::UI_EXT, "RejectAsyncResult napi_call_function failed: %{public}d", callStatus);
            }
        }
        FreeNativeReference(completionCallback_);
    }
}

void JsUIExtensionCallback::OnAbilityByTypeResult(int32_t errorCode)
{
    TAG_LOGI(AAFwkTag::UI_EXT, "call, errorCode: %{public}d", errorCode);
    if (env_ == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null env_");
        return;
    }
    int32_t jsErrCode = static_cast<int32_t>(GetJsErrorCodeByNativeError(errorCode));
    std::string innerMsg = GetErrorMsgByNativeError(errorCode);
    std::shared_ptr<JsUIExtensionCallback> jsUIExtensionCallback = shared_from_this();
    std::unique_ptr<NapiAsyncTask::CompleteCallback> complete = std::make_unique<NapiAsyncTask::CompleteCallback>
        ([jsUIExtensionCallback, errorCode, jsErrCode, innerMsg](napi_env env, NapiAsyncTask &task, int32_t status) {
            if (jsUIExtensionCallback == nullptr) {
                return;
            }
            if (errorCode == 0) {
                if (jsUIExtensionCallback->deferred_ != nullptr) {
                    napi_value value = CreateJsUndefined(env);
                    napi_resolve_deferred(env, jsUIExtensionCallback->deferred_, value);
                    jsUIExtensionCallback->deferred_ = nullptr;
                }
                if (jsUIExtensionCallback->completionCallback_ != nullptr) {
                    napi_value callback = jsUIExtensionCallback->completionCallback_->GetNapiValue();
                    if (callback != nullptr) {
                        napi_value argv[] = { CreateJsNull(env), CreateJsUndefined(env) };
                        napi_status callStatus = napi_call_function(env, nullptr, callback,
                            ArraySize(argv), argv, nullptr);
                        if (callStatus != napi_ok) {
                            TAG_LOGE(AAFwkTag::UI_EXT,
                                "OnAbilityByTypeResult resolve napi_call_function failed: %{public}d", callStatus);
                        }
                    }
                    jsUIExtensionCallback->FreeNativeReference(jsUIExtensionCallback->completionCallback_);
                }
            } else {
                jsUIExtensionCallback->RejectAsyncResult(jsErrCode, innerMsg);
            }
        });
    napi_ref callback = nullptr;
    std::unique_ptr<NapiAsyncTask::ExecuteCallback> execute = nullptr;
    NapiAsyncTask::Schedule("JsUIExtensionCallback::OnAbilityByTypeResult:",
        env_, std::make_unique<NapiAsyncTask>(callback, std::move(execute), std::move(complete)));
}

void JsUIExtensionCallback::OnResult(int32_t resultCode, const AAFwk::Want &want)
{
    TAG_LOGI(AAFwkTag::UI_EXT, "call");
    if (env_ == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null env_");
        return;
    }
    // js callback should run in js thread
    std::shared_ptr<JsUIExtensionCallback> jsUIExtensionCallback = shared_from_this();
    std::unique_ptr<NapiAsyncTask::CompleteCallback> complete = std::make_unique<NapiAsyncTask::CompleteCallback>
        ([jsUIExtensionCallback, resultCode, want](napi_env env, NapiAsyncTask &task, int32_t status) {
            if (jsUIExtensionCallback != nullptr) {
                jsUIExtensionCallback->CallJsResult(resultCode, want);
            }
        });
    napi_ref callback = nullptr;
    std::unique_ptr<NapiAsyncTask::ExecuteCallback> execute = nullptr;
    NapiAsyncTask::Schedule("JsUIExtensionCallback::OnResult:",
        env_, std::make_unique<NapiAsyncTask>(callback, std::move(execute), std::move(complete)));
#ifdef SUPPORT_SCREEN
    CloseModalUIExtension();
#endif
}

void JsUIExtensionCallback::CallJsResult(int32_t resultCode, const AAFwk::Want &want)
{
    TAG_LOGI(AAFwkTag::UI_EXT, "call");
    HandleScope handleScope(env_);
    if (env_ == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null env_");
        return;
    }
    napi_value abilityResult = OHOS::AppExecFwk::WrapAbilityResult(env_, resultCode, want);
    if (abilityResult == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null abilityResult");
        return;
    }

    if (jsCallbackObject_ == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null jsCallbackObject_ ");
        return;
    }
    napi_value obj = jsCallbackObject_->GetNapiValue();
    if (obj == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null obj");
        return;
    }
    napi_value method = nullptr;
    napi_get_named_property(env_, obj, "onResult", &method);
    if (method == nullptr || AppExecFwk::IsTypeForNapiValue(env_, method, napi_undefined)
        || AppExecFwk::IsTypeForNapiValue(env_, method, napi_null)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null method");
        return;
    }

    napi_value argv[] = { abilityResult };
    napi_call_function(env_, obj, method, ArraySize(argv), argv, nullptr);
    TAG_LOGI(AAFwkTag::UI_EXT, "end");
}

void JsUIExtensionCallback::CallJsError(int32_t number)
{
    TAG_LOGI(AAFwkTag::UI_EXT, "call");
    if (env_ == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null env_ ");
        return;
    }
    std::string name;
    std::string message;
#ifdef SUPPORT_SCREEN
    if (number != static_cast<int32_t>(Rosen::WSError::WS_OK)) {
        number = static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER);
        name = ERROR_MSG_INNER;
        message = "StartAbilityByType failed.";
    }
#endif // SUPPORT_SCREEN
    napi_value nativeNumber = CreateJsValue(env_, number);
    napi_value nativeName = CreateJsValue(env_, name);
    napi_value nativeMessage = CreateJsValue(env_, message);
    if (jsCallbackObject_ == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null jsCallbackObject_");
        return;
    }
    napi_value obj = jsCallbackObject_->GetNapiValue();
    if (obj == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null obj");
        return;
    }
    napi_value method = nullptr;
    napi_get_named_property(env_, obj, "onError", &method);
    if (method == nullptr || AppExecFwk::IsTypeForNapiValue(env_, method, napi_undefined)
        || AppExecFwk::IsTypeForNapiValue(env_, method, napi_null)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null method");
        return;
    }

    napi_value argv[] = { nativeNumber, nativeName, nativeMessage };
    napi_call_function(env_, obj, method, ArraySize(argv), argv, nullptr);
    TAG_LOGI(AAFwkTag::UI_EXT, "end");
}

bool JsUIExtensionCallback::SetCompletionCallback(napi_env env, napi_value callback)
{
    AbilityRuntime::HandleScope handleScope(env);
    TAG_LOGI(AAFwkTag::UI_EXT, "call");
    if (env == nullptr || callback == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "invalid parameters");
        return false;
    }
    napi_ref ref = nullptr;
    napi_status status = napi_create_reference(env, callback, 1, &ref);
    if (status == napi_ok) {
        completionCallback_ = std::unique_ptr<NativeReference>(reinterpret_cast<NativeReference*>(ref));
        return true;
    }
    TAG_LOGE(AAFwkTag::UI_EXT, "create completionCallback_ failed: %{public}d", status);
    return false;
}

void JsUIExtensionCallback::SetCompletionHandler(napi_env env, napi_value completionHandler)
{
    AbilityRuntime::HandleScope handleScope(env);
    TAG_LOGI(AAFwkTag::UI_EXT, "call");
    if (env == nullptr || completionHandler == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "invalid parameters");
        return;
    }
    napi_value onSuccess = AppExecFwk::GetPropertyValueByPropertyName(
        env, completionHandler, "onRequestSuccess", napi_function);
    if (onSuccess != nullptr) {
        napi_ref succRef = nullptr;
        napi_status status = napi_create_reference(env, onSuccess, 1, &succRef);
        if (status == napi_ok) {
            onRequestSuccess_ = std::unique_ptr<NativeReference>(reinterpret_cast<NativeReference*>(succRef));
        } else {
            TAG_LOGE(AAFwkTag::CONTEXT, "create onRequestSuccess, failed: %{public}d", status);
        }
    }
    napi_value onFailure = AppExecFwk::GetPropertyValueByPropertyName(
        env, completionHandler, "onRequestFailure", napi_function);
    if (onFailure != nullptr) {
        napi_ref failRef = nullptr;
        napi_status status = napi_create_reference(env, onFailure, 1, &failRef);
        if (status == napi_ok) {
            onRequestFailure_ = std::unique_ptr<NativeReference>(reinterpret_cast<NativeReference*>(failRef));
        } else {
            TAG_LOGE(AAFwkTag::CONTEXT, "create onRequestFailure, failed: %{public}d", status);
        }
    }
}

void JsUIExtensionCallback::OnRequestSuccess(const std::string& name)
{
    TAG_LOGI(AAFwkTag::UI_EXT, "call");
    if (onRequestSuccess_ == nullptr || env_ == nullptr) {
        return;
    }
    napi_value callback = onRequestSuccess_->GetNapiValue();
    if (callback == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "invalid callback");
        return;
    }
    napi_value argv[] = { CreateJsValue(env_, name) };
    napi_status status = napi_call_function(env_, nullptr, callback, ArraySize(argv), argv, nullptr);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::CONTEXT, "call OnRequestSuccess, failed: %{public}d", status);
    }
}

void JsUIExtensionCallback::OnRequestFailure(const std::string& name,
    int32_t failureCode, const std::string& failureMessage)
{
    TAG_LOGI(AAFwkTag::UI_EXT, "call");
    if (onRequestFailure_ == nullptr || env_ == nullptr) {
        return;
    }
    napi_value callback = onRequestFailure_->GetNapiValue();
    if (callback == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "invalid callback");
        return;
    }
    napi_value argv[] = {
        CreateJsValue(env_, name),
        CreateJsValue(env_, failureCode),
        CreateJsValue(env_, failureMessage)
    };
    napi_status status = napi_call_function(env_, nullptr, callback, ArraySize(argv), argv, nullptr);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::CONTEXT, "call onRequestFailure, failed: %{public}d", status);
    }
}
}  // namespace AbilityRuntime
}  // namespace OHOS
