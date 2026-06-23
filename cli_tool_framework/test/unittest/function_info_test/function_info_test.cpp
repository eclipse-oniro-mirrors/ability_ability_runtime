/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License"),
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
#include <parcel.h>

#include "function_info.h"
#include "hilog_tag_wrapper.h"

using namespace testing::ext;

namespace OHOS {
namespace CliTool {

class FunctionInfoTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void FunctionInfoTest::SetUpTestCase(void) {}
void FunctionInfoTest::TearDownTestCase(void) {}
void FunctionInfoTest::SetUp() {}
void FunctionInfoTest::TearDown() {}

// ==================== DefaultConstructor Tests ====================

/**
 * @tc.name: FunctionInfo_DefaultConstructor_0100
 * @tc.desc: Test FunctionInfo default constructor
 * @tc.type: FUNC
 */
HWTEST_F(FunctionInfoTest, FunctionInfo_DefaultConstructor_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "FunctionInfo_DefaultConstructor_0100 start");

    FunctionInfo function;

    EXPECT_TRUE(function.functionName.empty());
    EXPECT_TRUE(function.functionNamespace.empty());
    EXPECT_TRUE(function.version.empty());
    EXPECT_TRUE(function.description.empty());
    EXPECT_TRUE(function.inputSchema.empty());
    EXPECT_TRUE(function.outputSchema.empty());
    EXPECT_EQ(function.functionType, FunctionType::INTENT_FUNCTION);

    TAG_LOGI(AAFwkTag::TEST, "FunctionInfo_DefaultConstructor_0100 end");
}

// ==================== Marshalling Tests ====================

/**
 * @tc.name: FunctionInfo_Marshalling_0100
 * @tc.desc: Test FunctionInfo Marshalling with full data
 * @tc.type: FUNC
 */
HWTEST_F(FunctionInfoTest, FunctionInfo_Marshalling_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "FunctionInfo_Marshalling_0100 start");

    FunctionInfo function;
    function.functionName = "testFunction";
    function.functionNamespace = "com.test.namespace";
    function.version = "1.0.0";
    function.description = "Test function description";
    function.inputSchema = R"({"type": "object", "properties": {"arg1": {"type": "string"}}})";
    function.outputSchema = R"({"type": "string"})";
    function.functionType = FunctionType::INTENT_FUNCTION;

    Parcel parcel;
    bool ret = function.Marshalling(parcel);

    EXPECT_TRUE(ret);

    TAG_LOGI(AAFwkTag::TEST, "FunctionInfo_Marshalling_0100 end");
}

/**
 * @tc.name: FunctionInfo_Marshalling_0200
 * @tc.desc: Test FunctionInfo Marshalling with empty data
 * @tc.type: FUNC
 */
HWTEST_F(FunctionInfoTest, FunctionInfo_Marshalling_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "FunctionInfo_Marshalling_0200 start");

    FunctionInfo function;
    function.functionName = "testFunction";
    function.functionNamespace = "com.test.namespace";
    function.version = "";
    function.description = "";
    function.inputSchema = "";
    function.outputSchema = "";
    function.functionType = FunctionType::INTENT_FUNCTION;

    Parcel parcel;
    bool ret = function.Marshalling(parcel);

    EXPECT_TRUE(ret);

    TAG_LOGI(AAFwkTag::TEST, "FunctionInfo_Marshalling_0200 end");
}

/**
 * @tc.name: FunctionInfo_Marshalling_0300
 * @tc.desc: Test FunctionInfo Marshalling with complex schemas
 * @tc.type: FUNC
 */
HWTEST_F(FunctionInfoTest, FunctionInfo_Marshalling_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "FunctionInfo_Marshalling_0300 start");

    FunctionInfo function;
    function.functionName = "complexFunction";
    function.functionNamespace = "com.test.complex";
    function.version = "2.0.0";
    function.description = "Complex function with nested schemas";
    function.inputSchema = R"({
        "type": "object",
        "properties": {
            "name": {"type": "string", "description": "User name"},
            "age": {"type": "number", "minimum": 0},
            "address": {
                "type": "object",
                "properties": {
                    "city": {"type": "string"},
                    "country": {"type": "string"}
                },
                "required": ["city"]
            }
        },
        "required": ["name"]
    })";
    function.outputSchema = R"({"type": "array", "items": {"type": "string"}})";
    function.functionType = FunctionType::INTENT_FUNCTION;

    Parcel parcel;
    bool ret = function.Marshalling(parcel);

    EXPECT_TRUE(ret);

    TAG_LOGI(AAFwkTag::TEST, "FunctionInfo_Marshalling_0300 end");
}

// ==================== Unmarshalling Tests ====================

/**
 * @tc.name: FunctionInfo_Unmarshalling_0100
 * @tc.desc: Test FunctionInfo Unmarshalling with full data
 * @tc.type: FUNC
 */
