/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 */

#ifndef OHOS_ABILITY_RUNTIME_MOCK_CLI_TOOL_MGR_SERVICE_H
#define OHOS_ABILITY_RUNTIME_MOCK_CLI_TOOL_MGR_SERVICE_H

#include "icli_tool_manager.h"
#include "iremote_stub.h"

namespace OHOS {
namespace CliTool {
class MockCliToolMgrService : public IRemoteStub<ICliToolManager> {
public:
    int32_t GetAllToolSummaries(std::vector<ToolSummary> &summaries) override;
    int32_t GetToolInfoByName(const std::string &name, ToolInfo &tool) override;
    int32_t GetAllToolInfos(ToolsRawData &tools) override;
    int32_t RegisterTool(const ToolInfo &tool) override;
    int32_t ExecTool(const ExecToolParam &param, const std::string &eventId,
        const sptr<ICliToolManagerScheduler> &scheduler) override;
    int32_t SubscribeSession(const std::string &sessionId, const std::string &subscriptionId,
        const sptr<ICliToolManagerScheduler> &scheduler) override;
    int32_t UnsubscribeSession(const std::string &sessionId, const std::string &subscriptionId) override;
    int32_t ClearSession(const std::string &sessionId) override;
    int32_t QuerySession(const std::string &sessionId, CliSessionInfo &session) override;
    int32_t SendMessage(const std::string &sessionId, const std::string &inputText,
        const std::string &eventId, const sptr<ICliToolManagerScheduler> &scheduler) override;
    int32_t BatchQueryPermissionBySubCommand(
        const std::vector<Command> &cmds, std::vector<CommandPermission> &cmdPermissions) override;
};
} // namespace CliTool
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_MOCK_CLI_TOOL_MGR_SERVICE_H
