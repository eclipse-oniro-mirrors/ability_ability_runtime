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
#include <gmock/gmock.h>
#define private public
#define protected public
#include "agent_card_mgr.h"
#include "mock_my_flag.h"
#undef private
#undef protected

using namespace OHOS;
using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AgentRuntime {
class AgentCardMgrTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp() override;
    void TearDown() override;
};

void AgentCardMgrTest::SetUpTestCase(void)
{}

void AgentCardMgrTest::TearDownTestCase(void)
{}

void AgentCardMgrTest::SetUp(void)
{
    MyFlag::retInsertData = 0;
    MyFlag::retDeleteData = 0;
    MyFlag::retQueryData = 0;
    MyFlag::retQueryAllData = 0;
    MyFlag::queryDataCards.clear();
    MyFlag::queryAllDataCards.clear();
    MyFlag::retGetBundleInfo = true;
    MyFlag::retGetResConfigFile = true;
    MyFlag::retFromJson = true;
}

void AgentCardMgrTest::TearDown(void)
{}

/**
 * @tc.name: HandleBundleRemoveTest_001
 * @tc.desc: HandleBundleRemoveTest_001
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardMgrTest, HandleBundleRemoveTest_001, TestSize.Level1)
{
    AgentCardMgr agentCardMgr;
    int ret = agentCardMgr.HandleBundleRemove("", 100);
    EXPECT_TRUE(ret == -1);

    MyFlag::retDeleteData = 0;
    ret = agentCardMgr.HandleBundleRemove("test", 100);
    EXPECT_TRUE(ret == 0);
}

/**
 * @tc.name: HandleBundleInstallTest_001
 * @tc.desc: HandleBundleInstall with empty bundleName
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardMgrTest, HandleBundleInstallTest_001, TestSize.Level1)
{
    AgentCardMgr agentCardMgr;
    int ret = agentCardMgr.HandleBundleInstall("", 100);
    EXPECT_TRUE(ret == -1);
}

/**
 * @tc.name: HandleBundleInstallTest_002
 * @tc.desc: HandleBundleInstall when GetBundleInfo fails
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardMgrTest, HandleBundleInstallTest_002, TestSize.Level1)
{
    AgentCardMgr agentCardMgr;
    MyFlag::retGetBundleInfo = false;
    int ret = agentCardMgr.HandleBundleInstall("test", 100);
    EXPECT_TRUE(ret == -1);
}

/**
 * @tc.name: HandleBundleInstallTest_003
 * @tc.desc: HandleBundleInstall success
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardMgrTest, HandleBundleInstallTest_003, TestSize.Level1)
{
    AgentCardMgr agentCardMgr;
    MyFlag::retInsertData = 0;
    int ret = agentCardMgr.HandleBundleInstall("test", 100);
    EXPECT_TRUE(ret == 0);
}

/**
 * @tc.name: HandleBundleUpdateTest_001
 * @tc.desc: HandleBundleUpdate
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardMgrTest, HandleBundleUpdateTest_001, TestSize.Level1)
{
    AgentCardMgr agentCardMgr;
    MyFlag::retInsertData = 0;
    int ret = agentCardMgr.HandleBundleUpdate("test", 100);
    EXPECT_TRUE(ret == 0);
}

/**
 * @tc.name: GetAllAgentCardsTest_001
 * @tc.desc: GetAllAgentCards success
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardMgrTest, GetAllAgentCardsTest_001, TestSize.Level1)
{
    AgentCardMgr agentCardMgr;
    MyFlag::retQueryAllData = 0;
    AgentCardsRawData cards;
    int ret = agentCardMgr.GetAllAgentCards(cards);
    EXPECT_TRUE(ret == 0);
}

/**
 * @tc.name: GetAgentCardsByBundleNameTest_001
 * @tc.desc: GetAgentCardsByBundleName success
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardMgrTest, GetAgentCardsByBundleNameTest_001, TestSize.Level1)
{
    AgentCardMgr agentCardMgr;
    MyFlag::retQueryData = 0;
    std::vector<AgentCard> cards;
    int ret = agentCardMgr.GetAgentCardsByBundleName("test", cards);
    EXPECT_TRUE(ret == 0);
}

/**
 * @tc.name: GetAgentCardByAgentId_001
 * @tc.desc: GetAgentCardByAgentId when QueryData fails
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardMgrTest, GetAgentCardByAgentId_001, TestSize.Level1)
{
    AgentCardMgr agentCardMgr;
    MyFlag::retQueryData = -1;
    AgentCard card;
    int ret = agentCardMgr.GetAgentCardByAgentId("test", "test", card);
    EXPECT_TRUE(ret != 0);
}

/**
 * @tc.name: GetAgentCardByAgentId_002
 * @tc.desc: GetAgentCardByAgentId when agentId not found
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardMgrTest, GetAgentCardByAgentId_002, TestSize.Level1)
{
    AgentCardMgr agentCardMgr;
    MyFlag::retQueryData = 0;
    MyFlag::queryDataCards.clear();
    AgentCard card;
    int ret = agentCardMgr.GetAgentCardByAgentId("test", "test", card);
    EXPECT_TRUE(ret == ERR_NAME_NOT_FOUND);
}

/**
 * @tc.name: GetAgentCardByAgentId_003
 * @tc.desc: GetAgentCardByAgentId success
 * @tc.type:: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardMgrTest, GetAgentCardByAgentId_003, TestSize.Level1)
{
    AgentCardMgr agentCardMgr;
    MyFlag::retQueryData = 0;
    AgentCard testCard;
    testCard.agentId = "test";
    MyFlag::queryDataCards.push_back(testCard);
    AgentCard card;
    int ret = agentCardMgr.GetAgentCardByAgentId("test", "test", card);
    EXPECT_TRUE(ret == 0);
    EXPECT_TRUE(card.agentId == "test");
}
} // namespace AgentRuntime
} // namespace OHOS