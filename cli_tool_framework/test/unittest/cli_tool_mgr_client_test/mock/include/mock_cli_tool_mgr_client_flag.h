/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 */

#ifndef OHOS_ABILITY_RUNTIME_MOCK_CLI_TOOL_MGR_CLIENT_FLAG_H
#define OHOS_ABILITY_RUNTIME_MOCK_CLI_TOOL_MGR_CLIENT_FLAG_H

#include <string>
#include <vector>

#include "cli_session_info.h"
#include "icli_tool_data.h"
#include "iremote_object.h"
#include "tool_info.h"
#include "tool_summary.h"

namespace OHOS {
namespace CliTool {
class CliToolMgrClientFlag {
public:
    static int32_t retGetAllToolSummaries;
    static int32_t retGetToolInfoByName;
    static int32_t retGetAllToolInfos;
    static int32_t retRegisterTool;
    static int32_t retExecTool;
    static int32_t retSubscribeSession;
    static int32_t retUnsubscribeSession;
    static int32_t retClearSession;
    static int32_t retQuerySession;
    static int32_t retSendMessage;
    static int32_t retRegisterScheduler;
    static int32_t retBatchQueryPermission;
    static int32_t retLoadSystemAbility;
    static bool nullSystemAbility;
    static bool shouldCallback;
    static sptr<IRemoteObject> cliToolMgr;
    static std::string lastEventId;
    static std::string lastSubscriptionId;
    static std::vector<ToolInfo> toolInfos;
    static std::vector<ToolSummary> summaries;
    static CliSessionInfo querySession;
    static std::vector<CommandPermission> commandPermissions;

    static void Reset();
};
} // namespace CliTool
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_MOCK_CLI_TOOL_MGR_CLIENT_FLAG_H
