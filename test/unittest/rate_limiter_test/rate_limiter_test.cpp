/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#include <thread>
#include <chrono>

#include "hilog_tag_wrapper.h"
#define private public
#include "rate_limiter.h"
#undef private

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AAFwk {
namespace {
constexpr int64_t EXTENSION_LIMIT_INTERVAL_MS = 1000;  // 1s
constexpr int32_t EXTENSION_MAX_LIMIT = 50;
constexpr int64_t REPORT_LIMIT_INTERVAL_MS = 5000; // 5s
constexpr int32_t REPORT_MAX_LIMIT = 1;
constexpr int32_t MODULAR_OBJECT_MAX_LIMIT = 20;
}
class RateLimiterTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void RateLimiterTest::SetUpTestCase(void)
{}

void RateLimiterTest::TearDownTestCase(void)
{}

void RateLimiterTest::SetUp()
{
    RateLimiter::GetInstance().extensionCallMap_.clear();
    RateLimiter::GetInstance().modularObjectCallMap_.clear();
}

void RateLimiterTest::TearDown()
{}

/**
 * @tc.number: CheckExtensionLimitTest_0100
 * @tc.desc: Test CheckExtensionLimit
 * @tc.type: FUNC
 */
HWTEST_F(RateLimiterTest, CheckExtensionLimitTest_0100, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "CheckExtensionLimitTest_0100 start.");

    auto &rateLimiter = RateLimiter::GetInstance();
    auto uid = 20010001;
    auto isLimit = rateLimiter.CheckExtensionLimit(uid);
    EXPECT_FALSE(isLimit.limited);

    TAG_LOGI(AAFwkTag::TEST, "CheckExtensionLimitTest_0100 end.");
}

/**
 * @tc.number: CheckExtensionLimitTest_0200
 * @tc.desc: Test CheckExtensionLimit
 * @tc.type: FUNC
 */
HWTEST_F(RateLimiterTest, CheckExtensionLimitTest_0200, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "CheckExtensionLimitTest_0200 start.");

    auto &rateLimiter = RateLimiter::GetInstance();
    auto uid = 20010001;
    for (int i = 0; i < EXTENSION_MAX_LIMIT; i++) {
        rateLimiter.CheckExtensionLimit(uid);
    }
    auto isLimit = rateLimiter.CheckExtensionLimit(uid);
    EXPECT_TRUE(isLimit.limited);

    TAG_LOGI(AAFwkTag::TEST, "CheckExtensionLimitTest_0200 end.");
}

/**
 * @tc.number: CheckReportLimitTest_0100
 * @tc.desc: Test CheckReportLimit
 * @tc.type: FUNC
 */
HWTEST_F(RateLimiterTest, CheckReportLimitTest_0100, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "CheckReportLimitTest_0100 start.");

    auto &rateLimiter = RateLimiter::GetInstance();
    auto uid = 20010001;
    auto isLimit = rateLimiter.CheckReportLimit(uid, 50);
    EXPECT_FALSE(isLimit);

    TAG_LOGI(AAFwkTag::TEST, "CheckReportLimitTest_0100 end.");
}

/**
 * @tc.number: CheckReportLimitTest_0200
 * @tc.desc: Test CheckReportLimit
 * @tc.type: FUNC
 */
HWTEST_F(RateLimiterTest, CheckReportLimitTest_0200, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "CheckReportLimitTest_0200 start.");

    auto &rateLimiter = RateLimiter::GetInstance();
    auto uid = 20010001;
    for (int i = 0; i < REPORT_MAX_LIMIT; i++) {
        rateLimiter.CheckReportLimit(uid, 50);
    }
    auto isLimit = rateLimiter.CheckReportLimit(uid, 50);
    EXPECT_TRUE(isLimit);

    TAG_LOGI(AAFwkTag::TEST, "CheckReportLimitTest_0200 end.");
}

/**
 * @tc.number: CleanCallMapTest_0100
 * @tc.desc: Test CleanCallMap
 * @tc.type: FUNC
 */
HWTEST_F(RateLimiterTest, CleanCallMapTest_0100, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "CleanCallMapTest_0100 start.");

    auto &rateLimiter = RateLimiter::GetInstance();
    auto uid1 = 20010001;
    auto uid2 = 20010002;

    rateLimiter.CheckExtensionLimit(uid1);
    rateLimiter.CheckExtensionLimit(uid1);
    rateLimiter.CheckExtensionLimit(uid1);
    std::this_thread::sleep_for(std::chrono::milliseconds(EXTENSION_LIMIT_INTERVAL_MS + 100));
    rateLimiter.CheckExtensionLimit(uid2);
    rateLimiter.CheckExtensionLimit(uid2);

    rateLimiter.lastCleanTimeMillis_ = 0;
    rateLimiter.CleanCallMap();
    auto mapSize = rateLimiter.extensionCallMap_.size();
    TAG_LOGI(AAFwkTag::TEST, "extensionCallMap_ size:%{public}zu", mapSize);
    EXPECT_EQ(mapSize, 1);

    TAG_LOGI(AAFwkTag::TEST, "CleanCallMapTest_0100 end.");
}

/**
 * @tc.number: CleanCallMapTest_0200
 * @tc.desc: Test CleanCallMap
 * @tc.type: FUNC
 */
