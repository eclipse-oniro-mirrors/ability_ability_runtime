/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 */

#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <vector>

#define private public
#include "cli_event_reply_manager.h"
#include "cli_session_subscription_manager.h"
#include "cli_tool_mgr_client.h"
#undef private

#include "cli_error_code.h"
#include "mock_cli_tool_mgr_client_flag.h"
#include "mock_cli_tool_mgr_service.h"

using namespace testing::ext;

namespace OHOS {
namespace CliTool {
namespace {
ToolInfo BuildToolInfo(const std::string &name)
{
    ToolInfo tool;
    tool.name = name;
    tool.version = "1.0.0";
    tool.description = "mock tool";
    tool.executablePath = "/system/bin/mock";
    tool.inputSchema = "{}";
    tool.outputSchema = "{}";
    return tool;
}

ToolSummary BuildToolSummary(const std::string &name)
{
    ToolSummary summary;
    summary.name = name;
    summary.version = "1.0.0";
    summary.description = "mock summary";
    return summary;
}
} // namespace

class MockSessionCallback : public SessionEventCallback {
public:
    void OnToolEvent(const std::string &, const std::string &, const CliToolEvent &event) override
    {
        eventCount++;
        lastEventType = event.type;
    }

    int32_t eventCount = 0;
    std::string lastEventType;
};

class CliToolMGRClientTest : public testing::Test {
public:
    void SetUp() override
    {
        CliToolMgrClientFlag::Reset();
        CliEventReplyManager::GetInstance().ClearAllEvent();
        CliSessionSubscriptionManager::GetInstance().ClearAllSubscriptions();
        auto &client = CliToolMGRClient::GetInstance();
        client.ClearProxy();
        client.loadSaFinished_ = false;
        client.serviceDeathHandlers_.clear();
    }

    void TearDown() override
    {
        auto &client = CliToolMGRClient::GetInstance();
        client.ClearProxy();
        CliToolMgrClientFlag::Reset();
    }

