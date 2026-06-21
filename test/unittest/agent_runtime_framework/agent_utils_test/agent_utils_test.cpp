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

#include <cstdint>
#include <memory>
#include <type_traits>

#include "agent_card.h"
#include "agent_utils.h"
#include "want.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AAFwk;
using namespace OHOS::AgentRuntime;

namespace {
constexpr int64_t MIN_VERIFICATION_NONCE = 1000000000;

std::shared_ptr<AgentAppInfo> BuildAppInfo(const std::string &bundle, const std::string &ability,
    const std::string &module)
{
    auto info = std::make_shared<AgentAppInfo>();
    info->bundleName = bundle;
    info->abilityName = ability;
    info->moduleName = module;
    return info;
}

AgentCard BuildCard(const std::shared_ptr<AgentAppInfo> &appInfo)
{
    AgentCard card;
    card.appInfo = appInfo;
    return card;
}
}

class AgentUtilsTest : public testing::Test {};

// ===================== GenerateVerificationNonce =====================

/**
 * @tc.name  : GenerateVerificationNonce_001
 * @tc.desc  : GenerateVerificationNonce returns a positive fixed-width int64_t nonce.
 */
HWTEST_F(AgentUtilsTest, GenerateVerificationNonce_001, TestSize.Level1)
{
    static_assert(std::is_same<decltype(GenerateVerificationNonce()), int64_t>::value,
        "GenerateVerificationNonce must return int64_t");
    int64_t nonce = GenerateVerificationNonce();
    EXPECT_GT(nonce, 0);
}

/**
 * @tc.name  : GenerateVerificationNonce_002
 * @tc.desc  : GenerateVerificationNonce honors the minimum-nonce floor so values are far from zero.
 */
HWTEST_F(AgentUtilsTest, GenerateVerificationNonce_002, TestSize.Level1)
{
    int64_t nonce = GenerateVerificationNonce();
    EXPECT_GE(nonce, MIN_VERIFICATION_NONCE);
}

/**
 * @tc.name  : GenerateVerificationNonce_003
 * @tc.desc  : Two consecutive nonce generations are distinct (non-colliding in practice).
 */
HWTEST_F(AgentUtilsTest, GenerateVerificationNonce_003, TestSize.Level1)
{
    int64_t first = GenerateVerificationNonce();
    int64_t second = GenerateVerificationNonce();
    EXPECT_NE(first, second);
}

// ===================== IsMatchedAgentCardTarget =====================

/**
 * @tc.name  : IsMatchedAgentCardTarget_NullAppInfoReturnsFalse
 * @tc.desc  : IsMatchedAgentCardTarget returns false when the card has no appInfo.
 */
HWTEST_F(AgentUtilsTest, IsMatchedAgentCardTarget_NullAppInfoReturnsFalse, TestSize.Level1)
{
    AgentCard card;
    Want want;
    want.SetElementName("", "bundle", "ability", "entry");
    EXPECT_FALSE(IsMatchedAgentCardTarget(want, card));
}

/**
 * @tc.name  : IsMatchedAgentCardTarget_FullMatchReturnsTrue
 * @tc.desc  : IsMatchedAgentCardTarget returns true when bundle/ability/module all match.
 */
HWTEST_F(AgentUtilsTest, IsMatchedAgentCardTarget_FullMatchReturnsTrue, TestSize.Level1)
{
    auto card = BuildCard(BuildAppInfo("bundle", "ability", "entry"));
    Want want;
    want.SetElementName("", "bundle", "ability", "entry");
    EXPECT_TRUE(IsMatchedAgentCardTarget(want, card));
}

/**
 * @tc.name  : IsMatchedAgentCardTarget_MismatchedBundleReturnsFalse
 * @tc.desc  : IsMatchedAgentCardTarget returns false when the bundle name differs.
 */
HWTEST_F(AgentUtilsTest, IsMatchedAgentCardTarget_MismatchedBundleReturnsFalse, TestSize.Level1)
{
    auto card = BuildCard(BuildAppInfo("bundle", "ability", "entry"));
    Want want;
    want.SetElementName("", "other.bundle", "ability", "entry");
    EXPECT_FALSE(IsMatchedAgentCardTarget(want, card));
}

/**
 * @tc.name  : IsMatchedAgentCardTarget_MismatchedAbilityReturnsFalse
 * @tc.desc  : IsMatchedAgentCardTarget returns false when the ability name differs.
 */