HWTEST_F(FunctionInfoTest, FunctionInfo_Unmarshalling_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "FunctionInfo_Unmarshalling_0100 start");

    FunctionInfo original;
    original.functionName = "originalFunction";
    original.functionNamespace = "com.test.original";
    original.version = "1.0.0";
    original.description = "Original function";
    original.inputSchema = R"({"type": "object"})";
    original.outputSchema = R"({"type": "string"})";
    original.functionType = FunctionType::INTENT_FUNCTION;

    Parcel parcel;
    ASSERT_TRUE(original.Marshalling(parcel));

    parcel.RewindRead(0);
    FunctionInfo *result = FunctionInfo::Unmarshalling(parcel);

    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->functionName, "originalFunction");
    EXPECT_EQ(result->functionNamespace, "com.test.original");
    EXPECT_EQ(result->version, "1.0.0");
    EXPECT_EQ(result->description, "Original function");
    EXPECT_FALSE(result->inputSchema.empty());
    EXPECT_FALSE(result->outputSchema.empty());
    EXPECT_EQ(result->functionType, FunctionType::INTENT_FUNCTION);

    delete result;

    TAG_LOGI(AAFwkTag::TEST, "FunctionInfo_Unmarshalling_0100 end");
}

/**
 * @tc.name: FunctionInfo_Unmarshalling_0200
 * @tc.desc: Test FunctionInfo Unmarshalling with empty data
 * @tc.type: FUNC
 */
HWTEST_F(FunctionInfoTest, FunctionInfo_Unmarshalling_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "FunctionInfo_Unmarshalling_0200 start");

    FunctionInfo original;
    original.functionName = "emptyFunction";
    original.functionNamespace = "com.test.empty";
    original.version = "";
    original.description = "";
    original.inputSchema = "";
    original.outputSchema = "";
    original.functionType = FunctionType::INTENT_FUNCTION;

    Parcel parcel;
    ASSERT_TRUE(original.Marshalling(parcel));

    parcel.RewindRead(0);
    FunctionInfo *result = FunctionInfo::Unmarshalling(parcel);

    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->functionName, "emptyFunction");
    EXPECT_EQ(result->functionNamespace, "com.test.empty");

    delete result;

    TAG_LOGI(AAFwkTag::TEST, "FunctionInfo_Unmarshalling_0200 end");
}

/**
 * @tc.name: FunctionInfo_Unmarshalling_0300
 * @tc.desc: Test FunctionInfo Unmarshalling fails with empty parcel
 * @tc.type: FUNC
 */
HWTEST_F(FunctionInfoTest, FunctionInfo_Unmarshalling_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "FunctionInfo_Unmarshalling_0300 start");

    Parcel parcel;
    FunctionInfo *result = FunctionInfo::Unmarshalling(parcel);

    EXPECT_EQ(result, nullptr);

    TAG_LOGI(AAFwkTag::TEST, "FunctionInfo_Unmarshalling_0300 end");
}

/**
 * @tc.name: FunctionInfo_Unmarshalling_0400
 * @tc.desc: Test FunctionInfo Unmarshalling fails when functionName is missing
 * @tc.type: FUNC
 */
HWTEST_F(FunctionInfoTest, FunctionInfo_Unmarshalling_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "FunctionInfo_Unmarshalling_0400 start");

    Parcel parcel;
    // Write only part of the data
    ASSERT_TRUE(parcel.WriteString("partialFunction"));
    ASSERT_TRUE(parcel.WriteString("com.test.partial"));
    parcel.RewindRead(0);

    FunctionInfo *result = FunctionInfo::Unmarshalling(parcel);

    EXPECT_EQ(result, nullptr);

    TAG_LOGI(AAFwkTag::TEST, "FunctionInfo_Unmarshalling_0400 end");
}

/**
 * @tc.name: FunctionInfo_Unmarshalling_0500
 * @tc.desc: Test FunctionInfo Unmarshalling fails with invalid functionType
 * @tc.type: FUNC
 */
HWTEST_F(FunctionInfoTest, FunctionInfo_Unmarshalling_0500, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "FunctionInfo_Unmarshalling_0500 start");

    Parcel parcel;
    ASSERT_TRUE(parcel.WriteString("testFunction"));
    ASSERT_TRUE(parcel.WriteString("com.test.invalid"));
    ASSERT_TRUE(parcel.WriteString("1.0.0"));
    ASSERT_TRUE(parcel.WriteString("Test description"));
    ASSERT_TRUE(parcel.WriteString("{}"));
    ASSERT_TRUE(parcel.WriteString("{}"));
    // Write invalid functionType value (out of range)
    ASSERT_TRUE(parcel.WriteInt32(999));

    parcel.RewindRead(0);
    FunctionInfo *result = FunctionInfo::Unmarshalling(parcel);

    EXPECT_EQ(result, nullptr);

    TAG_LOGI(AAFwkTag::TEST, "FunctionInfo_Unmarshalling_0500 end");
}

/**
 * @tc.name: FunctionInfo_Unmarshalling_0600
 * @tc.desc: Test FunctionInfo Unmarshalling fails with negative functionType
 * @tc.type: FUNC
 */
HWTEST_F(FunctionInfoTest, FunctionInfo_Unmarshalling_0600, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "FunctionInfo_Unmarshalling_0600 start");

    Parcel parcel;
    ASSERT_TRUE(parcel.WriteString("testFunction"));
    ASSERT_TRUE(parcel.WriteString("com.test.negative"));
    ASSERT_TRUE(parcel.WriteString("1.0.0"));
    ASSERT_TRUE(parcel.WriteString("Test description"));
    ASSERT_TRUE(parcel.WriteString("{}"));
    ASSERT_TRUE(parcel.WriteString("{}"));
    // Write negative functionType value
    ASSERT_TRUE(parcel.WriteInt32(-1));

    parcel.RewindRead(0);
    FunctionInfo *result = FunctionInfo::Unmarshalling(parcel);

    EXPECT_EQ(result, nullptr);

    TAG_LOGI(AAFwkTag::TEST, "FunctionInfo_Unmarshalling_0600 end");
}

