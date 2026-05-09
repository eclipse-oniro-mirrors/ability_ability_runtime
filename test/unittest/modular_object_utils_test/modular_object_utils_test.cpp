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
    MockFlag::launchMode = MoeLaunchMode::CROSS_PROCESS;
    MockFlag::callerBundleName = "com.caller.bundle";
    MockFlag::processName = "";
    MockFlag::querySelfModularObjectRet = 0;
    MockFlag::modularObjectInfos.clear();
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

// ==================== CheckPermission IN_PROCESS tests ====================

/**
 * @tc.name: CheckPermission_ShouldReturnErrorWhenInProcessCrossApp
 * @tc.desc: IN_PROCESS with different uid returns ERR_MOE_CROSS_APP_IN_PROCESS
 */
HWTEST_F(ModularObjectUtilsTest,
    CheckPermission_ShouldReturnErrorWhenInProcessCrossApp, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckPermission_ShouldReturnErrorWhenInProcessCrossApp start";
    MockFlag::launchMode = MoeLaunchMode::IN_PROCESS;
    // callingUid is 1000 (from ResetFlags), request.uid is 100 (different)
    AbilityRequest request;
    request.uid = 100;
    request.want.SetElement(ElementName("", "com.test.bundle", "TestAbility"));
    auto ret = ModularObjectUtils::CheckPermission(request);
    EXPECT_EQ(ret, ERR_MOE_CROSS_APP_IN_PROCESS);
    GTEST_LOG_(INFO) << "CheckPermission_ShouldReturnErrorWhenInProcessCrossApp end";
}

/**
 * @tc.name: CheckPermission_ShouldReturnOkWhenInProcessSameUid
 * @tc.desc: IN_PROCESS with same uid passes and reaches foreground check
 */
HWTEST_F(ModularObjectUtilsTest,
    CheckPermission_ShouldReturnOkWhenInProcessSameUid, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckPermission_ShouldReturnOkWhenInProcessSameUid start";
    MockFlag::launchMode = MoeLaunchMode::IN_PROCESS;
    // callingUid is 1000, set request.uid to match
    AbilityRequest request;
    request.uid = 1000;
    request.want.SetElement(ElementName("", "com.test.bundle", "TestAbility"));
    // Will fail at CheckCallerForeground, but passes IN_PROCESS check
    MockFlag::getRunningProcessInfoRet = -1;
    auto ret = ModularObjectUtils::CheckPermission(request);
    EXPECT_EQ(ret, -1);
    GTEST_LOG_(INFO) << "CheckPermission_ShouldReturnOkWhenInProcessSameUid end";
}

/**
 * @tc.name: CheckPermission_ShouldReturnOkWhenCrossProcessDifferentCaller
 * @tc.desc: CROSS_PROCESS mode skips IN_PROCESS check, allows different caller
 */
HWTEST_F(ModularObjectUtilsTest,
    CheckPermission_ShouldReturnOkWhenCrossProcessDifferentCaller, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckPermission_ShouldReturnOkWhenCrossProcessDifferentCaller start";
    MockFlag::launchMode = MoeLaunchMode::CROSS_PROCESS;
    // Default callerBundleName differs from target, but CROSS_PROCESS skips check
    AbilityRequest request;
    request.uid = 100;
    request.want.SetElement(ElementName("", "com.test.bundle", "TestAbility"));
    MockFlag::getRunningProcessInfoRet = -1;
    auto ret = ModularObjectUtils::CheckPermission(request);
    EXPECT_EQ(ret, -1);
    GTEST_LOG_(INFO) << "CheckPermission_ShouldReturnOkWhenCrossProcessDifferentCaller end";
}

// ==================== CheckInProcessLaunchMode ====================

/**
 * @tc.name: CheckInProcessLaunchMode_ShouldReturnErrorWhenGetNameFail
 * @tc.desc: GetNameAndIndexForUid fails returns INNER_ERR
 */
