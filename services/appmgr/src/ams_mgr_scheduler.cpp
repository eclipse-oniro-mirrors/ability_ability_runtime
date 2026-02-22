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

#include "ams_mgr_scheduler.h"
#include <sys/types.h>

#include "ability_manager_errors.h"
#include "datetime_ex.h"
#include "ipc_skeleton.h"
#include "system_ability_definition.h"

#include "accesstoken_kit.h"
#include "app_death_recipient.h"
#include "app_mgr_constants.h"
#include "app_utils.h"
#include "hilog_tag_wrapper.h"
#include "perf_profile.h"
#include "permission_constants.h"
#include "permission_verification.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
constexpr const char* TASK_TERMINATE_ABILITY = "TerminateAbilityTask";
constexpr const char* TASK_UPDATE_ABILITY_STATE = "UpdateAbilityStateTask";
constexpr const char* TASK_UPDATE_EXTENSION_STATE = "UpdateExtensionStateTask";
constexpr const char* TASK_REGISTER_APP_STATE_CALLBACK = "RegisterAppStateCallbackTask";
constexpr const char* TASK_STOP_ALL_PROCESS = "StopAllProcessTask";
constexpr const char* TASK_ABILITY_BEHAVIOR_ANALYSIS = "AbilityBehaviorAnalysisTask";
constexpr const char* TASK_KILL_PROCESS_BY_ABILITY_TOKEN = "KillProcessByAbilityTokenTask";
constexpr const char* TASK_KILL_PROCESSES_BY_USERID = "KillProcessesByUserIdTask";
constexpr const char* TASK_ATTACH_PID_TO_PARENT = "AttachPidToParent";
constexpr const char* TASK_KILL_APPLICATION = "KillApplicationTask";
constexpr const char* TASK_CLEAR_PROCESS_BY_ABILITY_TOKEN = "ClearProcessByAbilityTokenTask";
constexpr const char* TASK_NOTIFY_LOAD_ABILITY_FINISHED = "NotifyLoadAbilityFinished";
constexpr const char* FOUNDATION_NAME = "foundation";
constexpr const char* SCENE_BOARD_BUNDLE_NAME = "com.ohos.sceneboard";
constexpr const char* SCENEBOARD_ABILITY_NAME = "com.ohos.sceneboard.MainAbility";
constexpr const char* TASK_SCENE_BOARD_ATTACH_TIMEOUT = "sceneBoardAttachTimeoutTask";
constexpr const char* TASK_ATTACHED_TO_STATUS_BAR = "AttachedToStatusBar";
constexpr const char* TASK_BLOCK_PROCESS_CACHE_BY_PIDS = "BlockProcessCacheByPids";
constexpr const char* POWER_OFF_ABILITY = "PoweroffAbility";
constexpr int32_t SCENE_BOARD_ATTACH_TIMEOUT_TASK_TIME = 1000;
constexpr int32_t LOAD_TASK_TIMEOUT = 60000; // ms
};  // namespace

AmsMgrScheduler::AmsMgrScheduler(
    const std::shared_ptr<AppMgrServiceInner> &mgrServiceInner_,
    const std::shared_ptr<AAFwk::TaskHandlerWrap> &handler_)
    : amsMgrServiceInner_(mgrServiceInner_), amsHandler_(handler_)
{}

AmsMgrScheduler::~AmsMgrScheduler()
{
    TAG_LOGI(AAFwkTag::APPMGR, "call");
}