/**
 * @tc.name: FunctionInfo_Unmarshalling_0700
 * @tc.desc: Test FunctionInfo Unmarshalling with complex schemas
 * @tc.type: FUNC
 */
HWTEST_F(FunctionInfoTest, FunctionInfo_Unmarshalling_0700, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "FunctionInfo_Unmarshalling_0700 start");

    FunctionInfo original;
    original.functionName = "complexFunction";
    original.functionNamespace = "com.test.complex";
    original.version = "2.0.0";
    original.description = "Complex function";
    original.inputSchema = R"({
        "type": "object",
        "properties": {
            "name": {"type": "string"},
            "age": {"type": "number"}
        }
    })";
    original.outputSchema = R"({"type": "array", "items": {"type": "string"}})";
    original.functionType = FunctionType::INTENT_FUNCTION;

    Parcel parcel;
    ASSERT_TRUE(original.Marshalling(parcel));

    parcel.RewindRead(0);
    FunctionInfo *result = FunctionInfo::Unmarshalling(parcel);

    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->functionName, "complexFunction");
    EXPECT_EQ(result->functionNamespace, "com.test.complex");
    EXPECT_EQ(result->version, "2.0.0");

    delete result;

    TAG_LOGI(AAFwkTag::TEST, "FunctionInfo_Unmarshalling_0700 end");
}

// ==================== RoundTrip Tests ====================

/**
 * @tc.name: FunctionInfo_Marshalling_Unmarshalling_RoundTrip_0100
 * @tc.desc: Test FunctionInfo Marshalling and Unmarshalling round trip
 * @tc.type: FUNC
 */
HWTEST_F(FunctionInfoTest, FunctionInfo_Marshalling_Unmarshalling_RoundTrip_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "FunctionInfo_Marshalling_Unmarshalling_RoundTrip_0100 start");

    FunctionInfo original;
    original.functionName = "roundTripFunction";
    original.functionNamespace = "com.test.roundtrip";
    original.version = "1.5.0";
    original.description = "Round trip test function";
    original.inputSchema = R"({"type": "object", "properties": {"input": {"type": "string"}}})";
    original.outputSchema = R"({"type": "string"})";
    original.functionType = FunctionType::INTENT_FUNCTION;

    Parcel parcel;
    ASSERT_TRUE(original.Marshalling(parcel));

    parcel.RewindRead(0);
    FunctionInfo *result = FunctionInfo::Unmarshalling(parcel);

    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->functionName, original.functionName);
    EXPECT_EQ(result->functionNamespace, original.functionNamespace);
    EXPECT_EQ(result->version, original.version);
    EXPECT_EQ(result->description, original.description);
    EXPECT_EQ(result->inputSchema, original.inputSchema);
    EXPECT_EQ(result->outputSchema, original.outputSchema);
    EXPECT_EQ(result->functionType, original.functionType);

    delete result;

    TAG_LOGI(AAFwkTag::TEST, "FunctionInfo_Marshalling_Unmarshalling_RoundTrip_0100 end");
}

// ==================== ParseFromJson Tests ====================

/**
 * @tc.name: FunctionInfo_ParseFromJson_0100
 * @tc.desc: Test FunctionInfo ParseFromJson with full data
 * @tc.type: FUNC
 */
HWTEST_F(FunctionInfoTest, FunctionInfo_ParseFromJson_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "FunctionInfo_ParseFromJson_0100 start");

    nlohmann::json json = R"({
        "functionName": "jsonFunction",
        "functionNamespace": "com.test.json",
        "version": "1.0.0",
        "description": "Function from JSON",
        "inputSchema": "{\"type\": \"object\", \"properties\": {\"arg1\": {\"type\": \"string\"}}}",
        "outputSchema": "{\"type\": \"string\"}",
        "functionType": 0
    })"_json;

    FunctionInfo function;
    bool result = FunctionInfo::ParseFromJson(json, function);

    EXPECT_TRUE(result);
    EXPECT_EQ(function.functionName, "jsonFunction");
    EXPECT_EQ(function.functionNamespace, "com.test.json");
    EXPECT_EQ(function.version, "1.0.0");
    EXPECT_EQ(function.description, "Function from JSON");
    EXPECT_FALSE(function.inputSchema.empty());
    EXPECT_FALSE(function.outputSchema.empty());
    EXPECT_EQ(function.functionType, FunctionType::INTENT_FUNCTION);

    TAG_LOGI(AAFwkTag::TEST, "FunctionInfo_ParseFromJson_0100 end");
}

/**
 * @tc.name: FunctionInfo_ParseFromJson_0200
 * @tc.desc: Test FunctionInfo ParseFromJson with minimal required data
 * @tc.type: FUNC
 */
HWTEST_F(FunctionInfoTest, FunctionInfo_ParseFromJson_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "FunctionInfo_ParseFromJson_0200 start");

    nlohmann::json json = R"({
        "functionName": "minimalFunction",
        "functionNamespace": "com.test.minimal",
        "functionType": 0,
        "version": "1.0.0",
        "description": ""
    })"_json;

    FunctionInfo function;
    bool result = FunctionInfo::ParseFromJson(json, function);

    EXPECT_TRUE(result);
    EXPECT_EQ(function.functionName, "minimalFunction");
    EXPECT_EQ(function.functionNamespace, "com.test.minimal");
    EXPECT_EQ(function.version, "1.0.0");
    EXPECT_TRUE(function.description.empty());
    EXPECT_TRUE(function.inputSchema.empty());
    EXPECT_TRUE(function.outputSchema.empty());

    TAG_LOGI(AAFwkTag::TEST, "FunctionInfo_ParseFromJson_0200 end");
}

