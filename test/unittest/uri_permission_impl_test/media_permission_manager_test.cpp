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

#include "ability_manager_errors.h"
#include "dynamic_feature_manager.h"
#include "media_permission_manager.h"
#include "mock_dynamic_features.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AAFwk {

namespace {
constexpr uint32_t CALLER_TOKEN_ID = 100;
constexpr uint32_t TARGET_TOKEN_ID = 200;
constexpr uint32_t FLAG = 1;
constexpr int32_t HIDE_SENSITIVE_TYPE = 0;
const std::string URI_A = "file://media/Photo/100.jpg";
const std::string URI_B = "file://media/Photo/101.jpg";
const std::string URI_C = "file://media/Photo/102.jpg";
}

// MediaPermissionManager is a thin forwarder: each public method acquires the
// MEDIA feature via DynamicFeatureManager and delegates to IMediaPermFeature.
// These tests exercise both the feature-unavailable fallback path (nullptr
// scope -> degraded result) and the delegation path (return-value and
// parameter pass-through to the injected MockMediaPermFeature).
//
// The real media_permission_manager.cpp is linked from libupms_static; the
// feature plugin is replaced by MockMediaPermFeature injected straight into
// DynamicFeatureManager::GetInstance().registry_ (same technique as
// uri_permission_manager_stub_impl_test.cpp). private access to registry_ is
// enabled by the file-local #define private public above.
class MediaPermissionManagerTest : public testing::Test {
public:
    void SetUp() override
    {
        MockMediaPermFeature::Reset();
        ClearRegistry();
    }
    void TearDown() override
    {
        ClearRegistry();
    }

    // Wipes the shared singleton registry so each case starts with no feature
    // loaded; also cancels any pending per-feature idle-unload ffrt task so it
    // cannot fire into the next case.
    static void ClearRegistry()
    {
        auto &reg = DynamicFeatureManager::GetInstance().registry_;
        for (auto &[id, entry] : reg) {
            if (entry.unloadHandle.has_value()) {
                ffrt::skip(*entry.unloadHandle);
                entry.unloadHandle.reset();
            }
            entry.instance.reset(); // destroy == nullptr for injected mocks
            entry.loaded = false;
            entry.activeCount = 0;
        }
        reg.clear();
    }

    // Injects the fixture's mock as the MEDIA feature (loaded, zero active scopes).
    void InjectMediaFeature()
    {
        auto &reg = DynamicFeatureManager::GetInstance().registry_;
        auto &me = reg[FeatureId::MEDIA];
        me.destroy = nullptr; // NoOpDestroy: manager never deletes injected mock
        me.instance.reset(&mock_);
        me.loaded = true;
        me.activeCount = 0;
        me.unloadHandle.reset();
    }

    static MockMediaPermFeature mock_;
};

MockMediaPermFeature MediaPermissionManagerTest::mock_;

// ============================================================================
// GetInstance
// ============================================================================

/*
 * Feature: MediaPermissionManager
 * Function: GetInstance
 * FunctionPoints: returns the same singleton across calls
 */
HWTEST_F(MediaPermissionManagerTest, GetInstance_Singleton_001, TestSize.Level1)
{
    auto &a = MediaPermissionManager::GetInstance();
    auto &b = MediaPermissionManager::GetInstance();
    EXPECT_EQ(&a, &b);
}

// ============================================================================
// CheckUriPermission — feature unavailable fallback
// ============================================================================

/*
 * Feature: MediaPermissionManager
 * Function: CheckUriPermission
 * FunctionPoints: media feature not loaded -> all-false vector sized to input
 */
HWTEST_F(MediaPermissionManagerTest, CheckUriPermission_FeatureUnavailable_001, TestSize.Level1)
{
    // SetUp cleared the registry; MEDIA feature is not available.
    std::vector<std::string> uris = {URI_A, URI_B, URI_C};
    auto result = MediaPermissionManager::GetInstance().CheckUriPermission(uris, CALLER_TOKEN_ID, FLAG);
    EXPECT_EQ(result.size(), uris.size());
    for (auto v : result) {
        EXPECT_FALSE(v);
    }
}

