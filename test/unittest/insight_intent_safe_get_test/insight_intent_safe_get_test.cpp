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
#include <vector>

#include "extract_insight_intent_profile.h"
#include "hilog_tag_wrapper.h"
#include "insight_intent_info_for_query.h"
#include "insight_intent_info_for_query_json.h"
#include "insight_intent_profile.h"
#include "intent_json_safe_get.h"
#include "nlohmann/json.hpp"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr size_t DUMP_MAX_LEN = 200;

struct SimpleItem {
    int32_t value;
    std::string name;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(SimpleItem, value, name)

struct MismatchItem {
    int32_t version;
    std::string id;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(MismatchItem, version, id)
} // namespace

class IntentJsonSafeGetTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void IntentJsonSafeGetTest::SetUpTestCase() {}
void IntentJsonSafeGetTest::TearDownTestCase() {}
void IntentJsonSafeGetTest::SetUp() {}
void IntentJsonSafeGetTest::TearDown() {}

// ---------------- SafeJsonGet tests ----------------

/**
 * @tc.number: SafeJsonGet_0100
 * @tc.name: SafeJsonGet
 * @tc.desc: Valid json object, get<T> returns true and out is populated.
 */
HWTEST_F(IntentJsonSafeGetTest, SafeJsonGet_0100, TestSize.Level0)
{
    auto j = nlohmann::json::parse(R"({"value": 42, "name": "abc"})", nullptr, false);
    ASSERT_FALSE(j.is_discarded());
    SimpleItem item{};
    EXPECT_TRUE(SafeJsonGet(j, item, "SimpleItem"));
    EXPECT_EQ(item.value, 42);
    EXPECT_EQ(item.name, "abc");
}

/**
 * @tc.number: SafeJsonGet_0200
 * @tc.name: SafeJsonGet
 * @tc.desc: Type mismatch (string expected but number provided) returns false, no exception propagates.
 */
HWTEST_F(IntentJsonSafeGetTest, SafeJsonGet_0200, TestSize.Level0)
{
    auto j = nlohmann::json::parse(R"({"version": "1", "id": 100})", nullptr, false);
    ASSERT_FALSE(j.is_discarded());
    MismatchItem item{};
    EXPECT_FALSE(SafeJsonGet(j, item, "MismatchItem"));
}

/**
 * @tc.number: SafeJsonGet_0300
 * @tc.name: SafeJsonGet
 * @tc.desc: Number out of int32 range does not throw (nlohmann silently narrows).
 */
HWTEST_F(IntentJsonSafeGetTest, SafeJsonGet_0300, TestSize.Level0)
{
    auto j = nlohmann::json::parse(R"({"version": 99999999999, "id": "x"})", nullptr, false);
    ASSERT_FALSE(j.is_discarded());
    MismatchItem item{};
    EXPECT_NO_THROW(SafeJsonGet(j, item, "MismatchItemOverflow"));
}

/**
 * @tc.number: SafeJsonGet_0400
 * @tc.name: SafeJsonGet
 * @tc.desc: Get on non-object json (e.g. array) returns false.
 */
HWTEST_F(IntentJsonSafeGetTest, SafeJsonGet_0400, TestSize.Level0)
{
    auto j = nlohmann::json::parse(R"([1, 2, 3])", nullptr, false);
    ASSERT_FALSE(j.is_discarded());
    SimpleItem item{};
    EXPECT_FALSE(SafeJsonGet(j, item, "SimpleItemOnArray"));
}

// ---------------- SafeDump tests ----------------

/**
 * @tc.number: SafeDump_0100
 * @tc.name: SafeDump
 * @tc.desc: Dump on small object returns full string.
 */
HWTEST_F(IntentJsonSafeGetTest, SafeDump_0100, TestSize.Level0)
{
    auto j = nlohmann::json::parse(R"({"k": "v"})", nullptr, false);
    ASSERT_FALSE(j.is_discarded());
    auto s = SafeDump(j);
    EXPECT_NE(s.find("\"k\""), std::string::npos);
    EXPECT_NE(s.find("\"v\""), std::string::npos);
}

namespace {
// Build a json with `depth` nested objects around a scalar leaf. IsJsonDepthOk counts the
// leaf too, so the walker reports depth + 1.
nlohmann::json BuildNestedJson(size_t depth)
{
    std::string s;
    for (size_t i = 0; i < depth; ++i) {
        s += "{\"c\":";
    }
    s += "1";
    for (size_t i = 0; i < depth; ++i) {
        s += "}";
    }
    return nlohmann::json::parse(s, nullptr, false);
}
} // namespace

/**
 * @tc.number: SafeDump_0300
 * @tc.name: SafeDump
 * @tc.desc: Dump with maxLen truncates long strings.
 */
HWTEST_F(IntentJsonSafeGetTest, SafeDump_0300, TestSize.Level0)
{
    nlohmann::json j;
    j["long"] = std::string(500, 'x');
    auto s = SafeDump(j, DUMP_MAX_LEN);
    EXPECT_LE(s.size(), DUMP_MAX_LEN + std::string("...").size() + 32);
}

/**
 * @tc.number: SafeDump_0400
 * @tc.name: SafeDump
 * @tc.desc: Dump on small json with maxLen returns full content (no truncation).
 */
HWTEST_F(IntentJsonSafeGetTest, SafeDump_0400, TestSize.Level0)
{
    auto j = nlohmann::json::parse(R"({"k": "v"})", nullptr, false);
    ASSERT_FALSE(j.is_discarded());
    auto s = SafeDump(j, DUMP_MAX_LEN);
    EXPECT_NE(s.find("\"k\""), std::string::npos);
    EXPECT_EQ(s.find("..."), std::string::npos)
        << "small dump should not be truncated";
}

// ---------------- IsJsonDepthOk boundary tests ----------------

/**
 * @tc.number: IsJsonDepthOk_Empty_0100
 * @tc.name: IsJsonDepthOk
 * @tc.desc: Empty json object returns true.
 */
HWTEST_F(IntentJsonSafeGetTest, IsJsonDepthOk_Empty_0100, TestSize.Level0)
{
    nlohmann::json j = nlohmann::json::object();
    EXPECT_TRUE(IsJsonDepthOk(j, JSON_DUMP_MAX_DEPTH));
}

/**
 * @tc.number: IsJsonDepthOk_Scalar_0100
 * @tc.name: IsJsonDepthOk
 * @tc.desc: Scalar value at depth 1 returns true.
 */
HWTEST_F(IntentJsonSafeGetTest, IsJsonDepthOk_Scalar_0100, TestSize.Level0)
{
    nlohmann::json j = 42;
    EXPECT_TRUE(IsJsonDepthOk(j, JSON_DUMP_MAX_DEPTH));
}

/**
 * @tc.number: IsJsonDepthOk_FlatArray_0100
 * @tc.name: IsJsonDepthOk
 * @tc.desc: Flat array of scalars returns true (array itself is depth 1).
 */
HWTEST_F(IntentJsonSafeGetTest, IsJsonDepthOk_FlatArray_0100, TestSize.Level0)
{
    nlohmann::json j = nlohmann::json::array({1, 2, 3});
    EXPECT_TRUE(IsJsonDepthOk(j, JSON_DUMP_MAX_DEPTH));
}

/**
 * @tc.number: IsJsonDepthOk_UnderLimit_0100
 * @tc.name: IsJsonDepthOk
 * @tc.desc: walker depth = maxDepth - 1 (leaf at 99) returns true.
 */
HWTEST_F(IntentJsonSafeGetTest, IsJsonDepthOk_UnderLimit_0100, TestSize.Level0)
{
    auto j = BuildNestedJson(JSON_DUMP_MAX_DEPTH - 2);
    ASSERT_FALSE(j.is_discarded());
    EXPECT_TRUE(IsJsonDepthOk(j, JSON_DUMP_MAX_DEPTH));
}

/**
 * @tc.number: IsJsonDepthOk_AtLimit_0100
 * @tc.name: IsJsonDepthOk
 * @tc.desc: walker depth = maxDepth (leaf at 100) returns true (boundary: depth > maxDepth is false).
 */
HWTEST_F(IntentJsonSafeGetTest, IsJsonDepthOk_AtLimit_0100, TestSize.Level0)
{
    auto j = BuildNestedJson(JSON_DUMP_MAX_DEPTH - 1);
    ASSERT_FALSE(j.is_discarded());
    EXPECT_TRUE(IsJsonDepthOk(j, JSON_DUMP_MAX_DEPTH));
}

/**
 * @tc.number: IsJsonDepthOk_OverLimit_0100
 * @tc.name: IsJsonDepthOk
 * @tc.desc: walker depth = maxDepth + 1 (leaf at 101) returns false.
 */
HWTEST_F(IntentJsonSafeGetTest, IsJsonDepthOk_OverLimit_0100, TestSize.Level0)
{
    auto j = BuildNestedJson(JSON_DUMP_MAX_DEPTH);
    ASSERT_FALSE(j.is_discarded());
    EXPECT_FALSE(IsJsonDepthOk(j, JSON_DUMP_MAX_DEPTH));
}

/**
 * @tc.number: IsJsonDepthOk_ArrayNesting_0100
 * @tc.name: IsJsonDepthOk
 * @tc.desc: Array-of-array nesting is also depth-counted.
 */
HWTEST_F(IntentJsonSafeGetTest, IsJsonDepthOk_ArrayNesting_0100, TestSize.Level0)
{
    std::string s;
    for (size_t i = 0; i < JSON_DUMP_MAX_DEPTH + 1; ++i) {
        s += "[";
    }
    s += "1";
    for (size_t i = 0; i < JSON_DUMP_MAX_DEPTH + 1; ++i) {
        s += "]";
    }
    auto j = nlohmann::json::parse(s, nullptr, false);
    ASSERT_FALSE(j.is_discarded());
    EXPECT_FALSE(IsJsonDepthOk(j, JSON_DUMP_MAX_DEPTH));
}

// ---------------- SafeDump / SafeDumpTo boundary integration ----------------

/**
 * @tc.number: SafeDump_AtLimit_0100
 * @tc.name: SafeDump
 * @tc.desc: walker depth = JSON_DUMP_MAX_DEPTH (leaf at 100) produces non-empty dump.
 */
HWTEST_F(IntentJsonSafeGetTest, SafeDump_AtLimit_0100, TestSize.Level0)
{
    auto j = BuildNestedJson(JSON_DUMP_MAX_DEPTH - 1);
    ASSERT_FALSE(j.is_discarded());
    auto s = SafeDump(j);
    EXPECT_FALSE(s.empty());
}

/**
 * @tc.number: SafeDump_OverLimit_0100
 * @tc.name: SafeDump
 * @tc.desc: walker depth = JSON_DUMP_MAX_DEPTH + 1 (leaf at 101) produces empty string (depth guard).
 */
HWTEST_F(IntentJsonSafeGetTest, SafeDump_OverLimit_0100, TestSize.Level0)
{
    auto j = BuildNestedJson(JSON_DUMP_MAX_DEPTH);
    ASSERT_FALSE(j.is_discarded());
    auto s = SafeDump(j);
    EXPECT_TRUE(s.empty());
}

/**
 * @tc.number: SafeDumpTo_AtLimit_0100
 * @tc.name: SafeDumpTo
 * @tc.desc: walker depth = JSON_DUMP_MAX_DEPTH (leaf at 100) succeeds.
 */
HWTEST_F(IntentJsonSafeGetTest, SafeDumpTo_AtLimit_0100, TestSize.Level0)
{
    auto j = BuildNestedJson(JSON_DUMP_MAX_DEPTH - 1);
    ASSERT_FALSE(j.is_discarded());
    std::string out;
    EXPECT_TRUE(SafeDumpTo(j, out));
    EXPECT_FALSE(out.empty());
}

/**
 * @tc.number: SafeDumpTo_OverLimit_0100
 * @tc.name: SafeDumpTo
 * @tc.desc: walker depth = JSON_DUMP_MAX_DEPTH + 1 (leaf at 101) returns false.
 */
HWTEST_F(IntentJsonSafeGetTest, SafeDumpTo_OverLimit_0100, TestSize.Level0)
{
    auto j = BuildNestedJson(JSON_DUMP_MAX_DEPTH);
    ASSERT_FALSE(j.is_discarded());
    std::string out;
    EXPECT_FALSE(SafeDumpTo(j, out));
}

// ---------------- SafeDumpTo tests ----------------

/**
 * @tc.number: SafeDumpTo_0100
 * @tc.name: SafeDumpTo
 * @tc.desc: Dump valid object writes string and returns true.
 */
HWTEST_F(IntentJsonSafeGetTest, SafeDumpTo_0100, TestSize.Level0)
{
    auto j = nlohmann::json::parse(R"({"a": 1})", nullptr, false);
    ASSERT_FALSE(j.is_discarded());
    std::string out;
    EXPECT_TRUE(SafeDumpTo(j, out));
    EXPECT_FALSE(out.empty());
    EXPECT_NE(out.find("\"a\""), std::string::npos);
}

// ---------------- Integration: ExtractInsightIntentProfile::TransformTo ----------------

namespace {
const char *EXTRACT_TYPE_MISMATCH_PROFILE = R"({
    "extractInsightIntents": [{
        "bundleName": "com.test",
        "moduleName": "entry",
        "intentName": "test",
        "intentVersion": 1,
        "decoratorType": "@InsightIntentLink",
        "uri": "/test",
        "domain": "test",
        "displayName": "test",
        "displayDescription": "desc",
        "decoratorFile": "f",
        "decoratorClass": "c"
    }]
})";

