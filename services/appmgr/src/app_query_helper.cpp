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

#include "app_query_helper.h"

#include <cinttypes>
#include <map>
#include <mutex>
#include <sstream>
#include <vector>

#include "ability_manager_errors.h"
#include "ability_state.h"
#include "accesstoken_kit.h"
#include "appfreeze_manager.h"
#include "app_mgr_constants.h"
#include "app_mgr_interface.h"
#include "app_mgr_service_const.h"
#include "app_preloader.h"
#include "app_running_manager.h"
#include "app_running_record.h"
#include "app_state_data.h"
#include "app_utils.h"
#include "application_info.h"
#include "application_state_filter.h"
#include "bundle_constants.h"
#include "bundle_mgr_helper.h"
#include "cache_process_manager.h"
#include "child_process_info.h"
#include "configuration.h"
#include "configuration_policy.h"
#include "datetime_ex.h"
#include "exit_resident_process_manager.h"
#include "ffrt.h"
#include "global_constant.h"
#include "hilog_tag_wrapper.h"
#include "hisysevent_report.h"
#include "hitrace_meter.h"
#include "hyper_snap_error_record.h"
#include "in_process_call_wrapper.h"
#include "ipc_skeleton.h"
#include "iremote_object.h"
#include "iservice_registry.h"
#include "killing_process_manager.h"
#include "mem_mgr_client.h"
#include "mem_mgr_process_state_info.h"
#include "multi_user_config_mgr.h"
#include "os_account_manager.h"
#include "os_account_manager_wrapper.h"
#include "parameters.h"
#include "permission_constants.h"
#include "permission_verification.h"
#include "process_uid_define.h"
#include "record_query_result.h"
#include "remote_client_manager.h"
#include "render_process_info.h"
#include "res_sched_util.h"
#include "running_multi_info.h"
#include "running_process_info.h"
#include "string_ex.h"
#include "system_ability_definition.h"
#include "ui_ability_last_caller_info.h"
#include "user_controller/user_controller.h"

namespace OHOS {
namespace AppExecFwk {
using namespace OHOS::AbilityRuntime;
using namespace OHOS::Security;

namespace {
constexpr int32_t BASE_USER_RANGE = 200000;
constexpr int32_t USER_SCALE = 200000;
constexpr int32_t USER100 = 100;
constexpr const int32_t U0_USER_ID = 0;
constexpr const int32_t U1_USER_ID = 1;
constexpr int32_t FOUNDATION_UID = 5523;
constexpr int32_t FILE_SYSTEM_SERVICE_UID = 7100;
constexpr const char* SYSTEM_BASIC = "system_basic";
constexpr const char* SYSTEM_CORE = "system_core";
constexpr const char* SCENE_BOARD_BUNDLE_NAME = "com.ohos.sceneboard";
constexpr const char* DEVELOPER_MODE_STATE = "const.security.developermode.state";
const std::string LARGE_SCREEN = "large_screen";
constexpr int32_t BUNDLE_INFO_FLAG_WITH_APP_EXT_HAP_PERM =
    static_cast<int32_t>(GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_APPLICATION) |
    static_cast<int32_t>(GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_EXTENSION_ABILITY) |
    static_cast<int32_t>(GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_HAP_MODULE) |
    static_cast<int32_t>(GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_REQUESTED_PERMISSION);

inline int32_t GetUserIdByUid(int32_t uid)
{
    return uid / BASE_USER_RANGE;
}

#define CHECK_CALLER_IS_SYSTEM_APP                                                             \
    if (!AAFwk::PermissionVerification::GetInstance()->JudgeCallerIsAllowedToUseSystemAPI()) { \
        TAG_LOGE(AAFwkTag::APPMGR, "can't use SA");    \
        return AAFwk::ERR_NOT_SYSTEM_APP;                                                      \
    }

#define CHECK_POINTER_AND_RETURN_LOG(object, log)      \
    if (!object) {                                     \
        TAG_LOGE(AAFwkTag::APPMGR, "%{public}s", log); \
        return;                                        \
    }

#define CHECK_POINTER_AND_RETURN_VALUE(object, value) \
    if (!object) {                              \
        TAG_LOGE(AAFwkTag::APPMGR, "nullptr");  \
        return value;                           \
    }
} // namespace

void AppQueryHelper::Init(const std::shared_ptr<AppRunningManager> &appRunningManager,
    const std::shared_ptr<RemoteClientManager> &remoteClientManager,
    const std::shared_ptr<MultiUserConfigurationMgr> &multiUserConfigurationMgr,
    const std::shared_ptr<AppPreloader> &appPreloader)
{
    appRunningManager_ = appRunningManager;
    remoteClientManager_ = remoteClientManager;
    multiUserConfigurationMgr_ = multiUserConfigurationMgr;
    appPreloader_ = appPreloader;
}

void AppQueryHelper::SetRemoveRenderRecordCallback(RemoveRenderRecordCallback callback)
{
    removeRenderRecordCallback_ = std::move(callback);
}

bool AppQueryHelper::IsSceneBoardCall()
{
    if (!AAFwk::PermissionVerification::GetInstance()->IsSystemAppCall()) {
        TAG_LOGE(AAFwkTag::APPMGR, "not system app call");
        return false;
    }
    if (remoteClientManager_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "remoteClientManager_ null");
        return false;
    }
    auto bundleMgrHelper = remoteClientManager_->GetBundleManagerHelper();
    if (bundleMgrHelper != nullptr) {
        int32_t callingUid = IPCSkeleton::GetCallingUid();
        std::string callerBundleName;
        IN_PROCESS_CALL(bundleMgrHelper->GetNameForUid(callingUid, callerBundleName));
        return callerBundleName == SCENE_BOARD_BUNDLE_NAME;
    }
    return false;
}

int32_t AppQueryHelper::GetAllRunningProcesses(std::vector<RunningProcessInfo> &info)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto isPerm = AAFwk::PermissionVerification::GetInstance()->VerifyRunningInfoPerm();
    // check permission
    for (const auto &item : appRunningManager_->GetAppRunningRecordMap()) {
        const auto &appRecord = item.second;
        if (!appRecord || !appRecord->GetSpawned()) {
            continue;
        }
        if (isPerm) {
            GetRunningProcesses(appRecord, info);
        } else {
            auto applicationInfo = appRecord->GetApplicationInfo();
            if (!applicationInfo) {
                continue;
            }
            auto callingTokenId = IPCSkeleton::GetCallingTokenID();
            auto tokenId = applicationInfo->accessTokenId;
            if (callingTokenId == tokenId) {
                GetRunningProcesses(appRecord, info);
            }
        }
    }
    return ERR_OK;
}

