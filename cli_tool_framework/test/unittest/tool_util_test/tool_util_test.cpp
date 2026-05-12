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

#include <access_token.h>
#include <cctype>
#include <functional>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <iremote_object.h>
#include <map>
#include <memory>
#include <nlohmann/json.hpp>
#include <unordered_set>
#include <vector>

#define private public
#include "tool_util.h"
#undef private

#include "array_wrapper.h"
#include "bool_wrapper.h"
#include "cli_error_code.h"
#include "double_wrapper.h"
#include "exec_tool_param.h"
#include "float_wrapper.h"
#include "int_wrapper.h"
#include "long_wrapper.h"
#include "session_record.h"
#include "skill_execute_result.h"
#include "string_wrapper.h"
#include "tool_info.h"
#include "want_params.h"
#include "want_params_wrapper.h"

using namespace testing::ext;
using namespace OHOS::CliTool;

namespace OHOS {
namespace CliTool {
namespace {
AAFwk::WantParams ConvertToWantParams(const std::map<std::string, std::string> &args)
{
    AAFwk::WantParams params;
    for (const auto &[key, value] : args) {
        if (value == "true") {
            params.SetParam(key, AAFwk::Boolean::Box(true));
            continue;
        }
        if (value == "false") {
            params.SetParam(key, AAFwk::Boolean::Box(false));
            continue;
        }
        params.SetParam(key, AAFwk::String::Box(value));
    }
    return params;
}

int32_t ValidateToolProperties(const std::string &inputSchema, const std::string &subcommand,
    const std::map<std::string, std::string> &args)
{
    if (!subcommand.empty() && subcommand != "build" && subcommand != "clean" && subcommand != "deploy") {
        return ERR_TOOL_NOT_EXIST;
    }
    return ToolUtil::ValidateInputSchemaProperties(inputSchema, ConvertToWantParams(args));
}
} // namespace

class ToolUtilTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void ToolUtilTest::SetUpTestCase(void)
{
    // Initialize test environment
}

void ToolUtilTest::TearDownTestCase(void)
{
    // Cleanup test environment
}

void ToolUtilTest::SetUp()
{
    // Reset state before each test
}

void ToolUtilTest::TearDown()
{
    // Cleanup after each test
}

/**
 * @tc.name: ToolUtil_ValidateInputSchemaProperties_0100
 * @tc.desc: Test ValidateInputSchemaProperties with empty schema
 * @tc.type: FUNC
 */
HWTEST_F(ToolUtilTest, ValidateInputSchemaProperties_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolUtil_ValidateInputSchemaProperties_0100 start";

    std::string emptySchema = "";
    std::string subcommand = "";
    std::map<std::string, std::string> args;

    int32_t result = ValidateToolProperties(emptySchema, subcommand, args);

    EXPECT_EQ(result, ERR_OK);

    GTEST_LOG_(INFO) << "ToolUtil_ValidateInputSchemaProperties_0100 end";
}

/**
 * @tc.name: ToolUtil_ValidateInputSchemaProperties_0200
 * @tc.desc: Test ValidateInputSchemaProperties with invalid JSON schema
 * @tc.type: FUNC
 */
HWTEST_F(ToolUtilTest, ValidateInputSchemaProperties_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolUtil_ValidateInputSchemaProperties_0200 start";

    std::string invalidSchema = "{invalid json}";
    std::string subcommand = "";
    std::map<std::string, std::string> args;

    int32_t result = ValidateToolProperties(invalidSchema, subcommand, args);

    EXPECT_EQ(result, ERR_OK);

    GTEST_LOG_(INFO) << "ToolUtil_ValidateInputSchemaProperties_0200 end";
}

/**
 * @tc.name: ToolUtil_ValidateInputSchemaProperties_0300
 * @tc.desc: Test ValidateInputSchemaProperties with schema missing properties
 * @tc.type: FUNC
 */
HWTEST_F(ToolUtilTest, ValidateInputSchemaProperties_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolUtil_ValidateInputSchemaProperties_0300 start";

    std::string schema = R"({"type": "object"})";
    std::string subcommand = "";
    std::map<std::string, std::string> args;

    int32_t result = ValidateToolProperties(schema, subcommand, args);

    EXPECT_EQ(result, ERR_OK);

    GTEST_LOG_(INFO) << "ToolUtil_ValidateInputSchemaProperties_0300 end";
}

/**
 * @tc.name: ToolUtil_ValidateInputSchemaProperties_0400
 * @tc.desc: Test ValidateInputSchemaProperties with valid schema and no args
 * @tc.type: FUNC
 */
HWTEST_F(ToolUtilTest, ValidateInputSchemaProperties_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolUtil_ValidateInputSchemaProperties_0400 start";

    std::string schema = R"({"properties": {"help": {"type": "boolean"}}})";
    std::string subcommand = "";
    std::map<std::string, std::string> args;

    int32_t result = ValidateToolProperties(schema, subcommand, args);

    EXPECT_EQ(result, ERR_OK);

    GTEST_LOG_(INFO) << "ToolUtil_ValidateInputSchemaProperties_0400 end";
}

/**
 * @tc.name: ToolUtil_ValidateInputSchemaProperties_0500
 * @tc.desc: Test ValidateInputSchemaProperties with valid args
 * @tc.type: FUNC
 */
HWTEST_F(ToolUtilTest, ValidateInputSchemaProperties_0500, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolUtil_ValidateInputSchemaProperties_0500 start";

    std::string schema = R"({
        "properties": {
            "force": {"type": "boolean"},
            "verbose": {"type": "boolean"}
        }
    })";
    std::string subcommand = "";
    std::map<std::string, std::string> args;
    args["force"] = "true";
    args["verbose"] = "false";

    int32_t result = ValidateToolProperties(schema, subcommand, args);

    EXPECT_EQ(result, ERR_OK);

    GTEST_LOG_(INFO) << "ToolUtil_ValidateInputSchemaProperties_0500 end";
}

/**
 * @tc.name: ToolUtil_ValidateInputSchemaProperties_0600
 * @tc.desc: Test ValidateInputSchemaProperties with invalid arg not in schema
 * @tc.type: FUNC
 */