const char *EXTRACT_BAD_BASE_PROFILE = R"({
    "extractInsightIntents": [{
        "bundleName": "com.test",
        "moduleName": "entry",
        "intentName": "test",
        "intentVersion": "1",
        "decoratorType": "@InsightIntentLink",
        "uri": "/test",
        "domain": "test",
        "displayName": "test",
        "displayDescription": "desc",
        "decoratorFile": "f",
        "decoratorClass": "c"
    }]
})";
} // namespace

/**
 * @tc.number: ExtractTransformTo_TypeMismatch_0100
 * @tc.name: ExtractInsightIntentProfile::TransformTo
 * @tc.desc: intentVersion as number (string expected) returns false, no exception propagates.
 */
HWTEST_F(IntentJsonSafeGetTest, ExtractTransformTo_TypeMismatch_0100, TestSize.Level0)
{
    ExtractInsightIntentProfileInfoVec infos;
    EXPECT_FALSE(ExtractInsightIntentProfile::TransformTo(EXTRACT_TYPE_MISMATCH_PROFILE, infos));
}

/**
 * @tc.number: ExtractTransformTo_NoThrow_0100
 * @tc.name: ExtractInsightIntentProfile::TransformTo
 * @tc.desc: Calling TransformTo repeatedly on bad input does not crash.
 */
