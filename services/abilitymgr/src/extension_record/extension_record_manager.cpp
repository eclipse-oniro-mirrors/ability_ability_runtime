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

#include "extension_record_manager.h"

#include "ability_util.h"
#include "app_utils.h"
#include "preload_ui_extension_execute_callback_proxy.h"
#include "preload_ui_extension_host_client.h"
#include "ui_extension_record.h"
#include "ui_extension_record_factory.h"
#include "ui_extension_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr const char *SEPARATOR = ":";
constexpr const char *EMBEDDEDUI = "embeddedUI";
const std::string IS_PRELOAD_UIEXTENSION_ABILITY = "ability.want.params.is_preload_uiextension_ability";
constexpr const char* UIEXTENSION_TYPE_KEY = "ability.want.params.uiExtensionType";
constexpr size_t HOST_PID_INDEX = 3;
constexpr const char* AGENT_EXTENSION_TYPE = "agent";
constexpr int32_t AGENT_UI_MAX_COUNT = 5;
int32_t GetAppIndexByRecord(std::shared_ptr<AAFwk::AbilityRecord> abilityRecord,
    std::shared_ptr<AAFwk::AbilityRecord> callerRecord, const AAFwk::AbilityRequest &abilityRequest)
{
    int32_t processAppIndex = abilityRecord->GetAppIndex();
    if (callerRecord != nullptr) {
        std::string extensionType = abilityRequest.want.GetStringParam(UIEXTENSION_TYPE_KEY);
        auto embeddedType = AppExecFwk::ConvertToExtensionAbilityType(extensionType);
        std::string callerBundleName = callerRecord->GetApplicationInfo().bundleName;
        if (callerBundleName == AAFwk::AbilityConfig::SCENEBOARD_BUNDLE_NAME &&
            embeddedType == AppExecFwk::ExtensionAbilityType::EMBEDDED_UI) {
            processAppIndex = abilityRequest.want.GetIntParam(AAFwk::Want::PARAM_APP_CLONE_INDEX_KEY, processAppIndex);
        }
    }
    return processAppIndex;
}
}
std::atomic_int32_t ExtensionRecordManager::extensionRecordId_ = INVALID_EXTENSION_RECORD_ID;

ExtensionRecordManager::ExtensionRecordManager(const int32_t userId) : userId_(userId)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "constructor.");
}

ExtensionRecordManager::~ExtensionRecordManager()
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "deconstructor");
}

int32_t ExtensionRecordManager::GenerateExtensionRecordId(const int32_t extensionRecordId)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Input id is %{public}d.", extensionRecordId);
    std::lock_guard<std::mutex> lock(mutex_);
    if (extensionRecordId != INVALID_EXTENSION_RECORD_ID &&
        !extensionRecordIdSet_.count(extensionRecordId)) {
        extensionRecordIdSet_.insert(extensionRecordId);
        extensionRecordId_ = extensionRecordId;
        return extensionRecordId_;
    }

    if (extensionRecordId == INVALID_EXTENSION_RECORD_ID) {
        ++extensionRecordId_;
    }

    while (extensionRecordIdSet_.count(extensionRecordId_)) {
        extensionRecordId_++;
    }

    return extensionRecordId_;
}

void ExtensionRecordManager::AddExtensionRecord(const int32_t extensionRecordId,
    const std::shared_ptr<ExtensionRecord> &record)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "extensionRecordId %{public}d.", extensionRecordId);
    std::lock_guard<std::mutex> lock(mutex_);
    extensionRecords_.emplace(extensionRecordId, record);
}

void ExtensionRecordManager::RemoveExtensionRecord(const int32_t extensionRecordId)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "extensionRecordId %{public}d.", extensionRecordId);
    std::lock_guard<std::mutex> lock(mutex_);
    extensionRecords_.erase(extensionRecordId);
    terminateRecords_.erase(extensionRecordId);
}

void ExtensionRecordManager::AddExtensionRecordToTerminatedList(const int32_t extensionRecordId)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "extensionRecordId %{public}d.", extensionRecordId);
    std::lock_guard<std::mutex> lock(mutex_);

    auto findRecord = extensionRecords_.find(extensionRecordId);
    if (findRecord == extensionRecords_.end()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "extensionRecordId %{public}d not found", extensionRecordId);
        return;
    }
    terminateRecords_.emplace(*findRecord);
}

int32_t ExtensionRecordManager::GetExtensionRecord(const int32_t extensionRecordId,
    const std::string &hostBundleName, std::shared_ptr<ExtensionRecord> &extensionRecord, bool &isLoaded)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "extensionRecordId %{public}d.", extensionRecordId);
    std::lock_guard<std::mutex> lock(mutex_);
    // find target record firstly
    auto it = extensionRecords_.find(extensionRecordId);
    if (it != extensionRecords_.end() && it->second != nullptr) {
        // check bundleName
        TAG_LOGD(AAFwkTag::ABILITYMGR, "Stored host bundleName: %{public}s, input bundleName is %{public}s.",
            it->second->hostBundleName_.c_str(), hostBundleName.c_str());
        if (it->second->hostBundleName_ == hostBundleName) {
            extensionRecord = it->second;
            isLoaded = true;
            return ERR_OK;
        }
    }
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Not found stored id %{public}d.", extensionRecordId);
    extensionRecord = nullptr;
    isLoaded = false;
    return ERR_NULL_OBJECT;
}

bool ExtensionRecordManager::IsBelongToManager(const AppExecFwk::AbilityInfo &abilityInfo)
{
    // only support UIExtension now
    return AAFwk::UIExtensionWrapper::IsUIExtension(abilityInfo.extensionAbilityType);
}

int32_t ExtensionRecordManager::GetActiveUIExtensionList(const int32_t pid, std::vector<std::string> &extensionList)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "called");
    std::lock_guard<std::mutex> lock(mutex_);
    for (const auto &it : extensionRecords_) {
        if (it.second == nullptr || it.second->abilityRecord_ == nullptr ||
            pid != it.second->abilityRecord_->GetPid()) {
            continue;
        }

        extensionList.push_back(it.second->abilityRecord_->GetAbilityInfo().moduleName + SEPARATOR +
                                it.second->abilityRecord_->GetAbilityInfo().name);
    }
    return ERR_OK;
}

int32_t ExtensionRecordManager::GetActiveUIExtensionList(
    const std::string &bundleName, std::vector<std::string> &extensionList)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "called");
    std::lock_guard<std::mutex> lock(mutex_);
    for (const auto &it : extensionRecords_) {
        if (it.second == nullptr || it.second->abilityRecord_ == nullptr ||
            bundleName != it.second->abilityRecord_->GetAbilityInfo().bundleName) {
            continue;
        }

        extensionList.push_back(it.second->abilityRecord_->GetAbilityInfo().moduleName + SEPARATOR +
                                it.second->abilityRecord_->GetAbilityInfo().name);
    }
    return ERR_OK;
}

