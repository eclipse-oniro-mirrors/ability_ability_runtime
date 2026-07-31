/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "dataobs_mgr_proxy.h"

#include "dataobs_mgr_interface.h"
#include "errors.h"
#include "hilog_tag_wrapper.h"
#include "dataobs_mgr_errors.h"
#include "common_utils.h"

namespace OHOS {
namespace AAFwk {
bool DataObsManagerProxy::WriteInterfaceToken(MessageParcel &data)
{
    if (!data.WriteInterfaceToken(DataObsManagerProxy::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::DBOBSMGR, "write token error");
        return false;
    }
    return true;
}

bool DataObsManagerProxy::WriteParam(MessageParcel &data, const Uri &uri, sptr<IDataAbilityObserver> dataObserver)
{
    if (!data.WriteString(uri.ToString())) {
        TAG_LOGE(AAFwkTag::DBOBSMGR, "write uri error");
        return false;
    }

    if (dataObserver == nullptr) {
        TAG_LOGE(AAFwkTag::DBOBSMGR, "null dataObserver");
        return false;
    }

    if (!data.WriteRemoteObject(dataObserver->AsObject())) {
        TAG_LOGE(AAFwkTag::DBOBSMGR, "write dataObserver error");
        return false;
    }
    return true;
}

bool DataObsManagerProxy::WriteObsOpt(MessageParcel &data, DataObsOption opt)
{
    if (!data.WriteBool(opt.IsSystem())) {
        TAG_LOGE(AAFwkTag::DBOBSMGR, "write opt error");
        return false;
    }
    if (!data.WriteUint32(opt.FirstCallerTokenID())) {
        TAG_LOGE(AAFwkTag::DBOBSMGR, "write opt error");
        return false;
    }
    if (!data.WriteInt32(opt.FirstCallerPid())) {
        TAG_LOGE(AAFwkTag::DBOBSMGR, "write opt error");
        return false;
    }
    if (!data.WriteBool(opt.IsDataShare())) {
        TAG_LOGE(AAFwkTag::DBOBSMGR, "write opt error");
        return false;
    }
    if (!data.WriteUint64(opt.FirstCallerFullTokenID())) {
        TAG_LOGE(AAFwkTag::DBOBSMGR, "write FirstCallerFullTokenID error");
        return false;
    }
    return true;
}

int32_t DataObsManagerProxy::RegisterObserver(const Uri &uri,
    sptr<IDataAbilityObserver> dataObserver, int32_t userId, DataObsOption opt)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return IPC_PARCEL_ERROR;
    }

    if (!WriteParam(data, uri, dataObserver)) {
        return INVALID_PARAM;
    }
    if (!data.WriteInt32(userId)) {
        return INVALID_PARAM;
    }
    if (!WriteObsOpt(data, opt)) {
        return INVALID_PARAM;
    }
    
    auto error = SendTransactCmd(IDataObsMgr::REGISTER_OBSERVER, data, reply, option);
    if (error != NO_ERROR) {
        TAG_LOGE(AAFwkTag::DBOBSMGR, "sendRequest error:%{public}d, uri:%{public}s", error,
            CommonUtils::Anonymous(uri.ToString()).c_str());
        return error;
    }

    int32_t res = IPC_ERROR;
    return reply.ReadInt32(res) ? res : IPC_ERROR;
}

int32_t DataObsManagerProxy::RegisterObserverFromExtension(const Uri &uri,
    sptr<IDataAbilityObserver> dataObserver, int32_t userId,
    DataObsOption opt)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return IPC_PARCEL_ERROR;
    }

    if (!WriteParam(data, uri, dataObserver)) {
        return INVALID_PARAM;
    }
    if (!data.WriteInt32(userId)) {
        return INVALID_PARAM;
    }
    if (!WriteObsOpt(data, opt)) {
        return INVALID_PARAM;
    }

    auto error = SendTransactCmd(IDataObsMgr::REGISTER_OBSERVER_FROM_EXTENSION, data, reply, option);
    if (error != NO_ERROR) {
        TAG_LOGE(AAFwkTag::DBOBSMGR, "sendRequest error:%{public}d, uri:%{public}s", error,
            CommonUtils::Anonymous(uri.ToString()).c_str());
        return error;
    }

    int32_t res = IPC_ERROR;
    return reply.ReadInt32(res) ? res : IPC_ERROR;
}