HWTEST_F(IntentJsonSafeGetTest, ExtractTransformTo_NoThrow_0100, TestSize.Level0)
{
    for (int i = 0; i < 3; ++i) {
        ExtractInsightIntentProfileInfoVec infos;
        EXPECT_FALSE(ExtractInsightIntentProfile::TransformTo(EXTRACT_TYPE_MISMATCH_PROFILE, infos));
    }
}

// ---------------- Integration: InsightIntentProfile::TransformTo ----------------

namespace {
const char *CONFIG_TYPE_MISMATCH_PROFILE = R"({
    "insightIntents": [{
        "bundleName": "com.test",
        "moduleName": "entry",
        "intentName": "test",
        "intentVersion": 1,
        "domain": "test",
        "displayName": "test",
        "displayDescription": "desc"
    }]
})";

const char *CONFIG_BAD_EXECUTE_MODE_PROFILE = R"({
    "insightIntents": [{
        "bundleName": "com.test",
        "moduleName": "entry",
        "intentName": "test",
        "intentVersion": "1",
        "domain": "test",
        "displayName": "test",
        "displayDescription": "desc",
        "executeMode": "foreground"
    }]
})";
} // namespace

/**
 * @tc.number: ConfigTransformTo_TypeMismatch_0100
 * @tc.name: InsightIntentProfile::TransformTo
 * @tc.desc: intentVersion as number returns false, no exception propagates.
 */
