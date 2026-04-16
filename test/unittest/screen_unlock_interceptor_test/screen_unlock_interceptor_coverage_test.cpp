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
#define private public
#define protected public
#include "ability_manager_service.h"
#include "extension_config.h"
#include "interceptor/screen_unlock_interceptor.h"
#undef private
#undef protected

#include "ability_util.h"
#include "bundle_mgr_helper.h"
#include "event_report.h"
#include "nlohmann/json.hpp"
#include "parameters.h"
#include "scene_board_judgement.h"
#include "start_ability_utils.h"
#include "screenlock_manager.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AppExecFwk;
using json = nlohmann::json;

namespace OHOS {
namespace AAFwk {

class ScreenUnlockInterceptorCoverageTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    void LoadTestConfig(const std::string &configStr);
    void SetupExtensionAbilityInfo(bool isSystemApp, const std::string &appIdentifier);

public:
    static std::shared_ptr<ExtensionConfig> extensionConfig_;
};

std::shared_ptr<ExtensionConfig> ScreenUnlockInterceptorCoverageTest::extensionConfig_ =
    DelayedSingleton<ExtensionConfig>::GetInstance();

void ScreenUnlockInterceptorCoverageTest::SetUpTestCase()
{}

void ScreenUnlockInterceptorCoverageTest::TearDownTestCase()
{}

void ScreenUnlockInterceptorCoverageTest::SetUp()
{
    extensionConfig_->configMap_.clear();
}

void ScreenUnlockInterceptorCoverageTest::TearDown()
{}

void ScreenUnlockInterceptorCoverageTest::LoadTestConfig(const std::string &configStr)
{
    nlohmann::json jsonConfig = nlohmann::json::parse(configStr);
    extensionConfig_->LoadExtensionConfig(jsonConfig);
}

void ScreenUnlockInterceptorCoverageTest::SetupExtensionAbilityInfo(bool isSystemApp,
    const std::string &appIdentifier)
{
    StartAbilityUtils::startAbilityInfo = std::make_shared<StartAbilityInfo>();
    StartAbilityUtils::startAbilityInfo->abilityInfo.type = AbilityType::EXTENSION;
    StartAbilityUtils::startAbilityInfo->abilityInfo.extensionTypeName = "form";
    StartAbilityUtils::startAbilityInfo->abilityInfo.applicationInfo.isSystemApp = isSystemApp;
    StartAbilityUtils::startAbilityInfo->abilityInfo.applicationInfo.allowAppRunWhenDeviceFirstLocked = false;
    StartAbilityUtils::startAbilityInfo->abilityInfo.applicationInfo.bundleName = "com.test.bundle";
}

/**
 * @tc.name: GetAppIdentifier_001
 * @tc.desc: Test GetAppIdentifier with empty bundleName
 * @tc.type: FUNC
 */
HWTEST_F(ScreenUnlockInterceptorCoverageTest, GetAppIdentifier_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetAppIdentifier_001 start";
    ScreenUnlockInterceptor screenUnlockInterceptor;
    auto appIdentifier = screenUnlockInterceptor.GetAppIdentifier("");
    EXPECT_EQ(appIdentifier, "");
    GTEST_LOG_(INFO) << "GetAppIdentifier_001 end";
}

/**
 * @tc.name: GetAppIdentifier_002
 * @tc.desc: Test GetAppIdentifier with valid bundleName (bundleMgrHelper may be null in test env)
 * @tc.type: FUNC
 */
HWTEST_F(ScreenUnlockInterceptorCoverageTest, GetAppIdentifier_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetAppIdentifier_002 start";
    ScreenUnlockInterceptor screenUnlockInterceptor;
    // In test environment, bundleMgrHelper may be null, so this should return empty string
    auto appIdentifier = screenUnlockInterceptor.GetAppIdentifier("com.test.bundle");
    // bundleMgrHelper is nullptr in test environment
    EXPECT_EQ(appIdentifier, "");
    GTEST_LOG_(INFO) << "GetAppIdentifier_002 end";
}

/**
 * @tc.name: GetTargetAbilityInfo_001
 * @tc.desc: Test GetTargetAbilityInfo when StartAbilityUtils::startAbilityInfo is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(ScreenUnlockInterceptorCoverageTest, GetTargetAbilityInfo_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetTargetAbilityInfo_001 start";
    ScreenUnlockInterceptor screenUnlockInterceptor;
    StartAbilityUtils::startAbilityInfo = nullptr;

    Want want;
    want.GetElement().SetBundleName("com.test.bundle");
    want.GetElement().SetAbilityName("TestAbility");
    AbilityInterceptorParam param(want, 0, 100, true, nullptr, []() { return false; });

    AppExecFwk::AbilityInfo targetAbilityInfo;
    bool ret = screenUnlockInterceptor.GetTargetAbilityInfo(param, targetAbilityInfo);
    // In test environment, QueryTargetAbilityInfo may fail, so ret should be false
    // or if it succeeds, the ability info should be populated
    GTEST_LOG_(INFO) << "GetTargetAbilityInfo result: " << ret;
    GTEST_LOG_(INFO) << "GetTargetAbilityInfo_001 end";
}

/**
 * @tc.name: GetTargetAbilityInfo_002
 * @tc.desc: Test GetTargetAbilityInfo when StartAbilityUtils::startAbilityInfo is not nullptr
 * @tc.type: FUNC
 */
HWTEST_F(ScreenUnlockInterceptorCoverageTest, GetTargetAbilityInfo_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetTargetAbilityInfo_002 start";
    ScreenUnlockInterceptor screenUnlockInterceptor;
    StartAbilityUtils::startAbilityInfo = std::make_shared<StartAbilityInfo>();
    StartAbilityUtils::startAbilityInfo->abilityInfo.type = AbilityType::EXTENSION;
    StartAbilityUtils::startAbilityInfo->abilityInfo.extensionTypeName = "form";
    StartAbilityUtils::startAbilityInfo->abilityInfo.applicationInfo.bundleName = "com.test.bundle";

    Want want;
    AbilityInterceptorParam param(want, 0, 100, true, nullptr, []() { return false; });

    AppExecFwk::AbilityInfo targetAbilityInfo;
    bool ret = screenUnlockInterceptor.GetTargetAbilityInfo(param, targetAbilityInfo);
    EXPECT_TRUE(ret);
    EXPECT_EQ(targetAbilityInfo.extensionTypeName, "form");
    GTEST_LOG_(INFO) << "GetTargetAbilityInfo_002 end";
}

/**
 * @tc.name: ProcessSystemApp_ShouldReturnBlockWhenAllowAppRunWhenDeviceFirstLockedIsFalse
 * @tc.desc: Test ProcessSystemApp with allowAppRunWhenDeviceFirstLocked = false
 * @tc.type: FUNC
 */
HWTEST_F(ScreenUnlockInterceptorCoverageTest,
    ProcessSystemApp_ShouldReturnBlockWhenAllowAppRunWhenDeviceFirstLockedIsFalse, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ProcessSystemApp_ShouldReturnBlockWhenAllowAppRunWhenDeviceFirstLockedIsFalse start";
    const std::string configStr = R"({
        "ams_extension_config": [{
            "name": "FormExtension",
            "extension_type_name": "form",
            "screen_unlock_access": {
                "defaultInterception": true
            }
        }]
    })";
    LoadTestConfig(configStr);

    ScreenUnlockInterceptor screenUnlockInterceptor;
    AppExecFwk::AbilityInfo targetAbilityInfo;
    targetAbilityInfo.type = AbilityType::EXTENSION;
    targetAbilityInfo.extensionTypeName = "form";
    targetAbilityInfo.applicationInfo.isSystemApp = true;
    targetAbilityInfo.applicationInfo.allowAppRunWhenDeviceFirstLocked = false;
    targetAbilityInfo.applicationInfo.bundleName = "com.test.bundle";

    auto ret = screenUnlockInterceptor.ProcessSystemApp(targetAbilityInfo);
    EXPECT_EQ(ret, ERR_BLOCK_START_FIRST_BOOT_SCREEN_UNLOCK);
    GTEST_LOG_(INFO) << "ProcessSystemApp_ShouldReturnBlockWhenAllowAppRunWhenDeviceFirstLockedIsFalse end";
}

