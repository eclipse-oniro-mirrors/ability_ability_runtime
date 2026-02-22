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

#include <limits>
#include <optional>
#include <random>

#include "ability_window_configuration.h"
#include "app_running_record.h"
#include "app_utils.h"
#include "render_record.h"
#include "app_mgr_service_inner.h"
#include "error_msg_util.h"
#include "event_report.h"
#include "exit_resident_process_manager.h"
#include "freeze_util.h"
#include "hitrace_meter.h"
#include "hilog_tag_wrapper.h"
#include "insight_intent_execute_param.h"
#include "ui_extension_wrapper.h"
#include "app_mgr_service_const.h"
#include "app_mgr_service_dump_error_code.h"
#include "cache_process_manager.h"
#include "hisysevent_report.h"
#ifdef SUPPORT_SCREEN
#include "window_visibility_info.h"
#endif //SUPPORT_SCREEN
#ifdef SUPPORT_UPMS
#include "uri_permission_manager_client.h"
#endif // SUPPORT_UPMS
namespace OHOS {
using AAFwk::AppUtils;
namespace AppExecFwk {
namespace {
constexpr int64_t NANOSECONDS = 1000000000;  // NANOSECONDS mean 10^9 nano second
constexpr int64_t MICROSECONDS = 1000000;    // MICROSECONDS mean 10^6 millias second
constexpr int32_t MAX_RESTART_COUNT = 3;
constexpr int32_t RESTART_INTERVAL_TIME = 120000;
constexpr int32_t HALF_TIMEOUT = 2;
constexpr const char* LAUNCHER_NAME = "com.ohos.sceneboard";
constexpr const char *EVENT_KEY_VERSION_NAME = "VERSION_NAME";
constexpr const char *EVENT_KEY_VERSION_CODE = "VERSION_CODE";
constexpr const char *EVENT_KEY_BUNDLE_NAME = "BUNDLE_NAME";
constexpr const char *EVENT_KEY_SUPPORT_STATE = "SUPPORT_STATE";
constexpr uint32_t PROCESS_MODE_RUN_WITH_MAIN_PROCESS =
    1 << static_cast<uint32_t>(AppExecFwk::ExtensionProcessMode::RUN_WITH_MAIN_PROCESS);
}

int64_t AppRunningRecord::appEventId_ = 0;


AppRunningRecord::AppRunningRecord(
    const std::shared_ptr<ApplicationInfo> &info, const int32_t recordId, const std::string &processName)
    : appRecordId_(recordId), processName_(processName)
{
    if (info) {
        appInfo_ = info;
        mainBundleName_ = info->bundleName;
        isLauncherApp_ = info->isLauncherApp;
        mainAppName_ = info->name;
    }
    priorityObject_ = std::make_shared<PriorityObject>();

    struct timespec t;
    t.tv_sec = 0;
    t.tv_nsec = 0;
    clock_gettime(CLOCK_MONOTONIC, &t);
    startTimeMillis_ = static_cast<int64_t>(((t.tv_sec) * NANOSECONDS + t.tv_nsec) / MICROSECONDS);
    appRunningUniqueId_ = GenerateRunningId();
}

void AppRunningRecord::SetApplicationClient(const sptr<IAppScheduler> &thread)
{
    if (!appLifeCycleDeal_) {
        appLifeCycleDeal_ = std::make_shared<AppLifeCycleDeal>();
    }
    appLifeCycleDeal_->SetApplicationClient(thread);

    auto moduleRecordList = GetAllModuleRecord();
    if (moduleRecordList.empty()) {
        TAG_LOGD(AAFwkTag::APPMGR, "moduleRecordList is empty");
        return;
    }
    for (const auto &moduleRecord : moduleRecordList) {
        moduleRecord->SetApplicationClient(appLifeCycleDeal_);
    }
}

const std::string &AppRunningRecord::GetBundleName() const
{
    return mainBundleName_;
}

int32_t AppRunningRecord::GetCallerPid() const
{
    return callerPid_;
}

void AppRunningRecord::SetCallerPid(int32_t pid)
{
    callerPid_ = pid;
}

int32_t AppRunningRecord::GetCallerUid() const
{
    return callerUid_;
}

void AppRunningRecord::SetCallerUid(int32_t uid)
{
    callerUid_ = uid;
}

int32_t AppRunningRecord::GetCallerTokenId() const
{
    return callerTokenId_;
}

void AppRunningRecord::SetCallerTokenId(int32_t tokenId)
{
    callerTokenId_ = tokenId;
}

bool AppRunningRecord::IsLauncherApp() const
{
    return isLauncherApp_;
}

int32_t AppRunningRecord::GetRecordId() const
{
    return appRecordId_;
}

const std::string &AppRunningRecord::GetName() const
{
    return mainAppName_;
}

const std::string &AppRunningRecord::GetSignCode() const
{
    return signCode_;
}

void AppRunningRecord::SetSignCode(const std::string &signCode)
{
    signCode_ = signCode;
}

const std::string &AppRunningRecord::GetJointUserId() const
{
    return jointUserId_;
}

void AppRunningRecord::SetJointUserId(const std::string &jointUserId)
{
    jointUserId_ = jointUserId;
}

const std::string &AppRunningRecord::GetProcessName() const
{
    return processName_;
}

bool AppRunningRecord::GetExtensionSandBoxFlag() const
{
    return isExtensionSandBox_;
}

void AppRunningRecord::SetExtensionSandBoxFlag(bool extensionSandBoxFlag)
{
    isExtensionSandBox_ = extensionSandBoxFlag;
}

void AppRunningRecord::SetSpecifiedProcessFlag(const std::string &flag)
{
    specifiedProcessFlag_ = flag;
}

const std::string &AppRunningRecord::GetSpecifiedProcessFlag() const
{
    return specifiedProcessFlag_;
}
void AppRunningRecord::SetCustomProcessFlag(const std::string &flag)
{
    customProcessFlag_ = flag;
}

const std::string &AppRunningRecord::GetCustomProcessFlag() const
{
    return customProcessFlag_;
}

int32_t AppRunningRecord::GetUid() const
{
    return mainUid_;
}

void AppRunningRecord::SetUid(const int32_t uid)
{
    mainUid_ = uid;
}

void AppRunningRecord::SetPreloadAttachTimeoutStartTime(const std::chrono::system_clock::time_point &time)
{
    preloadAttachTimeoutStartTime_ = time;
}

std::chrono::system_clock::time_point AppRunningRecord::GetPreloadAttachTimeoutStartTime() const
{
    return preloadAttachTimeoutStartTime_;
}

bool AppRunningRecord::ShouldSkipTimeout() const
{
    return isDebugApp_ || isNativeDebug_ || isAttachDebug_;
}

int32_t AppRunningRecord::GetUserId() const
{
    return mainUid_ / BASE_USER_RANGE;
}

ApplicationState AppRunningRecord::GetState() const
{
    return curState_.load();
}

void AppRunningRecord::SetState(const ApplicationState state)
{
    if (state >= ApplicationState::APP_STATE_END && state != ApplicationState::APP_STATE_CACHED) {
        TAG_LOGE(AAFwkTag::APPMGR, "Invalid application state");
        return;
    }
    if (state == ApplicationState::APP_STATE_FOREGROUND || state == ApplicationState::APP_STATE_BACKGROUND) {
        restartResidentProcCount_ = MAX_RESTART_COUNT;
    }
    std::string foreTag = "ForeApp:";
    if (state == ApplicationState::APP_STATE_FOREGROUND) {
        StartAsyncTrace(HITRACE_TAG_APP, foreTag + mainBundleName_, 0);
    } else if (state == ApplicationState::APP_STATE_BACKGROUND) {
        FinishAsyncTrace(HITRACE_TAG_APP, foreTag + mainBundleName_, 0);
    }
    curState_.store(state);
}

void AppRunningRecord::SetRestartTimeMillis(const int64_t restartTimeMillis)
{
    restartTimeMillis_ = restartTimeMillis;
}

const std::list<std::shared_ptr<ApplicationInfo>> AppRunningRecord::GetAppInfoList()
{
    std::list<std::shared_ptr<ApplicationInfo>> appInfoList;
    std::lock_guard<ffrt::mutex> appInfosLock(appInfosLock_);
    for (const auto &item : appInfos_) {
        appInfoList.push_back(item.second);
    }
    return appInfoList;
}

void AppRunningRecord::SetAppIdentifier(const std::string &appIdentifier)
{
    appIdentifier_ = appIdentifier;
}

const std::string &AppRunningRecord::GetAppIdentifier() const
{
    return appIdentifier_;
}

const std::map<const sptr<IRemoteObject>, std::shared_ptr<AbilityRunningRecord>> AppRunningRecord::GetAbilities()
{
    std::map<const sptr<IRemoteObject>, std::shared_ptr<AbilityRunningRecord>> abilitiesMap;
    auto moduleRecordList = GetAllModuleRecord();
    for (const auto &moduleRecord : moduleRecordList) {
        auto abilities = moduleRecord->GetAbilities();
        abilitiesMap.insert(abilities.begin(), abilities.end());
    }
    return abilitiesMap;
}

bool AppRunningRecord::IsAlreadyHaveAbility()
{
    auto moduleRecordList = GetAllModuleRecord();
    for (const auto &moduleRecord : moduleRecordList) {
        if (!moduleRecord) {
            continue;
        }
        auto abilities = moduleRecord->GetAbilities();
        if (!abilities.empty()) {
            return true;
        }
    }
    return false;
}

sptr<IAppScheduler> AppRunningRecord::GetApplicationClient() const
{
    return (appLifeCycleDeal_ ? appLifeCycleDeal_->GetApplicationClient() : nullptr);
}

std::shared_ptr<AbilityRunningRecord> AppRunningRecord::GetAbilityRunningRecord(const int64_t eventId) const
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    auto moduleRecordList = GetAllModuleRecord();
    for (const auto &moduleRecord : moduleRecordList) {
        auto abilityRecord = moduleRecord->GetAbilityRunningRecord(eventId);
        if (abilityRecord) {
            return abilityRecord;
        }
    }

    return nullptr;
}

void AppRunningRecord::RemoveModuleRecord(
    const std::shared_ptr<ModuleRunningRecord> &moduleRecord, bool isExtensionDebug)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");

    std::lock_guard<ffrt::mutex> hapModulesLock(hapModulesLock_);
    for (auto &item : hapModules_) {
        auto iter = std::find_if(item.second.begin(),
            item.second.end(),
            [&moduleRecord](const std::shared_ptr<ModuleRunningRecord> &record) { return moduleRecord == record; });
        if (iter != item.second.end()) {
            TAG_LOGD(AAFwkTag::APPMGR, "Removed a record.");
            iter = item.second.erase(iter);
            if (item.second.empty() && !isExtensionDebug) {
                {
                    std::lock_guard<ffrt::mutex> appInfosLock(appInfosLock_);
                    TAG_LOGD(AAFwkTag::APPMGR, "Removed an appInfo.");
                    appInfos_.erase(item.first);
                }
                hapModules_.erase(item.first);
            }
            return;
        }
    }
}

void AppRunningRecord::LaunchApplication(const Configuration &config)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    if (appLifeCycleDeal_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null appLifeCycleDeal_");
        return;
    }
    auto scheduler = appLifeCycleDeal_->GetApplicationClient();
    if (!scheduler) {
        TAG_LOGE(AAFwkTag::APPMGR, "null appThread");
        AddAppLifecycleEvent("AppRunningRecord::LaunchApplication; null scheduler");
        return;
    }
    AppLaunchData launchData;
    {
        std::lock_guard<ffrt::mutex> appInfosLock(appInfosLock_);
        auto moduleRecords = appInfos_.find(mainBundleName_);
        if (moduleRecords != appInfos_.end()) {
            launchData.SetApplicationInfo(*(moduleRecords->second));
        }
    }
    ProcessInfo processInfo(processName_, GetPid());
    processInfo.SetProcessType(processType_);
    launchData.SetProcessInfo(processInfo);
    launchData.SetRecordId(appRecordId_);
    launchData.SetUId(mainUid_);
    launchData.SetUserTestInfo(userTestRecord_);
    launchData.SetAppIndex(appIndex_);
    launchData.SetInstanceKey(instanceKey_);
    launchData.SetDebugApp(isDebugApp_);
    launchData.SetPerfCmd(perfCmd_);
    launchData.SetErrorInfoEnhance(isErrorInfoEnhance_);
    launchData.SetMultiThread(isMultiThread_);
    launchData.SetJITEnabled(jitEnabled_);
    launchData.SetNativeStart(isNativeStart_);
    launchData.SetAppRunningUniqueId(std::to_string(appRunningUniqueId_));
    launchData.SetIsNeedPreloadModule(isNeedPreloadModule_);
    launchData.SetAppPreloadMode(preloadMode_);
    launchData.SetNWebPreload(isAllowedNWebPreload_);
    launchData.SetPreloadModuleName(preloadModuleName_);
    launchData.SetDebugFromLocal(isDebugFromLocal_);
    launchData.SetStartupTaskData(startupTaskData_);

    TAG_LOGD(AAFwkTag::APPMGR, "%{public}s called,app is %{public}s.", __func__, GetName().c_str());
    AddAppLifecycleEvent("AppRunningRecord::LaunchApplication");
    AbilityRuntime::ErrorMsgGuard errorMsgGuard(GetPid(), reinterpret_cast<uintptr_t>(scheduler.GetRefPtr()),
        "LaunchApplication");
    appLifeCycleDeal_->LaunchApplication(launchData, config);
}