int32_t ExtensionRecordManager::GetOrCreateExtensionRecord(const AAFwk::AbilityRequest &abilityRequest,
    const std::string &hostBundleName, std::shared_ptr<AAFwk::BaseExtensionRecord> &abilityRecord, bool &isLoaded)
{
    CHECK_POINTER_AND_RETURN(abilityRequest.sessionInfo, ERR_INVALID_VALUE);
    abilityRecord = GetAbilityRecordBySessionInfo(abilityRequest.sessionInfo);
    if (abilityRecord != nullptr) {
        isLoaded = true;
        return ERR_OK;
    }

    // Check AGENT_UI launch limit before creating new record
    bool isAgentUI = AAFwk::UIExtensionWrapper::IsAgentUIExtension(abilityRequest.abilityInfo.extensionAbilityType);
    int32_t callerUid = IPCSkeleton::GetCallingUid();
    if (isAgentUI) {
        auto ret = CheckAgentUILaunchLimit(callerUid, abilityRequest.abilityInfo.bundleName);
        if (ret != ERR_OK) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "agentUI launch limit check failed, ret:%{public}d", ret);
            return ret;
        }
    }

    std::shared_ptr<ExtensionRecord> extensionRecord = nullptr;
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Check Preload Extension Record.");
    auto hostPid = IPCSkeleton::GetCallingPid();
    auto result = IsPreloadExtensionRecord(abilityRequest, hostPid, extensionRecord, isLoaded);
    if (result) {
        std::string abilityName = abilityRequest.want.GetElement().GetAbilityName();
        std::string bundleName = abilityRequest.want.GetElement().GetBundleName();
        std::string moduleName = abilityRequest.want.GetElement().GetModuleName();
        auto extensionRecordMapKey = std::make_tuple(abilityName, bundleName, moduleName, hostPid);
        RemovePreloadUIExtensionRecord(extensionRecordMapKey);
        HandlePreloadUIExtensionLoaded(extensionRecord);
    } else {
        int32_t ret = GetOrCreateExtensionRecordInner(abilityRequest, hostBundleName, extensionRecord, isLoaded);
        if (ret != ERR_OK) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "GetOrCreateExtensionRecordInner error");
            return ret;
        }
        // Add AGENT_UI launch record after new record created
        if (isAgentUI && extensionRecord != nullptr && !isLoaded) {
            AddAgentUILaunchRecord(callerUid, abilityRequest.abilityInfo.bundleName,
                extensionRecord->extensionRecordId_);
        }
    }
    if (extensionRecord != nullptr) {
        abilityRecord = extensionRecord->abilityRecord_;
    }
    return ERR_OK;
}

std::shared_ptr<AAFwk::BaseExtensionRecord> ExtensionRecordManager::GetAbilityRecordBySessionInfo(
    const sptr<AAFwk::SessionInfo> &sessionInfo)
{
    CHECK_POINTER_AND_RETURN(sessionInfo, nullptr);
    if (sessionInfo->uiExtensionComponentId == INVALID_EXTENSION_RECORD_ID) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "ExtensionAbility id invalid or not configured.");
        return nullptr;
    }

    std::lock_guard<std::mutex> lock(mutex_);
    for (const auto& it : extensionRecords_) {
        if (it.second == nullptr) {
            continue;
        }
        std::shared_ptr<AAFwk::BaseExtensionRecord> abilityRecord = it.second->abilityRecord_;
        if (abilityRecord == nullptr) {
            continue;
        }
        sptr<AAFwk::SessionInfo> recordSessionInfo = abilityRecord->GetSessionInfo();
        if (recordSessionInfo == nullptr) {
            continue;
        }
        if (recordSessionInfo->uiExtensionComponentId == sessionInfo->uiExtensionComponentId) {
            TAG_LOGD(AAFwkTag::ABILITYMGR,
                "found record, uiExtensionComponentId: %{public}" PRIu64, sessionInfo->uiExtensionComponentId);
            return abilityRecord;
        }
    }
    return nullptr;
}

bool ExtensionRecordManager::IsHostSpecifiedProcessValid(const AAFwk::AbilityRequest &abilityRequest,
    std::shared_ptr<ExtensionRecord> &record, const std::string &process)
{
    std::lock_guard<std::mutex> lock(mutex_);
    for (const auto &iter: extensionRecords_) {
        if (iter.second == nullptr || iter.second->abilityRecord_ == nullptr) {
            continue;
        }
        if (iter.second->abilityRecord_->GetProcessName() != process) {
            continue;
        }
        TAG_LOGD(AAFwkTag::ABILITYMGR, "found match extension record: id %{public}d", iter.first);
        AppExecFwk::AbilityInfo abilityInfo = iter.second->abilityRecord_->GetAbilityInfo();
        if (abilityRequest.abilityInfo.bundleName != abilityInfo.bundleName) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "bundleName not match");
            return false;
        }
        if (abilityRequest.abilityInfo.name != abilityInfo.name) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "abilityName not match");
            return false;
        }
        std::shared_ptr<AAFwk::AbilityRecord> callerRecord = AAFwk::Token::
            GetAbilityRecordByToken(abilityRequest.callerToken);
        if (callerRecord != nullptr && callerRecord->GetApplicationInfo().bundleName ==
            AAFwk::AbilityConfig::SCENEBOARD_BUNDLE_NAME) {
            if (abilityRequest.want.GetIntParam(AAFwk::Want::PARAM_APP_CLONE_INDEX_KEY, 0) !=
                iter.second->abilityRecord_->GetAppIndex()) {
                TAG_LOGE(AAFwkTag::ABILITYMGR, "appIndex not match by scb");
                return false;
            }
        } else if (record->abilityRecord_->GetAppIndex() != iter.second->abilityRecord_->GetAppIndex()) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "appIndex not match");
            return false;
        }
        return true;
    }
    TAG_LOGE(AAFwkTag::ABILITYMGR, "process not found, %{public}s", process.c_str());
    return false;
}

