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

#include "keep_alive_process_manager.h"

#include <utility>

#include "ability_util.h"
#include "ffrt.h"
#include "ipc_skeleton.h"
#include "keep_alive_utils.h"
#include "main_element_utils.h"
#include "parameters.h"
#include "permission_constants.h"
#include "process_options.h"
#include "user_controller/user_controller.h"

namespace OHOS {
namespace AAFwk {
namespace {
constexpr char PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED[] = "const.product.enterprisefeature.setting.enabled";
constexpr char FOUNDATION_PROCESS_NAME[] = "foundation";
constexpr int MAX_RETRY_TIMES = 3;
constexpr int RETRY_INTERVAL_MICRO_SECONDS = 200000; // 200ms
constexpr int CREATE_STATUS_BAR_TIMEOUT_MICRO_SECONDS = 5000000; // 5s
} // namespace

void CheckStatusBarTask::Cancel()
{
    std::lock_guard<ffrt::mutex> lock(cancelMutex_);
    task_ = nullptr;
}

void CheckStatusBarTask::Run()
{
    std::lock_guard<ffrt::mutex> lock(cancelMutex_);
    if (task_ == nullptr) {
        TAG_LOGI(AAFwkTag::KEEP_ALIVE, "task is canceled");
        return;
    }

    TAG_LOGI(AAFwkTag::KEEP_ALIVE, "run check task");
    task_();
}

KeepAliveProcessManager &KeepAliveProcessManager::GetInstance()
{
    static KeepAliveProcessManager instance;
    return instance;
}

KeepAliveProcessManager::KeepAliveProcessManager() {}

KeepAliveProcessManager::~KeepAliveProcessManager() {}

void KeepAliveProcessManager::StartKeepAliveProcessWithMainElement(std::vector<AppExecFwk::BundleInfo> &bundleInfos,
    int32_t userId)
{
    if (!DelayedSingleton<AbilityManagerService>::GetInstance()->IsSceneBoardReady(userId)) {
        TAG_LOGE(AAFwkTag::KEEP_ALIVE, "SCB not ready, do not start keep-alive apps");
        return;
    }
    for (const auto &bundleInfo : bundleInfos) {
        StartKeepAliveProcessWithMainElementPerBundle(bundleInfo, userId);
    }
}

void KeepAliveProcessManager::StartKeepAliveProcessWithMainElementPerBundle(const AppExecFwk::BundleInfo &bundleInfo,
    int32_t userId)
{
    if (!IsKeepAliveBundle(bundleInfo.name, userId)) {
        TAG_LOGE(AAFwkTag::KEEP_ALIVE, "bundle is not set keep-alive");
        return;
    }

    std::string mainElementName;
    if (!MainElementUtils::CheckMainUIAbility(bundleInfo, mainElementName)) {
        TAG_LOGE(AAFwkTag::KEEP_ALIVE, "bundle has no main uiability");
        return;
    }
    KeepAliveAbilityInfo info = {
        .userId = userId,
        .appCloneIndex = bundleInfo.appIndex,
        .uid = bundleInfo.uid,
        .bundleName = bundleInfo.name,
        .moduleName = bundleInfo.entryModuleName,
        .abilityName = mainElementName,
    };
    auto isMultiInstance =
        bundleInfo.applicationInfo.multiAppMode.multiAppModeType == AppExecFwk::MultiAppModeType::MULTI_INSTANCE;
    auto ret = StartKeepAliveMainAbility(info);
    if (ret == ERR_OK) {
        TAG_LOGI(AAFwkTag::KEEP_ALIVE, "start ok");
        AfterStartKeepAliveApp(bundleInfo.name, bundleInfo.applicationInfo.accessTokenId,
            bundleInfo.uid, userId, isMultiInstance);
        return;
    }

    TAG_LOGE(AAFwkTag::KEEP_ALIVE, "StartKeepAliveMainAbility failed:%{public}d, retry", ret);
    ffrt::submit([bundleName = bundleInfo.name, accessTokenId = bundleInfo.applicationInfo.accessTokenId,
        uid = bundleInfo.uid, userId, info, ret, isMultiInstance]() mutable {
        for (int tried = 0; tried < MAX_RETRY_TIMES && ret != ERR_OK; tried++) {
            usleep(RETRY_INTERVAL_MICRO_SECONDS);
            TAG_LOGI(AAFwkTag::KEEP_ALIVE, "retry attempt:%{public}d", tried + 1);
            ret = KeepAliveProcessManager::GetInstance().StartKeepAliveMainAbility(info);
            TAG_LOGI(AAFwkTag::KEEP_ALIVE, "retry result:%{public}d", ret);
        }
        if (ret != ERR_OK) {
            TAG_LOGW(AAFwkTag::KEEP_ALIVE, "reach max retry, failed:%{public}d", ret);
            return;
        }
        KeepAliveProcessManager::GetInstance().AfterStartKeepAliveApp(bundleName, accessTokenId, uid, userId,
            isMultiInstance);
    });
}

int32_t KeepAliveProcessManager::StartKeepAliveMainAbility(const KeepAliveAbilityInfo &info)
{
    Want want;
    want.SetElementName(info.bundleName, info.abilityName);
    want.SetParam(Want::PARAM_APP_CLONE_INDEX_KEY, info.appCloneIndex);
    TAG_LOGI(AAFwkTag::KEEP_ALIVE, "call, bundleName: %{public}s, moduleName: %{public}s, mainElement: %{public}s"
        " appCloneIndex: %{public}d", info.bundleName.c_str(), info.moduleName.c_str(), info.abilityName.c_str(),
        info.appCloneIndex);
    StartOptions options;
    if (DelayedSingleton<AbilityManagerService>::GetInstance()->IsSupportStatusBar(info.uid)) {
        options.processOptions = std::make_shared<ProcessOptions>();
        options.processOptions->isRestartKeepAlive = true;
        options.processOptions->startupVisibility = StartupVisibility::STARTUP_HIDE;
    }
    auto ret = IN_PROCESS_CALL(DelayedSingleton<AbilityManagerService>::GetInstance()->StartAbility(want,
        options, nullptr, info.userId, DEFAULT_INVAL_VALUE));
    MainElementUtils::UpdateMainElement(info.bundleName, info.moduleName, info.abilityName, true, info.userId);
    return ret;
}

void KeepAliveProcessManager::AfterStartKeepAliveApp(const std::string &bundleName,
    uint32_t accessTokenId, int32_t uid, int32_t userId, bool isMultiInstance)
{
    // not support statusbar and don't need check after 5s
    if (!DelayedSingleton<AbilityManagerService>::GetInstance()->IsSupportStatusBar(uid)) {
        TAG_LOGI(AAFwkTag::KEEP_ALIVE, "not support statusBar, don't need check when keep alive");
        return;
    }
    auto task = [bundleName, accessTokenId, uid, userId, isMultiInstance]() {
        bool isStatusBarCreated =
            DelayedSingleton<AbilityManagerService>::GetInstance()->IsInStatusBar(accessTokenId, uid, isMultiInstance);
        (void)KeepAliveProcessManager::GetInstance().RemoveCheckStatusBarTask(uid, false);
        if (isStatusBarCreated) {
            TAG_LOGI(AAFwkTag::KEEP_ALIVE, "status bar is created");
            return;
        }
        bool isRunning = false;
        bool result = DelayedSingleton<AppExecFwk::AppMgrClient>::GetInstance()->
            IsAppRunningByBundleNameAndUserId(bundleName, userId, isRunning);
        if (result == ERR_OK && !isRunning) {
            TAG_LOGE(AAFwkTag::KEEP_ALIVE, "keepAliveApp is not running");
            return;
        }
        TAG_LOGE(AAFwkTag::KEEP_ALIVE, "timeout, status bar not created, unsetting keep-alive");
        KeepAliveProcessManager::GetInstance().SetApplicationKeepAlive(bundleName, userId, false, true, true);
        (void)DelayedSingleton<AppExecFwk::AppMgrClient>::GetInstance()->KillApplication(bundleName);
    };

    std::lock_guard<ffrt::mutex> lock(checkStatusBarTasksMutex_);
    auto iter = std::find_if(checkStatusBarTasks_.begin(), checkStatusBarTasks_.end(),
        [uid](const std::shared_ptr<CheckStatusBarTask> &curTask) {
        return curTask != nullptr && curTask->GetUid() == uid;
    });
    if (iter != checkStatusBarTasks_.end()) {
        TAG_LOGI(AAFwkTag::KEEP_ALIVE, "exists same task, canceling");
        if (*iter != nullptr) {
            (*iter)->Cancel();
        }
        checkStatusBarTasks_.erase(iter);
    }
    auto checkStatusBarTask = std::make_shared<CheckStatusBarTask>(uid, std::move(task));
    checkStatusBarTasks_.push_back(checkStatusBarTask);
    ffrt::task_attr attr;
    attr.delay(CREATE_STATUS_BAR_TIMEOUT_MICRO_SECONDS);
    ffrt::submit([checkStatusBarTask]() {
        if (checkStatusBarTask != nullptr) {
            checkStatusBarTask->Run();
        }
        }, attr);
}

void KeepAliveProcessManager::RemoveCheckStatusBarTask(int32_t uid, bool shouldCancel)
{
    std::lock_guard<ffrt::mutex> lock(checkStatusBarTasksMutex_);
    auto iter = std::find_if(checkStatusBarTasks_.begin(), checkStatusBarTasks_.end(),
        [uid](const std::shared_ptr<CheckStatusBarTask> &curTask) {
        return curTask != nullptr && curTask->GetUid() == uid;
    });
    if (iter == checkStatusBarTasks_.end()) {
        TAG_LOGI(AAFwkTag::KEEP_ALIVE, "not exist");
        return;
    }
    if (*iter != nullptr && shouldCancel) {
        (*iter)->Cancel();
    }
    checkStatusBarTasks_.erase(iter);
}

int32_t KeepAliveProcessManager::SetApplicationKeepAlive(const std::string &bundleName,
    int32_t userId, bool updateEnable, bool isByEDM, bool isInner)
{
    auto result = isByEDM ? CheckPermissionForEDM() : CheckPermission();
    CHECK_RET_RETURN_RET(result, "permission denied");

    CHECK_TRUE_RETURN_RET(bundleName.empty(), INVALID_PARAMETERS_ERR, "input parameter error");

    auto bms = AbilityUtil::GetBundleManagerHelper();
    CHECK_POINTER_AND_RETURN(bms, INNER_ERR);

    userId = userId < 0 ? AbilityRuntime::UserController::GetInstance().GetCallerUserId() : userId;
    AppExecFwk::BundleInfo bundleInfo;
    if (!IN_PROCESS_CALL(bms->GetBundleInfo(
        bundleName, AppExecFwk::BundleFlag::GET_BUNDLE_DEFAULT, bundleInfo, userId))) {
        TAG_LOGE(AAFwkTag::KEEP_ALIVE, "get bundle info failed");
        return ERR_TARGET_BUNDLE_NOT_EXIST;
    }

    auto appMgrClient = DelayedSingleton<AppExecFwk::AppMgrClient>::GetInstance();
    CHECK_POINTER_AND_RETURN(appMgrClient, INNER_ERR);

    bool localEnable = IsKeepAliveBundle(bundleName, userId);
    if (updateEnable) {
        std::string mainElementName;
        CHECK_TRUE_RETURN_RET(!MainElementUtils::CheckMainUIAbility(bundleInfo, mainElementName),
            ERR_NO_MAIN_ABILITY, "bundle has no main uiability");
        if (DelayedSingleton<AbilityManagerService>::GetInstance()->IsSupportStatusBar(bundleInfo.uid)) {
            CHECK_TRUE_RETURN_RET(!MainElementUtils::CheckStatusBarAbility(bundleInfo),
                ERR_NO_STATUS_BAR_ABILITY, "app has no status bar");
            bool isRunning = false;
            result = IN_PROCESS_CALL(appMgrClient->IsAppRunningByBundleNameAndUserId(bundleName, userId, isRunning));
            CHECK_RET_RETURN_RET(result, "IsAppRunning failed");
            CHECK_TRUE_RETURN_RET((isRunning && !IsRunningAppInStatusBar(bundleInfo)),
                ERR_NOT_ATTACHED_TO_STATUS_BAR, "app is not attached to status bar");
        }
    }

    KeepAliveInfo info;
    info.bundleName = bundleName;
    info.userId = userId;
    info.appType = bundleInfo.applicationInfo.isSystemApp ? KeepAliveAppType::SYSTEM : KeepAliveAppType::THIRD_PARTY;
    info.setter = isByEDM ? KeepAliveSetter::SYSTEM : KeepAliveSetter::USER;
    result = AbilityKeepAliveService::GetInstance().SetApplicationKeepAlive(info, updateEnable);
    CHECK_RET_RETURN_RET(result, "set keep-alive failed");
    IN_PROCESS_CALL_WITHOUT_RET(appMgrClient->SetKeepAliveDkv(bundleName, updateEnable, 0));
    if (!updateEnable && localEnable) {
        TAG_LOGI(AAFwkTag::KEEP_ALIVE, "unsetting keep-alive");
        if (!isInner) {
            RemoveCheckStatusBarTask(bundleInfo.uid, true);
        }
        std::vector<AppExecFwk::BundleInfo> bundleInfos{ bundleInfo };
        KeepAliveUtils::NotifyDisableKeepAliveProcesses(bundleInfos, userId);
    }
    return ERR_OK;
}

bool KeepAliveProcessManager::IsRunningAppInStatusBar(const AppExecFwk::BundleInfo &bundleInfo)
{
    std::string mainElementName;
    if (!MainElementUtils::CheckMainUIAbility(bundleInfo, mainElementName)) {
        TAG_LOGE(AAFwkTag::KEEP_ALIVE, "bundle has no main uiability");
        return false;
    }
    auto isMultiInstance =
        bundleInfo.applicationInfo.multiAppMode.multiAppModeType == AppExecFwk::MultiAppModeType::MULTI_INSTANCE;
    return DelayedSingleton<AbilityManagerService>::GetInstance()->IsInStatusBar(
        bundleInfo.applicationInfo.accessTokenId, bundleInfo.uid, isMultiInstance);
}

void KeepAliveProcessManager::OnAppStateChanged(const AppInfo &info)
{
    if (info.state != AppState::BEGIN) {
        TAG_LOGD(AAFwkTag::KEEP_ALIVE, "Not a state of concern. state: %{public}d", info.state);
        return;
    }

    if (info.pid <= 0) {
        TAG_LOGD(AAFwkTag::KEEP_ALIVE, "The obtained application pid is incorrect. state: %{public}d", info.pid);
        return;
    }

    // user 0
    int32_t uid = 0;
    auto appScheduler = DelayedSingleton<AppScheduler>::GetInstance();
    if (appScheduler == nullptr) {
        TAG_LOGE(AAFwkTag::KEEP_ALIVE, "app scheduler error");
        return;
    }
    std::string bundleName;
    appScheduler->GetBundleNameByPid(info.pid, bundleName, uid);
    if (bundleName.empty()) {
        TAG_LOGE(AAFwkTag::KEEP_ALIVE, "get bundle name by pid failed");
        return;
    }

    auto appMgrClient = DelayedSingleton<AppExecFwk::AppMgrClient>::GetInstance();
    if (appMgrClient == nullptr) {
        TAG_LOGE(AAFwkTag::KEEP_ALIVE, "appMgrClient is null");
        return;
    }

    bool localEnableForAppservice = IsKeepAliveBundle(bundleName, U1_USER_ID);
    if (localEnableForAppservice) {
        IN_PROCESS_CALL_WITHOUT_RET(appMgrClient->SetKeepAliveAppService(bundleName, localEnableForAppservice, uid));
    }
}

bool KeepAliveProcessManager::IsKeepAliveBundle(const std::string &bundleName, int32_t userId)
{
    if (!system::GetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, false)) {
        TAG_LOGI(AAFwkTag::KEEP_ALIVE, "not supported");
        return false;
    }
    return AbilityKeepAliveService::GetInstance().IsKeepAliveApp(bundleName, userId);
}

bool KeepAliveProcessManager::GetKeepAliveBundleInfosForUser(std::vector<AppExecFwk::BundleInfo> &bundleInfos,
    int32_t userId)
{
    if (!system::GetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, false)) {
        TAG_LOGW(AAFwkTag::KEEP_ALIVE, "not supported");
        return false;
    }
    auto bundleMgrHelper = DelayedSingleton<AppExecFwk::BundleMgrHelper>::GetInstance();
    CHECK_POINTER_AND_RETURN(bundleMgrHelper, false);

