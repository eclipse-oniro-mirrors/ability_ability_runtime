/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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
#include "distributed_client.h"

#include "ability_manager_errors.h"
#include "distributed_parcel_helper.h"
#include "hilog_tag_wrapper.h"
#include "if_system_ability_manager.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "string_ex.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace AAFwk {
namespace {
const std::u16string DMS_PROXY_INTERFACE_TOKEN = u"ohos.distributedschedule.accessToken";
}
sptr<IRemoteObject> DistributedClient::GetDmsProxy()
{
    auto samgrProxy = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgrProxy == nullptr) {
        TAG_LOGE(AAFwkTag::DISTRIBUTED, "null samgrProxy");
        return nullptr;
    }
    return samgrProxy->CheckSystemAbility(DISTRIBUTED_SCHED_SA_ID);
}

int32_t DistributedClient::StartRemoteAbility(const OHOS::AAFwk::Want& want,
    int32_t callerUid, int32_t requestCode, uint32_t accessToken, uint32_t specifyTokenId)
{
    sptr<IRemoteObject> remote = GetDmsProxy();
    if (remote == nullptr) {
        return INVALID_PARAMETERS_ERR;
    }
    MessageParcel data;
    if (!data.WriteInterfaceToken(DMS_PROXY_INTERFACE_TOKEN)) {
        return ERR_FLATTEN_OBJECT;
    }
    PARCEL_WRITE_HELPER(data, Parcelable, &want);
    PARCEL_WRITE_HELPER(data, Int32, callerUid);
    PARCEL_WRITE_HELPER(data, Int32, requestCode);
    PARCEL_WRITE_HELPER(data, Uint32, accessToken);
    PARCEL_WRITE_HELPER(data, Uint32, specifyTokenId);
    MessageParcel reply;
    PARCEL_TRANSACT_SYNC_RET_INT(remote, START_REMOTE_ABILITY, data, reply);
}

int32_t DistributedClient::ConnectRemoteAbility(const OHOS::AAFwk::Want& want, const sptr<IRemoteObject>& connect)
{
    if (connect == nullptr) {
        TAG_LOGE(AAFwkTag::DISTRIBUTED, "null connect");
        return ERR_NULL_OBJECT;
    }

    sptr<IRemoteObject> remote = GetDmsProxy();
    if (remote == nullptr) {
        TAG_LOGE(AAFwkTag::DISTRIBUTED, "null remote");
        return INVALID_PARAMETERS_ERR;
    }
    MessageParcel data;
    if (!WriteInfosToParcel(data, want, connect)) {
        return ERR_FLATTEN_OBJECT;
    }
    MessageParcel reply;
    PARCEL_TRANSACT_SYNC_RET_INT(remote, CONNECT_REMOTE_ABILITY, data, reply);
}

int32_t DistributedClient::DisconnectRemoteAbility(const sptr<IRemoteObject>& connect, int32_t callerUid,
    uint32_t accessToken)
{
    if (connect == nullptr) {
        TAG_LOGE(AAFwkTag::DISTRIBUTED, "null connect");
        return ERR_NULL_OBJECT;
    }

    sptr<IRemoteObject> remote = GetDmsProxy();
    if (remote == nullptr) {
        TAG_LOGE(AAFwkTag::DISTRIBUTED, "null remote");
        return INVALID_PARAMETERS_ERR;
    }
    MessageParcel data;
    if (!data.WriteInterfaceToken(DMS_PROXY_INTERFACE_TOKEN)) {
        return ERR_FLATTEN_OBJECT;
    }
    PARCEL_WRITE_HELPER(data, RemoteObject, connect);
    PARCEL_WRITE_HELPER(data, Int32, callerUid);
    PARCEL_WRITE_HELPER(data, Uint32, accessToken);
    MessageParcel reply;
    PARCEL_TRANSACT_SYNC_RET_INT(remote, DISCONNECT_REMOTE_ABILITY, data, reply);
}

