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

#include "ability_info_callback_proxy.h"

#include "hilog_tag_wrapper.h"
#include "ipc_capacity_wrap.h"
#include "ipc_types.h"

namespace OHOS {
namespace AppExecFwk {
AbilityInfoCallbackProxy::AbilityInfoCallbackProxy(
    const sptr<IRemoteObject> &impl) : IRemoteProxy<IAbilityInfoCallback>(impl)
{}

bool AbilityInfoCallbackProxy::WriteInterfaceToken(MessageParcel &data)
{
    if (!data.WriteInterfaceToken(AbilityInfoCallbackProxy::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::APPMGR, "write token failed");
        return false;
    }
    return true;
}

void AbilityInfoCallbackProxy::NotifyAbilityToken(const sptr<IRemoteObject> token, const Want &want)
{
    MessageParcel data;
    MessageParcel reply;
    AAFwk::ExtendMaxIpcCapacityForInnerWant(data);
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        TAG_LOGW(AAFwkTag::APPMGR, "write data failed");
        return;
    }
    if (!data.WriteRemoteObject(token)) {
        TAG_LOGW(AAFwkTag::APPMGR, "write token failed");
        return;
    }
    if (!data.WriteParcelable(&want)) {
        TAG_LOGW(AAFwkTag::APPMGR, "write want failed");
        return;
    }
    int32_t ret = SendTransactCmd(IAbilityInfoCallback::Notify_ABILITY_TOKEN, data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest err: %{public}d", ret);
        return;
    }
}

void AbilityInfoCallbackProxy::NotifyRestartSpecifiedAbility(const sptr<IRemoteObject> &token)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        TAG_LOGW(AAFwkTag::APPMGR, "write data failed");
        return;
    }
    if (!data.WriteRemoteObject(token)) {
        TAG_LOGW(AAFwkTag::APPMGR, "write token failed");
        return;
    }

    int32_t ret = SendTransactCmd(IAbilityInfoCallback::Notify_RESTART_SPECIFIED_ABILITY, data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest err: %{public}d", ret);
    }
}

void AbilityInfoCallbackProxy::NotifyStartSpecifiedAbility(const sptr<IRemoteObject> &callerToken,
    const Want &want, int requestCode, sptr<Want> &extraParam)
{
    MessageParcel data;
    MessageParcel reply;
    AAFwk::ExtendMaxIpcCapacityForInnerWant(data);
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        TAG_LOGW(AAFwkTag::APPMGR, "write data failed");
        return;
    }
    if (!data.WriteRemoteObject(callerToken)) {
        TAG_LOGW(AAFwkTag::APPMGR, "write callerToken failed");
        return;
    }
    if (!data.WriteParcelable(&want)) {
        TAG_LOGW(AAFwkTag::APPMGR, "write want failed");
        return;
    }
    if (!data.WriteInt32(requestCode)) {
        TAG_LOGW(AAFwkTag::APPMGR, "write requestCode failed");
        return;
    }
    int32_t ret = SendTransactCmd(IAbilityInfoCallback::Notify_START_SPECIFIED_ABILITY, data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest failed, err: %{public}d", ret);
        return;
    }
    sptr<Want> tempWant = reply.ReadParcelable<Want>();
    if (tempWant != nullptr) {
        SetExtraParam(tempWant, extraParam);
    }
}

void AbilityInfoCallbackProxy::SetExtraParam(const sptr<Want> &want, sptr<Want> &extraParam)
{
    if (!want || !extraParam) {
        TAG_LOGE(AAFwkTag::APPMGR, "invalid param");
        return;
    }

    sptr<IRemoteObject> tempCallBack = want->GetRemoteObject(Want::PARAM_RESV_ABILITY_INFO_CALLBACK);
    if (tempCallBack == nullptr) {
        return;
    }
    extraParam->SetParam(Want::PARAM_RESV_REQUEST_PROC_CODE,
        want->GetIntParam(Want::PARAM_RESV_REQUEST_PROC_CODE, 0));
    extraParam->SetParam(Want::PARAM_RESV_REQUEST_TOKEN_CODE,
        want->GetIntParam(Want::PARAM_RESV_REQUEST_TOKEN_CODE, 0));
    extraParam->SetParam(Want::PARAM_RESV_ABILITY_INFO_CALLBACK, tempCallBack);
}

void AbilityInfoCallbackProxy::NotifyStartAbilityResult(const Want &want, int result)
{
    MessageParcel data;
    MessageParcel reply;
    AAFwk::ExtendMaxIpcCapacityForInnerWant(data);
    MessageOption option(MessageOption::TF_ASYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }
    if (!data.WriteParcelable(&want)) {
        TAG_LOGW(AAFwkTag::APPMGR, "write want failed");
        return;
    }
    if (!data.WriteInt32(result)) {
        TAG_LOGW(AAFwkTag::APPMGR, "write result failed");
        return;
    }
    int32_t ret = SendTransactCmd(IAbilityInfoCallback::Notify_START_ABILITY_RESULT, data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGW(AAFwkTag::APPMGR, "err: %{public}d", ret);
        return;
    }
}

int32_t AbilityInfoCallbackProxy::SendTransactCmd(uint32_t code, MessageParcel &data,
    MessageParcel &reply, MessageOption &option)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null remote");
        return ERR_NULL_OBJECT;
    }

    return remote->SendRequest(code, data, reply, option);
}
}  // namespace AppExecFwk
}  // namespace OHOS
