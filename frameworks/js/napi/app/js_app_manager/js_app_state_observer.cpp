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

#include "js_app_state_observer.h"
#include "hilog_tag_wrapper.h"
#include "js_runtime_utils.h"
#include "js_app_manager_utils.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "napi_common_util.h"
#include "ui_extension_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
constexpr size_t ARGC_ONE = 1;
std::unordered_map<int32_t, int32_t> JSAppStateObserver::innerStateToState_ = {
    {static_cast<int32_t>(AppExecFwk::ExtensionState::EXTENSION_STATE_CREATE),
        static_cast<int32_t>(AppExecFwk::AbilityState::ABILITY_STATE_CREATE)},
    {static_cast<int32_t>(AppExecFwk::ExtensionState::EXTENSION_STATE_READY),
        static_cast<int32_t>(AppExecFwk::AbilityState::ABILITY_STATE_READY)},
    {static_cast<int32_t>(AppExecFwk::ExtensionState::EXTENSION_STATE_CONNECTED),
        static_cast<int32_t>(AppExecFwk::AbilityState::ABILITY_STATE_CONNECTED)},
    {static_cast<int32_t>(AppExecFwk::ExtensionState::EXTENSION_STATE_DISCONNECTED),
        static_cast<int32_t>(AppExecFwk::AbilityState::ABILITY_STATE_DISCONNECTED)},
    {static_cast<int32_t>(AppExecFwk::ExtensionState::EXTENSION_STATE_TERMINATED),
        static_cast<int32_t>(AppExecFwk::AbilityState::ABILITY_STATE_TERMINATED)},
    {static_cast<int32_t>(AppExecFwk::ExtensionState::EXTENSION_STATE_FOREGROUND),
        static_cast<int32_t>(AppExecFwk::AbilityState::ABILITY_STATE_FOREGROUND)},
    {static_cast<int32_t>(AppExecFwk::ExtensionState::EXTENSION_STATE_BACKGROUND),
        static_cast<int32_t>(AppExecFwk::AbilityState::ABILITY_STATE_BACKGROUND)},
};

JSAppStateObserver::JSAppStateObserver(napi_env env) : env_(env) {}

JSAppStateObserver::~JSAppStateObserver() = default;

void JSAppStateObserver::OnForegroundApplicationChanged(const AppStateData &appStateData)
{
    TAG_LOGD(AAFwkTag::APPMGR, "bundleName:%{public}s, uid:%{public}d, state:%{public}d",
        appStateData.bundleName.c_str(), appStateData.uid, appStateData.state);
    if (!valid_) {
        TAG_LOGE(AAFwkTag::APPMGR, "invalid appmgr");
        return;
    }
    wptr<JSAppStateObserver> jsObserver = this;
    auto asyncTask = [jsObserver, appStateData, env = env_]() {
        HandleScope handleScope(env);
        sptr<JSAppStateObserver> jsObserverSptr = jsObserver.promote();
        if (!jsObserverSptr) {
            TAG_LOGW(AAFwkTag::APPMGR, "null jsObserverSptr");
            return;
        }
        jsObserverSptr->HandleOnForegroundApplicationChanged(appStateData);
    };
    napi_status ret = napi_send_event(env_, asyncTask, napi_eprio_immediate);
    if (ret != napi_status::napi_ok) {
        TAG_LOGE(AAFwkTag::APPMGR, "failed to send event");
    }
}

void JSAppStateObserver::HandleOnForegroundApplicationChanged(const AppStateData &appStateData)
{
    TAG_LOGD(AAFwkTag::APPMGR, "bundleName:%{public}s, uid:%{public}d, state:%{public}d",
        appStateData.bundleName.c_str(), appStateData.uid, appStateData.state);
    auto tmpMap = jsObserverObjectMap_;
    for (auto &item : tmpMap) {
        napi_value obj = (item.second)->GetNapiValue();
        napi_value argv[] = {CreateJsAppStateData(env_, appStateData)};
        CallJsFunction(obj, "onForegroundApplicationChanged", argv, ARGC_ONE);
    }
}

