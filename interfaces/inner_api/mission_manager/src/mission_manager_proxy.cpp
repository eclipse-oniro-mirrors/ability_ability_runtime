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

#include "ability_manager_errors.h"
#include "ability_manager_ipc_interface_code.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "message_parcel.h"
#include "mission_manager_proxy.h"
#include "want.h"

namespace OHOS::AAFwk {
bool MissionManagerProxy::WriteInterfaceToken(MessageParcel &data)
{
    if (!data.WriteInterfaceToken(MissionManagerProxy::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::MISSION, "write token failed");
        return false;
    }
    return true;
}

ErrCode MissionManagerProxy::SendRequest(AbilityManagerInterfaceCode code, MessageParcel &data,
    MessageParcel &reply, MessageOption &option)
{
    auto remote = Remote();
    if (remote == nullptr) {
        TAG_LOGE(AAFwkTag::MISSION, "null remote");
        return INVALID_REMOTE_PARAMETERS_ERR;
    }

    return remote->SendRequest(static_cast<uint32_t>(code), data, reply, option);
}

int MissionManagerProxy::ContinueMission(const std::string &srcDeviceId, const std::string &dstDeviceId,
    int32_t missionId, const sptr<IRemoteObject> &callBack, AAFwk::WantParams &wantParams)
{
    TAG_LOGI(AAFwkTag::MISSION, "called");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return INVALID_PARAMETERS_ERR;
    }
    if (!data.WriteString(srcDeviceId)) {
        TAG_LOGE(AAFwkTag::MISSION, "srcDeviceId write fail");
        return INVALID_PARAMETERS_ERR;
    }
    if (!data.WriteString(dstDeviceId)) {
        TAG_LOGE(AAFwkTag::MISSION, "dstDeviceId write fail");
        return INVALID_PARAMETERS_ERR;
    }
    if (!data.WriteInt32(missionId)) {
        TAG_LOGE(AAFwkTag::MISSION, "missionId write fail");
        return INVALID_PARAMETERS_ERR;
    }
    if (!data.WriteRemoteObject(callBack)) {
        TAG_LOGE(AAFwkTag::MISSION, "callBack write fail");
        return INVALID_PARAMETERS_ERR;
    }
    if (!data.WriteParcelable(&wantParams)) {
        TAG_LOGE(AAFwkTag::MISSION, "wantParams write fail");
        return INVALID_PARAMETERS_ERR;
    }

