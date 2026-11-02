/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <gtest/gtest.h>

#include <string>

#include "ability_manager_errors.h"
#include "app_mgr_client.h"
#include "app_mgr_interface.h"
#include "app_service_manager.h"
#include "errors.h"
#include "exec_options.h"
#include "mock_app_mgr_service.h"
#include "task_attribution_manager.h"
#include "task_mgr/attribution_record.h"

using namespace testing::ext;

// Value contract: the tool field crosses IPC as raw int32_t; climgr constant and
// appmgr enum must stay equal (mirrored at run time by AttributionToolContract_0100).
static_assert(OHOS::CliTool::ATTRIBUTION_TOOL_CLI ==
    static_cast<int32_t>(OHOS::AppExecFwk::AttributionTool::CLI),
    "ATTRIBUTION_TOOL_CLI drifted from AppExecFwk::AttributionTool::CLI");

namespace OHOS {
namespace CliTool {
namespace {
constexpr int32_t TEST_CALLER_UID = 3099;
constexpr int32_t TEST_OTHER_UID = 3100;
constexpr pid_t TEST_TOOL_PID = 4321;
constexpr int32_t TEST_RESOLVED_PID = 8765;
constexpr int64_t TEST_ATTRIBUTION_ID = 77;

// In-process appmgr endpoint overriding the three attribution methods; AppMgrClient
// reaches it via the test-side GetAppMgrService below.
class MockAppMgrEndpoint : public AppExecFwk::MockAppMgrService {
public:
    std::string lastRegisterTaskId;
    int32_t lastRegisterPid = 0;
    int32_t lastRegisterTool = -1;
    int32_t lastRegisterCallerUid = -1;
    std::string lastUnregisterTaskId;
    int64_t lastUnregisterAttributionId = 0;
    int32_t lastUnregisterCallerUid = -1;
    std::string lastInAppSkillTaskId;
    std::string lastInAppSkillBundle;
    int32_t lastInAppSkillCallerUid = -1;
    int32_t registerRet = ERR_OK;
    int32_t unregisterRet = ERR_OK;
    int32_t inAppSkillRet = ERR_OK;
    int32_t resolvedPid = TEST_RESOLVED_PID;
    int64_t registerAttributionId = TEST_ATTRIBUTION_ID;
    int registerCalls = 0;
    int unregisterCalls = 0;
    int inAppSkillCalls = 0;

    int32_t RegisterManagedProcess(const std::string &taskId, int32_t pid, int32_t tool,
        int32_t callerUid, int64_t &outAttributionId) override
    {
        ++registerCalls;
        lastRegisterTaskId = taskId;
        lastRegisterPid = pid;
        lastRegisterTool = tool;
        lastRegisterCallerUid = callerUid;
        outAttributionId = registerRet == ERR_OK ? registerAttributionId : 0;
        return registerRet;
    }

    int32_t UnregisterManagedProcess(const std::string &taskId, int64_t attributionId,
        int32_t callerUid) override
    {
        ++unregisterCalls;
        lastUnregisterTaskId = taskId;
        lastUnregisterAttributionId = attributionId;
        lastUnregisterCallerUid = callerUid;
        return unregisterRet;
    }

    int32_t RegisterInAppSkillProcess(const std::string &taskId, const std::string &bundleName,
        int32_t callerUid, int32_t &outPid, int64_t &outAttributionId) override
    {
        ++inAppSkillCalls;
        lastInAppSkillTaskId = taskId;
        lastInAppSkillBundle = bundleName;
        lastInAppSkillCallerUid = callerUid;
        outPid = this->resolvedPid;
        outAttributionId = inAppSkillRet == ERR_OK ? registerAttributionId : 0;
        return inAppSkillRet;
    }
};

sptr<MockAppMgrEndpoint> g_mockEndpoint;
} // namespace
} // namespace CliTool
} // namespace OHOS

namespace OHOS {
namespace AppExecFwk {
// Test-side samgr replacement: AppMgrClient connects to the in-process mock endpoint.
sptr<IRemoteObject> AppServiceManager::GetAppMgrService() const
{
    if (OHOS::CliTool::g_mockEndpoint == nullptr) {
        return nullptr;
    }
    return OHOS::CliTool::g_mockEndpoint->AsObject();
}
} // namespace AppExecFwk
} // namespace OHOS