int32_t AppQueryHelper::GetRunningProcessesByBundleType(BundleType bundleType,
    std::vector<RunningProcessInfo> &info)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    CHECK_CALLER_IS_SYSTEM_APP;
    if (!AAFwk::PermissionVerification::GetInstance()->VerifyRunningInfoPerm()) {
        TAG_LOGE(AAFwkTag::APPMGR, "permission deny");
        return ERR_PERMISSION_DENIED;
    }
    int32_t userId = UserController::GetInstance().GetCallerUserId();
    for (const auto &item : appRunningManager_->GetAppRunningRecordMap()) {
        const auto &appRecord = item.second;
        if (!appRecord || !appRecord->GetSpawned()) {
            continue;
        }
        if (GetUserIdByUid(appRecord->GetUid()) != userId) {
            continue;
        }
        auto appInfo = appRecord->GetApplicationInfo();
        if (appInfo && appInfo->bundleType == bundleType) {
            GetRunningProcesses(appRecord, info);
        }
    }
    return ERR_OK;
}

int32_t AppQueryHelper::GetAllRunningInstanceKeysBySelf(std::vector<std::string> &instanceKeys)
{
    if (remoteClientManager_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "remoteClientManager_ null");
        return ERR_NO_INIT;
    }
    auto bundleMgrHelper = remoteClientManager_->GetBundleManagerHelper();
    if (bundleMgrHelper == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "bundleMgrHelper null");
        return ERR_INVALID_VALUE;
    }
    std::string bundleName;
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    auto ret = IN_PROCESS_CALL(bundleMgrHelper->GetNameForUid(callingUid, bundleName));
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "GetNameForUid failed, ret=%{public}d", ret);
        return AAFwk::ERR_BUNDLE_NOT_EXIST;
    }
    int32_t userId = GetUserIdByUid(IPCSkeleton::GetCallingUid());
    return GetAllRunningInstanceKeysByBundleNameInner(bundleName, instanceKeys, userId);
}

int32_t AppQueryHelper::GetAllRunningInstanceKeysByBundleName(const std::string &bundleName,
    std::vector<std::string> &instanceKeys, int32_t userId)
{
    if (userId == -1) {
        userId = AbilityRuntime::UserController::GetInstance().GetCallerUserId();
    }
    if (VerifyAccountPermission(AAFwk::PermissionConstants::PERMISSION_GET_RUNNING_INFO, userId) ==
        ERR_PERMISSION_DENIED) {
        TAG_LOGE(AAFwkTag::APPMGR, "%{public}s: Permission verification fail", __func__);
        return ERR_PERMISSION_DENIED;
    }
    return GetAllRunningInstanceKeysByBundleNameInner(bundleName, instanceKeys, userId);
}

int32_t AppQueryHelper::GetAllRunningInstanceKeysByBundleNameInner(const std::string &bundleName,
    std::vector<std::string> &instanceKeys, int32_t userId)
{
    if (bundleName.empty()) {
        TAG_LOGE(AAFwkTag::APPMGR, "bundlename null");
        return AAFwk::INVALID_PARAMETERS_ERR;
    }
    if (remoteClientManager_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "remoteClientManager_ null");
        return ERR_INVALID_VALUE;
    }
    auto bundleMgrHelper = remoteClientManager_->GetBundleManagerHelper();
    if (bundleMgrHelper == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "bundleMgrHelper null");
        return ERR_INVALID_VALUE;
    }
    ApplicationInfo appInfo;
    TAG_LOGD(AAFwkTag::APPMGR, "userId: %{public}d, bundleName: %{public}s", userId, bundleName.c_str());
    auto queryRet = IN_PROCESS_CALL(bundleMgrHelper->GetApplicationInfo(bundleName,
        ApplicationFlag::GET_BASIC_APPLICATION_INFO, userId, appInfo));
    if (!queryRet) {
        TAG_LOGE(AAFwkTag::APPMGR, "bundle unexist");
        return AAFwk::ERR_BUNDLE_NOT_EXIST;
    }
    if (appInfo.multiAppMode.multiAppModeType != MultiAppModeType::MULTI_INSTANCE) {
        TAG_LOGE(AAFwkTag::APPMGR, "bundle unsupport multi-instance");
        return AAFwk::ERR_MULTI_INSTANCE_NOT_SUPPORTED;
    }
    if (!appRunningManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRunningManager null");
        return ERR_INVALID_VALUE;
    }
    auto multiAppInfoMap = appRunningManager_->GetAppRunningRecordMap();
    for (const auto &item : multiAppInfoMap) {
        const std::shared_ptr<AppRunningRecord> &appRecord = item.second;
        if (appRecord == nullptr || appRecord->GetBundleName() != bundleName) {
            continue;
        }
        if (GetUserIdByUid(appRecord->GetUid()) != userId) {
            continue;
        }
        GetRunningMultiInstanceKeys(appRecord, instanceKeys);
    }
    return ERR_OK;
}

void AppQueryHelper::GetRunningMultiInstanceKeys(const std::shared_ptr<AppRunningRecord> &appRecord,
    std::vector<std::string> &instanceKeys)
{
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRecord null");
        return;
    }
    auto PriorityObject = appRecord->GetPriorityObject();
    if (!PriorityObject) {
        TAG_LOGE(AAFwkTag::APPMGR, "priorityObject null");
        return;
    }
    size_t index = 0;
    for (; index < instanceKeys.size(); ++index) {
        if (instanceKeys[index] == appRecord->GetInstanceKey()) {
            return;
        }
    }
    instanceKeys.emplace_back(appRecord->GetInstanceKey());
}

int32_t AppQueryHelper::GetProcessRunningInfosByUserId(std::vector<RunningProcessInfo> &info, int32_t userId)
{
    if (VerifyAccountPermission(AAFwk::PermissionConstants::PERMISSION_GET_RUNNING_INFO, userId) ==
        ERR_PERMISSION_DENIED) {
        TAG_LOGE(AAFwkTag::APPMGR, "%{public}s: Permission verification fail", __func__);
        return ERR_PERMISSION_DENIED;
    }

    for (const auto &item : appRunningManager_->GetAppRunningRecordMap()) {
        const auto &appRecord = item.second;
        if (!appRecord || !appRecord->GetSpawned()) {
            continue;
        }
        int32_t userIdTemp = static_cast<int32_t>(appRecord->GetUid() / USER_SCALE);
        if (userIdTemp == userId) {
            GetRunningProcesses(appRecord, info);
        }
    }
    return ERR_OK;
}

int32_t AppQueryHelper::GetProcessRunningInfosByAccessTokenId(uint32_t accessTokenId,
    std::vector<RunningProcessInfo> &info)
{
    for (const auto &item : appRunningManager_->GetAppRunningRecordMap()) {
        const auto &appRecord = item.second;
        if (!appRecord || !appRecord->GetSpawned()) {
            continue;
        }
        if (appRecord->GetAccessTokenId() == accessTokenId) {
            GetRunningProcesses(appRecord, info);
        }
    }
    return ERR_OK;
}