int32_t DataObsManagerProxy::UnregisterObserver(const Uri &uri, sptr<IDataAbilityObserver> dataObserver,
    int32_t userId, DataObsOption opt)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return IPC_PARCEL_ERROR;
    }

    if (!WriteParam(data, uri, dataObserver)) {
        return INVALID_PARAM;
    }
    if (!data.WriteInt32(userId)) {
        return INVALID_PARAM;
    }
    if (!WriteObsOpt(data, opt)) {
        return INVALID_PARAM;
    }

    auto error = SendTransactCmd(IDataObsMgr::UNREGISTER_OBSERVER, data, reply, option);
    if (error != NO_ERROR) {
        TAG_LOGE(AAFwkTag::DBOBSMGR, "sendRequest error:%{public}d, uri:%{public}s", error,
            CommonUtils::Anonymous(uri.ToString()).c_str());
        return error;
    }
    int32_t res = IPC_ERROR;
    return reply.ReadInt32(res) ? res : IPC_ERROR;
}

int32_t DataObsManagerProxy::NotifyChange(const Uri &uri, int32_t userId, DataObsOption opt)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return IPC_PARCEL_ERROR;
    }
    if (!data.WriteString(uri.ToString())) {
        TAG_LOGE(AAFwkTag::DBOBSMGR, "write uri error, uri:%{public}s",
            CommonUtils::Anonymous(uri.ToString()).c_str());
        return INVALID_PARAM;
    }
    if (!data.WriteInt32(userId)) {
        return INVALID_PARAM;
    }
    if (!WriteObsOpt(data, opt)) {
        return INVALID_PARAM;
    }
    auto error = SendTransactCmd(IDataObsMgr::NOTIFY_CHANGE, data, reply, option);
    if (error != NO_ERROR) {
        TAG_LOGE(AAFwkTag::DBOBSMGR, "sendRequest error:%{public}d, uri:%{public}s", error,
            CommonUtils::Anonymous(uri.ToString()).c_str());
        return IPC_ERROR;
    }

    int32_t res = IPC_ERROR;
    return reply.ReadInt32(res) ? res : IPC_ERROR;
}

int32_t DataObsManagerProxy::NotifyChangeFromExtension(const Uri &uri, int32_t userId, DataObsOption opt)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return IPC_PARCEL_ERROR;
    }
    if (!data.WriteString(uri.ToString())) {
        TAG_LOGE(AAFwkTag::DBOBSMGR, "write uri error, uri:%{public}s",
            CommonUtils::Anonymous(uri.ToString()).c_str());
        return INVALID_PARAM;
    }
    if (!data.WriteInt32(userId)) {
        return INVALID_PARAM;
    }
    if (!WriteObsOpt(data, opt)) {
        return INVALID_PARAM;
    }
    auto error = SendTransactCmd(IDataObsMgr::NOTIFY_CHANGE_FROM_EXTENSION, data, reply, option);
    if (error != NO_ERROR) {
        TAG_LOGE(AAFwkTag::DBOBSMGR, "sendRequest error:%{public}d, uri:%{public}s", error,
            CommonUtils::Anonymous(uri.ToString()).c_str());
        return IPC_ERROR;
    }

    int32_t res = IPC_ERROR;
    return reply.ReadInt32(res) ? res : IPC_ERROR;
}

Status DataObsManagerProxy::RegisterObserverExt(const Uri &uri, sptr<IDataAbilityObserver> dataObserver,
    bool isDescendants, DataObsOption opt)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return IPC_PARCEL_ERROR;
    }

    if (!WriteParam(data, uri, dataObserver)) {
        return INVALID_PARAM;
    }

    if (!data.WriteBool(isDescendants)) {
        TAG_LOGE(AAFwkTag::DBOBSMGR, "isDescendants error, uri:%{public}s,isDescendants:%{public}d",
            CommonUtils::Anonymous(uri.ToString()).c_str(), isDescendants);
        return INVALID_PARAM;
    }
    if (!WriteObsOpt(data, opt)) {
        return INVALID_PARAM;
    }

    auto error = SendTransactCmd(IDataObsMgr::REGISTER_OBSERVER_EXT, data, reply, option);
    if (error != NO_ERROR) {
        TAG_LOGE(AAFwkTag::DBOBSMGR,
            "sendRequest error: %{public}d, uri:%{public}s, isDescendants:%{public}d", error,
            CommonUtils::Anonymous(uri.ToString()).c_str(), isDescendants);
        return IPC_ERROR;
    }
    int32_t res = IPC_ERROR;
    return reply.ReadInt32(res) ? static_cast<Status>(res) : IPC_ERROR;
}

