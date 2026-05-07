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

#include "modular_object_utils.h"
#include "ability_manager_errors.h"
#include "ability_record.h"
#include "base_extension_record.h"
#include "ability_record/ability_record_utils.h"
#include "ipc_object_stub.h"
#include "mock_flag.h"

using namespace testing::ext;
using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;

namespace {
void ResetFlags()
{
    MockFlag::isSupportModularObjectExtension = true;
    MockFlag::callingUid = 1000;
    MockFlag::callingPid = 1234;
    MockFlag::getRunningProcessInfoRet = 0;
    MockFlag::processState = 2; // APP_STATE_FOREGROUND
    MockFlag::isPreForeground = false;
    MockFlag::isDeveloperMode = false;
    MockFlag::queryDataRet = 0;
    MockFlag::extensionFound = true;
    MockFlag::extensionDisabled = false;
    MockFlag::bundleMgrHelperNull = false;
    MockFlag::getNameAndIndexRet = 0;
    MockFlag::getOsAccountRet = 0;
    MockFlag::getApplicationInfoRet = true;
    MockFlag::amsNull = false;
    MockFlag::isSceneBoardEnabled = true;
    MockFlag::hasRunningUIAbility = true;
    MockFlag::hasRunningUIExtension = false;
    MockFlag::missionListMgrNull = false;
    MockFlag::uiAbilityMgrNull = false;
    MockFlag::uiExtMgrNull = false;
    MockFlag::isSACall = false;
    MockFlag::modularObjectLimited = false;
    MockFlag::checkCallModularObjectExtensionPermissionRet = 0;
    // Token and AbilityRecord mock state
    Token::abilityRecord_ = nullptr;
    AbilityRecord::abilityInfo.extensionAbilityType = ExtensionAbilityType::UNSPECIFIED;
    BaseExtensionRecord::clientPid = -1;
}
} // namespace

class ModularObjectUtilsTest : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
    void SetUp() override { ResetFlags(); }
    void TearDown() override {}
};

// ==================== CheckRateLimit ====================

HWTEST_F(ModularObjectUtilsTest, CheckRateLimit_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckRateLimit_001 start";
    // SA call should bypass rate limit
    MockFlag::isSACall = true;
    MockFlag::modularObjectLimited = true;
    auto ret = ModularObjectUtils::CheckRateLimit();
    EXPECT_EQ(ret, ERR_OK);
    GTEST_LOG_(INFO) << "CheckRateLimit_001 end";
}

HWTEST_F(ModularObjectUtilsTest, CheckRateLimit_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckRateLimit_002 start";
    // Not SA call, not limited
    MockFlag::isSACall = false;
    MockFlag::modularObjectLimited = false;
    auto ret = ModularObjectUtils::CheckRateLimit();
    EXPECT_EQ(ret, ERR_OK);
    GTEST_LOG_(INFO) << "CheckRateLimit_002 end";
}

HWTEST_F(ModularObjectUtilsTest, CheckRateLimit_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckRateLimit_003 start";
    // Not SA call, rate limited
    MockFlag::isSACall = false;
    MockFlag::modularObjectLimited = true;
    auto ret = ModularObjectUtils::CheckRateLimit();
    EXPECT_EQ(ret, ERR_FREQ_START_ABILITY);
    GTEST_LOG_(INFO) << "CheckRateLimit_003 end";
}

// ==================== VerifyExported ====================

HWTEST_F(ModularObjectUtilsTest, VerifyExported_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VerifyExported_001 start";
    // Permission check pass
    MockFlag::checkCallModularObjectExtensionPermissionRet = 0;
    AbilityRequest request;
    auto ret = ModularObjectUtils::VerifyExported(request);
    EXPECT_EQ(ret, ERR_OK);
    GTEST_LOG_(INFO) << "VerifyExported_001 end";
}

