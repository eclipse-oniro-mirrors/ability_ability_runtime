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

#include "ability_manager_errors.h"
#include "agent_card_utils.h"
#include "mock_my_flag.h"

using namespace OHOS;
using namespace OHOS::AgentRuntime;
using namespace testing::ext;

namespace OHOS {
namespace AgentRuntime {
namespace {
AgentCard BuildCard(const std::string &agentId, const std::string &version,
    AgentCardType type = AgentCardType::APP)
{
    AgentCard card;
    card.agentId = agentId;
    card.type = type;
    card.name = agentId;
    card.description = "desc";
    card.version = version;
    card.category = "productivity";
    card.defaultInputModes = {"text/plain"};
    card.defaultOutputModes = {"text/plain"};
    auto skill = std::make_shared<AgentSkill>();
    skill->id = agentId + "_skill";
    skill->name = "skill";
    skill->description = "skill desc";
    skill->tags = {"tag"};
    card.skills = {skill};
    card.iconUrl = "http://example.com/icon.png";
    card.appInfo = std::make_shared<AgentAppInfo>();
    card.appInfo->bundleName = "test.bundle";
    card.appInfo->moduleName = "testModule";
    card.appInfo->abilityName = "TestAgent";
    return card;
}

AppExecFwk::ExtensionAbilityInfo BuildExtensionInfo(AppExecFwk::ExtensionAbilityType type)
{
    AppExecFwk::ExtensionAbilityInfo info;
    info.bundleName = "test.bundle";
    info.moduleName = "testModule";
    info.name = "TestAgent";
    info.type = type;
    return info;
}

AppExecFwk::HapModuleInfo BuildHapModuleInfo(AppExecFwk::ExtensionAbilityType type)
{
    AppExecFwk::HapModuleInfo hapModuleInfo;
    hapModuleInfo.moduleName = "testModule";
    hapModuleInfo.extensionInfos.push_back(BuildExtensionInfo(type));
    return hapModuleInfo;
}
}

class AgentCardUtilsTest : public testing::Test {
public:
    void SetUp() override
    {
        MyFlag::retGetBundleInfo = true;
        MyFlag::retGetResConfigFile = true;
        MyFlag::retFromJson = true;
        MyFlag::mockApplicationInfoIsSystemApp = true;
        MyFlag::mockExtensionInfos.clear();
        MyFlag::mockHapModuleInfos.clear();
        MyFlag::mockProfileInfos.clear();
        MyFlag::mockProfileInfoContent.clear();
    }
};

HWTEST_F(AgentCardUtilsTest, ValidateBundleAbility_001, TestSize.Level1)
{
    MyFlag::retGetBundleInfo = false;
    EXPECT_EQ(AgentCardUtils::ValidateBundleAbility("test.bundle", "TestAgent", 100), AAFwk::ERR_BUNDLE_NOT_EXIST);
}

HWTEST_F(AgentCardUtilsTest, ValidateBundleAbility_002, TestSize.Level1)
{
    EXPECT_EQ(AgentCardUtils::ValidateBundleAbility("test.bundle", "TestAgent", 100), AAFwk::RESOLVE_ABILITY_ERR);
}

HWTEST_F(AgentCardUtilsTest, ValidateBundleAbility_003, TestSize.Level1)
{
    MyFlag::mockHapModuleInfos.push_back(BuildHapModuleInfo(AppExecFwk::ExtensionAbilityType::SERVICE));
    EXPECT_EQ(AgentCardUtils::ValidateBundleAbility("test.bundle", "TestAgent", 100), AAFwk::ERR_WRONG_INTERFACE_CALL);
}

HWTEST_F(AgentCardUtilsTest, ValidateBundleAbility_004, TestSize.Level1)
{
    MyFlag::mockHapModuleInfos.push_back(BuildHapModuleInfo(AppExecFwk::ExtensionAbilityType::AGENT));
    EXPECT_EQ(AgentCardUtils::ValidateBundleAbility("test.bundle", "TestAgent", 100), ERR_OK);
}

HWTEST_F(AgentCardUtilsTest, HasRequiredRegisterFields_001, TestSize.Level1)
{
    AgentCard card;
    EXPECT_FALSE(AgentCardUtils::HasRequiredRegisterFields(card));
    EXPECT_TRUE(AgentCardUtils::HasRequiredRegisterFields(BuildCard("testAgent", "1.0.0")));
    auto invalidIconCard = BuildCard("invalidIcon", "1.0.0");
    invalidIconCard.iconUrl.clear();
    EXPECT_FALSE(AgentCardUtils::HasRequiredRegisterFields(invalidIconCard));
}

HWTEST_F(AgentCardUtilsTest, ShouldValidateAppInfo_001, TestSize.Level1)
{
    EXPECT_TRUE(AgentCardUtils::ShouldValidateAppInfo(BuildCard("app", "1.0.0", AgentCardType::APP)));
    EXPECT_TRUE(AgentCardUtils::ShouldValidateAppInfo(BuildCard("lowCode", "1.0.0", AgentCardType::LOW_CODE)));
    EXPECT_TRUE(AgentCardUtils::ShouldValidateAppInfo(BuildCard("atomic", "1.0.0", AgentCardType::ATOMIC_SERVICE)));
}

HWTEST_F(AgentCardUtilsTest, ShouldValidateBundleAbility_001, TestSize.Level1)
{
    EXPECT_TRUE(AgentCardUtils::ShouldValidateBundleAbility(BuildCard("app", "1.0.0", AgentCardType::APP), 100));
    EXPECT_TRUE(AgentCardUtils::ShouldValidateBundleAbility(
        BuildCard("lowCode", "1.0.0", AgentCardType::LOW_CODE), 100));
    EXPECT_TRUE(AgentCardUtils::ShouldValidateBundleAbility(
        BuildCard("atomic", "1.0.0", AgentCardType::ATOMIC_SERVICE), 100));

    MyFlag::retGetBundleInfo = false;
    EXPECT_FALSE(AgentCardUtils::ShouldValidateBundleAbility(
        BuildCard("atomic", "1.0.0", AgentCardType::ATOMIC_SERVICE), 100));
}

HWTEST_F(AgentCardUtilsTest, ValidateSystemAppRequirement_001, TestSize.Level1)
{
    EXPECT_EQ(AgentCardUtils::ValidateSystemAppRequirement(
        BuildCard("app", "1.0.0", AgentCardType::APP), 100), ERR_OK);
}

HWTEST_F(AgentCardUtilsTest, ValidateSystemAppRequirement_002, TestSize.Level1)
{
    MyFlag::retGetBundleInfo = false;
    EXPECT_EQ(AgentCardUtils::ValidateSystemAppRequirement(
        BuildCard("lowCode", "1.0.0", AgentCardType::LOW_CODE), 100), AAFwk::ERR_BUNDLE_NOT_EXIST);
}

HWTEST_F(AgentCardUtilsTest, ValidateSystemAppRequirement_003, TestSize.Level1)
{
    MyFlag::mockApplicationInfoIsSystemApp = false;
    EXPECT_EQ(AgentCardUtils::ValidateSystemAppRequirement(
        BuildCard("lowCode", "1.0.0", AgentCardType::LOW_CODE), 100), AAFwk::ERR_NOT_SYSTEM_APP);
}

HWTEST_F(AgentCardUtilsTest, ValidateSystemAppRequirement_004, TestSize.Level1)
{
    EXPECT_EQ(AgentCardUtils::ValidateSystemAppRequirement(
        BuildCard("lowCode", "1.0.0", AgentCardType::LOW_CODE), 100), ERR_OK);
}

HWTEST_F(AgentCardUtilsTest, IsCardOwnedByAbility_001, TestSize.Level1)
{
    auto card = BuildCard("testAgent", "1.0.0");
    EXPECT_TRUE(AgentCardUtils::IsCardOwnedByAbility(card, "test.bundle", "TestAgent"));
    EXPECT_FALSE(AgentCardUtils::IsCardOwnedByAbility(card, "other.bundle", "TestAgent"));
}

HWTEST_F(AgentCardUtilsTest, ApplyDeviceTypes_001, TestSize.Level1)
{
    auto card = BuildCard("testAgent", "1.0.0");
    card.appInfo->deviceTypes = {"phone", "watch"};
    AgentCardUtils::ApplyDeviceTypes({"phone", "tablet"}, card);
    ASSERT_EQ(card.appInfo->deviceTypes.size(), 1);
    EXPECT_EQ(card.appInfo->deviceTypes[0], "phone");
}

HWTEST_F(AgentCardUtilsTest, ApplyDeviceTypes_002, TestSize.Level1)
{
    auto card = BuildCard("testAgent", "1.0.0");
    card.appInfo->deviceTypes.clear();
    AgentCardUtils::ApplyDeviceTypes({"phone", "tablet"}, card);
    ASSERT_EQ(card.appInfo->deviceTypes.size(), 2);
}

HWTEST_F(AgentCardUtilsTest, ApplyDeviceTypes_003, TestSize.Level1)
{
    AgentCard card;
    card.appInfo = nullptr;

    AgentCardUtils::ApplyDeviceTypes({"phone", "tablet"}, card);

    ASSERT_NE(card.appInfo, nullptr);
    ASSERT_EQ(card.appInfo->deviceTypes.size(), 2);
    EXPECT_EQ(card.appInfo->deviceTypes[0], "phone");
    EXPECT_EQ(card.appInfo->deviceTypes[1], "tablet");
}

HWTEST_F(AgentCardUtilsTest, ShouldKeepStoredCard_001, TestSize.Level1)
{
    EXPECT_TRUE(AgentCardUtils::ShouldKeepStoredCard(
        BuildCard("testAgent", "invalid-version"), BuildCard("testAgent", "1.0.0")));
    EXPECT_TRUE(AgentCardUtils::ShouldKeepStoredCard(
        BuildCard("testAgent", "1.0.0"), BuildCard("testAgent", "invalid-version")));
    EXPECT_TRUE(AgentCardUtils::ShouldKeepStoredCard(
        BuildCard("testAgent", "1.0.0"), BuildCard("testAgent", "2.0.0")));
    EXPECT_FALSE(AgentCardUtils::ShouldKeepStoredCard(
        BuildCard("testAgent", "2.0.0"), BuildCard("testAgent", "1.0.0")));
}

HWTEST_F(AgentCardUtilsTest, FindHapDeviceTypes_001, TestSize.Level1)
{
    AppExecFwk::BundleInfo bundleInfo;
    AppExecFwk::HapModuleInfo hapModuleInfo;
    hapModuleInfo.moduleName = "testModule";
    hapModuleInfo.deviceTypes = {"phone", "tablet"};
    bundleInfo.hapModuleInfos.push_back(hapModuleInfo);

    auto deviceTypes = AgentCardUtils::FindHapDeviceTypes(bundleInfo, "testModule");
    ASSERT_EQ(deviceTypes.size(), 2);
    EXPECT_EQ(deviceTypes[0], "phone");
    EXPECT_EQ(deviceTypes[1], "tablet");
}
} // namespace AgentRuntime
} // namespace OHOS
