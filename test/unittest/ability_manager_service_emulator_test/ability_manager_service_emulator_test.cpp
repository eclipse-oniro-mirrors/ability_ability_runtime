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
#undef private
#undef protected

#include "ability_manager_errors.h"
#include "ability_start_setting.h"
#include "start_ability_utils.h"
#include "start_options.h"
#include "uri.h"
#include "want.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AAFwk {
namespace {
const int32_t USER_ID_U100 = 100;
const int REQUESTCODE = 10;
const std::string APP_LINKING_ONLY = "appLinkingOnly";
const std::string ATOMIC_SERVICE = "ATOMIC_SERVICE";
const std::string APP = "APP";
const std::string TEST_BUNDLE_NAME = "com.example.emulatortest";
const std::string TEST_ABILITY_NAME = "MainAbililty";
}  // namespace

class AbilityManagerServiceEmulatorTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void AbilityManagerServiceEmulatorTest::SetUpTestCase() {}

void AbilityManagerServiceEmulatorTest::TearDownTestCase() {}

void AbilityManagerServiceEmulatorTest::SetUp() {}

void AbilityManagerServiceEmulatorTest::TearDown() {}

/**
 * @tc.name: StartAbilityInner_FreeInstall_001
 * @tc.desc: installation-free start is rejected with ERR_EMULATOR_NOT_SUPPORTED on emulator.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AbilityManagerServiceEmulatorTest, StartAbilityInner_FreeInstall_001, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs, nullptr);
    Want want;
    want.SetFlags(Want::FLAG_INSTALL_ON_DEMAND);
    StartAbilityWrapParam param = { .want = want };
    auto result = abilityMs->StartAbilityInner(param);
    EXPECT_EQ(result, ERR_EMULATOR_NOT_SUPPORTED);
}

/**
 * @tc.name: StartAbilityInner_NotFreeInstall_001
 * @tc.desc: normal start without free-install flag is not rejected by emulator guard.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AbilityManagerServiceEmulatorTest, StartAbilityInner_NotFreeInstall_001, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs, nullptr);
    Want want;
    want.SetElementName(TEST_BUNDLE_NAME, TEST_ABILITY_NAME);
    StartAbilityWrapParam param = { .want = want };
    auto result = abilityMs->StartAbilityInner(param);
    EXPECT_NE(result, ERR_EMULATOR_NOT_SUPPORTED);
}

/**
 * @tc.name: StartAbilityDetails_FreeInstall_001
 * @tc.desc: installation-free start is rejected with ERR_EMULATOR_NOT_SUPPORTED on emulator.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AbilityManagerServiceEmulatorTest, StartAbilityDetails_FreeInstall_001, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs, nullptr);
    Want want;
    want.SetFlags(Want::FLAG_INSTALL_ON_DEMAND);
    AbilityStartSetting abilityStartSetting;
    const sptr<IRemoteObject> callerToken;
    auto result = abilityMs->StartAbilityDetails(want, abilityStartSetting, callerToken, -1, 0);
    EXPECT_EQ(result, ERR_EMULATOR_NOT_SUPPORTED);
}

/**
 * @tc.name: StartAbilityForOptionInner_FreeInstall_001
 * @tc.desc: installation-free start is rejected with ERR_EMULATOR_NOT_SUPPORTED on emulator.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AbilityManagerServiceEmulatorTest, StartAbilityForOptionInner_FreeInstall_001, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs, nullptr);
    Want want;
    want.SetFlags(Want::FLAG_INSTALL_ON_DEMAND);
    StartOptions startOptions;
    const sptr<IRemoteObject> callerToken;
    auto result = abilityMs->StartAbilityForOptionInner(want, startOptions, callerToken, true, 0, 0, false, 0, false);
    EXPECT_EQ(result, ERR_EMULATOR_NOT_SUPPORTED);
}

/**
 * @tc.name: OpenLinkInner_AppLinkingOnly_001
 * @tc.desc: app linking (appLinkingOnly=true) is rejected with ERR_EMULATOR_NOT_SUPPORTED on emulator.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AbilityManagerServiceEmulatorTest, OpenLinkInner_AppLinkingOnly_001, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs, nullptr);
    Want want;
    want.SetParam(APP_LINKING_ONLY, true);
    auto result = abilityMs->OpenLinkInner(want, nullptr, USER_ID_U100, REQUESTCODE, true);
    EXPECT_EQ(result, ERR_EMULATOR_NOT_SUPPORTED);
}

/**
 * @tc.name: OpenLinkInner_App_001
 * @tc.desc: app linking to a normal app (appLinkingOnly=false) is not rejected by emulator guard.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AbilityManagerServiceEmulatorTest, OpenLinkInner_App_001, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs, nullptr);
    Want want;
    Uri uri(APP);
    want.SetUri(uri);
    want.SetParam(APP_LINKING_ONLY, false);
    auto result = abilityMs->OpenLinkInner(want, nullptr, USER_ID_U100, REQUESTCODE, true);
    EXPECT_NE(result, ERR_EMULATOR_NOT_SUPPORTED);
}

/**
 * @tc.name: OpenLinkInner_AtomicService_001
 * @tc.desc: opening an atomic service is rejected with ERR_EMULATOR_NOT_SUPPORTED on emulator.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AbilityManagerServiceEmulatorTest, OpenLinkInner_AtomicService_001, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs, nullptr);
    Want want;
    Uri uri(ATOMIC_SERVICE);
    want.SetUri(uri);
    auto result = abilityMs->OpenLinkInner(want, nullptr, USER_ID_U100, REQUESTCODE, true);
    EXPECT_EQ(result, ERR_EMULATOR_NOT_SUPPORTED);
}
}  // namespace AAFwk
}  // namespace OHOS
