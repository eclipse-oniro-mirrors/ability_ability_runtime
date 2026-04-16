/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include "want_agent_client.h"

#include "ability_runtime_error_util.h"
#include "ability_manager_errors.h"
#include "ability_manager_interface.h"
#include "ability_util.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_chain_utils.h"
#include "hitrace_meter.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

using namespace OHOS::AbilityRuntime;
namespace OHOS {
namespace AAFwk {
WantAgentClient &WantAgentClient::GetInstance()
{
    static WantAgentClient client;
    return client;
}

WantAgentClient::WantAgentClient() {}

WantAgentClient::~WantAgentClient()
{
    RemoveDeathRecipient();
}

ErrCode WantAgentClient::GetWantSender(
    const WantSenderInfo &wantSenderInfo, const sptr<IRemoteObject> &callerToken, sptr<IWantSender> &wantSender,
    int32_t uid)
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_AND_RETURN(abms, ERR_ABILITY_RUNTIME_EXTERNAL_SERVICE_BUSY);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER;
    }
    if (!data.WriteParcelable(&wantSenderInfo)) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "write failed");
        return ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER;
    }
    if (callerToken) {
        if (!data.WriteBool(true) || !data.WriteRemoteObject(callerToken)) {
            TAG_LOGE(AAFwkTag::WANTAGENT, "flag and callerToken write failed");
            return ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER;
        }
    } else {
        if (!data.WriteBool(false)) {
            TAG_LOGE(AAFwkTag::WANTAGENT, "flag write failed");
            return ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER;
        }
    }

    if (!data.WriteInt32(uid)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "uid write fail");
        return ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER;
    }

    auto error = abms->SendRequest(static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_PENDING_WANT_SENDER),
        data, reply, option);
    if (error != NO_ERROR) {
        return ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_WANTAGENT;
    }
    wantSender = iface_cast<IWantSender>(reply.ReadRemoteObject());
    return ERR_OK;
}

ErrCode WantAgentClient::SendWantSender(sptr<IWantSender> target, SenderInfo &senderInfo)
{
    CHECK_POINTER_AND_RETURN(target, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_WANTAGENT);
    auto abms = GetAbilityManager();
    CHECK_POINTER_AND_RETURN(abms, ERR_ABILITY_RUNTIME_EXTERNAL_SERVICE_BUSY);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER;
    }
    if (target == nullptr || !data.WriteRemoteObject(target->AsObject())) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "target write failed");
        return ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_WANTAGENT;
    }
    if (!data.WriteParcelable(&senderInfo)) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "senderInfo write failed");
        return ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER;
    }

    auto error = abms->SendRequest(static_cast<uint32_t>(AbilityManagerInterfaceCode::SEND_PENDING_WANT_SENDER),
        data, reply, option);
    if (error != NO_ERROR) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "send request error: %{public}d", error);
        return ERR_ABILITY_RUNTIME_EXTERNAL_SERVICE_TIMEOUT;
    }
    std::unique_ptr<SenderInfo> completedDataReply(reply.ReadParcelable<SenderInfo>());
    if (!completedDataReply) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "readParcelableInfo fail");
        return INNER_ERR;
    }
    senderInfo = *completedDataReply;
    ErrCode res = reply.ReadInt32();
    if (res == ERR_WANTAGENT_CANCELED) {
        return ERR_ABILITY_RUNTIME_EXTERNAL_WANTAGENT_CANCELED;
    } else if (res == ERR_INVALID_VALUE) {
        return ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_WANTAGENT;
    }
    return ERR_OK;
}

ErrCode WantAgentClient::SendLocalWantSender(const SenderInfo &senderInfo)
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_AND_RETURN(abms, ERR_ABILITY_RUNTIME_EXTERNAL_SERVICE_BUSY);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER;
    }
    if (!data.WriteParcelable(&senderInfo)) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "senderInfo write failed");
        return ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER;
    }
    auto error = abms->SendRequest(static_cast<uint32_t>(AbilityManagerInterfaceCode::SEND_LOCAL_PENDING_WANT_SENDER),
        data, reply, option);
    if (error != NO_ERROR) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "send request error: %{public}d", error);
        return ERR_ABILITY_RUNTIME_EXTERNAL_SERVICE_TIMEOUT;
    }
    const auto res = reply.ReadInt32();
    if (res == CHECK_PERMISSION_FAILED) {
        return ERR_ABILITY_RUNTIME_EXTERNAL_NO_ACCESS_PERMISSION;
    }
    return res;
}

