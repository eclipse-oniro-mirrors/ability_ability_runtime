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

#include <limits>

#include "ability_manager_errors.h"
#include "agent_card.h"
#include "agent_extension_connection_constants.h"
#include "extension_ability_info.h"
#include "ipc_object_stub.h"
#define private public
#include "utils/agent_ability_util.h"
#undef private
#include "permission_verification.h"
#include "want.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;
using namespace OHOS::AgentRuntime;

namespace {
class AgentAbilityUtilRemoteObject : public IPCObjectStub {
public:
    AgentAbilityUtilRemoteObject() : IPCObjectStub(u"agent_ability_util_remote") {}
    int OnRemoteRequest(uint32_t, MessageParcel &, MessageParcel &, MessageOption &) override
    {
        return ERR_OK;
    }
};
}

class AgentAbilityUtilTest : public testing::Test {
public:
    void SetUp() override { g_mockGrantFoundationPermission = false; }
    void TearDown() override { g_mockGrantFoundationPermission = false; }
};

// ===================== IsAgentExtensionType =====================

HWTEST_F(AgentAbilityUtilTest, IsAgentExtensionTypeAgentReturnsTrue, TestSize.Level1)
{
    EXPECT_TRUE(AgentAbilityUtil::IsAgentExtensionType(ExtensionAbilityType::AGENT));
}

HWTEST_F(AgentAbilityUtilTest, IsAgentExtensionTypeNonAgentReturnsFalse, TestSize.Level1)
{
    EXPECT_FALSE(AgentAbilityUtil::IsAgentExtensionType(ExtensionAbilityType::SERVICE));
    EXPECT_FALSE(AgentAbilityUtil::IsAgentExtensionType(ExtensionAbilityType::FORM));
    EXPECT_FALSE(AgentAbilityUtil::IsAgentExtensionType(ExtensionAbilityType::UNSPECIFIED));
}

// ===================== IsAgentExtensionInfo =====================

HWTEST_F(AgentAbilityUtilTest, IsAgentExtensionInfoAgentTypeReturnsTrue, TestSize.Level1)
{
    ExtensionAbilityInfo info;
    info.type = ExtensionAbilityType::AGENT;
    EXPECT_TRUE(AgentAbilityUtil::IsAgentExtensionInfo(info));
}

HWTEST_F(AgentAbilityUtilTest, IsAgentExtensionInfoNonAgentReturnsFalse, TestSize.Level1)
{
    ExtensionAbilityInfo info;
    info.type = ExtensionAbilityType::SERVICE;
    EXPECT_FALSE(AgentAbilityUtil::IsAgentExtensionInfo(info));
}

// ===================== IsAgentExtensionAbilityInfo =====================

HWTEST_F(AgentAbilityUtilTest, IsAgentExtensionAbilityInfoExtensionAndAgentReturnsTrue, TestSize.Level1)
{
    AbilityInfo info;
    info.type = AbilityType::EXTENSION;
    info.extensionAbilityType = ExtensionAbilityType::AGENT;
    EXPECT_TRUE(AgentAbilityUtil::IsAgentExtensionAbilityInfo(info));
}

HWTEST_F(AgentAbilityUtilTest, IsAgentExtensionAbilityInfoNonExtensionReturnsFalse, TestSize.Level1)
{
    AbilityInfo info;
    info.type = AbilityType::PAGE;
    info.extensionAbilityType = ExtensionAbilityType::AGENT;
    EXPECT_FALSE(AgentAbilityUtil::IsAgentExtensionAbilityInfo(info));
}

HWTEST_F(AgentAbilityUtilTest, IsAgentExtensionAbilityInfoNonAgentExtensionReturnsFalse, TestSize.Level1)
{
    AbilityInfo info;
    info.type = AbilityType::EXTENSION;
    info.extensionAbilityType = ExtensionAbilityType::SERVICE;
    EXPECT_FALSE(AgentAbilityUtil::IsAgentExtensionAbilityInfo(info));
}

// ===================== IsAtomicServiceAgentExtensionInfo =====================

HWTEST_F(AgentAbilityUtilTest, IsAtomicServiceAgentExtensionInfoAtomicServiceReturnsTrue, TestSize.Level1)
{
    ExtensionAbilityInfo info;
    info.type = ExtensionAbilityType::AGENT;
    info.applicationInfo.bundleType = BundleType::ATOMIC_SERVICE;
    EXPECT_TRUE(AgentAbilityUtil::IsAtomicServiceAgentExtensionInfo(info));
}

HWTEST_F(AgentAbilityUtilTest, IsAtomicServiceAgentExtensionInfoNonAtomicReturnsFalse, TestSize.Level1)
{
    ExtensionAbilityInfo info;
    info.type = ExtensionAbilityType::AGENT;
    info.applicationInfo.bundleType = BundleType::APP;
    EXPECT_FALSE(AgentAbilityUtil::IsAtomicServiceAgentExtensionInfo(info));
}