int32_t DistributedClient::ContinueMission(const std::string& srcDeviceId, const std::string& dstDeviceId,
    int32_t missionId, const sptr<IRemoteObject>& callback, const OHOS::AAFwk::WantParams& wantParams)
{
    if (callback == nullptr) {
        TAG_LOGE(AAFwkTag::DISTRIBUTED, "null callback");
        return ERR_NULL_OBJECT;
    }
    sptr<IRemoteObject> remote = GetDmsProxy();
    if (remote == nullptr) {
        TAG_LOGE(AAFwkTag::DISTRIBUTED, "null remote");
        return INVALID_PARAMETERS_ERR;
    }
    MessageParcel data;
    if (!data.WriteInterfaceToken(DMS_PROXY_INTERFACE_TOKEN)) {
        return ERR_FLATTEN_OBJECT;
    }
    PARCEL_WRITE_HELPER(data, String, srcDeviceId);
    PARCEL_WRITE_HELPER(data, String, dstDeviceId);
    PARCEL_WRITE_HELPER(data, Int32, missionId);
    PARCEL_WRITE_HELPER(data, RemoteObject, callback);
    PARCEL_WRITE_HELPER(data, Parcelable, &wantParams);
    MessageParcel reply;
    PARCEL_TRANSACT_SYNC_RET_INT(remote, CONTINUE_MISSION, data, reply);
}

int32_t DistributedClient::ContinueMission(AAFwk::ContinueMissionInfo continueMissionInfo,
    const sptr<IRemoteObject> &callback)
{
    if (callback == nullptr) {
        TAG_LOGE(AAFwkTag::DISTRIBUTED, "null callback");
        return ERR_NULL_OBJECT;
    }
    sptr<IRemoteObject> remote = GetDmsProxy();
    if (remote == nullptr) {
        TAG_LOGE(AAFwkTag::DISTRIBUTED, "null remote");
        return INVALID_PARAMETERS_ERR;
    }
    MessageParcel data;
    if (!data.WriteInterfaceToken(DMS_PROXY_INTERFACE_TOKEN)) {
        return ERR_FLATTEN_OBJECT;
    }
    PARCEL_WRITE_HELPER(data, String, continueMissionInfo.srcDeviceId);
    PARCEL_WRITE_HELPER(data, String, continueMissionInfo.dstDeviceId);
    PARCEL_WRITE_HELPER(data, String, continueMissionInfo.bundleName);
    PARCEL_WRITE_HELPER(data, RemoteObject, callback);
    PARCEL_WRITE_HELPER(data, Parcelable, &continueMissionInfo.wantParams);
    PARCEL_WRITE_HELPER(data, String, continueMissionInfo.srcBundleName);
    PARCEL_WRITE_HELPER(data, String, continueMissionInfo.continueType);
    MessageParcel reply;
    PARCEL_TRANSACT_SYNC_RET_INT(remote, CONTINUE_MISSION_OF_BUNDLENAME, data, reply);
}

int32_t DistributedClient::StartContinuation(const OHOS::AAFwk::Want& want, int32_t missionId, int32_t callerUid,
    int32_t status, uint32_t accessToken)
{
    sptr<IRemoteObject> remote = GetDmsProxy();
    if (remote == nullptr) {
        TAG_LOGE(AAFwkTag::DISTRIBUTED, "null remote");
        return INVALID_PARAMETERS_ERR;
    }
    MessageParcel data;
    if (!data.WriteInterfaceToken(DMS_PROXY_INTERFACE_TOKEN)) {
        TAG_LOGD(AAFwkTag::DISTRIBUTED, "write interface token failed");
        return ERR_FLATTEN_OBJECT;
    }
    PARCEL_WRITE_HELPER(data, Parcelable, &want);
    PARCEL_WRITE_HELPER(data, Int32, missionId);
    PARCEL_WRITE_HELPER(data, Int32, callerUid);
    PARCEL_WRITE_HELPER(data, Int32, status);
    PARCEL_WRITE_HELPER(data, Uint32, accessToken);
    MessageParcel reply;
    PARCEL_TRANSACT_SYNC_RET_INT(remote, START_CONTINUATION, data, reply);
}

