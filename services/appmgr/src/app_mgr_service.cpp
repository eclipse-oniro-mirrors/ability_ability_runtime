/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#include "app_mgr_service.h"

#include <chrono>
#include <cstdlib>
#include <nlohmann/json.hpp>
#include <sys/types.h>
#include <thread>

#include "ability_manager_errors.h"
#include "ability_manager_xcollie.h"
#include "app_death_recipient.h"
#include "app_mgr_constants.h"
#include "app_utils.h"
#include "application_state_filter.h"
#include "datetime_ex.h"
#include "fd_guard.h"
#include "freeze_util.h"
#include "global_constant.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "in_process_call_wrapper.h"
#include "ipc_skeleton.h"
#include "perf_profile.h"
#include "permission_constants.h"
#include "permission_verification.h"
#include "system_ability_definition.h"
#include "accesstoken_kit.h"
#include "app_mgr_service_const.h"
#include "app_mgr_service_dump_error_code.h"
#include "cache_process_manager.h"
#include "uri_permission_manager_client.h"

namespace OHOS {
namespace AppExecFwk {
using AAFwk::FdGuard;
constexpr const char* OPTION_KEY_HELP = "-h";
constexpr const char* OPTION_KEY_DUMP_IPC = "--ipc";
constexpr const char* OPTION_KEY_DUMP_FFRT = "--ffrt";
constexpr const char* OPTION_KEY_DUMP_WEB = "--web";
const int32_t HIDUMPER_SERVICE_UID = 1212;
constexpr int32_t RESOURCE_MANAGER_UID = 1096;
constexpr const int INDEX_PID = 1;
constexpr const int INDEX_CMD = 2;
constexpr const int INDEX_WEB_CUSTOM_ARGS = 2;
constexpr const size_t VALID_DUMP_IPC_ARG_SIZE = 3;
constexpr const size_t VALID_DUMP_FFRT_ARG_SIZE = 2;
constexpr const size_t VALID_DUMP_WEB_ARG_SIZE = 2;
constexpr const int MAX_DUMP_FFRT_PID_NUMBER = 3;
constexpr const int BASE_TEN = 10;
constexpr const char SIGN_TERMINAL = '\0';
constexpr int32_t DEFAULT_CONCURRENT_NUMBER = 1;
namespace {
#define CHECK_CALLER_IS_SYSTEM_APP                                                             \
    if (!AAFwk::PermissionVerification::GetInstance()->JudgeCallerIsAllowedToUseSystemAPI()) { \
        TAG_LOGE(AAFwkTag::APPMGR, "can't use SA");    \
        return AAFwk::ERR_NOT_SYSTEM_APP;                                                      \
    }
using namespace std::chrono_literals;
constexpr const char* TASK_INIT_APPMGRSERVICEINNER = "InitAppMgrServiceInnerTask";
constexpr const char* TASK_ATTACH_APPLICATION = "AttachApplicationTask";
constexpr const char* TASK_APPLICATION_FOREGROUNDED = "ApplicationForegroundedTask";
constexpr const char* TASK_APPLICATION_BACKGROUNDED = "ApplicationBackgroundedTask";
constexpr const char* TASK_APPLICATION_TERMINATED = "ApplicationTerminatedTask";
constexpr const char* TASK_ABILITY_CLEANED = "AbilityCleanedTask";
constexpr const char* TASK_STARTUP_RESIDENT_PROCESS = "StartupResidentProcess";
constexpr const char* TASK_ADD_ABILITY_STAGE_DONE = "AddAbilityStageDone";
constexpr const char* TASK_START_USER_TEST_PROCESS = "StartUserTestProcess";
constexpr const char* TASK_FINISH_USER_TEST = "FinishUserTest";
constexpr const char* TASK_ATTACH_RENDER_PROCESS = "AttachRenderTask";
#ifdef SUPPORT_CHILD_PROCESS
constexpr const char* TASK_ATTACH_CHILD_PROCESS = "AttachChildProcessTask";
constexpr const char* TASK_EXIT_CHILD_PROCESS_SAFELY = "ExitChildProcessSafelyTask";
#endif // SUPPORT_CHILD_PROCESS
constexpr const char* FOUNDATION_PROCESS = "foundation";
constexpr const char* BS_PROCESS_NAME = "resource_schedule_service";
constexpr int32_t USER_UID = 2000;
constexpr const char* HIVIEW_PROCESS_NAME = "hiview";
constexpr const char* DEBUG_FROM = "ohos.param.debugFrom";
}  // namespace

REGISTER_SYSTEM_ABILITY_BY_ID(AppMgrService, APP_MGR_SERVICE_ID, true);

AppMgrService::AppMgrService()
{
    appMgrServiceInner_ = std::make_shared<AppMgrServiceInner>();
    TAG_LOGI(AAFwkTag::APPMGR, "instance created");
    PerfProfile::GetInstance().SetAmsLoadStartTime(GetTickCount());
}

AppMgrService::AppMgrService(const int32_t serviceId, bool runOnCreate) : SystemAbility(serviceId, runOnCreate)
{
    appMgrServiceInner_ = std::make_shared<AppMgrServiceInner>();
    TAG_LOGI(AAFwkTag::APPMGR, "instance created");
    PerfProfile::GetInstance().SetAmsLoadStartTime(GetTickCount());
}

AppMgrService::~AppMgrService()
{
    TAG_LOGI(AAFwkTag::APPMGR, "instance destroyed");
}

void AppMgrService::OnStart()
{
    TAG_LOGI(AAFwkTag::APPMGR, "start service ready");
    if (appMgrServiceState_.serviceRunningState == ServiceRunningState::STATE_RUNNING) {
        TAG_LOGW(AAFwkTag::APPMGR, "start service fail");
        return;
    }

    ErrCode errCode = Init();
    if (FAILED(errCode)) {
        TAG_LOGE(AAFwkTag::APPMGR, "fail, errCode: %{public}08x", errCode);
        return;
    }
    appMgrServiceState_.serviceRunningState = ServiceRunningState::STATE_RUNNING;
    if (!AddSystemAbilityListener(WINDOW_MANAGER_SERVICE_ID)) {
        TAG_LOGE(AAFwkTag::APPMGR, "OnStart, add listener err");
    }
    if (!AddSystemAbilityListener(URI_PERMISSION_MGR_SERVICE_ID)) {
        TAG_LOGE(AAFwkTag::APPMGR, "failed to add upms listener");
    }
    TAG_LOGI(AAFwkTag::APPMGR, "start service success");
    PerfProfile::GetInstance().SetAmsLoadEndTime(GetTickCount());
    PerfProfile::GetInstance().Dump();
}

void AppMgrService::OnStop()
{
    TAG_LOGI(AAFwkTag::APPMGR, "stop service ready");
    appMgrServiceState_.serviceRunningState = ServiceRunningState::STATE_NOT_START;
    eventHandler_.reset();
    taskHandler_.reset();
    if (appMgrServiceInner_) {
        appMgrServiceInner_->OnStop();
    }
    TAG_LOGI(AAFwkTag::APPMGR, "stop service success");
}

void AppMgrService::SetInnerService(const std::shared_ptr<AppMgrServiceInner> &innerService)
{
    appMgrServiceInner_ = innerService;
}

AppMgrServiceState AppMgrService::QueryServiceState()
{
    if (appMgrServiceInner_) {
        appMgrServiceState_.connectionState = appMgrServiceInner_->QueryAppSpawnConnectionState();
    }
    return appMgrServiceState_;
}

ErrCode AppMgrService::Init()
{
    TAG_LOGI(AAFwkTag::APPMGR, "init ready");
    if (!appMgrServiceInner_) {
        TAG_LOGE(AAFwkTag::APPMGR, "init failed");
        return ERR_INVALID_OPERATION;
    }

    taskHandler_ = AAFwk::TaskHandlerWrap::CreateConcurrentQueueHandler("app_mgr_task_queue",
        DEFAULT_CONCURRENT_NUMBER);
    taskHandler_->SetPrintTaskLog(true);
    eventHandler_ = std::make_shared<AMSEventHandler>(taskHandler_, appMgrServiceInner_);
    appMgrServiceInner_->SetTaskHandler(taskHandler_);
    appMgrServiceInner_->SetEventHandler(eventHandler_);
    DelayedSingleton<CacheProcessManager>::GetInstance()->SetAppMgr(appMgrServiceInner_);
    std::function<void()> initAppMgrServiceInnerTask = [appMgrServiceInner = appMgrServiceInner_]() {
        appMgrServiceInner->Init();
    };
    taskHandler_->SubmitTask(initAppMgrServiceInnerTask, TASK_INIT_APPMGRSERVICEINNER);

    ErrCode openErr = appMgrServiceInner_->OpenAppSpawnConnection();
    if (FAILED(openErr)) {
        TAG_LOGW(AAFwkTag::APPMGR, "fail, errCode: %{public}08x", openErr);
    }
    if (!Publish(this)) {
        TAG_LOGE(AAFwkTag::APPMGR, " publish fail");
        return ERR_APPEXECFWK_SERVICE_NOT_CONNECTED;
    }
    amsMgrScheduler_ = new (std::nothrow) AmsMgrScheduler(appMgrServiceInner_, taskHandler_);
    if (!amsMgrScheduler_) {
        TAG_LOGE(AAFwkTag::APPMGR, "init failed");
        return ERR_INVALID_OPERATION;
    }
    TAG_LOGI(AAFwkTag::APPMGR, "init success");
    return ERR_OK;
}

void AppMgrService::AttachApplication(const sptr<IRemoteObject> &app)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "AttachApplication failed");
        return;
    }

    AbilityRuntime::FreezeUtil::GetInstance().AddAppLifecycleEvent(IPCSkeleton::GetCallingPid(),
        "AppMgrService::AttachApplication");
    pid_t pid = IPCSkeleton::GetCallingPid();
    auto appScheduler = iface_cast<IAppScheduler>(app);
    if (appScheduler == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "appScheduler null");
    }
    std::function<void()> attachApplicationFunc = [appMgrServiceInner = appMgrServiceInner_, pid, appScheduler]() {
        appMgrServiceInner->AttachApplication(pid, appScheduler);
    };
    taskHandler_->SubmitTask(attachApplicationFunc, AAFwk::TaskAttribute{
        .taskName_ = TASK_ATTACH_APPLICATION,
        .taskQos_ = AAFwk::TaskQoS::USER_INTERACTIVE
    });
}

int32_t AppMgrService::PreloadApplication(const std::string &bundleName, int32_t userId,
    AppExecFwk::PreloadMode preloadMode, int32_t appIndex)
{
    XCOLLIE_TIMER_LESS(__PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::APPMGR, "PreloadApplication called");
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "PreloadApplication failed");
        return ERR_INVALID_OPERATION;
    }
    return appMgrServiceInner_->PreloadApplication(bundleName, userId, preloadMode, appIndex);
}

int32_t AppMgrService::PreloadExtension(const AAFwk::Want &want, int32_t appIndex, int32_t userId)
{
    TAG_LOGD(AAFwkTag::APPMGR, "PreloadExtension called");
    if (appIndex != 0) {
        TAG_LOGE(AAFwkTag::APPMGR, "PreloadExtension failed, appIndex != 0.");
        return ERR_INVALID_OPERATION;
    }
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "PreloadExtension failed, appMgr not ready.");
        return ERR_INVALID_OPERATION;
    }
    return appMgrServiceInner_->PreloadExtension(want, appIndex, userId);
}