HWTEST_F(ToolUtilTest, ValidateInputSchemaProperties_0600, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolUtil_ValidateInputSchemaProperties_0600 start";

    std::string schema = R"({"properties": {"help": {"type": "boolean"}}})";
    std::string subcommand = "";
    std::map<std::string, std::string> args;
    args["invalid_arg"] = "value";

    int32_t result = ValidateToolProperties(schema, subcommand, args);

    EXPECT_EQ(result, ERR_INVALID_PARAM);

    GTEST_LOG_(INFO) << "ToolUtil_ValidateInputSchemaProperties_0600 end";
}

/**
 * @tc.name: ToolUtil_ValidateInputSchemaProperties_0700
 * @tc.desc: Test ValidateInputSchemaProperties with valid subcommand
 * @tc.type: FUNC
 */
HWTEST_F(ToolUtilTest, ValidateInputSchemaProperties_0700, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolUtil_ValidateInputSchemaProperties_0700 start";

    std::string schema = R"({
        "properties": {
            "build": {
                "type": "object",
                "description": "Build subcommand"
            },
            "clean": {
                "type": "object",
                "description": "Clean subcommand"
            }
        }
    })";
    std::string subcommand = "build";
    std::map<std::string, std::string> args;

    int32_t result = ValidateToolProperties(schema, subcommand, args);

    EXPECT_EQ(result, ERR_OK);

    GTEST_LOG_(INFO) << "ToolUtil_ValidateInputSchemaProperties_0700 end";
}

/**
 * @tc.name: ToolUtil_ValidateInputSchemaProperties_0800
 * @tc.desc: Test ValidateInputSchemaProperties with invalid subcommand
 * @tc.type: FUNC
 */
HWTEST_F(ToolUtilTest, ValidateInputSchemaProperties_0800, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolUtil_ValidateInputSchemaProperties_0800 start";

    std::string schema = R"({
        "properties": {
            "build": {
                "type": "object",
                "description": "Build subcommand"
            }
        }
    })";
    std::string subcommand = "invalid_subcmd";
    std::map<std::string, std::string> args;

    int32_t result = ValidateToolProperties(schema, subcommand, args);

    EXPECT_EQ(result, ERR_TOOL_NOT_EXIST);

    GTEST_LOG_(INFO) << "ToolUtil_ValidateInputSchemaProperties_0800 end";
}

/**
 * @tc.name: ToolUtil_ValidateInputSchemaProperties_0900
 * @tc.desc: Test ValidateInputSchemaProperties with subcommand and args
 * @tc.type: FUNC
 */
HWTEST_F(ToolUtilTest, ValidateInputSchemaProperties_0900, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolUtil_ValidateInputSchemaProperties_0900 start";

    std::string schema = R"({
        "properties": {
            "build": {"type": "object"},
            "help": {"type": "boolean"}
        }
    })";
    std::string subcommand = "build";
    std::map<std::string, std::string> args;
    args["help"] = "true";

    int32_t result = ValidateToolProperties(schema, subcommand, args);

    EXPECT_EQ(result, ERR_OK);

    GTEST_LOG_(INFO) << "ToolUtil_ValidateInputSchemaProperties_0900 end";
}

/**
 * @tc.name: ToolUtil_ValidateInputSchemaProperties_1000
 * @tc.desc: Test ValidateInputSchemaProperties with complex schema
 * @tc.type: FUNC
 */
HWTEST_F(ToolUtilTest, ValidateInputSchemaProperties_1000, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolUtil_ValidateInputSchemaProperties_1000 start";

    std::string schema = R"({
        "properties": {
            "deploy": {
                "type": "object",
                "description": "Deploy subcommand"
            },
            "verbose": {"type": "boolean"},
            "output": {"type": "string"},
            "force": {"type": "boolean"}
        }
    })";
    std::string subcommand = "deploy";
    std::map<std::string, std::string> args;
    args["verbose"] = "true";
    args["output"] = "/tmp/deploy.log";

    int32_t result = ValidateToolProperties(schema, subcommand, args);

    EXPECT_EQ(result, ERR_OK);

    GTEST_LOG_(INFO) << "ToolUtil_ValidateInputSchemaProperties_1000 end";
}

/**
 * @tc.name: ToolUtil_GenerateCliSessionId_0100
 * @tc.desc: Test GenerateCliSessionId with basic name
 * @tc.type: FUNC
 */
HWTEST_F(ToolUtilTest, GenerateCliSessionId_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolUtil_GenerateCliSessionId_0100 start";

    std::string name = "test_tool";
    std::string sessionId = ToolUtil::GenerateCliSessionId(name, nullptr);

    EXPECT_FALSE(sessionId.empty());
    EXPECT_GE(sessionId.length(), name.length());

    // Check format: name_timestamp_random
    size_t firstUnderscore = sessionId.find('_');
    size_t lastUnderscore = sessionId.rfind('_');
    EXPECT_NE(firstUnderscore, std::string::npos);
    EXPECT_NE(lastUnderscore, std::string::npos);
    EXPECT_GT(lastUnderscore, firstUnderscore);

    GTEST_LOG_(INFO) << "ToolUtil_GenerateCliSessionId_0100 end";
}

/**
 * @tc.name: ToolUtil_GenerateCliSessionId_0200
 * @tc.desc: Test GenerateCliSessionId generates unique IDs
 * @tc.type: FUNC
 */
HWTEST_F(ToolUtilTest, GenerateCliSessionId_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolUtil_GenerateCliSessionId_0200 start";

    std::string name = "unique_tool";
    std::string sessionId1 = ToolUtil::GenerateCliSessionId(name, nullptr);
    std::string sessionId2 = ToolUtil::GenerateCliSessionId(name, nullptr);

    EXPECT_NE(sessionId1, sessionId2);

    GTEST_LOG_(INFO) << "ToolUtil_GenerateCliSessionId_0200 end";
}

/**
 * @tc.name: ToolUtil_GenerateCliSessionId_0300
 * @tc.desc: Test GenerateCliSessionId with empty name
 * @tc.type: FUNC
 */
HWTEST_F(ToolUtilTest, GenerateCliSessionId_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolUtil_GenerateCliSessionId_0300 start";

    std::string emptyName = "";
    std::string sessionId = ToolUtil::GenerateCliSessionId(emptyName, nullptr);

    // Should still generate a valid ID with format _timestamp_random
    EXPECT_FALSE(sessionId.empty());
    EXPECT_GE(sessionId.length(), 2); // At least "_X"

    GTEST_LOG_(INFO) << "ToolUtil_GenerateCliSessionId_0300 end";
}

