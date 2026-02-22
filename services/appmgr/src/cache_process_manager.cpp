/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#include <vector>
#include <sstream>
#include "hitrace_meter.h"
#include "parameters.h"
#include "hilog_tag_wrapper.h"
#include "app_state_observer_manager.h"
#include "app_mgr_service_inner.h"
#include "app_utils.h"
#include "cache_process_manager.h"
#include "hisysevent_report.h"
#include "record_cost_time_util.h"
#include "res_sched_util.h"
#include "ui_extension_wrapper.h"

namespace {
const std::string MAX_PROC_CACHE_NUM = "persist.sys.abilityms.maxProcessCacheNum";
const std::string RESOURCE_WARM_START_PROCESS_ENABLE = "persist.resourceschedule.enable_warm_start_process";
const std::string MAX_ALLOWED_CACHE_NUM = "const.resourceschedule.max_cached_process_nums";
const std::string PROCESS_CACHE_API_CHECK_CONFIG = "persist.sys.abilityms.processCacheApiCheck";
const std::string PROCESS_CACHE_SET_SUPPORT_CHECK_CONFIG = "persist.sys.abilityms.processCacheSetSupportCheck";
constexpr int32_t API12 = 12;
constexpr int32_t API_VERSION_MOD = 100;
constexpr int32_t DEFAULT_ALLOWED_CACHE_NUM = 64;
constexpr const char *EVENT_KEY_VERSION_NAME = "VERSION_NAME";
constexpr const char *EVENT_KEY_VERSION_CODE = "VERSION_CODE";
constexpr const char *EVENT_KEY_BUNDLE_NAME = "BUNDLE_NAME";
constexpr const char *EVENT_KEY_CACHE_STATE = "CACHE_STATE";
}

namespace OHOS {
namespace AppExecFwk {

CacheProcessManager::CacheProcessManager()
{
    maxProcCacheNum_ = OHOS::system::GetIntParameter<int>(MAX_PROC_CACHE_NUM, 0);
    shouldCheckApi = OHOS::system::GetBoolParameter(PROCESS_CACHE_API_CHECK_CONFIG, true);
    shouldCheckSupport = OHOS::system::GetBoolParameter(PROCESS_CACHE_SET_SUPPORT_CHECK_CONFIG, true);
    warmStartProcesEnable_ = OHOS::system::GetBoolParameter(RESOURCE_WARM_START_PROCESS_ENABLE, false);
    allowedCacheNum_ = OHOS::system::GetIntParameter<int>(MAX_ALLOWED_CACHE_NUM, DEFAULT_ALLOWED_CACHE_NUM);
    if (maxProcCacheNum_ > 0) {
        allowedCacheNum_ = maxProcCacheNum_;
    }
    TAG_LOGW(AAFwkTag::APPMGR,
        "maxProcCacheNum_ %{public}d, allowedCacheNum_ %{public}d", maxProcCacheNum_, allowedCacheNum_);
}

CacheProcessManager::~CacheProcessManager()
{
}

void CacheProcessManager::SetAppMgr(const std::weak_ptr<AppMgrServiceInner> &appMgr)
{
    TAG_LOGD(AAFwkTag::APPMGR, "Called");
    appMgr_ = appMgr;
}

void CacheProcessManager::RefreshCacheNum()
{
    maxProcCacheNum_ = OHOS::system::GetIntParameter<int>(MAX_PROC_CACHE_NUM, 0);
    allowedCacheNum_ = maxProcCacheNum_;
    TAG_LOGW(AAFwkTag::APPMGR, "maxProcCacheNum %{public}d", maxProcCacheNum_);
}

bool CacheProcessManager::QueryEnableProcessCache()
{
    return maxProcCacheNum_ > 0 || warmStartProcesEnable_;
}

bool CacheProcessManager::PenddingCacheProcess(const std::shared_ptr<AppRunningRecord> &appRecord)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::APPMGR, "Called");
    if (!QueryEnableProcessCache()) {
        return false;
    }
    if (appRecord == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "precheck failed");
        return false;
    }
    if (IsCachedProcess(appRecord)) {
        return false;
    }
    if (appRecord->IsKeepAliveApp()) {
        TAG_LOGW(AAFwkTag::APPMGR, "Not cache process");
        return false;
    }
    {
        std::lock_guard<ffrt::recursive_mutex> queueLock(cacheQueueMtx);
        cachedAppRecordQueue_.push_back(appRecord);
        AddToApplicationSet(appRecord);
        if (warmStartProcesEnable_) {
            appRecord->SetProcessCaching(true);
        }
    }
    ShrinkAndKillCache();
    TAG_LOGI(AAFwkTag::APPMGR, "Pending %{public}s success, %{public}s", appRecord->GetName().c_str(),
        PrintCacheQueue().c_str());
    return true;
}