HWTEST_F(ModularObjectUtilsTest, VerifyExported_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VerifyExported_002 start";
    // Permission check fail
    MockFlag::checkCallModularObjectExtensionPermissionRet = ABILITY_VISIBLE_FALSE_DENY_REQUEST;
    AbilityRequest request;
    auto ret = ModularObjectUtils::VerifyExported(request);
    EXPECT_EQ(ret, ABILITY_VISIBLE_FALSE_DENY_REQUEST);
    GTEST_LOG_(INFO) << "VerifyExported_002 end";
}

// ==================== CheckPermission ====================

HWTEST_F(ModularObjectUtilsTest, CheckPermission_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckPermission_001 start";
    // Device not supported
    MockFlag::isSupportModularObjectExtension = false;
    AbilityRequest request;
    auto ret = ModularObjectUtils::CheckPermission(request);
    EXPECT_EQ(ret, ERR_CAPABILITY_NOT_SUPPORT);
    GTEST_LOG_(INFO) << "CheckPermission_001 end";
}

HWTEST_F(ModularObjectUtilsTest, CheckPermission_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckPermission_002 start";
    // GetTargetExtensionInfoFromDb fails
    MockFlag::queryDataRet = -1;
    AbilityRequest request;
    auto ret = ModularObjectUtils::CheckPermission(request);
    EXPECT_EQ(ret, -1);
    GTEST_LOG_(INFO) << "CheckPermission_002 end";
}

HWTEST_F(ModularObjectUtilsTest, CheckPermission_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckPermission_003 start";
    // Extension not found in db
    MockFlag::extensionFound = false;
    AbilityRequest request;
    auto ret = ModularObjectUtils::CheckPermission(request);
    EXPECT_EQ(ret, RESOLVE_ABILITY_ERR);
    GTEST_LOG_(INFO) << "CheckPermission_003 end";
}

HWTEST_F(ModularObjectUtilsTest, CheckPermission_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckPermission_004 start";
    // Extension disabled and different uid
    MockFlag::extensionFound = true;
    MockFlag::extensionDisabled = true;
    MockFlag::callingUid = 999; // different from request.uid
    AbilityRequest request;
    request.uid = 100;
    request.want.SetElement(ElementName("", "com.test.bundle", "TestAbility"));
    auto ret = ModularObjectUtils::CheckPermission(request);
    EXPECT_EQ(ret, ERR_MODULAR_OBJECT_DISABLED);
    GTEST_LOG_(INFO) << "CheckPermission_004 end";
}

HWTEST_F(ModularObjectUtilsTest, CheckPermission_005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckPermission_005 start";
    // Caller not foreground
    MockFlag::processState = 4; // APP_STATE_BACKGROUND
    AbilityRequest request;
    request.uid = 100;
    request.want.SetElement(ElementName("", "com.test.bundle", "TestAbility"));
    auto ret = ModularObjectUtils::CheckPermission(request);
    EXPECT_EQ(ret, NOT_TOP_ABILITY);
    GTEST_LOG_(INFO) << "CheckPermission_005 end";
}

HWTEST_F(ModularObjectUtilsTest, CheckPermission_006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckPermission_006 start";
    // Caller is preForeground
    MockFlag::isPreForeground = true;
    AbilityRequest request;
    request.uid = 100;
    request.want.SetElement(ElementName("", "com.test.bundle", "TestAbility"));
    auto ret = ModularObjectUtils::CheckPermission(request);
    EXPECT_EQ(ret, NOT_TOP_ABILITY);
    GTEST_LOG_(INFO) << "CheckPermission_006 end";
}

HWTEST_F(ModularObjectUtilsTest, CheckPermission_007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckPermission_007 start";
    // GetCallerAppInfo fails - BundleMgrHelper GetNameAndIndexForUid fails
    MockFlag::getNameAndIndexRet = -1;
    AbilityRequest request;
    request.uid = 100;
    request.want.SetElement(ElementName("", "com.test.bundle", "TestAbility"));
    auto ret = ModularObjectUtils::CheckPermission(request);
    EXPECT_EQ(ret, INNER_ERR);
    GTEST_LOG_(INFO) << "CheckPermission_007 end";
}