    std::vector<KeepAliveInfo> infoList;
    auto ret = AbilityKeepAliveService::GetInstance().GetKeepAliveApplications(userId, infoList);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::KEEP_ALIVE, "failed get keep_alive bundle info: %{public}d", ret);
        return false;
    }
    for (const auto &info: infoList) {
        AppExecFwk::BundleInfo bundleInfo;
        if (!IN_PROCESS_CALL(bundleMgrHelper->GetBundleInfo(info.bundleName,
            AppExecFwk::BundleFlag::GET_BUNDLE_WITH_ABILITIES, bundleInfo, userId))) {
            TAG_LOGW(AAFwkTag::KEEP_ALIVE, "failed get bundle info: %{public}s", info.bundleName.c_str());
            continue;
        }
        bundleInfos.push_back(bundleInfo);
    }

    return !bundleInfos.empty();
}

void KeepAliveProcessManager::FilterNeedRestartKeepAliveBundleInfos(
    std::vector<AppExecFwk::BundleInfo> &bundleInfos)
{
    std::lock_guard<ffrt::mutex> lock(needRestartKeepAliveUidSetLock_);
    for (auto it = bundleInfos.begin(); it != bundleInfos.end();) {
        auto uidIter = needRestartKeepAliveUidSet_.find(it->uid);
        if (uidIter == needRestartKeepAliveUidSet_.end()) {
            TAG_LOGD(AAFwkTag::KEEP_ALIVE, "bundle no need to restart: %{public}s", it->name.c_str());
            it = bundleInfos.erase(it);
            continue;
        }
        needRestartKeepAliveUidSet_.erase(uidIter);
        ++it;
    }
}

