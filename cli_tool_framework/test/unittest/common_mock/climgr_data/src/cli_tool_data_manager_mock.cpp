/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 */

#include "cli_tool_data_manager_mock.h"

#include "../include/cli_tool_data_manager.h"
#include "cli_error_code.h"
#include "tool_info.h"
#include "function_info.h"

namespace OHOS {
namespace CliTool {
int32_t CliToolDataManagerMock::getToolByNameResult = ERR_TOOL_NOT_EXIST;
bool CliToolDataManagerMock::toolHasSubCommand = false;
bool CliToolDataManagerMock::toolIsLockScreenExecEnabled = false;
std::string CliToolDataManagerMock::subCommandName = "build";
std::vector<std::string> CliToolDataManagerMock::toolPermissions = {};
std::vector<std::string> CliToolDataManagerMock::subCommandPermissions = {};

void CliToolDataManagerMock::Reset()
{
    getToolByNameResult = ERR_TOOL_NOT_EXIST;
    toolHasSubCommand = false;
    toolIsLockScreenExecEnabled = false;
    subCommandName = "build";
    toolPermissions.clear();
    subCommandPermissions.clear();
}

// CliFunctionDataManagerMock
int32_t CliFunctionDataManagerMock::registerFunctionResult = ERR_OK;
int32_t CliFunctionDataManagerMock::unregisterFunctionResult = ERR_OK;
int32_t CliFunctionDataManagerMock::getFunctionResult = ERR_OK;
int32_t CliFunctionDataManagerMock::getAllFunctionsResult = ERR_OK;
int32_t CliFunctionDataManagerMock::unregisterByNamespaceResult = ERR_OK;

void CliFunctionDataManagerMock::Reset()
{
    registerFunctionResult = ERR_OK;
    unregisterFunctionResult = ERR_OK;
    getFunctionResult = ERR_OK;
    getAllFunctionsResult = ERR_OK;
    unregisterByNamespaceResult = ERR_OK;
}

CliToolDataManager::CliToolDataManager() noexcept = default;

CliToolDataManager::~CliToolDataManager() = default;

CliToolDataManager &CliToolDataManager::GetInstance()
{
    static CliToolDataManager instance;
    return instance;
}

int32_t CliToolDataManager::EnsureToolsLoaded()
{
    return ERR_OK;
}

int32_t CliToolDataManager::GetAllTools(std::vector<ToolInfo> &tools)
{
    tools.clear();
    return ERR_OK;
}

int32_t CliToolDataManager::GetAllToolsRawData(ToolsRawData &)
{
    return ERR_OK;
}

int32_t CliToolDataManager::QueryToolSummaries(std::vector<ToolSummary> &summaries)
{
    summaries.clear();
    return ERR_OK;
}

int32_t CliToolDataManager::JsonArrayToTools(const std::string &, std::vector<ToolInfo> &tools)
{
    tools.clear();
    return ERR_OK;
}

int32_t CliToolDataManager::GetToolByName(const std::string &name, ToolInfo &toolInfo)
{
    if (CliToolDataManagerMock::getToolByNameResult != ERR_OK) {
        return CliToolDataManagerMock::getToolByNameResult;
    }
    toolInfo.name = name;
    toolInfo.version = "1.0.0";
    toolInfo.description = "mock tool";
    toolInfo.executablePath = "/system/bin/mock";
    toolInfo.requirePermissions = CliToolDataManagerMock::toolPermissions;
    toolInfo.inputSchema = "{}";
    toolInfo.outputSchema = "{}";
    toolInfo.hasSubCommand = CliToolDataManagerMock::toolHasSubCommand;
    toolInfo.isLockScreenExecutionAllowed = CliToolDataManagerMock::toolIsLockScreenExecEnabled;
    if (toolInfo.hasSubCommand) {
        SubCommandInfo subCommand;
        subCommand.description = "mock subcommand";
        subCommand.requirePermissions = CliToolDataManagerMock::subCommandPermissions;
        subCommand.inputSchema = "{}";
        subCommand.outputSchema = "{}";
        toolInfo.subcommands[CliToolDataManagerMock::subCommandName] = subCommand;
    }
    return ERR_OK;
}

// CliFunctionDataManager mock implementations
CliFunctionDataManager::CliFunctionDataManager() noexcept = default;

CliFunctionDataManager::~CliFunctionDataManager() = default;

CliFunctionDataManager &CliFunctionDataManager::GetInstance()
{
    static CliFunctionDataManager instance;
    return instance;
}

int32_t CliFunctionDataManager::RegisterFunction(const FunctionInfo &)
{
    return CliFunctionDataManagerMock::registerFunctionResult;
}

int32_t CliFunctionDataManager::GetFunctionByName(const std::string &, const std::string &, FunctionInfo &)
{
    return CliFunctionDataManagerMock::getFunctionResult;
}

int32_t CliFunctionDataManager::UnregisterFunction(const std::string &, const std::string &)
{
    return CliFunctionDataManagerMock::unregisterFunctionResult;
}

int32_t CliFunctionDataManager::UnregisterIntentFunctionsByNamespace(const std::string &)
{
    return CliFunctionDataManagerMock::unregisterByNamespaceResult;
}

int32_t CliFunctionDataManager::GetAllFunctions(std::vector<FunctionInfo> &functions)
{
    functions.clear();
    return CliFunctionDataManagerMock::getAllFunctionsResult;
}

int32_t CliFunctionDataManager::BatchRegisterFunctions(const std::vector<FunctionInfo> &functions,
    int32_t &successCount)
{
    successCount = functions.size();
    return ERR_OK;
}

int32_t CliFunctionDataManager::EnsureFunctionsInitialized()
{
    return ERR_OK;
}

} // namespace CliTool
} // namespace OHOS