void AmsMgrScheduler::LoadAbility(const std::shared_ptr<AbilityInfo> &abilityInfo,
    const std::shared_ptr<ApplicationInfo> &appInfo, const std::shared_ptr<AAFwk::Want> &want,
    std::shared_ptr<AbilityRuntime::LoadParam> loadParam)
{
    if (!abilityInfo || !appInfo) {
        TAG_LOGE(AAFwkTag::APPMGR, "param error");
        return;
    }

    if (!IsReady()) {
        return;
    }

    if (amsMgrServiceInner_->VerifyRequestPermission() != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "verification failed");
        return;
    }
    PerfProfile::GetInstance().SetAbilityLoadStartTime(GetTickCount());
    TAG_LOGI(AAFwkTag::PROCESSMGR, "SubmitLoadTask: %{public}s-%{public}s", abilityInfo->bundleName.c_str(),
        abilityInfo->name.c_str());
    std::function<void()> loadAbilityFunc = [amsMgrServiceInner = amsMgrServiceInner_,
        abilityInfo, appInfo, want, loadParam]() {
        amsMgrServiceInner->LoadAbility(abilityInfo, appInfo, want, loadParam);
    };

    // cache other application load ability task before scene board attach
    if (!amsMgrServiceInner_->GetSceneBoardAttachFlag() && abilityInfo->bundleName != SCENE_BOARD_BUNDLE_NAME) {
        amsMgrServiceInner_->CacheLoadAbilityTask(std::move(loadAbilityFunc));
        return;
    }
    if (abilityInfo->bundleName == SCENE_BOARD_BUNDLE_NAME && abilityInfo->name == SCENEBOARD_ABILITY_NAME) {
        amsMgrServiceInner_->SetSceneBoardAttachFlag(false);
        // set scene board attach timeout task
        std::weak_ptr<AppMgrServiceInner> amsMgrServiceInner = amsMgrServiceInner_;
        auto timeoutTask = [amsMgrServiceInner]() {
            auto inner = amsMgrServiceInner.lock();
            if (inner != nullptr) {
                inner->SetSceneBoardAttachFlag(true);
            }
        };
        amsHandler_->SubmitTask(timeoutTask, TASK_SCENE_BOARD_ATTACH_TIMEOUT, SCENE_BOARD_ATTACH_TIMEOUT_TASK_TIME);
    }

    AAFwk::TaskAttribute taskAttr{
        .taskName_ = "LoadAbilityTask",
        .taskQos_ = AAFwk::TaskQoS::USER_INTERACTIVE,
        .timeoutMillis_ = LOAD_TASK_TIMEOUT
    };

    if (abilityInfo->bundleName == AAFwk::AppUtils::GetInstance().GetMigrateClientBundleName()) {
        taskAttr.taskPriority_ = AAFwk::TaskQueuePriority::IMMEDIATE;
    }
    if (abilityInfo->bundleName == SCENE_BOARD_BUNDLE_NAME && abilityInfo->name == POWER_OFF_ABILITY) {
        taskAttr.insertHead_ = true;
    }

    amsHandler_->SubmitTask(loadAbilityFunc, taskAttr);
}

void AmsMgrScheduler::NotifyLoadAbilityFinished(pid_t callingPid, pid_t targetPid, uint64_t callbackId)
{
    if (!IsReady()) {
        return;
    }

    if (!AAFwk::PermissionVerification::GetInstance()->CheckSpecificSystemAbilityAccessPermission(FOUNDATION_NAME)) {
        TAG_LOGE(AAFwkTag::APPMGR, "not foundation call");
        return;
    }
    std::function<void()> notifyFunc = [amsMgrServiceInner = amsMgrServiceInner_, callingPid, targetPid,
        callbackId] () {
        amsMgrServiceInner->NotifyLoadAbilityFinished(callingPid, targetPid, callbackId);
    };
    amsHandler_->SubmitTask(notifyFunc, AAFwk::TaskAttribute{
        .taskName_ = TASK_NOTIFY_LOAD_ABILITY_FINISHED,
        .taskQos_ = AAFwk::TaskQoS::USER_INTERACTIVE
    });
}

void AmsMgrScheduler::UpdateAbilityState(const sptr<IRemoteObject> &token, const AbilityState state)
{
    if (!IsReady()) {
        return;
    }

    if (amsMgrServiceInner_->VerifyRequestPermission() != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "verification failed");
        return;
    }
    std::function<void()> updateAbilityStateFunc = [amsMgrServiceInner = amsMgrServiceInner_, token, state] () {
        amsMgrServiceInner->UpdateAbilityState(token, state);
    };
    amsHandler_->SubmitTask(updateAbilityStateFunc, AAFwk::TaskAttribute{
        .taskName_ = TASK_UPDATE_ABILITY_STATE,
        .taskQos_ = AAFwk::TaskQoS::USER_INTERACTIVE
    });
}

void AmsMgrScheduler::UpdateExtensionState(const sptr<IRemoteObject> &token, const ExtensionState state)
{
    if (!IsReady()) {
        return;
    }

    if (amsMgrServiceInner_->VerifyRequestPermission() != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "verification failed");
        return;
    }
    std::function<void()> updateExtensionStateFunc = [amsMgrServiceInner = amsMgrServiceInner_, token, state]() {
        amsMgrServiceInner->UpdateExtensionState(token, state);
    };
    amsHandler_->SubmitTask(updateExtensionStateFunc, AAFwk::TaskAttribute{
        .taskName_ = TASK_UPDATE_EXTENSION_STATE,
        .taskQos_ = AAFwk::TaskQoS::USER_INTERACTIVE
    });
}