void KeepAliveProcessManager::AddNeedRestartKeepAliveUid(int32_t uid)
{
    std::lock_guard<ffrt::mutex> lock(needRestartKeepAliveUidSetLock_);
    needRestartKeepAliveUidSet_.emplace(uid);
}

int32_t KeepAliveProcessManager::QueryKeepAliveApplications(int32_t appType, int32_t userId,
    std::vector<KeepAliveInfo> &infoList, bool isByEDM)
{
    auto result = isByEDM ? CheckPermissionForEDM() : CheckPermission();
    CHECK_RET_RETURN_RET(result, "permission denied");
    return AbilityKeepAliveService::GetInstance().QueryKeepAliveApplications(userId, appType, infoList);
}

int32_t KeepAliveProcessManager::SetAppServiceExtensionKeepAlive(const std::string &bundleName, bool updateEnable,
    bool isByEDM, bool isAllowUserToCancel)
{
    auto result = isByEDM ? CheckPermissionForEDM() : CheckPermission();
    CHECK_RET_RETURN_RET(result, "permission denied");

    CHECK_TRUE_RETURN_RET(bundleName.empty(), INVALID_PARAMETERS_ERR, "input parameter error");
   
    auto bms = AbilityUtil::GetBundleManagerHelper();
    CHECK_POINTER_AND_RETURN(bms, INNER_ERR);
    AppExecFwk::BundleInfo bundleInfo;
    if (!IN_PROCESS_CALL(bms->GetBundleInfo(
        bundleName, AppExecFwk::BundleFlag::GET_BUNDLE_DEFAULT, bundleInfo, U1_USER_ID))) {
        TAG_LOGE(AAFwkTag::KEEP_ALIVE, "The target bundle is not in u1");
        return ERR_NO_U1;
    }

    std::string mainElementName;
    CHECK_TRUE_RETURN_RET(!MainElementUtils::CheckAppServiceExtension(bundleInfo, mainElementName),
        ERR_INVALID_MAIN_ELEMENT_TYPE, "Invalid main element type");
    
    auto appMgrClient = DelayedSingleton<AppExecFwk::AppMgrClient>::GetInstance();
    CHECK_POINTER_AND_RETURN(appMgrClient, INNER_ERR);

    KeepAliveInfo info;
    info.bundleName = bundleName;
    info.userId = U1_USER_ID;
    info.setterId = IPCSkeleton::GetCallingUid() / BASE_USER_RANGE;
    info.appType = bundleInfo.applicationInfo.isSystemApp ? KeepAliveAppType::SYSTEM : KeepAliveAppType::THIRD_PARTY;
    info.setter = isByEDM ? KeepAliveSetter::SYSTEM : KeepAliveSetter::USER;
    info.policy = isByEDM ? (isAllowUserToCancel ? KeepAlivePolicy::ALLOW_CANCEL : KeepAlivePolicy::NOT_ALLOW_CANCEL) :
        KeepAlivePolicy::UNSPECIFIED;
    result = AbilityKeepAliveService::GetInstance().SetAppServiceExtensionKeepAlive(info, updateEnable);
    CHECK_RET_RETURN_RET(result, "set keep-alive failed");
    IN_PROCESS_CALL_WITHOUT_RET(appMgrClient->SetKeepAliveAppService(bundleName, updateEnable, bundleInfo.uid));
    return ERR_OK;
}