ErrCode WantAgentClient::CancelWantSender(const sptr<IWantSender> &sender, uint32_t flags)
{
    CHECK_POINTER_AND_RETURN(sender, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
    auto abms = GetAbilityManager();
    CHECK_POINTER_AND_RETURN(abms, ERR_ABILITY_RUNTIME_EXTERNAL_SERVICE_BUSY);
    ErrCode error;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER;
    }

    if (!data.WriteRemoteObject(sender->AsObject())) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "sender write failed");
        return ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_WANTAGENT;
    }

    if (!data.WriteUint32(flags)) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "flags write failed");
        return ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER;
    }

    error = abms->SendRequest(static_cast<int32_t>(AbilityManagerInterfaceCode::CANCEL_PENDING_WANT_SENDER),
        data, reply, option);
    if (error != NO_ERROR) {
        return ERR_ABILITY_RUNTIME_EXTERNAL_SERVICE_BUSY;
    }

    return ERR_OK;
}

ErrCode WantAgentClient::GetPendingWantUid(const sptr<IWantSender> &target, int32_t &uid)
{
    CHECK_POINTER_AND_RETURN(target, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_WANTAGENT);
    auto abms = GetAbilityManager();
    CHECK_POINTER_AND_RETURN(abms, ERR_ABILITY_RUNTIME_EXTERNAL_SERVICE_BUSY);
    ErrCode error;
    MessageParcel reply;
    if (!SendRequest(static_cast<int32_t>(AbilityManagerInterfaceCode::GET_PENDING_WANT_UID),
        abms, target->AsObject(), reply, error)) {
        return error;
    }
    uid = reply.ReadInt32();
    return ERR_OK;
}

ErrCode WantAgentClient::GetPendingWantUserId(const sptr<IWantSender> &target, int32_t &userId)
{
    CHECK_POINTER_AND_RETURN(target, INVALID_PARAMETERS_ERR);
    auto abms = GetAbilityManager();
    CHECK_POINTER_AND_RETURN(abms, ABILITY_SERVICE_NOT_CONNECTED);
    ErrCode error;
    MessageParcel reply;
    if (!SendRequest(static_cast<int32_t>(AbilityManagerInterfaceCode::GET_PENDING_WANT_USERID),
        abms, target->AsObject(), reply, error)) {
        return error;
    }
    userId = reply.ReadInt32();
    return ERR_OK;
}

ErrCode WantAgentClient::GetPendingWantBundleName(const sptr<IWantSender> &target, std::string &bundleName)
{
    CHECK_POINTER_AND_RETURN(target, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_WANTAGENT);
    auto abms = GetAbilityManager();
    CHECK_POINTER_AND_RETURN(abms, ERR_ABILITY_RUNTIME_EXTERNAL_SERVICE_BUSY);
    ErrCode error;
    MessageParcel reply;
    if (!SendRequest(static_cast<int32_t>(AbilityManagerInterfaceCode::GET_PENDING_WANT_BUNDLENAME),
        abms, target->AsObject(), reply, error)) {
        return error;
    }
    bundleName = Str16ToStr8(reply.ReadString16());
    return ERR_OK;
}

ErrCode WantAgentClient::GetPendingWantCode(const sptr<IWantSender> &target, int32_t &code)
{
    CHECK_POINTER_AND_RETURN(target, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_WANTAGENT);
    auto abms = GetAbilityManager();
    CHECK_POINTER_AND_RETURN(abms, ERR_ABILITY_RUNTIME_EXTERNAL_SERVICE_BUSY);
    ErrCode error;
    MessageParcel reply;
    if (!SendRequest(static_cast<int32_t>(AbilityManagerInterfaceCode::GET_PENDING_WANT_CODE),
        abms, target->AsObject(), reply, error)) {
        return ERR_ABILITY_RUNTIME_EXTERNAL_SERVICE_TIMEOUT;
    }
    code = reply.ReadInt32();
    return ERR_OK;
}

ErrCode WantAgentClient::GetPendingWantType(sptr<IWantSender> target, int32_t &type)
{
    CHECK_POINTER_AND_RETURN(target, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_WANTAGENT);
    auto abms = GetAbilityManager();
    CHECK_POINTER_AND_RETURN(abms, ERR_ABILITY_RUNTIME_EXTERNAL_SERVICE_BUSY);
    ErrCode error;
    MessageParcel reply;
    if (!SendRequest(static_cast<int32_t>(AbilityManagerInterfaceCode::GET_PENDING_WANT_TYPE),
        abms, target->AsObject(), reply, error)) {
        return ERR_ABILITY_RUNTIME_EXTERNAL_SERVICE_TIMEOUT;
    }
    type = reply.ReadInt32();
    type < 0 ? type = 0 : type;
    return ERR_OK;
}