int32_t ExtensionRecordManager::UpdateProcessName(const AAFwk::AbilityRequest &abilityRequest,
    std::shared_ptr<ExtensionRecord> &record)
{
    CHECK_POINTER_AND_RETURN(record, ERR_INVALID_VALUE);
    std::shared_ptr<AAFwk::AbilityRecord> abilityRecord = record->abilityRecord_;
    std::shared_ptr<AAFwk::AbilityRecord> callerRecord = AAFwk::Token::
        GetAbilityRecordByToken(abilityRequest.callerToken);

    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);

    int32_t processAppIndex = GetAppIndexByRecord(abilityRecord, callerRecord, abilityRequest);
    auto appendAppIndex = [processAppIndex](std::string &processName) {
        if (processAppIndex > 0) {
            processName += SEPARATOR + std::to_string(processAppIndex);
        }
    };
    switch (record->processMode_) {
        case PROCESS_MODE_AGENT_UI: {
            std::string process = abilityRequest.abilityInfo.bundleName
                    + SEPARATOR + AGENT_EXTENSION_TYPE;
            abilityRecord->SetProcessName(process);
            break;
        }

        case PROCESS_MODE_PLUGIN: {
            std::string process = record->hostBundleName_ + SEPARATOR + abilityRequest.abilityInfo.bundleName
                + SEPARATOR + EMBEDDEDUI + SEPARATOR + std::to_string(abilityRequest.abilityInfo.appIndex);
            abilityRecord->SetProcessName(process);
            break;
        }
        case PROCESS_MODE_INSTANCE: {
            std::string process = abilityRequest.abilityInfo.bundleName + SEPARATOR + abilityRequest.abilityInfo.name
                + SEPARATOR + std::to_string(abilityRecord->GetUIExtensionAbilityId());
            appendAppIndex(process);
            abilityRecord->SetProcessName(process);
            break;
        }
        case PROCESS_MODE_TYPE: {
            std::string process = abilityRequest.abilityInfo.bundleName + SEPARATOR + abilityRequest.abilityInfo.name;
            appendAppIndex(process);
            abilityRecord->SetProcessName(process);
            break;
        }
        case PROCESS_MODE_CUSTOM: {
            std::string process = abilityRequest.abilityInfo.bundleName + abilityRequest.customProcess;
            appendAppIndex(process);
            abilityRecord->SetProcessName(process);
            abilityRecord->SetCustomProcessFlag(abilityRequest.customProcess);
            break;
        }
        case PROCESS_MODE_HOST_SPECIFIED: {
            std::string processName = abilityRequest.want.GetStringParam(PROCESS_MODE_HOST_SPECIFIED_KEY);
            if (!IsHostSpecifiedProcessValid(abilityRequest, record, processName)) {
                TAG_LOGE(AAFwkTag::ABILITYMGR, "invalid name, %{public}s", processName.c_str());
                return ERR_INVALID_VALUE;
            }
            abilityRecord->SetProcessName(processName);
            abilityRecord->SetCustomProcessFlag(abilityRequest.customProcess);
            break;
        }
        case PROCESS_MODE_RUN_WITH_MAIN_PROCESS: {
            std::string process = abilityRequest.abilityInfo.bundleName;
            if (!abilityRequest.appInfo.process.empty()) {
                process = abilityRequest.appInfo.process;
            }
            if (abilityRecord->GetAppIndex() > 0) {
                process += SEPARATOR + std::to_string(abilityRecord->GetAppIndex());
            }
            abilityRecord->SetProcessName(process);
            break;
        }
        default: // AppExecFwk::ExtensionProcessMode::UNDEFINED or AppExecFwk::ExtensionProcessMode::BUNDLE
            // no need to update
            if (!abilityRequest.moduleProcess.empty()) {
                std::string process = abilityRequest.moduleProcess;
                TAG_LOGD(AAFwkTag::ABILITYMGR, "moduleProcess: %{public}s", process.c_str());
                if (abilityRecord->GetAppIndex() > 0) {
                    process += std::to_string(abilityRecord->GetAppIndex());
                }
                abilityRecord->SetProcessName(process);
            } else if (abilityRecord->GetAppIndex() > 0) {
                std::string process = abilityRequest.abilityInfo.bundleName
                    + SEPARATOR + abilityRequest.abilityInfo.extensionTypeName;
                process += SEPARATOR + std::to_string(abilityRecord->GetAppIndex());
                abilityRecord->SetProcessName(process);
            }
            break;
    }
    return ERR_OK;
}

int32_t ExtensionRecordManager::GetHostPidForExtensionId(int32_t extensionRecordId, pid_t &hostPid)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call");
    std::lock_guard<std::mutex> lock(mutex_);
    std::shared_ptr<ExtensionRecord> extensionRecord = nullptr;
    if (extensionRecords_.find(extensionRecordId) != extensionRecords_.end()) {
        extensionRecord = extensionRecords_[extensionRecordId];
        CHECK_POINTER_AND_RETURN(extensionRecord, ERR_INVALID_VALUE);
        hostPid = extensionRecord->hostPid_;
        return ERR_OK;
    }
    return ERR_INVALID_VALUE;
}

int32_t ExtensionRecordManager::AddPreloadUIExtensionRecord(
    const std::shared_ptr<AAFwk::BaseExtensionRecord> abilityRecord)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call");
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);
    std::shared_ptr<ExtensionRecord> extensionRecord = nullptr;
    auto extensionRecordId = abilityRecord->GetUIExtensionAbilityId();
    if (extensionRecords_.find(extensionRecordId) != extensionRecords_.end()) {
        extensionRecord = extensionRecords_[extensionRecordId];
        CHECK_POINTER_AND_RETURN(extensionRecord, ERR_INVALID_VALUE);
        auto hostPid = extensionRecord->hostPid_;
        auto preLoadUIExtensionInfo = std::make_tuple(abilityRecord->GetWant().GetElement().GetAbilityName(),
            abilityRecord->GetWant().GetElement().GetBundleName(),
            abilityRecord->GetWant().GetElement().GetModuleName(), hostPid);
        TAG_LOGD(AAFwkTag::ABILITYMGR, "hostPid: %{public}d, elementName:%{public}s/%{public}s",
            hostPid, abilityRecord->GetWant().GetElement().GetBundleName().c_str(),
            abilityRecord->GetWant().GetElement().GetAbilityName().c_str());
        std::lock_guard<std::mutex> lock(preloadUIExtensionMapMutex_);
        preloadUIExtensionMap_[preLoadUIExtensionInfo].push_back(extensionRecord);
        return ERR_OK;
    }
    TAG_LOGE(AAFwkTag::ABILITYMGR, "extensionRecordId invalid");
    return ERR_INVALID_VALUE;
}

void ExtensionRecordManager::RemoveAllPreloadUIExtensionRecord(PreLoadUIExtensionMapKey &preLoadUIExtensionInfo)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call.");
    std::lock_guard<std::mutex> lock(preloadUIExtensionMapMutex_);
    if (preloadUIExtensionMap_.find(preLoadUIExtensionInfo) != preloadUIExtensionMap_.end()) {
        preloadUIExtensionMap_.erase(preLoadUIExtensionInfo);
    } else {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "The preLoadUIExtensionInfo has no corresponding extensionRecord object!");
    }
}

bool ExtensionRecordManager::IsPreloadExtensionRecord(const AAFwk::AbilityRequest &abilityRequest,
    const pid_t &hostPid, std::shared_ptr<ExtensionRecord> &extensionRecord, bool &isLoaded)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call.");
    std::string abilityName = abilityRequest.want.GetElement().GetAbilityName();
    std::string bundleName = abilityRequest.want.GetElement().GetBundleName();
    std::string moduleName = abilityRequest.want.GetElement().GetModuleName();
    auto extensionRecordMapKey = std::make_tuple(abilityName, bundleName, moduleName, hostPid);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "hostBundleName: %{public}d, bundleName: %{public}s",
        hostPid, bundleName.c_str());
    std::lock_guard<std::mutex> lock(preloadUIExtensionMapMutex_);
    auto item = preloadUIExtensionMap_.find(extensionRecordMapKey);
    if (item != preloadUIExtensionMap_.end()) {
        if (!item->second.empty()) {
            TAG_LOGD(AAFwkTag::ABILITYMGR, "UIExtensionAbility has been preloaded.");
            auto extensionRecords = item->second;
            extensionRecord = extensionRecords[0];
            if (extensionRecord == nullptr) {
                TAG_LOGE(AAFwkTag::ABILITYMGR, "null ExtensionRecord");
                return false;
            }
            extensionRecord->Update(abilityRequest);
            isLoaded = true;
            return true;
        }
    }
    TAG_LOGD(AAFwkTag::ABILITYMGR, "UIExtension is not preloaded.");
    return false;
}

