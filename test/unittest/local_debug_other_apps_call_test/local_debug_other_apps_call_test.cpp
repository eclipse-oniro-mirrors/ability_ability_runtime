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
#include "accesstoken_kit.h"
#include "hilog_tag_wrapper.h"
#include "mock_my_status.h"
#define private public
#define protected public
#include "permission_verification.h"
#undef private
#undef protected

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AAFwk {
class LocalDebugOtherAppsCallTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp() override;
    void TearDown() override;
};

void LocalDebugOtherAppsCallTest::SetUpTestCase(void)
{}

void LocalDebugOtherAppsCallTest::TearDownTestCase(void)
{}

void LocalDebugOtherAppsCallTest::SetUp()
{
    MyStatus::GetInstance().getBoolParameter_ = false;
    MyStatus::GetInstance().isVerifyAccessToken_ = 1; // PERMISSION_DENIED by default
    MyStatus::GetInstance().ipcGetCallingUid_ = 1; // uid 1 -> non-sandbox user by default
}

void LocalDebugOtherAppsCallTest::TearDown()
{
    MyStatus::GetInstance().getBoolParameter_ = false;
    MyStatus::GetInstance().isVerifyAccessToken_ = 1; // PERMISSION_DENIED by default
    MyStatus::GetInstance().ipcGetCallingUid_ = 1; // uid 1 -> non-sandbox user by default
}

constexpr uint32_t SPECIFY_TOKEN_ID = 100;
constexpr int32_t TARGET_USER_ID = 5;
constexpr int32_t SAME_USER_ID = 10;
constexpr int32_t BASE_USER_RANGE = 200000;
constexpr int32_t SANDBOX_USER_UID = SAME_USER_ID * BASE_USER_RANGE + 1000;

/**
 * @tc.name: IsLocalDebugOtherAppsCall_0100
 * @tc.desc: not in developer mode, IsLocalDebugOtherAppsCall should return false even if permission granted
 * @tc.type: FUNC
 */
HWTEST_F(LocalDebugOtherAppsCallTest, IsLocalDebugOtherAppsCall_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "IsLocalDebugOtherAppsCall_0100 start");
    MyStatus::GetInstance().getBoolParameter_ = false;
    MyStatus::GetInstance().isVerifyAccessToken_ =
        Security::AccessToken::PermissionState::PERMISSION_GRANTED;
    bool result = PermissionVerification::GetInstance()->IsLocalDebugOtherAppsCall();
    EXPECT_FALSE(result);
    TAG_LOGI(AAFwkTag::TEST, "IsLocalDebugOtherAppsCall_0100 end");
}

/**
 * @tc.name: IsLocalDebugOtherAppsCall_0200
 * @tc.desc: in developer mode but caller has no LOCAL_DEBUG_OTHER_APPS permission, return false
 * @tc.type: FUNC
 */
HWTEST_F(LocalDebugOtherAppsCallTest, IsLocalDebugOtherAppsCall_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "IsLocalDebugOtherAppsCall_0200 start");
    MyStatus::GetInstance().getBoolParameter_ = true;
    MyStatus::GetInstance().isVerifyAccessToken_ = 1; // PERMISSION_DENIED
    bool result = PermissionVerification::GetInstance()->IsLocalDebugOtherAppsCall();
    EXPECT_FALSE(result);
    TAG_LOGI(AAFwkTag::TEST, "IsLocalDebugOtherAppsCall_0200 end");
}

/**
 * @tc.name: IsLocalDebugOtherAppsCall_0300
 * @tc.desc: in developer mode and caller has LOCAL_DEBUG_OTHER_APPS permission, return true
 * @tc.type: FUNC
 */
HWTEST_F(LocalDebugOtherAppsCallTest, IsLocalDebugOtherAppsCall_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "IsLocalDebugOtherAppsCall_0300 start");
    MyStatus::GetInstance().getBoolParameter_ = true;
    MyStatus::GetInstance().isVerifyAccessToken_ =
        Security::AccessToken::PermissionState::PERMISSION_GRANTED;
    bool result = PermissionVerification::GetInstance()->IsLocalDebugOtherAppsCall();
    EXPECT_TRUE(result);
    TAG_LOGI(AAFwkTag::TEST, "IsLocalDebugOtherAppsCall_0300 end");
}

