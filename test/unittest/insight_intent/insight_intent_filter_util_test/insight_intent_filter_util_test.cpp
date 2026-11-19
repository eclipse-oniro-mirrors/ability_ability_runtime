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
using namespace OHOS::CliTool;
using namespace OHOS::AbilityRuntime;

namespace {
ExtractInsightIntentInfo MakeBgUiAbilityIntent(const std::string &bundle, const std::string &module,
    const std::string &intent, const std::string &ability)
{
    ExtractInsightIntentInfo info;
    info.genericInfo.bundleName = bundle;
    info.genericInfo.moduleName = module;
    info.genericInfo.intentName = intent;
    info.genericInfo.decoratorType = INSIGHT_INTENTS_DECORATOR_TYPE_ENTRY;
    info.genericInfo.set<InsightIntentEntryInfo>();
    auto &entry = info.genericInfo.get<InsightIntentEntryInfo>();
    entry.abilityName = ability;
    entry.executeMode = {AppExecFwk::ExecuteMode::UI_ABILITY_BACKGROUND};
    return info;
}

ExtractInsightIntentInfo MakeFgUiAbilityIntent(const std::string &bundle, const std::string &module,
    const std::string &intent, const std::string &ability)
{
    ExtractInsightIntentInfo info;
    info.genericInfo.bundleName = bundle;
    info.genericInfo.moduleName = module;
    info.genericInfo.intentName = intent;
    info.genericInfo.decoratorType = INSIGHT_INTENTS_DECORATOR_TYPE_ENTRY;
    info.genericInfo.set<InsightIntentEntryInfo>();
    auto &entry = info.genericInfo.get<InsightIntentEntryInfo>();
    entry.abilityName = ability;
    entry.executeMode = {AppExecFwk::ExecuteMode::UI_ABILITY_FOREGROUND};
    return info;
}

InsightIntentInfo MakeConfigServiceExtension(const std::string &bundle, const std::string &module,
    const std::string &intent, const std::string &ability)
{
    InsightIntentInfo info;
    info.bundleName = bundle;
    info.moduleName = module;
    info.intentName = intent;
    info.serviceExtensionIntentInfo.abilityName = ability;
    return info;
}

InsightIntentInfo MakeConfigFormOnly(const std::string &bundle, const std::string &module,
    const std::string &intent, const std::string &ability)
{
    InsightIntentInfo info;
    info.bundleName = bundle;
    info.moduleName = module;
    info.intentName = intent;
    info.formIntentInfo.abilityName = ability;
    info.formIntentInfo.formName = "form";
    return info;
}
} // namespace

class InsightIntentFilterUtilTest : public Test {};

HWTEST_F(InsightIntentFilterUtilTest, FilterGeneric_EmptyInput_NoChange, TestSize.Level1)
{
    std::vector<ExtractInsightIntentInfo> items;
    IntentFilterUtil filter;
    filter.FilterGeneric(items);
    EXPECT_TRUE(items.empty());
}

HWTEST_F(InsightIntentFilterUtilTest, FilterGeneric_FgUiAbilityDropped, TestSize.Level1)
{
    std::vector<ExtractInsightIntentInfo> items = {
        MakeFgUiAbilityIntent("bundle", "entry", "intent", "MainAbility"),
    };
    IntentFilterUtil filter;
    filter.FilterGeneric(items);
    EXPECT_TRUE(items.empty());
}

HWTEST_F(InsightIntentFilterUtilTest, FilterGeneric_BgUiAbilityKept, TestSize.Level1)
{
    std::vector<ExtractInsightIntentInfo> items = {
        MakeBgUiAbilityIntent("bundle", "entry", "intent", "BgAbility"),
    };
    IntentFilterUtil filter;
    filter.FilterGeneric(items);
    EXPECT_EQ(items.size(), 1u);
    EXPECT_EQ(items[0].genericInfo.moduleName, "entry");
}

HWTEST_F(InsightIntentFilterUtilTest, FilterConfig_EmptyInput_NoChange, TestSize.Level1)
{
    std::vector<InsightIntentInfo> items;
    IntentFilterUtil filter;
    filter.FilterConfig(items);
    EXPECT_TRUE(items.empty());
}

HWTEST_F(InsightIntentFilterUtilTest, FilterConfig_ServiceExtensionKept, TestSize.Level1)
{
    std::vector<InsightIntentInfo> items = {
        MakeConfigServiceExtension("bundle", "entry", "intent", "ServiceAbility"),
    };
    IntentFilterUtil filter;
    filter.FilterConfig(items);
    EXPECT_EQ(items.size(), 1u);
    EXPECT_EQ(items[0].moduleName, "entry");
}

HWTEST_F(InsightIntentFilterUtilTest, FilterConfig_FormOnlyDropped, TestSize.Level1)
{
    std::vector<InsightIntentInfo> items = {
        MakeConfigFormOnly("bundle", "entry", "intent", "FormAbility"),
    };
    IntentFilterUtil filter;
    filter.FilterConfig(items);
    EXPECT_TRUE(items.empty());
}
