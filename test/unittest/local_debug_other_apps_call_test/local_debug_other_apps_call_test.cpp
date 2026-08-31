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
}

void LocalDebugOtherAppsCallTest::TearDown()
{
    MyStatus::GetInstance().getBoolParameter_ = false;
    MyStatus::GetInstance().isVerifyAccessToken_ = 1; // PERMISSION_DENIED by default
}

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
}  // namespace AAFwk
}  // namespace OHOS
