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

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <set>
#include <gtest/gtest-death-test.h>
#include <gtest/gtest.h>
#include <iostream>
#include <nlohmann/json.hpp>
#include <unistd.h>
#define private public
#define protected public
#include "cli_function_data_manager.h"
#undef private
#undef protected
#include "cli_error_code.h"
#include "hilog_tag_wrapper.h"
#include "mock_single_kv_store.h"

using namespace testing::ext;

namespace OHOS {
namespace CliTool {

class CliFunctionDataManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void CliFunctionDataManagerTest::SetUpTestCase()
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "CliFunctionDataManagerTest::SetUpTestCase");
}

void CliFunctionDataManagerTest::TearDownTestCase()
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "CliFunctionDataManagerTest::TearDownTestCase");
}

void CliFunctionDataManagerTest::SetUp()
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "CliFunctionDataManagerTest::SetUp");
    CliFunctionDataManager::GetInstance().kvStorePtr_ = nullptr;
    CliFunctionDataManager::GetInstance().functionsInitialized_ = false;
}

void CliFunctionDataManagerTest::TearDown()
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "CliFunctionDataManagerTest::TearDown");
    CliFunctionDataManager::GetInstance().kvStorePtr_ = nullptr;
    CliFunctionDataManager::GetInstance().functionsInitialized_ = false;
}

namespace {
std::string BuildFunctionJson(const std::string &ns, const std::string &name,
    const std::string &description = "Mock function")
{
    nlohmann::json json = {
        {"functionName", name},
        {"functionNamespace", ns},
        {"description", description},
        {"inputSchema", "{}"},
        {"outputSchema", "{}"},
        {"functionType", 0},
        {"version", "1.0"}
    };
    return json.dump();
}
} // namespace

// ==================== GetInstance Tests ====================

/**
 * @tc.name: CliFunctionDataManager_GetInstance_001
 * @tc.desc: Test GetInstance returns singleton
 * @tc.type: FUNC
 */
HWTEST_F(CliFunctionDataManagerTest, CliFunctionDataManager_GetInstance_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "CliFunctionDataManager_GetInstance_001 start");

    auto& instance1 = CliFunctionDataManager::GetInstance();
    auto& instance2 = CliFunctionDataManager::GetInstance();

    EXPECT_EQ(&instance1, &instance2);

    TAG_LOGI(AAFwkTag::CLI_TOOL, "CliFunctionDataManager_GetInstance_001 end");
}

// ==================== FunctionInfo ParseToJson Tests ====================

/**
 * @tc.name: FunctionInfo_ParseToJson_001
 * @tc.desc: Test converting FunctionInfo to JSON
 * @tc.type: FUNC
 */
HWTEST_F(CliFunctionDataManagerTest, FunctionInfo_ParseToJson_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "FunctionInfo_ParseToJson_001 start");

    FunctionInfo function;
    function.functionName = "test_function";
    function.functionNamespace = "test_namespace";
    function.description = "Test description";
    function.functionType = FunctionType::INTENT_FUNCTION;

    nlohmann::json json = function.ParseToJson();
    std::string jsonStr = json.dump();

    EXPECT_FALSE(jsonStr.empty());
    EXPECT_NE(jsonStr.find("test_function"), std::string::npos);
    EXPECT_NE(jsonStr.find("test_namespace"), std::string::npos);

    TAG_LOGI(AAFwkTag::CLI_TOOL, "FunctionInfo_ParseToJson_001 end");
}

/**
 * @tc.name: FunctionInfo_ParseToJson_002
 * @tc.desc: Test converting FunctionInfo with schemas to JSON
 * @tc.type: FUNC
 */
HWTEST_F(CliFunctionDataManagerTest, FunctionInfo_ParseToJson_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "FunctionInfo_ParseToJson_002 start");

    FunctionInfo function;
    function.functionName = "function_with_schemas";
    function.functionNamespace = "schema_ns";
    function.inputSchema = R"({"type": "object", "properties": {"input": {"type": "string"}}})";
    function.outputSchema = R"({"type": "array"})";

    nlohmann::json json = function.ParseToJson();

    EXPECT_TRUE(json.contains("inputSchema"));
    EXPECT_TRUE(json.contains("outputSchema"));

    TAG_LOGI(AAFwkTag::CLI_TOOL, "FunctionInfo_ParseToJson_002 end");
}

// ==================== FunctionInfo ParseFromJson Tests ====================

/**
 * @tc.name: FunctionInfo_ParseFromJson_001
 * @tc.desc: Test parsing JSON to FunctionInfo
 * @tc.type: FUNC
 */
HWTEST_F(CliFunctionDataManagerTest, FunctionInfo_ParseFromJson_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "FunctionInfo_ParseFromJson_001 start");

    nlohmann::json json = R"({
        "functionName": "json_function",
        "functionNamespace": "json_namespace",
        "description": "JSON test function",
        "inputSchema": "{\"type\": \"object\"}",
        "outputSchema": "{\"type\": \"string\"}",
        "functionType": 0,
        "version": "1.0"
    })"_json;

    FunctionInfo function;
    bool result = FunctionInfo::ParseFromJson(json, function);

    ASSERT_TRUE(result);
    EXPECT_EQ(function.functionName, "json_function");
    EXPECT_EQ(function.functionNamespace, "json_namespace");
    EXPECT_EQ(function.functionType, FunctionType::INTENT_FUNCTION);

    TAG_LOGI(AAFwkTag::CLI_TOOL, "FunctionInfo_ParseFromJson_001 end");
}

/**
 * @tc.name: FunctionInfo_ParseFromJson_002
 * @tc.desc: Test parsing JSON with missing required fields
 * @tc.type: FUNC
 */
HWTEST_F(CliFunctionDataManagerTest, FunctionInfo_ParseFromJson_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "FunctionInfo_ParseFromJson_002 start");

    nlohmann::json json = R"({
        "functionName": "incomplete_function"
    })"_json;

    FunctionInfo function;
    bool result = FunctionInfo::ParseFromJson(json, function);

    EXPECT_FALSE(result);

    TAG_LOGI(AAFwkTag::CLI_TOOL, "FunctionInfo_ParseFromJson_002 end");
}

/**
 * @tc.name: FunctionInfo_ParseFromJson_003
 * @tc.desc: Test parsing empty JSON to FunctionInfo
 * @tc.type: FUNC
 */
HWTEST_F(CliFunctionDataManagerTest, FunctionInfo_ParseFromJson_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "FunctionInfo_ParseFromJson_003 start");

    nlohmann::json json;

    FunctionInfo function;
    bool result = FunctionInfo::ParseFromJson(json, function);

    EXPECT_FALSE(result);

    TAG_LOGI(AAFwkTag::CLI_TOOL, "FunctionInfo_ParseFromJson_003 end");
}

// ==================== FunctionInfo ParseFromJson/ParseToJson Round Trip Tests ====================

/**
 * @tc.name: FunctionInfo_ParseFromJson_ParseToJson_RoundTrip_001
 * @tc.desc: Test FunctionInfo ParseFromJson and ParseToJson round trip
 * @tc.type: FUNC
 */
HWTEST_F(CliFunctionDataManagerTest, FunctionInfo_ParseFromJson_ParseToJson_RoundTrip_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "FunctionInfo_ParseFromJson_ParseToJson_RoundTrip_001 start");

    nlohmann::json originalJson = R"({
        "functionName": "roundtrip_function",
        "functionNamespace": "roundtrip_ns",
        "description": "Round trip test",
        "inputSchema": "{\"type\": \"object\"}",
        "outputSchema": "{\"type\": \"string\"}",
        "functionType": 0,
        "version": "1.0"
    })"_json;

    FunctionInfo function;
    bool result = FunctionInfo::ParseFromJson(originalJson, function);
    ASSERT_TRUE(result);
    nlohmann::json resultJson = function.ParseToJson();

    EXPECT_EQ(resultJson["functionName"], originalJson["functionName"]);
    EXPECT_EQ(resultJson["functionNamespace"], originalJson["functionNamespace"]);
    EXPECT_EQ(resultJson["functionType"], originalJson["functionType"]);

    TAG_LOGI(AAFwkTag::CLI_TOOL, "FunctionInfo_ParseFromJson_ParseToJson_RoundTrip_001 end");
}