int32_t AppQueryHelper::GetProcessRunningInformation(RunningProcessInfo &info)
{
    if (!appRunningManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRunningManager_ null");
        return ERR_NO_INIT;
    }
    auto callerPid = IPCSkeleton::GetCallingPid();
    auto appRecord = GetAppRunningRecordByPid(callerPid);
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "no appRecord, callerPid:%{public}d", callerPid);
        return ERR_INVALID_VALUE;
    }
    GetRunningProcess(appRecord, info);
    return ERR_OK;
}

int32_t AppQueryHelper::GetAllRenderProcesses(std::vector<RenderProcessInfo> &info)
{
    auto isPerm = AAFwk::PermissionVerification::GetInstance()->VerifyRunningInfoPerm();
    // check permission
    for (const auto &item : appRunningManager_->GetAppRunningRecordMap()) {
        const auto &appRecord = item.second;
        if (isPerm) {
            GetRenderProcesses(appRecord, info);
        } else {
            if (appRecord == nullptr) {
                continue;
            }
            auto applicationInfo = appRecord->GetApplicationInfo();
            if (!applicationInfo) {
                continue;
            }
            auto callingTokenId = IPCSkeleton::GetCallingTokenID();
            auto tokenId = applicationInfo->accessTokenId;
            if (callingTokenId == tokenId) {
                GetRenderProcesses(appRecord, info);
            }
        }
    }
    return ERR_OK;
}

#ifdef SUPPORT_CHILD_PROCESS
int AppQueryHelper::GetAllChildrenProcesses(std::vector<ChildProcessInfo> &info)
{
    auto isPerm = AAFwk::PermissionVerification::GetInstance()->VerifyRunningInfoPerm();
    if (isPerm) {
        for (const auto &item : appRunningManager_->GetAppRunningRecordMap()) {
            GetChildrenProcesses(item.second, info);
        }
    } else {
        GetSelfChildrenProcesses(info);
    }
    return ERR_OK;
}

int AppQueryHelper::GetSelfChildrenProcesses(std::vector<ChildProcessInfo> &info)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    auto callingTokenId = IPCSkeleton::GetCallingTokenID();
    for (const auto &item : appRunningManager_->GetAppRunningRecordMap()) {
        const auto &appRecord = item.second;
        if (!appRecord) {
            continue;
        }
        auto applicationInfo = appRecord->GetApplicationInfo();
        if (!applicationInfo) {
            continue;
        }
        if (callingTokenId == applicationInfo->accessTokenId) {
            GetChildrenProcesses(appRecord, info);
        }
    }
    return ERR_OK;
}
#endif // SUPPORT_CHILD_PROCESS

void AppQueryHelper::GetRunningProcesses(const std::shared_ptr<AppRunningRecord> &appRecord,
    std::vector<RunningProcessInfo> &info)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    RunningProcessInfo runningProcessInfo;
    GetRunningProcess(appRecord, runningProcessInfo);
    info.emplace_back(runningProcessInfo);
}

void AppQueryHelper::GetRunningProcess(const std::shared_ptr<AppRunningRecord> &appRecord,
    RunningProcessInfo &info)
{
    CHECK_POINTER_AND_RETURN_LOG(appRecord, "appRecord unexist");
    info.processName_ = appRecord->GetProcessName();
    info.pid_ = appRecord->GetPid();
    info.uid_ = appRecord->GetUid();
    info.accessTokenId_ = appRecord->GetAccessTokenId();
    info.state_ = static_cast<AppProcessState>(appRecord->GetState());
    info.isContinuousTask = appRecord->IsContinuousTask();
    info.isKeepAlive = appRecord->IsKeepAliveApp();
    info.isKeepAliveAppService = appRecord->IsKeepAliveAppService();
    info.isFocused = appRecord->GetFocusFlag();
    info.startTimeMillis_ = appRecord->GetAppStartTime();
    appRecord->GetBundleNames(info.bundleNames);
    info.processType_ = appRecord->GetProcessType();
    info.extensionType_ = appRecord->GetExtensionType();
    info.preloadMode_ = appRecord->GetPreloadMode();
    auto appInfoForDebug = appRecord->GetApplicationInfo();
    if (appInfoForDebug != nullptr) {
        info.isDebugApp  = appInfoForDebug->debug;
    } else {
        TAG_LOGE(AAFwkTag::APPMGR, "appInfo nullptr");
    }
    info.isExiting = appRecord->IsTerminating() || appRecord->IsKilling()
        || appRecord->GetRestartAppFlag() || appRecord->IsUserRequestCleaning();
    info.isPreForeground  = appRecord->IsPreForeground();
    info.isCached = DelayedSingleton<CacheProcessManager>::GetInstance()->IsCachedProcess(appRecord);
    if (appRecord->GetUserTestInfo() != nullptr && system::GetBoolParameter(DEVELOPER_MODE_STATE, false)) {
        info.isTestMode = true;
    }
    auto appInfo = appRecord->GetApplicationInfo();
    if (appInfo) {
        info.bundleType = static_cast<int32_t>(appInfo->bundleType);
        if (appInfo->multiAppMode.multiAppModeType == MultiAppModeType::APP_CLONE) {
            info.appCloneIndex = appRecord->GetAppIndex();
        }
    }
    info.isPreload = SetPreloadFlagForProcessInfo(appRecord);
}

void AppQueryHelper::GetRenderProcesses(const std::shared_ptr<AppRunningRecord> &appRecord,
    std::vector<RenderProcessInfo> &info)
{
    CHECK_POINTER_AND_RETURN_LOG(appRecord, "appRecord unexist");
    auto renderRecordMap = appRecord->GetRenderRecordMap();
    if (renderRecordMap.empty()) {
        return;
    }
    for (auto iter : renderRecordMap) {
        auto renderRecord = iter.second;
        if (renderRecord != nullptr) {
            RenderProcessInfo renderProcessInfo;
            renderProcessInfo.bundleName_ = renderRecord->GetHostBundleName();
            renderProcessInfo.processName_ = renderRecord->GetProcessName();
            renderProcessInfo.pid_ = renderRecord->GetPid();
            renderProcessInfo.uid_ = renderRecord->GetUid();
            renderProcessInfo.hostUid_ = renderRecord->GetHostUid();
            renderProcessInfo.hostPid_ = renderRecord->GetHostPid();
            renderProcessInfo.state_ = renderRecord->GetState();
            info.emplace_back(renderProcessInfo);
        }
    }
}

#ifdef SUPPORT_CHILD_PROCESS
void AppQueryHelper::GetChildrenProcesses(const std::shared_ptr<AppRunningRecord> &appRecord,
    std::vector<ChildProcessInfo> &info)
{
    auto childProcessRecordMap = appRecord->GetChildProcessRecordMap();
    if (childProcessRecordMap.empty()) {
        return;
    }
    int32_t retCode = ERR_OK;
    for (auto iter : childProcessRecordMap) {
        auto childProcessRecord = iter.second;
        if (childProcessRecord != nullptr) {
            ChildProcessInfo childProcessInfo;
            retCode = GetChildProcessInfo(childProcessRecord, appRecord, childProcessInfo);
            if (retCode != ERR_OK) {
                TAG_LOGW(
                    AAFwkTag::APPMGR, "GetChildProcessInfo failed. host pid=%{public}d, child pid=%{public}d",
                    appRecord->GetPid(), childProcessRecord->GetPid());
                continue;
            }
            info.emplace_back(childProcessInfo);
        }
    }
}
#endif // SUPPORT_CHILD_PROCESS

