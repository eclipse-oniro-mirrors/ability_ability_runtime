/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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
class ScreenUnlockInterceptorTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    void LoadTestConfig(const std::string &configStr);
    void SetupExtensionAbilityInfo(bool isSystemApp, const std::string &appIdentifier,
        bool allowAppRunWhenDeviceFirstLocked = false);

public:
    static std::shared_ptr<ExtensionConfig> extensionConfig_;
};

std::shared_ptr<ExtensionConfig> ScreenUnlockInterceptorTest::extensionConfig_ =
    DelayedSingleton<ExtensionConfig>::GetInstance();

void ScreenUnlockInterceptorTest::SetUpTestCase()
{}

void ScreenUnlockInterceptorTest::TearDownTestCase()
{}

void ScreenUnlockInterceptorTest::SetUp()
{
    extensionConfig_->configMap_.clear();
}

void ScreenUnlockInterceptorTest::TearDown()
{}

void ScreenUnlockInterceptorTest::LoadTestConfig(const std::string &configStr)
{
    nlohmann::json jsonConfig = nlohmann::json::parse(configStr);
    extensionConfig_->LoadExtensionConfig(jsonConfig);
}

void ScreenUnlockInterceptorTest::SetupExtensionAbilityInfo(bool isSystemApp, const std::string &appIdentifier,
    bool allowAppRunWhenDeviceFirstLocked)
{
    StartAbilityUtils::startAbilityInfo = std::make_shared<StartAbilityInfo>();
    StartAbilityUtils::startAbilityInfo->abilityInfo.type = AbilityType::EXTENSION;
    StartAbilityUtils::startAbilityInfo->abilityInfo.extensionTypeName = "form";
    StartAbilityUtils::startAbilityInfo->abilityInfo.applicationInfo.isSystemApp = isSystemApp;
    StartAbilityUtils::startAbilityInfo->abilityInfo.applicationInfo.allowAppRunWhenDeviceFirstLocked =
        allowAppRunWhenDeviceFirstLocked;
    StartAbilityUtils::startAbilityInfo->abilityInfo.applicationInfo.bundleName = "com.test.bundle";
}

/**
 * @tc.name: ScreenUnlockInterceptorTest_DoProcess_001
 * @tc.desc: DoProcess
 * @tc.type: FUNC
 * @tc.require: issueI5I0DY
 */
HWTEST_F(ScreenUnlockInterceptorTest, DoProcess_WhenScreenLocked_001, TestSize.Level1)
{
    ScreenUnlockInterceptor screenUnlockInterceptor;
    Want want;
    int requestCode = 123;
    int32_t userId = 1001;
    bool isWithUI = true;
    sptr<IRemoteObject> callerToken;
    std::function<bool(void)> shouldBlockAllAppStartFunc = []() -> bool {
        return false;
    };
    StartAbilityUtils::startAbilityInfo = std::make_shared<StartAbilityInfo>();
    AbilityInterceptorParam param(want, requestCode, userId, isWithUI, callerToken, shouldBlockAllAppStartFunc);
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        auto screenLockManager = OHOS::ScreenLock::ScreenLockManager::GetInstance();
        EXPECT_NE(screenLockManager, nullptr);
        screenLockManager->SetScreenLockedState(true);
        auto ret = screenUnlockInterceptor.DoProcess(param);
        bool isLocked = screenLockManager->IsScreenLocked();
        EXPECT_TRUE(isLocked);
        EXPECT_EQ(ret, ERR_BLOCK_START_FIRST_BOOT_SCREEN_UNLOCK);
    }
}

/**
 * @tc.name: ScreenUnlockInterceptorTest_DoProcess_001
 * @tc.desc: DoProcess
 * @tc.type: FUNC
 * @tc.require: issueI5I0DY
 */
HWTEST_F(ScreenUnlockInterceptorTest, DoProcess_WhenScreenUnLocked_001, TestSize.Level1)
{
    ScreenUnlockInterceptor screenUnlockInterceptor;
    Want want;
    int requestCode = 123;
    int32_t userId = 1001;
    bool isWithUI = true;
    sptr<IRemoteObject> callerToken;
    std::function<bool(void)> shouldBlockAllAppStartFunc = []() -> bool {
        return false;
    };
    StartAbilityUtils::startAbilityInfo = std::make_shared<StartAbilityInfo>();
    AbilityInterceptorParam param(want, requestCode, userId, isWithUI, callerToken, shouldBlockAllAppStartFunc);
    auto ret = screenUnlockInterceptor.DoProcess(param);
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        auto screenLockManager = OHOS::ScreenLock::ScreenLockManager::GetInstance();
        EXPECT_NE(screenLockManager, nullptr);
        screenLockManager->SetScreenLockedState(false);
        auto ret = screenUnlockInterceptor.DoProcess(param);
        bool isLocked = screenLockManager->IsScreenLocked();
        EXPECT_FALSE(isLocked);
        EXPECT_EQ(ret, ERR_OK);
    }
}

