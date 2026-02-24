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
#include "ability_command.h"

#include <charconv>
#include <csignal>
#include <cstdlib>
#include <getopt.h>
#include <iostream>
#include <regex>
#include "ability_manager_client.h"
#include "ability_start_with_wait_observer.h"
#include "ability_start_with_wait_observer_utils.h"
#include "ability_tool_convert_util.h"
#include "app_mgr_client.h"
#include "hilog_tag_wrapper.h"
#include "iservice_registry.h"
#include "mission_snapshot.h"
#include "bool_wrapper.h"
#include "parameters.h"
#include "sa_mgr_client.h"
#include "system_ability_definition.h"
#include "test_observer.h"
#include "app_mem_info.h"

using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AAFwk {
using TerminateReason = AbilityStartWithWaitObserverUtil::TerminateReason;
namespace {
constexpr size_t PARAM_LENGTH = 1024;
constexpr int INDEX_OFFSET = 3;
constexpr int EXTRA_ARGUMENTS_FOR_KEY_VALUE_PAIR = 1;
constexpr int EXTRA_ARGUMENTS_FOR_NULL_STRING = 0;
constexpr int OPTION_PARAMETER_VALUE_OFFSET = 1;
constexpr int SHORT_OPTION_INDEX = 1;

constexpr int OPTION_PARAMETER_INTEGER = 257;
constexpr int OPTION_PARAMETER_STRING = 258;
constexpr int OPTION_PARAMETER_BOOL = 259;
constexpr int OPTION_PARAMETER_NULL_STRING = 260;
constexpr int OPTION_WINDOW_LEFT = 261;
constexpr int OPTION_WINDOW_TOP = 262;
constexpr int OPTION_WINDOW_HEIGHT = 263;
constexpr int OPTION_WINDOW_WIDTH = 264;

constexpr int INVALID_PID = 10104003;
constexpr int INVALID_LEVEL = 10104004;

constexpr int INNER_ERR_START = 10108101;
constexpr int INNER_ERR_TEST = 10108501;
constexpr int INNER_ERR_DEBUG = 10108601;
constexpr int64_t WAIT_INTERVAL = 10 * 1000; // us
constexpr int64_t MAX_WAIT_TIME = 15 * 1000 * 1000; // us

const std::string DEVELOPERMODE_STATE = "const.security.developermode.state";

const std::string SHORT_OPTIONS = "ch:d:a:b:e:t:p:s:m:A:U:CDESNR";
const std::string SHORT_OPTION_CHARS = "chdabetpsmAUCDESNR";
const std::string RESOLVE_ABILITY_ERR_SOLUTION_ONE =
    "Check if the parameter abilityName of aa -a and the parameter bundleName of -b are correct";
const std::string RESOLVE_ABILITY_ERR_SOLUTION_TWO =
    "Check if the application corresponding to the specified bundleName is installed";
const std::string RESOLVE_ABILITY_ERR_SOLUTION_THREE =
    "For multi-HAP applications, it is necessary to confirm whether the HAP to which"
    " the ability belongs has been installed";
const std::string GET_ABILITY_SERVICE_FAILED_SOLUTION_ONE =
    "Check if the application corresponding to the specified bundleName is installed";
const std::string ABILITY_SERVICE_NOT_CONNECTED_SOLUTION_ONE =
    "Try restarting the device and executing again";
const std::string RESOLVE_APP_ERR_SOLUTION_ONE =
    "The app information retrieved from BMS is missing the application name or package name";
const std::string START_ABILITY_WAITING_SOLUTION_ONE = "No need to process, just wait for the startup";
const std::string INNER_ERR_START_SOLUTION_ONE = "Confirm whether the system memory is sufficient and "
    "if there are any issues with the system version used by the device";
const std::string INNER_ERR_START_SOLUTION_TWO = "Check if too many abilities have been launched";
const std::string INNER_ERR_START_SOLUTION_THREE = "Try restarting the device";
const std::string INNER_ERR_DEBUG_SOLUTION_ONE = "Confirm whether the system memory is sufficient and "
    "if there are any issues with the system version used by the device";
const std::string INNER_ERR_DEBUG_SOLUTION_TWO = "Try restarting the device";
const std::string INNER_ERR_TEST_SOLUTION_ONE = "Confirm whether the system memory is sufficient and "
    "if there are any issues with the system version used by the device";
const std::string INNER_ERR_TEST_SOLUTION_TWO = "Try restarting the device";
const std::string TARGET_ABILITY_NOT_SERVICE_SOLUTION_ONE =
    "Check whether the ability corresponding to the parameter abilityName in aa -a is of type serviceAbility";
const std::string KILL_PROCESS_FAILED_SOLUTION_ONE = "Confirm whether the target application exists";
const std::string KILL_PROCESS_FAILED_SOLUTION_TWO = "Confirm the permissions of the target process";
const std::string CHECK_PERMISSION_FAILED_SOLUTION_ONE = "Confirm whether the target ability can be launched";
const std::string NO_FOUND_ABILITY_BY_CALLER_SOLUTION_ONE = "Normal specifications, no action needed";
const std::string ABILITY_VISIBLE_FALSE_DENY_REQUEST_SOLUTION_ONE = "Check if the exported configuration "
    "of the Ability field in the module.json5 of the pulled application is set to true. If not, set it to true";
const std::string GET_BUNDLE_INFO_FAILED_SOLUTION_ONE = "Check if the bundleName is correct";
const std::string GET_BUNDLE_INFO_FAILED_SOLUTION_TWO = "Check whether the application corresponding"
    " to the specified bundleName is installed";
const std::string ERR_NOT_DEVELOPER_MODE_SOLUTION_ONE = "Enable developer mode in the settings";
const std::string KILL_PROCESS_KEEP_ALIVE_SOLUTION_ONE = "Normal specifications, no action needed";
const std::string ERR_UNLOCK_SCREEN_FAILED_IN_DEVELOPER_MODE_SOLUTION_ONE =
    "Check in the settings whether the current device is in developer mode, and turn off developer mode";
const std::string ERR_NOT_SUPPORTED_PRODUCT_TYPE_SOLUTION_ONE = "Normal specifications, no action needed";
const std::string ERR_NOT_IN_APP_PROVISION_MODE_SOLUTION_ONE = "The same application can be compiled with"
    " the Debug mode process to produce an application that supports Debug mode";
const std::string ERR_NOT_DEBUG_APP_SOLUTION_ONE = "Configure the target application as a Debug application";
const std::string ERR_APP_CLONE_INDEX_INVALID_SOLUTION_ONE = "Confirm whether the appCloneIndex is valid";
const std::string ERROR_SERVICE_NOT_CONNECTED_SOLUTION_ONE = "Try restarting the device";
const std::string ERR_STATIC_CFG_PERMISSION_SOLUTION_ONE =
    "Confirm whether the permissions of the specified process are correct";
const std::string ERR_CROWDTEST_EXPIRED_SOLUTION_ONE =
    "Please check whether the application has expired for beta testing; "
    "applications that have passed their validity period cannot be launched";
const std::string ERR_APP_CONTROLLED_SOLUTION_ONE = "It is recommended to uninstall the application";
const std::string ERR_EDM_APP_CONTROLLED_SOLUTION_ONE =
    "Please contact the personnel related to enterprise device management";
const std::string ERR_IMPLICIT_START_ABILITY_FAIL_SOLUTION_ONE =
    "Make sure the parameter configuration of implicit startup is correct";
const std::string ERR_IMPLICIT_START_ABILITY_FAIL_SOLUTION_TWO =
    "Make sure the corresponding HAP package is installed";
const std::string ERR_INVALID_VALUE_SOLUTION_ONE =
    "Check if the application corresponding to the specified bundleName is installed.";
const std::string ERR_INVALID_PID_VALUE_SOLUTION_ONE =
    "Check if the pid specified by the application exists.";
const std::string ERR_INVALID_LEVEL_VALUE_SOLUTION_ONE =
    "Check if the value range of level is [0, 1, 2, 3, 4, 5, 6].";
const std::string BLACK_ACTION_SELECT_DATA = "ohos.want.action.select";

constexpr struct option LONG_OPTIONS[] = {
    {"help", no_argument, nullptr, 'h'},
    {"device", required_argument, nullptr, 'd'},
    {"ability", required_argument, nullptr, 'a'},
    {"bundle", required_argument, nullptr, 'b'},
    {"perf", required_argument, nullptr, 'p'},
    {"setting", required_argument, nullptr, 's'},
    {"module", required_argument, nullptr, 'm'},
    {"cold-start", no_argument, nullptr, 'C'},
    {"debug", no_argument, nullptr, 'D'},
    {"error-info-enhance", no_argument, nullptr, 'E'},
    {"native-debug", no_argument, nullptr, 'N'},
    {"mutil-thread", no_argument, nullptr, 'R'},
    {"action", required_argument, nullptr, 'A'},
    {"URI", required_argument, nullptr, 'U'},
    {"wait", no_argument, nullptr, 'W'},
    {"entity", required_argument, nullptr, 'e'},
    {"type", required_argument, nullptr, 't'},
    {"pi", required_argument, nullptr, OPTION_PARAMETER_INTEGER},
    {"ps", required_argument, nullptr, OPTION_PARAMETER_STRING},
    {"pb", required_argument, nullptr, OPTION_PARAMETER_BOOL},
    {"psn", required_argument, nullptr, OPTION_PARAMETER_NULL_STRING},
    {"wl", required_argument, nullptr, OPTION_WINDOW_LEFT},
    {"wt", required_argument, nullptr, OPTION_WINDOW_TOP},
    {"wh", required_argument, nullptr, OPTION_WINDOW_HEIGHT},
    {"ww", required_argument, nullptr, OPTION_WINDOW_WIDTH},
    {nullptr, 0, nullptr, 0},
};
const std::string SHORT_OPTIONS_APPLICATION_NOT_RESPONDING = "hp:";
#ifdef ABILITY_COMMAND_FOR_TEST
constexpr struct option LONG_OPTIONS_ApplicationNotResponding[] = {
    {"help", no_argument, nullptr, 'h'},
    {"pid", required_argument, nullptr, 'p'},
    {nullptr, 0, nullptr, 0},
};
#endif
#ifdef ABILITY_FAULT_AND_EXIT_TEST
const std::string SHORT_OPTIONS_FORCE_EXIT_APP = "hp:r:";
constexpr struct option LONG_OPTIONS_FORCE_EXIT_APP[] = {
    { "help", no_argument, nullptr, 'h' },
    { "pid", required_argument, nullptr, 'p' },
    { "reason", required_argument, nullptr, 'r' },
    { nullptr, 0, nullptr, 0 },
};
const std::string SHORT_OPTIONS_NOTIFY_APP_FAULT = "hn:m:s:t:p:";
constexpr struct option LONG_OPTIONS_NOTIFY_APP_FAULT[] = {
    {"help", no_argument, nullptr, 'h'},
    {"errorName", required_argument, nullptr, 'n'},
    {"errorMessage", required_argument, nullptr, 'm'},
    {"errorStack", required_argument, nullptr, 's'},
    {"faultType", required_argument, nullptr, 't'},
    {"pid", required_argument, nullptr, 'p'},
    {nullptr, 0, nullptr, 0},
};
#endif
const std::string SHORT_OPTIONS_DUMPSYS = "hal::i:e::p::r::d::u:c";
constexpr struct option LONG_OPTIONS_DUMPSYS[] = {
    {"help", no_argument, nullptr, 'h'},
    {"all", no_argument, nullptr, 'a'},
    {"mission-list", no_argument, nullptr, 'l'},
    {"ability", required_argument, nullptr, 'i'},
    {"extension", no_argument, nullptr, 'e'},
    {"pending", no_argument, nullptr, 'p'},
    {"process", no_argument, nullptr, 'r'},
    {"data", no_argument, nullptr, 'd'},
    {"userId", required_argument, nullptr, 'u'},
    {"client", no_argument, nullptr, 'c'},
    {nullptr, 0, nullptr, 0},
};
const std::string SHORT_OPTIONS_PROCESS = "ha:b:p:m:D:S";
constexpr struct option LONG_OPTIONS_PROCESS[] = {
    {"help", no_argument, nullptr, 'h'},
    {"ability", required_argument, nullptr, 'a'},
    {"bundle", required_argument, nullptr, 'b'},
    {"perf", required_argument, nullptr, 'p'},
    {"module", required_argument, nullptr, 'm'},
    {"debug", required_argument, nullptr, 'D'},
    {nullptr, 0, nullptr, 0},
};
const std::string SHORT_OPTIONS_APPDEBUG = "hb:p::c::g";
constexpr struct option LONG_OPTIONS_APPDEBUG[] = {
    { "help", no_argument, nullptr, 'h' },
    { "bundlename", required_argument, nullptr, 'b' },
    { "persist", no_argument, nullptr, 'p' },
    { "cancel", no_argument, nullptr, 'c' },
    { "get", no_argument, nullptr, 'g' },
    { nullptr, 0, nullptr, 0 },
};
const std::string SHORT_OPTIONS_ATTACH = "hb:";
constexpr struct option LONG_OPTIONS_ATTACH[] = {
    {"help", no_argument, nullptr, 'h'},
    {"bundle", required_argument, nullptr, 'b'},
    {nullptr, 0, nullptr, 0},
};
const std::string SHORT_OPTIONS_SEND_MEMORY_LEVEL = "hp:l:";
constexpr struct option LONG_OPTIONS_SEND_MEMORY_LEVEL[] = {
    {"help", no_argument, nullptr, 'h'},
    {"pid", required_argument, nullptr, 'p' },
    {"level", required_argument, nullptr, 'l' },
    {nullptr, 0, nullptr, 0 },
};
}  // namespace

AbilityManagerShellCommand::AbilityManagerShellCommand(int argc, char* argv[]) : ShellCommand(argc, argv, TOOL_NAME)
{
    for (int i = 0; i < argc_; i++) {
        if (i > 1) {
            TAG_LOGI(AAFwkTag::AA_TOOL, "argc greater than 2, ignoring the rest");
            return;
        }
        TAG_LOGI(AAFwkTag::AA_TOOL, "argv_[%{public}d]: %{public}s", i, argv_[i]);
    }
}

ErrCode AbilityManagerShellCommand::CreateCommandMap()
{
    commandMap_ = {
        {"help", [this]() { return this->RunAsHelpCommand(); }},
        {"start", [this]() { return this->RunAsStartAbility(); }},
        {"stop-service", [this]() { return this->RunAsStopService(); }},
        {"dump", [this]() { return this->RunAsDumpsysCommand(); }},
        {"force-stop", [this]() { return this->RunAsForceStop(); }},
        {"test", [this]() { return this->RunAsTestCommand(); }},
        {"process", [this]() { return this->RunAsProcessCommand(); }},
        {"attach", [this]() { return this->RunAsAttachDebugCommand(); }},
        {"detach", [this]() { return this->RunAsDetachDebugCommand(); }},
        {"appdebug", [this]() { return this->RunAsAppDebugDebugCommand(); }},
        {"send-memory-level", [this]() { return this->RunAsSendMemoryLevelCommand(); }},
#ifdef ABILITY_COMMAND_FOR_TEST
        {"force-timeout", [this]() { return this->RunForceTimeoutForTest(); }},
#endif
#ifdef ABILITY_FAULT_AND_EXIT_TEST
        {"forceexitapp", [this]() { return this->RunAsForceExitAppCommand(); }},
        {"notifyappfault", [this]() { return this->RunAsNotifyAppFaultCommand(); }},
#endif
    };

    return OHOS::ERR_OK;
}

ErrCode AbilityManagerShellCommand::CreateMessageMap()
{
    messageMap_[RESOLVE_ABILITY_ERR] = GetAaToolErrorInfo("10104001", "The specified ability does not exist",
        "The specified Ability is not installed",
        {RESOLVE_ABILITY_ERR_SOLUTION_ONE, RESOLVE_ABILITY_ERR_SOLUTION_TWO, RESOLVE_ABILITY_ERR_SOLUTION_THREE});
    messageMap_[GET_ABILITY_SERVICE_FAILED] = GetAaToolErrorInfo("10105002", "Failed to get the ability service",
        "The abilityInfo is empty when generating the Ability request through BMS",
        {GET_ABILITY_SERVICE_FAILED_SOLUTION_ONE});
    messageMap_[ABILITY_SERVICE_NOT_CONNECTED] = GetAaToolErrorInfo("10105001",
        "Ability service connection failed",
        "Failed to obtain the ability remote service",
        {ABILITY_SERVICE_NOT_CONNECTED_SOLUTION_ONE});
    messageMap_[RESOLVE_APP_ERR] = GetAaToolErrorInfo("10100101",
        "An error of the Want could not be resolved to app info from BMS",
        "Abnormal app information retrieved from BMS",
        {RESOLVE_APP_ERR_SOLUTION_ONE});
    messageMap_[ABILITY_EXISTED] = "error: ability existed.";
    messageMap_[CREATE_MISSION_STACK_FAILED] = "error: create mission stack failed.";
    messageMap_[CREATE_ABILITY_RECORD_FAILED] = "error: create ability record failed.";
    messageMap_[START_ABILITY_WAITING] = GetAaToolErrorInfo("10106101",
        "Another ability is being started. Wait until it finishes starting",
        "High system concurrency",
        {START_ABILITY_WAITING_SOLUTION_ONE});
    messageMap_[TERMINATE_LAUNCHER_DENIED] = "error: terminate launcher denied.";
    messageMap_[CONNECTION_NOT_EXIST] = "error: connection not exist.";
    messageMap_[INVALID_CONNECTION_STATE] = "error: invalid connection state.";
    messageMap_[LOAD_ABILITY_TIMEOUT] = "error: load ability timeout.";
    messageMap_[CONNECTION_TIMEOUT] = "error: connection timeout.";
    messageMap_[GET_BUNDLE_MANAGER_SERVICE_FAILED] = "error: get bundle manager service failed.";
    messageMap_[REMOVE_MISSION_FAILED] = "error: remove mission failed.";
    messageMap_[INNER_ERR] = "error: inner err.";
    messageMap_[INNER_ERR_START] = GetAaToolErrorInfo("10108101",
        "An internal error occurs while attempting to launch the ability.",
        "Kernel common errors such as memory allocation and multithreading processing. "
        "Specific reasons may include: internal object being null, processing timeout, "
        "failure to obtain application information from package management, failure to obtain system service, "
        "the number of launched ability instances has reached the limit, etc",
        {INNER_ERR_START_SOLUTION_ONE, INNER_ERR_START_SOLUTION_TWO, INNER_ERR_START_SOLUTION_THREE});
    messageMap_[INNER_ERR_DEBUG] = GetAaToolErrorInfo("10108601", "Internal error",
        "General kernel errors related to memory allocation, multithreading, etc. The specific reasons may "
        "include: internal objects being null, processing timeouts, failure to obtain system services, and so on",
        {INNER_ERR_DEBUG_SOLUTION_ONE, INNER_ERR_DEBUG_SOLUTION_TWO});
    messageMap_[INNER_ERR_TEST] = GetAaToolErrorInfo("10108501", "Internal error",
        "The current device is not in developer mode",
        {INNER_ERR_TEST_SOLUTION_ONE, INNER_ERR_TEST_SOLUTION_TWO});
    messageMap_[GET_RECENT_MISSIONS_FAILED] = "error: get recent missions failed.";
    messageMap_[REMOVE_STACK_LAUNCHER_DENIED] = "error: remove stack launcher denied.";
    messageMap_[TARGET_ABILITY_NOT_SERVICE] = GetAaToolErrorInfo("10103201",
        "The target ability is not of the ServiceAbility type.",
        "The ability corresponding to abilityName is not of service type",
        {TARGET_ABILITY_NOT_SERVICE_SOLUTION_ONE});
    messageMap_[TERMINATE_SERVICE_IS_CONNECTED] = "error: terminate service is connected.";
    messageMap_[START_SERVICE_ABILITY_ACTIVATING] = "error: start service ability activating.";
    messageMap_[KILL_PROCESS_FAILED] = GetAaToolErrorInfo("10106401", "kill process failed",
        "The specified application's process ID does not exist, "
        "there is no permission to kill the target process, or the connection to appManagerService was not successful",
        {KILL_PROCESS_FAILED_SOLUTION_ONE, KILL_PROCESS_FAILED_SOLUTION_TWO});
    messageMap_[UNINSTALL_APP_FAILED] = "error: uninstall app failed.";
    messageMap_[TERMINATE_ABILITY_RESULT_FAILED] = "error: terminate ability result failed.";
    messageMap_[CHECK_PERMISSION_FAILED] = GetAaToolErrorInfo("10107101",
        "Permission check failed when launching the ability",
        "No permission to start this ability",
        {CHECK_PERMISSION_FAILED_SOLUTION_ONE});
    messageMap_[NO_FOUND_ABILITY_BY_CALLER] = GetAaToolErrorInfo("10100102",
        "The aa start command cannot be used to launch a UIExtensionAbility.",
        "aa start does not meet the restrictions imposed by UIExtensionAbility on the initiating party",
        {NO_FOUND_ABILITY_BY_CALLER_SOLUTION_ONE});
    messageMap_[ABILITY_VISIBLE_FALSE_DENY_REQUEST] = GetAaToolErrorInfo("10103001",
        "Failed to verify the visibility of the target ability.",
        "Application visibility check failed",
        {ABILITY_VISIBLE_FALSE_DENY_REQUEST_SOLUTION_ONE});
    messageMap_[GET_BUNDLE_INFO_FAILED] = GetAaToolErrorInfo("10104002",
        "Failed to retrieve specified package information.",
        "The application corresponding to the specified package name is not installed.",
        {GET_BUNDLE_INFO_FAILED_SOLUTION_ONE, GET_BUNDLE_INFO_FAILED_SOLUTION_TWO});
    messageMap_[INVALID_PID] = GetAaToolErrorInfo("10104003", "The specified pid does not exist.",
        "The pid specified by the aa send-memory-level command does not exist.",
        {ERR_INVALID_PID_VALUE_SOLUTION_ONE});
    messageMap_[INVALID_LEVEL] = GetAaToolErrorInfo("10104004", "The specified level does not exist.",
        "The level specified by the aa send-memory-level command does not exist.",
        {ERR_INVALID_LEVEL_VALUE_SOLUTION_ONE});
    messageMap_[ERR_NOT_DEVELOPER_MODE] = GetAaToolErrorInfo("10106001", "not developer Mode",
        "The current device is not in developer mode",
        {ERR_NOT_DEVELOPER_MODE_SOLUTION_ONE});
    messageMap_[KILL_PROCESS_KEEP_ALIVE] = GetAaToolErrorInfo("10106402",
        "Persistent processes cannot be terminated.",
        "Designate the process as a persistent process and ensure that the device has sufficient memory",
        {KILL_PROCESS_KEEP_ALIVE_SOLUTION_ONE});
    messageMap_[ERR_UNLOCK_SCREEN_FAILED_IN_DEVELOPER_MODE] = GetAaToolErrorInfo("10106102",
        "The device screen is locked during the application launch, unlock screen failed.",
        "The current mode is developer mode, and the screen cannot be unlocked automatically",
        {ERR_UNLOCK_SCREEN_FAILED_IN_DEVELOPER_MODE_SOLUTION_ONE});
    messageMap_[ERR_NOT_SUPPORTED_PRODUCT_TYPE] = GetAaToolErrorInfo("10106107",
        "The current device does not support using window options.",
        "The user specified windowOptions, but the device does not support it",
        {ERR_NOT_SUPPORTED_PRODUCT_TYPE_SOLUTION_ONE});
    messageMap_[ERR_NOT_IN_APP_PROVISION_MODE] = GetAaToolErrorInfo("10106002",
        "The aa start command's window option or the aa test command does not support app with release signature.",
        "The application started by the aa command is a release signature",
        {ERR_NOT_IN_APP_PROVISION_MODE_SOLUTION_ONE});
    messageMap_[ERR_NOT_DEBUG_APP] = GetAaToolErrorInfo("10106701",
        "Cannot debug applications using a release certificate.",
        "The developer forgot to configure the target application as a Debug application",
        {ERR_NOT_DEBUG_APP_SOLUTION_ONE});
    messageMap_[ERR_APP_CLONE_INDEX_INVALID] = GetAaToolErrorInfo("10103102",
        "The passed appCloneIndex is invalid.",
        "If the appCloneIndex carried in the parameters of the aa tool is an invalid value, return that error code",
        {ERR_APP_CLONE_INDEX_INVALID_SOLUTION_ONE});
    messageMap_[ERROR_SERVICE_NOT_CONNECTED] = GetAaToolErrorInfo("10105003",
        "App service connection failed",
        "Failed to retrieve the App remote service",
        {ERROR_SERVICE_NOT_CONNECTED_SOLUTION_ONE});
    messageMap_[ERR_STATIC_CFG_PERMISSION] = GetAaToolErrorInfo("10107102",
        "The specified process does not have the permission",
        "The specified process permission check failed",
        {ERR_STATIC_CFG_PERMISSION_SOLUTION_ONE});
    messageMap_[ERR_CROWDTEST_EXPIRED] = GetAaToolErrorInfo("10106103",
        "Failed to unlock the screen in developer mode",
        "The current mode is developer mode, and the screen cannot be unlocked automatically",
        {ERR_CROWDTEST_EXPIRED_SOLUTION_ONE});
    messageMap_[ERR_APP_CONTROLLED] = GetAaToolErrorInfo("10106105",
        "The target application is under control.",
        "The application is suspected of malicious behavior and is restricted from launching by the appStore",
        {ERR_APP_CONTROLLED_SOLUTION_ONE});
    messageMap_[ERR_EDM_APP_CONTROLLED] = GetAaToolErrorInfo("10106106",
        "The target application is managed by EDM.",
        "The application is under the control of enterprise device management",
        {ERR_EDM_APP_CONTROLLED_SOLUTION_ONE});
    messageMap_[ERR_IMPLICIT_START_ABILITY_FAIL] = GetAaToolErrorInfo("10103101",
        "Failed to find a matching application for implicit launch.",
        "The parameter configuration of implicit startup is incorrect, or the specified HAP package is not installed.",
        {ERR_IMPLICIT_START_ABILITY_FAIL_SOLUTION_ONE, ERR_IMPLICIT_START_ABILITY_FAIL_SOLUTION_TWO});
    messageMap_[ERR_INVALID_VALUE] = GetAaToolErrorInfo("10103601",
        "The specified bundleName does not exist.",
        "The bundleName specified by the aa attach/detach command does not exist.",
        {ERR_INVALID_VALUE_SOLUTION_ONE});
    messageMap_[ERR_INVALID_OPERATION] = GetAaToolErrorInfo("10105002", "Failed to obtain ability information.",
        "The abilityInfo is empty when generating the Ability request through BMS",
        {GET_ABILITY_SERVICE_FAILED_SOLUTION_ONE});
    return OHOS::ERR_OK;
}

std::string AbilityManagerShellCommand::GetAaToolErrorInfo(std::string errorCode, std::string message,
    std::string cause, std::vector<std::string> solutions)
{
    AaToolErrorInfo aaToolErrorInfo;
    aaToolErrorInfo.code = errorCode;
    aaToolErrorInfo.message = message;
    aaToolErrorInfo.cause = cause;
    aaToolErrorInfo.solutions = solutions;
    return aaToolErrorInfo.ToString();
}

ErrCode AbilityManagerShellCommand::init()
{
    startTime_ = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    return AbilityManagerClient::GetInstance()->Connect();
}

ErrCode AbilityManagerShellCommand::RunAsHelpCommand()
{
    resultReceiver_.append(HELP_MSG);

    return OHOS::ERR_OK;
}

ErrCode AbilityManagerShellCommand::RunAsStartAbility()
{
    Want want;
    std::string windowMode;
    ErrCode result = MakeWantFromCmd(want, windowMode);
    if (result == OHOS::ERR_OK) {
        int windowModeKey = std::atoi(windowMode.c_str());
        if (windowModeKey > 0) {
            auto setting = AbilityStartSetting::GetEmptySetting();
            if (setting != nullptr) {
                setting->AddProperty(AbilityStartSetting::WINDOW_MODE_KEY, windowMode);
                result = AbilityManagerClient::GetInstance()->StartAbility(want, *(setting.get()), nullptr, -1);
            }
        } else if (startAbilityWithWaitFlag_) {
            result = StartAbilityWithWait(want);
        } else {
            result = AbilityManagerClient::GetInstance()->StartAbility(want);
        }
        if (result == OHOS::ERR_OK) {
            TAG_LOGI(AAFwkTag::AA_TOOL, "%{public}s", STRING_START_ABILITY_OK.c_str());
            resultReceiver_.append(STRING_START_ABILITY_OK).append("\n");
        } else {
            TAG_LOGI(AAFwkTag::AA_TOOL, "%{public}s result: %{public}d", STRING_START_ABILITY_NG.c_str(), result);
            if (result != START_ABILITY_WAITING) {
                resultReceiver_ = STRING_START_ABILITY_NG + "\n";
            }
            CheckStartAbilityResult(result);
            if (result == INNER_ERR) {
                result = INNER_ERR_START;
            }
            resultReceiver_.append(GetMessageFromCode(result));
        }
    } else {
        resultReceiver_.append(HELP_MSG_START);
        result = OHOS::ERR_INVALID_VALUE;
    }

    return result;
}

void AbilityManagerShellCommand::CheckStartAbilityResult(ErrCode& result)
{
    auto it = messageMap_.find(result);
    if (it == messageMap_.end()) {
        result = INNER_ERR;
    }
}

ErrCode AbilityManagerShellCommand::RunAsStopService()
{
    ErrCode result = OHOS::ERR_OK;

    Want want;
    result = MakeWantFromCmdForStopService(want);
    if (result == OHOS::ERR_OK) {
        result = AbilityManagerClient::GetInstance()->StopServiceAbility(want);
        if (result == OHOS::ERR_OK) {
            TAG_LOGI(AAFwkTag::AA_TOOL, "%{public}s", STRING_STOP_SERVICE_ABILITY_OK.c_str());
            resultReceiver_ = STRING_STOP_SERVICE_ABILITY_OK + "\n";
        } else {
            TAG_LOGI(
                AAFwkTag::AA_TOOL, "%{public}s result: %{public}d", STRING_STOP_SERVICE_ABILITY_NG.c_str(), result);
            resultReceiver_ = STRING_STOP_SERVICE_ABILITY_NG + "\n";

            resultReceiver_.append(GetMessageFromCode(result));
        }
    } else {
        resultReceiver_.append(HELP_MSG_STOP_SERVICE);
        result = OHOS::ERR_INVALID_VALUE;
    }

    return result;
}

ErrCode AbilityManagerShellCommand::RunAsDumpsysCommand()
{
    ErrCode result = OHOS::ERR_OK;
    bool isUserID = false;
    bool isClient = false;
    int userID = DEFAULT_INVAL_VALUE;
    bool isfirstCommand = false;
    std::string args;
    for (auto it = argList_.begin(); it != argList_.end(); it++) {
        if (*it == "-c" || *it == "--client") {
            if (isClient == false) {
                isClient = true;
            } else {
                result = OHOS::ERR_INVALID_VALUE;
                resultReceiver_.append(HELP_MSG_DUMPSYS);
                return result;
            }
        } else if (*it == "-u" || *it == "--userId") {
            if (it + 1 == argList_.end()) {
                result = OHOS::ERR_INVALID_VALUE;
                resultReceiver_.append(HELP_MSG_DUMPSYS);
                return result;
            }
            (void)StrToInt(*(it + 1), userID);
            if (userID == DEFAULT_INVAL_VALUE) {
                result = OHOS::ERR_INVALID_VALUE;
                resultReceiver_.append(HELP_MSG_DUMPSYS);
                return result;
            }
            if (isUserID == false) {
                isUserID = true;
            } else {
                result = OHOS::ERR_INVALID_VALUE;
                resultReceiver_.append(HELP_MSG_DUMPSYS);
                return result;
            }
        } else if (*it == std::to_string(userID)) {
            continue;
        } else {
            args += *it;
            args += " ";
        }
    }

    while (true) {
        int option = getopt_long(argc_, argv_, SHORT_OPTIONS_DUMPSYS.c_str(), LONG_OPTIONS_DUMPSYS, nullptr);

        TAG_LOGI(
            AAFwkTag::AA_TOOL, "option: %{public}d, optopt: %{public}d, optind: %{public}d", option, optopt, optind);

        if (optind < 0 || optind > argc_) {
            resultReceiver_.append(HELP_MSG_DUMPSYS);
            return OHOS::ERR_INVALID_VALUE;
        }

        if (option == -1) {
            break;
        }

        switch (option) {
            case 'h': {
                // 'aa dumpsys -h'
                // 'aa dumpsys --help'
                resultReceiver_.append(HELP_MSG_DUMPSYS);
                result = OHOS::ERR_INVALID_VALUE;
                return result;
            }
            case 'a': {
                if (isfirstCommand == false) {
                    isfirstCommand = true;
                } else {
                    result = OHOS::ERR_INVALID_VALUE;
                    resultReceiver_.append(HELP_MSG_DUMPSYS);
                    return result;
                }
                // 'aa dumpsys -a'
                // 'aa dumpsys --all'
                break;
            }
            case 'l': {
                if (isfirstCommand == false) {
                    isfirstCommand = true;
                } else {
                    // 'aa dump -i 10 -element -lastpage'
                    // 'aa dump -i 10 -render -lastpage'
                    // 'aa dump -i 10 -layer'
                    if ((optarg != nullptr) && strcmp(optarg, "astpage") && strcmp(optarg, "ayer")) {
                        result = OHOS::ERR_INVALID_VALUE;
                        resultReceiver_.append(HELP_MSG_DUMPSYS);
                        return result;
                    }
                }
                // 'aa dumpsys -l'
                // 'aa dumpsys --mission-list'
                break;
            }
            case 'i': {
                if (isfirstCommand == false) {
                    isfirstCommand = true;
                    int abilityRecordId = DEFAULT_INVAL_VALUE;
                    (void)StrToInt(optarg, abilityRecordId);
                    if (abilityRecordId == DEFAULT_INVAL_VALUE) {
                        result = OHOS::ERR_INVALID_VALUE;
                        resultReceiver_.append(HELP_MSG_DUMPSYS);
                        return result;
                    }
                } else {
                    // 'aa dumpsys -i 10 -inspector'
                    if ((optarg != nullptr) && strcmp(optarg, "nspector")) {
                        result = OHOS::ERR_INVALID_VALUE;
                        resultReceiver_.append(HELP_MSG_DUMPSYS);
                        return result;
                    }
                }
                // 'aa dumpsys -i'
                // 'aa dumpsys --ability'
                break;
            }
            case 'e': {
                if (isfirstCommand == false && optarg == nullptr) {
                    isfirstCommand = true;
                } else {
                    // 'aa dumpsys -i 10 -element'
                    if ((optarg != nullptr) && strcmp(optarg, "lement")) {
                        result = OHOS::ERR_INVALID_VALUE;
                        resultReceiver_.append(HELP_MSG_DUMPSYS);
                        return result;
                    }
                }
                // 'aa dumpsys -e'
                // 'aa dumpsys --extension'
                break;
            }
            case 'p': {
                if (isfirstCommand == false && optarg == nullptr) {
                    isfirstCommand = true;
                } else {
                    result = OHOS::ERR_INVALID_VALUE;
                    resultReceiver_.append(HELP_MSG_DUMPSYS);
                    return result;
                }
                // 'aa dumpsys -p'
                // 'aa dumpsys --pending'
                break;
            }
            case 'r': {
                if (isfirstCommand == false && optarg == nullptr) {
                    isfirstCommand = true;
                } else {
                    // 'aa dump -i 10 -render'
                    // 'aa dump -i 10 -rotation'
                    // 'aa dump -i 10 -frontend'
                    if ((optarg != nullptr) && strcmp(optarg, "ender") && strcmp(optarg, "otation") &&
                        strcmp(optarg, "ontend")) {
                        result = OHOS::ERR_INVALID_VALUE;
                        resultReceiver_.append(HELP_MSG_DUMPSYS);
                        return result;
                    }
                }
                // 'aa dumpsys -r'
                // 'aa dumpsys --process'
                break;
            }
            case 'd': {
                if (isfirstCommand == false && optarg == nullptr) {
                    isfirstCommand = true;
                } else {
                    result = OHOS::ERR_INVALID_VALUE;
                    resultReceiver_.append(HELP_MSG_DUMPSYS);
                    return result;
                }
                // 'aa dumpsys -d'
                // 'aa dumpsys --data'
                break;
            }
            case 'u': {
                // 'aa dumpsys -u'
                // 'aa dumpsys --userId'
                break;
            }
            case 'c': {
                // 'aa dumpsys -c'
                // 'aa dumpsys --client'
                break;
            }
            case '?': {
                if (!isfirstCommand) {
                    TAG_LOGI(AAFwkTag::AA_TOOL, "'aa %{public}s' option unknown", cmd_.c_str());
                    std::string unknownOption = "";
                    std::string unknownOptionMsg = GetUnknownOptionMsg(unknownOption);
                    resultReceiver_.append(unknownOptionMsg);
                    resultReceiver_.append(HELP_MSG_DUMPSYS);
                    result = OHOS::ERR_INVALID_VALUE;
                    return result;
                }
                break;
            }
            default: {
                TAG_LOGI(AAFwkTag::AA_TOOL, "'aa %{public}s' option unknown", cmd_.c_str());
                std::string unknownOption = "";
                std::string unknownOptionMsg = GetUnknownOptionMsg(unknownOption);
                resultReceiver_.append(unknownOptionMsg);
                result = OHOS::ERR_INVALID_VALUE;
                break;
            }
        }
    }

    if (result != OHOS::ERR_OK) {
        resultReceiver_.append(HELP_MSG_DUMPSYS);
    } else {
        if (isfirstCommand != true) {
            result = OHOS::ERR_INVALID_VALUE;
            resultReceiver_.append(HELP_MSG_NO_OPTION + "\n");
            resultReceiver_.append(HELP_MSG_DUMPSYS);
            return result;
        }

        std::vector<std::string> dumpResults;
        result = AbilityManagerClient::GetInstance()->DumpSysState(args, dumpResults, isClient, isUserID, userID);
        if (result == OHOS::ERR_OK) {
            for (auto it : dumpResults) {
                resultReceiver_ += it + "\n";
            }
        } else {
            resultReceiver_.append(GetMessageFromCode(result));
            TAG_LOGI(AAFwkTag::AA_TOOL, "dump state failed");
        }
    }
    return result;
}

ErrCode AbilityManagerShellCommand::RunAsForceStop()
{
    TAG_LOGI(AAFwkTag::AA_TOOL, "enter");
    if (argList_.empty()) {
        resultReceiver_.append(HELP_MSG_FORCE_STOP);
        return OHOS::ERR_INVALID_VALUE;
    }
    std::string bundleName = argList_[0];
    TAG_LOGI(AAFwkTag::AA_TOOL, "Bundle name %{public}s", bundleName.c_str());

    auto reason = Reason::REASON_UNKNOWN;
    std::string inputReason = "aa force-stop";
    pid_t pid = 0;
    for (auto index = INDEX_OFFSET; index < argc_; ++index) {
        TAG_LOGD(AAFwkTag::AA_TOOL, "argv_[%{public}d]: %{public}s", index, argv_[index]);
        std::string opt = argv_[index];
        if (opt == "-p") {
            index++;
            if (index <= argc_) {
                TAG_LOGD(AAFwkTag::AA_TOOL, "argv_[%{public}d]: %{public}s", index, argv_[index]);
                std::string inputPid = argv_[index];
                pid = ConvertPid(inputPid);
            }
        } else if (opt == "-r") {
            index++;
            if (index <= argc_) {
                TAG_LOGD(AAFwkTag::AA_TOOL, "argv_[%{public}d]: %{public}s", index, argv_[index]);
                inputReason = argv_[index];
                reason = AbilityToolConvertUtil::ConvertExitReason(inputReason);
            }
        }
    }

    TAG_LOGI(AAFwkTag::AA_TOOL, "pid %{public}d, reason %{public}s", pid, inputReason.c_str());
    if (pid != 0 && reason != Reason::REASON_UNKNOWN) {
        ExitReason exitReason = {reason, "aa force-stop"};
        if (AbilityManagerClient::GetInstance()->RecordProcessExitReason(pid, exitReason) != ERR_OK) {
            TAG_LOGE(AAFwkTag::AA_TOOL, "bundle %{public}s record reason %{public}d failed",
                bundleName.c_str(), reason);
        }
    }

    ErrCode result = OHOS::ERR_OK;
    result = AbilityManagerClient::GetInstance()->KillProcess(bundleName, false, 0, inputReason);
    if (result == OHOS::ERR_OK) {
        TAG_LOGI(AAFwkTag::AA_TOOL, "%{public}s", STRING_FORCE_STOP_OK.c_str());
        resultReceiver_ = STRING_FORCE_STOP_OK + "\n";
    } else {
        TAG_LOGI(AAFwkTag::AA_TOOL, "%{public}s result: %{public}d", STRING_FORCE_STOP_NG.c_str(), result);
        resultReceiver_ = STRING_FORCE_STOP_NG + "\n";
        resultReceiver_.append(GetMessageFromCode(result));
    }
    return result;
}

pid_t AbilityManagerShellCommand::ConvertPid(std::string& inputPid)
{
    pid_t pid = 0;
    auto res = std::from_chars(inputPid.c_str(), inputPid.c_str() + inputPid.size(), pid);
    if (res.ec != std::errc()) {
        TAG_LOGE(AAFwkTag::AA_TOOL, "pid stoi(%{public}s) failed", inputPid.c_str());
    }
    return pid;
}

ErrCode AbilityManagerShellCommand::RunAsAttachDebugCommand()
{
    TAG_LOGD(AAFwkTag::AA_TOOL, "called");
    std::string bundleName = "";
    ParseBundleName(bundleName);
    if (bundleName.empty()) {
        resultReceiver_.append(HELP_MSG_ATTACH_APP_DEBUG + "\n");
        return OHOS::ERR_INVALID_VALUE;
    }

    auto result = AbilityManagerClient::GetInstance()->AttachAppDebug(bundleName);
    if (result == INNER_ERR) {
        result = INNER_ERR_DEBUG;
    }
    if (result == OHOS::ERR_OK) {
        resultReceiver_.append(STRING_ATTACH_APP_DEBUG_OK + "\n");
        return result;
    }
    resultReceiver_.append(GetMessageFromCode(result));
    TAG_LOGD(AAFwkTag::AA_TOOL, "%{public}s result: %{public}d", STRING_ATTACH_APP_DEBUG_NG.c_str(), result);
    resultReceiver_.append(STRING_ATTACH_APP_DEBUG_NG + "\n");
    return result;
}

ErrCode AbilityManagerShellCommand::RunAsDetachDebugCommand()
{
    TAG_LOGD(AAFwkTag::AA_TOOL, "called");
    std::string bundleName = "";
    ParseBundleName(bundleName);
    if (bundleName.empty()) {
        resultReceiver_.append(HELP_MSG_DETACH_APP_DEBUG + "\n");
        return OHOS::ERR_INVALID_VALUE;
    }

    auto result = AbilityManagerClient::GetInstance()->DetachAppDebug(bundleName);
    if (result == OHOS::ERR_OK) {
        resultReceiver_.append(STRING_DETACH_APP_DEBUG_OK + "\n");
        return result;
    }
    if (result == INNER_ERR) {
        result = INNER_ERR_DEBUG;
    }
    resultReceiver_.append(GetMessageFromCode(result));
    TAG_LOGD(AAFwkTag::AA_TOOL, "%{public}s result: %{public}d", STRING_DETACH_APP_DEBUG_NG.c_str(), result);
    resultReceiver_.append(STRING_DETACH_APP_DEBUG_NG + "\n");
    return result;
}

bool AbilityManagerShellCommand::SwitchOptionForAppDebug(
    int32_t option, std::string &bundleName, bool &isPersist, bool &isCancel, bool &isGet)
{
    switch (option) {
        case 'h': { // 'aa appdebug -h' or 'aa appdebug --help'
            TAG_LOGD(AAFwkTag::AA_TOOL, "'aa %{public}s -h' help", cmd_.c_str());
            return true;
        }
        case 'b': { // 'aa appdebug -b bundlename'
            TAG_LOGD(AAFwkTag::AA_TOOL, "'aa %{public}s -b' bundle name", cmd_.c_str());
            bundleName = optarg;
            return false;
        }
        case 'p': { // 'aa appdebug -p persist'
            TAG_LOGD(AAFwkTag::AA_TOOL, "'aa %{public}s -p' persist", cmd_.c_str());
            isPersist = true;
            return false;
        }
        case 'c': { // 'aa appdebug -c cancel'
            TAG_LOGD(AAFwkTag::AA_TOOL, "'aa %{public}s -c' cancel", cmd_.c_str());
            isCancel = true;
            return true;
        }
        case 'g': { // 'aa appdebug -g get'
            TAG_LOGD(AAFwkTag::AA_TOOL, "'aa %{public}s -g' get", cmd_.c_str());
            isGet = true;
            return true;
        }
        default: {
            break;
        }
    }
    return true;
}

bool AbilityManagerShellCommand::ParseAppDebugParameter(
    std::string &bundleName, bool &isPersist, bool &isCancel, bool &isGet)
{
    int32_t option = -1;
    int32_t counter = 0;
    while (true) {
        counter++;
        option = getopt_long(argc_, argv_, SHORT_OPTIONS_APPDEBUG.c_str(), LONG_OPTIONS_APPDEBUG, nullptr);

        if (optind < 0 || optind > argc_) {
            return false;
        }

        if (option == -1) {
            if (counter == 1 && strcmp(argv_[optind], cmd_.c_str()) == 0) {
                TAG_LOGE(AAFwkTag::AA_TOOL, "'aa %{public}s' %{public}s", HELP_MSG_NO_OPTION.c_str(), cmd_.c_str());
                resultReceiver_.append(HELP_MSG_NO_OPTION + "\n");
                return false;
            }
            return true;
        }

        if (option == '?') {
            switch (optopt) {
                case 'b': {
                    // 'aa appdebug -b' with no argument
                    TAG_LOGI(AAFwkTag::AA_TOOL, "'aa %{public}s -b' no arg", cmd_.c_str());
                    resultReceiver_.append("error: option requires a valid value.\n");
                    return false;
                }
                default: {
                    // 'aa appdebug' with an unknown option: aa appdebug -x
                    std::string unknownOption = "";
                    std::string unknownOptionMsg = GetUnknownOptionMsg(unknownOption);
                    TAG_LOGI(AAFwkTag::AA_TOOL, "'aa appdebug' option unknown");
                    resultReceiver_.append(unknownOptionMsg);
                    return false;
                }
            }
        }

        if (SwitchOptionForAppDebug(option, bundleName, isPersist, isCancel, isGet)) {
            return true;
        }
    }
    return false;
}

ErrCode AbilityManagerShellCommand::RunAsAppDebugDebugCommand()
{
    TAG_LOGD(AAFwkTag::AA_TOOL, "called");
    std::string bundleName;
    bool isPersist = false;
    bool isCancel = false;
    bool isGet = false;

    if (!system::GetBoolParameter(DEVELOPERMODE_STATE, false)) {
        resultReceiver_ = STRING_APP_DEBUG_NG + "\n";
        resultReceiver_.append(GetMessageFromCode(ERR_NOT_DEVELOPER_MODE));
        return OHOS::ERR_INVALID_OPERATION;
    }

    if (!ParseAppDebugParameter(bundleName, isPersist, isCancel, isGet)) {
        resultReceiver_.append(HELP_MSG_APPDEBUG_APP_DEBUG + "\n");
        return OHOS::ERR_INVALID_VALUE;
    }

    int32_t result = OHOS::ERR_OK;
    std::vector<std::string> debugInfoList;
    if (isGet) {
        result = DelayedSingleton<AppMgrClient>::GetInstance()->GetWaitingDebugApp(debugInfoList);
    } else if (isCancel) {
        result = DelayedSingleton<AppMgrClient>::GetInstance()->CancelAppWaitingDebug();
    } else if (!bundleName.empty()) {
        result = DelayedSingleton<AppMgrClient>::GetInstance()->SetAppWaitingDebug(bundleName, isPersist);
    } else {
        resultReceiver_.append(HELP_MSG_APPDEBUG_APP_DEBUG);
        return OHOS::ERR_OK;
    }

    if (result != OHOS::ERR_OK) {
        TAG_LOGI(AAFwkTag::AA_TOOL, "%{public}s result: %{public}d", STRING_APP_DEBUG_NG.c_str(), result);
        resultReceiver_ = STRING_APP_DEBUG_NG + "\n";
        resultReceiver_.append(GetMessageFromCode(result));
        return result;
    }
    resultReceiver_ = STRING_APP_DEBUG_OK + "\n";
    resultReceiver_.append(GetMessageFromCode(result));
    if (isGet && !debugInfoList.empty()) {
        for (auto it : debugInfoList) {
            resultReceiver_ += it + "\n";
        }
    }
    return OHOS::ERR_OK;
}

ErrCode AbilityManagerShellCommand::RunAsProcessCommand()
{
    Want want;
    ErrCode result = MakeWantForProcess(want);
    if (result == OHOS::ERR_OK) {
        auto appMgrClient = std::make_shared<AppMgrClient>();
        result = appMgrClient->StartNativeProcessForDebugger(want);
        if (result == OHOS::ERR_OK) {
            TAG_LOGI(AAFwkTag::AA_TOOL, "%{public}s", STRING_START_NATIVE_PROCESS_OK.c_str());
            resultReceiver_ = STRING_START_NATIVE_PROCESS_OK;
        } else {
            TAG_LOGI(
                AAFwkTag::AA_TOOL, "%{public}s result:%{public}d", STRING_START_NATIVE_PROCESS_NG.c_str(), result);
            resultReceiver_ = STRING_START_NATIVE_PROCESS_NG;
            resultReceiver_.append(GetMessageFromCode(result));
        }
    } else {
        resultReceiver_.append(HELP_MSG_PROCESS);
        result = OHOS::ERR_INVALID_VALUE;
    }

    return result;
}

ErrCode AbilityManagerShellCommand::RunAsSendMemoryLevelCommand()
{
    TAG_LOGD(AAFwkTag::AA_TOOL, "sendMemoryLevel");
    std::string pidParse = "";
    std::string memoryLevelParse = "";
    ParsePidMemoryLevel(pidParse, memoryLevelParse);
    if (pidParse.empty() || memoryLevelParse.empty()) {
        resultReceiver_.append(HELP_MSG_SEND_MEMORY_LEVEL + "\n");
        return OHOS::ERR_INVALID_VALUE;
    }

    pid_t inputPid = static_cast<pid_t>(ConvertPid(pidParse));
    MemoryLevel inputLevel = static_cast<MemoryLevel>(ConvertPid(memoryLevelParse));

    std::string appRunningUniqueId;
    ErrCode queryResult = DelayedSingleton<AppMgrClient>::GetInstance()->GetAppRunningUniqueIdByPid(
        inputPid, appRunningUniqueId);
    if (inputPid <= 0 || queryResult != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "pid value error. The specified pid does not exist.");
        resultReceiver_.append(STRING_SEND_MEMORY_LEVEL_NG + "\n");
        resultReceiver_.append(GetMessageFromCode(INVALID_PID));
        return ERR_INVALID_VALUE;
    }

    if (!(inputLevel >= OHOS::AppExecFwk::MemoryLevel::MEMORY_LEVEL_MODERATE &&
        inputLevel <= OHOS::AppExecFwk::MemoryLevel::MEMORY_LEVEL_BACKGROUND_CRITICAL)) {
        TAG_LOGE(AAFwkTag::APPMGR, "level value error. Valid values: 0-6");
        resultReceiver_.append(STRING_SEND_MEMORY_LEVEL_NG + "\n");
        resultReceiver_.append(GetMessageFromCode(INVALID_LEVEL));
        return ERR_INVALID_VALUE;
    }

    std::map<pid_t, MemoryLevel> pidMemoryLevelMap;
    pidMemoryLevelMap.emplace(inputPid, inputLevel);
    auto result = DelayedSingleton<AppMgrClient>::GetInstance()->NotifyProcMemoryLevel(pidMemoryLevelMap);
    if (result == OHOS::ERR_OK) {
        TAG_LOGI(AAFwkTag::AA_TOOL, "%{public}s", STRING_SEND_MEMORY_LEVEL_OK.c_str());
        resultReceiver_ = STRING_SEND_MEMORY_LEVEL_OK + "\n";
    } else {
        TAG_LOGI(AAFwkTag::AA_TOOL, "%{public}s result: %{public}d", STRING_SEND_MEMORY_LEVEL_NG.c_str(), result);
        resultReceiver_ = STRING_SEND_MEMORY_LEVEL_NG + "\n";
        resultReceiver_.append(GetMessageFromCode(result));
    }

    return result;
}

