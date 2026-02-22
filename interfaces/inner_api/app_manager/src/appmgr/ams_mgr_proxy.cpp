/*
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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

#include "ams_mgr_proxy.h"

#include "ability_manager_errors.h"
#include "appexecfwk_errors.h"
#include "app_utils.h"
#include "freeze_util.h"
#include "hilog_tag_wrapper.h"
#include "ipc_types.h"
#include "iremote_object.h"
#include "param.h"
#include "parcel_util.h"
#include "string_ex.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
constexpr int32_t MAX_APP_DEBUG_COUNT = 100;
}
AmsMgrProxy::AmsMgrProxy(const sptr<IRemoteObject> &impl) : IRemoteProxy<IAmsMgr>(impl)
{}

bool AmsMgrProxy::IsProcessContainsOnlyUIAbility(const pid_t pid)
{
    TAG_LOGD(AAFwkTag::APPMGR, "IsProcessContainsOnlyUIAbility start");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        return false;
    }
    if (!data.WriteInt32(static_cast<int32_t>(pid))) {
        TAG_LOGE(AAFwkTag::APPMGR, "write pid failed");
        return false;
    }
    int32_t ret =
        SendTransactCmd(static_cast<uint32_t>(
            IAmsMgr::Message::IS_PROCESS_CONTAINS_ONLY_UI_EXTENSION), data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest err: %{public}d", ret);
        return false;
    }
    TAG_LOGD(AAFwkTag::APPMGR, "IsProcessContainsOnlyUIAbility end");
    return reply.ReadBool();
}

bool AmsMgrProxy::WriteInterfaceToken(MessageParcel &data)
{
    if (AAFwk::AppUtils::GetInstance().IsForbidStart()) {
        TAG_LOGW(AAFwkTag::APPMGR, "forbid start: WriteInterfaceToken");
        return false;
    }
    if (!data.WriteInterfaceToken(AmsMgrProxy::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::APPMGR, "write token failed");
        return false;
    }
    return true;
}

void AmsMgrProxy::LoadAbility(const std::shared_ptr<AbilityInfo> &abilityInfo,
    const std::shared_ptr<ApplicationInfo> &appInfo,
    const std::shared_ptr<AAFwk::Want> &want, std::shared_ptr<AbilityRuntime::LoadParam> loadParam)
{
    TAG_LOGD(AAFwkTag::APPMGR, "start");
    if (!abilityInfo || !appInfo) {
        TAG_LOGE(AAFwkTag::APPMGR, "param error");
        return;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }
    if (!data.WriteParcelable(abilityInfo.get())) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write abilityInfo failed");
        return;
    }
    if (!data.WriteParcelable(appInfo.get())) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write abilityInfo failed");
        return;
    }
    if (!data.WriteParcelable(want.get())) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write want failed");
        return;
    }
    if (!data.WriteParcelable(loadParam.get())) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write data loadParam failed");
        return;
    }

    int32_t ret = SendTransactCmd(static_cast<uint32_t>(IAmsMgr::Message::LOAD_ABILITY), data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest err: %{public}d", ret);
        AbilityRuntime::FreezeUtil::GetInstance().AddLifecycleEvent(loadParam->token,
            "AmsMgrProxy::LoadAbility fail, ipc error " + std::to_string(ret));
    }
    TAG_LOGD(AAFwkTag::APPMGR, "end");
}

void AmsMgrProxy::NotifyLoadAbilityFinished(pid_t callingPid, pid_t targetPid, uint64_t callbackId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }
    if (!data.WriteInt32(callingPid)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write callingPid failed");
        return;
    }
    if (!data.WriteInt32(targetPid)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write targetPid failed");
        return;
    }
    if (!data.WriteUint64(callbackId)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write callbackId failed");
        return;
    }

    int32_t ret = SendTransactCmd(static_cast<uint32_t>(IAmsMgr::Message::NOTIFY_LOAD_ABILITY_FINISHED),
        data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest err: %{public}d", ret);
    }
}

void AmsMgrProxy::TerminateAbility(const sptr<IRemoteObject> &token, bool clearMissionFlag)
{
    TAG_LOGD(AAFwkTag::APPMGR, "start");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }
    if (!data.WriteRemoteObject(token.GetRefPtr())) {
        TAG_LOGE(AAFwkTag::APPMGR, "Failed to write token");
        return;
    }
    if (!data.WriteBool(clearMissionFlag)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Failed to write clearMissionFlag");
        return;
    }
    int32_t ret = SendTransactCmd(static_cast<uint32_t>(IAmsMgr::Message::TERMINATE_ABILITY), data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest err: %{public}d", ret);
    }
    TAG_LOGD(AAFwkTag::APPMGR, "end");
}

void AmsMgrProxy::UpdateAbilityState(const sptr<IRemoteObject> &token, const AbilityState state)
{
    TAG_LOGD(AAFwkTag::APPMGR, "start");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Failed to write data");
        return;
    }
    if (!data.WriteRemoteObject(token.GetRefPtr())) {
        TAG_LOGE(AAFwkTag::APPMGR, "Failed to write token");
        return;
    }
    if (!data.WriteInt32(static_cast<int32_t>(state))) {
        TAG_LOGE(AAFwkTag::APPMGR, "Failed to write token");
        return;
    }
    int32_t ret =
        SendTransactCmd(static_cast<uint32_t>(IAmsMgr::Message::UPDATE_ABILITY_STATE), data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest err: %{public}d", ret);
    }
    TAG_LOGD(AAFwkTag::APPMGR, "end");
}

void AmsMgrProxy::UpdateExtensionState(const sptr<IRemoteObject> &token, const ExtensionState state)
{
    TAG_LOGD(AAFwkTag::APPMGR, "UpdateExtensionState begin");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }
    if (!data.WriteRemoteObject(token.GetRefPtr())) {
        TAG_LOGE(AAFwkTag::APPMGR, "Failed to write token");
        return;
    }
    if (!data.WriteInt32(static_cast<int32_t>(state))) {
        TAG_LOGE(AAFwkTag::APPMGR, "Failed to write state");
        return;
    }
    int32_t ret =
        SendTransactCmd(static_cast<uint32_t>(IAmsMgr::Message::UPDATE_EXTENSION_STATE), data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest err: %{public}d", ret);
    }
    TAG_LOGD(AAFwkTag::APPMGR, "end");
}

void AmsMgrProxy::RegisterAppStateCallback(const sptr<IAppStateCallback> &callback)
{
    TAG_LOGD(AAFwkTag::APPMGR, "begin");
    if (!callback) {
        TAG_LOGE(AAFwkTag::APPMGR, "callback null");
        return;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }

    if (callback->AsObject()) {
        if (!data.WriteBool(true) || !data.WriteRemoteObject(callback->AsObject())) {
            TAG_LOGE(AAFwkTag::APPMGR, "Failed to write flag and callback");
            return;
        }
    } else {
        if (!data.WriteBool(false)) {
            TAG_LOGE(AAFwkTag::APPMGR, "Failed to write flag");
            return;
        }
    }

    int32_t ret = SendTransactCmd(
        static_cast<uint32_t>(IAmsMgr::Message::REGISTER_APP_STATE_CALLBACK), data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest err: %{public}d", ret);
    }
    TAG_LOGD(AAFwkTag::APPMGR, "end");
}

void AmsMgrProxy::KillProcessByAbilityToken(const sptr<IRemoteObject> &token)
{
    TAG_LOGI(AAFwkTag::APPMGR, "start");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }
    if (!data.WriteRemoteObject(token.GetRefPtr())) {
        TAG_LOGE(AAFwkTag::APPMGR, "Failed to write token");
        return;
    }
    int32_t ret = SendTransactCmd(static_cast<uint32_t>(IAmsMgr::Message::KILL_PEOCESS_BY_ABILITY_TOKEN),
        data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest err: %{public}d", ret);
    }
    TAG_LOGD(AAFwkTag::APPMGR, "end");
}

void AmsMgrProxy::KillProcessesByUserId(int32_t userId, bool isNeedSendAppSpawnMsg,
    sptr<AAFwk::IUserCallback> callback)
{
    TAG_LOGI(AAFwkTag::APPMGR, "start");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }
    if (!data.WriteInt32(userId)) {
        TAG_LOGE(AAFwkTag::APPMGR, "parcel WriteInt32 failed");
        return;
    }
    if (!data.WriteBool(isNeedSendAppSpawnMsg)) {
        TAG_LOGE(AAFwkTag::APPMGR, "parcel WriteBool failed");
        return;
    }
    if (callback == nullptr) {
        TAG_LOGD(AAFwkTag::APPMGR, "callback is nullptr");
        data.WriteBool(false);
    } else {
        data.WriteBool(true);
        if (!data.WriteRemoteObject(callback->AsObject())) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "write IUserCallback fail");
            return;
        }
    }
    int32_t ret =
        SendTransactCmd(static_cast<uint32_t>(IAmsMgr::Message::KILL_PROCESSES_BY_USERID), data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest err: %{public}d", ret);
    }
    TAG_LOGD(AAFwkTag::APPMGR, "ending");
}

int32_t AmsMgrProxy::KillProcessesByPids(const std::vector<int32_t> &pids, const std::string &reason,
    bool subProcess, bool isKillPrecedeStart)
{
    TAG_LOGI(AAFwkTag::APPMGR, "start");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write token failed");
        return ERR_INVALID_DATA;
    }
    if (!data.WriteUint32(pids.size())) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write size failed");
        return ERR_FLATTEN_OBJECT;
    }
    for (const auto &pid: pids) {
        if (!data.WriteInt32(pid)) {
            TAG_LOGE(AAFwkTag::APPMGR, "Write pid failed");
            return ERR_FLATTEN_OBJECT;
        }
    }
    if (!data.WriteString(reason)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write reason failed");
        return ERR_FLATTEN_OBJECT;
    }
    if (!data.WriteBool(subProcess)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write subProcess failed");
        return ERR_FLATTEN_OBJECT;
    }
    if (!data.WriteBool(isKillPrecedeStart)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write isKillPrecedeStart failed");
        return ERR_FLATTEN_OBJECT;
    }
    int32_t ret =
        SendTransactCmd(static_cast<uint32_t>(IAmsMgr::Message::KILL_PROCESSES_BY_PIDS), data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest err: %{public}d", ret);
        return ret;
    }
    TAG_LOGD(AAFwkTag::APPMGR, "end");
    return reply.ReadInt32();
}

void AmsMgrProxy::AttachPidToParent(const sptr<IRemoteObject> &token, const sptr<IRemoteObject> &callerToken)
{
    TAG_LOGD(AAFwkTag::APPMGR, "start");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return;
    }
    if (!data.WriteRemoteObject(token)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Failed to write token");
        return;
    }
    if (!data.WriteRemoteObject(callerToken)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Failed to write callerToken");
        return;
    }
    int32_t ret = SendTransactCmd(static_cast<uint32_t>(IAmsMgr::Message::ATTACH_PID_TO_PARENT),
        data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest err: %{public}d", ret);
    }
    TAG_LOGD(AAFwkTag::APPMGR, "end");
}

int32_t AmsMgrProxy::KillProcessWithAccount(
    const std::string &bundleName, const int accountId, const bool clearPageStack, int32_t appIndex)
{
    TAG_LOGI(AAFwkTag::APPMGR, "start");

    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        return ERR_INVALID_DATA;
    }

    if (!data.WriteString(bundleName)) {
        TAG_LOGE(AAFwkTag::APPMGR, "WriteString failed");
        return ERR_FLATTEN_OBJECT;
    }

    if (!data.WriteInt32(accountId)) {
        TAG_LOGE(AAFwkTag::APPMGR, "parcel WriteInt32 failed");
        return ERR_FLATTEN_OBJECT;
    }

    if (!data.WriteBool(clearPageStack)) {
        TAG_LOGE(AAFwkTag::APPMGR, "parcel bool failed");
        return ERR_FLATTEN_OBJECT;
    }

    if (!data.WriteInt32(appIndex)) {
        TAG_LOGE(AAFwkTag::APPMGR, "parcel appIndex failed");
        return ERR_FLATTEN_OBJECT;
    }

    int32_t ret =
        SendTransactCmd(static_cast<uint32_t>(IAmsMgr::Message::KILL_PROCESS_WITH_ACCOUNT), data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest err: %{public}d", ret);
        return ret;
    }

    TAG_LOGD(AAFwkTag::APPMGR, "end");

    return reply.ReadInt32();
}

int32_t AmsMgrProxy::KillProcessesInBatch(const std::vector<int32_t> &pids)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        return ERR_INVALID_DATA;
    }

    if (!data.WriteUint32(pids.size())) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write size failed");
        return ERR_FLATTEN_OBJECT;
    }
    for (const auto &pid: pids) {
        if (!data.WriteInt32(pid)) {
            TAG_LOGE(AAFwkTag::APPMGR, "Write pid failed");
            return ERR_FLATTEN_OBJECT;
        }
    }

    int32_t ret = SendTransactCmd(static_cast<uint32_t>(IAmsMgr::Message::KILL_PROCESSES_IN_BATCH),
        data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest err: %{public}d", ret);
        return ret;
    }

    return reply.ReadInt32();
}

int32_t AmsMgrProxy::KillApplication(const std::string &bundleName, bool clearPageStack, int32_t appIndex,
    const std::string &reason)
{
    TAG_LOGI(AAFwkTag::APPMGR, "start");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        return ERR_INVALID_DATA;
    }

    if (!data.WriteString(bundleName)) {
        TAG_LOGE(AAFwkTag::APPMGR, "WriteString failed");
        return ERR_FLATTEN_OBJECT;
    }

    if (!data.WriteBool(clearPageStack)) {
        TAG_LOGE(AAFwkTag::APPMGR, "parcel bool failed");
        return ERR_FLATTEN_OBJECT;
    }

    if (!data.WriteInt32(appIndex)) {
        TAG_LOGE(AAFwkTag::APPMGR, "parcel appIndex failed");
        return ERR_FLATTEN_OBJECT;
    }
    
    if (!data.WriteString(reason)) {
        TAG_LOGE(AAFwkTag::APPMGR, "WriteString reason failed");
        return ERR_FLATTEN_OBJECT;
    }

    int32_t ret =
        SendTransactCmd(static_cast<uint32_t>(IAmsMgr::Message::KILL_APPLICATION), data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest err: %{public}d.", ret);
        return ret;
    }
    return reply.ReadInt32();
}

int32_t AmsMgrProxy::ForceKillApplication(const std::string &bundleName,
    const int userId, const int appIndex)
{
    TAG_LOGI(AAFwkTag::APPMGR, "start");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        return ERR_INVALID_DATA;
    }

    if (!data.WriteString(bundleName)) {
        TAG_LOGE(AAFwkTag::APPMGR, "parcel bundleName failed.");
        return ERR_FLATTEN_OBJECT;
    }

    if (!data.WriteInt32(userId)) {
        TAG_LOGE(AAFwkTag::APPMGR, "parcel userId failed");
        return ERR_FLATTEN_OBJECT;
    }

    if (!data.WriteInt32(appIndex)) {
        TAG_LOGE(AAFwkTag::APPMGR, "parcel appIndex failed");
        return ERR_FLATTEN_OBJECT;
    }

    int32_t ret =
        SendTransactCmd(static_cast<uint32_t>(IAmsMgr::Message::FORCE_KILL_APPLICATION), data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest err: %{public}d.", ret);
        return ret;
    }
    return reply.ReadInt32();
}

int32_t AmsMgrProxy::KillProcessesByAccessTokenId(const uint32_t accessTokenId)
{
    TAG_LOGI(AAFwkTag::APPMGR, "start");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        return ERR_INVALID_DATA;
    }

    if (!data.WriteInt32(accessTokenId)) {
        TAG_LOGE(AAFwkTag::APPMGR, "parcel accessTokenId failed");
        return ERR_FLATTEN_OBJECT;
    }

    int32_t ret =
        SendTransactCmd(static_cast<uint32_t>(IAmsMgr::Message::FORCE_KILL_APPLICATION_BY_ACCESS_TOKEN_ID),
            data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest err: %{public}d.", ret);
        return ret;
    }
    return reply.ReadInt32();
}

int32_t AmsMgrProxy::UpdateApplicationInfoInstalled(const std::string &bundleName, const int uid,
    const std::string &moduleName, bool isPlugin)
{
    TAG_LOGD(AAFwkTag::APPMGR, "start.");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        return ERR_INVALID_DATA;
    }
    if (!data.WriteString(bundleName)) {
        TAG_LOGE(AAFwkTag::APPMGR, "WriteString failed");
        return ERR_FLATTEN_OBJECT;
    }
    if (!data.WriteInt32(uid)) {
        TAG_LOGE(AAFwkTag::APPMGR, "uid write failed.");
        return ERR_FLATTEN_OBJECT;
    }
    if (!data.WriteString(moduleName)) {
        TAG_LOGE(AAFwkTag::APPMGR, "moduleName WriteString failed");
        return ERR_FLATTEN_OBJECT;
    }
    if (!data.WriteBool(isPlugin)) {
        TAG_LOGE(AAFwkTag::APPMGR, "isPlugin Write failed");
        return ERR_FLATTEN_OBJECT;
    }
    int32_t ret =
        SendTransactCmd(static_cast<uint32_t>(IAmsMgr::Message::UPDATE_APPLICATION_INFO_INSTALLED),
        data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest err: %{public}d", ret);
        return ret;
    }
    return reply.ReadInt32();
}

int32_t AmsMgrProxy::KillApplicationByUid(const std::string &bundleName, const int uid,
    const std::string& reason)
{
    TAG_LOGI(AAFwkTag::APPMGR, "start");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        return ERR_INVALID_DATA;
    }
    if (!data.WriteString(bundleName)) {
        TAG_LOGE(AAFwkTag::APPMGR, "failed to write bundle name");
        return ERR_FLATTEN_OBJECT;
    }
    if (!data.WriteInt32(uid)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Failed to write uid");
        return ERR_FLATTEN_OBJECT;
    }
    if (!data.WriteString(reason)) {
        TAG_LOGE(AAFwkTag::APPMGR, "failedto write reason");
        return ERR_FLATTEN_OBJECT;
    }
    int32_t ret =
        SendTransactCmd(static_cast<uint32_t>(IAmsMgr::Message::KILL_APPLICATION_BYUID), data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest err: %{public}d", ret);
        return ret;
    }
    return reply.ReadInt32();
}

int32_t AmsMgrProxy::NotifyUninstallOrUpgradeApp(const std::string &bundleName, int32_t uid, bool isUpgrade)
{
    TAG_LOGI(AAFwkTag::APPMGR, "start");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        return ERR_INVALID_DATA;
    }
    if (!data.WriteString(bundleName)) {
        TAG_LOGE(AAFwkTag::APPMGR, "failed to write bundle name");
        return ERR_FLATTEN_OBJECT;
    }
    if (!data.WriteInt32(uid)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Failed to write uid");
        return ERR_FLATTEN_OBJECT;
    }
    if (!data.WriteBool(isUpgrade)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Failed to write isUpgradeFlag");
        return ERR_FLATTEN_OBJECT;
    }
    int32_t ret =
        SendTransactCmd(static_cast<uint32_t>(IAmsMgr::Message::NOTIFY_UNINSTALL_OR_UPGRADE_APP), data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::APPMGR, "SendRequest err: %{public}d", ret);
        return ret;
    }
    return reply.ReadInt32();
}

void AmsMgrProxy::NotifyUninstallOrUpgradeAppEnd(int32_t uid)
{
    TAG_LOGI(AAFwkTag::APPMGR, "start");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }
    if (!data.WriteInt32(uid)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Failed to write uid");
        return;
    }
    int32_t ret = SendTransactCmd(
        static_cast<uint32_t>(IAmsMgr::Message::NOTIFY_UNINSTALL_OR_UPGRADE_APP_END), data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::APPMGR, "SendRequest err: %{public}d", ret);
    }
    TAG_LOGD(AAFwkTag::APPMGR, "end");
}


int32_t AmsMgrProxy::KillApplicationSelf(const bool clearPageStack, const std::string& reason)
{
    TAG_LOGI(AAFwkTag::APPMGR, "call");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        return ERR_INVALID_DATA;
    }

    if (!data.WriteBool(clearPageStack)) {
        TAG_LOGE(AAFwkTag::APPMGR, "parcel bool failed");
        return ERR_FLATTEN_OBJECT;
    }

    if (!data.WriteString(reason)) {
        TAG_LOGE(AAFwkTag::APPMGR, "failed to write reason");
        return ERR_FLATTEN_OBJECT;
    }

    int32_t ret =
        SendTransactCmd(static_cast<uint32_t>(IAmsMgr::Message::KILL_APPLICATION_SELF), data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::APPMGR, "SendRequest err: %{public}d", ret);
        return ret;
    }
    return reply.ReadInt32();
}

void AmsMgrProxy::AbilityAttachTimeOut(const sptr<IRemoteObject> &token)
{
    TAG_LOGD(AAFwkTag::APPMGR, "beginning");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }
    if (!data.WriteRemoteObject(token.GetRefPtr())) {
        TAG_LOGE(AAFwkTag::APPMGR, "Failed to write token");
        return;
    }
    int32_t ret =
        SendTransactCmd(static_cast<uint32_t>(IAmsMgr::Message::ABILITY_ATTACH_TIMEOUT), data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest err: %{public}d", ret);
    }
    TAG_LOGD(AAFwkTag::APPMGR, "end");
}

void AmsMgrProxy::PrepareTerminate(const sptr<IRemoteObject> &token, bool clearMissionFlag)
{
    TAG_LOGD(AAFwkTag::APPMGR, "start");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }
    if (!data.WriteRemoteObject(token.GetRefPtr())) {
        TAG_LOGE(AAFwkTag::APPMGR, "Failed to write token");
        return;
    }
    if (!data.WriteBool(clearMissionFlag)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Failed to write clearMissionFlag");
        return;
    }
    int32_t ret =
        SendTransactCmd(static_cast<uint32_t>(IAmsMgr::Message::PREPARE_TERMINATE_ABILITY),
            data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest err: %{public}d", ret);
    }
    TAG_LOGD(AAFwkTag::APPMGR, "end");
}

void AmsMgrProxy::GetRunningProcessInfoByToken(
    const sptr<IRemoteObject> &token, AppExecFwk::RunningProcessInfo &info)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }

    if (!data.WriteRemoteObject(token.GetRefPtr())) {
        return;
    }

    auto ret = SendTransactCmd(
        static_cast<uint32_t>(IAmsMgr::Message::GET_RUNNING_PROCESS_INFO_BY_TOKEN), data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest err: %{public}d", ret);
        return;
    }

    std::unique_ptr<AppExecFwk::RunningProcessInfo> processInfo(reply.ReadParcelable<AppExecFwk::RunningProcessInfo>());
    if (processInfo == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "recv process info faild");
        return;
    }

    info = *processInfo;
}

void AmsMgrProxy::SetAbilityForegroundingFlagToAppRecord(const pid_t pid)
{
    TAG_LOGD(AAFwkTag::APPMGR, "calling");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }

    if (!data.WriteInt32(static_cast<int32_t>(pid))) {
        TAG_LOGE(AAFwkTag::APPMGR, "parcel WriteInt32 failed");
        return;
    }

    auto ret = SendTransactCmd(
        static_cast<uint32_t>(IAmsMgr::Message::SET_ABILITY_FOREGROUNDING_FLAG), data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest err: %{public}d", ret);
    }
}

void AmsMgrProxy::PrepareTerminateApp(const pid_t pid, const std::string &moduleName)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "token write error");
        return;
    }
    if (!data.WriteInt32(pid)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write PrepareTerminateApp pid failed.");
        return;
    }
    if (!data.WriteString(moduleName)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write PrepareTerminateApp moduleName failed.");
        return;
    }
    auto ret = SendTransactCmd(
        static_cast<uint32_t>(IAmsMgr::Message::PREPARE_TERMINATE_APP), data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest PrepareTerminateApp err: %{public}d", ret);
        return;
    }
    TAG_LOGD(AAFwkTag::APPMGR, "Get PrepareTerminateApp reply success");
}

void AmsMgrProxy::StartSpecifiedAbility(const AAFwk::Want &want, const AppExecFwk::AbilityInfo &abilityInfo,
    int32_t requestId, const std::string &customProcess)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }

    if (!data.WriteParcelable(&want) || !data.WriteParcelable(&abilityInfo) ||
        !data.WriteInt32(requestId) || !data.WriteString(customProcess)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write data failed.");
        return;
    }

    auto ret = SendTransactCmd(
        static_cast<uint32_t>(IAmsMgr::Message::START_SPECIFIED_ABILITY), data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest err: %{public}d", ret);
    }
}

void AmsMgrProxy::StartSpecifiedProcess(const AAFwk::Want &want, const AppExecFwk::AbilityInfo &abilityInfo,
    int32_t requestId, const std::string &customProcess)

{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write data failed.");
        return;
    }

    if (!data.WriteParcelable(&want) || !data.WriteParcelable(&abilityInfo) ||
        data.WriteInt32(requestId)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write data failed.");
        return;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "Remote is nullptr.");
        return;
    }
    auto ret = remote->SendRequest(
        static_cast<uint32_t>(IAmsMgr::Message::START_SPECIFIED_PROCESS), data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest err: %{public}d", ret);
    }
}

void AmsMgrProxy::RegisterStartSpecifiedAbilityResponse(const sptr<IStartSpecifiedAbilityResponse> &response)
{
    TAG_LOGD(AAFwkTag::APPMGR, "Register multi instances response by proxy.");
    if (!response) {
        TAG_LOGE(AAFwkTag::APPMGR, "response null");
        return;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }
    if (!data.WriteRemoteObject(response->AsObject())) {
        TAG_LOGE(AAFwkTag::APPMGR, "write remote obj failed");
        return;
    }

    int32_t ret = SendTransactCmd(
        static_cast<uint32_t>(IAmsMgr::Message::REGISTER_START_SPECIFIED_ABILITY_RESPONSE), data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest err: %{public}d", ret);
    }
}

int AmsMgrProxy::GetApplicationInfoByProcessID(const int pid, AppExecFwk::ApplicationInfo &application, bool &debug)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "token write error");
        return ERR_FLATTEN_OBJECT;
    }
    if (!data.WriteInt32(pid)) {
        TAG_LOGE(AAFwkTag::APPMGR, "pid write error");
        return ERR_FLATTEN_OBJECT;
    }
    int32_t ret = SendTransactCmd(
        static_cast<uint32_t>(IAmsMgr::Message::GET_APPLICATION_INFO_BY_PROCESS_ID), data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::APPMGR, "send request fail");
        return ret;
    }
    auto result = reply.ReadInt32();
    if (result != NO_ERROR) {
        TAG_LOGE(AAFwkTag::APPMGR, "reply result false");
        return result;
    }
    std::unique_ptr<AppExecFwk::ApplicationInfo> info(reply.ReadParcelable<AppExecFwk::ApplicationInfo>());
    if (!info) {
        TAG_LOGE(AAFwkTag::APPMGR, "readParcelableInfo failed");
        return ERR_NAME_NOT_FOUND;
    }
    application = *info;
    debug = reply.ReadBool();
    TAG_LOGD(AAFwkTag::APPMGR, "get parcelable info success");
    return NO_ERROR;
}

int32_t AmsMgrProxy::NotifyAppMgrRecordExitReason(int32_t pid, int32_t reason, const std::string &exitMsg)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "token write error");
        return IPC_PROXY_ERR;
    }
    if (!data.WriteInt32(pid)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write pid failed");
        return IPC_PROXY_ERR;
    }
    if (!data.WriteInt32(reason)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write reason failed");
        return IPC_PROXY_ERR;
    }
    if (!data.WriteString16(Str8ToStr16(exitMsg))) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write exitMsg failed");
        return IPC_PROXY_ERR;
    }
    int32_t ret = SendTransactCmd(
        static_cast<uint32_t>(IAmsMgr::Message::NOTIFY_APP_MGR_RECORD_EXIT_REASON), data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::APPMGR, "send request fail");
        return ret;
    }
    return reply.ReadInt32();
}

int32_t AmsMgrProxy::NotifyAppMgrRecordExitReasonCompability(
    int32_t pid, int32_t killId, const std::string &killMsg, const std::string &innerMsg)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "token write error");
        return IPC_PROXY_ERR;
    }
    PARCEL_UTIL_WRITE_RET_INT(data, Int32, pid);
    PARCEL_UTIL_WRITE_RET_INT(data, Int32, killId);
    PARCEL_UTIL_WRITE_RET_INT(data, String, killMsg);
    PARCEL_UTIL_WRITE_RET_INT(data, String, innerMsg);
    int32_t ret = SendTransactCmd(
        static_cast<uint32_t>(IAmsMgr::Message::NOTIFY_APP_MGR_RECORD_EXIT_REASON_COMPABILITY), data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::APPMGR, "send request fail");
        return ret;
    }
    return reply.ReadInt32();
}

void AmsMgrProxy::SetEnableStartProcessFlagByUserId(int32_t userId, bool enableStartProcess)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }
    if (!data.WriteInt32(userId)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Failed to write userId");
        return;
    }
    if (!data.WriteBool(enableStartProcess)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Failed to write enableStartProcess");
        return;
    }
    int32_t ret =
        SendTransactCmd(static_cast<uint32_t>(IAmsMgr::Message::ENABLE_START_PROCESS_FLAG_BY_USER_ID),
            data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest err: %{public}d", ret);
    }
}

int32_t AmsMgrProxy::GetBundleNameByPid(const int pid, std::string &bundleName, int32_t &uid)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        return ERR_INVALID_DATA;
    }
    if (!data.WriteInt32(pid)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Failed to write pid");
        return ERR_INVALID_DATA;
    }
    int32_t ret =
        SendTransactCmd(static_cast<uint32_t>(IAmsMgr::Message::Get_BUNDLE_NAME_BY_PID),
            data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest err: %{public}d", ret);
    }
    bundleName = reply.ReadString();
    uid = reply.ReadInt32();
    return NO_ERROR;
}

int32_t AmsMgrProxy::RegisterAppDebugListener(const sptr<IAppDebugListener> &listener)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write token failed");
        return ERR_INVALID_DATA;
    }

    if (listener == nullptr || !data.WriteRemoteObject(listener->AsObject())) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write listener failed");
        return ERR_INVALID_DATA;
    }

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    auto ret = SendTransactCmd(static_cast<uint32_t>(IAmsMgr::Message::REGISTER_APP_DEBUG_LISTENER),
        data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::APPMGR, "Send request err: %{public}d", ret);
        return ret;
    }
    return reply.ReadInt32();
}

int32_t AmsMgrProxy::UnregisterAppDebugListener(const sptr<IAppDebugListener> &listener)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write token failed");
        return ERR_INVALID_DATA;
    }

    if (listener == nullptr || !data.WriteRemoteObject(listener->AsObject())) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write listener failed");
        return ERR_INVALID_DATA;
    }

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    auto ret = SendTransactCmd(static_cast<uint32_t>(IAmsMgr::Message::UNREGISTER_APP_DEBUG_LISTENER),
        data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::APPMGR, "Send request err: %{public}d", ret);
        return ret;
    }
    return reply.ReadInt32();
}

int32_t AmsMgrProxy::AttachAppDebug(const std::string &bundleName, bool isDebugFromLocal)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write token failed");
        return ERR_INVALID_DATA;
    }

    if (bundleName.empty() || !data.WriteString(bundleName)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write bundleName failed");
        return ERR_INVALID_DATA;
    }

    if (!data.WriteBool(isDebugFromLocal)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write isDebugFromLocal failed");
        return ERR_INVALID_DATA;
    }

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    auto ret = SendTransactCmd(static_cast<uint32_t>(IAmsMgr::Message::ATTACH_APP_DEBUG),
        data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::APPMGR, "Send request err: %{public}d", ret);
        return ret;
    }
    return reply.ReadInt32();
}

int32_t AmsMgrProxy::DetachAppDebug(const std::string &bundleName)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write token failed");
        return ERR_INVALID_DATA;
    }

    if (bundleName.empty() || !data.WriteString(bundleName)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write bundleName failed");
        return ERR_INVALID_DATA;
    }

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    auto ret = SendTransactCmd(static_cast<uint32_t>(IAmsMgr::Message::DETACH_APP_DEBUG),
        data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::APPMGR, "Send request err: %{public}d", ret);
        return ret;
    }
    return reply.ReadInt32();
}

void AmsMgrProxy::SetKeepAliveEnableState(const std::string &bundleName, bool enable, int32_t uid)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write token failed");
        return;
    }
    if (bundleName.empty() || !data.WriteString(bundleName)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write bundleName fail");
        return;
    }
    if (!data.WriteBool(enable) || !data.WriteInt32(uid)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write flag or uid fail");
        return;
    }
    MessageParcel reply;
    MessageOption option;
    auto ret = SendTransactCmd(static_cast<uint32_t>(IAmsMgr::Message::SET_KEEP_ALIVE_ENABLE_STATE),
        data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::APPMGR, "Send request err: %{public}d", ret);
    }
}

void AmsMgrProxy::SetKeepAliveDkv(const std::string &bundleName, bool enable, int32_t uid)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write token failed");
        return;
    }
    if (bundleName.empty() || !data.WriteString(bundleName)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write bundleName fail");
        return;
    }
    if (!data.WriteBool(enable) || !data.WriteInt32(uid)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write flag or uid fail");
        return;
    }
    MessageParcel reply;
    MessageOption option;
    auto ret = SendTransactCmd(static_cast<uint32_t>(IAmsMgr::Message::SET_KEEP_ALIVE_DKV),
        data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::APPMGR, "Send request err: %{public}d", ret);
    }
}

void AmsMgrProxy::SetKeepAliveAppService(const std::string &bundleName, bool enable, int32_t uid)
{
    TAG_LOGD(AAFwkTag::APPMGR, "SetKeepAliveAppService called");
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write token failed");
        return;
    }
    if (bundleName.empty() || !data.WriteString(bundleName)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write bundleName fail");
        return;
    }
    if (!data.WriteBool(enable) || !data.WriteInt32(uid)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write flag or uid fail");
        return;
    }
    MessageParcel reply;
    MessageOption option;
    auto ret = SendTransactCmd(static_cast<uint32_t>(IAmsMgr::Message::SET_KEEP_ALIVE_APP_SERVICE),
        data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::APPMGR, "Send request err: %{public}d", ret);
    }
}

int32_t AmsMgrProxy::SetAppWaitingDebug(const std::string &bundleName, bool isPersist)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write token failed");
        return ERR_INVALID_DATA;
    }

    if (bundleName.empty() || !data.WriteString(bundleName)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write bundleName failed");
        return ERR_INVALID_DATA;
    }

    if (!data.WriteBool(isPersist)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write persist flag failed.");
        return ERR_INVALID_DATA;
    }

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    auto ret = SendTransactCmd(static_cast<uint32_t>(IAmsMgr::Message::SET_APP_WAITING_DEBUG), data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::APPMGR, "Send request err: %{public}d", ret);
        return ret;
    }
    return reply.ReadInt32();
}

int32_t AmsMgrProxy::CancelAppWaitingDebug()
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write token failed");
        return ERR_INVALID_DATA;
    }

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    auto ret = SendTransactCmd(static_cast<uint32_t>(IAmsMgr::Message::CANCEL_APP_WAITING_DEBUG), data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::APPMGR, "Send request err: %{public}d", ret);
        return ret;
    }
    return reply.ReadInt32();
}

int32_t AmsMgrProxy::GetWaitingDebugApp(std::vector<std::string> &debugInfoList)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write token failed");
        return ERR_INVALID_DATA;
    }

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    auto ret = SendTransactCmd(static_cast<uint32_t>(IAmsMgr::Message::GET_WAITING_DEBUG_APP), data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::APPMGR, "Send request err: %{public}d", ret);
        return ret;
    }

    auto resultCode = reply.ReadInt32();
    if (resultCode != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "Reply err: %{public}d", resultCode);
        return resultCode;
    }

    auto infoSize = reply.ReadInt32();
    if (infoSize > MAX_APP_DEBUG_COUNT) {
        TAG_LOGE(AAFwkTag::APPMGR, "Max app debug count: %{public}d", infoSize);
        return ERR_INVALID_DATA;
    }

    if (!reply.ReadStringVector(&debugInfoList)) {
        TAG_LOGE(AAFwkTag::APPMGR, "ReadStringVector failed");
        return ERR_INVALID_DATA;
    }

    return NO_ERROR;
}

bool AmsMgrProxy::IsWaitingDebugApp(const std::string &bundleName)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write token failed");
        return false;
    }

    if (bundleName.empty() || !data.WriteString(bundleName)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write bundleName failed");
        return false;
    }

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    auto ret = SendTransactCmd(static_cast<uint32_t>(IAmsMgr::Message::IS_WAITING_DEBUG_APP), data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::APPMGR, "Send request err: %{public}d", ret);
        return false;
    }
    return reply.ReadBool();
}

void AmsMgrProxy::ClearNonPersistWaitingDebugFlag()
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write token failed");
        return;
    }

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    auto ret = SendTransactCmd(
        static_cast<uint32_t>(IAmsMgr::Message::CLEAR_NON_PERSIST_WAITING_DEBUG_FLAG), data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGW(AAFwkTag::APPMGR, "Send request err: %{public}d", ret);
    }
}

int32_t AmsMgrProxy::RegisterAbilityDebugResponse(const sptr<IAbilityDebugResponse> &response)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write token failed");
        return ERR_INVALID_DATA;
    }

    if (response == nullptr || !data.WriteRemoteObject(response->AsObject())) {
        TAG_LOGE(AAFwkTag::APPMGR, "write remote obj failed");
        return ERR_INVALID_DATA;
    }

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    int32_t ret = SendTransactCmd(
        static_cast<uint32_t>(IAmsMgr::Message::REGISTER_ABILITY_DEBUG_RESPONSE), data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::APPMGR, "SendRequest err: %{public}d", ret);
        return ret;
    }
    return reply.ReadInt32();
}

bool AmsMgrProxy::IsAttachDebug(const std::string &bundleName)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write token failed");
        return false;
    }

    if (bundleName.empty() || !data.WriteString(bundleName)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write bundleName fail");
        return false;
    }

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    auto ret = SendTransactCmd(static_cast<uint32_t>(IAmsMgr::Message::IS_ATTACH_DEBUG),
        data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::APPMGR, "Send request err: %{public}d", ret);
        return false;
    }
    return reply.ReadBool();
}

void AmsMgrProxy::ClearProcessByToken(sptr<IRemoteObject> token)
{
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "WriteInterfaceToken failed");
        return;
    }
    if (!data.WriteRemoteObject(token)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Failed to write token");
        return;
    }
    MessageParcel reply;
    MessageOption option;
    auto ret = SendTransactCmd(static_cast<uint32_t>(IAmsMgr::Message::CLEAR_PROCESS_BY_TOKEN), data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest err: %{public}d", ret);
    }
}

bool AmsMgrProxy::IsMemorySizeSufficient()
{
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write token failed");
        return true;
    }

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    auto ret = SendTransactCmd(static_cast<uint32_t>(IAmsMgr::Message::IS_MEMORY_SIZE_SUFFICIENT), data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::APPMGR, "Send request err: %{public}d", ret);
        return true;
    }
    return reply.ReadBool();
}

bool AmsMgrProxy::IsNoRequireBigMemory()
{
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write token failed");
        return true;
    }

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    auto ret = SendTransactCmd(static_cast<uint32_t>(IAmsMgr::Message::IS_NO_REQUIRE_BIG_MEMORY), data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::APPMGR, "Send request err: %{public}d", ret);
        return true;
    }
    return reply.ReadBool();
}

int32_t AmsMgrProxy::SendTransactCmd(uint32_t code, MessageParcel &data,
    MessageParcel &reply, MessageOption &option)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null remote");
        return ERR_NULL_OBJECT;
    }

    int32_t ret = remote->SendRequest(code, data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::APPMGR, "SendRequest err: %{public}d, ret: %{public}d", code, ret);
        return ret;
    }
    return ret;
}

void AmsMgrProxy::AttachedToStatusBar(const sptr<IRemoteObject> &token)
{
    TAG_LOGD(AAFwkTag::APPMGR, "start");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return;
    }
    if (!data.WriteRemoteObject(token)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Failed to write token");
        return;
    }
    int32_t ret = SendTransactCmd(static_cast<uint32_t>(IAmsMgr::Message::ATTACHED_TO_STATUS_BAR),
        data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest err: %{public}d", ret);
    }
    TAG_LOGD(AAFwkTag::APPMGR, "end");
}

void AmsMgrProxy::BlockProcessCacheByPids(const std::vector<int32_t> &pids)
{
    TAG_LOGD(AAFwkTag::APPMGR, "start");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write token failed");
        return;
    }
    if (!data.WriteUint32(pids.size())) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write size failed");
        return;
    }
    for (const auto &pid: pids) {
        if (!data.WriteInt32(pid)) {
            TAG_LOGE(AAFwkTag::APPMGR, "Write pid failed");
            return;
        }
    }
    int32_t ret =
        SendTransactCmd(static_cast<uint32_t>(IAmsMgr::Message::BLOCK_PROCESS_CACHE_BY_PIDS), data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest err: %{public}d", ret);
    }
    TAG_LOGD(AAFwkTag::APPMGR, "end");
}

bool AmsMgrProxy::IsKilledForUpgradeWeb(const std::string &bundleName)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write token failed");
        return false;
    }
    if (!data.WriteString(bundleName)) {
        TAG_LOGE(AAFwkTag::APPMGR, "WriteString failed");
        return false;
    }

    auto ret = SendTransactCmd(static_cast<uint32_t>(IAmsMgr::Message::IS_KILLED_FOR_UPGRADE_WEB), data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::APPMGR, "Send request err: %{public}d", ret);
        return false;
    }
    return reply.ReadBool();
}

bool AmsMgrProxy::CleanAbilityByUserRequest(const sptr<IRemoteObject> &token)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "WriteInterfaceToken failed");
        return false;
    }
    if (!data.WriteRemoteObject(token.GetRefPtr())) {
        TAG_LOGE(AAFwkTag::APPMGR, "Failed to write token");
        return false;
    }

    int32_t ret = SendTransactCmd(
        static_cast<uint32_t>(IAmsMgr::Message::CLEAN_UIABILITY_BY_USER_REQUEST), data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest err: %{public}d", ret);
    }
    return reply.ReadBool();
}

bool AmsMgrProxy::IsProcessAttached(sptr<IRemoteObject> token)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write interface token failed.");
        return false;
    }
    if (!data.WriteRemoteObject(token.GetRefPtr())) {
        TAG_LOGE(AAFwkTag::APPMGR, "Failed to write token");
        return false;
    }

    auto ret = SendTransactCmd(static_cast<uint32_t>(IAmsMgr::Message::IS_PROCESS_ATTACHED), data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::APPMGR, "Send request failed, error code is %{public}d.", ret);
        return false;
    }
    return reply.ReadBool();
}

bool AmsMgrProxy::IsCallerKilling(const std::string& callerKey)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write interface token failed.");
        return false;
    }
    if (!data.WriteString(callerKey)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Failed to write callerKey");
        return false;
    }

    auto ret = SendTransactCmd(static_cast<uint32_t>(IAmsMgr::Message::IS_CALLER_KILLING), data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::APPMGR, "Send request failed, error code is %{public}d.", ret);
        return false;
    }
    return reply.ReadBool();
}

int32_t AmsMgrProxy::PreloadApplicationByPhase(const std::string &bundleName, int32_t userId, int32_t appIndex,
    AppExecFwk::PreloadPhase preloadPhase)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write interface token failed.");
        return AAFwk::ERR_WRITE_INTERFACE_TOKEN_FAILED;
    }
    PARCEL_UTIL_WRITE_RET_INT(data, String, bundleName);
    PARCEL_UTIL_WRITE_RET_INT(data, Int32, userId);
    PARCEL_UTIL_WRITE_RET_INT(data, Int32, appIndex);
    PARCEL_UTIL_WRITE_RET_INT(data, Int32, static_cast<int32_t>(preloadPhase));

    auto ret = SendTransactCmd(static_cast<uint32_t>(IAmsMgr::Message::PRELOAD_APPLICATION_BY_PHASE), data, reply,
        option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::APPMGR, "Send request failed, err: %{public}d.", ret);
        return ret;
    }
    return reply.ReadInt32();
}

int32_t AmsMgrProxy::NotifyPreloadAbilityStateChanged(sptr<IRemoteObject> token, bool isPreForeground)
{
    if (token == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "token null.");
        return AAFwk::INVALID_CALLER_TOKEN;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write interface token failed.");
        return AAFwk::ERR_WRITE_INTERFACE_TOKEN_FAILED;
    }
    if (!data.WriteRemoteObject(token)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write token failed.");
        return AAFwk::ERR_WRITE_CALLER_TOKEN_FAILED;
    }
    PARCEL_UTIL_WRITE_RET_INT(data, Bool, isPreForeground);

    auto ret = SendTransactCmd(
        static_cast<uint32_t>(IAmsMgr::Message::NOTIFY_PRELOAD_ABILITY_STATE_CHANGED), data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::APPMGR, "Send request failed, err: %{public}d.", ret);
        return ret;
    }
    return reply.ReadInt32();
}

int32_t AmsMgrProxy::CheckPreloadAppRecordExist(const std::string &bundleName, int32_t userId, int32_t appIndex,
    bool &isExist)
{
    TAG_LOGD(AAFwkTag::APPMGR, "CheckPreloadAppRecordExist start");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return AAFwk::ERR_WRITE_CALLER_TOKEN_FAILED;
    }
    PARCEL_UTIL_WRITE_RET_BOOL(data, String, bundleName);
    PARCEL_UTIL_WRITE_RET_BOOL(data, Int32, userId);
    PARCEL_UTIL_WRITE_RET_BOOL(data, Int32, appIndex);

    int32_t ret = SendTransactCmd(static_cast<uint32_t>(IAmsMgr::Message::CHECK_PRELOAD_APP_RECORD_EXIST),
        data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::APPMGR, "Send request failed, err: %{public}d.", ret);
        return ret;
    }
    auto replyRet = reply.ReadInt32();
    if (replyRet != NO_ERROR) {
        return replyRet;
    }
    isExist = reply.ReadBool();
    return NO_ERROR;
}

int32_t AmsMgrProxy::VerifyKillProcessPermission(const std::string &bundleName)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write token failed");
        return ERR_INVALID_DATA;
    }

    if (bundleName.empty() || !data.WriteString(bundleName)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write bundleName failed");
        return ERR_INVALID_DATA;
    }

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    auto ret = SendTransactCmd(static_cast<uint32_t>(IAmsMgr::Message::VERIFY_KILL_PROCESS_PERMISSION),
        data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::APPMGR, "Send request err: %{public}d", ret);
        return ret;
    }
    return reply.ReadInt32();
}
} // namespace AppExecFwk
} // namespace OHOS