/**
 * @tc.name: ScreenUnlockInterceptorTest_DoProcess_004
 * @tc.desc: DoProcess
 * @tc.type: FUNC
 * @tc.require: issueI5I0DY
 */
HWTEST_F(ScreenUnlockInterceptorTest, DoProcess_004, TestSize.Level1)
{
    ScreenUnlockInterceptor screenUnlockInterceptor;
    Want want;
    int requestCode = 123;
    int32_t userId = 1001;
    bool isWithUI = true;
    sptr<IRemoteObject> callerToken;
    std::function<bool(void)> shouldBlockAllAppStartFunc = []() -> bool {
        return false;
    };
    StartAbilityUtils::startAbilityInfo = nullptr;
    AbilityInterceptorParam param(want, requestCode, userId, isWithUI, callerToken, shouldBlockAllAppStartFunc);
    auto ret = screenUnlockInterceptor.DoProcess(param);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: ScreenUnlockInterceptorTest_DoProcess_ScreenUnlocked
 * @tc.desc: DoProcess
 * @tc.type: FUNC
 */
HWTEST_F(ScreenUnlockInterceptorTest, DoProcess_ScreenUnlocked, TestSize.Level1)
{
    ScreenUnlockInterceptor screenUnlockInterceptor;
    Want want;
    int requestCode = 123;
    int32_t userId = 1001;
    bool isWithUI = true;
    sptr<IRemoteObject> callerToken;
    std::function<bool(void)> shouldBlockAllAppStartFunc = []() -> bool {
        return false;
    };
    StartAbilityUtils::startAbilityInfo = std::make_shared<StartAbilityInfo>();
    AppExecFwk::AbilityInfo targetAbilityInfo;
    targetAbilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    StartAbilityUtils::startAbilityInfo->abilityInfo = targetAbilityInfo;
    StartAbilityUtils::startAbilityInfo->abilityInfo.applicationInfo.allowAppRunWhenDeviceFirstLocked = false;
    StartAbilityUtils::startAbilityInfo->abilityInfo.applicationInfo.isSystemApp = true;
    AbilityInterceptorParam param(want, requestCode, userId, isWithUI, callerToken, shouldBlockAllAppStartFunc);
    auto screenLockManager = OHOS::ScreenLock::ScreenLockManager::GetInstance();
    EXPECT_NE(screenLockManager, nullptr);
    screenLockManager->SetScreenLockedState(false);
    auto ret = screenUnlockInterceptor.DoProcess(param);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: CheckExtensionInterception_NoConfig_ShouldBlock
 * @tc.desc: Test CheckExtensionInterception when no screen_unlock_access config exists
 * @tc.type: FUNC
 */
HWTEST_F(ScreenUnlockInterceptorTest, CheckExtensionInterception_NoConfig_ShouldBlock, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckExtensionInterception_NoConfig_ShouldBlock start";
    ScreenUnlockInterceptor screenUnlockInterceptor;
    auto ret = screenUnlockInterceptor.CheckExtensionInterception("form", "com.test.bundle", true);
    EXPECT_EQ(ret, ERR_BLOCK_START_FIRST_BOOT_SCREEN_UNLOCK);
    GTEST_LOG_(INFO) << "CheckExtensionInterception_NoConfig_ShouldBlock end";
}

/**
 * @tc.name: CheckExtensionInterception_SystemApp_SystemAppInterceptionTrue_NotInAllowList
 * @tc.desc: Test system app extension with systemAppInterception=true, IPC fails → not in allowlist
 * @tc.type: FUNC
 */
HWTEST_F(ScreenUnlockInterceptorTest,
    CheckExtensionInterception_SystemApp_SystemAppInterceptionTrue_NotInAllowList, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckExtensionInterception_SystemApp_SystemAppInterceptionTrue_NotInAllowList start";
    const std::string configStr = R"({
        "ams_extension_config": [{
            "name": "FormExtension",
            "extension_type_name": "form",
            "screen_unlock_access": {
                "systemAppInterception": true,
                "allowlist": [
                    {"appIdentifier": "other_app_id"}
                ]
            }
        }]
    })";
    LoadTestConfig(configStr);

    ScreenUnlockInterceptor screenUnlockInterceptor;
    auto ret = screenUnlockInterceptor.CheckExtensionInterception("form", "com.test.bundle", true);
    // IPC fails in test env, appIdentifier="" not in allowlist → block
    EXPECT_EQ(ret, ERR_BLOCK_START_FIRST_BOOT_SCREEN_UNLOCK);
    GTEST_LOG_(INFO) << "CheckExtensionInterception_SystemApp_SystemAppInterceptionTrue_NotInAllowList end";
}

