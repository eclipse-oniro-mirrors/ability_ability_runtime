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

#ifndef OHOS_ABILITY_RUNTIME_ABILITY_COMMAND_H
#define OHOS_ABILITY_RUNTIME_ABILITY_COMMAND_H

#include <regex>

#include "ability_manager_interface.h"
#include "shell_command.h"

namespace OHOS {
namespace AAFwk {
namespace {
using ParametersInteger = std::map<std::string, int>;
using ParametersString = std::map<std::string, std::string>;
using ParametersBool = std::map<std::string, bool>;

const std::string TOOL_NAME = "aa";

// not show in command
// process -- debug ability with options
const std::string HELP_MSG = "usage: aa <command> <options>\n"
    "These are common aa commands list:\n"
    "  help                        list available commands\n"
    "  start                       start ability with options\n"
    "  stop-service                stop service with options\n"
    "  dump                        dump the ability info\n"
    "  force-stop <bundle-name>    force stop the process with bundle name\n"
    "  attach                      attach application to enter debug mode\n"
    "  detach                      detach application to exit debug mode\n"
    "  send-memory-level           send memory level to application by pid \n"
#ifdef ABILITY_COMMAND_FOR_TEST
    "  test                        start the test framework with options\n"
    "  ApplicationNotResponding     Pass in pid with options\n"
#else
    "  test                        start the test framework with options\n"
    "  appdebug                    set / cancel / get waiting debug status\n"
    "  process                     debug and tune\n";
#endif

const std::string HELP_ApplicationNotResponding =
    "usage: aa ApplicationNotResponding <options>\n"
    "options list:\n"
    "  -h, --help                   list available commands\n"
    "  -p, --pid                    Pass in pid with option\n";

// not show in command
// [-N] [-p <perf-cmd>]
const std::string HELP_MSG_START =
    "usage: aa start <options>\n"
    "options list:\n"
    "  -h, --help                                                   list available commands\n"
    "  [-d <device-id>] [-a <ability-name> -b <bundle-name>] [-m <module-name>] [-p <perf-cmd>] "
    "  [-C] [-D] [-E] [-S] [-N] "
    "  [-R] [-c] [--ps <key> <string-value>] "
    "  [--pi <key> <integer-value>] "
    "  [--pb <key> <boolean-value>] "
    "  [--psn <key>] "
    "  [-A <action-name>] "
    "  [-U <URI>] "
    "  [-e <entity>] "
    "  [-t <mime-type>] "
    "  [-W] "
    "  [--wl <window-left>] "
    "  [--wt <window-top>] "
    "  [--wh <window-height>] "
    "  [--ww <window-width>] "
    "  [-u <user-id>] "
    "  start ability with an element name\n";

const std::string HELP_MSG_STOP_SERVICE =
    "usage: aa stop-service <options>\n"
    "options list:\n"
    "  -h, --help                                                   list available commands\n"
    "  [-d <device-id>] -a <ability-name> -b <bundle-name> [-m <module-name>] "
    "  stop service with an element name\n";

const std::string HELP_MSG_DUMPSYS = "usage: aa dump <options>\n"
    "options list:\n"
    "  -h, --help                   list available commands\n"
    "  -a, --all                    dump all abilities\n"
    "  -l, --mission-list           dump mission list\n"
    "  -i, --ability                dump abilityRecordId\n"
    "  -e, --extension              dump elementName (FA: serviceAbilityRecords,"
    "Stage: ExtensionRecords)\n"
    "  -p, --pending                dump pendingWantRecordId\n"
    "  -r, --process                dump process\n"
    "  -d, --data                   dump the data abilities\n"
    "  -u, --userId                 userId\n"
    "  -c, --client                 client\n"
    "  -c, -u are auxiliary parameters and cannot be used alone\n"
    "  The original -s parameter is invalid\n"
    "  The original -m parameter is invalid\n";

const std::string HELP_MSG_PROCESS = "usage: aa process <options>\n"
    "options list:\n"
    "  -h, --help                   list available commands\n"
    "  -a <ability-name> -b <bundle-name> [-m <module-name>]\n"
    "  -p <perf-cmd>                performance optimization command. Either -p or -D must be selected, "
    "-p takes precedence.\n"
    "  -D <debug-cmd>               debug command. Either -p or -D must be selected, -p takes precedence.\n"
    "  [-S]\n"
    "  debug ability with an element name\n";

const std::string HELP_MSG_TEST =
    "usage: aa test <options>\n"
    "options list:\n"
    "  -h, --help                                             list available commands\n"
    "  -b <bundle-name> -s unittest <test-runner>             start the test framework with options\n"
    "                  [-p <package-name>]                    the name of package with test-runner, "
    "required for the FA model\n"
    "                  [-m <module-name>]                     the name of module with test-runner, "
    "required for the STAGE model\n"
    "                  [-s class <test-class>]\n"
    "                  [-s level <test-level>]\n"
    "                  [-s size <test-size>]\n"
    "                  [-s testType <test-testType>]\n"
    "                  [-s timeout <test-timeout>]\n"
    "                  [-s <any-key> <any-value>]\n"
    "                  [-w <wait-time>]\n"
    "                  [-u <userId>]                          user id for running test\n"
    "                  [-D]\n";

const std::string HELP_MSG_SEND_MEMORY_LEVEL =
    "Usage: aa send-memory-level -p <PID> -l <LEVEL>\n"
    "Mandatory Options:\n"
    "  -p, --pid <PID>         Target process ID (integer)\n"
    "  -l, --level <0|1|2>     Memory level (0: Moderate, 1: Low, 2: Critical)\n\n"
    "Optional:\n"
    "  -h, --help              Show this help message\n";

const std::string HELP_MSG_PRESTART =
    "Usage: aa pre-start -m <MODE> -b <BUNDLE-NAME> -u <USER-ID>\n"
    "Mandatory Options:\n"
    "  -m, --mode <MODE>           Prestart mode (currently only supports 1 for Specific stage prelaunch)\n"
    "  -b, --bundle <BUNDLE>   Bundle name of the application\n"
    "  -u, --user <USER-ID>    User ID (integer)\n\n"
    "Optional:\n"
    "  -h, --help              Show this help message\n";

const std::string HELP_MSG_ATTACH_APP_DEBUG =
    "usage: aa attach <options>\n"
    "options list:\n"
    "  -h, --help                                             list available commands\n"
    "  -b <bundle-name>                                       let application enter debug mode by bundle name\n";
const std::string HELP_MSG_DETACH_APP_DEBUG =
    "usage: aa detach <options>\n"
    "options list:\n"
    "  -h, --help                                             list available commands\n"
    "  -b <bundle-name>                                       let application exit debug mode by bundle name\n";

const std::string HELP_MSG_APPDEBUG_APP_DEBUG =
    "usage: aa appdebug <options>\n"
    "options list:\n"
    "  -h, --help                                  list available commands\n"
    "  -b, --bundlename <bundle-name>              let application set wait debug mode by bundle name with options\n"
    "                  [-p, --persist]             option: persist flag\n"
    "  -c, --cancel                                let application cancel wait debug\n"
    "  -g, --get                                   get wait debug mode application bundle name and persist flag\n";

const std::string HELP_MSG_FORCE_STOP = "usage: aa force-stop <bundle-name> [-p pid] [-r kill-reason]\n";
const std::string HELP_MSG_BLOCK_ABILITY = "usage: aa block-ability <abilityrecordid>\n";
const std::string HELP_MSG_FORCE_TIMEOUT =
    "usage: aa force-timeout <ability-name> <INITIAL|INACTIVE|COMMAND|FOREGROUND|BACKGROUND|TERMINATING>\n"
    "usage: aa force-timeout clean.";
const std::string HELP_MSG_FORCE_TIMEOUT_CLEAN = "clean";

const std::string HELP_MSG_NO_ABILITY_NAME_OPTION = "error: -a <ability-name> is expected";
const std::string HELP_MSG_NO_BUNDLE_NAME_OPTION = "error: -b <bundle-name> is expected";

const std::string STRING_START_ABILITY_OK = "start ability successfully.";
const std::string STRING_START_ABILITY_NG = "error: failed to start ability.";

const std::string STRING_STOP_SERVICE_ABILITY_OK = "stop service ability successfully.";
const std::string STRING_STOP_SERVICE_ABILITY_NG = "error: failed to stop service ability.";

const std::string STRING_FORCE_STOP_OK = "force stop process successfully.";
const std::string STRING_FORCE_STOP_NG = "error: failed to force stop process.";

const std::string STRING_ATTACH_APP_DEBUG_OK = "attach app debug successfully.";
const std::string STRING_ATTACH_APP_DEBUG_NG = "error: failed to attach app debug.";

const std::string STRING_DETACH_APP_DEBUG_OK = "detach app debug successfully.";
const std::string STRING_DETACH_APP_DEBUG_NG = "error: failed to detach app debug.";

const std::string STRING_SEND_MEMORY_LEVEL_OK = "send memory level successfully.";
const std::string STRING_SEND_MEMORY_LEVEL_NG = "error: failed to send memory level.";

const std::string STRING_PRESTART_OK = "prestart successfully.";
const std::string STRING_PRESTART_NG = "error: failed to prestart.";

const std::string STRING_START_USER_TEST_NG = "error: failed to start user test.";
const std::string STRING_USER_TEST_STARTED = "user test started.";
const std::string STRING_USER_TEST_FINISHED = "user test finished.";

const std::string STRING_BLOCK_AMS_SERVICE_OK = "block ams service successfully.";
const std::string STRING_BLOCK_AMS_SERVICE_NG = "error: failed to block ams service.";

const std::string STRING_APP_DEBUG_OK = "app debug successfully.";
const std::string STRING_APP_DEBUG_NG = "error: failed to app debug.";

const std::string STRING_START_NATIVE_PROCESS_OK = "start native process successfully.";
const std::string STRING_START_NATIVE_PROCESS_NG = "error: failed to start native process.";

const int USER_TEST_COMMAND_START_INDEX = 2;
const int USER_TEST_COMMAND_PARAMS_NUM = 2;
const int TIME_RATE_MS = 1000;
const std::string STRING_FORCE_TIMEOUT_OK = "force ability timeout successfully.";
const std::string STRING_FORCE_TIMEOUT_NG = "error: failed to force ability timeout.";

const int NUMBER_TWO = 2;
const int NUMBER_ONE = 1;

const std::string DEBUG_VALUE = "true";

const std::string PERFCMD_FIRST_PROFILE = "profile";
const std::string PERFCMD_FIRST_DUMPHEAP = "dumpheap";

const std::string STRING_TEST_REGEX_INTEGER_NUMBERS = "^(0|[1-9][0-9]*|-[1-9][0-9]*)$";
const std::string STRING_REGEX_ALL_NUMBERS = "^(-)?([0-9]|[1-9][0-9]+)([\\.][0-9]+)?$";
const std::string STRING_IMPLICT_START_WITH_WAIT_NG = "The wait option does not support starting implict";
const std::string STRING_NON_UIABILITY_START_WITH_WAIT_NG = "The wait option does not support starting non-uiability";
}  // namespace

class AbilityManagerShellCommand : public ShellCommand {
public:
    AbilityManagerShellCommand(int argc, char* argv[]);
    ~AbilityManagerShellCommand() override
    {}

