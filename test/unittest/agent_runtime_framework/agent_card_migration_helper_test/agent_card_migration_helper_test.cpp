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
#include <nlohmann/json.hpp>
#include <unistd.h>

#include "distributed_kv_data_manager.h"
#include "errors.h"

#define private public
#include "agent_extension/agent_card_migration_helper.h"
#undef private

using namespace testing;
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AAFwk;
using namespace OHOS::DistributedKv;

namespace {
constexpr const char *TEST_STORE_DIR = "/data/service/el1/public/database/ability_manager_service";
const AppId TEST_APP_ID = { "agent_db" };
const StoreId TEST_STORE_ID = { "agent_card_infos" };

struct KvEntry {
    std::string key;
    std::string value;
};

class TestKvStoreGuard {
public:
    TestKvStoreGuard() = default;
    ~TestKvStoreGuard();
    bool Create();
    bool PutBatch(const std::vector<KvEntry> &entries);
    void CloseAndDelete();

private:
    DistributedKvDataManager dataManager_;
    std::shared_ptr<SingleKvStore> kvStorePtr_;
};

TestKvStoreGuard::~TestKvStoreGuard()
{
    CloseAndDelete();
}

bool TestKvStoreGuard::Create()
{
    Options options = {
        .createIfMissing = true,
        .encrypt = false,
        .autoSync = false,
        .syncable = false,
        .securityLevel = SecurityLevel::S2,
        .area = EL1,
        .kvStoreType = KvStoreType::SINGLE_VERSION,
        .baseDir = TEST_STORE_DIR,
    };
    Status status = dataManager_.GetSingleKvStore(options, TEST_APP_ID, TEST_STORE_ID, kvStorePtr_);
    return status == Status::SUCCESS && kvStorePtr_ != nullptr;
}

bool TestKvStoreGuard::PutBatch(const std::vector<KvEntry> &entries)
{
    if (kvStorePtr_ == nullptr) {
        return false;
    }
    for (const auto &entry : entries) {
        Status status = kvStorePtr_->Put(Key(entry.key), Value(entry.value));
        if (status != Status::SUCCESS) {
            return false;
        }
    }
    return true;
}

void TestKvStoreGuard::CloseAndDelete()
{
    if (kvStorePtr_ != nullptr) {
        dataManager_.CloseKvStore(TEST_APP_ID, kvStorePtr_);
        kvStorePtr_ = nullptr;
    }
    dataManager_.DeleteKvStore(TEST_APP_ID, TEST_STORE_ID, TEST_STORE_DIR);
}

std::string MakeKeyJson(const std::string &bundleName, int32_t userId)
{
    nlohmann::json j;
    j["bundleName"] = bundleName;
    j["userId"] = userId;
    return j.dump();
}

std::string MakeValueJson(const std::vector<std::pair<std::string, nlohmann::json>> &cards)
{
    nlohmann::json j;
    nlohmann::json cardsArray = nlohmann::json::array();
    for (const auto &card : cards) {
        nlohmann::json cardEntry;
        cardEntry["lastUpdateSource"] = card.first;
        cardEntry["card"] = card.second;
        cardsArray.push_back(cardEntry);
    }
    j["cards"] = cardsArray;
    return j.dump();
}

nlohmann::json MakeCardJson(const std::string &agentId, const std::string &name)
{
    nlohmann::json card;
    card["agentId"] = agentId;
    card["name"] = name;
    card["version"] = "1.0.0";
    return card;
}
}

class AgentCardMigrationHelperTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown() {}
};

HWTEST_F(AgentCardMigrationHelperTest, GetDynamicAgentCardsAsJson_001, TestSize.Level1)
{
    std::string jsonStr;
    auto ret = AgentCardMigrationHelper::GetInstance().GetDynamicAgentCardsAsJson(jsonStr);
    EXPECT_EQ(ret, ERR_OK);
}

HWTEST_F(AgentCardMigrationHelperTest, GetDynamicAgentCardsAsJson_002, TestSize.Level1)
{
    TestKvStoreGuard guard;
    ASSERT_TRUE(guard.Create());

    std::vector<KvEntry> entries;
    entries.push_back({
        MakeKeyJson("com.test.app1", 100),
        MakeValueJson({{"api", MakeCardJson("card1", "Card One")}})
    });
    ASSERT_TRUE(guard.PutBatch(entries));

    std::string jsonStr;
    auto ret = AgentCardMigrationHelper::GetInstance().GetDynamicAgentCardsAsJson(jsonStr);
    EXPECT_EQ(ret, ERR_OK);

    auto result = nlohmann::json::parse(jsonStr, nullptr, false);
    ASSERT_TRUE(result.is_array());
    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[0]["bundleName"], "com.test.app1");
    EXPECT_EQ(result[0]["userId"], 100);
    ASSERT_TRUE(result[0]["cards"].is_array());
    ASSERT_EQ(result[0]["cards"].size(), 1);
    EXPECT_EQ(result[0]["cards"][0]["agentId"], "card1");
}