bool ExtensionRecordManager::RemovePreloadUIExtensionRecordById(
    const std::tuple<std::string, std::string, std::string, pid_t> &extensionRecordMapKey,
    int32_t extensionRecordId)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call.");
    std::lock_guard<std::mutex> lock(preloadUIExtensionMapMutex_);
    auto item = preloadUIExtensionMap_.find(extensionRecordMapKey);
    if (item == preloadUIExtensionMap_.end()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "extensionRecords unfound");
        return false;
    }
    if (item->second.empty()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "clean the map key");
        preloadUIExtensionMap_.erase(extensionRecordMapKey);
        return false;
    }
    for (auto it = item->second.begin(); it != item->second.end(); ++it) {
        if ((*it)->extensionRecordId_ == extensionRecordId) {
            std::shared_ptr<ExtensionRecord> extensionRecord = *it;
            HandlePreloadUIExtensionDestroyed(extensionRecord);
            item->second.erase(it);
            TAG_LOGD(AAFwkTag::ABILITYMGR, "Remove extension record by id: %{public}d success.", extensionRecordId);
            if (item->second.empty()) {
                TAG_LOGD(AAFwkTag::ABILITYMGR, "Clean extensionRecord by map key");
                preloadUIExtensionMap_.erase(extensionRecordMapKey);
            }
            return true;
        }
    }
    TAG_LOGE(AAFwkTag::ABILITYMGR, "findRecordsbyID: %{public}d failed", extensionRecordId);
    return false;
}

bool ExtensionRecordManager::RemovePreloadUIExtensionRecord(
    const std::tuple<std::string, std::string, std::string, pid_t> extensionRecordMapKey)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call.");
    std::lock_guard<std::mutex> lock(preloadUIExtensionMapMutex_);
    auto item = preloadUIExtensionMap_.find(extensionRecordMapKey);
    if (item != preloadUIExtensionMap_.end()) {
        if (!item->second.empty()) {
            item->second.erase(item->second.begin());
        }
        if (item->second.empty()) {
            preloadUIExtensionMap_.erase(extensionRecordMapKey);
        }
        return true;
    }
    TAG_LOGE(AAFwkTag::ABILITYMGR, "preloadUIExtensionMap_ erase key error");
    return false;
}

int32_t ExtensionRecordManager::GetOrCreateExtensionRecordInner(const AAFwk::AbilityRequest &abilityRequest,
    const std::string &hostBundleName, std::shared_ptr<ExtensionRecord> &extensionRecord, bool &isLoaded)
{
    std::shared_ptr<ExtensionRecordFactory> factory = nullptr;
    if (AAFwk::UIExtensionWrapper::IsUIExtension(abilityRequest.abilityInfo.extensionAbilityType)) {
        factory = DelayedSingleton<UIExtensionRecordFactory>::GetInstance();
    }
    if (factory == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "invalid extensionAbilityType");
        return ERR_INVALID_VALUE;
    }
    int32_t result = factory->PreCheck(abilityRequest, hostBundleName);
    if (result != ERR_OK) {
        return result;
    }
    int32_t extensionRecordId = INVALID_EXTENSION_RECORD_ID;
    bool needReuse = factory->NeedReuse(abilityRequest, extensionRecordId);
    if (needReuse) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "reuse record, id: %{public}d", extensionRecordId);
        int32_t ret = GetExtensionRecord(extensionRecordId, hostBundleName, extensionRecord, isLoaded);
        if (ret == ERR_OK) {
            extensionRecord->Update(abilityRequest);
        }
        return ret;
    }
    result = factory->CreateRecord(abilityRequest, extensionRecord);
    if (result != ERR_OK) {
        return result;
    }
    CHECK_POINTER_AND_RETURN(extensionRecord, ERR_NULL_OBJECT);
    std::shared_ptr<AAFwk::BaseExtensionRecord> abilityRecord = extensionRecord->abilityRecord_;
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_NULL_OBJECT);
    isLoaded = false;
    // Reuse id or not has been checked, so alloc a new id here.
    extensionRecordId = GenerateExtensionRecordId(INVALID_EXTENSION_RECORD_ID);
    extensionRecord->extensionRecordId_ = extensionRecordId;
    extensionRecord->hostBundleName_ = hostBundleName;
    abilityRecord->SetOwnerMissionUserId(userId_);
    abilityRecord->SetUIExtensionAbilityId(extensionRecordId);
    result = SetAbilityProcessName(abilityRequest, abilityRecord, extensionRecord);
    if (result != ERR_OK) {
        return result;
    }
    TAG_LOGD(AAFwkTag::ABILITYMGR,
        "extensionRecordId: %{public}d, extensionProcessMode:%{public}d, process: %{public}s",
        extensionRecordId, abilityRequest.extensionProcessMode, abilityRecord->GetAbilityInfo().process.c_str());
    std::lock_guard<std::mutex> lock(mutex_);
    extensionRecords_[extensionRecordId] = extensionRecord;
    return ERR_OK;
}

int32_t ExtensionRecordManager::SetAbilityProcessName(const AAFwk::AbilityRequest &abilityRequest,
    const std::shared_ptr<AAFwk::BaseExtensionRecord> &abilityRecord,
    std::shared_ptr<ExtensionRecord> &extensionRecord)
{
    if (abilityRequest.abilityInfo.isolationProcess &&
        AAFwk::UIExtensionWrapper::IsUIExtension(abilityRecord->GetAbilityInfo().extensionAbilityType) &&
        AAFwk::AppUtils::GetInstance().IsStartSpecifiedProcess()) {
        abilityRecord->SetProcessName(
            abilityRequest.abilityInfo.bundleName + SEPARATOR + abilityRequest.abilityInfo.extensionTypeName);
        return ERR_OK;
    } else {
        return UpdateProcessName(abilityRequest, extensionRecord);
    }
}

int32_t ExtensionRecordManager::StartAbility(const AAFwk::AbilityRequest &abilityRequest)
{
    return ERR_OK;
}

void ExtensionRecordManager::SetCachedFocusedCallerToken(int32_t extensionRecordId,
    sptr<IRemoteObject> &focusedCallerToken)
{
    auto it = extensionRecords_.find(extensionRecordId);
    if (it != extensionRecords_.end() && it->second != nullptr) {
        it->second->SetFocusedCallerToken(focusedCallerToken);
    }
}