void AmsMgrScheduler::TerminateAbility(const sptr<IRemoteObject> &token, bool clearMissionFlag)
{
    if (!IsReady()) {
        return;
    }

    if (amsMgrServiceInner_->VerifyRequestPermission() != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "verification failed");
        return;
    }
    std::function<void()> terminateAbilityFunc = [amsMgrServiceInner = amsMgrServiceInner_, token, clearMissionFlag]() {
        amsMgrServiceInner->TerminateAbility(token, clearMissionFlag);
    };
    amsHandler_->SubmitTask(terminateAbilityFunc, AAFwk::TaskAttribute{
        .taskName_ = TASK_TERMINATE_ABILITY,
        .taskQos_ = AAFwk::TaskQoS::USER_INTERACTIVE
    });
}

void AmsMgrScheduler::RegisterAppStateCallback(const sptr<IAppStateCallback> &callback)
{
    if (!AAFwk::PermissionVerification::GetInstance()->IsSACall()) {
        TAG_LOGE(AAFwkTag::APPMGR, "caller is not SA");
        return;
    }
    if (!IsReady()) {
        return;
    }
    std::function<void()> registerAppStateCallbackFunc = [amsMgrServiceInner = amsMgrServiceInner_, callback]() {
        amsMgrServiceInner->RegisterAppStateCallback(callback);
    };
    amsHandler_->SubmitTask(registerAppStateCallbackFunc, TASK_REGISTER_APP_STATE_CALLBACK);
}

void AmsMgrScheduler::KillProcessByAbilityToken(const sptr<IRemoteObject> &token)
{
    if (!IsReady()) {
        return;
    }

    if (amsMgrServiceInner_->VerifyKillProcessPermission(token) != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "verification failed");
        return;
    }

    std::function<void()> killProcessByAbilityTokenFunc = [amsMgrServiceInner = amsMgrServiceInner_, token]() {
        amsMgrServiceInner->KillProcessByAbilityToken(token);
    };
    amsHandler_->SubmitTask(killProcessByAbilityTokenFunc, TASK_KILL_PROCESS_BY_ABILITY_TOKEN);
}

void AmsMgrScheduler::KillProcessesByUserId(int32_t userId, bool isNeedSendAppSpawnMsg,
    sptr<AAFwk::IUserCallback> callback)
{
    if (!IsReady()) {
        return;
    }

    if (!AAFwk::PermissionVerification::GetInstance()->JudgeCallerIsAllowedToUseSystemAPI()) {
        TAG_LOGE(AAFwkTag::APPMGR, "caller is not SA");
        return;
    }

    bool isCallingFromFoundation =
        AAFwk::PermissionVerification::GetInstance()->CheckSpecificSystemAbilityAccessPermission(FOUNDATION_NAME);
    auto permission = AAFwk::PermissionConstants::PERMISSION_CLEAN_BACKGROUND_PROCESSES;
    if (!isCallingFromFoundation &&
        amsMgrServiceInner_->VerifyAccountPermission(permission, userId) == ERR_PERMISSION_DENIED) {
        TAG_LOGE(AAFwkTag::APPMGR, "verification failed");
        return;
    }

    std::function<void()> killProcessesByUserIdFunc = [amsMgrServiceInner = amsMgrServiceInner_, userId,
                                                          isNeedSendAppSpawnMsg, callback]() {
        amsMgrServiceInner->KillProcessesByUserId(userId, isNeedSendAppSpawnMsg, callback);
    };
    amsHandler_->SubmitTask(killProcessesByUserIdFunc, TASK_KILL_PROCESSES_BY_USERID);
}

int32_t AmsMgrScheduler::KillProcessesByPids(const std::vector<int32_t> &pids, const std::string &reason,
    bool subProcess, bool isKillPrecedeStart)
{
    if (!IsReady()) {
        return ERR_INVALID_OPERATION;
    }

    pid_t callingPid = IPCSkeleton::GetCallingPid();
    pid_t pid = getprocpid();
    if (callingPid != pid) {
        TAG_LOGE(AAFwkTag::APPMGR, "not allow other process to call");
        return ERR_PERMISSION_DENIED;
    }

    return amsMgrServiceInner_->KillProcessesByPids(pids, reason, subProcess, isKillPrecedeStart);
}

void AmsMgrScheduler::AttachPidToParent(const sptr<IRemoteObject> &token, const sptr<IRemoteObject> &callerToken)
{
    if (!IsReady()) {
        return;
    }

    pid_t callingPid = IPCSkeleton::GetCallingPid();
    pid_t pid = getprocpid();
    if (callingPid != pid) {
        TAG_LOGE(AAFwkTag::APPMGR, "not allow other process to call");
        return;
    }

    std::function<void()> attachPidToParentFunc = [amsMgrServiceInner = amsMgrServiceInner_, token, callerToken]() {
        amsMgrServiceInner->AttachPidToParent(token, callerToken);
    };
    amsHandler_->SubmitTask(attachPidToParentFunc, TASK_ATTACH_PID_TO_PARENT);
}