void AppRunningRecord::UpdateApplicationInfoInstalled(const ApplicationInfo &appInfo, const std::string &moduleName)
{
    if (!isStageBasedModel_) {
        TAG_LOGI(AAFwkTag::APPMGR, "Current version than supports");
        return;
    }

    if (appLifeCycleDeal_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null appLifeCycleDeal_");
        return;
    }
    appLifeCycleDeal_->UpdateApplicationInfoInstalled(appInfo, moduleName);
}

void AppRunningRecord::AddAbilityStage()
{
    if (!isStageBasedModel_) {
        TAG_LOGI(AAFwkTag::APPMGR, "Current version than supports");
        return;
    }
    HapModuleInfo abilityStage;
    if (GetTheModuleInfoNeedToUpdated(mainBundleName_, abilityStage)) {
        auto timeout = GetAddStageTimeout() * AppUtils::GetInstance().GetTimeoutUnitTimeRatio();
        SendEvent(AMSEventHandler::ADD_ABILITY_STAGE_INFO_HALF_TIMEOUT_MSG, timeout / HALF_TIMEOUT);
        SendEvent(AMSEventHandler::ADD_ABILITY_STAGE_INFO_TIMEOUT_MSG, timeout);
        TAG_LOGI(AAFwkTag::APPMGR, "Current module : [%{public}s] | bundle : [%{public}s]",
            abilityStage.moduleName.c_str(), mainBundleName_.c_str());
        if (appLifeCycleDeal_ == nullptr) {
            TAG_LOGW(AAFwkTag::APPMGR, "null appLifeCycleDeal_");
            return;
        }
        appLifeCycleDeal_->AddAbilityStage(abilityStage);
    }
}

bool AppRunningRecord::AddAbilityStageBySpecifiedAbility(const std::string &bundleName)
{
    if (!eventHandler_) {
        TAG_LOGE(AAFwkTag::APPMGR, "null eventHandler_");
        return false;
    }

    HapModuleInfo hapModuleInfo;
    if (GetTheModuleInfoNeedToUpdated(bundleName, hapModuleInfo)) {
        if (!AppEventUtil::GetInstance().HasEvent(shared_from_this(),
            AMSEventHandler::ADD_ABILITY_STAGE_INFO_TIMEOUT_MSG)) {
            TAG_LOGI(
                AAFwkTag::APPMGR, "ADD_ABILITY_STAGE_INFO_TIMEOUT_MSG not exist");
            auto timeout = GetAddStageTimeout() * AppUtils::GetInstance().GetTimeoutUnitTimeRatio();
            SendEvent(AMSEventHandler::ADD_ABILITY_STAGE_INFO_HALF_TIMEOUT_MSG, timeout / HALF_TIMEOUT);
            SendEvent(AMSEventHandler::ADD_ABILITY_STAGE_INFO_TIMEOUT_MSG, timeout);
        }
        if (appLifeCycleDeal_ == nullptr) {
            TAG_LOGW(AAFwkTag::APPMGR, "null appLifeCycleDeal_");
            return false;
        }
        appLifeCycleDeal_->AddAbilityStage(hapModuleInfo);
        return true;
    }
    return false;
}

void AppRunningRecord::AddAbilityStageBySpecifiedProcess(const std::string &bundleName)
{
    TAG_LOGD(AAFwkTag::APPMGR, "call.");
    if (!eventHandler_) {
        TAG_LOGE(AAFwkTag::APPMGR, "null eventHandler_");
        return;
    }

    HapModuleInfo hapModuleInfo;
    if (GetTheModuleInfoNeedToUpdated(bundleName, hapModuleInfo)) {
        auto timeout = GetAddStageTimeout() * AppUtils::GetInstance().GetTimeoutUnitTimeRatio();
        SendEvent(AMSEventHandler::ADD_ABILITY_STAGE_INFO_HALF_TIMEOUT_MSG, timeout / HALF_TIMEOUT);
        SendEvent(AMSEventHandler::ADD_ABILITY_STAGE_INFO_TIMEOUT_MSG, timeout);
        if (appLifeCycleDeal_ == nullptr) {
            TAG_LOGW(AAFwkTag::APPMGR, "null appLifeCycleDeal_");
            return;
        }
        appLifeCycleDeal_->AddAbilityStage(hapModuleInfo);
    }
}

void AppRunningRecord::AddAbilityStageDone()
{
    TAG_LOGI(AAFwkTag::APPMGR, "bundle %{public}s", mainBundleName_.c_str());
    RemoveEvent(AMSEventHandler::START_PROCESS_SPECIFIED_ABILITY_TIMEOUT_MSG);
    RemoveEvent(AMSEventHandler::START_PROCESS_SPECIFIED_ABILITY_HALF_TIMEOUT_MSG);
    RemoveEvent(AMSEventHandler::ADD_ABILITY_STAGE_INFO_TIMEOUT_MSG);
    RemoveEvent(AMSEventHandler::ADD_ABILITY_STAGE_INFO_HALF_TIMEOUT_MSG);
    SetModuleLoaded(moduleName_);

    // Should proceed to the next notification
    if (IsStartSpecifiedAbility()) {
        ScheduleAcceptWant(moduleName_);
        return;
    }

    if (IsNewProcessRequest()) {
        TAG_LOGD(AAFwkTag::APPMGR, "ScheduleNewProcessRequest.");
        ScheduleNewProcessRequest(GetNewProcessRequestWant(), moduleName_);
        return;
    }

    AddAbilityStage();
}

void AppRunningRecord::SetModuleLoaded(const std::string &moduleName) const
{
    auto moduleRecordList = GetAllModuleRecord();
    for (const auto &item : moduleRecordList) {
        if (item && item->GetModuleName() == moduleName) {
            item->SetLoaded();
            break;
        }
    }
}

void AppRunningRecord::LaunchAbility(const std::shared_ptr<AbilityRunningRecord> &ability)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    if (appLifeCycleDeal_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null appLifeCycleDeal_");
        return;
    }
    if (!ability || !ability->GetToken()) {
        TAG_LOGE(AAFwkTag::APPMGR, "null abilityRecord or abilityToken");
        return;
    }

    auto moduleRecord = GetModuleRunningRecordByToken(ability->GetToken());
    if (!moduleRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "null moduleRecord");
        return;
    }

    moduleRecord->LaunchAbility(ability);
}

void AppRunningRecord::ScheduleTerminate()
{
    auto timeout = AMSEventHandler::TERMINATE_APPLICATION_TIMEOUT * AppUtils::GetInstance().GetTimeoutUnitTimeRatio();
    SendEvent(AMSEventHandler::TERMINATE_APPLICATION_HALF_TIMEOUT_MSG, timeout / HALF_TIMEOUT);
    SendEvent(AMSEventHandler::TERMINATE_APPLICATION_TIMEOUT_MSG, timeout);
    if (appLifeCycleDeal_ == nullptr) {
        TAG_LOGW(AAFwkTag::APPMGR, "null appLifeCycleDeal_");
        return;
    }
    bool isLastProcess = false;
    auto serviceInner = appMgrServiceInner_.lock();
    if (serviceInner != nullptr) {
        isLastProcess = serviceInner->IsFinalAppProcessByBundleName(GetBundleName());
    }
    appLifeCycleDeal_->ScheduleTerminate(isLastProcess);
}

void AppRunningRecord::LaunchPendingAbilities()
{
    TAG_LOGD(AAFwkTag::APPMGR, "Launch pending abilities.");
    AddAppLifecycleEvent("AppRunningRecord::LaunchPendingAbilities");
    auto moduleRecordList = GetAllModuleRecord();
    if (moduleRecordList.empty()) {
        TAG_LOGE(AAFwkTag::APPMGR, "empty moduleRecordList");
        return;
    }
    for (const auto &moduleRecord : moduleRecordList) {
        moduleRecord->SetApplicationClient(appLifeCycleDeal_);
        moduleRecord->LaunchPendingAbilities();
    }
}

bool AppRunningRecord::ScheduleForegroundRunning()
{
    SetApplicationScheduleState(ApplicationScheduleState::SCHEDULE_FOREGROUNDING);
    if (appLifeCycleDeal_) {
        AddAppLifecycleEvent("AppRunningRecord::ScheduleForegroundRunning");
        auto scheduler = appLifeCycleDeal_->GetApplicationClient();
        if (!scheduler) {
            AddAppLifecycleEvent("AppRunningRecord::ScheduleForegroundRunning; null scheduler");
            return false;
        }
        AbilityRuntime::ErrorMsgGuard errorMsgGuard(GetPid(), reinterpret_cast<uintptr_t>(scheduler.GetRefPtr()),
            "ScheduleForegroundRunning");
        return appLifeCycleDeal_->ScheduleForegroundRunning();
    }
    return false;
}

void AppRunningRecord::ScheduleBackgroundRunning()
{
    SetApplicationScheduleState(ApplicationScheduleState::SCHEDULE_BACKGROUNDING);
    int32_t recordId = GetRecordId();
    auto serviceInner = appMgrServiceInner_;
    auto appbackgroundtask = [recordId, serviceInner]() {
        auto serviceInnerObj = serviceInner.lock();
        if (serviceInnerObj == nullptr) {
            TAG_LOGW(AAFwkTag::APPMGR, "APPManager is invalid");
            return;
        }
        TAG_LOGE(AAFwkTag::APPMGR, "APPManager move timeout");
        serviceInnerObj->ApplicationBackgrounded(recordId);
    };
    auto taskName = std::string("appbackground_") + std::to_string(recordId);
    if (taskHandler_) {
        taskHandler_->CancelTask(taskName);
    }
    PostTask(taskName, AMSEventHandler::BACKGROUND_APPLICATION_TIMEOUT, appbackgroundtask);
    if (appLifeCycleDeal_) {
        AddAppLifecycleEvent("AppRunningRecord::ScheduleBackgroundRunning");
        appLifeCycleDeal_->ScheduleBackgroundRunning();
    }
    isAbilityForegrounding_.store(false);
}

void AppRunningRecord::ScheduleProcessSecurityExit()
{
    if (appLifeCycleDeal_) {
        auto appRecord = shared_from_this();
        DelayedSingleton<CacheProcessManager>::GetInstance()->PrepareActivateCache(appRecord);
        appLifeCycleDeal_->ScheduleProcessSecurityExit();
    }
}

void AppRunningRecord::ScheduleClearPageStack()
{
    if (appLifeCycleDeal_) {
        appLifeCycleDeal_->ScheduleClearPageStack();
    }
}

void AppRunningRecord::ScheduleTrimMemory()
{
    if (appLifeCycleDeal_ && priorityObject_) {
        appLifeCycleDeal_->ScheduleTrimMemory(priorityObject_->GetTimeLevel());
    }
}

void AppRunningRecord::ScheduleMemoryLevel(int32_t level)
{
    if (appLifeCycleDeal_) {
        appLifeCycleDeal_->ScheduleMemoryLevel(level);
    }
}

void AppRunningRecord::ScheduleHeapMemory(const int32_t pid, OHOS::AppExecFwk::MallocInfo &mallocInfo)
{
    if (appLifeCycleDeal_) {
        appLifeCycleDeal_->ScheduleHeapMemory(pid, mallocInfo);
    }
}

void AppRunningRecord::ScheduleJsHeapMemory(OHOS::AppExecFwk::JsHeapDumpInfo &info)
{
    if (appLifeCycleDeal_) {
        appLifeCycleDeal_->ScheduleJsHeapMemory(info);
    }
}

void AppRunningRecord::ScheduleCjHeapMemory(OHOS::AppExecFwk::CjHeapDumpInfo &info)
{
    if (appLifeCycleDeal_) {
        appLifeCycleDeal_->ScheduleCjHeapMemory(info);
    }
}

void AppRunningRecord::LowMemoryWarning()
{
    if (appLifeCycleDeal_) {
        appLifeCycleDeal_->LowMemoryWarning();
    }
}

void AppRunningRecord::AddModules(
    const std::shared_ptr<ApplicationInfo> &appInfo, const std::vector<HapModuleInfo> &moduleInfos)
{
    TAG_LOGD(AAFwkTag::APPMGR, "Add modules");

    if (moduleInfos.empty()) {
        TAG_LOGI(AAFwkTag::APPMGR, "empty moduleInfos");
        return;
    }

    for (auto &iter : moduleInfos) {
        AddModule(appInfo, nullptr, nullptr, iter, nullptr, 0);
    }
}

