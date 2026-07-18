/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "ability_cache_manager.h"

#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AAFwk {
const std::string FRS_APP_INDEX = "ohos.extra.param.key.frs_index";
const std::string FRS_BUNDLE_NAME = "com.ohos.formrenderservice";

AbilityCacheManager::AbilityCacheManager() {}

AbilityCacheManager::~AbilityCacheManager() {}

AbilityCacheManager &AbilityCacheManager::GetInstance()
{
    static AbilityCacheManager abilityRecMgr;
    return abilityRecMgr;
}

void AbilityCacheManager::Init(uint32_t devCapacity, uint32_t procCapacity)
{
    devLruCapacity_ = devCapacity;
    procLruCapacity_ = procCapacity;
}

void AbilityCacheManager::RemoveAbilityRecInDevList(std::shared_ptr<BaseExtensionRecord> abilityRecord)
{
    if (abilityRecord == nullptr) {
        return;
    }
    auto it = devRecLru_.begin();
    uint32_t accessTokenId = abilityRecord->GetApplicationInfo().accessTokenId;
    while (it != devRecLru_.end()) {
        if ((*it) && (*it)->GetRecordId() == abilityRecord->GetRecordId()) {
            devRecLru_.erase(it);
            devLruCnt_--;
            return;
        } else {
            it++;
        }
    }
}

void AbilityCacheManager::RemoveAbilityRecInProcList(std::shared_ptr<BaseExtensionRecord> abilityRecord)
{
    if (abilityRecord == nullptr) {
        return;
    }
    uint32_t accessTokenId = abilityRecord->GetApplicationInfo().accessTokenId;
    auto findProcInfo = procLruMap_.find(accessTokenId);
    if (findProcInfo == procLruMap_.end()) {
        TAG_LOGD(AAFwkTag::SERVICE_EXT, "no record");
        return;
    }
    auto it = findProcInfo->second.recList.begin();

    while (it != findProcInfo->second.recList.end()) {
        if ((*it) && (*it)->GetRecordId() == abilityRecord->GetRecordId()) {
            findProcInfo->second.recList.erase(it);
            findProcInfo->second.cnt--;
            if (findProcInfo->second.cnt == 0) {
                procLruMap_.erase(findProcInfo);
            }
            return;
        } else {
            it++;
        }
    }
}

std::shared_ptr<BaseExtensionRecord> AbilityCacheManager::AddToProcLru(
    std::shared_ptr<BaseExtensionRecord> abilityRecord)
{
    if (abilityRecord == nullptr) {
        return nullptr;
    }
    auto findProcInfo = procLruMap_.find(abilityRecord->GetApplicationInfo().accessTokenId);
    if (findProcInfo == procLruMap_.end()) {
        std::list<std::shared_ptr<BaseExtensionRecord>> recList;
        ProcRecordsInfo procRecInfo = {recList, 1};
        procRecInfo.recList.push_back(abilityRecord);
        procLruMap_[abilityRecord->GetApplicationInfo().accessTokenId] = procRecInfo;
        return nullptr;
    }
    if (findProcInfo->second.cnt == procLruCapacity_) {
        RemoveAbilityRecInDevList(findProcInfo->second.recList.front());
        std::shared_ptr<BaseExtensionRecord> rec = findProcInfo->second.recList.front();
        findProcInfo->second.recList.pop_front();
        findProcInfo->second.recList.push_back(abilityRecord);
        return rec;
    }
    findProcInfo->second.cnt++;
    findProcInfo->second.recList.push_back(abilityRecord);
    return nullptr;
}

std::shared_ptr<BaseExtensionRecord> AbilityCacheManager::AddToDevLru(
    std::shared_ptr<BaseExtensionRecord> abilityRecord, std::shared_ptr<BaseExtensionRecord> rec)
{
    if (rec != nullptr) {
        devRecLru_.push_back(abilityRecord);
        devLruCnt_++;
        return rec;
    }
    if (devLruCnt_ == devLruCapacity_ && devLruCnt_ > 0) {
        rec = devRecLru_.front();
        RemoveAbilityRecInProcList(rec);
        devRecLru_.pop_front();
        devLruCnt_--;
    }
    devRecLru_.push_back(abilityRecord);
    devLruCnt_++;
    return rec;
}