/**
 * @tc.name: IsLocalDebugOtherAppsCall_0400
 * @tc.desc: not in developer mode, IsLocalDebugOtherAppsCall should return false
 *           even if permission granted and caller is sandbox user
 * @tc.type: FUNC
 */
HWTEST_F(LocalDebugOtherAppsCallTest, IsLocalDebugOtherAppsCall_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "IsLocalDebugOtherAppsCall_0400 start");
    MyStatus::GetInstance().getBoolParameter_ = false;
    MyStatus::GetInstance().isVerifyAccessToken_ =
        Security::AccessToken::PermissionState::PERMISSION_GRANTED;
    MyStatus::GetInstance().ipcGetCallingUid_ = SANDBOX_USER_UID;
    bool result = PermissionVerification::GetInstance()->IsLocalDebugOtherAppsCall(SPECIFY_TOKEN_ID,
        TARGET_USER_ID);
    EXPECT_FALSE(result);
    TAG_LOGI(AAFwkTag::TEST, "IsLocalDebugOtherAppsCall_0400 end");
}

/**
 * @tc.name: IsLocalDebugOtherAppsCall_0500
 * @tc.desc: in developer mode, permission granted with specifyTokenId, targetUserId is -1
 *           (no cross-user check), should return true
 * @tc.type: FUNC
 */
HWTEST_F(LocalDebugOtherAppsCallTest, IsLocalDebugOtherAppsCall_0500, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "IsLocalDebugOtherAppsCall_0500 start");
    MyStatus::GetInstance().getBoolParameter_ = true;
    MyStatus::GetInstance().isVerifyAccessToken_ =
        Security::AccessToken::PermissionState::PERMISSION_GRANTED;
    MyStatus::GetInstance().ipcGetCallingUid_ = SANDBOX_USER_UID;
    bool result = PermissionVerification::GetInstance()->IsLocalDebugOtherAppsCall(SPECIFY_TOKEN_ID, -1);
    EXPECT_TRUE(result);
    TAG_LOGI(AAFwkTag::TEST, "IsLocalDebugOtherAppsCall_0500 end");
}

/**
 * @tc.name: IsLocalDebugOtherAppsCall_0600
 * @tc.desc: in developer mode, permission granted with specifyTokenId, target user equals
 *           caller sandbox user, should return true
 * @tc.type: FUNC
 */
HWTEST_F(LocalDebugOtherAppsCallTest, IsLocalDebugOtherAppsCall_0600, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "IsLocalDebugOtherAppsCall_0600 start");
    MyStatus::GetInstance().getBoolParameter_ = true;
    MyStatus::GetInstance().isVerifyAccessToken_ =
        Security::AccessToken::PermissionState::PERMISSION_GRANTED;
    MyStatus::GetInstance().ipcGetCallingUid_ = SANDBOX_USER_UID;
    bool result = PermissionVerification::GetInstance()->IsLocalDebugOtherAppsCall(SPECIFY_TOKEN_ID,
        SAME_USER_ID);
    EXPECT_TRUE(result);
    TAG_LOGI(AAFwkTag::TEST, "IsLocalDebugOtherAppsCall_0600 end");
}

/**
 * @tc.name: IsLocalDebugOtherAppsCall_0700
 * @tc.desc: in developer mode, permission granted but target user differs from caller
 *           sandbox user, cross-user operation should return false
 * @tc.type: FUNC
 */
HWTEST_F(LocalDebugOtherAppsCallTest, IsLocalDebugOtherAppsCall_0700, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "IsLocalDebugOtherAppsCall_0700 start");
    MyStatus::GetInstance().getBoolParameter_ = true;
    MyStatus::GetInstance().isVerifyAccessToken_ =
        Security::AccessToken::PermissionState::PERMISSION_GRANTED;
    MyStatus::GetInstance().ipcGetCallingUid_ = SANDBOX_USER_UID;
    bool result = PermissionVerification::GetInstance()->IsLocalDebugOtherAppsCall(SPECIFY_TOKEN_ID,
        TARGET_USER_ID);
    EXPECT_FALSE(result);
    TAG_LOGI(AAFwkTag::TEST, "IsLocalDebugOtherAppsCall_0700 end");
}