/**
 * @tc.name: ProcessSystemApp_ShouldReturnOkWhenIsUIAbility
 * @tc.desc: Test ProcessSystemApp with UIAbility (not extension)
 * @tc.type: FUNC
 */
HWTEST_F(ScreenUnlockInterceptorCoverageTest, ProcessSystemApp_ShouldReturnOkWhenIsUIAbility, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ProcessSystemApp_002 start";
    ScreenUnlockInterceptor screenUnlockInterceptor;
    AppExecFwk::AbilityInfo targetAbilityInfo;
    targetAbilityInfo.type = AbilityType::PAGE;
    targetAbilityInfo.applicationInfo.isSystemApp = true;
    targetAbilityInfo.applicationInfo.allowAppRunWhenDeviceFirstLocked = true;
    targetAbilityInfo.applicationInfo.bundleName = "com.test.bundle";

    auto ret = screenUnlockInterceptor.ProcessSystemApp(targetAbilityInfo);
    EXPECT_EQ(ret, ERR_OK);
    GTEST_LOG_(INFO) << "ProcessSystemApp_ShouldReturnOkWhenIsUIAbility end";
}

/**
 * @tc.name: ProcessNonSystemApp_001
 * @tc.desc: Test ProcessNonSystemApp with UIAbility (should be blocked)
 * @tc.type: FUNC
 */
HWTEST_F(ScreenUnlockInterceptorCoverageTest, ProcessNonSystemApp_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ProcessNonSystemApp_001 start";
    ScreenUnlockInterceptor screenUnlockInterceptor;
    AppExecFwk::AbilityInfo targetAbilityInfo;
    targetAbilityInfo.type = AbilityType::PAGE;
    targetAbilityInfo.applicationInfo.isSystemApp = false;
    targetAbilityInfo.applicationInfo.bundleName = "com.test.bundle";

    auto ret = screenUnlockInterceptor.ProcessNonSystemApp(targetAbilityInfo);
    EXPECT_EQ(ret, ERR_BLOCK_START_FIRST_BOOT_SCREEN_UNLOCK);
    GTEST_LOG_(INFO) << "ProcessNonSystemApp_001 end";
}