void JSAppStateObserver::OnAbilityStateChanged(const AbilityStateData &abilityStateData)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    if (!valid_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appMgr may has cancelled storage");
        return;
    }
    if (abilityStateData.isInnerNotify) {
        TAG_LOGD(AAFwkTag::APPMGR, "inner notify");
        return;
    }
    wptr<JSAppStateObserver> jsObserver = this;
    auto asyncTask = [jsObserver, abilityStateData, env = env_]() {
        HandleScope handleScope(env);
        sptr<JSAppStateObserver> jsObserverSptr = jsObserver.promote();
        if (!jsObserverSptr) {
            TAG_LOGW(AAFwkTag::APPMGR, "null jsObserverSptr");
            return;
        }
        jsObserverSptr->HandleOnAbilityStateChanged(abilityStateData);
    };
    napi_status ret = napi_send_event(env_, asyncTask, napi_eprio_immediate);
    if (ret != napi_status::napi_ok) {
        TAG_LOGE(AAFwkTag::APPMGR, "failed to send event");
    }
}

void JSAppStateObserver::HandleOnAbilityStateChanged(const AbilityStateData &abilityStateData)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    auto tmpMap = jsObserverObjectMap_;
    for (auto &item : tmpMap) {
        napi_value obj = (item.second)->GetNapiValue();
        napi_value argv[] = {CreateJsAbilityStateData(env_, abilityStateData)};
        CallJsFunction(obj, "onAbilityStateChanged", argv, ARGC_ONE);
    }
}

void JSAppStateObserver::OnExtensionStateChanged(const AbilityStateData &abilityStateData)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    if (!valid_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appMgr may destroyed");
        return;
    }
    AbilityStateData newAbilityStateData = abilityStateData;
    if (AAFwk::UIExtensionWrapper::IsUIExtension(
        static_cast<AppExecFwk::ExtensionAbilityType>(abilityStateData.extensionAbilityType))) {
        newAbilityStateData.abilityState = innerStateToState_[abilityStateData.abilityState];
    }
    wptr<JSAppStateObserver> jsObserver = this;
    auto asyncTask = [jsObserver, newAbilityStateData, env = env_]() {
        HandleScope handleScope(env);
        sptr<JSAppStateObserver> jsObserverSptr = jsObserver.promote();
        if (!jsObserverSptr) {
            TAG_LOGW(AAFwkTag::APPMGR, "null jsObserverSptr");
            return;
        }
        jsObserverSptr->HandleOnExtensionStateChanged(newAbilityStateData);
    };
    napi_status ret = napi_send_event(env_, asyncTask, napi_eprio_immediate);
    if (ret != napi_status::napi_ok) {
        TAG_LOGE(AAFwkTag::APPMGR, "failed to send event");
    }
}

void JSAppStateObserver::HandleOnExtensionStateChanged(const AbilityStateData &abilityStateData)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    auto tmpMap = jsObserverObjectMap_;
    for (auto &item : tmpMap) {
        napi_value obj = (item.second)->GetNapiValue();
        napi_value argv[] = {CreateJsAbilityStateData(env_, abilityStateData)};
        CallJsFunction(obj, "onAbilityStateChanged", argv, ARGC_ONE);
    }
}

void JSAppStateObserver::OnProcessCreated(const ProcessData &processData)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    if (!valid_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appmgr may has cancelled storage");
        return;
    }
    wptr<JSAppStateObserver> jsObserver = this;
    auto asyncTask = [jsObserver, processData, env = env_]() {
        HandleScope handleScope(env);
        sptr<JSAppStateObserver> jsObserverSptr = jsObserver.promote();
        if (!jsObserverSptr) {
            TAG_LOGW(AAFwkTag::APPMGR, "null jsObserverSptr");
            return;
        }
        jsObserverSptr->HandleOnProcessCreated(processData);
    };
    napi_status ret = napi_send_event(env_, asyncTask, napi_eprio_immediate);
    if (ret != napi_status::napi_ok) {
        TAG_LOGE(AAFwkTag::APPMGR, "failed to send event");
    }
}