/*
 * Feature: MediaPermissionManager
 * Function: CheckUriPermission
 * FunctionPoints: feature unavailable with empty input -> empty result (no crash)
 */
HWTEST_F(MediaPermissionManagerTest, CheckUriPermission_FeatureUnavailable_EmptyVec_001, TestSize.Level1)
{
    std::vector<std::string> uris;
    auto result = MediaPermissionManager::GetInstance().CheckUriPermission(uris, CALLER_TOKEN_ID, FLAG);
    EXPECT_TRUE(result.empty());
}

// ============================================================================
// CheckUriPermission — delegation to IMediaPermFeature
// ============================================================================

/*
 * Feature: MediaPermissionManager
 * Function: CheckUriPermission
 * FunctionPoints: forwards to feature; checkRet=true yields all-true
 */
HWTEST_F(MediaPermissionManagerTest, CheckUriPermission_Delegates_True_001, TestSize.Level1)
{
    InjectMediaFeature();
    MockMediaPermFeature::checkRet = true;
    std::vector<std::string> uris = {URI_A, URI_B};
    auto result = MediaPermissionManager::GetInstance().CheckUriPermission(uris, CALLER_TOKEN_ID, FLAG);
    ASSERT_EQ(result.size(), uris.size());
    for (auto v : result) {
        EXPECT_TRUE(v);
    }
}

/*
 * Feature: MediaPermissionManager
 * Function: CheckUriPermission
 * FunctionPoints: forwards to feature; checkRet=false yields all-false
 */
HWTEST_F(MediaPermissionManagerTest, CheckUriPermission_Delegates_False_001, TestSize.Level1)
{
    InjectMediaFeature();
    MockMediaPermFeature::checkRet = false;
    std::vector<std::string> uris = {URI_A, URI_B, URI_C};
    auto result = MediaPermissionManager::GetInstance().CheckUriPermission(uris, CALLER_TOKEN_ID, FLAG);
    ASSERT_EQ(result.size(), uris.size());
    for (auto v : result) {
        EXPECT_FALSE(v);
    }
}

/*
 * Feature: MediaPermissionManager
 * Function: CheckUriPermission
 * FunctionPoints: single-uri result mirrors checkRet
 */
HWTEST_F(MediaPermissionManagerTest, CheckUriPermission_Delegates_SingleUri_001, TestSize.Level1)
{
    InjectMediaFeature();
    MockMediaPermFeature::checkRet = true;
    std::vector<std::string> uris = {URI_A};
    auto result = MediaPermissionManager::GetInstance().CheckUriPermission(uris, CALLER_TOKEN_ID, FLAG);
    ASSERT_EQ(result.size(), 1u);
    EXPECT_TRUE(result[0]);
}

/*
 * Feature: MediaPermissionManager
 * Function: CheckUriPermission
 * FunctionPoints: empty input via feature -> empty result (pass-through size)
 */
HWTEST_F(MediaPermissionManagerTest, CheckUriPermission_Delegates_EmptyVec_001, TestSize.Level1)
{
    InjectMediaFeature();
    MockMediaPermFeature::checkRet = true;
    std::vector<std::string> uris;
    auto result = MediaPermissionManager::GetInstance().CheckUriPermission(uris, CALLER_TOKEN_ID, FLAG);
    EXPECT_TRUE(result.empty());
}

/*
 * Feature: MediaPermissionManager
 * Function: CheckUriPermission
 * FunctionPoints: arguments (uriVec, callerTokenId, flag) are passed through verbatim
 */