namespace OHOS {
namespace CliTool {
class TaskAttributionManagerTest : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
    void SetUp() override
    {
        TaskAttributionManager::GetInstance().ResetConnection();
        g_mockEndpoint = new MockAppMgrEndpoint();
    }
    void TearDown() override
    {
        g_mockEndpoint = nullptr;
    }
};

/**
 * @tc.name: GetInstance_0100
 * @tc.desc: The manager is a stateless process-wide singleton.
 * @tc.type: FUNC
 */
HWTEST_F(TaskAttributionManagerTest, GetInstance_0100, TestSize.Level1)
{
    EXPECT_EQ(&TaskAttributionManager::GetInstance(), &TaskAttributionManager::GetInstance());
}

/**
 * @tc.name: AttachProcess_0100
 * @tc.desc: A pass verdict attributes the process and forwards the CLI tool contract.
 * @tc.type: FUNC
 */
HWTEST_F(TaskAttributionManagerTest, AttachProcess_0100, TestSize.Level1)
{
    int64_t attributionId = 0;
    g_mockEndpoint->registerRet = ERR_OK;
    auto ret = TaskAttributionManager::GetInstance().AttachProcess("task_attach_0100", TEST_TOOL_PID,
        TEST_CALLER_UID, attributionId);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(g_mockEndpoint->registerCalls, 1);
    EXPECT_EQ(g_mockEndpoint->lastRegisterTaskId, "task_attach_0100");
    EXPECT_EQ(g_mockEndpoint->lastRegisterPid, static_cast<int32_t>(TEST_TOOL_PID));
    EXPECT_EQ(g_mockEndpoint->lastRegisterTool, ATTRIBUTION_TOOL_CLI);
    EXPECT_EQ(g_mockEndpoint->lastRegisterCallerUid, TEST_CALLER_UID);
}

/**
 * @tc.name: AttachProcess_0200
 * @tc.desc: A spoofed caller uid is rejected by the server verdict: not attributed, no
 *           exception, the return value only tells the caller to log and continue.
 * @tc.type: FUNC
 */
HWTEST_F(TaskAttributionManagerTest, AttachProcess_0200, TestSize.Level1)
{
    int64_t attributionId = 0;
    g_mockEndpoint->registerRet = AAFwk::ERR_TASK_NOT_FOUND;
    auto ret = TaskAttributionManager::GetInstance().AttachProcess("task_attach_0200", TEST_TOOL_PID,
        TEST_OTHER_UID, attributionId);
    EXPECT_EQ(ret, AAFwk::ERR_TASK_NOT_FOUND);
    EXPECT_EQ(g_mockEndpoint->registerCalls, 1);
}

/**
 * @tc.name: AttachProcess_0300
 * @tc.desc: An unknown/finished task id is rejected the same way (not distinguished
 *           from the ownership mismatch, to prevent enumeration).
 * @tc.type: FUNC
 */
HWTEST_F(TaskAttributionManagerTest, AttachProcess_0300, TestSize.Level1)
{
    int64_t attributionId = 0;
    g_mockEndpoint->registerRet = AAFwk::ERR_TASK_NOT_FOUND;
    auto ret = TaskAttributionManager::GetInstance().AttachProcess("task_attach_0300", TEST_TOOL_PID,
        TEST_CALLER_UID, attributionId);
    EXPECT_EQ(ret, AAFwk::ERR_TASK_NOT_FOUND);
    EXPECT_EQ(g_mockEndpoint->registerCalls, 1);
}

/**
 * @tc.name: AttachProcess_0400
 * @tc.desc: Locally invalid parameters are rejected before any IPC: INVALID_PARAMETERS_ERR
 *           and the server is never reached.
 * @tc.type: FUNC
 */
HWTEST_F(TaskAttributionManagerTest, AttachProcess_0400, TestSize.Level1)
{
    int64_t attributionId = 0;
    std::string tooLongTaskId(TRACE_ID_MAX_LEN + 1, 'a');
    auto ret = TaskAttributionManager::GetInstance().AttachProcess(tooLongTaskId, TEST_TOOL_PID,
        TEST_CALLER_UID, attributionId);
    EXPECT_EQ(ret, AAFwk::INVALID_PARAMETERS_ERR);
    ret = TaskAttributionManager::GetInstance().AttachProcess("", TEST_TOOL_PID, TEST_CALLER_UID, attributionId);
    EXPECT_EQ(ret, AAFwk::INVALID_PARAMETERS_ERR);
    ret = TaskAttributionManager::GetInstance().AttachProcess("task_attach_0400", -1, TEST_CALLER_UID, attributionId);
    EXPECT_EQ(ret, AAFwk::INVALID_PARAMETERS_ERR);
    EXPECT_EQ(g_mockEndpoint->registerCalls, 0);
}

/**
 * @tc.name: AttachProcess_0500
 * @tc.desc: A server-side INVALID_PARAMETERS_ERR verdict is passed through untouched.
 * @tc.type: FUNC
 */
HWTEST_F(TaskAttributionManagerTest, AttachProcess_0500, TestSize.Level1)
{
    int64_t attributionId = 0;
    g_mockEndpoint->registerRet = AAFwk::INVALID_PARAMETERS_ERR;
    auto ret = TaskAttributionManager::GetInstance().AttachProcess("task_attach_0500", TEST_TOOL_PID,
        TEST_CALLER_UID, attributionId);
    EXPECT_EQ(ret, AAFwk::INVALID_PARAMETERS_ERR);
    EXPECT_EQ(g_mockEndpoint->registerCalls, 1);
}

/**
 * @tc.name: AttachProcess_0600
 * @tc.desc: An unreachable appmgr means no attribution: INNER_ERR, the execution of the
 *           caller is unaffected (non-blocking).
 * @tc.type: FUNC
 */
HWTEST_F(TaskAttributionManagerTest, AttachProcess_0600, TestSize.Level1)
{
    int64_t attributionId = 0;
    g_mockEndpoint = nullptr;
    auto ret = TaskAttributionManager::GetInstance().AttachProcess("task_attach_0600", TEST_TOOL_PID,
        TEST_CALLER_UID, attributionId);
    EXPECT_EQ(ret, AAFwk::INNER_ERR);
}

/**
 * @tc.name: DetachProcess_0100
 * @tc.desc: A successful detach removes the attributed pid.
 * @tc.type: FUNC
 */
HWTEST_F(TaskAttributionManagerTest, DetachProcess_0100, TestSize.Level1)
{
    g_mockEndpoint->unregisterRet = ERR_OK;
    auto ret = TaskAttributionManager::GetInstance().DetachProcess("task_detach_0100", TEST_ATTRIBUTION_ID,
        TEST_CALLER_UID);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(g_mockEndpoint->unregisterCalls, 1);
    EXPECT_EQ(g_mockEndpoint->lastUnregisterTaskId, "task_detach_0100");
    EXPECT_EQ(g_mockEndpoint->lastUnregisterAttributionId, TEST_ATTRIBUTION_ID);
    EXPECT_EQ(g_mockEndpoint->lastUnregisterCallerUid, TEST_CALLER_UID);
}

/**
 * @tc.name: DetachProcess_0200
 * @tc.desc: No attribution (pid <= 0) is an idempotent no-op: ERR_OK and no IPC.
 * @tc.type: FUNC
 */
HWTEST_F(TaskAttributionManagerTest, DetachProcess_0200, TestSize.Level1)
{
    auto ret = TaskAttributionManager::GetInstance().DetachProcess("task_detach_0200", -1, TEST_CALLER_UID);
    EXPECT_EQ(ret, ERR_OK);
    ret = TaskAttributionManager::GetInstance().DetachProcess("", TEST_ATTRIBUTION_ID, TEST_CALLER_UID);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(g_mockEndpoint->unregisterCalls, 0);
}

/**
 * @tc.name: DetachProcess_0300
 * @tc.desc: A server rejection on detach (unknown task / already detached / not the
 *           owner) is only reported: the teardown path logs and continues.
 * @tc.type: FUNC
 */
HWTEST_F(TaskAttributionManagerTest, DetachProcess_0300, TestSize.Level1)
{
    g_mockEndpoint->unregisterRet = AAFwk::ERR_TASK_NOT_FOUND;
    auto ret = TaskAttributionManager::GetInstance().DetachProcess("task_detach_0300", TEST_ATTRIBUTION_ID,
        TEST_CALLER_UID);
    EXPECT_EQ(ret, AAFwk::ERR_TASK_NOT_FOUND);
    EXPECT_EQ(g_mockEndpoint->unregisterCalls, 1);
}

/**
 * @tc.name: AttachInAppSkillProcess_0100
 * @tc.desc: A pass verdict attributes the located main process and returns the resolved
 *           pid through the out parameter (for the later detach).
 * @tc.type: FUNC
 */
HWTEST_F(TaskAttributionManagerTest, AttachInAppSkillProcess_0100, TestSize.Level1)
{
    g_mockEndpoint->inAppSkillRet = ERR_OK;
    g_mockEndpoint->resolvedPid = TEST_RESOLVED_PID;
    pid_t outPid = -1;
    int64_t attributionId = 0;
    auto ret = TaskAttributionManager::GetInstance().AttachInAppSkillProcess("task_skill_0100",
        "com.example.skill.host", outPid, attributionId, TEST_CALLER_UID);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(outPid, static_cast<pid_t>(TEST_RESOLVED_PID));
    EXPECT_EQ(g_mockEndpoint->inAppSkillCalls, 1);
    EXPECT_EQ(g_mockEndpoint->lastInAppSkillTaskId, "task_skill_0100");
    EXPECT_EQ(g_mockEndpoint->lastInAppSkillBundle, "com.example.skill.host");
    EXPECT_EQ(g_mockEndpoint->lastInAppSkillCallerUid, TEST_CALLER_UID);
}

/**
 * @tc.name: AttachInAppSkillProcess_0200
 * @tc.desc: A locate failure (target application not running / on-demand startup not
 *           finished) means no attribution object: INNER_ERR and outPid = -1, the skill
 *           execution itself is unaffected.
 * @tc.type: FUNC
 */
HWTEST_F(TaskAttributionManagerTest, AttachInAppSkillProcess_0200, TestSize.Level1)
{
    g_mockEndpoint->inAppSkillRet = AAFwk::INNER_ERR;
    g_mockEndpoint->resolvedPid = -1;
    pid_t outPid = 0;
    int64_t attributionId = 0;
    auto ret = TaskAttributionManager::GetInstance().AttachInAppSkillProcess("task_skill_0200",
        "com.example.skill.host", outPid, attributionId, TEST_CALLER_UID);
    EXPECT_EQ(ret, AAFwk::INNER_ERR);
    EXPECT_EQ(outPid, -1);
    EXPECT_EQ(g_mockEndpoint->inAppSkillCalls, 1);
}

/**
 * @tc.name: AttachInAppSkillProcess_0300
 * @tc.desc: A spoofed caller uid is rejected: ERR_TASK_NOT_FOUND and outPid = -1.
 * @tc.type: FUNC
 */
HWTEST_F(TaskAttributionManagerTest, AttachInAppSkillProcess_0300, TestSize.Level1)
{
    g_mockEndpoint->inAppSkillRet = AAFwk::ERR_TASK_NOT_FOUND;
    pid_t outPid = 0;
    int64_t attributionId = 0;
    auto ret = TaskAttributionManager::GetInstance().AttachInAppSkillProcess("task_skill_0300",
        "com.example.skill.host", outPid, attributionId, TEST_OTHER_UID);
    EXPECT_EQ(ret, AAFwk::ERR_TASK_NOT_FOUND);
    EXPECT_EQ(outPid, -1);
    EXPECT_EQ(g_mockEndpoint->inAppSkillCalls, 1);
}

/**
 * @tc.name: AttachInAppSkillProcess_0400
 * @tc.desc: Locally invalid parameters are rejected before any IPC: outPid = -1 and the
 *           server is never reached.
 * @tc.type: FUNC
 */
HWTEST_F(TaskAttributionManagerTest, AttachInAppSkillProcess_0400, TestSize.Level1)
{
    pid_t outPid = 0;
    int64_t attributionId = 0;
    auto ret = TaskAttributionManager::GetInstance().AttachInAppSkillProcess("", "com.example.skill.host",
        outPid, attributionId, TEST_CALLER_UID);
    EXPECT_EQ(ret, AAFwk::INVALID_PARAMETERS_ERR);
    EXPECT_EQ(outPid, -1);
    ret = TaskAttributionManager::GetInstance().AttachInAppSkillProcess("task_skill_0400", "", outPid,
        attributionId, TEST_CALLER_UID);
    EXPECT_EQ(ret, AAFwk::INVALID_PARAMETERS_ERR);
    EXPECT_EQ(outPid, -1);
    EXPECT_EQ(g_mockEndpoint->inAppSkillCalls, 0);
}

/**
 * @tc.name: AttachInAppSkillProcess_0500
 * @tc.desc: An inconsistent server reply (ERR_OK without a usable resolved pid) is
 *           treated as a locate failure: INNER_ERR and outPid = -1.
 * @tc.type: FUNC
 */
HWTEST_F(TaskAttributionManagerTest, AttachInAppSkillProcess_0500, TestSize.Level1)
{
    g_mockEndpoint->inAppSkillRet = ERR_OK;
    g_mockEndpoint->resolvedPid = -1;
    pid_t outPid = 0;
    int64_t attributionId = 0;
    auto ret = TaskAttributionManager::GetInstance().AttachInAppSkillProcess("task_skill_0500",
        "com.example.skill.host", outPid, attributionId, TEST_CALLER_UID);
    EXPECT_EQ(ret, AAFwk::INNER_ERR);
    EXPECT_EQ(outPid, -1);
    EXPECT_EQ(g_mockEndpoint->inAppSkillCalls, 1);
}

/**
 * @tc.name: AttachInAppSkillProcess_0600
 * @tc.desc: An unreachable appmgr means no attribution: INNER_ERR and outPid = -1.
 * @tc.type: FUNC
 */
HWTEST_F(TaskAttributionManagerTest, AttachInAppSkillProcess_0600, TestSize.Level1)
{
    g_mockEndpoint = nullptr;
    pid_t outPid = 0;
    int64_t attributionId = 0;
    auto ret = TaskAttributionManager::GetInstance().AttachInAppSkillProcess("task_skill_0600",
        "com.example.skill.host", outPid, attributionId, TEST_CALLER_UID);
    EXPECT_EQ(ret, AAFwk::INNER_ERR);
    EXPECT_EQ(outPid, -1);
}

/**
 * @tc.name: DetachProcess_0400
 * @tc.desc: VM-5 (unit-level slice): repeated detach stays well-behaved — every
 *           call is forwarded, none throws or blocks, and the caller-side
 *           semantics never degrade (server-side idempotence absorbs the
 *           duplicate unregister; the teardown path only logs).
 * @tc.type: FUNC
 */
HWTEST_F(TaskAttributionManagerTest, DetachProcess_0400, TestSize.Level1)
{
    g_mockEndpoint->unregisterRet = ERR_OK;
    auto first = TaskAttributionManager::GetInstance().DetachProcess("task_detach_0400",
        TEST_ATTRIBUTION_ID, TEST_CALLER_UID);
    auto second = TaskAttributionManager::GetInstance().DetachProcess("task_detach_0400",
        TEST_ATTRIBUTION_ID, TEST_CALLER_UID);
    auto third = TaskAttributionManager::GetInstance().DetachProcess("task_detach_0400",
        TEST_ATTRIBUTION_ID, TEST_CALLER_UID);
    EXPECT_EQ(first, ERR_OK);
    EXPECT_EQ(second, ERR_OK);
    EXPECT_EQ(third, ERR_OK);
    EXPECT_EQ(g_mockEndpoint->unregisterCalls, 3);
    EXPECT_EQ(g_mockEndpoint->lastUnregisterTaskId, "task_detach_0400");
    EXPECT_EQ(g_mockEndpoint->lastUnregisterAttributionId, TEST_ATTRIBUTION_ID);
    EXPECT_EQ(g_mockEndpoint->lastUnregisterCallerUid, TEST_CALLER_UID);

    g_mockEndpoint->unregisterRet = AAFwk::ERR_TASK_NOT_FOUND;
    auto late = TaskAttributionManager::GetInstance().DetachProcess("task_detach_0400",
        TEST_ATTRIBUTION_ID, TEST_CALLER_UID);
    EXPECT_EQ(late, AAFwk::ERR_TASK_NOT_FOUND);
}

/**
 * @tc.name: AttributionToolContract_0100
 * @tc.desc: IPC value contract (review finding): the tool field crosses the
 *           attribution IPC boundary as a raw int32_t, so the climgr-side
 *           constant must equal the appmgr-side enum value — a future enum
 *           insertion would otherwise misroute silently. Mirrored at compile
 *           time by the static_assert at the top of this file.
 * @tc.type: FUNC
 */
HWTEST_F(TaskAttributionManagerTest, AttributionToolContract_0100, TestSize.Level1)
{
    EXPECT_EQ(ATTRIBUTION_TOOL_CLI, static_cast<int32_t>(AppExecFwk::AttributionTool::CLI));
}

/**
 * @tc.name: AttachProcess_ConnectionCached_0100
 * @tc.desc: The remote is cached after the first connect: a later call with no
 *           resolvable endpoint still reaches the cached one.
 * @tc.type: FUNC
 */
HWTEST_F(TaskAttributionManagerTest, AttachProcess_ConnectionCached_0100, TestSize.Level1)
{
    int64_t attributionId = 0;
    g_mockEndpoint->registerRet = ERR_OK;
    EXPECT_EQ(TaskAttributionManager::GetInstance().AttachProcess("task_cache_0100", TEST_TOOL_PID,
        TEST_CALLER_UID, attributionId), ERR_OK);

    auto cachedEndpoint = g_mockEndpoint;
    g_mockEndpoint = nullptr;
    EXPECT_EQ(TaskAttributionManager::GetInstance().AttachProcess("task_cache_0100_b", TEST_TOOL_PID,
        TEST_CALLER_UID, attributionId), ERR_OK);
    EXPECT_EQ(cachedEndpoint->registerCalls, 2);
    EXPECT_EQ(cachedEndpoint->lastRegisterTaskId, "task_cache_0100_b");
}

/**
 * @tc.name: AttachProcess_ReconnectAfterReset_0100
 * @tc.desc: ResetConnection drops the cached remote: the next call resolves the
 *           new endpoint and the old one is untouched.
 * @tc.type: FUNC
 */
HWTEST_F(TaskAttributionManagerTest, AttachProcess_ReconnectAfterReset_0100, TestSize.Level1)
{
    int64_t attributionId = 0;
    g_mockEndpoint->registerRet = ERR_OK;
    auto first = g_mockEndpoint;
    EXPECT_EQ(TaskAttributionManager::GetInstance().AttachProcess("task_reconnect_0100", TEST_TOOL_PID,
        TEST_CALLER_UID, attributionId), ERR_OK);
    EXPECT_EQ(first->registerCalls, 1);

    TaskAttributionManager::GetInstance().ResetConnection();
    g_mockEndpoint = new MockAppMgrEndpoint();
    EXPECT_EQ(TaskAttributionManager::GetInstance().AttachProcess("task_reconnect_0200", TEST_TOOL_PID,
        TEST_CALLER_UID, attributionId), ERR_OK);
    EXPECT_EQ(g_mockEndpoint->registerCalls, 1);
    EXPECT_EQ(g_mockEndpoint->lastRegisterTaskId, "task_reconnect_0200");
    EXPECT_EQ(first->registerCalls, 1);
}

/**
 * @tc.name: DetachProcess_ConnectionCached_0100
 * @tc.desc: The detach path (ffrt worker side) shares the same cached connection.
 * @tc.type: FUNC
 */
HWTEST_F(TaskAttributionManagerTest, DetachProcess_ConnectionCached_0100, TestSize.Level1)
{
    int64_t attributionId = 0;
    g_mockEndpoint->registerRet = ERR_OK;
    EXPECT_EQ(TaskAttributionManager::GetInstance().AttachProcess("task_detach_cache_0100", TEST_TOOL_PID,
        TEST_CALLER_UID, attributionId), ERR_OK);

    auto cachedEndpoint = g_mockEndpoint;
    g_mockEndpoint = nullptr;
    EXPECT_EQ(TaskAttributionManager::GetInstance().DetachProcess("task_detach_cache_0100",
        attributionId, TEST_CALLER_UID), ERR_OK);
    EXPECT_EQ(cachedEndpoint->unregisterCalls, 1);
    EXPECT_EQ(cachedEndpoint->lastUnregisterTaskId, "task_detach_cache_0100");
}
} // namespace CliTool
} // namespace OHOS