std::shared_ptr<BaseExtensionRecord> AbilityCacheManager::Put(std::shared_ptr<BaseExtensionRecord> abilityRecord)
{
    if (abilityRecord == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null abilityRecord");
        return nullptr;
    }
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "Put the ability to lru, service:%{public}s, extension type %{public}d",
        abilityRecord->GetURI().c_str(), abilityRecord->GetAbilityInfo().extensionAbilityType);
    std::lock_guard<std::mutex> lock(mutex_);
    std::shared_ptr<BaseExtensionRecord> rec = AddToProcLru(abilityRecord);
    return AddToDevLru(abilityRecord, rec);
}

void AbilityCacheManager::Remove(std::shared_ptr<BaseExtensionRecord> abilityRecord)
{
    if (abilityRecord == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null abilityRecord");
        return;
    }
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "Remove the ability from lru, service:%{public}s, extension type %{public}d",
        abilityRecord->GetURI().c_str(), abilityRecord->GetAbilityInfo().extensionAbilityType);
    std::lock_guard<std::mutex> lock(mutex_);
    RemoveAbilityRecInProcList(abilityRecord);
    RemoveAbilityRecInDevList(abilityRecord);
}

bool AbilityCacheManager::IsRecInfoSame(const AbilityRequest& abilityRequest,
    std::shared_ptr<BaseExtensionRecord> abilityRecord)
{
    return abilityRecord != nullptr &&
        abilityRequest.abilityInfo.moduleName == abilityRecord->GetAbilityInfo().moduleName &&
        abilityRequest.want.GetAbilityNameRef() == abilityRecord->GetAbilityName();
}

std::shared_ptr<BaseExtensionRecord> AbilityCacheManager::GetAbilityRecInProcList(const AbilityRequest &abilityRequest)
{
    auto findProcInfo = procLruMap_.find(abilityRequest.appInfo.accessTokenId);
    if (findProcInfo == procLruMap_.end()) {
        TAG_LOGD(AAFwkTag::SERVICE_EXT, "can't found bundleName");
        return nullptr;
    }
    ProcRecordsInfo &procRecordsInfo = findProcInfo->second;
    auto recIter = procRecordsInfo.recList.begin();
    while (recIter != procRecordsInfo.recList.end()) {
        if (IsRecInfoSame(abilityRequest, *recIter)) {
            std::shared_ptr<BaseExtensionRecord> abilityRecord = *recIter;
            procRecordsInfo.recList.erase(recIter);
            procRecordsInfo.cnt--;
            return abilityRecord;
        }
        recIter++;
    }
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "Can't found the abilityRecord in process list for get.");
    return nullptr;
}

std::shared_ptr<BaseExtensionRecord> AbilityCacheManager::Get(const AbilityRequest& abilityRequest)
{
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "Get the ability from lru, service:%{public}s, extension type %{public}d",
        abilityRequest.abilityInfo.uri.c_str(), abilityRequest.abilityInfo.extensionAbilityType);
    std::lock_guard<std::mutex> lock(mutex_);
    std::shared_ptr<BaseExtensionRecord> abilityRecord = GetAbilityRecInProcList(abilityRequest);
    if (abilityRecord == nullptr) {
        TAG_LOGD(AAFwkTag::SERVICE_EXT, "Can't found the abilityRecord for get.");
        return nullptr;
    }
    RemoveAbilityRecInDevList(abilityRecord);
    return abilityRecord;
}

std::shared_ptr<BaseExtensionRecord> AbilityCacheManager::FindRecordByToken(const sptr<IRemoteObject> &token)
{
    if (token == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null token");
        return nullptr;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = devRecLru_.begin();
    while (it != devRecLru_.end()) {
        if (*it) {
            sptr<IRemoteObject> srcToken = (*it)->GetToken();
            if (srcToken == token) {
                std::shared_ptr<BaseExtensionRecord> &abilityRecord = *it;
                TAG_LOGD(AAFwkTag::SERVICE_EXT,
                    "Find the ability by token from lru, service:%{public}s, extension type %{public}d",
                    abilityRecord->GetURI().c_str(), abilityRecord->GetAbilityInfo().extensionAbilityType);
                return abilityRecord;
            }
        }
        it++;
    }
    return nullptr;
}

std::list<std::shared_ptr<BaseExtensionRecord>> AbilityCacheManager::GetAbilityList()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return devRecLru_;
}

