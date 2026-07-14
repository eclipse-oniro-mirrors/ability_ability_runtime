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
#include "ohos_aa_command.h"

#include <charconv>
#include <cstdlib>
#include <getopt.h>
#include <iostream>
#include <nlohmann/json.hpp>
#include <regex>
#include "ability_manager_client.h"
#include "ability_start_with_wait_observer.h"
#include "ability_start_with_wait_observer_utils.h"
#include "global_constant.h"
#include "hilog_tag_wrapper.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

using namespace OHOS::AppExecFwk;
using json = nlohmann::json;

namespace OHOS {
namespace AAFwk {
using TerminateReason = AbilityStartWithWaitObserverUtil::TerminateReason;
namespace {
constexpr int INNER_ERR_START = 10108101;
const std::string ERR_INVALID_COMMAND = "ERR_INVALID_COMMAND";
const std::string ERR_INVALID_INPUT = "ERR_INVALID_INPUT";
constexpr int START_HELP_CODE = 10108104;
const std::string DEVELOPERMODE_STATE = "const.security.developermode.state";
const std::string SHORT_OPTION_CHARS = "chdabetpsmuAUCDESNR";
constexpr int SHORT_OPTION_INDEX = 1;

constexpr int64_t WAIT_INTERVAL = 10 * 1000; // us
constexpr int64_t MAX_WAIT_TIME = 15 * 1000 * 1000; // us

// Error solution strings
const std::string RESOLVE_ABILITY_ERR_SOLUTION_ONE =
    "Check if the parameter abilityName of ohos-aa -a and the parameter bundleName of -b are correct";
const std::string RESOLVE_ABILITY_ERR_SOLUTION_TWO =
    "Check if the application corresponding to the specified bundleName is installed";
const std::string RESOLVE_ABILITY_ERR_SOLUTION_THREE =
    "For multi-HAP applications, "
    "it is necessary to confirm whether the HAP to which the ability belongs has been installed";

const std::string GET_ABILITY_SERVICE_FAILED_SOLUTION_ONE =
    "Check if the application corresponding to the specified bundleName is installed";

const std::string ABILITY_SERVICE_NOT_CONNECTED_SOLUTION_ONE =
    "Try restarting the device and executing again";

const std::string RESOLVE_APP_ERR_SOLUTION_ONE =
    "The app information retrieved from BMS is missing the application name or package name";

const std::string START_ABILITY_WAITING_SOLUTION_ONE = "No need to process, just wait for the startup";

const std::string INNER_ERR_START_SOLUTION_ONE =
    "Confirm whether the system memory is sufficient and "
    "if there are any issues with the system version used by the device";
const std::string INNER_ERR_START_SOLUTION_TWO = "Check if too many abilities have been launched";
const std::string INNER_ERR_START_SOLUTION_THREE = "Try restarting the device";

const std::string KILL_PROCESS_FAILED_SOLUTION_ONE = "Confirm whether the target application exists";
const std::string KILL_PROCESS_FAILED_SOLUTION_TWO = "Confirm the permissions of the target process";

const std::string NO_FOUND_ABILITY_BY_CALLER_SOLUTION_ONE = "Normal specifications, no action needed";

const std::string ABILITY_VISIBLE_FALSE_DENY_REQUEST_SOLUTION_ONE =
    "Check if the exported configuration of the Ability field in the module.json5 of "
    "the pulled application is set to true. "
    "If not, set it to true";

const std::string GET_BUNDLE_INFO_FAILED_SOLUTION_ONE = "Check if the bundleName is correct";
const std::string GET_BUNDLE_INFO_FAILED_SOLUTION_TWO =
    "Check whether the application corresponding "
    "to the specified bundleName is installed";

const std::string KILL_PROCESS_KEEP_ALIVE_SOLUTION_ONE = "Normal specifications, no action needed";

const std::string ERR_UNLOCK_SCREEN_FAILED_IN_DEVELOPER_MODE_SOLUTION_ONE =
    "Check in the settings whether the current device is in developer mode, and turn off developer mode";

const std::string ERR_NOT_SUPPORTED_PRODUCT_TYPE_SOLUTION_ONE = "Normal specifications, no action needed";

const std::string ERR_NOT_IN_APP_PROVISION_MODE_SOLUTION_ONE =
    "The same application can be compiled with the Debug mode process "
    "to produce an application that supports Debug mode";

const std::string ERR_APP_CLONE_INDEX_INVALID_SOLUTION_ONE = "Confirm whether the appCloneIndex is valid";

const std::string ERR_SANDBOX_CLONE_INDEX_INVALID_SOLUTION_ONE =
    "Confirm whether the sandboxCloneIndex is valid (range: 2000-3000)";

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

const std::string BLACK_ACTION_SELECT_DATA = "ohos.want.action.select";

void AddEntities(const std::vector<std::string>& entities, Want& want)
{
    for (auto entity : entities) {
        want.AddEntity(entity);
    }
}
}  // namespace

ClawAaShellCommand::ClawAaShellCommand(int argc, char* argv[]) : ShellCommand(argc, argv, TOOL_NAME)
{
    for (int i = 0; i < argc_; i++) {
        if (i > 1) {
            TAG_LOGI(AAFwkTag::AA_TOOL, "argc greater than 2, ignoring the rest");
            return;
        }
        TAG_LOGI(AAFwkTag::AA_TOOL, "argv_[%{public}d]: %{public}s", i, argv_[i]);
    }
}

void PrintSuccess(const std::string& message)
{
    json response;
    response["type"] = "result";
    response["status"] = "success";
    response["data"]["message"] = message;
    std::cout << response.dump() << std::endl;
}

void PrintError(const AaToolErrorInfo& errorInfo)
{
    json response;
    response["type"] = "result";
    response["status"] = "failed";
    response["errCode"] = errorInfo.code;
    std::string errMessage = errorInfo.message;
    if (errMessage != errorInfo.cause) {
        errMessage = errMessage + " " + errorInfo.cause;
    }
    response["errMsg"] = errMessage;
    response["suggestion"] = errorInfo.SolutionsToString();
    std::cout << response.dump() << std::endl;
}

ErrCode ClawAaShellCommand::CreateCommandMap()
{
    commandMap_ = {
        {"--help", [this]() { return this->RunAsHelpCommand(); }},
        {"help", [this]() { return this->RunAsHelpCommand(); }},
        {"start", [this]() { return this->RunAsStartAbility(); }},
        {"force-stop", [this]() { return this->RunAsForceStop(); }},
    };
    return ERR_OK;
}

ErrCode ClawAaShellCommand::CreateErrorInfoMap()
{
    // Add error code mappings with detailed information (using enum constants as keys)
    errorInfoMap_[ABILITY_VISIBLE_FALSE_DENY_REQUEST] = {"ERR_ABILITY_VISIBLE_FALSE_DENY_REQUEST",
        "Failed to verify the visibility of the target ability.",
        "Application visibility check failed.",
        {ABILITY_VISIBLE_FALSE_DENY_REQUEST_SOLUTION_ONE}};

    errorInfoMap_[RESOLVE_ABILITY_ERR] = {"ERR_ABILITY_NOT_FOUND", "The specified ability does not exist.",
        "The specified Ability is not installed.",
        {RESOLVE_ABILITY_ERR_SOLUTION_ONE, RESOLVE_ABILITY_ERR_SOLUTION_TWO, RESOLVE_ABILITY_ERR_SOLUTION_THREE}};

    errorInfoMap_[ABILITY_SERVICE_NOT_CONNECTED] = {"ERR_ABILITY_SERVICE_NOT_CONNECTED",
        "Ability service connection failed.",
        "Failed to obtain the ability remote service.",
        {ABILITY_SERVICE_NOT_CONNECTED_SOLUTION_ONE}};

    errorInfoMap_[GET_ABILITY_SERVICE_FAILED] = {"ERR_GET_ABILITY_SERVICE_FAILED", "Failed to get the ability service.",
        "The abilityInfo is empty when generating the Ability request through BMS.",
        {GET_ABILITY_SERVICE_FAILED_SOLUTION_ONE}};

    errorInfoMap_[RESOLVE_APP_ERR] = {"ERR_APP_RESOLVE_APP_ERR",
        "An error of the Want could not be resolved to app info from BMS.",
        "Abnormal app information retrieved from BMS.",
        {RESOLVE_APP_ERR_SOLUTION_ONE}};

    errorInfoMap_[NO_FOUND_ABILITY_BY_CALLER] = {"ERR_ABILITY_NO_FOUND_ABILITY_BY_CALLER",
        "The oho-aa start command cannot be used to launch a UIExtensionAbility.",
        "ohos-aa start does not meet the restrictions imposed by UIExtensionAbility on the initiating party.",
        {NO_FOUND_ABILITY_BY_CALLER_SOLUTION_ONE}};

    errorInfoMap_[ERR_IMPLICIT_START_ABILITY_FAIL] = {"ERR_ABILITY_IMPLICIT_START_ABILITY_FAIL",
        "Failed to find a matching application for implicit launch.",
        "The parameter configuration of implicit startup is incorrect, or the specified HAP package is not installed.",
        {ERR_IMPLICIT_START_ABILITY_FAIL_SOLUTION_ONE, ERR_IMPLICIT_START_ABILITY_FAIL_SOLUTION_TWO}};

    errorInfoMap_[ERR_APP_CLONE_INDEX_INVALID] = {"ERR_APP_CLONE_INDEX_INVALID", "The passed appCloneIndex is invalid.",
        "If the appCloneIndex carried in the parameters of the command is an invalid value, return that error code.",
        {ERR_APP_CLONE_INDEX_INVALID_SOLUTION_ONE}};

    errorInfoMap_[ERR_SANDBOX_CLONE_INDEX_INVALID] = {"ERR_SANDBOX_CLONE_INDEX_INVALID",
        "The passed sandboxCloneIndex is invalid.",
        "If the sandboxCloneIndex carried in the parameters of the command is an invalid value, return that error code.",
        {ERR_SANDBOX_CLONE_INDEX_INVALID_SOLUTION_ONE}};

    errorInfoMap_[START_ABILITY_WAITING] = {"ERR_ABILITY_START_ABILITY_WAITING",
        "Another ability is being started. Wait until it finishes starting.",
        "High system concurrency.",
        {START_ABILITY_WAITING_SOLUTION_ONE}};

    errorInfoMap_[ERR_UNLOCK_SCREEN_FAILED_IN_DEVELOPER_MODE] = {"ERR_UNLOCK_SCREEN_FAILED_IN_DEVELOPER_MODE",
        "The device screen is locked during the application launch, unlock screen failed.",
        "The current mode is developer mode, and the screen cannot be unlocked automatically.",
        {ERR_UNLOCK_SCREEN_FAILED_IN_DEVELOPER_MODE_SOLUTION_ONE}};

    errorInfoMap_[ERR_CROWDTEST_EXPIRED] = {"ERR_CROWDTEST_EXPIRED",
        "Failed to unlock the screen in developer mode.",
        "The current mode is developer mode, and the screen cannot be unlocked automatically.",
        {ERR_CROWDTEST_EXPIRED_SOLUTION_ONE}};

    errorInfoMap_[ERR_APP_CONTROLLED] = {"ERR_APP_CONTROLLED",
        "The target application is under control.",
        "The application is suspected of malicious behavior and is restricted from launching by the appStore.",
        {ERR_APP_CONTROLLED_SOLUTION_ONE}};

    errorInfoMap_[ERR_EDM_APP_CONTROLLED] = {"ERR_EDM_APP_CONTROLLED",
        "The target application is managed by EDM.",
        "The application is under the control of enterprise device management.",
        {ERR_EDM_APP_CONTROLLED_SOLUTION_ONE}};

    errorInfoMap_[ERR_NOT_SUPPORTED_PRODUCT_TYPE] = {"ERR_NOT_SUPPORTED_PRODUCT_TYPE",
        "The current device does not support using window options.",
        "The user specified windowOptions, but the device does not support it.",
        {ERR_NOT_SUPPORTED_PRODUCT_TYPE_SOLUTION_ONE}};

    errorInfoMap_[ERR_STATIC_CFG_PERMISSION] = {"ERR_STATIC_CFG_PERMISSION",
        "The specified process does not have the permission.",
        "The specified process permission check failed.",
        {ERR_STATIC_CFG_PERMISSION_SOLUTION_ONE}};

    errorInfoMap_[INNER_ERR_START] = {"ERR_INNER_ERR_START",
        "An internal error occurs while attempting to launch the ability.",
        "Kernel common errors such as memory allocation and multithreading processing. "
        "Specific reasons may include: internal object being null, processing timeout, "
        "failure to obtain application information from package management, failure to obtain system service, "
        "the number of launched ability instances has reached the limit, etc",
        {INNER_ERR_START_SOLUTION_ONE, INNER_ERR_START_SOLUTION_TWO, INNER_ERR_START_SOLUTION_THREE}};

    errorInfoMap_[GET_BUNDLE_INFO_FAILED] = {"ERR_GET_BUNDLE_INFO_FAILED",
        "Failed to retrieve specified package information.",
        "The application corresponding to the specified package name is not installed.",
        {GET_BUNDLE_INFO_FAILED_SOLUTION_ONE, GET_BUNDLE_INFO_FAILED_SOLUTION_TWO}};

    errorInfoMap_[KILL_PROCESS_FAILED] = {"ERR_KILL_PROCESS_FAILED", "kill process failed.",
        "The specified application's process ID does not exist, "
        "there is no permission to kill the target process, or the connection to appManagerService was not successful.",
        {KILL_PROCESS_FAILED_SOLUTION_ONE, KILL_PROCESS_FAILED_SOLUTION_TWO}};

    errorInfoMap_[KILL_PROCESS_KEEP_ALIVE] = {"ERR_KILL_PROCESS_KEEP_ALIVE",
        "Persistent processes cannot be terminated.",
        "Designate the process as a persistent process and ensure that the device has sufficient memory.",
        {KILL_PROCESS_KEEP_ALIVE_SOLUTION_ONE}};

    return ERR_OK;
}

ErrCode ClawAaShellCommand::init()
{
    startTime_ = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    return AbilityManagerClient::GetInstance()->Connect();
}

ErrCode ClawAaShellCommand::RunAsHelpCommand()
{
    if (cmd_ != "--help") {
        std::string message = "Invalid command for ohos-aa.";
        AaToolErrorInfo errorInfo = {
            ERR_INVALID_COMMAND,
            message,
            message,
            {HELP_MSG},
        };
        PrintError(errorInfo);
    } else {
        std::cout << HELP_MSG << std::endl;
    }
    return ERR_OK;
}

ErrCode ClawAaShellCommand::RunAsStartAbility()
{
    Want want;
    int32_t userId = DEFAULT_INVAL_VALUE;
    ErrCode result = MakeWantFromCmd(want, userId);
    if (result == OHOS::ERR_OK) {
        if (startAbilityWithWaitFlag_) {
            result = StartAbilityWithWait(want);
        } else if (startSandboxCloneAbilityFlag_) {
            SandboxCloneParams params;
            // Get caller info from environment variables (set by SA-CLI via config["env"])
            if (const char* envCallerUid = std::getenv("ohos_cli_callerUid")) {
                auto res = std::from_chars(envCallerUid, envCallerUid + std::strlen(envCallerUid), params.callerUid);
                if (res.ec != std::errc()) {
                    TAG_LOGE(AAFwkTag::AA_TOOL, "Invalid callerUid from env: %{public}s", envCallerUid);
                    params.callerUid = -1;
                }
            }
            if (const char* envCallerTokenId = std::getenv("ohos_cli_callerTokenId")) {
                auto res = std::from_chars(envCallerTokenId, envCallerTokenId + std::strlen(envCallerTokenId),
                    params.callerTokenId);
                if (res.ec != std::errc()) {
                    TAG_LOGE(AAFwkTag::AA_TOOL, "Invalid callerTokenId from env: %{public}s", envCallerTokenId);
                    params.callerTokenId = 0;
                }
            }
            if (const char* envCallerBundleName = std::getenv("ohos_cli_callerBundleName"))
                params.callerBundleName = envCallerBundleName;
            TAG_LOGI(AAFwkTag::AA_TOOL, "StartSandboxCloneAbility with callerUid=%{public}d, callerTokenId=%{public}u, "
                "callerBundleName=%{public}s", params.callerUid, params.callerTokenId, params.callerBundleName.c_str());
            result = AbilityManagerClient::GetInstance()->StartSandboxCloneAbility(want, params);
        } else {
            result = AbilityManagerClient::GetInstance()->StartAbility(want);
        }
        if (result == OHOS::ERR_OK) {
            TAG_LOGI(AAFwkTag::AA_TOOL, "%{public}s", STRING_START_ABILITY_OK.c_str());
            resultReceiver_.append(STRING_START_ABILITY_OK);
            PrintSuccess(resultReceiver_);
        } else {
            TAG_LOGI(AAFwkTag::AA_TOOL, "%{public}s result: %{public}d", STRING_START_ABILITY_NG.c_str(), result);
            CheckStartAbilityResult(result);
            if (result == INNER_ERR) {
                result = INNER_ERR_START;
            }
            AaToolErrorInfo errorInfo = GetErrorInfoFromCode(result);
            PrintError(errorInfo);
        }
    } else if (result == START_HELP_CODE) {
        std::cout << HELP_MSG_START << std::endl;
    } else {
        std::string message = "Invalid options or parameters for start command.";
        if (resultReceiver_ == "") {
            resultReceiver_ = message;
        }
        AaToolErrorInfo errorInfo = {
            ERR_INVALID_INPUT,
            message,
            resultReceiver_,
            {HELP_MSG_START},
        };
        PrintError(errorInfo);
        result = OHOS::ERR_INVALID_VALUE;
    }

    return result;
}

void ClawAaShellCommand::CheckStartAbilityResult(ErrCode& result)
{
    auto it = errorInfoMap_.find(result);
    if (it == errorInfoMap_.end()) {
        result = INNER_ERR;
    }
}

ErrCode ClawAaShellCommand::RunAsForceStop()
{
    TAG_LOGI(AAFwkTag::AA_TOOL, "enter");
    if (argList_.size() == NUMBER_TWO && argList_[0] == "--bundlename") {
        std::string bundleName = argList_[1];
        std::string inputReason = "ohos-aa force-stop";
        TAG_LOGI(AAFwkTag::AA_TOOL, "Bundle name %{public}s", bundleName.c_str());
        ErrCode result = AbilityManagerClient::GetInstance()->KillProcess(bundleName, false, 0, inputReason);
        if (result == OHOS::ERR_OK) {
            TAG_LOGI(AAFwkTag::AA_TOOL, "%{public}s", STRING_FORCE_STOP_OK.c_str());
            PrintSuccess(STRING_FORCE_STOP_OK);
        } else {
            TAG_LOGI(AAFwkTag::AA_TOOL, "%{public}s result: %{public}d", STRING_FORCE_STOP_NG.c_str(), result);
            AaToolErrorInfo errorInfo = GetErrorInfoFromCode(result);
            resultReceiver_ = STRING_FORCE_STOP_NG;
            PrintError(errorInfo);
        }
        return result;
    } else if (argList_.size() == 1 && argList_[0] == "--help") {
        std::cout << HELP_MSG_FORCE_STOP << std::endl;
        return OHOS::ERR_OK;
    }

    AaToolErrorInfo errorInfo = {
        ERR_INVALID_INPUT,
        "Invalid options or parameters for force-stop command.",
        "Wrong options or Missing parameters or too many parameters.",
        {HELP_MSG_FORCE_STOP},
    };
    resultReceiver_ = errorInfo.message;
    PrintError(errorInfo);
    return OHOS::ERR_INVALID_VALUE;
}

pid_t ClawAaShellCommand::ConvertPid(std::string& inputPid)
{
    pid_t pid = 0;
    auto res = std::from_chars(inputPid.c_str(), inputPid.c_str() + inputPid.size(), pid);
    if (res.ec != std::errc()) {
        TAG_LOGE(AAFwkTag::AA_TOOL, "pid stoi(%{public}s) failed", inputPid.c_str());
    }
    return pid;
}

bool ClawAaShellCommand::IsLongStartOption(const std::string &argv)
{
    if (argv.find("--") != 0) {
        return false;
    }
    static std::vector<std::string> longOptions;
    if (longOptions.empty()) {
        for (const auto &longOpt : LONG_OPTIONS) {
            if (longOpt.name == nullptr) {
                continue;
            }
            longOptions.emplace_back("--" + std::string(longOpt.name));
        }
    }
    return std::find(longOptions.begin(), longOptions.end(), argv) != longOptions.end();
}

bool ClawAaShellCommand::IsShortStartOption(const std::string &argv)
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

bool ClawAaShellCommand::IsStartOption(const std::string &argv)
{
    if (argv.empty() || (argv.find("-") != 0 && argv.find("--") != 0)) {
        return false;
    }
    if (IsLongStartOption(argv)) {
        return true;
    }
    return IsShortStartOption(argv);
}

bool ClawAaShellCommand::CheckParameters(int extraArguments)
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
ErrCode ClawAaShellCommand::ParseParamInteger(ParametersInteger& pi)
{
    std::string sarg(optarg);
    if (!sarg.empty() && sarg.front() == '\'') {
        sarg.erase(0, 1);
    }
    if (!sarg.empty() && sarg.back() == '\'') {
        sarg.pop_back();
    }
    try {
        auto paramObj = nlohmann::json::parse(sarg.c_str());
        for (auto& [key, value] : paramObj.items()) {
            pi[key] = value.get<int>();
        }
    } catch(const std::exception& e) {
        resultReceiver_.append("invalid parameter for '--pi' option.");
        return OHOS::ERR_INVALID_VALUE;
    }
    return OHOS::ERR_OK;
}

// parse bool parameters
ErrCode ClawAaShellCommand::ParseParamBool(ParametersBool& pb)
{
    std::string sarg(optarg);
    if (!sarg.empty() && sarg.front() == '\'') {
        sarg.erase(0, 1);
    }
    if (!sarg.empty() && sarg.back() == '\'') {
        sarg.pop_back();
    }
    try {
        auto paramObj = nlohmann::json::parse(sarg.c_str());
        for (auto& [key, value] : paramObj.items()) {
            pb[key] = value.get<bool>();
        }
    } catch(const std::exception& e) {
        resultReceiver_.append("invalid parameter for '--pb' option.");
        return OHOS::ERR_INVALID_VALUE;
    }
    return OHOS::ERR_OK;
}

// parse string parameters
ErrCode ClawAaShellCommand::ParseParamString(ParametersString& ps)
{
    std::string sarg(optarg);
    if (!sarg.empty() && sarg.front() == '\'') {
        sarg.erase(0, 1);
    }
    if (!sarg.empty() && sarg.back() == '\'') {
        sarg.pop_back();
    }
    try {
        auto paramObj = nlohmann::json::parse(sarg.c_str());
        for (auto& [key, value] : paramObj.items()) {
            ps[key] = value.get<std::string>();
        }
    } catch(const std::exception& e) {
        resultReceiver_.append("invalid parameter for '--ps' option.");
        return OHOS::ERR_INVALID_VALUE;
    }
    return OHOS::ERR_OK;
}

void ClawAaShellCommand::SetParams(const ParametersInteger& pi, Want& want)
{
    for (auto it = pi.begin(); it != pi.end(); it++) {
        want.SetParam(it->first, it->second);
    }
}

void ClawAaShellCommand::SetParams(const ParametersString& ps, Want& want)
{
    for (auto it = ps.begin(); it != ps.end(); it++) {
        want.SetParam(it->first, it->second);
    }
}

void ClawAaShellCommand::SetParams(const ParametersBool& pb, Want& want)
{
    for (auto it = pb.begin(); it != pb.end(); it++) {
        want.SetParam(it->first, it->second);
    }
}

bool ClawAaShellCommand::MatchOrderString(const std::regex &regexScript, const std::string &orderCmd)
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

bool ClawAaShellCommand::CheckPerfCmdString(const char* optarg, const size_t paramLength, std::string &perfCmd)
{
    if (optarg == nullptr) {
        TAG_LOGE(AAFwkTag::AA_TOOL, "null optarg");
        return false;
    }

    if (strlen(optarg) >= paramLength) {
        TAG_LOGE(AAFwkTag::AA_TOOL, "ohos-aa start -p param length must < 1024");
        return false;
    }

    perfCmd = optarg;
    const std::regex regexDumpHeapType(R"(^\s*(dumpheap)\s*$)");
    const std::regex regexSleepType(R"(^\s*(sleep)((\s+\d*)|)\s*$)");
    const std::regex regexBaseLineProfileType(R"(^\s*(baseLineProfile)(\s+.*|$))");
    if (MatchOrderString(regexDumpHeapType, perfCmd) || MatchOrderString(regexSleepType, perfCmd) ||
        MatchOrderString(regexBaseLineProfileType, perfCmd)) {
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

ErrCode ClawAaShellCommand::MakeWantFromCmd(Want& want, int32_t& userId)
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
    int32_t sandBoxCloneIndex = 0;
    bool hasSandBoxCloneIndex = false;
    std::string creatorBundleName;  // Creator bundle name (untrusted, from command line)

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
                case OPTION_HELP: {
                    // 'aa start -h'
                    // 'aa stop-service -h'
                    TAG_LOGI(AAFwkTag::AA_TOOL, "'ohos-aa %{public}s --help' wrong arg", cmd_.c_str());
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
                case OPTION_ABILITY_NAME: {
                    // 'aa start -a' with no argument
                    // 'aa stop-service -a' with no argument
                    TAG_LOGI(AAFwkTag::AA_TOOL, "'ohos-aa %{public}s --abilityname' no arg", cmd_.c_str());

                    resultReceiver_.append("error: option ");
                    resultReceiver_.append("requires a value.\n");

                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
                case OPTION_BUNDLE_NAME: {
                    // 'aa start -b' with no argument
                    // 'aa stop-service -b' with no argument
                    TAG_LOGI(AAFwkTag::AA_TOOL, "'ohos-aa %{public}s --bundlename' no arg", cmd_.c_str());

                    resultReceiver_.append("error: option ");
                    resultReceiver_.append("requires a value.\n");

                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
                case OPTION_ENTITY: {
                    // 'aa start -e' with no argument
                    TAG_LOGI(AAFwkTag::AA_TOOL, "'ohos-aa %{public}s --entity no arg", cmd_.c_str());

                    resultReceiver_.append("error: option ");
                    resultReceiver_.append("requires a value.\n");

                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
                case OPTION_TYPE: {
                    // 'aa start -t' with no argument
                    TAG_LOGI(AAFwkTag::AA_TOOL, "'ohos-aa %{public}s --time no arg", cmd_.c_str());

                    resultReceiver_.append("error: option ");
                    resultReceiver_.append("requires a value.\n");

                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
                case OPTION_MODULE_NAME: {
                    // 'aa start -m' with no argument
                    // 'aa stop-service -m' with no argument
                    TAG_LOGI(AAFwkTag::AA_TOOL, "'ohos-aa %{public}s --modulename' no arg", cmd_.c_str());

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
                case OPTION_ACTION: {
                    // 'aa start -A' with no argument
                    TAG_LOGI(AAFwkTag::AA_TOOL, "'ohos-aa %{public}s --action' no arg", cmd_.c_str());

                    resultReceiver_.append("error: option ");
                    resultReceiver_.append("requires a value.\n");

                    result = OHOS::ERR_INVALID_VALUE;

                    break;
                }
                case OPTION_URI: {
                    // 'aa start -U' with no argument
                    TAG_LOGI(AAFwkTag::AA_TOOL, "'ohos-aa %{public}s --uri' no arg", cmd_.c_str());

                    resultReceiver_.append("error: option ");
                    resultReceiver_.append("requires a value.\n");

                    result = OHOS::ERR_INVALID_VALUE;

                    break;
                }
                case OPTION_TIME: {
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
            case OPTION_HELP: {
                // 'aa start -h'
                // 'aa start --help'
                // 'aa stop-service -h'
                // 'aa stop-service --help'
                result = START_HELP_CODE;
                break;
            }
            case OPTION_ABILITY_NAME: {
                // 'aa start -a xxx'
                // 'aa stop-service -a xxx'

                // save ability name
                abilityName = optarg;
                break;
            }
            case OPTION_BUNDLE_NAME: {
                // 'aa start -b xxx'
                // 'aa stop-service -b xxx'

                // save bundle name
                bundleName = optarg;
                break;
            }
            case OPTION_ENTITY: {
                // 'aa start -e xxx'

                // save entity
                entities.push_back(optarg);
                break;
            }
            case OPTION_TYPE: {
                // 'aa start -t xxx'

                // save type
                typeVal = optarg;
                break;
            }
            case OPTION_MODULE_NAME: {
                // 'aa start -m xxx'
                // 'aa stop-service -m xxx'

                // save module name
                moduleName = optarg;
                break;
            }
            case OPTION_TIME: {
                // 'aa start -W' with no argument
                startAbilityWithWaitFlag_ = true;
                break;
            }
            case OPTION_PARAMETER_INTEGER: {
                // 'ohos-aa start --pi xxx'
                // parse option arguments into a key-value map
                result = ParseParamInteger(parametersInteger);
                break;
            }
            case OPTION_PARAMETER_STRING: {
                // 'aa start --ps xxx'
                // parse option arguments into a key-value map
                result = ParseParamString(parametersString);

                break;
            }
            case OPTION_PARAMETER_BOOL: {
                // 'aa start --pb xxx'
                // parse option arguments into a key-value map
                result = ParseParamBool(parametersBool);

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
                parametersString[optarg] = "";
                result = OHOS::ERR_OK;
                
                break;
            }
            case OPTION_SANDBOX_CLONE_INDEX: {
                // 'ohos-aa start --sandboxCloneIndex xxx'
                if (optarg != nullptr) {
                    std::string sandBoxCloneIndexStr = optarg;
                    if (!std::regex_match(sandBoxCloneIndexStr, std::regex(STRING_TEST_REGEX_INTEGER_NUMBERS))) {
                        TAG_LOGE(AAFwkTag::AA_TOOL, "invalid sandboxCloneIndex: %{public}s",
                            sandBoxCloneIndexStr.c_str());
                        result = ERR_SANDBOX_CLONE_INDEX_INVALID;
                        break;
                    }
                    sandBoxCloneIndex = std::stoi(sandBoxCloneIndexStr);
                    hasSandBoxCloneIndex = true;
                    startSandboxCloneAbilityFlag_ = true;
                    TAG_LOGI(AAFwkTag::AA_TOOL, "sandBoxCloneIndex = %{public}d, Flag_ set to true", sandBoxCloneIndex);
                }
                break;
            }
            case OPTION_CREATOR_BUNDLE: {
                // 'ohos-aa start --creatorBundle xxx'
                if (optarg != nullptr) {
                    creatorBundleName = optarg;
                    TAG_LOGI(AAFwkTag::AA_TOOL, "creatorBundleName = %{public}s", creatorBundleName.c_str());
                }
                break;
            }
            case OPTION_URI: {
                // 'aa start -U xxx'

                // save URI
                uri = optarg;
                break;
            }
            case OPTION_ACTION: {
                // 'aa start -A xxx'

                // save action
                action = optarg;
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
        if (result != OHOS::ERR_OK) {
            break;
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
            // Sandbox clone application support parameters
            if (hasSandBoxCloneIndex) {
                want.SetParam(AbilityRuntime::GlobalConstant::SANDBOX_CLONE_INDEX, sandBoxCloneIndex);
            }
            // Set creator bundle name (untrusted, from command line parameter)
            if (!creatorBundleName.empty()) {
                want.SetParam(AbilityRuntime::GlobalConstant::CREATOR_BUNDLE_NAME, creatorBundleName);
                TAG_LOGI(AAFwkTag::AA_TOOL, "creatorBundleName: %{public}s", creatorBundleName.c_str());
            }
        }
    }

    return result;
}

ErrCode ClawAaShellCommand::StartAbilityWithWait(Want& want, int32_t userId)
{
    if (IsImplicitStartAction(want)) {
        auto ret = AbilityManagerClient::GetInstance()->StartAbility(want, DEFAULT_INVAL_VALUE, userId);
        if (ret != ERR_OK) {
            return ret;
        }
        resultReceiver_.append(STRING_IMPLICT_START_WITH_WAIT_NG + "\n");
        return ret;
    }
    if (userId != DEFAULT_INVAL_VALUE) {
        TAG_LOGW(AAFwkTag::AA_TOOL, "userId %{public}d is ignored when using -W option", userId);
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

void ClawAaShellCommand::FormatOutputForWithWait(const Want &want, const AbilityStartWithWaitObserverData& data)
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

bool ClawAaShellCommand::IsImplicitStartAction(const Want &want)
{
    auto element = want.GetElement();
    if (!element.GetAbilityName().empty()) {
        return false;
    }

    if (want.GetIntParam(AAFwk::SCREEN_MODE_KEY, ScreenMode::IDLE_SCREEN_MODE) != ScreenMode::IDLE_SCREEN_MODE) {
        TAG_LOGI(AAFwkTag::AA_TOOL, "not use implicit startup process");
        return false;
    }

    if (want.GetAction() != BLACK_ACTION_SELECT_DATA) {
        TAG_LOGI(AAFwkTag::AA_TOOL, "implicit start, action:%{public}s", want.GetAction().data());
        return true;
    }

    return false;
}

AaToolErrorInfo ClawAaShellCommand::GetErrorInfoFromCode(const int32_t code)
{
    TAG_LOGI(AAFwkTag::AA_TOOL, "code = %{public}d", code);

    AaToolErrorInfo result;
    if (errorInfoMap_.find(code) != errorInfoMap_.end()) {
        result = errorInfoMap_.at(code);
    }

    TAG_LOGI(AAFwkTag::AA_TOOL, "result: %{public}s", result.ToString().c_str());

    return result;
}

ErrCode ClawAaShellCommand::CreateMessageMap()
{
    return OHOS::ERR_OK;
}
}  // namespace AAFwk
}  // namespace OHOS