HWTEST_F(ModularObjectUtilsTest, CheckPermission_008, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckPermission_008 start";
    // Distribution type check - caller is "none", not developer mode
    MockFlag::isDeveloperMode = false;
    AbilityRequest request;
    request.uid = 1000;
    request.want.SetElement(ElementName("", "com.test.bundle", "TestAbility"));
    request.appInfo.appDistributionType = "none";
    auto ret = ModularObjectUtils::CheckPermission(request);
    EXPECT_EQ(ret, ERR_INVALID_DISTRIBUTION_TYPE);
    GTEST_LOG_(INFO) << "CheckPermission_008 end";
}

HWTEST_F(ModularObjectUtilsTest, CheckPermission_009, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckPermission_009 start";
    // Distribution type check - target is "none"
    MockFlag::isDeveloperMode = false;
    MockFlag::hasRunningUIAbility = false;
    AbilityRequest request;
    request.uid = 1000;
    request.want.SetElement(ElementName("", "com.test.bundle", "TestAbility"));
    request.appInfo.appDistributionType = "debug";
    auto ret = ModularObjectUtils::CheckPermission(request);
    EXPECT_EQ(ret, ERR_NO_RUNNING_ABILITIES_WITH_UI);
    GTEST_LOG_(INFO) << "CheckPermission_009 end";
}

HWTEST_F(ModularObjectUtilsTest, CheckPermission_010, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckPermission_010 start";
    // Target has no running ability
    MockFlag::hasRunningUIAbility = false;
    MockFlag::hasRunningUIExtension = false;
    AbilityRequest request;
    request.uid = 1000;
    request.want.SetElement(ElementName("", "com.test.bundle", "TestAbility"));
    auto ret = ModularObjectUtils::CheckPermission(request);
    EXPECT_EQ(ret, ERR_NO_RUNNING_ABILITIES_WITH_UI);
    GTEST_LOG_(INFO) << "CheckPermission_010 end";
}

// ==================== CheckAppDistributionType ====================

HWTEST_F(ModularObjectUtilsTest, CheckAppDistributionType_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckAppDistributionType_001 start";
    // Developer mode - allow
    MockFlag::isDeveloperMode = true;
    auto ret = ModularObjectUtils::CheckAppDistributionType("none", "none");
    EXPECT_EQ(ret, ERR_OK);
    GTEST_LOG_(INFO) << "CheckAppDistributionType_001 end";
}

HWTEST_F(ModularObjectUtilsTest, CheckAppDistributionType_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckAppDistributionType_002 start";
    // Caller distribution type is "none"
    MockFlag::isDeveloperMode = false;
    auto ret = ModularObjectUtils::CheckAppDistributionType("none", "debug");
    EXPECT_EQ(ret, ERR_INVALID_DISTRIBUTION_TYPE);
    GTEST_LOG_(INFO) << "CheckAppDistributionType_002 end";
}

HWTEST_F(ModularObjectUtilsTest, CheckAppDistributionType_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckAppDistributionType_003 start";
    // Target distribution type is "none"
    MockFlag::isDeveloperMode = false;
    auto ret = ModularObjectUtils::CheckAppDistributionType("debug", "none");
    EXPECT_EQ(ret, ERR_INVALID_DISTRIBUTION_TYPE);
    GTEST_LOG_(INFO) << "CheckAppDistributionType_003 end";
}

HWTEST_F(ModularObjectUtilsTest, CheckAppDistributionType_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckAppDistributionType_004 start";
    // Normal - both have valid distribution types
    MockFlag::isDeveloperMode = false;
    auto ret = ModularObjectUtils::CheckAppDistributionType("debug", "release");
    EXPECT_EQ(ret, ERR_OK);
    GTEST_LOG_(INFO) << "CheckAppDistributionType_004 end";
}

// ==================== CheckExtensionEnabled ====================

