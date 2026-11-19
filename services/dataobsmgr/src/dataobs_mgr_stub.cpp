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

#include "dataobs_mgr_stub.h"

#include "dataobs_mgr_interface.h"
#include "string_ex.h"

#include "data_ability_observer_proxy.h"
#include "dataobs_mgr_errors.h"
#include "datashare_log.h"
#include "ipc_skeleton.h"
#include "common_utils.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AAFwk {
using namespace DataShare;
using Uri = OHOS::Uri;

const DataObsManagerStub::RequestFuncType DataObsManagerStub::HANDLES[TRANS_BUTT] = {
    &DataObsManagerStub::RegisterObserverInner,
    &DataObsManagerStub::UnregisterObserverInner,
    &DataObsManagerStub::NotifyChangeInner,
    &DataObsManagerStub::RegisterObserverExtInner,
    &DataObsManagerStub::UnregisterObserverExtInner,
    &DataObsManagerStub::UnregisterObserverExtALLInner,
    &DataObsManagerStub::NotifyChangeExtInner,
    &DataObsManagerStub::NotifyProcessObserverInner,
    &DataObsManagerStub::RegisterObserverFromExtensionInner,
    &DataObsManagerStub::NotifyChangeFromExtensionInner
};

DataObsManagerStub::DataObsManagerStub() {}

DataObsManagerStub::~DataObsManagerStub() {}

DataObsOption ReadObsOpt(MessageParcel &data)
{
    bool isSystem = data.ReadBool();
    uint32_t token = data.ReadUint32();
    int32_t pid = data.ReadInt32();
    bool isDataShare = data.ReadBool();
    uint64_t fullToken = data.ReadUint64();
    DataObsOption opt(isSystem);
    opt.SetFirstCallerTokenID(token);
    opt.SetFirstCallerPid(pid);
    opt.SetDataShare(isDataShare);
    opt.SetFirstCallerFullTokenID(fullToken);
    return opt;
}

int DataObsManagerStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    TAG_LOGD(AAFwkTag::DBOBSMGR, "code: %{public}d, flags: %{public}d, callingPid:%{public}d", code, option.GetFlags(),
        IPCSkeleton::GetCallingPid());
    std::u16string descriptor = DataObsManagerStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (descriptor != remoteDescriptor) {
        TAG_LOGE(AAFwkTag::DBOBSMGR,
            "local descriptor≠remote, descriptor:%{public}s, remoteDescriptor:%{public}s",
            CommonUtils::Anonymous(Str16ToStr8(descriptor)).c_str(),
            CommonUtils::Anonymous(Str16ToStr8(remoteDescriptor)).c_str());
        return ERR_INVALID_STATE;
    }

    if (code < TRANS_HEAD || code >= TRANS_BUTT || HANDLES[code] == nullptr) {
        TAG_LOGE(AAFwkTag::DBOBSMGR, "invalid code:%{public}u, BUTT:%{public}d", code, TRANS_BUTT);
        return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
    return (this->*HANDLES[code])(data, reply);
}

