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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "ability_manager_errors.h"
#include "mock_single_kv_store.h"

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
namespace {
AgentCard BuildValidCard(const std::string &agentId)
{
    AgentCard card;
    card.agentId = agentId;
    card.name = "Test Agent";
    card.description = "Test description";
    card.version = "1.0.0";
    card.category = "test";
    card.defaultInputModes = {"text"};
    card.defaultOutputModes = {"text"};
    auto skill = std::make_shared<AgentSkill>();
    skill->id = "test_skill";
    skill->name = "Test Skill";
    skill->description = "Test skill description";
    skill->tags = {"test_tag"};
    card.skills.push_back(skill);
    card.iconUrl = "http://example.com/icon.png";
    return card;
}

std::vector<StoredAgentCardEntry> MakeStoredEntries(std::initializer_list<AgentCard> cards,
    AgentCardUpdateSource source = AgentCardUpdateSource::BUNDLE)
{
    std::vector<StoredAgentCardEntry> entries;
    for (const auto &card : cards) {
        entries.push_back({ card, source });
    }
    return entries;
}
} // namespace

class MockSingleKvStoreForDbMgr final : public OHOS::MockSingleKvStore {
public:
    DistributedKv::Status getStatus_ = DistributedKv::Status::SUCCESS;
    DistributedKv::Value getValue_;
    DistributedKv::Status getEntriesStatus_ = DistributedKv::Status::SUCCESS;
    std::vector<DistributedKv::Entry> entries_;

    DistributedKv::Status Get(const DistributedKv::Key &key, DistributedKv::Value &value) override
    {
        if (getStatus_ == DistributedKv::Status::SUCCESS) {
            value = getValue_;
        }
        return getStatus_;
    }

    DistributedKv::Status GetEntries(
        const DistributedKv::Key &prefix, std::vector<DistributedKv::Entry> &entries) const override
    {
        if (getEntriesStatus_ == DistributedKv::Status::SUCCESS) {
            entries = entries_;
        }
        return getEntriesStatus_;
    }

    DistributedKv::Status GetEntries(
        const DistributedKv::DataQuery &query, std::vector<DistributedKv::Entry> &entries) const override
    {
        if (getEntriesStatus_ == DistributedKv::Status::SUCCESS) {
            entries = entries_;
        }
        return getEntriesStatus_;
    }
};

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
    DistributedKv::Status ret = agentCardDbMgr.RestoreKvStore(DistributedKv::Status::SUCCESS);
    EXPECT_EQ(ret, DistributedKv::Status::SUCCESS);
    ret = agentCardDbMgr.RestoreKvStore(DistributedKv::Status::KEY_NOT_FOUND);
    EXPECT_EQ(ret, DistributedKv::Status::KEY_NOT_FOUND);
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
    DistributedKv::Options options = agentCardDbMgr.CreateKvStoreOptions();
    EXPECT_TRUE(options.createIfMissing);
    EXPECT_FALSE(options.encrypt);
    EXPECT_FALSE(options.autoSync);
    EXPECT_FALSE(options.syncable);
    EXPECT_EQ(options.kvStoreType, DistributedKv::KvStoreType::SINGLE_VERSION);
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
    agentCardDbMgr.kvStorePtr_ = std::make_shared<MockSingleKvStoreForDbMgr>();
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
    AgentCardDbMgr agentCardDbMgr;
    auto mockStore = std::make_shared<MockSingleKvStoreForDbMgr>();
    agentCardDbMgr.kvStorePtr_ = mockStore;
    auto cards = MakeStoredEntries({BuildValidCard("agent_insert_001")});
    int ret = agentCardDbMgr.InsertData("test", 100, cards);
    EXPECT_EQ(ret, ERR_OK);
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
    auto mockStore = std::make_shared<MockSingleKvStoreForDbMgr>();
    agentCardDbMgr.kvStorePtr_ = mockStore;
    int ret = agentCardDbMgr.DeleteData("test", 100);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: QueryDataTest_001
 * @tc.desc: QueryDataTest_001
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardDbMgrTest, QueryDataTest_001, TestSize.Level1)
{
    std::vector<StoredAgentCardEntry> cards;
    AgentCardDbMgr agentCardDbMgr;
    auto mockStore = std::make_shared<MockSingleKvStoreForDbMgr>();
    mockStore->getStatus_ = DistributedKv::Status::KEY_NOT_FOUND;
    agentCardDbMgr.kvStorePtr_ = mockStore;
    int ret = agentCardDbMgr.QueryData("test", 100, cards);
    EXPECT_EQ(ret, ERR_NAME_NOT_FOUND);
}

