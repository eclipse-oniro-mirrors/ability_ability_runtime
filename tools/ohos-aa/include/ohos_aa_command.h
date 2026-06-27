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

#ifndef OHOS_ABILITY_RUNTIME_CLAW_AA_COMMAND_H
#define OHOS_ABILITY_RUNTIME_CLAW_AA_COMMAND_H

#include <getopt.h>
#include <map>
#include <regex>
#include <string>
#include <vector>

#include "ability_manager_interface.h"
#include "ability_start_setting.h"
#include "shell_command.h"

namespace OHOS {
namespace AAFwk {
namespace {
using ParametersInteger = std::map<std::string, int>;
using ParametersString = std::map<std::string, std::string>;
using ParametersBool = std::map<std::string, bool>;

const std::string TOOL_NAME = "ohos-aa";

const std::string HELP_MSG =
    "ohos-aa - Package management utility for starting an ability or stopping an application on the system\n\n"
    "Usage:\n"
    "  ohos-aa [command] [options]\n\n"
    "Parameters:\n"
    "  --help                    Display this help message\n\n"
    "SubCommands:\n"
    "  start                     start an ability\n"
    "  force-stop                stop an application\n\n"
    "Examples:\n"
    "  # Start an ability\n"
    "  ohos-aa start --abilityname EntryAbility --bundlename com.acts.example\n\n"
    "  # Stop an applcation\n"
    "  ohos-aa force-stop --bundlename com.acts.example\n";

const std::string VERSION_MSG = "1.0.0\n";

const std::string HELP_MSG_START = "ohos-aa start - Start an ability on the system\n\n"
    "Usage:\n"
    "  ohos-aa start [options]\n\n"
    "Parameters:\n"
    "  --help                                                   Display this help message\n"
    "  --abilityname <abilityname>                              Ability name to be started\n"
    "  --bundlename <bundlename>                                bundle name to be started\n"
    "  --modulename <modulename>                                module name to be started\n"
    "  --sandboxCloneIndex <sandboxCloneIndex>                  sandbox clone index for launching sandbox clone application (range: 2000-3000)\n"
    "  --creatorBundle <creatorBundle>                          creator bundle name for sandbox clone application\n"
    "  --uri <uri>                                              URI for implicit startup\n"
    "  --action <action>                                        action for implicit startup\n"
    "  --entity <entity>                                        entity for implicit startup\n"
    "  --type <type>                                            type for implicit startup\n"
    "  --pi <'{\"key1\":100,\"key2\":101,\"key3\":102}'>        integer-type key-value pair\n"
    "  --pb <'{\"key1\":true,\"key2\":false,\"key3\":true}'>    bool-type key-value pair\n"
    "  --ps <'{\"key1\":\"str1\",\"key2\":\"str2\",\"key3\":\"str3\"}'>  string-type key-value pair\n"
    "  --psn <type>                                             type for implicit startup\n"
    "  --time                                                   flag for launch-to-foreground time\n\n"
    "Examples:\n"
    "  # Start an ability\n"
    "  ohos-aa start --abilityname EntryAbility --bundlename com.acts.example\n\n";

const std::string HELP_MSG_FORCE_STOP = "ohos-aa force-stop - Stop an application on the system\n\n"
    "Usage:\n"
    "  ohos-aa force-stop [options]\n\n"
    "Parameters:\n"
    "  --help                                             Display this help message\n"
    "  --bundlename <bundlename>                          bundle name to be stopped\n"
    "Examples:\n"
    "  # Stop an applcation\n"
    "  ohos-aa force-stop --bundlename com.acts.example\n";

const std::string HELP_MSG_NO_BUNDLE_NAME_OPTION = "error: -b <bundle-name> is expected.";

const std::string STRING_START_ABILITY_OK = "start ability successfully.";
const std::string STRING_START_ABILITY_NG = "error: failed to start ability.";

const std::string STRING_FORCE_STOP_OK = "force stop process successfully.";
const std::string STRING_FORCE_STOP_NG = "error: failed to force stop process.";

const int NUMBER_TWO = 2;

const std::string STRING_TEST_REGEX_INTEGER_NUMBERS = "^(0|[1-9][0-9]*|-[1-9][0-9]*)$";
const std::string STRING_REGEX_ALL_NUMBERS = "^(-)?([0-9]|[1-9][0-9]+)([\\.][0-9]+)?$";
const std::string STRING_IMPLICT_START_WITH_WAIT_NG = "The wait option does not support starting implict";
const std::string STRING_NON_UIABILITY_START_WITH_WAIT_NG = "The wait option does not support starting non-uiability";

constexpr int EXTRA_ARGUMENTS_FOR_KEY_VALUE_PAIR = 1;
constexpr int EXTRA_ARGUMENTS_FOR_NULL_STRING = 0;
constexpr int PARAM_LENGTH = 20;
constexpr int INDEX_OFFSET = 3;

enum OptionType {
    OPTION_PARAMETER_INTEGER = 1000,
    OPTION_PARAMETER_STRING,
    OPTION_PARAMETER_BOOL,
    OPTION_PARAMETER_NULL_STRING,
    OPTION_ABILITY_NAME,
    OPTION_BUNDLE_NAME,
    OPTION_MODULE_NAME,
    OPTION_URI,
    OPTION_ACTION,
    OPTION_ENTITY,
    OPTION_HELP,
    OPTION_TYPE,
    OPTION_TIME,
    OPTION_SANDBOX_CLONE_INDEX,      // Sandbox clone index for clone application
    OPTION_CREATOR_BUNDLE    // Creator bundle name (untrusted, from command line)
};

const std::string SHORT_OPTIONS = "";

struct option LONG_OPTIONS[] = {
    {"help", no_argument, 0, OPTION_HELP},
    {"abilityname", required_argument, 0, OPTION_ABILITY_NAME},
    {"bundlename", required_argument, 0, OPTION_BUNDLE_NAME},
    {"modulename", required_argument, 0, OPTION_MODULE_NAME},
    {"uri", required_argument, 0, OPTION_URI},
    {"action", required_argument, 0, OPTION_ACTION},
    {"entity", required_argument, 0, OPTION_ENTITY},
    {"type", required_argument, 0, OPTION_TYPE},
    {"time", no_argument, 0, OPTION_TIME}, //对应aa start命令的 -W 选项
    {"pi", required_argument, 0, OPTION_PARAMETER_INTEGER},
    {"ps", required_argument, 0, OPTION_PARAMETER_STRING},
    {"pb", required_argument, 0, OPTION_PARAMETER_BOOL},
    {"psn", required_argument, 0, OPTION_PARAMETER_NULL_STRING},
    {"sandboxCloneIndex", required_argument, 0, OPTION_SANDBOX_CLONE_INDEX},
    {"creatorBundle", required_argument, 0, OPTION_CREATOR_BUNDLE},
    {0, 0, 0, 0}
};
}

class ClawAaShellCommand : public ShellCommand {
public:
    ClawAaShellCommand(int argc, char* argv[]);
    ~ClawAaShellCommand() override
    {}

