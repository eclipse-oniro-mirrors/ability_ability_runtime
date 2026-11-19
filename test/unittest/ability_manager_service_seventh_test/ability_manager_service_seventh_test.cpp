/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#include "mock_ability_manager_service.h"
#include "ability_manager_errors.h"
#include "app_utils.h"
#include "hilog_tag_wrapper.h"
#include "permission_verification.h"
#include "process_options.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AppExecFwk;
namespace OHOS {
namespace AAFwk {
namespace {
const char* SUPPORT_BLOCK_ALL_APP_START = "persist.sys.abilityms.support_block_all_app_start";
}  // namespace
class AbilityManagerServiceSeventhTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void AbilityManagerServiceSeventhTest::SetUpTestCase() {}

void AbilityManagerServiceSeventhTest::TearDownTestCase() {}

void AbilityManagerServiceSeventhTest::SetUp() {}

void AbilityManagerServiceSeventhTest::TearDown() {}

/*
 * Feature: AbilityManagerService
 * Name: BlockAllAppStart_001
 * Function: BlockAllAppStart
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService BlockAllAppStart
 */
HWTEST_F(AbilityManagerServiceSeventhTest, BlockAllAppStart_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSeventhTest BlockAllAppStart_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    PermissionVerification::flag = PermissionVerification::FLAG::IS_SA_CALL;
    PermissionVerification::hasBlockAllAppStartPermission = true;
    AppUtils::isSupportBlockAllAppStart_ = true;
    EXPECT_EQ(AppUtils::GetInstance().IsSupportBlockAllAppStart(), true);
    EXPECT_EQ(PermissionVerification::GetInstance()->VerifyBlockAllAppStartPermission(), true);
    auto retCode = abilityMs_->BlockAllAppStart(true);
    EXPECT_EQ(retCode, ERR_OK);
    EXPECT_EQ(abilityMs_->shouldBlockAllAppStart_, true);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSeventhTest BlockAllAppStart_001 end");
}

/*
 * Feature: AbilityManagerService
 * Name: BlockAllAppStart_002
 * Function: BlockAllAppStart
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService BlockAllAppStart
 */
HWTEST_F(AbilityManagerServiceSeventhTest, BlockAllAppStart_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSeventhTest BlockAllAppStart_002 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    AppUtils::isSupportBlockAllAppStart_ = false;
    EXPECT_EQ(AppUtils::GetInstance().IsSupportBlockAllAppStart(), false);
    auto retCode = abilityMs_->BlockAllAppStart(true);
    EXPECT_EQ(retCode, ERR_PERMISSION_DENIED);
    EXPECT_EQ(abilityMs_->shouldBlockAllAppStart_, false);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSeventhTest BlockAllAppStart_002 end");
}

/*
 * Feature: AbilityManagerService
 * Name: BlockAllAppStart_003
 * Function: BlockAllAppStart
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService BlockAllAppStart
 */
HWTEST_F(AbilityManagerServiceSeventhTest, BlockAllAppStart_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSeventhTest BlockAllAppStart_003 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    AppUtils::isSupportBlockAllAppStart_ = true;
    EXPECT_EQ(AppUtils::GetInstance().IsSupportBlockAllAppStart(), true);
    PermissionVerification::flag = PermissionVerification::FLAG::IS_SA_CALL;
    PermissionVerification::hasBlockAllAppStartPermission = false;
    EXPECT_EQ(PermissionVerification::GetInstance()->VerifyBlockAllAppStartPermission(), false);
    auto retCode = abilityMs_->BlockAllAppStart(true);
    EXPECT_EQ(retCode, ERR_PERMISSION_DENIED);
    EXPECT_EQ(abilityMs_->shouldBlockAllAppStart_, false);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSeventhTest BlockAllAppStart_003 end");
}

/*
 * Feature: AbilityManagerService
 * Name: BlockAllAppStart_004
 * Function: BlockAllAppStart
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService BlockAllAppStart
 */
HWTEST_F(AbilityManagerServiceSeventhTest, BlockAllAppStart_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSeventhTest BlockAllAppStart_004 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    PermissionVerification::flag = PermissionVerification::FLAG::IS_SA_CALL;
    PermissionVerification::hasBlockAllAppStartPermission = true;
    AppUtils::isSupportBlockAllAppStart_ = true;
    EXPECT_EQ(AppUtils::GetInstance().IsSupportBlockAllAppStart(), true);
    EXPECT_EQ(PermissionVerification::GetInstance()->VerifyBlockAllAppStartPermission(), true);
    auto retCode = abilityMs_->BlockAllAppStart(false);
    EXPECT_EQ(retCode, ERR_OK);
    EXPECT_EQ(abilityMs_->shouldBlockAllAppStart_, false);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSeventhTest BlockAllAppStart_004 end");
}

/*
 * Feature: AbilityManagerService
 * Name: ShouldBlockAllAppStart_001
 * Function: ShouldBlockAllAppStart
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ShouldBlockAllAppStart
 */
HWTEST_F(AbilityManagerServiceSeventhTest, ShouldBlockAllAppStart_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSeventhTest ShouldBlockAllAppStart_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    AppUtils::isSupportBlockAllAppStart_ = true;
    EXPECT_EQ(AppUtils::GetInstance().IsSupportBlockAllAppStart(), true);
    abilityMs_->shouldBlockAllAppStart_ = true;
    EXPECT_EQ(abilityMs_->ShouldBlockAllAppStart(), true);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSeventhTest ShouldBlockAllAppStart_001 end");
}

/*
 * Feature: AbilityManagerService
 * Name: ShouldBlockAllAppStart_002
 * Function: ShouldBlockAllAppStart
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ShouldBlockAllAppStart
 */
HWTEST_F(AbilityManagerServiceSeventhTest, ShouldBlockAllAppStart_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSeventhTest ShouldBlockAllAppStart_002 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    AppUtils::isSupportBlockAllAppStart_ = false;
    EXPECT_EQ(AppUtils::GetInstance().IsSupportBlockAllAppStart(), false);
    EXPECT_EQ(abilityMs_->ShouldBlockAllAppStart(), false);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSeventhTest ShouldBlockAllAppStart_002 end");
}

/*
 * Feature: AbilityManagerService
 * Name: ShouldBlockAllAppStart_003
 * Function: ShouldBlockAllAppStart
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ShouldBlockAllAppStart
 */
HWTEST_F(AbilityManagerServiceSeventhTest, ShouldBlockAllAppStart_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSeventhTest ShouldBlockAllAppStart_003 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    AppUtils::isSupportBlockAllAppStart_ = true;
    EXPECT_EQ(AppUtils::GetInstance().IsSupportBlockAllAppStart(), true);
    abilityMs_->shouldBlockAllAppStart_ = false;
    EXPECT_EQ(abilityMs_->ShouldBlockAllAppStart(), false);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSeventhTest ShouldBlockAllAppStart_003 end");
}
}  // namespace AAFwk
}  // namespace OHOS