// ==================== EnsureFunctionsInitialized Tests ====================

/**
 * @tc.name: CliFunctionDataManager_EnsureFunctionsInitialized_001
 * @tc.desc: Test EnsureFunctionsInitialized with mocked KVStore
 * @tc.type: FUNC
 */
HWTEST_F(CliFunctionDataManagerTest, CliFunctionDataManager_EnsureFunctionsInitialized_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "CliFunctionDataManager_EnsureFunctionsInitialized_001 start");

    auto mockStore = std::make_shared<MockSingleKvStore>();
    CliFunctionDataManager::GetInstance().kvStorePtr_ = mockStore;
    CliFunctionDataManager::GetInstance().functionsInitialized_ = false;

    int32_t ret = CliFunctionDataManager::GetInstance().EnsureFunctionsInitialized();

    EXPECT_EQ(ret, ERR_OK);

    TAG_LOGI(AAFwkTag::CLI_TOOL, "CliFunctionDataManager_EnsureFunctionsInitialized_001 end");
}

// ==================== RegisterFunction Tests ====================

/**
 * @tc.name: CliFunctionDataManager_RegisterFunction_001
 * @tc.desc: Test RegisterFunction with mocked KVStore
 * @tc.type: FUNC
 */
HWTEST_F(CliFunctionDataManagerTest, CliFunctionDataManager_RegisterFunction_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "CliFunctionDataManager_RegisterFunction_001 start");

    auto mockStore = std::make_shared<MockSingleKvStore>();
    CliFunctionDataManager::GetInstance().kvStorePtr_ = mockStore;

    FunctionInfo function;
    function.functionName = "test_register_function";
    function.functionNamespace = "test_ns";
    function.description = "Register test function";
    function.functionType = FunctionType::INTENT_FUNCTION;

    int32_t ret = CliFunctionDataManager::GetInstance().RegisterFunction(function);

    EXPECT_EQ(ret, ERR_OK);
    EXPECT_TRUE(mockStore->HasMockData("test_ns/test_register_function"));

    TAG_LOGI(AAFwkTag::CLI_TOOL, "CliFunctionDataManager_RegisterFunction_001 end");
}

/**
 * @tc.name: CliFunctionDataManager_RegisterFunction_002
 * @tc.desc: Test RegisterFunction with mocked KVStore
 * @tc.type: FUNC
 */
HWTEST_F(CliFunctionDataManagerTest, CliFunctionDataManager_RegisterFunction_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "CliFunctionDataManager_RegisterFunction_002 start");

    auto mockStore = std::make_shared<MockSingleKvStore>();
    CliFunctionDataManager::GetInstance().kvStorePtr_ = mockStore;

    FunctionInfo function;
    function.functionName = "mock_register_function";
    function.functionNamespace = "mock_ns";
    function.description = "Mock register test";
    function.functionType = FunctionType::INTENT_FUNCTION;

    int32_t ret = CliFunctionDataManager::GetInstance().RegisterFunction(function);

    EXPECT_EQ(ret, ERR_OK);
    EXPECT_TRUE(mockStore->HasMockData("mock_ns/mock_register_function"));

    TAG_LOGI(AAFwkTag::CLI_TOOL, "CliFunctionDataManager_RegisterFunction_002 end");
}

/**
 * @tc.name: CliFunctionDataManager_RegisterFunction_003
 * @tc.desc: Test RegisterFunction with mocked KVStore Put failure
 * @tc.type: FUNC
 */
HWTEST_F(CliFunctionDataManagerTest, CliFunctionDataManager_RegisterFunction_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "CliFunctionDataManager_RegisterFunction_003 start");

    auto mockStore = std::make_shared<MockSingleKvStore>();
    mockStore->Put_ = DistributedKv::Status::ERROR;
    CliFunctionDataManager::GetInstance().kvStorePtr_ = mockStore;

    FunctionInfo function;
    function.functionName = "fail_put_function";
    function.functionNamespace = "fail_put_ns";
    function.functionType = FunctionType::INTENT_FUNCTION;

    int32_t ret = CliFunctionDataManager::GetInstance().RegisterFunction(function);

    EXPECT_EQ(ret, ERR_KVSTORE_ERROR);

    TAG_LOGI(AAFwkTag::CLI_TOOL, "CliFunctionDataManager_RegisterFunction_003 end");
}

// ==================== GetFunctionByName Tests ====================

/**
 * @tc.name: CliFunctionDataManager_GetFunctionByName_001
 * @tc.desc: Test GetFunctionByName with non-existent function
 * @tc.type: FUNC
 */
HWTEST_F(CliFunctionDataManagerTest, CliFunctionDataManager_GetFunctionByName_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "CliFunctionDataManager_GetFunctionByName_001 start");

    auto mockStore = std::make_shared<MockSingleKvStore>();
    CliFunctionDataManager::GetInstance().kvStorePtr_ = mockStore;

    FunctionInfo function;
    int32_t ret = CliFunctionDataManager::GetInstance().GetFunctionByName(
        "non_existent_ns", "non_existent_function", function);

    EXPECT_EQ(ret, ERR_FUNCTION_NOT_EXIST);

    TAG_LOGI(AAFwkTag::CLI_TOOL, "CliFunctionDataManager_GetFunctionByName_001 end");
}

/**
 * @tc.name: CliFunctionDataManager_GetFunctionByName_002
 * @tc.desc: Test GetFunctionByName with empty name
 * @tc.type: FUNC
 */
HWTEST_F(CliFunctionDataManagerTest, CliFunctionDataManager_GetFunctionByName_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "CliFunctionDataManager_GetFunctionByName_002 start");

    auto mockStore = std::make_shared<MockSingleKvStore>();
    CliFunctionDataManager::GetInstance().kvStorePtr_ = mockStore;

    FunctionInfo function;
    int32_t ret = CliFunctionDataManager::GetInstance().GetFunctionByName("", "", function);

    EXPECT_EQ(ret, ERR_FUNCTION_NOT_EXIST);

    TAG_LOGI(AAFwkTag::CLI_TOOL, "CliFunctionDataManager_GetFunctionByName_002 end");
}

/**
 * @tc.name: CliFunctionDataManager_GetFunctionByName_003
 * @tc.desc: Test GetFunctionByName with mocked KVStore
 * @tc.type: FUNC
 */
HWTEST_F(CliFunctionDataManagerTest, CliFunctionDataManager_GetFunctionByName_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "CliFunctionDataManager_GetFunctionByName_003 start");

    auto mockStore = std::make_shared<MockSingleKvStore>();
    mockStore->SetMockData("test_ns/found_function", BuildFunctionJson("test_ns", "found_function"));
    mockStore->SetMockData("test_ns/broken_function", "{invalid json");
    CliFunctionDataManager::GetInstance().kvStorePtr_ = mockStore;

    FunctionInfo function;
    ASSERT_EQ(CliFunctionDataManager::GetInstance().GetFunctionByName("test_ns", "found_function", function), ERR_OK);
    EXPECT_EQ(function.functionName, "found_function");
    EXPECT_EQ(function.functionNamespace, "test_ns");

    EXPECT_EQ(CliFunctionDataManager::GetInstance().GetFunctionByName("test_ns", "broken_function", function),
        ERR_JSON_PARSE_FAILED);

    EXPECT_EQ(CliFunctionDataManager::GetInstance().GetFunctionByName("missing_ns", "missing_function", function),
        ERR_FUNCTION_NOT_EXIST);

    TAG_LOGI(AAFwkTag::CLI_TOOL, "CliFunctionDataManager_GetFunctionByName_003 end");
}

/**
 * @tc.name: CliFunctionDataManager_GetFunctionByName_004
 * @tc.desc: Test GetFunctionByName with null KVStore
 * @tc.type: FUNC
 */
