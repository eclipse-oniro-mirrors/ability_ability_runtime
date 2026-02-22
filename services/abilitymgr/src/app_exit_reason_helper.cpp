/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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

#include "app_exit_reason_helper.h"

#include "accesstoken_kit.h"
#include "app_exit_reason_data_manager.h"
#include "app_mgr_util.h"
#include "bundle_mgr_helper.h"
#include "exit_info_data_manager.h"
#include "hitrace_meter.h"
#include "os_account_manager_wrapper.h"
#include "scene_board_judgement.h"

namespace OHOS {
namespace AAFwk {
namespace {
constexpr int32_t INVALID_KILLID = -2;
}

AppExitReasonHelper::AppExitReasonHelper(std::shared_ptr<SubManagersHelper> subManagersHelper)
    : subManagersHelper_(subManagersHelper) {}

int32_t AppExitReasonHelper::RecordAppWithReason(int32_t pid, int32_t uid, const ExitReasonCompability &exitReason)
{
    std::string bundleName;
    int32_t appIndex = 0;
    auto ret = IN_PROCESS_CALL(AbilityUtil::GetBundleManagerHelper()->GetNameAndIndexForUid(uid, bundleName, appIndex));
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "GetNameAndIndexForUid failed, ret: %{public}d", ret);
        return ret;
    }
    
    AppExecFwk::RunningProcessInfo processInfo;
    int32_t userId = -1;
    int32_t getOsAccountRet = DelayedSingleton<AppExecFwk::OsAccountManagerWrapper>::GetInstance()->
        GetOsAccountLocalIdFromUid(uid, userId);
    if (getOsAccountRet != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "get GetOsAccountLocalIdFromUid failed. ret: %{public}d", getOsAccountRet);
        return getOsAccountRet;
    }
    GetRunningProcessInfo(pid, userId, bundleName, processInfo);
    ExitReason tmpReason(exitReason.reason, exitReason.subReason, exitReason.exitMsg);
    int32_t resultCode = RecordProcessExtensionExitReason(pid, bundleName, tmpReason, processInfo, false);
    if (resultCode != ERR_OK) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "not record extension reason: %{public}d", resultCode);
    }
    std::vector<std::string> abilityList;
    int32_t getActiveAbilityListRet = GetActiveAbilityListWithPid(uid, abilityList, pid);
    if (getActiveAbilityListRet != ERR_OK) {
        return getActiveAbilityListRet;
    }
    if (abilityList.empty()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "abilityLists empty");
        return ERR_GET_ACTIVE_ABILITY_LIST_EMPTY;
    }
    ret = DelayedSingleton<AppScheduler>::GetInstance()->NotifyAppMgrRecordExitReasonCompability(
        pid, exitReason.killId, exitReason.killMsg, exitReason.innerMsg);
    if (ret != ERR_OK) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "notify failed: %{public}d", ret);
    }
    TAG_LOGD(AAFwkTag::ABILITYMGR,
        "userId: %{public}d, bundleName: %{public}s, appIndex: %{public}d", userId, bundleName.c_str(), appIndex);
    uint32_t accessTokenId = Security::AccessToken::AccessTokenKit::GetHapTokenID(userId, bundleName, appIndex);
    return DelayedSingleton<AbilityRuntime::AppExitReasonDataManager>::GetInstance()->SetAppExitReason(bundleName,
        accessTokenId, abilityList, tmpReason, processInfo, false);
}

