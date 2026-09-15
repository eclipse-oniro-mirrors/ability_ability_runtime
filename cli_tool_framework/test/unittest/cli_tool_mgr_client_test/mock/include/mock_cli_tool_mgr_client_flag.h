/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 */

#ifndef OHOS_ABILITY_RUNTIME_MOCK_CLI_TOOL_MGR_CLIENT_FLAG_H
#define OHOS_ABILITY_RUNTIME_MOCK_CLI_TOOL_MGR_CLIENT_FLAG_H

#include <string>
#include <vector>

#include "cli_session_info.h"
#include "function_info.h"
#include "function_result_wrap.h"
#include "icli_tool_data.h"
#include "invoke_function_param.h"
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
    static int32_t retRegisterFunction;
    static int32_t retBatchRegisterFunctions;
    static int32_t batchRegisterFunctionsSuccessCount;
    static int32_t retResetNamespaceFunctions;
    static int32_t resetNamespaceFunctionsSuccessCount;
    static int32_t retGetFunctionInfo;
    static int32_t retUnregisterFunction;
    static int32_t retGetAllFunctions;
    static int32_t retExecTool;
    static int32_t retExecCmd;
    static int32_t retSubscribeSession;
    static int32_t retUnsubscribeSession;
    static int32_t retClearSession;
    static int32_t retQuerySession;
    static int32_t retSendMessage;
    static int32_t retBatchQueryPermission;
    static int32_t retLoadSystemAbility;
    static int32_t retRegisterCliHook;
    static int32_t retUnregisterCliHook;
    static int32_t retRegisterFunctionHook;
    static int32_t retUnregisterFunctionHook;
    static int32_t retBeforeInvokeFunction;
    static int32_t retAfterInvokeFunction;
    static int32_t registerCliHookCount;
    static int32_t unregisterCliHookCount;
    static int32_t registerFunctionHookCount;
    static int32_t unregisterFunctionHookCount;
    static int32_t beforeInvokeFunctionCount;
    static int32_t afterInvokeFunctionCount;
    static int32_t lastHookActiveMethods;
    static std::string lastInvokeFunctionNamespace;
    static std::string lastInvokeFunctionName;
    static bool nullSystemAbility;
    static bool shouldCallback;
    static sptr<IRemoteObject> cliToolMgr;
    static std::string lastEventId;
    static std::string lastSubscriptionId;
    static std::vector<ToolInfo> toolInfos;
    static std::vector<FunctionInfo> functionInfos;
    static std::vector<ToolSummary> summaries;
    static CliSessionInfo querySession;
    static std::vector<CommandPermission> commandPermissions;

    static void Reset();
};
} // namespace CliTool
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_MOCK_CLI_TOOL_MGR_CLIENT_FLAG_H
