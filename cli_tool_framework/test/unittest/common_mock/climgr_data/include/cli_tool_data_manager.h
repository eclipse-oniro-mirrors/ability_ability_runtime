/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 */

#ifndef OHOS_ABILITY_RUNTIME_CLI_TOOL_DATA_MANAGER_COMMON_MOCK_SHIM_H
#define OHOS_ABILITY_RUNTIME_CLI_TOOL_DATA_MANAGER_COMMON_MOCK_SHIM_H

#include <string>
#include <vector>

#include "function_info.h"
#include "tool_info.h"
#include "tool_summary.h"

namespace OHOS {
namespace CliTool {
class CliToolDataManager {
public:
    CliToolDataManager() noexcept;
    ~CliToolDataManager();

    static CliToolDataManager &GetInstance();
    int32_t EnsureToolsLoaded();
    int32_t GetAllTools(std::vector<ToolInfo> &tools);
    int32_t GetAllToolsRawData(ToolsRawData &tools);
    int32_t QueryToolSummaries(std::vector<ToolSummary> &summaries);
    int32_t JsonArrayToTools(const std::string &jsonStr, std::vector<ToolInfo> &tools);
    int32_t GetToolByName(const std::string &name, ToolInfo &toolInfo);
};

class CliFunctionDataManager {
public:
    CliFunctionDataManager() noexcept;
    ~CliFunctionDataManager();

    static CliFunctionDataManager &GetInstance();
    int32_t RegisterFunction(const FunctionInfo &function);
    int32_t GetFunctionByName(const std::string &functionNamespace, const std::string &functionName,
        FunctionInfo &function);
    int32_t UnregisterFunction(const std::string &functionNamespace, const std::string &functionName);
    int32_t UnregisterIntentFunctionsByNamespace(const std::string &functionNamespace);
    int32_t GetAllFunctions(std::vector<FunctionInfo> &functions);
    int32_t EnsureFunctionsInitialized();
};
} // namespace CliTool
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_CLI_TOOL_DATA_MANAGER_COMMON_MOCK_SHIM_H