/**
 * @tc.name: CheckExtensionInterception_SystemApp_SystemAppInterceptionFalse_NotInBlockList
 * @tc.desc: Test system app extension with systemAppInterception=false, IPC fails → not in blocklist
 * @tc.type: FUNC
 */
HWTEST_F(ScreenUnlockInterceptorTest,
    CheckExtensionInterception_SystemApp_SystemAppInterceptionFalse_NotInBlockList, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckExtensionInterception_SystemApp_SystemAppInterceptionFalse_NotInBlockList start";
    const std::string configStr = R"({
        "ams_extension_config": [{
            "name": "FormExtension",
            "extension_type_name": "form",
            "screen_unlock_access": {
                "systemAppInterception": false,
                "blocklist": [
                    {"appIdentifier": "other_app_id"}
                ]
            }
        }]
    })";
    LoadTestConfig(configStr);

    ScreenUnlockInterceptor screenUnlockInterceptor;
    auto ret = screenUnlockInterceptor.CheckExtensionInterception("form", "com.test.bundle", true);
    // IPC fails in test env, appIdentifier="" not in blocklist → allow
    EXPECT_EQ(ret, ERR_OK);
    GTEST_LOG_(INFO) << "CheckExtensionInterception_SystemApp_SystemAppInterceptionFalse_NotInBlockList end";
}

/**
 * @tc.name: CheckExtensionInterception_SystemApp_UseDefaultInterceptionFalse_NotInBlockList
 * @tc.desc: Test system app extension using defaultInterception=false (no systemAppInterception)
 * @tc.type: FUNC
 */
HWTEST_F(ScreenUnlockInterceptorTest,
    CheckExtensionInterception_SystemApp_UseDefaultInterceptionFalse_NotInBlockList, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckExtensionInterception_SystemApp_UseDefaultInterceptionFalse_NotInBlockList start";
    const std::string configStr = R"({
        "ams_extension_config": [{
            "name": "FormExtension",
            "extension_type_name": "form",
            "screen_unlock_access": {
                "defaultInterception": false,
                "blocklist": [
                    {"appIdentifier": "other_app_id"}
                ]
            }
        }]
    })";
    LoadTestConfig(configStr);

    ScreenUnlockInterceptor screenUnlockInterceptor;
    auto ret = screenUnlockInterceptor.CheckExtensionInterception("form", "com.test.bundle", true);
    // IPC fails in test env, appIdentifier="" not in blocklist → allow
    EXPECT_EQ(ret, ERR_OK);
    GTEST_LOG_(INFO) << "CheckExtensionInterception_SystemApp_UseDefaultInterceptionFalse_NotInBlockList end";
}

/**
 * @tc.name: CheckExtensionInterception_ThirdParty_NoDefaultInterception_ShouldBlock
 * @tc.desc: Test third-party extension with no defaultInterception config
 * @tc.type: FUNC
 */
HWTEST_F(ScreenUnlockInterceptorTest,
    CheckExtensionInterception_ThirdParty_NoDefaultInterception_ShouldBlock, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckExtensionInterception_ThirdParty_NoDefaultInterception_ShouldBlock start";
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
    auto ret = screenUnlockInterceptor.CheckExtensionInterception("form", "com.test.bundle", false);
    EXPECT_EQ(ret, ERR_BLOCK_START_FIRST_BOOT_SCREEN_UNLOCK);
    GTEST_LOG_(INFO) << "CheckExtensionInterception_ThirdParty_NoDefaultInterception_ShouldBlock end";
}

/**
 * @tc.name: CheckExtensionInterception_ThirdParty_DefaultInterceptionTrue_NotInAllowList
 * @tc.desc: Test third-party extension with defaultInterception=true, IPC fails → not in allowlist
 * @tc.type: FUNC
 */