bool CacheProcessManager::CheckAndCacheProcess(const std::shared_ptr<AppRunningRecord> &appRecord)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::APPMGR, "Called");
    if (!QueryEnableProcessCache()) {
        return false;
    }
    if (appRecord == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "precheck failed");
        return false;
    }
    appRecord->SetProcessCaching(false);
    if (!IsCachedProcess(appRecord)) {
        return false;
    }
    if (!IsAppAbilitiesEmpty(appRecord)) {
        TAG_LOGD(AAFwkTag::APPMGR, "%{public}s not cache for abilities not empty",
            appRecord->GetName().c_str());
        return true;
    }
    if (!warmStartProcesEnable_) {
        appRecord->ScheduleCacheProcess();
    }
    auto appInfo = appRecord->GetApplicationInfo();
    std::string eventState = "processEnterCache";
    auto hisyseventReport = std::make_shared<AAFwk::HisyseventReport>(4);
    hisyseventReport->InsertParam(EVENT_KEY_VERSION_CODE, appInfo->versionCode);
    hisyseventReport->InsertParam(EVENT_KEY_VERSION_NAME, appInfo->versionName);
    hisyseventReport->InsertParam(EVENT_KEY_BUNDLE_NAME, appInfo->bundleName);
    hisyseventReport->InsertParam(EVENT_KEY_CACHE_STATE, eventState);
    hisyseventReport->Report("AAFWK", "CACHE_START_APP", HISYSEVENT_BEHAVIOR);
    auto notifyCached = [appRecord]() {
        DelayedSingleton<CacheProcessManager>::GetInstance()->CheckAndNotifyCachedState(appRecord);
    };
    std::string taskName = "DELAY_CACHED_STATE_NOTIFY";
    if (appRecord->GetPriorityObject()) {
        taskName += std::to_string(appRecord->GetPid());
    }
    auto res = appRecord->CancelTask(taskName);
    if (res) {
        TAG_LOGD(AAFwkTag::APPMGR, "Early delay task canceled.");
    }
    appRecord->PostTask(taskName, AMSEventHandler::DELAY_NOTIFY_PROCESS_CACHED_STATE, notifyCached);
    return true;
}

bool CacheProcessManager::CheckAndNotifyCachedState(const std::shared_ptr<AppRunningRecord> &appRecord)
{
    if (appRecord == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "precheck failed");
        return false;
    }
    auto appMgrSptr = appMgr_.lock();
    if (appMgrSptr == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null appMgr");
        return false;
    }
    auto &bundleName = appRecord->GetBundleName();
    auto uid = appRecord->GetUid();
    std::shared_ptr<AppRunningRecord> notifyRecord = nullptr;
    {
        std::lock_guard<ffrt::recursive_mutex> queueLock(cacheQueueMtx);
        if (sameAppSet.find(bundleName) == sameAppSet.end() ||
            sameAppSet[bundleName].find(uid) == sameAppSet[bundleName].end()) {
            TAG_LOGD(AAFwkTag::APPMGR, "app set not found.");
            return false;
        }
        if (sameAppSet[bundleName][uid].size() == 0) {
            return false;
        }
        if (!appMgrSptr->IsAppProcessesAllCached(bundleName, uid, sameAppSet[bundleName][uid])) {
            TAG_LOGI(AAFwkTag::APPMGR, "Not cache process");
            return false;
        }
        notifyRecord = *(sameAppSet[bundleName][uid].begin());
    }
    appRecord->SetProcessCaching(false);
    appMgrSptr->OnAppCacheStateChanged(notifyRecord, ApplicationState::APP_STATE_CACHED);
    TAG_LOGI(AAFwkTag::APPMGR, "notified: %{public}s, uid:%{public}d", bundleName.c_str(), uid);
    return true;
}