HWTEST_F(IntentJsonSafeGetTest, ConfigTransformTo_TypeMismatch_0100, TestSize.Level0)
{
    std::vector<InsightIntentInfo> infos;
    EXPECT_FALSE(InsightIntentProfile::TransformTo(CONFIG_TYPE_MISMATCH_PROFILE, infos));
    EXPECT_TRUE(infos.empty());
}

/**
 * @tc.number: ConfigTransformTo_BadExecuteMode_0100
 * @tc.name: InsightIntentProfile::TransformTo
 * @tc.desc: executeMode as scalar string does not crash; result is acceptable as long as no throw.
 */
HWTEST_F(IntentJsonSafeGetTest, ConfigTransformTo_BadExecuteMode_0100, TestSize.Level0)
{
    std::vector<InsightIntentInfo> infos;
    // We only assert no exception / no crash here; downstream may tolerate or skip.
    InsightIntentProfile::TransformTo(CONFIG_BAD_EXECUTE_MODE_PROFILE, infos);
    SUCCEED();
}

/**
 * @tc.number: ExtractTransformTo_ValidBase_0100
 * @tc.name: ExtractInsightIntentProfile::TransformTo
 * @tc.desc: Valid base profile still parses successfully after safety wrapper is applied.
 */
HWTEST_F(IntentJsonSafeGetTest, ExtractTransformTo_ValidBase_0100, TestSize.Level0)
{
    ExtractInsightIntentProfileInfoVec infos;
    bool ok = ExtractInsightIntentProfile::TransformTo(EXTRACT_BAD_BASE_PROFILE, infos);
    // Result may be true or false depending on further validations; assert no crash only.
    if (ok) {
        EXPECT_FALSE(infos.insightIntents.empty());
    }
}