int DataObsManagerStub::RegisterObserverInner(MessageParcel &data, MessageParcel &reply)
{
    Uri uri(data.ReadString());
    if (uri.ToString().empty()) {
        TAG_LOGE(AAFwkTag::DBOBSMGR, "invalid uri");
        return IPC_STUB_INVALID_DATA_ERR;
    }

    auto remote = data.ReadRemoteObject();
    auto observer = remote == nullptr ? nullptr : iface_cast<IDataAbilityObserver>(remote);
    int32_t userId = data.ReadInt32();
    DataObsOption opt = ReadObsOpt(data);
    int32_t result = RegisterObserver(uri, observer, userId, opt);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int DataObsManagerStub::RegisterObserverFromExtensionInner(MessageParcel &data, MessageParcel &reply)
{
    Uri uri(data.ReadString());
    if (uri.ToString().empty()) {
        TAG_LOGE(AAFwkTag::DBOBSMGR, "invalid uri");
        return IPC_STUB_INVALID_DATA_ERR;
    }

    auto remote = data.ReadRemoteObject();
    auto observer = remote == nullptr ? nullptr : iface_cast<IDataAbilityObserver>(remote);
    int32_t userId = data.ReadInt32();
    DataObsOption opt = ReadObsOpt(data);
    int32_t result = RegisterObserverFromExtension(uri, observer, userId, opt);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int DataObsManagerStub::UnregisterObserverInner(MessageParcel &data, MessageParcel &reply)
{
    Uri uri(data.ReadString());
    if (uri.ToString().empty()) {
        TAG_LOGE(AAFwkTag::DBOBSMGR, "invalid uri");
        return IPC_STUB_INVALID_DATA_ERR;
    }

    auto remote = data.ReadRemoteObject();
    auto observer = remote == nullptr ? nullptr : iface_cast<IDataAbilityObserver>(remote);
    int32_t userId = data.ReadInt32();
    DataObsOption opt = DataObsOption(data.ReadBool());
    int32_t result = UnregisterObserver(uri, observer, userId, opt);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int DataObsManagerStub::NotifyChangeInner(MessageParcel &data, MessageParcel &reply)
{
    Uri uri(data.ReadString());
    if (uri.ToString().empty()) {
        TAG_LOGE(AAFwkTag::DBOBSMGR, "invalid uri");
        return IPC_STUB_INVALID_DATA_ERR;
    }
    int32_t userId = data.ReadInt32();
    DataObsOption opt = DataObsOption(data.ReadBool());
    int32_t result = NotifyChange(uri, userId, opt);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int DataObsManagerStub::NotifyChangeFromExtensionInner(MessageParcel &data, MessageParcel &reply)
{
    Uri uri(data.ReadString());
    if (uri.ToString().empty()) {
        TAG_LOGE(AAFwkTag::DBOBSMGR, "invalid uri");
        return IPC_STUB_INVALID_DATA_ERR;
    }
    int32_t userId = data.ReadInt32();
    DataObsOption opt = ReadObsOpt(data);
    int32_t result = NotifyChangeFromExtension(uri, userId, opt);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int32_t DataObsManagerStub::RegisterObserverExtInner(MessageParcel &data, MessageParcel &reply)
{
    Uri uri(data.ReadString());
    if (uri.ToString().empty()) {
        TAG_LOGE(AAFwkTag::DBOBSMGR, "invalid uri");
        return IPC_STUB_INVALID_DATA_ERR;
    }
    auto remote = data.ReadRemoteObject();
    auto observer = remote == nullptr ? nullptr : iface_cast<IDataAbilityObserver>(remote);
    bool isDescendants = data.ReadBool();
    DataObsOption opt = ReadObsOpt(data);
    reply.WriteInt32(RegisterObserverExt(uri, observer, isDescendants, opt));
    return SUCCESS;
}

int32_t DataObsManagerStub::UnregisterObserverExtInner(MessageParcel &data, MessageParcel &reply)
{
    Uri uri(data.ReadString());
    if (uri.ToString().empty()) {
        TAG_LOGE(AAFwkTag::DBOBSMGR, "invalid uri");
        return IPC_STUB_INVALID_DATA_ERR;
    }
    auto remote = data.ReadRemoteObject();
    auto observer = remote == nullptr ? nullptr : iface_cast<IDataAbilityObserver>(remote);
    DataObsOption opt = ReadObsOpt(data);
    reply.WriteInt32(UnregisterObserverExt(uri, observer, opt));
    return SUCCESS;
}

int32_t DataObsManagerStub::UnregisterObserverExtALLInner(MessageParcel &data, MessageParcel &reply)
{
    auto remote = data.ReadRemoteObject();
    auto observer = remote == nullptr ? nullptr : iface_cast<IDataAbilityObserver>(remote);
    DataObsOption opt = ReadObsOpt(data);
    reply.WriteInt32(UnregisterObserverExt(observer, opt));
    return SUCCESS;
}

int32_t DataObsManagerStub::NotifyChangeExtInner(MessageParcel &data, MessageParcel &reply)
{
    ChangeInfo changeInfo;
    if (!ChangeInfo::Unmarshalling(changeInfo, data)) {
        LOG_ERROR("Failed to unmarshall changeInfo.");
        return IPC_STUB_INVALID_DATA_ERR;
    }
    DataObsOption opt = ReadObsOpt(data);
    reply.WriteInt32(NotifyChangeExt(changeInfo, opt));
    return SUCCESS;
}

int32_t DataObsManagerStub::NotifyProcessObserverInner(MessageParcel &data, MessageParcel &reply)
{
    std::string key = data.ReadString();
    auto observer = data.ReadRemoteObject();
    reply.WriteInt32(NotifyProcessObserver(key, observer));
    return SUCCESS;
}
}  // namespace AAFwk
}  // namespace OHOS
