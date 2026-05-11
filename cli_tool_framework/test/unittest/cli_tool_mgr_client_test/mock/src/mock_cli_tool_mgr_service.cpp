/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 */

#include "mock_cli_tool_mgr_service.h"

#include "cli_error_code.h"
#include "mock_cli_tool_mgr_client_flag.h"

namespace OHOS {
namespace CliTool {
int32_t CliToolMgrClientFlag::retGetAllToolSummaries = ERR_OK;
int32_t CliToolMgrClientFlag::retGetToolInfoByName = ERR_OK;
int32_t CliToolMgrClientFlag::retGetAllToolInfos = ERR_OK;
int32_t CliToolMgrClientFlag::retRegisterTool = ERR_OK;
int32_t CliToolMgrClientFlag::retExecTool = ERR_OK;
int32_t CliToolMgrClientFlag::retSubscribeSession = ERR_OK;
int32_t CliToolMgrClientFlag::retUnsubscribeSession = ERR_OK;
int32_t CliToolMgrClientFlag::retClearSession = ERR_OK;
int32_t CliToolMgrClientFlag::retQuerySession = ERR_OK;
int32_t CliToolMgrClientFlag::retSendMessage = ERR_OK;
int32_t CliToolMgrClientFlag::retRegisterScheduler = ERR_OK;
int32_t CliToolMgrClientFlag::retBatchQueryPermission = ERR_OK;
int32_t CliToolMgrClientFlag::retLoadSystemAbility = ERR_OK;
bool CliToolMgrClientFlag::nullSystemAbility = false;
bool CliToolMgrClientFlag::shouldCallback = true;
sptr<IRemoteObject> CliToolMgrClientFlag::cliToolMgr = nullptr;
std::string CliToolMgrClientFlag::lastEventId;
std::string CliToolMgrClientFlag::lastSubscriptionId;
std::vector<ToolInfo> CliToolMgrClientFlag::toolInfos;
std::vector<ToolSummary> CliToolMgrClientFlag::summaries;
CliSessionInfo CliToolMgrClientFlag::querySession;
std::vector<CommandPermission> CliToolMgrClientFlag::commandPermissions;

void CliToolMgrClientFlag::Reset()
{
    retGetAllToolSummaries = ERR_OK;
    retGetToolInfoByName = ERR_OK;
    retGetAllToolInfos = ERR_OK;
    retRegisterTool = ERR_OK;
    retExecTool = ERR_OK;
    retSubscribeSession = ERR_OK;
    retUnsubscribeSession = ERR_OK;
    retClearSession = ERR_OK;
    retQuerySession = ERR_OK;
    retSendMessage = ERR_OK;
    retRegisterScheduler = ERR_OK;
    retBatchQueryPermission = ERR_OK;
    retLoadSystemAbility = ERR_OK;
    nullSystemAbility = false;
    shouldCallback = true;
    cliToolMgr = nullptr;
    lastEventId.clear();
    lastSubscriptionId.clear();
    toolInfos.clear();
    summaries.clear();
    querySession = {};
    commandPermissions.clear();
}

int32_t MockCliToolMgrService::GetAllToolSummaries(std::vector<ToolSummary> &summaries)
{
    summaries = CliToolMgrClientFlag::summaries;
    return CliToolMgrClientFlag::retGetAllToolSummaries;
}

int32_t MockCliToolMgrService::GetToolInfoByName(const std::string &, ToolInfo &tool)
{
    if (!CliToolMgrClientFlag::toolInfos.empty()) {
        tool = CliToolMgrClientFlag::toolInfos.front();
    }
    return CliToolMgrClientFlag::retGetToolInfoByName;
}

int32_t MockCliToolMgrService::GetAllToolInfos(ToolsRawData &tools)
{
    if (CliToolMgrClientFlag::retGetAllToolInfos == ERR_OK) {
        ToolsRawData::FromToolInfoVec(CliToolMgrClientFlag::toolInfos, tools);
    }
    return CliToolMgrClientFlag::retGetAllToolInfos;
}

int32_t MockCliToolMgrService::RegisterTool(const ToolInfo &)
{
    return CliToolMgrClientFlag::retRegisterTool;
}

int32_t MockCliToolMgrService::ExecTool(const ExecToolParam &, const std::string &eventId)
{
    CliToolMgrClientFlag::lastEventId = eventId;
    return CliToolMgrClientFlag::retExecTool;
}

int32_t MockCliToolMgrService::SubscribeSession(const std::string &, const std::string &subscriptionId)
{
    CliToolMgrClientFlag::lastSubscriptionId = subscriptionId;
    return CliToolMgrClientFlag::retSubscribeSession;
}

int32_t MockCliToolMgrService::UnsubscribeSession(const std::string &, const std::string &)
{
    return CliToolMgrClientFlag::retUnsubscribeSession;
}

int32_t MockCliToolMgrService::ClearSession(const std::string &)
{
    return CliToolMgrClientFlag::retClearSession;
}

int32_t MockCliToolMgrService::QuerySession(const std::string &, CliSessionInfo &session)
{
    session = CliToolMgrClientFlag::querySession;
    return CliToolMgrClientFlag::retQuerySession;
}

int32_t MockCliToolMgrService::SendMessage(const std::string &, const std::string &, const std::string &eventId)
{
    CliToolMgrClientFlag::lastEventId = eventId;
    return CliToolMgrClientFlag::retSendMessage;
}

int32_t MockCliToolMgrService::RegisterScheduler(const sptr<ICliToolManagerScheduler> &)
{
    return CliToolMgrClientFlag::retRegisterScheduler;
}

int32_t MockCliToolMgrService::UnregisterScheduler()
{
    return ERR_OK;
}

int32_t MockCliToolMgrService::BatchQueryPermissionBySubCommand(
    const std::vector<Command> &, std::vector<CommandPermission> &cmdPermissions)
{
    cmdPermissions = CliToolMgrClientFlag::commandPermissions;
    return CliToolMgrClientFlag::retBatchQueryPermission;
}
} // namespace CliTool
} // namespace OHOS