int32_t AmsMgrScheduler::KillProcessWithAccount(
    const std::string &bundleName, const int accountId, const bool clearPageStack, int32_t appIndex)
{
    TAG_LOGI(AAFwkTag::APPMGR, "bundle=%{public}s, appIndex=%{public}d, userId=%{public}d, clearPageStack=%{public}d",
        bundleName.c_str(), appIndex, accountId, clearPageStack);
    if (!IsReady()) {
        return ERR_INVALID_OPERATION;
    }
    return amsMgrServiceInner_->KillApplicationByUserId(bundleName, appIndex, accountId, clearPageStack,
        "KillProcessWithAccount");
}

int32_t AmsMgrScheduler::KillProcessesInBatch(const std::vector<int32_t> &pids)
{
    TAG_LOGI(AAFwkTag::APPMGR, "pids.size=%{public}zu", pids.size());
    if (!IsReady()) {
        return ERR_INVALID_OPERATION;
    }
    return amsMgrServiceInner_->KillProcessesInBatch(pids);
}

void AmsMgrScheduler::AbilityAttachTimeOut(const sptr<IRemoteObject> &token)
{
    TAG_LOGI(AAFwkTag::APPMGR, "call");
    if (!IsReady()) {
        return;
    }

    if (amsMgrServiceInner_->VerifyRequestPermission() != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "verification failed");
        return;
    }
    auto task = [amsMgrServiceInner = amsMgrServiceInner_, token]() {
        amsMgrServiceInner->HandleAbilityAttachTimeOut(token);
    };
    amsHandler_->SubmitTask(task, "AbilityAttachTimeOut");
}

void AmsMgrScheduler::PrepareTerminate(const sptr<IRemoteObject> &token, bool clearMissionFlag)
{
    TAG_LOGD(AAFwkTag::APPMGR, "Notify AppMgrService to prepare to terminate the ability.");
    if (!IsReady()) {
        return;
    }

    if (amsMgrServiceInner_->VerifyRequestPermission() != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "verification failed");
        return;
    }
    auto task = [=]() { amsMgrServiceInner_->PrepareTerminate(token, clearMissionFlag); };
    amsHandler_->SubmitTask(task, {
        .taskName_ = "PrepareTerminate",
        .taskQos_ = AAFwk::TaskQoS::USER_INTERACTIVE
    });
}

int32_t AmsMgrScheduler::UpdateApplicationInfoInstalled(const std::string &bundleName, const int uid,
    const std::string &moduleName, bool isPlugin)
{
    if (!IsReady()) {
        return ERR_INVALID_OPERATION;
    }

    return amsMgrServiceInner_->UpdateApplicationInfoInstalled(bundleName, uid, moduleName, isPlugin);
}

int32_t AmsMgrScheduler::KillApplication(const std::string &bundleName, bool clearPageStack, int32_t appIndex,
    const std::string &reason)
{
    TAG_LOGI(AAFwkTag::APPMGR, "bundleName = %{public}s, clearPageStack = %{public}d",
        bundleName.c_str(), clearPageStack);
    if (!IsReady()) {
        return ERR_INVALID_OPERATION;
    }

    return amsMgrServiceInner_->KillApplication(bundleName, clearPageStack, appIndex, reason);
}

int32_t AmsMgrScheduler::ForceKillApplication(const std::string &bundleName,
    const int userId, const int appIndex)
{
    TAG_LOGI(AAFwkTag::APPMGR, "bundleName=%{public}s,userId=%{public}d,apIndex=%{public}d",
        bundleName.c_str(), userId, appIndex);
    if (!IsReady()) {
        return ERR_INVALID_OPERATION;
    }

    return amsMgrServiceInner_->ForceKillApplication(bundleName, userId, appIndex);
}

int32_t AmsMgrScheduler::KillProcessesByAccessTokenId(const uint32_t accessTokenId)
{
    TAG_LOGI(AAFwkTag::APPMGR, "accessTokenId=%{public}d", accessTokenId);
    if (!IsReady()) {
        return ERR_INVALID_OPERATION;
    }

    return amsMgrServiceInner_->KillProcessesByAccessTokenId(accessTokenId);
}

