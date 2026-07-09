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

#include <chrono>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#define private public
#include "agent_connect_manager.h"
#undef private

#include "ability_connect_callback_stub.h"
#include "ability_manager_errors.h"
#include "agent_connect_manager_types.h"
#include "agent_extension_connection_constants.h"
#include "errors.h"
#include "want.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AgentRuntime;

namespace {
constexpr int32_t CALLER_UID = 100;
constexpr int32_t OTHER_CALLER_UID = 200;
constexpr int32_t HOST_UID = 300;
constexpr int32_t CALLER_USER_ID = 0;
constexpr int64_t NONCE_A = 42;
constexpr int64_t NONCE_B = 43;
constexpr size_t MAX_AGENT_CONNECTIONS_PER_CALLER = 5;
constexpr size_t MAX_LOW_CODE_AGENTS_PER_HOST = 100;
constexpr size_t MAX_AGENT_CONNECT_PREFLIGHTS = 1024;

class MockAbilityConnection : public AAFwk::AbilityConnectionStub {
public:
    MockAbilityConnection() = default;
    ~MockAbilityConnection() override = default;

    void OnAbilityConnectDone(
        const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject, int resultCode) override
    {}

    void OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode) override
    {}
};

sptr<MockAbilityConnection> MakeConnection()
{
    return sptr<MockAbilityConnection>::MakeSptr();
}

AgentHostKey MakeHostKey(int32_t userId, const std::string &bundle, const std::string &module,
    const std::string &ability, int32_t appIndex = 0)
{
    AgentHostKey key;
    key.userId = userId;
    key.appIndex = appIndex;
    key.bundleName = bundle;
    key.moduleName = module;
    key.abilityName = ability;
    return key;
}

AgentHostKey DefaultHostKey()
{
    return MakeHostKey(CALLER_USER_ID, "com.host", "host.module", "HostAbility");
}

AgentQuotaKey MakeQuotaKey(const AgentHostKey &host, const std::string &agentId, bool isLowCode)
{
    AgentQuotaKey key;
    key.hostKey = host;
    key.agentId = agentId;
    key.isLowCode = isLowCode;
    return key;
}

AgentGenerateNonceFunc FixedNonceFunc(int64_t value)
{
    return [value]() { return value; };
}

AgentGenerateNonceFunc CounterNonceFunc(int64_t start)
{
    auto counter = std::make_shared<int64_t>(start);
    return [counter]() { return (*counter)++; };
}

AgentGenerateNonceFunc SequenceNonceFunc(std::initializer_list<int64_t> values)
{
    auto seq = std::make_shared<std::vector<int64_t>>(values);
    auto idx = std::make_shared<size_t>(0);
    return [seq, idx]() -> int64_t {
        if (*idx >= seq->size()) {
            return -1;
        }
        return seq->at((*idx)++);
    };
}

AAFwk::Want MakePreflightWant(const std::string &agentId, const std::string &bundle, const std::string &ability,
    const std::string &module = "host.module")
{
    AAFwk::Want want;
    want.SetParam(AGENTID_KEY, agentId);
    want.SetElementName("", bundle, ability, module);
    return want;
}

AAFwk::Want MakeVerifyWant(int64_t nonce)
{
    AAFwk::Want want;
    SetAgentVerificationNonceParam(want, nonce);
    return want;
}

AgentStandardConnectRequest MakeStandardRequest(const sptr<AAFwk::IAbilityConnection> &connection, int32_t callerUid,
    const std::string &agentId, const AgentHostKey &host, int64_t nonce,
    const std::string &identity = "caller-identity")
{
    AgentStandardConnectRequest request;
    request.connection = connection;
    request.callerUid = callerUid;
    request.agentId = agentId;
    request.quotaKey = MakeQuotaKey(host, agentId, false);
    request.verificationNonce = nonce;
    request.originalIdentity = identity;
    return request;
}

AgentConnectPlanRequest MakePlanRequest(const sptr<AAFwk::IAbilityConnection> &connection, int32_t callerUid,
    const AgentHostKey &host, int32_t hostUid, const std::string &agentId)
{
    AgentConnectPlanRequest request;
    request.connection = connection;
    request.callerUid = callerUid;
    request.hostKey = host;
    request.hostUid = hostUid;
    request.agentId = agentId;
    return request;
}

AgentCallerDeathHandler NoopDeathHandler()
{
    return [](const sptr<IRemoteObject> &) {};
}

// Drives a low-code agent through plan -> identity -> host-connect-done so it becomes connected.
void SetupConnectedLowCodeAgent(const sptr<AAFwk::IAbilityConnection> &connection, const AgentHostKey &host,
    const std::string &agentId, int64_t nonce, const sptr<IRemoteObject> &hostRemote, AgentConnectPlan &plan)
{
    auto ret = AgentConnectManager::GetInstance().PrepareLowCodeConnectPlan(
        MakePlanRequest(connection, CALLER_UID, host, HOST_UID, agentId), plan);
    ASSERT_EQ(ret, ERR_OK);
    ASSERT_EQ(AgentConnectManager::GetInstance().SetLowCodeConnectIdentity(host, agentId, "caller-identity", nonce),
        ERR_OK);
    AgentHostConnectDoneRequest doneRequest;
    doneRequest.hostKey = host;
    doneRequest.callerRemote = connection->AsObject();
    doneRequest.agentId = agentId;
    doneRequest.remoteObject = hostRemote;
    doneRequest.resultCode = ERR_OK;
    auto doneResult = AgentConnectManager::GetInstance().HandleAgentHostConnectDone(doneRequest);
    ASSERT_EQ(doneResult.callback, connection);
}
}  // namespace

