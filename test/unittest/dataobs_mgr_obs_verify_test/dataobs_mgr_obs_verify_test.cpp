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
#include <memory>
#include <functional>
#include <thread>
#include "uri.h"

// Workaround to access private members for testing
// NOLINTNEXTLINE(g.nam.03-cpp): Redefining C++ keyword 'private' for unit testing
#define private public

#include "obs_permission_verifier.h"
#include "hilog_tag_wrapper.h"

using namespace OHOS;
using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace AAFwk {
using Uri = OHOS::Uri;

class DataObsMgrObsVerifyTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    ObsPermissionVerifier *obsVerifyUtils_ = nullptr;
};

void DataObsMgrObsVerifyTest::SetUpTestCase(void) {}
void DataObsMgrObsVerifyTest::TearDownTestCase(void) {}
void DataObsMgrObsVerifyTest::SetUp()
{
    obsVerifyUtils_ = &ObsPermissionVerifier::GetInstance();
}
void DataObsMgrObsVerifyTest::TearDown()
{
    if (obsVerifyUtils_ != nullptr) {
        obsVerifyUtils_->groupsIdCache_.clear();
    }
}

static constexpr int32_t USER_TEST = 100;
static constexpr uint32_t TOKEN_TEST_1 = 1001;
static constexpr uint32_t TOKEN_TEST_2 = 1002;
static constexpr uint32_t TOKEN_SAME = 1001;

/*
 * Feature: OBSVerifyPermissionUtils
 * Function: VerifyPermission with same tokens
 * SubFunction: VerifyPermission
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Test permission verification when listenerTokenId equals tokenId
 */
HWTEST_F(DataObsMgrObsVerifyTest, OBSVerifyPermissionUtils_VerifyPermission_SameToken_0100, TestSize.Level0)
{
    ASSERT_TRUE(obsVerifyUtils_ != nullptr);

    Uri uri("rdb://com.example.data/app");
    uint32_t listenerTokenId = TOKEN_TEST_1;
    uint32_t tokenId = TOKEN_TEST_1; // Same token
    int32_t userId = USER_TEST;

    // Should return true when tokens are same
    bool result = obsVerifyUtils_->VerifyPermission(listenerTokenId, userId, uri, tokenId);

    EXPECT_TRUE(result);
}

/*
 * Feature: OBSVerifyPermissionUtils
 * Function: VerifyPermission with different tokens
 * SubFunction: VerifyPermission
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Test permission verification with different tokens - expect false without valid token info
 */
HWTEST_F(DataObsMgrObsVerifyTest, OBSVerifyPermissionUtils_VerifyPermission_DifferentToken_0100, TestSize.Level0)
{
    ASSERT_TRUE(obsVerifyUtils_ != nullptr);

    Uri uri("rdb://com.example.data/app");
    uint32_t listenerTokenId = TOKEN_TEST_1;
    uint32_t tokenId = TOKEN_TEST_2; // Different token
    int32_t userId = USER_TEST;

    // With different tokens and no valid group info, should return false
    bool result = obsVerifyUtils_->VerifyPermission(listenerTokenId, userId, uri, tokenId);

    // Expect false since tokens are different and no group match
    EXPECT_FALSE(result);
}

/*
 * Feature: OBSVerifyPermissionUtils
 * Function: VerifyPermission with RDB scheme
 * SubFunction: VerifyPermission
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Test permission verification with RDB URI scheme
 */
HWTEST_F(DataObsMgrObsVerifyTest, OBSVerifyPermissionUtils_VerifyPermission_RDBScheme_0100, TestSize.Level0)
{
    ASSERT_TRUE(obsVerifyUtils_ != nullptr);

    Uri uri("rdb://com.example.data/app");
    uint32_t listenerTokenId = TOKEN_TEST_1;
    uint32_t tokenId = TOKEN_SAME;
    int32_t userId = USER_TEST;

    bool result = obsVerifyUtils_->VerifyPermission(listenerTokenId, userId, uri, tokenId);

    // Should return true for same token regardless of scheme
    EXPECT_TRUE(result);
}

/*
 * Feature: OBSVerifyPermissionUtils
 * Function: VerifyPermission with preferences scheme
 * SubFunction: VerifyPermission
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Test permission verification with preferences URI scheme
 */
HWTEST_F(DataObsMgrObsVerifyTest, OBSVerifyPermissionUtils_VerifyPermission_PrefScheme_0100, TestSize.Level0)
{
    ASSERT_TRUE(obsVerifyUtils_ != nullptr);

    Uri uri("sharepreferences://com.example.data/app");
    uint32_t listenerTokenId = TOKEN_TEST_1;
    uint32_t tokenId = TOKEN_SAME;
    int32_t userId = USER_TEST;

    bool result = obsVerifyUtils_->VerifyPermission(listenerTokenId, userId, uri, tokenId);

    // Should return true for same token
    EXPECT_TRUE(result);
}

/*
 * Feature: OBSVerifyPermissionUtils
 * Function: GetGroupInfosFromCache with cache hit
 * SubFunction: GetGroupInfosFromCache
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Test cache retrieval when entry exists
 */
