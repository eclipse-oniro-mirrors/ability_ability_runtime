/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 */

#ifndef OHOS_ABILITY_RUNTIME_CLI_TOOL_DATA_MANAGER_COMMON_MOCK_SHIM_H
#define OHOS_ABILITY_RUNTIME_CLI_TOOL_DATA_MANAGER_COMMON_MOCK_SHIM_H

#include <string>
#include <vector>

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
    int32_t RegisterTool(const ToolInfo &tool);
    int32_t JsonArrayToTools(const std::string &jsonStr, std::vector<ToolInfo> &tools);
    int32_t GetToolByName(const std::string &name, ToolInfo &toolInfo);
};
} // namespace CliTool
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_CLI_TOOL_DATA_MANAGER_COMMON_MOCK_SHIM_H
