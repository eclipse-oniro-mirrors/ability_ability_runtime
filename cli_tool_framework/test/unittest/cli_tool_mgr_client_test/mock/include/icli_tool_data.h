/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 */

#ifndef OHOS_ABILITY_RUNTIME_MOCK_ICLI_TOOL_DATA_H
#define OHOS_ABILITY_RUNTIME_MOCK_ICLI_TOOL_DATA_H

#include <string>
#include <vector>

namespace OHOS {
namespace CliTool {
struct Command {
    std::string toolName;
    std::string subCommand;
};

struct CommandPermission {
    Command cmd;
    std::vector<std::string> permissions;
    bool isLockScreenExecutionAllowed = false;
    int32_t queryRet = 0;
};
} // namespace CliTool
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_MOCK_ICLI_TOOL_DATA_H