/**
 * @tc.name: ToolUtil_GenerateCliSessionId_0400
 * @tc.desc: Test GenerateCliSessionId with special characters in name
 * @tc.type: FUNC
 */
HWTEST_F(ToolUtilTest, GenerateCliSessionId_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolUtil_GenerateCliSessionId_0400 start";

    std::string name = "test-tool.special@name";
    std::string sessionId = ToolUtil::GenerateCliSessionId(name, nullptr);

    EXPECT_FALSE(sessionId.empty());
    EXPECT_TRUE(sessionId.find(name) == 0); // Should start with the name

    GTEST_LOG_(INFO) << "ToolUtil_GenerateCliSessionId_0400 end";
}

/**
 * @tc.name: ToolUtil_GenerateCliSessionId_0500
 * @tc.desc: Test GenerateCliSessionId format consistency
 * @tc.type: FUNC
 */
HWTEST_F(ToolUtilTest, GenerateCliSessionId_0500, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolUtil_GenerateCliSessionId_0500 start";

    std::string name = "formatTest";
    std::string sessionId = ToolUtil::GenerateCliSessionId(name, nullptr);

    // Verify format: name_timestamp_random
    size_t firstUnderscore = sessionId.find('_');
    size_t lastUnderscore = sessionId.rfind('_');

    ASSERT_NE(firstUnderscore, std::string::npos);
    ASSERT_NE(lastUnderscore, std::string::npos);
    ASSERT_GT(lastUnderscore, firstUnderscore);

    std::string prefix = sessionId.substr(0, firstUnderscore);
    std::string middle = sessionId.substr(firstUnderscore + 1, lastUnderscore - firstUnderscore - 1);
    std::string suffix = sessionId.substr(lastUnderscore + 1);

    EXPECT_EQ(prefix, name);

    // Verify middle (timestamp) is numeric
    for (char c : middle) {
        EXPECT_TRUE(std::isdigit(c));
    }

    // Verify suffix (random) is numeric
    for (char c : suffix) {
        EXPECT_TRUE(std::isdigit(c));
    }

    GTEST_LOG_(INFO) << "ToolUtil_GenerateCliSessionId_0500 end";
}

/**
 * @tc.name: ToolUtil_GenerateCliSessionId_0600
 * @tc.desc: Test GenerateCliSessionId with very long name
 * @tc.type: FUNC
 */
HWTEST_F(ToolUtilTest, GenerateCliSessionId_0600, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolUtil_GenerateCliSessionId_0600 start";

    std::string longName(1000, 'a'); // 1000 character name
    std::string sessionId = ToolUtil::GenerateCliSessionId(longName, nullptr);

    EXPECT_FALSE(sessionId.empty());
    EXPECT_TRUE(sessionId.find(longName) == 0);

    GTEST_LOG_(INFO) << "ToolUtil_GenerateCliSessionId_0600 end";
}

/**
 * @tc.name: ToolUtil_ValidateInputSchemaProperties_EdgeCase_0100
 * @tc.desc: Test ValidateInputSchemaProperties with nested properties
 * @tc.type: FUNC
 */
HWTEST_F(ToolUtilTest, ValidateInputSchemaProperties_EdgeCase_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolUtil_ValidateInputSchemaProperties_EdgeCase_0100 start";

    std::string schema = R"({
        "properties": {
            "option1": {
                "type": "object",
                "properties": {
                    "nested": {"type": "string"}
                }
            }
        }
    })";
    std::string subcommand = "";
    std::map<std::string, std::string> args;
    args["option1"] = "value";

    int32_t result = ValidateToolProperties(schema, subcommand, args);

    EXPECT_EQ(result, ERR_OK);

    GTEST_LOG_(INFO) << "ToolUtil_ValidateInputSchemaProperties_EdgeCase_0100 end";
}

/**
 * @tc.name: ToolUtil_ValidateInputSchemaProperties_EdgeCase_0200
 * @tc.desc: Test ValidateInputSchemaProperties with properties as array (invalid)
 * @tc.type: FUNC
 */
HWTEST_F(ToolUtilTest, ValidateInputSchemaProperties_EdgeCase_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolUtil_ValidateInputSchemaProperties_EdgeCase_0200 start";

    std::string schema = R"({"properties": ["item1", "item2"]})";
    std::string subcommand = "";
    std::map<std::string, std::string> args;

    int32_t result = ValidateToolProperties(schema, subcommand, args);

    EXPECT_EQ(result, ERR_OK);

    GTEST_LOG_(INFO) << "ToolUtil_ValidateInputSchemaProperties_EdgeCase_0200 end";
}

/**
 * @tc.name: ToolUtil_GenerateCliSessionId_EdgeCase_0100
 * @tc.desc: Test GenerateCliSessionId uniqueness in rapid succession
 * @tc.type: FUNC
 */
HWTEST_F(ToolUtilTest, GenerateCliSessionId_EdgeCase_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolUtil_GenerateCliSessionId_EdgeCase_0100 start";

    std::string name = "rapid_test";
    std::vector<std::string> sessionIds;

    // Generate multiple IDs rapidly
    for (int i = 0; i < 10; i++) {
        std::string sessionId = ToolUtil::GenerateCliSessionId(name, nullptr);
        sessionIds.push_back(sessionId);
    }

    // All should be unique (at least very likely due to random component)
    std::unordered_set<std::string> uniqueIds(sessionIds.begin(), sessionIds.end());
    EXPECT_GE(uniqueIds.size(), 9); // Allow for potential collision but very unlikely

    GTEST_LOG_(INFO) << "ToolUtil_GenerateCliSessionId_EdgeCase_0100 end";
}

/**
 * @tc.name: ToolUtil_GenerateCliSessionId_EdgeCase_0200
 * @tc.desc: Test GenerateCliSessionId with underscore in name
 * @tc.type: FUNC
 */