/**
 * @tc.name: CheckSystemAppExtensionInterception_NoDefaultInterception
 * @tc.desc: Test system app extension with no defaultInterception and no systemAppInterception
 * @tc.type: FUNC
 */
HWTEST_F(ScreenUnlockInterceptorCoverageTest,
    CheckSystemAppExtensionInterception_NoDefaultInterception, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckSystemAppExtensionInterception_NoDefaultInterception start";
    const std::string configStr = R"({
        "ams_extension_config": [{
            "name": "FormExtension",
            "extension_type_name": "form",
            "screen_unlock_access": {
                "allowlist": [{"appIdentifier": "test_id"}]
            }
        }]
    })";
    LoadTestConfig(configStr);

    ScreenUnlockInterceptor screenUnlockInterceptor;
    auto ret = screenUnlockInterceptor.CheckSystemAppExtensionInterception("form", "com.test.bundle");
    // No defaultInterception and no systemAppInterception, should be blocked
    EXPECT_EQ(ret, ERR_BLOCK_START_FIRST_BOOT_SCREEN_UNLOCK);
    GTEST_LOG_(INFO) << "CheckSystemAppExtensionInterception_NoDefaultInterception end";
}

/**
 * @tc.name: CheckThirdPartyExtensionInterception_NoDefaultInterception
 * @tc.desc: Test third-party extension with no defaultInterception
 * @tc.type: FUNC
 */
HWTEST_F(ScreenUnlockInterceptorCoverageTest,
    CheckThirdPartyExtensionInterception_NoDefaultInterception, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckThirdPartyExtensionInterception_NoDefaultInterception start";
    const std::string configStr = R"({
        "ams_extension_config": [{
            "name": "FormExtension",
            "extension_type_name": "form",
            "screen_unlock_access": {
                "systemAppInterception": true,
                "allowlist": [{"appIdentifier": "test_id"}]
            }
        }]
    })";
    LoadTestConfig(configStr);

    ScreenUnlockInterceptor screenUnlockInterceptor;
    auto ret = screenUnlockInterceptor.CheckThirdPartyExtensionInterception("form", "com.test.bundle");
    // Third-party apps only check defaultInterception, should be blocked
    EXPECT_EQ(ret, ERR_BLOCK_START_FIRST_BOOT_SCREEN_UNLOCK);
    GTEST_LOG_(INFO) << "CheckThirdPartyExtensionInterception_NoDefaultInterception end";
}

/**
 * @tc.name: CheckInterceptionByConfig_InterceptionTrue_NotInAllowList
 * @tc.desc: Test CheckInterceptionByConfig with interception=true and not in allowlist
 * @tc.type: FUNC
 */
HWTEST_F(ScreenUnlockInterceptorCoverageTest,
    CheckInterceptionByConfig_InterceptionTrue_NotInAllowList, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckInterceptionByConfig_InterceptionTrue_NotInAllowList start";
    const std::string configStr = R"({
        "ams_extension_config": [{
            "name": "FormExtension",
            "extension_type_name": "form",
            "screen_unlock_access": {
                "defaultInterception": true,
                "allowlist": [{"appIdentifier": "other_id"}]
            }
        }]
    })";
    LoadTestConfig(configStr);

    ScreenUnlockInterceptor screenUnlockInterceptor;
    auto ret = screenUnlockInterceptor.CheckInterceptionByConfig("form", "test_id", true, false);
    // interception=true, not in allowlist, should be blocked
    EXPECT_EQ(ret, ERR_BLOCK_START_FIRST_BOOT_SCREEN_UNLOCK);
    GTEST_LOG_(INFO) << "CheckInterceptionByConfig_InterceptionTrue_NotInAllowList end";
}

/**
 * @tc.name: CheckInterceptionByConfig_InterceptionFalse_InBlockList
 * @tc.desc: Test CheckInterceptionByConfig with interception=false and in blocklist
 * @tc.type: FUNC
 */
HWTEST_F(ScreenUnlockInterceptorCoverageTest, CheckInterceptionByConfig_InterceptionFalse_InBlockList, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckInterceptionByConfig_InterceptionFalse_InBlockList start";
    const std::string configStr = R"({
        "ams_extension_config": [{
            "name": "FormExtension",
            "extension_type_name": "form",
            "screen_unlock_access": {
                "defaultInterception": false,
                "blocklist": [{"appIdentifier": "test_id"}]
            }
        }]
    })";
    LoadTestConfig(configStr);

    ScreenUnlockInterceptor screenUnlockInterceptor;
    auto ret = screenUnlockInterceptor.CheckInterceptionByConfig("form", "test_id", false, false);
    // interception=false, in blocklist, should be blocked
    EXPECT_EQ(ret, ERR_BLOCK_START_FIRST_BOOT_SCREEN_UNLOCK);
    GTEST_LOG_(INFO) << "CheckInterceptionByConfig_InterceptionFalse_InBlockList end";
}

/**
 * @tc.name: CheckInterceptionByConfig_InterceptionFalse_NotInBlockList
 * @tc.desc: Test CheckInterceptionByConfig with interception=false and not in blocklist
 * @tc.type: FUNC
 */