HWTEST_F(AgentUtilsTest, IsMatchedAgentCardTarget_MismatchedAbilityReturnsFalse, TestSize.Level1)
{
    auto card = BuildCard(BuildAppInfo("bundle", "ability", "entry"));
    Want want;
    want.SetElementName("", "bundle", "other.ability", "entry");
    EXPECT_FALSE(IsMatchedAgentCardTarget(want, card));
}

/**
 * @tc.name  : IsMatchedAgentCardTarget_WantModuleEmptyReturnsTrue
 * @tc.desc  : IsMatchedAgentCardTarget returns true when the want module name is empty (wildcard).
 */
HWTEST_F(AgentUtilsTest, IsMatchedAgentCardTarget_WantModuleEmptyReturnsTrue, TestSize.Level1)
{
    auto card = BuildCard(BuildAppInfo("bundle", "ability", "entry"));
    Want want;
    want.SetElementName("", "bundle", "ability", "");
    EXPECT_TRUE(IsMatchedAgentCardTarget(want, card));
}

/**
 * @tc.name  : IsMatchedAgentCardTarget_CardModuleEmptyReturnsTrue
 * @tc.desc  : IsMatchedAgentCardTarget returns true when the card module name is empty (wildcard).
 */
HWTEST_F(AgentUtilsTest, IsMatchedAgentCardTarget_CardModuleEmptyReturnsTrue, TestSize.Level1)
{
    auto card = BuildCard(BuildAppInfo("bundle", "ability", ""));
    Want want;
    want.SetElementName("", "bundle", "ability", "entry");
    EXPECT_TRUE(IsMatchedAgentCardTarget(want, card));
}

/**
 * @tc.name  : IsMatchedAgentCardTarget_MismatchedModuleReturnsFalse
 * @tc.desc  : IsMatchedAgentCardTarget returns false when both module names are non-empty and differ.
 */
HWTEST_F(AgentUtilsTest, IsMatchedAgentCardTarget_MismatchedModuleReturnsFalse, TestSize.Level1)
{
    auto card = BuildCard(BuildAppInfo("bundle", "ability", "entry"));
    Want want;
    want.SetElementName("", "bundle", "ability", "other");
    EXPECT_FALSE(IsMatchedAgentCardTarget(want, card));
}

// ===================== NormalizeAgentConnectWant =====================

/**
 * @tc.name  : NormalizeAgentConnectWant_NullAppInfoIsNoOp
 * @tc.desc  : NormalizeAgentConnectWant leaves the want unchanged when the card has no appInfo.
 */
HWTEST_F(AgentUtilsTest, NormalizeAgentConnectWant_NullAppInfoIsNoOp, TestSize.Level1)
{
    AgentCard card;
    Want want;
    want.SetElementName("", "bundle", "ability", "");
    NormalizeAgentConnectWant(want, card);
    EXPECT_TRUE(want.GetElement().GetModuleName().empty());
}

/**
 * @tc.name  : NormalizeAgentConnectWant_FillsEmptyModuleFromCard
 * @tc.desc  : NormalizeAgentConnectWant fills an empty want module name from the card.
 */
HWTEST_F(AgentUtilsTest, NormalizeAgentConnectWant_FillsEmptyModuleFromCard, TestSize.Level1)
{
    auto card = BuildCard(BuildAppInfo("bundle", "ability", "entry"));
    Want want;
    want.SetElementName("", "bundle", "ability", "");
    NormalizeAgentConnectWant(want, card);
    EXPECT_EQ(want.GetElement().GetModuleName(), "entry");
}

/**
 * @tc.name  : NormalizeAgentConnectWant_KeepsExistingModule
 * @tc.desc  : NormalizeAgentConnectWant does not overwrite a non-empty want module name.
 */
HWTEST_F(AgentUtilsTest, NormalizeAgentConnectWant_KeepsExistingModule, TestSize.Level1)
{
    auto card = BuildCard(BuildAppInfo("bundle", "ability", "entry"));
    Want want;
    want.SetElementName("", "bundle", "ability", "caller");
    NormalizeAgentConnectWant(want, card);
    EXPECT_EQ(want.GetElement().GetModuleName(), "caller");
}

/**
 * @tc.name  : NormalizeAgentConnectWant_EmptyCardModuleIsNoOp
 * @tc.desc  : NormalizeAgentConnectWant leaves the want module empty when the card module is empty.
 */
HWTEST_F(AgentUtilsTest, NormalizeAgentConnectWant_EmptyCardModuleIsNoOp, TestSize.Level1)
{
    auto card = BuildCard(BuildAppInfo("bundle", "ability", ""));
    Want want;
    want.SetElementName("", "bundle", "ability", "");
    NormalizeAgentConnectWant(want, card);
    EXPECT_TRUE(want.GetElement().GetModuleName().empty());
}