int32_t AmsMgrScheduler::KillApplicationByUid(const std::string &bundleName, const int uid,
    const std::string& reason)
{
    TAG_LOGI(AAFwkTag::APPMGR, "bundleName = %{public}s, uid = %{public}d", bundleName.c_str(), uid);
    if (!IsReady()) {
        return ERR_INVALID_OPERATION;
    }
    return amsMgrServiceInner_->KillApplicationByUid(bundleName, uid, reason);
}

int32_t AmsMgrScheduler::NotifyUninstallOrUpgradeApp(const std::string &bundleName, int32_t uid, bool isUpgrade)
{
    TAG_LOGI(AAFwkTag::APPMGR, "bundleName = %{public}s, uid = %{public}d", bundleName.c_str(), uid);
    if (!IsReady()) {
        return ERR_INVALID_OPERATION;
    }

    pid_t callingPid = IPCSkeleton::GetCallingPid();
    pid_t pid = getprocpid();
    if (callingPid != pid) {
        TAG_LOGE(AAFwkTag::APPMGR, "not allow other process to call");
        return ERR_PERMISSION_DENIED;
    }
    
    return amsMgrServiceInner_->NotifyUninstallOrUpgradeApp(bundleName, uid, isUpgrade);
}

void AmsMgrScheduler::NotifyUninstallOrUpgradeAppEnd(int32_t uid)
{
    TAG_LOGI(AAFwkTag::APPMGR, "uid = %{public}d", uid);
    if (!IsReady()) {
        return;
    }

    pid_t callingPid = IPCSkeleton::GetCallingPid();
    pid_t pid = getprocpid();
    if (callingPid != pid) {
        TAG_LOGE(AAFwkTag::APPMGR, "not allow other process to call");
        return;
    }

    amsMgrServiceInner_->NotifyUninstallOrUpgradeAppEnd(uid);
}

int32_t AmsMgrScheduler::KillApplicationSelf(const bool clearPageStack, const std::string& reason)
{
    if (!IsReady()) {
        return ERR_INVALID_OPERATION;
    }
    return amsMgrServiceInner_->KillApplicationSelf(clearPageStack, reason);
}

bool AmsMgrScheduler::IsReady() const
{
    if (!amsMgrServiceInner_) {
        TAG_LOGE(AAFwkTag::APPMGR, "null amsMgrServiceInner_");
        return false;
    }
    if (!amsHandler_) {
        TAG_LOGE(AAFwkTag::APPMGR, "null amsHandler_");
        return false;
    }
    return true;
}

void AmsMgrScheduler::GetRunningProcessInfoByToken(
    const sptr<IRemoteObject> &token, AppExecFwk::RunningProcessInfo &info)
{
    if (!IsReady()) {
        return;
    }

    amsMgrServiceInner_->GetRunningProcessInfoByToken(token, info);
}

void AmsMgrScheduler::SetAbilityForegroundingFlagToAppRecord(const pid_t pid)
{
    if (!IsReady()) {
        return;
    }
    amsMgrServiceInner_->SetAbilityForegroundingFlagToAppRecord(pid);
}

void AmsMgrScheduler::StartSpecifiedAbility(const AAFwk::Want &want, const AppExecFwk::AbilityInfo &abilityInfo,
    int32_t requestId, const std::string &customProcess)
{
    if (!IsReady()) {
        return;
    }

    if (amsMgrServiceInner_->VerifyRequestPermission() != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "verification failed");
        return;
    }
    auto task = [=]() { amsMgrServiceInner_->StartSpecifiedAbility(want, abilityInfo, requestId, customProcess); };
    amsHandler_->SubmitTask(task, {
        .taskName_ = "StartSpecifiedAbility",
        .taskQos_ = AAFwk::TaskQoS::USER_INTERACTIVE
    });
}

void AmsMgrScheduler::PrepareTerminateApp(const pid_t pid, const std::string &moduleName)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    if (!IsReady()) {
        TAG_LOGW(AAFwkTag::APPMGR, "not ready");
        return;
    }
    amsMgrServiceInner_->SchedulePrepareTerminate(pid, moduleName);
}

void AmsMgrScheduler::StartSpecifiedProcess(const AAFwk::Want &want, const AppExecFwk::AbilityInfo &abilityInfo,
    int32_t requestId, const std::string &customProcess)
{
    if (!IsReady()) {
        TAG_LOGW(AAFwkTag::APPMGR, "not ready");
        return;
    }

    if (amsMgrServiceInner_->VerifyRequestPermission() != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "verification failed");
        return;
    }
    auto task = [=]() { amsMgrServiceInner_->StartSpecifiedProcess(want, abilityInfo, requestId, customProcess); };
    amsHandler_->SubmitTask(task, {
        .taskName_ = "StartSpecifiedProcess",
        .taskQos_ = AAFwk::TaskQoS::USER_INTERACTIVE
    });
}

