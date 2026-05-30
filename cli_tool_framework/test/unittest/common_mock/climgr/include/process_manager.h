/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 */

#ifndef OHOS_ABILITY_RUNTIME_PROCESS_MANAGER_H
#define OHOS_ABILITY_RUNTIME_PROCESS_MANAGER_H

#include <memory>
#include <string>
#include <sys/types.h>
#include <vector>

#include "errors.h"
#include "session_record.h"

namespace OHOS {
namespace CliTool {
class ExecCmdParam;
class ExecToolParam;
class ToolInfo;

class ProcessManager {
public:
    static ProcessManager &GetInstance();
    int32_t CreateChildProcess(const ExecToolParam &param, const std::string &sandboxConfig,
        const ToolInfo &toolInfo, std::shared_ptr<SessionRecord> record,
        const std::vector<std::shared_ptr<SessionRecord>> &fatherSessionRecords = {}) const;
    int32_t CreateShellProcess(const ExecCmdParam &param, const std::string &sandboxConfig,
        std::shared_ptr<SessionRecord> record,
        const std::vector<std::shared_ptr<SessionRecord>> &fatherSessionRecords = {}) const;
    bool Killpg(pid_t pid) const;
};
} // namespace CliTool
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_PROCESS_MANAGER_H