    ErrCode CreateMessageMap() override;
    void CheckStartAbilityResult(ErrCode& result);
    ErrCode CreateErrorInfoMap();
    AaToolErrorInfo GetErrorInfoFromCode(const int32_t code);

private:
    ErrCode init() override;
    ErrCode CreateCommandMap() override;
    
    ErrCode RunAsHelpCommand();
    ErrCode RunAsStartAbility();
    ErrCode RunAsForceStop();

    bool IsLongStartOption(const std::string &argv);
    bool IsShortStartOption(const std::string &argv);
    bool IsStartOption(const std::string &argv);
    bool CheckParameters(int target);
    ErrCode ParseParamInteger(ParametersInteger& pi);
    ErrCode ParseParamBool(ParametersBool& pb);
    ErrCode ParseParamString(ParametersString& ps);
    void SetParams(const ParametersInteger& pi, Want& want);
    void SetParams(const ParametersString& ps, Want& want);
    void SetParams(const ParametersBool& pb, Want& want);
    pid_t ConvertPid(std::string& inputPid);

    ErrCode MakeWantFromCmd(Want& want, int32_t& userId);
    ErrCode StartAbilityWithWait(Want& want, int32_t userId = DEFAULT_INVAL_VALUE);
    bool IsImplicitStartAction(const Want &want);
    bool MatchOrderString(const std::regex &regexScript, const std::string &orderCmd);
    bool CheckPerfCmdString(const char* optarg, const size_t paramLength, std::string &perfCmd);
    void FormatOutputForWithWait(const Want &want, const AbilityStartWithWaitObserverData& data);

    bool startAbilityWithWaitFlag_ = false;
    bool StartSandboxCloneAbilityFlag_ = false;
    std::map<int32_t, AaToolErrorInfo> errorInfoMap_;
};
}  // namespace AAFwk
}  // namespace OHOS

#endif  // OHOS_ABILITY_RUNTIME_CLAW_AA_COMMAND_H