ErrCode DistributedClient::NotifyCompleteContinuation(
    const std::u16string &devId, int32_t sessionId, bool isSuccess, const std::string &callerBundleName)
{
    sptr<IRemoteObject> remote = GetDmsProxy();
    if (remote == nullptr) {
        return INVALID_PARAMETERS_ERR;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(DMS_PROXY_INTERFACE_TOKEN)) {
        return ERR_FLATTEN_OBJECT;
    }
    if (!data.WriteString16(devId)) {
        TAG_LOGE(AAFwkTag::DISTRIBUTED, "write deviceId error");
        return ERR_FLATTEN_OBJECT;
    }
    if (!data.WriteInt32(sessionId)) {
        TAG_LOGE(AAFwkTag::DISTRIBUTED, "write sessionId error");
        return ERR_FLATTEN_OBJECT;
    }
    if (!data.WriteBool(isSuccess)) {
        TAG_LOGE(AAFwkTag::DISTRIBUTED, "write result error");
        return ERR_FLATTEN_OBJECT;
    }
    if (!data.WriteString(callerBundleName)) {
        TAG_LOGE(AAFwkTag::DISTRIBUTED, "write callerBundleName error");
        return ERR_FLATTEN_OBJECT;
    }
    MessageParcel reply;
    MessageOption option = {MessageOption::TF_ASYNC};
    int32_t result = remote->SendRequest(NOTIFY_COMPLETE_CONTINUATION, data, reply, option);
    if (result != ERR_NONE) {
        TAG_LOGE(AAFwkTag::DISTRIBUTED, "result = %{public}d", result);
        return result;
    }
    return ERR_OK;
}

int32_t DistributedClient::StartSyncRemoteMissions(const std::string& devId, bool fixConflict, int64_t tag,
    int32_t callingUid, uint32_t callingTokenId)
{
    sptr<IRemoteObject> remote = GetDmsProxy();
    if (remote == nullptr) {
        TAG_LOGE(AAFwkTag::DISTRIBUTED, "null remote");
        return INVALID_PARAMETERS_ERR;
    }

    MessageParcel data;
    MessageParcel reply;
    if (!data.WriteInterfaceToken(DMS_PROXY_INTERFACE_TOKEN)) {
        return ERR_FLATTEN_OBJECT;
    }
    PARCEL_WRITE_HELPER(data, String16, Str8ToStr16(devId));
    PARCEL_WRITE_HELPER(data, Bool, fixConflict);
    PARCEL_WRITE_HELPER(data, Int64, tag);
    PARCEL_WRITE_HELPER(data, Int32, callingUid);
    PARCEL_WRITE_HELPER(data, Uint32, callingTokenId);
    PARCEL_TRANSACT_SYNC_RET_INT(remote, START_SYNC_MISSIONS, data, reply);
}

int32_t DistributedClient::StopSyncRemoteMissions(const std::string& devId, int32_t callingUid)
{
    sptr<IRemoteObject> remote = GetDmsProxy();
    if (remote == nullptr) {
        TAG_LOGE(AAFwkTag::DISTRIBUTED, "null remote");
        return INVALID_PARAMETERS_ERR;
    }
    MessageParcel data;
    MessageParcel reply;
    if (!data.WriteInterfaceToken(DMS_PROXY_INTERFACE_TOKEN)) {
        return ERR_FLATTEN_OBJECT;
    }
    PARCEL_WRITE_HELPER(data, String16, Str8ToStr16(devId));
    PARCEL_WRITE_HELPER(data, Int32, callingUid);
    PARCEL_TRANSACT_SYNC_RET_INT(remote, STOP_SYNC_MISSIONS, data, reply);
}

int32_t DistributedClient::RegisterMissionListener(const std::u16string& devId,
    const sptr<IRemoteObject>& obj, int32_t callingUid)
{
    sptr<IRemoteObject> remote = GetDmsProxy();
    if (remote == nullptr) {
        TAG_LOGE(AAFwkTag::DISTRIBUTED, "null remote");
        return INVALID_PARAMETERS_ERR;
    }
    MessageParcel data;
    MessageParcel reply;
    if (!data.WriteInterfaceToken(DMS_PROXY_INTERFACE_TOKEN)) {
        return ERR_FLATTEN_OBJECT;
    }
    PARCEL_WRITE_HELPER(data, String16, devId);
    PARCEL_WRITE_HELPER(data, RemoteObject, obj);
    PARCEL_WRITE_HELPER(data, Int32, callingUid);
    PARCEL_TRANSACT_SYNC_RET_INT(remote, REGISTER_MISSION_LISTENER, data, reply);
}

