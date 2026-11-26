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

#include "intent_client.h"

#include "hilog_tag_wrapper.h"
#include "insight_intent_host_client.h"
#include "iservice_registry.h"
#include "message_parcel.h"
#include "string_ex.h"
#include "system_ability_definition.h"

#include "ability_manager_errors.h"
#include "ability_manager_ipc_interface_code.h"
#include "insight_intent_execute_param.h"
#include "long_wrapper.h"
#include "remote_object_wrapper.h"
#include "want_params_wrapper.h"

namespace OHOS {
namespace AAFwk {

namespace {
// IPC interface token for AbilityManagerService. Must match the descriptor
// declared by DECLARE_INTERFACE_DESCRIPTOR(IAbilityManager) in
// interfaces/inner_api/ability_manager/include/ability_manager_interface.h.
constexpr const char *PERMISSION_DESC = "ohos.aafwk.AbilityManager";
}

IntentClient &IntentClient::GetInstance()
{
    static IntentClient instance;
    return instance;
}

sptr<IRemoteObject> IntentClient::GetAbilityManagerRemote()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (remoteObj_ != nullptr) {
        return remoteObj_;
    }
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgr == nullptr) {
        TAG_LOGE(AAFwkLogTag::INTENT, "get samgr failed");
        return nullptr;
    }
    remoteObj_ = samgr->GetSystemAbility(ABILITY_MGR_SERVICE_ID);
    if (remoteObj_ == nullptr) {
        TAG_LOGE(AAFwkLogTag::INTENT, "get ability manager failed");
        return nullptr;
    }
    deathRecipient_ = sptr<IRemoteObject::DeathRecipient>(new DeathRecipient());
    if (deathRecipient_ != nullptr && remoteObj_->IsProxyObject()) {
        remoteObj_->AddDeathRecipient(deathRecipient_);
    }
    return remoteObj_;
}

void IntentClient::ResetRemote(wptr<IRemoteObject> remote)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (remoteObj_ == remote.promote()) {
        remoteObj_ = nullptr;
    }
}

void IntentClient::DeathRecipient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    TAG_LOGI(AAFwkLogTag::INTENT, "ability manager remote died");
    IntentClient::GetInstance().ResetRemote(remote);
}

int32_t IntentClient::SendExecuteRequest(uint32_t code, uint64_t key, const sptr<IRemoteObject> &callerToken,
    const AppExecFwk::FunctionCallExecuteParam &param)
{
    auto remote = GetAbilityManagerRemote();
    if (remote == nullptr) {
        TAG_LOGE(AAFwkLogTag::INTENT, "service not connected");
        return ABILITY_SERVICE_NOT_CONNECTED;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(Str8ToStr16(PERMISSION_DESC))) {
        TAG_LOGE(AAFwkLogTag::INTENT, "write interface token failed");
        return INNER_ERR;
    }
    if (!data.WriteUint64(key)) {
        TAG_LOGE(AAFwkLogTag::INTENT, "write key failed");
        return INNER_ERR;
    }
    if (!data.WriteRemoteObject(callerToken)) {
        TAG_LOGE(AAFwkLogTag::INTENT, "write callerToken failed");
        return INNER_ERR;
    }
    if (!data.WriteParcelable(&param)) {
        TAG_LOGE(AAFwkLogTag::INTENT, "write param failed");
        return INNER_ERR;
    }
    int32_t err = remote->SendRequest(code, data, reply, option);
    if (err != 0) {
        TAG_LOGE(AAFwkLogTag::INTENT, "send request failed: %{public}d", err);
        return err;
    }
    return reply.ReadInt32();
}