HWTEST_F(ModularObjectUtilsTest,
    CheckInProcessLaunchMode_ShouldReturnErrorWhenCallerUidDiffersFromTarget, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckInProcessLaunchMode_ShouldReturnErrorWhenCallerUidDiffersFromTarget start";
    // callingUid is 1000 (from ResetFlags), targetUid is different
    auto ret = ModularObjectUtils::CheckInProcessLaunchMode(MoeLaunchMode::IN_PROCESS, 2000);
    EXPECT_EQ(ret, ERR_MOE_CROSS_APP_IN_PROCESS);
    GTEST_LOG_(INFO) << "CheckInProcessLaunchMode_ShouldReturnErrorWhenCallerUidDiffersFromTarget end";
}

/**
 * @tc.name: CheckInProcessLaunchMode_ShouldReturnOkWhenCallerUidMatchesTarget
 * @tc.desc: Caller uid matches target uid returns ERR_OK
 */
HWTEST_F(ModularObjectUtilsTest,
    CheckInProcessLaunchMode_ShouldReturnOkWhenCallerUidMatchesTarget, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckInProcessLaunchMode_ShouldReturnOkWhenCallerUidMatchesTarget start";
    // callingUid is 1000 (from ResetFlags)
    auto ret = ModularObjectUtils::CheckInProcessLaunchMode(MoeLaunchMode::IN_PROCESS, 1000);
    EXPECT_EQ(ret, ERR_OK);
    GTEST_LOG_(INFO) << "CheckInProcessLaunchMode_ShouldReturnOkWhenCallerUidMatchesTarget end";
}

/**
 * @tc.name: CheckInProcessLaunchMode_ShouldReturnOkWhenCrossProcessMode
 * @tc.desc: CROSS_PROCESS mode skips the check and returns ERR_OK
 */
HWTEST_F(ModularObjectUtilsTest,
    CheckInProcessLaunchMode_ShouldReturnOkWhenCrossProcessMode, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckInProcessLaunchMode_ShouldReturnOkWhenCrossProcessMode start";
    // CROSS_PROCESS mode skips check regardless of uid
    auto ret = ModularObjectUtils::CheckInProcessLaunchMode(MoeLaunchMode::CROSS_PROCESS, 9999);
    EXPECT_EQ(ret, ERR_OK);
    GTEST_LOG_(INFO) << "CheckInProcessLaunchMode_ShouldReturnOkWhenCrossProcessMode end";
}

// ==================== CheckLimits ====================

HWTEST_F(ModularObjectUtilsTest, CheckLimits_ShouldReturnOkWhenBothUnderLimit, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckLimits_ShouldReturnOkWhenBothUnderLimit start";
    auto ret = ModularObjectUtils::CheckLimits(0, 0);
    EXPECT_EQ(ret, ERR_OK);
    GTEST_LOG_(INFO) << "CheckLimits_ShouldReturnOkWhenBothUnderLimit end";
}

HWTEST_F(ModularObjectUtilsTest, CheckLimits_ShouldReturnInstanceLimitWhenInstanceAtLimit, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckLimits_ShouldReturnInstanceLimitWhenInstanceAtLimit start";
    auto ret = ModularObjectUtils::CheckLimits(20, 0);
    EXPECT_EQ(ret, ERR_MOE_INSTANCE_LIMIT);
    GTEST_LOG_(INFO) << "CheckLimits_ShouldReturnInstanceLimitWhenInstanceAtLimit end";
}

HWTEST_F(ModularObjectUtilsTest, CheckLimits_ShouldReturnConnectionLimitWhenConnectionAtLimit, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckLimits_ShouldReturnConnectionLimitWhenConnectionAtLimit start";
    auto ret = ModularObjectUtils::CheckLimits(0, 5);
    EXPECT_EQ(ret, ERR_MOE_CONNECTION_LIMIT);
    GTEST_LOG_(INFO) << "CheckLimits_ShouldReturnConnectionLimitWhenConnectionAtLimit end";
}