int32_t AppQueryHelper::GetSelfUIAbilityChildProcesses(std::vector<ChildProcessInfo> &infos)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    if (!appRunningManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRunningManager_ null");
        return ERR_NO_INIT;
    }
    auto callingTokenId = IPCSkeleton::GetCallingTokenID();
    for (const auto &item : appRunningManager_->GetAppRunningRecordMap()) {
        const auto &appRecord = item.second;
        if (!appRecord) {
            continue;
        }
        auto applicationInfo = appRecord->GetApplicationInfo();
        if (!applicationInfo) {
            continue;
        }
        if (callingTokenId == applicationInfo->accessTokenId) {
            GetUIAbilityChildProcesses(appRecord, infos);
        }
    }
    return ERR_OK;
}

void AppQueryHelper::GetUIAbilityChildProcesses(const std::shared_ptr<AppRunningRecord> &appRecord,
    std::vector<ChildProcessInfo> &info)
{
    if (!appRecord) {
        return;
    }
    auto childAppRecordMap = appRecord->GetChildAppRecordMap();
    for (const auto &iter : childAppRecordMap) {
        auto childAppRecord = iter.second.lock();
        if (childAppRecord == nullptr) {
            continue;
        }
        ChildProcessInfo childProcessInfo;
        childProcessInfo.pid = childAppRecord->GetPid();
        childProcessInfo.hostPid = appRecord->GetPid();
        childProcessInfo.processName = childAppRecord->GetProcessName();
        info.emplace_back(childProcessInfo);
    }
}

void AppQueryHelper::GetRunningProcessInfoByToken(
    const sptr<IRemoteObject> &token, AppExecFwk::RunningProcessInfo &info)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    if (!CheckGetRunningInfoPermission()) {
        return;
    }

    appRunningManager_->GetRunningProcessInfoByToken(token, info);
}

int32_t AppQueryHelper::GetRunningProcessInfoByPid(const pid_t pid,
    OHOS::AppExecFwk::RunningProcessInfo &info) const
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    if (!CheckGetRunningInfoPermission()) {
        return ERR_PERMISSION_DENIED;
    }

    return appRunningManager_->GetRunningProcessInfoByPid(pid, info);
}

int32_t AppQueryHelper::GetRunningProcessInfoByChildProcessPid(const pid_t childPid,
    OHOS::AppExecFwk::RunningProcessInfo &info) const
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    if (!CheckGetRunningInfoPermission()) {
        return ERR_PERMISSION_DENIED;
    }

    return appRunningManager_->GetRunningProcessInfoByChildProcessPid(childPid, info);
}

bool AppQueryHelper::CheckGetRunningInfoPermission() const
{
    if (!appRunningManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRunningManager_ null");
        return false;
    }

    auto isPerm = AAFwk::PermissionVerification::GetInstance()->VerifyRunningInfoPerm();
    if (!isPerm) {
        TAG_LOGE(AAFwkTag::APPMGR, "permission verify fail");
        return false;
    }

    return true;
}

int32_t AppQueryHelper::GetForegroundApplications(std::vector<AppStateData> &list)
{
    TAG_LOGD(AAFwkTag::APPMGR, "begin.");
    CHECK_CALLER_IS_SYSTEM_APP;
    auto isPerm = AAFwk::PermissionVerification::GetInstance()->VerifyRunningInfoPerm();
    if (!isPerm) {
        TAG_LOGE(AAFwkTag::APPMGR, "permission verify fail");
        return ERR_PERMISSION_DENIED;
    }

    appRunningManager_->GetForegroundApplications(list);
    return ERR_OK;
}

std::shared_ptr<AppExecFwk::Configuration> AppQueryHelper::GetConfiguration()
{
    if (multiUserConfigurationMgr_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "multiUserConfigurationMgr_ null");
        return nullptr;
    }
    int32_t userId = 0;
    auto errNo = AccountSA::OsAccountManager::GetForegroundOsAccountLocalId(userId);
    if (errNo != 0) {
        TAG_LOGE(AAFwkTag::APPMGR, "GetForegroundOsAccountLocalId failed: %{public}d", errNo);
        userId = USER100;
    }
    TAG_LOGD(AAFwkTag::APPMGR, "GetForegroundOsAccountLocalId userId: %{public}d", userId);
    return multiUserConfigurationMgr_->GetConfigurationByUserId(userId);
}

std::shared_ptr<AppExecFwk::Configuration> AppQueryHelper::GetConfiguration(int32_t userId)
{
    if (multiUserConfigurationMgr_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "multiUserConfigurationMgr_ null");
        return nullptr;
    }

    TAG_LOGD(AAFwkTag::APPMGR, "Using specified user ID: %{public}d", userId);
    return multiUserConfigurationMgr_->GetConfigurationByUserId(userId);
}

int AppQueryHelper::GetAbilityRecordsByProcessID(const int pid, std::vector<sptr<IRemoteObject>> &tokens)
{
    auto appRecord = GetAppRunningRecordByPid(pid);
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "no appRecord");
        return ERR_NAME_NOT_FOUND;
    }

    auto isSaCall = AAFwk::PermissionVerification::GetInstance()->IsSACall();
    auto callingPid = IPCSkeleton::GetCallingPid();
    if (!isSaCall && callingPid != pid) {
        TAG_LOGE(AAFwkTag::APPMGR, "permission verify fail");
        return ERR_PERMISSION_DENIED;
    }
    for (auto &item : appRecord->GetAbilities()) {
        tokens.emplace_back(item.first);
    }
    return ERR_OK;
}

