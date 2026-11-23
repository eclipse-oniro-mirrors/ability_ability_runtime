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
#include "window_manager_service_handler_proxy.h"

#include "ability_manager_errors.h"
#include "hilog_tag_wrapper.h"
#include "parcel.h"
#include "pixel_map.h"

namespace OHOS {
namespace AAFwk {
WindowManagerServiceHandlerProxy::WindowManagerServiceHandlerProxy(const sptr<IRemoteObject> &impl)
    : IRemoteProxy<IWindowManagerServiceHandler>(impl) {}

void WindowManagerServiceHandlerProxy::NotifyWindowTransition(sptr<AbilityTransitionInfo> fromInfo,
    sptr<AbilityTransitionInfo> toInfo, bool& animaEnabled)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "called");
    MessageParcel data;
    if (!data.WriteInterfaceToken(IWindowManagerServiceHandler::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Write token failed");
        return;
    }
    if (!data.WriteParcelable(fromInfo.GetRefPtr())) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Write fromInfo failed");
        return;
    }
    if (!data.WriteParcelable(toInfo.GetRefPtr())) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Write toInfo failed");
        return;
    }
    if (!data.WriteBool(animaEnabled)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Write animaEnabled failed");
        return;
    }
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    int error = SendTransactCmd(WMSCmd::ON_NOTIFY_WINDOW_TRANSITION, data, reply, option);
    if (error != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "SendRequest error: %{public}d", error);
    }
    animaEnabled = reply.ReadBool();
}

int32_t WindowManagerServiceHandlerProxy::GetFocusWindow(sptr<IRemoteObject>& abilityToken)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "called");
    MessageParcel data;
    if (!data.WriteInterfaceToken(IWindowManagerServiceHandler::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Write token failed");
        return ERR_AAFWK_PARCEL_FAIL;
    }

    MessageParcel reply;
    MessageOption option;
    int error = SendTransactCmd(WMSCmd::ON_GET_FOCUS_ABILITY, data, reply, option);
    if (error != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "SendRequest error: %{public}d", error);
        return ERR_AAFWK_PARCEL_FAIL;
    }
    auto ret = reply.ReadInt32();
    if (ret == 0 && reply.ReadBool()) {
        abilityToken = reply.ReadRemoteObject();
    }
    TAG_LOGD(AAFwkTag::ABILITYMGR, "ending");
    return ret;
}

void WindowManagerServiceHandlerProxy::StartingWindow(sptr<AbilityTransitionInfo> info,
    std::shared_ptr<Media::PixelMap> pixelMap, uint32_t bgColor)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "called");
    MessageParcel data;
    if (!data.WriteInterfaceToken(IWindowManagerServiceHandler::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Failed to write token");
        return;
    }
    if (!data.WriteParcelable(info.GetRefPtr())) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Write info failed");
        return;
    }
    if (!data.WriteParcelable(pixelMap.get())) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Write pixelMap failed");
        return;
    }
    if (!data.WriteUint32(bgColor)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Failed to write bgColor");
        return;
    }
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    int error = SendTransactCmd(WMSCmd::ON_COLD_STARTING_WINDOW, data, reply, option);
    if (error != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "SendRequest error: %{public}d", error);
    }
}

void WindowManagerServiceHandlerProxy::StartingWindow(sptr<AbilityTransitionInfo> info,
    std::shared_ptr<Media::PixelMap> pixelMap)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "called");
    MessageParcel data;
    if (!data.WriteInterfaceToken(IWindowManagerServiceHandler::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Write token failed");
        return;
    }
    if (!data.WriteParcelable(info.GetRefPtr())) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Write info failed");
        return;
    }
    if (!data.WriteParcelable(pixelMap.get())) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Failed to write pixelMap");
        return;
    }
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    int error = SendTransactCmd(WMSCmd::ON_HOT_STARTING_WINDOW, data, reply, option);
    if (error != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "SendRequest error: %{public}d", error);
    }
}

void WindowManagerServiceHandlerProxy::CancelStartingWindow(sptr<IRemoteObject> abilityToken)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "called");
    MessageParcel data;
    if (!data.WriteInterfaceToken(IWindowManagerServiceHandler::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Write token failed");
        return;
    }
    if (!abilityToken) {
        if (!data.WriteBool(false)) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "Failed to write false");
            return;
        }
    } else {
        if (!data.WriteBool(true)) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "Write true failed");
            return;
        }
        if (!data.WriteRemoteObject(abilityToken)) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "Write abilityToken failed");
            return;
        }
    }
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    int error = SendTransactCmd(WMSCmd::ON_CANCEL_STARTING_WINDOW, data, reply, option);
    if (error != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "SendRequest error: %{public}d", error);
    }
}

void WindowManagerServiceHandlerProxy::NotifyAnimationAbilityDied(sptr<AbilityTransitionInfo> info)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "called");
    MessageParcel data;
    if (!data.WriteInterfaceToken(IWindowManagerServiceHandler::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Write token failed");
        return;
    }
    if (!data.WriteParcelable(info.GetRefPtr())) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Failed to write info");
        return;
    }
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    int error = SendTransactCmd(WMSCmd::ON_NOTIFY_ANIMATION_ABILITY_DIED, data, reply, option);
    if (error != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "SendRequest error: %{public}d", error);
    }
}

int32_t WindowManagerServiceHandlerProxy::MoveMissionsToForeground(const std::vector<int32_t>& missionIds,
    int32_t topMissionId)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "called");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(IWindowManagerServiceHandler::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "WriteInterfaceToken failed");
        return ERR_AAFWK_PARCEL_FAIL;
    }

    if (!data.WriteInt32Vector(missionIds)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Write missionIds failed");
        return ERR_AAFWK_PARCEL_FAIL;
    }

    if (!data.WriteInt32(topMissionId)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Failed to write TopMissionId");
        return ERR_AAFWK_PARCEL_FAIL;
    }

    int error = SendTransactCmd(WMSCmd::ON_MOVE_MISSINONS_TO_FOREGROUND, data, reply, option);
    if (error != ERR_NONE) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "SendoRequest error: %{public}d", error);
        return ERR_AAFWK_PARCEL_FAIL;
    }
    return reply.ReadInt32();
}

int32_t WindowManagerServiceHandlerProxy::MoveMissionsToBackground(const std::vector<int32_t>& missionIds,
    std::vector<int32_t>& result)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "called");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(IWindowManagerServiceHandler::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "WriteInterfaceToken failed");
        return ERR_AAFWK_PARCEL_FAIL;
    }

    if (!data.WriteInt32Vector(missionIds)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Write missionIds failed");
        return ERR_AAFWK_PARCEL_FAIL;
    }

    int error = SendTransactCmd(WMSCmd::ON_MOVE_MISSIONS_TO_BACKGROUND, data, reply, option);
    if (error != ERR_NONE) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "SendoRequest error: %{public}d", error);
        return ERR_AAFWK_PARCEL_FAIL;
    }
    if (!reply.ReadInt32Vector(&result)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Read hide result failed");
        return ERR_AAFWK_PARCEL_FAIL;
    };
    return reply.ReadInt32();
}

int32_t WindowManagerServiceHandlerProxy::SendTransactCmd(uint32_t code, MessageParcel &data,
    MessageParcel &reply, MessageOption &option)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null remote");
        return ERR_NULL_OBJECT;
    }

    int32_t ret = remote->SendRequest(code, data, reply, option);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "SendRequest failed. code: %{public}d, ret: %{public}d", code, ret);
        return ret;
    }
    return ERR_OK;
}
}  // namespace AAFwk
}  // namespace OHOS
#endif
