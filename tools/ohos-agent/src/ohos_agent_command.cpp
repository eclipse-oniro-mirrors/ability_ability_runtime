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

#include "ohos_agent_command.h"

#include <cstdlib>
#include <getopt.h>
#include <iostream>

#include "agent_extension_connection_constants.h"
#include "agent_manager_client.h"
#include "cli_agent_connection.h"
#include "hilog_tag_wrapper.h"
#include "session_protocol.h"
#include "want.h"

namespace OHOS {
namespace AAFwk {

OhosAgentShellCommand::OhosAgentShellCommand(int argc, char* argv[]) : ShellCommand(argc, argv, TOOL_NAME) {}

ErrCode OhosAgentShellCommand::CreateMessageMap()
{
    messageMap_[0] = "success";
    return ERR_OK;
}

ErrCode OhosAgentShellCommand::CreateErrorInfoMap()
{
    return ERR_OK;
}

AaToolErrorInfo OhosAgentShellCommand::GetErrorInfoFromCode(const int32_t code)
{
    AaToolErrorInfo info;
    auto it = errorInfoMap_.find(code);
    if (it != errorInfoMap_.end()) {
        return it->second;
    }
    return info;
}

ErrCode OhosAgentShellCommand::init()
{
    return ERR_OK;
}

ErrCode OhosAgentShellCommand::CreateCommandMap()
{
    commandMap_["help"] = std::bind(&OhosAgentShellCommand::RunAsHelpCommand, this);
    commandMap_["connect"] = std::bind(&OhosAgentShellCommand::RunAsConnect, this);
    return ERR_OK;
}

ErrCode OhosAgentShellCommand::RunAsHelpCommand()
{
    resultReceiver_ = HELP_MSG;
    return ERR_OK;
}

bool OhosAgentShellCommand::ParseConnectOptions(std::string &bundleName, std::string &abilityName,
    std::string &moduleName, std::string &agentId)
{
    int index = -1;
    while ((index = getopt_long(argc_, argv_, SHORT_OPTIONS.c_str(), LONG_OPTIONS, nullptr)) != -1) {
        switch (index) {
            case OPTION_HELP:
                helpOnly_ = true;
                resultReceiver_ = HELP_MSG_CONNECT;
                return false;
            case OPTION_BUNDLE_NAME:
                bundleName = optarg;
                break;
            case OPTION_ABILITY_NAME:
                abilityName = optarg;
                break;
            case OPTION_MODULE_NAME:
                moduleName = optarg;
                break;
            case OPTION_AGENT_ID:
                agentId = optarg;
                break;
            default:
                resultReceiver_ = "error: unknown option for connect.\n" + HELP_MSG_CONNECT;
                return false;
        }
    }
    if (bundleName.empty() || abilityName.empty() || agentId.empty()) {
        resultReceiver_ = "error: --bundlename, --abilityname and --agentid are required.\n" + HELP_MSG_CONNECT;
        return false;
    }
    return true;
}

ErrCode OhosAgentShellCommand::RunAsConnect()
{
    TAG_LOGI(AAFwkTag::AA_TOOL, "ohos-agent connect invoked");
    std::string bundleName;
    std::string abilityName;
    std::string moduleName;
    std::string agentId;
    if (!ParseConnectOptions(bundleName, abilityName, moduleName, agentId)) {
        // No event stream is running yet (pre-loop); the failure surfaces via exitCode -> EXIT(FAILED).
        TAG_LOGE(AAFwkTag::AA_TOOL, "connect: invalid arguments (helpOnly=%{public}d)",
            helpOnly_ ? 1 : 0);
        exitCode_ = helpOnly_ ? 0 : 1;  // --help is not a failure; missing args is
        return ERR_INVALID_VALUE;
    }

    // Build the target Want.
    Want want;
    if (moduleName.empty()) {
        want.SetElementName(bundleName, abilityName);
    } else {
        want.SetElementName("", bundleName, abilityName, moduleName);
    }
    want.SetParam(AgentRuntime::AGENTID_KEY, agentId);

    // app A's calling identity, captured by cliMgr and propagated to agentmgr's ForCli interface
    // (B-side ability auth sees app A's HAP token); without it the ForCli interface rejects.
    const char *idEnv = getenv("ohos_cli_callerIdentity");
    std::string callerIdentity = (idEnv != nullptr) ? idEnv : "";

    // Host stub (agent->host) + the IAbilityConnection callback agentmgr calls back.
    auto hostStub = sptr<CliAgentConnectorStub>::MakeSptr();
    auto connection = sptr<CliAgentConnection>::MakeSptr();
    connection->SetHostStub(hostStub);
    connection->SetAgentId(agentId);
    want.SetParam(AgentRuntime::AGENTEXTENSIONHOSTPROXY_KEY, hostStub->AsObject());

    // A peer disconnect must be able to unblock the idle stdin poll; without the wakeup pipe,
    // do not enter the loop (fatal).
    if (!connection->HasWakeup()) {
        EmitAgentEvent(BuildErrorEvent("wakeup-pipe-failed",
            "pipe2 failed; cannot run long-lived session without disconnect wakeup", true));
        exitCode_ = 1;
        return ERR_INVALID_VALUE;
    }

    // CLI-only connect: agentmgr validates app A's real perms/foreground on the callerIdentity
    // token (anti-spoofed by uid-consistency) and enforces APP-type internally.
    int32_t ret = AgentRuntime::AgentManagerClient::GetInstance().ConnectAgentExtensionAbilityForCli(
        want, connection, callerIdentity);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::AA_TOOL, "ConnectAgentExtensionAbilityForCli failed, ret=%{public}d", ret);
        EmitAgentEvent(BuildConnectDoneEvent(false, agentId, ret, "ConnectAgentExtensionAbilityForCli failed"));
        exitCode_ = 1;
        return ERR_INVALID_VALUE;
    }

    // Wait for the async OnAbilityConnectDone callback (delivers the agent's IAgentReceiver proxy).
    sptr<AgentRuntime::IAgentReceiver> receiverProxy;
    sptr<IRemoteObject> hostStubObj;
    int32_t connResult = connection->WaitForConnect(CONNECT_WAIT_TIMEOUT_MS, receiverProxy, hostStubObj);
    if (connResult != ERR_OK) {
        // Timeout (-1): the callback never fired; other failures already emitted
        // connect-done(fail). cliMgr resolves execTool as failed via process exit.
        exitCode_ = 1;
        if (connResult == -1) {
            EmitAgentEvent(BuildErrorEvent("connect-timeout", "OnAbilityConnectDone did not fire", true));
        }
        return ERR_INVALID_VALUE;
    }

    // Long-lived stdin command loop; returns when the session ends. A fatal infra error
    // (poll/read failure) surfaces as non-zero exit, a normal exit is 0.
    if (RunCommandLoop(connection, receiverProxy, hostStubObj)) {
        exitCode_ = 1;
    }
    return ERR_OK;
}

}  // namespace AAFwk
}  // namespace OHOS
