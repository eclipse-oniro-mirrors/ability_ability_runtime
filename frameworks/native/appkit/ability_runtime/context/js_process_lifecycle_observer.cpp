/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "js_process_lifecycle_observer.h"

#include "hilog_tag_wrapper.h"
#include "js_runtime_utils.h"
#include "napi/native_api.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr char ON_PROCESS_CREATED[] = "onProcessCreated";
constexpr char ON_PROCESS_DIED[] = "onProcessDied";
}  // namespace

JsProcessLifecycleObserver::JsProcessLifecycleObserver(napi_env env) : env_(env) {}

void JsProcessLifecycleObserver::CallJsFunction(napi_value value, const char *methodName,
    const napi_value *argv, size_t argc)
{
    if (value == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null value");
        return;
    }
    napi_value method = nullptr;
    napi_get_named_property(env_, value, methodName, &method);
    if (method == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null method %{public}s", methodName);
        return;
    }
    napi_call_function(env_, value, method, argc, argv, nullptr);
}

napi_value CreateJsProcessData(napi_env env, const OHOS::AppExecFwk::ProcessData &processData)
{
    napi_value object = nullptr;
    napi_create_object(env, &object);
    if (object == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null obj");
        return nullptr;
    }
    napi_set_named_property(env, object, "bundleName", CreateJsValue(env, processData.bundleName));
    napi_set_named_property(env, object, "pid", CreateJsValue(env, processData.pid));
    napi_set_named_property(env, object, "uid", CreateJsValue(env, processData.uid));
    napi_set_named_property(env, object, "state", CreateJsValue(env, processData.state));
    napi_set_named_property(env, object, "isContinuousTask", CreateJsValue(env, processData.isContinuousTask));
    napi_set_named_property(env, object, "isKeepAlive", CreateJsValue(env, processData.isKeepAlive));
    return object;
}

void JsProcessLifecycleObserver::HandleOnAppProcessStateChangeEvent(
    const OHOS::AppExecFwk::ProcessData &processData, const std::string &methodName)
{
    TAG_LOGD(AAFwkTag::APPKIT, "MethodName = %{public}s, pid:%{public}d", methodName.c_str(), processData.pid);
    std::set<std::shared_ptr<NativeReference>> observersCopy;
    {
        std::lock_guard<std::mutex> lock(observerLock_);
        observersCopy = observers_;
    }
    napi_value argv[] = { CreateJsProcessData(env_, processData) };
    for (const auto &callback : observersCopy) {
        if (!callback) {
            TAG_LOGE(AAFwkTag::APPKIT, "Invalid jsObserver");
            continue;
        }
        auto obj = callback->GetNapiValue();
        if (!CheckTypeForNapiValue(env_, obj, napi_object)) {
            TAG_LOGE(AAFwkTag::APPKIT, "get object failed");
            continue;
        }
        CallJsFunction(obj, methodName.c_str(), argv, 1);
    }
}

void JsProcessLifecycleObserver::OnProcessCreated(const OHOS::AppExecFwk::ProcessData &processData)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    wptr<JsProcessLifecycleObserver> self(this);
    auto asyncTask = [self, processData, env = env_]() {
        HandleScope handleScope(env);
        sptr<JsProcessLifecycleObserver> observer = self.promote();
        if (!observer) {
            TAG_LOGW(AAFwkTag::APPKIT, "null observer");
            return;
        }
        observer->HandleOnAppProcessStateChangeEvent(processData, ON_PROCESS_CREATED);
    };
    napi_status ret = napi_send_event(env_, asyncTask, napi_eprio_immediate);
    if (ret != napi_status::napi_ok) {
        TAG_LOGE(AAFwkTag::APPKIT, "failed to send event");
    }
}

void JsProcessLifecycleObserver::OnProcessDied(const OHOS::AppExecFwk::ProcessData &processData)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    wptr<JsProcessLifecycleObserver> self(this);
    auto asyncTask = [self, processData, env = env_]() {
        HandleScope handleScope(env);
        sptr<JsProcessLifecycleObserver> observer = self.promote();
        if (!observer) {
            TAG_LOGW(AAFwkTag::APPKIT, "null observer");
            return;
        }
        observer->HandleOnAppProcessStateChangeEvent(processData, ON_PROCESS_DIED);
    };
    napi_status ret = napi_send_event(env_, asyncTask, napi_eprio_immediate);
    if (ret != napi_status::napi_ok) {
        TAG_LOGE(AAFwkTag::APPKIT, "failed to send event");
    }
}

void JsProcessLifecycleObserver::Register(napi_value jsObserver)
{
    if (env_ == nullptr || jsObserver == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null env or jsObserver");
        return;
    }
    napi_ref ref = nullptr;
    napi_create_reference(env_, jsObserver, 1, &ref);
    std::lock_guard<std::mutex> lock(observerLock_);
    observers_.emplace(std::shared_ptr<NativeReference>(reinterpret_cast<NativeReference*>(ref)));
}

bool JsProcessLifecycleObserver::UnRegister(napi_value jsObserver)
{
    if (jsObserver == nullptr) {
        TAG_LOGI(AAFwkTag::APPKIT, "null jsObserver, unregister all");
        std::lock_guard<std::mutex> lock(observerLock_);
        observers_.clear();
        return true;
    }
    std::lock_guard<std::mutex> lock(observerLock_);
    for (auto &callback : observers_) {
        if (!callback) {
            TAG_LOGE(AAFwkTag::APPKIT, "Invalid jsCallback");
            continue;
        }
        napi_value value = callback->GetNapiValue();
        if (value == nullptr) {
            TAG_LOGE(AAFwkTag::APPKIT, "null value");
            continue;
        }
        bool isEqual = false;
        napi_strict_equals(env_, value, jsObserver, &isEqual);
        if (isEqual) {
            return observers_.erase(callback) == 1;
        }
    }
    return false;
}

bool JsProcessLifecycleObserver::IsEmpty()
{
    std::lock_guard<std::mutex> lock(observerLock_);
    return observers_.empty();
}
}  // namespace AbilityRuntime
}  // namespace OHOS