bool CacheProcessManager::IsCachedProcess(const std::shared_ptr<AppRunningRecord> &appRecord)
{
    if (appRecord == nullptr) {
        TAG_LOGI(AAFwkTag::APPMGR, "precheck failed");
        return false;
    }
    std::lock_guard<ffrt::recursive_mutex> queueLock(cacheQueueMtx);
    for (auto& tmpAppRecord : cachedAppRecordQueue_) {
        if (tmpAppRecord == appRecord) {
            return true;
        }
    }
    return false;
}

void CacheProcessManager::OnProcessKilled(const std::shared_ptr<AppRunningRecord> &appRecord)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    AAFwk::RecordCostTimeUtil timeRecord("OnProcessKilled");
    if (!QueryEnableProcessCache()) {
        return;
    }
    if (appRecord == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "precheck failed");
        return;
    }
    CheckAndNotifyCachedState(appRecord);
    {
        std::lock_guard<ffrt::recursive_mutex> queueLock(cacheQueueMtx);
        srvExtRecords.erase(appRecord);
        srvExtCheckedFlag.erase(appRecord);
    }
    if (!IsCachedProcess(appRecord)) {
        return;
    }
    RemoveCacheRecord(appRecord);
    auto appInfo = appRecord->GetApplicationInfo();
    auto hisyseventReport = std::make_shared<AAFwk::HisyseventReport>(4);
    std::string eventState = "destroyedByExternal";
    hisyseventReport->InsertParam(EVENT_KEY_VERSION_CODE, appInfo->versionCode);
    hisyseventReport->InsertParam(EVENT_KEY_VERSION_NAME, appInfo->versionName);
    hisyseventReport->InsertParam(EVENT_KEY_BUNDLE_NAME, appInfo->bundleName);
    hisyseventReport->InsertParam(EVENT_KEY_CACHE_STATE, eventState);
    hisyseventReport->Report("AAFWK", "CACHE_START_APP", HISYSEVENT_BEHAVIOR);
    TAG_LOGI(AAFwkTag::APPMGR, "%{public}s is killed, %{public}s", appRecord->GetName().c_str(),
        PrintCacheQueue().c_str());
}

bool CacheProcessManager::ReuseCachedProcess(const std::shared_ptr<AppRunningRecord> &appRecord)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    if (!QueryEnableProcessCache()) {
        return false;
    }
    if (appRecord == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "precheck failed");
        return false;
    }
    auto appInfo = appRecord->GetApplicationInfo();
    if (!IsCachedProcess(appRecord)) {
        return false;
    }
    RemoveCacheRecord(appRecord);
    auto hisyseventReport = std::make_shared<AAFwk::HisyseventReport>(4);
    std::string eventState = "exitCacheNormal";
    hisyseventReport->InsertParam(EVENT_KEY_VERSION_CODE, appInfo->versionCode);
    hisyseventReport->InsertParam(EVENT_KEY_VERSION_NAME, appInfo->versionName);
    hisyseventReport->InsertParam(EVENT_KEY_BUNDLE_NAME, appInfo->bundleName);
    hisyseventReport->InsertParam(EVENT_KEY_CACHE_STATE, eventState);
    hisyseventReport->Report("AAFWK", "CACHE_START_APP", HISYSEVENT_BEHAVIOR);
    std::string eventLaunchState = "processCacheLaunch";
    auto eventReport = std::make_shared<AAFwk::HisyseventReport>(4);
    eventReport->InsertParam(EVENT_KEY_VERSION_CODE, appInfo->versionCode);
    eventReport->InsertParam(EVENT_KEY_VERSION_NAME, appInfo->versionName);
    eventReport->InsertParam(EVENT_KEY_BUNDLE_NAME, appInfo->bundleName);
    eventReport->InsertParam(EVENT_KEY_CACHE_STATE, eventLaunchState);
    eventReport->Report("AAFWK", "CACHE_START_APP", HISYSEVENT_BEHAVIOR);
    auto appMgrSptr = appMgr_.lock();
    if (appMgrSptr == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null appMgr");
        return true;
    }
    appMgrSptr->OnAppCacheStateChanged(appRecord, ApplicationState::APP_STATE_READY);
    TAG_LOGI(AAFwkTag::APPMGR, "app none cached state is notified: %{public}s, uid: %{public}d, %{public}s",
        appRecord->GetBundleName().c_str(), appRecord->GetUid(), PrintCacheQueue().c_str());
    return true;
}

