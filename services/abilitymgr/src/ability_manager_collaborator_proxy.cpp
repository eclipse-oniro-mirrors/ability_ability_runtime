/*
 * Copyright (c) 2023-2026 Huawei Device Co., Ltd.
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

#include "ability_manager_collaborator_proxy.h"

#include "configuration.h"
#include "hilog_tag_wrapper.h"
#include "ipc_capacity_wrap.h"

namespace OHOS {
namespace AAFwk {
namespace {
constexpr int32_t MAX_URI_COUNT = 500;
}
int32_t AbilityManagerCollaboratorProxy::NotifyStartAbility(
    const AppExecFwk::AbilityInfo &abilityInfo, int32_t userId, Want &want,
    uint64_t accessTokenIDEx, int32_t windowMode)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    AAFwk::ExtendMaxIpcCapacityForInnerWant(data);

    if (!data.WriteInterfaceToken(AbilityManagerCollaboratorProxy::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write token fail");
        return ERR_INVALID_OPERATION;
    }
    if (!data.WriteParcelable(&abilityInfo)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "abilityInfo write fail");
        return ERR_INVALID_OPERATION;
    }
    if (!data.WriteInt32(userId)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "userId write fail");
        return ERR_INVALID_OPERATION;
    }
    if (!data.WriteParcelable(&want)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "want write fail");
        return ERR_INVALID_OPERATION;
    }
    if (!data.WriteUint64(accessTokenIDEx)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "accessTokenIDEx write fail");
        return ERR_INVALID_OPERATION;
    }
    if (!data.WriteInt32(windowMode)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "windowMode write fail");
        return ERR_INVALID_OPERATION;
    }
    int32_t ret = SendTransactCmd(IAbilityManagerCollaborator::NOTIFY_START_ABILITY, data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "request error:%{public}d", ret);
        return ret;
    }
    ret = reply.ReadInt32();
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "start ability failed");
        return ERR_INVALID_OPERATION;
    }
    std::unique_ptr<Want> wantInfo(reply.ReadParcelable<Want>());
    if (!wantInfo) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "readParcelable fail");
        return ERR_INVALID_OPERATION;
    }
    want = *wantInfo;
    return NO_ERROR;
}

int32_t AbilityManagerCollaboratorProxy::NotifyMissionCreated(int32_t missionId, const Want &want)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    AAFwk::ExtendMaxIpcCapacityForInnerWant(data);

    if (!data.WriteInterfaceToken(AbilityManagerCollaboratorProxy::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write token fail");
        return ERR_INVALID_OPERATION;
    }
    if (!data.WriteInt32(missionId)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "missionId write fail");
        return ERR_INVALID_OPERATION;
    }
    if (!data.WriteParcelable(&want)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "want write fail");
        return ERR_INVALID_OPERATION;
    }
    int32_t ret = SendTransactCmd(IAbilityManagerCollaborator::NOTIFY_MISSION_CREATED, data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "request error:%{public}d", ret);
        return ret;
    }
    return NO_ERROR;
}

int32_t AbilityManagerCollaboratorProxy::NotifyMissionCreated(const sptr<SessionInfo> &sessionInfo)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    if (!data.WriteInterfaceToken(AbilityManagerCollaboratorProxy::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write token fail");
        return ERR_INVALID_OPERATION;
    }
    if (sessionInfo) {
        AAFwk::ExtendMaxIpcCapacityForInnerWant(data);
        if (!data.WriteBool(true) || !data.WriteParcelable(sessionInfo)) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "flag and sessionInfo write fail");
            return ERR_INVALID_OPERATION;
        }
    } else {
        if (!data.WriteBool(false)) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "flag write fail");
            return ERR_INVALID_OPERATION;
        }
    }
    int32_t ret = SendTransactCmd(IAbilityManagerCollaborator::NOTIFY_MISSION_CREATED_BY_SCB, data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "request error:%{public}d", ret);
        return ret;
    }
    return NO_ERROR;
}

int32_t AbilityManagerCollaboratorProxy::NotifyLoadAbility(
    const AppExecFwk::AbilityInfo &abilityInfo, int32_t missionId, const Want &want)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    AAFwk::ExtendMaxIpcCapacityForInnerWant(data);

    if (!data.WriteInterfaceToken(AbilityManagerCollaboratorProxy::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write token fail");
        return ERR_INVALID_OPERATION;
    }
    if (!data.WriteParcelable(&abilityInfo)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "abilityInfo write fail");
        return ERR_INVALID_OPERATION;
    }
    if (!data.WriteInt32(missionId)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "missionId write fail");
        return ERR_INVALID_OPERATION;
    }
    if (!data.WriteParcelable(&want)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "want write fail");
        return ERR_INVALID_OPERATION;
    }
    int32_t ret = SendTransactCmd(IAbilityManagerCollaborator::NOTIFY_LOAD_ABILITY, data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "request error:%{public}d", ret);
        return ret;
    }
    return NO_ERROR;
}

int32_t AbilityManagerCollaboratorProxy::NotifyLoadAbility(
    const AppExecFwk::AbilityInfo &abilityInfo, const sptr<SessionInfo> &sessionInfo)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    if (!data.WriteInterfaceToken(AbilityManagerCollaboratorProxy::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write token fail");
        return ERR_INVALID_OPERATION;
    }
    if (!data.WriteParcelable(&abilityInfo)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "abilityInfo write fail");
        return ERR_INVALID_OPERATION;
    }
    if (sessionInfo) {
        AAFwk::ExtendMaxIpcCapacityForInnerWant(data);
        if (!data.WriteBool(true) || !data.WriteParcelable(sessionInfo)) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "flag and sessionInfo write fail");
            return ERR_INVALID_OPERATION;
        }
    } else {
        if (!data.WriteBool(false)) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "flag write fail");
            return ERR_INVALID_OPERATION;
        }
    }
    int32_t ret = SendTransactCmd(IAbilityManagerCollaborator::NOTIFY_LOAD_ABILITY_BY_SCB, data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "request error:%{public}d", ret);
        return ret;
    }
    return NO_ERROR;
}

int32_t AbilityManagerCollaboratorProxy::NotifyMoveMissionToBackground(int32_t missionId, int32_t userId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);

    if (!data.WriteInterfaceToken(AbilityManagerCollaboratorProxy::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write token fail");
        return ERR_INVALID_OPERATION;
    }
    if (!data.WriteInt32(missionId)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "missionId write fail");
        return ERR_INVALID_OPERATION;
    }
    if (!data.WriteInt32(userId)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "userId write fail");
        return ERR_INVALID_OPERATION;
    }
    int32_t ret = SendTransactCmd(
        IAbilityManagerCollaborator::NOTIFY_MOVE_MISSION_TO_BACKGROUND, data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "request error:%{public}d", ret);
        return ret;
    }
    return NO_ERROR;
}

int32_t AbilityManagerCollaboratorProxy::NotifyPreloadAbility(const std::string &bundleName)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);

    if (!data.WriteInterfaceToken(AbilityManagerCollaboratorProxy::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write token fail");
        return ERR_INVALID_OPERATION;
    }
    if (!data.WriteString16(Str8ToStr16(bundleName))) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "bundleName write fail");
        return ERR_INVALID_OPERATION;
    }
    auto remote = Remote();
    if (!remote) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null remote");
        return ERR_INVALID_OPERATION;
    }
    int32_t ret = remote->SendRequest(
        IAbilityManagerCollaborator::NOTIFY_PRELOAD_ABILITY, data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "request error:%{public}d", ret);
        return ret;
    }
    return NO_ERROR;
}

int32_t AbilityManagerCollaboratorProxy::NotifyMoveMissionToForeground(int32_t missionId, int32_t userId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);

    if (!data.WriteInterfaceToken(AbilityManagerCollaboratorProxy::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write token fail");
        return ERR_INVALID_OPERATION;
    }
    if (!data.WriteInt32(missionId)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "missionId write fail");
        return ERR_INVALID_OPERATION;
    }
    if (!data.WriteInt32(userId)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "userId write fail");
        return ERR_INVALID_OPERATION;
    }
    int32_t ret = SendTransactCmd(
        IAbilityManagerCollaborator::NOTIFY_MOVE_MISSION_TO_FOREGROUND, data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "request error:%{public}d", ret);
        return ret;
    }
    return NO_ERROR;
}

int32_t AbilityManagerCollaboratorProxy::NotifyTerminateMission(int32_t missionId, int32_t userId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);

    if (!data.WriteInterfaceToken(AbilityManagerCollaboratorProxy::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write token fail");
        return ERR_INVALID_OPERATION;
    }
    if (!data.WriteInt32(missionId)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "missionId write fail");
        return ERR_INVALID_OPERATION;
    }
    if (!data.WriteInt32(userId)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "userId write fail");
        return ERR_INVALID_OPERATION;
    }
    int32_t ret = SendTransactCmd(
        IAbilityManagerCollaborator::NOTIFY_TERMINATE_MISSION, data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "request error:%{public}d", ret);
        return ret;
    }
    return NO_ERROR;
}

int32_t AbilityManagerCollaboratorProxy::NotifyClearMission(int32_t missionId, int32_t userId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    if (!data.WriteInterfaceToken(AbilityManagerCollaboratorProxy::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write token fail");
        return ERR_INVALID_OPERATION;
    }
    if (!data.WriteInt32(missionId)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "missionId write fail");
        return ERR_INVALID_OPERATION;
    }
    if (!data.WriteInt32(userId)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "userId write fail");
        return ERR_INVALID_OPERATION;
    }
    int32_t ret = SendTransactCmd(
        IAbilityManagerCollaborator::NOTIFY_CLEAR_MISSION, data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "request error:%{public}d", ret);
        return ret;
    }
    return NO_ERROR;
}

int32_t AbilityManagerCollaboratorProxy::NotifyRemoveShellProcess(int32_t pid, int32_t type, const std::string &reason)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);

    if (!data.WriteInterfaceToken(AbilityManagerCollaboratorProxy::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write token fail");
        return ERR_INVALID_OPERATION;
    }
    if (!data.WriteInt32(pid)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "pid write fail");
        return ERR_INVALID_OPERATION;
    }
    if (!data.WriteInt32(type)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "type write fail");
        return ERR_INVALID_OPERATION;
    }
    if (!data.WriteString16(Str8ToStr16(reason))) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "reason write fail");
        return ERR_INVALID_OPERATION;
    }
    int32_t ret = SendTransactCmd(
        IAbilityManagerCollaborator::NOTIFY_REMOVE_SHELL_PROCESS, data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "request error:%{public}d", ret);
        return ret;
    }
    return NO_ERROR;
}

void AbilityManagerCollaboratorProxy::UpdateMissionInfo(sptr<SessionInfo> &sessionInfo)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(AbilityManagerCollaboratorProxy::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write token fail");
        return;
    }

    if (sessionInfo) {
        AAFwk::ExtendMaxIpcCapacityForInnerWant(data);
        if (!data.WriteBool(true) || !data.WriteParcelable(sessionInfo)) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "flag and sessionInfo write fail");
            return;
        }
    } else {
        if (!data.WriteBool(false)) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "flag write fail");
            return;
        }
    }

    int32_t ret = SendTransactCmd(IAbilityManagerCollaborator::UPDATE_MISSION_INFO_BY_SCB, data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "request error:%{public}d", ret);
        return;
    }

    sessionInfo = reply.ReadParcelable<SessionInfo>();
    return;
}

int32_t AbilityManagerCollaboratorProxy::CheckCallAbilityPermission(const Want &want)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    AAFwk::ExtendMaxIpcCapacityForInnerWant(data);

    if (!data.WriteInterfaceToken(AbilityManagerCollaboratorProxy::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write token fail");
        return ERR_INVALID_OPERATION;
    }

    if (!data.WriteParcelable(&want)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "want write fail");
        return ERR_INVALID_OPERATION;
    }
    int32_t ret = SendTransactCmd(IAbilityManagerCollaborator::CHECK_CALL_ABILITY_PERMISSION,
        data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "request error:%{public}d", ret);
        return ret;
    }
    return reply.ReadInt32();
}

bool AbilityManagerCollaboratorProxy::UpdateConfiguration(const AppExecFwk::Configuration &config, int32_t userId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(AbilityManagerCollaboratorProxy::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write token fail");
        return false;
    }
    if (!data.WriteParcelable(&config)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write config fail");
        return false;
    }
    if (!data.WriteInt32(userId)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write usr fail");
        return false;
    }
    auto error = SendTransactCmd(IAbilityManagerCollaborator::UPDATE_CONFIGURATION, data, reply, option);
    if (error != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "request error:%{public}d", error);
        return true;
    }
    return true;
}

int32_t AbilityManagerCollaboratorProxy::OpenFile(const Uri& uri, uint32_t flag, uint32_t tokenId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(AbilityManagerCollaboratorProxy::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write token fail");
        return -1;
    }
    if (!data.WriteParcelable(&uri)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write uri fail");
        return -1;
    }
    if (!data.WriteUint32(flag)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write flag fail");
        return -1;
    }
    if (!data.WriteUint32(tokenId)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write tokenId fail");
        return -1;
    }

    int32_t ret = SendTransactCmd(IAbilityManagerCollaborator::OPEN_FILE, data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "request error:%{public}d", ret);
        return -1;
    }
    return reply.ReadFileDescriptor();
}

int32_t AbilityManagerCollaboratorProxy::GrantUriPermission(const std::vector<std::string> &uriVec, uint32_t flag,
    uint32_t targetTokenId, const std::string &targetBundleName)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(AbilityManagerCollaboratorProxy::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write token fail");
        return ERR_INVALID_OPERATION;
    }
    if (!data.WriteStringVector(uriVec)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write uriVec fail");
        return ERR_INVALID_OPERATION;
    }
    if (!data.WriteUint32(flag)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write flag fail");
        return ERR_INVALID_OPERATION;
    }
    if (!data.WriteUint32(targetTokenId)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write targetTokenId fail");
        return ERR_INVALID_OPERATION;
    }
    if (!data.WriteString(targetBundleName)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write targetBundleName fail");
        return ERR_INVALID_OPERATION;
    }

    int32_t ret = SendTransactCmd(IAbilityManagerCollaborator::GRANT_URI_PERMISSION, data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "request error:%{public}d", ret);
        return ERR_INVALID_OPERATION;
    }
    return reply.ReadInt32();
}

int32_t AbilityManagerCollaboratorProxy::RevokeUriPermission(uint32_t tokenId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(AbilityManagerCollaboratorProxy::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write token fail");
        return ERR_INVALID_OPERATION;
    }
    if (!data.WriteUint32(tokenId)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write tokenId fail");
        return ERR_INVALID_OPERATION;
    }

    int32_t ret = SendTransactCmd(IAbilityManagerCollaborator::REVOKE_URI_PERMISSION, data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "request error:%{public}d", ret);
        return ERR_INVALID_OPERATION;
    }
    return reply.ReadInt32();
}

int32_t AbilityManagerCollaboratorProxy::VerifyUriPermission(const std::vector<std::string> &uriVec, uint32_t flag,
    uint32_t tokenId, std::vector<bool> &checkResults)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(AbilityManagerCollaboratorProxy::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write token fail");
        return ERR_INVALID_OPERATION;
    }
    if (!data.WriteStringVector(uriVec)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write uriVec fail");
        return ERR_INVALID_OPERATION;
    }
    if (!data.WriteUint32(flag)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write flag fail");
        return ERR_INVALID_OPERATION;
    }
    if (!data.WriteUint32(tokenId)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write tokenId fail");
        return ERR_INVALID_OPERATION;
    }

    int32_t ret = SendTransactCmd(IAbilityManagerCollaborator::VERIFY_URI_PERMISSION, data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "request error:%{public}d", ret);
        return ERR_INVALID_OPERATION;
    }
    ret = reply.ReadInt32();
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "VerifyUriPermission failed:%{public}d", ret);
        return ret;
    }
    if (!reply.ReadBoolVector(&checkResults)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "read checkResults fail");
        return ERR_INVALID_OPERATION;
    }
    return ERR_OK;
}

void AbilityManagerCollaboratorProxy::NotifyMissionBindPid(int32_t missionId, int32_t pid, int32_t userId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(AbilityManagerCollaboratorProxy::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write token fail");
        return;
    }
    if (!data.WriteInt32(missionId)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write missionId fail");
        return;
    }
    if (!data.WriteInt32(pid)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write pid fail");
        return;
    }
    if (!data.WriteInt32(userId)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "userId write fail");
        return;
    }
    auto error = SendTransactCmd(IAbilityManagerCollaborator::NOTIFY_MISSION_BIND_PID, data, reply, option);
    if (error != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "request error:%{public}d", error);
    }
}

int32_t AbilityManagerCollaboratorProxy::CheckStaticCfgPermission(const Want &want, bool isImplicit)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    AAFwk::ExtendMaxIpcCapacityForInnerWant(data);
    if (!data.WriteInterfaceToken(AbilityManagerCollaboratorProxy::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write token fail");
        return ERR_INVALID_OPERATION;
    }
    if (!data.WriteParcelable(&want)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "want write fail");
        return ERR_INVALID_OPERATION;
    }
    if (!data.WriteBool(isImplicit)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "isImplicit write failed.");
        return ERR_INVALID_OPERATION;
    }
    int32_t ret = SendTransactCmd(IAbilityManagerCollaborator::CHECK_STATIC_CFG_PERMISSION,
        data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "request error:%{public}d", ret);
        return ret;
    }
    return reply.ReadInt32();
}

int32_t AbilityManagerCollaboratorProxy::UpdateCallerIfNeed(Want &want)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    AAFwk::ExtendMaxIpcCapacityForInnerWant(data);
    if (!data.WriteInterfaceToken(AbilityManagerCollaboratorProxy::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write token failed");
        return ERR_INVALID_OPERATION;
    }
    if (!data.WriteParcelable(&want)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write want failed");
        return ERR_INVALID_OPERATION;
    }
    int32_t ret = SendTransactCmd(IAbilityManagerCollaborator::UPDATE_CALLER_IF_NEED,
        data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "update caller error:%{public}d", ret);
        return ret;
    }
    ret = reply.ReadInt32();
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "update caller failed:%{public}d", ret);
        return ERR_INVALID_OPERATION;
    }
    std::unique_ptr<Want> wantInfo(reply.ReadParcelable<Want>());
    if (!wantInfo) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "read want failed");
        return ERR_INVALID_OPERATION;
    }
    want = *wantInfo;
    return NO_ERROR;
}

int32_t AbilityManagerCollaboratorProxy::RemoveCallerIfNeed(Want &want)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    AAFwk::ExtendMaxIpcCapacityForInnerWant(data);
    if (!data.WriteInterfaceToken(AbilityManagerCollaboratorProxy::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write token failed");
        return ERR_INVALID_OPERATION;
    }
    if (!data.WriteParcelable(&want)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write want failed");
        return ERR_INVALID_OPERATION;
    }
    int32_t ret = SendTransactCmd(IAbilityManagerCollaborator::REMOVE_CALLER_IF_NEED,
        data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "update caller error:%{public}d", ret);
        return ret;
    }
    ret = reply.ReadInt32();
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "update caller failed:%{public}d", ret);
        return ERR_INVALID_OPERATION;
    }
    std::unique_ptr<Want> wantInfo(reply.ReadParcelable<Want>());
    if (!wantInfo) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "read want failed");
        return ERR_INVALID_OPERATION;
    }
    want = *wantInfo;
    return NO_ERROR;
}

int32_t AbilityManagerCollaboratorProxy::NotifyKillProcesses(const std::string &bundleName, int32_t userId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(AbilityManagerCollaboratorProxy::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write token fail");
        return ERR_INVALID_OPERATION;
    }
    if (!data.WriteString16(Str8ToStr16(bundleName))) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "bundleName write fail");
        return ERR_INVALID_OPERATION;
    }
    if (!data.WriteInt32(userId)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "userId write fail");
        return ERR_INVALID_OPERATION;
    }
    auto remote = Remote();
    if (!remote) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null remote");
        return ERR_INVALID_OPERATION;
    }
    int32_t ret = remote->SendRequest(
        IAbilityManagerCollaborator::NOTIFY_KILL_PROCESSES, data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "request error:%{public}d", ret);
        return ret;
    }
    return NO_ERROR;
}

int32_t AbilityManagerCollaboratorProxy::UpdateTargetIfNeed(Want &want)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    AAFwk::ExtendMaxIpcCapacityForInnerWant(data);
    if (!data.WriteInterfaceToken(AbilityManagerCollaboratorProxy::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write token failed");
        return ERR_INVALID_OPERATION;
    }
    if (!data.WriteParcelable(&want)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write want failed");
        return ERR_INVALID_OPERATION;
    }
    int32_t ret = SendTransactCmd(IAbilityManagerCollaborator::UPDATE_TARGET_IF_NEED,
        data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "update target error:%{public}d", ret);
        return ret;
    }
    ret = reply.ReadInt32();
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "update target failed:%{public}d", ret);
        return ERR_INVALID_OPERATION;
    }
    std::unique_ptr<Want> wantInfo(reply.ReadParcelable<Want>());
    if (!wantInfo) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "read want failed");
        return ERR_INVALID_OPERATION;
    }
    want = *wantInfo;
    return NO_ERROR;
}

int32_t AbilityManagerCollaboratorProxy::NotifyGrantUriPermissionStart(const std::vector<std::string> &uris,
    uint32_t flag, int32_t userId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(AbilityManagerCollaboratorProxy::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write token failed");
        return ERR_INVALID_OPERATION;
    }
    if (!data.WriteStringVector(uris)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write uris failed");
        return ERR_INVALID_OPERATION;
    }
    if (!data.WriteUint32(flag)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write flag failed");
        return ERR_INVALID_OPERATION;
    }
    if (!data.WriteInt32(userId)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write userId failed");
        return ERR_INVALID_OPERATION;
    }

    auto ret = SendTransactCmd(IAbilityManagerCollaborator::NOTIFY_GRANT_URI_PERMISSION_START, data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "request error:%{public}d", ret);
        return ERR_INVALID_OPERATION;
    }
    return reply.ReadInt32();
}

int32_t AbilityManagerCollaboratorProxy::NotifyGrantUriPermissionEnd(const std::vector<std::string> &uris,
    uint32_t flag, int32_t userId, const std::vector<bool> &checkResults)
{
    if (checkResults.size() != uris.size() || checkResults.size() > MAX_URI_COUNT) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Invalid uris or checkResults");
        return ERR_INVALID_OPERATION;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(AbilityManagerCollaboratorProxy::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write token failed");
        return ERR_INVALID_OPERATION;
    }
    if (!data.WriteStringVector(uris)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write uris failed");
        return ERR_INVALID_OPERATION;
    }
    if (!data.WriteUint32(flag)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write flag failed");
        return ERR_INVALID_OPERATION;
    }
    if (!data.WriteInt32(userId)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write userId failed");
        return ERR_INVALID_OPERATION;
    }
    if (!data.WriteBoolVector(checkResults)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write checkResults failed");
        return ERR_INVALID_OPERATION;
    }
    int32_t ret = SendTransactCmd(IAbilityManagerCollaborator::NOTIFY_GRANT_URI_PERMISSION_END, data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "request error:%{public}d", ret);
        return ERR_INVALID_OPERATION;
    }
    return reply.ReadInt32();
}

int32_t AbilityManagerCollaboratorProxy::SendTransactCmd(uint32_t code, MessageParcel &data,
    MessageParcel &reply, MessageOption &option)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null remote");
        return ERR_NULL_OBJECT;
    }

    return remote->SendRequest(code, data, reply, option);
}
}   // namespace AAFwk
}   // namespace OHOS
