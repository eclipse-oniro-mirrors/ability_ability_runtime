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

#include "modular_object_utils.h"

#include <singleton.h>

#include "ability_manager_errors.h"
#include "ability_manager_service.h"
#include "ability_record/ability_record_utils.h"
#include "ability_util.h"
#include "base_extension_record.h"
#include "app_mgr_client.h"
#include "app_utils.h"
#include "bundle_mgr_helper.h"
#include "hilog_tag_wrapper.h"
#include "ipc_skeleton.h"
#include "modular_object_manager.h"
#include "modular_object_rdb_storage_mgr.h"
#include "os_account_manager_wrapper.h"
#include "parameters.h"
#include "permission_verification.h"
#include "rate_limiter.h"
#include "running_process_info.h"
#include "scene_board_judgement.h"

namespace {
constexpr int32_t MOE_MAX_CONNECTIONS_PER_CALLER = 5;
constexpr int32_t MOE_MAX_INSTANCES = 20;
}

using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AAFwk {
int32_t ModularObjectUtils::CheckRateLimit()
{
    if (AAFwk::PermissionVerification::GetInstance()->IsSACall()) {
        return ERR_OK;
    }
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    if (RateLimiter::GetInstance().CheckModularObjectLimit(callingUid)) {
        TAG_LOGW(AAFwkTag::EXT, "moe rate limit exceeded, uid:%{public}d", callingUid);
        return ERR_FREQ_START_ABILITY;
    }
    return ERR_OK;
}

int32_t ModularObjectUtils::VerifyExported(const AbilityRequest &abilityRequest)
{
    AAFwk::PermissionVerification::VerificationInfo verificationInfo;
    verificationInfo.accessTokenId = abilityRequest.appInfo.accessTokenId;
    verificationInfo.visible = abilityRequest.abilityInfo.visible;
    auto result = AAFwk::PermissionVerification::GetInstance()->
        CheckCallModularObjectExtensionPermission(verificationInfo);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::EXT, "VerifyExported failed: %{public}d", result);
    }
    return result;
}

int32_t ModularObjectUtils::CheckPermission(const AbilityRequest &abilityRequest)
{
    if (!AppUtils::GetInstance().IsSupportModularObjectExtension()) {
        TAG_LOGE(AAFwkTag::EXT, "device not supported");
        return ERR_CAPABILITY_NOT_SUPPORT;
    }
    auto ret = VerifyExported(abilityRequest);
    if (ret != ERR_OK) {
        return ret;
    }
    int32_t validUserId = abilityRequest.userId;
    auto element = abilityRequest.want.GetElement();
    std::string bundleName = element.GetBundleName();
    std::string abilityName = element.GetAbilityName();
    int32_t appIndex = abilityRequest.want.GetIntParam(Want::PARAM_APP_CLONE_INDEX_KEY, 0);

    ModularObjectExtensionInfo targetExtensionInfo;
    ret = GetTargetExtensionInfoFromDb(bundleName, abilityName, appIndex, validUserId, targetExtensionInfo);
    if (ret != ERR_OK) {
        return ret;
    }
    ret = CheckExtensionEnabled(targetExtensionInfo, abilityRequest);
    if (ret != ERR_OK) {
        return ret;
    }
    ret = CheckInProcessLaunchMode(targetExtensionInfo.launchMode, abilityRequest.uid);
    if (ret != ERR_OK) {
        return ret;
    }
    ret = CheckCallerForeground();
    if (ret != ERR_OK) {
        return ret;
    }
    AppExecFwk::ApplicationInfo callerAppInfo;
    ret = GetCallerAppInfo(callerAppInfo);
    if (ret != ERR_OK) {
        return ret;
    }
    const auto &targetAppInfo = abilityRequest.appInfo;
    ret = CheckAppDistributionType(callerAppInfo.appDistributionType, targetAppInfo.appDistributionType);
    if (ret != ERR_OK) {
        return ret;
    }
    return CheckTargetHasRunningAbility(targetAppInfo.uid, validUserId, bundleName);
}