HWTEST_F(ModularObjectUtilsTest, CheckLimits_ShouldReturnInstanceLimitWhenBothOverLimit, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckLimits_ShouldReturnInstanceLimitWhenBothOverLimit start";
    auto ret = ModularObjectUtils::CheckLimits(25, 10);
    EXPECT_EQ(ret, ERR_MOE_INSTANCE_LIMIT);
    GTEST_LOG_(INFO) << "CheckLimits_ShouldReturnInstanceLimitWhenBothOverLimit end";
}

// ==================== QueryConfig ====================

HWTEST_F(ModularObjectUtilsTest, QueryConfig_ShouldReturnNullptrWhenQueryFails, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QueryConfig_ShouldReturnNullptrWhenQueryFails start";
    MockFlag::querySelfModularObjectRet = -1;
    AbilityRequest request;
    request.userId = 100;
    request.abilityInfo.bundleName = "com.test.bundle";
    request.appInfo.appIndex = 0;
    auto result = ModularObjectUtils::QueryConfig(request);
    EXPECT_EQ(result, nullptr);
    GTEST_LOG_(INFO) << "QueryConfig_ShouldReturnNullptrWhenQueryFails end";
}

HWTEST_F(ModularObjectUtilsTest, QueryConfig_ShouldReturnNullptrWhenEmptyInfos, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QueryConfig_ShouldReturnNullptrWhenEmptyInfos start";
    MockFlag::querySelfModularObjectRet = 0;
    MockFlag::modularObjectInfos.clear();
    AbilityRequest request;
    request.userId = 100;
    request.abilityInfo.bundleName = "com.test.bundle";
    request.abilityInfo.name = "TestAbility";
    request.appInfo.appIndex = 0;
    auto result = ModularObjectUtils::QueryConfig(request);
    EXPECT_EQ(result, nullptr);
    GTEST_LOG_(INFO) << "QueryConfig_ShouldReturnNullptrWhenEmptyInfos end";
}

HWTEST_F(ModularObjectUtilsTest, QueryConfig_ShouldReturnNullptrWhenAbilityNameNotFound, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QueryConfig_ShouldReturnNullptrWhenAbilityNameNotFound start";
    ModularObjectExtensionInfo info;
    info.bundleName = "com.test.bundle";
    info.abilityName = "OtherAbility";
    MockFlag::modularObjectInfos = {info};
    AbilityRequest request;
    request.userId = 100;
    request.abilityInfo.bundleName = "com.test.bundle";
    request.abilityInfo.name = "TestAbility";
    request.appInfo.appIndex = 0;
    auto result = ModularObjectUtils::QueryConfig(request);
    EXPECT_EQ(result, nullptr);
    GTEST_LOG_(INFO) << "QueryConfig_ShouldReturnNullptrWhenAbilityNameNotFound end";
}

HWTEST_F(ModularObjectUtilsTest, QueryConfig_ShouldReturnConfigWhenAbilityNameMatched, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QueryConfig_ShouldReturnConfigWhenAbilityNameMatched start";
    ModularObjectExtensionInfo info;
    info.bundleName = "com.test.bundle";
    info.abilityName = "TestAbility";
    info.launchMode = MoeLaunchMode::CROSS_PROCESS;
    MockFlag::modularObjectInfos = {info};
    AbilityRequest request;
    request.userId = 100;
    request.abilityInfo.bundleName = "com.test.bundle";
    request.abilityInfo.name = "TestAbility";
    request.appInfo.appIndex = 0;
    auto result = ModularObjectUtils::QueryConfig(request);
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->abilityName, "TestAbility");
    EXPECT_EQ(result->launchMode, MoeLaunchMode::CROSS_PROCESS);
    GTEST_LOG_(INFO) << "QueryConfig_ShouldReturnConfigWhenAbilityNameMatched end";
}

// ==================== SetupNewRecord ====================

HWTEST_F(ModularObjectUtilsTest, SetupNewRecord_ShouldNotCrashWhenServiceIsNull, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetupNewRecord_ShouldNotCrashWhenServiceIsNull start";
    AbilityRequest request;
    std::shared_ptr<BaseExtensionRecord> nullService = nullptr;
    ModularObjectUtils::SetupNewRecord(request, nullService, "key_123");
    GTEST_LOG_(INFO) << "SetupNewRecord_ShouldNotCrashWhenServiceIsNull end";
}