void JSAppStateObserver::HandleOnProcessCreated(const ProcessData &processData)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    auto tmpMap = jsObserverObjectMap_;
    for (auto &item : tmpMap) {
        napi_value obj = (item.second)->GetNapiValue();
        napi_value argv[] = {CreateJsProcessData(env_, processData)};
        CallJsFunction(obj, "onProcessCreated", argv, ARGC_ONE);
    }
}

void JSAppStateObserver::OnProcessStateChanged(const ProcessData &processData)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    if (!valid_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appMgr may destroyed");
        return;
    }
    wptr<JSAppStateObserver> jsObserver = this;
    auto asyncTask = [jsObserver, processData, env = env_]() {
        HandleScope handleScope(env);
        sptr<JSAppStateObserver> jsObserverSptr = jsObserver.promote();
        if (!jsObserverSptr) {
            TAG_LOGW(AAFwkTag::APPMGR, "null jsObserverSptr");
            return;
        }
        jsObserverSptr->HandleOnProcessStateChanged(processData);
    };
    napi_status ret = napi_send_event(env_, asyncTask, napi_eprio_immediate);
    if (ret != napi_status::napi_ok) {
        TAG_LOGE(AAFwkTag::APPMGR, "failed to send event");
    }
}

void JSAppStateObserver::HandleOnProcessStateChanged(const ProcessData &processData)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    auto tmpMap = jsObserverObjectMap_;
    for (auto &item : tmpMap) {
        napi_value obj = (item.second)->GetNapiValue();
        napi_value argv[] = {CreateJsProcessData(env_, processData)};
        CallJsFunction(obj, "onProcessStateChanged", argv, ARGC_ONE);
    }
}

void JSAppStateObserver::OnProcessDied(const ProcessData &processData)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    if (!valid_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appmgr may destroyed");
        return;
    }
    wptr<JSAppStateObserver> jsObserver = this;
    auto asyncTask = [jsObserver, processData, env = env_]() {
        HandleScope handleScope(env);
        sptr<JSAppStateObserver> jsObserverSptr = jsObserver.promote();
        if (!jsObserverSptr) {
            TAG_LOGW(AAFwkTag::APPMGR, "null jsObserverSptr");
            return;
        }
        jsObserverSptr->HandleOnProcessDied(processData);
    };
    napi_status ret = napi_send_event(env_, asyncTask, napi_eprio_immediate);
    if (ret != napi_status::napi_ok) {
        TAG_LOGE(AAFwkTag::APPMGR, "failed to send event");
    }
}

void JSAppStateObserver::HandleOnProcessDied(const ProcessData &processData)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    auto tmpMap = jsObserverObjectMap_;
    for (auto &item : tmpMap) {
        napi_value obj = (item.second)->GetNapiValue();
        napi_value argv[] = {CreateJsProcessData(env_, processData)};
        CallJsFunction(obj, "onProcessDied", argv, ARGC_ONE);
    }
}

void JSAppStateObserver::OnAppStarted(const AppStateData &appStateData)
{
    TAG_LOGD(AAFwkTag::APPMGR, "bundleName:%{public}s, uid:%{public}d, state:%{public}d",
        appStateData.bundleName.c_str(), appStateData.uid, appStateData.state);
    if (!valid_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appMgr may destroyed");
        return;
    }
    wptr<JSAppStateObserver> jsObserver = this;
    auto asyncTask = [jsObserver, appStateData, env = env_]() {
        HandleScope handleScope(env);
        sptr<JSAppStateObserver> jsObserverSptr = jsObserver.promote();
        if (!jsObserverSptr) {
            TAG_LOGW(AAFwkTag::APPMGR, "null jsObserverSptr");
            return;
        }
        jsObserverSptr->HandleOnAppStarted(appStateData);
    };
    napi_status ret = napi_send_event(env_, asyncTask, napi_eprio_immediate);
    if (ret != napi_status::napi_ok) {
        TAG_LOGE(AAFwkTag::APPMGR, "failed to send event");
    }
}