int32_t KeepAliveProcessManager::QueryKeepAliveAppServiceExtensions(std::vector<KeepAliveInfo> &infoList, bool isByEDM)
{
    auto result = isByEDM ? CheckPermissionForEDM() : CheckPermission();
    CHECK_RET_RETURN_RET(result, "permission denied");
    return AbilityKeepAliveService::GetInstance().QueryKeepAliveAppServiceExtensions(infoList);
}

void KeepAliveProcessManager::StartKeepAliveAppServiceExtension(std::vector<AppExecFwk::BundleInfo> &bundleInfos)
{
    for (const auto &bundleInfo : bundleInfos) {
        StartKeepAliveAppServiceExtensionPerBundle(bundleInfo);
    }
}

void KeepAliveProcessManager::StartKeepAliveAppServiceExtensionPerBundle(const AppExecFwk::BundleInfo &bundleInfo)
{
    auto userId = U1_USER_ID;
    if (!IsKeepAliveBundle(bundleInfo.name, userId)) {
        TAG_LOGE(AAFwkTag::KEEP_ALIVE, "bundle is not set keep-alive");
        return;
    }

    std::string mainElementName;
    if (!MainElementUtils::CheckAppServiceExtension(bundleInfo, mainElementName)) {
        TAG_LOGE(AAFwkTag::KEEP_ALIVE, "Invalid main element type");
        return;
    }

    KeepAliveAbilityInfo info = {
        .userId = userId,
        .appCloneIndex = bundleInfo.appIndex,
        .uid = bundleInfo.uid,
        .bundleName = bundleInfo.name,
        .moduleName = bundleInfo.entryModuleName,
        .abilityName = mainElementName,
    };
    auto ret = StartKeepAliveAppServiceExtensionInner(info);
    if (ret == ERR_OK) {
        TAG_LOGI(AAFwkTag::KEEP_ALIVE, "start ok");
        return;
    }

    TAG_LOGE(AAFwkTag::KEEP_ALIVE, "StartKeepAliveAppServiceExtension failed:%{public}d, retry", ret);
    ffrt::submit([bundleName = bundleInfo.name, userId, info, ret]() mutable {
        for (int tried = 0; tried < MAX_RETRY_TIMES && ret != ERR_OK; tried++) {
            usleep(RETRY_INTERVAL_MICRO_SECONDS);
            TAG_LOGI(AAFwkTag::KEEP_ALIVE, "retry attempt:%{public}d", tried + 1);
            ret = KeepAliveProcessManager::GetInstance().StartKeepAliveAppServiceExtensionInner(info);
            TAG_LOGI(AAFwkTag::KEEP_ALIVE, "retry result:%{public}d", ret);
        }
        if (ret != ERR_OK) {
            TAG_LOGE(AAFwkTag::KEEP_ALIVE, "reach max retry, failed:%{public}d, unsetting keep-alive", ret);
            return;
        }
    });
}