namespace OHOS {
namespace AgentRuntime {
class AgentConnectManagerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp() override;
    void TearDown() override;
};

void AgentConnectManagerTest::SetUpTestCase(void)
{}

void AgentConnectManagerTest::TearDownTestCase(void)
{}

void AgentConnectManagerTest::SetUp(void)
{
    AgentConnectManager::GetInstance().Clear();
}

void AgentConnectManagerTest::TearDown(void)
{
    AgentConnectManager::GetInstance().Clear();
}

// ---------------------------------------------------------------------------
// Singleton & Clear
// ---------------------------------------------------------------------------

/**
 * @tc.name      GetInstanceReturnsStableSingleton
 * @tc.desc      GetInstance must always return the same instance.
 */
HWTEST_F(AgentConnectManagerTest, GetInstanceReturnsStableSingleton, TestSize.Level1)
{
    auto &first = AgentConnectManager::GetInstance();
    auto &second = AgentConnectManager::GetInstance();
    EXPECT_EQ(&first, &second);
}

/**
 * @tc.name      ClearEmptiesAllLedgers
 * @tc.desc      Clear must remove tracked connections, preflights, quotas, host sessions and owners.
 */
HWTEST_F(AgentConnectManagerTest, ClearEmptiesAllLedgers, TestSize.Level1)
{
    auto &mgr = AgentConnectManager::GetInstance();
    auto conn = MakeConnection();
    AgentConnectPlanRequest planRequest = MakePlanRequest(conn, CALLER_UID, DefaultHostKey(), HOST_UID, "agent-1");
    AgentConnectPlan plan;
    ASSERT_EQ(mgr.PrepareLowCodeConnectPlan(planRequest, plan), ERR_OK);
    EXPECT_FALSE(mgr.trackedConnections_.empty());
    EXPECT_FALSE(mgr.agentHostSessions_.empty());
    EXPECT_FALSE(mgr.agentOwners_.empty());
    EXPECT_FALSE(mgr.callerQuotas_.empty());

    mgr.Clear();
    EXPECT_TRUE(mgr.trackedConnections_.empty());
    EXPECT_TRUE(mgr.connectPreflights_.empty());
    EXPECT_FALSE(mgr.connectPreflightCleanupScheduled_);
    EXPECT_TRUE(mgr.callerQuotas_.empty());
    EXPECT_TRUE(mgr.agentHostSessions_.empty());
    EXPECT_TRUE(mgr.agentOwners_.empty());
}

// ---------------------------------------------------------------------------
// Connect preflight register / consume / cleanup
// ---------------------------------------------------------------------------

/**
 * @tc.name      RegisterPreflightReturnsZeroNonceWhenGeneratorNull
 * @tc.desc      A null nonce generator yields nonce 0 and no schedule.
 */
HWTEST_F(AgentConnectManagerTest, RegisterPreflightReturnsZeroNonceWhenGeneratorNull, TestSize.Level1)
{
    AgentConnectPreflightRegisterRequest request;
    request.connectWant = MakePreflightWant("agent-1", "com.host", "HostAbility");
    request.agentId = "agent-1";
    request.callerUid = CALLER_UID;
    request.callerUserId = CALLER_USER_ID;
    auto result = AgentConnectManager::GetInstance().RegisterConnectPreflight(request, nullptr);
    EXPECT_EQ(result.nonce, 0);
    EXPECT_FALSE(result.needSchedule);
    EXPECT_TRUE(AgentConnectManager::GetInstance().connectPreflights_.empty());
}

/**
 * @tc.name      RegisterPreflightReturnsZeroNonceWhenGeneratorYieldsNonPositive
 * @tc.desc      A generator returning 0 (or negative) yields nonce 0 and stores nothing.
 */
HWTEST_F(AgentConnectManagerTest, RegisterPreflightReturnsZeroNonceWhenGeneratorYieldsNonPositive, TestSize.Level1)
{
    AgentConnectPreflightRegisterRequest request;
    request.connectWant = MakePreflightWant("agent-1", "com.host", "HostAbility");
    request.agentId = "agent-1";
    request.callerUid = CALLER_UID;
    request.callerUserId = CALLER_USER_ID;
    auto result = AgentConnectManager::GetInstance().RegisterConnectPreflight(request, FixedNonceFunc(0));
    EXPECT_EQ(result.nonce, 0);
    EXPECT_TRUE(AgentConnectManager::GetInstance().connectPreflights_.empty());
}

/**
 * @tc.name      RegisterPreflightStampsNonceAndSchedulesCleanup
 * @tc.desc      A valid nonce is stamped into the want, the record is stored and cleanup is scheduled.
 */
HWTEST_F(AgentConnectManagerTest, RegisterPreflightStampsNonceAndSchedulesCleanup, TestSize.Level1)
{
    auto &mgr = AgentConnectManager::GetInstance();
    AgentConnectPreflightRegisterRequest request;
    request.connectWant = MakePreflightWant("agent-1", "com.host", "HostAbility");
    request.agentId = "agent-1";
    request.card.agentId = "agent-1";
    request.callerUid = CALLER_UID;
    request.callerUserId = CALLER_USER_ID;
    auto result = mgr.RegisterConnectPreflight(request, FixedNonceFunc(NONCE_A));
    EXPECT_EQ(result.nonce, NONCE_A);
    EXPECT_EQ(GetAgentVerificationNonceParam(result.connectWant), NONCE_A);
    EXPECT_TRUE(result.needSchedule);
    EXPECT_EQ(mgr.connectPreflightCleanupAt_, result.cleanupAt);
    EXPECT_TRUE(mgr.connectPreflightCleanupScheduled_);
    ASSERT_EQ(mgr.connectPreflights_.size(), 1u);
    auto record = mgr.connectPreflights_.at(NONCE_A);
    EXPECT_EQ(record.callerUid, CALLER_UID);
    EXPECT_EQ(record.agentId, "agent-1");
    EXPECT_EQ(record.card.agentId, "agent-1");
}

/**
 * @tc.name      RegisterPreflightRetriesOnDuplicateNonce
 * @tc.desc      When the first nonce collides, registration retries until a fresh nonce is produced.
 */
HWTEST_F(AgentConnectManagerTest, RegisterPreflightRetriesOnDuplicateNonce, TestSize.Level1)
{
    auto &mgr = AgentConnectManager::GetInstance();
    AgentConnectPreflightRegisterRequest first;
    first.connectWant = MakePreflightWant("agent-1", "com.host", "HostAbility");
    first.agentId = "agent-1";
    first.callerUid = CALLER_UID;
    first.callerUserId = CALLER_USER_ID;
    auto firstResult = mgr.RegisterConnectPreflight(first, FixedNonceFunc(5));
    ASSERT_EQ(firstResult.nonce, 5);

    AgentConnectPreflightRegisterRequest second;
    second.connectWant = MakePreflightWant("agent-2", "com.host", "HostAbility");
    second.agentId = "agent-2";
    second.callerUid = CALLER_UID;
    second.callerUserId = CALLER_USER_ID;
    auto secondResult = mgr.RegisterConnectPreflight(second, SequenceNonceFunc({5, 7}));
    EXPECT_EQ(secondResult.nonce, 7);
    EXPECT_EQ(mgr.connectPreflights_.size(), 2u);
    EXPECT_NE(mgr.connectPreflights_.find(5), mgr.connectPreflights_.end());
    EXPECT_NE(mgr.connectPreflights_.find(7), mgr.connectPreflights_.end());
}

/**
 * @tc.name      RegisterPreflightEvictsOldestAtCapacity
 * @tc.desc      Once MAX_AGENT_CONNECT_PREFLIGHTS records exist, the next register evicts the oldest.
 */
HWTEST_F(AgentConnectManagerTest, RegisterPreflightEvictsOldestAtCapacity, TestSize.Level1)
{
    auto &mgr = AgentConnectManager::GetInstance();
    auto gen = CounterNonceFunc(1);
    for (size_t i = 0; i < MAX_AGENT_CONNECT_PREFLIGHTS; ++i) {
        AgentConnectPreflightRegisterRequest request;
        request.connectWant = MakePreflightWant("agent", "com.host", "HostAbility");
        request.agentId = "agent";
        request.callerUid = CALLER_UID;
        request.callerUserId = CALLER_USER_ID;
        auto result = mgr.RegisterConnectPreflight(request, gen);
        ASSERT_GT(result.nonce, 0);
    }
    ASSERT_EQ(mgr.connectPreflights_.size(), MAX_AGENT_CONNECT_PREFLIGHTS);

    AgentConnectPreflightRegisterRequest overflow;
    overflow.connectWant = MakePreflightWant("agent", "com.host", "HostAbility");
    overflow.agentId = "agent";
    overflow.callerUid = CALLER_UID;
    overflow.callerUserId = CALLER_USER_ID;
    auto result = mgr.RegisterConnectPreflight(overflow, FixedNonceFunc(MAX_AGENT_CONNECT_PREFLIGHTS + 1));
    EXPECT_EQ(result.nonce, static_cast<int64_t>(MAX_AGENT_CONNECT_PREFLIGHTS + 1));
    EXPECT_EQ(mgr.connectPreflights_.size(), MAX_AGENT_CONNECT_PREFLIGHTS);
    EXPECT_EQ(mgr.connectPreflights_.find(1), mgr.connectPreflights_.end());
    EXPECT_NE(mgr.connectPreflights_.find(static_cast<int64_t>(MAX_AGENT_CONNECT_PREFLIGHTS + 1)),
        mgr.connectPreflights_.end());
}

/**
 * @tc.name      ConsumePreflightFailsWithoutNonceInWant
 * @tc.desc      A consume want lacking the verification nonce never matches.
 */
HWTEST_F(AgentConnectManagerTest, ConsumePreflightFailsWithoutNonceInWant, TestSize.Level1)
{
    auto &mgr = AgentConnectManager::GetInstance();
    AgentConnectPreflightRegisterRequest request;
    request.connectWant = MakePreflightWant("agent-1", "com.host", "HostAbility");
    request.agentId = "agent-1";
    request.callerUid = CALLER_UID;
    request.callerUserId = CALLER_USER_ID;
    auto reg = mgr.RegisterConnectPreflight(request, FixedNonceFunc(NONCE_A));
    ASSERT_GT(reg.nonce, 0);

    AgentConnectPreflightConsumeRequest consume;
    consume.want = MakePreflightWant("agent-1", "com.host", "HostAbility");
    consume.callerUid = CALLER_UID;
    consume.callerUserId = CALLER_USER_ID;
    auto result = mgr.TryConsumeConnectPreflight(consume);
    EXPECT_FALSE(result.matched);
    EXPECT_EQ(mgr.connectPreflights_.size(), 1u);
}

/**
 * @tc.name      ConsumePreflightFailsOnUnknownNonce
 * @tc.desc      A nonce that was never registered does not match and is not stored.
 */
HWTEST_F(AgentConnectManagerTest, ConsumePreflightFailsOnUnknownNonce, TestSize.Level1)
{
    auto &mgr = AgentConnectManager::GetInstance();
    AgentConnectPreflightConsumeRequest consume;
    consume.want = MakeVerifyWant(9999);
    consume.callerUid = CALLER_UID;
    consume.callerUserId = CALLER_USER_ID;
    auto result = mgr.TryConsumeConnectPreflight(consume);
    EXPECT_FALSE(result.matched);
}

/**
 * @tc.name      ConsumePreflightFailsOnCallerUidMismatch
 * @tc.desc      A caller-uid mismatch consumes (erases) the record without matching.
 */
HWTEST_F(AgentConnectManagerTest, ConsumePreflightFailsOnCallerUidMismatch, TestSize.Level1)
{
    auto &mgr = AgentConnectManager::GetInstance();
    AgentConnectPreflightRegisterRequest request;
    request.connectWant = MakePreflightWant("agent-1", "com.host", "HostAbility");
    request.agentId = "agent-1";
    request.callerUid = CALLER_UID;
    request.callerUserId = CALLER_USER_ID;
    auto reg = mgr.RegisterConnectPreflight(request, FixedNonceFunc(NONCE_A));
    ASSERT_GT(reg.nonce, 0);

    AgentConnectPreflightConsumeRequest consume;
    consume.want = reg.connectWant;
    consume.callerUid = OTHER_CALLER_UID;
    consume.callerUserId = CALLER_USER_ID;
    auto result = mgr.TryConsumeConnectPreflight(consume);
    EXPECT_FALSE(result.matched);
    EXPECT_TRUE(mgr.connectPreflights_.empty());
}

/**
 * @tc.name      ConsumePreflightFailsOnCallerUserIdMismatch
 * @tc.desc      A caller-user-id mismatch consumes (erases) the record without matching.
 */
HWTEST_F(AgentConnectManagerTest, ConsumePreflightFailsOnCallerUserIdMismatch, TestSize.Level1)
{
    auto &mgr = AgentConnectManager::GetInstance();
    AgentConnectPreflightRegisterRequest request;
    request.connectWant = MakePreflightWant("agent-1", "com.host", "HostAbility");
    request.agentId = "agent-1";
    request.callerUid = CALLER_UID;
    request.callerUserId = CALLER_USER_ID;
    auto reg = mgr.RegisterConnectPreflight(request, FixedNonceFunc(NONCE_A));
    ASSERT_GT(reg.nonce, 0);

    AgentConnectPreflightConsumeRequest consume;
    consume.want = reg.connectWant;
    consume.callerUid = CALLER_UID;
    consume.callerUserId = CALLER_USER_ID + 1;
    auto result = mgr.TryConsumeConnectPreflight(consume);
    EXPECT_FALSE(result.matched);
    EXPECT_TRUE(mgr.connectPreflights_.empty());
}

/**
 * @tc.name      ConsumePreflightFailsOnAgentIdMismatch
 * @tc.desc      A target agentId mismatch consumes (erases) the record without matching.
 */
HWTEST_F(AgentConnectManagerTest, ConsumePreflightFailsOnAgentIdMismatch, TestSize.Level1)
{
    auto &mgr = AgentConnectManager::GetInstance();
    AgentConnectPreflightRegisterRequest request;
    request.connectWant = MakePreflightWant("agent-1", "com.host", "HostAbility");
    request.agentId = "agent-1";
    request.callerUid = CALLER_UID;
    request.callerUserId = CALLER_USER_ID;
    auto reg = mgr.RegisterConnectPreflight(request, FixedNonceFunc(NONCE_A));
    ASSERT_GT(reg.nonce, 0);

    AAFwk::Want want = reg.connectWant;
    want.SetParam(AGENTID_KEY, std::string("agent-2"));
    AgentConnectPreflightConsumeRequest consume;
    consume.want = want;
    consume.callerUid = CALLER_UID;
    consume.callerUserId = CALLER_USER_ID;
    auto result = mgr.TryConsumeConnectPreflight(consume);
    EXPECT_FALSE(result.matched);
    EXPECT_TRUE(mgr.connectPreflights_.empty());
}

/**
 * @tc.name      ConsumePreflightFailsOnElementMismatch
 * @tc.desc      A target element mismatch consumes (erases) the record without matching.
 */
HWTEST_F(AgentConnectManagerTest, ConsumePreflightFailsOnElementMismatch, TestSize.Level1)
{
    auto &mgr = AgentConnectManager::GetInstance();
    AgentConnectPreflightRegisterRequest request;
    request.connectWant = MakePreflightWant("agent-1", "com.host", "HostAbility");
    request.agentId = "agent-1";
    request.callerUid = CALLER_UID;
    request.callerUserId = CALLER_USER_ID;
    auto reg = mgr.RegisterConnectPreflight(request, FixedNonceFunc(NONCE_A));
    ASSERT_GT(reg.nonce, 0);

    AAFwk::Want want = reg.connectWant;
    want.SetElementName("", "com.other", "OtherAbility", "other.module");
    AgentConnectPreflightConsumeRequest consume;
    consume.want = want;
    consume.callerUid = CALLER_UID;
    consume.callerUserId = CALLER_USER_ID;
    auto result = mgr.TryConsumeConnectPreflight(consume);
    EXPECT_FALSE(result.matched);
    EXPECT_TRUE(mgr.connectPreflights_.empty());
}

/**
 * @tc.name      ConsumePreflightSucceedsAndErasesRecord
 * @tc.desc      A fully-matching consume returns the stored want/agentId/card and erases the record.
 */
HWTEST_F(AgentConnectManagerTest, ConsumePreflightSucceedsAndErasesRecord, TestSize.Level1)
{
    auto &mgr = AgentConnectManager::GetInstance();
    AgentConnectPreflightRegisterRequest request;
    request.connectWant = MakePreflightWant("agent-1", "com.host", "HostAbility");
    request.agentId = "agent-1";
    request.card.agentId = "agent-1";
    request.callerUid = CALLER_UID;
    request.callerUserId = CALLER_USER_ID;
    auto reg = mgr.RegisterConnectPreflight(request, FixedNonceFunc(NONCE_A));
    ASSERT_GT(reg.nonce, 0);

    AgentConnectPreflightConsumeRequest consume;
    consume.want = reg.connectWant;
    consume.callerUid = CALLER_UID;
    consume.callerUserId = CALLER_USER_ID;
    auto result = mgr.TryConsumeConnectPreflight(consume);
    EXPECT_TRUE(result.matched);
    EXPECT_EQ(result.agentId, "agent-1");
    EXPECT_EQ(result.card.agentId, "agent-1");
    EXPECT_TRUE(mgr.connectPreflights_.empty());

    auto second = mgr.TryConsumeConnectPreflight(consume);
    EXPECT_FALSE(second.matched);
}

/**
 * @tc.name      CleanupExpiredPreflightsIgnoresWrongScheduledAt
 * @tc.desc      A scheduledAt that does not match the recorded cleanup time is a no-op.
 */
HWTEST_F(AgentConnectManagerTest, CleanupExpiredPreflightsIgnoresWrongScheduledAt, TestSize.Level1)
{
    auto &mgr = AgentConnectManager::GetInstance();
    AgentConnectPreflightRegisterRequest request;
    request.connectWant = MakePreflightWant("agent-1", "com.host", "HostAbility");
    request.agentId = "agent-1";
    request.callerUid = CALLER_UID;
    request.callerUserId = CALLER_USER_ID;
    auto reg = mgr.RegisterConnectPreflight(request, FixedNonceFunc(NONCE_A));
    ASSERT_GT(reg.nonce, 0);
    ASSERT_TRUE(mgr.connectPreflightCleanupScheduled_);

    AgentPreflightTimePoint nextAt;
    auto wrong = reg.cleanupAt + std::chrono::minutes(1);
    auto ret = mgr.CleanupExpiredConnectPreflights(wrong, nextAt);
    EXPECT_FALSE(ret);
    EXPECT_TRUE(mgr.connectPreflightCleanupScheduled_);
    EXPECT_EQ(mgr.connectPreflights_.size(), 1u);
}

/**
 * @tc.name      CleanupExpiredPreflightsReschedulesWhenNonEmpty
 * @tc.desc      With the matching scheduledAt and a live record, cleanup re-arms the next deadline.
 */
HWTEST_F(AgentConnectManagerTest, CleanupExpiredPreflightsReschedulesWhenNonEmpty, TestSize.Level1)
{
    auto &mgr = AgentConnectManager::GetInstance();
    AgentConnectPreflightRegisterRequest request;
    request.connectWant = MakePreflightWant("agent-1", "com.host", "HostAbility");
    request.agentId = "agent-1";
    request.callerUid = CALLER_UID;
    request.callerUserId = CALLER_USER_ID;
    auto reg = mgr.RegisterConnectPreflight(request, FixedNonceFunc(NONCE_A));
    ASSERT_GT(reg.nonce, 0);

    AgentPreflightTimePoint nextAt;
    auto ret = mgr.CleanupExpiredConnectPreflights(reg.cleanupAt, nextAt);
    EXPECT_TRUE(ret);
    EXPECT_EQ(nextAt, reg.cleanupAt);
    EXPECT_TRUE(mgr.connectPreflightCleanupScheduled_);
    EXPECT_EQ(mgr.connectPreflights_.size(), 1u);
}

/**
 * @tc.name      CleanupExpiredPreflightsReturnsFalseWhenEmptyAfterConsume
 * @tc.desc      When the last preflight has been consumed, cleanup reports nothing left to schedule.
 */
HWTEST_F(AgentConnectManagerTest, CleanupExpiredPreflightsReturnsFalseWhenEmptyAfterConsume, TestSize.Level1)
{
    auto &mgr = AgentConnectManager::GetInstance();
    AgentConnectPreflightRegisterRequest request;
    request.connectWant = MakePreflightWant("agent-1", "com.host", "HostAbility");
    request.agentId = "agent-1";
    request.callerUid = CALLER_UID;
    request.callerUserId = CALLER_USER_ID;
    auto reg = mgr.RegisterConnectPreflight(request, FixedNonceFunc(NONCE_A));
    ASSERT_GT(reg.nonce, 0);
    AgentConnectPreflightConsumeRequest consume;
    consume.want = reg.connectWant;
    consume.callerUid = CALLER_UID;
    consume.callerUserId = CALLER_USER_ID;
    ASSERT_TRUE(mgr.TryConsumeConnectPreflight(consume).matched);
    ASSERT_TRUE(mgr.connectPreflights_.empty());
    ASSERT_TRUE(mgr.connectPreflightCleanupScheduled_);

    AgentPreflightTimePoint nextAt;
    auto ret = mgr.CleanupExpiredConnectPreflights(reg.cleanupAt, nextAt);
    EXPECT_FALSE(ret);
    EXPECT_FALSE(mgr.connectPreflightCleanupScheduled_);
}

// ---------------------------------------------------------------------------
// Standard agent connection register / quota
// ---------------------------------------------------------------------------

/**
 * @tc.name      RegisterStandardRejectsNullConnection
 * @tc.desc      A null connection or non-positive nonce is rejected before any state change.
 */
HWTEST_F(AgentConnectManagerTest, RegisterStandardRejectsNullConnection, TestSize.Level1)
{
    auto request = MakeStandardRequest(nullptr, CALLER_UID, "agent-1", DefaultHostKey(), NONCE_A);
    EXPECT_EQ(AgentConnectManager::GetInstance().RegisterStandardAgentConnection(request),
        ERR_INVALID_VALUE);
    auto goodConn = MakeConnection();
    auto badNonce = MakeStandardRequest(goodConn, CALLER_UID, "agent-1", DefaultHostKey(), 0);
    EXPECT_EQ(AgentConnectManager::GetInstance().RegisterStandardAgentConnection(badNonce),
        ERR_INVALID_VALUE);
    EXPECT_TRUE(AgentConnectManager::GetInstance().trackedConnections_.empty());
    EXPECT_TRUE(AgentConnectManager::GetInstance().callerQuotas_.empty());
}

/**
 * @tc.name      RegisterStandardAdmitsAndTracks
 * @tc.desc      A valid standard connect is admitted, tracked and produces a service connection.
 */
HWTEST_F(AgentConnectManagerTest, RegisterStandardAdmitsAndTracks, TestSize.Level1)
{
    auto &mgr = AgentConnectManager::GetInstance();
    auto conn = MakeConnection();
    auto request = MakeStandardRequest(conn, CALLER_UID, "agent-1", DefaultHostKey(), NONCE_A);
    EXPECT_EQ(mgr.RegisterStandardAgentConnection(request), ERR_OK);
    EXPECT_NE(request.serviceConnection, nullptr);
    ASSERT_EQ(mgr.trackedConnections_.size(), 1u);
    auto it = mgr.trackedConnections_.find(conn->AsObject());
    ASSERT_NE(it, mgr.trackedConnections_.end());
    const auto &record = it->second;
    EXPECT_EQ(record.callerUid, CALLER_UID);
    EXPECT_EQ(record.agentId, "agent-1");
    EXPECT_EQ(record.hostKey.bundleName, DefaultHostKey().bundleName);
    EXPECT_EQ(record.originalIdentity, "caller-identity");
    EXPECT_EQ(record.verificationNonce, NONCE_A);
    EXPECT_TRUE(record.hasQuota);
    EXPECT_FALSE(record.isLowCode);
    ASSERT_EQ(mgr.callerQuotas_.count(CALLER_UID), 1u);
    EXPECT_EQ(mgr.callerQuotas_[CALLER_UID].size(), 1u);
    EXPECT_EQ(mgr.callerQuotas_[CALLER_UID][request.quotaKey], 1u);
}

/**
 * @tc.name      RegisterStandardRejectsDuplicateAndRollsBackQuota
 * @tc.desc      Re-registering the same connection fails and the quota increment is rolled back.
 */
HWTEST_F(AgentConnectManagerTest, RegisterStandardRejectsDuplicateAndRollsBackQuota, TestSize.Level1)
{
    auto &mgr = AgentConnectManager::GetInstance();
    auto conn = MakeConnection();
    auto first = MakeStandardRequest(conn, CALLER_UID, "agent-1", DefaultHostKey(), NONCE_A);
    ASSERT_EQ(mgr.RegisterStandardAgentConnection(first), ERR_OK);

    auto second = MakeStandardRequest(conn, CALLER_UID, "agent-2", DefaultHostKey(), NONCE_B);
    EXPECT_EQ(mgr.RegisterStandardAgentConnection(second), ERR_INVALID_VALUE);
    EXPECT_EQ(mgr.trackedConnections_.size(), 1u);
    EXPECT_EQ(mgr.callerQuotas_[CALLER_UID].size(), 1u);
    EXPECT_EQ(mgr.callerQuotas_[CALLER_UID][first.quotaKey], 1u);
    EXPECT_EQ(mgr.callerQuotas_[CALLER_UID].count(second.quotaKey), 0u);
}

/**
 * @tc.name      RegisterStandardEnforcesCallerQuota
 * @tc.desc      The (MAX_AGENT_CONNECTIONS_PER_CALLER)th distinct quota key is rejected.
 */
HWTEST_F(AgentConnectManagerTest, RegisterStandardEnforcesCallerQuota, TestSize.Level1)
{
    auto &mgr = AgentConnectManager::GetInstance();
    auto host = DefaultHostKey();
    for (size_t i = 0; i < MAX_AGENT_CONNECTIONS_PER_CALLER; ++i) {
        auto conn = MakeConnection();
        auto request = MakeStandardRequest(conn, CALLER_UID, "agent-" + std::to_string(i), host, NONCE_A + i);
        ASSERT_EQ(mgr.RegisterStandardAgentConnection(request), ERR_OK) << "index " << i;
    }
    EXPECT_EQ(mgr.callerQuotas_[CALLER_UID].size(), MAX_AGENT_CONNECTIONS_PER_CALLER);

    auto overflow = MakeStandardRequest(MakeConnection(), CALLER_UID, "agent-overflow", host, NONCE_B);
    EXPECT_EQ(mgr.RegisterStandardAgentConnection(overflow), AAFwk::ERR_MAX_AGENT_CONNECTIONS_REACHED);
    EXPECT_EQ(mgr.callerQuotas_[CALLER_UID].size(), MAX_AGENT_CONNECTIONS_PER_CALLER);
    EXPECT_EQ(mgr.callerQuotas_[CALLER_UID].count(overflow.quotaKey), 0u);
}

// ---------------------------------------------------------------------------
// Tracked connection register (without quota)
// ---------------------------------------------------------------------------

/**
 * @tc.name      RegisterTrackedRejectsNullConnection
 * @tc.desc      A null connection yields a null identity remote and is rejected.
 */
HWTEST_F(AgentConnectManagerTest, RegisterTrackedRejectsNullConnection, TestSize.Level1)
{
    sptr<AAFwk::IAbilityConnection> serviceConnection;
    EXPECT_EQ(AgentConnectManager::GetInstance().RegisterTrackedConnectionAndGetServiceConnection(
        nullptr, CALLER_UID, NoopDeathHandler(), serviceConnection), ERR_INVALID_VALUE);
    EXPECT_EQ(serviceConnection, nullptr);
}

/**
 * @tc.name      RegisterTrackedSucceedsAndReturnsServiceConnection
 * @tc.desc      A tracked connection is registered and a service connection is produced.
 */
HWTEST_F(AgentConnectManagerTest, RegisterTrackedSucceedsAndReturnsServiceConnection, TestSize.Level1)
{
    auto &mgr = AgentConnectManager::GetInstance();
    auto conn = MakeConnection();
    sptr<AAFwk::IAbilityConnection> serviceConnection;
    EXPECT_EQ(mgr.RegisterTrackedConnectionAndGetServiceConnection(
        conn, CALLER_UID, NoopDeathHandler(), serviceConnection), ERR_OK);
    EXPECT_NE(serviceConnection, nullptr);
    ASSERT_EQ(mgr.trackedConnections_.size(), 1u);
    EXPECT_EQ(mgr.trackedConnections_.find(conn->AsObject())->second.serviceConnection, serviceConnection);
}

/**
 * @tc.name      RegisterTrackedRejectsDuplicate
 * @tc.desc      Re-registering the same connection is rejected.
 */
HWTEST_F(AgentConnectManagerTest, RegisterTrackedRejectsDuplicate, TestSize.Level1)
{
    auto &mgr = AgentConnectManager::GetInstance();
    auto conn = MakeConnection();
    sptr<AAFwk::IAbilityConnection> first;
    ASSERT_EQ(mgr.RegisterTrackedConnectionAndGetServiceConnection(conn, CALLER_UID, NoopDeathHandler(), first),
        ERR_OK);
    sptr<AAFwk::IAbilityConnection> second;
    EXPECT_EQ(mgr.RegisterTrackedConnectionAndGetServiceConnection(conn, CALLER_UID, NoopDeathHandler(), second),
        ERR_INVALID_VALUE);
    EXPECT_EQ(mgr.trackedConnections_.size(), 1u);
}

// ---------------------------------------------------------------------------
// Disconnect prepare / restore (standard path)
// ---------------------------------------------------------------------------

/**
 * @tc.name      PrepareAgentDisconnectFailsOnUntracked
 * @tc.desc      Preparing a disconnect for an untracked connection is an error.
 */
HWTEST_F(AgentConnectManagerTest, PrepareAgentDisconnectFailsOnUntracked, TestSize.Level1)
{
    AgentDisconnectRequest request;
    EXPECT_EQ(AgentConnectManager::GetInstance().PrepareAgentDisconnectRequest(
        MakeConnection(), CALLER_UID, request), ERR_INVALID_VALUE);
}

/**
 * @tc.name      PrepareAgentDisconnectMarksStandardDisconnecting
 * @tc.desc      A standard disconnect prepare stamps the disconnecting flag and fills the request.
 */
HWTEST_F(AgentConnectManagerTest, PrepareAgentDisconnectMarksStandardDisconnecting, TestSize.Level1)
{
    auto &mgr = AgentConnectManager::GetInstance();
    auto conn = MakeConnection();
    auto connect = MakeStandardRequest(conn, CALLER_UID, "agent-1", DefaultHostKey(), NONCE_A);
    ASSERT_EQ(mgr.RegisterStandardAgentConnection(connect), ERR_OK);

    AgentDisconnectRequest request;
    EXPECT_EQ(mgr.PrepareAgentDisconnectRequest(conn, CALLER_UID, request), ERR_OK);
    EXPECT_EQ(request.callerRemote, conn->AsObject());
    EXPECT_EQ(request.serviceConnection, connect.serviceConnection);
    EXPECT_FALSE(request.isLowCode);
    EXPECT_TRUE(mgr.trackedConnections_[conn->AsObject()].isDisconnecting);
}

/**
 * @tc.name      PrepareAgentDisconnectReportsAlreadyDisconnecting
 * @tc.desc      A second disconnect prepare for the same connection reports already-disconnecting.
 */
HWTEST_F(AgentConnectManagerTest, PrepareAgentDisconnectReportsAlreadyDisconnecting, TestSize.Level1)
{
    auto &mgr = AgentConnectManager::GetInstance();
    auto conn = MakeConnection();
    auto connect = MakeStandardRequest(conn, CALLER_UID, "agent-1", DefaultHostKey(), NONCE_A);
    ASSERT_EQ(mgr.RegisterStandardAgentConnection(connect), ERR_OK);
    AgentDisconnectRequest first;
    ASSERT_EQ(mgr.PrepareAgentDisconnectRequest(conn, CALLER_UID, first), ERR_OK);

    AgentDisconnectRequest second;
    EXPECT_EQ(mgr.PrepareAgentDisconnectRequest(conn, CALLER_UID, second), ERR_OK);
    EXPECT_TRUE(second.alreadyDisconnecting);
}

/**
 * @tc.name      RestoreStandardDisconnectingStateClearsFlag
 * @tc.desc      Restoring by remote clears the disconnecting flag.
 */
HWTEST_F(AgentConnectManagerTest, RestoreStandardDisconnectingStateClearsFlag, TestSize.Level1)
{
    auto &mgr = AgentConnectManager::GetInstance();
    auto conn = MakeConnection();
    auto connect = MakeStandardRequest(conn, CALLER_UID, "agent-1", DefaultHostKey(), NONCE_A);
    ASSERT_EQ(mgr.RegisterStandardAgentConnection(connect), ERR_OK);
    AgentDisconnectRequest request;
    ASSERT_EQ(mgr.PrepareAgentDisconnectRequest(conn, CALLER_UID, request), ERR_OK);
    ASSERT_TRUE(mgr.trackedConnections_[conn->AsObject()].isDisconnecting);

    mgr.RestoreStandardAgentDisconnectingState(conn->AsObject());
    EXPECT_FALSE(mgr.trackedConnections_[conn->AsObject()].isDisconnecting);
}

/**
 * @tc.name      PrepareServiceDisconnectFailsOnUntracked
 * @tc.desc      Preparing a service disconnect for an untracked connection is an error.
 */
HWTEST_F(AgentConnectManagerTest, PrepareServiceDisconnectFailsOnUntracked, TestSize.Level1)
{
    AgentDisconnectRequest request;
    EXPECT_EQ(AgentConnectManager::GetInstance().PrepareServiceDisconnectRequest(
        MakeConnection(), request), ERR_INVALID_VALUE);
}

/**
 * @tc.name      PrepareServiceDisconnectMarksDisconnecting
 * @tc.desc      A service disconnect prepare stamps the disconnecting flag and fills the request.
 */
HWTEST_F(AgentConnectManagerTest, PrepareServiceDisconnectMarksDisconnecting, TestSize.Level1)
{
    auto &mgr = AgentConnectManager::GetInstance();
    auto conn = MakeConnection();
    auto connect = MakeStandardRequest(conn, CALLER_UID, "agent-1", DefaultHostKey(), NONCE_A);
    ASSERT_EQ(mgr.RegisterStandardAgentConnection(connect), ERR_OK);

    AgentDisconnectRequest request;
    EXPECT_EQ(mgr.PrepareServiceDisconnectRequest(conn, request), ERR_OK);
    EXPECT_EQ(request.callerRemote, conn->AsObject());
    EXPECT_EQ(request.serviceConnection, connect.serviceConnection);
    EXPECT_TRUE(mgr.trackedConnections_[conn->AsObject()].isDisconnecting);
}

/**
 * @tc.name      PrepareServiceDisconnectReportsAlreadyDisconnecting
 * @tc.desc      A second service disconnect prepare reports already-disconnecting.
 */
HWTEST_F(AgentConnectManagerTest, PrepareServiceDisconnectReportsAlreadyDisconnecting, TestSize.Level1)
{
    auto &mgr = AgentConnectManager::GetInstance();
    auto conn = MakeConnection();
    auto connect = MakeStandardRequest(conn, CALLER_UID, "agent-1", DefaultHostKey(), NONCE_A);
    ASSERT_EQ(mgr.RegisterStandardAgentConnection(connect), ERR_OK);
    AgentDisconnectRequest first;
    ASSERT_EQ(mgr.PrepareServiceDisconnectRequest(conn, first), ERR_OK);

    AgentDisconnectRequest second;
    EXPECT_EQ(mgr.PrepareServiceDisconnectRequest(conn, second), ERR_OK);
    EXPECT_TRUE(second.alreadyDisconnecting);
}

/**
 * @tc.name      RestoreConnectionDisconnectingStateClearsFlag
 * @tc.desc      Restoring by connection clears the disconnecting flag.
 */
HWTEST_F(AgentConnectManagerTest, RestoreConnectionDisconnectingStateClearsFlag, TestSize.Level1)
{
    auto &mgr = AgentConnectManager::GetInstance();
    auto conn = MakeConnection();
    auto connect = MakeStandardRequest(conn, CALLER_UID, "agent-1", DefaultHostKey(), NONCE_A);
    ASSERT_EQ(mgr.RegisterStandardAgentConnection(connect), ERR_OK);
    AgentDisconnectRequest request;
    ASSERT_EQ(mgr.PrepareServiceDisconnectRequest(conn, request), ERR_OK);
    ASSERT_TRUE(mgr.trackedConnections_[conn->AsObject()].isDisconnecting);

    mgr.RestoreConnectionDisconnectingState(conn);
    EXPECT_FALSE(mgr.trackedConnections_[conn->AsObject()].isDisconnecting);
}

// ---------------------------------------------------------------------------
// Verify agent connect / disconnect requests (standard)
// ---------------------------------------------------------------------------

/**
 * @tc.name      VerifyConnectReturnsNotFoundForUnknownRemote
 * @tc.desc      A remote that owns no tracked connection yields CONNECTION_NOT_EXIST.
 */
HWTEST_F(AgentConnectManagerTest, VerifyConnectReturnsNotFoundForUnknownRemote, TestSize.Level1)
{
    std::string callerIdentity;
    EXPECT_EQ(AgentConnectManager::GetInstance().VerifyAgentConnectRequest(
        MakeConnection()->AsObject(), "agent-1", MakeVerifyWant(NONCE_A), callerIdentity),
        AAFwk::CONNECTION_NOT_EXIST);
    EXPECT_TRUE(callerIdentity.empty());
}

/**
 * @tc.name      VerifyConnectSucceedsForStandard
 * @tc.desc      A matching standard connection verifies with its original identity.
 */
HWTEST_F(AgentConnectManagerTest, VerifyConnectSucceedsForStandard, TestSize.Level1)
{
    auto &mgr = AgentConnectManager::GetInstance();
    auto conn = MakeConnection();
    auto request = MakeStandardRequest(conn, CALLER_UID, "agent-1", DefaultHostKey(), NONCE_A);
    ASSERT_EQ(mgr.RegisterStandardAgentConnection(request), ERR_OK);

    std::string callerIdentity;
    EXPECT_EQ(mgr.VerifyAgentConnectRequest(
        request.serviceConnection->AsObject(), "agent-1", MakeVerifyWant(NONCE_A), callerIdentity), ERR_OK);
    EXPECT_EQ(callerIdentity, "caller-identity");
}

/**
 * @tc.name      VerifyConnectFailsOnNonceMismatch
 * @tc.desc      A nonce mismatch for a standard connect yields ERR_WRONG_INTERFACE_CALL.
 */
HWTEST_F(AgentConnectManagerTest, VerifyConnectFailsOnNonceMismatch, TestSize.Level1)
{
    auto &mgr = AgentConnectManager::GetInstance();
    auto conn = MakeConnection();
    auto request = MakeStandardRequest(conn, CALLER_UID, "agent-1", DefaultHostKey(), NONCE_A);
    ASSERT_EQ(mgr.RegisterStandardAgentConnection(request), ERR_OK);

    std::string callerIdentity;
    EXPECT_EQ(mgr.VerifyAgentConnectRequest(
        request.serviceConnection->AsObject(), "agent-1", MakeVerifyWant(NONCE_B), callerIdentity),
        AAFwk::ERR_WRONG_INTERFACE_CALL);
}

/**
 * @tc.name      VerifyConnectFailsOnAgentIdMismatch
 * @tc.desc      A non-empty agentId that differs from the record yields ERR_WRONG_INTERFACE_CALL.
 */
HWTEST_F(AgentConnectManagerTest, VerifyConnectFailsOnAgentIdMismatch, TestSize.Level1)
{
    auto &mgr = AgentConnectManager::GetInstance();
    auto conn = MakeConnection();
    auto request = MakeStandardRequest(conn, CALLER_UID, "agent-1", DefaultHostKey(), NONCE_A);
    ASSERT_EQ(mgr.RegisterStandardAgentConnection(request), ERR_OK);

    std::string callerIdentity;
    EXPECT_EQ(mgr.VerifyAgentConnectRequest(
        request.serviceConnection->AsObject(), "agent-2", MakeVerifyWant(NONCE_A), callerIdentity),
        AAFwk::ERR_WRONG_INTERFACE_CALL);
}

/**
 * @tc.name      VerifyConnectSkipsAgentIdCheckWhenEmpty
 * @tc.desc      An empty requested agentId skips the agentId check and still verifies by nonce.
 */
HWTEST_F(AgentConnectManagerTest, VerifyConnectSkipsAgentIdCheckWhenEmpty, TestSize.Level1)
{
    auto &mgr = AgentConnectManager::GetInstance();
    auto conn = MakeConnection();
    auto request = MakeStandardRequest(conn, CALLER_UID, "agent-1", DefaultHostKey(), NONCE_A);
    ASSERT_EQ(mgr.RegisterStandardAgentConnection(request), ERR_OK);

    std::string callerIdentity;
    EXPECT_EQ(mgr.VerifyAgentConnectRequest(
        request.serviceConnection->AsObject(), "", MakeVerifyWant(NONCE_A), callerIdentity), ERR_OK);
    EXPECT_EQ(callerIdentity, "caller-identity");
}

/**
 * @tc.name      VerifyDisconnectSucceedsForMatchingStandardWant
 * @tc.desc      A standard disconnect with a matching nonce verifies.
 */
HWTEST_F(AgentConnectManagerTest, VerifyDisconnectSucceedsForMatchingStandardWant, TestSize.Level1)
{
    auto &mgr = AgentConnectManager::GetInstance();
    auto conn = MakeConnection();
    auto request = MakeStandardRequest(conn, CALLER_UID, "agent-1", DefaultHostKey(), NONCE_A);
    ASSERT_EQ(mgr.RegisterStandardAgentConnection(request), ERR_OK);

    std::vector<AAFwk::Want> wants { MakeVerifyWant(NONCE_A) };
    std::string callerIdentity;
    EXPECT_EQ(mgr.VerifyAgentDisconnectRequests(request.serviceConnection->AsObject(), wants, callerIdentity),
        ERR_OK);
    EXPECT_EQ(callerIdentity, "caller-identity");
}

/**
 * @tc.name      VerifyDisconnectReturnsNotFoundOnNonceMismatch
 * @tc.desc      A nonce mismatch falls through to low-code lookup and yields CONNECTION_NOT_EXIST.
 */
HWTEST_F(AgentConnectManagerTest, VerifyDisconnectReturnsNotFoundOnNonceMismatch, TestSize.Level1)
{
    auto &mgr = AgentConnectManager::GetInstance();
    auto conn = MakeConnection();
    auto request = MakeStandardRequest(conn, CALLER_UID, "agent-1", DefaultHostKey(), NONCE_A);
    ASSERT_EQ(mgr.RegisterStandardAgentConnection(request), ERR_OK);

    std::vector<AAFwk::Want> wants { MakeVerifyWant(NONCE_B) };
    std::string callerIdentity;
    EXPECT_EQ(mgr.VerifyAgentDisconnectRequests(request.serviceConnection->AsObject(), wants, callerIdentity),
        AAFwk::CONNECTION_NOT_EXIST);
}

/**
 * @tc.name      VerifyDisconnectReturnsNotFoundForEmptyWants
 * @tc.desc      With no wants to match, only the low-code batch is consulted and yields NOT_EXIST.
 */
HWTEST_F(AgentConnectManagerTest, VerifyDisconnectReturnsNotFoundForEmptyWants, TestSize.Level1)
{
    auto &mgr = AgentConnectManager::GetInstance();
    auto conn = MakeConnection();
    auto request = MakeStandardRequest(conn, CALLER_UID, "agent-1", DefaultHostKey(), NONCE_A);
    ASSERT_EQ(mgr.RegisterStandardAgentConnection(request), ERR_OK);

    std::vector<AAFwk::Want> wants;
    std::string callerIdentity;
    EXPECT_EQ(mgr.VerifyAgentDisconnectRequests(request.serviceConnection->AsObject(), wants, callerIdentity),
        AAFwk::CONNECTION_NOT_EXIST);
}

// ---------------------------------------------------------------------------
// Release / HandleConnectionDone
// ---------------------------------------------------------------------------

/**
 * @tc.name      ReleaseTrackedConnectionErasesRecord
 * @tc.desc      Releasing by connection erases the tracked record and releases quota.
 */
HWTEST_F(AgentConnectManagerTest, ReleaseTrackedConnectionErasesRecord, TestSize.Level1)
{
    auto &mgr = AgentConnectManager::GetInstance();
    auto conn = MakeConnection();
    auto connect = MakeStandardRequest(conn, CALLER_UID, "agent-1", DefaultHostKey(), NONCE_A);
    ASSERT_EQ(mgr.RegisterStandardAgentConnection(connect), ERR_OK);
    ASSERT_FALSE(mgr.callerQuotas_.empty());

    mgr.ReleaseTrackedConnection(conn);
    EXPECT_TRUE(mgr.trackedConnections_.empty());
    EXPECT_TRUE(mgr.callerQuotas_.empty());
}

/**
 * @tc.name      ReleaseTrackedConnectionByRemoteErasesRecord
 * @tc.desc      Releasing by remote erases the tracked record.
 */
HWTEST_F(AgentConnectManagerTest, ReleaseTrackedConnectionByRemoteErasesRecord, TestSize.Level1)
{
    auto &mgr = AgentConnectManager::GetInstance();
    auto conn = MakeConnection();
    auto connect = MakeStandardRequest(conn, CALLER_UID, "agent-1", DefaultHostKey(), NONCE_A);
    ASSERT_EQ(mgr.RegisterStandardAgentConnection(connect), ERR_OK);
    mgr.ReleaseTrackedConnectionByRemote(conn->AsObject());
    EXPECT_TRUE(mgr.trackedConnections_.empty());
}

/**
 * @tc.name      ReleaseUntrackedConnectionIsNoop
 * @tc.desc      Releasing an untracked connection is a safe no-op.
 */
HWTEST_F(AgentConnectManagerTest, ReleaseUntrackedConnectionIsNoop, TestSize.Level1)
{
    auto &mgr = AgentConnectManager::GetInstance();
    auto conn = MakeConnection();
    mgr.ReleaseTrackedConnection(conn);
    mgr.ReleaseTrackedConnectionByRemote(conn->AsObject());
    EXPECT_TRUE(mgr.trackedConnections_.empty());
}

/**
 * @tc.name      HandleConnectionDoneReleasesOnFailure
 * @tc.desc      A non-OK connect result releases the tracked connection.
 */
HWTEST_F(AgentConnectManagerTest, HandleConnectionDoneReleasesOnFailure, TestSize.Level1)
{
    auto &mgr = AgentConnectManager::GetInstance();
    auto conn = MakeConnection();
    auto connect = MakeStandardRequest(conn, CALLER_UID, "agent-1", DefaultHostKey(), NONCE_A);
    ASSERT_EQ(mgr.RegisterStandardAgentConnection(connect), ERR_OK);
    mgr.HandleConnectionDone(conn, AAFwk::INVALID_PARAMETERS_ERR, false);
    EXPECT_TRUE(mgr.trackedConnections_.empty());
}

/**
 * @tc.name      HandleConnectionDoneReleasesOnDisconnect
 * @tc.desc      A disconnect-done (even with OK) releases the tracked connection.
 */
HWTEST_F(AgentConnectManagerTest, HandleConnectionDoneReleasesOnDisconnect, TestSize.Level1)
{
    auto &mgr = AgentConnectManager::GetInstance();
    auto conn = MakeConnection();
    auto connect = MakeStandardRequest(conn, CALLER_UID, "agent-1", DefaultHostKey(), NONCE_A);
    ASSERT_EQ(mgr.RegisterStandardAgentConnection(connect), ERR_OK);
    mgr.HandleConnectionDone(conn, ERR_OK, true);
    EXPECT_TRUE(mgr.trackedConnections_.empty());
}

/**
 * @tc.name      HandleConnectionDoneKeepsOnSuccessConnect
 * @tc.desc      A successful connect-done keeps the tracked connection.
 */
HWTEST_F(AgentConnectManagerTest, HandleConnectionDoneKeepsOnSuccessConnect, TestSize.Level1)
{
    auto &mgr = AgentConnectManager::GetInstance();
    auto conn = MakeConnection();
    auto connect = MakeStandardRequest(conn, CALLER_UID, "agent-1", DefaultHostKey(), NONCE_A);
    ASSERT_EQ(mgr.RegisterStandardAgentConnection(connect), ERR_OK);
    mgr.HandleConnectionDone(conn, ERR_OK, false);
    EXPECT_EQ(mgr.trackedConnections_.size(), 1u);
}

// ---------------------------------------------------------------------------
// Caller death (standard path)
// ---------------------------------------------------------------------------

/**
 * @tc.name      PrepareCallerDeathReturnsFalseForNullRemote
 * @tc.desc      A null remote yields false.
 */
HWTEST_F(AgentConnectManagerTest, PrepareCallerDeathReturnsFalseForNullRemote, TestSize.Level1)
{
    AgentCallerDeathRequest request;
    EXPECT_FALSE(AgentConnectManager::GetInstance().PrepareCallerDeathRequest(nullptr, request));
}

/**
 * @tc.name      PrepareCallerDeathReturnsFalseForUntracked
 * @tc.desc      An untracked remote yields false.
 */
HWTEST_F(AgentConnectManagerTest, PrepareCallerDeathReturnsFalseForUntracked, TestSize.Level1)
{
    AgentCallerDeathRequest request;
    EXPECT_FALSE(AgentConnectManager::GetInstance().PrepareCallerDeathRequest(
        MakeConnection()->AsObject(), request));
}

/**
 * @tc.name      PrepareCallerDeathStandardSetsServiceConnection
 * @tc.desc      For a standard tracked connection, the service connection is returned and the
 *               disconnecting flag is set without releasing the tracked record.
 */
HWTEST_F(AgentConnectManagerTest, PrepareCallerDeathStandardSetsServiceConnection, TestSize.Level1)
{
    auto &mgr = AgentConnectManager::GetInstance();
    auto conn = MakeConnection();
    auto connect = MakeStandardRequest(conn, CALLER_UID, "agent-1", DefaultHostKey(), NONCE_A);
    ASSERT_EQ(mgr.RegisterStandardAgentConnection(connect), ERR_OK);

    AgentCallerDeathRequest request;
    EXPECT_TRUE(mgr.PrepareCallerDeathRequest(conn->AsObject(), request));
    EXPECT_EQ(request.serviceConnection, connect.serviceConnection);
    EXPECT_TRUE(mgr.trackedConnections_[conn->AsObject()].isDisconnecting);
    EXPECT_EQ(mgr.trackedConnections_.size(), 1u);
}

// ---------------------------------------------------------------------------
// Low-code connect plan
// ---------------------------------------------------------------------------

/**
 * @tc.name      PrepareLowCodePlanRejectsNullConnection
 * @tc.desc      A null connection yields a null identity remote and is rejected.
 */
HWTEST_F(AgentConnectManagerTest, PrepareLowCodePlanRejectsNullConnection, TestSize.Level1)
{
    AgentConnectPlan plan;
    EXPECT_EQ(AgentConnectManager::GetInstance().PrepareLowCodeConnectPlan(
        MakePlanRequest(nullptr, CALLER_UID, DefaultHostKey(), HOST_UID, "agent-1"), plan),
        ERR_INVALID_VALUE);
}

/**
 * @tc.name      PrepareLowCodePlanCreatesNewHostSession
 * @tc.desc      The first plan for a host creates a session, registers a tracked low-code
 *               connection and admits one agent.
 */
HWTEST_F(AgentConnectManagerTest, PrepareLowCodePlanCreatesNewHostSession, TestSize.Level1)
{
    auto &mgr = AgentConnectManager::GetInstance();
    auto conn = MakeConnection();
    auto host = DefaultHostKey();
    AgentConnectPlan plan;
    EXPECT_EQ(mgr.PrepareLowCodeConnectPlan(
        MakePlanRequest(conn, CALLER_UID, host, HOST_UID, "agent-1"), plan), ERR_OK);
    EXPECT_NE(plan.hostConnection, nullptr);
    EXPECT_TRUE(plan.needRealConnect);
    EXPECT_TRUE(plan.registeredTrackedConnection);
    EXPECT_EQ(plan.callerRemote, conn->AsObject());
    ASSERT_EQ(mgr.agentHostSessions_.size(), 1u);
    auto session = mgr.agentHostSessions_[host];
    EXPECT_EQ(session->agents.size(), 1u);
    EXPECT_TRUE(session->agents["agent-1"].isPending);
    EXPECT_EQ(mgr.callerQuotas_[CALLER_UID][mgr.BuildLowCodeQuotaKey(host)], 1u);
    ASSERT_EQ(mgr.trackedConnections_.size(), 1u);
    EXPECT_TRUE(mgr.trackedConnections_[conn->AsObject()].isLowCode);
    EXPECT_EQ(mgr.agentOwners_.count({CALLER_UID, "agent-1"}), 1u);
}

/**
 * @tc.name      PrepareLowCodePlanReusesHostSessionForSecondCaller
 * @tc.desc      A second caller on the same host reuses the session and registers its own tracked
 *               connection without a real connect.
 */
HWTEST_F(AgentConnectManagerTest, PrepareLowCodePlanReusesHostSessionForSecondCaller, TestSize.Level1)
{
    auto &mgr = AgentConnectManager::GetInstance();
    auto host = DefaultHostKey();
    AgentConnectPlan first;
    ASSERT_EQ(mgr.PrepareLowCodeConnectPlan(
        MakePlanRequest(MakeConnection(), CALLER_UID, host, HOST_UID, "agent-1"), first), ERR_OK);

    AgentConnectPlan second;
    EXPECT_EQ(mgr.PrepareLowCodeConnectPlan(
        MakePlanRequest(MakeConnection(), OTHER_CALLER_UID, host, HOST_UID, "agent-2"), second), ERR_OK);
    EXPECT_TRUE(second.reusedHostSession);
    EXPECT_FALSE(second.needRealConnect);
    EXPECT_TRUE(second.registeredTrackedConnection);
    EXPECT_EQ(mgr.agentHostSessions_[host]->agents.size(), 2u);
    EXPECT_EQ(mgr.trackedConnections_.size(), 2u);
}

/**
 * @tc.name      PrepareLowCodePlanReusesCallerConnectionForSecondAgent
 * @tc.desc      A second agent for the same caller reuses the tracked connection and host connection.
 */
HWTEST_F(AgentConnectManagerTest, PrepareLowCodePlanReusesCallerConnectionForSecondAgent, TestSize.Level1)
{
    auto &mgr = AgentConnectManager::GetInstance();
    auto conn = MakeConnection();
    auto host = DefaultHostKey();
    AgentConnectPlan first;
    ASSERT_EQ(mgr.PrepareLowCodeConnectPlan(
        MakePlanRequest(conn, CALLER_UID, host, HOST_UID, "agent-1"), first), ERR_OK);

    AgentConnectPlan second;
    EXPECT_EQ(mgr.PrepareLowCodeConnectPlan(
        MakePlanRequest(conn, CALLER_UID, host, HOST_UID, "agent-2"), second), ERR_OK);
    EXPECT_TRUE(second.reusedCallerConnection);
    EXPECT_FALSE(second.registeredTrackedConnection);
    EXPECT_EQ(second.hostConnection, first.hostConnection);
    EXPECT_EQ(mgr.trackedConnections_.size(), 1u);
    EXPECT_EQ(mgr.agentHostSessions_[host]->agents.size(), 2u);
    EXPECT_EQ(mgr.callerQuotas_[CALLER_UID][mgr.BuildLowCodeQuotaKey(host)], 1u);
}

/**
 * @tc.name      PrepareLowCodePlanRejectsDuplicateAgent
 * @tc.desc      Re-planning the same agentId on the same caller/host is rejected as already active.
 */
HWTEST_F(AgentConnectManagerTest, PrepareLowCodePlanRejectsDuplicateAgent, TestSize.Level1)
{
    auto &mgr = AgentConnectManager::GetInstance();
    auto conn = MakeConnection();
    auto host = DefaultHostKey();
    AgentConnectPlan first;
    ASSERT_EQ(mgr.PrepareLowCodeConnectPlan(
        MakePlanRequest(conn, CALLER_UID, host, HOST_UID, "agent-1"), first), ERR_OK);

    AgentConnectPlan second;
    EXPECT_EQ(mgr.PrepareLowCodeConnectPlan(
        MakePlanRequest(conn, CALLER_UID, host, HOST_UID, "agent-1"), second),
        AAFwk::ERR_LOW_CODE_AGENT_ALREADY_ACTIVE);
    EXPECT_EQ(mgr.agentHostSessions_[host]->agents.size(), 1u);
}

/**
 * @tc.name      PrepareLowCodePlanRejectsIncompatibleTrackedConnection
 * @tc.desc      Reusing a connection already tracked as standard yields an incompatible error.
 */
HWTEST_F(AgentConnectManagerTest, PrepareLowCodePlanRejectsIncompatibleTrackedConnection, TestSize.Level1)
{
    auto &mgr = AgentConnectManager::GetInstance();
    auto conn = MakeConnection();
    sptr<AAFwk::IAbilityConnection> serviceConnection;
    ASSERT_EQ(mgr.RegisterTrackedConnectionAndGetServiceConnection(
        conn, CALLER_UID, NoopDeathHandler(), serviceConnection), ERR_OK);

    AgentConnectPlan plan;
    EXPECT_EQ(mgr.PrepareLowCodeConnectPlan(
        MakePlanRequest(conn, CALLER_UID, DefaultHostKey(), HOST_UID, "agent-1"), plan),
        ERR_INVALID_VALUE);
}

/**
 * @tc.name      PrepareLowCodePlanEnforcesHostAgentQuota
 * @tc.desc      The (MAX_LOW_CODE_AGENTS_PER_HOST)th agent on a host is rejected.
 */
HWTEST_F(AgentConnectManagerTest, PrepareLowCodePlanEnforcesHostAgentQuota, TestSize.Level1)
{
    auto &mgr = AgentConnectManager::GetInstance();
    auto conn = MakeConnection();
    auto host = DefaultHostKey();
    AgentConnectPlan first;
    ASSERT_EQ(mgr.PrepareLowCodeConnectPlan(
        MakePlanRequest(conn, CALLER_UID, host, HOST_UID, "agent-0"), first), ERR_OK);
    for (size_t i = 1; i < MAX_LOW_CODE_AGENTS_PER_HOST; ++i) {
        AgentConnectPlan plan;
        ASSERT_EQ(mgr.PrepareLowCodeConnectPlan(
            MakePlanRequest(conn, CALLER_UID, host, HOST_UID, "agent-" + std::to_string(i)), plan), ERR_OK)
            << "index " << i;
    }
    EXPECT_EQ(mgr.agentHostSessions_[host]->agents.size(), MAX_LOW_CODE_AGENTS_PER_HOST);

    AgentConnectPlan overflow;
    EXPECT_EQ(mgr.PrepareLowCodeConnectPlan(
        MakePlanRequest(conn, CALLER_UID, host, HOST_UID, "agent-overflow"), overflow),
        AAFwk::ERR_MAX_AGENT_CONNECTIONS_REACHED);
    EXPECT_EQ(mgr.agentHostSessions_[host]->agents.size(), MAX_LOW_CODE_AGENTS_PER_HOST);
}

// ---------------------------------------------------------------------------
// Low-code identity / connect-done / verify
// ---------------------------------------------------------------------------

/**
 * @tc.name      SetLowCodeIdentityFailsOnMissingSession
 * @tc.desc      Setting identity for an unknown host is an error.
 */
HWTEST_F(AgentConnectManagerTest, SetLowCodeIdentityFailsOnMissingSession, TestSize.Level1)
{
    EXPECT_EQ(AgentConnectManager::GetInstance().SetLowCodeConnectIdentity(
        DefaultHostKey(), "agent-1", "caller-identity", NONCE_A), ERR_INVALID_VALUE);
}

/**
 * @tc.name      SetLowCodeIdentityFailsOnMissingAgent
 * @tc.desc      Setting identity for an unknown agent in a live session is an error.
 */
HWTEST_F(AgentConnectManagerTest, SetLowCodeIdentityFailsOnMissingAgent, TestSize.Level1)
{
    auto &mgr = AgentConnectManager::GetInstance();
    auto conn = MakeConnection();
    auto host = DefaultHostKey();
    AgentConnectPlan plan;
    ASSERT_EQ(mgr.PrepareLowCodeConnectPlan(
        MakePlanRequest(conn, CALLER_UID, host, HOST_UID, "agent-1"), plan), ERR_OK);
    EXPECT_EQ(mgr.SetLowCodeConnectIdentity(host, "agent-2", "caller-identity", NONCE_A),
        ERR_INVALID_VALUE);
}

/**
 * @tc.name      SetLowCodeIdentitySucceeds
 * @tc.desc      Identity is stamped onto the agent record.
 */
HWTEST_F(AgentConnectManagerTest, SetLowCodeIdentitySucceeds, TestSize.Level1)
{
    auto &mgr = AgentConnectManager::GetInstance();
    auto conn = MakeConnection();
    auto host = DefaultHostKey();
    AgentConnectPlan plan;
    ASSERT_EQ(mgr.PrepareLowCodeConnectPlan(
        MakePlanRequest(conn, CALLER_UID, host, HOST_UID, "agent-1"), plan), ERR_OK);
    EXPECT_EQ(mgr.SetLowCodeConnectIdentity(host, "agent-1", "caller-identity", NONCE_A), ERR_OK);
    const auto &agent = mgr.agentHostSessions_[host]->agents["agent-1"];
    EXPECT_EQ(agent.originalIdentity, "caller-identity");
    EXPECT_EQ(agent.verificationNonce, NONCE_A);
}

/**
 * @tc.name      HandleHostConnectDoneForUnknownSessionIsEmpty
 * @tc.desc      A connect-done for an unknown host returns an empty result.
 */
HWTEST_F(AgentConnectManagerTest, HandleHostConnectDoneForUnknownSessionIsEmpty, TestSize.Level1)
{
    AgentHostConnectDoneRequest request;
    request.hostKey = DefaultHostKey();
    request.callerRemote = MakeConnection()->AsObject();
    request.agentId = "agent-1";
    request.remoteObject = MakeConnection()->AsObject();
    request.resultCode = ERR_OK;
    auto result = AgentConnectManager::GetInstance().HandleAgentHostConnectDone(request);
    EXPECT_EQ(result.callback, nullptr);
    EXPECT_FALSE(result.releaseConnectionOnFailure);
}

/**
 * @tc.name      HandleHostConnectDoneSuccessConnectsSession
 * @tc.desc      A successful connect-done marks the session connected and clears the agent's pending flag.
 */
HWTEST_F(AgentConnectManagerTest, HandleHostConnectDoneSuccessConnectsSession, TestSize.Level1)
{
    auto &mgr = AgentConnectManager::GetInstance();
    auto conn = MakeConnection();
    auto host = DefaultHostKey();
    auto hostRemote = MakeConnection()->AsObject();
    AgentConnectPlan plan;
    SetupConnectedLowCodeAgent(conn, host, "agent-1", NONCE_A, hostRemote, plan);
    auto session = mgr.agentHostSessions_[host];
    EXPECT_TRUE(session->isConnected);
    EXPECT_EQ(session->remoteObject, hostRemote);
    EXPECT_FALSE(session->agents["agent-1"].isPending);
}

/**
 * @tc.name      HandleHostConnectDoneFailureReleasesAgent
 * @tc.desc      A failed connect-done removes the agent and (for a sole caller) the tracked connection.
 */
HWTEST_F(AgentConnectManagerTest, HandleHostConnectDoneFailureReleasesAgent, TestSize.Level1)
{
    auto &mgr = AgentConnectManager::GetInstance();
    auto conn = MakeConnection();
    auto host = DefaultHostKey();
    AgentConnectPlan plan;
    ASSERT_EQ(mgr.PrepareLowCodeConnectPlan(
        MakePlanRequest(conn, CALLER_UID, host, HOST_UID, "agent-1"), plan), ERR_OK);
    ASSERT_EQ(mgr.SetLowCodeConnectIdentity(host, "agent-1", "caller-identity", NONCE_A), ERR_OK);

    AgentHostConnectDoneRequest request;
    request.hostKey = host;
    request.callerRemote = conn->AsObject();
    request.agentId = "agent-1";
    request.remoteObject = nullptr;
    request.resultCode = AAFwk::INVALID_PARAMETERS_ERR;
    auto result = mgr.HandleAgentHostConnectDone(request);
    EXPECT_EQ(result.callback->AsObject(), conn->AsObject());
    EXPECT_TRUE(result.releaseConnectionOnFailure);
    EXPECT_TRUE(mgr.agentHostSessions_.empty());
    EXPECT_TRUE(mgr.trackedConnections_.empty());
    EXPECT_TRUE(mgr.agentOwners_.empty());
    EXPECT_TRUE(mgr.callerQuotas_.empty());
}

/**
 * @tc.name      VerifyConnectSucceedsForLowCode
 * @tc.desc      A connected low-code agent verifies by host remote, nonce and identity.
 */
HWTEST_F(AgentConnectManagerTest, VerifyConnectSucceedsForLowCode, TestSize.Level1)
{
    auto &mgr = AgentConnectManager::GetInstance();
    auto conn = MakeConnection();
    auto host = DefaultHostKey();
    auto hostRemote = MakeConnection()->AsObject();
    AgentConnectPlan plan;
    SetupConnectedLowCodeAgent(conn, host, "agent-1", NONCE_A, hostRemote, plan);

    std::string callerIdentity;
    EXPECT_EQ(mgr.VerifyAgentConnectRequest(
        plan.hostConnection->AsObject(), "agent-1", MakeVerifyWant(NONCE_A), callerIdentity), ERR_OK);
    EXPECT_EQ(callerIdentity, "caller-identity");
}

/**
 * @tc.name      VerifyConnectLowCodeFailsOnNonceMismatch
 * @tc.desc      A nonce mismatch for a low-code connect yields ERR_WRONG_INTERFACE_CALL.
 */
HWTEST_F(AgentConnectManagerTest, VerifyConnectLowCodeFailsOnNonceMismatch, TestSize.Level1)
{
    auto &mgr = AgentConnectManager::GetInstance();
    auto conn = MakeConnection();
    auto host = DefaultHostKey();
    auto hostRemote = MakeConnection()->AsObject();
    AgentConnectPlan plan;
    SetupConnectedLowCodeAgent(conn, host, "agent-1", NONCE_A, hostRemote, plan);

    std::string callerIdentity;
    EXPECT_EQ(mgr.VerifyAgentConnectRequest(
        plan.hostConnection->AsObject(), "agent-1", MakeVerifyWant(NONCE_B), callerIdentity),
        AAFwk::ERR_WRONG_INTERFACE_CALL);
}

/**
 * @tc.name      VerifyConnectLowCodeSkipsOnAgentIdMismatch
 * @tc.desc      A non-matching agentId skips the record and yields CONNECTION_NOT_EXIST.
 */
HWTEST_F(AgentConnectManagerTest, VerifyConnectLowCodeSkipsOnAgentIdMismatch, TestSize.Level1)
{
    auto &mgr = AgentConnectManager::GetInstance();
    auto conn = MakeConnection();
    auto host = DefaultHostKey();
    auto hostRemote = MakeConnection()->AsObject();
    AgentConnectPlan plan;
    SetupConnectedLowCodeAgent(conn, host, "agent-1", NONCE_A, hostRemote, plan);

    std::string callerIdentity;
    EXPECT_EQ(mgr.VerifyAgentConnectRequest(
        plan.hostConnection->AsObject(), "agent-2", MakeVerifyWant(NONCE_A), callerIdentity),
        AAFwk::CONNECTION_NOT_EXIST);
}

// ---------------------------------------------------------------------------
// Low-code disconnect (PrepareAgentDisconnectRequest low-code path)
// ---------------------------------------------------------------------------

/**
 * @tc.name      LowCodeDisconnectPreparesTargets
 * @tc.desc      Preparing a low-code disconnect schedules a target and marks the agent disconnecting.
 */
HWTEST_F(AgentConnectManagerTest, LowCodeDisconnectPreparesTargets, TestSize.Level1)
{
    auto &mgr = AgentConnectManager::GetInstance();
    auto conn = MakeConnection();
    auto host = DefaultHostKey();
    auto hostRemote = MakeConnection()->AsObject();
    AgentConnectPlan plan;
    SetupConnectedLowCodeAgent(conn, host, "agent-1", NONCE_A, hostRemote, plan);
    auto session = mgr.agentHostSessions_[host];
    auto hostConnRemote = plan.hostConnection->AsObject();

    AgentDisconnectRequest request;
    EXPECT_EQ(mgr.PrepareAgentDisconnectRequest(conn, CALLER_UID, request), ERR_OK);
    EXPECT_TRUE(request.isLowCode);
    EXPECT_EQ(request.hostKey.bundleName, host.bundleName);
    EXPECT_EQ(request.callerRemote, conn->AsObject());
    ASSERT_EQ(request.lowCodeTargets.size(), 1u);
    EXPECT_EQ(request.lowCodeTargets[0].agentIds, std::set<std::string> { "agent-1" });
    EXPECT_TRUE(session->agents["agent-1"].isDisconnecting);
    EXPECT_TRUE(mgr.trackedConnections_[conn->AsObject()].isDisconnecting);
    EXPECT_FALSE(session->pendingDisconnects[hostConnRemote].empty());
}

/**
 * @tc.name      LowCodeDisconnectReportsAlreadyDisconnecting
 * @tc.desc      A second low-code disconnect prepare reports already-disconnecting.
 */
HWTEST_F(AgentConnectManagerTest, LowCodeDisconnectReportsAlreadyDisconnecting, TestSize.Level1)
{
    auto &mgr = AgentConnectManager::GetInstance();
    auto conn = MakeConnection();
    auto host = DefaultHostKey();
    auto hostRemote = MakeConnection()->AsObject();
    AgentConnectPlan plan;
    SetupConnectedLowCodeAgent(conn, host, "agent-1", NONCE_A, hostRemote, plan);
    AgentDisconnectRequest first;
    ASSERT_EQ(mgr.PrepareAgentDisconnectRequest(conn, CALLER_UID, first), ERR_OK);

    AgentDisconnectRequest second;
    EXPECT_EQ(mgr.PrepareAgentDisconnectRequest(conn, CALLER_UID, second), ERR_OK);
    EXPECT_TRUE(second.alreadyDisconnecting);
    EXPECT_TRUE(second.lowCodeTargets.empty());
}

/**
 * @tc.name      LowCodeDisconnectFailsOnMissingIdentity
 * @tc.desc      Preparing a low-code disconnect before identity is set is an error.
 */
HWTEST_F(AgentConnectManagerTest, LowCodeDisconnectFailsOnMissingIdentity, TestSize.Level1)
{
    auto &mgr = AgentConnectManager::GetInstance();
    auto conn = MakeConnection();
    auto host = DefaultHostKey();
    auto hostRemote = MakeConnection()->AsObject();
    AgentConnectPlan plan;
    // plan only, no SetLowCodeConnectIdentity / connect-done
    ASSERT_EQ(mgr.PrepareLowCodeConnectPlan(
        MakePlanRequest(conn, CALLER_UID, host, HOST_UID, "agent-1"), plan), ERR_OK);

    AgentDisconnectRequest request;
    EXPECT_EQ(mgr.PrepareAgentDisconnectRequest(conn, CALLER_UID, request),
        ERR_INVALID_VALUE);
}

/**
 * @tc.name      VerifyDisconnectSucceedsForLowCodeBatch
 * @tc.desc      After a low-code disconnect prepare, the pending batch verifies.
 */
HWTEST_F(AgentConnectManagerTest, VerifyDisconnectSucceedsForLowCodeBatch, TestSize.Level1)
{
    auto &mgr = AgentConnectManager::GetInstance();
    auto conn = MakeConnection();
    auto host = DefaultHostKey();
    auto hostRemote = MakeConnection()->AsObject();
    AgentConnectPlan plan;
    SetupConnectedLowCodeAgent(conn, host, "agent-1", NONCE_A, hostRemote, plan);
    AgentDisconnectRequest request;
    ASSERT_EQ(mgr.PrepareAgentDisconnectRequest(conn, CALLER_UID, request), ERR_OK);

    std::vector<AAFwk::Want> wants { MakeVerifyWant(NONCE_A) };
    std::string callerIdentity;
    EXPECT_EQ(mgr.VerifyAgentDisconnectRequests(plan.hostConnection->AsObject(), wants, callerIdentity),
        ERR_OK);
    EXPECT_EQ(callerIdentity, "caller-identity");
}

/**
 * @tc.name      HandleHostDisconnectDoneReleasesAgentsAndConnection
 * @tc.desc      A host disconnect-done removes the agents, releases the tracked connection and session.
 */
HWTEST_F(AgentConnectManagerTest, HandleHostDisconnectDoneReleasesAgentsAndConnection, TestSize.Level1)
{
    auto &mgr = AgentConnectManager::GetInstance();
    auto conn = MakeConnection();
    auto host = DefaultHostKey();
    auto hostRemote = MakeConnection()->AsObject();
    AgentConnectPlan plan;
    SetupConnectedLowCodeAgent(conn, host, "agent-1", NONCE_A, hostRemote, plan);
    AgentDisconnectRequest request;
    ASSERT_EQ(mgr.PrepareAgentDisconnectRequest(conn, CALLER_UID, request), ERR_OK);

    AgentHostDisconnectDoneRequest done;
    done.hostKey = host;
    done.callerRemote = conn->AsObject();
    done.hostConnectionRemote = plan.hostConnection->AsObject();
    done.resultCode = ERR_OK;
    auto result = mgr.HandleAgentHostDisconnectDone(done);
    ASSERT_EQ(result.callbacks.size(), 1u);
    EXPECT_EQ(result.callbacks[0]->AsObject(), conn->AsObject());
    EXPECT_TRUE(mgr.agentHostSessions_.empty());
    EXPECT_TRUE(mgr.trackedConnections_.empty());
    EXPECT_TRUE(mgr.agentOwners_.empty());
    EXPECT_TRUE(mgr.callerQuotas_.empty());
}

// ---------------------------------------------------------------------------
// PrepareLowCodeComplete / disconnect queue / restore / next
// ---------------------------------------------------------------------------

/**
 * @tc.name      PrepareLowCodeCompleteFailsOnUnknownOwner
 * @tc.desc      Completing an agent with no owner record yields ERR_INVALID_AGENT_CARD_ID.
 */
HWTEST_F(AgentConnectManagerTest, PrepareLowCodeCompleteFailsOnUnknownOwner, TestSize.Level1)
{
    LowCodeCompleteRequest request;
    EXPECT_EQ(AgentConnectManager::GetInstance().PrepareLowCodeComplete(
        "agent-1", CALLER_UID, request), AAFwk::ERR_INVALID_AGENT_CARD_ID);
}

/**
 * @tc.name      PrepareLowCodeCompleteSchedulesWhenIdle
 * @tc.desc      Completing a sole connected agent schedules a disconnect and marks it disconnecting.
 */
HWTEST_F(AgentConnectManagerTest, PrepareLowCodeCompleteSchedulesWhenIdle, TestSize.Level1)
{
    auto &mgr = AgentConnectManager::GetInstance();
    auto conn = MakeConnection();
    auto host = DefaultHostKey();
    auto hostRemote = MakeConnection()->AsObject();
    AgentConnectPlan plan;
    SetupConnectedLowCodeAgent(conn, host, "agent-1", NONCE_A, hostRemote, plan);

    LowCodeCompleteRequest request;
    EXPECT_EQ(mgr.PrepareLowCodeComplete("agent-1", CALLER_UID, request), ERR_OK);
    EXPECT_EQ(request.hostKey.bundleName, host.bundleName);
    EXPECT_EQ(request.agentId, "agent-1");
    EXPECT_EQ(request.hostConnection, plan.hostConnection);
    EXPECT_TRUE(mgr.agentHostSessions_[host]->agents["agent-1"].isDisconnecting);
    EXPECT_FALSE(mgr.agentHostSessions_[host]->pendingDisconnects[plan.hostConnection->AsObject()].empty());
}

/**
 * @tc.name      PrepareLowCodeCompleteNoopWhenAlreadyDisconnecting
 * @tc.desc      Completing an already-disconnecting agent is a no-op that does not fill the request.
 */
HWTEST_F(AgentConnectManagerTest, PrepareLowCodeCompleteNoopWhenAlreadyDisconnecting, TestSize.Level1)
{
    auto &mgr = AgentConnectManager::GetInstance();
    auto conn = MakeConnection();
    auto host = DefaultHostKey();
    auto hostRemote = MakeConnection()->AsObject();
    AgentConnectPlan plan;
    SetupConnectedLowCodeAgent(conn, host, "agent-1", NONCE_A, hostRemote, plan);
    LowCodeCompleteRequest first;
    ASSERT_EQ(mgr.PrepareLowCodeComplete("agent-1", CALLER_UID, first), ERR_OK);

    LowCodeCompleteRequest second;
    EXPECT_EQ(mgr.PrepareLowCodeComplete("agent-1", CALLER_UID, second), ERR_OK);
    EXPECT_EQ(second.hostConnection, nullptr);
}

/**
 * @tc.name      PrepareLowCodeCompleteRemovesImmediatelyWhenCallerHasOtherAgent
 * @tc.desc      When the caller still owns another agent on the host, completion removes the agent
 *               immediately without scheduling a host disconnect.
 */
HWTEST_F(AgentConnectManagerTest, PrepareLowCodeCompleteRemovesImmediatelyWhenCallerHasOtherAgent, TestSize.Level1)
{
    auto &mgr = AgentConnectManager::GetInstance();
    auto conn = MakeConnection();
    auto host = DefaultHostKey();
    auto hostRemote = MakeConnection()->AsObject();
    AgentConnectPlan plan1;
    SetupConnectedLowCodeAgent(conn, host, "agent-1", NONCE_A, hostRemote, plan1);
    AgentConnectPlan plan2;
    ASSERT_EQ(mgr.PrepareLowCodeConnectPlan(
        MakePlanRequest(conn, CALLER_UID, host, HOST_UID, "agent-2"), plan2), ERR_OK);
    ASSERT_EQ(mgr.SetLowCodeConnectIdentity(host, "agent-2", "caller-identity", NONCE_B), ERR_OK);
    AgentHostConnectDoneRequest done;
    done.hostKey = host;
    done.callerRemote = conn->AsObject();
    done.agentId = "agent-2";
    done.remoteObject = hostRemote;
    done.resultCode = ERR_OK;
    ASSERT_EQ(mgr.HandleAgentHostConnectDone(done).callback->AsObject(), conn->AsObject());

    LowCodeCompleteRequest request;
    EXPECT_EQ(mgr.PrepareLowCodeComplete("agent-1", CALLER_UID, request), ERR_OK);
    EXPECT_EQ(request.hostConnection, nullptr);
    auto session = mgr.agentHostSessions_[host];
    EXPECT_EQ(session->agents.size(), 1u);
    EXPECT_EQ(session->agents.count("agent-1"), 0u);
    EXPECT_EQ(mgr.agentOwners_.count({CALLER_UID, "agent-1"}), 0u);
    EXPECT_EQ(mgr.agentOwners_.count({CALLER_UID, "agent-2"}), 1u);
    EXPECT_EQ(mgr.callerQuotas_[CALLER_UID][mgr.BuildLowCodeQuotaKey(host)], 1u);
}

/**
 * @tc.name      PrepareLowCodeCompleteQueuesWhenPendingExists
 * @tc.desc      When a pending disconnect already exists, a second completion queues but does not
 *               schedule; PrepareNext then drains the queue.
 */
HWTEST_F(AgentConnectManagerTest, PrepareLowCodeCompleteQueuesWhenPendingExists, TestSize.Level1)
{
    auto &mgr = AgentConnectManager::GetInstance();
    auto conn = MakeConnection();
    auto conn2 = MakeConnection();
    auto host = DefaultHostKey();
    auto hostRemote = MakeConnection()->AsObject();
    AgentConnectPlan plan1;
    SetupConnectedLowCodeAgent(conn, host, "agent-1", NONCE_A, hostRemote, plan1);
    AgentConnectPlan plan2;
    ASSERT_EQ(mgr.PrepareLowCodeConnectPlan(
        MakePlanRequest(conn2, OTHER_CALLER_UID, host, HOST_UID, "agent-2"), plan2), ERR_OK);
    ASSERT_EQ(mgr.SetLowCodeConnectIdentity(host, "agent-2", "caller-identity", NONCE_B), ERR_OK);
    AgentHostConnectDoneRequest done;
    done.hostKey = host;
    done.callerRemote = conn2->AsObject();
    done.agentId = "agent-2";
    done.remoteObject = hostRemote;
    done.resultCode = ERR_OK;
    ASSERT_EQ(mgr.HandleAgentHostConnectDone(done).callback->AsObject(), conn2->AsObject());

    // agent-1 is the sole agent for caller A -> its completion schedules; agent-2 (caller B) finds a
    // pending disconnect already present -> it queues without scheduling. Each caller owns a distinct
    // AgentHostConnection, so the two pending batches live under separate host remotes.
    LowCodeCompleteRequest first;
    ASSERT_EQ(mgr.PrepareLowCodeComplete("agent-1", CALLER_UID, first), ERR_OK);
    EXPECT_NE(first.hostConnection, nullptr);
    LowCodeCompleteRequest second;
    ASSERT_EQ(mgr.PrepareLowCodeComplete("agent-2", OTHER_CALLER_UID, second), ERR_OK);
    EXPECT_EQ(second.hostConnection, nullptr);
    auto session = mgr.agentHostSessions_[host];
    EXPECT_TRUE(session->agents["agent-1"].isDisconnecting);
    EXPECT_TRUE(session->agents["agent-2"].isDisconnecting);
    EXPECT_EQ(session->pendingDisconnects.size(), 2u);
    EXPECT_EQ(session->pendingDisconnects[plan1.hostConnection->AsObject()].size(), 1u);

    // PrepareNext drains one batch; which one is pointer-order-dependent, so assert order-independently.
    LowCodeDisconnectTarget target;
    sptr<IRemoteObject> callerRemote;
    EXPECT_TRUE(mgr.PrepareNextLowCodeDisconnectRequest(host, target, callerRemote));
    EXPECT_EQ(target.agentIds.size(), 1u);
    EXPECT_TRUE(target.hostConnection == plan1.hostConnection || target.hostConnection == plan2.hostConnection);
    EXPECT_TRUE(callerRemote == conn->AsObject() || callerRemote == conn2->AsObject());
}

/**
 * @tc.name      PrepareNextLowCodeDisconnectReturnsFalseWhenIdle
 * @tc.desc      With no pending disconnects, PrepareNext returns false.
 */
HWTEST_F(AgentConnectManagerTest, PrepareNextLowCodeDisconnectReturnsFalseWhenIdle, TestSize.Level1)
{
    auto &mgr = AgentConnectManager::GetInstance();
    auto conn = MakeConnection();
    auto host = DefaultHostKey();
    auto hostRemote = MakeConnection()->AsObject();
    AgentConnectPlan plan;
    SetupConnectedLowCodeAgent(conn, host, "agent-1", NONCE_A, hostRemote, plan);

    LowCodeDisconnectTarget target;
    sptr<IRemoteObject> callerRemote;
    EXPECT_FALSE(mgr.PrepareNextLowCodeDisconnectRequest(host, target, callerRemote));
}

/**
 * @tc.name      RestoreLowCodeDisconnectingStateClearsFlags
 * @tc.desc      Restore clears the pending records and the disconnecting flags.
 */
HWTEST_F(AgentConnectManagerTest, RestoreLowCodeDisconnectingStateClearsFlags, TestSize.Level1)
{
    auto &mgr = AgentConnectManager::GetInstance();
    auto conn = MakeConnection();
    auto host = DefaultHostKey();
    auto hostRemote = MakeConnection()->AsObject();
    AgentConnectPlan plan1;
    SetupConnectedLowCodeAgent(conn, host, "agent-1", NONCE_A, hostRemote, plan1);
    AgentConnectPlan plan2;
    ASSERT_EQ(mgr.PrepareLowCodeConnectPlan(
        MakePlanRequest(conn, CALLER_UID, host, HOST_UID, "agent-2"), plan2), ERR_OK);
    ASSERT_EQ(mgr.SetLowCodeConnectIdentity(host, "agent-2", "caller-identity", NONCE_B), ERR_OK);
    AgentHostConnectDoneRequest done;
    done.hostKey = host;
    done.callerRemote = conn->AsObject();
    done.agentId = "agent-2";
    done.remoteObject = hostRemote;
    done.resultCode = ERR_OK;
    ASSERT_EQ(mgr.HandleAgentHostConnectDone(done).callback->AsObject(), conn->AsObject());

    // Both agents share caller A's host connection, so one PrepareAgentDisconnectRequest schedules a
    // single deterministic batch containing both agents and marks both disconnecting.
    AgentDisconnectRequest request;
    ASSERT_EQ(mgr.PrepareAgentDisconnectRequest(conn, CALLER_UID, request), ERR_OK);
    ASSERT_EQ(request.lowCodeTargets.size(), 1u);
    EXPECT_EQ(request.lowCodeTargets[0].agentIds, (std::set<std::string> { "agent-1", "agent-2" }));
    EXPECT_TRUE(mgr.agentHostSessions_[host]->agents["agent-1"].isDisconnecting);
    EXPECT_TRUE(mgr.agentHostSessions_[host]->agents["agent-2"].isDisconnecting);

    mgr.RestoreLowCodeDisconnectingState(host, conn->AsObject(), { "agent-1", "agent-2" });
    auto session = mgr.agentHostSessions_[host];
    EXPECT_FALSE(session->agents["agent-1"].isDisconnecting);
    EXPECT_FALSE(session->agents["agent-2"].isDisconnecting);
    EXPECT_TRUE(session->pendingDisconnects.empty());
    EXPECT_FALSE(mgr.trackedConnections_[conn->AsObject()].isDisconnecting);
}

// ---------------------------------------------------------------------------
// CleanupLowCodeConnectPlan / CleanupLowCodeCallerDeathTargets / quota
// ---------------------------------------------------------------------------

/**
 * @tc.name      CleanupLowCodeConnectPlanRemovesAgentAndSession
 * @tc.desc      Cleaning up a sole low-code agent removes the agent, quota, session and tracked conn.
 */
HWTEST_F(AgentConnectManagerTest, CleanupLowCodeConnectPlanRemovesAgentAndSession, TestSize.Level1)
{
    auto &mgr = AgentConnectManager::GetInstance();
    auto conn = MakeConnection();
    auto host = DefaultHostKey();
    auto hostRemote = MakeConnection()->AsObject();
    AgentConnectPlan plan;
    SetupConnectedLowCodeAgent(conn, host, "agent-1", NONCE_A, hostRemote, plan);

    mgr.CleanupLowCodeConnectPlan(plan, "agent-1");
    EXPECT_TRUE(mgr.agentHostSessions_.empty());
    EXPECT_TRUE(mgr.agentOwners_.empty());
    EXPECT_TRUE(mgr.trackedConnections_.empty());
    EXPECT_TRUE(mgr.callerQuotas_.empty());
}

/**
 * @tc.name      CleanupLowCodeConnectPlanKeepsSessionForRemainingAgent
 * @tc.desc      Cleaning up one of two same-caller agents keeps the session for the other agent.
 */
HWTEST_F(AgentConnectManagerTest, CleanupLowCodeConnectPlanKeepsSessionForRemainingAgent, TestSize.Level1)
{
    auto &mgr = AgentConnectManager::GetInstance();
    auto conn = MakeConnection();
    auto host = DefaultHostKey();
    auto hostRemote = MakeConnection()->AsObject();
    AgentConnectPlan plan1;
    SetupConnectedLowCodeAgent(conn, host, "agent-1", NONCE_A, hostRemote, plan1);
    AgentConnectPlan plan2;
    ASSERT_EQ(mgr.PrepareLowCodeConnectPlan(
        MakePlanRequest(conn, CALLER_UID, host, HOST_UID, "agent-2"), plan2), ERR_OK);

    mgr.CleanupLowCodeConnectPlan(plan1, "agent-1");
    auto session = mgr.agentHostSessions_[host];
    EXPECT_EQ(session->agents.size(), 1u);
    EXPECT_EQ(session->agents.count("agent-2"), 1u);
    EXPECT_EQ(mgr.agentOwners_.count({CALLER_UID, "agent-2"}), 1u);
    EXPECT_EQ(mgr.trackedConnections_.size(), 1u);
    EXPECT_EQ(mgr.callerQuotas_[CALLER_UID][mgr.BuildLowCodeQuotaKey(host)], 1u);
}

/**
 * @tc.name      PrepareCallerDeathLowCodePreparesTargetsAndReleasesTracked
 * @tc.desc      Caller death for a low-code connection prepares targets, queues pending disconnects
 *               and releases the tracked connection (quota is released by later disconnect-done).
 */
HWTEST_F(AgentConnectManagerTest, PrepareCallerDeathLowCodePreparesTargetsAndReleasesTracked, TestSize.Level1)
{
    auto &mgr = AgentConnectManager::GetInstance();
    auto conn = MakeConnection();
    auto host = DefaultHostKey();
    auto hostRemote = MakeConnection()->AsObject();
    AgentConnectPlan plan;
    SetupConnectedLowCodeAgent(conn, host, "agent-1", NONCE_A, hostRemote, plan);

    AgentCallerDeathRequest request;
    EXPECT_TRUE(mgr.PrepareCallerDeathRequest(conn->AsObject(), request));
    EXPECT_EQ(request.hostKey.bundleName, host.bundleName);
    ASSERT_EQ(request.lowCodeTargets.size(), 1u);
    EXPECT_EQ(request.lowCodeTargets[0].agentIds, std::set<std::string> { "agent-1" });
    EXPECT_TRUE(request.lowCodeTargets[0].cleanupOnFailure);
    EXPECT_TRUE(mgr.trackedConnections_.empty());
    auto session = mgr.agentHostSessions_[host];
    EXPECT_TRUE(session->agents["agent-1"].isDisconnecting);
    EXPECT_TRUE(session->callerConnections.empty());
    EXPECT_EQ(mgr.callerQuotas_[CALLER_UID][mgr.BuildLowCodeQuotaKey(host)], 1u);
}

/**
 * @tc.name      CleanupLowCodeCallerDeathTargetsRemovesAgents
 * @tc.desc      Cleaning up all caller-owned agents tears down the session and owners.
 */
HWTEST_F(AgentConnectManagerTest, CleanupLowCodeCallerDeathTargetsRemovesAgents, TestSize.Level1)
{
    auto &mgr = AgentConnectManager::GetInstance();
    auto conn = MakeConnection();
    auto host = DefaultHostKey();
    auto hostRemote = MakeConnection()->AsObject();
    AgentConnectPlan plan1;
    SetupConnectedLowCodeAgent(conn, host, "agent-1", NONCE_A, hostRemote, plan1);
    AgentConnectPlan plan2;
    ASSERT_EQ(mgr.PrepareLowCodeConnectPlan(
        MakePlanRequest(conn, CALLER_UID, host, HOST_UID, "agent-2"), plan2), ERR_OK);

    mgr.CleanupLowCodeCallerDeathTargets(host, conn->AsObject(), { "agent-1", "agent-2" });
    EXPECT_TRUE(mgr.agentHostSessions_.empty());
    EXPECT_TRUE(mgr.agentOwners_.empty());
    EXPECT_TRUE(mgr.callerQuotas_.empty());
    EXPECT_EQ(mgr.trackedConnections_.size(), 1u);
}

// ---------------------------------------------------------------------------
// BuildLowCodeQuotaKey
// ---------------------------------------------------------------------------

/**
 * @tc.name      BuildLowCodeQuotaKeySetsLowCodeFlag
 * @tc.desc      The low-code quota key copies the host key and sets the low-code flag.
 */
HWTEST_F(AgentConnectManagerTest, BuildLowCodeQuotaKeySetsLowCodeFlag, TestSize.Level1)
{
    auto host = DefaultHostKey();
    auto key = AgentConnectManager::GetInstance().BuildLowCodeQuotaKey(host);
    EXPECT_TRUE(key.isLowCode);
    EXPECT_EQ(key.hostKey.bundleName, host.bundleName);
    EXPECT_TRUE(key.agentId.empty());
}

// ---------------------------------------------------------------------------
// Additional error-path / edge-branch coverage surfaced by review
// ---------------------------------------------------------------------------

/**
 * @tc.name      PrepareLowCodeCompleteFailsOnMissingIdentity
 * @tc.desc      Completing a sole low-code agent whose verification identity was never set is an error
 *               and does not mark it disconnecting.
 */
HWTEST_F(AgentConnectManagerTest, PrepareLowCodeCompleteFailsOnMissingIdentity, TestSize.Level1)
{
    auto &mgr = AgentConnectManager::GetInstance();
    auto conn = MakeConnection();
    auto host = DefaultHostKey();
    AgentConnectPlan plan;
    ASSERT_EQ(mgr.PrepareLowCodeConnectPlan(
        MakePlanRequest(conn, CALLER_UID, host, HOST_UID, "agent-1"), plan), ERR_OK);
    // Identity never stamped -> the missing-identity branch must reject before scheduling.
    LowCodeCompleteRequest request;
    EXPECT_EQ(mgr.PrepareLowCodeComplete("agent-1", CALLER_UID, request), ERR_INVALID_VALUE);
    EXPECT_EQ(request.hostConnection, nullptr);
    EXPECT_FALSE(mgr.agentHostSessions_[host]->agents["agent-1"].isDisconnecting);
}

/**
 * @tc.name      VerifyConnectStandardFailsOnEmptyIdentity
 * @tc.desc      A standard connection registered with an empty original identity yields ERR_INVALID_VALUE.
 */
HWTEST_F(AgentConnectManagerTest, VerifyConnectStandardFailsOnEmptyIdentity, TestSize.Level1)
{
    auto &mgr = AgentConnectManager::GetInstance();
    auto conn = MakeConnection();
    auto request = MakeStandardRequest(conn, CALLER_UID, "agent-1", DefaultHostKey(), NONCE_A, "");
    ASSERT_EQ(mgr.RegisterStandardAgentConnection(request), ERR_OK);

    std::string callerIdentity;
    EXPECT_EQ(mgr.VerifyAgentConnectRequest(request.serviceConnection->AsObject(), "agent-1",
        MakeVerifyWant(NONCE_A), callerIdentity), ERR_INVALID_VALUE);
    EXPECT_TRUE(callerIdentity.empty());
}

/**
 * @tc.name      PrepareAgentDisconnectResolvesByCallerUidFallback
 * @tc.desc      When the exact connection is not tracked, disconnect resolves a standard connection by
 *               callerUid and operates on the resolved record.
 */
HWTEST_F(AgentConnectManagerTest, PrepareAgentDisconnectResolvesByCallerUidFallback, TestSize.Level1)
{
    auto &mgr = AgentConnectManager::GetInstance();
    auto conn = MakeConnection();
    auto connect = MakeStandardRequest(conn, CALLER_UID, "agent-1", DefaultHostKey(), NONCE_A);
    ASSERT_EQ(mgr.RegisterStandardAgentConnection(connect), ERR_OK);
    auto other = MakeConnection();  // not tracked, but same callerUid

    AgentDisconnectRequest request;
    EXPECT_EQ(mgr.PrepareAgentDisconnectRequest(other, CALLER_UID, request), ERR_OK);
    EXPECT_EQ(request.callerRemote, conn->AsObject());
    EXPECT_TRUE(mgr.trackedConnections_[conn->AsObject()].isDisconnecting);
}

/**
 * @tc.name      HandleHostConnectDoneFailureKeepsConnectionWhenCallerHasOtherAgent
 * @tc.desc      A failed connect-done for one of a caller's agents removes only that agent; the tracked
 *               connection and host session are retained because the caller still owns another agent.
 */
HWTEST_F(AgentConnectManagerTest, HandleHostConnectDoneFailureKeepsConnectionWhenCallerHasOtherAgent, TestSize.Level1)
{
    auto &mgr = AgentConnectManager::GetInstance();
    auto conn = MakeConnection();
    auto host = DefaultHostKey();
    auto hostRemote = MakeConnection()->AsObject();
    AgentConnectPlan plan1;
    SetupConnectedLowCodeAgent(conn, host, "agent-1", NONCE_A, hostRemote, plan1);
    AgentConnectPlan plan2;
    ASSERT_EQ(mgr.PrepareLowCodeConnectPlan(
        MakePlanRequest(conn, CALLER_UID, host, HOST_UID, "agent-2"), plan2), ERR_OK);
    ASSERT_EQ(mgr.SetLowCodeConnectIdentity(host, "agent-2", "caller-identity", NONCE_B), ERR_OK);
    AgentHostConnectDoneRequest done;
    done.hostKey = host;
    done.callerRemote = conn->AsObject();
    done.agentId = "agent-2";
    done.remoteObject = hostRemote;
    done.resultCode = ERR_OK;
    ASSERT_EQ(mgr.HandleAgentHostConnectDone(done).callback->AsObject(), conn->AsObject());

    AgentHostConnectDoneRequest fail;
    fail.hostKey = host;
    fail.callerRemote = conn->AsObject();
    fail.agentId = "agent-1";
    fail.remoteObject = nullptr;
    fail.resultCode = ERR_INVALID_VALUE;
    auto result = mgr.HandleAgentHostConnectDone(fail);
    EXPECT_EQ(result.callback->AsObject(), conn->AsObject());
    EXPECT_FALSE(result.releaseConnectionOnFailure);
    EXPECT_FALSE(mgr.agentHostSessions_.empty());
    auto session = mgr.agentHostSessions_[host];
    EXPECT_EQ(session->agents.size(), 1u);
    EXPECT_EQ(session->agents.count("agent-1"), 0u);
    EXPECT_EQ(session->agents.count("agent-2"), 1u);
    EXPECT_EQ(mgr.agentOwners_.count({CALLER_UID, "agent-1"}), 0u);
    EXPECT_EQ(mgr.agentOwners_.count({CALLER_UID, "agent-2"}), 1u);
    EXPECT_EQ(mgr.trackedConnections_.size(), 1u);
    EXPECT_EQ(mgr.callerQuotas_[CALLER_UID][mgr.BuildLowCodeQuotaKey(host)], 1u);
}

// ---------------------------------------------------------------------------
// Coverage: ValidateLowCodePendingDisconnectBatchLocked error branches.
// AgentHostSession and LowCodePendingDisconnectRecord are public-member structs,
// so a connected low-code session's pendingDisconnects can be injected directly
// to exercise the batch-validation error paths reachable via VerifyAgentDisconnectRequests.
// ---------------------------------------------------------------------------

/**
 * @tc.name      VerifyDisconnectLowCodeFailsOnEmptyBatch
 * @tc.desc      An empty pending batch for a host remote yields a batch mismatch.
 */
HWTEST_F(AgentConnectManagerTest, VerifyDisconnectLowCodeFailsOnEmptyBatch, TestSize.Level1)
{
    auto &mgr = AgentConnectManager::GetInstance();
    auto conn = MakeConnection();
    auto host = DefaultHostKey();
    auto hostRemote = MakeConnection()->AsObject();
    AgentConnectPlan plan;
    SetupConnectedLowCodeAgent(conn, host, "agent-1", NONCE_A, hostRemote, plan);
    mgr.agentHostSessions_[host]->pendingDisconnects[plan.hostConnection->AsObject()] = {};

    std::string callerIdentity;
    EXPECT_EQ(mgr.VerifyAgentDisconnectRequests(plan.hostConnection->AsObject(), {}, callerIdentity),
        AAFwk::ERR_LOW_CODE_AGENT_DISCONNECT_BATCH_MISMATCH);
    EXPECT_TRUE(callerIdentity.empty());
}

/**
 * @tc.name      VerifyDisconnectLowCodeFailsOnInvalidPendingData
 * @tc.desc      A pending record with an empty agentId yields a batch mismatch.
 */
HWTEST_F(AgentConnectManagerTest, VerifyDisconnectLowCodeFailsOnInvalidPendingData, TestSize.Level1)
{
    auto &mgr = AgentConnectManager::GetInstance();
    auto conn = MakeConnection();
    auto host = DefaultHostKey();
    auto hostRemote = MakeConnection()->AsObject();
    AgentConnectPlan plan;
    SetupConnectedLowCodeAgent(conn, host, "agent-1", NONCE_A, hostRemote, plan);
    LowCodePendingDisconnectRecord rec;
    rec.agentId = "";
    rec.originalIdentity = "caller-identity";
    rec.verificationNonce = NONCE_A;
    rec.callerRemote = conn->AsObject();
    mgr.agentHostSessions_[host]->pendingDisconnects[plan.hostConnection->AsObject()] = { rec };

    std::string callerIdentity;
    EXPECT_EQ(mgr.VerifyAgentDisconnectRequests(plan.hostConnection->AsObject(), {}, callerIdentity),
        AAFwk::ERR_LOW_CODE_AGENT_DISCONNECT_BATCH_MISMATCH);
}

/**
 * @tc.name      VerifyDisconnectLowCodeFailsOnMissingAgent
 * @tc.desc      A pending record for an agent not in the session yields a batch mismatch.
 */
HWTEST_F(AgentConnectManagerTest, VerifyDisconnectLowCodeFailsOnMissingAgent, TestSize.Level1)
{
    auto &mgr = AgentConnectManager::GetInstance();
    auto conn = MakeConnection();
    auto host = DefaultHostKey();
    auto hostRemote = MakeConnection()->AsObject();
    AgentConnectPlan plan;
    SetupConnectedLowCodeAgent(conn, host, "agent-1", NONCE_A, hostRemote, plan);
    LowCodePendingDisconnectRecord rec;
    rec.agentId = "ghost";
    rec.originalIdentity = "caller-identity";
    rec.verificationNonce = NONCE_A;
    rec.callerRemote = conn->AsObject();
    mgr.agentHostSessions_[host]->pendingDisconnects[plan.hostConnection->AsObject()] = { rec };

    std::string callerIdentity;
    EXPECT_EQ(mgr.VerifyAgentDisconnectRequests(plan.hostConnection->AsObject(), {}, callerIdentity),
        AAFwk::ERR_LOW_CODE_AGENT_DISCONNECT_BATCH_MISMATCH);
}

/**
 * @tc.name      VerifyDisconnectLowCodeFailsOnStateMismatch
 * @tc.desc      A pending record for an agent that is not disconnecting yields a batch mismatch.
 */
HWTEST_F(AgentConnectManagerTest, VerifyDisconnectLowCodeFailsOnStateMismatch, TestSize.Level1)
{
    auto &mgr = AgentConnectManager::GetInstance();
    auto conn = MakeConnection();
    auto host = DefaultHostKey();
    auto hostRemote = MakeConnection()->AsObject();
    AgentConnectPlan plan;
    SetupConnectedLowCodeAgent(conn, host, "agent-1", NONCE_A, hostRemote, plan);
    LowCodePendingDisconnectRecord rec;
    rec.agentId = "agent-1";
    rec.originalIdentity = "caller-identity";
    rec.verificationNonce = NONCE_A;
    rec.callerRemote = conn->AsObject();
    mgr.agentHostSessions_[host]->pendingDisconnects[plan.hostConnection->AsObject()] = { rec };

    std::string callerIdentity;
    EXPECT_EQ(mgr.VerifyAgentDisconnectRequests(plan.hostConnection->AsObject(), {}, callerIdentity),
        AAFwk::ERR_LOW_CODE_AGENT_DISCONNECT_BATCH_MISMATCH);
}

/**
 * @tc.name      VerifyDisconnectLowCodeFailsOnNonceMismatch
 * @tc.desc      A pending record whose nonce differs from the agent record yields a batch mismatch.
 */
HWTEST_F(AgentConnectManagerTest, VerifyDisconnectLowCodeFailsOnNonceMismatch, TestSize.Level1)
{
    auto &mgr = AgentConnectManager::GetInstance();
    auto conn = MakeConnection();
    auto host = DefaultHostKey();
    auto hostRemote = MakeConnection()->AsObject();
    AgentConnectPlan plan;
    SetupConnectedLowCodeAgent(conn, host, "agent-1", NONCE_A, hostRemote, plan);
    AgentDisconnectRequest request;
    ASSERT_EQ(mgr.PrepareAgentDisconnectRequest(conn, CALLER_UID, request), ERR_OK);
    auto pendingRemote = plan.hostConnection->AsObject();
    mgr.agentHostSessions_[host]->pendingDisconnects[pendingRemote][0].verificationNonce = NONCE_B;

    std::string callerIdentity;
    EXPECT_EQ(mgr.VerifyAgentDisconnectRequests(pendingRemote, {}, callerIdentity),
        AAFwk::ERR_LOW_CODE_AGENT_DISCONNECT_BATCH_MISMATCH);
}

/**
 * @tc.name      VerifyDisconnectLowCodeFailsOnCallerMismatch
 * @tc.desc      A pending record with a null callerRemote yields a batch mismatch.
 */
HWTEST_F(AgentConnectManagerTest, VerifyDisconnectLowCodeFailsOnCallerMismatch, TestSize.Level1)
{
    auto &mgr = AgentConnectManager::GetInstance();
    auto conn = MakeConnection();
    auto host = DefaultHostKey();
    auto hostRemote = MakeConnection()->AsObject();
    AgentConnectPlan plan;
    SetupConnectedLowCodeAgent(conn, host, "agent-1", NONCE_A, hostRemote, plan);
    AgentDisconnectRequest request;
    ASSERT_EQ(mgr.PrepareAgentDisconnectRequest(conn, CALLER_UID, request), ERR_OK);
    auto pendingRemote = plan.hostConnection->AsObject();
    mgr.agentHostSessions_[host]->pendingDisconnects[pendingRemote][0].callerRemote = nullptr;

    std::string callerIdentity;
    EXPECT_EQ(mgr.VerifyAgentDisconnectRequests(pendingRemote, {}, callerIdentity),
        AAFwk::ERR_LOW_CODE_AGENT_DISCONNECT_BATCH_MISMATCH);
}

/**
 * @tc.name      VerifyDisconnectLowCodeFailsOnDuplicateAgent
 * @tc.desc      A pending batch with a duplicate agentId yields a batch mismatch.
 */
HWTEST_F(AgentConnectManagerTest, VerifyDisconnectLowCodeFailsOnDuplicateAgent, TestSize.Level1)
{
    auto &mgr = AgentConnectManager::GetInstance();
    auto conn = MakeConnection();
    auto host = DefaultHostKey();
    auto hostRemote = MakeConnection()->AsObject();
    AgentConnectPlan plan;
    SetupConnectedLowCodeAgent(conn, host, "agent-1", NONCE_A, hostRemote, plan);
    AgentDisconnectRequest request;
    ASSERT_EQ(mgr.PrepareAgentDisconnectRequest(conn, CALLER_UID, request), ERR_OK);
    auto pendingRemote = plan.hostConnection->AsObject();
    mgr.agentHostSessions_[host]->pendingDisconnects[pendingRemote].push_back(
        mgr.agentHostSessions_[host]->pendingDisconnects[pendingRemote][0]);

    std::string callerIdentity;
    EXPECT_EQ(mgr.VerifyAgentDisconnectRequests(pendingRemote, {}, callerIdentity),
        AAFwk::ERR_LOW_CODE_AGENT_DISCONNECT_BATCH_MISMATCH);
}

/**
 * @tc.name      VerifyDisconnectLowCodeFailsOnAgentSetMismatch
 * @tc.desc      When the pending agent set differs from the actually-disconnecting set, the batch mismatches.
 */
HWTEST_F(AgentConnectManagerTest, VerifyDisconnectLowCodeFailsOnAgentSetMismatch, TestSize.Level1)
{
    auto &mgr = AgentConnectManager::GetInstance();
    auto conn = MakeConnection();
    auto host = DefaultHostKey();
    auto hostRemote = MakeConnection()->AsObject();
    AgentConnectPlan plan1;
    SetupConnectedLowCodeAgent(conn, host, "agent-1", NONCE_A, hostRemote, plan1);
    AgentConnectPlan plan2;
    ASSERT_EQ(mgr.PrepareLowCodeConnectPlan(
        MakePlanRequest(conn, CALLER_UID, host, HOST_UID, "agent-2"), plan2), ERR_OK);
    ASSERT_EQ(mgr.SetLowCodeConnectIdentity(host, "agent-2", "caller-identity", NONCE_B), ERR_OK);
    AgentHostConnectDoneRequest done;
    done.hostKey = host;
    done.callerRemote = conn->AsObject();
    done.agentId = "agent-2";
    done.remoteObject = hostRemote;
    done.resultCode = ERR_OK;
    ASSERT_EQ(mgr.HandleAgentHostConnectDone(done).callback->AsObject(), conn->AsObject());
    AgentDisconnectRequest request;
    ASSERT_EQ(mgr.PrepareAgentDisconnectRequest(conn, CALLER_UID, request), ERR_OK);
    auto pendingRemote = plan1.hostConnection->AsObject();
    mgr.agentHostSessions_[host]->pendingDisconnects[pendingRemote].pop_back();

    std::string callerIdentity;
    EXPECT_EQ(mgr.VerifyAgentDisconnectRequests(pendingRemote, {}, callerIdentity),
        AAFwk::ERR_LOW_CODE_AGENT_DISCONNECT_BATCH_MISMATCH);
}

// ---------------------------------------------------------------------------
// Coverage: remaining SUT edge branches surfaced as gaps
// ---------------------------------------------------------------------------

/**
 * @tc.name      HandleHostConnectDoneForUnknownAgentIsNoop
 * @tc.desc      A connect-done for an agentId absent from a live session returns the callback, reports no
 *               release, and leaves the session intact.
 */
HWTEST_F(AgentConnectManagerTest, HandleHostConnectDoneForUnknownAgentIsNoop, TestSize.Level1)
{
    auto &mgr = AgentConnectManager::GetInstance();
    auto conn = MakeConnection();
    auto host = DefaultHostKey();
    auto hostRemote = MakeConnection()->AsObject();
    AgentConnectPlan plan;
    SetupConnectedLowCodeAgent(conn, host, "agent-1", NONCE_A, hostRemote, plan);

    AgentHostConnectDoneRequest req;
    req.hostKey = host;
    req.callerRemote = conn->AsObject();
    req.agentId = "ghost";
    req.remoteObject = hostRemote;
    req.resultCode = ERR_OK;
    auto result = mgr.HandleAgentHostConnectDone(req);
    EXPECT_EQ(result.callback->AsObject(), conn->AsObject());
    EXPECT_FALSE(result.releaseConnectionOnFailure);
    EXPECT_FALSE(mgr.agentHostSessions_.empty());
    EXPECT_EQ(mgr.agentHostSessions_[host]->agents.count("agent-1"), 1u);
}

/**
 * @tc.name      HandleHostDisconnectDoneWithNullRemoteUsesRequestAgentIds
 * @tc.desc      When hostConnectionRemote is null and no pending batch matches, disconnect-done falls back to
 *               the request's agentIds and tears down the agent + connection.
 */
HWTEST_F(AgentConnectManagerTest, HandleHostDisconnectDoneWithNullRemoteUsesRequestAgentIds, TestSize.Level1)
{
    auto &mgr = AgentConnectManager::GetInstance();
    auto conn = MakeConnection();
    auto host = DefaultHostKey();
    auto hostRemote = MakeConnection()->AsObject();
    AgentConnectPlan plan;
    SetupConnectedLowCodeAgent(conn, host, "agent-1", NONCE_A, hostRemote, plan);

    AgentHostDisconnectDoneRequest req;
    req.hostKey = host;
    req.callerRemote = conn->AsObject();
    req.hostConnectionRemote = nullptr;
    req.agentIds = { "agent-1" };
    req.resultCode = ERR_OK;
    auto result = mgr.HandleAgentHostDisconnectDone(req);
    ASSERT_EQ(result.callbacks.size(), 1u);
    EXPECT_EQ(result.callbacks[0]->AsObject(), conn->AsObject());
    EXPECT_TRUE(mgr.agentHostSessions_.empty());
    EXPECT_TRUE(mgr.trackedConnections_.empty());
    EXPECT_TRUE(mgr.agentOwners_.empty());
    EXPECT_TRUE(mgr.callerQuotas_.empty());
}

/**
 * @tc.name      HandleHostDisconnectDoneUnsolicitedTearsDownHostSession
 * @tc.desc      Unsolicited (empty queue): teardown -> reconnect, no stale agent (ERR_LOW_CODE_AGENT_ALREADY_ACTIVE).
 */
HWTEST_F(AgentConnectManagerTest, HandleHostDisconnectDoneUnsolicitedTearsDownHostSession, TestSize.Level1)
{
    auto &mgr = AgentConnectManager::GetInstance();
    auto conn = MakeConnection();
    auto host = DefaultHostKey();
    auto hostRemote = MakeConnection()->AsObject();
    AgentConnectPlan plan;
    SetupConnectedLowCodeAgent(conn, host, "agent-1", NONCE_A, hostRemote, plan);

    AgentHostDisconnectDoneRequest done;
    done.hostKey = host;
    done.callerRemote = conn->AsObject();
    done.hostConnectionRemote = plan.hostConnection->AsObject();
    done.resultCode = ERR_OK;
    auto result = mgr.HandleAgentHostDisconnectDone(done);
    ASSERT_EQ(result.callbacks.size(), 1u);
    EXPECT_EQ(result.callbacks[0]->AsObject(), conn->AsObject());
    EXPECT_TRUE(mgr.agentHostSessions_.empty());
    EXPECT_TRUE(mgr.trackedConnections_.empty());
    EXPECT_TRUE(mgr.agentOwners_.empty());
    EXPECT_TRUE(mgr.callerQuotas_.empty());
}

/**
 * @tc.name      HandleHostDisconnectDoneUnsolicitedReleasesQuotaForMultiAgentCaller
 * @tc.desc      Multi-agent caller quota released once (per-agent leaks); guards ReleaseLowCodeHostQuotasLocked.
 */
HWTEST_F(AgentConnectManagerTest, HandleHostDisconnectDoneUnsolicitedReleasesQuotaForMultiAgentCaller, TestSize.Level1)
{
    auto &mgr = AgentConnectManager::GetInstance();
    auto conn = MakeConnection();
    auto host = DefaultHostKey();
    auto hostRemote = MakeConnection()->AsObject();
    AgentConnectPlan planA;
    SetupConnectedLowCodeAgent(conn, host, "agent-1", NONCE_A, hostRemote, planA);
    AgentConnectPlan planB;
    SetupConnectedLowCodeAgent(conn, host, "agent-2", NONCE_B, hostRemote, planB);
    ASSERT_EQ(mgr.agentHostSessions_.size(), 1u);

    AgentHostDisconnectDoneRequest done;
    done.hostKey = host;
    done.callerRemote = conn->AsObject();
    done.hostConnectionRemote = planA.hostConnection->AsObject();
    done.resultCode = ERR_OK;
    auto result = mgr.HandleAgentHostDisconnectDone(done);
    ASSERT_EQ(result.callbacks.size(), 1u);
    EXPECT_EQ(result.callbacks[0]->AsObject(), conn->AsObject());
    EXPECT_TRUE(mgr.agentHostSessions_.empty());
    EXPECT_TRUE(mgr.trackedConnections_.empty());
    EXPECT_TRUE(mgr.agentOwners_.empty());
    EXPECT_TRUE(mgr.callerQuotas_.empty());
}

/**
 * @tc.name      HandleHostDisconnectDoneUnsolicitedSurfacesAllDistinctCallers
 * @tc.desc      Each caller surfaced + quota released once; guards ReleaseLowCodeHostQuotasLocked (dedupes callerUid).
 */
HWTEST_F(AgentConnectManagerTest, HandleHostDisconnectDoneUnsolicitedSurfacesAllDistinctCallers, TestSize.Level1)
{
    auto &mgr = AgentConnectManager::GetInstance();
    auto conn = MakeConnection();
    auto conn2 = MakeConnection();
    auto host = DefaultHostKey();
    auto hostRemote = MakeConnection()->AsObject();
    // Caller 1 (CALLER_UID): attaches agent-1, creates shared host session.
    AgentConnectPlan plan1;
    SetupConnectedLowCodeAgent(conn, host, "agent-1", NONCE_A, hostRemote, plan1);
    // Caller 2 (OTHER_CALLER_UID): reuses SAME host session, distinct remote/uid.
    AgentConnectPlan plan2;
    ASSERT_EQ(mgr.PrepareLowCodeConnectPlan(
        MakePlanRequest(conn2, OTHER_CALLER_UID, host, HOST_UID, "agent-2"), plan2), ERR_OK);
    ASSERT_EQ(mgr.SetLowCodeConnectIdentity(host, "agent-2", "caller-identity", NONCE_B), ERR_OK);
    AgentHostConnectDoneRequest connect2;
    connect2.hostKey = host;
    connect2.callerRemote = conn2->AsObject();
    connect2.agentId = "agent-2";
    connect2.remoteObject = hostRemote;
    connect2.resultCode = ERR_OK;
    ASSERT_EQ(mgr.HandleAgentHostConnectDone(connect2).callback->AsObject(), conn2->AsObject());
    ASSERT_EQ(mgr.agentHostSessions_.size(), 1u);
    ASSERT_EQ(mgr.agentHostSessions_[host]->callerConnections.size(), 2u);
    ASSERT_EQ(mgr.callerQuotas_.size(), 2u);

    // Unsolicited host break: non-null hostConnectionRemote, no pending-disconnect queue.
    AgentHostDisconnectDoneRequest done;
    done.hostKey = host;
    done.callerRemote = conn->AsObject();
    done.hostConnectionRemote = plan1.hostConnection->AsObject();
    done.resultCode = ERR_OK;
    auto result = mgr.HandleAgentHostDisconnectDone(done);
    // Each distinct caller surfaced once; remotes are pointer-keyed so order is unstable across runs —
    // check membership, not positional indexing.
    ASSERT_EQ(result.callbacks.size(), 2u);
    bool foundCaller1 = false;
    bool foundCaller2 = false;
    for (const auto &callback : result.callbacks) {
        if (callback->AsObject() == conn->AsObject()) {
            foundCaller1 = true;
        }
        if (callback->AsObject() == conn2->AsObject()) {
            foundCaller2 = true;
        }
    }
    EXPECT_TRUE(foundCaller1);
    EXPECT_TRUE(foundCaller2);
    // Each caller quota released once, no stale owner/connection/session -> reconnect avoids
    // ERR_LOW_CODE_AGENT_ALREADY_ACTIVE.
    EXPECT_TRUE(mgr.agentHostSessions_.empty());
    EXPECT_TRUE(mgr.trackedConnections_.empty());
    EXPECT_TRUE(mgr.agentOwners_.empty());
    EXPECT_TRUE(mgr.callerQuotas_.empty());
}

/**
 * @tc.name      HandleHostDisconnectDoneUnsolicitedIgnoresNullCallerRemote
 * @tc.desc      TearDownHostSessionLocked ignores callerRemote: nullptr + non-null hostConnectionRemote tears down.
 */
HWTEST_F(AgentConnectManagerTest, HandleHostDisconnectDoneUnsolicitedIgnoresNullCallerRemote, TestSize.Level1)
{
    auto &mgr = AgentConnectManager::GetInstance();
    auto conn = MakeConnection();
    auto host = DefaultHostKey();
    auto hostRemote = MakeConnection()->AsObject();
    AgentConnectPlan plan;
    SetupConnectedLowCodeAgent(conn, host, "agent-1", NONCE_A, hostRemote, plan);

    AgentHostDisconnectDoneRequest done;
    done.hostKey = host;
    done.callerRemote = nullptr;
    done.hostConnectionRemote = plan.hostConnection->AsObject();
    done.resultCode = ERR_OK;
    auto result = mgr.HandleAgentHostDisconnectDone(done);
    ASSERT_EQ(result.callbacks.size(), 1u);
    EXPECT_EQ(result.callbacks[0]->AsObject(), conn->AsObject());
    EXPECT_TRUE(mgr.agentHostSessions_.empty());
    EXPECT_TRUE(mgr.trackedConnections_.empty());
    EXPECT_TRUE(mgr.agentOwners_.empty());
    EXPECT_TRUE(mgr.callerQuotas_.empty());
}

/**
 * @tc.name      TearDownHostSessionLockedReturnsEmptyForUnknownHostKey
 * @tc.desc      Defensive: unknown hostKey with no session returns empty callbacks, leaves state untouched.
 */
HWTEST_F(AgentConnectManagerTest, TearDownHostSessionLockedReturnsEmptyForUnknownHostKey, TestSize.Level1)
{
    auto &mgr = AgentConnectManager::GetInstance();
    auto stale = MakeHostKey(CALLER_USER_ID, "com.ghost", "ghost.module", "GhostAbility");
    auto callbacks = mgr.TearDownHostSessionLocked(stale);
    EXPECT_TRUE(callbacks.empty());
    EXPECT_TRUE(mgr.agentHostSessions_.empty());
    EXPECT_TRUE(mgr.trackedConnections_.empty());
    EXPECT_TRUE(mgr.agentOwners_.empty());
    EXPECT_TRUE(mgr.callerQuotas_.empty());
}

/**
 * @tc.name      TearDownHostSessionLockedErasesNullSession
 * @tc.desc      Erases null session entry, returns empty callbacks; near-unreachable in HandleAgentHostDisconnectDone.
 */
HWTEST_F(AgentConnectManagerTest, TearDownHostSessionLockedErasesNullSession, TestSize.Level1)
{
    auto &mgr = AgentConnectManager::GetInstance();
    auto host = DefaultHostKey();
    // Inject a null session entry directly (private->public access); nothing else populated.
    mgr.agentHostSessions_[host] = nullptr;
    ASSERT_EQ(mgr.agentHostSessions_.count(host), 1u);
    auto callbacks = mgr.TearDownHostSessionLocked(host);
    EXPECT_TRUE(callbacks.empty());
    EXPECT_EQ(mgr.agentHostSessions_.count(host), 0u);
}

/**
 * @tc.name      ReconnectAfterUnsolicitedTeardownDoesNotSeeStaleAgent
 * @tc.desc      Unsolicited teardown: reconnect SAME agentId/SAME host succeeds, no ERR_LOW_CODE_AGENT_ALREADY_ACTIVE.
 */
HWTEST_F(AgentConnectManagerTest, ReconnectAfterUnsolicitedTeardownDoesNotSeeStaleAgent, TestSize.Level1)
{
    auto &mgr = AgentConnectManager::GetInstance();
    auto conn = MakeConnection();
    auto host = DefaultHostKey();
    auto hostRemote = MakeConnection()->AsObject();
    AgentConnectPlan plan;
    SetupConnectedLowCodeAgent(conn, host, "agent-1", NONCE_A, hostRemote, plan);
    ASSERT_EQ(mgr.agentHostSessions_.size(), 1u);
    ASSERT_EQ(mgr.agentOwners_.size(), 1u);

    // Unsolicited host break: tear down session, clear all residue.
    AgentHostDisconnectDoneRequest done;
    done.hostKey = host;
    done.callerRemote = conn->AsObject();
    done.hostConnectionRemote = plan.hostConnection->AsObject();
    done.resultCode = ERR_OK;
    auto result = mgr.HandleAgentHostDisconnectDone(done);
    ASSERT_EQ(result.callbacks.size(), 1u);
    EXPECT_TRUE(mgr.agentHostSessions_.empty());
    EXPECT_TRUE(mgr.agentOwners_.empty());
    EXPECT_TRUE(mgr.callerQuotas_.empty());

    // Reconnect SAME agentId on SAME host with fresh caller: must succeed (helper asserts ERR_OK each step ->
    // end reached = no stale-agent residue).
    auto reconnect = MakeConnection();
    AgentConnectPlan reconnectPlan;
    SetupConnectedLowCodeAgent(reconnect, host, "agent-1", NONCE_B, hostRemote, reconnectPlan);
    EXPECT_EQ(mgr.agentHostSessions_.size(), 1u);
    EXPECT_EQ(mgr.agentHostSessions_[host]->agents.count("agent-1"), 1u);
    EXPECT_EQ(mgr.agentOwners_.size(), 1u);
    EXPECT_EQ(mgr.callerQuotas_.size(), 1u);
    EXPECT_EQ(mgr.trackedConnections_.size(), 1u);
}

/**
 * @tc.name      PrepareNextLowCodeDisconnectSkipsOriginMismatch
 * @tc.desc      Within one host-remote batch, a pending record whose cleanupOnFailure flag differs from the
 *               first is skipped, so PrepareNext returns only the consistently-flagged agents.
 */
HWTEST_F(AgentConnectManagerTest, PrepareNextLowCodeDisconnectSkipsOriginMismatch, TestSize.Level1)
{
    auto &mgr = AgentConnectManager::GetInstance();
    auto conn = MakeConnection();
    auto host = DefaultHostKey();
    auto hostRemote = MakeConnection()->AsObject();
    AgentConnectPlan plan1;
    SetupConnectedLowCodeAgent(conn, host, "agent-1", NONCE_A, hostRemote, plan1);
    AgentConnectPlan plan2;
    ASSERT_EQ(mgr.PrepareLowCodeConnectPlan(
        MakePlanRequest(conn, CALLER_UID, host, HOST_UID, "agent-2"), plan2), ERR_OK);
    ASSERT_EQ(mgr.SetLowCodeConnectIdentity(host, "agent-2", "caller-identity", NONCE_B), ERR_OK);
    AgentHostConnectDoneRequest done;
    done.hostKey = host;
    done.callerRemote = conn->AsObject();
    done.agentId = "agent-2";
    done.remoteObject = hostRemote;
    done.resultCode = ERR_OK;
    ASSERT_EQ(mgr.HandleAgentHostConnectDone(done).callback->AsObject(), conn->AsObject());
    AgentDisconnectRequest request;
    ASSERT_EQ(mgr.PrepareAgentDisconnectRequest(conn, CALLER_UID, request), ERR_OK);
    auto pendingRemote = plan1.hostConnection->AsObject();
    mgr.agentHostSessions_[host]->pendingDisconnects[pendingRemote][1].cleanupOnFailure = true;

    LowCodeDisconnectTarget target;
    sptr<IRemoteObject> callerRemote;
    EXPECT_TRUE(mgr.PrepareNextLowCodeDisconnectRequest(host, target, callerRemote));
    EXPECT_EQ(target.agentIds, (std::set<std::string> { "agent-1" }));
    EXPECT_EQ(target.hostConnection, plan1.hostConnection);
    EXPECT_EQ(callerRemote, conn->AsObject());
}

/**
 * @tc.name      ReleaseCallerQuotaDecrementsCountForSharedQuotaKey
 * @tc.desc      Two standard connections sharing one quota key raise the count to 2; releasing one decrements
 *               to 1 without erasing the entry.
 */
HWTEST_F(AgentConnectManagerTest, ReleaseCallerQuotaDecrementsCountForSharedQuotaKey, TestSize.Level1)
{
    auto &mgr = AgentConnectManager::GetInstance();
    auto conn1 = MakeConnection();
    auto conn2 = MakeConnection();
    auto host = DefaultHostKey();
    auto req1 = MakeStandardRequest(conn1, CALLER_UID, "agent-1", host, NONCE_A);
    auto req2 = MakeStandardRequest(conn2, CALLER_UID, "agent-1", host, NONCE_A);
    ASSERT_EQ(mgr.RegisterStandardAgentConnection(req1), ERR_OK);
    ASSERT_EQ(mgr.RegisterStandardAgentConnection(req2), ERR_OK);
    EXPECT_EQ(mgr.callerQuotas_[CALLER_UID][req1.quotaKey], 2u);

    mgr.ReleaseTrackedConnection(conn1);
    EXPECT_EQ(mgr.callerQuotas_[CALLER_UID][req1.quotaKey], 1u);
    EXPECT_EQ(mgr.callerQuotas_[CALLER_UID].size(), 1u);
    EXPECT_EQ(mgr.trackedConnections_.size(), 1u);
}

/**
 * @tc.name      RegisterTrackedConnectionRegistersDeathRecipient
 * @tc.desc      Registering a tracked connection attaches a death recipient; releasing it removes the record.
 */
HWTEST_F(AgentConnectManagerTest, RegisterTrackedConnectionRegistersDeathRecipient, TestSize.Level1)
{
    auto &mgr = AgentConnectManager::GetInstance();
    auto conn = MakeConnection();
    sptr<AAFwk::IAbilityConnection> serviceConnection;
    ASSERT_EQ(mgr.RegisterTrackedConnectionAndGetServiceConnection(
        conn, CALLER_UID, NoopDeathHandler(), serviceConnection), ERR_OK);
    EXPECT_NE(mgr.trackedConnections_[conn->AsObject()].deathRecipient, nullptr);

    mgr.ReleaseTrackedConnection(conn);
    EXPECT_TRUE(mgr.trackedConnections_.empty());
}

/**
 * @tc.name      ExpiredPreflightIsPrunedOnConsume
 * @tc.desc      A preflight whose expiry is in the past is pruned during consume, so it does not match.
 */
HWTEST_F(AgentConnectManagerTest, ExpiredPreflightIsPrunedOnConsume, TestSize.Level1)
{
    auto &mgr = AgentConnectManager::GetInstance();
    AgentConnectPreflightRegisterRequest request;
    request.connectWant = MakePreflightWant("agent-1", "com.host", "HostAbility");
    request.agentId = "agent-1";
    request.callerUid = CALLER_UID;
    request.callerUserId = CALLER_USER_ID;
    auto reg = mgr.RegisterConnectPreflight(request, FixedNonceFunc(NONCE_A));
    ASSERT_GT(reg.nonce, 0);
    mgr.connectPreflights_[reg.nonce].expiresAt = AgentPreflightClock::now() - std::chrono::seconds(1);

    AgentConnectPreflightConsumeRequest consume;
    consume.want = reg.connectWant;
    consume.callerUid = CALLER_UID;
    consume.callerUserId = CALLER_USER_ID;
    auto result = mgr.TryConsumeConnectPreflight(consume);
    EXPECT_FALSE(result.matched);
    EXPECT_TRUE(mgr.connectPreflights_.empty());
}

/**
 * @tc.name      CleanupExpiredPreflightsPrunesExpiredRecord
 * @tc.desc      Cleanup with the matching scheduledAt prunes an expired record and, finding none left,
 *               reports nothing to reschedule.
 */
HWTEST_F(AgentConnectManagerTest, CleanupExpiredPreflightsPrunesExpiredRecord, TestSize.Level1)
{
    auto &mgr = AgentConnectManager::GetInstance();
    AgentConnectPreflightRegisterRequest request;
    request.connectWant = MakePreflightWant("agent-1", "com.host", "HostAbility");
    request.agentId = "agent-1";
    request.callerUid = CALLER_UID;
    request.callerUserId = CALLER_USER_ID;
    auto reg = mgr.RegisterConnectPreflight(request, FixedNonceFunc(NONCE_A));
    ASSERT_GT(reg.nonce, 0);
    ASSERT_TRUE(mgr.connectPreflightCleanupScheduled_);
    mgr.connectPreflights_[reg.nonce].expiresAt = AgentPreflightClock::now() - std::chrono::seconds(1);

    AgentPreflightTimePoint nextAt;
    auto ret = mgr.CleanupExpiredConnectPreflights(reg.cleanupAt, nextAt);
    EXPECT_FALSE(ret);
    EXPECT_TRUE(mgr.connectPreflights_.empty());
    EXPECT_FALSE(mgr.connectPreflightCleanupScheduled_);
}

/**
 * @tc.name      ClearAgentHostSessionReleasesAllCallers
 * @tc.desc      The (currently caller-less) ClearAgentHostSessionLocked releases every caller's tracked
 *               connection, quotas and owners, then erases the session.
 */
HWTEST_F(AgentConnectManagerTest, ClearAgentHostSessionReleasesAllCallers, TestSize.Level1)
{
    auto &mgr = AgentConnectManager::GetInstance();
    auto conn = MakeConnection();
    auto conn2 = MakeConnection();
    auto host = DefaultHostKey();
    auto hostRemote = MakeConnection()->AsObject();
    AgentConnectPlan plan1;
    SetupConnectedLowCodeAgent(conn, host, "agent-1", NONCE_A, hostRemote, plan1);
    AgentConnectPlan plan2;
    ASSERT_EQ(mgr.PrepareLowCodeConnectPlan(
        MakePlanRequest(conn2, OTHER_CALLER_UID, host, HOST_UID, "agent-2"), plan2), ERR_OK);
    ASSERT_EQ(mgr.SetLowCodeConnectIdentity(host, "agent-2", "caller-identity", NONCE_B), ERR_OK);
    AgentHostConnectDoneRequest done;
    done.hostKey = host;
    done.callerRemote = conn2->AsObject();
    done.agentId = "agent-2";
    done.remoteObject = hostRemote;
    done.resultCode = ERR_OK;
    ASSERT_EQ(mgr.HandleAgentHostConnectDone(done).callback->AsObject(), conn2->AsObject());
    ASSERT_EQ(mgr.trackedConnections_.size(), 2u);
    ASSERT_FALSE(mgr.callerQuotas_.empty());

    mgr.ClearAgentHostSessionLocked(host);
    EXPECT_TRUE(mgr.agentHostSessions_.empty());
    EXPECT_TRUE(mgr.agentOwners_.empty());
    EXPECT_TRUE(mgr.trackedConnections_.empty());
    EXPECT_TRUE(mgr.callerQuotas_.empty());
}
}  // namespace AgentRuntime
}  // namespace OHOS