HWTEST_F(ToolUtilTest, GenerateCliSessionId_EdgeCase_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolUtil_GenerateCliSessionId_EdgeCase_0200 start";

    std::string name = "test_tool_name";
    std::string sessionId = ToolUtil::GenerateCliSessionId(name, nullptr);

    EXPECT_FALSE(sessionId.empty());
    EXPECT_TRUE(sessionId.find(name) == 0);

    GTEST_LOG_(INFO) << "ToolUtil_GenerateCliSessionId_EdgeCase_0200 end";
}

/**
 * @tc.name: ToolUtil_ValidateInputSchemaProperties_TypeValidation_0100
 * @tc.desc: Test ValidateInputSchemaProperties with correct string type
 * @tc.type: FUNC
 */
HWTEST_F(ToolUtilTest, ValidateInputSchemaProperties_TypeValidation_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolUtil_ValidateInputSchemaProperties_TypeValidation_0100 start";

    std::string schema = R"({
        "properties": {
            "target": {"type": "string"},
            "output": {"type": "string"}
        }
    })";
    AAFwk::WantParams args;
    args.SetParam("target", AAFwk::String::Box("192.168.1.1"));
    args.SetParam("output", AAFwk::String::Box("/tmp/output.txt"));

    int32_t result = ToolUtil::ValidateInputSchemaProperties(schema, args);

    EXPECT_EQ(result, ERR_OK);

    GTEST_LOG_(INFO) << "ToolUtil_ValidateInputSchemaProperties_TypeValidation_0100 end";
}

/**
 * @tc.name: ToolUtil_ValidateInputSchemaProperties_TypeValidation_0200
 * @tc.desc: Test ValidateInputSchemaProperties with type mismatch (string vs boolean)
 * @tc.type: FUNC
 */
HWTEST_F(ToolUtilTest, ValidateInputSchemaProperties_TypeValidation_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolUtil_ValidateInputSchemaProperties_TypeValidation_0200 start";

    std::string schema = R"({
        "properties": {
            "verbose": {"type": "boolean"}
        }
    })";
    AAFwk::WantParams args;
    args.SetParam("verbose", AAFwk::String::Box("true"));  // Wrong: string instead of boolean

    int32_t result = ToolUtil::ValidateInputSchemaProperties(schema, args);

    EXPECT_EQ(result, ERR_INVALID_PARAM);

    GTEST_LOG_(INFO) << "ToolUtil_ValidateInputSchemaProperties_TypeValidation_0200 end";
}

/**
 * @tc.name: ToolUtil_ValidateInputSchemaProperties_TypeValidation_0300
 * @tc.desc: Test ValidateInputSchemaProperties with correct boolean type
 * @tc.type: FUNC
 */
HWTEST_F(ToolUtilTest, ValidateInputSchemaProperties_TypeValidation_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolUtil_ValidateInputSchemaProperties_TypeValidation_0300 start";

    std::string schema = R"({
        "properties": {
            "verbose": {"type": "boolean"},
            "force": {"type": "boolean"}
        }
    })";
    AAFwk::WantParams args;
    args.SetParam("verbose", AAFwk::Boolean::Box(true));
    args.SetParam("force", AAFwk::Boolean::Box(false));

    int32_t result = ToolUtil::ValidateInputSchemaProperties(schema, args);

    EXPECT_EQ(result, ERR_OK);

    GTEST_LOG_(INFO) << "ToolUtil_ValidateInputSchemaProperties_TypeValidation_0300 end";
}

/**
 * @tc.name: ToolUtil_ValidateInputSchemaProperties_TypeValidation_0400
 * @tc.desc: Test ValidateInputSchemaProperties with correct integer type
 * @tc.type: FUNC
 */
HWTEST_F(ToolUtilTest, ValidateInputSchemaProperties_TypeValidation_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolUtil_ValidateInputSchemaProperties_TypeValidation_0400 start";

    std::string schema = R"({
        "properties": {
            "timeout": {"type": "integer"},
            "port": {"type": "integer"}
        }
    })";
    AAFwk::WantParams args;
    args.SetParam("timeout", AAFwk::Integer::Box(5000));
    args.SetParam("port", AAFwk::Integer::Box(8080));

    int32_t result = ToolUtil::ValidateInputSchemaProperties(schema, args);

    EXPECT_EQ(result, ERR_OK);

    GTEST_LOG_(INFO) << "ToolUtil_ValidateInputSchemaProperties_TypeValidation_0400 end";
}

/**
 * @tc.name: ToolUtil_ValidateInputSchemaProperties_TypeValidation_0500
 * @tc.desc: Test ValidateInputSchemaProperties with type mismatch (integer vs string)
 * @tc.type: FUNC
 */
HWTEST_F(ToolUtilTest, ValidateInputSchemaProperties_TypeValidation_0500, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolUtil_ValidateInputSchemaProperties_TypeValidation_0500 start";

    std::string schema = R"({
        "properties": {
            "timeout": {"type": "integer"}
        }
    })";
    AAFwk::WantParams args;
    args.SetParam("timeout", AAFwk::String::Box("5000"));  // Wrong: string instead of integer

    int32_t result = ToolUtil::ValidateInputSchemaProperties(schema, args);

    EXPECT_EQ(result, ERR_INVALID_PARAM);

    GTEST_LOG_(INFO) << "ToolUtil_ValidateInputSchemaProperties_TypeValidation_0500 end";
}

/**
 * @tc.name: ToolUtil_ValidateInputSchemaProperties_TypeValidation_0600
 * @tc.desc: Test ValidateInputSchemaProperties with correct number type (double)
 * @tc.type: FUNC
 */
HWTEST_F(ToolUtilTest, ValidateInputSchemaProperties_TypeValidation_0600, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolUtil_ValidateInputSchemaProperties_TypeValidation_0600 start";

    std::string schema = R"({
        "properties": {
            "rate": {"type": "number"}
        }
    })";
    AAFwk::WantParams args;
    args.SetParam("rate", AAFwk::Double::Box(3.14));

    int32_t result = ToolUtil::ValidateInputSchemaProperties(schema, args);

    EXPECT_EQ(result, ERR_OK);

    GTEST_LOG_(INFO) << "ToolUtil_ValidateInputSchemaProperties_TypeValidation_0600 end";
}

/**
 * @tc.name: ToolUtil_ValidateInputSchemaProperties_TypeValidation_0700
 * @tc.desc: Test ValidateInputSchemaProperties with correct array type
 * @tc.type: FUNC
 */