int32_t AppExitReasonHelper::RecordAppExitReason(const ExitReason &exitReason)
{
    if (!IsExitReasonValid(exitReason)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "exit reason invalid");
        return ERR_INVALID_VALUE;
    }
    auto uid = IPCSkeleton::GetCallingUid();
    std::string bundleName;
    int32_t appIndex = 0;
    auto ret = IN_PROCESS_CALL(AbilityUtil::GetBundleManagerHelper()->GetNameAndIndexForUid(uid, bundleName, appIndex));
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "GetNameAndIndexForUid failed, ret: %{public}d", ret);
        return ret;
    }
    
    int32_t pid = exitReason.reason != Reason::REASON_CPP_CRASH ? IPCSkeleton::GetCallingPid() : NO_PID;
    AppExecFwk::RunningProcessInfo processInfo;
    int32_t userId = -1;
    int32_t getOsAccountRet = DelayedSingleton<AppExecFwk::OsAccountManagerWrapper>::GetInstance()->
        GetOsAccountLocalIdFromUid(uid, userId);
    GetRunningProcessInfo(pid, userId, bundleName, processInfo);
    int32_t resultCode = RecordProcessExtensionExitReason(pid, bundleName, exitReason, processInfo, false);
    if (resultCode != ERR_OK) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "not record extension reason: %{public}d", resultCode);
    }
    std::vector<std::string> abilityList;
    int32_t getActiveAbilityListRet = GetActiveAbilityListWithPid(uid, abilityList, pid);
    if (getActiveAbilityListRet != ERR_OK) {
        return getActiveAbilityListRet;
    }
    ret = DelayedSingleton<AppScheduler>::GetInstance()->NotifyAppMgrRecordExitReason(IPCSkeleton::GetCallingPid(),
        exitReason.reason, exitReason.exitMsg);
    if (ret != ERR_OK) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "notify ret: %{public}d", ret);
    }
    if (abilityList.empty()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "abilityLists empty");
        return ERR_GET_ACTIVE_ABILITY_LIST_EMPTY;
    }
    if (getOsAccountRet != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "get GetOsAccountLocalIdFromUid failed. ret: %{public}d", getOsAccountRet);
        return ERR_INVALID_VALUE;
    }
    TAG_LOGD(AAFwkTag::ABILITYMGR,
        "userId: %{public}d, bundleName: %{public}s, appIndex: %{public}d", userId, bundleName.c_str(), appIndex);
    uint32_t accessTokenId = Security::AccessToken::AccessTokenKit::GetHapTokenID(userId, bundleName, appIndex);
    return DelayedSingleton<AbilityRuntime::AppExitReasonDataManager>::GetInstance()->SetAppExitReason(bundleName,
        accessTokenId, abilityList, exitReason, processInfo, false);
}

int32_t AppExitReasonHelper::RecordProcessExitReasonForTimeout(const AppExecFwk::AbilityInfo &abilityInfo,
    const ExitReason &exitReason, const std::vector<std::string> &abilityList,
    const AppExecFwk::RunningProcessInfo &processInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto pid = processInfo.pid_;
    if (!IsExitReasonValid(exitReason) || pid <= 0) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "invalid params");
        return ERR_INVALID_VALUE;
    }

    std::string bundleName = abilityInfo.bundleName;
    auto uid = abilityInfo.applicationInfo.uid;
    auto accessTokenId = abilityInfo.applicationInfo.accessTokenId;
    int32_t resultCode = RecordProcessExtensionExitReason(pid, bundleName, exitReason, processInfo, false);
    if (resultCode != ERR_OK) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "not record extension reason: %{public}d", resultCode);
    }

    int32_t targetUserId;
    int32_t getOsAccountRet = DelayedSingleton<AppExecFwk::OsAccountManagerWrapper>::GetInstance()->
        GetOsAccountLocalIdFromUid(uid, targetUserId);
    if (getOsAccountRet != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "get GetOsAccountLocalIdFromUid failed. ret: %{public}d", getOsAccountRet);
        return ERR_INVALID_VALUE;
    }
    TAG_LOGD(AAFwkTag::ABILITYMGR, "targetUserId: %{public}d", targetUserId);

    auto ret = DelayedSingleton<AppScheduler>::GetInstance()->NotifyAppMgrRecordExitReason(pid, exitReason.reason,
        exitReason.exitMsg);
    if (ret != ERR_OK) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "notify ret:%{public}d", ret);
    }

    if (abilityList.empty()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "active abilityList empty");
        return ERR_GET_ACTIVE_ABILITY_LIST_EMPTY;
    }
    return DelayedSingleton<AbilityRuntime::AppExitReasonDataManager>::GetInstance()->SetAppExitReason(bundleName,
        accessTokenId, abilityList, exitReason, processInfo, false);
}