void AppMgrService::PreloadModuleFinished(const int32_t recordId)
{
    TAG_LOGD(AAFwkTag::APPMGR, "PreloadModuleFinished called");
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "PreloadModuleFinished failed");
        return;
    }
    if (!JudgeAppSelfCalled(recordId)) {
        return;
    }
    return appMgrServiceInner_->PreloadModuleFinished(IPCSkeleton::GetCallingPid());
}

int32_t AppMgrService::MakeImage(const AAFwk::Want &want, int32_t userId,
    AppExecFwk::PreloadMode preloadMode, int32_t appIndex, sptr<IImageErrorHandler> errorHandler)
{
    TAG_LOGD(AAFwkTag::APPMGR, "MakeImage called");
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "MakeImage failed");
        return ERR_INVALID_OPERATION;
    }
    CHECK_CALLER_IS_SYSTEM_APP;
    if (!AAFwk::PermissionVerification::GetInstance()->VerifyPreloadApplicationPermission()) {
        TAG_LOGE(AAFwkTag::APPMGR, "permission verify fail");
        return ERR_PERMISSION_DENIED;
    }
    auto task = [appMgrServiceInner = appMgrServiceInner_, want, userId, preloadMode,
        appIndex, errorHandler] () {
        appMgrServiceInner->MakeImage(want, userId, preloadMode, appIndex, errorHandler);
    };
    taskHandler_->SubmitTask(task, AAFwk::TaskAttribute{
        .taskName_ = "MakeImageTask",
        .taskQos_ = AAFwk::TaskQoS::USER_INTERACTIVE
    });
    return ERR_OK;
}

int32_t AppMgrService::DestroyImage(uint64_t checkpointId, sptr<IImageErrorHandler> errorHandler)
{
    TAG_LOGD(AAFwkTag::APPMGR, "DestroyImage called");
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "DestroyImage failed");
        return ERR_INVALID_OPERATION;
    }
    CHECK_CALLER_IS_SYSTEM_APP;
    if (!AAFwk::PermissionVerification::GetInstance()->VerifyPreloadApplicationPermission()) {
        TAG_LOGE(AAFwkTag::APPMGR, "permission verify fail");
        return ERR_PERMISSION_DENIED;
    }
    auto task = [appMgrServiceInner = appMgrServiceInner_, checkpointId, errorHandler] () {
        appMgrServiceInner->DestroyImage(checkpointId, errorHandler);
    };
    taskHandler_->SubmitTask(task, AAFwk::TaskAttribute{
        .taskName_ = "DestroyImageTask",
        .taskQos_ = AAFwk::TaskQoS::USER_INTERACTIVE
    });
    return ERR_OK;
}

int32_t AppMgrService::NotifyTemplateProcessDeepFrozen(int32_t pid)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "not ready");
        return ERR_INVALID_OPERATION;
    }
    if (IPCSkeleton::GetCallingUid() != RESOURCE_MANAGER_UID) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "not rss");
        return AAFwk::CHECK_PERMISSION_FAILED;
    }
    if (!taskHandler_) {
        TAG_LOGE(AAFwkTag::APPMGR, "null taskHandler_");
        return ERR_INVALID_VALUE;
    }
    auto task = [appMgrServiceInner = appMgrServiceInner_, pid] () {
        appMgrServiceInner->HandleForkAll(pid);
    };
    taskHandler_->SubmitTask(task, AAFwk::TaskAttribute{
        .taskName_ = "NotifyTemplateProcessDeepFrozenTask",
        .taskQos_ = AAFwk::TaskQoS::USER_INTERACTIVE
    });
    return ERR_OK;
}

void AppMgrService::ApplicationForegrounded(const int32_t recordId)
{
    if (!IsReady()) {
        return;
    }
    if (!JudgeAppSelfCalled(recordId)) {
        return;
    }
    AbilityRuntime::FreezeUtil::GetInstance().AddAppLifecycleEvent(IPCSkeleton::GetCallingPid(),
        "AppMgrService::AppForegrounded");
    std::function<void()> applicationForegroundedFunc = [appMgrServiceInner = appMgrServiceInner_, recordId]() {
        appMgrServiceInner->ApplicationForegrounded(recordId);
    };
    taskHandler_->SubmitTask(applicationForegroundedFunc, AAFwk::TaskAttribute{
        .taskName_ = TASK_APPLICATION_FOREGROUNDED,
        .taskQos_ = AAFwk::TaskQoS::USER_INTERACTIVE
    });
}

void AppMgrService::ApplicationBackgrounded(const int32_t recordId)
{
    if (!IsReady()) {
        return;
    }
    if (!JudgeAppSelfCalled(recordId)) {
        return;
    }
    AbilityRuntime::FreezeUtil::GetInstance().AddAppLifecycleEvent(IPCSkeleton::GetCallingPid(),
        "AppMgrService::AppBackgrounded");
    taskHandler_->CancelTask("appbackground_" + std::to_string(recordId));
    std::function<void()> applicationBackgroundedFunc = [appMgrServiceInner = appMgrServiceInner_, recordId]() {
        appMgrServiceInner->ApplicationBackgrounded(recordId);
    };
    taskHandler_->SubmitTask(applicationBackgroundedFunc, AAFwk::TaskAttribute{
        .taskName_ = TASK_APPLICATION_BACKGROUNDED,
        .taskQos_ = AAFwk::TaskQoS::USER_INTERACTIVE
    });
}

void AppMgrService::ApplicationTerminated(const int32_t recordId)
{
    if (!IsReady()) {
        return;
    }
    if (!JudgeAppSelfCalled(recordId)) {
        return;
    }
    std::function<void()> applicationTerminatedFunc = [appMgrServiceInner = appMgrServiceInner_, recordId]() {
        appMgrServiceInner->ApplicationTerminated(recordId);
    };
    taskHandler_->SubmitTask(applicationTerminatedFunc, AAFwk::TaskAttribute{
        .taskName_ = TASK_APPLICATION_TERMINATED,
        .taskQos_ = AAFwk::TaskQoS::USER_INTERACTIVE
    });
}

void AppMgrService::AbilityCleaned(const sptr<IRemoteObject> &token)
{
    if (!IsReady()) {
        return;
    }

    auto callerUid = IPCSkeleton::GetCallingUid();
    auto appRecord = appMgrServiceInner_->GetTerminatingAppRunningRecord(token);
    if (!appRecord || appRecord->GetUid() != callerUid) {
        TAG_LOGW(AAFwkTag::APPMGR, "verification failed");
        return;
    }

    std::function<void()> abilityCleanedFunc = [appMgrServiceInner = appMgrServiceInner_, token]() {
        appMgrServiceInner->AbilityTerminated(token);
    };
    taskHandler_->SubmitTask(abilityCleanedFunc, AAFwk::TaskAttribute{
        .taskName_ = TASK_ABILITY_CLEANED,
        .taskQos_ = AAFwk::TaskQoS::USER_INTERACTIVE
    });
}

bool AppMgrService::IsReady() const
{
    if (appMgrServiceInner_ && taskHandler_ && eventHandler_) {
        return true;
    }

    TAG_LOGW(AAFwkTag::APPMGR, "not ready");
    return false;
}

void AppMgrService::StartupResidentProcess(const std::vector<AppExecFwk::BundleInfo> &bundleInfos)
{
    if (!IsReady()) {
        return;
    }
    pid_t callingPid = IPCSkeleton::GetCallingPid();
    pid_t pid = getprocpid();
    if (callingPid != pid) {
        TAG_LOGE(AAFwkTag::APPMGR, "not process call");
        return;
    }
    TAG_LOGI(AAFwkTag::APPMGR, "start process");
    std::function <void()> startupResidentProcess = [appMgrServiceInner = appMgrServiceInner_, bundleInfos]() {
        appMgrServiceInner->LoadResidentProcess(bundleInfos);
    };
    taskHandler_->SubmitTask(startupResidentProcess, AAFwk::TaskAttribute{
        .taskName_ = TASK_STARTUP_RESIDENT_PROCESS,
        .taskQos_ = AAFwk::TaskQoS::USER_INTERACTIVE
    });
}

sptr<IAmsMgr> AppMgrService::GetAmsMgr()
{
    return amsMgrScheduler_;
}

int32_t AppMgrService::ClearUpApplicationData(const std::string &bundleName, int32_t appCloneIndex, int32_t userId)
{
    if (!AAFwk::PermissionVerification::GetInstance()->JudgeCallerIsAllowedToUseSystemAPI()) {
        TAG_LOGE(AAFwkTag::APPMGR, "caller is not SA");
        return AAFwk::ERR_NOT_SYSTEM_APP;
    }
    if (!IsReady()) {
        return ERR_INVALID_OPERATION;
    }
    std::shared_ptr<RemoteClientManager> remoteClientManager = std::make_shared<RemoteClientManager>();
    if (remoteClientManager == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null remoteClientManager");
        return ERR_INVALID_OPERATION;
    }
    auto bundleMgrHelper = remoteClientManager->GetBundleManagerHelper();
    if (bundleMgrHelper == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null bundleMgrHelper");
        return ERR_INVALID_OPERATION;
    }
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    if ((callingUid != 0 && callingUid != USER_UID) || userId < 0) {
        auto isCallingPerm = AAFwk::PermissionVerification::GetInstance()->VerifyCallingPermission(
            AAFwk::PermissionConstants::PERMISSION_CLEAN_APPLICATION_DATA);
        if (!isCallingPerm) {
            TAG_LOGE(AAFwkTag::APPMGR, "verification failed");
            return AAFwk::CHECK_PERMISSION_FAILED;
        }
    }
    if (appCloneIndex < 0 || appCloneIndex > AbilityRuntime::GlobalConstant::MAX_APP_CLONE_INDEX) {
        TAG_LOGE(AAFwkTag::APPMGR, "appCloneIndex invalid");
        return AAFwk::ERR_APP_CLONE_INDEX_INVALID;
    }
    pid_t pid = IPCSkeleton::GetCallingPid();
    return appMgrServiceInner_->ClearUpApplicationData(bundleName, callingUid, pid, appCloneIndex, userId);
}

int32_t AppMgrService::ClearUpApplicationDataBySelf(int32_t userId)
{
    if (!IsReady()) {
        return ERR_INVALID_OPERATION;
    }
    int32_t uid = IPCSkeleton::GetCallingUid();
    pid_t pid = IPCSkeleton::GetCallingPid();
    return appMgrServiceInner_->ClearUpApplicationDataBySelf(uid, pid, userId);
}

int32_t AppMgrService::GetAllRunningProcesses(std::vector<RunningProcessInfo> &info)
{
    if (!IsReady()) {
        return ERR_INVALID_OPERATION;
    }
    return appMgrServiceInner_->GetAllRunningProcesses(info);
}