HWTEST_F(ModularObjectUtilsTest, CheckExtensionEnabled_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckExtensionEnabled_001 start";
    // Disabled and different uid
    MockFlag::callingUid = 999;
    ModularObjectExtensionInfo info;
    info.isDisabled = true;
    info.bundleName = "com.test";
    info.abilityName = "TestAbility";
    AbilityRequest request;
    request.uid = 100;
    auto ret = ModularObjectUtils::CheckExtensionEnabled(info, request);
    EXPECT_EQ(ret, ERR_MODULAR_OBJECT_DISABLED);
    GTEST_LOG_(INFO) << "CheckExtensionEnabled_001 end";
}

HWTEST_F(ModularObjectUtilsTest, CheckExtensionEnabled_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckExtensionEnabled_002 start";
    // Disabled but same uid - should pass
    MockFlag::callingUid = 100;
    ModularObjectExtensionInfo info;
    info.isDisabled = true;
    info.bundleName = "com.test";
    info.abilityName = "TestAbility";
    AbilityRequest request;
    request.uid = 100;
    auto ret = ModularObjectUtils::CheckExtensionEnabled(info, request);
    EXPECT_EQ(ret, ERR_OK);
    GTEST_LOG_(INFO) << "CheckExtensionEnabled_002 end";
}

HWTEST_F(ModularObjectUtilsTest, CheckExtensionEnabled_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckExtensionEnabled_003 start";
    // Not disabled - should pass regardless of uid
    MockFlag::callingUid = 999;
    ModularObjectExtensionInfo info;
    info.isDisabled = false;
    info.bundleName = "com.test";
    info.abilityName = "TestAbility";
    AbilityRequest request;
    request.uid = 100;
    auto ret = ModularObjectUtils::CheckExtensionEnabled(info, request);
    EXPECT_EQ(ret, ERR_OK);
    GTEST_LOG_(INFO) << "CheckExtensionEnabled_003 end";
}

// ==================== CheckCallerForeground ====================

HWTEST_F(ModularObjectUtilsTest, CheckCallerForeground_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckCallerForeground_001 start";
    // GetRunningProcessInfoByPid fails
    MockFlag::getRunningProcessInfoRet = -1;
    auto ret = ModularObjectUtils::CheckCallerForeground();
    EXPECT_EQ(ret, -1);
    GTEST_LOG_(INFO) << "CheckCallerForeground_001 end";
}

HWTEST_F(ModularObjectUtilsTest, CheckCallerForeground_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckCallerForeground_002 start";
    // Caller not foreground (background state)
    MockFlag::processState = 4; // APP_STATE_BACKGROUND
    auto ret = ModularObjectUtils::CheckCallerForeground();
    EXPECT_EQ(ret, NOT_TOP_ABILITY);
    GTEST_LOG_(INFO) << "CheckCallerForeground_002 end";
}

HWTEST_F(ModularObjectUtilsTest, CheckCallerForeground_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckCallerForeground_003 start";
    // Caller is preForeground
    MockFlag::isPreForeground = true;
    auto ret = ModularObjectUtils::CheckCallerForeground();
    EXPECT_EQ(ret, NOT_TOP_ABILITY);
    GTEST_LOG_(INFO) << "CheckCallerForeground_003 end";
}

HWTEST_F(ModularObjectUtilsTest, CheckCallerForeground_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckCallerForeground_004 start";
    // Caller is foreground
    MockFlag::processState = 2; // APP_STATE_FOREGROUND
    MockFlag::isPreForeground = false;
    auto ret = ModularObjectUtils::CheckCallerForeground();
    EXPECT_EQ(ret, ERR_OK);
    GTEST_LOG_(INFO) << "CheckCallerForeground_004 end";
}

HWTEST_F(ModularObjectUtilsTest, HasRunningUIAbilityOrExtension_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HasRunningUIAbilityOrExtension_001 start";
    // SceneBoard enabled with running UIAbility
    MockFlag::isSceneBoardEnabled = true;
    MockFlag::hasRunningUIAbility = true;
    auto ret = ModularObjectUtils::HasRunningUIAbilityOrExtension(1000, 100);
    EXPECT_TRUE(ret);
    GTEST_LOG_(INFO) << "HasRunningUIAbilityOrExtension_001 end";
}