/**
 * @tc.name: IsLocalDebugOtherAppsCall_0800
 * @tc.desc: in developer mode but caller has no LOCAL_DEBUG_OTHER_APPS permission,
 *           should return false even if same sandbox user
 * @tc.type: FUNC
 */
HWTEST_F(LocalDebugOtherAppsCallTest, IsLocalDebugOtherAppsCall_0800, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "IsLocalDebugOtherAppsCall_0800 start");
    MyStatus::GetInstance().getBoolParameter_ = true;
    MyStatus::GetInstance().isVerifyAccessToken_ = 1; // PERMISSION_DENIED
    MyStatus::GetInstance().ipcGetCallingUid_ = SANDBOX_USER_UID;
    bool result = PermissionVerification::GetInstance()->IsLocalDebugOtherAppsCall(SPECIFY_TOKEN_ID,
        SAME_USER_ID);
    EXPECT_FALSE(result);
    TAG_LOGI(AAFwkTag::TEST, "IsLocalDebugOtherAppsCall_0800 end");
}

/**
 * @tc.name: IsCustomSandboxCall_0100
 * @tc.desc: not in developer mode, IsCustomSandboxCall should return false even if permission granted
 * @tc.type: FUNC
 */
HWTEST_F(LocalDebugOtherAppsCallTest, IsCustomSandboxCall_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "IsCustomSandboxCall_0100 start");
    MyStatus::GetInstance().getBoolParameter_ = false;
    MyStatus::GetInstance().isVerifyAccessToken_ =
        Security::AccessToken::PermissionState::PERMISSION_GRANTED;
    bool result = PermissionVerification::GetInstance()->IsCustomSandboxCall();
    EXPECT_FALSE(result);
    TAG_LOGI(AAFwkTag::TEST, "IsCustomSandboxCall_0100 end");
}

/**
 * @tc.name: IsCustomSandboxCall_0200
 * @tc.desc: in developer mode but caller has no CUSTOM_SANDBOX permission, return false
 * @tc.type: FUNC
 */
HWTEST_F(LocalDebugOtherAppsCallTest, IsCustomSandboxCall_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "IsCustomSandboxCall_0200 start");
    MyStatus::GetInstance().getBoolParameter_ = true;
    MyStatus::GetInstance().isVerifyAccessToken_ = 1; // PERMISSION_DENIED
    bool result = PermissionVerification::GetInstance()->IsCustomSandboxCall();
    EXPECT_FALSE(result);
    TAG_LOGI(AAFwkTag::TEST, "IsCustomSandboxCall_0200 end");
}

/**
 * @tc.name: IsCustomSandboxCall_0300
 * @tc.desc: in developer mode and caller has CUSTOM_SANDBOX permission, return true
 * @tc.type: FUNC
 */
HWTEST_F(LocalDebugOtherAppsCallTest, IsCustomSandboxCall_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "IsCustomSandboxCall_0300 start");
    MyStatus::GetInstance().getBoolParameter_ = true;
    MyStatus::GetInstance().isVerifyAccessToken_ =
        Security::AccessToken::PermissionState::PERMISSION_GRANTED;
    bool result = PermissionVerification::GetInstance()->IsCustomSandboxCall();
    EXPECT_TRUE(result);
    TAG_LOGI(AAFwkTag::TEST, "IsCustomSandboxCall_0300 end");
}

/**
 * @tc.name: IsCustomSandboxCall_0400
 * @tc.desc: in developer mode, permission granted with specifyTokenId, target user equals
 *           caller sandbox user, should return true
 * @tc.type: FUNC
 */
HWTEST_F(LocalDebugOtherAppsCallTest, IsCustomSandboxCall_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "IsCustomSandboxCall_0400 start");
    MyStatus::GetInstance().getBoolParameter_ = true;
    MyStatus::GetInstance().isVerifyAccessToken_ =
        Security::AccessToken::PermissionState::PERMISSION_GRANTED;
    MyStatus::GetInstance().ipcGetCallingUid_ = SANDBOX_USER_UID;
    bool result = PermissionVerification::GetInstance()->IsCustomSandboxCall(SPECIFY_TOKEN_ID,
        SAME_USER_ID);
    EXPECT_TRUE(result);
    TAG_LOGI(AAFwkTag::TEST, "IsCustomSandboxCall_0400 end");
}

