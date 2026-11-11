/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "ecological_rule/ability_ecological_rule_mgr_service.h"

#include "ability_manager_errors.h"
#include "iservice_registry.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "ipc_capacity_wrap.h"
#include "record_cost_time_util.h"

namespace OHOS {
using AAFwk::RecordCostTimeUtil;
namespace EcologicalRuleMgrService {

using namespace std::chrono;

static inline const std::u16string ERMS_INTERFACE_TOKEN =
    u"ohos.cloud.ecologicalrulemgrservice.IEcologicalRuleMgrService";
constexpr int32_t CYCLE_LIMIT = 1000;
const int32_t ECOLOGICALRULEMANAGERSERVICE_ID = 6105;

std::mutex AbilityEcologicalRuleMgrServiceClient::instanceLock_;
std::mutex AbilityEcologicalRuleMgrServiceClient::proxyLock_;
sptr<AbilityEcologicalRuleMgrServiceClient> AbilityEcologicalRuleMgrServiceClient::instance_;
sptr<IAbilityEcologicalRuleMgrService> AbilityEcologicalRuleMgrServiceClient::ecologicalRuleMgrServiceProxy_;
sptr<IRemoteObject::DeathRecipient> AbilityEcologicalRuleMgrServiceClient::deathRecipient_;

std::string AbilityEcologicalRuleMgrServiceClient::ERMS_ORIGINAL_TARGET = "ecological_experience_original_target";

inline int64_t GetCurrentTimeMicro()
{
    return duration_cast<microseconds>(system_clock::now().time_since_epoch()).count();
}

AbilityEcologicalRuleMgrServiceClient::~AbilityEcologicalRuleMgrServiceClient()
{
    std::lock_guard<std::mutex> autoLock(proxyLock_);
    if (ecologicalRuleMgrServiceProxy_ != nullptr) {
        auto remoteObj = ecologicalRuleMgrServiceProxy_->AsObject();
        if (remoteObj != nullptr) {
            remoteObj->RemoveDeathRecipient(deathRecipient_);
        }
    }
}

sptr<AbilityEcologicalRuleMgrServiceClient> AbilityEcologicalRuleMgrServiceClient::GetInstance()
{
    if (instance_ == nullptr) {
        std::lock_guard<std::mutex> autoLock(instanceLock_);
        if (instance_ == nullptr) {
            instance_ = new AbilityEcologicalRuleMgrServiceClient;
        }
    }
    return instance_;
}

sptr<IAbilityEcologicalRuleMgrService> AbilityEcologicalRuleMgrServiceClient::ConnectService()
{
    sptr<ISystemAbilityManager> samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgr == nullptr) {
        TAG_LOGE(AAFwkTag::ECOLOGICAL_RULE, "null samgr");
        return nullptr;
    }

    auto systemAbility = samgr->CheckSystemAbility(ECOLOGICALRULEMANAGERSERVICE_ID);
    if (systemAbility == nullptr) {
        TAG_LOGE(AAFwkTag::ECOLOGICAL_RULE, "null systemAbility");
        return nullptr;
    }

    deathRecipient_ = new AbilityEcologicalRuleMgrServiceDeathRecipient();
    systemAbility->AddDeathRecipient(deathRecipient_);