HWTEST_F(AgentCardMigrationHelperTest, GetDynamicAgentCardsAsJson_003, TestSize.Level1)
{
    TestKvStoreGuard guard;
    ASSERT_TRUE(guard.Create());

    std::vector<KvEntry> entries;
    entries.push_back({
        MakeKeyJson("com.test.app2", 100),
        MakeValueJson({{"bundle", MakeCardJson("card2", "Card Two")}})
    });
    ASSERT_TRUE(guard.PutBatch(entries));

    std::string jsonStr;
    auto ret = AgentCardMigrationHelper::GetInstance().GetDynamicAgentCardsAsJson(jsonStr);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(jsonStr, "[]");
}

HWTEST_F(AgentCardMigrationHelperTest, GetDynamicAgentCardsAsJson_004, TestSize.Level1)
{
    TestKvStoreGuard guard;
    ASSERT_TRUE(guard.Create());

    std::vector<KvEntry> entries;
    entries.push_back({
        MakeKeyJson("com.test.app3", 100),
        MakeValueJson({
            {"api", MakeCardJson("card3", "Card Three")},
            {"bundle", MakeCardJson("card4", "Card Four")},
            {"api", MakeCardJson("card5", "Card Five")}
        })
    });
    ASSERT_TRUE(guard.PutBatch(entries));

    std::string jsonStr;
    auto ret = AgentCardMigrationHelper::GetInstance().GetDynamicAgentCardsAsJson(jsonStr);
    EXPECT_EQ(ret, ERR_OK);

    auto result = nlohmann::json::parse(jsonStr, nullptr, false);
    ASSERT_TRUE(result.is_array());
    ASSERT_EQ(result.size(), 1);
    ASSERT_TRUE(result[0]["cards"].is_array());
    ASSERT_EQ(result[0]["cards"].size(), 2);
    EXPECT_EQ(result[0]["cards"][0]["agentId"], "card3");
    EXPECT_EQ(result[0]["cards"][1]["agentId"], "card5");
}

HWTEST_F(AgentCardMigrationHelperTest, GetDynamicAgentCardsAsJson_005, TestSize.Level1)
{
    TestKvStoreGuard guard;
    ASSERT_TRUE(guard.Create());

    std::vector<KvEntry> entries;
    entries.push_back({
        MakeKeyJson("com.test.app4", 100),
        MakeValueJson({{"api", MakeCardJson("card6", "Card Six")}})
    });
    entries.push_back({
        MakeKeyJson("com.test.app5", 101),
        MakeValueJson({{"api", MakeCardJson("card7", "Card Seven")}})
    });
    ASSERT_TRUE(guard.PutBatch(entries));

    std::string jsonStr;
    auto ret = AgentCardMigrationHelper::GetInstance().GetDynamicAgentCardsAsJson(jsonStr);
    EXPECT_EQ(ret, ERR_OK);

    auto result = nlohmann::json::parse(jsonStr, nullptr, false);
    ASSERT_TRUE(result.is_array());
    ASSERT_EQ(result.size(), 2);
}

HWTEST_F(AgentCardMigrationHelperTest, GetDynamicAgentCardsAsJson_006, TestSize.Level1)
{
    TestKvStoreGuard guard;
    ASSERT_TRUE(guard.Create());

    std::vector<KvEntry> entries;
    entries.push_back({"not_valid_json", "{\"cards\":[]}"});
    entries.push_back({
        MakeKeyJson("com.test.app6", 100),
        "not_valid_json"
    });
    entries.push_back({
        MakeKeyJson("com.test.app7", 100),
        "{\"cards\":\"not_array\"}"
    });
    entries.push_back({
        "{\"noBundleName\":true,\"userId\":100}",
        MakeValueJson({{"api", MakeCardJson("card8", "Card Eight")}})
    });
    entries.push_back({
        MakeKeyJson("com.test.app8", 100),
        "{\"cards\":[{\"lastUpdateSource\":\"api\"}]}"
    });
    ASSERT_TRUE(guard.PutBatch(entries));

    std::string jsonStr;
    auto ret = AgentCardMigrationHelper::GetInstance().GetDynamicAgentCardsAsJson(jsonStr);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(jsonStr, "[]");
}

HWTEST_F(AgentCardMigrationHelperTest, GetDynamicAgentCardsAsJson_007, TestSize.Level1)
{
    TestKvStoreGuard guard;
    ASSERT_TRUE(guard.Create());

    std::vector<KvEntry> entries;
    entries.push_back({
        MakeKeyJson("com.test.app9", 100),
        MakeValueJson({})
    });
    ASSERT_TRUE(guard.PutBatch(entries));

    std::string jsonStr;
    auto ret = AgentCardMigrationHelper::GetInstance().GetDynamicAgentCardsAsJson(jsonStr);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(jsonStr, "[]");
}