/**
 * @tc.name: IsCustomSandboxCall_0500
 * @tc.desc: in developer mode, permission granted but target user differs from caller
 *           sandbox user, cross-user operation should return false
 * @tc.type: FUNC
 */
HWTEST_F(LocalDebugOtherAppsCallTest, IsCustomSandboxCall_0500, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "IsCustomSandboxCall_0500 start");
    MyStatus::GetInstance().getBoolParameter_ = true;
    MyStatus::GetInstance().isVerifyAccessToken_ =
        Security::AccessToken::PermissionState::PERMISSION_GRANTED;
    MyStatus::GetInstance().ipcGetCallingUid_ = SANDBOX_USER_UID;
    bool result = PermissionVerification::GetInstance()->IsCustomSandboxCall(SPECIFY_TOKEN_ID,
        TARGET_USER_ID);
    EXPECT_FALSE(result);
    TAG_LOGI(AAFwkTag::TEST, "IsCustomSandboxCall_0500 end");
}

/**
 * @tc.name: IsCrossUserForSandboxCall_0100
 * @tc.desc: negative targetUserId means no cross-user check, return false
 * @tc.type: FUNC
 */
HWTEST_F(LocalDebugOtherAppsCallTest, IsCrossUserForSandboxCall_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "IsCrossUserForSandboxCall_0100 start");
    MyStatus::GetInstance().ipcGetCallingUid_ = SANDBOX_USER_UID;
    bool result = PermissionVerification::GetInstance()->IsCrossUserForSandboxCall(-1);
    EXPECT_FALSE(result);
    TAG_LOGI(AAFwkTag::TEST, "IsCrossUserForSandboxCall_0100 end");
}

/**
 * @tc.name: IsCrossUserForSandboxCall_0200
 * @tc.desc: caller uid maps to user 0/1 (non-sandbox), return false
 * @tc.type: FUNC
 */
HWTEST_F(LocalDebugOtherAppsCallTest, IsCrossUserForSandboxCall_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "IsCrossUserForSandboxCall_0200 start");
    MyStatus::GetInstance().ipcGetCallingUid_ = 1;
    bool result = PermissionVerification::GetInstance()->IsCrossUserForSandboxCall(TARGET_USER_ID);
    EXPECT_FALSE(result);
    TAG_LOGI(AAFwkTag::TEST, "IsCrossUserForSandboxCall_0200 end");
}

/**
 * @tc.name: IsCrossUserForSandboxCall_0300
 * @tc.desc: caller sandbox user differs from targetUserId, return true
 * @tc.type: FUNC
 */
HWTEST_F(LocalDebugOtherAppsCallTest, IsCrossUserForSandboxCall_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "IsCrossUserForSandboxCall_0300 start");
    MyStatus::GetInstance().ipcGetCallingUid_ = SANDBOX_USER_UID;
    bool result = PermissionVerification::GetInstance()->IsCrossUserForSandboxCall(TARGET_USER_ID);
    EXPECT_TRUE(result);
    TAG_LOGI(AAFwkTag::TEST, "IsCrossUserForSandboxCall_0300 end");
}

/**
 * @tc.name: IsCrossUserForSandboxCall_0400
 * @tc.desc: caller sandbox user equals targetUserId, return false
 * @tc.type: FUNC
 */
HWTEST_F(LocalDebugOtherAppsCallTest, IsCrossUserForSandboxCall_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "IsCrossUserForSandboxCall_0400 start");
    MyStatus::GetInstance().ipcGetCallingUid_ = SANDBOX_USER_UID;
    bool result = PermissionVerification::GetInstance()->IsCrossUserForSandboxCall(SAME_USER_ID);
    EXPECT_FALSE(result);
    TAG_LOGI(AAFwkTag::TEST, "IsCrossUserForSandboxCall_0400 end");
}
}  // namespace AAFwk
}  // namespace OHOS