void AmsMgrScheduler::RegisterStartSpecifiedAbilityResponse(const sptr<IStartSpecifiedAbilityResponse> &response)
{
    if (!AAFwk::PermissionVerification::GetInstance()->IsSACall()) {
        TAG_LOGE(AAFwkTag::APPMGR, "caller is not SA");
        return;
    }
    if (!IsReady()) {
        return;
    }
    auto task = [=]() { amsMgrServiceInner_->RegisterStartSpecifiedAbilityResponse(response); };
    amsHandler_->SubmitTask(task, "RegisterStartSpecifiedAbilityResponse");
}

int AmsMgrScheduler::GetApplicationInfoByProcessID(const int pid, AppExecFwk::ApplicationInfo &application, bool &debug)
{
    if (!IsReady()) {
        return ERR_INVALID_OPERATION;
    }
    return amsMgrServiceInner_->GetApplicationInfoByProcessID(pid, application, debug);
}

int32_t AmsMgrScheduler::NotifyAppMgrRecordExitReason(int32_t pid, int32_t reason, const std::string &exitMsg)
{
    if (!IsReady()) {
        return ERR_INVALID_OPERATION;
    }
    return amsMgrServiceInner_->NotifyAppMgrRecordExitReason(pid, reason, exitMsg);
}

int32_t AmsMgrScheduler::NotifyAppMgrRecordExitReasonCompability(
    int32_t pid, int32_t killId, const std::string &killMsg, const std::string &innerMsg)
{
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "not ready");
        return ERR_INVALID_OPERATION;
    }
    return amsMgrServiceInner_->NotifyAppMgrRecordExitReasonCompability(pid, killId, killMsg, innerMsg);
}

void AmsMgrScheduler::SetEnableStartProcessFlagByUserId(int32_t userId, bool enableStartProcess)
{
    if (!IsReady()) {
        return;
    }
    if (amsMgrServiceInner_->VerifyRequestPermission() != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "verification failed");
        return;
    }
    amsMgrServiceInner_->SetEnableStartProcessFlagByUserId(userId, enableStartProcess);
}

int32_t AmsMgrScheduler::GetBundleNameByPid(const int pid, std::string &bundleName, int32_t &uid)
{
    if (!IsReady()) {
        return ERR_INVALID_OPERATION;
    }
    return amsMgrServiceInner_->GetBundleNameByPid(pid, bundleName, uid);
}

int32_t AmsMgrScheduler::RegisterAppDebugListener(const sptr<IAppDebugListener> &listener)
{
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "not ready");
        return ERR_INVALID_OPERATION;
    }
    return amsMgrServiceInner_->RegisterAppDebugListener(listener);
}

int32_t AmsMgrScheduler::UnregisterAppDebugListener(const sptr<IAppDebugListener> &listener)
{
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "not ready");
        return ERR_INVALID_OPERATION;
    }
    return amsMgrServiceInner_->UnregisterAppDebugListener(listener);
}

int32_t AmsMgrScheduler::AttachAppDebug(const std::string &bundleName, bool isDebugFromLocal)
{
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "not ready");
        return ERR_INVALID_OPERATION;
    }
    if (!AAFwk::PermissionVerification::GetInstance()->CheckSpecificSystemAbilityAccessPermission(FOUNDATION_NAME)) {
        TAG_LOGE(AAFwkTag::APPMGR, "caller is not foundation");
        return ERR_INVALID_OPERATION;
    }
    return amsMgrServiceInner_->AttachAppDebug(bundleName, isDebugFromLocal);
}

int32_t AmsMgrScheduler::DetachAppDebug(const std::string &bundleName)
{
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "not ready");
        return ERR_INVALID_OPERATION;
    }
    if (!AAFwk::PermissionVerification::GetInstance()->CheckSpecificSystemAbilityAccessPermission(FOUNDATION_NAME)) {
        TAG_LOGE(AAFwkTag::APPMGR, "caller is not foundation");
        return ERR_INVALID_OPERATION;
    }
    return amsMgrServiceInner_->DetachAppDebug(bundleName);
}

int32_t AmsMgrScheduler::SetAppWaitingDebug(const std::string &bundleName, bool isPersist)
{
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "not ready");
        return ERR_INVALID_OPERATION;
    }
    return amsMgrServiceInner_->SetAppWaitingDebug(bundleName, isPersist);
}

