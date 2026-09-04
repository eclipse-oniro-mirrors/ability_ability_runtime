/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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
#include "js_app_foreground_state_observer.h"

#include "hilog_tag_wrapper.h"
#include "js_app_manager_utils.h"
#include "js_runtime_utils.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr size_t ARGC_ONE = 1;
}
JSAppForegroundStateObserver::JSAppForegroundStateObserver(napi_env env) : env_(env) {}

void JSAppForegroundStateObserver::OnAppStateChanged(const AppStateData &appStateData)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    if (!valid_) {
        TAG_LOGE(AAFwkTag::APPMGR, "invalid appMgr");
        return;
    }
    wptr<JSAppForegroundStateObserver> self = this;
    std::unique_ptr<NapiAsyncTask::CompleteCallback> complete = std::make_unique<NapiAsyncTask::CompleteCallback>(
        [self, appStateData](napi_env env, NapiAsyncTask &task, int32_t status) {
            sptr<JSAppForegroundStateObserver> jsObserver = self.promote();
            if (jsObserver == nullptr) {
                TAG_LOGE(AAFwkTag::APPMGR, "null observer");
                return;
            }
            jsObserver->HandleOnAppStateChanged(appStateData);
        });
    napi_ref callback = nullptr;
    NapiAsyncTask::Schedule("JSAppForegroundStateObserver::OnAppStateChanged", env_,
        std::make_unique<NapiAsyncTask>(callback, nullptr, std::move(complete)));
}

void JSAppForegroundStateObserver::HandleOnAppStateChanged(const AppStateData &appStateData)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    std::lock_guard<std::mutex> lock(jsObserverObjectSetLock_);
    for (auto &item : jsObserverObjectSet_) {
        if (item == nullptr) {
            TAG_LOGE(AAFwkTag::APPMGR, "null observer item");
            continue;
        }
        napi_value obj = item->GetNapiValue();
        napi_value argv[] = { CreateJsAppStateData(env_, appStateData) };
        CallJsFunction(obj, "onAppStateChanged", argv, ARGC_ONE);
    }
}

void JSAppForegroundStateObserver::CallJsFunction(
    const napi_value value, const char *methodName, const napi_value *argv, const size_t argc)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    if (value == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null value");
        return;
    }

    napi_value method = nullptr;
    napi_get_named_property(env_, value, methodName, &method);
    if (method == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null method");
        return;
    }
    napi_value callResult = nullptr;
    napi_status status = napi_call_function(env_, value, method, argc, argv, &callResult);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::APPMGR, "call failed %{public}d", status);
    }
    TAG_LOGD(AAFwkTag::APPMGR, "end");
}

void JSAppForegroundStateObserver::AddJsObserverObject(const napi_value &jsObserverObject)
{
    if (jsObserverObject == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null observer");
        return;
    }

    if (GetObserverObject(jsObserverObject) == nullptr) {
        std::lock_guard<std::mutex> lock(jsObserverObjectSetLock_);
        napi_ref ref = nullptr;
        napi_status status = napi_create_reference(env_, jsObserverObject, 1, &ref);
        if (status != napi_ok || ref == nullptr) {
            TAG_LOGE(AAFwkTag::APPMGR, "napi_create_reference failed: %{public}d", status);
            return;
        }
        jsObserverObjectSet_.emplace(std::shared_ptr<NativeReference>(reinterpret_cast<NativeReference *>(ref)));
    } else {
        TAG_LOGD(AAFwkTag::APPMGR, "observer exist");
    }
}

void JSAppForegroundStateObserver::RemoveAllJsObserverObjects()
{
    std::lock_guard<std::mutex> lock(jsObserverObjectSetLock_);
    jsObserverObjectSet_.clear();
}

void JSAppForegroundStateObserver::RemoveJsObserverObject(const napi_value &jsObserverObject)
{
    if (jsObserverObject == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null observer");
        return;
    }

    auto observer = GetObserverObject(jsObserverObject);
    if (observer != nullptr) {
        std::lock_guard<std::mutex> lock(jsObserverObjectSetLock_);
        jsObserverObjectSet_.erase(observer);
    }
}

std::shared_ptr<NativeReference> JSAppForegroundStateObserver::GetObserverObject(const napi_value &jsObserverObject)
{
    if (jsObserverObject == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null observer");
        return nullptr;
    }

    std::lock_guard<std::mutex> lock(jsObserverObjectSetLock_);
    for (auto &observer : jsObserverObjectSet_) {
        if (observer == nullptr) {
            TAG_LOGE(AAFwkTag::APPMGR, "null observer");
            continue;
        }

        napi_value value = observer->GetNapiValue();
        if (value == nullptr) {
            TAG_LOGE(AAFwkTag::APPMGR, "null value");
            continue;
        }

        bool isEqual = false;
        napi_strict_equals(env_, value, jsObserverObject, &isEqual);
        if (isEqual) {
            return observer;
        }
    }
    return nullptr;
}

void JSAppForegroundStateObserver::SetValid(bool valid)
{
    valid_ = valid;
}

bool JSAppForegroundStateObserver::IsEmpty()
{
    std::lock_guard<std::mutex> lock(jsObserverObjectSetLock_);
    return jsObserverObjectSet_.empty();
}
} // namespace AbilityRuntime
} // namespace OHOS