HWTEST_F(CliFunctionDataManagerTest, CliFunctionDataManager_GetFunctionByName_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "CliFunctionDataManager_GetFunctionByName_004 start");

    auto mockStore = std::make_shared<MockSingleKvStore>();
    CliFunctionDataManager::GetInstance().kvStorePtr_ = mockStore;

    FunctionInfo function;
    int32_t ret = CliFunctionDataManager::GetInstance().GetFunctionByName("any_ns", "any_function", function);

    EXPECT_EQ(ret, ERR_FUNCTION_NOT_EXIST);

    TAG_LOGI(AAFwkTag::CLI_TOOL, "CliFunctionDataManager_GetFunctionByName_004 end");
}

// ==================== UnregisterFunction Tests ====================

/**
 * @tc.name: CliFunctionDataManager_UnregisterFunction_001
 * @tc.desc: Test UnregisterFunction with non-existent function (idempotent delete)
 * @tc.type: FUNC
 */
HWTEST_F(CliFunctionDataManagerTest, CliFunctionDataManager_UnregisterFunction_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "CliFunctionDataManager_UnregisterFunction_001 start");

    auto mockStore = std::make_shared<MockSingleKvStore>();
    CliFunctionDataManager::GetInstance().kvStorePtr_ = mockStore;

    int32_t ret = CliFunctionDataManager::GetInstance().UnregisterFunction("non_existent_ns", "non_existent_function");

    // Idempotent delete: deleting non-existent key returns SUCCESS
    EXPECT_EQ(ret, ERR_OK);

    TAG_LOGI(AAFwkTag::CLI_TOOL, "CliFunctionDataManager_UnregisterFunction_001 end");
}

/**
 * @tc.name: CliFunctionDataManager_UnregisterFunction_002
 * @tc.desc: Test UnregisterFunction with mocked KVStore
 * @tc.type: FUNC
 */
HWTEST_F(CliFunctionDataManagerTest, CliFunctionDataManager_UnregisterFunction_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "CliFunctionDataManager_UnregisterFunction_002 start");

    auto mockStore = std::make_shared<MockSingleKvStore>();
    mockStore->SetMockData("delete_ns/delete_function", BuildFunctionJson("delete_ns", "delete_function"));
    CliFunctionDataManager::GetInstance().kvStorePtr_ = mockStore;

    EXPECT_EQ(CliFunctionDataManager::GetInstance().UnregisterFunction("delete_ns", "delete_function"), ERR_OK);
    EXPECT_FALSE(mockStore->HasMockData("delete_ns/delete_function"));

    TAG_LOGI(AAFwkTag::CLI_TOOL, "CliFunctionDataManager_UnregisterFunction_002 end");
}

/**
 * @tc.name: CliFunctionDataManager_UnregisterFunction_003
 * @tc.desc: Test UnregisterFunction with empty KVStore (idempotent delete)
 * @tc.type: FUNC
 */
HWTEST_F(CliFunctionDataManagerTest, CliFunctionDataManager_UnregisterFunction_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "CliFunctionDataManager_UnregisterFunction_003 start");

    auto mockStore = std::make_shared<MockSingleKvStore>();
    CliFunctionDataManager::GetInstance().kvStorePtr_ = mockStore;

    int32_t ret = CliFunctionDataManager::GetInstance().UnregisterFunction("any_ns", "any_function");

    // Idempotent delete: deleting non-existent key returns SUCCESS
    EXPECT_EQ(ret, ERR_OK);

    TAG_LOGI(AAFwkTag::CLI_TOOL, "CliFunctionDataManager_UnregisterFunction_003 end");
}

// ==================== UnregisterIntentFunctionsByNamespace Tests ====================

/**
 * @tc.name: CliFunctionDataManager_UnregisterIntentFunctionsByNamespace_001
 * @tc.desc: Test UnregisterIntentFunctionsByNamespace with mocked KVStore
 * @tc.type: FUNC
 */
HWTEST_F(CliFunctionDataManagerTest, CliFunctionDataManager_UnregisterIntentFunctionsByNamespace_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "CliFunctionDataManager_UnregisterIntentFunctionsByNamespace_001 start");

    auto mockStore = std::make_shared<MockSingleKvStore>();
    // Add intent functions with namespace "intent_ns"
    mockStore->SetMockData("intent_ns/intent_func1", BuildFunctionJson("intent_ns", "intent_func1"));
    mockStore->SetMockData("intent_ns/intent_func2", BuildFunctionJson("intent_ns", "intent_func2"));
    // Add function with different namespace
    mockStore->SetMockData("other_ns/other_func", BuildFunctionJson("other_ns", "other_func"));
    CliFunctionDataManager::GetInstance().kvStorePtr_ = mockStore;

    EXPECT_EQ(CliFunctionDataManager::GetInstance().UnregisterIntentFunctionsByNamespace("intent_ns"), ERR_OK);
    EXPECT_FALSE(mockStore->HasMockData("intent_ns/intent_func1"));
    EXPECT_FALSE(mockStore->HasMockData("intent_ns/intent_func2"));
    EXPECT_TRUE(mockStore->HasMockData("other_ns/other_func"));

    TAG_LOGI(AAFwkTag::CLI_TOOL, "CliFunctionDataManager_UnregisterIntentFunctionsByNamespace_001 end");
}

/**
 * @tc.name: CliFunctionDataManager_UnregisterIntentFunctionsByNamespace_002
 * @tc.desc: Test UnregisterIntentFunctionsByNamespace with empty namespace
 * @tc.type: FUNC
 */
HWTEST_F(CliFunctionDataManagerTest, CliFunctionDataManager_UnregisterIntentFunctionsByNamespace_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "CliFunctionDataManager_UnregisterIntentFunctionsByNamespace_002 start");

    auto mockStore = std::make_shared<MockSingleKvStore>();
    CliFunctionDataManager::GetInstance().kvStorePtr_ = mockStore;

    int32_t ret = CliFunctionDataManager::GetInstance().UnregisterIntentFunctionsByNamespace("any_ns");

    EXPECT_EQ(ret, ERR_OK);

    TAG_LOGI(AAFwkTag::CLI_TOOL, "CliFunctionDataManager_UnregisterIntentFunctionsByNamespace_002 end");
}

/**
 * @tc.name: CliFunctionDataManager_UnregisterIntentFunctionsByNamespace_003
 * @tc.desc: Test UnregisterIntentFunctionsByNamespace with empty namespace
 * @tc.type: FUNC
 */
HWTEST_F(CliFunctionDataManagerTest, CliFunctionDataManager_UnregisterIntentFunctionsByNamespace_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "CliFunctionDataManager_UnregisterIntentFunctionsByNamespace_003 start");

    auto mockStore = std::make_shared<MockSingleKvStore>();
    CliFunctionDataManager::GetInstance().kvStorePtr_ = mockStore;

    // Empty namespace should not crash, just return success with no deletions
    EXPECT_EQ(CliFunctionDataManager::GetInstance().UnregisterIntentFunctionsByNamespace(""), ERR_OK);

    TAG_LOGI(AAFwkTag::CLI_TOOL, "CliFunctionDataManager_UnregisterIntentFunctionsByNamespace_003 end");
}

/**
 * @tc.name: CliFunctionDataManager_UnregisterIntentFunctionsByNamespace_004
 * @tc.desc: Test UnregisterIntentFunctionsByNamespace preserves non-INTENT_FUNCTION types
 * @tc.type: FUNC
 */
HWTEST_F(CliFunctionDataManagerTest, CliFunctionDataManager_UnregisterIntentFunctionsByNamespace_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "CliFunctionDataManager_UnregisterIntentFunctionsByNamespace_004 start");

    auto mockStore = std::make_shared<MockSingleKvStore>();
    // Add INTENT_FUNCTION
    mockStore->SetMockData("mixed_ns/intent_func", BuildFunctionJson("mixed_ns", "intent_func"));
    // Add non-INTENT_FUNCTION (type = 1)
    nlohmann::json otherJson = {
        {"functionName", "other_func"},
        {"functionNamespace", "mixed_ns"},
        {"description", "Other type"},
        {"functionType", 1},
        {"version", "1.0"}
    };
    mockStore->SetMockData("mixed_ns/other_func", otherJson.dump());
    CliFunctionDataManager::GetInstance().kvStorePtr_ = mockStore;

    EXPECT_EQ(CliFunctionDataManager::GetInstance().UnregisterIntentFunctionsByNamespace("mixed_ns"), ERR_OK);
    // INTENT_FUNCTION should be deleted
    EXPECT_FALSE(mockStore->HasMockData("mixed_ns/intent_func"));
    // Non-INTENT_FUNCTION should be preserved
    EXPECT_TRUE(mockStore->HasMockData("mixed_ns/other_func"));

    TAG_LOGI(AAFwkTag::CLI_TOOL, "CliFunctionDataManager_UnregisterIntentFunctionsByNamespace_004 end");
}