int32_t DistributedClient::RegisterOnListener(const std::string& type,
    const sptr<IRemoteObject>& obj, int32_t callingUid)
{
    sptr<IRemoteObject> remote = GetDmsProxy();
    if (remote == nullptr) {
        TAG_LOGE(AAFwkTag::DISTRIBUTED, "null remote");
        return INVALID_PARAMETERS_ERR;
    }
    MessageParcel data;
    MessageParcel reply;
    if (!data.WriteInterfaceToken(DMS_PROXY_INTERFACE_TOKEN)) {
        return ERR_FLATTEN_OBJECT;
    }
    PARCEL_WRITE_HELPER(data, String, type);
    PARCEL_WRITE_HELPER(data, RemoteObject, obj);
    PARCEL_WRITE_HELPER(data, Int32, callingUid);
    PARCEL_TRANSACT_SYNC_RET_INT(remote, REGISTER_ON_LISTENER, data, reply);
}

int32_t DistributedClient::RegisterOffListener(const std::string& type,
    const sptr<IRemoteObject>& obj, int32_t callingUid)
{
    sptr<IRemoteObject> remote = GetDmsProxy();
    if (remote == nullptr) {
        TAG_LOGE(AAFwkTag::DISTRIBUTED, "null remote");
        return INVALID_PARAMETERS_ERR;
    }
    MessageParcel data;
    MessageParcel reply;
    if (!data.WriteInterfaceToken(DMS_PROXY_INTERFACE_TOKEN)) {
        TAG_LOGE(AAFwkTag::DISTRIBUTED, "write token failed");
        return ERR_FLATTEN_OBJECT;
    }
    PARCEL_WRITE_HELPER(data, String, type);
    PARCEL_WRITE_HELPER(data, RemoteObject, obj);
    PARCEL_WRITE_HELPER(data, Int32, callingUid);
    PARCEL_TRANSACT_SYNC_RET_INT(remote, REGISTER_OFF_LISTENER, data, reply);
}

int32_t DistributedClient::UnRegisterMissionListener(const std::u16string& devId,
    const sptr<IRemoteObject>& obj)
{
    sptr<IRemoteObject> remote = GetDmsProxy();
    if (remote == nullptr) {
        TAG_LOGE(AAFwkTag::DISTRIBUTED, "null remote");
        return INVALID_PARAMETERS_ERR;
    }
    MessageParcel data;
    MessageParcel reply;
    if (!data.WriteInterfaceToken(DMS_PROXY_INTERFACE_TOKEN)) {
        return ERR_FLATTEN_OBJECT;
    }
    PARCEL_WRITE_HELPER(data, String16, devId);
    PARCEL_WRITE_HELPER(data, RemoteObject, obj);
    PARCEL_TRANSACT_SYNC_RET_INT(remote, UNREGISTER_MISSION_LISTENER, data, reply);
}

int32_t DistributedClient::GetMissionInfos(const std::string& deviceId, int32_t numMissions,
    std::vector<AAFwk::MissionInfo>& missionInfos)
{
    sptr<IRemoteObject> remote = GetDmsProxy();
    if (remote == nullptr) {
        TAG_LOGE(AAFwkTag::DISTRIBUTED, "null remote");
        return INVALID_PARAMETERS_ERR;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(DMS_PROXY_INTERFACE_TOKEN)) {
        TAG_LOGE(AAFwkTag::DISTRIBUTED, "write token failed");
        return ERR_FLATTEN_OBJECT;
    }
    PARCEL_WRITE_HELPER(data, String16, Str8ToStr16(deviceId));
    PARCEL_WRITE_HELPER(data, Int32, numMissions);
    int32_t ret = remote->SendRequest(GET_MISSION_INFOS, data, reply, option);
    if (ret != ERR_NONE) {
        TAG_LOGW(AAFwkTag::DISTRIBUTED, "error: %{public}d", ret);
        return ret;
    }
    return ReadMissionInfosFromParcel(reply, missionInfos) ? ERR_NONE : ERR_FLATTEN_OBJECT;
}

