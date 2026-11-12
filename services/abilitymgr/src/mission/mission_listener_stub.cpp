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

#include "mission_listener_stub.h"

#include "hilog_tag_wrapper.h"
#ifdef SUPPORT_SCREEN
#include "pixel_map.h"
#endif //SUPPORT_SCREEN

namespace OHOS {
namespace AAFwk {
MissionListenerStub::MissionListenerStub() {}

int MissionListenerStub::OnMissionCreatedInner(MessageParcel &data, MessageParcel &reply)
{
    auto missionId = data.ReadInt32();
    OnMissionCreated(missionId);
    return NO_ERROR;
}

int MissionListenerStub::OnMissionDestroyedInner(MessageParcel &data, MessageParcel &reply)
{
    auto missionId = data.ReadInt32();
    OnMissionDestroyed(missionId);
    return NO_ERROR;
}

int MissionListenerStub::OnMissionSnapshotChangedInner(MessageParcel &data, MessageParcel &reply)
{
    auto missionId = data.ReadInt32();
    OnMissionSnapshotChanged(missionId);
    return NO_ERROR;
}

int MissionListenerStub::OnMissionMovedToFrontInner(MessageParcel &data, MessageParcel &reply)
{
    auto missionId = data.ReadInt32();
    OnMissionMovedToFront(missionId);
    return NO_ERROR;
}

int MissionListenerStub::OnMissionMovedToBackgroundInner(MessageParcel &data, MessageParcel &reply)
{
    auto missionId = data.ReadInt32();
    OnMissionMovedToBackground(missionId);
    return NO_ERROR;
}

int MissionListenerStub::OnMissionFocusedInner(MessageParcel &data, MessageParcel &reply)
{
    auto missionId = data.ReadInt32();
    OnMissionFocused(missionId);
    return NO_ERROR;
}

int MissionListenerStub::OnMissionUnfocusedInner(MessageParcel &data, MessageParcel &reply)
{
    auto missionId = data.ReadInt32();
    OnMissionUnfocused(missionId);
    return NO_ERROR;
}

int MissionListenerStub::OnMissionIconUpdatedInner(MessageParcel &data, MessageParcel &reply)
{
#ifdef SUPPORT_SCREEN
    auto missionId = data.ReadInt32();
    std::shared_ptr<Media::PixelMap> icon(data.ReadParcelable<Media::PixelMap>());
    OnMissionIconUpdated(missionId, icon);
    return NO_ERROR;
#else
    TAG_LOGE(AAFwkTag::ABILITYMGR, "not support OnMissionIconUpdated");
    return ERR_INVALID_STATE;
#endif
}

int MissionListenerStub::OnMissionClosedInner(MessageParcel &data, MessageParcel &reply)
{
    OnMissionClosed(data.ReadInt32());
    return NO_ERROR;
}

int MissionListenerStub::OnMissionLabelUpdatedInner(MessageParcel &data, MessageParcel &reply)
{
    OnMissionLabelUpdated(data.ReadInt32());
    return NO_ERROR;
}

int MissionListenerStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    std::u16string descriptor = MissionListenerStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (descriptor != remoteDescriptor) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "local descriptor not equal to remote");
        return ERR_INVALID_STATE;
    }
    if (code < IMissionListener::MISSION_LINSTENER_CMD_MAX && code >= 0) {
        switch (code) {
            case ON_MISSION_CREATED:
                return OnMissionCreatedInner(data, reply);
                break;
            case ON_MISSION_DESTROYED:
                return OnMissionDestroyedInner(data, reply);
                break;
            case ON_MISSION_SNAPSHOT_CHANGED:
                return OnMissionSnapshotChangedInner(data, reply);
                break;
            case ON_MISSION_MOVED_TO_FRONT:
                return OnMissionMovedToFrontInner(data, reply);
                break;
            case ON_MISSION_ICON_UPDATED:
                return OnMissionIconUpdatedInner(data, reply);
                break;
            case ON_MISSION_CLOSED:
                return OnMissionClosedInner(data, reply);
                break;
            case ON_MISSION_LABEL_UPDATED:
                return OnMissionLabelUpdatedInner(data, reply);
                break;
            case ON_MISSION_FOCUSED:
                return OnMissionFocusedInner(data, reply);
                break;
            case ON_MISSION_UNFOCUSED:
                return OnMissionUnfocusedInner(data, reply);
                break;
            case ON_MISSION_MOVED_TO_BACKGROUND:
                return OnMissionMovedToBackgroundInner(data, reply);
        }
    }
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}
}  // namespace AAFwk
}  // namespace OHOS