bool CacheProcessManager::IsProcessSupportHotStart(const std::shared_ptr<AppRunningRecord> &appRecord)
{
    if (appRecord == nullptr) {
        return false;
    }
    auto appInfo = appRecord->GetApplicationInfo();
    if (appInfo == nullptr) {
        TAG_LOGD(AAFwkTag::APPMGR, "appinfo nullptr");
        return false;
    }
    auto actualVer = appInfo->apiTargetVersion % API_VERSION_MOD;
    if (shouldCheckApi && actualVer < API12) {
        TAG_LOGD(AAFwkTag::APPMGR, "App %{public}s 's apiTargetVersion has %{public}d, smaller than 12",
            appRecord->GetName().c_str(), actualVer);
        return false;
    }
    if (!appRecord->HasUIAbilityLaunched() &&
        !AAFwk::UIExtensionWrapper::IsUIExtension(appRecord->GetExtensionType())) {
        TAG_LOGD(AAFwkTag::APPMGR, "%{public}s of %{public}s has not created uiability before.",
            appRecord->GetProcessName().c_str(), appRecord->GetBundleName().c_str());
        return false;
    }
    return true;
}

void CacheProcessManager::CheckAndSetProcessCacheEnable(const std::shared_ptr<AppRunningRecord> &appRecord)
{
    if (appRecord == nullptr || !warmStartProcesEnable_) {
        return;
    }
    if (appRecord->GetSupportProcessCacheState() != SupportProcessCacheState::SUPPORT) {
        return;
    }
    if (!appRecord->GetEnableProcessCache()) {
        return;
    }
    if (!appRecord->GetPriorityObject()) {
        return;
    }
    if (appRecord->GetProcessCacheBlocked()) {
        return;
    }
    bool forceKillProcess =
        AAFwk::ResSchedUtil::GetInstance().CheckShouldForceKillProcess(
            appRecord->GetPid(), appRecord->GetBundleName());
    if (forceKillProcess) {
        appRecord->SetProcessCacheBlocked(true);
        return;
    }
}

bool CacheProcessManager::IsAppSupportProcessCache(const std::shared_ptr<AppRunningRecord> &appRecord)
{
    if (appRecord == nullptr) {
        TAG_LOGI(AAFwkTag::APPMGR, "precheck failed");
        return false;
    }
    if (appRecord->IsAttachedToStatusBar()) {
        TAG_LOGD(AAFwkTag::APPMGR, "%{public}s of %{public}s is attached to statusbar, not support cache",
            appRecord->GetProcessName().c_str(), appRecord->GetBundleName().c_str());
        return false;
    }
    if (appRecord->IsKeepAliveApp()) {
        TAG_LOGD(AAFwkTag::APPMGR, "Keepalive app.");
        return false;
    }
    if (appRecord->GetParentAppRecord() != nullptr) {
        TAG_LOGD(AAFwkTag::APPMGR, "Child App, not support.");
        return false;
    }
    if (maxProcCacheNum_ > 0 && !IsProcessSupportHotStart(appRecord)) {
        return false;
    }
    return IsAppSupportProcessCacheInnerFirst(appRecord);
}

