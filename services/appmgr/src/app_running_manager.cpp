/*
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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

#include "app_running_manager.h"

#include "app_mgr_service_inner.h"
#include "datetime_ex.h"
#include "iremote_object.h"

#include "appexecfwk_errors.h"
#include "app_utils.h"
#include "common_event_support.h"
#include "exit_resident_process_manager.h"
#include "freeze_util.h"
#include "global_constant.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "killing_process_manager.h"
#include "os_account_manager_wrapper.h"
#include "perf_profile.h"
#include "parameters.h"
#include "quick_fix_callback_with_record.h"
#include <cstddef>
#ifdef SUPPORT_SCREEN
#include "scene_board_judgement.h"
#include "window_visibility_info.h"
#endif //SUPPORT_SCREEN
#include "app_mgr_service_const.h"
#include "app_mgr_service_dump_error_code.h"
#include "cache_process_manager.h"
#include "res_sched_util.h"
#include "task_handler_wrap.h"
#include "time_util.h"
#include "ui_extension_wrapper.h"
#include "app_native_spawn_manager.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
constexpr int32_t QUICKFIX_UID = 5524;
constexpr int32_t DEAD_APP_RECORD_CLEAR_TIME = 3000; // ms
constexpr int32_t DEAD_CHILD_RELATION_CLEAR_TIME = 3000; // ms
constexpr const char* DEVELOPER_MODE_STATE = "const.security.developermode.state";
}
using EventFwk::CommonEventSupport;

AppRunningManager::AppRunningManager()
{}
AppRunningManager::~AppRunningManager()
{}

std::shared_ptr<AppRunningRecord> AppRunningManager::CreateAppRunningRecord(
    const std::shared_ptr<ApplicationInfo> &appInfo, const std::string &processName, const BundleInfo &bundleInfo,
    const std::string &instanceKey, const std::string &customProcessFlag)
{
    if (!appInfo) {
        TAG_LOGE(AAFwkTag::APPMGR, "param error");
        return nullptr;
    }

    if (processName.empty()) {
        TAG_LOGE(AAFwkTag::APPMGR, "processName error");
        return nullptr;
    }

    auto recordId = AppRecordId::Create();
    auto appRecord = std::make_shared<AppRunningRecord>(appInfo, recordId, processName);

    std::regex rule("[a-zA-Z.]+[-_#]{1}");
    std::string signCode;
    bool isStageBasedModel = false;
    ClipStringContent(rule, bundleInfo.appId, signCode);
    if (!bundleInfo.hapModuleInfos.empty()) {
        isStageBasedModel = bundleInfo.hapModuleInfos.back().isStageBasedModel;
    }
    TAG_LOGD(AAFwkTag::APPMGR,
        "Create AppRunningRecord, processName: %{public}s, StageBasedModel:%{public}d, recordId: %{public}d",
        processName.c_str(), isStageBasedModel, recordId);

    appRecord->SetStageModelState(isStageBasedModel);
    appRecord->SetSingleton(bundleInfo.singleton);
    appRecord->SetKeepAliveBundle(bundleInfo.isKeepAlive);
    appRecord->SetSignCode(signCode);
    appRecord->SetJointUserId(bundleInfo.jointUserId);
    appRecord->SetAppIdentifier(bundleInfo.signatureInfo.appIdentifier);
    appRecord->SetInstanceKey(instanceKey);
    appRecord->SetCustomProcessFlag(customProcessFlag);
    {
        std::lock_guard guard(runningRecordMapMutex_);
        appRunningRecordMap_.emplace(recordId, appRecord);
    }
    {
        std::lock_guard guard(updateConfigurationDelayedLock_);
        updateConfigurationDelayedMap_.emplace(recordId, false);
    }
    return appRecord;
}

bool AppRunningManager::CheckAppProcessNameIsSame(const std::shared_ptr<AppRunningRecord> &appRecord,
    const std::string &processName, bool isPreload)
{
    if (appRecord == nullptr) {
        return false;
    }
    return (appRecord->GetProcessName() == processName) && (isPreload
        || appRecord->GetPreloadMode() == PreloadMode::PRELOAD_MODULE || !(appRecord->GetExtensionSandBoxFlag()));
}

std::shared_ptr<AppRunningRecord> AppRunningManager::CheckAppRunningRecordIsExist(const std::string &appName,
    const std::string &processName, const int uid, const BundleInfo &bundleInfo,
    const std::string &specifiedProcessFlag, bool *isProCache, const std::string &instanceKey,
    const std::string &customProcessFlag, const bool notReuseCachedPorcess, bool isPreload)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::APPMGR,
        "appName: %{public}s, processName: %{public}s, uid: %{public}d, specifiedProcessFlag: %{public}s, \
         customProcessFlag: %{public}s",
        appName.c_str(), processName.c_str(), uid, specifiedProcessFlag.c_str(), customProcessFlag.c_str());
    std::regex rule("[a-zA-Z.]+[-_#]{1}");
    std::string signCode;
    auto jointUserId = bundleInfo.jointUserId;
    TAG_LOGD(AAFwkTag::APPMGR, "jointUserId : %{public}s", jointUserId.c_str());
    ClipStringContent(rule, bundleInfo.appId, signCode);
    auto findSameProcess = [signCode, specifiedProcessFlag, processName, jointUserId, customProcessFlag, isPreload]
        (const auto &pair) {
            return (pair.second != nullptr) &&
            (specifiedProcessFlag.empty() || pair.second->GetSpecifiedProcessFlag() == specifiedProcessFlag) &&
            (pair.second->GetCustomProcessFlag() == customProcessFlag) &&
            (pair.second->GetSignCode() == signCode) &&
            AppRunningManager::CheckAppProcessNameIsSame(pair.second, processName, isPreload) &&
            (pair.second->GetJointUserId() == jointUserId) && !(pair.second->IsTerminating()) &&
            !(pair.second->IsKilling()) && !(pair.second->GetRestartAppFlag()) &&
            !(pair.second->IsKillPrecedeStart());
    };
    auto appRunningMap = GetAppRunningRecordMap();
    if (!jointUserId.empty()) {
        auto iter = std::find_if(appRunningMap.begin(), appRunningMap.end(), findSameProcess);
        return ((iter == appRunningMap.end()) ? nullptr : iter->second);
    }
    for (const auto &item : appRunningMap) {
        const auto &appRecord = item.second;
        if (appRecord && CheckAppProcessNameIsSame(appRecord, processName, isPreload) &&
            appRecord->GetInstanceKey() == instanceKey &&
            (specifiedProcessFlag.empty() || appRecord->GetSpecifiedProcessFlag() == specifiedProcessFlag) &&
            (appRecord->GetCustomProcessFlag() == customProcessFlag) &&
            !(appRecord->IsTerminating()) && !(appRecord->IsKilling()) && !(appRecord->GetRestartAppFlag()) &&
            !(appRecord->IsUserRequestCleaning()) &&
            !(appRecord->IsCaching() && appRecord->GetProcessCacheBlocked()) &&
            !(appRecord->IsKillPrecedeStart())) {
            auto appInfoList = appRecord->GetAppInfoList();
            TAG_LOGD(AAFwkTag::APPMGR,
                "appInfoList: %{public}zu, processName: %{public}s, specifiedProcessFlag: %{public}s, \
                 customProcessFlag: %{public}s",
                appInfoList.size(), appRecord->GetProcessName().c_str(), specifiedProcessFlag.c_str(),
                customProcessFlag.c_str());
            auto isExist = [&appName, &uid, &appRecord](const std::shared_ptr<ApplicationInfo> &appInfo) {
                TAG_LOGD(AAFwkTag::APPMGR, "appInfo->name: %{public}s", appInfo->name.c_str());
                return (appInfo->name == appName && appInfo->uid == uid) ||
                    (appInfo->bundleType == BundleType::APP_PLUGIN && appRecord->GetUid() == uid);
            };
            auto appInfoIter = std::find_if(appInfoList.begin(), appInfoList.end(), isExist);
            if (appInfoIter == appInfoList.end()) {
                continue;
            }
            // preload feature just check appRecord exist or not
            if (notReuseCachedPorcess) {
                return appRecord;
            }
            bool isProcCacheInner =
                DelayedSingleton<CacheProcessManager>::GetInstance()->ReuseCachedProcess(appRecord);
            if (isProCache != nullptr) {
                *isProCache = isProcCacheInner;
            }
            return appRecord;
        }
    }
    return nullptr;
}

std::shared_ptr<AppRunningRecord> AppRunningManager::CheckAppRunningRecordForSpecifiedProcess(
    int32_t uid, const std::string &instanceKey, const std::string &customProcessFlag)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::APPMGR, "uid: %{public}d: customProcessFlag: %{public}s", uid, customProcessFlag.c_str());
    auto appRunningMap = GetAppRunningRecordMap();
    for (const auto &item : appRunningMap) {
        const auto &appRecord = item.second;
        if (appRecord->GetInstanceKey() == instanceKey && appRecord->GetUid() == uid &&
            appRecord->GetProcessType() == ProcessType::NORMAL &&
            (appRecord->GetCustomProcessFlag() == customProcessFlag) &&
            !(appRecord->IsTerminating()) && !(appRecord->IsKilling()) && !(appRecord->GetRestartAppFlag()) &&
            !(appRecord->IsUserRequestCleaning()) &&
            !(appRecord->IsCaching() && appRecord->GetProcessCacheBlocked()) &&
            !(appRecord->IsKillPrecedeStart())) {
            return appRecord;
        }
    }
    return nullptr;
}

#ifdef APP_NO_RESPONSE_DIALOG
bool AppRunningManager::CheckAppRunningRecordIsExist(const std::string &bundleName, const std::string &abilityName)
{
    std::lock_guard guard(runningRecordMapMutex_);
    if (appRunningRecordMap_.empty()) {
        return false;
    }
    for (const auto &item : appRunningRecordMap_) {
        const auto &appRecord = item.second;
        if (!appRecord) {
            continue;
        }
        if (appRecord->GetBundleName() != bundleName) {
            continue;
        }
        const auto &abilityRunningRecordMap = appRecord->GetAbilities();
        for (const auto &abilityItem : abilityRunningRecordMap) {
            const auto &abilityRunning = abilityItem.second;
            if (abilityRunning && abilityRunning->GetName() == abilityName) {
                return true;
            }
        }
    }
    return false;
}
#endif

bool AppRunningManager::IsSameAbilityType(const std::shared_ptr<AppRunningRecord> &appRecord,
    const AppExecFwk::AbilityInfo &abilityInfo)
{
    if (appRecord == nullptr) {
        return false;
    }
    bool isUIAbility = (abilityInfo.type == AppExecFwk::AbilityType::PAGE);
    bool isUIExtension = (abilityInfo.extensionAbilityType == AppExecFwk::ExtensionAbilityType::SYS_COMMON_UI);
    if (((appRecord->GetProcessType() == ProcessType::NORMAL ||
        appRecord->GetExtensionType() == AppExecFwk::ExtensionAbilityType::SERVICE ||
        appRecord->GetExtensionType() == AppExecFwk::ExtensionAbilityType::DATASHARE) && isUIAbility)||
        (appRecord->GetExtensionType() == AppExecFwk::ExtensionAbilityType::SYS_COMMON_UI && isUIExtension)) {
            return true;
    }
    return false;
}

bool AppRunningManager::IsAppRunningRecordValid(const std::shared_ptr<AppRunningRecord> &appRecord)
{
    if (appRecord && !(appRecord->IsTerminating()) &&
        !(appRecord->IsKilling()) && !(appRecord->GetRestartAppFlag()) &&
        !(appRecord->IsUserRequestCleaning()) &&
        !(appRecord->IsCaching() && appRecord->GetProcessCacheBlocked()) &&
        !(appRecord->IsKillPrecedeStart())) {
        return true;
    }
    return false;
}

std::shared_ptr<AppRunningRecord> AppRunningManager::FindMasterProcessAppRunningRecord(
    const std::string &appName, const AppExecFwk::AbilityInfo &abilityInfo, const int uid)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::APPMGR, "uid: %{public}d: appName: %{public}s", uid, appName.c_str());
    auto appRunningMap = GetAppRunningRecordMap();
    int64_t maxTimeStamp = INT64_MIN;
    int64_t minAppRecordId = INT32_MAX;
    std::shared_ptr<AppRunningRecord> maxAppRecord;
    std::shared_ptr<AppRunningRecord> minAppRecord;
    std::shared_ptr<AppRunningRecord> resMasterRecord;
    bool isUIAbility = (abilityInfo.type == AppExecFwk::AbilityType::PAGE);
    bool isUIExtension = (abilityInfo.extensionAbilityType == AppExecFwk::ExtensionAbilityType::SYS_COMMON_UI);
    for (const auto &item : appRunningMap) {
        const auto &appRecord = item.second;
        if (!(appRecord && appRecord->GetUid() == uid)) {
            continue;
        }
        if (!IsAppRunningRecordValid(appRecord)) {
            continue;
        }
        if (appRecord->IsMasterProcess() && IsSameAbilityType(appRecord, abilityInfo)) {
            resMasterRecord = appRecord;
            break;
        }
        if (appRecord->GetTimeStamp() != 0 && maxTimeStamp < appRecord->GetTimeStamp() &&
            IsSameAbilityType(appRecord, abilityInfo)) {
            maxTimeStamp = appRecord->GetTimeStamp();
            maxAppRecord = appRecord;
        }
        if ((appRecord->GetExtensionType() == AppExecFwk::ExtensionAbilityType::SYS_COMMON_UI && isUIExtension) &&
            appRecord->GetRecordId() < minAppRecordId) {
            minAppRecordId = appRecord->GetRecordId();
            minAppRecord = appRecord;
        }
    }
    resMasterRecord = (resMasterRecord == nullptr && maxAppRecord != nullptr) ? maxAppRecord : resMasterRecord;
    resMasterRecord = (resMasterRecord == nullptr) ? minAppRecord : resMasterRecord;
    if (resMasterRecord) {
        resMasterRecord->SetMasterProcess(true);
        resMasterRecord->SetTimeStamp(0);
        DelayedSingleton<CacheProcessManager>::GetInstance()->ReuseCachedProcess(resMasterRecord);
    }
    return resMasterRecord;
}

bool AppRunningManager::CheckMasterProcessAppRunningRecordIsExist(
    const std::string &appName, const AppExecFwk::AbilityInfo &abilityInfo, const int uid)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::APPMGR, "uid: %{public}d: appName: %{public}s", uid, appName.c_str());
    auto appRunningMap = GetAppRunningRecordMap();
    bool isUIAbility = (abilityInfo.type == AppExecFwk::AbilityType::PAGE);
    bool isUIExtension = (abilityInfo.extensionAbilityType == AppExecFwk::ExtensionAbilityType::SYS_COMMON_UI);
    for (const auto &item : appRunningMap) {
        const auto &appRecord = item.second;
        if (appRecord && appRecord->GetUid() == uid && appRecord->IsMasterProcess() &&
            IsSameAbilityType(appRecord, abilityInfo)) {
            return true;
        }
    }
    return false;
}

bool AppRunningManager::IsAppExist(uint32_t accessTokenId)
{
    std::lock_guard guard(runningRecordMapMutex_);
    if (appRunningRecordMap_.empty()) {
        return false;
    }
    for (const auto &item : appRunningRecordMap_) {
        const auto &appRecord = item.second;
        if (appRecord == nullptr) {
            continue;
        }
        auto appInfo = appRecord->GetApplicationInfo();
        if (appInfo == nullptr) {
            continue;
        }
        if (appInfo->accessTokenId == accessTokenId && !(appRecord->GetRestartAppFlag())) {
            return true;
        }
    }
    return false;
}

bool AppRunningManager::CheckAppRunningRecordIsExistByUid(int32_t uid)
{
    std::lock_guard guard(runningRecordMapMutex_);
    if (appRunningRecordMap_.empty()) {
        return false;
    }
    for (const auto &item : appRunningRecordMap_) {
        const auto &appRecord = item.second;
        if (appRecord && appRecord->GetUid() == uid && !(appRecord->GetRestartAppFlag())) {
            return true;
        }
    }
    return false;
}

int32_t AppRunningManager::CheckAppCloneRunningRecordIsExistByBundleName(const std::string &bundleName,
    int32_t appCloneIndex, bool &isRunning)
{
    std::lock_guard guard(runningRecordMapMutex_);
    for (const auto &item : appRunningRecordMap_) {
        const auto &appRecord = item.second;
        if (appRecord && appRecord->GetBundleName() == bundleName && !(appRecord->GetRestartAppFlag()) &&
            appRecord->GetAppIndex() == appCloneIndex) {
            isRunning = true;
            break;
        }
    }
    return ERR_OK;
}

int32_t AppRunningManager::IsAppRunningByBundleName(const std::string &bundleName, int32_t appCloneIndex,
    int32_t userId, bool &isRunning)
{
    isRunning = false;
    std::lock_guard guard(runningRecordMapMutex_);
    for (const auto &item : appRunningRecordMap_) {
        const auto &appRecord = item.second;
        if (appRecord && appRecord->GetBundleName() == bundleName && !(appRecord->GetRestartAppFlag()) &&
            (appCloneIndex < 0 || appRecord->GetAppIndex() == appCloneIndex) &&
            (userId < 0 || appRecord->GetUserId() == userId)) {
            isRunning = true;
            break;
        }
    }
    return ERR_OK;
}

int32_t AppRunningManager::IsAppRunningByBundleNameAndUserId(const std::string &bundleName,
    int32_t userId, bool &isRunning)
{
    auto appRunningMap = GetAppRunningRecordMap();
    for (const auto &item : appRunningMap) {
        const auto &appRecord = item.second;
        if (appRecord && appRecord->GetBundleName() == bundleName && !(appRecord->GetRestartAppFlag()) &&
            appRecord->GetUid() / BASE_USER_RANGE == userId) {
            isRunning = true;
            break;
        }
    }
    return ERR_OK;
}

int32_t AppRunningManager::GetAllAppRunningRecordCountByBundleName(const std::string &bundleName)
{
    int32_t count = 0;
    std::lock_guard guard(runningRecordMapMutex_);
    for (const auto &item : appRunningRecordMap_) {
        const auto &appRecord = item.second;
        if (appRecord && appRecord->GetBundleName() == bundleName) {
            count++;
        }
    }

    return count;
}

std::shared_ptr<AppRunningRecord> AppRunningManager::GetAppRunningRecordByPid(const pid_t pid)
{
    std::lock_guard guard(runningRecordMapMutex_);
    auto iter = std::find_if(appRunningRecordMap_.begin(), appRunningRecordMap_.end(), [&pid](const auto &pair) {
        return pair.second->GetPid() == pid;
    });
    return ((iter == appRunningRecordMap_.end()) ? nullptr : iter->second);
}

std::shared_ptr<AppRunningRecord> AppRunningManager::GetAppRunningRecordByAbilityToken(
    const sptr<IRemoteObject> &abilityToken)
{
    std::lock_guard guard(runningRecordMapMutex_);
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    for (const auto &item : appRunningRecordMap_) {
        const auto &appRecord = item.second;
        if (appRecord && appRecord->GetAbilityRunningRecordByToken(abilityToken)) {
            return appRecord;
        }
    }
    return nullptr;
}

bool AppRunningManager::ProcessExitByBundleName(
    const std::string &bundleName, std::list<pid_t> &pids, const bool clearPageStack)
{
    auto appRunningMap = GetAppRunningRecordMap();
    for (const auto &item : appRunningMap) {
        const auto &appRecord = item.second;
        // condition [!appRecord->IsKeepAliveApp()] Is to not kill the resident process.
        // Before using this method, consider whether you need.
        if (appRecord && (!appRecord->IsKeepAliveApp() ||
            !ExitResidentProcessManager::GetInstance().IsMemorySizeSufficient())) {
            pid_t pid = appRecord->GetPid();
            auto appInfoList = appRecord->GetAppInfoList();
            auto isExist = [&bundleName](const std::shared_ptr<ApplicationInfo> &appInfo) {
                return appInfo->bundleName == bundleName;
            };
            auto iter = std::find_if(appInfoList.begin(), appInfoList.end(), isExist);
            if (iter == appInfoList.end() || pid <= 0) {
                continue;
            }
            pids.push_back(pid);
            if (clearPageStack) {
                appRecord->ScheduleClearPageStack();
            }
            appRecord->ScheduleProcessSecurityExit();
        }
    }

    return !pids.empty();
}

bool AppRunningManager::GetPidsByUserId(int32_t userId, std::list<pid_t> &pids)
{
    auto appRunningMap = GetAppRunningRecordMap();
    for (const auto &item : appRunningMap) {
        const auto &appRecord = item.second;
        if (appRecord) {
            int32_t id = -1;
            if ((DelayedSingleton<OsAccountManagerWrapper>::GetInstance()->
                GetOsAccountLocalIdFromUid(appRecord->GetUid(), id) == 0) && (id == userId)) {
                TAG_LOGD(AAFwkTag::APPMGR, "GetOsAccountLocalIdFromUid id: %{public}d", id);
                pid_t pid = appRecord->GetPid();
                if (pid > 0) {
                    pids.push_back(pid);
                    appRecord->ScheduleProcessSecurityExit();
                }
            }
        }
    }

    return (!pids.empty());
}

bool AppRunningManager::GetProcessInfosByUserId(int32_t userId, std::list<SimpleProcessInfo> &processInfos)
{
    auto appRunningMap = GetAppRunningRecordMap();
    for (const auto &item : appRunningMap) {
        const auto &appRecord = item.second;
        if (appRecord == nullptr) {
            continue;
        }
        int32_t id = -1;
        if ((DelayedSingleton<OsAccountManagerWrapper>::GetInstance()->
            GetOsAccountLocalIdFromUid(appRecord->GetUid(), id) == 0) && (id == userId)) {
            TAG_LOGD(AAFwkTag::APPMGR, "GetOsAccountLocalIdFromUid id: %{public}d", id);
            pid_t pid = appRecord->GetPid();
            if (pid > 0) {
                auto processInfo = SimpleProcessInfo(pid, appRecord->GetProcessName());
                processInfos.emplace_back(processInfo);
                appRecord->GetRenderProcessInfos(processInfos);
                #ifdef SUPPORT_CHILD_PROCESS
                appRecord->GetChildProcessInfos(processInfos);
                #endif // SUPPORT_CHILD_PROCESS
                appRecord->ScheduleProcessSecurityExit();
            }
        }
    }

    return (!processInfos.empty());
}

int32_t AppRunningManager::ProcessUpdateApplicationInfoInstalled(
    const ApplicationInfo& appInfo, const std::string& moduleName)
{
    auto appRunningMap = GetAppRunningRecordMap();
    int32_t result = ERR_OK;
    for (const auto &item : appRunningMap) {
        const auto &appRecord = item.second;
        if (!appRecord) {
            continue;
        }
        auto appInfoList = appRecord->GetAppInfoList();
        if (appInfo.bundleType == BundleType::APP_PLUGIN) {
            if (appRecord->GetUid() == appInfo.uid) {
                TAG_LOGI(AAFwkTag::APPMGR, "UpdateApplicationInfoInstalled: %{public}s", moduleName.c_str());
                appRecord->UpdateApplicationInfoInstalled(appInfo, moduleName);
                continue;
            }
        }
        for (auto iter : appInfoList) {
            if (iter->bundleName == appInfo.bundleName && iter->uid == appInfo.uid) {
                appRecord->UpdateApplicationInfoInstalled(appInfo, moduleName);
                break;
            }
        }
    }
    return result;
}

bool AppRunningManager::ProcessExitByBundleNameAndUid(
    const std::string &bundleName, const int uid, std::list<pid_t> &pids, const KillProcessConfig &config)
{
    auto appRunningMap = GetAppRunningRecordMap();
    for (const auto &item : appRunningMap) {
        const auto &appRecord = item.second;
        if (appRecord == nullptr) {
            continue;
        }
        auto appInfoList = appRecord->GetAppInfoList();
        auto isExist = [&bundleName, &uid](const std::shared_ptr<ApplicationInfo> &appInfo) {
            return appInfo->bundleName == bundleName && appInfo->uid == uid;
        };
        auto iter = std::find_if(appInfoList.begin(), appInfoList.end(), isExist);
        pid_t pid = appRecord->GetPid();
        if (iter == appInfoList.end() || pid <= 0) {
            continue;
        }
        pids.push_back(pid);
        if (config.clearPageStack) {
            appRecord->ScheduleClearPageStack();
        }
        if (config.addKillingCaller) {
            std::string callerKey = std::to_string(pid) + ":" + std::to_string(appRecord->GetUid());
            KillingProcessManager::GetInstance().AddKillingCallerKey(callerKey);
        }
        appRecord->SetKilling();
        appRecord->SetKillReason(config.reason);
        appRecord->ScheduleProcessSecurityExit();
    }

    return (pids.empty() ? false : true);
}

bool AppRunningManager::ProcessExitByPid(int32_t pid, const KillProcessConfig &config)
{
    auto appRunningMap = GetAppRunningRecordMap();
    for (const auto &[id, appRecord] : appRunningMap) {
        if (appRecord == nullptr || appRecord->GetPid() != pid) {
            continue;
        }

        if (config.clearPageStack) {
            appRecord->ScheduleClearPageStack();
        }
        if (config.addKillingCaller) {
            std::string callerKey = std::to_string(pid) + ":" + std::to_string(appRecord->GetUid());
            KillingProcessManager::GetInstance().AddKillingCallerKey(callerKey);
        }
        appRecord->SetKilling();
        appRecord->SetKillReason(config.reason);
        appRecord->ScheduleProcessSecurityExit();
        return true;
    }

    return false;
}

bool AppRunningManager::ProcessExitByBundleNameAndAppIndex(const std::string &bundleName, int32_t appIndex,
    std::list<pid_t> &pids, bool clearPageStack)
{
    auto appRunningMap = GetAppRunningRecordMap();
    for (const auto &item : appRunningMap) {
        const auto &appRecord = item.second;
        if (appRecord == nullptr) {
            continue;
        }
        if (appRecord->IsKeepAliveApp() && ExitResidentProcessManager::GetInstance().IsMemorySizeSufficient()) {
            continue;
        }
        auto appInfo = appRecord->GetApplicationInfo();
        if (appInfo == nullptr) {
            continue;
        }
        if (appRecord->GetPriorityObject() == nullptr) {
            continue;
        }

        if (appInfo->bundleName == bundleName && appRecord->GetAppIndex() == appIndex) {
            pid_t pid = appRecord->GetPid();
            if (pid <= 0) {
                continue;
            }
            pids.push_back(pid);
            if (clearPageStack) {
                appRecord->ScheduleClearPageStack();
            }
            appRecord->SetKilling();
            appRecord->ScheduleProcessSecurityExit();
        }
    }
    return !pids.empty();
}

bool AppRunningManager::ProcessExitByTokenIdAndInstance(uint32_t accessTokenId, const std::string &instanceKey,
    std::list<pid_t> &pids, bool clearPageStack)
{
    auto appRunningMap = GetAppRunningRecordMap();
    for (const auto &item : appRunningMap) {
        const auto &appRecord = item.second;
        if (appRecord == nullptr) {
            continue;
        }
        auto appInfo = appRecord->GetApplicationInfo();
        if (appInfo == nullptr) {
            continue;
        }
        if (appInfo->accessTokenId != accessTokenId) {
            continue;
        }
        if (appInfo->multiAppMode.multiAppModeType != MultiAppModeType::MULTI_INSTANCE) {
            TAG_LOGI(AAFwkTag::APPMGR, "not multi-instance");
            continue;
        }
        if (appRecord->GetInstanceKey() != instanceKey) {
            continue;
        }
        if (appRecord->GetPriorityObject() == nullptr) {
            continue;
        }
        pid_t pid = appRecord->GetPid();
        if (pid <= 0) {
            continue;
        }
        pids.push_back(pid);
        if (clearPageStack) {
            appRecord->ScheduleClearPageStack();
        }
        appRecord->SetKilling();
        appRecord->ScheduleProcessSecurityExit();
    }

    return !pids.empty();
}

bool AppRunningManager::GetPidsByBundleNameUserIdAndAppIndex(const std::string &bundleName,
    const int userId, const int appIndex, std::list<pid_t> &pids)
{
    auto appRunningMap = GetAppRunningRecordMap();
    for (const auto &item : appRunningMap) {
        const auto &appRecord = item.second;
        if (appRecord == nullptr) {
            continue;
        }
        auto appInfoList = appRecord->GetAppInfoList();
        auto isExist = [&bundleName, &userId, &appIndex](const std::shared_ptr<ApplicationInfo> &appInfo) {
            return appInfo->bundleName == bundleName && appInfo->uid / BASE_USER_RANGE == userId &&
                appInfo->appIndex == appIndex;
        };
        auto iter = std::find_if(appInfoList.begin(), appInfoList.end(), isExist);
        pid_t pid = appRecord->GetPid();
        if (iter == appInfoList.end() || pid <= 0) {
            continue;
        }
        pids.push_back(pid);
        appRecord->SetKilling();
    }

    return (!pids.empty());
}

std::shared_ptr<AppRunningRecord> AppRunningManager::OnRemoteDied(const wptr<IRemoteObject> &remote,
    std::shared_ptr<AppMgrServiceInner> appMgrServiceInner)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    sptr<IRemoteObject> object = remote.promote();
    if (!object) {
        TAG_LOGE(AAFwkTag::APPMGR, "null object");
        return nullptr;
    }

    std::shared_ptr<AppRunningRecord> appRecord = nullptr;
    {
        std::lock_guard guard(runningRecordMapMutex_);
        const auto &iter =
            std::find_if(appRunningRecordMap_.begin(), appRunningRecordMap_.end(), [&object](const auto &pair) {
                if (pair.second && pair.second->GetApplicationClient() != nullptr) {
                    return pair.second->GetApplicationClient()->AsObject() == object;
                }
                return false;
            });
        if (iter == appRunningRecordMap_.end()) {
            TAG_LOGE(AAFwkTag::APPMGR, "remote not in map");
            return nullptr;
        }
        appRecord = iter->second;
        appRunningRecordMap_.erase(iter);
    }
    if (appRecord != nullptr) {
        {
            std::lock_guard guard(updateConfigurationDelayedLock_);
            updateConfigurationDelayedMap_.erase(appRecord->GetRecordId());
        }
        appRecord->RemoveAppDeathRecipient();
        appRecord->SetApplicationClient(nullptr);
        auto priorityObject = appRecord->GetPriorityObject();
        if (priorityObject != nullptr) {
            TAG_LOGI(AAFwkTag::APPMGR, "pname: %{public}s, pid: %{public}d", appRecord->GetProcessName().c_str(),
                priorityObject->GetPid());
            if (appMgrServiceInner != nullptr) {
                appMgrServiceInner->KillProcessByPid(priorityObject->GetPid(), "OnRemoteDied");
            }
            AbilityRuntime::FreezeUtil::GetInstance().DeleteAppLifecycleEvent(priorityObject->GetPid());
        }
    }
    if (appRecord != nullptr && appRecord->GetPriorityObject() != nullptr) {
        RemoveUIExtensionLauncherItem(appRecord->GetPid());
    }

    return appRecord;
}

std::map<const int32_t, const std::shared_ptr<AppRunningRecord>> AppRunningManager::GetAppRunningRecordMap()
{
    std::lock_guard guard(runningRecordMapMutex_);
    return appRunningRecordMap_;
}

void AppRunningManager::RemoveAppRunningRecordById(const int32_t recordId)
{
    std::shared_ptr<AppRunningRecord> appRecord = nullptr;
    {
        std::lock_guard guard(runningRecordMapMutex_);
        auto it = appRunningRecordMap_.find(recordId);
        if (it != appRunningRecordMap_.end()) {
            appRecord = it->second;
            appRunningRecordMap_.erase(it);
        }
    }
    {
        std::lock_guard guard(updateConfigurationDelayedLock_);
        updateConfigurationDelayedMap_.erase(recordId);
    }

    if (appRecord != nullptr && appRecord->GetPriorityObject() != nullptr) {
        RemoveUIExtensionLauncherItem(appRecord->GetPid());
        AbilityRuntime::FreezeUtil::GetInstance().DeleteAppLifecycleEvent(appRecord->GetPid());
    }

    // unregister child process exit notify when parent exit
    if (appRecord != nullptr) {
        AppNativeSpawnManager::GetInstance().RemoveNativeChildCallbackByPid(appRecord->GetPid());
    }
}

void AppRunningManager::ClearAppRunningRecordMap()
{
    std::lock_guard guard(runningRecordMapMutex_);
    appRunningRecordMap_.clear();
}

void AppRunningManager::HandleTerminateTimeOut(int64_t eventId)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    auto abilityRecord = GetAbilityRunningRecord(eventId);
    if (!abilityRecord) {
        TAG_LOGW(AAFwkTag::APPMGR, "null abilityRecord");
        return;
    }
    auto abilityToken = abilityRecord->GetToken();
    auto appRecord = GetTerminatingAppRunningRecord(abilityToken);
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "null appRecord");
        return;
    }
    appRecord->AbilityTerminated(abilityToken);
}

std::shared_ptr<AppRunningRecord> AppRunningManager::GetTerminatingAppRunningRecord(
    const sptr<IRemoteObject> &abilityToken)
{
    std::lock_guard guard(runningRecordMapMutex_);
    for (const auto &item : appRunningRecordMap_) {
        const auto &appRecord = item.second;
        if (appRecord && appRecord->GetAbilityByTerminateLists(abilityToken)) {
            return appRecord;
        }
    }
    return nullptr;
}

std::shared_ptr<AbilityRunningRecord> AppRunningManager::GetAbilityRunningRecord(const int64_t eventId)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    std::lock_guard guard(runningRecordMapMutex_);
    for (auto &item : appRunningRecordMap_) {
        if (item.second) {
            auto abilityRecord = item.second->GetAbilityRunningRecord(eventId);
            if (abilityRecord) {
                return abilityRecord;
            }
        }
    }
    return nullptr;
}

void AppRunningManager::HandleAbilityAttachTimeOut(const sptr<IRemoteObject> &token,
    std::shared_ptr<AppMgrServiceInner> serviceInner)
{
    TAG_LOGI(AAFwkTag::APPMGR, "call");
    if (token == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null token");
        return;
    }

    auto appRecord = GetAppRunningRecordByAbilityToken(token);
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "null appRecord");
        return;
    }

    std::shared_ptr<AbilityRunningRecord> abilityRecord = appRecord->GetAbilityRunningRecordByToken(token);
    bool isPage = false;
    if (abilityRecord) {
        abilityRecord->SetTerminating();
        if (abilityRecord->GetAbilityInfo() != nullptr) {
            isPage = (abilityRecord->GetAbilityInfo()->type == AbilityType::PAGE);
        }
        appRecord->StateChangedNotifyObserver(abilityRecord, static_cast<int32_t>(
            AbilityState::ABILITY_STATE_TERMINATED), true, false);
        //UIExtension notifies Extension & Ability state changes
        if (AAFwk::UIExtensionWrapper::IsUIExtension(appRecord->GetExtensionType())) {
            appRecord->StateChangedNotifyObserver(abilityRecord,
                static_cast<int32_t>(ExtensionState::EXTENSION_STATE_TERMINATED), false, false);
        }
    }

    if ((isPage || appRecord->IsLastAbilityRecord(token)) && (!appRecord->IsKeepAliveApp() ||
        !ExitResidentProcessManager::GetInstance().IsMemorySizeSufficient())) {
        appRecord->SetTerminating();
    }

    std::weak_ptr<AppRunningRecord> appRecordWptr(appRecord);
    auto timeoutTask = [appRecordWptr, token]() {
        auto appRecord = appRecordWptr.lock();
        if (appRecord == nullptr) {
            TAG_LOGW(AAFwkTag::APPMGR, "null appRecord");
            return;
        }
        appRecord->TerminateAbility(token, true, true);
    };
    appRecord->PostTask("DELAY_KILL_ABILITY", AMSEventHandler::KILL_PROCESS_TIMEOUT, timeoutTask);
}

void AppRunningManager::PrepareTerminate(const sptr<IRemoteObject> &token, bool clearMissionFlag)
{
    if (token == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null token");
        return;
    }

    auto appRecord = GetAppRunningRecordByAbilityToken(token);
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "null appRecord");
        return;
    }

    auto abilityRecord = appRecord->GetAbilityRunningRecordByToken(token);
    if (abilityRecord) {
        abilityRecord->SetTerminating();
    }

    // set app record terminating when close last page ability
    auto isLastAbility =
        clearMissionFlag ? appRecord->IsLastPageAbilityRecord(token) : appRecord->IsLastAbilityRecord(token);
    if (isLastAbility && (!appRecord->IsKeepAliveApp())) {
        auto cacheProcMgr = DelayedSingleton<CacheProcessManager>::GetInstance();
        if (cacheProcMgr != nullptr && cacheProcMgr->IsAppShouldCache(appRecord)) {
            cacheProcMgr->PenddingCacheProcess(appRecord);
            TAG_LOGI(AAFwkTag::APPMGR, "App %{public}s not supports terminate record",
                appRecord->GetBundleName().c_str());
            return;
        }
        TAG_LOGI(AAFwkTag::APPMGR, "ability is the last:%{public}s", appRecord->GetName().c_str());
        appRecord->SetTerminating();
        std::string killReason = clearMissionFlag ? "Kill Reason:ClearSession" : "";
        appRecord->SetKillReason(killReason);
    }
}

void AppRunningManager::TerminateAbility(const sptr<IRemoteObject> &token, bool clearMissionFlag,
    std::shared_ptr<AppMgrServiceInner> appMgrServiceInner)
{
    auto appRecord = GetAppRunningRecordByAbilityToken(token);
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "null appRecord");
        return;
    }

    std::weak_ptr<AppRunningRecord> appRecordWeakPtr(appRecord);
    auto killProcess = [appRecordWeakPtr, token, inner = appMgrServiceInner]() {
        auto appRecordSptr = appRecordWeakPtr.lock();
        if (appRecordSptr == nullptr || token == nullptr || inner == nullptr) {
            TAG_LOGE(AAFwkTag::APPMGR, "parameter error");
            return;
        }
        appRecordSptr->RemoveTerminateAbilityTimeoutTask(token);
        TAG_LOGD(AAFwkTag::APPMGR, "The ability is the last, kill application");
        auto priorityObject = appRecordSptr->GetPriorityObject();
        if (priorityObject == nullptr) {
            TAG_LOGE(AAFwkTag::APPMGR, "null priorityObject");
            return;
        }
        auto pid = priorityObject->GetPid();
        if (pid < 0) {
            TAG_LOGE(AAFwkTag::APPMGR, "pid error");
            return;
        }
        auto result = inner->KillProcessByPid(pid, "TerminateAbility");
        if (result < 0) {
            TAG_LOGW(AAFwkTag::APPMGR, "failed, pid: %{public}d", pid);
        }
        inner->NotifyAppStatus(appRecordSptr->GetBundleName(), appRecordSptr->GetAppIndex(),
            CommonEventSupport::COMMON_EVENT_PACKAGE_RESTARTED);
        };

    auto isLastAbility = appRecord->IsLastAbilityByFlag(token, clearMissionFlag);
#ifdef SUPPORT_SCREEN
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        appRecord->TerminateAbility(token, true);
    } else {
        appRecord->TerminateAbility(token, false);
    }
#endif //SUPPORT_SCREEN
    auto isLauncherApp = appRecord->GetApplicationInfo()->isLauncherApp;
    auto isKeepAliveApp = appRecord->IsKeepAliveApp();
    TAG_LOGI(AAFwkTag::PROCESSMGR, "terminate isLast:%{public}d,keepAlive:%{public}d", isLastAbility, isKeepAliveApp);
    if (isLastAbility && !isKeepAliveApp && !isLauncherApp) {
        auto cacheProcMgr = DelayedSingleton<CacheProcessManager>::GetInstance();
        if (cacheProcMgr != nullptr) {
            cacheProcMgr->CheckAndSetProcessCacheEnable(appRecord);
        }
        if (cacheProcMgr != nullptr && cacheProcMgr->IsAppShouldCache(appRecord)) {
            cacheProcMgr->PenddingCacheProcess(appRecord);
            TAG_LOGI(AAFwkTag::APPMGR, "app %{public}s is not terminate app",
                appRecord->GetBundleName().c_str());
            if (clearMissionFlag) {
                NotifyAppPreCache(appRecord, appMgrServiceInner);
            }
            return;
        }
        TAG_LOGI(AAFwkTag::PROCESSMGR, "Terminate last:%{public}s.", appRecord->GetName().c_str());
        appRecord->SetTerminating();
        if ((clearMissionFlag || appRecord->IsPrepareExit()) && appMgrServiceInner != nullptr) {
            auto delayTime = appRecord->ExtensionAbilityRecordExists() ?
                AMSEventHandler::DELAY_KILL_EXTENSION_PROCESS_TIMEOUT : AMSEventHandler::DELAY_KILL_PROCESS_TIMEOUT;
            std::string taskName = std::string("DELAY_KILL_PROCESS_") + std::to_string(appRecord->GetRecordId());
            appRecord->PostTask(taskName, delayTime, killProcess);
        }
    }
}

void AppRunningManager::NotifyAppPreCache(const std::shared_ptr<AppRunningRecord>& appRecord,
    const std::shared_ptr<AppMgrServiceInner>& appMgrServiceInner)
{
    if (appMgrServiceInner == nullptr || appRecord == nullptr ||
        appRecord->GetPriorityObject() == nullptr) {
        return;
    }
    int32_t pid = appRecord->GetPid();
    int32_t userId = appRecord->GetUid() / BASE_USER_RANGE;
    auto notifyAppPreCache = [pid, userId, inner = appMgrServiceInner]() {
        if (inner == nullptr) {
            return;
        }
        inner->NotifyAppPreCache(pid, userId);
    };
    appRecord->PostTask("NotifyAppPreCache", 0, notifyAppPreCache);
}

void AppRunningManager::GetRunningProcessInfoByToken(
    const sptr<IRemoteObject> &token, AppExecFwk::RunningProcessInfo &info)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto appRecord = GetAppRunningRecordByAbilityToken(token);
    AssignRunningProcessInfoByAppRecord(appRecord, info);
}

int32_t AppRunningManager::GetRunningProcessInfoByPid(const pid_t pid, OHOS::AppExecFwk::RunningProcessInfo &info)
{
    if (pid <= 0) {
        TAG_LOGE(AAFwkTag::APPMGR, "invalid process pid:%{public}d", pid);
        return ERR_INVALID_OPERATION;
    }
    auto appRecord = GetAppRunningRecordByPid(pid);
    return AssignRunningProcessInfoByAppRecord(appRecord, info);
}

int32_t AppRunningManager::GetRunningProcessInfoByChildProcessPid(const pid_t childPid,
    OHOS::AppExecFwk::RunningProcessInfo &info)
{
    if (childPid <= 0) {
        TAG_LOGE(AAFwkTag::APPMGR, "invalid process pid:%{public}d", childPid);
        return ERR_INVALID_OPERATION;
    }
    auto appRecord = GetAppRunningRecordByPid(childPid);
    if (appRecord == nullptr) {
        TAG_LOGI(AAFwkTag::APPMGR, "null appRecord, try get by child pid");
        appRecord = GetAppRunningRecordByChildProcessPid(childPid);
    }
    return AssignRunningProcessInfoByAppRecord(appRecord, info);
}

int32_t AppRunningManager::AssignRunningProcessInfoByAppRecord(
    std::shared_ptr<AppRunningRecord> appRecord, AppExecFwk::RunningProcessInfo &info) const
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (!appRecord) {
        TAG_LOGD(AAFwkTag::APPMGR, "null");
        return ERR_INVALID_OPERATION;
    }

    info.processName_ = appRecord->GetProcessName();
    info.pid_ = appRecord->GetPid();
    info.uid_ = appRecord->GetUid();
    info.bundleNames.emplace_back(appRecord->GetBundleName());
    info.state_ = static_cast<AppExecFwk::AppProcessState>(appRecord->GetState());
    info.isContinuousTask = appRecord->IsContinuousTask();
    info.isKeepAlive = appRecord->IsKeepAliveApp();
    info.isFocused = appRecord->GetFocusFlag();
    info.isTestProcess = (appRecord->GetUserTestInfo() != nullptr);
    info.startTimeMillis_ = appRecord->GetAppStartTime();
    info.isAbilityForegrounding = appRecord->GetAbilityForegroundingFlag();
    info.isTestMode = info.isTestProcess && system::GetBoolParameter(DEVELOPER_MODE_STATE, false);
    info.extensionType_ = appRecord->GetExtensionType();
    info.processType_ = appRecord->GetProcessType();
    info.isStrictMode = appRecord->IsStrictMode();
    auto appInfo = appRecord->GetApplicationInfo();
    if (appInfo) {
        info.bundleType = static_cast<int32_t>(appInfo->bundleType);
        info.appMode = appInfo->multiAppMode.multiAppModeType;
    }
    info.appCloneIndex = appRecord->GetAppIndex();
    info.instanceKey = appRecord->GetInstanceKey();
    info.rssValue = appRecord->GetRssValue();
    info.pssValue = appRecord->GetPssValue();
    return ERR_OK;
}

void AppRunningManager::SetAbilityForegroundingFlagToAppRecord(const pid_t pid)
{
    auto appRecord = GetAppRunningRecordByPid(pid);
    if (appRecord == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null appRecord");
        return;
    }
    appRecord->SetAbilityForegroundingFlag();
}

void AppRunningManager::ClipStringContent(const std::regex &re, const std::string &source, std::string &afterCutStr)
{
    std::smatch basket;
    try {
        if (std::regex_search(source, basket, re)) {
            afterCutStr = basket.prefix().str() + basket.suffix().str();
        }
    } catch (...) {
        TAG_LOGE(AAFwkTag::APPMGR, "regex failed");
    }
}

void AppRunningManager::GetForegroundApplications(std::vector<AppStateData> &list)
{
    std::lock_guard guard(runningRecordMapMutex_);
    for (const auto &item : appRunningRecordMap_) {
        const auto &appRecord = item.second;
        if (!appRecord) {
            TAG_LOGE(AAFwkTag::APPMGR, "null appRecord");
            return;
        }
        auto state = appRecord->GetState();
        if (state == ApplicationState::APP_STATE_FOREGROUND) {
            AppStateData appData;
            appData.bundleName = appRecord->GetBundleName();
            appData.uid = appRecord->GetUid();
            appData.pid = appRecord->GetPid();
            appData.state = static_cast<int32_t>(ApplicationState::APP_STATE_FOREGROUND);
            auto appInfo = appRecord->GetApplicationInfo();
            appData.accessTokenId = appInfo ? appInfo->accessTokenId : 0;
            appData.extensionType = appRecord->GetExtensionType();
            appData.isFocused = appRecord->GetFocusFlag();
            appData.appIndex = appRecord->GetAppIndex();
            list.push_back(appData);
            TAG_LOGD(AAFwkTag::APPMGR, "bundleName:%{public}s", appData.bundleName.c_str());
        }
    }
}
int32_t AppRunningManager::UpdateConfiguration(const Configuration& config, const int32_t userId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);

    auto appRunningMap = GetAppRunningRecordMap();
    TAG_LOGD(AAFwkTag::APPMGR, "current app size %{public}zu", appRunningMap.size());

    {
        std::lock_guard guard(appInfosLock_);
        for (auto &info : appInfos_) {
            AAFwk::TaskHandlerWrap::GetFfrtHandler()->CancelTask(
                info.bandleName.c_str() + std::to_string(info.appIndex));
        }
        appInfos_.clear();
    }

    for (const auto& item : appRunningMap) {
        const auto& appRecord = item.second;
        if (appRecord && appRecord->GetState() == ApplicationState::APP_STATE_CREATE) {
            TAG_LOGD(AAFwkTag::APPMGR, "app not ready, appName is %{public}s", appRecord->GetBundleName().c_str());
            continue;
        }
        if (!(userId == -1 || appRecord->GetUid() / BASE_USER_RANGE == 0 ||
                appRecord->GetUid() / BASE_USER_RANGE == userId)) {
            continue;
        }
        if (appRecord->GetDelayConfiguration() == nullptr) {
            appRecord->ResetDelayConfiguration();
        }
        if (appRecord) {
            TAG_LOGD(AAFwkTag::APPMGR, "Notification app [%{public}s]", appRecord->GetName().c_str());
            std::lock_guard guard(updateConfigurationDelayedLock_);
            int32_t result = ERR_OK;
            if (appRecord->NeedUpdateConfigurationBackground() ||
                appRecord->GetState() != ApplicationState::APP_STATE_BACKGROUND) {
                updateConfigurationDelayedMap_[appRecord->GetRecordId()] = false;
                result = appRecord->UpdateConfiguration(config);
            } else {
                auto delayConfig = appRecord->GetDelayConfiguration();
                std::vector<std::string> diffVe;
                delayConfig->CompareDifferent(diffVe, config);
                delayConfig->Merge(diffVe, config);
                updateConfigurationDelayedMap_[appRecord->GetRecordId()] = true;
            }
            if (result != ERR_OK) {
                TAG_LOGE(AAFwkTag::APPMGR,
                    "UpdateConfig failed app:%{public}s %{public}d", appRecord->GetName().c_str(), result);
            }
        }
    }
    return ERR_OK;
}

bool AppRunningManager::UpdateConfiguration(std::shared_ptr<AppRunningRecord>& appRecord,
    Rosen::ConfigMode configMode, ConfigUpdateReason reason)
{
    if (appRecord == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null ptr");
        return false;
    }

    if (configMode != Rosen::ConfigMode::COLOR_MODE) {
        TAG_LOGI(AAFwkTag::APPKIT, "not color mode");
        return false;
    }
    AppExecFwk::Configuration config;
    auto delayConfig = appRecord->GetDelayConfiguration();
    if (delayConfig == nullptr) {
        appRecord->ResetDelayConfiguration();
        TAG_LOGE(AAFwkTag::APPKIT, "delayConfig null");
        return false;
    } else {
        std::string value = delayConfig->GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_COLORMODE);
        if (value == ConfigurationInner::EMPTY_STRING) {
            TAG_LOGE(AAFwkTag::APPKIT, "colorMode null");
            return false;
        }
        config.AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_COLORMODE, value);
        TAG_LOGI(AAFwkTag::APPKIT, "colorMode: %{public}s", value.c_str());
    }
    delayConfig->RemoveItem(AAFwk::GlobalConfigurationKey::SYSTEM_COLORMODE);
    appRecord->UpdateConfiguration(config, reason);
    return true;
}

void AppRunningManager::ExecuteConfigurationTask(const BackgroundAppInfo& info, const int32_t userId,
    ConfigUpdateReason reason)
{
    auto appRunningMap = GetAppRunningRecordMap();

    std::lock_guard guard(updateConfigurationDelayedLock_);
    for (auto &item : updateConfigurationDelayedMap_) {
        std::shared_ptr<AppRunningRecord> appRecord = nullptr;
        auto it = appRunningMap.find(item.first);
        if (it != appRunningMap.end()) {
            appRecord = it->second;
        }
        if (appRecord == nullptr) {
            continue;
        }
        bool userIdFlag = (userId == -1 || appRecord->GetUid() / BASE_USER_RANGE == 0 ||
                           appRecord->GetUid() / BASE_USER_RANGE == userId);
        if (!userIdFlag) {
            continue;
        }

        if (info.bandleName == appRecord->GetBundleName() && info.appIndex == appRecord->GetAppIndex() && item.second
            && appRecord->GetState() == ApplicationState::APP_STATE_BACKGROUND) {
            if (UpdateConfiguration(appRecord, Rosen::ConfigMode::COLOR_MODE, reason)) {
                item.second = false;
            }
        }
    }

    return;
}

int32_t AppRunningManager::UpdateConfigurationForBackgroundApp(const std::vector<BackgroundAppInfo> &appInfos,
    const AppExecFwk::ConfigurationPolicy& policy, const int32_t userId)
{
    int8_t maxCountPerBatch = policy.maxCountPerBatch;
    if (maxCountPerBatch < 1) {
        TAG_LOGE(AAFwkTag::APPMGR, "maxCountPerBatch invalid");
        return ERR_INVALID_VALUE;
    }

    int16_t intervalTime = policy.intervalTime;
    if (intervalTime < 0) {
        TAG_LOGE(AAFwkTag::APPMGR, "intervalTime invalid");
        return ERR_INVALID_VALUE;
    }

    {
        std::lock_guard guard(appInfosLock_);
        for (auto &info : appInfos_) {
            AAFwk::TaskHandlerWrap::GetFfrtHandler()->CancelTask(
                info.bandleName.c_str() + std::to_string(info.appIndex));
        }
        appInfos_ = appInfos;
    }

    int32_t taskCount = 0;
    int32_t batchCount = 0;
    for (auto &info : appInfos) {
        auto policyTask = [weak = weak_from_this(), info, userId] {
            auto appRuningMgr = weak.lock();
            if (appRuningMgr == nullptr) {
                TAG_LOGE(AAFwkTag::APPMGR, "appRuningMgr null");
                return;
            }
            appRuningMgr->ExecuteConfigurationTask(
                info, userId, ConfigUpdateReason::CONFIG_UPDATE_REASON_IN_WHITE_LIST);
        };
        AAFwk::TaskHandlerWrap::GetFfrtHandler()->SubmitTask(policyTask,
            info.bandleName.c_str() + std::to_string(info.appIndex), intervalTime * batchCount);
        if (++taskCount >= maxCountPerBatch) {
            batchCount++;
            taskCount = 0;
        }
    }

    return ERR_OK;
}

int32_t AppRunningManager::UpdateConfigurationByBundleName(const Configuration &config, const std::string &name,
    int32_t appIndex)
{
    auto appRunningMap = GetAppRunningRecordMap();
    int32_t result = ERR_OK;
    for (const auto &item : appRunningMap) {
        const auto &appRecord = item.second;
        if (appRecord && appRecord->GetState() == ApplicationState::APP_STATE_CREATE) {
            TAG_LOGD(AAFwkTag::APPMGR, "app not ready, appName is %{public}s", appRecord->GetBundleName().c_str());
            continue;
        }
        if (appRecord && appRecord->GetBundleName() == name &&
            appRecord->GetAppIndex() == appIndex) {
            TAG_LOGD(AAFwkTag::APPMGR, "Notification app [%{public}s], index:%{public}d",
                appRecord->GetName().c_str(), appIndex);
            result = appRecord->UpdateConfiguration(config);
        }
    }
    return result;
}

int32_t AppRunningManager::NotifyMemoryLevel(int32_t level)
{
    auto appRunningMap = GetAppRunningRecordMap();
    for (const auto &item : appRunningMap) {
        const auto &appRecord = item.second;
        if (!appRecord) {
            TAG_LOGE(AAFwkTag::APPMGR, "appRecord null");
            continue;
        }
        appRecord->ScheduleMemoryLevel(level);
    }
    return ERR_OK;
}

int32_t AppRunningManager::NotifyProcMemoryLevel(const std::map<pid_t, MemoryLevel> &procLevelMap, bool isShellCall)
{
    auto appRunningMap = GetAppRunningRecordMap();
    for (const auto &item : appRunningMap) {
        const auto &appRecord = item.second;
        if (!appRecord) {
            TAG_LOGE(AAFwkTag::APPMGR, "appRecord null");
            continue;
        }
        auto priorityObject = appRecord->GetPriorityObject();
        if (!priorityObject) {
            TAG_LOGW(AAFwkTag::APPMGR, "priorityObject null");
            continue;
        }
        auto pid = priorityObject->GetPid();
        auto it = procLevelMap.find(pid);
        if (it == procLevelMap.end()) {
            TAG_LOGW(AAFwkTag::APPMGR, "pid%{public}d not found", pid);
        } else {
            TAG_LOGD(AAFwkTag::APPMGR, "pid%{public}d memory level = %{public}d", pid, it->second);
            appRecord->ScheduleMemoryLevel(it->second, isShellCall);
        }
    }
    return ERR_OK;
}

int32_t AppRunningManager::DumpHeapMemory(const int32_t pid, OHOS::AppExecFwk::MallocInfo &mallocInfo)
{
    std::shared_ptr<AppRunningRecord> appRecord;
    {
        std::lock_guard guard(runningRecordMapMutex_);
        TAG_LOGI(AAFwkTag::APPMGR, "app size %{public}zu", appRunningRecordMap_.size());
        auto iter = std::find_if(appRunningRecordMap_.begin(), appRunningRecordMap_.end(), [&pid](const auto &pair) {
            auto priorityObject = pair.second->GetPriorityObject();
            return priorityObject && priorityObject->GetPid() == pid;
        });
        if (iter == appRunningRecordMap_.end()) {
            TAG_LOGE(AAFwkTag::APPMGR, "no application found");
            return ERR_INVALID_VALUE;
        }
        appRecord = iter->second;
        if (appRecord == nullptr) {
            TAG_LOGE(AAFwkTag::APPMGR, "null appRecord");
            return ERR_INVALID_VALUE;
        }
    }
    appRecord->ScheduleHeapMemory(pid, mallocInfo);
    return ERR_OK;
}

int32_t AppRunningManager::DumpJsHeapMemory(OHOS::AppExecFwk::JsHeapDumpInfo &info)
{
    int32_t pid = static_cast<int32_t>(info.pid);
    auto appRecord = GetAppRunningRecordByPid(pid);
    if (appRecord == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null appRecord");
        return ERR_INVALID_VALUE;
    }
    appRecord->ScheduleJsHeapMemory(info);
    return ERR_OK;
}

int32_t AppRunningManager::DumpCjHeapMemory(OHOS::AppExecFwk::CjHeapDumpInfo &info)
{
    auto appRecord = GetAppRunningRecordByPid(info.pid);
    if (appRecord == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null appRecord");
        return ERR_INVALID_VALUE;
    }
    appRecord->ScheduleCjHeapMemory(info);
    return ERR_OK;
}

std::shared_ptr<AppRunningRecord> AppRunningManager::GetAppRunningRecordByRenderPid(const pid_t pid)
{
    std::lock_guard guard(runningRecordMapMutex_);
    auto iter = std::find_if(appRunningRecordMap_.begin(), appRunningRecordMap_.end(), [&pid](const auto &pair) {
        auto renderRecordMap = pair.second->GetRenderRecordMap();
        if (renderRecordMap.empty()) {
            return false;
        }
        for (auto it : renderRecordMap) {
            auto renderRecord = it.second;
            if (renderRecord && renderRecord->GetPid() == pid) {
                return true;
            }
        }
        return false;
    });
    return ((iter == appRunningRecordMap_.end()) ? nullptr : iter->second);
}

std::shared_ptr<RenderRecord> AppRunningManager::OnRemoteRenderDied(const wptr<IRemoteObject> &remote)
{
    if (remote == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null remote");
        return nullptr;
    }
    sptr<IRemoteObject> object = remote.promote();
    if (!object) {
        TAG_LOGE(AAFwkTag::APPMGR, "promote failed");
        return nullptr;
    }

    std::lock_guard guard(runningRecordMapMutex_);
    std::shared_ptr<RenderRecord> renderRecord;
    const auto &it =
        std::find_if(appRunningRecordMap_.begin(), appRunningRecordMap_.end(),
            [&object, &renderRecord](const auto &pair) {
            if (!pair.second) {
                return false;
            }

            auto renderRecordMap = pair.second->GetRenderRecordMap();
            if (renderRecordMap.empty()) {
                return false;
            }
            for (auto iter : renderRecordMap) {
                if (iter.second == nullptr) {
                    continue;
                }
                auto scheduler = iter.second->GetScheduler();
                if (scheduler && scheduler->AsObject() == object) {
                    renderRecord = iter.second;
                    return true;
                }
            }
            return false;
        });
    if (it != appRunningRecordMap_.end()) {
        auto appRecord = it->second;
        appRecord->RemoveRenderRecord(renderRecord);
        TAG_LOGI(AAFwkTag::APPMGR, "RemoveRenderRecord pid:%{public}d, uid:%{public}d", renderRecord->GetPid(),
            renderRecord->GetUid());
        return renderRecord;
    }
    return nullptr;
}

bool AppRunningManager::GetAppRunningStateByBundleName(const std::string &bundleName)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    std::lock_guard guard(runningRecordMapMutex_);
    for (const auto &item : appRunningRecordMap_) {
        const auto &appRecord = item.second;
        if (appRecord && appRecord->GetBundleName() == bundleName) {
            TAG_LOGD(AAFwkTag::APPMGR, "Process of [%{public}s] is running, processName: %{public}s.",
                bundleName.c_str(), appRecord->GetProcessName().c_str());
            if (IPCSkeleton::GetCallingUid() == QUICKFIX_UID && appRecord->GetPriorityObject() != nullptr) {
                TAG_LOGI(AAFwkTag::APPMGR, "pid: %{public}d", appRecord->GetPid());
            }
            return true;
        }
    }
    return false;
}

int32_t AppRunningManager::NotifyLoadRepairPatch(const std::string &bundleName, const sptr<IQuickFixCallback> &callback)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    int32_t result = ERR_OK;
    bool loadSucceed = false;
    auto callbackByRecord = sptr<QuickFixCallbackWithRecord>::MakeSptr(callback);
    if (callbackByRecord == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null record");
        return ERR_INVALID_VALUE;
    }

    auto appRunningMap = GetAppRunningRecordMap();
    for (const auto &item : appRunningMap) {
        const auto &appRecord = item.second;
        if (appRecord && appRecord->GetBundleName() == bundleName) {
            auto recordId = appRecord->GetRecordId();
            TAG_LOGD(AAFwkTag::APPMGR, "Notify application [%{public}s] load patch, record id %{public}d.",
                appRecord->GetProcessName().c_str(), recordId);
            callbackByRecord->AddRecordId(recordId);
            result = appRecord->NotifyLoadRepairPatch(bundleName, callbackByRecord, recordId);
            if (result == ERR_OK) {
                loadSucceed = true;
            } else {
                callbackByRecord->RemoveRecordId(recordId);
            }
        }
    }
    return loadSucceed == true ? ERR_OK : result;
}

int32_t AppRunningManager::NotifyHotReloadPage(const std::string &bundleName, const sptr<IQuickFixCallback> &callback)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    int32_t result = ERR_OK;
    bool reloadPageSucceed = false;
    auto callbackByRecord = sptr<QuickFixCallbackWithRecord>::MakeSptr(callback);
    if (callbackByRecord == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null record");
        return ERR_INVALID_VALUE;
    }

    auto appRunningMap = GetAppRunningRecordMap();
    for (const auto &item : appRunningMap) {
        const auto &appRecord = item.second;
        if (appRecord && appRecord->GetBundleName() == bundleName) {
            auto recordId = appRecord->GetRecordId();
            TAG_LOGD(AAFwkTag::APPMGR, "Notify application [%{public}s] reload page, record id %{public}d.",
                appRecord->GetProcessName().c_str(), recordId);
            callbackByRecord->AddRecordId(recordId);
            result = appRecord->NotifyHotReloadPage(callbackByRecord, recordId);
            if (result == ERR_OK) {
                reloadPageSucceed = true;
            } else {
                callbackByRecord->RemoveRecordId(recordId);
            }
        }
    }
    return reloadPageSucceed == true ? ERR_OK : result;
}

int32_t AppRunningManager::NotifyUnLoadRepairPatch(const std::string &bundleName,
    const sptr<IQuickFixCallback> &callback)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    int32_t result = ERR_OK;
    bool unLoadSucceed = false;
    auto callbackByRecord = sptr<QuickFixCallbackWithRecord>::MakeSptr(callback);
    if (callbackByRecord == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null record");
        return ERR_INVALID_VALUE;
    }

    auto appRunningMap = GetAppRunningRecordMap();
    for (const auto &item : appRunningMap) {
        const auto &appRecord = item.second;
        if (appRecord && appRecord->GetBundleName() == bundleName) {
            auto recordId = appRecord->GetRecordId();
            TAG_LOGD(AAFwkTag::APPMGR, "Notify application [%{public}s] unload patch, record id %{public}d.",
                appRecord->GetProcessName().c_str(), recordId);
            callbackByRecord->AddRecordId(recordId);
            result = appRecord->NotifyUnLoadRepairPatch(bundleName, callbackByRecord, recordId);
            if (result == ERR_OK) {
                unLoadSucceed = true;
            } else {
                callbackByRecord->RemoveRecordId(recordId);
            }
        }
    }
    return unLoadSucceed == true ? ERR_OK : result;
}

bool AppRunningManager::IsApplicationFirstForeground(const AppRunningRecord &foregroundingRecord)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    if (AAFwk::UIExtensionWrapper::IsUIExtension(foregroundingRecord.GetExtensionType())
        || AAFwk::UIExtensionWrapper::IsWindowExtension(foregroundingRecord.GetExtensionType())) {
        return false;
    }

    std::lock_guard guard(runningRecordMapMutex_);
    for (const auto &item : appRunningRecordMap_) {
        const auto &appRecord = item.second;
        if (appRecord == nullptr || appRecord->GetUid() != foregroundingRecord.GetUid()
            || AAFwk::UIExtensionWrapper::IsUIExtension(appRecord->GetExtensionType())
            || AAFwk::UIExtensionWrapper::IsWindowExtension(appRecord->GetExtensionType())
            || appRecord->GetAppIndex() != foregroundingRecord.GetAppIndex()) {
            continue;
        }
        auto state = appRecord->GetState();
        if (state == ApplicationState::APP_STATE_FOREGROUND &&
            appRecord->GetRecordId() != foregroundingRecord.GetRecordId()) {
            return false;
        }
    }
    return true;
}

bool AppRunningManager::IsApplicationBackground(const AppRunningRecord &backgroundingRecord)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    std::lock_guard guard(runningRecordMapMutex_);
    for (const auto &item : appRunningRecordMap_) {
        const auto &appRecord = item.second;
        if (appRecord == nullptr) {
            TAG_LOGE(AAFwkTag::APPMGR, "null appRecord");
            return false;
        }
        if (appRecord->GetUid() != backgroundingRecord.GetUid()
            || AAFwk::UIExtensionWrapper::IsUIExtension(appRecord->GetExtensionType())
            || AAFwk::UIExtensionWrapper::IsWindowExtension(appRecord->GetExtensionType())
            || appRecord->GetAppIndex() != backgroundingRecord.GetAppIndex()) {
            continue;
        }
        auto state = appRecord->GetState();
        if (state == ApplicationState::APP_STATE_FOREGROUND) {
            return false;
        }
    }
    return true;
}

bool AppRunningManager::NeedNotifyAppStateChangeWhenProcessDied(std::shared_ptr<AppRunningRecord> currentAppRecord)
{
    TAG_LOGD(AAFwkTag::APPMGR, "NeedNotifyAppStateChangeWhenProcessDied called");
    if (currentAppRecord == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null currentAppRecord");
        return false;
    }
    if (currentAppRecord->GetState() != ApplicationState::APP_STATE_FOREGROUND) {
        return false;
    }
    std::lock_guard guard(runningRecordMapMutex_);
    bool needNotify = false;
    for (const auto &item : appRunningRecordMap_) {
        const auto &appRecord = item.second;
        if (appRecord == nullptr) {
            TAG_LOGE(AAFwkTag::APPMGR, "null appRecord");
            continue;
        }
        if (appRecord == currentAppRecord) {
            continue;
        }
        if (AAFwk::UIExtensionWrapper::IsUIExtension(appRecord->GetExtensionType())
            || AAFwk::UIExtensionWrapper::IsWindowExtension(appRecord->GetExtensionType())) {
            continue;
        }
        auto state = appRecord->GetState();
        if (appRecord->GetUid() != currentAppRecord->GetUid()) {
            continue;
        }
        if (state == ApplicationState::APP_STATE_FOREGROUND) {
            return false;
        }
        needNotify = true;
    }
    return needNotify;
}

#ifdef SUPPORT_SCREEN
void AppRunningManager::OnWindowVisibilityChanged(
    const std::vector<sptr<OHOS::Rosen::WindowVisibilityInfo>> &windowVisibilityInfos)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    std::set<int32_t> pids;
    for (const auto &info : windowVisibilityInfos) {
        if (info == nullptr) {
            TAG_LOGE(AAFwkTag::APPMGR, "null info");
            continue;
        }
        if (pids.find(info->pid_) != pids.end()) {
            continue;
        }
        auto appRecord = GetAppRunningRecordByPid(info->pid_);
        if (appRecord == nullptr) {
            TAG_LOGE(AAFwkTag::APPMGR, "null appRecord");
            return;
        }
        TAG_LOGD(AAFwkTag::APPMGR, "The visibility of %{public}s was changed.", appRecord->GetBundleName().c_str());
        appRecord->OnWindowVisibilityChanged(windowVisibilityInfos);
        pids.emplace(info->pid_);
    }
}
#endif //SUPPORT_SCREEN
bool AppRunningManager::IsApplicationFirstFocused(const AppRunningRecord &focusedRecord)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    std::lock_guard guard(runningRecordMapMutex_);
    for (const auto &item : appRunningRecordMap_) {
        const auto &appRecord = item.second;
        if (appRecord == nullptr || appRecord->GetUid() != focusedRecord.GetUid()) {
            continue;
        }
        if (appRecord->GetFocusFlag() && appRecord->GetRecordId() != focusedRecord.GetRecordId()) {
            return false;
        }
    }
    return true;
}

bool AppRunningManager::IsApplicationUnfocused(const int32_t uid)
{
    TAG_LOGD(AAFwkTag::APPMGR, "check is application unfocused.");
    std::lock_guard guard(runningRecordMapMutex_);
    for (const auto &item : appRunningRecordMap_) {
        const auto &appRecord = item.second;
        if (appRecord && appRecord->GetUid() == uid && appRecord->GetFocusFlag()) {
            return false;
        }
    }
    return true;
}

void AppRunningManager::SetAttachAppDebug(const std::string &bundleName, const bool &isAttachDebug,
    bool isDebugFromLocal)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    auto appRunningMap = GetAppRunningRecordMap();
    for (const auto &item : appRunningMap) {
        const auto &appRecord = item.second;
        if (appRecord == nullptr) {
            continue;
        }
        if (appRecord->GetBundleName() == bundleName) {
            TAG_LOGD(AAFwkTag::APPMGR, "The application: %{public}s will be set debug mode.", bundleName.c_str());
            appRecord->SetAttachDebug(isAttachDebug, isDebugFromLocal);
        }
    }
}

std::vector<AppDebugInfo> AppRunningManager::GetAppDebugInfosByBundleName(
    const std::string &bundleName, const bool &isDetachDebug)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    std::lock_guard guard(runningRecordMapMutex_);
    std::vector<AppDebugInfo> debugInfos;
    for (const auto &item : appRunningRecordMap_) {
        const auto &appRecord = item.second;
        if (appRecord == nullptr || appRecord->GetBundleName() != bundleName ||
            (isDetachDebug && (appRecord->IsDebugApp() || appRecord->IsAssertionPause()))) {
            continue;
        }

        AppDebugInfo debugInfo;
        debugInfo.bundleName = bundleName;
        auto priorityObject = appRecord->GetPriorityObject();
        if (priorityObject) {
            debugInfo.pid = priorityObject->GetPid();
        }
        debugInfo.uid = appRecord->GetUid();
        debugInfo.isDebugStart = (appRecord->IsDebugApp() || appRecord->IsAssertionPause());
        debugInfos.emplace_back(debugInfo);
    }
    return debugInfos;
}

void AppRunningManager::GetAbilityTokensByBundleName(
    const std::string &bundleName, std::vector<sptr<IRemoteObject>> &abilityTokens)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    std::lock_guard guard(runningRecordMapMutex_);
    for (const auto &item : appRunningRecordMap_) {
        const auto &appRecord = item.second;
        if (appRecord == nullptr || appRecord->GetBundleName() != bundleName) {
            continue;
        }

        for (const auto &token : appRecord->GetAbilities()) {
            abilityTokens.emplace_back(token.first);
        }
    }
}

#ifdef SUPPORT_CHILD_PROCESS
std::shared_ptr<AppRunningRecord> AppRunningManager::GetAppRunningRecordByChildProcessPid(const pid_t pid)
{
    std::lock_guard guard(runningRecordMapMutex_);
    auto iter = std::find_if(appRunningRecordMap_.begin(), appRunningRecordMap_.end(), [&pid](const auto &pair) {
        auto childProcessRecordMap = pair.second->GetChildProcessRecordMap();
        return childProcessRecordMap.find(pid) != childProcessRecordMap.end();
    });
    if (iter != appRunningRecordMap_.end()) {
        return iter->second;
    }
    return nullptr;
}

bool AppRunningManager::IsChildProcessReachLimit(uint32_t accessTokenId, bool multiProcessFeature)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called.");
    int32_t childCount = 0;
    std::lock_guard guard(runningRecordMapMutex_);
    for (auto &pair : appRunningRecordMap_) {
        auto appRecord = pair.second;
        if (!appRecord || !appRecord->GetApplicationInfo() ||
            accessTokenId != appRecord->GetApplicationInfo()->accessTokenId) {
            continue;
        }
        childCount += appRecord->GetChildProcessCount();
    }
    auto maxCount = multiProcessFeature ? AAFwk::AppUtils::GetInstance().MaxMultiProcessFeatureChildProcess() :
        AAFwk::AppUtils::GetInstance().MaxChildProcess();
    return childCount >= maxCount;
}

void AppRunningManager::HandleChildRelation(
    std::shared_ptr<ChildProcessRecord> childRecord, std::shared_ptr<AppRunningRecord> appRecord)
{
    if (!childRecord || !appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "null child or app record");
        return;
    }
    if (childRecord->IsNativeSpawnStarted() &&
        AppNativeSpawnManager::GetInstance().GetNativeChildCallbackByPid(appRecord->GetPid()) != nullptr) {
        AppNativeSpawnManager::GetInstance().AddChildRelation(childRecord->GetPid(), appRecord->GetPid());
    }
    int32_t childPid = childRecord->GetPid();
    auto delayClear = [childPid]() {
        AppNativeSpawnManager::GetInstance().RemoveChildRelation(childPid);
    };
    AAFwk::TaskHandlerWrap::GetFfrtHandler()->SubmitTask(delayClear, "HandleChildRelation",
        DEAD_CHILD_RELATION_CLEAR_TIME);
}

std::shared_ptr<ChildProcessRecord> AppRunningManager::OnChildProcessRemoteDied(const wptr<IRemoteObject> &remote)
{
    TAG_LOGD(AAFwkTag::APPMGR, "On child process remote died");
    if (remote == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null remote");
        return nullptr;
    }
    sptr<IRemoteObject> object = remote.promote();
    if (!object) {
        TAG_LOGE(AAFwkTag::APPMGR, "promote failed");
        return nullptr;
    }

    std::lock_guard guard(runningRecordMapMutex_);
    std::shared_ptr<ChildProcessRecord> childRecord;
    const auto &it = std::find_if(appRunningRecordMap_.begin(), appRunningRecordMap_.end(),
        [&object, &childRecord](const auto &pair) {
            auto appRecord = pair.second;
            if (!appRecord) {
                return false;
            }
            auto childRecordMap = appRecord->GetChildProcessRecordMap();
            if (childRecordMap.empty()) {
                return false;
            }
            for (auto iter : childRecordMap) {
                if (iter.second == nullptr) {
                    continue;
                }
                auto scheduler = iter.second->GetScheduler();
                if (scheduler && scheduler->AsObject() == object) {
                    childRecord = iter.second;
                    return true;
                }
            }
            return false;
        });
    if (it != appRunningRecordMap_.end()) {
        auto appRecord = it->second;
        HandleChildRelation(childRecord, appRecord);
        appRecord->RemoveChildProcessRecord(childRecord);
        TAG_LOGI(AAFwkTag::APPMGR, "RemoveChildProcessRecord pid:%{public}d, uid:%{public}d", childRecord->GetPid(),
            childRecord->GetUid());
        return childRecord;
    }
    return nullptr;
}
#endif //SUPPORT_CHILD_PROCESS

std::shared_ptr<AppRunningRecord> AppRunningManager::GetAppRunningRecordByChildRecordPid(const pid_t pid)
{
    std::lock_guard guard(runningRecordMapMutex_);
    auto iter = std::find_if(appRunningRecordMap_.begin(), appRunningRecordMap_.end(), [&pid](const auto &pair) {
        auto childAppRecordMap = pair.second->GetChildAppRecordMap();
        return childAppRecordMap.find(pid) != childAppRecordMap.end();
    });
    if (iter != appRunningRecordMap_.end()) {
        return iter->second;
    }
    return nullptr;
}

int32_t AppRunningManager::SignRestartAppFlag(int32_t uid, const std::string &instanceKey)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    std::lock_guard guard(runningRecordMapMutex_);
    for (const auto &item : appRunningRecordMap_) {
        const auto &appRecord = item.second;
        if (appRecord == nullptr || appRecord->GetUid() != uid || appRecord->GetInstanceKey() != instanceKey) {
            continue;
        }
        TAG_LOGI(AAFwkTag::APPMGR, "SignRestartAppFlag");
        appRecord->SetRestartAppFlag(true);
        return ERR_OK;
    }
    TAG_LOGE(AAFwkTag::APPMGR, "null apprecord");
    return ERR_INVALID_VALUE;
}

int32_t AppRunningManager::SignRestartProcess(int32_t pid)
{
    std::lock_guard guard(runningRecordMapMutex_);
    for (const auto &item : appRunningRecordMap_) {
        const auto &appRecord = item.second;
        if (appRecord == nullptr || appRecord->GetPid() != pid) {
            continue;
        }
        TAG_LOGI(AAFwkTag::APPMGR, "SignRestartProcess");
        appRecord->SetRestartAppFlag(true);
        return ERR_OK;
    }
    TAG_LOGE(AAFwkTag::APPMGR, "null apprecord");
    return ERR_INVALID_VALUE;
}

int32_t AppRunningManager::GetAppRunningUniqueIdByPid(pid_t pid, std::string &appRunningUniqueId)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    auto appRecord = GetAppRunningRecordByPid(pid);
    if (appRecord == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null appRecord");
        return ERR_INVALID_VALUE;
    }
    appRunningUniqueId = std::to_string(appRecord->GetAppRunningUniqueId());
    TAG_LOGD(AAFwkTag::APPMGR, "appRunningUniqueId = %{public}s.", appRunningUniqueId.c_str());
    return ERR_OK;
}

int32_t AppRunningManager::GetAllUIExtensionRootHostPid(pid_t pid, std::vector<pid_t> &hostPids)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    std::lock_guard guard(uiExtensionMapLock_);
    for (auto &item: uiExtensionLauncherMap_) {
        auto temp = item.second.second;
        if (temp == pid) {
            hostPids.emplace_back(item.second.first);
        }
    }
    std::string hostPidStr = std::accumulate(hostPids.begin(), hostPids.end(), std::string(),
        [](const std::string& a, pid_t b) {
            return a + std::to_string(b) + " ";
        });
    TAG_LOGD(AAFwkTag::APPMGR, "pid: %{public}s, hostPid: %{public}s.", std::to_string(pid).c_str(),
        hostPidStr.c_str());
    return ERR_OK;
}

int32_t AppRunningManager::GetAllUIExtensionProviderPid(pid_t hostPid, std::vector<pid_t> &providerPids)
{
    std::lock_guard guard(uiExtensionMapLock_);
    for (auto &item: uiExtensionLauncherMap_) {
        auto temp = item.second.first;
        if (temp == hostPid) {
            providerPids.emplace_back(item.second.second);
        }
    }

    return ERR_OK;
}

int32_t AppRunningManager::AddUIExtensionLauncherItem(int32_t uiExtensionAbilityId, pid_t hostPid, pid_t providerPid)
{
    std::lock_guard guard(uiExtensionMapLock_);
    uiExtensionLauncherMap_.emplace(uiExtensionAbilityId, std::pair<pid_t, pid_t>(hostPid, providerPid));
    return ERR_OK;
}

int32_t AppRunningManager::RemoveUIExtensionLauncherItem(pid_t pid)
{
    std::lock_guard guard(uiExtensionMapLock_);
    for (auto it = uiExtensionLauncherMap_.begin(); it != uiExtensionLauncherMap_.end();) {
        if (it->second.first == pid || it->second.second == pid) {
            it = uiExtensionLauncherMap_.erase(it);
            continue;
        }
        it++;
    }

    return ERR_OK;
}

int32_t AppRunningManager::RemoveUIExtensionLauncherItemById(int32_t uiExtensionAbilityId)
{
    std::lock_guard guard(uiExtensionMapLock_);
    for (auto it = uiExtensionLauncherMap_.begin(); it != uiExtensionLauncherMap_.end();) {
        if (it->first == uiExtensionAbilityId) {
            it = uiExtensionLauncherMap_.erase(it);
            continue;
        }
        it++;
    }

    return ERR_OK;
}

int AppRunningManager::DumpIpcAllStart(std::string& result)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    int errCode = DumpErrorCode::ERR_OK;
    for (const auto &item : GetAppRunningRecordMap()) {
        const auto &appRecord = item.second;
        TAG_LOGD(AAFwkTag::APPMGR, "AppRunningManager::DumpIpcAllStart::pid:%{public}d",
            appRecord->GetPid());
        std::string currentResult;
        errCode = appRecord->DumpIpcStart(currentResult);
        result += currentResult + "\n";
        if (errCode != DumpErrorCode::ERR_OK) {
            return errCode;
        }
    }
    return errCode;
}

int AppRunningManager::DumpIpcAllStop(std::string& result)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    int errCode = DumpErrorCode::ERR_OK;
    for (const auto &item : GetAppRunningRecordMap()) {
        const auto &appRecord = item.second;
        TAG_LOGD(AAFwkTag::APPMGR, "AppRunningManager::DumpIpcAllStop::pid:%{public}d",
            appRecord->GetPid());
        std::string currentResult;
        errCode = appRecord->DumpIpcStop(currentResult);
        result += currentResult + "\n";
        if (errCode != DumpErrorCode::ERR_OK) {
            return errCode;
        }
    }
    return errCode;
}

int AppRunningManager::DumpIpcAllStat(std::string& result)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    int errCode = DumpErrorCode::ERR_OK;
    for (const auto &item : GetAppRunningRecordMap()) {
        const auto &appRecord = item.second;
        TAG_LOGD(AAFwkTag::APPMGR, "AppRunningManager::DumpIpcAllStat::pid:%{public}d",
            appRecord->GetPid());
        std::string currentResult;
        errCode = appRecord->DumpIpcStat(currentResult);
        result += currentResult + "\n";
        if (errCode != DumpErrorCode::ERR_OK) {
            return errCode;
        }
    }
    return errCode;
}

int AppRunningManager::DumpIpcStart(const int32_t pid, std::string& result)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    const auto& appRecord = GetAppRunningRecordByPid(pid);
    if (!appRecord) {
        result.append(MSG_DUMP_IPC_START_STAT, strlen(MSG_DUMP_IPC_START_STAT))
            .append(MSG_DUMP_FAIL, strlen(MSG_DUMP_FAIL))
            .append(MSG_DUMP_FAIL_REASON_INVALILD_PID, strlen(MSG_DUMP_FAIL_REASON_INVALILD_PID));
        TAG_LOGE(AAFwkTag::APPMGR, "pid %{public}d does not exist", pid);
        return DumpErrorCode::ERR_INVALID_PID_ERROR;
    }
    return appRecord->DumpIpcStart(result);
}

int AppRunningManager::DumpIpcStop(const int32_t pid, std::string& result)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    const auto& appRecord = GetAppRunningRecordByPid(pid);
    if (!appRecord) {
        result.append(MSG_DUMP_IPC_STOP_STAT, strlen(MSG_DUMP_IPC_STOP_STAT))
            .append(MSG_DUMP_FAIL, strlen(MSG_DUMP_FAIL))
            .append(MSG_DUMP_FAIL_REASON_INVALILD_PID, strlen(MSG_DUMP_FAIL_REASON_INVALILD_PID));
        TAG_LOGE(AAFwkTag::APPMGR, "pid %{public}d does not exist", pid);
        return DumpErrorCode::ERR_INVALID_PID_ERROR;
    }
    return appRecord->DumpIpcStop(result);
}

int AppRunningManager::DumpIpcStat(const int32_t pid, std::string& result)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    const auto& appRecord = GetAppRunningRecordByPid(pid);
    if (!appRecord) {
        result.append(MSG_DUMP_IPC_STAT, strlen(MSG_DUMP_IPC_STAT))
            .append(MSG_DUMP_FAIL, strlen(MSG_DUMP_FAIL))
            .append(MSG_DUMP_FAIL_REASON_INVALILD_PID, strlen(MSG_DUMP_FAIL_REASON_INVALILD_PID));
        TAG_LOGE(AAFwkTag::APPMGR, "pid %{public}d does not exist", pid);
        return DumpErrorCode::ERR_INVALID_PID_ERROR;
    }
    return appRecord->DumpIpcStat(result);
}

int AppRunningManager::DumpFfrt(const std::vector<int32_t>& pids, std::string& result)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    int errCode = DumpErrorCode::ERR_OK;
    size_t count = 0;
    for (const auto& pid : pids) {
        TAG_LOGD(AAFwkTag::APPMGR, "DumpFfrt current pid:%{public}d", pid);
        const auto& appRecord = GetAppRunningRecordByPid(pid);
        if (!appRecord) {
            TAG_LOGE(AAFwkTag::APPMGR, "pid %{public}d does not exist", pid);
            ++count;
            continue;
        }
        std::string currentResult;
        errCode = appRecord->DumpFfrt(currentResult);
        if (errCode != DumpErrorCode::ERR_OK) {
            continue;
        }
        result += currentResult + "\n";
    }
    if (count == pids.size()) {
        TAG_LOGE(AAFwkTag::APPMGR, "no valid pid");
        return DumpErrorCode::ERR_INVALID_PID_ERROR;
    }
    if (result.empty()) {
        TAG_LOGE(AAFwkTag::APPMGR, "ffrt is empty");
        return DumpErrorCode::ERR_INTERNAL_ERROR;
    }
    return DumpErrorCode::ERR_OK;
}

int32_t AppRunningManager::DumpArkWeb(const std::vector<int32_t> &pids, const std::string &customArgs,
    std::string &result)
{
    TAG_LOGD(AAFwkTag::APPMGR, "DumpArkWeb called");
    bool hasValidPid = false;
    for (const auto &pid : pids) {
        TAG_LOGD(AAFwkTag::APPMGR, "DumpArkWeb current pid:%{public}d", pid);
        auto appRecord = GetAppRunningRecordByPid(pid);
        if (!appRecord) {
            TAG_LOGE(AAFwkTag::APPMGR, "pid %{public}d does not exist", pid);
            continue;
        }
        hasValidPid = true;
        std::string currentResult;
        auto errCode = appRecord->DumpArkWeb(customArgs, currentResult);
        if (errCode != DumpErrorCode::ERR_OK) {
            continue;
        }
        result += currentResult + "\n";
    }
    if (!hasValidPid) {
        TAG_LOGE(AAFwkTag::APPMGR, "no valid pid");
        return DumpErrorCode::ERR_INVALID_PID_ERROR;
    }
    if (result.empty()) {
        TAG_LOGE(AAFwkTag::APPMGR, "arkWeb empty");
        return DumpErrorCode::ERR_INTERNAL_ERROR;
    }
    return DumpErrorCode::ERR_OK;
}

bool AppRunningManager::HandleUserRequestClean(const sptr<IRemoteObject> &abilityToken, pid_t &pid, int32_t &uid)
{
    if (abilityToken == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null abilityToken");
        return false;
    }

    auto appRecord = GetAppRunningRecordByAbilityToken(abilityToken);
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "null appRecord");
        return false;
    }
    if ((appRecord->GetSupportProcessCacheState() == SupportProcessCacheState::SUPPORT) &&
        (appRecord->GetEnableProcessCache())) {
        TAG_LOGI(AAFwkTag::APPMGR, "support porcess cache should not force clean");
        return false;
    }
    auto abilityRecord = appRecord->GetAbilityRunningRecordByToken(abilityToken);
    if (!abilityRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "null abilityRecord");
        return false;
    }
    abilityRecord->SetUserRequestCleaningStatus();

    bool canKill = appRecord->IsAllAbilityReadyToCleanedByUserRequest();
    if (!canKill || appRecord->IsKeepAliveApp()) {
        return false;
    }

    appRecord->SetUserRequestCleaning();
    if (appRecord->GetPriorityObject()) {
        pid = appRecord->GetPid();
    }
    uid = appRecord->GetUid();
    return true;
}

bool AppRunningManager::IsAppProcessesAllCached(const std::string &bundleName, int32_t uid,
    const std::set<std::shared_ptr<AppRunningRecord>> &cachedSet)
{
    if (cachedSet.size() == 0) {
        TAG_LOGI(AAFwkTag::APPMGR, "empty cache set");
        return false;
    }
    std::lock_guard guard(runningRecordMapMutex_);
    for (const auto &item : appRunningRecordMap_) {
        auto &itemRecord = item.second;
        if (itemRecord == nullptr) {
            continue;
        }
        if (itemRecord->GetBundleName() == bundleName && itemRecord->GetUid() == uid) {
            auto supportCache =
                DelayedSingleton<CacheProcessManager>::GetInstance()->IsAppSupportProcessCache(itemRecord);
            // need wait for unsupported processes
            if ((cachedSet.find(itemRecord) == cachedSet.end() && supportCache) || !supportCache) {
                return false;
            }
        }
    }
    return true;
}

int32_t AppRunningManager::UpdateConfigurationDelayed(const std::shared_ptr<AppRunningRecord>& appRecord)
{
    std::lock_guard guard(updateConfigurationDelayedLock_);
    int32_t result = ERR_OK;
    auto it = updateConfigurationDelayedMap_.find(appRecord->GetRecordId());
    if (it != updateConfigurationDelayedMap_.end() && it->second) {
        auto delayConfig = appRecord->GetDelayConfiguration();
        if (delayConfig == nullptr) {
            appRecord->ResetDelayConfiguration();
        }
        TAG_LOGI(AAFwkTag::APPKIT, "delayConfig: %{public}s", delayConfig->GetName().c_str());
        result = appRecord->UpdateConfiguration(*delayConfig);
        appRecord->ResetDelayConfiguration();
        it->second = false;
    }
    return result;
}

int32_t AppRunningManager::CheckIsKiaProcess(pid_t pid, bool &isKia)
{
    auto appRunningRecord = GetAppRunningRecordByPid(pid);
    if (appRunningRecord == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRunningRecord is nullptr");
        return ERR_INVALID_VALUE;
    }
    isKia = appRunningRecord->GetIsKia();
    return ERR_OK;
}

bool AppRunningManager::CheckAppRunningRecordIsLast(const std::shared_ptr<AppRunningRecord> &appRecord)
{
    if (appRecord == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRecord null");
        return false;
    }
    std::lock_guard guard(runningRecordMapMutex_);
    if (appRunningRecordMap_.empty()) {
        return true;
    }
    auto uid = appRecord->GetUid();
    auto appRecordId = appRecord->GetRecordId();

    for (const auto &item : appRunningRecordMap_) {
        const auto &itemAppRecord = item.second;
        if (itemAppRecord != nullptr &&
            itemAppRecord->GetRecordId() != appRecordId &&
            itemAppRecord->GetUid() == uid &&
            !(appRecord->GetRestartAppFlag())) {
            return false;
        }
    }
    return true;
}

int32_t AppRunningManager::AddUIExtensionBindItem(
    int32_t uiExtensionBindAbilityId, UIExtensionProcessBindInfo &bindInfo)
{
    std::lock_guard guard(uiExtensionBindMapLock_);
    uiExtensionBindMap_.emplace(uiExtensionBindAbilityId, bindInfo);
    return ERR_OK;
}

int32_t AppRunningManager::RemoveUIExtensionBindItemById(int32_t uiExtensionBindAbilityId)
{
    std::lock_guard guard(uiExtensionBindMapLock_);
    auto it = uiExtensionBindMap_.find(uiExtensionBindAbilityId);
    if (it != uiExtensionBindMap_.end()) {
        uiExtensionBindMap_.erase(it);
    }
    return ERR_OK;
}

int32_t AppRunningManager::QueryUIExtensionBindItemById(
    int32_t uiExtensionBindAbilityId, UIExtensionProcessBindInfo &bindInfo)
{
    std::lock_guard guard(uiExtensionBindMapLock_);
    auto it = uiExtensionBindMap_.find(uiExtensionBindAbilityId);
    if (it != uiExtensionBindMap_.end()) {
        bindInfo = it->second;
        return ERR_OK;
    }
    return ERR_INVALID_VALUE;
}
}  // namespace AppExecFwk
}  // namespace OHOS