void WantAgentClient::RegisterCancelListener(const sptr<IWantSender> &sender, const sptr<IWantReceiver> &receiver)
{
    if (!CheckSenderAndRecevier(sender, receiver)) {
        return;
    }
    auto abms = GetAbilityManager();
    if (!abms) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "ability proxy is nullptr");
        return;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return;
    }
    if (!data.WriteRemoteObject(sender->AsObject())) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "sender write failed");
        return;
    }
    if (!data.WriteRemoteObject(receiver->AsObject())) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "receiver write failed");
        return;
    }
    auto error = abms->SendRequest(static_cast<uint32_t>(AbilityManagerInterfaceCode::REGISTER_CANCEL_LISTENER),
        data, reply, option);
    if (error != NO_ERROR) {
        return;
    }
}

void WantAgentClient::UnregisterCancelListener(
    const sptr<IWantSender> &sender, const sptr<IWantReceiver> &receiver)
{
    if (!CheckSenderAndRecevier(sender, receiver)) {
        return;
    }
    auto abms = GetAbilityManager();
    if (!abms) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "ability proxy is nullptr");
        return;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return;
    }
    if (!data.WriteRemoteObject(sender->AsObject())) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "sender write failed");
        return;
    }
    if (!data.WriteRemoteObject(receiver->AsObject())) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "receiver write failed");
        return;
    }
    auto error = abms->SendRequest(static_cast<uint32_t>(AbilityManagerInterfaceCode::UNREGISTER_CANCEL_LISTENER),
        data, reply, option);
    if (error != NO_ERROR) {
        return;
    }
}

ErrCode WantAgentClient::GetPendingRequestWant(const sptr<IWantSender> &target, std::shared_ptr<Want> &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    CHECK_POINTER_AND_RETURN(target, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_WANTAGENT);
    CHECK_POINTER_AND_RETURN(want, INVALID_PARAMETERS_ERR);
    auto abms = GetAbilityManager();
    CHECK_POINTER_AND_RETURN(abms, ERR_ABILITY_RUNTIME_EXTERNAL_SERVICE_BUSY);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "Input data target is invalid");
        return ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER;
    }

    sptr<IRemoteObject> obj = target->AsObject();
    if (obj == nullptr) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "Input Param target is invalid");
        return ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER;
    }

    if (!data.WriteRemoteObject(obj)) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "target write failed");
        return ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER;
    }
    if (!data.WriteParcelable(want.get())) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "want write failed");
        return ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER;
    }
    auto error = abms->SendRequest(static_cast<int32_t>(AbilityManagerInterfaceCode::GET_PENDING_REQUEST_WANT),
        data, reply, option);
    if (error != NO_ERROR) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "Send request error: %{public}d", error);
        return ERR_ABILITY_RUNTIME_EXTERNAL_SERVICE_TIMEOUT;
    }
    std::unique_ptr<Want> wantInfo(reply.ReadParcelable<Want>());
    if (!wantInfo) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "ReadParcelable failed");
        return ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER;
    }
    want = std::move(wantInfo);

    return NO_ERROR;
}

ErrCode WantAgentClient::GetPendingRequestWantFromProxy(const sptr<IWantSender> &target, std::shared_ptr<Want> &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    CHECK_POINTER_AND_RETURN(target, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_WANTAGENT);
    CHECK_POINTER_AND_RETURN(want, INVALID_PARAMETERS_ERR);
    auto abms = GetAbilityManager();
    CHECK_POINTER_AND_RETURN(abms, ERR_ABILITY_RUNTIME_EXTERNAL_SERVICE_BUSY);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "Input data target is invalid");
        return ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER;
    }

    sptr<IRemoteObject> obj = target->AsObject();
    if (obj == nullptr) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "Input Param target is invalid");
        return ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER;
    }

    if (!data.WriteRemoteObject(obj)) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "target write failed");
        return ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER;
    }
    if (!data.WriteParcelable(want.get())) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "want write failed");
        return ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER;
    }
    auto error = abms->SendRequest(static_cast<int32_t>(AbilityManagerInterfaceCode::
        GET_PENDING_REQUEST_WANT_FROM_PROXY), data, reply, option);
    if (error != NO_ERROR) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "Send request error: %{public}d", error);
        return ERR_ABILITY_RUNTIME_EXTERNAL_SERVICE_TIMEOUT;
    }
    std::unique_ptr<Want> wantInfo(reply.ReadParcelable<Want>());
    if (!wantInfo) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "ReadParcelable failed");
        return ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER;
    }
    want = std::move(wantInfo);

    return NO_ERROR;
}