int32_t AppMgrService::GetRunningMultiAppInfoByBundleName(const std::string &bundleName,
    RunningMultiAppInfo &info)
{
    if (AAFwk::AppUtils::GetInstance().IsForbidStart()) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "forbid start: GetRunningMultiAppInfoByBundleName");
        return AAFwk::INNER_ERR;
    }
    if (!IsReady()) {
        return ERR_INVALID_OPERATION;
    }

    if (!AAFwk::PermissionVerification::GetInstance()->JudgeCallerIsAllowedToUseSystemAPI()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "caller is not SA");
        return ERR_INVALID_OPERATION;
    }

    bool isCallingPermission = AAFwk::PermissionVerification::GetInstance()->VerifyRunningInfoPerm();
    if (!isCallingPermission) {
        TAG_LOGE(AAFwkTag::APPMGR, "verification failed");
        return ERR_PERMISSION_DENIED;
    }
    return appMgrServiceInner_->GetRunningMultiAppInfoByBundleName(bundleName, info);
}

int32_t AppMgrService::GetAllRunningInstanceKeysBySelf(std::vector<std::string> &instanceKeys)
{
    if (!IsReady()) {
        return ERR_INVALID_OPERATION;
    }
    return appMgrServiceInner_->GetAllRunningInstanceKeysBySelf(instanceKeys);
}

int32_t AppMgrService::GetAllRunningInstanceKeysByBundleName(const std::string &bundleName,
    std::vector<std::string> &instanceKeys, int32_t userId)
{
    if (!IsReady()) {
        return ERR_INVALID_OPERATION;
    }
    return appMgrServiceInner_->GetAllRunningInstanceKeysByBundleName(bundleName, instanceKeys, userId);
}

int32_t AppMgrService::GetRunningProcessesByBundleType(BundleType bundleType,
    std::vector<RunningProcessInfo> &info)
{
    if (!IsReady()) {
        return ERR_INVALID_OPERATION;
    }
    return appMgrServiceInner_->GetRunningProcessesByBundleType(bundleType, info);
}

int32_t AppMgrService::GetAllRenderProcesses(std::vector<RenderProcessInfo> &info)
{
    if (!IsReady()) {
        return ERR_INVALID_OPERATION;
    }
    return appMgrServiceInner_->GetAllRenderProcesses(info);
}

int AppMgrService::GetAllChildrenProcesses(std::vector<ChildProcessInfo> &info)
{
#ifdef SUPPORT_CHILD_PROCESS
    if (!IsReady()) {
        return ERR_INVALID_OPERATION;
    }
    return appMgrServiceInner_->GetAllChildrenProcesses(info);
#endif // SUPPORT_CHILD_PROCESS
    return ERR_INVALID_OPERATION;
}

int32_t AppMgrService::IsTerminatingByPid(pid_t pid, bool &isTerminating)
{
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "not ready");
        return ERR_INVALID_OPERATION;
    }
    bool isCallingPermission =
        AAFwk::PermissionVerification::GetInstance()->CheckSpecificSystemAbilityAccessPermission(FOUNDATION_PROCESS);
    if (!isCallingPermission) {
        TAG_LOGE(AAFwkTag::APPMGR, "verification failed");
        return ERR_PERMISSION_DENIED;
    }
    auto appRunningRecord = appMgrServiceInner_->GetAppRunningRecordByPid(pid);
    if (appRunningRecord && appRunningRecord->IsTerminating()) {
        isTerminating = true;
        TAG_LOGD(AAFwkTag::APPMGR, "current pid %{public}d is terminating.", pid);
        return ERR_OK;
    }
    TAG_LOGD(AAFwkTag::APPMGR, "current pid %{public}d is not terminating.", pid);
    return ERR_OK;
}

int32_t AppMgrService::GetProcessRunningInfosByUserId(std::vector<RunningProcessInfo> &info, int32_t userId)
{
    if (!IsReady()) {
        return ERR_INVALID_OPERATION;
    }
    return appMgrServiceInner_->GetProcessRunningInfosByUserId(info, userId);
}

int32_t AppMgrService::GetProcessRunningInformation(RunningProcessInfo &info)
{
    if (!IsReady()) {
        return ERR_INVALID_OPERATION;
    }
    return appMgrServiceInner_->GetProcessRunningInformation(info);
}

int32_t AppMgrService::NotifyMemoryLevel(int32_t level)
{
    if (!IsReady()) {
        return ERR_INVALID_OPERATION;
    }
    return appMgrServiceInner_->NotifyMemoryLevel(level);
}

int32_t AppMgrService::NotifyProcMemoryLevel(const std::map<pid_t, MemoryLevel> &procLevelMap)
{
    if (!IsReady()) {
        return ERR_INVALID_OPERATION;
    }
    return appMgrServiceInner_->NotifyProcMemoryLevel(procLevelMap);
}

int32_t AppMgrService::DumpHeapMemory(const int32_t pid, OHOS::AppExecFwk::MallocInfo &mallocInfo)
{
    if (!IsReady()) {
        return ERR_INVALID_OPERATION;
    }
    return appMgrServiceInner_->DumpHeapMemory(pid, mallocInfo);
}

// Authenticate dump permissions
bool AppMgrService::HasDumpPermission() const
{
    uint32_t callingTokenID = IPCSkeleton::GetCallingTokenID();
    int res = Security::AccessToken::AccessTokenKit::VerifyAccessToken(callingTokenID, "ohos.permission.DUMP");
    if (res != Security::AccessToken::PermissionState::PERMISSION_GRANTED) {
        TAG_LOGE(AAFwkTag::APPMGR, "no permission");
        return false;
    }
    return true;
}

int32_t AppMgrService::DumpJsHeapMemory(OHOS::AppExecFwk::JsHeapDumpInfo &info)
{
    if (!IsReady() || !HasDumpPermission()) {
        return ERR_INVALID_OPERATION;
    }
    return appMgrServiceInner_->DumpJsHeapMemory(info);
}

int32_t AppMgrService::DumpCjHeapMemory(OHOS::AppExecFwk::CjHeapDumpInfo &info)
{
    if (!IsReady() || !HasDumpPermission()) {
        return ERR_INVALID_OPERATION;
    }
    static std::mutex mutex_;
    std::lock_guard<std::mutex> lock(mutex_);
    return appMgrServiceInner_->DumpCjHeapMemory(info);
}

int32_t AppMgrService::DumpMem(OHOS::AppExecFwk::MemDumpInfo &info, std::string &dumpResult)
{
    if (!IsReady() || !HasDumpPermission()) {
        return ERR_INVALID_OPERATION;
    }
    return appMgrServiceInner_->DumpMem(info, dumpResult);
}

void AppMgrService::AddAbilityStageDone(const int32_t recordId)
{
    if (!IsReady()) {
        return;
    }
    if (!JudgeAppSelfCalled(recordId)) {
        return;
    }
    std::function <void()> addAbilityStageDone = [appMgrServiceInner = appMgrServiceInner_, recordId]() {
        appMgrServiceInner->AddAbilityStageDone(recordId);
    };
    taskHandler_->SubmitTask(addAbilityStageDone, AAFwk::TaskAttribute{
        .taskName_ = TASK_ADD_ABILITY_STAGE_DONE,
        .taskQos_ = AAFwk::TaskQoS::USER_INTERACTIVE
    });
}

int32_t AppMgrService::RegisterApplicationStateObserver(const sptr<IApplicationStateObserver> &observer,
    const std::vector<std::string> &bundleNameList)
{
    TAG_LOGD(AAFwkTag::APPMGR, "begin");
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "not ready");
        return ERR_INVALID_OPERATION;
    }
    return appMgrServiceInner_->RegisterApplicationStateObserver(observer, bundleNameList);
}

int32_t AppMgrService::UnregisterApplicationStateObserver(const sptr<IApplicationStateObserver> &observer)
{
    TAG_LOGD(AAFwkTag::APPMGR, "begin");
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "not ready");
        return ERR_INVALID_OPERATION;
    }
    return appMgrServiceInner_->UnregisterApplicationStateObserver(observer);
}

int32_t AppMgrService::RegisterImageProcessStateObserver(const sptr<IImageProcessStateObserver> &observer)
{
    TAG_LOGD(AAFwkTag::APPMGR, "begin");
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "not ready");
        return ERR_INVALID_OPERATION;
    }
    return appMgrServiceInner_->RegisterImageProcessStateObserver(observer);
}

int32_t AppMgrService::UnregisterImageProcessStateObserver(const sptr<IImageProcessStateObserver> &observer)
{
    TAG_LOGD(AAFwkTag::APPMGR, "begin");
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "not ready");
        return ERR_INVALID_OPERATION;
    }
    return appMgrServiceInner_->UnregisterImageProcessStateObserver(observer);
}

int32_t AppMgrService::RegisterNativeChildExitNotify(const sptr<INativeChildNotify> notify)
{
    TAG_LOGD(AAFwkTag::APPMGR, "begin");
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "not ready");
        return ERR_INVALID_OPERATION;
    }
    return appMgrServiceInner_->RegisterNativeChildExitNotify(notify);
}

int32_t AppMgrService::UnregisterNativeChildExitNotify(const sptr<INativeChildNotify> notify)
{
    TAG_LOGD(AAFwkTag::APPMGR, "begin");
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "not ready");
        return ERR_INVALID_OPERATION;
    }
    return appMgrServiceInner_->UnregisterNativeChildExitNotify(notify);
}

int32_t AppMgrService::RegisterAbilityForegroundStateObserver(const sptr<IAbilityForegroundStateObserver> &observer)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "not ready");
        return ERR_INVALID_OPERATION;
    }
    return appMgrServiceInner_->RegisterAbilityForegroundStateObserver(observer);
}

int32_t AppMgrService::UnregisterAbilityForegroundStateObserver(const sptr<IAbilityForegroundStateObserver> &observer)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "not ready");
        return ERR_INVALID_OPERATION;
    }
    return appMgrServiceInner_->UnregisterAbilityForegroundStateObserver(observer);
}

int32_t AppMgrService::GetForegroundApplications(std::vector<AppStateData> &list)
{
    TAG_LOGD(AAFwkTag::APPMGR, "begin");
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "not ready");
        return ERR_INVALID_OPERATION;
    }
    return appMgrServiceInner_->GetForegroundApplications(list);
}

int AppMgrService::StartUserTestProcess(const AAFwk::Want &want, const sptr<IRemoteObject> &observer,
    const AppExecFwk::BundleInfo &bundleInfo, int32_t userId)
{
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "not ready");
        return ERR_INVALID_OPERATION;
    }
    if (!AAFwk::PermissionVerification::GetInstance()->IsShellCall()) {
        TAG_LOGE(AAFwkTag::APPMGR, "StartUserTestProcess is not shell call");
        return ERR_INVALID_OPERATION;
    }
    std::function<void()> startUserTestProcessFunc = [appMgrServiceInner = appMgrServiceInner_,
        want, observer, bundleInfo, userId]() {
        appMgrServiceInner->StartUserTestProcess(want, observer, bundleInfo, userId);
    };
    taskHandler_->SubmitTask(startUserTestProcessFunc, TASK_START_USER_TEST_PROCESS);
    return ERR_OK;
}