int AppQueryHelper::GetApplicationInfoByProcessID(const int pid, AppExecFwk::ApplicationInfo &application,
    bool &debug)
{
    auto isSaCall = AAFwk::PermissionVerification::GetInstance()->IsSACall();
    auto isShellCall = AAFwk::PermissionVerification::GetInstance()->IsShellCall();
    if (!isSaCall && !isShellCall) {
        TAG_LOGE(AAFwkTag::APPMGR, "no permissions");
        return ERR_PERMISSION_DENIED;
    }
    auto appRecord = GetAppRunningRecordByPid(pid);
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "no appRecord for PID:%{public}d", pid);
        return ERR_NAME_NOT_FOUND;
    }

    auto info = appRecord->GetApplicationInfo();
    if (info == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "appInfo null");
        return ERR_NO_INIT;
    }
    application = *info;
    debug = appRecord->IsDebugApp();
    return ERR_OK;
}
int AppQueryHelper::VerifyAccountPermission(const std::string &permissionName, const int userId) const
{
    int32_t callerUser = IPCSkeleton::GetCallingUid() / BASE_USER_RANGE;
    if (((callerUser != U0_USER_ID && callerUser != U1_USER_ID) ||
        !UserController::GetInstance().IsForegroundUser(userId)) &&
        callerUser != userId) {
        auto isCallingPermAccount = AAFwk::PermissionVerification::GetInstance()->VerifyCallingPermission(
            AAFwk::PermissionConstants::PERMISSION_INTERACT_ACROSS_LOCAL_ACCOUNTS);
        if (!isCallingPermAccount) {
            TAG_LOGE(AAFwkTag::APPMGR, "permission accounts verify fail");
            return ERR_PERMISSION_DENIED;
        }
    }
    auto isCallingPerm = AAFwk::PermissionVerification::GetInstance()->VerifyCallingPermission(permissionName);
    return isCallingPerm ? ERR_OK : ERR_PERMISSION_DENIED;
}

int AppQueryHelper::GetRenderProcessTerminationStatus(pid_t renderPid, int &status)
{
    auto callingPid = IPCSkeleton::GetCallingPid();
    TAG_LOGD(AAFwkTag::APPMGR, "GetRenderProcessTerminationStatus, callingPid:%{public}d, renderPid:%{public}d",
        callingPid, renderPid);
    if (!appRunningManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRunningManager_ null");
        return ERR_INVALID_VALUE;
    }
    auto hostRecord = appRunningManager_->GetAppRunningRecordByPid(callingPid);
    if (!hostRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "hostRecord null");
        return ERR_INVALID_VALUE;
    }
    if (!hostRecord->ConstainsRenderPid(renderPid)) {
        TAG_LOGE(AAFwkTag::APPMGR,
            "permission denied, callingPid:%{public}d, renderPid:%{public}d",
            callingPid, renderPid);
        return ERR_PERMISSION_DENIED;
    }
    if (remoteClientManager_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "remoteClientManager_ null");
        return ERR_INVALID_VALUE;
    }
    auto nwebSpawnClient = remoteClientManager_->GetNWebSpawnClient();
    if (!nwebSpawnClient) {
        TAG_LOGE(AAFwkTag::APPMGR, "nwebSpawnClient null");
        return ERR_INVALID_VALUE;
    }

    AppSpawnStartMsg startMsg;
    startMsg.pid = renderPid;
    startMsg.code = MSG_GET_RENDER_TERMINATION_STATUS;
    ErrCode errCode = nwebSpawnClient->GetRenderProcessTerminationStatus(startMsg, status);
    if (FAILED(errCode)) {
        TAG_LOGE(AAFwkTag::APPMGR, "get render process termination status fail, errCode %{public}08x", errCode);
        return ERR_INVALID_VALUE;
    }
    TAG_LOGD(AAFwkTag::APPMGR, "Get render process termination status success, renderPid:%{public}d, status:%{public}d",
        renderPid, status);
    hostRecord->RemoveRenderPid(renderPid);
    if (removeRenderRecordCallback_) {
        removeRenderRecordCallback_(hostRecord, renderPid);
    } else {
        TAG_LOGE(AAFwkTag::APPMGR, "removeRenderRecordCallback_ is null, renderPid:%{public}d", renderPid);
    }
    return 0;
}

bool AppQueryHelper::GetAppRunningStateByBundleName(const std::string &bundleName)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    if (!appRunningManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "running manager null");
        return false;
    }

    if (!AAFwk::PermissionVerification::GetInstance()->IsSACall()) {
        TAG_LOGE(AAFwkTag::APPMGR, "permission deny not SA");
        return false;
    }

    return appRunningManager_->GetAppRunningStateByBundleName(bundleName);
}

int32_t AppQueryHelper::IsApplicationRunning(const std::string &bundleName, bool &isRunning)
{
    TAG_LOGD(AAFwkTag::APPMGR, "Called, bundleName: %{public}s", bundleName.c_str());
    CHECK_CALLER_IS_SYSTEM_APP;
    if (!CheckGetRunningInfoPermission()) {
        TAG_LOGE(AAFwkTag::APPMGR, "permission verification fail");
        return ERR_PERMISSION_DENIED;
    }

    return appRunningManager_->CheckAppCloneRunningRecordIsExistByBundleName(bundleName, 0, isRunning);
}

int32_t AppQueryHelper::IsAppRunning(const std::string &bundleName, int32_t appCloneIndex,
    bool &isRunning)
{
    TAG_LOGD(AAFwkTag::APPMGR,
        "Called, bundleName: %{public}s, appCloneIndex: %{public}d ", bundleName.c_str(), appCloneIndex);
    if (!CheckGetRunningInfoPermission()) {
        TAG_LOGE(AAFwkTag::APPMGR, "permission verification fail");
        return ERR_PERMISSION_DENIED;
    }
    if (appCloneIndex < 0 || appCloneIndex > AbilityRuntime::GlobalConstant::MAX_APP_CLONE_INDEX) {
        TAG_LOGE(AAFwkTag::APPMGR, "appCloneIndex invalid");
        return AAFwk::ERR_APP_CLONE_INDEX_INVALID;
    }
    if (remoteClientManager_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "remoteClientManager null");
        return ERR_INVALID_OPERATION;
    }
    auto bundleMgrHelper = remoteClientManager_->GetBundleManagerHelper();
    if (bundleMgrHelper == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "bundleMgrHelper null");
        return ERR_INVALID_OPERATION;
    }
    int32_t userId = UserController::GetInstance().GetCallerUserId();
    BundleInfo bundleInfo;
    int32_t bundleMgrResult;
    if (appCloneIndex == 0) {
        bundleMgrResult = IN_PROCESS_CALL(bundleMgrHelper->GetBundleInfoV9(bundleName,
            BUNDLE_INFO_FLAG_WITH_APP_EXT_HAP_PERM, bundleInfo, userId));
    } else {
        bundleMgrResult = IN_PROCESS_CALL(bundleMgrHelper->GetCloneBundleInfo(bundleName,
            static_cast<int32_t>(GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_APPLICATION),
            appCloneIndex, bundleInfo, userId));
    }

    if (bundleMgrResult != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "query bundleInfo fail");
        return AAFwk::ERR_APP_CLONE_INDEX_INVALID;
    }

    return appRunningManager_->CheckAppCloneRunningRecordIsExistByBundleName(bundleName, appCloneIndex, isRunning);
}

