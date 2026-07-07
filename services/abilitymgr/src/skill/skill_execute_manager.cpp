/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "skill_execute_manager.h"

#include <cinttypes>
#include <vector>

#include "ability_event_handler.h"
#include "ability_manager_errors.h"
#include "ability_manager_service.h"
#include "accesstoken_kit.h"
#include "bundle_constants.h"
#include "bundle_mgr_helper.h"
#include "hilog_tag_wrapper.h"
#include "in_process_call_wrapper.h"
#include "iservice_registry.h"
#include "permission_constants.h"
#include "permission_verification.h"
#include "system_ability_definition.h"
#ifdef SUPPORT_UPMS
#include "uri_permission_manager_client.h"
#endif

namespace OHOS {
namespace AAFwk {

SkillExecuteManager::SkillExecuteManager() {}

SkillExecuteManager::~SkillExecuteManager() {}

sptr<AppExecFwk::IBundleSkillManager> SkillExecuteManager::GetSkillManagerProxy()
{
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgr == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "failed to get SystemAbilityManager");
        return nullptr;
    }
    auto bmsObj = samgr->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    if (bmsObj == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "failed to get BMS from samgr");
        return nullptr;
    }
    auto bundleMgr = iface_cast<AppExecFwk::IBundleMgr>(bmsObj);
    if (bundleMgr == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "failed to cast IBundleMgr");
        return nullptr;
    }
    return bundleMgr->GetSkillManagerProxy();
}

int32_t SkillExecuteManager::QuerySkillInfo(const std::string &bundleName, const std::string &moduleName,
    const std::string &skillName, int32_t userId, AppExecFwk::SkillInfo &skillInfo)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR,
        "query skill info, bundle:%{public}s module:%{public}s skill:%{public}s",
        bundleName.c_str(), moduleName.c_str(), skillName.c_str());

    auto skillMgr = GetSkillManagerProxy();
    if (skillMgr == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "failed to get SkillManager proxy");
        return ERR_INVALID_VALUE;
    }

    uint32_t flags = static_cast<uint32_t>(AppExecFwk::SkillInfoFlag::GET_SKILL_INFO_WITH_SRC_ENTRIES) |
        static_cast<uint32_t>(AppExecFwk::SkillInfoFlag::GET_SKILL_INFO_WITH_PERMISSIONS) |
        static_cast<uint32_t>(AppExecFwk::SkillInfoFlag::GET_SKILL_INFO_WITH_REQUEST_PERMISSIONS);

    auto ret = skillMgr->GetSkillInfo(bundleName, moduleName, skillName, flags, userId, skillInfo);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "GetSkillInfo failed, ret:%{public}d", ret);
        return ret;
    }
    return ERR_OK;
}

int32_t SkillExecuteManager::CheckSkillPermission(const AppExecFwk::SkillInfo &skillInfo,
    uint32_t callerTokenId)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR,
        "check skill permission, skill:%{public}s",
        skillInfo.skillName.c_str());

    auto permVerif = PermissionVerification::GetInstance();
    if (!permVerif->IsSACall()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "direct caller is not SA");
        return ERR_NOT_SYSTEM_APP;
    }

    if (Security::AccessToken::AccessTokenKit::VerifyAccessToken(callerTokenId,
        PermissionConstants::PERMISSION_START_INVISIBLE_ABILITY, false) ==
        AppExecFwk::Constants::PERMISSION_GRANTED) {
        TAG_LOGI(AAFwkTag::ABILITYMGR,
            "caller has START_INVISIBLE_ABILITY, skip skill permission check, skill:%{public}s",
            skillInfo.skillName.c_str());
        return ERR_OK;
    }

    for (const auto &permission : skillInfo.permissions) {
        if (permission.empty()) {
            continue;
        }
        if (Security::AccessToken::AccessTokenKit::VerifyAccessToken(
            callerTokenId, permission, false) != AppExecFwk::Constants::PERMISSION_GRANTED) {
            TAG_LOGE(AAFwkTag::ABILITYMGR,
                "caller lacks permission:%{public}s for skill:%{public}s",
                permission.c_str(), skillInfo.skillName.c_str());
            return CHECK_PERMISSION_FAILED;
        }
    }
    return ERR_OK;
}