int32_t ModularObjectUtils::CheckExtensionEnabled(const ModularObjectExtensionInfo &info,
    const AbilityRequest &abilityRequest)
{
    if (info.isDisabled && IPCSkeleton::GetCallingUid() != abilityRequest.uid) {
        TAG_LOGE(AAFwkTag::EXT, "Extension is disabled: %{public}s/%{public}s. targetUid:%{public}d",
            info.bundleName.c_str(), info.abilityName.c_str(), abilityRequest.uid);
        return ERR_MODULAR_OBJECT_DISABLED;
    }
    return ERR_OK;
}

int32_t ModularObjectUtils::CheckInProcessLaunchMode(MoeLaunchMode launchMode, int32_t targetUid)
{
    if (launchMode != MoeLaunchMode::IN_PROCESS) {
        return ERR_OK;
    }
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    if (callingUid != targetUid) {
        TAG_LOGE(AAFwkTag::EXT, "IN_PROCESS not support cross-app, callerUid: %{public}d, targetUid: %{public}d",
            callingUid, targetUid);
        return ERR_MOE_CROSS_APP_IN_PROCESS;
    }
    return ERR_OK;
}

int32_t ModularObjectUtils::CheckCallerForeground()
{
    pid_t callingPid = IPCSkeleton::GetCallingPid();
    AppExecFwk::RunningProcessInfo processInfo;
    auto ret = IN_PROCESS_CALL(
        DelayedSingleton<AppExecFwk::AppMgrClient>::GetInstance()->GetRunningProcessInfoByPid(
            callingPid, processInfo));
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::EXT, "GetRunningProcessInfoByPid fail:%{public}d", ret);
        return ret;
    }
    if (processInfo.state_ != AppExecFwk::AppProcessState::APP_STATE_FOREGROUND) {
        TAG_LOGE(AAFwkTag::EXT, "Caller not foreground, callingPid: %{public}d, state: %{public}d",
            callingPid, static_cast<int32_t>(processInfo.state_));
        return NOT_TOP_ABILITY;
    }
    if (processInfo.isPreForeground) {
        TAG_LOGE(AAFwkTag::EXT, "Caller is preForeground");
        return NOT_TOP_ABILITY;
    }
    return ERR_OK;
}

int32_t ModularObjectUtils::CheckAppDistributionType(const std::string &callerAppDistributionType,
    const std::string &targetAppDistributionType)
{
    bool isDeveloperMode = system::GetBoolParameter("const.security.developermode.state", false);
    if (isDeveloperMode) {
        TAG_LOGD(AAFwkTag::EXT, "Developer mode, allow");
        return ERR_OK;
    }

    if (callerAppDistributionType == "none") {
        TAG_LOGE(AAFwkTag::EXT, "Caller appDistributionType is none, not allowed");
        return ERR_INVALID_DISTRIBUTION_TYPE;
    }
    if (targetAppDistributionType == "none") {
        TAG_LOGE(AAFwkTag::EXT, "Target appDistributionType is none, not allowed");
        return ERR_INVALID_DISTRIBUTION_TYPE;
    }
    return ERR_OK;
}

bool ModularObjectUtils::HasRunningUIAbilityOrExtension(int32_t targetUid, int32_t userId)
{
    auto service = DelayedSingleton<AbilityManagerService>::GetInstance();
    if (service == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "service is null");
        return false;
    }
    std::vector<std::string> abilityList;
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        auto uiAbilityManager = service->GetUIAbilityManagerByUserId(userId);
        if (uiAbilityManager) {
            uiAbilityManager->GetActiveAbilityList(targetUid, abilityList);
        }
    } else {
        auto missionListManager = service->GetMissionListManagerByUserId(userId);
        if (missionListManager) {
            missionListManager->GetActiveAbilityList(targetUid, abilityList);
        }
    }
    if (!abilityList.empty()) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "Found running UIAbility for uid: %{public}d", targetUid);
        return true;
    }

    auto uiExtManager = service->GetUIExtensionAbilityManagerByUserId(userId);
    if (uiExtManager) {
        std::vector<std::string> extensionList;
        uiExtManager->GetActiveUIExtensionListByUid(targetUid, extensionList);
        if (!extensionList.empty()) {
            TAG_LOGD(AAFwkTag::ABILITYMGR, "Found running UIExtension for uid: %{public}d", targetUid);
            return true;
        }
    }

    TAG_LOGI(AAFwkTag::ABILITYMGR, "No UIAbility or UIExt for uid: %{public}d", targetUid);
    return false;
}

