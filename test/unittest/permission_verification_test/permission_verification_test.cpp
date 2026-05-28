/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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
#include "ability_manager_errors.h"
#include "permission_verification.h"
#undef private
#undef protected

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AAFwk {
namespace {
#ifdef WITH_DLP
const std::string DLP_PARAMS_INDEX = "ohos.dlp.params.index";
#endif // WITH_DLP
const int32_t SHELL_START_EXTENSION_FLOOR = 0;
const int32_t SHELL_START_EXTENSION_MAX = 22; // EMBEDDED_UI
}
class PermissionVerificationTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp() override;
    void TearDown() override;
};

void PermissionVerificationTest::SetUpTestCase(void)
{}
void PermissionVerificationTest::TearDownTestCase(void)
{}
void PermissionVerificationTest::SetUp()
{}
void PermissionVerificationTest::TearDown()
{}

/**
 * @tc.name: StartUpAbilityPermissionCheck_0100
 * @tc.desc: CheckCallAbilityPermission Test
 * @tc.type: FUNC
 * @tc.require: issueI5QXCQ
 */
HWTEST_F(PermissionVerificationTest, StartUpAbilityPermissionCheck_0100, TestSize.Level0)
{
    AAFwk::PermissionVerification::VerificationInfo verificationInfo;
    verificationInfo.visible = true;
    verificationInfo.isBackgroundCall = false;
    int result = AAFwk::PermissionVerification::GetInstance()->CheckCallAbilityPermission(verificationInfo);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: StartUpAbilityPermissionCheck_0200
 * @tc.desc: StartUpAbilityPermissionCheck Test
 * @tc.type: FUNC
 * @tc.require: issueI5QXCQ
 */
HWTEST_F(PermissionVerificationTest, StartUpAbilityPermissionCheck_0200, TestSize.Level0)
{
    AAFwk::PermissionVerification::VerificationInfo verificationInfo;
    verificationInfo.visible = true;
    verificationInfo.isBackgroundCall = false;
    int result = AAFwk::PermissionVerification::GetInstance()->CheckStartByCallPermission(verificationInfo);
    EXPECT_EQ(result, CHECK_PERMISSION_FAILED);
}

/**
 * @tc.name: StartUpServicePermissionCheck_0100
 * @tc.desc: StartUpServicePermissionCheck Test
 * @tc.type: FUNC
 * @tc.require: issueI5PRJ7
 */
HWTEST_F(PermissionVerificationTest, StartUpServicePermissionCheck_0100, TestSize.Level0)
{
    AAFwk::PermissionVerification::VerificationInfo verificationInfo;
    verificationInfo.visible = true;
    verificationInfo.isBackgroundCall = false;
    int result = AAFwk::PermissionVerification::GetInstance()->CheckCallServiceAbilityPermission(verificationInfo);
    EXPECT_EQ(result, CHECK_PERMISSION_FAILED);
}

/**
 * @tc.name: StartUpServicePermissionCheck_0200
 * @tc.desc: StartUpServicePermissionCheck Test
 * @tc.type: FUNC
 * @tc.require: issueI5PRJ7
 */
HWTEST_F(PermissionVerificationTest, StartUpServicePermissionCheck_0200, TestSize.Level0)
{
    AAFwk::PermissionVerification::VerificationInfo verificationInfo;
    verificationInfo.visible = true;
    verificationInfo.isBackgroundCall = false;
    int result = AAFwk::PermissionVerification::GetInstance()->CheckCallServiceExtensionPermission(verificationInfo);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: StartUpServicePermissionCheck_0300
 * @tc.desc: StartUpServicePermissionCheck Test
 * @tc.type: FUNC
 * @tc.require: issueI5PRJ7
 */
HWTEST_F(PermissionVerificationTest, StartUpServicePermissionCheck_0300, TestSize.Level0)
{
    AAFwk::PermissionVerification::VerificationInfo verificationInfo;
    verificationInfo.visible = true;
    verificationInfo.isBackgroundCall = false;
    int result = AAFwk::PermissionVerification::GetInstance()->CheckCallDataAbilityPermission(verificationInfo, false);
    EXPECT_EQ(result, CHECK_PERMISSION_FAILED);
}

/**
 * @tc.name: CheckCallAbilityPermission_0100
 * @tc.desc: CheckCallAbilityPermission Test
 * @tc.type: FUNC
 * @tc.require: issueI5QXCQ
 */
HWTEST_F(PermissionVerificationTest, CheckCallAbilityPermission_0100, TestSize.Level1)
{
    AAFwk::PermissionVerification::VerificationInfo verificationInfo;
    verificationInfo.visible = true;
    verificationInfo.isBackgroundCall = false;
    int result = AAFwk::PermissionVerification::GetInstance()->CheckCallAbilityPermission(verificationInfo);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: CheckSpecificSystemAbilityAccessPermission_0100
 * @tc.desc: CheckSpecificSystemAbilityAccessPermission Test
 * @tc.type: FUNC
 * @tc.require: issueI5AFTJ
 */
HWTEST_F(PermissionVerificationTest, CheckSpecificSystemAbilityAccessPermission_0100, TestSize.Level1)
{
    std::string processName;
    bool result = AAFwk::PermissionVerification::GetInstance()->CheckSpecificSystemAbilityAccessPermission(processName);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: VerifyControllerPerm_0100
 * @tc.desc: VerifyControllerPerm Test
 * @tc.type: FUNC
 * @tc.require: issueI4WS9F
 */
HWTEST_F(PermissionVerificationTest, VerifyControllerPerm_0100, TestSize.Level0)
{
    bool result = AAFwk::PermissionVerification::GetInstance()->VerifyControllerPerm();
    EXPECT_FALSE(result);
}

#ifdef WITH_DLP
/**
 * @tc.name: VerifyDlpPermission_0100
 * @tc.desc: VerifyDlpPermission Test
 * @tc.type: FUNC
 * @tc.require: issueI55UJZ
 */
HWTEST_F(PermissionVerificationTest, VerifyDlpPermission_0100, TestSize.Level1)
{
    Want want;
    want.SetParam(DLP_PARAMS_INDEX, 1);
    bool result = AAFwk::PermissionVerification::GetInstance()->VerifyDlpPermission(want);
    EXPECT_FALSE(result);
}
#endif // WITH_DLP

/**
 * @tc.name: VerifyMissionPermission_0100
 * @tc.desc: VerifyMissionPermission Test
 * @tc.type: FUNC
 * @tc.require: issueI5I8XP
 */
HWTEST_F(PermissionVerificationTest, VerifyMissionPermission_0100, TestSize.Level1)
{
    bool result = AAFwk::PermissionVerification::GetInstance()->VerifyMissionPermission();
    EXPECT_FALSE(result);
}

/**
 * @tc.name: VerifyInstallBundlePermission_0100
 * @tc.desc: VerifyInstallBundlePermission Test
 * @tc.type: FUNC
 * @tc.require: issueI581VW
 */
HWTEST_F(PermissionVerificationTest, VerifyInstallBundlePermission_0100, TestSize.Level2)
{
    bool result = AAFwk::PermissionVerification::GetInstance()->VerifyInstallBundlePermission();
    EXPECT_TRUE(result);
}

/**
 * @tc.name: VerifyGetBundleInfoPrivilegedPermission_0100
 * @tc.desc: VerifyGetBundleInfoPrivilegedPermission Test
 * @tc.type: FUNC
 * @tc.require: issueI581VW
 */
HWTEST_F(PermissionVerificationTest, VerifyGetBundleInfoPrivilegedPermission_0100, TestSize.Level2)
{
    bool result = AAFwk::PermissionVerification::GetInstance()->VerifyGetBundleInfoPrivilegedPermission();
    EXPECT_TRUE(result);
}

/**
 * @tc.name: CheckCallAbilityPermission_0200
 * @tc.desc: CheckCallAbilityPermission Test
 * @tc.type: FUNC
 * @tc.require: issueI5QXCQ
 */
HWTEST_F(PermissionVerificationTest, CheckCallAbilityPermission_0200, TestSize.Level1)
{
    AAFwk::PermissionVerification::VerificationInfo verificationInfo;
    verificationInfo.visible = true;
    verificationInfo.isBackgroundCall = true;
    int result = AAFwk::PermissionVerification::GetInstance()->CheckCallAbilityPermission(verificationInfo);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: CheckCallAbilityPermission_0300
 * @tc.desc: CheckCallAbilityPermission Test
 * @tc.type: FUNC
 * @tc.require: issueI5QXCQ
 */
HWTEST_F(PermissionVerificationTest, CheckCallAbilityPermission_0300, TestSize.Level1)
{
    AAFwk::PermissionVerification::VerificationInfo verificationInfo;
    verificationInfo.visible = false;
    verificationInfo.isBackgroundCall = false;
    int code = AAFwk::PermissionVerification::GetInstance()->CheckCallAbilityPermission(verificationInfo);
    bool result = (code == ABILITY_VISIBLE_FALSE_DENY_REQUEST) || (code == ERR_OK);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: CheckCallAbilityPermission_0400
 * @tc.desc: visible=true and isBackgroundCall=false, all checks pass directly
 * @tc.type: FUNC
 * @tc.require: issueI5QXCQ
 */
HWTEST_F(PermissionVerificationTest, CheckCallAbilityPermission_0400, TestSize.Level1)
{
    AAFwk::PermissionVerification::VerificationInfo verificationInfo;
    verificationInfo.visible = true;
    verificationInfo.isBackgroundCall = false;
    int result = AAFwk::PermissionVerification::GetInstance()->CheckCallAbilityPermission(verificationInfo);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: CheckCallAbilityPermission_0500
 * @tc.desc: visible=false and isBackgroundCall=true, most restrictive case
 * @tc.type: FUNC
 * @tc.require: issueI5QXCQ
 */
HWTEST_F(PermissionVerificationTest, CheckCallAbilityPermission_0500, TestSize.Level1)
{
    AAFwk::PermissionVerification::VerificationInfo verificationInfo;
    verificationInfo.visible = false;
    verificationInfo.isBackgroundCall = true;
    int code = AAFwk::PermissionVerification::GetInstance()->CheckCallAbilityPermission(verificationInfo);
    bool result = (code == ABILITY_VISIBLE_FALSE_DENY_REQUEST) || (code == CHECK_PERMISSION_FAILED) ||
        (code == ERR_OK);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: CheckCallAbilityPermission_0600
 * @tc.desc: isCallByShortcut=true with visible=false, bypass invisible check
 * @tc.type: FUNC
 * @tc.require: issueI5QXCQ
 */
HWTEST_F(PermissionVerificationTest, CheckCallAbilityPermission_0600, TestSize.Level1)
{
    AAFwk::PermissionVerification::VerificationInfo verificationInfo;
    verificationInfo.visible = false;
    verificationInfo.isBackgroundCall = false;
    bool isCallByShortcut = true;
    int result = AAFwk::PermissionVerification::GetInstance()->CheckCallAbilityPermission(
        verificationInfo, isCallByShortcut);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: CheckCallAbilityPermission_0700
 * @tc.desc: isCliToolToken and isLaunchAbility with visible=false, bypass invisible check
 * @tc.type: FUNC
 * @tc.require: issueI5QXCQ
 */
HWTEST_F(PermissionVerificationTest, CheckCallAbilityPermission_0700, TestSize.Level1)
{
    AAFwk::PermissionVerification::VerificationInfo verificationInfo;
    verificationInfo.visible = false;
    verificationInfo.isBackgroundCall = false;
    verificationInfo.isCliToolToken = true;
    verificationInfo.isLaunchAbility = true;
    int result = AAFwk::PermissionVerification::GetInstance()->CheckCallAbilityPermission(verificationInfo);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: CheckStartByCallPermission_0100
 * @tc.desc: CheckStartByCallPermission Test
 * @tc.type: FUNC
 * @tc.require: issueI5QXCQ
 */
HWTEST_F(PermissionVerificationTest, CheckStartByCallPermission_0100, TestSize.Level2)
{
    AAFwk::PermissionVerification::VerificationInfo verificationInfo;
    verificationInfo.visible = true;
    verificationInfo.isBackgroundCall = false;
    int result = AAFwk::PermissionVerification::GetInstance()->CheckStartByCallPermission(verificationInfo);
    EXPECT_EQ(result, CHECK_PERMISSION_FAILED);
}

/**
 * @tc.name: CheckStartByCallPermission_0200
 * @tc.desc: CheckStartByCallPermission Test
 * @tc.type: FUNC
 * @tc.require: issueI5QXCQ
 */
HWTEST_F(PermissionVerificationTest, CheckStartByCallPermission_0200, TestSize.Level2)
{
    AAFwk::PermissionVerification::VerificationInfo verificationInfo;
    verificationInfo.visible = false;
    int result = AAFwk::PermissionVerification::GetInstance()->CheckStartByCallPermission(verificationInfo);
    EXPECT_EQ(result, CHECK_PERMISSION_FAILED);
}

/**
 * @tc.name: CheckStartByCallPermission_0300
 * @tc.desc: CheckStartByCallPermission Test
 * @tc.type: FUNC
 * @tc.require: issueI5QXCQ
 */
HWTEST_F(PermissionVerificationTest, CheckStartByCallPermission_0300, TestSize.Level2)
{
    AAFwk::PermissionVerification::VerificationInfo verificationInfo;
    verificationInfo.visible = true;
    verificationInfo.isBackgroundCall = true;
    int result = AAFwk::PermissionVerification::GetInstance()->CheckStartByCallPermission(verificationInfo);
    EXPECT_EQ(result, CHECK_PERMISSION_FAILED);
}

/**
 * @tc.name: CheckCallServiceAbilityPermission_0100
 * @tc.desc: CheckCallServiceAbilityPermission Test
 * @tc.type: FUNC
 * @tc.require: issueI5PRJ7
 */
HWTEST_F(PermissionVerificationTest, CheckCallServiceAbilityPermission_0100, TestSize.Level2)
{
    AAFwk::PermissionVerification::VerificationInfo verificationInfo;
    int api9 = 9;
    verificationInfo.apiTargetVersion = api9;
    verificationInfo.visible = false;
    verificationInfo.isBackgroundCall = true;
    verificationInfo.associatedWakeUp = false;
    int code = AAFwk::PermissionVerification::GetInstance()->CheckCallServiceAbilityPermission(verificationInfo);
    bool result = (code == ABILITY_VISIBLE_FALSE_DENY_REQUEST) || (code == CHECK_PERMISSION_FAILED);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: CheckCallServiceAbilityPermission_0200
 * @tc.desc: CheckCallServiceAbilityPermission Test
 * @tc.type: FUNC
 * @tc.require: issueI5PRJ7
 */
HWTEST_F(PermissionVerificationTest, CheckCallServiceAbilityPermission_0200, TestSize.Level2)
{
    AAFwk::PermissionVerification::VerificationInfo verificationInfo;
    int api9 = 9;
    verificationInfo.apiTargetVersion = api9;
    verificationInfo.visible = true;
    verificationInfo.isBackgroundCall = false;
    verificationInfo.associatedWakeUp = false;
    int result = AAFwk::PermissionVerification::GetInstance()->CheckCallServiceAbilityPermission(verificationInfo);
    EXPECT_EQ(result, CHECK_PERMISSION_FAILED);
}

/**
 * @tc.name: CheckCallServiceAbilityPermission_0300
 * @tc.desc: CheckCallServiceAbilityPermission Test
 * @tc.type: FUNC
 * @tc.require: issueI5PRJ7
 */
HWTEST_F(PermissionVerificationTest, CheckCallServiceAbilityPermission_0300, TestSize.Level2)
{
    AAFwk::PermissionVerification::VerificationInfo verificationInfo;
    int api9 = 9;
    verificationInfo.apiTargetVersion = api9;
    verificationInfo.visible = true;
    verificationInfo.isBackgroundCall = false;
    verificationInfo.associatedWakeUp = true;
    int result = AAFwk::PermissionVerification::GetInstance()->CheckCallServiceAbilityPermission(verificationInfo);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: CheckCallServiceAbilityPermission_0400
 * @tc.desc: CheckCallServiceAbilityPermission Test
 * @tc.type: FUNC
 * @tc.require: issueI5PRJ7
 */
HWTEST_F(PermissionVerificationTest, CheckCallServiceAbilityPermission_0400, TestSize.Level2)
{
    AAFwk::PermissionVerification::VerificationInfo verificationInfo;
    int api9 = 9;
    verificationInfo.apiTargetVersion = api9;
    verificationInfo.visible = false;
    verificationInfo.isBackgroundCall = false;
    verificationInfo.associatedWakeUp = false;
    int code = AAFwk::PermissionVerification::GetInstance()->CheckCallServiceAbilityPermission(verificationInfo);
    bool result = (code == ABILITY_VISIBLE_FALSE_DENY_REQUEST) || (code == CHECK_PERMISSION_FAILED);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: CheckCallServiceAbilityPermission_0500
 * @tc.desc: CheckCallServiceAbilityPermission Test
 * @tc.type: FUNC
 * @tc.require: issueI5PRJ7
 */
HWTEST_F(PermissionVerificationTest, CheckCallServiceAbilityPermission_0500, TestSize.Level2)
{
    AAFwk::PermissionVerification::VerificationInfo verificationInfo;
    int api8 = 8;
    verificationInfo.apiTargetVersion = api8;
    verificationInfo.visible = true;
    verificationInfo.isBackgroundCall = false;
    verificationInfo.associatedWakeUp = true;
    int result = AAFwk::PermissionVerification::GetInstance()->CheckCallServiceAbilityPermission(verificationInfo);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: CheckCallServiceAbilityPermission_0600
 * @tc.desc: CheckCallServiceAbilityPermission Test
 * @tc.type: FUNC
 * @tc.require: issueI5PRJ7
 */
HWTEST_F(PermissionVerificationTest, CheckCallServiceAbilityPermission_0600, TestSize.Level2)
{
    AAFwk::PermissionVerification::VerificationInfo verificationInfo;
    int api8 = 8;
    verificationInfo.apiTargetVersion = api8;
    verificationInfo.visible = true;
    verificationInfo.isBackgroundCall = false;
    verificationInfo.associatedWakeUp = false;
    int result = AAFwk::PermissionVerification::GetInstance()->CheckCallServiceAbilityPermission(verificationInfo);
    EXPECT_EQ(result, CHECK_PERMISSION_FAILED);
}

/**
 * @tc.name: CheckCallServiceAbilityPermission_0700
 * @tc.desc: CheckCallServiceAbilityPermission Test
 * @tc.type: FUNC
 * @tc.require: issueI5PRJ7
 */
HWTEST_F(PermissionVerificationTest, CheckCallServiceAbilityPermission_0700, TestSize.Level2)
{
    AAFwk::PermissionVerification::VerificationInfo verificationInfo;
    int api8 = 8;
    verificationInfo.apiTargetVersion = api8;
    verificationInfo.visible = false;
    verificationInfo.isBackgroundCall = false;
    verificationInfo.associatedWakeUp = false;
    int code = AAFwk::PermissionVerification::GetInstance()->CheckCallServiceAbilityPermission(verificationInfo);
    bool result = (code == ABILITY_VISIBLE_FALSE_DENY_REQUEST) || (code == CHECK_PERMISSION_FAILED);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: CheckCallServiceAbilityPermission_0800
 * @tc.desc: CheckCallServiceAbilityPermission Test
 * @tc.type: FUNC
 * @tc.require: issueI5PRJ7
 */
HWTEST_F(PermissionVerificationTest, CheckCallServiceAbilityPermission_0800, TestSize.Level2)
{
    AAFwk::PermissionVerification::VerificationInfo verificationInfo;
    int api9 = 9;
    verificationInfo.apiTargetVersion = api9;
    verificationInfo.isBackgroundCall = true;
    int code = AAFwk::PermissionVerification::GetInstance()->CheckCallServiceAbilityPermission(verificationInfo);
    bool result = (code == ABILITY_VISIBLE_FALSE_DENY_REQUEST) || (code == CHECK_PERMISSION_FAILED);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: CheckCallServiceAbilityPermission_0900
 * @tc.desc: CheckCallServiceAbilityPermission Test
 * @tc.type: FUNC
 * @tc.require: issueI5PRJ7
 */
HWTEST_F(PermissionVerificationTest, CheckCallServiceAbilityPermission_0900, TestSize.Level2)
{
    AAFwk::PermissionVerification::VerificationInfo verificationInfo;
    int api8 = 8;
    verificationInfo.apiTargetVersion = api8;
    verificationInfo.isBackgroundCall = false;
    verificationInfo.visible = false;
    int code = AAFwk::PermissionVerification::GetInstance()->CheckCallServiceAbilityPermission(verificationInfo);
    bool result = (code == ABILITY_VISIBLE_FALSE_DENY_REQUEST) || (code == CHECK_PERMISSION_FAILED);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: CheckCallServiceAbilityPermission_1000
 * @tc.desc: CheckCallServiceAbilityPermission Test
 * @tc.type: FUNC
 * @tc.require: issueI5PRJ7
 */
HWTEST_F(PermissionVerificationTest, CheckCallServiceAbilityPermission_1000, TestSize.Level2)
{
    AAFwk::PermissionVerification::VerificationInfo verificationInfo;
    int api8 = 8;
    verificationInfo.apiTargetVersion = api8;
    verificationInfo.isBackgroundCall = false;
    verificationInfo.visible = true;
    verificationInfo.associatedWakeUp = false;
    int result = AAFwk::PermissionVerification::GetInstance()->CheckCallServiceAbilityPermission(verificationInfo);
    EXPECT_EQ(result, CHECK_PERMISSION_FAILED);
}

/**
 * @tc.name: CheckCallServiceAbilityPermission_1100
 * @tc.desc: CheckCallServiceAbilityPermission with specified token id
 * @tc.type: FUNC
 */
HWTEST_F(PermissionVerificationTest, CheckCallServiceAbilityPermission_1100, TestSize.Level2)
{
    AAFwk::PermissionVerification::VerificationInfo verificationInfo;
    int api9 = 9;
    verificationInfo.apiTargetVersion = api9;
    verificationInfo.visible = true;
    verificationInfo.isBackgroundCall = false;
    verificationInfo.associatedWakeUp = true;
    uint32_t specifyTokenId = 0;
    int result = AAFwk::PermissionVerification::GetInstance()->CheckCallServiceAbilityPermission(
        verificationInfo, specifyTokenId);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: CheckCallServiceExtensionPermission_0100
 * @tc.desc: CheckCallServiceExtensionPermission Test
 * @tc.type: FUNC
 * @tc.require: issueI5PRJ7
 */
HWTEST_F(PermissionVerificationTest, CheckCallServiceExtensionPermission_0100, TestSize.Level2)
{
    AAFwk::PermissionVerification::VerificationInfo verificationInfo;
    verificationInfo.visible = true;
    verificationInfo.isBackgroundCall = false;
    int result = AAFwk::PermissionVerification::GetInstance()->CheckCallServiceExtensionPermission(verificationInfo);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: CheckCallServiceExtensionPermission_0200
 * @tc.desc: CheckCallServiceExtensionPermission Test
 * @tc.type: FUNC
 * @tc.require: issueI5PRJ7
 */
HWTEST_F(PermissionVerificationTest, CheckCallServiceExtensionPermission_0200, TestSize.Level2)
{
    AAFwk::PermissionVerification::VerificationInfo verificationInfo;
    verificationInfo.visible = false;
    verificationInfo.isBackgroundCall = false;
    int code = AAFwk::PermissionVerification::GetInstance()->CheckCallServiceExtensionPermission(verificationInfo);
    bool result = (code == ABILITY_VISIBLE_FALSE_DENY_REQUEST) || (code == ERR_OK);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: CheckCallServiceExtensionPermission_0300
 * @tc.desc: CheckCallServiceExtensionPermission Test
 * @tc.type: FUNC
 * @tc.require: issueI5PRJ7
 */
HWTEST_F(PermissionVerificationTest, CheckCallServiceExtensionPermission_0300, TestSize.Level2)
{
    AAFwk::PermissionVerification::VerificationInfo verificationInfo;
    verificationInfo.visible = true;
    verificationInfo.isBackgroundCall = true;
    int result = AAFwk::PermissionVerification::GetInstance()->CheckCallServiceExtensionPermission(verificationInfo);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: CheckCallDataAbilityPermission_0100
 * @tc.desc: CheckCallDataAbilityPermission Test
 * @tc.type: FUNC
 * @tc.require: issueI5PRJ7
 */
HWTEST_F(PermissionVerificationTest, CheckCallDataAbilityPermission_0100, TestSize.Level2)
{
    AAFwk::PermissionVerification::VerificationInfo verificationInfo;
    int api9 = 9;
    verificationInfo.apiTargetVersion = api9;
    verificationInfo.visible = false;
    verificationInfo.isBackgroundCall = true;
    verificationInfo.associatedWakeUp = false;
    int code = AAFwk::PermissionVerification::GetInstance()->CheckCallDataAbilityPermission(verificationInfo, false);
    bool result = (code == ABILITY_VISIBLE_FALSE_DENY_REQUEST) || (code == CHECK_PERMISSION_FAILED);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: CheckCallDataAbilityPermission_0200
 * @tc.desc: CheckCallDataAbilityPermission Test
 * @tc.type: FUNC
 * @tc.require: issueI5PRJ7
 */
HWTEST_F(PermissionVerificationTest, CheckCallDataAbilityPermission_0200, TestSize.Level2)
{
    AAFwk::PermissionVerification::VerificationInfo verificationInfo;
    int api9 = 9;
    verificationInfo.apiTargetVersion = api9;
    verificationInfo.visible = true;
    verificationInfo.isBackgroundCall = false;
    verificationInfo.associatedWakeUp = false;
    int result = AAFwk::PermissionVerification::GetInstance()->CheckCallDataAbilityPermission(verificationInfo, false);
    EXPECT_EQ(result, CHECK_PERMISSION_FAILED);
}

/**
 * @tc.name: CheckCallDataAbilityPermission_0300
 * @tc.desc: CheckCallDataAbilityPermission Test
 * @tc.type: FUNC
 * @tc.require: issueI5PRJ7
 */
HWTEST_F(PermissionVerificationTest, CheckCallDataAbilityPermission_0300, TestSize.Level2)
{
    AAFwk::PermissionVerification::VerificationInfo verificationInfo;
    int api9 = 9;
    verificationInfo.apiTargetVersion = api9;
    verificationInfo.visible = true;
    verificationInfo.isBackgroundCall = false;
    verificationInfo.associatedWakeUp = true;
    int result = AAFwk::PermissionVerification::GetInstance()->CheckCallDataAbilityPermission(verificationInfo, false);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: CheckCallDataAbilityPermission_0400
 * @tc.desc: CheckCallDataAbilityPermission Test
 * @tc.type: FUNC
 * @tc.require: issueI5PRJ7
 */
HWTEST_F(PermissionVerificationTest, CheckCallDataAbilityPermission_0400, TestSize.Level2)
{
    AAFwk::PermissionVerification::VerificationInfo verificationInfo;
    int api9 = 9;
    verificationInfo.apiTargetVersion = api9;
    verificationInfo.visible = false;
    verificationInfo.isBackgroundCall = false;
    verificationInfo.associatedWakeUp = false;
    int code = AAFwk::PermissionVerification::GetInstance()->CheckCallDataAbilityPermission(verificationInfo, false);
    bool result = (code == ABILITY_VISIBLE_FALSE_DENY_REQUEST) || (code == CHECK_PERMISSION_FAILED);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: CheckCallDataAbilityPermission_0500
 * @tc.desc: CheckCallDataAbilityPermission Test
 * @tc.type: FUNC
 * @tc.require: issueI5PRJ7
 */
HWTEST_F(PermissionVerificationTest, CheckCallDataAbilityPermission_0500, TestSize.Level2)
{
    AAFwk::PermissionVerification::VerificationInfo verificationInfo;
    int api8 = 8;
    verificationInfo.apiTargetVersion = api8;
    verificationInfo.visible = true;
    verificationInfo.isBackgroundCall = false;
    verificationInfo.associatedWakeUp = true;
    int result = AAFwk::PermissionVerification::GetInstance()->CheckCallDataAbilityPermission(verificationInfo, false);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: CheckCallDataAbilityPermission_0600
 * @tc.desc: CheckCallDataAbilityPermission Test
 * @tc.type: FUNC
 * @tc.require: issueI5PRJ7
 */
HWTEST_F(PermissionVerificationTest, CheckCallDataAbilityPermission_0600, TestSize.Level2)
{
    AAFwk::PermissionVerification::VerificationInfo verificationInfo;
    int api8 = 8;
    verificationInfo.apiTargetVersion = api8;
    verificationInfo.visible = true;
    verificationInfo.isBackgroundCall = false;
    verificationInfo.associatedWakeUp = false;
    int result = AAFwk::PermissionVerification::GetInstance()->CheckCallDataAbilityPermission(verificationInfo, false);
    EXPECT_EQ(result, CHECK_PERMISSION_FAILED);
}

/**
 * @tc.name: CheckCallDataAbilityPermission_0700
 * @tc.desc: CheckCallDataAbilityPermission Test
 * @tc.type: FUNC
 * @tc.require: issueI5PRJ7
 */
HWTEST_F(PermissionVerificationTest, CheckCallDataAbilityPermission_0700, TestSize.Level2)
{
    AAFwk::PermissionVerification::VerificationInfo verificationInfo;
    int api8 = 8;
    verificationInfo.apiTargetVersion = api8;
    verificationInfo.visible = false;
    verificationInfo.isBackgroundCall = false;
    verificationInfo.associatedWakeUp = false;
    int code = AAFwk::PermissionVerification::GetInstance()->CheckCallDataAbilityPermission(verificationInfo, false);
    bool result = (code == ABILITY_VISIBLE_FALSE_DENY_REQUEST) || (code == CHECK_PERMISSION_FAILED);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: CheckCallDataAbilityPermission_0800
 * @tc.desc: CheckCallDataAbilityPermission Test
 * @tc.type: FUNC
 * @tc.require: issueI5PRJ7
 */
HWTEST_F(PermissionVerificationTest, CheckCallDataAbilityPermission_0800, TestSize.Level2)
{
    AAFwk::PermissionVerification::VerificationInfo verificationInfo;
    int api9 = 9;
    verificationInfo.apiTargetVersion = api9;
    verificationInfo.isBackgroundCall = true;
    int code = AAFwk::PermissionVerification::GetInstance()->CheckCallDataAbilityPermission(verificationInfo, false);
    bool result = (code == ABILITY_VISIBLE_FALSE_DENY_REQUEST) || (code == CHECK_PERMISSION_FAILED);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: CheckCallDataAbilityPermission_0900
 * @tc.desc: CheckCallDataAbilityPermission Test
 * @tc.type: FUNC
 * @tc.require: issueI5PRJ7
 */
HWTEST_F(PermissionVerificationTest, CheckCallDataAbilityPermission_0900, TestSize.Level2)
{
    AAFwk::PermissionVerification::VerificationInfo verificationInfo;
    int api8 = 8;
    verificationInfo.apiTargetVersion = api8;
    verificationInfo.visible = false;
    verificationInfo.isBackgroundCall = false;

    int code = AAFwk::PermissionVerification::GetInstance()->CheckCallDataAbilityPermission(verificationInfo, false);
    bool result = (code == ABILITY_VISIBLE_FALSE_DENY_REQUEST) || (code == CHECK_PERMISSION_FAILED);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: CheckCallDataAbilityPermission_1000
 * @tc.desc: CheckCallDataAbilityPermission Test
 * @tc.type: FUNC
 * @tc.require: issueI5PRJ7
 */
HWTEST_F(PermissionVerificationTest, CheckCallDataAbilityPermission_1000, TestSize.Level2)
{
    AAFwk::PermissionVerification::VerificationInfo verificationInfo;
    int api8 = 8;
    verificationInfo.apiTargetVersion = api8;
    verificationInfo.visible = true;
    verificationInfo.isBackgroundCall = false;
    verificationInfo.associatedWakeUp = false;

    int result = AAFwk::PermissionVerification::GetInstance()->CheckCallDataAbilityPermission(verificationInfo, false);
    EXPECT_EQ(result, CHECK_PERMISSION_FAILED);
}

/**
 * @tc.name: JudgeCallerIsAllowedToUseSystemAPI_1000
 * @tc.desc: JudgeCallerIsAllowedToUseSystemAPI Test
 * @tc.type: FUNC
 * @tc.require: issuesI663VU
 */
HWTEST_F(PermissionVerificationTest, JudgeCallerIsAllowedToUseSystemAPI_1000, TestSize.Level2)
{
    bool result = AAFwk::PermissionVerification::GetInstance()->JudgeCallerIsAllowedToUseSystemAPI();
    EXPECT_EQ(result, true);
}

/**
 * @tc.name: CheckObserverCallerPermission_0001
 * @tc.desc: CheckObserverCallerPermission VerifyStartRecentAbilityPermission Test
 * @tc.type: FUNC
 * @tc.require:   VerifyRunningInfoPerm CheckObserverCallerPermission  etc
 */
HWTEST_F(PermissionVerificationTest, CheckObserverCallerPermission_0001, TestSize.Level2)
{
    AAFwk::PermissionVerification::GetInstance()->CheckObserverCallerPermission();
    AAFwk::PermissionVerification::GetInstance()->VerifyStartRecentAbilityPermission();
    auto ret = AAFwk::PermissionVerification::GetInstance()->VerifyRunningInfoPerm();
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: JudgeStartInvisibleAbility_0001
 * @tc.desc: JudgeStartInvisibleAbility VerifyPreStartAtomicServicePermission  Test
 * @tc.type: FUNC  VerifyKillProcessDependedOnWebPermission
 * @tc.require: VerifyPrepareTerminatePermission VerifyShellStartExtensionType VerifyPreloadApplicationPermission
 */
HWTEST_F(PermissionVerificationTest, JudgeStartInvisibleAbility_0001, TestSize.Level1)
{
    AAFwk::PermissionVerification::GetInstance()->VerifyKillProcessDependedOnWebPermission();
    AAFwk::PermissionVerification::GetInstance()->VerifyPreStartAtomicServicePermission();
    AAFwk::PermissionVerification::GetInstance()->VerifyPreloadApplicationPermission();
    AAFwk::PermissionVerification::GetInstance()->VerifyShellStartExtensionType(SHELL_START_EXTENSION_FLOOR);

    AAFwk::PermissionVerification::GetInstance()->VerifyShellStartExtensionType(SHELL_START_EXTENSION_MAX);
    int tokenId = 20;
    AAFwk::PermissionVerification::GetInstance()->VerifyPrepareTerminatePermission(tokenId);

    uint32_t aTkId = 1;
    uint32_t sTkId = 20;
    bool ret = AAFwk::PermissionVerification::GetInstance()->JudgeStartInvisibleAbility(aTkId, true, sTkId);
    EXPECT_EQ(ret, true);
    ret = AAFwk::PermissionVerification::GetInstance()->JudgeStartInvisibleAbility(aTkId, false, sTkId);
    aTkId = 20;
    ret = AAFwk::PermissionVerification::GetInstance()->JudgeStartInvisibleAbility(aTkId, false, sTkId);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: CheckVerifyStartLocalDebug_0100
 * @tc.desc: CheckVerifyStartLocalDebug Test
 * @tc.type: FUNC
 */
HWTEST_F(PermissionVerificationTest, CheckVerifyStartLocalDebug_0100, TestSize.Level2)
{
    int result = AAFwk::PermissionVerification::GetInstance()->VerifyStartLocalDebug(0);
    EXPECT_EQ(result, false);
}

/**
 * @tc.name: IsSystemAppCallByTokenId_0100
 * @tc.desc: IsSystemAppCallByTokenId Test
 * @tc.type: FUNC
 */
HWTEST_F(PermissionVerificationTest, IsSystemAppCallByTokenId_0100, TestSize.Level1)
{
    int result = AAFwk::PermissionVerification::GetInstance()->IsSystemAppCallByTokenId(0);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: IsSystemAppCallByTokenId_0200
 * @tc.desc: IsSystemAppCallByTokenId Test
 * @tc.type: FUNC
 */
HWTEST_F(PermissionVerificationTest, IsSystemAppCallByTokenId_0200, TestSize.Level1)
{
    int result = AAFwk::PermissionVerification::GetInstance()->IsSystemAppCallByTokenId(1);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: VerifyPrepareTerminatePermission_0100
 * @tc.desc: VerifyPrepareTerminatePermission Test
 * @tc.type: FUNC
 */
HWTEST_F(PermissionVerificationTest, VerifyPrepareTerminatePermission_0100, TestSize.Level1)
{
    int result = AAFwk::PermissionVerification::GetInstance()->VerifyPrepareTerminatePermission();
    EXPECT_FALSE(result);
}

/**
 * @tc.name: VerifyPreloadApplicationPermission_0100
 * @tc.desc: VerifyPreloadApplicationPermission Test
 * @tc.type: FUNC
 */
HWTEST_F(PermissionVerificationTest, VerifyPreloadApplicationPermission_0100, TestSize.Level1)
{
    int result = AAFwk::PermissionVerification::GetInstance()->VerifyPreloadApplicationPermission();
    EXPECT_FALSE(result);
}

/**
 * @tc.name: VerifyPreStartAtomicServicePermission_0100
 * @tc.desc: VerifyPreStartAtomicServicePermission Test
 * @tc.type: FUNC
 */
HWTEST_F(PermissionVerificationTest, VerifyPreStartAtomicServicePermission_0100, TestSize.Level1)
{
    int result = AAFwk::PermissionVerification::GetInstance()->VerifyPreStartAtomicServicePermission();
    EXPECT_FALSE(result);
}

/**
 * @tc.name: VerifyBlockAllAppStartPermission_0100
 * @tc.desc: VerifyBlockAllAppStartPermission Test
 * @tc.type: FUNC
 */
HWTEST_F(PermissionVerificationTest, VerifyBlockAllAppStartPermission_0100, TestSize.Level1)
{
    int result = AAFwk::PermissionVerification::GetInstance()->VerifyBlockAllAppStartPermission();
    EXPECT_FALSE(result);
}

/**
 * @tc.name: VerifySuperviseKiaServicePermission_0100
 * @tc.desc: VerifySuperviseKiaServicePermission Test
 * @tc.type: FUNC
 */
HWTEST_F(PermissionVerificationTest, VerifySuperviseKiaServicePermission_0100, TestSize.Level1)
{
    int result = AAFwk::PermissionVerification::GetInstance()->VerifySuperviseKiaServicePermission();
    EXPECT_FALSE(result);
}

/**
 * @tc.name: VerifyStartSelfUIAbility_0100
 * @tc.desc: VerifyStartSelfUIAbility Test
 * @tc.type: FUNC
 */
HWTEST_F(PermissionVerificationTest, VerifyStartSelfUIAbility_0100, TestSize.Level1)
{
    int result = AAFwk::PermissionVerification::GetInstance()->VerifyStartSelfUIAbility(0);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: VerifyFusionAccessPermission_0100
 * @tc.desc: VerifyFusionAccessPermission Test
 * @tc.type: FUNC
 */
HWTEST_F(PermissionVerificationTest, VerifyFusionAccessPermission_0100, TestSize.Level1)
{
    int result = AAFwk::PermissionVerification::GetInstance()->VerifyFusionAccessPermission();
    EXPECT_FALSE(result);
}

/**
 * @tc.name: VerifyContinuationNotifyPermission_0100
 * @tc.desc: VerifyContinuationNotifyPermission Test
 * @tc.type: FUNC
 */
HWTEST_F(PermissionVerificationTest, VerifyContinuationNotifyPermission_0100, TestSize.Level1)
{
    bool result = AAFwk::PermissionVerification::GetInstance()->VerifyContinuationNotifyPermission();
    EXPECT_FALSE(result);
}
}  // namespace AAFwk
}  // namespace OHOS