int32_t SkillExecuteManager::GenerateSkillWant(const AppExecFwk::SkillInfo &skillInfo, Want &want,
    int32_t userId, const std::string &requestCode, AppExecFwk::ExtensionAbilityType &targetType,
    const std::string &scriptPath, const std::string &functionName,
    const std::shared_ptr<AAFwk::WantParams> &skillArgs)
{
    std::string abilityName = skillInfo.abilityName;
    if (abilityName.empty()) {
        abilityName = ResolveDefaultAbilityName(skillInfo.bundleName, skillInfo.moduleName, userId);
        if (abilityName.empty()) {
            TAG_LOGE(AAFwkTag::ABILITYMGR,
                "no abilityName in skillProfile and no default ability found for bundle:%{public}s module:%{public}s",
                skillInfo.bundleName.c_str(), skillInfo.moduleName.c_str());
            return ERR_INVALID_VALUE;
        }
        TAG_LOGD(AAFwkTag::ABILITYMGR,
            "abilityName not specified, resolved default ability:%{public}s", abilityName.c_str());
    }

    targetType = ResolveTargetType(skillInfo.bundleName, skillInfo.moduleName, abilityName, userId);
    TAG_LOGD(AAFwkTag::ABILITYMGR,
        "generate skill want, bundle:%{public}s ability:%{public}s type:%{public}d",
        skillInfo.bundleName.c_str(), abilityName.c_str(), static_cast<int>(targetType));

    want.SetElementName("", skillInfo.bundleName, abilityName, skillInfo.moduleName);
    AppExecFwk::SkillExecuteParam::WriteToWant(want, skillInfo.bundleName, skillInfo.moduleName,
        skillInfo.skillName, scriptPath, functionName, skillArgs, skillInfo.srcEntries, requestCode,
        skillInfo.hapPath);
    return ERR_OK;
}

std::string SkillExecuteManager::CreateExecuteRecord(const sptr<IRemoteObject> &callerToken,
    const std::string &targetBundleName, const std::string &callerBundleName,
    uint32_t callerTokenId,
    const sptr<ISkillExecuteCallback> &callback,
    const std::string &externalRequestCode)
{
    std::lock_guard<ffrt::mutex> lock(mutex_);
    uint64_t currentSeq = ++requestCodeSeq_;
    std::string requestCode;
    if (!externalRequestCode.empty()) {
        requestCode = externalRequestCode;
    } else {
        requestCode = std::to_string(currentSeq);
    }
    auto record = std::make_shared<SkillExecuteRecord>();
    record->requestCode = requestCode;
    record->callerToken = callerToken;
    record->targetBundleName = targetBundleName;
    record->callerBundleName = callerBundleName;
    record->callerTokenId = callerTokenId;
    record->requestCodeSeq = currentSeq;
    record->state = SkillExecuteState::EXECUTING;
    record->callback = callback;

    if (callerToken != nullptr) {
        auto deathRecipient = sptr<CallerDeathRecipient>::MakeSptr(
            [this](const std::string &reqCode) { OnCallerDied(reqCode); }, requestCode);
        callerToken->AddDeathRecipient(deathRecipient);
        record->deathRecipient = deathRecipient;
    }

    records_[requestCode] = record;
    PostSkillExecuteTimeout(requestCode, currentSeq);
    EnsureAppStateObserverRegistered();
    TAG_LOGD(AAFwkTag::ABILITYMGR,
        "create execute record, requestCode:%{public}s", requestCode.c_str());
    return requestCode;
}