void AppRunningRecord::AddModule(std::shared_ptr<ApplicationInfo> appInfo,
    std::shared_ptr<AbilityInfo> abilityInfo, sptr<IRemoteObject> token,
    const HapModuleInfo &hapModuleInfo, std::shared_ptr<AAFwk::Want> want, int32_t abilityRecordId)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");

    if (!appInfo) {
        TAG_LOGE(AAFwkTag::APPMGR, "null appInfo");
        return;
    }

    std::shared_ptr<ModuleRunningRecord> moduleRecord;
    std::lock_guard<ffrt::mutex> hapModulesLock(hapModulesLock_);
    const auto &iter = hapModules_.find(appInfo->bundleName);
    if (iter != hapModules_.end()) {
        moduleRecord = GetModuleRecordByModuleName(appInfo->bundleName, hapModuleInfo.moduleName);
        if (!moduleRecord) {
            moduleRecord = std::make_shared<ModuleRunningRecord>(appInfo, eventHandler_);
            iter->second.push_back(moduleRecord);
            moduleRecord->Init(hapModuleInfo, appIndex_, appMgrServiceInner_, appLifeCycleDeal_);
        }
    } else {
        moduleRecord = std::make_shared<ModuleRunningRecord>(appInfo, eventHandler_);
        std::vector<std::shared_ptr<ModuleRunningRecord>> moduleList;
        moduleList.push_back(moduleRecord);
        hapModules_.emplace(appInfo->bundleName, moduleList);
        {
            std::lock_guard<ffrt::mutex> appInfosLock(appInfosLock_);
            appInfos_.emplace(appInfo->bundleName, appInfo);
        }
        moduleRecord->Init(hapModuleInfo, appIndex_, appMgrServiceInner_, appLifeCycleDeal_);
    }

    if (!abilityInfo || !token) {
        TAG_LOGE(AAFwkTag::APPMGR, "null abilityInfo or token");
        return;
    }
    moduleRecord->AddAbility(token, abilityInfo, want, abilityRecordId);

    return;
}

std::shared_ptr<ModuleRunningRecord> AppRunningRecord::GetModuleRecordByModuleName(
    const std::string &bundleName, const std::string &moduleName)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    auto moduleRecords = hapModules_.find(bundleName);
    if (moduleRecords != hapModules_.end()) {
        for (auto &iter : moduleRecords->second) {
            if (iter->GetModuleName() == moduleName) {
                return iter;
            }
        }
    }

    return nullptr;
}

void AppRunningRecord::StateChangedNotifyObserver(const std::shared_ptr<AbilityRunningRecord> &ability,
    int32_t state, bool isAbility, bool isFromWindowFocusChanged)
{
    if (ability == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null ability");
        return;
    }
    auto abilityInfo = ability->GetAbilityInfo();
    if (abilityInfo == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null abilityInfo");
        return;
    }
    AbilityStateData abilityStateData;
    abilityStateData.bundleName = abilityInfo->applicationInfo.bundleName;
    abilityStateData.moduleName = abilityInfo->moduleName;
    abilityStateData.abilityName = ability->GetName();
    abilityStateData.pid = GetPid();
    abilityStateData.abilityState = state;
    abilityStateData.uid = abilityInfo->applicationInfo.uid;
    abilityStateData.token = ability->GetToken();
    abilityStateData.abilityType = static_cast<int32_t>(abilityInfo->type);
    abilityStateData.isFocused = ability->GetFocusFlag();
    abilityStateData.abilityRecordId = ability->GetAbilityRecordId();
    auto applicationInfo = GetApplicationInfo();
    if (applicationInfo && (static_cast<int32_t>(applicationInfo->multiAppMode.multiAppModeType) ==
            static_cast<int32_t>(MultiAppModeType::APP_CLONE))) {
            abilityStateData.appCloneIndex = appIndex_;
    }
    if (ability->GetWant() != nullptr) {
        abilityStateData.callerAbilityName = ability->GetWant()->GetStringParam(Want::PARAM_RESV_CALLER_ABILITY_NAME);
        abilityStateData.callerBundleName = ability->GetWant()->GetStringParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME);
        abilityStateData.callerUid = ability->GetWant()->GetIntParam(Want::PARAM_RESV_CALLER_UID, -1);
        abilityStateData.callerPid = ability->GetWant()->GetIntParam(Want::PARAM_RESV_CALLER_PID, -1);
    }
    if (applicationInfo && applicationInfo->bundleType == AppExecFwk::BundleType::ATOMIC_SERVICE) {
        abilityStateData.isAtomicService = true;
    }
    abilityStateData.extensionAbilityType = static_cast<int32_t>(abilityInfo->extensionAbilityType);
    bool isUIExtension = AAFwk::UIExtensionWrapper::IsUIExtension(abilityInfo->extensionAbilityType);
    if (isAbility && isUIExtension) {
        abilityStateData.isInnerNotify = true;
    }
    abilityStateData.processType = static_cast<int32_t>(processType_);
    abilityStateData.preloadMode = static_cast<int32_t>(preloadMode_);
    BundleType bundleType = applicationInfo ? applicationInfo->bundleType : AppExecFwk::BundleType::APP;
    auto serviceInner = appMgrServiceInner_.lock();
    if (serviceInner) {
        serviceInner->StateChangedNotifyObserver(abilityStateData, isAbility, isFromWindowFocusChanged, bundleType);
    }
}

std::shared_ptr<ModuleRunningRecord> AppRunningRecord::GetModuleRunningRecordByToken(
    const sptr<IRemoteObject> &token) const
{
    if (!token) {
        return nullptr;
    }

    auto moduleRecordList = GetAllModuleRecord();
    for (const auto &moduleRecord : moduleRecordList) {
        if (moduleRecord && moduleRecord->GetAbilityRunningRecordByToken(token)) {
            return moduleRecord;
        }
    }

    return nullptr;
}

std::shared_ptr<ModuleRunningRecord> AppRunningRecord::GetModuleRunningRecordByTerminateLists(
    const sptr<IRemoteObject> &token) const
{
    if (!token) {
        TAG_LOGE(AAFwkTag::APPMGR, "null token");
        return nullptr;
    }

    auto moduleRecordList = GetAllModuleRecord();
    for (const auto &moduleRecord : moduleRecordList) {
        if (moduleRecord && moduleRecord->GetAbilityByTerminateLists(token)) {
            return moduleRecord;
        }
    }

    return nullptr;
}

std::shared_ptr<AbilityRunningRecord> AppRunningRecord::GetAbilityRunningRecordByToken(
    const sptr<IRemoteObject> &token) const
{
    auto moduleRecord = GetModuleRunningRecordByToken(token);
    if (!moduleRecord) {
        return nullptr;
    }
    return moduleRecord->GetAbilityRunningRecordByToken(token);
}

std::shared_ptr<AbilityRunningRecord> AppRunningRecord::GetAbilityByTerminateLists(
    const sptr<IRemoteObject> &token) const
{
    auto moduleRecord = GetModuleRunningRecordByTerminateLists(token);
    if (!moduleRecord) {
        return nullptr;
    }
    return moduleRecord->GetAbilityByTerminateLists(token);
}

bool AppRunningRecord::UpdateAbilityFocusState(const sptr<IRemoteObject> &token, bool isFocus)
{
    TAG_LOGD(AAFwkTag::APPMGR, "focus state is :%{public}d", isFocus);
    auto abilityRecord = GetAbilityRunningRecordByToken(token);
    if (!abilityRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "can not find ability record");
        return false;
    }

    bool lastFocusState = abilityRecord->GetFocusFlag();
    if (lastFocusState == isFocus) {
        TAG_LOGE(AAFwkTag::APPMGR, "no need update");
        return false;
    }

    if (isFocus) {
        return AbilityFocused(abilityRecord);
    } else {
        return AbilityUnfocused(abilityRecord);
    }
}

void AppRunningRecord::UpdateAbilityState(const sptr<IRemoteObject> &token, const AbilityState state)
{
    TAG_LOGD(AAFwkTag::APPMGR, "state is :%{public}d", static_cast<int32_t>(state));
    auto abilityRecord = GetAbilityRunningRecordByToken(token);
    if (!abilityRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "can not find ability record");
        return;
    }
    if (state == AbilityState::ABILITY_STATE_CREATE) {
        StateChangedNotifyObserver(
            abilityRecord, static_cast<int32_t>(AbilityState::ABILITY_STATE_CREATE), true, false);
        return;
    }
    if (state == abilityRecord->GetState()) {
        TAG_LOGE(AAFwkTag::APPMGR, "no need update");
        return;
    }

    if (state == AbilityState::ABILITY_STATE_FOREGROUND) {
        AbilityForeground(abilityRecord);
    } else if (state == AbilityState::ABILITY_STATE_BACKGROUND) {
        AbilityBackground(abilityRecord);
    } else {
        TAG_LOGW(AAFwkTag::APPMGR, "wrong state");
    }
}

void AppRunningRecord::AbilityForeground(const std::shared_ptr<AbilityRunningRecord> &ability)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    if (!ability) {
        TAG_LOGE(AAFwkTag::APPMGR, "null ability");
        return;
    }

    AbilityState curAbilityState = ability->GetState();
    if (curAbilityState != AbilityState::ABILITY_STATE_READY &&
        curAbilityState != AbilityState::ABILITY_STATE_BACKGROUND) {
        TAG_LOGE(AAFwkTag::APPMGR, "ability state(%{public}d) error", static_cast<int32_t>(curAbilityState));
        return;
    }

    TAG_LOGI(AAFwkTag::APPMGR, "appState: %{public}d, pState: %{public}d, %{public}s/%{public}s",
        GetState(), pendingState_, mainBundleName_.c_str(), ability->GetName().c_str());
    // We need schedule application to foregrounded when current application state is ready or background running.
    if (GetState() == ApplicationState::APP_STATE_FOREGROUND
        && pendingState_ != ApplicationPendingState::BACKGROUNDING) {
        // Just change ability to foreground if current application state is foreground or focus.
        auto moduleRecord = GetModuleRunningRecordByToken(ability->GetToken());
        if (moduleRecord == nullptr) {
            TAG_LOGE(AAFwkTag::APPMGR, "null moduleRecord");
            return;
        }

        moduleRecord->OnAbilityStateChanged(ability, AbilityState::ABILITY_STATE_FOREGROUND);
        StateChangedNotifyObserver(
            ability, static_cast<int32_t>(AbilityState::ABILITY_STATE_FOREGROUND), true, false);

        auto serviceInner = appMgrServiceInner_.lock();
        if (serviceInner) {
            serviceInner->OnAppStateChanged(shared_from_this(), GetState(), false, false, false);
        }
        return;
    }
    if (GetState() == ApplicationState::APP_STATE_READY
        || GetState() == ApplicationState::APP_STATE_BACKGROUND
        || GetState() == ApplicationState::APP_STATE_FOREGROUND) {
        auto pendingState = pendingState_;
        SetApplicationPendingState(ApplicationPendingState::FOREGROUNDING);
        if (pendingState == ApplicationPendingState::READY && !ScheduleForegroundRunning()) {
            AbilityRuntime::FreezeUtil::GetInstance().AppendLifecycleEvent(ability->GetToken(),
                "ScheduleForegroundRunning fail");
        }
        foregroundingAbilityTokens_.insert(ability->GetToken());
        TAG_LOGD(AAFwkTag::APPMGR, "foregroundingAbility size: %{public}d",
            static_cast<int32_t>(foregroundingAbilityTokens_.size()));
    } else {
        TAG_LOGW(AAFwkTag::APPMGR, "wrong state");
    }
}

