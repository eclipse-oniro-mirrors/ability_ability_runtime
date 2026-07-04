/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "js_mission_listener.h"

#include <memory>

#include "hilog_tag_wrapper.h"
#include "js_runtime_utils.h"

#ifdef SUPPORT_SCREEN
#include "pixel_map_napi.h"
#endif

namespace OHOS {
namespace AbilityRuntime {
constexpr size_t ARGC_ONE = 1;

void JsMissionListener::OnMissionCreated(int32_t missionId)
{
    CallJsMethod("onMissionCreated", missionId);
}

void JsMissionListener::OnMissionDestroyed(int32_t missionId)
{
    CallJsMethod("onMissionDestroyed", missionId);
}

void JsMissionListener::OnMissionSnapshotChanged(int32_t missionId)
{
    CallJsMethod("onMissionSnapshotChanged", missionId);
}

void JsMissionListener::OnMissionMovedToFront(int32_t missionId)
{
    CallJsMethod("onMissionMovedToFront", missionId);
}

void JsMissionListener::OnMissionClosed(int32_t missionId)
{
    CallJsMethod("onMissionClosed", missionId);
}

void JsMissionListener::OnMissionLabelUpdated(int32_t missionId)
{
    CallJsMethod("onMissionLabelUpdated", missionId);
}

void JsMissionListener::AddJsListenerObject(int32_t listenerId, napi_value jsListenerObject, bool isSync)
{
    napi_ref ref = nullptr;
    if (isSync) {
        std::lock_guard<std::mutex> lock(jsListenerObjectMapLock_);
        napi_status status = napi_create_reference(env_, jsListenerObject, 1, &ref);
        if (status != napi_ok || ref == nullptr) {
            TAG_LOGE(AAFwkTag::MISSION, "create reference failed, status:%{public}d", status);
            return;
        }
        jsListenerObjectMapSync_.emplace(
            listenerId, std::shared_ptr<NativeReference>(reinterpret_cast<NativeReference*>(ref)));
    } else {
        std::lock_guard<std::mutex> lock(jsListenerObjectMapLock_);
        napi_status status = napi_create_reference(env_, jsListenerObject, 1, &ref);
        if (status != napi_ok || ref == nullptr) {
            TAG_LOGE(AAFwkTag::MISSION, "create reference failed, status:%{public}d", status);
            return;
        }
        jsListenerObjectMap_.emplace(
            listenerId, std::shared_ptr<NativeReference>(reinterpret_cast<NativeReference*>(ref)));
    }
}

bool JsMissionListener::RemoveJsListenerObject(int32_t listenerId, bool isSync)
{
    bool result = false;
    if (isSync) {
        std::lock_guard<std::mutex> lock(jsListenerObjectMapLock_);
        result = (jsListenerObjectMapSync_.erase(listenerId) == 1);
    } else {
        std::lock_guard<std::mutex> lock(jsListenerObjectMapLock_);
        result = (jsListenerObjectMap_.erase(listenerId) == 1);
    }
    return result;
}

bool JsMissionListener::IsEmpty()
{
    std::lock_guard<std::mutex> lock(jsListenerObjectMapLock_);
    return jsListenerObjectMap_.empty() && jsListenerObjectMapSync_.empty();
}

void JsMissionListener::CallJsMethod(const std::string &methodName, int32_t missionId)
{
    TAG_LOGD(AAFwkTag::MISSION, "methodName:%{public}s", methodName.c_str());
    if (env_ == nullptr) {
        TAG_LOGE(AAFwkTag::MISSION, "null env_");
        return;
    }

    // js callback should run in js thread
    wptr<JsMissionListener> jsMissionListener = this;
    std::unique_ptr<NapiAsyncTask::CompleteCallback> complete = std::make_unique<NapiAsyncTask::CompleteCallback>
        ([jsMissionListener, methodName, missionId](napi_env env, NapiAsyncTask &task, int32_t status) {
            sptr<JsMissionListener> jsMissionListenerSptr = jsMissionListener.promote();
            if (jsMissionListener != nullptr) {
                jsMissionListener->CallJsMethodInner(methodName, missionId);
            }
        });
    napi_ref callback = nullptr;
    std::unique_ptr<NapiAsyncTask::ExecuteCallback> execute = nullptr;
    NapiAsyncTask::Schedule("JsMissionListener::CallJsMethod:" + methodName,
        env_, std::make_unique<NapiAsyncTask>(callback, std::move(execute), std::move(complete)));
}

void JsMissionListener::CallJsMethodInner(const std::string &methodName, int32_t missionId)
{
    // jsListenerObjectMap_ may be changed in env_->CallFunction()
    HandleScope handleScope(env_);
    std::map<int32_t, std::shared_ptr<NativeReference>> tmpMap;
    {
        std::lock_guard<std::mutex> lock(jsListenerObjectMapLock_);
        tmpMap = jsListenerObjectMap_;
    }
    for (auto &item : tmpMap) {
        napi_value obj = (item.second)->GetNapiValue();
        napi_value argv[] = { CreateJsValue(env_, missionId) };
        CallJsFunction(obj, methodName.c_str(), argv, ARGC_ONE);
    }
    {
        std::lock_guard<std::mutex> lock(jsListenerObjectMapLock_);
        tmpMap = jsListenerObjectMapSync_;
    }
    for (auto &item : tmpMap) {
        napi_value obj = (item.second)->GetNapiValue();
        napi_value argv[] = { CreateJsValue(env_, missionId) };
        CallJsFunction(obj, methodName.c_str(), argv, ARGC_ONE);
    }
}

void JsMissionListener::CallJsFunction(
    napi_value obj, const char* methodName, napi_value *argv, size_t argc)
{
    TAG_LOGI(AAFwkTag::MISSION, "method:%{public}s", methodName);
    HandleScope handleScope(env_);
    if (obj == nullptr) {
        TAG_LOGE(AAFwkTag::MISSION, "get obj failed");
        return;
    }

    napi_value method = nullptr;
    napi_get_named_property(env_, obj, methodName, &method);
    if (method == nullptr || AppExecFwk::IsTypeForNapiValue(env_, method, napi_undefined)
        || AppExecFwk::IsTypeForNapiValue(env_, method, napi_null)) {
        TAG_LOGE(AAFwkTag::MISSION, "Failed to get %{public}s", methodName);
        return;
    }
    napi_value callResult = nullptr;
    napi_call_function(env_, obj, method, argc, argv, &callResult);
}

#ifdef SUPPORT_SCREEN
void JsMissionListener::OnMissionIconUpdated(int32_t missionId, const std::shared_ptr<Media::PixelMap> &icon)
{
    TAG_LOGD(AAFwkTag::MISSION, "missionId: %{public}d", missionId);
    if (env_ == nullptr) {
        TAG_LOGE(AAFwkTag::MISSION, "null env_");
        return;
    }

    if (missionId <= 0 || !icon) {
        TAG_LOGE(AAFwkTag::MISSION, "invalid missionId or icon, missionId:%{public}d", missionId);
        return;
    }

    // js callback must run in js thread
    wptr<JsMissionListener> jsMissionListener = this;
    std::unique_ptr<NapiAsyncTask::CompleteCallback> complete = std::make_unique<NapiAsyncTask::CompleteCallback>
        ([jsMissionListener, missionId, icon](napi_env env, NapiAsyncTask &task, int32_t status) {
            sptr<JsMissionListener> jsMissionListenerSptr = jsMissionListener.promote();
            if (jsMissionListener != nullptr) {
                jsMissionListener->CallJsMissionIconUpdated(missionId, icon);
            }
        });
    napi_ref callback = nullptr;
    std::unique_ptr<NapiAsyncTask::ExecuteCallback> execute = nullptr;
    NapiAsyncTask::Schedule("JsMissionListener::OnMissionIconUpdated", env_,
        std::make_unique<NapiAsyncTask>(callback, std::move(execute), std::move(complete)));
}

void JsMissionListener::CallJsMissionIconUpdated(int32_t missionId, const std::shared_ptr<Media::PixelMap> &icon)
{
    if (env_ == nullptr) {
        TAG_LOGE(AAFwkTag::MISSION, "null env_");
        return;
    }

    HandleScope handleScope(env_);
    napi_value nativeMissionId = CreateJsValue(env_, missionId);
    auto nativeIcon = Media::PixelMapNapi::CreatePixelMap(env_, icon);

    std::map<int32_t, std::shared_ptr<NativeReference>> tmpMap;
    {
        std::lock_guard<std::mutex> lock(jsListenerObjectMapLock_);
        tmpMap = jsListenerObjectMap_;
    }
    for (auto &item : tmpMap) {
        napi_value obj = (item.second)->GetNapiValue();
        if (obj == nullptr) {
            TAG_LOGE(AAFwkTag::MISSION, "get js obj failed");
            continue;
        }
        napi_value method = nullptr;
        napi_get_named_property(env_, obj, "onMissionIconUpdated", &method);
        if (method == nullptr || AppExecFwk::IsTypeForNapiValue(env_, method, napi_undefined)
            || AppExecFwk::IsTypeForNapiValue(env_, method, napi_null)) {
            TAG_LOGE(AAFwkTag::MISSION, "get onMissionIconUpdated method from obj failed");
            continue;
        }

        napi_value argv[] = { nativeMissionId, nativeIcon };
        napi_value callResult = nullptr;
        napi_call_function(env_, obj, method, ArraySize(argv), argv, &callResult);
    }
}
#endif
}  // namespace AbilityRuntime
}  // namespace OHOS