HWTEST_F(ScreenUnlockInterceptorCoverageTest,
    CheckInterceptionByConfig_InterceptionFalse_NotInBlockList, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckInterceptionByConfig_InterceptionFalse_NotInBlockList start";
    const std::string configStr = R"({
        "ams_extension_config": [{
            "name": "FormExtension",
            "extension_type_name": "form",
            "screen_unlock_access": {
                "defaultInterception": false,
                "blocklist": [{"appIdentifier": "other_id"}]
            }
        }]
    })";
    LoadTestConfig(configStr);

    ScreenUnlockInterceptor screenUnlockInterceptor;
    auto ret = screenUnlockInterceptor.CheckInterceptionByConfig("form", "test_id", false, false);
    // interception=false, not in blocklist, should be allowed
    EXPECT_EQ(ret, ERR_OK);
    GTEST_LOG_(INFO) << "CheckInterceptionByConfig_InterceptionFalse_NotInBlockList end";
}

/**
 * @tc.name: DoProcess_SystemAppAllowAppRunWhenDeviceFirstLockedFalse
 * @tc.desc: Test system app with allowAppRunWhenDeviceFirstLocked=false should be blocked
 * @tc.type: FUNC
 */
HWTEST_F(ScreenUnlockInterceptorCoverageTest, DoProcess_SystemAppAllowAppRunWhenDeviceFirstLockedFalse, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DoProcess_SystemAppAllowAppRunWhenDeviceFirstLockedFalse start";
    const std::string configStr = R"({
        "ams_extension_config": [{
            "name": "FormExtension",
            "extension_type_name": "form",
            "screen_unlock_access": {
                "systemAppInterception": false,
                "defaultInterception": true
            }
        }]
    })";
    LoadTestConfig(configStr);

    ScreenUnlockInterceptor screenUnlockInterceptor;
    StartAbilityUtils::startAbilityInfo = std::make_shared<StartAbilityInfo>();
    StartAbilityUtils::startAbilityInfo->abilityInfo.type = AbilityType::PAGE;
    StartAbilityUtils::startAbilityInfo->abilityInfo.applicationInfo.isSystemApp = true;
    StartAbilityUtils::startAbilityInfo->abilityInfo.applicationInfo.allowAppRunWhenDeviceFirstLocked = false;

    Want want;
    AbilityInterceptorParam param(want, 0, 100, true, nullptr, []() { return false; });

    auto screenLockManager = OHOS::ScreenLock::ScreenLockManager::GetInstance();
    EXPECT_NE(screenLockManager, nullptr);
    screenLockManager->SetScreenLockedState(true);

    auto ret = screenUnlockInterceptor.DoProcess(param);
    EXPECT_EQ(ret, ERR_BLOCK_START_FIRST_BOOT_SCREEN_UNLOCK);
    GTEST_LOG_(INFO) << "DoProcess_SystemAppAllowAppRunWhenDeviceFirstLockedFalse end";
}

/**
 * @tc.name: LoadScreenUnlockAppIdentifierList_InvalidData
 * @tc.desc: Test LoadScreenUnlockAppIdentifierList with invalid data formats
 * @tc.type: FUNC
 */
HWTEST_F(ScreenUnlockInterceptorCoverageTest, LoadScreenUnlockAppIdentifierList_InvalidData, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "LoadScreenUnlockAppIdentifierList_InvalidData start";
    const std::string configStr = R"({
        "ams_extension_config": [{
            "name": "FormExtension",
            "extension_type_name": "form",
            "screen_unlock_access": {
                "defaultInterception": true,
                "allowlist": [
                    "invalid_string_item",
                    123,
                    {"bundleName": "no_app_identifier_field"},
                    {"appIdentifier": 12345},
                    {"appIdentifier": ""},
                    {"appIdentifier": "valid_id"}
                ]
            }
        }]
    })";
    LoadTestConfig(configStr);

    // Only the last item with valid appIdentifier should be loaded
    bool inList = extensionConfig_->IsInScreenUnlockAccessAllowList("form", "valid_id");
    EXPECT_TRUE(inList);

    // Empty string appIdentifier should not be in list
    inList = extensionConfig_->IsInScreenUnlockAccessAllowList("form", "");
    EXPECT_FALSE(inList);

    GTEST_LOG_(INFO) << "LoadScreenUnlockAppIdentifierList_InvalidData end";
}

/**
 * @tc.name: DoProcess_CompleteFlow_SystemApp
 * @tc.desc: Test complete DoProcess flow for system app with config
 * @tc.type: FUNC
 */