/**
 * @tc.name: FunctionInfo_ParseFromJson_0300
 * @tc.desc: Test FunctionInfo ParseFromJson with empty JSON
 * @tc.type: FUNC
 */
HWTEST_F(FunctionInfoTest, FunctionInfo_ParseFromJson_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "FunctionInfo_ParseFromJson_0300 start");

    nlohmann::json json;

    FunctionInfo function;
    bool result = FunctionInfo::ParseFromJson(json, function);

    EXPECT_FALSE(result);

    TAG_LOGI(AAFwkTag::TEST, "FunctionInfo_ParseFromJson_0300 end");
}

/**
 * @tc.name: FunctionInfo_ParseFromJson_0400
 * @tc.desc: Test FunctionInfo ParseFromJson without functionName
 * @tc.type: FUNC
 */
HWTEST_F(FunctionInfoTest, FunctionInfo_ParseFromJson_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "FunctionInfo_ParseFromJson_0400 start");

    nlohmann::json json = R"({
        "functionNamespace": "com.test.missing",
        "functionType": 0,
        "version": "1.0.0",
        "description": ""
    })"_json;

    FunctionInfo function;
    bool result = FunctionInfo::ParseFromJson(json, function);

    EXPECT_FALSE(result);

    TAG_LOGI(AAFwkTag::TEST, "FunctionInfo_ParseFromJson_0400 end");
}

/**
 * @tc.name: FunctionInfo_ParseFromJson_0500
 * @tc.desc: Test FunctionInfo ParseFromJson with empty functionName
 * @tc.type: FUNC
 */
HWTEST_F(FunctionInfoTest, FunctionInfo_ParseFromJson_0500, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "FunctionInfo_ParseFromJson_0500 start");

    nlohmann::json json = R"({
        "functionName": "",
        "functionNamespace": "com.test.empty",
        "functionType": 0,
        "version": "1.0.0",
        "description": ""
    })"_json;

    FunctionInfo function;
    bool result = FunctionInfo::ParseFromJson(json, function);

    EXPECT_FALSE(result);

    TAG_LOGI(AAFwkTag::TEST, "FunctionInfo_ParseFromJson_0500 end");
}

/**
 * @tc.name: FunctionInfo_ParseFromJson_0600
 * @tc.desc: Test FunctionInfo ParseFromJson without functionNamespace
 * @tc.type: FUNC
 */
HWTEST_F(FunctionInfoTest, FunctionInfo_ParseFromJson_0600, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "FunctionInfo_ParseFromJson_0600 start");

    nlohmann::json json = R"({
        "functionName": "missingNamespace",
        "functionType": 0,
        "version": "1.0.0",
        "description": ""
    })"_json;

    FunctionInfo function;
    bool result = FunctionInfo::ParseFromJson(json, function);

    EXPECT_FALSE(result);

    TAG_LOGI(AAFwkTag::TEST, "FunctionInfo_ParseFromJson_0600 end");
}

/**
 * @tc.name: FunctionInfo_ParseFromJson_0700
 * @tc.desc: Test FunctionInfo ParseFromJson with empty functionNamespace
 * @tc.type: FUNC
 */
HWTEST_F(FunctionInfoTest, FunctionInfo_ParseFromJson_0700, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "FunctionInfo_ParseFromJson_0700 start");

    nlohmann::json json = R"({
        "functionName": "emptyNamespace",
        "functionNamespace": "",
        "functionType": 0,
        "version": "1.0.0",
        "description": ""
    })"_json;

    FunctionInfo function;
    bool result = FunctionInfo::ParseFromJson(json, function);

    EXPECT_FALSE(result);

    TAG_LOGI(AAFwkTag::TEST, "FunctionInfo_ParseFromJson_0700 end");
}

/**
 * @tc.name: FunctionInfo_ParseFromJson_0800
 * @tc.desc: Test FunctionInfo ParseFromJson without functionType
 * @tc.type: FUNC
 */
HWTEST_F(FunctionInfoTest, FunctionInfo_ParseFromJson_0800, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "FunctionInfo_ParseFromJson_0800 start");

    nlohmann::json json = R"({
        "functionName": "missingType",
        "functionNamespace": "com.test.missing",
        "version": "1.0.0",
        "description": ""
    })"_json;

    FunctionInfo function;
    bool result = FunctionInfo::ParseFromJson(json, function);

    EXPECT_FALSE(result);

    TAG_LOGI(AAFwkTag::TEST, "FunctionInfo_ParseFromJson_0800 end");
}

/**
 * @tc.name: FunctionInfo_ParseFromJson_0900
 * @tc.desc: Test FunctionInfo ParseFromJson with invalid functionType (string)
 * @tc.type: FUNC
 */
HWTEST_F(FunctionInfoTest, FunctionInfo_ParseFromJson_0900, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "FunctionInfo_ParseFromJson_0900 start");

    nlohmann::json json = R"({
        "functionName": "invalidType",
        "functionNamespace": "com.test.invalid",
        "functionType": "invalid",
        "version": "1.0.0",
        "description": ""
    })"_json;

    FunctionInfo function;
    bool result = FunctionInfo::ParseFromJson(json, function);

    EXPECT_FALSE(result);

    TAG_LOGI(AAFwkTag::TEST, "FunctionInfo_ParseFromJson_0900 end");
}