HWTEST_F(MediaPermissionManagerTest, CheckUriPermission_ParamPassthrough_001, TestSize.Level1)
{
    InjectMediaFeature();
    std::vector<std::string> uris = {URI_A, URI_B, URI_C};
    MediaPermissionManager::GetInstance().CheckUriPermission(uris, CALLER_TOKEN_ID, FLAG);
    EXPECT_EQ(MockMediaPermFeature::lastCheckUris, uris);
    EXPECT_EQ(MockMediaPermFeature::lastCheckCallerTokenId, CALLER_TOKEN_ID);
    EXPECT_EQ(MockMediaPermFeature::lastCheckFlag, FLAG);
}

// ============================================================================
// GrantUriPermission — feature unavailable fallback
// ============================================================================

/*
 * Feature: MediaPermissionManager
 * Function: GrantUriPermission
 * FunctionPoints: media feature not loaded -> INNER_ERR
 */
HWTEST_F(MediaPermissionManagerTest, GrantUriPermission_FeatureUnavailable_001, TestSize.Level1)
{
    std::vector<std::string> uris = {URI_A, URI_B};
    auto ret = MediaPermissionManager::GetInstance().GrantUriPermission(uris, FLAG, CALLER_TOKEN_ID,
        TARGET_TOKEN_ID, HIDE_SENSITIVE_TYPE);
    EXPECT_EQ(ret, INNER_ERR);
}

// ============================================================================
// GrantUriPermission — delegation to IMediaPermFeature
// ============================================================================

/*
 * Feature: MediaPermissionManager
 * Function: GrantUriPermission
 * FunctionPoints: forwards to feature; grantRet=ERR_OK -> ERR_OK
 */
HWTEST_F(MediaPermissionManagerTest, GrantUriPermission_Delegates_Success_001, TestSize.Level1)
{
    InjectMediaFeature();
    MockMediaPermFeature::grantRet = ERR_OK;
    std::vector<std::string> uris = {URI_A, URI_B};
    auto ret = MediaPermissionManager::GetInstance().GrantUriPermission(uris, FLAG, CALLER_TOKEN_ID,
        TARGET_TOKEN_ID, HIDE_SENSITIVE_TYPE);
    EXPECT_EQ(ret, ERR_OK);
}

/*
 * Feature: MediaPermissionManager
 * Function: GrantUriPermission
 * FunctionPoints: forwards to feature; grantRet=INNER_ERR -> INNER_ERR
 */
HWTEST_F(MediaPermissionManagerTest, GrantUriPermission_Delegates_Failure_001, TestSize.Level1)
{
    InjectMediaFeature();
    MockMediaPermFeature::grantRet = INNER_ERR;
    std::vector<std::string> uris = {URI_A};
    auto ret = MediaPermissionManager::GetInstance().GrantUriPermission(uris, FLAG, CALLER_TOKEN_ID,
        TARGET_TOKEN_ID, HIDE_SENSITIVE_TYPE);
    EXPECT_EQ(ret, INNER_ERR);
}

/*
 * Feature: MediaPermissionManager
 * Function: GrantUriPermission
 * FunctionPoints: arguments (uris, flag, callerTokenId, targetTokenId, hideSensitiveType) passed through
 */
HWTEST_F(MediaPermissionManagerTest, GrantUriPermission_ParamPassthrough_001, TestSize.Level1)
{
    InjectMediaFeature();
    std::vector<std::string> uris = {URI_A, URI_B, URI_C};
    MediaPermissionManager::GetInstance().GrantUriPermission(uris, FLAG, CALLER_TOKEN_ID, TARGET_TOKEN_ID,
        HIDE_SENSITIVE_TYPE);
    EXPECT_EQ(MockMediaPermFeature::lastGrantUris, uris);
    EXPECT_EQ(MockMediaPermFeature::lastGrantFlag, FLAG);
    EXPECT_EQ(MockMediaPermFeature::lastGrantCallerTokenId, CALLER_TOKEN_ID);
    EXPECT_EQ(MockMediaPermFeature::lastGrantTargetTokenId, TARGET_TOKEN_ID);
    EXPECT_EQ(MockMediaPermFeature::lastGrantHideSensitiveType, HIDE_SENSITIVE_TYPE);
}