HWTEST_F(ScreenUnlockInterceptorCoverageTest, DoProcess_CompleteFlow_SystemApp, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DoProcess_CompleteFlow_SystemApp start";
    const std::string configStr = R"({
        "ams_extension_config": [{
            "name": "FormExtension",
            "extension_type_name": "form",
            "screen_unlock_access": {
                "systemAppInterception": true,
                "defaultInterception": false,
                "allowlist": [{"appIdentifier": "system_app_id"}]
            }
        }]
    })";
    LoadTestConfig(configStr);

    ScreenUnlockInterceptor screenUnlockInterceptor;
    StartAbilityUtils::startAbilityInfo = std::make_shared<StartAbilityInfo>();
    StartAbilityUtils::startAbilityInfo->abilityInfo.type = AbilityType::EXTENSION;
    StartAbilityUtils::startAbilityInfo->abilityInfo.extensionTypeName = "form";
    StartAbilityUtils::startAbilityInfo->abilityInfo.applicationInfo.isSystemApp = true;
    StartAbilityUtils::startAbilityInfo->abilityInfo.applicationInfo.allowAppRunWhenDeviceFirstLocked = true;
    StartAbilityUtils::startAbilityInfo->abilityInfo.applicationInfo.bundleName = "com.test.system.app";

    Want want;
    AbilityInterceptorParam param(want, 0, 100, true, nullptr, []() { return false; });

    auto screenLockManager = OHOS::ScreenLock::ScreenLockManager::GetInstance();
    EXPECT_NE(screenLockManager, nullptr);
    screenLockManager->SetScreenLockedState(true);

    // Note: GetAppIdentifier will fail in test env, so the test may not fully execute
    // But it should not crash
    auto ret = screenUnlockInterceptor.DoProcess(param);
    // The result depends on whether GetAppIdentifier succeeds
    GTEST_LOG_(INFO) << "DoProcess result: " << ret;
    GTEST_LOG_(INFO) << "DoProcess_CompleteFlow_SystemApp end";
}

/**
 * @tc.name: DoProcess_CompleteFlow_ThirdPartyApp
 * @tc.desc: Test complete DoProcess flow for third-party app with config
 * @tc.type: FUNC
 */
HWTEST_F(ScreenUnlockInterceptorCoverageTest, DoProcess_CompleteFlow_ThirdPartyApp, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DoProcess_CompleteFlow_ThirdPartyApp start";
    const std::string configStr = R"({
        "ams_extension_config": [{
            "name": "FormExtension",
            "extension_type_name": "form",
            "screen_unlock_access": {
                "defaultInterception": false,
                "blocklist": [{"appIdentifier": "blocked_app_id"}]
            }
        }]
    })";
    LoadTestConfig(configStr);

    ScreenUnlockInterceptor screenUnlockInterceptor;
    StartAbilityUtils::startAbilityInfo = std::make_shared<StartAbilityInfo>();
    StartAbilityUtils::startAbilityInfo->abilityInfo.type = AbilityType::EXTENSION;
    StartAbilityUtils::startAbilityInfo->abilityInfo.extensionTypeName = "form";
    StartAbilityUtils::startAbilityInfo->abilityInfo.applicationInfo.isSystemApp = false;
    StartAbilityUtils::startAbilityInfo->abilityInfo.applicationInfo.bundleName = "com.test.third.party";

    Want want;
    AbilityInterceptorParam param(want, 0, 100, true, nullptr, []() { return false; });

    auto screenLockManager = OHOS::ScreenLock::ScreenLockManager::GetInstance();
    EXPECT_NE(screenLockManager, nullptr);
    screenLockManager->SetScreenLockedState(true);

    // Note: GetAppIdentifier will fail in test env, so the test may not fully execute
    // But it should not crash
    auto ret = screenUnlockInterceptor.DoProcess(param);
    GTEST_LOG_(INFO) << "DoProcess result: " << ret;
    GTEST_LOG_(INFO) << "DoProcess_CompleteFlow_ThirdPartyApp end";
}

/**
 * @tc.name: DoProcess_SystemAppExtension_DefaultInterceptionTrue_NoList
 * @tc.desc: System app Extension with defaultInterception=true and empty allowlist/blocklist
 * @tc.type: FUNC
 */
HWTEST_F(ScreenUnlockInterceptorCoverageTest,
    DoProcess_SystemAppExtension_DefaultInterceptionTrue_NoList, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DoProcess_SystemAppExtension_DefaultInterceptionTrue_NoList start";
    const std::string configStr = R"({
        "ams_extension_config": [{
            "name": "FormExtension",
            "extension_type_name": "form",
            "screen_unlock_access": {
                "defaultInterception": true
            }
        }]
    })";
    LoadTestConfig(configStr);

    ScreenUnlockInterceptor screenUnlockInterceptor;
    StartAbilityUtils::startAbilityInfo = std::make_shared<StartAbilityInfo>();
    StartAbilityUtils::startAbilityInfo->abilityInfo.type = AbilityType::EXTENSION;
    StartAbilityUtils::startAbilityInfo->abilityInfo.extensionTypeName = "form";
    StartAbilityUtils::startAbilityInfo->abilityInfo.applicationInfo.isSystemApp = true;
    StartAbilityUtils::startAbilityInfo->abilityInfo.applicationInfo.allowAppRunWhenDeviceFirstLocked = true;
    StartAbilityUtils::startAbilityInfo->abilityInfo.applicationInfo.bundleName = "com.test.system.app";

    Want want;
    AbilityInterceptorParam param(want, 0, 100, true, nullptr, []() { return false; });

    auto screenLockManager = OHOS::ScreenLock::ScreenLockManager::GetInstance();
    EXPECT_NE(screenLockManager, nullptr);
    screenLockManager->SetScreenLockedState(true);

    // defaultInterception=true (whitelist mode), allowlist empty → block
    auto ret = screenUnlockInterceptor.DoProcess(param);
    EXPECT_EQ(ret, ERR_BLOCK_START_FIRST_BOOT_SCREEN_UNLOCK);
    GTEST_LOG_(INFO) << "DoProcess_SystemAppExtension_DefaultInterceptionTrue_NoList end";
}