void JSAppStateObserver::HandleOnAppStarted(const AppStateData &appStateData)
{
    TAG_LOGD(AAFwkTag::APPMGR,
        "bundleName:%{public}s, uid:%{public}d, state:%{public}d",
        appStateData.bundleName.c_str(), appStateData.uid, appStateData.state);
    auto tmpMap = jsObserverObjectMap_;
    for (auto &item : tmpMap) {
        if (item.second == nullptr) {
            continue;
        }
        napi_value obj = (item.second)->GetNapiValue();
        napi_value argv[] = {CreateJsAppStateData(env_, appStateData)};
        CallJsFunction(obj, "onAppStarted", argv, ARGC_ONE);
    }
}

void JSAppStateObserver::OnAppStopped(const AppStateData &appStateData)
{
    TAG_LOGD(AAFwkTag::APPMGR, "bundleName:%{public}s, uid:%{public}d, state:%{public}d",
        appStateData.bundleName.c_str(), appStateData.uid, appStateData.state);
    if (!valid_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appMgr may destroyed");
        return;
    }
    wptr<JSAppStateObserver> jsObserver = this;
    auto asyncTask = [jsObserver, appStateData, env = env_]() {
        HandleScope handleScope(env);
        sptr<JSAppStateObserver> jsObserverSptr = jsObserver.promote();
        if (!jsObserverSptr) {
            TAG_LOGW(AAFwkTag::APPMGR, "null jsObserverSptr");
            return;
        }
        jsObserverSptr->HandleOnAppStopped(appStateData);
    };
    napi_status ret = napi_send_event(env_, asyncTask, napi_eprio_immediate);
    if (ret != napi_status::napi_ok) {
        TAG_LOGE(AAFwkTag::APPMGR, "failed to send event");
    }
}

void JSAppStateObserver::HandleOnAppStopped(const AppStateData &appStateData)
{
    TAG_LOGD(AAFwkTag::APPMGR,
        "bundleName:%{public}s, uid:%{public}d, state:%{public}d",
        appStateData.bundleName.c_str(), appStateData.uid, appStateData.state);
    auto tmpMap = jsObserverObjectMap_;
    for (auto &item : tmpMap) {
        if (item.second == nullptr) {
            continue;
        }
        napi_value obj = (item.second)->GetNapiValue();
        napi_value argv[] = {CreateJsAppStateData(env_, appStateData)};
        CallJsFunction(obj, "onAppStopped", argv, ARGC_ONE);
    }
}

void JSAppStateObserver::CallJsFunction(
    napi_value value, const char *methodName, napi_value* argv, size_t argc)
{
    TAG_LOGD(AAFwkTag::APPMGR, "call method:%{public}s", methodName);
    if (value == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "Failed to get object");
        return;
    }

    napi_value method = nullptr;
    napi_get_named_property(env_, value, methodName, &method);
    if (method == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "get method failed");
        return;
    }
    napi_value callResult = nullptr;
    napi_call_function(env_, value, method, argc, argv, &callResult);
    TAG_LOGD(AAFwkTag::APPMGR, "end");
}

void JSAppStateObserver::AddJsObserverObject(const int32_t observerId, napi_value jsObserverObject)
{
    napi_ref ref = nullptr;
    napi_create_reference(env_, jsObserverObject, 1, &ref);
    jsObserverObjectMap_.emplace(observerId, std::shared_ptr<NativeReference>(reinterpret_cast<NativeReference*>(ref)));
}

bool JSAppStateObserver::RemoveJsObserverObject(const int32_t observerId)
{
    bool result = (jsObserverObjectMap_.erase(observerId) == 1);
    return result;
}

bool JSAppStateObserver::FindObserverByObserverId(const int32_t observerId)
{
    auto item = jsObserverObjectMap_.find(observerId);
    bool isExist = (item != jsObserverObjectMap_.end());
    return isExist;
}

size_t JSAppStateObserver::GetJsObserverMapSize()
{
    size_t length = jsObserverObjectMap_.size();
    return length;
}

void JSAppStateObserver::SetValid(const bool valid)
{
    valid_ = valid;
}
}  // namespace AbilityRuntime
}  // namespace OHOS
