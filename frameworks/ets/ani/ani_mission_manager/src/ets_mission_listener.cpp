/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "ets_mission_listener.h"

#include <memory>

#include "ets_runtime.h"
#include "hilog_tag_wrapper.h"

#ifdef SUPPORT_SCREEN
#include "pixel_map_taihe_ani.h"
#endif

namespace OHOS {
namespace AbilityRuntime {

EtsMissionListener::~EtsMissionListener()
{
    ani_env *env = AppExecFwk::AttachAniEnv(vm_, isAttachThread_);
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::MISSION, "GetEnv failed");
        return;
    }
    std::lock_guard<std::mutex> lock(listenerLock_);
    ani_status status = ANI_ERROR;
    for (auto it = etsListenerObjectMap_.begin(); it != etsListenerObjectMap_.end();) {
        if ((status = env->GlobalReference_Delete(it->second)) != ANI_OK) {
            TAG_LOGE(AAFwkTag::MISSION, "GlobalReference_Delete status: %{public}d", status);
        }
        it++;
    }
    for (auto it = etsListenerObjectMapSync_.begin(); it != etsListenerObjectMapSync_.end();) {
        if ((status = env->GlobalReference_Delete(it->second)) != ANI_OK) {
            TAG_LOGE(AAFwkTag::MISSION, "GlobalReference_Delete status: %{public}d", status);
        }
        it++;
    }
    AppExecFwk::DetachAniEnv(vm_, isAttachThread_);
}

void EtsMissionListener::OnMissionCreated(int32_t missionId)
{
    CallEtsMethod("onMissionCreated", missionId);
}

void EtsMissionListener::OnMissionDestroyed(int32_t missionId)
{
    CallEtsMethod("onMissionDestroyed", missionId);
}

void EtsMissionListener::OnMissionSnapshotChanged(int32_t missionId)
{
    CallEtsMethod("onMissionSnapshotChanged", missionId);
}

void EtsMissionListener::OnMissionMovedToFront(int32_t missionId)
{
    CallEtsMethod("onMissionMovedToFront", missionId);
}

void EtsMissionListener::OnMissionClosed(int32_t missionId)
{
    CallEtsMethod("onMissionClosed", missionId);
}

void EtsMissionListener::OnMissionLabelUpdated(int32_t missionId)
{
    CallEtsMethod("onMissionLabelUpdated", missionId);
}

void EtsMissionListener::AddEtsListenerObject(
    ani_env *env, int32_t listenerId, ani_object etsListenerObject, bool isSync)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::MISSION, "null env");
        return;
    }
    std::lock_guard<std::mutex> lock(listenerLock_);
    if (isSync) {
        ani_ref objRef = nullptr;
        ani_status status = env->GlobalReference_Create(etsListenerObject, &objRef);
        if (status != ANI_OK || objRef == nullptr) {
            TAG_LOGE(AAFwkTag::MISSION, "GlobalReference_Create failed or null objRef, status: %{public}d", status);
            return;
        }
        etsListenerObjectMapSync_.emplace(listenerId, reinterpret_cast<ani_object>(objRef));
        return;
    }
    ani_ref objRef = nullptr;
    ani_status status = env->GlobalReference_Create(etsListenerObject, &objRef);
    if (status != ANI_OK || objRef == nullptr) {
        TAG_LOGE(AAFwkTag::MISSION, "GlobalReference_Create failed or null objRef, status: %{public}d", status);
        return;
    }
    etsListenerObjectMap_.emplace(listenerId, reinterpret_cast<ani_object>(objRef));
}

bool EtsMissionListener::RemoveEtsListenerObject(int32_t listenerId, bool isSync)
{
    std::lock_guard<std::mutex> lock(listenerLock_);
    if (isSync) {
        return (etsListenerObjectMapSync_.erase(listenerId) == 1);
    }
    return (etsListenerObjectMap_.erase(listenerId) == 1);
}

bool EtsMissionListener::IsEmpty()
{
    std::lock_guard<std::mutex> lock(listenerLock_);
    return etsListenerObjectMap_.empty() && etsListenerObjectMapSync_.empty();
}

void EtsMissionListener::CallEtsMethod(const std::string &methodName, int32_t missionId)
{
    TAG_LOGD(AAFwkTag::MISSION, "methodName:%{public}s", methodName.c_str());
    ani_env *env = AppExecFwk::AttachAniEnv(vm_, isAttachThread_);
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::MISSION, "null env");
        return;
    }
    std::map<int32_t, ani_object> tmpMap;
    std::map<int32_t, ani_object> tmpMapSync;
    {
        std::lock_guard<std::mutex> lock(listenerLock_);
        tmpMap = etsListenerObjectMap_;
        tmpMapSync = etsListenerObjectMapSync_;
    }
    for (const auto &item : tmpMap) {
        ani_object etsListenerObj = reinterpret_cast<ani_object>(item.second);
        ani_status status = ANI_ERROR;
        status = env->Object_CallMethodByName_Void(etsListenerObj, methodName.c_str(), "i:", missionId);
        if (status != ANI_OK) {
            TAG_LOGE(AAFwkTag::MISSION, "Object_CallMethodByName_Void failed, status: %{public}d", status);
        }
    }
    for (const auto &item : tmpMapSync) {
        ani_object etsListenerObj = reinterpret_cast<ani_object>(item.second);
        ani_status status = ANI_ERROR;
        status = env->Object_CallMethodByName_Void(etsListenerObj, methodName.c_str(), "i:", missionId);
        if (status != ANI_OK) {
            TAG_LOGE(AAFwkTag::MISSION, "Object_CallMethodByName_Void failed, status: %{public}d", status);
        }
    }
    AppExecFwk::DetachAniEnv(vm_, isAttachThread_);
}

#ifdef SUPPORT_SCREEN
void EtsMissionListener::OnMissionIconUpdated(int32_t missionId, const std::shared_ptr<Media::PixelMap> &icon)
{
    TAG_LOGD(AAFwkTag::MISSION, "missionId: %{public}d", missionId);
    ani_env *env = AppExecFwk::AttachAniEnv(vm_, isAttachThread_);
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::MISSION, "null env");
        return;
    }

    if (missionId <= 0 || !icon) {
        TAG_LOGE(AAFwkTag::MISSION, "invalid missionId or icon, missionId:%{public}d", missionId);
        AppExecFwk::DetachAniEnv(vm_, isAttachThread_);
        return;
    }

    auto iconObj = OHOS::Media::PixelMapTaiheAni::CreateEtsPixelMap(env, icon);
    std::map<int32_t, ani_object> tmpMap;
    {
        std::lock_guard<std::mutex> lock(listenerLock_);
        tmpMap = etsListenerObjectMap_;
    }
    for (const auto &item : tmpMap) {
        ani_object etsListenerObj = reinterpret_cast<ani_object>(item.second);
        ani_status status = ANI_ERROR;
        status = env->Object_CallMethodByName_Void(
            etsListenerObj, "onMissionIconUpdated", "iC{@ohos.multimedia.image.image.PixelMap}:", missionId, iconObj);
        if (status != ANI_OK) {
            TAG_LOGE(AAFwkTag::MISSION, "Object_CallMethodByName_Void failed, status: %{public}d", status);
        }
    }
    AppExecFwk::DetachAniEnv(vm_, isAttachThread_);
}
#endif
}  // namespace AbilityRuntime
}  // namespace OHOS