// ============================================================================
// RevokeUriPermission — feature unavailable fallback
// ============================================================================

/*
 * Feature: MediaPermissionManager
 * Function: RevokeUriPermission
 * FunctionPoints: media feature not loaded -> INNER_ERR
 */
HWTEST_F(MediaPermissionManagerTest, RevokeUriPermission_FeatureUnavailable_001, TestSize.Level1)
{
    auto ret = MediaPermissionManager::GetInstance().RevokeUriPermission(CALLER_TOKEN_ID, TARGET_TOKEN_ID, URI_A);
    EXPECT_EQ(ret, INNER_ERR);
}

// ============================================================================
// RevokeUriPermission — delegation to IMediaPermFeature
// ============================================================================

/*
 * Feature: MediaPermissionManager
 * Function: RevokeUriPermission
 * FunctionPoints: forwards to feature; revokeRet=ERR_OK -> ERR_OK
 */
HWTEST_F(MediaPermissionManagerTest, RevokeUriPermission_Delegates_Success_001, TestSize.Level1)
{
    InjectMediaFeature();
    MockMediaPermFeature::revokeRet = ERR_OK;
    auto ret = MediaPermissionManager::GetInstance().RevokeUriPermission(CALLER_TOKEN_ID, TARGET_TOKEN_ID, URI_A);
    EXPECT_EQ(ret, ERR_OK);
}

/*
 * Feature: MediaPermissionManager
 * Function: RevokeUriPermission
 * FunctionPoints: forwards to feature; revokeRet=INNER_ERR -> INNER_ERR
 */
HWTEST_F(MediaPermissionManagerTest, RevokeUriPermission_Delegates_Failure_001, TestSize.Level1)
{
    InjectMediaFeature();
    MockMediaPermFeature::revokeRet = INNER_ERR;
    auto ret = MediaPermissionManager::GetInstance().RevokeUriPermission(CALLER_TOKEN_ID, TARGET_TOKEN_ID, URI_A);
    EXPECT_EQ(ret, INNER_ERR);
}

/*
 * Feature: MediaPermissionManager
 * Function: RevokeUriPermission
 * FunctionPoints: arguments (callerTokenId, targetTokenId, uri) passed through
 */
HWTEST_F(MediaPermissionManagerTest, RevokeUriPermission_ParamPassthrough_001, TestSize.Level1)
{
    InjectMediaFeature();
    MediaPermissionManager::GetInstance().RevokeUriPermission(CALLER_TOKEN_ID, TARGET_TOKEN_ID, URI_A);
    EXPECT_EQ(MockMediaPermFeature::lastRevokeCallerTokenId, CALLER_TOKEN_ID);
    EXPECT_EQ(MockMediaPermFeature::lastRevokeTargetTokenId, TARGET_TOKEN_ID);
    EXPECT_EQ(MockMediaPermFeature::lastRevokeUri, URI_A);
}

// ============================================================================
// RAII interaction: scope released after each call arms idle-unload
// ============================================================================

/*
 * Feature: MediaPermissionManager
 * Function: CheckUriPermission
 * FunctionPoints: after a delegated call the feature scope is released (activeCount back to 0)
 */
HWTEST_F(MediaPermissionManagerTest, Scope_Released_AfterCall_001, TestSize.Level1)
{
    InjectMediaFeature();
    auto &me = DynamicFeatureManager::GetInstance().registry_[FeatureId::MEDIA];
    EXPECT_EQ(me.activeCount, 0);
    std::vector<std::string> uris = {URI_A};
    MediaPermissionManager::GetInstance().CheckUriPermission(uris, CALLER_TOKEN_ID, FLAG);
    EXPECT_EQ(me.activeCount, 0); // scope destroyed at call return
}

}  // namespace AAFwk
}  // namespace OHOS
