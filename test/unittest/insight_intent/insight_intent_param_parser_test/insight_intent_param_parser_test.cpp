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
#include <string>
#include <vector>

#include "extract_insight_intent_profile.h"
#include "insight_intent_execute_param.h"
#include "insight_intent_param_parser.h"
#include "want_params.h"

using namespace testing;
using namespace OHOS::AAFwk;
using namespace OHOS::AbilityRuntime;

namespace {
ExtractInsightIntentGenericInfo MakeBgUiAbilityCandidate(const std::string &bundle,
    const std::string &module, const std::string &intent, const std::string &ability)
{
    ExtractInsightIntentGenericInfo info;
    info.bundleName = bundle;
    info.moduleName = module;
    info.intentName = intent;
    info.decoratorType = INSIGHT_INTENTS_DECORATOR_TYPE_ENTRY;
    info.set<InsightIntentEntryInfo>();
    auto &entry = info.get<InsightIntentEntryInfo>();
    entry.abilityName = ability;
    entry.executeMode = {AppExecFwk::ExecuteMode::UI_ABILITY_BACKGROUND};
    return info;
}

ExtractInsightIntentGenericInfo MakeFgUiAbilityCandidate(const std::string &bundle,
    const std::string &module, const std::string &intent, const std::string &ability)
{
    ExtractInsightIntentGenericInfo info;
    info.bundleName = bundle;
    info.moduleName = module;
    info.intentName = intent;
    info.decoratorType = INSIGHT_INTENTS_DECORATOR_TYPE_ENTRY;
    info.set<InsightIntentEntryInfo>();
    auto &entry = info.get<InsightIntentEntryInfo>();
    entry.abilityName = ability;
    entry.executeMode = {AppExecFwk::ExecuteMode::UI_ABILITY_FOREGROUND};
    return info;
}
} // namespace

class InsightIntentParamParserTest : public Test {};

HWTEST_F(InsightIntentParamParserTest, Build_EmptyBundleName_ReturnsInvalidValue, TestSize.Level1)
{
    InsightIntentParamParser parser;
    WantParams wantParam;
    InsightIntentParamParser::ParseResult out;
    auto ret = parser.Build("", "intent", wantParam, {}, 0, out);
    EXPECT_NE(ret, 0);
}

HWTEST_F(InsightIntentParamParserTest, Build_EmptyIntentName_ReturnsInvalidValue, TestSize.Level1)
{
    InsightIntentParamParser parser;
    WantParams wantParam;
    InsightIntentParamParser::ParseResult out;
    auto ret = parser.Build("bundle", "", wantParam, {}, 0, out);
    EXPECT_NE(ret, 0);
}

HWTEST_F(InsightIntentParamParserTest, Build_NoCandidate_ReturnsInvalidValue, TestSize.Level1)
{
    InsightIntentParamParser parser;
    WantParams wantParam;
    InsightIntentParamParser::ParseResult out;
    auto ret = parser.Build("bundle", "intent", wantParam, {}, 0, out);
    EXPECT_NE(ret, 0);
}

HWTEST_F(InsightIntentParamParserTest, Build_AllFgCandidates_ReturnsInvalidValue, TestSize.Level1)
{
    std::vector<ExtractInsightIntentGenericInfo> candidates = {
        MakeFgUiAbilityCandidate("bundle", "entry", "intent", "MainAbility"),
    };
    InsightIntentParamParser parser;
    WantParams wantParam;
    InsightIntentParamParser::ParseResult out;
    auto ret = parser.Build("bundle", "intent", wantParam, candidates, 0, out);
    EXPECT_NE(ret, 0);
}

HWTEST_F(InsightIntentParamParserTest, Build_SingleBgUiAbilityCandidate_ReturnsParam, TestSize.Level1)
{
    const std::string expectedModule = "entry";
    const std::string expectedAbility = "BgAbility";
    std::vector<ExtractInsightIntentGenericInfo> candidates = {
        MakeBgUiAbilityCandidate("bundle", expectedModule, "intent", expectedAbility),
    };
    InsightIntentParamParser parser;
    WantParams wantParam;
    InsightIntentParamParser::ParseResult out;
    auto ret = parser.Build("bundle", "intent", wantParam, candidates, 0, out);
    EXPECT_EQ(ret, 0);
    ASSERT_NE(out.param, nullptr);
    EXPECT_EQ(out.param->moduleName_, expectedModule);
    EXPECT_EQ(out.param->abilityName_, expectedAbility);
    EXPECT_FALSE(out.ignoreAbilityName);
    EXPECT_FALSE(out.openLinkExecuteFlag);
}