int AppMgrService::FinishUserTest(const std::string &msg, const int64_t &resultCode, const std::string &bundleName)
{
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "not ready");
        return ERR_INVALID_OPERATION;
    }
    std::shared_ptr<RemoteClientManager> remoteClientManager = std::make_shared<RemoteClientManager>();
    if (remoteClientManager == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null remoteClientManager");
        return ERR_INVALID_OPERATION;
    }
    auto bundleMgrHelper = remoteClientManager->GetBundleManagerHelper();
    if (bundleMgrHelper == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null bundleMgrHelper");
        return ERR_INVALID_OPERATION;
    }
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    std::string callerBundleName;
    auto result = IN_PROCESS_CALL(bundleMgrHelper->GetNameForUid(callingUid, callerBundleName));
    if (result == ERR_OK) {
        TAG_LOGI(AAFwkTag::APPMGR, "callingPid_ %{public}s", callerBundleName.c_str());
        if (bundleName != callerBundleName) {
            TAG_LOGE(AAFwkTag::APPMGR, "not process call");
            return ERR_INVALID_OPERATION;
        }
    } else {
        TAG_LOGE(AAFwkTag::APPMGR, "fail: %{public}d", result);
        return ERR_INVALID_OPERATION;
    }
    pid_t callingPid = IPCSkeleton::GetCallingPid();
    std::function<void()> finishUserTestProcessFunc = [appMgrServiceInner = appMgrServiceInner_, msg,
        resultCode, bundleName, callingPid]() {
        appMgrServiceInner->FinishUserTest(msg, resultCode, bundleName, callingPid);
    };
    taskHandler_->SubmitTask(finishUserTestProcessFunc, TASK_FINISH_USER_TEST);
    return ERR_OK;
}

int AppMgrService::Dump(int fd, const std::vector<std::u16string>& args)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "not ready");
        return ERR_APPEXECFWK_HIDUMP_ERROR;
    }

    std::string result;
    auto errCode = Dump(args, result);
    int ret = dprintf(fd, "%s\n", result.c_str());
    if (ret < 0) {
        TAG_LOGE(AAFwkTag::APPMGR, "dprintf error");
        return ERR_APPEXECFWK_HIDUMP_ERROR;
    }
    return errCode;
}

int AppMgrService::Dump(const std::vector<std::u16string>& args, std::string& result)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    auto size = args.size();
    if (size == 0) {
        return ShowHelp(args, result);
    }

    std::string optionKey = Str16ToStr8(args[0]);
    if (optionKey == OPTION_KEY_HELP) {
        return ShowHelp(args, result);
    }
    if (optionKey == OPTION_KEY_DUMP_IPC) {
        return DumpIpc(args, result);
    }
    if (optionKey == OPTION_KEY_DUMP_FFRT) {
        return DumpFfrt(args, result);
    }
    if (optionKey == OPTION_KEY_DUMP_WEB) {
        return DumpArkWeb(args, result);
    }
    result.append("error: unkown option.\n");
    TAG_LOGE(AAFwkTag::APPMGR, "option key %{public}s does not exist", optionKey.c_str());
    return DumpErrorCode::ERR_UNKNOWN_OPTION_ERROR;
}

int AppMgrService::ShowHelp(const std::vector<std::u16string>& args, std::string& result)
{
    result.append("Usage:\n")
        .append("-h                          ")
        .append("help text for the tool\n")
        .append("--ffrt pid1[,pid2,pid3]     ")
        .append("dump ffrt info\n")
        .append("--ipc pid ARG               ")
        .append("ipc load statistic; pid must be specified or set to -a dump all processes. ARG must be one of "
            "--start-stat | --stop-stat | --stat\n")
        .append("--web pid1[,pid2,pid3] [ARG]    ")
        .append("dump arkweb info, ARG must be one of (--all | --nweb | ...)\n");

    return ERR_OK;
}

int AppMgrService::DumpIpcAllInner(const AppMgrService::DumpIpcKey key, std::string& result)
{
    TAG_LOGI(AAFwkTag::APPMGR, "call");
    switch (key) {
        case KEY_DUMP_IPC_START:
            return DumpIpcAllStart(result);
        case KEY_DUMP_IPC_STOP:
            return DumpIpcAllStop(result);
        case KEY_DUMP_IPC_STAT:
            return DumpIpcAllStat(result);
        default: {
            result.append(MSG_DUMP_FAIL, strlen(MSG_DUMP_FAIL))
                .append(MSG_DUMP_FAIL_REASON_INTERNAL, strlen(MSG_DUMP_FAIL_REASON_INTERNAL));
            TAG_LOGE(AAFwkTag::APPMGR, "dump ipc all function does not exist");
            return DumpErrorCode::ERR_INTERNAL_ERROR;
        }
    }
}

int AppMgrService::DumpIpcWithPidInner(const AppMgrService::DumpIpcKey key,
    const std::string& optionPid, std::string& result)
{
    TAG_LOGI(AAFwkTag::APPMGR, "call");
    int32_t pid = -1;
    char* end = nullptr;
    pid = static_cast<int32_t>(std::strtol(optionPid.c_str(), &end, BASE_TEN));
    if (end && *end != SIGN_TERMINAL) {
        result.append(MSG_DUMP_FAIL, strlen(MSG_DUMP_FAIL))
            .append(MSG_DUMP_FAIL_REASON_INVALILD_PID, strlen(MSG_DUMP_FAIL_REASON_INVALILD_PID));
        TAG_LOGE(AAFwkTag::APPMGR, "invalid pid: %{public}s", optionPid.c_str());
        return DumpErrorCode::ERR_INVALID_PID_ERROR;
    }
    if (pid < 0) {
        result.append(MSG_DUMP_FAIL, strlen(MSG_DUMP_FAIL))
            .append(MSG_DUMP_FAIL_REASON_INVALILD_PID, strlen(MSG_DUMP_FAIL_REASON_INVALILD_PID));
        TAG_LOGE(AAFwkTag::APPMGR, "invalid pid: %{public}s", optionPid.c_str());
        return DumpErrorCode::ERR_INVALID_PID_ERROR;
    }

    switch (key) {
        case KEY_DUMP_IPC_START:
            return DumpIpcStart(pid, result);
        case KEY_DUMP_IPC_STOP:
            return DumpIpcStop(pid, result);
        case KEY_DUMP_IPC_STAT:
            return DumpIpcStat(pid, result);
        default: {
            TAG_LOGE(AAFwkTag::APPMGR, "option key %{public}d does not exist", key);
            result.append(MSG_DUMP_FAIL, strlen(MSG_DUMP_FAIL))
                .append(MSG_DUMP_FAIL_REASON_INTERNAL, strlen(MSG_DUMP_FAIL_REASON_INTERNAL));
            TAG_LOGE(AAFwkTag::APPMGR, "dump ipc function does not exist");
            return DumpErrorCode::ERR_INTERNAL_ERROR;
        }
    }
}

int AppMgrService::DumpFfrtInner(const std::string& pidsRaw, std::string& result)
{
    TAG_LOGI(AAFwkTag::APPMGR, "call");
    std::vector<int32_t> pids;
    auto ret = DumpParsePids(pidsRaw, pids);
    if (ret != ERR_OK) {
        return ret;
    }
    return appMgrServiceInner_->DumpFfrt(pids, result);
}

int32_t AppMgrService::DumpParsePids(const std::string &pidsRaw, std::vector<int32_t> &pids)
{
    std::vector<std::string> pidsStr;
    SplitStr(pidsRaw, ",", pidsStr);
    if (pidsStr.empty()) {
        TAG_LOGE(AAFwkTag::APPMGR, "pids are empty");
        return DumpErrorCode::ERR_INVALID_PID_ERROR;
    }
    if (pidsStr.size() > MAX_DUMP_FFRT_PID_NUMBER) {
        pidsStr.resize(MAX_DUMP_FFRT_PID_NUMBER);
    }
    for (const auto& pidStr : pidsStr) {
        int pid = -1;
        char* end = nullptr;
        pid = static_cast<int32_t>(std::strtol(pidStr.c_str(), &end, BASE_TEN));
        if (end && *end != SIGN_TERMINAL) {
            TAG_LOGE(AAFwkTag::APPMGR, "invalid pid:%{public}s", pidStr.c_str());
            continue;
        }
        if (pid < 0) {
            TAG_LOGE(AAFwkTag::APPMGR, "invalid pid: %{public}s", pidStr.c_str());
            continue;
        }
        TAG_LOGD(AAFwkTag::APPMGR, "valid pid:%{public}d", pid);
        pids.push_back(pid);
    }
    if (pids.empty()) {
        TAG_LOGE(AAFwkTag::APPMGR, "pids are empty");
        return DumpErrorCode::ERR_INVALID_PID_ERROR;
    }
    return ERR_OK;
}

bool AppMgrService::GetDumpIpcKeyByOption(const std::string &option, DumpIpcKey &key)
{
    if (option == "--start-stat") {
        key = KEY_DUMP_IPC_START;
        return true;
    }
    if (option == "--stop-stat") {
        key = KEY_DUMP_IPC_STOP;
        return true;
    }
    if (option == "--stat") {
        key = KEY_DUMP_IPC_STAT;
        return true;
    }
    return false;
}

int AppMgrService::DumpIpc(const std::vector<std::u16string>& args, std::string& result)
{
    TAG_LOGD(AAFwkTag::APPMGR, "Called. AppMgrService::DumpIpc start");
    if (args.size() != VALID_DUMP_IPC_ARG_SIZE) {
        result.append(MSG_DUMP_FAIL, strlen(MSG_DUMP_FAIL))
            .append(MSG_DUMP_FAIL_REASON_INVALILD_NUM_ARGS, strlen(MSG_DUMP_FAIL_REASON_INVALILD_NUM_ARGS));
        TAG_LOGE(AAFwkTag::APPMGR, "invalid arguments");
        return DumpErrorCode::ERR_INVALID_NUM_ARGS_ERROR;
    }
    auto isHidumperServiceCall = (IPCSkeleton::GetCallingUid() == HIDUMPER_SERVICE_UID);
    if (!isHidumperServiceCall) {
        result.append(MSG_DUMP_FAIL, strlen(MSG_DUMP_FAIL))
            .append(MSG_DUMP_FAIL_REASON_PERMISSION_DENY, strlen(MSG_DUMP_FAIL_REASON_PERMISSION_DENY));
        TAG_LOGE(AAFwkTag::APPMGR, "permission deny");
        return DumpErrorCode::ERR_PERMISSION_DENY_ERROR;
    }

    std::string optionCmd = Str16ToStr8(args[INDEX_CMD]);
    std::string optionPid = Str16ToStr8(args[INDEX_PID]);
    TAG_LOGD(AAFwkTag::APPMGR, "option pid:%{public}s, option cmd:%{public}s",
        optionPid.c_str(), optionCmd.c_str());

    DumpIpcKey key;
    if (!GetDumpIpcKeyByOption(optionCmd, key)) {
        result.append(MSG_DUMP_FAIL, strlen(MSG_DUMP_FAIL))
            .append(MSG_DUMP_FAIL_REASON_INVALILD_CMD, strlen(MSG_DUMP_FAIL_REASON_INVALILD_CMD));
        TAG_LOGE(AAFwkTag::APPMGR, "option command %{public}s does not exist", optionCmd.c_str());
        return DumpErrorCode::ERR_INVALID_CMD_ERROR;
    }

    if (optionPid == "-a" || optionPid == "all" || optionPid == "--all") {
        return DumpIpcAllInner(key, result);
    }
    return DumpIpcWithPidInner(key, optionPid, result);
}

