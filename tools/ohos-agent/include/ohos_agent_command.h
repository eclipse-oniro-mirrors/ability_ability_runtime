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

#ifndef OHOS_ABILITY_RUNTIME_OHOS_AGENT_COMMAND_H
#define OHOS_ABILITY_RUNTIME_OHOS_AGENT_COMMAND_H

#include <getopt.h>
#include <map>
#include <string>
#include <vector>

#include "shell_command.h"

namespace OHOS {
namespace AAFwk {
namespace {
const std::string TOOL_NAME = "ohos-agent";

const std::string HELP_MSG =
    "ohos-agent - CLI tool for connecting AgentExtensionAbility via a long-lived cliMgr session\n\n"
    "Usage:\n"
    "  ohos-agent [command] [options]\n\n"
    "Parameters:\n"
    "  --help                    Display this help message\n\n"
    "SubCommands:\n"
    "  connect                   connect to an AgentExtensionAbility (APP type) and enter command loop\n\n"
    "Examples:\n"
    "  # Connect to an agent (long-lived session; subsequent sendData/authorize/disconnect via stdin)\n"
    "  ohos-agent connect --bundlename com.acts.example --abilityname EntryAgent --agentid <agentId>\n";

const std::string HELP_MSG_CONNECT =
    "ohos-agent connect - Connect to an AgentExtensionAbility (APP type) and enter a long-lived command loop\n\n"
    "Usage:\n"
    "  ohos-agent connect [options]\n\n"
    "Parameters:\n"
    "  --help                    Display this help message\n"
    "  --bundlename <name>       (required) bundleName of the target AgentExtensionAbility host\n"
    "  --abilityname <name>      (required) abilityName of the target AgentExtensionAbility\n"
    "  --modulename <name>       (optional) moduleName of the target AgentExtensionAbility\n"
    "  --agentid <id>             (required) agentId (must be pre-registered as AgentCardType::APP)\n\n"
    "After connect succeeds, the process stays long-lived and reads JSON-line commands from stdin:\n"
    "  {\"op\":\"sendData\",\"data\":\"<string>\"}\n"
    "  {\"op\":\"authorize\",\"data\":\"<string>\"}\n"
    "  {\"op\":\"disconnect\",\"reason\":\"<string>\"}\n"
    "Events are written to stdout as JSON-lines (connect-done/OnData/OnAuthorize/disconnect-done/error).\n\n"
    "Examples:\n"
    "  ohos-agent connect --bundlename com.acts.example --abilityname EntryAgent --agentid agent001\n";

enum OptionType {
    OPTION_HELP = 1000,
    OPTION_BUNDLE_NAME,
    OPTION_ABILITY_NAME,
    OPTION_MODULE_NAME,
    OPTION_AGENT_ID,
};

const std::string SHORT_OPTIONS = "";

struct option LONG_OPTIONS[] = {
    {"help", no_argument, 0, OPTION_HELP},
    {"bundlename", required_argument, 0, OPTION_BUNDLE_NAME},
    {"abilityname", required_argument, 0, OPTION_ABILITY_NAME},
    {"modulename", required_argument, 0, OPTION_MODULE_NAME},
    {"agentid", required_argument, 0, OPTION_AGENT_ID},
    {0, 0, 0, 0}
};
}  // namespace

class OhosAgentShellCommand : public ShellCommand {
public:
    OhosAgentShellCommand(int argc, char* argv[]);
    ~OhosAgentShellCommand() override = default;

    ErrCode CreateMessageMap() override;
    ErrCode CreateErrorInfoMap();
    AaToolErrorInfo GetErrorInfoFromCode(const int32_t code);

    // Exit code surfaced to main(): 0 on clean disconnect / --help; non-zero on connect failure
    // (preflight/connect IPC error, OnAbilityConnectDone non-zero/timeout, invalid args) so the host
    // detects failure via the cliMgr exit event (BR-12).
    int GetExitCode() const { return exitCode_; }

private:
    ErrCode init() override;
    ErrCode CreateCommandMap() override;

    ErrCode RunAsHelpCommand();
    ErrCode RunAsConnect();

    bool ParseConnectOptions(std::string &bundleName, std::string &abilityName,
        std::string &moduleName, std::string &agentId);

    std::map<int32_t, AaToolErrorInfo> errorInfoMap_;
    int exitCode_ = 0;
    bool helpOnly_ = false;  // true when --help was requested (exit 0, not a failure)
};
}  // namespace AAFwk
}  // namespace OHOS

#endif  // OHOS_ABILITY_RUNTIME_OHOS_AGENT_COMMAND_H