int32_t IntentClient::ExecuteIntentByFunctionCall(const ExecuteIntentParam &param)
{
    if (param.callback == nullptr) {
        TAG_LOGE(AAFwkLogTag::INTENT, "null callback");
        return ERR_INVALID_VALUE;
    }

    // Strip the caller ability token and the pre-registered execute record id from
    // the business WantParams; they are system parameters and must be carried as
    // dedicated IPC fields instead of leaking into the target Want.
    WantParams cleanedWant = param.wantParam;
    sptr<IRemoteObject> rawCallerToken = nullptr;
    sptr<IInterface> tokenValue = cleanedWant.GetParam(AppExecFwk::INSIGHT_INTENT_CALLER_TOKEN);
    if (tokenValue != nullptr) {
        AAFwk::IWantParams *iwp = AAFwk::IWantParams::Query(tokenValue);
        if (iwp != nullptr) {
            AAFwk::WantParams tokenWrap = AAFwk::WantParamWrapper::Unbox(iwp);
            sptr<IInterface> remoteObjVal = tokenWrap.GetParam(AAFwk::VALUE_PROPERTY);
            AAFwk::IRemoteObjectWrap *iRemoteObj = AAFwk::IRemoteObjectWrap::Query(remoteObjVal);
            rawCallerToken = AAFwk::RemoteObjectWrap::UnBox(iRemoteObj);
        }
    }
    cleanedWant.Remove(AppExecFwk::INSIGHT_INTENT_CALLER_TOKEN);

    int32_t intentId = 0;
    sptr<IInterface> recordIdValue = cleanedWant.GetParam(AppExecFwk::INSIGHT_INTENT_EXECUTE_RECORD_ID);
    if (recordIdValue != nullptr) {
        AAFwk::ILong *iLong = AAFwk::ILong::Query(recordIdValue);
        if (iLong != nullptr) {
            intentId = static_cast<int32_t>(AAFwk::Long::Unbox64(iLong));
        }
    }
    cleanedWant.Remove(AppExecFwk::INSIGHT_INTENT_EXECUTE_RECORD_ID);

    AppExecFwk::FunctionCallExecuteParam funcParam;
    funcParam.bundleName = param.bundleName;
    funcParam.intentName = param.intentName;
    funcParam.wantParam = cleanedWant;
    funcParam.callerAbilityToken = rawCallerToken;
    funcParam.intentId = intentId;

    auto hostClient = AbilityRuntime::InsightIntentHostClient::GetInstance();
    uint64_t key = hostClient->AddInsightIntentExecute(param.callback);

    int32_t err = SendExecuteRequest(
        static_cast<uint32_t>(AbilityManagerInterfaceCode::EXECUTE_INTENT_BY_FUNCTION_CALL),
        key, hostClient, funcParam);
    if (err != ERR_OK) {
        hostClient->RemoveInsightIntentExecute(key);
    }
    return err;
}

int32_t IntentClient::RemoveIntentRecord(int32_t intentId)
{
    TAG_LOGD(AAFwkLogTag::INTENT, "called, intentId: %{public}d", intentId);
    auto remote = GetAbilityManagerRemote();
    if (remote == nullptr) {
        TAG_LOGE(AAFwkLogTag::INTENT, "service not connected");
        return ABILITY_SERVICE_NOT_CONNECTED;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(Str8ToStr16(PERMISSION_DESC))) {
        TAG_LOGE(AAFwkLogTag::INTENT, "write interface token failed");
        return INNER_ERR;
    }
    if (!data.WriteInt32(intentId)) {
        TAG_LOGE(AAFwkLogTag::INTENT, "write intentId failed");
        return INNER_ERR;
    }
    int32_t err = remote->SendRequest(
        static_cast<uint32_t>(AbilityManagerInterfaceCode::REMOVE_INTENT_RECORD), data, reply, option);
    if (err != 0) {
        TAG_LOGE(AAFwkLogTag::INTENT, "send request failed: %{public}d", err);
        return err;
    }
    int32_t ret = reply.ReadInt32();
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkLogTag::INTENT, "remove execute record failed: %{public}d", ret);
    }
    return ret;
}

int32_t IntentClient::PreRegisterIntentRecord(const std::string &bundleName, const std::string &intentName,
    int32_t &intentId)
{
    intentId = 0;
    auto remote = GetAbilityManagerRemote();
    if (remote == nullptr) {
        TAG_LOGE(AAFwkLogTag::INTENT, "service not connected");
        return ABILITY_SERVICE_NOT_CONNECTED;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(Str8ToStr16(PERMISSION_DESC))) {
        TAG_LOGE(AAFwkLogTag::INTENT, "write interface token failed");
        return INNER_ERR;
    }
    if (!data.WriteString(bundleName)) {
        TAG_LOGE(AAFwkLogTag::INTENT, "write bundleName failed");
        return INNER_ERR;
    }
    if (!data.WriteString(intentName)) {
        TAG_LOGE(AAFwkLogTag::INTENT, "write intentName failed");
        return INNER_ERR;
    }
    int32_t err = remote->SendRequest(
        static_cast<uint32_t>(AbilityManagerInterfaceCode::PRE_REGISTER_INTENT_RECORD), data, reply, option);
    if (err != 0) {
        TAG_LOGE(AAFwkLogTag::INTENT, "send request failed: %{public}d", err);
        return err;
    }
    int32_t result = reply.ReadInt32();
    intentId = reply.ReadInt32();
    return result;
}

} // namespace AAFwk
} // namespace OHOS
