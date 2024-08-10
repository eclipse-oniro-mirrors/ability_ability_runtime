/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "ability_manager_service.h"

#include <atomic>
#include <chrono>
#include <csignal>
#include <cstdlib>
#include <fstream>
#include <functional>
#include <getopt.h>
#include <memory>
#include <mutex>
#include <nlohmann/json.hpp>
#include <string>
#include <thread>
#include <unistd.h>
#include <unordered_set>

#include "ability_background_connection.h"
#include "ability_connect_manager.h"
#include "ability_debug_deal.h"
#include "ability_manager_constants.h"
#include "ability_manager_errors.h"
#include "ability_manager_radar.h"
#include "ability_resident_process_rdb.h"
#include "ability_util.h"
#include "accesstoken_kit.h"
#ifdef APP_DOMAIN_VERIFY_ENABLED
#include "ag_convert_callback_impl.h"
#include "app_domain_verify_mgr_client.h"
#endif
#include "app_utils.h"
#include "app_exit_reason_data_manager.h"
#include "app_recovery/default_recovery_config.h"
#include "application_util.h"
#include "recovery_info_timer.h"
#include "assert_fault_callback_death_mgr.h"
#include "assert_fault_proxy.h"
#include "bundle_mgr_client.h"
#include "common_event_manager.h"
#include "common_event_support.h"
#include "connection_state_manager.h"
#include "display_manager.h"
#include "distributed_client.h"
#ifdef WITH_DLP
#include "dlp_utils.h"
#endif // WITH_DLP
#include "errors.h"
#include "extension_config.h"
#include "freeze_util.h"
#include "global_constant.h"
#include "hilog_tag_wrapper.h"
#include "hisysevent.h"
#include "hitrace_meter.h"
#include "if_system_ability_manager.h"
#include "in_process_call_wrapper.h"
#include "insight_intent_execute_callback_proxy.h"
#include "insight_intent_execute_manager.h"
#include "int_wrapper.h"
#include "interceptor/ability_jump_interceptor.h"
#include "interceptor/control_interceptor.h"
#include "interceptor/crowd_test_interceptor.h"
#include "interceptor/disposed_rule_interceptor.h"
#include "interceptor/ecological_rule_interceptor.h"
#include "interceptor/extension_control_interceptor.h"
#include "interceptor/screen_unlock_interceptor.h"
#include "interceptor/start_other_app_interceptor.h"
#include "ipc_skeleton.h"
#include "ipc_types.h"
#include "iservice_registry.h"
#include "itest_observer.h"
#include "mission_info.h"
#include "mission_info_mgr.h"
#include "mock_session_manager_service.h"
#include "modal_system_ui_extension.h"
#include "os_account_manager_wrapper.h"
#include "parameters.h"
#include "permission_constants.h"
#include "process_options.h"
#include "recovery_param.h"
#include "res_sched_util.h"
#include "restart_app_manager.h"
#include "sa_mgr_client.h"
#include "scene_board_judgement.h"
#include "server_constant.h"
#include "session_info.h"
#include "softbus_bus_center.h"
#include "start_ability_handler/start_ability_sandbox_savefile.h"
#include "start_options.h"
#include "start_ability_utils.h"
#include "startup_util.h"
#include "status_bar_delegate_interface.h"
#include "string_ex.h"
#include "string_wrapper.h"
#include "system_ability_definition.h"
#include "system_ability_token_callback.h"
#include "extension_record_manager.h"
#include "ui_extension_utils.h"
#include "ui_service_extension_connection_constants.h"
#include "unlock_screen_manager.h"
#include "uri_permission_manager_client.h"
#include "uri_utils.h"
#include "view_data.h"
#include "xcollie/watchdog.h"
#include "config_policy_utils.h"
#include "running_multi_info.h"
#include "utils/ability_permission_util.h"
#include "utils/dump_utils.h"
#include "utils/extension_permissions_util.h"
#include "utils/window_options_utils.h"
#ifdef SUPPORT_GRAPHICS
#include "dialog_session_manager.h"
#include "application_anr_listener.h"
#include "input_manager.h"
#include "ability_first_frame_state_observer_manager.h"
#include "window_focus_changed_listener.h"
#endif

using OHOS::AppExecFwk::ElementName;
using OHOS::Security::AccessToken::AccessTokenKit;

namespace OHOS {
using AbilityRuntime::FreezeUtil;
namespace AAFwk {
using AutoStartupInfo = AbilityRuntime::AutoStartupInfo;
namespace {
#define CHECK_CALLER_IS_SYSTEM_APP                                                             \
    if (!AAFwk::PermissionVerification::GetInstance()->JudgeCallerIsAllowedToUseSystemAPI()) { \
        TAG_LOGE(AAFwkTag::ABILITYMGR,                                                         \
        "The caller is not system-app, can not use system-api");                               \
        return ERR_NOT_SYSTEM_APP;                                                             \
    }

constexpr const char* ARGS_USER_ID = "-u";
constexpr const char* ARGS_CLIENT = "-c";
constexpr const char* ILLEGAL_INFOMATION = "The arguments are illegal and you can enter '-h' for help.";


constexpr int32_t NEW_RULE_VALUE_SIZE = 6;
constexpr int32_t APP_ALIVE_TIME_MS = 1000;  // Allow background startup within 1 second after application startup
constexpr int32_t REGISTER_FOCUS_DELAY = 5000;
constexpr size_t OFFSET = 32;
constexpr const char* IS_DELEGATOR_CALL = "isDelegatorCall";

// Startup rule switch
constexpr const char* COMPONENT_STARTUP_NEW_RULES = "component.startup.newRules";
constexpr const char* NEW_RULES_EXCEPT_LAUNCHER_SYSTEMUI = "component.startup.newRules.except.LauncherSystemUI";
constexpr const char* BACKGROUND_JUDGE_FLAG = "component.startup.backgroundJudge.flag";
constexpr const char* WHITE_LIST_ASS_WAKEUP_FLAG = "component.startup.whitelist.associatedWakeUp";

// White list app
constexpr const char* BUNDLE_NAME_SETTINGSDATA = "com.ohos.settingsdata";
// Support prepare terminate
constexpr int32_t PREPARE_TERMINATE_ENABLE_SIZE = 6;
constexpr const char* PREPARE_TERMINATE_ENABLE_PARAMETER = "persist.sys.prepare_terminate";
// UIExtension type
constexpr const char* UIEXTENSION_TYPE_KEY = "ability.want.params.uiExtensionType";
constexpr const char* UIEXTENSION_TARGET_TYPE_KEY = "ability.want.params.uiExtensionTargetType";
constexpr const char* SYSTEM_SHARE = "share";
constexpr const char* SYSTEM_SHARE_TYPE = "sysPicker/share";
// Share picker params
constexpr char SHARE_PICKER_DIALOG_BUNDLE_NAME_KEY[] = "const.system.sharePicker.bundleName";
constexpr char SHARE_PICKER_DIALOG_ABILITY_NAME_KEY[] = "const.system.sharePicker.abilityName";
constexpr char SHARE_PICKER_UIEXTENSION_NAME_KEY[] = "const.system.sharePicker.UIExtensionAbilityName";
constexpr char SHARE_PICKER_DIALOG_DEFAULY_BUNDLE_NAME[] = "com.ohos.sharepickerdialog";
constexpr char SHARE_PICKER_DIALOG_DEFAULY_ABILITY_NAME[] = "PickerDialog";
constexpr char TOKEN_KEY[] = "ohos.ability.params.token";
// Developer mode param
constexpr char DEVELOPER_MODE_STATE[] = "const.security.developermode.state";
// Broker params key
constexpr const char* KEY_VISIBLE_ID = "ohos.anco.param.visible";
constexpr const char* START_ABILITY_TYPE = "ABILITY_INNER_START_WITH_ACCOUNT";
constexpr const char* SHELL_ASSISTANT_BUNDLENAME = "com.huawei.shell_assistant";
constexpr const char* SHELL_ASSISTANT_ABILITYNAME = "MainAbility";
constexpr const char* BUNDLE_NAME_DIALOG = "com.ohos.amsdialog";
constexpr const char* STR_PHONE = "phone";
constexpr const char* PARAM_RESV_ANCO_CALLER_UID = "ohos.anco.param.callerUid";
constexpr const char* PARAM_RESV_ANCO_CALLER_BUNDLENAME = "ohos.anco.param.callerBundleName";
// Distributed continued session Id
constexpr const char* DMS_CONTINUED_SESSION_ID = "ohos.dms.continueSessionId";
constexpr const char* DMS_PERSISTENT_ID = "ohos.dms.persistentId";

constexpr const char* DEBUG_APP = "debugApp";
constexpr const char* AUTO_FILL_PASSWORD_TPYE = "autoFill/password";
constexpr const char* AUTO_FILL_SMART_TPYE = "autoFill/smart";
constexpr size_t INDEX_ZERO = 0;
constexpr size_t INDEX_ONE = 1;
constexpr size_t INDEX_TWO = 2;
constexpr size_t ARGC_THREE = 3;
constexpr static char WANT_PARAMS_VIEW_DATA_KEY[] = "ohos.ability.params.viewData";
constexpr const char* WANT_PARAMS_HOST_WINDOW_ID_KEY = "ohos.extra.param.key.hostwindowid";

constexpr int32_t FOUNDATION_UID = 5523;
constexpr const char* FRS_BUNDLE_NAME = "com.ohos.formrenderservice";
constexpr const char* FOUNDATION_PROCESS_NAME = "foundation";
constexpr const char* RSS_PROCESS_NAME = "resource_schedule_service";
constexpr const char* IS_PRELOAD_UIEXTENSION_ABILITY = "ability.want.params.is_preload_uiextension_ability";
constexpr const char* UIEXTENSION_MODAL_TYPE = "ability.want.params.modalType";
constexpr const char* SUPPORT_CLOSE_ON_BLUR = "supportCloseOnBlur";
constexpr const char* ATOMIC_SERVICE_PREFIX = "com.atomicservice.";
constexpr const char* PARAM_SPECIFIED_PROCESS_FLAG = "ohosSpecifiedProcessFlag";
constexpr const char* VERIFY_DOMINATE_SCREEN = "persist.sys.abilityms.verify_start_ability_without_caller_token";

constexpr char ASSERT_FAULT_DETAIL[] = "assertFaultDialogDetail";
constexpr char PRODUCT_ASSERT_FAULT_DIALOG_ENABLED[] = "persisit.sys.abilityms.support_assert_fault_dialog";
const std::unordered_set<std::string> WHITE_LIST_ASS_WAKEUP_SET = { BUNDLE_NAME_SETTINGSDATA };
const std::unordered_set<std::string> COMMON_PICKER_TYPE = {
    "share", "action"
};
std::atomic<bool> g_isDmsAlive = false;

void SendAbilityEvent(const EventName &eventName, HiSysEventType type, const EventInfo &eventInfo)
{
    auto taskHandler = DelayedSingleton<AbilityManagerService>::GetInstance()->GetTaskHandler();
    if (taskHandler == nullptr) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "task handler null.");
        return;
    }
    taskHandler->SubmitTask([eventName, type, eventInfo]() {
        EventReport::SendAbilityEvent(eventName, type, eventInfo);
        });
}
} // namespace

using namespace std::chrono;
using namespace std::chrono_literals;
#ifdef BGTASKMGR_CONTINUOUS_TASK_ENABLE
using namespace BackgroundTaskMgr;
#endif
const bool CONCURRENCY_MODE_FALSE = false;
constexpr int32_t MAIN_USER_ID = 100;
constexpr auto DATA_ABILITY_START_TIMEOUT = 5s;
constexpr int32_t NON_ANONYMIZE_LENGTH = 6;
constexpr uint32_t SCENE_FLAG_NORMAL = 0;
constexpr int32_t MAX_NUMBER_OF_DISTRIBUTED_MISSIONS = 20;
constexpr int32_t SWITCH_ACCOUNT_TRY = 3;
#ifdef ABILITY_COMMAND_FOR_TEST
constexpr int32_t BLOCK_AMS_SERVICE_TIME = 65;
#endif
constexpr int32_t CONVERT_CALLBACK_TIMEOUT_SECONDS = 2; // 2s
constexpr const char* EMPTY_DEVICE_ID = "";
constexpr int32_t APP_MEMORY_SIZE = 512;
constexpr int32_t GET_PARAMETER_INCORRECT = -9;
constexpr int32_t GET_PARAMETER_OTHER = -1;
constexpr int32_t SIZE_10 = 10;
constexpr int32_t HIDUMPER_SERVICE_UID = 1212;
constexpr int32_t ACCOUNT_MGR_SERVICE_UID = 3058;
constexpr int32_t BROKER_UID = 5557;
constexpr int32_t BROKER_RESERVE_UID = 5005;
constexpr int32_t DMS_UID = 5522;
constexpr int32_t PREPARE_TERMINATE_TIMEOUT_MULTIPLE = 10;
constexpr int32_t BOOTEVENT_COMPLETED_DELAY_TIME = 1000;
constexpr int32_t BOOTEVENT_BOOT_ANIMATION_READY_SIZE = 6;
constexpr const char* BUNDLE_NAME_KEY = "bundleName";
constexpr const char* DM_PKG_NAME = "ohos.distributedhardware.devicemanager";
constexpr const char* ACTION_CHOOSE = "ohos.want.action.select";
constexpr const char* HIGHEST_PRIORITY_ABILITY_ENTITY = "flag.home.intent.from.system";
constexpr const char* DMS_API_VERSION = "dmsApiVersion";
constexpr const char* DMS_IS_CALLER_BACKGROUND = "dmsIsCallerBackGround";
constexpr const char* DMS_PROCESS_NAME = "distributedsched";
constexpr const char* DMS_MISSION_ID = "dmsMissionId";
constexpr const char* BOOTEVENT_APPFWK_READY = "bootevent.appfwk.ready";
constexpr const char* BOOTEVENT_BOOT_COMPLETED = "bootevent.boot.completed";
constexpr const char* BOOTEVENT_BOOT_ANIMATION_STARTED = "bootevent.bootanimation.started";
constexpr const char* BOOTEVENT_BOOT_ANIMATION_READY = "bootevent.bootanimation.ready";
constexpr const char* NEED_STARTINGWINDOW = "ohos.ability.NeedStartingWindow";
constexpr const char* PERMISSIONMGR_BUNDLE_NAME = "com.ohos.permissionmanager";
constexpr const char* PERMISSIONMGR_ABILITY_NAME = "com.ohos.permissionmanager.GrantAbility";
constexpr const char* IS_CALL_BY_SCB = "isCallBySCB";
constexpr const char* SPECIFY_TOKEN_ID = "specifyTokenId";
constexpr const char* PROCESS_SUFFIX = "embeddable";
constexpr int32_t DEFAULT_DMS_MISSION_ID = -1;
constexpr const char* PARAM_PREVENT_STARTABILITY = "persist.sys.abilityms.prevent_startability";
constexpr const char* SUSPEND_SERVICE_CONFIG_FILE = "/etc/efficiency_manager/prevent_startability_whitelist.json";
constexpr int32_t MAX_BUFFER = 2048;
nlohmann::json whiteListJsonObj;
constexpr int32_t API12 = 12;
constexpr int32_t API_VERSION_MOD = 100;
constexpr const char* WHITE_LIST = "white_list";

const bool REGISTER_RESULT =
    SystemAbility::MakeAndRegisterAbility(DelayedSingleton<AbilityManagerService>::GetInstance().get());
sptr<AbilityManagerService> AbilityManagerService::instance_;

AbilityManagerService::AbilityManagerService()
    : SystemAbility(ABILITY_MGR_SERVICE_ID, true),
      state_(ServiceRunningState::STATE_NOT_START),
      bundleMgrHelper_(nullptr)
{}

AbilityManagerService::~AbilityManagerService()
{}

std::shared_ptr<AbilityManagerService> AbilityManagerService::GetPubInstance()
{
    return DelayedSingleton<AbilityManagerService>::GetInstance();
}

void AbilityManagerService::OnStart()
{
    if (state_ == ServiceRunningState::STATE_RUNNING) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "Ability manager service has already started.");
        return;
    }
    TAG_LOGI(AAFwkTag::ABILITYMGR, "Ability manager service starting.");
    if (!Init()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Failed to init ability manager service.");
        return;
    }
    state_ = ServiceRunningState::STATE_RUNNING;
    /* Publish service maybe failed, so we need call this function at the last,
     * so it can't affect the TDD test program */
    instance_ = DelayedSingleton<AbilityManagerService>::GetInstance().get();
    if (instance_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Ability manager service enter OnStart, but instance_ is nullptr!");
        return;
    }
    bool ret = Publish(instance_);
    if (!ret) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Publish ability manager service failed!");
        return;
    }

    SetParameter(BOOTEVENT_APPFWK_READY, "true");
    AddSystemAbilityListener(BACKGROUND_TASK_MANAGER_SERVICE_ID);
    AddSystemAbilityListener(DISTRIBUTED_SCHED_SA_ID);
    AddSystemAbilityListener(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
#ifdef SUPPORT_SCREEN
    AddSystemAbilityListener(MULTIMODAL_INPUT_SERVICE_ID);
#endif
    TAG_LOGI(AAFwkTag::ABILITYMGR, "Ability manager service start success.");
}

bool AbilityManagerService::Init()
{
    HiviewDFX::Watchdog::GetInstance().InitFfrtWatchdog(); // For ffrt watchdog available in foundation
    taskHandler_ = TaskHandlerWrap::CreateQueueHandler(AbilityConfig::NAME_ABILITY_MGR_SERVICE);
    eventHandler_ = std::make_shared<AbilityEventHandler>(taskHandler_, weak_from_this());
    freeInstallManager_ = std::make_shared<FreeInstallManager>(weak_from_this());
    CHECK_POINTER_RETURN_BOOL(freeInstallManager_);

    // init user controller.
    userController_ = std::make_shared<UserController>();
    userController_->Init();
    AmsConfigurationParameter::GetInstance().Parse();
    TAG_LOGI(AAFwkTag::ABILITYMGR, "Ability manager service config parse");
    subManagersHelper_ = std::make_shared<SubManagersHelper>(taskHandler_, eventHandler_);
    subManagersHelper_->InitSubManagers(MAIN_USER_ID, true);
    SwitchManagers(U0_USER_ID, false);
#ifdef SUPPORT_SCREEN
    implicitStartProcessor_ = std::make_shared<ImplicitStartProcessor>();
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        InitFocusListener();
    }
    AppExecFwk::AbilityFirstFrameStateObserverManager::GetInstance().Init();
#endif

    DelayedSingleton<ConnectionStateManager>::GetInstance()->Init(taskHandler_);

    InitInterceptor();
    InitStartAbilityChain();
    InitDeepLinkReserve();
    InitDefaultRecoveryList();

    abilityAutoStartupService_ = std::make_shared<AbilityRuntime::AbilityAutoStartupService>();
    InitPushTask();
    AbilityCacheManager::GetInstance().Init(AppUtils::GetInstance().GetLimitMaximumExtensionsPerDevice(),
        AppUtils::GetInstance().GetLimitMaximumExtensionsPerProc());

    SubscribeScreenUnlockedEvent();
    appExitReasonHelper_ = std::make_shared<AppExitReasonHelper>(subManagersHelper_);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "Init success.");
    return true;
}

void AbilityManagerService::InitDeepLinkReserve()
{
    if (!DeepLinkReserveConfig::GetInstance().LoadConfiguration()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "InitDeepLinkReserve failed.");
    }
}

void AbilityManagerService::InitDefaultRecoveryList()
{
    if (!DefaultRecoveryConfig::GetInstance().LoadConfiguration()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Load default recovery list failed.");
    }
}

void AbilityManagerService::InitInterceptor()
{
    interceptorExecuter_ = std::make_shared<AbilityInterceptorExecuter>();
    interceptorExecuter_->AddInterceptor("ScreenUnlock", std::make_shared<ScreenUnlockInterceptor>());
    interceptorExecuter_->AddInterceptor("CrowdTest", std::make_shared<CrowdTestInterceptor>());
    interceptorExecuter_->AddInterceptor("Control", std::make_shared<ControlInterceptor>());
    afterCheckExecuter_ = std::make_shared<AbilityInterceptorExecuter>();
    afterCheckExecuter_->AddInterceptor("ExtensionControl", std::make_shared<ExtensionControlInterceptor>());
    afterCheckExecuter_->AddInterceptor("StartOtherApp", std::make_shared<StartOtherAppInterceptor>());
    afterCheckExecuter_->AddInterceptor("DisposedRule", std::make_shared<DisposedRuleInterceptor>());
    afterCheckExecuter_->AddInterceptor("EcologicalRule", std::make_shared<EcologicalRuleInterceptor>());
    afterCheckExecuter_->SetTaskHandler(taskHandler_);
    bool isAppJumpEnabled = OHOS::system::GetBoolParameter(
        OHOS::AppExecFwk::PARAMETER_APP_JUMP_INTERCEPTOR_ENABLE, false);
    if (isAppJumpEnabled) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "App jump intercetor enabled, add AbilityJumpInterceptor to Executer");
        interceptorExecuter_->AddInterceptor("AbilityJump", std::make_shared<AbilityJumpInterceptor>());
    }
}

void AbilityManagerService::InitPushTask()
{
    if (taskHandler_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "taskHandler_ is nullptr.");
        return;
    }

    auto initStartupFlagTask = [aams = shared_from_this()]() { aams->InitStartupFlag(); };
    taskHandler_->SubmitTask(initStartupFlagTask, "InitStartupFlag");
#ifdef SUPPORT_SCREEN
    auto initPrepareTerminateConfigTask = [aams = shared_from_this()]() { aams->InitPrepareTerminateConfig(); };
    taskHandler_->SubmitTask(initPrepareTerminateConfigTask, "InitPrepareTerminateConfig");
#endif // SUPPORT_SCREEN

    auto initExtensionConfigTask = []() {
        DelayedSingleton<ExtensionConfig>::GetInstance()->LoadExtensionConfiguration();
    };
    taskHandler_->SubmitTask(initExtensionConfigTask, "InitExtensionConfigTask");

    auto bootCompletedTask = [handler = taskHandler_]() {
        if (ApplicationUtil::IsBootCompleted()) {
            auto task = []() {
                ApplicationUtil::AppFwkBootEventCallback(BOOTEVENT_BOOT_COMPLETED, "true", nullptr);
            };
            handler->SubmitTask(task, "BootCompletedDelayTask", BOOTEVENT_COMPLETED_DELAY_TIME);
        } else {
            WatchParameter(BOOTEVENT_BOOT_COMPLETED, ApplicationUtil::AppFwkBootEventCallback, nullptr);
        }
    };
    if (!ParseJsonFromBoot(whiteListJsonObj, SUSPEND_SERVICE_CONFIG_FILE, WHITE_LIST)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Parse json from boot fail");
    }
    isParamStartAbilityEnable_ = system::GetBoolParameter(PARAM_PREVENT_STARTABILITY, false);
    taskHandler_->SubmitTask(bootCompletedTask, "BootCompletedTask");
}

void AbilityManagerService::InitStartupFlag()
{
    startUpNewRule_ = CheckNewRuleSwitchState(COMPONENT_STARTUP_NEW_RULES);
    newRuleExceptLauncherSystemUI_ = CheckNewRuleSwitchState(NEW_RULES_EXCEPT_LAUNCHER_SYSTEMUI);
    backgroundJudgeFlag_ = CheckNewRuleSwitchState(BACKGROUND_JUDGE_FLAG);
    whiteListassociatedWakeUpFlag_ = CheckNewRuleSwitchState(WHITE_LIST_ASS_WAKEUP_FLAG);
}

void AbilityManagerService::InitStartAbilityChain()
{
    auto startSandboxSaveFile = std::make_shared<StartAbilitySandboxSavefile>();
    startAbilityChain_.emplace(startSandboxSaveFile->GetPriority(), startSandboxSaveFile);
}

void AbilityManagerService::OnStop()
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "Stop ability manager service.");
#ifdef BGTASKMGR_CONTINUOUS_TASK_ENABLE
    std::unique_lock<ffrt::mutex> lock(bgtaskObserverMutex_);
    if (bgtaskObserver_) {
        int ret = BackgroundTaskMgrHelper::UnsubscribeBackgroundTask(*bgtaskObserver_);
        if (ret != ERR_OK) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "unsubscribe bgtask failed, err:%{public}d.", ret);
        }
    }
#endif
    if (abilityBundleEventCallback_) {
        auto bms = GetBundleManager();
        if (bms) {
            bool ret = IN_PROCESS_CALL(bms->UnregisterBundleEventCallback(abilityBundleEventCallback_));
            if (ret != ERR_OK) {
                TAG_LOGE(AAFwkTag::ABILITYMGR, "unsubscribe bundle event callback failed, err:%{public}d.", ret);
            }
        }
    }
    eventHandler_.reset();
    taskHandler_.reset();
    state_ = ServiceRunningState::STATE_NOT_START;
}

ServiceRunningState AbilityManagerService::QueryServiceState() const
{
    return state_;
}

int AbilityManagerService::StartAbility(const Want &want, int32_t userId, int requestCode)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    bool isDebugApp = want.GetBoolParam(DEBUG_APP, false);
    bool hasWindowOptions = (want.HasParameter(Want::PARAM_RESV_WINDOW_LEFT) ||
        want.HasParameter(Want::PARAM_RESV_WINDOW_TOP) ||
        want.HasParameter(Want::PARAM_RESV_WINDOW_HEIGHT) ||
        want.HasParameter(Want::PARAM_RESV_WINDOW_WIDTH));
    TAG_LOGD(AAFwkTag::ABILITYMGR, "isDebugApp=%{public}d, hasWindowOptions=%{public}d",
        static_cast<int>(isDebugApp), static_cast<int>(hasWindowOptions));
    bool checkDeveloperModeFlag = (isDebugApp || hasWindowOptions);
    if (checkDeveloperModeFlag && !system::GetBoolParameter(DEVELOPER_MODE_STATE, false)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Developer Mode is false.");
        return ERR_NOT_DEVELOPER_MODE;
    }
    if (!UnlockScreenManager::GetInstance().UnlockScreen()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Screen need passord to unlock");
        return ERR_UNLOCK_SCREEN_FAILED_IN_DEVELOPER_MODE;
    }
    TAG_LOGD(AAFwkTag::ABILITYMGR, "coldStart:%{public}d", want.GetBoolParam("coldStart", false));
    bool startWithAccount = want.GetBoolParam(START_ABILITY_TYPE, false);
    if (startWithAccount || IsCrossUserCall(userId)) {
        (const_cast<Want &>(want)).RemoveParam(START_ABILITY_TYPE);
        CHECK_CALLER_IS_SYSTEM_APP;
    }
    if (hasWindowOptions) {
        if (!AppUtils::GetInstance().IsStartOptionsWithAnimation()) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "window options are not supported in the current product type.");
            return ERR_NOT_SUPPORTED_PRODUCT_TYPE;
        }
        int32_t err = ERR_OK;
        if (userId == DEFAULT_INVAL_VALUE) {
            userId = GetValidUserId(userId);
        }
        if ((err = StartAbilityUtils::CheckAppProvisionMode(want, userId)) != ERR_OK) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "CheckAppProvisionMode returns errcode=%{public}d", err);
            return err;
        }
    }
    InsightIntentExecuteParam::RemoveInsightIntent(const_cast<Want &>(want));
    AbilityUtil::RemoveShowModeKey(const_cast<Want &>(want));
    EventInfo eventInfo = BuildEventInfo(want, userId);
    SendAbilityEvent(EventName::START_ABILITY, HiSysEventType::BEHAVIOR, eventInfo);
    int32_t ret = StartAbilityWrap(want, nullptr, requestCode, false, userId);
    AAFWK::ContinueRadar::GetInstance().ClickIconStartAbility("StartAbilityWrap", ret);
    if (ret != ERR_OK) {
        eventInfo.errCode = ret;
        SendAbilityEvent(EventName::START_ABILITY_ERROR, HiSysEventType::FAULT, eventInfo);
    }
    return ret;
}

int AbilityManagerService::StartAbility(const Want &want, const sptr<IRemoteObject> &callerToken,
    int32_t userId, int requestCode)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    AbilityUtil::RemoveShowModeKey(const_cast<Want &>(want));
    return StartAbilityByFreeInstall(want, callerToken, userId, requestCode);
}

int32_t AbilityManagerService::StartAbilityByFreeInstall(const Want &want, sptr<IRemoteObject> callerToken,
    int32_t userId, int32_t requestCode)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    bool startWithAccount = want.GetBoolParam(START_ABILITY_TYPE, false);
    if (startWithAccount || IsCrossUserCall(userId)) {
        (const_cast<Want &>(want)).RemoveParam(START_ABILITY_TYPE);
        CHECK_CALLER_IS_SYSTEM_APP;
    }
    auto flags = want.GetFlags();
    EventInfo eventInfo = BuildEventInfo(want, userId);
    SendAbilityEvent(EventName::START_ABILITY, HiSysEventType::BEHAVIOR, eventInfo);
    if ((flags & Want::FLAG_ABILITY_CONTINUATION) == Want::FLAG_ABILITY_CONTINUATION) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "StartAbility with continuation flags is not allowed!");
        eventInfo.errCode = ERR_INVALID_VALUE;
        SendAbilityEvent(EventName::START_ABILITY_ERROR, HiSysEventType::FAULT, eventInfo);
        return ERR_INVALID_CONTINUATION_FLAG;
    }

    TAG_LOGD(AAFwkTag::ABILITYMGR, "Start ability come, ability is %{public}s, userId is %{public}d",
        want.GetElement().GetAbilityName().c_str(), userId);

    int32_t ret = StartAbilityWrap(want, callerToken, requestCode, false, userId);
    if (ret != ERR_OK) {
        eventInfo.errCode = ret;
        SendAbilityEvent(EventName::START_ABILITY_ERROR, HiSysEventType::FAULT, eventInfo);
    }
    return ret;
}

int AbilityManagerService::StartAbilityWithSpecifyTokenId(const Want &want, const sptr<IRemoteObject> &callerToken,
    uint32_t specifyTokenId, int32_t userId, int requestCode)
{
    if (IPCSkeleton::GetCallingUid() != FOUNDATION_UID) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "StartAbility with specialId, the current process is not foundation process.");
        return ERR_INVALID_CONTINUATION_FLAG;
    }
    return StartAbilityWithSpecifyTokenIdInner(want, callerToken, specifyTokenId, false, userId, requestCode);
}

int AbilityManagerService::StartAbilityWithSpecifyTokenIdInner(const Want &want, const sptr<IRemoteObject> &callerToken,
    uint32_t specifyTokenId, bool isPendingWantCaller, int32_t userId, int requestCode)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    InsightIntentExecuteParam::RemoveInsightIntent(const_cast<Want &>(want));
    AbilityUtil::RemoveShowModeKey(const_cast<Want &>(want));
    auto flags = want.GetFlags();
    EventInfo eventInfo = BuildEventInfo(want, userId);
    SendAbilityEvent(EventName::START_ABILITY, HiSysEventType::BEHAVIOR, eventInfo);
    if ((flags & Want::FLAG_ABILITY_CONTINUATION) == Want::FLAG_ABILITY_CONTINUATION) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "StartAbility with continuation flags is not allowed.");
        eventInfo.errCode = ERR_INVALID_VALUE;
        SendAbilityEvent(EventName::START_ABILITY_ERROR, HiSysEventType::FAULT, eventInfo);
        return ERR_INVALID_CONTINUATION_FLAG;
    }

    TAG_LOGI(AAFwkTag::ABILITYMGR,
        "Start ability come, ability is %{public}s, userId is %{public}d, specifyTokenId is %{public}u.",
        want.GetElement().GetAbilityName().c_str(), userId, specifyTokenId);

    int32_t ret = StartAbilityWrap(want, callerToken, requestCode, isPendingWantCaller, userId, false, specifyTokenId);
    if (ret != ERR_OK) {
        eventInfo.errCode = ret;
        SendAbilityEvent(EventName::START_ABILITY_ERROR, HiSysEventType::FAULT, eventInfo);
    }
    return ret;
}

int AbilityManagerService::StartAbilityWithSpecifyTokenIdInner(const Want &want, const StartOptions &startOptions,
    const sptr<IRemoteObject> &callerToken, bool isPendingWantCaller,
    int32_t userId, int requestCode, uint32_t callerTokenId)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Start ability with startOptions by trigger.");
    AbilityUtil::RemoveShowModeKey(const_cast<Want &>(want));
    return StartUIAbilityForOptionWrap(
        want, startOptions, callerToken, isPendingWantCaller, userId, requestCode, callerTokenId);
}

int32_t AbilityManagerService::StartAbilityByInsightIntent(const Want &want, const sptr<IRemoteObject> &callerToken,
    uint64_t intentId, int32_t userId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::string bundleNameFromWant = want.GetElement().GetBundleName();
    std::string bundleNameFromIntentMgr = "";
    if (DelayedSingleton<InsightIntentExecuteManager>::GetInstance()->
        GetBundleName(intentId, bundleNameFromIntentMgr) != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "no such bundle matched intentId");
        return ERR_INVALID_VALUE;
    }
    auto abilityRecord = Token::GetAbilityRecordByToken(callerToken);
    if (abilityRecord == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "no such bundle matched token");
        return ERR_INVALID_VALUE;
    }
    std::string bundleNameFromAbilityRecord = abilityRecord->GetAbilityInfo().bundleName;
    if (!bundleNameFromWant.empty() && bundleNameFromWant == bundleNameFromIntentMgr &&
        bundleNameFromWant == bundleNameFromAbilityRecord) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "bundleName match");
        return StartAbility(want, callerToken, userId, -1);
    }
    TAG_LOGE(AAFwkTag::ABILITYMGR, "bundleName not match");
    return ERR_INSIGHT_INTENT_START_INVALID_COMPONENT;
}

int AbilityManagerService::StartAbilityByUIContentSession(const Want &want, const sptr<IRemoteObject> &callerToken,
    const sptr<SessionInfo> &sessionInfo, int32_t userId, int requestCode)
{
    if (!callerToken || !sessionInfo) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "callerToken or sessionInfo is nullptr");
        return ERR_INVALID_VALUE;
    }
    sptr<IRemoteObject> token;
#ifdef SUPPORT_SCREEN
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        Rosen::FocusChangeInfo focusChangeInfo;
        Rosen::WindowManager::GetInstance().GetFocusWindowInfo(focusChangeInfo);
        token = focusChangeInfo.abilityToken_;
    } else {
        if (!wmsHandler_) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "wmsHandler_ is nullptr.");
            return ERR_INVALID_VALUE;
        }
        wmsHandler_->GetFocusWindow(token);
    }
#endif // SUPPORT_SCREEN
    if (!token) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "token is nullptr");
        return ERR_INVALID_VALUE;
    }

    if (token != sessionInfo->callerToken) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "callerToken is not equal to top ablity token");
        return NOT_TOP_ABILITY;
    }
    return StartAbility(want, callerToken, userId, requestCode);
}

int AbilityManagerService::StartAbilityByUIContentSession(const Want &want, const StartOptions &startOptions,
    const sptr<IRemoteObject> &callerToken, const sptr<SessionInfo> &sessionInfo, int32_t userId, int requestCode)
{
    if (!callerToken || !sessionInfo) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "callerToken or sessionInfo is nullptr");
        return ERR_INVALID_VALUE;
    }
    sptr<IRemoteObject> token;
#ifdef SUPPORT_SCREEN
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        Rosen::FocusChangeInfo focusChangeInfo;
        Rosen::WindowManager::GetInstance().GetFocusWindowInfo(focusChangeInfo);
        token = focusChangeInfo.abilityToken_;
    } else {
        if (!wmsHandler_) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "wmsHandler_ is nullptr.");
            return ERR_INVALID_VALUE;
        }
        wmsHandler_->GetFocusWindow(token);
    }
#endif // SUPPORT_SCREEN

    if (!token) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "token is nullptr");
        return ERR_INVALID_VALUE;
    }

    if (token != sessionInfo->callerToken) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "callerToken is not equal to top ablity token");
        return NOT_TOP_ABILITY;
    }
    return StartAbility(want, startOptions, callerToken, userId, requestCode);
}

int AbilityManagerService::StartAbilityAsCaller(const Want &want, const sptr<IRemoteObject> &callerToken,
    sptr<IRemoteObject> asCallerSourceToken, int32_t userId, int requestCode)
{
    return StartAbilityAsCallerDetails(want, callerToken, asCallerSourceToken, userId, requestCode);
}

int AbilityManagerService::ImplicitStartAbilityAsCaller(const Want &want, const sptr<IRemoteObject> &callerToken,
    sptr<IRemoteObject> asCallerSourceToken, int32_t userId, int requestCode)
{
    return StartAbilityAsCallerDetails(want, callerToken, asCallerSourceToken, userId,
        requestCode, true);
}

int AbilityManagerService::StartAbilityAsCallerDetails(const Want &want, const sptr<IRemoteObject> &callerToken,
    sptr<IRemoteObject> asCallerSourceToken, int32_t userId, int requestCode, bool isImplicit)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    CHECK_CALLER_IS_SYSTEM_APP;
    auto flags = want.GetFlags();
    AbilityUtil::RemoveShowModeKey(const_cast<Want &>(want));
    EventInfo eventInfo = BuildEventInfo(want, userId);
    SendAbilityEvent(EventName::START_ABILITY, HiSysEventType::BEHAVIOR, eventInfo);
    if ((flags & Want::FLAG_ABILITY_CONTINUATION) == Want::FLAG_ABILITY_CONTINUATION) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "StartAbility with continuation flags is not allowed!");
        eventInfo.errCode = ERR_INVALID_VALUE;
        SendAbilityEvent(EventName::START_ABILITY_ERROR, HiSysEventType::FAULT, eventInfo);
        return ERR_INVALID_CONTINUATION_FLAG;
    }

    AAFwk::Want newWant = want;
    UpdateAsCallerSourceInfo(newWant, asCallerSourceToken, callerToken);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "Start ability come, ability is %{public}s, userId is %{public}d",
        want.GetElement().GetAbilityName().c_str(), userId);
    std::string callerPkg;
    std::string targetPkg;
    if (AbilityUtil::CheckJumpInterceptorWant(newWant, callerPkg, targetPkg)) {
        TAG_LOGI(AAFwkTag::ABILITYMGR,
            "the call is from interceptor dialog, callerPkg:%{public}s, targetPkg:%{public}s",
            callerPkg.c_str(), targetPkg.c_str());
        AbilityUtil::AddAbilityJumpRuleToBms(callerPkg, targetPkg, GetUserId());
    }
    int32_t ret = StartAbilityWrap(newWant, callerToken, requestCode, false, userId, true,
        0, false, isImplicit);
    if (ret != ERR_OK) {
        eventInfo.errCode = ret;
        SendAbilityEvent(EventName::START_ABILITY_ERROR, HiSysEventType::FAULT, eventInfo);
    }
    return ret;
}

int AbilityManagerService::StartAbilityPublicPrechainCheck(StartAbilityParams &params)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    // 1. CheckCallerToken
    if (params.callerToken != nullptr && !VerificationAllToken(params.callerToken)) {
        auto isSpecificSA = AAFwk::PermissionVerification::GetInstance()->
            CheckSpecificSystemAbilityAccessPermission(DMS_PROCESS_NAME);
        if (!isSpecificSA) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "%{public}s VerificationAllToken failed.", __func__);
            return ERR_INVALID_CALLER;
        }
        TAG_LOGI(AAFwkTag::ABILITYMGR, "%{public}s: Caller is specific system ability.", __func__);
    }

    // 2. validUserId, multi-user
    if (!JudgeMultiUserConcurrency(params.GetValidUserId())) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Multi-user non-concurrent mode is not satisfied.");
        return ERR_CROSS_USER;
    }

    return ERR_OK;
}

int AbilityManagerService::StartAbilityPrechainInterceptor(StartAbilityParams &params)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    AbilityInterceptorParam interceptorParam = AbilityInterceptorParam(params.want, params.requestCode,
        GetUserId(), true, nullptr);
    auto interceptorResult = interceptorExecuter_ == nullptr ? ERR_INVALID_VALUE :
        interceptorExecuter_->DoProcess(interceptorParam);
    if (interceptorResult != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "interceptorExecuter_ is nullptr or DoProcess return error.");
        return interceptorResult;
    }

    return ERR_OK;
}

bool AbilityManagerService::StartAbilityInChain(StartAbilityParams &params, int &result)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::shared_ptr<StartAbilityHandler> reqHandler;
    for (const auto &item : startAbilityChain_) {
        if (item.second->MatchStartRequest(params)) {
            reqHandler = item.second;
            break;
        }
    }

    if (!reqHandler) {
        return false;
    }

    result = StartAbilityPublicPrechainCheck(params);
    if (result != ERR_OK) {
        return true;
    }
    result = StartAbilityPrechainInterceptor(params);
    if (result != ERR_OK) {
        return true;
    }
    result = reqHandler->HandleStartRequest(params);
    return true;
}

int AbilityManagerService::StartAbilityWrap(const Want &want, const sptr<IRemoteObject> &callerToken,
    int requestCode, bool isPendingWantCaller, int32_t userId, bool isStartAsCaller, uint32_t specifyToken,
    bool isForegroundToRestartApp, bool isImplicit)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    StartAbilityParams startParams(const_cast<Want &>(want));
    startParams.callerToken = callerToken;
    startParams.userId = userId;
    startParams.requestCode = requestCode;
    startParams.isStartAsCaller = isStartAsCaller;
    startParams.SetValidUserId(GetValidUserId(userId));

    int result = ERR_OK;
    if (StartAbilityInChain(startParams, result)) {
        return result;
    }

    return StartAbilityInner(want, callerToken,
        requestCode, isPendingWantCaller, userId, isStartAsCaller, specifyToken, isForegroundToRestartApp, isImplicit);
}

void AbilityManagerService::SetReserveInfo(const std::string &linkString, AbilityRequest& abilityRequest)
{
    if (!linkString.size()) {
        return;
    }

    std::string reservedBundleName = "";
#ifdef SUPPORT_SCREEN
    if (DeepLinkReserveConfig::GetInstance().isLinkReserved(linkString, reservedBundleName)) {
        abilityRequest.uriReservedFlag = true;
        abilityRequest.reservedBundleName = reservedBundleName;
    } else {
        abilityRequest.uriReservedFlag = false;
        abilityRequest.reservedBundleName = reservedBundleName;
    }
#endif // SUPPORT_SCREEN
}

int AbilityManagerService::CheckExtensionCallPermission(const Want& want, const AbilityRequest& abilityRequest)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, "startExtensionCheck");
    auto isSACall = AAFwk::PermissionVerification::GetInstance()->IsSACall();
    auto isSystemAppCall = AAFwk::PermissionVerification::GetInstance()->IsSystemAppCall();
    auto isShellCall = AAFwk::PermissionVerification::GetInstance()->IsShellCall();
    auto isToPermissionMgr = IsTargetPermission(want);
    if (!isSACall && !isSystemAppCall && !isShellCall && !isToPermissionMgr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR,
            "Cannot start extension by start ability, use startServiceExtensionAbility.");
        return ERR_WRONG_INTERFACE_CALL;
    }
    int result = CheckCallServicePermission(abilityRequest);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Check permission failed");
    }
    return result;
}

int AbilityManagerService::CheckServiceCallPermission(const AbilityRequest& abilityRequest,
    const AppExecFwk::AbilityInfo& abilityInfo)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR,
        "Check call service or extension permission, name is %{public}s.", abilityInfo.name.c_str());
    int result = CheckCallServicePermission(abilityRequest);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Check permission failed");
    }
    return result;
}

int AbilityManagerService::CheckBrokerCallPermission(const AbilityRequest& abilityRequest,
    const AppExecFwk::AbilityInfo& abilityInfo)
{
    // temp add for broker, remove when delete issacall
    if (abilityRequest.collaboratorType != CollaboratorType::RESERVE_TYPE && !abilityInfo.visible) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "Check permission failed");
        return CHECK_PERMISSION_FAILED;
    }
    TAG_LOGD(AAFwkTag::ABILITYMGR,
        "Check call service or extension permission, name is %{public}s.", abilityInfo.name.c_str());
    auto collaborator = GetCollaborator(CollaboratorType::RESERVE_TYPE);
    if (collaborator == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Collaborator is nullptr.");
        return CHECK_PERMISSION_FAILED;
    }
    int result = collaborator->CheckCallAbilityPermission(abilityRequest.want);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Check permission failed from broker.");
        return CHECK_PERMISSION_FAILED;
    }
    return result;
}

int AbilityManagerService::CheckAbilityCallPermission(const AbilityRequest& abilityRequest,
    const AppExecFwk::AbilityInfo& abilityInfo, uint32_t specifyTokenId)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Check call ability permission, name is %{public}s.", abilityInfo.name.c_str());
    int result = CheckCallAbilityPermission(abilityRequest, specifyTokenId);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Check permission failed");
    }
    return result;
}

int AbilityManagerService::CheckCallPermission(const Want& want, const AppExecFwk::AbilityInfo& abilityInfo,
    const AbilityRequest& abilityRequest, bool isForegroundToRestartApp,
    bool isSendDialogResult, uint32_t specifyTokenId,
    const std::string& callerBundleName)
{
    auto type = abilityInfo.type;
    if (type == AppExecFwk::AbilityType::DATA) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Cannot start data ability by start ability.");
        return ERR_WRONG_INTERFACE_CALL;
    }
    if (type == AppExecFwk::AbilityType::EXTENSION) {
        return CheckExtensionCallPermission(want, abilityRequest);
    }
    if (type == AppExecFwk::AbilityType::SERVICE) {
        return CheckServiceCallPermission(abilityRequest, abilityInfo);
    }
    if ((callerBundleName == SHELL_ASSISTANT_BUNDLENAME && AppUtils::GetInstance().IsSupportAncoApp()) ||
        IPCSkeleton::GetCallingUid() == BROKER_UID) {
        return CheckBrokerCallPermission(abilityRequest, abilityInfo);
    }
    if (!isForegroundToRestartApp && (!isSendDialogResult || want.GetBoolParam("isSelector", false))) {
        return CheckAbilityCallPermission(abilityRequest, abilityInfo, specifyTokenId);
    }
    return ERR_OK;
}

int AbilityManagerService::StartAbilityInner(const Want &want, const sptr<IRemoteObject> &callerToken,
    int requestCode, bool isPendingWantCaller, int32_t userId, bool isStartAsCaller, uint32_t specifyTokenId,
    bool isForegroundToRestartApp, bool isImplicit)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::string dialogSessionId = want.GetStringParam("dialogSessionId");
    bool isSendDialogResult = false;
#ifdef SUPPORT_SCREEN
    if (!dialogSessionId.empty() &&
        DialogSessionManager::GetInstance().GetDialogCallerInfo(dialogSessionId) != nullptr) {
        isSendDialogResult = true;
    }
#endif // SUPPORT_SCREEN

    // prevent the app from dominating the screen
    if (system::GetBoolParameter(VERIFY_DOMINATE_SCREEN, true) &&
        callerToken == nullptr && !IsCallerSceneBoard() && !isSendDialogResult && !isForegroundToRestartApp &&
        AbilityPermissionUtil::GetInstance().IsDominateScreen(want, isPendingWantCaller)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "caller is invalid.");
        return ERR_INVALID_CALLER;
    }
    {
#ifdef WITH_DLP
        HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, "CHECK_DLP");
        if (!DlpUtils::OtherAppsAccessDlpCheck(callerToken, want) ||
            VerifyAccountPermission(userId) == CHECK_PERMISSION_FAILED ||
            !DlpUtils::DlpAccessOtherAppsCheck(callerToken, want)) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "%{public}s: Permission verification failed.", __func__);
            return CHECK_PERMISSION_FAILED;
        }

        if (AbilityUtil::HandleDlpApp(const_cast<Want &>(want))) {
            InsightIntentExecuteParam::RemoveInsightIntent(const_cast<Want &>(want));
            return StartExtensionAbilityInner(want, callerToken, userId,
                AppExecFwk::ExtensionAbilityType::SERVICE, false, false, true);
        }
#endif // WITH_DLP
    }

    AbilityUtil::RemoveWindowModeKey(const_cast<Want &>(want));
    if (callerToken != nullptr && !VerificationAllToken(callerToken) && !isSendDialogResult) {
        auto isSpecificSA = AAFwk::PermissionVerification::GetInstance()->
            CheckSpecificSystemAbilityAccessPermission(DMS_PROCESS_NAME);
        if (!isSpecificSA) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "%{public}s VerificationAllToken failed.", __func__);
            return ERR_INVALID_CALLER;
        }
        TAG_LOGI(AAFwkTag::ABILITYMGR, "%{public}s: Caller is specific system ability.", __func__);
    }

    int32_t oriValidUserId = GetValidUserId(userId);
    int32_t validUserId = oriValidUserId;

    int32_t appIndex = 0;
    if (!StartAbilityUtils::GetAppIndex(want, callerToken, appIndex)) {
        return ERR_APP_CLONE_INDEX_INVALID;
    }
    StartAbilityInfoWrap threadLocalInfo(want, validUserId, appIndex, callerToken);
    AbilityInterceptorParam interceptorParam = AbilityInterceptorParam(want, requestCode, GetUserId(),
        true, nullptr);
    auto result = interceptorExecuter_ == nullptr ? ERR_INVALID_VALUE :
        interceptorExecuter_->DoProcess(interceptorParam);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "interceptorExecuter_ is nullptr or DoProcess return error.");
        return result;
    }

    if (callerToken != nullptr && CheckIfOperateRemote(want)) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "try to StartRemoteAbility");
        return StartRemoteAbility(want, requestCode, validUserId, callerToken);
    }

    if (!JudgeMultiUserConcurrency(validUserId)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Multi-user non-concurrent mode is not satisfied.");
        return ERR_CROSS_USER;
    }

    AbilityRequest abilityRequest;
#ifdef SUPPORT_SCREEN
    if (ImplicitStartProcessor::IsImplicitStartAction(want)) {
        abilityRequest.Voluation(want, requestCode, callerToken);
        if (specifyTokenId > 0 && callerToken != nullptr) { // for sa specify tokenId and caller token
            UpdateCallerInfoFromToken(abilityRequest.want, callerToken);
        } else if (!isStartAsCaller) {
            TAG_LOGD(AAFwkTag::ABILITYMGR, "do not start as caller, UpdateCallerInfo");
            UpdateCallerInfo(abilityRequest.want, callerToken);
        } else {
            TAG_LOGD(AAFwkTag::ABILITYMGR, "start as caller, skip UpdateCallerInfo!");
        }
        CHECK_POINTER_AND_RETURN(implicitStartProcessor_, ERR_IMPLICIT_START_ABILITY_FAIL);
        SetReserveInfo(want.GetUriString(), abilityRequest);
        return implicitStartProcessor_->ImplicitStartAbility(abilityRequest, validUserId);
    }
    if (want.GetAction().compare(ACTION_CHOOSE) == 0) {
        return ShowPickerDialog(want, validUserId, callerToken);
    }
#endif
    result = GenerateAbilityRequest(want, requestCode, abilityRequest, callerToken, validUserId);
    auto abilityRecord = Token::GetAbilityRecordByToken(callerToken);
    std::string callerBundleName = abilityRecord ? abilityRecord->GetAbilityInfo().bundleName : "";
    bool selfFreeInstallEnable = (result == RESOLVE_ABILITY_ERR && want.GetElement().GetModuleName() != "" &&
                                  want.GetElement().GetBundleName() == callerBundleName);
    bool isStartFreeInstallByWant = AbilityUtil::IsStartFreeInstall(want);
    if (isStartFreeInstallByWant || selfFreeInstallEnable) {
        Want localWant;
        auto freeInstallResult = PreStartFreeInstall(want, callerToken, specifyTokenId, isStartAsCaller, localWant);
        if (freeInstallResult != ERR_OK) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "PreStartFreeInstall failed.");
            return freeInstallResult;
        }

        if (isStartFreeInstallByWant) {
            return freeInstallManager_->StartFreeInstall(localWant, validUserId, requestCode,
                callerToken, true, specifyTokenId);
        }
        int32_t ret = freeInstallManager_->StartFreeInstall(localWant, validUserId, requestCode, callerToken, false);
        if (ret == ERR_OK) {
            result = GenerateAbilityRequest(want, requestCode, abilityRequest, callerToken, validUserId);
        }
    }

    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Generate ability request local error.");
        return result;
    }
    if (!UriUtils::GetInstance().CheckNonImplicitShareFileUri(abilityRequest)) {
        return ERR_SHARE_FILE_URI_NON_IMPLICITLY;
    }

    if (specifyTokenId > 0 && callerToken != nullptr) { // for sa specify tokenId and caller token
        UpdateCallerInfoFromToken(abilityRequest.want, callerToken);
    } else if (!isStartAsCaller) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "do not start as caller, UpdateCallerInfo");
        UpdateCallerInfo(abilityRequest.want, callerToken);
    } else if (callerBundleName == BUNDLE_NAME_DIALOG ||
        (isSendDialogResult && want.GetBoolParam("isSelector", false))) {
#ifdef SUPPORT_SCREEN
        CHECK_POINTER_AND_RETURN(implicitStartProcessor_, ERR_IMPLICIT_START_ABILITY_FAIL);
        implicitStartProcessor_->ResetCallingIdentityAsCaller(abilityRequest.want.GetIntParam(
            Want::PARAM_RESV_CALLER_TOKEN, 0));
#endif // SUPPORT_SCREEN
    }

    auto abilityInfo = abilityRequest.abilityInfo;
    validUserId = abilityInfo.applicationInfo.singleton ? U0_USER_ID : validUserId;
    TAG_LOGD(AAFwkTag::ABILITYMGR, "userId is : %{public}d, singleton is : %{public}d",
        validUserId, static_cast<int>(abilityInfo.applicationInfo.singleton));

    if ((isSendDialogResult && want.GetBoolParam("isSelector", false))) {
        isImplicit = true;
    }
    result = CheckStaticCfgPermission(abilityRequest, isStartAsCaller,
        abilityRequest.want.GetIntParam(Want::PARAM_RESV_CALLER_TOKEN, 0), false, false, isImplicit);
    if (result != AppExecFwk::Constants::PERMISSION_GRANTED) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "CheckStaticCfgPermission error, result is %{public}d.", result);
        return ERR_STATIC_CFG_PERMISSION;
    }

    result = CheckCallPermission(want, abilityInfo, abilityRequest, isForegroundToRestartApp,
        isSendDialogResult, specifyTokenId, callerBundleName);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "CheckCallPermission error, result is %{public}d.", result);
        return result;
    }

    Want newWant = abilityRequest.want;
    AbilityInterceptorParam afterCheckParam = AbilityInterceptorParam(newWant, requestCode, GetUserId(),
        true, callerToken, std::make_shared<AppExecFwk::AbilityInfo>(abilityInfo), isStartAsCaller);
    result = afterCheckExecuter_ == nullptr ? ERR_INVALID_VALUE :
        afterCheckExecuter_->DoProcess(afterCheckParam);
    bool isReplaceWantExist = newWant.GetBoolParam("queryWantFromErms", false);
    newWant.RemoveParam("queryWantFromErms");
    if (result != ERR_OK && isReplaceWantExist == false) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "DoProcess failed or replaceWant not exist");
        return result;
    }
#ifdef SUPPORT_SCREEN
    if (result != ERR_OK && isReplaceWantExist && !isSendDialogResult &&
        callerBundleName != BUNDLE_NAME_DIALOG) {
        return DialogSessionManager::GetInstance().HandleErmsResult(abilityRequest, GetUserId(), newWant);
    }
    if (result == ERR_OK &&
        DialogSessionManager::GetInstance().IsCreateCloneSelectorDialog(abilityInfo.bundleName, GetUserId())) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "create clone selector dialog");
        return CreateCloneSelectorDialog(abilityRequest, GetUserId());
    }
#endif // SUPPORT_SCREEN

    if (!AbilityUtil::IsSystemDialogAbility(abilityInfo.bundleName, abilityInfo.name)) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "PreLoadAppDataAbilities:%{public}s.", abilityInfo.bundleName.c_str());
        result = PreLoadAppDataAbilities(abilityInfo.bundleName, validUserId);
        if (result != ERR_OK) {
            TAG_LOGE(AAFwkTag::ABILITYMGR,
                "StartAbility: App data ability preloading failed, '%{public}s', %{public}d.",
                abilityInfo.bundleName.c_str(), result);
            return result;
        }
    }

    if (abilityInfo.type == AppExecFwk::AbilityType::SERVICE ||
        abilityInfo.type == AppExecFwk::AbilityType::EXTENSION) {
        return StartAbilityByConnectManager(want, abilityRequest, abilityInfo, validUserId, callerToken);
    }

    if (!IsAbilityControllerStart(want, abilityInfo.bundleName)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "IsAbilityControllerStart failed: %{public}s.", abilityInfo.bundleName.c_str());
        return ERR_WOULD_BLOCK;
    }

    abilityRequest.want.RemoveParam(SPECIFY_TOKEN_ID);
    if (specifyTokenId > 0) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "Set specifyTokenId, the specifyTokenId is %{public}d.", specifyTokenId);
        abilityRequest.want.SetParam(SPECIFY_TOKEN_ID, static_cast<int32_t>(specifyTokenId));
        abilityRequest.specifyTokenId = specifyTokenId;
    }
    abilityRequest.want.RemoveParam(PARAM_SPECIFIED_PROCESS_FLAG);
    // sceneboard
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        ReportEventToRSS(abilityInfo, abilityRequest.callerToken);
        abilityRequest.userId = oriValidUserId;
        abilityRequest.want.SetParam(IS_CALL_BY_SCB, false);
        // other sa or shell can not use continueSessionId and persistentId
        auto abilityRecord = Token::GetAbilityRecordByToken(abilityRequest.callerToken);
        if (abilityRecord == nullptr &&
            !PermissionVerification::GetInstance()->CheckSpecificSystemAbilityAccessPermission(DMS_PROCESS_NAME)) {
            TAG_LOGW(AAFwkTag::ABILITYMGR, "StartAbilityInner, Remove continueSessionId and persistentId");
            abilityRequest.want.RemoveParam(DMS_CONTINUED_SESSION_ID);
            abilityRequest.want.RemoveParam(DMS_PERSISTENT_ID);
        }
        auto uiAbilityManager = GetUIAbilityManagerByUserId(oriValidUserId);
        CHECK_POINTER_AND_RETURN(uiAbilityManager, ERR_INVALID_VALUE);
        return uiAbilityManager->NotifySCBToStartUIAbility(abilityRequest);
    }

    auto missionListManager = GetMissionListManagerByUserId(oriValidUserId);
    if (missionListManager == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "missionListManager is nullptr. userId=%{public}d", validUserId);
        return ERR_INVALID_VALUE;
    }

    ReportAbilitStartInfoToRSS(abilityInfo);
    ReportEventToRSS(abilityInfo, callerToken);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Start ability, name is %{public}s.", abilityInfo.name.c_str());
    return missionListManager->StartAbility(abilityRequest);
}

int AbilityManagerService::PreStartFreeInstall(const Want &want, sptr<IRemoteObject> callerToken,
    uint32_t specifyTokenId, bool isStartAsCaller, Want &localWant)
{
    if (freeInstallManager_ == nullptr) {
        return ERR_INVALID_VALUE;
    }
    (const_cast<Want &>(want)).RemoveParam("send_to_erms_embedded");
    localWant = want;
    if (!localWant.GetDeviceId().empty()) {
        localWant.SetDeviceId("");
    }
    if (specifyTokenId > 0 && callerToken != nullptr) { // for sa specify tokenId and caller token
        UpdateCallerInfoFromToken(localWant, callerToken);
    } else if (!isStartAsCaller) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "do not start as caller, UpdateCallerInfo");
        UpdateCallerInfo(localWant, callerToken);
    } else {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "start as caller, skip UpdateCallerInfo!");
    }
    return ERR_OK;
}

int AbilityManagerService::StartAbilityByConnectManager(const Want& want, const AbilityRequest& abilityRequest,
    const AppExecFwk::AbilityInfo& abilityInfo, int validUserId, sptr<IRemoteObject> callerToken)
{
    auto connectManager = GetConnectManagerByUserId(validUserId);
    if (!connectManager) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "connectManager is nullptr. userId=%{public}d", validUserId);
        return ERR_INVALID_VALUE;
    }
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Start service or extension, name is %{public}s.", abilityInfo.name.c_str());
    ReportEventToRSS(abilityInfo, callerToken);
    InsightIntentExecuteParam::RemoveInsightIntent(const_cast<Want &>(want));
    return connectManager->StartAbility(abilityRequest);
}

int AbilityManagerService::StartAbility(const Want &want, const AbilityStartSetting &abilityStartSetting,
    const sptr<IRemoteObject> &callerToken, int32_t userId, int requestCode)
{
    return StartAbilityDetails(want, abilityStartSetting, callerToken, userId, requestCode);
}

int AbilityManagerService::ImplicitStartAbility(const Want &want, const AbilityStartSetting &abilityStartSetting,
    const sptr<IRemoteObject> &callerToken, int32_t userId, int requestCode)
{
    return StartAbilityDetails(want, abilityStartSetting, callerToken, userId, requestCode, true);
}

int AbilityManagerService::StartAbilityDetails(const Want &want, const AbilityStartSetting &abilityStartSetting,
    const sptr<IRemoteObject> &callerToken, int32_t userId, int requestCode, bool isImplicit)
{
    if (want.GetBoolParam(DEBUG_APP, false) && !system::GetBoolParameter(DEVELOPER_MODE_STATE, false)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Developer Mode is false.");
        return ERR_NOT_DEVELOPER_MODE;
    }
    if (!UnlockScreenManager::GetInstance().UnlockScreen()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Screen need passord to unlock");
        return ERR_UNLOCK_SCREEN_FAILED_IN_DEVELOPER_MODE;
    }
    AbilityUtil::RemoveWantKey(const_cast<Want &>(want));
    StartAbilityParams startParams(const_cast<Want &>(want));
    startParams.callerToken = callerToken;
    startParams.userId = userId;
    startParams.requestCode = requestCode;
    startParams.SetValidUserId(GetValidUserId(userId));

    int result = ERR_OK;
    if (StartAbilityInChain(startParams, result)) {
        return result;
    }

    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Start ability setting.");
    if (IsCrossUserCall(userId)) {
        CHECK_CALLER_IS_SYSTEM_APP;
    }
    EventInfo eventInfo = BuildEventInfo(want, userId);
    SendAbilityEvent(EventName::START_ABILITY, HiSysEventType::BEHAVIOR, eventInfo);

#ifdef WITH_DLP
    if (!DlpUtils::OtherAppsAccessDlpCheck(callerToken, want) ||
        VerifyAccountPermission(userId) == CHECK_PERMISSION_FAILED ||
        !DlpUtils::DlpAccessOtherAppsCheck(callerToken, want)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "%{public}s: Permission verification failed", __func__);
        eventInfo.errCode = CHECK_PERMISSION_FAILED;
        SendAbilityEvent(EventName::START_ABILITY_ERROR, HiSysEventType::FAULT, eventInfo);
        return CHECK_PERMISSION_FAILED;
    }
#endif // WITH_DLP

    if (callerToken != nullptr && !VerificationAllToken(callerToken)) {
        eventInfo.errCode = ERR_INVALID_VALUE;
        SendAbilityEvent(EventName::START_ABILITY_ERROR, HiSysEventType::FAULT, eventInfo);
        return ERR_INVALID_CALLER;
    }

    int32_t oriValidUserId = GetValidUserId(userId);
    int32_t validUserId = oriValidUserId;
    int32_t appIndex = 0;
    if (!StartAbilityUtils::GetAppIndex(want, callerToken, appIndex)) {
        return ERR_APP_CLONE_INDEX_INVALID;
    }
    StartAbilityInfoWrap threadLocalInfo(want, validUserId, appIndex, callerToken);
    AbilityInterceptorParam interceptorParam = AbilityInterceptorParam(want, requestCode, GetUserId(),
        true, nullptr);
    result = interceptorExecuter_ == nullptr ? ERR_INVALID_VALUE :
        interceptorExecuter_->DoProcess(interceptorParam);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "interceptorExecuter_ is nullptr or DoProcess return error.");
        eventInfo.errCode = result;
        SendAbilityEvent(EventName::START_ABILITY_ERROR, HiSysEventType::FAULT, eventInfo);
        return result;
    }

    if (AbilityUtil::IsStartFreeInstall(want)) {
        if (CheckIfOperateRemote(want) || freeInstallManager_ == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "can not start remote free install");
            return ERR_INVALID_VALUE;
        }
        (const_cast<Want &>(want)).RemoveParam("send_to_erms_embedded");
        Want localWant = want;
        UpdateCallerInfo(localWant, callerToken);
        return freeInstallManager_->StartFreeInstall(localWant, validUserId, requestCode, callerToken, true);
    }

    if (!JudgeMultiUserConcurrency(validUserId)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Multi-user non-concurrent mode is not satisfied.");
        eventInfo.errCode = ERR_INVALID_VALUE;
        SendAbilityEvent(EventName::START_ABILITY_ERROR, HiSysEventType::FAULT, eventInfo);
        return ERR_CROSS_USER;
    }

    AbilityRequest abilityRequest;
#ifdef SUPPORT_SCREEN
    if (ImplicitStartProcessor::IsImplicitStartAction(want)) {
        abilityRequest.Voluation(
            want, requestCode, callerToken, std::make_shared<AbilityStartSetting>(abilityStartSetting));
        abilityRequest.callType = AbilityCallType::START_SETTINGS_TYPE;
        CHECK_POINTER_AND_RETURN(implicitStartProcessor_, ERR_IMPLICIT_START_ABILITY_FAIL);
        result = implicitStartProcessor_->ImplicitStartAbility(abilityRequest, validUserId);
        if (result != ERR_OK) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "implicit start ability error.");
            eventInfo.errCode = result;
            SendAbilityEvent(EventName::START_ABILITY_ERROR, HiSysEventType::FAULT, eventInfo);
        }
        return result;
    }
    if (want.GetAction().compare(ACTION_CHOOSE) == 0) {
        return ShowPickerDialog(want, validUserId, callerToken);
    }
#endif
    result = GenerateAbilityRequest(want, requestCode, abilityRequest, callerToken, validUserId);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Generate ability request local error.");
        eventInfo.errCode = result;
        SendAbilityEvent(EventName::START_ABILITY_ERROR, HiSysEventType::FAULT, eventInfo);
        return result;
    }
    abilityRequest.want.RemoveParam(PARAM_SPECIFIED_PROCESS_FLAG);

    auto abilityInfo = abilityRequest.abilityInfo;
    validUserId = abilityInfo.applicationInfo.singleton ? U0_USER_ID : validUserId;
    TAG_LOGD(AAFwkTag::ABILITYMGR, "userId : %{public}d, singleton is : %{public}d",
        validUserId, static_cast<int>(abilityInfo.applicationInfo.singleton));

    result = CheckStaticCfgPermission(abilityRequest, false, -1, false, false, isImplicit);
    if (result != AppExecFwk::Constants::PERMISSION_GRANTED) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "CheckStaticCfgPermission error, result is %{public}d.", result);
        eventInfo.errCode = result;
        SendAbilityEvent(EventName::START_ABILITY_ERROR, HiSysEventType::FAULT, eventInfo);
        return ERR_STATIC_CFG_PERMISSION;
    }
    result = CheckCallAbilityPermission(abilityRequest);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "%{public}s CheckCallAbilityPermission error.", __func__);
        eventInfo.errCode = result;
        SendAbilityEvent(EventName::START_ABILITY_ERROR, HiSysEventType::FAULT, eventInfo);
        return result;
    }

    abilityRequest.startSetting = std::make_shared<AbilityStartSetting>(abilityStartSetting);

    if (abilityInfo.type == AppExecFwk::AbilityType::DATA) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Cannot start data ability, use 'AcquireDataAbility()' instead.");
        eventInfo.errCode = ERR_INVALID_VALUE;
        SendAbilityEvent(EventName::START_ABILITY_ERROR, HiSysEventType::FAULT, eventInfo);
        return ERR_WRONG_INTERFACE_CALL;
    }

    AbilityInterceptorParam afterCheckParam = AbilityInterceptorParam(abilityRequest.want, requestCode,
        GetUserId(), true, callerToken, std::make_shared<AppExecFwk::AbilityInfo>(abilityInfo));
    result = afterCheckExecuter_ == nullptr ? ERR_INVALID_VALUE :
        afterCheckExecuter_->DoProcess(afterCheckParam);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "afterCheckExecuter_ is nullptr or DoProcess return error.");
        return result;
    }

    if (!AbilityUtil::IsSystemDialogAbility(abilityInfo.bundleName, abilityInfo.name)) {
        result = PreLoadAppDataAbilities(abilityInfo.bundleName, validUserId);
        if (result != ERR_OK) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "StartAbility: App data ability preloading failed, '%{public}s', %{public}d",
                abilityInfo.bundleName.c_str(),
                result);
            eventInfo.errCode = result;
            SendAbilityEvent(EventName::START_ABILITY_ERROR, HiSysEventType::FAULT, eventInfo);
            return result;
        }
    }
#ifdef SUPPORT_GRAPHICS
    if (abilityInfo.type != AppExecFwk::AbilityType::PAGE) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Only support for page type ability.");
        eventInfo.errCode = ERR_INVALID_VALUE;
        SendAbilityEvent(EventName::START_ABILITY_ERROR, HiSysEventType::FAULT, eventInfo);
        return ERR_WRONG_INTERFACE_CALL;
    }
#endif
    if (!IsAbilityControllerStart(want, abilityInfo.bundleName)) {
        eventInfo.errCode = ERR_WOULD_BLOCK;
        SendAbilityEvent(EventName::START_ABILITY_ERROR, HiSysEventType::FAULT, eventInfo);
        return ERR_WOULD_BLOCK;
    }
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        UpdateCallerInfo(abilityRequest.want, callerToken);
        abilityRequest.userId = oriValidUserId;
        abilityRequest.want.SetParam(IS_CALL_BY_SCB, false);
        auto uiAbilityManager = GetUIAbilityManagerByUserId(oriValidUserId);
        CHECK_POINTER_AND_RETURN(uiAbilityManager, ERR_INVALID_VALUE);
        return uiAbilityManager->NotifySCBToStartUIAbility(abilityRequest);
    }
    auto missionListManager = GetMissionListManagerByUserId(oriValidUserId);
    if (missionListManager == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "missionListManager is Null. userId=%{public}d", validUserId);
        eventInfo.errCode = ERR_INVALID_VALUE;
        SendAbilityEvent(EventName::START_ABILITY_ERROR, HiSysEventType::FAULT, eventInfo);
        return ERR_INVALID_VALUE;
    }
    UpdateCallerInfo(abilityRequest.want, callerToken);
    auto ret = missionListManager->StartAbility(abilityRequest);
    if (ret != ERR_OK) {
        eventInfo.errCode = ret;
        SendAbilityEvent(EventName::START_ABILITY_ERROR, HiSysEventType::FAULT, eventInfo);
    }
    return ret;
}

int AbilityManagerService::StartAbility(const Want &want, const StartOptions &startOptions,
    const sptr<IRemoteObject> &callerToken, int32_t userId, int requestCode)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Start ability with startOptions.");
    AbilityUtil::RemoveShowModeKey(const_cast<Want &>(want));
    return StartUIAbilityForOptionWrap(want, startOptions, callerToken, false, userId, requestCode);
}

int AbilityManagerService::ImplicitStartAbility(const Want &want, const StartOptions &startOptions,
    const sptr<IRemoteObject> &callerToken, int32_t userId, int requestCode)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Implicit Start ability with startOptions.");
    AbilityUtil::RemoveShowModeKey(const_cast<Want &>(want));
    return StartUIAbilityForOptionWrap(want, startOptions, callerToken, false, userId, requestCode, 0, true);
}

int AbilityManagerService::StartUIAbilityForOptionWrap(const Want &want, const StartOptions &options,
    sptr<IRemoteObject> callerToken, bool isPendingWantCaller, int32_t userId,
    int requestCode, uint32_t callerTokenId, bool isImplicit,
    bool isCallByShortcut)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto ret = CheckProcessOptions(want, options, userId);
    if (ret != ERR_OK) {
        return ret;
    }
    return StartAbilityForOptionWrap(want, options, callerToken, isPendingWantCaller, userId, requestCode, false,
        callerTokenId, isImplicit, isCallByShortcut);
}

int AbilityManagerService::StartAbilityAsCaller(const Want &want, const StartOptions &startOptions,
    const sptr<IRemoteObject> &callerToken, sptr<IRemoteObject> asCallerSourceToken,
    int32_t userId, int requestCode)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Start ability as caller with startOptions.");
    CHECK_CALLER_IS_SYSTEM_APP;

    AbilityUtil::RemoveWantKey(const_cast<Want &>(want));
    AAFwk::Want newWant = want;
    UpdateAsCallerSourceInfo(newWant, asCallerSourceToken, callerToken);
    return StartAbilityForOptionWrap(newWant, startOptions, callerToken, false, userId, requestCode, true);
}

int AbilityManagerService::StartAbilityForResultAsCaller(
    const Want &want, const sptr<IRemoteObject> &callerToken, int requestCode, int32_t userId)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "called");
    CHECK_CALLER_IS_SYSTEM_APP;

    AbilityUtil::RemoveShowModeKey(const_cast<Want &>(want));
    AAFwk::Want newWant = want;
    auto connectManager = GetCurrentConnectManager();
    CHECK_POINTER_AND_RETURN(connectManager, ERR_NO_INIT);
    auto asCallerSourceToken = connectManager->GetUIExtensionSourceToken(callerToken);
    UpdateAsCallerSourceInfo(newWant, asCallerSourceToken, callerToken);
    return StartAbilityWrap(newWant, callerToken, requestCode, false, userId, true);
}

int AbilityManagerService::StartAbilityForResultAsCaller(const Want &want, const StartOptions &startOptions,
    const sptr<IRemoteObject> &callerToken, int requestCode, int32_t userId)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "called");
    CHECK_CALLER_IS_SYSTEM_APP;

    AAFwk::Want newWant = want;
    auto connectManager = GetCurrentConnectManager();
    CHECK_POINTER_AND_RETURN(connectManager, ERR_NO_INIT);
    auto asCallerSourceToken = connectManager->GetUIExtensionSourceToken(callerToken);
    UpdateAsCallerSourceInfo(newWant, asCallerSourceToken, callerToken);
    return StartAbilityForOptionWrap(newWant, startOptions, callerToken, false, userId, requestCode, true);
}

int AbilityManagerService::StartAbilityForOptionWrap(const Want &want, const StartOptions &startOptions,
    const sptr<IRemoteObject> &callerToken, bool isPendingWantCaller, int32_t userId, int requestCode,
    bool isStartAsCaller, uint32_t callerTokenId, bool isImplicit, bool isCallByShortcut)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    StartAbilityParams startParams(const_cast<Want &>(want));
    startParams.callerToken = callerToken;
    startParams.userId = userId;
    startParams.requestCode = requestCode;
    startParams.isStartAsCaller = isStartAsCaller;
    startParams.startOptions = &startOptions;
    startParams.SetValidUserId(GetValidUserId(userId));

    int result = ERR_OK;
    if (StartAbilityInChain(startParams, result)) {
        return result;
    }

    return StartAbilityForOptionInner(want, startOptions, callerToken, isPendingWantCaller, userId, requestCode,
        isStartAsCaller, callerTokenId, isImplicit, isCallByShortcut);
}

int AbilityManagerService::StartAbilityForOptionInner(const Want &want, const StartOptions &startOptions,
    const sptr<IRemoteObject> &callerToken, bool isPendingWantCaller, int32_t userId, int requestCode,
    bool isStartAsCaller, uint32_t specifyTokenId, bool isImplicit, bool isCallByShortcut)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    // prevent the app from dominating the screen
    if (system::GetBoolParameter(VERIFY_DOMINATE_SCREEN, true) &&
        callerToken == nullptr && !IsCallerSceneBoard() &&
        AbilityPermissionUtil::GetInstance().IsDominateScreen(want, isPendingWantCaller)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "caller is invalid.");
        return ERR_INVALID_CALLER;
    }

    bool startWithAccount = want.GetBoolParam(START_ABILITY_TYPE, false);
    if (startWithAccount || IsCrossUserCall(userId)) {
        (const_cast<Want &>(want)).RemoveParam(START_ABILITY_TYPE);
        CHECK_CALLER_IS_SYSTEM_APP;
    }
    InsightIntentExecuteParam::RemoveInsightIntent(const_cast<Want &>(want));
    EventInfo eventInfo = BuildEventInfo(want, userId);
    SendAbilityEvent(EventName::START_ABILITY, HiSysEventType::BEHAVIOR, eventInfo);

#ifdef WITH_DLP
    if (!DlpUtils::OtherAppsAccessDlpCheck(callerToken, want) ||
        VerifyAccountPermission(userId) == CHECK_PERMISSION_FAILED ||
        !DlpUtils::DlpAccessOtherAppsCheck(callerToken, want)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "%{public}s: Permission verification failed", __func__);
        eventInfo.errCode = CHECK_PERMISSION_FAILED;
        SendAbilityEvent(EventName::START_ABILITY_ERROR, HiSysEventType::FAULT, eventInfo);
        return CHECK_PERMISSION_FAILED;
    }
#endif // WITH_DLP

    if (callerToken != nullptr && !VerificationAllToken(callerToken)) {
        eventInfo.errCode = ERR_INVALID_VALUE;
        SendAbilityEvent(EventName::START_ABILITY_ERROR, HiSysEventType::FAULT, eventInfo);
        return ERR_INVALID_CALLER;
    }

    int32_t oriValidUserId = GetValidUserId(userId);
    int32_t validUserId = oriValidUserId;
    int32_t appIndex = 0;
    if (!StartAbilityUtils::GetAppIndex(want, callerToken, appIndex)) {
        return ERR_APP_CLONE_INDEX_INVALID;
    }
    StartAbilityInfoWrap threadLocalInfo(want, validUserId, appIndex, callerToken);
    AbilityInterceptorParam interceptorParam = AbilityInterceptorParam(want, requestCode, GetUserId(),
        true, nullptr);
    auto result = interceptorExecuter_ == nullptr ? ERR_INVALID_VALUE :
        interceptorExecuter_->DoProcess(interceptorParam);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "interceptorExecuter_ is nullptr or DoProcess return error.");
        eventInfo.errCode = result;
        SendAbilityEvent(EventName::START_ABILITY_ERROR, HiSysEventType::FAULT, eventInfo);
        return result;
    }

    if (AbilityUtil::IsStartFreeInstall(want)) {
        if (CheckIfOperateRemote(want) || freeInstallManager_ == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "can not start remote free install");
            return ERR_INVALID_VALUE;
        }
        (const_cast<Want &>(want)).RemoveParam("send_to_erms_embedded");
        Want localWant = want;
        if (!isStartAsCaller) {
            TAG_LOGD(AAFwkTag::ABILITYMGR, "do not start as caller, UpdateCallerInfo");
            UpdateCallerInfo(localWant, callerToken);
        }
        return freeInstallManager_->StartFreeInstall(localWant, validUserId, requestCode,
            callerToken, true, specifyTokenId);
    }
    if (!JudgeMultiUserConcurrency(validUserId)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Multi-user non-concurrent mode is not satisfied.");
        eventInfo.errCode = ERR_INVALID_VALUE;
        SendAbilityEvent(EventName::START_ABILITY_ERROR, HiSysEventType::FAULT, eventInfo);
        return ERR_CROSS_USER;
    }

    AbilityRequest abilityRequest;
#ifdef SUPPORT_SCREEN
    if (ImplicitStartProcessor::IsImplicitStartAction(want)) {
        abilityRequest.Voluation(want, requestCode, callerToken);
        if (PermissionVerification::GetInstance()->IsSystemAppCall()) {
            bool windowFocused = startOptions.GetWindowFocused();
            abilityRequest.want.SetParam(Want::PARAM_RESV_WINDOW_FOCUSED, windowFocused);
        } else {
            abilityRequest.want.RemoveParam(Want::PARAM_RESV_WINDOW_FOCUSED);
        }
        if (startOptions.GetDisplayID() == 0) {
            abilityRequest.want.SetParam(Want::PARAM_RESV_DISPLAY_ID,
                static_cast<int32_t>(Rosen::DisplayManager::GetInstance().GetDefaultDisplayId()));
        } else {
            abilityRequest.want.SetParam(Want::PARAM_RESV_DISPLAY_ID, startOptions.GetDisplayID());
        }
        WindowOptionsUtils::SetWindowPositionAndSize(abilityRequest.want, callerToken, startOptions);
        abilityRequest.callType = AbilityCallType::START_OPTIONS_TYPE;
        CHECK_POINTER_AND_RETURN(implicitStartProcessor_, ERR_IMPLICIT_START_ABILITY_FAIL);
        if (specifyTokenId > 0 && callerToken) { // for sa specify tokenId and caller token
            UpdateCallerInfoFromToken(abilityRequest.want, callerToken);
        } else if (!isStartAsCaller) {
            TAG_LOGD(AAFwkTag::ABILITYMGR, "do not start as caller, UpdateCallerInfo");
            UpdateCallerInfo(abilityRequest.want, callerToken);
        }
        result = implicitStartProcessor_->ImplicitStartAbility(abilityRequest, validUserId,
            startOptions.GetWindowMode());
        if (result != ERR_OK) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "implicit start ability error.");
            eventInfo.errCode = result;
            SendAbilityEvent(EventName::START_ABILITY_ERROR, HiSysEventType::FAULT, eventInfo);
        }
        return result;
    }
    if (want.GetAction().compare(ACTION_CHOOSE) == 0) {
        return ShowPickerDialog(want, validUserId, callerToken);
    }
#endif
    result = GenerateAbilityRequest(want, requestCode, abilityRequest, callerToken, validUserId);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Generate ability request local error.");
        eventInfo.errCode = result;
        SendAbilityEvent(EventName::START_ABILITY_ERROR, HiSysEventType::FAULT, eventInfo);
        return result;
    }
    if (!UriUtils::GetInstance().CheckNonImplicitShareFileUri(abilityRequest)) {
        return ERR_SHARE_FILE_URI_NON_IMPLICITLY;
    }

    if (!isStartAsCaller) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "do not start as caller, UpdateCallerInfo");
        UpdateCallerInfo(abilityRequest.want, callerToken);
    }
    auto abilityInfo = abilityRequest.abilityInfo;
    validUserId = abilityInfo.applicationInfo.singleton ? U0_USER_ID : validUserId;
    TAG_LOGD(AAFwkTag::ABILITYMGR, "userId : %{public}d, singleton is : %{public}d",
        validUserId, static_cast<int>(abilityInfo.applicationInfo.singleton));

    result = CheckStaticCfgPermission(abilityRequest, isStartAsCaller,
        abilityRequest.want.GetIntParam(Want::PARAM_RESV_CALLER_TOKEN, 0), false, false, isImplicit);
    if (result != AppExecFwk::Constants::PERMISSION_GRANTED) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "CheckStaticCfgPermission error, result is %{public}d.", result);
        eventInfo.errCode = result;
        SendAbilityEvent(EventName::START_ABILITY_ERROR, HiSysEventType::FAULT, eventInfo);
        return ERR_STATIC_CFG_PERMISSION;
    }
    result = CheckCallAbilityPermission(abilityRequest, 0, isCallByShortcut);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "%{public}s CheckCallAbilityPermission error.", __func__);
        eventInfo.errCode = result;
        SendAbilityEvent(EventName::START_ABILITY_ERROR, HiSysEventType::FAULT, eventInfo);
        return result;
    }

    if (abilityInfo.type != AppExecFwk::AbilityType::PAGE) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Only support for page type ability.");
        eventInfo.errCode = ERR_INVALID_VALUE;
        SendAbilityEvent(EventName::START_ABILITY_ERROR, HiSysEventType::FAULT, eventInfo);
        return ERR_INVALID_VALUE;
    }

    if (!AbilityUtil::IsSystemDialogAbility(abilityInfo.bundleName, abilityInfo.name)) {
        result = PreLoadAppDataAbilities(abilityInfo.bundleName, validUserId);
        if (result != ERR_OK) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "StartAbility: App data ability preloading failed, '%{public}s', %{public}d",
                abilityInfo.bundleName.c_str(),
                result);
            eventInfo.errCode = result;
            SendAbilityEvent(EventName::START_ABILITY_ERROR, HiSysEventType::FAULT, eventInfo);
            return result;
        }
    }

    if (!IsAbilityControllerStart(want, abilityInfo.bundleName)) {
        eventInfo.errCode = ERR_WOULD_BLOCK;
        SendAbilityEvent(EventName::START_ABILITY_ERROR, HiSysEventType::FAULT, eventInfo);
        return ERR_WOULD_BLOCK;
    }
#ifdef SUPPORT_SCREEN
    if (abilityInfo.isStageBasedModel && !CheckWindowMode(startOptions.GetWindowMode(), abilityInfo.windowModes)) {
        return ERR_AAFWK_INVALID_WINDOW_MODE;
    }
#endif
    if (startOptions.GetDisplayID() == 0) {
        abilityRequest.want.SetParam(Want::PARAM_RESV_DISPLAY_ID,
            static_cast<int32_t>(Rosen::DisplayManager::GetInstance().GetDefaultDisplayId()));
    } else {
        abilityRequest.want.SetParam(Want::PARAM_RESV_DISPLAY_ID, startOptions.GetDisplayID());
    }
    AbilityUtil::ProcessWindowMode(abilityRequest.want, abilityInfo.applicationInfo.accessTokenId,
        startOptions.GetWindowMode());

    WindowOptionsUtils::SetWindowPositionAndSize(abilityRequest.want, callerToken, startOptions);

    if (PermissionVerification::GetInstance()->IsSystemAppCall()) {
        bool focused = abilityRequest.want.GetBoolParam(Want::PARAM_RESV_WINDOW_FOCUSED, true);
        if (focused) {
            bool windowfocused = startOptions.GetWindowFocused();
            abilityRequest.want.SetParam(Want::PARAM_RESV_WINDOW_FOCUSED, windowfocused);
        }
    } else {
        abilityRequest.want.RemoveParam(Want::PARAM_RESV_WINDOW_FOCUSED);
    }

    Want newWant = abilityRequest.want;
    AbilityInterceptorParam afterCheckParam = AbilityInterceptorParam(newWant, requestCode, GetUserId(),
        true, callerToken, std::make_shared<AppExecFwk::AbilityInfo>(abilityInfo), isStartAsCaller);
    result = afterCheckExecuter_ == nullptr ? ERR_INVALID_VALUE :
        afterCheckExecuter_->DoProcess(afterCheckParam);
    bool isReplaceWantExist = newWant.GetBoolParam("queryWantFromErms", false);
    newWant.RemoveParam("queryWantFromErms");
    if (result != ERR_OK && isReplaceWantExist == false) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "DoProcess failed or replaceWant not exist");
        return result;
    }
#ifdef SUPPORT_SCREEN
    if (result != ERR_OK && isReplaceWantExist) {
        return DialogSessionManager::GetInstance().HandleErmsResult(abilityRequest, GetUserId(), newWant);
    }
    if (result == ERR_OK &&
        DialogSessionManager::GetInstance().IsCreateCloneSelectorDialog(abilityInfo.bundleName, GetUserId())) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "create clone selector dialog");
        return CreateCloneSelectorDialog(abilityRequest, GetUserId());
    }
#endif // SUPPORT_GRAPHICS
    abilityRequest.want.RemoveParam(SPECIFY_TOKEN_ID);
    if (specifyTokenId > 0) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "Set specifyTokenId, the specifyTokenId is %{public}d.", specifyTokenId);
        abilityRequest.want.SetParam(SPECIFY_TOKEN_ID, static_cast<int32_t>(specifyTokenId));
        abilityRequest.specifyTokenId = specifyTokenId;
    }
    abilityRequest.want.RemoveParam(PARAM_SPECIFIED_PROCESS_FLAG);
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        abilityRequest.userId = oriValidUserId;
        abilityRequest.want.SetParam(IS_CALL_BY_SCB, false);
        abilityRequest.processOptions = startOptions.processOptions;
        auto uiAbilityManager = GetUIAbilityManagerByUserId(oriValidUserId);
        CHECK_POINTER_AND_RETURN(uiAbilityManager, ERR_INVALID_VALUE);
        return uiAbilityManager->NotifySCBToStartUIAbility(abilityRequest);
    }
    auto missionListManager = GetMissionListManagerByUserId(oriValidUserId);
    if (missionListManager == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "missionListManager is Null. userId=%{public}d", oriValidUserId);
        eventInfo.errCode = ERR_INVALID_VALUE;
        SendAbilityEvent(EventName::START_ABILITY_ERROR, HiSysEventType::FAULT, eventInfo);
        return ERR_INVALID_VALUE;
    }

    auto ret = missionListManager->StartAbility(abilityRequest);
    if (ret != ERR_OK) {
        eventInfo.errCode = ret;
        SendAbilityEvent(EventName::START_ABILITY_ERROR, HiSysEventType::FAULT, eventInfo);
    }
    return ret;
}

int32_t AbilityManagerService::RequestDialogService(const Want &want, const sptr<IRemoteObject> &callerToken)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto flags = want.GetFlags();
    if ((flags & Want::FLAG_ABILITY_CONTINUATION) == Want::FLAG_ABILITY_CONTINUATION) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "RequestDialogService with continuation flags is not allowed!");
        return ERR_INVALID_CONTINUATION_FLAG;
    }

    TAG_LOGI(AAFwkTag::ABILITYMGR, "request dialog service, target is %{public}s", want.GetElement().GetURI().c_str());
    return RequestDialogServiceInner(want, callerToken, -1, -1);
}

int32_t AbilityManagerService::ReportDrawnCompleted(const sptr<IRemoteObject> &callerToken)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "called");
    if (callerToken == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "callerToken is nullptr");
        return INNER_ERR;
    }

    auto abilityRecord = Token::GetAbilityRecordByToken(callerToken);
    if (abilityRecord == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "abilityRecord is nullptr");
        return INNER_ERR;
    }
    auto abilityInfo = abilityRecord->GetAbilityInfo();

    EventInfo eventInfo;
    eventInfo.userId = IPCSkeleton::GetCallingUid() / BASE_USER_RANGE;
    eventInfo.pid = IPCSkeleton::GetCallingPid();
    eventInfo.bundleName = abilityInfo.bundleName;
    eventInfo.moduleName = abilityInfo.moduleName;
    eventInfo.abilityName = abilityInfo.name;
    EventReport::SendAppEvent(EventName::DRAWN_COMPLETED, HiSysEventType::BEHAVIOR, eventInfo);
    return ERR_OK;
}

int32_t AbilityManagerService::RequestDialogServiceInner(const Want &want, const sptr<IRemoteObject> &callerToken,
    int requestCode, int32_t userId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (callerToken == nullptr || !VerificationAllToken(callerToken)) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "caller is invalid.");
        return ERR_INVALID_CALLER;
    }

    {
#ifdef WITH_DLP
        HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, "CHECK_DLP");
        if (!DlpUtils::OtherAppsAccessDlpCheck(callerToken, want) ||
            !DlpUtils::DlpAccessOtherAppsCheck(callerToken, want)) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "%{public}s: Permission verification failed.", __func__);
            return CHECK_PERMISSION_FAILED;
        }

        if (AbilityUtil::HandleDlpApp(const_cast<Want &>(want))) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "Cannot handle dlp by RequestDialogService.");
            return ERR_WRONG_INTERFACE_CALL;
        }
#endif // WITH_DLP
    }

    AbilityUtil::RemoveShowModeKey(const_cast<Want &>(want));
    int32_t validUserId = GetValidUserId(userId);
    AbilityInterceptorParam interceptorParam = AbilityInterceptorParam(want, requestCode, GetUserId(),
        true, nullptr);
    auto result = interceptorExecuter_ == nullptr ? ERR_INVALID_VALUE :
        interceptorExecuter_->DoProcess(interceptorParam);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "interceptorExecuter_ is nullptr or DoProcess return error.");
        return result;
    }

    if (!JudgeMultiUserConcurrency(validUserId)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Multi-user non-concurrent mode is not satisfied.");
        return ERR_CROSS_USER;
    }
    AbilityRequest abilityRequest;
    result = GenerateExtensionAbilityRequest(want, abilityRequest, callerToken, validUserId);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Generate ability request local error when RequestDialogService.");
        return result;
    }
    UpdateCallerInfo(abilityRequest.want, callerToken);

    auto abilityInfo = abilityRequest.abilityInfo;
    validUserId = abilityInfo.applicationInfo.singleton ? U0_USER_ID : validUserId;
    TAG_LOGD(AAFwkTag::ABILITYMGR, "userId is : %{public}d, singleton is : %{public}d",
        validUserId, static_cast<int>(abilityInfo.applicationInfo.singleton));

    result = CheckStaticCfgPermission(abilityRequest, false, -1);
    if (result != AppExecFwk::Constants::PERMISSION_GRANTED) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "CheckStaticCfgPermission error, result is %{public}d.", result);
        return ERR_STATIC_CFG_PERMISSION;
    }

    auto type = abilityInfo.type;
    if (type == AppExecFwk::AbilityType::EXTENSION &&
        abilityInfo.extensionAbilityType == AppExecFwk::ExtensionAbilityType::SERVICE) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "Check call ability permission, name is %{public}s.", abilityInfo.name.c_str());
        result = CheckCallServicePermission(abilityRequest);
        if (result != ERR_OK) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "Check permission failed");
            return result;
        }
    } else {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "RequestDialogService do not support other component.");
        return ERR_WRONG_INTERFACE_CALL;
    }

    AbilityInterceptorParam afterCheckParam = AbilityInterceptorParam(abilityRequest.want, requestCode,
        GetUserId(), true, callerToken, std::make_shared<AppExecFwk::AbilityInfo>(abilityInfo));
    result = afterCheckExecuter_ == nullptr ? ERR_INVALID_VALUE :
        afterCheckExecuter_->DoProcess(afterCheckParam);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "afterCheckExecuter_ is nullptr or DoProcess return error.");
        return result;
    }

    auto connectManager = GetConnectManagerByUserId(validUserId);
    if (!connectManager) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "connectManager is nullptr. userId=%{public}d", validUserId);
        return ERR_INVALID_VALUE;
    }
    TAG_LOGD(AAFwkTag::ABILITYMGR,
        "request dialog service, start service extension,name is %{public}s.", abilityInfo.name.c_str());
    ReportEventToRSS(abilityInfo, callerToken);
    return connectManager->StartAbility(abilityRequest);
}

int32_t AbilityManagerService::OpenAtomicService(AAFwk::Want& want, const StartOptions &options,
    sptr<IRemoteObject> callerToken, int32_t requestCode, int32_t userId)
{
    auto accessTokenId = IPCSkeleton::GetCallingTokenID();
    auto type = Security::AccessToken::AccessTokenKit::GetTokenTypeFlag(accessTokenId);
    if (type != Security::AccessToken::TypeATokenTypeEnum::TOKEN_HAP) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "The caller is not hap.");
        return CHECK_PERMISSION_FAILED;
    }
    want.SetParam(AAFwk::SCREEN_MODE_KEY, AAFwk::ScreenMode::JUMP_SCREEN_MODE);
    return StartUIAbilityForOptionWrap(want, options, callerToken, false, userId, requestCode);
}

int AbilityManagerService::StartUIAbilityBySCB(sptr<SessionInfo> sessionInfo, bool &isColdStart)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Call.");
    if (sessionInfo == nullptr || sessionInfo->sessionToken == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "sessionInfo is nullptr");
        return ERR_INVALID_VALUE;
    }

    if (!IsCallerSceneBoard()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Not sceneboard called, not allowed.");
        return ERR_WRONG_INTERFACE_CALL;
    }

    if (!(sessionInfo->want).HasParameter(KEY_SESSION_ID)) {
        return StartUIAbilityBySCBDefault(sessionInfo, isColdStart);
    }

    std::string sessionId = (sessionInfo->want).GetStringParam(KEY_SESSION_ID);
    if (sessionId.empty()) {
        return StartUIAbilityBySCBDefault(sessionInfo, isColdStart);
    }

    TAG_LOGI(AAFwkTag::ABILITYMGR, "sessionId=%{public}s", sessionId.c_str());

    if (freeInstallManager_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "freeInstallManager_ is nullptr.");
        return ERR_INVALID_VALUE;
    }
    FreeInstallInfo taskInfo;
    if (!freeInstallManager_->GetFreeInstallTaskInfo(sessionId, taskInfo)) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "failed to find free install task");
        if (sessionInfo->isAtomicService &&
            ((sessionInfo->want).GetElement().GetAbilityName().empty() ||
            (sessionInfo->want).GetElement().GetModuleName().empty())) {
            auto bundleMgrHelper = AbilityUtil::GetBundleManagerHelper();
            CHECK_POINTER_AND_RETURN(bundleMgrHelper, ERR_INVALID_VALUE);
            Want launchWant;
            auto errCode = IN_PROCESS_CALL(bundleMgrHelper->GetLaunchWantForBundle(
                (sessionInfo->want).GetBundle(), launchWant, sessionInfo->userId));
            if (errCode != ERR_OK) {
                TAG_LOGE(AAFwkTag::ABILITYMGR, "GetLaunchWantForBundle returns %{public}d.", errCode);
                return errCode;
            }
            (sessionInfo->want).SetElement(launchWant.GetElement());
        }
        return StartUIAbilityBySCBDefault(sessionInfo, isColdStart);
    }

    if (taskInfo.isFreeInstallFinished) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "free install task is already finished");
        if (!taskInfo.isInstalled) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "free install task failed,resultCode=%{public}d",
                taskInfo.resultCode);
            return taskInfo.resultCode;
        }
        TAG_LOGI(AAFwkTag::ABILITYMGR, "free install succeeds");
        auto err = StartUIAbilityByPreInstallInner(sessionInfo, taskInfo.specifyTokenId, isColdStart);
        if (err != ERR_OK) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "StartUIAbilityByPreInstallInner failed.");
        }
        return err;
    }

    {
        std::lock_guard<ffrt::mutex> guard(preStartSessionMapLock_);
        preStartSessionMap_.insert(std::make_pair(sessionId, sessionInfo));
    }

    TAG_LOGI(AAFwkTag::ABILITYMGR, "free install task is still in progress");
    const Want& want = sessionInfo->want;
    freeInstallManager_->SetSCBCallStatus(want.GetElement().GetBundleName(), want.GetElement().GetAbilityName(),
        want.GetStringParam(Want::PARAM_RESV_START_TIME), true);
    return ERR_OK;
}

int AbilityManagerService::StartUIAbilityBySCBDefault(sptr<SessionInfo> sessionInfo, bool &isColdStart)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Call.");

    auto currentUserId = IPCSkeleton::GetCallingUid() / BASE_USER_RANGE;
    if (sessionInfo->userId == DEFAULT_INVAL_VALUE) {
        sessionInfo->userId = currentUserId;
    }

    (sessionInfo->want).RemoveParam(AAFwk::SCREEN_MODE_KEY);
    EventInfo eventInfo = BuildEventInfo(sessionInfo->want, currentUserId);
    SendAbilityEvent(EventName::START_ABILITY, HiSysEventType::BEHAVIOR, eventInfo);

    auto requestCode = sessionInfo->requestCode;
    int32_t appIndex = 0;
    if (!StartAbilityUtils::GetAppIndex(sessionInfo->want, sessionInfo->callerToken, appIndex)) {
        return ERR_APP_CLONE_INDEX_INVALID;
    }
    StartAbilityInfoWrap threadLocalInfo(sessionInfo->want, currentUserId, appIndex, sessionInfo->callerToken);
    if (sessionInfo->want.GetBoolParam(IS_CALL_BY_SCB, true)) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "interceptorExecuter_ called");
        AbilityInterceptorParam interceptorParam = AbilityInterceptorParam(sessionInfo->want, requestCode,
            currentUserId, true, nullptr);
        auto result = interceptorExecuter_ == nullptr ? ERR_INVALID_VALUE :
        interceptorExecuter_->DoProcess(interceptorParam);
        if (result != ERR_OK) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "interceptorExecuter_ is nullptr or DoProcess return error.");
            eventInfo.errCode = result;
            SendAbilityEvent(EventName::START_ABILITY_ERROR, HiSysEventType::FAULT, eventInfo);
            return result;
        }
    }

    AbilityRequest abilityRequest;
    auto result = GenerateAbilityRequest(sessionInfo->want, requestCode, abilityRequest,
        sessionInfo->callerToken, currentUserId);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Generate ability request local error.");
        return result;
    }

    if (sessionInfo->want.GetBoolParam(IS_CALL_BY_SCB, true)) {
        if (sessionInfo->startSetting != nullptr) {
            TAG_LOGD(AAFwkTag::ABILITYMGR, "Start by scb, last not.");
            sessionInfo->startSetting->AddProperty(AbilityStartSetting::IS_START_BY_SCB_KEY, "true");
        }

        if (abilityRequest.startSetting != nullptr) {
            TAG_LOGD(AAFwkTag::ABILITYMGR, "Start by scb.");
            abilityRequest.startSetting->AddProperty(AbilityStartSetting::IS_START_BY_SCB_KEY, "true");
        }
    }

    abilityRequest.collaboratorType = sessionInfo->collaboratorType;
    uint32_t specifyTokenId = static_cast<uint32_t>(sessionInfo->want.GetIntParam(SPECIFY_TOKEN_ID, 0));
    (sessionInfo->want).RemoveParam(SPECIFY_TOKEN_ID);
    abilityRequest.specifyTokenId = specifyTokenId;

    auto abilityInfo = abilityRequest.abilityInfo;
    if (!AAFwk::PermissionVerification::GetInstance()->IsSystemAppCall() &&
        abilityInfo.type != AppExecFwk::AbilityType::PAGE) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Only support for page type ability.");
        return ERR_INVALID_VALUE;
    }

    if (sessionInfo->want.GetBoolParam(IS_CALL_BY_SCB, true)) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "afterCheckExecuter_ called");
        Want newWant = abilityRequest.want;
        AbilityInterceptorParam afterCheckParam = AbilityInterceptorParam(newWant, requestCode,
            GetUserId(), true, sessionInfo->callerToken, std::make_shared<AppExecFwk::AbilityInfo>(abilityInfo));
        result = afterCheckExecuter_ == nullptr ? ERR_INVALID_VALUE :
            afterCheckExecuter_->DoProcess(afterCheckParam);
        bool isReplaceWantExist = newWant.GetBoolParam("queryWantFromErms", false);
        newWant.RemoveParam("queryWantFromErms");
        if (result != ERR_OK) {
            if (isReplaceWantExist == false) {
                TAG_LOGE(AAFwkTag::ABILITYMGR, "DoProcess failed or replaceWant not exist");
                return result;
            }
            auto systemUIExtension = std::make_shared<OHOS::Rosen::ModalSystemUiExtension>();
            (const_cast<Want &>(newWant)).SetParam(UIEXTENSION_MODAL_TYPE, 1);
            (const_cast<Want &>(newWant)).SetParam(SUPPORT_CLOSE_ON_BLUR, true);
            return IN_PROCESS_CALL(systemUIExtension->CreateModalUIExtension(newWant)) ?
                ERR_ECOLOGICAL_CONTROL_STATUS : INNER_ERR;
        }
    }

    if (!AbilityUtil::IsSystemDialogAbility(abilityInfo.bundleName, abilityInfo.name)) {
        result = PreLoadAppDataAbilities(abilityInfo.bundleName, currentUserId);
        if (result != ERR_OK) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "StartAbility: App data ability preloading failed, '%{public}s', %{public}d",
                abilityInfo.bundleName.c_str(), result);
            return result;
        }
    }

    ReportAbilitStartInfoToRSS(abilityInfo);
    ReportAbilitAssociatedStartInfoToRSS(abilityInfo, RES_TYPE_SCB_START_ABILITY, sessionInfo->callerToken);
    auto uiAbilityManager = GetUIAbilityManagerByUid(IPCSkeleton::GetCallingUid());
    CHECK_POINTER_AND_RETURN(uiAbilityManager, ERR_INVALID_VALUE);
    // here we don't need want param "IS_CALL_BY_SCB" any more, remove it.
    (sessionInfo->want).RemoveParam(IS_CALL_BY_SCB);
    return uiAbilityManager->StartUIAbility(abilityRequest, sessionInfo, isColdStart);
}

bool AbilityManagerService::CheckCallingTokenId(const std::string &bundleName, int32_t userId, int32_t appIndex)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto bundleMgrHelper = DelayedSingleton<AppExecFwk::BundleMgrHelper>::GetInstance();
    CHECK_POINTER_AND_RETURN(bundleMgrHelper, false);
    auto validUserId = GetValidUserId(userId);
    AppExecFwk::ApplicationInfo appInfo;
    IN_PROCESS_CALL_WITHOUT_RET(bundleMgrHelper->GetApplicationInfoWithAppIndex(bundleName,
        appIndex, validUserId, appInfo));
    auto accessTokenId = IPCSkeleton::GetCallingTokenID();
    if (accessTokenId != appInfo.accessTokenId) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Permission verification failed");
        return false;
    }
    return true;
}

bool AbilityManagerService::IsCallerSceneBoard()
{
    int32_t userId = IPCSkeleton::GetCallingUid() / BASE_USER_RANGE;
    auto connectManager = GetConnectManagerByUserId(userId);
    CHECK_POINTER_AND_RETURN(connectManager, false);
    auto sceneBoardTokenId = connectManager->GetSceneBoardTokenId();
    if (sceneBoardTokenId != 0 && IPCSkeleton::GetCallingTokenID() == sceneBoardTokenId) {
        return true;
    }
    return false;
}

bool AbilityManagerService::IsBackgroundTaskUid(const int uid)
{
#ifdef BGTASKMGR_CONTINUOUS_TASK_ENABLE
    std::lock_guard<ffrt::mutex> lock(bgtaskObserverMutex_);
    if (bgtaskObserver_) {
        return bgtaskObserver_->IsBackgroundTaskUid(uid);
    }
    return false;
#else
    return false;
#endif
}

bool AbilityManagerService::IsDmsAlive() const
{
    return g_isDmsAlive.load();
}

void AbilityManagerService::AppUpgradeCompleted(const std::string &bundleName, int32_t uid)
{
    if (!AAFwk::PermissionVerification::GetInstance()->IsSACall()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Not sa call");
        return;
    }

    auto bms = GetBundleManager();
    CHECK_POINTER(bms);
    auto userId = uid / BASE_USER_RANGE;

    AppExecFwk::BundleInfo bundleInfo;
    if (!IN_PROCESS_CALL(
        bms->GetBundleInfo(bundleName, AppExecFwk::BundleFlag::GET_BUNDLE_WITH_ABILITIES, bundleInfo, userId))) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Failed to get bundle info.");
        return;
    }

    if (userId != U0_USER_ID) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Application upgrade for non U0 users.");
        return;
    }

    bool keepAliveEnable = bundleInfo.isKeepAlive;
    AmsResidentProcessRdb::GetInstance().GetResidentProcessEnable(bundleInfo.name, keepAliveEnable);
    if (!keepAliveEnable) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "Not a resident application.");
        return;
    }

    std::vector<AppExecFwk::BundleInfo> bundleInfos = { bundleInfo };
    DelayedSingleton<ResidentProcessManager>::GetInstance()->StartResidentProcessWithMainElement(bundleInfos);

    if (!bundleInfos.empty()) {
        DelayedSingleton<ResidentProcessManager>::GetInstance()->StartResidentProcess(bundleInfos);
    }
}

int32_t AbilityManagerService::RecordAppExitReason(const ExitReason &exitReason)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "RecordAppExitReason reason:%{public}d, exitMsg: %{public}s", exitReason.reason,
        exitReason.exitMsg.c_str());

    CHECK_POINTER_AND_RETURN(appExitReasonHelper_, ERR_NULL_OBJECT);
    return appExitReasonHelper_->RecordAppExitReason(exitReason);
}

int32_t AbilityManagerService::RecordProcessExitReason(const int32_t pid, const ExitReason &exitReason)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "RecordProcessExitReason pid:%{public}d, reason:%{public}d, exitMsg: %{public}s",
        pid, exitReason.reason, exitReason.exitMsg.c_str());

    if (!AAFwk::PermissionVerification::GetInstance()->IsSACall() &&
        !AAFwk::PermissionVerification::GetInstance()->IsShellCall()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Not sa call");
        return ERR_PERMISSION_DENIED;
    }

    CHECK_POINTER_AND_RETURN(appExitReasonHelper_, ERR_NULL_OBJECT);
    return appExitReasonHelper_->RecordProcessExitReason(pid, exitReason);
}

int32_t AbilityManagerService::ForceExitApp(const int32_t pid, const ExitReason &exitReason)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "ForceExitApp pid:%{public}d, reason:%{public}d, exitMsg: %{public}s",
        pid, exitReason.reason, exitReason.exitMsg.c_str());

    if (!AAFwk::PermissionVerification::GetInstance()->IsSACall() &&
        !AAFwk::PermissionVerification::GetInstance()->IsShellCall()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Not sa or shell call");
        return ERR_PERMISSION_DENIED;
    }

    AppExecFwk::ApplicationInfo application;
    bool debug = false;
    auto ret = IN_PROCESS_CALL(DelayedSingleton<AppScheduler>::GetInstance()->GetApplicationInfoByProcessID(pid,
        application, debug));
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "GetApplicationInfoByProcessID failed.");
        return ret;
    }

    std::string bundleName = application.bundleName;
    int32_t uid = application.uid;
    int32_t appIndex = application.appIndex;

    CHECK_POINTER_AND_RETURN(appExitReasonHelper_, ERR_NULL_OBJECT);
    appExitReasonHelper_->RecordAppExitReason(bundleName, uid, appIndex, exitReason);

    return DelayedSingleton<AppScheduler>::GetInstance()->KillApplication(bundleName);
}

int32_t AbilityManagerService::GetConfiguration(AppExecFwk::Configuration& config)
{
    auto appMgr = GetAppMgr();
    if (appMgr == nullptr) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "GetAppMgr failed");
        return -1;
    }

    return appMgr->GetConfiguration(config);
}

OHOS::sptr<OHOS::AppExecFwk::IAppMgr> AbilityManagerService::GetAppMgr()
{
    if (appMgr_) {
        return appMgr_;
    }

    OHOS::sptr<OHOS::ISystemAbilityManager> systemAbilityManager =
        OHOS::SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (!systemAbilityManager) {
        return nullptr;
    }
    OHOS::sptr<OHOS::IRemoteObject> object = systemAbilityManager->GetSystemAbility(OHOS::APP_MGR_SERVICE_ID);
    appMgr_ = OHOS::iface_cast<OHOS::AppExecFwk::IAppMgr>(object);
    return appMgr_;
}

int AbilityManagerService::CheckOptExtensionAbility(const Want &want, AbilityRequest &abilityRequest,
    int32_t validUserId, AppExecFwk::ExtensionAbilityType extensionType, bool isImplicit)
{
    auto abilityInfo = abilityRequest.abilityInfo;
    auto type = abilityInfo.type;
    if (type != AppExecFwk::AbilityType::EXTENSION) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Not extension ability, not allowed.");
        return ERR_WRONG_INTERFACE_CALL;
    }
    if (extensionType != AppExecFwk::ExtensionAbilityType::UNSPECIFIED &&
        extensionType != abilityInfo.extensionAbilityType) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Extension ability type not match, set type: %{public}d, real type: %{public}d",
            static_cast<int32_t>(extensionType), static_cast<int32_t>(abilityInfo.extensionAbilityType));
        return ERR_WRONG_INTERFACE_CALL;
    }

    auto result = CheckStaticCfgPermission(abilityRequest, false, -1, false, false, isImplicit);
    if (result != AppExecFwk::Constants::PERMISSION_GRANTED) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "CheckStaticCfgPermission error, result is %{public}d.", result);
        return ERR_STATIC_CFG_PERMISSION;
    }

    if (abilityInfo.extensionAbilityType == AppExecFwk::ExtensionAbilityType::DATASHARE ||
        abilityInfo.extensionAbilityType == AppExecFwk::ExtensionAbilityType::SERVICE) {
        result = CheckCallServiceExtensionPermission(abilityRequest);
        if (result != ERR_OK) {
            return result;
        }
    } else {
        result = CheckCallOtherExtensionPermission(abilityRequest);
        if (result != ERR_OK) {
            return result;
        }
    }

    UpdateCallerInfo(abilityRequest.want, abilityRequest.callerToken);
    return ERR_OK;
}

void AbilityManagerService::OnAddSystemAbility(int32_t systemAbilityId, const std::string& deviceId)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "systemAbilityId: %{public}d add", systemAbilityId);
    switch (systemAbilityId) {
        case BACKGROUND_TASK_MANAGER_SERVICE_ID: {
            SubscribeBackgroundTask();
            break;
        }
        case DISTRIBUTED_SCHED_SA_ID: {
            g_isDmsAlive.store(true);
            break;
        }
        case BUNDLE_MGR_SERVICE_SYS_ABILITY_ID: {
            SubscribeBundleEventCallback();
            break;
        }
#ifdef SUPPORT_SCREEN
        case MULTIMODAL_INPUT_SERVICE_ID: {
            auto anrListener = std::make_shared<ApplicationAnrListener>();
            MMI::InputManager::GetInstance()->SetAnrObserver(anrListener);
            break;
        }
#endif
        default:
            break;
    }
}

void AbilityManagerService::OnRemoveSystemAbility(int32_t systemAbilityId, const std::string& deviceId)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "systemAbilityId: %{public}d remove", systemAbilityId);
    switch (systemAbilityId) {
        case BACKGROUND_TASK_MANAGER_SERVICE_ID: {
            UnSubscribeBackgroundTask();
            break;
        }
        case DISTRIBUTED_SCHED_SA_ID: {
            g_isDmsAlive.store(false);
            break;
        }
        case BUNDLE_MGR_SERVICE_SYS_ABILITY_ID: {
            UnsubscribeBundleEventCallback();
            break;
        }
        default:
            break;
    }
}

void AbilityManagerService::SubscribeBackgroundTask()
{
#ifdef BGTASKMGR_CONTINUOUS_TASK_ENABLE
    std::unique_lock<ffrt::mutex> lock(bgtaskObserverMutex_);
    if (!bgtaskObserver_) {
        bgtaskObserver_ = std::make_shared<BackgroundTaskObserver>();
    }
    int ret = BackgroundTaskMgrHelper::SubscribeBackgroundTask(*bgtaskObserver_);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "%{public}s failed, err:%{public}d.", __func__, ret);
        return;
    }
    bgtaskObserver_->GetContinuousTaskApps();
    TAG_LOGI(AAFwkTag::ABILITYMGR, "%{public}s success.", __func__);
#endif
}

void AbilityManagerService::UnSubscribeBackgroundTask()
{
#ifdef BGTASKMGR_CONTINUOUS_TASK_ENABLE
    std::unique_lock<ffrt::mutex> lock(bgtaskObserverMutex_);
    if (!bgtaskObserver_) {
        return;
    }
    TAG_LOGI(AAFwkTag::ABILITYMGR, "%{public}s success.", __func__);
#endif
}

void AbilityManagerService::SubscribeBundleEventCallback()
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "SubscribeBundleEventCallback begin.");
    if (taskHandler_) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "submit StartResidentApps task.");
        auto startResidentAppsTask = [aams = shared_from_this()]() { aams->StartResidentApps(); };
        taskHandler_->SubmitTask(startResidentAppsTask, "StartResidentApps");
    }

    if (abilityBundleEventCallback_) {
        return;
    }

    // Register abilityBundleEventCallback to receive hap updates
    abilityBundleEventCallback_ =
        new (std::nothrow) AbilityBundleEventCallback(taskHandler_, abilityAutoStartupService_);
    auto bms = GetBundleManager();
    if (bms) {
        bool ret = IN_PROCESS_CALL(bms->RegisterBundleEventCallback(abilityBundleEventCallback_));
        if (!ret) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "RegisterBundleEventCallback failed!");
        }
    } else {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Get BundleManager failed!");
    }
    TAG_LOGD(AAFwkTag::ABILITYMGR, "SubscribeBundleEventCallback success.");
}

void AbilityManagerService::UnsubscribeBundleEventCallback()
{
    if (!abilityBundleEventCallback_) {
        return;
    }
    abilityBundleEventCallback_ = nullptr;
    TAG_LOGD(AAFwkTag::ABILITYMGR, "UnsubscribeBundleEventCallback success.");
}

void AbilityManagerService::ReportAbilitStartInfoToRSS(const AppExecFwk::AbilityInfo &abilityInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (abilityInfo.type == AppExecFwk::AbilityType::PAGE &&
        abilityInfo.launchMode != AppExecFwk::LaunchMode::SPECIFIED) {
        std::vector<AppExecFwk::RunningProcessInfo> runningProcessInfos;
        if (IN_PROCESS_CALL(GetProcessRunningInfos(runningProcessInfos)) != ERR_OK) {
            return;
        }
        bool isColdStart = true;
        int32_t pid = 0;
        for (auto const &info : runningProcessInfos) {
            if (info.uid_ == abilityInfo.applicationInfo.uid) {
                isColdStart = false;
                pid = info.pid_;
                break;
            }
        }
        ResSchedUtil::GetInstance().ReportAbilitStartInfoToRSS(abilityInfo, pid, isColdStart);
    }
}

void AbilityManagerService::ReportAbilitAssociatedStartInfoToRSS(
    const AppExecFwk::AbilityInfo &abilityInfo, int64_t type, const sptr<IRemoteObject> &callerToken)
{
    CHECK_POINTER_LOG(callerToken, "associated start caller token is nullptr");
    auto callerAbility = Token::GetAbilityRecordByToken(callerToken);
    CHECK_POINTER_LOG(callerAbility, "associated start caller ability is nullptr");
    int32_t callerUid = callerAbility->GetUid();
    int32_t callerPid = callerAbility->GetPid();
    ResSchedUtil::GetInstance().ReportAbilitAssociatedStartInfoToRSS(abilityInfo, type, callerUid, callerPid);
}

void AbilityManagerService::ReportEventToRSS(const AppExecFwk::AbilityInfo &abilityInfo,
    sptr<IRemoteObject> callerToken)
{
    CHECK_POINTER_LOG(taskHandler_, "taskhandler null");
    std::string reason;
    if (abilityInfo.type == AppExecFwk::AbilityType::PAGE) {
        reason = "THAW_BY_START_PAGE_ABILITY";
    } else if (abilityInfo.type == AppExecFwk::AbilityType::EXTENSION &&
               abilityInfo.extensionAbilityType == AppExecFwk::ExtensionAbilityType::SERVICE) {
        reason = "THAW_BY_START_SERVICE_EXTENSION";
    } else if (abilityInfo.type == AppExecFwk::AbilityType::EXTENSION &&
               AAFwk::UIExtensionUtils::IsUIExtension(abilityInfo.extensionAbilityType)) {
        reason = "THAW_BY_START_UI_EXTENSION";
    } else {
        reason = "THAW_BY_START_NOT_PAGE_ABILITY";
    }
    const auto uid = abilityInfo.applicationInfo.uid;
    const auto bundleName = abilityInfo.applicationInfo.bundleName;
    auto callerAbility = Token::GetAbilityRecordByToken(callerToken);
    const int32_t callerPid = (callerAbility != nullptr) ? callerAbility->GetPid() : IPCSkeleton::GetCallingPid();
    TAG_LOGD(AAFwkTag::ABILITYMGR, "%{public}d_%{public}s reason=%{public}s callerPid=%{public}d", uid,
        bundleName.c_str(), reason.c_str(), callerPid);
    taskHandler_->SubmitTask([reason, uid, bundleName, callerPid]() {
        ResSchedUtil::GetInstance().ReportEventToRSS(uid, bundleName, reason, callerPid);
    });
}

int AbilityManagerService::StartExtensionAbility(const Want &want, const sptr<IRemoteObject> &callerToken,
    int32_t userId, AppExecFwk::ExtensionAbilityType extensionType)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    InsightIntentExecuteParam::RemoveInsightIntent(const_cast<Want &>(want));
    if (extensionType == AppExecFwk::ExtensionAbilityType::VPN) {
        return StartExtensionAbilityInner(want, callerToken, userId, extensionType, false);
    }
    return StartExtensionAbilityInner(want, callerToken, userId, extensionType, true);
}

int AbilityManagerService::ImplicitStartExtensionAbility(const Want &want, const sptr<IRemoteObject> &callerToken,
    int32_t userId, AppExecFwk::ExtensionAbilityType extensionType)
{
    InsightIntentExecuteParam::RemoveInsightIntent(const_cast<Want &>(want));
    if (extensionType == AppExecFwk::ExtensionAbilityType::VPN) {
        return StartExtensionAbilityInner(want, callerToken, userId, extensionType, false, true);
    }
    return StartExtensionAbilityInner(want, callerToken, userId, extensionType, true, true);
}

int AbilityManagerService::PreloadUIExtensionAbility(const Want &want, std::string &bundleName, int32_t userId)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "called");
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    // check preload ui extension permission.
    CHECK_CALLER_IS_SYSTEM_APP;
    if (!PermissionVerification::GetInstance()->VerifyCallingPermission(
        PermissionConstants::PERMISSION_PRELOAD_UI_EXTENSION_ABILITY)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Permission %{public}s verification failed.",
            PermissionConstants::PERMISSION_PRELOAD_UI_EXTENSION_ABILITY);
        return ERR_PERMISSION_DENIED;
    }
    return PreloadUIExtensionAbilityInner(want, bundleName, userId);
}

int AbilityManagerService::PreloadUIExtensionAbilityInner(const Want &want, std::string &hostBundleName, int32_t userId)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Preload ui extension called, elementName: %{public}s.",
        want.GetElement().GetURI().c_str());
    int32_t validUserId = GetValidUserId(userId);
    AbilityRequest abilityRequest;
    ErrCode result = ERR_OK;
    result = GenerateExtensionAbilityRequest(want, abilityRequest, nullptr, validUserId);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Generate ability request error.");
        return result;
    }
    abilityRequest.want.SetParam(IS_PRELOAD_UIEXTENSION_ABILITY, true);
    auto abilityInfo = abilityRequest.abilityInfo;
    auto res = JudgeAbilityVisibleControl(abilityInfo);
    if (res != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Target ability is invisible");
        return res;
    }
    auto connectManager = GetConnectManagerByUserId(validUserId);
    if (connectManager == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "connectManager is nullptr, userId: %{public}d", validUserId);
        return ERR_INVALID_VALUE;
    }
    return connectManager->PreloadUIExtensionAbilityLocked(abilityRequest, hostBundleName);
}

int AbilityManagerService::UnloadUIExtensionAbility(const std::shared_ptr<AAFwk::AbilityRecord> &abilityRecord,
    std::string &hostBundleName)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "Call.");
    auto connectManager = GetConnectManagerByToken(abilityRecord->GetToken());
    if (connectManager == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "connectManager is nullptr.");
        return ERR_INVALID_VALUE;
    }
    return connectManager->UnloadUIExtensionAbility(abilityRecord, hostBundleName);
}

int AbilityManagerService::RequestModalUIExtension(const Want &want)
{
    CHECK_CALLER_IS_SYSTEM_APP;
    return RequestModalUIExtensionInner(want);
}

int AbilityManagerService::RequestModalUIExtensionInner(Want want)
{
    sptr<IRemoteObject> token = nullptr;
    int ret = IN_PROCESS_CALL(GetTopAbility(token));
    if (ret != ERR_OK || token == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "token is nullptr.");
        return ERR_INVALID_VALUE;
    }

    // Gets the record corresponding to the current focus appliaction
    auto record = Token::GetAbilityRecordByToken(token);
    if (!record) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Record is nullptr.");
        return ERR_INVALID_VALUE;
    }

    // Gets the abilityName, bundleName, modulename corresponding to the current focus appliaction
    EventInfo focusInfo;
    focusInfo.bundleName = record->GetAbilityInfo().bundleName;

    // Gets the abilityName, bundleName, modulename corresponding to the caller appliaction
    EventInfo callerInfo;
    callerInfo.bundleName = want.GetParams().GetStringParam("bundleName");
    TAG_LOGI(AAFwkTag::ABILITYMGR, "focusbundlname: %{public}s, callerbundlname: %{public}s.",
        focusInfo.bundleName.c_str(), callerInfo.bundleName.c_str());

    // Compare
    if (record->GetAbilityInfo().type == AppExecFwk::AbilityType::PAGE &&
        focusInfo.bundleName == callerInfo.bundleName) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "CreateModalUIExtension is called!");
        return record->CreateModalUIExtension(want);
    }

    TAG_LOGD(AAFwkTag::ABILITYMGR, "Window Modal System Create UIExtension is called!");
    want.SetParam(UIEXTENSION_MODAL_TYPE, 1);
    auto connection = std::make_shared<Rosen::ModalSystemUiExtension>();
    return connection->CreateModalUIExtension(want) ? ERR_OK : INNER_ERR;
}

int AbilityManagerService::ChangeAbilityVisibility(sptr<IRemoteObject> token, bool isShow)
{
    bool isEnable = AppUtils::GetInstance().IsStartOptionsWithProcessOptions();
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled() || !isEnable) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Capability not support.");
        return ERR_CAPABILITY_NOT_SUPPORT;
    }
    auto uiAbilityManager = GetUIAbilityManagerByUid(IPCSkeleton::GetCallingUid());
    CHECK_POINTER_AND_RETURN(uiAbilityManager, ERR_INVALID_VALUE);
    return uiAbilityManager->ChangeAbilityVisibility(token, isShow);
}

int AbilityManagerService::ChangeUIAbilityVisibilityBySCB(sptr<SessionInfo> sessionInfo, bool isShow)
{
    if (!IsCallerSceneBoard()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Not sceneboard called, not allowed.");
        return ERR_WRONG_INTERFACE_CALL;
    }
    auto uiAbilityManager = GetUIAbilityManagerByUid(IPCSkeleton::GetCallingUid());
    CHECK_POINTER_AND_RETURN(uiAbilityManager, ERR_INVALID_VALUE);
    return uiAbilityManager->ChangeUIAbilityVisibilityBySCB(sessionInfo, isShow);
}

int AbilityManagerService::StartExtensionAbilityInner(const Want &want, const sptr<IRemoteObject> &callerToken,
    int32_t userId, AppExecFwk::ExtensionAbilityType extensionType, bool checkSystemCaller, bool isImplicit,
    bool isDlp)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::ABILITYMGR,
        "Start extension ability come, bundlename: %{public}s, ability is %{public}s, userId is %{public}d",
        want.GetElement().GetBundleName().c_str(), want.GetElement().GetAbilityName().c_str(), userId);
    if (checkSystemCaller) {
        CHECK_CALLER_IS_SYSTEM_APP;
    }
    EventInfo eventInfo = BuildEventInfo(want, userId);
    eventInfo.extensionType = static_cast<int32_t>(extensionType);

    int result;
#ifdef WITH_DLP
    result = CheckDlpForExtension(want, callerToken, userId, eventInfo, EventName::START_EXTENSION_ERROR);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "CheckDlpForExtension error.");
        return result;
    }
#endif // WITH_DLP

    if (callerToken != nullptr && !VerificationAllToken(callerToken)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "%{public}s VerificationAllToken failed.", __func__);
        eventInfo.errCode = ERR_INVALID_VALUE;
        EventReport::SendExtensionEvent(EventName::START_EXTENSION_ERROR, HiSysEventType::FAULT, eventInfo);
        return ERR_INVALID_CALLER;
    }

    int32_t validUserId = GetValidUserId(userId);
    int32_t appIndex = 0;
    if (!StartAbilityUtils::GetAppIndex(want, callerToken, appIndex)) {
        return ERR_APP_CLONE_INDEX_INVALID;
    }
    StartAbilityInfoWrap threadLocalInfo(want, validUserId, appIndex, callerToken, true);
    AbilityInterceptorParam interceptorParam = AbilityInterceptorParam(want, 0, GetUserId(), false, nullptr);
    result = interceptorExecuter_ == nullptr ? ERR_INVALID_VALUE :
        interceptorExecuter_->DoProcess(interceptorParam);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "interceptorExecuter_ is nullptr or DoProcess return error.");
        eventInfo.errCode = result;
        SendAbilityEvent(EventName::START_ABILITY_ERROR, HiSysEventType::FAULT, eventInfo);
        return result;
    }

    if (!JudgeMultiUserConcurrency(validUserId)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Multi-user non-concurrent mode is not satisfied.");
        eventInfo.errCode = ERR_INVALID_VALUE;
        EventReport::SendExtensionEvent(EventName::START_EXTENSION_ERROR, HiSysEventType::FAULT, eventInfo);
        return ERR_CROSS_USER;
    }

    AbilityRequest abilityRequest;
#ifdef SUPPORT_SCREEN
    if (ImplicitStartProcessor::IsImplicitStartAction(want)) {
        abilityRequest.Voluation(want, DEFAULT_INVAL_VALUE, callerToken);
        abilityRequest.callType = AbilityCallType::START_EXTENSION_TYPE;
        abilityRequest.extensionType = extensionType;
        CHECK_POINTER_AND_RETURN(implicitStartProcessor_, ERR_IMPLICIT_START_ABILITY_FAIL);
        result = implicitStartProcessor_->ImplicitStartAbility(abilityRequest, validUserId);
        if (result != ERR_OK) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "implicit start ability error.");
            eventInfo.errCode = result;
            EventReport::SendExtensionEvent(EventName::START_EXTENSION_ERROR, HiSysEventType::FAULT, eventInfo);
        }
        return result;
    }
#endif
    result = GenerateExtensionAbilityRequest(want, abilityRequest, callerToken, validUserId);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Generate ability request local error.");
        eventInfo.errCode = result;
        EventReport::SendExtensionEvent(EventName::START_EXTENSION_ERROR, HiSysEventType::FAULT, eventInfo);
        return result;
    }
    if (!UriUtils::GetInstance().CheckNonImplicitShareFileUri(abilityRequest)) {
        return ERR_SHARE_FILE_URI_NON_IMPLICITLY;
    }

    auto abilityInfo = abilityRequest.abilityInfo;
    validUserId = abilityInfo.applicationInfo.singleton ? U0_USER_ID : validUserId;
    TAG_LOGD(AAFwkTag::ABILITYMGR, "userId is : %{public}d, singleton is : %{public}d",
        validUserId, static_cast<int>(abilityInfo.applicationInfo.singleton));

#ifdef WITH_DLP
    result = isDlp ? IN_PROCESS_CALL(
        CheckOptExtensionAbility(want, abilityRequest, validUserId, extensionType, isImplicit)) :
        CheckOptExtensionAbility(want, abilityRequest, validUserId, extensionType, isImplicit);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "CheckOptExtensionAbility error.");
        eventInfo.errCode = result;
        EventReport::SendExtensionEvent(EventName::START_EXTENSION_ERROR, HiSysEventType::FAULT, eventInfo);
        return result;
    }
#endif // WITH_DLP

    AbilityInterceptorParam afterCheckParam = AbilityInterceptorParam(abilityRequest.want, 0, GetUserId(),
        false, callerToken, std::make_shared<AppExecFwk::AbilityInfo>(abilityInfo));
    result = afterCheckExecuter_ == nullptr ? ERR_INVALID_VALUE :
        afterCheckExecuter_->DoProcess(afterCheckParam);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "afterCheckExecuter_ is nullptr or DoProcess return error.");
        return result;
    }

    auto connectManager = GetConnectManagerByUserId(validUserId);
    if (!connectManager) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "connectManager is nullptr. userId=%{public}d", validUserId);
        eventInfo.errCode = ERR_INVALID_VALUE;
        EventReport::SendExtensionEvent(EventName::START_EXTENSION_ERROR, HiSysEventType::FAULT, eventInfo);
        return ERR_INVALID_VALUE;
    }
    UpdateCallerInfo(abilityRequest.want, callerToken);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Start extension begin, name is %{public}s.", abilityInfo.name.c_str());
    SetAbilityRequestSessionInfo(abilityRequest, extensionType);
    eventInfo.errCode = connectManager->StartAbility(abilityRequest);
    if (eventInfo.errCode != ERR_OK) {
        EventReport::SendExtensionEvent(EventName::START_EXTENSION_ERROR, HiSysEventType::FAULT, eventInfo);
    }
    ReportAbilitAssociatedStartInfoToRSS(abilityRequest.abilityInfo, RES_TYPE_EXTENSION_START_ABILITY, callerToken);
    return eventInfo.errCode;
}

void AbilityManagerService::SetPickerElementName(const sptr<SessionInfo> &extensionSessionInfo, int32_t userId)
{
    CHECK_POINTER_IS_NULLPTR(extensionSessionInfo);
    std::string targetType = extensionSessionInfo->want.GetStringParam(UIEXTENSION_TARGET_TYPE_KEY);
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled() &&
        extensionSessionInfo->want.GetElement().GetBundleName().empty() &&
        extensionSessionInfo->want.GetElement().GetAbilityName().empty() &&
        COMMON_PICKER_TYPE.find(targetType) != COMMON_PICKER_TYPE.end()) {
        std::string abilityName = "CommonSelectPickerAbility";
        std::string bundleName = "com.ohos.amsdialog";
        extensionSessionInfo->want.SetElementName(bundleName, abilityName);
        WantParams &parameters = const_cast<WantParams &>(extensionSessionInfo->want.GetParams());
        parameters.SetParam(UIEXTENSION_TYPE_KEY, AAFwk::String::Box("sys/commonUI"));
        extensionSessionInfo->want.SetParams(parameters);
        return;
    }
    if (extensionSessionInfo->want.GetElement().GetBundleName().empty() &&
        extensionSessionInfo->want.GetElement().GetAbilityName().empty() && !targetType.empty()) {
        std::string abilityName;
        std::string bundleName;
        std::string pickerType;
        std::vector<AppExecFwk::ExtensionAbilityInfo> extensionInfos;
        auto pickerMap = AmsConfigurationParameter::GetInstance().GetPickerMap();
        auto it = pickerMap.find(targetType);
        if (it == pickerMap.end()) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "can not find the targetType: %{public}s", targetType.c_str());
            return;
        }
        pickerType = it->second;
        auto bms = GetBundleManager();
        CHECK_POINTER(bms);
        int32_t validUserId = GetValidUserId(userId);
        TAG_LOGI(AAFwkTag::ABILITYMGR, "targetType: %{public}s, pickerType: %{public}s, userId: %{public}d",
            targetType.c_str(), pickerType.c_str(), validUserId);
        auto ret = IN_PROCESS_CALL(bms->QueryExtensionAbilityInfosOnlyWithTypeName(pickerType,
            static_cast<int32_t>(AppExecFwk::GetExtensionAbilityInfoFlag::GET_EXTENSION_ABILITY_INFO_WITH_PERMISSION),
            validUserId,
            extensionInfos));
        if (ret != ERR_OK) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "QueryExtensionAbilityInfosOnlyWithTypeName failed");
            return;
        }
        abilityName = extensionInfos[0].name;
        bundleName = extensionInfos[0].bundleName;
        TAG_LOGI(AAFwkTag::ABILITYMGR,
            "abilityName: %{public}s, bundleName: %{public}s", abilityName.c_str(), bundleName.c_str());
        extensionSessionInfo->want.SetElementName(bundleName, abilityName);
        WantParams &parameters = const_cast<WantParams &>(extensionSessionInfo->want.GetParams());
        parameters.SetParam(UIEXTENSION_TYPE_KEY, AAFwk::String::Box(pickerType));
        extensionSessionInfo->want.SetParams(parameters);
    }
}

void AbilityManagerService::SetAutoFillElementName(const sptr<SessionInfo> &extensionSessionInfo)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "called");
    CHECK_POINTER_IS_NULLPTR(extensionSessionInfo);
    std::vector<std::string> argList;
    if (extensionSessionInfo->want.GetStringParam(UIEXTENSION_TYPE_KEY) == AUTO_FILL_PASSWORD_TPYE) {
        SplitStr(KEY_AUTO_FILL_ABILITY, "/", argList);
    } else if (extensionSessionInfo->want.GetStringParam(UIEXTENSION_TYPE_KEY) == AUTO_FILL_SMART_TPYE) {
        SplitStr(KEY_SMART_AUTO_FILL_ABILITY, "/", argList);
    } else {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "It is not autofill type.");
        return;
    }

    if (argList.size() != ARGC_THREE) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Parse auto fill extension element name failed.");
        return;
    }
    extensionSessionInfo->want.SetElementName(argList[INDEX_ZERO], argList[INDEX_TWO]);
    extensionSessionInfo->want.SetModuleName(argList[INDEX_ONE]);
}

int AbilityManagerService::CheckUIExtensionUsage(AppExecFwk::UIExtensionUsage uiExtensionUsage,
    AppExecFwk::ExtensionAbilityType extensionType)
{
    if (uiExtensionUsage == UIExtensionUsage::EMBEDDED &&
        !AAFwk::UIExtensionUtils::IsPublicForEmbedded(extensionType)) {
        CHECK_CALLER_IS_SYSTEM_APP;
    }

    if (uiExtensionUsage == UIExtensionUsage::CONSTRAINED_EMBEDDED &&
        !AAFwk::UIExtensionUtils::IsPublicForConstrainedEmbedded(extensionType)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "error extension type %u for SecureConstrainedEmbedded.", extensionType);
        return ERR_INVALID_VALUE;
    }
    return ERR_OK;
}

int AbilityManagerService::StartUIExtensionAbility(const sptr<SessionInfo> &extensionSessionInfo, int32_t userId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Start ui extension ability come");
    CHECK_POINTER_AND_RETURN(extensionSessionInfo, ERR_INVALID_VALUE);
    SetPickerElementName(extensionSessionInfo, userId);
    SetAutoFillElementName(extensionSessionInfo);

    if (extensionSessionInfo->want.HasParameter(AAFwk::SCREEN_MODE_KEY)) {
        int32_t screenMode = extensionSessionInfo->want.GetIntParam(AAFwk::SCREEN_MODE_KEY, AAFwk::IDLE_SCREEN_MODE);
        if (screenMode != AAFwk::EMBEDDED_FULL_SCREEN_MODE) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "Only support embedded pull-ups");
            return ERR_INVALID_VALUE;
        }
        auto bms = GetBundleManager();
        CHECK_POINTER_AND_RETURN(bms, ERR_INVALID_VALUE);
        AppExecFwk::BundleInfo bundleInfo;
        if (!IN_PROCESS_CALL(bms->GetBundleInfo(extensionSessionInfo->want.GetBundle(),
            AppExecFwk::BundleFlag::GET_BUNDLE_WITH_ABILITIES, bundleInfo, GetValidUserId(userId)))) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "VerifyPermission failed to get application info");
            return CHECK_PERMISSION_FAILED;
        }
        if (bundleInfo.applicationInfo.bundleType != AppExecFwk::BundleType::ATOMIC_SERVICE) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "Only support atomicService");
            return ERR_INVALID_CALLER;
        }
        if (extensionSessionInfo->want.GetElement().GetAbilityName().empty()) {
            if (bundleInfo.abilityInfos.empty()) {
                TAG_LOGE(AAFwkTag::ABILITYMGR, "failed to get abilityInfos");
                return ERR_INVALID_VALUE;
            }
            extensionSessionInfo->want.SetElementName(bundleInfo.name, bundleInfo.abilityInfos.begin()->name);
        }
        extensionSessionInfo->want.SetParam("send_to_erms_embedded", 1);
    }
    std::string extensionTypeStr = extensionSessionInfo->want.GetStringParam(UIEXTENSION_TYPE_KEY);
    AppExecFwk::ExtensionAbilityType extensionType = extensionTypeStr.empty() ?
        AppExecFwk::ExtensionAbilityType::UI : AppExecFwk::ConvertToExtensionAbilityType(extensionTypeStr);
    if (extensionType == AppExecFwk::ExtensionAbilityType::UNSPECIFIED) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Input extension ability type is invalid.");
        return ERR_INVALID_VALUE;
    }
    EventInfo eventInfo = BuildEventInfo(extensionSessionInfo->want, userId);
    eventInfo.extensionType = static_cast<int32_t>(extensionType);

    auto ret = CheckUIExtensionUsage(extensionSessionInfo->uiExtensionUsage, extensionType);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "check usage failed.");
        return ret;
    }

    if (InsightIntentExecuteParam::IsInsightIntentExecute(extensionSessionInfo->want)) {
        int32_t result = DelayedSingleton<InsightIntentExecuteManager>::GetInstance()->CheckAndUpdateWant(
            extensionSessionInfo->want, AppExecFwk::ExecuteMode::UI_EXTENSION_ABILITY);
        if (result != ERR_OK) {
            return result;
        }
    }

    sptr<IRemoteObject> callerToken = extensionSessionInfo->callerToken;

#ifdef WITH_DLP
    if (!DlpUtils::OtherAppsAccessDlpCheck(callerToken, extensionSessionInfo->want) ||
        VerifyAccountPermission(userId) == CHECK_PERMISSION_FAILED ||
        !DlpUtils::DlpAccessOtherAppsCheck(callerToken, extensionSessionInfo->want)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "StartUIExtensionAbility: Permission verification failed.");
        eventInfo.errCode = CHECK_PERMISSION_FAILED;
        EventReport::SendExtensionEvent(EventName::START_EXTENSION_ERROR, HiSysEventType::FAULT, eventInfo);
        return CHECK_PERMISSION_FAILED;
    }
#endif // WITH_DLP

    if (callerToken != nullptr && !VerificationAllToken(callerToken)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "StartUIExtensionAbility VerificationAllToken failed.");
        eventInfo.errCode = ERR_INVALID_VALUE;
        EventReport::SendExtensionEvent(EventName::START_EXTENSION_ERROR, HiSysEventType::FAULT, eventInfo);
        return ERR_INVALID_CALLER;
    }

    auto callerRecord = Token::GetAbilityRecordByToken(callerToken);
    if (callerRecord == nullptr || !JudgeSelfCalled(callerRecord)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "invalid callerToken.");
        eventInfo.errCode = ERR_INVALID_VALUE;
        EventReport::SendExtensionEvent(EventName::START_EXTENSION_ERROR, HiSysEventType::FAULT, eventInfo);
        return ERR_INVALID_CALLER;
    }

    AbilityInterceptorParam interceptorParam = AbilityInterceptorParam(extensionSessionInfo->want, 0, GetUserId(),
        true, nullptr);
    auto result = interceptorExecuter_ == nullptr ? ERR_INVALID_VALUE :
        interceptorExecuter_->DoProcess(interceptorParam);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "interceptorExecuter_ is nullptr or DoProcess return error.");
        eventInfo.errCode = result;
        EventReport::SendExtensionEvent(EventName::START_EXTENSION_ERROR, HiSysEventType::FAULT, eventInfo);
        return result;
    }

    int32_t validUserId = GetValidUserId(userId);
    if (!JudgeMultiUserConcurrency(validUserId)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Multi-user non-concurrent mode is not satisfied.");
        eventInfo.errCode = ERR_INVALID_VALUE;
        EventReport::SendExtensionEvent(EventName::START_EXTENSION_ERROR, HiSysEventType::FAULT, eventInfo);
        return ERR_INVALID_VALUE;
    }
#ifdef SUPPORT_GRAPHICS
    if (ImplicitStartProcessor::IsImplicitStartAction(extensionSessionInfo->want)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "UI extension ability donot support implicit start.");
        eventInfo.errCode = ERR_INVALID_VALUE;
        EventReport::SendExtensionEvent(EventName::START_EXTENSION_ERROR, HiSysEventType::FAULT, eventInfo);
        return ERR_INVALID_VALUE;
    }
#endif // SUPPORT_GRAPHICS
    AbilityRequest abilityRequest;
    abilityRequest.Voluation(extensionSessionInfo->want, DEFAULT_INVAL_VALUE, callerToken);
    abilityRequest.callType = AbilityCallType::START_EXTENSION_TYPE;
    abilityRequest.sessionInfo = extensionSessionInfo;
    result = GenerateEmbeddableUIAbilityRequest(extensionSessionInfo->want, abilityRequest, callerToken, validUserId);
    CHECK_POINTER_AND_RETURN(abilityRequest.sessionInfo, ERR_INVALID_VALUE);
    abilityRequest.sessionInfo->uiExtensionComponentId = (
        static_cast<uint64_t>(callerRecord->GetRecordId()) << OFFSET) |
        static_cast<uint64_t>(abilityRequest.sessionInfo->persistentId);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "UIExtension component id: %{public}" PRId64 ", element: %{public}s.",
        abilityRequest.sessionInfo->uiExtensionComponentId, extensionSessionInfo->want.GetElement().GetURI().c_str());
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Generate ability request local error.");
        eventInfo.errCode = result;
        EventReport::SendExtensionEvent(EventName::START_EXTENSION_ERROR, HiSysEventType::FAULT, eventInfo);
        return result;
    }
    if (!UriUtils::GetInstance().CheckNonImplicitShareFileUri(abilityRequest)) {
        return ERR_SHARE_FILE_URI_NON_IMPLICITLY;
    }
    abilityRequest.extensionType = abilityRequest.abilityInfo.extensionAbilityType;

    auto abilityInfo = abilityRequest.abilityInfo;
    validUserId = abilityInfo.applicationInfo.singleton ? U0_USER_ID : validUserId;
    TAG_LOGD(AAFwkTag::ABILITYMGR, "userId is : %{public}d, singleton is : %{public}d",
        validUserId, static_cast<int>(abilityInfo.applicationInfo.singleton));

    result = CheckOptExtensionAbility(extensionSessionInfo->want, abilityRequest, validUserId, extensionType);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "CheckOptExtensionAbility error.");
        eventInfo.errCode = result;
        EventReport::SendExtensionEvent(EventName::START_EXTENSION_ERROR, HiSysEventType::FAULT, eventInfo);
        return result;
    }

    sptr<IRemoteObject> parentToken = extensionSessionInfo->parentToken;
    if (parentToken && parentToken != callerToken) {
        UpdateCallerInfoFromToken(abilityRequest.want, parentToken);
    }

    result = JudgeAbilityVisibleControl(abilityInfo);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "JudgeAbilityVisibleControl error.");
        eventInfo.errCode = result;
        EventReport::SendExtensionEvent(EventName::START_EXTENSION_ERROR, HiSysEventType::FAULT, eventInfo);
        return result;
    }

    AbilityInterceptorParam afterCheckParam = AbilityInterceptorParam(abilityRequest.want, 0, GetUserId(),
        true, callerToken, std::make_shared<AppExecFwk::AbilityInfo>(abilityInfo));
    result = afterCheckExecuter_ == nullptr ? ERR_INVALID_VALUE :
        afterCheckExecuter_->DoProcess(afterCheckParam);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "afterCheckExecuter_ is nullptr or DoProcess return error.");
        return result;
    }

    auto connectManager = GetConnectManagerByUserId(validUserId);
    if (!connectManager) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "connectManager is nullptr. userId=%{public}d", validUserId);
        eventInfo.errCode = ERR_INVALID_VALUE;
        EventReport::SendExtensionEvent(EventName::START_EXTENSION_ERROR, HiSysEventType::FAULT, eventInfo);
        return ERR_INVALID_VALUE;
    }
    ReportEventToRSS(abilityRequest.abilityInfo, abilityRequest.callerToken);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "Start extension begin, name is %{public}s.", abilityInfo.name.c_str());
    eventInfo.errCode = connectManager->StartAbility(abilityRequest);
    if (eventInfo.errCode != ERR_OK) {
        EventReport::SendExtensionEvent(EventName::START_EXTENSION_ERROR, HiSysEventType::FAULT, eventInfo);
    }
    return eventInfo.errCode;
}

int AbilityManagerService::StopExtensionAbility(const Want &want, const sptr<IRemoteObject> &callerToken,
    int32_t userId, AppExecFwk::ExtensionAbilityType extensionType)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR,
        "Stop extension ability come, bundlename: %{public}s, ability is %{public}s, userId is %{public}d",
        want.GetElement().GetBundleName().c_str(), want.GetElement().GetAbilityName().c_str(), userId);
    if (extensionType != AppExecFwk::ExtensionAbilityType::VPN) {
        CHECK_CALLER_IS_SYSTEM_APP;
    }
    EventInfo eventInfo = BuildEventInfo(want, userId);
    eventInfo.extensionType = static_cast<int32_t>(extensionType);

    int result;
#ifdef WITH_DLP
    result = CheckDlpForExtension(want, callerToken, userId, eventInfo, EventName::STOP_EXTENSION_ERROR);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "CheckDlpForExtension error.");
        return result;
    }
#endif // WITH_DLP

    if (callerToken != nullptr && !VerificationAllToken(callerToken)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "%{public}s VerificationAllToken failed.", __func__);
        if (!PermissionVerification::GetInstance()->CheckSpecificSystemAbilityAccessPermission(DMS_PROCESS_NAME)) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "VerificationAllToken failed.");
            eventInfo.errCode = ERR_INVALID_VALUE;
            EventReport::SendExtensionEvent(EventName::STOP_EXTENSION_ERROR, HiSysEventType::FAULT, eventInfo);
            return ERR_INVALID_CALLER;
        }
        TAG_LOGD(AAFwkTag::ABILITYMGR, "Caller is specific system ability.");
    }

    int32_t validUserId = GetValidUserId(userId);
    if (!JudgeMultiUserConcurrency(validUserId)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Multi-user non-concurrent mode is not satisfied.");
        eventInfo.errCode = ERR_INVALID_VALUE;
        EventReport::SendExtensionEvent(EventName::STOP_EXTENSION_ERROR, HiSysEventType::FAULT, eventInfo);
        return ERR_CROSS_USER;
    }

    if (callerToken != nullptr && CheckIfOperateRemote(want)) {
        auto callerUid = IPCSkeleton::GetCallingUid();
        uint32_t accessToken = IPCSkeleton::GetCallingTokenID();
        DistributedClient dmsClient;
        return dmsClient.StopRemoteExtensionAbility(want, callerUid, accessToken, eventInfo.extensionType);
    }

    AbilityRequest abilityRequest;
    result = GenerateExtensionAbilityRequest(want, abilityRequest, callerToken, validUserId);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Generate ability request local error.");
        eventInfo.errCode = result;
        EventReport::SendExtensionEvent(EventName::STOP_EXTENSION_ERROR, HiSysEventType::FAULT, eventInfo);
        return result;
    }

    auto abilityInfo = abilityRequest.abilityInfo;
    validUserId = abilityInfo.applicationInfo.singleton ? U0_USER_ID : validUserId;
    TAG_LOGD(AAFwkTag::ABILITYMGR, "userId is : %{public}d, singleton is : %{public}d",
        validUserId, static_cast<int>(abilityInfo.applicationInfo.singleton));

    result = CheckOptExtensionAbility(want, abilityRequest, validUserId, extensionType);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "CheckOptExtensionAbility error.");
        eventInfo.errCode = result;
        EventReport::SendExtensionEvent(EventName::STOP_EXTENSION_ERROR, HiSysEventType::FAULT, eventInfo);
        return result;
    }

    auto connectManager = GetConnectManagerByUserId(validUserId);
    if (!connectManager) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "connectManager is nullptr. userId=%{public}d", validUserId);
        eventInfo.errCode = ERR_INVALID_VALUE;
        EventReport::SendExtensionEvent(EventName::STOP_EXTENSION_ERROR, HiSysEventType::FAULT, eventInfo);
        return ERR_INVALID_VALUE;
    }
    TAG_LOGI(AAFwkTag::ABILITYMGR, "Stop extension begin, name is %{public}s.", abilityInfo.name.c_str());
    eventInfo.errCode = connectManager->StopServiceAbility(abilityRequest);
    if (eventInfo.errCode != ERR_OK) {
        EventReport::SendExtensionEvent(EventName::STOP_EXTENSION_ERROR, HiSysEventType::FAULT, eventInfo);
    }
    return eventInfo.errCode;
}

void AbilityManagerService::StopSwitchUserDialog()
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Stop switch user dialog extension ability come");
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Scene board enabled.");
        return;
    }

    if (userController_ == nullptr || userController_->GetFreezingNewUserId() == DEFAULT_INVAL_VALUE) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Get last userId error.");
        return;
    }
#ifdef SUPPORT_GRAPHICS
    auto sysDialog = DelayedSingleton<SystemDialogScheduler>::GetInstance();
    if (sysDialog == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "System dialog scheduler instance is nullptr.");
        return;
    }
    Want stopWant = sysDialog->GetSwitchUserDialogWant();
    StopSwitchUserDialogInner(stopWant, userController_->GetFreezingNewUserId());
#endif // SUPPORT_GRAPHICS
    userController_->SetFreezingNewUserId(DEFAULT_INVAL_VALUE);
    return;
}

void AbilityManagerService::StopSwitchUserDialogInner(const Want &want, const int32_t lastUserId)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Stop switch user dialog inner come");
    EventInfo eventInfo = BuildEventInfo(want, lastUserId);
    eventInfo.extensionType = static_cast<int32_t>(AppExecFwk::ExtensionAbilityType::SERVICE);
    AbilityRequest abilityRequest;
    auto result =
        GenerateExtensionAbilityRequest(want, abilityRequest, nullptr, lastUserId);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Generate ability request local error.");
        eventInfo.errCode = result;
        EventReport::SendExtensionEvent(EventName::STOP_EXTENSION_ERROR, HiSysEventType::FAULT, eventInfo);
        return;
    }

    auto abilityInfo = abilityRequest.abilityInfo;
    auto stopUserId = abilityInfo.applicationInfo.singleton ? U0_USER_ID : lastUserId;
    result = CheckOptExtensionAbility(want, abilityRequest, stopUserId, AppExecFwk::ExtensionAbilityType::SERVICE);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Check extensionAbility type error.");
        eventInfo.errCode = result;
        EventReport::SendExtensionEvent(EventName::STOP_EXTENSION_ERROR, HiSysEventType::FAULT, eventInfo);
        return;
    }

    auto connectManager = GetConnectManagerByUserId(stopUserId);
    if (connectManager == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "ConnectManager is nullptr. userId:%{public}d", stopUserId);
        eventInfo.errCode = ERR_INVALID_VALUE;
        EventReport::SendExtensionEvent(EventName::STOP_EXTENSION_ERROR, HiSysEventType::FAULT, eventInfo);
        return;
    }

    eventInfo.errCode = connectManager->StopServiceAbility(abilityRequest);
    if (eventInfo.errCode != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "EventInfo errCode is %{public}d", eventInfo.errCode);
        EventReport::SendExtensionEvent(EventName::STOP_EXTENSION_ERROR, HiSysEventType::FAULT, eventInfo);
    }
}

int AbilityManagerService::MoveAbilityToBackground(const sptr<IRemoteObject> &token)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Move ability to background begin");
    if (!VerificationAllToken(token)) {
        return ERR_INVALID_VALUE;
    }
    auto abilityRecord = Token::GetAbilityRecordByToken(token);
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);
    if (!JudgeSelfCalled(abilityRecord)) {
        return CHECK_PERMISSION_FAILED;
    }

    if (!IsAbilityControllerForeground(abilityRecord->GetAbilityInfo().bundleName)) {
        return ERR_WOULD_BLOCK;
    }

    auto ownerUserId = abilityRecord->GetOwnerMissionUserId();
    auto missionListManager = GetMissionListManagerByUserId(ownerUserId);
    if (!missionListManager) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "missionListManager is Null. ownerUserId=%{public}d", ownerUserId);
        return ERR_INVALID_VALUE;
    }
    return missionListManager->MoveAbilityToBackground(abilityRecord);
}

int32_t AbilityManagerService::MoveUIAbilityToBackground(const sptr<IRemoteObject> token)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "called");
    if (!AppUtils::GetInstance().EnableMoveUIAbilityToBackgroundApi()) {
        return ERR_OPERATION_NOT_SUPPORTED_ON_CURRENT_DEVICE;
    }
    if (!VerificationAllToken(token)) {
        return ERR_INVALID_VALUE;
    }
    auto abilityRecord = Token::GetAbilityRecordByToken(token);
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);
    if (!IsAppSelfCalled(abilityRecord)) {
        return CHECK_PERMISSION_FAILED;
    }
    if (!IsAbilityControllerForeground(abilityRecord->GetAbilityInfo().bundleName)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Can not move ability to background in Wukong mode.");
        return ERR_WUKONG_MODE_CANT_MOVE_STATE;
    }
    if (!abilityRecord->IsAbilityState(FOREGROUND) && !abilityRecord->IsAbilityState(FOREGROUNDING)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Ability not in foregorund state.");
        return ERR_ABILITY_NOT_FOREGROUND;
    }
    if (abilityRecord->GetAbilityInfo().type != AppExecFwk::AbilityType::PAGE) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Cannot background non UIAbility.");
        return RESOLVE_CALL_ABILITY_TYPE_ERR;
    }
    auto ownerUserId = abilityRecord->GetOwnerMissionUserId();
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        auto uiAbilityManager = GetUIAbilityManagerByUserId(ownerUserId);
        CHECK_POINTER_AND_RETURN(uiAbilityManager, ERR_INVALID_VALUE);
        return uiAbilityManager->NotifySCBToMinimizeUIAbility(abilityRecord, token);
    }

    auto missionListManager = GetMissionListManagerByUserId(ownerUserId);
    CHECK_POINTER_AND_RETURN(missionListManager, ERR_INVALID_VALUE);
    return missionListManager->MoveAbilityToBackground(abilityRecord);
}

int AbilityManagerService::TerminateAbility(const sptr<IRemoteObject> &token, int resultCode, const Want *resultWant)
{
    auto abilityRecord = Token::GetAbilityRecordByToken(token);
    if (!abilityRecord) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "abilityRecord is Null.");
        return ERR_INVALID_VALUE;
    }
    return TerminateAbilityWithFlag(token, resultCode, resultWant, true);
}

int AbilityManagerService::CloseAbility(const sptr<IRemoteObject> &token, int resultCode, const Want *resultWant)
{
    EventInfo eventInfo;
    SendAbilityEvent(EventName::CLOSE_ABILITY, HiSysEventType::BEHAVIOR, eventInfo);
    return TerminateAbilityWithFlag(token, resultCode, resultWant, false);
}

int AbilityManagerService::TerminateAbilityWithFlag(const sptr<IRemoteObject> &token, int resultCode,
    const Want *resultWant, bool flag)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Terminate ability begin, flag:%{public}d.", flag);
    if (!VerificationAllToken(token)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "%{public}s VerificationAllToken failed.", __func__);
        return ERR_INVALID_VALUE;
    }

    auto abilityRecord = Token::GetAbilityRecordByToken(token);
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);
    if (!JudgeSelfCalled(abilityRecord)) {
        return CHECK_PERMISSION_FAILED;
    }

    if (IsSystemUiApp(abilityRecord->GetAbilityInfo())) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "System ui not allow terminate.");
        return ERR_INVALID_VALUE;
    }

    auto userId = abilityRecord->GetApplicationInfo().uid / BASE_USER_RANGE;
    auto type = abilityRecord->GetAbilityInfo().type;
    if (type == AppExecFwk::AbilityType::SERVICE || type == AppExecFwk::AbilityType::EXTENSION) {
        auto connectManager = GetConnectManagerByUserId(userId);
        if (!connectManager) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "connectManager is nullptr. userId=%{public}d", userId);
            return ERR_INVALID_VALUE;
        }
        return connectManager->TerminateAbility(token);
    }

    if (type == AppExecFwk::AbilityType::DATA) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Cannot terminate data ability, use 'ReleaseDataAbility()' instead.");
        return ERR_WRONG_INTERFACE_CALL;
    }

    if (!IsAbilityControllerForeground(abilityRecord->GetAbilityInfo().bundleName)) {
        return ERR_WOULD_BLOCK;
    }

    auto ownerUserId = abilityRecord->GetOwnerMissionUserId();
    auto missionListManager = GetMissionListManagerByUserId(ownerUserId);
    if (missionListManager) {
        return missionListManager->TerminateAbility(abilityRecord, resultCode, resultWant, flag);
    }
    TAG_LOGW(AAFwkTag::ABILITYMGR, "missionListManager is Null. ownerUserId=%{public}d", ownerUserId);
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        auto uiAbilityManager = GetUIAbilityManagerByUserId(ownerUserId);
        CHECK_POINTER_AND_RETURN(uiAbilityManager, ERR_INVALID_VALUE);
        return uiAbilityManager->CloseUIAbility(abilityRecord, resultCode, resultWant, false);
    }
    return ERR_INVALID_VALUE;
}

int AbilityManagerService::TerminateUIExtensionAbility(const sptr<SessionInfo> &extensionSessionInfo, int resultCode,
    const Want *resultWant)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Terminate ui extension ability begin.");
    CHECK_POINTER_AND_RETURN(extensionSessionInfo, ERR_INVALID_VALUE);
    auto abilityRecord = Token::GetAbilityRecordByToken(extensionSessionInfo->callerToken);
    std::shared_ptr<AbilityConnectManager> connectManager;
    std::shared_ptr<AbilityRecord> targetRecord;
    GetConnectManagerAndUIExtensionBySessionInfo(extensionSessionInfo, connectManager, targetRecord);
    CHECK_POINTER_AND_RETURN(targetRecord, ERR_INVALID_VALUE);
    CHECK_POINTER_AND_RETURN(connectManager, ERR_INVALID_VALUE);

    // self terminate or caller terminate is allowed.
    if (!(JudgeSelfCalled(targetRecord) || (abilityRecord != nullptr && JudgeSelfCalled(abilityRecord)))) {
        return CHECK_PERMISSION_FAILED;
    }

    auto result = JudgeAbilityVisibleControl(targetRecord->GetAbilityInfo());
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "JudgeAbilityVisibleControl error.");
        return result;
    }

    if (!UIExtensionUtils::IsUIExtension(targetRecord->GetAbilityInfo().extensionAbilityType)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Cannot terminate except ui extension ability.");
        return ERR_WRONG_INTERFACE_CALL;
    }

    TAG_LOGD(AAFwkTag::ABILITYMGR, "UIExtension persistentId: %{public}d, element: %{public}s.",
        extensionSessionInfo->persistentId, extensionSessionInfo->want.GetElement().GetURI().c_str());
    connectManager->TerminateAbilityWindowLocked(targetRecord, extensionSessionInfo);
    return ERR_OK;
}

int AbilityManagerService::CloseUIAbilityBySCB(const sptr<SessionInfo> &sessionInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (sessionInfo == nullptr || sessionInfo->sessionToken == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "sessionInfo is nullptr");
        return ERR_INVALID_VALUE;
    }

    if (!IsCallerSceneBoard()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Not sceneboard called, not allowed.");
        return ERR_WRONG_INTERFACE_CALL;
    }

    auto uiAbilityManager = GetUIAbilityManagerByUid(IPCSkeleton::GetCallingUid());
    CHECK_POINTER_AND_RETURN(uiAbilityManager, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::ABILITYMGR,
        "close session: %{public}d, resultCode: %{public}d", sessionInfo->persistentId, sessionInfo->resultCode);
    auto abilityRecord = uiAbilityManager->GetUIAbilityRecordBySessionInfo(sessionInfo);
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);
    if (!IsAbilityControllerForeground(abilityRecord->GetAbilityInfo().bundleName)) {
        return ERR_WOULD_BLOCK;
    }

    if (sessionInfo->isClearSession) {
        const auto &abilityInfo = abilityRecord->GetAbilityInfo();
        std::string abilityName = abilityInfo.name;
        if (abilityInfo.launchMode == AppExecFwk::LaunchMode::STANDARD) {
            abilityName += std::to_string(sessionInfo->persistentId);
        }
        (void)DelayedSingleton<AbilityRuntime::AppExitReasonDataManager>::GetInstance()->
            DeleteAbilityRecoverInfo(abilityInfo.applicationInfo.accessTokenId, abilityInfo.moduleName, abilityName);
    }

    EventInfo eventInfo;
    eventInfo.bundleName = abilityRecord->GetAbilityInfo().bundleName;
    eventInfo.abilityName = abilityRecord->GetAbilityInfo().name;
    SendAbilityEvent(EventName::CLOSE_ABILITY, HiSysEventType::BEHAVIOR, eventInfo);
    eventInfo.errCode = uiAbilityManager->CloseUIAbility(abilityRecord, sessionInfo->resultCode,
        &(sessionInfo->want), sessionInfo->isClearSession);
    if (eventInfo.errCode != ERR_OK) {
        SendAbilityEvent(EventName::TERMINATE_ABILITY_ERROR, HiSysEventType::FAULT, eventInfo);
    }
    return eventInfo.errCode;
}

int AbilityManagerService::SendResultToAbility(int32_t requestCode, int32_t resultCode, Want &resultWant)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "%{public}s", __func__);
    if (!CheckCallerIsDmsProcess()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Check processName failed");
        return ERR_INVALID_VALUE;
    }
    int missionId = resultWant.GetIntParam(DMS_MISSION_ID, DEFAULT_DMS_MISSION_ID);
    resultWant.RemoveParam(DMS_MISSION_ID);
    if (missionId == DEFAULT_DMS_MISSION_ID) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "MissionId is empty");
        return ERR_INVALID_VALUE;
    }
    std::shared_ptr<AbilityRecord> abilityRecord = nullptr;
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        auto uiAbilityManager = GetCurrentUIAbilityManager();
        CHECK_POINTER_AND_RETURN(uiAbilityManager, ERR_INVALID_VALUE);
        abilityRecord = uiAbilityManager->GetAbilityRecordsById(missionId);
    } else {
        sptr<IRemoteObject> abilityToken = GetAbilityTokenByMissionId(missionId);
        CHECK_POINTER_AND_RETURN(abilityToken, ERR_INVALID_VALUE);
        abilityRecord = Token::GetAbilityRecordByToken(abilityToken);
    }
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);

    abilityRecord->SetResult(std::make_shared<AbilityResult>(requestCode, resultCode, resultWant));
    abilityRecord->SendResult(0, 0);
    return ERR_OK;
}

int AbilityManagerService::StartRemoteAbility(const Want &want, int requestCode, int32_t validUserId,
    const sptr<IRemoteObject> &callerToken)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "%{public}s", __func__);
    Want remoteWant = want;
    if (AddStartControlParam(remoteWant, callerToken) != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "%{public}s AddStartControlParam failed.", __func__);
        return ERR_INVALID_VALUE;
    }
    if (AbilityUtil::IsStartFreeInstall(remoteWant)) {
        return freeInstallManager_ == nullptr ? ERR_INVALID_VALUE :
            freeInstallManager_->StartRemoteFreeInstall(remoteWant, requestCode, validUserId, callerToken);
    }
    if (remoteWant.GetBoolParam(Want::PARAM_RESV_FOR_RESULT, false)) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "%{public}s: try to StartAbilityForResult", __func__);
        int32_t missionId = -1;
        if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
            missionId = GetMissionIdByAbilityTokenInner(callerToken);
            if (!missionId) {
                TAG_LOGE(AAFwkTag::ABILITYMGR, "Invalid missionId id.");
                return ERR_INVALID_VALUE;
            }
        } else {
            missionId = GetMissionIdByAbilityToken(callerToken);
        }
        if (missionId < 0) {
            return ERR_INVALID_VALUE;
        }
        remoteWant.SetParam(DMS_MISSION_ID, missionId);
    }

    int32_t callerUid = IPCSkeleton::GetCallingUid();
    uint32_t accessToken = IPCSkeleton::GetCallingTokenID();
    UriUtils::GetInstance().FilterUriWithPermissionDms(remoteWant, accessToken);
    DistributedClient dmsClient;
    int result = dmsClient.StartRemoteAbility(remoteWant, callerUid, requestCode, accessToken);
    if (result != ERR_NONE) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "AbilityManagerService::StartRemoteAbility failed, result = %{public}d", result);
    }
    return result;
}

bool AbilityManagerService::CheckIsRemote(const std::string& deviceId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (deviceId.empty()) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "CheckIsRemote: deviceId is empty.");
        return false;
    }
    std::string localDeviceId;
    if (!GetLocalDeviceId(localDeviceId)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "CheckIsRemote: get local deviceId failed");
        return false;
    }
    if (localDeviceId == deviceId) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "CheckIsRemote: deviceId is local.");
        return false;
    }
    TAG_LOGD(AAFwkTag::ABILITYMGR, "CheckIsRemote, deviceId = %{public}s", AnonymizeDeviceId(deviceId).c_str());
    return true;
}

bool AbilityManagerService::CheckIfOperateRemote(const Want &want)
{
    std::string deviceId = want.GetElement().GetDeviceID();
    if (deviceId.empty() || want.GetElement().GetBundleName().empty() ||
        want.GetElement().GetAbilityName().empty()) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "CheckIfOperateRemote: DeviceId or BundleName or GetAbilityName empty");
        return false;
    }
    return CheckIsRemote(deviceId);
}

bool AbilityManagerService::GetLocalDeviceId(std::string& localDeviceId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto localNode = std::make_unique<NodeBasicInfo>();
    int32_t errCode = GetLocalNodeDeviceInfo(DM_PKG_NAME, localNode.get());
    if (errCode != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "AbilityManagerService::GetLocalNodeDeviceInfo errCode = %{public}d", errCode);
        return false;
    }
    if (localNode != nullptr) {
        localDeviceId = localNode->networkId;
        TAG_LOGD(AAFwkTag::ABILITYMGR, "get local deviceId, deviceId = %{public}s",
            AnonymizeDeviceId(localDeviceId).c_str());
        return true;
    }
    TAG_LOGE(AAFwkTag::ABILITYMGR, "AbilityManagerService::GetLocalDeviceId localDeviceId null");
    return false;
}

std::string AbilityManagerService::AnonymizeDeviceId(const std::string& deviceId)
{
    if (deviceId.length() < NON_ANONYMIZE_LENGTH) {
        return EMPTY_DEVICE_ID;
    }
    std::string anonDeviceId = deviceId.substr(0, NON_ANONYMIZE_LENGTH);
    anonDeviceId.append("******");
    return anonDeviceId;
}

int AbilityManagerService::MinimizeAbility(const sptr<IRemoteObject> &token, bool fromUser)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "Minimize ability, fromUser:%{public}d.", fromUser);
    if (!VerificationAllToken(token)) {
        return ERR_INVALID_VALUE;
    }

    auto abilityRecord = Token::GetAbilityRecordByToken(token);
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);
    if (!JudgeSelfCalled(abilityRecord)) {
        return CHECK_PERMISSION_FAILED;
    }

    auto type = abilityRecord->GetAbilityInfo().type;
    if (type != AppExecFwk::AbilityType::PAGE) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Cannot minimize except page ability.");
        return ERR_WRONG_INTERFACE_CALL;
    }

    if (!IsAbilityControllerForeground(abilityRecord->GetAbilityInfo().bundleName)) {
        return ERR_WOULD_BLOCK;
    }

    auto missionListManager = GetMissionListManagerByUserId(abilityRecord->GetOwnerMissionUserId());
    if (!missionListManager) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "missionListManager is Null.");
        return ERR_INVALID_VALUE;
    }
    return missionListManager->MinimizeAbility(token, fromUser);
}

int AbilityManagerService::MinimizeUIExtensionAbility(const sptr<SessionInfo> &extensionSessionInfo,
    bool fromUser)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Minimize ui extension ability, fromUser:%{public}d.", fromUser);
    CHECK_POINTER_AND_RETURN(extensionSessionInfo, ERR_INVALID_VALUE);
    auto abilityRecord = Token::GetAbilityRecordByToken(extensionSessionInfo->callerToken);
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);
    if (!JudgeSelfCalled(abilityRecord)) {
        return CHECK_PERMISSION_FAILED;
    }

    std::shared_ptr<AbilityConnectManager> connectManager;
    std::shared_ptr<AbilityRecord> targetRecord;
    GetConnectManagerAndUIExtensionBySessionInfo(extensionSessionInfo, connectManager, targetRecord);
    CHECK_POINTER_AND_RETURN(targetRecord, ERR_INVALID_VALUE);
    CHECK_POINTER_AND_RETURN(connectManager, ERR_INVALID_VALUE);

    auto result = JudgeAbilityVisibleControl(targetRecord->GetAbilityInfo());
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "JudgeAbilityVisibleControl error.");
        return result;
    }

    if (!UIExtensionUtils::IsUIExtension(targetRecord->GetAbilityInfo().extensionAbilityType)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Cannot minimize except ui extension ability.");
        return ERR_WRONG_INTERFACE_CALL;
    }
    extensionSessionInfo->uiExtensionComponentId = (
        static_cast<uint64_t>(abilityRecord->GetRecordId()) << OFFSET) |
        static_cast<uint64_t>(extensionSessionInfo->persistentId);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "UIExtension component id: %{public}" PRId64 ", element: %{public}s.",
        extensionSessionInfo->uiExtensionComponentId, extensionSessionInfo->want.GetElement().GetURI().c_str());
    connectManager->BackgroundAbilityWindowLocked(targetRecord, extensionSessionInfo);
    return ERR_OK;
}

int AbilityManagerService::MinimizeUIAbilityBySCB(const sptr<SessionInfo> &sessionInfo, bool fromUser)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call");
    if (sessionInfo == nullptr || sessionInfo->sessionToken == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "sessionInfo is nullptr");
        return ERR_INVALID_VALUE;
    }

    if (!IsCallerSceneBoard()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Not sceneboard called, not allowed.");
        return ERR_WRONG_INTERFACE_CALL;
    }

    auto uiAbilityManager = GetUIAbilityManagerByUid(IPCSkeleton::GetCallingUid());
    CHECK_POINTER_AND_RETURN(uiAbilityManager, ERR_INVALID_VALUE);
    auto abilityRecord = uiAbilityManager->GetUIAbilityRecordBySessionInfo(sessionInfo);
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);
    if (!IsAbilityControllerForeground(abilityRecord->GetAbilityInfo().bundleName)) {
        return ERR_WOULD_BLOCK;
    }

    return uiAbilityManager->MinimizeUIAbility(abilityRecord, fromUser);
}

int AbilityManagerService::ConnectAbility(
    const Want &want, const sptr<IAbilityConnection> &connect, const sptr<IRemoteObject> &callerToken, int32_t userId)
{
    return ConnectAbilityCommon(want, connect, callerToken, AppExecFwk::ExtensionAbilityType::SERVICE, userId);
}

int AbilityManagerService::ConnectAbilityCommon(
    const Want &want, const sptr<IAbilityConnection> &connect, const sptr<IRemoteObject> &callerToken,
    AppExecFwk::ExtensionAbilityType extensionType, int32_t userId, bool isQueryExtensionOnly)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGI(AAFwkTag::ABILITYMGR,
        "Connect ability called, element uri: %{public}s.", want.GetElement().GetURI().c_str());
    CHECK_POINTER_AND_RETURN(connect, ERR_INVALID_VALUE);
    CHECK_POINTER_AND_RETURN(connect->AsObject(), ERR_INVALID_VALUE);
    if (extensionType == AppExecFwk::ExtensionAbilityType::SERVICE && IsCrossUserCall(userId)) {
        CHECK_CALLER_IS_SYSTEM_APP;
    }
    EventInfo eventInfo = BuildEventInfo(want, userId);

    int result;
#ifdef WITH_DLP
    result = CheckDlpForExtension(want, callerToken, userId, eventInfo, EventName::CONNECT_SERVICE_ERROR);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "CheckDlpForExtension error.");
        eventInfo.errCode = result;
        EventReport::SendExtensionEvent(EventName::CONNECT_SERVICE_ERROR, HiSysEventType::FAULT, eventInfo);
        return result;
    }
#endif // WITH_DLP

    AbilityInterceptorParam interceptorParam = AbilityInterceptorParam(want, 0, GetUserId(), false, nullptr);
    result = interceptorExecuter_ == nullptr ? ERR_INVALID_VALUE :
        interceptorExecuter_->DoProcess(interceptorParam);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "interceptorExecuter_ is nullptr or DoProcess return error.");
        eventInfo.errCode = result;
        EventReport::SendExtensionEvent(EventName::CONNECT_SERVICE_ERROR, HiSysEventType::FAULT, eventInfo);
        return result;
    }

    int32_t validUserId = GetValidUserId(userId);

    if (AbilityUtil::IsStartFreeInstall(want) && freeInstallManager_ != nullptr) {
        std::string localDeviceId;
        if (!GetLocalDeviceId(localDeviceId)) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "%{public}s: Get Local DeviceId failed", __func__);
            eventInfo.errCode = ERR_INVALID_VALUE;
            EventReport::SendExtensionEvent(EventName::CONNECT_SERVICE_ERROR, HiSysEventType::FAULT, eventInfo);
            return ERR_INVALID_VALUE;
        }
        result = freeInstallManager_->ConnectFreeInstall(want, validUserId, callerToken, localDeviceId);
        if (result != ERR_OK) {
            eventInfo.errCode = result;
            EventReport::SendExtensionEvent(EventName::CONNECT_SERVICE_ERROR, HiSysEventType::FAULT, eventInfo);
            return result;
        }
    }

    Want abilityWant = want;
    AbilityRequest abilityRequest;
    std::string uri = abilityWant.GetUri().ToString();
    bool isFileUri = (abilityWant.GetUri().GetScheme() == "file");
    if (!uri.empty() && !isFileUri) {
        // if the want include uri, it may only has uri information. it is probably a datashare extension.
        TAG_LOGD(AAFwkTag::ABILITYMGR,
            "%{public}s called. uri:%{public}s, userId %{public}d", __func__, uri.c_str(), validUserId);
        AppExecFwk::ExtensionAbilityInfo extensionInfo;
        auto bms = GetBundleManager();
        CHECK_POINTER_AND_RETURN(bms, ERR_INVALID_VALUE);

        abilityWant.SetParam("abilityConnectionObj", connect->AsObject());
        abilityWant.RemoveParam("abilityConnectionObj");

        bool queryResult = IN_PROCESS_CALL(bms->QueryExtensionAbilityInfoByUri(uri, validUserId, extensionInfo));
        if (!queryResult || extensionInfo.name.empty() || extensionInfo.bundleName.empty()) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "Invalid extension ability info.");
            eventInfo.errCode = ERR_INVALID_VALUE;
            EventReport::SendExtensionEvent(EventName::CONNECT_SERVICE_ERROR, HiSysEventType::FAULT, eventInfo);
            return ERR_INVALID_VALUE;
        }
        abilityWant.SetElementName(extensionInfo.bundleName, extensionInfo.name);
    }

    if (CheckIfOperateRemote(abilityWant)) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "AbilityManagerService::ConnectAbility. try to ConnectRemoteAbility");
        eventInfo.errCode = ConnectRemoteAbility(abilityWant, callerToken, connect->AsObject());
        if (eventInfo.errCode != ERR_OK) {
            EventReport::SendExtensionEvent(EventName::CONNECT_SERVICE_ERROR, HiSysEventType::FAULT, eventInfo);
        }
        return eventInfo.errCode;
    }
    UpdateCallerInfo(abilityWant, callerToken);

    if (callerToken != nullptr && callerToken->GetObjectDescriptor() != u"ohos.aafwk.AbilityToken") {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "invalid Token.");
        eventInfo.errCode = ConnectLocalAbility(abilityWant, validUserId, connect, nullptr, extensionType);
        if (eventInfo.errCode != ERR_OK) {
            EventReport::SendExtensionEvent(EventName::CONNECT_SERVICE_ERROR, HiSysEventType::FAULT, eventInfo);
        }
        return eventInfo.errCode;
    }
    eventInfo.errCode = ConnectLocalAbility(abilityWant, validUserId, connect, callerToken, extensionType, nullptr,
        isQueryExtensionOnly);
    if (eventInfo.errCode != ERR_OK) {
        EventReport::SendExtensionEvent(EventName::CONNECT_SERVICE_ERROR, HiSysEventType::FAULT, eventInfo);
    }
    return eventInfo.errCode;
}

int AbilityManagerService::ConnectUIExtensionAbility(const Want &want, const sptr<IAbilityConnection> &connect,
    const sptr<SessionInfo> &sessionInfo, int32_t userId, sptr<UIExtensionAbilityConnectInfo> connectInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::ABILITYMGR,
        "Connect ui extension called, bundlename: %{public}s, ability is %{public}s, userId is %{pravite}d",
        want.GetElement().GetBundleName().c_str(), want.GetElement().GetAbilityName().c_str(), userId);
    CHECK_POINTER_AND_RETURN(connect, ERR_INVALID_VALUE);
    CHECK_POINTER_AND_RETURN(connect->AsObject(), ERR_INVALID_VALUE);
    CHECK_POINTER_AND_RETURN(sessionInfo, ERR_INVALID_VALUE);

    if (IsCrossUserCall(userId)) {
        CHECK_CALLER_IS_SYSTEM_APP;
    }

    EventInfo eventInfo = BuildEventInfo(want, userId);
    sptr<IRemoteObject> callerToken = sessionInfo->callerToken;

    if (callerToken != nullptr && !VerificationAllToken(callerToken)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "ConnectUIExtensionAbility VerificationAllToken failed.");
        eventInfo.errCode = ERR_INVALID_VALUE;
        EventReport::SendExtensionEvent(EventName::CONNECT_SERVICE_ERROR, HiSysEventType::FAULT, eventInfo);
        return ERR_INVALID_CALLER;
    }

    int result;
#ifdef WITH_DLP
    result = CheckDlpForExtension(want, callerToken, userId, eventInfo, EventName::CONNECT_SERVICE_ERROR);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "CheckDlpForExtension error.");
        eventInfo.errCode = result;
        EventReport::SendExtensionEvent(EventName::CONNECT_SERVICE_ERROR, HiSysEventType::FAULT, eventInfo);
        return result;
    }
#endif // WITH_DLP

    AbilityInterceptorParam interceptorParam = AbilityInterceptorParam(want, 0, GetUserId(), false, nullptr);
    result = interceptorExecuter_ == nullptr ? ERR_INVALID_VALUE :
        interceptorExecuter_->DoProcess(interceptorParam);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "interceptorExecuter_ is nullptr or DoProcess return error.");
        eventInfo.errCode = result;
        EventReport::SendExtensionEvent(EventName::CONNECT_SERVICE_ERROR, HiSysEventType::FAULT, eventInfo);
        return result;
    }

    int32_t validUserId = GetValidUserId(userId);

    Want abilityWant = want;
    AbilityRequest abilityRequest;
    std::string uri = abilityWant.GetUri().ToString();
    if (!uri.empty()) {
        // if the want include uri, it may only has uri information.
        TAG_LOGI(AAFwkTag::ABILITYMGR,
            "%{public}s called. uri:%{public}s, userId %{public}d", __func__, uri.c_str(), validUserId);
        AppExecFwk::ExtensionAbilityInfo extensionInfo;
        auto bms = GetBundleManager();
        CHECK_POINTER_AND_RETURN(bms, ERR_INVALID_VALUE);

        abilityWant.SetParam("abilityConnectionObj", connect->AsObject());
        abilityWant.RemoveParam("abilityConnectionObj");

        bool queryResult = IN_PROCESS_CALL(bms->QueryExtensionAbilityInfoByUri(uri, validUserId, extensionInfo));
        if (!queryResult || extensionInfo.name.empty() || extensionInfo.bundleName.empty()) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "Invalid extension ability info.");
            eventInfo.errCode = ERR_INVALID_VALUE;
            EventReport::SendExtensionEvent(EventName::CONNECT_SERVICE_ERROR, HiSysEventType::FAULT, eventInfo);
            return ERR_INVALID_VALUE;
        }
        abilityWant.SetElementName(extensionInfo.bundleName, extensionInfo.name);
    }

    UpdateCallerInfo(abilityWant, callerToken);

    if (callerToken != nullptr && callerToken->GetObjectDescriptor() != u"ohos.aafwk.AbilityToken") {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "%{public}s invalid Token.", __func__);
        eventInfo.errCode = ConnectLocalAbility(abilityWant, validUserId, connect, nullptr,
            AppExecFwk::ExtensionAbilityType::UI, sessionInfo, false, connectInfo);
        if (eventInfo.errCode != ERR_OK) {
            EventReport::SendExtensionEvent(EventName::CONNECT_SERVICE_ERROR, HiSysEventType::FAULT, eventInfo);
        }
        return eventInfo.errCode;
    }
    eventInfo.errCode = ConnectLocalAbility(abilityWant, validUserId, connect, callerToken,
        AppExecFwk::ExtensionAbilityType::UI, sessionInfo, false, connectInfo);
    if (eventInfo.errCode != ERR_OK) {
        EventReport::SendExtensionEvent(EventName::CONNECT_SERVICE_ERROR, HiSysEventType::FAULT, eventInfo);
    }
    return eventInfo.errCode;
}

EventInfo AbilityManagerService::BuildEventInfo(const Want &want, int32_t userId)
{
    EventInfo eventInfo;
    eventInfo.userId = userId;
    eventInfo.bundleName = want.GetElement().GetBundleName();
    eventInfo.moduleName = want.GetElement().GetModuleName();
    eventInfo.abilityName = want.GetElement().GetAbilityName();
    return eventInfo;
}

int AbilityManagerService::DisconnectAbility(sptr<IAbilityConnection> connect)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Disconnect ability begin.");
    EventInfo eventInfo;
    CHECK_POINTER_AND_RETURN(connect, ERR_INVALID_VALUE);
    CHECK_POINTER_AND_RETURN(connect->AsObject(), ERR_INVALID_VALUE);

    if (ERR_OK != DisconnectLocalAbility(connect) &&
        ERR_OK != DisconnectRemoteAbility(connect->AsObject())) {
        eventInfo.errCode = INNER_ERR;
        EventReport::SendExtensionEvent(EventName::DISCONNECT_SERVICE_ERROR, HiSysEventType::FAULT, eventInfo);
    }
    return ERR_OK;
}

int AbilityManagerService::ConnectLocalAbility(const Want &want, const int32_t userId,
    const sptr<IAbilityConnection> &connect, const sptr<IRemoteObject> &callerToken,
    AppExecFwk::ExtensionAbilityType extensionType, const sptr<SessionInfo> &sessionInfo,
    bool isQueryExtensionOnly, sptr<UIExtensionAbilityConnectInfo> connectInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "called");
    AbilityUtil::RemoveShowModeKey(const_cast<Want &>(want));
    bool isEnterpriseAdmin = AAFwk::UIExtensionUtils::IsEnterpriseAdmin(extensionType);
    if (!isEnterpriseAdmin && !JudgeMultiUserConcurrency(userId)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Multi-user non-concurrent mode is not satisfied.");
        return ERR_CROSS_USER;
    }

    AbilityRequest abilityRequest;
    ErrCode result = ERR_OK;
    if (isQueryExtensionOnly ||
        AAFwk::UIExtensionUtils::IsUIExtension(extensionType)) {
        result = GenerateExtensionAbilityRequest(want, abilityRequest, callerToken, userId);
    } else {
        result = GenerateAbilityRequest(want, DEFAULT_INVAL_VALUE, abilityRequest, callerToken, userId);
    }
    abilityRequest.sessionInfo = sessionInfo;

    Want requestWant = want;
    CHECK_POINTER_AND_RETURN_LOG(connect, ERR_INVALID_VALUE, "connect is nullptr");
    CHECK_POINTER_AND_RETURN_LOG(connect->AsObject(), ERR_INVALID_VALUE, "abilityConnectionObj is nullptr");
    requestWant.SetParam("abilityConnectionObj", connect->AsObject());
    TAG_LOGD(AAFwkTag::ABILITYMGR, "requestWant SetParam success");

    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Generate ability request error.");
        return result;
    }
    if (!UriUtils::GetInstance().CheckNonImplicitShareFileUri(abilityRequest)) {
        return ERR_SHARE_FILE_URI_NON_IMPLICITLY;
    }
    result = CheckPermissionForUIService(want, abilityRequest);
    if (result != ERR_OK) {
        return result;
    }

    if (abilityRequest.abilityInfo.isStageBasedModel) {
        bool isService =
            (abilityRequest.abilityInfo.extensionAbilityType == AppExecFwk::ExtensionAbilityType::SERVICE);
        if (isService && extensionType != AppExecFwk::ExtensionAbilityType::SERVICE) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "Service extension type, please use ConnectAbility.");
            return ERR_WRONG_INTERFACE_CALL;
        }
    }
    auto abilityInfo = abilityRequest.abilityInfo;
    int32_t validUserId = abilityInfo.applicationInfo.singleton ? U0_USER_ID : userId;
    TAG_LOGD(AAFwkTag::ABILITYMGR, "validUserId : %{public}d, singleton is : %{public}d",
        validUserId, static_cast<int>(abilityInfo.applicationInfo.singleton));

    result = CheckStaticCfgPermission(abilityRequest, false, -1);
    if (result != AppExecFwk::Constants::PERMISSION_GRANTED) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "CheckStaticCfgPermission error, result is %{public}d.", result);
        return ERR_STATIC_CFG_PERMISSION;
    }

    AppExecFwk::ExtensionAbilityType targetExtensionType = abilityInfo.extensionAbilityType;
    TAG_LOGD(AAFwkTag::ABILITYMGR, "extension type %{public}d.", targetExtensionType);
    if (AAFwk::UIExtensionUtils::IsUIExtension(extensionType)) {
        if (!AAFwk::UIExtensionUtils::IsUIExtension(targetExtensionType)
            && targetExtensionType != AppExecFwk::ExtensionAbilityType::WINDOW) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "Try to connect UI extension, but target ability is not UI extension.");
            return ERR_WRONG_INTERFACE_CALL;
        }

        // Cause window has used this api, don't check it when type is window.
        if (targetExtensionType != AppExecFwk::ExtensionAbilityType::WINDOW &&
            !PermissionVerification::GetInstance()->VerifyCallingPermission(
                PermissionConstants::PERMISSION_CONNECT_UI_EXTENSION_ABILITY)) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "Permission %{public}s verification failed.",
                PermissionConstants::PERMISSION_CONNECT_UI_EXTENSION_ABILITY);
            return ERR_PERMISSION_DENIED;
        }
    }

    auto type = abilityInfo.type;
    if (type != AppExecFwk::AbilityType::SERVICE && type != AppExecFwk::AbilityType::EXTENSION) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Connect ability failed, target ability is not Service.");
        return TARGET_ABILITY_NOT_SERVICE;
    }

    AbilityInterceptorParam afterCheckParam = AbilityInterceptorParam(abilityRequest.want, 0, GetUserId(),
        false, callerToken, std::make_shared<AppExecFwk::AbilityInfo>(abilityInfo));
    result = afterCheckExecuter_ == nullptr ? ERR_INVALID_VALUE :
        afterCheckExecuter_->DoProcess(afterCheckParam);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "afterCheckExecuter_ is nullptr or DoProcess return error.");
        return result;
    }

    result = CheckCallServicePermission(abilityRequest);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "%{public}s CheckCallServicePermission error.", __func__);
        return result;
    }

    if (!ExtensionPermissionsUtil::CheckSAPermission(targetExtensionType)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "The SA doesn't have permission for target extension.");
        return CHECK_PERMISSION_FAILED;
    }

    result = PreLoadAppDataAbilities(abilityInfo.bundleName, validUserId);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "ConnectAbility: App data ability preloading failed, '%{public}s', %{public}d",
            abilityInfo.bundleName.c_str(),
            result);
        return result;
    }

    auto connectManager = GetConnectManagerByUserId(validUserId);
    if (connectManager == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "connectManager is nullptr. userId=%{public}d", validUserId);
        return ERR_INVALID_VALUE;
    }

    ReportEventToRSS(abilityInfo, callerToken);
    return connectManager->ConnectAbilityLocked(abilityRequest, connect, callerToken, sessionInfo, connectInfo);
}

int AbilityManagerService::ConnectRemoteAbility(Want &want, const sptr<IRemoteObject> &callerToken,
    const sptr<IRemoteObject> &connect)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "%{public}s begin ConnectAbilityRemote", __func__);
    if (AddStartControlParam(want, callerToken) != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "%{public}s AddStartControlParam failed.", __func__);
        return ERR_INVALID_VALUE;
    }
    DistributedClient dmsClient;
    return dmsClient.ConnectRemoteAbility(want, connect);
}

int AbilityManagerService::DisconnectLocalAbility(const sptr<IAbilityConnection> &connect)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "called");
    auto currentConnectManager = GetCurrentConnectManager();
    CHECK_POINTER_AND_RETURN(currentConnectManager, ERR_NO_INIT);
    if (currentConnectManager->DisconnectAbilityLocked(connect) == ERR_OK) {
        return ERR_OK;
    }
    // If current connectManager does not exist connect, then try connectManagerU0
    auto connectManager = GetConnectManagerByUserId(U0_USER_ID);
    CHECK_POINTER_AND_RETURN(connectManager, ERR_NO_INIT);
    if (connectManager->DisconnectAbilityLocked(connect) == ERR_OK) {
        return ERR_OK;
    }

    auto userId = IPCSkeleton::GetCallingUid() / BASE_USER_RANGE;
    if (userId == U0_USER_ID) {
        auto connectManagers = GetConnectManagers();
        for (auto& item : connectManagers) {
            if (item.second && item.second->DisconnectAbilityLocked(connect) == ERR_OK) {
                return ERR_OK;
            }
        }
    }

    // EnterpriseAdminExtensionAbility Scene
    connectManager = GetConnectManagerByUserId(USER_ID_DEFAULT);
    CHECK_POINTER_AND_RETURN(connectManager, ERR_NO_INIT);
    return connectManager->DisconnectAbilityLocked(connect);
}

int AbilityManagerService::DisconnectRemoteAbility(const sptr<IRemoteObject> &connect)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "%{public}s begin DisconnectAbilityRemote", __func__);
    int32_t callerUid = IPCSkeleton::GetCallingUid();
    uint32_t accessToken = IPCSkeleton::GetCallingTokenID();
    DistributedClient dmsClient;
    return dmsClient.DisconnectRemoteAbility(connect, callerUid, accessToken);
}

int AbilityManagerService::ContinueMission(const std::string &srcDeviceId, const std::string &dstDeviceId,
    int32_t missionId, const sptr<IRemoteObject> &callBack, AAFwk::WantParams &wantParams)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "ContinueMission missionId: %{public}d", missionId);
    if (!PermissionVerification::GetInstance()->VerifyMissionPermission()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "%{public}s: Permission verification failed", __func__);
        return CHECK_PERMISSION_FAILED;
    }

    DistributedClient dmsClient;
    return dmsClient.ContinueMission(srcDeviceId, dstDeviceId, missionId, callBack, wantParams);
}

int AbilityManagerService::ContinueMission(AAFwk::ContinueMissionInfo continueMissionInfo,
    const sptr<IRemoteObject> &callback)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "called");
    AAFWK::ContinueRadar::GetInstance().ClickIconContinue("ContinueMission");
    if (!PermissionVerification::GetInstance()->VerifyMissionPermission()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "%{public}s: Permission verification failed", __func__);
        return CHECK_PERMISSION_FAILED;
    }

    DistributedClient dmsClient;
    return dmsClient.ContinueMission(continueMissionInfo, callback);
}

int AbilityManagerService::ContinueAbility(const std::string &deviceId, int32_t missionId, uint32_t versionCode)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR,
        "ContinueAbility missionId = %{public}d, version = %{public}u.", missionId, versionCode);
    if (!CheckCallerIsDmsProcess()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Check processName failed");
        return ERR_INVALID_VALUE;
    }

    std::shared_ptr<AbilityRecord> abilityRecord = nullptr;
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        auto uiAbilityManager = GetCurrentUIAbilityManager();
        CHECK_POINTER_AND_RETURN(uiAbilityManager, ERR_INVALID_VALUE);
        abilityRecord = uiAbilityManager->GetAbilityRecordsById(missionId);
    } else {
        sptr<IRemoteObject> abilityToken = GetAbilityTokenByMissionId(missionId);
        CHECK_POINTER_AND_RETURN(abilityToken, ERR_INVALID_VALUE);
        abilityRecord = Token::GetAbilityRecordByToken(abilityToken);
    }
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);

    abilityRecord->ContinueAbility(deviceId, versionCode);
    return ERR_OK;
}

int AbilityManagerService::StartContinuation(const Want &want, const sptr<IRemoteObject> &abilityToken, int32_t status)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "Start Continuation.");
    if (!CheckIfOperateRemote(want)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "deviceId or bundle name or abilityName empty");
        return ERR_INVALID_VALUE;
    }
    CHECK_POINTER_AND_RETURN(abilityToken, ERR_INVALID_VALUE);

    int32_t appUid = IPCSkeleton::GetCallingUid();
    uint32_t accessToken = IPCSkeleton::GetCallingTokenID();
        TAG_LOGI(AAFwkTag::ABILITYMGR,
            "AbilityManagerService::Try to StartContinuation");
    int32_t missionId = -1;
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        missionId = GetMissionIdByAbilityTokenInner(abilityToken);
        if (!missionId) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "Invalid missionId id.");
            return ERR_INVALID_VALUE;
        }
    } else {
        missionId = GetMissionIdByAbilityToken(abilityToken);
    }
    if (missionId < 0) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "AbilityManagerService::StartContinuation failed to get missionId.");
        return ERR_INVALID_VALUE;
    }
    AAFWK::ContinueRadar::GetInstance().SaveDataRemoteWant("StartContinuation");
    DistributedClient dmsClient;
    auto result =  dmsClient.StartContinuation(want, missionId, appUid, status, accessToken);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "StartContinuation failed, result = %{public}d, notify caller", result);
        NotifyContinuationResult(missionId, result);
    }
    return result;
}

void AbilityManagerService::NotifyCompleteContinuation(const std::string &deviceId,
    int32_t sessionId, bool isSuccess)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "NotifyCompleteContinuation.");
    AAFWK::ContinueRadar::GetInstance().ClickIconRecvOver("NotifyCompleteContinuation");
    sptr<ISystemAbilityManager> samgrProxy = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgrProxy == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "failed to get samgrProxy");
        return;
    }
    sptr<IRemoteObject> bmsProxy = samgrProxy->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    if (bmsProxy == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "failed to get bms from samgr");
        return;
    }
    auto bundleMgr = iface_cast<AppExecFwk::IBundleMgr>(bmsProxy);
    if (bundleMgr == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "failed to get bms");
        return;
    }
    int32_t callerUid = IPCSkeleton::GetCallingUid();
    std::string callerBundleName;
    // reset ipc identity
    auto identity = IPCSkeleton::ResetCallingIdentity();
    bool result = bundleMgr->GetBundleNameForUid(callerUid, callerBundleName);
    // set ipc identity to raw
    IPCSkeleton::SetCallingIdentity(identity);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "callerBundleName: %{public}s", callerBundleName.c_str());
    DistributedClient dmsClient;
    dmsClient.NotifyCompleteContinuation(Str8ToStr16(deviceId), sessionId, isSuccess, callerBundleName);
}

int AbilityManagerService::NotifyContinuationResult(int32_t missionId, int32_t result)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "Notify Continuation Result : %{public}d.", result);

    std::shared_ptr<AbilityRecord> abilityRecord = nullptr;
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        auto uiAbilityManager = GetCurrentUIAbilityManager();
        CHECK_POINTER_AND_RETURN(uiAbilityManager, ERR_INVALID_VALUE);
        abilityRecord = uiAbilityManager->GetAbilityRecordsById(missionId);
    } else {
        sptr<IRemoteObject> abilityToken = GetAbilityTokenByMissionId(missionId);
        CHECK_POINTER_AND_RETURN(abilityToken, ERR_INVALID_VALUE);
        abilityRecord = Token::GetAbilityRecordByToken(abilityToken);
    }
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);

    if (!JudgeSelfCalled(abilityRecord) && !CheckCallerIsDmsProcess()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Permission deny.");
        return ERR_INVALID_VALUE;
    }
    abilityRecord->NotifyContinuationResult(result);
    return ERR_OK;
}

int AbilityManagerService::StartSyncRemoteMissions(const std::string& devId, bool fixConflict, int64_t tag)
{
    if (!PermissionVerification::GetInstance()->VerifyMissionPermission()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "%{public}s: Permission verification failed", __func__);
        return CHECK_PERMISSION_FAILED;
    }
    DistributedClient dmsClient;
    return dmsClient.StartSyncRemoteMissions(devId, fixConflict, tag);
}

int AbilityManagerService::StopSyncRemoteMissions(const std::string& devId)
{
    if (!PermissionVerification::GetInstance()->VerifyMissionPermission()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "%{public}s: Permission verification failed", __func__);
        return CHECK_PERMISSION_FAILED;
    }
    DistributedClient dmsClient;
    return dmsClient.StopSyncRemoteMissions(devId);
}

int AbilityManagerService::RegisterObserver(const sptr<AbilityRuntime::IConnectionObserver> &observer)
{
    if (!PermissionVerification::GetInstance()->CheckObserverCallerPermission()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Permission verification failed");
        return CHECK_PERMISSION_FAILED;
    }
    return DelayedSingleton<ConnectionStateManager>::GetInstance()->RegisterObserver(observer);
}

int AbilityManagerService::UnregisterObserver(const sptr<AbilityRuntime::IConnectionObserver> &observer)
{
    if (!PermissionVerification::GetInstance()->CheckObserverCallerPermission()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Permission verification failed");
        return CHECK_PERMISSION_FAILED;
    }
    return DelayedSingleton<ConnectionStateManager>::GetInstance()->UnregisterObserver(observer);
}

#ifdef WITH_DLP
int AbilityManagerService::GetDlpConnectionInfos(std::vector<AbilityRuntime::DlpConnectionInfo> &infos)
{
    if (!AAFwk::PermissionVerification::GetInstance()->IsSACall()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "can not get dlp connection infos if caller is not sa.");
        return CHECK_PERMISSION_FAILED;
    }
    DelayedSingleton<ConnectionStateManager>::GetInstance()->GetDlpConnectionInfos(infos);

    return ERR_OK;
}
#endif // WITH_DLP

int AbilityManagerService::GetConnectionData(std::vector<AbilityRuntime::ConnectionData> &connectionData)
{
    if (!AAFwk::PermissionVerification::GetInstance()->IsSACall()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "can not get connection data if caller is not sa.");
        return CHECK_PERMISSION_FAILED;
    }
    DelayedSingleton<ConnectionStateManager>::GetInstance()->GetConnectionData(connectionData);

    return ERR_OK;
}

int AbilityManagerService::RegisterMissionListener(const std::string &deviceId,
    const sptr<IRemoteMissionListener> &listener)
{
    CHECK_CALLER_IS_SYSTEM_APP;
    std::string localDeviceId;
    if (!GetLocalDeviceId(localDeviceId) || localDeviceId == deviceId) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "RegisterMissionListener: Check DeviceId failed");
        return REGISTER_REMOTE_MISSION_LISTENER_FAIL;
    }
    CHECK_POINTER_AND_RETURN(listener, ERR_INVALID_VALUE);
    if (!PermissionVerification::GetInstance()->VerifyMissionPermission()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "%{public}s: Permission verification failed", __func__);
        return CHECK_PERMISSION_FAILED;
    }
    DistributedClient dmsClient;
    return dmsClient.RegisterMissionListener(Str8ToStr16(deviceId), listener->AsObject());
}

int AbilityManagerService::RegisterOnListener(const std::string &type,
    const sptr<IRemoteOnListener> &listener)
{
    CHECK_CALLER_IS_SYSTEM_APP;
    CHECK_POINTER_AND_RETURN(listener, ERR_INVALID_VALUE);
    if (!PermissionVerification::GetInstance()->VerifyMissionPermission()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "%{public}s: Permission verification failed", __func__);
        return CHECK_PERMISSION_FAILED;
    }
    DistributedClient dmsClient;
    return dmsClient.RegisterOnListener(type, listener->AsObject());
}

int AbilityManagerService::RegisterOffListener(const std::string &type,
    const sptr<IRemoteOnListener> &listener)
{
    CHECK_CALLER_IS_SYSTEM_APP;
    CHECK_POINTER_AND_RETURN(listener, ERR_INVALID_VALUE);
    if (!PermissionVerification::GetInstance()->VerifyMissionPermission()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "%{public}s: Permission verification failed", __func__);
        return CHECK_PERMISSION_FAILED;
    }
    DistributedClient dmsClient;
    return dmsClient.RegisterOffListener(type, listener->AsObject());
}

int AbilityManagerService::UnRegisterMissionListener(const std::string &deviceId,
    const sptr<IRemoteMissionListener> &listener)
{
    CHECK_CALLER_IS_SYSTEM_APP;
    std::string localDeviceId;
    if (!GetLocalDeviceId(localDeviceId) || localDeviceId == deviceId) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "RegisterMissionListener: Check DeviceId failed");
        return REGISTER_REMOTE_MISSION_LISTENER_FAIL;
    }
    CHECK_POINTER_AND_RETURN(listener, ERR_INVALID_VALUE);
    if (!PermissionVerification::GetInstance()->VerifyMissionPermission()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "%{public}s: Permission verification failed", __func__);
        return CHECK_PERMISSION_FAILED;
    }
    DistributedClient dmsClient;
    return dmsClient.UnRegisterMissionListener(Str8ToStr16(deviceId), listener->AsObject());
}

sptr<IWantSender> AbilityManagerService::GetWantSender(
    const WantSenderInfo &wantSenderInfo, const sptr<IRemoteObject> &callerToken)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "called");
    auto pendingWantManager = GetCurrentPendingWantManager();
    CHECK_POINTER_AND_RETURN(pendingWantManager, nullptr);

    auto bms = GetBundleManager();
    CHECK_POINTER_AND_RETURN(bms, nullptr);

    int32_t callerUid = IPCSkeleton::GetCallingUid();
    int32_t userId = wantSenderInfo.userId;
    int32_t bundleMgrResult = 0;
    if (userId < 0) {
        if (DelayedSingleton<AppExecFwk::OsAccountManagerWrapper>::GetInstance()->
            GetOsAccountLocalIdFromUid(callerUid, userId) != 0) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "GetOsAccountLocalIdFromUid failed. uid=%{public}d", callerUid);
            return nullptr;
        }
    }

    int32_t appUid = 0;
    int32_t appIndex = 0;
    if (!wantSenderInfo.allWants.empty()) {
        AppExecFwk::BundleInfo bundleInfo;
        std::string bundleName = wantSenderInfo.allWants.back().want.GetElement().GetBundleName();
        GetRunningMultiAppIndex(bundleName, callerUid, appIndex);
        bundleMgrResult = IN_PROCESS_CALL(bms->GetCloneBundleInfo(bundleName,
            static_cast<int32_t>(AppExecFwk::GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_APPLICATION),
            appIndex, bundleInfo, userId));
        if (bundleMgrResult == ERR_OK) {
            appUid = bundleInfo.uid;
        }
        TAG_LOGD(AAFwkTag::ABILITYMGR, "App bundleName: %{public}s, uid: %{public}d", bundleName.c_str(), appUid);
    }
    if (!CheckSenderWantInfo(callerUid, wantSenderInfo)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "check bundleName failed");
        return nullptr;
    }

    bool isSystemApp = false;
    if (!wantSenderInfo.bundleName.empty()) {
        AppExecFwk::BundleInfo bundleInfo;
        bundleMgrResult = IN_PROCESS_CALL(bms->GetBundleInfo(wantSenderInfo.bundleName,
            AppExecFwk::BundleFlag::GET_BUNDLE_DEFAULT, bundleInfo, userId));
        if (bundleMgrResult) {
            isSystemApp = bundleInfo.applicationInfo.isSystemApp;
        }
    }

    TAG_LOGD(AAFwkTag::ABILITYMGR, "bundleName = %{public}s, appIndex:%{public}d",
        wantSenderInfo.bundleName.c_str(), appIndex);
    return pendingWantManager->GetWantSender(callerUid, appUid, isSystemApp, wantSenderInfo, callerToken, appIndex);
}

int AbilityManagerService::SendWantSender(sptr<IWantSender> target, const SenderInfo &senderInfo)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "Send want sender.");
    auto pendingWantManager = GetCurrentPendingWantManager();
    CHECK_POINTER_AND_RETURN(pendingWantManager, ERR_INVALID_VALUE);
    CHECK_POINTER_AND_RETURN(target, ERR_INVALID_VALUE);
    return pendingWantManager->SendWantSender(target, senderInfo);
}

void AbilityManagerService::CancelWantSender(const sptr<IWantSender> &sender)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "called");
    auto pendingWantManager = GetCurrentPendingWantManager();
    CHECK_POINTER(pendingWantManager);
    CHECK_POINTER(sender);

    sptr<IRemoteObject> obj = sender->AsObject();
    if (!obj || obj->IsProxyObject()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "obj is nullptr or obj is a proxy obj.");
        return;
    }

    auto bms = GetBundleManager();
    CHECK_POINTER(bms);

    int32_t callerUid = IPCSkeleton::GetCallingUid();
    sptr<PendingWantRecord> record = iface_cast<PendingWantRecord>(obj);

    int userId = -1;
    if (DelayedSingleton<AppExecFwk::OsAccountManagerWrapper>::GetInstance()->
        GetOsAccountLocalIdFromUid(callerUid, userId) != 0) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "GetOsAccountLocalIdFromUid failed. uid=%{public}d", callerUid);
        return;
    }
    bool isSystemApp = false;
    if (record->GetKey() != nullptr && !record->GetKey()->GetBundleName().empty()) {
        AppExecFwk::BundleInfo bundleInfo;
        bool bundleMgrResult = IN_PROCESS_CALL(bms->GetBundleInfo(record->GetKey()->GetBundleName(),
            AppExecFwk::BundleFlag::GET_BUNDLE_DEFAULT, bundleInfo, userId));
        if (!bundleMgrResult) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "GetBundleInfo is fail.");
            return;
        }
        isSystemApp = bundleInfo.applicationInfo.isSystemApp;
    }

    pendingWantManager->CancelWantSender(isSystemApp, sender);
}

int AbilityManagerService::GetPendingWantUid(const sptr<IWantSender> &target)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "%{public}s:begin.", __func__);
    auto pendingWantManager = GetCurrentPendingWantManager();
    CHECK_POINTER_AND_RETURN(pendingWantManager, -1);
    if (target == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "%s, target is nullptr", __func__);
        return -1;
    }
    return pendingWantManager->GetPendingWantUid(target);
}

int AbilityManagerService::GetPendingWantUserId(const sptr<IWantSender> &target)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "%{public}s:begin.", __func__);
    auto pendingWantManager = GetCurrentPendingWantManager();
    CHECK_POINTER_AND_RETURN(pendingWantManager, -1);
    if (target == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "%s, target is nullptr", __func__);
        return -1;
    }
    return pendingWantManager->GetPendingWantUserId(target);
}

std::string AbilityManagerService::GetPendingWantBundleName(const sptr<IWantSender> &target)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "Get pending want bundle name.");
    auto pendingWantManager = GetCurrentPendingWantManager();
    CHECK_POINTER_AND_RETURN(pendingWantManager, "");
    CHECK_POINTER_AND_RETURN(target, "");
    return pendingWantManager->GetPendingWantBundleName(target);
}

int AbilityManagerService::GetPendingWantCode(const sptr<IWantSender> &target)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "%{public}s:begin.", __func__);
    auto pendingWantManager = GetCurrentPendingWantManager();
    CHECK_POINTER_AND_RETURN(pendingWantManager, -1);
    if (target == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "%s, target is nullptr", __func__);
        return -1;
    }
    return pendingWantManager->GetPendingWantCode(target);
}

int AbilityManagerService::GetPendingWantType(const sptr<IWantSender> &target)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "%{public}s:begin.", __func__);
    auto pendingWantManager = GetCurrentPendingWantManager();
    CHECK_POINTER_AND_RETURN(pendingWantManager, -1);
    if (target == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "%s, target is nullptr", __func__);
        return -1;
    }
    return pendingWantManager->GetPendingWantType(target);
}

void AbilityManagerService::RegisterCancelListener(const sptr<IWantSender> &sender,
    const sptr<IWantReceiver> &receiver)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "Register cancel listener.");
    auto pendingWantManager = GetCurrentPendingWantManager();
    CHECK_POINTER(pendingWantManager);
    CHECK_POINTER(sender);
    CHECK_POINTER(receiver);
    pendingWantManager->RegisterCancelListener(sender, receiver);
}

void AbilityManagerService::UnregisterCancelListener(
    const sptr<IWantSender> &sender, const sptr<IWantReceiver> &receiver)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "Unregister cancel listener.");
    auto pendingWantManager = GetCurrentPendingWantManager();
    CHECK_POINTER(pendingWantManager);
    CHECK_POINTER(sender);
    CHECK_POINTER(receiver);
    pendingWantManager->UnregisterCancelListener(sender, receiver);
}

int AbilityManagerService::GetPendingRequestWant(const sptr<IWantSender> &target, std::shared_ptr<Want> &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Get pending request want.");
    auto pendingWantManager = GetCurrentPendingWantManager();
    CHECK_POINTER_AND_RETURN(pendingWantManager, ERR_INVALID_VALUE);
    CHECK_POINTER_AND_RETURN(target, ERR_INVALID_VALUE);
    CHECK_POINTER_AND_RETURN(want, ERR_INVALID_VALUE);
    CHECK_CALLER_IS_SYSTEM_APP;
    return pendingWantManager->GetPendingRequestWant(target, want);
}

int AbilityManagerService::LockMissionForCleanup(int32_t missionId)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "request unlock mission for clean up all, id :%{public}d", missionId);
    auto missionListManager = GetCurrentMissionListManager();
    CHECK_POINTER_AND_RETURN(missionListManager, ERR_NO_INIT);
    CHECK_CALLER_IS_SYSTEM_APP;

    if (!PermissionVerification::GetInstance()->VerifyMissionPermission()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "%{public}s: Permission verification failed", __func__);
        return CHECK_PERMISSION_FAILED;
    }
    return missionListManager->SetMissionLockedState(missionId, true);
}

int AbilityManagerService::UnlockMissionForCleanup(int32_t missionId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "request unlock mission for clean up all, id :%{public}d", missionId);
    auto missionListManager = GetCurrentMissionListManager();
    CHECK_POINTER_AND_RETURN(missionListManager, ERR_NO_INIT);
    CHECK_CALLER_IS_SYSTEM_APP;

    if (!PermissionVerification::GetInstance()->VerifyMissionPermission()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "%{public}s: Permission verification failed", __func__);
        return CHECK_PERMISSION_FAILED;
    }
    return missionListManager->SetMissionLockedState(missionId, false);
}

void AbilityManagerService::SetLockedState(int32_t sessionId, bool lockedState)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "request lock abilityRecord, sessionId :%{public}d", sessionId);
    if (!IsCallerSceneBoard()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Not sceneboard called, not allowed.");
        return;
    }
    auto uiAbilityManager = GetUIAbilityManagerByUid(IPCSkeleton::GetCallingUid());
    CHECK_POINTER(uiAbilityManager);
    auto abilityRecord = uiAbilityManager->GetAbilityRecordsById(sessionId);
    if (!abilityRecord) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "abilityRecord is null.");
        return;
    }
    abilityRecord->SetLockedState(lockedState);
}

int AbilityManagerService::RegisterMissionListener(const sptr<IMissionListener> &listener)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "request RegisterMissionListener ");
    auto missionListManager = GetCurrentMissionListManager();
    CHECK_POINTER_AND_RETURN(missionListManager, ERR_NO_INIT);
    CHECK_CALLER_IS_SYSTEM_APP;

    if (!PermissionVerification::GetInstance()->VerifyMissionPermission()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "%{public}s: Permission verification failed", __func__);
        return CHECK_PERMISSION_FAILED;
    }
    return missionListManager->RegisterMissionListener(listener);
}

int AbilityManagerService::UnRegisterMissionListener(const sptr<IMissionListener> &listener)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "request RegisterMissionListener ");
    auto missionListManager = GetCurrentMissionListManager();
    CHECK_POINTER_AND_RETURN(missionListManager, ERR_NO_INIT);
    CHECK_CALLER_IS_SYSTEM_APP;

    if (!PermissionVerification::GetInstance()->VerifyMissionPermission()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "%{public}s: Permission verification failed", __func__);
        return CHECK_PERMISSION_FAILED;
    }
    return missionListManager->UnRegisterMissionListener(listener);
}

int AbilityManagerService::GetMissionInfos(const std::string& deviceId, int32_t numMax,
    std::vector<MissionInfo> &missionInfos)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "request GetMissionInfos.");
    auto missionListManager = GetCurrentMissionListManager();
    CHECK_POINTER_AND_RETURN(missionListManager, ERR_NO_INIT);
    CHECK_CALLER_IS_SYSTEM_APP;

    if (!PermissionVerification::GetInstance()->VerifyMissionPermission()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "%{public}s: Permission verification failed", __func__);
        return CHECK_PERMISSION_FAILED;
    }

    if (CheckIsRemote(deviceId)) {
        return GetRemoteMissionInfos(deviceId, numMax, missionInfos);
    }

    return missionListManager->GetMissionInfos(numMax, missionInfos);
}

int AbilityManagerService::GetRemoteMissionInfos(const std::string& deviceId, int32_t numMax,
    std::vector<MissionInfo> &missionInfos)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "GetRemoteMissionInfos begin");
    DistributedClient dmsClient;
    int result = dmsClient.GetMissionInfos(deviceId, numMax, missionInfos);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "GetRemoteMissionInfos failed, result = %{public}d", result);
        return result;
    }
    return ERR_OK;
}

int AbilityManagerService::GetMissionInfo(const std::string& deviceId, int32_t missionId,
    MissionInfo &missionInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "request GetMissionInfo, missionId:%{public}d", missionId);
    auto missionListManager = GetCurrentMissionListManager();
    CHECK_POINTER_AND_RETURN(missionListManager, ERR_NO_INIT);
    CHECK_CALLER_IS_SYSTEM_APP;

    if (!PermissionVerification::GetInstance()->VerifyMissionPermission()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "%{public}s: Permission verification failed", __func__);
        return CHECK_PERMISSION_FAILED;
    }

    if (CheckIsRemote(deviceId)) {
        return GetRemoteMissionInfo(deviceId, missionId, missionInfo);
    }

    return missionListManager->GetMissionInfo(missionId, missionInfo);
}

int AbilityManagerService::GetRemoteMissionInfo(const std::string& deviceId, int32_t missionId,
    MissionInfo &missionInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "GetMissionInfoFromDms begin");
    std::vector<MissionInfo> missionVector;
    int result = GetRemoteMissionInfos(deviceId, MAX_NUMBER_OF_DISTRIBUTED_MISSIONS, missionVector);
    if (result != ERR_OK) {
        return result;
    }
    for (auto iter = missionVector.begin(); iter != missionVector.end(); iter++) {
        if (iter->id == missionId) {
            missionInfo = *iter;
            return ERR_OK;
        }
    }
    TAG_LOGW(AAFwkTag::ABILITYMGR, "missionId not found");
    return ERR_INVALID_VALUE;
}

int AbilityManagerService::CleanMission(int32_t missionId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "request CleanMission, missionId:%{public}d", missionId);
    auto missionListManager = GetCurrentMissionListManager();
    CHECK_POINTER_AND_RETURN(missionListManager, ERR_NO_INIT);
    CHECK_CALLER_IS_SYSTEM_APP;

    if (!PermissionVerification::GetInstance()->VerifyMissionPermission()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "%{public}s: Permission verification failed", __func__);
        return CHECK_PERMISSION_FAILED;
    }

    return missionListManager->ClearMission(missionId);
}

int AbilityManagerService::CleanAllMissions()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "request CleanAllMissions ");
    auto missionListManager = GetCurrentMissionListManager();
    CHECK_POINTER_AND_RETURN(missionListManager, ERR_NO_INIT);
    CHECK_CALLER_IS_SYSTEM_APP;

    if (!PermissionVerification::GetInstance()->VerifyMissionPermission()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "%{public}s: Permission verification failed", __func__);
        return CHECK_PERMISSION_FAILED;
    }

    Want want;
    want.SetElementName(AbilityConfig::LAUNCHER_BUNDLE_NAME, AbilityConfig::LAUNCHER_ABILITY_NAME);
    if (!IsAbilityControllerStart(want, AbilityConfig::LAUNCHER_BUNDLE_NAME)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "IsAbilityControllerStart failed: %{public}s", want.GetBundle().c_str());
        return ERR_WOULD_BLOCK;
    }

    return missionListManager->ClearAllMissions();
}

int AbilityManagerService::MoveMissionToFront(int32_t missionId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "request MoveMissionToFront, missionId:%{public}d", missionId);
    CHECK_CALLER_IS_SYSTEM_APP;
    if (!PermissionVerification::GetInstance()->VerifyMissionPermission()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "%{public}s: Permission verification failed", __func__);
        return CHECK_PERMISSION_FAILED;
    }

    if (!IsAbilityControllerStartById(missionId)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "IsAbilityControllerStart false");
        return ERR_WOULD_BLOCK;
    }

    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        auto uiAbilityManager = GetUIAbilityManagerByUid(IPCSkeleton::GetCallingUid());
        CHECK_POINTER_AND_RETURN(uiAbilityManager, ERR_INVALID_VALUE);
        return uiAbilityManager->MoveMissionToFront(missionId);
    }

    auto missionListManager = GetCurrentMissionListManager();
    CHECK_POINTER_AND_RETURN(missionListManager, ERR_NO_INIT);
    return missionListManager->MoveMissionToFront(missionId);
}

int AbilityManagerService::MoveMissionToFront(int32_t missionId, const StartOptions &startOptions)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "request MoveMissionToFront, missionId:%{public}d", missionId);
    CHECK_CALLER_IS_SYSTEM_APP;
    if (!PermissionVerification::GetInstance()->VerifyMissionPermission()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "%{public}s: Permission verification failed", __func__);
        return CHECK_PERMISSION_FAILED;
    }

    if (!IsAbilityControllerStartById(missionId)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "IsAbilityControllerStart false");
        return ERR_WOULD_BLOCK;
    }

    auto options = std::make_shared<StartOptions>(startOptions);
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        auto uiAbilityManager = GetUIAbilityManagerByUid(IPCSkeleton::GetCallingUid());
        CHECK_POINTER_AND_RETURN(uiAbilityManager, ERR_INVALID_VALUE);
        return uiAbilityManager->MoveMissionToFront(missionId, options);
    }

    auto missionListManager = GetCurrentMissionListManager();
    CHECK_POINTER_AND_RETURN(missionListManager, ERR_NO_INIT);
    return missionListManager->MoveMissionToFront(missionId, options);
}

int AbilityManagerService::MoveMissionsToForeground(const std::vector<int32_t>& missionIds, int32_t topMissionId)
{
    CHECK_CALLER_IS_SYSTEM_APP;
    if (!PermissionVerification::GetInstance()->VerifyMissionPermission()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "%{public}s: Permission verification failed", __func__);
        return CHECK_PERMISSION_FAILED;
    }
#ifdef SUPPORT_SCREEN
    if (wmsHandler_) {
        auto ret = wmsHandler_->MoveMissionsToForeground(missionIds, topMissionId);
        if (ret) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "MoveMissionsToForeground failed, missiondIds may be invalid");
            return ERR_INVALID_VALUE;
        } else {
            return NO_ERROR;
        }
    }
#endif // SUPPORT_SCREEN
    return ERR_NO_INIT;
}

int AbilityManagerService::MoveMissionsToBackground(const std::vector<int32_t>& missionIds,
    std::vector<int32_t>& result)
{
    CHECK_CALLER_IS_SYSTEM_APP;
    if (!PermissionVerification::GetInstance()->VerifyMissionPermission()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "%{public}s: Permission verification failed", __func__);
        return CHECK_PERMISSION_FAILED;
    }
#ifdef SUPPORT_SCREEN
    if (wmsHandler_) {
        auto ret = wmsHandler_->MoveMissionsToBackground(missionIds, result);
        if (ret) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "MoveMissionsToBackground failed, missiondIds may be invalid");
            return ERR_INVALID_VALUE;
        } else {
            return NO_ERROR;
        }
    }
#endif // SUPPORT_SCREEN
    return ERR_NO_INIT;
}

int32_t AbilityManagerService::GetMissionIdByToken(const sptr<IRemoteObject> &token)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "request GetMissionIdByToken.");
    auto abilityRecord = Token::GetAbilityRecordByToken(token);
    if (!abilityRecord) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "abilityRecord is null.");
        return ERR_INVALID_VALUE;
    }
    if (!JudgeSelfCalled(abilityRecord) && !CheckCallerIsDmsProcess()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Permission deny.");
        return ERR_INVALID_VALUE;
    }
    return GetMissionIdByAbilityTokenInner(token);
}

bool AbilityManagerService::IsAbilityControllerStartById(int32_t missionId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto missionListWrap = GetMissionListWrap();
    if (missionListWrap == nullptr) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "missionListWrap null.");
        return true;
    }
    InnerMissionInfo innerMissionInfo;
    int getMission = missionListWrap->GetInnerMissionInfoById(missionId, innerMissionInfo);
    if (getMission != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR,
            "cannot find mission info from MissionInfoList by missionId: %{public}d", missionId);
        return true;
    }
    if (!IsAbilityControllerStart(innerMissionInfo.missionInfo.want, innerMissionInfo.missionInfo.want.GetBundle())) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "IsAbilityControllerStart failed: %{public}s",
            innerMissionInfo.missionInfo.want.GetBundle().c_str());
        return false;
    }
    return true;
}

std::list<std::shared_ptr<ConnectionRecord>> AbilityManagerService::GetConnectRecordListByCallback(
    sptr<IAbilityConnection> callback)
{
    auto connectManager = GetCurrentConnectManager();
    CHECK_POINTER_AND_RETURN(connectManager, std::list<std::shared_ptr<ConnectionRecord>>());
    return connectManager->GetConnectRecordListByCallback(callback);
}

bool AbilityManagerService::GenerateDataAbilityRequestByUri(const std::string& dataAbilityUri,
    AbilityRequest &abilityRequest, sptr<IRemoteObject> callerToken, int32_t userId)
{
    auto bms = GetBundleManager();
    CHECK_POINTER_AND_RETURN(bms, false);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "called. userId %{public}d", userId);
    bool queryResult = IN_PROCESS_CALL(bms->QueryAbilityInfoByUri(dataAbilityUri, userId, abilityRequest.abilityInfo));
    if (!queryResult || abilityRequest.abilityInfo.name.empty() || abilityRequest.abilityInfo.bundleName.empty()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Invalid ability info for data ability acquiring.");
        return false;
    }
    abilityRequest.callerToken = callerToken;
    return true;
}

sptr<IAbilityScheduler> AbilityManagerService::AcquireDataAbility(
    const Uri &uri, bool tryBind, const sptr<IRemoteObject> &callerToken)
{
    auto localUri(uri);
    if (localUri.GetScheme() != AbilityConfig::SCHEME_DATA_ABILITY) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Acquire data ability with invalid uri scheme.");
        return nullptr;
    }
    std::vector<std::string> pathSegments;
    localUri.GetPathSegments(pathSegments);
    if (pathSegments.empty()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Acquire data ability with invalid uri path.");
        return nullptr;
    }

    auto userId = GetValidUserId(INVALID_USER_ID);
    AbilityRequest abilityRequest;
    if (!GenerateDataAbilityRequestByUri(localUri.ToString(), abilityRequest, callerToken, userId)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Generate data ability request by uri failed.");
        return nullptr;
    }

    auto isShellCall = AAFwk::PermissionVerification::GetInstance()->IsShellCall();
    auto isSaCall = AAFwk::PermissionVerification::GetInstance()->IsSACall();
    if (!isSaCall && CheckCallDataAbilityPermission(abilityRequest, isShellCall, isSaCall) != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Invalid ability request info for data ability acquiring.");
        return nullptr;
    }

    TAG_LOGD(AAFwkTag::ABILITYMGR, "Query data ability info: %{public}s|%{public}s|%{public}s",
        abilityRequest.appInfo.name.c_str(), abilityRequest.appInfo.bundleName.c_str(),
        abilityRequest.abilityInfo.name.c_str());

    if (CheckStaticCfgPermission(abilityRequest, false, -1, true, isSaCall) !=
        AppExecFwk::Constants::PERMISSION_GRANTED) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "CheckStaticCfgPermission fail");
        return nullptr;
    }

    if (!VerificationAllToken(callerToken)) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "VerificationAllToken fail");
        return nullptr;
    }

    if (abilityRequest.abilityInfo.applicationInfo.singleton) {
        userId = U0_USER_ID;
    }

    std::shared_ptr<DataAbilityManager> dataAbilityManager = GetDataAbilityManagerByUserId(userId);
    CHECK_POINTER_AND_RETURN(dataAbilityManager, nullptr);
    ReportEventToRSS(abilityRequest.abilityInfo, callerToken);
    bool isNotHap = isSaCall || isShellCall;
    UpdateCallerInfo(abilityRequest.want, callerToken);
    return dataAbilityManager->Acquire(abilityRequest, tryBind, callerToken, isNotHap);
}

int AbilityManagerService::ReleaseDataAbility(
    sptr<IAbilityScheduler> dataAbilityScheduler, const sptr<IRemoteObject> &callerToken)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "called");
    if (!dataAbilityScheduler || !callerToken) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "dataAbilitySchedule or callerToken is nullptr");
        return ERR_INVALID_VALUE;
    }

    std::shared_ptr<DataAbilityManager> dataAbilityManager = GetDataAbilityManager(dataAbilityScheduler);
    if (!dataAbilityManager) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "dataAbilityScheduler is not exists");
        return ERR_INVALID_VALUE;
    }

    auto isSaCall = AAFwk::PermissionVerification::GetInstance()->IsSACall();
    auto isShellCall = AAFwk::PermissionVerification::GetInstance()->IsShellCall();
    bool isNotHap = isSaCall || isShellCall;
    return dataAbilityManager->Release(dataAbilityScheduler, callerToken, isNotHap);
}

int AbilityManagerService::AttachAbilityThread(
    const sptr<IAbilityScheduler> &scheduler, const sptr<IRemoteObject> &token)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "called");
    CHECK_POINTER_AND_RETURN(scheduler, ERR_INVALID_VALUE);
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled() && !VerificationAllToken(token)) {
        return ERR_INVALID_VALUE;
    }
    auto abilityRecord = Token::GetAbilityRecordByToken(token);
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);
    if (!JudgeSelfCalled(abilityRecord)) {
        return CHECK_PERMISSION_FAILED;
    }

    auto userId = abilityRecord->GetApplicationInfo().uid / BASE_USER_RANGE;
    auto abilityInfo = abilityRecord->GetAbilityInfo();
    auto type = abilityInfo.type;
    // force timeout ability for test
    if (IsNeedTimeoutForTest(abilityInfo.name, AbilityRecord::ConvertAbilityState(AbilityState::INITIAL))) {
        TAG_LOGW(AAFwkTag::ABILITYMGR,
            "force timeout ability for test, state:INITIAL, ability: %{public}s", abilityInfo.name.c_str());
        return ERR_OK;
    }
    if (type == AppExecFwk::AbilityType::SERVICE || type == AppExecFwk::AbilityType::EXTENSION) {
        auto connectManager = GetConnectManagerByUserId(userId);
        if (!connectManager) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "connectManager is nullptr. userId=%{public}d", userId);
            return ERR_INVALID_VALUE;
        }
        return connectManager->AttachAbilityThreadLocked(scheduler, token);
    } else if (type == AppExecFwk::AbilityType::DATA) {
        auto dataAbilityManager = GetDataAbilityManagerByUserId(userId);
        if (!dataAbilityManager) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "dataAbilityManager is Null. userId=%{public}d", userId);
            return ERR_INVALID_VALUE;
        }
        return dataAbilityManager->AttachAbilityThread(scheduler, token);
    } else {
        FreezeUtil::LifecycleFlow flow = { token, FreezeUtil::TimeoutState::LOAD };
        auto entry = std::to_string(AbilityUtil::SystemTimeMillis()) + "; AbilityManagerService::AttachAbilityThread;" +
            " the end of load lifecycle.";
        FreezeUtil::GetInstance().AddLifecycleEvent(flow, entry);
        int32_t ownerMissionUserId = abilityRecord->GetOwnerMissionUserId();
        if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
            auto uiAbilityManager = GetUIAbilityManagerByUserId(ownerMissionUserId);
            CHECK_POINTER_AND_RETURN(uiAbilityManager, ERR_INVALID_VALUE);
            return uiAbilityManager->AttachAbilityThread(scheduler, token);
        }
        auto missionListManager = GetMissionListManagerByUserId(ownerMissionUserId);
        CHECK_POINTER_AND_RETURN(missionListManager, ERR_INVALID_VALUE);
        return missionListManager->AttachAbilityThread(scheduler, token);
    }
}

void AbilityManagerService::DumpSysInner(
    const std::string& args, std::vector<std::string>& info, bool isClient, bool isUserID, int userId)
{
    std::vector<std::string> argList;
    SplitStr(args, " ", argList);
    if (argList.empty()) {
        return;
    }

    DumpSysMissionListInner(args, info, isClient, isUserID, userId);
    DumpSysStateInner(args, info, isClient, isUserID, userId);
    DumpSysPendingInner(args, info, isClient, isUserID, userId);
    DumpSysProcess(args, info, isClient, isUserID, userId);
}

void AbilityManagerService::DumpSysMissionListInner(
    const std::string &args, std::vector<std::string> &info, bool isClient, bool isUserID, int userId)
{
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        DumpSysMissionListInnerBySCB(args, info, isClient, isUserID, userId);
        return;
    }
    std::shared_ptr<MissionListManagerInterface> targetManager;
    if (isUserID) {
        auto missionListManager = GetMissionListManagerByUserId(userId);
        if (missionListManager == nullptr) {
            info.push_back("error: No user found.");
            return;
        }
        targetManager = missionListManager;
    } else {
        targetManager = GetCurrentMissionListManager();
    }

    CHECK_POINTER(targetManager);

    std::vector<std::string> argList;
    SplitStr(args, " ", argList);
    if (argList.empty()) {
        return;
    }

    if (argList.size() == MIN_DUMP_ARGUMENT_NUM) {
        targetManager->DumpMissionList(info, isClient, argList[1]);
    } else if (argList.size() < MIN_DUMP_ARGUMENT_NUM) {
        targetManager->DumpMissionList(info, isClient);
    } else {
        info.emplace_back("error: invalid argument, please see 'hidumper -s AbilityManagerService -a '-h''.");
    }
}

void AbilityManagerService::DumpSysMissionListInnerBySCB(
    const std::string &args, std::vector<std::string> &info, bool isClient, bool isUserID, int userId)
{
    if (!isUserID) {
        userId = GetUserId();
    }

    std::vector<std::string> argList;
    SplitStr(args, " ", argList);
    if (argList.empty()) {
        return;
    }

    auto uiAbilityManager = GetUIAbilityManagerByUserId(userId);
    CHECK_POINTER(uiAbilityManager);
    if (argList.size() == MIN_DUMP_ARGUMENT_NUM) {
        uiAbilityManager->DumpMissionList(info, isClient, argList[1]);
    } else if (argList.size() < MIN_DUMP_ARGUMENT_NUM) {
        uiAbilityManager->DumpMissionList(info, isClient);
    } else {
        info.emplace_back("error: invalid argument, please see 'hidumper -s AbilityManagerService -a '-h''.");
    }
}

void AbilityManagerService::DumpSysAbilityInner(
    const std::string &args, std::vector<std::string> &info, bool isClient, bool isUserID, int userId)
{
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        DumpSysAbilityInnerBySCB(args, info, isClient, isUserID, userId);
        return;
    }
    std::shared_ptr<MissionListManagerInterface> targetManager;
    if (isUserID) {
        auto missionListManager = GetMissionListManagerByUserId(userId);
        if (missionListManager == nullptr) {
            info.push_back("error: No user found.");
            return;
        }
        targetManager = missionListManager;
    } else {
        targetManager = GetCurrentMissionListManager();
    }

    CHECK_POINTER(targetManager);

    std::vector<std::string> argList;
    SplitStr(args, " ", argList);
    if (argList.empty()) {
        return;
    }
    if (argList.size() >= MIN_DUMP_ARGUMENT_NUM) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "argList = %{public}s", argList[1].c_str());
        std::vector<std::string> params(argList.begin() + MIN_DUMP_ARGUMENT_NUM, argList.end());
        try {
            auto abilityId = static_cast<int32_t>(std::stoi(argList[1]));
            targetManager->DumpMissionListByRecordId(info, isClient, abilityId, params);
        } catch (...) {
            TAG_LOGW(AAFwkTag::ABILITYMGR, "stoi(%{public}s) failed", argList[1].c_str());
            info.emplace_back("error: invalid argument, please see 'hidumper -s AbilityManagerService -a '-h''.");
        }
    } else {
        info.emplace_back("error: invalid argument, please see 'hidumper -s AbilityManagerService -a '-h''.");
    }
}

void AbilityManagerService::DumpSysAbilityInnerBySCB(
    const std::string &args, std::vector<std::string> &info, bool isClient, bool isUserID, int userId)
{
    if (!isUserID) {
        userId = GetUserId();
    }

    std::vector<std::string> argList;
    SplitStr(args, " ", argList);
    if (argList.empty()) {
        return;
    }
    if (argList.size() >= MIN_DUMP_ARGUMENT_NUM) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "argList = %{public}s", argList[1].c_str());
        std::vector<std::string> params(argList.begin() + MIN_DUMP_ARGUMENT_NUM, argList.end());
        try {
            auto abilityId = static_cast<int32_t>(std::stoi(argList[1]));
            auto uiAbilityManager = GetUIAbilityManagerByUserId(userId);
            CHECK_POINTER(uiAbilityManager);
            uiAbilityManager->DumpMissionListByRecordId(info, isClient, abilityId, params);
        } catch (...) {
            TAG_LOGW(AAFwkTag::ABILITYMGR, "stoi(%{public}s) failed", argList[1].c_str());
            info.emplace_back("error: invalid argument, please see 'hidumper -s AbilityManagerService -a '-h''.");
        }
    } else {
        info.emplace_back("error: invalid argument, please see 'hidumper -s AbilityManagerService -a '-h''.");
    }
}

void AbilityManagerService::DumpSysStateInner(
    const std::string& args, std::vector<std::string>& info, bool isClient, bool isUserID, int userId)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "DumpSysStateInner begin:%{public}s", args.c_str());
    std::shared_ptr<AbilityConnectManager> targetManager;

    if (isUserID) {
        auto connectManager = GetConnectManagerByUserId(userId);
        if (connectManager == nullptr) {
            info.push_back("error: No user found.");
            return;
        }
        targetManager = connectManager;
    } else {
        targetManager = GetCurrentConnectManager();
    }

    CHECK_POINTER(targetManager);

    std::vector<std::string> argList;
    SplitStr(args, " ", argList);
    if (argList.empty()) {
        return;
    }

    if (argList.size() == MIN_DUMP_ARGUMENT_NUM) {
        targetManager->DumpState(info, isClient, argList[1]);
    } else if (argList.size() < MIN_DUMP_ARGUMENT_NUM) {
        targetManager->DumpState(info, isClient);
    } else {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "uri = %{public}s", argList[1].c_str());
        std::vector<std::string> params(argList.begin() + MIN_DUMP_ARGUMENT_NUM, argList.end());
        targetManager->DumpStateByUri(info, isClient, argList[1], params);
    }
}

void AbilityManagerService::DumpSysPendingInner(
    const std::string& args, std::vector<std::string>& info, bool isClient, bool isUserID, int userId)
{
    std::shared_ptr<PendingWantManager> targetManager;
    if (isUserID) {
        auto pendingWantManager = GetPendingWantManagerByUserId(userId);
        if (pendingWantManager == nullptr) {
            info.push_back("error: No user found.");
            return;
        }
        targetManager = pendingWantManager;
    } else {
        targetManager = GetCurrentPendingWantManager();
    }

    CHECK_POINTER(targetManager);

    std::vector<std::string> argList;
    SplitStr(args, " ", argList);
    if (argList.empty()) {
        return;
    }

    if (argList.size() == MIN_DUMP_ARGUMENT_NUM) {
        targetManager->DumpByRecordId(info, argList[1]);
    } else if (argList.size() < MIN_DUMP_ARGUMENT_NUM) {
        targetManager->Dump(info);
    } else {
        info.emplace_back("error: invalid argument, please see 'hidumper -s AbilityManagerService -a '-h''.");
    }
}

void AbilityManagerService::DumpSysProcess(
    const std::string& args, std::vector<std::string>& info, bool isClient, bool isUserID, int userId)
{
    std::vector<std::string> argList;
    SplitStr(args, " ", argList);
    if (argList.empty()) {
        return;
    }
    std::vector<AppExecFwk::RunningProcessInfo> processInfos;
    int ret = 0;
    if (isUserID) {
        ret = GetProcessRunningInfosByUserId(processInfos, userId);
    } else {
        ret = GetProcessRunningInfos(processInfos);
    }

    if (ret != ERR_OK || processInfos.size() == 0) {
        return;
    }

    std::string dumpInfo = "  AppRunningRecords:";
    info.push_back(dumpInfo);
    auto processInfoID = 0;
    auto hasProcessName = (argList.size() == MIN_DUMP_ARGUMENT_NUM ? true : false);
    for (const auto& processInfo : processInfos) {
        if (hasProcessName && argList[1] != processInfo.processName_) {
            continue;
        }

        dumpInfo = "    AppRunningRecord ID #" + std::to_string(processInfoID);
        processInfoID++;
        info.push_back(dumpInfo);
        dumpInfo = "      process name [" + processInfo.processName_ + "]";
        info.push_back(dumpInfo);
        dumpInfo = "      pid #" + std::to_string(processInfo.pid_) +
            "  uid #" + std::to_string(processInfo.uid_);
        info.push_back(dumpInfo);
        auto appState = static_cast<AppState>(processInfo.state_);
        dumpInfo = "      state #" + DelayedSingleton<AppScheduler>::GetInstance()->ConvertAppState(appState);
        info.push_back(dumpInfo);
        DumpUIExtensionRootHostRunningInfos(processInfo.pid_, info);
        DumpUIExtensionProviderRunningInfos(processInfo.pid_, info);
    }
}

void AbilityManagerService::DumpUIExtensionRootHostRunningInfos(pid_t pid, std::vector<std::string> &info)
{
    auto appMgr = GetAppMgr();
    if (appMgr == nullptr) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "Get AppMgr failed.");
        return;
    }

    std::vector<pid_t> hostPids;
    auto ret = IN_PROCESS_CALL(appMgr->GetAllUIExtensionRootHostPid(pid, hostPids));
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Get root host process info faild.");
        return;
    }

    if (hostPids.size() == 0) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "There's no ui extenson root host of pid %{public}d.", pid);
        return;
    }

    std::string temp;
    for (size_t i = 0; i < hostPids.size(); i++) {
        temp = "      root caller #" + std::to_string(i);
        info.push_back(temp);
        temp = "        pid #" + std::to_string(hostPids[i]);
        info.push_back(temp);
    }
}

void AbilityManagerService::DumpUIExtensionProviderRunningInfos(pid_t hostPid, std::vector<std::string> &info)
{
    auto appMgr = GetAppMgr();
    if (appMgr == nullptr) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "Get AppMgr failed.");
        return;
    }

    std::vector<pid_t> providerPids;
    auto ret = IN_PROCESS_CALL(appMgr->GetAllUIExtensionProviderPid(hostPid, providerPids));
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Get provider process info faild.");
        return;
    }

    if (providerPids.size() == 0) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "There's no ui extension provider of pid %{public}d.", hostPid);
        return;
    }

    std::string temp;
    for (size_t i = 0; i < providerPids.size(); i++) {
        temp = "      uiextension provider #" + std::to_string(i);
        info.push_back(temp);
        temp = "        pid #" + std::to_string(providerPids[i]);
        info.push_back(temp);
    }
}

void AbilityManagerService::DataDumpSysStateInner(
    const std::string& args, std::vector<std::string>& info, bool isClient, bool isUserID, int userId)
{
    std::shared_ptr<DataAbilityManager> targetManager;
    if (isUserID) {
        auto dataAbilityManager = GetDataAbilityManagerByUserId(userId);
        if (dataAbilityManager == nullptr) {
            info.push_back("error: No user found.");
            return;
        }
        targetManager = dataAbilityManager;
    } else {
        targetManager = GetCurrentDataAbilityManager();
    }

    CHECK_POINTER(targetManager);

    std::vector<std::string> argList;
    SplitStr(args, " ", argList);
    if (argList.empty()) {
        return;
    }
    if (argList.size() == MIN_DUMP_ARGUMENT_NUM) {
        targetManager->DumpSysState(info, isClient, argList[1]);
    } else if (argList.size() < MIN_DUMP_ARGUMENT_NUM) {
        targetManager->DumpSysState(info, isClient);
    } else {
        info.emplace_back("error: invalid argument, please see 'hidumper -s AbilityManagerService -a '-h''.");
    }
}

void AbilityManagerService::DumpInner(const std::string &args, std::vector<std::string> &info)
{
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        auto uiAbilityManager = GetCurrentUIAbilityManager();
        CHECK_POINTER(uiAbilityManager);
        uiAbilityManager->Dump(info);
        return;
    }

    auto missionListManager = GetCurrentMissionListManager();
    if (missionListManager) {
        missionListManager->Dump(info);
    }
}

void AbilityManagerService::DumpMissionListInner(const std::string &args, std::vector<std::string> &info)
{
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        auto uiAbilityManager = GetCurrentUIAbilityManager();
        CHECK_POINTER(uiAbilityManager);
        uiAbilityManager->DumpMissionList(info, false, " ");
        return;
    }
    auto missionListManager = GetCurrentMissionListManager();
    if (missionListManager) {
        missionListManager->DumpMissionList(info, false, "");
    }
}

void AbilityManagerService::DumpMissionInfosInner(const std::string &args, std::vector<std::string> &info)
{
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "call");
#ifdef SUPPORT_GRAPHICS
        Rosen::WindowManager::GetInstance().DumpSessionAll(info);
#endif // SUPPORT_GRAPHICS
        return;
    }
    auto missionListManager = GetCurrentMissionListManager();
    if (missionListManager) {
        missionListManager->DumpMissionInfos(info);
    }
}

void AbilityManagerService::DumpMissionInner(const std::string &args, std::vector<std::string> &info)
{
    std::vector<std::string> argList;
    SplitStr(args, " ", argList);
    if (argList.empty()) {
        return;
    }
    if (argList.size() < MIN_DUMP_ARGUMENT_NUM) {
        info.push_back("error: invalid argument, please see 'ability dump -h'.");
        return;
    }
    int missionId = DEFAULT_INVAL_VALUE;
    (void)StrToInt(argList[1], missionId);

    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "call");
#ifdef SUPPORT_GRAPHICS
        Rosen::WindowManager::GetInstance().DumpSessionWithId(missionId, info);
#endif // SUPPORT_GRAPHICS
        return;
    }

    auto missionListManager = GetCurrentMissionListManager();
    CHECK_POINTER_LOG(missionListManager, "Current mission manager not init.");
    missionListManager->DumpMission(missionId, info);
}

void AbilityManagerService::DumpStateInner(const std::string &args, std::vector<std::string> &info)
{
    auto connectManager = GetCurrentConnectManager();
    CHECK_POINTER_LOG(connectManager, "Current mission manager not init.");
    std::vector<std::string> argList;
    SplitStr(args, " ", argList);
    if (argList.empty()) {
        return;
    }
    if (argList.size() == MIN_DUMP_ARGUMENT_NUM) {
        connectManager->DumpState(info, false, argList[1]);
    } else if (argList.size() < MIN_DUMP_ARGUMENT_NUM) {
        connectManager->DumpState(info, false);
    } else {
        info.emplace_back("error: invalid argument, please see 'ability dump -h'.");
    }
}

void AbilityManagerService::DataDumpStateInner(const std::string &args, std::vector<std::string> &info)
{
    auto dataAbilityManager = GetCurrentDataAbilityManager();
    CHECK_POINTER(dataAbilityManager);
    std::vector<std::string> argList;
    SplitStr(args, " ", argList);
    if (argList.empty()) {
        return;
    }
    if (argList.size() == MIN_DUMP_ARGUMENT_NUM) {
        dataAbilityManager->DumpState(info, argList[1]);
    } else if (argList.size() < MIN_DUMP_ARGUMENT_NUM) {
        dataAbilityManager->DumpState(info);
    } else {
        info.emplace_back("error: invalid argument, please see 'ability dump -h'.");
    }
}

void AbilityManagerService::DumpState(const std::string &args, std::vector<std::string> &info)
{
    auto isShellCall = AAFwk::PermissionVerification::GetInstance()->IsShellCall();
    auto isHidumperServiceCall = (IPCSkeleton::GetCallingUid() == HIDUMPER_SERVICE_UID);
    if (!isShellCall && !isHidumperServiceCall) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Permission deny.");
        return;
    }
    std::vector<std::string> argList;
    SplitStr(args, " ", argList);
    if (argList.empty()) {
        return;
    }
    auto key = DumpUtils::DumpMap(argList[0]);
    if (!key.first) {
        return;
    }
    switch (key.second) {
        case DumpUtils::KEY_DUMP_SERVICE:
            DumpStateInner(args, info);
            break;
        case DumpUtils::KEY_DUMP_DATA:
            DataDumpStateInner(args, info);
            break;
        case DumpUtils::KEY_DUMP_ALL:
            DumpInner(args, info);
            break;
        case DumpUtils::KEY_DUMP_MISSION:
            DumpMissionInner(args, info);
            break;
        case DumpUtils::KEY_DUMP_MISSION_LIST:
            DumpMissionListInner(args, info);
            break;
        case DumpUtils::KEY_DUMP_MISSION_INFOS:
            DumpMissionInfosInner(args, info);
            break;
        default:
            info.push_back("error: invalid argument, please see 'ability dump -h'.");
            break;
    }
}

void AbilityManagerService::DumpSysState(
    const std::string& args, std::vector<std::string>& info, bool isClient, bool isUserID, int userId)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "%{public}s begin", __func__);
    auto isShellCall = AAFwk::PermissionVerification::GetInstance()->IsShellCall();
    auto isHidumperServiceCall = (IPCSkeleton::GetCallingUid() == HIDUMPER_SERVICE_UID);
    if (!isShellCall && !isHidumperServiceCall) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Permission deny.");
        return;
    }
    std::vector<std::string> argList;
    SplitStr(args, " ", argList);
    if (argList.empty()) {
        return;
    }
    auto key = DumpUtils::DumpsysMap(argList[0]);
    if (!key.first) {
        return;
    }
    switch (key.second) {
        case DumpUtils::KEY_DUMP_SYS_ALL:
            DumpSysInner(args, info, isClient, isUserID, userId);
            break;
        case DumpUtils::KEY_DUMP_SYS_SERVICE:
            DumpSysStateInner(args, info, isClient, isUserID, userId);
            break;
        case DumpUtils::KEY_DUMP_SYS_PENDING:
            DumpSysPendingInner(args, info, isClient, isUserID, userId);
            break;
        case DumpUtils::KEY_DUMP_SYS_PROCESS:
            DumpSysProcess(args, info, isClient, isUserID, userId);
            break;
        case DumpUtils::KEY_DUMP_SYS_DATA:
            DataDumpSysStateInner(args, info, isClient, isUserID, userId);
            break;
        case DumpUtils::KEY_DUMP_SYS_MISSION_LIST:
            DumpSysMissionListInner(args, info, isClient, isUserID, userId);
            break;
        case DumpUtils::KEY_DUMP_SYS_ABILITY:
            DumpSysAbilityInner(args, info, isClient, isUserID, userId);
            break;
        default:
            info.push_back("error: invalid argument, please see 'ability dump -h'.");
            break;
    }
}

int AbilityManagerService::AbilityTransitionDone(const sptr<IRemoteObject> &token, int state, const PacMap &saveData)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Lifecycle: state:%{public}d.", state);
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled() && !VerificationAllToken(token)) {
        return ERR_INVALID_VALUE;
    }
    auto abilityRecord = Token::GetAbilityRecordByToken(token);
    CHECK_POINTER_AND_RETURN_LOG(abilityRecord, ERR_INVALID_VALUE, "Ability record is nullptr.");
    if (!JudgeSelfCalled(abilityRecord)) {
        return CHECK_PERMISSION_FAILED;
    }

    auto abilityInfo = abilityRecord->GetAbilityInfo();
    TAG_LOGI(AAFwkTag::ABILITYMGR, "Lifecycle: ability: %{public}s.", abilityRecord->GetURI().c_str());
    auto type = abilityInfo.type;
    auto userId = abilityRecord->GetApplicationInfo().uid / BASE_USER_RANGE;
    // force timeout ability for test
    int targetState = AbilityRecord::ConvertLifeCycleToAbilityState(static_cast<AbilityLifeCycleState>(state));
    bool isTerminate = abilityRecord->IsAbilityState(AbilityState::TERMINATING) && targetState == AbilityState::INITIAL;
    std::string tempState = isTerminate ? AbilityRecord::ConvertAbilityState(AbilityState::TERMINATING) :
        AbilityRecord::ConvertAbilityState(static_cast<AbilityState>(targetState));
    if (IsNeedTimeoutForTest(abilityInfo.name, tempState)) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "force timeout ability for test, state:%{public}s, ability: %{public}s",
            tempState.c_str(),
            abilityInfo.name.c_str());
        return ERR_OK;
    }
    if (type == AppExecFwk::AbilityType::SERVICE || type == AppExecFwk::AbilityType::EXTENSION) {
        auto connectManager = GetConnectManagerByUserId(userId);
        if (!connectManager) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "connectManager is nullptr. userId=%{public}d", userId);
            return ERR_INVALID_VALUE;
        }
        return connectManager->AbilityTransitionDone(token, state);
    }
    if (type == AppExecFwk::AbilityType::DATA) {
        auto dataAbilityManager = GetDataAbilityManagerByUserId(userId);
        if (!dataAbilityManager) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "dataAbilityManager is Null. userId=%{public}d", userId);
            return ERR_INVALID_VALUE;
        }
        return dataAbilityManager->AbilityTransitionDone(token, state);
    }

    if (targetState == AbilityState::BACKGROUND) {
        FreezeUtil::LifecycleFlow flow = { token, FreezeUtil::TimeoutState::BACKGROUND };
        auto entry = std::to_string(AbilityUtil::SystemTimeMillis()) +
            "; AbilityManagerService::AbilityTransitionDone; the end of background lifecycle.";
        FreezeUtil::GetInstance().AddLifecycleEvent(flow, entry);
    } else if (targetState != AbilityState::INITIAL) {
        FreezeUtil::LifecycleFlow flow = { token, FreezeUtil::TimeoutState::FOREGROUND };
        auto entry = std::to_string(AbilityUtil::SystemTimeMillis()) +
            "; AbilityManagerService::AbilityTransitionDone; the end of foreground lifecycle.";
        entry += " the end of foreground lifecycle.";
        FreezeUtil::GetInstance().AddLifecycleEvent(flow, entry);
    }

    int32_t ownerMissionUserId = abilityRecord->GetOwnerMissionUserId();
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        auto uiAbilityManager = GetUIAbilityManagerByUserId(ownerMissionUserId);
        CHECK_POINTER_AND_RETURN(uiAbilityManager, ERR_INVALID_VALUE);
        return uiAbilityManager->AbilityTransactionDone(token, state, saveData);
    } else {
        auto missionListManager = GetMissionListManagerByUserId(ownerMissionUserId);
        if (!missionListManager) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "missionListManager is Null. userId=%{public}d", ownerMissionUserId);
            return ERR_INVALID_VALUE;
        }
        return missionListManager->AbilityTransactionDone(token, state, saveData);
    }
}

int AbilityManagerService::AbilityWindowConfigTransitionDone(
    const sptr<IRemoteObject> &token, const WindowConfig &windowConfig)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled() && !VerificationAllToken(token)) {
        return ERR_INVALID_VALUE;
    }
    auto abilityRecord = Token::GetAbilityRecordByToken(token);
    CHECK_POINTER_AND_RETURN_LOG(abilityRecord, ERR_INVALID_VALUE, "Ability record is nullptr.");
    if (!JudgeSelfCalled(abilityRecord)) {
        return CHECK_PERMISSION_FAILED;
    }

    TAG_LOGI(AAFwkTag::ABILITYMGR, "Lifecycle: ability: %{public}s.", abilityRecord->GetURI().c_str());
    int32_t ownerMissionUserId = abilityRecord->GetOwnerMissionUserId();
    auto uiAbilityManager = GetUIAbilityManagerByUserId(ownerMissionUserId);
    CHECK_POINTER_AND_RETURN(uiAbilityManager, ERR_INVALID_VALUE);
    return uiAbilityManager->AbilityWindowConfigTransactionDone(token, windowConfig);
}

int AbilityManagerService::ScheduleConnectAbilityDone(
    const sptr<IRemoteObject> &token, const sptr<IRemoteObject> &remoteObject)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "called");
    if (!VerificationAllToken(token)) {
        return ERR_INVALID_VALUE;
    }

    auto abilityRecord = Token::GetAbilityRecordByToken(token);
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);
    if (!JudgeSelfCalled(abilityRecord)) {
        return CHECK_PERMISSION_FAILED;
    }

    auto type = abilityRecord->GetAbilityInfo().type;
    if (type != AppExecFwk::AbilityType::SERVICE && type != AppExecFwk::AbilityType::EXTENSION) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Connect ability failed, target ability is not service.");
        return TARGET_ABILITY_NOT_SERVICE;
    }
    auto userId = abilityRecord->GetApplicationInfo().uid / BASE_USER_RANGE;
    auto connectManager = GetConnectManagerByUserId(userId);
    if (!connectManager) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "connectManager is nullptr. userId=%{public}d", userId);
        return ERR_INVALID_VALUE;
    }
    return connectManager->ScheduleConnectAbilityDoneLocked(token, remoteObject);
}

int AbilityManagerService::ScheduleDisconnectAbilityDone(const sptr<IRemoteObject> &token)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "Schedule disconnect ability done.");
    if (!VerificationAllToken(token)) {
        return ERR_INVALID_VALUE;
    }

    auto abilityRecord = Token::GetAbilityRecordByToken(token);
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);
    if (!JudgeSelfCalled(abilityRecord)) {
        return CHECK_PERMISSION_FAILED;
    }

    auto type = abilityRecord->GetAbilityInfo().type;
    if (type != AppExecFwk::AbilityType::SERVICE && type != AppExecFwk::AbilityType::EXTENSION) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Connect ability failed, target ability is not service.");
        return TARGET_ABILITY_NOT_SERVICE;
    }
    auto userId = abilityRecord->GetApplicationInfo().uid / BASE_USER_RANGE;
    auto connectManager = GetConnectManagerByUserId(userId);
    if (!connectManager) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "connectManager is nullptr. userId=%{public}d", userId);
        return ERR_INVALID_VALUE;
    }
    return connectManager->ScheduleDisconnectAbilityDoneLocked(token);
}

int AbilityManagerService::ScheduleCommandAbilityDone(const sptr<IRemoteObject> &token)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Schedule command ability done.");
    if (!VerificationAllToken(token)) {
        return ERR_INVALID_VALUE;
    }

    auto abilityRecord = Token::GetAbilityRecordByToken(token);
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);
    if (!JudgeSelfCalled(abilityRecord)) {
        return CHECK_PERMISSION_FAILED;
    }
    // force timeout ability for test
    if (IsNeedTimeoutForTest(abilityRecord->GetAbilityInfo().name, std::string("COMMAND"))) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "force timeout ability for test, state:COMMAND, ability: %{public}s",
            abilityRecord->GetAbilityInfo().name.c_str());
        return ERR_OK;
    }
    auto type = abilityRecord->GetAbilityInfo().type;
    if (type != AppExecFwk::AbilityType::SERVICE && type != AppExecFwk::AbilityType::EXTENSION) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Connect ability failed, target ability is not service.");
        return TARGET_ABILITY_NOT_SERVICE;
    }
    auto userId = abilityRecord->GetApplicationInfo().uid / BASE_USER_RANGE;
    auto connectManager = GetConnectManagerByUserId(userId);
    if (!connectManager) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "connectManager is nullptr. userId=%{public}d", userId);
        return ERR_INVALID_VALUE;
    }
    return connectManager->ScheduleCommandAbilityDoneLocked(token);
}

int AbilityManagerService::ScheduleCommandAbilityWindowDone(
    const sptr<IRemoteObject> &token,
    const sptr<SessionInfo> &sessionInfo,
    WindowCommand winCmd,
    AbilityCommand abilityCmd)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "enter.");
    if (!VerificationAllToken(token)) {
        return ERR_INVALID_VALUE;
    }

    auto abilityRecord = Token::GetAbilityRecordByToken(token);
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);
    if (!JudgeSelfCalled(abilityRecord)) {
        return CHECK_PERMISSION_FAILED;
    }

    if (!UIExtensionUtils::IsUIExtension(abilityRecord->GetAbilityInfo().extensionAbilityType)
        && !UIExtensionUtils::IsWindowExtension(abilityRecord->GetAbilityInfo().extensionAbilityType)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "target ability is not ui or window extension.");
        return ERR_INVALID_VALUE;
    }
    auto userId = abilityRecord->GetApplicationInfo().uid / BASE_USER_RANGE;
    auto connectManager = GetConnectManagerByUserId(userId);
    if (!connectManager) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "connectManager is nullptr. userId=%{public}d", userId);
        return ERR_INVALID_VALUE;
    }
    return connectManager->ScheduleCommandAbilityWindowDone(token, sessionInfo, winCmd, abilityCmd);
}

void AbilityManagerService::OnAbilityRequestDone(const sptr<IRemoteObject> &token, const int32_t state)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto abilityRecord = Token::GetAbilityRecordByToken(token);
    CHECK_POINTER(abilityRecord);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "name is %{public}s", abilityRecord->GetAbilityInfo().name.c_str());
    auto userId = abilityRecord->GetApplicationInfo().uid / BASE_USER_RANGE;

    auto type = abilityRecord->GetAbilityInfo().type;
    switch (type) {
        case AppExecFwk::AbilityType::DATA: {
            auto dataAbilityManager = GetDataAbilityManagerByUserId(userId);
            if (!dataAbilityManager) {
                TAG_LOGE(AAFwkTag::ABILITYMGR, "dataAbilityManager is Null. userId=%{public}d", userId);
                return;
            }
            dataAbilityManager->OnAbilityRequestDone(token, state);
            break;
        }
        case AppExecFwk::AbilityType::SERVICE:
        case AppExecFwk::AbilityType::EXTENSION: {
            auto connectManager = GetConnectManagerByUserId(userId);
            if (!connectManager) {
                TAG_LOGE(AAFwkTag::ABILITYMGR, "connectManager is nullptr. userId=%{public}d", userId);
                return;
            }
            connectManager->OnAbilityRequestDone(token, state);
            break;
        }
        default: {
            int32_t ownerUserId = abilityRecord->GetOwnerMissionUserId();
            if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
                auto uiAbilityManager = GetUIAbilityManagerByUserId(ownerUserId);
                CHECK_POINTER(uiAbilityManager);
                uiAbilityManager->OnAbilityRequestDone(token, state);
            } else {
                auto missionListManager = GetMissionListManagerByUserId(ownerUserId);
                if (!missionListManager) {
                    TAG_LOGE(AAFwkTag::ABILITYMGR, "missionListManager is Null. userId=%{public}d", ownerUserId);
                    return;
                }
                missionListManager->OnAbilityRequestDone(token, state);
            }
            break;
        }
    }
}

void AbilityManagerService::OnAppStateChanged(const AppInfo &info)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "called");
    auto connectManager = GetCurrentConnectManager();
    CHECK_POINTER_LOG(connectManager, "Connect manager not init.");
    connectManager->OnAppStateChanged(info);
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        auto uiAbilityManager = GetCurrentUIAbilityManager();
        CHECK_POINTER(uiAbilityManager);
        uiAbilityManager->OnAppStateChanged(info);
    } else {
        auto missionListManager = GetCurrentMissionListManager();
        CHECK_POINTER_LOG(missionListManager, "Current mission list manager not init.");
        missionListManager->OnAppStateChanged(info);
    }
    auto dataAbilityManager = GetCurrentDataAbilityManager();
    CHECK_POINTER(dataAbilityManager);
    dataAbilityManager->OnAppStateChanged(info);

    auto residentProcessMgr = DelayedSingleton<ResidentProcessManager>::GetInstance();
    CHECK_POINTER(residentProcessMgr);
    residentProcessMgr->OnAppStateChanged(info);
}

std::shared_ptr<AbilityEventHandler> AbilityManagerService::GetEventHandler()
{
    return eventHandler_;
}

// multi user scene
int32_t AbilityManagerService::GetUserId() const
{
    if (userController_) {
        auto userId = userController_->GetCurrentUserId();
        TAG_LOGD(AAFwkTag::ABILITYMGR, "userId is %{public}d", userId);
        return userId;
    }
    return U0_USER_ID;
}

void AbilityManagerService::StartHighestPriorityAbility(int32_t userId, bool isBoot)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "%{public}s", __func__);
    auto bms = GetBundleManager();
    CHECK_POINTER(bms);

    /* Query the highest priority ability or extension ability, and start it. usually, it is OOBE or launcher */
    Want want;
    want.AddEntity(HIGHEST_PRIORITY_ABILITY_ENTITY);
    AppExecFwk::AbilityInfo abilityInfo;
    AppExecFwk::ExtensionAbilityInfo extensionAbilityInfo;
    int attemptNums = 0;
    while (!IN_PROCESS_CALL(bms->ImplicitQueryInfoByPriority(want,
        AppExecFwk::AbilityInfoFlag::GET_ABILITY_INFO_DEFAULT, userId,
        abilityInfo, extensionAbilityInfo))) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "Waiting query highest priority ability info completed.");
        ++attemptNums;
        if (!isBoot && attemptNums > SWITCH_ACCOUNT_TRY) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "Query highest priority ability failed.");
            return;
        }
        AbilityRequest abilityRequest;
        usleep(REPOLL_TIME_MICRO_SECONDS);
    }

    if (abilityInfo.name.empty() && extensionAbilityInfo.name.empty()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Query highest priority ability failed");
        return;
    }

    Want abilityWant; // donot use 'want' here, because the entity of 'want' is not empty
    if (!abilityInfo.name.empty()) {
        /* highest priority ability */
        TAG_LOGI(AAFwkTag::ABILITYMGR, "Start the highest priority ability. bundleName: %{public}s, ability:%{public}s",
            abilityInfo.bundleName.c_str(), abilityInfo.name.c_str());
        abilityWant.SetElementName(abilityInfo.bundleName, abilityInfo.name);
    } else {
        /* highest priority extension ability */
        TAG_LOGI(AAFwkTag::ABILITYMGR,
            "Start the highest priority extension ability. bundleName: %{public}s, ability:%{public}s",
            extensionAbilityInfo.bundleName.c_str(), extensionAbilityInfo.name.c_str());
        abilityWant.SetElementName(extensionAbilityInfo.bundleName, extensionAbilityInfo.name);
    }

#ifdef SUPPORT_GRAPHICS
    abilityWant.SetParam(NEED_STARTINGWINDOW, false);
    // wait BOOT_ANIMATION_STARTED to start LAUNCHER
    WaitBootAnimationStart();
#endif

    /* note: OOBE APP need disable itself, otherwise, it will be started when restart system everytime */
    (void)StartAbility(abilityWant, userId, DEFAULT_INVAL_VALUE);
}

int AbilityManagerService::GenerateAbilityRequest(const Want &want, int requestCode, AbilityRequest &request,
    const sptr<IRemoteObject> &callerToken, int32_t userId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto abilityRecord = Token::GetAbilityRecordByToken(callerToken);
    if (abilityRecord && abilityRecord->GetAppIndex() > AbilityRuntime::GlobalConstant::MAX_APP_CLONE_INDEX &&
        abilityRecord->GetApplicationInfo().bundleName == want.GetElement().GetBundleName()) {
        (const_cast<Want &>(want)).SetParam(AbilityRuntime::ServerConstant::DLP_INDEX, abilityRecord->GetAppIndex());
    }

    if (abilityRecord != nullptr) {
        (const_cast<Want &>(want)).SetParam(DEBUG_APP, abilityRecord->IsDebugApp());
    }

    request.want = want;
    request.requestCode = requestCode;
    request.callerToken = callerToken;
    auto setting = AbilityStartSetting::GetEmptySetting();
    if (setting != nullptr) {
        auto bundleName = want.GetElement().GetBundleName();
        auto defaultRecovery = DefaultRecoveryConfig::GetInstance().IsBundleDefaultRecoveryEnabled(bundleName);
        TAG_LOGD(AAFwkTag::ABILITYMGR, "bundleName: %{public}s, defaultRecovery: %{public}d.", bundleName.c_str(),
            defaultRecovery);
        setting->AddProperty(AbilityStartSetting::DEFAULT_RECOVERY_KEY, defaultRecovery ? "true" : "false");
        setting->AddProperty(AbilityStartSetting::IS_START_BY_SCB_KEY, "false"); // default is false
        request.startSetting = std::make_shared<AbilityStartSetting>(*(setting.get()));
    }

    auto abilityInfo = StartAbilityUtils::startAbilityInfo;
    if (abilityInfo == nullptr || abilityInfo->GetAppBundleName() != want.GetElement().GetBundleName()) {
        int32_t appIndex = 0;
        if (!AbilityRuntime::StartupUtil::GetAppIndex(want, appIndex)) {
            return ERR_APP_CLONE_INDEX_INVALID;
        }
        Want localWant = want;
        if (!StartAbilityUtils::IsCallFromAncoShellOrBroker(callerToken)) {
            localWant.RemoveParam(PARAM_RESV_ANCO_CALLER_UID);
            localWant.RemoveParam(PARAM_RESV_ANCO_CALLER_BUNDLENAME);
        }
        abilityInfo = StartAbilityInfo::CreateStartAbilityInfo(localWant, userId, appIndex);
    }
    CHECK_POINTER_AND_RETURN(abilityInfo, GET_ABILITY_SERVICE_FAILED);
    if (abilityInfo->status != ERR_OK) {
        return abilityInfo->status;
    }
    request.abilityInfo = abilityInfo->abilityInfo;
    request.extensionProcessMode = abilityInfo->extensionProcessMode;

    if (request.abilityInfo.applicationInfo.codePath == std::to_string(CollaboratorType::RESERVE_TYPE)) {
        request.collaboratorType = CollaboratorType::RESERVE_TYPE;
    } else if (request.abilityInfo.applicationInfo.codePath == std::to_string(CollaboratorType::OTHERS_TYPE)) {
        request.collaboratorType = CollaboratorType::OTHERS_TYPE;
    }

    if (request.abilityInfo.type == AppExecFwk::AbilityType::SERVICE && request.abilityInfo.isStageBasedModel) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "Stage mode, abilityInfo SERVICE type reset EXTENSION.");
        request.abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    }

    if (request.abilityInfo.applicationInfo.name.empty() || request.abilityInfo.applicationInfo.bundleName.empty()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Get app info failed.");
        return RESOLVE_APP_ERR;
    }
    if (want.GetIntParam(AAFwk::SCREEN_MODE_KEY, ScreenMode::IDLE_SCREEN_MODE) == ScreenMode::JUMP_SCREEN_MODE &&
        (request.abilityInfo.applicationInfo.bundleType != AppExecFwk::BundleType::ATOMIC_SERVICE ||
        request.abilityInfo.launchMode != AppExecFwk::LaunchMode::SINGLETON)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "The interface of starting atomicService can start only atomicService.");
        return TARGET_ABILITY_NOT_SERVICE;
    }
    request.appInfo = request.abilityInfo.applicationInfo;
    request.uid = request.appInfo.uid;
    TAG_LOGD(AAFwkTag::ABILITYMGR,
        "GenerateAbilityRequest end, app name: %{public}s, moduleName name: %{public}s, uid: %{public}d.",
        request.appInfo.name.c_str(), request.abilityInfo.moduleName.c_str(), request.uid);

    request.want.SetModuleName(request.abilityInfo.moduleName);

    if (want.GetBoolParam(Want::PARAM_RESV_START_RECENT, false) &&
        AAFwk::PermissionVerification::GetInstance()->VerifyStartRecentAbilityPermission()) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "Set start recent.");
        request.startRecent = true;
    }

    return ERR_OK;
}

int AbilityManagerService::GenerateExtensionAbilityRequest(
    const Want &want, AbilityRequest &request, const sptr<IRemoteObject> &callerToken, int32_t userId)
{
    auto abilityRecord = Token::GetAbilityRecordByToken(callerToken);
    if (abilityRecord && abilityRecord->GetAppIndex() > AbilityRuntime::GlobalConstant::MAX_APP_CLONE_INDEX &&
        abilityRecord->GetApplicationInfo().bundleName == want.GetElement().GetBundleName()) {
        (const_cast<Want &>(want)).SetParam(AbilityRuntime::ServerConstant::DLP_INDEX, abilityRecord->GetAppIndex());
    }
    request.want = want;
    request.callerToken = callerToken;
    request.startSetting = nullptr;

    auto abilityInfo = StartAbilityUtils::startAbilityInfo;
    if (abilityInfo == nullptr || abilityInfo->GetAppBundleName() != want.GetElement().GetBundleName()) {
        int32_t appIndex = 0;
        if (!AbilityRuntime::StartupUtil::GetAppIndex(want, appIndex)) {
            return ERR_APP_CLONE_INDEX_INVALID;
        }
        abilityInfo = StartAbilityInfo::CreateStartExtensionInfo(want, userId, appIndex);
    }
    CHECK_POINTER_AND_RETURN(abilityInfo, GET_ABILITY_SERVICE_FAILED);
    if (abilityInfo->status != ERR_OK) {
        return abilityInfo->status;
    }

    auto result = InitialAbilityRequest(request, *abilityInfo);
    return result;
}

int32_t AbilityManagerService::InitialAbilityRequest(AbilityRequest &request,
    const StartAbilityInfo &abilityInfo) const
{
    request.abilityInfo = abilityInfo.abilityInfo;
    request.extensionProcessMode = abilityInfo.extensionProcessMode;
    if (request.abilityInfo.applicationInfo.name.empty() || request.abilityInfo.applicationInfo.bundleName.empty()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Get app info failed.");
        return RESOLVE_APP_ERR;
    }
    request.appInfo = request.abilityInfo.applicationInfo;
    request.uid = request.appInfo.uid;
    TAG_LOGD(AAFwkTag::ABILITYMGR,
        "GenerateExtensionAbilityRequest end, app name: %{public}s, bundle name: %{public}s, uid: %{public}d.",
        request.appInfo.name.c_str(), request.appInfo.bundleName.c_str(), request.uid);

    TAG_LOGD(AAFwkTag::ABILITYMGR,
        "GenerateExtensionAbilityRequest, moduleName: %{public}s.", request.abilityInfo.moduleName.c_str());
    request.want.SetModuleName(request.abilityInfo.moduleName);

    return ERR_OK;
}

int AbilityManagerService::StopServiceAbility(const Want &want, int32_t userId, const sptr<IRemoteObject> &token)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call.");

    auto isSaCall = AAFwk::PermissionVerification::GetInstance()->IsSACall();
    auto isShellCall = AAFwk::PermissionVerification::GetInstance()->IsShellCall();
    if (!isSaCall && !isShellCall) {
        auto abilityRecord = Token::GetAbilityRecordByToken(token);
        if (abilityRecord == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "callerRecord is nullptr");
            return ERR_INVALID_VALUE;
        }
    }

    int32_t validUserId = GetValidUserId(userId);
    if (!JudgeMultiUserConcurrency(validUserId)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Multi-user non-concurrent mode is not satisfied.");
        return ERR_CROSS_USER;
    }

    AbilityUtil::RemoveShowModeKey(const_cast<Want &>(want));
    AbilityRequest abilityRequest;
    auto result = GenerateAbilityRequest(want, DEFAULT_INVAL_VALUE, abilityRequest, nullptr, validUserId);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Generate ability request local error.");
        return result;
    }

    auto abilityInfo = abilityRequest.abilityInfo;
    validUserId = abilityInfo.applicationInfo.singleton ? U0_USER_ID : validUserId;
    TAG_LOGD(AAFwkTag::ABILITYMGR, "validUserId : %{public}d, singleton is : %{public}d",
        validUserId, static_cast<int>(abilityInfo.applicationInfo.singleton));

    auto type = abilityInfo.type;
    if (type != AppExecFwk::AbilityType::SERVICE && type != AppExecFwk::AbilityType::EXTENSION) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Target ability is not service type.");
        return TARGET_ABILITY_NOT_SERVICE;
    }

    auto res = JudgeAbilityVisibleControl(abilityInfo);
    if (res != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Target ability is invisible");
        return res;
    }

    auto connectManager = GetConnectManagerByUserId(validUserId);
    if (connectManager == nullptr) {
        return ERR_INVALID_VALUE;
    }

    return connectManager->StopServiceAbility(abilityRequest);
}

void AbilityManagerService::OnAbilityDied(std::shared_ptr<AbilityRecord> abilityRecord)
{
    CHECK_POINTER(abilityRecord);
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        if (abilityRecord->GetAbilityInfo().type == AbilityType::PAGE) {
            auto uiAbilityManager = GetUIAbilityManagerByUserId(abilityRecord->GetOwnerMissionUserId());
            CHECK_POINTER(uiAbilityManager);
            uiAbilityManager->OnAbilityDied(abilityRecord);
            return;
        }
    } else {
        auto manager = GetMissionListManagerByUserId(abilityRecord->GetOwnerMissionUserId());
        if (manager && abilityRecord->GetAbilityInfo().type == AbilityType::PAGE) {
            ReleaseAbilityTokenMap(abilityRecord->GetToken());
            manager->OnAbilityDied(abilityRecord, GetUserId());
            return;
        }
    }

    auto connectManager = GetConnectManagerByToken(abilityRecord->GetToken());
    if (connectManager) {
        connectManager->OnAbilityDied(abilityRecord, GetUserId());
        return;
    }

    auto dataAbilityManager = GetDataAbilityManagerByToken(abilityRecord->GetToken());
    if (dataAbilityManager) {
        dataAbilityManager->OnAbilityDied(abilityRecord);
    }
}

void AbilityManagerService::OnCallConnectDied(std::shared_ptr<CallRecord> callRecord)
{
    CHECK_POINTER(callRecord);
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        auto uiAbilityManager = GetCurrentUIAbilityManager();
        CHECK_POINTER(uiAbilityManager);
        uiAbilityManager->OnCallConnectDied(callRecord);
        return;
    }
    auto missionListManager = GetCurrentMissionListManager();
    if (missionListManager) {
        missionListManager->OnCallConnectDied(callRecord);
    }
}

void AbilityManagerService::ReleaseAbilityTokenMap(const sptr<IRemoteObject> &token)
{
    std::lock_guard<ffrt::mutex> autoLock(abilityTokenLock_);
    for (auto iter = callStubTokenMap_.begin(); iter != callStubTokenMap_.end(); iter++) {
        if (iter->second == token) {
            callStubTokenMap_.erase(iter);
            break;
        }
    }
}

int AbilityManagerService::KillProcess(const std::string &bundleName, const bool clearPageStack)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Kill process, bundleName: %{public}s, clearPageStack: %{public}d",
        bundleName.c_str(), clearPageStack);
    CHECK_CALLER_IS_SYSTEM_APP;
    auto bms = GetBundleManager();
    CHECK_POINTER_AND_RETURN(bms, KILL_PROCESS_FAILED);
    int32_t userId = GetUserId();
    AppExecFwk::BundleInfo bundleInfo;
    if (!IN_PROCESS_CALL(
        bms->GetBundleInfo(bundleName, AppExecFwk::BundleFlag::GET_BUNDLE_DEFAULT, bundleInfo, userId))) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Failed to get bundle info when kill process.");
        return GET_BUNDLE_INFO_FAILED;
    }

    bool keepAliveEnable = bundleInfo.isKeepAlive;
    AmsResidentProcessRdb::GetInstance().GetResidentProcessEnable(bundleName, keepAliveEnable);
    if (keepAliveEnable && DelayedSingleton<AppScheduler>::GetInstance()->IsMemorySizeSufficent()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Can not kill keep alive process.");
        return KILL_PROCESS_KEEP_ALIVE;
    }

    int ret = DelayedSingleton<AppScheduler>::GetInstance()->KillApplication(bundleName, clearPageStack);
    if (ret != ERR_OK) {
        return KILL_PROCESS_FAILED;
    }

    auto connectMgr = GetConnectManagerByUserId(userId);
    CHECK_POINTER_AND_RETURN(connectMgr, ERR_NO_INIT);
    connectMgr->DeleteInvalidServiceRecord(bundleName);
    return ERR_OK;
}

int AbilityManagerService::UninstallApp(const std::string &bundleName, int32_t uid)
{
    return UninstallApp(bundleName, uid, 0);
}

int32_t AbilityManagerService::UninstallApp(const std::string &bundleName, int32_t uid, int32_t appIndex)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "Uninstall app, bundleName: %{public}s, uid=%{public}d, appIndex:%{public}d",
        bundleName.c_str(), uid, appIndex);
    return UninstallAppInner(bundleName, uid, appIndex, false, "");
}

int32_t AbilityManagerService::UpgradeApp(const std::string &bundleName, const int32_t uid, const std::string &exitMsg,
    int32_t appIndex)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR,
        "UpgradeApp app, bundleName: %{public}s, uid=%{public}d, exitMsg: %{public}s, appIndex:%{public}d",
        bundleName.c_str(), uid, exitMsg.c_str(), appIndex);
    return UninstallAppInner(bundleName, uid, appIndex, true, exitMsg);
}

int32_t AbilityManagerService::UninstallAppInner(const std::string &bundleName, const int32_t uid, int32_t appIndex,
    const bool isUpgrade, const std::string &exitMsg)
{
    pid_t callingPid = IPCSkeleton::GetCallingPid();
    pid_t pid = getprocpid();
    if (callingPid != pid) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "%{public}s: Not bundleMgr call.", __func__);
        return CHECK_PERMISSION_FAILED;
    }

    if (isUpgrade) {
        CHECK_POINTER_AND_RETURN(appExitReasonHelper_, ERR_NULL_OBJECT);
        AAFwk::ExitReason exitReason = { REASON_UPGRADE, exitMsg };
        appExitReasonHelper_->RecordAppExitReason(bundleName, uid, appIndex, exitReason);
    }

    CHECK_POINTER_AND_RETURN(subManagersHelper_, ERR_NULL_OBJECT);
    subManagersHelper_->UninstallApp(bundleName, uid);
    int ret = DelayedSingleton<AppScheduler>::GetInstance()->KillApplicationByUid(bundleName, uid);
    if (ret != ERR_OK) {
        return UNINSTALL_APP_FAILED;
    }
    if (!isUpgrade) {
        DelayedSingleton<AbilityRuntime::AppExitReasonDataManager>::GetInstance()->DeleteAppExitReason(bundleName, uid,
            appIndex);
    }
    return ERR_OK;
}

std::shared_ptr<AppExecFwk::BundleMgrHelper> AbilityManagerService::GetBundleManager()
{
    if (bundleMgrHelper_ == nullptr) {
        bundleMgrHelper_ = AbilityUtil::GetBundleManagerHelper();
    }
    return bundleMgrHelper_;
}

int AbilityManagerService::PreLoadAppDataAbilities(const std::string &bundleName, const int32_t userId)
{
    if (bundleName.empty()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Invalid bundle name when app data abilities preloading.");
        return ERR_INVALID_VALUE;
    }

    if (taskHandler_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "taskHandler nullptr.");
        return ERR_INVALID_STATE;
    }

    taskHandler_->SubmitTask([weak = weak_from_this(), bundleName, userId]() {
        auto pthis = weak.lock();
        if (pthis == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "pthis nullptr.");
            return;
        }
        pthis->PreLoadAppDataAbilitiesTask(bundleName, userId);
        });

    return ERR_OK;
}

void AbilityManagerService::PreLoadAppDataAbilitiesTask(const std::string &bundleName, const int32_t userId)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "called");
    auto dataAbilityManager = GetDataAbilityManagerByUserId(userId);
    if (dataAbilityManager == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Invalid data ability manager when app data abilities preloading.");
        return;
    }

    auto bms = GetBundleManager();
    CHECK_POINTER(bms);

    AppExecFwk::BundleInfo bundleInfo;
    bool ret = IN_PROCESS_CALL(
        bms->GetBundleInfo(bundleName, AppExecFwk::BundleFlag::GET_BUNDLE_WITH_ABILITIES, bundleInfo, userId));
    if (!ret) {
        TAG_LOGE(AAFwkTag::ABILITYMGR,
            "Failed to get bundle info when app data abilities preloading, userId is %{public}d", userId);
        return;
    }

    auto begin = system_clock::now();
    AbilityRequest dataAbilityRequest;
    UpdateCallerInfo(dataAbilityRequest.want, nullptr);
    dataAbilityRequest.appInfo = bundleInfo.applicationInfo;
    for (auto it = bundleInfo.abilityInfos.begin(); it != bundleInfo.abilityInfos.end(); ++it) {
        if (it->type != AppExecFwk::AbilityType::DATA) {
            continue;
        }
        if ((system_clock::now() - begin) >= DATA_ABILITY_START_TIMEOUT) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "App data ability preloading for '%{public}s' timeout.", bundleName.c_str());
            return;
        }
        dataAbilityRequest.abilityInfo = *it;
        dataAbilityRequest.uid = bundleInfo.uid;
        TAG_LOGD(AAFwkTag::ABILITYMGR, "App data ability preloading: '%{public}s.%{public}s'...",
            it->bundleName.c_str(), it->name.c_str());

        auto dataAbility = dataAbilityManager->Acquire(dataAbilityRequest, false, nullptr, false);
        if (dataAbility == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITYMGR,
                "Failed to preload data ability '%{public}s.%{public}s'.", it->bundleName.c_str(), it->name.c_str());
            return;
        }
    }
}

bool AbilityManagerService::IsSystemUiApp(const AppExecFwk::AbilityInfo &info) const
{
    if (info.bundleName != AbilityConfig::SYSTEM_UI_BUNDLE_NAME) {
        return false;
    }
    return (info.name == AbilityConfig::SYSTEM_UI_NAVIGATION_BAR ||
        info.name == AbilityConfig::SYSTEM_UI_STATUS_BAR ||
        info.name == AbilityConfig::SYSTEM_UI_ABILITY_NAME);
}

bool AbilityManagerService::IsSystemUI(const std::string &bundleName) const
{
    return bundleName == AbilityConfig::SYSTEM_UI_BUNDLE_NAME;
}

void AbilityManagerService::HandleLoadTimeOut(int64_t abilityRecordId, bool isHalf)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Handle load timeout.");
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        auto uiAbilityManagers = GetUIAbilityManagers();
        for (auto& item : uiAbilityManagers) {
            if (item.second) {
                item.second->OnTimeOut(AbilityManagerService::LOAD_TIMEOUT_MSG, abilityRecordId, isHalf);
            }
        }
        return;
    }
    auto missionListManagers = GetMissionListManagers();
    for (auto& item : missionListManagers) {
        if (item.second) {
            item.second->OnTimeOut(AbilityManagerService::LOAD_TIMEOUT_MSG, abilityRecordId, isHalf);
}
    }
}

void AbilityManagerService::HandleActiveTimeOut(int64_t abilityRecordId)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Handle active timeout.");
    auto missionListManagers = GetMissionListManagers();
    for (auto& item : missionListManagers) {
        if (item.second) {
            item.second->OnTimeOut(AbilityManagerService::ACTIVE_TIMEOUT_MSG, abilityRecordId);
        }
    }
}

void AbilityManagerService::HandleInactiveTimeOut(int64_t abilityRecordId)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Handle inactive timeout.");
    auto missionListManagers = GetMissionListManagers();
    for (auto& item : missionListManagers) {
        if (item.second) {
            item.second->OnTimeOut(AbilityManagerService::INACTIVE_TIMEOUT_MSG, abilityRecordId);
        }
    }
    auto connectManagers = GetConnectManagers();
    for (auto& item : connectManagers) {
        if (item.second) {
            item.second->OnTimeOut(AbilityManagerService::INACTIVE_TIMEOUT_MSG, abilityRecordId);
        }
    }
}

void AbilityManagerService::HandleForegroundTimeOut(int64_t abilityRecordId, bool isHalf)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Handle foreground timeout.");
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        auto uiAbilityManagers = GetUIAbilityManagers();
        for (auto& item : uiAbilityManagers) {
            if (item.second) {
                item.second->OnTimeOut(AbilityManagerService::FOREGROUND_TIMEOUT_MSG, abilityRecordId, isHalf);
            }
        }
        return;
    }
    auto missionListManagers = GetMissionListManagers();
    for (auto& item : missionListManagers) {
        if (item.second) {
            item.second->OnTimeOut(AbilityManagerService::FOREGROUND_TIMEOUT_MSG, abilityRecordId, isHalf);
}
    }
}

void AbilityManagerService::HandleShareDataTimeOut(int64_t uniqueId)
{
    WantParams wantParam;
    int32_t ret = GetShareDataPairAndReturnData(nullptr, ERR_TIMED_OUT, uniqueId, wantParam);
    if (ret) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "acqurieShareData failed.");
    }
}

int32_t AbilityManagerService::GetShareDataPairAndReturnData(std::shared_ptr<AbilityRecord> abilityRecord,
    const int32_t &resultCode, const int32_t &uniqueId, WantParams &wantParam)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "resultCode:%{public}d, uniqueId:%{public}d, wantParam size:%{public}d.",
        resultCode, uniqueId, wantParam.Size());
    auto it = iAcquireShareDataMap_.find(uniqueId);
    if (it != iAcquireShareDataMap_.end()) {
        auto shareDataPair = it->second;
        if (abilityRecord && shareDataPair.first != abilityRecord->GetAbilityRecordId()) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "abilityRecord is not the abilityRecord from request.");
            return ERR_INVALID_VALUE;
        }
        auto callback = shareDataPair.second;
        if (!callback) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "callback object is nullptr.");
            return ERR_INVALID_VALUE;
        }
        auto ret = callback->AcquireShareDataDone(resultCode, wantParam);
        iAcquireShareDataMap_.erase(it);
        return ret;
    }
    TAG_LOGE(AAFwkTag::ABILITYMGR, "iAcquireShareData is null.");
    return ERR_INVALID_VALUE;
}

bool AbilityManagerService::VerificationToken(const sptr<IRemoteObject> &token)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "Verification token.");
    auto dataAbilityManager = GetCurrentDataAbilityManager();
    CHECK_POINTER_RETURN_BOOL(dataAbilityManager);
    auto connectManager = GetCurrentConnectManager();
    CHECK_POINTER_RETURN_BOOL(connectManager);
    auto missionListManager = GetCurrentMissionListManager();
    CHECK_POINTER_RETURN_BOOL(missionListManager);

    if (missionListManager->GetAbilityRecordByToken(token)) {
        return true;
    }
    if (missionListManager->GetAbilityFromTerminateList(token)) {
        return true;
    }

    if (dataAbilityManager->GetAbilityRecordByToken(token)) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "Verification token4.");
        return true;
    }

    if (connectManager->GetExtensionByTokenFromServiceMap(token)) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "Verification token5.");
        return true;
    }

    if (connectManager->GetExtensionByTokenFromAbilityCache(token)) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "Verification token5.");
        return true;
    }

    if (connectManager->GetExtensionByTokenFromTerminatingMap(token)) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "Verification token5.");
        return true;
    }

    TAG_LOGE(AAFwkTag::ABILITYMGR, "Failed to verify token.");
    return false;
}

bool AbilityManagerService::VerificationAllToken(const sptr<IRemoteObject> &token)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    CHECK_POINTER_AND_RETURN(subManagersHelper_, false);
    return subManagersHelper_->VerificationAllToken(token);
}

std::shared_ptr<DataAbilityManager> AbilityManagerService::GetCurrentDataAbilityManager()
{
    CHECK_POINTER_AND_RETURN(subManagersHelper_, nullptr);
    return subManagersHelper_->GetCurrentDataAbilityManager();
}

std::shared_ptr<DataAbilityManager> AbilityManagerService::GetDataAbilityManager(
    const sptr<IAbilityScheduler> &scheduler)
{
    CHECK_POINTER_AND_RETURN(subManagersHelper_, nullptr);
    return subManagersHelper_->GetDataAbilityManager(scheduler);
}

std::shared_ptr<DataAbilityManager> AbilityManagerService::GetDataAbilityManagerByUserId(int32_t userId)
{
    CHECK_POINTER_AND_RETURN(subManagersHelper_, nullptr);
    return subManagersHelper_->GetDataAbilityManagerByUserId(userId);
}

std::shared_ptr<DataAbilityManager> AbilityManagerService::GetDataAbilityManagerByToken(
    const sptr<IRemoteObject> &token)
{
    CHECK_POINTER_AND_RETURN(subManagersHelper_, nullptr);
    return subManagersHelper_->GetDataAbilityManagerByToken(token);
}

std::unordered_map<int, std::shared_ptr<AbilityConnectManager>> AbilityManagerService::GetConnectManagers()
{
    if (subManagersHelper_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "pointer is nullptr.");
        return std::unordered_map<int, std::shared_ptr<AbilityConnectManager>>();
    }
    return subManagersHelper_->GetConnectManagers();
}

std::shared_ptr<AbilityConnectManager> AbilityManagerService::GetCurrentConnectManager()
{
    CHECK_POINTER_AND_RETURN(subManagersHelper_, nullptr);
    return subManagersHelper_->GetCurrentConnectManager();
}

std::shared_ptr<AbilityConnectManager> AbilityManagerService::GetConnectManagerByUserId(int32_t userId)
{
    CHECK_POINTER_AND_RETURN(subManagersHelper_, nullptr);
    return subManagersHelper_->GetConnectManagerByUserId(userId);
}

std::shared_ptr<AbilityConnectManager> AbilityManagerService::GetConnectManagerByToken(
    const sptr<IRemoteObject> &token)
{
    CHECK_POINTER_AND_RETURN(subManagersHelper_, nullptr);
    return subManagersHelper_->GetConnectManagerByToken(token);
}

std::shared_ptr<PendingWantManager> AbilityManagerService::GetCurrentPendingWantManager()
{
    CHECK_POINTER_AND_RETURN(subManagersHelper_, nullptr);
    return subManagersHelper_->GetCurrentPendingWantManager();
}

std::shared_ptr<PendingWantManager> AbilityManagerService::GetPendingWantManagerByUserId(int32_t userId)
{
    CHECK_POINTER_AND_RETURN(subManagersHelper_, nullptr);
    return subManagersHelper_->GetPendingWantManagerByUserId(userId);
}

std::unordered_map<int, std::shared_ptr<MissionListManagerInterface>> AbilityManagerService::GetMissionListManagers()
{
    if (subManagersHelper_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "pointer is nullptr.");
        return std::unordered_map<int, std::shared_ptr<MissionListManagerInterface>>();
    }
    return subManagersHelper_->GetMissionListManagers();
}

std::shared_ptr<MissionListManagerInterface> AbilityManagerService::GetCurrentMissionListManager()
{
    CHECK_POINTER_AND_RETURN(subManagersHelper_, nullptr);
    return subManagersHelper_->GetCurrentMissionListManager();
}

std::shared_ptr<MissionListManagerInterface> AbilityManagerService::GetMissionListManagerByUserId(int32_t userId)
{
    CHECK_POINTER_AND_RETURN(subManagersHelper_, nullptr);
    return subManagersHelper_->GetMissionListManagerByUserId(userId);
}

std::shared_ptr<MissionListWrap> AbilityManagerService::GetMissionListWrap()
{
    CHECK_POINTER_AND_RETURN(subManagersHelper_, nullptr);
    return subManagersHelper_->GetMissionListWrap();
}

std::unordered_map<int, std::shared_ptr<UIAbilityLifecycleManager>> AbilityManagerService::GetUIAbilityManagers()
{
    if (subManagersHelper_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "pointer is nullptr.");
        return std::unordered_map<int, std::shared_ptr<UIAbilityLifecycleManager>>();
    }
    return subManagersHelper_->GetUIAbilityManagers();
}

std::shared_ptr<UIAbilityLifecycleManager> AbilityManagerService::GetCurrentUIAbilityManager()
{
    CHECK_POINTER_AND_RETURN(subManagersHelper_, nullptr);
    return subManagersHelper_->GetCurrentUIAbilityManager();
}

std::shared_ptr<UIAbilityLifecycleManager> AbilityManagerService::GetUIAbilityManagerByUserId(int32_t userId)
{
    CHECK_POINTER_AND_RETURN(subManagersHelper_, nullptr);
    return subManagersHelper_->GetUIAbilityManagerByUserId(userId);
}

std::shared_ptr<UIAbilityLifecycleManager> AbilityManagerService::GetUIAbilityManagerByUid(int32_t uid)
{
    CHECK_POINTER_AND_RETURN(subManagersHelper_, nullptr);
    return subManagersHelper_->GetUIAbilityManagerByUid(uid);
}

void AbilityManagerService::StartResidentApps()
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "%{public}s", __func__);
    ConnectBmsService();
    auto bms = GetBundleManager();
    CHECK_POINTER_IS_NULLPTR(bms);
    std::vector<AppExecFwk::BundleInfo> bundleInfos;
    if (!IN_PROCESS_CALL(
        bms->GetBundleInfos(OHOS::AppExecFwk::GET_BUNDLE_DEFAULT, bundleInfos, U0_USER_ID))) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Get resident bundleinfos failed");
        return;
    }
    DelayedSingleton<ResidentProcessManager>::GetInstance()->Init();
    TAG_LOGI(AAFwkTag::ABILITYMGR, "StartResidentApps GetBundleInfos size: %{public}zu", bundleInfos.size());

    DelayedSingleton<ResidentProcessManager>::GetInstance()->StartResidentProcessWithMainElement(bundleInfos);
    if (!bundleInfos.empty()) {
#ifdef SUPPORT_GRAPHICS
        WaitBootAnimationStart();
#endif
        DelayedSingleton<ResidentProcessManager>::GetInstance()->StartResidentProcess(bundleInfos);
    }
}

void AbilityManagerService::StartAutoStartupApps()
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "called");
    if (abilityAutoStartupService_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "abilityAutoStartupService_ is nullptr.");
        return;
    }
    std::vector<AutoStartupInfo> infoList;
    int32_t result = abilityAutoStartupService_->QueryAllAutoStartupApplicationsWithoutPermission(infoList,
        GetUserId());
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Failed to query data.");
        return;
    }

    constexpr int retryCount = 5;
    RetryStartAutoStartupApps(infoList, retryCount);
}

void AbilityManagerService::RetryStartAutoStartupApps(
    const std::vector<AutoStartupInfo> &infoList, int32_t retryCount)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR,
        "Called, retryCount: %{public}d, infoList.size:%{public}zu", retryCount, infoList.size());
    std::vector<AutoStartupInfo> failedList;
    for (auto info : infoList) {
        AppExecFwk::ElementName element;
        element.SetBundleName(info.bundleName);
        element.SetAbilityName(info.abilityName);
        element.SetModuleName(info.moduleName);
        Want want;
        want.SetElement(element);
        want.SetParam(Want::PARAM_APP_AUTO_STARTUP_LAUNCH_REASON, true);
        if (info.appCloneIndex > 0 && info.appCloneIndex <= AbilityRuntime::GlobalConstant::MAX_APP_CLONE_INDEX) {
            want.SetParam(Want::PARAM_APP_CLONE_INDEX_KEY, info.appCloneIndex);
        }
        if (StartAbility(want) != ERR_OK) {
            failedList.push_back(info);
        }
    }

    TAG_LOGD(AAFwkTag::ABILITYMGR,
        "RetryCount: %{public}d, failedList.size:%{public}zu", retryCount, failedList.size());
    if (!failedList.empty() && retryCount > 0) {
        auto retryStartAutoStartupAppsTask = [aams = weak_from_this(), list = failedList, retryCount]() {
            auto obj = aams.lock();
            if (obj == nullptr) {
                TAG_LOGE(AAFwkTag::ABILITYMGR, "Retry start auto startup app error, obj is nullptr");
                return;
            }
            obj->RetryStartAutoStartupApps(list, retryCount - 1);
        };
        constexpr int delaytime = 2000;
        taskHandler_->SubmitTask(retryStartAutoStartupAppsTask, "RetryStartAutoStartupApps", delaytime);
    }
}

void AbilityManagerService::SubscribeScreenUnlockedEvent()
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "called");
    // add listen screen unlocked.
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_UNLOCKED);
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_USER_UNLOCKED);
    EventFwk::CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    subscribeInfo.SetThreadMode(EventFwk::CommonEventSubscribeInfo::COMMON);
    auto callback = [abilityManager = weak_from_this()]() {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "On screen unlocked.");
        auto abilityMgr = abilityManager.lock();
        if (abilityMgr == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "Invalid abilityMgr pointer.");
            return;
        }
        auto taskHandler = abilityMgr->GetTaskHandler();
        if (taskHandler == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "Invalid taskHandler pointer.");
            return;
        }
        auto startAutoStartupAppsTask = [abilityManager]() {
            auto abilityMgr = abilityManager.lock();
            if (abilityMgr == nullptr) {
                TAG_LOGE(AAFwkTag::ABILITYMGR, "Invalid abilityMgr pointer.");
                return;
            }
            abilityMgr->RemoveScreenUnlockInterceptor();
            abilityMgr->StartAutoStartupApps();
            abilityMgr->UnSubscribeScreenUnlockedEvent();
        };
        taskHandler->SubmitTask(startAutoStartupAppsTask, "StartAutoStartupApps");
    };
    auto userScreenUnlockCallback = [abilityManager = weak_from_this()]() {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "On user screen unlocked.");
        auto abilityMgr = abilityManager.lock();
        if (abilityMgr == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "Invalid abilityMgr pointer.");
            return;
        }
        abilityMgr->RemoveScreenUnlockInterceptor();
    };
    screenSubscriber_ = std::make_shared<AbilityRuntime::AbilityManagerEventSubscriber>(subscribeInfo, callback,
        userScreenUnlockCallback);
    bool subResult = EventFwk::CommonEventManager::SubscribeCommonEvent(screenSubscriber_);
    if (!subResult) {
        constexpr int retryCount = 20;
        RetrySubscribeScreenUnlockedEvent(retryCount);
    }
}

void AbilityManagerService::UnSubscribeScreenUnlockedEvent()
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "called");
    bool subResult = EventFwk::CommonEventManager::UnSubscribeCommonEvent(screenSubscriber_);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Screen unlocked event subscriber unsubscribe result is %{public}d.", subResult);
}

void AbilityManagerService::RetrySubscribeScreenUnlockedEvent(int32_t retryCount)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "RetryCount: %{public}d.", retryCount);
    auto retrySubscribeScreenUnlockedEventTask = [aams = weak_from_this(), screenSubscriber = screenSubscriber_,
                                                     retryCount]() {
        bool subResult = EventFwk::CommonEventManager::SubscribeCommonEvent(screenSubscriber);
        auto obj = aams.lock();
        if (obj == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "Retry subscribe screen unlocked event, obj is nullptr.");
            return;
        }
        if (!subResult && retryCount > 0) {
            obj->RetrySubscribeScreenUnlockedEvent(retryCount - 1);
        }
    };
    constexpr int delaytime = 200;
    taskHandler_->SubmitTask(retrySubscribeScreenUnlockedEventTask, "RetrySubscribeScreenUnlockedEvent", delaytime);
}

void AbilityManagerService::RemoveScreenUnlockInterceptor()
{
    interceptorExecuter_->RemoveInterceptor("ScreenUnlock");
}

void AbilityManagerService::ConnectBmsService()
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "%{public}s", __func__);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "Waiting AppMgr Service run completed.");
    while (!DelayedSingleton<AppScheduler>::GetInstance()->Init(shared_from_this())) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "failed to init AppScheduler");
        usleep(REPOLL_TIME_MICRO_SECONDS);
    }

    TAG_LOGI(AAFwkTag::ABILITYMGR, "Waiting BundleMgr Service run completed.");
    /* wait until connected to bundle manager service */
    std::lock_guard<ffrt::mutex> guard(globalLock_);
    while (iBundleManager_ == nullptr) {
        sptr<IRemoteObject> bundle_obj =
            OHOS::DelayedSingleton<SaMgrClient>::GetInstance()->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
        if (bundle_obj == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "failed to get bundle manager service");
            usleep(REPOLL_TIME_MICRO_SECONDS);
            continue;
        }
        iBundleManager_ = iface_cast<AppExecFwk::IBundleMgr>(bundle_obj);
    }

    TAG_LOGI(AAFwkTag::ABILITYMGR, "Connect bms success!");
}

int AbilityManagerService::GetWantSenderInfo(const sptr<IWantSender> &target, std::shared_ptr<WantSenderInfo> &info)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "Get pending request info.");
    auto pendingWantManager = GetCurrentPendingWantManager();
    CHECK_POINTER_AND_RETURN(pendingWantManager, ERR_INVALID_VALUE);
    CHECK_POINTER_AND_RETURN(target, ERR_INVALID_VALUE);
    CHECK_POINTER_AND_RETURN(info, ERR_INVALID_VALUE);
    return pendingWantManager->GetWantSenderInfo(target, info);
}

int AbilityManagerService::GetAppMemorySize()
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "service GetAppMemorySize start");
    const char *key = "const.product.arkheaplimit";
    const char *def = "512m";
    char *valueGet = nullptr;
    unsigned int len = 128;
    int ret = GetParameter(key, def, valueGet, len);
    int resultInt = 0;
    if ((ret != GET_PARAMETER_OTHER) && (ret != GET_PARAMETER_INCORRECT)) {
        if (valueGet == nullptr) {
            TAG_LOGW(AAFwkTag::ABILITYMGR, "%{public}s, valueGet is nullptr", __func__);
            return APP_MEMORY_SIZE;
        }
        int len = strlen(valueGet);
        for (int i = 0; i < len; i++) {
            if (valueGet[i] >= '0' && valueGet[i] <= '9') {
                resultInt *= SIZE_10;
                resultInt += valueGet[i] - '0';
            }
        }
        if (resultInt == 0) {
            return APP_MEMORY_SIZE;
        }
        return resultInt;
    }
    return APP_MEMORY_SIZE;
}

bool AbilityManagerService::IsRamConstrainedDevice()
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "service IsRamConstrainedDevice start");
    const char *key = "const.product.islowram";
    const char *def = "0";
    char *valueGet = nullptr;
    unsigned int len = 128;
    int ret = GetParameter(key, def, valueGet, len);
    if ((ret != GET_PARAMETER_OTHER) && (ret != GET_PARAMETER_INCORRECT)) {
        int value = atoi(valueGet);
        if (value) {
            return true;
        }
        return false;
    }
    return false;
}

int32_t AbilityManagerService::GetMissionIdByAbilityToken(const sptr<IRemoteObject> &token)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto abilityRecord = Token::GetAbilityRecordByToken(token);
    if (!abilityRecord) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "abilityRecord is Null.");
        return -1;
    }
    if (!JudgeSelfCalled(abilityRecord)) {
        return -1;
    }
    return GetMissionIdByAbilityTokenInner(token);
}

int32_t AbilityManagerService::GetMissionIdByAbilityTokenInner(const sptr<IRemoteObject> &token)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto abilityRecord = Token::GetAbilityRecordByToken(token);
    if (!abilityRecord) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "abilityRecord is Null.");
        return -1;
    }
    auto userId = abilityRecord->GetOwnerMissionUserId();
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        auto uiAbilityManager = GetUIAbilityManagerByUserId(userId);
        CHECK_POINTER_AND_RETURN(uiAbilityManager, ERR_INVALID_VALUE);
        return uiAbilityManager->GetSessionIdByAbilityToken(token);
    }
    auto missionListManager = GetMissionListManagerByUserId(userId);
    if (!missionListManager) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "missionListManager is Null. owner mission userId=%{public}d", userId);
        return -1;
    }
    return missionListManager->GetMissionIdByAbilityToken(token);
}

sptr<IRemoteObject> AbilityManagerService::GetAbilityTokenByMissionId(int32_t missionId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto missionListManager = GetCurrentMissionListManager();
    if (!missionListManager) {
        return nullptr;
    }
    return missionListManager->GetAbilityTokenByMissionId(missionId);
}

int AbilityManagerService::StartRemoteAbilityByCall(const Want &want, const sptr<IRemoteObject> &callerToken,
    const sptr<IRemoteObject> &connect)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "%{public}s begin StartRemoteAbilityByCall", __func__);
    Want remoteWant = want;
    if (AddStartControlParam(remoteWant, callerToken) != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "%{public}s AddStartControlParam failed.", __func__);
        return ERR_INVALID_VALUE;
    }
    int32_t missionId = -1;
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        missionId = GetMissionIdByAbilityTokenInner(callerToken);
        if (!missionId) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "Invalid missionId id.");
            return ERR_INVALID_VALUE;
        }
    } else {
        missionId = GetMissionIdByAbilityToken(callerToken);
}
    if (missionId < 0) {
        return ERR_INVALID_VALUE;
    }
    remoteWant.SetParam(DMS_MISSION_ID, missionId);
    DistributedClient dmsClient;
    return dmsClient.StartRemoteAbilityByCall(remoteWant, connect);
}

int AbilityManagerService::ReleaseRemoteAbility(const sptr<IRemoteObject> &connect,
    const AppExecFwk::ElementName &element)
{
    DistributedClient dmsClient;
    return dmsClient.ReleaseRemoteAbility(connect, element);
}

int AbilityManagerService::StartAbilityByCall(const Want &want, const sptr<IAbilityConnection> &connect,
    const sptr<IRemoteObject> &callerToken, int32_t accountId)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "call ability.");
    CHECK_POINTER_AND_RETURN(connect, ERR_INVALID_VALUE);
    CHECK_POINTER_AND_RETURN(connect->AsObject(), ERR_INVALID_VALUE);
    if (IsCrossUserCall(accountId)) {
        CHECK_CALLER_IS_SYSTEM_APP;
    }

    if (VerifyAccountPermission(accountId) == CHECK_PERMISSION_FAILED) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "%{public}s: Permission verification failed.", __func__);
        return CHECK_PERMISSION_FAILED;
    }

    auto abilityRecord = Token::GetAbilityRecordByToken(callerToken);
    if (abilityRecord && !JudgeSelfCalled(abilityRecord)) {
        return CHECK_PERMISSION_FAILED;
    }

    AbilityUtil::RemoveWantKey(const_cast<Want &>(want));
    int32_t appIndex = 0;
    if (!StartAbilityUtils::GetAppIndex(want, callerToken, appIndex)) {
        return ERR_APP_CLONE_INDEX_INVALID;
    }
    StartAbilityInfoWrap threadLocalInfo(want, GetUserId(), appIndex, callerToken);
    AbilityInterceptorParam interceptorParam = AbilityInterceptorParam(want, 0, GetUserId(), true, nullptr);
    auto result = interceptorExecuter_ == nullptr ? ERR_INVALID_VALUE :
        interceptorExecuter_->DoProcess(interceptorParam);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "interceptorExecuter_ is nullptr or DoProcess return error.");
        return result;
    }

    if (CheckIfOperateRemote(want)) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "start remote ability by call");
        return StartRemoteAbilityByCall(want, callerToken, connect->AsObject());
    }

    int32_t oriValidUserId = GetValidUserId(accountId);
    if (!JudgeMultiUserConcurrency(oriValidUserId)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Multi-user non-concurrent mode is not satisfied.");
        return ERR_CROSS_USER;
    }

    AbilityRequest abilityRequest;
    abilityRequest.callType = AbilityCallType::CALL_REQUEST_TYPE;
    abilityRequest.callerUid = IPCSkeleton::GetCallingUid();
    abilityRequest.callerToken = callerToken;
    abilityRequest.startSetting = nullptr;
    abilityRequest.want = want;
    abilityRequest.connect = connect;
    result = GenerateAbilityRequest(want, -1, abilityRequest, callerToken, GetUserId());
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Generate ability request error.");
        return result;
    }

    if (!abilityRequest.abilityInfo.isStageBasedModel) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "target ability is not stage base model.");
        return RESOLVE_CALL_ABILITY_VERSION_ERR;
    }

    result = CheckStartByCallPermission(abilityRequest);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "CheckStartByCallPermission fail, result: %{public}d", result);
        return result;
    }

    TAG_LOGD(AAFwkTag::ABILITYMGR, "abilityInfo.applicationInfo.singleton is %{public}s",
        abilityRequest.abilityInfo.applicationInfo.singleton ? "true" : "false");
    UpdateCallerInfo(abilityRequest.want, callerToken);
    AbilityInterceptorParam afterCheckParam = AbilityInterceptorParam(abilityRequest.want, 0, GetUserId(),
        false, callerToken, std::make_shared<AppExecFwk::AbilityInfo>(abilityRequest.abilityInfo));
    result = afterCheckExecuter_ == nullptr ? ERR_INVALID_VALUE :
        afterCheckExecuter_->DoProcess(afterCheckParam);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "afterCheckExecuter_ is nullptr or DoProcess return error.");
        return result;
    }
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        ReportEventToRSS(abilityRequest.abilityInfo, callerToken);
        abilityRequest.want.SetParam(IS_CALL_BY_SCB, false);
        auto uiAbilityManager = GetUIAbilityManagerByUserId(oriValidUserId);
        CHECK_POINTER_AND_RETURN(uiAbilityManager, ERR_INVALID_VALUE);
        return uiAbilityManager->ResolveLocked(abilityRequest);
    }

    auto missionListMgr = GetMissionListManagerByUserId(oriValidUserId);
    if (missionListMgr == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "missionListMgr is Null. Designated User Id=%{public}d", oriValidUserId);
        return ERR_INVALID_VALUE;
    }
    ReportEventToRSS(abilityRequest.abilityInfo, callerToken);

    return missionListMgr->ResolveLocked(abilityRequest);
}

int AbilityManagerService::StartAbilityJust(AbilityRequest &abilityRequest, int32_t validUserId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call");
    UpdateCallerInfo(abilityRequest.want, abilityRequest.callerToken);
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        ReportEventToRSS(abilityRequest.abilityInfo, abilityRequest.callerToken);
        auto uiAbilityManager = GetUIAbilityManagerByUserId(validUserId);
        CHECK_POINTER_AND_RETURN(uiAbilityManager, ERR_INVALID_VALUE);
        return uiAbilityManager->ResolveLocked(abilityRequest);
    }

    auto missionListMgr = GetMissionListManagerByUserId(validUserId);
    if (missionListMgr == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "missionListMgr is Null. Designated User Id=%{public}d", validUserId);
        return ERR_INVALID_VALUE;
    }
    ReportEventToRSS(abilityRequest.abilityInfo, abilityRequest.callerToken);

    return missionListMgr->ResolveLocked(abilityRequest);
}

int AbilityManagerService::ReleaseCall(
    const sptr<IAbilityConnection> &connect, const AppExecFwk::ElementName &element)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Release called ability.");

    CHECK_POINTER_AND_RETURN(connect, ERR_INVALID_VALUE);
    CHECK_POINTER_AND_RETURN(connect->AsObject(), ERR_INVALID_VALUE);

    std::string elementName = element.GetURI();
    TAG_LOGD(AAFwkTag::ABILITYMGR, "try to release called ability, name: %{public}s.", elementName.c_str());

    if (CheckIsRemote(element.GetDeviceID())) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "release remote ability");
        return ReleaseRemoteAbility(connect->AsObject(), element);
    }

    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        auto uiAbilityManager = GetCurrentUIAbilityManager();
        CHECK_POINTER_AND_RETURN(uiAbilityManager, ERR_INVALID_VALUE);
        return uiAbilityManager->ReleaseCallLocked(connect, element);
    }
    auto missionListManager = GetCurrentMissionListManager();
    CHECK_POINTER_AND_RETURN(missionListManager, ERR_NO_INIT);
    return missionListManager->ReleaseCallLocked(connect, element);
}

int AbilityManagerService::JudgeAbilityVisibleControl(const AppExecFwk::AbilityInfo &abilityInfo)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Call.");
    if (abilityInfo.visible) {
        return ERR_OK;
    }
    auto callerTokenId = IPCSkeleton::GetCallingTokenID();
    if (callerTokenId == abilityInfo.applicationInfo.accessTokenId ||
        callerTokenId == static_cast<uint32_t>(IPCSkeleton::GetSelfTokenID())) {  // foundation call is allowed
        return ERR_OK;
    }
    if (AccessTokenKit::VerifyAccessToken(callerTokenId,
        PermissionConstants::PERMISSION_START_INVISIBLE_ABILITY, false) == AppExecFwk::Constants::PERMISSION_GRANTED) {
        return ERR_OK;
    }
    TAG_LOGE(AAFwkTag::ABILITYMGR, "callerToken: %{private}u, targetToken: %{private}u, caller doesn's have permission",
        callerTokenId, abilityInfo.applicationInfo.accessTokenId);
    return ABILITY_VISIBLE_FALSE_DENY_REQUEST;
}

int AbilityManagerService::StartUser(int userId, sptr<IUserCallback> callback)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "StartUser in service:%{public}d.", userId);
    if (IPCSkeleton::GetCallingUid() != ACCOUNT_MGR_SERVICE_UID) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "StartUser permission verification failed, not account process.");
        if (callback != nullptr) {
            callback->OnStartUserDone(userId, CHECK_PERMISSION_FAILED);
        }
        return CHECK_PERMISSION_FAILED;
    }

    if (userController_) {
        userController_->StartUser(userId, callback, true);
    }
    return 0;
}

int AbilityManagerService::StopUser(int userId, const sptr<IUserCallback> &callback)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "StopUser in service:%{public}d", userId);
    if (IPCSkeleton::GetCallingUid() != ACCOUNT_MGR_SERVICE_UID) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "StopUser permission verification failed, not account process");
        if (callback != nullptr) {
            callback->OnStopUserDone(userId, CHECK_PERMISSION_FAILED);
        }
        return CHECK_PERMISSION_FAILED;
    }

    auto ret = -1;
    if (userController_) {
        ret = userController_->StopUser(userId);
        TAG_LOGD(AAFwkTag::ABILITYMGR, "ret = %{public}d", ret);
    }
    if (callback) {
        callback->OnStopUserDone(userId, ret);
    }
    return 0;
}

int AbilityManagerService::LogoutUser(int32_t userId)
{
    if (IPCSkeleton::GetCallingUid() != ACCOUNT_MGR_SERVICE_UID) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Permission verification failed, not account process");
        return CHECK_PERMISSION_FAILED;
    }

    if (userController_) {
        auto ret = userController_->LogoutUser(userId);
        TAG_LOGD(AAFwkTag::ABILITYMGR, "logout user return = %{public}d", ret);
        return ret;
    }
    return ERR_OK;
}

void AbilityManagerService::OnAcceptWantResponse(
    const AAFwk::Want &want, const std::string &flag, int32_t requestId)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "On accept want response");
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        auto uiAbilityManager = GetCurrentUIAbilityManager();
        CHECK_POINTER(uiAbilityManager);
        uiAbilityManager->OnAcceptWantResponse(want, flag, requestId);
        return;
    }
    auto missionListManager = GetCurrentMissionListManager();
    if (!missionListManager) {
        return;
    }
    missionListManager->OnAcceptWantResponse(want, flag);
}

void AbilityManagerService::OnStartSpecifiedAbilityTimeoutResponse(const AAFwk::Want &want, int32_t requestId)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "%{public}s.", want.GetElement().GetURI().c_str());
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        auto uiAbilityManager = GetCurrentUIAbilityManager();
        CHECK_POINTER(uiAbilityManager);
        uiAbilityManager->OnStartSpecifiedAbilityTimeoutResponse(want, requestId);
        return;
    }
    auto missionListManager = GetCurrentMissionListManager();
    if (!missionListManager) {
        return;
    }
    missionListManager->OnStartSpecifiedAbilityTimeoutResponse(want);
}

void AbilityManagerService::OnStartSpecifiedProcessResponse(const AAFwk::Want &want, const std::string &flag,
    int32_t requestId)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "flag = %{public}s", flag.c_str());
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        auto uiAbilityManager = GetCurrentUIAbilityManager();
        CHECK_POINTER(uiAbilityManager);
        uiAbilityManager->OnStartSpecifiedProcessResponse(want, flag, requestId);
        return;
    }
}

void AbilityManagerService::OnStartSpecifiedProcessTimeoutResponse(const AAFwk::Want &want, int32_t requestId)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "%{public}s.", want.GetElement().GetURI().c_str());
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        auto uiAbilityManager = GetCurrentUIAbilityManager();
        CHECK_POINTER(uiAbilityManager);
        uiAbilityManager->OnStartSpecifiedAbilityTimeoutResponse(want, requestId);
        return;
    }
}

int AbilityManagerService::GetAbilityRunningInfos(std::vector<AbilityRunningInfo> &info)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Get running ability infos.");
    CHECK_CALLER_IS_SYSTEM_APP;
    auto isPerm = AAFwk::PermissionVerification::GetInstance()->VerifyRunningInfoPerm();
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        auto uiAbilityManager = GetUIAbilityManagerByUid(IPCSkeleton::GetCallingUid());
        CHECK_POINTER_AND_RETURN(uiAbilityManager, ERR_INVALID_VALUE);
        uiAbilityManager->GetAbilityRunningInfos(info, isPerm);
    } else {
        auto missionListManager = GetCurrentMissionListManager();
        CHECK_POINTER_AND_RETURN(missionListManager, ERR_INVALID_VALUE);
        missionListManager->GetAbilityRunningInfos(info, isPerm);
    }

    UpdateFocusState(info);

    return ERR_OK;
}

void AbilityManagerService::UpdateFocusState(std::vector<AbilityRunningInfo> &info)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (info.empty()) {
        return;
    }

#ifdef SUPPORT_GRAPHICS
    sptr<IRemoteObject> token;
    int ret = IN_PROCESS_CALL(GetTopAbility(token));
    if (ret != ERR_OK || token == nullptr) {
        return;
    }

    auto abilityRecord = Token::GetAbilityRecordByToken(token);
    if (abilityRecord == nullptr) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "%{public}s abilityRecord is null.", __func__);
        return;
    }

    for (auto &item : info) {
        if (item.uid == abilityRecord->GetUid() && item.pid == abilityRecord->GetPid() &&
            item.ability == abilityRecord->GetElementName()) {
            item.abilityState = static_cast<int>(AbilityState::ACTIVE);
            break;
        }
    }
#endif
}

int AbilityManagerService::GetExtensionRunningInfos(int upperLimit, std::vector<ExtensionRunningInfo> &info)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Get extension infos, upperLimit : %{public}d", upperLimit);
    CHECK_CALLER_IS_SYSTEM_APP;
    auto isPerm = AAFwk::PermissionVerification::GetInstance()->VerifyRunningInfoPerm();
    auto connectManager = GetCurrentConnectManager();
    CHECK_POINTER_AND_RETURN(connectManager, ERR_INVALID_VALUE);
    connectManager->GetExtensionRunningInfos(upperLimit, info, GetUserId(), isPerm);
    return ERR_OK;
}

int AbilityManagerService::GetProcessRunningInfos(std::vector<AppExecFwk::RunningProcessInfo> &info)
{
    return DelayedSingleton<AppScheduler>::GetInstance()->GetProcessRunningInfos(info);
}

int AbilityManagerService::GetProcessRunningInfosByUserId(
    std::vector<AppExecFwk::RunningProcessInfo> &info, int32_t userId)
{
    return DelayedSingleton<AppScheduler>::GetInstance()->GetProcessRunningInfosByUserId(info, userId);
}

void AbilityManagerService::ClearUserData(int32_t userId)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "%{public}s", __func__);
    CHECK_POINTER(subManagersHelper_);
    subManagersHelper_->ClearSubManagers(userId);
}

int AbilityManagerService::RegisterSnapshotHandler(const sptr<ISnapshotHandler>& handler)
{
    auto isSaCall = AAFwk::PermissionVerification::GetInstance()->IsSACall();
    if (!isSaCall) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "%{public}s: Permission verification failed", __func__);
        return 0;
    }

    auto missionListManager = GetCurrentMissionListManager();
    CHECK_POINTER_AND_RETURN(missionListManager, INNER_ERR);
    missionListManager->RegisterSnapshotHandler(handler);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "snapshot: AbilityManagerService register snapshot handler success.");
    return ERR_OK;
}

int32_t AbilityManagerService::GetMissionSnapshot(const std::string& deviceId, int32_t missionId,
    MissionSnapshot& missionSnapshot, bool isLowResolution)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    CHECK_CALLER_IS_SYSTEM_APP;
    if (!PermissionVerification::GetInstance()->VerifyMissionPermission()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "%{public}s: Permission verification failed", __func__);
        return CHECK_PERMISSION_FAILED;
    }

    if (CheckIsRemote(deviceId)) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "get remote mission snapshot.");
        return GetRemoteMissionSnapshotInfo(deviceId, missionId, missionSnapshot);
    }
    TAG_LOGI(AAFwkTag::ABILITYMGR, "get local mission snapshot.");
    auto missionListManager = GetCurrentMissionListManager();
    CHECK_POINTER_AND_RETURN(missionListManager, INNER_ERR);
    auto token = GetAbilityTokenByMissionId(missionId);
    bool result = missionListManager->GetMissionSnapshot(missionId, token, missionSnapshot, isLowResolution);
    if (!result) {
        return INNER_ERR;
    }
    return ERR_OK;
}
#ifdef SUPPORT_SCREEN
void AbilityManagerService::UpdateMissionSnapShot(const sptr<IRemoteObject> &token,
    const std::shared_ptr<Media::PixelMap> &pixelMap)
{
    if (!PermissionVerification::GetInstance()->CheckSpecificSystemAbilityAccessPermission(FOUNDATION_PROCESS_NAME)) {
        return;
    }
    auto missionListManager = GetCurrentMissionListManager();
    if (missionListManager) {
        missionListManager->UpdateSnapShot(token, pixelMap);
    }
}
#endif // SUPPORT_SCREEN
void AbilityManagerService::EnableRecoverAbility(const sptr<IRemoteObject>& token)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Enable recovery ability.");
    if (token == nullptr) {
        return;
    }
    auto record = Token::GetAbilityRecordByToken(token);
    if (record == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "%{public}s AppRecovery::failed find abilityRecord by given token.", __func__);
        return;
    }
    if (record->IsClearMissionFlag()) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "%{public}s AppRecovery::not allow EnableRecoverAbility before clearMission.",
            __func__);
        return;
    }

    auto callingTokenId = IPCSkeleton::GetCallingTokenID();
    auto tokenID = record->GetApplicationInfo().accessTokenId;
    if (callingTokenId != tokenID) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "AppRecovery ScheduleRecoverAbility not self, not enabled");
        return;
    }
    {
        std::lock_guard<ffrt::mutex> guard(globalLock_);
        auto it = appRecoveryHistory_.find(record->GetUid());
        if (it == appRecoveryHistory_.end()) {
            appRecoveryHistory_.emplace(record->GetUid(), 0);
        }
    }
    auto userId = record->GetOwnerMissionUserId();
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        auto uiAbilityManager = GetUIAbilityManagerByUserId(userId);
        CHECK_POINTER(uiAbilityManager);
        const auto& abilityInfo = record->GetAbilityInfo();
        std::string abilityName = abilityInfo.name;
        auto sessionId = uiAbilityManager->GetSessionIdByAbilityToken(token);
        if (abilityInfo.launchMode == AppExecFwk::LaunchMode::STANDARD) {
            abilityName += std::to_string(sessionId);
        }
        (void)DelayedSingleton<AbilityRuntime::AppExitReasonDataManager>::GetInstance()->AddAbilityRecoverInfo(
            abilityInfo.applicationInfo.accessTokenId, abilityInfo.moduleName, abilityName, sessionId);
    } else {
        auto missionListMgr = GetMissionListManagerByUserId(userId);
        if (missionListMgr == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "missionListMgr is nullptr");
            return;
        }
        missionListMgr->EnableRecoverAbility(record->GetMissionId());
    }
}

void AbilityManagerService::ScheduleClearRecoveryPageStack()
{
    int32_t callerUid = IPCSkeleton::GetCallingUid();
    std::string bundleName;
    auto bms = GetBundleManager();
    if (bms == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "ScheduleClearRecoveryPageStack failed to get bms");
        return;
    }

    if (IN_PROCESS_CALL(bms->GetNameForUid(callerUid, bundleName)) != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "ScheduleClearRecoveryPageStack failed to get bundleName by uid");
        return;
    }

    auto tokenId = IPCSkeleton::GetCallingTokenID();

    TAG_LOGI(AAFwkTag::ABILITYMGR,
        "ScheduleClearRecoveryPageStack bundleName = %{public}s, callerUid = %{public}d, tokenId = %{public}d",
        bundleName.c_str(), callerUid, tokenId);
    (void)DelayedSingleton<AbilityRuntime::AppExitReasonDataManager>::GetInstance()->
        DeleteAppExitReason(bundleName, tokenId);
    (void)DelayedSingleton<AbilityRuntime::AppExitReasonDataManager>::GetInstance()->
        DeleteAllRecoverInfoByTokenId(tokenId);
}

void AbilityManagerService::ReportAppRecoverResult(const int32_t appId, const AppExecFwk::ApplicationInfo &appInfo,
    const std::string& abilityName, const std::string& result)
{
    HiSysEventWrite(HiSysEvent::Domain::AAFWK, "APP_RECOVERY", HiSysEvent::EventType::BEHAVIOR,
        "APP_UID", appId,
        "VERSION_CODE", std::to_string(appInfo.versionCode),
        "VERSION_NAME", appInfo.versionName,
        "BUNDLE_NAME", appInfo.bundleName,
        "ABILITY_NAME", abilityName,
        "RECOVERY_RESULT", result);
}

void AbilityManagerService::SubmitSaveRecoveryInfo(const sptr<IRemoteObject>& token)
{
    if (token == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "submitInfo token is nullptr");
        return;
    }
    auto abilityRecord = Token::GetAbilityRecordByToken(token);
    if (abilityRecord == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "submitInfo abilityRecord is nullptr");
        return;
    }
    auto abilityInfo = abilityRecord->GetAbilityInfo();
    auto userId = abilityRecord->GetOwnerMissionUserId();
    auto tokenId = abilityRecord->GetApplicationInfo().accessTokenId;
    auto callingTokenId = IPCSkeleton::GetCallingTokenID();
    if (callingTokenId != tokenId) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "SubmitSaveRecoveryInfo not self, not enabled");
        return;
    }
    std::string abilityName = abilityInfo.name;
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        auto uiAbilityManager = GetUIAbilityManagerByUserId(userId);
        CHECK_POINTER(uiAbilityManager);
        auto sessionId = uiAbilityManager->GetSessionIdByAbilityToken(token);
        if (abilityInfo.launchMode == AppExecFwk::LaunchMode::STANDARD) {
            abilityName += std::to_string(sessionId);
        }
    } else {
        auto missionListMgr = GetMissionListManagerByUserId(userId);
        if (missionListMgr == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "missionListMgr is nullptr");
            return;
        }
        abilityName += std::to_string(abilityRecord->GetMissionId());
    }
    TAG_LOGI(AAFwkTag::ABILITYMGR,
        "submitInfo bundleName = %{public}s, moduleName = %{public}s, abilityName = %{public}s, tokenId = %{public}d",
        abilityInfo.bundleName.c_str(),  abilityInfo.moduleName.c_str(), abilityName.c_str(), tokenId);
    RecoveryInfo recoveryInfo;
    recoveryInfo.bundleName = abilityInfo.bundleName;
    recoveryInfo.moduleName = abilityInfo.moduleName;
    recoveryInfo.abilityName = abilityName;
    recoveryInfo.time = time(nullptr);
    OHOS::AAFwk::RecoveryInfoTimer::GetInstance().SubmitSaveRecoveryInfo(recoveryInfo);
}

void AbilityManagerService::AppRecoverKill(pid_t pid, int32_t reason)
{
    AppExecFwk::AppFaultDataBySA faultDataSA;
    faultDataSA.errorObject.name = "appRecovery";
    switch (reason) {
        case AppExecFwk::StateReason::CPP_CRASH:
            faultDataSA.faultType = AppExecFwk::FaultDataType::CPP_CRASH;
            break;
        case AppExecFwk::StateReason::JS_ERROR:
            faultDataSA.faultType = AppExecFwk::FaultDataType::JS_ERROR;
            break;
        case AppExecFwk::StateReason::LIFECYCLE:
        case AppExecFwk::StateReason::APP_FREEZE:
            faultDataSA.faultType = AppExecFwk::FaultDataType::APP_FREEZE;
            break;
        default:
            faultDataSA.faultType = AppExecFwk::FaultDataType::UNKNOWN;
    }
    faultDataSA.pid = pid;
    IN_PROCESS_CALL(DelayedSingleton<AppExecFwk::AppMgrClient>::GetInstance()->NotifyAppFaultBySA(faultDataSA));
}

void AbilityManagerService::ScheduleRecoverAbility(const sptr<IRemoteObject>& token, int32_t reason, const Want *want)
{
    if (token == nullptr) {
        return;
    }
    auto record = Token::GetAbilityRecordByToken(token);
    if (record == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "%{public}s AppRecovery::failed find abilityRecord by given token.", __func__);
        return;
    }
    if (!record->IsForeground() && !record->GetAbilityForegroundingFlag()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "%{public}s AppRecovery::failed to recoveryAbility."
            "due to it is background", __func__);
        return;
    }

    auto callingTokenId = IPCSkeleton::GetCallingTokenID();
    auto tokenID = record->GetApplicationInfo().accessTokenId;
    if (callingTokenId != tokenID) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "AppRecovery ScheduleRecoverAbility not self, not enabled");
        return;
    }

    AAFwk::Want curWant;
    {
        std::lock_guard<ffrt::mutex> guard(globalLock_);
        auto type = record->GetAbilityInfo().type;
        if (type != AppExecFwk::AbilityType::PAGE) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "%{public}s AppRecovery::only do recover for page ability.", __func__);
            return;
        }

        constexpr int64_t MIN_RECOVERY_TIME = 60;
        int64_t now = time(nullptr);
        auto it = appRecoveryHistory_.find(record->GetUid());
        auto appInfo = record->GetApplicationInfo();
        auto abilityInfo = record->GetAbilityInfo();

        if ((it != appRecoveryHistory_.end()) &&
            (it->second + MIN_RECOVERY_TIME > now)) {
            TAG_LOGE(AAFwkTag::ABILITYMGR,
                "%{public}s AppRecovery recover app more than once in one minute, just kill app(%{public}d).",
                __func__, record->GetPid());
            ReportAppRecoverResult(record->GetUid(), appInfo, abilityInfo.name, "FAIL_WITHIN_ONE_MINUTE");
            AppRecoverKill(record->GetPid(), reason);
            return;
        }

        if (want != nullptr) {
            TAG_LOGD(AAFwkTag::ABILITYMGR, "BundleName:%{public}s targetBundleName:%{public}s.",
                appInfo.bundleName.c_str(), want->GetElement().GetBundleName().c_str());
            if (want->GetElement().GetBundleName().empty() ||
                (appInfo.bundleName.compare(want->GetElement().GetBundleName()) != 0)) {
                TAG_LOGE(AAFwkTag::ABILITYMGR, "AppRecovery BundleName not match, Not recovery ability!");
                ReportAppRecoverResult(record->GetUid(), appInfo, abilityInfo.name, "FAIL_BUNDLE_NAME_NOT_MATCH");
                return;
            } else if (want->GetElement().GetAbilityName().empty()) {
                TAG_LOGD(AAFwkTag::ABILITYMGR, "AppRecovery recovery target ability is empty");
                ReportAppRecoverResult(record->GetUid(), appInfo, abilityInfo.name, "FAIL_TARGET_ABILITY_EMPTY");
                return;
            } else {
                auto bms = GetBundleManager();
                if (bms == nullptr) {
                    TAG_LOGE(AAFwkTag::ABILITYMGR, "bms is nullptr");
                    return;
                }
                AppExecFwk::BundleInfo bundleInfo;
                auto bundleName = want->GetElement().GetBundleName();
                int32_t userId = GetUserId();
                bool ret = IN_PROCESS_CALL(
                    bms->GetBundleInfo(bundleName, AppExecFwk::BundleFlag::GET_BUNDLE_WITH_ABILITIES, bundleInfo,
                    userId));
                if (!ret) {
                    TAG_LOGE(AAFwkTag::ABILITYMGR, "AppRecovery Failed to get bundle info, not do recovery!");
                    return;
                }
                bool isRestartPage = false;
                auto abilityName = want->GetElement().GetAbilityName();
                for (auto it = bundleInfo.abilityInfos.begin(); it != bundleInfo.abilityInfos.end(); ++it) {
                    if ((abilityName.compare(it->name) == 0) && it->type == AppExecFwk::AbilityType::PAGE) {
                        isRestartPage = true;
                        break;
                    }
                }
                if (!isRestartPage) {
                    TAG_LOGI(AAFwkTag::ABILITYMGR, "AppRecovery the target ability type is not PAGE!");
                    ReportAppRecoverResult(record->GetUid(), appInfo, abilityName, "FAIL_TARGET_ABILITY_NOT_PAGE");
                    return;
                }
            }
        }

        appRecoveryHistory_[record->GetUid()] = now;
        curWant = (want == nullptr) ? record->GetWant() : *want;
        curWant.SetParam(AAFwk::Want::PARAM_ABILITY_RECOVERY_RESTART, true);

        ReportAppRecoverResult(record->GetUid(), appInfo, abilityInfo.name, "SUCCESS");
    }
    RestartApp(curWant);
}

int32_t AbilityManagerService::GetRemoteMissionSnapshotInfo(const std::string& deviceId, int32_t missionId,
    MissionSnapshot& missionSnapshot)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "GetRemoteMissionSnapshotInfo begin");
    std::unique_ptr<MissionSnapshot> missionSnapshotPtr = std::make_unique<MissionSnapshot>();
    DistributedClient dmsClient;
    int result = dmsClient.GetRemoteMissionSnapshotInfo(deviceId, missionId, missionSnapshotPtr);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "GetRemoteMissionSnapshotInfo failed, result = %{public}d", result);
        return result;
    }
    missionSnapshot = *missionSnapshotPtr;
    return ERR_OK;
}

void AbilityManagerService::StartSwitchUserDialog()
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Start switch user dialog extension ability come");
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Scene board enabled, dialog not show.");
        return;
    }

    if (userController_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "User Controller instance is nullptr.");
        return;
    }
#ifdef SUPPORT_GRAPHICS
    auto sysDialog = DelayedSingleton<SystemDialogScheduler>::GetInstance();
    if (sysDialog == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "System dialog scheduler instance is nullptr.");
        return;
    }

    Want dialogWant = sysDialog->GetSwitchUserDialogWant();
    StartSwitchUserDialogInner(dialogWant, userController_->GetFreezingNewUserId());
#endif // SUPPORT_GRAPHICS
}


void AbilityManagerService::StartSwitchUserDialogInner(const Want &want, int32_t lastUserId)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Start switch user dialog inner come");
    EventInfo eventInfo = BuildEventInfo(want, lastUserId);
    eventInfo.extensionType = static_cast<int32_t>(AppExecFwk::ExtensionAbilityType::SERVICE);
    AbilityRequest abilityRequest;
    auto result = GenerateExtensionAbilityRequest(want, abilityRequest, nullptr, lastUserId);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Generate ability request local error.");
        eventInfo.errCode = result;
        EventReport::SendExtensionEvent(EventName::START_EXTENSION_ERROR, HiSysEventType::FAULT, eventInfo);
        return;
    }

    auto abilityInfo = abilityRequest.abilityInfo;
    auto startUserId = abilityInfo.applicationInfo.singleton ? U0_USER_ID : lastUserId;
    result = CheckOptExtensionAbility(want, abilityRequest, startUserId, AppExecFwk::ExtensionAbilityType::SERVICE);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Check extensionAbility type error.");
        eventInfo.errCode = result;
        EventReport::SendExtensionEvent(EventName::START_EXTENSION_ERROR, HiSysEventType::FAULT, eventInfo);
        return;
    }

    auto connectManager = GetConnectManagerByUserId(startUserId);
    if (connectManager == nullptr) {
        CHECK_POINTER(subManagersHelper_);
        subManagersHelper_->InitConnectManager(startUserId, false);
        connectManager = GetConnectManagerByUserId(startUserId);
        if (connectManager == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "ConnectManager is nullptr. userId=%{public}d", startUserId);
            eventInfo.errCode = ERR_INVALID_VALUE;
            EventReport::SendExtensionEvent(EventName::START_EXTENSION_ERROR, HiSysEventType::FAULT, eventInfo);
            return;
        }
    }

    eventInfo.errCode = connectManager->StartAbility(abilityRequest);
    if (eventInfo.errCode != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "EventInfo errCode is %{public}d", eventInfo.errCode);
        EventReport::SendExtensionEvent(EventName::START_EXTENSION_ERROR, HiSysEventType::FAULT, eventInfo);
    }
}

void AbilityManagerService::StartFreezingScreen()
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "%{public}s", __func__);
#ifdef SUPPORT_GRAPHICS
    StartSwitchUserDialog();
    std::vector<Rosen::DisplayId> displayIds = Rosen::DisplayManager::GetInstance().GetAllDisplayIds();
    IN_PROCESS_CALL_WITHOUT_RET(Rosen::DisplayManager::GetInstance().Freeze(displayIds));
#endif
}

void AbilityManagerService::StopFreezingScreen()
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "%{public}s", __func__);
#ifdef SUPPORT_GRAPHICS
    std::vector<Rosen::DisplayId> displayIds = Rosen::DisplayManager::GetInstance().GetAllDisplayIds();
    IN_PROCESS_CALL_WITHOUT_RET(Rosen::DisplayManager::GetInstance().Unfreeze(displayIds));
    StopSwitchUserDialog();
#endif
}

void AbilityManagerService::UserStarted(int32_t userId)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "%{public}s", __func__);
    CHECK_POINTER(subManagersHelper_);
    subManagersHelper_->InitSubManagers(userId, false);
}

void AbilityManagerService::SwitchToUser(int32_t oldUserId, int32_t userId, sptr<IUserCallback> callback)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR,
        "%{public}s, oldUserId:%{public}d, newUserId:%{public}d", __func__, oldUserId, userId);
    SwitchManagers(userId);
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        PauseOldUser(oldUserId);
        ConnectBmsService();
        StartUserApps();
    }
    callback->OnStartUserDone(userId, ERR_OK);
    bool isBoot = oldUserId == U0_USER_ID ? true : false;
    StartHighestPriorityAbility(userId, isBoot);
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled() &&
        AmsConfigurationParameter::GetInstance().MultiUserType() != 0) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "no need to terminate old scb.");
        return;
    }
    PauseOldConnectManager(oldUserId);
}

void AbilityManagerService::SwitchManagers(int32_t userId, bool switchUser)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "%{public}s, SwitchManagers:%{public}d-----begin", __func__, userId);
    CHECK_POINTER(subManagersHelper_);
    subManagersHelper_->InitSubManagers(userId, switchUser);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "%{public}s, SwitchManagers:%{public}d-----end", __func__, userId);
}

void AbilityManagerService::PauseOldUser(int32_t userId)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "%{public}s, PauseOldUser:%{public}d-----begin", __func__, userId);
    PauseOldMissionListManager(userId);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "%{public}s, PauseOldUser:%{public}d-----end", __func__, userId);
}

void AbilityManagerService::PauseOldMissionListManager(int32_t userId)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "%{public}s, PauseOldMissionListManager:%{public}d-----begin", __func__, userId);
    auto manager = GetMissionListManagerByUserId(userId);
    CHECK_POINTER(manager);
    manager->PauseManager();
    TAG_LOGI(AAFwkTag::ABILITYMGR, "%{public}s, PauseOldMissionListManager:%{public}d-----end", __func__, userId);
}

void AbilityManagerService::PauseOldConnectManager(int32_t userId)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "%{public}s, PauseOldConnectManager:%{public}d-----begin", __func__, userId);
    if (userId == U0_USER_ID) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "%{public}s, u0 not stop, id:%{public}d-----nullptr", __func__, userId);
        return;
    }

    auto manager = GetConnectManagerByUserId(userId);
    CHECK_POINTER(manager);
    manager->PauseExtensions();
    TAG_LOGI(AAFwkTag::ABILITYMGR, "%{public}s, PauseOldConnectManager:%{public}d-----end", __func__, userId);
}

void AbilityManagerService::StartUserApps()
{
    auto missionListManager = GetCurrentMissionListManager();
    if (missionListManager && missionListManager->IsStarted()) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "missionListManager ResumeManager");
        missionListManager->ResumeManager();
    }
}

int32_t AbilityManagerService::GetValidUserId(const int32_t userId)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "userId = %{public}d.", userId);
    int32_t validUserId = userId;

    if (DEFAULT_INVAL_VALUE == userId) {
        validUserId = IPCSkeleton::GetCallingUid() / BASE_USER_RANGE;
        TAG_LOGD(AAFwkTag::ABILITYMGR, "validUserId = %{public}d, CallingUid = %{public}d.", validUserId,
            IPCSkeleton::GetCallingUid());
        if (validUserId == U0_USER_ID) {
            validUserId = GetUserId();
        }
    }
    return validUserId;
}

int AbilityManagerService::SetAbilityController(const sptr<IAbilityController> &abilityController,
    bool imAStabilityTest)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "%{public}s, imAStabilityTest: %{public}d", __func__, imAStabilityTest);
    auto isPerm = AAFwk::PermissionVerification::GetInstance()->VerifyControllerPerm();
    if (!isPerm) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "%{public}s: Permission verification failed", __func__);
        return CHECK_PERMISSION_FAILED;
    }

    std::lock_guard<ffrt::mutex> guard(globalLock_);
    abilityController_ = abilityController;
    controllerIsAStabilityTest_ = imAStabilityTest;
    TAG_LOGD(AAFwkTag::ABILITYMGR, "%{public}s, end", __func__);
    return ERR_OK;
}

bool AbilityManagerService::IsRunningInStabilityTest()
{
    std::lock_guard<ffrt::mutex> guard(globalLock_);
    bool ret = abilityController_ != nullptr && controllerIsAStabilityTest_;
    TAG_LOGD(AAFwkTag::ABILITYMGR, "%{public}s, IsRunningInStabilityTest: %{public}d", __func__, ret);
    return ret;
}

bool AbilityManagerService::IsAbilityControllerStart(const Want &want, const std::string &bundleName)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "method call, controllerIsAStabilityTest_: %{public}d", controllerIsAStabilityTest_);
    if (abilityController_ == nullptr) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "abilityController_ is nullptr");
        return true;
    }

    if (controllerIsAStabilityTest_) {
        bool isStart = abilityController_->AllowAbilityStart(want, bundleName);
        if (!isStart) {
            TAG_LOGI(AAFwkTag::ABILITYMGR,
                "Not finishing start ability because controller starting: %{public}s", bundleName.c_str());
            return false;
        }
    }
    return true;
}

bool AbilityManagerService::IsAbilityControllerForeground(const std::string &bundleName)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "method call, controllerIsAStabilityTest_: %{public}d", controllerIsAStabilityTest_);
    if (abilityController_ == nullptr) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "abilityController_ is nullptr");
        return true;
    }

    if (controllerIsAStabilityTest_) {
        bool isResume = abilityController_->AllowAbilityBackground(bundleName);
        if (!isResume) {
            TAG_LOGI(AAFwkTag::ABILITYMGR,
                "Not finishing terminate ability because controller resuming: %{public}s", bundleName.c_str());
            return false;
        }
    }
    return true;
}

int AbilityManagerService::StartUserTest(const Want &want, const sptr<IRemoteObject> &observer)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "enter");
    if (observer == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "observer is nullptr");
        return ERR_INVALID_VALUE;
    }

    std::string bundleName = want.GetStringParam("-b");
    if (bundleName.empty()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Invalid bundle name");
        return ERR_INVALID_VALUE;
    }

    auto bms = GetBundleManager();
    CHECK_POINTER_AND_RETURN(bms, START_USER_TEST_FAIL);
    AppExecFwk::BundleInfo bundleInfo;
    if (!IN_PROCESS_CALL(
        bms->GetBundleInfo(bundleName, AppExecFwk::BundleFlag::GET_BUNDLE_DEFAULT, bundleInfo, U0_USER_ID))) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Failed to get bundle info by U0_USER_ID %{public}d.", U0_USER_ID);
        int32_t userId = GetUserId();
        if (!IN_PROCESS_CALL(
            bms->GetBundleInfo(bundleName, AppExecFwk::BundleFlag::GET_BUNDLE_DEFAULT, bundleInfo, userId))) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "Failed to get bundle info by userId %{public}d.", userId);
            return GET_BUNDLE_INFO_FAILED;
        }
    }

    return DelayedSingleton<AppScheduler>::GetInstance()->StartUserTest(want, observer, bundleInfo, GetUserId());
}

int AbilityManagerService::FinishUserTest(
    const std::string &msg, const int64_t &resultCode, const std::string &bundleName)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "enter");
    if (bundleName.empty()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Invalid bundle name.");
        return ERR_INVALID_VALUE;
    }

    return DelayedSingleton<AppScheduler>::GetInstance()->FinishUserTest(msg, resultCode, bundleName);
}

int AbilityManagerService::GetTopAbility(sptr<IRemoteObject> &token)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto isSaCall = AAFwk::PermissionVerification::GetInstance()->IsSACall();
    if (!isSaCall) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Permission verification failed");
        return CHECK_PERMISSION_FAILED;
    }
#ifdef SUPPORT_SCREEN
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        Rosen::FocusChangeInfo focusChangeInfo;
        Rosen::WindowManager::GetInstance().GetFocusWindowInfo(focusChangeInfo);
        token = focusChangeInfo.abilityToken_;
    } else {
        if (!wmsHandler_) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "wmsHandler_ is nullptr.");
            return ERR_INVALID_VALUE;
        }
        wmsHandler_->GetFocusWindow(token);
    }

    if (!token) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "token is nullptr");
        return ERR_INVALID_VALUE;
    }
#endif
    return ERR_OK;
}

int AbilityManagerService::DelegatorDoAbilityForeground(const sptr<IRemoteObject> &token)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "enter");
    CHECK_POINTER_AND_RETURN(token, ERR_INVALID_VALUE);
    auto &&abilityRecord = Token::GetAbilityRecordByToken(token);
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);
    int32_t callerPid = IPCSkeleton::GetCallingPid();
    int32_t appPid = abilityRecord->GetPid();
    TAG_LOGD(AAFwkTag::ABILITYMGR, "callerPid: %{public}d, appPid: %{public}d", callerPid, appPid);
    if (callerPid != appPid) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Caller is not the application itself");
        return ERR_INVALID_VALUE;
    }
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        auto sessionId = GetMissionIdByAbilityTokenInner(token);
        if (!sessionId) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "Invalid session id.");
            return ERR_INVALID_VALUE;
        }
        auto want = abilityRecord->GetWant();
        if (!IsAbilityControllerStart(want, want.GetBundle())) {
            TAG_LOGE(AAFwkTag::ABILITYMGR,
                "SceneBoard IsAbilityControllerStart failed: %{public}s", want.GetBundle().c_str());
            return ERR_WOULD_BLOCK;
        }
        return ERR_OK;
    }
    auto missionId = GetMissionIdByAbilityToken(token);
    if (missionId < 0) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Invalid mission id.");
        return ERR_INVALID_VALUE;
    }
    return DelegatorMoveMissionToFront(missionId);
}

int AbilityManagerService::DelegatorDoAbilityBackground(const sptr<IRemoteObject> &token)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "enter");
    CHECK_POINTER_AND_RETURN(token, ERR_INVALID_VALUE);
    auto &&abilityRecord = Token::GetAbilityRecordByToken(token);
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);
    int32_t appPid = abilityRecord->GetPid();
    int32_t callerPid = IPCSkeleton::GetCallingPid();
    TAG_LOGD(AAFwkTag::ABILITYMGR, "callerPid: %{public}d, appPid: %{public}d", callerPid, appPid);
    if (callerPid != appPid) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Caller is not the application itself");
        return ERR_INVALID_VALUE;
    }
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        return ERR_OK;
    }
    return MinimizeAbility(token, true);
}

int AbilityManagerService::DoAbilityForeground(const sptr<IRemoteObject> &token, uint32_t flag)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "DoAbilityForeground, sceneFlag:%{public}u", flag);
    CHECK_POINTER_AND_RETURN(token, ERR_INVALID_VALUE);
    if (!VerificationToken(token) && !VerificationAllToken(token)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "%{public}s token error.", __func__);
        return ERR_INVALID_VALUE;
    }

    std::lock_guard<ffrt::mutex> guard(globalLock_);
    auto abilityRecord = Token::GetAbilityRecordByToken(token);
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);
    if (!JudgeSelfCalled(abilityRecord)) {
        return CHECK_PERMISSION_FAILED;
    }

    auto type = abilityRecord->GetAbilityInfo().type;
    if (type != AppExecFwk::AbilityType::PAGE) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Cannot minimize except page ability.");
        return ERR_WRONG_INTERFACE_CALL;
    }

    if (!IsAbilityControllerForeground(abilityRecord->GetAbilityInfo().bundleName)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "IsAbilityControllerForeground false.");
        return ERR_WOULD_BLOCK;
    }

    auto missionListManager = GetCurrentMissionListManager();
    CHECK_POINTER_AND_RETURN(missionListManager, ERR_NO_INIT);
    return missionListManager->DoAbilityForeground(abilityRecord, flag);
}

int AbilityManagerService::DoAbilityBackground(const sptr<IRemoteObject> &token, uint32_t flag)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "DoAbilityBackground, sceneFlag:%{public}u", flag);
    CHECK_POINTER_AND_RETURN(token, ERR_INVALID_VALUE);

    auto abilityRecord = Token::GetAbilityRecordByToken(token);
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);

    abilityRecord->lifeCycleStateInfo_.sceneFlag = flag;
    int ret = MinimizeAbility(token);
    abilityRecord->lifeCycleStateInfo_.sceneFlag = SCENE_FLAG_NORMAL;
    return ret;
}

int AbilityManagerService::DelegatorMoveMissionToFront(int32_t missionId)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "enter missionId : %{public}d", missionId);
    auto missionListManager = GetCurrentMissionListManager();
    CHECK_POINTER_AND_RETURN(missionListManager, ERR_NO_INIT);

    if (!IsAbilityControllerStartById(missionId)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "IsAbilityControllerStart false");
        return ERR_WOULD_BLOCK;
    }

    return missionListManager->MoveMissionToFront(missionId);
}

void AbilityManagerService::UpdateCallerInfo(Want& want, const sptr<IRemoteObject> &callerToken)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (!StartAbilityUtils::IsCallFromAncoShellOrBroker(callerToken)) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "not call from anco or broker.");
        want.RemoveParam(PARAM_RESV_ANCO_CALLER_UID);
        want.RemoveParam(PARAM_RESV_ANCO_CALLER_BUNDLENAME);
    }
    int32_t tokenId = static_cast<int32_t>(IPCSkeleton::GetCallingTokenID());
    int32_t callerUid = IPCSkeleton::GetCallingUid();
    int32_t callerPid = IPCSkeleton::GetCallingPid();
    want.RemoveParam(Want::PARAM_RESV_CALLER_TOKEN);
    want.SetParam(Want::PARAM_RESV_CALLER_TOKEN, tokenId);
    want.RemoveParam(Want::PARAM_RESV_CALLER_UID);
    want.SetParam(Want::PARAM_RESV_CALLER_UID, callerUid);
    want.RemoveParam(Want::PARAM_RESV_CALLER_PID);
    want.SetParam(Want::PARAM_RESV_CALLER_PID, callerPid);

    auto abilityRecord = Token::GetAbilityRecordByToken(callerToken);
    if (!abilityRecord) {
        std::string bundleName = "";
        auto bundleMgr = GetBundleManager();
        if (bundleMgr != nullptr) {
            IN_PROCESS_CALL(bundleMgr->GetNameForUid(callerUid, bundleName));
        }
        want.RemoveParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME);
        want.SetParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME, bundleName);
        want.RemoveParam(Want::PARAM_RESV_CALLER_ABILITY_NAME);
        want.SetParam(Want::PARAM_RESV_CALLER_ABILITY_NAME, std::string(""));
        return;
    }
    std::string callerBundleName = abilityRecord->GetAbilityInfo().bundleName;
    want.RemoveParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME);
    want.SetParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME, callerBundleName);
    std::string callerAbilityName = abilityRecord->GetAbilityInfo().name;
    want.RemoveParam(Want::PARAM_RESV_CALLER_ABILITY_NAME);
    want.SetParam(Want::PARAM_RESV_CALLER_ABILITY_NAME, callerAbilityName);
}

void AbilityManagerService::UpdateAsCallerSourceInfo(Want& want, sptr<IRemoteObject> asCallerSourceToken,
    sptr<IRemoteObject> callerToken)
{
    if (asCallerSourceToken != nullptr) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "Update as caller source info from token.");
        UpdateAsCallerInfoFromToken(want, asCallerSourceToken);
    } else if (callerToken != nullptr) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "Update as caller source info from callerRecord.");
        UpdateAsCallerInfoFromCallerRecord(want, callerToken);
    }
}

void AbilityManagerService::UpdateAsCallerInfoFromToken(Want& want, sptr<IRemoteObject> asCallerSourceToken)
{
    if (!StartAbilityUtils::IsCallFromAncoShellOrBroker(asCallerSourceToken)) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "not call from anco or broker.");
        want.RemoveParam(PARAM_RESV_ANCO_CALLER_UID);
        want.RemoveParam(PARAM_RESV_ANCO_CALLER_BUNDLENAME);
    }
    want.RemoveParam(Want::PARAM_RESV_CALLER_TOKEN);
    want.RemoveParam(Want::PARAM_RESV_CALLER_UID);
    want.RemoveParam(Want::PARAM_RESV_CALLER_PID);
    want.RemoveParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME);
    want.RemoveParam(Want::PARAM_RESV_CALLER_ABILITY_NAME);

    auto abilityRecord = Token::GetAbilityRecordByToken(asCallerSourceToken);
    if (abilityRecord == nullptr) {
#ifdef SUPPORT_SCREEN
        UpdateAsCallerInfoFromDialog(want);
#endif // SUPPORT_SCREEN
        return;
    }
    AppExecFwk::RunningProcessInfo processInfo = {};
    DelayedSingleton<AppScheduler>::GetInstance()->GetRunningProcessInfoByToken(asCallerSourceToken, processInfo);
    int32_t tokenId = abilityRecord->GetApplicationInfo().accessTokenId;
    want.SetParam(Want::PARAM_RESV_CALLER_TOKEN, tokenId);
    want.SetParam(Want::PARAM_RESV_CALLER_UID, processInfo.uid_);
    want.SetParam(Want::PARAM_RESV_CALLER_PID, processInfo.pid_);

    std::string callerBundleName = abilityRecord->GetAbilityInfo().bundleName;
    want.SetParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME, callerBundleName);
    std::string callerAbilityName = abilityRecord->GetAbilityInfo().name;
    want.SetParam(Want::PARAM_RESV_CALLER_ABILITY_NAME, callerAbilityName);
}

void AbilityManagerService::UpdateAsCallerInfoFromCallerRecord(Want& want, sptr<IRemoteObject> callerToken)
{
    if (!StartAbilityUtils::IsCallFromAncoShellOrBroker(callerToken)) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "not call from anco or broker.");
        want.RemoveParam(PARAM_RESV_ANCO_CALLER_UID);
        want.RemoveParam(PARAM_RESV_ANCO_CALLER_BUNDLENAME);
    }
    want.RemoveParam(Want::PARAM_RESV_CALLER_TOKEN);
    want.RemoveParam(Want::PARAM_RESV_CALLER_UID);
    want.RemoveParam(Want::PARAM_RESV_CALLER_PID);
    want.RemoveParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME);
    want.RemoveParam(Want::PARAM_RESV_CALLER_ABILITY_NAME);
    auto callerRecord = Token::GetAbilityRecordByToken(callerToken);
    CHECK_POINTER(callerRecord);
    auto sourceInfo = callerRecord->GetCallerInfo();
    CHECK_POINTER(sourceInfo);
    want.SetParam(Want::PARAM_RESV_CALLER_TOKEN, sourceInfo->callerTokenId);
    want.SetParam(Want::PARAM_RESV_CALLER_UID, sourceInfo->callerUid);
    want.SetParam(Want::PARAM_RESV_CALLER_PID, sourceInfo->callerPid);
    want.SetParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME, sourceInfo->callerBundleName);
    want.SetParam(Want::PARAM_RESV_CALLER_ABILITY_NAME, sourceInfo->callerAbilityName);
}

void AbilityManagerService::UpdateAsCallerInfoFromDialog(Want& want)
{
    std::string dialogSessionId = want.GetStringParam("dialogSessionId");
    auto dialogCallerInfo = DialogSessionManager::GetInstance().GetDialogCallerInfo(dialogSessionId);
    if (dialogCallerInfo == nullptr) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "failed to get dialog caller info.");
        return;
    }
    Want dialogCallerWant = dialogCallerInfo->targetWant;
    int32_t tokenId = dialogCallerWant.GetIntParam(Want::PARAM_RESV_CALLER_TOKEN, 0);
    int32_t uid = dialogCallerWant.GetIntParam(Want::PARAM_RESV_CALLER_UID, 0);
    int32_t pid = dialogCallerWant.GetIntParam(Want::PARAM_RESV_CALLER_PID, 0);
    std::string callerBundleName = dialogCallerWant.GetStringParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME);
    std::string callerAbilityName = dialogCallerWant.GetStringParam(Want::PARAM_RESV_CALLER_ABILITY_NAME);
    want.SetParam(Want::PARAM_RESV_CALLER_TOKEN, tokenId);
    want.SetParam(Want::PARAM_RESV_CALLER_UID, uid);
    want.SetParam(Want::PARAM_RESV_CALLER_PID, pid);
    want.SetParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME, callerBundleName);
    want.SetParam(Want::PARAM_RESV_CALLER_ABILITY_NAME, callerAbilityName);
}

void AbilityManagerService::UpdateCallerInfoFromToken(Want& want, const sptr<IRemoteObject> &token)
{
    auto abilityRecord = Token::GetAbilityRecordByToken(token);
    if (!abilityRecord) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "caller abilityRecord is null.");
        return;
    }

    int32_t tokenId = abilityRecord->GetApplicationInfo().accessTokenId;
    int32_t callerUid = abilityRecord->GetUid();
    int32_t callerPid = abilityRecord->GetPid();
    want.RemoveParam(Want::PARAM_RESV_CALLER_TOKEN);
    want.SetParam(Want::PARAM_RESV_CALLER_TOKEN, tokenId);
    want.RemoveParam(Want::PARAM_RESV_CALLER_UID);
    want.SetParam(Want::PARAM_RESV_CALLER_UID, callerUid);
    want.RemoveParam(Want::PARAM_RESV_CALLER_PID);
    want.SetParam(Want::PARAM_RESV_CALLER_PID, callerPid);

    std::string callerBundleName = abilityRecord->GetAbilityInfo().bundleName;
    want.RemoveParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME);
    want.SetParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME, callerBundleName);
    std::string callerAbilityName = abilityRecord->GetAbilityInfo().name;
    want.RemoveParam(Want::PARAM_RESV_CALLER_ABILITY_NAME);
    want.SetParam(Want::PARAM_RESV_CALLER_ABILITY_NAME, callerAbilityName);
}

bool AbilityManagerService::JudgeMultiUserConcurrency(const int32_t userId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (userId == U0_USER_ID) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "%{public}s, userId is 0.", __func__);
        return true;
    }

    TAG_LOGD(AAFwkTag::ABILITYMGR, "userId : %{public}d, current userId : %{public}d", userId, GetUserId());

    // Only non-concurrent mode is supported
    bool concurrencyMode = CONCURRENCY_MODE_FALSE;
    if (!concurrencyMode) {
        return (userId == GetUserId());
    }

    return true;
}

#ifdef ABILITY_COMMAND_FOR_TEST
int AbilityManagerService::ForceTimeoutForTest(const std::string &abilityName, const std::string &state)
{
    if (abilityName.empty()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "abilityName is empty.");
        return INVALID_DATA;
    }
    if (abilityName == "clean") {
        timeoutMap_.clear();
        return ERR_OK;
    }
    if (state != AbilityRecord::ConvertAbilityState(AbilityState::INITIAL) &&
        state != AbilityRecord::ConvertAbilityState(AbilityState::INACTIVE) &&
        state != AbilityRecord::ConvertAbilityState(AbilityState::FOREGROUND) &&
        state != AbilityRecord::ConvertAbilityState(AbilityState::BACKGROUND) &&
        state != AbilityRecord::ConvertAbilityState(AbilityState::TERMINATING) &&
        state != std::string("COMMAND")) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "lifecycle state is invalid.");
        return INVALID_DATA;
    }
    timeoutMap_.insert(std::make_pair(state, abilityName));
    return ERR_OK;
}
#endif

int AbilityManagerService::CheckStaticCfgPermissionForAbility(const AppExecFwk::AbilityInfo &abilityInfo,
    uint32_t tokenId)
{
    if (abilityInfo.permissions.empty() || AccessTokenKit::VerifyAccessToken(tokenId,
        PermissionConstants::PERMISSION_START_INVISIBLE_ABILITY, false) == ERR_OK) {
        return AppExecFwk::Constants::PERMISSION_GRANTED;
    }

    for (auto permission : abilityInfo.permissions) {
        if (AccessTokenKit::VerifyAccessToken(tokenId, permission, false) !=
            AppExecFwk::Constants::PERMISSION_GRANTED) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "verify access token fail, Ability permission: %{public}s",
                permission.c_str());
            return AppExecFwk::Constants::PERMISSION_NOT_GRANTED;
        }
    }

    return AppExecFwk::Constants::PERMISSION_GRANTED;
}

bool AbilityManagerService::CheckOneSkillPermission(const AppExecFwk::Skill &skill, uint32_t tokenId)
{
    for (auto permission : skill.permissions) {
        if (AccessTokenKit::VerifyAccessToken(tokenId, permission, false) !=
            AppExecFwk::Constants::PERMISSION_GRANTED) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "verify access token fail, Skill permission: %{public}s",
                permission.c_str());
            return false;
        }
    }

    return true;
}

int AbilityManagerService::CheckStaticCfgPermissionForSkill(const AppExecFwk::AbilityRequest &abilityRequest,
    uint32_t tokenId)
{
    auto abilityInfo = abilityRequest.abilityInfo;
    auto resultAbilityPermission = CheckStaticCfgPermissionForAbility(abilityInfo, tokenId);
    if (resultAbilityPermission != AppExecFwk::Constants::PERMISSION_GRANTED) {
        return resultAbilityPermission;
    }

    if (abilityInfo.skills.empty()) {
        return AppExecFwk::Constants::PERMISSION_GRANTED;
    }
    int32_t result = AppExecFwk::Constants::PERMISSION_GRANTED;
    for (auto skill : abilityInfo.skills) {
        if (skill.Match(abilityRequest.want)) {
            if (CheckOneSkillPermission(skill, tokenId)) {
                return AppExecFwk::Constants::PERMISSION_GRANTED;
            } else {
                result = AppExecFwk::Constants::PERMISSION_NOT_GRANTED;
            }
        }
    }
    return result;
}

int AbilityManagerService::CheckStaticCfgPermission(const AppExecFwk::AbilityRequest &abilityRequest,
    bool isStartAsCaller, uint32_t callerTokenId, bool isData, bool isSaCall, bool isImplicit)
{
    auto abilityInfo = abilityRequest.abilityInfo;
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (!isData) {
        isSaCall = AAFwk::PermissionVerification::GetInstance()->IsSACall();
    }
    if (isSaCall) {
        // do not need check static config permission when start ability by SA
        return AppExecFwk::Constants::PERMISSION_GRANTED;
    }

    uint32_t tokenId;
    if (isStartAsCaller) {
        tokenId = callerTokenId;
    } else {
        tokenId = IPCSkeleton::GetCallingTokenID();
    }

    if (abilityInfo.applicationInfo.accessTokenId == tokenId) {
        return ERR_OK;
    }

    if ((abilityInfo.type == AppExecFwk::AbilityType::EXTENSION &&
        abilityInfo.extensionAbilityType == AppExecFwk::ExtensionAbilityType::DATASHARE) ||
        (abilityInfo.type == AppExecFwk::AbilityType::DATA)) {
        // just need check the read permission and write permission of extension ability or data ability
        if (!abilityInfo.readPermission.empty()) {
            int checkReadPermission = AccessTokenKit::VerifyAccessToken(tokenId, abilityInfo.readPermission, false);
            if (checkReadPermission == ERR_OK) {
                return AppExecFwk::Constants::PERMISSION_GRANTED;
            }
            TAG_LOGW(AAFwkTag::ABILITYMGR,
                "verify access token fail, read permission: %{public}s", abilityInfo.readPermission.c_str());
        }
        if (!abilityInfo.writePermission.empty()) {
            int checkWritePermission = AccessTokenKit::VerifyAccessToken(tokenId, abilityInfo.writePermission, false);
            if (checkWritePermission == ERR_OK) {
                return AppExecFwk::Constants::PERMISSION_GRANTED;
            }
            TAG_LOGW(AAFwkTag::ABILITYMGR,
                "verify access token fail, write permission: %{public}s", abilityInfo.writePermission.c_str());
        }

        if (!abilityInfo.readPermission.empty() || !abilityInfo.writePermission.empty()) {
            // 'readPermission' and 'writePermission' take precedence over 'permission'
            // when 'readPermission' or 'writePermission' is not empty, no need check 'permission'
            return AppExecFwk::Constants::PERMISSION_NOT_GRANTED;
        }
    }

    if (!isImplicit) {
        return CheckStaticCfgPermissionForAbility(abilityInfo, tokenId);
    }
    return CheckStaticCfgPermissionForSkill(abilityRequest, tokenId);
}

int AbilityManagerService::CheckPermissionForUIService(const Want &want, const AbilityRequest &abilityRequest)
{
    AppExecFwk::ExtensionAbilityType extType = abilityRequest.abilityInfo.extensionAbilityType;
    if (want.HasParameter(UISERVICEHOSTPROXY_KEY) && extType != AppExecFwk::ExtensionAbilityType::UI_SERVICE) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Target ability is not UI_SERVICE");
        return ERR_WRONG_INTERFACE_CALL;
    } else if (!want.HasParameter(UISERVICEHOSTPROXY_KEY) && extType == AppExecFwk::ExtensionAbilityType::UI_SERVICE) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "need UISERVICEHOSTPROXY_KEY to connect UI_SERVICE");
        return ERR_WRONG_INTERFACE_CALL;
    }

    if (extType != AppExecFwk::ExtensionAbilityType::UI_SERVICE) {
        return ERR_OK;
    }

    AAFwk::PermissionVerification::VerificationInfo verificationInfo = CreateVerificationInfo(abilityRequest);
    if (IsCallFromBackground(abilityRequest, verificationInfo.isBackgroundCall) != ERR_OK) {
        return ERR_INVALID_VALUE;
    }

    int result = AAFwk::PermissionVerification::GetInstance()->CheckCallServiceExtensionPermission(verificationInfo);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "CheckCallServiceExtensionPermission failed");
        return result;
    }

    return ERR_OK;
}

bool AbilityManagerService::IsNeedTimeoutForTest(const std::string &abilityName, const std::string &state) const
{
    for (auto iter = timeoutMap_.begin(); iter != timeoutMap_.end(); iter++) {
        if (iter->first == state && iter->second == abilityName) {
            return true;
        }
    }
    return false;
}

bool AbilityManagerService::GetValidDataAbilityUri(const std::string &abilityInfoUri, std::string &adjustUri)
{
    // note: do not use abilityInfo.uri directly, need check uri first.
    size_t firstSeparator = abilityInfoUri.find_first_of('/');
    size_t lastSeparator = abilityInfoUri.find_last_of('/');
    if (lastSeparator - firstSeparator != 1) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "ability info uri error, uri: %{public}s", abilityInfoUri.c_str());
        return false;
    }

    adjustUri = abilityInfoUri;
    adjustUri.insert(lastSeparator, "/");
    return true;
}

bool AbilityManagerService::GetDataAbilityUri(const std::vector<AppExecFwk::AbilityInfo> &abilityInfos,
    const std::string &mainAbility, std::string &uri)
{
    if (abilityInfos.empty() || mainAbility.empty()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR,
            "abilityInfos or mainAbility is empty. mainAbility: %{public}s", mainAbility.c_str());
        return false;
    }

    std::string dataAbilityUri;
    for (auto abilityInfo : abilityInfos) {
        if (abilityInfo.type == AppExecFwk::AbilityType::DATA &&
            abilityInfo.name == mainAbility) {
            dataAbilityUri = abilityInfo.uri;
            TAG_LOGI(AAFwkTag::ABILITYMGR, "get data ability uri: %{public}s", dataAbilityUri.c_str());
            break;
        }
    }

    return GetValidDataAbilityUri(dataAbilityUri, uri);
}

void AbilityManagerService::GetAbilityRunningInfo(std::vector<AbilityRunningInfo> &info,
    std::shared_ptr<AbilityRecord> &abilityRecord)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    AbilityRunningInfo runningInfo;
    AppExecFwk::RunningProcessInfo processInfo;

    runningInfo.ability = abilityRecord->GetElementName();
    runningInfo.startTime = abilityRecord->GetStartTime();
    runningInfo.abilityState = static_cast<int>(abilityRecord->GetAbilityState());

    DelayedSingleton<AppScheduler>::GetInstance()->
        GetRunningProcessInfoByToken(abilityRecord->GetToken(), processInfo);
    runningInfo.pid = processInfo.pid_;
    runningInfo.uid = processInfo.uid_;
    runningInfo.processName = processInfo.processName_;
    runningInfo.appCloneIndex = processInfo.appCloneIndex;
    info.emplace_back(runningInfo);
}

int AbilityManagerService::VerifyAccountPermission(int32_t userId)
{
    if ((userId < 0) || (userController_ && (userController_->GetCurrentUserId() == userId))) {
        return ERR_OK;
    }
    return AAFwk::PermissionVerification::GetInstance()->VerifyAccountPermission();
}

#ifdef ABILITY_COMMAND_FOR_TEST
int AbilityManagerService::BlockAmsService()
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "%{public}s", __func__);
    if (AAFwk::PermissionVerification::GetInstance()->IsShellCall()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Not shell call");
        return ERR_PERMISSION_DENIED;
    }
    if (taskHandler_) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "%{public}s begin post block ability manager service task", __func__);
        auto BlockAmsServiceTask = [aams = shared_from_this()]() {
            while (1) {
                TAG_LOGD(AAFwkTag::ABILITYMGR, "%{public}s begin waiting", __func__);
                std::this_thread::sleep_for(BLOCK_AMS_SERVICE_TIME*1s);
            }
        };
        taskHandler_->SubmitTask(BlockAmsServiceTask, "blockamsservice");
        return ERR_OK;
    }
    return ERR_NO_INIT;
}

int AbilityManagerService::BlockAbility(int32_t abilityRecordId)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "%{public}s", __func__);
    if (AAFwk::PermissionVerification::GetInstance()->IsShellCall()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Not shell call");
        return ERR_PERMISSION_DENIED;
    }
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        auto uiAbilityManager = GetCurrentUIAbilityManager();
        CHECK_POINTER_AND_RETURN(uiAbilityManager, ERR_INVALID_VALUE);
        return uiAbilityManager->BlockAbility(abilityRecordId);
    }
    auto missionListManager = GetCurrentMissionListManager();
    CHECK_POINTER_AND_RETURN(missionListManager, ERR_NO_INIT);
    return missionListManager->BlockAbility(abilityRecordId);
}

int AbilityManagerService::BlockAppService()
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "%{public}s", __func__);
    if (AAFwk::PermissionVerification::GetInstance()->IsShellCall()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Not shell call");
        return ERR_PERMISSION_DENIED;
    }
    return DelayedSingleton<AppScheduler>::GetInstance()->BlockAppService();
}
#endif

int AbilityManagerService::FreeInstallAbilityFromRemote(const Want &want, const sptr<IRemoteObject> &callback,
    int32_t userId, int requestCode)
{
    auto callingUid = IPCSkeleton::GetCallingUid();
    if (callingUid != DMS_UID) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "The interface only support for DMS");
        return CHECK_PERMISSION_FAILED;
    }
    int32_t validUserId = GetValidUserId(userId);
    if (freeInstallManager_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "freeInstallManager_ is nullptr");
        return ERR_INVALID_VALUE;
    }
    return freeInstallManager_->FreeInstallAbilityFromRemote(want, callback, validUserId, requestCode);
}

AppExecFwk::ElementName AbilityManagerService::GetTopAbility(bool isNeedLocalDeviceId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "%{public}s start.", __func__);
    AppExecFwk::ElementName elementName = {};
    if (!PermissionVerification::GetInstance()->JudgeCallerIsAllowedToUseSystemAPI()) {
        auto callerPid = IPCSkeleton::GetCallingPid();
        AppExecFwk::RunningProcessInfo processInfo;
        DelayedSingleton<AppScheduler>::GetInstance()->GetRunningProcessInfoByPid(callerPid, processInfo);
        if (!processInfo.isTestProcess) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "caller can not use system-api or not test process.");
            return elementName;
        }
    }
#ifdef SUPPORT_GRAPHICS
    sptr<IRemoteObject> token;
    int ret = IN_PROCESS_CALL(GetTopAbility(token));
    if (ret) {
        return elementName;
    }
    if (!token) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "token is nullptr");
        return elementName;
    }
    auto abilityRecord = Token::GetAbilityRecordByToken(token);
    if (abilityRecord == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "%{public}s abilityRecord is null.", __func__);
        return elementName;
    }
    elementName = abilityRecord->GetElementName();
    bool isDeviceEmpty = elementName.GetDeviceID().empty();
    std::string localDeviceId;
    if (isDeviceEmpty && isNeedLocalDeviceId && GetLocalDeviceId(localDeviceId)) {
        elementName.SetDeviceID(localDeviceId);
    }
#endif
    return elementName;
}

AppExecFwk::ElementName AbilityManagerService::GetElementNameByToken(sptr<IRemoteObject> token,
    bool isNeedLocalDeviceId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "%{public}s start.", __func__);
    AppExecFwk::ElementName elementName = {};
#ifdef SUPPORT_GRAPHICS
    if (!token) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "token is nullptr");
        return elementName;
    }
    auto abilityRecord = Token::GetAbilityRecordByToken(token);
    if (abilityRecord == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "%{public}s abilityRecord is null.", __func__);
        return elementName;
    }
    elementName = abilityRecord->GetElementName();
    bool isDeviceEmpty = elementName.GetDeviceID().empty();
    std::string localDeviceId;
    if (isDeviceEmpty && isNeedLocalDeviceId && GetLocalDeviceId(localDeviceId)) {
        elementName.SetDeviceID(localDeviceId);
    }
#endif
    return elementName;
}

int AbilityManagerService::Dump(int fd, const std::vector<std::u16string>& args)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Dump begin fd: %{public}d", fd);
    std::string result;
    auto errCode = Dump(args, result);
    int ret = dprintf(fd, "%s\n", result.c_str());
    if (ret < 0) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "dprintf error");
        return ERR_AAFWK_HIDUMP_ERROR;
    }
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Dump end");
    return errCode;
}

int AbilityManagerService::Dump(const std::vector<std::u16string>& args, std::string& result)
{
    ErrCode errCode = ERR_OK;
    auto size = args.size();
    if (size == 0) {
        ShowHelp(result);
        return errCode;
    }

    std::vector<std::string> argsStr;
    for (auto arg : args) {
        argsStr.emplace_back(Str16ToStr8(arg));
    }

    if (argsStr[0] == "-h") {
        ShowHelp(result);
    } else {
        errCode = ProcessMultiParam(argsStr, result);
        if (errCode == ERR_AAFWK_HIDUMP_INVALID_ARGS) {
            ShowIllegalInfomation(result);
        }
    }
    return errCode;
}

ErrCode AbilityManagerService::ProcessMultiParam(std::vector<std::string>& argsStr, std::string& result)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "%{public}s begin", __func__);
    bool isClient = false;
    bool isUser = false;
    int userID = DEFAULT_INVAL_VALUE;
    std::vector<std::string>::iterator it;
    for (it = argsStr.begin(); it != argsStr.end();) {
        if (*it == ARGS_CLIENT) {
            isClient = true;
            it = argsStr.erase(it);
            continue;
        }
        if (*it == ARGS_USER_ID) {
            it = argsStr.erase(it);
            if (it == argsStr.end()) {
                TAG_LOGE(AAFwkTag::ABILITYMGR, "ARGS_USER_ID id invalid");
                return ERR_AAFWK_HIDUMP_INVALID_ARGS;
            }
            (void)StrToInt(*it, userID);
            if (userID < 0) {
                TAG_LOGE(AAFwkTag::ABILITYMGR, "ARGS_USER_ID id invalid");
                return ERR_AAFWK_HIDUMP_INVALID_ARGS;
            }
            isUser = true;
            it = argsStr.erase(it);
            continue;
        }
        it++;
    }
    std::string cmd;
    for (unsigned int i = 0; i < argsStr.size(); i++) {
        cmd.append(argsStr[i]);
        if (i != argsStr.size() - 1) {
            cmd.append(" ");
        }
    }
    TAG_LOGI(AAFwkTag::ABILITYMGR, "%{public}s, isClient:%{public}d, userID is : %{public}d, cmd is : %{public}s",
        __func__, isClient, userID, cmd.c_str());

    std::vector<std::string> dumpResults;
    DumpSysState(cmd, dumpResults, isClient, isUser, userID);
    for (auto it : dumpResults) {
        result += it + "\n";
    }
    return ERR_OK;
}

void AbilityManagerService::ShowHelp(std::string& result)
{
    result.append("Usage:\n")
        .append("-h                          ")
        .append("help text for the tool\n")
        .append("-a [-c | -u {UserId}]       ")
        .append("dump all ability infomation in the system or all ability infomation of client/UserId\n")
        .append("-l                          ")
        .append("dump all mission list information in the system\n")
        .append("-i {AbilityRecordId}        ")
        .append("dump an ability infomation by ability record id\n")
        .append("-e                          ")
        .append("dump all extension infomation in the system(FA: ServiceAbilityRecords, Stage: ExtensionRecords)\n")
        .append("-p [PendingWantRecordId]    ")
        .append("dump all pendingwant record infomation in the system\n")
        .append("-r                          ")
        .append("dump all process in the system\n")
        .append("-d                          ")
        .append("dump all data ability infomation in the system");
}

void AbilityManagerService::ShowIllegalInfomation(std::string& result)
{
    result.append(ILLEGAL_INFOMATION);
}

int AbilityManagerService::DumpAbilityInfoDone(std::vector<std::string> &infos, const sptr<IRemoteObject> &callerToken)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "DumpAbilityInfoDone begin");
    auto abilityRecord = Token::GetAbilityRecordByToken(callerToken);
    if (abilityRecord == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "abilityRecord nullptr");
        return ERR_INVALID_VALUE;
    }
    if (!JudgeSelfCalled(abilityRecord)) {
        return CHECK_PERMISSION_FAILED;
    }
    abilityRecord->DumpAbilityInfoDone(infos);
    return ERR_OK;
}

int AbilityManagerService::SetMissionContinueState(const sptr<IRemoteObject> &token, const AAFwk::ContinueState &state)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "SetMissionContinueState begin. State: %{public}d", state);

    CHECK_POINTER_AND_RETURN(token, ERR_INVALID_VALUE);

    int32_t missionId = GetMissionIdByAbilityToken(token);
    if (missionId == -1) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "SetMissionContinueState failed to get missionId. State: %{public}d", state);
        return ERR_INVALID_VALUE;
    }

    auto abilityRecord = Token::GetAbilityRecordByToken(token);
    if (!abilityRecord) {
        TAG_LOGE(AAFwkTag::ABILITYMGR,
            "SetMissionContinueState: No such ability record. Mission id: %{public}d, state: %{public}d",
            missionId, state);
        return -1;
    }

    auto callingTokenId = IPCSkeleton::GetCallingTokenID();
    auto tokenID = abilityRecord->GetApplicationInfo().accessTokenId;
    if (callingTokenId != tokenID) {
        TAG_LOGE(AAFwkTag::ABILITYMGR,
            "SetMissionContinueState not self, not enabled. Mission id: %{public}d, state: %{public}d",
            missionId, state);
        return -1;
    }

    auto userId = abilityRecord->GetOwnerMissionUserId();
    auto missionListManager = GetMissionListManagerByUserId(userId);
    if (!missionListManager) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "failed to find mission list manager. Mission id: %{public}d, state: %{public}d",
            missionId, state);
        return -1;
    }

    auto setResult = missionListManager->SetMissionContinueState(token, missionId, state);
    if (setResult != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR,
            "missionListManager set failed, result: %{public}d, mission id: %{public}d, state: %{public}d",
            setResult, missionId, state);
        return setResult;
    }

    DistributedClient dmsClient;
    auto result =  dmsClient.SetMissionContinueState(missionId, state);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR,
            "Notify DMS client failed, result: %{public}d. Mission id: %{public}d, state: %{public}d",
            result, missionId, state);
    }

    TAG_LOGD(AAFwkTag::ABILITYMGR,
        "SetMissionContinueState end. Mission id: %{public}d, state: %{public}d", missionId, state);
    return ERR_OK;
}

#ifdef SUPPORT_SCREEN
int AbilityManagerService::SetMissionLabel(const sptr<IRemoteObject> &token, const std::string &label)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "%{public}s", __func__);
    auto abilityRecord = Token::GetAbilityRecordByToken(token);
    if (!abilityRecord) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "no such ability record");
        return -1;
    }

    auto callingTokenId = IPCSkeleton::GetCallingTokenID();
    auto tokenID = abilityRecord->GetApplicationInfo().accessTokenId;
    if (callingTokenId != tokenID) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "SetMissionLabel not self, not enabled");
        return -1;
    }

    auto userId = abilityRecord->GetOwnerMissionUserId();
    auto missionListManager = GetMissionListManagerByUserId(userId);
    if (!missionListManager) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "failed to find mission list manager when set mission label.");
        return -1;
    }

    return missionListManager->SetMissionLabel(token, label);
}

int AbilityManagerService::SetMissionIcon(const sptr<IRemoteObject> &token,
    const std::shared_ptr<OHOS::Media::PixelMap> &icon)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "%{public}s", __func__);
    CHECK_CALLER_IS_SYSTEM_APP;
    auto abilityRecord = Token::GetAbilityRecordByToken(token);
    if (!abilityRecord) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "no such ability record");
        return -1;
    }

    auto callingTokenId = IPCSkeleton::GetCallingTokenID();
    auto tokenID = abilityRecord->GetApplicationInfo().accessTokenId;
    if (callingTokenId != tokenID) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "not self, not enable to set mission icon");
        return -1;
    }

    auto userId = abilityRecord->GetOwnerMissionUserId();
    auto missionListManager = GetMissionListManagerByUserId(userId);
    if (!missionListManager) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "failed to find mission list manager.");
        return -1;
    }

    return missionListManager->SetMissionIcon(token, icon);
}

int AbilityManagerService::RegisterWindowManagerServiceHandler(const sptr<IWindowManagerServiceHandler> &handler,
    bool animationEnabled)
{
    auto isSaCall = AAFwk::PermissionVerification::GetInstance()->IsSACall();
    if (!isSaCall) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "%{public}s: Permission verification failed", __func__);
        return CHECK_PERMISSION_FAILED;
    }
    wmsHandler_ = handler;
    isAnimationEnabled_ = animationEnabled;
    TAG_LOGD(AAFwkTag::ABILITYMGR, "%{public}s: WMS handler registered successfully.", __func__);
    return ERR_OK;
}

sptr<IWindowManagerServiceHandler> AbilityManagerService::GetWMSHandler() const
{
    return wmsHandler_;
}

void AbilityManagerService::CompleteFirstFrameDrawing(const sptr<IRemoteObject> &abilityToken)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "called");
    if (IPCSkeleton::GetCallingUid() != FOUNDATION_UID) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Not foundation call.");
        return;
    }
    auto abilityRecord = Token::GetAbilityRecordByToken(abilityToken);
    CHECK_POINTER(abilityRecord);

    auto ownerUserId = abilityRecord->GetOwnerMissionUserId();
    auto missionListManager = GetMissionListManagerByUserId(ownerUserId);
    CHECK_POINTER(missionListManager);
    missionListManager->CompleteFirstFrameDrawing(abilityToken);
}

void AbilityManagerService::CompleteFirstFrameDrawing(int32_t sessionId)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "called");
    if (!IsCallerSceneBoard()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Not sceneboard called, not allowed.");
        return;
    }
    auto uiAbilityManager = GetUIAbilityManagerByUid(IPCSkeleton::GetCallingUid());
    CHECK_POINTER(uiAbilityManager);
    uiAbilityManager->CompleteFirstFrameDrawing(sessionId);
}

int32_t AbilityManagerService::ShowPickerDialog(
    const Want& want, int32_t userId, const sptr<IRemoteObject> &callerToken)
{
    AAFwk::Want newWant = want;
    std::string sharePickerBundleName =
        OHOS::system::GetParameter(SHARE_PICKER_DIALOG_BUNDLE_NAME_KEY, SHARE_PICKER_DIALOG_DEFAULY_BUNDLE_NAME);
    std::string sharePickerAbilityName =
        OHOS::system::GetParameter(SHARE_PICKER_DIALOG_ABILITY_NAME_KEY, SHARE_PICKER_DIALOG_DEFAULY_ABILITY_NAME);
    newWant.SetElementName(sharePickerBundleName, sharePickerAbilityName);
    newWant.SetParam(TOKEN_KEY, callerToken);
    // note: clear actions
    newWant.SetAction("");
    return IN_PROCESS_CALL(StartAbility(newWant, DEFAULT_INVAL_VALUE, userId));
}

bool AbilityManagerService::CheckWindowMode(int32_t windowMode,
    const std::vector<AppExecFwk::SupportWindowMode>& windowModes) const
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "Window mode is %{public}d.", windowMode);
    if (windowMode == AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_UNDEFINED) {
        return true;
    }

    auto bmsWindowMode = WindowOptionsUtils::WindowModeMap(windowMode);
    if (bmsWindowMode.first) {
        for (const auto& mode : windowModes) {
            if (mode == bmsWindowMode.second) {
                return true;
            }
        }
    }
    return false;
}

int AbilityManagerService::PrepareTerminateAbility(const sptr<IRemoteObject> &token,
    sptr<IPrepareTerminateCallback> &callback)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call");
    if (callback == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "callback is nullptr.");
        return ERR_INVALID_VALUE;
    }
    if (!CheckPrepareTerminateEnable()) {
        callback->DoPrepareTerminate();
        return ERR_INVALID_VALUE;
    }

    auto abilityRecord = Token::GetAbilityRecordByToken(token);
    if (abilityRecord == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "record is nullptr.");
        callback->DoPrepareTerminate();
        return ERR_INVALID_VALUE;
    }

    if (!JudgeSelfCalled(abilityRecord)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Not self call.");
        callback->DoPrepareTerminate();
        return CHECK_PERMISSION_FAILED;
    }

    auto type = abilityRecord->GetAbilityInfo().type;
    if (type != AppExecFwk::AbilityType::PAGE) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Only support PAGE.");
        callback->DoPrepareTerminate();
        return RESOLVE_CALL_ABILITY_TYPE_ERR;
    }

    auto timeoutTask = [&callback]() {
        callback->DoPrepareTerminate();
    };
    int prepareTerminateTimeout =
        AmsConfigurationParameter::GetInstance().GetAppStartTimeoutTime() * PREPARE_TERMINATE_TIMEOUT_MULTIPLE;
    if (taskHandler_) {
        taskHandler_->SubmitTask(timeoutTask, "PrepareTermiante_" + std::to_string(abilityRecord->GetAbilityRecordId()),
            prepareTerminateTimeout);
    }

    bool res = abilityRecord->PrepareTerminateAbility();
    if (!res) {
        callback->DoPrepareTerminate();
    }
    if (taskHandler_) {
        taskHandler_->CancelTask("PrepareTermiante_" + std::to_string(abilityRecord->GetAbilityRecordId()));
    }
    return ERR_OK;
}

void AbilityManagerService::HandleFocused(const sptr<OHOS::Rosen::FocusChangeInfo> &focusChangeInfo)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "handle focused event");
    auto missionListManager = GetCurrentMissionListManager();
    CHECK_POINTER(missionListManager);

    int32_t missionId = GetMissionIdByAbilityToken(focusChangeInfo->abilityToken_);
    missionListManager->NotifyMissionFocused(missionId);
}

void AbilityManagerService::HandleUnfocused(const sptr<OHOS::Rosen::FocusChangeInfo> &focusChangeInfo)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "handle unfocused event");
    auto missionListManager = GetCurrentMissionListManager();
    CHECK_POINTER(missionListManager);

    int32_t missionId = GetMissionIdByAbilityToken(focusChangeInfo->abilityToken_);
    missionListManager->NotifyMissionUnfocused(missionId);
}

void AbilityManagerService::InitFocusListener()
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "Init ability focus listener");
    if (focusListener_) {
        return;
    }

    focusListener_ = new WindowFocusChangedListener(shared_from_this(), taskHandler_);
    auto registerTask = [innerService = shared_from_this()]() {
        if (innerService) {
            TAG_LOGI(AAFwkTag::ABILITYMGR, "RegisterFocusListener task");
            innerService->RegisterFocusListener();
        }
    };
    if (taskHandler_) {
        taskHandler_->SubmitTask(registerTask, "RegisterFocusListenerTask", REGISTER_FOCUS_DELAY);
    }
}

void AbilityManagerService::RegisterFocusListener()
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "Register focus listener");
    if (!focusListener_) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "no listener obj");
        return;
    }
    Rosen::WindowManager::GetInstance().RegisterFocusChangedListener(focusListener_);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "Register focus listener success");
}

void AbilityManagerService::InitPrepareTerminateConfig()
{
    char value[PREPARE_TERMINATE_ENABLE_SIZE] = "false";
    int retSysParam = GetParameter(PREPARE_TERMINATE_ENABLE_PARAMETER, "false", value, PREPARE_TERMINATE_ENABLE_SIZE);
    TAG_LOGI(AAFwkTag::ABILITYMGR,
        "CheckPrepareTerminateEnable, %{public}s value is %{public}s.", PREPARE_TERMINATE_ENABLE_PARAMETER,
        value);
    if (retSysParam > 0 && !std::strcmp(value, "true")) {
        isPrepareTerminateEnable_ = true;
    }
}

int AbilityManagerService::RegisterAbilityFirstFrameStateObserver(
    const sptr<IAbilityFirstFrameStateObserver> &observer, const std::string &targetBundleName)
{
    return AppExecFwk::AbilityFirstFrameStateObserverManager::GetInstance().
        RegisterAbilityFirstFrameStateObserver(observer, targetBundleName);
}

int AbilityManagerService::UnregisterAbilityFirstFrameStateObserver(
    const sptr<IAbilityFirstFrameStateObserver> &observer)
{
    return AppExecFwk::AbilityFirstFrameStateObserverManager::GetInstance().
        UnregisterAbilityFirstFrameStateObserver(observer);
}

bool AbilityManagerService::GetAnimationFlag()
{
    return isAnimationEnabled_;
}

#endif

int AbilityManagerService::CheckCallServicePermission(const AbilityRequest &abilityRequest)
{
    if (abilityRequest.want.GetIntParam(Want::PARAM_RESV_CALLER_UID, IPCSkeleton::GetCallingUid()) == BROKER_UID &&
        abilityRequest.want.GetElement().GetBundleName() == SHELL_ASSISTANT_BUNDLENAME) {
        auto collaborator = GetCollaborator(CollaboratorType::RESERVE_TYPE);
        if (collaborator != nullptr) {
            TAG_LOGI(AAFwkTag::ABILITYMGR, "Collaborator CheckCallAbilityPermission.");
            return collaborator->CheckCallAbilityPermission(abilityRequest.want);
        }
    }
    if (abilityRequest.abilityInfo.isStageBasedModel) {
        auto extensionType = abilityRequest.abilityInfo.extensionAbilityType;
        TAG_LOGD(AAFwkTag::ABILITYMGR, "extensionType is %{public}d.", static_cast<int>(extensionType));
        if (extensionType == AppExecFwk::ExtensionAbilityType::SERVICE ||
            extensionType == AppExecFwk::ExtensionAbilityType::DATASHARE) {
            return CheckCallServiceExtensionPermission(abilityRequest);
        } else {
            return CheckCallOtherExtensionPermission(abilityRequest);
        }
    } else {
        return CheckCallServiceAbilityPermission(abilityRequest);
    }
}

int AbilityManagerService::CheckCallDataAbilityPermission(AbilityRequest &abilityRequest, bool isShell, bool isSACall)
{
    abilityRequest.appInfo = abilityRequest.abilityInfo.applicationInfo;
    abilityRequest.uid = abilityRequest.appInfo.uid;
    if (abilityRequest.appInfo.name.empty() || abilityRequest.appInfo.bundleName.empty()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Invalid app info for data ability acquiring.");
        return ERR_INVALID_VALUE;
    }
    if (abilityRequest.abilityInfo.type != AppExecFwk::AbilityType::DATA) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "BMS query result is not a data ability.");
        return ERR_WRONG_INTERFACE_CALL;
    }

    AAFwk::PermissionVerification::VerificationInfo verificationInfo = CreateVerificationInfo(abilityRequest,
        true, isShell, isSACall);
    if (isShell) {
        verificationInfo.isBackgroundCall = true;
    }
    if (!isShell && IsCallFromBackground(abilityRequest, verificationInfo.isBackgroundCall, true) != ERR_OK) {
        return ERR_INVALID_VALUE;
    }
    int result = AAFwk::PermissionVerification::GetInstance()->CheckCallDataAbilityPermission(verificationInfo,
        isShell);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Do not have permission to start DataAbility");
        return result;
    }

    return ERR_OK;
}

AAFwk::PermissionVerification::VerificationInfo AbilityManagerService::CreateVerificationInfo(
    const AbilityRequest &abilityRequest, bool isData, bool isShell, bool isSA)
{
    AAFwk::PermissionVerification::VerificationInfo verificationInfo;
    verificationInfo.accessTokenId = abilityRequest.appInfo.accessTokenId;
    verificationInfo.visible = abilityRequest.abilityInfo.visible;
    verificationInfo.withContinuousTask = IsBackgroundTaskUid(IPCSkeleton::GetCallingUid());
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Call ServiceAbility or DataAbility, target bundleName: %{public}s.",
        abilityRequest.appInfo.bundleName.c_str());
    if (whiteListassociatedWakeUpFlag_ &&
        WHITE_LIST_ASS_WAKEUP_SET.find(abilityRequest.appInfo.bundleName) != WHITE_LIST_ASS_WAKEUP_SET.end()) {
        TAG_LOGD(AAFwkTag::ABILITYMGR,
            "Call ServiceAbility or DataAbility, target bundle in white-list, allow associatedWakeUp.");
        verificationInfo.associatedWakeUp = true;
    } else {
        verificationInfo.associatedWakeUp = abilityRequest.appInfo.associatedWakeUp;
    }
    if (!isData) {
        isSA = AAFwk::PermissionVerification::GetInstance()->IsSACall();
        isShell = AAFwk::PermissionVerification::GetInstance()->IsShellCall();
    }
    if (isSA || isShell) {
        return verificationInfo;
    }
    std::shared_ptr<AbilityRecord> callerAbility = Token::GetAbilityRecordByToken(abilityRequest.callerToken);
    if (callerAbility) {
        verificationInfo.apiTargetVersion = callerAbility->GetApplicationInfo().apiTargetVersion;
    }

    return verificationInfo;
}

int AbilityManagerService::CheckCallServiceExtensionPermission(const AbilityRequest &abilityRequest)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "begin");

    AAFwk::PermissionVerification::VerificationInfo verificationInfo;
    verificationInfo.accessTokenId = abilityRequest.appInfo.accessTokenId;
    verificationInfo.visible = abilityRequest.abilityInfo.visible;
    verificationInfo.withContinuousTask = IsBackgroundTaskUid(IPCSkeleton::GetCallingUid());
    verificationInfo.isBackgroundCall = false;
    if (isParamStartAbilityEnable_) {
        bool stopContinuousTaskFlag = ShouldPreventStartAbility(abilityRequest);
        if (stopContinuousTaskFlag) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "Do not have permission to start ServiceExtension");
            return CHECK_PERMISSION_FAILED;
        }
    }

    int result = AAFwk::PermissionVerification::GetInstance()->CheckCallServiceExtensionPermission(verificationInfo);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Do not have permission to start ServiceExtension or DataShareExtension");
    }
    return result;
}

int AbilityManagerService::CheckCallOtherExtensionPermission(const AbilityRequest &abilityRequest)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Call");
    if (IPCSkeleton::GetCallingUid() != BROKER_UID && AAFwk::PermissionVerification::GetInstance()->IsSACall()) {
        return ERR_OK;
    }

    auto extensionType = abilityRequest.abilityInfo.extensionAbilityType;
    TAG_LOGD(AAFwkTag::ABILITYMGR, "OtherExtension type: %{public}d.", static_cast<int32_t>(extensionType));
    if (system::GetBoolParameter(DEVELOPER_MODE_STATE, false) &&
        PermissionVerification::GetInstance()->VerifyShellStartExtensionType(static_cast<int32_t>(extensionType))) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "CheckCallOtherExtensionPermission, allow aa start with debug mode.");
        return ERR_OK;
    }
    if (extensionType == AppExecFwk::ExtensionAbilityType::WINDOW) {
        CHECK_CALLER_IS_SYSTEM_APP;
        return ERR_OK;
    }
    if (extensionType == AppExecFwk::ExtensionAbilityType::ADS_SERVICE) {
        return ERR_OK;
    }
    if (extensionType == AppExecFwk::ExtensionAbilityType::AUTO_FILL_PASSWORD ||
        extensionType == AppExecFwk::ExtensionAbilityType::AUTO_FILL_SMART) {
        if (!abilityRequest.appInfo.isSystemApp) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "The application requesting the call is a non system application.");
            return CHECK_PERMISSION_FAILED;
        }
        std::string jsonDataStr = abilityRequest.want.GetStringParam(WANT_PARAMS_VIEW_DATA_KEY);
        AbilityBase::ViewData viewData;
        viewData.FromJsonString(jsonDataStr.c_str());
        if (!CheckCallingTokenId(viewData.bundleName)) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "Not %{public}s called, not allowed.", viewData.bundleName.c_str());
            return ERR_WRONG_INTERFACE_CALL;
        }
        return ERR_OK;
    }
    if (AAFwk::UIExtensionUtils::IsUIExtension(extensionType)) {
        return CheckUIExtensionPermission(abilityRequest);
    }
    if (extensionType == AppExecFwk::ExtensionAbilityType::VPN ||
        extensionType == AppExecFwk::ExtensionAbilityType::UI_SERVICE) {
        return ERR_OK;
    }

    const std::string fileAccessPermission = "ohos.permission.FILE_ACCESS_MANAGER";
    if (extensionType == AppExecFwk::ExtensionAbilityType::FILEACCESS_EXTENSION &&
        AAFwk::PermissionVerification::GetInstance()->VerifyCallingPermission(fileAccessPermission)) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "Temporary, FILEACCESS_EXTENSION use serviceExtension start-up rule.");
        return CheckCallServiceExtensionPermission(abilityRequest);
    }

    TAG_LOGE(AAFwkTag::ABILITYMGR, "Not SA, can not start other Extension");
    return CHECK_PERMISSION_FAILED;
}

int AbilityManagerService::CheckUIExtensionPermission(const AbilityRequest &abilityRequest)
{
    if (abilityRequest.want.HasParameter(AAFwk::SCREEN_MODE_KEY)) {
        // If started by embedded atomic service, allow it.
        return ERR_OK;
    }

    auto extensionType = abilityRequest.abilityInfo.extensionAbilityType;
    if (AAFwk::UIExtensionUtils::IsSystemUIExtension(extensionType)) {
        auto callerRecord = Token::GetAbilityRecordByToken(abilityRequest.callerToken);
        if (callerRecord == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "Invalid caller.");
            return NO_FOUND_ABILITY_BY_CALLER;
        }

        if (!abilityRequest.appInfo.isSystemApp && !callerRecord->GetApplicationInfo().isSystemApp) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "Bundle %{public}s wanna to start or caller bundle %{public}s "
                "isn't system app, type %{public}d not allowed.", abilityRequest.appInfo.bundleName.c_str(),
                callerRecord->GetApplicationInfo().bundleName.c_str(), extensionType);
            return CHECK_PERMISSION_FAILED;
        }
    }

    if (AAFwk::UIExtensionUtils::IsSystemCallerNeeded(extensionType)) {
        auto callerRecord = Token::GetAbilityRecordByToken(abilityRequest.callerToken);
        if (callerRecord == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "Invalid caller.");
            return NO_FOUND_ABILITY_BY_CALLER;
        }

        if (!callerRecord->GetApplicationInfo().isSystemApp
            && !AAFwk::PermissionVerification::GetInstance()->IsSACall()) {
            TAG_LOGE(AAFwkTag::ABILITYMGR,
                     "Bundle %{public}s wanna to start but caller bundle %{public}s "
                     "isn't system app, type %{public}d not allowed.",
                     abilityRequest.appInfo.bundleName.c_str(), callerRecord->GetApplicationInfo().bundleName.c_str(),
                     extensionType);
            return CHECK_PERMISSION_FAILED;
        }
    }
    return ERR_OK;
}

int AbilityManagerService::CheckCallServiceAbilityPermission(const AbilityRequest &abilityRequest)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Call");
    AAFwk::PermissionVerification::VerificationInfo verificationInfo = CreateVerificationInfo(abilityRequest);
    if (IsCallFromBackground(abilityRequest, verificationInfo.isBackgroundCall) != ERR_OK) {
        return ERR_INVALID_VALUE;
    }

    int result = AAFwk::PermissionVerification::GetInstance()->CheckCallServiceAbilityPermission(verificationInfo);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Do not have permission to start ServiceAbility");
    }
    return result;
}

int AbilityManagerService::CheckCallAbilityPermission(const AbilityRequest &abilityRequest, uint32_t specifyTokenId,
    bool isCallByShortcut)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Call");

    AAFwk::PermissionVerification::VerificationInfo verificationInfo;
    verificationInfo.accessTokenId = abilityRequest.appInfo.accessTokenId;
    verificationInfo.visible = abilityRequest.abilityInfo.visible;
    verificationInfo.withContinuousTask = IsBackgroundTaskUid(IPCSkeleton::GetCallingUid());
    verificationInfo.specifyTokenId = specifyTokenId;
    if (IsCallFromBackground(abilityRequest, verificationInfo.isBackgroundCall) != ERR_OK) {
        return ERR_INVALID_VALUE;
    }

    int result = AAFwk::PermissionVerification::GetInstance()->CheckCallAbilityPermission(
        verificationInfo, isCallByShortcut);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Do not have permission to start PageAbility(FA) or Ability(Stage)");
    }
    return result;
}

int AbilityManagerService::CheckStartByCallPermission(const AbilityRequest &abilityRequest)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "Call");
    // check whether the target ability is page type and not specified mode.
    if (abilityRequest.abilityInfo.type != AppExecFwk::AbilityType::PAGE ||
        abilityRequest.abilityInfo.launchMode == AppExecFwk::LaunchMode::SPECIFIED) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Called ability is not common ability.");
        return RESOLVE_CALL_ABILITY_TYPE_ERR;
    }

    AAFwk::PermissionVerification::VerificationInfo verificationInfo;
    verificationInfo.accessTokenId = abilityRequest.appInfo.accessTokenId;
    verificationInfo.visible = abilityRequest.abilityInfo.visible;
    verificationInfo.withContinuousTask = IsBackgroundTaskUid(IPCSkeleton::GetCallingUid());
    if (IsCallFromBackground(abilityRequest, verificationInfo.isBackgroundCall) != ERR_OK) {
        return ERR_INVALID_VALUE;
    }

    if (AAFwk::PermissionVerification::GetInstance()->CheckStartByCallPermission(verificationInfo) != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Do not have permission to StartAbilityByCall.");
        return RESOLVE_CALL_NO_PERMISSIONS;
    }
    TAG_LOGD(AAFwkTag::ABILITYMGR, "The caller has permission to resolve the call proxy of common ability.");
    return ERR_OK;
}

int AbilityManagerService::IsCallFromBackground(const AbilityRequest &abilityRequest, bool &isBackgroundCall,
    bool isData)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (!isData && AAFwk::PermissionVerification::GetInstance()->IsShellCall()) {
        isBackgroundCall = true;
        return ERR_OK;
    }

    if (!isData && (AAFwk::PermissionVerification::GetInstance()->IsSACall() ||
        AbilityUtil::IsStartFreeInstall(abilityRequest.want))) {
        isBackgroundCall = false;
        return ERR_OK;
    }

    AppExecFwk::RunningProcessInfo processInfo;
    std::shared_ptr<AbilityRecord> callerAbility = Token::GetAbilityRecordByToken(abilityRequest.callerToken);
    if (callerAbility && callerAbility->GetAbilityInfo().bundleName == BUNDLE_NAME_DIALOG) {
        callerAbility = callerAbility->GetCallerRecord();
    }
    if (callerAbility) {
        if (callerAbility->IsForeground() || callerAbility->GetAbilityForegroundingFlag()) {
            isBackgroundCall = false;
            return ERR_OK;
        }
        // CallerAbility is not foreground, so check process state
        DelayedSingleton<AppScheduler>::GetInstance()->
            GetRunningProcessInfoByToken(callerAbility->GetToken(), processInfo);
        if (IsDelegatorCall(processInfo, abilityRequest)) {
            TAG_LOGD(AAFwkTag::ABILITYMGR, "The call is from AbilityDelegator, allow background-call.");
            isBackgroundCall = false;
            return ERR_OK;
        }
        auto abilityState = callerAbility->GetAbilityState();
        if (abilityState == AbilityState::BACKGROUND || abilityState == AbilityState::BACKGROUNDING ||
            // If uiability or uiextensionability ability state is foreground when terminate,
            // it will move to background firstly. So if startAbility in onBackground() lifecycle,
            // the actual ability state may be had changed to terminating from background or backgrounding.
            abilityState == AbilityState::TERMINATING) {
            return ERR_OK;
        }
    } else {
        auto callerPid = IPCSkeleton::GetCallingPid();
        DelayedSingleton<AppScheduler>::GetInstance()->GetRunningProcessInfoByPid(callerPid, processInfo);
        if (processInfo.processName_.empty()) {
            TAG_LOGD(AAFwkTag::ABILITYMGR, "Can not find caller application by callerPid: %{private}d.", callerPid);
            if (AAFwk::PermissionVerification::GetInstance()->VerifyCallingPermission(
                PermissionConstants::PERMISSION_START_ABILITIES_FROM_BACKGROUND)) {
                TAG_LOGD(AAFwkTag::ABILITYMGR, "Caller has PERMISSION_START_ABILITIES_FROM_BACKGROUND, PASS.");
                isBackgroundCall = false;
                return ERR_OK;
            }
            TAG_LOGE(AAFwkTag::ABILITYMGR, "Caller does not have PERMISSION_START_ABILITIES_FROM_BACKGROUND, REJECT.");
            return ERR_INVALID_VALUE;
        }
    }
    return SetBackgroundCall(processInfo, abilityRequest, isBackgroundCall);
}

int32_t AbilityManagerService::SetBackgroundCall(const AppExecFwk::RunningProcessInfo &processInfo,
    const AbilityRequest &abilityRequest, bool &isBackgroundCall) const
{
    if (IsDelegatorCall(processInfo, abilityRequest)) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "The call is from AbilityDelegator, allow background-call.");
        isBackgroundCall = false;
        return ERR_OK;
    }

    if (backgroundJudgeFlag_) {
        isBackgroundCall = processInfo.state_ != AppExecFwk::AppProcessState::APP_STATE_FOREGROUND &&
            !processInfo.isFocused && !processInfo.isAbilityForegrounding;
    } else {
        isBackgroundCall = !processInfo.isFocused;
        if (!processInfo.isFocused && processInfo.state_ == AppExecFwk::AppProcessState::APP_STATE_FOREGROUND) {
            // Allow background startup within 1 second after application startup if state is FOREGROUND
            int64_t aliveTime = AbilityUtil::SystemTimeMillis() - processInfo.startTimeMillis_;
            isBackgroundCall = aliveTime > APP_ALIVE_TIME_MS;
            TAG_LOGD(AAFwkTag::ABILITYMGR, "Process %{public}s is alive %{public}s ms.",
                processInfo.processName_.c_str(), std::to_string(aliveTime).c_str());
        }
    }
    TAG_LOGD(AAFwkTag::ABILITYMGR,
        "backgroundJudgeFlag: %{public}d, isBackgroundCall: %{public}d, callerAppState: %{public}d.",
        static_cast<int32_t>(backgroundJudgeFlag_),
        static_cast<int32_t>(isBackgroundCall),
        static_cast<int32_t>(processInfo.state_));

    return ERR_OK;
}

bool AbilityManagerService::IsTargetPermission(const Want &want) const
{
    if (want.GetElement().GetBundleName() == PERMISSIONMGR_BUNDLE_NAME &&
        want.GetElement().GetAbilityName() == PERMISSIONMGR_ABILITY_NAME) {
        return true;
    }

    return false;
}

inline bool AbilityManagerService::IsDelegatorCall(
    const AppExecFwk::RunningProcessInfo &processInfo, const AbilityRequest &abilityRequest) const
{
    /*  To make sure the AbilityDelegator is not counterfeited
     *   1. The caller-process must be test-process
     *   2. The callerToken must be nullptr
     */
    if (processInfo.isTestProcess &&
        !abilityRequest.callerToken && abilityRequest.want.GetBoolParam(IS_DELEGATOR_CALL, false)) {
        return true;
    }
    return false;
}

bool AbilityManagerService::CheckNewRuleSwitchState(const std::string &param)
{
    char value[NEW_RULE_VALUE_SIZE] = "false";
    int retSysParam = GetParameter(param.c_str(), "false", value, NEW_RULE_VALUE_SIZE);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "CheckNewRuleSwitchState, %{public}s value is %{public}s.", param.c_str(), value);
    if (retSysParam > 0 && !std::strcmp(value, "true")) {
        return true;
    }
    return false;
}

bool AbilityManagerService::GetStartUpNewRuleFlag() const
{
    return startUpNewRule_;
}

void AbilityManagerService::CallRequestDone(const sptr<IRemoteObject> &token, const sptr<IRemoteObject> &callStub)
{
    {
        std::lock_guard<ffrt::mutex> autoLock(abilityTokenLock_);
        callStubTokenMap_[callStub] = token;
    }
    auto abilityRecord = Token::GetAbilityRecordByToken(token);
    CHECK_POINTER(abilityRecord);
    if (!JudgeSelfCalled(abilityRecord)) {
        return;
    }

    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        auto uiAbilityManager = GetCurrentUIAbilityManager();
        CHECK_POINTER(uiAbilityManager);
        uiAbilityManager->CallRequestDone(abilityRecord, callStub);
        return;
    }

    auto missionListManager = GetCurrentMissionListManager();
    CHECK_POINTER(missionListManager);
    missionListManager->CallRequestDone(abilityRecord, callStub);
}

void AbilityManagerService::GetAbilityTokenByCalleeObj(const sptr<IRemoteObject> &callStub, sptr<IRemoteObject> &token)
{
    std::lock_guard<ffrt::mutex> autoLock(abilityTokenLock_);
    auto it = callStubTokenMap_.find(callStub);
    if (it == callStubTokenMap_.end()) {
        token = nullptr;
        return;
    }
    token = callStubTokenMap_[callStub];
}

int AbilityManagerService::AddStartControlParam(Want &want, const sptr<IRemoteObject> &callerToken)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (AAFwk::PermissionVerification::GetInstance()->IsSACall() ||
        AAFwk::PermissionVerification::GetInstance()->IsShellCall()) {
        return ERR_OK;
    }
    auto abilityRecord = Token::GetAbilityRecordByToken(callerToken);
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);
    int32_t apiVersion = abilityRecord->GetApplicationInfo().apiTargetVersion;
    want.SetParam(DMS_API_VERSION, apiVersion);
    bool isCallerBackground = true;
    AppExecFwk::RunningProcessInfo processInfo;
    DelayedSingleton<AppScheduler>::GetInstance()->
        GetRunningProcessInfoByToken(abilityRecord->GetToken(), processInfo);
    if (backgroundJudgeFlag_) {
        isCallerBackground = processInfo.state_ != AppExecFwk::AppProcessState::APP_STATE_FOREGROUND;
    } else {
        isCallerBackground = !processInfo.isFocused;
    }
    want.SetParam(DMS_IS_CALLER_BACKGROUND, isCallerBackground);
    return ERR_OK;
}

#ifdef WITH_DLP
int AbilityManagerService::CheckDlpForExtension(
    const Want &want, const sptr<IRemoteObject> &callerToken,
    int32_t userId, EventInfo &eventInfo, const EventName &eventName)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    // check if form frs
    auto callingUid = IPCSkeleton::GetCallingUid();
    std::string bundleName = want.GetBundle();
    if (callingUid == FOUNDATION_UID && FRS_BUNDLE_NAME == bundleName) {
        return ERR_OK;
    }

    if (!DlpUtils::OtherAppsAccessDlpCheck(callerToken, want) ||
        VerifyAccountPermission(userId) == CHECK_PERMISSION_FAILED ||
        !DlpUtils::DlpAccessOtherAppsCheck(callerToken, want)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "%{public}s: Permission verification failed", __func__);
        eventInfo.errCode = CHECK_PERMISSION_FAILED;
        EventReport::SendExtensionEvent(eventName, HiSysEventType::FAULT, eventInfo);
        return CHECK_PERMISSION_FAILED;
    }
    return ERR_OK;
}
#endif // WITH_DLP

bool AbilityManagerService::JudgeSelfCalled(const std::shared_ptr<AbilityRecord> &abilityRecord)
{
    if (IPCSkeleton::GetCallingPid() == getprocpid()) {
        return true;
    }

    auto callingTokenId = IPCSkeleton::GetCallingTokenID();
    auto tokenID = abilityRecord->GetApplicationInfo().accessTokenId;
    if (callingTokenId != tokenID) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Is not self, not enabled");
        return false;
    }

    return true;
}

bool AbilityManagerService::IsAppSelfCalled(const std::shared_ptr<AbilityRecord> &abilityRecord)
{
    CHECK_POINTER_RETURN_BOOL(abilityRecord);
    auto callingTokenId = IPCSkeleton::GetCallingTokenID();
    auto tokenID = abilityRecord->GetApplicationInfo().accessTokenId;
    if (callingTokenId != tokenID) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "not app self called");
        return false;
    }
    return true;
}

std::shared_ptr<AbilityRecord> AbilityManagerService::GetFocusAbility()
{
#ifdef SUPPORT_SCREEN
    sptr<IRemoteObject> token;
    if (!wmsHandler_) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "wmsHandler_ is nullptr.");
        return nullptr;
    }

    wmsHandler_->GetFocusWindow(token);
    if (!token) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "token is nullptr");
        return nullptr;
    }

    auto abilityRecord = Token::GetAbilityRecordByToken(token);
    if (!abilityRecord) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "abilityRecord is nullptr.");
    }
    return abilityRecord;
#endif

    return nullptr;
}

int AbilityManagerService::CheckUIExtensionIsFocused(uint32_t uiExtensionTokenId, bool& isFocused)
{
    sptr<IRemoteObject> token;
    auto ret = GetTopAbility(token);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "GetTopAbility failed");
        return ret;
    }

    bool focused = false;
    int32_t userId = GetValidUserId(DEFAULT_INVAL_VALUE);
    auto connectManager = GetConnectManagerByUserId(userId);
    if (connectManager) {
        focused = connectManager->IsUIExtensionFocused(uiExtensionTokenId, token)
            || connectManager->IsWindowExtensionFocused(uiExtensionTokenId, token);
    } else {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "connectManager is nullptr, userId: %{public}d", userId);
    }
    if (!focused && userId != U0_USER_ID) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "Check connectManager in user0");
        connectManager = GetConnectManagerByUserId(U0_USER_ID);
        if (connectManager) {
            focused = connectManager->IsUIExtensionFocused(uiExtensionTokenId, token)
                || connectManager->IsWindowExtensionFocused(uiExtensionTokenId, token);
        } else {
            TAG_LOGW(AAFwkTag::ABILITYMGR, "connectManager is nullptr, userId: 0");
        }
    }
    isFocused = focused;
    TAG_LOGD(AAFwkTag::ABILITYMGR, "isFocused: %{public}d", isFocused);
    return ERR_OK;
}

int AbilityManagerService::AddFreeInstallObserver(const sptr<IRemoteObject> &callerToken,
    const sptr<AbilityRuntime::IFreeInstallObserver> &observer)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (freeInstallManager_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "freeInstallManager_ is nullptr.");
        return ERR_INVALID_VALUE;
    }
    return freeInstallManager_->AddFreeInstallObserver(callerToken, observer);
}

int32_t AbilityManagerService::IsValidMissionIds(
    const std::vector<int32_t> &missionIds, std::vector<MissionValidResult> &results)
{
    auto userId = IPCSkeleton::GetCallingUid() / BASE_USER_RANGE;
    auto missionlistMgr = GetMissionListManagerByUserId(userId);
    if (missionlistMgr == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "missionlistMgr is nullptr.");
        return ERR_INVALID_VALUE;
    }

    return missionlistMgr->IsValidMissionIds(missionIds, results);
}

int AbilityManagerService::VerifyPermission(const std::string &permission, int pid, int uid)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "permission=%{public}s, pid=%{public}d, uid=%{public}d",
        permission.c_str(),
        pid,
        uid);
    if (permission.empty()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "VerifyPermission permission invalid");
        return CHECK_PERMISSION_FAILED;
    }

    auto bms = GetBundleManager();
    CHECK_POINTER_AND_RETURN(bms, ERR_INVALID_VALUE);

    std::string bundleName;
    if (IN_PROCESS_CALL(bms->GetNameForUid(uid, bundleName)) != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "VerifyPermission failed to get bundle name by uid");
        return CHECK_PERMISSION_FAILED;
    }

    int account = -1;
    DelayedSingleton<AppExecFwk::OsAccountManagerWrapper>::GetInstance()->GetOsAccountLocalIdFromUid(uid, account);
    AppExecFwk::ApplicationInfo appInfo;
    if (!IN_PROCESS_CALL(bms->GetApplicationInfo(bundleName, AppExecFwk::BundleFlag::GET_BUNDLE_DEFAULT,
        account, appInfo))) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "VerifyPermission failed to get application info");
        return CHECK_PERMISSION_FAILED;
    }

    int32_t ret = Security::AccessToken::AccessTokenKit::VerifyAccessToken(appInfo.accessTokenId, permission, false);
    if (ret != Security::AccessToken::PermissionState::PERMISSION_GRANTED) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "PERMISSION_DENIED");
        return CHECK_PERMISSION_FAILED;
    }

    return ERR_OK;
}

int32_t AbilityManagerService::AcquireShareData(
    const int32_t &missionId, const sptr<IAcquireShareDataCallback> &shareData)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "missionId is %{public}d.", missionId);
    CHECK_CALLER_IS_SYSTEM_APP;
    std::shared_ptr<AbilityRecord> abilityRecord = nullptr;
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        auto uiAbilityManager = GetCurrentUIAbilityManager();
        CHECK_POINTER_AND_RETURN(uiAbilityManager, ERR_INVALID_VALUE);
        abilityRecord = uiAbilityManager->GetAbilityRecordsById(missionId);
    } else {
        auto missionListManager = GetCurrentMissionListManager();
        CHECK_POINTER_AND_RETURN(missionListManager, ERR_INVALID_VALUE);
        abilityRecord = missionListManager->GetAbilityRecordByMissionId(missionId);
    }
    if (!abilityRecord) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "abilityRecord is null.");
        return ERR_INVALID_VALUE;
    }
    uniqueId_ = (uniqueId_ == INT_MAX) ? 0 : (uniqueId_ + 1);
    std::pair<int64_t, const sptr<IAcquireShareDataCallback>> shareDataPair =
        std::make_pair(abilityRecord->GetAbilityRecordId(), shareData);
    iAcquireShareDataMap_.emplace(uniqueId_, shareDataPair);
    abilityRecord->ShareData(uniqueId_);
    return ERR_OK;
}

int32_t AbilityManagerService::ShareDataDone(
    const sptr<IRemoteObject> &token, const int32_t &resultCode, const int32_t &uniqueId, WantParams &wantParam)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "resultCode:%{public}d, uniqueId:%{public}d.", resultCode, uniqueId);
    if (!VerificationAllToken(token)) {
        return ERR_INVALID_VALUE;
    }
    auto abilityRecord = Token::GetAbilityRecordByToken(token);
    CHECK_POINTER_AND_RETURN_LOG(abilityRecord, ERR_INVALID_VALUE, "ability record is nullptr.");
    if (!JudgeSelfCalled(abilityRecord)) {
        return CHECK_PERMISSION_FAILED;
    }
    CHECK_POINTER_AND_RETURN_LOG(eventHandler_, ERR_INVALID_VALUE, "fail to get abilityEventHandler.");
    eventHandler_->RemoveEvent(SHAREDATA_TIMEOUT_MSG, uniqueId);
    return GetShareDataPairAndReturnData(abilityRecord, resultCode, uniqueId, wantParam);
}

int32_t AbilityManagerService::NotifySaveAsResult(const Want &want, int resultCode, int requestCode)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "requestCode is %{public}d.", requestCode);

#ifdef WITH_DLP
    //caller check
    if (!DlpUtils::CheckCallerIsDlpManager(GetBundleManager())) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "caller check failed");
        return CHECK_PERMISSION_FAILED;
    }
#endif // WITH_DLP

    for (const auto &item : startAbilityChain_) {
        if (item.second->GetHandlerName() == StartAbilitySandboxSavefile::handlerName_) {
            auto savefileHandler = (StartAbilitySandboxSavefile*)(item.second.get());
            savefileHandler->HandleResult(want, resultCode, requestCode);
            break;
        }
    }
    return ERR_OK;
}

void AbilityManagerService::SetRootSceneSession(const sptr<IRemoteObject> &rootSceneSession)
{
    if (!IsCallerSceneBoard()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Not sceneboard called, not allowed.");
        return;
    }
    auto uiAbilityManager = GetUIAbilityManagerByUid(IPCSkeleton::GetCallingUid());
    CHECK_POINTER(uiAbilityManager);
    uiAbilityManager->SetRootSceneSession(rootSceneSession);
}

void AbilityManagerService::CallUIAbilityBySCB(const sptr<SessionInfo> &sessionInfo, bool &isColdStart)
{
    if (!IsCallerSceneBoard()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Not sceneboard called, not allowed.");
        return;
    }
    auto uiAbilityManager = GetUIAbilityManagerByUid(IPCSkeleton::GetCallingUid());
    CHECK_POINTER(uiAbilityManager);
    uiAbilityManager->CallUIAbilityBySCB(sessionInfo, isColdStart);
}

int32_t AbilityManagerService::SetSessionManagerService(const sptr<IRemoteObject> &sessionManagerService)
{
    if (!IsCallerSceneBoard()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Not sceneboard called, not allowed.");
        return ERR_WRONG_INTERFACE_CALL;
    }

    TAG_LOGI(AAFwkTag::ABILITYMGR, "Call SetSessionManagerService of WMS.");
    auto ret = Rosen::MockSessionManagerService::GetInstance().SetSessionManagerService(sessionManagerService);
    if (ret) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "Call SetSessionManagerService of WMS.");
        return ERR_OK;
    }
    TAG_LOGE(AAFwkTag::ABILITYMGR, "SMS SetSessionManagerService return false.");
    return ERR_OK;
}

bool AbilityManagerService::CheckPrepareTerminateEnable()
{
    if (!isPrepareTerminateEnable_) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "Only support PC.");
        return false;
    }
    if (!AAFwk::PermissionVerification::GetInstance()->VerifyPrepareTerminatePermission()) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "failed, please apply permission ohos.permission.PREPARE_APP_TERMINATE");
        return false;
    }
    return true;
}

void AbilityManagerService::StartSpecifiedAbilityBySCB(const Want &want)
{
    if (!IsCallerSceneBoard()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Not sceneboard called, not allowed.");
        return;
    }
    auto uiAbilityManager = GetUIAbilityManagerByUid(IPCSkeleton::GetCallingUid());
    CHECK_POINTER(uiAbilityManager);
    uiAbilityManager->StartSpecifiedAbilityBySCB(want);
}

int32_t AbilityManagerService::RegisterIAbilityManagerCollaborator(
    int32_t type, const sptr<IAbilityManagerCollaborator> &impl)
{
    auto isSaCall = AAFwk::PermissionVerification::GetInstance()->IsSACall();
    auto callingUid = IPCSkeleton::GetCallingUid();
    if (!isSaCall || (callingUid != BROKER_UID && callingUid != BROKER_RESERVE_UID)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "The interface only support for broker");
        return CHECK_PERMISSION_FAILED;
    }
    if (!CheckCollaboratorType(type)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "collaborator register failed, invalid type.");
        return ERR_INVALID_VALUE;
    }
    {
        std::lock_guard<ffrt::mutex> autoLock(collaboratorMapLock_);
        collaboratorMap_[type] = impl;
    }
    return ERR_OK;
}

int32_t AbilityManagerService::UnregisterIAbilityManagerCollaborator(int32_t type)
{
    auto isSaCall = AAFwk::PermissionVerification::GetInstance()->IsSACall();
    auto callingUid = IPCSkeleton::GetCallingUid();
    if (!isSaCall || (callingUid != BROKER_UID && callingUid != BROKER_RESERVE_UID)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "The interface only support for broker");
        return CHECK_PERMISSION_FAILED;
    }
    if (!CheckCollaboratorType(type)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "collaborator unregister failed, invalid type.");
        return ERR_INVALID_VALUE;
    }
    {
        std::lock_guard<ffrt::mutex> autoLock(collaboratorMapLock_);
        collaboratorMap_.erase(type);
    }
    return ERR_OK;
}

sptr<IAbilityManagerCollaborator> AbilityManagerService::GetCollaborator(int32_t type)
{
    if (!CheckCollaboratorType(type)) {
        return nullptr;
    }
    {
        std::lock_guard<ffrt::mutex> autoLock(collaboratorMapLock_);
        auto it = collaboratorMap_.find(type);
        if (it != collaboratorMap_.end()) {
            return it->second;
        }
    }
    return nullptr;
}

bool AbilityManagerService::CheckCollaboratorType(int32_t type)
{
    if (type != CollaboratorType::RESERVE_TYPE && type != CollaboratorType::OTHERS_TYPE) {
        return false;
    }
    return true;
}

void AbilityManagerService::GetConnectManagerAndUIExtensionBySessionInfo(const sptr<SessionInfo> &sessionInfo,
    std::shared_ptr<AbilityConnectManager> &connectManager, std::shared_ptr<AbilityRecord> &targetAbility)
{
    targetAbility = nullptr;
    int32_t userId = GetValidUserId(DEFAULT_INVAL_VALUE);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "userId=%{public}d", userId);
    connectManager = GetConnectManagerByUserId(userId);
    if (connectManager) {
        targetAbility = connectManager->GetUIExtensioBySessionInfo(sessionInfo);
    } else {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "connectManager is nullptr, userId: %{public}d", userId);
    }
    if (targetAbility == nullptr && userId != U0_USER_ID) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "try to find UIExtension in user0");
        connectManager = GetConnectManagerByUserId(U0_USER_ID);
        if (connectManager) {
            targetAbility = connectManager->GetUIExtensioBySessionInfo(sessionInfo);
        } else {
            TAG_LOGW(AAFwkTag::ABILITYMGR, "connectManager is nullptr, userId: 0");
        }
    }
}

int32_t AbilityManagerService::RegisterStatusBarDelegate(sptr<AbilityRuntime::IStatusBarDelegate> delegate)
{
    if (!IsCallerSceneBoard()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Not sceneboard called, not allowed.");
        return ERR_WRONG_INTERFACE_CALL;
    }
    auto uiAbilityManager = GetUIAbilityManagerByUid(IPCSkeleton::GetCallingUid());
    CHECK_POINTER_AND_RETURN(uiAbilityManager, ERR_INVALID_VALUE);
    return uiAbilityManager->RegisterStatusBarDelegate(delegate);
}

int32_t AbilityManagerService::KillProcessWithPrepareTerminate(const std::vector<int32_t>& pids)
{
    if (!IsCallerSceneBoard()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Not sceneboard called, not allowed.");
        return ERR_WRONG_INTERFACE_CALL;
    }
    auto uiAbilityManager = GetUIAbilityManagerByUid(IPCSkeleton::GetCallingUid());
    CHECK_POINTER_AND_RETURN(uiAbilityManager, ERR_INVALID_VALUE);
    return uiAbilityManager->KillProcessWithPrepareTerminate(pids);
}

int32_t AbilityManagerService::RegisterAutoStartupSystemCallback(const sptr<IRemoteObject> &callback)
{
    if (abilityAutoStartupService_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "abilityAutoStartupService_ is nullptr.");
        return ERR_NO_INIT;
    }
    return abilityAutoStartupService_->RegisterAutoStartupSystemCallback(callback);
}

int32_t AbilityManagerService::UnregisterAutoStartupSystemCallback(const sptr<IRemoteObject> &callback)
{
    if (abilityAutoStartupService_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "abilityAutoStartupService_ is nullptr.");
        return ERR_NO_INIT;
    }
    return abilityAutoStartupService_->UnregisterAutoStartupSystemCallback(callback);
}

int32_t AbilityManagerService::SetApplicationAutoStartup(const AutoStartupInfo &info)
{
    if (abilityAutoStartupService_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "abilityAutoStartupService_ is nullptr.");
        return ERR_NO_INIT;
    }
    return abilityAutoStartupService_->SetApplicationAutoStartup(info);
}

int32_t AbilityManagerService::CancelApplicationAutoStartup(const AutoStartupInfo &info)
{
    if (abilityAutoStartupService_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "abilityAutoStartupService_ is nullptr.");
        return ERR_NO_INIT;
    }
    return abilityAutoStartupService_->CancelApplicationAutoStartup(info);
}

int32_t AbilityManagerService::QueryAllAutoStartupApplications(std::vector<AutoStartupInfo> &infoList)
{
    if (abilityAutoStartupService_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "abilityAutoStartupService_ is nullptr.");
        return ERR_NO_INIT;
    }
    return abilityAutoStartupService_->QueryAllAutoStartupApplications(infoList, GetUserId());
}

int AbilityManagerService::PrepareTerminateAbilityBySCB(const sptr<SessionInfo> &sessionInfo, bool &isTerminate)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Call.");
    if (sessionInfo == nullptr || sessionInfo->sessionToken == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "sessionInfo is nullptr");
        return ERR_INVALID_VALUE;
    }

    if (!IsCallerSceneBoard()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Not sceneboard called, not allowed.");
        return ERR_WRONG_INTERFACE_CALL;
    }

    auto uiAbilityManager = GetUIAbilityManagerByUid(IPCSkeleton::GetCallingUid());
    CHECK_POINTER_AND_RETURN(uiAbilityManager, ERR_INVALID_VALUE);
    auto abilityRecord = uiAbilityManager->GetUIAbilityRecordBySessionInfo(sessionInfo);
    isTerminate = uiAbilityManager->PrepareTerminateAbility(abilityRecord);

    return ERR_OK;
}

int AbilityManagerService::RegisterSessionHandler(const sptr<IRemoteObject> &object)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "call");
    if (!IsCallerSceneBoard()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Not sceneboard called, not allowed.");
        return ERR_WRONG_INTERFACE_CALL;
    }
    auto uiAbilityManager = GetUIAbilityManagerByUid(IPCSkeleton::GetCallingUid());
    CHECK_POINTER_AND_RETURN(uiAbilityManager, ERR_INVALID_VALUE);
    sptr<ISessionHandler> handler = iface_cast<ISessionHandler>(object);
    uiAbilityManager->SetSessionHandler(handler);
    return ERR_OK;
}

bool AbilityManagerService::CheckUserIdActive(int32_t userId)
{
    std::vector<int32_t> osActiveAccountIds;
    auto ret = DelayedSingleton<AppExecFwk::OsAccountManagerWrapper>::GetInstance()->
        QueryActiveOsAccountIds(osActiveAccountIds);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "QueryActiveOsAccountIds failed.");
        return false;
    }
    if (osActiveAccountIds.empty()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "%{public}s, QueryActiveOsAccountIds is empty, no accounts.", __func__);
        return false;
    }
    auto iter = std::find(osActiveAccountIds.begin(), osActiveAccountIds.end(), userId);
    if (iter == osActiveAccountIds.end()) {
        return false;
    }
    return true;
}

int32_t AbilityManagerService::CheckProcessOptions(const Want &want, const StartOptions &startOptions, int32_t userId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (startOptions.processOptions == nullptr ||
        !ProcessOptions::IsValidProcessMode(startOptions.processOptions->processMode)) {
        return ERR_OK;
    }

    TAG_LOGI(AAFwkTag::ABILITYMGR, "start ability with process options.");
    bool isEnable = AppUtils::GetInstance().IsStartOptionsWithProcessOptions();
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled() || !isEnable) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Not support process options.");
        return ERR_CAPABILITY_NOT_SUPPORT;
    }

    auto element = want.GetElement();
    if (element.GetAbilityName().empty() || want.GetAction().compare(ACTION_CHOOSE) == 0) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Not allow implicit start.");
        return ERR_NOT_ALLOW_IMPLICIT_START;
    }

    int32_t appIndex = 0;
    appIndex = !AbilityRuntime::StartupUtil::GetAppIndex(want, appIndex) ? 0 : appIndex;
    if (!CheckCallingTokenId(element.GetBundleName(), userId, appIndex)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Not self application.");
        return ERR_NOT_SELF_APPLICATION;
    }

    auto uiAbilityManager = GetUIAbilityManagerByUid(IPCSkeleton::GetCallingUid());
    CHECK_POINTER_AND_RETURN(uiAbilityManager, ERR_INVALID_VALUE);

    if (ProcessOptions::IsAttachToStatusBarMode(startOptions.processOptions->processMode) &&
        !uiAbilityManager->IsCallerInStatusBar()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Caller is not in status bar in attch to status bar mode.");
        return ERR_START_OPTIONS_CHECK_FAILED;
    }

    auto abilityRecords = uiAbilityManager->GetAbilityRecordsByName(element);
    if (!abilityRecords.empty() && abilityRecords[0] &&
        abilityRecords[0]->GetAbilityInfo().launchMode != AppExecFwk::LaunchMode::STANDARD) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "If it is not in STANDARD mode, repeated starts are not allowed");
        return ERR_ABILITY_ALREADY_RUNNING;
    }

    return ERR_OK;
}

int32_t AbilityManagerService::RegisterAppDebugListener(sptr<AppExecFwk::IAppDebugListener> listener)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "called");
    if (!AAFwk::PermissionVerification::GetInstance()->IsSACall()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Permission verification failed.");
        return CHECK_PERMISSION_FAILED;
    }
    return DelayedSingleton<AppScheduler>::GetInstance()->RegisterAppDebugListener(listener);
}

int32_t AbilityManagerService::UnregisterAppDebugListener(sptr<AppExecFwk::IAppDebugListener> listener)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "called");
    if (!AAFwk::PermissionVerification::GetInstance()->IsSACall()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Permission verification failed.");
        return CHECK_PERMISSION_FAILED;
    }
    return DelayedSingleton<AppScheduler>::GetInstance()->UnregisterAppDebugListener(listener);
}

std::shared_ptr<AbilityDebugDeal> AbilityManagerService::ConnectInitAbilityDebugDeal()
{
    if (abilityDebugDeal_ != nullptr) {
        return abilityDebugDeal_;
    }

    std::unique_lock<ffrt::mutex> lock(abilityDebugDealLock_);
    if (abilityDebugDeal_ != nullptr) {
        return abilityDebugDeal_;
    }

    TAG_LOGD(AAFwkTag::ABILITYMGR, "Creat ability debug deal object.");
    abilityDebugDeal_ = std::make_shared<AbilityDebugDeal>();
    if (abilityDebugDeal_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Creat ability debug deal object failed.");
        return nullptr;
    }

    abilityDebugDeal_->RegisterAbilityDebugResponse();
    return abilityDebugDeal_;
}

int32_t AbilityManagerService::AttachAppDebug(const std::string &bundleName)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "called");
    if (!system::GetBoolParameter(DEVELOPER_MODE_STATE, false)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Developer Mode is false.");
        return ERR_NOT_DEVELOPER_MODE;
    }

    if (!AAFwk::PermissionVerification::GetInstance()->IsSACall() &&
        !AAFwk::PermissionVerification::GetInstance()->IsShellCall()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Permission verification failed.");
        return CHECK_PERMISSION_FAILED;
    }

    ConnectInitAbilityDebugDeal();
    return DelayedSingleton<AppScheduler>::GetInstance()->AttachAppDebug(bundleName);
}

int32_t AbilityManagerService::DetachAppDebug(const std::string &bundleName)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "called");
    if (!AAFwk::PermissionVerification::GetInstance()->IsSACall() &&
        !AAFwk::PermissionVerification::GetInstance()->IsShellCall()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Permission verification failed.");
        return CHECK_PERMISSION_FAILED;
    }

    return DelayedSingleton<AppScheduler>::GetInstance()->DetachAppDebug(bundleName);
}

int32_t AbilityManagerService::ExecuteIntent(uint64_t key, const sptr<IRemoteObject> &callerToken,
    const InsightIntentExecuteParam &param)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "called");
    auto paramPtr = std::make_shared<InsightIntentExecuteParam>(param);
    int32_t ret = DelayedSingleton<InsightIntentExecuteManager>::GetInstance()->CheckAndUpdateParam(key, callerToken,
        paramPtr);
    if (ret != ERR_OK) {
        return ret;
    }

    Want want;
    ret = InsightIntentExecuteManager::GenerateWant(paramPtr, want);
    if (ret != ERR_OK) {
        return ret;
    }

    switch (param.executeMode_) {
        case AppExecFwk::ExecuteMode::UI_ABILITY_FOREGROUND:
            TAG_LOGD(AAFwkTag::ABILITYMGR, "ExecuteMode UI_ABILITY_FOREGROUND.");
            ret = StartAbilityWithInsightIntent(want);
            break;
        case AppExecFwk::ExecuteMode::UI_ABILITY_BACKGROUND: {
            TAG_LOGD(AAFwkTag::ABILITYMGR, "ExecuteMode UI_ABILITY_BACKGROUND.");
            ret = StartAbilityByCallWithInsightIntent(want, callerToken, param);
            break;
        }
        case AppExecFwk::ExecuteMode::UI_EXTENSION_ABILITY:
            TAG_LOGW(AAFwkTag::ABILITYMGR, "ExecuteMode UI_EXTENSION_ABILITY not supported.");
            ret = ERR_INVALID_OPERATION;
            break;
        case AppExecFwk::ExecuteMode::SERVICE_EXTENSION_ABILITY:
            TAG_LOGD(AAFwkTag::ABILITYMGR, "ExecuteMode SERVICE_EXTENSION_ABILITY.");
            ret = StartExtensionAbilityWithInsightIntent(want, AppExecFwk::ExtensionAbilityType::SERVICE);
            break;
        default:
            TAG_LOGE(AAFwkTag::ABILITYMGR, "Invalid ExecuteMode.");
            ret = ERR_INVALID_OPERATION;
            break;
    }
    if (ret == START_ABILITY_WAITING) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "Top ability is foregrounding. The intent will be queued for execution");
        ret = ERR_OK;
    }
    if (ret != ERR_OK) {
        DelayedSingleton<InsightIntentExecuteManager>::GetInstance()->RemoveExecuteIntent(paramPtr->insightIntentId_);
    }

    TAG_LOGD(AAFwkTag::ABILITYMGR, "ExecuteIntent done, ret: %{public}d.", ret);
    return ret;
}

bool AbilityManagerService::IsAbilityStarted(AbilityRequest &abilityRequest,
    std::shared_ptr<AbilityRecord> &targetRecord, const int32_t oriValidUserId)
{
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "scene board is enable");
        auto uiAbilityManager = GetUIAbilityManagerByUserId(oriValidUserId);
        CHECK_POINTER_AND_RETURN(uiAbilityManager, false);
        return uiAbilityManager->IsAbilityStarted(abilityRequest, targetRecord);
    }

    auto missionListMgr = GetMissionListManagerByUserId(oriValidUserId);
    if (missionListMgr == nullptr) {
        return false;
    }
    return missionListMgr->IsAbilityStarted(abilityRequest, targetRecord);
}

int32_t AbilityManagerService::OnExecuteIntent(AbilityRequest &abilityRequest,
    std::shared_ptr<AbilityRecord> &targetRecord)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "OnExecuteIntent");
    if (targetRecord == nullptr || targetRecord->GetScheduler() == nullptr) {
        return ERR_INVALID_VALUE;
    }
    targetRecord->GetScheduler()->OnExecuteIntent(abilityRequest.want);

    return ERR_OK;
}

int32_t AbilityManagerService::StartAbilityWithInsightIntent(const Want &want, int32_t userId, int requestCode)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    bool startWithAccount = want.GetBoolParam(START_ABILITY_TYPE, false);
    if (startWithAccount || IsCrossUserCall(userId)) {
        (const_cast<Want &>(want)).RemoveParam(START_ABILITY_TYPE);
        CHECK_CALLER_IS_SYSTEM_APP;
    }
    AbilityUtil::RemoveShowModeKey(const_cast<Want &>(want));
    EventInfo eventInfo = BuildEventInfo(want, userId);
    SendAbilityEvent(EventName::START_ABILITY, HiSysEventType::BEHAVIOR, eventInfo);
    int32_t ret = StartAbilityWrap(want, nullptr, requestCode, false, userId);
    if (ret != ERR_OK) {
        eventInfo.errCode = ret;
        SendAbilityEvent(EventName::START_ABILITY_ERROR, HiSysEventType::FAULT, eventInfo);
    }
    return ret;
}

int32_t AbilityManagerService::StartExtensionAbilityWithInsightIntent(const Want &want,
    AppExecFwk::ExtensionAbilityType extensionType)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "called");
    return StartExtensionAbilityInner(want, nullptr, DEFAULT_INVAL_VALUE, extensionType, true);
}

int32_t AbilityManagerService::StartAbilityByCallWithInsightIntent(const Want &want,
    const sptr<IRemoteObject> &callerToken, const InsightIntentExecuteParam &param)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "call StartAbilityByCallWithInsightIntent.");
    sptr<IAbilityConnection> connect = sptr<AbilityBackgroundConnection>::MakeSptr();
    if (connect == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Invalid connect.");
        return ERR_INVALID_VALUE;
    }

    AbilityUtil::RemoveWantKey(const_cast<Want &>(want));
    AbilityRequest abilityRequest;
    abilityRequest.callType = AbilityCallType::CALL_REQUEST_TYPE;
    abilityRequest.callerUid = IPCSkeleton::GetCallingUid();
    abilityRequest.callerToken = callerToken;
    abilityRequest.startSetting = nullptr;
    abilityRequest.want = want;
    abilityRequest.connect = connect;
    int32_t result = GenerateAbilityRequest(want, -1, abilityRequest, callerToken, GetUserId());
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Generate ability request error.");
        return result;
    }
    std::shared_ptr<AbilityRecord> targetRecord;
    int32_t oriValidUserId = GetValidUserId(DEFAULT_INVAL_VALUE);
    auto missionListMgr = GetMissionListManagerByUserId(oriValidUserId);
    if (IsAbilityStarted(abilityRequest, targetRecord, oriValidUserId)) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "ability has already started");
        UpdateCallerInfo(abilityRequest.want, callerToken);
        result = OnExecuteIntent(abilityRequest, targetRecord);
    }  else {
        result = StartAbilityByCall(want, connect, callerToken);
    }

    TAG_LOGI(AAFwkTag::ABILITYMGR, "StartAbilityByCallWithInsightIntent %{public}d", result);
    return result;
}

bool AbilityManagerService::IsAbilityControllerStart(const Want &want)
{
    auto callingUid = IPCSkeleton::GetCallingUid();
    bool isBrokerCall = (callingUid == BROKER_UID || callingUid == BROKER_RESERVE_UID);
    if (isBrokerCall) {
        return IsAbilityControllerStart(want, want.GetBundle());
    }
    TAG_LOGE(AAFwkTag::ABILITYMGR, "The interface only support for broker");
    return true;
}

int32_t AbilityManagerService::ExecuteInsightIntentDone(const sptr<IRemoteObject> &token, uint64_t intentId,
    const InsightIntentExecuteResult &result)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto abilityRecord = Token::GetAbilityRecordByToken(token);
    CHECK_POINTER_AND_RETURN_LOG(abilityRecord, ERR_INVALID_VALUE, "Ability record is nullptr.");
    if (!JudgeSelfCalled(abilityRecord)) {
        return CHECK_PERMISSION_FAILED;
    }

    // check send by same bundleName.
    std::string bundleNameStored = "";
    auto ret = DelayedSingleton<InsightIntentExecuteManager>::GetInstance()->GetBundleName(intentId, bundleNameStored);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Get matched bundleName failed, intentId: %{public}" PRIu64"", intentId);
        return ERR_INVALID_VALUE;
    }

    std::string bundleName = abilityRecord->GetAbilityInfo().bundleName;
    if (bundleNameStored != bundleName) {
        TAG_LOGE(AAFwkTag::ABILITYMGR,
            "BundleName %{public}s and %{public}s mismatch.", bundleName.c_str(), bundleNameStored.c_str());
        return ERR_INVALID_VALUE;
    }

    return DelayedSingleton<InsightIntentExecuteManager>::GetInstance()->ExecuteIntentDone(
        intentId, result.innerErr, result);
}

int32_t AbilityManagerService::SetApplicationAutoStartupByEDM(const AutoStartupInfo &info, bool flag)
{
    if (abilityAutoStartupService_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "abilityAutoStartupService_ is nullptr.");
        return ERR_NO_INIT;
    }
    return abilityAutoStartupService_->SetApplicationAutoStartupByEDM(info, flag);
}

int32_t AbilityManagerService::CancelApplicationAutoStartupByEDM(const AutoStartupInfo &info, bool flag)
{
    if (abilityAutoStartupService_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "abilityAutoStartupService_ is nullptr.");
        return ERR_NO_INIT;
    }
    return abilityAutoStartupService_->CancelApplicationAutoStartupByEDM(info, flag);
}

int32_t AbilityManagerService::GetForegroundUIAbilities(std::vector<AppExecFwk::AbilityStateData> &list)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "called");
    CHECK_CALLER_IS_SYSTEM_APP;
    auto isPerm = AAFwk::PermissionVerification::GetInstance()->VerifyRunningInfoPerm();
    if (!isPerm) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Permission verification failed.");
        return CHECK_PERMISSION_FAILED;
    }

    std::vector<AbilityRunningInfo> abilityRunningInfos;
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        auto uiAbilityManager = GetUIAbilityManagerByUid(IPCSkeleton::GetCallingUid());
        CHECK_POINTER_AND_RETURN(uiAbilityManager, ERR_INVALID_VALUE);
        uiAbilityManager->GetAbilityRunningInfos(abilityRunningInfos, isPerm);
    } else {
        auto missionListManager = GetCurrentMissionListManager();
        CHECK_POINTER_AND_RETURN(missionListManager, ERR_NULL_OBJECT);
        missionListManager->GetAbilityRunningInfos(abilityRunningInfos, isPerm);
    }

    for (auto &info : abilityRunningInfos) {
        if (info.abilityState != AbilityState::FOREGROUND) {
            continue;
        }

        AppExecFwk::AbilityStateData abilityData;
        abilityData.bundleName = info.ability.GetBundleName();
        abilityData.moduleName = info.ability.GetModuleName();
        abilityData.abilityName = info.ability.GetAbilityName();
        abilityData.abilityState = info.abilityState;
        abilityData.pid = info.pid;
        abilityData.uid = info.uid;
        abilityData.abilityType = static_cast<int32_t>(AppExecFwk::AbilityType::PAGE);
        abilityData.appCloneIndex = info.appCloneIndex;
        AppExecFwk::ApplicationInfo appInfo;
        if (!StartAbilityUtils::GetApplicationInfo(abilityData.bundleName, GetUserId(), appInfo)) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "can not get applicationInfo through bundleName");
        } else if (appInfo.bundleType == AppExecFwk::BundleType::ATOMIC_SERVICE) {
            abilityData.isAtomicService = true;
        }
        list.push_back(abilityData);
    }
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Get foreground ui abilities end, list.size = %{public}zu.", list.size());
    return ERR_OK;
}

void AbilityManagerService::NotifyConfigurationChange(const AppExecFwk::Configuration &config, int32_t userId)
{
    auto collaborator = GetCollaborator(CollaboratorType::RESERVE_TYPE);
    if (collaborator == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "collaborator GetCollaborator is nullptr.");
        return;
    }
    collaborator->UpdateConfiguration(config, userId);
}

void AbilityManagerService::NotifyStartResidentProcess(std::vector<AppExecFwk::BundleInfo> &bundleInfos)
{
    DelayedSingleton<ResidentProcessManager>::GetInstance()->StartResidentProcessWithMainElement(bundleInfos);
    if (!bundleInfos.empty()) {
        DelayedSingleton<ResidentProcessManager>::GetInstance()->StartResidentProcess(bundleInfos);
    }
}

void AbilityManagerService::OnAppRemoteDied(const std::vector<sptr<IRemoteObject>> &abilityTokens)
{
    std::shared_ptr<AbilityRecord> abilityRecord;
    for (auto &token : abilityTokens) {
        abilityRecord = Token::GetAbilityRecordByToken(token);
        if (abilityRecord == nullptr) {
            continue;
        }
        TAG_LOGI(AAFwkTag::ABILITYMGR, "App OnRemoteDied, ability is %{public}s, app is %{public}s",
            abilityRecord->GetAbilityInfo().name.c_str(), abilityRecord->GetAbilityInfo().bundleName.c_str());
        abilityRecord->OnProcessDied();
    }
}

int32_t AbilityManagerService::OpenFile(const Uri& uri, uint32_t flag)
{
    auto accessTokenId = IPCSkeleton::GetCallingTokenID();
    if (!IN_PROCESS_CALL(AAFwk::UriPermissionManagerClient::GetInstance().VerifyUriPermission(
        uri, flag, accessTokenId))) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "premission check failed");
        return -1;
    }
    auto collaborator = GetCollaborator(CollaboratorType::RESERVE_TYPE);
    if (collaborator == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "collaborator GetCollaborator is nullptr.");
        return ERR_COLLABORATOR_NOT_REGISTER;
    }
    return collaborator->OpenFile(uri, flag);
}
#ifdef SUPPORT_SCREEN
int AbilityManagerService::GetDialogSessionInfo(const std::string &dialogSessionId,
    sptr<DialogSessionInfo> &dialogSessionInfo)
{
    CHECK_CALLER_IS_SYSTEM_APP;
    dialogSessionInfo = DialogSessionManager::GetInstance().GetDialogSessionInfo(dialogSessionId);
    if (dialogSessionInfo) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "success");
        return ERR_OK;
    }
    TAG_LOGD(AAFwkTag::ABILITYMGR, "fail");
    return INNER_ERR;
}

int AbilityManagerService::SendDialogResult(const Want &want, const std::string &dialogSessionId, bool isAllowed)
{
    CHECK_CALLER_IS_SYSTEM_APP;
    return DialogSessionManager::GetInstance().SendDialogResult(want, dialogSessionId, isAllowed);
}

int AbilityManagerService::CreateCloneSelectorDialog(AbilityRequest &request, int32_t userId,
    const std::string &replaceWantString)
{
    CHECK_POINTER_AND_RETURN(implicitStartProcessor_, ERR_IMPLICIT_START_ABILITY_FAIL);
    return implicitStartProcessor_->ImplicitStartAbility(request, userId, 0, replaceWantString, true);
}
#endif // SUPPORT_SCREEN
void AbilityManagerService::RemoveLauncherDeathRecipient(int32_t userId)
{
    auto connectManager = GetConnectManagerByUserId(userId);
    if (connectManager == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "connectManager is nullptr. userId=%{public}d", userId);
        return;
    }
    connectManager->RemoveLauncherDeathRecipient();
}

int32_t AbilityManagerService::GenerateEmbeddableUIAbilityRequest(
    const Want &want, AbilityRequest &request, const sptr<IRemoteObject> &callerToken, int32_t userId)
{
    int32_t screenMode = want.GetIntParam(AAFwk::SCREEN_MODE_KEY, AAFwk::IDLE_SCREEN_MODE);
    int32_t result = ERR_OK;
    if (screenMode == AAFwk::EMBEDDED_FULL_SCREEN_MODE) {
        result = GenerateAbilityRequest(want, -1, request, callerToken, userId);
        request.abilityInfo.isModuleJson = true;
        request.abilityInfo.isStageBasedModel = true;
        request.abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
        request.abilityInfo.extensionAbilityType = AppExecFwk::ExtensionAbilityType::UI;
        struct timespec time = {0, 0};
        clock_gettime(CLOCK_MONOTONIC, &time);
        int64_t times = static_cast<int64_t>(time.tv_sec);
        request.abilityInfo.process = request.abilityInfo.bundleName + PROCESS_SUFFIX + std::to_string(times);
    } else {
        result = GenerateExtensionAbilityRequest(want, request, callerToken, userId);
    }
    return result;
}

int32_t AbilityManagerService::CheckDebugAssertPermission()
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "called");
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (!system::GetBoolParameter(PRODUCT_ASSERT_FAULT_DIALOG_ENABLED, false)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Product of assert fault dialog is not enabled.");
        return ERR_NOT_SUPPORTED_PRODUCT_TYPE;
    }
    if (!system::GetBoolParameter(DEVELOPER_MODE_STATE, false)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Developer Mode is false.");
        return ERR_NOT_SUPPORTED_PRODUCT_TYPE;
    }

    auto bundleMgr = GetBundleManager();
    if (bundleMgr == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Get bundle manager instance is nullptr.");
        return ERR_INVALID_VALUE;
    }
    int32_t flags = static_cast<int32_t>(AppExecFwk::GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_APPLICATION);
    AppExecFwk::BundleInfo bundleInfo;
    auto ret = bundleMgr->GetBundleInfoForSelf(flags, bundleInfo);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Get bundle Info failed.");
        return ret;
    }
    if (!bundleInfo.applicationInfo.debug) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Non-debug version application.");
        return ERR_INVALID_VALUE;
    }
    return ERR_OK;
}

void AbilityManagerService::CloseAssertDialog(const std::string &assertSessionId)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Close assert fault dialog begin.");
    auto validUserId = GetUserId();
    auto connectManager = GetConnectManagerByUserId(validUserId);
    if (connectManager == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Connect manager is nullptr, userId: %{public}d.", validUserId);
        return;
    }

    connectManager->CloseAssertDialog(assertSessionId);
}

int32_t AbilityManagerService::SetResidentProcessEnabled(const std::string &bundleName, bool enable)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "called");
    if (!AAFwk::PermissionVerification::GetInstance()->IsSystemAppCall()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Permission verification failed.");
        return ERR_NOT_SYSTEM_APP;
    }

    auto residentProcessManager = DelayedSingleton<ResidentProcessManager>::GetInstance();
    if (residentProcessManager == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Get resident proces mgr is nullptr");
        return INNER_ERR;
    }

    std::string callerName;
    int32_t uid = 0;
    auto callerPid = IPCSkeleton::GetCallingPid();
    DelayedSingleton<AppScheduler>::GetInstance()->GetBundleNameByPid(callerPid, callerName, uid);
    if (callerName.empty()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Failed to obtain caller name.");
        return INNER_ERR;
    }

    return residentProcessManager->SetResidentProcessEnabled(bundleName, callerName, enable);
}

int32_t AbilityManagerService::RequestAssertFaultDialog(
    const sptr<IRemoteObject> &callback, const AAFwk::WantParams &wantParams)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Request to display assert fault dialog begin.");
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto checkRet = CheckDebugAssertPermission();
    if (checkRet != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Check debug assert permission error.");
        return checkRet;
    }
    sptr<IRemoteObject> remoteCallback = callback;
    if (remoteCallback == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Params remote callback is nullptr");
        return ERR_INVALID_VALUE;
    }
    auto debugDeal = ConnectInitAbilityDebugDeal();
    Want want;
#ifdef SUPPORT_SCREEN
    auto sysDialog = DelayedSingleton<SystemDialogScheduler>::GetInstance();
    if (sysDialog == nullptr || debugDeal == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "sysDialog or debugDeal is nullptr.");
        return ERR_INVALID_VALUE;
    }
    if (!sysDialog->GetAssertFaultDialogWant(want)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Get assert fault dialog want failed.");
        return ERR_INVALID_VALUE;
    }
#endif // SUPPORT_SCREEN
    uint64_t assertFaultSessionId = reinterpret_cast<uint64_t>(remoteCallback.GetRefPtr());
    want.SetParam(Want::PARAM_ASSERT_FAULT_SESSION_ID, std::to_string(assertFaultSessionId));
    want.SetParam(ASSERT_FAULT_DETAIL, wantParams.GetStringParam(ASSERT_FAULT_DETAIL));
    auto &connection = AbilityRuntime::ModalSystemAssertUIExtension::GetInstance();
    want.SetParam(UIEXTENSION_MODAL_TYPE, 1);
    if (!IN_PROCESS_CALL(connection.CreateModalUIExtension(want))) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Create modal ui extension failed.");
        return ERR_INVALID_VALUE;
    }
    auto callbackDeathMgr = DelayedSingleton<AbilityRuntime::AssertFaultCallbackDeathMgr>::GetInstance();
    if (callbackDeathMgr == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Get callback death manager instance is nullptr.");
        return ERR_INVALID_VALUE;
    }
    auto callbackTask = [weak = weak_from_this()] (const std::string &assertSessionId) {
        auto abilityMgr = weak.lock();
        if (abilityMgr == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "ability manager instance is nullptr.");
            return;
        }
        abilityMgr->CloseAssertDialog(assertSessionId);
    };
    callbackDeathMgr->AddAssertFaultCallback(remoteCallback, callbackTask);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Request to display assert fault dialog end.");
    return ERR_OK;
}

int32_t AbilityManagerService::NotifyDebugAssertResult(uint64_t assertFaultSessionId, AAFwk::UserStatus userStatus)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (!system::GetBoolParameter(PRODUCT_ASSERT_FAULT_DIALOG_ENABLED, false)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Product of assert fault dialog is not enabled.");
        return ERR_NOT_SUPPORTED_PRODUCT_TYPE;
    }

    CHECK_CALLER_IS_SYSTEM_APP;
    auto permissionSA = PermissionVerification::GetInstance();
    if (permissionSA == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Permission verification instance is nullptr.");
        return ERR_INVALID_VALUE;
    }
    if (!permissionSA->VerifyCallingPermission(PermissionConstants::PERMISSION_NOTIFY_DEBUG_ASSERT_RESULT)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Permission %{public}s verification failed.",
            PermissionConstants::PERMISSION_NOTIFY_DEBUG_ASSERT_RESULT);
        return ERR_PERMISSION_DENIED;
    }

    auto callbackDeathMgr = DelayedSingleton<AbilityRuntime::AssertFaultCallbackDeathMgr>::GetInstance();
    if (callbackDeathMgr == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Get callback death manager instance is nullptr.");
        return ERR_INVALID_VALUE;
    }
    callbackDeathMgr->CallAssertFaultCallback(assertFaultSessionId, userStatus);
    return ERR_OK;
}

int32_t AbilityManagerService::UpdateSessionInfoBySCB(std::list<SessionInfo> &sessionInfos, int32_t userId,
    std::vector<int32_t> &sessionIds)
{
    if (!IsCallerSceneBoard()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Not sceneboard called, not allowed.");
        return ERR_WRONG_INTERFACE_CALL;
    }
    TAG_LOGI(AAFwkTag::ABILITYMGR, "The sceneboard is being restored.");
    auto uiAbilityManager = GetUIAbilityManagerByUid(IPCSkeleton::GetCallingUid());
    CHECK_POINTER_AND_RETURN(uiAbilityManager, ERR_INVALID_VALUE);
    return uiAbilityManager->UpdateSessionInfoBySCB(sessionInfos, sessionIds);
}

bool AbilityManagerService::CheckSenderWantInfo(int32_t callerUid, const WantSenderInfo &wantSenderInfo)
{
    if (!AAFwk::PermissionVerification::GetInstance()->IsSACall()) {
        auto bms = GetBundleManager();
        CHECK_POINTER_AND_RETURN(bms, false);

        std::string bundleName;
        if (IN_PROCESS_CALL(bms->GetNameForUid(callerUid, bundleName)) != ERR_OK) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "Get Bundle Name failed.");
            return false;
        }
        if (wantSenderInfo.bundleName != bundleName) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "wantSender bundleName check failed");
            return false;
        }
    }
    return true;
}

bool AbilityManagerService::CheckCallerIsDmsProcess()
{
    Security::AccessToken::NativeTokenInfo nativeTokenInfo;
    uint32_t accessToken = IPCSkeleton::GetCallingTokenID();
    auto tokenType = Security::AccessToken::AccessTokenKit::GetTokenTypeFlag(accessToken);
    int32_t result = Security::AccessToken::AccessTokenKit::GetNativeTokenInfo(accessToken, nativeTokenInfo);
    if (tokenType != Security::AccessToken::ATokenTypeEnum::TOKEN_NATIVE ||
        result != ERR_OK || nativeTokenInfo.processName != DMS_PROCESS_NAME) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "caller is not dms");
        return false;
    }
    return true;
}

void AbilityManagerService::WaitBootAnimationStart()
{
    char value[BOOTEVENT_BOOT_ANIMATION_READY_SIZE] = "";
    int32_t ret = GetParameter(BOOTEVENT_BOOT_ANIMATION_READY, "", value,
        BOOTEVENT_BOOT_ANIMATION_READY_SIZE);
    if (ret > 0 && !std::strcmp(value, "false")) {
        // Get new param success and new param is not ready, wait the new param.
        WaitParameter(BOOTEVENT_BOOT_ANIMATION_READY, "true",
            AmsConfigurationParameter::GetInstance().GetBootAnimationTimeoutTime());
    } else if (ret <= 0 || !std::strcmp(value, "")) {
        // Get new param failed or new param is not set, wait the old param.
        WaitParameter(BOOTEVENT_BOOT_ANIMATION_STARTED, "true",
            AmsConfigurationParameter::GetInstance().GetBootAnimationTimeoutTime());
    }
    // other, the animation is ready, not wait.
}

int32_t AbilityManagerService::GetUIExtensionRootHostInfo(const sptr<IRemoteObject> token,
    UIExtensionHostInfo &hostInfo, int32_t userId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Get ui extension host info.");
    CHECK_POINTER_AND_RETURN(token, ERR_INVALID_VALUE);

    if (!AAFwk::PermissionVerification::GetInstance()->IsSACall() && !IsCallerSceneBoard()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Permission deny.");
        return ERR_PERMISSION_DENIED;
    }

    auto validUserId = GetValidUserId(userId);
    auto connectManager = GetConnectManagerByUserId(validUserId);
    if (connectManager == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Connect manager is nullptr, userId: %{public}d.", validUserId);
        return ERR_INVALID_VALUE;
    }

    auto callerRecord = connectManager->GetUIExtensionRootHostInfo(token);
    if (callerRecord == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Get root host info failed.");
        return ERR_INVALID_VALUE;
    }

    hostInfo.elementName_ = callerRecord->GetElementName();
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Root host uri: %{public}s.", hostInfo.elementName_.GetURI().c_str());
    return ERR_OK;
}

int32_t AbilityManagerService::GetUIExtensionSessionInfo(const sptr<IRemoteObject> token,
    UIExtensionSessionInfo &uiExtensionSessionInfo, int32_t userId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Get ui extension host info.");
    CHECK_POINTER_AND_RETURN(token, ERR_INVALID_VALUE);

    if (!AAFwk::PermissionVerification::GetInstance()->IsSACall() && !IsCallerSceneBoard()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Permission deny.");
        return ERR_PERMISSION_DENIED;
    }

    auto validUserId = GetValidUserId(userId);
    auto connectManager = GetConnectManagerByUserId(validUserId);
    if (connectManager == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Connect manager is nullptr, userId: %{public}d.", validUserId);
        return ERR_INVALID_VALUE;
    }

    auto ret = connectManager->GetUIExtensionSessionInfo(token, uiExtensionSessionInfo);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Get ui extension session info failed.");
        return ret;
    }

    return ERR_OK;
}

int32_t AbilityManagerService::RestartApp(const AAFwk::Want &want)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call.");
    int result = CheckRestartAppWant(want);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "CheckRestartAppWant error.");
        return result;
    }

    std::string bundleName = want.GetElement().GetBundleName();
    int32_t userId = IPCSkeleton::GetCallingUid() / BASE_USER_RANGE;
    int64_t now = time(nullptr);
    RestartAppKeyType key(bundleName, userId);
    if (RestartAppManager::GetInstance().IsRestartAppFrequent(key, now)) {
        return AAFwk::ERR_RESTART_APP_FREQUENT;
    }

    bool isForegroundToRestartApp = RestartAppManager::GetInstance().IsForegroundToRestartApp();
    if (!isForegroundToRestartApp) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "RestartApp, IsForegroundToRestartApp failed.");
        return AAFwk::NOT_TOP_ABILITY;
    }

    result = SignRestartAppFlag(userId, bundleName);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "SignRestartAppFlag error.");
        return result;
    }
    result = DelayedSingleton<AppExecFwk::AppMgrClient>::GetInstance()->KillApplicationSelf();
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "KillApplicationSelf error.");
        return result;
    }

    TAG_LOGD(AAFwkTag::ABILITYMGR, "RestartApp, start ability without CheckCallAbilityPermission.");
    result = StartAbilityWrap(want, nullptr,
        DEFAULT_INVAL_VALUE, false, DEFAULT_INVAL_VALUE, false, 0, isForegroundToRestartApp);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "StartAbility error.");
        return result;
    }
    RestartAppManager::GetInstance().AddRestartAppHistory(key, now);
    return result;
}

int32_t AbilityManagerService::CheckRestartAppWant(const AAFwk::Want &want)
{
    std::string bundleName = want.GetElement().GetBundleName();
    if (!CheckCallingTokenId(bundleName)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Not itself called, not allowed.");
        return AAFwk::ERR_RESTART_APP_INCORRECT_ABILITY;
    }

    auto bms = GetBundleManager();
    CHECK_POINTER_AND_RETURN(bms, GET_ABILITY_SERVICE_FAILED);
    auto abilityInfoFlag = AbilityRuntime::StartupUtil::BuildAbilityInfoFlag();
    auto userId = IPCSkeleton::GetCallingUid() / BASE_USER_RANGE;
    AppExecFwk::AbilityInfo abilityInfo;
    bool queryResult = IN_PROCESS_CALL(bms->QueryAbilityInfo(want, abilityInfoFlag, userId, abilityInfo));
    if (!queryResult || abilityInfo.name.empty() ||
        abilityInfo.bundleName.empty() || abilityInfo.type != AbilityType::PAGE) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Ability is invalid or not UIAbility.");
        return AAFwk::ERR_RESTART_APP_INCORRECT_ABILITY;
    }
    return ERR_OK;
}

int32_t AbilityManagerService::SignRestartAppFlag(int32_t userId, const std::string &bundleName)
{
    auto appMgr = GetAppMgr();
    if (appMgr == nullptr) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "GetAppMgr failed");
        return ERR_INVALID_VALUE;
    }
    auto ret = IN_PROCESS_CALL(appMgr->SignRestartAppFlag(bundleName));
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "AppMgr SignRestartAppFlag error");
        return ret;
    }

    auto connectManager = GetConnectManagerByUserId(userId);
    connectManager->SignRestartAppFlag(bundleName);
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        auto uiAbilityManager = GetUIAbilityManagerByUserId(userId);
        CHECK_POINTER_AND_RETURN(uiAbilityManager, ERR_INVALID_VALUE);
        uiAbilityManager->SignRestartAppFlag(bundleName);
        return ERR_OK;
    }
    auto missionListManager = GetMissionListManagerByUserId(userId);
    if (missionListManager == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "missionListManager is nullptr. userId:%{public}d", userId);
        return ERR_INVALID_VALUE;
    }
    missionListManager->SignRestartAppFlag(bundleName);
    return ERR_OK;
}

bool AbilityManagerService::IsEmbeddedOpenAllowed(sptr<IRemoteObject> callerToken, const std::string &appId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (!AppUtils::GetInstance().IsLaunchEmbededUIAbility()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "device type is not allowd.");
        return false;
    }
    auto accessTokenId = IPCSkeleton::GetCallingTokenID();
    auto type = Security::AccessToken::AccessTokenKit::GetTokenTypeFlag(accessTokenId);
    if (type != Security::AccessToken::TypeATokenTypeEnum::TOKEN_HAP) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "The caller is not hap.");
        return false;
    }
    auto uiAbilityManager = GetUIAbilityManagerByUid(IPCSkeleton::GetCallingUid());
    CHECK_POINTER_AND_RETURN(uiAbilityManager, ERR_INVALID_VALUE);
    auto callerAbility = uiAbilityManager->GetAbilityRecordByToken(callerToken);
    if (callerAbility == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "The caller is invalid.");
        return false;
    }
    if (callerAbility->GetApplicationInfo().accessTokenId != accessTokenId) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "The callerToken does not belong to the caller.");
        return false;
    }
    if (!callerAbility->IsForeground() && !callerAbility->GetAbilityForegroundingFlag()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "The caller not foreground.");
        return false;
    }
    std::string bundleName = ATOMIC_SERVICE_PREFIX + appId;
    Want want;
    want.SetBundle(bundleName);
    want.SetParam("send_to_erms_embedded", 1);
    int32_t ret = freeInstallManager_->StartFreeInstall(want, GetUserId(), 0, callerToken, false);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "The target is not allowed to free install.");
        return false;
    }
    want.SetParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME, callerAbility->GetElementName().GetBundleName());
    auto erms = std::make_shared<EcologicalRuleInterceptor>();
    return erms->DoProcess(want, GetUserId());
}

bool AbilityManagerService::ShouldPreventStartAbility(const AbilityRequest &abilityRequest)
{
    std::shared_ptr<AbilityRecord> abilityRecord = Token::GetAbilityRecordByToken(abilityRequest.callerToken);
    if (abilityRecord == nullptr) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "No matched token pass");
        return false;
    }
    auto abilityInfo = abilityRequest.abilityInfo;
    auto callerAbilityInfo = abilityRecord->GetAbilityInfo();
    PrintStartAbilityInfo(callerAbilityInfo, abilityInfo);
    if (abilityRecord->GetApplicationInfo().apiTargetVersion % API_VERSION_MOD < API12) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "API version %{public}d pass",
            abilityRecord->GetApplicationInfo().apiTargetVersion % API_VERSION_MOD);
        return false;
    }
    bool continuousFlag = IsBackgroundTaskUid(IPCSkeleton::GetCallingUid());
    if(!IN_PROCESS_CALL(DelayedSingleton<AppScheduler>::GetInstance()->
        IsProcessContainsOnlyUIAbility(abilityRecord->GetPid()))) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "Process has other extension except UIAbility, pass");
        return false;
    }
    if (abilityInfo.extensionAbilityType != AppExecFwk::ExtensionAbilityType::DATASHARE &&
        abilityInfo.extensionAbilityType != AppExecFwk::ExtensionAbilityType::SERVICE) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "Process did not call service or datashare extension Pass");
        return false;
    }
    if (abilityInfo.applicationInfo.uid == IPCSkeleton::GetCallingUid()) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "Process is in same bundle Pass");
        return false;
    }
    if (callerAbilityInfo.type != AppExecFwk::AbilityType::PAGE) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "Is not UI Ability Pass");
        return false;
    }
    if (abilityRecord->GetAbilityState() != AAFwk::AbilityState::BACKGROUND) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "Process is not on background Pass");
        return false;
    }
    if (continuousFlag) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "Process has continuous task Pass");
        return false;
    }
    if (IsInWhiteList(callerAbilityInfo.bundleName, abilityInfo.bundleName, abilityInfo.name)) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "Process is in white list Pass");
        return false;
    }
    TAG_LOGE(AAFwkTag::ABILITYMGR, "Do not have permission to start ServiceExtension %{public}s.",
        abilityRecord->GetURI().c_str());
    ReportPreventStartAbilityResult(callerAbilityInfo, abilityInfo);
    return true;
}

void AbilityManagerService::PrintStartAbilityInfo(AppExecFwk::AbilityInfo callerInfo, AppExecFwk::AbilityInfo calledInfo)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "calledAbilityInfo toString: "
        "calledUid is: %{public}d, "
        "name is: %{public}s, "
        "bundleName is: %{public}s, "
        "type is: %{public}d, "
        "extensionAbilityType is: %{public}d, "
        "moduleName is: %{public}s, "
        "applicationName is: %{public}s",
        calledInfo.applicationInfo.uid,
        calledInfo.name.c_str(),
        calledInfo.bundleName.c_str(),
        static_cast<int32_t>(calledInfo.type),
        static_cast<int32_t>(calledInfo.extensionAbilityType),
        calledInfo.moduleName.c_str(),
        calledInfo.applicationName.c_str());


    TAG_LOGD(AAFwkTag::ABILITYMGR, "callerAbilityInfo toString: "
        "callerUid is: %{public}d, "
        "callerPid is: %{public}d, "
        "name is: %{public}s, "
        "bundleName is: %{public}s, "
        "type is: %{public}d, "
        "extensionAbilityType is: %{public}d, "
        "moduleName is: %{public}s, "
        "applicationName is: %{public}s",
        IPCSkeleton::GetCallingUid(),
        IPCSkeleton::GetCallingPid(),
        callerInfo.name.c_str(),
        callerInfo.bundleName.c_str(),
        static_cast<int32_t>(callerInfo.type),
        static_cast<int32_t>(callerInfo.extensionAbilityType),
        callerInfo.moduleName.c_str(),
        callerInfo.applicationName.c_str());
}

void AbilityManagerService::ReportPreventStartAbilityResult(const AppExecFwk::AbilityInfo &callerAbilityInfo,
    const AppExecFwk::AbilityInfo &abilityInfo)
{
    int32_t callerUid = IPCSkeleton::GetCallingUid();
    int32_t callerPid = IPCSkeleton::GetCallingPid();
    int32_t extensionAbilityType = static_cast<int32_t>(abilityInfo.extensionAbilityType);
    TAG_LOGD(AAFwkTag::ABILITYMGR,
        "Prevent start ability debug log CALLER_BUNDLE_NAME %{public}s CALLEE_BUNDLE_NAME"
        "%{public}s ABILITY_NAME %{public}s",
        callerAbilityInfo.bundleName.c_str(), abilityInfo.name.c_str(), abilityInfo.name.c_str());
    HiSysEventWrite(HiSysEvent::Domain::AAFWK, "PREVENT_START_ABILITY", HiSysEvent::EventType::BEHAVIOR,
        "CALLER_UID", callerUid,
        "CALLER_PID", callerPid,
        "CALLER_PROCESS_NAME", callerAbilityInfo.process,
        "CALLER_BUNDLE_NAME", callerAbilityInfo.bundleName,
        "CALLEE_BUNDLE_NAME", abilityInfo.bundleName,
        "CALLEE_PROCESS_NAME", abilityInfo.process,
        "EXTENSION_ABILITY_TYPE", extensionAbilityType,
        "ABILITY_NAME", abilityInfo.name);
}

bool AbilityManagerService::IsInWhiteList(const std::string &callerBundleName, const std::string &calleeBundleName,
    const std::string &calleeAbilityName)
{
    std::map<std::string, std::list<std::string>>::iterator iter = whiteListMap_.find(callerBundleName);
    std::string uri = calleeBundleName + "/" + calleeAbilityName;
    if (iter != whiteListMap_.end()) {
        if (std::find(std::begin(iter->second), std::end(iter->second), uri) != std::end(iter->second)) {
            return true;
        }
    }
    std::list<std::string>::iterator it = std::find(exportWhiteList_.begin(), exportWhiteList_.end(), uri);
    if (it != exportWhiteList_.end()) {
        return true;
    }
    return false;
}

bool AbilityManagerService::ParseJsonFromBoot(nlohmann::json jsonObj, const std::string &relativePath,
    const std::string &WHITE_LIST)
{
    std::string absolutePath = GetConfigFileAbsolutePath(relativePath);
    if (ParseJsonValueFromFile(jsonObj, absolutePath) != ERR_OK) {
        return false;
    }
    nlohmann::json whiteListJsonList = jsonObj[WHITE_LIST];
    for (const auto& [key, value] : whiteListJsonList.items()) {
        if (!value.is_array()) {
            continue;
        }
        whiteListMap_.emplace(key, std::list<std::string>());
        for (const auto& it : value) {
            if (it.is_string()) {
                whiteListMap_[key].push_back(it);
            }
        }
    }
    if (!jsonObj.contains("exposed_white_list")) {
        return false;
    }
    nlohmann::json exportWhiteJsonList = jsonObj["exposed_white_list"];
    for (const auto& it : exportWhiteJsonList) {
        if (it.is_string()) {
            exportWhiteList_.push_back(it);
        }
    }
    return true;
}

std::string AbilityManagerService::GetConfigFileAbsolutePath(const std::string &relativePath)
{
    if (relativePath.empty()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "relativePath is empty");
        return "";
    }
    char buf[PATH_MAX];
    char *tmpPath = GetOneCfgFile(relativePath.c_str(), buf, PATH_MAX);
    char absolutePath[PATH_MAX] = {0};
    if (!tmpPath || strlen(tmpPath) > PATH_MAX || !realpath(tmpPath, absolutePath)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "get file fail.");
        return "";
    }
    return std::string(absolutePath);
}

int32_t AbilityManagerService::ParseJsonValueFromFile(nlohmann::json &value, const std::string &filePath)
{
    std::ifstream fin;
    std::string realPath;
    if (!ConvertFullPath(filePath, realPath)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Get real path failed");
        return ERR_INVALID_VALUE;
    }
    fin.open(realPath, std::ios::in);
    if (!fin.is_open()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "cannot open file %{private}s", realPath.c_str());
        return ERR_INVALID_VALUE;
    }
    char buffer[MAX_BUFFER];
    std::ostringstream os;
    while (fin.getline(buffer, MAX_BUFFER)) {
        os << buffer;
    }
    const std::string data = os.str();
    value = nlohmann::json::parse(data, nullptr, false);
    if (value.is_discarded()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "failed due to data is discarded");
        return ERR_INVALID_VALUE;
    }
    return ERR_OK;
}

bool AbilityManagerService::ConvertFullPath(const std::string& partialPath, std::string& fullPath)
{
    if (partialPath.empty() || partialPath.length() >= PATH_MAX) {
        return false;
    }
    char tmpPath[PATH_MAX] = {0};
    if (realpath(partialPath.c_str(), tmpPath) == nullptr) {
        return false;
    }
    fullPath = tmpPath;
    return true;
}

int32_t AbilityManagerService::StartShortcut(const Want &want, const StartOptions &startOptions)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (!PermissionVerification::GetInstance()->IsSystemAppCall()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "non-system app calling system api.");
        return ERR_NOT_SYSTEM_APP;
    }
    if (!PermissionVerification::GetInstance()->VerifyCallingPermission(
        PermissionConstants::PERMISSION_START_SHORTCUT)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Permission %{public}s verification failed.",
            PermissionConstants::PERMISSION_START_SHORTCUT);
        return ERR_PERMISSION_DENIED;
    }
    AbilityUtil::RemoveShowModeKey(const_cast<Want &>(want));
    return StartUIAbilityForOptionWrap(want, startOptions, nullptr, false, DEFAULT_INVAL_VALUE, DEFAULT_INVAL_VALUE,
        0, false, true);
}

int32_t AbilityManagerService::GetAbilityStateByPersistentId(int32_t persistentId, bool &state)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (!CheckCallerIsDmsProcess()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "GetAbilityStateByPersistentId, caller is not dms.");
        return ERR_PERMISSION_DENIED;
    }

    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        auto uiAbilityManager = GetUIAbilityManagerByUid(IPCSkeleton::GetCallingUid());
        CHECK_POINTER_AND_RETURN(uiAbilityManager, ERR_INVALID_VALUE);
        return uiAbilityManager->GetAbilityStateByPersistentId(persistentId, state);
    }
    TAG_LOGE(AAFwkTag::ABILITYMGR, "GetAbilityStateByPersistentId, mission not have persistent id.");
    return INNER_ERR;
}

int32_t AbilityManagerService::TransferAbilityResultForExtension(const sptr<IRemoteObject> &callerToken,
    int32_t resultCode, const Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto abilityRecord = Token::GetAbilityRecordByToken(callerToken);
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);
    if (!JudgeSelfCalled(abilityRecord)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Caller mismatch.");
        return ERR_INVALID_VALUE;
    }
    auto type = abilityRecord->GetAbilityInfo().type;
    if (type != AppExecFwk::AbilityType::EXTENSION) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "type is not uiextension.");
        return ERR_INVALID_VALUE;
    }
    // save result to caller AbilityRecord.
    Want* newWant = const_cast<Want*>(&want);
    newWant->RemoveParam(Want::PARAM_RESV_CALLER_TOKEN);
    abilityRecord->SaveResultToCallers(resultCode, newWant);
    abilityRecord->SendResultToCallers();
    return ERR_OK;
}

void AbilityManagerService::GetRunningMultiAppIndex(const std::string &bundleName, int32_t uid, int32_t &appIndex)
{
    AppExecFwk::RunningMultiAppInfo runningMultiAppInfo;
    auto appMgr = GetAppMgr();
    if (appMgr == nullptr) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "GetAppMgr failed");
        return;
    }
    auto ret = IN_PROCESS_CALL(appMgr->GetRunningMultiAppInfoByBundleName(bundleName, runningMultiAppInfo));
    if (ret != ERR_OK) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "GetRunningMultiAppInfo failed bundleName = %{public}s",
            bundleName.c_str());
    }
    for (auto &item : runningMultiAppInfo.runningAppClones) {
        if (item.uid == uid) {
            appIndex = item.appCloneIndex;
            break;
        }
    }
}

void AbilityManagerService::NotifyFrozenProcessByRSS(const std::vector<int32_t> &pidList, int32_t uid)
{
    if (!PermissionVerification::GetInstance()->CheckSpecificSystemAbilityAccessPermission(RSS_PROCESS_NAME)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "caller is not RSS.");
        return;
    }
    auto userId = uid / BASE_USER_RANGE;
    auto connectManager = GetConnectManagerByUserId(userId);
    CHECK_POINTER_LOG(connectManager, "can not find user connect manager");
    connectManager->HandleProcessFrozen(pidList, uid);
}

void AbilityManagerService::HandleRestartResidentProcessDependedOnWeb()
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call");
    auto appMgr = GetAppMgr();
    CHECK_POINTER_LOG(appMgr, "get appMgr fail");
    appMgr->RestartResidentProcessDependedOnWeb();
}

int32_t AbilityManagerService::PreStartMission(const std::string& bundleName, const std::string& moduleName,
    const std::string& abilityName, const std::string& startTime)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "called");
    CHECK_CALLER_IS_SYSTEM_APP;

    if (!PermissionVerification::GetInstance()->VerifyPreStartAtomicServicePermission()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "calling user is not ag.");
        return ERR_PERMISSION_DENIED;
    }

    if (!freeInstallManager_) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "freeInstallManager_ is nullptr.");
        return ERR_INVALID_VALUE;
    }

    FreeInstallInfo taskInfo;
    if (!freeInstallManager_->GetFreeInstallTaskInfo(bundleName, abilityName, startTime, taskInfo)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR,
            "failed to find free install task info:bundleName=%{public}s,abilityName=%{public}s,startTime=%{public}s",
            bundleName.c_str(), abilityName.c_str(), startTime.c_str());
        return ERR_FREE_INSTALL_TASK_NOT_EXIST;
    }

    if (taskInfo.isFreeInstallFinished) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "free install is finished.");
        if (!taskInfo.isInstalled) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "free install task failed,resultCode=%{public}d",
                taskInfo.resultCode);
        } else {
            TAG_LOGI(AAFwkTag::ABILITYMGR, "free install has succeeded.");
        }
        // if free install is already finished then either the window is opened (on success)
        // or the user is informed of the error (on failure).
        return taskInfo.resultCode;
    }

    return PreStartInner(taskInfo);
}

int32_t AbilityManagerService::PreStartInner(const FreeInstallInfo& taskInfo)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "called");

    const Want& want = taskInfo.want;
    sptr<IRemoteObject> callerToken = taskInfo.callerToken;

    EventInfo eventInfo = BuildEventInfo(want, taskInfo.userId);
    SendAbilityEvent(EventName::START_ABILITY, HiSysEventType::BEHAVIOR, eventInfo);

    if (callerToken != nullptr && !VerificationAllToken(callerToken)) {
        eventInfo.errCode = ERR_INVALID_VALUE;
        SendAbilityEvent(EventName::START_ABILITY_ERROR, HiSysEventType::FAULT, eventInfo);
        return ERR_INVALID_CALLER;
    }

    int32_t oriValidUserId = GetValidUserId(taskInfo.userId);

    int32_t appIndex = 0;
    StartAbilityInfoWrap threadLocalInfo(want, oriValidUserId,
        StartAbilityUtils::GetAppIndex(want, callerToken, appIndex), callerToken);

    AbilityRequest abilityRequest = {
        .want = want,
        .requestCode = taskInfo.requestCode,
        .callerToken = callerToken,
        .startSetting = nullptr
    };

    TAG_LOGD(AAFwkTag::ABILITYMGR, "do not start as caller, UpdateCallerInfo");
    UpdateCallerInfo(abilityRequest.want, callerToken);

    // sceneboard
    abilityRequest.userId = oriValidUserId;
    abilityRequest.want.SetParam(IS_CALL_BY_SCB, false);
    std::string sessionId = std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count());
    abilityRequest.want.SetParam(KEY_SESSION_ID, sessionId);
    auto uiAbilityManager = GetUIAbilityManagerByUserId(oriValidUserId);
    CHECK_POINTER_AND_RETURN(uiAbilityManager, ERR_INVALID_VALUE);
    sptr<SessionInfo> sessionInfo = nullptr;
    auto errCode = uiAbilityManager->NotifySCBToPreStartUIAbility(abilityRequest, sessionInfo);
    if (errCode != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "failed to notify sceneboard to pre-start uiability.");
        return errCode;
    }
    freeInstallManager_->SetFreeInstallTaskSessionId(taskInfo.want.GetElement().GetBundleName(),
        taskInfo.want.GetElement().GetAbilityName(),
        taskInfo.want.GetStringParam(Want::PARAM_RESV_START_TIME), sessionId);

    freeInstallManager_->SetPreStartMissionCallStatus(taskInfo.want.GetElement().GetBundleName(),
        taskInfo.want.GetElement().GetAbilityName(),
        taskInfo.want.GetStringParam(Want::PARAM_RESV_START_TIME),
        true);
    return ERR_OK;
}

int32_t AbilityManagerService::StartUIAbilityByPreInstall(const FreeInstallInfo &taskInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "called");
    if (!taskInfo.isFreeInstallFinished || !taskInfo.isInstalled) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "free install is not finished or has failed.");
        return ERR_INVALID_VALUE;
    }
    if (!taskInfo.isStartUIAbilityBySCBCalled) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "Free install is finished, StartUIAbilityBySCB has not been called");
        return ERR_OK;
    }

    const auto& want = taskInfo.want;
    auto sessionId = want.GetStringParam(KEY_SESSION_ID);
    if (sessionId.empty()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "session id is empty.");
        return ERR_INVALID_VALUE;
    }
    auto bundleName = want.GetElement().GetBundleName();
    auto abilityName = want.GetElement().GetAbilityName();
    auto startTime = want.GetStringParam(Want::PARAM_RESV_START_TIME);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "called"
        "sessionId=%{public}s,bundleName=%{public}s,abilityName=%{public}s,startTime=%{public}s",
        sessionId.c_str(), bundleName.c_str(), abilityName.c_str(), startTime.c_str());
    sptr<SessionInfo> sessionInfo = nullptr;
    {
        std::lock_guard<ffrt::mutex> guard(preStartSessionMapLock_);
        auto it = preStartSessionMap_.find(sessionId);
        if (it == preStartSessionMap_.end()) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "failed to find session info with sessionId=%{public}s",
                sessionId.c_str());
            return ERR_INVALID_VALUE;
        }
        sessionInfo = it->second;
        (sessionInfo->want).SetElement(want.GetElement());
    }

    int errCode = ERR_OK;
    bool isColdStart = true;
    if ((errCode = StartUIAbilityByPreInstallInner(sessionInfo, taskInfo.specifyTokenId, isColdStart)) != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "StartUIAbilityByPreInstallInner failed,errCode=%{public}d.", errCode);
    }
    RemovePreStartSession(sessionId);
    return errCode;
}

// StartUIAbilityByPreInstallInner is called when free install task is already finished
int AbilityManagerService::StartUIAbilityByPreInstallInner(sptr<SessionInfo> sessionInfo,
    uint32_t specifyTokenId, bool &isColdStart)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "called");
    auto callerToken = sessionInfo->callerToken;
    const auto& want = sessionInfo->want;
    const auto userId = sessionInfo->userId;
    const auto requestCode = sessionInfo->requestCode;
    bool isStartAsCaller = false;

    if (callerToken != nullptr && !VerificationAllToken(callerToken)) {
        auto isSpecificSA = AAFwk::PermissionVerification::GetInstance()->
            CheckSpecificSystemAbilityAccessPermission(DMS_PROCESS_NAME);
        if (!isSpecificSA) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "%{public}s VerificationAllToken failed.", __func__);
            return ERR_INVALID_CALLER;
        }
        TAG_LOGI(AAFwkTag::ABILITYMGR, "%{public}s: Caller is specific system ability.", __func__);
    }

    int32_t oriValidUserId = GetValidUserId(userId);
    int32_t validUserId = oriValidUserId;

    int32_t appIndex = 0;
    if (!StartAbilityUtils::GetAppIndex(want, callerToken, appIndex)) {
        return ERR_APP_CLONE_INDEX_INVALID;
    }
    StartAbilityInfoWrap threadLocalInfo(want, validUserId, appIndex, callerToken);
    AbilityInterceptorParam interceptorParam = AbilityInterceptorParam(want, requestCode, GetUserId(),
        true, nullptr);
    auto result = interceptorExecuter_ == nullptr ? ERR_INVALID_VALUE :
        interceptorExecuter_->DoProcess(interceptorParam);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "interceptorExecuter_ is nullptr or DoProcess return error.");
        return result;
    }

    AbilityRequest abilityRequest;
    result = GenerateAbilityRequest(want, requestCode, abilityRequest, callerToken, validUserId);
    auto abilityRecord = Token::GetAbilityRecordByToken(callerToken);
    std::string callerBundleName = abilityRecord ? abilityRecord->GetAbilityInfo().bundleName : "";

    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Generate ability request local error.");
        return result;
    }
    if (!UriUtils::GetInstance().CheckNonImplicitShareFileUri(abilityRequest)) {
        return ERR_SHARE_FILE_URI_NON_IMPLICITLY;
    }

    if (specifyTokenId > 0 && callerToken != nullptr) { // for sa specify tokenId and caller token
        UpdateCallerInfoFromToken(abilityRequest.want, callerToken);
    } else if (!isStartAsCaller) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "do not start as caller, UpdateCallerInfo");
        UpdateCallerInfo(abilityRequest.want, callerToken);
    } else if (callerBundleName == BUNDLE_NAME_DIALOG) {
#ifdef SUPPORT_SCREEN
        CHECK_POINTER_AND_RETURN(implicitStartProcessor_, ERR_IMPLICIT_START_ABILITY_FAIL);
        implicitStartProcessor_->ResetCallingIdentityAsCaller(abilityRequest.want.GetIntParam(
            Want::PARAM_RESV_CALLER_TOKEN, 0));
#endif // SUPPORT_SCREEN
    }

    auto abilityInfo = abilityRequest.abilityInfo;
    validUserId = abilityInfo.applicationInfo.singleton ? U0_USER_ID : validUserId;
    TAG_LOGD(AAFwkTag::ABILITYMGR, "userId is : %{public}d, singleton is : %{public}d",
        validUserId, static_cast<int>(abilityInfo.applicationInfo.singleton));

    result = CheckStaticCfgPermission(abilityRequest, isStartAsCaller,
        abilityRequest.want.GetIntParam(Want::PARAM_RESV_CALLER_TOKEN, 0), false, false, false);
    if (result != AppExecFwk::Constants::PERMISSION_GRANTED) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "CheckStaticCfgPermission error, result is %{public}d.", result);
        return ERR_STATIC_CFG_PERMISSION;
    }

    result = CheckCallPermission(want, abilityInfo, abilityRequest, false,
        false, specifyTokenId, callerBundleName);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "CheckCallPermission error, result is %{public}d.", result);
        return result;
    }

    Want newWant = abilityRequest.want;
    AbilityInterceptorParam afterCheckParam = AbilityInterceptorParam(newWant, requestCode, GetUserId(),
        true, callerToken, std::make_shared<AppExecFwk::AbilityInfo>(abilityInfo), isStartAsCaller);
    result = afterCheckExecuter_ == nullptr ? ERR_INVALID_VALUE :
        afterCheckExecuter_->DoProcess(afterCheckParam);
    bool isReplaceWantExist = newWant.GetBoolParam("queryWantFromErms", false);
    newWant.RemoveParam("queryWantFromErms");
    if (result != ERR_OK && isReplaceWantExist == false) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "DoProcess failed or replaceWant not exist");
        return result;
    }
#ifdef SUPPORT_SCREEN
    if (result != ERR_OK && isReplaceWantExist && callerBundleName != BUNDLE_NAME_DIALOG) {
        return DialogSessionManager::GetInstance().HandleErmsResult(abilityRequest, GetUserId(), newWant);
    }
    if (result == ERR_OK &&
        DialogSessionManager::GetInstance().IsCreateCloneSelectorDialog(abilityInfo.bundleName, GetUserId())) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "create clone selector dialog");
        return CreateCloneSelectorDialog(abilityRequest, GetUserId());
    }
#endif // SUPPORT_SCREEN

    if (abilityInfo.type == AppExecFwk::AbilityType::SERVICE ||
        abilityInfo.type == AppExecFwk::AbilityType::EXTENSION) {
        return StartAbilityByConnectManager(want, abilityRequest, abilityInfo, validUserId, callerToken);
    }

    if (!IsAbilityControllerStart(want, abilityInfo.bundleName)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "IsAbilityControllerStart failed: %{public}s.", abilityInfo.bundleName.c_str());
        return ERR_WOULD_BLOCK;
    }

    abilityRequest.want.RemoveParam(SPECIFY_TOKEN_ID);
    if (specifyTokenId > 0) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "Set specifyTokenId, the specifyTokenId is %{public}d.", specifyTokenId);
        abilityRequest.want.SetParam(SPECIFY_TOKEN_ID, static_cast<int32_t>(specifyTokenId));
        abilityRequest.specifyTokenId = specifyTokenId;
    }
    abilityRequest.want.RemoveParam(PARAM_SPECIFIED_PROCESS_FLAG);

    auto uiAbilityManager = GetCurrentUIAbilityManager();
    CHECK_POINTER_AND_RETURN(uiAbilityManager, ERR_INVALID_VALUE);

    return uiAbilityManager->StartUIAbility(abilityRequest, sessionInfo, isColdStart);
}

void AbilityManagerService::NotifySCBToHandleAtomicServiceException(const std::string& sessionId, int32_t errCode,
    const std::string& reason)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "called");
    sptr<SessionInfo> sessionInfo = nullptr;
    {
        std::lock_guard<ffrt::mutex> guard(preStartSessionMapLock_);
        auto it = preStartSessionMap_.find(sessionId);
        if (it == preStartSessionMap_.end()) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "failed to find session info with sessionId=%{public}s",
                sessionId.c_str());
            return;
        }
        sessionInfo = it->second;
        preStartSessionMap_.erase(it);
    }
    if (sessionInfo == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "sessionInfo is nullptr.");
        return;
    }
    auto uiAbilityManager = GetCurrentUIAbilityManager();
    CHECK_POINTER(uiAbilityManager);
    return uiAbilityManager->NotifySCBToHandleAtomicServiceException(sessionInfo, errCode, reason);
}

void AbilityManagerService::RemovePreStartSession(const std::string& sessionId)
{
    std::lock_guard<ffrt::mutex> guard(preStartSessionMapLock_);
    preStartSessionMap_.erase(sessionId);
}

ErrCode AbilityManagerService::OpenLink(const Want& want, sptr<IRemoteObject> callerToken,
    int32_t userId, int requestCode)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "called");
    std::string url = want.GetUriString();
    bool isAtomicServiceShortUrl = false;
#ifdef APP_DOMAIN_VERIFY_ENABLED
    isAtomicServiceShortUrl = AppDomainVerify::AppDomainVerifyMgrClient::GetInstance()->IsAtomicServiceUrl(url);
#endif
    int32_t retCode = ERR_OK;
    if (!isAtomicServiceShortUrl) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "not atomic service short url, start ability by default.");
        retCode = StartAbility(want, callerToken, userId, requestCode);
        CHECK_RET_RETURN_RET(retCode, "StartAbility failed");
        return ERR_OPEN_LINK_START_ABILITY_DEFAULT_OK;
    }

    Want convertedWant = want;
    retCode = ConvertToExplicitWant(convertedWant);
    if (retCode != ERR_OK) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "failed to convert to explicit want, start ability by default.");
        retCode = StartAbility(want, callerToken, userId, requestCode);
        CHECK_RET_RETURN_RET(retCode, "StartAbility failed");
        return ERR_OPEN_LINK_START_ABILITY_DEFAULT_OK;
    }

    if (!freeInstallManager_) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "free install manager is nullptr, start ability by default.");
        retCode = StartAbility(want, callerToken, userId, requestCode);
        CHECK_RET_RETURN_RET(retCode, "StartAbility failed");
        return ERR_OPEN_LINK_START_ABILITY_DEFAULT_OK;
    }

    convertedWant.AddFlags(Want::FLAG_INSTALL_ON_DEMAND);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "convertedWant=%{public}s", convertedWant.ToString().c_str());
    retCode = freeInstallManager_->StartFreeInstall(convertedWant, GetValidUserId(userId),
        requestCode, callerToken, true, 0, true, std::make_shared<Want>(want));
    if (retCode != ERR_OK) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "StartFreeInstall returns errCode=%{public}d.", retCode);
        if (retCode == NOT_TOP_ABILITY) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "start from background is not allowed.");
            return retCode;
        }
        TAG_LOGI(AAFwkTag::ABILITYMGR, "start ability by default.");
        retCode = StartAbility(want, callerToken, userId, requestCode);
        CHECK_RET_RETURN_RET(retCode, "StartAbility failed");
        return ERR_OPEN_LINK_START_ABILITY_DEFAULT_OK;
    }
    return ERR_OK;
}

ErrCode AbilityManagerService::ConvertToExplicitWant(Want& want)
{
    ErrCode retCode = ERR_OK;
#ifdef APP_DOMAIN_VERIFY_ENABLED
    auto bundleMgrHelper = GetBundleManager();
    if (bundleMgrHelper == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "bundleMgrHelper is invalid.");
        return ERR_INVALID_VALUE;
    }
    int32_t callerUid = IPCSkeleton::GetCallingUid();
    std::string callerBundleName;
    retCode = IN_PROCESS_CALL(bundleMgrHelper->GetNameForUid(callerUid, callerBundleName));
    if (retCode != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Get callerBundleName failed,retCode=%{public}d.", retCode);
        return retCode;
    }
    TAG_LOGI(AAFwkTag::ABILITYMGR, "callerBundleName=%{public}s.", callerBundleName.c_str());
    want.SetParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME, callerBundleName);

    bool isUsed = false;
    ffrt::condition_variable callbackDoneCv;
    ffrt::mutex callbackDoneMutex;
    ConvertCallbackTask task = [&retCode, &isUsed, &callbackDoneCv, &callbackDoneMutex,
        &convertedWant = want, this](int resultCode, AAFwk::Want& want) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "in convert callback task, resultCode=%{public}d,want=%{public}s",
            resultCode, want.ToString().c_str());
        retCode = resultCode;
        convertedWant = want;
        {
            std::lock_guard<ffrt::mutex> lock(callbackDoneMutex);
            isUsed = true;
        }
        TAG_LOGI(AAFwkTag::ABILITYMGR, "start to notify.");
        callbackDoneCv.notify_all();
        TAG_LOGI(AAFwkTag::ABILITYMGR, "convert callback task finished");
    };
    sptr<ConvertCallbackImpl> callbackTask = new ConvertCallbackImpl(std::move(task));
    sptr<OHOS::AppDomainVerify::IConvertCallback> callback = callbackTask;
    AppDomainVerify::AppDomainVerifyMgrClient::GetInstance()->ConvertToExplicitWant(want, callback);
    auto condition = [&isUsed] { return isUsed; };
    std::unique_lock<ffrt::mutex> lock(callbackDoneMutex);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "start to wait for condition.");
    if (!callbackDoneCv.wait_for(lock, seconds(CONVERT_CALLBACK_TIMEOUT_SECONDS), condition)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "convert callback timeout.");
        callbackTask->Cancel();
        retCode = ERR_TIMED_OUT;
    }
    TAG_LOGI(AAFwkTag::ABILITYMGR, "finish wait for condition.");
#endif
    return retCode;
}

int32_t AbilityManagerService::CleanUIAbilityBySCB(const sptr<SessionInfo> &sessionInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (sessionInfo == nullptr || sessionInfo->sessionToken == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "sessionInfo is invalid.");
        return ERR_INVALID_VALUE;
    }

    if (!IsCallerSceneBoard()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "only support sceneboard call.");
        return ERR_WRONG_INTERFACE_CALL;
    }

    auto uiAbilityManager = GetUIAbilityManagerByUid(IPCSkeleton::GetCallingUid());
    CHECK_POINTER_AND_RETURN(uiAbilityManager, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "user request ot clean session: %{public}d.", sessionInfo->persistentId);
    auto abilityRecord = uiAbilityManager->GetUIAbilityRecordBySessionInfo(sessionInfo);
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);

    int32_t errCode = uiAbilityManager->CleanUIAbility(abilityRecord);
    ReportCleanSession(sessionInfo, abilityRecord, errCode);
    return errCode;
}

void AbilityManagerService::ReportCleanSession(const sptr<SessionInfo> &sessionInfo,
    const std::shared_ptr<AbilityRecord> &abilityRecord, int32_t errCode)
{
    if (!sessionInfo || !abilityRecord) {
        return;
    }

    const auto &abilityInfo = abilityRecord->GetAbilityInfo();
    std::string abilityName = abilityInfo.name;
    if (abilityInfo.launchMode == AppExecFwk::LaunchMode::STANDARD) {
        abilityName += std::to_string(sessionInfo->persistentId);
    }
    (void)DelayedSingleton<AbilityRuntime::AppExitReasonDataManager>::GetInstance()->
        DeleteAbilityRecoverInfo(abilityInfo.applicationInfo.accessTokenId, abilityInfo.moduleName, abilityName);

    EventInfo eventInfo;
    eventInfo.errCode = errCode;
    eventInfo.bundleName = abilityRecord->GetAbilityInfo().bundleName;
    eventInfo.abilityName = abilityRecord->GetAbilityInfo().name;
    SendAbilityEvent(EventName::CLOSE_ABILITY, HiSysEventType::BEHAVIOR, eventInfo);
    if (eventInfo.errCode != ERR_OK) {
        SendAbilityEvent(EventName::TERMINATE_ABILITY_ERROR, HiSysEventType::FAULT, eventInfo);
    }
}

void AbilityManagerService::SetAbilityRequestSessionInfo(AbilityRequest &abilityRequest, AppExecFwk::ExtensionAbilityType extensionType)
{
    if (extensionType != AppExecFwk::ExtensionAbilityType::UI_SERVICE) {
        return;
    }
    sptr<SessionInfo> sessionInfo = new SessionInfo();
    sessionInfo->callerToken = abilityRequest.callerToken;
    auto callerAbilityRecord = Token::GetAbilityRecordByToken(abilityRequest.callerToken);
    if(callerAbilityRecord != nullptr) {
        sptr<SessionInfo> callerSessionInfo = callerAbilityRecord->GetSessionInfo();
        if (callerSessionInfo != nullptr) {
            sessionInfo->hostWindowId = callerSessionInfo->persistentId;
        }
    } else {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "callerAbilityRecord is nullptr");
    }
    if (abilityRequest.abilityInfo.extensionAbilityType == AppExecFwk::ExtensionAbilityType::UI_SERVICE) {
        abilityRequest.want.SetParam(WANT_PARAMS_HOST_WINDOW_ID_KEY, static_cast<int32_t>(sessionInfo->hostWindowId));
    }
    sessionInfo->want = abilityRequest.want;
    sessionInfo->callingTokenId = static_cast<uint32_t>(abilityRequest.want.GetIntParam(Want::PARAM_RESV_CALLER_TOKEN,
    IPCSkeleton::GetCallingTokenID()));
    abilityRequest.sessionInfo = sessionInfo;
}

int32_t AbilityManagerService::TerminateMission(int32_t missionId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "TerminateMission call");
    auto missionListManager = GetCurrentMissionListManager();
    CHECK_POINTER_AND_RETURN(missionListManager, ERR_NO_INIT);
    CHECK_CALLER_IS_SYSTEM_APP;

    if (!PermissionVerification::GetInstance()->VerifyCallingPermission(
        PermissionConstants::PERMISSION_KILL_APP_PROCESSES)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Permission verification failed");
        return CHECK_PERMISSION_FAILED;
    }

    return missionListManager->ClearMission(missionId);
}
}  // namespace AAFwk
}  // namespace OHOS