HWTEST_F(RateLimiterTest, CleanCallMapTest_0200, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "CleanCallMapTest_0200 start.");

    auto &rateLimiter = RateLimiter::GetInstance();
    auto uid1 = 20010001;
    auto uid2 = 20010002;

    rateLimiter.CheckReportLimit(uid1, 50);
    std::this_thread::sleep_for(std::chrono::milliseconds(REPORT_LIMIT_INTERVAL_MS + 100));
    rateLimiter.CheckReportLimit(uid2, 50);

    rateLimiter.lastCleanTimeMillis_ = 0;
    rateLimiter.CleanCallMap();
    auto mapSize = rateLimiter.tierReportCallMap_.size();
    TAG_LOGI(AAFwkTag::TEST, "tierReportCallMap_ size:%{public}zu", mapSize);
    EXPECT_EQ(mapSize, 1);

    TAG_LOGI(AAFwkTag::TEST, "CleanCallMapTest_0200 end.");
}

/**
 * @tc.number: CheckModularObjectLimitTest_0100
 * @tc.desc: Test CheckModularObjectLimit under limit
 * @tc.type: FUNC
 */
HWTEST_F(RateLimiterTest, CheckModularObjectLimitTest_0100, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "CheckModularObjectLimitTest_0100 start.");

    auto &rateLimiter = RateLimiter::GetInstance();
    auto uid = 20010001;
    for (int i = 0; i < MODULAR_OBJECT_MAX_LIMIT; i++) {
        auto limited = rateLimiter.CheckModularObjectLimit(uid);
        EXPECT_FALSE(limited);
    }

    TAG_LOGI(AAFwkTag::TEST, "CheckModularObjectLimitTest_0100 end.");
}

/**
 * @tc.number: CheckModularObjectLimitTest_0200
 * @tc.desc: Test CheckModularObjectLimit over limit
 * @tc.type: FUNC
 */
HWTEST_F(RateLimiterTest, CheckModularObjectLimitTest_0200, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "CheckModularObjectLimitTest_0200 start.");

    auto &rateLimiter = RateLimiter::GetInstance();
    auto uid = 20010001;
    for (int i = 0; i < MODULAR_OBJECT_MAX_LIMIT; i++) {
        rateLimiter.CheckModularObjectLimit(uid);
    }
    auto limited = rateLimiter.CheckModularObjectLimit(uid);
    EXPECT_TRUE(limited);

    TAG_LOGI(AAFwkTag::TEST, "CheckModularObjectLimitTest_0200 end.");
}

/**
 * @tc.number: CheckModularObjectLimitTest_0300
 * @tc.desc: Test CheckModularObjectLimit sliding window
 * @tc.type: FUNC
 */
HWTEST_F(RateLimiterTest, CheckModularObjectLimitTest_0300, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "CheckModularObjectLimitTest_0300 start.");

    auto &rateLimiter = RateLimiter::GetInstance();
    auto uid = 20010001;
    for (int i = 0; i < MODULAR_OBJECT_MAX_LIMIT; i++) {
        rateLimiter.CheckModularObjectLimit(uid);
    }
    EXPECT_TRUE(rateLimiter.CheckModularObjectLimit(uid));

    std::this_thread::sleep_for(std::chrono::milliseconds(EXTENSION_LIMIT_INTERVAL_MS + 100));
    auto limited = rateLimiter.CheckModularObjectLimit(uid);
    EXPECT_FALSE(limited);

    TAG_LOGI(AAFwkTag::TEST, "CheckModularObjectLimitTest_0300 end.");
}

/**
 * @tc.number: CheckModularObjectLimitTest_0400
 * @tc.desc: Test CheckModularObjectLimit different UIDs are independent
 * @tc.type: FUNC
 */
HWTEST_F(RateLimiterTest, CheckModularObjectLimitTest_0400, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "CheckModularObjectLimitTest_0400 start.");

    auto &rateLimiter = RateLimiter::GetInstance();
    auto uid1 = 20010001;
    auto uid2 = 20010002;
    for (int i = 0; i < MODULAR_OBJECT_MAX_LIMIT; i++) {
        rateLimiter.CheckModularObjectLimit(uid1);
    }
    EXPECT_TRUE(rateLimiter.CheckModularObjectLimit(uid1));
    EXPECT_FALSE(rateLimiter.CheckModularObjectLimit(uid2));

    TAG_LOGI(AAFwkTag::TEST, "CheckModularObjectLimitTest_0400 end.");
}

/**
 * @tc.number: CleanCallMapTest_0300
 * @tc.desc: Test CleanCallMap cleans modularObjectCallMap_
 * @tc.type: FUNC
 */
HWTEST_F(RateLimiterTest, CleanCallMapTest_0300, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "CleanCallMapTest_0300 start.");

    auto &rateLimiter = RateLimiter::GetInstance();
    auto uid1 = 20010001;
    auto uid2 = 20010002;

    rateLimiter.CheckModularObjectLimit(uid1);
    rateLimiter.CheckModularObjectLimit(uid1);
    std::this_thread::sleep_for(std::chrono::milliseconds(EXTENSION_LIMIT_INTERVAL_MS + 100));
    rateLimiter.CheckModularObjectLimit(uid2);

    rateLimiter.lastCleanTimeMillis_ = 0;
    rateLimiter.CleanCallMap();
    auto mapSize = rateLimiter.modularObjectCallMap_.size();
    TAG_LOGI(AAFwkTag::TEST, "modularObjectCallMap_ size:%{public}zu", mapSize);
    EXPECT_EQ(mapSize, 1);

    TAG_LOGI(AAFwkTag::TEST, "CleanCallMapTest_0300 end.");
}
}  // namespace AAFwk
}  // namespace OHOS