bool CacheProcessManager::IsAppSupportProcessCacheInnerFirst(const std::shared_ptr<AppRunningRecord> &appRecord)
{
    if (appRecord == nullptr) {
        TAG_LOGI(AAFwkTag::APPMGR, "precheck failed");
        return false;
    }
    if (appRecord->GetBundleName() == AAFwk::AppUtils::GetInstance().GetBrokerDelegateBundleName()) {
        TAG_LOGD(AAFwkTag::APPMGR, "shell assistant, not support.");
        return false;
    }
    if (appRecord->GetProcessCacheBlocked()) {
        TAG_LOGD(AAFwkTag::APPMGR, "%{public}s of %{public}s 's process cache temporarily blocked.",
            appRecord->GetProcessName().c_str(), appRecord->GetBundleName().c_str());
        return false;
    }
    if (warmStartProcesEnable_) {
        if (!appRecord->HasUIAbilityLaunched() &&
            !AAFwk::UIExtensionWrapper::IsUIExtension(appRecord->GetExtensionType())) {
            return false;
        }
    }

    return CheckSupportState(appRecord);
}

bool CacheProcessManager::CheckSupportState(const std::shared_ptr<AppRunningRecord> &appRecord)
{
    auto supportState = appRecord->GetSupportProcessCacheState();
    switch (supportState) {
        case SupportProcessCacheState::UNSPECIFIED:
            TAG_LOGD(AAFwkTag::APPMGR, "App %{public}s has not defined support state.",
                appRecord->GetBundleName().c_str());
            return shouldCheckSupport ? false : true;
        case SupportProcessCacheState::SUPPORT:
            return true;
        case SupportProcessCacheState::NOT_SUPPORT:
            TAG_LOGD(AAFwkTag::APPMGR, "App %{public}s defines not support.",
                appRecord->GetBundleName().c_str());
            return false;
        default:
            TAG_LOGD(AAFwkTag::APPMGR, "Invalid support state.");
            return false;
    }
}

bool CacheProcessManager::IsAppShouldCache(const std::shared_ptr<AppRunningRecord> &appRecord)
{
    if (appRecord == nullptr) {
        return false;
    }
    if (appRecord->IsTerminating()) {
        return false;
    }
    if (!QueryEnableProcessCache()) {
        return false;
    }
    if (!CheckSupportState(appRecord)) {
        TAG_LOGD(AAFwkTag::APPMGR, "App %{public}s defines not support.",
            appRecord->GetBundleName().c_str());
        return false;
    }
    if (IsCachedProcess(appRecord) && !appRecord->GetProcessCacheBlocked()) {
        return true;
    }
    if (!IsAppSupportProcessCache(appRecord)) {
        return false;
    }
    if (!appRecord->GetEnableProcessCache()) {
        return false;
    }
    return true;
}

bool CacheProcessManager::IsAppAbilitiesEmpty(const std::shared_ptr<AppRunningRecord> &appRecord)
{
    if (appRecord == nullptr) {
        TAG_LOGI(AAFwkTag::APPMGR, "precheck failed");
        return false;
    }
    auto allModuleRecord = appRecord->GetAllModuleRecord();
    for (auto moduleRecord : allModuleRecord) {
        if (moduleRecord != nullptr && !moduleRecord->GetAbilities().empty()) {
            return false;
        }
    }
    TAG_LOGD(AAFwkTag::APPMGR, "abilities all empty: %{public}s",
        appRecord->GetName().c_str());
    return true;
}

int CacheProcessManager::GetCurrentCachedProcNum()
{
    std::lock_guard<ffrt::recursive_mutex> queueLock(cacheQueueMtx);
    return static_cast<int>(cachedAppRecordQueue_.size());
}

void CacheProcessManager::RemoveCacheRecord(const std::shared_ptr<AppRunningRecord> &appRecord)
{
    std::lock_guard<ffrt::recursive_mutex> queueLock(cacheQueueMtx);
    for (auto it = cachedAppRecordQueue_.begin(); it != cachedAppRecordQueue_.end();) {
        if (appRecord == *it) {
            RemoveFromApplicationSet(*it);
            it = cachedAppRecordQueue_.erase(it);
        } else {
            it++;
        }
    }
}