HWTEST_F(ScreenUnlockInterceptorTest,
    CheckExtensionInterception_ThirdParty_DefaultInterceptionTrue_NotInAllowList, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckExtensionInterception_ThirdParty_DefaultInterceptionTrue_NotInAllowList start";
    const std::string configStr = R"({
        "ams_extension_config": [{
            "name": "FormExtension",
            "extension_type_name": "form",
            "screen_unlock_access": {
                "defaultInterception": true,
                "allowlist": [
                    {"appIdentifier": "other_app_id"}
                ]
            }
        }]
    })";
    LoadTestConfig(configStr);

    ScreenUnlockInterceptor screenUnlockInterceptor;
    auto ret = screenUnlockInterceptor.CheckExtensionInterception("form", "com.test.bundle", false);
    // IPC fails in test env, appIdentifier="" not in allowlist → block
    EXPECT_EQ(ret, ERR_BLOCK_START_FIRST_BOOT_SCREEN_UNLOCK);
    GTEST_LOG_(INFO) << "CheckExtensionInterception_ThirdParty_DefaultInterceptionTrue_NotInAllowList end";
}

/**
 * @tc.name: CheckExtensionInterception_ThirdParty_DefaultInterceptionFalse_NotInBlockList
 * @tc.desc: Test third-party extension with defaultInterception=false, IPC fails → not in blocklist
 * @tc.type: FUNC
 */
HWTEST_F(ScreenUnlockInterceptorTest,
    CheckExtensionInterception_ThirdParty_DefaultInterceptionFalse_NotInBlockList, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckExtensionInterception_ThirdParty_DefaultInterceptionFalse_NotInBlockList start";
    const std::string configStr = R"({
        "ams_extension_config": [{
            "name": "FormExtension",
            "extension_type_name": "form",
            "screen_unlock_access": {
                "defaultInterception": false,
                "blocklist": [
                    {"appIdentifier": "other_app_id"}
                ]
            }
        }]
    })";
    LoadTestConfig(configStr);

    ScreenUnlockInterceptor screenUnlockInterceptor;
    auto ret = screenUnlockInterceptor.CheckExtensionInterception("form", "com.test.bundle", false);
    // IPC fails in test env, appIdentifier="" not in blocklist → allow
    EXPECT_EQ(ret, ERR_OK);
    GTEST_LOG_(INFO) << "CheckExtensionInterception_ThirdParty_DefaultInterceptionFalse_NotInBlockList end";
}

/**
 * @tc.name: DoProcess_NonSystemUIAbility_ShouldBlock
 * @tc.desc: Test non-system UIAbility should be blocked when screen locked
 * @tc.type: FUNC
 */
HWTEST_F(ScreenUnlockInterceptorTest, DoProcess_NonSystemUIAbility_ShouldBlock, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DoProcess_NonSystemUIAbility_ShouldBlock start";
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
    StartAbilityUtils::startAbilityInfo->abilityInfo.type = AbilityType::PAGE;
    StartAbilityUtils::startAbilityInfo->abilityInfo.applicationInfo.isSystemApp = false;
    StartAbilityUtils::startAbilityInfo->abilityInfo.applicationInfo.allowAppRunWhenDeviceFirstLocked = false;

    Want want;
    AbilityInterceptorParam param(want, 0, 100, true, nullptr, []() { return false; });

    auto screenLockManager = OHOS::ScreenLock::ScreenLockManager::GetInstance();
    EXPECT_NE(screenLockManager, nullptr);
    screenLockManager->SetScreenLockedState(true);

    auto ret = screenUnlockInterceptor.DoProcess(param);
    EXPECT_EQ(ret, ERR_BLOCK_START_FIRST_BOOT_SCREEN_UNLOCK);
    GTEST_LOG_(INFO) << "DoProcess_NonSystemUIAbility_ShouldBlock end";
}

/**
 * @tc.name: DoProcess_SystemAppExtension_WithConfig
 * @tc.desc: Test system app extension with proper config
 * @tc.type: FUNC
 */
HWTEST_F(ScreenUnlockInterceptorTest, DoProcess_SystemAppExtension_WithConfig, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DoProcess_SystemAppExtension_WithConfig start";
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
    SetupExtensionAbilityInfo(true, "test_app_id", true);

    Want want;
    AbilityInterceptorParam param(want, 0, 100, true, nullptr, []() { return false; });

    auto screenLockManager = OHOS::ScreenLock::ScreenLockManager::GetInstance();
    EXPECT_NE(screenLockManager, nullptr);
    screenLockManager->SetScreenLockedState(true);

    auto ret = screenUnlockInterceptor.DoProcess(param);
    // systemAppInterception=false, blocklist empty → skip IPC → allow
    EXPECT_EQ(ret, ERR_OK);
    GTEST_LOG_(INFO) << "DoProcess_SystemAppExtension_WithConfig end";
}