int32_t AppExitReasonHelper::RecordProcessExitReason(const int32_t pid, const ExitReason &exitReason,
    bool fromKillWithReason)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    AppExecFwk::ApplicationInfo application;
    bool debug = false;
    auto ret = IN_PROCESS_CALL(DelayedSingleton<AppScheduler>::GetInstance()->GetApplicationInfoByProcessID(pid,
        application, debug));
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "getApplicationInfoByProcessID failed");
        return ret;
    }
    auto bundleName = application.bundleName;
    AppExecFwk::RunningProcessInfo processInfo;
    if (pid > 0) {
        DelayedSingleton<AppScheduler>::GetInstance()->GetRunningProcessInfoByPid(
            static_cast<pid_t>(pid), processInfo);
    }
    int32_t resultCode = RecordProcessExtensionExitReason(pid, bundleName, exitReason, processInfo,
        fromKillWithReason);
    if (resultCode != ERR_OK) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "not record extension reason: %{public}d", resultCode);
    }

    return RecordProcessExitReason(pid, bundleName, application.uid, application.accessTokenId, exitReason,
        processInfo, fromKillWithReason);
}

int32_t AppExitReasonHelper::RecordAppExitReason(const std::string &bundleName, int32_t uid, int32_t appIndex,
    const ExitReason &exitReason)
{
    int32_t userId;
    int32_t getOsAccountRet = DelayedSingleton<AppExecFwk::OsAccountManagerWrapper>::GetInstance()->
        GetOsAccountLocalIdFromUid(uid, userId);
    if (getOsAccountRet != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "get GetOsAccountLocalIdFromUid failed. ret: %{public}d", getOsAccountRet);
        return ERR_INVALID_VALUE;
    }
    TAG_LOGD(AAFwkTag::ABILITYMGR,
        "userId: %{public}d, bundleName: %{public}s, appIndex: %{public}d", userId, bundleName.c_str(), appIndex);
    uint32_t accessTokenId = Security::AccessToken::AccessTokenKit::GetHapTokenID(userId, bundleName, appIndex);
    AppExecFwk::RunningProcessInfo processInfo;
    GetRunningProcessInfo(NO_PID, userId, bundleName, processInfo);
    return RecordProcessExitReason(NO_PID, bundleName, uid, accessTokenId, exitReason, processInfo, false);
}

int32_t AppExitReasonHelper::RecordProcessExitReason(int32_t pid, int32_t uid, const ExitReason &exitReason)
{
    uint32_t accessTokenId = 0;
    AbilityRuntime::ExitCacheInfo cacheInfo = {};
    if (!AbilityRuntime::ExitInfoDataManager::GetInstance().GetExitInfo(pid, uid, accessTokenId, cacheInfo)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "not found, pid: %{public}d, uid: %{public}d", pid, uid);
        return ERR_NO_APP_RECORD;
    }
    return DelayedSingleton<AbilityRuntime::AppExitReasonDataManager>::GetInstance()->SetAppExitReason(
        cacheInfo.bundleName, accessTokenId, cacheInfo.abilityNames, exitReason, cacheInfo.exitInfo, false);
}

int32_t AppExitReasonHelper::RecordProcessExitReason(const int32_t pid, const std::string bundleName,
    const int32_t uid, const uint32_t accessTokenId, const ExitReason &exitReason,
    const AppExecFwk::RunningProcessInfo &processInfo, bool fromKillWithReason)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (!IsExitReasonValid(exitReason)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "reason invalid");
        return ERR_INVALID_VALUE;
    }

    int32_t targetUserId;
    int32_t getOsAccountRet = DelayedSingleton<AppExecFwk::OsAccountManagerWrapper>::GetInstance()->
        GetOsAccountLocalIdFromUid(uid, targetUserId);
    if (getOsAccountRet != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "get GetOsAccountLocalIdFromUid failed. ret: %{public}d", getOsAccountRet);
        return ERR_INVALID_VALUE;
    }
    TAG_LOGD(AAFwkTag::ABILITYMGR, "targetUserId: %{public}d", targetUserId);
    std::vector<std::string> abilityLists;
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        GetActiveAbilityListFromUIAbilityManager(uid, abilityLists, pid);
    } else  {
        GetActiveAbilityList(uid, abilityLists, pid);
    }

    auto ret = DelayedSingleton<AppScheduler>::GetInstance()->NotifyAppMgrRecordExitReason(pid, exitReason.reason,
        exitReason.exitMsg);
    if (ret != ERR_OK) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "notify ret:%{public}d", ret);
    }

    if (abilityLists.empty()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "active abilityLists empty");
        return ERR_GET_ACTIVE_ABILITY_LIST_EMPTY;
    }
    return DelayedSingleton<AbilityRuntime::AppExitReasonDataManager>::GetInstance()->SetAppExitReason(bundleName,
        accessTokenId, abilityLists, exitReason, processInfo, fromKillWithReason);
}

