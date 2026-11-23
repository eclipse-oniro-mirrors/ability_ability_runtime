/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

#ifdef SUPPORT_SCREEN
#include "window_manager_service_handler_stub.h"

#include "ability_manager_errors.h"
#include "hilog_tag_wrapper.h"
#include "pixel_map.h"

namespace OHOS {
namespace AAFwk {
WindowManagerServiceHandlerStub::WindowManagerServiceHandlerStub()
{
    Init();
}

WindowManagerServiceHandlerStub::~WindowManagerServiceHandlerStub() {}

void WindowManagerServiceHandlerStub::Init() {}

int WindowManagerServiceHandlerStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    if (data.ReadInterfaceToken() != IWindowManagerServiceHandler::GetDescriptor()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "invalid descriptor");
        return ERR_AAFWK_PARCEL_FAIL;
    }

    switch (code) {
        case ON_NOTIFY_WINDOW_TRANSITION:
            return NotifyWindowTransitionInner(data, reply);
        case ON_GET_FOCUS_ABILITY:
            return GetFocusWindowInner(data, reply);
        case ON_COLD_STARTING_WINDOW:
            return StartingWindowCold(data, reply);
        case ON_HOT_STARTING_WINDOW:
            return StartingWindowHot(data, reply);
        case ON_CANCEL_STARTING_WINDOW:
            return CancelStartingWindowInner(data, reply);
        case ON_NOTIFY_ANIMATION_ABILITY_DIED:
            return NotifyAnimationAbilityDiedInner(data, reply);
        case ON_MOVE_MISSINONS_TO_FOREGROUND:
            return MoveMissionsToForegroundInner(data, reply);
        case ON_MOVE_MISSIONS_TO_BACKGROUND:
            return MoveMissionsToBackgroundInner(data, reply);
    }

    TAG_LOGW(AAFwkTag::ABILITYMGR, "default case to be checked");
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

int WindowManagerServiceHandlerStub::NotifyWindowTransitionInner(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "called");
    sptr<AbilityTransitionInfo> fromInfo(data.ReadParcelable<AbilityTransitionInfo>());
    if (!fromInfo) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "read fromInfo failed");
        return ERR_AAFWK_PARCEL_FAIL;
    }
    sptr<AbilityTransitionInfo> toInfo(data.ReadParcelable<AbilityTransitionInfo>());
    if (!toInfo) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "read toInfo failed");
        return ERR_AAFWK_PARCEL_FAIL;
    }
    bool animaEnabled = data.ReadBool();
    NotifyWindowTransition(fromInfo, toInfo, animaEnabled);
    reply.WriteBool(animaEnabled);
    return ERR_OK;
}

int WindowManagerServiceHandlerStub::GetFocusWindowInner(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "called");
    sptr<IRemoteObject> abilityToken = nullptr;
    int32_t ret = GetFocusWindow(abilityToken);
    if (!reply.WriteInt32(ret)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write result failed");
        return ERR_AAFWK_PARCEL_FAIL;
    }
    if (abilityToken) {
        if (!reply.WriteBool(true)) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "write true failed");
            return ERR_AAFWK_PARCEL_FAIL;
        }
        if (!reply.WriteRemoteObject(abilityToken)) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "write abilityToken failed");
            return ERR_AAFWK_PARCEL_FAIL;
        }
    } else {
        if (!reply.WriteBool(false)) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "write false failed");
            return ERR_AAFWK_PARCEL_FAIL;
        }
    }
    return ERR_OK;
}

int WindowManagerServiceHandlerStub::StartingWindowCold(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "called");
    sptr<AbilityTransitionInfo> info(data.ReadParcelable<AbilityTransitionInfo>());
    if (!info) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "read info failed");
        return ERR_AAFWK_PARCEL_FAIL;
    }
    std::shared_ptr<Media::PixelMap> pixelMap
        = std::shared_ptr<Media::PixelMap>(data.ReadParcelable<Media::PixelMap>());
    if (pixelMap == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "read pixelMap failed");
        return ERR_AAFWK_PARCEL_FAIL;
    }
    auto bgColor = data.ReadUint32();
    StartingWindow(info, pixelMap, bgColor);
    return ERR_OK;
}

int WindowManagerServiceHandlerStub::StartingWindowHot(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "called");
    sptr<AbilityTransitionInfo> info(data.ReadParcelable<AbilityTransitionInfo>());
    if (!info) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "read info failed");
        return ERR_AAFWK_PARCEL_FAIL;
    }
    std::shared_ptr<Media::PixelMap> pixelMap
        = std::shared_ptr<Media::PixelMap>(data.ReadParcelable<Media::PixelMap>());
    if (pixelMap == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Failed to read pixelMap");
        return ERR_AAFWK_PARCEL_FAIL;
    }
    StartingWindow(info, pixelMap);
    return ERR_OK;
}

int WindowManagerServiceHandlerStub::CancelStartingWindowInner(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "called");
    sptr<IRemoteObject> abilityToken = nullptr;
    if (data.ReadBool()) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "valid abilityToken");
        abilityToken = data.ReadRemoteObject();
    }
    CancelStartingWindow(abilityToken);
    return ERR_OK;
}

int WindowManagerServiceHandlerStub::NotifyAnimationAbilityDiedInner(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "called");
    sptr<AbilityTransitionInfo> info(data.ReadParcelable<AbilityTransitionInfo>());
    if (!info) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "read info failed");
        return ERR_AAFWK_PARCEL_FAIL;
    }
    NotifyAnimationAbilityDied(info);
    return ERR_OK;
}

int WindowManagerServiceHandlerStub::MoveMissionsToForegroundInner(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "called");
    std::vector<int32_t> missionIds;
    data.ReadInt32Vector(&missionIds);
    int32_t topMissionId = data.ReadInt32();
    auto errCode = MoveMissionsToForeground(missionIds, topMissionId);
    reply.WriteInt32(errCode);
    return errCode;
}

int WindowManagerServiceHandlerStub::MoveMissionsToBackgroundInner(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "called");
    std::vector<int32_t> missionIds;
    std::vector<int32_t> result;
    data.ReadInt32Vector(&missionIds);
    auto errCode = MoveMissionsToBackground(missionIds, result);
    reply.WriteInt32Vector(result);
    return errCode;
}

}  // namespace AAFwk
}  // namespace OHOS
#endif