void AppRunningRecord::AbilityBackground(const std::shared_ptr<AbilityRunningRecord> &ability)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    if (!ability) {
        TAG_LOGE(AAFwkTag::APPMGR, "null ability");
        return;
    }
    TAG_LOGD(AAFwkTag::APPMGR, "ability is %{public}s", mainBundleName_.c_str());
    if (ability->GetState() != AbilityState::ABILITY_STATE_FOREGROUND &&
        ability->GetState() != AbilityState::ABILITY_STATE_READY) {
        TAG_LOGE(AAFwkTag::APPMGR, "ability state is not foreground or focus");
        return;
    }

    // First change ability to background.
    auto moduleRecord = GetModuleRunningRecordByToken(ability->GetToken());
    if (moduleRecord == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null moduleRecord");
        return;
    }
    moduleRecord->OnAbilityStateChanged(ability, AbilityState::ABILITY_STATE_BACKGROUND);
    StateChangedNotifyObserver(
        ability, static_cast<int32_t>(AbilityState::ABILITY_STATE_BACKGROUND), true, false);
    if (GetState() != ApplicationState::APP_STATE_FOREGROUND &&
        GetState() != ApplicationState::APP_STATE_CACHED &&
        GetState() != ApplicationState::APP_STATE_READY) {
        TAG_LOGW(AAFwkTag::APPMGR, "wrong state: %{public}d", GetState());
        return;
    }
    int32_t foregroundSize = 0;
    auto abilitiesMap = GetAbilities();
    for (const auto &item : abilitiesMap) {
        const auto &abilityRecord = item.second;
        if (abilityRecord && abilityRecord->GetState() == AbilityState::ABILITY_STATE_FOREGROUND &&
            abilityRecord->GetAbilityInfo() &&
            (abilityRecord->GetAbilityInfo()->type == AppExecFwk::AbilityType::PAGE
            || AAFwk::UIExtensionWrapper::IsUIExtension(abilityRecord->GetAbilityInfo()->extensionAbilityType))) {
            foregroundSize++;
            break;
        }
    }

    // Then schedule application background when all ability is not foreground.
    if (foregroundSize == 0 && (mainBundleName_ != LAUNCHER_NAME || IsAllowScbProcessMoveToBackground())
        && IsWindowIdsEmpty()) {
        auto pendingState = pendingState_;
        SetApplicationPendingState(ApplicationPendingState::BACKGROUNDING);
        if (pendingState == ApplicationPendingState::READY) {
            ScheduleBackgroundRunning();
        }
    }
}

bool AppRunningRecord::AbilityFocused(const std::shared_ptr<AbilityRunningRecord> &ability)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    if (!ability) {
        TAG_LOGE(AAFwkTag::APPMGR, "null ability");
        return false;
    }
    ability->UpdateFocusState(true);

    // update ability state
    int32_t abilityState = static_cast<int32_t>(ability->GetState());
    bool isAbility = true;
    if (ability->GetAbilityInfo() != nullptr && ability->GetAbilityInfo()->type == AbilityType::EXTENSION) {
        isAbility = false;
    }
    StateChangedNotifyObserver(ability, abilityState, isAbility, true);

    if (isFocused_) {
        // process state is already focused, no need update process state.
        return false;
    }

    // update process focus state to true.
    isFocused_ = true;
    return true;
}

bool AppRunningRecord::AbilityUnfocused(const std::shared_ptr<AbilityRunningRecord> &ability)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    if (!ability) {
        TAG_LOGE(AAFwkTag::APPMGR, "null ability");
        return false;
    }
    ability->UpdateFocusState(false);

    // update ability state to unfocused.
    int32_t abilityState = static_cast<int32_t>(ability->GetState());
    bool isAbility = true;
    if (ability->GetAbilityInfo() != nullptr && ability->GetAbilityInfo()->type == AbilityType::EXTENSION) {
        isAbility = false;
    }
    StateChangedNotifyObserver(ability, abilityState, isAbility, true);

    if (!isFocused_) {
        return false; // invalid process focus state, already unfocused, process state not change.
    }

    bool changeProcessToUnfocused = true;
    auto abilitysMap = GetAbilities();
    for (const auto &item : abilitysMap) {
        const auto &abilityRecord = item.second;
        if (abilityRecord && abilityRecord->GetFocusFlag()) {
            changeProcessToUnfocused = false;
            break;
        }
    }

    if (changeProcessToUnfocused) {
        isFocused_ = false; // process focus state : from focus to unfocus.
    }
    return changeProcessToUnfocused;
}

void AppRunningRecord::PopForegroundingAbilityTokens()
{
    TAG_LOGI(AAFwkTag::PROCESSMGR, "ability size: %{public}d",
        static_cast<int32_t>(foregroundingAbilityTokens_.size()));
    for (auto iter = foregroundingAbilityTokens_.begin(); iter != foregroundingAbilityTokens_.end();) {
        auto ability = GetAbilityRunningRecordByToken(*iter);
        auto moduleRecord = GetModuleRunningRecordByToken(*iter);
        if (moduleRecord != nullptr) {
            moduleRecord->OnAbilityStateChanged(ability, AbilityState::ABILITY_STATE_FOREGROUND);
            StateChangedNotifyObserver(
                ability, static_cast<int32_t>(AbilityState::ABILITY_STATE_FOREGROUND), true, false);
        } else {
            TAG_LOGW(AAFwkTag::APPMGR, "null moduleRecord");
        }
        // The token should be removed even though the module record didn't exist.
        iter = foregroundingAbilityTokens_.erase(iter);
    }
}

void AppRunningRecord::TerminateAbility(const sptr<IRemoteObject> &token, const bool isForce, bool isTimeout)
{
    TAG_LOGD(AAFwkTag::APPMGR, "isForce: %{public}d", static_cast<int>(isForce));

    auto moduleRecord = GetModuleRunningRecordByToken(token);
    if (!moduleRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "null moduleRecord");
        return;
    }

    auto abilityRecord = GetAbilityRunningRecordByToken(token);
    if (abilityRecord) {
        TAG_LOGI(AAFwkTag::PROCESSMGR, "terminate %{public}s", abilityRecord->GetName().c_str());
    }
    if (!isTimeout) {
        StateChangedNotifyObserver(
            abilityRecord, static_cast<int32_t>(AbilityState::ABILITY_STATE_TERMINATED), true, false);
    }
    moduleRecord->TerminateAbility(shared_from_this(), token, isForce);
}

void AppRunningRecord::AbilityTerminated(const sptr<IRemoteObject> &token)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    auto moduleRecord = GetModuleRunningRecordByTerminateLists(token);
    if (!moduleRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "null moduleRecord");
        return;
    }

    bool isExtensionDebug = false;
    auto abilityRecord = moduleRecord->GetAbilityByTerminateLists(token);
    if (abilityRecord != nullptr && abilityRecord->GetAbilityInfo() != nullptr) {
        isExtensionDebug = (abilityRecord->GetAbilityInfo()->type == AppExecFwk::AbilityType::EXTENSION) &&
                           (isAttachDebug_ || isDebugApp_);
    }
    TAG_LOGD(AAFwkTag::APPMGR, "Extension debug is [%{public}s]", isExtensionDebug ? "true" : "false");

    moduleRecord->AbilityTerminated(token);

    auto appRecord = shared_from_this();
    auto cacheProcMgr = DelayedSingleton<CacheProcessManager>::GetInstance();
    bool needCache = false;
    if (cacheProcMgr != nullptr && cacheProcMgr->IsAppShouldCache(appRecord)) {
        cacheProcMgr->CheckAndCacheProcess(appRecord);
        TAG_LOGI(AAFwkTag::APPMGR, "App %{public}s should not remove module and terminate app",
            appRecord->GetBundleName().c_str());
        needCache = true;
    }
    auto state = static_cast<int>(GetSupportProcessCacheState());
    auto appInfo = appRecord->GetApplicationInfo();
    auto hisyseventReport = std::make_shared<AAFwk::HisyseventReport>(4);
    hisyseventReport->InsertParam(EVENT_KEY_VERSION_CODE, appInfo->versionCode);
    hisyseventReport->InsertParam(EVENT_KEY_VERSION_NAME, appInfo->versionName);
    hisyseventReport->InsertParam(EVENT_KEY_BUNDLE_NAME, appInfo->bundleName);
    hisyseventReport->InsertParam(EVENT_KEY_SUPPORT_STATE, state);
    hisyseventReport->Report("AAFWK", "CACHE_SUPPORT_STATE", HISYSEVENT_BEHAVIOR);
    if (moduleRecord->GetAbilities().empty() && (!IsKeepAliveApp()
        || AAFwk::UIExtensionWrapper::IsUIExtension(GetExtensionType())
        || !ExitResidentProcessManager::GetInstance().IsMemorySizeSufficient()) && !needCache) {
        RemoveModuleRecord(moduleRecord, isExtensionDebug);
    }

    auto moduleRecordList = GetAllModuleRecord();
    if (moduleRecordList.empty() && (!IsKeepAliveApp()
        || AAFwk::UIExtensionWrapper::IsUIExtension(GetExtensionType())
        || !ExitResidentProcessManager::GetInstance().IsMemorySizeSufficient()) && !isExtensionDebug
        && !needCache) {
        ScheduleTerminate();
    }
}

std::list<std::shared_ptr<ModuleRunningRecord>> AppRunningRecord::GetAllModuleRecord() const
{
    std::list<std::shared_ptr<ModuleRunningRecord>> moduleRecordList;
    std::lock_guard<ffrt::mutex> hapModulesLock(hapModulesLock_);
    for (const auto &item : hapModules_) {
        for (const auto &list : item.second) {
            moduleRecordList.push_back(list);
        }
    }
    return moduleRecordList;
}

void AppRunningRecord::RemoveAppDeathRecipient() const
{
    if (appLifeCycleDeal_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null appLifeCycleDeal_");
        return;
    }
    if (!appLifeCycleDeal_->GetApplicationClient()) {
        TAG_LOGE(AAFwkTag::APPMGR, "null appThread");
        return;
    }
    auto object = appLifeCycleDeal_->GetApplicationClient()->AsObject();
    if (object) {
        if (!object->RemoveDeathRecipient(appDeathRecipient_)) {
            TAG_LOGD(AAFwkTag::APPMGR, "Failed to remove deathRecipient.");
        }
    }
}

void AppRunningRecord::SetAppMgrServiceInner(const std::weak_ptr<AppMgrServiceInner> &inner)
{
    appMgrServiceInner_ = inner;

    auto moduleRecordList = GetAllModuleRecord();
    if (moduleRecordList.empty()) {
        TAG_LOGW(AAFwkTag::APPMGR, "empty moduleRecordList");
        return;
    }

    for (const auto &moduleRecord : moduleRecordList) {
        moduleRecord->SetAppMgrServiceInner(appMgrServiceInner_);
    }
}

void AppRunningRecord::SetAppDeathRecipient(const sptr<AppDeathRecipient> &appDeathRecipient)
{
    appDeathRecipient_ = appDeathRecipient;
}

std::shared_ptr<PriorityObject> AppRunningRecord::GetPriorityObject()
{
    return priorityObject_;
}

void AppRunningRecord::SendEventForSpecifiedAbility()
{
    auto timeOUt = AMSEventHandler::START_PROCESS_SPECIFIED_ABILITY_TIMEOUT *
        AppUtils::GetInstance().GetTimeoutUnitTimeRatio();
    SendEvent(AMSEventHandler::START_PROCESS_SPECIFIED_ABILITY_HALF_TIMEOUT_MSG, timeOUt / HALF_TIMEOUT);
    SendEvent(AMSEventHandler::START_PROCESS_SPECIFIED_ABILITY_TIMEOUT_MSG, timeOUt);
}

void AppRunningRecord::SendAppStartupTypeEvent(const std::shared_ptr<AbilityRunningRecord> &ability,
    const AppStartType startType)
{
    if (!ability) {
        TAG_LOGE(AAFwkTag::APPMGR, "null ability");
        return;
    }
    AAFwk::EventInfo eventInfo;
    auto applicationInfo = GetApplicationInfo();
    if (!applicationInfo) {
        TAG_LOGE(AAFwkTag::APPMGR, "null applicationInfo");
    } else {
        eventInfo.bundleName = applicationInfo->name;
        eventInfo.versionName = applicationInfo->versionName;
        eventInfo.versionCode = applicationInfo->versionCode;
    }

    auto abilityInfo = ability->GetAbilityInfo();
    if (!abilityInfo) {
        TAG_LOGE(AAFwkTag::APPMGR, "null abilityInfo");
    } else {
        eventInfo.abilityName = abilityInfo->name;
    }
    if (GetPriorityObject() == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null priorityObject");
    } else {
        eventInfo.pid = GetPid();
    }
    eventInfo.startType = static_cast<int32_t>(startType);
    AAFwk::EventReport::SendAppEvent(AAFwk::EventName::APP_STARTUP_TYPE, HISYSEVENT_BEHAVIOR, eventInfo);
}

void AppRunningRecord::SendEvent(uint32_t msg, int64_t timeOut)
{
    if (!eventHandler_) {
        TAG_LOGE(AAFwkTag::APPMGR, "null eventHandler_");
        return;
    }

    if (ShouldSkipTimeout()) {
        TAG_LOGI(AAFwkTag::APPMGR, "no need to handle time out");
        return;
    }

    appEventId_++;
    auto param = appEventId_;

    eventHandler_->SendEvent(AAFwk::EventWrap(msg, param), timeOut, false);
    AppEventUtil::GetInstance().AddEvent(shared_from_this(), msg, param);
}

void AppRunningRecord::RemoveEvent(uint32_t msg)
{
    if (!eventHandler_) {
        TAG_LOGE(AAFwkTag::APPMGR, "null eventHandler_");
        return;
    }
    auto eventList = AppEventUtil::GetInstance().RemoveEvent(shared_from_this(), msg);
    for (const auto &eventData : eventList) {
        eventHandler_->RemoveEvent(msg, eventData.param);
    }
}