// ===================== HasAtomicServiceAgentExtensionInfo =====================

HWTEST_F(AgentAbilityUtilTest, HasAtomicServiceAgentExtensionInfoEmptyVectorReturnsFalse, TestSize.Level1)
{
    std::vector<ExtensionAbilityInfo> infos;
    EXPECT_FALSE(AgentAbilityUtil::HasAtomicServiceAgentExtensionInfo(infos));
}

HWTEST_F(AgentAbilityUtilTest, HasAtomicServiceAgentExtensionInfoWithAtomicServiceReturnsTrue, TestSize.Level1)
{
    std::vector<ExtensionAbilityInfo> infos;
    ExtensionAbilityInfo info;
    info.type = ExtensionAbilityType::AGENT;
    info.applicationInfo.bundleType = BundleType::ATOMIC_SERVICE;
    infos.push_back(info);
    EXPECT_TRUE(AgentAbilityUtil::HasAtomicServiceAgentExtensionInfo(infos));
}

HWTEST_F(AgentAbilityUtilTest, HasAtomicServiceAgentExtensionInfoNoMatchReturnsFalse, TestSize.Level1)
{
    std::vector<ExtensionAbilityInfo> infos;
    ExtensionAbilityInfo info;
    info.type = ExtensionAbilityType::SERVICE;
    info.applicationInfo.bundleType = BundleType::ATOMIC_SERVICE;
    infos.push_back(info);
    EXPECT_FALSE(AgentAbilityUtil::HasAtomicServiceAgentExtensionInfo(infos));
}

// ===================== HasAgentOnlyParams =====================

HWTEST_F(AgentAbilityUtilTest, HasAgentOnlyParamsCleanWantReturnsFalse, TestSize.Level1)
{
    Want want;
    EXPECT_FALSE(AgentAbilityUtil::HasAgentOnlyParams(want));
}

HWTEST_F(AgentAbilityUtilTest, HasAgentOnlyParamsAgentIdPresentReturnsTrue, TestSize.Level1)
{
    Want want;
    want.SetParam(AGENTID_KEY, std::string("agent-1"));
    EXPECT_TRUE(AgentAbilityUtil::HasAgentOnlyParams(want));
}

HWTEST_F(AgentAbilityUtilTest, HasAgentOnlyParamsCardTypePresentReturnsTrue, TestSize.Level1)
{
    Want want;
    want.SetParam(AGENT_CARD_TYPE_KEY, static_cast<int32_t>(AgentCardType::LOW_CODE));
    EXPECT_TRUE(AgentAbilityUtil::HasAgentOnlyParams(want));
}

HWTEST_F(AgentAbilityUtilTest, HasAgentOnlyParamsNoncePresentReturnsTrue, TestSize.Level1)
{
    Want want;
    want.SetParam(AGENT_VERIFICATION_NONCE_KEY, 1L);
    EXPECT_TRUE(AgentAbilityUtil::HasAgentOnlyParams(want));
}

HWTEST_F(AgentAbilityUtilTest, HasAgentOnlyParamsHostProxyPresentReturnsTrue, TestSize.Level1)
{
    Want want;
    want.SetParam(AGENTEXTENSIONHOSTPROXY_KEY, std::string("placeholder"));
    EXPECT_TRUE(AgentAbilityUtil::HasAgentOnlyParams(want));
}

HWTEST_F(AgentAbilityUtilTest, AgentVerificationNonceParamRoundTripsFixedWidth, TestSize.Level1)
{
    constexpr int64_t nonce = std::numeric_limits<int64_t>::max() - 31;
    Want want;

    AgentAbilityUtil::SetAgentVerificationNonceParam(want, nonce);

    EXPECT_EQ(AgentAbilityUtil::GetAgentVerificationNonceParam(want), nonce);
}

// ===================== CheckAgentConnectEntry =====================

HWTEST_F(AgentAbilityUtilTest, CheckAgentConnectEntryNonAgentCleanWantReturnsOk, TestSize.Level1)
{
    Want want;
    EXPECT_EQ(AgentAbilityUtil::CheckAgentConnectEntry(want, ExtensionAbilityType::SERVICE), ERR_OK);
}

HWTEST_F(AgentAbilityUtilTest, CheckAgentConnectEntryNonAgentWithAgentParamsReturnsWrongCall, TestSize.Level1)
{
    Want want;
    want.SetParam(AGENTID_KEY, std::string("agent-1"));
    EXPECT_EQ(AgentAbilityUtil::CheckAgentConnectEntry(want, ExtensionAbilityType::SERVICE),
        ERR_WRONG_INTERFACE_CALL);
}

HWTEST_F(AgentAbilityUtilTest, CheckAgentConnectEntryAgentNoPermissionReturnsCheckPermFailed, TestSize.Level1)
{
    g_mockGrantFoundationPermission = false;
    Want want;
    EXPECT_EQ(AgentAbilityUtil::CheckAgentConnectEntry(want, ExtensionAbilityType::AGENT),
        CHECK_PERMISSION_FAILED);
}