int32_t KeepAliveProcessManager::StartKeepAliveAppServiceExtensionInner(const KeepAliveAbilityInfo &info)
{
    Want want;
    want.SetElementName(info.bundleName, info.abilityName);
    want.SetParam(Want::PARAM_APP_CLONE_INDEX_KEY, info.appCloneIndex);
    TAG_LOGI(AAFwkTag::KEEP_ALIVE, "call, bundleName: %{public}s, moduleName: %{public}s, mainElement: %{public}s"
        " appCloneIndex: %{public}d", info.bundleName.c_str(), info.moduleName.c_str(), info.abilityName.c_str(),
        info.appCloneIndex);
    auto ret = IN_PROCESS_CALL(DelayedSingleton<AbilityManagerService>::GetInstance()->StartExtensionAbility(want,
        nullptr, info.userId, AppExecFwk::ExtensionAbilityType::APP_SERVICE));
    return ret;
}

int32_t KeepAliveProcessManager::ClearKeepAliveAppServiceExtension(int32_t userId)
{
    KeepAliveInfo info;
    info.setterId = userId;
    return AbilityKeepAliveService::GetInstance().ClearKeepAliveAppServiceExtension(info);
}

int32_t KeepAliveProcessManager::CheckPermission()
{
    if (!system::GetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, false)) {
        TAG_LOGE(AAFwkTag::KEEP_ALIVE, "not supported");
        return ERR_CAPABILITY_NOT_SUPPORT;
    }

    if (!PermissionVerification::GetInstance()->JudgeCallerIsAllowedToUseSystemAPI()) {
        TAG_LOGE(AAFwkTag::KEEP_ALIVE, "not use system-api");
        return ERR_NOT_SYSTEM_APP;
    }

    if (!PermissionVerification::GetInstance()->VerifyCallingPermission(
        PermissionConstants::PERMISSION_MANAGE_APP_KEEP_ALIVE)) {
        TAG_LOGE(AAFwkTag::KEEP_ALIVE, "verify PERMISSION_MANAGE_APP_KEEP_ALIVE fail");
        return CHECK_PERMISSION_FAILED;
    }

    return ERR_OK;
}

