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
#include "js_focused_application_observer.h"

#include "hilog_tag_wrapper.h"
#include "js_runtime_utils.h"
#include "napi/native_api.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr size_t ARGC_ONE = 1;

napi_value CreateJsFocusedApplicationInfo(napi_env env, const FocusedApplicationInfo &focusedApplicationInfo)
{
    napi_value value = nullptr;
    napi_status status = napi_create_object(env, &value);
    if (status != napi_ok || value == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "napi_create_object failed: %{public}d", status);
        return nullptr;
    }
    napi_value focusedUidVal = CreateJsValue(env, focusedApplicationInfo.focusedUid);
    napi_value focusedBundleNameVal = CreateJsValue(env, focusedApplicationInfo.focusedBundleName);
    if (napi_set_named_property(env, value, "focusedUid", focusedUidVal) != napi_ok ||
        napi_set_named_property(env, value, "focusedBundleName", focusedBundleNameVal) != napi_ok) {
        TAG_LOGE(AAFwkTag::APPMGR, "napi_set_named_property failed");
        return nullptr;
    }
    return value;
}
}
JSFocusedApplicationObserver::JSFocusedApplicationObserver(napi_env env) : env_(env) {}

void JSFocusedApplicationObserver::OnFocusedApplicationChanged(const FocusedApplicationInfo &focusedApplicationInfo)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    if (!valid_.load()) {
        TAG_LOGE(AAFwkTag::APPMGR, "invalid observer");
        return;
    }
    wptr<JSFocusedApplicationObserver> self = this;
    auto asyncTask = [self, focusedApplicationInfo, env = env_]() {
        HandleScope handleScope(env);
        sptr<JSFocusedApplicationObserver> jsObserver = self.promote();
        if (jsObserver == nullptr) {
            TAG_LOGE(AAFwkTag::APPMGR, "null observer");
            return;
        }
        jsObserver->HandleOnFocusedApplicationChanged(focusedApplicationInfo);
    };
    napi_status ret = napi_send_event(env_, asyncTask, napi_eprio_high);
    if (ret != napi_status::napi_ok) {
        TAG_LOGE(AAFwkTag::APPMGR, "failed to send event");
    }
}

void JSFocusedApplicationObserver::HandleOnFocusedApplicationChanged(
    const FocusedApplicationInfo &focusedApplicationInfo)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    std::vector<std::shared_ptr<NativeReference>> observers;
    {
        std::lock_guard<std::mutex> lock(jsObserverMapLock_);
        for (const auto &entry : jsObserverMap_) {
            if (entry.second != nullptr) {
                observers.push_back(entry.second);
            } else {
                TAG_LOGE(AAFwkTag::APPMGR, "null observer item");
            }
        }
    }
    napi_value jsData = CreateJsFocusedApplicationInfo(env_, focusedApplicationInfo);
    if (jsData == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null jsData, skip all observers");
        return;
    }
    for (const auto &item : observers) {
        napi_value obj = item->GetNapiValue();
        napi_value argv[] = { jsData };
        if (argv[0] == nullptr) {
            TAG_LOGE(AAFwkTag::APPMGR, "null argv, skip");
            continue;
        }
        CallJsFunction(obj, "onFocusedApplicationChanged", argv, ARGC_ONE);
    }
}

void JSFocusedApplicationObserver::CallJsFunction(napi_value value, const char *methodName, const napi_value *argv,
    size_t argc)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    if (value == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null value");
        return;
    }

    napi_value method = nullptr;
    napi_status status = napi_get_named_property(env_, value, methodName, &method);
    if (status != napi_ok || method == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "get method failed: %{public}d", status);
        return;
    }
    napi_valuetype type = napi_undefined;
    status = napi_typeof(env_, method, &type);
    if (status != napi_ok || type != napi_function) {
        TAG_LOGE(AAFwkTag::APPMGR, "method is not a function");
        return;
    }
    napi_value callResult = nullptr;
    status = napi_call_function(env_, value, method, argc, argv, &callResult);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::APPMGR, "call failed %{public}d", status);
    }
    TAG_LOGD(AAFwkTag::APPMGR, "end");
}

void JSFocusedApplicationObserver::AddJsObserverObjectWithId(int32_t observerId, const napi_value &jsObserverObject)
{
    if (jsObserverObject == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null observer");
        return;
    }
    std::lock_guard<std::mutex> lock(jsObserverMapLock_);
    if (jsObserverMap_.find(observerId) != jsObserverMap_.end()) {
        TAG_LOGD(AAFwkTag::APPMGR, "observerId exist: %{public}d", observerId);
        return;
    }
    napi_ref ref = nullptr;
    napi_status status = napi_create_reference(env_, jsObserverObject, 1, &ref);
    if (status != napi_ok || ref == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "napi_create_reference failed: %{public}d", status);
        return;
    }
    jsObserverMap_.emplace(observerId,
        std::shared_ptr<NativeReference>(reinterpret_cast<NativeReference *>(ref)));
}

bool JSFocusedApplicationObserver::RemoveJsObserverObjectById(int32_t observerId)
{
    std::lock_guard<std::mutex> lock(jsObserverMapLock_);
    return jsObserverMap_.erase(observerId) > 0;
}

void JSFocusedApplicationObserver::SetValid(bool valid)
{
    valid_.store(valid);
}

bool JSFocusedApplicationObserver::IsEmpty()
{
    std::lock_guard<std::mutex> lock(jsObserverMapLock_);
    return jsObserverMap_.empty();
}
}  // namespace AbilityRuntime
}  // namespace OHOS