int32_t DistributedClient::GetRemoteMissionSnapshotInfo(const std::string& deviceId, int32_t missionId,
    std::unique_ptr<MissionSnapshot>& missionSnapshot)
{
    if (deviceId.empty()) {
        TAG_LOGE(AAFwkTag::DISTRIBUTED, "null deviceId");
        return ERR_NULL_OBJECT;
    }
    sptr<IRemoteObject> remote = GetDmsProxy();
    if (remote == nullptr) {
        TAG_LOGE(AAFwkTag::DISTRIBUTED, "null remote");
        return INVALID_PARAMETERS_ERR;
    }
    MessageParcel data;
    if (!data.WriteInterfaceToken(DMS_PROXY_INTERFACE_TOKEN)) {
        return ERR_FLATTEN_OBJECT;
    }
    PARCEL_WRITE_HELPER(data, String, deviceId);
    PARCEL_WRITE_HELPER(data, Int32, missionId);
    MessageParcel reply;
    MessageOption option;
    int32_t error = remote->SendRequest(GET_REMOTE_MISSION_SNAPSHOT_INFO, data, reply, option);
    if (error != ERR_NONE) {
        TAG_LOGE(AAFwkTag::DISTRIBUTED, "error: %{public}d", error);
        return error;
    }
    std::unique_ptr<MissionSnapshot> missionSnapshotPtr(reply.ReadParcelable<MissionSnapshot>());
    missionSnapshot = std::move(missionSnapshotPtr);
    return ERR_NONE;
}

bool DistributedClient::ReadMissionInfosFromParcel(Parcel& parcel,
    std::vector<AAFwk::MissionInfo>& missionInfos)
{
    int32_t hasMissions = parcel.ReadInt32();
    if (hasMissions == 1) {
        int32_t len = parcel.ReadInt32();
        TAG_LOGD(AAFwkTag::DISTRIBUTED, "readLength is:%{public}d", len);
        if (len < 0) {
            return false;
        }
        size_t size = static_cast<size_t>(len);
        if ((size > parcel.GetReadableBytes()) || (missionInfos.max_size() < size)) {
            TAG_LOGE(AAFwkTag::DISTRIBUTED, "failed, size = %{public}zu", size);
            return false;
        }
        missionInfos.clear();
        for (size_t i = 0; i < size; i++) {
            AAFwk::MissionInfo *ptr = parcel.ReadParcelable<AAFwk::MissionInfo>();
            if (ptr == nullptr) {
                TAG_LOGW(AAFwkTag::DISTRIBUTED, "read MissionInfo failed");
                return false;
            }
            missionInfos.emplace_back(*ptr);
            delete ptr;
        }
    }
    return true;
}

int32_t DistributedClient::StartRemoteAbilityByCall(const OHOS::AAFwk::Want& want, const sptr<IRemoteObject>& connect)
{
    if (connect == nullptr) {
        TAG_LOGE(AAFwkTag::DISTRIBUTED, "null connect");
        return ERR_NULL_OBJECT;
    }

    sptr<IRemoteObject> remote = GetDmsProxy();
    if (remote == nullptr) {
        TAG_LOGE(AAFwkTag::DISTRIBUTED, "null remote");
        return INVALID_PARAMETERS_ERR;
    }
    MessageParcel data;
    if (!WriteInfosToParcel(data, want, connect)) {
        return ERR_FLATTEN_OBJECT;
    }
    MessageParcel reply;
    PARCEL_TRANSACT_SYNC_RET_INT(remote, START_REMOTE_ABILITY_BY_CALL, data, reply);
}

int32_t DistributedClient::ReleaseRemoteAbility(const sptr<IRemoteObject>& connect,
    const AppExecFwk::ElementName &element)
{
    if (connect == nullptr) {
        TAG_LOGE(AAFwkTag::DISTRIBUTED, "null connect");
        return ERR_NULL_OBJECT;
    }

    sptr<IRemoteObject> remote = GetDmsProxy();
    if (remote == nullptr) {
        TAG_LOGE(AAFwkTag::DISTRIBUTED, "null remote");
        return INVALID_PARAMETERS_ERR;
    }
    MessageParcel data;
    if (!data.WriteInterfaceToken(DMS_PROXY_INTERFACE_TOKEN)) {
        return ERR_FLATTEN_OBJECT;
    }
    PARCEL_WRITE_HELPER(data, RemoteObject, connect);
    if (!data.WriteParcelable(&element)) {
        TAG_LOGE(AAFwkTag::DISTRIBUTED, "write element error");
        return ERR_INVALID_VALUE;
    }
    MessageParcel reply;
    PARCEL_TRANSACT_SYNC_RET_INT(remote, RELEASE_REMOTE_ABILITY, data, reply);
}