int32_t AmsMgrScheduler::CancelAppWaitingDebug()
{
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "not ready");
        return ERR_INVALID_OPERATION;
    }
    return amsMgrServiceInner_->CancelAppWaitingDebug();
}

int32_t AmsMgrScheduler::GetWaitingDebugApp(std::vector<std::string> &debugInfoList)
{
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "not ready");
        return ERR_INVALID_OPERATION;
    }
    return amsMgrServiceInner_->GetWaitingDebugApp(debugInfoList);
}

bool AmsMgrScheduler::IsWaitingDebugApp(const std::string &bundleName)
{
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "not ready");
        return false;
    }
    return amsMgrServiceInner_->IsWaitingDebugApp(bundleName);
}

void AmsMgrScheduler::ClearNonPersistWaitingDebugFlag()
{
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "not ready");
        return;
    }
    amsMgrServiceInner_->ClearNonPersistWaitingDebugFlag();
}

int32_t AmsMgrScheduler::RegisterAbilityDebugResponse(const sptr<IAbilityDebugResponse> &response)
{
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "not ready");
        return ERR_INVALID_OPERATION;
    }
    return amsMgrServiceInner_->RegisterAbilityDebugResponse(response);
}

bool AmsMgrScheduler::IsAttachDebug(const std::string &bundleName)
{
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "not ready");
        return false;
    }
    return amsMgrServiceInner_->IsAttachDebug(bundleName);
}

void AmsMgrScheduler::SetKeepAliveEnableState(const std::string &bundleName, bool enable, int32_t uid)
{
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "not ready");
        return;
    }
    amsMgrServiceInner_->SetKeepAliveEnableState(bundleName, enable, uid);
}

void AmsMgrScheduler::SetKeepAliveDkv(const std::string &bundleName, bool enable, int32_t uid)
{
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "not ready");
        return;
    }
    amsMgrServiceInner_->SetKeepAliveDkv(bundleName, enable, uid);
}

void AmsMgrScheduler::SetKeepAliveAppService(const std::string &bundleName, bool enable, int32_t uid)
{
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "not ready");
        return;
    }
    amsMgrServiceInner_->SetKeepAliveAppService(bundleName, enable, uid);
}

void AmsMgrScheduler::ClearProcessByToken(sptr<IRemoteObject> token)
{
    if (!IsReady()) {
        return;
    }

    auto callerTokenId = IPCSkeleton::GetCallingTokenID();
    Security::AccessToken::NativeTokenInfo nativeInfo;
    Security::AccessToken::AccessTokenKit::GetNativeTokenInfo(callerTokenId, nativeInfo);
    if (nativeInfo.processName != "foundation") {
        TAG_LOGE(AAFwkTag::APPMGR, "not foundation");
        return;
    }

    std::function<void()> clearProcessByTokenFunc = [amsMgrServiceInner = amsMgrServiceInner_, token]() {
        amsMgrServiceInner->ClearProcessByToken(token);
    };
    amsHandler_->SubmitTask(clearProcessByTokenFunc, TASK_CLEAR_PROCESS_BY_ABILITY_TOKEN);
}

bool AmsMgrScheduler::IsMemorySizeSufficient()
{
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "not ready");
        return true;
    }
    if (amsMgrServiceInner_->VerifyRequestPermission() != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "verification failed");
        return true;
    }
    return amsMgrServiceInner_->IsMemorySizeSufficient();
}

bool AmsMgrScheduler::IsNoRequireBigMemory()
{
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "not ready");
        return true;
    }
    if (amsMgrServiceInner_->VerifyRequestPermission() != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "verification failed");
        return true;
    }
    return amsMgrServiceInner_->IsNoRequireBigMemory();
}

void AmsMgrScheduler::AttachedToStatusBar(const sptr<IRemoteObject> &token)
{
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "not ready");
        return;
    }
    auto callerTokenId = IPCSkeleton::GetCallingTokenID();
    Security::AccessToken::NativeTokenInfo nativeInfo;
    Security::AccessToken::AccessTokenKit::GetNativeTokenInfo(callerTokenId, nativeInfo);
    if (nativeInfo.processName != "foundation") {
        TAG_LOGE(AAFwkTag::APPMGR, "not foundation");
        return;
    }
    std::function<void()> attachedToStatusBarFunc =
        std::bind(&AppMgrServiceInner::AttachedToStatusBar, amsMgrServiceInner_, token);
    amsHandler_->SubmitTask(attachedToStatusBarFunc, TASK_ATTACHED_TO_STATUS_BAR);
}

