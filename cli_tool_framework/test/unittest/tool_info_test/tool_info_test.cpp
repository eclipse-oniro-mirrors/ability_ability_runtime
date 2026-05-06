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
#include <nlohmann/json.hpp>
#include <parcel.h>

#include "tool_info.h"

using namespace nlohmann::literals;
using namespace testing::ext;

namespace OHOS {
namespace CliTool {

class ToolInfoTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void ToolInfoTest::SetUpTestCase(void) {}
void ToolInfoTest::TearDownTestCase(void) {}
void ToolInfoTest::SetUp() {}
void ToolInfoTest::TearDown() {}

// ==================== ToolInfo Tests ====================

/**
 * @tc.name: ToolInfo_Marshalling_0100
 * @tc.desc: Test ToolInfo Marshalling
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_Marshalling_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_Marshalling_0100 start";

    ToolInfo tool;
    tool.name = "test_tool";
    tool.version = "1.0.0";
    tool.description = "Test tool";
    tool.executablePath = "/bin/test";
    tool.requirePermissions = {"ohos.permission.INTERNET"};
    tool.inputSchema = "{}";
    tool.outputSchema = "{}";
    tool.eventSchemas = "{}";
    tool.eventTypes = {"stdout"};
    tool.hasSubCommand = false;

    Parcel parcel;
    bool ret = tool.Marshalling(parcel);

    EXPECT_TRUE(ret);

    GTEST_LOG_(INFO) << "ToolInfo_Marshalling_0100 end";
}

/**
 * @tc.name: ToolInfo_Marshalling_0200
 * @tc.desc: Test ToolInfo Marshalling with subcommands
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_Marshalling_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_Marshalling_0200 start";

    ToolInfo tool;
    tool.name = "test_tool_no_arg";
    tool.version = "2.0.0";
    tool.description = "Tool with subcommands";
    tool.executablePath = "/bin/test2";
    tool.requirePermissions = {"ohos.permission.INTERNET"};
    tool.inputSchema = "{}";
    tool.outputSchema = "{}";
    tool.eventSchemas = "{}";
    tool.eventTypes = {};
    tool.hasSubCommand = true;
    SubCommandInfo subCmd;
    subCmd.description = "sub1";
    subCmd.inputSchema = "{}";
    subCmd.outputSchema = "{}";
    tool.subcommands["sub1"] = subCmd;

    Parcel parcel;
    bool ret = tool.Marshalling(parcel);

    EXPECT_TRUE(ret);

    GTEST_LOG_(INFO) << "ToolInfo_Marshalling_0200 end";
}

/**
 * @tc.name: ToolInfo_Unmarshalling_0100
 * @tc.desc: Test ToolInfo Unmarshalling
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_Unmarshalling_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_Unmarshalling_0100 start";

    ToolInfo original;
    original.name = "original_tool";
    original.version = "3.0.0";
    original.description = "Original tool description";
    original.executablePath = "/usr/bin/original";
    original.requirePermissions = {"ohos.permission.CAMERA", "ohos.permission.MICROPHONE"};
    original.inputSchema = R"({"type": "object", "properties": {"input": {"type": "string"}}})";
    original.outputSchema = R"({"type": "string"})";
    original.eventSchemas = "{}";
    original.eventTypes = {"stdout", "stderr", "exit"};
    original.hasSubCommand = true;
    SubCommandInfo buildSubCmd;
    buildSubCmd.description = "Build subcommand";
    buildSubCmd.inputSchema = "{}";
    buildSubCmd.outputSchema = "{}";
    original.subcommands["build"] = buildSubCmd;

    Parcel parcel;
    ASSERT_TRUE(original.Marshalling(parcel));

    parcel.RewindRead(0);
    ToolInfo *result = ToolInfo::Unmarshalling(parcel);

    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->name, "original_tool");
    EXPECT_EQ(result->version, "3.0.0");
    EXPECT_EQ(result->requirePermissions.size(), 2u);
    EXPECT_TRUE(result->hasSubCommand);
    EXPECT_EQ(result->subcommands.size(), 1u);

    delete result;

    GTEST_LOG_(INFO) << "ToolInfo_Unmarshalling_0100 end";
}

/**
 * @tc.name: ToolInfo_Unmarshalling_0200
 * @tc.desc: Test ToolInfo Unmarshalling with simple data
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_Unmarshalling_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_Unmarshalling_0200 start";

    ToolInfo original;
    original.name = "simple_tool";
    original.version = "1.0.0";
    original.description = "Simple tool";
    original.executablePath = "/bin/simple";
    original.requirePermissions = {"ohos.permission.INTERNET"};
    original.inputSchema = "{}";
    original.outputSchema = "{}";
    original.eventSchemas = "{}";
    original.eventTypes = {};
    original.hasSubCommand = false;

    Parcel parcel;
    ASSERT_TRUE(original.Marshalling(parcel));

    parcel.RewindRead(0);
    ToolInfo *result = ToolInfo::Unmarshalling(parcel);

    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->name, "simple_tool");
    EXPECT_FALSE(result->hasSubCommand);

    delete result;

    GTEST_LOG_(INFO) << "ToolInfo_Unmarshalling_0200 end";
}

/**
 * @tc.name: ToolInfo_Unmarshalling_0300
 * @tc.desc: Test ToolInfo Unmarshalling fail with empty parcel
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_Unmarshalling_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_Unmarshalling_0300 start";

    Parcel parcel;
    ToolInfo *result = ToolInfo::Unmarshalling(parcel);

    EXPECT_EQ(result, nullptr);

    GTEST_LOG_(INFO) << "ToolInfo_Unmarshalling_0300 end";
}

/**
 * @tc.name: ToolInfo_Unmarshalling_0400
 * @tc.desc: Test ToolInfo Unmarshalling with full subcommands data
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_Unmarshalling_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_Unmarshalling_0400 start";

    ToolInfo original;
    original.name = "tool_with_full_subcommands";
    original.version = "1.0.0";
    original.description = "Tool with full subcommands";
    original.executablePath = "/bin/tool";
    original.hasSubCommand = true;

    SubCommandInfo subCmd;
    subCmd.description = "Full subcommand";
    subCmd.requirePermissions = {"ohos.permission.INTERNET"};
    subCmd.inputSchema = R"({"type": "object", "properties": {"arg": {"type": "string"}}})";
    subCmd.outputSchema = R"({"type": "string"})";
    subCmd.eventTypes = {"stdout", "stderr"};
    subCmd.eventSchemas = R"({"stdout": {"type": "string"}})";

    original.subcommands["run"] = subCmd;

    Parcel parcel;
    ASSERT_TRUE(original.Marshalling(parcel));

    parcel.RewindRead(0);
    ToolInfo *result = ToolInfo::Unmarshalling(parcel);

    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->name, "tool_with_full_subcommands");
    EXPECT_TRUE(result->hasSubCommand);
    EXPECT_EQ(result->subcommands.size(), 1u);
    EXPECT_TRUE(result->subcommands.find("run") != result->subcommands.end());

    const auto &resultSubCmd = result->subcommands["run"];
    EXPECT_EQ(resultSubCmd.description, "Full subcommand");
    EXPECT_EQ(resultSubCmd.requirePermissions.size(), 1u);
    EXPECT_EQ(resultSubCmd.requirePermissions[0], "ohos.permission.INTERNET");
    EXPECT_EQ(nlohmann::json::parse(resultSubCmd.inputSchema),
        nlohmann::json::parse(R"({"type": "object", "properties": {"arg": {"type": "string"}}})"));
    EXPECT_EQ(nlohmann::json::parse(resultSubCmd.outputSchema), nlohmann::json::parse(R"({"type": "string"})"));
    EXPECT_EQ(resultSubCmd.eventTypes.size(), 2u);
    EXPECT_EQ(nlohmann::json::parse(resultSubCmd.eventSchemas),
        nlohmann::json::parse(R"({"stdout": {"type": "string"}})"));

    delete result;

    GTEST_LOG_(INFO) << "ToolInfo_Unmarshalling_0400 end";
}

// ==================== ToolsRawData Tests ====================

/**
 * @tc.name: ToolsRawData_Marshalling_0100
 * @tc.desc: Test ToolsRawData Marshalling success
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolsRawData_Marshalling_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolsRawData_Marshalling_0100 start";

    ToolsRawData rawData;
    rawData.data = {1, 2, 3, 4, 5};

    Parcel parcel;
    bool ret = rawData.Marshalling(parcel);

    EXPECT_TRUE(ret);

    GTEST_LOG_(INFO) << "ToolsRawData_Marshalling_0100 end";
}

/**
 * @tc.name: ToolsRawData_Marshalling_0200
 * @tc.desc: Test ToolsRawData Marshalling with empty data
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolsRawData_Marshalling_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolsRawData_Marshalling_0200 start";

    ToolsRawData rawData;
    rawData.data = {};

    Parcel parcel;
    bool ret = rawData.Marshalling(parcel);

    EXPECT_TRUE(ret);

    GTEST_LOG_(INFO) << "ToolsRawData_Marshalling_0200 end";
}

/**
 * @tc.name: ToolsRawData_Unmarshalling_0100
 * @tc.desc: Test ToolsRawData Unmarshalling success
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolsRawData_Unmarshalling_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolsRawData_Unmarshalling_0100 start";

    ToolsRawData original;
    original.data = {10, 20, 30, 40, 50, 60};

    Parcel parcel;
    ASSERT_TRUE(original.Marshalling(parcel));

    parcel.RewindRead(0);
    ToolsRawData *result = ToolsRawData::Unmarshalling(parcel);

    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->data.size(), 6u);
    EXPECT_EQ(result->data[0], 10u);
    EXPECT_EQ(result->data[5], 60u);

    delete result;

    GTEST_LOG_(INFO) << "ToolsRawData_Unmarshalling_0100 end";
}

/**
 * @tc.name: ToolsRawData_Unmarshalling_0200
 * @tc.desc: Test ToolsRawData Unmarshalling with empty data
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolsRawData_Unmarshalling_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolsRawData_Unmarshalling_0200 start";

    ToolsRawData original;
    original.data = {};

    Parcel parcel;
    ASSERT_TRUE(original.Marshalling(parcel));

    parcel.RewindRead(0);
    ToolsRawData *result = ToolsRawData::Unmarshalling(parcel);

    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->data.size(), 0u);

    delete result;

    GTEST_LOG_(INFO) << "ToolsRawData_Unmarshalling_0200 end";
}

// ==================== ToolInfo ParseToJson Tests ====================

/**
 * @tc.name: ToolInfo_ParseToJson_0100
 * @tc.desc: Test ToolInfo ParseToJson with full data
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_ParseToJson_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_ParseToJson_0100 start";

    ToolInfo tool;
    tool.name = "json_tool";
    tool.version = "1.0.0";
    tool.description = "JSON test tool";
    tool.executablePath = "/bin/json";
    tool.requirePermissions = {"ohos.permission.INTERNET"};
    tool.inputSchema = R"({"type": "object"})";
    tool.outputSchema = R"({"type": "string"})";
    tool.eventSchemas = R"({"stdout": {"type": "string"}})";
    tool.eventTypes = {"stdout", "stderr"};
    tool.hasSubCommand = false;

    nlohmann::json json = tool.ParseToJson();

    EXPECT_EQ(json["name"], "json_tool");
    EXPECT_EQ(json["version"], "1.0.0");
    EXPECT_EQ(json["description"], "JSON test tool");
    EXPECT_EQ(json["executablePath"], "/bin/json");

    GTEST_LOG_(INFO) << "ToolInfo_ParseToJson_0100 end";
}

/**
 * @tc.name: ToolInfo_ParseToJson_0200
 * @tc.desc: Test ToolInfo ParseToJson with minimal data
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_ParseToJson_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_ParseToJson_0200 start";

    ToolInfo tool;
    tool.name = "no_arg_tool";

    nlohmann::json json = tool.ParseToJson();

    EXPECT_EQ(json["name"], "no_arg_tool");

    GTEST_LOG_(INFO) << "ToolInfo_ParseToJson_0200 end";
}

/**
 * @tc.name: ToolInfo_ParseToJson_0300
 * @tc.desc: Test ToolInfo ParseToJson with subcommands
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_ParseToJson_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_ParseToJson_0300 start";

    ToolInfo tool;
    tool.name = "tool_with_subs";
    tool.hasSubCommand = true;

    SubCommandInfo subCmd;
    subCmd.description = "Test subcommand";
    subCmd.inputSchema = R"({"type": "object"})";
    tool.subcommands["sub1"] = subCmd;

    nlohmann::json json = tool.ParseToJson();

    EXPECT_TRUE(json.contains("subcommands"));
    EXPECT_TRUE(json["subcommands"].contains("sub1"));
    EXPECT_EQ(json["subcommands"]["sub1"]["description"], "Test subcommand");

    GTEST_LOG_(INFO) << "ToolInfo_ParseToJson_0300 end";
}

// ==================== ToolInfo ParseFromJson Tests ====================

/**
 * @tc.name: ToolInfo_ParseFromJson_0100
 * @tc.desc: Test ToolInfo ParseFromJson with full data
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_ParseFromJson_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_0100 start";

    nlohmann::json json = R"({
        "name": "ohos-parsed_tool",
        "version": "2.0.0",
        "description": "Parsed from JSON",
        "executablePath": "/bin/parsed",
        "requirePermissions": ["ohos.permission.CAMERA"],
        "inputSchema": {"type": "object"},
        "outputSchema": {"type": "array"},
        "eventSchemas": {"exit": {"type": "number"}},
        "eventTypes": ["stdout", "exit"],
        "hasSubCommand": false
    })"_json;

    ToolInfo tool;
    bool result = ToolInfo::ParseFromJson(json, tool);

    EXPECT_TRUE(result);
    EXPECT_EQ(tool.name, "ohos-parsed_tool");
    EXPECT_EQ(tool.version, "2.0.0");
    EXPECT_EQ(tool.description, "Parsed from JSON");
    EXPECT_EQ(tool.executablePath, "/bin/parsed");
    EXPECT_EQ(tool.requirePermissions.size(), 1u);
    EXPECT_FALSE(tool.hasSubCommand);

    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_0100 end";
}

/**
 * @tc.name: ToolInfo_ParseFromJson_0200
 * @tc.desc: Test ToolInfo ParseFromJson with subcommands
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_ParseFromJson_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_0200 start";

    nlohmann::json json = R"({
        "name": "hms-tool_with_sub",
        "version": "1.0.0",
        "description": "Tool with subcommands",
        "executablePath": "/bin/tool",
        "requirePermissions": ["ohos.permission.INTERNET"],
        "inputSchema": {},
        "outputSchema": {},
        "hasSubCommand": true,
        "subcommands": {
            "build": {
                "description": "Build the project",
                "requirePermissions": [],
                "inputSchema": {"type": "object"},
                "outputSchema": {"type": "string"}
            },
            "run": {
                "description": "Run the project",
                "requirePermissions": [],
                "inputSchema": {"type": "object"},
                "outputSchema": {"type": "string"}
            }
        }
    })"_json;

    ToolInfo tool;
    bool result = ToolInfo::ParseFromJson(json, tool);

    ASSERT_TRUE(result);
    EXPECT_EQ(tool.name, "hms-tool_with_sub");
    EXPECT_TRUE(tool.hasSubCommand);
    EXPECT_EQ(tool.subcommands.size(), 2u);
    EXPECT_EQ(tool.subcommands["build"].description, "Build the project");

    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_0200 end";
}

/**
 * @tc.name: ToolInfo_ParseFromJson_0300
 * @tc.desc: Test ToolInfo ParseFromJson with empty JSON
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_ParseFromJson_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_0300 start";

    nlohmann::json json;

    ToolInfo tool;
    bool result = ToolInfo::ParseFromJson(json, tool);

    EXPECT_FALSE(result);
    EXPECT_TRUE(tool.name.empty());

    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_0300 end";
}

// ==================== ToolInfo ParseFromJson/ParseToJson Round Trip Tests ====================

/**
 * @tc.name: ToolInfo_ParseFromJson_ParseToJson_RoundTrip_0100
 * @tc.desc: Test ToolInfo ParseFromJson and ParseToJson round trip
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_ParseFromJson_ParseToJson_RoundTrip_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_ParseToJson_RoundTrip_0100 start";

    nlohmann::json originalJson = R"({
        "name": "ohos-roundtrip_tool",
        "version": "3.0.0",
        "description": "Round trip test",
        "executablePath": "/bin/roundtrip",
        "requirePermissions": ["ohos.permission.INTERNET", "ohos.permission.CAMERA"],
        "inputSchema": {"type": "object", "properties": {"input": {"type": "string"}}},
        "outputSchema": {"type": "array"},
        "eventSchemas": {"stdout": {"type": "string"}},
        "eventTypes": ["stdout", "stderr"],
        "hasSubCommand": true,
        "subcommands": {
            "sub1": {
                "description": "Sub 1",
                "requirePermissions": [],
                "inputSchema": {"type": "object"},
                "outputSchema": {"type": "string"}
            }
        }
    })"_json;

    ToolInfo tool;
    bool result = ToolInfo::ParseFromJson(originalJson, tool);
    ASSERT_TRUE(result);
    nlohmann::json resultJson = tool.ParseToJson();

    EXPECT_EQ(resultJson["name"], originalJson["name"]);
    EXPECT_EQ(resultJson["version"], originalJson["version"]);
    EXPECT_EQ(resultJson["description"], originalJson["description"]);
    EXPECT_EQ(resultJson["executablePath"], originalJson["executablePath"]);
    EXPECT_EQ(resultJson["hasSubCommand"], originalJson["hasSubCommand"]);
    EXPECT_EQ(resultJson["eventTypes"], originalJson["eventTypes"]);

    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_ParseToJson_RoundTrip_0100 end";
}

/**
 * @tc.name: ToolInfo_ParseToJson_0400
 * @tc.desc: Test ToolInfo ParseToJson with invalid inputSchema JSON string
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_ParseToJson_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_ParseToJson_0400 start";

    ToolInfo tool;
    tool.name = "invalid_input_schema";
    tool.description = "Invalid inputSchema test";
    tool.inputSchema = "not a valid json";
    tool.outputSchema = R"({"type": "string"})";
    tool.eventSchemas = R"({"stdout": {"type": "string"}})";

    nlohmann::json json = tool.ParseToJson();

    EXPECT_EQ(json["name"], "invalid_input_schema");
    EXPECT_TRUE(json.contains("inputSchema"));
    EXPECT_EQ(json["inputSchema"], "not a valid json");
    EXPECT_TRUE(json.contains("outputSchema"));
    EXPECT_TRUE(json.contains("eventSchemas"));

    GTEST_LOG_(INFO) << "ToolInfo_ParseToJson_0400 end";
}

/**
 * @tc.name: ToolInfo_ParseToJson_0500
 * @tc.desc: Test ToolInfo ParseToJson with invalid outputSchema JSON string
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_ParseToJson_0500, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_ParseToJson_0500 start";

    ToolInfo tool;
    tool.name = "invalid_output_schema";
    tool.description = "Invalid outputSchema test";
    tool.inputSchema = R"({"type": "object"})";
    tool.outputSchema = "{broken json}";
    tool.eventSchemas = "{}";

    nlohmann::json json = tool.ParseToJson();

    EXPECT_EQ(json["name"], "invalid_output_schema");
    EXPECT_TRUE(json.contains("inputSchema"));
    EXPECT_TRUE(json.contains("outputSchema"));
    EXPECT_EQ(json["outputSchema"], "{broken json}");

    GTEST_LOG_(INFO) << "ToolInfo_ParseToJson_0500 end";
}

/**
 * @tc.name: ToolInfo_ParseToJson_0600
 * @tc.desc: Test ToolInfo ParseToJson with invalid eventSchemas JSON string
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_ParseToJson_0600, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_ParseToJson_0600 start";

    ToolInfo tool;
    tool.name = "invalid_event_schemas";
    tool.description = "Invalid eventSchemas test";
    tool.inputSchema = R"({"type": "object"})";
    tool.outputSchema = R"({"type": "string"})";
    tool.eventSchemas = "invalid event schemas";

    nlohmann::json json = tool.ParseToJson();

    EXPECT_EQ(json["name"], "invalid_event_schemas");
    EXPECT_TRUE(json.contains("inputSchema"));
    EXPECT_TRUE(json.contains("outputSchema"));
    EXPECT_TRUE(json.contains("eventSchemas"));
    EXPECT_EQ(json["eventSchemas"], "invalid event schemas");

    GTEST_LOG_(INFO) << "ToolInfo_ParseToJson_0600 end";
}

/**
 * @tc.name: ToolInfo_ParseToJson_0700
 * @tc.desc: Test ToolInfo ParseToJson with complex valid schemas
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_ParseToJson_0700, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_ParseToJson_0700 start";

    ToolInfo tool;
    tool.name = "complex_schemas";
    tool.description = "Complex schema test";
    tool.inputSchema = R"({
        "type": "object",
        "properties": {
            "command": {"type": "string", "enum": ["start", "stop", "restart"]},
            "options": {
                "type": "array",
                "items": {"type": "string"}
            },
            "config": {
                "type": "object",
                "properties": {
                    "debug": {"type": "boolean"},
                    "verbose": {"type": "boolean"}
                }
            }
        },
        "required": ["command"]
    })";
    tool.outputSchema = R"({
        "type": "object",
        "properties": {
            "success": {"type": "boolean"},
            "message": {"type": "string"},
            "data": {"type": "array", "items": {"type": "object"}}
        }
    })";
    tool.eventSchemas = R"({
        "stdout": {"type": "string", "description": "Standard output"},
        "stderr": {"type": "string", "description": "Standard error"},
        "exit": {"type": "number", "description": "Exit code"},
        "progress": {"type": "object", "properties": {"percent": {"type": "number"}}}
    })";

    nlohmann::json json = tool.ParseToJson();

    EXPECT_TRUE(json.contains("inputSchema"));
    EXPECT_TRUE(json["inputSchema"].is_object());
    EXPECT_TRUE(json["inputSchema"].contains("properties"));
    EXPECT_TRUE(json["inputSchema"]["properties"].contains("command"));
    EXPECT_TRUE(json["inputSchema"]["properties"]["command"].contains("enum"));

    EXPECT_TRUE(json.contains("outputSchema"));
    EXPECT_TRUE(json["outputSchema"].is_object());
    EXPECT_TRUE(json["outputSchema"]["properties"].contains("success"));

    EXPECT_TRUE(json.contains("eventSchemas"));
    EXPECT_TRUE(json["eventSchemas"].is_object());
    EXPECT_TRUE(json["eventSchemas"].contains("stdout"));
    EXPECT_TRUE(json["eventSchemas"].contains("progress"));

    GTEST_LOG_(INFO) << "ToolInfo_ParseToJson_0700 end";
}

/**
 * @tc.name: ToolInfo_ParseToJson_0800
 * @tc.desc: Test ToolInfo ParseToJson with all invalid schemas
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_ParseToJson_0800, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_ParseToJson_0800 start";

    ToolInfo tool;
    tool.name = "all_invalid_schemas";
    tool.description = "All invalid schemas test";
    tool.inputSchema = "invalid input";
    tool.outputSchema = "invalid output";
    tool.eventSchemas = "invalid events";

    nlohmann::json json = tool.ParseToJson();

    EXPECT_EQ(json["name"], "all_invalid_schemas");
    EXPECT_TRUE(json.contains("inputSchema"));
    EXPECT_EQ(json["inputSchema"], "invalid input");
    EXPECT_TRUE(json.contains("outputSchema"));
    EXPECT_EQ(json["outputSchema"], "invalid output");
    EXPECT_TRUE(json.contains("eventSchemas"));
    EXPECT_EQ(json["eventSchemas"], "invalid events");

    GTEST_LOG_(INFO) << "ToolInfo_ParseToJson_0800 end";
}

/**
 * @tc.name: ToolInfo_ParseToJson_0900
 * @tc.desc: Test ToolInfo ParseToJson with empty inputSchema
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_ParseToJson_0900, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_ParseToJson_0900 start";

    ToolInfo tool;
    tool.name = "empty_input_schema";
    tool.description = "Empty inputSchema test";
    tool.inputSchema = "";
    tool.outputSchema = R"({"type": "string"})";
    tool.eventSchemas = "{}";

    nlohmann::json json = tool.ParseToJson();

    EXPECT_EQ(json["name"], "empty_input_schema");
    EXPECT_FALSE(json.contains("inputSchema"));
    EXPECT_TRUE(json.contains("outputSchema"));

    GTEST_LOG_(INFO) << "ToolInfo_ParseToJson_0900 end";
}

/**
 * @tc.name: ToolInfo_ParseToJson_1000
 * @tc.desc: Test ToolInfo ParseToJson with empty outputSchema
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_ParseToJson_1000, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_ParseToJson_1000 start";

    ToolInfo tool;
    tool.name = "empty_output_schema";
    tool.description = "Empty outputSchema test";
    tool.inputSchema = R"({"type": "object"})";
    tool.outputSchema = "";
    tool.eventSchemas = "{}";

    nlohmann::json json = tool.ParseToJson();

    EXPECT_EQ(json["name"], "empty_output_schema");
    EXPECT_TRUE(json.contains("inputSchema"));
    EXPECT_FALSE(json.contains("outputSchema"));

    GTEST_LOG_(INFO) << "ToolInfo_ParseToJson_1000 end";
}

/**
 * @tc.name: ToolInfo_ParseToJson_1100
 * @tc.desc: Test ToolInfo ParseToJson with empty eventSchemas
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_ParseToJson_1100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_ParseToJson_1100 start";

    ToolInfo tool;
    tool.name = "empty_event_schemas";
    tool.description = "Empty eventSchemas test";
    tool.inputSchema = R"({"type": "object"})";
    tool.outputSchema = R"({"type": "string"})";
    tool.eventSchemas = "";

    nlohmann::json json = tool.ParseToJson();

    EXPECT_EQ(json["name"], "empty_event_schemas");
    EXPECT_TRUE(json.contains("inputSchema"));
    EXPECT_TRUE(json.contains("outputSchema"));
    EXPECT_FALSE(json.contains("eventSchemas"));

    GTEST_LOG_(INFO) << "ToolInfo_ParseToJson_1100 end";
}

/**
 * @tc.name: ToolInfo_ParseToJson_1200
 * @tc.desc: Test ToolInfo ParseToJson with subcommands containing schemas
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_ParseToJson_1200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_ParseToJson_1200 start";

    ToolInfo tool;
    tool.name = "tool_with_subcmd_schemas";
    tool.hasSubCommand = true;

    SubCommandInfo subCmd1;
    subCmd1.description = "Subcommand with valid schemas";
    subCmd1.inputSchema = R"({"type": "object", "properties": {"arg": {"type": "string"}}})";
    subCmd1.outputSchema = R"({"type": "number"})";
    subCmd1.eventSchemas = R"({"result": {"type": "string"}})";

    SubCommandInfo subCmd2;
    subCmd2.description = "Subcommand with invalid schemas";
    subCmd2.inputSchema = "invalid";
    subCmd2.outputSchema = "also invalid";
    subCmd2.eventSchemas = "invalid too";

    tool.subcommands["valid"] = subCmd1;
    tool.subcommands["invalid"] = subCmd2;

    nlohmann::json json = tool.ParseToJson();

    EXPECT_TRUE(json.contains("subcommands"));
    EXPECT_TRUE(json["subcommands"].contains("valid"));
    EXPECT_TRUE(json["subcommands"].contains("invalid"));

    EXPECT_TRUE(json["subcommands"]["valid"].contains("inputSchema"));
    EXPECT_TRUE(json["subcommands"]["valid"]["inputSchema"].is_object());
    EXPECT_TRUE(json["subcommands"]["valid"].contains("outputSchema"));
    EXPECT_TRUE(json["subcommands"]["valid"].contains("eventSchemas"));

    EXPECT_TRUE(json["subcommands"]["invalid"].contains("inputSchema"));
    EXPECT_EQ(json["subcommands"]["invalid"]["inputSchema"], "invalid");
    EXPECT_TRUE(json["subcommands"]["invalid"].contains("outputSchema"));
    EXPECT_EQ(json["subcommands"]["invalid"]["outputSchema"], "also invalid");
    EXPECT_TRUE(json["subcommands"]["invalid"].contains("eventSchemas"));
    EXPECT_EQ(json["subcommands"]["invalid"]["eventSchemas"], "invalid too");

    GTEST_LOG_(INFO) << "ToolInfo_ParseToJson_1200 end";
}

/**
 * @tc.name: ToolInfo_ParseToJson_1300
 * @tc.desc: Test ToolInfo ParseToJson with primitive type schemas
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_ParseToJson_1300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_ParseToJson_1300 start";

    ToolInfo tool;
    tool.name = "primitive_schemas";
    tool.description = "Primitive type schema test";
    tool.inputSchema = R"({"type": "string", "minLength": 1, "maxLength": 100})";
    tool.outputSchema = R"({"type": "number", "minimum": 0, "maximum": 100})";
    tool.eventSchemas = R"({"status": {"type": "boolean"}})";

    nlohmann::json json = tool.ParseToJson();

    EXPECT_TRUE(json.contains("inputSchema"));
    EXPECT_EQ(json["inputSchema"]["type"], "string");
    EXPECT_TRUE(json["inputSchema"].contains("minLength"));

    EXPECT_TRUE(json.contains("outputSchema"));
    EXPECT_EQ(json["outputSchema"]["type"], "number");
    EXPECT_TRUE(json["outputSchema"].contains("minimum"));

    EXPECT_TRUE(json.contains("eventSchemas"));
    EXPECT_EQ(json["eventSchemas"]["status"]["type"], "boolean");

    GTEST_LOG_(INFO) << "ToolInfo_ParseToJson_1300 end";
}

// ==================== ValidateName Tests ====================

/**
 * @tc.name: ToolInfo_ValidateName_0100
 * @tc.desc: Test ToolInfo ValidateName with valid ohos- prefix
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_ValidateName_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_ValidateName_0100 start";

    EXPECT_TRUE(ToolInfo::ValidateName("ohos-ls"));
    EXPECT_TRUE(ToolInfo::ValidateName("ohos-test"));
    EXPECT_TRUE(ToolInfo::ValidateName("ohos-abc"));
    EXPECT_TRUE(ToolInfo::ValidateName("ohos-1234567890123456"));  // 16 chars suffix

    GTEST_LOG_(INFO) << "ToolInfo_ValidateName_0100 end";
}

/**
 * @tc.name: ToolInfo_ValidateName_0200
 * @tc.desc: Test ToolInfo ValidateName with valid hms- prefix
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_ValidateName_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_ValidateName_0200 start";

    EXPECT_TRUE(ToolInfo::ValidateName("hms-ls"));
    EXPECT_TRUE(ToolInfo::ValidateName("hms-test"));
    EXPECT_TRUE(ToolInfo::ValidateName("hms-abc"));
    EXPECT_TRUE(ToolInfo::ValidateName("hms-1234567890123456"));  // 16 chars suffix

    GTEST_LOG_(INFO) << "ToolInfo_ValidateName_0200 end";
}

/**
 * @tc.name: ToolInfo_ValidateName_0300
 * @tc.desc: Test ToolInfo ValidateName with invalid prefix
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_ValidateName_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_ValidateName_0300 start";

    EXPECT_FALSE(ToolInfo::ValidateName("ls"));  // no prefix
    EXPECT_FALSE(ToolInfo::ValidateName("test-tool"));  // wrong prefix
    EXPECT_FALSE(ToolInfo::ValidateName("OHOS-ls"));  // uppercase prefix
    EXPECT_FALSE(ToolInfo::ValidateName("HMS-ls"));  // uppercase prefix
    EXPECT_FALSE(ToolInfo::ValidateName("ohos_ls"));  // underscore instead of dash
    EXPECT_FALSE(ToolInfo::ValidateName("hms_ls"));  // underscore instead of dash
    EXPECT_FALSE(ToolInfo::ValidateName("ohos"));  // no suffix
    EXPECT_FALSE(ToolInfo::ValidateName("hms"));  // no suffix

    GTEST_LOG_(INFO) << "ToolInfo_ValidateName_0300 end";
}

/**
 * @tc.name: ToolInfo_ValidateName_0400
 * @tc.desc: Test ToolInfo ValidateName with suffix exceeding 16 chars
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_ValidateName_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_ValidateName_0400 start";

    EXPECT_FALSE(ToolInfo::ValidateName("ohos-12345678901234567"));  // 17 chars suffix
    EXPECT_FALSE(ToolInfo::ValidateName("hms-abcdefghijklmnopq"));  // 17 chars suffix
    EXPECT_FALSE(ToolInfo::ValidateName("ohos-thisisaverylongname"));  // long suffix

    GTEST_LOG_(INFO) << "ToolInfo_ValidateName_0400 end";
}

/**
 * @tc.name: ToolInfo_ValidateName_0500
 * @tc.desc: Test ToolInfo ValidateName with empty name
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_ValidateName_0500, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_ValidateName_0500 start";

    EXPECT_FALSE(ToolInfo::ValidateName(""));
    EXPECT_FALSE(ToolInfo::ValidateName("ohos-"));  // empty suffix
    EXPECT_FALSE(ToolInfo::ValidateName("hms-"));  // empty suffix

    GTEST_LOG_(INFO) << "ToolInfo_ValidateName_0500 end";
}

/**
 * @tc.name: ToolInfo_ValidateName_0600
 * @tc.desc: Test ToolInfo ValidateName with edge cases
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_ValidateName_0600, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_ValidateName_0600 start";

    // Valid edge cases
    EXPECT_TRUE(ToolInfo::ValidateName("ohos-a"));  // 1 char suffix
    EXPECT_TRUE(ToolInfo::ValidateName("hms-x"));  // 1 char suffix
    EXPECT_TRUE(ToolInfo::ValidateName("ohos-1234567890abcdef"));  // exactly 16 chars
    EXPECT_TRUE(ToolInfo::ValidateName("hms-abcdefghijklmnop"));  // exactly 16 chars

    // Invalid edge cases
    EXPECT_FALSE(ToolInfo::ValidateName("ohos-1234567890abcdefg"));  // 17 chars
    EXPECT_FALSE(ToolInfo::ValidateName("ohos-"));  // 0 chars suffix

    GTEST_LOG_(INFO) << "ToolInfo_ValidateName_0600 end";
}

// ==================== ValidateExecutablePath Tests ====================

/**
 * @tc.name: ToolInfo_ValidateExecutablePath_0100
 * @tc.desc: Test ToolInfo ValidateExecutablePath with valid absolute paths
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_ValidateExecutablePath_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_ValidateExecutablePath_0100 start";

    EXPECT_TRUE(ToolInfo::ValidateExecutablePath("/bin/ls"));
    EXPECT_TRUE(ToolInfo::ValidateExecutablePath("/usr/bin/test"));
    EXPECT_TRUE(ToolInfo::ValidateExecutablePath("/system/bin/tool"));
    EXPECT_TRUE(ToolInfo::ValidateExecutablePath("/"));
    EXPECT_TRUE(ToolInfo::ValidateExecutablePath("/a"));

    GTEST_LOG_(INFO) << "ToolInfo_ValidateExecutablePath_0100 end";
}

/**
 * @tc.name: ToolInfo_ValidateExecutablePath_0200
 * @tc.desc: Test ToolInfo ValidateExecutablePath with invalid paths
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_ValidateExecutablePath_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_ValidateExecutablePath_0200 start";

    EXPECT_FALSE(ToolInfo::ValidateExecutablePath(""));  // empty
    EXPECT_FALSE(ToolInfo::ValidateExecutablePath("bin/ls"));  // relative path
    EXPECT_FALSE(ToolInfo::ValidateExecutablePath("./test"));  // relative path
    EXPECT_FALSE(ToolInfo::ValidateExecutablePath("../tool"));  // relative path
    EXPECT_FALSE(ToolInfo::ValidateExecutablePath("test"));  // no path

    GTEST_LOG_(INFO) << "ToolInfo_ValidateExecutablePath_0200 end";
}

// ==================== ValidateRequirePermissions Tests ====================

/**
 * @tc.name: ToolInfo_ValidateRequirePermissions_0100
 * @tc.desc: Test ToolInfo ValidateRequirePermissions with empty permissions
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_ValidateRequirePermissions_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_ValidateRequirePermissions_0100 start";

    std::vector<std::string> permissions;
    EXPECT_TRUE(ToolInfo::ValidateRequirePermissions(permissions));

    GTEST_LOG_(INFO) << "ToolInfo_ValidateRequirePermissions_0100 end";
}

/**
 * @tc.name: ToolInfo_ValidateRequirePermissions_0200
 * @tc.desc: Test ToolInfo ValidateRequirePermissions with unique permissions
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_ValidateRequirePermissions_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_ValidateRequirePermissions_0200 start";

    std::vector<std::string> permissions = {"ohos.permission.INTERNET", "ohos.permission.CAMERA"};
    EXPECT_TRUE(ToolInfo::ValidateRequirePermissions(permissions));

    GTEST_LOG_(INFO) << "ToolInfo_ValidateRequirePermissions_0200 end";
}

/**
 * @tc.name: ToolInfo_ValidateRequirePermissions_0300
 * @tc.desc: Test ToolInfo ValidateRequirePermissions with duplicate permissions
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_ValidateRequirePermissions_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_ValidateRequirePermissions_0300 start";

    std::vector<std::string> permissions = {"ohos.permission.INTERNET", "ohos.permission.INTERNET"};
    EXPECT_FALSE(ToolInfo::ValidateRequirePermissions(permissions));

    GTEST_LOG_(INFO) << "ToolInfo_ValidateRequirePermissions_0300 end";
}

/**
 * @tc.name: ToolInfo_ValidateRequirePermissions_0400
 * @tc.desc: Test ToolInfo ValidateRequirePermissions with multiple duplicates
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_ValidateRequirePermissions_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_ValidateRequirePermissions_0400 start";

    std::vector<std::string> permissions = {
        "ohos.permission.INTERNET",
        "ohos.permission.CAMERA",
        "ohos.permission.INTERNET",
        "ohos.permission.READ_STORAGE"
    };
    EXPECT_FALSE(ToolInfo::ValidateRequirePermissions(permissions));

    GTEST_LOG_(INFO) << "ToolInfo_ValidateRequirePermissions_0400 end";
}

/**
 * @tc.name: ToolInfo_ValidateRequirePermissions_0500
 * @tc.desc: Test ToolInfo ValidateRequirePermissions with single permission
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_ValidateRequirePermissions_0500, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_ValidateRequirePermissions_0500 start";

    std::vector<std::string> permissions = {"ohos.permission.INTERNET"};
    EXPECT_TRUE(ToolInfo::ValidateRequirePermissions(permissions));

    GTEST_LOG_(INFO) << "ToolInfo_ValidateRequirePermissions_0500 end";
}

/**
 * @tc.name: ToolInfo_ValidateRequirePermissions_0600
 * @tc.desc: Test ToolInfo ValidateRequirePermissions with many unique permissions
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_ValidateRequirePermissions_0600, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_ValidateRequirePermissions_0600 start";

    std::vector<std::string> permissions = {
        "ohos.permission.INTERNET",
        "ohos.permission.CAMERA",
        "ohos.permission.READ_STORAGE",
        "ohos.permission.WRITE_STORAGE",
        "ohos.permission.LOCATION"
    };
    EXPECT_TRUE(ToolInfo::ValidateRequirePermissions(permissions));

    GTEST_LOG_(INFO) << "ToolInfo_ValidateRequirePermissions_0600 end";
}

// ==================== ValidateEventTypes Tests ====================

/**
 * @tc.name: ToolInfo_ValidateEventTypes_0100
 * @tc.desc: Test ToolInfo ValidateEventTypes with empty eventTypes
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_ValidateEventTypes_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_ValidateEventTypes_0100 start";

    std::vector<std::string> eventTypes;
    EXPECT_TRUE(ToolInfo::ValidateEventTypes(eventTypes));

    GTEST_LOG_(INFO) << "ToolInfo_ValidateEventTypes_0100 end";
}

/**
 * @tc.name: ToolInfo_ValidateEventTypes_0200
 * @tc.desc: Test ToolInfo ValidateEventTypes with unique eventTypes
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_ValidateEventTypes_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_ValidateEventTypes_0200 start";

    std::vector<std::string> eventTypes = {"stdout", "stderr", "exit"};
    EXPECT_TRUE(ToolInfo::ValidateEventTypes(eventTypes));

    GTEST_LOG_(INFO) << "ToolInfo_ValidateEventTypes_0200 end";
}

/**
 * @tc.name: ToolInfo_ValidateEventTypes_0300
 * @tc.desc: Test ToolInfo ValidateEventTypes with duplicate eventTypes
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_ValidateEventTypes_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_ValidateEventTypes_0300 start";

    std::vector<std::string> eventTypes = {"stdout", "stdout"};
    EXPECT_FALSE(ToolInfo::ValidateEventTypes(eventTypes));

    GTEST_LOG_(INFO) << "ToolInfo_ValidateEventTypes_0300 end";
}

/**
 * @tc.name: ToolInfo_ValidateEventTypes_0400
 * @tc.desc: Test ToolInfo ValidateEventTypes with multiple duplicates
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_ValidateEventTypes_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_ValidateEventTypes_0400 start";

    std::vector<std::string> eventTypes = {"stdout", "stderr", "stdout", "exit"};
    EXPECT_FALSE(ToolInfo::ValidateEventTypes(eventTypes));

    GTEST_LOG_(INFO) << "ToolInfo_ValidateEventTypes_0400 end";
}

/**
 * @tc.name: ToolInfo_ValidateEventTypes_0500
 * @tc.desc: Test ToolInfo ValidateEventTypes with single eventType
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_ValidateEventTypes_0500, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_ValidateEventTypes_0500 start";

    std::vector<std::string> eventTypes = {"stdout"};
    EXPECT_TRUE(ToolInfo::ValidateEventTypes(eventTypes));

    GTEST_LOG_(INFO) << "ToolInfo_ValidateEventTypes_0500 end";
}

// ==================== ParseFromJson EventTypes Validation Tests ====================

/**
 * @tc.name: ToolInfo_ParseFromJson_EventTypes_0100
 * @tc.desc: Test ToolInfo ParseFromJson with duplicate eventTypes (now allowed)
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_ParseFromJson_EventTypes_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_EventTypes_0100 start";

    nlohmann::json json = R"({
        "name": "ohos-test",
        "version": "1.0.0",
        "description": "Test tool",
        "executablePath": "/bin/test",
        "requirePermissions": [],
        "inputSchema": {},
        "outputSchema": {},
        "eventTypes": ["stdout", "stdout"]
    })"_json;

    ToolInfo tool;
    bool result = ToolInfo::ParseFromJson(json, tool);

    // After removing ValidateEventTypes call, duplicate eventTypes are now allowed
    ASSERT_TRUE(result);
    EXPECT_EQ(tool.eventTypes.size(), 2u);

    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_EventTypes_0100 end";
}

/**
 * @tc.name: ToolInfo_ParseFromJson_EventTypes_0200
 * @tc.desc: Test ToolInfo ParseFromJson with unique eventTypes
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_ParseFromJson_EventTypes_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_EventTypes_0200 start";

    nlohmann::json json = R"({
        "name": "ohos-test",
        "version": "1.0.0",
        "description": "Test tool",
        "executablePath": "/bin/test",
        "requirePermissions": [],
        "inputSchema": {},
        "outputSchema": {},
        "eventTypes": ["stdout", "stderr", "exit"]
    })"_json;

    ToolInfo tool;
    bool result = ToolInfo::ParseFromJson(json, tool);

    ASSERT_TRUE(result);
    EXPECT_EQ(tool.eventTypes.size(), 3u);

    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_EventTypes_0200 end";
}

/**
 * @tc.name: ToolInfo_ParseFromJson_EventTypes_0300
 * @tc.desc: Test ToolInfo ParseFromJson with empty eventTypes
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_ParseFromJson_EventTypes_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_EventTypes_0300 start";

    nlohmann::json json = R"({
        "name": "ohos-test",
        "version": "1.0.0",
        "description": "Test tool",
        "executablePath": "/bin/test",
        "requirePermissions": [],
        "inputSchema": {},
        "outputSchema": {},
        "eventTypes": []
    })"_json;

    ToolInfo tool;
    bool result = ToolInfo::ParseFromJson(json, tool);

    ASSERT_TRUE(result);
    EXPECT_TRUE(tool.eventTypes.empty());

    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_EventTypes_0300 end";
}

/**
 * @tc.name: ToolInfo_ParseFromJson_EventTypes_0400
 * @tc.desc: Test ToolInfo ParseFromJson with empty string in eventTypes (should be skipped)
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_ParseFromJson_EventTypes_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_EventTypes_0400 start";

    nlohmann::json json = R"({
        "name": "ohos-test",
        "version": "1.0.0",
        "description": "Test tool",
        "executablePath": "/bin/test",
        "requirePermissions": [],
        "inputSchema": {},
        "outputSchema": {},
        "eventTypes": ["", "stdout", ""]
    })"_json;

    ToolInfo tool;
    bool result = ToolInfo::ParseFromJson(json, tool);

    ASSERT_TRUE(result);
    EXPECT_EQ(tool.eventTypes.size(), 1u);
    EXPECT_EQ(tool.eventTypes[0], "stdout");

    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_EventTypes_0400 end";
}

// ==================== ParseFromJson RequirePermissions Validation Tests ====================

/**
 * @tc.name: ToolInfo_ParseFromJson_RequirePermissions_0100
 * @tc.desc: Test ToolInfo ParseFromJson with duplicate requirePermissions (now allowed)
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_ParseFromJson_RequirePermissions_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_RequirePermissions_0100 start";

    nlohmann::json json = R"({
        "name": "ohos-test",
        "version": "1.0.0",
        "description": "Test tool",
        "executablePath": "/bin/test",
        "requirePermissions": ["ohos.permission.INTERNET", "ohos.permission.INTERNET"],
        "inputSchema": {},
        "outputSchema": {}
    })"_json;

    ToolInfo tool;
    bool result = ToolInfo::ParseFromJson(json, tool);

    // After removing ValidateRequirePermissions call, duplicate permissions are now allowed
    ASSERT_TRUE(result);
    EXPECT_EQ(tool.requirePermissions.size(), 2u);

    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_RequirePermissions_0100 end";
}

/**
 * @tc.name: ToolInfo_ParseFromJson_RequirePermissions_0200
 * @tc.desc: Test ToolInfo ParseFromJson with unique requirePermissions
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_ParseFromJson_RequirePermissions_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_RequirePermissions_0200 start";

    nlohmann::json json = R"({
        "name": "ohos-test",
        "version": "1.0.0",
        "description": "Test tool",
        "executablePath": "/bin/test",
        "requirePermissions": ["ohos.permission.INTERNET", "ohos.permission.CAMERA"],
        "inputSchema": {},
        "outputSchema": {}
    })"_json;

    ToolInfo tool;
    bool result = ToolInfo::ParseFromJson(json, tool);

    ASSERT_TRUE(result);
    EXPECT_EQ(tool.requirePermissions.size(), 2u);

    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_RequirePermissions_0200 end";
}

/**
 * @tc.name: ToolInfo_ParseFromJson_RequirePermissions_0300
 * @tc.desc: Test ToolInfo ParseFromJson with empty requirePermissions (valid)
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_ParseFromJson_RequirePermissions_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_RequirePermissions_0300 start";

    nlohmann::json json = R"({
        "name": "ohos-test",
        "version": "1.0.0",
        "description": "Test tool",
        "executablePath": "/bin/test",
        "requirePermissions": [],
        "inputSchema": {"type": "object"},
        "outputSchema": {"type": "string"}
    })"_json;

    ToolInfo tool;
    bool result = ToolInfo::ParseFromJson(json, tool);

    EXPECT_TRUE(result);
    EXPECT_TRUE(tool.requirePermissions.empty());

    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_RequirePermissions_0300 end";
}

/**
 * @tc.name: ToolInfo_ParseFromJson_RequirePermissions_0400
 * @tc.desc: Test ToolInfo ParseFromJson with empty string in requirePermissions (should be skipped)
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_ParseFromJson_RequirePermissions_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_RequirePermissions_0400 start";

    nlohmann::json json = R"({
        "name": "ohos-test",
        "version": "1.0.0",
        "description": "Test tool",
        "executablePath": "/bin/test",
        "requirePermissions": ["", "ohos.permission.INTERNET", ""],
        "inputSchema": {},
        "outputSchema": {}
    })"_json;

    ToolInfo tool;
    bool result = ToolInfo::ParseFromJson(json, tool);

    ASSERT_TRUE(result);
    EXPECT_EQ(tool.requirePermissions.size(), 1u);
    EXPECT_EQ(tool.requirePermissions[0], "ohos.permission.INTERNET");

    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_RequirePermissions_0400 end";
}

/**
 * @tc.name: ToolInfo_ParseFromJson_RequirePermissions_0500
 * @tc.desc: Test ToolInfo ParseFromJson without requirePermissions field (should fail - required field)
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_ParseFromJson_RequirePermissions_0500, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_RequirePermissions_0500 start";

    nlohmann::json json = R"({
        "name": "ohos-test",
        "version": "1.0.0",
        "description": "Test tool",
        "executablePath": "/bin/test"
    })"_json;

    ToolInfo tool;
    bool result = ToolInfo::ParseFromJson(json, tool);

    EXPECT_FALSE(result);

    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_RequirePermissions_0500 end";
}

// ==================== ParseFromJson EventSchemas Validation Tests ====================

/**
 * @tc.name: ToolInfo_ParseFromJson_EventSchemas_0100
 * @tc.desc: Test ToolInfo ParseFromJson without eventSchemas (valid)
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_ParseFromJson_EventSchemas_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_EventSchemas_0100 start";

    nlohmann::json json = R"({
        "name": "ohos-test",
        "version": "1.0.0",
        "description": "Test tool",
        "executablePath": "/bin/test",
        "requirePermissions": ["ohos.permission.INTERNET"],
        "inputSchema": {"type": "object"},
        "outputSchema": {"type": "string"}
    })"_json;

    ToolInfo tool;
    bool result = ToolInfo::ParseFromJson(json, tool);

    EXPECT_TRUE(result);
    EXPECT_TRUE(tool.eventSchemas.empty());

    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_EventSchemas_0100 end";
}

/**
 * @tc.name: ToolInfo_ParseFromJson_EventSchemas_0200
 * @tc.desc: Test ToolInfo ParseFromJson with valid eventSchemas object
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_ParseFromJson_EventSchemas_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_EventSchemas_0200 start";

    nlohmann::json json = R"({
        "name": "ohos-test",
        "version": "1.0.0",
        "description": "Test tool",
        "executablePath": "/bin/test",
        "requirePermissions": ["ohos.permission.INTERNET"],
        "inputSchema": {"type": "object"},
        "outputSchema": {"type": "string"},
        "eventSchemas": {"stdout": {"type": "string"}}
    })"_json;

    ToolInfo tool;
    bool result = ToolInfo::ParseFromJson(json, tool);

    EXPECT_TRUE(result);
    EXPECT_EQ(tool.eventSchemas, R"({"stdout":{"type":"string"}})");

    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_EventSchemas_0200 end";
}

/**
 * @tc.name: ToolInfo_ParseFromJson_EventSchemas_0300
 * @tc.desc: Test ToolInfo ParseFromJson with eventSchemas not object (should fail)
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_ParseFromJson_EventSchemas_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_EventSchemas_0300 start";

    nlohmann::json json = R"({
        "name": "ohos-test",
        "version": "1.0.0",
        "description": "Test tool",
        "executablePath": "/bin/test",
        "requirePermissions": ["ohos.permission.INTERNET"],
        "inputSchema": {"type": "object"},
        "outputSchema": {"type": "string"},
        "eventSchemas": "not an object"
    })"_json;

    ToolInfo tool;
    bool result = ToolInfo::ParseFromJson(json, tool);

    EXPECT_FALSE(result);

    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_EventSchemas_0300 end";
}

/**
 * @tc.name: ToolInfo_ParseFromJson_EventSchemas_0400
 * @tc.desc: Test ToolInfo ParseFromJson with eventSchemas as array (should fail)
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_ParseFromJson_EventSchemas_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_EventSchemas_0400 start";

    nlohmann::json json = R"({
        "name": "ohos-test",
        "version": "1.0.0",
        "description": "Test tool",
        "executablePath": "/bin/test",
        "requirePermissions": ["ohos.permission.INTERNET"],
        "inputSchema": {"type": "object"},
        "outputSchema": {"type": "string"},
        "eventSchemas": ["a", "b"]
    })"_json;

    ToolInfo tool;
    bool result = ToolInfo::ParseFromJson(json, tool);

    EXPECT_FALSE(result);

    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_EventSchemas_0400 end";
}

// ==================== ParseFromJson InputSchema/OutputSchema Validation Tests ====================

/**
 * @tc.name: ToolInfo_ParseFromJson_Schema_0100
 * @tc.desc: Test ToolInfo ParseFromJson with inputSchema not object
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_ParseFromJson_Schema_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_Schema_0100 start";

    nlohmann::json json = R"({
        "name": "ohos-test",
        "version": "1.0.0",
        "description": "Test tool",
        "executablePath": "/bin/test",
        "requirePermissions": ["ohos.permission.INTERNET"],
        "inputSchema": "not an object",
        "outputSchema": {"type": "string"}
    })"_json;

    ToolInfo tool;
    bool result = ToolInfo::ParseFromJson(json, tool);

    EXPECT_FALSE(result);

    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_Schema_0100 end";
}

/**
 * @tc.name: ToolInfo_ParseFromJson_Schema_0200
 * @tc.desc: Test ToolInfo ParseFromJson with inputSchema as array
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_ParseFromJson_Schema_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_Schema_0200 start";

    nlohmann::json json = R"({
        "name": "ohos-test",
        "version": "1.0.0",
        "description": "Test tool",
        "executablePath": "/bin/test",
        "requirePermissions": ["ohos.permission.INTERNET"],
        "inputSchema": ["a", "b"],
        "outputSchema": {"type": "string"}
    })"_json;

    ToolInfo tool;
    bool result = ToolInfo::ParseFromJson(json, tool);

    EXPECT_FALSE(result);

    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_Schema_0200 end";
}

/**
 * @tc.name: ToolInfo_ParseFromJson_Schema_0300
 * @tc.desc: Test ToolInfo ParseFromJson with inputSchema as number
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_ParseFromJson_Schema_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_Schema_0300 start";

    nlohmann::json json = R"({
        "name": "ohos-test",
        "version": "1.0.0",
        "description": "Test tool",
        "executablePath": "/bin/test",
        "requirePermissions": ["ohos.permission.INTERNET"],
        "inputSchema": 123,
        "outputSchema": {"type": "string"}
    })"_json;

    ToolInfo tool;
    bool result = ToolInfo::ParseFromJson(json, tool);

    EXPECT_FALSE(result);

    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_Schema_0300 end";
}

/**
 * @tc.name: ToolInfo_ParseFromJson_Schema_0400
 * @tc.desc: Test ToolInfo ParseFromJson with valid inputSchema object
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_ParseFromJson_Schema_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_Schema_0400 start";

    nlohmann::json json = R"({
        "name": "ohos-test",
        "version": "1.0.0",
        "description": "Test tool",
        "executablePath": "/bin/test",
        "requirePermissions": ["ohos.permission.INTERNET"],
        "inputSchema": {"type": "object"},
        "outputSchema": {"type": "string"}
    })"_json;

    ToolInfo tool;
    bool result = ToolInfo::ParseFromJson(json, tool);

    EXPECT_TRUE(result);
    EXPECT_EQ(tool.inputSchema, R"({"type":"object"})");

    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_Schema_0400 end";
}

/**
 * @tc.name: ToolInfo_ParseFromJson_Schema_0500
 * @tc.desc: Test ToolInfo ParseFromJson without inputSchema (should fail - required field)
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_ParseFromJson_Schema_0500, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_Schema_0500 start";

    nlohmann::json json = R"({
        "name": "ohos-test",
        "version": "1.0.0",
        "description": "Test tool",
        "executablePath": "/bin/test",
        "requirePermissions": ["ohos.permission.INTERNET"],
        "outputSchema": {"type": "string"}
    })"_json;

    ToolInfo tool;
    bool result = ToolInfo::ParseFromJson(json, tool);

    EXPECT_FALSE(result);

    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_Schema_0500 end";
}

/**
 * @tc.name: ToolInfo_ParseFromJson_Schema_0600
 * @tc.desc: Test ToolInfo ParseFromJson with outputSchema not object
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_ParseFromJson_Schema_0600, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_Schema_0600 start";

    nlohmann::json json = R"({
        "name": "ohos-test",
        "version": "1.0.0",
        "description": "Test tool",
        "executablePath": "/bin/test",
        "requirePermissions": ["ohos.permission.INTERNET"],
        "inputSchema": {"type": "object"},
        "outputSchema": "not an object"
    })"_json;

    ToolInfo tool;
    bool result = ToolInfo::ParseFromJson(json, tool);

    EXPECT_FALSE(result);

    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_Schema_0600 end";
}

/**
 * @tc.name: ToolInfo_ParseFromJson_Schema_0700
 * @tc.desc: Test ToolInfo ParseFromJson with outputSchema as array
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_ParseFromJson_Schema_0700, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_Schema_0700 start";

    nlohmann::json json = R"({
        "name": "ohos-test",
        "version": "1.0.0",
        "description": "Test tool",
        "executablePath": "/bin/test",
        "requirePermissions": ["ohos.permission.INTERNET"],
        "inputSchema": {"type": "object"},
        "outputSchema": ["a", "b"]
    })"_json;

    ToolInfo tool;
    bool result = ToolInfo::ParseFromJson(json, tool);

    EXPECT_FALSE(result);

    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_Schema_0700 end";
}

/**
 * @tc.name: ToolInfo_ParseFromJson_Schema_0800
 * @tc.desc: Test ToolInfo ParseFromJson with valid outputSchema object
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_ParseFromJson_Schema_0800, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_Schema_0800 start";

    nlohmann::json json = R"({
        "name": "ohos-test",
        "version": "1.0.0",
        "description": "Test tool",
        "executablePath": "/bin/test",
        "requirePermissions": ["ohos.permission.INTERNET"],
        "inputSchema": {"type": "object"},
        "outputSchema": {"type": "string"}
    })"_json;

    ToolInfo tool;
    bool result = ToolInfo::ParseFromJson(json, tool);

    EXPECT_TRUE(result);
    EXPECT_EQ(tool.outputSchema, R"({"type":"string"})");

    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_Schema_0800 end";
}

/**
 * @tc.name: ToolInfo_ParseFromJson_Schema_0900
 * @tc.desc: Test ToolInfo ParseFromJson without outputSchema (should fail - required field)
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_ParseFromJson_Schema_0900, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_Schema_0900 start";

    nlohmann::json json = R"({
        "name": "ohos-test",
        "version": "1.0.0",
        "description": "Test tool",
        "executablePath": "/bin/test",
        "requirePermissions": ["ohos.permission.INTERNET"],
        "inputSchema": {"type": "object"}
    })"_json;

    ToolInfo tool;
    bool result = ToolInfo::ParseFromJson(json, tool);

    EXPECT_FALSE(result);

    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_Schema_0900 end";
}

/**
 * @tc.name: ToolInfo_ParseFromJson_Schema_1000
 * @tc.desc: Test ToolInfo ParseFromJson with both valid schemas
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_ParseFromJson_Schema_1000, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_Schema_1000 start";

    nlohmann::json json = R"({
        "name": "ohos-test",
        "version": "1.0.0",
        "description": "Test tool",
        "executablePath": "/bin/test",
        "requirePermissions": ["ohos.permission.INTERNET"],
        "inputSchema": {"type": "object"},
        "outputSchema": {"type": "array"}
    })"_json;

    ToolInfo tool;
    bool result = ToolInfo::ParseFromJson(json, tool);

    EXPECT_TRUE(result);
    EXPECT_EQ(tool.inputSchema, R"({"type":"object"})");
    EXPECT_EQ(tool.outputSchema, R"({"type":"array"})");

    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_Schema_1000 end";
}

// ==================== ParseFromJson Name Validation Tests ====================

/**
 * @tc.name: ToolInfo_ParseFromJson_NameValidation_0100
 * @tc.desc: Test ToolInfo ParseFromJson with invalid name (no prefix)
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_ParseFromJson_NameValidation_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_NameValidation_0100 start";

    nlohmann::json json = R"({
        "name": "invalid_tool",
        "version": "1.0.0"
    })"_json;

    ToolInfo tool;
    bool result = ToolInfo::ParseFromJson(json, tool);

    EXPECT_FALSE(result);
    EXPECT_TRUE(tool.name.empty());

    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_NameValidation_0100 end";
}

/**
 * @tc.name: ToolInfo_ParseFromJson_NameValidation_0200
 * @tc.desc: Test ToolInfo ParseFromJson with invalid name (suffix too long)
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_ParseFromJson_NameValidation_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_NameValidation_0200 start";

    nlohmann::json json = R"({
        "name": "ohos-thisisaverylongtoolname",
        "version": "1.0.0"
    })"_json;

    ToolInfo tool;
    bool result = ToolInfo::ParseFromJson(json, tool);

    EXPECT_FALSE(result);

    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_NameValidation_0200 end";
}

/**
 * @tc.name: ToolInfo_ParseFromJson_NameValidation_0300
 * @tc.desc: Test ToolInfo ParseFromJson without name field
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_ParseFromJson_NameValidation_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_NameValidation_0300 start";

    nlohmann::json json = R"({
        "version": "1.0.0",
        "description": "Tool without name"
    })"_json;

    ToolInfo tool;
    bool result = ToolInfo::ParseFromJson(json, tool);

    EXPECT_FALSE(result);

    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_NameValidation_0300 end";
}

/**
 * @tc.name: ToolInfo_ParseFromJson_NameValidation_0400
 * @tc.desc: Test ToolInfo ParseFromJson with name not string
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_ParseFromJson_NameValidation_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_NameValidation_0400 start";

    nlohmann::json json = R"({
        "name": 123,
        "version": "1.0.0"
    })"_json;

    ToolInfo tool;
    bool result = ToolInfo::ParseFromJson(json, tool);

    EXPECT_FALSE(result);

    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_NameValidation_0400 end";
}

// ==================== ParseFromJson Required Fields Validation Tests ====================

/**
 * @tc.name: ToolInfo_ParseFromJson_RequiredFields_0100
 * @tc.desc: Test ToolInfo ParseFromJson without version field
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_ParseFromJson_RequiredFields_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_RequiredFields_0100 start";

    nlohmann::json json = R"({
        "name": "ohos-test",
        "description": "Tool without version",
        "executablePath": "/bin/test"
    })"_json;

    ToolInfo tool;
    bool result = ToolInfo::ParseFromJson(json, tool);

    EXPECT_FALSE(result);

    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_RequiredFields_0100 end";
}

/**
 * @tc.name: ToolInfo_ParseFromJson_RequiredFields_0200
 * @tc.desc: Test ToolInfo ParseFromJson with version not string
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_ParseFromJson_RequiredFields_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_RequiredFields_0200 start";

    nlohmann::json json = R"({
        "name": "ohos-test",
        "version": 123,
        "description": "Tool",
        "executablePath": "/bin/test"
    })"_json;

    ToolInfo tool;
    bool result = ToolInfo::ParseFromJson(json, tool);

    EXPECT_FALSE(result);

    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_RequiredFields_0200 end";
}

/**
 * @tc.name: ToolInfo_ParseFromJson_RequiredFields_0220
 * @tc.desc: Test ToolInfo ParseFromJson with empty version
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_ParseFromJson_RequiredFields_0220, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_RequiredFields_0220 start";

    nlohmann::json json = R"({
        "name": "ohos-test",
        "version": "",
        "description": "Tool",
        "executablePath": "/bin/test"
    })"_json;

    ToolInfo tool;
    bool result = ToolInfo::ParseFromJson(json, tool);

    EXPECT_FALSE(result);

    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_RequiredFields_0220 end";
}

/**
 * @tc.name: ToolInfo_ParseFromJson_RequiredFields_0300
 * @tc.desc: Test ToolInfo ParseFromJson without description field
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_ParseFromJson_RequiredFields_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_RequiredFields_0300 start";

    nlohmann::json json = R"({
        "name": "ohos-test",
        "version": "1.0.0",
        "executablePath": "/bin/test"
    })"_json;

    ToolInfo tool;
    bool result = ToolInfo::ParseFromJson(json, tool);

    EXPECT_FALSE(result);

    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_RequiredFields_0300 end";
}

/**
 * @tc.name: ToolInfo_ParseFromJson_RequiredFields_0400
 * @tc.desc: Test ToolInfo ParseFromJson with description not string
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_ParseFromJson_RequiredFields_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_RequiredFields_0400 start";

    nlohmann::json json = R"({
        "name": "ohos-test",
        "version": "1.0.0",
        "description": 123,
        "executablePath": "/bin/test"
    })"_json;

    ToolInfo tool;
    bool result = ToolInfo::ParseFromJson(json, tool);

    EXPECT_FALSE(result);

    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_RequiredFields_0400 end";
}

/**
 * @tc.name: ToolInfo_ParseFromJson_RequiredFields_0420
 * @tc.desc: Test ToolInfo ParseFromJson with empty description
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_ParseFromJson_RequiredFields_0420, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_RequiredFields_0420 start";

    nlohmann::json json = R"({
        "name": "ohos-test",
        "version": "1.0.0",
        "description": "",
        "executablePath": "/bin/test"
    })"_json;

    ToolInfo tool;
    bool result = ToolInfo::ParseFromJson(json, tool);

    EXPECT_FALSE(result);

    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_RequiredFields_0420 end";
}

/**
 * @tc.name: ToolInfo_ParseFromJson_RequiredFields_0500
 * @tc.desc: Test ToolInfo ParseFromJson without executablePath field
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_ParseFromJson_RequiredFields_0500, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_RequiredFields_0500 start";

    nlohmann::json json = R"({
        "name": "ohos-test",
        "version": "1.0.0",
        "description": "Tool without path"
    })"_json;

    ToolInfo tool;
    bool result = ToolInfo::ParseFromJson(json, tool);

    EXPECT_FALSE(result);

    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_RequiredFields_0500 end";
}

/**
 * @tc.name: ToolInfo_ParseFromJson_RequiredFields_0600
 * @tc.desc: Test ToolInfo ParseFromJson with executablePath not string
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_ParseFromJson_RequiredFields_0600, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_RequiredFields_0600 start";

    nlohmann::json json = R"({
        "name": "ohos-test",
        "version": "1.0.0",
        "description": "Tool",
        "executablePath": 123
    })"_json;

    ToolInfo tool;
    bool result = ToolInfo::ParseFromJson(json, tool);

    EXPECT_FALSE(result);

    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_RequiredFields_0600 end";
}

/**
 * @tc.name: ToolInfo_ParseFromJson_RequiredFields_0700
 * @tc.desc: Test ToolInfo ParseFromJson with relative executablePath
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_ParseFromJson_RequiredFields_0700, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_RequiredFields_0700 start";

    nlohmann::json json = R"({
        "name": "ohos-test",
        "version": "1.0.0",
        "description": "Tool",
        "executablePath": "bin/test"
    })"_json;

    ToolInfo tool;
    bool result = ToolInfo::ParseFromJson(json, tool);

    EXPECT_FALSE(result);

    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_RequiredFields_0700 end";
}

/**
 * @tc.name: ToolInfo_ParseFromJson_RequiredFields_0800
 * @tc.desc: Test ToolInfo ParseFromJson with empty executablePath
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_ParseFromJson_RequiredFields_0800, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_RequiredFields_0800 start";

    nlohmann::json json = R"({
        "name": "ohos-test",
        "version": "1.0.0",
        "description": "Tool",
        "executablePath": ""
    })"_json;

    ToolInfo tool;
    bool result = ToolInfo::ParseFromJson(json, tool);

    EXPECT_FALSE(result);

    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_RequiredFields_0800 end";
}

/**
 * @tc.name: ToolInfo_ParseFromJson_RequiredFields_0900
 * @tc.desc: Test ToolInfo ParseFromJson with all required fields valid
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_ParseFromJson_RequiredFields_0900, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_RequiredFields_0900 start";

    nlohmann::json json = R"({
        "name": "ohos-testtool",
        "version": "1.0.0",
        "description": "A valid tool",
        "executablePath": "/bin/test",
        "requirePermissions": ["ohos.permission.INTERNET"],
        "inputSchema": {"type": "object"},
        "outputSchema": {"type": "string"}
    })"_json;

    ToolInfo tool;
    bool result = ToolInfo::ParseFromJson(json, tool);

    EXPECT_TRUE(result);
    EXPECT_EQ(tool.name, "ohos-testtool");
    EXPECT_EQ(tool.version, "1.0.0");
    EXPECT_EQ(tool.description, "A valid tool");
    EXPECT_EQ(tool.executablePath, "/bin/test");

    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_RequiredFields_0900 end";
}

// ==================== Validate Tests ====================

/**
 * @tc.name: ToolInfo_Validate_0100
 * @tc.desc: Test ToolInfo::Validate with valid data
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_Validate_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_Validate_0100 start";

    ToolInfo tool;
    tool.name = "ohos-valid_tool";
    tool.version = "1.0.0";
    tool.description = "A valid tool";
    tool.executablePath = "/bin/test";
    tool.requirePermissions = {"ohos.permission.INTERNET"};
    tool.inputSchema = R"({"type": "object"})";
    tool.outputSchema = R"({"type": "string"})";

    EXPECT_TRUE(ToolInfo::Validate(tool));

    GTEST_LOG_(INFO) << "ToolInfo_Validate_0100 end";
}

/**
 * @tc.name: ToolInfo_Validate_0200
 * @tc.desc: Test ToolInfo::Validate with invalid name
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_Validate_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_Validate_0200 start";

    ToolInfo tool;
    tool.name = "invalid_name";
    tool.version = "1.0.0";
    tool.description = "Test";
    tool.executablePath = "/bin/test";
    tool.requirePermissions = {"ohos.permission.INTERNET"};
    tool.inputSchema = "{}";
    tool.outputSchema = "{}";

    EXPECT_FALSE(ToolInfo::Validate(tool));

    GTEST_LOG_(INFO) << "ToolInfo_Validate_0200 end";
}

/**
 * @tc.name: ToolInfo_Validate_0300
 * @tc.desc: Test ToolInfo::Validate with empty version
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_Validate_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_Validate_0300 start";

    ToolInfo tool;
    tool.name = "ohos-test";
    tool.version = "";
    tool.description = "Test";
    tool.executablePath = "/bin/test";
    tool.requirePermissions = {"ohos.permission.INTERNET"};
    tool.inputSchema = "{}";
    tool.outputSchema = "{}";

    EXPECT_FALSE(ToolInfo::Validate(tool));

    GTEST_LOG_(INFO) << "ToolInfo_Validate_0300 end";
}

/**
 * @tc.name: ToolInfo_Validate_0400
 * @tc.desc: Test ToolInfo::Validate with empty description
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_Validate_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_Validate_0400 start";

    ToolInfo tool;
    tool.name = "ohos-test";
    tool.version = "1.0.0";
    tool.description = "";
    tool.executablePath = "/bin/test";
    tool.requirePermissions = {"ohos.permission.INTERNET"};
    tool.inputSchema = "{}";
    tool.outputSchema = "{}";

    EXPECT_FALSE(ToolInfo::Validate(tool));

    GTEST_LOG_(INFO) << "ToolInfo_Validate_0400 end";
}

/**
 * @tc.name: ToolInfo_Validate_0500
 * @tc.desc: Test ToolInfo::Validate with invalid executablePath
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_Validate_0500, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_Validate_0500 start";

    ToolInfo tool;
    tool.name = "ohos-test";
    tool.version = "1.0.0";
    tool.description = "Test";
    tool.executablePath = "bin/test";
    tool.requirePermissions = {"ohos.permission.INTERNET"};
    tool.inputSchema = "{}";
    tool.outputSchema = "{}";

    EXPECT_FALSE(ToolInfo::Validate(tool));

    GTEST_LOG_(INFO) << "ToolInfo_Validate_0500 end";
}

/**
 * @tc.name: ToolInfo_Validate_0550
 * @tc.desc: Test ToolInfo::Validate with empty requirePermissions (valid)
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_Validate_0550, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_Validate_0550 start";

    ToolInfo tool;
    tool.name = "ohos-test";
    tool.version = "1.0.0";
    tool.description = "Test";
    tool.executablePath = "/bin/test";
    tool.requirePermissions = {};
    tool.inputSchema = "{}";
    tool.outputSchema = "{}";

    EXPECT_TRUE(ToolInfo::Validate(tool));

    GTEST_LOG_(INFO) << "ToolInfo_Validate_0550 end";
}

/**
 * @tc.name: ToolInfo_Validate_0600
 * @tc.desc: Test ToolInfo::Validate with duplicate requirePermissions (duplicates are allowed)
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_Validate_0600, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_Validate_0600 start";

    ToolInfo tool;
    tool.name = "ohos-test";
    tool.version = "1.0.0";
    tool.description = "Test";
    tool.executablePath = "/bin/test";
    tool.requirePermissions = {"ohos.permission.INTERNET", "ohos.permission.INTERNET"};
    tool.inputSchema = "{}";
    tool.outputSchema = "{}";

    EXPECT_TRUE(ToolInfo::Validate(tool));  // duplicate permissions are now allowed

    GTEST_LOG_(INFO) << "ToolInfo_Validate_0600 end";
}

/**
 * @tc.name: ToolInfo_Validate_0700
 * @tc.desc: Test ToolInfo::Validate with unique requirePermissions
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_Validate_0700, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_Validate_0700 start";

    ToolInfo tool;
    tool.name = "ohos-test";
    tool.version = "1.0.0";
    tool.description = "Test";
    tool.executablePath = "/bin/test";
    tool.requirePermissions = {"ohos.permission.INTERNET", "ohos.permission.CAMERA"};
    tool.inputSchema = "{}";
    tool.outputSchema = "{}";

    EXPECT_TRUE(ToolInfo::Validate(tool));

    GTEST_LOG_(INFO) << "ToolInfo_Validate_0700 end";
}

/**
 * @tc.name: ToolInfo_Validate_0800
 * @tc.desc: Test ToolInfo::Validate with empty inputSchema (should fail - required field)
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_Validate_0800, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_Validate_0800 start";

    ToolInfo tool;
    tool.name = "ohos-test";
    tool.version = "1.0.0";
    tool.description = "Test";
    tool.executablePath = "/bin/test";
    tool.requirePermissions = {"ohos.permission.INTERNET"};
    tool.inputSchema = "";
    tool.outputSchema = "{}";

    EXPECT_FALSE(ToolInfo::Validate(tool));

    GTEST_LOG_(INFO) << "ToolInfo_Validate_0800 end";
}

/**
 * @tc.name: ToolInfo_Validate_0900
 * @tc.desc: Test ToolInfo::Validate with invalid inputSchema JSON
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_Validate_0900, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_Validate_0900 start";

    ToolInfo tool;
    tool.name = "ohos-test";
    tool.version = "1.0.0";
    tool.description = "Test";
    tool.executablePath = "/bin/test";
    tool.requirePermissions = {"ohos.permission.INTERNET"};
    tool.inputSchema = "not valid json";
    tool.outputSchema = "{}";

    EXPECT_FALSE(ToolInfo::Validate(tool));

    GTEST_LOG_(INFO) << "ToolInfo_Validate_0900 end";
}

/**
 * @tc.name: ToolInfo_Validate_1000
 * @tc.desc: Test ToolInfo::Validate with empty outputSchema (should fail - required field)
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_Validate_1000, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_Validate_1000 start";

    ToolInfo tool;
    tool.name = "ohos-test";
    tool.version = "1.0.0";
    tool.description = "Test";
    tool.executablePath = "/bin/test";
    tool.requirePermissions = {"ohos.permission.INTERNET"};
    tool.inputSchema = "{}";
    tool.outputSchema = "";

    EXPECT_FALSE(ToolInfo::Validate(tool));

    GTEST_LOG_(INFO) << "ToolInfo_Validate_1000 end";
}

/**
 * @tc.name: ToolInfo_Validate_1100
 * @tc.desc: Test ToolInfo::Validate with invalid outputSchema JSON
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_Validate_1100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_Validate_1100 start";

    ToolInfo tool;
    tool.name = "ohos-test";
    tool.version = "1.0.0";
    tool.description = "Test";
    tool.executablePath = "/bin/test";
    tool.requirePermissions = {"ohos.permission.INTERNET"};
    tool.inputSchema = "{}";
    tool.outputSchema = "{invalid}";

    EXPECT_FALSE(ToolInfo::Validate(tool));

    GTEST_LOG_(INFO) << "ToolInfo_Validate_1100 end";
}

/**
 * @tc.name: ToolInfo_Validate_1700
 * @tc.desc: Test ToolInfo::Validate with duplicate eventTypes (duplicates are allowed)
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_Validate_1700, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_Validate_1700 start";

    ToolInfo tool;
    tool.name = "ohos-test";
    tool.version = "1.0.0";
    tool.description = "Test";
    tool.executablePath = "/bin/test";
    tool.requirePermissions = {"ohos.permission.INTERNET"};
    tool.inputSchema = "{}";
    tool.outputSchema = "{}";
    tool.eventTypes = {"stdout", "stdout"};

    EXPECT_TRUE(ToolInfo::Validate(tool));  // duplicate eventTypes are now allowed

    GTEST_LOG_(INFO) << "ToolInfo_Validate_1700 end";
}

/**
 * @tc.name: ToolInfo_Validate_1800
 * @tc.desc: Test ToolInfo::Validate with unique eventTypes
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_Validate_1800, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_Validate_1800 start";

    ToolInfo tool;
    tool.name = "ohos-test";
    tool.version = "1.0.0";
    tool.description = "Test";
    tool.executablePath = "/bin/test";
    tool.requirePermissions = {"ohos.permission.INTERNET"};
    tool.inputSchema = "{}";
    tool.outputSchema = "{}";
    tool.eventTypes = {"stdout", "stderr", "exit"};

    EXPECT_TRUE(ToolInfo::Validate(tool));

    GTEST_LOG_(INFO) << "ToolInfo_Validate_1800 end";
}

/**
 * @tc.name: ToolInfo_Validate_1900
 * @tc.desc: Test ToolInfo::Validate with invalid eventSchemas JSON
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_Validate_1900, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_Validate_1900 start";

    ToolInfo tool;
    tool.name = "ohos-test";
    tool.version = "1.0.0";
    tool.description = "Test";
    tool.executablePath = "/bin/test";
    tool.requirePermissions = {"ohos.permission.INTERNET"};
    tool.inputSchema = "{}";
    tool.outputSchema = "{}";
    tool.eventSchemas = "invalid json";

    EXPECT_FALSE(ToolInfo::Validate(tool));

    GTEST_LOG_(INFO) << "ToolInfo_Validate_1900 end";
}

/**
 * @tc.name: ToolInfo_Validate_2000
 * @tc.desc: Test ToolInfo::Validate with valid eventSchemas
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_Validate_2000, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_Validate_2000 start";

    ToolInfo tool;
    tool.name = "ohos-test";
    tool.version = "1.0.0";
    tool.description = "Test";
    tool.executablePath = "/bin/test";
    tool.requirePermissions = {"ohos.permission.INTERNET"};
    tool.inputSchema = "{}";
    tool.outputSchema = "{}";
    tool.eventSchemas = R"({"stdout": {"type": "string"}})";

    EXPECT_TRUE(ToolInfo::Validate(tool));

    GTEST_LOG_(INFO) << "ToolInfo_Validate_2000 end";
}

/**
 * @tc.name: ToolInfo_Validate_2100
 * @tc.desc: Test ToolInfo::Validate with hasSubCommand true but empty subcommands
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_Validate_2100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_Validate_2100 start";

    ToolInfo tool;
    tool.name = "ohos-test";
    tool.version = "1.0.0";
    tool.description = "Test";
    tool.executablePath = "/bin/test";
    tool.requirePermissions = {"ohos.permission.INTERNET"};
    tool.inputSchema = "{}";
    tool.outputSchema = "{}";
    tool.hasSubCommand = true;
    tool.subcommands = {};

    EXPECT_FALSE(ToolInfo::Validate(tool));

    GTEST_LOG_(INFO) << "ToolInfo_Validate_2100 end";
}

/**
 * @tc.name: ToolInfo_Validate_2200
 * @tc.desc: Test ToolInfo::Validate with hasSubCommand true and non-empty subcommands
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_Validate_2200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_Validate_2200 start";

    ToolInfo tool;
    tool.name = "ohos-test";
    tool.version = "1.0.0";
    tool.description = "Test";
    tool.executablePath = "/bin/test";
    tool.requirePermissions = {"ohos.permission.INTERNET"};
    tool.inputSchema = "{}";
    tool.outputSchema = "{}";
    tool.hasSubCommand = true;

    SubCommandInfo subCmd;
    subCmd.description = "Test subcommand";
    subCmd.inputSchema = "{}";
    subCmd.outputSchema = "{}";
    tool.subcommands["sub1"] = subCmd;

    EXPECT_TRUE(ToolInfo::Validate(tool));

    GTEST_LOG_(INFO) << "ToolInfo_Validate_2200 end";
}

/**
 * @tc.name: ToolInfo_Validate_2300
 * @tc.desc: Test ToolInfo::Validate with hasSubCommand false and empty subcommands
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_Validate_2300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_Validate_2300 start";

    ToolInfo tool;
    tool.name = "ohos-test";
    tool.version = "1.0.0";
    tool.description = "Test";
    tool.executablePath = "/bin/test";
    tool.requirePermissions = {"ohos.permission.INTERNET"};
    tool.inputSchema = "{}";
    tool.outputSchema = "{}";
    tool.hasSubCommand = false;
    tool.subcommands = {};

    EXPECT_TRUE(ToolInfo::Validate(tool));

    GTEST_LOG_(INFO) << "ToolInfo_Validate_2300 end";
}

// ==================== ParseFromJson HasSubCommand Validation Tests ====================

/**
 * @tc.name: ToolInfo_ParseFromJson_HasSubCommand_0100
 * @tc.desc: Test ToolInfo ParseFromJson with hasSubCommand not boolean (should fail)
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_ParseFromJson_HasSubCommand_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_HasSubCommand_0100 start";

    nlohmann::json json = R"({
        "name": "ohos-test",
        "version": "1.0.0",
        "description": "Test tool",
        "executablePath": "/bin/test",
        "hasSubCommand": "true"
    })"_json;

    ToolInfo tool;
    bool result = ToolInfo::ParseFromJson(json, tool);

    EXPECT_FALSE(result);

    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_HasSubCommand_0100 end";
}

/**
 * @tc.name: ToolInfo_ParseFromJson_HasSubCommand_0200
 * @tc.desc: Test ToolInfo ParseFromJson with hasSubCommand false and subcommands present (ignored)
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_ParseFromJson_HasSubCommand_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_HasSubCommand_0200 start";

    nlohmann::json json = R"({
        "name": "ohos-test",
        "version": "1.0.0",
        "description": "Test tool",
        "executablePath": "/bin/test",
        "requirePermissions": ["ohos.permission.INTERNET"],
        "inputSchema": {"type": "object"},
        "outputSchema": {"type": "string"},
        "hasSubCommand": false,
        "subcommands": {
            "sub1": {
                "description": "Subcommand 1",
                "inputSchema": {},
                "outputSchema": {}
            }
        }
    })"_json;

    ToolInfo tool;
    bool result = ToolInfo::ParseFromJson(json, tool);

    EXPECT_TRUE(result);
    EXPECT_FALSE(tool.hasSubCommand);
    EXPECT_TRUE(tool.subcommands.empty());

    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_HasSubCommand_0200 end";
}

/**
 * @tc.name: ToolInfo_ParseFromJson_HasSubCommand_0300
 * @tc.desc: Test ToolInfo ParseFromJson with hasSubCommand true but no subcommands (should fail)
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_ParseFromJson_HasSubCommand_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_HasSubCommand_0300 start";

    nlohmann::json json = R"({
        "name": "ohos-test",
        "version": "1.0.0",
        "description": "Test tool",
        "executablePath": "/bin/test",
        "hasSubCommand": true
    })"_json;

    ToolInfo tool;
    bool result = ToolInfo::ParseFromJson(json, tool);

    EXPECT_FALSE(result);

    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_HasSubCommand_0300 end";
}

/**
 * @tc.name: ToolInfo_ParseFromJson_HasSubCommand_0400
 * @tc.desc: Test ToolInfo ParseFromJson with hasSubCommand true but empty subcommands (should fail)
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_ParseFromJson_HasSubCommand_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_HasSubCommand_0400 start";

    nlohmann::json json = R"({
        "name": "ohos-test",
        "version": "1.0.0",
        "description": "Test tool",
        "executablePath": "/bin/test",
        "hasSubCommand": true,
        "subcommands": {}
    })"_json;

    ToolInfo tool;
    bool result = ToolInfo::ParseFromJson(json, tool);

    EXPECT_FALSE(result);

    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_HasSubCommand_0400 end";
}

/**
 * @tc.name: ToolInfo_ParseFromJson_HasSubCommand_0500
 * @tc.desc: Test ToolInfo ParseFromJson with hasSubCommand true and valid subcommands
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_ParseFromJson_HasSubCommand_0500, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_HasSubCommand_0500 start";

    nlohmann::json json = R"({
        "name": "ohos-test",
        "version": "1.0.0",
        "description": "Test tool",
        "executablePath": "/bin/test",
        "requirePermissions": ["ohos.permission.INTERNET"],
        "inputSchema": {"type": "object"},
        "outputSchema": {"type": "string"},
        "hasSubCommand": true,
        "subcommands": {
            "build": {
                "description": "Build subcommand",
                "requirePermissions": [],
                "inputSchema": {},
                "outputSchema": {}
            }
        }
    })"_json;

    ToolInfo tool;
    bool result = ToolInfo::ParseFromJson(json, tool);

    EXPECT_TRUE(result);
    EXPECT_TRUE(tool.hasSubCommand);
    EXPECT_EQ(tool.subcommands.size(), 1u);
    EXPECT_TRUE(tool.subcommands.find("build") != tool.subcommands.end());

    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_HasSubCommand_0500 end";
}

/**
 * @tc.name: ToolInfo_ParseFromJson_HasSubCommand_0600
 * @tc.desc: Test ToolInfo ParseFromJson without hasSubCommand (default false)
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_ParseFromJson_HasSubCommand_0600, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_HasSubCommand_0600 start";

    nlohmann::json json = R"({
        "name": "ohos-test",
        "version": "1.0.0",
        "description": "Test tool",
        "executablePath": "/bin/test",
        "requirePermissions": ["ohos.permission.INTERNET"],
        "inputSchema": {"type": "object"},
        "outputSchema": {"type": "string"}
    })"_json;

    ToolInfo tool;
    bool result = ToolInfo::ParseFromJson(json, tool);

    EXPECT_TRUE(result);
    EXPECT_FALSE(tool.hasSubCommand);
    EXPECT_TRUE(tool.subcommands.empty());

    GTEST_LOG_(INFO) << "ToolInfo_ParseFromJson_HasSubCommand_0600 end";
}

/**
 * @tc.name: ToolInfo_Validate_2400
 * @tc.desc: Test ToolInfo::Validate with all valid fields
 * @tc.type: FUNC
 */
HWTEST_F(ToolInfoTest, ToolInfo_Validate_2400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ToolInfo_Validate_2400 start";

    ToolInfo tool;
    tool.name = "ohos-full_tool";
    tool.version = "2.0.0";
    tool.description = "Full valid tool";
    tool.executablePath = "/usr/bin/fulltool";
    tool.requirePermissions = {"ohos.permission.INTERNET", "ohos.permission.CAMERA"};
    tool.inputSchema = R"({"type": "object", "properties": {"input": {"type": "string"}}})";
    tool.outputSchema = R"({"type": "array", "items": {"type": "string"}})";
    tool.eventTypes = {"stdout", "stderr", "exit"};
    tool.eventSchemas = R"({"stdout": {"type": "string"}, "exit": {"type": "number"}})";
    tool.hasSubCommand = false;

    EXPECT_TRUE(ToolInfo::Validate(tool));

    GTEST_LOG_(INFO) << "ToolInfo_Validate_2400 end";
}

} // namespace CliTool
} // namespace OHOS