int32_t SkillExecuteManager::ExecuteSkillDone(const std::string &requestCode, int32_t resultCode,
    const AppExecFwk::SkillExecuteResult &result, const std::string &callerBundleName)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR,
        "execute skill done, requestCode:%{public}s code:%{public}d",
        requestCode.c_str(), resultCode);

    sptr<ISkillExecuteCallback> callback;
    {
        std::lock_guard<ffrt::mutex> lock(mutex_);
        auto it = records_.find(requestCode);
        if (it == records_.end()) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "record not found, requestCode:%{public}s",
                requestCode.c_str());
            return ERR_CODE_INVALID_ID;
        }

        auto record = it->second;
        if (record->targetBundleName != callerBundleName) {
            TAG_LOGE(AAFwkTag::ABILITYMGR,
                "bundleName %{public}s and %{public}s mismatch",
                callerBundleName.c_str(), record->targetBundleName.c_str());
            return ERR_INVALID_VALUE;
        }
        if (record->state != SkillExecuteState::EXECUTING) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "invalid state:%{public}d", static_cast<int>(record->state));
            return ERR_INVALID_VALUE;
        }

#ifdef SUPPORT_UPMS
        if (!result.uris.empty() && !record->callerBundleName.empty()) {
            std::vector<Uri> uriList;
            for (const auto &uriStr : result.uris) {
                uriList.emplace_back(uriStr);
            }
            auto &uriPermClient = UriPermissionManagerClient::GetInstance();
            auto ret = uriPermClient.GrantUriPermission(
                uriList, result.flags, record->callerBundleName, 0, record->callerTokenId);
            if (ret != ERR_OK) {
                TAG_LOGE(AAFwkTag::ABILITYMGR, "GrantUriPermission failed, ret:%{public}d", ret);
            }
        }
#endif

        record->state = SkillExecuteState::EXECUTE_DONE;
        callback = record->callback;
        RemoveRecord(requestCode);
    }

    if (callback != nullptr) {
        callback->OnExecuteDone(requestCode, resultCode, result);
    }
    return ERR_OK;
}

void SkillExecuteManager::RemoveRecord(const std::string &requestCode)
{
    auto it = records_.find(requestCode);
    if (it == records_.end()) {
        return;
    }
    auto record = it->second;
    if (record->callerToken != nullptr && record->deathRecipient != nullptr) {
        record->callerToken->RemoveDeathRecipient(record->deathRecipient);
    }
    records_.erase(it);
}

void SkillExecuteManager::OnCallerDied(const std::string &requestCode)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "caller died, requestCode:%{public}s", requestCode.c_str());
    std::lock_guard<ffrt::mutex> lock(mutex_);
    auto it = records_.find(requestCode);
    if (it != records_.end()) {
        RemoveSkillExecuteTimeoutLocked(it->second->requestCodeSeq);
        it->second->state = SkillExecuteState::REMOTE_DIED;
        RemoveRecord(requestCode);
    }
}

std::string SkillExecuteManager::ResolveDefaultAbilityName(const std::string &bundleName,
    const std::string &moduleName, int32_t userId)
{
    auto bundleMgrHelper = DelayedSingleton<AppExecFwk::BundleMgrHelper>::GetInstance();
    if (bundleMgrHelper == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "failed to get BundleMgrHelper");
        return "";
    }

    AppExecFwk::BundleInfo bundleInfo;
    auto flags = static_cast<int32_t>(AppExecFwk::GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_HAP_MODULE) |
        static_cast<int32_t>(AppExecFwk::GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_ABILITY);
    auto ret = IN_PROCESS_CALL(bundleMgrHelper->GetBundleInfoV9(bundleName, flags, bundleInfo, userId));
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "GetBundleInfoV9 failed for bundle:%{public}s",
            bundleName.c_str());
        return "";
    }

    for (const auto &hapModuleInfo : bundleInfo.hapModuleInfos) {
        if (hapModuleInfo.moduleName != moduleName) {
            continue;
        }
        std::string mainElement;
        if (hapModuleInfo.isModuleJson) {
            mainElement = hapModuleInfo.mainElementName;
        } else {
            mainElement = hapModuleInfo.mainAbility;
        }
        TAG_LOGD(AAFwkTag::ABILITYMGR,
            "resolved default ability for module:%{public}s",
            moduleName.c_str());
        return mainElement;
    }

    TAG_LOGE(AAFwkTag::ABILITYMGR, "module not found:%{public}s in bundle:%{public}s",
        moduleName.c_str(), bundleName.c_str());
    return "";
}