ErrCode AbilityManagerShellCommand::ParsePidMemoryLevel(std::string &pidParse, std::string &memoryLevelParse)
{
    int option = -1;
    int counter = 0;
    while (true) {
        counter++;
        option = getopt_long(argc_, argv_, SHORT_OPTIONS_SEND_MEMORY_LEVEL.c_str(),
            LONG_OPTIONS_SEND_MEMORY_LEVEL, nullptr);
        TAG_LOGD(AAFwkTag::AA_TOOL, "getopt_long option: %{public}d, optopt: %{public}d, optind: %{public}d",
            option, optopt, optind);
        
        if (optind < 0 || optind > argc_) {
            return OHOS::ERR_INVALID_VALUE;
        }
        // aa command without option
        if (option == -1) {
            if (counter == 1 && strcmp(argv_[optind], cmd_.c_str()) == 0) {
                resultReceiver_.append(HELP_MSG_NO_OPTION + "\n");
            }
            break;
        }

        switch (option) {
            case 'h':{
                TAG_LOGI(AAFwkTag::AA_TOOL, "'aa %{public}s -h' no arg", cmd_.c_str());
                // 'aa send-memory-level -h' no arg
                break;
            }
            case 'p':{
                TAG_LOGI(AAFwkTag::AA_TOOL, "'aa %{public}s -p' pid", cmd_.c_str());
                // 'aa send-memory-level -p pid'
                pidParse = optarg;
                break;
            }
            case 'l':{
                TAG_LOGI(AAFwkTag::AA_TOOL, "'aa %{public}s -l' level", cmd_.c_str());
                // 'aa send-memory-level -l level'
                memoryLevelParse = optarg;
                break;
            }
            case '?':{
                std::string unknownOption = "";
                std::string unknownOptionMsg = GetUnknownOptionMsg(unknownOption);
                resultReceiver_.append(unknownOptionMsg);
                break;
            }
            default:
                break;
        }
    }

    return OHOS::ERR_OK;
}