int32_t KeepAliveProcessManager::CheckPermissionForEDM()
{
    if (!system::GetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, false)) {
        TAG_LOGE(AAFwkTag::KEEP_ALIVE, "not supported");
        return ERR_CAPABILITY_NOT_SUPPORT;
    }
    if (PermissionVerification::GetInstance()->CheckSpecificSystemAbilityAccessPermission(FOUNDATION_PROCESS_NAME)
        || (PermissionVerification::GetInstance()->IsSACall()
        && PermissionVerification::GetInstance()->VerifyCallingPermission(
            PermissionConstants::PERMISSION_MANAGE_APP_KEEP_ALIVE_INTERNAL))) {
        return ERR_OK;
    }
    TAG_LOGE(AAFwkTag::KEEP_ALIVE, "verify PERMISSION_MANAGE_APP_KEEP_ALIVE_INTERNAL fail");
    return CHECK_PERMISSION_FAILED;
}

void KeepAliveProcessManager::SaveAppSeriviceRestartAfterUpgrade(const std::string &bundleName, int32_t uid)
{
    if (!IsKeepAliveBundle(bundleName, U1_USER_ID)) {
        TAG_LOGE(AAFwkTag::KEEP_ALIVE, "bundle is not set keep-alive");
        return;
    }

    auto appMgrClient = DelayedSingleton<AppExecFwk::AppMgrClient>::GetInstance();
    if (!appMgrClient) {
        TAG_LOGE(AAFwkTag::KEEP_ALIVE, "appMgrClient is null");
        return;
    }
    
    std::vector<AppExecFwk::RunningProcessInfo> infos;
    auto ret = IN_PROCESS_CALL(appMgrClient->GetProcessRunningInfosByUserId(infos, uid / BASE_USER_RANGE));
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::KEEP_ALIVE, "get ProcessRunningInfos by userId fail");
        return;
    }

    std::lock_guard<std::mutex> lock(restartAfterUpgradeMutex_);
    for (const auto& info : infos) {
        if (info.uid_ == uid && info.isKeepAliveAppService) {
            restartAfterUpgradeList_.insert(uid);
            IN_PROCESS_CALL_WITHOUT_RET(appMgrClient->SetKeepAliveAppService(bundleName, false, uid));
        }
    }
}