AppExecFwk::ExtensionAbilityType SkillExecuteManager::ResolveTargetType(const std::string &bundleName,
    const std::string &moduleName, const std::string &abilityName, int32_t userId)
{
    auto bundleMgrHelper = DelayedSingleton<AppExecFwk::BundleMgrHelper>::GetInstance();
    if (bundleMgrHelper == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "failed to get BundleMgrHelper");
        return AppExecFwk::ExtensionAbilityType::UNSPECIFIED;
    }

    Want queryWant;
    queryWant.SetElementName("", bundleName, abilityName, moduleName);
    std::vector<AppExecFwk::ExtensionAbilityInfo> extensionInfos;
    auto flags = static_cast<int32_t>(
        AppExecFwk::GetExtensionAbilityInfoFlag::GET_EXTENSION_ABILITY_INFO_DEFAULT);
    bool ret = IN_PROCESS_CALL(bundleMgrHelper->QueryExtensionAbilityInfos(
        queryWant, flags, userId, extensionInfos));
    if (ret && !extensionInfos.empty()) {
        auto type = extensionInfos[0].type;
        TAG_LOGD(AAFwkTag::ABILITYMGR,
            "abilityName:%{public}s is extension, type:%{public}d",
            abilityName.c_str(), static_cast<int>(type));
        return type;
    }

    TAG_LOGD(AAFwkTag::ABILITYMGR,
        "abilityName:%{public}s is not extension, default to UIAbility", abilityName.c_str());
    return AppExecFwk::ExtensionAbilityType::UNSPECIFIED;
}

void SkillExecuteManager::PostSkillExecuteTimeout(
    const std::string &requestCode, uint64_t requestCodeSeq)
{
    auto handler = DelayedSingleton<AbilityManagerService>::GetInstance()->GetEventHandler();
    if (handler == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null event handler");
        return;
    }
    uint32_t timeout = static_cast<uint32_t>(
        AmsConfigurationParameter::GetInstance().GetAppStartTimeoutTime()) *
        static_cast<uint32_t>(GlobalConstant::SKILL_EXECUTE_TIMEOUT_MULTIPLE);
    seqToRequestCodeMap_[requestCodeSeq] = requestCode;
    auto event = EventWrap(AbilityManagerService::SKILL_EXECUTE_TIMEOUT_MSG,
        static_cast<int64_t>(requestCodeSeq));
    event.SetTimeout(timeout);
    handler->SendEvent(event, timeout, false);
}

void SkillExecuteManager::RemoveSkillExecuteTimeoutLocked(uint64_t requestCodeSeq)
{
    auto handler = DelayedSingleton<AbilityManagerService>::GetInstance()->GetEventHandler();
    if (handler != nullptr) {
        handler->RemoveEvent(AbilityManagerService::SKILL_EXECUTE_TIMEOUT_MSG,
            static_cast<int64_t>(requestCodeSeq));
    }
    seqToRequestCodeMap_.erase(requestCodeSeq);
}

void SkillExecuteManager::OnTimeout(int64_t requestCodeSeq)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "called, seq:%{public}" PRId64, requestCodeSeq);

    sptr<ISkillExecuteCallback> callback;
    std::string requestCode;
    {
        std::lock_guard<ffrt::mutex> lock(mutex_);
        auto seqIt = seqToRequestCodeMap_.find(requestCodeSeq);
        if (seqIt == seqToRequestCodeMap_.end()) {
            TAG_LOGW(AAFwkTag::ABILITYMGR, "seq not found, seq:%{public}" PRId64, requestCodeSeq);
            return;
        }
        requestCode = seqIt->second;
        seqToRequestCodeMap_.erase(seqIt);

        auto it = records_.find(requestCode);
        if (it == records_.end()) {
            return;
        }
        auto &record = it->second;
        if (record->state != SkillExecuteState::EXECUTING) {
            return;
        }

        TAG_LOGW(AAFwkTag::ABILITYMGR, "skill execute timed out, req:%{public}s", requestCode.c_str());
        record->state = SkillExecuteState::TIMED_OUT;
        callback = record->callback;
        RemoveRecord(requestCode);
    }

    if (callback != nullptr) {
        AppExecFwk::SkillExecuteResult emptyResult;
        emptyResult.code = ERR_TIMED_OUT;
        callback->OnExecuteDone(requestCode, ERR_TIMED_OUT, emptyResult);
    }
}