bool AbilityManagerShellCommand::MatchOrderString(const std::regex &regexScript, const std::string &orderCmd)
{
    TAG_LOGD(AAFwkTag::AA_TOOL, "orderCmd: %{public}s", orderCmd.c_str());
    if (orderCmd.empty()) {
        TAG_LOGE(AAFwkTag::AA_TOOL, "empty orderCmd");
        return false;
    }

    std::match_results<std::string::const_iterator> matchResults;
    try {
        if (!std::regex_match(orderCmd, matchResults, regexScript)) {
            TAG_LOGE(AAFwkTag::AA_TOOL, "order mismatch");
            return false;
        }
    } catch (...) {
        TAG_LOGE(AAFwkTag::AA_TOOL, "regex failed");
        return false;
    }
    return true;
}

bool AbilityManagerShellCommand::CheckPerfCmdString(
    const char* optarg, const size_t paramLength, std::string &perfCmd)
{
    if (optarg == nullptr) {
        TAG_LOGE(AAFwkTag::AA_TOOL, "null optarg");
        return false;
    }

    if (strlen(optarg) >= paramLength) {
        TAG_LOGE(AAFwkTag::AA_TOOL, "debuggablePipe aa start -p param length must <1024");
        return false;
    }

    perfCmd = optarg;
    const std::regex regexDumpHeapType(R"(^\s*(dumpheap)\s*$)");
    const std::regex regexSleepType(R"(^\s*(sleep)((\s+\d*)|)\s*$)");
    if (MatchOrderString(regexDumpHeapType, perfCmd) || MatchOrderString(regexSleepType, perfCmd)) {
        return true;
    }

    TAG_LOGD(AAFwkTag::AA_TOOL, "command mismatch");
    const std::regex regexProfileType(R"(^\s*(profile)\s+(nativeperf|jsperf)(\s+.*|$))");
    if (!MatchOrderString(regexProfileType, perfCmd)) {
        TAG_LOGE(AAFwkTag::AA_TOOL, "invalid command");
        return false;
    }

    auto findPos = perfCmd.find("jsperf");
    if (findPos != std::string::npos) {
        const std::regex regexCmd(R"(^jsperf($|\s+($|((5000|([1-9]|[1-4]\d)\d\d)|)\s*($|nativeperf.*))))");
        if (!MatchOrderString(regexCmd, perfCmd.substr(findPos, perfCmd.length() - findPos))) {
            TAG_LOGE(AAFwkTag::AA_TOOL, "invalid order");
            return false;
        }
    }
    return true;
}