HWTEST_F(DataObsMgrObsVerifyTest, OBSVerifyPermissionUtils_GetGroupInfos_CacheHit_0100, TestSize.Level1)
{
    ASSERT_TRUE(obsVerifyUtils_ != nullptr);

    // Setup: populate cache
    std::string testUri = "rdb://com.example.test/data";
    std::string testBundle = "com.example.test";
    std::vector<std::string> testGroupIds = {"group1", "group2", "group3"};
    std::string key = testBundle + ":" + std::to_string(USER_TEST);

    std::unique_lock<std::shared_mutex> writeLock(obsVerifyUtils_->groupsIdMutex_);
    obsVerifyUtils_->groupsIdCache_.clear();
    obsVerifyUtils_->groupsIdCache_.emplace_back(key, testGroupIds);
    writeLock.unlock();

    // Call GetGroupInfosFromCache
    auto result = obsVerifyUtils_->GetGroupInfosFromCache(testBundle, USER_TEST);

    // Should return cached group IDs
    EXPECT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], "group1");
    EXPECT_EQ(result[1], "group2");
    EXPECT_EQ(result[2], "group3");
}

/*
 * Feature: OBSVerifyPermissionUtils
 * Function: GetGroupInfosFromCache with cache miss
 * SubFunction: GetGroupInfosFromCache
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Test cache miss scenario (will try to query BMS)
 */
HWTEST_F(DataObsMgrObsVerifyTest, OBSVerifyPermissionUtils_GetGroupInfos_CacheMiss_0100, TestSize.Level1)
{
    ASSERT_TRUE(obsVerifyUtils_ != nullptr);

    std::string testUri = "rdb://com.example.nonexistent/data";
    std::string testBundle = "com.example.nonexistent";

    // Clear cache to ensure miss
    std::unique_lock<std::shared_mutex> writeLock(obsVerifyUtils_->groupsIdMutex_);
    obsVerifyUtils_->groupsIdCache_.clear();
    writeLock.unlock();

    // Call GetGroupInfosFromCache - will try to query BMS (may return empty)
    auto result = obsVerifyUtils_->GetGroupInfosFromCache(testBundle, USER_TEST);

    // Should not crash, result depends on BMS availability
    // Since BMS query will likely fail in test environment, expect empty
    EXPECT_TRUE(result.empty());
}

/*
 * Feature: OBSVerifyPermissionUtils
 * Function: GetGroupInfosFromCache cache full scenario
 * SubFunction: GetGroupInfosFromCache
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Test cache behavior when size threshold is reached
 */
HWTEST_F(DataObsMgrObsVerifyTest, OBSVerifyPermissionUtils_GetGroupInfos_CacheFull_0100, TestSize.Level1)
{
    ASSERT_TRUE(obsVerifyUtils_ != nullptr);

    std::string testBundle = "com.example.test";
    int32_t userId = USER_TEST;

    // Fill cache to threshold
    std::unique_lock<std::shared_mutex> writeLock(obsVerifyUtils_->groupsIdMutex_);
    obsVerifyUtils_->groupsIdCache_.clear();

    // Add entries up to CACHE_SIZE_THRESHOLD
    for (int i = 0; i < obsVerifyUtils_->CACHE_SIZE_THRESHOLD; i++) {
        std::string bundle = "cache" + std::to_string(i);
        std::string key = bundle + ":" + std::to_string(userId);
        std::vector<std::string> groupIds = {"group" + std::to_string(i)};
        obsVerifyUtils_->groupsIdCache_.emplace_back(key, groupIds);
    }

    size_t sizeBefore = obsVerifyUtils_->groupsIdCache_.size();
    EXPECT_EQ(sizeBefore, obsVerifyUtils_->CACHE_SIZE_THRESHOLD);
    writeLock.unlock();

    // This will trigger cache overflow logic - pop_front then emplace_back
    // But since BMS query may fail in test, we just verify no crash
    std::string testUri = "rdb://com.example.test/newentry";
    auto result = obsVerifyUtils_->GetGroupInfosFromCache(testBundle, userId);

    // Should not crash, expect empty since BMS query fails
    EXPECT_TRUE(result.empty());
}

/*
 * Feature: OBSVerifyPermissionUtils
 * Function: VerifyPermission with empty groupIds
 * SubFunction: VerifyPermission
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Test permission verification when group IDs are empty
 */