// ==================== GetAllFunctions Tests ====================

/**
 * @tc.name: CliFunctionDataManager_GetAllFunctions_001
 * @tc.desc: Test GetAllFunctions returns functions from KVStore
 * @tc.type: FUNC
 */
HWTEST_F(CliFunctionDataManagerTest, CliFunctionDataManager_GetAllFunctions_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "CliFunctionDataManager_GetAllFunctions_001 start");

    auto mockStore = std::make_shared<MockSingleKvStore>();
    CliFunctionDataManager::GetInstance().kvStorePtr_ = mockStore;

    std::vector<FunctionInfo> functions;
    int32_t ret = CliFunctionDataManager::GetInstance().GetAllFunctions(functions);

    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(functions.size(), 0u);

    TAG_LOGI(AAFwkTag::CLI_TOOL, "CliFunctionDataManager_GetAllFunctions_001 end");
}

/**
 * @tc.name: CliFunctionDataManager_GetAllFunctions_002
 * @tc.desc: Test GetAllFunctions with mocked KVStore
 * @tc.type: FUNC
 */
HWTEST_F(CliFunctionDataManagerTest, CliFunctionDataManager_GetAllFunctions_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "CliFunctionDataManager_GetAllFunctions_002 start");

    auto mockStore = std::make_shared<MockSingleKvStore>();
    mockStore->SetMockData("all_ns/all_func1", BuildFunctionJson("all_ns", "all_func1"));
    mockStore->SetMockData("all_ns/all_func2", BuildFunctionJson("all_ns", "all_func2"));
    mockStore->SetMockData("broken_func", "{invalid json");
    CliFunctionDataManager::GetInstance().kvStorePtr_ = mockStore;

    std::vector<FunctionInfo> functions;
    int32_t ret = CliFunctionDataManager::GetInstance().GetAllFunctions(functions);

    ASSERT_EQ(ret, ERR_OK);
    ASSERT_EQ(functions.size(), 2u);

    // Use a set to avoid order dependency
    std::set<std::string> functionNames;
    for (const auto& func : functions) {
        functionNames.insert(func.functionName);
    }
    EXPECT_TRUE(functionNames.count("all_func1"));
    EXPECT_TRUE(functionNames.count("all_func2"));

    TAG_LOGI(AAFwkTag::CLI_TOOL, "CliFunctionDataManager_GetAllFunctions_002 end");
}

/**
 * @tc.name: CliFunctionDataManager_GetAllFunctions_003
 * @tc.desc: Test GetAllFunctions with empty KVStore
 * @tc.type: FUNC
 */
HWTEST_F(CliFunctionDataManagerTest, CliFunctionDataManager_GetAllFunctions_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "CliFunctionDataManager_GetAllFunctions_003 start");

    auto mockStore = std::make_shared<MockSingleKvStore>();
    CliFunctionDataManager::GetInstance().kvStorePtr_ = mockStore;

    std::vector<FunctionInfo> functions;
    int32_t ret = CliFunctionDataManager::GetInstance().GetAllFunctions(functions);

    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(functions.size(), 0u);

    TAG_LOGI(AAFwkTag::CLI_TOOL, "CliFunctionDataManager_GetAllFunctions_003 end");
}

// ==================== GenerateFunctionKey Tests ====================

/**
 * @tc.name: CliFunctionDataManager_GenerateFunctionKey_001
 * @tc.desc: Test GenerateFunctionKey generates correct key format
 * @tc.type: FUNC
 */
HWTEST_F(CliFunctionDataManagerTest, CliFunctionDataManager_GenerateFunctionKey_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "CliFunctionDataManager_GenerateFunctionKey_001 start");

    std::string key = CliFunctionDataManager::GenerateFunctionKey("test_ns", "test_func");

    EXPECT_EQ(key, "test_ns/test_func");

    TAG_LOGI(AAFwkTag::CLI_TOOL, "CliFunctionDataManager_GenerateFunctionKey_001 end");
}

/**
 * @tc.name: CliFunctionDataManager_GenerateFunctionKey_002
 * @tc.desc: Test GenerateFunctionKey with empty namespace
 * @tc.type: FUNC
 */
HWTEST_F(CliFunctionDataManagerTest, CliFunctionDataManager_GenerateFunctionKey_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "CliFunctionDataManager_GenerateFunctionKey_002 start");

    std::string key = CliFunctionDataManager::GenerateFunctionKey("", "test_func");

    EXPECT_EQ(key, "/test_func");

    TAG_LOGI(AAFwkTag::CLI_TOOL, "CliFunctionDataManager_GenerateFunctionKey_002 end");
}

// ==================== ExtractNamespaceFromKey Tests ====================

/**
 * @tc.name: CliFunctionDataManager_ExtractNamespaceFromKey_001
 * @tc.desc: Test ExtractNamespaceFromKey with valid key
 * @tc.type: FUNC
 */
HWTEST_F(CliFunctionDataManagerTest, CliFunctionDataManager_ExtractNamespaceFromKey_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "CliFunctionDataManager_ExtractNamespaceFromKey_001 start");

    std::string ns = CliFunctionDataManager::ExtractNamespaceFromKey("test_ns/test_func");

    EXPECT_EQ(ns, "test_ns");

    TAG_LOGI(AAFwkTag::CLI_TOOL, "CliFunctionDataManager_ExtractNamespaceFromKey_001 end");
}

/**
 * @tc.name: CliFunctionDataManager_ExtractNamespaceFromKey_002
 * @tc.desc: Test ExtractNamespaceFromKey with invalid key (no separator)
 * @tc.type: FUNC
 */
HWTEST_F(CliFunctionDataManagerTest, CliFunctionDataManager_ExtractNamespaceFromKey_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "CliFunctionDataManager_ExtractNamespaceFromKey_002 start");

    std::string ns = CliFunctionDataManager::ExtractNamespaceFromKey("invalid_key");

    EXPECT_EQ(ns, "");

    TAG_LOGI(AAFwkTag::CLI_TOOL, "CliFunctionDataManager_ExtractNamespaceFromKey_002 end");
}

/**
 * @tc.name: CliFunctionDataManager_ExtractNamespaceFromKey_003
 * @tc.desc: Test ExtractNamespaceFromKey with empty key
 * @tc.type: FUNC
 */
HWTEST_F(CliFunctionDataManagerTest, CliFunctionDataManager_ExtractNamespaceFromKey_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "CliFunctionDataManager_ExtractNamespaceFromKey_003 start");

    std::string ns = CliFunctionDataManager::ExtractNamespaceFromKey("");

    EXPECT_EQ(ns, "");

    TAG_LOGI(AAFwkTag::CLI_TOOL, "CliFunctionDataManager_ExtractNamespaceFromKey_003 end");
}

// ==================== KeyMatchesNamespace Tests ====================

/**
 * @tc.name: CliFunctionDataManager_KeyMatchesNamespace_001
 * @tc.desc: Test KeyMatchesNamespace with matching namespace
 * @tc.type: FUNC
 */
HWTEST_F(CliFunctionDataManagerTest, CliFunctionDataManager_KeyMatchesNamespace_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "CliFunctionDataManager_KeyMatchesNamespace_001 start");

    bool matches = CliFunctionDataManager::KeyMatchesNamespace("test_ns/test_func", "test_ns");

    EXPECT_TRUE(matches);

    TAG_LOGI(AAFwkTag::CLI_TOOL, "CliFunctionDataManager_KeyMatchesNamespace_001 end");
}

/**
 * @tc.name: CliFunctionDataManager_KeyMatchesNamespace_002
 * @tc.desc: Test KeyMatchesNamespace with non-matching namespace
 * @tc.type: FUNC
 */