void AmsMgrScheduler::BlockProcessCacheByPids(const std::vector<int32_t> &pids)
{
    if (!IsReady()) {
        return;
    }

    pid_t callingPid = IPCSkeleton::GetCallingPid();
    pid_t pid = getprocpid();
    if (callingPid != pid) {
        TAG_LOGE(AAFwkTag::APPMGR, "not allow other process to call");
        return;
    }

    std::function<void()> blockProcCacheFunc = [amsMgrServiceInner = amsMgrServiceInner_, pids]() mutable {
        amsMgrServiceInner->BlockProcessCacheByPids(pids);
    };
    amsHandler_->SubmitTask(blockProcCacheFunc, TASK_BLOCK_PROCESS_CACHE_BY_PIDS);
}

bool AmsMgrScheduler::IsKilledForUpgradeWeb(const std::string &bundleName)
{
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "not ready");
        return false;
    }
    return amsMgrServiceInner_->IsKilledForUpgradeWeb(bundleName);
}

bool AmsMgrScheduler::CleanAbilityByUserRequest(const sptr<IRemoteObject> &token)
{
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "not ready");
        return false;
    }

    if (IPCSkeleton::GetCallingPid() != getprocpid()) {
        TAG_LOGE(AAFwkTag::APPMGR, "not allow other process to call");
        return false;
    }
    return amsMgrServiceInner_->CleanAbilityByUserRequest(token);
}
bool AmsMgrScheduler::IsProcessContainsOnlyUIAbility(const pid_t pid)
{
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "not ready");
        return false;
    }
    pid_t callingPid = IPCSkeleton::GetCallingPid();
    pid_t procPid = getprocpid();
    if (callingPid != procPid) {
        TAG_LOGE(AAFwkTag::APPMGR, "not allow other process to call");
        return false;
    }
    return amsMgrServiceInner_->IsProcessContainsOnlyUIAbility(pid);
}

bool AmsMgrScheduler::IsProcessAttached(sptr<IRemoteObject> token)
{
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "not ready");
        return false;
    }
    return amsMgrServiceInner_->IsProcessAttached(token);
}

bool AmsMgrScheduler::IsCallerKilling(const std::string& callerKey)
{
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "AmsMgrService is not ready.");
        return false;
    }
    return amsMgrServiceInner_->IsCallerKilling(callerKey);
}

int32_t AmsMgrScheduler::PreloadApplicationByPhase(const std::string &bundleName, int32_t userId, int32_t appIndex,
    AppExecFwk::PreloadPhase preloadPhase)
{
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "appMgrServiceInner is not ready.");
        return AAFwk::ERR_APP_MGR_SERVICE_NOT_READY;
    }
    if (!amsMgrServiceInner_->IsFoundationCall()) {
        TAG_LOGE(AAFwkTag::APPMGR, "Not foundation call.");
        return ERR_PERMISSION_DENIED;
    }
    return amsMgrServiceInner_->PreloadApplicationByPhase(bundleName, userId, appIndex, preloadPhase);
}

int32_t AmsMgrScheduler::NotifyPreloadAbilityStateChanged(sptr<IRemoteObject> token, bool isPreForeground)
{
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "appMgrServiceInner is not ready.");
        return AAFwk::ERR_APP_MGR_SERVICE_NOT_READY;
    }
    if (!amsMgrServiceInner_->IsFoundationCall()) {
        TAG_LOGE(AAFwkTag::APPMGR, "Not foundation call.");
        return ERR_PERMISSION_DENIED;
    }
    return amsMgrServiceInner_->NotifyPreloadAbilityStateChanged(token, isPreForeground);
}

int32_t AmsMgrScheduler::CheckPreloadAppRecordExist(const std::string &bundleName, int32_t userId, int32_t appIndex,
    bool &isExist)
{
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "appMgrServiceInner is not ready.");
        return AAFwk::ERR_APP_MGR_SERVICE_NOT_READY;
    }
    if (!amsMgrServiceInner_->IsFoundationCall()) {
        TAG_LOGE(AAFwkTag::APPMGR, "Not foundation call.");
        return ERR_PERMISSION_DENIED;
    }
    isExist = amsMgrServiceInner_->CheckPreloadAppRecordExist(bundleName, userId, appIndex);
    return ERR_OK;
}

int32_t AmsMgrScheduler::VerifyKillProcessPermission(const std::string &bundleName)
{
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "not ready");
        return AAFwk::ERR_APP_MGR_SERVICE_NOT_READY;
    }
    return static_cast<int32_t>(amsMgrServiceInner_->VerifyKillProcessPermission(bundleName));
}
} // namespace AppExecFwk
}  // namespace OHOS