HWTEST_F(DataObsMgrObsVerifyTest, OBSVerifyPermissionUtils_VerifyPermission_EmptyGroups_0100, TestSize.Level1)
{
    ASSERT_TRUE(obsVerifyUtils_ != nullptr);

    // Setup: cache with empty group IDs
    std::string testBundle = "com.example.empty";
    std::string testUri = "rdb://com.example.empty/data";
    std::vector<std::string> emptyGroupIds;
    std::string key = testBundle + ":" + std::to_string(USER_TEST);

    std::unique_lock<std::shared_mutex> writeLock(obsVerifyUtils_->groupsIdMutex_);
    obsVerifyUtils_->groupsIdCache_.clear();
    obsVerifyUtils_->groupsIdCache_.emplace_back(key, emptyGroupIds);
    writeLock.unlock();

    Uri uri(testUri);
    uint32_t listenerTokenId = TOKEN_TEST_1;
    uint32_t tokenId = TOKEN_TEST_2;
    int32_t userId = USER_TEST;

    // Should return false if GetCallingInfo returns non-SA and empty groups
    // (actual behavior depends on token lookup)
    bool result = obsVerifyUtils_->VerifyPermission(listenerTokenId, userId, uri, tokenId);
    // With empty group IDs and different tokens, expect false
    EXPECT_FALSE(result);
}

/*
 * Feature: OBSVerifyPermissionUtils
 * Function: VerifyPermission with non-matching authority
 * SubFunction: VerifyPermission
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Test when cached group IDs don't match URI authority
 */
HWTEST_F(DataObsMgrObsVerifyTest, OBSVerifyPermissionUtils_VerifyPermission_NoMatch_0100, TestSize.Level1)
{
    ASSERT_TRUE(obsVerifyUtils_ != nullptr);

    // Setup: cache with group IDs that don't match URI
    std::string testBundle = "com.example.test";
    std::string testUri = "rdb://com.example.test/data";
    std::vector<std::string> testGroupIds = {"differentGroupId", "anotherGroupId"};
    std::string key = testBundle + ":" + std::to_string(USER_TEST);

    std::unique_lock<std::shared_mutex> writeLock(obsVerifyUtils_->groupsIdMutex_);
    obsVerifyUtils_->groupsIdCache_.clear();
    obsVerifyUtils_->groupsIdCache_.emplace_back(key, testGroupIds);
    writeLock.unlock();

    Uri uri(testUri);
    uint32_t listenerTokenId = TOKEN_TEST_1;
    uint32_t tokenId = TOKEN_TEST_2;
    int32_t userId = USER_TEST;

    bool result = obsVerifyUtils_->VerifyPermission(listenerTokenId, userId, uri, tokenId);

    // Should return false since authority doesn't match any group ID
    EXPECT_FALSE(result);
}

/*
 * Feature: OBSVerifyPermissionUtils
 * Function: GetGroupInfosFromCache with same key insertion
 * SubFunction: GetGroupInfosFromCache
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Test double-check pattern in cache insertion
 */
HWTEST_F(DataObsMgrObsVerifyTest, OBSVerifyPermissionUtils_GetGroupInfos_DoubleCheck_0100, TestSize.Level1)
{
    ASSERT_TRUE(obsVerifyUtils_ != nullptr);

    std::string testBundle = "com.example.dblcheck";
    std::string testUri = "rdb://com.example.dblcheck/data";
    std::vector<std::string> testGroupIds = {"group1"};
    std::string key = testBundle + ":" + std::to_string(USER_TEST);

    std::unique_lock<std::shared_mutex> writeLock(obsVerifyUtils_->groupsIdMutex_);
    obsVerifyUtils_->groupsIdCache_.clear();
    writeLock.unlock();

    // First call - will try to query BMS (fails in test)
    auto result1 = obsVerifyUtils_->GetGroupInfosFromCache(testBundle, USER_TEST);
    EXPECT_TRUE(result1.empty());

    // Manually add to cache for testing
    writeLock.lock();
    obsVerifyUtils_->groupsIdCache_.emplace_back(key, testGroupIds);
    writeLock.unlock();

    // Second call - should hit cache
    auto result2 = obsVerifyUtils_->GetGroupInfosFromCache(testBundle, USER_TEST);

    // Verify second call returns cached data
    EXPECT_EQ(result2.size(), 1);
    EXPECT_EQ(result2[0], "group1");
}

/*
 * Feature: OBSVerifyPermissionUtils
 * Function: VerifyPermission with preferences scheme (not RDB)
 * SubFunction: VerifyPermission
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Test with sharepreferences scheme to ensure correct error reporting
 */
HWTEST_F(DataObsMgrObsVerifyTest, OBSVerifyPermissionUtils_VerifyPermission_PrefSchemeFail_0100, TestSize.Level1)
{
    ASSERT_TRUE(obsVerifyUtils_ != nullptr);

    // Setup: empty cache to ensure verification fails
    std::string testUri = "sharepreferences://com.example.test/data";

    std::unique_lock<std::shared_mutex> writeLock(obsVerifyUtils_->groupsIdMutex_);
    obsVerifyUtils_->groupsIdCache_.clear();
    writeLock.unlock();

    Uri uri(testUri);
    uint32_t listenerTokenId = TOKEN_TEST_1;
    uint32_t tokenId = TOKEN_TEST_2;
    int32_t userId = USER_TEST;

    bool result = obsVerifyUtils_->VerifyPermission(listenerTokenId, userId, uri, tokenId);

    // Should handle preferences scheme correctly
    // With different tokens and no group match, expect false
    EXPECT_FALSE(result);
}
} // namespace AAFwk
} // namespace OHOS