    sptr<IAbilityEcologicalRuleMgrService> service = iface_cast<IAbilityEcologicalRuleMgrService>(systemAbility);
    if (service == nullptr) {
        TAG_LOGD(AAFwkTag::ECOLOGICAL_RULE, "erms transfered to foundation");
        service = new AbilityEcologicalRuleMgrServiceProxy(systemAbility);
    }
    return service;
}

bool AbilityEcologicalRuleMgrServiceClient::CheckConnectService()
{
    if (ecologicalRuleMgrServiceProxy_ == nullptr) {
        TAG_LOGW(AAFwkTag::ECOLOGICAL_RULE, "redo ConnectService");
        ecologicalRuleMgrServiceProxy_ = ConnectService();
    }
    if (ecologicalRuleMgrServiceProxy_ == nullptr) {
        TAG_LOGE(AAFwkTag::ECOLOGICAL_RULE, "Connect SA Failed");
        return false;
    }
    return true;
}

void AbilityEcologicalRuleMgrServiceClient::OnRemoteSaDied(const wptr<IRemoteObject> &object)
{
    std::lock_guard<std::mutex> autoLock(proxyLock_);
    ecologicalRuleMgrServiceProxy_ = ConnectService();
}

int32_t AbilityEcologicalRuleMgrServiceClient::EvaluateResolveInfos(const AAFwk::Want &want,
    const AbilityCallerInfo &callerInfo, int32_t type, vector<AbilityInfo> &abilityInfos,
    const vector<AppExecFwk::ExtensionAbilityInfo> &extInfos)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    RecordCostTimeUtil timeRecord("EvaluateResolveInfos");
    TAG_LOGD(AAFwkTag::ECOLOGICAL_RULE, "want: %{private}s, callerInfo: %{public}s, type: %{public}d",
        want.ToString().c_str(), callerInfo.ToString().c_str(), type);
    sptr<IAbilityEcologicalRuleMgrService> proxy;
    {
        std::lock_guard<std::mutex> autoLock(proxyLock_);
        if (!CheckConnectService()) {
            return AAFwk::ERR_CONNECT_ERMS_FAILED;
        }
        proxy = ecologicalRuleMgrServiceProxy_;
    }
    return proxy->EvaluateResolveInfos(want, callerInfo, type, abilityInfos);
}

int32_t AbilityEcologicalRuleMgrServiceClient::QueryStartExperience(const OHOS::AAFwk::Want &want,
    const AbilityCallerInfo &callerInfo, AbilityExperienceRule &rule)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    RecordCostTimeUtil timeRecord("QueryStartExperience");
    TAG_LOGD(AAFwkTag::ECOLOGICAL_RULE, "callerInfo: %{public}s, want: %{private}s", callerInfo.ToString().c_str(),
        want.ToString().c_str());

    sptr<IAbilityEcologicalRuleMgrService> proxy;
    {
        std::lock_guard<std::mutex> autoLock(proxyLock_);
        if (!CheckConnectService()) {
            return AAFwk::ERR_CONNECT_ERMS_FAILED;
        }
        proxy = ecologicalRuleMgrServiceProxy_;
    }
    int32_t res = proxy->QueryStartExperience(want, callerInfo, rule);
    if (rule.replaceWant != nullptr) {
        rule.replaceWant->SetParam(ERMS_ORIGINAL_TARGET, want.ToString());
        TAG_LOGD(AAFwkTag::ECOLOGICAL_RULE,
            "queryStart finish: resultCode = %{public}d, sceneCode = %{public}s, replaceWant = %{private}s",
            rule.resultCode, rule.sceneCode.c_str(), (*(rule.replaceWant)).ToString().c_str());
    }
    return res;
}

void AbilityEcologicalRuleMgrServiceDeathRecipient::OnRemoteDied(const wptr<IRemoteObject> &object)
{
    if (AbilityEcologicalRuleMgrServiceClient::GetInstance()) {
        AbilityEcologicalRuleMgrServiceClient::GetInstance()->OnRemoteSaDied(object);
    }
}

AbilityEcologicalRuleMgrServiceProxy::AbilityEcologicalRuleMgrServiceProxy(
    const sptr<IRemoteObject>& impl) : IRemoteProxy<IAbilityEcologicalRuleMgrService>(impl)
{}