HWTEST_F(ModularObjectUtilsTest, SetupNewRecord_ShouldNotSetProcessNameWhenConfigIsNull, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetupNewRecord_ShouldNotSetProcessNameWhenConfigIsNull start";
    MockFlag::modularObjectInfos.clear();
    AbilityRequest request;
    auto service = std::make_shared<BaseExtensionRecord>();
    ModularObjectUtils::SetupNewRecord(request, service, "key_123");
    EXPECT_EQ(AbilityRecord::processName_, "");
    GTEST_LOG_(INFO) << "SetupNewRecord_ShouldNotSetProcessNameWhenConfigIsNull end";
}

HWTEST_F(ModularObjectUtilsTest, SetupNewRecord_ShouldUseProcessNameWhenInProcessMode, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetupNewRecord_ShouldUseProcessNameWhenInProcessMode start";
    ModularObjectExtensionInfo info;
    info.bundleName = "com.test.bundle";
    info.abilityName = "TestAbility";
    info.launchMode = MoeLaunchMode::IN_PROCESS;
    MockFlag::modularObjectInfos = {info};
    MockFlag::processName = "com.test.process";
    AbilityRequest request;
    request.abilityInfo.bundleName = "com.test.bundle";
    request.abilityInfo.name = "TestAbility";
    auto service = std::make_shared<BaseExtensionRecord>();
    AbilityRecord::processName_ = "";
    ModularObjectUtils::SetupNewRecord(request, service, "key_456");
    EXPECT_EQ(AbilityRecord::processName_, "com.test.process");
    GTEST_LOG_(INFO) << "SetupNewRecord_ShouldUseProcessNameWhenInProcessMode end";
}

HWTEST_F(ModularObjectUtilsTest, SetupNewRecord_ShouldUseBundleProcessModeWhenCrossProcessBundle, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetupNewRecord_ShouldUseBundleProcessModeWhenCrossProcessBundle start";
    ModularObjectExtensionInfo info;
    info.bundleName = "com.test.bundle";
    info.abilityName = "TestAbility";
    info.launchMode = MoeLaunchMode::CROSS_PROCESS;
    info.processMode = MoeProcessMode::BUNDLE;
    MockFlag::modularObjectInfos = {info};
    AbilityRequest request;
    request.abilityInfo.bundleName = "com.test.bundle";
    request.abilityInfo.name = "TestAbility";
    request.abilityInfo.extensionTypeName = "modularObject";
    auto service = std::make_shared<BaseExtensionRecord>();
    AbilityRecord::processName_ = "";
    ModularObjectUtils::SetupNewRecord(request, service, "key_202");
    EXPECT_EQ(AbilityRecord::processName_, "com.test.bundle:modularObject");
    GTEST_LOG_(INFO) << "SetupNewRecord_ShouldUseBundleProcessModeWhenCrossProcessBundle end";
}

HWTEST_F(ModularObjectUtilsTest, SetupNewRecord_ShouldUseTypeProcessModeWhenCrossProcessType, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetupNewRecord_ShouldUseTypeProcessModeWhenCrossProcessType start";
    ModularObjectExtensionInfo info;
    info.bundleName = "com.test.bundle";
    info.abilityName = "TestAbility";
    info.launchMode = MoeLaunchMode::CROSS_PROCESS;
    info.processMode = MoeProcessMode::TYPE;
    MockFlag::modularObjectInfos = {info};
    AbilityRequest request;
    request.abilityInfo.bundleName = "com.test.bundle";
    request.abilityInfo.name = "TestAbility";
    auto service = std::make_shared<BaseExtensionRecord>();
    AbilityRecord::processName_ = "";
    ModularObjectUtils::SetupNewRecord(request, service, "key_303");
    EXPECT_EQ(AbilityRecord::processName_, "com.test.bundle:TestAbility");
    GTEST_LOG_(INFO) << "SetupNewRecord_ShouldUseTypeProcessModeWhenCrossProcessType end";
}