/**
 * @tc.name: DoProcess_ThirdPartyExtension_WithConfig
 * @tc.desc: Test third-party extension with proper config
 * @tc.type: FUNC
 */
HWTEST_F(ScreenUnlockInterceptorTest, DoProcess_ThirdPartyExtension_WithConfig, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DoProcess_ThirdPartyExtension_WithConfig start";
    const std::string configStr = R"({
        "ams_extension_config": [{
            "name": "FormExtension",
            "extension_type_name": "form",
            "screen_unlock_access": {
                "defaultInterception": false,
                "blocklist": [
                    {"appIdentifier": "blocked_app_id"}
                ]
            }
        }]
    })";
    LoadTestConfig(configStr);

    ScreenUnlockInterceptor screenUnlockInterceptor;
    SetupExtensionAbilityInfo(false, "test_app_id");

    Want want;
    AbilityInterceptorParam param(want, 0, 100, true, nullptr, []() { return false; });

    auto screenLockManager = OHOS::ScreenLock::ScreenLockManager::GetInstance();
    EXPECT_NE(screenLockManager, nullptr);
    screenLockManager->SetScreenLockedState(true);

    auto ret = screenUnlockInterceptor.DoProcess(param);
    // IPC fails in test env, appIdentifier="" not in blocklist → allow
    EXPECT_EQ(ret, ERR_OK);
    GTEST_LOG_(INFO) << "DoProcess_ThirdPartyExtension_WithConfig end";
}

/**
 * @tc.name: CheckExtensionInterception_EmptyBundleName
 * @tc.desc: Test CheckExtensionInterception with empty bundleName, IPC returns empty appIdentifier
 * @tc.type: FUNC
 */
HWTEST_F(ScreenUnlockInterceptorTest, CheckExtensionInterception_EmptyBundleName, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckExtensionInterception_EmptyBundleName start";
    const std::string configStr = R"({
        "ams_extension_config": [{
            "name": "FormExtension",
            "extension_type_name": "form",
            "screen_unlock_access": {
                "defaultInterception": true,
                "allowlist": [
                    {"appIdentifier": "test_app_id"}
                ]
            }
        }]
    })";
    LoadTestConfig(configStr);

    ScreenUnlockInterceptor screenUnlockInterceptor;
    // Empty bundleName → GetAppIdentifier returns "" → not in allowlist → block
    auto ret = screenUnlockInterceptor.CheckExtensionInterception("form", "", false);
    EXPECT_EQ(ret, ERR_BLOCK_START_FIRST_BOOT_SCREEN_UNLOCK);
    GTEST_LOG_(INFO) << "CheckExtensionInterception_EmptyBundleName end";
}

/**
 * @tc.name: CheckExtensionInterception_UnknownExtensionType
 * @tc.desc: Test CheckExtensionInterception with unknown extension type
 * @tc.type: FUNC
 */
HWTEST_F(ScreenUnlockInterceptorTest, CheckExtensionInterception_UnknownExtensionType, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckExtensionInterception_UnknownExtensionType start";
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
    auto ret = screenUnlockInterceptor.CheckExtensionInterception("unknown_type", "com.test.bundle", false);
    EXPECT_EQ(ret, ERR_BLOCK_START_FIRST_BOOT_SCREEN_UNLOCK);
    GTEST_LOG_(INFO) << "CheckExtensionInterception_UnknownExtensionType end";
}

/**
 * @tc.name: CheckExtensionInterception_SystemApp_NoInterceptionConfig
 * @tc.desc: Test system app extension with screen_unlock_access but no interception config
 * @tc.type: FUNC
 */
HWTEST_F(ScreenUnlockInterceptorTest, CheckExtensionInterception_SystemApp_NoInterceptionConfig, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckExtensionInterception_SystemApp_NoInterceptionConfig start";
    const std::string configStr = R"({
        "ams_extension_config": [{
            "name": "FormExtension",
            "extension_type_name": "form",
            "screen_unlock_access": {
                "intercept": true,
                "intercept_exclude_system_app": true
            }
        }]
    })";
    LoadTestConfig(configStr);

    ScreenUnlockInterceptor screenUnlockInterceptor;
    auto ret = screenUnlockInterceptor.CheckExtensionInterception("form", "com.test.bundle", true);
    EXPECT_EQ(ret, ERR_BLOCK_START_FIRST_BOOT_SCREEN_UNLOCK);
    GTEST_LOG_(INFO) << "CheckExtensionInterception_SystemApp_NoInterceptionConfig end";
}
} // namespace AAFwk
} // namespace OHOS