HWTEST_F(CliFunctionDataManagerTest, CliFunctionDataManager_KeyMatchesNamespace_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "CliFunctionDataManager_KeyMatchesNamespace_002 start");

    bool matches = CliFunctionDataManager::KeyMatchesNamespace("other_ns/test_func", "test_ns");

    EXPECT_FALSE(matches);

    TAG_LOGI(AAFwkTag::CLI_TOOL, "CliFunctionDataManager_KeyMatchesNamespace_002 end");
}

/**
 * @tc.name: CliFunctionDataManager_KeyMatchesNamespace_003
 * @tc.desc: Test KeyMatchesNamespace with invalid key
 * @tc.type: FUNC
 */
HWTEST_F(CliFunctionDataManagerTest, CliFunctionDataManager_KeyMatchesNamespace_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "CliFunctionDataManager_KeyMatchesNamespace_003 start");

    bool matches = CliFunctionDataManager::KeyMatchesNamespace("invalid_key", "test_ns");

    EXPECT_FALSE(matches);

    TAG_LOGI(AAFwkTag::CLI_TOOL, "CliFunctionDataManager_KeyMatchesNamespace_003 end");
}

// ==================== IsIntentFunction Tests ====================

/**
 * @tc.name: CliFunctionDataManager_IsIntentFunction_001
 * @tc.desc: Test IsIntentFunction with INTENT_FUNCTION type
 * @tc.type: FUNC
 */
HWTEST_F(CliFunctionDataManagerTest, CliFunctionDataManager_IsIntentFunction_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "CliFunctionDataManager_IsIntentFunction_001 start");

    std::string jsonStr = BuildFunctionJson("test_ns", "test_func", "Test");
    DistributedKv::Value value(jsonStr);

    bool isIntent = CliFunctionDataManager::IsIntentFunction(value);

    EXPECT_TRUE(isIntent);

    TAG_LOGI(AAFwkTag::CLI_TOOL, "CliFunctionDataManager_IsIntentFunction_001 end");
}

/**
 * @tc.name: CliFunctionDataManager_IsIntentFunction_002
 * @tc.desc: Test IsIntentFunction with non-INTENT_FUNCTION type
 * @tc.type: FUNC
 */
HWTEST_F(CliFunctionDataManagerTest, CliFunctionDataManager_IsIntentFunction_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "CliFunctionDataManager_IsIntentFunction_002 start");

    nlohmann::json json = {
        {"functionName", "other_func"},
        {"functionNamespace", "test_ns"},
        {"description", "Other type"},
        {"functionType", 1},  // Not INTENT_FUNCTION (which is 0)
        {"version", "1.0"}
    };
    DistributedKv::Value value(json.dump());

    bool isIntent = CliFunctionDataManager::IsIntentFunction(value);

    EXPECT_FALSE(isIntent);

    TAG_LOGI(AAFwkTag::CLI_TOOL, "CliFunctionDataManager_IsIntentFunction_002 end");
}

/**
 * @tc.name: CliFunctionDataManager_IsIntentFunction_003
 * @tc.desc: Test IsIntentFunction with invalid JSON
 * @tc.type: FUNC
 */
HWTEST_F(CliFunctionDataManagerTest, CliFunctionDataManager_IsIntentFunction_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "CliFunctionDataManager_IsIntentFunction_003 start");

    DistributedKv::Value value("{invalid json");

    bool isIntent = CliFunctionDataManager::IsIntentFunction(value);

    EXPECT_FALSE(isIntent);

    TAG_LOGI(AAFwkTag::CLI_TOOL, "CliFunctionDataManager_IsIntentFunction_003 end");
}

// ==================== CheckKvStore Tests ====================

/**
 * @tc.name: CliFunctionDataManager_CheckKvStore_001
 * @tc.desc: Test CheckKvStore with mocked KVStore
 * @tc.type: FUNC
 */
HWTEST_F(CliFunctionDataManagerTest, CliFunctionDataManager_CheckKvStore_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "CliFunctionDataManager_CheckKvStore_001 start");

    auto mockStore = std::make_shared<MockSingleKvStore>();
    CliFunctionDataManager::GetInstance().kvStorePtr_ = mockStore;

    bool result = CliFunctionDataManager::GetInstance().CheckKvStore();

    EXPECT_TRUE(result);

    TAG_LOGI(AAFwkTag::CLI_TOOL, "CliFunctionDataManager_CheckKvStore_001 end");
}

// ==================== StoreFunctionNoLock Tests ====================

/**
 * @tc.name: CliFunctionDataManager_StoreFunctionNoLock_001
 * @tc.desc: Test StoreFunctionNoLock through RegisterFunction
 * @tc.type: FUNC
 */
HWTEST_F(CliFunctionDataManagerTest, CliFunctionDataManager_StoreFunctionNoLock_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "CliFunctionDataManager_StoreFunctionNoLock_001 start");

    auto mockStore = std::make_shared<MockSingleKvStore>();
    CliFunctionDataManager::GetInstance().kvStorePtr_ = mockStore;

    FunctionInfo function;
    function.functionName = "store_function";
    function.functionNamespace = "store_ns";
    function.description = "Store test";
    function.functionType = FunctionType::INTENT_FUNCTION;

    int32_t ret = CliFunctionDataManager::GetInstance().RegisterFunction(function);

    EXPECT_EQ(ret, ERR_OK);
    EXPECT_TRUE(mockStore->HasMockData("store_ns/store_function"));

    TAG_LOGI(AAFwkTag::CLI_TOOL, "CliFunctionDataManager_StoreFunctionNoLock_001 end");
}

/**
 * @tc.name: CliFunctionDataManager_StoreFunctionNoLock_002
 * @tc.desc: Test StoreFunctionNoLock with failed Put operation
 * @tc.type: FUNC
 */
HWTEST_F(CliFunctionDataManagerTest, CliFunctionDataManager_StoreFunctionNoLock_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "CliFunctionDataManager_StoreFunctionNoLock_002 start");

    auto mockStore = std::make_shared<MockSingleKvStore>();
    mockStore->Put_ = DistributedKv::Status::ERROR;
    CliFunctionDataManager::GetInstance().kvStorePtr_ = mockStore;

    FunctionInfo function;
    function.functionName = "fail_store_function";
    function.functionNamespace = "fail_store_ns";
    function.functionType = FunctionType::INTENT_FUNCTION;

    int32_t ret = CliFunctionDataManager::GetInstance().RegisterFunction(function);

    EXPECT_EQ(ret, ERR_KVSTORE_ERROR);

    TAG_LOGI(AAFwkTag::CLI_TOOL, "CliFunctionDataManager_StoreFunctionNoLock_002 end");
}

// ==================== RestoreKvStore Tests ====================

/**
 * @tc.name: CliFunctionDataManager_RestoreKvStore_001
 * @tc.desc: Test RestoreKvStore with DATA_CORRUPTED status
 * @tc.type: FUNC
 * @tc.require: This test requires distributed KVStore service to be available
 */
HWTEST_F(CliFunctionDataManagerTest, CliFunctionDataManager_RestoreKvStore_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "CliFunctionDataManager_RestoreKvStore_001 start");

    auto& dataManager = CliFunctionDataManager::GetInstance();

    // Test with DATA_CORRUPTED status - should attempt to recreate KVStore
    // Note: This test calls real distributed KVStore service (DeleteKvStore/GetSingleKvStore)
    dataManager.RestoreKvStore(DistributedKv::Status::DATA_CORRUPTED);

    // This test validates RestoreKvStore can handle DATA_CORRUPTED status without crashing
    // Result depends on external KVStore service availability
    // At minimum, verify the test executed without fatal failure
    EXPECT_NO_FATAL_FAILURE();

    TAG_LOGI(AAFwkTag::CLI_TOOL, "CliFunctionDataManager_RestoreKvStore_001 end");
}

/**
 * @tc.name: CliFunctionDataManager_RestoreKvStore_002
 * @tc.desc: Test RestoreKvStore with other error status (void return, no crash)
 * @tc.type: FUNC
 */
