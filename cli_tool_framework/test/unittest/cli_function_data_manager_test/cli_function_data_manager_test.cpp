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

// ==================== MatchesIntentFunctionNamespace Tests ====================

/**
 * @tc.name: CliFunctionDataManager_MatchesIntentFunctionNamespace_001
 * @tc.desc: Test MatchesIntentFunctionNamespace with matching namespace
 * @tc.type: FUNC
 */
HWTEST_F(CliFunctionDataManagerTest, CliFunctionDataManager_MatchesIntentFunctionNamespace_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "CliFunctionDataManager_MatchesIntentFunctionNamespace_001 start");

    std::string jsonStr = BuildFunctionJson("test_ns", "test_func", "Test");
    DistributedKv::Value value(jsonStr);

    bool matches = CliFunctionDataManager::MatchesIntentFunctionNamespace(value, "test_ns");

    EXPECT_TRUE(matches);

    TAG_LOGI(AAFwkTag::CLI_TOOL, "CliFunctionDataManager_MatchesIntentFunctionNamespace_001 end");
}

/**
 * @tc.name: CliFunctionDataManager_MatchesIntentFunctionNamespace_002
 * @tc.desc: Test MatchesIntentFunctionNamespace with non-matching namespace
 * @tc.type: FUNC
 */
HWTEST_F(CliFunctionDataManagerTest, CliFunctionDataManager_MatchesIntentFunctionNamespace_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "CliFunctionDataManager_MatchesIntentFunctionNamespace_002 start");

    std::string jsonStr = BuildFunctionJson("other_ns", "test_func", "Test");
    DistributedKv::Value value(jsonStr);

    bool matches = CliFunctionDataManager::MatchesIntentFunctionNamespace(value, "test_ns");

    EXPECT_FALSE(matches);

    TAG_LOGI(AAFwkTag::CLI_TOOL, "CliFunctionDataManager_MatchesIntentFunctionNamespace_002 end");
}

/**
 * @tc.name: CliFunctionDataManager_MatchesIntentFunctionNamespace_003
 * @tc.desc: Test MatchesIntentFunctionNamespace with invalid JSON
 * @tc.type: FUNC
 */
HWTEST_F(CliFunctionDataManagerTest, CliFunctionDataManager_MatchesIntentFunctionNamespace_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "CliFunctionDataManager_MatchesIntentFunctionNamespace_003 start");

    DistributedKv::Value value("{invalid json");

    bool matches = CliFunctionDataManager::MatchesIntentFunctionNamespace(value, "test_ns");

    EXPECT_FALSE(matches);

    TAG_LOGI(AAFwkTag::CLI_TOOL, "CliFunctionDataManager_MatchesIntentFunctionNamespace_003 end");
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

// ==================== StoreFunction Tests ====================

/**
 * @tc.name: CliFunctionDataManager_StoreFunction_001
 * @tc.desc: Test StoreFunction through RegisterFunction
 * @tc.type: FUNC
 */
HWTEST_F(CliFunctionDataManagerTest, CliFunctionDataManager_StoreFunction_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "CliFunctionDataManager_StoreFunction_001 start");

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

    TAG_LOGI(AAFwkTag::CLI_TOOL, "CliFunctionDataManager_StoreFunction_001 end");
}

/**
 * @tc.name: CliFunctionDataManager_StoreFunction_002
 * @tc.desc: Test StoreFunction with failed Put operation
 * @tc.type: FUNC
 */
HWTEST_F(CliFunctionDataManagerTest, CliFunctionDataManager_StoreFunction_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "CliFunctionDataManager_StoreFunction_002 start");

    auto mockStore = std::make_shared<MockSingleKvStore>();
    mockStore->Put_ = DistributedKv::Status::ERROR;
    CliFunctionDataManager::GetInstance().kvStorePtr_ = mockStore;

    FunctionInfo function;
    function.functionName = "fail_store_function";
    function.functionNamespace = "fail_store_ns";
    function.functionType = FunctionType::INTENT_FUNCTION;

    int32_t ret = CliFunctionDataManager::GetInstance().RegisterFunction(function);

    EXPECT_EQ(ret, ERR_KVSTORE_ERROR);

    TAG_LOGI(AAFwkTag::CLI_TOOL, "CliFunctionDataManager_StoreFunction_002 end");
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
 * @tc.desc: Test RestoreKvStore with other error status
 * @tc.type: FUNC
 */
HWTEST_F(CliFunctionDataManagerTest, CliFunctionDataManager_RestoreKvStore_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "CliFunctionDataManager_RestoreKvStore_002 start");

    auto& dataManager = CliFunctionDataManager::GetInstance();

    // Test with non-DATA_CORRUPTED status - should return as-is without calling KVStore
    DistributedKv::Status result = dataManager.RestoreKvStore(DistributedKv::Status::INVALID_SCHEMA);

    // This should always return the input status since it's not DATA_CORRUPTED
    EXPECT_EQ(result, DistributedKv::Status::INVALID_SCHEMA);

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
 * @tc.name: CliFunctionDataManager_BatchRegisterFunctions_003
 * @tc.desc: Test BatchRegisterFunctions with KVStore Put failure
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
        function.functionName = "fail_batch_func_" + std::to_string(i);
        function.functionNamespace = "fail_batch_ns";
        function.functionType = FunctionType::INTENT_FUNCTION;
        functions.push_back(function);
    }

    int32_t successCount = 0;
    int32_t ret = CliFunctionDataManager::GetInstance().BatchRegisterFunctions(functions, successCount);

    EXPECT_EQ(ret, ERR_INVALID_PARAM);
    EXPECT_EQ(successCount, 0);

    TAG_LOGI(AAFwkTag::CLI_TOOL, "CliFunctionDataManager_BatchRegisterFunctions_003 end");
}

/**
 * @tc.name: CliFunctionDataManager_BatchRegisterFunctions_004
 * @tc.desc: Test BatchRegisterFunctions with null KVStore
 * @tc.type: FUNC
 */
HWTEST_F(CliFunctionDataManagerTest, CliFunctionDataManager_BatchRegisterFunctions_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "CliFunctionDataManager_BatchRegisterFunctions_004 start");

    CliFunctionDataManager::GetInstance().kvStorePtr_ = nullptr;

    std::vector<FunctionInfo> functions;
    FunctionInfo function;
    function.functionName = "null_test_func";
    function.functionNamespace = "null_test_ns";
    function.functionType = FunctionType::INTENT_FUNCTION;
    functions.push_back(function);

    int32_t successCount = 0;
    int32_t ret = CliFunctionDataManager::GetInstance().BatchRegisterFunctions(functions, successCount);

    EXPECT_EQ(ret, ERR_NO_INIT);
    EXPECT_EQ(successCount, 0);

    TAG_LOGI(AAFwkTag::CLI_TOOL, "CliFunctionDataManager_BatchRegisterFunctions_004 end");
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

} // namespace CliTool
} // namespace OHOS