HWTEST_F(ModularObjectUtilsTest, HasRunningUIAbilityOrExtension_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HasRunningUIAbilityOrExtension_002 start";
    // SceneBoard enabled, no UIAbility but has UIExtension
    MockFlag::isSceneBoardEnabled = true;
    MockFlag::hasRunningUIAbility = false;
    MockFlag::hasRunningUIExtension = true;
    MockFlag::uiExtMgrNull = false;
    auto ret = ModularObjectUtils::HasRunningUIAbilityOrExtension(1000, 100);
    EXPECT_TRUE(ret);
    GTEST_LOG_(INFO) << "HasRunningUIAbilityOrExtension_002 end";
}

HWTEST_F(ModularObjectUtilsTest, HasRunningUIAbilityOrExtension_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HasRunningUIAbilityOrExtension_003 start";
    // No running abilities at all
    MockFlag::isSceneBoardEnabled = true;
    MockFlag::hasRunningUIAbility = false;
    MockFlag::hasRunningUIExtension = false;
    MockFlag::uiExtMgrNull = false;
    auto ret = ModularObjectUtils::HasRunningUIAbilityOrExtension(1000, 100);
    EXPECT_FALSE(ret);
    GTEST_LOG_(INFO) << "HasRunningUIAbilityOrExtension_003 end";
}

HWTEST_F(ModularObjectUtilsTest, HasRunningUIAbilityOrExtension_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HasRunningUIAbilityOrExtension_004 start";
    // SceneBoard disabled with running ability via MissionListManager
    MockFlag::isSceneBoardEnabled = false;
    MockFlag::hasRunningUIAbility = true;
    MockFlag::missionListMgrNull = false;
    auto ret = ModularObjectUtils::HasRunningUIAbilityOrExtension(1000, 100);
    EXPECT_TRUE(ret);
    GTEST_LOG_(INFO) << "HasRunningUIAbilityOrExtension_004 end";
}

// ==================== CheckTargetHasRunningAbility ====================

HWTEST_F(ModularObjectUtilsTest, CheckTargetHasRunningAbility_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckTargetHasRunningAbility_001 start";
    // No running ability
    MockFlag::hasRunningUIAbility = false;
    MockFlag::hasRunningUIExtension = false;
    auto ret = ModularObjectUtils::CheckTargetHasRunningAbility(1000, 100, "com.test");
    EXPECT_EQ(ret, ERR_NO_RUNNING_ABILITIES_WITH_UI);
    GTEST_LOG_(INFO) << "CheckTargetHasRunningAbility_001 end";
}

HWTEST_F(ModularObjectUtilsTest, CheckTargetHasRunningAbility_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckTargetHasRunningAbility_002 start";
    // Has running ability
    MockFlag::hasRunningUIAbility = true;
    auto ret = ModularObjectUtils::CheckTargetHasRunningAbility(1000, 100, "com.test");
    EXPECT_EQ(ret, ERR_OK);
    GTEST_LOG_(INFO) << "CheckTargetHasRunningAbility_002 end";
}

// ==================== GetTargetExtensionInfoFromDb ====================

HWTEST_F(ModularObjectUtilsTest, GetTargetExtensionInfoFromDb_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetTargetExtensionInfoFromDb_001 start";
    // QueryData fails
    MockFlag::queryDataRet = -1;
    ModularObjectExtensionInfo targetInfo;
    auto ret = ModularObjectUtils::GetTargetExtensionInfoFromDb("bundle", "ability", 0, 100, targetInfo);
    EXPECT_EQ(ret, -1);
    GTEST_LOG_(INFO) << "GetTargetExtensionInfoFromDb_001 end";
}