int32_t AppQueryHelper::IsAppRunning(const std::string &bundleName, int32_t appCloneIndex,
    int32_t userId, bool &isRunning)
{
    TAG_LOGD(AAFwkTag::APPMGR, "Called, bundleName: %{public}s, appCloneIndex: %{public}d, userId: %{public}d",
        bundleName.c_str(), appCloneIndex, userId);
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    if (callingUid != FOUNDATION_UID && callingUid != FILE_SYSTEM_SERVICE_UID) {
        TAG_LOGE(AAFwkTag::APPMGR, "not foundation or file system service call");
        return ERR_PERMISSION_DENIED;
    }
    if (!CheckGetRunningInfoPermission()) {
        TAG_LOGE(AAFwkTag::APPMGR, "permission verification fail");
        return ERR_PERMISSION_DENIED;
    }
    if (appCloneIndex < 0 || appCloneIndex > AbilityRuntime::GlobalConstant::MAX_APP_CLONE_INDEX) {
        TAG_LOGE(AAFwkTag::APPMGR, "appCloneIndex invalid, bundleName: %{public}s, appCloneIndex: %{public}d",
            bundleName.c_str(), appCloneIndex);
        return AAFwk::ERR_APP_CLONE_INDEX_INVALID;
    }
    if (userId < 0) {
        TAG_LOGE(AAFwkTag::APPMGR,
            "userId invalid, bundleName: %{public}s, userId: %{public}d", bundleName.c_str(), userId);
        return AAFwk::INVALID_PARAMETERS_ERR;
    }

    return appRunningManager_->IsAppRunningByBundleName(bundleName, appCloneIndex, userId, isRunning);
}

int32_t AppQueryHelper::IsAppRunningByBundleNameAndUserId(const std::string &bundleName, int32_t userId,
    bool &isRunning)
{
    TAG_LOGD(AAFwkTag::APPMGR, "Called, bundleName=%{public}s,userId=%{public}d", bundleName.c_str(), userId);
    if (IPCSkeleton::GetCallingUid() != FOUNDATION_UID) {
        TAG_LOGE(AAFwkTag::APPMGR, "not foundation call");
        return ERR_PERMISSION_DENIED;
    }

    if (appRunningManager_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRunningManager_ null");
        return ERR_NO_INIT;
    }

    if (userId < 0) {
        userId = UserController::GetInstance().GetCallerUserId();
    }

    return appRunningManager_->IsAppRunningByBundleNameAndUserId(bundleName, userId, isRunning);
}

int32_t AppQueryHelper::GetBundleNameByPid(const int32_t pid, std::string &bundleName, int32_t &uid)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    if (!AAFwk::PermissionVerification::GetInstance()->IsSACall()) {
        TAG_LOGE(AAFwkTag::APPMGR, "permission verification fail");
        return ERR_PERMISSION_DENIED;
    }
    auto callerRecord = GetAppRunningRecordByPid(pid);
    if (callerRecord == nullptr) {
        TAG_LOGD(AAFwkTag::APPMGR, "callerRecord null");
        return ERR_INVALID_OPERATION;
    }
    bundleName = callerRecord->GetBundleName();
    uid = callerRecord->GetUid();
    return ERR_OK;
}

int32_t AppQueryHelper::GetRunningProcessInformation(
    const std::string &bundleName, int32_t userId, std::vector<RunningProcessInfo> &info)
{
    CHECK_CALLER_IS_SYSTEM_APP;
    if (!appRunningManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRunningManager null");
        return ERR_NO_INIT;
    }

    if (remoteClientManager_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "remoteClientManager_ null");
        return ERR_NO_INIT;
    }
    auto bundleMgrHelper = remoteClientManager_->GetBundleManagerHelper();
    if (bundleMgrHelper == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "bundleMgrHelper null");
        return ERR_NO_INIT;
    }
    TAG_LOGD(AAFwkTag::APPMGR, "uid value: %{public}d", userId);
    const auto &appRunningRecordMap = appRunningManager_->GetAppRunningRecordMap();
    for (const auto &item : appRunningRecordMap) {
        const auto &appRecord = item.second;
        if (appRecord == nullptr) {
            continue;
        }
        if (GetUserIdByUid(appRecord->GetUid()) != userId) {
            continue;
        }
        auto appInfoList = appRecord->GetAppInfoList();
        for (const auto &appInfo : appInfoList) {
            if (appInfo == nullptr) {
                continue;
            }
            if (appInfo->bundleName == bundleName) {
                GetRunningProcesses(appRecord, info);
                break;
            }
        }
    }
    return ERR_OK;
}

bool AppQueryHelper::IsFinalAppProcessByBundleName(const std::string &bundleName)
{
    if (appRunningManager_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "running manager null");
        return false;
    }

    auto name = bundleName;
    if (bundleName.empty()) {
        auto callingPid = IPCSkeleton::GetCallingPid();
        auto appRecord = appRunningManager_->GetAppRunningRecordByPid(callingPid);
        if (appRecord == nullptr) {
            TAG_LOGE(AAFwkTag::APPMGR, "get running record null");
            return false;
        }
        name = appRecord->GetBundleName();
    }

    auto count = appRunningManager_->GetAllAppRunningRecordCountByBundleName(name);
    TAG_LOGD(AAFwkTag::APPMGR, "Get application %{public}s process list size[%{public}d].", name.c_str(), count);
    return count == 1;
}

int32_t AppQueryHelper::GetAppRunningUniqueIdByPid(pid_t pid, std::string &appRunningUniqueId)
{
    TAG_LOGD(AAFwkTag::APPMGR, "call.");
    if (!appRunningManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRunningManager_ null");
        return ERR_NO_INIT;
    }
    bool isCallingPermission = AAFwk::PermissionVerification::GetInstance()->IsSACall() &&
        AAFwk::PermissionVerification::GetInstance()->VerifyRunningInfoPerm();
    auto isShellCall = AAFwk::PermissionVerification::GetInstance()->IsShellCall();
    auto isLocalDebugOtherAppsCall = AAFwk::PermissionVerification::GetInstance()->IsLocalDebugOtherAppsCall();
    bool isDevelopMode = system::GetBoolParameter(DEVELOPER_MODE_STATE, false);
    if (!isCallingPermission && !(isShellCall && isDevelopMode) && !isLocalDebugOtherAppsCall) {
        TAG_LOGE(AAFwkTag::APPMGR, "GetAppRunningUniqueIdByPid not SA call or verification failed");
        return ERR_PERMISSION_DENIED;
    }
    return appRunningManager_->GetAppRunningUniqueIdByPid(pid, appRunningUniqueId);
}

int32_t AppQueryHelper::GetAllUIExtensionRootHostPid(pid_t pid, std::vector<pid_t> &hostPids)
{
    if (!appRunningManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "running manager null");
        return ERR_NO_INIT;
    }

    CHECK_CALLER_IS_SYSTEM_APP;
    if (!AAFwk::PermissionVerification::GetInstance()->VerifyRunningInfoPerm()) {
        TAG_LOGE(AAFwkTag::APPMGR, "permission deny");
        return ERR_PERMISSION_DENIED;
    }

    return appRunningManager_->GetAllUIExtensionRootHostPid(pid, hostPids);
}