    ErrCode CreateMessageMap() override;
    std::string GetAaToolErrorInfo(std::string errorCode, std::string message, std::string cause,
        std::vector<std::string> solutions);
    void CheckStartAbilityResult(ErrCode& result);
    bool IsTestCommandIntegrity(const std::map<std::string, std::string>& params);
    ErrCode StartUserTest(const std::map<std::string, std::string>& params);

private:
    ErrCode CreateCommandMap() override;
    ErrCode init() override;

    ErrCode RunAsHelpCommand();
    ErrCode RunAsStartAbility();
    ErrCode RunAsStopService();
    ErrCode RunAsDumpsysCommand();
    ErrCode RunAsForceStop();
    bool SwitchOptionForAppDebug(int32_t option, std::string &bundleName, bool &isPersist, bool &isCancel, bool &isGet);
    bool ParseAppDebugParameter(std::string &bundleName, bool &isPersist, bool &isCancel, bool &isGet);
    ErrCode RunAsAppDebugDebugCommand();
    ErrCode RunAsProcessCommand();
    ErrCode RunAsAttachDebugCommand();
    ErrCode RunAsDetachDebugCommand();
    ErrCode RunAsSendMemoryLevelCommand();
    ErrCode RunAsPreStartCommand();
    ErrCode ParsePreStartOptions(int32_t& mode, std::string& bundleName, int32_t& userId);
    ErrCode ParsePidMemoryLevel(std::string& pidParse, std::string& memoryLevelParse);
    bool IsLongStartOption(const std::string &argv);
    bool IsShortStartOption(const std::string &argv);
    bool IsStartOption(const std::string &argv);
    bool CheckParameters(int target);
    ErrCode ParseParam(ParametersInteger& pi);
    ErrCode ParseParam(ParametersString& ps, bool isNull);
    ErrCode ParseParam(ParametersBool& pb);
    void SetParams(const ParametersInteger& pi, Want& want);
    void SetParams(const ParametersString& ps, Want& want);
    void SetParams(const ParametersBool& pb, Want& want);
    pid_t ConvertPid(std::string& inputPid);

#ifdef ABILITY_COMMAND_FOR_TEST
    ErrCode RunForceTimeoutForTest();
#endif
#ifdef ABILITY_FAULT_AND_EXIT_TEST
    ErrCode RunAsForceExitAppCommand();
    ErrCode RunAsNotifyAppFaultCommand();
#endif
    sptr<IAbilityManager> GetAbilityManagerService();

    ErrCode MakeWantFromCmd(Want& want, std::string& windowMode, int32_t& userId);
    ErrCode MakeWantFromCmd(Want& want, std::string& windowMode);
    ErrCode MakeWantFromCmdForStopService(Want& want);
    ErrCode MakeWantForProcess(Want& want);
    ErrCode RunAsTestCommand();
    ErrCode TestCommandError(const std::string& info);
    ErrCode ParseTestCommandOption(const std::string &opt, int &i, std::map<std::string, std::string> &params);
    bool MatchOrderString(const std::regex &r, const std::string &orderCmd);
    bool CheckPerfCmdString(const char* optarg, const size_t paramLength, std::string &perfCmd);
    void ParseBundleName(std::string &bundleName);
    ErrCode StartAbilityWithWait(Want& want, int32_t userId = DEFAULT_INVAL_VALUE);
    void FormatOutputForWithWait(const Want &want, const AbilityStartWithWaitObserverData& data);
    bool IsImplicitStartAction(const Want &want);
    bool startAbilityWithWaitFlag_ = false;
};
}  // namespace AAFwk
}  // namespace OHOS

#endif  // OHOS_ABILITY_RUNTIME_ABILITY_COMMAND_H