int32_t DistributedClient::StartRemoteFreeInstall(const OHOS::AAFwk::Want& want,
    int32_t callerUid, int32_t requestCode, uint32_t accessToken, const sptr<IRemoteObject>& callback)
{
    if (callback == nullptr) {
        TAG_LOGE(AAFwkTag::DISTRIBUTED, "null callback");
        return ERR_NULL_OBJECT;
    }

    sptr<IRemoteObject> remote = GetDmsProxy();
    if (remote == nullptr) {
        TAG_LOGE(AAFwkTag::DISTRIBUTED, "null remote");
        return INVALID_PARAMETERS_ERR;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(DMS_PROXY_INTERFACE_TOKEN)) {
        TAG_LOGE(AAFwkTag::DISTRIBUTED, "write token failed");
        return ERR_FLATTEN_OBJECT;
    }

    PARCEL_WRITE_HELPER(data, Parcelable, &want);
    PARCEL_WRITE_HELPER(data, Int32, callerUid);
    PARCEL_WRITE_HELPER(data, Int32, requestCode);
    PARCEL_WRITE_HELPER(data, Uint32, accessToken);
    PARCEL_WRITE_HELPER(data, RemoteObject, callback);
    MessageParcel reply;
    PARCEL_TRANSACT_SYNC_RET_INT(remote, START_REMOTE_FREE_INSTALL, data, reply);
}

bool DistributedClient::WriteInfosToParcel(MessageParcel& data, const OHOS::AAFwk::Want& want,
    const sptr<IRemoteObject>& connect)
{
    int32_t callerUid = IPCSkeleton::GetCallingUid();
    int32_t callerPid = IPCSkeleton::GetCallingPid();
    uint32_t accessToken = IPCSkeleton::GetCallingTokenID();
    if (!data.WriteInterfaceToken(DMS_PROXY_INTERFACE_TOKEN)) {
        return false;
    }
    PARCEL_WRITE_HELPER_RET(data, Parcelable, &want, false);
    PARCEL_WRITE_HELPER_RET(data, RemoteObject, connect, false);
    PARCEL_WRITE_HELPER_RET(data, Int32, callerUid, false);
    PARCEL_WRITE_HELPER_RET(data, Int32, callerPid, false);
    PARCEL_WRITE_HELPER_RET(data, Uint32, accessToken, false);
    return true;
}

int32_t DistributedClient::StopRemoteExtensionAbility(const Want &want, int32_t callerUid,
    uint32_t accessToken, int32_t extensionType)
{
    sptr<IRemoteObject> remote = GetDmsProxy();
    if (remote == nullptr) {
        TAG_LOGE(AAFwkTag::DISTRIBUTED, "null remote");
        return INVALID_PARAMETERS_ERR;
    }
    MessageParcel data;
    if (!data.WriteInterfaceToken(DMS_PROXY_INTERFACE_TOKEN)) {
        TAG_LOGE(AAFwkTag::DISTRIBUTED, "WriteInterfaceToken failed");
        return ERR_FLATTEN_OBJECT;
    }
    PARCEL_WRITE_HELPER(data, Parcelable, &want);
    PARCEL_WRITE_HELPER(data, Int32, callerUid);
    PARCEL_WRITE_HELPER(data, Uint32, accessToken);
    PARCEL_WRITE_HELPER(data, Int32, extensionType);
    MessageParcel reply;
    PARCEL_TRANSACT_SYNC_RET_INT(remote, STOP_REMOTE_EXTERNSION_ABILITY, data, reply);
}

int32_t DistributedClient::SetMissionContinueState(int32_t missionId, const AAFwk::ContinueState &state,
    int32_t callingUid)
{
    sptr<IRemoteObject> remote = GetDmsProxy();
    if (remote == nullptr) {
        TAG_LOGE(AAFwkTag::DISTRIBUTED, "null remote");
        return INVALID_PARAMETERS_ERR;
    }
    MessageParcel data;
    MessageParcel reply;
    if (!data.WriteInterfaceToken(DMS_PROXY_INTERFACE_TOKEN)) {
        return ERR_FLATTEN_OBJECT;
    }
    PARCEL_WRITE_HELPER(data, Int32, missionId);
    PARCEL_WRITE_HELPER(data, Int32, static_cast<int32_t>(state));
    PARCEL_WRITE_HELPER(data, Int32, callingUid);
    PARCEL_TRANSACT_SYNC_RET_INT(remote, SET_MISSION_CONTINUE_STATE, data, reply);
}