sptr<IRemoteObject> ExtensionRecordManager::GetCachedFocusedCallerToken(int32_t extensionRecordId) const
{
    auto it = extensionRecords_.find(extensionRecordId);
    if (it != extensionRecords_.end() && it->second != nullptr) {
        return it->second->GetFocusedCallerToken();
    }
    return nullptr;
}

sptr<IRemoteObject> ExtensionRecordManager::GetRootCallerTokenLocked(
    int32_t extensionRecordId, const std::shared_ptr<AAFwk::AbilityRecord> &abilityRecord)
{
    auto it = extensionRecords_.find(extensionRecordId);
    if (it != extensionRecords_.end() && it->second != nullptr) {
        sptr<IRemoteObject> rootCallerToken = it->second->GetRootCallerToken();
        if (rootCallerToken != nullptr) {
            return rootCallerToken;
        }
        std::list<sptr<IRemoteObject>> callerList;
        GetCallerTokenList(abilityRecord, callerList);

        if (callerList.empty()) {
            TAG_LOGW(AAFwkTag::ABILITYMGR, "callerList empty");
            return nullptr;
        }

        rootCallerToken = callerList.front();
        it->second->SetRootCallerToken(rootCallerToken);
        return rootCallerToken;
    }
    TAG_LOGE(AAFwkTag::ABILITYMGR, "not found id %{public}d", extensionRecordId);
    return nullptr;
}

int32_t ExtensionRecordManager::CreateExtensionRecord(const AAFwk::AbilityRequest &abilityRequest,
    const std::string &hostBundleName, std::shared_ptr<ExtensionRecord> &extensionRecord,
    int32_t &extensionRecordId, int32_t hostPid)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call");
    std::shared_ptr<ExtensionRecordFactory> factory = nullptr;
    if (AAFwk::UIExtensionWrapper::IsUIExtension(abilityRequest.abilityInfo.extensionAbilityType)) {
        factory = DelayedSingleton<UIExtensionRecordFactory>::GetInstance();
    }
    if (factory == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "invalid extensionAbilityType");
        return ERR_INVALID_VALUE;
    }
    int32_t result = factory->PreCheck(abilityRequest, hostBundleName);
    if (result != ERR_OK) {
        return result;
    }
    result = factory->CreateRecord(abilityRequest, extensionRecord);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "createRecord error");
        return result;
    }
    CHECK_POINTER_AND_RETURN(extensionRecord, ERR_NULL_OBJECT);
    std::shared_ptr<AAFwk::BaseExtensionRecord> abilityRecord = extensionRecord->abilityRecord_;
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_NULL_OBJECT);
    extensionRecordId = GenerateExtensionRecordId(extensionRecordId);
    extensionRecord->extensionRecordId_ = extensionRecordId;
    extensionRecord->requestCode_ = abilityRequest.requestCode;
    extensionRecord->hostBundleName_ = hostBundleName;
    abilityRecord->SetOwnerMissionUserId(userId_);
    abilityRecord->SetUIExtensionAbilityId(extensionRecordId);
    extensionRecord->hostPid_ = (hostPid == AAFwk::DEFAULT_INVAL_VALUE) ? IPCSkeleton::GetCallingPid() : hostPid;
    //add uiextension record register state observer object.
    if (abilityRecord->GetWant().GetBoolParam(IS_PRELOAD_UIEXTENSION_ABILITY, false)) {
        auto ret = extensionRecord->RegisterStateObserver(hostBundleName);
        if (ret != ERR_OK) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "register failed, err: %{public}d", ret);
            return ERR_INVALID_VALUE;
        }
    }
    result = UpdateProcessName(abilityRequest, extensionRecord);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "update processname error");
        return result;
    }
    TAG_LOGI(AAFwkTag::ABILITYMGR,
        "extensionRecordId: %{public}d, extensionProcessMode:%{public}d, process: %{public}s",
        extensionRecordId, abilityRequest.extensionProcessMode, abilityRecord->GetAbilityInfo().process.c_str());
    std::lock_guard<std::mutex> lock(mutex_);
    extensionRecords_[extensionRecordId] = extensionRecord;
    return ERR_OK;
}

std::shared_ptr<AAFwk::AbilityRecord> ExtensionRecordManager::GetUIExtensionRootHostInfo(
    const sptr<IRemoteObject> token)
{
    if (token == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "input param invalid");
        return nullptr;
    }

    auto abilityRecord = AAFwk::Token::GetAbilityRecordByToken(token);
    if (abilityRecord == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "abilityRecord empty");
        return nullptr;
    }

    if (!AAFwk::UIExtensionWrapper::IsUIExtension(abilityRecord->GetAbilityInfo().extensionAbilityType)) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "not uiextension ability");
        return nullptr;
    }

    sptr<IRemoteObject> rootCallerToken = nullptr;
    auto extensionRecordId = abilityRecord->GetUIExtensionAbilityId();
    {
        std::lock_guard<std::mutex> lock(mutex_);
        rootCallerToken = GetRootCallerTokenLocked(extensionRecordId, abilityRecord);
    }

    if (rootCallerToken == nullptr) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "get record failed");
        return nullptr;
    }

    return AAFwk::Token::GetAbilityRecordByToken(rootCallerToken);
}

int32_t ExtensionRecordManager::GetUIExtensionSessionInfo(
    const sptr<IRemoteObject> token, UIExtensionSessionInfo &uiExtensionSessionInfo)
{
    if (token == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "input param invalid");
        return ERR_NULL_OBJECT;
    }

    auto abilityRecord = AAFwk::Token::GetAbilityRecordByToken(token);
    if (abilityRecord == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "abilityRecord empty");
        return ERR_NULL_OBJECT;
    }

    if (!AAFwk::UIExtensionWrapper::IsUIExtension(abilityRecord->GetAbilityInfo().extensionAbilityType)) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "not uiextension ability");
        return ERR_INVALID_VALUE;
    }

    auto sessionInfo = abilityRecord->GetSessionInfo();
    if (sessionInfo == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null sessionInfo");
        return ERR_NULL_OBJECT;
    }

    uiExtensionSessionInfo.persistentId = sessionInfo->persistentId;
    uiExtensionSessionInfo.hostWindowId = sessionInfo->hostWindowId;
    uiExtensionSessionInfo.uiExtensionUsage = sessionInfo->uiExtensionUsage;
    uiExtensionSessionInfo.elementName = abilityRecord->GetElementName();
    uiExtensionSessionInfo.extensionAbilityType = abilityRecord->GetAbilityInfo().extensionAbilityType;

    auto callerToken = sessionInfo->callerToken;
    if (callerToken != nullptr) {
        auto callerAbilityRecord = AAFwk::Token::GetAbilityRecordByToken(callerToken);
        if (callerAbilityRecord != nullptr) {
            uiExtensionSessionInfo.hostElementName = callerAbilityRecord->GetElementName();
        }
    }
    return ERR_OK;
}