int AppMgrService::DumpFfrt(const std::vector<std::u16string>& args, std::string& result)
{
    TAG_LOGD(AAFwkTag::APPMGR, "Called. AppMgrService::DumpFfrt start");
    if (args.size() != VALID_DUMP_FFRT_ARG_SIZE) {
        result.append(MSG_DUMP_FAIL, strlen(MSG_DUMP_FAIL))
            .append(MSG_DUMP_FAIL_REASON_INVALILD_NUM_ARGS, strlen(MSG_DUMP_FAIL_REASON_INVALILD_NUM_ARGS));
        TAG_LOGE(AAFwkTag::APPMGR, "invalid arguments");
        return DumpErrorCode::ERR_INVALID_NUM_ARGS_ERROR;
    }
    auto isHidumperServiceCall = (IPCSkeleton::GetCallingUid() == HIDUMPER_SERVICE_UID);
    if (!isHidumperServiceCall) {
        result.append(MSG_DUMP_FAIL, strlen(MSG_DUMP_FAIL))
            .append(MSG_DUMP_FAIL_REASON_PERMISSION_DENY, strlen(MSG_DUMP_FAIL_REASON_PERMISSION_DENY));
        TAG_LOGE(AAFwkTag::APPMGR, "permission deny");
        return DumpErrorCode::ERR_PERMISSION_DENY_ERROR;
    }

    std::string pidsRaw = Str16ToStr8(args[INDEX_PID]);
    TAG_LOGD(AAFwkTag::APPMGR, "pids:%{public}s", pidsRaw.c_str());

    return DumpFfrtInner(pidsRaw, result);
}

int AppMgrService::DumpIpcAllStart(std::string& result)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    return appMgrServiceInner_->DumpIpcAllStart(result);
}

int AppMgrService::DumpIpcAllStop(std::string& result)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    return appMgrServiceInner_->DumpIpcAllStop(result);
}

int AppMgrService::DumpIpcAllStat(std::string& result)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    return appMgrServiceInner_->DumpIpcAllStat(result);
}

int AppMgrService::DumpIpcStart(const int32_t pid, std::string& result)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    return appMgrServiceInner_->DumpIpcStart(pid, result);
}

int AppMgrService::DumpIpcStop(const int32_t pid, std::string& result)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    return appMgrServiceInner_->DumpIpcStop(pid, result);
}

int AppMgrService::DumpIpcStat(const int32_t pid, std::string& result)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    return appMgrServiceInner_->DumpIpcStat(pid, result);
}

int32_t AppMgrService::DumpArkWeb(const std::vector<std::u16string> &args, std::string &result)
{
    TAG_LOGI(AAFwkTag::APPMGR, "DumpArkWeb start");
    if (args.size() < VALID_DUMP_WEB_ARG_SIZE) {
        result.append(MSG_DUMP_FAIL, strlen(MSG_DUMP_FAIL))
            .append(MSG_DUMP_FAIL_REASON_INVALILD_NUM_ARGS, strlen(MSG_DUMP_FAIL_REASON_INVALILD_NUM_ARGS));
        TAG_LOGE(AAFwkTag::APPMGR, "invalid number of arguments.");
        return DumpErrorCode::ERR_INVALID_NUM_ARGS_ERROR;
    }

    auto isHidumperServiceCall = (IPCSkeleton::GetCallingUid() == HIDUMPER_SERVICE_UID);
    if (!isHidumperServiceCall) {
        result.append(MSG_DUMP_FAIL, strlen(MSG_DUMP_FAIL))
            .append(MSG_DUMP_FAIL_REASON_PERMISSION_DENY, strlen(MSG_DUMP_FAIL_REASON_PERMISSION_DENY));
        TAG_LOGE(AAFwkTag::APPMGR, "permission deny");
        return DumpErrorCode::ERR_PERMISSION_DENY_ERROR;
    }

    std::string pidsRaw = Str16ToStr8(args[INDEX_PID]);
    std::string customArgs;
    if (args.size() > INDEX_WEB_CUSTOM_ARGS) {
        customArgs = Str16ToStr8(args[INDEX_WEB_CUSTOM_ARGS]);
    }
    TAG_LOGD(AAFwkTag::APPMGR, "pids:%{public}s, customArgs:%{public}s", pidsRaw.c_str(), customArgs.c_str());
    std::vector<int32_t> pids;
    auto ret = DumpParsePids(pidsRaw, pids);
    if (ret != ERR_OK) {
        return ret;
    }
    if (appMgrServiceInner_ == nullptr) {
        return DumpErrorCode::ERR_INTERNAL_ERROR;
    }
    return appMgrServiceInner_->DumpArkWeb(pids, customArgs, result);
}

void AppMgrService::ScheduleAcceptWantDone(const int32_t recordId, const AAFwk::Want &want, const std::string &flag)
{
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "not ready");
        return;
    }
    if (!JudgeAppSelfCalled(recordId)) {
        return;
    }
    auto task = [appMgrServiceInner = appMgrServiceInner_, recordId, want, flag]() {
        appMgrServiceInner->ScheduleAcceptWantDone(recordId, want, flag);
    };
    taskHandler_->SubmitTask(task, "ScheduleAcceptWantDone");
}

void AppMgrService::ScheduleNewProcessRequestDone(const int32_t recordId, const AAFwk::Want &want,
    const std::string &flag)
{
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "not ready");
        return;
    }
    if (!JudgeAppSelfCalled(recordId)) {
        return;
    }
    auto task = [appMgrServiceInner = appMgrServiceInner_, recordId, want, flag]() {
        appMgrServiceInner->ScheduleNewProcessRequestDone(recordId, want, flag);
    };
    taskHandler_->SubmitTask(task, AAFwk::TaskAttribute{
        .taskName_ = "ScheduleNewProcessRequestDone",
        .taskQos_ = AAFwk::TaskQoS::USER_INTERACTIVE
    });
}

int AppMgrService::GetAbilityRecordsByProcessID(const int pid, std::vector<sptr<IRemoteObject>> &tokens)
{
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "not ready");
        return ERR_INVALID_OPERATION;
    }
    auto isSaCall = AAFwk::PermissionVerification::GetInstance()->IsSACall();
    if (!isSaCall) {
        TAG_LOGE(AAFwkTag::APPMGR, "null Sacall");
        return ERR_INVALID_OPERATION;
    }
    return appMgrServiceInner_->GetAbilityRecordsByProcessID(pid, tokens);
}

int32_t AppMgrService::PreStartNWebSpawnProcess()
{
    XCOLLIE_TIMER_LESS(__PRETTY_FUNCTION__);
    TAG_LOGI(AAFwkTag::APPMGR, "PreStartNWebSpawnProcess");
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "not ready");
        return ERR_INVALID_OPERATION;
    }

    return appMgrServiceInner_->PreStartNWebSpawnProcess(IPCSkeleton::GetCallingPid());
}

int32_t AppMgrService::StartRenderProcess(const std::string &renderParam, int32_t ipcFd,
    int32_t sharedFd, int32_t crashFd, pid_t &renderPid, bool isGPU)
{
    XCOLLIE_TIMER_LESS(__PRETTY_FUNCTION__);
    FdGuard ipcFdGuard(ipcFd);
    FdGuard sharedFdGuard(sharedFd);
    FdGuard crashFdGuard(crashFd);
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "not ready");
        return ERR_INVALID_OPERATION;
    }

    return appMgrServiceInner_->StartRenderProcess(IPCSkeleton::GetCallingPid(), renderParam,
        std::move(ipcFdGuard), std::move(sharedFdGuard), std::move(crashFdGuard), renderPid, isGPU);
}

void AppMgrService::AttachRenderProcess(const sptr<IRemoteObject> &scheduler)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGI(AAFwkTag::APPMGR, "AttachRenderProcess");
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "not ready");
        return;
    }

    auto pid = IPCSkeleton::GetCallingPid();
    auto fun = [appMgrServiceInner = appMgrServiceInner_, pid, scheduler]() {
        appMgrServiceInner->AttachRenderProcess(pid, iface_cast<IRenderScheduler>(scheduler));
    };
    taskHandler_->SubmitTask(fun, AAFwk::TaskAttribute{
        .taskName_ = TASK_ATTACH_RENDER_PROCESS,
        .taskQos_ = AAFwk::TaskQoS::USER_INTERACTIVE
    });
}

void AppMgrService::SaveBrowserChannel(sptr<IRemoteObject> browser)
{
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "not ready");
        return;
    }

    appMgrServiceInner_->SaveBrowserChannel(IPCSkeleton::GetCallingPid(), browser);
}

int32_t AppMgrService::GetRenderProcessTerminationStatus(pid_t renderPid, int &status)
{
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "not ready");
        return ERR_INVALID_OPERATION;
    }

    return appMgrServiceInner_->GetRenderProcessTerminationStatus(renderPid, status);
}

int32_t AppMgrService::GetConfiguration(Configuration& config)
{
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "not ready");
        return ERR_INVALID_OPERATION;
    }
    config = *(appMgrServiceInner_->GetConfiguration());
    return ERR_OK;
}

int32_t AppMgrService::GetConfiguration(Configuration& config, int32_t userId)
{
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "not ready");
        return ERR_INVALID_OPERATION;
    }
    config = *(appMgrServiceInner_->GetConfiguration(userId));
    return ERR_OK;
}

int32_t AppMgrService::UpdateConfiguration(const Configuration& config, const int32_t userId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "not ready");
        return ERR_INVALID_OPERATION;
    }
    return appMgrServiceInner_->UpdateConfiguration(config, userId);
}

int32_t AppMgrService::UpdateConfigurationByUserIds(
    const Configuration& config, const std::vector<int32_t> userIds)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "not ready");
        return ERR_INVALID_OPERATION;
    }

    return appMgrServiceInner_->UpdateConfigurationByUserIds(config, userIds);
}

int32_t AppMgrService::UpdateConfigurationForBackgroundApp(const std::vector<BackgroundAppInfo>& appInfos,
    const AppExecFwk::ConfigurationPolicy& policy, const int32_t userId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (!AAFwk::PermissionVerification::GetInstance()->IsSACall()) {
        TAG_LOGE(AAFwkTag::APPMGR, "caller not SA");
        return ERR_PERMISSION_DENIED;
    }
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "not ready");
        return ERR_INVALID_OPERATION;
    }
    return appMgrServiceInner_->UpdateConfigurationForBackgroundApp(appInfos, policy, userId);
}

int32_t AppMgrService::UpdateConfigurationByBundleName(const Configuration& config, const std::string &name,
    int32_t appIndex)
{
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "not ready");
        return ERR_INVALID_OPERATION;
    }
    return appMgrServiceInner_->UpdateConfigurationByBundleName(config, name, appIndex);
}

int32_t AppMgrService::RegisterConfigurationObserver(const sptr<IConfigurationObserver> &observer,
    const int32_t userId)
{
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "not ready");
        return ERR_INVALID_OPERATION;
    }
    return appMgrServiceInner_->RegisterConfigurationObserver(observer, userId);
}

int32_t AppMgrService::UnregisterConfigurationObserver(const sptr<IConfigurationObserver> &observer)
{
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "not ready");
        return ERR_INVALID_OPERATION;
    }
    return appMgrServiceInner_->UnregisterConfigurationObserver(observer);
}