/**
 * @tc.name: DoProcess_ThirdPartyExtension_DefaultInterceptionFalse_NoList
 * @tc.desc: Third-party Extension with defaultInterception=false and empty allowlist/blocklist
 * @tc.type: FUNC
 */
HWTEST_F(ScreenUnlockInterceptorCoverageTest,
    DoProcess_ThirdPartyExtension_DefaultInterceptionFalse_NoList, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DoProcess_ThirdPartyExtension_DefaultInterceptionFalse_NoList start";
    const std::string configStr = R"({
        "ams_extension_config": [{
            "name": "FormExtension",
            "extension_type_name": "form",
            "screen_unlock_access": {
                "defaultInterception": false
            }
        }]
    })";
    LoadTestConfig(configStr);

    ScreenUnlockInterceptor screenUnlockInterceptor;
    StartAbilityUtils::startAbilityInfo = std::make_shared<StartAbilityInfo>();
    StartAbilityUtils::startAbilityInfo->abilityInfo.type = AbilityType::EXTENSION;
    StartAbilityUtils::startAbilityInfo->abilityInfo.extensionTypeName = "form";
    StartAbilityUtils::startAbilityInfo->abilityInfo.applicationInfo.isSystemApp = false;
    StartAbilityUtils::startAbilityInfo->abilityInfo.applicationInfo.bundleName = "com.test.third.party";

    Want want;
    AbilityInterceptorParam param(want, 0, 100, true, nullptr, []() { return false; });

    auto screenLockManager = OHOS::ScreenLock::ScreenLockManager::GetInstance();
    EXPECT_NE(screenLockManager, nullptr);
    screenLockManager->SetScreenLockedState(true);

    // defaultInterception=false (blocklist mode), blocklist empty → allow
    auto ret = screenUnlockInterceptor.DoProcess(param);
    EXPECT_EQ(ret, ERR_OK);
    GTEST_LOG_(INFO) << "DoProcess_ThirdPartyExtension_DefaultInterceptionFalse_NoList end";
}

/**
 * @tc.name: CheckSystemAppExt_SystemAppInterceptionTrue_EmptyAllowList_SkipIPC
 * @tc.desc: systemAppInterception=true + empty allowlist → skip IPC, directly block
 * @tc.type: FUNC
 */
HWTEST_F(ScreenUnlockInterceptorCoverageTest,
    CheckSystemAppExt_SystemAppInterceptionTrue_EmptyAllowList_SkipIPC, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckSystemAppExt_SystemAppInterceptionTrue_EmptyAllowList_SkipIPC start";
    const std::string configStr = R"({
        "ams_extension_config": [{
            "name": "FormExtension",
            "extension_type_name": "form",
            "screen_unlock_access": {
                "systemAppInterception": true
            }
        }]
    })";
    LoadTestConfig(configStr);

    ScreenUnlockInterceptor screenUnlockInterceptor;
    auto ret = screenUnlockInterceptor.CheckSystemAppExtensionInterception("form", "com.test.bundle");
    EXPECT_EQ(ret, ERR_BLOCK_START_FIRST_BOOT_SCREEN_UNLOCK);
    GTEST_LOG_(INFO) << "CheckSystemAppExt_SystemAppInterceptionTrue_EmptyAllowList_SkipIPC end";
}

/**
 * @tc.name: CheckSystemAppExt_SystemAppInterceptionFalse_EmptyBlockList_SkipIPC
 * @tc.desc: systemAppInterception=false + empty blocklist → skip IPC, directly allow
 * @tc.type: FUNC
 */
HWTEST_F(ScreenUnlockInterceptorCoverageTest,
    CheckSystemAppExt_SystemAppInterceptionFalse_EmptyBlockList_SkipIPC, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckSystemAppExt_SystemAppInterceptionFalse_EmptyBlockList_SkipIPC start";
    const std::string configStr = R"({
        "ams_extension_config": [{
            "name": "FormExtension",
            "extension_type_name": "form",
            "screen_unlock_access": {
                "systemAppInterception": false
            }
        }]
    })";
    LoadTestConfig(configStr);

    ScreenUnlockInterceptor screenUnlockInterceptor;
    auto ret = screenUnlockInterceptor.CheckSystemAppExtensionInterception("form", "com.test.bundle");
    EXPECT_EQ(ret, ERR_OK);
    GTEST_LOG_(INFO) << "CheckSystemAppExt_SystemAppInterceptionFalse_EmptyBlockList_SkipIPC end";
}

/**
 * @tc.name: CheckSystemAppExt_DefaultInterceptionTrue_EmptyAllowList_SkipIPC
 * @tc.desc: no systemAppInterception + defaultInterception=true + empty allowlist → skip IPC, directly block
 * @tc.type: FUNC
 */