/**
 * @tc.name: FunctionInfo_ParseFromJson_1000
 * @tc.desc: Test FunctionInfo ParseFromJson with invalid functionType (out of range)
 * @tc.type: FUNC
 */
HWTEST_F(FunctionInfoTest, FunctionInfo_ParseFromJson_1000, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "FunctionInfo_ParseFromJson_1000 start");

    nlohmann::json json = R"({
        "functionName": "outOfRange",
        "functionNamespace": "com.test.outofrange",
        "functionType": 999,
        "version": "1.0.0",
        "description": ""
    })"_json;

    FunctionInfo function;
    bool result = FunctionInfo::ParseFromJson(json, function);

    EXPECT_FALSE(result);

    TAG_LOGI(AAFwkTag::TEST, "FunctionInfo_ParseFromJson_1000 end");
}

/**
 * @tc.name: FunctionInfo_ParseFromJson_1100
 * @tc.desc: Test FunctionInfo ParseFromJson with valid inputSchema
 * @tc.type: FUNC
 */
HWTEST_F(FunctionInfoTest, FunctionInfo_ParseFromJson_1100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "FunctionInfo_ParseFromJson_1100 start");

    nlohmann::json json = R"({
        "functionName": "validInput",
        "functionNamespace": "com.test.validinput",
        "functionType": 0,
        "inputSchema": "{\"type\": \"object\", \"properties\": {\"name\": {\"type\": \"string\"}}}",
        "version": "1.0.0",
        "description": ""
    })"_json;

    FunctionInfo function;
    bool result = FunctionInfo::ParseFromJson(json, function);

    EXPECT_TRUE(result);
    EXPECT_FALSE(function.inputSchema.empty());

    TAG_LOGI(AAFwkTag::TEST, "FunctionInfo_ParseFromJson_1100 end");
}

/**
 * @tc.name: FunctionInfo_ParseFromJson_1200
 * @tc.desc: Test FunctionInfo ParseFromJson with invalid inputSchema (not object)
 * @tc.type: FUNC
 */
HWTEST_F(FunctionInfoTest, FunctionInfo_ParseFromJson_1200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "FunctionInfo_ParseFromJson_1200 start");

    nlohmann::json json = R"({
        "functionName": "invalidInput",
        "functionNamespace": "com.test.invalidinput",
        "functionType": 0,
        "inputSchema": "not an object",
        "version": "1.0.0",
        "description": ""
    })"_json;

    FunctionInfo function;
    bool result = FunctionInfo::ParseFromJson(json, function);

    EXPECT_FALSE(result);

    TAG_LOGI(AAFwkTag::TEST, "FunctionInfo_ParseFromJson_1200 end");
}

/**
 * @tc.name: FunctionInfo_ParseFromJson_1300
 * @tc.desc: Test FunctionInfo ParseFromJson with valid outputSchema
 * @tc.type: FUNC
 */
HWTEST_F(FunctionInfoTest, FunctionInfo_ParseFromJson_1300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "FunctionInfo_ParseFromJson_1300 start");

    nlohmann::json json = R"({
        "functionName": "validOutput",
        "functionNamespace": "com.test.validoutput",
        "functionType": 0,
        "outputSchema": "{\"type\": \"string\"}",
        "version": "1.0.0",
        "description": ""
    })"_json;

    FunctionInfo function;
    bool result = FunctionInfo::ParseFromJson(json, function);

    EXPECT_TRUE(result);
    EXPECT_FALSE(function.outputSchema.empty());

    TAG_LOGI(AAFwkTag::TEST, "FunctionInfo_ParseFromJson_1300 end");
}

/**
 * @tc.name: FunctionInfo_ParseFromJson_1400
 * @tc.desc: Test FunctionInfo ParseFromJson with invalid outputSchema (not object)
 * @tc.type: FUNC
 */
HWTEST_F(FunctionInfoTest, FunctionInfo_ParseFromJson_1400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "FunctionInfo_ParseFromJson_1400 start");

    nlohmann::json json = R"({
        "functionName": "invalidOutput",
        "functionNamespace": "com.test.invalidoutput",
        "functionType": 0,
        "outputSchema": 123,
        "version": "1.0.0",
        "description": ""
    })"_json;

    FunctionInfo function;
    bool result = FunctionInfo::ParseFromJson(json, function);

    EXPECT_FALSE(result);

    TAG_LOGI(AAFwkTag::TEST, "FunctionInfo_ParseFromJson_1400 end");
}

/**
 * @tc.name: FunctionInfo_ParseFromJson_1500
 * @tc.desc: Test FunctionInfo ParseFromJson with non-string functionName
 * @tc.type: FUNC
 */
HWTEST_F(FunctionInfoTest, FunctionInfo_ParseFromJson_1500, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "FunctionInfo_ParseFromJson_1500 start");

    nlohmann::json json = R"({
        "functionName": 123,
        "functionNamespace": "com.test.nonstring",
        "functionType": 0,
        "version": "1.0.0",
        "description": ""
    })"_json;

    FunctionInfo function;
    bool result = FunctionInfo::ParseFromJson(json, function);

    EXPECT_FALSE(result);

    TAG_LOGI(AAFwkTag::TEST, "FunctionInfo_ParseFromJson_1500 end");
}

/**
 * @tc.name: FunctionInfo_ParseFromJson_1600
 * @tc.desc: Test FunctionInfo ParseFromJson with non-string functionNamespace
 * @tc.type: FUNC
 */