int32_t AppQueryHelper::GetAllUIExtensionProviderPid(pid_t hostPid, std::vector<pid_t> &providerPids)
{
    if (!appRunningManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "running manager null");
        return ERR_NO_INIT;
    }

    CHECK_CALLER_IS_SYSTEM_APP;
    if (!AAFwk::PermissionVerification::GetInstance()->VerifyRunningInfoPerm()) {
        TAG_LOGE(AAFwkTag::APPMGR, "permission deny");
        return ERR_PERMISSION_DENIED;
    }

    return appRunningManager_->GetAllUIExtensionProviderPid(hostPid, providerPids);
}

bool AppQueryHelper::IsMemorySizeSufficient()
{
    return ExitResidentProcessManager::GetInstance().IsMemorySizeSufficient();
}

bool AppQueryHelper::IsNoRequireBigMemory()
{
    return ExitResidentProcessManager::GetInstance().IsNoRequireBigMemory();
}

int32_t AppQueryHelper::IsProcessCacheSupported(int32_t pid, bool &isSupported)
{
    TAG_LOGI(AAFwkTag::APPMGR, "get supported process cache, pid:%{public}d", pid);
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    if (!appRunningManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRunningManager_ is nullptr");
        return AAFwk::ERR_NULL_APP_RUNNING_MANAGER;
    }

    auto appRecord = GetAppRunningRecordByPid(pid);
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "no such appRecord, pid:%{public}d", pid);
        return AAFwk::ERR_NO_APP_RECORD;
    }

    if (!DelayedSingleton<CacheProcessManager>::GetInstance()->QueryEnableProcessCache()) {
        TAG_LOGE(AAFwkTag::APPMGR, "process cache feature disabled");
        return AAFwk::ERR_CAPABILITY_NOT_SUPPORT;
    }
    isSupported = (appRecord->GetSupportProcessCacheState() == SupportProcessCacheState::SUPPORT);
    return ERR_OK;
}

int32_t AppQueryHelper::IsChildProcessSupported(bool isNative, bool &isSupported)
{
    pid_t pid = IPCSkeleton::GetCallingPid();
    TAG_LOGD(AAFwkTag::APPMGR, "IsChildProcessSupported called, pid:%{public}d, isNative:%{public}d", pid, isNative);
    auto appRecord = GetAppRunningRecordByPid(pid);
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "no such appRecord, pid:%{public}d", pid);
        return AAFwk::ERR_NO_APP_RECORD;
    }
    bool isMultiProcessModel = AAFwk::AppUtils::GetInstance().IsMultiProcessModel();
    if (isNative) {
        isSupported = isMultiProcessModel ||
            AAFwk::AppUtils::GetInstance().IsAllowNativeChildProcess(appRecord->GetAppIdentifier()) ||
            AllowChildProcessInMultiProcessFeatureApp(appRecord);
    } else {
        isSupported = isMultiProcessModel || AllowChildProcessInMultiProcessFeatureApp(appRecord);
    }
    return ERR_OK;
}

int32_t AppQueryHelper::CheckCallingIsUserTestModeInner(const pid_t pid, bool &isUserTest)
{
    if (!IsSceneBoardCall()) {
        TAG_LOGE(AAFwkTag::APPMGR, "not call");
        return AAFwk::CHECK_PERMISSION_FAILED;
    }
    if (pid <= 0) {
        TAG_LOGE(AAFwkTag::APPMGR, "hht-invalid pid:%{public}d", pid);
        return ERR_INVALID_VALUE;
    }
    auto appRecord = GetAppRunningRecordByPid(pid);
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "hht-no appRecord");
        return ERR_INVALID_VALUE;
    }
    if (appRecord->GetUserTestInfo() == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "hht-no user test info");
        return ERR_INVALID_VALUE;
    }
    isUserTest = true;
    return ERR_OK;
}

bool AppQueryHelper::IsKilledForUpgradeWeb(const std::string &bundleName) const
{
    auto callerUid = IPCSkeleton::GetCallingUid();
    if (callerUid != FOUNDATION_UID) {
        TAG_LOGE(AAFwkTag::APPMGR, "not foundation call");
        return false;
    }
    return ExitResidentProcessManager::GetInstance().IsKilledForUpgradeWeb(bundleName);
}

bool AppQueryHelper::IsProcessContainsOnlyUIAbility(const pid_t pid)
{
    auto appRecord = GetAppRunningRecordByPid(pid);
    if (appRecord == nullptr) {
        return false;
    }

    auto abilityRecordList = appRecord->GetAbilities();

    for (auto it = abilityRecordList.begin(); it != abilityRecordList.end(); ++it) {
        if (it->second == nullptr) {
            return false;
        }
        auto abilityInfo = it->second->GetAbilityInfo();
        if (abilityInfo == nullptr) {
            return false;
        }

        bool isUIAbility = (abilityInfo->type == AppExecFwk::AbilityType::PAGE);
        if (!isUIAbility) {
            return false;
        }
    }
    return true;
}

bool AppQueryHelper::IsProcessAttached(sptr<IRemoteObject> token) const
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    if (IPCSkeleton::GetCallingUid() != FOUNDATION_UID) {
        TAG_LOGE(AAFwkTag::APPMGR, "not foundation call");
        return false;
    }
    auto appRecord = GetAppRunningRecordByAbilityToken(token);
    if (appRecord == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "abilityRecord null");
        return false;
    }
    return appRecord->IsProcessAttached();
}

bool AppQueryHelper::IsCallerKilling(const std::string& callerKey) const
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    if (IPCSkeleton::GetCallingUid() != FOUNDATION_UID) {
        TAG_LOGE(AAFwkTag::APPMGR, "Not foundation call.");
        return false;
    }
    return KillingProcessManager::GetInstance().IsCallerKilling(callerKey);
}

int32_t AppQueryHelper::GetSupportedProcessCachePids(const std::string &bundleName,
    std::vector<int32_t> &pidList)
{
    auto cachePrcoMgr = DelayedSingleton<CacheProcessManager>::GetInstance();
    if (cachePrcoMgr == nullptr || appRunningManager_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "inner manager null");
        return AAFwk::INNER_ERR;
    }
    pidList.clear();
    int32_t callderUserId = -1;
    int32_t getOsAccountRet = OsAccountManagerWrapper::GetOsAccountLocalIdFromUid(
        IPCSkeleton::GetCallingUid(), callderUserId);
    if (getOsAccountRet != 0) {
        TAG_LOGE(AAFwkTag::APPMGR, "get caller local id fail. ret: %{public}d", getOsAccountRet);
        return AAFwk::INNER_ERR;
    }
    TAG_LOGD(AAFwkTag::APPMGR, "callderUserId: %{public}d", callderUserId);
    for (const auto &item : appRunningManager_->GetAppRunningRecordMap()) {
        auto appRecord = item.second;
        if (appRecord == nullptr) {
            continue;
        }
        int32_t procUserId = -1;
        int32_t procGetOsAccountRet = OsAccountManagerWrapper::GetOsAccountLocalIdFromUid(
            appRecord->GetUid(), procUserId);
        if (appRecord->GetBundleName() == bundleName && procGetOsAccountRet == 0 &&
            procUserId == callderUserId && cachePrcoMgr->IsAppSupportProcessCache(appRecord) &&
            appRecord->GetPriorityObject() != nullptr) {
            TAG_LOGD(AAFwkTag::APPMGR,
                "procUserId: %{public}d, procGetOsAccountRet: %{public}d", procUserId, procGetOsAccountRet);
            pidList.push_back(appRecord->GetPid());
        }
    }
    return ERR_OK;
}