HWTEST_F(ScreenUnlockInterceptorCoverageTest,
    CheckSystemAppExt_DefaultInterceptionTrue_EmptyAllowList_SkipIPC, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckSystemAppExt_DefaultInterceptionTrue_EmptyAllowList_SkipIPC start";
    const std::string configStr = R"({
        "ams_extension_config": [{
            "name": "FormExtension",
            "extension_type_name": "form",
            "screen_unlock_access": {
                "defaultInterception": true
            }
        }]
    })";
    LoadTestConfig(configStr);

    ScreenUnlockInterceptor screenUnlockInterceptor;
    auto ret = screenUnlockInterceptor.CheckSystemAppExtensionInterception("form", "com.test.bundle");
    EXPECT_EQ(ret, ERR_BLOCK_START_FIRST_BOOT_SCREEN_UNLOCK);
    GTEST_LOG_(INFO) << "CheckSystemAppExt_DefaultInterceptionTrue_EmptyAllowList_SkipIPC end";
}

/**
 * @tc.name: CheckSystemAppExt_DefaultInterceptionFalse_EmptyBlockList_SkipIPC
 * @tc.desc: no systemAppInterception + defaultInterception=false + empty blocklist → skip IPC, directly allow
 * @tc.type: FUNC
 */
HWTEST_F(ScreenUnlockInterceptorCoverageTest,
    CheckSystemAppExt_DefaultInterceptionFalse_EmptyBlockList_SkipIPC, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckSystemAppExt_DefaultInterceptionFalse_EmptyBlockList_SkipIPC start";
    const std::string configStr = R"({
        "ams_extension_config": [{
            "name": "FormExtension",
            "extension_type_name": "form",
            "screen_unlock_access": {
                "defaultInterception": false
            }
        }]
    })";
    LoadTestConfig(configStr);

    ScreenUnlockInterceptor screenUnlockInterceptor;
    auto ret = screenUnlockInterceptor.CheckSystemAppExtensionInterception("form", "com.test.bundle");
    EXPECT_EQ(ret, ERR_OK);
    GTEST_LOG_(INFO) << "CheckSystemAppExt_DefaultInterceptionFalse_EmptyBlockList_SkipIPC end";
}

/**
 * @tc.name: CheckSystemAppExt_SystemAppInterceptionTrue_NonEmptyAllowList_NeedIPC
 * @tc.desc: systemAppInterception=true + non-empty allowlist → IPC called but fails in test env → block
 * @tc.type: FUNC
 */
HWTEST_F(ScreenUnlockInterceptorCoverageTest,
    CheckSystemAppExt_SystemAppInterceptionTrue_NonEmptyAllowList_NeedIPC, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckSystemAppExt_SystemAppInterceptionTrue_NonEmptyAllowList_NeedIPC start";
    const std::string configStr = R"({
        "ams_extension_config": [{
            "name": "FormExtension",
            "extension_type_name": "form",
            "screen_unlock_access": {
                "systemAppInterception": true,
                "allowlist": [{"appIdentifier": "app_id"}]
            }
        }]
    })";
    LoadTestConfig(configStr);

    ScreenUnlockInterceptor screenUnlockInterceptor;
    auto ret = screenUnlockInterceptor.CheckSystemAppExtensionInterception("form", "com.test.bundle");
    // IPC fails in test env, appIdentifier="" not in allowlist → block
    EXPECT_EQ(ret, ERR_BLOCK_START_FIRST_BOOT_SCREEN_UNLOCK);
    GTEST_LOG_(INFO) << "CheckSystemAppExt_SystemAppInterceptionTrue_NonEmptyAllowList_NeedIPC end";
}

/**
 * @tc.name: CheckSystemAppExt_SystemAppInterceptionFalse_NonEmptyBlockList_NeedIPC
 * @tc.desc: systemAppInterception=false + non-empty blocklist → IPC called but fails in test env → allow
 * @tc.type: FUNC
 */
HWTEST_F(ScreenUnlockInterceptorCoverageTest,
    CheckSystemAppExt_SystemAppInterceptionFalse_NonEmptyBlockList_NeedIPC, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckSystemAppExt_SystemAppInterceptionFalse_NonEmptyBlockList_NeedIPC start";
    const std::string configStr = R"({
        "ams_extension_config": [{
            "name": "FormExtension",
            "extension_type_name": "form",
            "screen_unlock_access": {
                "systemAppInterception": false,
                "blocklist": [{"appIdentifier": "app_id"}]
            }
        }]
    })";
    LoadTestConfig(configStr);

    ScreenUnlockInterceptor screenUnlockInterceptor;
    auto ret = screenUnlockInterceptor.CheckSystemAppExtensionInterception("form", "com.test.bundle");
    // IPC fails in test env, appIdentifier="" not in blocklist → allow
    EXPECT_EQ(ret, ERR_OK);
    GTEST_LOG_(INFO) << "CheckSystemAppExt_SystemAppInterceptionFalse_NonEmptyBlockList_NeedIPC end";
}

