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
#include "function_call_convert.h"
#include "insight_intent_profile.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::CliTool;
using namespace OHOS::AbilityRuntime;

namespace {
InsightIntentInfo MakeConfigIntent(const std::string &bundle, const std::string &module,
    const std::string &intent)
{
    InsightIntentInfo info;
    info.bundleName = bundle;
    info.moduleName = module;
    info.intentName = intent;
    info.uiAbilityIntentInfo.abilityName = "MainAbility";
    return info;
}

ExtractInsightIntentInfo MakeLinkIntent(const std::string &bundle, const std::string &module,
    const std::string &intent)
{
    ExtractInsightIntentInfo info;
    info.genericInfo.bundleName = bundle;
    info.genericInfo.moduleName = module;
    info.genericInfo.intentName = intent;
    info.genericInfo.set<InsightIntentLinkInfo>();
    auto &link = info.genericInfo.get<InsightIntentLinkInfo>();
    link.uri = "example://test";
    link.parameters = R"({"type":"object"})";
    return info;
}

void SetLinkSchema(ExtractInsightIntentInfo &info, const std::string &schemaJson)
{
    auto &link = info.genericInfo.get<InsightIntentLinkInfo>();
    link.uri = "example://test";
    link.parameters = schemaJson;
}

std::string MakeDeepNestedSchema(int depth)
{
    std::string schema;
    for (int i = 0; i < depth; i++) {
        schema += R"({"nested":)";
    }
    schema += "null";
    for (int i = 0; i < depth; i++) {
        schema += "}";
    }
    return schema;
}
}

class FunctionCallConvertTest : public Test {};

HWTEST_F(FunctionCallConvertTest, ConvertFromConfigIntent_EmptyInput_NoChange, TestSize.Level1)
{
    std::vector<InsightIntentInfo> configInfos;
    std::vector<FunctionInfo> functions;
    EXPECT_TRUE(ConvertFromConfigIntent(configInfos, functions));
    EXPECT_TRUE(functions.empty());
}

HWTEST_F(FunctionCallConvertTest, ConvertFromConfigIntent_EmptyIntentName_Skipped, TestSize.Level1)
{
    std::vector<InsightIntentInfo> configInfos;
    configInfos.push_back(MakeConfigIntent("com.test", "module", ""));
    std::vector<FunctionInfo> functions;
    EXPECT_TRUE(ConvertFromConfigIntent(configInfos, functions));
    EXPECT_TRUE(functions.empty());
}

HWTEST_F(FunctionCallConvertTest, ConvertFromConfigIntent_ValidInput_FunctionInfoBuilt, TestSize.Level1)
{
    std::vector<InsightIntentInfo> configInfos;
    auto info = MakeConfigIntent("com.test.demo", "entry", "QueryWeather");
    info.inputParams = {"city"};
    info.outputParams = {"temperature"};
    info.displayDescription = "query weather";
    configInfos.push_back(info);

    std::vector<FunctionInfo> functions;
    EXPECT_TRUE(ConvertFromConfigIntent(configInfos, functions));
    ASSERT_EQ(functions.size(), 1u);
    EXPECT_EQ(functions[0].functionName, "QueryWeather");
    EXPECT_EQ(functions[0].functionNamespace, "com.test.demo");
    EXPECT_EQ(functions[0].description, "query weather");
    EXPECT_EQ(functions[0].functionType, FunctionType::INTENT_FUNCTION);
    EXPECT_NE(functions[0].inputSchema.find("city"), std::string::npos);
    EXPECT_NE(functions[0].outputSchema.find("temperature"), std::string::npos);
}

HWTEST_F(FunctionCallConvertTest, ConvertFromExtractIntentInfo_EmptyInput_NoChange, TestSize.Level1)
{
    std::vector<ExtractInsightIntentInfo> intentInfos;
    std::vector<FunctionInfo> functions;
    EXPECT_TRUE(ConvertFromExtractIntentInfo(intentInfos, functions));
    EXPECT_TRUE(functions.empty());
}

HWTEST_F(FunctionCallConvertTest, ConvertFromExtractIntentInfo_EmptyIntentName_Skipped, TestSize.Level1)
{
    std::vector<ExtractInsightIntentInfo> intentInfos;
    intentInfos.push_back(MakeLinkIntent("com.test", "module", ""));
    std::vector<FunctionInfo> functions;
    EXPECT_TRUE(ConvertFromExtractIntentInfo(intentInfos, functions));
    EXPECT_TRUE(functions.empty());
}

HWTEST_F(FunctionCallConvertTest, ConvertFromExtractIntentInfo_LinkType_FunctionInfoBuilt, TestSize.Level1)
{
    std::vector<ExtractInsightIntentInfo> intentInfos;
    auto info = MakeLinkIntent("com.test.demo", "entry", "OpenLink");
    info.displayDescription = "open link";
    info.result = R"({"type":"object"})";
    intentInfos.push_back(info);

    std::vector<FunctionInfo> functions;
    EXPECT_TRUE(ConvertFromExtractIntentInfo(intentInfos, functions));
    ASSERT_EQ(functions.size(), 1u);
    EXPECT_EQ(functions[0].functionName, "OpenLink");
    EXPECT_EQ(functions[0].functionNamespace, "com.test.demo");
    EXPECT_EQ(functions[0].description, "open link");
    EXPECT_EQ(functions[0].functionType, FunctionType::INTENT_FUNCTION);
    EXPECT_FALSE(functions[0].inputSchema.empty());
}

