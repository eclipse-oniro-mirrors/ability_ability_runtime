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

#include "ets_focused_application_observer.h"

#include "ani_task.h"
#include "ets_app_manager_utils.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {

ETSFocusedApplicationObserver::ETSFocusedApplicationObserver(ani_vm *etsVm) : etsVm_(etsVm) {}

ETSFocusedApplicationObserver::~ETSFocusedApplicationObserver()
{
    std::lock_guard<std::mutex> lock(etsObserverMapLock_);
    for (auto &entry : etsObserverMap_) {
        if (entry.second != nullptr && etsVm_ != nullptr) {
            AppManagerEts::ReleaseObjectReference(etsVm_, entry.second);
        }
    }
    etsObserverMap_.clear();
}

void ETSFocusedApplicationObserver::OnFocusedApplicationChanged(
    const FocusedApplicationInfo &focusedApplicationInfo)
{
    TAG_LOGD(AAFwkTag::APPMGR, "OnFocusedApplicationChanged called");
    if (!valid_) {
        TAG_LOGE(AAFwkTag::APPMGR, "invalid observer");
        return;
    }
    wptr<ETSFocusedApplicationObserver> weakPtr = this;
    auto task = [focusedApplicationInfo, weakPtr]() {
        auto observer = weakPtr.promote();
        if (observer == nullptr) {
            TAG_LOGE(AAFwkTag::APPMGR, "null observerPtr");
            return;
        }
        observer->HandleOnFocusedApplicationChanged(focusedApplicationInfo);
    };
    if (AniTask::AniSendEvent(task) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "Failed to aniSendEvent");
    }
    TAG_LOGD(AAFwkTag::APPMGR, "OnFocusedApplicationChanged end");
}

void ETSFocusedApplicationObserver::HandleOnFocusedApplicationChanged(
    const FocusedApplicationInfo &focusedApplicationInfo)
{
    TAG_LOGD(AAFwkTag::APPMGR, "HandleOnFocusedApplicationChanged called");
    if (etsVm_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "etsVm_ nullptr");
        return;
    }
    bool isAttachThread = false;
    ani_env *env = AppExecFwk::AttachAniEnv(etsVm_, isAttachThread);
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "GetEnv failed");
        return;
    }
    ani_object infoObj = AppManagerEts::WrapFocusedApplicationInfo(env, focusedApplicationInfo);
    if (infoObj == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "WrapFocusedApplicationInfo failed");
        AppExecFwk::DetachAniEnv(etsVm_, isAttachThread);
        return;
    }
    ani_value argv[] = {{.r = infoObj}};
    std::vector<ani_ref> observers;
    {
        std::lock_guard<std::mutex> lock(etsObserverMapLock_);
        for (const auto &entry : etsObserverMap_) {
            if (entry.second != nullptr) {
                observers.push_back(entry.second);
            }
        }
    }
    for (const auto &observerRef : observers) {
        ani_object observerObj = reinterpret_cast<ani_object>(observerRef);
        env->ResetError();
        ani_status status = env->Object_CallMethodByName_Void_A(
            observerObj, "onFocusedApplicationChanged",
            "C{application.FocusedApplicationInfo.FocusedApplicationInfo}:", argv);
        if (status != ANI_OK) {
            TAG_LOGE(AAFwkTag::APPMGR, "Call onFocusedApplicationChanged failed: %{public}d", status);
        }
    }
    AppExecFwk::DetachAniEnv(etsVm_, isAttachThread);
    TAG_LOGD(AAFwkTag::APPMGR, "HandleOnFocusedApplicationChanged end");
}

void ETSFocusedApplicationObserver::AddEtsObserverObject(
    ani_env *env, int32_t observerId, const ani_object &observerObj)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null env");
        return;
    }
    ani_ref observerRef = nullptr;
    ani_status status = env->GlobalReference_Create(observerObj, &observerRef);
    if (status != ANI_OK || observerRef == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "GlobalReference_Create failed: %{public}d", status);
        return;
    }
    std::lock_guard<std::mutex> lock(etsObserverMapLock_);
    etsObserverMap_.emplace(observerId, observerRef);
}

bool ETSFocusedApplicationObserver::RemoveEtsObserverObjectById(int32_t observerId)
{
    std::lock_guard<std::mutex> lock(etsObserverMapLock_);
    auto it = etsObserverMap_.find(observerId);
    if (it != etsObserverMap_.end()) {
        if (it->second != nullptr && etsVm_ != nullptr) {
            AppManagerEts::ReleaseObjectReference(etsVm_, it->second);
        }
        etsObserverMap_.erase(it);
        return true;
    }
    return false;
}

bool ETSFocusedApplicationObserver::IsEmpty()
{
    std::lock_guard<std::mutex> lock(etsObserverMapLock_);
    return etsObserverMap_.empty();
}

void ETSFocusedApplicationObserver::SetValid(bool valid)
{
    valid_ = valid;
}
} // namespace AbilityRuntime
} // namespace OHOS