int32_t AppExitReasonHelper::RecordProcessExtensionExitReason(
    const int32_t pid, const std::string &bundleName, const ExitReason &exitReason,
    const AppExecFwk::RunningProcessInfo &processInfo, bool withKillMsg)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "called");
    CHECK_POINTER_AND_RETURN(subManagersHelper_, ERR_NULL_OBJECT);
    auto connectManager = subManagersHelper_->GetCurrentUIExtensionAbilityManager();
    CHECK_POINTER_AND_RETURN(connectManager, ERR_NULL_OBJECT);
    std::vector<std::string> extensionList;
    int32_t resultCode = ERR_OK;
    if (pid <= NO_PID) {
        resultCode = connectManager->GetActiveUIExtensionList(bundleName, extensionList);
    } else {
        resultCode = connectManager->GetActiveUIExtensionList(pid, extensionList);
    }
    if (resultCode != ERR_OK) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "ResultCode: %{public}d", resultCode);
        return ERR_GET_ACTIVE_EXTENSION_LIST_EMPTY;
    }

    if (extensionList.empty()) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "ExtensionList is empty.");
        return ERR_GET_ACTIVE_EXTENSION_LIST_EMPTY;
    }

    auto appExitReasonDataMgr = DelayedSingleton<AbilityRuntime::AppExitReasonDataManager>::GetInstance();
    if (appExitReasonDataMgr == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null appExitReasonDataMgr");
        return ERR_INVALID_VALUE;
    }

    return appExitReasonDataMgr->SetUIExtensionAbilityExitReason(bundleName, extensionList, exitReason,
        processInfo, withKillMsg);
}

int32_t AppExitReasonHelper::AddAppExitReason(const std::string &bundleName, int32_t pid, int32_t uid, int32_t appIndex,
    const ExitReasonCompability &exitReason)
{
    int32_t userId = -1;
    int32_t getOsAccountRet = DelayedSingleton<AppExecFwk::OsAccountManagerWrapper>::GetInstance()->
        GetOsAccountLocalIdFromUid(uid, userId);
    if (getOsAccountRet != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "get GetOsAccountLocalIdFromUid failed. ret: %{public}d", getOsAccountRet);
        return getOsAccountRet;
    }
    TAG_LOGD(AAFwkTag::ABILITYMGR,
        "userId: %{public}d, bundleName: %{public}s, appIndex: %{public}d", userId, bundleName.c_str(), appIndex);
    uint32_t accessTokenId = Security::AccessToken::AccessTokenKit::GetHapTokenID(userId, bundleName, appIndex);
    AppExecFwk::RunningProcessInfo processInfo;
    GetRunningProcessInfo(pid, userId, bundleName, processInfo);
    RecordExitReasonParams params;
    params.pid = pid;
    params.uid = uid;
    params.bundleName = bundleName;
    params.accessTokenId = accessTokenId;
    params.exitReason = exitReason;
    params.processInfo = processInfo;
    return AddProcessExitReason(params);
}

int32_t AppExitReasonHelper::AddBundleExitReason(
    const std::string &bundleName, int32_t userId, int32_t appIndex, const ExitReasonCompability &exitReason)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR,
        "userId: %{public}d, bundleName: %{public}s, appIndex: %{public}d", userId, bundleName.c_str(), appIndex);
    uint32_t accessTokenId = Security::AccessToken::AccessTokenKit::GetHapTokenID(userId, bundleName, appIndex);

    auto bms = AbilityUtil::GetBundleManagerHelper();
    CHECK_POINTER_AND_RETURN(bms, ERR_INVALID_VALUE);
    AppExecFwk::BundleInfo bundleInfo;
    auto ret = IN_PROCESS_CALL(bms->GetCloneBundleInfo(
        bundleName, static_cast<int32_t>(AppExecFwk::GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_APPLICATION),
        appIndex, bundleInfo, userId));
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "failed get bundle info for %{public}s", bundleName.c_str());
        return ret;
    }
    AppExecFwk::RunningProcessInfo processInfo;
    GetRunningProcessInfo(NO_PID, userId, bundleName, processInfo);
    RecordExitReasonParams params;
    params.pid = processInfo.pid_;
    params.uid = bundleInfo.uid;
    params.bundleName = bundleName;
    params.accessTokenId = accessTokenId;
    params.exitReason = exitReason;
    params.processInfo = processInfo;
    return AddProcessExitReason(params);
}