HWTEST_F(FunctionCallConvertTest, BatchUpdateInsightIntentFunctions_EmptyBundleName_ReturnsFalse, TestSize.Level1)
{
    std::vector<ExtractInsightIntentInfo> intentInfos;
    std::vector<InsightIntentInfo> configInfos;
    int32_t successCount = -1;
    EXPECT_FALSE(BatchUpdateInsightIntentFunctions(intentInfos, configInfos, "", 0, successCount));
    EXPECT_EQ(successCount, 0);
}

HWTEST_F(FunctionCallConvertTest, ConvertFromExtractIntentInfo_PropertiesIsString_NoCrash, TestSize.Level1)
{
    std::vector<ExtractInsightIntentInfo> intentInfos;
    auto info = MakeLinkIntent("com.test.demo", "entry", "OpenLink");
    SetLinkSchema(info, R"({"type":"object","properties":"oops"})");
    intentInfos.push_back(info);
    std::vector<FunctionInfo> functions;
    EXPECT_TRUE(ConvertFromExtractIntentInfo(intentInfos, functions));
    ASSERT_EQ(functions.size(), 1u);
    EXPECT_NE(functions[0].inputSchema.find("ohos.insightIntent.options"), std::string::npos);
}

HWTEST_F(FunctionCallConvertTest, ConvertFromExtractIntentInfo_PropertiesIsNumber_NoCrash, TestSize.Level1)
{
    std::vector<ExtractInsightIntentInfo> intentInfos;
    auto info = MakeLinkIntent("com.test.demo", "entry", "OpenLink");
    SetLinkSchema(info, R"({"type":"object","properties":123})");
    intentInfos.push_back(info);
    std::vector<FunctionInfo> functions;
    EXPECT_TRUE(ConvertFromExtractIntentInfo(intentInfos, functions));
    ASSERT_EQ(functions.size(), 1u);
    EXPECT_NE(functions[0].inputSchema.find("ohos.insightIntent.options"), std::string::npos);
}

HWTEST_F(FunctionCallConvertTest, ConvertFromExtractIntentInfo_PropertiesIsBool_NoCrash, TestSize.Level1)
{
    std::vector<ExtractInsightIntentInfo> intentInfos;
    auto info = MakeLinkIntent("com.test.demo", "entry", "OpenLink");
    SetLinkSchema(info, R"({"type":"object","properties":true})");
    intentInfos.push_back(info);
    std::vector<FunctionInfo> functions;
    EXPECT_TRUE(ConvertFromExtractIntentInfo(intentInfos, functions));
    ASSERT_EQ(functions.size(), 1u);
    EXPECT_NE(functions[0].inputSchema.find("ohos.insightIntent.options"), std::string::npos);
}

HWTEST_F(FunctionCallConvertTest, ConvertFromExtractIntentInfo_PropertiesIsArray_NoCrash, TestSize.Level1)
{
    std::vector<ExtractInsightIntentInfo> intentInfos;
    auto info = MakeLinkIntent("com.test.demo", "entry", "OpenLink");
    SetLinkSchema(info, R"({"type":"object","properties":["a","b"]})");
    intentInfos.push_back(info);
    std::vector<FunctionInfo> functions;
    EXPECT_TRUE(ConvertFromExtractIntentInfo(intentInfos, functions));
    ASSERT_EQ(functions.size(), 1u);
    EXPECT_NE(functions[0].inputSchema.find("ohos.insightIntent.options"), std::string::npos);
}

HWTEST_F(FunctionCallConvertTest, ConvertFromExtractIntentInfo_InvalidJson_NoCrash, TestSize.Level1)
{
    std::vector<ExtractInsightIntentInfo> intentInfos;
    auto info = MakeLinkIntent("com.test.demo", "entry", "OpenLink");
    SetLinkSchema(info, R"({invalid json)");
    intentInfos.push_back(info);
    std::vector<FunctionInfo> functions;
    EXPECT_TRUE(ConvertFromExtractIntentInfo(intentInfos, functions));
    ASSERT_EQ(functions.size(), 1u);
    EXPECT_NE(functions[0].inputSchema.find("ohos.insightIntent.options"), std::string::npos);
}

HWTEST_F(FunctionCallConvertTest, ConvertFromExtractIntentInfo_SchemaNotObject_NoCrash, TestSize.Level1)
{
    std::vector<ExtractInsightIntentInfo> intentInfos;
    auto info = MakeLinkIntent("com.test.demo", "entry", "OpenLink");
    SetLinkSchema(info, R"([1,2,3])");
    intentInfos.push_back(info);
    std::vector<FunctionInfo> functions;
    EXPECT_TRUE(ConvertFromExtractIntentInfo(intentInfos, functions));
    ASSERT_EQ(functions.size(), 1u);
    EXPECT_NE(functions[0].inputSchema.find("ohos.insightIntent.options"), std::string::npos);
}

HWTEST_F(FunctionCallConvertTest, ConvertFromExtractIntentInfo_OverDepthLimit_ResetToEmpty, TestSize.Level1)
{
    std::vector<ExtractInsightIntentInfo> intentInfos;
    auto info = MakeLinkIntent("com.test.demo", "entry", "OpenLink");
    SetLinkSchema(info, MakeDeepNestedSchema(101));
    intentInfos.push_back(info);
    std::vector<FunctionInfo> functions;
    EXPECT_TRUE(ConvertFromExtractIntentInfo(intentInfos, functions));
    ASSERT_EQ(functions.size(), 1u);
    EXPECT_NE(functions[0].inputSchema.find("ohos.insightIntent.options"), std::string::npos);
}