/**
 * @tc.name: QueryDataTest_002
 * @tc.desc: Test QueryData returns INNER_ERR when stored JSON is invalid
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardDbMgrTest, QueryDataTest_002, TestSize.Level1)
{
    auto insertCards = MakeStoredEntries({BuildValidCard("agent_query_001")});
    AgentCardDbMgr agentCardDbMgr;
    auto mockStore = std::make_shared<MockSingleKvStoreForDbMgr>();
    mockStore->getStatus_ = DistributedKv::Status::SUCCESS;
    mockStore->getValue_ = agentCardDbMgr.ConvertValue(insertCards);
    agentCardDbMgr.kvStorePtr_ = mockStore;
    std::vector<StoredAgentCardEntry> queryCards;
    int ret = agentCardDbMgr.QueryData("com.test.query", 100, queryCards);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(queryCards.size(), 1);
    EXPECT_EQ(queryCards[0].card.agentId, "agent_query_001");
}

/**
 * @tc.name: QueryDataTest_005
 * @tc.desc: Test QueryData supports legacy raw-array payloads and defaults source to bundle
 * @tc.type: FUNC
 */
HWTEST_F(AgentCardDbMgrTest, QueryDataTest_005, TestSize.Level1)
{
    AgentCardDbMgr agentCardDbMgr;
    auto mockStore = std::make_shared<MockSingleKvStoreForDbMgr>();
    nlohmann::json legacyArray = nlohmann::json::array();
    legacyArray.push_back(BuildValidCard("agent_query_legacy").ToJson());
    mockStore->getStatus_ = DistributedKv::Status::SUCCESS;
    mockStore->getValue_ = DistributedKv::Value(legacyArray.dump());
    agentCardDbMgr.kvStorePtr_ = mockStore;

    std::vector<StoredAgentCardEntry> queryCards;
    int ret = agentCardDbMgr.QueryData("com.test.legacy", 100, queryCards);
    EXPECT_EQ(ret, ERR_OK);
    ASSERT_EQ(queryCards.size(), 1);
    EXPECT_EQ(queryCards[0].card.agentId, "agent_query_legacy");
    EXPECT_EQ(queryCards[0].source, AgentCardUpdateSource::BUNDLE);
}

/**
 * @tc.name: QueryDataTest_006
 * @tc.desc: Test QueryData round-trips stored entry source metadata
 * @tc.type: FUNC
 */
HWTEST_F(AgentCardDbMgrTest, QueryDataTest_006, TestSize.Level1)
{
    AgentCardDbMgr agentCardDbMgr;
    auto mockStore = std::make_shared<MockSingleKvStoreForDbMgr>();
    auto insertCards = MakeStoredEntries({BuildValidCard("agent_query_api")}, AgentCardUpdateSource::API);
    mockStore->getStatus_ = DistributedKv::Status::SUCCESS;
    mockStore->getValue_ = agentCardDbMgr.ConvertValue(insertCards);
    agentCardDbMgr.kvStorePtr_ = mockStore;

    std::vector<StoredAgentCardEntry> queryCards;
    int ret = agentCardDbMgr.QueryData("com.test.api", 100, queryCards);
    EXPECT_EQ(ret, ERR_OK);
    ASSERT_EQ(queryCards.size(), 1);
    EXPECT_EQ(queryCards[0].card.agentId, "agent_query_api");
    EXPECT_EQ(queryCards[0].source, AgentCardUpdateSource::API);
}

