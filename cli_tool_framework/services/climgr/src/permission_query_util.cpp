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

#include "permission_query_util.h"
#include "cli_tool_data_manager.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace CliTool {
int32_t PermissionQueryUtil::BatchQueryPermissions(
    const std::vector<Command> &cmds,
    std::vector<CommandPermission> &cmdPermissions)
{
    cmdPermissions.clear();
    cmdPermissions.reserve(cmds.size());
    for (const auto &cmd : cmds) {
        std::vector<std::string> permissions;
        bool isLockScreenExecutionAllowed = false;
        int32_t ret = QuerySingleCommand(cmd, permissions, isLockScreenExecutionAllowed);
        int32_t queryRet;
        if (ret == ERR_OK) {
            queryRet = QueryResult::SUCCESS;
        } else if (ret == ERR_TOOL_NOT_EXIST) {
            queryRet = QueryResult::COMMAND_NOT_EXIST;
            permissions.clear();
        } else {
            queryRet = QueryResult::DB_ERROR;
            permissions.clear();
        }
        cmdPermissions.push_back(BuildCommandPermission(cmd, permissions, queryRet, isLockScreenExecutionAllowed));
    }
    TAG_LOGI(AAFwkTag::CLI_TOOL, "Batch query completed, total=%{public}zu", cmdPermissions.size());
    return ERR_OK;
}

int32_t PermissionQueryUtil::QuerySingleCommand(
    const Command &cmd,
    std::vector<std::string> &permissions,
    bool &isLockScreenExecutionAllowed)
{
    if (cmd.toolName.empty()) {
        TAG_LOGW(AAFwkTag::CLI_TOOL, "Tool name is empty");
        return ERR_TOOL_NOT_EXIST;
    }
    if (cmd.subCommand.empty()) {
        return QueryMainCommandPermission(cmd.toolName, permissions, isLockScreenExecutionAllowed);
    }
    return QuerySubCommandPermission(cmd.toolName, cmd.subCommand, permissions, isLockScreenExecutionAllowed);
}

int32_t PermissionQueryUtil::QueryMainCommandPermission(
    const std::string &toolName,
    std::vector<std::string> &permissions,
    bool &isLockScreenExecutionAllowed)
{
    ToolInfo toolInfo;
    int32_t ret = CliToolDataManager::GetInstance().GetToolByName(toolName, toolInfo);
    if (ret == ERR_NO_INIT) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "DB error when querying tool: %{public}s", toolName.c_str());
        return ERR_NO_INIT;
    } else if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Tool not found: %{public}s", toolName.c_str());
        return ERR_TOOL_NOT_EXIST;
    }
    permissions = toolInfo.requirePermissions;
    isLockScreenExecutionAllowed = toolInfo.isLockScreenExecutionAllowed;
    return ERR_OK;
}

int32_t PermissionQueryUtil::QuerySubCommandPermission(
    const std::string &toolName,
    const std::string &subCommand,
    std::vector<std::string> &permissions,
    bool &isLockScreenExecutionAllowed)
{
    ToolInfo toolInfo;
    int32_t ret = CliToolDataManager::GetInstance().GetToolByName(toolName, toolInfo);
    if (ret == ERR_NO_INIT) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "DB error when querying tool: %{public}s", toolName.c_str());
        return ERR_NO_INIT;
    } else if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Tool not found: %{public}s", toolName.c_str());
        return ERR_TOOL_NOT_EXIST;
    }
    if (!toolInfo.hasSubCommand) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Tool has no subcommand: %{public}s", toolName.c_str());
        return ERR_TOOL_NOT_EXIST;
    }
    auto it = toolInfo.subcommands.find(subCommand);
    if (it == toolInfo.subcommands.end()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Subcommand not found: %{public}s.%{public}s",
            toolName.c_str(), subCommand.c_str());
        return ERR_TOOL_NOT_EXIST;
    }
    permissions = it->second.requirePermissions;
    isLockScreenExecutionAllowed = toolInfo.isLockScreenExecutionAllowed;
    return ERR_OK;
}

CommandPermission PermissionQueryUtil::BuildCommandPermission(
    const Command &cmd,
    const std::vector<std::string> &permissions,
    int32_t queryRet,
    bool isLockScreenExecutionAllowed)
{
    CommandPermission cmdPerm;
    cmdPerm.cmd = cmd;
    cmdPerm.permissions = permissions;
    cmdPerm.queryRet = queryRet;
    cmdPerm.isLockScreenExecutionAllowed = isLockScreenExecutionAllowed;
    return cmdPerm;
}
} // namespace CliTool
} // namespace OHOS