std::shared_ptr<ExtensionRecord> ExtensionRecordManager::GetExtensionRecordById(int32_t extensionRecordId)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto findRecord = extensionRecords_.find(extensionRecordId);
    if (findRecord != extensionRecords_.end()) {
        return findRecord->second;
    }
    findRecord = terminateRecords_.find(extensionRecordId);
    if (findRecord == terminateRecords_.end()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "uiextension record  unfound, id: %{public}d", extensionRecordId);
        return nullptr;
    }

    return findRecord->second;
}

void ExtensionRecordManager::LoadTimeout(int32_t extensionRecordId)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "called");
    auto uiExtensionRecord = std::static_pointer_cast<UIExtensionRecord>(GetExtensionRecordById(extensionRecordId));
    if (uiExtensionRecord == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "parsing uiExtensionRecord failed");
        return;
    }
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Start load timeout.");
    HandlePreloadUIExtensionSuccess(extensionRecordId, false);
    uiExtensionRecord->LoadTimeout();
}

void ExtensionRecordManager::ForegroundTimeout(int32_t extensionRecordId)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "called");
    auto uiExtensionRecord = std::static_pointer_cast<UIExtensionRecord>(GetExtensionRecordById(extensionRecordId));
    if (uiExtensionRecord == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "parsing uiExtensionRecord failed");
        return;
    }
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Start foreground timeout.");
    uiExtensionRecord->ForegroundTimeout();
}

void ExtensionRecordManager::BackgroundTimeout(int32_t extensionRecordId)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "called");
    auto uiExtensionRecord = std::static_pointer_cast<UIExtensionRecord>(GetExtensionRecordById(extensionRecordId));
    if (uiExtensionRecord == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "parsing uiextension record failed");
        return;
    }
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Start background timeout.");
    uiExtensionRecord->BackgroundTimeout();
}

void ExtensionRecordManager::TerminateTimeout(int32_t extensionRecordId)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "called");
    auto uiExtensionRecord = std::static_pointer_cast<UIExtensionRecord>(GetExtensionRecordById(extensionRecordId));
    if (uiExtensionRecord == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "parsing uiExtensionRecord failed");
        return;
    }
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Start terminate timeout.");
    uiExtensionRecord->TerminateTimeout();
}

void ExtensionRecordManager::GetCallerTokenList(
    const std::shared_ptr<AAFwk::AbilityRecord> &abilityRecord, std::list<sptr<IRemoteObject>> &callerList)
{
    CHECK_POINTER(abilityRecord);
    auto extensionRecordId = abilityRecord->GetUIExtensionAbilityId();
    auto sessionInfo = abilityRecord->GetSessionInfo();
    if (!sessionInfo) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "sessionInfo null, id: %{public}d", extensionRecordId);
        callerList.clear();
        return;
    }

    auto callerToken = sessionInfo->callerToken;
    auto callerAbilityRecord = AAFwk::Token::GetAbilityRecordByToken(callerToken);
    if (callerAbilityRecord == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "callerRecord null, id: %{public}d", extensionRecordId);
        callerList.clear();
        return;
    }

    TAG_LOGD(AAFwkTag::ABILITYMGR, "ability:%{public}s/%{public}s, pid: %{public}d, tokenId: %{public}d",
        callerAbilityRecord->GetWant().GetElement().GetBundleName().c_str(),
        callerAbilityRecord->GetWant().GetElement().GetAbilityName().c_str(), callerAbilityRecord->GetPid(),
        callerAbilityRecord->GetApplicationInfo().accessTokenId);

    auto callerExtensionRecordId = callerAbilityRecord->GetUIExtensionAbilityId();
    if (callerExtensionRecordId == INVALID_EXTENSION_RECORD_ID) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "Get caller, callerRecord: %{public}d", callerExtensionRecordId);
        callerList.push_front(callerToken);
        return;
    }

    // If caller extension record id is same with current, need terminate, prevent possible stack-overflow.
    if (callerExtensionRecordId == extensionRecordId) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "callerRecordId: %{public}d, same with caller", extensionRecordId);
        callerList.clear();
        return;
    }

    callerList.push_front(callerToken);
    GetCallerTokenList(callerAbilityRecord, callerList);
}

bool ExtensionRecordManager::IsFocused(
    int32_t extensionRecordId, const sptr<IRemoteObject> &token, const sptr<IRemoteObject> &focusToken)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto cachedCaller = GetCachedFocusedCallerToken(extensionRecordId);
    if (cachedCaller != nullptr && cachedCaller == focusToken) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "id: %{public}d has focused", extensionRecordId);
        return true;
    }

    auto abilityRecord = AAFwk::Token::GetAbilityRecordByToken(token);
    if (!abilityRecord) {
        return false;
    }

    TAG_LOGD(AAFwkTag::ABILITYMGR, "ability:%{public}s/%{public}s, pid: %{public}d, tokenId: %{public}d",
        abilityRecord->GetWant().GetElement().GetBundleName().c_str(),
        abilityRecord->GetWant().GetElement().GetAbilityName().c_str(), abilityRecord->GetPid(),
        abilityRecord->GetApplicationInfo().accessTokenId);

    if (!AAFwk::UIExtensionWrapper::IsUIExtension(abilityRecord->GetAbilityInfo().extensionAbilityType)) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "not uiextension");
        return false;
    }

    bool isFocused = false;
    std::list<sptr<IRemoteObject>> callerList;
    GetCallerTokenList(abilityRecord, callerList);
    for (auto& item : callerList) {
        auto ability = AAFwk::Token::GetAbilityRecordByToken(item);
        if (ability == nullptr) {
            TAG_LOGW(AAFwkTag::ABILITYMGR, "wrong ability");
            continue;
        }

        if (item == focusToken) {
            isFocused = true;
            SetCachedFocusedCallerToken(extensionRecordId, item);
            break;
        }
    }
    return isFocused;
}

int32_t ExtensionRecordManager::QueryPreLoadUIExtensionRecord(const AppExecFwk::ElementName &element,
                                                              const std::string &moduleName,
                                                              const int32_t hostPid,
                                                              int32_t &recordNum)
{
    std::string abilityName = element.GetAbilityName();
    std::string bundleName = element.GetBundleName();
    TAG_LOGD(AAFwkTag::UI_EXT,
             "hostPid: %{public}d, bundleName: %{public}s, moduleName: %{public}s, abilityName: %{public}s",
             hostPid, bundleName.c_str(), moduleName.c_str(), abilityName.c_str());
    if (element.GetAbilityName().empty() || element.GetBundleName().empty() || moduleName.empty()) {
        recordNum = 0;
        TAG_LOGD(AAFwkTag::UI_EXT, "element is null.");
        return ERR_INVALID_VALUE;
    }

    auto extensionRecordMapKey =
        std::make_tuple(abilityName, bundleName, moduleName, hostPid);
    std::lock_guard<std::mutex> lock(preloadUIExtensionMapMutex_);
    auto item = preloadUIExtensionMap_.find(extensionRecordMapKey);
    if (item != preloadUIExtensionMap_.end()) {
        if (!item->second.empty()) {
            recordNum = item->second.size();
            TAG_LOGD(AAFwkTag::ABILITYMGR, "UIExtensionAbility has been preloaded,recordNum:%{public}d.", recordNum);
            return ERR_OK;
        }
    }
    TAG_LOGD(AAFwkTag::UI_EXT, "UIExtension is not preloaded.");
    recordNum = 0;
    return ERR_OK;
}

