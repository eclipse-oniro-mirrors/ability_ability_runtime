/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include "js_error_observer.h"

#include <cstdint>

#include "hilog_tag_wrapper.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "napi/native_api.h"

namespace OHOS {
namespace AbilityRuntime {
constexpr size_t ARGC_ONE = 1;
JsErrorObserver::JsErrorObserver(napi_env env) : env_(env) {}

JsErrorObserver::~JsErrorObserver() = default;

void JsErrorObserver::OnUnhandledException(const std::string errMsg)
{
    TAG_LOGD(AAFwkTag::JSNAPI, "called");
    std::weak_ptr<JsErrorObserver> thisWeakPtr(shared_from_this());
    std::unique_ptr<NapiAsyncTask::CompleteCallback> complete = std::make_unique<NapiAsyncTask::CompleteCallback>
        ([thisWeakPtr, errMsg](napi_env env, NapiAsyncTask &task, int32_t status) {
            std::shared_ptr<JsErrorObserver> jsObserver = thisWeakPtr.lock();
            if (jsObserver) {
                jsObserver->HandleOnUnhandledException(errMsg);
            }
        });
    napi_ref callback = nullptr;
    std::unique_ptr<NapiAsyncTask::ExecuteCallback> execute = nullptr;
    NapiAsyncTask::Schedule("JsErrorObserver::OnUnhandledException",
        env_, std::make_unique<NapiAsyncTask>(callback, std::move(execute), std::move(complete)));
}

void JsErrorObserver::HandleOnUnhandledException(const std::string &errMsg)
{
    TAG_LOGD(AAFwkTag::JSNAPI, "called");
    auto tmpMap = jsObserverObjectMap_;
    for (auto &item : tmpMap) {
        napi_value value = (item.second)->GetNapiValue();
        napi_value argv[] = { CreateJsValue(env_, errMsg) };
        CallJsFunction(value, "onUnhandledException", argv, ARGC_ONE);
    }
    tmpMap = jsObserverObjectMapSync_;
    for (auto &item : tmpMap) {
        napi_value value = (item.second)->GetNapiValue();
        napi_value argv[] = { CreateJsValue(env_, errMsg) };
        CallJsFunction(value, "onUnhandledException", argv, ARGC_ONE);
    }
}

void JsErrorObserver::CallJsFunction(napi_value obj, const char* methodName, napi_value const* argv, size_t argc)
{
    TAG_LOGD(AAFwkTag::JSNAPI, "call method:%{public}s", methodName);
    if (obj == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "null obj");
        return;
    }

    napi_value method = nullptr;
    napi_get_named_property(env_, obj, methodName, &method);
    if (method == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "null method");
        return;
    }
    napi_value callResult = nullptr;
    napi_call_function(env_, obj, method, argc, argv, &callResult);
}

void JsErrorObserver::AddJsObserverObject(const int32_t observerId, napi_value jsObserverObject, bool isSync)
{
    napi_ref ref = nullptr;
    napi_create_reference(env_, jsObserverObject, 1, &ref);
    if (isSync) {
        jsObserverObjectMapSync_.emplace(
            observerId, std::shared_ptr<NativeReference>(reinterpret_cast<NativeReference*>(ref)));
    } else {
        jsObserverObjectMap_.emplace(
            observerId, std::shared_ptr<NativeReference>(reinterpret_cast<NativeReference*>(ref)));
    }
}

bool JsErrorObserver::RemoveJsObserverObject(const int32_t observerId, bool isSync)
{
    bool result = false;
    if (isSync) {
        result = (jsObserverObjectMapSync_.erase(observerId) == 1);
    } else {
        result = (jsObserverObjectMap_.erase(observerId) == 1);
    }
    return result;
}

bool JsErrorObserver::IsEmpty()
{
    bool isEmpty = jsObserverObjectMap_.empty() && jsObserverObjectMapSync_.empty();
    return isEmpty;
}

void JsErrorObserver::OnExceptionObject(const AppExecFwk::ErrorObject &errorObj)
{
    TAG_LOGD(AAFwkTag::JSNAPI, "called");
    std::weak_ptr<JsErrorObserver> thisWeakPtr(shared_from_this());
    std::unique_ptr<NapiAsyncTask::CompleteCallback> complete = std::make_unique<NapiAsyncTask::CompleteCallback>
        ([thisWeakPtr, errorObj](napi_env env, NapiAsyncTask &task, int32_t status) {
            std::shared_ptr<JsErrorObserver> jsObserver = thisWeakPtr.lock();
            if (jsObserver) {
                jsObserver->HandleException(errorObj);
            }
        });
    napi_ref callback = nullptr;
    std::unique_ptr<NapiAsyncTask::ExecuteCallback> execute = nullptr;
    NapiAsyncTask::Schedule("JsErrorObserver::OnExceptionObject",
        env_, std::make_unique<NapiAsyncTask>(callback, std::move(execute), std::move(complete)));
}

void JsErrorObserver::HandleException(const AppExecFwk::ErrorObject &errorObj)
{
    TAG_LOGD(AAFwkTag::JSNAPI, "called");
    auto tmpMap = jsObserverObjectMap_;
    for (auto &item : tmpMap) {
        napi_value jsObj = (item.second)->GetNapiValue();
        napi_value jsValue[] = { CreateJsErrorObject(env_, errorObj) };
        CallJsFunction(jsObj, "onException", jsValue, ARGC_ONE);
    }
    tmpMap = jsObserverObjectMapSync_;
    for (auto &item : tmpMap) {
        napi_value jsObj = (item.second)->GetNapiValue();
        napi_value jsValue[] = { CreateJsErrorObject(env_, errorObj) };
        CallJsFunction(jsObj, "onException", jsValue, ARGC_ONE);
    }
}

napi_value JsErrorObserver::CreateJsErrorObject(napi_env env, const AppExecFwk::ErrorObject &errorObj)
{
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        TAG_LOGW(AAFwkTag::JSNAPI, "null obj");
        return objValue;
    }
    napi_set_named_property(env, objValue, "name", CreateJsValue(env, errorObj.name));
    napi_set_named_property(env, objValue, "message", CreateJsValue(env, errorObj.message));
    if (!errorObj.stack.empty()) {
        napi_set_named_property(env, objValue, "stack", CreateJsValue(env, errorObj.stack));
    }

    return objValue;
}
}  // namespace AbilityRuntime
}  // namespace OHOS