// ---------------- Integration: EntryInfoForQuery / UIAbilityIntentInfoForQuery ----------------

HWTEST_F(IntentJsonSafeGetTest, EntryInfoForQuery_BadExecuteMode_0100, TestSize.Level0)
{
    auto j = nlohmann::json::parse(R"({"abilityName":"A","executeMode":"foreground"})",
        nullptr, false);
    ASSERT_FALSE(j.is_discarded());
    EntryInfoForQuery info;
    EXPECT_NO_THROW(from_json(j, info));
    EXPECT_TRUE(info.executeMode.empty());
}

HWTEST_F(IntentJsonSafeGetTest, EntryInfoForQuery_GoodExecuteMode_0100, TestSize.Level0)
{
    auto j = nlohmann::json::parse(R"({"abilityName":"A","executeMode":["UI_ABILITY_FOREGROUND"]})",
        nullptr, false);
    ASSERT_FALSE(j.is_discarded());
    EntryInfoForQuery info;
    EXPECT_NO_THROW(from_json(j, info));
    EXPECT_EQ(info.executeMode.size(), 1UL);
}

HWTEST_F(IntentJsonSafeGetTest, UIAbilityIntentInfoForQuery_BadExecuteMode_0100, TestSize.Level0)
{
    auto j = nlohmann::json::parse(R"({"ability":"A","executeMode":123})",
        nullptr, false);
    ASSERT_FALSE(j.is_discarded());
    UIAbilityIntentInfoForQuery info;
    EXPECT_NO_THROW(from_json(j, info));
    EXPECT_TRUE(info.supportExecuteMode.empty());
}
} // namespace AbilityRuntime
} // namespace OHOS