    sptr<MockCliToolMgrService> SetMockService()
    {
        auto mockService = sptr<MockCliToolMgrService>::MakeSptr();
        CliToolMgrClientFlag::cliToolMgr = mockService->AsObject();
        CliToolMGRClient::GetInstance().cliToolMgr_ = mockService;
        return mockService;
    }
};

/**
 * @tc.name: GetInstance_0100
 * @tc.desc: Test GetInstance returns singleton instance
 * @tc.type: FUNC
 */
HWTEST_F(CliToolMGRClientTest, GetInstance_0100, TestSize.Level1)
{
    auto &instance1 = CliToolMGRClient::GetInstance();
    auto &instance2 = CliToolMGRClient::GetInstance();

    EXPECT_EQ(&instance1, &instance2);
}

/**
 * @tc.name: GetCliToolMgrProxy_0100
 * @tc.desc: Test cached proxy, null system ability and successful load branches
 * @tc.type: FUNC
 */
HWTEST_F(CliToolMGRClientTest, GetCliToolMgrProxy_0100, TestSize.Level1)
{
    auto &client = CliToolMGRClient::GetInstance();
    auto mockService = SetMockService();
    EXPECT_EQ(client.GetCliToolMgrProxy()->AsObject(), mockService->AsObject());

    client.ClearProxy();
    CliToolMgrClientFlag::nullSystemAbility = true;
    EXPECT_EQ(client.GetCliToolMgrProxy(), nullptr);

    CliToolMgrClientFlag::nullSystemAbility = false;
    CliToolMgrClientFlag::cliToolMgr = mockService->AsObject();
    EXPECT_EQ(client.GetCliToolMgrProxy()->AsObject(), mockService->AsObject());
}

/**
 * @tc.name: LoadCliToolMgrService_0100
 * @tc.desc: Test load failure, timeout and success branches
 * @tc.type: FUNC
 */
HWTEST_F(CliToolMGRClientTest, LoadCliToolMgrService_0100, TestSize.Level1)
{
    auto &client = CliToolMGRClient::GetInstance();
    CliToolMgrClientFlag::nullSystemAbility = true;
    EXPECT_FALSE(client.LoadCliToolMgrService());

    CliToolMgrClientFlag::nullSystemAbility = false;
    CliToolMgrClientFlag::retLoadSystemAbility = ERR_INVALID_VALUE;
    EXPECT_FALSE(client.LoadCliToolMgrService());

    CliToolMgrClientFlag::retLoadSystemAbility = ERR_OK;
    CliToolMgrClientFlag::shouldCallback = true;
    CliToolMgrClientFlag::cliToolMgr = sptr<MockCliToolMgrService>::MakeSptr()->AsObject();
    EXPECT_TRUE(client.LoadCliToolMgrService());
}

/**
 * @tc.name: QueryInterfaces_0100
 * @tc.desc: Test query/register interfaces return proxy results and populate outputs
 * @tc.type: FUNC
 */
HWTEST_F(CliToolMGRClientTest, QueryInterfaces_0100, TestSize.Level1)
{
    SetMockService();
    CliToolMgrClientFlag::summaries = {BuildToolSummary("ohos-summary")};
    CliToolMgrClientFlag::toolInfos = {BuildToolInfo("ohos-tool")};

    std::vector<ToolSummary> summaries;
    EXPECT_EQ(CliToolMGRClient::GetInstance().GetAllToolSummaries(summaries), ERR_OK);
    ASSERT_EQ(summaries.size(), 1u);
    EXPECT_EQ(summaries[0].name, "ohos-summary");

    ToolInfo tool;
    EXPECT_EQ(CliToolMGRClient::GetInstance().GetToolInfoByName("ohos-tool", tool), ERR_OK);
    EXPECT_EQ(tool.name, "ohos-tool");

    std::vector<ToolInfo> tools;
    EXPECT_EQ(CliToolMGRClient::GetInstance().GetAllToolInfos(tools), ERR_OK);
    ASSERT_EQ(tools.size(), 1u);
    EXPECT_EQ(tools[0].name, "ohos-tool");

    EXPECT_EQ(CliToolMGRClient::GetInstance().RegisterTool(tool), ERR_OK);
}

/**
 * @tc.name: QueryInterfaces_0200
 * @tc.desc: Test proxy error branches for query/register interfaces
 * @tc.type: FUNC
 */
HWTEST_F(CliToolMGRClientTest, QueryInterfaces_0200, TestSize.Level1)
{
    SetMockService();
    CliToolMgrClientFlag::retGetAllToolSummaries = ERR_INVALID_VALUE;
    CliToolMgrClientFlag::retGetToolInfoByName = ERR_INVALID_VALUE;
    CliToolMgrClientFlag::retGetAllToolInfos = ERR_INVALID_VALUE;
    CliToolMgrClientFlag::retRegisterTool = ERR_INVALID_VALUE;

    std::vector<ToolSummary> summaries;
    ToolInfo tool;
    std::vector<ToolInfo> tools;
    EXPECT_EQ(CliToolMGRClient::GetInstance().GetAllToolSummaries(summaries), ERR_INVALID_VALUE);
    EXPECT_EQ(CliToolMGRClient::GetInstance().GetToolInfoByName("ohos-tool", tool), ERR_INVALID_VALUE);
    EXPECT_EQ(CliToolMGRClient::GetInstance().GetAllToolInfos(tools), ERR_INVALID_VALUE);
    EXPECT_EQ(CliToolMGRClient::GetInstance().RegisterTool(tool), ERR_INVALID_VALUE);
}

/**
 * @tc.name: NullProxyInterfaces_0100
 * @tc.desc: Test public interfaces return service-connect failure when proxy cannot be loaded
 * @tc.type: FUNC
 */
HWTEST_F(CliToolMGRClientTest, NullProxyInterfaces_0100, TestSize.Level1)
{
    CliToolMgrClientFlag::nullSystemAbility = true;
    std::vector<ToolSummary> summaries;
    ToolInfo tool;
    std::vector<ToolInfo> tools;
    std::vector<Command> commands;
    std::vector<CommandPermission> permissions;
    CliSessionInfo session;
    std::string subscriptionId;

    EXPECT_EQ(CliToolMGRClient::GetInstance().GetAllToolSummaries(summaries), GET_CLI_TOOL_MGR_SERVICE_FAILED);
    EXPECT_EQ(CliToolMGRClient::GetInstance().GetToolInfoByName("tool", tool), GET_CLI_TOOL_MGR_SERVICE_FAILED);
    EXPECT_EQ(CliToolMGRClient::GetInstance().GetAllToolInfos(tools), GET_CLI_TOOL_MGR_SERVICE_FAILED);
    EXPECT_EQ(CliToolMGRClient::GetInstance().RegisterTool(tool), GET_CLI_TOOL_MGR_SERVICE_FAILED);
    EXPECT_EQ(CliToolMGRClient::GetInstance().ExecTool(ExecToolParam {}, nullptr), GET_CLI_TOOL_MGR_SERVICE_FAILED);
    EXPECT_EQ(CliToolMGRClient::GetInstance().SubscribeSession("session", std::make_shared<MockSessionCallback>(),
        subscriptionId), GET_CLI_TOOL_MGR_SERVICE_FAILED);
    EXPECT_EQ(CliToolMGRClient::GetInstance().UnsubscribeSession("session", "sub"), GET_CLI_TOOL_MGR_SERVICE_FAILED);
    EXPECT_EQ(CliToolMGRClient::GetInstance().ClearSession("session"), GET_CLI_TOOL_MGR_SERVICE_FAILED);
    EXPECT_EQ(CliToolMGRClient::GetInstance().QuerySession("session", session), GET_CLI_TOOL_MGR_SERVICE_FAILED);
    EXPECT_EQ(CliToolMGRClient::GetInstance().SendMessage("session", "input", nullptr),
        GET_CLI_TOOL_MGR_SERVICE_FAILED);
    EXPECT_EQ(CliToolMGRClient::GetInstance().BatchQueryPermissionBySubCommand(commands, permissions),
        GET_CLI_TOOL_MGR_SERVICE_FAILED);
}

/**
 * @tc.name: ExecTool_0100
 * @tc.desc: Test scheduler failure, execute failure cleanup and success callback activation
 * @tc.type: FUNC
 */
HWTEST_F(CliToolMGRClientTest, ExecTool_0100, TestSize.Level1)
{
    SetMockService();
    ExecToolParam param;
    param.toolName = "ohos-tool";
    int32_t callbackCode = -1;

    CliToolMgrClientFlag::retRegisterScheduler = ERR_INVALID_VALUE;
    EXPECT_EQ(CliToolMGRClient::GetInstance().ExecTool(param,
        [&callbackCode](int32_t code, const CliSessionInfo &) { callbackCode = code; }), ERR_INVALID_VALUE);

    CliToolMGRClient::GetInstance().schedulerRegistered_ = false;
    CliToolMgrClientFlag::retRegisterScheduler = ERR_OK;
    CliToolMgrClientFlag::retExecTool = ERR_INVALID_VALUE;
    EXPECT_EQ(CliToolMGRClient::GetInstance().ExecTool(param,
        [&callbackCode](int32_t code, const CliSessionInfo &) { callbackCode = code; }), ERR_INVALID_VALUE);
    EXPECT_EQ(CliEventReplyManager::GetInstance().HandleEventReply(
        CliToolMgrClientFlag::lastEventId, CliEventReplyResult {}), -1);

    CliToolMgrClientFlag::retExecTool = ERR_OK;
    EXPECT_EQ(CliToolMGRClient::GetInstance().ExecTool(param,
        [&callbackCode](int32_t code, const CliSessionInfo &) { callbackCode = code; }), ERR_OK);
    CliSessionInfo session;
    session.sessionId = "session";
    CliEventReplyResult result;
    result.code = ERR_OK;
    result.sessionInfo = session;
    EXPECT_EQ(CliEventReplyManager::GetInstance().HandleEventReply(CliToolMgrClientFlag::lastEventId, result), ERR_OK);
    EXPECT_EQ(callbackCode, ERR_OK);
}

/**
 * @tc.name: SessionInterfaces_0100
 * @tc.desc: Test subscribe/unsubscribe/query/clear interfaces
 * @tc.type: FUNC
 */
HWTEST_F(CliToolMGRClientTest, SessionInterfaces_0100, TestSize.Level1)
{
    SetMockService();
    std::string subscriptionId;
    auto callback = std::make_shared<MockSessionCallback>();
    EXPECT_EQ(CliToolMGRClient::GetInstance().SubscribeSession("session", callback, subscriptionId), ERR_OK);
    EXPECT_FALSE(subscriptionId.empty());

    CliToolEvent event;
    event.type = "stdout";
    EXPECT_EQ(CliSessionSubscriptionManager::GetInstance().HandleSessionEvent("session", subscriptionId, event),
        ERR_OK);
    EXPECT_EQ(callback->eventCount, 1);
    EXPECT_EQ(callback->lastEventType, "stdout");

    EXPECT_EQ(CliToolMGRClient::GetInstance().UnsubscribeSession("session", subscriptionId), ERR_OK);
    EXPECT_EQ(CliToolMGRClient::GetInstance().ClearSession("session"), ERR_OK);

    CliToolMgrClientFlag::querySession.sessionId = "session";
    CliSessionInfo session;
    EXPECT_EQ(CliToolMGRClient::GetInstance().QuerySession("session", session), ERR_OK);
    EXPECT_EQ(session.sessionId, "session");
}

/**
 * @tc.name: SessionInterfaces_0200
 * @tc.desc: Test subscribe failure removes provisional subscription and direct session proxy errors
 * @tc.type: FUNC
 */
HWTEST_F(CliToolMGRClientTest, SessionInterfaces_0200, TestSize.Level1)
{
    SetMockService();
    CliToolMgrClientFlag::retSubscribeSession = ERR_INVALID_VALUE;
    std::string subscriptionId;
    EXPECT_EQ(CliToolMGRClient::GetInstance().SubscribeSession(
        "session", std::make_shared<MockSessionCallback>(), subscriptionId), ERR_INVALID_VALUE);
    EXPECT_TRUE(subscriptionId.empty());

    CliToolMgrClientFlag::retUnsubscribeSession = ERR_INVALID_VALUE;
    CliToolMgrClientFlag::retClearSession = ERR_INVALID_VALUE;
    CliToolMgrClientFlag::retQuerySession = ERR_INVALID_VALUE;
    CliSessionInfo session;
    EXPECT_EQ(CliToolMGRClient::GetInstance().UnsubscribeSession("session", "sub"), ERR_INVALID_VALUE);
    EXPECT_EQ(CliToolMGRClient::GetInstance().ClearSession("session"), ERR_INVALID_VALUE);
    EXPECT_EQ(CliToolMGRClient::GetInstance().QuerySession("session", session), ERR_INVALID_VALUE);
}

/**
 * @tc.name: SendMessage_0100
 * @tc.desc: Test send message failure cleanup and success callback activation
 * @tc.type: FUNC
 */
HWTEST_F(CliToolMGRClientTest, SendMessage_0100, TestSize.Level1)
{
    SetMockService();
    int32_t callbackCode = -1;
    CliToolMgrClientFlag::retSendMessage = ERR_INVALID_VALUE;
    EXPECT_EQ(CliToolMGRClient::GetInstance().SendMessage(
        "session", "input", [&callbackCode](int32_t code) { callbackCode = code; }), ERR_INVALID_VALUE);
    EXPECT_EQ(CliEventReplyManager::GetInstance().HandleEventReply(
        CliToolMgrClientFlag::lastEventId, CliEventReplyResult {}), -1);

    CliToolMgrClientFlag::retSendMessage = ERR_OK;
    EXPECT_EQ(CliToolMGRClient::GetInstance().SendMessage(
        "session", "input", [&callbackCode](int32_t code) { callbackCode = code; }), ERR_OK);
    CliEventReplyResult result;
    result.code = ERR_OK;
    EXPECT_EQ(CliEventReplyManager::GetInstance().HandleEventReply(CliToolMgrClientFlag::lastEventId, result), ERR_OK);
    EXPECT_EQ(callbackCode, ERR_OK);
}

/**
 * @tc.name: BatchQueryPermission_0100
 * @tc.desc: Test batch query permission success and failure forwarding
 * @tc.type: FUNC
 */
HWTEST_F(CliToolMGRClientTest, BatchQueryPermission_0100, TestSize.Level1)
{
    SetMockService();
    CommandPermission permission;
    permission.cmd.toolName = "ohos-tool";
    permission.permissions = {"ohos.permission.TEST"};
    CliToolMgrClientFlag::commandPermissions = {permission};
    std::vector<Command> commands = {Command {"ohos-tool", ""}};
    std::vector<CommandPermission> permissions;
    EXPECT_EQ(CliToolMGRClient::GetInstance().BatchQueryPermissionBySubCommand(commands, permissions), ERR_OK);
    ASSERT_EQ(permissions.size(), 1u);
    EXPECT_EQ(permissions[0].permissions[0], "ohos.permission.TEST");

    CliToolMgrClientFlag::retBatchQueryPermission = ERR_INVALID_VALUE;
    EXPECT_EQ(CliToolMGRClient::GetInstance().BatchQueryPermissionBySubCommand(commands, permissions),
        ERR_INVALID_VALUE);
}

/**
 * @tc.name: ProxyLifecycle_0100
 * @tc.desc: Test callbacks, clear proxy and death recipient branches
 * @tc.type: FUNC
 */
HWTEST_F(CliToolMGRClientTest, ProxyLifecycle_0100, TestSize.Level1)
{
    auto &client = CliToolMGRClient::GetInstance();
    auto mockService = SetMockService();
    client.schedulerRegistered_ = true;
    bool deathHandlerCalled = false;
    client.serviceDeathHandlers_.push_back([&deathHandlerCalled]() { deathHandlerCalled = true; });
    client.ClearProxy();
    EXPECT_EQ(client.cliToolMgr_, nullptr);
    EXPECT_FALSE(client.schedulerRegistered_);
    EXPECT_TRUE(deathHandlerCalled);

    client.OnLoadSystemAbilitySuccess(mockService->AsObject());
    EXPECT_NE(client.cliToolMgr_, nullptr);
    EXPECT_TRUE(client.loadSaFinished_);

    client.OnLoadSystemAbilityFail();
    EXPECT_EQ(client.cliToolMgr_, nullptr);
    EXPECT_TRUE(client.loadSaFinished_);

    bool recipientCalled = false;
    CliToolMGRClient::CliMgrDeathRecipient recipient(
        [&recipientCalled](const wptr<IRemoteObject> &) { recipientCalled = true; });
    recipient.OnRemoteDied(nullptr);
    EXPECT_TRUE(recipientCalled);
}
} // namespace CliTool
} // namespace OHOS