HWTEST_F(FunctionInfoTest, FunctionInfo_ParseFromJson_1600, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "FunctionInfo_ParseFromJson_1600 start");

    nlohmann::json json = R"({
        "functionName": "test",
        "functionNamespace": 456,
        "functionType": 0,
        "version": "1.0.0",
        "description": ""
    })"_json;

    FunctionInfo function;
    bool result = FunctionInfo::ParseFromJson(json, function);

    EXPECT_FALSE(result);

    TAG_LOGI(AAFwkTag::TEST, "FunctionInfo_ParseFromJson_1600 end");
}

// ==================== ParseToJson Tests ====================

/**
 * @tc.name: FunctionInfo_ParseToJson_0100
 * @tc.desc: Test FunctionInfo ParseToJson with full data
 * @tc.type: FUNC
 */
HWTEST_F(FunctionInfoTest, FunctionInfo_ParseToJson_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "FunctionInfo_ParseToJson_0100 start");

    FunctionInfo function;
    function.functionName = "toJsonFunction";
    function.functionNamespace = "com.test.tojson";
    function.version = "1.0.0";
    function.description = "Function to JSON";
    function.inputSchema = R"({"type": "object", "properties": {"arg1": {"type": "string"}}})";
    function.outputSchema = R"({"type": "string"})";
    function.functionType = FunctionType::INTENT_FUNCTION;

    nlohmann::json json = function.ParseToJson();

    EXPECT_EQ(json["functionName"], "toJsonFunction");
    EXPECT_EQ(json["functionNamespace"], "com.test.tojson");
    EXPECT_EQ(json["version"], "1.0.0");
    EXPECT_EQ(json["description"], "Function to JSON");

    // inputSchema and outputSchema are stored as strings
    ASSERT_TRUE(json["inputSchema"].is_string());
    ASSERT_TRUE(json["outputSchema"].is_string());

    // Parse and validate the schema strings
    nlohmann::json inputSchemaJson = nlohmann::json::parse(json["inputSchema"].get<std::string>());
    nlohmann::json outputSchemaJson = nlohmann::json::parse(json["outputSchema"].get<std::string>());

    EXPECT_EQ(inputSchemaJson["type"], "object");
    EXPECT_EQ(outputSchemaJson["type"], "string");
    EXPECT_EQ(json["functionType"], 0);

    TAG_LOGI(AAFwkTag::TEST, "FunctionInfo_ParseToJson_0100 end");
}

/**
 * @tc.name: FunctionInfo_ParseToJson_0200
 * @tc.desc: Test FunctionInfo ParseToJson with empty data
 * @tc.type: FUNC
 */
HWTEST_F(FunctionInfoTest, FunctionInfo_ParseToJson_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "FunctionInfo_ParseToJson_0200 start");

    FunctionInfo function;
    function.functionName = "emptyFunction";
    function.functionNamespace = "com.test.emptyjson";
    function.inputSchema = "";
    function.outputSchema = "";
    function.functionType = FunctionType::INTENT_FUNCTION;

    nlohmann::json json = function.ParseToJson();

    EXPECT_EQ(json["functionName"], "emptyFunction");
    EXPECT_EQ(json["functionNamespace"], "com.test.emptyjson");
    EXPECT_FALSE(json.contains("inputSchema"));
    EXPECT_FALSE(json.contains("outputSchema"));

    TAG_LOGI(AAFwkTag::TEST, "FunctionInfo_ParseToJson_0200 end");
}

/**
 * @tc.name: FunctionInfo_ParseToJson_0300
 * @tc.desc: Test FunctionInfo ParseToJson with invalid inputSchema string
 * @tc.type: FUNC
 */
HWTEST_F(FunctionInfoTest, FunctionInfo_ParseToJson_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "FunctionInfo_ParseToJson_0300 start");

    FunctionInfo function;
    function.functionName = "invalidInput";
    function.functionNamespace = "com.test.invalidinput";
    function.inputSchema = "invalid json string";
    function.outputSchema = R"({"type": "string"})";
    function.functionType = FunctionType::INTENT_FUNCTION;

    nlohmann::json json = function.ParseToJson();

    EXPECT_TRUE(json.contains("inputSchema"));
    EXPECT_EQ(json["inputSchema"], "invalid json string");
    EXPECT_TRUE(json.contains("outputSchema"));

    // outputSchema is stored as string, parse it to validate
    ASSERT_TRUE(json["outputSchema"].is_string());
    nlohmann::json outputSchemaJson = nlohmann::json::parse(json["outputSchema"].get<std::string>());
    EXPECT_EQ(outputSchemaJson["type"], "string");

    TAG_LOGI(AAFwkTag::TEST, "FunctionInfo_ParseToJson_0300 end");
}

/**
 * @tc.name: FunctionInfo_ParseToJson_0400
 * @tc.desc: Test FunctionInfo ParseToJson with invalid outputSchema string
 * @tc.type: FUNC
 */