HWTEST_F(ToolUtilTest, ValidateInputSchemaProperties_TypeValidation_0700, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolUtil_ValidateInputSchemaProperties_TypeValidation_0700 start";

    std::string schema = R"({
        "properties": {
            "ports": {"type": "array"}
        }
    })";
    AAFwk::WantParams args;
    sptr<AAFwk::IArray> array = new (std::nothrow) AAFwk::Array(3, AAFwk::g_IID_IInteger);
    if (array != nullptr) {
        array->Set(0, AAFwk::Integer::Box(80).GetRefPtr());
        array->Set(1, AAFwk::Integer::Box(443).GetRefPtr());
        array->Set(2, AAFwk::Integer::Box(8080).GetRefPtr());
        args.SetParam("ports", array);
    }

    int32_t result = ToolUtil::ValidateInputSchemaProperties(schema, args);

    EXPECT_EQ(result, ERR_OK);

    GTEST_LOG_(INFO) << "ToolUtil_ValidateInputSchemaProperties_TypeValidation_0700 end";
}

/**
 * @tc.name: ToolUtil_ValidateInputSchemaProperties_TypeValidation_0800
 * @tc.desc: Test ValidateInputSchemaProperties with type mismatch (array vs string)
 * @tc.type: FUNC
 */
HWTEST_F(ToolUtilTest, ValidateInputSchemaProperties_TypeValidation_0800, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolUtil_ValidateInputSchemaProperties_TypeValidation_0800 start";

    std::string schema = R"({
        "properties": {
            "ports": {"type": "array"}
        }
    })";
    AAFwk::WantParams args;
    args.SetParam("ports", AAFwk::String::Box("80,443,8080"));  // Wrong: string instead of array

    int32_t result = ToolUtil::ValidateInputSchemaProperties(schema, args);

    EXPECT_EQ(result, ERR_INVALID_PARAM);

    GTEST_LOG_(INFO) << "ToolUtil_ValidateInputSchemaProperties_TypeValidation_0800 end";
}

/**
 * @tc.name: ToolUtil_ValidateInputSchemaProperties_TypeValidation_0900
 * @tc.desc: Test ValidateInputSchemaProperties with correct object type
 * @tc.type: FUNC
 */
HWTEST_F(ToolUtilTest, ValidateInputSchemaProperties_TypeValidation_0900, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolUtil_ValidateInputSchemaProperties_TypeValidation_0900 start";

    std::string schema = R"({
        "properties": {
            "config": {"type": "object"}
        }
    })";
    AAFwk::WantParams args;
    AAFwk::WantParams nestedConfig;
    nestedConfig.SetParam("key", AAFwk::String::Box("value"));
    args.SetParam("config", AAFwk::WantParamWrapper::Box(nestedConfig));

    int32_t result = ToolUtil::ValidateInputSchemaProperties(schema, args);

    EXPECT_EQ(result, ERR_OK);

    GTEST_LOG_(INFO) << "ToolUtil_ValidateInputSchemaProperties_TypeValidation_0900 end";
}

/**
 * @tc.name: ToolUtil_ValidateInputSchemaProperties_TypeValidation_1000
 * @tc.desc: Test ValidateInputSchemaProperties with mixed types
 * @tc.type: FUNC
 */
HWTEST_F(ToolUtilTest, ValidateInputSchemaProperties_TypeValidation_1000, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolUtil_ValidateInputSchemaProperties_TypeValidation_1000 start";

    std::string schema = R"({
        "properties": {
            "target": {"type": "string"},
            "timeout": {"type": "integer"},
            "verbose": {"type": "boolean"},
            "ports": {"type": "array"}
        }
    })";
    AAFwk::WantParams args;
    args.SetParam("target", AAFwk::String::Box("192.168.1.1"));
    args.SetParam("timeout", AAFwk::Integer::Box(5000));
    args.SetParam("verbose", AAFwk::Boolean::Box(true));
    sptr<AAFwk::IArray> array = new (std::nothrow) AAFwk::Array(2, AAFwk::g_IID_IInteger);
    if (array != nullptr) {
        array->Set(0, AAFwk::Integer::Box(80).GetRefPtr());
        array->Set(1, AAFwk::Integer::Box(443).GetRefPtr());
        args.SetParam("ports", array);
    }

    int32_t result = ToolUtil::ValidateInputSchemaProperties(schema, args);

    EXPECT_EQ(result, ERR_OK);

    GTEST_LOG_(INFO) << "ToolUtil_ValidateInputSchemaProperties_TypeValidation_1000 end";
}

/**
 * @tc.name: ToolUtil_ValidateInputSchemaProperties_TypeValidation_1100
 * @tc.desc: Test ValidateInputSchemaProperties with no type specified (should pass)
 * @tc.type: FUNC
 */
HWTEST_F(ToolUtilTest, ValidateInputSchemaProperties_TypeValidation_1100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolUtil_ValidateInputSchemaProperties_TypeValidation_1100 start";

    std::string schema = R"({
        "properties": {
            "target": {"description": "Target address"}
        }
    })";
    AAFwk::WantParams args;
    args.SetParam("target", AAFwk::String::Box("192.168.1.1"));

    int32_t result = ToolUtil::ValidateInputSchemaProperties(schema, args);

    // Should pass because no type is specified
    EXPECT_EQ(result, ERR_OK);

    GTEST_LOG_(INFO) << "ToolUtil_ValidateInputSchemaProperties_TypeValidation_1100 end";
}

/**
 * @tc.name: ToolUtil_ValidateInputSchemaProperties_NestedObject_0100
 * @tc.desc: Test ValidateInputSchemaProperties with nested object validation
 * @tc.type: FUNC
 */