    auto error = SendRequest(AbilityManagerInterfaceCode::CONTINUE_MISSION, data, reply, option);
    if (error != NO_ERROR) {
        TAG_LOGE(AAFwkTag::MISSION, "sendRequest error: %{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}

int MissionManagerProxy::ContinueMission(AAFwk::ContinueMissionInfo continueMissionInfo,
    const sptr<IRemoteObject> &callback)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return INVALID_PARAMETERS_ERR;
    }
    if (!data.WriteString(continueMissionInfo.srcDeviceId)) {
        TAG_LOGE(AAFwkTag::MISSION, "srcDeviceId write fail");
        return INVALID_PARAMETERS_ERR;
    }
    if (!data.WriteString(continueMissionInfo.dstDeviceId)) {
        TAG_LOGE(AAFwkTag::MISSION, "dstDeviceId write fail");
        return INVALID_PARAMETERS_ERR;
    }
    if (!data.WriteString(continueMissionInfo.bundleName)) {
        TAG_LOGE(AAFwkTag::MISSION, "missionId write fail");
        return INVALID_PARAMETERS_ERR;
    }
    if (!data.WriteRemoteObject(callback)) {
        TAG_LOGE(AAFwkTag::MISSION, "callBack write fail");
        return INVALID_PARAMETERS_ERR;
    }
    if (!data.WriteParcelable(&continueMissionInfo.wantParams)) {
        TAG_LOGE(AAFwkTag::MISSION, "wantParams write fail");
        return INVALID_PARAMETERS_ERR;
    }
    if (!data.WriteString(continueMissionInfo.srcBundleName)) {
        TAG_LOGE(AAFwkTag::MISSION, "srcBundleName write fail");
        return INVALID_PARAMETERS_ERR;
    }
    if (!data.WriteString(continueMissionInfo.continueType)) {
        TAG_LOGE(AAFwkTag::MISSION, "continueType write fail");
        return INVALID_PARAMETERS_ERR;
    }
    auto error = SendRequest(AbilityManagerInterfaceCode::CONTINUE_MISSION_OF_BUNDLENAME, data, reply, option);
    if (error != NO_ERROR) {
        TAG_LOGE(AAFwkTag::MISSION, "request error:%{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}

int MissionManagerProxy::LockMissionForCleanup(int32_t missionId)
{
    int error;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return INVALID_PARAMETERS_ERR;
    }
    if (!data.WriteInt32(missionId)) {
        TAG_LOGE(AAFwkTag::MISSION, "missionId write fail");
        return INVALID_PARAMETERS_ERR;
    }

    error = SendRequest(AbilityManagerInterfaceCode::LOCK_MISSION_FOR_CLEANUP, data, reply, option);
    if (error != NO_ERROR) {
        TAG_LOGE(AAFwkTag::MISSION, "send error:%{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}

int MissionManagerProxy::UnlockMissionForCleanup(int32_t missionId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    int error;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return INVALID_PARAMETERS_ERR;
    }
    if (!data.WriteInt32(missionId)) {
        TAG_LOGE(AAFwkTag::MISSION, "writeInt32 fail");
        return INVALID_PARAMETERS_ERR;
    }
    error = SendRequest(AbilityManagerInterfaceCode::UNLOCK_MISSION_FOR_CLEANUP, data, reply, option);
    if (error != NO_ERROR) {
        TAG_LOGE(AAFwkTag::MISSION, "unlock mission,error:%{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}

int MissionManagerProxy::RegisterMissionListener(const sptr<IMissionListener> &listener)
{
    int error;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!listener) {
        TAG_LOGE(AAFwkTag::MISSION, "listener null");
        return INVALID_PARAMETERS_ERR;
    }

    if (!WriteInterfaceToken(data)) {
        return INVALID_PARAMETERS_ERR;
    }
    if (!data.WriteRemoteObject(listener->AsObject())) {
        TAG_LOGE(AAFwkTag::MISSION, "write missionListener fail");
        return INVALID_PARAMETERS_ERR;
    }

    error = SendRequest(AbilityManagerInterfaceCode::REGISTER_MISSION_LISTENER, data, reply, option);
    if (error != NO_ERROR) {
        TAG_LOGE(AAFwkTag::MISSION, "request error: %{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}

int MissionManagerProxy::UnRegisterMissionListener(const sptr<IMissionListener> &listener)
{
    int error;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!listener) {
        TAG_LOGE(AAFwkTag::MISSION, "listener null");
        return INVALID_PARAMETERS_ERR;
    }

    if (!WriteInterfaceToken(data)) {
        return INVALID_PARAMETERS_ERR;
    }
    if (!data.WriteRemoteObject(listener->AsObject())) {
        TAG_LOGE(AAFwkTag::MISSION, "write missionListener fail");
        return INVALID_PARAMETERS_ERR;
    }

    error = SendRequest(AbilityManagerInterfaceCode::UNREGISTER_MISSION_LISTENER, data, reply, option);
    if (error != NO_ERROR) {
        TAG_LOGE(AAFwkTag::MISSION, "request error:%{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}

int MissionManagerProxy::RegisterMissionListener(const std::string &deviceId,
    const sptr<IRemoteMissionListener> &listener)
{
    if (!listener) {
        TAG_LOGE(AAFwkTag::MISSION, "listener null");
        return INVALID_PARAMETERS_ERR;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return INVALID_PARAMETERS_ERR;
    }
    if (!data.WriteString(deviceId)) {
        TAG_LOGE(AAFwkTag::MISSION, "deviceId write fail");
        return INVALID_PARAMETERS_ERR;
    }
    if (!data.WriteRemoteObject(listener->AsObject())) {
        TAG_LOGE(AAFwkTag::MISSION, "listener write fail");
        return INVALID_PARAMETERS_ERR;
    }

    auto error = SendRequest(AbilityManagerInterfaceCode::REGISTER_REMOTE_MISSION_LISTENER,
        data, reply, option);
    if (error != NO_ERROR) {
        TAG_LOGE(AAFwkTag::MISSION, "request error:%{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}

int MissionManagerProxy::UnRegisterMissionListener(const std::string &deviceId,
    const sptr<IRemoteMissionListener> &listener)
{
    if (!listener) {
        TAG_LOGE(AAFwkTag::MISSION, "listener null");
        return INVALID_PARAMETERS_ERR;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return INVALID_PARAMETERS_ERR;
    }
    if (!data.WriteString(deviceId)) {
        TAG_LOGE(AAFwkTag::MISSION, "deviceId write fail");
        return INVALID_PARAMETERS_ERR;
    }
    if (!data.WriteRemoteObject(listener->AsObject())) {
        TAG_LOGE(AAFwkTag::MISSION, "listener write fail");
        return INVALID_PARAMETERS_ERR;
    }

    auto error = SendRequest(AbilityManagerInterfaceCode::UNREGISTER_REMOTE_MISSION_LISTENER,
        data, reply, option);
    if (error != NO_ERROR) {
        TAG_LOGE(AAFwkTag::MISSION, "request error:%{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}

template <typename T>
int MissionManagerProxy::GetParcelableInfos(MessageParcel &reply, std::vector<T> &parcelableInfos)
{
    int32_t infoSize = reply.ReadInt32();
    if (infoSize > CYCLE_LIMIT) {
        TAG_LOGE(AAFwkTag::MISSION, "infoSize large");
        return INVALID_PARAMETERS_ERR;
    }

    for (int32_t i = 0; i < infoSize; i++) {
        std::unique_ptr<T> info(reply.ReadParcelable<T>());
        if (!info) {
            TAG_LOGE(AAFwkTag::MISSION, "readParcelableInfos fail");
            return INVALID_PARAMETERS_ERR;
        }
        parcelableInfos.emplace_back(*info);
    }
    return NO_ERROR;
}

int MissionManagerProxy::GetMissionInfos(const std::string &deviceId, int32_t numMax,
    std::vector<MissionInfo> &missionInfos)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    int error;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return INVALID_PARAMETERS_ERR;
    }
    if (!data.WriteString16(Str8ToStr16(deviceId))) {
        TAG_LOGE(AAFwkTag::MISSION, "write deviceId fail");
        return INVALID_PARAMETERS_ERR;
    }
    if (!data.WriteInt32(numMax)) {
        TAG_LOGE(AAFwkTag::MISSION, "writeInt32 fail");
        return INVALID_PARAMETERS_ERR;
    }
    error = SendRequest(AbilityManagerInterfaceCode::GET_MISSION_INFOS, data, reply, option);
    if (error != NO_ERROR) {
        TAG_LOGE(AAFwkTag::MISSION, " request error:%{public}d", error);
        return error;
    }
    error = GetParcelableInfos<MissionInfo>(reply, missionInfos);
    if (error != NO_ERROR) {
        TAG_LOGE(AAFwkTag::MISSION, "getMissionInfos error: %{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}

int MissionManagerProxy::GetMissionInfo(const std::string &deviceId, int32_t missionId,
    MissionInfo &missionInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    int error;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return INVALID_PARAMETERS_ERR;
    }
    if (!data.WriteString16(Str8ToStr16(deviceId))) {
        TAG_LOGE(AAFwkTag::MISSION, "write deviceId failed");
        return INVALID_PARAMETERS_ERR;
    }
    if (!data.WriteInt32(missionId)) {
        TAG_LOGE(AAFwkTag::MISSION, "writeInt32 failed");
        return INVALID_PARAMETERS_ERR;
    }
    error = SendRequest(AbilityManagerInterfaceCode::GET_MISSION_INFO_BY_ID, data, reply, option);
    if (error != NO_ERROR) {
        TAG_LOGE(AAFwkTag::MISSION, "request error:%{public}d", error);
        return error;
    }

    std::unique_ptr<MissionInfo> info(reply.ReadParcelable<MissionInfo>());
    if (!info) {
        TAG_LOGE(AAFwkTag::MISSION, "read missioninfo fail");
        return ERR_UNKNOWN_OBJECT;
    }
    missionInfo = *info;
    return reply.ReadInt32();
}

int MissionManagerProxy::GetMissionSnapshot(const std::string &deviceId, int32_t missionId,
    MissionSnapshot &snapshot, bool isLowResolution)
{
    int error;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return INVALID_PARAMETERS_ERR;
    }
    if (!data.WriteString(deviceId)) {
        TAG_LOGE(AAFwkTag::MISSION, "deviceId write fail");
        return INVALID_PARAMETERS_ERR;
    }
    if (!data.WriteInt32(missionId)) {
        TAG_LOGE(AAFwkTag::MISSION, "missionId write fail");
        return INVALID_PARAMETERS_ERR;
    }
    if (!data.WriteBool(isLowResolution)) {
        TAG_LOGE(AAFwkTag::MISSION, "isLowResolution write fail");
        return INVALID_PARAMETERS_ERR;
    }
    error = SendRequest(AbilityManagerInterfaceCode::GET_MISSION_SNAPSHOT_INFO, data, reply, option);
    if (error != NO_ERROR) {
        TAG_LOGE(AAFwkTag::MISSION, "request error:%{public}d", error);
        return error;
    }
    std::unique_ptr<MissionSnapshot> info(reply.ReadParcelable<MissionSnapshot>());
    if (!info) {
        TAG_LOGE(AAFwkTag::MISSION, "readParcelableInfo fail");
        auto errorCode = reply.ReadInt32();
        return errorCode ? errorCode : ERR_UNKNOWN_OBJECT;
    }
    snapshot = *info;
    return reply.ReadInt32();
}

int MissionManagerProxy::CleanMission(int32_t missionId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    int error;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return INVALID_PARAMETERS_ERR;
    }
    if (!data.WriteInt32(missionId)) {
        TAG_LOGE(AAFwkTag::MISSION, "writeInt32 fail");
        return INVALID_PARAMETERS_ERR;
    }
    error = SendRequest(AbilityManagerInterfaceCode::CLEAN_MISSION, data, reply, option);
    if (error != NO_ERROR) {
        TAG_LOGE(AAFwkTag::MISSION, "clean mission, error: %{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}

int MissionManagerProxy::CleanAllMissions()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    int error;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return INVALID_PARAMETERS_ERR;
    }
    error = SendRequest(AbilityManagerInterfaceCode::CLEAN_ALL_MISSIONS, data, reply, option);
    if (error != NO_ERROR) {
        TAG_LOGE(AAFwkTag::MISSION, "request error:%{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}

int MissionManagerProxy::MoveMissionToFront(int32_t missionId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    int error;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return INVALID_PARAMETERS_ERR;
    }
    if (!data.WriteInt32(missionId)) {
        TAG_LOGE(AAFwkTag::MISSION, "writeInt32 fail");
        return INVALID_PARAMETERS_ERR;
    }
    error = SendRequest(AbilityManagerInterfaceCode::MOVE_MISSION_TO_FRONT, data, reply, option);
    if (error != NO_ERROR) {
        TAG_LOGE(AAFwkTag::MISSION, "request error:%{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}

int MissionManagerProxy::MoveMissionToFront(int32_t missionId, const StartOptions &startOptions)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    int error;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return INVALID_PARAMETERS_ERR;
    }
    if (!data.WriteInt32(missionId)) {
        TAG_LOGE(AAFwkTag::MISSION, "writeInt32 fail");
        return INVALID_PARAMETERS_ERR;
    }
    if (!data.WriteParcelable(&startOptions)) {
        TAG_LOGE(AAFwkTag::MISSION, "startOptions write fail");
        return INVALID_PARAMETERS_ERR;
    }
    error = SendRequest(AbilityManagerInterfaceCode::MOVE_MISSION_TO_FRONT_BY_OPTIONS, data, reply, option);
    if (error != NO_ERROR) {
        TAG_LOGE(AAFwkTag::MISSION, "request error:%{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}

int MissionManagerProxy::MoveMissionsToForeground(const std::vector<int32_t> &missionIds, int32_t topMissionId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return INVALID_PARAMETERS_ERR;
    }

    if (!data.WriteInt32Vector(missionIds)) {
        TAG_LOGE(AAFwkTag::MISSION, "missionIds write fail");
        return INVALID_PARAMETERS_ERR;
    }

    if (!data.WriteInt32(topMissionId)) {
        TAG_LOGE(AAFwkTag::MISSION, "topMissionId write fail");
        return INVALID_PARAMETERS_ERR;
    }

    auto error = SendRequest(AbilityManagerInterfaceCode::MOVE_MISSIONS_TO_FOREGROUND, data, reply, option);
    if (error != NO_ERROR) {
        TAG_LOGE(AAFwkTag::MISSION, "request error:%{public}d", error);
        return error;
    }

    return reply.ReadInt32();
}

int MissionManagerProxy::MoveMissionsToBackground(const std::vector<int32_t> &missionIds, std::vector<int32_t> &result)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return INVALID_PARAMETERS_ERR;
    }

    if (!data.WriteInt32Vector(missionIds)) {
        TAG_LOGE(AAFwkTag::MISSION, "mission id write fail");
        return INVALID_PARAMETERS_ERR;
    }

    auto error = SendRequest(AbilityManagerInterfaceCode::MOVE_MISSIONS_TO_BACKGROUND, data, reply, option);
    if (error != NO_ERROR) {
        TAG_LOGE(AAFwkTag::MISSION, "request error:%{public}d", error);
        return error;
    }

    if (!reply.ReadInt32Vector(&result)) {
        TAG_LOGE(AAFwkTag::MISSION, "read result fail");
        return INVALID_PARAMETERS_ERR;
    }
    return reply.ReadInt32();
}

int32_t MissionManagerProxy::GetMissionIdByToken(const sptr<IRemoteObject> &token)
{
    if (!token) {
        TAG_LOGE(AAFwkTag::MISSION, "token null");
        return ERR_INVALID_CALLER;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::MISSION, "write token fail");
        return INVALID_PARAMETERS_ERR;
    }

    if (!data.WriteRemoteObject(token)) {
        TAG_LOGE(AAFwkTag::MISSION, "data write fail");
        return INVALID_PARAMETERS_ERR;
    }

    auto error = SendRequest(AbilityManagerInterfaceCode::GET_MISSION_ID_BY_ABILITY_TOKEN,
        data, reply, option);
    if (error != NO_ERROR) {
        TAG_LOGE(AAFwkTag::MISSION, "request error:%{public}d", error);
        return INVALID_PARAMETERS_ERR;
    }

    return reply.ReadInt32();
}

int MissionManagerProxy::StartSyncRemoteMissions(const std::string &devId, bool fixConflict, int64_t tag)
{
    TAG_LOGI(AAFwkTag::MISSION, "called");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::MISSION, "writeInterfaceToken fail");
        return INVALID_PARAMETERS_ERR;
    }
    if (!data.WriteString(devId)) {
        TAG_LOGE(AAFwkTag::MISSION, "write deviceId fail");
        return INVALID_PARAMETERS_ERR;
    }

    if (!data.WriteBool(fixConflict)) {
        TAG_LOGE(AAFwkTag::MISSION, "writeBool fail");
        return INVALID_PARAMETERS_ERR;
    }

    if (!data.WriteInt64(tag)) {
        TAG_LOGE(AAFwkTag::MISSION, "writeInt64 fail");
        return INVALID_PARAMETERS_ERR;
    }

    auto error = SendRequest(AbilityManagerInterfaceCode::START_SYNC_MISSIONS, data, reply, option);
    if (error != NO_ERROR) {
        TAG_LOGE(AAFwkTag::MISSION, "request error:%{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}

int32_t MissionManagerProxy::StopSyncRemoteMissions(const std::string &devId)
{
    TAG_LOGI(AAFwkTag::MISSION, "call");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::MISSION, "writeInterfaceToken fail");
        return INVALID_PARAMETERS_ERR;
    }
    if (!data.WriteString(devId)) {
        TAG_LOGE(AAFwkTag::MISSION, "write deviceId fail");
        return INVALID_PARAMETERS_ERR;
    }
    auto error = SendRequest(AbilityManagerInterfaceCode::STOP_SYNC_MISSIONS, data, reply, option);
    if (error != NO_ERROR) {
        TAG_LOGE(AAFwkTag::MISSION, "request error:%{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}

int MissionManagerProxy::SetMissionContinueState(const sptr<IRemoteObject> &token, const AAFwk::ContinueState &state)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return INVALID_PARAMETERS_ERR;
    }
    if (!data.WriteRemoteObject(token)) {
        TAG_LOGE(AAFwkTag::MISSION, "write token fail");
        return INVALID_PARAMETERS_ERR;
    }
    if (!data.WriteInt32(static_cast<int32_t>(state))) {
        TAG_LOGE(AAFwkTag::MISSION, "write state fail");
        return INVALID_PARAMETERS_ERR;
    }
    auto error = SendRequest(AbilityManagerInterfaceCode::SET_MISSION_CONTINUE_STATE, data, reply, option);
    if (error != NO_ERROR) {
        TAG_LOGE(AAFwkTag::MISSION, "request error:%{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}

#ifdef SUPPORT_SCREEN
int MissionManagerProxy::SetMissionLabel(const sptr<IRemoteObject> &token, const std::string &label)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return INVALID_PARAMETERS_ERR;
    }
    if (!data.WriteRemoteObject(token)) {
        TAG_LOGE(AAFwkTag::MISSION, "write token fail");
        return INVALID_PARAMETERS_ERR;
    }
    if (!data.WriteString16(Str8ToStr16(label))) {
        TAG_LOGE(AAFwkTag::MISSION, "write label fail");
        return INVALID_PARAMETERS_ERR;
    }
    auto error = SendRequest(AbilityManagerInterfaceCode::SET_MISSION_LABEL, data, reply, option);
    if (error != NO_ERROR) {
        TAG_LOGE(AAFwkTag::MISSION, "request error:%{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}

int MissionManagerProxy::SetMissionIcon(const sptr<IRemoteObject> &token,
    const std::shared_ptr<OHOS::Media::PixelMap> &icon)
{
    if (!token || !icon) {
        TAG_LOGE(AAFwkTag::MISSION, "abilitytoken or icon invalid");
        return INVALID_PARAMETERS_ERR;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return INVALID_PARAMETERS_ERR;
    }
    if (!data.WriteRemoteObject(token)) {
        TAG_LOGE(AAFwkTag::MISSION, "write token fail");
        return INVALID_PARAMETERS_ERR;
    }

    if (!data.WriteParcelable(icon.get())) {
        TAG_LOGE(AAFwkTag::MISSION, "write icon fail");
        return INVALID_PARAMETERS_ERR;
    }

    auto error = SendRequest(AbilityManagerInterfaceCode::SET_MISSION_ICON, data, reply, option);
    if (error != NO_ERROR) {
        TAG_LOGE(AAFwkTag::MISSION, "request error:%{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}

void MissionManagerProxy::UpdateMissionSnapShot(const sptr<IRemoteObject> &token,
    const std::shared_ptr<Media::PixelMap> &pixelMap)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);

    if (!WriteInterfaceToken(data)) {
        return;
    }
    if (!data.WriteRemoteObject(token)) {
        TAG_LOGE(AAFwkTag::MISSION, "write token fail");
        return;
    }
    if (!data.WriteParcelable(pixelMap.get())) {
        TAG_LOGE(AAFwkTag::MISSION, "write pixelMap fail");
        return;
    }
    auto error = SendRequest(AbilityManagerInterfaceCode::UPDATE_MISSION_SNAPSHOT_FROM_WMS,
        data, reply, option);
    if (error != NO_ERROR) {
        TAG_LOGE(AAFwkTag::MISSION, "request error:%{public}d", error);
    }
}
#endif

int32_t MissionManagerProxy::IsValidMissionIds(
    const std::vector<int32_t> &missionIds, std::vector<MissionValidResult> &results)
{
    TAG_LOGI(AAFwkTag::MISSION, "call, query size:%{public}zu", missionIds.size());
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::MISSION, "write token fail");
        return INVALID_PARAMETERS_ERR;
    }

    constexpr int32_t MAX_COUNT = 20;
    int32_t num = static_cast<int32_t>(missionIds.size() > MAX_COUNT ? MAX_COUNT : missionIds.size());
    if (!data.WriteInt32(num)) {
        TAG_LOGE(AAFwkTag::MISSION, "write num fail");
        return INVALID_PARAMETERS_ERR;
    }
    for (auto i = 0; i < num; ++i) {
        if (!data.WriteInt32(missionIds.at(i))) {
            TAG_LOGE(AAFwkTag::MISSION, "write missionId fail");
            return INVALID_PARAMETERS_ERR;
        }
    }

    auto error = SendRequest(AbilityManagerInterfaceCode::QUERY_MISSION_VAILD, data, reply, option);
    if (error != NO_ERROR) {
        TAG_LOGE(AAFwkTag::MISSION, "request error:%{public}d", error);
        return error;
    }

    auto resultCode = reply.ReadInt32();
    if (resultCode != ERR_OK) {
        TAG_LOGE(AAFwkTag::MISSION, "request error:%{public}d", resultCode);
        return resultCode;
    }

    auto infoSize = reply.ReadInt32();
    for (auto i = 0; i < infoSize && i < MAX_COUNT; ++i) {
        std::unique_ptr<MissionValidResult> info(reply.ReadParcelable<MissionValidResult>());
        if (!info) {
            TAG_LOGE(AAFwkTag::MISSION, "read result fail");
            return INVALID_PARAMETERS_ERR;
        }
        results.emplace_back(*info);
    }

    return resultCode;
}

int32_t MissionManagerProxy::PreStartMission(const std::string &bundleName, const std::string &moduleName,
    const std::string &abilityName, const std::string &startTime)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return INVALID_PARAMETERS_ERR;
    }
    if (!data.WriteString(bundleName)) {
        TAG_LOGE(AAFwkTag::MISSION, "write bundleName fail");
        return INVALID_PARAMETERS_ERR;
    }
    if (!data.WriteString(moduleName)) {
        TAG_LOGE(AAFwkTag::MISSION, "write moduleName fail");
        return INVALID_PARAMETERS_ERR;
    }
    if (!data.WriteString(abilityName)) {
        TAG_LOGE(AAFwkTag::MISSION, "write abilityName fail");
        return INVALID_PARAMETERS_ERR;
    }
    if (!data.WriteString(startTime)) {
        TAG_LOGE(AAFwkTag::MISSION, "write startTime fail");
        return INVALID_PARAMETERS_ERR;
    }
    auto error = SendRequest(AbilityManagerInterfaceCode::PRE_START_MISSION, data, reply, option);
    if (error != NO_ERROR) {
        TAG_LOGE(AAFwkTag::MISSION, "request error:%{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}

int32_t MissionManagerProxy::TerminateMission(int32_t missionId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return INVALID_PARAMETERS_ERR;
    }
    if (!data.WriteInt32(missionId)) {
        TAG_LOGE(AAFwkTag::MISSION, "missionId write fail");
        return INVALID_PARAMETERS_ERR;
    }

    auto error = SendRequest(AbilityManagerInterfaceCode::TERMINATE_MISSION,
        data, reply, option);
    if (error != NO_ERROR) {
        TAG_LOGE(AAFwkTag::MISSION, "request error: %{public}d", error);
        return error;
    }

    return reply.ReadInt32();
}
}  // namespace OHOS::AAFwk
