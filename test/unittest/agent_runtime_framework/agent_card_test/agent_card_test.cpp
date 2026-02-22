/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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
#include <gmock/gmock.h>

#include "ability_manager_errors.h"
#include "agent_card.h"
#include "securec.h"

using namespace OHOS;
using namespace testing;
using namespace testing::ext;
using namespace OHOS::AAFwk;

namespace OHOS {
namespace AgentRuntime {
constexpr uint32_t MAX_AGENT_CARD_COUNT = 200000;

class AgentCardTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp() override;
    void TearDown() override;
};

void AgentCardTest::SetUpTestCase(void)
{}

void AgentCardTest::TearDownTestCase(void)
{}

void AgentCardTest::SetUp(void)
{}

void AgentCardTest::TearDown(void)
{}

/**
 * @tc.name: ProviderMarshallingTest_001
 * @tc.desc: Test AgentProvider Marshalling method with valid data
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, ProviderMarshallingTest_001, TestSize.Level1)
{
    AgentProvider provider;
    provider.organization = "test1";
    provider.url = "abc";

    Parcel parcelMock;
    bool result = provider.Marshalling(parcelMock);

    EXPECT_TRUE(result);
}

/**
 * @tc.name: ProviderMarshallingTest_002
 * @tc.desc: Test Provider Marshalling method with empty strings
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, ProviderMarshallingTest_002, TestSize.Level1)
{
    AgentProvider provider;
    provider.organization = "";
    provider.url = "";

    Parcel parcelMock;
    bool result = provider.Marshalling(parcelMock);

    EXPECT_TRUE(result);
}

/**
 * @tc.name: ProviderMarshallingTest_003
 * @tc.desc: Test Provider Marshalling method when WriteString fails for organization
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, ProviderMarshallingTest_003, TestSize.Level1)
{
    AgentProvider provider;
    provider.organization = "test";
    provider.url = "test";

    Parcel parcelMock;
    bool result = provider.Marshalling(parcelMock);

    EXPECT_FALSE(result);
}

/**
 * @tc.name: ProviderMarshallingTest_004
 * @tc.desc: Test Provider Marshalling method when WriteString fails for url
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, ProviderMarshallingTest_004, TestSize.Level1)
{
    AgentProvider provider;
    provider.organization = "test";
    provider.url = "test";

    Parcel parcelMock;
    bool result = provider.Marshalling(parcelMock);

    EXPECT_FALSE(result);
}

/**
 * @tc.name: ProviderUnmarshalling_001
 * @tc.desc: ProviderUnmarshalling_001
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, ProviderUnmarshalling_001, TestSize.Level1)
{
    Parcel parcelMock;
    AgentProvider *provider = AgentProvider::Unmarshalling(parcelMock);

    EXPECT_TRUE(provider != nullptr);
}

/**
 * @tc.name: CapabilitiesMarshalling_001
 * @tc.desc: CapabilitiesMarshalling_001
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, CapabilitiesMarshalling_001, TestSize.Level1)
{
    AgentCapabilities capabilities;
    capabilities.streaming = true;
    capabilities.pushNotifications = true;
    capabilities.stateTransitionHistory = true;

    Parcel parcelMock;
    bool result = capabilities.Marshalling(parcelMock);

    EXPECT_TRUE(result);
}

/**
 * @tc.name: CapabilitiesMarshalling_002
 * @tc.desc: CapabilitiesMarshalling_002
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, CapabilitiesMarshalling_002, TestSize.Level1)
{
    AgentCapabilities capabilities;
    capabilities.streaming = false;
    capabilities.pushNotifications = true;
    capabilities.stateTransitionHistory = true;

    Parcel parcelMock;
    bool result = capabilities.Marshalling(parcelMock);

    EXPECT_FALSE(result);
}

/**
 * @tc.name: CapabilitiesMarshalling_003
 * @tc.desc: CapabilitiesMarshalling_003
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, CapabilitiesMarshalling_003, TestSize.Level1)
{
    AgentCapabilities capabilities;
    capabilities.streaming = true;
    capabilities.pushNotifications = false;
    capabilities.stateTransitionHistory = true;

    Parcel parcelMock;
    bool result = capabilities.Marshalling(parcelMock);

    EXPECT_FALSE(result);
}

/**
 * @tc.name: CapabilitiesMarshalling_004
 * @tc.desc: CapabilitiesMarshalling_004
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, CapabilitiesMarshalling_004, TestSize.Level1)
{
    AgentCapabilities capabilities;
    capabilities.streaming = true;
    capabilities.pushNotifications = true;
    capabilities.stateTransitionHistory = false;

    Parcel parcelMock;
    bool result = capabilities.Marshalling(parcelMock);

    EXPECT_FALSE(result);
}

/**
 * @tc.name: CapabilitiesUnmarshalling_001
 * @tc.desc: CapabilitiesUnmarshalling_001
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, CapabilitiesUnmarshalling_001, TestSize.Level1)
{
    Parcel parcelMock;
    AgentCapabilities *capabilities = AgentCapabilities::Unmarshalling(parcelMock);

    EXPECT_TRUE(capabilities != nullptr);
}

/**
 * @tc.name: SkillMarshalling_001
 * @tc.desc: SkillMarshalling_001
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, SkillMarshalling_001, TestSize.Level1)
{
    AgentSkill skill;
    skill.id = "test1";
    skill.name = "test1";
    skill.description = "test1";
    std::vector<std::string> tags;
    tags.push_back("test");
    skill.tags = tags;
    std::vector<std::string> examples;
    examples.push_back("test");
    skill.examples = examples;
    std::vector<std::string> inputModes;
    inputModes.push_back("test");
    skill.inputModes = inputModes;
    std::vector<std::string> outputModes;
    outputModes.push_back("test");
    skill.outputModes = outputModes;

    Parcel parcelMock;
    bool result = skill.Marshalling(parcelMock);

    EXPECT_TRUE(result);
}

/**
 * @tc.name: SkillMarshalling_002
 * @tc.desc: SkillMarshalling_002
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, SkillMarshalling_002, TestSize.Level1)
{
    AgentSkill skill;
    skill.id = "test";
    skill.name = "test1";
    skill.description = "test1";
    std::vector<std::string> tags;
    tags.push_back("test");
    skill.tags = tags;
    std::vector<std::string> examples;
    examples.push_back("test");
    skill.examples = examples;
    std::vector<std::string> inputModes;
    inputModes.push_back("test");
    skill.inputModes = inputModes;
    std::vector<std::string> outputModes;
    outputModes.push_back("test");
    skill.outputModes = outputModes;

    Parcel parcelMock;
    bool result = skill.Marshalling(parcelMock);

    EXPECT_FALSE(result);
}

/**
 * @tc.name: SkillMarshalling_003
 * @tc.desc: SkillMarshalling_003
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, SkillMarshalling_003, TestSize.Level1)
{
    AgentSkill skill;
    skill.id = "test1";
    skill.name = "test";
    skill.description = "test1";
    std::vector<std::string> tags;
    tags.push_back("test");
    skill.tags = tags;
    std::vector<std::string> examples;
    examples.push_back("test");
    skill.examples = examples;
    std::vector<std::string> inputModes;
    inputModes.push_back("test");
    skill.inputModes = inputModes;
    std::vector<std::string> outputModes;
    outputModes.push_back("test");
    skill.outputModes = outputModes;

    Parcel parcelMock;
    bool result = skill.Marshalling(parcelMock);

    EXPECT_FALSE(result);
}

/**
 * @tc.name: SkillMarshalling_004
 * @tc.desc: SkillMarshalling_004
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, SkillMarshalling_004, TestSize.Level1)
{
    AgentSkill skill;
    skill.id = "test1";
    skill.name = "test1";
    skill.description = "test";
    std::vector<std::string> tags;
    tags.push_back("test");
    skill.tags = tags;
    std::vector<std::string> examples;
    examples.push_back("test");
    skill.examples = examples;
    std::vector<std::string> inputModes;
    inputModes.push_back("test");
    skill.inputModes = inputModes;
    std::vector<std::string> outputModes;
    outputModes.push_back("test");
    skill.outputModes = outputModes;

    Parcel parcelMock;
    bool result = skill.Marshalling(parcelMock);

    EXPECT_FALSE(result);
}

/**
 * @tc.name: SkillMarshalling_005
 * @tc.desc: SkillMarshalling_005
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, SkillMarshalling_005, TestSize.Level1)
{
    AgentSkill skill;
    skill.id = "test1";
    skill.name = "test1";
    skill.description = "test1";
    std::vector<std::string> tags;
    skill.tags = tags;
    std::vector<std::string> examples;
    examples.push_back("test");
    skill.examples = examples;
    std::vector<std::string> inputModes;
    inputModes.push_back("test");
    skill.inputModes = inputModes;
    std::vector<std::string> outputModes;
    outputModes.push_back("test");
    skill.outputModes = outputModes;

    Parcel parcelMock;
    bool result = skill.Marshalling(parcelMock);

    EXPECT_FALSE(result);
}

/**
 * @tc.name: SkillMarshalling_006
 * @tc.desc: SkillMarshalling_006
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, SkillMarshalling_006, TestSize.Level1)
{
    AgentSkill skill;
    skill.id = "test1";
    skill.name = "test1";
    skill.description = "test1";
    std::vector<std::string> tags;
    tags.push_back("test");
    skill.tags = tags;
    std::vector<std::string> examples;
    skill.examples = examples;
    std::vector<std::string> inputModes;
    inputModes.push_back("test");
    skill.inputModes = inputModes;
    std::vector<std::string> outputModes;
    outputModes.push_back("test");
    skill.outputModes = outputModes;

    Parcel parcelMock;
    bool result = skill.Marshalling(parcelMock);

    EXPECT_FALSE(result);
}

/**
 * @tc.name: SkillMarshalling_007
 * @tc.desc: SkillMarshalling_007
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, SkillMarshalling_007, TestSize.Level1)
{
    AgentSkill skill;
    skill.id = "test1";
    skill.name = "test1";
    skill.description = "test1";
    std::vector<std::string> tags;
    tags.push_back("test");
    skill.tags = tags;
    std::vector<std::string> examples;
    examples.push_back("test");
    skill.examples = examples;
    std::vector<std::string> inputModes;
    skill.inputModes = inputModes;
    std::vector<std::string> outputModes;
    outputModes.push_back("test");
    skill.outputModes = outputModes;

    Parcel parcelMock;
    bool result = skill.Marshalling(parcelMock);

    EXPECT_FALSE(result);
}

/**
 * @tc.name: SkillMarshalling_008
 * @tc.desc: SkillMarshalling_008
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, SkillMarshalling_008, TestSize.Level1)
{
    AgentSkill skill;
    skill.id = "test1";
    skill.name = "test1";
    skill.description = "test1";
    std::vector<std::string> tags;
    tags.push_back("test");
    skill.tags = tags;
    std::vector<std::string> examples;
    examples.push_back("test");
    skill.examples = examples;
    std::vector<std::string> inputModes;
    inputModes.push_back("test");
    skill.inputModes = inputModes;
    std::vector<std::string> outputModes;
    skill.outputModes = outputModes;

    Parcel parcelMock;
    bool result = skill.Marshalling(parcelMock);

    EXPECT_FALSE(result);
}

/**
 * @tc.name: SkillUnmarshalling_001
 * @tc.desc: SkillUnmarshalling_001
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, SkillUnmarshalling_001, TestSize.Level1)
{
    Parcel parcelMock;
    AgentSkill *skill = AgentSkill::Unmarshalling(parcelMock);

    EXPECT_TRUE(skill == nullptr);
}

/**
 * @tc.name: AgentCardReadFromParcel_001
 * @tc.desc: AgentCardReadFromParcel_001
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, AgentCardReadFromParcel_001, TestSize.Level1)
{
    AgentCard agentCard;
    std::vector<std::string> defaultInputModes;
    defaultInputModes.push_back("test");
    agentCard.defaultInputModes = defaultInputModes;
    std::vector<std::string> defaultOutputModes;
    defaultOutputModes.push_back("test");
    agentCard.defaultOutputModes = defaultOutputModes;
    Parcel parcelMock;
    bool result = agentCard.ReadFromParcel(parcelMock);

    EXPECT_TRUE(result);
}

/**
 * @tc.name: AgentCardReadFromParcel_002
 * @tc.desc: AgentCardReadFromParcel_002
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, AgentCardReadFromParcel_002, TestSize.Level1)
{
    AgentCard agentCard;
    std::vector<std::string> defaultInputModes;
    agentCard.defaultInputModes = defaultInputModes;
    std::vector<std::string> defaultOutputModes;
    defaultOutputModes.push_back("test");
    agentCard.defaultOutputModes = defaultOutputModes;
    Parcel parcelMock;
    bool result = agentCard.ReadFromParcel(parcelMock);

    EXPECT_FALSE(result);
}

/**
 * @tc.name: AgentCardReadFromParcel_003
 * @tc.desc: AgentCardReadFromParcel_003
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, AgentCardReadFromParcel_003, TestSize.Level1)
{
    AgentCard agentCard;
    std::vector<std::string> defaultInputModes;
    defaultInputModes.push_back("test");
    agentCard.defaultInputModes = defaultInputModes;
    std::vector<std::string> defaultOutputModes;
    agentCard.defaultOutputModes = defaultOutputModes;
    Parcel parcelMock;
    bool result = agentCard.ReadFromParcel(parcelMock);

    EXPECT_FALSE(result);
}

/**
 * @tc.name: AgentCardMarshalling_001
 * @tc.desc: AgentCardMarshalling_001
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, AgentCardMarshalling_001, TestSize.Level1)
{
    AgentCard agentCard;
    agentCard.agentId = "1";
    agentCard.name = "test1";
    agentCard.description = "test1";
    std::shared_ptr<AgentProvider> provider = std::make_shared<AgentProvider>();
    agentCard.provider = provider;
    agentCard.version = "test1";
    agentCard.documentationUrl = "test1";
    std::shared_ptr<AgentCapabilities> capabilities = std::make_shared<AgentCapabilities>();
    agentCard.capabilities = capabilities;
    std::vector<std::string> defaultInputModes;
    defaultInputModes.push_back("test");
    agentCard.defaultInputModes = defaultInputModes;
    std::vector<std::string> defaultOutputModes;
    defaultOutputModes.push_back("test");
    agentCard.defaultOutputModes = defaultOutputModes;
    std::vector<std::shared_ptr<AgentSkill>> skills;
    skills.push_back(std::make_shared<AgentSkill>());
    agentCard.skills = skills;

    Parcel parcelMock;
    bool result = agentCard.Marshalling(parcelMock);

    EXPECT_TRUE(result);
}

/**
 * @tc.name: AgentCardMarshalling_002
 * @tc.desc: AgentCardMarshalling_002
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, AgentCardMarshalling_002, TestSize.Level1)
{
    AgentCard agentCard;
    agentCard.agentId = "1";
    agentCard.name = "test";
    agentCard.description = "test1";
    std::shared_ptr<AgentProvider> provider = std::make_shared<AgentProvider>();
    agentCard.provider = provider;
    agentCard.version = "test1";
    agentCard.documentationUrl = "test1";
    std::shared_ptr<AgentCapabilities> capabilities = std::make_shared<AgentCapabilities>();
    agentCard.capabilities = capabilities;
    std::vector<std::string> defaultInputModes;
    defaultInputModes.push_back("test");
    agentCard.defaultInputModes = defaultInputModes;
    std::vector<std::string> defaultOutputModes;
    defaultOutputModes.push_back("test");
    agentCard.defaultOutputModes = defaultOutputModes;
    std::vector<std::shared_ptr<AgentSkill>> skills;
    skills.push_back(std::make_shared<AgentSkill>());
    agentCard.skills = skills;

    Parcel parcelMock;
    bool result = agentCard.Marshalling(parcelMock);

    EXPECT_FALSE(result);
}

/**
 * @tc.name: AgentCardMarshalling_003
 * @tc.desc: AgentCardMarshalling_003
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, AgentCardMarshalling_003, TestSize.Level1)
{
    AgentCard agentCard;
    agentCard.agentId = "1";
    agentCard.name = "test1";
    agentCard.description = "test";
    std::shared_ptr<AgentProvider> provider = std::make_shared<AgentProvider>();
    agentCard.provider = provider;
    agentCard.version = "test1";
    agentCard.documentationUrl = "test1";
    std::shared_ptr<AgentCapabilities> capabilities = std::make_shared<AgentCapabilities>();
    agentCard.capabilities = capabilities;
    std::vector<std::string> defaultInputModes;
    defaultInputModes.push_back("test");
    agentCard.defaultInputModes = defaultInputModes;
    std::vector<std::string> defaultOutputModes;
    defaultOutputModes.push_back("test");
    agentCard.defaultOutputModes = defaultOutputModes;
    std::vector<std::shared_ptr<AgentSkill>> skills;
    skills.push_back(std::make_shared<AgentSkill>());
    agentCard.skills = skills;

    Parcel parcelMock;
    bool result = agentCard.Marshalling(parcelMock);

    EXPECT_FALSE(result);
}

/**
 * @tc.name: AgentCardMarshalling_004
 * @tc.desc: AgentCardMarshalling_004
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, AgentCardMarshalling_004, TestSize.Level1)
{
    AgentCard agentCard;
    agentCard.agentId = "1";
    agentCard.name = "test1";
    agentCard.description = "test1";
    std::shared_ptr<AgentProvider> provider = std::make_shared<AgentProvider>();
    agentCard.provider = provider;
    agentCard.version = "test1";
    agentCard.documentationUrl = "test1";
    std::shared_ptr<AgentCapabilities> capabilities = std::make_shared<AgentCapabilities>();
    agentCard.capabilities = capabilities;
    std::vector<std::string> defaultInputModes;
    defaultInputModes.push_back("test");
    agentCard.defaultInputModes = defaultInputModes;
    std::vector<std::string> defaultOutputModes;
    defaultOutputModes.push_back("test");
    agentCard.defaultOutputModes = defaultOutputModes;
    std::vector<std::shared_ptr<AgentSkill>> skills;
    skills.push_back(std::make_shared<AgentSkill>());
    agentCard.skills = skills;

    Parcel parcelMock;
    bool result = agentCard.Marshalling(parcelMock);

    EXPECT_FALSE(result);
}

/**
 * @tc.name: AgentCardMarshalling_005
 * @tc.desc: AgentCardMarshalling_005
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, AgentCardMarshalling_005, TestSize.Level1)
{
    AgentCard agentCard;
    agentCard.agentId = "1";
    agentCard.name = "test1";
    agentCard.description = "test1";
    std::shared_ptr<AgentProvider> provider = nullptr;
    agentCard.provider = provider;
    agentCard.version = "test1";
    agentCard.documentationUrl = "test1";
    std::shared_ptr<AgentCapabilities> capabilities = std::make_shared<AgentCapabilities>();
    agentCard.capabilities = capabilities;
    std::vector<std::string> defaultInputModes;
    defaultInputModes.push_back("test");
    agentCard.defaultInputModes = defaultInputModes;
    std::vector<std::string> defaultOutputModes;
    defaultOutputModes.push_back("test");
    agentCard.defaultOutputModes = defaultOutputModes;
    std::vector<std::shared_ptr<AgentSkill>> skills;
    skills.push_back(std::make_shared<AgentSkill>());
    agentCard.skills = skills;

    Parcel parcelMock;
    bool result = agentCard.Marshalling(parcelMock);

    EXPECT_FALSE(result);
}

/**
 * @tc.name: AgentCardMarshalling_006
 * @tc.desc: AgentCardMarshalling_006
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, AgentCardMarshalling_006, TestSize.Level1)
{
    AgentCard agentCard;
    agentCard.agentId = "1";
    agentCard.name = "test1";
    agentCard.description = "test1";
    std::shared_ptr<AgentProvider> provider = std::make_shared<AgentProvider>();
    agentCard.provider = provider;
    agentCard.version = "test";
    agentCard.documentationUrl = "test1";
    std::shared_ptr<AgentCapabilities> capabilities = std::make_shared<AgentCapabilities>();
    agentCard.capabilities = capabilities;
    std::vector<std::string> defaultInputModes;
    defaultInputModes.push_back("test");
    agentCard.defaultInputModes = defaultInputModes;
    std::vector<std::string> defaultOutputModes;
    defaultOutputModes.push_back("test");
    agentCard.defaultOutputModes = defaultOutputModes;
    std::vector<std::shared_ptr<AgentSkill>> skills;
    skills.push_back(std::make_shared<AgentSkill>());
    agentCard.skills = skills;

    Parcel parcelMock;
    bool result = agentCard.Marshalling(parcelMock);

    EXPECT_FALSE(result);
}

/**
 * @tc.name: AgentCardMarshalling_007
 * @tc.desc: AgentCardMarshalling_007
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, AgentCardMarshalling_007, TestSize.Level1)
{
    AgentCard agentCard;
    agentCard.agentId = "1";
    agentCard.name = "test1";
    agentCard.description = "test1";
    std::shared_ptr<AgentProvider> provider = std::make_shared<AgentProvider>();
    agentCard.provider = provider;
    agentCard.version = "test1";
    agentCard.documentationUrl = "test";
    std::shared_ptr<AgentCapabilities> capabilities = std::make_shared<AgentCapabilities>();
    agentCard.capabilities = capabilities;
    std::vector<std::string> defaultInputModes;
    defaultInputModes.push_back("test");
    agentCard.defaultInputModes = defaultInputModes;
    std::vector<std::string> defaultOutputModes;
    defaultOutputModes.push_back("test");
    agentCard.defaultOutputModes = defaultOutputModes;
    std::vector<std::shared_ptr<AgentSkill>> skills;
    skills.push_back(std::make_shared<AgentSkill>());
    agentCard.skills = skills;

    Parcel parcelMock;
    bool result = agentCard.Marshalling(parcelMock);

    EXPECT_FALSE(result);
}

/**
 * @tc.name: AgentCardMarshalling_008
 * @tc.desc: AgentCardMarshalling_008
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, AgentCardMarshalling_008, TestSize.Level1)
{
    AgentCard agentCard;
    agentCard.agentId = "1";
    agentCard.name = "test1";
    agentCard.description = "test1";
    std::shared_ptr<AgentProvider> provider = std::make_shared<AgentProvider>();
    agentCard.provider = provider;
    agentCard.version = "test1";
    agentCard.documentationUrl = "test1";
    std::shared_ptr<AgentCapabilities> capabilities = nullptr;
    agentCard.capabilities = capabilities;
    std::vector<std::string> defaultInputModes;
    defaultInputModes.push_back("test");
    agentCard.defaultInputModes = defaultInputModes;
    std::vector<std::string> defaultOutputModes;
    defaultOutputModes.push_back("test");
    agentCard.defaultOutputModes = defaultOutputModes;
    std::vector<std::shared_ptr<AgentSkill>> skills;
    skills.push_back(std::make_shared<AgentSkill>());
    agentCard.skills = skills;

    Parcel parcelMock;
    bool result = agentCard.Marshalling(parcelMock);

    EXPECT_FALSE(result);
}

/**
 * @tc.name: AgentCardMarshalling_009
 * @tc.desc: AgentCardMarshalling_009
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, AgentCardMarshalling_009, TestSize.Level1)
{
    AgentCard agentCard;
    agentCard.agentId = "1";
    agentCard.name = "test1";
    agentCard.description = "test1";
    std::shared_ptr<AgentProvider> provider = std::make_shared<AgentProvider>();
    agentCard.provider = provider;
    agentCard.version = "test1";
    agentCard.documentationUrl = "test1";
    std::shared_ptr<AgentCapabilities> capabilities = std::make_shared<AgentCapabilities>();
    agentCard.capabilities = capabilities;
    std::vector<std::string> defaultInputModes;
    defaultInputModes.push_back("test");
    agentCard.defaultInputModes = defaultInputModes;
    std::vector<std::string> defaultOutputModes;
    defaultOutputModes.push_back("test");
    agentCard.defaultOutputModes = defaultOutputModes;
    std::vector<std::shared_ptr<AgentSkill>> skills;
    skills.push_back(std::make_shared<AgentSkill>());
    agentCard.skills = skills;

    Parcel parcelMock;
    bool result = agentCard.Marshalling(parcelMock);

    EXPECT_FALSE(result);
}

/**
 * @tc.name: AgentCardMarshalling_010
 * @tc.desc: AgentCardMarshalling_010
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, AgentCardMarshalling_010, TestSize.Level1)
{
    AgentCard agentCard;
    agentCard.agentId = "1";
    agentCard.name = "test1";
    agentCard.description = "test1";
    std::shared_ptr<AgentProvider> provider = std::make_shared<AgentProvider>();
    agentCard.provider = provider;
    agentCard.version = "test1";
    agentCard.documentationUrl = "test1";
    std::shared_ptr<AgentCapabilities> capabilities = std::make_shared<AgentCapabilities>();
    agentCard.capabilities = capabilities;
    std::vector<std::string> defaultInputModes;
    agentCard.defaultInputModes = defaultInputModes;
    std::vector<std::string> defaultOutputModes;
    defaultOutputModes.push_back("test");
    agentCard.defaultOutputModes = defaultOutputModes;
    std::vector<std::shared_ptr<AgentSkill>> skills;
    skills.push_back(std::make_shared<AgentSkill>());
    agentCard.skills = skills;

    Parcel parcelMock;
    bool result = agentCard.Marshalling(parcelMock);

    EXPECT_FALSE(result);
}

/**
 * @tc.name: AgentCardMarshalling_011
 * @tc.desc: AgentCardMarshalling_011
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, AgentCardMarshalling_011, TestSize.Level1)
{
    AgentCard agentCard;
    agentCard.agentId = "1";
    agentCard.name = "test1";
    agentCard.description = "test1";
    std::shared_ptr<AgentProvider> provider = std::make_shared<AgentProvider>();
    agentCard.provider = provider;
    agentCard.version = "test1";
    agentCard.documentationUrl = "test1";
    std::shared_ptr<AgentCapabilities> capabilities = std::make_shared<AgentCapabilities>();
    agentCard.capabilities = capabilities;
    std::vector<std::string> defaultInputModes;
    defaultInputModes.push_back("test");
    agentCard.defaultInputModes = defaultInputModes;
    std::vector<std::string> defaultOutputModes;
    agentCard.defaultOutputModes = defaultOutputModes;
    std::vector<std::shared_ptr<AgentSkill>> skills;
    skills.push_back(std::make_shared<AgentSkill>());
    agentCard.skills = skills;

    Parcel parcelMock;
    bool result = agentCard.Marshalling(parcelMock);

    EXPECT_FALSE(result);
}

/**
 * @tc.name: AgentCardMarshalling_012
 * @tc.desc: AgentCardMarshalling_012
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, AgentCardMarshalling_012, TestSize.Level1)
{
    AgentCard agentCard;
    agentCard.agentId = "1";
    agentCard.name = "test1";
    agentCard.description = "test1";
    std::shared_ptr<AgentProvider> provider = std::make_shared<AgentProvider>();
    agentCard.provider = provider;
    agentCard.version = "test1";
    agentCard.documentationUrl = "test1";
    std::shared_ptr<AgentCapabilities> capabilities = std::make_shared<AgentCapabilities>();
    agentCard.capabilities = capabilities;
    std::vector<std::string> defaultInputModes;
    defaultInputModes.push_back("test");
    agentCard.defaultInputModes = defaultInputModes;
    std::vector<std::string> defaultOutputModes;
    defaultOutputModes.push_back("test");
    agentCard.defaultOutputModes = defaultOutputModes;
    std::vector<std::shared_ptr<AgentSkill>> skills;
    agentCard.skills = skills;

    Parcel parcelMock;
    bool result = agentCard.Marshalling(parcelMock);

    EXPECT_FALSE(result);
}

/**
 * @tc.name: AgentCardUnmarshalling_001
 * @tc.desc: AgentCardUnmarshalling_001
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, AgentCardUnmarshalling_001, TestSize.Level1)
{
    Parcel parcelMock;
    AgentCard *agentCard = AgentCard::Unmarshalling(parcelMock);

    EXPECT_TRUE(agentCard == nullptr);
}

/**
 * @tc.name: AgentCardToJson_001
 * @tc.desc: AgentCardToJson_001
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, AgentCardToJson_001, TestSize.Level1)
{
    AgentCard agentCard;
    std::shared_ptr<AgentProvider> provider = std::make_shared<AgentProvider>();
    agentCard.provider = provider;
    std::shared_ptr<AgentCapabilities> capabilities = std::make_shared<AgentCapabilities>();
    agentCard.capabilities = capabilities;
    std::vector<std::shared_ptr<AgentSkill>> skills;
    skills.push_back(std::make_shared<AgentSkill>());
    agentCard.skills = skills;
    nlohmann::json jsonObject = agentCard.ToJson();
    EXPECT_TRUE(jsonObject.contains("provider"));

    agentCard.provider = nullptr;
    jsonObject = agentCard.ToJson();
    EXPECT_FALSE(jsonObject.contains("provider"));

    agentCard.capabilities = nullptr;
    jsonObject = agentCard.ToJson();
    EXPECT_FALSE(jsonObject.contains("capabilities"));

    std::vector<std::shared_ptr<AgentSkill>> skills1;
    agentCard.skills = skills1;
    jsonObject = agentCard.ToJson();
    EXPECT_FALSE(jsonObject.contains("skills"));
}

/**
 * @tc.name: AgentCardFromJson_001
 * @tc.desc: AgentCardFromJson_001
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, AgentCardFromJson_001, TestSize.Level1)
{
    AgentProvider provider;
    AgentCapabilities capabilities;
    std::vector<std::string> defaultInputModes;
    std::vector<std::string> defaultOutputModes;
    AgentSkill skill;
    nlohmann::json jsonArray = { skill.ToJson() };
    nlohmann::json jsonObject = nlohmann::json {
        { "agentId", "1" },
        { "name", "test" },
        { "description", "test" },
        { "version", "1.0" },
        { "documentationUrl", "http://example.com/docs" },
        { "url", "test" },
        { "provider", provider.ToJson() },
        { "capabilities", capabilities.ToJson() },
        { "defaultInputModes", defaultInputModes },
        { "defaultOutputModes", defaultOutputModes },
        { "skills", jsonArray },
    };
    AgentCard agentCard;
    ASSERT_TRUE(AgentCard::FromJson(jsonObject, agentCard));

    EXPECT_TRUE(agentCard.agentId == "1");
    EXPECT_TRUE(agentCard.name == "test");
    EXPECT_TRUE(agentCard.description == "test");
    EXPECT_TRUE(agentCard.version == "1.0");
    EXPECT_TRUE(agentCard.documentationUrl == "http://example.com/docs");
}

/**
 * @tc.name: ProviderToJson_001
 * @tc.desc: ProviderToJson_001
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, ProviderToJson_001, TestSize.Level1)
{
    AgentProvider provider;
    provider.organization = "test";
    nlohmann::json jsonObject = provider.ToJson();

    EXPECT_TRUE(jsonObject["organization"] == "test");
}

/**
 * @tc.name: ProviderFromJson_001
 * @tc.desc: ProviderFromJson_001 - valid input
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, ProviderFromJson_001, TestSize.Level1)
{
    nlohmann::json jsonObject = nlohmann::json {
        { "organization", "test" },
        { "url", "test" },
    };
    AgentProvider provider;
    EXPECT_TRUE(AgentProvider::FromJson(jsonObject, provider));
    EXPECT_TRUE(jsonObject["organization"] == provider.organization);
    EXPECT_TRUE(provider.url == "test");
}

/**
 * @tc.name: ProviderFromJson_002
 * @tc.desc: Test FromJson returns false when organization is missing
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, ProviderFromJson_002, TestSize.Level1)
{
    nlohmann::json jsonObject = nlohmann::json {
        { "url", "test" },
    };
    AgentProvider provider;
    EXPECT_FALSE(AgentProvider::FromJson(jsonObject, provider));
}

/**
 * @tc.name: ProviderFromJson_003
 * @tc.desc: Test FromJson returns false when organization is not a string
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, ProviderFromJson_003, TestSize.Level1)
{
    nlohmann::json jsonObject = nlohmann::json {
        { "organization", 123 },
        { "url", "test" },
    };
    AgentProvider provider;
    EXPECT_FALSE(AgentProvider::FromJson(jsonObject, provider));
}

/**
 * @tc.name: ProviderFromJson_004
 * @tc.desc: Test FromJson returns false when organization length exceeds 128
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, ProviderFromJson_004, TestSize.Level1)
{
    std::string longOrg(129, 'a');
    nlohmann::json jsonObject = nlohmann::json {
        { "organization", longOrg },
        { "url", "test" },
    };
    AgentProvider provider;
    EXPECT_FALSE(AgentProvider::FromJson(jsonObject, provider));
}

/**
 * @tc.name: ProviderFromJson_005
 * @tc.desc: Test FromJson returns false when organization is empty string
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, ProviderFromJson_005, TestSize.Level1)
{
    nlohmann::json jsonObject = nlohmann::json {
        { "organization", "" },
        { "url", "test" },
    };
    AgentProvider provider;
    EXPECT_FALSE(AgentProvider::FromJson(jsonObject, provider));
}

/**
 * @tc.name: ProviderFromJson_006
 * @tc.desc: Test FromJson returns false when url is missing
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, ProviderFromJson_006, TestSize.Level1)
{
    nlohmann::json jsonObject = nlohmann::json {
        { "organization", "test" },
    };
    AgentProvider provider;
    EXPECT_FALSE(AgentProvider::FromJson(jsonObject, provider));
}

/**
 * @tc.name: ProviderFromJson_007
 * @tc.desc: Test FromJson returns false when url is not a string
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, ProviderFromJson_007, TestSize.Level1)
{
    nlohmann::json jsonObject = nlohmann::json {
        { "organization", "test" },
        { "url", 456 },
    };
    AgentProvider provider;
    EXPECT_FALSE(AgentProvider::FromJson(jsonObject, provider));
}

/**
 * @tc.name: ProviderFromJson_008
 * @tc.desc: Test FromJson returns false when url length exceeds 512
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, ProviderFromJson_008, TestSize.Level1)
{
    std::string longUrl(513, 'a');
    nlohmann::json jsonObject = nlohmann::json {
        { "organization", "test" },
        { "url", longUrl },
    };
    AgentProvider provider;
    EXPECT_FALSE(AgentProvider::FromJson(jsonObject, provider));
}

/**
 * @tc.name: ProviderFromJson_009
 * @tc.desc: Test FromJson returns false when url is empty string
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, ProviderFromJson_009, TestSize.Level1)
{
    nlohmann::json jsonObject = nlohmann::json {
        { "organization", "test" },
        { "url", "" },
    };
    AgentProvider provider;
    EXPECT_FALSE(AgentProvider::FromJson(jsonObject, provider));
}

/**
 * @tc.name: ProviderFromJson_010
 * @tc.desc: Test FromJson returns true with valid boundary values
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, ProviderFromJson_010, TestSize.Level1)
{
    std::string validOrg(128, 'a');
    std::string validUrl(512, 'b');
    nlohmann::json jsonObject = nlohmann::json {
        { "organization", validOrg },
        { "url", validUrl },
    };
    AgentProvider provider;
    EXPECT_TRUE(AgentProvider::FromJson(jsonObject, provider));
    EXPECT_EQ(provider.organization.length(), 128);
    EXPECT_EQ(provider.url.length(), 512);
}

/**
 * @tc.name: CapabilitiesToJson_001
 * @tc.desc: CapabilitiesToJson_001
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, CapabilitiesToJson_001, TestSize.Level1)
{
    AgentCapabilities capabilities;
    capabilities.streaming = true;
    nlohmann::json jsonObject = capabilities.ToJson();

    EXPECT_TRUE(jsonObject["streaming"]);
}

/**
 * @tc.name: CapabilitiesFromJson_001
 * @tc.desc: CapabilitiesFromJson_001
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, CapabilitiesFromJson_001, TestSize.Level1)
{
    nlohmann::json jsonObject = nlohmann::json {
        { "streaming", true },
        { "pushNotifications", true },
        { "stateTransitionHistory", true },
    };
    AgentCapabilities capabilities = AgentCapabilities::FromJson(jsonObject);
    EXPECT_TRUE(jsonObject["streaming"]);

    jsonObject = nlohmann::json {};
    capabilities = AgentCapabilities::FromJson(jsonObject);
    EXPECT_TRUE(jsonObject.contains("streaming") == false);
}

/**
 * @tc.name: SkillToJson_001
 * @tc.desc: SkillToJson_001
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, SkillToJson_001, TestSize.Level1)
{
    AgentSkill skill;
    skill.id = "test";
    nlohmann::json jsonObject = skill.ToJson();

    EXPECT_TRUE(jsonObject["id"] == "test");
}

/**
 * @tc.name: SkillFromJson_001
 * @tc.desc: SkillFromJson_001 - valid input
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, SkillFromJson_001, TestSize.Level1)
{
    nlohmann::json jsonObject = nlohmann::json {
        { "id", "test" },
        { "name", "test" },
        { "description", "test" },
        { "tags", nlohmann::json::array({ "test", "test" }) },
        { "examples", nlohmann::json::array({ "test", "test" }) },
        { "inputModes", nlohmann::json::array({ "test", "test" }) },
        { "outputModes", nlohmann::json::array({ "test", "test" }) },
    };
    AgentSkill skill;
    EXPECT_TRUE(AgentSkill::FromJson(jsonObject, skill));
    EXPECT_TRUE(skill.id == "test");
    EXPECT_EQ(skill.tags.size(), 2);
}

/**
 * @tc.name: SkillFromJson_002
 * @tc.desc: Test FromJson returns false when id is missing
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, SkillFromJson_002, TestSize.Level1)
{
    nlohmann::json jsonObject = nlohmann::json {
        { "name", "test" },
        { "description", "test" },
        { "tags", nlohmann::json::array({ "test" }) },
    };
    AgentSkill skill;
    EXPECT_FALSE(AgentSkill::FromJson(jsonObject, skill));
}

/**
 * @tc.name: SkillFromJson_003
 * @tc.desc: Test FromJson returns false when id is not a string
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, SkillFromJson_003, TestSize.Level1)
{
    nlohmann::json jsonObject = nlohmann::json {
        { "id", 123 },
        { "name", "test" },
        { "description", "test" },
        { "tags", nlohmann::json::array({ "test" }) },
    };
    AgentSkill skill;
    EXPECT_FALSE(AgentSkill::FromJson(jsonObject, skill));
}

/**
 * @tc.name: SkillFromJson_004
 * @tc.desc: Test FromJson returns false when id length exceeds 64
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, SkillFromJson_004, TestSize.Level1)
{
    std::string longId(65, 'a');
    nlohmann::json jsonObject = nlohmann::json {
        { "id", longId },
        { "name", "test" },
        { "description", "test" },
        { "tags", nlohmann::json::array({ "test" }) },
    };
    AgentSkill skill;
    EXPECT_FALSE(AgentSkill::FromJson(jsonObject, skill));
}

/**
 * @tc.name: SkillFromJson_005
 * @tc.desc: Test FromJson returns false when id is empty string
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, SkillFromJson_005, TestSize.Level1)
{
    nlohmann::json jsonObject = nlohmann::json {
        { "id", "" },
        { "name", "test" },
        { "description", "test" },
        { "tags", nlohmann::json::array({ "test" }) },
    };
    AgentSkill skill;
    EXPECT_FALSE(AgentSkill::FromJson(jsonObject, skill));
}

/**
 * @tc.name: SkillFromJson_006
 * @tc.desc: Test FromJson returns false when name is missing
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, SkillFromJson_006, TestSize.Level1)
{
    nlohmann::json jsonObject = nlohmann::json {
        { "id", "test" },
        { "description", "test" },
        { "tags", nlohmann::json::array({ "test" }) },
    };
    AgentSkill skill;
    EXPECT_FALSE(AgentSkill::FromJson(jsonObject, skill));
}

/**
 * @tc.name: SkillFromJson_007
 * @tc.desc: Test FromJson returns false when name is not a string
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, SkillFromJson_007, TestSize.Level1)
{
    nlohmann::json jsonObject = nlohmann::json {
        { "id", "test" },
        { "name", 456 },
        { "description", "test" },
        { "tags", nlohmann::json::array({ "test" }) },
    };
    AgentSkill skill;
    EXPECT_FALSE(AgentSkill::FromJson(jsonObject, skill));
}

/**
 * @tc.name: SkillFromJson_008
 * @tc.desc: Test FromJson returns false when name length exceeds 128
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, SkillFromJson_008, TestSize.Level1)
{
    std::string longName(129, 'a');
    nlohmann::json jsonObject = nlohmann::json {
        { "id", "test" },
        { "name", longName },
        { "description", "test" },
        { "tags", nlohmann::json::array({ "test" }) },
    };
    AgentSkill skill;
    EXPECT_FALSE(AgentSkill::FromJson(jsonObject, skill));
}

/**
 * @tc.name: SkillFromJson_009
 * @tc.desc: Test FromJson returns false when name is empty string
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, SkillFromJson_009, TestSize.Level1)
{
    nlohmann::json jsonObject = nlohmann::json {
        { "id", "test" },
        { "name", "" },
        { "description", "test" },
        { "tags", nlohmann::json::array({ "test" }) },
    };
    AgentSkill skill;
    EXPECT_FALSE(AgentSkill::FromJson(jsonObject, skill));
}

/**
 * @tc.name: SkillFromJson_010
 * @tc.desc: Test FromJson returns false when description is missing
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, SkillFromJson_010, TestSize.Level1)
{
    nlohmann::json jsonObject = nlohmann::json {
        { "id", "test" },
        { "name", "test" },
        { "tags", nlohmann::json::array({ "test" }) },
    };
    AgentSkill skill;
    EXPECT_FALSE(AgentSkill::FromJson(jsonObject, skill));
}

/**
 * @tc.name: SkillFromJson_011
 * @tc.desc: Test FromJson returns false when description is not a string
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, SkillFromJson_011, TestSize.Level1)
{
    nlohmann::json jsonObject = nlohmann::json {
        { "id", "test" },
        { "name", "test" },
        { "description", 789 },
        { "tags", nlohmann::json::array({ "test" }) },
    };
    AgentSkill skill;
    EXPECT_FALSE(AgentSkill::FromJson(jsonObject, skill));
}

/**
 * @tc.name: SkillFromJson_012
 * @tc.desc: Test FromJson returns false when description length exceeds 512
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, SkillFromJson_012, TestSize.Level1)
{
    std::string longDescription(513, 'a');
    nlohmann::json jsonObject = nlohmann::json {
        { "id", "test" },
        { "name", "test" },
        { "description", longDescription },
        { "tags", nlohmann::json::array({ "test" }) },
    };
    AgentSkill skill;
    EXPECT_FALSE(AgentSkill::FromJson(jsonObject, skill));
}

/**
 * @tc.name: SkillFromJson_013
 * @tc.desc: Test FromJson returns false when description is empty string
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, SkillFromJson_013, TestSize.Level1)
{
    nlohmann::json jsonObject = nlohmann::json {
        { "id", "test" },
        { "name", "test" },
        { "description", "" },
        { "tags", nlohmann::json::array({ "test" }) },
    };
    AgentSkill skill;
    EXPECT_FALSE(AgentSkill::FromJson(jsonObject, skill));
}

/**
 * @tc.name: SkillFromJson_014
 * @tc.desc: Test FromJson returns false when tags is missing
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, SkillFromJson_014, TestSize.Level1)
{
    nlohmann::json jsonObject = nlohmann::json {
        { "id", "test" },
        { "name", "test" },
        { "description", "test" },
    };
    AgentSkill skill;
    EXPECT_FALSE(AgentSkill::FromJson(jsonObject, skill));
}

/**
 * @tc.name: SkillFromJson_015
 * @tc.desc: Test FromJson returns false when tags is not an array
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, SkillFromJson_015, TestSize.Level1)
{
    nlohmann::json jsonObject = nlohmann::json {
        { "id", "test" },
        { "name", "test" },
        { "description", "test" },
        { "tags", "not an array" },
    };
    AgentSkill skill;
    EXPECT_FALSE(AgentSkill::FromJson(jsonObject, skill));
}

/**
 * @tc.name: SkillFromJson_016
 * @tc.desc: Test FromJson returns false when tags size exceeds 100
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, SkillFromJson_016, TestSize.Level1)
{
    nlohmann::json tagsArray = nlohmann::json::array();
    for (int i = 0; i < 101; i++) {
        tagsArray.push_back("tag" + std::to_string(i));
    }
    nlohmann::json jsonObject = nlohmann::json {
        { "id", "test" },
        { "name", "test" },
        { "description", "test" },
        { "tags", tagsArray },
    };
    AgentSkill skill;
    EXPECT_FALSE(AgentSkill::FromJson(jsonObject, skill));
}

/**
 * @tc.name: SkillFromJson_017
 * @tc.desc: Test FromJson returns false when all tags are invalid (too long), resulting in empty tags
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, SkillFromJson_017, TestSize.Level1)
{
    std::string longTag(33, 'a');
    nlohmann::json jsonObject = nlohmann::json {
        { "id", "test" },
        { "name", "test" },
        { "description", "test" },
        { "tags", nlohmann::json::array({ longTag }) },
    };
    AgentSkill skill;
    EXPECT_FALSE(AgentSkill::FromJson(jsonObject, skill));
}

/**
 * @tc.name: SkillFromJson_019
 * @tc.desc: Test FromJson returns true with valid boundary values
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, SkillFromJson_019, TestSize.Level1)
{
    std::string validId(64, 'a');
    std::string validName(128, 'b');
    std::string validDescription(512, 'c');
    std::string validTag(32, 'd');

    nlohmann::json tagsArray = nlohmann::json::array();
    for (int i = 0; i < 100; i++) {
        tagsArray.push_back(validTag);
    }

    nlohmann::json jsonObject = nlohmann::json {
        { "id", validId },
        { "name", validName },
        { "description", validDescription },
        { "tags", tagsArray },
    };
    AgentSkill skill;
    EXPECT_TRUE(AgentSkill::FromJson(jsonObject, skill));
    EXPECT_EQ(skill.id.length(), 64);
    EXPECT_EQ(skill.name.length(), 128);
    EXPECT_EQ(skill.description.length(), 512);
    EXPECT_EQ(skill.tags.size(), 100);
}

/**
 * @tc.name: SkillFromJson_020
 * @tc.desc: Test FromJson skips non-string elements in examples array
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, SkillFromJson_020, TestSize.Level1)
{
    nlohmann::json jsonObject = nlohmann::json {
        { "id", "test" },
        { "name", "test" },
        { "description", "test" },
        { "tags", nlohmann::json::array({ "test" }) },
        { "examples", nlohmann::json::array({ "example1", 123, true, "example2", nullptr }) },
    };
    AgentSkill skill;
    EXPECT_TRUE(AgentSkill::FromJson(jsonObject, skill));
    EXPECT_EQ(skill.examples.size(), 2);
    EXPECT_EQ(skill.examples[0], "example1");
    EXPECT_EQ(skill.examples[1], "example2");
}

/**
 * @tc.name: SkillFromJson_021
 * @tc.desc: Test FromJson skips non-string elements in inputModes array
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, SkillFromJson_021, TestSize.Level1)
{
    nlohmann::json jsonObject = nlohmann::json {
        { "id", "test" },
        { "name", "test" },
        { "description", "test" },
        { "tags", nlohmann::json::array({ "test" }) },
        { "inputModes", nlohmann::json::array({ "mode1", 456, false, "mode2", nullptr }) },
    };
    AgentSkill skill;
    EXPECT_TRUE(AgentSkill::FromJson(jsonObject, skill));
    EXPECT_EQ(skill.inputModes.size(), 2);
    EXPECT_EQ(skill.inputModes[0], "mode1");
    EXPECT_EQ(skill.inputModes[1], "mode2");
}

/**
 * @tc.name: SkillFromJson_022
 * @tc.desc: Test FromJson skips non-string elements in outputModes array
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, SkillFromJson_022, TestSize.Level1)
{
    nlohmann::json jsonObject = nlohmann::json {
        { "id", "test" },
        { "name", "test" },
        { "description", "test" },
        { "tags", nlohmann::json::array({ "test" }) },
        { "outputModes", nlohmann::json::array({ "output1", 789, true, "output2", nullptr }) },
    };
    AgentSkill skill;
    EXPECT_TRUE(AgentSkill::FromJson(jsonObject, skill));
    EXPECT_EQ(skill.outputModes.size(), 2);
    EXPECT_EQ(skill.outputModes[0], "output1");
    EXPECT_EQ(skill.outputModes[1], "output2");
}

/**
 * @tc.name: SkillFromJson_023
 * @tc.desc: Test FromJson skips non-string elements in tags array
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, SkillFromJson_023, TestSize.Level1)
{
    nlohmann::json jsonObject = nlohmann::json {
        { "id", "test" },
        { "name", "test" },
        { "description", "test" },
        { "tags", nlohmann::json::array({ "tag1", 123, true, "tag2", nullptr }) },
    };
    AgentSkill skill;
    EXPECT_TRUE(AgentSkill::FromJson(jsonObject, skill));
    EXPECT_EQ(skill.tags.size(), 2);
    EXPECT_EQ(skill.tags[0], "tag1");
    EXPECT_EQ(skill.tags[1], "tag2");
}

/**
 * @tc.name: SkillFromJson_024
 * @tc.desc: Test FromJson skips empty string tags
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, SkillFromJson_024, TestSize.Level1)
{
    nlohmann::json jsonObject = nlohmann::json {
        { "id", "test" },
        { "name", "test" },
        { "description", "test" },
        { "tags", nlohmann::json::array({ "", "validTag1", "", "validTag2", "" }) },
    };
    AgentSkill skill;
    EXPECT_TRUE(AgentSkill::FromJson(jsonObject, skill));
    EXPECT_EQ(skill.tags.size(), 2);
    EXPECT_EQ(skill.tags[0], "validTag1");
    EXPECT_EQ(skill.tags[1], "validTag2");
}

/**
 * @tc.name: SkillFromJson_025
 * @tc.desc: Test FromJson returns false when tags array is empty
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, SkillFromJson_025, TestSize.Level1)
{
    nlohmann::json jsonObject = nlohmann::json {
        { "id", "test" },
        { "name", "test" },
        { "description", "test" },
        { "tags", nlohmann::json::array() },
    };
    AgentSkill skill;
    EXPECT_FALSE(AgentSkill::FromJson(jsonObject, skill));
}

/**
 * @tc.name: SkillFromJson_026
 * @tc.desc: Test FromJson returns false when all tags are invalid (empty strings or too long)
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, SkillFromJson_026, TestSize.Level1)
{
    std::string longTag(33, 'a');
    nlohmann::json jsonObject = nlohmann::json {
        { "id", "test" },
        { "name", "test" },
        { "description", "test" },
        { "tags", nlohmann::json::array({ "", longTag, 123, nullptr, "" }) },
    };
    AgentSkill skill;
    EXPECT_FALSE(AgentSkill::FromJson(jsonObject, skill));
}

/**
 * @tc.name: SkillFromJson_027
 * @tc.desc: Test FromJson skips tags longer than 32 chars but keeps valid ones
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, SkillFromJson_027, TestSize.Level1)
{
    std::string longTag(33, 'a');
    nlohmann::json jsonObject = nlohmann::json {
        { "id", "test" },
        { "name", "test" },
        { "description", "test" },
        { "tags", nlohmann::json::array({ "valid1", longTag, "valid2" }) },
    };
    AgentSkill skill;
    EXPECT_TRUE(AgentSkill::FromJson(jsonObject, skill));
    EXPECT_EQ(skill.tags.size(), 2);
    EXPECT_EQ(skill.tags[0], "valid1");
    EXPECT_EQ(skill.tags[1], "valid2");
}

/**
 * @tc.name: SkillFromJson_028
 * @tc.desc: Test FromJson succeeds with valid extension field (length 1-1280)
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, SkillFromJson_028, TestSize.Level1)
{
    nlohmann::json jsonObject = nlohmann::json {
        { "id", "test" },
        { "name", "test" },
        { "description", "test" },
        { "tags", nlohmann::json::array({ "test" }) },
        { "extension", "valid extension data" },
    };
    AgentSkill skill;
    EXPECT_TRUE(AgentSkill::FromJson(jsonObject, skill));
    EXPECT_EQ(skill.extension, "valid extension data");
}

/**
 * @tc.name: SkillFromJson_029
 * @tc.desc: Test FromJson succeeds when extension length is exactly 1280
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, SkillFromJson_029, TestSize.Level1)
{
    std::string validExtension(1280, 'a');
    nlohmann::json jsonObject = nlohmann::json {
        { "id", "test" },
        { "name", "test" },
        { "description", "test" },
        { "tags", nlohmann::json::array({ "test" }) },
        { "extension", validExtension },
    };
    AgentSkill skill;
    EXPECT_TRUE(AgentSkill::FromJson(jsonObject, skill));
    EXPECT_EQ(skill.extension.length(), 1280);
}

/**
 * @tc.name: SkillFromJson_030
 * @tc.desc: Test FromJson sets extension to empty when length exceeds 1280
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, SkillFromJson_030, TestSize.Level1)
{
    std::string longExtension(1281, 'a');
    nlohmann::json jsonObject = nlohmann::json {
        { "id", "test" },
        { "name", "test" },
        { "description", "test" },
        { "tags", nlohmann::json::array({ "test" }) },
        { "extension", longExtension },
    };
    AgentSkill skill;
    EXPECT_TRUE(AgentSkill::FromJson(jsonObject, skill));
    EXPECT_EQ(skill.extension, "");
}

/**
 * @tc.name: SkillFromJson_031
 * @tc.desc: Test FromJson sets extension to empty when extension is empty string
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, SkillFromJson_031, TestSize.Level1)
{
    nlohmann::json jsonObject = nlohmann::json {
        { "id", "test" },
        { "name", "test" },
        { "description", "test" },
        { "tags", nlohmann::json::array({ "test" }) },
        { "extension", "" },
    };
    AgentSkill skill;
    EXPECT_TRUE(AgentSkill::FromJson(jsonObject, skill));
    EXPECT_EQ(skill.extension, "");
}

/**
 * @tc.name: SkillFromJson_032
 * @tc.desc: Test FromJson succeeds when extension field is missing (optional)
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, SkillFromJson_032, TestSize.Level1)
{
    nlohmann::json jsonObject = nlohmann::json {
        { "id", "test" },
        { "name", "test" },
        { "description", "test" },
        { "tags", nlohmann::json::array({ "test" }) },
    };
    AgentSkill skill;
    EXPECT_TRUE(AgentSkill::FromJson(jsonObject, skill));
    EXPECT_EQ(skill.extension, "");
}

/**
 * @tc.name: SkillFromJson_033
 * @tc.desc: Test FromJson ignores extension when it's not a string
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, SkillFromJson_033, TestSize.Level1)
{
    nlohmann::json jsonObject = nlohmann::json {
        { "id", "test" },
        { "name", "test" },
        { "description", "test" },
        { "tags", nlohmann::json::array({ "test" }) },
        { "extension", 12345 },
    };
    AgentSkill skill;
    EXPECT_TRUE(AgentSkill::FromJson(jsonObject, skill));
    EXPECT_EQ(skill.extension, "");
}

/**
* @tc.name  : Marshalling_ShouldReturnTrue_WhenAllParcelWriteOperationsSucceed
* @tc.number: AgentCard_Marshalling_001
* @tc.desc  : Test that Marshalling returns true when all parcel write operations succeed.
*/
HWTEST_F(AgentCardTest, AgentCard_Marshalling_001, TestSize.Level1)
{
    Parcel parcelMock;
    auto agentCard = std::make_shared<AgentCard>();
    agentCard->agentId = "1";
    agentCard->name = "ExampleName";
    agentCard->description = "ExampleDescription";
    agentCard->provider = std::make_shared<AgentProvider>();
    agentCard->version = "1.0.0";
    agentCard->documentationUrl = "http://docs.example.com";
    agentCard->capabilities = std::make_shared<AgentCapabilities>();
    agentCard->defaultInputModes = {"input1", "input2"};
    agentCard->defaultOutputModes = {"output1", "output2"};
    agentCard->skills = {std::make_shared<AgentSkill>()};

    EXPECT_TRUE(agentCard->Marshalling(parcelMock));
}

/**
* @tc.name  : Marshalling_ShouldReturnFalse_WhenWriteBundleNameFails
* @tc.number: AgentCard_Marshalling_002
* @tc.desc  : Test that Marshalling returns false when writing bundleName fails.
*/
HWTEST_F(AgentCardTest, AgentCard_Marshalling_002, TestSize.Level1)
{
    Parcel parcelMock;
    auto agentCard = std::make_shared<AgentCard>();
    EXPECT_FALSE(agentCard->Marshalling(parcelMock));
}

/**
* @tc.name  : Marshalling_ShouldReturnFalse_WhenWriteModuleNameFails
* @tc.number: AgentCard_Marshalling_003
* @tc.desc  : Test that Marshalling returns false when writing moduleName fails.
*/
HWTEST_F(AgentCardTest, AgentCard_Marshalling_003, TestSize.Level1)
{
    Parcel parcelMock;
    auto agentCard = std::make_shared<AgentCard>();
    EXPECT_FALSE(agentCard->Marshalling(parcelMock));
}

/**
* @tc.name  : Marshalling_ShouldReturnFalse_WhenWriteAbilityNameFails
* @tc.number: AgentCard_Marshalling_004
* @tc.desc  : Test that Marshalling returns false when writing abilityName fails.
*/
HWTEST_F(AgentCardTest, AgentCard_Marshalling_004, TestSize.Level1)
{
    Parcel parcelMock;
    auto agentCard = std::make_shared<AgentCard>();
    EXPECT_FALSE(agentCard->Marshalling(parcelMock));
}

/**
* @tc.name  : Marshalling_ShouldReturnFalse_WhenWriteAgentIdFails
* @tc.number: AgentCard_Marshalling_005
* @tc.desc  : Test that Marshalling returns false when writing agentId fails.
*/
HWTEST_F(AgentCardTest, AgentCard_Marshalling_005, TestSize.Level1)
{
    Parcel parcelMock;
    auto agentCard = std::make_shared<AgentCard>();
    agentCard->agentId = "0";

    EXPECT_FALSE(agentCard->Marshalling(parcelMock));
}

/**
* @tc.name  : Marshalling_ShouldReturnFalse_WhenWriteNameFails
* @tc.number: AgentCard_Marshalling_006
* @tc.desc  : Test that Marshalling returns false when writing name fails.
*/
HWTEST_F(AgentCardTest, AgentCard_Marshalling_006, TestSize.Level1)
{
    Parcel parcelMock;
    auto agentCard = std::make_shared<AgentCard>();
    agentCard->agentId = "1";
    agentCard->name = "test";

    EXPECT_FALSE(agentCard->Marshalling(parcelMock));
}

/**
* @tc.name  : Marshalling_ShouldReturnFalse_WhenWriteDescriptionFails
* @tc.number: AgentCard_Marshalling_007
* @tc.desc  : Test that Marshalling returns false when writing description fails.
*/
HWTEST_F(AgentCardTest, AgentCard_Marshalling_007, TestSize.Level1)
{
    Parcel parcelMock;
    auto agentCard = std::make_shared<AgentCard>();
    agentCard->agentId = "1";
    agentCard->name = "ExampleName";
    agentCard->description = "test";

    EXPECT_FALSE(agentCard->Marshalling(parcelMock));
}

/**
* @tc.name  : Marshalling_ShouldReturnFalse_WhenWriteUrlFails
* @tc.number: AgentCard_Marshalling_008
* @tc.desc  : Test that Marshalling returns false when writing url fails.
*/
HWTEST_F(AgentCardTest, AgentCard_Marshalling_008, TestSize.Level1)
{
    Parcel parcelMock;
    auto agentCard = std::make_shared<AgentCard>();
    agentCard->agentId = "1";
    agentCard->name = "ExampleName";
    agentCard->description = "ExampleDescription";

    EXPECT_FALSE(agentCard->Marshalling(parcelMock));
}

/**
* @tc.name  : Marshalling_ShouldReturnFalse_WhenWriteProviderFails
* @tc.number: AgentCard_Marshalling_009
* @tc.desc  : Test that Marshalling returns false when writing provider fails.
*/
HWTEST_F(AgentCardTest, AgentCard_Marshalling_009, TestSize.Level1)
{
    Parcel parcelMock;
    auto agentCard = std::make_shared<AgentCard>();
    agentCard->agentId = "1";
    agentCard->name = "ExampleName";
    agentCard->description = "ExampleDescription";

    EXPECT_FALSE(agentCard->Marshalling(parcelMock));
}

/**
* @tc.name  : Marshalling_ShouldReturnFalse_WhenWriteVersionFails
* @tc.number: AgentCard_Marshalling_010
* @tc.desc  : Test that Marshalling returns false when writing version fails.
*/
HWTEST_F(AgentCardTest, AgentCard_Marshalling_010, TestSize.Level1)
{
    Parcel parcelMock;
    auto agentCard = std::make_shared<AgentCard>();
    agentCard->agentId = "1";
    agentCard->name = "ExampleName";
    agentCard->description = "ExampleDescription";
    agentCard->provider = std::make_shared<AgentProvider>();
    agentCard->version = "test";

    EXPECT_FALSE(agentCard->Marshalling(parcelMock));
}

/**
* @tc.name  : Marshalling_ShouldReturnFalse_WhenWriteDocumentationUrlFails
* @tc.number: AgentCard_Marshalling_011
* @tc.desc  : Test that Marshalling returns false when writing documentationUrl fails.
*/
HWTEST_F(AgentCardTest, AgentCard_Marshalling_011, TestSize.Level1)
{
    Parcel parcelMock;
    auto agentCard = std::make_shared<AgentCard>();
    agentCard->agentId = "1";
    agentCard->name = "ExampleName";
    agentCard->description = "ExampleDescription";
    agentCard->provider = std::make_shared<AgentProvider>();
    agentCard->version = "1.0.0";
    agentCard->documentationUrl = "test";

    EXPECT_FALSE(agentCard->Marshalling(parcelMock));
}

/**
* @tc.name  : Marshalling_ShouldReturnFalse_WhenWriteCapabilitiesFails
* @tc.number: AgentCard_Marshalling_012
* @tc.desc  : Test that Marshalling returns false when writing capabilities fails.
*/
HWTEST_F(AgentCardTest, AgentCard_Marshalling_012, TestSize.Level1)
{
    Parcel parcelMock;
    auto agentCard = std::make_shared<AgentCard>();
    agentCard->agentId = "1";
    agentCard->name = "ExampleName";
    agentCard->description = "ExampleDescription";
    agentCard->provider = std::make_shared<AgentProvider>();
    agentCard->version = "1.0.0";
    agentCard->documentationUrl = "http://docs.example.com";

    EXPECT_FALSE(agentCard->Marshalling(parcelMock));
}

/**
* @tc.name  : Marshalling_ShouldReturnFalse_WhenWriteDefaultInputModesFails
* @tc.number: AgentCard_Marshalling_014
* @tc.desc  : Test that Marshalling returns false when writing defaultInputModes fails.
*/
HWTEST_F(AgentCardTest, AgentCard_Marshalling_014, TestSize.Level1)
{
    Parcel parcelMock;
    auto agentCard = std::make_shared<AgentCard>();
    agentCard->agentId = "1";
    agentCard->name = "ExampleName";
    agentCard->description = "ExampleDescription";
    agentCard->provider = std::make_shared<AgentProvider>();
    agentCard->version = "1.0.0";
    agentCard->documentationUrl = "http://docs.example.com";
    agentCard->capabilities = std::make_shared<AgentCapabilities>();

    EXPECT_FALSE(agentCard->Marshalling(parcelMock));
}

/**
* @tc.name  : Marshalling_ShouldReturnFalse_WhenWriteDefaultOutputModesFails
* @tc.number: AgentCard_Marshalling_015
* @tc.desc  : Test that Marshalling returns false when writing defaultOutputModes fails.
*/
HWTEST_F(AgentCardTest, AgentCard_Marshalling_015, TestSize.Level1)
{
    Parcel parcelMock;
    auto agentCard = std::make_shared<AgentCard>();
    agentCard->agentId = "1";
    agentCard->name = "ExampleName";
    agentCard->description = "ExampleDescription";
    agentCard->provider = std::make_shared<AgentProvider>();
    agentCard->version = "1.0.0";
    agentCard->documentationUrl = "http://docs.example.com";
    agentCard->capabilities = std::make_shared<AgentCapabilities>();
    agentCard->defaultInputModes = {"input1", "input2"};

    EXPECT_FALSE(agentCard->Marshalling(parcelMock));
}

/**
* @tc.name  : Marshalling_ShouldReturnFalse_WhenWriteSkillSizeFails
* @tc.number: AgentCard_Marshalling_016
* @tc.desc  : Test that Marshalling returns false when writing skill size fails.
*/
HWTEST_F(AgentCardTest, AgentCard_Marshalling_016, TestSize.Level1)
{
    Parcel parcelMock;
    auto agentCard = std::make_shared<AgentCard>();
    agentCard->agentId = "1";
    agentCard->name = "ExampleName";
    agentCard->description = "ExampleDescription";
    agentCard->provider = std::make_shared<AgentProvider>();
    agentCard->version = "1.0.0";
    agentCard->documentationUrl = "http://docs.example.com";
    agentCard->capabilities = std::make_shared<AgentCapabilities>();
    agentCard->defaultInputModes = {"input1", "input2"};
    agentCard->defaultOutputModes = {"output1", "output2"};

    EXPECT_FALSE(agentCard->Marshalling(parcelMock));
}

/**
* @tc.name  : Marshalling_ShouldReturnFalse_WhenWriteSkillFails
* @tc.number: AgentCard_Marshalling_017
* @tc.desc  : Test that Marshalling returns false when writing skill fails.
*/
HWTEST_F(AgentCardTest, AgentCard_Marshalling_017, TestSize.Level1)
{
    Parcel parcelMock;
    auto agentCard = std::make_shared<AgentCard>();
    agentCard->agentId = "1";
    agentCard->name = "ExampleName";
    agentCard->description = "ExampleDescription";
    agentCard->provider = std::make_shared<AgentProvider>();
    agentCard->version = "1.0.0";
    agentCard->documentationUrl = "http://docs.example.com";
    agentCard->capabilities = std::make_shared<AgentCapabilities>();
    agentCard->defaultInputModes = {"input1", "input2"};
    agentCard->defaultOutputModes = {"output1", "output2"};
    agentCard->skills = { nullptr };

    EXPECT_FALSE(agentCard->Marshalling(parcelMock));
}

/**
* @tc.name  : RawDataCpy_ShouldReturnError_WhenReadDataIsNull
* @tc.number: RawDataCpy_001
* @tc.desc  : Test that RawDataCpy returns ERR_INVALID_AGENT_CARD_DATA when readdata is nullptr
*/
HWTEST_F(AgentCardTest, RawDataCpy_001, TestSize.Level1)
{
    AgentCardsRawData* agentCardData = new AgentCardsRawData();
    const void* readdata = nullptr;
    int32_t result = agentCardData->RawDataCpy(readdata);
    EXPECT_EQ(result, ERR_INVALID_AGENT_CARD_DATA);
    delete agentCardData;
}

/**
* @tc.name  : RawDataCpy_ShouldReturnError_WhenSizeIsZero
* @tc.number: RawDataCpy_002
* @tc.desc  : Test that RawDataCpy returns ERR_INVALID_AGENT_CARD_DATA when size is zero
*/
HWTEST_F(AgentCardTest, RawDataCpy_002, TestSize.Level1)
{
    AgentCardsRawData* agentCardData = new AgentCardsRawData();
    agentCardData->size = 0;
    const void* readdata = malloc(10);
    int32_t result = agentCardData->RawDataCpy(readdata);
    EXPECT_EQ(result, ERR_INVALID_AGENT_CARD_DATA);
    free(const_cast<void*>(readdata));
    delete agentCardData;
}

/**
* @tc.name  : RawDataCpy_ShouldCopyDataSuccessfully_WhenInputsAreValid
* @tc.number: RawDataCpy_005
* @tc.desc  : Test that RawDataCpy successfully copies data when inputs are valid
*/
HWTEST_F(AgentCardTest, RawDataCpy_005, TestSize.Level1)
{
    AgentCardsRawData* agentCardData = new AgentCardsRawData();
    const void* readdata = malloc(10);
    agentCardData->size = 10;
    memset(const_cast<void*>(readdata), 0xAA, 10);

    int32_t result = agentCardData->RawDataCpy(readdata);
    EXPECT_EQ(result, ERR_OK);
    EXPECT_NE(agentCardData->data, nullptr);
    EXPECT_EQ(agentCardData->isMalloc, true);

    free(const_cast<void*>(readdata));
    delete agentCardData;
}

/**
* @tc.name  : RawDataCpy_ShouldFreeOldData_WhenDataIsNotNull
* @tc.number: RawDataCpy_006
* @tc.desc  : Test that RawDataCpy frees the old data when data is not nullptr
*/
HWTEST_F(AgentCardTest, RawDataCpy_006, TestSize.Level1)
{
    AgentCardsRawData* agentCardData = new AgentCardsRawData();
    // Initialize old data
    agentCardData->data = malloc(10);
    agentCardData->size = 10;

    const void* readdata = malloc(10);
    memset(const_cast<void*>(readdata), 0xAA, 10);

    int32_t result = agentCardData->RawDataCpy(readdata);
    EXPECT_EQ(result, ERR_OK);
    EXPECT_NE(agentCardData->data, nullptr);
    EXPECT_EQ(agentCardData->isMalloc, true);

    // Verify the old data is freed
    EXPECT_NE(agentCardData->data, const_cast<void*>(readdata));

    free(const_cast<void*>(readdata));
    delete agentCardData;
}

/**
* @tc.name  : ToAgentCardVec_ShouldReturnError_WhenCountIsZero
* @tc.number: ToAgentCardVec_001
* @tc.desc  : Test that ToAgentCardVec returns ERR_AGENT_CARD_LIST_OUT_OF_RANGE when count is zero.
*/
HWTEST_F(AgentCardTest, ToAgentCardVec_001, TestSize.Level1)
{
    AgentCardsRawData rawData;
    rawData.data = new uint8_t[4]{0, 0, 0, 0}; // count = 0
    rawData.isMalloc = true;
    rawData.size = 4;

    std::vector<AgentCard> cards;
    int32_t result = AgentCardsRawData::ToAgentCardVec(rawData, cards);

    EXPECT_EQ(result, ERR_AGENT_CARD_LIST_OUT_OF_RANGE);
}

/**
* @tc.name  : ToAgentCardVec_ShouldReturnError_WhenCountExceedsMaxLimit
* @tc.number: ToAgentCardVec_002
* @tc.desc  : Test that ToAgentCardVec returns ERR_AGENT_CARD_LIST_OUT_OF_RANGE when count exceeds MAX_AGENT_CARD_COUNT.
*/
HWTEST_F(AgentCardTest, ToAgentCardVec_002, TestSize.Level1)
{
    AgentCardsRawData rawData;
    uint32_t count = MAX_AGENT_CARD_COUNT + 1;
    rawData.data = new uint8_t[4]{static_cast<uint8_t>(count), static_cast<uint8_t>(count >> 8),
                                  static_cast<uint8_t>(count >> 16), static_cast<uint8_t>(count >> 24)};
    rawData.isMalloc = true;
    rawData.size = 4;

    std::vector<AgentCard> cards;
    int32_t result = AgentCardsRawData::ToAgentCardVec(rawData, cards);

    EXPECT_EQ(result, ERR_AGENT_CARD_LIST_OUT_OF_RANGE);
}

/**
* @tc.name  : ToAgentCardVec_ShouldReturnError_WhenCardSizeIsInvalid
* @tc.number: ToAgentCardVec_003
* @tc.desc  : Test that ToAgentCardVec returns ERR_INVALID_AGENT_CARD_DATA when cardSize is invalid.
*/
HWTEST_F(AgentCardTest, ToAgentCardVec_003, TestSize.Level1)
{
    AgentCardsRawData rawData;
    uint32_t count = 1;
    uint32_t cardSize = 100; // Invalid card size
    rawData.data = new uint8_t[8]{static_cast<uint8_t>(count), static_cast<uint8_t>(count >> 8),
                                  static_cast<uint8_t>(count >> 16), static_cast<uint8_t>(count >> 24),
                                  static_cast<uint8_t>(cardSize), static_cast<uint8_t>(cardSize >> 8),
                                  static_cast<uint8_t>(cardSize >> 16), static_cast<uint8_t>(cardSize >> 24)};
    rawData.isMalloc = true;
    rawData.size = 8;

    std::vector<AgentCard> cards;
    int32_t result = AgentCardsRawData::ToAgentCardVec(rawData, cards);

    EXPECT_EQ(result, ERR_INVALID_AGENT_CARD_DATA);
}

/**
* @tc.name  : ToAgentCardVec_ShouldReturnSuccess_WhenDataIsValid
* @tc.number: ToAgentCardVec_004
* @tc.desc  : Test that ToAgentCardVec returns ERR_OK when the input data is valid.
*/
HWTEST_F(AgentCardTest, ToAgentCardVec_004, TestSize.Level1)
{
    AgentCard agentCard;
    agentCard.agentId = "1";
    agentCard.name = "ExampleName";
    agentCard.description = "ExampleDescription";
    agentCard.provider = std::make_shared<AgentProvider>();
    agentCard.version = "1.0.0";
    agentCard.documentationUrl = "http://docs.example.com";
    agentCard.capabilities = std::make_shared<AgentCapabilities>();
    agentCard.defaultInputModes = {"input1", "input2"};
    agentCard.defaultOutputModes = {"output1", "output2"};
    agentCard.skills = {std::make_shared<AgentSkill>()};

    AgentCardsRawData rawData;
    std::vector<AgentCard> vec = { agentCard };
    AgentCardsRawData::FromAgentCardVec(vec, rawData);
    EXPECT_NE(rawData.data, nullptr);
    EXPECT_TRUE(rawData.size > 0);

    std::vector<AgentCard> cards;
    int32_t result = AgentCardsRawData::ToAgentCardVec(rawData, cards);

    EXPECT_EQ(result, ERR_OK);
}

/**
* @tc.name  : FromAgentCardVec
* @tc.number: FromAgentCardVec_001
* @tc.desc  : FromAgentCardVec_001
*/
HWTEST_F(AgentCardTest, FromAgentCardVec_001, TestSize.Level1)
{
    AgentCard agentCard;
    agentCard.agentId = "1";
    agentCard.name = "ExampleName";
    agentCard.description = "ExampleDescription";
    agentCard.provider = std::make_shared<AgentProvider>();
    agentCard.version = "1.0.0";
    agentCard.documentationUrl = "http://docs.example.com";
    agentCard.capabilities = std::make_shared<AgentCapabilities>();
    agentCard.defaultInputModes = {"input1", "input2"};
    agentCard.defaultOutputModes = {"output1", "output2"};
    agentCard.skills = {std::make_shared<AgentSkill>()};

    AgentCardsRawData rawData;
    std::vector<AgentCard> vec = { agentCard };
    AgentCardsRawData::FromAgentCardVec(vec, rawData);
    EXPECT_NE(rawData.data, nullptr);
    EXPECT_TRUE(rawData.size > 0);
}

/**
 * @tc.name: AgentCardFromJson_002
 * @tc.desc: Test FromJson returns false when agentId is missing
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, AgentCardFromJson_002, TestSize.Level1)
{
    nlohmann::json jsonObject = nlohmann::json {
        // agentId is missing
        { "name", "test" },
        { "description", "test description" },
        { "version", "1.0" },
        { "documentationUrl", "http://example.com" },
    };
    AgentCard agentCard;
    EXPECT_FALSE(AgentCard::FromJson(jsonObject, agentCard));
}

/**
 * @tc.name: AgentCardFromJson_003
 * @tc.desc: Test FromJson returns false when agentId is not a string
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, AgentCardFromJson_003, TestSize.Level1)
{
    nlohmann::json jsonObject = nlohmann::json {
        { "agentId", 123 }, // agentId is a number, not a string
        { "name", "test" },
        { "description", "test description" },
        { "version", "1.0" },
        { "documentationUrl", "http://example.com" },
    };
    AgentCard agentCard;
    EXPECT_FALSE(AgentCard::FromJson(jsonObject, agentCard));
}

/**
 * @tc.name: AgentCardFromJson_004
 * @tc.desc: Test FromJson returns false when agentId length exceeds 64
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, AgentCardFromJson_004, TestSize.Level1)
{
    std::string longAgentId(65, 'a'); // 65 characters, exceeds LENGTH_64
    nlohmann::json jsonObject = nlohmann::json {
        { "agentId", longAgentId },
        { "name", "test" },
        { "description", "test description" },
        { "version", "1.0" },
        { "documentationUrl", "http://example.com" },
    };
    AgentCard agentCard;
    EXPECT_FALSE(AgentCard::FromJson(jsonObject, agentCard));
}

/**
 * @tc.name: AgentCardFromJson_005
 * @tc.desc: Test FromJson returns false when agentId is empty string
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, AgentCardFromJson_005, TestSize.Level1)
{
    nlohmann::json jsonObject = nlohmann::json {
        { "agentId", "" }, // empty string
        { "name", "test" },
        { "description", "test description" },
        { "version", "1.0" },
        { "documentationUrl", "http://example.com" },
    };
    AgentCard agentCard;
    EXPECT_FALSE(AgentCard::FromJson(jsonObject, agentCard));
}

/**
 * @tc.name: AgentCardFromJson_006
 * @tc.desc: Test FromJson returns true when agentId length is exactly 64
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, AgentCardFromJson_006, TestSize.Level1)
{
    std::string validAgentId(64, 'a'); // exactly 64 characters
    nlohmann::json jsonObject = nlohmann::json {
        { "agentId", validAgentId },
        { "name", "test" },
        { "description", "test description" },
        { "version", "1.0" },
        { "documentationUrl", "http://example.com" },
    };
    AgentCard agentCard;
    EXPECT_TRUE(AgentCard::FromJson(jsonObject, agentCard));
    EXPECT_EQ(agentCard.agentId, validAgentId);
}

/**
 * @tc.name: AgentCardFromJson_007
 * @tc.desc: Test FromJson returns false when name is missing
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, AgentCardFromJson_007, TestSize.Level1)
{
    nlohmann::json jsonObject = nlohmann::json {
        { "agentId", "1" },
        // name is missing
        { "description", "test description" },
        { "version", "1.0" },
        { "documentationUrl", "http://example.com" },
    };
    AgentCard agentCard;
    EXPECT_FALSE(AgentCard::FromJson(jsonObject, agentCard));
}

/**
 * @tc.name: AgentCardFromJson_008
 * @tc.desc: Test FromJson returns false when name is not a string
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, AgentCardFromJson_008, TestSize.Level1)
{
    nlohmann::json jsonObject = nlohmann::json {
        { "agentId", "1" },
        { "name", 123 }, // name is a number, not a string
        { "description", "test description" },
        { "version", "1.0" },
        { "documentationUrl", "http://example.com" },
    };
    AgentCard agentCard;
    EXPECT_FALSE(AgentCard::FromJson(jsonObject, agentCard));
}

/**
 * @tc.name: AgentCardFromJson_009
 * @tc.desc: Test FromJson returns false when name length exceeds 64
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, AgentCardFromJson_009, TestSize.Level1)
{
    std::string longName(65, 'a'); // 65 characters, exceeds LENGTH_64
    nlohmann::json jsonObject = nlohmann::json {
        { "agentId", "1" },
        { "name", longName },
        { "description", "test description" },
        { "version", "1.0" },
        { "documentationUrl", "http://example.com" },
    };
    AgentCard agentCard;
    EXPECT_FALSE(AgentCard::FromJson(jsonObject, agentCard));
}

/**
 * @tc.name: AgentCardFromJson_010
 * @tc.desc: Test FromJson returns false when name is empty string
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, AgentCardFromJson_010, TestSize.Level1)
{
    nlohmann::json jsonObject = nlohmann::json {
        { "agentId", "1" },
        { "name", "" }, // empty string
        { "description", "test description" },
        { "version", "1.0" },
        { "documentationUrl", "http://example.com" },
    };
    AgentCard agentCard;
    EXPECT_FALSE(AgentCard::FromJson(jsonObject, agentCard));
}

/**
 * @tc.name: AgentCardFromJson_011
 * @tc.desc: Test FromJson returns false when description is missing
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, AgentCardFromJson_011, TestSize.Level1)
{
    nlohmann::json jsonObject = nlohmann::json {
        { "agentId", "1" },
        { "name", "test" },
        // description is missing
        { "version", "1.0" },
        { "documentationUrl", "http://example.com" },
    };
    AgentCard agentCard;
    EXPECT_FALSE(AgentCard::FromJson(jsonObject, agentCard));
}

/**
 * @tc.name: AgentCardFromJson_012
 * @tc.desc: Test FromJson returns false when description is not a string
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, AgentCardFromJson_012, TestSize.Level1)
{
    nlohmann::json jsonObject = nlohmann::json {
        { "agentId", "1" },
        { "name", "test" },
        { "description", 456 }, // description is a number, not a string
        { "version", "1.0" },
        { "documentationUrl", "http://example.com" },
    };
    AgentCard agentCard;
    EXPECT_FALSE(AgentCard::FromJson(jsonObject, agentCard));
}

/**
 * @tc.name: AgentCardFromJson_013
 * @tc.desc: Test FromJson returns false when description length exceeds 512
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, AgentCardFromJson_013, TestSize.Level1)
{
    std::string longDescription(513, 'a'); // 513 characters, exceeds LENGTH_512
    nlohmann::json jsonObject = nlohmann::json {
        { "agentId", "1" },
        { "name", "test" },
        { "description", longDescription },
        { "version", "1.0" },
        { "documentationUrl", "http://example.com" },
    };
    AgentCard agentCard;
    EXPECT_FALSE(AgentCard::FromJson(jsonObject, agentCard));
}

/**
 * @tc.name: AgentCardFromJson_014
 * @tc.desc: Test FromJson returns false when description is empty string
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, AgentCardFromJson_014, TestSize.Level1)
{
    nlohmann::json jsonObject = nlohmann::json {
        { "agentId", "1" },
        { "name", "test" },
        { "description", "" }, // empty string
        { "version", "1.0" },
        { "documentationUrl", "http://example.com" },
    };
    AgentCard agentCard;
    EXPECT_FALSE(AgentCard::FromJson(jsonObject, agentCard));
}

/**
 * @tc.name: AgentCardFromJson_015
 * @tc.desc: Test FromJson returns false when version is missing
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, AgentCardFromJson_015, TestSize.Level1)
{
    nlohmann::json jsonObject = nlohmann::json {
        { "agentId", "1" },
        { "name", "test" },
        { "description", "test description" },
        // version is missing
        { "documentationUrl", "http://example.com" },
    };
    AgentCard agentCard;
    EXPECT_FALSE(AgentCard::FromJson(jsonObject, agentCard));
}

/**
 * @tc.name: AgentCardFromJson_016
 * @tc.desc: Test FromJson returns false when version is not a string
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, AgentCardFromJson_016, TestSize.Level1)
{
    nlohmann::json jsonObject = nlohmann::json {
        { "agentId", "1" },
        { "name", "test" },
        { "description", "test description" },
        { "version", 789 }, // version is a number, not a string
        { "documentationUrl", "http://example.com" },
    };
    AgentCard agentCard;
    EXPECT_FALSE(AgentCard::FromJson(jsonObject, agentCard));
}

/**
 * @tc.name: AgentCardFromJson_017
 * @tc.desc: Test FromJson returns false when version length exceeds 32
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, AgentCardFromJson_017, TestSize.Level1)
{
    std::string longVersion(33, 'a'); // 33 characters, exceeds LENGTH_32
    nlohmann::json jsonObject = nlohmann::json {
        { "agentId", "1" },
        { "name", "test" },
        { "description", "test description" },
        { "version", longVersion },
        { "documentationUrl", "http://example.com" },
    };
    AgentCard agentCard;
    EXPECT_FALSE(AgentCard::FromJson(jsonObject, agentCard));
}

/**
 * @tc.name: AgentCardFromJson_018
 * @tc.desc: Test FromJson returns false when version is empty string
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, AgentCardFromJson_018, TestSize.Level1)
{
    nlohmann::json jsonObject = nlohmann::json {
        { "agentId", "1" },
        { "name", "test" },
        { "description", "test description" },
        { "version", "" }, // empty string
        { "documentationUrl", "http://example.com" },
    };
    AgentCard agentCard;
    EXPECT_FALSE(AgentCard::FromJson(jsonObject, agentCard));
}

/**
 * @tc.name: AgentCardFromJson_019
 * @tc.desc: Test FromJson returns true when documentationUrl is missing (optional field)
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, AgentCardFromJson_019, TestSize.Level1)
{
    nlohmann::json jsonObject = nlohmann::json {
        { "agentId", "1" },
        { "name", "test" },
        { "description", "test description" },
        { "version", "1.0" },
        // documentationUrl is missing (optional)
    };
    AgentCard agentCard;
    EXPECT_TRUE(AgentCard::FromJson(jsonObject, agentCard));
    EXPECT_TRUE(agentCard.documentationUrl.empty());
}

/**
 * @tc.name: AgentCardFromJson_020
 * @tc.desc: Test FromJson sets documentationUrl to empty when length exceeds 512
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, AgentCardFromJson_020, TestSize.Level1)
{
    std::string longUrl(513, 'a'); // 513 characters, exceeds LENGTH_512
    nlohmann::json jsonObject = nlohmann::json {
        { "agentId", "1" },
        { "name", "test" },
        { "description", "test description" },
        { "version", "1.0" },
        { "documentationUrl", longUrl },
    };
    AgentCard agentCard;
    EXPECT_TRUE(AgentCard::FromJson(jsonObject, agentCard));
    EXPECT_TRUE(agentCard.documentationUrl.empty());
}

/**
 * @tc.name: AgentCardFromJson_021
 * @tc.desc: Test FromJson returns true with empty documentationUrl (optional field)
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, AgentCardFromJson_021, TestSize.Level1)
{
    nlohmann::json jsonObject = nlohmann::json {
        { "agentId", "1" },
        { "name", "test" },
        { "description", "test description" },
        { "version", "1.0" },
        { "documentationUrl", "" }, // empty string is allowed
    };
    AgentCard agentCard;
    EXPECT_TRUE(AgentCard::FromJson(jsonObject, agentCard));
}

/**
 * @tc.name: AgentCardFromJson_022
 * @tc.desc: Test FromJson returns true with valid boundary values
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, AgentCardFromJson_022, TestSize.Level1)
{
    std::string validName(64, 'a'); // exactly 64 characters
    std::string validDescription(512, 'b'); // exactly 512 characters
    std::string validVersion(32, 'c'); // exactly 32 characters
    std::string validDocUrl(512, 'd'); // exactly 512 characters

    nlohmann::json jsonObject = nlohmann::json {
        { "agentId", "1" },
        { "name", validName },
        { "description", validDescription },
        { "version", validVersion },
        { "documentationUrl", validDocUrl },
    };
    AgentCard agentCard;
    EXPECT_TRUE(AgentCard::FromJson(jsonObject, agentCard));
    EXPECT_EQ(agentCard.name.length(), 64);
    EXPECT_EQ(agentCard.description.length(), 512);
    EXPECT_EQ(agentCard.version.length(), 32);
    EXPECT_EQ(agentCard.documentationUrl.length(), 512);
}

/**
 * @tc.name: AgentCardFromJson_023
 * @tc.desc: Test FromJson skips non-string elements in defaultInputModes array
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, AgentCardFromJson_023, TestSize.Level1)
{
    nlohmann::json jsonObject = nlohmann::json {
        { "agentId", "1" },
        { "name", "test" },
        { "description", "test description" },
        { "version", "1.0" },
        { "defaultInputModes", nlohmann::json::array({ "input1", 123, true, "input2", nullptr }) },
    };
    AgentCard agentCard;
    EXPECT_TRUE(AgentCard::FromJson(jsonObject, agentCard));
    EXPECT_EQ(agentCard.defaultInputModes.size(), 2);
    EXPECT_EQ(agentCard.defaultInputModes[0], "input1");
    EXPECT_EQ(agentCard.defaultInputModes[1], "input2");
}

/**
 * @tc.name: AgentCardFromJson_024
 * @tc.desc: Test FromJson skips non-string elements in defaultOutputModes array
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, AgentCardFromJson_024, TestSize.Level1)
{
    nlohmann::json jsonObject = nlohmann::json {
        { "agentId", "1" },
        { "name", "test" },
        { "description", "test description" },
        { "version", "1.0" },
        { "defaultOutputModes", nlohmann::json::array({ "output1", 456, false, "output2", nullptr }) },
    };
    AgentCard agentCard;
    EXPECT_TRUE(AgentCard::FromJson(jsonObject, agentCard));
    EXPECT_EQ(agentCard.defaultOutputModes.size(), 2);
    EXPECT_EQ(agentCard.defaultOutputModes[0], "output1");
    EXPECT_EQ(agentCard.defaultOutputModes[1], "output2");
}

/**
 * @tc.name: AgentCardFromJson_025
 * @tc.desc: Test FromJson succeeds with valid extension field (length 1-51200)
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, AgentCardFromJson_025, TestSize.Level1)
{
    nlohmann::json jsonObject = nlohmann::json {
        { "agentId", "1" },
        { "name", "test" },
        { "description", "test description" },
        { "version", "1.0" },
        { "extension", "valid extension data" },
    };
    AgentCard agentCard;
    EXPECT_TRUE(AgentCard::FromJson(jsonObject, agentCard));
    EXPECT_EQ(agentCard.extension, "valid extension data");
}

/**
 * @tc.name: AgentCardFromJson_026
 * @tc.desc: Test FromJson succeeds when extension length is exactly 51200
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, AgentCardFromJson_026, TestSize.Level1)
{
    std::string validExtension(51200, 'a');
    nlohmann::json jsonObject = nlohmann::json {
        { "agentId", "1" },
        { "name", "test" },
        { "description", "test description" },
        { "version", "1.0" },
        { "extension", validExtension },
    };
    AgentCard agentCard;
    EXPECT_TRUE(AgentCard::FromJson(jsonObject, agentCard));
    EXPECT_EQ(agentCard.extension.length(), 51200);
}

/**
 * @tc.name: AgentCardFromJson_027
 * @tc.desc: Test FromJson sets extension to empty when length exceeds 51200
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, AgentCardFromJson_027, TestSize.Level1)
{
    std::string longExtension(51201, 'a');
    nlohmann::json jsonObject = nlohmann::json {
        { "agentId", "1" },
        { "name", "test" },
        { "description", "test description" },
        { "version", "1.0" },
        { "extension", longExtension },
    };
    AgentCard agentCard;
    EXPECT_TRUE(AgentCard::FromJson(jsonObject, agentCard));
    EXPECT_EQ(agentCard.extension, "");
}

/**
 * @tc.name: AgentCardFromJson_028
 * @tc.desc: Test FromJson sets extension to empty when extension is empty string
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, AgentCardFromJson_028, TestSize.Level1)
{
    nlohmann::json jsonObject = nlohmann::json {
        { "agentId", "1" },
        { "name", "test" },
        { "description", "test description" },
        { "version", "1.0" },
        { "extension", "" },
    };
    AgentCard agentCard;
    EXPECT_TRUE(AgentCard::FromJson(jsonObject, agentCard));
    EXPECT_EQ(agentCard.extension, "");
}

/**
 * @tc.name: AgentCardFromJson_029
 * @tc.desc: Test FromJson succeeds when extension field is missing (optional)
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, AgentCardFromJson_029, TestSize.Level1)
{
    nlohmann::json jsonObject = nlohmann::json {
        { "agentId", "1" },
        { "name", "test" },
        { "description", "test description" },
        { "version", "1.0" },
    };
    AgentCard agentCard;
    EXPECT_TRUE(AgentCard::FromJson(jsonObject, agentCard));
    EXPECT_EQ(agentCard.extension, "");
}

/**
 * @tc.name: AgentCardFromJson_030
 * @tc.desc: Test FromJson ignores extension when it's not a string
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, AgentCardFromJson_030, TestSize.Level1)
{
    nlohmann::json jsonObject = nlohmann::json {
        { "agentId", "1" },
        { "name", "test" },
        { "description", "test description" },
        { "version", "1.0" },
        { "extension", 12345 },
    };
    AgentCard agentCard;
    EXPECT_TRUE(AgentCard::FromJson(jsonObject, agentCard));
    EXPECT_EQ(agentCard.extension, "");
}

/**
 * @tc.name: AgentCardFromJson_041
 * @tc.desc: Test FromJson succeeds with valid iconUrl field (length 1-512)
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, AgentCardFromJson_041, TestSize.Level1)
{
    nlohmann::json jsonObject = nlohmann::json {
        { "agentId", "1" },
        { "name", "test" },
        { "description", "test description" },
        { "version", "1.0" },
        { "iconUrl", "http://example.com/icon.png" },
    };
    AgentCard agentCard;
    EXPECT_TRUE(AgentCard::FromJson(jsonObject, agentCard));
    EXPECT_EQ(agentCard.iconUrl, "http://example.com/icon.png");
}

/**
 * @tc.name: AgentCardFromJson_031
 * @tc.desc: Test FromJson succeeds when iconUrl length is exactly 512
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, AgentCardFromJson_031, TestSize.Level1)
{
    std::string validIconUrl(512, 'a');
    nlohmann::json jsonObject = nlohmann::json {
        { "agentId", "1" },
        { "name", "test" },
        { "description", "test description" },
        { "version", "1.0" },
        { "iconUrl", validIconUrl },
    };
    AgentCard agentCard;
    EXPECT_TRUE(AgentCard::FromJson(jsonObject, agentCard));
    EXPECT_EQ(agentCard.iconUrl.length(), 512);
}

/**
 * @tc.name: AgentCardFromJson_032
 * @tc.desc: Test FromJson sets iconUrl to empty when length exceeds 512
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, AgentCardFromJson_032, TestSize.Level1)
{
    std::string longIconUrl(513, 'a');
    nlohmann::json jsonObject = nlohmann::json {
        { "agentId", "1" },
        { "name", "test" },
        { "description", "test description" },
        { "version", "1.0" },
        { "iconUrl", longIconUrl },
    };
    AgentCard agentCard;
    EXPECT_TRUE(AgentCard::FromJson(jsonObject, agentCard));
    EXPECT_EQ(agentCard.iconUrl, "");
}

/**
 * @tc.name: AgentCardFromJson_033
 * @tc.desc: Test FromJson sets iconUrl to empty when iconUrl is empty string
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, AgentCardFromJson_033, TestSize.Level1)
{
    nlohmann::json jsonObject = nlohmann::json {
        { "agentId", "1" },
        { "name", "test" },
        { "description", "test description" },
        { "version", "1.0" },
        { "iconUrl", "" },
    };
    AgentCard agentCard;
    EXPECT_TRUE(AgentCard::FromJson(jsonObject, agentCard));
    EXPECT_EQ(agentCard.iconUrl, "");
}

/**
 * @tc.name: AgentCardFromJson_034
 * @tc.desc: Test FromJson ignores iconUrl when it's not a string
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, AgentCardFromJson_034, TestSize.Level1)
{
    nlohmann::json jsonObject = nlohmann::json {
        { "agentId", "1" },
        { "name", "test" },
        { "description", "test description" },
        { "version", "1.0" },
        { "iconUrl", 12345 },
    };
    AgentCard agentCard;
    EXPECT_TRUE(AgentCard::FromJson(jsonObject, agentCard));
    EXPECT_EQ(agentCard.iconUrl, "");
}

/**
 * @tc.name: AgentCardFromJson_035
 * @tc.desc: Test FromJson succeeds when category is provided with valid length (1-64)
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, AgentCardFromJson_035, TestSize.Level1)
{
    nlohmann::json jsonObject = nlohmann::json {
        { "agentId", "1" },
        { "name", "test" },
        { "description", "test description" },
        { "version", "1.0" },
        { "category", "productivity" },
    };
    AgentCard agentCard;
    EXPECT_TRUE(AgentCard::FromJson(jsonObject, agentCard));
    EXPECT_EQ(agentCard.category, "productivity");
}

/**
 * @tc.name: AgentCardFromJson_036
 * @tc.desc: Test FromJson fails when category is missing (required field)
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, AgentCardFromJson_036, TestSize.Level1)
{
    nlohmann::json jsonObject = nlohmann::json {
        { "agentId", "1" },
        { "name", "test" },
        { "description", "test description" },
        { "version", "1.0" },
    };
    AgentCard agentCard;
    EXPECT_FALSE(AgentCard::FromJson(jsonObject, agentCard));
}

/**
 * @tc.name: AgentCardFromJson_037
 * @tc.desc: Test FromJson fails when category length exceeds 64
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, AgentCardFromJson_037, TestSize.Level1)
{
    std::string longCategory(65, 'a');
    nlohmann::json jsonObject = nlohmann::json {
        { "agentId", "1" },
        { "name", "test" },
        { "description", "test description" },
        { "version", "1.0" },
        { "category", longCategory },
    };
    AgentCard agentCard;
    EXPECT_FALSE(AgentCard::FromJson(jsonObject, agentCard));
}

/**
 * @tc.name: AgentCardFromJson_038
 * @tc.desc: Test FromJson fails when category is empty string
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, AgentCardFromJson_038, TestSize.Level1)
{
    nlohmann::json jsonObject = nlohmann::json {
        { "agentId", "1" },
        { "name", "test" },
        { "description", "test description" },
        { "version", "1.0" },
        { "category", "" },
    };
    AgentCard agentCard;
    EXPECT_FALSE(AgentCard::FromJson(jsonObject, agentCard));
}

/**
 * @tc.name: AgentCardFromJson_039
 * @tc.desc: Test FromJson succeeds when category length is exactly 64
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, AgentCardFromJson_039, TestSize.Level1)
{
    std::string validCategory(64, 'a');
    nlohmann::json jsonObject = nlohmann::json {
        { "agentId", "1" },
        { "name", "test" },
        { "description", "test description" },
        { "version", "1.0" },
        { "category", validCategory },
    };
    AgentCard agentCard;
    EXPECT_TRUE(AgentCard::FromJson(jsonObject, agentCard));
    EXPECT_EQ(agentCard.category.length(), 64);
}

/**
 * @tc.name: AgentCardFromJson_040
 * @tc.desc: Test FromJson fails when category is not a string
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, AgentCardFromJson_040, TestSize.Level1)
{
    nlohmann::json jsonObject = nlohmann::json {
        { "agentId", "1" },
        { "name", "test" },
        { "description", "test description" },
        { "version", "1.0" },
        { "category", 12345 },
    };
    AgentCard agentCard;
    EXPECT_FALSE(AgentCard::FromJson(jsonObject, agentCard));
}

/**
 * @tc.name: AgentCardFromJson_042
 * @tc.desc: Test AgentCard FromJson succeeds with appInfo
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, AgentCardFromJson_042, TestSize.Level1)
{
    nlohmann::json jsonObject = nlohmann::json {
        { "agentId", "1" },
        { "name", "test" },
        { "description", "test description" },
        { "version", "1.0" },
        { "category", "productivity" },
        { "appInfo", nlohmann::json {
            { "bundleName", "test.bundle" },
            { "moduleName", "test" },
            { "abilityName", "TestAbility" },
            { "deviceTypes", "phone" },
            { "minAppVersion", "1.0.0" },
        }},
    };
    AgentCard agentCard;
    EXPECT_TRUE(AgentCard::FromJson(jsonObject, agentCard));
    EXPECT_NE(agentCard.appInfo, nullptr);
    EXPECT_EQ(agentCard.appInfo->bundleName, "test.bundle");
    EXPECT_EQ(agentCard.appInfo->moduleName, "test");
    EXPECT_EQ(agentCard.appInfo->abilityName, "TestAbility");
    EXPECT_EQ(agentCard.appInfo->deviceTypes, "phone");
    EXPECT_EQ(agentCard.appInfo->minAppVersion, "1.0.0");
}

/**
 * @tc.name: AgentCardFromJson_043
 * @tc.desc: Test AgentCard FromJson fails when appInfo has invalid deviceTypes
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, AgentCardFromJson_043, TestSize.Level1)
{
    std::string longDeviceTypes(129, 'a');
    nlohmann::json jsonObject = nlohmann::json {
        { "agentId", "1" },
        { "name", "test" },
        { "description", "test description" },
        { "version", "1.0" },
        { "category", "productivity" },
        { "appInfo", nlohmann::json {
            { "deviceTypes", longDeviceTypes },
        }},
    };
    AgentCard agentCard;
    EXPECT_FALSE(AgentCard::FromJson(jsonObject, agentCard));
}

/**
 * @tc.name: AgentAppInfoFromJson_001
 * @tc.desc: Test AgentAppInfo FromJson succeeds with all optional fields
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, AgentAppInfoFromJson_001, TestSize.Level1)
{
    nlohmann::json jsonObject = nlohmann::json {
        { "bundleName", "test.bundle" },
        { "moduleName", "test" },
        { "abilityName", "TestAbility" },
        { "deviceTypes", "phone" },
        { "minAppVersion", "1.0.0" },
    };
    AgentAppInfo appInfo;
    EXPECT_TRUE(AgentAppInfo::FromJson(jsonObject, appInfo));
    EXPECT_EQ(appInfo.bundleName, "test.bundle");
    EXPECT_EQ(appInfo.moduleName, "test");
    EXPECT_EQ(appInfo.abilityName, "TestAbility");
    EXPECT_EQ(appInfo.deviceTypes, "phone");
    EXPECT_EQ(appInfo.minAppVersion, "1.0.0");
}

/**
 * @tc.name: AgentAppInfoFromJson_002
 * @tc.desc: Test AgentAppInfo FromJson succeeds with empty json
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, AgentAppInfoFromJson_002, TestSize.Level1)
{
    nlohmann::json jsonObject = nlohmann::json {};
    AgentAppInfo appInfo;
    EXPECT_TRUE(AgentAppInfo::FromJson(jsonObject, appInfo));
    EXPECT_TRUE(appInfo.bundleName.empty());
    EXPECT_TRUE(appInfo.moduleName.empty());
    EXPECT_TRUE(appInfo.abilityName.empty());
    EXPECT_TRUE(appInfo.deviceTypes.empty());
    EXPECT_TRUE(appInfo.minAppVersion.empty());
}

/**
 * @tc.name: AgentAppInfoFromJson_003
 * @tc.desc: Test AgentAppInfo FromJson fails when deviceTypes length exceeds 128
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, AgentAppInfoFromJson_003, TestSize.Level1)
{
    std::string longDeviceTypes(129, 'a');
    nlohmann::json jsonObject = nlohmann::json {
        { "deviceTypes", longDeviceTypes },
    };
    AgentAppInfo appInfo;
    EXPECT_FALSE(AgentAppInfo::FromJson(jsonObject, appInfo));
}

/**
 * @tc.name: AgentAppInfoFromJson_004
 * @tc.desc: Test AgentAppInfo FromJson succeeds when deviceTypes length is exactly 128
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, AgentAppInfoFromJson_004, TestSize.Level1)
{
    std::string validDeviceTypes(128, 'a');
    nlohmann::json jsonObject = nlohmann::json {
        { "deviceTypes", validDeviceTypes },
    };
    AgentAppInfo appInfo;
    EXPECT_TRUE(AgentAppInfo::FromJson(jsonObject, appInfo));
    EXPECT_EQ(appInfo.deviceTypes.length(), 128);
}

/**
 * @tc.name: AgentAppInfoFromJson_005
 * @tc.desc: Test AgentAppInfo FromJson fails when minAppVersion length exceeds 32
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, AgentAppInfoFromJson_005, TestSize.Level1)
{
    std::string longMinAppVersion(33, 'a');
    nlohmann::json jsonObject = nlohmann::json {
        { "minAppVersion", longMinAppVersion },
    };
    AgentAppInfo appInfo;
    EXPECT_FALSE(AgentAppInfo::FromJson(jsonObject, appInfo));
}

/**
 * @tc.name: AgentAppInfoFromJson_006
 * @tc.desc: Test AgentAppInfo FromJson succeeds when minAppVersion length is exactly 32
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, AgentAppInfoFromJson_006, TestSize.Level1)
{
    std::string validMinAppVersion(32, 'a');
    nlohmann::json jsonObject = nlohmann::json {
        { "minAppVersion", validMinAppVersion },
    };
    AgentAppInfo appInfo;
    EXPECT_TRUE(AgentAppInfo::FromJson(jsonObject, appInfo));
    EXPECT_EQ(appInfo.minAppVersion.length(), 32);
}

/**
 * @tc.name: CapabilitiesFromJson_009
 * @tc.desc: Test Capabilities FromJson succeeds with extension and extendedAgentCard
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, CapabilitiesFromJson_009, TestSize.Level1)
{
    nlohmann::json jsonObject = nlohmann::json {
        { "streaming", true },
        { "extension", "test extension data" },
        { "extendedAgentCard", true },
    };
    AgentCapabilities capabilities = AgentCapabilities::FromJson(jsonObject);
    EXPECT_TRUE(capabilities.streaming);
    EXPECT_EQ(capabilities.extension, "test extension data");
    EXPECT_TRUE(capabilities.extendedAgentCard);
}

/**
 * @tc.name: CapabilitiesFromJson_010
 * @tc.desc: Test Capabilities FromJson fails when extension length exceeds 1280
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, CapabilitiesFromJson_010, TestSize.Level1)
{
    std::string longExtension(1281, 'a');
    nlohmann::json jsonObject = nlohmann::json {
        { "streaming", true },
        { "extension", longExtension },
    };
    AgentCapabilities capabilities = AgentCapabilities::FromJson(jsonObject);
    EXPECT_TRUE(capabilities.extension.empty());
}

/**
 * @tc.name: AgentAppInfoParcel_001
 * @tc.desc: Test AgentAppInfo Marshalling and Unmarshalling
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, AgentAppInfoParcel_001, TestSize.Level1)
{
    AgentAppInfo originalAppInfo;
    originalAppInfo.bundleName = "test.bundle";
    originalAppInfo.moduleName = "test";
    originalAppInfo.abilityName = "TestAbility";
    originalAppInfo.deviceTypes = "phone";
    originalAppInfo.minAppVersion = "1.0.0";

    Parcel parcel;
    EXPECT_TRUE(originalAppInfo.Marshalling(parcel));

    AgentAppInfo *unmarshalledAppInfo = AgentAppInfo::Unmarshalling(parcel);
    EXPECT_NE(unmarshalledAppInfo, nullptr);
    if (unmarshalledAppInfo != nullptr) {
        EXPECT_EQ(unmarshalledAppInfo->bundleName, originalAppInfo.bundleName);
        EXPECT_EQ(unmarshalledAppInfo->moduleName, originalAppInfo.moduleName);
        EXPECT_EQ(unmarshalledAppInfo->abilityName, originalAppInfo.abilityName);
        EXPECT_EQ(unmarshalledAppInfo->deviceTypes, originalAppInfo.deviceTypes);
        EXPECT_EQ(unmarshalledAppInfo->minAppVersion, originalAppInfo.minAppVersion);
        delete unmarshalledAppInfo;
    }
}
} // namespace AgentRuntime
} // namespace OHOS