HWTEST_F(ModularObjectUtilsTest, SetupNewRecord_ShouldUseInstanceProcessModeWhenCrossProcessInstance, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetupNewRecord_ShouldUseInstanceProcessModeWhenCrossProcessInstance start";
    ModularObjectExtensionInfo info;
    info.bundleName = "com.test.bundle";
    info.abilityName = "TestAbility";
    info.launchMode = MoeLaunchMode::CROSS_PROCESS;
    info.processMode = MoeProcessMode::INSTANCE;
    MockFlag::modularObjectInfos = {info};
    AbilityRequest request;
    request.abilityInfo.bundleName = "com.test.bundle";
    request.abilityInfo.name = "TestAbility";
    auto service = std::make_shared<BaseExtensionRecord>();
    AbilityRecord::recordId_ = 42;
    AbilityRecord::processName_ = "";
    ModularObjectUtils::SetupNewRecord(request, service, "key_404");
    EXPECT_EQ(AbilityRecord::processName_, "com.test.bundle:TestAbility:42");
    GTEST_LOG_(INFO) << "SetupNewRecord_ShouldUseInstanceProcessModeWhenCrossProcessInstance end";
}

HWTEST_F(ModularObjectUtilsTest, SetupNewRecord_ShouldSetRequestIdWhenUnderscoreExists, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetupNewRecord_ShouldSetRequestIdWhenUnderscoreExists start";
    ModularObjectExtensionInfo info;
    info.bundleName = "com.test.bundle";
    info.abilityName = "TestAbility";
    info.launchMode = MoeLaunchMode::CROSS_PROCESS;
    info.processMode = MoeProcessMode::BUNDLE;
    MockFlag::modularObjectInfos = {info};
    AbilityRequest request;
    request.abilityInfo.bundleName = "com.test.bundle";
    request.abilityInfo.name = "TestAbility";
    request.abilityInfo.extensionTypeName = "modularObject";
    auto service = std::make_shared<BaseExtensionRecord>();
    BaseExtensionRecord::requestId_ = "";
    ModularObjectUtils::SetupNewRecord(request, service, "key_myrandid123");
    EXPECT_EQ(BaseExtensionRecord::requestId_, "myrandid123");
    GTEST_LOG_(INFO) << "SetupNewRecord_ShouldSetRequestIdWhenUnderscoreExists end";
}

HWTEST_F(ModularObjectUtilsTest, SetupNewRecord_ShouldNotSetRequestIdWhenEmptyServiceKey, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetupNewRecord_ShouldNotSetRequestIdWhenEmptyServiceKey start";
    ModularObjectExtensionInfo info;
    info.bundleName = "com.test.bundle";
    info.abilityName = "TestAbility";
    info.launchMode = MoeLaunchMode::CROSS_PROCESS;
    info.processMode = MoeProcessMode::BUNDLE;
    MockFlag::modularObjectInfos = {info};
    AbilityRequest request;
    request.abilityInfo.bundleName = "com.test.bundle";
    request.abilityInfo.name = "TestAbility";
    request.abilityInfo.extensionTypeName = "modularObject";
    auto service = std::make_shared<BaseExtensionRecord>();
    BaseExtensionRecord::requestId_ = "oldvalue";
    std::string emptyKey;
    ModularObjectUtils::SetupNewRecord(request, service, emptyKey);
    EXPECT_EQ(BaseExtensionRecord::requestId_, "oldvalue");
    GTEST_LOG_(INFO) << "SetupNewRecord_ShouldNotSetRequestIdWhenEmptyServiceKey end";
}