void AppRunningRecord::PostTask(std::string msg, int64_t timeOut, const Closure &task)
{
    if (!taskHandler_) {
        TAG_LOGE(AAFwkTag::APPMGR, "null taskHandler_");
        return;
    }
    taskHandler_->SubmitTask(task, msg, timeOut);
}

void AppRunningRecord::SetTaskHandler(std::shared_ptr<AAFwk::TaskHandlerWrap> taskHandler)
{
    taskHandler_ = taskHandler;
}

void AppRunningRecord::SetEventHandler(const std::shared_ptr<AMSEventHandler> &handler)
{
    eventHandler_ = handler;
}

bool AppRunningRecord::IsLastAbilityByFlag(sptr<IRemoteObject> token, bool clearMissionFlag)
{
    if (clearMissionFlag || isPrepareExit_) {
        return IsLastPageAbilityRecord(token);
    }
    return IsLastAbilityRecord(token);
}

bool AppRunningRecord::IsLastAbilityRecord(const sptr<IRemoteObject> &token)
{
    {
        std::lock_guard lock(specifiedMutex_);
        if (!IsTerminating() && specifiedAbilityRequest_ != nullptr) {
            return false;
        }
    }
    auto moduleRecord = GetModuleRunningRecordByToken(token);
    if (!moduleRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "null moduleRecord");
        return false;
    }

    auto moduleRecordList = GetAllModuleRecord();
    if (moduleRecordList.size() == 1) {
        return moduleRecord->IsLastAbilityRecord(token);
    }

    return false;
}

bool AppRunningRecord::ExtensionAbilityRecordExists()
{
    auto moduleRecordList = GetAllModuleRecord();
    for (auto moduleRecord : moduleRecordList) {
        if (moduleRecord && moduleRecord->ExtensionAbilityRecordExists()) {
            return true;
        }
    }
    TAG_LOGD(AAFwkTag::APPMGR, "can not find extension record");
    return false;
}

bool AppRunningRecord::IsLastPageAbilityRecord(const sptr<IRemoteObject> &token)
{
    {
        std::lock_guard lock(specifiedMutex_);
        if (!IsTerminating() && specifiedAbilityRequest_ != nullptr) {
            return false;
        }
    }
    auto moduleRecord = GetModuleRunningRecordByToken(token);
    if (!moduleRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "null moduleRecord");
        return false;
    }

    int32_t pageAbilitySize = 0;
    auto moduleRecordList = GetAllModuleRecord();
    for (auto moduleRecord : moduleRecordList) {
        if (moduleRecord) {
            pageAbilitySize += moduleRecord->GetPageAbilitySize();
        }
        if (pageAbilitySize > 1) {
            return false;
        }
    }

    return pageAbilitySize == 1;
}

void AppRunningRecord::SetTerminating()
{
    isTerminating = true;
    auto prioObject = GetPriorityObject();
    if (prioObject) {
        AbilityRuntime::FreezeUtil::GetInstance().DeleteAppLifecycleEvent(prioObject->GetPid());
    }
}

bool AppRunningRecord::IsTerminating()
{
    return isTerminating;
}

bool AppRunningRecord::IsKeepAliveApp() const
{
    if (!isMainProcess_ || !isKeepAliveBundle_ || !isKeepAliveRdb_) {
        return false;
    }
    auto userId = GetUid() / BASE_USER_RANGE;
    if (userId == 0) {
        return isSingleton_;
    }
    return true;
}

bool AppRunningRecord::IsKeepAliveDkv() const
{
    return isKeepAliveDkv_;
}

bool AppRunningRecord::IsKeepAliveAppService() const
{
    return isKeepAliveAppService_;
}

void AppRunningRecord::SetKeepAliveEnableState(bool isKeepAliveEnable)
{
    isKeepAliveRdb_ = isKeepAliveEnable;
}

void AppRunningRecord::SetKeepAliveDkv(bool isKeepAliveDkv)
{
    isKeepAliveDkv_ = isKeepAliveDkv;
}

void AppRunningRecord::SetMainElementRunning(bool isMainElementRunning)
{
    isMainElementRunning_ = isMainElementRunning;
}

bool AppRunningRecord::IsMainElementRunning() const
{
    return isMainElementRunning_;
}

void AppRunningRecord::SetKeepAliveAppService(bool isKeepAliveAppService)
{
    isKeepAliveAppService_ = isKeepAliveAppService;
}

void AppRunningRecord::SetKeepAliveBundle(bool isKeepAliveBundle)
{
    isKeepAliveBundle_ = isKeepAliveBundle;
}

bool AppRunningRecord::IsEmptyKeepAliveApp() const
{
    return isEmptyKeepAliveApp_;
}

void AppRunningRecord::SetEmptyKeepAliveAppState(bool isEmptyKeepAliveApp)
{
    isEmptyKeepAliveApp_ = isEmptyKeepAliveApp;
}

bool AppRunningRecord::IsMainProcess() const
{
    return isMainProcess_;
}

void AppRunningRecord::SetMainProcess(bool isMainProcess)
{
    isMainProcess_ = isMainProcess;
}

void AppRunningRecord::SetSingleton(bool isSingleton)
{
    isSingleton_ = isSingleton;
}

void AppRunningRecord::SetStageModelState(bool isStageBasedModel)
{
    isStageBasedModel_ = isStageBasedModel;
}

bool AppRunningRecord::GetTheModuleInfoNeedToUpdated(const std::string bundleName, HapModuleInfo &info)
{
    bool result = false;
    std::lock_guard<ffrt::mutex> hapModulesLock(hapModulesLock_);
    auto moduleInfoVectorIter = hapModules_.find(bundleName);
    if (moduleInfoVectorIter == hapModules_.end() || moduleInfoVectorIter->second.empty()) {
        return result;
    }
    std::string moduleName = moduleName_;
    auto findCondition = [moduleName](const std::shared_ptr<ModuleRunningRecord> &record) {
        if (record) {
            return (moduleName.empty() || (moduleName == record->GetModuleName())) &&
                (record->GetModuleRecordState() == ModuleRecordState::INITIALIZED_STATE);
        }
        return false;
    };
    auto moduleRecordIter =
        std::find_if(moduleInfoVectorIter->second.begin(), moduleInfoVectorIter->second.end(), findCondition);
    if (moduleRecordIter != moduleInfoVectorIter->second.end()) {
        (*moduleRecordIter)->GetHapModuleInfo(info);
        (*moduleRecordIter)->SetModuleRecordState(ModuleRecordState::RUNNING_STATE);
        result = true;
    }

    return result;
}

void AppRunningRecord::SetRestartResidentProcCount(int count)
{
    restartResidentProcCount_ = count;
}

void AppRunningRecord::DecRestartResidentProcCount()
{
    restartResidentProcCount_--;
}

int AppRunningRecord::GetRestartResidentProcCount() const
{
    return restartResidentProcCount_;
}

bool AppRunningRecord::CanRestartResidentProc()
{
    struct timespec t;
    t.tv_sec = 0;
    t.tv_nsec = 0;
    clock_gettime(CLOCK_MONOTONIC, &t);
    int64_t systemTimeMillis = static_cast<int64_t>(((t.tv_sec) * NANOSECONDS + t.tv_nsec) / MICROSECONDS);
    if ((restartResidentProcCount_ >= 0) || ((systemTimeMillis - restartTimeMillis_) > RESTART_INTERVAL_TIME)) {
        return true;
    }
    return false;
}

void AppRunningRecord::GetBundleNames(std::vector<std::string> &bundleNames)
{
    std::lock_guard<ffrt::mutex> appInfosLock(appInfosLock_);
    for (auto &app : appInfos_) {
        bundleNames.emplace_back(app.first);
    }
}

void AppRunningRecord::SetUserTestInfo(const std::shared_ptr<UserTestRecord> &record)
{
    userTestRecord_ = record;
}

std::shared_ptr<UserTestRecord> AppRunningRecord::GetUserTestInfo()
{
    return userTestRecord_;
}

void AppRunningRecord::SetProcessAndExtensionType(
    const std::shared_ptr<AbilityInfo> &abilityInfo, uint32_t extensionProcessMode)
{
    if (abilityInfo == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null abilityInfo");
        return;
    }
    extensionType_ = abilityInfo->extensionAbilityType;
    preloadModuleName_ = abilityInfo->moduleName;
    if (extensionType_ == ExtensionAbilityType::UNSPECIFIED) {
        //record Service Ability in FA model as Service Extension
        if (abilityInfo->type == AbilityType::SERVICE) {
            processType_ = ProcessType::EXTENSION;
            extensionType_ = ExtensionAbilityType::SERVICE;
            return;
        }
        //record Data Ability in FA model as Datashare Extension
        if (abilityInfo->type == AbilityType::DATA) {
            processType_ = ProcessType::EXTENSION;
            extensionType_ = ExtensionAbilityType::DATASHARE;
            return;
        }
        processType_ = ProcessType::NORMAL;
        return;
    }

    if (AAFwk::UIExtensionWrapper::IsUIExtension(extensionType_) &&
         extensionProcessMode == PROCESS_MODE_RUN_WITH_MAIN_PROCESS) {
        processType_ = ProcessType::NORMAL;
    } else {
        processType_ = ProcessType::EXTENSION;
    }
    return;
}

void AppRunningRecord::SetStartupTaskData(const AAFwk::Want &want)
{
    std::lock_guard<ffrt::mutex> lock(startupTaskDataLock_);
    startupTaskData_ = std::make_shared<StartupTaskData>();
    startupTaskData_->uri = want.GetUriString();
    startupTaskData_->action = want.GetAction();
    const AppExecFwk::WantParams &wantParams = want.GetParams();
    if (wantParams.HasParam(AppExecFwk::INSIGHT_INTENT_EXECUTE_PARAM_NAME)) {
        startupTaskData_->insightIntentName = wantParams.GetStringParam(AppExecFwk::INSIGHT_INTENT_EXECUTE_PARAM_NAME);
    }
    TAG_LOGD(AAFwkTag::APPMGR, "uri: %{public}s, action: %{public}s, intentName: %{public}s",
        startupTaskData_->uri.c_str(), startupTaskData_->action.c_str(),
        startupTaskData_->insightIntentName.c_str());
}

void AppRunningRecord::SetSpecifiedAbilityFlagAndWant(
    int requestId, const AAFwk::Want &want, const std::string &moduleName)
{
    std::lock_guard lock(specifiedMutex_);
    if (specifiedAbilityRequest_ != nullptr) {
        TAG_LOGW(AAFwkTag::APPMGR, "specifiedRequestId: %{public}d", specifiedAbilityRequest_->requestId);
    }
    specifiedAbilityRequest_ = std::make_shared<SpecifiedRequest>();
    specifiedAbilityRequest_->requestId = requestId;
    specifiedAbilityRequest_->want = want;
    moduleName_ = moduleName;
}

int32_t AppRunningRecord::GetSpecifiedRequestId() const
{
    std::lock_guard lock(specifiedMutex_);
    if (specifiedAbilityRequest_ != nullptr) {
        return specifiedAbilityRequest_->requestId;
    }
    return -1;
}

void AppRunningRecord::ResetSpecifiedRequest()
{
    std::lock_guard lock(specifiedMutex_);
    specifiedAbilityRequest_.reset();
}

void AppRunningRecord::SetScheduleNewProcessRequestState(int32_t requestId,
    const AAFwk::Want &want, const std::string &moduleName)
{
    std::lock_guard lock(specifiedMutex_);
    if (specifiedProcessRequest_ != nullptr) {
        TAG_LOGW(AAFwkTag::APPMGR, "newProcessRequestId: %{public}d", specifiedProcessRequest_->requestId);
    }
    specifiedProcessRequest_ = std::make_shared<SpecifiedRequest>();
    specifiedProcessRequest_->requestId = requestId;
    specifiedProcessRequest_->want = want;
    moduleName_ = moduleName;
    specifiedProcessRequestId_ = requestId;
}

bool AppRunningRecord::IsNewProcessRequest() const
{
    std::lock_guard lock(specifiedMutex_);
    return specifiedProcessRequest_ != nullptr;
}

bool AppRunningRecord::IsStartSpecifiedAbility() const
{
    std::lock_guard lock(specifiedMutex_);
    return specifiedAbilityRequest_ != nullptr;
}

void AppRunningRecord::SchedulePrepareTerminate(const std::string &moduleName)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    if (appLifeCycleDeal_ == nullptr) {
        TAG_LOGW(AAFwkTag::APPMGR, "null appLifeCycleDeal_");
        return;
    }
    bool found = false;
    {
        std::lock_guard<ffrt::mutex> hapModulesLock(hapModulesLock_);
        for (const auto &iter : hapModules_) {
            for (const auto &moduleRecord : iter.second) {
                if (moduleRecord != nullptr && moduleName == moduleRecord->GetModuleName()) {
                    found = true;
                    break;
                }
            }
        }
    }
    if (!found) {
        TAG_LOGE(AAFwkTag::APPMGR, "moduleName not exist");
        return;
    }
    appLifeCycleDeal_->SchedulePrepareTerminate(moduleName);
}