bool AppMgrService::GetAppRunningStateByBundleName(const std::string &bundleName)
{
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "not ready");
        return false;
    }

    return appMgrServiceInner_->GetAppRunningStateByBundleName(bundleName);
}

int32_t AppMgrService::NotifyLoadRepairPatch(const std::string &bundleName, const sptr<IQuickFixCallback> &callback)
{
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "not ready");
        return ERR_INVALID_OPERATION;
    }
    return appMgrServiceInner_->NotifyLoadRepairPatch(bundleName, callback);
}

int32_t AppMgrService::NotifyHotReloadPage(const std::string &bundleName, const sptr<IQuickFixCallback> &callback)
{
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "not ready");
        return ERR_INVALID_OPERATION;
    }
    return appMgrServiceInner_->NotifyHotReloadPage(bundleName, callback);
}

#ifdef BGTASKMGR_CONTINUOUS_TASK_ENABLE
int32_t AppMgrService::SetContinuousTaskProcess(int32_t pid, bool isContinuousTask)
{
    if (!AAFwk::PermissionVerification::GetInstance()->CheckSpecificSystemAbilityAccessPermission(BS_PROCESS_NAME) &&
        !AAFwk::PermissionVerification::GetInstance()->CheckSpecificSystemAbilityAccessPermission(FOUNDATION_PROCESS)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "not rss or foundation");
        return ERR_INVALID_OPERATION;
    }
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "not ready");
        return ERR_INVALID_OPERATION;
    }

    return appMgrServiceInner_->SetContinuousTaskProcess(pid, isContinuousTask);
}
#endif

int32_t AppMgrService::NotifyUnLoadRepairPatch(const std::string &bundleName, const sptr<IQuickFixCallback> &callback)
{
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "not ready");
        return ERR_INVALID_OPERATION;
    }
    return appMgrServiceInner_->NotifyUnLoadRepairPatch(bundleName, callback);
}

bool AppMgrService::JudgeAppSelfCalled(int32_t recordId)
{
    if (appMgrServiceInner_ == nullptr) {
        return false;
    }

    auto callingTokenId = IPCSkeleton::GetCallingTokenID();
    std::shared_ptr<AppRunningRecord> appRecord = appMgrServiceInner_->GetAppRunningRecordByAppRecordId(recordId);
    if (appRecord == nullptr || ((appRecord->GetApplicationInfo())->accessTokenId) != callingTokenId) {
        TAG_LOGE(AAFwkTag::APPMGR, "not enabled");
        return false;
    }

    return true;
}

bool AppMgrService::IsSharedBundleRunning(const std::string &bundleName, uint32_t versionCode)
{
    if (!IsReady()) {
        return false;
    }
    return appMgrServiceInner_->IsSharedBundleRunning(bundleName, versionCode);
}

int32_t AppMgrService::StartNativeProcessForDebugger(const AAFwk::Want &want)
{
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "not ready");
        return ERR_INVALID_OPERATION;
    }
    bool isShellCall = AAFwk::PermissionVerification::GetInstance()->IsShellCall();
    auto callingTokenId = IPCSkeleton::GetCallingTokenID();
    bool isLocalDebugCall = AAFwk::PermissionVerification::GetInstance()->VerifyStartLocalDebug(callingTokenId);
    if (!isShellCall && !isLocalDebugCall) {
        TAG_LOGE(AAFwkTag::APPMGR, "permission denied");
        return ERR_INVALID_OPERATION;
    }
    auto ret = appMgrServiceInner_->StartNativeProcessForDebugger(want);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "start native process fail");
    }
    return ret;
}

int32_t AppMgrService::GetBundleNameByPid(const int32_t pid, std::string &bundleName, int32_t &uid)
{
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "not ready");
        return ERR_INVALID_OPERATION;
    }
    return appMgrServiceInner_->GetBundleNameByPid(pid, bundleName, uid);
}

int32_t AppMgrService::GetRunningProcessInfoByPid(const pid_t pid, OHOS::AppExecFwk::RunningProcessInfo &info)
{
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "not ready");
        return ERR_INVALID_OPERATION;
    }
    return appMgrServiceInner_->GetRunningProcessInfoByPid(pid, info);
}

int32_t AppMgrService::GetRunningProcessInfoByChildProcessPid(const pid_t childPid,
    OHOS::AppExecFwk::RunningProcessInfo &info)
{
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "not ready");
        return ERR_INVALID_OPERATION;
    }
    return appMgrServiceInner_->GetRunningProcessInfoByChildProcessPid(childPid, info);
}

int32_t AppMgrService::NotifyAppFault(const FaultData &faultData)
{
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "not ready");
        return ERR_INVALID_OPERATION;
    }

    auto ret = appMgrServiceInner_->NotifyAppFault(faultData);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "faultData fail");
    }
    return ret;
}

int32_t AppMgrService::NotifyAppFaultBySA(const AppFaultDataBySA &faultData)
{
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "not ready");
        return ERR_INVALID_OPERATION;
    }

    auto ret = appMgrServiceInner_->NotifyAppFaultBySA(faultData);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "faultData fail");
    }
    return ret;
}

bool AppMgrService::SetAppFreezeFilter(int32_t pid)
{
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "not ready");
        return false;
    }

    auto ret = appMgrServiceInner_->SetAppFreezeFilter(pid);
    if (!ret) {
        TAG_LOGE(AAFwkTag::APPMGR, "SetAppFreezeFilter fail");
    }
    return ret;
}

int32_t AppMgrService::GetProcessMemoryByPid(const int32_t pid, int32_t &memorySize)
{
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "not ready");
        return ERR_INVALID_OPERATION;
    }

    return appMgrServiceInner_->GetProcessMemoryByPid(pid, memorySize);
}

int32_t AppMgrService::GetRunningProcessInformation(const std::string &bundleName, int32_t userId,
    std::vector<RunningProcessInfo> &info)
{
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "not ready");
        return ERR_INVALID_OPERATION;
    }

    return appMgrServiceInner_->GetRunningProcessInformation(bundleName, userId, info);
}

void AppMgrService::OnAddSystemAbility(int32_t systemAbilityId, const std::string& deviceId)
{
    TAG_LOGI(AAFwkTag::APPMGR, "systemAbilityId: %{public}d add", systemAbilityId);
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "not ready");
        return;
    }
    if (systemAbilityId == URI_PERMISSION_MGR_SERVICE_ID) {
        AAFwk::UriPermissionManagerClient::GetInstance().SetUriPermServiceStarted();
    }

    if (systemAbilityId != WINDOW_MANAGER_SERVICE_ID) {
        return;
    }

    appMgrServiceInner_->InitFocusListener();
#ifdef SUPPORT_SCREEN
    appMgrServiceInner_->InitWindowVisibilityChangedListener();
    appMgrServiceInner_->InitWindowPidVisibilityChangedListener();
#endif
}

void AppMgrService::OnRemoveSystemAbility(int32_t systemAbilityId, const std::string& deviceId)
{
    TAG_LOGI(AAFwkTag::APPMGR, "systemAbilityId: %{public}d remove", systemAbilityId);
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "not ready");
        return;
    }

    if (systemAbilityId != WINDOW_MANAGER_SERVICE_ID) {
        return;
    }

    appMgrServiceInner_->FreeFocusListener();
#ifdef SUPPORT_SCREEN
    appMgrServiceInner_->FreeWindowVisibilityChangedListener();
    appMgrServiceInner_->FreeWindowPidVisibilityChangedListener();
#endif
}

int32_t AppMgrService::ChangeAppGcState(pid_t pid, int32_t state, uint64_t tid)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    if (!appMgrServiceInner_) {
        return ERR_INVALID_VALUE;
    }
    return appMgrServiceInner_->ChangeAppGcState(pid, state, tid);
}

int32_t AppMgrService::NotifyPageShow(const sptr<IRemoteObject> &token, const PageStateData &pageStateData)
{
    TAG_LOGD(AAFwkTag::APPMGR,
        "bundleName: %{public}s, moduelName: %{public}s, abilityName: %{public}s, pageName: %{public}s",
        pageStateData.bundleName.c_str(), pageStateData.moduleName.c_str(), pageStateData.abilityName.c_str(),
        pageStateData.pageName.c_str());
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "not ready");
        return ERR_INVALID_OPERATION;
    }
    return appMgrServiceInner_->NotifyPageShow(token, pageStateData);
}

int32_t AppMgrService::NotifyPageHide(const sptr<IRemoteObject> &token, const PageStateData &pageStateData)
{
    TAG_LOGD(AAFwkTag::APPMGR,
        "bundleName: %{public}s, moduelName: %{public}s, abilityName: %{public}s, pageName: %{public}s",
        pageStateData.bundleName.c_str(), pageStateData.moduleName.c_str(), pageStateData.abilityName.c_str(),
        pageStateData.pageName.c_str());
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "not ready");
        return ERR_INVALID_OPERATION;
    }
    return appMgrServiceInner_->NotifyPageHide(token, pageStateData);
}

int32_t AppMgrService::RegisterAppRunningStatusListener(const sptr<IRemoteObject> &listener)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "not ready");
        return ERR_INVALID_OPERATION;
    }
    return appMgrServiceInner_->RegisterAppRunningStatusListener(listener);
}

int32_t AppMgrService::UnregisterAppRunningStatusListener(const sptr<IRemoteObject> &listener)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "not ready");
        return ERR_INVALID_OPERATION;
    }
    return appMgrServiceInner_->UnregisterAppRunningStatusListener(listener);
}

int32_t AppMgrService::RegisterAppForegroundStateObserver(const sptr<IAppForegroundStateObserver> &observer)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "not ready");
        return ERR_INVALID_OPERATION;
    }
    return appMgrServiceInner_->RegisterAppForegroundStateObserver(observer);
}

int32_t AppMgrService::UnregisterAppForegroundStateObserver(const sptr<IAppForegroundStateObserver> &observer)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "not ready");
        return ERR_INVALID_OPERATION;
    }
    return appMgrServiceInner_->UnregisterAppForegroundStateObserver(observer);
}

int32_t AppMgrService::IsApplicationRunning(const std::string &bundleName, bool &isRunning)
{
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "not ready");
        return ERR_INVALID_OPERATION;
    }
    return appMgrServiceInner_->IsApplicationRunning(bundleName, isRunning);
}

int32_t AppMgrService::IsAppRunning(const std::string &bundleName, int32_t appCloneIndex, bool &isRunning)
{
    isRunning = false;
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "not ready");
        return ERR_INVALID_OPERATION;
    }
    return appMgrServiceInner_->IsAppRunning(bundleName, appCloneIndex, isRunning);
}

int32_t AppMgrService::IsAppRunningByBundleNameAndUserId(const std::string &bundleName, int32_t userId,
    bool &isRunning)
{
    isRunning = false;
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "not ready");
        return ERR_INVALID_OPERATION;
    }
    return appMgrServiceInner_->IsAppRunningByBundleNameAndUserId(bundleName, userId, isRunning);
}

int32_t AppMgrService::IsAppRunning(const std::string &bundleName, int32_t appCloneIndex,
    int32_t userId, bool &isRunning)
{
    isRunning = false;
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "not ready");
        return ERR_INVALID_OPERATION;
    }
    return appMgrServiceInner_->IsAppRunning(bundleName, appCloneIndex, userId, isRunning);
}