HWTEST_F(ModularObjectUtilsTest, SetupNewRecord_ShouldNotSetRequestIdWhenNoUnderscore, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetupNewRecord_ShouldNotSetRequestIdWhenNoUnderscore start";
    ModularObjectExtensionInfo info;
    info.bundleName = "com.test.bundle";
    info.abilityName = "TestAbility";
    info.launchMode = MoeLaunchMode::CROSS_PROCESS;
    info.processMode = MoeProcessMode::BUNDLE;
    MockFlag::modularObjectInfos = {info};
    AbilityRequest request;
    request.abilityInfo.bundleName = "com.test.bundle";
    request.abilityInfo.name = "TestAbility";
    request.abilityInfo.extensionTypeName = "modularObject";
    auto service = std::make_shared<BaseExtensionRecord>();
    BaseExtensionRecord::requestId_ = "oldvalue";
    ModularObjectUtils::SetupNewRecord(request, service, "noUnderscoreKey");
    EXPECT_EQ(BaseExtensionRecord::requestId_, "oldvalue");
    GTEST_LOG_(INFO) << "SetupNewRecord_ShouldNotSetRequestIdWhenNoUnderscore end";
}

HWTEST_F(ModularObjectUtilsTest, SetupNewRecord_ShouldSetEmptyRequestIdWhenTrailingUnderscore, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetupNewRecord_ShouldSetEmptyRequestIdWhenTrailingUnderscore start";
    ModularObjectExtensionInfo info;
    info.bundleName = "com.test.bundle";
    info.abilityName = "TestAbility";
    info.launchMode = MoeLaunchMode::CROSS_PROCESS;
    info.processMode = MoeProcessMode::BUNDLE;
    MockFlag::modularObjectInfos = {info};
    AbilityRequest request;
    request.abilityInfo.bundleName = "com.test.bundle";
    request.abilityInfo.name = "TestAbility";
    request.abilityInfo.extensionTypeName = "modularObject";
    auto service = std::make_shared<BaseExtensionRecord>();
    BaseExtensionRecord::requestId_ = "oldvalue";
    ModularObjectUtils::SetupNewRecord(request, service, "key_");
    EXPECT_EQ(BaseExtensionRecord::requestId_, "");
    GTEST_LOG_(INFO) << "SetupNewRecord_ShouldSetEmptyRequestIdWhenTrailingUnderscore end";
}

HWTEST_F(ModularObjectUtilsTest, SetupNewRecord_ShouldNotSetProcessNameWhenInvalidProcessMode, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetupNewRecord_ShouldNotSetProcessNameWhenInvalidProcessMode start";
    ModularObjectExtensionInfo info;
    info.bundleName = "com.test.bundle";
    info.abilityName = "TestAbility";
    info.launchMode = MoeLaunchMode::CROSS_PROCESS;
    info.processMode = static_cast<MoeProcessMode>(99);
    MockFlag::modularObjectInfos = {info};
    AbilityRequest request;
    request.abilityInfo.bundleName = "com.test.bundle";
    request.abilityInfo.name = "TestAbility";
    auto service = std::make_shared<BaseExtensionRecord>();
    AbilityRecord::processName_ = "";
    ModularObjectUtils::SetupNewRecord(request, service, "key_123");
    EXPECT_EQ(AbilityRecord::processName_, "");
    GTEST_LOG_(INFO) << "SetupNewRecord_ShouldNotSetProcessNameWhenInvalidProcessMode end";
}

HWTEST_F(ModularObjectUtilsTest,
    SetupNewRecord_ShouldAppendAppCloneIndexWhenCrossProcessBundleWithClone, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetupNewRecord_ShouldAppendAppCloneIndexWhenCrossProcessBundleWithClone start";
    // CROSS_PROCESS + BUNDLE mode + appCloneIndex > 0
    ModularObjectExtensionInfo info;
    info.bundleName = "com.test.bundle";
    info.abilityName = "TestAbility";
    info.launchMode = MoeLaunchMode::CROSS_PROCESS;
    info.processMode = MoeProcessMode::BUNDLE;
    MockFlag::modularObjectInfos = {info};
    AbilityRequest request;
    request.abilityInfo.bundleName = "com.test.bundle";
    request.abilityInfo.name = "TestAbility";
    request.abilityInfo.extensionTypeName = "modularObject";
    request.appInfo.appIndex = 2;
    auto service = std::make_shared<BaseExtensionRecord>();
    AbilityRecord::processName_ = "";
    ModularObjectUtils::SetupNewRecord(request, service, "key_500");
    EXPECT_EQ(AbilityRecord::processName_, "com.test.bundle:modularObject:2");
    GTEST_LOG_(INFO) << "SetupNewRecord_ShouldAppendAppCloneIndexWhenCrossProcessBundleWithClone end";
}