int32_t AbilityEcologicalRuleMgrServiceProxy::EvaluateResolveInfos(const Want &want,
    const AbilityCallerInfo &callerInfo, int32_t type, std::vector<AbilityInfo> &abilityInfos)
{
    TAG_LOGD(AAFwkTag::ECOLOGICAL_RULE, "called");
    MessageParcel data;
    AAFwk::ExtendMaxIpcCapacityForInnerWant(data);

    if (!data.WriteInterfaceToken(ERMS_INTERFACE_TOKEN)) {
        TAG_LOGE(AAFwkTag::ECOLOGICAL_RULE, "write token failed");
        return ERR_FAILED;
    }

    if (!data.WriteParcelable(&want)) {
        TAG_LOGE(AAFwkTag::ECOLOGICAL_RULE, "write want failed");
        return ERR_FAILED;
    }

    if (!data.WriteInt32(type)) {
        TAG_LOGE(AAFwkTag::ECOLOGICAL_RULE, "write type failed");
        return ERR_FAILED;
    }

    if (!data.WriteInt32(abilityInfos.size())) {
        TAG_LOGE(AAFwkTag::ECOLOGICAL_RULE, "write abilityInfos size failed");
        return ERR_FAILED;
    }

    for (auto &abilityInfo : abilityInfos) {
        if (!data.WriteParcelable(&abilityInfo)) {
            TAG_LOGE(AAFwkTag::ECOLOGICAL_RULE, "write abilityInfo failed");
            return ERR_FAILED;
        }
    }

    if (!data.WriteParcelable(&callerInfo)) {
        TAG_LOGE(AAFwkTag::ECOLOGICAL_RULE, "write callerInfo failed");
        return ERR_FAILED;
    }

    MessageOption option = { MessageOption::TF_SYNC };
    MessageParcel reply;

    auto remote = Remote();
    if (remote == nullptr) {
        TAG_LOGE(AAFwkTag::ECOLOGICAL_RULE, "null remote");
        return ERR_FAILED;
    }

    int32_t ret = remote->SendRequest(EVALUATE_RESOLVE_INFO_CMD, data, reply, option);
    if (ret != ERR_NONE) {
        TAG_LOGE(AAFwkTag::ECOLOGICAL_RULE, "SendRequest error:%{public}d", ret);
        return ERR_FAILED;
    }

    if (!ReadParcelableVector(abilityInfos, reply)) {
        TAG_LOGE(AAFwkTag::ECOLOGICAL_RULE, "GetParcelableInfos fail");
    }
    TAG_LOGD(AAFwkTag::ECOLOGICAL_RULE, "end");
    return ERR_OK;
}

template <typename T>
bool AbilityEcologicalRuleMgrServiceProxy::ReadParcelableVector(std::vector<T> &parcelableVector, MessageParcel &reply)
{
    int32_t infoSize = reply.ReadInt32();
    if (infoSize > CYCLE_LIMIT) {
        TAG_LOGE(AAFwkTag::ECOLOGICAL_RULE, "size too large");
        return false;
    }
    parcelableVector.clear();
    for (int32_t i = 0; i < infoSize; i++) {
        sptr<T> info = reply.ReadParcelable<T>();
        if (info == nullptr) {
            TAG_LOGE(AAFwkTag::ECOLOGICAL_RULE, "null info");
            return false;
        }
        parcelableVector.emplace_back(*info);
    }
    return true;
}

int32_t AbilityEcologicalRuleMgrServiceProxy::QueryStartExperience(const Want &want,
    const AbilityCallerInfo &callerInfo, AbilityExperienceRule &rule)
{
    TAG_LOGD(AAFwkTag::ECOLOGICAL_RULE, "called");
    MessageParcel data;
    AAFwk::ExtendMaxIpcCapacityForInnerWant(data);
    if (!data.WriteInterfaceToken(ERMS_INTERFACE_TOKEN)) {
        TAG_LOGE(AAFwkTag::ECOLOGICAL_RULE, "write token failed");
        return ERR_FAILED;
    }

    if (!data.WriteParcelable(&want)) {
        TAG_LOGE(AAFwkTag::ECOLOGICAL_RULE, "write want failed");
        return ERR_FAILED;
    }

    if (!data.WriteParcelable(&callerInfo)) {
        TAG_LOGE(AAFwkTag::ECOLOGICAL_RULE, "write callerInfo failed");
        return ERR_FAILED;
    }

    MessageOption option = { MessageOption::TF_SYNC };
    MessageParcel reply;

    auto remote = Remote();
    if (remote == nullptr) {
        TAG_LOGE(AAFwkTag::ECOLOGICAL_RULE, "null remote");
        return ERR_FAILED;
    }

    int32_t ret = remote->SendRequest(QUERY_START_EXPERIENCE_CMD, data, reply, option);
    if (ret != ERR_NONE) {
        TAG_LOGE(AAFwkTag::ECOLOGICAL_RULE, "SendRequest error: %{public}d", ret);
        return ERR_FAILED;
    }

    sptr<AbilityExperienceRule> sptrRule = reply.ReadParcelable<AbilityExperienceRule>();
    if (sptrRule == nullptr) {
        TAG_LOGE(AAFwkTag::ECOLOGICAL_RULE, "null sptrRule");
        return ERR_FAILED;
    }

    rule = *sptrRule;
    TAG_LOGD(AAFwkTag::ECOLOGICAL_RULE, "end");
    return ERR_OK;
}
} // namespace EcologicalRuleMgrService
} // namespace OHOS