void AppRunningRecord::ScheduleAcceptWant(const std::string &moduleName)
{
    auto timeOUt = AMSEventHandler::START_SPECIFIED_ABILITY_TIMEOUT *
        AppUtils::GetInstance().GetTimeoutUnitTimeRatio();
    SendEvent(AMSEventHandler::START_SPECIFIED_ABILITY_HALF_TIMEOUT_MSG, timeOUt / HALF_TIMEOUT);
    SendEvent(AMSEventHandler::START_SPECIFIED_ABILITY_TIMEOUT_MSG, timeOUt);
    if (appLifeCycleDeal_ == nullptr) {
        TAG_LOGW(AAFwkTag::APPMGR, "null appLifeCycleDeal_");
        return;
    }
    appLifeCycleDeal_->ScheduleAcceptWant(GetSpecifiedWant(), moduleName);
}

void AppRunningRecord::ScheduleAcceptWantDone()
{
    TAG_LOGI(AAFwkTag::APPMGR, "ScheduleAcceptWantDone, bundle %{public}s", mainBundleName_.c_str());
    RemoveEvent(AMSEventHandler::START_SPECIFIED_ABILITY_HALF_TIMEOUT_MSG);
    RemoveEvent(AMSEventHandler::START_SPECIFIED_ABILITY_TIMEOUT_MSG);
    SetModuleLoaded(moduleName_);
}

void AppRunningRecord::ScheduleNewProcessRequest(const AAFwk::Want &want, const std::string &moduleName)
{
    auto timeOUt = AMSEventHandler::START_SPECIFIED_PROCESS_TIMEOUT *
        AppUtils::GetInstance().GetTimeoutUnitTimeRatio();
    SendEvent(AMSEventHandler::START_SPECIFIED_PROCESS_HALF_TIMEOUT_MSG, timeOUt / HALF_TIMEOUT);
    SendEvent(AMSEventHandler::START_SPECIFIED_PROCESS_TIMEOUT_MSG, timeOUt);
    if (appLifeCycleDeal_ == nullptr) {
        TAG_LOGW(AAFwkTag::APPMGR, "null appLifeCycleDeal_");
        return;
    }
    appLifeCycleDeal_->ScheduleNewProcessRequest(want, moduleName);
}

void AppRunningRecord::ScheduleNewProcessRequestDone()
{
    TAG_LOGI(AAFwkTag::APPMGR, "bundle %{public}s", mainBundleName_.c_str());
    RemoveEvent(AMSEventHandler::START_SPECIFIED_PROCESS_HALF_TIMEOUT_MSG);
    RemoveEvent(AMSEventHandler::START_SPECIFIED_PROCESS_TIMEOUT_MSG);
    SetModuleLoaded(moduleName_);
}

void AppRunningRecord::ApplicationTerminated()
{
    TAG_LOGD(AAFwkTag::APPMGR, "Application terminated bundle %{public}s", mainBundleName_.c_str());

    RemoveEvent(AMSEventHandler::TERMINATE_APPLICATION_HALF_TIMEOUT_MSG);
    RemoveEvent(AMSEventHandler::TERMINATE_APPLICATION_TIMEOUT_MSG);
}

AAFwk::Want AppRunningRecord::GetSpecifiedWant() const
{
    std::lock_guard lock(specifiedMutex_);
    if (specifiedAbilityRequest_ != nullptr) {
        return specifiedAbilityRequest_->want;
    }
    return AAFwk::Want();
}

AAFwk::Want AppRunningRecord::GetNewProcessRequestWant() const
{
    std::lock_guard lock(specifiedMutex_);
    if (specifiedProcessRequest_ != nullptr) {
        return specifiedProcessRequest_->want;
    }
    return AAFwk::Want();
}

int32_t AppRunningRecord::GetNewProcessRequestId() const
{
    std::lock_guard lock(specifiedMutex_);
    if (specifiedProcessRequest_ != nullptr) {
        return specifiedProcessRequest_->requestId;
    }
    return -1;
}

void AppRunningRecord::ResetNewProcessRequest()
{
    std::lock_guard lock(specifiedMutex_);
    specifiedProcessRequest_.reset();
}

int32_t AppRunningRecord::UpdateConfiguration(const Configuration &config, ConfigUpdateReason reason)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (!appLifeCycleDeal_) {
        TAG_LOGI(AAFwkTag::APPMGR, "null appLifeCycleDeal_");
        return ERR_INVALID_VALUE;
    }
    return appLifeCycleDeal_->UpdateConfiguration(config, reason);
}

void AppRunningRecord::AddRenderRecord(const std::shared_ptr<RenderRecord> &record)
{
    if (!record) {
        TAG_LOGD(AAFwkTag::APPMGR, "AddRenderRecord: record is null");
        return;
    }
    {
        std::lock_guard renderPidSetLock(renderPidSetLock_);
        renderPidSet_.insert(record->GetPid());
    }
    std::lock_guard renderRecordMapLock(renderRecordMapLock_);
    renderRecordMap_.emplace(record->GetUid(), record);
}

void AppRunningRecord::RemoveRenderRecord(const std::shared_ptr<RenderRecord> &record)
{
    if (!record) {
        TAG_LOGD(AAFwkTag::APPMGR, "RemoveRenderRecord: record null");
        return;
    }
    std::lock_guard renderRecordMapLock(renderRecordMapLock_);
    renderRecordMap_.erase(record->GetUid());
}

void AppRunningRecord::RemoveRenderPid(pid_t renderPid)
{
    std::lock_guard renderPidSetLock(renderPidSetLock_);
    renderPidSet_.erase(renderPid);
}

void AppRunningRecord::GetRenderProcessInfos(std::list<SimpleProcessInfo> &processInfos)
{
    std::lock_guard renderRecordMapLock(renderRecordMapLock_);
    for (auto &item : renderRecordMap_) {
        auto renderRecord = item.second;
        if (renderRecord && renderRecord->GetPid() > 0) {
            auto processInfo = SimpleProcessInfo(renderRecord->GetPid(), renderRecord->GetProcessName());
            processInfos.emplace_back(processInfo);
        }
    }
}

bool AppRunningRecord::ConstainsRenderPid(pid_t renderPid)
{
    std::lock_guard renderPidSetLock(renderPidSetLock_);
    return renderPidSet_.find(renderPid) != renderPidSet_.end();
}

std::shared_ptr<RenderRecord> AppRunningRecord::GetRenderRecordByPid(const pid_t pid)
{
    std::lock_guard renderRecordMapLock(renderRecordMapLock_);
    if (renderRecordMap_.empty()) {
        return nullptr;
    }
    for (auto iter : renderRecordMap_) {
        auto renderRecord = iter.second;
        if (renderRecord && renderRecord->GetPid() == pid) {
            return renderRecord;
        }
    }
    return nullptr;
}

std::map<int32_t, std::shared_ptr<RenderRecord>> AppRunningRecord::GetRenderRecordMap()
{
    std::lock_guard renderRecordMapLock(renderRecordMapLock_);
    return renderRecordMap_;
}

void AppRunningRecord::SetStartMsg(const AppSpawnStartMsg &msg)
{
    startMsg_ = msg;
}

AppSpawnStartMsg AppRunningRecord::GetStartMsg()
{
    return startMsg_;
}

bool AppRunningRecord::IsDebug()
{
    if (IsDebugApp() || isNativeDebug_ || !perfCmd_.empty() || IsAttachDebug() || IsAssertionPause()) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "debugApp, no need to handle");
        return true;
    }
    return false;
}

void AppRunningRecord::SetDebugApp(bool isDebugApp)
{
    TAG_LOGD(AAFwkTag::APPMGR, "value is %{public}d", isDebugApp);
    isDebugApp_ = isDebugApp;
}

bool AppRunningRecord::IsDebugApp()
{
    return isDebugApp_;
}

void AppRunningRecord::SetNativeDebug(bool isNativeDebug)
{
    TAG_LOGD(AAFwkTag::APPMGR, "SetNativeDebug, value is %{public}d", isNativeDebug);
    isNativeDebug_ = isNativeDebug;
}

void AppRunningRecord::SetPerfCmd(const std::string &perfCmd)
{
    perfCmd_ = perfCmd;
}

void AppRunningRecord::SetErrorInfoEnhance(bool errorInfoEnhance)
{
    isErrorInfoEnhance_ = errorInfoEnhance;
}

void AppRunningRecord::SetMultiThread(bool multiThread)
{
    isMultiThread_ = multiThread;
}

void AppRunningRecord::SetAppIndex(const int32_t appIndex)
{
    appIndex_ = appIndex;
}

void AppRunningRecord::SetInstanceKey(const std::string& instanceKey)
{
    instanceKey_ = instanceKey;
}

void AppRunningRecord::GetSplitModeAndFloatingMode(bool &isSplitScreenMode, bool &isFloatingWindowMode)
{
    auto abilitiesMap = GetAbilities();
    isSplitScreenMode = false;
    isFloatingWindowMode = false;
    for (const auto &item : abilitiesMap) {
        const auto &abilityRecord = item.second;
        if (abilityRecord == nullptr) {
            continue;
        }
        const auto &abilityWant = abilityRecord->GetWant();
        if (abilityWant != nullptr) {
            int windowMode = abilityWant->GetIntParam(Want::PARAM_RESV_WINDOW_MODE, -1);
            if (windowMode == AAFwk::AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_FLOATING) {
                isFloatingWindowMode = true;
            }
            if (windowMode == AAFwk::AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_PRIMARY ||
                windowMode == AAFwk::AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_SECONDARY) {
                isSplitScreenMode = true;
            }
        }
        if (isFloatingWindowMode && isSplitScreenMode) {
            break;
        }
    }
}

int32_t AppRunningRecord::GetAppIndex() const
{
    return appIndex_;
}

std::string AppRunningRecord::GetInstanceKey() const
{
    return instanceKey_;
}

void AppRunningRecord::SetSecurityFlag(bool securityFlag)
{
    securityFlag_ = securityFlag;
}

bool AppRunningRecord::GetSecurityFlag() const
{
    return securityFlag_;
}

void AppRunningRecord::SetKilling()
{
    isKilling_.store(true);
}

bool AppRunningRecord::IsKilling() const
{
    return isKilling_.load();
}

void AppRunningRecord::SetPreForeground(bool isPreForeground)
{
    isPreForeground_.store(isPreForeground);
}

bool AppRunningRecord::IsPreForeground() const
{
    return isPreForeground_.load();
}

bool AppRunningRecord::NeedUpdateConfigurationBackground()
{
    bool needUpdate = false;
    auto abilitiesMap = GetAbilities();
    for (const auto &item : abilitiesMap) {
        const auto &abilityRecord = item.second;
        if (!abilityRecord || !abilityRecord->GetAbilityInfo()) {
            continue;
        }
        if (abilityRecord->GetAbilityInfo()->type != AppExecFwk::AbilityType::PAGE &&
            !(AAFwk::UIExtensionWrapper::IsUIExtension(abilityRecord->GetAbilityInfo()->extensionAbilityType))) {
            needUpdate = true;
            break;
        }
    }
    return needUpdate;
}

void AppRunningRecord::RemoveTerminateAbilityTimeoutTask(const sptr<IRemoteObject>& token) const
{
    auto moduleRecord = GetModuleRunningRecordByToken(token);
    if (!moduleRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "null moduleRecord");
        return;
    }
    (void)moduleRecord->RemoveTerminateAbilityTimeoutTask(token);
}

int32_t AppRunningRecord::NotifyLoadRepairPatch(const std::string &bundleName, const sptr<IQuickFixCallback> &callback,
    const int32_t recordId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    if (!appLifeCycleDeal_) {
        TAG_LOGE(AAFwkTag::APPMGR, "null appLifeCycleDeal_");
        return ERR_INVALID_VALUE;
    }
    return appLifeCycleDeal_->NotifyLoadRepairPatch(bundleName, callback, recordId);
}

int32_t AppRunningRecord::NotifyHotReloadPage(const sptr<IQuickFixCallback> &callback, const int32_t recordId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    if (!appLifeCycleDeal_) {
        TAG_LOGE(AAFwkTag::APPMGR, "null appLifeCycleDeal_");
        return ERR_INVALID_VALUE;
    }
    return appLifeCycleDeal_->NotifyHotReloadPage(callback, recordId);
}

int32_t AppRunningRecord::NotifyUnLoadRepairPatch(const std::string &bundleName,
    const sptr<IQuickFixCallback> &callback, const int32_t recordId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    if (!appLifeCycleDeal_) {
        TAG_LOGE(AAFwkTag::APPMGR, "null appLifeCycleDeal_");
        return ERR_INVALID_VALUE;
    }
    return appLifeCycleDeal_->NotifyUnLoadRepairPatch(bundleName, callback, recordId);
}