HWTEST_F(FunctionInfoTest, FunctionInfo_ParseToJson_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "FunctionInfo_ParseToJson_0400 start");

    FunctionInfo function;
    function.functionName = "invalidOutput";
    function.functionNamespace = "com.test.invalidoutput";
    function.inputSchema = R"({"type": "object"})";
    function.outputSchema = "{invalid json}";
    function.functionType = FunctionType::INTENT_FUNCTION;

    nlohmann::json json = function.ParseToJson();

    EXPECT_TRUE(json.contains("inputSchema"));

    // inputSchema is stored as string, parse it to validate
    ASSERT_TRUE(json["inputSchema"].is_string());
    nlohmann::json inputSchemaJson = nlohmann::json::parse(json["inputSchema"].get<std::string>());
    EXPECT_EQ(inputSchemaJson["type"], "object");

    EXPECT_TRUE(json.contains("outputSchema"));
    EXPECT_EQ(json["outputSchema"], "{invalid json}");

    TAG_LOGI(AAFwkTag::TEST, "FunctionInfo_ParseToJson_0400 end");
}

/**
 * @tc.name: FunctionInfo_ParseToJson_0500
 * @tc.desc: Test FunctionInfo ParseToJson with complex schemas
 * @tc.type: FUNC
 */
HWTEST_F(FunctionInfoTest, FunctionInfo_ParseToJson_0500, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "FunctionInfo_ParseToJson_0500 start");

    FunctionInfo function;
    function.functionName = "complexJson";
    function.functionNamespace = "com.test.complexjson";
    function.inputSchema = R"({
        "type": "object",
        "properties": {
            "name": {"type": "string", "description": "User name"},
            "age": {"type": "number", "minimum": 0}
        },
        "required": ["name"]
    })";
    function.outputSchema = R"({"type": "array", "items": {"type": "string"}})";
    function.functionType = FunctionType::INTENT_FUNCTION;

    nlohmann::json json = function.ParseToJson();

    // Schemas are stored as strings, parse them to validate
    ASSERT_TRUE(json["inputSchema"].is_string());
    ASSERT_TRUE(json["outputSchema"].is_string());

    nlohmann::json inputSchemaJson = nlohmann::json::parse(json["inputSchema"].get<std::string>());
    nlohmann::json outputSchemaJson = nlohmann::json::parse(json["outputSchema"].get<std::string>());

    EXPECT_TRUE(inputSchemaJson["properties"].is_object());
    EXPECT_EQ(inputSchemaJson["properties"]["name"]["type"], "string");
    EXPECT_EQ(outputSchemaJson["type"], "array");

    TAG_LOGI(AAFwkTag::TEST, "FunctionInfo_ParseToJson_0500 end");
}

// ==================== ParseFromJson/ParseToJson RoundTrip Tests ====================

/**
 * @tc.name: FunctionInfo_ParseFromJson_ParseToJson_RoundTrip_0100
 * @tc.desc: Test FunctionInfo ParseFromJson and ParseToJson round trip
 * @tc.type: FUNC
 */
HWTEST_F(FunctionInfoTest, FunctionInfo_ParseFromJson_ParseToJson_RoundTrip_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "FunctionInfo_ParseFromJson_ParseToJson_RoundTrip_0100 start");

    nlohmann::json originalJson = R"({
        "functionName": "roundTripJson",
        "functionNamespace": "com.test.roundtripjson",
        "version": "1.0.0",
        "description": "Round trip JSON test",
        "inputSchema": "{\"type\": \"object\", \"properties\": {\"input\": {\"type\": \"string\"}}}",
        "outputSchema": "{\"type\": \"string\"}",
        "functionType": 0
    })"_json;

    FunctionInfo function;
    bool result = FunctionInfo::ParseFromJson(originalJson, function);
    EXPECT_TRUE(result);

    nlohmann::json resultJson = function.ParseToJson();

    EXPECT_EQ(resultJson["functionName"], originalJson["functionName"]);
    EXPECT_EQ(resultJson["functionNamespace"], originalJson["functionNamespace"]);
    EXPECT_EQ(resultJson["version"], originalJson["version"]);
    EXPECT_EQ(resultJson["description"], originalJson["description"]);
    EXPECT_EQ(resultJson["functionType"], originalJson["functionType"]);

    TAG_LOGI(AAFwkTag::TEST, "FunctionInfo_ParseFromJson_ParseToJson_RoundTrip_0100 end");
}

// ==================== Validate Tests ====================

/**
 * @tc.name: FunctionInfo_Validate_0100
 * @tc.desc: Test FunctionInfo::Validate with valid data
 * @tc.type: FUNC
 */
HWTEST_F(FunctionInfoTest, FunctionInfo_Validate_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "FunctionInfo_Validate_0100 start");

    FunctionInfo function;
    function.functionName = "validFunction";
    function.functionNamespace = "com.test.valid";
    function.inputSchema = R"({"type": "object"})";
    function.outputSchema = R"({"type": "string"})";
    function.functionType = FunctionType::INTENT_FUNCTION;

    EXPECT_TRUE(FunctionInfo::Validate(function));

    TAG_LOGI(AAFwkTag::TEST, "FunctionInfo_Validate_0100 end");
}

/**
 * @tc.name: FunctionInfo_Validate_0200
 * @tc.desc: Test FunctionInfo::Validate with empty functionName
 * @tc.type: FUNC
 */
HWTEST_F(FunctionInfoTest, FunctionInfo_Validate_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "FunctionInfo_Validate_0200 start");

    FunctionInfo function;
    function.functionName = "";
    function.functionNamespace = "com.test.emptyname";
    function.inputSchema = R"({"type": "object"})";
    function.outputSchema = R"({"type": "string"})";
    function.functionType = FunctionType::INTENT_FUNCTION;

    EXPECT_FALSE(FunctionInfo::Validate(function));

    TAG_LOGI(AAFwkTag::TEST, "FunctionInfo_Validate_0200 end");
}

