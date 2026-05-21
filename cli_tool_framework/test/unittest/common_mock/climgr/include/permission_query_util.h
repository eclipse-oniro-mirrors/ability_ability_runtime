/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 */

#ifndef OHOS_ABILITY_RUNTIME_PERMISSION_QUERY_UTIL_H
#define OHOS_ABILITY_RUNTIME_PERMISSION_QUERY_UTIL_H

#include <vector>

#include "cli_error_code.h"
#include "icli_tool_data.h"

namespace OHOS {
namespace CliTool {
namespace QueryResult {
    constexpr int32_t SUCCESS = 0;
    constexpr int32_t COMMAND_NOT_EXIST = 1;
    constexpr int32_t DB_ERROR = 2;
} // namespace QueryResult

class PermissionQueryUtil {
public:
    static int32_t BatchQueryPermissions(
        const std::vector<Command> &cmds,
        std::vector<CommandPermission> &cmdPermissions);
};
} // namespace CliTool
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_PERMISSION_QUERY_UTIL_H