HWTEST_F(ModularObjectUtilsTest, GetTargetExtensionInfoFromDb_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetTargetExtensionInfoFromDb_002 start";
    // Extension not found
    MockFlag::extensionFound = false;
    ModularObjectExtensionInfo targetInfo;
    auto ret = ModularObjectUtils::GetTargetExtensionInfoFromDb("bundle", "ability", 0, 100, targetInfo);
    EXPECT_EQ(ret, RESOLVE_ABILITY_ERR);
    GTEST_LOG_(INFO) << "GetTargetExtensionInfoFromDb_002 end";
}

HWTEST_F(ModularObjectUtilsTest, GetTargetExtensionInfoFromDb_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetTargetExtensionInfoFromDb_003 start";
    // Success - extension found
    MockFlag::extensionFound = true;
    ModularObjectExtensionInfo targetInfo;
    auto ret = ModularObjectUtils::GetTargetExtensionInfoFromDb(
        "com.test.bundle", "TestAbility", 0, 100, targetInfo);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(targetInfo.bundleName, "com.test.bundle");
    EXPECT_EQ(targetInfo.abilityName, "TestAbility");
    GTEST_LOG_(INFO) << "GetTargetExtensionInfoFromDb_003 end";
}

// ==================== GetCallerAppInfo ====================

HWTEST_F(ModularObjectUtilsTest, GetCallerAppInfo_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetCallerAppInfo_001 start";
    // BundleMgrHelper is null - can't easily test since singleton always returns instance
    // GetNameAndIndexForUid fails
    MockFlag::getNameAndIndexRet = -1;
    ApplicationInfo callerAppInfo;
    auto ret = ModularObjectUtils::GetCallerAppInfo(callerAppInfo);
    EXPECT_EQ(ret, INNER_ERR);
    GTEST_LOG_(INFO) << "GetCallerAppInfo_001 end";
}

HWTEST_F(ModularObjectUtilsTest, GetCallerAppInfo_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetCallerAppInfo_002 start";
    // GetOsAccountLocalIdFromUid fails
    MockFlag::getOsAccountRet = -1;
    ApplicationInfo callerAppInfo;
    auto ret = ModularObjectUtils::GetCallerAppInfo(callerAppInfo);
    EXPECT_EQ(ret, INNER_ERR);
    GTEST_LOG_(INFO) << "GetCallerAppInfo_002 end";
}

HWTEST_F(ModularObjectUtilsTest, GetCallerAppInfo_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetCallerAppInfo_003 start";
    // GetApplicationInfoWithAppIndex fails
    MockFlag::getApplicationInfoRet = false;
    ApplicationInfo callerAppInfo;
    auto ret = ModularObjectUtils::GetCallerAppInfo(callerAppInfo);
    EXPECT_EQ(ret, INNER_ERR);
    GTEST_LOG_(INFO) << "GetCallerAppInfo_003 end";
}

HWTEST_F(ModularObjectUtilsTest, GetCallerAppInfo_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetCallerAppInfo_004 start";
    // Success
    ApplicationInfo callerAppInfo;
    auto ret = ModularObjectUtils::GetCallerAppInfo(callerAppInfo);
    EXPECT_EQ(ret, ERR_OK);
    GTEST_LOG_(INFO) << "GetCallerAppInfo_004 end";
}

// ==================== GetPidToCheckByCallerToken ====================

HWTEST_F(ModularObjectUtilsTest, GetPidToCheckByCallerToken_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetPidToCheckByCallerToken_001 start";
    // callerToken is nullptr
    pid_t outPid = -1;
    auto ret = ModularObjectUtils::GetPidToCheckByCallerToken(nullptr, outPid);
    EXPECT_FALSE(ret);
    EXPECT_EQ(outPid, -1);
    GTEST_LOG_(INFO) << "GetPidToCheckByCallerToken_001 end";
}

