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

#include <set>
#include <limits>
#include <string>

#include <gtest/gtest.h>

#include "agent_connect_manager_types.h"
#include "want.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AgentRuntime;

namespace {
AgentQuotaKey MakeKey(int32_t userId, const std::string &agentId, const std::string &bundleName,
    int32_t appIndex, bool isLowCode)
{
    AgentQuotaKey key;
    key.hostKey.userId = userId;
    key.hostKey.appIndex = appIndex;
    key.hostKey.bundleName = bundleName;
    key.hostKey.moduleName = "module";
    key.hostKey.abilityName = "ability";
    key.agentId = agentId;
    key.isLowCode = isLowCode;
    return key;
}
}  // namespace

class AgentConnectManagerTypesTest : public testing::Test {
public:
    void SetUp() override {}
    void TearDown() override {}
};

/**
 * @tc.name      EqualKeysAreNotOrdered
 * @tc.desc      Two structurally equal keys must not be ordered relative to each other.
 */
HWTEST_F(AgentConnectManagerTypesTest, EqualKeysAreNotOrdered, TestSize.Level1)
{
    auto lhs = MakeKey(100, "agent-1", "com.host", 0, false);
    auto rhs = MakeKey(100, "agent-1", "com.host", 0, false);
    EXPECT_FALSE(lhs < rhs);
    EXPECT_FALSE(rhs < lhs);
}

/**
 * @tc.name      LowCodeOrdersBeforeStandard
 * @tc.desc      operator< orders by isLowCode first; false (standard) sorts before true (low-code).
 */
HWTEST_F(AgentConnectManagerTypesTest, LowCodeOrdersAfterStandard, TestSize.Level1)
{
    auto standard = MakeKey(100, "agent-1", "com.host", 0, false);
    auto lowCode = MakeKey(100, "agent-1", "com.host", 0, true);
    EXPECT_TRUE(standard < lowCode) << "standard (isLowCode=false) must sort before low-code";
    EXPECT_FALSE(lowCode < standard);
}

/**
 * @tc.name      HostKeyDifferenceDrivesOrdering
 * @tc.desc      When isLowCode ties, operator< delegates to AgentHostKey::operator<.
 */
HWTEST_F(AgentConnectManagerTypesTest, HostKeyDifferenceDrivesOrdering, TestSize.Level1)
{
    auto smallerHost = MakeKey(50, "agent-1", "com.host.a", 0, false);
    auto largerHost = MakeKey(200, "agent-1", "com.host.b", 0, false);
    EXPECT_TRUE(smallerHost < largerHost);
    EXPECT_FALSE(largerHost < smallerHost);
}

/**
 * @tc.name      AgentIdBreaksHostTie
 * @tc.desc      When isLowCode and hostKey tie, operator< delegates to agentId string compare.
 */
HWTEST_F(AgentConnectManagerTypesTest, AgentIdBreaksHostTie, TestSize.Level1)
{
    auto lhs = MakeKey(100, "agent-1", "com.host", 0, false);
    auto rhs = MakeKey(100, "agent-2", "com.host", 0, false);
    EXPECT_TRUE(lhs < rhs);
    EXPECT_FALSE(rhs < lhs);
}

/**
 * @tc.name      AgentIdBreaksHostTieLowCode
 * @tc.desc      agentId also breaks ties on the low-code branch.
 */
HWTEST_F(AgentConnectManagerTypesTest, AgentIdBreaksHostTieLowCode, TestSize.Level1)
{
    auto lhs = MakeKey(100, "agent-1", "com.host", 0, true);
    auto rhs = MakeKey(100, "agent-10", "com.host", 0, true);
    EXPECT_TRUE(lhs < rhs);
    EXPECT_FALSE(rhs < lhs);
}

/**
 * @tc.name      StandardAndLowCodeWithSameAgentIdAreDistinct
 * @tc.desc      A standard key and a low-code key with the same agentId must be distinct
 *               (the low-code one sorts after the standard one).
 */
HWTEST_F(AgentConnectManagerTypesTest, StandardAndLowCodeWithSameAgentIdAreDistinct, TestSize.Level1)
{
    auto standard = MakeKey(100, "shared-id", "com.host", 0, false);
    auto lowCode = MakeKey(100, "shared-id", "com.host", 0, true);
    EXPECT_TRUE(standard < lowCode);
    EXPECT_FALSE(lowCode < standard);
}

/**
 * @tc.name      KeysAreUsableInOrderedSet
 * @tc.desc      AgentQuotaKey must be usable as a std::set key (smoke test for ordering strict-weak-ordering).
 */
HWTEST_F(AgentConnectManagerTypesTest, KeysAreUsableInOrderedSet, TestSize.Level1)
{
    std::set<AgentQuotaKey> keys;
    keys.insert(MakeKey(100, "agent-2", "com.host", 0, false));
    keys.insert(MakeKey(100, "agent-1", "com.host", 0, false));
    keys.insert(MakeKey(50, "agent-1", "com.host", 0, false));
    keys.insert(MakeKey(100, "agent-1", "com.host", 0, true));
    // 3 distinct standard keys + 1 low-code key => 4 entries.
    EXPECT_EQ(keys.size(), 4u);
}

/**
 * @tc.name      VerificationNonceParamRoundTripsFixedWidth
 * @tc.desc      Agent verification nonce helpers must preserve fixed-width int64 values in Want params.
 */
HWTEST_F(AgentConnectManagerTypesTest, VerificationNonceParamRoundTripsFixedWidth, TestSize.Level1)
{
    AAFwk::Want want;
    constexpr int64_t nonce = std::numeric_limits<int64_t>::max() - 17;
    SetAgentVerificationNonceParam(want, nonce);
    EXPECT_EQ(GetAgentVerificationNonceParam(want), nonce);
}