Status DataObsManagerProxy::UnregisterObserverExt(const Uri &uri, sptr<IDataAbilityObserver> dataObserver,
    DataObsOption opt)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return IPC_PARCEL_ERROR;
    }

    if (!WriteParam(data, uri, dataObserver)) {
        return INVALID_PARAM;
    }
    if (!WriteObsOpt(data, opt)) {
        return INVALID_PARAM;
    }

    auto error = SendTransactCmd(IDataObsMgr::UNREGISTER_OBSERVER_EXT, data, reply, option);
    if (error != NO_ERROR) {
        TAG_LOGE(AAFwkTag::DBOBSMGR, "sendRequest error:%{public}d, uri:%{public}s", error,
            CommonUtils::Anonymous(uri.ToString()).c_str());
        return IPC_ERROR;
    }
    int32_t res = IPC_ERROR;
    return reply.ReadInt32(res) ? static_cast<Status>(res) : IPC_ERROR;
}

Status DataObsManagerProxy::UnregisterObserverExt(sptr<IDataAbilityObserver> dataObserver, DataObsOption opt)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return IPC_PARCEL_ERROR;
    }

    if (dataObserver == nullptr) {
        TAG_LOGE(AAFwkTag::DBOBSMGR, "null dataObserver");
        return INVALID_PARAM;
    }

    if (!data.WriteRemoteObject(dataObserver->AsObject())) {
        TAG_LOGE(AAFwkTag::DBOBSMGR, "write dataObserver error");
        return INVALID_PARAM;
    }
    if (!WriteObsOpt(data, opt)) {
        return INVALID_PARAM;
    }

    auto error = SendTransactCmd(IDataObsMgr::UNREGISTER_OBSERVER_ALL_EXT, data, reply, option);
    if (error != NO_ERROR) {
        TAG_LOGE(AAFwkTag::DBOBSMGR, "sendRequest error:%{public}d", error);
        return IPC_ERROR;
    }
    int32_t res = IPC_ERROR;
    return reply.ReadInt32(res) ? static_cast<Status>(res) : IPC_ERROR;
}

Status DataObsManagerProxy::NotifyChangeExt(const ChangeInfo &changeInfo, DataObsOption opt)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return IPC_PARCEL_ERROR;
    }

    if (!ChangeInfo::Marshalling(changeInfo, data)) {
        TAG_LOGE(AAFwkTag::DBOBSMGR,
            "changeInfo marshalling error, changeType:%{public}ud, num:%{public}zu,"
            "null data:%{public}d, size:%{public}ud",
            changeInfo.changeType_, changeInfo.uris_.size(), changeInfo.data_ == nullptr, changeInfo.size_);
        return INVALID_PARAM;
    }
    if (!WriteObsOpt(data, opt)) {
        return INVALID_PARAM;
    }

    auto error = SendTransactCmd(IDataObsMgr::NOTIFY_CHANGE_EXT, data, reply, option);
    if (error != NO_ERROR) {
        TAG_LOGE(AAFwkTag::DBOBSMGR,
            "sendRequest error: %{public}d, changeType:%{public}ud, num:%{public}zu,"
            "null data:%{public}d, size:%{public}ud",
            error, changeInfo.changeType_, changeInfo.uris_.size(), changeInfo.data_ == nullptr, changeInfo.size_);
        return IPC_ERROR;
    }
    int32_t res = IPC_ERROR;
    return reply.ReadInt32(res) ? static_cast<Status>(res) : IPC_ERROR;
}

Status DataObsManagerProxy::NotifyProcessObserver(const std::string &key, const sptr<IRemoteObject> observer)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return IPC_PARCEL_ERROR;
    }

    if (!data.WriteString(key)) {
        TAG_LOGE(AAFwkTag::DBOBSMGR, "write key error");
        return INVALID_PARAM;
    }

    if (!data.WriteRemoteObject(observer)) {
        TAG_LOGE(AAFwkTag::DBOBSMGR, "write observer error");
        return INVALID_PARAM;
    }
    auto error = SendTransactCmd(IDataObsMgr::NOTIFY_PROCESS, data, reply, option);
    if (error != NO_ERROR) {
        TAG_LOGE(AAFwkTag::DBOBSMGR, "sendRequest error: %{public}d, key:%{public}s", error, key.c_str());
        return IPC_ERROR;
    }
    int32_t res = IPC_ERROR;
    return reply.ReadInt32(res) ? static_cast<Status>(res) : IPC_ERROR;
}

int32_t DataObsManagerProxy::SendTransactCmd(uint32_t code, MessageParcel &data,
    MessageParcel &reply, MessageOption &option)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TAG_LOGE(AAFwkTag::DBOBSMGR, "null remote");
        return ERR_NULL_OBJECT;
    }

    int32_t ret = remote->SendRequest(code, data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::DBOBSMGR, "sendRequest code:%{public}d, ret:%{public}d", code, ret);
        return ret;
    }
    return NO_ERROR;
}
}  // namespace AAFwk
}  // namespace OHOS