int32_t DistributedClient::OnCollaborateDone(
    const std::string &collabToken, int32_t result, int32_t pid, int32_t uid, int32_t accessTokenId)
{
    sptr<IRemoteObject> remote = GetDmsProxy();
    if (remote == nullptr) {
        TAG_LOGE(AAFwkTag::DISTRIBUTED, "null remote");
        return ERR_NULL_OBJECT;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(DMS_PROXY_INTERFACE_TOKEN)) {
        TAG_LOGE(AAFwkTag::DISTRIBUTED, "WriteInterfaceToken fail");
        return ERR_FLATTEN_OBJECT;
    }
    PARCEL_WRITE_HELPER(data, String, collabToken);
    PARCEL_WRITE_HELPER(data, Int32, result);
    PARCEL_WRITE_HELPER(data, Int32, pid);
    PARCEL_WRITE_HELPER(data, Int32, uid);
    PARCEL_WRITE_HELPER(data, Int32, accessTokenId);

    MessageParcel reply;
    PARCEL_TRANSACT_SYNC_RET_INT(remote,
        static_cast<uint32_t>(NOTIFY_ON_COLLABORATE_DONE), data, reply);
}

int32_t DistributedClient::StartRemoteIntent(const OHOS::AAFwk::Want& want,
    const OHOS::AAFwk::IntentCallerInfo& callerInfo,
    const sptr<IRemoteObject>& resultCallback)
{
    sptr<IRemoteObject> remote = GetDmsProxy();
    if (remote == nullptr) {
        TAG_LOGE(AAFwkTag::DISTRIBUTED, "StartRemoteIntent null remote");
        return INVALID_PARAMETERS_ERR;
    }
    MessageParcel data;
    if (!data.WriteInterfaceToken(DMS_PROXY_INTERFACE_TOKEN)) {
        return ERR_FLATTEN_OBJECT;
    }
    PARCEL_WRITE_HELPER(data, Parcelable, &want);
    PARCEL_WRITE_HELPER(data, String, want.GetElement().GetModuleName());
    PARCEL_WRITE_HELPER(data, Int32, callerInfo.callerUid);
    PARCEL_WRITE_HELPER(data, Uint64, callerInfo.requestCode);
    PARCEL_WRITE_HELPER(data, Uint32, callerInfo.accessToken);
    PARCEL_WRITE_HELPER(data, Uint32, callerInfo.specifyTokenId);
    PARCEL_WRITE_HELPER(data, RemoteObject, resultCallback);
    MessageParcel reply;
    PARCEL_TRANSACT_SYNC_RET_INT(remote, START_REMOTE_INTENT, data, reply);
}

int32_t DistributedClient::SendIntentResult(const OHOS::AAFwk::Want& want,
    const OHOS::AAFwk::IntentCallerInfo& callerInfo, const std::string& msg)
{
    sptr<IRemoteObject> remote = GetDmsProxy();
    if (remote == nullptr) {
        TAG_LOGE(AAFwkTag::DISTRIBUTED, "SendIntentResult null remote");
        return INVALID_PARAMETERS_ERR;
    }
    MessageParcel data;
    if (!data.WriteInterfaceToken(DMS_PROXY_INTERFACE_TOKEN)) {
        return ERR_FLATTEN_OBJECT;
    }
    PARCEL_WRITE_HELPER(data, Parcelable, &want);
    PARCEL_WRITE_HELPER(data, Int32, callerInfo.callerUid);
    PARCEL_WRITE_HELPER(data, Uint64, callerInfo.requestCode);
    PARCEL_WRITE_HELPER(data, Uint32, callerInfo.accessToken);
    PARCEL_WRITE_HELPER(data, Uint32, callerInfo.specifyTokenId);
    PARCEL_WRITE_HELPER(data, String, msg);
    MessageParcel reply;
    PARCEL_TRANSACT_SYNC_RET_INT(remote, SEND_INTENT_RESULT, data, reply);
}
}  // namespace AAFwk
}  // namespace OHOS
