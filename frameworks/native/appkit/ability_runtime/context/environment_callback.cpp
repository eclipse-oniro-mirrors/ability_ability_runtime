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

#include "environment_callback.h"

#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "js_data_struct_converter.h"
#include "js_runtime_utils.h"

namespace OHOS {
namespace AbilityRuntime {
JsEnvironmentCallback::JsEnvironmentCallback(napi_env env)
    : env_(env)
{
}

int32_t JsEnvironmentCallback::serialNumber_ = 0;

void JsEnvironmentCallback::CallConfigurationUpdatedInner(const std::string &methodName,
    const AppExecFwk::Configuration &config, const std::map<int32_t, std::shared_ptr<NativeReference>> &callbacks)
{
    TAG_LOGD(AAFwkTag::APPKIT, "methodName = %{public}s", methodName.c_str());
    HandleScope scope(env_);
    for (auto &callback : callbacks) {
        if (!callback.second) {
            TAG_LOGE(AAFwkTag::APPKIT, "invalid jsCallback");
            return;
        }

        auto obj = callback.second->GetNapiValue();
        if (!CheckTypeForNapiValue(env_, obj, napi_object)) {
            TAG_LOGE(AAFwkTag::APPKIT, "get object failed");
            return;
        }

        napi_value method = nullptr;
        napi_get_named_property(env_, obj, methodName.data(), &method);
        if (method == nullptr) {
            TAG_LOGE(AAFwkTag::APPKIT, "null method %{public}s", methodName.data());
            return;
        }

        napi_value argv[] = { CreateJsConfiguration(env_, config) };
        napi_call_function(env_, obj, method, ArraySize(argv), argv, nullptr);
    }
}

void JsEnvironmentCallback::OnConfigurationUpdated(const AppExecFwk::Configuration &config)
{
    std::weak_ptr<JsEnvironmentCallback> thisWeakPtr(shared_from_this());
    auto callbacks = callbacks_;
    auto callbacksSync = callbacksSync_;
    // When vsync_first is enabled, events marked with napi_barrier_need are
    // treated as barriers and will execute *before* the next vsync.
    napi_send_event(env_, [thisWeakPtr, config, callbacks, callbacksSync]() {
        std::shared_ptr<JsEnvironmentCallback> jsEnvCallback = thisWeakPtr.lock();
        if (jsEnvCallback) {
            jsEnvCallback->CallConfigurationUpdatedInner("onConfigurationUpdated", config, callbacks);
            jsEnvCallback->CallConfigurationUpdatedInner("onConfigurationUpdated", config, callbacksSync);
        }
    }, napi_eprio_low, "JsEnvironmentCallback::OnConfigurationUpdated", napi_barrier_need);
}

void JsEnvironmentCallback::CallMemoryLevelInner(const std::string &methodName, const int level,
    const std::map<int32_t, std::shared_ptr<NativeReference>> &callbacks)
{
    TAG_LOGD(AAFwkTag::APPKIT, "methodName = %{public}s", methodName.c_str());
    HandleScope scope(env_);
    for (auto &callback : callbacks) {
        if (!callback.second) {
            TAG_LOGE(AAFwkTag::APPKIT, "Invalid jsCallback");
            return;
        }

        auto obj = callback.second->GetNapiValue();
        if (!CheckTypeForNapiValue(env_, obj, napi_object)) {
            TAG_LOGE(AAFwkTag::APPKIT, "get object failed");
            return;
        }

        napi_value method = nullptr;
        napi_get_named_property(env_, obj, methodName.data(), &method);
        if (method == nullptr) {
            TAG_LOGE(AAFwkTag::APPKIT, "null method %{public}s", methodName.data());
            return;
        }

        napi_value argv[] = { CreateJsValue(env_, level) };
        napi_call_function(env_, obj, method, ArraySize(argv), argv, nullptr);
    }
}

void JsEnvironmentCallback::OnMemoryLevel(const int level)
{
    std::weak_ptr<JsEnvironmentCallback> thisWeakPtr(shared_from_this());
    std::unique_ptr<NapiAsyncTask::CompleteCallback> complete = std::make_unique<NapiAsyncTask::CompleteCallback>(
        [thisWeakPtr, level, callbacks = callbacks_, callbacksSync = callbacksSync_]
        (napi_env env, NapiAsyncTask &task, int32_t status) {
            std::shared_ptr<JsEnvironmentCallback> jsEnvCallback = thisWeakPtr.lock();
            if (jsEnvCallback) {
                jsEnvCallback->CallMemoryLevelInner("onMemoryLevel", level, callbacks);
                jsEnvCallback->CallMemoryLevelInner("onMemoryLevel", level, callbacksSync);
            }
        }
    );
    napi_ref callback = nullptr;
    std::unique_ptr<NapiAsyncTask::ExecuteCallback> execute = nullptr;
    NapiAsyncTask::Schedule("JsEnvironmentCallback::OnMemoryLevel",
        env_, std::make_unique<NapiAsyncTask>(callback, std::move(execute), std::move(complete)));
}

int32_t JsEnvironmentCallback::Register(napi_value jsCallback, bool isSync)
{
    if (env_ == nullptr) {
        return -1;
    }
    int32_t callbackId = serialNumber_;
    if (serialNumber_ < INT32_MAX) {
        serialNumber_++;
    } else {
        serialNumber_ = 0;
    }
    napi_ref ref = nullptr;
    napi_create_reference(env_, jsCallback, 1, &ref);
    if (isSync) {
        callbacksSync_.emplace(callbackId, std::shared_ptr<NativeReference>(reinterpret_cast<NativeReference*>(ref)));
    } else {
        callbacks_.emplace(callbackId, std::shared_ptr<NativeReference>(reinterpret_cast<NativeReference*>(ref)));
    }
    return callbackId;
}

bool JsEnvironmentCallback::UnRegister(int32_t callbackId, bool isSync)
{
    TAG_LOGD(AAFwkTag::APPKIT, "callbackId : %{public}d", callbackId);
    if (isSync) {
        auto it = callbacksSync_.find(callbackId);
        if (it == callbacksSync_.end()) {
            TAG_LOGE(AAFwkTag::APPKIT, "callbackId: %{public}d not in callbacksSync_", callbackId);
            return false;
        }
        TAG_LOGD(AAFwkTag::APPKIT, "callbacksSync_.callbackId : %{public}d", it->first);
        return callbacksSync_.erase(callbackId) == 1;
    }
    auto it = callbacks_.find(callbackId);
    if (it == callbacks_.end()) {
        TAG_LOGE(AAFwkTag::APPKIT, "callbackId: %{public}d not in callbacks_", callbackId);
        return false;
    }
    TAG_LOGD(AAFwkTag::APPKIT, "callbacks_.callbackId : %{public}d", it->first);
    return callbacks_.erase(callbackId) == 1;
}

bool JsEnvironmentCallback::IsEmpty() const
{
    return callbacks_.empty() && callbacksSync_.empty();
}
}  // namespace AbilityRuntime
}  // namespace OHOS