bool AbilityManagerShellCommand::IsLongStartOption(const std::string &argv)
{
    if (argv.find("--") != 0) {
        return false;
    }
    static std::vector<std::string> longOptions;
    if (longOptions.empty()) {
        for (const auto &longOpt : LONG_OPTIONS) {
            if (longOpt.name == nullptr) {
                break;
            }
            longOptions.emplace_back(std::string("--") + longOpt.name);
        }
    }
    for (const std::string &longOption : longOptions) {
        if (argv.find(longOption) == 0) {
            return true;
        }
    }
    return false;
}

bool AbilityManagerShellCommand::IsShortStartOption(const std::string &argv)
{
    std::string shortOption = "-c";
    for (char c : SHORT_OPTION_CHARS) {
        shortOption[SHORT_OPTION_INDEX] = c;
        if (argv.find(shortOption) == 0) {
            return true;
        }
    }
    return false;
}

bool AbilityManagerShellCommand::IsStartOption(const std::string &argv)
{
    if (argv.empty() || (argv.find("-") != 0 && argv.find("--") != 0)) {
        return false;
    }
    if (IsLongStartOption(argv)) {
        return true;
    }
    return IsShortStartOption(argv);
}

bool AbilityManagerShellCommand::CheckParameters(int extraArguments)
{
    if (optind + extraArguments >= argc_) return false;
    int index = optind + 1; // optind is the index of 'start' which is right behind optarg
    int count = 0;
    while (index < argc_ && !IsStartOption(argv_[index])) {
        count++;
        index++;
    }
    return count == extraArguments;
}