HWTEST_F(ModularObjectUtilsTest,
    SetupNewRecord_ShouldAppendAppCloneIndexWhenCrossProcessTypeWithClone, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetupNewRecord_ShouldAppendAppCloneIndexWhenCrossProcessTypeWithClone start";
    // CROSS_PROCESS + TYPE mode + appCloneIndex > 0
    ModularObjectExtensionInfo info;
    info.bundleName = "com.test.bundle";
    info.abilityName = "TestAbility";
    info.launchMode = MoeLaunchMode::CROSS_PROCESS;
    info.processMode = MoeProcessMode::TYPE;
    MockFlag::modularObjectInfos = {info};
    AbilityRequest request;
    request.abilityInfo.bundleName = "com.test.bundle";
    request.abilityInfo.name = "TestAbility";
    request.appInfo.appIndex = 3;
    auto service = std::make_shared<BaseExtensionRecord>();
    AbilityRecord::processName_ = "";
    ModularObjectUtils::SetupNewRecord(request, service, "key_600");
    EXPECT_EQ(AbilityRecord::processName_, "com.test.bundle:TestAbility:3");
    GTEST_LOG_(INFO) << "SetupNewRecord_ShouldAppendAppCloneIndexWhenCrossProcessTypeWithClone end";
}

HWTEST_F(ModularObjectUtilsTest,
    SetupNewRecord_ShouldNotAppendAppCloneIndexWhenInProcessWithClone, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetupNewRecord_ShouldNotAppendAppCloneIndexWhenInProcessWithClone start";
    // IN_PROCESS mode + appCloneIndex > 0 → should NOT append
    ModularObjectExtensionInfo info;
    info.bundleName = "com.test.bundle";
    info.abilityName = "TestAbility";
    info.launchMode = MoeLaunchMode::IN_PROCESS;
    MockFlag::modularObjectInfos = {info};
    MockFlag::processName = "com.test.process";
    AbilityRequest request;
    request.abilityInfo.bundleName = "com.test.bundle";
    request.abilityInfo.name = "TestAbility";
    request.appInfo.appIndex = 2;
    auto service = std::make_shared<BaseExtensionRecord>();
    AbilityRecord::processName_ = "";
    ModularObjectUtils::SetupNewRecord(request, service, "key_700");
    EXPECT_EQ(AbilityRecord::processName_, "com.test.process");
    GTEST_LOG_(INFO) << "SetupNewRecord_ShouldNotAppendAppCloneIndexWhenInProcessWithClone end";
}

HWTEST_F(ModularObjectUtilsTest,
    SetupNewRecord_ShouldReturnErrorWhenInProcessGetProcessInfoFailed, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetupNewRecord_ShouldReturnErrorWhenInProcessGetProcessInfoFailed start";
    // IN_PROCESS + GetRunningProcessInfoByPid fails → return error
    ModularObjectExtensionInfo info;
    info.bundleName = "com.test.bundle";
    info.abilityName = "TestAbility";
    info.launchMode = MoeLaunchMode::IN_PROCESS;
    MockFlag::modularObjectInfos = {info};
    MockFlag::getRunningProcessInfoRet = -1;
    AbilityRequest request;
    request.abilityInfo.bundleName = "com.test.bundle";
    request.abilityInfo.name = "TestAbility";
    request.appInfo.appIndex = 2;
    auto service = std::make_shared<BaseExtensionRecord>();
    AbilityRecord::processName_ = "";
    auto ret = ModularObjectUtils::SetupNewRecord(request, service, "key_800");
    EXPECT_NE(ret, ERR_OK);
    MockFlag::getRunningProcessInfoRet = 0;
    GTEST_LOG_(INFO) << "SetupNewRecord_ShouldReturnErrorWhenInProcessGetProcessInfoFailed end";
}
