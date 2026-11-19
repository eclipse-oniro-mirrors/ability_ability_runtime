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

#ifndef OHOS_ABILITY_RUNTIME_PERMISSION_QUERY_UTIL_H
#define OHOS_ABILITY_RUNTIME_PERMISSION_QUERY_UTIL_H

#include <string>
#include <vector>
#include "cli_error_code.h"
#include "icli_tool_data.h"

namespace OHOS {
namespace CliTool {

/**
 * @brief Query result codes for batch permission query
 */
namespace QueryResult {
    constexpr int32_t SUCCESS = 0;
    constexpr int32_t COMMAND_NOT_EXIST = 1;
    constexpr int32_t DB_ERROR = 2;
} // namespace QueryResult

/**
 * @brief CLI tool permission query utility class
 * Provides static methods to handle command permission query logic
 */
class PermissionQueryUtil {
public:
    /**
     * @brief Batch query command permissions
     * @param cmds Command list to query
     * @param cmdPermissions Output vector of CommandPermission query results
     * @return ERR_OK on success
     */
    static int32_t BatchQueryPermissions(
        const std::vector<Command> &cmds,
        std::vector<CommandPermission> &cmdPermissions);

private:
    /**
     * @brief Query permissions for a single command
     * @param cmd Command to query
     * @param permissions Output vector of permission strings
     * @param isLockScreenExecutionAllowed Output whether lock screen execution is enabled
     * @return ERR_OK on success
     *         ERR_TOOL_NOT_EXIST when tool not found
     *         ERR_NO_INIT on database error
     */
    static int32_t QuerySingleCommand(
        const Command &cmd,
        std::vector<std::string> &permissions,
        bool &isLockScreenExecutionAllowed);

    /**
     * @brief Query permissions for main command (no subcommand)
     * @param toolName Tool name
     * @param permissions Output vector of permission strings
     * @param isLockScreenExecutionAllowed Output whether lock screen execution is enabled
     * @return ERR_OK on success
     *         ERR_TOOL_NOT_EXIST when tool not found
     *         ERR_NO_INIT on database error
     */
    static int32_t QueryMainCommandPermission(
        const std::string &toolName,
        std::vector<std::string> &permissions,
        bool &isLockScreenExecutionAllowed);

    /**
     * @brief Query permissions for subcommand
     * @param toolName Tool name
     * @param subCommand Subcommand name
     * @param permissions Output vector of permission strings
     * @param isLockScreenExecutionAllowed Output whether lock screen execution is enabled
     * @return ERR_OK on success
     *         ERR_TOOL_NOT_EXIST when tool or subcommand not found
     *         ERR_NO_INIT on database error
     */
    static int32_t QuerySubCommandPermission(
        const std::string &toolName,
        const std::string &subCommand,
        std::vector<std::string> &permissions,
        bool &isLockScreenExecutionAllowed);

    /**
     * @brief Build CommandPermission result object
     * @param cmd Command
     * @param permissions Permission list
     * @param queryRet Query result code
     * @param isLockScreenExecutionAllowed Whether lock screen execution is enabled
     * @return CommandPermission object
     */
    static CommandPermission BuildCommandPermission(
        const Command &cmd,
        const std::vector<std::string> &permissions,
        int32_t queryRet,
        bool isLockScreenExecutionAllowed);
};
} // namespace CliTool
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_PERMISSION_QUERY_UTIL_H