// parse integer parameters
ErrCode AbilityManagerShellCommand::ParseParam(ParametersInteger& pi)
{
    std::string key = optarg;
    char *tmp = argv_[optind + OPTION_PARAMETER_VALUE_OFFSET];
    std::string intString = tmp;
    argv_[optind + OPTION_PARAMETER_VALUE_OFFSET] = argv_[optind];
    argv_[optind] = tmp;
    ++optind;
    int32_t intValue = 0;
    auto res = std::from_chars(intString.c_str(), intString.c_str() + intString.size(), intValue);
    if (res.ec != std::errc() || res.ptr != intString.c_str() + intString.size()) {
        resultReceiver_.append("invalid parameter ");
        resultReceiver_.append(intString);
        resultReceiver_.append(" for integer option\n");

        return OHOS::ERR_INVALID_VALUE;
    }
    pi[key] = intValue;
    return OHOS::ERR_OK;
}

// parse string parameters
ErrCode AbilityManagerShellCommand::ParseParam(ParametersString& ps, bool isNull = false)
{
    std::string key = optarg;
    std::string value = "";
    if (!isNull) {
        char *tmp = argv_[optind + OPTION_PARAMETER_VALUE_OFFSET];
        value = tmp;
        argv_[optind + OPTION_PARAMETER_VALUE_OFFSET] = argv_[optind];
        argv_[optind] = tmp;
        ++optind;
    }

    ps[key] = value;

    return OHOS::ERR_OK;
}

// parse bool parameters
ErrCode AbilityManagerShellCommand::ParseParam(ParametersBool& pb)
{
    std::string key = optarg;
    char *tmp = argv_[optind + OPTION_PARAMETER_VALUE_OFFSET];
    std::string boolString = tmp;
    argv_[optind + OPTION_PARAMETER_VALUE_OFFSET] = argv_[optind];
    argv_[optind] = tmp;
    ++optind;
    std::transform(boolString.begin(), boolString.end(), boolString.begin(), ::tolower);
    bool value;
    if (boolString == "true" || boolString == "t") {
        value = true;
    } else if (boolString == "false" || boolString == "f") {
        value = false;
    } else {
        resultReceiver_.append("invalid parameter ");
        resultReceiver_.append(boolString);
        resultReceiver_.append(" for bool option\n");

        return OHOS::ERR_INVALID_VALUE;
    }

    pb[key] = value;

    return OHOS::ERR_OK;
}

void AbilityManagerShellCommand::SetParams(const ParametersInteger& pi, Want& want)
{
    for (auto it = pi.begin(); it != pi.end(); it++) {
        want.SetParam(it->first, it->second);
    }
}

void AbilityManagerShellCommand::SetParams(const ParametersString& ps, Want& want)
{
    for (auto it = ps.begin(); it != ps.end(); it++) {
        want.SetParam(it->first, it->second);
    }
}

void AbilityManagerShellCommand::SetParams(const ParametersBool& pb, Want& want)
{
    for (auto it = pb.begin(); it != pb.end(); it++) {
        want.SetParam(it->first, it->second);
    }
}

void AddEntities(const std::vector<std::string>& entities, Want& want)
{
    for (auto entity : entities)
        want.AddEntity(entity);
}