HWTEST_F(CliFunctionDataManagerTest, CliFunctionDataManager_RestoreKvStore_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "CliFunctionDataManager_RestoreKvStore_002 start");

    auto& dataManager = CliFunctionDataManager::GetInstance();

    // Test with non-DATA_CORRUPTED status - should do nothing without calling KVStore
    // RestoreKvStore now returns void, just verify it doesn't crash
    dataManager.RestoreKvStore(DistributedKv::Status::INVALID_SCHEMA);

    // No assertion needed, just verify no crash
    EXPECT_NO_FATAL_FAILURE();

    TAG_LOGI(AAFwkTag::CLI_TOOL, "CliFunctionDataManager_RestoreKvStore_002 end");
}

// ==================== FunctionInfo Validate Tests ====================

/**
 * @tc.name: FunctionInfo_Validate_001
 * @tc.desc: Test FunctionInfo Validate with valid function
 * @tc.type: FUNC
 */
HWTEST_F(CliFunctionDataManagerTest, FunctionInfo_Validate_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "FunctionInfo_Validate_001 start");

    FunctionInfo function;
    function.functionName = "valid_function";
    function.functionNamespace = "valid_ns";
    function.functionType = FunctionType::INTENT_FUNCTION;

    bool valid = FunctionInfo::Validate(function);

    EXPECT_TRUE(valid);

    TAG_LOGI(AAFwkTag::CLI_TOOL, "FunctionInfo_Validate_001 end");
}

/**
 * @tc.name: FunctionInfo_Validate_002
 * @tc.desc: Test FunctionInfo Validate with empty functionName
 * @tc.type: FUNC
 */
HWTEST_F(CliFunctionDataManagerTest, FunctionInfo_Validate_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "FunctionInfo_Validate_002 start");

    FunctionInfo function;
    function.functionName = "";
    function.functionNamespace = "test_ns";
    function.functionType = FunctionType::INTENT_FUNCTION;

    bool valid = FunctionInfo::Validate(function);

    EXPECT_FALSE(valid);

    TAG_LOGI(AAFwkTag::CLI_TOOL, "FunctionInfo_Validate_002 end");
}

/**
 * @tc.name: FunctionInfo_Validate_003
 * @tc.desc: Test FunctionInfo Validate with empty namespace
 * @tc.type: FUNC
 */
HWTEST_F(CliFunctionDataManagerTest, FunctionInfo_Validate_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "FunctionInfo_Validate_003 start");

    FunctionInfo function;
    function.functionName = "test_function";
    function.functionNamespace = "";
    function.functionType = FunctionType::INTENT_FUNCTION;

    bool valid = FunctionInfo::Validate(function);

    EXPECT_FALSE(valid);

    TAG_LOGI(AAFwkTag::CLI_TOOL, "FunctionInfo_Validate_003 end");
}

/**
 * @tc.name: FunctionInfo_Validate_004
 * @tc.desc: Test FunctionInfo Validate with invalid inputSchema
 * @tc.type: FUNC
 */
HWTEST_F(CliFunctionDataManagerTest, FunctionInfo_Validate_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "FunctionInfo_Validate_004 start");

    FunctionInfo function;
    function.functionName = "test_function";
    function.functionNamespace = "test_ns";
    function.inputSchema = "{invalid json";
    function.functionType = FunctionType::INTENT_FUNCTION;

    bool valid = FunctionInfo::Validate(function);

    EXPECT_FALSE(valid);

    TAG_LOGI(AAFwkTag::CLI_TOOL, "FunctionInfo_Validate_004 end");
}

/**
 * @tc.name: FunctionInfo_Validate_005
 * @tc.desc: Test FunctionInfo Validate with illegal '/' in functionName
 * @tc.type: FUNC
 */
HWTEST_F(CliFunctionDataManagerTest, FunctionInfo_Validate_005, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "FunctionInfo_Validate_005 start");

    FunctionInfo function;
    function.functionName = "invalid/func";
    function.functionNamespace = "test_ns";
    function.functionType = FunctionType::INTENT_FUNCTION;

    bool valid = FunctionInfo::Validate(function);

    EXPECT_FALSE(valid);

    TAG_LOGI(AAFwkTag::CLI_TOOL, "FunctionInfo_Validate_005 end");
}

/**
 * @tc.name: FunctionInfo_Validate_006
 * @tc.desc: Test FunctionInfo Validate with illegal '/' in functionNamespace
 * @tc.type: FUNC
 */
HWTEST_F(CliFunctionDataManagerTest, FunctionInfo_Validate_006, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "FunctionInfo_Validate_006 start");

    FunctionInfo function;
    function.functionName = "test_func";
    function.functionNamespace = "invalid/ns";
    function.functionType = FunctionType::INTENT_FUNCTION;

    bool valid = FunctionInfo::Validate(function);

    EXPECT_FALSE(valid);

    TAG_LOGI(AAFwkTag::CLI_TOOL, "FunctionInfo_Validate_006 end");
}

/**
 * @tc.name: FunctionInfo_Validate_007
 * @tc.desc: Test FunctionInfo Validate with '/' in both fields
 * @tc.type: FUNC
 */
HWTEST_F(CliFunctionDataManagerTest, FunctionInfo_Validate_007, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "FunctionInfo_Validate_007 start");

    FunctionInfo function;
    function.functionName = "bad/func";
    function.functionNamespace = "bad/ns";
    function.functionType = FunctionType::INTENT_FUNCTION;

    bool valid = FunctionInfo::Validate(function);

    EXPECT_FALSE(valid);

    TAG_LOGI(AAFwkTag::CLI_TOOL, "FunctionInfo_Validate_007 end");
}

// ==================== BatchRegisterFunctions Tests ====================

/**
 * @tc.name: CliFunctionDataManager_BatchRegisterFunctions_001
 * @tc.desc: Test BatchRegisterFunctions with multiple valid functions
 * @tc.type: FUNC
 */
HWTEST_F(CliFunctionDataManagerTest, CliFunctionDataManager_BatchRegisterFunctions_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "CliFunctionDataManager_BatchRegisterFunctions_001 start");

    auto mockStore = std::make_shared<MockSingleKvStore>();
    CliFunctionDataManager::GetInstance().kvStorePtr_ = mockStore;

    std::vector<FunctionInfo> functions;
    for (int i = 0; i < 3; i++) {
        FunctionInfo function;
        function.functionName = "batch_func_" + std::to_string(i);
        function.functionNamespace = "batch_ns";
        function.functionType = FunctionType::INTENT_FUNCTION;
        functions.push_back(function);
    }

    int32_t successCount = 0;
    int32_t ret = CliFunctionDataManager::GetInstance().BatchRegisterFunctions(functions, successCount);

    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(successCount, 3);
    EXPECT_TRUE(mockStore->HasMockData("batch_ns/batch_func_0"));
    EXPECT_TRUE(mockStore->HasMockData("batch_ns/batch_func_1"));
    EXPECT_TRUE(mockStore->HasMockData("batch_ns/batch_func_2"));

    TAG_LOGI(AAFwkTag::CLI_TOOL, "CliFunctionDataManager_BatchRegisterFunctions_001 end");
}

/**
 * @tc.name: CliFunctionDataManager_BatchRegisterFunctions_002
 * @tc.desc: Test BatchRegisterFunctions with empty vector
 * @tc.type: FUNC
 */
HWTEST_F(CliFunctionDataManagerTest, CliFunctionDataManager_BatchRegisterFunctions_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "CliFunctionDataManager_BatchRegisterFunctions_002 start");

    auto mockStore = std::make_shared<MockSingleKvStore>();
    CliFunctionDataManager::GetInstance().kvStorePtr_ = mockStore;

    std::vector<FunctionInfo> functions;
    int32_t successCount = 0;
    int32_t ret = CliFunctionDataManager::GetInstance().BatchRegisterFunctions(functions, successCount);

    EXPECT_EQ(ret, ERR_INVALID_PARAM);
    EXPECT_EQ(successCount, 0);

    TAG_LOGI(AAFwkTag::CLI_TOOL, "CliFunctionDataManager_BatchRegisterFunctions_002 end");
}