HWTEST_F(ModularObjectUtilsTest, GetPidToCheckByCallerToken_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetPidToCheckByCallerToken_002 start";
    // callerToken valid but GetAbilityRecordByToken returns nullptr
    Token::abilityRecord_ = nullptr;
    pid_t outPid = -1;
    OHOS::sptr<OHOS::IRemoteObject> token(new OHOS::IPCObjectStub());
    auto ret = ModularObjectUtils::GetPidToCheckByCallerToken(token, outPid);
    EXPECT_FALSE(ret);
    EXPECT_EQ(outPid, -1);
    GTEST_LOG_(INFO) << "GetPidToCheckByCallerToken_002 end";
}

HWTEST_F(ModularObjectUtilsTest, GetPidToCheckByCallerToken_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetPidToCheckByCallerToken_003 start";
    // extensionAbilityType is not MODULAR_OBJECT
    Token::abilityRecord_ = std::make_shared<AbilityRecord>();
    AbilityRecord::abilityInfo.extensionAbilityType = ExtensionAbilityType::SERVICE;
    BaseExtensionRecord::clientPid = 5678;
    pid_t outPid = -1;
    OHOS::sptr<OHOS::IRemoteObject> token(new OHOS::IPCObjectStub());
    auto ret = ModularObjectUtils::GetPidToCheckByCallerToken(token, outPid);
    EXPECT_FALSE(ret);
    EXPECT_EQ(outPid, -1);
    GTEST_LOG_(INFO) << "GetPidToCheckByCallerToken_003 end";
}

HWTEST_F(ModularObjectUtilsTest, GetPidToCheckByCallerToken_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetPidToCheckByCallerToken_004 start";
    // extensionAbilityType is MODULAR_OBJECT but clientPid <= 0
    Token::abilityRecord_ = std::make_shared<BaseExtensionRecord>();
    AbilityRecord::abilityInfo.extensionAbilityType = ExtensionAbilityType::MODULAR_OBJECT;
    BaseExtensionRecord::clientPid = 0;
    pid_t outPid = -1;
    OHOS::sptr<OHOS::IRemoteObject> token(new OHOS::IPCObjectStub());
    auto ret = ModularObjectUtils::GetPidToCheckByCallerToken(token, outPid);
    EXPECT_FALSE(ret);
    EXPECT_EQ(outPid, -1);
    GTEST_LOG_(INFO) << "GetPidToCheckByCallerToken_004 end";
}

HWTEST_F(ModularObjectUtilsTest, GetPidToCheckByCallerToken_005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetPidToCheckByCallerToken_005 start";
    // extensionAbilityType is MODULAR_OBJECT and clientPid < 0
    Token::abilityRecord_ = std::make_shared<BaseExtensionRecord>();
    AbilityRecord::abilityInfo.extensionAbilityType = ExtensionAbilityType::MODULAR_OBJECT;
    BaseExtensionRecord::clientPid = -100;
    pid_t outPid = -1;
    OHOS::sptr<OHOS::IRemoteObject> token(new OHOS::IPCObjectStub());
    auto ret = ModularObjectUtils::GetPidToCheckByCallerToken(token, outPid);
    EXPECT_FALSE(ret);
    EXPECT_EQ(outPid, -1);
    GTEST_LOG_(INFO) << "GetPidToCheckByCallerToken_005 end";
}

HWTEST_F(ModularObjectUtilsTest, GetPidToCheckByCallerToken_006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetPidToCheckByCallerToken_006 start";
    // Success: MODULAR_OBJECT type and valid clientPid
    Token::abilityRecord_ = std::make_shared<BaseExtensionRecord>();
    AbilityRecord::abilityInfo.extensionAbilityType = ExtensionAbilityType::MODULAR_OBJECT;
    BaseExtensionRecord::clientPid = 5678;
    pid_t outPid = -1;
    OHOS::sptr<OHOS::IRemoteObject> token(new OHOS::IPCObjectStub());
    auto ret = ModularObjectUtils::GetPidToCheckByCallerToken(token, outPid);
    EXPECT_TRUE(ret);
    EXPECT_EQ(outPid, 5678);
    GTEST_LOG_(INFO) << "GetPidToCheckByCallerToken_006 end";
}