int32_t AppQueryHelper::CheckIsKiaProcess(pid_t pid, bool &isKia)
{
    TAG_LOGI(AAFwkTag::APPMGR, "call");
    if (!AAFwk::AppUtils::GetInstance().IsStartOptionsWithAnimation() ||
        !AAFwk::PermissionVerification::GetInstance()->VerifySuperviseKiaServicePermission()) {
        TAG_LOGE(AAFwkTag::APPMGR, "no kia permission.");
        return ERR_PERMISSION_DENIED;
    }
    if (!appRunningManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRunningManager_ is nullptr");
        return ERR_INVALID_VALUE;
    }
    return appRunningManager_->CheckIsKiaProcess(pid, isKia);
}

int32_t AppQueryHelper::QueryRunningSharedBundles(pid_t pid, std::map<std::string, uint32_t> &sharedBundles)
{
    TAG_LOGD(AAFwkTag::APPMGR, "QueryRunningSharedBundles call, pid:%{public}d", pid);
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto appRecord = GetAppRunningRecordByPid(pid);
    if (!appRecord) {
        TAG_LOGW(AAFwkTag::APPMGR, "No app record for pid: %{public}d", pid);
        return AAFwk::ERR_NO_APP_RECORD;
    }
    const auto &hspList = appRecord->GetStartMsg().hspList;
    for (const auto &item: hspList) {
        sharedBundles.emplace(item.bundleName, item.versionCode);
    }
    return ERR_OK;
}

int32_t AppQueryHelper::GetAllAbilityInfos(const int32_t pid, std::vector<AppExecFwk::AbilityStateData> &infos)
{
    if (!appRunningManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRunningManager_ is nullptr");
        return ERR_NO_INIT;
    }
    return appRunningManager_->GetAllAbilityInfos(pid, infos);
}

std::shared_ptr<AppRunningRecord> AppQueryHelper::GetAppRunningRecordByPid(const pid_t pid) const
{
    if (!appRunningManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRunningManager null");
        return nullptr;
    }
    return appRunningManager_->GetAppRunningRecordByPid(pid);
}

std::shared_ptr<AppRunningRecord> AppQueryHelper::GetAppRunningRecordByAbilityToken(
    const sptr<IRemoteObject> &abilityToken) const
{
    if (!appRunningManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRunningManager_ null");
        return nullptr;
    }

    return appRunningManager_->GetAppRunningRecordByAbilityToken(abilityToken);
}

bool AppQueryHelper::SetPreloadFlagForProcessInfo(const std::shared_ptr<AppRunningRecord> &appRecord)
{
    if (appRecord == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRecord null");
        return false;
    }
    if (appRecord->GetPreloadMode() != PreloadMode::PRELOAD_NONE || appRecord->GetUIExtensionPreloadState()) {
        TAG_LOGD(AAFwkTag::APPMGR,
            "Preload process: %{public}s, mode=%{public}d, isPreload: %{public}d",
            appRecord->GetProcessName().c_str(), appRecord->GetPreloadMode(), appRecord->GetUIExtensionPreloadState());
        return true;
    }

    return false;
}

#ifdef SUPPORT_CHILD_PROCESS
int32_t AppQueryHelper::GetChildProcessInfo(const std::shared_ptr<ChildProcessRecord> childProcessRecord,
    const std::shared_ptr<AppRunningRecord> appRecord, ChildProcessInfo &info)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    if (!childProcessRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "no child process record");
        return ERR_NAME_NOT_FOUND;
    }
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "no appRecord");
        return ERR_NAME_NOT_FOUND;
    }
    int32_t userId = -1;
    int errCode = OsAccountManagerWrapper::GetOsAccountLocalIdFromUid(appRecord->GetUid(), userId);
    if (errCode != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "GetOsAccountLocalIdFromUid failed,errcode=%{public}d", errCode);
        return errCode;
    }
    TAG_LOGD(AAFwkTag::APPMGR, "GetOsAccountLocalIdFromUid userId: %{public}d", userId);
    info.userId = userId;
    info.pid = childProcessRecord->GetPid();
    info.hostPid = childProcessRecord->GetHostPid();
    info.uid = childProcessRecord->GetUid();
    info.hostUid = appRecord->GetUid();
    info.bundleName = appRecord->GetBundleName();
    info.processName = childProcessRecord->GetProcessName();
    return ERR_OK;
}
#endif // SUPPORT_CHILD_PROCESS

bool AppQueryHelper::AllowChildProcessInMultiProcessFeatureApp(std::shared_ptr<AppRunningRecord> appRecord)
{
    if (!AAFwk::AppUtils::GetInstance().AllowChildProcessInMultiProcessFeatureApp()) {
        return false;
    }
    CHECK_POINTER_AND_RETURN_VALUE(appRecord, false);
    auto support = appRecord->IsSupportMultiProcessDeviceFeature();
    if (support.has_value()) {
        return support.value();
    }
    auto bundleMgrHelper = remoteClientManager_->GetBundleManagerHelper();
    CHECK_POINTER_AND_RETURN_VALUE(bundleMgrHelper, false);
    BundleInfo bundleInfo;
    auto ret = IN_PROCESS_CALL(bundleMgrHelper->GetBundleInfoV9(appRecord->GetBundleName(),
        static_cast<int32_t>(GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_HAP_MODULE), bundleInfo, appRecord->GetUserId()));
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "getBundleInfo fail");
        return false;
    }
    for (const auto &info : bundleInfo.hapModuleInfos) {
        if (info.moduleType != AppExecFwk::ModuleType::ENTRY) {
            continue;
        }
        auto deviceType = OHOS::system::GetDeviceType();
        auto it = info.requiredDeviceFeatures.find(deviceType);
        if (it == info.requiredDeviceFeatures.end()) {
            return false;
        }
        auto &deviceFeatures = it->second;
        auto supportMultiProcess =
            std::find(deviceFeatures.begin(), deviceFeatures.end(), LARGE_SCREEN) != deviceFeatures.end();
        appRecord->SetSupportMultiProcessDeviceFeature(supportMultiProcess);
        return supportMultiProcess;
    }
    return false;
}

}  // namespace AppExecFwk
}  // namespace OHOS