void CacheProcessManager::ShrinkAndKillCache()
{
    TAG_LOGD(AAFwkTag::APPMGR, "Called");
    if (maxProcCacheNum_ <= 0 && !warmStartProcesEnable_) {
        TAG_LOGI(AAFwkTag::APPMGR, "Cache disabled.");
        return;
    }
    std::vector<std::shared_ptr<AppRunningRecord>> cleanList;
    {
        std::lock_guard<ffrt::recursive_mutex> queueLock(cacheQueueMtx);
        while (GetCurrentCachedProcNum() > allowedCacheNum_) {
            const auto& tmpAppRecord = cachedAppRecordQueue_.front();
            cachedAppRecordQueue_.pop_front();
            RemoveFromApplicationSet(tmpAppRecord);
            if (tmpAppRecord == nullptr) {
                continue;
            }
            cleanList.push_back(tmpAppRecord);
            TAG_LOGI(AAFwkTag::APPMGR, "need clean record %{public}s, current =%{public}d",
                tmpAppRecord->GetName().c_str(), GetCurrentCachedProcNum());
        }
    }
    for (auto& tmpAppRecord : cleanList) {
        auto appInfo = tmpAppRecord->GetApplicationInfo();
        std::string eventState = "killForOverload";
        auto hisyseventReport = std::make_shared<AAFwk::HisyseventReport>(4);
        hisyseventReport->InsertParam(EVENT_KEY_VERSION_CODE, appInfo->versionCode);
        hisyseventReport->InsertParam(EVENT_KEY_VERSION_NAME, appInfo->versionName);
        hisyseventReport->InsertParam(EVENT_KEY_BUNDLE_NAME, appInfo->bundleName);
        hisyseventReport->InsertParam(EVENT_KEY_CACHE_STATE, eventState);
        hisyseventReport->Report("AAFWK", "CACHE_START_APP", HISYSEVENT_BEHAVIOR);
        KillProcessByRecord(tmpAppRecord);
    }
}

bool CacheProcessManager::KillProcessByRecord(const std::shared_ptr<AppRunningRecord> &appRecord)
{
    if (appRecord == nullptr) {
        TAG_LOGW(AAFwkTag::APPMGR, "precheck failed");
        return false;
    }
    auto appMgrSptr = appMgr_.lock();
    if (appMgrSptr == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null appMgr");
        return false;
    }
    appRecord->SetProcessCaching(false);
    // notify before kill
    appMgrSptr->OnAppCacheStateChanged(appRecord, ApplicationState::APP_STATE_READY);
    // this uses ScheduleProcessSecurityExit
    appMgrSptr->KillApplicationByRecord(appRecord);
    return true;
}

std::string CacheProcessManager::PrintCacheQueue()
{
    std::lock_guard<ffrt::recursive_mutex> queueLock(cacheQueueMtx);
    std::stringstream ss;
    ss << "queue size: " << cachedAppRecordQueue_.size() << ", record in queue: ";
    for (auto& record : cachedAppRecordQueue_) {
        if (record == nullptr) {
            ss << "null, ";
        } else {
            ss << record->GetName() << ", ";
        }
    }
    ss << ".";
    return ss.str();
}

void CacheProcessManager::AddToApplicationSet(const std::shared_ptr<AppRunningRecord> &appRecord)
{
    if (appRecord == nullptr) {
        return;
    }
    auto &bundleName = appRecord->GetBundleName();
    std::lock_guard<ffrt::recursive_mutex> queueLock(cacheQueueMtx);
    if (sameAppSet.find(bundleName) == sameAppSet.end()) {
        std::map<int32_t, std::set<std::shared_ptr<AppRunningRecord>>> uidMap;
        std::set<std::shared_ptr<AppRunningRecord>> recordSet;
        recordSet.insert(appRecord);
        uidMap.insert(std::make_pair(appRecord->GetUid(), recordSet));
        sameAppSet.insert(std::make_pair(bundleName, uidMap));
    }
    auto uid = appRecord->GetUid();
    if (sameAppSet[bundleName].find(uid) == sameAppSet[bundleName].end()) {
        std::set<std::shared_ptr<AppRunningRecord>> recordSet;
        recordSet.insert(appRecord);
        sameAppSet[bundleName].insert(std::make_pair(uid, recordSet));
        return;
    }
    sameAppSet[bundleName][uid].insert(appRecord);
}