void AppExitReasonHelper::GetActiveAbilityList(int32_t uid, std::vector<std::string> &abilityLists,
    const int32_t pid)
{
    int32_t targetUserId;
    int32_t getOsAccountRet = DelayedSingleton<AppExecFwk::OsAccountManagerWrapper>::GetInstance()->
        GetOsAccountLocalIdFromUid(uid, targetUserId);
    if (getOsAccountRet != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "get GetOsAccountLocalIdFromUid failed. ret: %{public}d", getOsAccountRet);
        return;
    }
    TAG_LOGD(AAFwkTag::ABILITYMGR, "targetUserId: %{public}d", targetUserId);
    CHECK_POINTER(subManagersHelper_);
    if (targetUserId == U0_USER_ID) {
        auto missionListManagers = subManagersHelper_->GetMissionListManagers();
        for (auto& item: missionListManagers) {
            CHECK_POINTER_CONTINUE(item.second);
            std::vector<std::string> abilityList;
            item.second->GetActiveAbilityList(uid, abilityList, pid);
            if (!abilityList.empty()) {
                abilityLists.insert(abilityLists.end(), abilityList.begin(), abilityList.end());
            }
        }
        return;
    }

    auto listManager = subManagersHelper_->GetMissionListManagerByUserId(targetUserId);
    CHECK_POINTER(listManager);
    listManager->GetActiveAbilityList(uid, abilityLists, pid);
}

void AppExitReasonHelper::GetActiveAbilityListFromUIAbilityManager(int32_t uid, std::vector<std::string> &abilityLists,
    const int32_t pid)
{
    CHECK_POINTER(subManagersHelper_);
    int32_t targetUserId;
    int32_t getOsAccountRet = DelayedSingleton<AppExecFwk::OsAccountManagerWrapper>::GetInstance()->
        GetOsAccountLocalIdFromUid(uid, targetUserId);
    if (getOsAccountRet != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "get GetOsAccountLocalIdFromUid failed. ret: %{public}d", getOsAccountRet);
        return;
    }
    TAG_LOGD(AAFwkTag::ABILITYMGR, "targetUserId: %{public}d", targetUserId);
    if (targetUserId == U0_USER_ID) {
        auto uiAbilityManagers = subManagersHelper_->GetUIAbilityManagers();
        for (auto& item: uiAbilityManagers) {
            CHECK_POINTER_CONTINUE(item.second);
            std::vector<std::string> abilityList;
            item.second->GetActiveAbilityList(uid, abilityList, pid);
            if (!abilityList.empty()) {
                abilityLists.insert(abilityLists.end(), abilityList.begin(), abilityList.end());
            }
        }
        return;
    }

    auto uiAbilityManager = subManagersHelper_->GetUIAbilityManagerByUserId(targetUserId);
    CHECK_POINTER(uiAbilityManager);
    uiAbilityManager->GetActiveAbilityList(uid, abilityLists, pid);
}

bool AppExitReasonHelper::IsExitReasonValid(const ExitReason &exitReason)
{
    const Reason reason = exitReason.reason;
    return reason >= REASON_MIN && reason <= REASON_MAX;
}

int32_t AppExitReasonHelper::GetActiveAbilityListWithPid(int32_t uid, std::vector<std::string> &abilityList,
    int32_t pid)
{
    CHECK_POINTER_AND_RETURN(subManagersHelper_, ERR_NULL_OBJECT);
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        auto uiAbilityManager = subManagersHelper_->GetUIAbilityManagerByUid(uid);
        CHECK_POINTER_AND_RETURN(uiAbilityManager, ERR_NULL_OBJECT);
        uiAbilityManager->GetActiveAbilityList(uid, abilityList, pid);
    } else {
        auto missionListManager = subManagersHelper_->GetMissionListManagerByUid(uid);
        CHECK_POINTER_AND_RETURN(missionListManager, ERR_NULL_OBJECT);
        missionListManager->GetActiveAbilityList(uid, abilityList, pid);
    }
    return ERR_OK;
}