HWTEST_F(AgentAbilityUtilTest, CheckAgentConnectEntryAgentInvalidParamsReturnsInvalidValue, TestSize.Level1)
{
    g_mockGrantFoundationPermission = true;
    Want want;
    // AGENTID_KEY absent => empty string; AGENT_CARD_TYPE_KEY absent; nonce absent (<=0).
    EXPECT_EQ(AgentAbilityUtil::CheckAgentConnectEntry(want, ExtensionAbilityType::AGENT), ERR_INVALID_VALUE);
}

HWTEST_F(AgentAbilityUtilTest, CheckAgentConnectEntryAgentMissingCardTypeReturnsInvalidValue, TestSize.Level1)
{
    g_mockGrantFoundationPermission = true;
    Want want;
    want.SetParam(AGENTID_KEY, std::string("agent-1"));
    // AGENT_CARD_TYPE_KEY still absent.
    EXPECT_EQ(AgentAbilityUtil::CheckAgentConnectEntry(want, ExtensionAbilityType::AGENT), ERR_INVALID_VALUE);
}

HWTEST_F(AgentAbilityUtilTest, CheckAgentConnectEntryAgentNullHostProxyReturnsInvalidValue, TestSize.Level1)
{
    g_mockGrantFoundationPermission = true;
    Want want;
    want.SetParam(AGENTID_KEY, std::string("agent-1"));
    want.SetParam(AGENT_CARD_TYPE_KEY, static_cast<int32_t>(AgentCardType::LOW_CODE));
    want.SetParam(AGENT_VERIFICATION_NONCE_KEY, 1L);
    // AGENTEXTENSIONHOSTPROXY_KEY not set as a remote object.
    EXPECT_EQ(AgentAbilityUtil::CheckAgentConnectEntry(want, ExtensionAbilityType::AGENT), ERR_INVALID_VALUE);
}

HWTEST_F(AgentAbilityUtilTest, CheckAgentConnectEntryAgentAcceptsFixedWidthNonce, TestSize.Level1)
{
    constexpr int64_t nonce = std::numeric_limits<int64_t>::max() - 17;
    g_mockGrantFoundationPermission = true;
    Want want;
    want.SetParam(AGENTID_KEY, std::string("agent-1"));
    want.SetParam(AGENT_CARD_TYPE_KEY, static_cast<int32_t>(AgentCardType::APP));
    sptr<IRemoteObject> hostProxy = sptr<AgentAbilityUtilRemoteObject>::MakeSptr();
    want.SetParam(AGENTEXTENSIONHOSTPROXY_KEY, hostProxy);
    AgentAbilityUtil::SetAgentVerificationNonceParam(want, nonce);

    EXPECT_EQ(AgentAbilityUtil::CheckAgentConnectEntry(want, ExtensionAbilityType::AGENT), ERR_OK);
}

// ===================== CheckConnectAgentResolvedTarget =====================

HWTEST_F(AgentAbilityUtilTest, CheckConnectResolvedAgentRequestAndAgentTargetReturnsOk, TestSize.Level1)
{
    AbilityInfo info;
    info.type = AbilityType::EXTENSION;
    info.extensionAbilityType = ExtensionAbilityType::AGENT;
    EXPECT_EQ(AgentAbilityUtil::CheckConnectAgentResolvedTarget(ExtensionAbilityType::AGENT, info), ERR_OK);
}

HWTEST_F(AgentAbilityUtilTest, CheckConnectResolvedAgentRequestNonAgentTargetReturnsWrongCall, TestSize.Level1)
{
    AbilityInfo info;
    info.type = AbilityType::EXTENSION;
    info.extensionAbilityType = ExtensionAbilityType::SERVICE;
    EXPECT_EQ(AgentAbilityUtil::CheckConnectAgentResolvedTarget(ExtensionAbilityType::AGENT, info),
        ERR_WRONG_INTERFACE_CALL);
}

HWTEST_F(AgentAbilityUtilTest, CheckConnectResolvedNonAgentRequestAgentTargetReturnsWrongCall, TestSize.Level1)
{
    AbilityInfo info;
    info.type = AbilityType::EXTENSION;
    info.extensionAbilityType = ExtensionAbilityType::AGENT;
    EXPECT_EQ(AgentAbilityUtil::CheckConnectAgentResolvedTarget(ExtensionAbilityType::SERVICE, info),
        ERR_WRONG_INTERFACE_CALL);
}

HWTEST_F(AgentAbilityUtilTest, CheckConnectResolvedNonAgentRequestNonAgentTargetReturnsOk, TestSize.Level1)
{
    AbilityInfo info;
    info.type = AbilityType::EXTENSION;
    info.extensionAbilityType = ExtensionAbilityType::SERVICE;
    EXPECT_EQ(AgentAbilityUtil::CheckConnectAgentResolvedTarget(ExtensionAbilityType::SERVICE, info), ERR_OK);
}
