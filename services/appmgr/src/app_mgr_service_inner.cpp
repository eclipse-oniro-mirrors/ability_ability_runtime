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

#include "app_mgr_service_inner.h"

#include <cinttypes>
#include <csignal>
#include <cstdint>
#include <mutex>
#include <nlohmann/json.hpp>
#include <queue>
#include <securec.h>
#include <sstream>
#include <sys/stat.h>
#include <unistd.h>

#include "ability_manager_errors.h"
#include "ability_window_configuration.h"
#include "accesstoken_kit.h"
#include "ams_configuration_parameter.h"
#include "app_config_data_manager.h"
#include "app_mem_info.h"
#include "app_mgr_service.h"
#include "app_mgr_event.h"
#include "app_process_data.h"
#include "app_state_observer_manager.h"
#include "app_utils.h"
#include "appfreeze_manager.h"
#include "application_state_observer_stub.h"
#include "appspawn_util.h"
#include "bundle_constants.h"
#include "common_event.h"
#include "common_event_manager.h"
#include "common_event_support.h"
#include "datetime_ex.h"
#include "distributed_data_mgr.h"
#include "exit_resident_process_manager.h"
#include "extension_ability_info.h"
#include "freeze_util.h"
#include "global_constant.h"
#include "hap_token_info.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "in_process_call_wrapper.h"
#include "ipc_skeleton.h"
#include "iremote_object.h"
#include "iservice_registry.h"
#include "itest_observer.h"
#include "killing_process_manager.h"
#include "last_exit_detail_info.h"
#include "os_account_manager.h"
#ifdef SUPPORT_SCREEN
#include "locale_config.h"
#endif
#include "mem_mgr_client.h"
#include "mem_mgr_process_state_info.h"
#include "os_account_manager_wrapper.h"
#ifdef OHOS_ACCOUNT_ENABLED
#include "ohos_account_kits.h"
#endif // OHOS_ACCOUNT_ENABLED
#include "parameter.h"
#include "parameters.h"
#include "perf_profile.h"
#include "permission_constants.h"
#include "permission_verification.h"
#include "render_state_observer_manager.h"
#include "res_sched_util.h"
#include "startup_util.h"
#include "string_ex.h"
#ifdef ABILITY_RUNTIME_FEATURE_SANDBOXMANAGER
#include "sandbox_manager_kit.h"
#endif
#include "system_ability_definition.h"
#include "time_util.h"
#include "ui_extension_utils.h"
#ifdef SUPPORT_UPMS
#include "uri_permission_manager_client.h"
#endif // SUPPORT_UPMS
#include "user_record_manager.h"
#ifdef APP_MGR_SERVICE_APPMS
#include "net_conn_client.h"
#endif
#include "application_info.h"
#include "meminfo.h"
#include "app_mgr_service_const.h"
#include "app_mgr_service_dump_error_code.h"
#include "param.h"
#include "window_focus_changed_listener.h"
#include "window_visibility_changed_listener.h"
#include "window_pid_visibility_changed_listener.h"
#include "cache_process_manager.h"
#include "res_sched_util.h"
#ifdef APP_NO_RESPONSE_DIALOG
#include "fault_data.h"
#include "modal_system_app_freeze_uiextension.h"
#endif
#ifdef APP_MGR_SERVICE_HICOLLIE_ENABLE
#include "xcollie/xcollie.h"
#include "xcollie/xcollie_define.h"
#endif

namespace OHOS {
namespace AppExecFwk {
#ifdef SUPPORT_SCREEN
using namespace OHOS::Rosen;
#endif //SUPPORT_SCREEN
using namespace OHOS::Security;

namespace {
#define CHECK_CALLER_IS_SYSTEM_APP                                                             \
    if (!AAFwk::PermissionVerification::GetInstance()->JudgeCallerIsAllowedToUseSystemAPI()) { \
        TAG_LOGE(AAFwkTag::APPMGR, "can't use SA");    \
        return AAFwk::ERR_NOT_SYSTEM_APP;                                                      \
    }

#define CHECK_IS_SA_CALL(listener)                                                                              \
    auto instance = AAFwk::PermissionVerification::GetInstance();                                               \
    if ((listener) == nullptr || instance == nullptr || appRunningStatusModule_ == nullptr) {                     \
        TAG_LOGE(AAFwkTag::APPMGR, "null listener, getInstance or appRunningStatusModule_"); \
        return ERR_INVALID_VALUE;                                                                               \
    }                                                                                                           \
    if (!instance->IsSACall()) {                                                                                \
        TAG_LOGE(AAFwkTag::APPMGR, "callerToken not SA");                                                      \
        return ERR_PERMISSION_DENIED;                                                                           \
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

// NANOSECONDS mean 10^9 nano second
constexpr int64_t NANOSECONDS = 1000000000;
// MICROSECONDS mean 10^6 milli second
constexpr int64_t MICROSECONDS = 1000000;
// Kill process timeout setting
constexpr int KILL_PROCESS_TIMEOUT_MICRO_SECONDS = 1000;
// Kill process delay time setting
constexpr int KILL_PROCESS_DELAYTIME_MICRO_SECONDS = 200;
// delay register focus listener to wms
constexpr int REGISTER_FOCUS_DELAY = 5000;
constexpr int REGISTER_VISIBILITY_DELAY = 5000;
constexpr int REGISTER_PID_VISIBILITY_DELAY = 5000;
// Max render process number limitation for phone device.
constexpr int PHONE_MAX_RENDER_PROCESS_NUM = 40;
constexpr int PROCESS_RESTART_MARGIN_MICRO_SECONDS = 2000;
constexpr int32_t DFX_TASKWORKER_NUM = 2;
constexpr const int32_t API10 = 10;
constexpr const int32_t API15 = 15;
constexpr const int32_t API_VERSION_MOD = 100;
constexpr const int32_t U0_USER_ID = 0;
constexpr const char* CLASS_NAME = "ohos.app.MainThread";
constexpr const char* FUNC_NAME = "main";
constexpr const char* RENDER_PARAM = "invalidparam";
constexpr const char* COLD_START = "coldStart";
constexpr const char* PERF_CMD = "perfCmd";
constexpr const char* ERROR_INFO_ENHANCE = "errorInfoEnhance";
constexpr const char* MULTI_THREAD = "multiThread";
constexpr const char* DEBUG_CMD = "debugCmd";
constexpr const char* ENTER_SANDBOX = "sandboxApp";
constexpr const char* PERMISSION_INTERNET = "ohos.permission.INTERNET";
constexpr const char* PERMISSION_MANAGE_VPN = "ohos.permission.MANAGE_VPN";
constexpr const char* PERMISSION_ACCESS_BUNDLE_DIR = "ohos.permission.ACCESS_BUNDLE_DIR";
constexpr const char* PERMISSION_PROTECT_SCREEN_LOCK_DATA = "ohos.permission.PROTECT_SCREEN_LOCK_DATA";
constexpr const char* PERMISSION_TEMP_JIT_ALLOW = "TEMPJITALLOW";
constexpr const char* TARGET_UID_KEY = "ohos.aafwk.param.targetUid";
constexpr const int32_t KILL_PROCESS_BY_USER_INTERVAL = 20;
constexpr const int32_t KILL_PROCESS_BY_USER_DELAY_BASE = 500;
constexpr const int64_t PRELOAD_FREEZE_TIMEOUT = 11000;

#ifdef WITH_DLP
constexpr const char* DLP_PARAMS_SECURITY_FLAG = "ohos.dlp.params.securityFlag";
#endif // WITH_DLP

constexpr const char* SUPPORT_ISOLATION_MODE = "persist.bms.supportIsolationMode";
constexpr const char* SUPPORT_SERVICE_EXT_MULTI_PROCESS = "component.startup.extension.multiprocess.enable";
constexpr const char* SERVICE_EXT_MULTI_PROCESS_WHITE_LIST = "component.startup.extension.multiprocess.whitelist";
constexpr const char* SCENE_BOARD_BUNDLE_NAME = "com.ohos.sceneboard";
constexpr const char* DEBUG_APP = "debugApp";
constexpr const char* NATIVE_DEBUG = "nativeDebug";
constexpr const char* DEBUG_FROM = "ohos.param.debugFrom";
constexpr const char* SERVICE_EXTENSION = ":ServiceExtension";
constexpr const char* KEEP_ALIVE = ":KeepAlive";
constexpr const char* PARAM_SPECIFIED_PROCESS_FLAG = "ohoSpecifiedProcessFlag";
constexpr const char* TSAN_FLAG_NAME = "tsanEnabled";
constexpr const char* HWASAN_FLAG_NAME = "hwasanEnabled";
constexpr const char* UBSAN_FLAG_NAME = "ubsanEnabled";
constexpr const char* UIEXTENSION_ABILITY_ID = "ability.want.params.uiExtensionAbilityId";
constexpr const char* UIEXTENSION_ROOT_HOST_PID = "ability.want.params.uiExtensionRootHostPid";
constexpr const char* UIEXTENSION_HOST_PID = "ability.want.params.uiExtensionHostPid";
constexpr const char* UIEXTENSION_HOST_UID = "ability.want.params.uiExtensionHostUid";
constexpr const char* UIEXTENSION_HOST_BUNDLENAME = "ability.want.params.uiExtensionHostBundleName";
constexpr const char* UIEXTENSION_BIND_ABILITY_ID = "ability.want.params.uiExtensionBindAbilityId";
constexpr const char* UIEXTENSION_NOTIFY_BIND = "ohos.uiextension.params.notifyProcessBind";
constexpr const char* MEMMGR_PROC_NAME = "memmgrservice";
constexpr const char* ISOLATED_SANDBOX = "isolatedSandbox";
constexpr const char* RENDER_PROCESS_NAME = ":render";
constexpr const char* RENDER_PROCESS_TYPE = "render";
constexpr const char* GPU_PROCESS_NAME = ":gpu";
constexpr const char* GPU_PROCESS_TYPE = "gpu";
constexpr const char* KILL_REASON_USER_REQUEST = "User Request";
const std::string TOKEN_ID = "TOKEN_ID";
const std::string CUSTOM_SANDBOX_KEY = "com.huawei.service.sandboxmode.custom";
const std::string DEVELOPERMODE_STATE = "const.security.developermode.state";
const int32_t SIGNAL_KILL = 9;
constexpr int32_t USER_SCALE = 200000;
#define ENUM_TO_STRING(s) #s
#define APP_ACCESS_BUNDLE_DIR 0x20
#define APP_OVERLAY_FLAG 0x100

constexpr int32_t MAX_RESTART_COUNT = 3;
constexpr int32_t RESTART_INTERVAL_TIME = 120000;
constexpr int32_t FIRST_FRAME_NOTIFY_TASK_DELAY = 5; //ms

constexpr ErrCode APPMGR_ERR_OFFSET = ErrCodeOffset(SUBSYS_APPEXECFWK, 0x01);
 // Error code for already exist render.
constexpr ErrCode ERR_ALREADY_EXIST_RENDER = APPMGR_ERR_OFFSET + 100;
 // Error code for reaching render process number limitation.
constexpr ErrCode ERR_REACHING_MAXIMUM_RENDER_PROCESS_LIMITATION = APPMGR_ERR_OFFSET + 101;
constexpr const char* EVENT_KEY_UID = "UID";
constexpr const char* EVENT_KEY_PID = "PID";
constexpr const char* EVENT_KEY_PACKAGE_NAME = "PACKAGE_NAME";
constexpr const char* EVENT_KEY_PROCESS_NAME = "PROCESS_NAME";
constexpr const char* EVENT_KEY_MESSAGE = "MSG";
constexpr const char* EVENT_KEY_FOREGROUND = "FOREGROUND";

// Developer mode param
constexpr const char* DEVELOPER_MODE_STATE = "const.security.developermode.state";
constexpr const char* PRODUCT_ASSERT_FAULT_DIALOG_ENABLED = "persisit.sys.abilityms.support_assert_fault_dialog";

// Msg length is less than 48 characters
constexpr const char* EVENT_MESSAGE_TERMINATE_ABILITY_TIMEOUT = "Terminate Ability TimeOut!";
constexpr const char* EVENT_MESSAGE_TERMINATE_APPLICATION_TIMEOUT = "Terminate Application TimeOut!";
constexpr const char* EVENT_MESSAGE_ADD_ABILITY_STAGE_INFO_TIMEOUT = "Add Ability Stage TimeOut!";
constexpr const char* EVENT_MESSAGE_START_SPECIFIED_PROCESS_TIMEOUT = "Start Specified Process Timeout!";
constexpr const char* EVENT_MESSAGE_START_SPECIFIED_ABILITY_TIMEOUT = "Start Specified Ability TimeOut!";
constexpr const char* EVENT_MESSAGE_START_PROCESS_SPECIFIED_ABILITY_TIMEOUT =
    "Start Process Specified Ability TimeOut!";
constexpr const char* EVENT_MESSAGE_DEFAULT = "AppMgrServiceInner HandleTimeOut!";
constexpr const char* SUPPORT_CALL_NOTIFY_MEMORY_CHANGED =
    "persist.sys.abilityms.support_call_notify_memory_changed";

constexpr const char* SYSTEM_BASIC = "system_basic";
constexpr const char* SYSTEM_CORE = "system_core";
constexpr const char* ABILITY_OWNER_USERID = "AbilityMS_Owner_UserId";
constexpr const char* PROCESS_EXIT_EVENT_TASK = "Send Process Exit Event Task";
constexpr const char* KILL_PROCESS_REASON_PREFIX = "Kill Reason:";
constexpr const char* PRELOAD_APPLIATION_TASK = "PreloadApplicactionTask";
constexpr const char* KEY_WATERMARK_BUSINESS_NAME = "com.ohos.param.watermarkBusinessName";
constexpr const char* KEY_IS_WATERMARK_ENABLED = "com.ohos.param.isWatermarkEnabled";
constexpr const char* KILL_SUB_PROCESS_REASON_PREFIX = "Kill SubProcess Reason:";

constexpr const char* PROC_SELF_TASK_PATH = "/proc/self/task/";

constexpr int32_t ROOT_UID = 0;
constexpr int32_t FOUNDATION_UID = 5523;
constexpr int32_t QUICKFIX_UID = 5524;
constexpr int32_t DEFAULT_USER_ID = 0;
constexpr int32_t CURRENT_USER_ID = -1;
constexpr int32_t RESOURCE_MANAGER_UID = 1096;

constexpr int32_t BLUETOOTH_GROUPID = 1002;

#ifdef APP_MGR_SERVICE_APPMS
constexpr int32_t NETSYS_SOCKET_GROUPID = 1097;
#endif

constexpr int32_t SHADER_CACHE_GROUPID = 3099;
constexpr int32_t DEFAULT_INVAL_VALUE = -1;
constexpr int32_t NO_ABILITY_RECORD_ID = -1;
constexpr int32_t EXIT_REASON_UNKNOWN = 0;
constexpr int32_t PROCESS_START_FAILED_SUB_REASON_UNKNOWN = 0;

constexpr int32_t MAX_SPECIFIED_PROCESS_NAME_LENGTH = 255;

constexpr int32_t NWEB_PRELOAD_DELAY = 3000;

constexpr const char* APP_INSTANCE_KEY_0 = "app_instance_0";

// Max child process number limitation for pc device.
constexpr int32_t PC_MAX_CHILD_PROCESS_NUM = 50;
constexpr int32_t USER100 = 100;
int32_t GetUserIdByUid(int32_t uid)
{
    return uid / BASE_USER_RANGE;
}

bool IsCjAbility(const std::string& info)
{
    // in cj application, the srcEntry format should be packageName.AbilityClassName.
    std::string pattern = "^([a-zA-Z0-9_]+\\.)+[a-zA-Z0-9_]+$";
    return std::regex_match(info, std::regex(pattern));
}

bool IsCjApplication(const BundleInfo &bundleInfo)
{
    bool findEntryHapModuleInfo = false;
    AppExecFwk::HapModuleInfo entryHapModuleInfo;
    if (!bundleInfo.hapModuleInfos.empty()) {
        for (auto hapModuleInfo : bundleInfo.hapModuleInfos) {
            if (hapModuleInfo.moduleType == AppExecFwk::ModuleType::ENTRY) {
                findEntryHapModuleInfo = true;
                entryHapModuleInfo = hapModuleInfo;
                break;
            }
        }
        if (!findEntryHapModuleInfo) {
            TAG_LOGW(AAFwkTag::APPMGR, "not find hap module");
            entryHapModuleInfo = bundleInfo.hapModuleInfos.back();
        }
        if (entryHapModuleInfo.srcEntrance.length() > 0) {
            return IsCjAbility(entryHapModuleInfo.srcEntrance);
        }
    }
    return false;
}

std::string GetEventName(int32_t eventId)
{
    switch (eventId) {
        case AMSEventHandler::TERMINATE_ABILITY_TIMEOUT_MSG:
        case AMSEventHandler::TERMINATE_APPLICATION_TIMEOUT_MSG:
        case AMSEventHandler::ADD_ABILITY_STAGE_INFO_TIMEOUT_MSG:
        case AMSEventHandler::START_PROCESS_SPECIFIED_ABILITY_TIMEOUT_MSG:
        case AMSEventHandler::START_SPECIFIED_ABILITY_TIMEOUT_MSG:
        case AMSEventHandler::START_SPECIFIED_PROCESS_TIMEOUT_MSG:
            return AppExecFwk::AppFreezeType::LIFECYCLE_TIMEOUT;
        default:
            return AppExecFwk::AppFreezeType::LIFECYCLE_HALF_TIMEOUT;
    }
}

std::pair<int, std::string> GetEventTypeAndMsg(int32_t eventId)
{
    std::pair<int, std::string> result{AppExecFwk::AppfreezeManager::TypeAttribute::NORMAL_TIMEOUT, ""};
    switch (eventId) {
        case AMSEventHandler::TERMINATE_ABILITY_TIMEOUT_MSG:
        case AMSEventHandler::TERMINATE_ABILITY_HALF_TIMEOUT_MSG:
            result.second = EVENT_MESSAGE_TERMINATE_ABILITY_TIMEOUT;
            break;
        case AMSEventHandler::TERMINATE_APPLICATION_TIMEOUT_MSG:
        case AMSEventHandler::TERMINATE_APPLICATION_HALF_TIMEOUT_MSG:
            result.second = EVENT_MESSAGE_TERMINATE_APPLICATION_TIMEOUT;
            break;
        case AMSEventHandler::ADD_ABILITY_STAGE_INFO_TIMEOUT_MSG:
        case AMSEventHandler::ADD_ABILITY_STAGE_INFO_HALF_TIMEOUT_MSG:
            result.second = EVENT_MESSAGE_ADD_ABILITY_STAGE_INFO_TIMEOUT;
            result.first = AppExecFwk::AppfreezeManager::TypeAttribute::CRITICAL_TIMEOUT;
            break;
        case AMSEventHandler::START_PROCESS_SPECIFIED_ABILITY_TIMEOUT_MSG:
        case AMSEventHandler::START_PROCESS_SPECIFIED_ABILITY_HALF_TIMEOUT_MSG:
            result.second = EVENT_MESSAGE_START_PROCESS_SPECIFIED_ABILITY_TIMEOUT;
            result.first = AppExecFwk::AppfreezeManager::TypeAttribute::CRITICAL_TIMEOUT;
            break;
        case AMSEventHandler::START_SPECIFIED_ABILITY_TIMEOUT_MSG:
        case AMSEventHandler::START_SPECIFIED_ABILITY_HALF_TIMEOUT_MSG:
            result.second = EVENT_MESSAGE_START_SPECIFIED_ABILITY_TIMEOUT;
            result.first = AppExecFwk::AppfreezeManager::TypeAttribute::CRITICAL_TIMEOUT;
            break;
        case AMSEventHandler::START_SPECIFIED_PROCESS_TIMEOUT_MSG:
        case AMSEventHandler::START_SPECIFIED_PROCESS_HALF_TIMEOUT_MSG:
            result.second = EVENT_MESSAGE_START_SPECIFIED_PROCESS_TIMEOUT;
            result.first = AppExecFwk::AppfreezeManager::TypeAttribute::CRITICAL_TIMEOUT;
            break;
        default:
            result.second = EVENT_MESSAGE_DEFAULT;
            break;
    }
    return result;
}
}  // namespace

using OHOS::AppExecFwk::Constants::PERMISSION_GRANTED;
using OHOS::AppExecFwk::Constants::PERMISSION_NOT_GRANTED;

AppMgrServiceInner::AppMgrServiceInner()
    : remoteClientManager_(std::make_shared<RemoteClientManager>()),
      appRunningManager_(std::make_shared<AppRunningManager>()),
      appDebugManager_(std::make_shared<AppDebugManager>()),
      appRunningStatusModule_(std::make_shared<AbilityRuntime::AppRunningStatusModule>()),
      securityModeManager_(std::make_shared<AdvancedSecurityModeManager>()),
      appPreloader_(std::make_shared<AppPreloader>(remoteClientManager_)),
      multiUserConfigurationMgr_(std::make_shared<MultiUserConfigurationMgr>())
{}

void AppMgrServiceInner::Init()
{
    InitGlobalConfiguration();
    AddWatchParameter();
    supportIsolationMode_ = OHOS::system::GetParameter(SUPPORT_ISOLATION_MODE, "false");
    supportServiceExtMultiProcess_ = OHOS::system::GetParameter(SUPPORT_SERVICE_EXT_MULTI_PROCESS, "false");
    ParseServiceExtMultiProcessWhiteList();
    DelayedSingleton<AppStateObserverManager>::GetInstance()->Init();
    DelayedSingleton<RenderStateObserverManager>::GetInstance()->Init();
    dfxTaskHandler_ = AAFwk::TaskHandlerWrap::CreateConcurrentQueueHandler(
        "dfx_freeze_task_queue", DFX_TASKWORKER_NUM, AAFwk::TaskQoS::USER_INITIATED);
    dfxTaskHandler_->SetPrintTaskLog(true);
    otherTaskHandler_ = AAFwk::TaskHandlerWrap::CreateQueueHandler("other_app_mgr_task_queue");
    otherTaskHandler_->SetPrintTaskLog(true);
    willKillPidsNum_ = 0;
    delayKillTaskHandler_ = AAFwk::TaskHandlerWrap::CreateQueueHandler("delay_kill_task_queue");
    if (securityModeManager_) {
        securityModeManager_->Init();
    }
    otherTaskHandler_->SubmitTask([pThis = shared_from_this()]() {
        pThis->nwebPreloadSet_ = AAFwk::ResSchedUtil::GetInstance().GetNWebPreloadSet();
        }, NWEB_PRELOAD_DELAY);
}

AppMgrServiceInner::~AppMgrServiceInner()
{}

void AppMgrServiceInner::StartSpecifiedProcess(const AAFwk::Want &want, const AppExecFwk::AbilityInfo &abilityInfo,
    int32_t requestId)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::APPMGR, "call.");
    BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    auto appInfo = std::make_shared<ApplicationInfo>(abilityInfo.applicationInfo);

    int32_t appIndex = 0;
    (void)AbilityRuntime::StartupUtil::GetAppIndex(want, appIndex);
    if (!GetBundleAndHapInfo(abilityInfo, appInfo, bundleInfo, hapModuleInfo, appIndex)) {
        return;
    }
    if (UserRecordManager::GetInstance().IsLogoutUser(GetUserIdByUid(appInfo->uid))) {
        TAG_LOGE(AAFwkTag::APPMGR, "disable start process in logout user");
        return;
    }

    std::string processName;
    auto abilityInfoPtr = std::make_shared<AbilityInfo>(abilityInfo);
    MakeProcessName(abilityInfoPtr, appInfo, hapModuleInfo, appIndex, "", processName, false);
    TAG_LOGD(AAFwkTag::APPMGR, "processName = %{public}s", processName.c_str());
    auto instanceKey = want.GetStringParam(Want::APP_INSTANCE_KEY);
    auto customProcessFlag = abilityInfo.process;
    auto mainAppRecord = appRunningManager_->CheckAppRunningRecordIsExist(appInfo->name, processName, appInfo->uid,
        bundleInfo, "", nullptr, instanceKey, customProcessFlag);
    if (mainAppRecord != nullptr) {
        TAG_LOGI(AAFwkTag::APPMGR, "main process exists.");
        mainAppRecord->SetScheduleNewProcessRequestState(requestId, want, hapModuleInfo.moduleName);
        auto moduleRecord = mainAppRecord->GetModuleRecordByModuleName(appInfo->bundleName, hapModuleInfo.moduleName);
        if (!moduleRecord) {
            TAG_LOGI(AAFwkTag::APPMGR, "module record is nullptr, add modules");
            std::vector<HapModuleInfo> hapModules = { hapModuleInfo };
            mainAppRecord->AddModules(appInfo, hapModules);
            if (mainAppRecord->GetApplicationClient() != nullptr) {
                mainAppRecord->AddAbilityStageBySpecifiedProcess(appInfo->bundleName);
            }
            return;
        }
        TAG_LOGD(AAFwkTag::APPMGR, "schedule new process request.");
        mainAppRecord->ScheduleNewProcessRequest(want, hapModuleInfo.moduleName);
        return;
    }
    TAG_LOGI(AAFwkTag::APPMGR, "main process do not exists.");
    if (startSpecifiedAbilityResponse_) {
        startSpecifiedAbilityResponse_->OnNewProcessRequestResponse(want, "", requestId);
    }
}

int32_t AppMgrServiceInner::PreloadApplication(const std::string &bundleName, int32_t userId,
    AppExecFwk::PreloadMode preloadMode, int32_t appIndex)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    TAG_LOGI(AAFwkTag::APPMGR,
        "bundleName:%{public}s, userId:%{public}d, preloadMode:%{public}d, appIndex:%{public}d",
        bundleName.c_str(), userId, preloadMode, appIndex);

    CHECK_CALLER_IS_SYSTEM_APP;
    auto isPerm = AAFwk::PermissionVerification::GetInstance()->VerifyPreloadApplicationPermission();
    if (!isPerm) {
        TAG_LOGE(AAFwkTag::APPMGR, "permission verify fail");
        return ERR_PERMISSION_DENIED;
    }
    if (!appPreloader_) {
        TAG_LOGE(AAFwkTag::APPMGR, "null appPreloader");
        return ERR_INVALID_VALUE;
    }
    if (userId == CURRENT_USER_ID) {
        userId = currentUserId_;
    }
    if (UserRecordManager::GetInstance().IsLogoutUser(userId)) {
        TAG_LOGE(AAFwkTag::APPMGR, "disable start process in logout user");
        return ERR_INVALID_OPERATION;
    }
    auto allowPreload = appPreloader_->PreCheck(bundleName, preloadMode);
    if (!allowPreload) {
        TAG_LOGI(AAFwkTag::APPMGR, "bundleName: %{public}s preload preCheck:unallow", bundleName.c_str());
        return AAFwk::ERR_NOT_ALLOW_PRELOAD_BY_RSS;
    }

    PreloadRequest request;
    auto ret = appPreloader_->GeneratePreloadRequest(bundleName, userId, appIndex, request);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "generatePreloadRequest fail");
        return ret;
    }
    SetPreloadDebugApp(request.want, request.appInfo);
    request.preloadMode = preloadMode;
    auto task = [innerServiceWeak = weak_from_this(), request] () {
        auto innerService = innerServiceWeak.lock();
        CHECK_POINTER_AND_RETURN_LOG(innerService, "get appMgrServiceInner fail");
        innerService->HandlePreloadApplication(request);
    };
    if (!taskHandler_) {
        TAG_LOGE(AAFwkTag::APPMGR, "null taskHandler_");
        return ERR_INVALID_VALUE;
    }
    TAG_LOGI(AAFwkTag::APPMGR, "submit task, bundleName:%{public}s, userId:%{public}d",
        bundleName.c_str(), userId);
    taskHandler_->SubmitTask(task, PRELOAD_APPLIATION_TASK);
    return ERR_OK;
}

void AppMgrServiceInner::SetPreloadDebugApp(std::shared_ptr<AAFwk::Want> want, std::shared_ptr<ApplicationInfo> appInfo)
{
    CHECK_POINTER_AND_RETURN_LOG(want, "SetPreloadDebugApp want null");
    CHECK_POINTER_AND_RETURN_LOG(appInfo, "SetPreloadDebugApp appInfo null");
    if (!appInfo->debug ||
        appInfo->appProvisionType != AppExecFwk::Constants::APP_PROVISION_TYPE_DEBUG ||
        !system::GetBoolParameter(DEVELOPER_MODE_STATE, false) ||
        !IsWaitingDebugAppInner(appInfo->bundleName)) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "Not meeting the set debugging conditions.");
        return;
    }
    want->SetParam(DEBUG_APP, true);
}

void AppMgrServiceInner::HandlePreloadApplication(const PreloadRequest &request)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    auto abilityInfo = request.abilityInfo;
    if (!abilityInfo) {
        TAG_LOGE(AAFwkTag::APPMGR, "null request");
        return;
    }
    auto bundleInfo = request.bundleInfo;
    TAG_LOGI(AAFwkTag::APPMGR, "bundleName:%{public}s, abilityName:%{public}s, \
    appIndex:%{public}d", bundleInfo.name.c_str(), abilityInfo->name.c_str(), request.appIndex);

    auto appInfo = request.appInfo;
    auto hapModuleInfo = request.hapModuleInfo;

    auto want = request.want;
    std::string specifiedProcessFlag = GetSpecifiedProcessFlag(abilityInfo, want);

    std::string processName;
    MakeProcessName(abilityInfo, appInfo, hapModuleInfo, request.appIndex, specifiedProcessFlag, processName, false);
    TAG_LOGD(AAFwkTag::APPMGR, "HandlePreloadApplication processName = %{public}s", processName.c_str());

    if (!appRunningManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "handlePreloadApplication fail");
        return;
    }
    std::shared_ptr<AppRunningRecord> appRecord = appRunningManager_->CheckAppRunningRecordIsExist(appInfo->name,
        processName, appInfo->uid, bundleInfo, specifiedProcessFlag);
    if (appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRecord already exists");
        return;
    }

    AAFwk::ResSchedUtil::GetInstance().ReportPreloadApplicationToRSS(abilityInfo,
        static_cast<int32_t>(request.preloadMode));
    bool appExistFlag = appRunningManager_->IsAppExist(appInfo->accessTokenId);
    bool appMultiUserExistFlag = appRunningManager_->CheckAppRunningRecordIsExistByUid(bundleInfo.uid);
    if (!appMultiUserExistFlag) {
        NotifyAppRunningStatusEvent(
            bundleInfo.name, appInfo->uid, AbilityRuntime::RunningStatus::APP_RUNNING_START);
    }
    auto loadParam = std::make_shared<AbilityRuntime::LoadParam>();
    appRecord = CreateAppRunningRecord(loadParam, appInfo, abilityInfo, processName, bundleInfo, hapModuleInfo, want);
    if (appRecord != nullptr) {
        appRecord->SetPreloadState(PreloadState::PRELOADING);
        appRecord->SetPreloadMode(request.preloadMode);
        appRecord->SetNeedPreloadModule(request.preloadMode == AppExecFwk::PreloadMode::PRELOAD_MODULE);
        appRecord->SetNeedLimitPrio(request.preloadMode != PreloadMode::PRESS_DOWN);
        LoadAbilityNoAppRecord(appRecord, false, appInfo, abilityInfo, processName, specifiedProcessFlag, bundleInfo,
            hapModuleInfo, want, appExistFlag, true, request.preloadMode);
        appRecord->SetNeedLimitPrio(false);
        if (request.preloadMode == AppExecFwk::PreloadMode::PRELOAD_MODULE) {
            reportpreLoadTask(appRecord);
        }
    }
}

void AppMgrServiceInner::reportpreLoadTask(const std::shared_ptr<AppRunningRecord> appRecord)
{
    auto reportLoadTask = [appRecord]() {
        auto priorityObj = appRecord->GetPriorityObject();
        if (priorityObj) {
            AAFwk::ResSchedUtil::GetInstance().ReportLoadingEventToRss(AAFwk::LoadingStage::PRELOAD_BEGIN,
                priorityObj->GetPid(), appRecord->GetUid(), PRELOAD_FREEZE_TIMEOUT, 0);
        }
    };
    if (taskHandler_) {
        taskHandler_->SubmitTask(reportLoadTask, "reportpreLoadTask");
    }
}

int32_t AppMgrServiceInner::MakeKiaProcess(std::shared_ptr<AAFwk::Want> want, bool &isKia,
    std::string &watermarkBusinessName, bool &isWatermarkEnabled,
    bool &isFileUri, std::string &processName)
{
    if (!AAFwk::AppUtils::GetInstance().IsStartOptionsWithAnimation()) {
        return ERR_OK;
    }
    if (want == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "want is nullptr");
        return ERR_INVALID_VALUE;
    }
#ifdef INCLUDE_ZURI
    isFileUri = !want->GetUriString().empty() && want->GetUri().GetScheme() == "file";
#endif
    if (isFileUri && kiaInterceptor_ != nullptr) {
        auto resultCode = kiaInterceptor_->OnIntercept(*want);
        watermarkBusinessName = want->GetStringParam(KEY_WATERMARK_BUSINESS_NAME);
        isWatermarkEnabled = want->GetBoolParam(KEY_IS_WATERMARK_ENABLED, false);
        TAG_LOGI(AAFwkTag::APPMGR, "After calling kiaInterceptor_->OnIntercept,"
            "resultCode=%{public}d,watermarkBusinessName=%{private}s,isWatermarkEnabled=%{private}d",
            resultCode, watermarkBusinessName.c_str(),
            static_cast<int>(isWatermarkEnabled));
        isKia = (resultCode == ERR_OK && !watermarkBusinessName.empty() && isWatermarkEnabled);
        if (isKia) {
            processName += "_KIA";
        }
    }
    return ERR_OK;
}

int32_t AppMgrServiceInner::ProcessKia(bool isKia, std::shared_ptr<AppRunningRecord> appRecord,
    const std::string& watermarkBusinessName, bool isWatermarkEnabled)
{
    if (!AAFwk::AppUtils::GetInstance().IsStartOptionsWithAnimation() || !isKia) {
        return ERR_OK;
    }
    if (appRecord == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRecord is nullptr");
        return ERR_INVALID_VALUE;
    }
#ifdef SUPPORT_SCREEN
    TAG_LOGI(AAFwkTag::APPMGR, "Openning KIA file, start setting watermark");
    int32_t resultCode = static_cast<int32_t>(WindowManager::GetInstance().SetProcessWatermark(
        appRecord->GetPid(), watermarkBusinessName, isWatermarkEnabled));
    if (resultCode != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "setting watermark fails with result code:%{public}d", resultCode);
        return resultCode;
    }
    TAG_LOGI(AAFwkTag::APPMGR, "setting watermark succeeds, start setting snapshot skip");
    resultCode = static_cast<int32_t>(WindowManager::GetInstance().SkipSnapshotForAppProcess(
        appRecord->GetPid(), isWatermarkEnabled));
    if (resultCode != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "setting snapshot skip fails with result code:%{public}d", resultCode);
        return resultCode;
    }
    TAG_LOGI(AAFwkTag::APPMGR, "setting snapshot skip succeeds");
#endif // SUPPORT_SCREEN
    return ERR_OK;
}

void AppMgrServiceInner::LoadAbility(std::shared_ptr<AbilityInfo> abilityInfo, std::shared_ptr<ApplicationInfo> appInfo,
    std::shared_ptr<AAFwk::Want> want, std::shared_ptr<AbilityRuntime::LoadParam> loadParam)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    if (loadParam == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null loadParam");
        return;
    }
    if (!CheckLoadAbilityConditions(loadParam->token, abilityInfo, appInfo)) {
        TAG_LOGE(AAFwkTag::APPMGR, "checkLoadAbilityConditions fail");
        return;
    }
    if (abilityInfo->type == AbilityType::PAGE) {
        std::string entry = "AppMgrServiceInner::LoadAbility";
        AbilityRuntime::FreezeUtil::GetInstance().AddLifecycleEvent(loadParam->token, entry);
    }

    if (!appRunningManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "null appRunningManager_");
        return;
    }

    if (UserRecordManager::GetInstance().IsLogoutUser(GetUserIdByUid(appInfo->uid))) {
        TAG_LOGE(AAFwkTag::APPMGR, "disable start process in logout user");
        return;
    }

    BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    int32_t appIndex = 0;
    std::string callerKey;
    if (want != nullptr) {
        (void)AbilityRuntime::StartupUtil::GetAppIndex(*want, appIndex);
        callerKey = want->GetStringParam(Want::PARAMS_REAL_CALLER_KEY);
        want->RemoveParam(Want::PARAMS_REAL_CALLER_KEY);
    }
    if (!GetBundleAndHapInfo(*abilityInfo, appInfo, bundleInfo, hapModuleInfo, appIndex)) {
        TAG_LOGE(AAFwkTag::APPMGR, "getBundleAndHapInfo fail");
        return;
    }
    // for isolation process
    std::string specifiedProcessFlag = GetSpecifiedProcessFlag(abilityInfo, want);
    std::string processName;
    MakeProcessName(abilityInfo, appInfo, hapModuleInfo, appIndex, specifiedProcessFlag,
        processName, loadParam->isCallerSetProcess);
    TAG_LOGI(AAFwkTag::APPMGR, "%{public}s name:%{public}s-%{public}s processName = %{public}s",
        __func__, abilityInfo->bundleName.c_str(), abilityInfo->name.c_str(), processName.c_str());

    bool isKia = false;
    std::string watermarkBusinessName;
    bool isWatermarkEnabled = false;
    bool isFileUri = false;
    if (MakeKiaProcess(want, isKia, watermarkBusinessName, isWatermarkEnabled, isFileUri, processName) != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "MakeKiaProcess failed");
        return;
    }

    std::shared_ptr<AppRunningRecord> appRecord;
    bool isProcCache = false;
    std::string customProcessFlag = loadParam->customProcessFlag;
    appRecord = appRunningManager_->CheckAppRunningRecordIsExist(appInfo->name,
        processName, appInfo->uid, bundleInfo, specifiedProcessFlag, &isProcCache, loadParam->instanceKey,
        customProcessFlag);
    if (appRecord && appRecord->IsCaching()) {
        auto priorityObj = appRecord->GetPriorityObject();
        if (priorityObj) {
            int32_t pid = priorityObj->GetPid();
            TAG_LOGI(AAFwkTag::APPMGR, "process %{public}s is caching start ability set to blocked pid %{public}d",
                processName.c_str(), pid);
        }
        appRecord->SetProcessCacheBlocked(true);
        appRecord = nullptr;
    }
    if (appRecord && abilityInfo->type == AppExecFwk::AbilityType::PAGE) {
        NotifyMemMgrPriorityChanged(appRecord);
    }

    if (!appRecord) {
        TAG_LOGD(AAFwkTag::APPMGR, "appRecord null");
        if (KillingProcessManager::GetInstance().IsCallerKilling(callerKey)) {
            TAG_LOGE(AAFwkTag::APPMGR, "caller is killing");
            NotifyLoadAbilityFailed(loadParam->token);
            return;
        }
        bool appExistFlag = appRunningManager_->IsAppExist(appInfo->accessTokenId);
        bool appMultiUserExistFlag = appRunningManager_->CheckAppRunningRecordIsExistByUid(bundleInfo.uid);
        if (!appMultiUserExistFlag) {
            NotifyAppRunningStatusEvent(
                bundleInfo.name, appInfo->uid, AbilityRuntime::RunningStatus::APP_RUNNING_START);
        }
        appRecord = CreateAppRunningRecord(loadParam, appInfo, abilityInfo,
            processName, bundleInfo, hapModuleInfo, want, isKia);
        LoadAbilityNoAppRecord(appRecord, loadParam->isShellCall, appInfo, abilityInfo, processName,
            specifiedProcessFlag, bundleInfo, hapModuleInfo, want, appExistFlag, false,
            AppExecFwk::PreloadMode::PRESS_DOWN, loadParam->token, customProcessFlag);
        if (ProcessKia(isKia, appRecord, watermarkBusinessName, isWatermarkEnabled) != ERR_OK) {
            TAG_LOGE(AAFwkTag::APPMGR, "ProcessKia failed");
            return;
        }
    } else {
        TAG_LOGI(AAFwkTag::APPMGR, "have apprecord");
        appRunningManager_->UpdateConfigurationDelayed(appRecord);
        if (!isProcCache) {
            SendAppStartupTypeEvent(appRecord, abilityInfo, AppStartType::MULTI_INSTANCE, AppStartReason::NONE);
            SendPreloadAppStartupTypeEvent(appRecord, abilityInfo);
        } else {
            SendAppStartupTypeEvent(appRecord, abilityInfo, AppStartType::PROCESS_CACHE_LAUNCH, AppStartReason::NONE);
            SendAppStartupTypeEvent(appRecord, abilityInfo, AppStartType::WARM, AppStartReason::SUGGEST_CACHE);
        }
        if (appRecord->IsPreloaded()) {
            appRecord->SetPreloadState(PreloadState::NONE);
        }
        int32_t requestProcCode = (want == nullptr) ? 0 : want->GetIntParam(Want::PARAM_RESV_REQUEST_PROC_CODE, 0);
        if (requestProcCode != 0 && appRecord->GetRequestProcCode() == 0) {
            appRecord->SetRequestProcCode(requestProcCode);
            DelayedSingleton<AppStateObserverManager>::GetInstance()->OnProcessReused(appRecord);
        }
        StartAbility(loadParam->token, loadParam->preToken, abilityInfo, appRecord, hapModuleInfo, want,
            loadParam->abilityRecordId);
        if (AAFwk::UIExtensionUtils::IsUIExtension(abilityInfo->extensionAbilityType)) {
            AddUIExtensionBindItem(want, appRecord, loadParam->token);
            AddUIExtensionLauncherItem(want, appRecord, loadParam->token);
        }
    }

    if (AAFwk::UIExtensionUtils::IsUIExtension(abilityInfo->extensionAbilityType) &&
        appRecord != nullptr && want != nullptr) {
        auto abilityRunningRecord = appRecord->GetAbilityRunningRecordByToken(loadParam->token);
        auto uiExtensionAbilityId = want->GetIntParam(UIEXTENSION_ABILITY_ID, -1);
        auto uiExtensionBindAbilityId = want->GetIntParam(UIEXTENSION_BIND_ABILITY_ID, -1);
        if (abilityRunningRecord != nullptr) {
            abilityRunningRecord->SetUIExtensionAbilityId(uiExtensionAbilityId);
            abilityRunningRecord->SetUIExtensionBindAbilityId(uiExtensionBindAbilityId);
        }
        want->RemoveParam(UIEXTENSION_BIND_ABILITY_ID);
    }
    AfterLoadAbility(appRecord, abilityInfo, loadParam);
}

void AppMgrServiceInner::AfterLoadAbility(std::shared_ptr<AppRunningRecord> appRecord,
    std::shared_ptr<AbilityInfo> abilityInfo, std::shared_ptr<AbilityRuntime::LoadParam> loadParam)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    if (!appRecord || !abilityInfo || !loadParam) {
        return;
    }
    if (abilityInfo->type == AppExecFwk::AbilityType::PAGE) {
        appRecord->SetUIAbilityLaunched(true);
    }
    PerfProfile::GetInstance().SetAbilityLoadEndTime(GetTickCount());
    PerfProfile::GetInstance().Dump();
    PerfProfile::GetInstance().Reset();

    auto reportLoadTask = [appRecord, abilityRecordId = loadParam->abilityRecordId]() {
        auto priorityObj = appRecord->GetPriorityObject();
        if (priorityObj) {
            auto timeOut = AbilityRuntime::GlobalConstant::GetLoadTimeOutBase() *
                AAFwk::AppUtils::GetInstance().GetTimeoutUnitTimeRatio();
            if (appRecord->GetExtensionType() == ExtensionAbilityType::SERVICE) {
                timeOut = AbilityRuntime::GlobalConstant::GetLoadAndInactiveTimeout() *
                    AAFwk::AppUtils::GetInstance().GetTimeoutUnitTimeRatio();
            }

            AAFwk::ResSchedUtil::GetInstance().ReportLoadingEventToRss(AAFwk::LoadingStage::LOAD_BEGIN,
                priorityObj->GetPid(), appRecord->GetUid(), timeOut, static_cast<int64_t>(abilityRecordId));
        }
    };
    if (taskHandler_) {
        taskHandler_->SubmitTask(reportLoadTask, "reportLoadTask");
    }
    //UIExtension notifies Extension & Ability state changes
    if (AAFwk::UIExtensionUtils::IsUIExtension(appRecord->GetExtensionType())) {
        UpdateExtensionState(loadParam->token, ExtensionState::EXTENSION_STATE_CREATE);
    }
    appRecord->UpdateAbilityState(loadParam->token, AbilityState::ABILITY_STATE_CREATE);
}

void AppMgrServiceInner::AddUIExtensionLauncherItem(std::shared_ptr<AAFwk::Want> want,
    std::shared_ptr<AppRunningRecord> appRecord, sptr<IRemoteObject> token)
{
    if (want == nullptr || appRecord == nullptr || token == nullptr || appRunningManager_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "invalid input params");
        return;
    }

    auto uiExtensionAbilityId = want->GetIntParam(UIEXTENSION_ABILITY_ID, -1);
    auto hostPid = want->GetIntParam(UIEXTENSION_ROOT_HOST_PID, -1);
    pid_t providerPid = -1;
    if (appRecord->GetPriorityObject() != nullptr) {
        providerPid = appRecord->GetPid();
    }
    if (uiExtensionAbilityId == -1 || hostPid == -1 || providerPid == -1) {
        TAG_LOGE(AAFwkTag::APPMGR, "invalid want params");
        return;
    }

    TAG_LOGI(AAFwkTag::APPMGR, "uiExtensionAbilityId: %{public}d, hostPid: %{public}d, "
        "providerPid: %{public}d", uiExtensionAbilityId, hostPid, providerPid);
    appRunningManager_->AddUIExtensionLauncherItem(uiExtensionAbilityId, hostPid, providerPid);

    want->RemoveParam(UIEXTENSION_ABILITY_ID);
    want->RemoveParam(UIEXTENSION_ROOT_HOST_PID);
}

void AppMgrServiceInner::RemoveUIExtensionLauncherItem(std::shared_ptr<AppRunningRecord> appRecord,
    sptr<IRemoteObject> token)
{
    if (appRecord == nullptr || token == nullptr || appRunningManager_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "invalid input params");
        return;
    }

    auto abilityRunningRecord = appRecord->GetAbilityRunningRecordByToken(token);
    if (abilityRunningRecord == nullptr) {
        TAG_LOGW(AAFwkTag::APPMGR, "invalid ability");
        return;
    }

    auto abilityInfo = abilityRunningRecord->GetAbilityInfo();
    if (abilityInfo == nullptr) {
        TAG_LOGW(AAFwkTag::APPMGR, "invalid ability");
        return;
    }

    if (!AAFwk::UIExtensionUtils::IsUIExtension(abilityInfo->extensionAbilityType)) {
        return;
    }

    auto uiExtensionAbilityId = abilityRunningRecord->GetUIExtensionAbilityId();
    appRunningManager_->RemoveUIExtensionLauncherItemById(uiExtensionAbilityId);
}

bool AppMgrServiceInner::CheckLoadAbilityConditions(const sptr<IRemoteObject> &token,
    const std::shared_ptr<AbilityInfo> &abilityInfo, const std::shared_ptr<ApplicationInfo> &appInfo)
{
    if (!token || !abilityInfo || !appInfo) {
        TAG_LOGE(AAFwkTag::APPMGR, "param error");
        return false;
    }
    if (abilityInfo->name.empty() || appInfo->name.empty()) {
        TAG_LOGE(AAFwkTag::APPMGR, "error abilityInfo or appInfo");
        return false;
    }
    if (abilityInfo->applicationName != appInfo->name) {
        TAG_LOGE(AAFwkTag::APPMGR, "abilityInfo and appInfo have diff appName");
        return false;
    }

    return true;
}

void AppMgrServiceInner::MakeServiceExtProcessName(const std::shared_ptr<AbilityInfo> &abilityInfo,
    const std::shared_ptr<ApplicationInfo> &appInfo, std::string &processName) const
{
    if (abilityInfo == nullptr || appInfo == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null ability or app info");
        return;
    }

    if (supportServiceExtMultiProcess_.compare("true") != 0) {
        return;
    }

    if (processName == appInfo->bundleName &&
        abilityInfo->extensionAbilityType == ExtensionAbilityType::SERVICE) {
        auto iter = std::find(
            serviceExtensionWhiteList_.begin(), serviceExtensionWhiteList_.end(), processName);
        if (iter != serviceExtensionWhiteList_.end()) {
            TAG_LOGD(AAFwkTag::APPMGR, "Application is in whiteList, skipping!");
            return;
        }

        processName += SERVICE_EXTENSION;
        if (appInfo->keepAlive) {
            processName += KEEP_ALIVE;
        }
    }
}

void AppMgrServiceInner::MakeProcessName(const std::shared_ptr<AbilityInfo> &abilityInfo,
    const std::shared_ptr<ApplicationInfo> &appInfo, const HapModuleInfo &hapModuleInfo, int32_t appIndex,
    const std::string &specifiedProcessFlag, std::string &processName, bool isCallerSetProcess) const
{
    if (!abilityInfo || !appInfo) {
        TAG_LOGE(AAFwkTag::APPMGR, "param error");
        return;
    }
    if (!abilityInfo->process.empty() && (isCallerSetProcess || specifiedProcessFlag.empty())) {
        TAG_LOGD(AAFwkTag::APPMGR, "Process not null");
        if (abilityInfo->type == AppExecFwk::AbilityType::PAGE && abilityInfo->isStageBasedModel) {
            processName = appInfo->bundleName + abilityInfo->process;
        } else {
            processName = abilityInfo->process;
        }
        // extension's process is bundleName:extensionType, generated at installation time
        MakeIsolateSandBoxProcessName(abilityInfo, hapModuleInfo, processName);
        if (appIndex != 0) {
            processName += ":" + std::to_string(appIndex);
        }
        return;
    }
    MakeProcessName(appInfo, hapModuleInfo, processName);
    MakeServiceExtProcessName(abilityInfo, appInfo, processName);
    if (appIndex != 0) {
        processName += std::to_string(appIndex);
    }

    if (!specifiedProcessFlag.empty()) {
        processName = (processName + ":" + specifiedProcessFlag).substr(0, MAX_SPECIFIED_PROCESS_NAME_LENGTH);
        TAG_LOGI(AAFwkTag::APPMGR, "specifiedProcessFlag: %{public}s, processName: %{public}s",
            specifiedProcessFlag.c_str(), processName.c_str());
    }
}

void AppMgrServiceInner::MakeProcessName(
    const std::shared_ptr<ApplicationInfo> &appInfo, const HapModuleInfo &hapModuleInfo, std::string &processName) const
{
    if (!appInfo) {
        TAG_LOGE(AAFwkTag::APPMGR, "appInfo null");
        return;
    }
    // check after abilityInfo, because abilityInfo contains extension process.
    if (hapModuleInfo.isStageBasedModel && !hapModuleInfo.process.empty()
        && hapModuleInfo.process != appInfo->bundleName) {
        processName = hapModuleInfo.process;
        TAG_LOGI(AAFwkTag::APPMGR, "processName:%{public}s", processName.c_str());
        return;
    }
    bool isRunInIsolationMode = CheckIsolationMode(hapModuleInfo);
    if (hapModuleInfo.isStageBasedModel && isRunInIsolationMode) {
        processName = appInfo->bundleName;
        processName.append(":");
        processName.append(hapModuleInfo.name);
        return;
    }
    if (!appInfo->process.empty()) {
        processName = appInfo->process;
        return;
    }
    processName = appInfo->bundleName;
}

void AppMgrServiceInner::LoadAbilityNoAppRecord(const std::shared_ptr<AppRunningRecord> appRecord,
    bool isShellCall, std::shared_ptr<ApplicationInfo> appInfo,
    std::shared_ptr<AbilityInfo> abilityInfo, const std::string &processName,
    const std::string &specifiedProcessFlag, const BundleInfo &bundleInfo, const HapModuleInfo &hapModuleInfo,
    std::shared_ptr<AAFwk::Want> want, bool appExistFlag, bool isPreload, AppExecFwk::PreloadMode preloadMode,
    sptr<IRemoteObject> token, const std::string &customProcessFlag)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    TAG_LOGI(AAFwkTag::APPMGR, "processName:%{public}s, isPreload:%{public}d",
        processName.c_str(), isPreload);
    if (!appRecord || !abilityInfo) {
        TAG_LOGE(AAFwkTag::APPMGR, "null appRecord or abilityInfo");
        return;
    }
    if (!specifiedProcessFlag.empty()) {
        appRecord->SetSpecifiedProcessFlag(specifiedProcessFlag);
    }
    if (!customProcessFlag.empty()) {
        appRecord->SetCustomProcessFlag(customProcessFlag);
    }
    if (hapModuleInfo.isStageBasedModel && !IsMainProcess(appInfo, processName)) {
        appRecord->SetEmptyKeepAliveAppState(false);
        appRecord->SetMainProcess(false);
        TAG_LOGI(AAFwkTag::APPMGR, "%{public}s will not alive", hapModuleInfo.process.c_str());
    }
    // As taskHandler_ is busy now, the task should be submit to other task queue.
    if (otherTaskHandler_ != nullptr) {
        otherTaskHandler_->SubmitTaskJust([appRecord, abilityInfo, innerServiceWeak = weak_from_this()]() {
            auto innerService = innerServiceWeak.lock();
            CHECK_POINTER_AND_RETURN_LOG(innerService, "get appMgrServiceInner fail");
            innerService->OnAppStateChanged(appRecord, ApplicationState::APP_STATE_SET_COLD_START, false, false);
            innerService->SendAppStartupTypeEvent(appRecord, abilityInfo, AppStartType::COLD, AppStartReason::NONE);
            }, "AppStateChangedNotify", FIRST_FRAME_NOTIFY_TASK_DELAY);
    }
    uint32_t startFlags = (want == nullptr) ? 0 : AppspawnUtil::BuildStartFlags(*want, *abilityInfo);
    int32_t bundleIndex = 0;
    if (want != nullptr) {
        (void)AbilityRuntime::StartupUtil::GetAppIndex(*want, bundleIndex);
    }
    if (StartProcess(abilityInfo->applicationName, processName, startFlags, appRecord,
        appInfo->uid, bundleInfo, appInfo->bundleName, bundleIndex, appExistFlag, isPreload, preloadMode,
        abilityInfo->moduleName, abilityInfo->name, token, want, abilityInfo->extensionAbilityType) != ERR_OK) {
        NotifyStartProcessFailed(token);
    }
    if (isShellCall) {
        std::string perfCmd = (want == nullptr) ? "" : want->GetStringParam(PERF_CMD);
        bool isSandboxApp = (want == nullptr) ? false : want->GetBoolParam(ENTER_SANDBOX, false);
        (void)StartPerfProcess(appRecord, perfCmd, "", isSandboxApp);
    }
}

std::string AppMgrServiceInner::GetSpecifiedProcessFlag(std::shared_ptr<AbilityInfo> abilityInfo,
    std::shared_ptr<AAFwk::Want> want)
{
    if (!abilityInfo) {
        TAG_LOGE(AAFwkTag::APPMGR, "abilityInfo null");
        return "";
    }
    if (!want) {
        TAG_LOGE(AAFwkTag::APPMGR, "want null");
        return "";
    }
    return GetSpecifiedProcessFlag(*abilityInfo, *want);
}

std::string AppMgrServiceInner::GetSpecifiedProcessFlag(const AbilityInfo &abilityInfo, const AAFwk::Want &want)
{
    std::string specifiedProcessFlag;
    bool isUIAbility = (abilityInfo.type == AppExecFwk::AbilityType::PAGE && abilityInfo.isStageBasedModel);
    bool isSpecifiedProcess = abilityInfo.isolationProcess &&
        AAFwk::AppUtils::GetInstance().IsStartSpecifiedProcess() && isUIAbility;
    if (isSpecifiedProcess) {
        specifiedProcessFlag = want.GetStringParam(PARAM_SPECIFIED_PROCESS_FLAG);
        TAG_LOGI(AAFwkTag::APPMGR, "specifiedProcessFlag: %{public}s", specifiedProcessFlag.c_str());
    }
    return specifiedProcessFlag;
}

bool AppMgrServiceInner::IsMainProcess(const std::shared_ptr<ApplicationInfo> &appInfo,
    const std::string &processName) const
{
    if (!appInfo) {
        return true;
    }
    if (!appInfo->process.empty()) {
        if (processName == appInfo->process) {
            return true;
        }
    } else {
        if (processName == appInfo->bundleName) {
            return true;
        }
    }
    return false;
}

bool AppMgrServiceInner::CheckIsolationMode(const HapModuleInfo &hapModuleInfo) const
{
    IsolationMode isolationMode = hapModuleInfo.isolationMode;
    if (supportIsolationMode_.compare("true") == 0) {
        switch (isolationMode) {
            case IsolationMode::ISOLATION_FIRST:
                return true;
            case IsolationMode::ISOLATION_ONLY:
                return true;
            default:
                return false;
        }
    }
    return false;
}

bool AppMgrServiceInner::GetBundleAndHapInfo(const AbilityInfo &abilityInfo,
    const std::shared_ptr<ApplicationInfo> &appInfo, BundleInfo &bundleInfo, HapModuleInfo &hapModuleInfo,
    int32_t appIndex) const
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto bundleMgrHelper = remoteClientManager_->GetBundleManagerHelper();
    if (bundleMgrHelper == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "bundleMgrHelper null");
        return false;
    }

    auto userId = GetUserIdByUid(appInfo->uid);
    TAG_LOGD(AAFwkTag::APPMGR, "userId: %{public}d, bundleName: %{public}s, appIndex: %{public}d", userId,
        appInfo->bundleName.c_str(), appIndex);
    int32_t bundleMgrResult;
    if (appIndex == 0) {
        bundleMgrResult = IN_PROCESS_CALL(bundleMgrHelper->GetBundleInfoV9(appInfo->bundleName,
            static_cast<int32_t>(GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_APPLICATION) |
            static_cast<int32_t>(GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_EXTENSION_ABILITY) |
            static_cast<int32_t>(GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_HAP_MODULE) |
            static_cast<int32_t>(GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_REQUESTED_PERMISSION), bundleInfo, userId));
    } else if (appIndex <= AbilityRuntime::GlobalConstant::MAX_APP_CLONE_INDEX) {
        bundleMgrResult = IN_PROCESS_CALL(bundleMgrHelper->GetCloneBundleInfo(appInfo->bundleName,
            static_cast<int32_t>(GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_APPLICATION) |
            static_cast<int32_t>(GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_REQUESTED_PERMISSION),
            appIndex, bundleInfo, userId));
    } else {
        bundleMgrResult = IN_PROCESS_CALL(bundleMgrHelper->GetSandboxBundleInfo(appInfo->bundleName,
            appIndex, userId, bundleInfo));
    }

    if (bundleMgrResult != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "getBundleInfo fail");
        return false;
    }
    bool hapQueryResult = false;
    if (appIndex <= AbilityRuntime::GlobalConstant::MAX_APP_CLONE_INDEX) {
        hapQueryResult = bundleMgrHelper->GetHapModuleInfo(abilityInfo, userId, hapModuleInfo);
    } else {
        hapQueryResult = (bundleMgrHelper->GetSandboxHapModuleInfo(abilityInfo, appIndex, userId, hapModuleInfo) == 0);
    }
    if (!hapQueryResult) {
        TAG_LOGE(AAFwkTag::APPMGR, "getHapModuleInfo fail");
        return false;
    }
    return true;
}

void AppMgrServiceInner::AttachApplication(const pid_t pid, const sptr<IAppScheduler> &appScheduler)
{
    TAG_LOGI(AAFwkTag::APPMGR, "%{public}s called", __func__);
    if (pid <= 0) {
        TAG_LOGE(AAFwkTag::APPMGR, "invalid pid:%{public}d", pid);
        return;
    }
    AbilityRuntime::FreezeUtil::GetInstance().AddAppLifecycleEvent(pid, "ServiceInner::AttachApplication");
    auto appRecord = GetAppRunningRecordByPid(pid);
    CHECK_POINTER_AND_RETURN_LOG(appRecord, "no such appRecord");
    auto applicationInfo = appRecord->GetApplicationInfo();
    AAFwk::EventInfo eventInfo;
    if (!applicationInfo) {
        TAG_LOGE(AAFwkTag::APPMGR, "null appInfo");
    } else {
        eventInfo.bundleName = applicationInfo->name;
        eventInfo.versionName = applicationInfo->versionName;
        eventInfo.versionCode = applicationInfo->versionCode;
    }
    std::string connector = "##";
    std::string traceName = __PRETTY_FUNCTION__ + connector + eventInfo.bundleName;
    HITRACE_METER_NAME(HITRACE_TAG_APP, traceName);
    if (appScheduler == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "attach null, pid: %{public}d, bundleName: %{public}s", pid,
            eventInfo.bundleName.c_str());
        NotifyAppAttachFailed(appRecord);
        return;
    }
    TAG_LOGI(AAFwkTag::APPMGR, "attach pid:%{public}d, bundle:%{public}s", pid, eventInfo.bundleName.c_str());
    sptr<AppDeathRecipient> appDeathRecipient = sptr<AppDeathRecipient>::MakeSptr();
    CHECK_POINTER_AND_RETURN_LOG(appDeathRecipient, "Failed to create death recipient.");
    appDeathRecipient->SetTaskHandler(taskHandler_);
    appDeathRecipient->SetAppMgrServiceInner(shared_from_this());
    auto object = appScheduler->AsObject();
    if (!object || !object->AddDeathRecipient(appDeathRecipient)) {
        TAG_LOGE(AAFwkTag::APPMGR, "add DeathRecipient for %{public}s fail", appRecord->GetProcessName().c_str());
        return;
    }

    appRecord->SetAppDeathRecipient(appDeathRecipient);
    appRecord->SetApplicationClient(appScheduler);
    if (appRecord->GetState() == ApplicationState::APP_STATE_CREATE) {
        LaunchApplicationExt(appRecord);
    }

    // submit cached load ability task after scene board attach
    if (appRecord->GetBundleName() == SCENE_BOARD_BUNDLE_NAME) {
        sceneBoardAttachFlag_ = true;
        SubmitCacheLoadAbilityTask();
    }
    eventInfo.pid = appRecord->GetPid();
    eventInfo.processName = appRecord->GetProcessName();
    AAFwk::EventReport::SendAppEvent(AAFwk::EventName::APP_ATTACH, HiSysEventType::BEHAVIOR, eventInfo);
}

void AppMgrServiceInner::LaunchApplicationExt(const std::shared_ptr<AppRunningRecord> &appRecord)
{
    auto isPreload = IsAllowedNWebPreload(appRecord->GetProcessName());
    appRecord->SetNWebPreload(isPreload);
    LaunchApplication(appRecord);
}

bool AppMgrServiceInner::IsAllowedNWebPreload(const std::string &processName)
{
    // nwebPreloadSet_ only be initialized in Init(), no lock required.
    return nwebPreloadSet_.count(processName);
}

void AppMgrServiceInner::NotifyAppAttachFailed(std::shared_ptr<AppRunningRecord> appRecord)
{
    CHECK_POINTER_AND_RETURN_LOG(appRecord, "AppRecord null.");
    std::vector<sptr<IRemoteObject>> abilityTokens;
    for (const auto &token : appRecord->GetAbilities()) {
        abilityTokens.emplace_back(token.first);
    }
    TAG_LOGI(AAFwkTag::APPMGR, "attach fail name: %{public}s %{public}zu", appRecord->GetProcessName().c_str(),
        abilityTokens.size());
    std::lock_guard lock(appStateCallbacksLock_);
    for (const auto &item : appStateCallbacks_) {
        if (item.callback != nullptr) {
            item.callback->OnAppRemoteDied(abilityTokens);
        }
    }
}

void AppMgrServiceInner::NotifyLoadAbilityFailed(sptr<IRemoteObject> token)
{
    CHECK_POINTER_AND_RETURN_LOG(token, "token null.");
    std::vector<sptr<IRemoteObject>> abilityTokens;
    abilityTokens.emplace_back(token);
    std::lock_guard lock(appStateCallbacksLock_);
    for (const auto &item : appStateCallbacks_) {
        if (item.callback != nullptr) {
            item.callback->OnAppRemoteDied(abilityTokens);
        }
    }
}

void AppMgrServiceInner::NotifyStartProcessFailed(sptr<IRemoteObject> token)
{
    CHECK_POINTER_AND_RETURN_LOG(token, "token null.");
    std::lock_guard lock(appStateCallbacksLock_);
    for (const auto &item : appStateCallbacks_) {
        if (item.callback != nullptr) {
            item.callback->OnStartProcessFailed(token);
        }
    }
}

void AppMgrServiceInner::LaunchApplication(const std::shared_ptr<AppRunningRecord> &appRecord)
{
    CHECK_POINTER_AND_RETURN_LOG(appRecord, "appRecord null");
    appRecord->AddAppLifecycleEvent("ServiceInner::LaunchApplication");
    auto applicationInfo = appRecord->GetApplicationInfo();
    std::string bundleName = "";
    if (!applicationInfo) {
        TAG_LOGE(AAFwkTag::APPMGR, "appInfo null");
    } else {
        bundleName = applicationInfo->name;
    }
    std::string connector = "##";
    std::string traceName = __PRETTY_FUNCTION__ + connector + bundleName;
    HITRACE_METER_NAME(HITRACE_TAG_APP, traceName);

    if (appRecord->GetState() != ApplicationState::APP_STATE_CREATE) {
        TAG_LOGE(AAFwkTag::APPMGR, "wrong app state:%{public}d", appRecord->GetState());
        return;
    }
    if (multiUserConfigurationMgr_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "multiUserConfigurationMgr_ null");
        return;
    }
    int32_t userId = appRecord->GetUid() / BASE_USER_RANGE;
    auto config = multiUserConfigurationMgr_->GetConfigurationByUserId(userId);
    if (config == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "config null");
        return;
    }

    TAG_LOGD(AAFwkTag::APPMGR, "LaunchApplication configuration:%{public}s", config->GetName().c_str());
    appRecord->LaunchApplication(*config);
    appRecord->SetState(ApplicationState::APP_STATE_READY);
    int restartResidentProcCount = MAX_RESTART_COUNT;
    appRecord->SetRestartResidentProcCount(restartResidentProcCount);

    // There is no ability when the empty resident process starts
    // The status of all resident processes is ready
    // There is no process of switching the foreground, waiting for his first ability to start
    if (appRecord->IsEmptyKeepAliveApp()) {
        appRecord->AddAbilityStage();
        return;
    }
    appRecord->LaunchPendingAbilities();
    AddAbilityStageForSpecified(appRecord);

    if (appRecord->IsPreloading()) {
        appRecord->SetPreloadState(PreloadState::PRELOADED);
    }
    SendAppLaunchEvent(appRecord);
}

void AppMgrServiceInner::AddAbilityStageForSpecified(std::shared_ptr<AppRunningRecord> appRecord)
{
    CHECK_POINTER_AND_RETURN_LOG(appRecord, "appRecord null");
    if (appRecord->IsStartSpecifiedAbility()) {
        TAG_LOGI(AAFwkTag::APPMGR, "start specified ability");
        auto moduleRecordList = appRecord->GetAllModuleRecord();
        for (const auto &iter : moduleRecordList) {
            iter->SetModuleRecordState(ModuleRecordState::INITIALIZED_STATE);
        }
        appRecord->AddAbilityStageBySpecifiedAbility(appRecord->GetBundleName());
    }

    if (appRecord->IsNewProcessRequest()) {
        appRecord->AddAbilityStageBySpecifiedProcess(appRecord->GetBundleName());
    }
}

void AppMgrServiceInner::AddAbilityStageDone(const int32_t recordId)
{
    auto appRecord = GetAppRunningRecordByAppRecordId(recordId);
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "get appRecord fail");
        return;
    }
    appRecord->AddAbilityStageDone();
}

void AppMgrServiceInner::UpdateAllProviderConfig(const std::shared_ptr<AppRunningRecord> &appRecord)
{
    if (appRunningManager_ == nullptr || appRecord == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null ptr");
        return;
    }

    auto obj = appRecord->GetPriorityObject();
    if (obj == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null ptr");
        return;
    }

    TAG_LOGD(AAFwkTag::APPMGR, "hostPid: %{public}d", obj->GetPid());
    std::vector<pid_t> providerPids;
    appRunningManager_->GetAllUIExtensionProviderPid(obj->GetPid(), providerPids);

    for (pid_t providerPid : providerPids) {
        auto providerRecord = appRunningManager_->GetAppRunningRecordByPid(providerPid);
        appRunningManager_->UpdateConfigurationDelayed(providerRecord);
    }
}

void AppMgrServiceInner::ApplicationForegrounded(const int32_t recordId)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    auto appRecord = GetAppRunningRecordByAppRecordId(recordId);
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "get appRecord fail");
        return;
    }
    appRecord->AddAppLifecycleEvent("ServiceInner::AppForegrounded");
    // Prevent forged requests from changing the app's state.
    if (appRecord->GetApplicationScheduleState() != ApplicationScheduleState::SCHEDULE_FOREGROUNDING) {
        TAG_LOGE(AAFwkTag::APPMGR, "app is not scheduling to foreground");
        return;
    }
    appRecord->SetApplicationScheduleState(ApplicationScheduleState::SCHEDULE_READY);
    ApplicationState appState = appRecord->GetState();
    if (appState == ApplicationState::APP_STATE_READY || appState == ApplicationState::APP_STATE_BACKGROUND) {
        if (appState == ApplicationState::APP_STATE_BACKGROUND) {
            appRunningManager_->UpdateConfigurationDelayed(appRecord);
            UpdateAllProviderConfig(appRecord);
        }
        appRecord->SetState(ApplicationState::APP_STATE_FOREGROUND);
        bool needNotifyApp = appRunningManager_->IsApplicationFirstForeground(*appRecord);
        OnAppStateChanged(appRecord, ApplicationState::APP_STATE_FOREGROUND, needNotifyApp, false);
        DelayedSingleton<AppStateObserverManager>::GetInstance()->OnProcessStateChanged(appRecord);
    } else {
        TAG_LOGW(AAFwkTag::APPMGR, "app name(%{public}s), app state(%{public}d)",
            appRecord->GetName().c_str(), static_cast<ApplicationState>(appState));
    }
    appRecord->PopForegroundingAbilityTokens();

    TAG_LOGI(AAFwkTag::APPMGR, "ApplicationForegrounded, bundle: %{public}s", appRecord->GetBundleName().c_str());
    if (appRecord->GetApplicationPendingState() == ApplicationPendingState::BACKGROUNDING) {
        appRecord->ScheduleBackgroundRunning();
    } else if (appRecord->GetApplicationPendingState() == ApplicationPendingState::FOREGROUNDING) {
        appRecord->SetApplicationPendingState(ApplicationPendingState::READY);
    }
    auto eventInfo = BuildEventInfo(appRecord);
    int32_t callerPid = appRecord->GetCallerPid() == -1 ? IPCSkeleton::GetCallingPid() : appRecord->GetCallerPid();
    auto callerRecord = GetAppRunningRecordByPid(callerPid);
    if (callerRecord != nullptr) {
        eventInfo.callerBundleName = callerRecord->GetBundleName();
    } else {
        TAG_LOGE(AAFwkTag::APPMGR, "callerRecord null");
    }
    AAFwk::EventReport::SendAppForegroundEvent(AAFwk::EventName::APP_FOREGROUND, eventInfo);
}

void AppMgrServiceInner::ApplicationBackgrounded(const int32_t recordId)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    auto appRecord = GetAppRunningRecordByAppRecordId(recordId);
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "get appRecord fail");
        return;
    }
    // Prevent forged requests from changing the app's state.
    appRecord->AddAppLifecycleEvent("ServiceInner::AppBackgrounded");
    if (appRecord->GetApplicationScheduleState() != ApplicationScheduleState::SCHEDULE_BACKGROUNDING) {
        TAG_LOGE(AAFwkTag::APPMGR, "app is not scheduling to background");
        return;
    }
    appRecord->SetApplicationScheduleState(ApplicationScheduleState::SCHEDULE_READY);
    if (appRecord->GetState() == ApplicationState::APP_STATE_FOREGROUND) {
        appRecord->SetState(ApplicationState::APP_STATE_BACKGROUND);
        bool needNotifyApp = !AAFwk::UIExtensionUtils::IsUIExtension(appRecord->GetExtensionType())
            && !AAFwk::UIExtensionUtils::IsWindowExtension(appRecord->GetExtensionType())
            && appRunningManager_->IsApplicationBackground(*appRecord);
        OnAppStateChanged(appRecord, ApplicationState::APP_STATE_BACKGROUND, needNotifyApp, false);
        DelayedSingleton<AppStateObserverManager>::GetInstance()->OnProcessStateChanged(appRecord);
    } else {
        TAG_LOGW(AAFwkTag::APPMGR, "app name(%{public}s), app state(%{public}d)",
            appRecord->GetName().c_str(), static_cast<ApplicationState>(appRecord->GetState()));
    }
    auto pendingState = appRecord->GetApplicationPendingState();
    TAG_LOGI(AAFwkTag::APPMGR, "app backgrounded: %{public}s, pState: %{public}d", appRecord->GetBundleName().c_str(),
        pendingState);
    if (pendingState == ApplicationPendingState::FOREGROUNDING) {
        appRecord->ScheduleForegroundRunning();
    } else if (pendingState == ApplicationPendingState::BACKGROUNDING) {
        appRecord->SetApplicationPendingState(ApplicationPendingState::READY);
    }
    auto eventInfo = BuildEventInfo(appRecord);
    AAFwk::EventReport::SendAppBackgroundEvent(AAFwk::EventName::APP_BACKGROUND, eventInfo);
}

AAFwk::EventInfo AppMgrServiceInner::BuildEventInfo(std::shared_ptr<AppRunningRecord> appRecord) const
{
    AAFwk::EventInfo eventInfo;
    if (appRecord == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRecord null");
        return eventInfo;
    }
    auto applicationInfo = appRecord->GetApplicationInfo();
    if (!applicationInfo) {
        TAG_LOGW(AAFwkTag::APPMGR, "appInfo null");
    } else {
        eventInfo.bundleName = applicationInfo->name;
        eventInfo.versionName = applicationInfo->versionName;
        eventInfo.versionCode = applicationInfo->versionCode;
        eventInfo.bundleType = static_cast<int32_t>(applicationInfo->bundleType);
    }
    if (appRecord->GetPriorityObject() != nullptr) {
        eventInfo.pid = appRecord->GetPid();
    }
    eventInfo.processName = appRecord->GetProcessName();
    eventInfo.processType = static_cast<int32_t>(appRecord->GetProcessType());
    return eventInfo;
}

void AppMgrServiceInner::ApplicationTerminated(const int32_t recordId)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    if (!appRunningManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRunningManager_ null");
        return;
    }
    auto appRecord = GetAppRunningRecordByAppRecordId(recordId);
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "get appRecord fail");
        return;
    }
    appRecord->ApplicationTerminated();
    if (appRecord->GetState() != ApplicationState::APP_STATE_BACKGROUND) {
        TAG_LOGD(AAFwkTag::APPMGR, "current state is not background");
        return;
    }

    KillRenderProcess(appRecord);
#ifdef SUPPORT_CHILD_PROCESS
    KillChildProcess(appRecord);
    KillAttachedChildProcess(appRecord);
#endif // SUPPORT_CHILD_PROCESS
    appRecord->SetState(ApplicationState::APP_STATE_TERMINATED);
    appRecord->RemoveAppDeathRecipient();
    appRecord->SetProcessChangeReason(ProcessChangeReason::REASON_APP_TERMINATED);
    OnAppStateChanged(appRecord, ApplicationState::APP_STATE_TERMINATED, false, false);
    appRunningManager_->RemoveAppRunningRecordById(recordId);
    AAFwk::EventInfo eventInfo;
    auto applicationInfo = appRecord->GetApplicationInfo();
    if (!applicationInfo) {
        TAG_LOGE(AAFwkTag::APPMGR, "appInfo null");
    } else {
        eventInfo.bundleName = applicationInfo->name;
        eventInfo.versionName = applicationInfo->versionName;
        eventInfo.versionCode = applicationInfo->versionCode;
    }
    ClearAppRunningDataForKeepAlive(appRecord);
    eventInfo.pid = appRecord->GetPid();
    eventInfo.processName = appRecord->GetProcessName();
    AAFwk::EventReport::SendAppEvent(AAFwk::EventName::APP_TERMINATE, HiSysEventType::BEHAVIOR, eventInfo);

    ApplicationTerminatedSendProcessEvent(appRecord);
    taskHandler_->CancelTask("DELAY_KILL_PROCESS_" + std::to_string(recordId));
    auto uid = appRecord->GetUid();
    bool foreground = appRecord->GetState() == ApplicationState::APP_STATE_FOREGROUND ||
        appRecord->GetState() == ApplicationState::APP_STATE_FOCUS;
    std::string killReason = appRecord->GetKillReason().empty() ? "Kill Reason:app exit" : appRecord->GetKillReason();
    auto result = HiSysEventWrite(OHOS::HiviewDFX::HiSysEvent::Domain::FRAMEWORK, "PROCESS_KILL",
        OHOS::HiviewDFX::HiSysEvent::EventType::FAULT,
        EVENT_KEY_PID, std::to_string(eventInfo.pid), EVENT_KEY_PROCESS_NAME, eventInfo.processName,
        EVENT_KEY_MESSAGE, killReason, EVENT_KEY_FOREGROUND, foreground);
    TAG_LOGW(AAFwkTag::APPMGR, "hisysevent write result=%{public}d, send [FRAMEWORK,PROCESS_KILL], pid=%{public}d,"
        " processName=%{public}s, msg=%{public}s, FOREGROUND = %{public}d",
        result, eventInfo.pid, eventInfo.processName.c_str(), killReason.c_str(), foreground);
    NotifyAppRunningStatusEvent(appRecord->GetBundleName(), uid, AbilityRuntime::RunningStatus::APP_RUNNING_STOP);
}

int32_t AppMgrServiceInner::UpdateApplicationInfoInstalled(
    const std::string &bundleName, const int uid, const std::string &moduleName)
{
    if (!appRunningManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRunningManager_ null");
        return ERR_NO_INIT;
    }

    int32_t result = VerifyRequestPermission();
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "permission verification fail");
        return result;
    }

    if (remoteClientManager_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "remoteClientManager_ fail");
        return ERR_NO_INIT;
    }

    auto bundleMgrHelper = remoteClientManager_->GetBundleManagerHelper();
    if (bundleMgrHelper == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "bundleMgrHelper null");
        return ERR_NO_INIT;
    }
    auto userId = GetUserIdByUid(uid);
    TAG_LOGD(AAFwkTag::APPMGR, "userId: %{public}d, bundleName: %{public}s", userId, bundleName.c_str());
    ApplicationInfo appInfo;
    HITRACE_METER_NAME(HITRACE_TAG_APP, "BMS->GetApplicationInfo");
    bool bundleMgrResult = bundleMgrHelper->GetApplicationInfo(bundleName,
        ApplicationFlag::GET_BASIC_APPLICATION_INFO, userId, appInfo);
    if (!bundleMgrResult) {
        TAG_LOGE(AAFwkTag::APPMGR, "get applicationInfo fail");
        return ERR_INVALID_OPERATION;
    }

    TAG_LOGD(AAFwkTag::APPMGR, "uid value is %{public}d", uid);
    result = appRunningManager_->ProcessUpdateApplicationInfoInstalled(appInfo, moduleName);
    if (result != ERR_OK) {
        TAG_LOGI(AAFwkTag::APPMGR, "process corresponding package name unstart");
    }

    return result;
}

int32_t AppMgrServiceInner::KillApplication(const std::string &bundleName, bool clearPageStack, int32_t appIndex)
{
    TAG_LOGI(AAFwkTag::APPMGR, "call");
    if (!appRunningManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRunningManager_ null");
        return ERR_NO_INIT;
    }

    auto result = VerifyKillProcessPermission(bundleName);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "permission verification fail");
        return result;
    }

    return KillApplicationByBundleName(bundleName, appIndex, clearPageStack, "KillApplication");
}

int32_t AppMgrServiceInner::ForceKillApplication(const std::string &bundleName,
    const int userId, const int appIndex)
{
    TAG_LOGI(AAFwkTag::APPMGR, "call");
    if (!IsSceneBoardCall()) {
        TAG_LOGE(AAFwkTag::APPMGR, "not called by sceneBoard");
        return AAFwk::CHECK_PERMISSION_FAILED;
    }

    return ForceKillApplicationInner(bundleName, userId, appIndex);
}

int32_t AppMgrServiceInner::ForceKillApplicationInner(const std::string &bundleName,
    const int userId, const int appIndex)
{
    TAG_LOGD(AAFwkTag::APPMGR, "Called.");
    if (!appRunningManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRunningManager_ null");
        return ERR_NO_INIT;
    }

    std::list<pid_t> pids;
    int32_t newUserId = userId;
    if (userId == DEFAULT_INVAL_VALUE) {
        newUserId = GetUserIdByUid(IPCSkeleton::GetCallingUid());
    }
    int32_t result = ERR_OK;
    if (!appRunningManager_->GetPidsByBundleNameUserIdAndAppIndex(bundleName, newUserId, appIndex, pids)) {
        TAG_LOGI(AAFwkTag::APPMGR, "unstart");
        return result;
    }
    for (auto iter = pids.begin(); iter != pids.end(); ++iter) {
        result = KillProcessByPid(*iter, "ForceKillApplicationByBundleName");
        if (result < 0) {
            TAG_LOGE(AAFwkTag::APPMGR,
                "forceKillApplicationByBundleName fail for bundleName:%{public}s pid:%{public}d",
                bundleName.c_str(), *iter);
            return result;
        }
    }
    return result;
}

int32_t AppMgrServiceInner::WaitProcessesExitAndKill(std::list<pid_t> &pids, const int64_t startTime,
    const std::string& reason)
{
    int32_t result = ERR_OK;
    if (WaitForRemoteProcessExit(pids, startTime)) {
        TAG_LOGI(AAFwkTag::APPMGR, "remote process exited successs");
        return result;
    }
    for (auto iter = pids.begin(); iter != pids.end(); ++iter) {
        auto singleRet = KillProcessByPid(*iter, reason);
        if (singleRet != 0 && singleRet != AAFwk::ERR_KILL_PROCESS_NOT_EXIST) {
            TAG_LOGE(AAFwkTag::APPMGR, "killApplication fail for pid:%{public}d", *iter);
            result = singleRet;
        }
    }
    return result;
}

void AppMgrServiceInner::DoAllProcessExitCallback(std::list<SimpleProcessInfo> &processInfos,
    int32_t userId, sptr<AAFwk::IUserCallback> callback)
{
    TAG_LOGI(AAFwkTag::APPMGR, "DoAllProcessExitCallback call");
    if (callback == nullptr) {
        return;
    }
    if (ProcessUtil::IsAllProcessKilled(processInfos)) {
        TAG_LOGI(AAFwkTag::APPMGR, "all process exit");
        callback->OnLogoutUserDone(userId, ERR_OK);
        return;
    }
    auto checkProcessExistCallback = [processInfos, userId, callback] () mutable {
        TAG_LOGI(AAFwkTag::APPMGR, "checkProcessExistCallback call");
        if (callback == nullptr) {
            return;
        }
        if (ProcessUtil::IsAllProcessKilled(processInfos)) {
            TAG_LOGI(AAFwkTag::APPMGR, "all process exit");
            callback->OnLogoutUserDone(userId, ERR_OK);
            return;
        }
        TAG_LOGI(AAFwkTag::APPMGR, "not all process exit");
        callback->OnLogoutUserDone(userId, AAFwk::KILL_PROCESS_FAILED);
    };
    if (taskHandler_) {
        taskHandler_->SubmitTaskJust(checkProcessExistCallback, "DelayCheckProcessExit",
            AMSEventHandler::DELAY_CHECK_ALL_PROCESSES_EXITED);
    }
}

int32_t AppMgrServiceInner::WaitProcessesExitAndKill(std::list<SimpleProcessInfo> &processInfos,
    const int64_t startTime, const std::string& reason, int32_t userId, sptr<AAFwk::IUserCallback> callback)
{
    int32_t result = ERR_OK;
    ProcessUtil::UpdateProcessNameByProcFile(processInfos);
    if (WaitForRemoteProcessExit(processInfos, startTime)) {
        TAG_LOGI(AAFwkTag::APPMGR, "remote process exited successs");
        if (callback) {
            callback->OnLogoutUserDone(userId, ERR_OK);
        }
        return result;
    }
    for (auto iter = processInfos.begin(); iter != processInfos.end(); ++iter) {
        auto singleRet = KillProcessByPid((*iter).pid, reason);
        if (singleRet != 0 && singleRet != AAFwk::ERR_KILL_PROCESS_NOT_EXIST) {
            TAG_LOGE(AAFwkTag::APPMGR, "killApplication fail for pid:%{public}d", (*iter).pid);
            result = singleRet;
        }
    }
    DoAllProcessExitCallback(processInfos, userId, callback);
    return result;
}

int32_t AppMgrServiceInner::KillProcessesByAccessTokenId(const uint32_t accessTokenId)
{
    TAG_LOGI(AAFwkTag::APPMGR, "call");
    CHECK_CALLER_IS_SYSTEM_APP;
    auto isCallingPerm = AAFwk::PermissionVerification::GetInstance()->VerifyCallingPermission(
        AAFwk::PermissionConstants::PERMISSION_KILL_APP_PROCESSES);
    if (!isCallingPerm) {
        TAG_LOGE(AAFwkTag::APPMGR, "no permission to kill processes");
        return ERR_PERMISSION_DENIED;
    }

    int32_t result = ERR_OK;
    std::vector<pid_t> pids;
    result = GetPidsByAccessTokenId(accessTokenId, pids);
    if (result != ERR_OK) {
        return result;
    }
    if (pids.empty()) {
        TAG_LOGI(AAFwkTag::APPMGR, "no accessTokenId");
        return ERR_OK;
    }

    for (auto iter = pids.begin(); iter != pids.end(); ++iter) {
        result = KillProcessByPid(*iter, "KillProcessesByAccessTokenId");
        if (result < 0) {
            TAG_LOGE(AAFwkTag::APPMGR,
                "killProcessesByAccessTokenId fail for accessTokenId:%{public}d,pid:%{public}d",
                accessTokenId, *iter);
            return result;
        }
    }
    return result;
}

int32_t AppMgrServiceInner::KillApplicationByUid(const std::string &bundleName, const int uid,
    const std::string& reason)
{
    if (!appRunningManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRunningManager_ null");
        return ERR_NO_INIT;
    }

    int32_t result = ERR_OK;
    result = VerifyKillProcessPermission(bundleName);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "Permission verification failed.");
        return result;
    }

    int64_t startTime = SystemTimeMillisecond();
    std::list<pid_t> pids;
    TAG_LOGI(AAFwkTag::APPMGR, "uid value: %{public}d", uid);
    KillProcessConfig config{false, false, reason};
    if (!appRunningManager_->ProcessExitByBundleNameAndUid(bundleName, uid, pids, config)) {
        TAG_LOGI(AAFwkTag::APPMGR, "unstart");
        return result;
    }
    return WaitProcessesExitAndKill(pids, startTime, reason);
}

int32_t AppMgrServiceInner::UpdateProcessMemoryState(const std::vector<ProcessMemoryState> &procMemState)
{
    for (const auto &state : procMemState) {
        auto appRecord = GetAppRunningRecordByPid(state.pid);
        if (!appRecord) {
            TAG_LOGW(AAFwkTag::APPMGR, "no appRecord, pid:%{public}d", state.pid);
            continue;
        }
        appRecord->SetRssValue(state.rssValue);
        appRecord->SetPssValue(state.pssValue);
    }
    return ERR_OK;
}

void AppMgrServiceInner::SendProcessExitEventTask(pid_t pid, const std::string &processName, int32_t extensionType,
    int32_t exitReason, time_t exitTime, int32_t count)
{
    auto exitResult = !ProcessUtil::ProcessExist(pid);
    constexpr int32_t EXIT_SUCESS = 0;
    constexpr int32_t EXIT_FAILED = -1;
    AAFwk::EventInfo eventInfo;
    eventInfo.time = exitTime;
    eventInfo.pid = pid;
    eventInfo.processName = processName;
    eventInfo.extensionType = extensionType;
    eventInfo.exitReason = exitReason;

    if (exitResult) {
        eventInfo.exitResult = EXIT_SUCESS;
        AAFwk::EventReport::SendProcessExitEvent(AAFwk::EventName::PROCESS_EXIT, eventInfo);
        return;
    }

    if (--count <= 0) {
        eventInfo.exitResult = EXIT_FAILED;
        AAFwk::EventReport::SendProcessExitEvent(AAFwk::EventName::PROCESS_EXIT, eventInfo);
        return;
    }

    auto sendEventTask = [innerServiceWeak = weak_from_this(), pid, processName, extensionType, exitReason, exitTime,
        count] () {
        auto innerService = innerServiceWeak.lock();
        CHECK_POINTER_AND_RETURN_LOG(innerService, "get appMgrServiceInner fail");
        innerService->SendProcessExitEventTask(pid, processName, extensionType, exitReason, exitTime, count);
    };
    CHECK_POINTER_AND_RETURN_LOG(taskHandler_, "Task handler is nullptr.");
    taskHandler_->SubmitTaskJust(sendEventTask, PROCESS_EXIT_EVENT_TASK, KILL_PROCESS_DELAYTIME_MICRO_SECONDS);
}

void AppMgrServiceInner::SendProcessExitEvent(std::shared_ptr<AppRunningRecord> appRecord)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    if (appRecord == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRecord null");
        return;
    }
    if (appRecord->GetPriorityObject() == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "priority object null");
        return;
    }
    time_t currentTime;
    time(&currentTime);
    constexpr int32_t RETRY_COUNT = 5;

    auto pid = appRecord->GetPid();
    auto processName = appRecord->GetProcessName();
    auto extensionType = static_cast<int32_t>(appRecord->GetExtensionType());
    auto exitReason = appRecord->GetExitReason();
    SendProcessExitEventTask(pid, processName, extensionType, exitReason, currentTime, RETRY_COUNT);
    return;
}

int32_t AppMgrServiceInner::KillApplicationSelf(const bool clearPageStack, const std::string& reason)
{
    TAG_LOGI(AAFwkTag::APPMGR, "start");
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
    int64_t startTime = SystemTimeMillisecond();
    auto bundleName = appRecord->GetBundleName();
    auto callingUid = IPCSkeleton::GetCallingUid();
    TAG_LOGI(AAFwkTag::APPMGR, "uid value: %{public}d", callingUid);
    std::list<pid_t> pids;
    KillProcessConfig config{clearPageStack, false, reason};
    if (!appRunningManager_->ProcessExitByBundleNameAndUid(bundleName, callingUid, pids, config)) {
        TAG_LOGI(AAFwkTag::APPMGR, "unstart");
        return ERR_OK;
    }
    return WaitProcessesExitAndKill(pids, startTime, reason);
}

int32_t AppMgrServiceInner::KillAppSelfWithInstanceKey(const std::string &instanceKey, bool clearPageStack,
    const std::string& reason)
{
    if (!appRunningManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRunningManager_ null");
        return ERR_NO_INIT;
    }

    int64_t startTime = SystemTimeMillisecond();
    auto callingTokenId = IPCSkeleton::GetCallingTokenID();
    TAG_LOGI(AAFwkTag::APPMGR, "callingTokenId value: %{public}d", callingTokenId);
    std::list<pid_t> pids;
    if (!appRunningManager_->ProcessExitByTokenIdAndInstance(callingTokenId, instanceKey, pids, clearPageStack)) {
        TAG_LOGI(AAFwkTag::APPMGR, "not exist");
        return ERR_OK;
    }
    return WaitProcessesExitAndKill(pids, startTime, reason);
}

int32_t AppMgrServiceInner::KillApplicationByBundleName(
    const std::string &bundleName, int32_t appIndex, bool clearPageStack, const std::string& reason)
{
    int result = ERR_OK;
    int64_t startTime = SystemTimeMillisecond();
    std::list<pid_t> pids;

    if (!appRunningManager_->ProcessExitByBundleNameAndAppIndex(bundleName, appIndex, pids, clearPageStack)) {
        TAG_LOGE(AAFwkTag::APPMGR, "process corresponding to the package name did not start");
        return result;
    }
    result = WaitProcessesExitAndKill(pids, startTime, reason);
    NotifyAppStatus(bundleName, appIndex, EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_RESTARTED);
    return result;
}

int32_t AppMgrServiceInner::KillApplicationByUserId(
    const std::string &bundleName, int32_t appCloneIndex, const int userId,
    const bool clearPageStack, const std::string& reason)
{
    CHECK_CALLER_IS_SYSTEM_APP;
    if (VerifyAccountPermission(
        AAFwk::PermissionConstants::PERMISSION_KILL_APP_PROCESSES, userId) == ERR_PERMISSION_DENIED &&
        VerifyAccountPermission(
            AAFwk::PermissionConstants::PERMISSION_CLEAN_BACKGROUND_PROCESSES, userId) == ERR_PERMISSION_DENIED) {
        TAG_LOGE(AAFwkTag::APPMGR, "permission verify fail");
        return ERR_PERMISSION_DENIED;
    }

    KillProcessConfig config{clearPageStack, true, reason};
    return KillApplicationByUserIdLocked(bundleName, appCloneIndex, userId, config);
}

int32_t AppMgrServiceInner::KillApplicationByUserIdLocked(
    const std::string &bundleName, int32_t appCloneIndex, const int userId, const KillProcessConfig &config)
{
    if (!appRunningManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRunningManager_ null");
        return ERR_NO_INIT;
    }

    int64_t startTime = SystemTimeMillisecond();
    std::list<pid_t> pids;
    if (remoteClientManager_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "remoteClientManager_ null");
        return ERR_NO_INIT;
    }
    auto bundleMgrHelper = remoteClientManager_->GetBundleManagerHelper();
    if (bundleMgrHelper == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "bundleMgrHelper null");
        return ERR_NO_INIT;
    }

    TAG_LOGD(AAFwkTag::APPMGR, "userId: %{public}d, bundleName: %{public}s, appCloneIndex: %{public}d",
        userId, bundleName.c_str(), appCloneIndex);
    int uid = IN_PROCESS_CALL(bundleMgrHelper->GetUidByBundleName(bundleName, userId, appCloneIndex));
    TAG_LOGI(AAFwkTag::APPMGR, "uID value: %{public}d", uid);
    if (!appRunningManager_->ProcessExitByBundleNameAndUid(bundleName, uid, pids, config)) {
        TAG_LOGI(AAFwkTag::APPMGR, "process corresponding package name unstart");
        return ERR_OK;
    }
    return WaitProcessesExitAndKill(pids, startTime, "KillApplicationByUserId");
}

int32_t AppMgrServiceInner::ClearUpApplicationData(const std::string &bundleName,
    int32_t callerUid, pid_t callerPid, int32_t appCloneIndex, int32_t userId)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    int32_t newUserId = userId;
    if (userId == DEFAULT_INVAL_VALUE) {
        newUserId = GetUserIdByUid(callerUid);
        if (newUserId == U0_USER_ID) {
            newUserId = currentUserId_;
        }
    }
    TAG_LOGI(AAFwkTag::APPMGR, "bundleName: %{public}s, uId: %{public}d, appIndex: %{public}d", bundleName.c_str(),
        newUserId, appCloneIndex);
    return ClearUpApplicationDataByUserId(bundleName, callerUid, callerPid, appCloneIndex, newUserId,
        false, "ClearUpApplicationData");
}

int32_t AppMgrServiceInner::ClearUpApplicationDataBySelf(int32_t callerUid, pid_t callerPid, int32_t userId)
{
    auto appRecord = GetAppRunningRecordByPid(callerPid);
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "no appRecord, callerPid:%{public}d", callerPid);
        return ERR_INVALID_VALUE;
    }
    auto callerBundleName = appRecord->GetBundleName();
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    int32_t newUserId = userId;
    if (userId == DEFAULT_INVAL_VALUE) {
        newUserId = GetUserIdByUid(callerUid);
        if (newUserId == U0_USER_ID) {
            newUserId = currentUserId_;
        }
    }
    auto appCloneIndex = appRecord->GetAppIndex();
    return ClearUpApplicationDataByUserId(callerBundleName, callerUid, callerPid, appCloneIndex, newUserId, true,
        "ClearUpApplicationDataBySelf");
}

int32_t AppMgrServiceInner::ClearUpApplicationDataByUserId(const std::string &bundleName, int32_t callerUid,
    pid_t callerPid, int32_t appCloneIndex, int32_t userId, bool isBySelf, const std::string& reason)
{
    if (callerPid <= 0) {
        TAG_LOGE(AAFwkTag::APPMGR, "invalid callerPid:%{public}d", callerPid);
        return ERR_INVALID_OPERATION;
    }
    if (callerUid < 0) {
        TAG_LOGE(AAFwkTag::APPMGR, "invalid callerUid:%{public}d", callerUid);
        return ERR_INVALID_OPERATION;
    }
    auto bundleMgrHelper = remoteClientManager_->GetBundleManagerHelper();
    if (bundleMgrHelper == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "bundleMgrHelper null");
        return ERR_INVALID_OPERATION;
    }

    // request to clear user information permission.
    TAG_LOGD(AAFwkTag::APPMGR, "userId: %{public}d, bundleName: %{public}s, appCloneIndex: %{public}d",
        userId, bundleName.c_str(), appCloneIndex);
    auto tokenId = AccessToken::AccessTokenKit::GetHapTokenID(userId, bundleName, appCloneIndex);
    int32_t result = AccessToken::AccessTokenKit::ClearUserGrantedPermissionState(tokenId);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "clearUserGrantedPermissionState fail, ret:%{public}d", result);
        return AAFwk::ERR_APP_CLONE_INDEX_INVALID;
    }
    // 2.delete bundle side user data
    if (!IN_PROCESS_CALL(bundleMgrHelper->CleanBundleDataFiles(bundleName, userId, appCloneIndex))) {
        TAG_LOGE(AAFwkTag::APPMGR, "delete user data fail");
        return AAFwk::ERR_APP_CLONE_INDEX_INVALID;
    }
    // 3.kill application
    // 4.revoke user rights
    result =
        isBySelf ? KillApplicationSelf(false, reason)
            : KillApplicationByUserId(bundleName, appCloneIndex, userId, false, reason);
    if (result < 0) {
        TAG_LOGE(AAFwkTag::APPMGR, "kill app fail");
        return ERR_INVALID_OPERATION;
    }
    // 5.revoke uri permission rights
#ifdef SUPPORT_UPMS
    auto ret = IN_PROCESS_CALL(AAFwk::UriPermissionManagerClient::GetInstance().RevokeAllUriPermissions(tokenId));
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "revoke all uri permissions fail");
    }
#endif // SUPPORT_UPMS
    auto dataMgr = OHOS::DistributedKv::DistributedDataMgr();
    auto dataRet = dataMgr.ClearAppStorage(bundleName, userId, appCloneIndex, tokenId);
    if (dataRet != 0) {
        TAG_LOGW(
            AAFwkTag::APPMGR, "fail, bundleName:%{public}s", bundleName.c_str());
    }
    int targetUid = IN_PROCESS_CALL(bundleMgrHelper->GetUidByBundleName(bundleName, userId, appCloneIndex));
    NotifyAppStatusByCallerUid(bundleName, tokenId, userId, callerUid, targetUid,
        EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_DATA_CLEARED);
    TAG_LOGI(AAFwkTag::APPMGR, "clear");
    return ERR_OK;
}

int32_t AppMgrServiceInner::GetAllRunningProcesses(std::vector<RunningProcessInfo> &info)
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

int32_t AppMgrServiceInner::GetRunningProcessesByBundleType(BundleType bundleType,
    std::vector<RunningProcessInfo> &info)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    CHECK_CALLER_IS_SYSTEM_APP;
    if (!AAFwk::PermissionVerification::GetInstance()->VerifyRunningInfoPerm()) {
        TAG_LOGE(AAFwkTag::APPMGR, "permission deny");
        return ERR_PERMISSION_DENIED;
    }
    for (const auto &item : appRunningManager_->GetAppRunningRecordMap()) {
        const auto &appRecord = item.second;
        if (!appRecord || !appRecord->GetSpawned()) {
            continue;
        }
        if (GetUserIdByUid(appRecord->GetUid()) != currentUserId_) {
            continue;
        }
        auto appInfo = appRecord->GetApplicationInfo();
        if (appInfo && appInfo->bundleType == bundleType) {
            GetRunningProcesses(appRecord, info);
        }
    }
    return ERR_OK;
}

int32_t AppMgrServiceInner::GetRunningMultiAppInfoByBundleName(const std::string &bundleName,
    RunningMultiAppInfo &info)
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
    TAG_LOGD(AAFwkTag::APPMGR, "userId: %{public}d, bundleName: %{public}s", currentUserId_, bundleName.c_str());
    auto queryRet = IN_PROCESS_CALL(bundleMgrHelper->GetApplicationInfo(bundleName,
        ApplicationFlag::GET_BASIC_APPLICATION_INFO, currentUserId_, appInfo));
    if (!queryRet) {
        TAG_LOGE(AAFwkTag::APPMGR, "bundle unexist");
        return AAFwk::ERR_BUNDLE_NOT_EXIST;
    }
    if (appInfo.multiAppMode.multiAppModeType == MultiAppModeType::UNSPECIFIED) {
        TAG_LOGE(AAFwkTag::APPMGR, "bundle unsupport multi-app");
        return AAFwk::ERR_MULTI_APP_NOT_SUPPORTED;
    }
    info.bundleName = bundleName;
    info.mode = static_cast<int32_t>(appInfo.multiAppMode.multiAppModeType);
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
        if (GetUserIdByUid(appRecord->GetUid()) != currentUserId_) {
            continue;
        }
#ifdef SUPPORT_CHILD_PROCESS
        GetRunningCloneAppInfo(appRecord, info);
#endif // SUPPORT_CHILD_PROCESS
    }
    return ERR_OK;
}

int32_t AppMgrServiceInner::GetAllRunningInstanceKeysBySelf(std::vector<std::string> &instanceKeys)
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
    return GetAllRunningInstanceKeysByBundleNameInner(bundleName, instanceKeys, currentUserId_);
}

int32_t AppMgrServiceInner::GetAllRunningInstanceKeysByBundleName(const std::string &bundleName,
    std::vector<std::string> &instanceKeys, int32_t userId)
{
    if (userId == -1) {
        userId = currentUserId_;
    }
    if (VerifyAccountPermission(AAFwk::PermissionConstants::PERMISSION_GET_RUNNING_INFO, userId) ==
        ERR_PERMISSION_DENIED) {
        TAG_LOGE(AAFwkTag::APPMGR, "%{public}s: Permission verification fail", __func__);
        return ERR_PERMISSION_DENIED;
    }
    return GetAllRunningInstanceKeysByBundleNameInner(bundleName, instanceKeys, userId);
}

int32_t AppMgrServiceInner::GetAllRunningInstanceKeysByBundleNameInner(const std::string &bundleName,
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

#ifdef SUPPORT_CHILD_PROCESS
void AppMgrServiceInner::GetRunningCloneAppInfo(const std::shared_ptr<AppRunningRecord> &appRecord,
    RunningMultiAppInfo &info)
{
    if (info.mode == static_cast<int32_t>(MultiAppModeType::APP_CLONE)) {
        GetAppCloneInfo(appRecord, info);
        return;
    }
    if (info.mode == static_cast<int32_t>(MultiAppModeType::MULTI_INSTANCE)) {
        GetMultiInstanceInfo(appRecord, info);
    }
}
#endif // SUPPORT_CHILD_PROCESS

bool AppMgrServiceInner::CheckAppRecordAndPriorityObject(const std::shared_ptr<AppRunningRecord> &appRecord)
{
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRecord null");
        return false;
    }
    if (appRecord->GetPriorityObject() == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "priorityObject null");
        return false;
    }
    return true;
}

#ifdef SUPPORT_CHILD_PROCESS
void AppMgrServiceInner::GetAppCloneInfo(const std::shared_ptr<AppRunningRecord> &appRecord,
    RunningMultiAppInfo &info)
{
    if (!CheckAppRecordAndPriorityObject(appRecord)) {
        return;
    }
    auto PriorityObject = appRecord->GetPriorityObject();
    size_t index = 0;
    for (; index < info.runningAppClones.size(); index++) {
        if (info.runningAppClones[index].appCloneIndex == appRecord->GetAppIndex()) {
            break;
        }
    }
    auto childProcessRecordMap = appRecord->GetChildProcessRecordMap();
    if (index < info.runningAppClones.size()) {
        info.runningAppClones[index].pids.emplace_back(PriorityObject->GetPid());
        for (auto it : childProcessRecordMap) {
            info.runningAppClones[index].pids.emplace_back(it.first);
        }
        return;
    }
    RunningAppClone cloneInfo;
    cloneInfo.appCloneIndex = appRecord->GetAppIndex();
    cloneInfo.uid = appRecord->GetUid();
    cloneInfo.pids.emplace_back(PriorityObject->GetPid());
    for (auto it : childProcessRecordMap) {
        cloneInfo.pids.emplace_back(it.first);
    }
    info.runningAppClones.emplace_back(cloneInfo);
}

void AppMgrServiceInner::GetMultiInstanceInfo(const std::shared_ptr<AppRunningRecord> &appRecord,
    RunningMultiAppInfo &info)
{
    if (!CheckAppRecordAndPriorityObject(appRecord)) {
        return;
    }
    auto PriorityObject = appRecord->GetPriorityObject();
    size_t index = 0;
    for (; index < info.runningMultiIntanceInfos.size(); index++) {
        if (info.runningMultiIntanceInfos[index].instanceKey == appRecord->GetInstanceKey()) {
            break;
        }
    }
    auto childProcessRecordMap = appRecord->GetChildProcessRecordMap();
    if (index < info.runningMultiIntanceInfos.size()) {
        info.runningMultiIntanceInfos[index].pids.emplace_back(PriorityObject->GetPid());
        for (auto it : childProcessRecordMap) {
            info.runningMultiIntanceInfos[index].pids.emplace_back(it.first);
        }
        return;
    }
    RunningMultiInstanceInfo instanceInfo;
    instanceInfo.instanceKey = appRecord->GetInstanceKey();
    instanceInfo.uid = appRecord->GetUid();
    instanceInfo.pids.emplace_back(PriorityObject->GetPid());
    for (auto it : childProcessRecordMap) {
        instanceInfo.pids.emplace_back(it.first);
    }
    info.runningMultiIntanceInfos.emplace_back(instanceInfo);
}
#endif // SUPPORT_CHILD_PROCESS

void AppMgrServiceInner::GetRunningMultiInstanceKeys(const std::shared_ptr<AppRunningRecord> &appRecord,
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

int32_t AppMgrServiceInner::GetProcessRunningInfosByUserId(std::vector<RunningProcessInfo> &info, int32_t userId)
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

int32_t AppMgrServiceInner::GetProcessRunningInformation(RunningProcessInfo &info)
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

int32_t AppMgrServiceInner::GetAllRenderProcesses(std::vector<RenderProcessInfo> &info)
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
int AppMgrServiceInner::GetAllChildrenProcesses(std::vector<ChildProcessInfo> &info)
{
    auto isPerm = AAFwk::PermissionVerification::GetInstance()->VerifyRunningInfoPerm();
    // check permission
    for (const auto &item : appRunningManager_->GetAppRunningRecordMap()) {
        const auto &appRecord = item.second;
        if (isPerm) {
            GetChildrenProcesses(appRecord, info);
        } else {
            auto applicationInfo = appRecord->GetApplicationInfo();
            if (!applicationInfo) {
                continue;
            }
            auto callingTokenId = IPCSkeleton::GetCallingTokenID();
            auto tokenId = applicationInfo->accessTokenId;
            if (callingTokenId == tokenId) {
                GetChildrenProcesses(appRecord, info);
            }
        }
    }
    return ERR_OK;
}
#endif // SUPPORT_CHILD_PROCESS

int32_t AppMgrServiceInner::NotifyMemoryLevel(int32_t level)
{
    TAG_LOGI(AAFwkTag::APPMGR, "start");

    bool isMemmgrCall = AAFwk::PermissionVerification::GetInstance()->CheckSpecificSystemAbilityAccessPermission(
        MEMMGR_PROC_NAME);
    if (!isMemmgrCall) {
        TAG_LOGE(AAFwkTag::APPMGR, "callerToken not %{public}s", MEMMGR_PROC_NAME);
        return ERR_INVALID_VALUE;
    }
    if (!(level == OHOS::AppExecFwk::MemoryLevel::MEMORY_LEVEL_MODERATE ||
        level == OHOS::AppExecFwk::MemoryLevel::MEMORY_LEVEL_CRITICAL ||
        level == OHOS::AppExecFwk::MemoryLevel::MEMORY_LEVEL_LOW)) {
        TAG_LOGE(AAFwkTag::APPMGR, "level value error");
        return ERR_INVALID_VALUE;
    }
    if (!appRunningManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRunningManager null");
        return ERR_INVALID_VALUE;
    }

    return appRunningManager_->NotifyMemoryLevel(level);
}

int32_t AppMgrServiceInner::NotifyProcMemoryLevel(const std::map<pid_t, MemoryLevel> &procLevelMap)
{
    TAG_LOGI(AAFwkTag::APPMGR, "start");

    bool isMemmgrCall = AAFwk::PermissionVerification::GetInstance()->CheckSpecificSystemAbilityAccessPermission(
        MEMMGR_PROC_NAME);
    auto isShellCall = AAFwk::PermissionVerification::GetInstance()->IsShellCall();
    bool isDevelopMode = system::GetBoolParameter(DEVELOPER_MODE_STATE, false);
    if (!(isMemmgrCall || (isShellCall && isDevelopMode))) {
        TAG_LOGE(AAFwkTag::APPMGR, "Permission check failed: callerToken is not %{public}s, isMemmgrCall=%{public}d, "
        "isShellCall=%{public}d, isDevelopMode=%{public}d", MEMMGR_PROC_NAME, isMemmgrCall, isShellCall, isDevelopMode);
        return ERR_INVALID_VALUE;
    }
    if (!appRunningManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRunningManager null");
        return ERR_INVALID_VALUE;
    }

    return appRunningManager_->NotifyProcMemoryLevel(procLevelMap);
}

int32_t AppMgrServiceInner::DumpHeapMemory(const int32_t pid, OHOS::AppExecFwk::MallocInfo &mallocInfo)
{
    auto isSaCall = AAFwk::PermissionVerification::GetInstance()->IsSACall();
    if (!isSaCall) {
        TAG_LOGE(AAFwkTag::APPMGR, "callerToken not SA");
        return ERR_INVALID_VALUE;
    }
    if (pid < 0) {
        TAG_LOGE(AAFwkTag::APPMGR, "pid illegal");
        return ERR_INVALID_VALUE;
    }
    if (!appRunningManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRunningManager null");
        return ERR_INVALID_VALUE;
    }
    return appRunningManager_->DumpHeapMemory(pid, mallocInfo);
}

int32_t AppMgrServiceInner::DumpJsHeapMemory(OHOS::AppExecFwk::JsHeapDumpInfo &info)
{
    auto isSaCall = AAFwk::PermissionVerification::GetInstance()->IsSACall();
    if (!isSaCall) {
        TAG_LOGE(AAFwkTag::APPMGR, "callerToken not SA");
        return ERR_INVALID_VALUE;
    }
    if (info.pid == 0) {
        TAG_LOGE(AAFwkTag::APPMGR, "pid illegal");
        return ERR_INVALID_VALUE;
    }
    if (!appRunningManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRunningManager null");
        return ERR_INVALID_VALUE;
    }
    return appRunningManager_->DumpJsHeapMemory(info);
}

void AppMgrServiceInner::GetRunningProcesses(const std::shared_ptr<AppRunningRecord> &appRecord,
    std::vector<RunningProcessInfo> &info)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    RunningProcessInfo runningProcessInfo;
    GetRunningProcess(appRecord, runningProcessInfo);
    info.emplace_back(runningProcessInfo);
}

void AppMgrServiceInner::GetRunningProcess(const std::shared_ptr<AppRunningRecord> &appRecord,
    RunningProcessInfo &info)
{
    CHECK_POINTER_AND_RETURN_LOG(appRecord, "appRecord unexist");
    info.processName_ = appRecord->GetProcessName();
    info.pid_ = appRecord->GetPid();
    info.uid_ = appRecord->GetUid();
    info.state_ = static_cast<AppProcessState>(appRecord->GetState());
    info.isContinuousTask = appRecord->IsContinuousTask();
    info.isKeepAlive = appRecord->IsKeepAliveApp();
    info.isFocused = appRecord->GetFocusFlag();
    info.startTimeMillis_ = appRecord->GetAppStartTime();
    appRecord->GetBundleNames(info.bundleNames);
    info.processType_ = appRecord->GetProcessType();
    info.extensionType_ = appRecord->GetExtensionType();
    info.preloadMode_ = appRecord->GetPreloadMode();
    info.isDebugApp  = appRecord->IsDebug();
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
}

void AppMgrServiceInner::GetRenderProcesses(const std::shared_ptr<AppRunningRecord> &appRecord,
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
void AppMgrServiceInner::GetChildrenProcesses(const std::shared_ptr<AppRunningRecord> &appRecord,
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
            retCode = GetChildProcessInfo(childProcessRecord, appRecord, childProcessInfo, true);
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

int32_t AppMgrServiceInner::KillProcessByPid(const pid_t pid, const std::string& reason)
{
    if (!ProcessUtil::ProcessExist(pid)) {
        TAG_LOGI(AAFwkTag::APPMGR, "null killProcessByPid, pid: %{public}d", pid);
        return AAFwk::ERR_KILL_PROCESS_NOT_EXIST;
    }
    std::string killReason = KILL_PROCESS_REASON_PREFIX + reason + ",callingPid=" +
        std::to_string(IPCSkeleton::GetCallingPid());
    auto appRecord = GetAppRunningRecordByPid(pid);
    if (appRecord && appRecord->GetExitReason() == EXIT_REASON_UNKNOWN) {
        appRecord->SetExitMsg(killReason);
    }
    TAG_LOGI(AAFwkTag::APPMGR, "kill reason=%{public}s, pid=%{public}d", reason.c_str(), pid);
    return KillProcessByPidInner(pid, reason, killReason, appRecord);
}

int32_t AppMgrServiceInner::KillProcessByPidInner(const pid_t pid, const std::string& reason,
    const std::string& killReason, std::shared_ptr<AppRunningRecord> appRecord)
{
    int32_t ret = -1;
    if (pid > 0) {
        if (CheckIsThreadInFoundation(pid)) {
            TAG_LOGI(AAFwkTag::APPMGR, "don't kill pid %{public}d", pid);
            return AAFwk::ERR_KILL_FOUNDATION_UID;
        }
        ret = kill(pid, SIGNAL_KILL);
        if (reason == "OnRemoteDied") {
            TAG_LOGW(AAFwkTag::APPMGR, "application is dead, double check, pid=%{public}d", pid);
        } else {
            TAG_LOGW(AAFwkTag::APPMGR, "kill pid %{public}d, ret:%{public}d, %{public}s",
                pid, ret, killReason.c_str());
        }
    }
    AAFwk::EventInfo eventInfo;
    if (!appRecord) {
        return ret;
    }
    appRecord->SetKillReason(reason);
    auto applicationInfo = appRecord->GetApplicationInfo();
    if (!applicationInfo) {
        TAG_LOGE(AAFwkTag::APPMGR, "appInfo null");
    } else {
        eventInfo.bundleName = applicationInfo->name;
        eventInfo.versionName = applicationInfo->versionName;
        eventInfo.versionCode = applicationInfo->versionCode;
    }
    if (ret >= 0) {
        std::lock_guard lock(killedProcessMapLock_);
        int64_t killTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::
            system_clock::now().time_since_epoch()).count();
        killedProcessMap_.emplace(killTime, appRecord->GetProcessName());
    }
    DelayedSingleton<CacheProcessManager>::GetInstance()->OnProcessKilled(appRecord);
    eventInfo.pid = appRecord->GetPid();
    eventInfo.processName = appRecord->GetProcessName();
    std::string newReason = appRecord->GetKillReason().empty() ? killReason : appRecord->GetKillReason();
    bool foreground = appRecord->GetState() == ApplicationState::APP_STATE_FOREGROUND ||
        appRecord->GetState() == ApplicationState::APP_STATE_FOCUS;
    AAFwk::EventReport::SendAppEvent(AAFwk::EventName::APP_TERMINATE, HiSysEventType::BEHAVIOR, eventInfo);
    int result = HiSysEventWrite(OHOS::HiviewDFX::HiSysEvent::Domain::FRAMEWORK, "PROCESS_KILL",
        OHOS::HiviewDFX::HiSysEvent::EventType::FAULT, EVENT_KEY_PID, std::to_string(eventInfo.pid),
        EVENT_KEY_PROCESS_NAME, eventInfo.processName, EVENT_KEY_MESSAGE, newReason,
        EVENT_KEY_FOREGROUND, foreground);
    TAG_LOGW(AAFwkTag::APPMGR, "hisysevent write result=%{public}d, send event [FRAMEWORK,PROCESS_KILL], pid="
        "%{public}d, processName=%{public}s, msg=%{public}s, FOREGROUND=%{public}d, ret=%{public}d",
        result, pid, eventInfo.processName.c_str(), newReason.c_str(), foreground, ret);
    return ret;
}

bool AppMgrServiceInner::CheckIsThreadInFoundation(pid_t pid)
{
    std::ostringstream pathBuilder;
    pathBuilder << PROC_SELF_TASK_PATH << pid;
    std::string path = pathBuilder.str();
    TAG_LOGD(AAFwkTag::APPMGR, "CheckIsThreadInFoundation path:%{public}s", path.c_str());
    return access(path.c_str(), F_OK) == 0;
}

bool AppMgrServiceInner::WaitForRemoteProcessExit(std::list<pid_t> &pids, const int64_t startTime)
{
    int64_t delayTime = SystemTimeMillisecond() - startTime;
    if (ProcessUtil::CheckAllProcessExit(pids)) {
        return true;
    }
    while (delayTime < KILL_PROCESS_TIMEOUT_MICRO_SECONDS) {
        usleep(KILL_PROCESS_DELAYTIME_MICRO_SECONDS);
        if (ProcessUtil::CheckAllProcessExit(pids)) {
            return true;
        }
        delayTime = SystemTimeMillisecond() - startTime;
    }
    return false;
}

bool AppMgrServiceInner::WaitForRemoteProcessExit(std::list<SimpleProcessInfo> &processInfos, const int64_t startTime)
{
    int64_t delayTime = SystemTimeMillisecond() - startTime;
    if (ProcessUtil::CheckAllProcessExit(processInfos)) {
        return true;
    }
    while (delayTime < KILL_PROCESS_TIMEOUT_MICRO_SECONDS) {
        usleep(KILL_PROCESS_DELAYTIME_MICRO_SECONDS);
        if (ProcessUtil::CheckAllProcessExit(processInfos)) {
            return true;
        }
        delayTime = SystemTimeMillisecond() - startTime;
    }
    return false;
}

int64_t AppMgrServiceInner::SystemTimeMillisecond()
{
    struct timespec t;
    t.tv_sec = 0;
    t.tv_nsec = 0;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return (int64_t)((t.tv_sec) * NANOSECONDS + t.tv_nsec) / MICROSECONDS;
}

std::shared_ptr<AppRunningRecord> AppMgrServiceInner::GetAppRunningRecordByPid(const pid_t pid) const
{
    if (!appRunningManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRunningManager null");
        return nullptr;
    }
    return appRunningManager_->GetAppRunningRecordByPid(pid);
}

std::shared_ptr<AppRunningRecord> AppMgrServiceInner::CreateAppRunningRecord(
    std::shared_ptr<AbilityRuntime::LoadParam> loadParam, std::shared_ptr<ApplicationInfo> appInfo,
    std::shared_ptr<AbilityInfo> abilityInfo, const std::string &processName, const BundleInfo &bundleInfo,
    const HapModuleInfo &hapModuleInfo, std::shared_ptr<AAFwk::Want> want, bool isKia)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    if (want != nullptr && (want->GetBoolParam(DEBUG_APP, false) || want->GetBoolParam(NATIVE_DEBUG, false) ||
        want->GetBoolParam(DEBUG_FROM, false))) {
        if (appInfo != nullptr && appInfo->appProvisionType != AppExecFwk::Constants::APP_PROVISION_TYPE_DEBUG) {
            TAG_LOGE(AAFwkTag::APPMGR, "release app not support debug");
            return nullptr;
        }
    }
    if (!appRunningManager_ || loadParam == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRunningManager or loadParam null");
        return nullptr;
    }
    auto appRecord = appRunningManager_->CreateAppRunningRecord(appInfo, processName, bundleInfo,
        loadParam->instanceKey, abilityInfo ? abilityInfo->process : "");
    CHECK_POINTER_AND_RETURN_VALUE(appRecord, nullptr);
    appRecord->SetProcessAndExtensionType(abilityInfo, loadParam->extensionProcessMode);
    appRecord->SetKeepAliveEnableState(bundleInfo.isKeepAlive);
    appRecord->SetKeepAliveDkv(loadParam->isKeepAlive);
    appRecord->SetEmptyKeepAliveAppState(false);
    appRecord->SetTaskHandler(taskHandler_);
    appRecord->SetEventHandler(eventHandler_);
    appRecord->AddModule(appInfo, abilityInfo, loadParam->token, hapModuleInfo, want, loadParam->abilityRecordId);
    appRecord->SetIsKia(isKia);
    SetAppRunningRecordStrictMode(appRecord, loadParam);
    if (want) {
        appRecord->SetDebugApp(want->GetBoolParam(DEBUG_APP, false));
        appRecord->SetNativeDebug(want->GetBoolParam("nativeDebug", false));
        appRecord->SetDebugFromLocal(want->GetBoolParam(DEBUG_FROM, false));
        if (want->GetBoolParam(COLD_START, false)) {
            appRecord->SetDebugApp(true);
        }
        appRecord->SetPerfCmd(want->GetStringParam(PERF_CMD));
        appRecord->SetErrorInfoEnhance(want->GetBoolParam(ERROR_INFO_ENHANCE, false));
        appRecord->SetMultiThread(want->GetBoolParam(MULTI_THREAD, false));
        int32_t appIndex = 0;
        (void)AbilityRuntime::StartupUtil::GetAppIndex(*want, appIndex);
        appRecord->SetAppIndex(appIndex);
#ifdef WITH_DLP
        appRecord->SetSecurityFlag(want->GetBoolParam(DLP_PARAMS_SECURITY_FLAG, false));
#endif // WITH_DLP
        appRecord->SetRequestProcCode(want->GetIntParam(Want::PARAM_RESV_REQUEST_PROC_CODE, 0));
        appRecord->SetCallerPid(want->GetIntParam(Want::PARAM_RESV_CALLER_PID, -1));
        appRecord->SetCallerUid(want->GetIntParam(Want::PARAM_RESV_CALLER_UID, -1));
        appRecord->SetCallerTokenId(want->GetIntParam(Want::PARAM_RESV_CALLER_TOKEN, -1));
        appRecord->SetAssignTokenId(want->GetIntParam("specifyTokenId", 0));
        appRecord->SetNativeStart(want->GetBoolParam("native", false));
        appRecord->SetDebugFromLocal(want->GetBoolParam(DEBUG_FROM, false));
    }
    return appRecord;
}

void AppMgrServiceInner::SetAppRunningRecordStrictMode(std::shared_ptr<AppRunningRecord> appRecord,
    std::shared_ptr<AbilityRuntime::LoadParam> loadParam)
{
    CHECK_POINTER_AND_RETURN_LOG(appRecord, "appRecord is null");
    CHECK_POINTER_AND_RETURN_LOG(loadParam, "loadParam is null");
    appRecord->SetStrictMode(loadParam->extensionLoadParam.strictMode);
    appRecord->SetNetworkEnableFlags(loadParam->extensionLoadParam.networkEnableFlags);
    appRecord->SetSAEnableFlags(loadParam->extensionLoadParam.saEnableFlags);
}

void AppMgrServiceInner::TerminateAbility(const sptr<IRemoteObject> &token, bool clearMissionFlag)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::APPMGR, "Terminate ability come.");
    if (!token) {
        TAG_LOGE(AAFwkTag::APPMGR, "terminateAbility token null");
        return;
    }
    auto appRecord = GetAppRunningRecordByAbilityToken(token);
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "terminateAbility app unexist");
        return;
    }

    RemoveUIExtensionBindItem(appRecord, token);
    RemoveUIExtensionLauncherItem(appRecord, token);

    if (appRunningManager_) {
        std::shared_ptr<AppMgrServiceInner> appMgrServiceInner = shared_from_this();
        appRunningManager_->TerminateAbility(token, clearMissionFlag, appMgrServiceInner);
        if (appRecord->IsTerminating() && appRunningManager_->CheckAppRunningRecordIsLast(appRecord)) {
            TAG_LOGD(AAFwkTag::APPMGR, "clear uri permission");
            appRecord->UnSetPolicy();
        }
    }
}

void AppMgrServiceInner::UpdateAbilityState(const sptr<IRemoteObject> &token, const AbilityState state)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::APPMGR, "state %{public}d.", static_cast<int32_t>(state));
    if (!token) {
        TAG_LOGE(AAFwkTag::APPMGR, "token null");
        return;
    }

    if (state == AbilityState::ABILITY_STATE_FOREGROUND) {
        AbilityRuntime::FreezeUtil::GetInstance().AppendLifecycleEvent(token, "ServiceInner::UpdateAbilityState");
    }
    auto appRecord = GetAppRunningRecordByAbilityToken(token);
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "app unexist");
        return;
    }
    auto abilityRecord = appRecord->GetAbilityRunningRecordByToken(token);
    if (!abilityRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "can't find record");
        return;
    }
    if (state == abilityRecord->GetState()) {
        TAG_LOGE(AAFwkTag::APPMGR, "current state is already");
        return;
    }
    if (abilityRecord->GetAbilityInfo() == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "info null");
        return;
    }
    auto type = abilityRecord->GetAbilityInfo()->type;
    if (type == AppExecFwk::AbilityType::SERVICE &&
        (state == AbilityState::ABILITY_STATE_CREATE ||
        state == AbilityState::ABILITY_STATE_TERMINATED ||
        state == AbilityState::ABILITY_STATE_CONNECTED ||
        state == AbilityState::ABILITY_STATE_DISCONNECTED)) {
        TAG_LOGI(
            AAFwkTag::APPMGR, "state:%{public}d", static_cast<int32_t>(state));
        appRecord->StateChangedNotifyObserver(abilityRecord, static_cast<int32_t>(state), true, false);
        return;
    }
    if (state > AbilityState::ABILITY_STATE_BACKGROUND || state < AbilityState::ABILITY_STATE_FOREGROUND) {
        TAG_LOGE(AAFwkTag::APPMGR, "state is not foreground or background");
        return;
    }

    appRecord->UpdateAbilityState(token, state);
    CheckCleanAbilityByUserRequest(appRecord, abilityRecord, state);
}

void AppMgrServiceInner::UpdateExtensionState(const sptr<IRemoteObject> &token, const ExtensionState state)
{
    if (!token) {
        TAG_LOGE(AAFwkTag::APPMGR, "token null");
        return;
    }
    auto appRecord = GetAppRunningRecordByAbilityToken(token);
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "app unexist");
        return;
    }
    auto abilityRecord = appRecord->GetAbilityRunningRecordByToken(token);
    if (!abilityRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "can't find record");
        return;
    }
    appRecord->StateChangedNotifyObserver(abilityRecord, static_cast<int32_t>(state), false, false);
}

void AppMgrServiceInner::OnStop()
{
    if (!appRunningManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRunningManager null");
        return;
    }

    appRunningManager_->ClearAppRunningRecordMap();
    CloseAppSpawnConnection();
}

ErrCode AppMgrServiceInner::OpenAppSpawnConnection()
{
    if (remoteClientManager_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "remoteClientManager_ null");
        return ERR_INVALID_VALUE;
    }

    if (remoteClientManager_->GetSpawnClient()) {
        return remoteClientManager_->GetSpawnClient()->OpenConnection();
    }
    return ERR_APPEXECFWK_BAD_APPSPAWN_CLIENT;
}

void AppMgrServiceInner::CloseAppSpawnConnection() const
{
    if (remoteClientManager_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "remoteClientManager_ null");
        return;
    }

    if (remoteClientManager_->GetSpawnClient()) {
        remoteClientManager_->GetSpawnClient()->CloseConnection();
    }
}

SpawnConnectionState AppMgrServiceInner::QueryAppSpawnConnectionState() const
{
    if (remoteClientManager_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "remoteClientManager_ null");
        return SpawnConnectionState::STATE_NOT_CONNECT;
    }

    if (remoteClientManager_->GetSpawnClient()) {
        return remoteClientManager_->GetSpawnClient()->QueryConnectionState();
    }
    return SpawnConnectionState::STATE_NOT_CONNECT;
}

void AppMgrServiceInner::SetAppSpawnClient(std::shared_ptr<AppSpawnClient> spawnClient)
{
    if (remoteClientManager_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "remoteClientManager_ null");
        return;
    }

    remoteClientManager_->SetSpawnClient(std::move(spawnClient));
}

void AppMgrServiceInner::SetBundleManagerHelper(const std::shared_ptr<BundleMgrHelper> &bundleMgrHelper)
{
    if (remoteClientManager_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "remoteClientManager_ null");
        return;
    }

    remoteClientManager_->SetBundleManagerHelper(bundleMgrHelper);
}

void AppMgrServiceInner::RegisterAppStateCallback(const sptr<IAppStateCallback>& callback)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    if (callback != nullptr) {
        std::lock_guard lock(appStateCallbacksLock_);
        TAG_LOGI(AAFwkTag::APPMGR, "RegisterAppStateCallback");
        appStateCallbacks_.push_back(
            AppStateCallbackWithUserId { callback, GetUserIdByUid(IPCSkeleton::GetCallingUid()) });
        auto remoteObjedct = callback->AsObject();
        if (remoteObjedct) {
            remoteObjedct->AddDeathRecipient(
                sptr<AppStateCallbackDeathRecipient>(new AppStateCallbackDeathRecipient(weak_from_this())));
        }
    }
}

void AppMgrServiceInner::RemoveDeadAppStateCallback(const wptr<IRemoteObject> &remote)
{
    auto remoteObject = remote.promote();
    if (remoteObject == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "remoteObject null");
        return;
    }

    std::lock_guard lock(appStateCallbacksLock_);
    for (auto it = appStateCallbacks_.begin(); it != appStateCallbacks_.end(); ++it) {
        auto callback = (*it).callback;
        if (callback && callback->AsObject() == remoteObject) {
            TAG_LOGI(AAFwkTag::APPMGR, "RemoveDeadAppStateCallback");
            appStateCallbacks_.erase(it);
            break;
        }
    }
}

void AppMgrServiceInner::KillProcessByAbilityToken(const sptr<IRemoteObject> &token)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    if (!token) {
        TAG_LOGE(AAFwkTag::APPMGR, "token null");
        return;
    }
    auto appRecord = GetAppRunningRecordByAbilityToken(token);
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRecord unexist");
        return;
    }

    // before exec ScheduleProcessSecurityExit return
    // The resident process won't let him die
    if (appRecord->IsKeepAliveApp() && IsMemorySizeSufficient()) {
        return;
    }

    pid_t pid = appRecord->GetPid();
    if (pid > 0) {
        std::list<pid_t> pids;
        pids.push_back(pid);
        appRecord->ScheduleProcessSecurityExit();
        if (!WaitForRemoteProcessExit(pids, SystemTimeMillisecond())) {
            int32_t result = KillProcessByPid(pid, "KillProcessByAbilityToken");
            if (result < 0) {
                TAG_LOGE(AAFwkTag::APPMGR, "killProcessByAbilityToken kill process fail");
                return;
            }
        }
    }
}

void AppMgrServiceInner::KillProcessesByUserId(int32_t userId, bool isNeedSendAppSpawnMsg,
    sptr<AAFwk::IUserCallback> callback)
{
    if (!appRunningManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRunningManager_ null");
        return;
    }
    int32_t result = ERR_OK;
    int64_t startTime = SystemTimeMillisecond();
    if (callback == nullptr) {
        std::list<pid_t> pids;
        if (!appRunningManager_->GetPidsByUserId(userId, pids)) {
            TAG_LOGI(AAFwkTag::APPMGR, "process corresponding uId unstart");
            if (isNeedSendAppSpawnMsg) {
                TAG_LOGI(AAFwkTag::APPMGR, "developer mode, send uninstall debug hap messages");
                SendAppSpawnUninstallDebugHapMsg(userId);
            }
            return;
        }
        result = WaitProcessesExitAndKill(pids, startTime, "KillProcessesByUserId");
    } else {
        // for logout user to callback
        std::list<SimpleProcessInfo> processInfos;
        if (!appRunningManager_->GetProcessInfosByUserId(userId, processInfos)) {
            TAG_LOGI(AAFwkTag::APPMGR, "process corresponding uId unstart");
            callback->OnLogoutUserDone(userId, ERR_OK);
            if (isNeedSendAppSpawnMsg) {
                TAG_LOGI(AAFwkTag::APPMGR, "developer mode, send uninstall debug hap messages");
                SendAppSpawnUninstallDebugHapMsg(userId);
            }
            return;
        }
        result = WaitProcessesExitAndKill(processInfos, startTime, "KillProcessesByUserId", userId, callback);
    }
    if (result == ERR_OK && isNeedSendAppSpawnMsg) {
        TAG_LOGI(AAFwkTag::APPMGR, "developer mode, send uninstall debug hap messages");
        SendAppSpawnUninstallDebugHapMsg(userId);
    }
}

int32_t AppMgrServiceInner::KillProcessesInBatch(const std::vector<int32_t> &pids)
{
    CHECK_CALLER_IS_SYSTEM_APP;
    if (!AAFwk::PermissionVerification::GetInstance()->VerifyCallingPermission(
        AAFwk::PermissionConstants::PERMISSION_KILL_APP_PROCESSES)) {
        TAG_LOGE(AAFwkTag::APPMGR, "verify permission failed.");
        return ERR_PERMISSION_DENIED;
    }
    if (!AAFwk::AppUtils::GetInstance().IsStartOptionsWithAnimation()) {
        TAG_LOGE(AAFwkTag::APPMGR, "not supported.");
        return AAFwk::ERR_CAPABILITY_NOT_SUPPORT;
    }
    for (const auto& pid: pids) {
        auto appRecord = GetAppRunningRecordByPid(pid);
        if (appRecord == nullptr) {
            TAG_LOGE(AAFwkTag::APPMGR, "appRecord null");
            continue;
        }
        std::string callerKey = std::to_string(pid) + ":" + std::to_string(appRecord->GetUid());
        KillingProcessManager::GetInstance().AddKillingCallerKey(callerKey);
        (void)KillProcessByPid(pid, "KillProcessesInBatch");
    }
    return ERR_OK;
}

int32_t AppMgrServiceInner::KillSubProcessBypidInner(const pid_t pid, const std::string &reason,
    AAFwk::EventInfo &eventInfo)
{
    int32_t ret = -1;
    if (!ProcessUtil::ProcessExist(pid) || pid <= 0) {
        TAG_LOGE(AAFwkTag::APPMGR, "invalid pid: %{public}d", pid);
        return AAFwk::ERR_KILL_PROCESS_NOT_EXIST;
    }
    if (CheckIsThreadInFoundation(pid)) {
        TAG_LOGI(AAFwkTag::APPMGR, "don't kill pid %{public}d", pid);
        return AAFwk::ERR_KILL_FOUNDATION_UID;
    }
    ret = kill(pid, SIGNAL_KILL);
    if (ret < 0) {
        TAG_LOGE(AAFwkTag::APPMGR, "Kill pid %{public}d err: %{public}d", pid, ret);
    }

    std::string killReason = KILL_SUB_PROCESS_REASON_PREFIX + reason;
    AAFwk::EventReport::SendAppEvent(AAFwk::EventName::APP_TERMINATE, HiSysEventType::BEHAVIOR, eventInfo);
    int result = HiSysEventWrite(OHOS::HiviewDFX::HiSysEvent::Domain::FRAMEWORK, "PROCESS_KILL",
        OHOS::HiviewDFX::HiSysEvent::EventType::FAULT, EVENT_KEY_PID, std::to_string(eventInfo.pid),
        EVENT_KEY_PROCESS_NAME, eventInfo.processName, EVENT_KEY_MESSAGE, killReason);
    TAG_LOGW(AAFwkTag::APPMGR, "hisysevent write result=%{public}d, send event [FRAMEWORK,PROCESS_KILL], pid="
        "%{public}d, processName=%{public}s, msg=%{public}s, ret=%{public}d",
        result, pid, eventInfo.processName.c_str(), killReason.c_str(), ret);
    return ret;
}

int32_t AppMgrServiceInner::KillSubProcessBypid(const pid_t pid, const std::string &reason)
{
    int32_t ret = ERR_OK;
    if (!appRunningManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRunningManager_ null");
        return ERR_INVALID_VALUE;
    }
    // get render record
    auto appRecord = appRunningManager_->GetAppRunningRecordByRenderPid(pid);
    if (appRecord) {
        auto renderRecord = appRecord->GetRenderRecordByPid(pid);
        if (renderRecord) {
            AAFwk::EventInfo renderEventInfo;
            renderEventInfo.pid = renderRecord->GetPid();
            renderEventInfo.processName = renderRecord->GetProcessName();
            return KillSubProcessBypidInner(pid, reason, renderEventInfo);
        }
    }
    // get child process
#ifdef SUPPORT_CHILD_PROCESS
    appRecord = appRunningManager_->GetAppRunningRecordByChildProcessPid(pid);
    if (appRecord) {
        auto childRecordMap = appRecord->GetChildProcessRecordMap();
        auto iter = childRecordMap.find(pid);
        if (iter != childRecordMap.end()) {
            auto childProcessRecord = iter->second;
            if (childProcessRecord) {
                AAFwk::EventInfo childEventInfo;
                childEventInfo.pid = childProcessRecord->GetPid();
                childEventInfo.processName = childProcessRecord->GetProcessName();
                return KillSubProcessBypidInner(pid, reason, childEventInfo);
            }
        }
    }
#endif
    // get child record process
    appRecord = appRunningManager_->GetAppRunningRecordByChildRecordPid(pid);
    if (appRecord) {
        auto childAppRecordMap = appRecord->GetChildAppRecordMap();
        auto it = childAppRecordMap.find(pid);
        if (it != childAppRecordMap.end()) {
            auto weakChildAppRecord = it->second;
            auto childAppRecord = weakChildAppRecord.lock();
            if (childAppRecord) {
                AAFwk::EventInfo childRecordEventInfo;
                childRecordEventInfo.pid = childAppRecord->GetPid();
                childRecordEventInfo.processName = childAppRecord->GetProcessName();
                return KillSubProcessBypidInner(pid, reason, childRecordEventInfo);
            }
        }
    }

    return ERR_OK;
}

int32_t AppMgrServiceInner::KillProcessesByPids(const std::vector<int32_t> &pids, const std::string &reason,
    bool subProcess)
{
    int32_t ret = ERR_OK;
    for (const auto& pid: pids) {
        auto appRecord = GetAppRunningRecordByPid(pid);
        if (appRecord != nullptr) {
            if (KillProcessByPid(pid, reason) < 0) {
                TAG_LOGW(AAFwkTag::APPMGR, "fail, pid:%{public}d", pid);
            }
            continue;
        }
        if (subProcess == false) {
            continue;
        }
        ret = KillSubProcessBypid(pid, reason);
        if (ret < 0) {
            TAG_LOGW(AAFwkTag::APPMGR, "KillSubProcessBypid fail, pid:%{public}d, ret:%{public}d", pid, ret);
            return ret;
        }
    }
    return ret;
}

void AppMgrServiceInner::AttachPidToParent(const sptr<IRemoteObject> &token, const sptr<IRemoteObject> &callerToken)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    auto appRecord = GetAppRunningRecordByAbilityToken(token);
    if (appRecord == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "abilityRecord null");
        return;
    }
    auto pid = appRecord->GetPid();
    if (pid <= 0) {
        TAG_LOGE(AAFwkTag::APPMGR, "invalid pid");
        return;
    }
    auto callRecord = GetAppRunningRecordByAbilityToken(callerToken);
    if (callRecord == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "callRecord null");
        auto result = KillProcessByPid(pid, "AttachPidToParent");
        if (result < 0) {
            TAG_LOGW(AAFwkTag::APPMGR, "fail, pid: %{public}d", pid);
        }
        return;
    }
    appRecord->SetParentAppRecord(callRecord);
    callRecord->AddChildAppRecord(pid, appRecord);
}

void AppMgrServiceInner::StartAbility(sptr<IRemoteObject> token, sptr<IRemoteObject> preToken,
    std::shared_ptr<AbilityInfo> abilityInfo, std::shared_ptr<AppRunningRecord> appRecord,
    const HapModuleInfo &hapModuleInfo, std::shared_ptr<AAFwk::Want> want, int32_t abilityRecordId)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    CHECK_POINTER_AND_RETURN_LOG(abilityInfo, "abilityInfo null");
    TAG_LOGI(AAFwkTag::APPMGR, "start ability, %{public}s-%{public}s",
        abilityInfo->bundleName.c_str(), abilityInfo->name.c_str());
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRecord null");
        return;
    }

    if (want) {
#ifdef WITH_DLP
        want->SetParam(DLP_PARAMS_SECURITY_FLAG, appRecord->GetSecurityFlag());
#endif // WITH_DLP

        auto isDebugApp = want->GetBoolParam(DEBUG_APP, false);
        if (isDebugApp && !appRecord->IsDebugApp()) {
            ProcessAppDebug(appRecord, isDebugApp);
        }
    }

    auto ability = appRecord->GetAbilityRunningRecordByToken(token);
    if (abilityInfo->launchMode == LaunchMode::SINGLETON && ability != nullptr) {
        TAG_LOGW(AAFwkTag::APPMGR, "can't add same ability");
        return;
    }

    if (ability && preToken) {
        TAG_LOGE(AAFwkTag::APPMGR, "ability already start");
        return;
    }

    auto appInfo = std::make_shared<ApplicationInfo>(abilityInfo->applicationInfo);
    appRecord->AddModule(appInfo, abilityInfo, token, hapModuleInfo, want, abilityRecordId);
    auto moduleRecord = appRecord->GetModuleRecordByModuleName(appInfo->bundleName, hapModuleInfo.moduleName);
    if (!moduleRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "add moduleRecord fail");
        return;
    }

    ability = moduleRecord->GetAbilityRunningRecordByToken(token);
    if (!ability) {
        TAG_LOGE(AAFwkTag::APPMGR, "add ability fail");
        return;
    }

    ApplicationState appState = appRecord->GetState();
    if (appState == ApplicationState::APP_STATE_CREATE) {
        TAG_LOGE(AAFwkTag::APPMGR, "don't launch ability, bundleName:%{public}s, ability:%{public}s",
            appInfo->bundleName.c_str(), abilityInfo->name.c_str());
        return;
    }
    appRecord->LaunchAbility(ability);
}

std::shared_ptr<AppRunningRecord> AppMgrServiceInner::GetAppRunningRecordByAbilityToken(
    const sptr<IRemoteObject> &abilityToken) const
{
    if (!appRunningManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRunningManager_ null");
        return nullptr;
    }

    return appRunningManager_->GetAppRunningRecordByAbilityToken(abilityToken);
}

std::shared_ptr<AppRunningRecord> AppMgrServiceInner::GetTerminatingAppRunningRecord(
    const sptr<IRemoteObject> &token) const
{
    if (!appRunningManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRunningManager_ null");
        return nullptr;
    }
    return appRunningManager_->GetTerminatingAppRunningRecord(token);
}

void AppMgrServiceInner::AbilityTerminated(const sptr<IRemoteObject> &token)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::APPMGR, "Terminate ability come.");
    if (!token) {
        TAG_LOGE(AAFwkTag::APPMGR, "token null");
        return;
    }

    auto appRecord = appRunningManager_->GetTerminatingAppRunningRecord(token);
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRecord unexist");
        return;
    }

    appRecord->AbilityTerminated(token);
}

std::shared_ptr<AppRunningRecord> AppMgrServiceInner::GetAppRunningRecordByAppRecordId(const int32_t recordId) const
{
    if (appRunningManager_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRunningManager null");
        return nullptr;
    }
    const auto&& appRunningRecordMap = appRunningManager_->GetAppRunningRecordMap();
    const auto& iter = appRunningRecordMap.find(recordId);
    return iter != appRunningRecordMap.end() ? iter->second : nullptr;
}

void AppMgrServiceInner::OnAppStateChanged(
    const std::shared_ptr<AppRunningRecord> &appRecord,
    const ApplicationState state,
    bool needNotifyApp,
    bool isFromWindowFocusChanged)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRecord null");
        return;
    }

    TAG_LOGD(AAFwkTag::APPMGR, "OnAppStateChanged begin, bundleName is %{public}s, state:%{public}d",
        appRecord->GetBundleName().c_str(), static_cast<int32_t>(state));
    {
        std::lock_guard lock(appStateCallbacksLock_);
        for (const auto &item : appStateCallbacks_) {
            if (item.callback != nullptr) {
                item.callback->OnAppStateChanged(WrapAppProcessData(appRecord, state));
            }
        }
    }

    DelayedSingleton<AppStateObserverManager>::GetInstance()->OnAppStateChanged(
        appRecord, state, needNotifyApp, isFromWindowFocusChanged);
}

void AppMgrServiceInner::OnAppStarted(const std::shared_ptr<AppRunningRecord> &appRecord)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRecord null");
        return;
    }

    if (appRecord->GetPriorityObject() == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "priorityobject null");
        return;
    }

    TAG_LOGD(AAFwkTag::APPMGR, "OnAppStarted begin, bundleName is %{public}s, pid:%{public}d",
        appRecord->GetBundleName().c_str(), appRecord->GetPid());

    DelayedSingleton<AppStateObserverManager>::GetInstance()->OnAppStarted(appRecord);
}


void AppMgrServiceInner::OnAppStopped(const std::shared_ptr<AppRunningRecord> &appRecord)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRecord null");
        return;
    }

    if (appRecord->GetPriorityObject() == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "priorityObject null");
        return;
    }

    TAG_LOGD(AAFwkTag::APPMGR, "OnAppStopped begin, bundleName is %{public}s, pid:%{public}d",
        appRecord->GetBundleName().c_str(), appRecord->GetPid());

    DelayedSingleton<AppStateObserverManager>::GetInstance()->OnAppStopped(appRecord);
}

AppProcessData AppMgrServiceInner::WrapAppProcessData(const std::shared_ptr<AppRunningRecord> &appRecord,
    const ApplicationState state)
{
    AppProcessData processData;
    CHECK_POINTER_AND_RETURN_VALUE(appRecord, processData);
    auto appInfoList = appRecord->GetAppInfoList();
    for (const auto &list : appInfoList) {
        AppData data;
        if (!list) {
            continue;
        }
        data.appName = list->name;
        data.uid = list->uid;
        processData.appDatas.push_back(data);
    }
    processData.processName = appRecord->GetProcessName();
    processData.pid = appRecord->GetPid();
    processData.appState = state;
    processData.isFocused = appRecord->GetFocusFlag();
    processData.appIndex = appRecord->GetAppIndex();
    processData.instanceKey = appRecord->GetInstanceKey();
    processData.bundleName = appRecord->GetBundleName();
    auto renderRecordMap = appRecord->GetRenderRecordMap();
    if (!renderRecordMap.empty()) {
        for (auto iter : renderRecordMap) {
            auto renderRecord = iter.second;
            if (renderRecord != nullptr) {
                processData.renderPids.emplace_back(renderRecord->GetPid());
            }
        }
    }
    return processData;
}

void AppMgrServiceInner::OnAbilityStateChanged(
    const std::shared_ptr<AbilityRunningRecord> &ability, const AbilityState state)
{
    if (!ability) {
        TAG_LOGE(AAFwkTag::APPMGR, "ability null");
        return;
    }
    std::lock_guard lock(appStateCallbacksLock_);
    for (const auto &item : appStateCallbacks_) {
        if (item.callback != nullptr) {
            item.callback->OnAbilityRequestDone(ability->GetToken(), state);
        }
    }
}

void AppMgrServiceInner::StateChangedNotifyObserver(
    const AbilityStateData abilityStateData, bool isAbility, bool isFromWindowFocusChanged)
{
    DelayedSingleton<AppStateObserverManager>::GetInstance()->StateChangedNotifyObserver(
        abilityStateData, isAbility, isFromWindowFocusChanged);
}

int32_t AppMgrServiceInner::StartPerfProcessByStartMsg(AppSpawnStartMsg &startMsg,
    const std::string& perfCmd, const std::string& debugCmd, bool isSandboxApp)
{
    if (!remoteClientManager_ || !remoteClientManager_->GetSpawnClient()) {
        TAG_LOGE(AAFwkTag::APPMGR, "appSpawnClient null");
        return ERR_NO_INIT;
    }
    if (perfCmd.empty() && debugCmd.empty()) {
        TAG_LOGD(AAFwkTag::APPMGR, "perfCmd is empty");
        return ERR_INVALID_OPERATION;
    }

    startMsg.code = static_cast<int32_t>(MSG_SPAWN_NATIVE_PROCESS);
    if (!isSandboxApp) {
        TAG_LOGD(AAFwkTag::APPMGR, "debuggablePipe sandbox: false.");
        startMsg.flags |= (START_FLAG_BASE << StartFlags::NO_SANDBOX);
    } else {
        TAG_LOGI(AAFwkTag::APPMGR, "debuggablePipe sandbox: true");
    }
    if (!perfCmd.empty()) {
        startMsg.renderParam = perfCmd;
        TAG_LOGI(AAFwkTag::APPMGR, "debuggablePipe perfCmd:%{public}s", perfCmd.c_str());
    } else {
        startMsg.renderParam = debugCmd;
        TAG_LOGI(AAFwkTag::APPMGR, "debuggablePipe debugCmd:%{public}s", debugCmd.c_str());
    }
    pid_t pid = 0;
    auto errCode = remoteClientManager_->GetSpawnClient()->StartProcess(startMsg, pid);
    if (FAILED(errCode)) {
        TAG_LOGE(AAFwkTag::APPMGR, "new native process fail, errCode %{public}08x", errCode);
        return errCode;
    }
    return ERR_OK;
}

int32_t AppMgrServiceInner::StartPerfProcess(const std::shared_ptr<AppRunningRecord> &appRecord,
    const std::string& perfCmd, const std::string& debugCmd, bool isSandboxApp)
{
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRecord null");
        return ERR_INVALID_OPERATION;
    }

    auto&& startMsg = appRecord->GetStartMsg();
    return StartPerfProcessByStartMsg(startMsg, perfCmd, debugCmd, isSandboxApp);
}

void AppMgrServiceInner::SetOverlayInfo(const std::string &bundleName,
                                        const int32_t userId,
                                        AppSpawnStartMsg &startMsg)
{
    if (remoteClientManager_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "remoteClientManager_ null");
        return;
    }
    auto bundleMgrHelper = remoteClientManager_->GetBundleManagerHelper();
    if (bundleMgrHelper == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "bundleMgrHelper null");
        return;
    }
    auto overlayMgrProxy = bundleMgrHelper->GetOverlayManagerProxy();
    if (overlayMgrProxy !=  nullptr) {
        std::vector<OverlayModuleInfo> overlayModuleInfo;
        TAG_LOGD(AAFwkTag::APPMGR, "Check overlay app begin.");
        HITRACE_METER_NAME(HITRACE_TAG_APP, "BMS->GetOverlayModuleInfoForTarget");
        auto targetRet = IN_PROCESS_CALL(overlayMgrProxy->GetOverlayModuleInfoForTarget(
            bundleName, "", overlayModuleInfo, userId));
        if (targetRet == ERR_OK && overlayModuleInfo.size() != 0) {
            TAG_LOGD(AAFwkTag::APPMGR, "Start an overlay app process.");
            startMsg.flags = startMsg.flags | APP_OVERLAY_FLAG;
            std::string overlayInfoPaths;
            for (auto it : overlayModuleInfo) {
                overlayInfoPaths += (it.hapPath + "|");
            }
            startMsg.overlayInfo = overlayInfoPaths;
        }
    }
}

void AppMgrServiceInner::SetAppEnvInfo(const BundleInfo &bundleInfo, AppSpawnStartMsg& startMsg)
{
    if (bundleInfo.applicationInfo.tsanEnabled) {
        startMsg.appEnv.emplace(TSAN_FLAG_NAME, std::to_string(1));
    } else {
        startMsg.appEnv.emplace(TSAN_FLAG_NAME, std::to_string(0));
    }

    if (bundleInfo.applicationInfo.hwasanEnabled) {
        startMsg.appEnv.emplace(HWASAN_FLAG_NAME, std::to_string(1));
    } else {
        startMsg.appEnv.emplace(HWASAN_FLAG_NAME, std::to_string(0));
    }

    if (bundleInfo.applicationInfo.ubsanEnabled) {
        startMsg.appEnv.emplace(UBSAN_FLAG_NAME, std::to_string(1));
    } else {
        startMsg.appEnv.emplace(UBSAN_FLAG_NAME, std::to_string(0));
    }

    if (!bundleInfo.applicationInfo.appEnvironments.empty()) {
        for (const auto& appEnvironment : bundleInfo.applicationInfo.appEnvironments) {
            startMsg.appEnv.emplace(appEnvironment.name, appEnvironment.value);
        }
    }
}

void AppMgrServiceInner::AddMountPermission(uint32_t accessTokenId, std::set<std::string> &permissions)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    CHECK_POINTER_AND_RETURN_LOG(remoteClientManager_, "remoteClientManager_ null");
    auto spawnClient = remoteClientManager_->GetSpawnClient();
    if (!spawnClient) {
        TAG_LOGE(AAFwkTag::APPMGR, "spawnClient null");
        return;
    }
    auto handle = spawnClient->GetAppSpawnClientHandle();
    int32_t maxPermissionIndex = GetMaxPermissionIndex(handle);
    if (maxPermissionIndex <= 0) {
        TAG_LOGE(AAFwkTag::APPMGR, "maxPermissionIndex error: %{public}d", maxPermissionIndex);
        return;
    }
    std::vector<std::string> tmpPermissionList;
    tmpPermissionList.reserve(maxPermissionIndex);
    for (int i = 0; i < maxPermissionIndex; i++) {
        tmpPermissionList.emplace_back(std::string(GetPermissionByIndex(handle, i)));
    }

    std::vector<int32_t> permStateList;
    auto result = Security::AccessToken::AccessTokenKit::VerifyAccessToken(accessTokenId, tmpPermissionList,
        permStateList, true);
    if (result != ERR_OK || permStateList.size() != tmpPermissionList.size()) {
        TAG_LOGE(AAFwkTag::APPMGR, "VerifyAccessToken error: %{public}d", result);
        return;
    }
    for (int i = 0; i < permStateList.size(); i++) {
        if (permStateList[i] == Security::AccessToken::PERMISSION_GRANTED) {
            permissions.insert(tmpPermissionList[i]);
        }
    }
}

void AppMgrServiceInner::StartProcessVerifyPermission(const BundleInfo &bundleInfo, bool &hasAccessBundleDirReq,
                                                      uint8_t &setAllowInternet, uint8_t &allowInternet,
                                                      std::vector<int32_t> &gids)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    hasAccessBundleDirReq = std::any_of(bundleInfo.reqPermissions.begin(), bundleInfo.reqPermissions.end(),
        [] (const auto &reqPermission) {
            if (PERMISSION_ACCESS_BUNDLE_DIR == reqPermission) {
                return true;
            }
            return false;
        });

    auto token = bundleInfo.applicationInfo.accessTokenId;
    {
        HITRACE_METER_NAME(HITRACE_TAG_APP, "AccessTokenKit::VerifyAccessToken");
#ifdef ABILITY_PLATFORM_CHECK_PERMISSION
        int result = CheckStablePermission(bundleInfo);
#else
        int result = Security::AccessToken::AccessTokenKit::VerifyAccessToken(token, PERMISSION_INTERNET, false);
#endif //ABILITY_PLATFORM_CHECK_PERMISSION
        if (result != Security::AccessToken::PERMISSION_GRANTED) {
            setAllowInternet = 1;
            allowInternet = 0;
    #ifdef APP_MGR_SERVICE_APPMS
            auto ret = OHOS::NetManagerStandard::NetConnClient::GetInstance().SetInternetPermission(bundleInfo.uid, 0);
            TAG_LOGD(AAFwkTag::APPMGR, "SetInternetPermission, ret = %{public}d", ret);
        } else {
            auto ret = OHOS::NetManagerStandard::NetConnClient::GetInstance().SetInternetPermission(bundleInfo.uid, 1);
            TAG_LOGD(AAFwkTag::APPMGR, "SetInternetPermission, ret = %{public}d", ret);
            gids.push_back(NETSYS_SOCKET_GROUPID);
    #endif
        }

        result = Security::AccessToken::AccessTokenKit::VerifyAccessToken(token, PERMISSION_MANAGE_VPN, false);
        if (result == Security::AccessToken::PERMISSION_GRANTED) {
            gids.push_back(BLUETOOTH_GROUPID);
        }

        if (hasAccessBundleDirReq) {
            int result = Security::AccessToken::AccessTokenKit::VerifyAccessToken(token,
                PERMISSION_ACCESS_BUNDLE_DIR, false);
            if (result != Security::AccessToken::PERMISSION_GRANTED) {
                TAG_LOGE(AAFwkTag::APPMGR, "startProcess not granted");
                hasAccessBundleDirReq = false;
            }
        }
    }
}

#ifdef ABILITY_PLATFORM_CHECK_PERMISSION
int AppMgrServiceInner::CheckStablePermission(const BundleInfo &bundleInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, "AccessTokenKit::CheckStablePermission");
    auto token = bundleInfo.applicationInfo.accessTokenId;
    std::vectorSecurity::AccessToken::PermissionStateFull reqPermList;
    auto deviceid = bundleInfo.applicationInfo.deviceId;
    Security::AccessToken::AccessTokenKit::GetReqPermissions(token, reqPermList, true);
    bool granted = std::any_of(
        reqPermList.begin(), reqPermList.end(),
        [deviceid](const Security::AccessToken::PermissionStateFull &status) {
            if (status.permissionName == PERMISSION_INTERNET &&
                status.grantStatus.size() == status.resDeviceID.size()) {
                if (CheckDeviceStatus(status, deviceId)) {
                    return true;
                }
            }
        return false;
    });
    int result = granted ? Security::AccessToken::PERMISSION_GRANTED : Security::AccessToken::PERMISSION_DENIED;
    TAG_LOGI(AAFwkTag::APPMGR, "GetInternetPermission, ret %{public}d, uid %{public}d, token %{public}d", result,
        bundleInfo.uid, token);
    return result;
}

bool AppMgrServiceInner::CheckDeviceStatus(Security::AccessToken::PermissionStateFull &status,
    std::string deviceid)
{
    for (size_t i = 0; i < status.resDeviceID.size(); i++) {
        if (status.resDeviceID[i] == deviceid &&
            status.grantStatus[i] == Security::AccessToken::PERMISSION_GRANTED) {
            return true;
        }
    }
    return false;
}
#endif //ABILITY_PLATFORM_CHECK_PERMISSION

int32_t AppMgrServiceInner::CreatNewStartMsg(const Want &want, const AbilityInfo &abilityInfo,
    const std::shared_ptr<ApplicationInfo> &appInfo, const std::string &processName, AppSpawnStartMsg &startMsg)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    if (!remoteClientManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "remoteClientManager null");
        return ERR_NO_INIT;
    }
    auto bundleMgrHelper = remoteClientManager_->GetBundleManagerHelper();
    if (!bundleMgrHelper) {
        TAG_LOGE(AAFwkTag::APPMGR, "bundleMgrHelper null");
        return ERR_NO_INIT;
    }

    BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    int32_t appIndex = want.GetIntParam(AppspawnUtil::DLP_PARAMS_INDEX, 0);
    if (!GetBundleAndHapInfo(abilityInfo, appInfo, bundleInfo, hapModuleInfo, appIndex)) {
        TAG_LOGE(AAFwkTag::APPMGR, "getBundleAndHapInfo fail");
        return ERR_NO_INIT;
    }

    uint32_t startFlags = AppspawnUtil::BuildStartFlags(want, abilityInfo);
    CHECK_POINTER_AND_RETURN_VALUE(appInfo, ERR_NO_INIT);
    auto uid = appInfo->uid;
    auto bundleType = appInfo->bundleType;
    CreateStartMsgParam startMsgParam;
    startMsgParam.processName = processName;
    startMsgParam.startFlags = startFlags;
    startMsgParam.uid = uid;
    startMsgParam.bundleInfo = bundleInfo;
    startMsgParam.bundleIndex = appIndex;
    startMsgParam.bundleType = bundleType;
    auto ret = CreateStartMsg(startMsgParam, startMsg);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "createStartMsg fail");
    }
    return ret;
}

void AppMgrServiceInner::SetAtomicServiceInfo(BundleType bundleType, AppSpawnStartMsg &startMsg)
{
#ifdef OHOS_ACCOUNT_ENABLED
    TAG_LOGD(AAFwkTag::APPMGR, "execute with OHOS_ACCOUNT_ENABLED on");
    if (bundleType == BundleType::ATOMIC_SERVICE) {
        TAG_LOGI(AAFwkTag::APPMGR, "app is atomic service type");
        AccountSA::OhosAccountInfo accountInfo;
        auto errCode = AccountSA::OhosAccountKits::GetInstance().GetOhosAccountInfo(accountInfo);
        if (errCode == ERR_OK) {
            TAG_LOGI(AAFwkTag::APPMGR, "getOhosAccountInfo succeed, uid %{public}s", accountInfo.uid_.c_str());
            startMsg.atomicServiceFlag = true;
            startMsg.atomicAccount = accountInfo.uid_;
        } else {
            TAG_LOGE(AAFwkTag::APPMGR, "get ohos account info:%{public}d fail", errCode);
        }
    }
#else
    TAG_LOGD(AAFwkTag::APPMGR, "execute with OHOS_ACCOUNT_ENABLED off");
#endif // OHOS_ACCOUNT_ENABLED
}

void AppMgrServiceInner::SetAppInfo(const BundleInfo &bundleInfo, AppSpawnStartMsg &startMsg)
{
    bool hasAccessBundleDirReq;
    bool tempJitAllow = false;
    uint8_t setAllowInternet = 0;
    uint8_t allowInternet = 1;
    std::vector<int32_t> gids;
    StartProcessVerifyPermission(bundleInfo, hasAccessBundleDirReq, setAllowInternet, allowInternet, gids);
    startMsg.uid = bundleInfo.uid;
    startMsg.gid = bundleInfo.gid;
    startMsg.hapFlags = bundleInfo.isPreInstallApp ? 1 : 0;
    startMsg.accessTokenId = bundleInfo.applicationInfo.accessTokenId;
    startMsg.accessTokenIdEx = bundleInfo.applicationInfo.accessTokenIdEx;
    startMsg.apl = bundleInfo.applicationInfo.appPrivilegeLevel;
    startMsg.ownerId = bundleInfo.signatureInfo.appIdentifier;
    startMsg.provisionType = bundleInfo.applicationInfo.appProvisionType;
#ifdef SUPPORT_CHILD_PROCESS
    if (bundleInfo.applicationInfo.apiTargetVersion % API_VERSION_MOD < API15) {
        startMsg.maxChildProcess = 0;
    } else {
        if (startMsg.maxChildProcess == 0) {
            startMsg.maxChildProcess = bundleInfo.applicationInfo.maxChildProcess;
        }
    }
#endif // SUPPORT_CHILD_PROCESS
    startMsg.setAllowInternet = setAllowInternet;
    startMsg.allowInternet = allowInternet;
    startMsg.gids = gids;
    startMsg.flags |= hasAccessBundleDirReq ? APP_ACCESS_BUNDLE_DIR : 0;
    tempJitAllow = std::any_of(bundleInfo.reqPermissions.begin(), bundleInfo.reqPermissions.end(),
        [] (const auto &reqPermission) {
            if (PERMISSION_TEMP_JIT_ALLOW == reqPermission) {
                return true;
            }
            return false;
        });
    startMsg.flags |= tempJitAllow ? START_FLAG_BASE << StartFlags::TEMP_JIT_ALLOW : 0;
    SetAppEnvInfo(bundleInfo, startMsg);
}

int32_t AppMgrServiceInner::CreateStartMsg(const CreateStartMsgParam &param, AppSpawnStartMsg &startMsg)
{
    if (!remoteClientManager_ || !otherTaskHandler_) {
        TAG_LOGE(AAFwkTag::APPMGR, "remoteClientManager or otherTaskHandler null");
        return ERR_NO_INIT;
    }
    auto bundleMgrHelper = remoteClientManager_->GetBundleManagerHelper();
    if (!bundleMgrHelper) {
        TAG_LOGE(AAFwkTag::APPMGR, "get bundle manager helper fail");
        return ERR_NO_INIT;
    }

    auto &bundleInfo = param.bundleInfo;
    AAFwk::AutoSyncTaskHandle autoSync(otherTaskHandler_->SubmitTask([&]() {
        AddMountPermission(bundleInfo.applicationInfo.accessTokenId, startMsg.permissions);
        }, AAFwk::TaskAttribute{
            .taskName_ = "AddMountPermission",
            .taskQos_ = AAFwk::TaskQoS::USER_INTERACTIVE
        }));

    HspList hspList;
    auto ret = bundleMgrHelper->GetBaseSharedBundleInfos(bundleInfo.name, hspList,
        AppExecFwk::GetDependentBundleInfoFlag::GET_ALL_DEPENDENT_BUNDLE_INFO);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "getBaseSharedBundleInfos fail: %{public}d", ret);
        return ret;
    }
    startMsg.hspList = hspList;

    auto userId = GetUserIdByUid(param.uid);
    DataGroupInfoList dataGroupInfoList;
    bool result = bundleMgrHelper->QueryDataGroupInfos(bundleInfo.name, userId, dataGroupInfoList);
    if (!result || dataGroupInfoList.empty()) {
        TAG_LOGD(AAFwkTag::APPMGR, "the bundle has no groupInfos.");
    }
    QueryExtensionSandBox(param.moduleName, param.abilityName, bundleInfo, startMsg, dataGroupInfoList, param.want);
    SetStartMsgStrictMode(startMsg, param);
    startMsg.bundleName = bundleInfo.name;
    startMsg.renderParam = RENDER_PARAM;
    startMsg.flags = param.startFlags;
    startMsg.bundleIndex = param.bundleIndex;
    startMsg.procName = param.processName;
    SetAtomicServiceInfo(param.bundleType, startMsg);
    SetOverlayInfo(bundleInfo.name, userId, startMsg);
    SetAppInfo(bundleInfo, startMsg);
    SetStartMsgCustomSandboxFlag(startMsg, userId);
    GetKernelPermissions(bundleInfo.applicationInfo.accessTokenId, startMsg.jitPermissionsMap);
    TAG_LOGI(AAFwkTag::APPMGR, "apl: %{public}s, bundleName: %{public}s, startFlags: %{public}d, userId: %{public}d",
        startMsg.apl.c_str(), bundleInfo.name.c_str(), param.startFlags, userId);

    autoSync.Sync();
    return ERR_OK;
}

void AppMgrServiceInner::SetStartMsgCustomSandboxFlag(AppSpawnStartMsg &startMsg, int32_t userId)
{
    if (AAFwk::AmsConfigurationParameter::GetInstance().CustomSandbox() == 0 ||
        !AAFwk::AppUtils::GetInstance().IsStartOptionsWithAnimation()) {
        TAG_LOGE(AAFwkTag::APPMGR, "Not in PC mode or developer mode");
        return;
    }

    if (remoteClientManager_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "remoteClientManager null");
        return;
    }
    auto bundleMgrHelper = remoteClientManager_->GetBundleManagerHelper();
    if (bundleMgrHelper == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "bundleMgrHelper null");
        return;
    }

    AppExecFwk::AppProvisionInfo appProvisionInfo;
    if (bundleMgrHelper->GetAppProvisionInfo(startMsg.bundleName, userId, appProvisionInfo) != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "Failed to get app provision info");
        return;
    }
    std::string jsonString = appProvisionInfo.appServiceCapabilities;
    if (jsonString.empty()) {
        TAG_LOGE(AAFwkTag::APPMGR, "JSON string is empty");
        return;
    }

    nlohmann::json jsonData = nlohmann::json::parse(jsonString);
    if (jsonData.is_discarded()) {
        TAG_LOGE(AAFwkTag::APPMGR, "failed to parse json string");
        return;
    }
    if (jsonData.contains(CUSTOM_SANDBOX_KEY)) {
        startMsg.isCustomSandboxFlag = true;
    } else {
        TAG_LOGD(AAFwkTag::APPMGR, "jsonData is not contains %{public}s", CUSTOM_SANDBOX_KEY.c_str());
    }
}

void AppMgrServiceInner::GetKernelPermissions(uint32_t accessTokenId, JITPermissionsMap &permissionsMap)
{
    std::vector<Security::AccessToken::PermissionWithValue> permissionWithValueList;
    AccessToken::AccessTokenKit::GetKernelPermissions(accessTokenId, permissionWithValueList);
    for (const auto &item : permissionWithValueList) {
        permissionsMap.emplace(item.permissionName, item.value);
    }
    std::vector<std::string> permissionsList;
    AppspawnUtil::SetJITPermissions(accessTokenId, permissionsList);
    for (const auto &item : permissionsList) {
        auto it = permissionsMap.find(item);
        if (it == permissionsMap.end()) {
            permissionsMap.emplace(item, "");
        }
    }
}

void AppMgrServiceInner::SetStartMsgStrictMode(AppSpawnStartMsg &startMsg, const CreateStartMsgParam &param)
{
    startMsg.strictMode = param.strictMode;
    if (param.extensionAbilityType == ExtensionAbilityType::INPUTMETHOD) {
        startMsg.isolatedSandboxFlagLegacy = true;
    } else {
        startMsg.isolatedNetworkFlag = !param.networkEnableFlags;
        startMsg.isolatedSELinuxFlag = !param.saEnableFlags;
        startMsg.extensionTypeName = ConvertToExtensionTypeName(param.extensionAbilityType);
    }
}

#ifdef SUPPORT_CHILD_PROCESS
void AppMgrServiceInner::PresetMaxChildProcess(std::shared_ptr<AppRunningRecord> appRecord, int32_t &maxChildProcess)
{
    CHECK_POINTER_AND_RETURN_LOG(appRecord, "appRecord null");
    ProcessType processType = appRecord->GetProcessType();
    ExtensionAbilityType extensionType = appRecord->GetExtensionType();
    if (processType == ProcessType::EXTENSION && extensionType != ExtensionAbilityType::DATASHARE &&
        extensionType != ExtensionAbilityType::SERVICE) {
        maxChildProcess = 1;
    }
}
#endif // SUPPORT_CHILD_PROCESS

void AppMgrServiceInner::QueryExtensionSandBox(const std::string &moduleName, const std::string &abilityName,
    const BundleInfo &bundleInfo, AppSpawnStartMsg &startMsg, DataGroupInfoList &dataGroupInfoList,
    std::shared_ptr<AAFwk::Want> want)
{
    std::vector<ExtensionAbilityInfo> extensionInfos;
    for (auto hapModuleInfo: bundleInfo.hapModuleInfos) {
        extensionInfos.insert(extensionInfos.end(), hapModuleInfo.extensionInfos.begin(),
            hapModuleInfo.extensionInfos.end());
    }
    auto isExist = (want == nullptr) ? false : want->HasParameter(ISOLATED_SANDBOX);
    bool isolatedSandbox = false;
    if (isExist) {
        isolatedSandbox = (want == nullptr) ? false : want->GetBoolParam(ISOLATED_SANDBOX, false);
    }
    auto infoExisted = [&moduleName, &abilityName, &isExist, &isolatedSandbox](
                           const ExtensionAbilityInfo &info) {
        auto ret = info.moduleName == moduleName && info.name == abilityName && info.needCreateSandbox;
        if (isExist) {
            return ret && isolatedSandbox;
        }
        return ret;
    };
    auto infoIter = std::find_if(extensionInfos.begin(), extensionInfos.end(), infoExisted);
    DataGroupInfoList extensionDataGroupInfoList;
    if (infoIter != extensionInfos.end()) {
        startMsg.isolatedExtension = true;
        startMsg.extensionSandboxPath = infoIter->moduleName + "-" + infoIter->name;
        for (auto dataGroupInfo : dataGroupInfoList) {
            auto groupIdExisted = [&dataGroupInfo](const std::string &dataGroupId) {
                return dataGroupInfo.dataGroupId == dataGroupId;
            };
            if (std::find_if(infoIter->dataGroupIds.begin(), infoIter->dataGroupIds.end(), groupIdExisted) !=
                infoIter->dataGroupIds.end()) {
                extensionDataGroupInfoList.emplace_back(dataGroupInfo);
            }
        }
        startMsg.dataGroupInfoList = extensionDataGroupInfoList;
    } else {
        startMsg.dataGroupInfoList = dataGroupInfoList;
    }
    bool isScreenLockDataProtect = std::any_of(bundleInfo.reqPermissions.begin(), bundleInfo.reqPermissions.end(),
        [] (const auto &reqPermission) {
            if (PERMISSION_PROTECT_SCREEN_LOCK_DATA == reqPermission) {
                TAG_LOGD(AAFwkTag::APPMGR, "has el5 permission: %{public}s", PERMISSION_PROTECT_SCREEN_LOCK_DATA);
                return true;
            }
            return false;
        });
    startMsg.isScreenLockDataProtect = isScreenLockDataProtect;
}

int32_t AppMgrServiceInner::StartProcess(const std::string &appName, const std::string &processName,
    uint32_t startFlags, std::shared_ptr<AppRunningRecord> appRecord, const int uid, const BundleInfo &bundleInfo,
    const std::string &bundleName, const int32_t bundleIndex, bool appExistFlag, bool isPreload,
    AppExecFwk::PreloadMode preloadMode, const std::string &moduleName, const std::string &abilityName,
    sptr<IRemoteObject> token, std::shared_ptr<AAFwk::Want> want, ExtensionAbilityType extensionAbilityType)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::APPMGR, "bundleName: %{public}s, isPreload: %{public}d", bundleName.c_str(), isPreload);
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRecord null");
        return ERR_INVALID_VALUE;
    }
    bool isCJApp = IsCjApplication(bundleInfo);
    if (!remoteClientManager_ || !remoteClientManager_->GetSpawnClient()) {
        TAG_LOGE(AAFwkTag::APPMGR, "appSpawnClient null");
        appRunningManager_->RemoveAppRunningRecordById(appRecord->GetRecordId());
        if (!isCJApp) {
            SendProcessStartFailedEvent(appRecord, ProcessStartFailedReason::GET_SPAWN_CLIENT_FAILED,
                PROCESS_START_FAILED_SUB_REASON_UNKNOWN);
        }
        return AAFwk::ERR_GET_SPAWN_CLIENT_FAILED;
    }

    AppSpawnStartMsg startMsg;
    auto appInfo = appRecord->GetApplicationInfo();
    auto bundleType = appInfo ? appInfo->bundleType : BundleType::APP;
#ifdef SUPPORT_CHILD_PROCESS
    PresetMaxChildProcess(appRecord, startMsg.maxChildProcess);
#endif // SUPPORT_CHILD_PROCESS
    CreateStartMsgParam startMsgParam;
    startMsgParam.processName = processName;
    startMsgParam.startFlags = startFlags;
    startMsgParam.uid = uid;
    startMsgParam.bundleInfo = bundleInfo;
    startMsgParam.bundleIndex = bundleIndex;
    startMsgParam.bundleType = bundleType;
    startMsgParam.want = want;
    startMsgParam.moduleName = moduleName;
    startMsgParam.abilityName = abilityName;
    startMsgParam.strictMode = appRecord->IsStrictMode();
    startMsgParam.networkEnableFlags = appRecord->GetNetworkEnableFlags();
    startMsgParam.saEnableFlags = appRecord->GetSAEnableFlags();
    startMsgParam.extensionAbilityType = extensionAbilityType;
    auto ret = CreateStartMsg(startMsgParam, startMsg);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "createStartMsg fail");
        appRunningManager_->RemoveAppRunningRecordById(appRecord->GetRecordId());
        if (!isCJApp) {
            SendProcessStartFailedEvent(appRecord, ProcessStartFailedReason::CREATE_START_MSG_FAILED, ret);
        }
        return AAFwk::ERR_CREATE_START_MSG_FAILED;
    };
    SetProcessJITState(appRecord);
    PerfProfile::GetInstance().SetAppForkStartTime(GetTickCount());
    pid_t pid = 0;
    ErrCode errCode = ERR_OK;
    if (isCJApp) {
        if (!remoteClientManager_->GetCJSpawnClient()) {
            TAG_LOGE(AAFwkTag::APPMGR, "appSpawnClient null");
            appRunningManager_->RemoveAppRunningRecordById(appRecord->GetRecordId());
            return AAFwk::ERR_GET_SPAWN_CLIENT_FAILED;
        }
        SendCreateAtomicServiceProcessEvent(appRecord, bundleType, moduleName, abilityName);
        errCode = remoteClientManager_->GetCJSpawnClient()->StartProcess(startMsg, pid);
    } else {
        SendCreateAtomicServiceProcessEvent(appRecord, bundleType, moduleName, abilityName);
        startMsg.gids.push_back(SHADER_CACHE_GROUPID);
        errCode = remoteClientManager_->GetSpawnClient()->StartProcess(startMsg, pid);
    }
    if (FAILED(errCode)) {
        TAG_LOGE(AAFwkTag::APPMGR, "spawn new app fail, errCode %{public}08x", errCode);
        appRunningManager_->RemoveAppRunningRecordById(appRecord->GetRecordId());
        if (!isCJApp) {
            SendProcessStartFailedEvent(appRecord, ProcessStartFailedReason::APPSPAWN_FAILED,
                static_cast<int32_t>(errCode));
        }
        return AAFwk::ERR_SPAWN_PROCESS_FAILED;
    }

    #ifdef ABILITY_RUNTIME_FEATURE_SANDBOXMANAGER
    bool checkApiVersion = (appInfo && (appInfo->apiTargetVersion % API_VERSION_MOD == API10));
    TAG_LOGD(AAFwkTag::APPMGR, "version of api is %{public}d", appInfo->apiTargetVersion % API_VERSION_MOD);
    if (checkApiVersion && AAFwk::AppUtils::GetInstance().IsGrantPersistUriPermission()) {
        uint32_t tokenId = appInfo->accessTokenId;
        auto sandboxRet = AccessControl::SandboxManager::SandboxManagerKit::StartAccessingByTokenId(tokenId);
        TAG_LOGI(AAFwkTag::APPMGR, "tokenId: %{public}u, ret: %{public}d", tokenId, sandboxRet);
    }
    #endif

    TAG_LOGI(AAFwkTag::APPMGR, "start process success, pid: %{public}d, processName: %{public}s",
        pid, processName.c_str());
    SetRunningSharedBundleList(bundleName, startMsg.hspList);
    CHECK_POINTER_AND_RETURN_VALUE(appRecord->GetPriorityObject(), ERR_INVALID_VALUE);
    appRecord->GetPriorityObject()->SetPid(pid);
    appRecord->SetUid(startMsg.uid);
    appRecord->SetStartMsg(startMsg);
    appRecord->SetAppMgrServiceInner(weak_from_this());
    appRecord->SetSpawned();
    if (AAFwk::UIExtensionUtils::IsUIExtension(extensionAbilityType)) {
        TAG_LOGD(AAFwkTag::APPMGR, "Add UIExtension LauncherItem.");
        AddUIExtensionLauncherItem(want, appRecord, token);
    }
    OnAppStateChanged(appRecord, ApplicationState::APP_STATE_CREATE, false, false);
    DelayedSingleton<AppStateObserverManager>::GetInstance()->OnProcessCreated(appRecord, isPreload);
    if (AAFwk::UIExtensionUtils::IsUIExtension(extensionAbilityType)) {
        AddUIExtensionBindItem(want, appRecord, token);
    }
    if (!appExistFlag) {
        OnAppStarted(appRecord);
    }
    PerfProfile::GetInstance().SetAppForkEndTime(GetTickCount());
    SendProcessStartEvent(appRecord, isPreload, preloadMode);
    ProcessAppDebug(appRecord, appRecord->IsDebugApp());
    return ERR_OK;
}

void AppMgrServiceInner::SetProcessJITState(const std::shared_ptr<AppRunningRecord> appRecord)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRecord null");
        return;
    }
    if (!securityModeManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "securityModeManager_ null");
        appRecord->SetJITEnabled(true);
        return;
    }
    appRecord->SetJITEnabled(securityModeManager_->IsJITEnabled());
}

AppDebugInfo AppMgrServiceInner::MakeAppDebugInfo(
    const std::shared_ptr<AppRunningRecord> &appRecord, const bool &isDebugStart)
{
    AppDebugInfo info;
    if (appRecord == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRecord null");
        return info;
    }

    info.bundleName = appRecord->GetBundleName();
    auto priorityObject = appRecord->GetPriorityObject();
    if (priorityObject) {
        info.pid = priorityObject->GetPid();
    }
    info.uid = appRecord->GetUid();
    info.isDebugStart = isDebugStart;
    return info;
}

void AppMgrServiceInner::ProcessAppDebug(const std::shared_ptr<AppRunningRecord> &appRecord, const bool &isDebugStart)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    if (appRecord == nullptr || appDebugManager_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRecord or appDebugManager_ null");
        return;
    }

    auto startDebug = [this, appRecord](const bool &isDebugStart) {
        std::vector<AppDebugInfo> debugInfos;
        debugInfos.emplace_back(MakeAppDebugInfo(appRecord, isDebugStart));
        appDebugManager_->StartDebug(debugInfos);
    };

    if (isDebugStart && !appRecord->IsDebugApp()) {
        appRecord->SetDebugApp(true);
        startDebug(true);
        return;
    }

    if (appRecord->IsDebugApp()) {
        startDebug(true);
        return;
    }

    auto bundleName = appRecord->GetBundleName();
    auto isDebugFromLocal = appRecord->GetDebugFromLocal();
    if (appDebugManager_->IsAttachDebug(bundleName)) {
        appRecord->SetAttachDebug(true, isDebugFromLocal);
        startDebug(false);
    }
}

bool AppMgrServiceInner::SendCreateAtomicServiceProcessEvent(const std::shared_ptr<AppRunningRecord> &appRecord,
    const BundleType &bundleType, const std::string &moduleName, const std::string &abilityName)
{
    if (bundleType != BundleType::ATOMIC_SERVICE) {
        return false;
    }
    if (!appRecord) {
        TAG_LOGI(AAFwkTag::APPMGR, "null appRecord");
        return false;
    }
    TAG_LOGI(AAFwkTag::APPMGR, "report createAtomicServiceProcessEvent");
    auto callerPid = appRecord->GetCallerPid() == -1 ? IPCSkeleton::GetCallingPid() : appRecord->GetCallerPid();
    auto callerAppRecord = GetAppRunningRecordByPid(callerPid);
    return AppMgrEventUtil::SendCreateAtomicServiceProcessEvent(callerAppRecord, appRecord, moduleName, abilityName);
}

bool AppMgrServiceInner::SendProcessStartEvent(const std::shared_ptr<AppRunningRecord> &appRecord, bool isPreload,
    AppExecFwk::PreloadMode preloadMode)
{
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRecord null");
        return false;
    }
    AAFwk::EventInfo eventInfo;
    auto callerPid = appRecord->GetCallerPid() == -1 ? IPCSkeleton::GetCallingPid() : appRecord->GetCallerPid();
    auto callerAppRecord = GetAppRunningRecordByPid(callerPid);
    eventInfo.isPreload = isPreload;
    eventInfo.preloadMode = static_cast<int32_t>(preloadMode);
    AppMgrEventUtil::SendProcessStartEvent(callerAppRecord, appRecord, eventInfo);
    SendReStartProcessEvent(eventInfo, appRecord->GetUid());
    return true;
}

void AppMgrServiceInner::SendReStartProcessEvent(AAFwk::EventInfo &eventInfo, int32_t appUid)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    std::lock_guard lock(killedProcessMapLock_);
    int64_t restartTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::
        system_clock::now().time_since_epoch()).count();
    for (auto iter = killedProcessMap_.begin(); iter != killedProcessMap_.end();) {
        int64_t killTime = iter->first;
        if (restartTime - killTime > PROCESS_RESTART_MARGIN_MICRO_SECONDS) {
            iter = killedProcessMap_.erase(iter);
            continue;
        }
        if (eventInfo.bundleName == eventInfo.callerBundleName &&
            eventInfo.processName != eventInfo.callerProcessName) {
            AppMgrEventUtil::SendReStartProcessEvent(eventInfo, appUid, restartTime);
            iter = killedProcessMap_.erase(iter);
            continue;
        }
        ++iter;
    }
}

bool AppMgrServiceInner::SendProcessStartFailedEvent(std::shared_ptr<AppRunningRecord> appRecord,
    ProcessStartFailedReason reason, int32_t subReason)
{
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRecord null");
        return false;
    }
    TAG_LOGD(AAFwkTag::APPMGR, "processName:%{public}s, reason:%{public}d, subReason:%{public}d",
        appRecord->GetProcessName().c_str(), reason, subReason);
    AAFwk::EventInfo eventInfo;
    eventInfo.reason = static_cast<int32_t>(reason);
    eventInfo.subReason = subReason;
    auto callerPid = appRecord->GetCallerPid() == -1 ? IPCSkeleton::GetCallingPid() : appRecord->GetCallerPid();
    auto callerAppRecord = GetAppRunningRecordByPid(callerPid);
    AppMgrEventUtil::SendProcessStartFailedEvent(callerAppRecord, appRecord, eventInfo);
    return true;
}

void AppMgrServiceInner::SendPreloadAppStartupTypeEvent(const std::shared_ptr<AppRunningRecord> &appRecord,
    const std::shared_ptr<AbilityInfo> &abilityInfo)
{
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRecord null");
        return;
    }

    PreloadMode preloadMode = appRecord->GetPreloadMode();
    if (!appRecord->IsPreloaded()) {
        TAG_LOGD(AAFwkTag::APPMGR, "not preload app start");
        return;
    }
    if (preloadMode == PreloadMode::PRE_MAKE) {
        SendAppStartupTypeEvent(appRecord, abilityInfo, AppStartType::WARM, AppStartReason::PRE_MAKE);
    } else if (preloadMode == PreloadMode::PRELOAD_MODULE) {
        SendAppStartupTypeEvent(appRecord, abilityInfo, AppStartType::WARM, AppStartReason::PRELOAD_MODULE);
    } else {
        TAG_LOGD(AAFwkTag::APPMGR, "app preload mode: %{public}d", static_cast<int32_t>(preloadMode));
    }
}

void AppMgrServiceInner::SendAppStartupTypeEvent(const std::shared_ptr<AppRunningRecord> &appRecord,
    const std::shared_ptr<AbilityInfo> &abilityInfo, const AppStartType startType, const AppStartReason reason)
{
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRecord null");
        return;
    }
    AAFwk::EventInfo eventInfo;
    auto applicationInfo = appRecord->GetApplicationInfo();
    if (!applicationInfo) {
        TAG_LOGE(AAFwkTag::APPMGR, "applicationInfo null");
    } else {
        eventInfo.bundleName = applicationInfo->name;
        eventInfo.versionName = applicationInfo->versionName;
        eventInfo.versionCode = applicationInfo->versionCode;
    }
    if (!abilityInfo) {
        TAG_LOGE(AAFwkTag::APPMGR, "abilityInfo null");
    } else {
        eventInfo.abilityName = abilityInfo->name;
    }
    if (appRecord->GetPriorityObject() == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "priorityObject null");
    } else {
        eventInfo.pid = appRecord->GetPid();
    }
    eventInfo.startType = static_cast<int32_t>(startType);
    eventInfo.startReason = static_cast<int32_t>(reason);
    AAFwk::EventReport::SendAppEvent(AAFwk::EventName::APP_STARTUP_TYPE, HiSysEventType::BEHAVIOR, eventInfo);
}

static bool GetAbilityNames(
    const std::map<const sptr<IRemoteObject>, std::shared_ptr<AbilityRunningRecord>> &abilityRecordList,
    std::vector<std::string> &abilityNames, std::vector<std::string> &uiExtensionNames,
    std::string &bundleName)
{
    std::string abilityName = "";
    std::string moduleName = "";
    for (auto it = abilityRecordList.begin(); it != abilityRecordList.end(); ++it) {
        if (it->second == nullptr) {
            continue;
        }
        auto abilityInfo = it->second->GetAbilityInfo();
        if (abilityInfo == nullptr) {
            continue;
        }
        abilityName = it->second->GetName();
        moduleName = it->second->GetModuleName();
        bundleName = it->second->GetBundleName();
        if (abilityInfo->type == AppExecFwk::AbilityType::PAGE) {
            abilityNames.push_back(abilityName);
        } else if (AAFwk::UIExtensionUtils::IsUIExtension(abilityInfo->extensionAbilityType)) {
            uiExtensionNames.push_back(moduleName + ":" + abilityName);
        }
    }
    if (abilityNames.empty() && uiExtensionNames.empty()) {
        TAG_LOGI(AAFwkTag::APPMGR, "no ability or uiextension");
        return false;
    }
    return true;
}

void AppMgrServiceInner::CacheExitInfo(const std::shared_ptr<AppRunningRecord> &appRecord)
{
    if (appRecord == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRecord null");
        return;
    }
    if (appRecord->GetReasonExist()) {
        TAG_LOGI(AAFwkTag::APPMGR, "exit info exist");
        return;
    }
    AAFwk::LastExitDetailInfo exitInfo = {};
    exitInfo.pid = appRecord->GetPid();
    exitInfo.uid = appRecord->GetUid();
    exitInfo.rss = appRecord->GetRssValue();
    exitInfo.pss = appRecord->GetPssValue();
    exitInfo.processName = appRecord->GetProcessName();
    std::vector<std::string> abilityNames;
    std::vector<std::string> uiExtensionNames;
    std::string bundleName = "";
    auto applicationInfo = appRecord->GetApplicationInfo();
    if (applicationInfo == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "applicationInfo null");
        return;
    }
    uint32_t accessTokenId = applicationInfo->accessTokenId;
    auto abilityRecordList = appRecord->GetAbilities();
    if (!GetAbilityNames(abilityRecordList, abilityNames, uiExtensionNames, bundleName)) {
        return;
    }
    {
        std::lock_guard lock(appStateCallbacksLock_);
        for (const auto &item : appStateCallbacks_) {
            if (item.callback != nullptr) {
                item.callback->OnCacheExitInfo(accessTokenId, exitInfo, bundleName, abilityNames, uiExtensionNames);
            }
        }
    }
}

void AppMgrServiceInner::OnRemoteDied(const wptr<IRemoteObject> &remote, bool isRenderProcess, bool isChildProcess)
{
    TAG_LOGD(AAFwkTag::APPMGR, "On remote died.");
    if (isRenderProcess) {
        OnRenderRemoteDied(remote);
        return;
    }
#ifdef SUPPORT_CHILD_PROCESS
    if (isChildProcess) {
        OnChildProcessRemoteDied(remote);
        return;
    }
#endif // SUPPORT_CHILD_PROCESS

    std::shared_ptr<AppRunningRecord> appRecord = nullptr;
    {
        std::lock_guard lock(exceptionLock_);
        appRecord = appRunningManager_->OnRemoteDied(remote, shared_from_this());
    }
    if (appRecord == nullptr) {
        TAG_LOGI(AAFwkTag::APPMGR, "null appRecord");
        return;
    }
    AppExecFwk::AppfreezeManager::GetInstance()->RemoveDeathProcess(appRecord->GetBundleName());
    std::vector<sptr<IRemoteObject>> abilityTokens;
    for (const auto &token : appRecord->GetAbilities()) {
        abilityTokens.emplace_back(token.first);
    }
    CacheExitInfo(appRecord);
    {
        std::lock_guard lock(appStateCallbacksLock_);
        for (const auto &item : appStateCallbacks_) {
            if (item.callback != nullptr) {
                item.callback->OnAppRemoteDied(abilityTokens);
            }
        }
    }
    ClearData(appRecord);
    
    if (appRecord->IsStartSpecifiedAbility() && startSpecifiedAbilityResponse_) {
        startSpecifiedAbilityResponse_->OnStartSpecifiedFailed(appRecord->GetSpecifiedRequestId());
    }
    appRecord->ResetSpecifiedRequest();
    if (appRecord->IsNewProcessRequest() && startSpecifiedAbilityResponse_) {
        startSpecifiedAbilityResponse_->OnStartSpecifiedFailed(appRecord->GetNewProcessRequestId());
    }
    appRecord->ResetNewProcessRequest();
}

void AppMgrServiceInner::ClearAppRunningData(const std::shared_ptr<AppRunningRecord> &appRecord)
{
    if (!appRecord) {
        return;
    }

    appRunningManager_->RemoveAppRunningRecordById(appRecord->GetRecordId());
    FinishUserTestLocked("App died", -1, appRecord);
    appRecord->SetProcessChangeReason(ProcessChangeReason::REASON_REMOTE_DIED);

    for (const auto &item : appRecord->GetAbilities()) {
        const auto &abilityRecord = item.second;
        //UIExtension notifies Extension & Ability state changes
        if (AAFwk::UIExtensionUtils::IsUIExtension(appRecord->GetExtensionType())) {
            appRecord->StateChangedNotifyObserver(abilityRecord,
                static_cast<int32_t>(ExtensionState::EXTENSION_STATE_TERMINATED), false, false);
        }
        appRecord->StateChangedNotifyObserver(abilityRecord,
            static_cast<int32_t>(AbilityState::ABILITY_STATE_TERMINATED), true, false);
    }
    DelayedSingleton<AppStateObserverManager>::GetInstance()->OnProcessDied(appRecord);
    DelayedSingleton<CacheProcessManager>::GetInstance()->OnProcessKilled(appRecord);

    // kill render if exist.
    KillRenderProcess(appRecord);
#ifdef SUPPORT_CHILD_PROCESS
    KillChildProcess(appRecord);
    KillAttachedChildProcess(appRecord);
#endif // SUPPORT_CHILD_PROCESS

    SendProcessExitEvent(appRecord);

    auto appInfo = appRecord->GetApplicationInfo();
    if (appInfo != nullptr && !appRunningManager_->IsAppExist(appInfo->accessTokenId)) {
        appRecord->UnSetPolicy();
        TAG_LOGW(AAFwkTag::APPMGR, "before OnAppStopped");
        OnAppStopped(appRecord);
    }

    if (appDebugManager_ != nullptr) {
        auto info = MakeAppDebugInfo(appRecord, appRecord->IsDebugApp());
        appDebugManager_->RemoveAppDebugInfo(info);
    }

    ClearAppRunningDataForKeepAlive(appRecord);

    auto uid = appRecord->GetUid();
    TAG_LOGW(AAFwkTag::APPMGR, "before NotifyAppRunningStatusEvent");
    NotifyAppRunningStatusEvent(appRecord->GetBundleName(), uid, AbilityRuntime::RunningStatus::APP_RUNNING_STOP);
}

void AppMgrServiceInner::HandleTimeOut(const AAFwk::EventWrap &event)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    if (!appRunningManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRunningManager null");
        return;
    }

    // check libc.hook_mode
    const int bufferLen = 128;
    char paramOutBuf[bufferLen] = {0};
    const char *hook_mode = "startup:";
    int ret = GetParameter("libc.hook_mode", "", paramOutBuf, bufferLen);
    if (ret > 0 && strncmp(paramOutBuf, hook_mode, strlen(hook_mode)) == 0) {
        TAG_LOGD(AAFwkTag::APPMGR, "HandleTimeOut, Hook_mode: no handle time out");
        return;
    }
    auto appRecord = AppEventUtil::GetInstance().RemoveEvent(event.GetEventId(), event.GetParam());
    switch (event.GetEventId()) {
        case AMSEventHandler::TERMINATE_ABILITY_TIMEOUT_MSG:
            appRunningManager_->HandleTerminateTimeOut(event.GetParam());
            break;
        case AMSEventHandler::TERMINATE_APPLICATION_TIMEOUT_MSG:
            SendHiSysEvent(event.GetEventId(), appRecord);
            HandleTerminateApplicationTimeOut(appRecord);
            break;
        case AMSEventHandler::START_SPECIFIED_PROCESS_TIMEOUT_MSG:
            SendHiSysEvent(event.GetEventId(), appRecord);
            HandleStartSpecifiedProcessTimeout(appRecord);
            break;
        case AMSEventHandler::START_PROCESS_SPECIFIED_ABILITY_TIMEOUT_MSG:
        case AMSEventHandler::ADD_ABILITY_STAGE_INFO_TIMEOUT_MSG:
            SendHiSysEvent(event.GetEventId(), appRecord);
            HandleAddAbilityStageTimeOut(appRecord);
            break;
        case AMSEventHandler::START_SPECIFIED_ABILITY_TIMEOUT_MSG:
            SendHiSysEvent(event.GetEventId(), appRecord);
            HandleStartSpecifiedAbilityTimeOut(appRecord);
            break;
        case AMSEventHandler::TERMINATE_APPLICATION_HALF_TIMEOUT_MSG:
        case AMSEventHandler::START_SPECIFIED_PROCESS_HALF_TIMEOUT_MSG:
        case AMSEventHandler::START_PROCESS_SPECIFIED_ABILITY_HALF_TIMEOUT_MSG:
        case AMSEventHandler::ADD_ABILITY_STAGE_INFO_HALF_TIMEOUT_MSG:
        case AMSEventHandler::START_SPECIFIED_ABILITY_HALF_TIMEOUT_MSG:
            SendHiSysEvent(event.GetEventId(), appRecord);
            break;
        default:
            break;
    }
}

void AppMgrServiceInner::HandleAbilityAttachTimeOut(const sptr<IRemoteObject> &token)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    if (!appRunningManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRunningManager_ null");
        return;
    }
    appRunningManager_->HandleAbilityAttachTimeOut(token, shared_from_this());
}

void AppMgrServiceInner::PrepareTerminate(const sptr<IRemoteObject> &token, bool clearMissionFlag)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    if (!appRunningManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRunningManager_ null");
        return;
    }
    appRunningManager_->PrepareTerminate(token, clearMissionFlag);
}

void AppMgrServiceInner::HandleTerminateApplicationTimeOut(std::shared_ptr<AppRunningRecord> appRecord)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    TerminateApplication(appRecord);
}

void AppMgrServiceInner::TerminateApplication(const std::shared_ptr<AppRunningRecord> &appRecord)
{
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRecord null");
        return;
    }
    appRecord->SetState(ApplicationState::APP_STATE_TERMINATED);
    appRecord->RemoveAppDeathRecipient();
    appRecord->SetProcessChangeReason(ProcessChangeReason::REASON_APP_TERMINATED_TIMEOUT);
    OnAppStateChanged(appRecord, ApplicationState::APP_STATE_TERMINATED, false, false);
    pid_t pid = appRecord->GetPid();
    int32_t uid = appRecord->GetUid();
    if (pid > 0) {
        auto timeoutTask = [appRecord, pid, uid, innerServiceWeak = weak_from_this()]() {
            auto innerService = innerServiceWeak.lock();
            CHECK_POINTER_AND_RETURN_LOG(innerService, "get appMgrServiceInner fail");
            TAG_LOGI(AAFwkTag::APPMGR, "killProcessByPid %{public}d, uid: %{public}d", pid, uid);
            int32_t result = innerService->KillProcessByPid(pid, "TerminateApplication");
            innerService->SendProcessExitEvent(appRecord);
            if (result < 0) {
                TAG_LOGE(AAFwkTag::APPMGR, "killProcessByPid kill process fail");
                return;
            }
        };
        if (!taskHandler_) {
            TAG_LOGE(AAFwkTag::APPMGR, "taskHandler_ null");
            return;
        }
        taskHandler_->SubmitTaskJust(timeoutTask, "DelayKillProcess", AMSEventHandler::KILL_PROCESS_TIMEOUT);
    }
    appRunningManager_->RemoveAppRunningRecordById(appRecord->GetRecordId());
    if (!GetAppRunningStateByBundleName(appRecord->GetBundleName())) {
        RemoveRunningSharedBundleList(appRecord->GetBundleName());
    }
    DelayedSingleton<AppStateObserverManager>::GetInstance()->OnProcessDied(appRecord);
    DelayedSingleton<CacheProcessManager>::GetInstance()->OnProcessKilled(appRecord);
    auto appInfo = appRecord->GetApplicationInfo();
    if (appInfo != nullptr && !appRunningManager_->IsAppExist(appInfo->accessTokenId)) {
        OnAppStopped(appRecord);
    }

    if (appDebugManager_) {
        auto info = MakeAppDebugInfo(appRecord, appRecord->IsDebugApp());
        appDebugManager_->RemoveAppDebugInfo(info);
    }
    ClearAppRunningDataForKeepAlive(appRecord);

    NotifyAppRunningStatusEvent(appRecord->GetBundleName(), uid, AbilityRuntime::RunningStatus::APP_RUNNING_STOP);
}

void AppMgrServiceInner::HandleAddAbilityStageTimeOut(std::shared_ptr<AppRunningRecord> appRecord)
{
    TAG_LOGI(AAFwkTag::APPMGR, "call");
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRecord null");
        return;
    }

    if (appRecord->IsStartSpecifiedAbility() && startSpecifiedAbilityResponse_) {
        startSpecifiedAbilityResponse_->OnTimeoutResponse(appRecord->GetSpecifiedWant(),
            appRecord->GetSpecifiedRequestId());
    }
    appRecord->ResetSpecifiedRequest();

    if (appRecord->IsNewProcessRequest() && startSpecifiedAbilityResponse_) {
        startSpecifiedAbilityResponse_->OnNewProcessRequestTimeoutResponse(appRecord->GetNewProcessRequestWant(),
            appRecord->GetNewProcessRequestId());
    }
    appRecord->ResetNewProcessRequest();

    KillApplicationByRecord(appRecord);
}

void AppMgrServiceInner::GetRunningProcessInfoByToken(
    const sptr<IRemoteObject> &token, AppExecFwk::RunningProcessInfo &info)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    if (!CheckGetRunningInfoPermission()) {
        return;
    }

    appRunningManager_->GetRunningProcessInfoByToken(token, info);
}

int32_t AppMgrServiceInner::GetRunningProcessInfoByPid(const pid_t pid,
    OHOS::AppExecFwk::RunningProcessInfo &info) const
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    if (!CheckGetRunningInfoPermission()) {
        return ERR_PERMISSION_DENIED;
    }

    return appRunningManager_->GetRunningProcessInfoByPid(pid, info);
}

int32_t AppMgrServiceInner::GetRunningProcessInfoByChildProcessPid(const pid_t childPid,
    OHOS::AppExecFwk::RunningProcessInfo &info) const
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    if (!CheckGetRunningInfoPermission()) {
        return ERR_PERMISSION_DENIED;
    }

    return appRunningManager_->GetRunningProcessInfoByChildProcessPid(childPid, info);
}

void AppMgrServiceInner::SetAbilityForegroundingFlagToAppRecord(const pid_t pid) const
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    if (!AAFwk::PermissionVerification::GetInstance()->IsSACall()) {
        return;
    }

    appRunningManager_->SetAbilityForegroundingFlagToAppRecord(pid);
}

bool AppMgrServiceInner::CheckGetRunningInfoPermission() const
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

void AppMgrServiceInner::LoadResidentProcess(const std::vector<AppExecFwk::BundleInfo> &infos)
{
    TAG_LOGI(AAFwkTag::APPMGR, "%{public}s call", __func__);

    TAG_LOGI(AAFwkTag::APPMGR, "info size: [%{public}zu]", infos.size());
    StartResidentProcess(infos, -1, true);
}

void AppMgrServiceInner::StartResidentProcess(const std::vector<BundleInfo> &infos, int restartCount,
    bool isEmptyKeepAliveApp)
{
    TAG_LOGI(AAFwkTag::APPMGR, "startResidentProcess");
    if (infos.empty()) {
        TAG_LOGE(AAFwkTag::APPMGR, "infos null");
        return;
    }

    if (!appRunningManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRunningManager_ null");
        return;
    }

    for (auto &bundle : infos) {
        TAG_LOGI(AAFwkTag::APPMGR, "processName: [%{public}s]", bundle.applicationInfo.process.c_str());
        if (bundle.applicationInfo.process.empty()) {
            continue;
        }
        auto processName = bundle.applicationInfo.process;
        // Inspection records
        auto appRecord = appRunningManager_->CheckAppRunningRecordIsExist(
            bundle.applicationInfo.name, processName, bundle.applicationInfo.uid, bundle);
        if (appRecord) {
            TAG_LOGI(AAFwkTag::APPMGR, "processName [%{public}s] already exists", processName.c_str());
            continue;
        }
        TAG_LOGI(AAFwkTag::APPMGR, "processName: [%{public}s]", processName.c_str());
        StartEmptyResidentProcess(bundle, processName, restartCount, isEmptyKeepAliveApp);
    }
}

void AppMgrServiceInner::StartEmptyResidentProcess(
    const BundleInfo &info, const std::string &processName, int restartCount, bool isEmptyKeepAliveApp)
{
    TAG_LOGI(AAFwkTag::APPMGR, "start bundle [%{public}s | processName [%{public}s]]", info.name.c_str(),
        processName.c_str());
    if (!CheckRemoteClient() || !appRunningManager_) {
        TAG_LOGI(AAFwkTag::APPMGR, "startResidentProcess fail");
        return;
    }

    bool appExistFlag = appRunningManager_->IsAppExist(info.applicationInfo.accessTokenId);
    bool appMultiUserExistFlag = appRunningManager_->CheckAppRunningRecordIsExistByUid(info.uid);
    auto appInfo = std::make_shared<ApplicationInfo>(info.applicationInfo);

    if (!appMultiUserExistFlag) {
        NotifyAppRunningStatusEvent(info.name, appInfo->uid, AbilityRuntime::RunningStatus::APP_RUNNING_START);
    }
    if (UserRecordManager::GetInstance().IsLogoutUser(GetUserIdByUid(appInfo->uid))) {
        TAG_LOGE(AAFwkTag::APPMGR, "disable start process in logout user");
        return;
    }

    auto appRecord = appRunningManager_->CreateAppRunningRecord(appInfo, processName, info, "");
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "start process [%{public}s] fail", processName.c_str());
        return;
    }

    StartProcess(appInfo->name, processName, 0, appRecord, appInfo->uid, info, appInfo->bundleName, 0, appExistFlag);

    // If it is empty, the startup failed
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "start process [%{public}s] fail", processName.c_str());
        return;
    }

    if (restartCount > 0) {
        TAG_LOGI(AAFwkTag::APPMGR, "startEmptyResidentProcess restartCount: [%{public}d]", restartCount);
        appRecord->SetRestartResidentProcCount(restartCount);
    }
    appRecord->SetEmptyKeepAliveAppState(isEmptyKeepAliveApp);
    appRecord->SetKeepAliveEnableState(true);

    appRecord->SetTaskHandler(taskHandler_);
    appRecord->SetEventHandler(eventHandler_);
    std::vector<HapModuleInfo> hapModuleInfos;
    for (auto &iter : info.hapModuleInfos) {
        std::string keepAliveName = (appInfo->process.empty())?(appInfo->bundleName):(appInfo->process);
        std::string moduleProcessName = (iter.process.empty())?(appInfo->bundleName):(iter.process);
        if (keepAliveName == moduleProcessName) {
            hapModuleInfos.emplace_back(iter);
        }
    }
    appRecord->AddModules(appInfo, hapModuleInfos);
    TAG_LOGI(AAFwkTag::APPMGR, "StartEmptyResidentProcess of pid : [%{public}d], ",
        appRecord->GetPid());
}

bool AppMgrServiceInner::CheckRemoteClient()
{
    if (!remoteClientManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "remoteClientManager_ null");
        return false;
    }

    if (!remoteClientManager_->GetSpawnClient()) {
        TAG_LOGE(AAFwkTag::APPMGR, "appSpawnClient null");
        return false;
    }

    if (!remoteClientManager_->GetBundleManagerHelper()) {
        TAG_LOGE(AAFwkTag::APPMGR, "get bundle manager helper fail");
        return false;
    }
    return true;
}

void AppMgrServiceInner::RestartKeepAliveProcess(std::shared_ptr<AppRunningRecord> appRecord)
{
    TAG_LOGI(AAFwkTag::APPMGR, "restart keep-alive process begins.");
    if (appRecord == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRecord null");
        return;
    }

    if (!CheckRemoteClient() || !appRecord || !appRunningManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "restart keep-alive fail");
        return;
    }

    auto bundleMgrHelper = remoteClientManager_->GetBundleManagerHelper();
    BundleInfo bundleInfo;
    auto userId = GetUserIdByUid(appRecord->GetUid());
    auto flags = static_cast<int32_t>(GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_APPLICATION)
        | static_cast<int32_t>(GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_HAP_MODULE)
        | static_cast<int32_t>(GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_ABILITY)
        | static_cast<int32_t>(GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_EXTENSION_ABILITY);
    if (IN_PROCESS_CALL(bundleMgrHelper->GetCloneBundleInfo(appRecord->GetBundleName(), flags,
        appRecord->GetAppIndex(), bundleInfo, userId)) != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "getCloneBundleInfo fail");
        return;
    }
    std::vector<BundleInfo> infos;
    infos.emplace_back(bundleInfo);
    TAG_LOGI(AAFwkTag::APPMGR, "keepAliveProcess %{public}s", appRecord->GetProcessName().c_str());
    NotifyStartKeepAliveProcess(infos);
}

void AppMgrServiceInner::RestartResidentProcess(std::shared_ptr<AppRunningRecord> appRecord)
{
    if (appRecord == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRecord null");
        return;
    }
    struct timespec t;
    t.tv_sec = 0;
    t.tv_nsec = 0;
    clock_gettime(CLOCK_MONOTONIC, &t);
    appRecord->SetRestartTimeMillis(static_cast<int64_t>(((t.tv_sec) * NANOSECONDS + t.tv_nsec) / MICROSECONDS));
    appRecord->DecRestartResidentProcCount();

    auto findRestartResidentTask = [appRecord](const std::shared_ptr<AppRunningRecord> &appRunningRecord) {
        return (appRecord != nullptr && appRecord->GetBundleName() == appRunningRecord->GetBundleName());
    };

    {
        std::lock_guard guard(restartResidentTaskListMutex_);
        auto findIter = find_if(restartResidentTaskList_.begin(), restartResidentTaskList_.end(),
            findRestartResidentTask);
        if (findIter != restartResidentTaskList_.end()) {
            restartResidentTaskList_.erase(findIter);
        }
    }

    if (!CheckRemoteClient() || !appRecord || !appRunningManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "restart resident fail");
        return;
    }

    auto bundleMgrHelper = remoteClientManager_->GetBundleManagerHelper();
    BundleInfo bundleInfo;
    auto userId = GetUserIdByUid(appRecord->GetUid());
    auto flags = BundleFlag::GET_BUNDLE_DEFAULT | BundleFlag::GET_BUNDLE_WITH_REQUESTED_PERMISSION;
    if (!IN_PROCESS_CALL(bundleMgrHelper->GetBundleInfo(
        appRecord->GetBundleName(),
        static_cast<BundleFlag>(flags),
        bundleInfo, userId))) {
        TAG_LOGE(AAFwkTag::APPMGR, "getBundleInfo fail");
        return;
    }
    std::vector<BundleInfo> infos;
    infos.emplace_back(bundleInfo);
    TAG_LOGI(AAFwkTag::APPMGR, "residentProcess [%{public}s] remaining restarts num: [%{public}d]",
        appRecord->GetProcessName().c_str(), (int)appRecord->GetRestartResidentProcCount());
    StartResidentProcess(infos, appRecord->GetRestartResidentProcCount(), appRecord->IsEmptyKeepAliveApp());
}

void AppMgrServiceInner::NotifyAppStatus(const std::string &bundleName, int32_t appIndex, const std::string &eventData)
{
    TAG_LOGD(AAFwkTag::APPMGR, "bundle name is %{public}s, event is %{public}s",
        bundleName.c_str(), eventData.c_str());
    Want want;
    want.SetAction(eventData);
    ElementName element;
    element.SetBundleName(bundleName);
    want.SetElement(element);
    want.SetParam(Constants::USER_ID, 0);
    want.SetParam(Constants::APP_INDEX, appIndex);
    EventFwk::CommonEventData commonData {want};
    EventFwk::CommonEventManager::PublishCommonEvent(commonData);
}

void AppMgrServiceInner::NotifyAppStatusByCallerUid(const std::string &bundleName, const int32_t tokenId,
    const int32_t userId, const int32_t callerUid, const int32_t targetUid, const std::string &eventData)
{
    TAG_LOGI(AAFwkTag::APPMGR,
        "%{public}s call, bundle name: %{public}s, userId: %{public}d, event: %{public}s", __func__,
        bundleName.c_str(), userId, eventData.c_str());
    Want want;
    want.SetAction(eventData);
    ElementName element;
    element.SetBundleName(bundleName);
    want.SetElement(element);
    want.SetParam(TOKEN_ID, tokenId);
    want.SetParam(Constants::USER_ID, userId);
    want.SetParam(Constants::UID, callerUid);
    want.SetParam(Want::PARAM_RESV_CALLER_UID, callerUid);
    want.SetParam(TARGET_UID_KEY, targetUid);
    EventFwk::CommonEventData commonData {want};
    EventFwk::CommonEventManager::PublishCommonEvent(commonData);
}

int32_t AppMgrServiceInner::RegisterApplicationStateObserver(
    const sptr<IApplicationStateObserver> &observer, const std::vector<std::string> &bundleNameList)
{
    return DelayedSingleton<AppStateObserverManager>::GetInstance()->RegisterApplicationStateObserver(
        observer, bundleNameList);
}

int32_t AppMgrServiceInner::UnregisterApplicationStateObserver(const sptr<IApplicationStateObserver> &observer)
{
    return DelayedSingleton<AppStateObserverManager>::GetInstance()->UnregisterApplicationStateObserver(observer);
}

int32_t AppMgrServiceInner::RegisterAppForegroundStateObserver(const sptr<IAppForegroundStateObserver> &observer)
{
    CHECK_CALLER_IS_SYSTEM_APP;
    return DelayedSingleton<AppStateObserverManager>::GetInstance()->RegisterAppForegroundStateObserver(observer);
}

int32_t AppMgrServiceInner::UnregisterAppForegroundStateObserver(const sptr<IAppForegroundStateObserver> &observer)
{
    CHECK_CALLER_IS_SYSTEM_APP;
    return DelayedSingleton<AppStateObserverManager>::GetInstance()->UnregisterAppForegroundStateObserver(observer);
}

int32_t AppMgrServiceInner::RegisterAbilityForegroundStateObserver(
    const sptr<IAbilityForegroundStateObserver> &observer)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    CHECK_CALLER_IS_SYSTEM_APP;
    return DelayedSingleton<AppStateObserverManager>::GetInstance()->RegisterAbilityForegroundStateObserver(observer);
}

int32_t AppMgrServiceInner::UnregisterAbilityForegroundStateObserver(
    const sptr<IAbilityForegroundStateObserver> &observer)
{
    CHECK_CALLER_IS_SYSTEM_APP;
    return DelayedSingleton<AppStateObserverManager>::GetInstance()->UnregisterAbilityForegroundStateObserver(observer);
}

int32_t AppMgrServiceInner::GetForegroundApplications(std::vector<AppStateData> &list)
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

int AppMgrServiceInner::StartUserTestProcess(
    const AAFwk::Want &want, const sptr<IRemoteObject> &observer, const BundleInfo &bundleInfo, int32_t userId)
{
    TAG_LOGI(AAFwkTag::APPMGR, "enter");
    if (!observer) {
        TAG_LOGE(AAFwkTag::APPMGR, "observer null");
        return ERR_INVALID_VALUE;
    }
    if (!appRunningManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRunningManager_ null");
        return ERR_INVALID_VALUE;
    }

    std::string bundleName = want.GetStringParam("-b");
    if (bundleName.empty()) {
        TAG_LOGE(AAFwkTag::APPMGR, "invalid bundle name");
        return ERR_INVALID_VALUE;
    }

    KillProcessConfig config{false, false, "StartUserTestProcess"};
    if (KillApplicationByUserIdLocked(bundleName, 0, userId, config)) {
        TAG_LOGE(AAFwkTag::APPMGR, "kill app fail");
        return ERR_INVALID_VALUE;
    }

    HapModuleInfo hapModuleInfo;
    if (GetHapModuleInfoForTestRunner(want, observer, bundleInfo, hapModuleInfo)) {
        TAG_LOGE(AAFwkTag::APPMGR, "get HapModuleInfo fail");
        return ERR_INVALID_VALUE;
    }

    std::string processName;
    MakeProcessName(std::make_shared<ApplicationInfo>(bundleInfo.applicationInfo), hapModuleInfo, processName);
    TAG_LOGI(AAFwkTag::APPMGR, "processName: [%{public}s]", processName.c_str());

    // Inspection records
    auto appRecord = appRunningManager_->CheckAppRunningRecordIsExist(
        bundleInfo.applicationInfo.name, processName, bundleInfo.applicationInfo.uid, bundleInfo);
    if (appRecord) {
        TAG_LOGI(AAFwkTag::APPMGR, "processName [%{public}s] already exists", processName.c_str());
        return ERR_INVALID_VALUE;
    }

    return StartEmptyProcess(want, observer, bundleInfo, processName, userId);
}

int AppMgrServiceInner::GetHapModuleInfoForTestRunner(const AAFwk::Want &want, const sptr<IRemoteObject> &observer,
    const BundleInfo &bundleInfo, HapModuleInfo &hapModuleInfo)
{
    TAG_LOGI(AAFwkTag::APPMGR, "enter");
    if (!observer) {
        TAG_LOGE(AAFwkTag::APPMGR, "observer null");
        return ERR_INVALID_VALUE;
    }

    bool moduleJson = false;
    if (!bundleInfo.hapModuleInfos.empty()) {
        moduleJson = bundleInfo.hapModuleInfos.back().isModuleJson;
    }
    if (moduleJson) {
        std::string moduleName = want.GetStringParam("-m");
        if (moduleName.empty()) {
            UserTestAbnormalFinish(observer, "No module name is specified.");
            return ERR_INVALID_VALUE;
        }

        bool found = false;
        for (auto item : bundleInfo.hapModuleInfos) {
            if (item.moduleName == moduleName) {
                hapModuleInfo = item;
                found = true;
                break;
            }
        }
        if (!found) {
            UserTestAbnormalFinish(observer, "The specified module name is not found.");
            return ERR_INVALID_VALUE;
        }
    }
    return ERR_OK;
}

int AppMgrServiceInner::UserTestAbnormalFinish(const sptr<IRemoteObject> &observer, const std::string &msg)
{
    sptr<AAFwk::ITestObserver> observerProxy = iface_cast<AAFwk::ITestObserver>(observer);
    if (!observerProxy) {
        TAG_LOGE(AAFwkTag::APPMGR, "get ITestObserver proxy fail");
        return ERR_INVALID_VALUE;
    }
    observerProxy->TestFinished(msg, -1);
    return ERR_OK;
}

int AppMgrServiceInner::StartEmptyProcess(const AAFwk::Want &want, const sptr<IRemoteObject> &observer,
    const BundleInfo &info, const std::string &processName, const int userId)
{
    TAG_LOGI(AAFwkTag::APPMGR, "enter bundle [%{public}s | processName [%{public}s]]", info.name.c_str(),
        processName.c_str());
    if (!CheckRemoteClient() || !appRunningManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "start test fail");
        return ERR_INVALID_VALUE;
    }

    bool appExistFlag = appRunningManager_->IsAppExist(info.applicationInfo.accessTokenId);
    bool appMultiUserExistFlag = appRunningManager_->CheckAppRunningRecordIsExistByUid(info.uid);
    auto appInfo = std::make_shared<ApplicationInfo>(info.applicationInfo);
    if (!appMultiUserExistFlag) {
        NotifyAppRunningStatusEvent(info.name, appInfo->uid, AbilityRuntime::RunningStatus::APP_RUNNING_START);
    }
    if (UserRecordManager::GetInstance().IsLogoutUser(GetUserIdByUid(appInfo->uid))) {
        TAG_LOGE(AAFwkTag::APPMGR, "disable start process in logout user");
        return ERR_INVALID_OPERATION;
    }
    auto appRecord = appRunningManager_->CreateAppRunningRecord(appInfo, processName, info, "");
    CHECK_POINTER_AND_RETURN_VALUE(appRecord, ERR_INVALID_VALUE);

    auto isDebug = want.GetBoolParam(DEBUG_APP, false);
    TAG_LOGI(AAFwkTag::APPMGR, "setDebug: %{public}s", (isDebug ? "true" : "false"));
    appRecord->SetDebugApp(isDebug);
    bool isDebugFromLocal = want.GetBoolParam(DEBUG_FROM, false);
    TAG_LOGI(AAFwkTag::APPMGR, "SetDebugFromLocal: %{public}d", isDebugFromLocal);
    appRecord->SetDebugFromLocal(isDebugFromLocal);
    if (want.GetBoolParam(COLD_START, false)) {
        appRecord->SetDebugApp(true);
    }

    std::shared_ptr<UserTestRecord> testRecord = std::make_shared<UserTestRecord>();
    if (!testRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "make userTestRecord fail");
        return ERR_INVALID_VALUE;
    }
    testRecord->want = want;
    testRecord->observer = observer;
    testRecord->isFinished = false;
    testRecord->userId = userId;
    appRecord->SetUserTestInfo(testRecord);

    int32_t appIndex = 0;
    (void)AbilityRuntime::StartupUtil::GetAppIndex(want, appIndex);
    uint32_t startFlags = AppspawnUtil::BuildStartFlags(want, info.applicationInfo);
    StartProcess(appInfo->name, processName, startFlags, appRecord, appInfo->uid, info, appInfo->bundleName,
        appIndex, appExistFlag);

    // If it is empty, the startup failed
    CHECK_POINTER_AND_RETURN_VALUE(appRecord, ERR_INVALID_VALUE);
    appRecord->SetTaskHandler(taskHandler_);
    appRecord->SetEventHandler(eventHandler_);
    appRecord->AddModules(appInfo, info.hapModuleInfos);
    TAG_LOGI(AAFwkTag::APPMGR, "startEmptyProcess pid: [%{public}d]", appRecord->GetPid());

    return ERR_OK;
}

int AppMgrServiceInner::FinishUserTest(
    const std::string &msg, const int64_t &resultCode, const std::string &bundleName, const pid_t &pid)
{
    TAG_LOGI(AAFwkTag::APPMGR, "enter");
    if (bundleName.empty()) {
        TAG_LOGE(AAFwkTag::APPMGR, "invalid bundle name");
        return ERR_INVALID_VALUE;
    }
    auto appRecord = GetAppRunningRecordByPid(pid);
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "no appRecord");
        return ERR_INVALID_VALUE;
    }

    auto userTestRecord = appRecord->GetUserTestInfo();
    if (!userTestRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "unstart user test");
        return ERR_INVALID_VALUE;
    }

    FinishUserTestLocked(msg, resultCode, appRecord);

    KillProcessConfig config{false, false, "FinishUserTest"};
    int ret = KillApplicationByUserIdLocked(bundleName, 0, userTestRecord->userId, config);
    if (ret) {
        TAG_LOGE(AAFwkTag::APPMGR, "kill process fail");
        return ret;
    }

    return ERR_OK;
}

int AppMgrServiceInner::FinishUserTestLocked(
    const std::string &msg, const int64_t &resultCode, const std::shared_ptr<AppRunningRecord> &appRecord)
{
    TAG_LOGD(AAFwkTag::APPMGR, "Enter");
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "invalid appRecord");
        return ERR_INVALID_VALUE;
    }

    std::lock_guard<ffrt::mutex> lock(userTestLock_);
    auto userTestRecord = appRecord->GetUserTestInfo();
    if (!userTestRecord) {
        TAG_LOGD(AAFwkTag::APPMGR, "not start user test");
        return ERR_INVALID_VALUE;
    }
    if (!userTestRecord->isFinished) {
        sptr<AAFwk::ITestObserver> observerProxy = iface_cast<AAFwk::ITestObserver>(userTestRecord->observer);
        if (!observerProxy) {
            TAG_LOGE(AAFwkTag::APPMGR, "get iTestObserver proxy fail");
            return ERR_INVALID_VALUE;
        }
        observerProxy->TestFinished(msg, resultCode);

        userTestRecord->isFinished = true;
    }

    return ERR_OK;
}

void AppMgrServiceInner::StartSpecifiedAbility(const AAFwk::Want &want, const AppExecFwk::AbilityInfo &abilityInfo,
    int32_t requestId)
{
    TAG_LOGD(AAFwkTag::APPMGR, "Start specified ability.");
    if (!CheckRemoteClient()) {
        return;
    }

    BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    auto appInfo = std::make_shared<ApplicationInfo>(abilityInfo.applicationInfo);

    int32_t appIndex = 0;
    (void)AbilityRuntime::StartupUtil::GetAppIndex(want, appIndex);
    if (!GetBundleAndHapInfo(abilityInfo, appInfo, bundleInfo, hapModuleInfo, appIndex)) {
        return;
    }
    if (UserRecordManager::GetInstance().IsLogoutUser(GetUserIdByUid(appInfo->uid))) {
        TAG_LOGE(AAFwkTag::APPMGR, "disable start process in logout user");
        return;
    }

    std::string processName;
    auto abilityInfoPtr = std::make_shared<AbilityInfo>(abilityInfo);
    MakeProcessName(abilityInfoPtr, appInfo, hapModuleInfo, appIndex, "", processName, false);

    std::vector<HapModuleInfo> hapModules;
    hapModules.emplace_back(hapModuleInfo);

    std::shared_ptr<AppRunningRecord> appRecord;
    auto instanceKey = want.GetStringParam(Want::APP_INSTANCE_KEY);
    auto customProcessFlag = abilityInfo.process;
    appRecord = appRunningManager_->CheckAppRunningRecordIsExist(appInfo->name, processName, appInfo->uid, bundleInfo,
        "", nullptr, instanceKey, customProcessFlag);
    if (!appRecord) {
        bool appExistFlag = appRunningManager_->IsAppExist(appInfo->accessTokenId);
        bool appMultiUserExistFlag = appRunningManager_->CheckAppRunningRecordIsExistByUid(bundleInfo.uid);
        if (!appMultiUserExistFlag) {
            NotifyAppRunningStatusEvent(
                bundleInfo.name, appInfo->uid, AbilityRuntime::RunningStatus::APP_RUNNING_START);
        }
        // new app record
        appRecord = appRunningManager_->CreateAppRunningRecord(appInfo, processName, bundleInfo, instanceKey,
            customProcessFlag);
        if (!appRecord) {
            TAG_LOGE(AAFwkTag::APPMGR, "start process [%{public}s] fail", processName.c_str());
            return;
        }
        if (hapModuleInfo.isStageBasedModel && !IsMainProcess(appInfo, processName)) {
            appRecord->SetEmptyKeepAliveAppState(false);
            appRecord->SetMainProcess(false);
            TAG_LOGD(AAFwkTag::APPMGR, "The process %{public}s will not keepalive", hapModuleInfo.process.c_str());
        }
        auto wantPtr = std::make_shared<AAFwk::Want>(want);
        if (wantPtr != nullptr) {
            appRecord->SetCallerPid(wantPtr->GetIntParam(Want::PARAM_RESV_CALLER_PID, -1));
            appRecord->SetCallerUid(wantPtr->GetIntParam(Want::PARAM_RESV_CALLER_UID, -1));
            appRecord->SetCallerTokenId(wantPtr->GetIntParam(Want::PARAM_RESV_CALLER_TOKEN, -1));
            appRecord->SetDebugApp(wantPtr->GetBoolParam(DEBUG_APP, false));
            appRecord->SetDebugFromLocal(wantPtr->GetBoolParam(DEBUG_FROM, false));
            if (appRecord->IsDebugApp()) {
                ProcessAppDebug(appRecord, true);
            }
            appRecord->SetNativeDebug(wantPtr->GetBoolParam("nativeDebug", false));
            if (wantPtr->GetBoolParam(COLD_START, false)) {
                appRecord->SetDebugApp(true);
            }
            appRecord->SetPerfCmd(wantPtr->GetStringParam(PERF_CMD));
            appRecord->SetErrorInfoEnhance(wantPtr->GetBoolParam(ERROR_INFO_ENHANCE, false));
            appRecord->SetMultiThread(wantPtr->GetBoolParam(MULTI_THREAD, false));
        }
        appRecord->SetProcessAndExtensionType(abilityInfoPtr);
        appRecord->SetTaskHandler(taskHandler_);
        appRecord->SetEventHandler(eventHandler_);
        appRecord->SendEventForSpecifiedAbility();
        appRecord->SetAppIndex(appIndex);
        uint32_t startFlags = AppspawnUtil::BuildStartFlags(want, abilityInfo);
        StartProcess(appInfo->name, processName, startFlags, appRecord, appInfo->uid, bundleInfo, appInfo->bundleName,
            appIndex, appExistFlag);

        appRecord->SetSpecifiedAbilityFlagAndWant(requestId, want, hapModuleInfo.moduleName);
        appRecord->AddModules(appInfo, hapModules);
    } else {
        TAG_LOGD(AAFwkTag::APPMGR, "process is exist");
        auto isDebugApp = want.GetBoolParam(DEBUG_APP, false);
        if (isDebugApp && !appRecord->IsDebugApp()) {
            ProcessAppDebug(appRecord, isDebugApp);
        }

        appRecord->SetSpecifiedAbilityFlagAndWant(requestId, want, hapModuleInfo.moduleName);
        auto moduleRecord = appRecord->GetModuleRecordByModuleName(appInfo->bundleName, hapModuleInfo.moduleName);
        if (!moduleRecord) {
            TAG_LOGD(AAFwkTag::APPMGR, "module record is nullptr, add modules");
            appRecord->AddModules(appInfo, hapModules);
            appRecord->AddAbilityStageBySpecifiedAbility(appInfo->bundleName);
        } else if (!appRecord->AddAbilityStageBySpecifiedAbility(appInfo->bundleName)) {
            TAG_LOGD(AAFwkTag::APPMGR, "schedule accept want");
            appRecord->ScheduleAcceptWant(hapModuleInfo.moduleName);
        }
    }
}

void AppMgrServiceInner::RegisterStartSpecifiedAbilityResponse(const sptr<IStartSpecifiedAbilityResponse> &response)
{
    if (!response) {
        TAG_LOGE(AAFwkTag::APPMGR, "response null");
        return;
    }
    startSpecifiedAbilityResponse_ = response;
}

void AppMgrServiceInner::ScheduleAcceptWantDone(
    const int32_t recordId, const AAFwk::Want &want, const std::string &flag)
{
    TAG_LOGD(AAFwkTag::APPMGR, "ScheduleAcceptWantDone, flag: %{private}s", flag.c_str());

    auto appRecord = GetAppRunningRecordByAppRecordId(recordId);
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "get appRecord fail");
        return;
    }
    auto requestId = appRecord->GetSpecifiedRequestId();
    appRecord->ScheduleAcceptWantDone();
    appRecord->ResetSpecifiedRequest();

    if (startSpecifiedAbilityResponse_) {
        startSpecifiedAbilityResponse_->OnAcceptWantResponse(want, flag, requestId);
    }
}

void AppMgrServiceInner::SchedulePrepareTerminate(const pid_t pid, const std::string &moduleName)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    if (IPCSkeleton::GetCallingUid() != FOUNDATION_UID) {
        TAG_LOGE(AAFwkTag::APPMGR, "not foundation call");
        return;
    }
    auto appRecord = GetAppRunningRecordByPid(pid);
    if (appRecord == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "get appRecord fail");
        return;
    }
    appRecord->SchedulePrepareTerminate(moduleName);
}

void AppMgrServiceInner::HandleStartSpecifiedAbilityTimeOut(std::shared_ptr<AppRunningRecord> appRecord)
{
    TAG_LOGI(AAFwkTag::APPMGR, "startSpecifiedAbility timeOut");
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRecord null");
        return;
    }

    if (appRecord->IsStartSpecifiedAbility() && startSpecifiedAbilityResponse_) {
        startSpecifiedAbilityResponse_->OnTimeoutResponse(appRecord->GetSpecifiedWant(),
            appRecord->GetSpecifiedRequestId());
    }
    appRecord->ResetSpecifiedRequest();

    KillApplicationByRecord(appRecord);
}

void AppMgrServiceInner::ScheduleNewProcessRequestDone(
    const int32_t recordId, const AAFwk::Want &want, const std::string &flag)
{
    TAG_LOGD(AAFwkTag::APPMGR, "ScheduleNewProcessRequestDone, flag: %{public}s", flag.c_str());

    auto appRecord = GetAppRunningRecordByAppRecordId(recordId);
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "get appRecord fail");
        return;
    }
    auto requestId = appRecord->GetNewProcessRequestId();
    appRecord->ScheduleNewProcessRequestDone();
    appRecord->ResetNewProcessRequest();

    if (startSpecifiedAbilityResponse_) {
        startSpecifiedAbilityResponse_->OnNewProcessRequestResponse(want, flag, requestId);
    }
    appRecord->ResetNewProcessRequest();
}

void AppMgrServiceInner::HandleStartSpecifiedProcessTimeout(std::shared_ptr<AppRunningRecord> appRecord)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called start specified process time out!");
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRecord null");
        return;
    }

    if (startSpecifiedAbilityResponse_) {
        startSpecifiedAbilityResponse_->OnNewProcessRequestTimeoutResponse(appRecord->GetNewProcessRequestWant(),
            appRecord->GetNewProcessRequestId());
    }
    appRecord->ResetNewProcessRequest();
}

int32_t AppMgrServiceInner::DealWithUserConfiguration(const Configuration& config, const int32_t userId,
    int32_t &notifyUserId)
{
    if (multiUserConfigurationMgr_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "multiUserConfigurationMgr_ null");
        return ERR_INVALID_VALUE;
    }
    std::vector<std::string> changeKeyV;
    bool isNotifyUser0 = false;
    multiUserConfigurationMgr_->HandleConfiguration(userId, config, changeKeyV, isNotifyUser0);
    TAG_LOGI(AAFwkTag::APPMGR,
        "changeKeyV size: %{public}zu Config: %{public}s userId: %{public}d, NotifyUser0: %{public}d",
        changeKeyV.size(), config.GetName().c_str(), userId, static_cast<int32_t>(isNotifyUser0));

    if (!config.GetItem(AAFwk::GlobalConfigurationKey::THEME).empty() || !changeKeyV.empty()) {
        notifyUserId = userId;
        return ERR_OK;
    } else if (isNotifyUser0) {
        notifyUserId = U0_USER_ID;
        return ERR_OK;
    } else {
        TAG_LOGE(AAFwkTag::APPMGR, "changeKeyV empty");
        return ERR_INVALID_VALUE;
    }
}

int32_t AppMgrServiceInner::UpdateConfiguration(const Configuration &config, const int32_t userId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (!appRunningManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRunningManager_ null");
        return ERR_INVALID_VALUE;
    }
    CHECK_CALLER_IS_SYSTEM_APP;
    auto ret = AAFwk::PermissionVerification::GetInstance()->VerifyUpdateConfigurationPerm();
    if (ret != ERR_OK) {
        return ret;
    }
    int32_t notifyUserId;
    ret = DealWithUserConfiguration(config, userId, notifyUserId);
    if (ret != ERR_OK) {
        return ret;
    }

    // all app
    int32_t result = appRunningManager_->UpdateConfiguration(config, notifyUserId);
    HandleConfigurationChange(config, notifyUserId);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "update error");
        return result;
    }
    // notify
    std::lock_guard<ffrt::mutex> notifyLock(configurationObserverLock_);
    for (auto &item : configurationObservers_) {
        if (item.observer != nullptr && (notifyUserId == -1 || item.userId == 0 || item.userId == notifyUserId)) {
            item.observer->OnConfigurationUpdated(config);
        }
    }

    return result;
}

int32_t AppMgrServiceInner::UpdateConfigurationByBundleName(const Configuration &config, const std::string &name,
    int32_t appIndex)
{
    if (!appRunningManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRunningManager_ null");
        return ERR_INVALID_VALUE;
    }
    CHECK_CALLER_IS_SYSTEM_APP;
    auto ret = AAFwk::PermissionVerification::GetInstance()->VerifyUpdateAPPConfigurationPerm();
    if (ret != ERR_OK) {
        return ret;
    }
    int32_t result = appRunningManager_->UpdateConfigurationByBundleName(config, name, appIndex);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "update error");
        return result;
    }
    return result;
}

void AppMgrServiceInner::HandleConfigurationChange(const Configuration& config, const int32_t userId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::lock_guard lock(appStateCallbacksLock_);

    for (const auto &item : appStateCallbacks_) {
        if (item.callback != nullptr && (userId == -1 || item.userId == 0 || item.userId == userId)) {
            item.callback->NotifyConfigurationChange(config, currentUserId_);
        }
    }
}

int32_t AppMgrServiceInner::RegisterConfigurationObserver(const sptr<IConfigurationObserver>& observer)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    if (!AAFwk::PermissionVerification::GetInstance()->IsSACall()) {
        TAG_LOGE(AAFwkTag::APPMGR, "caller not SA");
        return ERR_INVALID_VALUE;
    }

    if (observer == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "observer null");
        return ERR_INVALID_VALUE;
    }
    std::lock_guard<ffrt::mutex> registerLock(configurationObserverLock_);
    auto it = std::find_if(configurationObservers_.begin(), configurationObservers_.end(),
        [&observer](const ConfigurationObserverWithUserId& item) {
            return (item.observer && item.observer->AsObject() == observer->AsObject());
        });
    if (it != configurationObservers_.end()) {
        TAG_LOGE(AAFwkTag::APPMGR, "observer exist");
        return ERR_INVALID_VALUE;
    }
    configurationObservers_.push_back(
        ConfigurationObserverWithUserId { observer, GetUserIdByUid(IPCSkeleton::GetCallingUid()) });
    return NO_ERROR;
}

int32_t AppMgrServiceInner::UnregisterConfigurationObserver(const sptr<IConfigurationObserver>& observer)
{
    TAG_LOGI(AAFwkTag::APPMGR, "call");
    if (!AAFwk::PermissionVerification::GetInstance()->IsSACall()) {
        TAG_LOGE(AAFwkTag::APPMGR, "caller not SA");
        return ERR_INVALID_VALUE;
    }
    if (observer == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "observer null");
        return ERR_INVALID_VALUE;
    }
    std::lock_guard<ffrt::mutex> unregisterLock(configurationObserverLock_);
    auto it = std::find_if(configurationObservers_.begin(), configurationObservers_.end(),
        [&observer](const ConfigurationObserverWithUserId &item) {
            return (item.observer && item.observer->AsObject() == observer->AsObject());
        });
    if (it != configurationObservers_.end()) {
        configurationObservers_.erase(it);
        return NO_ERROR;
    }
    TAG_LOGI(AAFwkTag::APPMGR, "end");
    return ERR_INVALID_VALUE;
}

void AppMgrServiceInner::InitGlobalConfiguration()
{
    std::shared_ptr<AppExecFwk::Configuration> globalConfiguration = std::make_shared<Configuration>();
    if (!globalConfiguration) {
        TAG_LOGE(AAFwkTag::APPMGR, "globalConfiguration null");
        return;
    }

#ifdef SUPPORT_SCREEN
    // Currently only this interface is known
    auto language = OHOS::Global::I18n::LocaleConfig::GetEffectiveLanguage();
    TAG_LOGI(AAFwkTag::APPMGR, "current global language: %{public}s", language.c_str());
    globalConfiguration->AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE, language);
    std::string sysHour = OHOS::Global::I18n::LocaleConfig::GetSystemHour();
    TAG_LOGI(AAFwkTag::APPMGR, "current 24 hour clock: %{public}s", sysHour.c_str());
    globalConfiguration->AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_HOUR, sysHour);
#endif

    // Assign to default colorMode "light"
    TAG_LOGI(AAFwkTag::APPMGR, "current global colorMode: %{public}s", ConfigurationInner::COLOR_MODE_LIGHT);
    globalConfiguration->AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_COLORMODE, ConfigurationInner::COLOR_MODE_LIGHT);

    // Get input pointer device
    std::string hasPointerDevice = system::GetParameter(AAFwk::GlobalConfigurationKey::INPUT_POINTER_DEVICE, "false");
    TAG_LOGI(AAFwkTag::APPMGR, "current hasPointerDevice: %{public}s", hasPointerDevice.c_str());
    globalConfiguration->AddItem(AAFwk::GlobalConfigurationKey::INPUT_POINTER_DEVICE, hasPointerDevice);

    // Get DeviceType
    auto deviceType = GetDeviceType();
    TAG_LOGI(AAFwkTag::APPMGR, "current deviceType: %{public}s", deviceType);
    globalConfiguration->AddItem(AAFwk::GlobalConfigurationKey::DEVICE_TYPE, deviceType);
    globalConfiguration->AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_FONT_SIZE_SCALE, "1.0");
    globalConfiguration->AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_FONT_WEIGHT_SCALE, "1.0");
    if (multiUserConfigurationMgr_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "multiUserConfigurationMgr_ null");
        return;
    }
    multiUserConfigurationMgr_->InitConfiguration(globalConfiguration);
    TAG_LOGI(AAFwkTag::APPMGR, "InitGlobalConfiguration Config: %{public}s", globalConfiguration->GetName().c_str());
}

std::shared_ptr<AppExecFwk::Configuration> AppMgrServiceInner::GetConfiguration()
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

void AppMgrServiceInner::KillApplicationByRecord(const std::shared_ptr<AppRunningRecord> &appRecord)
{
    TAG_LOGD(AAFwkTag::APPMGR, "Kill application by appRecord.");
    if (!appRecord || !taskHandler_) {
        TAG_LOGW(AAFwkTag::APPMGR, "appRecord or taskHandler_ null");
        return;
    }

    auto pid = appRecord->GetPid();
    appRecord->SetTerminating();
    appRecord->ScheduleProcessSecurityExit();

    auto startTime = SystemTimeMillisecond();
    std::list<pid_t> pids = {pid};
    if (WaitForRemoteProcessExit(pids, startTime)) {
        TAG_LOGI(AAFwkTag::APPMGR, "remote process exited success");
        return;
    }

    auto timeoutTask = [pid, innerServiceWeak = weak_from_this()]() {
        auto innerService = innerServiceWeak.lock();
        CHECK_POINTER_AND_RETURN_LOG(innerService, "get appMgrServiceInner fail");
        TAG_LOGI(AAFwkTag::APPMGR, "killProcessByPid: %{public}d", pid);
        int32_t result = innerService->KillProcessByPid(pid, "KillApplicationByRecord");
        if (result < 0) {
            TAG_LOGE(AAFwkTag::APPMGR, "kill app fail, pid: %{public}d", pid);
            return;
        }
    };
    taskHandler_->SubmitTaskJust(timeoutTask, "DelayKillProcess", AMSEventHandler::KILL_PROCESS_TIMEOUT);
}

void AppMgrServiceInner::SendHiSysEvent(int32_t innerEventId, std::shared_ptr<AppRunningRecord> appRecord)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called AppMgrServiceInner SendHiSysEvent!");
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRecord null");
        return;
    }
    const int bufferLen = 128;
    char paramOutBuf[bufferLen] = {0};
    const char *hook_mode = "startup:";
    int ret = GetParameter("libc.hook_mode", "", paramOutBuf, bufferLen);
    if (ret > 0 && strncmp(paramOutBuf, hook_mode, strlen(hook_mode)) == 0) {
        TAG_LOGD(AAFwkTag::APPMGR, "SendHiSysEvent, Hook_mode: no handle time out");
        return;
    }

    std::string eventName = GetEventName(innerEventId);
    int32_t pid = appRecord->GetPid();
    int32_t uid = appRecord->GetUid();
    std::string packageName = appRecord->GetBundleName();
    std::string processName = appRecord->GetProcessName();
    std::string msg = eventName + ",";
    auto eventTypeAndMsg = GetEventTypeAndMsg(innerEventId);
    msg += eventTypeAndMsg.second;

    TAG_LOGW(AAFwkTag::APPMGR, "lifecycle_timeout, eventName: %{public}s, uid: %{public}d, pid: %{public}d, \
        packageName: %{public}s, processName: %{public}s, msg: %{public}s",
        eventName.c_str(), uid, pid, packageName.c_str(), processName.c_str(), msg.c_str());
    AppfreezeManager::ParamInfo info = {
        .typeId = eventTypeAndMsg.first,
        .pid = pid,
        .eventName = eventName,
        .bundleName = packageName,
        .msg = msg
    };
    AppfreezeManager::GetInstance()->LifecycleTimeoutHandle(info);
}

int AppMgrServiceInner::GetAbilityRecordsByProcessID(const int pid, std::vector<sptr<IRemoteObject>> &tokens)
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

int AppMgrServiceInner::GetApplicationInfoByProcessID(const int pid, AppExecFwk::ApplicationInfo &application,
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

int32_t AppMgrServiceInner::NotifyAppMgrRecordExitReason(int32_t pid, int32_t reason, const std::string &exitMsg)
{
    TAG_LOGD(AAFwkTag::APPMGR, "NotifyAppMgrRecordExitReason pid:%{public}d, reason:%{public}d, exitMsg:%{public}s.",
        pid, reason, exitMsg.c_str());
    auto callerUid = IPCSkeleton::GetCallingUid();
    if (callerUid != FOUNDATION_UID) {
        TAG_LOGE(AAFwkTag::APPMGR, "not foundation call");
        return ERR_PERMISSION_DENIED;
    }
    auto appRecord = GetAppRunningRecordByPid(pid);
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "no appRecord for pid:%{public}d", pid);
        return ERR_NAME_NOT_FOUND;
    }
    appRecord->SetExitReason(reason);
    appRecord->SetExitMsg(exitMsg);
    appRecord->SetReasonExist(true);
    return ERR_OK;
}

int AppMgrServiceInner::VerifyKillProcessPermission(const std::string &bundleName) const
{
    TAG_LOGI(AAFwkTag::APPMGR, "callerUid:%{public}d, callerPid:%{public}d",
        IPCSkeleton::GetCallingUid(), IPCSkeleton::GetCallingPid());
    int32_t ret = VerifyKillProcessPermissionCommon();
    if (ret != ERR_PERMISSION_DENIED) {
        return ret;
    }

    auto isCallingPerm = AAFwk::PermissionVerification::GetInstance()->VerifyCallingPermission(
        AAFwk::PermissionConstants::PERMISSION_CLEAN_BACKGROUND_PROCESSES);
    if (isCallingPerm) {
        auto callerPid = IPCSkeleton::GetCallingPid();
        auto appRecord = GetAppRunningRecordByPid(callerPid);
        if (!appRecord || appRecord->GetBundleName() != bundleName) {
            TAG_LOGE(AAFwkTag::APPMGR, "permission verification fail");
            return ERR_PERMISSION_DENIED;
        }
    } else {
        TAG_LOGE(AAFwkTag::APPMGR, "permission verification fail");
        return ERR_PERMISSION_DENIED;
    }

    return ERR_OK;
}

int AppMgrServiceInner::VerifyKillProcessPermission(const sptr<IRemoteObject> &token) const
{
    TAG_LOGI(AAFwkTag::APPMGR, "callerUid:%{public}d, callerPid:%{public}d",
        IPCSkeleton::GetCallingUid(), IPCSkeleton::GetCallingPid());
    int32_t ret = VerifyKillProcessPermissionCommon();
    if (ret != ERR_PERMISSION_DENIED) {
        return ret;
    }

    auto isCallingPerm = AAFwk::PermissionVerification::GetInstance()->VerifyCallingPermission(
        AAFwk::PermissionConstants::PERMISSION_CLEAN_BACKGROUND_PROCESSES);
    if (isCallingPerm) {
        auto callerUid = IPCSkeleton::GetCallingUid();
        auto appRecord = GetAppRunningRecordByAbilityToken(token);
        if (!appRecord || appRecord->GetUid() != callerUid) {
            TAG_LOGE(AAFwkTag::APPMGR, "permission verification fail");
            return ERR_PERMISSION_DENIED;
        }
    } else {
        TAG_LOGE(AAFwkTag::APPMGR, "permission verification fail");
        return ERR_PERMISSION_DENIED;
    }

    return ERR_OK;
}

int32_t AppMgrServiceInner::VerifyKillProcessPermissionCommon() const
{
    auto isCallingPerm = AAFwk::PermissionVerification::GetInstance()->VerifyCallingPermission(
        AAFwk::PermissionConstants::PERMISSION_KILL_APP_PROCESSES);
    if (isCallingPerm) {
        return ERR_OK;
    }

    // VerifyAPL and ohos.permission.CLEAN_BACKGROUND_PROCESSES will be removed on API18
    auto isSaCall = AAFwk::PermissionVerification::GetInstance()->IsSACall();
    auto isShellCall = AAFwk::PermissionVerification::GetInstance()->IsShellCall();
    if (isSaCall || isShellCall) {
        return ERR_OK;
    }

    if (VerifyAPL()) {
        return ERR_OK;
    }

    return ERR_PERMISSION_DENIED;
}

bool AppMgrServiceInner::VerifyAPL() const
{
    if (!appRunningManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRunningManager_ null");
        return false;
    }

    auto callerPid = IPCSkeleton::GetCallingPid();
    auto appRecord = appRunningManager_->GetAppRunningRecordByPid(callerPid);
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "get app running record fail, callingPId: %{public}d", callerPid);
        return false;
    }

    auto applicationInfo = appRecord->GetApplicationInfo();
    if (!applicationInfo) {
        TAG_LOGE(AAFwkTag::APPMGR, "get app info fail");
        return false;
    }

    auto apl = applicationInfo->appPrivilegeLevel;
    if (apl != SYSTEM_BASIC && apl != SYSTEM_CORE) {
        TAG_LOGE(AAFwkTag::APPMGR, "caller is not system_basic or system_core");
        return false;
    }
    TAG_LOGE(AAFwkTag::APPMGR, "%{public}s without ohos.permission.KILL_APP_PROCESSES",
        applicationInfo->name.c_str());
    return true;
}

int AppMgrServiceInner::VerifyAccountPermission(const std::string &permissionName, const int userId) const
{
    if (userId != currentUserId_) {
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

int AppMgrServiceInner::VerifyRequestPermission() const
{
    auto callerUid = IPCSkeleton::GetCallingUid();
    if (callerUid == ROOT_UID || callerUid == FOUNDATION_UID) {
        return ERR_OK;
    } else {
        TAG_LOGE(AAFwkTag::APPMGR, "permission verification fail, callerUid: %{public}d", callerUid);
        return ERR_PERMISSION_DENIED;
    }
}

int AppMgrServiceInner::PreStartNWebSpawnProcess(const pid_t hostPid)
{
    TAG_LOGI(AAFwkTag::APPMGR, "call");
    if (hostPid <= 0) {
        TAG_LOGE(AAFwkTag::APPMGR, "invalid param, hostPid:%{public}d", hostPid);
        return ERR_INVALID_VALUE;
    }

    auto nwebSpawnClient = remoteClientManager_->GetNWebSpawnClient();
    if (!nwebSpawnClient) {
        TAG_LOGE(AAFwkTag::APPMGR, "nwebSpawnClient null");
        return ERR_INVALID_VALUE;
    }
    if (UserRecordManager::GetInstance().IsLogoutUser(GetUserIdByUid(IPCSkeleton::GetCallingUid()))) {
        TAG_LOGE(AAFwkTag::APPMGR, "disable start process in logout user");
        return ERR_INVALID_OPERATION;
    }

    auto appRecord = appRunningManager_->GetAppRunningRecordByPid(hostPid);
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "no appRecord, pid:%{public}d", hostPid);
        return ERR_INVALID_VALUE;
    }

    ErrCode errCode = nwebSpawnClient->PreStartNWebSpawnProcess();
    if (FAILED(errCode)) {
        TAG_LOGE(AAFwkTag::APPMGR, "spawn new render process fail, errCode %{public}08x", errCode);
        return ERR_INVALID_VALUE;
    }

    return 0;
}

int AppMgrServiceInner::StartRenderProcess(const pid_t hostPid, const std::string &renderParam,
    FdGuard &&ipcFd, FdGuard &&sharedFd, FdGuard &&crashFd, pid_t &renderPid, bool isGPU)
{
    TAG_LOGI(AAFwkTag::APPMGR, "hostPid:%{public}d", hostPid);
    if (hostPid <= 0 || renderParam.empty() || ipcFd.Get() <= 0 || sharedFd.Get() <= 0 || crashFd.Get() <= 0) {
        TAG_LOGE(AAFwkTag::APPMGR, "invalid param: hostPid:%{public}d renderParam:%{private}s "
                    "ipcFd:%{public}d  crashFd:%{public}d sharedFd:%{public}d",
            hostPid, renderParam.c_str(), ipcFd.Get(), crashFd.Get(), sharedFd.Get());
        return ERR_INVALID_VALUE;
    }
    CHECK_POINTER_AND_RETURN_VALUE(appRunningManager_, ERR_INVALID_VALUE);
    if (UserRecordManager::GetInstance().IsLogoutUser(GetUserIdByUid(IPCSkeleton::GetCallingUid()))) {
        TAG_LOGE(AAFwkTag::APPMGR, "disable start process in logout user");
        return ERR_INVALID_OPERATION;
    }
    auto appRecord = GetAppRunningRecordByPid(hostPid);
    CHECK_POINTER_AND_RETURN_VALUE(appRecord, ERR_INVALID_VALUE);
    auto renderRecordMap = appRecord->GetRenderRecordMap();
    if (!isGPU && !renderRecordMap.empty() && !AAFwk::AppUtils::GetInstance().IsUseMultiRenderProcess()) {
        for (auto iter : renderRecordMap) {
            if (iter.second != nullptr) {
                renderPid = iter.second->GetPid();
                if (ProcessUtil::ProcessExist(renderPid)) {
                    TAG_LOGW(AAFwkTag::APPMGR,
                        "render process repeat, renderPid:%{public}d", renderPid);
                    return ERR_ALREADY_EXIST_RENDER;
                }
                auto scheduler = iter.second->GetScheduler();
                if (scheduler) {
                    TAG_LOGW(AAFwkTag::APPMGR, "null render, renderPid:%{public}d", renderPid);
                    OnRenderRemoteDied(scheduler->AsObject());
                }
            }
        }
    }
    appRecord->SetHasGPU(isGPU);
    int32_t childNumLimit = appRecord->HasGPU() ? PHONE_MAX_RENDER_PROCESS_NUM + 1 : PHONE_MAX_RENDER_PROCESS_NUM;
    // The phone device allows a maximum of 40 render processes to be created.
    if (AAFwk::AppUtils::GetInstance().IsLimitMaximumOfRenderProcess() &&
        renderRecordMap.size() >= static_cast<uint32_t>(childNumLimit)) {
        TAG_LOGE(AAFwkTag::APPMGR, "maximum render process limitation, hostPid:%{public}d", hostPid);
        return ERR_REACHING_MAXIMUM_RENDER_PROCESS_LIMITATION;
    }

    auto renderRecord = RenderRecord::CreateRenderRecord(hostPid, renderParam,
        std::move(ipcFd), std::move(sharedFd), std::move(crashFd), appRecord);
    if (!renderRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "create renderRecord fail, hostPid:%{public}d", hostPid);
        return ERR_INVALID_VALUE;
    }

    return StartRenderProcessImpl(renderRecord, appRecord, renderPid, isGPU);
}

void AppMgrServiceInner::AttachRenderProcess(const pid_t pid, const sptr<IRenderScheduler> &scheduler)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGI(AAFwkTag::APPMGR, "attachRenderProcess start");
    if (pid <= 0) {
        TAG_LOGE(AAFwkTag::APPMGR, "invalid render process pid:%{public}d", pid);
        return;
    }
    if (!scheduler) {
        TAG_LOGE(AAFwkTag::APPMGR, "render scheduler null");
        return;
    }

    if (!appRunningManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRunningManager_ null");
        return;
    }

    auto appRecord = appRunningManager_->GetAppRunningRecordByRenderPid(pid);
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "no appRecord, pid:%{public}d", pid);
        return;
    }

    auto renderRecord = appRecord->GetRenderRecordByPid(pid);
    if (!renderRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "no renderRecord, pid:%{public}d", pid);
        return;
    }

    sptr<AppDeathRecipient> appDeathRecipient = new AppDeathRecipient();
    appDeathRecipient->SetTaskHandler(taskHandler_);
    appDeathRecipient->SetAppMgrServiceInner(shared_from_this());
    appDeathRecipient->SetIsRenderProcess(true);
    renderRecord->SetScheduler(scheduler);
    renderRecord->SetDeathRecipient(appDeathRecipient);
    renderRecord->RegisterDeathRecipient();

    TAG_LOGI(AAFwkTag::APPMGR, "attachRenderProcess_%{public}d, notify fd", pid);
    // notify fd to render process
    if (appRecord->GetBrowserHost() != nullptr && renderRecord->GetProcessType() == ProcessType::GPU) {
        TAG_LOGD(AAFwkTag::APPMGR, "GPU has host remote object");
        scheduler->NotifyBrowserFd(renderRecord->GetIpcFd(),
            renderRecord->GetSharedFd(), renderRecord->GetCrashFd(), appRecord->GetBrowserHost());
    } else {
        scheduler->NotifyBrowserFd(renderRecord->GetIpcFd(),
            renderRecord->GetSharedFd(), renderRecord->GetCrashFd(), nullptr);
    }
}

void AppMgrServiceInner::SaveBrowserChannel(const pid_t hostPid, sptr<IRemoteObject> browser)
{
    std::lock_guard<ffrt::mutex> lock(browserHostLock_);
    TAG_LOGD(AAFwkTag::APPMGR, "save browser channel.");
    auto appRecord = GetAppRunningRecordByPid(hostPid);
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "save browser host no appRecord, pid:%{public}d",
            hostPid);
        return;
    }
    appRecord->SetBrowserHost(browser);
}

bool AppMgrServiceInner::GenerateRenderUid(int32_t &renderUid)
{
    std::lock_guard<ffrt::mutex> lock(renderUidSetLock_);
    int32_t uid = lastRenderUid_ + 1;
    bool needSecondScan = true;
    if (uid > Constants::END_UID_FOR_RENDER_PROCESS) {
        uid = Constants::START_UID_FOR_RENDER_PROCESS;
        needSecondScan = false;
    }

    if (renderUidSet_.empty()) {
        renderUid = uid;
        renderUidSet_.insert(renderUid);
        lastRenderUid_ = renderUid;
        return true;
    }

    for (int32_t i = uid; i <= Constants::END_UID_FOR_RENDER_PROCESS; i++) {
        if (renderUidSet_.find(i) == renderUidSet_.end()) {
            renderUid = i;
            renderUidSet_.insert(renderUid);
            lastRenderUid_ = renderUid;
            return true;
        }
    }

    if (needSecondScan) {
        for (int32_t i = Constants::START_UID_FOR_RENDER_PROCESS; i <= lastRenderUid_; i++) {
            if (renderUidSet_.find(i) == renderUidSet_.end()) {
                renderUid = i;
                renderUidSet_.insert(renderUid);
                lastRenderUid_ = renderUid;
                return true;
            }
        }
    }

    return false;
}

int AppMgrServiceInner::StartRenderProcessImpl(const std::shared_ptr<RenderRecord> &renderRecord,
    const std::shared_ptr<AppRunningRecord> appRecord, pid_t &renderPid, bool isGPU)
{
    if (!renderRecord || !appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "renderRecord or appRecord null");
        return ERR_INVALID_VALUE;
    }

    auto nwebSpawnClient = remoteClientManager_->GetNWebSpawnClient();
    if (!nwebSpawnClient) {
        TAG_LOGE(AAFwkTag::APPMGR, "nwebSpawnClient null");
        AppMgrEventUtil::SendRenderProcessStartFailedEvent(renderRecord,
            ProcessStartFailedReason::GET_SPAWN_CLIENT_FAILED, ERR_INVALID_VALUE);
        return ERR_INVALID_VALUE;
    }
    int32_t renderUid = Constants::INVALID_UID;
    if (!GenerateRenderUid(renderUid)) {
        TAG_LOGE(AAFwkTag::APPMGR, "generate renderUid fail");
        AppMgrEventUtil::SendRenderProcessStartFailedEvent(renderRecord,
            ProcessStartFailedReason::GENERATE_RENDER_UID_FAILED, ERR_INVALID_OPERATION);
        return ERR_INVALID_OPERATION;
    }
    AppSpawnStartMsg startMsg = appRecord->GetStartMsg();
    SetRenderStartMsg(startMsg, renderRecord, renderUid, isGPU);
    pid_t pid = 0;
    ErrCode errCode = nwebSpawnClient->StartProcess(startMsg, pid);
    if (FAILED(errCode)) {
        TAG_LOGE(AAFwkTag::APPMGR, "spawn new render process fail, errCode %{public}08x", errCode);
        std::lock_guard<ffrt::mutex> lock(renderUidSetLock_);
        renderUidSet_.erase(renderUid);
        AppMgrEventUtil::SendRenderProcessStartFailedEvent(renderRecord,
            ProcessStartFailedReason::APPSPAWN_FAILED, static_cast<int32_t>(errCode));
        return ERR_INVALID_VALUE;
    }
    renderPid = pid;
    renderRecord->SetPid(pid);
    renderRecord->SetUid(renderUid);
    renderRecord->SetProcessName(startMsg.procName);
    if (isGPU) {
        renderRecord->SetProcessType(ProcessType::GPU);
        appRecord->SetGPUPid(pid);
    }
    appRecord->AddRenderRecord(renderRecord);
    bool isPreload = appRecord->IsNWebPreload();
    TAG_LOGI(AAFwkTag::APPMGR,
        "startRenderProcess success, hostPid:%{public}d, hostUid:%{public}d, pid:%{public}d, uid:%{public}d"
        "isPreload:%{public}d", renderRecord->GetHostPid(), renderRecord->GetHostUid(), pid, renderUid, isPreload);
    DelayedSingleton<AppStateObserverManager>::GetInstance()->OnRenderProcessCreated(renderRecord, isPreload);
    return 0;
}

void AppMgrServiceInner::SetRenderStartMsg(AppSpawnStartMsg &startMsg, std::shared_ptr<RenderRecord> renderRecord,
    const int32_t renderUid, const bool isGPU)
{
    startMsg.renderParam = renderRecord->GetRenderParam();
    startMsg.uid = renderUid;
    startMsg.gid = renderUid;
    if (isGPU) {
        startMsg.procName += GPU_PROCESS_NAME;
        startMsg.processType = GPU_PROCESS_TYPE;
        if (std::find(startMsg.gids.begin(), startMsg.gids.end(), SHADER_CACHE_GROUPID) == startMsg.gids.end()) {
            startMsg.gids.push_back(SHADER_CACHE_GROUPID);
        }
    } else {
        startMsg.procName += RENDER_PROCESS_NAME;
        startMsg.processType = RENDER_PROCESS_TYPE;
    }
    startMsg.code = 0; // 0: DEFAULT
}

int AppMgrServiceInner::GetRenderProcessTerminationStatus(pid_t renderPid, int &status)
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
    RemoveRenderRecordNoAttach(hostRecord, renderPid);
    return 0;
}

void AppMgrServiceInner::RemoveRenderRecordNoAttach(const std::shared_ptr<AppRunningRecord> &hostRecord,
    int32_t renderPid)
{
    if (!hostRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "hostRecord null");
        return;
    }
    auto renderRecord = hostRecord->GetRenderRecordByPid(renderPid);
    if (!renderRecord) {
        TAG_LOGD(AAFwkTag::APPMGR, "renderRecord null");
        return;
    }
    if (renderRecord->GetScheduler() == nullptr) {
        hostRecord->RemoveRenderRecord(renderRecord);
        {
            std::lock_guard<ffrt::mutex> lock(renderUidSetLock_);
            renderUidSet_.erase(renderRecord->GetUid());
        }
        DelayedSingleton<AppStateObserverManager>::GetInstance()->OnRenderProcessDied(renderRecord);
    }
}

void AppMgrServiceInner::OnRenderRemoteDied(const wptr<IRemoteObject> &remote)
{
    TAG_LOGE(AAFwkTag::APPMGR, "on render remote died");
    if (appRunningManager_) {
        auto renderRecord = appRunningManager_->OnRemoteRenderDied(remote);
        if (renderRecord) {
            {
                std::lock_guard<ffrt::mutex> lock(renderUidSetLock_);
                renderUidSet_.erase(renderRecord->GetUid());
            }
            DelayedSingleton<AppStateObserverManager>::GetInstance()->OnRenderProcessDied(renderRecord);
        }
    }
}

void AppMgrServiceInner::AddWatchParameter()
{
    TAG_LOGI(AAFwkTag::APPMGR, "call");
    auto context = new (std::nothrow) std::weak_ptr<AppMgrServiceInner>(shared_from_this());
    int ret = WatchParameter(AAFwk::GlobalConfigurationKey::INPUT_POINTER_DEVICE, PointerDeviceEventCallback,
        context);
    if (ret != 0) {
        TAG_LOGE(AAFwkTag::APPMGR, "watch parameter %{public}s fail with %{public}d",
            AAFwk::GlobalConfigurationKey::INPUT_POINTER_DEVICE, ret);
    }
}

void AppMgrServiceInner::InitFocusListener()
{
    TAG_LOGI(AAFwkTag::APPMGR, "begin initFocusListener");
#ifdef SUPPORT_SCREEN
    if (focusListener_) {
        return;
    }

    focusListener_ = new WindowFocusChangedListener(shared_from_this(), taskHandler_);
#endif // SUPPORT_SCREEN
    auto registerTask = [innerServiceWeak = weak_from_this()]() {
        auto innerService = innerServiceWeak.lock();
        if (innerService) {
            TAG_LOGI(AAFwkTag::APPMGR, "registerFocusListenerTask");
            innerService->RegisterFocusListener();
        }
    };
    if (taskHandler_) {
        taskHandler_->SubmitTaskJust(registerTask, "RegisterFocusListenerTask", REGISTER_FOCUS_DELAY);
        TAG_LOGI(AAFwkTag::APPMGR, "submit registerFocusListenerTask");
    }
}

void AppMgrServiceInner::RegisterFocusListener()
{
    TAG_LOGI(AAFwkTag::APPMGR, "registerFocusListener begin");
#ifdef SUPPORT_SCREEN
    if (!focusListener_) {
        TAG_LOGE(AAFwkTag::APPMGR, "no focusListener_");
        return;
    }
    WindowManager::GetInstance().RegisterFocusChangedListener(focusListener_);
#endif // SUPPORT_SCREEN
    TAG_LOGI(AAFwkTag::APPMGR, "registerFocusListener end");
}

void AppMgrServiceInner::FreeFocusListener()
{
    TAG_LOGI(AAFwkTag::APPMGR, "freeFocusListener begin");
#ifdef SUPPORT_SCREEN
    if (!focusListener_) {
        TAG_LOGE(AAFwkTag::APPMGR, "no focusListener_");
        return;
    }
    WindowManager::GetInstance().UnregisterFocusChangedListener(focusListener_);
    focusListener_ = nullptr;
#endif // SUPPORT_SCREEN
    TAG_LOGI(AAFwkTag::APPMGR, "freeFocusListener end");
}
#ifdef SUPPORT_SCREEN
void AppMgrServiceInner::HandleFocused(const sptr<OHOS::Rosen::FocusChangeInfo> &focusChangeInfo)
{
    if (!focusChangeInfo) {
        TAG_LOGW(AAFwkTag::APPMGR, "invalid focusChangeInfo");
        return;
    }
    TAG_LOGI(AAFwkTag::APPMGR, "uid:%{public}d, pid:%{public}d", focusChangeInfo->uid_, focusChangeInfo->pid_);

    if (focusChangeInfo->pid_ <= 0) {
        TAG_LOGE(AAFwkTag::APPMGR, "invalid pid:%{public}d", focusChangeInfo->pid_);
        return;
    }

    auto appRecord = GetAppRunningRecordByPid(focusChangeInfo->pid_);
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "no appRecord, pid:%{public}d", focusChangeInfo->pid_);
        return;
    }

    if (!appRecord->UpdateAbilityFocusState(focusChangeInfo->abilityToken_, true)) {
        TAG_LOGD(
            AAFwkTag::APPMGR, "only change ability focus state, do not change process or application focus state.");
        return;
    }

    bool needNotifyApp = appRunningManager_->IsApplicationFirstFocused(*appRecord);
    if (appRecord->GetState() == ApplicationState::APP_STATE_FOREGROUND) {
        OnAppStateChanged(appRecord, ApplicationState::APP_STATE_FOREGROUND, needNotifyApp, true);
    }
    DelayedSingleton<AppStateObserverManager>::GetInstance()->OnProcessStateChanged(appRecord);
}

void AppMgrServiceInner::HandleUnfocused(const sptr<OHOS::Rosen::FocusChangeInfo> &focusChangeInfo)
{
    if (!focusChangeInfo) {
        TAG_LOGW(AAFwkTag::APPMGR, "invalid focusChangeInfo");
        return;
    }
    TAG_LOGD(
        AAFwkTag::APPMGR, "unfocused, uid:%{public}d, pid:%{public}d", focusChangeInfo->uid_, focusChangeInfo->pid_);

    if (focusChangeInfo->pid_ <= 0) {
        TAG_LOGE(AAFwkTag::APPMGR, "invalid pid:%{public}d", focusChangeInfo->pid_);
        return;
    }

    auto appRecord = GetAppRunningRecordByPid(focusChangeInfo->pid_);
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "no appRecord, pid:%{public}d", focusChangeInfo->pid_);
        return;
    }

    if (!appRecord->UpdateAbilityFocusState(focusChangeInfo->abilityToken_, false)) {
        TAG_LOGD(AAFwkTag::APPMGR,
            "only change ability from focus to unfocus, do not change process or application focus state.");
        return;
    }

    bool needNotifyApp = appRunningManager_->IsApplicationUnfocused(appRecord->GetBundleName());
    OnAppStateChanged(appRecord, appRecord->GetState(), needNotifyApp, true);
    DelayedSingleton<AppStateObserverManager>::GetInstance()->OnProcessStateChanged(appRecord);
}

void AppMgrServiceInner::InitWindowVisibilityChangedListener()
{
    TAG_LOGD(AAFwkTag::APPMGR, "Begin.");
    if (windowVisibilityChangedListener_ != nullptr) {
        TAG_LOGW(AAFwkTag::APPMGR, "visibility listener already initiate");
        return;
    }
    windowVisibilityChangedListener_ =
        new (std::nothrow) WindowVisibilityChangedListener(weak_from_this(), taskHandler_);
    if (windowVisibilityChangedListener_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "window visibility changed listener null");
        return;
    }
    WindowManager::GetInstance().RegisterVisibilityChangedListener(windowVisibilityChangedListener_);

    std::vector<sptr<WindowVisibilityInfo>> windowVisibilityInfos;
    windowVisibilityInfos.clear();
    WindowManager::GetInstance().GetVisibilityWindowInfo(windowVisibilityInfos);

    if (windowVisibilityInfos.empty()) {
        TAG_LOGW(AAFwkTag::APPMGR, "window visibility info is empty");
        return;
    }

    for (const auto &info : windowVisibilityInfos) {
        if (info == nullptr) {
            TAG_LOGE(AAFwkTag::APPMGR, "info null");
            continue;
        }
        auto appRecord = GetAppRunningRecordByPid(info->pid_);
        if (appRecord == nullptr) {
            TAG_LOGE(AAFwkTag::APPMGR, "appRecord null");
            continue;
        }
        appRecord->ChangeWindowVisibility(info);
    }

    TAG_LOGD(AAFwkTag::APPMGR, "End.");
}

void AppMgrServiceInner::FreeWindowVisibilityChangedListener()
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    if (windowVisibilityChangedListener_ == nullptr) {
        TAG_LOGW(AAFwkTag::APPMGR, "visibility listener already free");
        return;
    }
    WindowManager::GetInstance().UnregisterVisibilityChangedListener(windowVisibilityChangedListener_);
    windowVisibilityChangedListener_  = nullptr;
}

void AppMgrServiceInner::HandleWindowVisibilityChanged(
    const std::vector<sptr<OHOS::Rosen::WindowVisibilityInfo>> &windowVisibilityInfos)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    if (windowVisibilityInfos.empty()) {
        TAG_LOGW(AAFwkTag::APPMGR, "window visibility info empty");
        return;
    }
    if (appRunningManager_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "running manager null");
        return;
    }
    appRunningManager_->OnWindowVisibilityChanged(windowVisibilityInfos);
}

void AppMgrServiceInner::InitWindowPidVisibilityChangedListener()
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    if (windowPidVisibilityChangedListener_ != nullptr) {
        TAG_LOGW(AAFwkTag::APPMGR, "Pid visibility listener has been initiated.");
        return;
    }
    windowPidVisibilityChangedListener_ =
        sptr<WindowPidVisibilityChangedListener>::MakeSptr(weak_from_this(), taskHandler_);
    auto registerTask = [innerService = weak_from_this()] () {
        auto inner = innerService.lock();
        if (inner == nullptr) {
            TAG_LOGE(AAFwkTag::APPMGR, "Service inner is nullptr.");
            return;
        }
        if (inner->windowPidVisibilityChangedListener_ == nullptr) {
            TAG_LOGE(AAFwkTag::APPMGR, "Window pid visibility changed listener is nullptr.");
            return;
        }
        WMError ret = WindowManager::GetInstance().RegisterWindowPidVisibilityChangedListener(
            inner->windowPidVisibilityChangedListener_);
        if (ret != WMError::WM_OK) {
            TAG_LOGE(AAFwkTag::APPMGR, "RegisterWindowPidVisibilityChangedListener failed.");
            return;
        }
    };

    if (taskHandler_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "Task handler is nullptr.");
        return;
    }
    taskHandler_->SubmitTaskJust(registerTask, "RegisterPidVisibilityListener.", REGISTER_PID_VISIBILITY_DELAY);
}

void AppMgrServiceInner::FreeWindowPidVisibilityChangedListener()
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    if (windowPidVisibilityChangedListener_ == nullptr) {
        TAG_LOGW(AAFwkTag::APPMGR, "pid visibility changed listener has been freed.");
        return;
    }
    WindowManager::GetInstance().UnregisterWindowPidVisibilityChangedListener(windowPidVisibilityChangedListener_);
    windowPidVisibilityChangedListener_ = nullptr;
}

void AppMgrServiceInner::HandleWindowPidVisibilityChanged(
    const sptr<WindowPidVisibilityInfo>& windowPidVisibilityInfo)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    if (!windowPidVisibilityInfo) {
        TAG_LOGW(AAFwkTag::APPMGR, "Window pid visibility info is empty.");
        return;
    }
    auto appRecord = GetAppRunningRecordByPid(windowPidVisibilityInfo->pid_);
    if (appRecord == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "App running record is nullptr.");
        return;
    }

    if (windowPidVisibilityInfo->visibilityState_ == OHOS::Rosen::WindowPidVisibilityState::VISIBILITY_STATE) {
        DelayedSingleton<AppStateObserverManager>::GetInstance()->OnWindowShow(appRecord);
    }
    if (windowPidVisibilityInfo->visibilityState_ == OHOS::Rosen::WindowPidVisibilityState::INVISIBILITY_STATE) {
        DelayedSingleton<AppStateObserverManager>::GetInstance()->OnWindowHidden(appRecord);
    }
}
#endif // SUPPORT_SCREEN
void AppMgrServiceInner::PointerDeviceEventCallback(const char *key, const char *value, void *context)
{
    TAG_LOGI(AAFwkTag::APPMGR, "call");
    auto weak = static_cast<std::weak_ptr<AppMgrServiceInner>*>(context);
    if (weak == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "context null");
        return;
    }

    auto appMgrServiceInner = weak->lock();
    if (appMgrServiceInner == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "manager service inner null");
        return;
    }

    if ((strcmp(key, AAFwk::GlobalConfigurationKey::INPUT_POINTER_DEVICE) != 0) ||
        ((strcmp(value, "true") != 0) && (strcmp(value, "false") != 0))) {
        TAG_LOGE(AAFwkTag::APPMGR, "key %{public}s or value %{public}s mismatch", key, value);
        return;
    }

    Configuration changeConfig;
    if (!changeConfig.AddItem(AAFwk::GlobalConfigurationKey::INPUT_POINTER_DEVICE, value)) {
        TAG_LOGE(AAFwkTag::APPMGR, "add %{public}s item fail", key);
        return;
    }

    TAG_LOGD(AAFwkTag::APPMGR, "update config %{public}s to %{public}s", key, value);
    auto result = IN_PROCESS_CALL(appMgrServiceInner->UpdateConfiguration(changeConfig));
    if (result != 0) {
        TAG_LOGE(AAFwkTag::APPMGR, "update config fail with %{public}d, key: %{public}s, value: %{public}s", result,
            key, value);
        return;
    }
}

bool AppMgrServiceInner::GetAppRunningStateByBundleName(const std::string &bundleName)
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

int32_t AppMgrServiceInner::NotifyLoadRepairPatch(const std::string &bundleName,
    const sptr<IQuickFixCallback> &callback)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    if (!appRunningManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "running manager null");
        return ERR_INVALID_OPERATION;
    }

    if (IPCSkeleton::GetCallingUid() != QUICKFIX_UID) {
        TAG_LOGE(AAFwkTag::APPMGR, "not quick_fix");
        return ERR_PERMISSION_DENIED;
    }

    return appRunningManager_->NotifyLoadRepairPatch(bundleName, callback);
}

int32_t AppMgrServiceInner::NotifyHotReloadPage(const std::string &bundleName, const sptr<IQuickFixCallback> &callback)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    if (!appRunningManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "running manager null");
        return ERR_INVALID_OPERATION;
    }

    if (IPCSkeleton::GetCallingUid() != QUICKFIX_UID) {
        TAG_LOGE(AAFwkTag::APPMGR, "not quick_fix");
        return ERR_PERMISSION_DENIED;
    }

    return appRunningManager_->NotifyHotReloadPage(bundleName, callback);
}

#ifdef BGTASKMGR_CONTINUOUS_TASK_ENABLE
int32_t AppMgrServiceInner::SetContinuousTaskProcess(int32_t pid, bool isContinuousTask)
{
    if (!appRunningManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "running manager null");
        return ERR_INVALID_OPERATION;
    }

    auto appRecord = appRunningManager_->GetAppRunningRecordByPid(pid);
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "get running record fail, pid: %{public}d", pid);
        return ERR_INVALID_VALUE;
    }
    appRecord->SetContinuousTaskAppState(isContinuousTask);
    DelayedSingleton<AppStateObserverManager>::GetInstance()->OnProcessStateChanged(appRecord);

    return ERR_OK;
}
#endif

int32_t AppMgrServiceInner::NotifyUnLoadRepairPatch(const std::string &bundleName,
    const sptr<IQuickFixCallback> &callback)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    if (!appRunningManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "running manager null");
        return ERR_INVALID_OPERATION;
    }

    if (IPCSkeleton::GetCallingUid() != QUICKFIX_UID) {
        TAG_LOGE(AAFwkTag::APPMGR, "not quick_fix");
        return ERR_PERMISSION_DENIED;
    }

    return appRunningManager_->NotifyUnLoadRepairPatch(bundleName, callback);
}

void AppMgrServiceInner::AppRecoveryNotifyApp(int32_t pid, const std::string& bundleName,
    FaultDataType faultType, const std::string& markers)
{
    if (faultType != FaultDataType::APP_FREEZE) {
        TAG_LOGI(AAFwkTag::APPMGR,
            "kill appRecovery NotifyApp bundleName: %{public}s, faultType: "
            "%{public}d, pid: %{public}d", bundleName.c_str(), faultType, pid);
        KillProcessByPid(pid, "AppRecoveryNotifyApp");
        return;
    }

    std::string timeOutName = "waitSaveTask" + std::to_string(pid) + bundleName;
    if (markers == "appRecovery") {
        TAG_LOGI(AAFwkTag::APPMGR, "waitSaveTask finish, but not kill process "
            "immediately, wait for dump stack util 2s timeout");
        return;
    }

    if (markers != "recoveryTimeout") {
        return;
    }
    auto waitSaveTask = [pid, bundleName, innerServiceWeak = weak_from_this()]() {
        auto innerService = innerServiceWeak.lock();
        CHECK_POINTER_AND_RETURN_LOG(innerService, "get appMgrServiceInner fail");
        auto appRecord = innerService->GetAppRunningRecordByPid(pid);
        if (appRecord == nullptr) {
            TAG_LOGE(AAFwkTag::APPMGR, "no appRecord");
            return;
        }
        std::string name = appRecord->GetBundleName();
        if (bundleName == name) {
            TAG_LOGI(AAFwkTag::APPMGR,
                "waitSaveTask timeout %{public}s,pid: %{public}d will exit",
                bundleName.c_str(), pid);
            innerService->KillProcessByPid(pid, "AppRecoveryNotifyApp");
        }
    };
    constexpr int32_t timeOut = 2000;
    CHECK_POINTER_AND_RETURN_LOG(taskHandler_, "no taskHandler_");
    taskHandler_->SubmitTaskJust(waitSaveTask, timeOutName, timeOut);
}

void AppMgrServiceInner::ParseInfoToAppfreeze(const FaultData &faultData, int32_t pid, int32_t uid,
    const std::string &bundleName, const std::string &processName, const bool isOccurException)
{
    if (faultData.faultType == FaultDataType::APP_FREEZE) {
        AppfreezeManager::AppInfo info = {
            .pid = pid,
            .uid = uid,
            .bundleName = bundleName,
            .processName = processName,
            .isOccurException = isOccurException,
        };
        AppExecFwk::AppfreezeManager::GetInstance()->AppfreezeHandleWithStack(faultData, info);
    }

    TAG_LOGW(AAFwkTag::APPMGR,
        "name: %{public}s, faultType: %{public}d, uid: %{public}d, pid: %{public}d, bundleName: %{public}s,"
        " processName: %{public}s, faultData.forceExit:%{public}d, faultData.waitSaveState:%{public}d,"
        " isOccurException:%{public}d, stuckTimeout:%{public}u",
        faultData.errorObject.name.c_str(), faultData.faultType, uid, pid, bundleName.c_str(),
        processName.c_str(), faultData.forceExit, faultData.waitSaveState, isOccurException, faultData.stuckTimeout);
}


int AppMgrServiceInner::GetExceptionTimerId(const FaultData &faultData, const std::string &bundleName,
    const std::shared_ptr<AppRunningRecord> &appRecord, const int32_t pid, const int32_t callerUid)
{
    auto exceptionCallback = [faultData, bundleName, appRecord, pid, callerUid,
        innerService = shared_from_this()](void *) {
        auto threadTask = [faultData, bundleName, appRecord, pid, callerUid, innerService]() {
            if (innerService->CheckAppFault(appRecord, faultData)) {
                TAG_LOGI(AAFwkTag::APPMGR, "Ffrt Exception current dfx task is working.");
                return;
            }
            bool isOccurException = true;
            innerService->ParseInfoToAppfreeze(faultData, pid, callerUid, bundleName, appRecord->GetProcessName(),
                isOccurException);
            if (faultData.errorObject.name != AppFreezeType::THREAD_BLOCK_3S ||
                faultData.errorObject.name != AppFreezeType::LIFECYCLE_HALF_TIMEOUT) {
                TAG_LOGI(AAFwkTag::APPMGR, "Ffrt Exception faultData: %{public}s,pid: %{public}d "
                    "will exit because"" %{public}s", bundleName.c_str(), pid,
                    innerService->FaultTypeToString(faultData.faultType).c_str());
                innerService->KillProcessByPid(pid, faultData.errorObject.name);
                return;
            }
        };
        std::thread dfxThread(threadTask);
        if (dfxThread.joinable()) {
            dfxThread.join();
        }
    };
    constexpr uint32_t timeout = 30; // 30s
    int exceptionId = -1;
#ifdef APP_MGR_SERVICE_HICOLLIE_ENABLE
    exceptionId = HiviewDFX::XCollie::GetInstance().SetTimer("DfxFault::Exception", timeout,
        exceptionCallback, nullptr, HiviewDFX::XCOLLIE_FLAG_NOOP);
#endif
    return exceptionId;
}

int32_t AppMgrServiceInner::SubmitDfxFaultTask(const FaultData &faultData, const std::string &bundleName,
    const std::shared_ptr<AppRunningRecord> &appRecord, const int32_t pid)
{
    int32_t callerUid = IPCSkeleton::GetCallingUid();
    std::string processName = appRecord->GetProcessName();
    int exceptionId = GetExceptionTimerId(faultData, bundleName, appRecord, pid, callerUid);
    auto notifyAppTask = [appRecord, pid, callerUid, bundleName, processName, faultData, exceptionId,
        innerServiceWeak = weak_from_this()]() {
#ifdef APP_MGR_SERVICE_HICOLLIE_ENABLE
        HiviewDFX::XCollie::GetInstance().CancelTimer(exceptionId);
#endif
        auto innerService = innerServiceWeak.lock();
        CHECK_POINTER_AND_RETURN_LOG(innerService, "get appMgrServiceInner fail");
        innerService->ParseInfoToAppfreeze(faultData, pid, callerUid, bundleName, processName);
    };

    if (!dfxTaskHandler_) {
        TAG_LOGW(AAFwkTag::APPMGR, "get dfx handler fail");
        return ERR_INVALID_VALUE;
    }
    int64_t startTime = AbilityRuntime::TimeUtil::CurrentTimeMillis();
    dfxTaskHandler_->SubmitTask(notifyAppTask, "NotifyAppFaultTask");
    TAG_LOGW(AAFwkTag::APPDFR, "submit NotifyAppFaultTask, eventName:%{public}s, bundleName:%{public}s, "
        "endTime:%{public}s, interval:%{public}" PRId64 " ms", faultData.errorObject.name.c_str(),
        bundleName.c_str(), AbilityRuntime::TimeUtil::DefaultCurrentTimeStr().c_str(),
        AbilityRuntime::TimeUtil::CurrentTimeMillis() - startTime);

    constexpr int delayTime = 15 * 1000; // 15s
    auto task = [pid]() {
        AppExecFwk::AppfreezeManager::GetInstance()->DeleteStack(pid);
    };
    dfxTaskHandler_->SubmitTaskJust(task, "DeleteStack", delayTime);

    return ERR_OK;
}

int32_t AppMgrServiceInner::NotifyAppFault(const FaultData &faultData)
{
    TAG_LOGI(AAFwkTag::APPMGR, "call");
    int32_t pid = IPCSkeleton::GetCallingPid();
    auto appRecord = GetAppRunningRecordByPid(pid);
    if (appRecord == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "no appRecord, pid:%{public}d", pid);
        return ERR_INVALID_VALUE;
    }
    if (appRecord->GetState() == ApplicationState::APP_STATE_TERMINATED ||
        appRecord->GetState() == ApplicationState::APP_STATE_END) {
        TAG_LOGE(AAFwkTag::APPMGR, "appfreeze detect end");
        return ERR_OK;
    }
    std::string bundleName = appRecord->GetBundleName();
    std::string eventName = faultData.errorObject.name;

    TAG_LOGW(AAFwkTag::APPDFR, "called, eventName:%{public}s, pid:%{public}d, bundleName:%{public}s, "
        "currentTime:%{public}s", eventName.c_str(), pid, bundleName.c_str(),
        AbilityRuntime::TimeUtil::DefaultCurrentTimeStr().c_str());
    if (AppExecFwk::AppfreezeManager::GetInstance()->IsProcessDebug(pid, bundleName)) {
        TAG_LOGW(AAFwkTag::APPMGR, "don't report event and kill:%{public}s, pid:%{public}d, bundleName:%{public}s",
            eventName.c_str(), pid, bundleName.c_str());
        return ERR_OK;
    }

    if (faultData.faultType == FaultDataType::APP_FREEZE) {
        if (CheckAppFault(appRecord, faultData)) {
            return ERR_OK;
        }

        if (faultData.waitSaveState) {
            AppRecoveryNotifyApp(pid, bundleName, FaultDataType::APP_FREEZE, "recoveryTimeout");
        }
    }
    if (eventName == AppFreezeType::LIFECYCLE_TIMEOUT || eventName == AppFreezeType::APP_INPUT_BLOCK ||
        eventName == AppFreezeType::THREAD_BLOCK_6S || eventName == AppFreezeType::THREAD_BLOCK_3S) {
        if (AppExecFwk::AppfreezeManager::GetInstance()->IsNeedIgnoreFreezeEvent(pid, eventName)) {
            TAG_LOGE(AAFwkTag::APPDFR, "appFreeze happend, pid:%{public}d, eventName:%{public}s",
                pid, eventName.c_str());
            return ERR_OK;
        }
    }
    if (SubmitDfxFaultTask(faultData, bundleName, appRecord, pid) != ERR_OK) {
        return ERR_INVALID_VALUE;
    }

    if (appRecord->GetApplicationInfo()->asanEnabled) {
        TAG_LOGI(AAFwkTag::APPMGR, "asan enable, bundleName:%{public}s, pid:%{public}d", bundleName.c_str(), pid);
        return ERR_OK;
    }

#ifdef APP_NO_RESPONSE_DIALOG
    // A dialog box is displayed when the PC appfreeze
    bool isDialogExist = appRunningManager_ ?
        appRunningManager_->CheckAppRunningRecordIsExist(APP_NO_RESPONSE_BUNDLENAME, APP_NO_RESPONSE_ABILITY) : false;
    auto killFaultApp = std::bind(&AppMgrServiceInner::KillFaultApp, this, pid, bundleName, faultData, false);
    ModalSystemAppFreezeUIExtension::GetInstance().ProcessAppFreeze(appRecord->GetFocusFlag(), faultData,
        std::to_string(pid), bundleName, killFaultApp, isDialogExist);
#else
    KillFaultApp(pid, bundleName, faultData);
#endif

    return ERR_OK;
}

bool AppMgrServiceInner::CheckAppFault(const std::shared_ptr<AppRunningRecord> &appRecord, const FaultData &faultData)
{
    if (faultData.timeoutMarkers != "" && !dfxTaskHandler_->CancelTask(faultData.timeoutMarkers)) {
        return true;
    }

    if (appRecord->IsDebugging()) {
        return true;
    }
    return false;
}

int32_t AppMgrServiceInner::KillFaultApp(int32_t pid, const std::string &bundleName, const FaultData &faultData,
    bool isNeedExit)
{
    auto killAppTask = [pid, bundleName, faultData, isNeedExit, innerServiceWeak = weak_from_this()]() {
        auto innerService = innerServiceWeak.lock();
        CHECK_POINTER_AND_RETURN_LOG(innerService, "get appMgrServiceInner fail");
        if (isNeedExit || (faultData.forceExit && !faultData.waitSaveState)) {
            TAG_LOGW(AAFwkTag::APPMGR, "faultData: %{public}s,pid: %{public}d will exit because %{public}s",
                bundleName.c_str(), pid, innerService->FaultTypeToString(faultData.faultType).c_str());
            innerService->KillProcessByPid(pid, faultData.errorObject.name);
            return;
        }
    };
    constexpr int32_t waitTime = 3500;
    // wait 3.5s before kill application
    CHECK_POINTER_AND_RETURN_VALUE(taskHandler_, ERR_NO_INIT);
    taskHandler_->SubmitTaskJust(killAppTask, "killAppTask", waitTime);
    return ERR_OK;
}

void AppMgrServiceInner::TimeoutNotifyApp(int32_t pid, int32_t uid,
    const std::string& bundleName, const std::string& processName, const FaultData &faultData)
{
    bool isNeedExit = (faultData.errorObject.name == AppFreezeType::APP_INPUT_BLOCK) ||
        (faultData.errorObject.name == AppFreezeType::LIFECYCLE_TIMEOUT);
#ifdef APP_NO_RESPONSE_DIALOG
    bool isDialogExist = appRunningManager_ ?
        appRunningManager_->CheckAppRunningRecordIsExist(APP_NO_RESPONSE_BUNDLENAME, APP_NO_RESPONSE_ABILITY) :
        false;
    auto killFaultApp = std::bind(&AppMgrServiceInner::KillFaultApp, this, pid, bundleName, faultData, isNeedExit);
    ModalSystemAppFreezeUIExtension::GetInstance().ProcessAppFreeze(true, faultData, std::to_string(pid),
        bundleName, killFaultApp, isDialogExist);
#else
    KillFaultApp(pid, bundleName, faultData, isNeedExit);
#endif
    if (faultData.faultType == FaultDataType::APP_FREEZE) {
        AppfreezeManager::AppInfo info = {
            .pid = pid,
            .uid = uid,
            .bundleName = bundleName,
            .processName = processName,
        };
        AppExecFwk::AppfreezeManager::GetInstance()->AppfreezeHandleWithStack(faultData, info);
    }
}

int32_t AppMgrServiceInner::TransformedNotifyAppFault(const AppFaultDataBySA &faultData)
{
    int32_t pid = faultData.pid;
    auto record = GetAppRunningRecordByPid(pid);
    if (record == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "no such AppRunningRecord");
        return ERR_INVALID_VALUE;
    }

    FaultData transformedFaultData = ConvertDataTypes(faultData);
    int32_t uid = record->GetUid();
    std::string bundleName = record->GetBundleName();
    std::string processName = record->GetProcessName();
    if (AppExecFwk::AppfreezeManager::GetInstance()->IsProcessDebug(pid, bundleName)) {
        TAG_LOGW(AAFwkTag::APPMGR,
            "don't report event and kill:%{public}s, pid:%{public}d, bundleName:%{public}s.",
            faultData.errorObject.name.c_str(), pid, bundleName.c_str());
        return ERR_OK;
    }
    if (faultData.errorObject.name == "appRecovery") {
        AppRecoveryNotifyApp(pid, bundleName, faultData.faultType, "appRecovery");
        return ERR_OK;
    }

    if (transformedFaultData.timeoutMarkers.empty()) {
        transformedFaultData.timeoutMarkers = "notifyFault:" + transformedFaultData.errorObject.name +
            std::to_string(pid) + "-" + std::to_string(SystemTimeMillisecond());
    }
    const int64_t timeout = 1000;
    if (faultData.faultType == FaultDataType::APP_FREEZE) {
        if (!AppExecFwk::AppfreezeManager::GetInstance()->IsHandleAppfreeze(bundleName) || record->IsDebugging()) {
            return ERR_OK;
        }
        auto timeoutNotifyApp = [this, pid, uid, bundleName, processName, transformedFaultData]() {
            this->TimeoutNotifyApp(pid, uid, bundleName, processName, transformedFaultData);
        };
        dfxTaskHandler_->SubmitTask(timeoutNotifyApp, transformedFaultData.timeoutMarkers, timeout);
    }
    record->NotifyAppFault(transformedFaultData);
    TAG_LOGW(AAFwkTag::APPMGR, "FaultDataBySA is: name: %{public}s, faultType: %{public}s, uid: %{public}d,"
        "pid: %{public}d, bundleName: %{public}s, eventId: %{public}d", faultData.errorObject.name.c_str(),
        FaultTypeToString(faultData.faultType).c_str(), uid, pid, bundleName.c_str(), faultData.eventId);
    return ERR_OK;
}

int32_t AppMgrServiceInner::NotifyAppFaultBySA(const AppFaultDataBySA &faultData)
{
    if (remoteClientManager_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "remoteClientManager_ null");
        return ERR_NO_INIT;
    }
    std::string callerBundleName;
    if (auto bundleMgrHelper = remoteClientManager_->GetBundleManagerHelper(); bundleMgrHelper != nullptr) {
        int32_t callingUid = IPCSkeleton::GetCallingUid();
        IN_PROCESS_CALL(bundleMgrHelper->GetNameForUid(callingUid, callerBundleName));
    }
#ifdef ABILITY_FAULT_AND_EXIT_TEST
    if ((AAFwk::PermissionVerification::GetInstance()->IsSACall()) ||
        AAFwk::PermissionVerification::GetInstance()->IsShellCall()) {
#else
    if ((AAFwk::PermissionVerification::GetInstance()->IsSACall()) || callerBundleName == SCENE_BOARD_BUNDLE_NAME) {
#endif
        return TransformedNotifyAppFault(faultData);
    }
    TAG_LOGD(AAFwkTag::APPMGR, "this is not called by SA.");
    return AAFwk::CHECK_PERMISSION_FAILED;
}

bool AppMgrServiceInner::SetAppFreezeFilter(int32_t pid)
{
    int32_t callingPid = IPCSkeleton::GetCallingPid();
    auto callerRecord = GetAppRunningRecordByPid(pid);
    if (callerRecord == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "callerRecord null");
        return false;
    }
    std::string bundleName = callerRecord->GetBundleName();
    if (callingPid == pid && AppExecFwk::AppfreezeManager::GetInstance()->IsValidFreezeFilter(pid, bundleName)) {
        bool cancelResult = AppExecFwk::AppfreezeManager::GetInstance()->CancelAppFreezeDetect(pid, bundleName);
        auto resetAppfreezeTask = [pid, bundleName]() {
            AppExecFwk::AppfreezeManager::GetInstance()->ResetAppfreezeState(pid, bundleName);
        };
        constexpr int32_t waitTime = 120000; // wait 2min
        CHECK_POINTER_AND_RETURN_VALUE(dfxTaskHandler_, false);
        dfxTaskHandler_->SubmitTaskJust(resetAppfreezeTask, "resetAppfreezeTask", waitTime);
        return cancelResult;
    }
    TAG_LOGE(AAFwkTag::APPDFR, "SetAppFreezeFilter failed, pid %{public}d calling pid %{public}d",
        pid, callingPid);
    return false;
}

FaultData AppMgrServiceInner::ConvertDataTypes(const AppFaultDataBySA &faultData)
{
    FaultData newfaultData;
    newfaultData.faultType = faultData.faultType;
    newfaultData.errorObject.message =
        "\nFault time:" + AbilityRuntime::TimeUtil::FormatTime("%Y/%m/%d-%H:%M:%S") + "\n";
    newfaultData.errorObject.message += faultData.errorObject.message;
    newfaultData.errorObject.name = faultData.errorObject.name;
    newfaultData.errorObject.stack = faultData.errorObject.stack;
    newfaultData.timeoutMarkers = faultData.timeoutMarkers;
    newfaultData.waitSaveState = faultData.waitSaveState;
    newfaultData.notifyApp = faultData.notifyApp;
    newfaultData.forceExit = faultData.forceExit;
    newfaultData.token = faultData.token;
    newfaultData.state = faultData.state;
    newfaultData.eventId = faultData.eventId;
    return newfaultData;
}

std::string AppMgrServiceInner::FaultTypeToString(AppExecFwk::FaultDataType type)
{
    std::string typeStr = "UNKNOWN";
    switch (type) {
        case AppExecFwk::FaultDataType::CPP_CRASH:
            typeStr = "CPP_CRASH";
            break;
        case AppExecFwk::FaultDataType::JS_ERROR:
            typeStr = "JS_ERROR";
            break;
        case AppExecFwk::FaultDataType::APP_FREEZE:
            typeStr = "APP_FREEZE";
            break;
        case AppExecFwk::FaultDataType::PERFORMANCE_CONTROL:
            typeStr = "PERFORMANCE_CONTROL";
            break;
        case AppExecFwk::FaultDataType::RESOURCE_CONTROL:
            typeStr = "RESOURCE_CONTROL";
            break;
        default:
            break;
    }
    return typeStr;
}

bool AppMgrServiceInner::IsSharedBundleRunning(const std::string &bundleName, uint32_t versionCode)
{
    if (!CheckGetRunningInfoPermission()) {
        return false;
    }

    std::lock_guard lock(runningSharedBundleListMutex_);
    for (const auto &it : runningSharedBundleList_) {
        for (const auto &item : it.second) {
            if (item.bundleName == bundleName && item.versionCode == versionCode) {
                return true;
            }
        }
    }
    return false;
}

int32_t AppMgrServiceInner::IsApplicationRunning(const std::string &bundleName, bool &isRunning)
{
    TAG_LOGD(AAFwkTag::APPMGR, "Called, bundleName: %{public}s", bundleName.c_str());
    CHECK_CALLER_IS_SYSTEM_APP;
    if (!CheckGetRunningInfoPermission()) {
        TAG_LOGE(AAFwkTag::APPMGR, "permission verification fail");
        return ERR_PERMISSION_DENIED;
    }

    return appRunningManager_->CheckAppCloneRunningRecordIsExistByBundleName(bundleName, 0, isRunning);
}

int32_t AppMgrServiceInner::IsAppRunning(const std::string &bundleName, int32_t appCloneIndex,
    bool &isRunning)
{
    TAG_LOGD(AAFwkTag::APPMGR, "Called, bundleName: %{public}s", bundleName.c_str());
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
    BundleInfo bundleInfo;
    auto userId = GetCurrentAccountId();
    int32_t bundleMgrResult;
    if (appCloneIndex == 0) {
        bundleMgrResult = IN_PROCESS_CALL(bundleMgrHelper->GetBundleInfoV9(bundleName,
            static_cast<int32_t>(GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_APPLICATION) |
            static_cast<int32_t>(GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_EXTENSION_ABILITY) |
            static_cast<int32_t>(GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_HAP_MODULE) |
            static_cast<int32_t>(GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_REQUESTED_PERMISSION), bundleInfo, userId));
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

int32_t AppMgrServiceInner::IsAppRunningByBundleNameAndUserId(const std::string &bundleName, int32_t userId,
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
        userId = GetCurrentAccountId();
    }

    return appRunningManager_->IsAppRunningByBundleNameAndUserId(bundleName, userId, isRunning);
}

bool AppMgrServiceInner::CreateAbilityInfo(const AAFwk::Want &want, AbilityInfo &abilityInfo)
{
    auto&& bundleMgrHelper = remoteClientManager_->GetBundleManagerHelper();
    if (!bundleMgrHelper) {
        TAG_LOGE(AAFwkTag::APPMGR, "get bundle manager helper error");
        return false;
    }
    auto userId = GetCurrentAccountId();
    auto abilityInfoFlag = AbilityRuntime::StartupUtil::BuildAbilityInfoFlag();
    if (IN_PROCESS_CALL(bundleMgrHelper->QueryAbilityInfo(want, abilityInfoFlag, userId, abilityInfo))) {
        TAG_LOGI(AAFwkTag::APPMGR, "queryAbilityInfo ok");
        return true;
    }
    std::vector<AppExecFwk::ExtensionAbilityInfo> extensionInfos;
    int32_t appIndex = want.GetIntParam(AppspawnUtil::DLP_PARAMS_INDEX, 0);
    if (appIndex == 0) {
        if (!IN_PROCESS_CALL(bundleMgrHelper->QueryExtensionAbilityInfos(want, abilityInfoFlag,
            userId, extensionInfos))) {
            TAG_LOGE(AAFwkTag::APPMGR, "queryExtensionAbilityInfos fail");
            return false;
        }
    } else {
        if (!IN_PROCESS_CALL(bundleMgrHelper->GetSandboxExtAbilityInfos(want, appIndex,
            abilityInfoFlag, userId, extensionInfos))) {
            TAG_LOGE(AAFwkTag::APPMGR, "getSandboxExtAbilityInfos fail");
            return false;
        }
    }
    if (extensionInfos.size() <= 0) {
        TAG_LOGE(AAFwkTag::APPMGR, "get extension info fail");
        return ERR_INVALID_OPERATION;
    }
    AppExecFwk::ExtensionAbilityInfo extensionInfo = extensionInfos.front();
    AbilityRuntime::StartupUtil::InitAbilityInfoFromExtension(extensionInfo, abilityInfo);
    return true;
}

int32_t AppMgrServiceInner::StartNativeProcessForDebugger(const AAFwk::Want &want)
{
    CHECK_POINTER_AND_RETURN_VALUE(appRunningManager_, ERR_INVALID_OPERATION);
    TAG_LOGI(AAFwkTag::APPMGR, "bundleName:%{public}s, moduleName:%{public}s, abilityName:%{public}s",
        want.GetElement().GetBundleName().c_str(), want.GetElement().GetModuleName().c_str(),
        want.GetElement().GetAbilityName().c_str());
    AbilityInfo abilityInfo;
    if (!CreateAbilityInfo(want, abilityInfo)) {
        TAG_LOGE(AAFwkTag::APPMGR, "createAbilityInfo fail");
        return ERR_INVALID_OPERATION;
    }
    BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    auto appInfo = std::make_shared<ApplicationInfo>(abilityInfo.applicationInfo);
    if (!GetBundleAndHapInfo(abilityInfo, appInfo, bundleInfo, hapModuleInfo, 0)) {
        TAG_LOGE(AAFwkTag::APPMGR, "getBundleAndHapInfo fail");
        return ERR_INVALID_OPERATION;
    }

    std::string processName;
    auto abilityInfoPtr = std::make_shared<AbilityInfo>(abilityInfo);
    MakeProcessName(abilityInfoPtr, appInfo, hapModuleInfo, 0, "", processName, false);
    if (UserRecordManager::GetInstance().IsLogoutUser(GetUserIdByUid(appInfo->uid))) {
        TAG_LOGE(AAFwkTag::APPMGR, "disable start process in logout user");
        return ERR_INVALID_OPERATION;
    }
    auto&& appRecord =
        appRunningManager_->CheckAppRunningRecordIsExist(appInfo->name, processName, appInfo->uid, bundleInfo);
    AppSpawnStartMsg startMsg;
    bool isDevelopeMode = system::GetBoolParameter(DEVELOPER_MODE_STATE, false);
    if (appRecord) {
        startMsg = appRecord->GetStartMsg();
    } else if (!isDevelopeMode || CreatNewStartMsg(want, abilityInfo, appInfo, processName, startMsg) != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "invalid Operation");
        return ERR_INVALID_OPERATION;
    }

    bool isSandboxApp = want.GetBoolParam(ENTER_SANDBOX, false);
    auto&& pefCmd = want.GetStringParam(PERF_CMD);
    std::string debugCmd = "";
    if (pefCmd.empty()) {
        if (!appInfo->debug) {
            TAG_LOGE(AAFwkTag::APPMGR, "app don't debug mode");
            return ERR_INVALID_OPERATION;
        }
        debugCmd = want.GetStringParam(DEBUG_CMD);
    }
    return StartPerfProcessByStartMsg(startMsg, pefCmd, debugCmd, isSandboxApp);
}

int32_t AppMgrServiceInner::GetCurrentAccountId() const
{
    std::vector<int32_t> osActiveAccountIds;
    ErrCode ret = DelayedSingleton<AppExecFwk::OsAccountManagerWrapper>::GetInstance()->
        QueryActiveOsAccountIds(osActiveAccountIds);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "queryActiveOsAccountIds fail");
        return DEFAULT_USER_ID;
    }
    if (osActiveAccountIds.empty()) {
        TAG_LOGE(AAFwkTag::APPMGR, "queryActiveOsAccountIds empty");
        return DEFAULT_USER_ID;
    }
    TAG_LOGD(AAFwkTag::APPMGR, "osActiveAccountId: %{public}d", osActiveAccountIds.front());
    return osActiveAccountIds.front();
}

void AppMgrServiceInner::SetRunningSharedBundleList(const std::string &bundleName,
    const std::vector<BaseSharedBundleInfo> baseSharedBundleInfoList)
{
    std::lock_guard lock(runningSharedBundleListMutex_);
    runningSharedBundleList_.try_emplace(bundleName, baseSharedBundleInfoList);
}

void AppMgrServiceInner::RemoveRunningSharedBundleList(const std::string &bundleName)
{
    std::lock_guard lock(runningSharedBundleListMutex_);
    auto iterator = runningSharedBundleList_.find(bundleName);
    if (iterator == runningSharedBundleList_.end()) {
        return;
    }
    runningSharedBundleList_.erase(iterator);
}

void AppMgrServiceInner::SetCurrentUserId(const int32_t userId)
{
    if (IPCSkeleton::GetCallingUid() != FOUNDATION_UID) {
        return;
    }
    TAG_LOGD(AAFwkTag::APPMGR, "set current userId: %{public}d", userId);
    currentUserId_ = userId;
}

void AppMgrServiceInner::SetEnableStartProcessFlagByUserId(int32_t userId, bool enableStartProcess)
{
    UserRecordManager::GetInstance().SetEnableStartProcessFlagByUserId(userId, enableStartProcess);
}

int32_t AppMgrServiceInner::GetBundleNameByPid(const int32_t pid, std::string &bundleName, int32_t &uid)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    if (!AAFwk::PermissionVerification::GetInstance()->IsSACall()) {
        TAG_LOGE(AAFwkTag::APPMGR, "permission verification fail");
        return ERR_PERMISSION_DENIED;
    }
    auto callerRecord = GetAppRunningRecordByPid(pid);
    if (callerRecord == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "callerRecord null");
        return ERR_INVALID_OPERATION;
    }
    bundleName = callerRecord->GetBundleName();
    uid = callerRecord->GetUid();
    return ERR_OK;
}

void AppMgrServiceInner::KillRenderProcess(const std::shared_ptr<AppRunningRecord> &appRecord) {
    if (appRecord == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRecord null");
        return;
    }
    auto renderRecordMap = appRecord->GetRenderRecordMap();
    if (!renderRecordMap.empty()) {
        for (auto iter : renderRecordMap) {
            auto renderRecord = iter.second;
            if (renderRecord && renderRecord->GetPid() > 0) {
                auto pid = renderRecord->GetPid();
                auto uid = renderRecord->GetUid();
                TAG_LOGI(AAFwkTag::APPMGR, "pid:%{public}d, uid:%{public}d",
                    pid, uid);
                KillProcessByPid(pid, "KillRenderProcess");
                {
                    std::lock_guard lock(renderUidSetLock_);
                    renderUidSet_.erase(uid);
                }
                DelayedSingleton<AppStateObserverManager>::GetInstance()->OnRenderProcessDied(renderRecord);
            }
        }
    }
}

int32_t AppMgrServiceInner::GetProcessMemoryByPid(const int32_t pid, int32_t &memorySize)
{
    CHECK_CALLER_IS_SYSTEM_APP;
    uint64_t memSize = OHOS::MemInfo::GetPssByPid(pid);
    memorySize = static_cast<int32_t>(memSize);
    return ERR_OK;
}

int32_t AppMgrServiceInner::GetRunningProcessInformation(
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
    TAG_LOGI(AAFwkTag::APPMGR, "uid value: %{public}d", userId);
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

int32_t AppMgrServiceInner::ChangeAppGcState(pid_t pid, int32_t state)
{
    auto callerUid = IPCSkeleton::GetCallingUid();
    TAG_LOGD(AAFwkTag::APPMGR, "called, pid:%{public}d, state:%{public}d, uid:%{public}d.", pid, state, callerUid);
    if (callerUid != RESOURCE_MANAGER_UID) { // The current UID for resource management is 1096
        TAG_LOGE(AAFwkTag::APPMGR, "caller is not resource manager");
        return ERR_INVALID_VALUE;
    }
    auto appRecord = GetAppRunningRecordByPid(pid);
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "no appRecord");
        return ERR_INVALID_VALUE;
    }
    return appRecord->ChangeAppGcState(state);
}

int32_t AppMgrServiceInner::RegisterAppDebugListener(const sptr<IAppDebugListener> &listener)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    if (!AAFwk::PermissionVerification::GetInstance()->IsSACall()) {
        TAG_LOGE(AAFwkTag::APPMGR, "permission verification fail");
        return ERR_PERMISSION_DENIED;
    }

    if (appDebugManager_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "appDebugManager_ null");
        return ERR_NO_INIT;
    }
    return appDebugManager_->RegisterAppDebugListener(listener);
}

int32_t AppMgrServiceInner::UnregisterAppDebugListener(const sptr<IAppDebugListener> &listener)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    if (!AAFwk::PermissionVerification::GetInstance()->IsSACall()) {
        TAG_LOGE(AAFwkTag::APPMGR, "permission verification fail");
        return ERR_PERMISSION_DENIED;
    }

    if (appDebugManager_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "appDebugManager_ null");
        return ERR_NO_INIT;
    }
    return appDebugManager_->UnregisterAppDebugListener(listener);
}

int32_t AppMgrServiceInner::AttachAppDebug(const std::string &bundleName, bool isDebugFromLocal)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    if (!system::GetBoolParameter(DEVELOPER_MODE_STATE, false)) {
        TAG_LOGE(AAFwkTag::APPMGR, "developer mode false");
        return ERR_INVALID_OPERATION;
    }

    if (!AAFwk::PermissionVerification::GetInstance()->IsSACall() &&
        !AAFwk::PermissionVerification::GetInstance()->IsShellCall()) {
        TAG_LOGE(AAFwkTag::APPMGR, "permission verification fail");
        return ERR_PERMISSION_DENIED;
    }

    if (appRunningManager_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRunningManager_ null");
        return ERR_NO_INIT;
    }
    appRunningManager_->SetAttachAppDebug(bundleName, true, isDebugFromLocal);

    auto debugInfos = appRunningManager_->GetAppDebugInfosByBundleName(bundleName, false);
    if (!debugInfos.empty() && appDebugManager_ != nullptr) {
        appDebugManager_->StartDebug(debugInfos);
    }

    NotifyAbilitiesDebugChange(bundleName, true);
    return ERR_OK;
}

int32_t AppMgrServiceInner::DetachAppDebug(const std::string &bundleName)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    if (!AAFwk::PermissionVerification::GetInstance()->IsSACall() &&
        !AAFwk::PermissionVerification::GetInstance()->IsShellCall()) {
        TAG_LOGE(AAFwkTag::APPMGR, "permission verification fail");
        return ERR_PERMISSION_DENIED;
    }

    if (appRunningManager_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRunningManager_ null");
        return ERR_NO_INIT;
    }

    auto debugInfos = appRunningManager_->GetAppDebugInfosByBundleName(bundleName, true);
    if (!debugInfos.empty()) {
        appRunningManager_->SetAttachAppDebug(bundleName, false, false);
        if (appDebugManager_ != nullptr) {
            appDebugManager_->StopDebug(debugInfos);
        }
    }

    NotifyAbilitiesDebugChange(bundleName, false);
    return ERR_OK;
}

int32_t AppMgrServiceInner::SetAppWaitingDebug(const std::string &bundleName, bool isPersist)
{
    TAG_LOGD(AAFwkTag::APPMGR,
        "Called, bundle name is %{public}s, persist flag is %{public}d.", bundleName.c_str(), isPersist);
    if (!AAFwk::PermissionVerification::GetInstance()->IsShellCall()) {
        TAG_LOGE(AAFwkTag::APPMGR, "not shell call");
        return ERR_PERMISSION_DENIED;
    }

    if (!system::GetBoolParameter(DEVELOPER_MODE_STATE, false)) {
        TAG_LOGE(AAFwkTag::APPMGR, "developer mode false");
        return AAFwk::ERR_NOT_DEVELOPER_MODE;
    }

    if (bundleName.empty()) {
        TAG_LOGE(AAFwkTag::APPMGR, "bundle name empty");
        return ERR_INVALID_VALUE;
    }

    if (!CheckIsDebugApp(bundleName)) {
        TAG_LOGE(AAFwkTag::APPMGR, "is not debug app");
        return AAFwk::ERR_NOT_DEBUG_APP;
    }

    InitAppWaitingDebugList();

    bool isClear = false;
    {
        std::lock_guard<ffrt::mutex> lock(waitingDebugLock_);
        if (!waitingDebugBundleList_.empty()) {
            waitingDebugBundleList_.clear();
            isClear = true;
        }
    }
    if (isClear) {
        DelayedSingleton<AbilityRuntime::AppConfigDataManager>::GetInstance()->ClearAppWaitingDebugInfo();
    }

    {
        std::lock_guard<ffrt::mutex> lock(waitingDebugLock_);
        waitingDebugBundleList_.try_emplace(bundleName, isPersist);
    }
    if (isPersist) {
        return DelayedSingleton<AbilityRuntime::AppConfigDataManager>::GetInstance()->SetAppWaitingDebugInfo(
            bundleName);
    }
    return ERR_OK;
}

int32_t AppMgrServiceInner::CancelAppWaitingDebug()
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    if (!AAFwk::PermissionVerification::GetInstance()->IsShellCall()) {
        TAG_LOGE(AAFwkTag::APPMGR, "not shell call");
        return ERR_PERMISSION_DENIED;
    }

    if (!system::GetBoolParameter(DEVELOPER_MODE_STATE, false)) {
        TAG_LOGE(AAFwkTag::APPMGR, "developer mode false");
        return AAFwk::ERR_NOT_DEVELOPER_MODE;
    }

    {
        std::lock_guard<ffrt::mutex> lock(waitingDebugLock_);
        waitingDebugBundleList_.clear();
    }
    return DelayedSingleton<AbilityRuntime::AppConfigDataManager>::GetInstance()->ClearAppWaitingDebugInfo();
}

int32_t AppMgrServiceInner::GetWaitingDebugApp(std::vector<std::string> &debugInfoList)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    if (!AAFwk::PermissionVerification::GetInstance()->IsShellCall()) {
        TAG_LOGE(AAFwkTag::APPMGR, "not shell call");
        return ERR_PERMISSION_DENIED;
    }

    if (!system::GetBoolParameter(DEVELOPER_MODE_STATE, false)) {
        TAG_LOGE(AAFwkTag::APPMGR, "developer mode false");
        return AAFwk::ERR_NOT_DEVELOPER_MODE;
    }

    InitAppWaitingDebugList();

    std::lock_guard<ffrt::mutex> lock(waitingDebugLock_);
    if (waitingDebugBundleList_.empty()) {
        TAG_LOGD(AAFwkTag::APPMGR, "The waiting debug bundle list is empty.");
        return ERR_OK;
    }

    for (const auto &item : waitingDebugBundleList_) {
        std::string debugBundleInfo;
        debugBundleInfo.append("bundle name : ").append(item.first).append(", persist : ")
            .append(item.second ? "true" : "false");
        debugInfoList.emplace_back(debugBundleInfo);
    }
    return ERR_OK;
}

void AppMgrServiceInner::InitAppWaitingDebugList()
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    {
        std::lock_guard<ffrt::mutex> lock(waitingDebugLock_);
        if (isInitAppWaitingDebugListExecuted_) {
            TAG_LOGD(AAFwkTag::APPMGR, "No need to initialize again.");
            return;
        }
        isInitAppWaitingDebugListExecuted_ = true;
    }

    std::vector<std::string> bundleNameList;
    DelayedSingleton<AbilityRuntime::AppConfigDataManager>::GetInstance()->GetAppWaitingDebugList(bundleNameList);
    if (!bundleNameList.empty()) {
        std::lock_guard<ffrt::mutex> lock(waitingDebugLock_);
        for (const auto &item : bundleNameList) {
            waitingDebugBundleList_.try_emplace(item, true);
        }
    }
}

bool AppMgrServiceInner::CheckIsDebugApp(const std::string &bundleName)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    CHECK_POINTER_AND_RETURN_VALUE(remoteClientManager_, false);
    auto bundleMgrHelper = remoteClientManager_->GetBundleManagerHelper();
    CHECK_POINTER_AND_RETURN_VALUE(bundleMgrHelper, false);

    BundleInfo bundleInfo;
    auto ret = IN_PROCESS_CALL(bundleMgrHelper->GetBundleInfoV9(bundleName,
        static_cast<int32_t>(GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_APPLICATION), bundleInfo, currentUserId_));
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "getBundleInfo fail");
        return false;
    }

    return bundleInfo.applicationInfo.debug &&
           (bundleInfo.applicationInfo.appProvisionType == AppExecFwk::Constants::APP_PROVISION_TYPE_DEBUG);
}

bool AppMgrServiceInner::IsWaitingDebugApp(const std::string &bundleName)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");

    if (IPCSkeleton::GetCallingUid() != FOUNDATION_UID) {
        TAG_LOGE(AAFwkTag::APPMGR, "not foundation call");
        return false;
    }
    return IsWaitingDebugAppInner(bundleName);
}

bool AppMgrServiceInner::IsWaitingDebugAppInner(const std::string &bundleName)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    InitAppWaitingDebugList();

    std::lock_guard<ffrt::mutex> lock(waitingDebugLock_);
    if (waitingDebugBundleList_.empty()) {
        TAG_LOGD(AAFwkTag::APPMGR, "The waiting debug bundles list is empty.");
        return false;
    }

    for (const auto &item : waitingDebugBundleList_) {
        if (item.first == bundleName) {
            return true;
        }
    }
    return false;
}

void AppMgrServiceInner::ClearNonPersistWaitingDebugFlag()
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");

    if (IPCSkeleton::GetCallingUid() != FOUNDATION_UID) {
        TAG_LOGE(AAFwkTag::APPMGR, "not foundation call");
        return;
    }

    bool isClear = false;
    {
        std::lock_guard<ffrt::mutex> lock(waitingDebugLock_);
        for (const auto &item : waitingDebugBundleList_) {
            if (!item.second) {
                isClear = true;
                break;
            }
        }
        if (isClear) {
            waitingDebugBundleList_.clear();
        }
    }

    if (isClear) {
        DelayedSingleton<AbilityRuntime::AppConfigDataManager>::GetInstance()->ClearAppWaitingDebugInfo();
    }
}

int32_t AppMgrServiceInner::RegisterAbilityDebugResponse(const sptr<IAbilityDebugResponse> &response)
{
    if (IPCSkeleton::GetCallingUid() != FOUNDATION_UID) {
        TAG_LOGE(AAFwkTag::APPMGR, "not foundation call");
        return ERR_PERMISSION_DENIED;
    }
    if (response == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "response null");
        return ERR_INVALID_VALUE;
    }

    abilityDebugResponse_ = response;
    return ERR_OK;
}

int32_t AppMgrServiceInner::NotifyAbilitiesDebugChange(const std::string &bundleName, const bool &isAppDebug)
{
    if (appRunningManager_ == nullptr || abilityDebugResponse_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRunningManager_ or abilityDebugResponse null");
        return ERR_NO_INIT;
    }

    std::vector<sptr<IRemoteObject>> tokens;
    appRunningManager_->GetAbilityTokensByBundleName(bundleName, tokens);
    if (!tokens.empty()) {
        isAppDebug ? abilityDebugResponse_->OnAbilitysDebugStarted(tokens) :
            abilityDebugResponse_->OnAbilitysDebugStoped(tokens);
    }
    return ERR_OK;
}

int32_t AppMgrServiceInner::NotifyAbilitiesAssertDebugChange(
    const std::shared_ptr<AppRunningRecord> &appRecord, bool isAssertDebug)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    if (appRecord == nullptr || abilityDebugResponse_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "record or abilityDebugResponse null");
        return ERR_NO_INIT;
    }

    std::vector<sptr<IRemoteObject>> abilityTokens;
    auto abilities = appRecord->GetAbilities();
    for (const auto &token : abilities) {
        abilityTokens.emplace_back(token.first);
    }

    if (!abilityTokens.empty()) {
        abilityDebugResponse_->OnAbilitysAssertDebugChange(abilityTokens, isAssertDebug);
    }
    return ERR_OK;
}

bool AppMgrServiceInner::IsAttachDebug(const std::string &bundleName)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    auto isSaCall = AAFwk::PermissionVerification::GetInstance()->IsSACall();
    if (!isSaCall) {
        TAG_LOGE(AAFwkTag::APPMGR, "caller token not SA");
        return false;
    }
    if (appRunningManager_ == nullptr || bundleName.empty()) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRunningManager_ or bundleName null");
        return false;
    }
    return appDebugManager_->IsAttachDebug(bundleName);
}

void AppMgrServiceInner::ApplicationTerminatedSendProcessEvent(const std::shared_ptr<AppRunningRecord> &appRecord)
{
    if (appRecord == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRecord null");
        return;
    }

    DelayedSingleton<AppStateObserverManager>::GetInstance()->OnProcessDied(appRecord);
    DelayedSingleton<CacheProcessManager>::GetInstance()->OnProcessKilled(appRecord);
    if (!GetAppRunningStateByBundleName(appRecord->GetBundleName())) {
        RemoveRunningSharedBundleList(appRecord->GetBundleName());
    }

    if (appRunningManager_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "running manager null");
        return;
    }
    auto appInfo = appRecord->GetApplicationInfo();
    if (appInfo != nullptr && !appRunningManager_->IsAppExist(appInfo->accessTokenId)) {
        OnAppStopped(appRecord);
    }

    if (appDebugManager_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "debug manager null");
        return;
    }
    auto info = MakeAppDebugInfo(appRecord, appRecord->IsDebugApp());
    appDebugManager_->RemoveAppDebugInfo(info);

    TAG_LOGD(AAFwkTag::APPMGR, "Application is terminated.");
    SendProcessExitEvent(appRecord);
}

void AppMgrServiceInner::ClearAppRunningDataForKeepAlive(const std::shared_ptr<AppRunningRecord> &appRecord)
{
    ClearResidentProcessAppRunningData(appRecord);
    ClearNonResidentKeepAliveAppRunningData(appRecord);
}

void AppMgrServiceInner::ClearResidentProcessAppRunningData(const std::shared_ptr<AppRunningRecord> &appRecord)
{
    if (appRecord == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRecord null");
        return;
    }

    auto userId = GetUserIdByUid(appRecord->GetUid());
    if (appRecord->IsKeepAliveApp() && (userId == 0 || userId == currentUserId_) &&
        appRecord->GetBundleName() != SCENE_BOARD_BUNDLE_NAME) {
        if (ExitResidentProcessManager::GetInstance().IsKilledForUpgradeWeb(appRecord->GetBundleName())) {
            TAG_LOGI(AAFwkTag::APPMGR, "is killed for upgrade web");
            return;
        }
        if (!AAFwk::AppUtils::GetInstance().IsAllowResidentInExtremeMemory(appRecord->GetBundleName()) &&
            ExitResidentProcessManager::GetInstance().RecordExitResidentBundleName(appRecord->GetBundleName(),
                appRecord->GetUid())) {
            TAG_LOGI(AAFwkTag::APPMGR, "memory size insufficient");
            return;
        }
        TAG_LOGI(AAFwkTag::APPMGR, "memory size sufficient");
        auto restartProcess = [appRecord, innerService = shared_from_this()]() {
            innerService->RestartResidentProcess(appRecord);
        };
        if (taskHandler_ == nullptr) {
            TAG_LOGE(AAFwkTag::APPMGR, "taskHandler_ null");
            return;
        }
        if (appRecord->CanRestartResidentProc()) {
            taskHandler_->SubmitTask(restartProcess, "RestartResidentProcess");
        } else {
            auto findRestartResidentTask = [appRecord](const std::shared_ptr<AppRunningRecord> &appRunningRecord) {
                return (appRecord != nullptr && appRunningRecord != nullptr &&
                        appRecord->GetBundleName() == appRunningRecord->GetBundleName());
            };
            {
                std::lock_guard guard(restartResidentTaskListMutex_);
                auto findIter = find_if(restartResidentTaskList_.begin(),
                    restartResidentTaskList_.end(),
                    findRestartResidentTask);
                if (findIter != restartResidentTaskList_.end()) {
                    TAG_LOGW(AAFwkTag::APPMGR, "reboot task already registered");
                    return;
                }
                restartResidentTaskList_.emplace_back(appRecord);
            }
            TAG_LOGD(AAFwkTag::APPMGR, "Post restart resident process delay task.");
            taskHandler_->SubmitTaskJust(restartProcess, "RestartResidentProcessDelayTask", RESTART_INTERVAL_TIME);
        }
    }
}

void AppMgrServiceInner::ClearNonResidentKeepAliveAppRunningData(const std::shared_ptr<AppRunningRecord> &appRecord)
{
    if (appRecord == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRecord null");
        return;
    }

    auto userId = GetUserIdByUid(appRecord->GetUid());
    bool isDefaultInstance = appRecord->GetInstanceKey().empty() || appRecord->GetInstanceKey() == APP_INSTANCE_KEY_0;
    if (!appRecord->GetRestartAppFlag() && appRecord->IsKeepAliveDkv() &&
        isDefaultInstance && (userId == 0 || userId == currentUserId_) &&
        appRecord->GetBundleName() != SCENE_BOARD_BUNDLE_NAME) {
        if (ExitResidentProcessManager::GetInstance().IsKilledForUpgradeWeb(appRecord->GetBundleName())) {
            TAG_LOGI(AAFwkTag::APPMGR, "is killed for upgrade web");
            return;
        }
        if (!IsMemorySizeSufficient()) {
            TAG_LOGI(AAFwkTag::APPMGR, "memory size insufficient");
            return;
        }
        TAG_LOGI(AAFwkTag::APPMGR, "memory size sufficient");
        auto restartProcess = [appRecord, innerService = shared_from_this()]() {
            TAG_LOGI(AAFwkTag::APPMGR, "restarting keep-alive process.");
            innerService->RestartKeepAliveProcess(appRecord);
        };
        if (taskHandler_ == nullptr) {
            TAG_LOGE(AAFwkTag::APPMGR, "taskHandler_ null");
            return;
        }
        TAG_LOGI(AAFwkTag::APPMGR, "submit restart keep-alive process task.");
        taskHandler_->SubmitTask(restartProcess, "RestartKeepAliveProcess");
    }
}

int32_t AppMgrServiceInner::NotifyPageShow(const sptr<IRemoteObject> &token, const PageStateData &pageStateData)
{
    if (!JudgeSelfCalledByToken(token, pageStateData)) {
        return ERR_PERMISSION_DENIED;
    }

    const_cast<PageStateData &>(pageStateData).uid = IPCSkeleton::GetCallingUid();
    DelayedSingleton<AppStateObserverManager>::GetInstance()->OnPageShow(pageStateData);
    return ERR_OK;
}

int32_t AppMgrServiceInner::NotifyPageHide(const sptr<IRemoteObject> &token, const PageStateData &pageStateData)
{
    if (!JudgeSelfCalledByToken(token, pageStateData)) {
        return ERR_PERMISSION_DENIED;
    }

    DelayedSingleton<AppStateObserverManager>::GetInstance()->OnPageHide(pageStateData);
    return ERR_OK;
}

bool AppMgrServiceInner::JudgeSelfCalledByToken(const sptr<IRemoteObject> &token, const PageStateData &pageStateData)
{
    if (!token) {
        TAG_LOGE(AAFwkTag::APPMGR, "token null");
        return false;
    }
    auto appRecord = GetAppRunningRecordByAbilityToken(token);
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "app unexist");
        return false;
    }
    auto callingTokenId = IPCSkeleton::GetCallingTokenID();
    if (appRecord->GetApplicationInfo() == nullptr ||
        ((appRecord->GetApplicationInfo())->accessTokenId) != callingTokenId) {
        TAG_LOGE(AAFwkTag::APPMGR, "is not self");
        return false;
    }
    auto abilityRecord = appRecord->GetAbilityRunningRecordByToken(token);
    if (!abilityRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "can't find record");
        return false;
    }
    if (abilityRecord->GetBundleName() != pageStateData.bundleName ||
        abilityRecord->GetModuleName() != pageStateData.moduleName ||
        abilityRecord->GetName() != pageStateData.abilityName) {
        TAG_LOGE(AAFwkTag::APPMGR, "can't map ability");
        return false;
    }
    return true;
}

int32_t AppMgrServiceInner::RegisterAppRunningStatusListener(const sptr<IRemoteObject> &listener)
{
    TAG_LOGD(AAFwkTag::APPMGR, "Call.");
    CHECK_IS_SA_CALL(listener);
    auto appRunningStatusListener = iface_cast<AbilityRuntime::AppRunningStatusListenerInterface>(listener);
    return appRunningStatusModule_->RegisterListener(appRunningStatusListener);
}

int32_t AppMgrServiceInner::UnregisterAppRunningStatusListener(const sptr<IRemoteObject> &listener)
{
    TAG_LOGD(AAFwkTag::APPMGR, "Call.");
    CHECK_IS_SA_CALL(listener);
    auto appRunningStatusListener = iface_cast<AbilityRuntime::AppRunningStatusListenerInterface>(listener);
    return appRunningStatusModule_->UnregisterListener(appRunningStatusListener);
}

#ifdef SUPPORT_CHILD_PROCESS
int32_t AppMgrServiceInner::StartChildProcess(const pid_t callingPid, pid_t &childPid,
    const ChildProcessRequest &request)
{
    TAG_LOGI(AAFwkTag::APPMGR, "callingPid:%{public}d", callingPid);
    auto errCode = StartChildProcessPreCheck(callingPid, request.childProcessType);
    if (errCode != ERR_OK) {
        return errCode;
    }
    auto &srcEntry = request.srcEntry;
    if (callingPid <= 0 || srcEntry.empty()) {
        TAG_LOGE(AAFwkTag::APPMGR, "invalid callingPid:%{public}d srcEntry:%{private}s", callingPid, srcEntry.c_str());
        return ERR_INVALID_VALUE;
    }
    if (UserRecordManager::GetInstance().IsLogoutUser(GetUserIdByUid(IPCSkeleton::GetCallingUid()))) {
        TAG_LOGE(AAFwkTag::APPMGR, "disable start process in logout user");
        return ERR_INVALID_OPERATION;
    }
    CHECK_POINTER_AND_RETURN_VALUE(appRunningManager_, ERR_NO_INIT);
    auto appRecord = GetAppRunningRecordByPid(callingPid);
    auto childProcessRecord = ChildProcessRecord::CreateChildProcessRecord(callingPid, request, appRecord);
    if (!childProcessRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "childProcessRecord null");
        return ERR_NULL_OBJECT;
    }
    auto &args = request.args;
    auto &options = request.options;
    childProcessRecord->SetEntryParams(args.entryParams);
    TAG_LOGI(AAFwkTag::APPMGR, "srcEntry:%{private}s, args.entryParams size:%{public}zu,"
        " processName:%{public}s, args.fds size:%{public}zu, options.isolationMode:%{public}d",
        request.srcEntry.c_str(), args.entryParams.length(), childProcessRecord->GetProcessName().c_str(),
        args.fds.size(), options.isolationMode);
    return StartChildProcessImpl(childProcessRecord, appRecord, childPid, args, options);
}

int32_t AppMgrServiceInner::StartChildProcessPreCheck(pid_t callingPid, int32_t childProcessType)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called.");
    CHECK_POINTER_AND_RETURN_VALUE(appRunningManager_, ERR_NO_INIT);
    auto childRecord = appRunningManager_->GetAppRunningRecordByChildProcessPid(callingPid);
    if (childRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "already in child process.");
        return AAFwk::ERR_ALREADY_IN_CHILD_PROCESS;
    }
    auto hostRecord = GetAppRunningRecordByPid(callingPid);
    CHECK_POINTER_AND_RETURN_VALUE(hostRecord, ERR_NULL_OBJECT);
    auto &appUtils = AAFwk::AppUtils::GetInstance();
    if (!appUtils.IsMultiProcessModel()) {
        bool checkAllowList = childProcessType == CHILD_PROCESS_TYPE_NATIVE_ARGS ||
            childProcessType == CHILD_PROCESS_TYPE_NATIVE;
        if (!checkAllowList || !appUtils.IsAllowNativeChildProcess(hostRecord->GetAppIdentifier())) {
            TAG_LOGE(AAFwkTag::APPMGR, "not support child process.");
            return AAFwk::ERR_NOT_SUPPORT_CHILD_PROCESS;
        }
    }
    auto applicationInfo = hostRecord->GetApplicationInfo();
    CHECK_POINTER_AND_RETURN_VALUE(applicationInfo, ERR_NULL_OBJECT);
    if (appRunningManager_->IsChildProcessReachLimit(applicationInfo->accessTokenId)) {
        TAG_LOGE(AAFwkTag::APPMGR, "child process count reach limit.");
        return AAFwk::ERR_CHILD_PROCESS_REACH_LIMIT;
    }
    return ERR_OK;
}

int32_t AppMgrServiceInner::StartChildProcessImpl(const std::shared_ptr<ChildProcessRecord> childProcessRecord,
    const std::shared_ptr<AppRunningRecord> appRecord, pid_t &childPid, const ChildProcessArgs &args,
    const ChildProcessOptions &options)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "no appRecord, childPid:%{public}d", childPid);
        return ERR_NAME_NOT_FOUND;
    }
    if (!childProcessRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "no child process record, childPid:%{public}d", childPid);
        return ERR_NAME_NOT_FOUND;
    }
    bool isNativeFromJs = childProcessRecord->GetChildProcessType() == CHILD_PROCESS_TYPE_NATIVE_ARGS;
    auto spawnClient =  isNativeFromJs ? remoteClientManager_->GetNativeSpawnClient() :
        remoteClientManager_->GetSpawnClient();
    if (!spawnClient) {
        TAG_LOGE(AAFwkTag::APPMGR, "spawnClient null");
        AppMgrEventUtil::SendChildProcessStartFailedEvent(childProcessRecord,
            ProcessStartFailedReason::GET_SPAWN_CLIENT_FAILED, ERR_APPEXECFWK_BAD_APPSPAWN_CLIENT);
        return ERR_APPEXECFWK_BAD_APPSPAWN_CLIENT;
    }
    if (!args.CheckFdsSize() || !args.CheckFdsKeyLength()) {
        AppMgrEventUtil::SendChildProcessStartFailedEvent(childProcessRecord,
            ProcessStartFailedReason::CHECK_CHILD_FDS_FAILED, ERR_INVALID_VALUE);
        return ERR_INVALID_VALUE;
    }

    AppSpawnStartMsg startMsg = appRecord->GetStartMsg();
    startMsg.procName = childProcessRecord->GetProcessName();
    startMsg.childProcessType = childProcessRecord->GetChildProcessType();
    startMsg.fds = args.fds;
    startMsg.isolationMode = options.isolationMode;
    pid_t pid = 0;
    {
        std::lock_guard<ffrt::mutex> lock(startChildProcessLock_);
        ErrCode errCode = spawnClient->StartProcess(startMsg, pid);
        if (FAILED(errCode)) {
            TAG_LOGE(AAFwkTag::APPMGR, "spawn new child process fail, errCode %{public}08x", errCode);
            AppMgrEventUtil::SendChildProcessStartFailedEvent(childProcessRecord,
                ProcessStartFailedReason::APPSPAWN_FAILED, static_cast<int32_t>(errCode));
            return ERR_APPEXECFWK_BAD_APPSPAWN_CLIENT;
        }
        childPid = pid;
        childProcessRecord->SetPid(pid);
        childProcessRecord->SetUid(startMsg.uid);
        appRecord->AddChildProcessRecord(pid, childProcessRecord);
    }
    TAG_LOGI(AAFwkTag::APPMGR, "start childProcess success,pid:%{public}d,hostPid:%{public}d,uid:%{public}d,"
        "processName:%{public}s", pid, childProcessRecord->GetHostPid(), startMsg.uid,
        childProcessRecord->GetProcessName().c_str());
    DelayedSingleton<AppStateObserverManager>::GetInstance()->OnChildProcessCreated(childProcessRecord);
    return ERR_OK;
}

int32_t AppMgrServiceInner::GetChildProcessInfoForSelf(ChildProcessInfo &info)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    if (!appRunningManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRunningManager_ null");
        return ERR_NO_INIT;
    }
    auto callingPid = IPCSkeleton::GetCallingPid();
    if (appRunningManager_->GetAppRunningRecordByPid(callingPid)) {
        TAG_LOGD(AAFwkTag::APPMGR, "record of callingPid is not child record.");
        return ERR_NAME_NOT_FOUND;
    }
    std::lock_guard<ffrt::mutex> lock(startChildProcessLock_);
    auto appRecord = appRunningManager_->GetAppRunningRecordByChildProcessPid(callingPid);
    if (!appRecord) {
        TAG_LOGW(AAFwkTag::APPMGR, "no appRecord, childPid:%{public}d", callingPid);
        return ERR_NAME_NOT_FOUND;
    }
    auto childRecordMap = appRecord->GetChildProcessRecordMap();
    auto iter = childRecordMap.find(callingPid);
    if (iter != childRecordMap.end()) {
        auto childProcessRecord = iter->second;
        return GetChildProcessInfo(childProcessRecord, appRecord, info);
    }
    return ERR_NAME_NOT_FOUND;
}

int32_t AppMgrServiceInner::GetChildProcessInfo(const std::shared_ptr<ChildProcessRecord> childProcessRecord,
    const std::shared_ptr<AppRunningRecord> appRecord, ChildProcessInfo &info, bool isCallFromGetChildrenProcesses)
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
    auto osAccountMgr = DelayedSingleton<OsAccountManagerWrapper>::GetInstance();
    if (osAccountMgr == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "osAccountMgr is nullptr");
        return ERR_INVALID_VALUE;
    }
    int32_t userId = -1;
    int errCode = osAccountMgr->GetOsAccountLocalIdFromUid(appRecord->GetUid(), userId);
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
    if (!isCallFromGetChildrenProcesses) {
        info.childProcessType = childProcessRecord->GetChildProcessType();
        info.srcEntry = childProcessRecord->GetSrcEntry();
        info.entryFunc = childProcessRecord->GetEntryFunc();
        info.entryParams = childProcessRecord->GetEntryParams();
        info.jitEnabled = appRecord->IsJITEnabled();
        info.isStartWithDebug = childProcessRecord->isStartWithDebug();
        auto applicationInfo = appRecord->GetApplicationInfo();
        if (applicationInfo) {
            TAG_LOGD(AAFwkTag::APPMGR, "applicationInfo is exist, debug:%{public}d", applicationInfo->debug);
            info.isDebugApp = applicationInfo->debug;
        }
        info.isStartWithNative = appRecord->isNativeStart();
    }
    return ERR_OK;
}

void AppMgrServiceInner::AttachChildProcess(const pid_t pid, const sptr<IChildScheduler> &childScheduler)
{
    TAG_LOGI(AAFwkTag::APPMGR, "attachChildProcess pid:%{public}d", pid);
    if (pid <= 0) {
        TAG_LOGE(AAFwkTag::APPMGR, "invalid child process pid:%{public}d", pid);
        return;
    }
    if (!childScheduler) {
        TAG_LOGE(AAFwkTag::APPMGR, "childScheduler null");
        return;
    }
    if (!appRunningManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRunningManager_ null");
        return;
    }
    auto appRecord = appRunningManager_->GetAppRunningRecordByChildProcessPid(pid);
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "no appRecord, pid:%{public}d", pid);
        return;
    }
    auto childRecord = appRecord->GetChildProcessRecordByPid(pid);
    if (!childRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "no child process record, pid:%{public}d", pid);
        return;
    }

    sptr<AppDeathRecipient> appDeathRecipient = new AppDeathRecipient();
    appDeathRecipient->SetTaskHandler(taskHandler_);
    appDeathRecipient->SetAppMgrServiceInner(shared_from_this());
    appDeathRecipient->SetIsChildProcess(true);
    childRecord->SetScheduler(childScheduler);
    childRecord->SetDeathRecipient(appDeathRecipient);
    childRecord->RegisterDeathRecipient();

    if (childRecord->GetChildProcessType() != CHILD_PROCESS_TYPE_NATIVE) {
        childScheduler->ScheduleLoadChild();
    } else {
        childScheduler->ScheduleRunNativeProc(childRecord->GetMainProcessCallback());
        childRecord->ClearMainProcessCallback();
    }
}

void AppMgrServiceInner::OnChildProcessRemoteDied(const wptr<IRemoteObject> &remote)
{
    if (appRunningManager_) {
        auto childRecord = appRunningManager_->OnChildProcessRemoteDied(remote);
        if (childRecord) {
            DelayedSingleton<AppStateObserverManager>::GetInstance()->OnChildProcessDied(childRecord);
        }
    }
}

void AppMgrServiceInner::KillChildProcess(const std::shared_ptr<AppRunningRecord> &appRecord) {
    if (appRecord == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRecord null");
        return;
    }
    auto childRecordMap = appRecord->GetChildProcessRecordMap();
    if (childRecordMap.empty()) {
        return;
    }
    for (auto iter : childRecordMap) {
        auto childRecord = iter.second;
        if (!childRecord) {
            continue;
        }
        auto childPid = childRecord->GetPid();
        if (childPid > 0) {
            TAG_LOGI(AAFwkTag::APPMGR, "kill child process, childPid:%{public}d, childUid:%{public}d",
                childPid, childRecord->GetUid());
            KillProcessByPid(childPid, "KillChildProcess");
            DelayedSingleton<AppStateObserverManager>::GetInstance()->OnChildProcessDied(childRecord);
        }
    }
}

void AppMgrServiceInner::ExitChildProcessSafelyByChildPid(const pid_t pid)
{
    if (pid <= 0) {
        TAG_LOGE(AAFwkTag::APPMGR, "pid <= 0");
        return;
    }
    auto appRecord = appRunningManager_->GetAppRunningRecordByChildProcessPid(pid);
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "no apprecord, pid:%{public}d", pid);
        return;
    }
    auto childRecord = appRecord->GetChildProcessRecordByPid(pid);
    if (!childRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "no child process record, pid:%{public}d", pid);
        return;
    }
    childRecord->ScheduleExitProcessSafely();
    childRecord->RemoveDeathRecipient();
    int64_t startTime = SystemTimeMillisecond();
    std::list<pid_t> pids;
    pids.push_back(pid);
    if (WaitForRemoteProcessExit(pids, startTime)) {
        TAG_LOGI(AAFwkTag::APPMGR, "remote child process exited, pid:%{public}d", pid);
        appRecord->RemoveChildProcessRecord(childRecord);
        DelayedSingleton<AppStateObserverManager>::GetInstance()->OnChildProcessDied(childRecord);
        return;
    }
    childRecord->RegisterDeathRecipient();
    TAG_LOGI(AAFwkTag::APPMGR, "kill child process, childPid:%{public}d, childUid:%{public}d",
        pid, childRecord->GetUid());
    int32_t result = KillProcessByPid(pid, "ExitChildProcessSafelyByChildPid");
    if (result < 0) {
        TAG_LOGE(AAFwkTag::APPMGR, "kill killChildProcessByPid fail");
        return;
    }
}

void AppMgrServiceInner::KillAttachedChildProcess(const std::shared_ptr<AppRunningRecord> &appRecord)
{
    if (appRecord == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRecord null");
        return;
    }
    auto parentAppRecord = appRecord->GetParentAppRecord();
    if (parentAppRecord) {
        parentAppRecord->RemoveChildAppRecord(appRecord->GetPid());
    }
    std::vector<pid_t> pids;
    std::queue<std::shared_ptr<AppRunningRecord>> queue;
    queue.push(appRecord);
    while (!queue.empty()) {
        auto front = queue.front();
        queue.pop();
        if (front == nullptr) {
            continue;
        }
        auto childAppRecordMap = front->GetChildAppRecordMap();
        for (const auto& [pid, weakChildAppRecord] : childAppRecordMap) {
            auto childRecord = weakChildAppRecord.lock();
            if (childRecord) {
                queue.push(childRecord);
                pids.push_back(pid);
            }
        }
        front->ClearChildAppRecordMap();
    }
    for (const auto& pid : pids) {
        KillProcessByPid(pid, "KillAttachedChildProcess");
    }
}
#endif // SUPPORT_CHILD_PROCESS

int AppMgrServiceInner::DumpIpcAllStart(std::string& result)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    if (!appRunningManager_) {
        result.append(MSG_DUMP_IPC_START_STAT, strlen(MSG_DUMP_IPC_START_STAT))
            .append(MSG_DUMP_FAIL, strlen(MSG_DUMP_FAIL))
            .append(MSG_DUMP_FAIL_REASON_INTERNAL, strlen(MSG_DUMP_FAIL_REASON_INTERNAL));
        TAG_LOGE(AAFwkTag::APPMGR, "appRunningManager_ null");
        return DumpErrorCode::ERR_INTERNAL_ERROR;
    }
    return appRunningManager_->DumpIpcAllStart(result);
}

int AppMgrServiceInner::DumpIpcAllStop(std::string& result)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    if (!appRunningManager_) {
        result.append(MSG_DUMP_IPC_STOP_STAT, strlen(MSG_DUMP_IPC_STOP_STAT))
            .append(MSG_DUMP_FAIL, strlen(MSG_DUMP_FAIL))
            .append(MSG_DUMP_FAIL_REASON_INTERNAL, strlen(MSG_DUMP_FAIL_REASON_INTERNAL));
        TAG_LOGE(AAFwkTag::APPMGR, "appRunningManager_ null");
        return DumpErrorCode::ERR_INTERNAL_ERROR;
    }
    return appRunningManager_->DumpIpcAllStop(result);
}

int AppMgrServiceInner::DumpIpcAllStat(std::string& result)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    if (!appRunningManager_) {
        result.append(MSG_DUMP_IPC_STAT, strlen(MSG_DUMP_IPC_STAT))
            .append(MSG_DUMP_FAIL, strlen(MSG_DUMP_FAIL))
            .append(MSG_DUMP_FAIL_REASON_INTERNAL, strlen(MSG_DUMP_FAIL_REASON_INTERNAL));
        TAG_LOGE(AAFwkTag::APPMGR, "appRunningManager_ null");
        return DumpErrorCode::ERR_INTERNAL_ERROR;
    }
    return appRunningManager_->DumpIpcAllStat(result);
}

int AppMgrServiceInner::DumpIpcStart(const int32_t pid, std::string& result)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    if (!appRunningManager_) {
        result.append(MSG_DUMP_IPC_START_STAT, strlen(MSG_DUMP_IPC_START_STAT))
            .append(MSG_DUMP_FAIL, strlen(MSG_DUMP_FAIL))
            .append(MSG_DUMP_FAIL_REASON_INTERNAL, strlen(MSG_DUMP_FAIL_REASON_INTERNAL));
        TAG_LOGE(AAFwkTag::APPMGR, "appRunningManager_ null");
        return DumpErrorCode::ERR_INTERNAL_ERROR;
    }
    return appRunningManager_->DumpIpcStart(pid, result);
}

int AppMgrServiceInner::DumpIpcStop(const int32_t pid, std::string& result)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    if (!appRunningManager_) {
        result.append(MSG_DUMP_IPC_STOP_STAT, strlen(MSG_DUMP_IPC_STOP_STAT))
            .append(MSG_DUMP_FAIL, strlen(MSG_DUMP_FAIL))
            .append(MSG_DUMP_FAIL_REASON_INTERNAL, strlen(MSG_DUMP_FAIL_REASON_INTERNAL));
        TAG_LOGE(AAFwkTag::APPMGR, "appRunningManager_ null");
        return DumpErrorCode::ERR_INTERNAL_ERROR;
    }
    return appRunningManager_->DumpIpcStop(pid, result);
}

int AppMgrServiceInner::DumpIpcStat(const int32_t pid, std::string& result)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    if (!appRunningManager_) {
        result.append(MSG_DUMP_IPC_STAT, strlen(MSG_DUMP_IPC_STAT))
            .append(MSG_DUMP_FAIL, strlen(MSG_DUMP_FAIL))
            .append(MSG_DUMP_FAIL_REASON_INTERNAL, strlen(MSG_DUMP_FAIL_REASON_INTERNAL));
        TAG_LOGE(AAFwkTag::APPMGR, "appRunningManager_ null");
        return DumpErrorCode::ERR_INTERNAL_ERROR;
    }
    return appRunningManager_->DumpIpcStat(pid, result);
}

int AppMgrServiceInner::DumpFfrt(const std::vector<int32_t>& pids, std::string& result)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    if (!appRunningManager_) {
        result.append(MSG_DUMP_FAIL, strlen(MSG_DUMP_FAIL))
            .append(MSG_DUMP_FAIL_REASON_INTERNAL, strlen(MSG_DUMP_FAIL_REASON_INTERNAL));
        TAG_LOGE(AAFwkTag::APPMGR, "appRunningManager_ null");
        return DumpErrorCode::ERR_INTERNAL_ERROR;
    }
    return appRunningManager_->DumpFfrt(pids, result);
}

void AppMgrServiceInner::NotifyAppRunningStatusEvent(
    const std::string &bundle, int32_t uid, AbilityRuntime::RunningStatus runningStatus)
{
    if (appRunningStatusModule_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "get running status module object null");
        return;
    }
    appRunningStatusModule_->NotifyAppRunningStatusEvent(bundle, uid, runningStatus);
}

void AppMgrServiceInner::SendAppLaunchEvent(const std::shared_ptr<AppRunningRecord> &appRecord)
{
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRecord null");
        return;
    }
    AAFwk::EventInfo eventInfo;
    auto applicationInfo = appRecord->GetApplicationInfo();
    if (!applicationInfo) {
        TAG_LOGE(AAFwkTag::APPMGR, "appInfo null");
    } else {
        eventInfo.bundleName = applicationInfo->name;
        eventInfo.versionName = applicationInfo->versionName;
        eventInfo.versionCode = applicationInfo->versionCode;
    }
    if (appRecord->GetPriorityObject() != nullptr) {
        eventInfo.pid = appRecord->GetPid();
    }
    eventInfo.processName = appRecord->GetProcessName();
    int32_t callerPid = appRecord->GetCallerPid() == -1 ?
        IPCSkeleton::GetCallingPid() : appRecord->GetCallerPid();
    auto callerRecord = GetAppRunningRecordByPid(callerPid);
    if (callerRecord != nullptr) {
        eventInfo.callerBundleName = callerRecord->GetBundleName();
        eventInfo.callerUid = callerRecord->GetUid();
        eventInfo.callerState = static_cast<int32_t>(callerRecord->GetState());
        auto callerApplicationInfo = callerRecord->GetApplicationInfo();
        if (callerApplicationInfo != nullptr) {
            eventInfo.callerVersionName = callerApplicationInfo->versionName;
            eventInfo.callerVersionCode = callerApplicationInfo->versionCode;
        }
    }
    AAFwk::EventReport::SendAppLaunchEvent(AAFwk::EventName::APP_LAUNCH, eventInfo);
}

bool AppMgrServiceInner::IsFinalAppProcessByBundleName(const std::string &bundleName)
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

void AppMgrServiceInner::ParseServiceExtMultiProcessWhiteList()
{
    auto serviceExtMultiProcessWhiteList =
        OHOS::system::GetParameter(SERVICE_EXT_MULTI_PROCESS_WHITE_LIST, "");
    if (serviceExtMultiProcessWhiteList.empty()) {
        TAG_LOGW(AAFwkTag::APPMGR, "no multi process white list");
        return;
    }
    SplitStr(serviceExtMultiProcessWhiteList, ";", serviceExtensionWhiteList_);
}

void AppMgrServiceInner::ClearProcessByToken(sptr<IRemoteObject> token)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    if (token == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "token null");
        return;
    }

    std::shared_ptr<AppRunningRecord> appRecord = nullptr;
    {
        std::lock_guard lock(exceptionLock_);
        appRecord = GetAppRunningRecordByAbilityToken(token);
        if (appRecord == nullptr) {
            TAG_LOGI(AAFwkTag::APPMGR, "null appRecord");
            return;
        }
        appRecord->SetApplicationClient(nullptr);
        auto recordId = appRecord->GetRecordId();
        if (appRunningManager_ == nullptr) {
            TAG_LOGE(AAFwkTag::APPMGR, "appRunningManager_ null");
            return;
        }
        appRunningManager_->RemoveAppRunningRecordById(recordId);
    }
    ClearData(appRecord);
}

void AppMgrServiceInner::ClearData(std::shared_ptr<AppRunningRecord> appRecord)
{
    if (appRecord == nullptr) {
        TAG_LOGW(AAFwkTag::APPMGR, "null appRecord");
        return;
    }
    ClearAppRunningData(appRecord);
    if (!GetAppRunningStateByBundleName(appRecord->GetBundleName())) {
        RemoveRunningSharedBundleList(appRecord->GetBundleName());
    }
}

int32_t AppMgrServiceInner::RegisterRenderStateObserver(const sptr<IRenderStateObserver> &observer)
{
    if (observer == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "observer null");
        return ERR_INVALID_VALUE;
    }
    return DelayedSingleton<RenderStateObserverManager>::GetInstance()->RegisterRenderStateObserver(observer);
}

int32_t AppMgrServiceInner::UnregisterRenderStateObserver(const sptr<IRenderStateObserver> &observer)
{
    if (observer == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "observer null");
        return ERR_INVALID_VALUE;
    }
    return DelayedSingleton<RenderStateObserverManager>::GetInstance()->UnregisterRenderStateObserver(observer);
}

void AppMgrServiceInner::SetAppAssertionPauseState(bool flag)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    if (!system::GetBoolParameter(PRODUCT_ASSERT_FAULT_DIALOG_ENABLED, false)) {
        TAG_LOGE(AAFwkTag::APPMGR, "product of assert fault dialog not enabled");
        return;
    }
    if (!system::GetBoolParameter(DEVELOPER_MODE_STATE, false)) {
        TAG_LOGE(AAFwkTag::APPMGR, "developer mode false");
        return;
    }

    auto callerPid = IPCSkeleton::GetCallingPid();
    auto appRecord = GetAppRunningRecordByPid(callerPid);
    if (appRecord == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "no appRecord pid: %{public}d", callerPid);
        return;
    }
    appRecord->SetAssertionPauseFlag(flag);
    auto isDebugStart = appRecord->IsDebugApp() || appRecord->IsAttachDebug();
    if (!isDebugStart) {
        std::vector<AppDebugInfo> debugInfos;
        debugInfos.emplace_back(MakeAppDebugInfo(appRecord, flag));
        flag ? appDebugManager_->StartDebug(debugInfos) : appDebugManager_->StopDebug(debugInfos);
    }

    NotifyAbilitiesAssertDebugChange(appRecord, flag);
}

int32_t AppMgrServiceInner::UpdateRenderState(pid_t renderPid, int32_t state)
{
    int32_t hostPid = IPCSkeleton::GetCallingPid();
    auto appRecord = GetAppRunningRecordByPid(hostPid);
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "no appRecord, hostPid:%{public}d", hostPid);
        return ERR_INVALID_VALUE;
    }

    auto renderRecord = appRecord->GetRenderRecordByPid(renderPid);
    if (renderRecord == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "renderPid:%{pubclic}d unexist", renderPid);
        return ERR_INVALID_VALUE;
    }
    renderRecord->SetState(state);
    return DelayedSingleton<RenderStateObserverManager>::GetInstance()->OnRenderStateChanged(
        renderRecord, state);
}

int32_t AppMgrServiceInner::SignRestartAppFlag(int32_t uid, const std::string &instanceKey)
{
    TAG_LOGD(AAFwkTag::APPMGR, "call.");
    if (!appRunningManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRunningManager_ null");
        return ERR_NO_INIT;
    }
    return appRunningManager_->SignRestartAppFlag(uid, instanceKey);
}

int32_t AppMgrServiceInner::GetAppRunningUniqueIdByPid(pid_t pid, std::string &appRunningUniqueId)
{
    TAG_LOGD(AAFwkTag::APPMGR, "call.");
    if (!appRunningManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRunningManager_ null");
        return ERR_NO_INIT;
    }
    bool isCallingPermission = AAFwk::PermissionVerification::GetInstance()->IsSACall() &&
        AAFwk::PermissionVerification::GetInstance()->VerifyRunningInfoPerm();
    auto isShellCall = AAFwk::PermissionVerification::GetInstance()->IsShellCall();
    bool isDevelopMode = system::GetBoolParameter(DEVELOPER_MODE_STATE, false);
    if (!isCallingPermission && !(isShellCall && isDevelopMode)) {
        TAG_LOGE(AAFwkTag::APPMGR, "GetAppRunningUniqueIdByPid not SA call or verification failed");
        return ERR_PERMISSION_DENIED;
    }
    return appRunningManager_->GetAppRunningUniqueIdByPid(pid, appRunningUniqueId);
}

bool AppMgrServiceInner::NotifyMemMgrPriorityChanged(const std::shared_ptr<AppRunningRecord> appRecord)
{
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRecord null");
        return false;
    }
    auto priorityObject = appRecord->GetPriorityObject();
    if (!priorityObject) {
        TAG_LOGE(AAFwkTag::APPMGR, "priorityObject null");
        return false;
    }
    int32_t pid = priorityObject->GetPid();
    int32_t uid = appRecord->GetUid();
    TAG_LOGI(AAFwkTag::APPMGR, "pid:%{public}d, uid:%{public}d", pid, uid);

    Memory::MemMgrProcessStateInfo info;
    info.pid_ = pid;
    info.uid_ = uid;
    info.reason_ = Memory::ProcPriorityUpdateReason::START_ABILITY;
    int32_t result = ERR_OK;
    {
        HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
        result = Memory::MemMgrClient::GetInstance().NotifyProcessStateChangedAsync(info);
    }
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "notifyPriorityChangedSync error, result:%{public}d", result);
        return false;
    }
    return true;
}

int32_t AppMgrServiceInner::GetAllUIExtensionRootHostPid(pid_t pid, std::vector<pid_t> &hostPids)
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

int32_t AppMgrServiceInner::GetAllUIExtensionProviderPid(pid_t hostPid, std::vector<pid_t> &providerPids)
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

int32_t AppMgrServiceInner::NotifyMemorySizeStateChanged(bool isMemorySizeSufficient)
{
    TAG_LOGI(AAFwkTag::APPMGR, "isMemorySizeSufficient: %{public}d",
        isMemorySizeSufficient);
    bool isMemmgrCall = AAFwk::PermissionVerification::GetInstance()->CheckSpecificSystemAbilityAccessPermission(
        MEMMGR_PROC_NAME);
    bool isSupportCall = OHOS::system::GetBoolParameter(SUPPORT_CALL_NOTIFY_MEMORY_CHANGED, false);
    if (!isMemmgrCall && !isSupportCall) {
        TAG_LOGE(AAFwkTag::APPMGR, "callerToken not %{public}s", MEMMGR_PROC_NAME);
        return ERR_PERMISSION_DENIED;
    }

    if (!isMemorySizeSufficient) {
        auto ret = ExitResidentProcessManager::GetInstance().HandleMemorySizeInSufficent();
        if (ret != ERR_OK) {
            TAG_LOGE(AAFwkTag::APPMGR, "handleMemorySizeInSufficent fail, ret: %{public}d", ret);
        }
        return ret;
    }
    std::vector<ExitResidentProcessInfo> exitProcessInfos;
    auto ret = ExitResidentProcessManager::GetInstance().HandleMemorySizeSufficient(exitProcessInfos);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "HandleMemorySizeSufficient fail, ret: %{public}d", ret);
        return ret;
    }
    auto StartExitKeepAliveProcessTask = [exitProcessInfos, innerServiceWeak = weak_from_this()]() {
        auto innerService = innerServiceWeak.lock();
        CHECK_POINTER_AND_RETURN_LOG(innerService, "get appMgrServiceInner fail");
        std::vector<AppExecFwk::BundleInfo> exitBundleInfos;
        ExitResidentProcessManager::GetInstance().QueryExitBundleInfos(exitProcessInfos, exitBundleInfos);

        innerService->NotifyStartResidentProcess(exitBundleInfos);
        innerService->NotifyStartKeepAliveProcess(exitBundleInfos);
    };
    CHECK_POINTER_AND_RETURN_VALUE(taskHandler_, ERR_NO_INIT);
    taskHandler_->SubmitTask(StartExitKeepAliveProcessTask, "startexitkeepaliveprocess");
    return ERR_OK;
}

bool AppMgrServiceInner::IsMemorySizeSufficient()
{
    return ExitResidentProcessManager::GetInstance().IsMemorySizeSufficient();
}

void AppMgrServiceInner::NotifyAppPreCache(int32_t pid, int32_t userId)
{
    std::lock_guard lock(appStateCallbacksLock_);
    for (const auto &item : appStateCallbacks_) {
        if (item.callback != nullptr) {
            item.callback->NotifyAppPreCache(pid, userId);
        }
    }
}

void AppMgrServiceInner::NotifyStartResidentProcess(std::vector<AppExecFwk::BundleInfo> &bundleInfos)
{
    std::lock_guard lock(appStateCallbacksLock_);
    for (const auto &item : appStateCallbacks_) {
        if (item.callback != nullptr) {
            item.callback->NotifyStartResidentProcess(bundleInfos);
        }
    }
}

void AppMgrServiceInner::NotifyStartKeepAliveProcess(std::vector<AppExecFwk::BundleInfo> &bundleInfos)
{
    std::lock_guard lock(appStateCallbacksLock_);
    for (const auto &item : appStateCallbacks_) {
        if (item.callback != nullptr) {
            item.callback->NotifyStartKeepAliveProcess(bundleInfos);
        }
    }
}

void AppMgrServiceInner::SetKeepAliveEnableState(const std::string &bundleName, bool enable, int32_t uid)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    if (bundleName.empty()) {
        TAG_LOGE(AAFwkTag::APPMGR, "bundle name empty");
        return;
    }

    if (appRunningManager_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "running manager error");
        return;
    }

    auto callerUid = IPCSkeleton::GetCallingUid();
    if (callerUid != FOUNDATION_UID) {
        TAG_LOGE(AAFwkTag::APPMGR, "not foundation call");
        return;
    }

    for (const auto &item : appRunningManager_->GetAppRunningRecordMap()) {
        const auto &appRecord = item.second;
        if (appRecord != nullptr && appRecord->GetBundleName() == bundleName &&
            (uid == 0 || appRecord->GetUid() == uid)) {
            TAG_LOGD(AAFwkTag::APPMGR, "%{public}s update state: %{public}d",
                bundleName.c_str(), static_cast<int32_t>(enable));
            appRecord->SetKeepAliveEnableState(enable);
        }
    }
}

void AppMgrServiceInner::SetKeepAliveDkv(const std::string &bundleName, bool enable, int32_t uid)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    if (bundleName.empty()) {
        TAG_LOGE(AAFwkTag::APPMGR, "bundle name empty");
        return;
    }

    if (appRunningManager_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "running manager error");
        return;
    }

    auto callerUid = IPCSkeleton::GetCallingUid();
    if (callerUid != FOUNDATION_UID) {
        TAG_LOGE(AAFwkTag::APPMGR, "not foundation call");
        return;
    }

    for (const auto &item : appRunningManager_->GetAppRunningRecordMap()) {
        const auto &appRecord = item.second;
        if (appRecord != nullptr && appRecord->GetBundleName() == bundleName &&
            (uid == 0 || appRecord->GetUid() == uid)) {
            TAG_LOGD(AAFwkTag::APPMGR, "%{public}s update state: %{public}d",
                bundleName.c_str(), static_cast<int32_t>(enable));
            appRecord->SetKeepAliveDkv(enable);
        }
    }
}

int32_t AppMgrServiceInner::SetSupportedProcessCacheSelf(bool isSupport)
{
    TAG_LOGI(AAFwkTag::APPMGR, "call");
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
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

    if (!DelayedSingleton<CacheProcessManager>::GetInstance()->QueryEnableProcessCache()) {
        TAG_LOGE(AAFwkTag::APPMGR, "process cache feature is disabled.");
        return AAFwk::ERR_CAPABILITY_NOT_SUPPORT;
    }
    appRecord->SetSupportedProcessCache(isSupport);
    return ERR_OK;
}

int32_t AppMgrServiceInner::SetSupportedProcessCache(int32_t pid, bool isSupport)
{
    TAG_LOGI(AAFwkTag::APPMGR, "called");
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    if (!appRunningManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRunningManager_ is nullptr");
        return ERR_NO_INIT;
    }

    auto appRecord = GetAppRunningRecordByPid(pid);
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "no such appRecord, pid:%{public}d", pid);
        return ERR_INVALID_VALUE;
    }

    if (!DelayedSingleton<CacheProcessManager>::GetInstance()->QueryEnableProcessCache()) {
        TAG_LOGE(AAFwkTag::APPMGR, "process cache feature disabled");
        return AAFwk::ERR_CAPABILITY_NOT_SUPPORT;
    }
    appRecord->SetEnableProcessCache(isSupport);
    return ERR_OK;
}

bool AppMgrServiceInner::IsAppProcessesAllCached(const std::string &bundleName, int32_t uid,
                                                 const std::set<std::shared_ptr<AppRunningRecord>> &cachedSet)
{
    if (!appRunningManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRunningManager_ null");
        return false;
    }
    return appRunningManager_->IsAppProcessesAllCached(bundleName, uid, cachedSet);
}

int32_t AppMgrServiceInner::CheckCallingIsUserTestModeInner(const pid_t pid, bool &isUserTest)
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

bool AppMgrServiceInner::IsSceneBoardCall() {
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

void AppMgrServiceInner::OnAppCacheStateChanged(const std::shared_ptr<AppRunningRecord> &appRecord,
    ApplicationState state)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRecord null");
        return;
    }

    if (appRecord->GetPriorityObject() == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "priorityobject null");
        return;
    }

    TAG_LOGD(AAFwkTag::APPMGR, "OnAppCacheStateChanged begin, bundleName is %{public}s, pid:%{public}d",
        appRecord->GetBundleName().c_str(), appRecord->GetPid());

    DelayedSingleton<AppStateObserverManager>::GetInstance()->OnAppCacheStateChanged(appRecord, state);
}

#ifdef SUPPORT_CHILD_PROCESS
int32_t AppMgrServiceInner::StartNativeChildProcess(const pid_t hostPid, const std::string &libName,
    int32_t childProcessCount, const sptr<IRemoteObject> &callback)
{
    TAG_LOGI(AAFwkTag::APPMGR, "hostPid:%{public}d", hostPid);
    if (hostPid <= 0 || libName.empty() || !callback) {
        TAG_LOGE(AAFwkTag::APPMGR, "invalid param: hostPid:%{public}d libName:%{private}s",
            hostPid, libName.c_str());
        return ERR_INVALID_VALUE;
    }

    int32_t errCode = StartChildProcessPreCheck(hostPid, CHILD_PROCESS_TYPE_NATIVE);
    if (errCode != ERR_OK) {
        return errCode;
    }

    if (UserRecordManager::GetInstance().IsLogoutUser(GetUserIdByUid(IPCSkeleton::GetCallingUid()))) {
        TAG_LOGE(AAFwkTag::APPMGR, "disable start process in logout user");
        return ERR_INVALID_OPERATION;
    }

    auto appRecord = GetAppRunningRecordByPid(hostPid);
    if (!appRecord) {
        TAG_LOGI(AAFwkTag::APPMGR, "get record(hostPid:%{public}d) fail", hostPid);
        return ERR_INVALID_OPERATION;
    }

    if (!AAFwk::AppUtils::GetInstance().IsSupportNativeChildProcess() &&
        !AAFwk::AppUtils::GetInstance().IsAllowNativeChildProcess(appRecord->GetAppIdentifier())) {
        TAG_LOGE(AAFwkTag::APPMGR, "unSupport native child process");
        return AAFwk::ERR_NOT_SUPPORT_NATIVE_CHILD_PROCESS;
    }

    std::lock_guard<std::mutex> lock(childProcessRecordMapMutex_);
    auto childRecordMap = appRecord->GetChildProcessRecordMap();
    auto count = count_if(childRecordMap.begin(), childRecordMap.end(), [] (const auto &pair) -> bool {
        return pair.second->GetChildProcessType() == CHILD_PROCESS_TYPE_NATIVE;
    });

    if (count >= PC_MAX_CHILD_PROCESS_NUM) {
        TAG_LOGI(AAFwkTag::APPMGR, "The number of native child process reached the limit (hostPid:%{public}d)",
            hostPid);
        return ERR_OVERFLOW;
    }

    pid_t dummyChildPid = 0;
    auto nativeChildRecord = ChildProcessRecord::CreateNativeChildProcessRecord(
        hostPid, libName, appRecord, callback, childProcessCount, false);
    ChildProcessArgs args;
    ChildProcessOptions options;
    return StartChildProcessImpl(nativeChildRecord, appRecord, dummyChildPid, args, options);
}
#endif // SUPPORT_CHILD_PROCESS

void AppMgrServiceInner::CacheLoadAbilityTask(const LoadAbilityTaskFunc&& func)
{
    std::lock_guard lock(loadTaskListMutex_);
    loadAbilityTaskFuncList_.emplace_back(std::move(func));
}

void AppMgrServiceInner::SubmitCacheLoadAbilityTask()
{
    std::lock_guard lock(loadTaskListMutex_);
    std::weak_ptr<AAFwk::TaskHandlerWrap> taskHandler = taskHandler_;
    for_each(loadAbilityTaskFuncList_.begin(), loadAbilityTaskFuncList_.end(),
        [taskHandler](LoadAbilityTaskFunc loadAbilityFunc) {
            auto LoadAbilityhandler = taskHandler.lock();
            if (LoadAbilityhandler != nullptr && loadAbilityFunc) {
                LoadAbilityhandler->SubmitTask(loadAbilityFunc, "loadAbilityFunc");
            }
        });
    loadAbilityTaskFuncList_.clear();
}

bool AppMgrServiceInner::GetSceneBoardAttachFlag() const
{
    return sceneBoardAttachFlag_;
}

void AppMgrServiceInner::SetSceneBoardAttachFlag(bool flag)
{
    sceneBoardAttachFlag_ = flag;
}

void AppMgrServiceInner::AttachedToStatusBar(const sptr<IRemoteObject> &token)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    auto appRecord = GetAppRunningRecordByAbilityToken(token);
    if (appRecord == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "abilityRecord null");
        return;
    }
    appRecord->SetAttachedToStatusBar(true);
}

int32_t AppMgrServiceInner::NotifyProcessDependedOnWeb()
{
    int32_t pid = IPCSkeleton::GetCallingPid();
    auto appRecord = GetAppRunningRecordByPid(pid);
    if (appRecord == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "no appRecord");
        return ERR_INVALID_VALUE;
    }
    TAG_LOGD(AAFwkTag::APPMGR, "call");
    appRecord->SetIsDependedOnArkWeb(true);
    return ERR_OK;
}

void AppMgrServiceInner::KillProcessDependedOnWeb()
{
    TAG_LOGI(AAFwkTag::APPMGR, "call");
    CHECK_POINTER_AND_RETURN_LOG(appRunningManager_, "appRunningManager_ is nullptr");
    for (const auto &item : appRunningManager_->GetAppRunningRecordMap()) {
        const auto &appRecord = item.second;
        if (!appRecord || !appRecord->GetSpawned() ||
            !appRecord->GetPriorityObject() || !appRecord->IsDependedOnArkWeb()) {
            continue;
        }

        std::string bundleName = appRecord->GetBundleName();
        pid_t pid = appRecord->GetPid();
        if (appRecord->IsKeepAliveApp()) {
            ExitResidentProcessManager::GetInstance().RecordExitResidentBundleDependedOnWeb(bundleName,
                appRecord->GetUid());
        }
        KillProcessByPid(pid, "KillProcessDependedOnWeb");
    }
}

void AppMgrServiceInner::RestartResidentProcessDependedOnWeb()
{
    TAG_LOGD(AAFwkTag::APPMGR, "call");
    std::vector<ExitResidentProcessInfo> bundleNames;
    ExitResidentProcessManager::GetInstance().HandleExitResidentBundleDependedOnWeb(bundleNames);
    if (bundleNames.empty()) {
        TAG_LOGE(AAFwkTag::APPMGR, "exit resident bundle names empty");
        return;
    }

    auto RestartResidentProcessDependedOnWebTask = [bundleNames, innerServiceWeak = weak_from_this()]() {
        auto innerService = innerServiceWeak.lock();
        CHECK_POINTER_AND_RETURN_LOG(innerService, "get AppMgrServiceInner failed");
        std::vector<AppExecFwk::BundleInfo> exitBundleInfos;
        ExitResidentProcessManager::GetInstance().QueryExitBundleInfos(bundleNames, exitBundleInfos);

        innerService->NotifyStartResidentProcess(exitBundleInfos);
        innerService->NotifyStartKeepAliveProcess(exitBundleInfos);
    };
    CHECK_POINTER_AND_RETURN_LOG(taskHandler_, "taskHandler_ null");
    taskHandler_->SubmitTask(RestartResidentProcessDependedOnWebTask, "RestartResidentProcessDependedOnWeb");
}

void AppMgrServiceInner::BlockProcessCacheByPids(const std::vector<int32_t>& pids)
{
    for (const auto& pid : pids) {
        auto appRecord = GetAppRunningRecordByPid(pid);
        if (appRecord == nullptr) {
            TAG_LOGE(AAFwkTag::APPMGR, "appRecord null");
            continue;
        }
        appRecord->SetProcessCacheBlocked(true);
        DelayedSingleton<CacheProcessManager>::GetInstance()->OnAppProcessCacheBlocked(appRecord);
    }
}

bool AppMgrServiceInner::CleanAbilityByUserRequest(const sptr<IRemoteObject> &token)
{
    TAG_LOGD(AAFwkTag::APPMGR, "call");
    if (!token) {
        TAG_LOGE(AAFwkTag::APPMGR, "token invalid");
        return false;
    }

    if (!appRunningManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRunningManager_ invalid");
        return false;
    }

    pid_t targetPid = 0;
    int32_t targetUid = 0;
    if (!appRunningManager_->HandleUserRequestClean(token, targetPid, targetUid)) {
        TAG_LOGW(AAFwkTag::APPMGR, "can not clean process");
        return false;
    }

    if (targetPid <= 0 || targetUid <= 0) {
        TAG_LOGE(AAFwkTag::APPMGR, "get pid or uid invalid, pid:%{public}d, uid:%{public}d", targetPid, targetUid);
        return false;
    }
    TAG_LOGI(AAFwkTag::APPMGR, "clean ability set up bg, force kill pid:%{public}d", targetPid);
    willKillPidsNum_ += 1;
    int32_t delayTime = willKillPidsNum_ * KILL_PROCESS_BY_USER_INTERVAL + KILL_PROCESS_BY_USER_DELAY_BASE;
    TAG_LOGD(AAFwkTag::APPMGR, "delayTime:%{public}d", delayTime);
    auto delayKillTask = [targetPid, innerServiceWeak = weak_from_this()]() {
        auto self = innerServiceWeak.lock();
        CHECK_POINTER_AND_RETURN_LOG(self, "get AppMgrServiceInner failed");
        self->KillProcessByPid(targetPid, KILL_REASON_USER_REQUEST);
        self->DecreaseWillKillPidsNum();
        TAG_LOGD(AAFwkTag::APPMGR, "pid:%{public}d killed", targetPid);
    };
    delayKillTaskHandler_->SubmitTaskJust(delayKillTask, "delayKillUIAbility", delayTime);

    return true;
}

void AppMgrServiceInner::CheckCleanAbilityByUserRequest(const std::shared_ptr<AppRunningRecord> &appRecord,
    const std::shared_ptr<AbilityRunningRecord> &abilityRecord, const AbilityState state)
{
    if (!appRecord || !abilityRecord) {
        return;
    }

    if (state != AbilityState::ABILITY_STATE_BACKGROUND) {
        return;
    }

    if (abilityRecord->GetAbilityInfo() && abilityRecord->GetAbilityInfo()->type != AppExecFwk::AbilityType::PAGE) {
        return;
    }

    if (appRecord->IsKeepAliveApp()) {
        return;
    }

    if (!appRecord->IsAllAbilityReadyToCleanedByUserRequest()) {
        TAG_LOGD(AAFwkTag::APPMGR,
            "not ready to clean when user request. bundleName:%{public}s", appRecord->GetBundleName().c_str());
        return;
    }
    appRecord->SetUserRequestCleaning();

    pid_t pid = 0;
    if (appRecord->GetPriorityObject()) {
        pid = appRecord->GetPid();
    }
    TAG_LOGI(AAFwkTag::APPMGR, "clean ability set up bg, force kill, pid:%{public}d", pid);
    KillProcessByPid(pid, KILL_REASON_USER_REQUEST);
}

bool AppMgrServiceInner::IsKilledForUpgradeWeb(const std::string &bundleName) const
{
    auto callerUid = IPCSkeleton::GetCallingUid();
    if (callerUid != FOUNDATION_UID) {
        TAG_LOGE(AAFwkTag::APPMGR, "not foundation call");
        return false;
    }
    return ExitResidentProcessManager::GetInstance().IsKilledForUpgradeWeb(bundleName);
}

int32_t AppMgrServiceInner::GetPidsByAccessTokenId(const uint32_t accessTokenId, std::vector<pid_t> &pids)
{
    if (!appRunningManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRunningManager_ null");
        return ERR_NO_INIT;
    }
    std::vector<pid_t> foregroundPids;
    for (const auto &item : appRunningManager_->GetAppRunningRecordMap()) {
        const auto &appRecord = item.second;
        if (!appRecord || !appRecord->GetSpawned()) {
            continue;
        }
        auto applicationInfo = appRecord->GetApplicationInfo();
        if (!applicationInfo) {
            continue;
        }
        if (accessTokenId == applicationInfo->accessTokenId) {
            pid_t curPid = appRecord->GetPid();
            if (appRecord->GetState() == ApplicationState::APP_STATE_FOREGROUND) {
                foregroundPids.push_back(curPid);
                continue;
            }
            pids.push_back(curPid);
        }
    }
    for (pid_t foregroundPid : foregroundPids) {
        pids.push_back(foregroundPid);
    }
    return ERR_OK;
}

bool AppMgrServiceInner::IsProcessContainsOnlyUIAbility(const pid_t pid)
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

void AppMgrServiceInner::MakeIsolateSandBoxProcessName(const std::shared_ptr<AbilityInfo> &abilityInfo,
    const HapModuleInfo &hapModuleInfo, std::string &processName) const
{
    if (abilityInfo == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "abilityInfo nullptr");
        return;
    }
    auto type = abilityInfo->type;
    auto extensionType = abilityInfo->extensionAbilityType;
    if (type != AppExecFwk::AbilityType::EXTENSION ||
        extensionType == AppExecFwk::ExtensionAbilityType::DATASHARE ||
        extensionType == AppExecFwk::ExtensionAbilityType::SERVICE) {
        return;
    }
    for (const auto& extensionInfo: hapModuleInfo.extensionInfos) {
        if (extensionInfo.name == abilityInfo->name) {
            if (extensionInfo.needCreateSandbox) {
                processName = (processName + ":" + abilityInfo->name);
            }
            return;
        }
    }
}

bool AppMgrServiceInner::IsProcessAttached(sptr<IRemoteObject> token) const
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

bool AppMgrServiceInner::IsCallerKilling(const std::string& callerKey) const
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    if (IPCSkeleton::GetCallingUid() != FOUNDATION_UID) {
        TAG_LOGE(AAFwkTag::APPMGR, "Not foundation call.");
        return false;
    }
    return KillingProcessManager::GetInstance().IsCallerKilling(callerKey);
}

int32_t AppMgrServiceInner::GetSupportedProcessCachePids(const std::string &bundleName,
    std::vector<int32_t> &pidList)
{
    auto cachePrcoMgr = DelayedSingleton<CacheProcessManager>::GetInstance();
    auto osAccountMgr = DelayedSingleton<OsAccountManagerWrapper>::GetInstance();
    if (cachePrcoMgr == nullptr || osAccountMgr == nullptr || appRunningManager_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "inner manager null");
        return AAFwk::INNER_ERR;
    }
    pidList.clear();
    int32_t callderUserId = -1;
    int32_t getOsAccountRet = osAccountMgr->GetOsAccountLocalIdFromUid(IPCSkeleton::GetCallingUid(), callderUserId);
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
        int32_t procGetOsAccountRet = osAccountMgr->GetOsAccountLocalIdFromUid(appRecord->GetUid(), procUserId);
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

int AppMgrServiceInner::RegisterKiaInterceptor(const sptr<IKiaInterceptor> &interceptor)
{
    TAG_LOGI(AAFwkTag::APPMGR, "call");
    if (!AAFwk::AppUtils::GetInstance().IsStartOptionsWithAnimation() ||
        !AAFwk::PermissionVerification::GetInstance()->VerifySuperviseKiaServicePermission()) {
        TAG_LOGE(AAFwkTag::APPMGR, "no kia permission.");
        return ERR_PERMISSION_DENIED;
    }
    if (interceptor == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "interceptor is nullptr.");
        return ERR_INVALID_VALUE;
    }
    kiaInterceptor_ = interceptor;
    return ERR_OK;
}

int32_t AppMgrServiceInner::CheckIsKiaProcess(pid_t pid, bool &isKia)
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

void AppMgrServiceInner::UpdateInstanceKeyBySpecifiedId(int32_t specifiedId, std::string &instanceKey)
{
    if (!appRunningManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRunningManager_ is nullptr");
        return;
    }
    appRunningManager_->UpdateInstanceKeyBySpecifiedId(specifiedId, instanceKey);
}

void AppMgrServiceInner::SendAppSpawnUninstallDebugHapMsg(int32_t userId)
{
    if (remoteClientManager_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null remoteClientManager_");
        return;
    }
    auto spawnClient = remoteClientManager_->GetSpawnClient();
    if (spawnClient == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null spawnClient");
        return;
    }
    auto errCode = spawnClient->SendAppSpawnUninstallDebugHapMsg(userId);
    if (FAILED(errCode)) {
        TAG_LOGE(AAFwkTag::APPMGR, "SendAppSpawnUninstallDebugHapMsg failed, errCode %{public}08x", errCode);
    }
}

bool AppMgrServiceInner::IsSpecifiedModuleLoaded(const AAFwk::Want &want, const AbilityInfo &abilityInfo)
{
    if (!CheckRemoteClient() || !appRunningManager_) {
        return false;
    }
    auto appInfo = std::make_shared<ApplicationInfo>(abilityInfo.applicationInfo);
    int32_t appIndex = 0;
    (void)AbilityRuntime::StartupUtil::GetAppIndex(want, appIndex);
    BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    if (!GetBundleAndHapInfo(abilityInfo, appInfo, bundleInfo, hapModuleInfo, appIndex)) {
        return false;
    }

    auto abilityInfoPtr = std::make_shared<AbilityInfo>(abilityInfo);
    std::string processName;
    MakeProcessName(abilityInfoPtr, appInfo, hapModuleInfo, appIndex, "", processName, false);
    auto instanceKey = want.GetStringParam(Want::APP_INSTANCE_KEY);
    auto customProcessFlag = abilityInfo.process;
    auto appRecord = appRunningManager_->CheckAppRunningRecordIsExist(appInfo->name,
        processName, appInfo->uid, bundleInfo, "", nullptr, instanceKey, customProcessFlag);
    if (appRecord == nullptr) {
        return false;
    }

    auto moduleRecord = appRecord->GetModuleRecordByModuleName(appInfo->bundleName, hapModuleInfo.moduleName);
    if (moduleRecord == nullptr) {
        return false;
    }
    TAG_LOGD(AAFwkTag::APPMGR, "IsSpecifiedModuleLoaded state %{public}d", moduleRecord->GetModuleRecordState());
    return moduleRecord->IsLoaded();
}

int32_t AppMgrServiceInner::GetKilledProcessInfo(int pid, int uid, KilledProcessInfo &info)
{
    if (!appRunningManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRunningManager null");
        return AAFwk::ERR_NULL_APP_RUNNING_MANAGER;
    }
    auto appRecord = appRunningManager_->QueryAppRecordPlus(pid, uid);
    if (appRecord == nullptr) {
        TAG_LOGI(AAFwkTag::APPMGR, "appRecord not found");
        return AAFwk::ERR_NO_APP_RECORD;
    }
    info.pid = pid;
    info.uid = uid;
    auto appInfo = appRecord->GetApplicationInfo();
    if (appInfo) {
        info.accessTokenId = appInfo->accessTokenId;
    } else {
        TAG_LOGW(AAFwkTag::APPMGR, "appInfo null");
    }
    info.bundleName = appRecord->GetBundleName();
    info.processName = appRecord->GetProcessName();
    return ERR_OK;
}

void AppMgrServiceInner::AddUIExtensionBindItem(
    std::shared_ptr<AAFwk::Want> want, std::shared_ptr<AppRunningRecord> appRecord, sptr<IRemoteObject> token)
{
    if (want == nullptr || appRecord == nullptr || token == nullptr || appRunningManager_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "invalid input params");
        return;
    }

    auto notifyProcessBind = want->GetIntParam(UIEXTENSION_NOTIFY_BIND, -1);
    if (notifyProcessBind != 1) {
        TAG_LOGE(AAFwkTag::APPMGR, "no bind permission");
        return;
    }

    UIExtensionProcessBindInfo bindInfo;
    if (!WarpBindInfo(want, appRecord, bindInfo)) {
        TAG_LOGE(AAFwkTag::APPMGR, "warp bindInfo fail");
        return;
    }
    auto uiExtensionBindAbilityId = want->GetIntParam(UIEXTENSION_BIND_ABILITY_ID, -1);
    appRunningManager_->AddUIExtensionBindItem(uiExtensionBindAbilityId, bindInfo);
    BindUIExtensionProcess(appRecord, bindInfo);
    want->RemoveParam(UIEXTENSION_HOST_PID);
    want->RemoveParam(UIEXTENSION_HOST_UID);
    want->RemoveParam(UIEXTENSION_HOST_BUNDLENAME);
    want->RemoveParam(UIEXTENSION_NOTIFY_BIND);
}

void AppMgrServiceInner::RemoveUIExtensionBindItem(
    std::shared_ptr<AppRunningRecord> appRecord, sptr<IRemoteObject> token)
{
    if (appRecord == nullptr || token == nullptr || appRunningManager_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "invalid input params");
        return;
    }

    auto abilityRunningRecord = appRecord->GetAbilityRunningRecordByToken(token);
    if (abilityRunningRecord == nullptr) {
        TAG_LOGW(AAFwkTag::APPMGR, "invalid ability");
        return;
    }

    auto abilityInfo = abilityRunningRecord->GetAbilityInfo();
    if (abilityInfo == nullptr) {
        TAG_LOGW(AAFwkTag::APPMGR, "invalid ability");
        return;
    }

    if (!AAFwk::UIExtensionUtils::IsUIExtension(abilityInfo->extensionAbilityType)) {
        TAG_LOGE(AAFwkTag::APPMGR, "abilityType not match");
        return;
    }

    auto uiExtensionBindAbilityId = abilityRunningRecord->GetUIExtensionBindAbilityId();
    UIExtensionProcessBindInfo bindInfo;
    auto result = appRunningManager_->QueryUIExtensionBindItemById(uiExtensionBindAbilityId, bindInfo);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "bindInfo not exist");
        return;
    }
    if (bindInfo.notifyProcessBind != 1) {
        TAG_LOGE(AAFwkTag::APPMGR, "no unbind permission");
        return;
    }
    UnBindUIExtensionProcess(appRecord, bindInfo);
    appRunningManager_->RemoveUIExtensionBindItemById(uiExtensionBindAbilityId);
}

void AppMgrServiceInner::BindUIExtensionProcess(
    const std::shared_ptr<AppRunningRecord> &appRecord, const UIExtensionProcessBindInfo &bindInfo)
{
    std::lock_guard guard(uiExtensionBindReleationsLock_);
    auto pid = bindInfo.pid;
    auto callerPid = bindInfo.callerPid;
    auto it = uiExtensionBindReleations_.find(pid);

    if (it == uiExtensionBindReleations_.end() || it->second.find(callerPid) == it->second.end()) {
        uiExtensionBindReleations_[pid][callerPid] = 1;
        DelayedSingleton<AppStateObserverManager>::GetInstance()->OnProcessBindingRelationChanged(
            appRecord, bindInfo, 1);
    } else {
        it->second[callerPid]++;
    }
}

void AppMgrServiceInner::UnBindUIExtensionProcess(
    const std::shared_ptr<AppRunningRecord> &appRecord, const UIExtensionProcessBindInfo &bindInfo)
{
    std::lock_guard guard(uiExtensionBindReleationsLock_);
    auto pid = bindInfo.pid;
    auto callerPid = bindInfo.callerPid;
    auto it = uiExtensionBindReleations_.find(pid);

    if (it != uiExtensionBindReleations_.end()) {
        auto &innerMap = it->second;
        auto innerIt = innerMap.find(callerPid);
        if (innerIt != innerMap.end()) {
            innerIt->second--;
            if (innerIt->second == 0) {
                DelayedSingleton<AppStateObserverManager>::GetInstance()->OnProcessBindingRelationChanged(
                    appRecord, bindInfo, 0);
                innerMap.erase(innerIt);
            }
        }
        if (innerMap.empty()) {
            uiExtensionBindReleations_.erase(it);
        }
    }
}

bool AppMgrServiceInner::WarpBindInfo(std::shared_ptr<AAFwk::Want> &want, std::shared_ptr<AppRunningRecord> &appRecord,
    UIExtensionProcessBindInfo &bindInfo)
{
    auto notifyProcessBind = want->GetIntParam(UIEXTENSION_NOTIFY_BIND, -1);
    auto uiExtensionBindAbilityId = want->GetIntParam(UIEXTENSION_BIND_ABILITY_ID, -1);
    auto callerPid = want->GetIntParam(UIEXTENSION_HOST_PID, -1);
    auto callerUid = want->GetIntParam(UIEXTENSION_HOST_UID, -1);
    auto callerBundleName = want->GetStringParam(UIEXTENSION_HOST_BUNDLENAME);
    pid_t providerPid = -1;
    pid_t providerUid = -1;
    if (appRecord->GetPriorityObject() != nullptr) {
        providerPid = appRecord->GetPid();
        providerUid = appRecord->GetUid();
    }
    TAG_LOGI(AAFwkTag::APPMGR,
        "uiExtensionBindAbilityId: %{public}d, providerUid: "
        "%{public}d,providerPid: %{public}d,callerUid: %{public}d, "
        "callerPid: %{public}d,callerBundleName: %{public}s",
        uiExtensionBindAbilityId,
        providerUid,
        providerPid,
        callerUid,
        callerPid,
        callerBundleName.c_str());
    if (uiExtensionBindAbilityId == -1 || providerPid == -1 || providerUid == -1 || callerPid == -1 ||
        callerUid == -1 || callerBundleName.empty()) {
        TAG_LOGE(AAFwkTag::APPMGR, "invalid want params");
        return false;
    }
    bindInfo.pid = providerPid;
    bindInfo.uid = providerUid;
    bindInfo.callerPid = callerPid;
    bindInfo.callerUid = callerUid;
    bindInfo.callerBundleName = callerBundleName;
    bindInfo.notifyProcessBind = notifyProcessBind;
    bindInfo.isKeepAlive = appRecord->IsKeepAliveApp();
    bindInfo.extensionType = appRecord->GetExtensionType();
    bindInfo.processType = appRecord->GetProcessType();
    return true;
}
} // namespace AppExecFwk
}  // namespace OHOS
