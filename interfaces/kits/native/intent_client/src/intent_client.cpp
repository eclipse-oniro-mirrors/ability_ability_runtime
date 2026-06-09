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
#include "system_ability_definition.h"

#include "ability_manager_errors.h"
#include "ability_manager_ipc_interface_code.h"

namespace OHOS {
namespace AAFwk {

namespace {
constexpr const char *PERMISSION_DESC = "ohos.aafwk.AbilityManager";

std::u16string ToUtf16(const std::string &str)
{
    return std::u16string(str.begin(), str.end());
}
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

int32_t IntentClient::SendExecuteRequest(uint64_t key, const sptr<IRemoteObject> &callerToken,
    const std::string &bundleName, const std::string &intentName, const WantParams &wantParam)
{
    auto remote = GetAbilityManagerRemote();
    if (remote == nullptr) {
        TAG_LOGE(AAFwkLogTag::INTENT, "service not connected");
        return ABILITY_SERVICE_NOT_CONNECTED;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(ToUtf16(PERMISSION_DESC))) {
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
    if (!data.WriteString(bundleName)) {
        TAG_LOGE(AAFwkLogTag::INTENT, "write bundleName failed");
        return INNER_ERR;
    }
    if (!data.WriteString(intentName)) {
        TAG_LOGE(AAFwkLogTag::INTENT, "write intentName failed");
        return INNER_ERR;
    }
    if (!data.WriteParcelable(&wantParam)) {
        TAG_LOGE(AAFwkLogTag::INTENT, "write wantParam failed");
        return INNER_ERR;
    }
    int32_t err = remote->SendRequest(
        static_cast<uint32_t>(AbilityManagerInterfaceCode::EXECUTE_INTENT_BY_FUNCTION_CALL),
        data, reply, option);
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

    auto hostClient = AbilityRuntime::InsightIntentHostClient::GetInstance();
    uint64_t key = hostClient->AddInsightIntentExecute(param.callback);

    int32_t err = SendExecuteRequest(key, hostClient, param.bundleName, param.intentName, param.wantParam);
    if (err != ERR_OK) {
        hostClient->RemoveInsightIntentExecute(key);
    }
    return err;
}

} // namespace AAFwk
} // namespace OHOS