#ifdef SUPPORT_CHILD_PROCESS
int32_t AppMgrService::StartChildProcess(pid_t &childPid, const ChildProcessRequest &request)
{
    XCOLLIE_TIMER_LESS(__PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    std::vector<FdGuard> fds;
    for (const auto &[name, fd] : request.args.fds) {
        fds.emplace_back(fd);
    }
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "not ready");
        return ERR_INVALID_OPERATION;
    }

    return appMgrServiceInner_->StartChildProcess(IPCSkeleton::GetCallingPid(), childPid, request);
}

int32_t AppMgrService::GetChildProcessInfoForSelf(ChildProcessInfo &info)
{
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "not ready");
        return ERR_INVALID_OPERATION;
    }
    return appMgrServiceInner_->GetChildProcessInfoForSelf(info);
}

void AppMgrService::AttachChildProcess(const sptr<IRemoteObject> &childScheduler)
{
    TAG_LOGD(AAFwkTag::APPMGR, "AttachChildProcess.");
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "not ready");
        return;
    }
    if (!taskHandler_) {
        TAG_LOGE(AAFwkTag::APPMGR, "null taskHandler_");
        return;
    }
    pid_t pid = IPCSkeleton::GetCallingPid();
    std::function<void()> task = [appMgrServiceInner = appMgrServiceInner_, pid, childScheduler]() {
        appMgrServiceInner->AttachChildProcess(pid, iface_cast<IChildScheduler>(childScheduler));
    };
    taskHandler_->SubmitTask(task, AAFwk::TaskAttribute{
        .taskName_ = TASK_ATTACH_CHILD_PROCESS,
        .taskQos_ = AAFwk::TaskQoS::USER_INTERACTIVE
    });
}

void AppMgrService::ExitChildProcessSafely()
{
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "not ready");
        return;
    }
    if (!taskHandler_) {
        TAG_LOGE(AAFwkTag::APPMGR, "null taskHandler_");
        return;
    }
    pid_t pid = IPCSkeleton::GetCallingPid();
    std::function<void()> task = [appMgrServiceInner = appMgrServiceInner_, pid]() {
        appMgrServiceInner->ExitChildProcessSafelyByChildPid(pid);
    };
    taskHandler_->SubmitTask(task, AAFwk::TaskAttribute{
        .taskName_ = TASK_EXIT_CHILD_PROCESS_SAFELY,
        .taskQos_ = AAFwk::TaskQoS::USER_INTERACTIVE
    });
}
#endif // SUPPORT_CHILD_PROCESS

bool AppMgrService::IsFinalAppProcess()
{
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "not ready");
        return false;
    }
    return appMgrServiceInner_->IsFinalAppProcessByBundleName("");
}

int32_t AppMgrService::RegisterRenderStateObserver(const sptr<IRenderStateObserver> &observer)
{
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "not ready");
        return ERR_INVALID_OPERATION;
    }

    if (AAFwk::PermissionVerification::GetInstance()->VerifyAppStateObserverPermission() == ERR_PERMISSION_DENIED) {
        TAG_LOGE(AAFwkTag::APPMGR, "verification failed");
        return ERR_PERMISSION_DENIED;
    }
    return appMgrServiceInner_->RegisterRenderStateObserver(observer);
}

int32_t AppMgrService::UnregisterRenderStateObserver(const sptr<IRenderStateObserver> &observer)
{
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "not ready");
        return ERR_INVALID_OPERATION;
    }

    if (AAFwk::PermissionVerification::GetInstance()->VerifyAppStateObserverPermission() == ERR_PERMISSION_DENIED) {
        TAG_LOGE(AAFwkTag::APPMGR, "verification failed");
        return ERR_PERMISSION_DENIED;
    }
    return appMgrServiceInner_->UnregisterRenderStateObserver(observer);
}

int32_t AppMgrService::UpdateRenderState(pid_t renderPid, int32_t state)
{
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "not ready");
        return ERR_INVALID_OPERATION;
    }
    return appMgrServiceInner_->UpdateRenderState(renderPid, state);
}

int32_t AppMgrService::SignRestartAppFlag(int32_t uid, const std::string &instanceKey)
{
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "not ready");
        return ERR_INVALID_OPERATION;
    }
    bool isCallingPermission =
        AAFwk::PermissionVerification::GetInstance()->CheckSpecificSystemAbilityAccessPermission(FOUNDATION_PROCESS);
    if (!isCallingPermission) {
        TAG_LOGE(AAFwkTag::APPMGR, "verification failed");
        return ERR_PERMISSION_DENIED;
    }
    return appMgrServiceInner_->SignRestartAppFlag(uid, instanceKey);
}

int32_t AppMgrService::SignRestartProcess(int32_t pid)
{
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "not ready");
        return AAFwk::ERR_APP_MGR_SERVICE_NOT_READY;
    }
    bool isCallingPermission =
        AAFwk::PermissionVerification::GetInstance()->CheckSpecificSystemAbilityAccessPermission(FOUNDATION_PROCESS);
    if (!isCallingPermission) {
        TAG_LOGE(AAFwkTag::APPMGR, "not foundation");
        return AAFwk::ERR_NO_ALLOW_OUTSIDE_CALL;
    }
    return appMgrServiceInner_->SignRestartProcess(pid);
}

int32_t AppMgrService::GetAppRunningUniqueIdByPid(pid_t pid, std::string &appRunningUniqueId)
{
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "not ready");
        return ERR_INVALID_OPERATION;
    }
    return appMgrServiceInner_->GetAppRunningUniqueIdByPid(pid, appRunningUniqueId);
}

int32_t AppMgrService::GetAllUIExtensionRootHostPid(pid_t pid, std::vector<pid_t> &hostPids)
{
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "not ready");
        return ERR_INVALID_OPERATION;
    }

    return appMgrServiceInner_->GetAllUIExtensionRootHostPid(pid, hostPids);
}

int32_t AppMgrService::GetAllUIExtensionProviderPid(pid_t hostPid, std::vector<pid_t> &providerPids)
{
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "not ready");
        return ERR_INVALID_OPERATION;
    }

    return appMgrServiceInner_->GetAllUIExtensionProviderPid(hostPid, providerPids);
}

int32_t AppMgrService::NotifyMemorySizeStateChanged(int32_t memorySizeState)
{
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "not ready");
        return ERR_INVALID_OPERATION;
    }

    return appMgrServiceInner_->NotifyMemorySizeStateChanged(memorySizeState);
}

int32_t AppMgrService::SetSupportedProcessCacheSelf(bool isSupport)
{
    TAG_LOGD(AAFwkTag::APPMGR, "call");
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "not ready");
        return ERR_INVALID_OPERATION;
    }
    return appMgrServiceInner_->SetSupportedProcessCacheSelf(isSupport);
}

int32_t AppMgrService::SetSupportedProcessCache(int32_t pid, bool isSupport)
{
    TAG_LOGD(AAFwkTag::APPMGR, "Called");
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "Not ready.");
        return ERR_INVALID_OPERATION;
    }
    if (!AAFwk::PermissionVerification::GetInstance()->CheckSpecificSystemAbilityAccessPermission(FOUNDATION_PROCESS)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "caller not foundation");
        return ERR_INVALID_OPERATION;
    }
    return appMgrServiceInner_->SetSupportedProcessCache(pid, isSupport);
}

int32_t AppMgrService::IsProcessCacheSupported(int32_t pid, bool &isSupported)
{
    TAG_LOGD(AAFwkTag::APPMGR, "get supported process cache");
    if (appMgrServiceInner_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "appMgrServiceInner_ is nullptr");
        return AAFwk::ERR_NULL_APP_MGR_SERVICE_INNER;
    }
    if (!AAFwk::PermissionVerification::GetInstance()->CheckSpecificSystemAbilityAccessPermission(
        FOUNDATION_PROCESS)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "caller not foundation");
        return AAFwk::ERR_NO_PERMISSION_CALLER;
    }
    return appMgrServiceInner_->IsProcessCacheSupported(pid, isSupported);
}

int32_t AppMgrService::SetProcessCacheEnable(int32_t pid, bool enable)
{
    TAG_LOGD(AAFwkTag::APPMGR, "set enable process cache");
    if (appMgrServiceInner_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "appMgrServiceInner_ is nullptr");
        return AAFwk::ERR_NULL_APP_MGR_SERVICE_INNER;
    }
    if (!AAFwk::PermissionVerification::GetInstance()->CheckSpecificSystemAbilityAccessPermission(
        FOUNDATION_PROCESS)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "caller not foundation");
        return AAFwk::ERR_NO_PERMISSION_CALLER;
    }
    return appMgrServiceInner_->SetProcessCacheEnable(pid, enable);
}

int32_t AppMgrService::LockProcessCache(int32_t pid, bool isLock)
{
    TAG_LOGD(AAFwkTag::APPMGR, "lock process cache");
    if (appMgrServiceInner_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "appMgrServiceInner_ is nullptr");
        return AAFwk::ERR_NULL_APP_MGR_SERVICE_INNER;
    }
    if (!AAFwk::PermissionVerification::GetInstance()->CheckSpecificSystemAbilityAccessPermission(
        FOUNDATION_PROCESS)) {
        TAG_LOGE(AAFwkTag::APPMGR, "caller not foundation");
        return AAFwk::ERR_NO_PERMISSION_CALLER;
    }
    return appMgrServiceInner_->LockProcessCache(pid, isLock);
}

void AppMgrService::SetAppAssertionPauseState(bool flag)
{
    TAG_LOGI(AAFwkTag::APPMGR, "call");
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "not ready");
        return;
    }
    return appMgrServiceInner_->SetAppAssertionPauseState(flag);
}

#ifdef SUPPORT_CHILD_PROCESS
int32_t AppMgrService::CreateNativeChildProcess(const std::string &libName,
    const sptr<IRemoteObject> &callback, const ChildProcessRequest &request)
{
    XCOLLIE_TIMER_LESS(__PRETTY_FUNCTION__);
    TAG_LOGI(AAFwkTag::APPMGR, "call");
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "not ready");
        return ERR_INVALID_OPERATION;
    }
    
    return appMgrServiceInner_->CreateNativeChildProcess(
        IPCSkeleton::GetCallingPid(), libName, callback, request);
}
#endif // SUPPORT_CHILD_PROCESS

int32_t AppMgrService::CheckCallingIsUserTestMode(const pid_t pid, bool &isUserTest)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    if (!appMgrServiceInner_) {
        return ERR_INVALID_VALUE;
    }
    return appMgrServiceInner_->CheckCallingIsUserTestModeInner(pid, isUserTest);
}

int32_t AppMgrService::NotifyProcessDependedOnWeb()
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    if (!appMgrServiceInner_) {
        TAG_LOGE(AAFwkTag::APPMGR, "not ready");
        return ERR_INVALID_VALUE;
    }
    return appMgrServiceInner_->NotifyProcessDependedOnWeb();
}

void AppMgrService::KillProcessDependedOnWeb()
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    if (!AAFwk::PermissionVerification::GetInstance()->VerifyKillProcessDependedOnWebPermission()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "no permission");
        return;
    }
    if (!appMgrServiceInner_) {
        return;
    }
    appMgrServiceInner_->KillProcessDependedOnWeb();
}