void KeepAliveProcessManager::SaveKeepAliveAppRestartAfterUpgrade(const std::string &bundleName, int32_t uid)
{
    TAG_LOGD(AAFwkTag::KEEP_ALIVE, "call SaveKeepAliveAppRestartAfterUpgrade");
    int32_t userId = uid / BASE_USER_RANGE;
    if (!IsKeepAliveBundle(bundleName, userId)) {
        TAG_LOGE(AAFwkTag::KEEP_ALIVE, "bundle is not set keep-alive");
        return;
    }

    auto appMgrClient = DelayedSingleton<AppExecFwk::AppMgrClient>::GetInstance();
    CHECK_POINTER_LOG(appMgrClient, "appMgrClient nullptr");

    bool isRunning = false;
    int32_t result = IN_PROCESS_CALL(appMgrClient->IsAppRunningByBundleNameAndUserId(bundleName, userId, isRunning));
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::KEEP_ALIVE, "IsAppRunningByBundleNameAndUserId fail");
        return;
    }

    if (isRunning) {
        std::lock_guard<std::mutex> lock(restartAfterUpgradeMutex_);
        TAG_LOGD(AAFwkTag::KEEP_ALIVE, "keepAliveApp is running while update. uid: %{public}d", uid);
        keepAliveRestartAfterUpgradeList_.insert(uid);
    }
}