HWTEST_F(ToolUtilTest, ValidateInputSchemaProperties_NestedObject_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolUtil_ValidateInputSchemaProperties_NestedObject_0100 start";

    std::string schema = R"({
        "properties": {
            "app": {
                "type": "object",
                "properties": {
                    "name": {"type": "string"},
                    "version": {"type": "string"},
                    "env": {
                        "type": "object",
                        "properties": {
                            "production": {"type": "boolean"},
                            "debug": {"type": "boolean"}
                        }
                    }
                }
            }
        }
    })";
    AAFwk::WantParams args;
    AAFwk::WantParams appParams;
    appParams.SetParam("name", AAFwk::String::Box("myapp"));
    appParams.SetParam("version", AAFwk::String::Box("1.0.0"));
    AAFwk::WantParams envParams;
    envParams.SetParam("production", AAFwk::Boolean::Box(true));
    envParams.SetParam("debug", AAFwk::Boolean::Box(false));
    appParams.SetParam("env", AAFwk::WantParamWrapper::Box(envParams));
    args.SetParam("app", AAFwk::WantParamWrapper::Box(appParams));

    int32_t result = ToolUtil::ValidateInputSchemaProperties(schema, args);

    EXPECT_EQ(result, ERR_OK);

    GTEST_LOG_(INFO) << "ToolUtil_ValidateInputSchemaProperties_NestedObject_0100 end";
}

/**
 * @tc.name: ToolUtil_ValidateInputSchemaProperties_NestedObject_0200
 * @tc.desc: Test ValidateInputSchemaProperties with nested object type mismatch
 * @tc.type: FUNC
 */
HWTEST_F(ToolUtilTest, ValidateInputSchemaProperties_NestedObject_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolUtil_ValidateInputSchemaProperties_NestedObject_0200 start";

    std::string schema = R"({
        "properties": {
            "app": {
                "type": "object",
                "properties": {
                    "name": {"type": "string"},
                    "version": {"type": "string"}
                }
            }
        }
    })";
    AAFwk::WantParams args;
    AAFwk::WantParams appParams;
    appParams.SetParam("name", AAFwk::String::Box("myapp"));
    appParams.SetParam("version", AAFwk::Integer::Box(100));  // Wrong: integer instead of string
    args.SetParam("app", AAFwk::WantParamWrapper::Box(appParams));

    int32_t result = ToolUtil::ValidateInputSchemaProperties(schema, args);

    EXPECT_EQ(result, ERR_OK);

    GTEST_LOG_(INFO) << "ToolUtil_ValidateInputSchemaProperties_NestedObject_0200 end";
}

/**
 * @tc.name: ToolUtil_ValidateInputSchemaProperties_NestedObject_0300
 * @tc.desc: Test ValidateInputSchemaProperties with nested object missing required property
 * @tc.type: FUNC
 */
HWTEST_F(ToolUtilTest, ValidateInputSchemaProperties_NestedObject_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolUtil_ValidateInputSchemaProperties_NestedObject_0300 start";

    std::string schema = R"({
        "properties": {
            "server": {
                "type": "object",
                "properties": {
                    "host": {"type": "string"},
                    "port": {"type": "integer"}
                },
                "required": ["host", "port"]
            }
        }
    })";
    AAFwk::WantParams args;
    AAFwk::WantParams serverParams;
    serverParams.SetParam("host", AAFwk::String::Box("localhost"));
    // Missing required "port" property
    args.SetParam("server", AAFwk::WantParamWrapper::Box(serverParams));

    int32_t result = ToolUtil::ValidateInputSchemaProperties(schema, args);

    EXPECT_EQ(result, ERR_OK);

    GTEST_LOG_(INFO) << "ToolUtil_ValidateInputSchemaProperties_NestedObject_0300 end";
}

/**
 * @tc.name: ToolUtil_ValidateInputSchemaProperties_NestedObject_0400
 * @tc.desc: Test ValidateInputSchemaProperties with deep nested objects
 * @tc.type: FUNC
 */
HWTEST_F(ToolUtilTest, ValidateInputSchemaProperties_NestedObject_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolUtil_ValidateInputSchemaProperties_NestedObject_0400 start";

    std::string schema = R"({
        "properties": {
            "config": {
                "type": "object",
                "properties": {
                    "app": {
                        "type": "object",
                        "properties": {
                            "name": {"type": "string"},
                            "settings": {
                                "type": "object",
                                "properties": {
                                    "timeout": {"type": "integer"},
                                    "retries": {"type": "integer"}
                                }
                            }
                        }
                    }
                }
            }
        }
    })";
    AAFwk::WantParams args;
    AAFwk::WantParams configParams;
    AAFwk::WantParams appParams;
    appParams.SetParam("name", AAFwk::String::Box("myapp"));
    AAFwk::WantParams settingsParams;
    settingsParams.SetParam("timeout", AAFwk::Integer::Box(5000));
    settingsParams.SetParam("retries", AAFwk::Integer::Box(3));
    appParams.SetParam("settings", AAFwk::WantParamWrapper::Box(settingsParams));
    configParams.SetParam("app", AAFwk::WantParamWrapper::Box(appParams));
    args.SetParam("config", AAFwk::WantParamWrapper::Box(configParams));

    int32_t result = ToolUtil::ValidateInputSchemaProperties(schema, args);

    EXPECT_EQ(result, ERR_OK);

    GTEST_LOG_(INFO) << "ToolUtil_ValidateInputSchemaProperties_NestedObject_0400 end";
}

/**
 * @tc.name: ToolUtil_ValidateInputSchemaProperties_ArrayItems_0100
 * @tc.desc: Test ValidateInputSchemaProperties with array item type validation
 * @tc.type: FUNC
 */
HWTEST_F(ToolUtilTest, ValidateInputSchemaProperties_ArrayItems_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolUtil_ValidateInputSchemaProperties_ArrayItems_0100 start";

    std::string schema = R"({
        "properties": {
            "ports": {
                "type": "array",
                "items": {
                    "type": "integer"
                }
            }
        }
    })";
    AAFwk::WantParams args;
    sptr<AAFwk::IArray> array = new (std::nothrow) AAFwk::Array(3, AAFwk::g_IID_IInteger);
    if (array != nullptr) {
        array->Set(0, AAFwk::Integer::Box(80).GetRefPtr());
        array->Set(1, AAFwk::Integer::Box(443).GetRefPtr());
        array->Set(2, AAFwk::Integer::Box(8080).GetRefPtr());
        args.SetParam("ports", array);
    }

    int32_t result = ToolUtil::ValidateInputSchemaProperties(schema, args);

    EXPECT_EQ(result, ERR_OK);

    GTEST_LOG_(INFO) << "ToolUtil_ValidateInputSchemaProperties_ArrayItems_0100 end";
}