int32_t AppExitReasonHelper::RecordUIAbilityExitReason(const pid_t pid, const std::string &abilityName,
    const ExitReason &exitReason)
{
    AppExecFwk::ApplicationInfo application;
    bool debug = false;
    auto ret = IN_PROCESS_CALL(DelayedSingleton<AppScheduler>::GetInstance()->GetApplicationInfoByProcessID(pid,
        application, debug));
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "getApplicationInfoByProcessID failed");
        return ret;
    }
    auto bundleName = application.bundleName;
    AppExecFwk::RunningProcessInfo processInfo;
    if (pid > 0) {
        DelayedSingleton<AppScheduler>::GetInstance()->GetRunningProcessInfoByPid(pid, processInfo);
    }
    std::vector<std::string> abilityLists = {};
    DelayedSingleton<AbilityRuntime::AppExitReasonDataManager>::GetInstance()->
        GetRecordAppAbilityNames(application.accessTokenId, abilityLists);
    bool isAbilityListsEmpty = abilityLists.empty();
    abilityLists.emplace_back(abilityName);
    if (isAbilityListsEmpty) {
        return DelayedSingleton<AbilityRuntime::AppExitReasonDataManager>::GetInstance()->SetAppExitReason(bundleName,
            application.accessTokenId, abilityLists, exitReason, processInfo, false);
    } else {
        DelayedSingleton<AbilityRuntime::AppExitReasonDataManager>::GetInstance()->
            UpdateAppExitReason(application.accessTokenId, abilityLists, exitReason, processInfo, false);
    }
    return ERR_OK;
}

void AppExitReasonHelper::GetRunningProcessInfo(int32_t pid, int32_t userId, const std::string &bundleName,
    AppExecFwk::RunningProcessInfo &processInfo)
{
    if (pid != NO_PID) {
        DelayedSingleton<AppScheduler>::GetInstance()->GetRunningProcessInfoByPid(static_cast<pid_t>(pid),
            processInfo);
        return;
    }
    if (userId == -1 || bundleName.empty()) {
        return;
    }
    auto appMgr = AppMgrUtil::GetAppMgr();
    if (appMgr == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "appMgr null");
        return;
    }
    std::vector<AppExecFwk::RunningProcessInfo> infoList;
    IN_PROCESS_CALL(appMgr->GetRunningProcessInformation(bundleName, userId, infoList));
    if (infoList.size() == 1) {
        processInfo = infoList.front();
    }
}

int32_t AppExitReasonHelper::AddProcessExitReason(const RecordExitReasonParams &params)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::vector<std::string> abilityLists;
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        GetActiveAbilityListFromUIAbilityManager(params.uid, abilityLists, params.pid);
    } else {
        GetActiveAbilityList(params.uid, abilityLists, params.pid);
    }

    if (abilityLists.empty()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "active abilityLists empty");
        return ERR_GET_ACTIVE_ABILITY_LIST_EMPTY;
    }
    ExitReasonCompability exitReason = params.exitReason;
    auto ret = DelayedSingleton<AppScheduler>::GetInstance()->NotifyAppMgrRecordExitReasonCompability(
        params.pid, exitReason.killId, exitReason.killMsg, exitReason.innerMsg);
    if (ret != ERR_OK) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "notify failed:%{public}d", ret);
    }
    ExitReason tmpReason(exitReason.reason, exitReason.subReason, exitReason.exitMsg);
    return DelayedSingleton<AbilityRuntime::AppExitReasonDataManager>::GetInstance()->SetAppExitReason(
        params.bundleName, params.accessTokenId, abilityLists, tmpReason, params.processInfo,
        params.fromKillWithReason);
}

void AppExitReasonHelper::RecordInvalidKillId(int32_t pid, const ExitReasonCompability &params,
    const std::string &bundleName, int32_t userId)
{
    if (pid == NO_PID) {
        AppExecFwk::RunningProcessInfo processInfo;
        GetRunningProcessInfo(NO_PID, userId, bundleName, processInfo);
        pid = processInfo.pid_;
    }
    DelayedSingleton<AppScheduler>::GetInstance()->NotifyAppMgrRecordExitReasonCompability(
        pid, INVALID_KILLID, params.killMsg, params.innerMsg);
}
}  // namespace AppExecFwk
}  // namespace OHOS