/**
 * @tc.name: FunctionInfo_Validate_0300
 * @tc.desc: Test FunctionInfo::Validate with empty functionNamespace
 * @tc.type: FUNC
 */
HWTEST_F(FunctionInfoTest, FunctionInfo_Validate_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "FunctionInfo_Validate_0300 start");

    FunctionInfo function;
    function.functionName = "testFunction";
    function.functionNamespace = "";
    function.inputSchema = R"({"type": "object"})";
    function.outputSchema = R"({"type": "string"})";
    function.functionType = FunctionType::INTENT_FUNCTION;

    EXPECT_FALSE(FunctionInfo::Validate(function));

    TAG_LOGI(AAFwkTag::TEST, "FunctionInfo_Validate_0300 end");
}

/**
 * @tc.name: FunctionInfo_Validate_0400
 * @tc.desc: Test FunctionInfo::Validate with empty inputSchema (valid)
 * @tc.type: FUNC
 */
HWTEST_F(FunctionInfoTest, FunctionInfo_Validate_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "FunctionInfo_Validate_0400 start");

    FunctionInfo function;
    function.functionName = "testFunction";
    function.functionNamespace = "com.test.emptyinput";
    function.inputSchema = "";
    function.outputSchema = R"({"type": "string"})";
    function.functionType = FunctionType::INTENT_FUNCTION;

    EXPECT_TRUE(FunctionInfo::Validate(function));

    TAG_LOGI(AAFwkTag::TEST, "FunctionInfo_Validate_0400 end");
}

/**
 * @tc.name: FunctionInfo_Validate_0500
 * @tc.desc: Test FunctionInfo::Validate with invalid inputSchema JSON
 * @tc.type: FUNC
 */
HWTEST_F(FunctionInfoTest, FunctionInfo_Validate_0500, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "FunctionInfo_Validate_0500 start");

    FunctionInfo function;
    function.functionName = "testFunction";
    function.functionNamespace = "com.test.badinput";
    function.inputSchema = "not a valid json";
    function.outputSchema = R"({"type": "string"})";
    function.functionType = FunctionType::INTENT_FUNCTION;

    EXPECT_FALSE(FunctionInfo::Validate(function));

    TAG_LOGI(AAFwkTag::TEST, "FunctionInfo_Validate_0500 end");
}

/**
 * @tc.name: FunctionInfo_Validate_0600
 * @tc.desc: Test FunctionInfo::Validate with empty outputSchema (valid)
 * @tc.type: FUNC
 */
HWTEST_F(FunctionInfoTest, FunctionInfo_Validate_0600, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "FunctionInfo_Validate_0600 start");

    FunctionInfo function;
    function.functionName = "testFunction";
    function.functionNamespace = "com.test.emptyoutput";
    function.inputSchema = R"({"type": "object"})";
    function.outputSchema = "";
    function.functionType = FunctionType::INTENT_FUNCTION;

    EXPECT_TRUE(FunctionInfo::Validate(function));

    TAG_LOGI(AAFwkTag::TEST, "FunctionInfo_Validate_0600 end");
}

/**
 * @tc.name: FunctionInfo_Validate_0700
 * @tc.desc: Test FunctionInfo::Validate with invalid outputSchema JSON
 * @tc.type: FUNC
 */
HWTEST_F(FunctionInfoTest, FunctionInfo_Validate_0700, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "FunctionInfo_Validate_0700 start");

    FunctionInfo function;
    function.functionName = "testFunction";
    function.functionNamespace = "com.test.badoutput";
    function.inputSchema = R"({"type": "object"})";
    function.outputSchema = "{invalid json}";
    function.functionType = FunctionType::INTENT_FUNCTION;

    EXPECT_FALSE(FunctionInfo::Validate(function));

    TAG_LOGI(AAFwkTag::TEST, "FunctionInfo_Validate_0700 end");
}

/**
 * @tc.name: FunctionInfo_Validate_0800
 * @tc.desc: Test FunctionInfo::Validate with all valid fields
 * @tc.type: FUNC
 */
HWTEST_F(FunctionInfoTest, FunctionInfo_Validate_0800, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "FunctionInfo_Validate_0800 start");

    FunctionInfo function;
    function.functionName = "allValidFunction";
    function.functionNamespace = "com.test.allvalid";
    function.version = "2.0.0";
    function.description = "All valid fields";
    function.inputSchema = R"({"type": "object", "properties": {"input": {"type": "string"}}})";
    function.outputSchema = R"({"type": "array", "items": {"type": "string"}})";
    function.functionType = FunctionType::INTENT_FUNCTION;

    EXPECT_TRUE(FunctionInfo::Validate(function));

    TAG_LOGI(AAFwkTag::TEST, "FunctionInfo_Validate_0800 end");
}

/**
 * @tc.name: FunctionInfo_Validate_0900
 * @tc.desc: Test FunctionInfo::Validate with minimal valid data
 * @tc.type: FUNC
 */
HWTEST_F(FunctionInfoTest, FunctionInfo_Validate_0900, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "FunctionInfo_Validate_0900 start");

    FunctionInfo function;
    function.functionName = "minimalFunction";
    function.functionNamespace = "com.test.minimal";
    function.functionType = FunctionType::INTENT_FUNCTION;

    EXPECT_TRUE(FunctionInfo::Validate(function));

    TAG_LOGI(AAFwkTag::TEST, "FunctionInfo_Validate_0900 end");
}

} // namespace CliTool
} // namespace OHOS
