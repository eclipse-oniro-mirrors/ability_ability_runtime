/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 */

#ifndef OHOS_ABILITY_RUNTIME_MOCK_ICLI_TOOL_MANAGER_H
#define OHOS_ABILITY_RUNTIME_MOCK_ICLI_TOOL_MANAGER_H

#include "cli_session_info.h"
#include "errors.h"
#include "exec_cmd_param.h"
#include "exec_result_wrap.h"
#include "exec_tool_param.h"
#include "function_info.h"
#include "function_result_wrap.h"
#include "icli_tool_data.h"
#include "icli_tool_manager_scheduler.h"
#include "invoke_function_param.h"
#include "iremote_broker.h"
#include "tool_info.h"
#include "tool_summary.h"

namespace OHOS {
namespace CliTool {
class ICliHookInterface : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"OHOS.CliTool.ICliHookInterface")
    virtual ErrCode BeforeCallTool(ExecToolParam &param) = 0;
    virtual ErrCode AfterCallTool(ExecResultWrap &execResultWrap) = 0;
    virtual ErrCode BeforeCallCmd(ExecCmdParam &param) = 0;
    virtual ErrCode AfterCallCmd(ExecResultWrap &execResultWrap) = 0;
};

class IFunctionHookInterface : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"OHOS.CliTool.IFunctionHookInterface")
    virtual ErrCode BeforeInvokeFunction(InvokeFunctionParam &param) = 0;
    virtual ErrCode AfterInvokeFunction(FunctionResultWrap &functionResultWrap) = 0;
};

class ICliToolManager : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"OHOS.CliTool.ICliToolManager")

    virtual int32_t GetAllToolSummaries(std::vector<ToolSummary> &summaries) = 0;
    virtual int32_t GetToolInfoByName(const std::string &name, ToolInfo &tool) = 0;
    virtual int32_t GetAllToolInfos(ToolsRawData &tools) = 0;
    virtual int32_t RegisterFunction(const FunctionInfo &function) = 0;
    virtual int32_t BatchRegisterFunctions(const FunctionsRawData &functions,
        int32_t &successCount) = 0;
    virtual int32_t GetFunctionInfo(const std::string &functionNamespace, const std::string &functionName,
        FunctionInfo &function) = 0;
    virtual int32_t UnregisterFunction(const std::string &functionNamespace, const std::string &functionName) = 0;
    virtual int32_t UnregisterIntentFunctionsByNamespace(const std::string &functionNamespace) = 0;
    virtual int32_t ResetNamespaceFunctions(const std::string &functionNamespace,
        const FunctionsRawData &functions, int32_t &successCount) = 0;
    virtual int32_t GetAllFunctions(FunctionsRawData &functions) = 0;
    virtual int32_t BatchRegisterFunctionsAsync(const FunctionsRawData &functions) = 0;
    virtual int32_t UnregisterIntentFunctionsByNamespaceAsync(const std::string &functionNamespace) = 0;
    virtual int32_t ResetNamespaceFunctionsAsync(const std::string &functionNamespace,
        const FunctionsRawData &functions) = 0;
    virtual int32_t ExecTool(const ExecToolParam &param, const std::string &eventId,
        const sptr<ICliToolManagerScheduler> &scheduler) = 0;
    virtual int32_t ExecCmd(const ExecCmdParam &param, const std::string &eventId,
        const sptr<ICliToolManagerScheduler> &scheduler, const std::string &subscriptionId) = 0;
    virtual int32_t SubscribeSession(const std::string &sessionId, const std::string &subscriptionId,
        const sptr<ICliToolManagerScheduler> &scheduler) = 0;
    virtual int32_t UnsubscribeSession(const std::string &sessionId, const std::string &subscriptionId) = 0;
    virtual int32_t ClearSession(const std::string &sessionId) = 0;
    virtual int32_t QuerySession(const std::string &sessionId, CliSessionInfo &session) = 0;
    virtual int32_t SendMessage(const std::string &sessionId, const std::string &inputText,
        const std::string &eventId, const sptr<ICliToolManagerScheduler> &scheduler) = 0;
    virtual int32_t BatchQueryPermissionBySubCommand(
        const std::vector<Command> &cmds, std::vector<CommandPermission> &cmdPermissions) = 0;
    virtual int32_t RegisterCliHook(const sptr<ICliHookInterface> &hook, int32_t activeMethods) = 0;
    virtual int32_t UnregisterCliHook(const sptr<ICliHookInterface> &hook) = 0;
    virtual int32_t RegisterFunctionHook(const sptr<IFunctionHookInterface> &hook, int32_t activeMethods) = 0;
    virtual int32_t UnregisterFunctionHook(const sptr<IFunctionHookInterface> &hook) = 0;
    virtual int32_t BeforeInvokeFunction(InvokeFunctionParam &param) = 0;
    virtual int32_t AfterInvokeFunction(FunctionResultWrap &functionResultWrap) = 0;
};
} // namespace CliTool
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_MOCK_ICLI_TOOL_MANAGER_H
