/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 */

#include "cli_mgr_service_mock.h"

#include <memory>
#include <string>
#include <vector>

#include "ccm_util.h"
#include "cli_error_code.h"
#include "cli_tool_data_manager.h"
#include "event_dispatcher.h"
#include "exec_result.h"
#include "io_monitor.h"
#include "permission_query_util.h"
#include "permission_util.h"
#include "process_manager.h"
#include "skill/skill_execute_result.h"
#include "tool_info.h"
#include "tool_util.h"

namespace OHOS {
namespace CliTool {
int32_t CliMgrServiceMock::createChildProcessResult = ERR_OK;
bool CliMgrServiceMock::killpgResult = true;
int32_t CliMgrServiceMock::registerSessionResult = ERR_OK;
int32_t CliMgrServiceMock::unregisterSessionCount = 0;
int32_t CliMgrServiceMock::stopCount = 0;
int32_t CliMgrServiceMock::sendMessageCount = 0;
int32_t CliMgrServiceMock::ensureToolsLoadedResult = ERR_OK;
int32_t CliMgrServiceMock::getToolByNameResult = ERR_OK;
int32_t CliMgrServiceMock::connectAppMgrResult = 0;
int32_t CliMgrServiceMock::registerAppObserverResult = ERR_OK;
int32_t CliMgrServiceMock::querySkillTypeResult = ERR_OK;
int32_t CliMgrServiceMock::executeSkillResult = ERR_OK;
int32_t CliMgrServiceMock::skillType = -1;
std::string CliMgrServiceMock::lastSkillName;
bool CliMgrServiceMock::toolHasSubCommand = false;
std::string CliMgrServiceMock::subCommandName = "build";
std::vector<std::string> CliMgrServiceMock::toolPermissions = {};
std::vector<std::string> CliMgrServiceMock::subCommandPermissions = {};

void CliMgrServiceMock::Reset()
{
    createChildProcessResult = ERR_OK;
    killpgResult = true;
    registerSessionResult = ERR_OK;
    unregisterSessionCount = 0;
    stopCount = 0;
    sendMessageCount = 0;
    ensureToolsLoadedResult = ERR_OK;
    getToolByNameResult = ERR_OK;
    connectAppMgrResult = 0;
    registerAppObserverResult = ERR_OK;
    querySkillTypeResult = ERR_OK;
    executeSkillResult = ERR_OK;
    skillType = -1;
    lastSkillName.clear();
    toolHasSubCommand = false;
    subCommandName = "build";
    toolPermissions.clear();
    subCommandPermissions.clear();
}

ProcessManager &ProcessManager::GetInstance()
{
    static ProcessManager instance;
    return instance;
}

int32_t ProcessManager::CreateChildProcess(const ExecToolParam &, const std::string &, const ToolInfo &,
    std::shared_ptr<SessionRecord> record) const
{
    if (record != nullptr) {
        record->processId = 1001;
        record->stdoutPipe[0] = -1;
        record->stderrPipe[0] = -1;
        record->stdinPipe[1] = -1;
    }
    return CliMgrServiceMock::createChildProcessResult;
}

bool ProcessManager::Killpg(pid_t) const
{
    return CliMgrServiceMock::killpgResult;
}

std::shared_ptr<IOMonitor> IOMonitor::Create()
{
    return std::make_shared<IOMonitor>();
}

bool IOMonitor::Start()
{
    return true;
}

void IOMonitor::Stop()
{
    CliMgrServiceMock::stopCount++;
}

bool IOMonitor::RegisterSession(const std::string &, int, int, int)
{
    return CliMgrServiceMock::registerSessionResult == ERR_OK;
}

void IOMonitor::UnregisterSession(const std::string &)
{
    CliMgrServiceMock::unregisterSessionCount++;
}

void IOMonitor::SetOutputCallback(OutputCallback) {}
void IOMonitor::SetInputReplyCallback(InputReplyCallback) {}
void IOMonitor::SetSessionClosedCallback(SessionClosedCallback) {}
void IOMonitor::SetSessionDrainedCallback(SessionDrainedCallback) {}

void IOMonitor::SendMessage(const std::string &, const std::string &, const std::string &)
{
    CliMgrServiceMock::sendMessageCount++;
}

CliToolDataManager &CliToolDataManager::GetInstance()
{
    static CliToolDataManager instance;
    return instance;
}

int32_t CliToolDataManager::EnsureToolsLoaded()
{
    return CliMgrServiceMock::ensureToolsLoadedResult;
}

int32_t CliToolDataManager::GetAllToolsRawData(ToolsRawData &)
{
    return ERR_OK;
}

int32_t CliToolDataManager::QueryToolSummaries(std::vector<ToolSummary> &)
{
    return ERR_OK;
}

int32_t CliToolDataManager::GetToolByName(const std::string &name, ToolInfo &toolInfo)
{
    if (CliMgrServiceMock::getToolByNameResult != ERR_OK) {
        return CliMgrServiceMock::getToolByNameResult;
    }
    toolInfo.name = name;
    toolInfo.version = "1.0.0";
    toolInfo.description = "mock tool";
    toolInfo.executablePath = "/system/bin/mock";
    toolInfo.requirePermissions = CliMgrServiceMock::toolPermissions;
    toolInfo.inputSchema = "{}";
    toolInfo.outputSchema = "{}";
    toolInfo.hasSubCommand = CliMgrServiceMock::toolHasSubCommand;
    if (toolInfo.hasSubCommand) {
        SubCommandInfo subCommand;
        subCommand.description = "mock subcommand";
        subCommand.requirePermissions = CliMgrServiceMock::subCommandPermissions;
        subCommand.inputSchema = "{}";
        subCommand.outputSchema = "{}";
        toolInfo.subcommands[CliMgrServiceMock::subCommandName] = subCommand;
    }
    return ERR_OK;
}

bool PermissionUtil::VerifyAccessToken(Security::AccessToken::AccessTokenID, const std::vector<std::string> &)
{
    return true;
}

bool PermissionUtil::VerifyAccessToken(Security::AccessToken::AccessTokenID, const std::string &)
{
    return true;
}

CcmUtil &CcmUtil::GetInstance()
{
    static CcmUtil instance;
    return instance;
}

int32_t CcmUtil::GetCliConcurrencyLimit()
{
    return 10;
}

EventDispatcher &EventDispatcher::GetInstance()
{
    static EventDispatcher instance;
    return instance;
}

bool EventDispatcher::RegisterScheduler(int32_t, const sptr<ICliToolManagerScheduler> &)
{
    return true;
}

void EventDispatcher::UnregisterScheduler(int32_t) {}

bool EventDispatcher::RegisterSubscriber(const std::string &, const std::string &, int32_t)
{
    return true;
}

bool EventDispatcher::UnregisterSubscriber(const std::string &, const std::string &, int32_t)
{
    return true;
}

void EventDispatcher::DispatchIOEvent(const std::string &, const std::string &, const std::string &) {}
void EventDispatcher::DispatchErrorEvent(const std::string &, const std::string &) {}
void EventDispatcher::DispatchExitEvent(const std::string &, int32_t) {}

bool EventDispatcher::DispatchInputReplyEvent(int32_t, const std::string &, int32_t)
{
    return true;
}

bool EventDispatcher::DispatchExecToolReplyEvent(int32_t, const std::string &, int32_t, const CliSessionInfo &)
{
    return true;
}

void EventDispatcher::ClearSessionSubscribers(const std::string &) {}
void EventDispatcher::ClearAll() {}

int32_t PermissionQueryUtil::BatchQueryPermissions(
    const std::vector<Command> &, std::vector<CommandPermission> &cmdPermissions)
{
    cmdPermissions.clear();
    return ERR_OK;
}

int32_t ToolUtil::ValidateProperties(const ToolInfo &, ExecToolParam &, Security::AccessToken::AccessTokenID)
{
    return ERR_OK;
}

std::string ToolUtil::GenerateCliSessionId(const std::string &name, std::shared_ptr<SessionRecord> record)
{
    if (record != nullptr) {
        record->startTime = 1000;
    }
    return name + "_session";
}

bool ToolUtil::GenerateSandboxConfig(const ExecToolParam &, Security::AccessToken::AccessTokenID,
    std::string &sandboxConfig, std::string &bundleName)
{
    sandboxConfig = "{}";
    bundleName = "bundle.mock";
    return true;
}

void ToolUtil::TransferToCmdParam(const ToolInfo &, const AAFwk::WantParams &, std::string &) {}

bool ToolUtil::IsSkillTool(const std::string &toolName)
{
    return toolName == "ohos-arkTSScript";
}

void ToolUtil::NormalizeSkillParamKeys(AAFwk::WantParams &) {}
void ToolUtil::ExpandArgsJsonString(AAFwk::WantParams &) {}

std::shared_ptr<AAFwk::WantParams> ToolUtil::FilterSkillArgs(const AAFwk::WantParams &args)
{
    return std::make_shared<AAFwk::WantParams>(args);
}

CliSessionInfo ToolUtil::BuildSkillSessionInfo(
    const std::string &sessionId, int32_t resultCode, const AppExecFwk::SkillExecuteResult &result)
{
    CliSessionInfo session;
    session.sessionId = sessionId;
    session.status = resultCode == ERR_OK ? "completed" : "failed";
    session.result = std::make_shared<ExecResult>();
    session.result->exitCode = resultCode;
    if (result.result != nullptr) {
        session.result->outputText = result.result->ToString();
    }
    return session;
}
} // namespace CliTool
} // namespace OHOS