int32_t ModularObjectUtils::CheckTargetHasRunningAbility(
    int32_t targetUid, int32_t userId, const std::string &targetBundleName)
{
    if (!HasRunningUIAbilityOrExtension(targetUid, userId)) {
        TAG_LOGE(AAFwkTag::EXT,
            "Target has no running UIAbility or UIExtension, uid: %{public}d, bundle: %{public}s",
            targetUid, targetBundleName.c_str());
        return ERR_NO_RUNNING_ABILITIES_WITH_UI;
    }
    return ERR_OK;
}

int32_t ModularObjectUtils::GetTargetExtensionInfoFromDb(const std::string &bundleName,
    const std::string &abilityName, int32_t appIndex, int32_t validUserId,
    ModularObjectExtensionInfo &targetExtensionInfo)
{
    std::string key = std::to_string(validUserId) + "_" + bundleName + "_" + std::to_string(appIndex);
    std::vector<ModularObjectExtensionInfo> infos;
    auto ret = DelayedSingleton<ModularObjectExtensionRdbStorageMgr>::GetInstance()->QueryData(key, infos);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::EXT, "QueryData failed, ret: %{public}d", ret);
        return ret;
    }
    bool found = false;
    for (const auto &info : infos) {
        if (info.bundleName == bundleName && info.abilityName == abilityName) {
            targetExtensionInfo = info;
            found = true;
            break;
        }
    }
    if (!found) {
        TAG_LOGE(AAFwkTag::EXT, "Extension not found: %{public}s/%{public}s", bundleName.c_str(), abilityName.c_str());
        return RESOLVE_ABILITY_ERR;
    }
    return ERR_OK;
}

int32_t ModularObjectUtils::GetCallerAppInfo(AppExecFwk::ApplicationInfo &callerAppInfo)
{
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    auto bundleMgrHelper = DelayedSingleton<AppExecFwk::BundleMgrHelper>::GetInstance();
    CHECK_POINTER_AND_RETURN(bundleMgrHelper, INNER_ERR);
    std::string callerBundleName;
    int32_t callerAppIndex = 0;
    auto ret = IN_PROCESS_CALL(bundleMgrHelper->GetNameAndIndexForUid(callingUid, callerBundleName, callerAppIndex));
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::EXT, "Get caller info failed, callingUid: %{public}d, ret: %{public}d", callingUid, ret);
        return INNER_ERR;
    }
    int32_t callerUserId = -1;
    auto osAccountRet = DelayedSingleton<AppExecFwk::OsAccountManagerWrapper>::GetInstance()
        ->GetOsAccountLocalIdFromUid(callingUid, callerUserId);
    if (osAccountRet != 0) {
        TAG_LOGE(AAFwkTag::EXT, "getUserId fail, callingUid: %{public}d, ret:%{public}d", callingUid, osAccountRet);
        return INNER_ERR;
    }
    if (!IN_PROCESS_CALL(bundleMgrHelper->GetApplicationInfoWithAppIndex(
        callerBundleName, callerAppIndex, callerUserId, callerAppInfo))) {
        TAG_LOGE(AAFwkTag::EXT, "Get caller appInfo failed, bundle: %{public}s", callerBundleName.c_str());
        return INNER_ERR;
    }
    return ERR_OK;
}

bool ModularObjectUtils::GetPidToCheckByCallerToken(sptr<IRemoteObject> callerToken, pid_t &outPid)
{
    if (callerToken == nullptr) {
        return false;
    }
    auto callerRecord = Token::GetAbilityRecordByToken(callerToken);
    if (callerRecord == nullptr) {
        TAG_LOGW(AAFwkTag::EXT, "callerToken invalid, use original PID");
        return false;
    }
    auto &abilityInfo = callerRecord->GetAbilityInfo();
    if (abilityInfo.extensionAbilityType != AppExecFwk::ExtensionAbilityType::MODULAR_OBJECT) {
        return false;
    }
    auto extensionRecord = std::static_pointer_cast<BaseExtensionRecord>(callerRecord);
    auto clientPid = extensionRecord->GetClientPid();
    if (clientPid > 0) {
        outPid = clientPid;
        TAG_LOGD(AAFwkTag::EXT, "MOE: use client PID=%{public}d", clientPid);
        return true;
    }
    return false;
}

