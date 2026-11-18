/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef SERVICES_INCLUDE_ECOLOGICAL_RULE_MANAGER_SERVICE_PROXY_H
#define SERVICES_INCLUDE_ECOLOGICAL_RULE_MANAGER_SERVICE_PROXY_H

#include <mutex>
#include "iremote_broker.h"
#include "iremote_object.h"
#include "iremote_proxy.h"
#include "ability_ecological_rule_mgr_service_interface.h"

namespace OHOS {
namespace EcologicalRuleMgrService {

using namespace std;
using Want = OHOS::AAFwk::Want;
using AbilityInfo = OHOS::AppExecFwk::AbilityInfo;

class AbilityEcologicalRuleMgrServiceClient : public RefBase {
public:
    DISALLOW_COPY_AND_MOVE(AbilityEcologicalRuleMgrServiceClient);
    static sptr<AbilityEcologicalRuleMgrServiceClient> GetInstance();
    void OnRemoteSaDied(const wptr<IRemoteObject> &object);

    int32_t EvaluateResolveInfos(const Want &want, const AbilityCallerInfo &callerInfo, int32_t type,
        vector<AbilityInfo> &abInfo);
    int32_t QueryStartExperience(const Want &want, const AbilityCallerInfo &callerInfo, AbilityExperienceRule &rule);

private:
    AbilityEcologicalRuleMgrServiceClient() {};
    ~AbilityEcologicalRuleMgrServiceClient();
    static sptr<IAbilityEcologicalRuleMgrService> ConnectService();
    static bool CheckConnectService();

    static mutex instanceLock_;
    static mutex proxyLock_;
    static sptr<AbilityEcologicalRuleMgrServiceClient> instance_;
    static sptr<IAbilityEcologicalRuleMgrService> ecologicalRuleMgrServiceProxy_;
    static sptr<IRemoteObject::DeathRecipient> deathRecipient_;

    static string ERMS_ORIGINAL_TARGET;
};

class AbilityEcologicalRuleMgrServiceDeathRecipient : public IRemoteObject::DeathRecipient {
public:
    AbilityEcologicalRuleMgrServiceDeathRecipient() {};
    ~AbilityEcologicalRuleMgrServiceDeathRecipient() = default;
    void OnRemoteDied(const wptr<IRemoteObject> &object) override;

private:
    DISALLOW_COPY_AND_MOVE(AbilityEcologicalRuleMgrServiceDeathRecipient);
};

class AbilityEcologicalRuleMgrServiceProxy : public IRemoteProxy<IAbilityEcologicalRuleMgrService> {
public:
    explicit AbilityEcologicalRuleMgrServiceProxy(const sptr<IRemoteObject>& impl);
    virtual ~AbilityEcologicalRuleMgrServiceProxy() = default;

    int32_t EvaluateResolveInfos(const Want &want, const AbilityCallerInfo &callerInfo, int32_t type,
        vector<AbilityInfo> &abilityInfo) override;
    int32_t QueryStartExperience(const Want &want, const AbilityCallerInfo &callerInfo,
        AbilityExperienceRule &rule) override;

private:
    template <typename T> bool ReadParcelableVector(vector<T> &parcelableVector, MessageParcel &reply);
    static inline BrokerDelegator<AbilityEcologicalRuleMgrServiceProxy> delegator_;
};
} // namespace EcologicalRuleMgrService
} // namespace OHOS

#endif // SERVICES_INCLUDE_ECOLOGICAL_RULE_MANAGER_SERVICE_PROXY_H