std::shared_ptr<BaseExtensionRecord> AbilityCacheManager::FindRecordBySessionId(const std::string &assertSessionId)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = devRecLru_.begin();
    while (it != devRecLru_.end()) {
        if (!*it) {
            it++;
            continue;
        }
        auto assertSessionStr = (*it)->GetStringParam(Want::PARAM_ASSERT_FAULT_SESSION_ID);
        if (assertSessionStr == assertSessionId) {
            std::shared_ptr<BaseExtensionRecord> &abilityRecord = *it;
            TAG_LOGD(AAFwkTag::SERVICE_EXT,
                "Find the ability by sessionId from lru, service:%{public}s, extension type %{public}d",
                abilityRecord->GetURI().c_str(), abilityRecord->GetAbilityInfo().extensionAbilityType);
            return abilityRecord;
        } else {
            it++;
        }
    }
    return nullptr;
}

std::shared_ptr<BaseExtensionRecord> AbilityCacheManager::FindRecordByServiceKey(const std::string &serviceKey)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = devRecLru_.begin();
    while (it != devRecLru_.end()) {
        if (!*it) {
            it++;
            continue;
        }
        std::string curServiceKey = (*it)->GetURI();
        if (FRS_BUNDLE_NAME == (*it)->GetAbilityInfo().bundleName) {
            curServiceKey = curServiceKey + std::to_string((*it)->GetIntParam(FRS_APP_INDEX, 0));
        }
        if (curServiceKey.compare(serviceKey) == 0) {
            std::shared_ptr<BaseExtensionRecord> &abilityRecord = *it;
            TAG_LOGD(AAFwkTag::SERVICE_EXT,
                "Find the ability by serviceKey from lru, service:%{public}s, extension type %{public}d",
                abilityRecord->GetURI().c_str(), abilityRecord->GetAbilityInfo().extensionAbilityType);
            return abilityRecord;
        } else {
            it++;
        }
    }
    return nullptr;
}

void AbilityCacheManager::RemoveLauncherDeathRecipient()
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = devRecLru_.begin();
    while (it != devRecLru_.end()) {
        auto targetExtension = *it;
        if (targetExtension != nullptr && targetExtension->GetAbilityInfo().type == AbilityType::EXTENSION &&
            ((targetExtension->GetAbilityInfo().name == AbilityConfig::LAUNCHER_ABILITY_NAME &&
            targetExtension->GetAbilityInfo().bundleName == AbilityConfig::LAUNCHER_BUNDLE_NAME) ||
            targetExtension->IsSceneBoard())) {
            targetExtension->RemoveAbilityDeathRecipient();
            return;
        }
        it++;
    }
}

void AbilityCacheManager::SignRestartAppFlag(int32_t uid, const std::string &instanceKey)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = devRecLru_.begin();
    while (it != devRecLru_.end()) {
        auto abilityRecord = *it;
        if (abilityRecord != nullptr && abilityRecord->GetUid() == uid &&
            abilityRecord->GetInstanceKey() == instanceKey) {
            abilityRecord->SetRestartAppFlag(true);
        }
        it++;
    }
}

void AbilityCacheManager::SignRestartProcess(int32_t pid)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = devRecLru_.begin();
    while (it != devRecLru_.end()) {
        auto abilityRecord = *it;
        if (abilityRecord != nullptr && abilityRecord->GetPid() == pid) {
            abilityRecord->SetRestartAppFlag(true);
        }
        it++;
    }
}

void AbilityCacheManager::DeleteInvalidServiceRecord(const std::string &bundleName)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = devRecLru_.begin();
    while (it != devRecLru_.end()) {
        auto abilityRecord = *it;
        if (abilityRecord != nullptr && abilityRecord->GetApplicationInfo().bundleName == bundleName) {
            RemoveAbilityRecInProcList(abilityRecord);
            RemoveAbilityRecInDevList(abilityRecord);
        }
        it++;
    }
}
}  // namespace AAFwk
} // namespace OHOS