ErrCode AbilityManagerShellCommand::MakeWantForProcess(Want& want)
{
    int result = OHOS::ERR_OK;
    int option = -1;
    int counter = 0;
    std::string deviceId = "";
    std::string bundleName = "";
    std::string abilityName = "";
    std::string moduleName = "";
    std::string perfCmd = "";
    std::string debugCmd = "";
    bool isPerf = false;
    bool isSandboxApp = false;

    while (true) {
        counter++;

        option = getopt_long(argc_, argv_, SHORT_OPTIONS_PROCESS.c_str(), LONG_OPTIONS_PROCESS, nullptr);

        TAG_LOGI(
            AAFwkTag::AA_TOOL, "option: %{public}d, optopt: %{public}d, optind: %{public}d", option, optopt, optind);

        if (optind < 0 || optind > argc_) {
            return OHOS::ERR_INVALID_VALUE;
        }

        if (option == -1) {
            // When scanning the first argument
            if (counter == 1 && strcmp(argv_[optind], cmd_.c_str()) == 0) {
                // 'aa process' with no option: aa process
                // 'aa process' with a wrong argument: aa process xxx
                TAG_LOGI(AAFwkTag::AA_TOOL, "'aa %{public}s' %{public}s!", HELP_MSG_NO_OPTION.c_str(), cmd_.c_str());

                resultReceiver_.append(HELP_MSG_NO_OPTION + "\n");
                result = OHOS::ERR_INVALID_VALUE;
            }
            break;
        }

        if (option == '?') {
            switch (optopt) {
                case 'a': {
                    // 'aa process -a' with no argument
                    TAG_LOGI(AAFwkTag::AA_TOOL, "'aa %{public}s -a' no arg", cmd_.c_str());

                    resultReceiver_.append("error: option ");
                    resultReceiver_.append("requires a value.\n");

                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
                case 'b': {
                    // 'aa process -b' with no argument
                    TAG_LOGI(AAFwkTag::AA_TOOL, "'aa %{public}s -b' no arg", cmd_.c_str());

                    resultReceiver_.append("error: option ");
                    resultReceiver_.append("requires a value.\n");

                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
                case 'm': {
                    // 'aa process -m' with no argument
                    TAG_LOGI(AAFwkTag::AA_TOOL, "'aa %{public}s -m' no arg", cmd_.c_str());

                    resultReceiver_.append("error: option ");
                    resultReceiver_.append("requires a value.\n");

                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
                case 'p': {
                    // 'aa process -p' with no argument
                    TAG_LOGI(AAFwkTag::AA_TOOL, "'aa %{public}s -p' no arg", cmd_.c_str());

                    resultReceiver_.append("error: option ");
                    resultReceiver_.append("requires a value.\n");

                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
                case 'D': {
                    // 'aa process -D' with no argument
                    TAG_LOGI(AAFwkTag::AA_TOOL, "'aa %{public}s -D' no arg", cmd_.c_str());

                    resultReceiver_.append("error: option ");
                    resultReceiver_.append("requires a value.\n");

                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
                default: {
                    // 'aa process' with an unknown option: aa process -x
                    // 'aa process' with an unknown option: aa process -xxx
                    std::string unknownOption = "";
                    std::string unknownOptionMsg = GetUnknownOptionMsg(unknownOption);

                    TAG_LOGI(AAFwkTag::AA_TOOL, "'aa %{public}s' opt unknown", cmd_.c_str());

                    resultReceiver_.append(unknownOptionMsg);
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
            }
            break;
        }

        switch (option) {
            case 'h': {
                // 'aa process -h'
                // 'aa process --help'
                result = OHOS::ERR_INVALID_VALUE;
                break;
            }
            case 'a': {
                // 'aa process -a xxx'
                // save ability name
                abilityName = optarg;
                break;
            }
            case 'b': {
                // 'aa process -b xxx'
                // save bundle name
                bundleName = optarg;
                break;
            }
            case 'm': {
                // 'aa process -m xxx'
                // save module name
                moduleName = optarg;
                break;
            }
            case 'p': {
                // 'aa process -p xxx'
                // save perf cmd
                if (strlen(optarg) < PARAM_LENGTH) {
                    perfCmd = optarg;
                    isPerf = true;
                }
                break;
            }
            case 'D': {
                // 'aa process -D xxx'
                // save debug cmd
                if (!isPerf && strlen(optarg) < PARAM_LENGTH) {
                    TAG_LOGI(AAFwkTag::AA_TOOL, "debug cmd");
                    debugCmd = optarg;
                }
                break;
            }
            case 'S': {
                // 'aa process -S'
                // enter sandbox to perform app
                isSandboxApp = true;
                break;
            }
            default: {
                break;
            }
        }
    }

    if (result == OHOS::ERR_OK) {
        if (perfCmd.empty() && debugCmd.empty()) {
            TAG_LOGI(AAFwkTag::AA_TOOL,
                "debuggablePipe aa process must contains -p or -D and param length must <1024");
            return OHOS::ERR_INVALID_VALUE;
        }

        if (abilityName.size() == 0 || bundleName.size() == 0) {
            // 'aa process -a <ability-name> -b <bundle-name> [-D]'
            TAG_LOGI(AAFwkTag::AA_TOOL, "'aa %{public}s' without enough options", cmd_.c_str());

            if (abilityName.size() == 0) {
                resultReceiver_.append(HELP_MSG_NO_ABILITY_NAME_OPTION + "\n");
            }

            if (bundleName.size() == 0) {
                resultReceiver_.append(HELP_MSG_NO_BUNDLE_NAME_OPTION + "\n");
            }

            result = OHOS::ERR_INVALID_VALUE;
        } else {
            ElementName element(deviceId, bundleName, abilityName, moduleName);
            want.SetElement(element);

            if (!perfCmd.empty()) {
                want.SetParam("perfCmd", perfCmd);
            }
            if (!debugCmd.empty()) {
                want.SetParam("debugCmd", debugCmd);
            }
            if (isSandboxApp) {
                want.SetParam("sandboxApp", isSandboxApp);
            }
        }
    }

    return result;
}

void AbilityManagerShellCommand::ParseBundleName(std::string &bundleName)
{
    int option = -1;
    int counter = 0;

    while (true) {
        counter++;
        option = getopt_long(argc_, argv_, SHORT_OPTIONS_ATTACH.c_str(), LONG_OPTIONS_ATTACH, nullptr);
        TAG_LOGD(AAFwkTag::AA_TOOL, "getopt_long option: %{public}d, optopt: %{public}d, optind: %{public}d", option,
            optopt, optind);

        if (optind < 0 || optind > argc_) {
            break;
        }

        if (option == -1) {
            // aa command without option
            if (counter == 1 && strcmp(argv_[optind], cmd_.c_str()) == 0) {
                resultReceiver_.append(HELP_MSG_NO_OPTION + "\n");
            }
            break;
        }

        if (option == '?') {
            switch (optopt) {
                case 'b':
                case 'h':
                    break;
                default: {
                    // 'aa attach/detach' with an unknown option
                    std::string unknownOption = "";
                    std::string unknownOptionMsg = GetUnknownOptionMsg(unknownOption);
                    resultReceiver_.append(unknownOptionMsg);
                    break;
                }
            }
            break;
        }

        switch (option) {
            case 'b': {
                bundleName = optarg;
                break;
            }
            default:
                break;
        }
    }
}

#ifdef ABILITY_COMMAND_FOR_TEST
ErrCode AbilityManagerShellCommand::RunForceTimeoutForTest()
{
    TAG_LOGI(AAFwkTag::AA_TOOL, "[%{public}s(%{public}s)] enter", __FILE__, __FUNCTION__);
    if (argList_.empty()) {
        resultReceiver_.append(HELP_MSG_FORCE_TIMEOUT + "\n");
        return OHOS::ERR_INVALID_VALUE;
    }

    ErrCode result = OHOS::ERR_OK;
    if (argList_.size() == NUMBER_ONE && argList_[0] == HELP_MSG_FORCE_TIMEOUT_CLEAN) {
        TAG_LOGI(AAFwkTag::AA_TOOL, "clear ability timeout flags");
        result = AbilityManagerClient::GetInstance()->ForceTimeoutForTest(argList_[0], "");
    } else if (argList_.size() == NUMBER_TWO) {
        TAG_LOGI(AAFwkTag::AA_TOOL, "Ability name : %{public}s, state: %{public}s", argList_[0].c_str(),
            argList_[1].c_str());
        result = AbilityManagerClient::GetInstance()->ForceTimeoutForTest(argList_[0], argList_[1]);
    } else {
        resultReceiver_.append(HELP_MSG_FORCE_TIMEOUT + "\n");
        return OHOS::ERR_INVALID_VALUE;
    }
    if (result == OHOS::ERR_OK) {
        TAG_LOGI(AAFwkTag::AA_TOOL, "%{public}s", STRING_FORCE_TIMEOUT_OK.c_str());
        resultReceiver_ = STRING_FORCE_TIMEOUT_OK + "\n";
    } else {
        TAG_LOGI(AAFwkTag::AA_TOOL, "%{public}s result: %{public}d", STRING_FORCE_TIMEOUT_NG.c_str(), result);
        resultReceiver_ = STRING_FORCE_TIMEOUT_NG + "\n";
        resultReceiver_.append(GetMessageFromCode(result));
    }
    return result;
}
#endif

ErrCode AbilityManagerShellCommand::MakeWantFromCmd(Want& want, std::string& windowMode)
{
    int result = OHOS::ERR_OK;

    int option = -1;
    int counter = 0;

    std::string deviceId = "";
    std::string bundleName = "";
    std::string abilityName = "";
    std::string moduleName;
    std::string perfCmd;
    ParametersInteger parametersInteger;
    ParametersString parametersString;
    ParametersBool parametersBool;
    std::string uri;
    std::string action;
    std::vector<std::string> entities;
    std::string typeVal;
    bool isColdStart = false;
    bool isDebugApp = false;
    bool isErrorInfoEnhance = false;
    bool isContinuation = false;
    bool isSandboxApp = false;
    bool isNativeDebug = false;
    bool isMultiThread = false;
    int windowLeft = 0;
    bool hasWindowLeft = false;
    int windowTop = 0;
    bool hasWindowTop = false;
    int windowHeight = 0;
    bool hasWindowHeight = false;
    int windowWidth = 0;
    bool hasWindowWidth = false;

    while (true) {
        counter++;

        option = getopt_long(argc_, argv_, SHORT_OPTIONS.c_str(), LONG_OPTIONS, nullptr);

        TAG_LOGI(
            AAFwkTag::AA_TOOL, "option: %{public}d, optopt: %{public}d, optind: %{public}d", option, optopt, optind);

        if (optind < 0 || optind > argc_) {
            return OHOS::ERR_INVALID_VALUE;
        }

        if (option == -1) {
            // When scanning the first argument
            if (counter == 1 && strcmp(argv_[optind], cmd_.c_str()) == 0) {
                // 'aa start' with no option: aa start
                // 'aa start' with a wrong argument: aa start xxx
                // 'aa stop-service' with no option: aa stop-service
                // 'aa stop-service' with a wrong argument: aa stop-service xxx
                TAG_LOGI(AAFwkTag::AA_TOOL, "'aa %{public}s' %{public}s", HELP_MSG_NO_OPTION.c_str(), cmd_.c_str());

                resultReceiver_.append(HELP_MSG_NO_OPTION + "\n");
                result = OHOS::ERR_INVALID_VALUE;
            }
            break;
        }

        if (option == '?') {
            switch (optopt) {
                case 'h': {
                    // 'aa start -h'
                    // 'aa stop-service -h'
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
                case 'd': {
                    // 'aa start -d' with no argument
                    // 'aa stop-service -d' with no argument
                    TAG_LOGI(AAFwkTag::AA_TOOL, "'aa %{public}s -d' no arg", cmd_.c_str());

                    resultReceiver_.append("error: option ");
                    resultReceiver_.append("requires a value.\n");

                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
                case 'a': {
                    // 'aa start -a' with no argument
                    // 'aa stop-service -a' with no argument
                    TAG_LOGI(AAFwkTag::AA_TOOL, "'aa %{public}s -a' no arg", cmd_.c_str());

                    resultReceiver_.append("error: option ");
                    resultReceiver_.append("requires a value.\n");

                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
                case 'b': {
                    // 'aa start -b' with no argument
                    // 'aa stop-service -b' with no argument
                    TAG_LOGI(AAFwkTag::AA_TOOL, "'aa %{public}s -b' no arg", cmd_.c_str());

                    resultReceiver_.append("error: option ");
                    resultReceiver_.append("requires a value.\n");

                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
                case 'e': {
                    // 'aa start -e' with no argument
                    TAG_LOGI(AAFwkTag::AA_TOOL, "'aa %{public}s -e no arg", cmd_.c_str());

                    resultReceiver_.append("error: option ");
                    resultReceiver_.append("requires a value.\n");

                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
                case 't': {
                    // 'aa start -t' with no argument
                    TAG_LOGI(AAFwkTag::AA_TOOL, "'aa %{public}s -t no arg", cmd_.c_str());

                    resultReceiver_.append("error: option ");
                    resultReceiver_.append("requires a value.\n");

                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
                case 's': {
                    // 'aa start -s' with no argument
                    // 'aa stop-service -s' with no argument
                    TAG_LOGI(AAFwkTag::AA_TOOL, "'aa %{public}s -s' no arg", cmd_.c_str());

                    resultReceiver_.append("error: option ");
                    resultReceiver_.append("requires a value.\n");

                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
                case 'm': {
                    // 'aa start -m' with no argument
                    // 'aa stop-service -m' with no argument
                    TAG_LOGI(AAFwkTag::AA_TOOL, "'aa %{public}s -m' no arg", cmd_.c_str());

                    resultReceiver_.append("error: option ");
                    resultReceiver_.append("requires a value.\n");

                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
                case 'p': {
                    // 'aa start -p' with no argument
                    // 'aa stop-service -p' with no argument
                    TAG_LOGI(AAFwkTag::AA_TOOL, "'aa %{public}s -p' no arg", cmd_.c_str());

                    resultReceiver_.append("error: option ");
                    resultReceiver_.append("requires a value.\n");

                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
                case OPTION_PARAMETER_INTEGER: {
                    // 'aa start --pi' with no argument
                    TAG_LOGI(AAFwkTag::AA_TOOL, "'aa %{public}s --pi' no arg", cmd_.c_str());

                    resultReceiver_.append("error: option ");
                    resultReceiver_.append("requires a value.\n");

                    result = OHOS::ERR_INVALID_VALUE;

                    break;
                }
                case OPTION_PARAMETER_STRING: {
                    // 'aa start --ps' with no argument
                    TAG_LOGI(AAFwkTag::AA_TOOL, "'aa %{public}s --ps' no arg", cmd_.c_str());

                    resultReceiver_.append("error: option ");
                    resultReceiver_.append("requires a value.\n");

                    result = OHOS::ERR_INVALID_VALUE;

                    break;
                }
                case OPTION_PARAMETER_BOOL: {
                    // 'aa start --pb' with no argument
                    TAG_LOGI(AAFwkTag::AA_TOOL, "'aa %{public}s -pb' no arg", cmd_.c_str());

                    resultReceiver_.append("error: option ");
                    resultReceiver_.append("requires a value.\n");

                    result = OHOS::ERR_INVALID_VALUE;

                    break;
                }
                case OPTION_PARAMETER_NULL_STRING: {
                    // 'aa start --psn' with no argument
                    TAG_LOGI(AAFwkTag::AA_TOOL, "'aa %{public}s --psn' no arg", cmd_.c_str());

                    resultReceiver_.append("error: option ");
                    resultReceiver_.append("requires a value.\n");

                    result = OHOS::ERR_INVALID_VALUE;

                    break;
                }
                case OPTION_WINDOW_LEFT: {
                    // 'aa start --wl' with no argument
                    TAG_LOGI(AAFwkTag::AA_TOOL, "'aa %{public}s --wl' no arg", cmd_.c_str());

                    resultReceiver_.append("error: option ");
                    resultReceiver_.append("requires a value.\n");

                    result = OHOS::ERR_INVALID_VALUE;

                    break;
                }
                case OPTION_WINDOW_TOP: {
                    // 'aa start --wt' with no argument
                    TAG_LOGI(AAFwkTag::AA_TOOL, "'aa %{public}s --wt' no arg", cmd_.c_str());

                    resultReceiver_.append("error: option ");
                    resultReceiver_.append("requires a value.\n");

                    result = OHOS::ERR_INVALID_VALUE;

                    break;
                }
                case OPTION_WINDOW_HEIGHT: {
                    // 'aa start --wh' with no argument
                    TAG_LOGI(AAFwkTag::AA_TOOL, "'aa %{public}s --wh' no arg", cmd_.c_str());

                    resultReceiver_.append("error: option ");
                    resultReceiver_.append("requires a value.\n");

                    result = OHOS::ERR_INVALID_VALUE;

                    break;
                }
                case OPTION_WINDOW_WIDTH: {
                    // 'aa start --ww' with no argument
                    TAG_LOGI(AAFwkTag::AA_TOOL, "'aa %{public}s --ww' no arg", cmd_.c_str());

                    resultReceiver_.append("error: option ");
                    resultReceiver_.append("requires a value.\n");

                    result = OHOS::ERR_INVALID_VALUE;

                    break;
                }

                case 'A': {
                    // 'aa start -A' with no argument
                    TAG_LOGI(AAFwkTag::AA_TOOL, "'aa %{public}s -A' no arg", cmd_.c_str());

                    resultReceiver_.append("error: option ");
                    resultReceiver_.append("requires a value.\n");

                    result = OHOS::ERR_INVALID_VALUE;

                    break;
                }
                case 'U': {
                    // 'aa start -U' with no argument
                    TAG_LOGI(AAFwkTag::AA_TOOL, "'aa %{public}s -U' no arg", cmd_.c_str());

                    resultReceiver_.append("error: option ");
                    resultReceiver_.append("requires a value.\n");

                    result = OHOS::ERR_INVALID_VALUE;

                    break;
                }
                case 'W': {
                    // 'aa start -W' with no argument
                    startAbilityWithWaitFlag_ = true;
                    break;
                }
                case 0: {
                    // 'aa start' with an unknown option: aa start --x
                    // 'aa start' with an unknown option: aa start --xxx
                    // 'aa stop-service' with an unknown option: aa stop-service --x
                    // 'aa stop-service' with an unknown option: aa stop-service --xxx
                    std::string unknownOption = "";
                    std::string unknownOptionMsg = GetUnknownOptionMsg(unknownOption);

                    TAG_LOGI(AAFwkTag::AA_TOOL, "'aa %{public}s' opt unknown", cmd_.c_str());

                    resultReceiver_.append(unknownOptionMsg);
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
                default: {
                    // 'aa start' with an unknown option: aa start -x
                    // 'aa start' with an unknown option: aa start -xxx
                    // 'aa stop-service' with an unknown option: aa stop-service -x
                    // 'aa stop-service' with an unknown option: aa stop-service -xxx
                    std::string unknownOption = "";
                    std::string unknownOptionMsg = GetUnknownOptionMsg(unknownOption);

                    TAG_LOGI(AAFwkTag::AA_TOOL, "'aa %{public}s' opt unknown", cmd_.c_str());

                    resultReceiver_.append(unknownOptionMsg);
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
            }
            break;
        }

        switch (option) {
            case 'h': {
                // 'aa start -h'
                // 'aa start --help'
                // 'aa stop-service -h'
                // 'aa stop-service --help'
                result = OHOS::ERR_INVALID_VALUE;
                break;
            }
            case 'd': {
                // 'aa start -d xxx'
                // 'aa stop-service -d xxx'

                // save device ID
                if (optarg != nullptr) {
                    deviceId = optarg;
                }
                break;
            }
            case 'a': {
                // 'aa start -a xxx'
                // 'aa stop-service -a xxx'

                // save ability name
                abilityName = optarg;
                break;
            }
            case 'b': {
                // 'aa start -b xxx'
                // 'aa stop-service -b xxx'

                // save bundle name
                bundleName = optarg;
                break;
            }
            case 'e': {
                // 'aa start -e xxx'

                // save entity
                entities.push_back(optarg);
                break;
            }
            case 't': {
                // 'aa start -t xxx'

                // save type
                typeVal = optarg;
                break;
            }
            case 's': {
                // 'aa start -s xxx'
                // save windowMode
                windowMode = optarg;
                break;
            }
            case 'm': {
                // 'aa start -m xxx'
                // 'aa stop-service -m xxx'

                // save module name
                moduleName = optarg;
                break;
            }
            case 'p': {
                // 'aa start -p xxx'
                // 'aa stop-service -p xxx'

                // save module name
                if (!CheckPerfCmdString(optarg, PARAM_LENGTH, perfCmd)) {
                    TAG_LOGE(AAFwkTag::AA_TOOL, "input perfCmd invalid %{public}s", perfCmd.c_str());
                    result = OHOS::ERR_INVALID_VALUE;
                }
                break;
            }
            case OPTION_PARAMETER_INTEGER: {
                // 'aa start --pi xxx'
                if (!CheckParameters(EXTRA_ARGUMENTS_FOR_KEY_VALUE_PAIR)) {
                    resultReceiver_.append("invalid number of parameters for option --pi\n");
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }

                // parse option arguments into a key-value map
                result = ParseParam(parametersInteger);

                break;
            }
            case OPTION_PARAMETER_STRING: {
                // 'aa start --ps xxx'
                if (!CheckParameters(EXTRA_ARGUMENTS_FOR_KEY_VALUE_PAIR)) {
                    resultReceiver_.append("invalid number of parameters for option --ps\n");
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }

                // parse option arguments into a key-value map
                result = ParseParam(parametersString);

                break;
            }
            case OPTION_PARAMETER_BOOL: {
                // 'aa start --pb xxx'
                if (!CheckParameters(EXTRA_ARGUMENTS_FOR_KEY_VALUE_PAIR)) {
                    resultReceiver_.append("invalid number of parameters for option --pb\n");
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }

                // parse option arguments into a key-value map
                result = ParseParam(parametersBool);

                break;
            }
            case OPTION_PARAMETER_NULL_STRING: {
                // 'aa start --psn xxx'
                if (!CheckParameters(EXTRA_ARGUMENTS_FOR_NULL_STRING)) {
                    resultReceiver_.append("invalid number of parameters for option --psn\n");
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }

                // parse option arguments into a key-value map
                result = ParseParam(parametersString, true);

                break;
            }
            case OPTION_WINDOW_LEFT: {
                // 'aa start --wl xxx'
                if (!std::regex_match(optarg, std::regex(STRING_REGEX_ALL_NUMBERS))) {
                    resultReceiver_.append("invalid argument for option --wl\n");
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
                windowLeft = int(atof(optarg));
                hasWindowLeft = true;
                TAG_LOGI(AAFwkTag::AA_TOOL, "windowLeft=%{public}d", windowLeft);

                break;
            }
            case OPTION_WINDOW_TOP: {
                // 'aa start --wt xxx'
                if (!std::regex_match(optarg, std::regex(STRING_REGEX_ALL_NUMBERS))) {
                    resultReceiver_.append("invalid argument for option --wt\n");
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
                windowTop = int(atof(optarg));
                hasWindowTop = true;
                TAG_LOGI(AAFwkTag::AA_TOOL, "windowTop=%{public}d", windowTop);

                break;
            }
            case OPTION_WINDOW_HEIGHT: {
                // 'aa start --wh xxx'
                if (!std::regex_match(optarg, std::regex(STRING_REGEX_ALL_NUMBERS))) {
                    resultReceiver_.append("invalid argument for option --wh\n");
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
                windowHeight = int(atof(optarg));
                hasWindowHeight = true;
                TAG_LOGI(AAFwkTag::AA_TOOL, "windowHeight=%{public}d", windowHeight);

                break;
            }
            case OPTION_WINDOW_WIDTH: {
                // 'aa start --ww xxx'
                if (!std::regex_match(optarg, std::regex(STRING_REGEX_ALL_NUMBERS))) {
                    resultReceiver_.append("invalid argument for option --ww\n");
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
                windowWidth = int(atof(optarg));
                hasWindowWidth = true;
                TAG_LOGI(AAFwkTag::AA_TOOL, "windowWidth=%{public}d", windowWidth);

                break;
            }
            case 'U': {
                // 'aa start -U xxx'

                // save URI
                uri = optarg;
                break;
            }
            case 'A': {
                // 'aa start -A xxx'

                // save action
                action = optarg;
                break;
            }
            case 'C': {
                // 'aa start -C'
                // cold start app
                isColdStart = true;
                break;
            }
            case 'D': {
                // 'aa start -D'
                // debug app
                isDebugApp = true;
                break;
            }
            case 'E': {
                // 'aa start -E'
                // error info enhance
                isErrorInfoEnhance = true;
                TAG_LOGD(AAFwkTag::AA_TOOL, "isErrorInfoEnhance");
                break;
            }
            case 'S': {
                // 'aa start -b <bundleName> -a <abilityName> -p <perf-cmd> -S'
                // enter sandbox to perform app
                isSandboxApp = true;
                break;
            }
            case 'c': {
                // 'aa start -c'
                // set ability launch reason = continuation
                isContinuation = true;
                break;
            }
            case 'N': {
                // 'aa start -N'
                // wait for debug in appspawn
                isNativeDebug = true;
                break;
            }
            case 'R': {
                // 'aa start -R'
                // app multi thread
                isMultiThread = true;
                TAG_LOGD(AAFwkTag::AA_TOOL, "isMultiThread");
                break;
            }
            case 0: {
                // 'aa start' with an unknown option: aa start -x
                // 'aa start' with an unknown option: aa start -xxx
                break;
            }
            default: {
                break;
            }
        }
    }

    if (result == OHOS::ERR_OK) {
        if (!abilityName.empty() && bundleName.empty()) {
            // explicitly start ability must have both ability and bundle names

            // 'aa start [-d <device-id>] -a <ability-name> -b <bundle-name> [-D]'
            // 'aa stop-service [-d <device-id>] -a <ability-name> -b <bundle-name>'
            TAG_LOGI(AAFwkTag::AA_TOOL, "'aa %{public}s' without enough options", cmd_.c_str());

            resultReceiver_.append(HELP_MSG_NO_BUNDLE_NAME_OPTION + "\n");
            result = OHOS::ERR_INVALID_VALUE;
        } else {
            ElementName element(deviceId, bundleName, abilityName, moduleName);
            want.SetElement(element);

            if (isColdStart) {
                want.SetParam("coldStart", isColdStart);
            }
            if (isDebugApp) {
                want.SetParam("debugApp", isDebugApp);
            }
            if (isContinuation) {
                want.AddFlags(Want::FLAG_ABILITY_CONTINUATION);
            }
            if (!perfCmd.empty()) {
                want.SetParam("perfCmd", perfCmd);
            }
            if (isSandboxApp) {
                want.SetParam("sandboxApp", isSandboxApp);
            }
            if (isNativeDebug) {
                want.SetParam("nativeDebug", isNativeDebug);
            }
            if (!parametersInteger.empty()) {
                SetParams(parametersInteger, want);
            }
            if (!parametersBool.empty()) {
                SetParams(parametersBool, want);
            }
            if (!parametersString.empty()) {
                SetParams(parametersString, want);
            }
            if (!action.empty()) {
                want.SetAction(action);
            }
            if (!uri.empty()) {
                want.SetUri(uri);
            }
            if (!entities.empty()) {
                AddEntities(entities, want);
            }
            if (!typeVal.empty()) {
                want.SetType(typeVal);
            }
            if (isErrorInfoEnhance) {
                want.SetParam("errorInfoEnhance", isErrorInfoEnhance);
            }
            if (isMultiThread) {
                want.SetParam("multiThread", isMultiThread);
            }
            if (hasWindowLeft) {
                want.SetParam(Want::PARAM_RESV_WINDOW_LEFT, windowLeft);
            }
            if (hasWindowTop) {
                want.SetParam(Want::PARAM_RESV_WINDOW_TOP, windowTop);
            }
            if (hasWindowHeight) {
                want.SetParam(Want::PARAM_RESV_WINDOW_HEIGHT, windowHeight);
            }
            if (hasWindowWidth) {
                want.SetParam(Want::PARAM_RESV_WINDOW_WIDTH, windowWidth);
            }
        }
    }

    return result;
}

ErrCode AbilityManagerShellCommand::MakeWantFromCmdForStopService(Want& want)
{
    int result = OHOS::ERR_OK;
    int option = -1;
    int counter = 0;
    std::string deviceId = "";
    std::string bundleName = "";
    std::string abilityName = "";
    std::string moduleName;

    while (true) {
        counter++;

        option = getopt_long(argc_, argv_, SHORT_OPTIONS.c_str(), LONG_OPTIONS, nullptr);

        TAG_LOGI(
            AAFwkTag::AA_TOOL, "option: %{public}d, optopt: %{public}d, optind: %{public}d", option, optopt, optind);

        if (optind < 0 || optind > argc_) {
            return OHOS::ERR_INVALID_VALUE;
        }

        if (option == -1) {
            // When scanning the first argument
            if (counter == 1 && strcmp(argv_[optind], cmd_.c_str()) == 0) {
                // 'aa stop-service' with no option: aa stop-service
                // 'aa stop-service' with a wrong argument: aa stop-service xxx
                TAG_LOGI(AAFwkTag::AA_TOOL, "'aa %{public}s' %{public}s", HELP_MSG_NO_OPTION.c_str(), cmd_.c_str());

                resultReceiver_.append(HELP_MSG_NO_OPTION + "\n");
                result = OHOS::ERR_INVALID_VALUE;
            }
            break;
        }

        if (option == '?') {
            switch (optopt) {
                case 'h': {
                    // 'aa stop-service -h'
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
                case 'd': {
                    // 'aa stop-service -d' with no argument
                    TAG_LOGI(AAFwkTag::AA_TOOL, "'aa %{public}s -d' no arg", cmd_.c_str());

                    resultReceiver_.append("error: option ");
                    resultReceiver_.append("requires a value.\n");

                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
                case 'a': {
                    // 'aa stop-service -a' with no argument
                    TAG_LOGI(AAFwkTag::AA_TOOL, "'aa %{public}s -a' no arg", cmd_.c_str());

                    resultReceiver_.append("error: option ");
                    resultReceiver_.append("requires a value.\n");

                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
                case 'b': {
                    // 'aa stop-service -b' with no argument
                    TAG_LOGI(AAFwkTag::AA_TOOL, "'aa %{public}s -b' no arg", cmd_.c_str());

                    resultReceiver_.append("error: option ");
                    resultReceiver_.append("requires a value.\n");

                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
                case 'm': {
                    // 'aa stop-service -m' with no argument
                    TAG_LOGI(AAFwkTag::AA_TOOL, "'aa %{public}s -m' no arg", cmd_.c_str());

                    resultReceiver_.append("error: option ");
                    resultReceiver_.append("requires a value.\n");

                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
                case 0: {
                    // 'aa stop-service' with an unknown option: aa stop-service --x
                    // 'aa stop-service' with an unknown option: aa stop-service --xxx
                    std::string unknownOption = "";
                    std::string unknownOptionMsg = GetUnknownOptionMsg(unknownOption);

                    TAG_LOGI(AAFwkTag::AA_TOOL, "'aa %{public}s' opt unknown", cmd_.c_str());

                    resultReceiver_.append(unknownOptionMsg);
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
                default: {
                    // 'aa stop-service' with an unknown option: aa stop-service -x
                    // 'aa stop-service' with an unknown option: aa stop-service -xxx
                    std::string unknownOption = "";
                    std::string unknownOptionMsg = GetUnknownOptionMsg(unknownOption);

                    TAG_LOGI(AAFwkTag::AA_TOOL, "'aa %{public}s' opt unknown", cmd_.c_str());

                    resultReceiver_.append(unknownOptionMsg);
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
            }
            break;
        }

        switch (option) {
            case 'h': {
                // 'aa stop-service -h'
                // 'aa stop-service --help'
                result = OHOS::ERR_INVALID_VALUE;
                break;
            }
            case 'd': {
                // 'aa stop-service -d xxx'

                // save device ID
                if (optarg != nullptr) {
                    deviceId = optarg;
                }
                break;
            }
            case 'a': {
                // 'aa stop-service -a xxx'

                // save ability name
                abilityName = optarg;
                break;
            }
            case 'b': {
                // 'aa stop-service -b xxx'

                // save bundle name
                bundleName = optarg;
                break;
            }
            case 'm': {
                // 'aa stop-service -m xxx'

                // save module name
                moduleName = optarg;
                break;
            }
            case 0: {
                break;
            }
            default: {
                break;
            }
        }
    }

    if (result == OHOS::ERR_OK) {
        if (!abilityName.empty() && bundleName.empty()) {
            // explicitly start ability must have both ability and bundle names

            // 'aa stop-service [-d <device-id>] -a <ability-name> -b <bundle-name>'
            TAG_LOGI(AAFwkTag::AA_TOOL, "'aa %{public}s' without enough options", cmd_.c_str());

            resultReceiver_.append(HELP_MSG_NO_BUNDLE_NAME_OPTION + "\n");
            result = OHOS::ERR_INVALID_VALUE;
        } else {
            ElementName element(deviceId, bundleName, abilityName, moduleName);
            want.SetElement(element);
        }
    }

    return result;
}

ErrCode AbilityManagerShellCommand::RunAsTestCommand()
{
    TAG_LOGD(AAFwkTag::AA_TOOL, "enter");
    std::map<std::string, std::string> params;

    for (int i = USER_TEST_COMMAND_START_INDEX; i < argc_; i++) {
        TAG_LOGI(AAFwkTag::AA_TOOL, "argv_[%{public}d]: %{public}s", i, argv_[i]);
        std::string opt = argv_[i];
        if ((opt == "-h") || (opt == "--help")) {
            resultReceiver_.append(HELP_MSG_TEST);
            return OHOS::ERR_OK;
        } else if ((opt == "-b") || (opt == "-p") || (opt == "-m")) {
            if (i >= argc_ - 1) {
                return TestCommandError("error: option [" + opt + "] requires a value.\n");
            }
            std::string argv = argv_[++i];
            params[opt] = argv;
        } else if (opt == "-w") {
            if (i >= argc_ - 1) {
                return TestCommandError("error: option [" + opt + "] requires a value.\n");
            }

            std::string argv = argv_[++i];
            if (!std::regex_match(argv, std::regex(STRING_TEST_REGEX_INTEGER_NUMBERS))) {
                return TestCommandError("error: option [" + opt + "] only supports integer numbers.\n");
            }

            params[opt] = argv;
        } else if (opt == "-s") {
            if (i >= argc_ - USER_TEST_COMMAND_PARAMS_NUM) {
                return TestCommandError("error: option [-s] is incorrect.\n");
            }
            std::string argKey = argv_[++i];
            std::string argValue = argv_[++i];
            params[opt + " " + argKey] = argValue;
        } else if (opt == "-D") {
            params[opt] = DEBUG_VALUE;
        } else if (opt.at(0) == '-') {
            return TestCommandError("error: unknown option: " + opt + "\n");
        }
    }

    if (!IsTestCommandIntegrity(params)) {
        return OHOS::ERR_INVALID_VALUE;
    }

    return StartUserTest(params);
}

bool AbilityManagerShellCommand::IsTestCommandIntegrity(const std::map<std::string, std::string>& params)
{
    TAG_LOGD(AAFwkTag::AA_TOOL, "enter");

    std::vector<std::string> opts = { "-b" };
    for (auto opt : opts) {
        auto it = params.find(opt);
        if (it == params.end()) {
            TestCommandError("error: the option [" + opt + "] is expected.\n");
            return false;
        }
    }
    return true;
}

ErrCode AbilityManagerShellCommand::TestCommandError(const std::string& info)
{
    resultReceiver_.append(info);
    resultReceiver_.append(HELP_MSG_TEST);
    return OHOS::ERR_INVALID_VALUE;
}

ErrCode AbilityManagerShellCommand::StartUserTest(const std::map<std::string, std::string>& params)
{
    TAG_LOGD(AAFwkTag::AA_TOOL, "enter");

    Want want;
    for (auto param : params) {
        want.SetParam(param.first, param.second);
    }

    auto dPos = params.find("-D");
    if (dPos != params.end() && dPos->second.compare(DEBUG_VALUE) == 0) {
        TAG_LOGI(AAFwkTag::AA_TOOL, "Set Debug to want");
        want.SetParam("debugApp", true);
    }

    sptr<TestObserver> observer = new (std::nothrow) TestObserver();
    if (!observer) {
        TAG_LOGE(AAFwkTag::AA_TOOL, "Failed: the TestObserver is null");
        return OHOS::ERR_INVALID_VALUE;
    }

    int result = AbilityManagerClient::GetInstance()->StartUserTest(want, observer->AsObject());
    if (result != OHOS::ERR_OK) {
        TAG_LOGI(AAFwkTag::AA_TOOL, "%{public}s result: %{public}d", STRING_START_USER_TEST_NG.c_str(), result);
        resultReceiver_ = STRING_START_USER_TEST_NG + "\n";
            if (result == INNER_ERR) {
                result = INNER_ERR_TEST;
            }
        resultReceiver_.append(GetMessageFromCode(result));
        return result;
    }
    TAG_LOGI(AAFwkTag::AA_TOOL, "%{public}s", STRING_USER_TEST_STARTED.c_str());

    std::signal(SIGCHLD, SIG_DFL);

    int64_t timeMs = 0;
    if (!want.GetStringParam("-w").empty()) {
        try {
            auto time = std::stoi(want.GetStringParam("-w"));
            timeMs = time > 0 ? time * TIME_RATE_MS : 0;
        } catch (...) {
            TAG_LOGE(AAFwkTag::AA_TOOL, "stoi(%{public}s) failed", want.GetStringParam("-w").c_str());
        }
    }
    if (!observer->WaitForFinish(timeMs)) {
        resultReceiver_ = "Timeout: user test is not completed within the specified time.\n";
        return OHOS::ERR_INVALID_VALUE;
    }

    TAG_LOGI(AAFwkTag::AA_TOOL, "%{public}s", STRING_USER_TEST_FINISHED.c_str());
    resultReceiver_ = STRING_USER_TEST_FINISHED + "\n";

    return result;
}

sptr<IAbilityManager> AbilityManagerShellCommand::GetAbilityManagerService()
{
    sptr<ISystemAbilityManager> systemManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemManager == nullptr) {
        TAG_LOGE(AAFwkTag::AA_TOOL, "Get registry failed");
        return nullptr;
    }
    sptr<IRemoteObject> remoteObject = systemManager->GetSystemAbility(ABILITY_MGR_SERVICE_ID);
    return iface_cast<IAbilityManager>(remoteObject);
}

ErrCode AbilityManagerShellCommand::StartAbilityWithWait(Want& want)
{
    if (IsImplicitStartAction(want)) {
        auto ret = AbilityManagerClient::GetInstance()->StartAbility(want);
        if (ret != ERR_OK) {
            return ret;
        }
        resultReceiver_.append(STRING_IMPLICT_START_WITH_WAIT_NG + "\n");
        return ret;
    }
    auto observer = sptr<AbilityStartWithWaitObserver>::MakeSptr();
    if (!observer) {
        TAG_LOGE(AAFwkTag::AA_TOOL, "inner error, alloc memory failed.");
        return INNER_ERR;
    }
    auto ret = AbilityManagerClient::GetInstance()->StartAbilityWithWait(want, observer);
    if (ret != ERR_OK) {
        return ret;
    }
    auto maxWaitTime = MAX_WAIT_TIME;
    AbilityStartWithWaitObserverData data;
    while (true) {
        bool isAlwaysWaiting = true;
        observer->GetData(isAlwaysWaiting, data);
        if (!isAlwaysWaiting) {
            FormatOutputForWithWait(want, data);
            break;
        }
        usleep(WAIT_INTERVAL);
        maxWaitTime -= WAIT_INTERVAL;
        if (maxWaitTime <= 0) {
            TAG_LOGE(AAFwkTag::AA_TOOL, "start ability with wait timeout.");
            break;
        }
    }
    return ret;
}

void AbilityManagerShellCommand::FormatOutputForWithWait(const Want& want, const AbilityStartWithWaitObserverData& data)
{
    switch (static_cast<TerminateReason>(data.reason)) {
        case TerminateReason::TERMINATE_FOR_NONE: {
            auto totalTime = data.foregroundTime - data.startTime;
            auto now = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count();
            auto waitTime = now - startTime_;
            resultReceiver_.append("StartMode: ").append(data.coldStart ? "Cold" : "Hot").append("\n")
                .append("BundleName: " + data.bundleName + "\n").append("AbilityName: " + data.abilityName + "\n");
            if (!want.GetModuleName().empty()) {
                resultReceiver_.append("ModuleName: " + want.GetModuleName() + "\n");
            }
            resultReceiver_.append("TotalTime: " + std::to_string(totalTime) + "\n")
                .append("WaitTime: " + std::to_string(waitTime) + "\n");
            break;
        }
        case TerminateReason::TERMINATE_FOR_NON_UI_ABILITY: {
            resultReceiver_.append(STRING_NON_UIABILITY_START_WITH_WAIT_NG + "\n");
            break;
        }
        default:
            // do nothing
            break;
    }
}

bool AbilityManagerShellCommand::IsImplicitStartAction(const Want& want)
{
    auto element = want.GetElement();
    if (!element.GetAbilityName().empty()) {
        return false;
    }

    if (want.GetIntParam(AAFwk::SCREEN_MODE_KEY, ScreenMode::IDLE_SCREEN_MODE) != ScreenMode::IDLE_SCREEN_MODE) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "not use implicit startup process");
        return false;
    }

    if (want.GetAction() != BLACK_ACTION_SELECT_DATA) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "implicit start, action:%{public}s", want.GetAction().data());
        return true;
    }

    return false;
}
#ifdef ABILITY_FAULT_AND_EXIT_TEST

ErrCode AbilityManagerShellCommand::RunAsForceExitAppCommand()
{
    TAG_LOGD(AAFwkTag::AA_TOOL, "enter");
    int result = OHOS::ERR_OK;

    int option = -1;
    int counter = 0;

    std::string pid;
    std::string reason;

    while (true) {
        counter++;
        option = getopt_long(argc_, argv_, SHORT_OPTIONS_FORCE_EXIT_APP.c_str(), LONG_OPTIONS_FORCE_EXIT_APP, nullptr);
        TAG_LOGD(
            AAFwkTag::AA_TOOL, "option: %{public}d, optopt: %{public}d, optind: %{public}d", option, optopt, optind);

        if (optind < 0 || optind > argc_) {
            return OHOS::ERR_INVALID_VALUE;
        }

        if (option == -1) {
            if (counter == 1 && strcmp(argv_[optind], cmd_.c_str()) == 0) {
                TAG_LOGE(AAFwkTag::AA_TOOL, "'aa %{public}s' %{public}s", HELP_MSG_NO_OPTION.c_str(), cmd_.c_str());
                resultReceiver_.append(HELP_MSG_NO_OPTION + "\n");
                result = OHOS::ERR_INVALID_VALUE;
            }
            break;
        }

        switch (option) {
            case 'h': {
                TAG_LOGI(AAFwkTag::AA_TOOL, "'aa %{public}s -h' no arg", cmd_.c_str());
                // 'aa forceexitapp -h'
                // 'aa forceexitapp --help'
                result = OHOS::ERR_INVALID_VALUE;
                break;
            }
            case 'p': {
                TAG_LOGI(AAFwkTag::AA_TOOL, "'aa %{public}s -p' pid", cmd_.c_str());
                // 'aa forceexitapp -p pid'
                pid = optarg;
                break;
            }
            case 'r': {
                TAG_LOGI(AAFwkTag::AA_TOOL, "'aa %{public}s -r' reason", cmd_.c_str());
                // 'aa forceexitapp -r reason'
                reason = optarg;
                break;
            }
            case '?': {
                std::string unknownOption = "";
                std::string unknownOptionMsg = GetUnknownOptionMsg(unknownOption);
                TAG_LOGI(AAFwkTag::AA_TOOL, "'aa notifyappfault' option unknown");
                resultReceiver_.append(unknownOptionMsg);
                result = OHOS::ERR_INVALID_VALUE;
                break;
            }
            default: {
                break;
            }
        }
    }

    if (result != OHOS::ERR_OK) {
        result = OHOS::ERR_INVALID_VALUE;
    }

    ExitReason exitReason = { AbilityToolConvertUtil::ConvertExitReason(reason), "Force exit app by aa." };
    result = AbilityManagerClient::GetInstance()->ForceExitApp(ConvertPid(pid), exitReason);
    if (result == OHOS::ERR_OK) {
        resultReceiver_ = STRING_BLOCK_AMS_SERVICE_OK + "\n";
    } else {
        TAG_LOGI(AAFwkTag::AA_TOOL, "%{public}s result: %{public}d", STRING_BLOCK_AMS_SERVICE_NG.c_str(), result);
        resultReceiver_ = STRING_BLOCK_AMS_SERVICE_NG + "\n";
        resultReceiver_.append(GetMessageFromCode(result));
    }

    TAG_LOGD(AAFwkTag::AA_TOOL, "pid: %{public}s, reason: %{public}s", pid.c_str(), reason.c_str());
    return result;
}

FaultDataType CovertFaultType(std::string &cmd)
{
    if (cmd.empty()) {
        return FaultDataType::UNKNOWN;
    }

    if (cmd.compare("UNKNOWN") == 0) {
        return FaultDataType::UNKNOWN;
    } else if (cmd.compare("CPP_CRASH") == 0) {
        return FaultDataType::CPP_CRASH;
    } else if (cmd.compare("JS_ERROR") == 0) {
        return FaultDataType::JS_ERROR;
    } else if (cmd.compare("APP_FREEZE") == 0) {
        return FaultDataType::APP_FREEZE;
    } else if (cmd.compare("PERFORMANCE_CONTROL") == 0) {
        return FaultDataType::PERFORMANCE_CONTROL;
    } else if (cmd.compare("RESOURCE_CONTROL") == 0) {
        return FaultDataType::RESOURCE_CONTROL;
    }

    return FaultDataType::UNKNOWN;
}

ErrCode AbilityManagerShellCommand::RunAsNotifyAppFaultCommand()
{
    TAG_LOGD(AAFwkTag::AA_TOOL, "called");
    int result = OHOS::ERR_OK;
    int option = -1;
    int counter = 0;
    std::string errorName = "";
    std::string errorMessage = "";
    std::string errorStack = "";
    std::string faultType = "";
    std::string pid = "";
    while (true) {
        counter++;
        option = getopt_long(
            argc_, argv_, SHORT_OPTIONS_NOTIFY_APP_FAULT.c_str(), LONG_OPTIONS_NOTIFY_APP_FAULT, nullptr);
        TAG_LOGI(
            AAFwkTag::AA_TOOL, "option: %{public}d, optopt: %{public}d, optind: %{public}d", option, optopt, optind);
        if (optind < 0 || optind > argc_) {
            return OHOS::ERR_INVALID_VALUE;
        }

        if (option == -1) {
            if (counter == 1 && strcmp(argv_[optind], cmd_.c_str()) == 0) {
                TAG_LOGI(AAFwkTag::AA_TOOL, "'aa %{public}s' %{public}s", HELP_MSG_NO_OPTION.c_str(), cmd_.c_str());
                resultReceiver_.append(HELP_MSG_NO_OPTION + "\n");
                result = OHOS::ERR_INVALID_VALUE;
            }
            break;
        }

        switch (option) {
            case 'h': {
                TAG_LOGI(AAFwkTag::AA_TOOL, "'aa %{public}s -h' no arg", cmd_.c_str());
                // 'aa notifyappfault -h'
                // 'aa notifyappfault --help'
                result = OHOS::ERR_INVALID_VALUE;
                break;
            }
            case 'n': {
                TAG_LOGI(AAFwkTag::AA_TOOL, "'aa %{public}s -n' errorName", cmd_.c_str());
                // 'aa notifyappfault -n errorName'
                errorName = optarg;
                break;
            }
            case 'm': {
                TAG_LOGI(AAFwkTag::AA_TOOL, "'aa %{public}s -m' errorMessage", cmd_.c_str());
                // 'aa notifyappfault -m errorMessage'
                errorMessage = optarg;
                break;
            }
            case 's': {
                TAG_LOGI(AAFwkTag::AA_TOOL, "'aa %{public}s -s' errorStack", cmd_.c_str());
                // 'aa notifyappfault -s errorStack'
                errorStack = optarg;
                break;
            }
            case 't': {
                TAG_LOGI(AAFwkTag::AA_TOOL, "'aa %{public}s -t' faultType", cmd_.c_str());
                // 'aa notifyappfault -t faultType'
                faultType = optarg;
                break;
            }
            case 'p': {
                TAG_LOGI(AAFwkTag::AA_TOOL, "'aa %{public}s -p' pid", cmd_.c_str());
                // 'aa notifyappfault -p pid'
                pid = optarg;
                break;
            }
            case '?': {
                std::string unknownOption = "";
                std::string unknownOptionMsg = GetUnknownOptionMsg(unknownOption);
                TAG_LOGI(AAFwkTag::AA_TOOL, "'aa notifyappfault' option unknown");
                resultReceiver_.append(unknownOptionMsg);
                result = OHOS::ERR_INVALID_VALUE;
                break;
            }
            default: {
                break;
            }
        }
    }

    if (result != OHOS::ERR_OK) {
        result = OHOS::ERR_INVALID_VALUE;
    }

    TAG_LOGI(AAFwkTag::AA_TOOL,
        "name: %{public}s, message: %{public}s, stack: %{public}s, type: %{public}s, pid: %{public}s",
        errorName.c_str(), errorMessage.c_str(), errorStack.c_str(), faultType.c_str(), pid.c_str());

    AppFaultDataBySA faultData;
    faultData.errorObject.name = errorName;
    faultData.errorObject.message = errorMessage;
    faultData.errorObject.stack = errorStack;
    faultData.faultType = CovertFaultType(faultType);
    faultData.pid = ConvertPid(pid);
    DelayedSingleton<AppMgrClient>::GetInstance()->NotifyAppFaultBySA(faultData);
    return result;
}
#endif
}  // namespace AAFwk
}  // namespace OHOS