/**
 * @tc.name: CheckThirdPartyExt_DefaultInterceptionTrue_EmptyAllowList_SkipIPC
 * @tc.desc: defaultInterception=true + empty allowlist → skip IPC, directly block
 * @tc.type: FUNC
 */
HWTEST_F(ScreenUnlockInterceptorCoverageTest,
    CheckThirdPartyExt_DefaultInterceptionTrue_EmptyAllowList_SkipIPC, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckThirdPartyExt_DefaultInterceptionTrue_EmptyAllowList_SkipIPC start";
    const std::string configStr = R"({
        "ams_extension_config": [{
            "name": "FormExtension",
            "extension_type_name": "form",
            "screen_unlock_access": {
                "defaultInterception": true
            }
        }]
    })";
    LoadTestConfig(configStr);

    ScreenUnlockInterceptor screenUnlockInterceptor;
    auto ret = screenUnlockInterceptor.CheckThirdPartyExtensionInterception("form", "com.test.bundle");
    EXPECT_EQ(ret, ERR_BLOCK_START_FIRST_BOOT_SCREEN_UNLOCK);
    GTEST_LOG_(INFO) << "CheckThirdPartyExt_DefaultInterceptionTrue_EmptyAllowList_SkipIPC end";
}

/**
 * @tc.name: CheckThirdPartyExt_DefaultInterceptionFalse_EmptyBlockList_SkipIPC
 * @tc.desc: defaultInterception=false + empty blocklist → skip IPC, directly allow
 * @tc.type: FUNC
 */
HWTEST_F(ScreenUnlockInterceptorCoverageTest,
    CheckThirdPartyExt_DefaultInterceptionFalse_EmptyBlockList_SkipIPC, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckThirdPartyExt_DefaultInterceptionFalse_EmptyBlockList_SkipIPC start";
    const std::string configStr = R"({
        "ams_extension_config": [{
            "name": "FormExtension",
            "extension_type_name": "form",
            "screen_unlock_access": {
                "defaultInterception": false
            }
        }]
    })";
    LoadTestConfig(configStr);

    ScreenUnlockInterceptor screenUnlockInterceptor;
    auto ret = screenUnlockInterceptor.CheckThirdPartyExtensionInterception("form", "com.test.bundle");
    EXPECT_EQ(ret, ERR_OK);
    GTEST_LOG_(INFO) << "CheckThirdPartyExt_DefaultInterceptionFalse_EmptyBlockList_SkipIPC end";
}

/**
 * @tc.name: CheckThirdPartyExt_DefaultInterceptionTrue_NonEmptyAllowList_NeedIPC
 * @tc.desc: defaultInterception=true + non-empty allowlist → IPC fails in test env → block
 * @tc.type: FUNC
 */
HWTEST_F(ScreenUnlockInterceptorCoverageTest,
    CheckThirdPartyExt_DefaultInterceptionTrue_NonEmptyAllowList_NeedIPC, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckThirdPartyExt_DefaultInterceptionTrue_NonEmptyAllowList_NeedIPC start";
    const std::string configStr = R"({
        "ams_extension_config": [{
            "name": "FormExtension",
            "extension_type_name": "form",
            "screen_unlock_access": {
                "defaultInterception": true,
                "allowlist": [{"appIdentifier": "app_id"}]
            }
        }]
    })";
    LoadTestConfig(configStr);

    ScreenUnlockInterceptor screenUnlockInterceptor;
    auto ret = screenUnlockInterceptor.CheckThirdPartyExtensionInterception("form", "com.test.bundle");
    // IPC fails in test env, appIdentifier="" not in allowlist → block
    EXPECT_EQ(ret, ERR_BLOCK_START_FIRST_BOOT_SCREEN_UNLOCK);
    GTEST_LOG_(INFO) << "CheckThirdPartyExt_DefaultInterceptionTrue_NonEmptyAllowList_NeedIPC end";
}

/**
 * @tc.name: CheckThirdPartyExt_DefaultInterceptionFalse_NonEmptyBlockList_NeedIPC
 * @tc.desc: defaultInterception=false + non-empty blocklist → IPC fails in test env → allow
 * @tc.type: FUNC
 */
HWTEST_F(ScreenUnlockInterceptorCoverageTest,
    CheckThirdPartyExt_DefaultInterceptionFalse_NonEmptyBlockList_NeedIPC, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckThirdPartyExt_DefaultInterceptionFalse_NonEmptyBlockList_NeedIPC start";
    const std::string configStr = R"({
        "ams_extension_config": [{
            "name": "FormExtension",
            "extension_type_name": "form",
            "screen_unlock_access": {
                "defaultInterception": false,
                "blocklist": [{"appIdentifier": "app_id"}]
            }
        }]
    })";
    LoadTestConfig(configStr);

    ScreenUnlockInterceptor screenUnlockInterceptor;
    auto ret = screenUnlockInterceptor.CheckThirdPartyExtensionInterception("form", "com.test.bundle");
    // IPC fails in test env, appIdentifier="" not in blocklist → allow
    EXPECT_EQ(ret, ERR_OK);
    GTEST_LOG_(INFO) << "CheckThirdPartyExt_DefaultInterceptionFalse_NonEmptyBlockList_NeedIPC end";
}

} // namespace AAFwk
} // namespace OHOS
