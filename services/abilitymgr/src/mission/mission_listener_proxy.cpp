/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#include "mission_listener_proxy.h"

#include "hilog_tag_wrapper.h"
#ifdef SUPPORT_SCREEN
#include "pixel_map.h"
#endif //SUPPORT_SCREEN

namespace OHOS {
namespace AAFwk {
void MissionListenerProxy::OnMissionCreated(int32_t missionId)
{
    SendRequestCommon(missionId, IMissionListener::ON_MISSION_CREATED);
}

void MissionListenerProxy::OnMissionDestroyed(int32_t missionId)
{
    SendRequestCommon(missionId, IMissionListener::ON_MISSION_DESTROYED);
}

void MissionListenerProxy::OnMissionSnapshotChanged(int32_t missionId)
{
    SendRequestCommon(missionId, IMissionListener::ON_MISSION_SNAPSHOT_CHANGED);
}

void MissionListenerProxy::OnMissionMovedToFront(int32_t missionId)
{
    SendRequestCommon(missionId, IMissionListener::ON_MISSION_MOVED_TO_FRONT);
}

void MissionListenerProxy::OnMissionMovedToBackground(int32_t missionId)
{
    SendRequestCommon(missionId, IMissionListener::ON_MISSION_MOVED_TO_BACKGROUND);
}

void MissionListenerProxy::OnMissionFocused(int32_t missionId)
{
    SendRequestCommon(missionId, IMissionListener::ON_MISSION_FOCUSED);
}

void MissionListenerProxy::OnMissionUnfocused(int32_t missionId)
{
    SendRequestCommon(missionId, IMissionListener::ON_MISSION_UNFOCUSED);
}

#ifdef SUPPORT_SCREEN
void MissionListenerProxy::OnMissionIconUpdated(int32_t missionId, const std::shared_ptr<Media::PixelMap> &icon)
{
    if (!icon) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "invalid mission icon");
        return;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    TAG_LOGD(AAFwkTag::ABILITYMGR, "mission_listener_proxy, OnMissionIconUpdated,missionId:%{public}d", missionId);
    if (!data.WriteInterfaceToken(IMissionListener::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write interface token fail");
        return;
    }

    if (!data.WriteInt32(missionId)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write missionId fail");
        return;
    }

    if (!data.WriteParcelable(icon.get())) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write icon fail");
        return;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "remote null");
        return;
    }
    int error = remote->SendRequest(IMissionListener::ON_MISSION_ICON_UPDATED, data, reply, option);
    if (error != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "SendRequest icon updated fail, error: %{public}d", error);
        return;
    }
}
#endif

void MissionListenerProxy::OnMissionClosed(int32_t missionId)
{
    SendRequestCommon(missionId, IMissionListener::ON_MISSION_CLOSED);
}

void MissionListenerProxy::OnMissionLabelUpdated(int32_t missionId)
{
    SendRequestCommon(missionId, IMissionListener::ON_MISSION_LABEL_UPDATED);
}

void MissionListenerProxy::SendRequestCommon(int32_t missionId, IMissionListener::MissionListenerCmd cmd)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    TAG_LOGD(AAFwkTag::ABILITYMGR, "mission_listener_proxy, sendrequest, cmd:%{public}d, missionId:%{public}d", cmd,
        missionId);
    if (!data.WriteInterfaceToken(IMissionListener::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write interface token fail");
        return;
    }

    if (!data.WriteInt32(missionId)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write missionId error");
        return;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "remote null");
        return;
    }
    int error = remote->SendRequest(cmd, data, reply, option);
    if (error != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "OnMissionCreated fail, error: %{public}d", error);
        return;
    }
}
}  // namespace AAFwk
}  // namespace OHOS