/**
 * @tc.name: QueryAllDataTest_001
 * @tc.desc: Test QueryAllData with valid data - should return ERR_OK with cards
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardDbMgrTest, QueryAllDataTest_001, TestSize.Level1)
{
    auto insertCards = MakeStoredEntries({BuildValidCard("agent_all_001")});
    AgentCardDbMgr agentCardDbMgr;
    auto mockStore = std::make_shared<MockSingleKvStoreForDbMgr>();
    DistributedKv::Entry entry;
    entry.key = DistributedKv::Key("k1");
    entry.value = agentCardDbMgr.ConvertValue(insertCards);
    mockStore->entries_.push_back(entry);
    agentCardDbMgr.kvStorePtr_ = mockStore;
    std::vector<StoredAgentCardEntry> queryCards;
    int queryRet = agentCardDbMgr.QueryAllData(queryCards);
    EXPECT_EQ(queryRet, ERR_OK);
    EXPECT_EQ(queryCards.size(), 1);
    EXPECT_EQ(queryCards[0].card.agentId, "agent_all_001");
}

/**
 * @tc.name: QueryAllDataTest_002
 * @tc.desc: Test QueryAllData with empty database - should return ERR_OK with empty cards
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardDbMgrTest, QueryAllDataTest_002, TestSize.Level1)
{
    AgentCardDbMgr agentCardDbMgr;
    auto mockStore = std::make_shared<MockSingleKvStoreForDbMgr>();
    agentCardDbMgr.kvStorePtr_ = mockStore;
    std::vector<StoredAgentCardEntry> cards;
    int ret = agentCardDbMgr.QueryAllData(cards);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_TRUE(cards.empty());
}

/**
 * @tc.name: QueryAllDataTest_003
 * @tc.desc: Test QueryAllData after inserting multiple cards - should return all cards
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardDbMgrTest, QueryAllDataTest_003, TestSize.Level1)
{
    AgentCardDbMgr agentCardDbMgr;
    auto mockStore = std::make_shared<MockSingleKvStoreForDbMgr>();
    DistributedKv::Entry e1;
    e1.key = DistributedKv::Key("k1");
    e1.value = agentCardDbMgr.ConvertValue(MakeStoredEntries({BuildValidCard("agent_multi_001")}));
    DistributedKv::Entry e2;
    e2.key = DistributedKv::Key("k2");
    e2.value = agentCardDbMgr.ConvertValue(MakeStoredEntries({BuildValidCard("agent_multi_002")}));
    DistributedKv::Entry e3;
    e3.key = DistributedKv::Key("k3");
    e3.value = agentCardDbMgr.ConvertValue(MakeStoredEntries({BuildValidCard("agent_multi_003")}));
    mockStore->entries_ = {e1, e2, e3};
    agentCardDbMgr.kvStorePtr_ = mockStore;
    std::vector<StoredAgentCardEntry> queryCards;
    int queryRet = agentCardDbMgr.QueryAllData(queryCards);
    EXPECT_EQ(queryRet, ERR_OK);
    EXPECT_EQ(queryCards.size(), 3);
}

/**
 * @tc.name: QueryAllDataTest_004
 * @tc.desc: Test QueryAllData with multiple cards in single bundle - should return all cards
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardDbMgrTest, QueryAllDataTest_004, TestSize.Level1)
{
    std::vector<AgentCard> insertCards = {
        BuildValidCard("agent_single_001"),
        BuildValidCard("agent_single_002"),
        BuildValidCard("agent_single_003"),
    };
    AgentCardDbMgr agentCardDbMgr;
    auto mockStore = std::make_shared<MockSingleKvStoreForDbMgr>();
    DistributedKv::Entry entry;
    entry.key = DistributedKv::Key("single_bundle");
    entry.value = agentCardDbMgr.ConvertValue(MakeStoredEntries({insertCards[0], insertCards[1], insertCards[2]}));
    mockStore->entries_ = {entry};
    agentCardDbMgr.kvStorePtr_ = mockStore;
    std::vector<StoredAgentCardEntry> queryCards;
    int queryRet = agentCardDbMgr.QueryAllData(queryCards);
    EXPECT_EQ(queryRet, ERR_OK);
    EXPECT_EQ(queryCards.size(), 3);
}

/**
 * @tc.name: QueryAllDataTest_005
 * @tc.desc: Test QueryAllData - verify cards contain correct data
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardDbMgrTest, QueryAllDataTest_005, TestSize.Level1)
{
    auto card = BuildValidCard("agent_verify_001");
    card.name = "Verify Test Agent";
    card.version = "2.0.0";
    AgentCardDbMgr agentCardDbMgr;
    auto mockStore = std::make_shared<MockSingleKvStoreForDbMgr>();
    DistributedKv::Entry entry;
    entry.key = DistributedKv::Key("verify_key");
    entry.value = agentCardDbMgr.ConvertValue(MakeStoredEntries({card}));
    mockStore->entries_ = {entry};
    agentCardDbMgr.kvStorePtr_ = mockStore;
    std::vector<StoredAgentCardEntry> queryCards;
    int queryRet = agentCardDbMgr.QueryAllData(queryCards);
    EXPECT_EQ(queryRet, ERR_OK);
    EXPECT_EQ(queryCards.size(), 1);
    EXPECT_EQ(queryCards[0].card.name, "Verify Test Agent");
    EXPECT_EQ(queryCards[0].card.version, "2.0.0");
}

/**
 * @tc.name: QueryAllDataTest_006
 * @tc.desc: Test QueryAllData after delete - should not contain deleted data
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardDbMgrTest, QueryAllDataTest_006, TestSize.Level1)
{
    AgentCardDbMgr agentCardDbMgr;
    auto mockStore = std::make_shared<MockSingleKvStoreForDbMgr>();
    agentCardDbMgr.kvStorePtr_ = mockStore;
    int deleteRet = agentCardDbMgr.DeleteData("com.test.todelete", 100);
    EXPECT_EQ(deleteRet, ERR_OK);
}

/**
 * @tc.name: QueryAllDataTest_007
 * @tc.desc: Test QueryAllData with empty cards vector input - should work correctly
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardDbMgrTest, QueryAllDataTest_007, TestSize.Level1)
{
    AgentCardDbMgr agentCardDbMgr;
    auto mockStore = std::make_shared<MockSingleKvStoreForDbMgr>();
    DistributedKv::Entry entry;
    entry.key = DistributedKv::Key("empty_vec");
    entry.value = agentCardDbMgr.ConvertValue(MakeStoredEntries({BuildValidCard("agent_emptyvec_001")}));
    mockStore->entries_ = {entry};
    agentCardDbMgr.kvStorePtr_ = mockStore;
    std::vector<StoredAgentCardEntry> queryCards;
    int ret = agentCardDbMgr.QueryAllData(queryCards);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(queryCards.size(), 1);
}

/**
 * @tc.name: QueryAllDataTest_008
 * @tc.desc: Test QueryAllData returns INNER_ERR when stored JSON is invalid
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardDbMgrTest, QueryAllDataTest_008, TestSize.Level1)
{
    AgentCardDbMgr agentCardDbMgr;
    auto mockStore = std::make_shared<MockSingleKvStoreForDbMgr>();
    DistributedKv::Entry entry;
    entry.key = DistributedKv::Key("invalid_json");
    entry.value = DistributedKv::Value("invalid_json_payload");
    mockStore->entries_ = {entry};
    agentCardDbMgr.kvStorePtr_ = mockStore;
    std::vector<StoredAgentCardEntry> queryCards;
    int ret = agentCardDbMgr.QueryAllData(queryCards);
    EXPECT_EQ(ret, AAFwk::INNER_ERR);
}

/**
 * @tc.name: InsertDataTest_002
 * @tc.desc: Test InsertData returns ERR_INVALID_OPERATION when kv put fails.
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardDbMgrTest, InsertDataTest_002, TestSize.Level1)
{
    AgentCardDbMgr agentCardDbMgr;
    auto mockStore = std::make_shared<MockSingleKvStoreForDbMgr>();
    mockStore->Put_ = DistributedKv::Status::KEY_NOT_FOUND;
    agentCardDbMgr.kvStorePtr_ = mockStore;

    std::vector<StoredAgentCardEntry> cards;
    int ret = agentCardDbMgr.InsertData("test.put.fail", 100, cards);
    EXPECT_EQ(ret, ERR_INVALID_OPERATION);
}

/**
 * @tc.name: DeleteDataTest_002
 * @tc.desc: Test DeleteData returns ERR_INVALID_OPERATION when kv delete fails.
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardDbMgrTest, DeleteDataTest_002, TestSize.Level1)
{
    AgentCardDbMgr agentCardDbMgr;
    auto mockStore = std::make_shared<MockSingleKvStoreForDbMgr>();
    mockStore->Delete_ = DistributedKv::Status::KEY_NOT_FOUND;
    agentCardDbMgr.kvStorePtr_ = mockStore;

    int ret = agentCardDbMgr.DeleteData("test.delete.fail", 100);
    EXPECT_EQ(ret, ERR_INVALID_OPERATION);
}

/**
 * @tc.name: QueryDataTest_003
 * @tc.desc: Test QueryData returns INNER_ERR for invalid json value.
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardDbMgrTest, QueryDataTest_003, TestSize.Level1)
{
    AgentCardDbMgr agentCardDbMgr;
    auto mockStore = std::make_shared<MockSingleKvStoreForDbMgr>();
    mockStore->getStatus_ = DistributedKv::Status::SUCCESS;
    mockStore->getValue_ = DistributedKv::Value("not-json");
    agentCardDbMgr.kvStorePtr_ = mockStore;

    std::vector<StoredAgentCardEntry> cards;
    int ret = agentCardDbMgr.QueryData("test.query.invalidjson", 100, cards);
    EXPECT_EQ(ret, AAFwk::INNER_ERR);
}

/**
 * @tc.name: QueryDataTest_004
 * @tc.desc: Test QueryData returns non-KEY_NOT_FOUND failure status.
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardDbMgrTest, QueryDataTest_004, TestSize.Level1)
{
    AgentCardDbMgr agentCardDbMgr;
    auto mockStore = std::make_shared<MockSingleKvStoreForDbMgr>();
    // Use a synthetic non-KEY_NOT_FOUND status to cover the generic error path
    // without triggering DATA_CORRUPTED recovery flow (which touches real kv backend).
    constexpr auto testStatus = static_cast<DistributedKv::Status>(-1);
    mockStore->getStatus_ = testStatus;
    agentCardDbMgr.kvStorePtr_ = mockStore;

    std::vector<StoredAgentCardEntry> cards;
    int ret = agentCardDbMgr.QueryData("test.query.fail", 100, cards);
    EXPECT_EQ(ret, static_cast<int32_t>(testStatus));
}

/**
 * @tc.name: QueryAllDataTest_009
 * @tc.desc: Test QueryAllData returns status on get entries failure.
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardDbMgrTest, QueryAllDataTest_009, TestSize.Level1)
{
    AgentCardDbMgr agentCardDbMgr;
    auto mockStore = std::make_shared<MockSingleKvStoreForDbMgr>();
    mockStore->getEntriesStatus_ = DistributedKv::Status::KEY_NOT_FOUND;
    agentCardDbMgr.kvStorePtr_ = mockStore;

    std::vector<StoredAgentCardEntry> cards;
    int ret = agentCardDbMgr.QueryAllData(cards);
    EXPECT_EQ(ret, static_cast<int32_t>(DistributedKv::Status::KEY_NOT_FOUND));
}

/**
 * @tc.name: QueryAllDataTest_010
 * @tc.desc: Test QueryAllData returns INNER_ERR for invalid json entry value.
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardDbMgrTest, QueryAllDataTest_010, TestSize.Level1)
{
    AgentCardDbMgr agentCardDbMgr;
    auto mockStore = std::make_shared<MockSingleKvStoreForDbMgr>();
    mockStore->getEntriesStatus_ = DistributedKv::Status::SUCCESS;
    DistributedKv::Entry entry;
    entry.key = DistributedKv::Key("key_invalid_json");
    entry.value = DistributedKv::Value("not-json");
    mockStore->entries_.push_back(entry);
    agentCardDbMgr.kvStorePtr_ = mockStore;

    std::vector<StoredAgentCardEntry> cards;
    int ret = agentCardDbMgr.QueryAllData(cards);
    EXPECT_EQ(ret, AAFwk::INNER_ERR);
}

/**
 * @tc.name: QueryAllDataTest_011
 * @tc.desc: Test QueryAllData continues when AgentCard::FromJson fails.
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardDbMgrTest, QueryAllDataTest_011, TestSize.Level1)
{
    AgentCardDbMgr agentCardDbMgr;
    auto mockStore = std::make_shared<MockSingleKvStoreForDbMgr>();
    mockStore->getEntriesStatus_ = DistributedKv::Status::SUCCESS;
    DistributedKv::Entry entry;
    entry.key = DistributedKv::Key("key_invalid_card");
    entry.value = DistributedKv::Value(R"([{"name":"missing_agent_id"}])");
    mockStore->entries_.push_back(entry);
    agentCardDbMgr.kvStorePtr_ = mockStore;

    std::vector<StoredAgentCardEntry> cards;
    int ret = agentCardDbMgr.QueryAllData(cards);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_TRUE(cards.empty());
}
} // namespace AgentRuntime
} // namespace OHOS