/**
 * @tc.name: ToolUtil_ValidateInputSchemaProperties_ArrayItems_0200
 * @tc.desc: Test ValidateInputSchemaProperties with array item type mismatch
 * @tc.type: FUNC
 */
HWTEST_F(ToolUtilTest, ValidateInputSchemaProperties_ArrayItems_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolUtil_ValidateInputSchemaProperties_ArrayItems_0200 start";

    std::string schema = R"({
        "properties": {
            "ports": {
                "type": "array",
                "items": {
                    "type": "integer"
                }
            }
        }
    })";
    AAFwk::WantParams args;
    sptr<AAFwk::IArray> array = new (std::nothrow) AAFwk::Array(1, AAFwk::g_IID_IString);
    if (array != nullptr) {
        array->Set(0, AAFwk::String::Box("443").GetRefPtr());  // Wrong: string in integer array
        args.SetParam("ports", array);
    }

    int32_t result = ToolUtil::ValidateInputSchemaProperties(schema, args);

    EXPECT_EQ(result, ERR_INVALID_PARAM);

    GTEST_LOG_(INFO) << "ToolUtil_ValidateInputSchemaProperties_ArrayItems_0200 end";
}

/**
 * @tc.name: ToolUtil_ValidateInputSchemaProperties_1100
 * @tc.desc: Test ValidateInputSchemaProperties with non-empty args and empty schema
 * @tc.type: FUNC
 */
HWTEST_F(ToolUtilTest, ValidateInputSchemaProperties_1100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolUtil_ValidateInputSchemaProperties_1100 start";

    AAFwk::WantParams args;
    args.SetParam("target", AAFwk::String::Box("device"));

    EXPECT_EQ(ToolUtil::ValidateInputSchemaProperties("", args), ERR_INVALID_PARAM);

    GTEST_LOG_(INFO) << "ToolUtil_ValidateInputSchemaProperties_1100 end";
}

/**
 * @tc.name: ToolUtil_ValidateInputSchemaProperties_1200
 * @tc.desc: Test ValidateInputSchemaProperties with invalid schema and non-empty args
 * @tc.type: FUNC
 */
HWTEST_F(ToolUtilTest, ValidateInputSchemaProperties_1200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolUtil_ValidateInputSchemaProperties_1200 start";

    AAFwk::WantParams args;
    args.SetParam("target", AAFwk::String::Box("device"));

    EXPECT_EQ(ToolUtil::ValidateInputSchemaProperties("{invalid json}", args), ERR_NO_INIT);

    GTEST_LOG_(INFO) << "ToolUtil_ValidateInputSchemaProperties_1200 end";
}

/**
 * @tc.name: ToolUtil_ValidateInputSchemaProperties_1300
 * @tc.desc: Test ValidateInputSchemaProperties help branch rejects additional args
 * @tc.type: FUNC
 */
HWTEST_F(ToolUtilTest, ValidateInputSchemaProperties_1300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolUtil_ValidateInputSchemaProperties_1300 start";

    AAFwk::WantParams args;
    args.SetParam("help", AAFwk::Boolean::Box(true));
    args.SetParam("target", AAFwk::String::Box("device"));

    std::string schema = R"({"properties":{"help":{"type":"boolean"},"target":{"type":"string"}}})";
    EXPECT_EQ(ToolUtil::ValidateInputSchemaProperties(schema, args), ERR_INVALID_PARAM);

    GTEST_LOG_(INFO) << "ToolUtil_ValidateInputSchemaProperties_1300 end";
}

/**
 * @tc.name: ToolUtil_ValidateInputSchemaProperties_1400
 * @tc.desc: Test unknown schema type is allowed for compatibility
 * @tc.type: FUNC
 */
HWTEST_F(ToolUtilTest, ValidateInputSchemaProperties_1400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolUtil_ValidateInputSchemaProperties_1400 start";

    AAFwk::WantParams args;
    args.SetParam("target", AAFwk::String::Box("device"));

    std::string schema = R"({"properties":{"target":{"type":"custom"}}})";
    EXPECT_EQ(ToolUtil::ValidateInputSchemaProperties(schema, args), ERR_OK);

    GTEST_LOG_(INFO) << "ToolUtil_ValidateInputSchemaProperties_1400 end";
}

/**
 * @tc.name: ToolUtil_GenerateCliSessionId_0700
 * @tc.desc: Test GenerateCliSessionId records start time when record is supplied
 * @tc.type: FUNC
 */
HWTEST_F(ToolUtilTest, GenerateCliSessionId_0700, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolUtil_GenerateCliSessionId_0700 start";

    auto record = std::make_shared<SessionRecord>();
    EXPECT_EQ(record->startTime, 0);

    std::string sessionId = ToolUtil::GenerateCliSessionId("record_tool", record);

    EXPECT_FALSE(sessionId.empty());
    EXPECT_GT(record->startTime, 0);

    GTEST_LOG_(INFO) << "ToolUtil_GenerateCliSessionId_0700 end";
}

/**
 * @tc.name: ToolUtil_NormalizeSkillParamKeys_0100
 * @tc.desc: Test NormalizeSkillParamKeys renames dashed args and preserves existing bare keys
 * @tc.type: FUNC
 */
HWTEST_F(ToolUtilTest, NormalizeSkillParamKeys_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolUtil_NormalizeSkillParamKeys_0100 start";

    AAFwk::WantParams args;
    args.SetParam("--target", AAFwk::String::Box("device"));
    args.SetParam("-force", AAFwk::Boolean::Box(true));
    args.SetParam("--exists", AAFwk::String::Box("prefixed"));
    args.SetParam("exists", AAFwk::String::Box("bare"));

    ToolUtil::NormalizeSkillParamKeys(args);

    EXPECT_EQ(args.GetStringParam("target"), "device");
    auto forceValue = AAFwk::IBoolean::Query(args.GetParam("force"));
    ASSERT_NE(forceValue, nullptr);
    bool force = false;
    EXPECT_EQ(forceValue->GetValue(force), ERR_OK);
    EXPECT_TRUE(force);
    EXPECT_EQ(args.GetStringParam("exists"), "bare");
    EXPECT_TRUE(args.GetStringParam("--target").empty());
    EXPECT_EQ(args.GetStringParam("--exists"), "prefixed");

    GTEST_LOG_(INFO) << "ToolUtil_NormalizeSkillParamKeys_0100 end";
}