sptr<AAFwk::IPreloadUIExtensionExecuteCallback> ExtensionRecordManager::GetRemoteCallback(
    std::shared_ptr<ExtensionRecord> uiExtensionRecord)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "GetRemoteCallback called");
    if (uiExtensionRecord == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null uiExtensionRecord");
        return nullptr;
    }
    if (uiExtensionRecord->hostPid_ == 0) {
        TAG_LOGD(AAFwkTag::UI_EXT, "uiExtensionAbility not preload");
        return nullptr;
    }

    sptr<AAFwk::IPreloadUIExtensionExecuteCallback> remoteCallback = nullptr;
    {
        std::lock_guard<std::mutex> lock(preloadUIExtensionHostClientMutex_);
        auto it = preloadUIExtensionHostClientCallerTokens_.find(uiExtensionRecord->hostPid_);
        if (it == preloadUIExtensionHostClientCallerTokens_.end() || it->second == nullptr) {
            TAG_LOGE(AAFwkTag::UI_EXT, "null preloadUIExtensionHostClientCallerTokens_");
            return nullptr;
        } else {
            remoteCallback = iface_cast<AAFwk::IPreloadUIExtensionExecuteCallback>(it->second);
            if (remoteCallback == nullptr) {
                TAG_LOGE(AAFwkTag::UI_EXT, "null remoteCallback");
                return nullptr;
            }
        }
    }
    return remoteCallback;
}

void ExtensionRecordManager::HandlePreloadUIExtensionLoaded(
    const std::shared_ptr<ExtensionRecord> &uiExtensionRecord)
{
    if (uiExtensionRecord == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null uiExtensionRecord");
        return;
    }
    int32_t extensionRecordId = uiExtensionRecord->extensionRecordId_;
    TAG_LOGD(AAFwkTag::UI_EXT, "HandlePreloadUIExtensionLoaded called, id: %{public}d", extensionRecordId);
    sptr<AAFwk::IPreloadUIExtensionExecuteCallback> remoteCallback = GetRemoteCallback(uiExtensionRecord);
    if (remoteCallback == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null remoteCallback");
        return;
    }
    remoteCallback->OnLoadedDone(extensionRecordId);
}

void ExtensionRecordManager::HandlePreloadUIExtensionDestroyed(
    const std::shared_ptr<ExtensionRecord> &uiExtensionRecord)
{
    if (uiExtensionRecord == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null uiExtensionRecord");
        return;
    }
    int32_t extensionRecordId = uiExtensionRecord->extensionRecordId_;
    TAG_LOGD(AAFwkTag::UI_EXT, "HandlePreloadUIExtensionDestroyed called, id: %{public}d", extensionRecordId);
    sptr<AAFwk::IPreloadUIExtensionExecuteCallback> remoteCallback = GetRemoteCallback(uiExtensionRecord);
    if (remoteCallback == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null remoteCallback");
        return;
    }
    remoteCallback->OnDestroyDone(extensionRecordId);
}

void ExtensionRecordManager::HandlePreloadUIExtensionSuccess(int32_t extensionRecordId, bool isPreloadedSuccess)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "HandlePreloadUIExtensionSuccess called, id: %{public}d", extensionRecordId);
    auto uiExtensionRecord = std::static_pointer_cast<UIExtensionRecord>(GetExtensionRecordById(extensionRecordId));
    if (uiExtensionRecord == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null uiExtensionRecord");
        return;
    }

    sptr<AAFwk::IPreloadUIExtensionExecuteCallback> remoteCallback = GetRemoteCallback(uiExtensionRecord);
    if (remoteCallback == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null remoteCallback");
        return;
    }
    remoteCallback->OnPreloadSuccess(uiExtensionRecord->requestCode_, extensionRecordId,
        isPreloadedSuccess ? ERR_OK : AAFwk::INNER_ERR);
    if (!isPreloadedSuccess) {
        uiExtensionRecord->UnloadUIExtensionAbility();
    }
}

int32_t ExtensionRecordManager::ClearPreloadedUIExtensionAbility(int32_t extensionRecordId)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "ClearPreloadedUIExtensionAbility call, record:%{public}d", extensionRecordId);
    std::shared_ptr<ExtensionRecord> recordToUnload;
    std::shared_ptr<AAFwk::AbilityRecord> abilityRecord;
    auto hostPid = 0;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto findRecord = extensionRecords_.find(extensionRecordId);
        if (findRecord == extensionRecords_.end()) {
            TAG_LOGE(AAFwkTag::UI_EXT, "record: %{public}d not found", extensionRecordId);
            return AAFwk::ERR_CODE_INVALID_ID;
        }
        if (findRecord->second == nullptr) {
            TAG_LOGE(AAFwkTag::UI_EXT, "record %{public}d is null", extensionRecordId);
            extensionRecords_.erase(extensionRecordId);
            return ERR_INVALID_VALUE;
        }
        int32_t callingPid = IPCSkeleton::GetCallingPid();
        hostPid = findRecord->second->hostPid_;
        if (callingPid != hostPid) {
            TAG_LOGE(AAFwkTag::UI_EXT, "callingPid: %{public}d not match hostPid: %{public}d", callingPid, hostPid);
            return AAFwk::ERR_CODE_INVALID_ID;
        }
        abilityRecord = findRecord->second->abilityRecord_;
        if (abilityRecord == nullptr) {
            TAG_LOGE(AAFwkTag::UI_EXT, "abilityRecord is null for record %{public}d", extensionRecordId);
            extensionRecords_.erase(extensionRecordId);
            return ERR_INVALID_VALUE;
        }
        recordToUnload = findRecord->second;
    }
    auto abilityInfo = abilityRecord->GetAbilityInfo();
    auto extensionRecordMapKey = std::make_tuple(
        abilityInfo.name, abilityInfo.bundleName, abilityInfo.moduleName, hostPid);
    bool ret = RemovePreloadUIExtensionRecordById(extensionRecordMapKey, extensionRecordId);
    if (!ret) {
        TAG_LOGE(AAFwkTag::UI_EXT, "remove failed for record %{public}d", extensionRecordId);
        return ERR_INVALID_VALUE;
    }

    if (recordToUnload != nullptr) {
        recordToUnload->UnloadUIExtensionAbility();
    }
    return ERR_OK;
}

