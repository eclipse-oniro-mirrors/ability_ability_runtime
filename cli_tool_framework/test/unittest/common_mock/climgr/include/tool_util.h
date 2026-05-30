/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 */

#ifndef OHOS_ABILITY_RUNTIME_TOOL_UTIL_H
#define OHOS_ABILITY_RUNTIME_TOOL_UTIL_H

#include <access_token.h>
#include <memory>
#include <string>

#include "cli_session_info.h"
#include "want_params.h"

namespace OHOS {
namespace AppExecFwk {
struct SkillExecuteResult;
} // namespace AppExecFwk
namespace CliTool {
class ExecCmdParam;
class ExecOptions;
class ExecToolParam;
class SessionRecord;
class ToolInfo;

class ToolUtil {
public:
    static int32_t ValidateProperties(const ToolInfo &toolInfo, ExecToolParam &param,
        Security::AccessToken::AccessTokenID tokenId);
    static int32_t ValidateExecOptionsProperties(ExecOptions &options);
    static std::string GenerateCliSessionId(const std::string &name, std::shared_ptr<SessionRecord> record);
    static bool GenerateSandboxConfig(const ExecToolParam &param, Security::AccessToken::AccessTokenID tokenId,
        std::string &sandboxConfig, std::string &bundleName);
    static bool GenerateCmdSandboxConfig(const ExecCmdParam &param, AccessToken::AccessTokenID tokenId,
        std::string &sandboxConfig, std::string &bundleName);
    static void TransferToCmdParam(const ToolInfo &toolInfo, const AAFwk::WantParams &args, std::string &cmdLine);
    static bool IsSkillTool(const std::string &toolName);
    static void NormalizeSkillParamKeys(AAFwk::WantParams &args);
    static void ExpandArgsJsonString(AAFwk::WantParams &args);
    static std::shared_ptr<AAFwk::WantParams> FilterSkillArgs(const AAFwk::WantParams &args);
    static CliSessionInfo BuildSkillSessionInfo(const std::string &sessionId,
        int32_t resultCode, const AppExecFwk::SkillExecuteResult &skillResult);
};
} // namespace CliTool
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_TOOL_UTIL_H