ErrCode WantAgentClient::GetWantSenderInfo(const sptr<IWantSender> &target, std::shared_ptr<WantSenderInfo> &info)
{
    CHECK_POINTER_AND_RETURN(target, INVALID_PARAMETERS_ERR);
    CHECK_POINTER_AND_RETURN(info, INVALID_PARAMETERS_ERR);
    auto abms = GetAbilityManager();
    CHECK_POINTER_AND_RETURN(abms, ABILITY_SERVICE_NOT_CONNECTED);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    if (!data.WriteRemoteObject(target->AsObject())) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "target write failed");
        return INNER_ERR;
    }
    if (!data.WriteParcelable(info.get())) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "info write failed");
        return INNER_ERR;
    }
    auto error = abms->SendRequest(static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_PENDING_WANT_SENDER_INFO),
        data, reply, option);
    if (error != NO_ERROR) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "Send request error: %{public}d", error);
        return error;
    }
    std::unique_ptr<WantSenderInfo> wantSenderInfo(reply.ReadParcelable<WantSenderInfo>());
    if (!wantSenderInfo) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "readParcelable Info failed");
        return INNER_ERR;
    }
    info = std::move(wantSenderInfo);

    return NO_ERROR;
}

sptr<IRemoteObject> WantAgentClient::GetAbilityManager()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::lock_guard<std::mutex> lock(mutex_);
    if (proxy_ == nullptr) {
        auto systemManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        if (systemManager == nullptr) {
            TAG_LOGE(AAFwkTag::WANTAGENT, "Fail to get registry");
            return nullptr;
        }
        auto remoteObj = systemManager->GetSystemAbility(ABILITY_MGR_SERVICE_ID);
        if (remoteObj == nullptr) {
            TAG_LOGE(AAFwkTag::WANTAGENT, "Fail to connect ability manager service");
            return nullptr;
        }

        deathRecipient_ = sptr<IRemoteObject::DeathRecipient>(new (std::nothrow) WantAgentDeathRecipient());
        if (deathRecipient_ == nullptr) {
            TAG_LOGE(AAFwkTag::WANTAGENT, "Failed to create DeathRecipient");
            return nullptr;
        }
        if (!remoteObj->AddDeathRecipient(deathRecipient_)) {
            TAG_LOGE(AAFwkTag::WANTAGENT, "Add death recipient to failed");
        }
        proxy_ = remoteObj;
    }

    return proxy_;
}

void WantAgentClient::RemoveDeathRecipient()
{
    TAG_LOGI(AAFwkTag::WANTAGENT, "remove death recipient");
    std::lock_guard<std::mutex> lock(mutex_);
    if (proxy_ == nullptr) {
        TAG_LOGI(AAFwkTag::WANTAGENT, "null proxy_");
        return;
    }

    if (deathRecipient_ == nullptr) {
        TAG_LOGI(AAFwkTag::WANTAGENT, "null deathRecipient_");
        return;
    }

    if (proxy_->RemoveDeathRecipient(deathRecipient_)) {
        proxy_ = nullptr;
        deathRecipient_ = nullptr;
        TAG_LOGI(AAFwkTag::WANTAGENT, "remove success");
    }
}

void WantAgentClient::WantAgentDeathRecipient::OnRemoteDied(const wptr<IRemoteObject>& remote)
{
    TAG_LOGI(AAFwkTag::WANTAGENT, "call");
    WantAgentClient::GetInstance().ResetProxy(remote);
}

void WantAgentClient::ResetProxy(const wptr<IRemoteObject>& remote)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (!proxy_) {
        return;
    }
    if (proxy_ == remote.promote()) {
        proxy_->RemoveDeathRecipient(deathRecipient_);
    }
    proxy_ = nullptr;
}

bool WantAgentClient::WriteInterfaceToken(MessageParcel &data)
{
    if (!data.WriteInterfaceToken(IAbilityManager::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "write interface token failed");
        return false;
    }
    return true;
}

bool WantAgentClient::CheckSenderAndRecevier(const sptr<IWantSender> &sender, const sptr<IWantReceiver> &receiver)
{
    if (sender == nullptr) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "sender is nullptr");
        return false;
    }
    if (receiver == nullptr) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "receiver is nullptr");
        return false;
    }

    return true;
}

bool WantAgentClient::SendRequest(int32_t operation, const sptr<IRemoteObject> &abms,
    const sptr<IRemoteObject> &remoteObject, MessageParcel &reply, ErrCode &error)
{
    MessageParcel data;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        error = ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER;
        return false;
    }
    if (!data.WriteRemoteObject(remoteObject)) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "write failed");
        error = ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER;
        return false;
    }
    error = abms->SendRequest(operation, data, reply, option);
    if (error != NO_ERROR) {
        error = ERR_ABILITY_RUNTIME_EXTERNAL_SERVICE_BUSY;
        return false;
    }

    return true;
}
}  // namespace AAFwk
}  // namespace OHOS