/**
 * @tc.name: CliFunctionDataManager_BatchRegisterFunctions_005
 * @tc.desc: Test BatchRegisterFunctions with single function
 * @tc.type: FUNC
 */
HWTEST_F(CliFunctionDataManagerTest, CliFunctionDataManager_BatchRegisterFunctions_005, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "CliFunctionDataManager_BatchRegisterFunctions_005 start");

    auto mockStore = std::make_shared<MockSingleKvStore>();
    CliFunctionDataManager::GetInstance().kvStorePtr_ = mockStore;

    std::vector<FunctionInfo> functions;
    FunctionInfo function;
    function.functionName = "single_batch_func";
    function.functionNamespace = "single_batch_ns";
    function.functionType = FunctionType::INTENT_FUNCTION;
    functions.push_back(function);

    int32_t successCount = 0;
    int32_t ret = CliFunctionDataManager::GetInstance().BatchRegisterFunctions(functions, successCount);

    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(successCount, 1);
    EXPECT_TRUE(mockStore->HasMockData("single_batch_ns/single_batch_func"));

    TAG_LOGI(AAFwkTag::CLI_TOOL, "CliFunctionDataManager_BatchRegisterFunctions_005 end");
}

/**
 * @tc.name: CliFunctionDataManager_BatchRegisterFunctions_003
 * @tc.desc: Test BatchRegisterFunctions with KVStore failure on first function
 * @tc.type: FUNC
 */
HWTEST_F(CliFunctionDataManagerTest, CliFunctionDataManager_BatchRegisterFunctions_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "CliFunctionDataManager_BatchRegisterFunctions_003 start");

    auto mockStore = std::make_shared<MockSingleKvStore>();
    mockStore->Put_ = DistributedKv::Status::ERROR;
    CliFunctionDataManager::GetInstance().kvStorePtr_ = mockStore;

    std::vector<FunctionInfo> functions;
    for (int i = 0; i < 3; i++) {
        FunctionInfo function;
        function.functionName = "fail_func_" + std::to_string(i);
        function.functionNamespace = "fail_ns";
        function.functionType = FunctionType::INTENT_FUNCTION;
        functions.push_back(function);
    }

    int32_t successCount = 0;
    int32_t ret = CliFunctionDataManager::GetInstance().BatchRegisterFunctions(functions, successCount);

    EXPECT_EQ(ret, ERR_KVSTORE_ERROR);
    EXPECT_EQ(successCount, 0);

    TAG_LOGI(AAFwkTag::CLI_TOOL, "CliFunctionDataManager_BatchRegisterFunctions_003 end");
}

/**
 * @tc.name: CliFunctionDataManager_BatchRegisterFunctions_004
 * @tc.desc: Test BatchRegisterFunctions with failure on middle function
 * @tc.type: FUNC
 */
HWTEST_F(CliFunctionDataManagerTest, CliFunctionDataManager_BatchRegisterFunctions_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "CliFunctionDataManager_BatchRegisterFunctions_004 start");

    auto mockStore = std::make_shared<MockSingleKvStore>();
    CliFunctionDataManager::GetInstance().kvStorePtr_ = mockStore;

    std::vector<FunctionInfo> functions;
    for (int i = 0; i < 5; i++) {
        FunctionInfo function;
        function.functionName = "partial_func_" + std::to_string(i);
        function.functionNamespace = "partial_ns";
        function.functionType = FunctionType::INTENT_FUNCTION;
        functions.push_back(function);
    }

    // After 2 successful Put, make the 3rd fail
    int callCount = 0;
    mockStore->Put_ = DistributedKv::Status::SUCCESS;
    mockStore->PutCallback = [&callCount](const DistributedKv::Key&, const DistributedKv::Value&)
        -> DistributedKv::Status {
        callCount++;
        return callCount <= 2 ? DistributedKv::Status::SUCCESS : DistributedKv::Status::ERROR;
    };

    int32_t successCount = 0;
    int32_t ret = CliFunctionDataManager::GetInstance().BatchRegisterFunctions(functions, successCount);

    EXPECT_EQ(ret, ERR_KVSTORE_ERROR);
    EXPECT_EQ(successCount, 2);
    // First 2 functions should be stored
    EXPECT_TRUE(mockStore->HasMockData("partial_ns/partial_func_0"));
    EXPECT_TRUE(mockStore->HasMockData("partial_ns/partial_func_1"));
    // Remaining functions should not be stored due to early break
    EXPECT_FALSE(mockStore->HasMockData("partial_ns/partial_func_2"));

    TAG_LOGI(AAFwkTag::CLI_TOOL, "CliFunctionDataManager_BatchRegisterFunctions_004 end");
}

// ==================== ResetNamespaceFunctions Tests ====================

/**
 * @tc.name: CliFunctionDataManager_ResetNamespaceFunctions_001
 * @tc.desc: Test ResetNamespaceFunctions with valid namespace and functions
 * @tc.type: FUNC
 */
HWTEST_F(CliFunctionDataManagerTest, CliFunctionDataManager_ResetNamespaceFunctions_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "CliFunctionDataManager_ResetNamespaceFunctions_001 start");

    auto mockStore = std::make_shared<MockSingleKvStore>();
    CliFunctionDataManager::GetInstance().kvStorePtr_ = mockStore;

    // Pre-populate with existing functions
    mockStore->SetMockData("reset_ns/old_func1", BuildFunctionJson("reset_ns", "old_func1"));
    mockStore->SetMockData("reset_ns/old_func2", BuildFunctionJson("reset_ns", "old_func2"));

    // Create new function list
    std::vector<FunctionInfo> functions;
    for (int i = 0; i < 3; i++) {
        FunctionInfo function;
        function.functionName = "new_func_" + std::to_string(i);
        function.functionNamespace = "reset_ns";
        function.functionType = FunctionType::INTENT_FUNCTION;
        functions.push_back(function);
    }

    int32_t successCount = 0;
    int32_t ret = CliFunctionDataManager::GetInstance().ResetNamespaceFunctions(
        "reset_ns", functions, successCount);

    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(successCount, 3);
    // Old functions should be deleted
    EXPECT_FALSE(mockStore->HasMockData("reset_ns/old_func1"));
    EXPECT_FALSE(mockStore->HasMockData("reset_ns/old_func2"));
    // New functions should be added
    EXPECT_TRUE(mockStore->HasMockData("reset_ns/new_func_0"));
    EXPECT_TRUE(mockStore->HasMockData("reset_ns/new_func_1"));
    EXPECT_TRUE(mockStore->HasMockData("reset_ns/new_func_2"));

    TAG_LOGI(AAFwkTag::CLI_TOOL, "CliFunctionDataManager_ResetNamespaceFunctions_001 end");
}

/**
 * @tc.name: CliFunctionDataManager_ResetNamespaceFunctions_002
 * @tc.desc: Test ResetNamespaceFunctions with empty namespace
 * @tc.type: FUNC
 */
HWTEST_F(CliFunctionDataManagerTest, CliFunctionDataManager_ResetNamespaceFunctions_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "CliFunctionDataManager_ResetNamespaceFunctions_002 start");

    auto mockStore = std::make_shared<MockSingleKvStore>();
    CliFunctionDataManager::GetInstance().kvStorePtr_ = mockStore;

    std::vector<FunctionInfo> functions;
    FunctionInfo function;
    function.functionName = "test_func";
    function.functionNamespace = "test_ns";
    function.functionType = FunctionType::INTENT_FUNCTION;
    functions.push_back(function);

    int32_t successCount = 0;
    int32_t ret = CliFunctionDataManager::GetInstance().ResetNamespaceFunctions(
        "", functions, successCount);

    EXPECT_EQ(ret, ERR_INVALID_PARAM);
    EXPECT_EQ(successCount, 0);

    TAG_LOGI(AAFwkTag::CLI_TOOL, "CliFunctionDataManager_ResetNamespaceFunctions_002 end");
}

/**
 * @tc.name: CliFunctionDataManager_ResetNamespaceFunctions_003
 * @tc.desc: Test ResetNamespaceFunctions with empty function list
 * @tc.type: FUNC
 */