std::shared_ptr<ModularObjectExtensionInfo> ModularObjectUtils::QueryConfig(const AbilityRequest &abilityRequest)
{
    auto mgr = DelayedSingleton<AbilityRuntime::ModularObjectManager>::GetInstance();
    if (mgr == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "ModularObjectManager is null");
        return nullptr;
    }
    std::vector<ModularObjectExtensionInfo> infos;
    int32_t userId = abilityRequest.userId;
    const auto &bundleName = abilityRequest.abilityInfo.bundleName;
    int32_t appIndex = abilityRequest.appInfo.appIndex;
    if (mgr->QuerySelfModularObjectExtensionInfos(userId, bundleName, appIndex, infos) != ERR_OK) {
        TAG_LOGD(AAFwkTag::EXT, "query modular object infos failed");
        return nullptr;
    }
    const auto &abilityName = abilityRequest.abilityInfo.name;
    for (const auto &info : infos) {
        if (info.abilityName == abilityName) {
            return std::make_shared<ModularObjectExtensionInfo>(info);
        }
    }
    return nullptr;
}

int32_t ModularObjectUtils::SetupNewRecord(const AbilityRequest &abilityRequest,
    std::shared_ptr<BaseExtensionRecord> &targetService, const std::string &serviceKey)
{
    if (targetService == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "targetService is null");
        return ERR_INVALID_VALUE;
    }
    auto config = QueryConfig(abilityRequest);
    if (config == nullptr) {
        return ERR_INVALID_VALUE;
    }
    // Determine processName
    std::string process;
    if (config->launchMode == MoeLaunchMode::IN_PROCESS) {
        targetService->SetIsInProcess(true);
        pid_t callingPid = IPCSkeleton::GetCallingPid();
        AppExecFwk::RunningProcessInfo processInfo;
        auto procRet = IN_PROCESS_CALL(
            DelayedSingleton<AppExecFwk::AppMgrClient>::GetInstance()->GetRunningProcessInfoByPid(
                callingPid, processInfo));
        if (procRet != ERR_OK || processInfo.processName_.empty()) {
            TAG_LOGE(AAFwkTag::EXT, "GetRunningProcessInfoByPid failed in IN_PROCESS mode, ret:%{public}d", procRet);
            return INNER_ERR;
        }
        process = processInfo.processName_;
    } else {
        switch (config->processMode) {
            case MoeProcessMode::BUNDLE:
                process = abilityRequest.abilityInfo.bundleName + ":" +
                    abilityRequest.abilityInfo.extensionTypeName;
                break;
            case MoeProcessMode::TYPE:
                process = abilityRequest.abilityInfo.bundleName + ":" +
                    abilityRequest.abilityInfo.name;
                break;
            case MoeProcessMode::INSTANCE:
                process = abilityRequest.abilityInfo.bundleName + ":" +
                    abilityRequest.abilityInfo.name + ":" +
                    std::to_string(targetService->GetRecordId());
                break;
            default:
                break;
        }
        int32_t appCloneIndex = abilityRequest.appInfo.appIndex;
        if (appCloneIndex > 0) {
            process = process + ":" + std::to_string(appCloneIndex);
        }
    }
    if (!process.empty()) {
        targetService->SetProcessName(process);
        TAG_LOGI(AAFwkTag::EXT, "ModularObject processName: %{public}s", process.c_str());
    }
    // Save requestId for disconnect serviceKey reconstruction
    auto pos = serviceKey.rfind('_');
    if (pos != std::string::npos) {
        targetService->SetRequestId(serviceKey.substr(pos + 1));
    }
    return ERR_OK;
}

int32_t ModularObjectUtils::CheckLimits(int32_t instanceCount, int32_t connectionCount)
{
    if (instanceCount >= MOE_MAX_INSTANCES) {
        TAG_LOGE(AAFwkTag::EXT, "MoeAbility instance limit reached, count: %{public}d", instanceCount);
        return ERR_MOE_INSTANCE_LIMIT;
    }
    if (connectionCount >= MOE_MAX_CONNECTIONS_PER_CALLER) {
        TAG_LOGE(AAFwkTag::EXT, "MoeAbility connection limit reached, count: %{public}d", connectionCount);
        return ERR_MOE_CONNECTION_LIMIT;
    }
    return ERR_OK;
}
}  // namespace AAFwk
}  // namespace OHOS
