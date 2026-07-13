/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#include "sa_interceptor_manager.h"

#include <random>

#include "ability_manager_errors.h"
#include "ability_manager_service.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"

namespace OHOS {
namespace AbilityRuntime {
constexpr int32_t BASE_USER_RANGE = 200000;

SAInterceptorManager::SAInterceptorManager()
{}

SAInterceptorManager::~SAInterceptorManager()
{}

SAInterceptorManager &SAInterceptorManager::GetInstance()
{
    static SAInterceptorManager manager;
    return manager;
}

int32_t SAInterceptorManager::AddSAInterceptor(sptr<ISAInterceptor> interceptor)
{
    if (interceptor == nullptr) {
        TAG_LOGE(AAFwkTag::SA_INTERCEPTOR, "null interceptor");
        return AAFwk::ERR_NULL_SA_INTERCEPTOR_EXECUTER;
    }

    if (ObserverExist(interceptor)) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "interceptor exist");
        return ERR_OK;
    }

    {
        std::lock_guard<std::mutex> lock(saInterceptorLock_);
        saInterceptors_.emplace_back(interceptor);
        if (!deathRecipient_) {
            // add death recipient
            deathRecipient_ = new SAInterceptorRecipient([](const wptr<IRemoteObject> &remote) {
                SAInterceptorManager::GetInstance().OnObserverDied(remote);
            });
        }
    }

    auto observerObj = interceptor->AsObject();
    if (!observerObj || !observerObj->AddDeathRecipient(deathRecipient_)) {
        TAG_LOGE(AAFwkTag::SA_INTERCEPTOR, "AddDeathRecipient failed");
    }

    return ERR_OK;
}

int32_t SAInterceptorManager::RemoveSAInterceptor(sptr<ISAInterceptor> interceptor)
{
    if (interceptor == nullptr) {
        TAG_LOGE(AAFwkTag::SA_INTERCEPTOR, "null interceptor");
        return AAFwk::ERR_NULL_SA_INTERCEPTOR_EXECUTER;
    }

    std::lock_guard<std::mutex> lock(saInterceptorLock_);
    for (auto interceptorIter = saInterceptors_.begin(); interceptorIter != saInterceptors_.end(); interceptorIter++) {
        if (*interceptorIter && (*interceptorIter)->AsObject() == interceptor) {
            if (interceptor && deathRecipient_) {
                interceptor->RemoveDeathRecipient(deathRecipient_);
            }
            saInterceptors_.erase(interceptorIter);
            TAG_LOGI(AAFwkTag::ABILITYMGR, "interceptor removed");
            return ERR_OK;
        }
    }
    TAG_LOGE(AAFwkTag::ABILITYMGR, "interceptor not found");
    return ERR_OK;
}

bool SAInterceptorManager::SAInterceptorListIsEmpty()
{
    std::lock_guard<std::mutex> lock(saInterceptorLock_);
    return saInterceptors_.empty();
}

int32_t SAInterceptorManager::ExecuteSAInterceptor(const std::string &params, Rule &rule)
{
    TAG_LOGI(AAFwkTag::SA_INTERCEPTOR, "call ExecuteSAInterceptor");
    std::vector<sptr<ISAInterceptor>> tempSaInterceptors;
    {
        std::lock_guard<std::mutex> lock(saInterceptorLock_);
        tempSaInterceptors = saInterceptors_;
    }
    for (auto &interceptor : tempSaInterceptors) {
        if (interceptor == nullptr) {
            continue;
        }
        HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
        auto result = interceptor->OnCheckStarting(params, rule);
        if (result != ERR_OK || rule.type != RuleType::ALLOW) {
            TAG_LOGW(AAFwkTag::SA_INTERCEPTOR, "OnCheckStarting error: %{public}d", result);
            return result;
        }
    }
    return ERR_OK;
}

void SAInterceptorManager::OnObserverDied(const wptr<IRemoteObject> &remote)
{
    TAG_LOGI(AAFwkTag::SA_INTERCEPTOR, "call OnObserverDied");
    auto remoteObj = remote.promote();
    if (remoteObj == nullptr) {
        TAG_LOGE(AAFwkTag::SA_INTERCEPTOR, "null remote");
        return;
    }
    std::lock_guard<std::mutex> lock(saInterceptorLock_);
    for (auto interceptorIter = saInterceptors_.begin(); interceptorIter != saInterceptors_.end(); interceptorIter++) {
        if (*interceptorIter && (*interceptorIter)->AsObject() == remoteObj) {
            saInterceptors_.erase(interceptorIter);
            return;
        }
    }
}

bool SAInterceptorManager::ObserverExist(sptr<IRemoteBroker> observer)
{
    if (observer == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null observer");
        return false;
    }
    std::lock_guard<std::mutex> lockRegister(saInterceptorLock_);
    for (auto it = saInterceptors_.begin(); it != saInterceptors_.end(); ++it) {
        if ((*it)->AsObject() == observer->AsObject()) {
            return true;
        }
    }
    return false;
}

std::string SAInterceptorManager::GenerateSAInterceptorParams(const AAFwk::Want &want, sptr<IRemoteObject> callerToken,
    const AppExecFwk::AbilityInfo &abilityInfo, const std::string &dialogSessionId)
{
    nlohmann::json jsonObj;
    auto abilityRecord = AAFwk::Token::GetAbilityRecordByToken(callerToken);
    if (abilityRecord) {
        const auto &callerAbilityInfo = abilityRecord->GetAbilityInfo();
        jsonObj["callerBundleName"] = callerAbilityInfo.bundleName;
        jsonObj["callerModuleName"] = callerAbilityInfo.moduleName;
        jsonObj["callerAbilityName"] = callerAbilityInfo.name;
    } else {
        jsonObj["callerBundleName"] = "";
        jsonObj["callerModuleName"] = "";
        jsonObj["callerAbilityName"] = "";
    }
    jsonObj["callerUid"] = IPCSkeleton::GetCallingUid();
    jsonObj["callerAppState"] = abilityRecord ? static_cast<int32_t>(abilityRecord->GetAppState()) : 0;
    jsonObj["callerUserId"] = IPCSkeleton::GetCallingUid() / BASE_USER_RANGE;

    jsonObj["targetBundleName"] = abilityInfo.bundleName;
    jsonObj["targetModuleName"] = abilityInfo.moduleName;
    jsonObj["targetAbilityName"] = abilityInfo.name;
    jsonObj["targetUid"] = abilityInfo.applicationInfo.uid;
    jsonObj["targetAbilityType"] = abilityInfo.type;
    jsonObj["targetAppIndex"] = abilityInfo.appIndex;
    jsonObj["targetExtensionAbilityType"] = abilityInfo.extensionAbilityType;
    jsonObj["linking"] = want.GetUriString();
    jsonObj["dialogSessionId"] = dialogSessionId;
    return jsonObj.dump();
}

SAInterceptorRecipient::SAInterceptorRecipient(RemoteDiedHandler handler) : handler_(handler)
{}

SAInterceptorRecipient::~SAInterceptorRecipient()
{}

void SAInterceptorRecipient::OnRemoteDied(const wptr<IRemoteObject> &__attribute__((unused)) remote)
{
    TAG_LOGI(AAFwkTag::SA_INTERCEPTOR, "call OnRemoteDied");
    if (handler_) {
        handler_(remote);
    }
}
} // namespace AbilityRuntime
} // namespace OHOS