void AppMgrService::RestartResidentProcessDependedOnWeb()
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    if (!AAFwk::PermissionVerification::GetInstance()->CheckSpecificSystemAbilityAccessPermission(FOUNDATION_PROCESS)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "not foundation");
        return;
    }
    if (!appMgrServiceInner_) {
        return;
    }
    appMgrServiceInner_->RestartResidentProcessDependedOnWeb();
}

int32_t AppMgrService::GetSupportedProcessCachePids(const std::string &bundleName,
    std::vector<int32_t> &pidList)
{
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "not ready");
        return ERR_INVALID_OPERATION;
    }
    if (!AAFwk::PermissionVerification::GetInstance()->JudgeCallerIsAllowedToUseSystemAPI()) {
        TAG_LOGE(AAFwkTag::APPMGR, "caller is not SA");
        return AAFwk::ERR_NOT_SYSTEM_APP;
    }
    auto isPerm = AAFwk::PermissionVerification::GetInstance()->VerifyRunningInfoPerm();
    if (!isPerm) {
        return AAFwk::CHECK_PERMISSION_FAILED;
    }
    if (!DelayedSingleton<CacheProcessManager>::GetInstance()->QueryEnableProcessCache()) {
        return AAFwk::ERR_CAPABILITY_NOT_SUPPORT;
    }
    return appMgrServiceInner_->GetSupportedProcessCachePids(bundleName, pidList);
}

int32_t AppMgrService::RegisterKiaInterceptor(const sptr<IKiaInterceptor> &interceptor)
{
    TAG_LOGD(AAFwkTag::APPMGR, "Called");
    if (!appMgrServiceInner_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appMgrServiceInner_ is nullptr");
        return ERR_INVALID_VALUE;
    }
    return appMgrServiceInner_->RegisterKiaInterceptor(interceptor);
}

int32_t AppMgrService::CheckIsKiaProcess(pid_t pid, bool &isKia)
{
    TAG_LOGD(AAFwkTag::APPMGR, "Called");
    if (!appMgrServiceInner_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appMgrServiceInner_ is nullptr");
        return ERR_INVALID_VALUE;
    }
    return appMgrServiceInner_->CheckIsKiaProcess(pid, isKia);
}

int32_t AppMgrService::KillAppSelfWithInstanceKey(const std::string &instanceKey, bool clearPageStack,
    const std::string& reason)
{
    if (!appMgrServiceInner_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appMgrServiceInner_ is nullptr");
        return ERR_INVALID_VALUE;
    }
    return appMgrServiceInner_->KillAppSelfWithInstanceKey(instanceKey, clearPageStack, reason);
}

int32_t AppMgrService::KillProcessByPidForExit(int32_t pid, const std::string &reason)
{
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "not ready");
        return AAFwk::ERR_APP_MGR_SERVICE_NOT_READY;
    }
    bool isCallingPermission =
        AAFwk::PermissionVerification::GetInstance()->CheckSpecificSystemAbilityAccessPermission(FOUNDATION_PROCESS);
    if (!isCallingPermission) {
        TAG_LOGE(AAFwkTag::APPMGR, "not foundation");
        return AAFwk::ERR_NO_ALLOW_OUTSIDE_CALL;
    }
    return appMgrServiceInner_->KillProcessByPidForExit(pid, reason);
}

int32_t AppMgrService::IsSpecifiedModuleLoaded(const AAFwk::Want &want, const AbilityInfo &abilityInfo, bool &result,
    bool &isDebug)
{
    if (!IsReady()) {
        return ERR_INVALID_OPERATION;
    }
    pid_t callingPid = IPCSkeleton::GetCallingPid();
    pid_t pid = getprocpid();
    if (callingPid != pid) {
        TAG_LOGE(AAFwkTag::APPMGR, "other process");
        return ERR_INVALID_OPERATION;
    }
    if (!appMgrServiceInner_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appMgrServiceInner_ is nullptr");
        return ERR_INVALID_VALUE;
    }
    result = appMgrServiceInner_->IsSpecifiedModuleLoaded(want, abilityInfo, isDebug);
    return ERR_OK;
}

int32_t AppMgrService::UpdateProcessMemoryState(const std::vector<ProcessMemoryState> &procMemState)
{
    TAG_LOGI(AAFwkTag::APPMGR, "procMemState.size = %{public}zu", procMemState.size());
    if (!IsReady()) {
        return ERR_INVALID_OPERATION;
    }
    if (!AAFwk::PermissionVerification::GetInstance()->IsSACall() &&
        !AAFwk::PermissionVerification::GetInstance()->IsShellCall()) {
        TAG_LOGE(AAFwkTag::APPMGR, "caller is not sa or shell");
        return ERR_PERMISSION_DENIED;
    }
    if (!AAFwk::PermissionVerification::GetInstance()->CheckSpecificSystemAbilityAccessPermission(
        HIVIEW_PROCESS_NAME)) {
        TAG_LOGW(AAFwkTag::APPMGR, "caller not sa dfx");
    }
    if (!appMgrServiceInner_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appMgrServiceInner_ is nullptr");
        return ERR_INVALID_VALUE;
    }
    return appMgrServiceInner_->UpdateProcessMemoryState(procMemState);
}

int32_t AppMgrService::LaunchAbility(sptr<IRemoteObject> token)
{
    if (!IsReady()) {
        return AAFwk::ERR_APP_MGR_SERVICE_NOT_READY;
    }
    if (!AAFwk::PermissionVerification::GetInstance()->CheckSpecificSystemAbilityAccessPermission(FOUNDATION_PROCESS)) {
        TAG_LOGE(AAFwkTag::APPMGR, "not foundation");
        return AAFwk::ERR_NO_ALLOW_OUTSIDE_CALL;
    }
    return appMgrServiceInner_->LaunchAbility(token);
}

int32_t AppMgrService::PromoteCurrentToCandidateMasterProcess(bool isInsertToHead)
{
    TAG_LOGD(AAFwkTag::APPMGR, "call");
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "Service not ready");
        return AAFwk::ERR_APP_MGR_SERVICE_NOT_READY;
    }
    return appMgrServiceInner_->PromoteCurrentToCandidateMasterProcess(isInsertToHead);
}

int32_t AppMgrService::DemoteCurrentFromCandidateMasterProcess()
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "Service not ready");
        return AAFwk::ERR_APP_MGR_SERVICE_NOT_READY;
    }
    return appMgrServiceInner_->DemoteCurrentFromCandidateMasterProcess();
}

int32_t AppMgrService::ExitMasterProcessRole()
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "Service not ready");
        return AAFwk::ERR_APP_MGR_SERVICE_NOT_READY;
    }
    return appMgrServiceInner_->ExitMasterProcessRole();
}

void AppMgrService::SetSpecifiedProcessRequestId(int32_t recordId, int32_t requestId)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "Service not ready");
        return;
    }
    pid_t callingPid = IPCSkeleton::GetCallingPid();
    pid_t pid = getprocpid();
    if (callingPid != pid) {
        TAG_LOGE(AAFwkTag::APPMGR, "not process call");
        return;
    }
    appMgrServiceInner_->SetSpecifiedProcessRequestId(recordId, requestId);
}

int32_t AppMgrService::QueryRunningSharedBundles(pid_t pid, std::map<std::string, uint32_t> &sharedBundles)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "Service not ready");
        return AAFwk::ERR_APP_MGR_SERVICE_NOT_READY;
    }
    if (!appMgrServiceInner_->IsFoundationCall()) {
        TAG_LOGE(AAFwkTag::APPMGR, "Not foundation call.");
        return ERR_PERMISSION_DENIED;
    }
    return appMgrServiceInner_->QueryRunningSharedBundles(pid, sharedBundles);
}

int32_t AppMgrService::RegisterApplicationStateObserverWithFilter(sptr<IApplicationStateObserver> observer,
    const std::vector<std::string> &bundleNameList, const AppStateFilter &appStateFilter, bool isUsingFilter)
{
    TAG_LOGD(AAFwkTag::APPMGR, "begin");
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "not ready");
        return ERR_INVALID_OPERATION;
    }
    int32_t ret = 0;
    if (isUsingFilter) {
        CHECK_CALLER_IS_SYSTEM_APP;
        ret = appMgrServiceInner_->RegisterApplicationStateObserver(observer, bundleNameList, appStateFilter);
    } else {
        FilterCallback callbacks = static_cast<FilterCallback>(
            static_cast<uint32_t>(FilterCallback::ON_FOREGROUND_APPLICATION_CHANGED) |
            static_cast<uint32_t>(FilterCallback::ON_ABILITY_STATE_CHANGED) |
            static_cast<uint32_t>(FilterCallback::ON_PROCESS_CREATED) |
            static_cast<uint32_t>(FilterCallback::ON_PROCESS_DIED) |
            static_cast<uint32_t>(FilterCallback::ON_PROCESS_STATE_CHANGED) |
            static_cast<uint32_t>(FilterCallback::ON_APP_STARTED) |
            static_cast<uint32_t>(FilterCallback::ON_APP_STOPPED) |
            static_cast<uint32_t>(FilterCallback::ON_EXTENSION_STATE_CHANGED)
        );
        AppStateFilter defaultAppStateFilter {callbacks, FilterBundleType::ALL, FilterAppStateType::ALL,
            FilterProcessStateType::ALL, FilterAbilityStateType::ALL};
        ret = appMgrServiceInner_->RegisterApplicationStateObserver(observer, bundleNameList, defaultAppStateFilter);
    }
    return ret;
}

void AppMgrService::AllowScbProcessMoveToBackground()
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "Service not ready");
        return;
    }
    appMgrServiceInner_->AllowScbProcessMoveToBackground();
}

int32_t AppMgrService::KillChildProcessByPid(int32_t pid)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "Service not ready");
        return AAFwk::ERR_APP_MGR_SERVICE_NOT_READY;
    }
    return appMgrServiceInner_->KillChildProcessByPid(pid);
}

void AppMgrService::SetProcessPrepareExit(int32_t pid)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "Service not ready");
        return;
    }
    if (!appMgrServiceInner_->IsFoundationCall()) {
        TAG_LOGE(AAFwkTag::APPMGR, "Not foundation call.");
        return;
    }
    appMgrServiceInner_->SetProcessPrepareExit(pid);
}

int32_t AppMgrService::GetAllAbilityInfos(const int32_t pid, std::vector<AppExecFwk::AbilityStateData> &infos)
{
    auto callingUid = IPCSkeleton::GetCallingUid();
    bool isRssCalling = AAFwk::PermissionVerification::GetInstance()->CheckSpecificSystemAbilityAccessPermission(
        BS_PROCESS_NAME) && (callingUid == RESOURCE_MANAGER_UID);
    if (!isRssCalling) {
        TAG_LOGE(AAFwkTag::APPMGR, "Permission denied");
        return ERR_PERMISSION_DENIED;
    }
    if (!appMgrServiceInner_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appMgrServiceInner_ is nullptr");
        return AAFwk::ERR_APP_MGR_SERVICE_NOT_READY;
    }
    return appMgrServiceInner_->GetAllAbilityInfos(pid, infos);
}
}  // namespace AppExecFwk
}  // namespace OHOS