void CacheProcessManager::RemoveFromApplicationSet(const std::shared_ptr<AppRunningRecord> &appRecord)
{
    if (appRecord == nullptr) {
        return;
    }
    auto &bundleName = appRecord->GetBundleName();
    std::lock_guard<ffrt::recursive_mutex> queueLock(cacheQueueMtx);
    if (sameAppSet.find(bundleName) == sameAppSet.end()) {
        return;
    }
    auto uid = appRecord->GetUid();
    if (sameAppSet[bundleName].find(uid) == sameAppSet[bundleName].end()) {
        return;
    }
    sameAppSet[bundleName][uid].erase(appRecord);
    if (sameAppSet[bundleName][uid].size() == 0) {
        sameAppSet[bundleName].erase(uid);
    }
    if (sameAppSet[bundleName].size() == 0) {
        sameAppSet.erase(bundleName);
    }
}

void CacheProcessManager::PrepareActivateCache(const std::shared_ptr<AppRunningRecord> &appRecord)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    if (!QueryEnableProcessCache()) {
        return;
    }
    if (appRecord == nullptr) {
        return;
    }
    if (!IsCachedProcess(appRecord)) {
        return;
    }
    TAG_LOGD(AAFwkTag::APPMGR, "%{public}s needs activate.", appRecord->GetBundleName().c_str());
    auto appMgrSptr = appMgr_.lock();
    if (appMgrSptr == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null appMgr");
        return;
    }
    appMgrSptr->OnAppCacheStateChanged(appRecord, ApplicationState::APP_STATE_READY);
}

bool CacheProcessManager::IsAppContainsSrvExt(const std::shared_ptr<AppRunningRecord> &appRecord)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    std::lock_guard<ffrt::recursive_mutex> queueLock(cacheQueueMtx);
    if (appRecord == nullptr) {
        return false;
    }
    if (srvExtCheckedFlag.find(appRecord) != srvExtCheckedFlag.end()) {
        return srvExtRecords.find(appRecord) != srvExtRecords.end() ? true : false;
    }
    auto allModuleRecord = appRecord->GetAllModuleRecord();
    for (auto moduleRecord : allModuleRecord) {
        if (moduleRecord == nullptr) {
            continue;
        }
        HapModuleInfo hapModuleInfo;
        moduleRecord->GetHapModuleInfo(hapModuleInfo);
        for (auto abilityInfo : hapModuleInfo.abilityInfos) {
            if (abilityInfo.type == AppExecFwk::AbilityType::EXTENSION &&
                abilityInfo.extensionAbilityType == AppExecFwk::ExtensionAbilityType::SERVICE) {
                    srvExtRecords.insert(appRecord);
                TAG_LOGD(AAFwkTag::APPMGR, "%{public}s of %{public}s is service, will not cache",
                    abilityInfo.name.c_str(), appRecord->GetBundleName().c_str());
            }
        }
        for (auto extAbilityInfo : hapModuleInfo.extensionInfos) {
            if (extAbilityInfo.type == AppExecFwk::ExtensionAbilityType::SERVICE) {
                srvExtRecords.insert(appRecord);
                TAG_LOGD(AAFwkTag::APPMGR, "%{public}s of %{public}s is service, will not cache",
                    extAbilityInfo.name.c_str(), appRecord->GetBundleName().c_str());
            }
        }
    }
    srvExtCheckedFlag.insert(appRecord);
    return srvExtRecords.find(appRecord) != srvExtRecords.end() ? true : false;
}

void CacheProcessManager::OnAppProcessCacheBlocked(const std::shared_ptr<AppRunningRecord> &appRecord)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    if (!QueryEnableProcessCache()) {
        return;
    }
    if (appRecord == nullptr || !IsCachedProcess(appRecord)) {
        return;
    }
    TAG_LOGI(AAFwkTag::APPMGR, "%{public}s is cached and is blocked, which needs exit.",
        appRecord->GetBundleName().c_str());
    RemoveCacheRecord(appRecord);
    KillProcessByRecord(appRecord);
}
} // namespace OHOS
} // namespace AppExecFwk