void SkillExecuteManager::EnsureAppStateObserverRegistered()
{
    if (appStateObserver_ != nullptr) {
        return;
    }
    auto appManager = AppMgrUtil::GetAppMgr();
    if (appManager == nullptr) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "null appManager, will retry on next record");
        return;
    }
    auto observer = sptr<SkillAppStateObserver>::MakeSptr(
        [](const std::string &bundleName) {
            DelayedSingleton<SkillExecuteManager>::GetInstance()->OnTargetProcessDied(bundleName);
        });
    if (observer == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "new SkillAppStateObserver failed");
        return;
    }
    auto err = appManager->RegisterApplicationStateObserver(observer);
    if (err != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "register app state observer err:%{public}d", err);
        return;
    }
    appStateObserver_ = observer;
    TAG_LOGI(AAFwkTag::ABILITYMGR, "app state observer registered");
}

void SkillExecuteManager::OnLaunchCompleted(const std::string &requestCode)
{
    std::lock_guard<ffrt::mutex> lock(mutex_);
    auto it = records_.find(requestCode);
    if (it == records_.end()) {
        TAG_LOGW(AAFwkTag::ABILITYMGR,
            "OnLaunchCompleted: record gone, req:%{public}s", requestCode.c_str());
        return;
    }
    RemoveSkillExecuteTimeoutLocked(it->second->requestCodeSeq);
    TAG_LOGD(AAFwkTag::ABILITYMGR,
        "launch completed, keep record for async result, req:%{public}s", requestCode.c_str());
}

void SkillExecuteManager::OnLaunchFailed(const std::string &requestCode, int32_t errCode)
{
    std::lock_guard<ffrt::mutex> lock(mutex_);
    auto it = records_.find(requestCode);
    if (it == records_.end()) {
        TAG_LOGW(AAFwkTag::ABILITYMGR,
            "OnLaunchFailed: record gone, req:%{public}s", requestCode.c_str());
        return;
    }
    auto record = it->second;
    RemoveSkillExecuteTimeoutLocked(record->requestCodeSeq);
    if (record->state != SkillExecuteState::EXECUTING) {
        TAG_LOGW(AAFwkTag::ABILITYMGR,
            "OnLaunchFailed: invalid state:%{public}d", static_cast<int>(record->state));
        return;
    }
    record->state = SkillExecuteState::REMOTE_DIED;
    if (record->callback != nullptr) {
        AppExecFwk::SkillExecuteResult emptyResult;
        emptyResult.code = errCode;
        record->callback->OnExecuteDone(requestCode, errCode, emptyResult);
    }
    RemoveRecord(requestCode);
}

void SkillExecuteManager::OnTargetProcessDied(const std::string &bundleName)
{
    std::lock_guard<ffrt::mutex> lock(mutex_);
    std::vector<std::string> hitCodes;
    for (const auto &entry : records_) {
        const auto &record = entry.second;
        if (record->targetBundleName == bundleName &&
            record->state == SkillExecuteState::EXECUTING) {
            hitCodes.push_back(entry.first);
        }
    }
    if (hitCodes.empty()) {
        return;
    }
    TAG_LOGW(AAFwkTag::ABILITYMGR,
        "target process died, bundle:%{public}s, hit %{public}zu record(s)",
        bundleName.c_str(), hitCodes.size());
    for (const auto &requestCode : hitCodes) {
        auto it = records_.find(requestCode);
        if (it == records_.end()) {
            continue;
        }
        auto record = it->second;
        record->state = SkillExecuteState::REMOTE_DIED;
        if (record->callback != nullptr) {
            AppExecFwk::SkillExecuteResult emptyResult;
            emptyResult.code = ERR_SKILL_EXECUTE_TARGET_DIED;
            record->callback->OnExecuteDone(requestCode, ERR_SKILL_EXECUTE_TARGET_DIED, emptyResult);
        }
        RemoveRecord(requestCode);
    }
}

} // namespace AAFwk
} // namespace OHOS