int32_t AppRunningRecord::NotifyAppFault(const FaultData &faultData)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    if (!appLifeCycleDeal_) {
        TAG_LOGE(AAFwkTag::APPMGR, "null appLifeCycleDeal_");
        return ERR_INVALID_VALUE;
    }
    return appLifeCycleDeal_->NotifyAppFault(faultData);
}

bool AppRunningRecord::IsAbilitiesBackground()
{
    std::lock_guard<ffrt::mutex> hapModulesLock(hapModulesLock_);
    for (const auto &iter : hapModules_) {
        for (const auto &moduleRecord : iter.second) {
            if (moduleRecord == nullptr) {
                TAG_LOGE(AAFwkTag::APPMGR, "null moduleRecord");
                continue;
            }
            if (!moduleRecord->IsAbilitiesBackgrounded()) {
                return false;
            }
        }
    }
    return true;
}

#ifdef SUPPORT_SCREEN
void AppRunningRecord::ChangeWindowVisibility(const sptr<OHOS::Rosen::WindowVisibilityInfo> &info)
{
    if (info == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null info");
        return;
    }

    if (GetPriorityObject() == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null priorityObject");
        return;
    }

    if (info->pid_ != GetPid()) {
        return;
    }

    std::lock_guard windowIdsLock(windowIdsLock_);
    auto iter = windowIds_.find(info->windowId_);
    if (iter != windowIds_.end() &&
        info->visibilityState_ == OHOS::Rosen::WindowVisibilityState::WINDOW_VISIBILITY_STATE_TOTALLY_OCCUSION) {
        windowIds_.erase(iter);
        return;
    }
    if (iter == windowIds_.end() &&
        info->visibilityState_ < OHOS::Rosen::WindowVisibilityState::WINDOW_VISIBILITY_STATE_TOTALLY_OCCUSION) {
        windowIds_.emplace(info->windowId_);
    }
}

void AppRunningRecord::OnWindowVisibilityChanged(
    const std::vector<sptr<OHOS::Rosen::WindowVisibilityInfo>> &windowVisibilityInfos)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    AddAppLifecycleEvent("AppRunningRecord::OnWindowVisibilityChanged");
    if (windowVisibilityInfos.empty()) {
        TAG_LOGW(AAFwkTag::APPMGR, "empty info");
        return;
    }

    for (const auto &info : windowVisibilityInfos) {
        if (info == nullptr || info->pid_ != GetPid()) {
            TAG_LOGD(AAFwkTag::APPMGR, "null info or info pid is not matched");
            continue;
        }
        std::lock_guard windowIdsLock(windowIdsLock_);
        auto iter = windowIds_.find(info->windowId_);
        if (iter != windowIds_.end() &&
            info->visibilityState_ == OHOS::Rosen::WindowVisibilityState::WINDOW_VISIBILITY_STATE_TOTALLY_OCCUSION) {
            windowIds_.erase(iter);
            continue;
        }
        if (iter == windowIds_.end() &&
            info->visibilityState_ < OHOS::Rosen::WindowVisibilityState::WINDOW_VISIBILITY_STATE_TOTALLY_OCCUSION) {
            windowIds_.emplace(info->windowId_);
        }
    }
    OnWindowVisibilityChangedWithPendingState();
}

void AppRunningRecord::OnWindowVisibilityChangedWithPendingState()
{
    TAG_LOGI(AAFwkTag::APPMGR, "wnd call, %{public}s_%{public}d, isEmpty_%{public}d, c_%{public}d -> p_%{public}d",
        GetBundleName().c_str(), GetPid(), IsWindowIdsEmpty(), GetState(), pendingState_);
    if (pendingState_ == ApplicationPendingState::READY) {
        if (!IsWindowIdsEmpty()) {
            if (GetState() != ApplicationState::APP_STATE_FOREGROUND) {
                SetApplicationPendingState(ApplicationPendingState::FOREGROUNDING);
                ScheduleForegroundRunning();
            }
            if (watchdogVisibilityState_ != WatchdogVisibilityState::WATCHDOG_STATE_VISIBILITY) {
                watchdogVisibilityState_ = WatchdogVisibilityState::WATCHDOG_STATE_VISIBILITY;
                SetWatchdogBackgroundStatusRunning(false);
            }
        } else {
            if (IsAbilitiesBackground() && GetState() == ApplicationState::APP_STATE_FOREGROUND) {
                SetApplicationPendingState(ApplicationPendingState::BACKGROUNDING);
                ScheduleBackgroundRunning();
            }
            if (watchdogVisibilityState_ != WatchdogVisibilityState::WATCHDOG_STATE_UNVISIBILITY) {
                watchdogVisibilityState_ = WatchdogVisibilityState::WATCHDOG_STATE_UNVISIBILITY;
                SetWatchdogBackgroundStatusRunning(true);
            }
        }
    } else {
        if (!IsWindowIdsEmpty()) {
            SetApplicationPendingState(ApplicationPendingState::FOREGROUNDING);
        }
        if (IsWindowIdsEmpty() && IsAbilitiesBackground() && foregroundingAbilityTokens_.empty()) {
            SetApplicationPendingState(ApplicationPendingState::BACKGROUNDING);
        }
    }
}
#endif //SUPPORT_SCREEN

bool AppRunningRecord::IsWindowIdsEmpty()
{
    std::lock_guard windowIdsLock(windowIdsLock_);
    return windowIds_.empty();
}

bool AppRunningRecord::IsContinuousTask()
{
    return isContinuousTask_;
}

void AppRunningRecord::SetContinuousTaskAppState(bool isContinuousTask)
{
    isContinuousTask_ = isContinuousTask;
}

bool AppRunningRecord::GetFocusFlag() const
{
    return isFocused_;
}

int64_t AppRunningRecord::GetAppStartTime() const
{
    return startTimeMillis_;
}

uint64_t AppRunningRecord::GetAppRunningUniqueId() const
{
    return appRunningUniqueId_;
}

uint64_t AppRunningRecord::GenerateRunningId()
{
    std::random_device rd;
    std::mt19937_64 engine(rd());
    std::uniform_int_distribution<uint64_t> dist(
        std::numeric_limits<uint64_t>::min(),
        std::numeric_limits<uint64_t>::max()
    );
    return dist(engine);
}

void AppRunningRecord::SetRequestProcCode(int32_t requestProcCode)
{
    requestProcCode_ = requestProcCode;
}

int32_t AppRunningRecord::GetRequestProcCode() const
{
    return requestProcCode_;
}

void AppRunningRecord::SetProcessChangeReason(ProcessChangeReason reason)
{
    processChangeReason_ = reason;
}

ProcessChangeReason AppRunningRecord::GetProcessChangeReason() const
{
    return processChangeReason_;
}

ExtensionAbilityType AppRunningRecord::GetExtensionType() const
{
    return extensionType_;
}

ProcessType AppRunningRecord::GetProcessType() const
{
    std::shared_lock<std::shared_mutex> lock(processTypeLock_);
    return processType_;
}

std::map<pid_t, std::weak_ptr<AppRunningRecord>> AppRunningRecord::GetChildAppRecordMap() const
{
    return childAppRecordMap_;
}

void AppRunningRecord::AddChildAppRecord(pid_t pid, std::shared_ptr<AppRunningRecord> appRecord)
{
    childAppRecordMap_[pid] = appRecord;
}

void AppRunningRecord::RemoveChildAppRecord(pid_t pid)
{
    childAppRecordMap_.erase(pid);
}

void AppRunningRecord::ClearChildAppRecordMap()
{
    childAppRecordMap_.clear();
}

void AppRunningRecord::SetParentAppRecord(std::shared_ptr<AppRunningRecord> appRecord)
{
    parentAppRecord_ = appRecord;
}

std::shared_ptr<AppRunningRecord> AppRunningRecord::GetParentAppRecord()
{
    return parentAppRecord_.lock();
}

void AppRunningRecord::SetProcessType(ProcessType processType)
{
    std::unique_lock<std::shared_mutex> lock(processTypeLock_);
    processType_ = processType;
}

int32_t AppRunningRecord::ChangeAppGcState(int32_t state, uint64_t tid)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    if (appLifeCycleDeal_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null appLifeCycleDeal_");
        return ERR_INVALID_VALUE;
    }
    return appLifeCycleDeal_->ChangeAppGcState(state, tid);
}

void AppRunningRecord::SetAttachDebug(bool isAttachDebug, bool isDebugFromLocal)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    isAttachDebug_ = isAttachDebug;
    isDebugFromLocal_ = isDebugFromLocal;

    if (appLifeCycleDeal_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null appLifeCycleDeal_");
        return;
    }
    isAttachDebug_ ? appLifeCycleDeal_->AttachAppDebug(isDebugFromLocal_) : appLifeCycleDeal_->DetachAppDebug();
}

bool AppRunningRecord::IsAttachDebug() const
{
    return isAttachDebug_;
}

void AppRunningRecord::SetApplicationPendingState(ApplicationPendingState pendingState)
{
    pendingState_ = pendingState;
}

ApplicationPendingState AppRunningRecord::GetApplicationPendingState() const
{
    return pendingState_;
}

void AppRunningRecord::SetApplicationScheduleState(ApplicationScheduleState scheduleState)
{
    scheduleState_ = scheduleState;
}

ApplicationScheduleState AppRunningRecord::GetApplicationScheduleState() const
{
    return scheduleState_;
}

#ifdef SUPPORT_CHILD_PROCESS
void AppRunningRecord::AddChildProcessRecord(pid_t pid, std::shared_ptr<ChildProcessRecord> record)
{
    if (!record) {
        TAG_LOGE(AAFwkTag::APPMGR, "null record");
        return;
    }
    if (pid <= 0) {
        TAG_LOGE(AAFwkTag::APPMGR, "pid <= 0");
        return;
    }
    std::lock_guard lock(childProcessRecordMapLock_);
    childProcessRecordMap_.emplace(pid, record);
}

void AppRunningRecord::RemoveChildProcessRecord(std::shared_ptr<ChildProcessRecord> record)
{
    if (!record) {
        TAG_LOGE(AAFwkTag::APPMGR, "null record");
        return;
    }
    TAG_LOGI(AAFwkTag::APPMGR, "pid: %{public}d", record->GetPid());
    auto pid = record->GetPid();
    if (pid <= 0) {
        TAG_LOGE(AAFwkTag::APPMGR, "pid <= 0");
        return;
    }
    std::lock_guard lock(childProcessRecordMapLock_);
    childProcessRecordMap_.erase(pid);
}

std::shared_ptr<ChildProcessRecord> AppRunningRecord::GetChildProcessRecordByPid(pid_t pid)
{
    std::lock_guard lock(childProcessRecordMapLock_);
    auto iter = childProcessRecordMap_.find(pid);
    if (iter == childProcessRecordMap_.end()) {
        return nullptr;
    }
    return iter->second;
}

std::map<int32_t, std::shared_ptr<ChildProcessRecord>> AppRunningRecord::GetChildProcessRecordMap()
{
    std::lock_guard lock(childProcessRecordMapLock_);
    return childProcessRecordMap_;
}

int32_t AppRunningRecord::GetChildProcessCount()
{
    std::lock_guard lock(childProcessRecordMapLock_);
    return childProcessRecordMap_.size();
}

void AppRunningRecord::GetChildProcessInfos(std::list<SimpleProcessInfo> &processInfos)
{
    std::lock_guard lock(childProcessRecordMapLock_);
    for (auto &iter : childProcessRecordMap_) {
        auto childRecord = iter.second;
        if (childRecord && childRecord->GetPid() > 0) {
            auto processInfo = SimpleProcessInfo(childRecord->GetPid(), childRecord->GetProcessName());
            processInfos.emplace_back(processInfo);
        }
    }
}
#endif //SUPPORT_CHILD_PROCESS

void AppRunningRecord::SetJITEnabled(const bool jitEnabled)
{
    jitEnabled_ = jitEnabled;
}

bool AppRunningRecord::IsJITEnabled() const
{
    return jitEnabled_;
}

void AppRunningRecord::SetPreloadMode(PreloadMode mode)
{
    preloadMode_ = mode;
}

PreloadMode AppRunningRecord::GetPreloadMode()
{
    return preloadMode_;
}

void AppRunningRecord::SetPreloadModuleName(const std::string& preloadModuleName)
{
    preloadModuleName_ = preloadModuleName;
}

std::string AppRunningRecord::GetPreloadModuleName() const
{
    return preloadModuleName_;
}

void AppRunningRecord::SetPreloadState(PreloadState state)
{
    preloadState_ = state;
}

void AppRunningRecord::SetPreloadPhase(PreloadPhase phase)
{
    preloadPhase_ = phase;
}

PreloadPhase AppRunningRecord::GetPreloadPhase()
{
    return preloadPhase_;
}

bool AppRunningRecord::IsPreloading() const
{
    return preloadState_ == PreloadState::PRELOADING;
}

bool AppRunningRecord::IsPreloaded() const
{
    return preloadState_ == PreloadState::PRELOADED;
}