bool KeepAliveProcessManager::CheckNeedRestartAfterUpgrade(int32_t uid)
{
    std::lock_guard<std::mutex> lock(restartAfterUpgradeMutex_);
    auto iter = restartAfterUpgradeList_.find(uid);
    if (iter == restartAfterUpgradeList_.end()) {
        return false;
    }
    restartAfterUpgradeList_.erase(iter);
    return true;
}

bool KeepAliveProcessManager::KeepAliveCheckNeedRestartAfterUpgrade(int32_t uid)
{
    std::lock_guard<std::mutex> lock(restartAfterUpgradeMutex_);
    auto iter = keepAliveRestartAfterUpgradeList_.find(uid);
    if (iter == keepAliveRestartAfterUpgradeList_.end()) {
        return false;
    }
    keepAliveRestartAfterUpgradeList_.erase(iter);
    return true;
}

bool KeepAliveProcessManager::KeepAliveIsRestartAfterUpdate(int32_t uid)
{
    std::lock_guard<std::mutex> lock(restartAfterUpgradeMutex_);
    return keepAliveRestartAfterUpgradeList_.find(uid) != keepAliveRestartAfterUpgradeList_.end();
}

void KeepAliveProcessManager::StartKeepAliveAfterAppUpgrade(std::vector<AppExecFwk::BundleInfo> &bundleInfos,
    int32_t uid)
{
    if (!KeepAliveCheckNeedRestartAfterUpgrade(uid)) {
        TAG_LOGD(AAFwkTag::KEEP_ALIVE, "keepAlive no need to restart after update. uid: %{public}d", uid);
        return;
    }
    int32_t userId = uid / BASE_USER_RANGE;
    StartKeepAliveProcessWithMainElement(bundleInfos, userId);
}
}  // namespace AAFwk
}  // namespace OHOS