/**
 * @tc.name: ToolUtil_ExpandArgsFromJson_0100
 * @tc.desc: Test ExpandArgsFromJson expands supported values and skips reserved/object values
 * @tc.type: FUNC
 */
HWTEST_F(ToolUtilTest, ExpandArgsFromJson_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolUtil_ExpandArgsFromJson_0100 start";

    AAFwk::WantParams args;
    args.SetParam("args", AAFwk::String::Box("placeholder"));
    std::string argsStr = R"({
        "target": "device",
        "count": 2,
        "enabled": true,
        "bundleName": "reserved.bundle",
        "nested": {"ignored": true}
    })";

    EXPECT_TRUE(ToolUtil::ExpandArgsFromJson(args, argsStr));
    EXPECT_TRUE(args.GetStringParam("args").empty());
    EXPECT_EQ(args.GetStringParam("target"), "device");
    auto countValue = AAFwk::IInteger::Query(args.GetParam("count"));
    ASSERT_NE(countValue, nullptr);
    int32_t count = 0;
    EXPECT_EQ(countValue->GetValue(count), ERR_OK);
    EXPECT_EQ(count, 2);
    auto enabledValue = AAFwk::IBoolean::Query(args.GetParam("enabled"));
    ASSERT_NE(enabledValue, nullptr);
    bool enabled = false;
    EXPECT_EQ(enabledValue->GetValue(enabled), ERR_OK);
    EXPECT_TRUE(enabled);
    EXPECT_TRUE(args.GetStringParam("bundleName").empty());
    EXPECT_TRUE(args.GetStringParam("nested").empty());

    GTEST_LOG_(INFO) << "ToolUtil_ExpandArgsFromJson_0100 end";
}

/**
 * @tc.name: ToolUtil_ExpandArgsFromJson_0200
 * @tc.desc: Test ExpandArgsFromJson rejects invalid and non-object json
 * @tc.type: FUNC
 */
HWTEST_F(ToolUtilTest, ExpandArgsFromJson_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolUtil_ExpandArgsFromJson_0200 start";

    AAFwk::WantParams args;

    EXPECT_FALSE(ToolUtil::ExpandArgsFromJson(args, "{invalid json}"));
    EXPECT_FALSE(ToolUtil::ExpandArgsFromJson(args, R"(["not", "object"])"));

    GTEST_LOG_(INFO) << "ToolUtil_ExpandArgsFromJson_0200 end";
}

/**
 * @tc.name: ToolUtil_ExpandArgsFromWantParams_0100
 * @tc.desc: Test ExpandArgsFromWantParams expands nested args and skips reserved keys
 * @tc.type: FUNC
 */
HWTEST_F(ToolUtilTest, ExpandArgsFromWantParams_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolUtil_ExpandArgsFromWantParams_0100 start";

    AAFwk::WantParams nestedArgs;
    nestedArgs.SetParam("target", AAFwk::String::Box("device"));
    nestedArgs.SetParam("bundleName", AAFwk::String::Box("reserved.bundle"));

    AAFwk::WantParams args;
    args.SetParam("args", AAFwk::WantParamWrapper::Box(nestedArgs));

    ToolUtil::ExpandArgsFromWantParams(args);

    EXPECT_TRUE(args.GetStringParam("args").empty());
    EXPECT_EQ(args.GetStringParam("target"), "device");
    EXPECT_TRUE(args.GetStringParam("bundleName").empty());

    GTEST_LOG_(INFO) << "ToolUtil_ExpandArgsFromWantParams_0100 end";
}

/**
 * @tc.name: ToolUtil_FilterSkillArgs_0100
 * @tc.desc: Test FilterSkillArgs removes reserved skill keys
 * @tc.type: FUNC
 */
HWTEST_F(ToolUtilTest, FilterSkillArgs_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolUtil_FilterSkillArgs_0100 start";

    AAFwk::WantParams args;
    args.SetParam("bundleName", AAFwk::String::Box("reserved.bundle"));
    args.SetParam("target", AAFwk::String::Box("device"));
    args.SetParam("moduleName", AAFwk::String::Box("module"));

    auto filteredArgs = ToolUtil::FilterSkillArgs(args);

    ASSERT_NE(filteredArgs, nullptr);
    EXPECT_EQ(filteredArgs->GetStringParam("target"), "device");
    EXPECT_TRUE(filteredArgs->GetStringParam("bundleName").empty());
    EXPECT_TRUE(filteredArgs->GetStringParam("moduleName").empty());

    GTEST_LOG_(INFO) << "ToolUtil_FilterSkillArgs_0100 end";
}

/**
 * @tc.name: ToolUtil_BuildSkillSessionInfo_0100
 * @tc.desc: Test BuildSkillSessionInfo success and failure status branches
 * @tc.type: FUNC
 */
HWTEST_F(ToolUtilTest, BuildSkillSessionInfo_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolUtil_BuildSkillSessionInfo_0100 start";

    AppExecFwk::SkillExecuteResult skillResult;
    skillResult.code = 12;
    skillResult.result = std::make_shared<AAFwk::WantParams>();
    skillResult.result->SetParam("output", AAFwk::String::Box("skill result"));

    CliSessionInfo completed = ToolUtil::BuildSkillSessionInfo("session", ERR_OK, skillResult);
    EXPECT_EQ(completed.sessionId, "session");
    EXPECT_EQ(completed.toolName, "ohos-arkTSScript");
    EXPECT_EQ(completed.status, "completed");
    ASSERT_NE(completed.result, nullptr);
    EXPECT_EQ(completed.result->exitCode, 12);
    EXPECT_FALSE(completed.result->outputText.empty());

    AppExecFwk::SkillExecuteResult failedSkillResult;
    failedSkillResult.code = -1;
    CliSessionInfo failed = ToolUtil::BuildSkillSessionInfo("failed-session", ERR_INVALID_PARAM, failedSkillResult);
    EXPECT_EQ(failed.status, "failed");
    ASSERT_NE(failed.result, nullptr);
    EXPECT_EQ(failed.result->exitCode, -1);
    EXPECT_TRUE(failed.result->outputText.empty());

    GTEST_LOG_(INFO) << "ToolUtil_BuildSkillSessionInfo_0100 end";
}

} // namespace CliTool
} // namespace OHOS