int32_t AppRunningRecord::GetAssignTokenId() const
{
    return assignTokenId_;
}

void AppRunningRecord::SetAssignTokenId(int32_t assignTokenId)
{
    assignTokenId_ = assignTokenId;
}

void AppRunningRecord::SetRestartAppFlag(bool isRestartApp)
{
    isRestartApp_ = isRestartApp;
}

bool AppRunningRecord::GetRestartAppFlag() const
{
    return isRestartApp_;
}

void AppRunningRecord::SetAssertionPauseFlag(bool flag)
{
    isAssertPause_ = flag;
}

bool AppRunningRecord::IsAssertionPause() const
{
    return isAssertPause_;
}

bool AppRunningRecord::IsDebugging() const
{
    return isDebugApp_ || isAssertPause_;
}

void AppRunningRecord::SetNativeStart(bool isNativeStart)
{
    isNativeStart_ = isNativeStart;
}

bool AppRunningRecord::isNativeStart() const
{
    return isNativeStart_;
}

void AppRunningRecord::SetExitReason(int32_t reason)
{
    exitReason_ = reason;
}

int32_t AppRunningRecord::GetExitReason() const
{
    return exitReason_;
}

void AppRunningRecord::SetExitMsg(const std::string &exitMsg)
{
    exitMsg_ = exitMsg;
}

std::string AppRunningRecord::GetExitMsg() const
{
    return exitMsg_;
}

void AppRunningRecord::SetKillId(int32_t killId)
{
    killId_ = killId;
}

int32_t AppRunningRecord::GetKillId() const
{
    return killId_;
}

void AppRunningRecord::SetKillMsg(const std::string &killMsg)
{
    killMsg_ = killMsg;
}

std::string AppRunningRecord::GetKillMsg() const
{
    return killMsg_;
}

void AppRunningRecord::SetInnerMsg(const std::string &innerMsg)
{
    innerMsg_ = innerMsg;
}

std::string AppRunningRecord::GetInnerMsg() const
{
    return innerMsg_;
}

int AppRunningRecord::DumpIpcStart(std::string& result)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    if (appLifeCycleDeal_ == nullptr) {
        result.append(MSG_DUMP_IPC_START_STAT, strlen(MSG_DUMP_IPC_START_STAT))
            .append(MSG_DUMP_FAIL, strlen(MSG_DUMP_FAIL))
            .append(MSG_DUMP_FAIL_REASON_INTERNAL, strlen(MSG_DUMP_FAIL_REASON_INTERNAL));
        TAG_LOGE(AAFwkTag::APPMGR, "null appLifeCycleDeal_");
        return DumpErrorCode::ERR_INTERNAL_ERROR;
    }
    return appLifeCycleDeal_->DumpIpcStart(result);
}

int AppRunningRecord::DumpIpcStop(std::string& result)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    if (appLifeCycleDeal_ == nullptr) {
        result.append(MSG_DUMP_IPC_STOP_STAT, strlen(MSG_DUMP_IPC_STOP_STAT))
            .append(MSG_DUMP_FAIL, strlen(MSG_DUMP_FAIL))
            .append(MSG_DUMP_FAIL_REASON_INTERNAL, strlen(MSG_DUMP_FAIL_REASON_INTERNAL));
        TAG_LOGE(AAFwkTag::APPMGR, "null appLifeCycleDeal_");
        return DumpErrorCode::ERR_INTERNAL_ERROR;
    }
    return appLifeCycleDeal_->DumpIpcStop(result);
}

int AppRunningRecord::DumpIpcStat(std::string& result)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    if (appLifeCycleDeal_ == nullptr) {
        result.append(MSG_DUMP_IPC_STAT, strlen(MSG_DUMP_IPC_STAT))
            .append(MSG_DUMP_FAIL, strlen(MSG_DUMP_FAIL))
            .append(MSG_DUMP_FAIL_REASON_INTERNAL, strlen(MSG_DUMP_FAIL_REASON_INTERNAL));
        TAG_LOGE(AAFwkTag::APPMGR, "null appLifeCycleDeal_");
        return DumpErrorCode::ERR_INTERNAL_ERROR;
    }
    return appLifeCycleDeal_->DumpIpcStat(result);
}

int AppRunningRecord::DumpFfrt(std::string& result)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    if (appLifeCycleDeal_ == nullptr) {
        result.append(MSG_DUMP_FAIL, strlen(MSG_DUMP_FAIL))
            .append(MSG_DUMP_FAIL_REASON_INTERNAL, strlen(MSG_DUMP_FAIL_REASON_INTERNAL));
        TAG_LOGE(AAFwkTag::APPMGR, "null appLifeCycleDeal_");
        return DumpErrorCode::ERR_INTERNAL_ERROR;
    }
    return appLifeCycleDeal_->DumpFfrt(result);
}

int32_t AppRunningRecord::DumpArkWeb(const std::string &customArgs, std::string &result)
{
    TAG_LOGD(AAFwkTag::APPMGR, "DumpArkWeb called");
    if (appLifeCycleDeal_ == nullptr) {
        result.append(MSG_DUMP_FAIL, strlen(MSG_DUMP_FAIL))
            .append(MSG_DUMP_FAIL_REASON_INTERNAL, strlen(MSG_DUMP_FAIL_REASON_INTERNAL));
        TAG_LOGE(AAFwkTag::APPMGR, "null appLifeCycleDeal_");
        return DumpErrorCode::ERR_INTERNAL_ERROR;
    }
    return appLifeCycleDeal_->DumpArkWeb(customArgs, result);
}

void AppRunningRecord::SetWatchdogBackgroundStatusRunning(bool status)
{
    if (appLifeCycleDeal_) {
        appLifeCycleDeal_->SetWatchdogBackgroundStatusRunning(status);
    }
}

bool AppRunningRecord::SetSupportedProcessCache(bool isSupport)
{
    TAG_LOGD(AAFwkTag::APPMGR, "call");
    procCacheSupportState_ = isSupport ? SupportProcessCacheState::SUPPORT : SupportProcessCacheState::NOT_SUPPORT;
    return true;
}

bool AppRunningRecord::SetEnableProcessCache(bool enable)
{
    TAG_LOGD(AAFwkTag::APPMGR, "call");
    enableProcessCache_ = enable;
    return true;
}

bool AppRunningRecord::GetEnableProcessCache()
{
    return enableProcessCache_;
}

SupportProcessCacheState AppRunningRecord::GetSupportProcessCacheState()
{
    return procCacheSupportState_;
}

void AppRunningRecord::ScheduleCacheProcess()
{
    if (appLifeCycleDeal_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null appLifeCycleDeal_");
        return;
    }
    appLifeCycleDeal_->ScheduleCacheProcess();
}

bool AppRunningRecord::CancelTask(std::string msg)
{
    if (!taskHandler_) {
        TAG_LOGE(AAFwkTag::APPMGR, "null taskHandler_");
        return false;
    }
    return taskHandler_->CancelTask(msg);
}

void AppRunningRecord::SetBrowserHost(sptr<IRemoteObject> browser)
{
    browserHost_ = browser;
}

sptr<IRemoteObject> AppRunningRecord::GetBrowserHost()
{
    return browserHost_;
}

void AppRunningRecord::SetHasGPU(bool gpu)
{
    if (gpu) {
        hasGPU_ = gpu;
    }
}

bool AppRunningRecord::HasGPU()
{
    return hasGPU_;
}

void AppRunningRecord::SetGPUPid(pid_t gpuPid)
{
    gpuPid_ = gpuPid;
}

pid_t AppRunningRecord::GetGPUPid()
{
    return gpuPid_;
}

pid_t AppRunningRecord::GetPid()
{
    auto prioObj = GetPriorityObject();
    if (prioObj) {
        return prioObj->GetPid();
    }
    return 0;
}

void AppRunningRecord::SetAttachedToStatusBar(bool isAttached)
{
    isAttachedToStatusBar = isAttached;
}

bool AppRunningRecord::IsAttachedToStatusBar()
{
    return isAttachedToStatusBar;
}

void AppRunningRecord::SetProcessCacheBlocked(bool isBlocked)
{
    processCacheBlocked = isBlocked;
}

bool AppRunningRecord::GetProcessCacheBlocked()
{
    return processCacheBlocked;
}

void AppRunningRecord::SetProcessCacheLocked(bool isLock)
{
    processCacheLocked_.store(isLock);
}

bool AppRunningRecord::GetProcessCacheLocked()
{
    return processCacheLocked_.load();
}

bool AppRunningRecord::IsAllAbilityReadyToCleanedByUserRequest()
{
    std::lock_guard<ffrt::mutex> lock(hapModulesLock_);
    for (const auto &iter : hapModules_) {
        for (const auto &moduleRecord : iter.second) {
            if (moduleRecord == nullptr) {
                TAG_LOGE(AAFwkTag::APPMGR, "null moduleRecord");
                continue;
            }
            if (!moduleRecord->IsAllAbilityReadyToCleanedByUserRequest()) {
                return false;
            }
        }
    }
    return true;
}

void AppRunningRecord::SetUserRequestCleaning()
{
    isUserRequestCleaning_ = true;
}

bool AppRunningRecord::IsUserRequestCleaning() const
{
    return isUserRequestCleaning_;
}

bool AppRunningRecord::IsProcessAttached() const
{
    if (appLifeCycleDeal_ == nullptr) {
        return false;
    }
    return appLifeCycleDeal_->GetApplicationClient() != nullptr;
}

void AppRunningRecord::SetUIAbilityLaunched(bool hasLaunched)
{
    hasUIAbilityLaunched_ = hasLaunched;
}

bool AppRunningRecord::HasUIAbilityLaunched()
{
    return hasUIAbilityLaunched_;
}

void AppRunningRecord::SetProcessCaching(bool isCaching)
{
    isCaching_ = isCaching;
}

bool AppRunningRecord::IsCaching()
{
    return isCaching_;
}

void AppRunningRecord::AddAppLifecycleEvent(const std::string &msg)
{
    pid_t pid = GetPid();
    if (pid != 0) {
        AbilityRuntime::FreezeUtil::GetInstance().AddAppLifecycleEvent(pid, msg);
    }
}

void AppRunningRecord::SetNeedPreloadModule(bool isNeedPreloadModule)
{
    isNeedPreloadModule_ = isNeedPreloadModule;
}

bool AppRunningRecord::GetNeedPreloadModule()
{
    return isNeedPreloadModule_;
}

void AppRunningRecord::SetNWebPreload(const bool isAllowedNWebPreload)
{
    isAllowedNWebPreload_ = isAllowedNWebPreload;
}

bool AppRunningRecord::IsNWebPreload() const
{
    return isAllowedNWebPreload_;
}

void AppRunningRecord::SetIsUnSetPermission(bool isUnSetPermission)
{
    isUnSetPermission_ = isUnSetPermission;
}

bool AppRunningRecord::IsUnSetPermission()
{
    return isUnSetPermission_;
}

bool AppRunningRecord::GetNeedLimitPrio()
{
    return isNeedLimitPrio_;
}

void AppRunningRecord::SetNeedLimitPrio(bool isNeedLimitPrio)
{
    isNeedLimitPrio_ = isNeedLimitPrio;
}

void AppRunningRecord::UnSetPolicy()
{
    TAG_LOGD(AAFwkTag::APPMGR, "UnSetPolicy call");
    auto appInfo = GetApplicationInfo();
    if (appInfo == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "appInfo  null");
        return;
    }
    if (IsUnSetPermission()) {
        TAG_LOGI(AAFwkTag::APPMGR, "app is unset permission");
        return;
    }
    SetIsUnSetPermission(true);
#ifdef SUPPORT_UPMS
    AAFwk::UriPermissionManagerClient::GetInstance().ClearPermissionTokenByMap(appInfo->accessTokenId);
#endif // SUPPORT_UPMS
}

int32_t AppRunningRecord::GetAddStageTimeout() const
{
    if (IsEmptyKeepAliveApp()) {
        return AMSEventHandler::ADD_ABILITY_STAGE_EMPTY_RESIDENT_TIMEOUT;
    }
    return AMSEventHandler::ADD_ABILITY_STAGE_INFO_TIMEOUT;
}

void AppRunningRecord::SetDebugFromLocal(bool isDebugFromLocal)
{
    isDebugFromLocal_ = isDebugFromLocal;
}

std::optional<bool> AppRunningRecord::IsSupportMultiProcessDeviceFeature() const
{
    std::lock_guard<ffrt::mutex> supportLock(supportMultiProcessDeviceFeatureLock_);
    return supportMultiProcessDeviceFeature_;
}

void AppRunningRecord::SetSupportMultiProcessDeviceFeature(bool support)
{
    std::lock_guard<ffrt::mutex> supportLock(supportMultiProcessDeviceFeatureLock_);
    supportMultiProcessDeviceFeature_ = support;
}
}  // namespace AppExecFwk
}  // namespace OHOS