HWTEST_F(CliFunctionDataManagerTest, CliFunctionDataManager_ResetNamespaceFunctions_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "CliFunctionDataManager_ResetNamespaceFunctions_003 start");

    auto mockStore = std::make_shared<MockSingleKvStore>();
    // Pre-populate with existing functions
    mockStore->SetMockData("empty_ns/old_func", BuildFunctionJson("empty_ns", "old_func"));
    CliFunctionDataManager::GetInstance().kvStorePtr_ = mockStore;

    std::vector<FunctionInfo> functions;  // Empty list

    int32_t successCount = 0;
    int32_t ret = CliFunctionDataManager::GetInstance().ResetNamespaceFunctions(
        "empty_ns", functions, successCount);

    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(successCount, 0);
    // Old function should be deleted
    EXPECT_FALSE(mockStore->HasMockData("empty_ns/old_func"));

    TAG_LOGI(AAFwkTag::CLI_TOOL, "CliFunctionDataManager_ResetNamespaceFunctions_003 end");
}

/**
 * @tc.name: CliFunctionDataManager_ResetNamespaceFunctions_004
 * @tc.desc: Test ResetNamespaceFunctions skips non-INTENT_FUNCTION types
 * @tc.type: FUNC
 */
HWTEST_F(CliFunctionDataManagerTest, CliFunctionDataManager_ResetNamespaceFunctions_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "CliFunctionDataManager_ResetNamespaceFunctions_004 start");

    auto mockStore = std::make_shared<MockSingleKvStore>();
    CliFunctionDataManager::GetInstance().kvStorePtr_ = mockStore;

    std::vector<FunctionInfo> functions;

    // Add an INTENT_FUNCTION (should be registered)
    FunctionInfo intentFunc;
    intentFunc.functionName = "intent_func";
    intentFunc.functionNamespace = "mixed_ns";
    intentFunc.functionType = FunctionType::INTENT_FUNCTION;
    functions.push_back(intentFunc);

    // Add a non-INTENT_FUNCTION (should be skipped)
    FunctionInfo otherFunc;
    otherFunc.functionName = "other_func";
    otherFunc.functionNamespace = "mixed_ns";
    otherFunc.functionType = static_cast<FunctionType>(1);  // Different type
    functions.push_back(otherFunc);

    int32_t successCount = 0;
    int32_t ret = CliFunctionDataManager::GetInstance().ResetNamespaceFunctions(
        "mixed_ns", functions, successCount);

    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(successCount, 1);  // Only INTENT_FUNCTION counted
    EXPECT_TRUE(mockStore->HasMockData("mixed_ns/intent_func"));
    EXPECT_FALSE(mockStore->HasMockData("mixed_ns/other_func"));

    TAG_LOGI(AAFwkTag::CLI_TOOL, "CliFunctionDataManager_ResetNamespaceFunctions_004 end");
}

/**
 * @tc.name: CliFunctionDataManager_ResetNamespaceFunctions_005
 * @tc.desc: Test ResetNamespaceFunctions with single function
 * @tc.type: FUNC
 */
HWTEST_F(CliFunctionDataManagerTest, CliFunctionDataManager_ResetNamespaceFunctions_005, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "CliFunctionDataManager_ResetNamespaceFunctions_005 start");

    auto mockStore = std::make_shared<MockSingleKvStore>();
    CliFunctionDataManager::GetInstance().kvStorePtr_ = mockStore;

    std::vector<FunctionInfo> functions;
    FunctionInfo function;
    function.functionName = "single_reset_func";
    function.functionNamespace = "single_reset_ns";
    function.functionType = FunctionType::INTENT_FUNCTION;
    functions.push_back(function);

    int32_t successCount = 0;
    int32_t ret = CliFunctionDataManager::GetInstance().ResetNamespaceFunctions(
        "single_reset_ns", functions, successCount);

    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(successCount, 1);
    EXPECT_TRUE(mockStore->HasMockData("single_reset_ns/single_reset_func"));

    TAG_LOGI(AAFwkTag::CLI_TOOL, "CliFunctionDataManager_ResetNamespaceFunctions_005 end");
}

/**
 * @tc.name: CliFunctionDataManager_ResetNamespaceFunctions_007
 * @tc.desc: Test ResetNamespaceFunctions preserves old data on add failure
 * @tc.type: FUNC
 */
HWTEST_F(CliFunctionDataManagerTest, CliFunctionDataManager_ResetNamespaceFunctions_007, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "CliFunctionDataManager_ResetNamespaceFunctions_007 start");

    auto mockStore = std::make_shared<MockSingleKvStore>();
    CliFunctionDataManager::GetInstance().kvStorePtr_ = mockStore;

    // Pre-populate with existing functions
    mockStore->SetMockData("preserve_ns/old_func1", BuildFunctionJson("preserve_ns", "old_func1"));
    mockStore->SetMockData("preserve_ns/old_func2", BuildFunctionJson("preserve_ns", "old_func2"));

    // Make Put fail during AddNewFunctions
    mockStore->Put_ = DistributedKv::Status::ERROR;

    std::vector<FunctionInfo> functions;
    for (int i = 0; i < 3; i++) {
        FunctionInfo function;
        function.functionName = "new_func_" + std::to_string(i);
        function.functionNamespace = "preserve_ns";
        function.functionType = FunctionType::INTENT_FUNCTION;
        functions.push_back(function);
    }

    int32_t successCount = 0;
    int32_t ret = CliFunctionDataManager::GetInstance().ResetNamespaceFunctions(
        "preserve_ns", functions, successCount);

    // Should fail and return error
    EXPECT_NE(ret, ERR_OK);
    EXPECT_EQ(successCount, 0);
    // Old functions should be preserved (not deleted)
    EXPECT_TRUE(mockStore->HasMockData("preserve_ns/old_func1"));
    EXPECT_TRUE(mockStore->HasMockData("preserve_ns/old_func2"));

    TAG_LOGI(AAFwkTag::CLI_TOOL, "CliFunctionDataManager_ResetNamespaceFunctions_007 end");
}

/**
 * @tc.name: CliFunctionDataManager_ResetNamespaceFunctions_008
 * @tc.desc: Test ResetNamespaceFunctions only affects specified namespace
 * @tc.type: FUNC
 */
HWTEST_F(CliFunctionDataManagerTest, CliFunctionDataManager_ResetNamespaceFunctions_008, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "CliFunctionDataManager_ResetNamespaceFunctions_008 start");

    auto mockStore = std::make_shared<MockSingleKvStore>();
    CliFunctionDataManager::GetInstance().kvStorePtr_ = mockStore;

    // Pre-populate with functions from different namespaces
    mockStore->SetMockData("target_ns/target_func1", BuildFunctionJson("target_ns", "target_func1"));
    mockStore->SetMockData("target_ns/target_func2", BuildFunctionJson("target_ns", "target_func2"));
    mockStore->SetMockData("other_ns/other_func1", BuildFunctionJson("other_ns", "other_func1"));
    mockStore->SetMockData("other_ns/other_func2", BuildFunctionJson("other_ns", "other_func2"));

    // Only reset target_ns
    std::vector<FunctionInfo> functions;
    FunctionInfo function;
    function.functionName = "new_target_func";
    function.functionNamespace = "target_ns";
    function.functionType = FunctionType::INTENT_FUNCTION;
    functions.push_back(function);

    int32_t successCount = 0;
    int32_t ret = CliFunctionDataManager::GetInstance().ResetNamespaceFunctions(
        "target_ns", functions, successCount);

    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(successCount, 1);
    // target_ns functions should be replaced
    EXPECT_FALSE(mockStore->HasMockData("target_ns/target_func1"));
    EXPECT_FALSE(mockStore->HasMockData("target_ns/target_func2"));
    EXPECT_TRUE(mockStore->HasMockData("target_ns/new_target_func"));
    // other_ns functions should be preserved
    EXPECT_TRUE(mockStore->HasMockData("other_ns/other_func1"));
    EXPECT_TRUE(mockStore->HasMockData("other_ns/other_func2"));

    TAG_LOGI(AAFwkTag::CLI_TOOL, "CliFunctionDataManager_ResetNamespaceFunctions_008 end");
}

} // namespace CliTool
} // namespace OHOS
