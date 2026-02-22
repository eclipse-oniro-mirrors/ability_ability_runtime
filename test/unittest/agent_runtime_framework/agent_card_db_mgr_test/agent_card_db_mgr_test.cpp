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
#include "agent_card_db_mgr.h"
#undef private
#undef protected

using namespace OHOS;
using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AgentRuntime {
class AgentCardDbMgrTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp() override;
    void TearDown() override;
};

void AgentCardDbMgrTest::SetUpTestCase(void)
{}

void AgentCardDbMgrTest::TearDownTestCase(void)
{}

void AgentCardDbMgrTest::SetUp(void)
{}

void AgentCardDbMgrTest::TearDown(void)
{}

/**
 * @tc.name: RestoreKvStoreTest_001
 * @tc.desc: RestoreKvStoreTest_001
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardDbMgrTest, RestoreKvStoreTest_001, TestSize.Level1)
{
    AgentCardDbMgr agentCardDbMgr;
    DistributedKv::Status ret = agentCardDbMgr.RestoreKvStore(DistributedKv::Status::DATA_CORRUPTED);
    EXPECT_TRUE(ret != DistributedKv::Status::DATA_CORRUPTED);

    ret = agentCardDbMgr.RestoreKvStore(DistributedKv::Status::SUCCESS);
    EXPECT_TRUE(ret == DistributedKv::Status::SUCCESS);
}

/**
 * @tc.name: GetKvStoreTest_001
 * @tc.desc: GetKvStoreTest_001
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardDbMgrTest, GetKvStoreTest_001, TestSize.Level1)
{
    AgentCardDbMgr agentCardDbMgr;
    DistributedKv::Status ret = agentCardDbMgr.GetKvStore();
    EXPECT_TRUE(ret == DistributedKv::Status::SUCCESS);
}

/**
 * @tc.name: CheckKvStoreTest_001
 * @tc.desc: CheckKvStoreTest_001
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardDbMgrTest, CheckKvStoreTest_001, TestSize.Level1)
{
    AgentCardDbMgr agentCardDbMgr;
    bool ret = agentCardDbMgr.CheckKvStore();
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: InsertDataTest_001
 * @tc.desc: InsertDataTest_001
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardDbMgrTest, InsertDataTest_001, TestSize.Level1)
{
    std::vector<AgentCard> cards;
    AgentCardDbMgr agentCardDbMgr;
    int ret = agentCardDbMgr.InsertData("test", 100, cards);
    EXPECT_TRUE(ret == ERR_OK);
}

/**
 * @tc.name: DeleteDataTest_001
 * @tc.desc: DeleteDataTest_001
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardDbMgrTest, DeleteDataTest_001, TestSize.Level1)
{
    AgentCardDbMgr agentCardDbMgr;
    int ret = agentCardDbMgr.DeleteData("test", 100);
    EXPECT_TRUE(ret == ERR_OK);
}

/**
 * @tc.name: QueryDataTest_001
 * @tc.desc: QueryDataTest_001
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardDbMgrTest, QueryDataTest_001, TestSize.Level1)
{
    std::vector<AgentCard> cards;
    AgentCardDbMgr agentCardDbMgr;
    int ret = agentCardDbMgr.QueryData("test", 100, cards);
    EXPECT_TRUE(ret != ERR_OK);
}

/**
 * @tc.name: QueryAllDataTest_001
 * @tc.desc: Test QueryAllData with valid data - should return ERR_OK with cards
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardDbMgrTest, QueryAllDataTest_001, TestSize.Level1)
{
    // Arrange: Prepare test data
    std::vector<AgentCard> insertCards;
    AgentCard card1;
    card1.agentId = "agent_001";
    card1.name = "Test Agent 1";
    card1.description = "Test agent 1 description";
    card1.version = "1.0.0";
    insertCards.push_back(card1);

    // Act: Insert data and then query all
    AgentCardDbMgr agentCardDbMgr;
    int insertRet = agentCardDbMgr.InsertData("com.test.agent1", 100, insertCards);
    EXPECT_TRUE(insertRet == ERR_OK);

    std::vector<AgentCard> queryCards;
    int queryRet = agentCardDbMgr.QueryAllData(queryCards);

    // Assert: Verify the query result
    EXPECT_TRUE(queryRet == ERR_OK);
    EXPECT_TRUE(queryCards.size() >= 1);
}

/**
 * @tc.name: QueryAllDataTest_002
 * @tc.desc: Test QueryAllData with empty database - should return ERR_OK with empty cards
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardDbMgrTest, QueryAllDataTest_002, TestSize.Level1)
{
    // Arrange: Clear database by deleting test data
    AgentCardDbMgr agentCardDbMgr;
    agentCardDbMgr.DeleteData("com.test.queryall", 100);

    // Act: Query all data from empty database
    std::vector<AgentCard> cards;
    int ret = agentCardDbMgr.QueryAllData(cards);

    // Assert: Should return success with empty cards (no error thrown)
    EXPECT_TRUE(ret == ERR_OK);
}

/**
 * @tc.name: QueryAllDataTest_003
 * @tc.desc: Test QueryAllData after inserting multiple cards - should return all cards
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardDbMgrTest, QueryAllDataTest_003, TestSize.Level1)
{
    // Arrange: Insert multiple cards with different bundle names and users
    AgentCardDbMgr agentCardDbMgr;

    std::vector<AgentCard> cards1;
    AgentCard card1;
    card1.agentId = "agent_multi_001";
    card1.name = "Multi Test Agent 1";
    card1.version = "1.0.0";
    cards1.push_back(card1);

    std::vector<AgentCard> cards2;
    AgentCard card2;
    card2.agentId = "agent_multi_002";
    card2.name = "Multi Test Agent 2";
    card2.version = "1.0.0";
    cards2.push_back(card2);

    std::vector<AgentCard> cards3;
    AgentCard card3;
    card3.agentId = "agent_multi_003";
    card3.name = "Multi Test Agent 3";
    card3.version = "1.0.0";
    cards3.push_back(card3);

    // Act: Insert cards for different bundles/users
    int ret1 = agentCardDbMgr.InsertData("com.test.multiple1", 100, cards1);
    int ret2 = agentCardDbMgr.InsertData("com.test.multiple2", 101, cards2);
    int ret3 = agentCardDbMgr.InsertData("com.test.multiple3", 102, cards3);

    EXPECT_TRUE(ret1 == ERR_OK);
    EXPECT_TRUE(ret2 == ERR_OK);
    EXPECT_TRUE(ret3 == ERR_OK);

    // Query all data
    std::vector<AgentCard> queryCards;
    int queryRet = agentCardDbMgr.QueryAllData(queryCards);

    // Assert: Should return success with all cards
    EXPECT_TRUE(queryRet == ERR_OK);
    EXPECT_TRUE(queryCards.size() >= 3);
}

/**
 * @tc.name: QueryAllDataTest_004
 * @tc.desc: Test QueryAllData with multiple cards in single bundle - should return all cards
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardDbMgrTest, QueryAllDataTest_004, TestSize.Level1)
{
    // Arrange: Insert multiple cards for the same bundle
    std::vector<AgentCard> insertCards;
    AgentCard card1;
    card1.agentId = "agent_single_001";
    card1.name = "Single Bundle Agent 1";
    card1.version = "1.0.0";
    insertCards.push_back(card1);

    AgentCard card2;
    card2.agentId = "agent_single_002";
    card2.name = "Single Bundle Agent 2";
    card2.version = "1.0.0";
    insertCards.push_back(card2);

    AgentCard card3;
    card3.agentId = "agent_single_003";
    card3.name = "Single Bundle Agent 3";
    card3.version = "1.0.0";
    insertCards.push_back(card3);

    // Act: Insert multiple cards for same bundle
    AgentCardDbMgr agentCardDbMgr;
    int insertRet = agentCardDbMgr.InsertData("com.test.singlebundle", 100, insertCards);
    EXPECT_TRUE(insertRet == ERR_OK);

    // Query all data
    std::vector<AgentCard> queryCards;
    int queryRet = agentCardDbMgr.QueryAllData(queryCards);

    // Assert: Should return success with all cards
    EXPECT_TRUE(queryRet == ERR_OK);
    EXPECT_TRUE(queryCards.size() >= 3);
}

/**
 * @tc.name: QueryAllDataTest_005
 * @tc.desc: Test QueryAllData - verify cards contain correct data
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardDbMgrTest, QueryAllDataTest_005, TestSize.Level1)
{
    // Arrange: Insert a card with specific data
    std::vector<AgentCard> insertCards;
    AgentCard card;
    card.agentId = "agent_verify_001";
    card.name = "Verify Test Agent";
    card.description = "This is a verification test";
    card.version = "2.0.0";

    std::vector<std::string> inputModes = {"text", "voice"};
    std::vector<std::string> outputModes = {"text", "voice"};
    card.defaultInputModes = inputModes;
    card.defaultOutputModes = outputModes;
    insertCards.push_back(card);

    // Act: Insert and query
    AgentCardDbMgr agentCardDbMgr;
    int insertRet = agentCardDbMgr.InsertData("com.test.verify", 100, insertCards);
    EXPECT_TRUE(insertRet == ERR_OK);

    std::vector<AgentCard> queryCards;
    int queryRet = agentCardDbMgr.QueryAllData(queryCards);

    // Assert: Verify the returned data matches
    EXPECT_TRUE(queryRet == ERR_OK);
    bool found = false;
    for (const auto &queryCard : queryCards) {
        if (queryCard.agentId == "agent_verify_001") {
            EXPECT_TRUE(queryCard.name == "Verify Test Agent");
            EXPECT_TRUE(queryCard.version == "2.0.0");
            found = true;
            break;
        }
    }
    EXPECT_TRUE(found);
}

/**
 * @tc.name: QueryAllDataTest_006
 * @tc.desc: Test QueryAllData after delete - should not contain deleted data
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardDbMgrTest, QueryAllDataTest_006, TestSize.Level1)
{
    // Arrange: Insert data then delete it
    std::vector<AgentCard> insertCards;
    AgentCard card;
    card.agentId = "agent_delete_001";
    card.name = "Delete Test Agent";
    card.version = "1.0.0";
    insertCards.push_back(card);

    AgentCardDbMgr agentCardDbMgr;
    int insertRet = agentCardDbMgr.InsertData("com.test.todelete", 100, insertCards);
    EXPECT_TRUE(insertRet == ERR_OK);

    // Verify it exists
    std::vector<AgentCard> queryCards1;
    agentCardDbMgr.QueryAllData(queryCards1);
    bool foundBeforeDelete = false;
    for (const auto &c : queryCards1) {
        if (c.agentId == "agent_delete_001") {
            foundBeforeDelete = true;
            break;
        }
    }
    EXPECT_TRUE(foundBeforeDelete);

    // Act: Delete the data
    int deleteRet = agentCardDbMgr.DeleteData("com.test.todelete", 100);
    EXPECT_TRUE(deleteRet == ERR_OK);

    // Query all again
    std::vector<AgentCard> queryCards2;
    int queryRet = agentCardDbMgr.QueryAllData(queryCards2);

    // Assert: Deleted data should not exist
    EXPECT_TRUE(queryRet == ERR_OK);
    bool foundAfterDelete = false;
    for (const auto &c : queryCards2) {
        if (c.agentId == "agent_delete_001") {
            foundAfterDelete = true;
            break;
        }
    }
    EXPECT_TRUE(!foundAfterDelete);
}

/**
 * @tc.name: QueryAllDataTest_007
 * @tc.desc: Test QueryAllData with empty cards vector input - should work correctly
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardDbMgrTest, QueryAllDataTest_007, TestSize.Level1)
{
    // Arrange: Prepare a fresh empty vector
    std::vector<AgentCard> insertCards;
    AgentCard card;
    card.agentId = "agent_emptyvec_001";
    card.name = "Empty Vector Test";
    card.version = "1.0.0";
    insertCards.push_back(card);

    AgentCardDbMgr agentCardDbMgr;
    agentCardDbMgr.InsertData("com.test.emptyvec", 100, insertCards);

    // Act: Query with empty vector
    std::vector<AgentCard> queryCards;  // Empty vector
    int ret = agentCardDbMgr.QueryAllData(queryCards);

    // Assert: Should populate the vector correctly
    EXPECT_TRUE(ret == ERR_OK);
    EXPECT_TRUE(queryCards.size() >= 1);
}
} // namespace AgentRuntime
} // namespace OHOS