int32_t ExtensionRecordManager::ClearAllPreloadUIExtensionRecordForHost()
{
    TAG_LOGD(AAFwkTag::UI_EXT, "ClearAllPreloadUIExtensionRecordForHost call");
    int32_t callingPid = IPCSkeleton::GetCallingPid();
    std::vector<std::shared_ptr<ExtensionRecord>> recordsToUnload;
    {
        std::lock_guard<std::mutex> lock(preloadUIExtensionMapMutex_);
        for (auto it = preloadUIExtensionMap_.begin(); it != preloadUIExtensionMap_.end();) {
            if (std::get<HOST_PID_INDEX>(it->first) != callingPid) {
                ++it;
                continue;
            }
            ConvertToUnloadExtensionRecords(it->second, recordsToUnload);
            if (it->second.empty()) {
                it = preloadUIExtensionMap_.erase(it);
            } else {
                ++it;
            }
        }
    }
    for (const auto &record : recordsToUnload) {
        if (record != nullptr) {
            std::shared_ptr<ExtensionRecord> extensionRecord = record;
            HandlePreloadUIExtensionDestroyed(extensionRecord);
            record->UnloadUIExtensionAbility();
        }
    }
    return ERR_OK;
}

void ExtensionRecordManager::ConvertToUnloadExtensionRecords(
    std::vector<std::shared_ptr<ExtensionRecord>> &records,
    std::vector<std::shared_ptr<ExtensionRecord>> &recordsToUnload)
{
    for (auto recordIt = records.begin(); recordIt != records.end();) {
        if (*recordIt != nullptr) {
            recordsToUnload.push_back(*recordIt);
            recordIt = records.erase(recordIt);
        } else {
            ++recordIt;
        }
    }
}

void ExtensionRecordManager::RegisterPreloadUIExtensionHostClient(const sptr<IRemoteObject> &callerToken)
{
    std::lock_guard<std::mutex> lock(preloadUIExtensionHostClientMutex_);
    int32_t callerPid = IPCSkeleton::GetCallingPid();
    if (callerToken == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null callerToken");
        return;
    }
    preloadUIExtensionHostClientCallerTokens_[callerPid] = callerToken;
}

void ExtensionRecordManager::UnRegisterPreloadUIExtensionHostClient(
    int32_t key, const sptr<IRemoteObject::DeathRecipient> &deathRecipient)
{
    std::lock_guard<std::mutex> lock(preloadUIExtensionHostClientMutex_);
    auto it = preloadUIExtensionHostClientCallerTokens_.find(key);
    if (it != preloadUIExtensionHostClientCallerTokens_.end() && it->second != nullptr) {
        it->second->RemoveDeathRecipient(deathRecipient);
        preloadUIExtensionHostClientCallerTokens_.erase(it);
    }
}

int32_t ExtensionRecordManager::CheckAgentUILaunchLimit(int32_t callerUid, const std::string &bundleName)
{
    std::lock_guard<std::mutex> lock(agentUIExtensionMutex_);
    auto callerIt = agentUIExtensionRecords_.find(callerUid);
    if (callerIt == agentUIExtensionRecords_.end()) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "agentUI launch allowed");
        return ERR_OK;
    }
    auto& bundleMap = callerIt->second;
    auto bundleIt = bundleMap.find(bundleName);
    if (bundleIt == bundleMap.end()) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "agentUI launch allowed");
        return ERR_OK;
    }
    auto& extensionSet = bundleIt->second;

    if (extensionSet.size() >= AGENT_UI_MAX_COUNT) {
        TAG_LOGE(AAFwkTag::ABILITYMGR,
            "agentUI reach limit, callerUid:%{public}d, bundle:%{public}s, count:%{public}zu",
            callerUid, bundleName.c_str(), extensionSet.size());
        return ERR_OVERFLOW;
    }

    TAG_LOGD(AAFwkTag::ABILITYMGR, "agentUI launch allowed");
    return ERR_OK;
}

void ExtensionRecordManager::AddAgentUILaunchRecord(int32_t callerUid, const std::string &bundleName,
    int32_t extensionAbilityId)
{
    std::lock_guard<std::mutex> lock(agentUIExtensionMutex_);
    auto callerIt = agentUIExtensionRecords_.find(callerUid);
    if (callerIt == agentUIExtensionRecords_.end()) {
        auto& bundleMap = agentUIExtensionRecords_[callerUid];
        auto& extensionSet = bundleMap[bundleName];
        extensionSet.insert(extensionAbilityId);
        TAG_LOGI(AAFwkTag::ABILITYMGR,
            "agentUI added, callerUid:%{public}d, bundle:%{public}s, id:%{public}d, count:%{public}zu",
            callerUid, bundleName.c_str(), extensionAbilityId, extensionSet.size());
        return;
    }
    auto& bundleMap = callerIt->second;
    auto bundleIt = bundleMap.find(bundleName);
    if (bundleIt == bundleMap.end()) {
        auto& extensionSet = bundleMap[bundleName];
        extensionSet.insert(extensionAbilityId);
        TAG_LOGI(AAFwkTag::ABILITYMGR,
            "agentUI added, callerUid:%{public}d, bundle:%{public}s, id:%{public}d, count:%{public}zu",
            callerUid, bundleName.c_str(), extensionAbilityId, extensionSet.size());
        return;
    }
    auto& extensionSet = bundleIt->second;
    if (extensionSet.find(extensionAbilityId) == extensionSet.end()) {
        extensionSet.insert(extensionAbilityId);
        TAG_LOGI(AAFwkTag::ABILITYMGR,
            "agentUI added, callerUid:%{public}d, bundle:%{public}s, id:%{public}d, count:%{public}zu",
            callerUid, bundleName.c_str(), extensionAbilityId, extensionSet.size());
    }
}

void ExtensionRecordManager::RemoveAgentUILaunchRecord(int32_t callerUid, const std::string &bundleName,
    int32_t extensionAbilityId)
{
    std::lock_guard<std::mutex> lock(agentUIExtensionMutex_);
    auto callerIt = agentUIExtensionRecords_.find(callerUid);
    if (callerIt == agentUIExtensionRecords_.end()) {
        return;
    }
    auto& bundleMap = callerIt->second;
    auto bundleIt = bundleMap.find(bundleName);
    if (bundleIt == bundleMap.end()) {
        return;
    }
    auto& extensionSet = bundleIt->second;
    auto extIt = extensionSet.find(extensionAbilityId);
    if (extIt != extensionSet.end()) {
        extensionSet.erase(extIt);
        TAG_LOGI(AAFwkTag::ABILITYMGR,
            "agentUI removed, callerUid:%{public}d, bundle:%{public}s, id:%{public}d, count:%{public}zu",
            callerUid, bundleName.c_str(), extensionAbilityId, extensionSet.size());
    }
    if (extensionSet.empty()) {
        bundleMap.erase(bundleIt);
    }
    if (bundleMap.empty()) {
        agentUIExtensionRecords_.erase(callerIt);
    }
}
} // namespace AbilityRuntime
} // namespace OHOS
