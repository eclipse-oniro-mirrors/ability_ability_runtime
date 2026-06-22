/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "extension_permissions_util.h"
#include "hilog_tag_wrapper.h"
#include "permission_verification.h"

using namespace testing;
using namespace testing::ext;
using OHOS::AppExecFwk::ExtensionAbilityType;

namespace OHOS {
namespace AAFwk {
class ExtensionPermissionsUtilTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void ExtensionPermissionsUtilTest::SetUpTestCase() {}

void ExtensionPermissionsUtilTest::TearDownTestCase() {}

void ExtensionPermissionsUtilTest::SetUp() {}

void ExtensionPermissionsUtilTest::TearDown() {}

/*
 * Feature: ExtensionPermissionsUtil
 * Function: CheckSAPermission
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckSAPermission
 */
HWTEST_F(ExtensionPermissionsUtilTest, CheckSAPermission_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ExtensionPermissionsUtilTest CheckSAPermission_001 start");
    MyFlag::flag_ = 0;
    EXPECT_FALSE(ExtensionPermissionsUtil::CheckSAPermission(ExtensionAbilityType::FORM));
    TAG_LOGI(AAFwkTag::TEST, "ExtensionPermissionsUtilTest CheckSAPermission_001 end");
}

/*
 * Feature: ExtensionPermissionsUtil
 * Function: CheckSAPermission
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckSAPermission
 */
HWTEST_F(ExtensionPermissionsUtilTest, CheckSAPermission_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ExtensionPermissionsUtilTest CheckSAPermission_002 start");

    MyFlag::flag_ = 1;
    MyFlag::hasPerm_ = true;
    EXPECT_TRUE(ExtensionPermissionsUtil::CheckSAPermission(ExtensionAbilityType::FORM));
    MyFlag::flag_ = 0;
    TAG_LOGI(AAFwkTag::TEST, "ExtensionPermissionsUtilTest CheckSAPermission_002 end");
}

/*
 * Feature: ExtensionPermissionsUtil
 * Function: CheckSAPermission
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckSAPermission
 */
HWTEST_F(ExtensionPermissionsUtilTest, CheckSAPermission_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ExtensionPermissionsUtilTest CheckSAPermission_002 start");

    MyFlag::flag_ = 1;
    MyFlag::hasPerm_ = false;
    EXPECT_FALSE(ExtensionPermissionsUtil::CheckSAPermission(ExtensionAbilityType::FORM));
    MyFlag::flag_ = 0;
    TAG_LOGI(AAFwkTag::TEST, "ExtensionPermissionsUtilTest CheckSAPermission_002 end");
}

/*
 * Feature: ExtensionPermissionsUtil
 * Function: CheckSAPermission
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckSAPermission
 */
HWTEST_F(ExtensionPermissionsUtilTest, CheckSAPermission_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ExtensionPermissionsUtilTest CheckSAPermission_004 start");

    MyFlag::flag_ = 2;
    MyFlag::hasPerm_ = false;
    EXPECT_TRUE(ExtensionPermissionsUtil::CheckSAPermission(ExtensionAbilityType::INPUTMETHOD));
    MyFlag::flag_ = 0;
    TAG_LOGI(AAFwkTag::TEST, "ExtensionPermissionsUtilTest CheckSAPermission_004 end");
}

/*
 * Feature: ExtensionPermissionsUtil
 * Function: CheckSAPermission
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckSAPermission
 */
HWTEST_F(ExtensionPermissionsUtilTest, CheckSAPermission_005, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ExtensionPermissionsUtilTest CheckSAPermission_005 start");

    MyFlag::flag_ = 1;
    MyFlag::hasPerm_ = true;
    EXPECT_TRUE(ExtensionPermissionsUtil::CheckSAPermission(ExtensionAbilityType::ASSET_ACCELERATION));
    MyFlag::flag_ = 0;
    TAG_LOGI(AAFwkTag::TEST, "ExtensionPermissionsUtilTest CheckSAPermission_005 end");
}

/*
 * Feature: ExtensionPermissionsUtil
 * Function: CheckSAPermission
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckSAPermission
 */
HWTEST_F(ExtensionPermissionsUtilTest, CheckSAPermission_006, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ExtensionPermissionsUtilTest CheckSAPermission_006 start");

    MyFlag::flag_ = 1;
    MyFlag::hasPerm_ = true;
    EXPECT_TRUE(ExtensionPermissionsUtil::CheckSAPermission(ExtensionAbilityType::DISTRIBUTED));
    MyFlag::flag_ = 0;
    TAG_LOGI(AAFwkTag::TEST, "ExtensionPermissionsUtilTest CheckSAPermission_006 end");
}

/*
 * Feature: ExtensionPermissionsUtil
 * Function: CheckSAPermission
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckSAPermission
 */
HWTEST_F(ExtensionPermissionsUtilTest, CheckSAPermission_007, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ExtensionPermissionsUtilTest CheckSAPermission_007 start");

    MyFlag::flag_ = 1;
    MyFlag::hasPerm_ = true;
    EXPECT_TRUE(ExtensionPermissionsUtil::CheckSAPermission(ExtensionAbilityType::WORK_SCHEDULER));
    MyFlag::flag_ = 0;
    TAG_LOGI(AAFwkTag::TEST, "ExtensionPermissionsUtilTest CheckSAPermission_007 end");
}

/*
 * Feature: ExtensionPermissionsUtil
 * Function: CheckSAPermission
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckSAPermission
 */
HWTEST_F(ExtensionPermissionsUtilTest, CheckSAPermission_008, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ExtensionPermissionsUtilTest CheckSAPermission_008 start");

    MyFlag::flag_ = 1;
    MyFlag::hasPerm_ = true;
    EXPECT_TRUE(ExtensionPermissionsUtil::CheckSAPermission(ExtensionAbilityType::INPUTMETHOD));
    MyFlag::flag_ = 0;
    TAG_LOGI(AAFwkTag::TEST, "ExtensionPermissionsUtilTest CheckSAPermission_008 end");
}

/*
 * Feature: ExtensionPermissionsUtil
 * Function: CheckSAPermission
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckSAPermission
 */
HWTEST_F(ExtensionPermissionsUtilTest, CheckSAPermission_009, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ExtensionPermissionsUtilTest CheckSAPermission_009 start");

    MyFlag::flag_ = 1;
    MyFlag::hasPerm_ = true;
    EXPECT_TRUE(ExtensionPermissionsUtil::CheckSAPermission(ExtensionAbilityType::ACCESSIBILITY));
    MyFlag::flag_ = 0;
    TAG_LOGI(AAFwkTag::TEST, "ExtensionPermissionsUtilTest CheckSAPermission_009 end");
}

/*
 * Feature: ExtensionPermissionsUtil
 * Function: CheckSAPermission
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckSAPermission
 */
HWTEST_F(ExtensionPermissionsUtilTest, CheckSAPermission_010, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ExtensionPermissionsUtilTest CheckSAPermission_010 start");

    MyFlag::flag_ = 1;
    MyFlag::hasPerm_ = true;
    EXPECT_TRUE(ExtensionPermissionsUtil::CheckSAPermission(ExtensionAbilityType::STATICSUBSCRIBER));
    MyFlag::flag_ = 0;
    TAG_LOGI(AAFwkTag::TEST, "ExtensionPermissionsUtilTest CheckSAPermission_010 end");
}

/*
 * Feature: ExtensionPermissionsUtil
 * Function: CheckSAPermission
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckSAPermission
 */
HWTEST_F(ExtensionPermissionsUtilTest, CheckSAPermission_011, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ExtensionPermissionsUtilTest CheckSAPermission_011 start");

    MyFlag::flag_ = 1;
    MyFlag::hasPerm_ = true;
    EXPECT_TRUE(ExtensionPermissionsUtil::CheckSAPermission(ExtensionAbilityType::WALLPAPER));
    MyFlag::flag_ = 0;
    TAG_LOGI(AAFwkTag::TEST, "ExtensionPermissionsUtilTest CheckSAPermission_011 end");
}

/*
 * Feature: ExtensionPermissionsUtil
 * Function: CheckSAPermission
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckSAPermission
 */
HWTEST_F(ExtensionPermissionsUtilTest, CheckSAPermission_012, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ExtensionPermissionsUtilTest CheckSAPermission_012 start");

    MyFlag::flag_ = 1;
    MyFlag::hasPerm_ = true;
    EXPECT_TRUE(ExtensionPermissionsUtil::CheckSAPermission(ExtensionAbilityType::BACKUP));
    MyFlag::flag_ = 0;
    TAG_LOGI(AAFwkTag::TEST, "ExtensionPermissionsUtilTest CheckSAPermission_012 end");
}

/*
 * Feature: ExtensionPermissionsUtil
 * Function: CheckSAPermission
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckSAPermission
 */
HWTEST_F(ExtensionPermissionsUtilTest, CheckSAPermission_013, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ExtensionPermissionsUtilTest CheckSAPermission_013 start");

    MyFlag::flag_ = 1;
    MyFlag::hasPerm_ = true;
    EXPECT_TRUE(ExtensionPermissionsUtil::CheckSAPermission(ExtensionAbilityType::ENTERPRISE_ADMIN));
    MyFlag::flag_ = 0;
    TAG_LOGI(AAFwkTag::TEST, "ExtensionPermissionsUtilTest CheckSAPermission_013 end");
}

/*
 * Feature: ExtensionPermissionsUtil
 * Function: CheckSAPermission
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckSAPermission
 */
HWTEST_F(ExtensionPermissionsUtilTest, CheckSAPermission_014, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ExtensionPermissionsUtilTest CheckSAPermission_014 start");

    MyFlag::flag_ = 1;
    MyFlag::hasPerm_ = true;
    EXPECT_TRUE(ExtensionPermissionsUtil::CheckSAPermission(ExtensionAbilityType::PRINT));
    MyFlag::flag_ = 0;
    TAG_LOGI(AAFwkTag::TEST, "ExtensionPermissionsUtilTest CheckSAPermission_014 end");
}

/*
 * Feature: ExtensionPermissionsUtil
 * Function: CheckSAPermission
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckSAPermission
 */
HWTEST_F(ExtensionPermissionsUtilTest, CheckSAPermission_015, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ExtensionPermissionsUtilTest CheckSAPermission_015 start");

    MyFlag::flag_ = 1;
    MyFlag::hasPerm_ = false;
    EXPECT_TRUE(ExtensionPermissionsUtil::CheckSAPermission(ExtensionAbilityType::VPN));
    MyFlag::flag_ = 0;
    TAG_LOGI(AAFwkTag::TEST, "ExtensionPermissionsUtilTest CheckSAPermission_015 end");
}

/*
 * Feature: ExtensionPermissionsUtil
 * Function: CheckSAPermission
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckSAPermission
 */
HWTEST_F(ExtensionPermissionsUtilTest, CheckSAPermission_016, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ExtensionPermissionsUtilTest CheckSAPermission_016 start");

    MyFlag::flag_ = 1;
    MyFlag::hasPerm_ = true;
    EXPECT_TRUE(ExtensionPermissionsUtil::CheckSAPermission(ExtensionAbilityType::SELECTION));
    MyFlag::flag_ = 0;
    TAG_LOGI(AAFwkTag::TEST, "ExtensionPermissionsUtilTest CheckSAPermission_016 end");
}

/*
 * Feature: ExtensionPermissionsUtil
 * Function: CheckSAPermission
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckSAPermission
 */
HWTEST_F(ExtensionPermissionsUtilTest, CheckSAPermission_017, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ExtensionPermissionsUtilTest CheckSAPermission_017 start");

    MyFlag::flag_ = 1;
    MyFlag::hasPerm_ = true;
    EXPECT_TRUE(ExtensionPermissionsUtil::CheckSAPermission(ExtensionAbilityType::AGENT));
    MyFlag::flag_ = 0;
    TAG_LOGI(AAFwkTag::TEST, "ExtensionPermissionsUtilTest CheckSAPermission_017 end");
}

/*
 * Feature: ExtensionPermissionsUtil
 * Function: CheckSAPermission
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckSAPermission
 */
HWTEST_F(ExtensionPermissionsUtilTest, CheckSAPermission_018, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ExtensionPermissionsUtilTest CheckSAPermission_018 start");

    MyFlag::flag_ = MyFlag::FLAG::IS_SHELL_CALL;
    MyFlag::hasPerm_ = false;
    EXPECT_FALSE(ExtensionPermissionsUtil::CheckSAPermission(ExtensionAbilityType::AGENT));
    MyFlag::flag_ = 0;
    TAG_LOGI(AAFwkTag::TEST, "ExtensionPermissionsUtilTest CheckSAPermission_018 end");
}

/*
 * Feature: ExtensionPermissionsUtil
 * Function: CheckSAPermission
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckSAPermissionMore
 */
HWTEST_F(ExtensionPermissionsUtilTest, CheckSAPermissionMore_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ExtensionPermissionsUtilTest CheckSAPermissionMore_001 start");

    MyFlag::hasPerm_ = true;
    EXPECT_TRUE(ExtensionPermissionsUtil::CheckSAPermissionMore(ExtensionAbilityType::FILEACCESS_EXTENSION));
    TAG_LOGI(AAFwkTag::TEST, "ExtensionPermissionsUtilTest CheckSAPermissionMore_001 end");
}

/*
 * Feature: ExtensionPermissionsUtil
 * Function: CheckSAPermission
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckSAPermissionMore
 */
HWTEST_F(ExtensionPermissionsUtilTest, CheckSAPermissionMore_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ExtensionPermissionsUtilTest CheckSAPermissionMore_001 start");

    MyFlag::hasPerm_ = false;
    EXPECT_FALSE(ExtensionPermissionsUtil::CheckSAPermissionMore(ExtensionAbilityType::FILEACCESS_EXTENSION));
    TAG_LOGI(AAFwkTag::TEST, "ExtensionPermissionsUtilTest CheckSAPermissionMore_001 end");
}

/*
 * Feature: ExtensionPermissionsUtil
 * Function: CheckSAPermission
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckSAPermissionMore
 */
HWTEST_F(ExtensionPermissionsUtilTest, CheckSAPermissionMore_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ExtensionPermissionsUtilTest CheckSAPermissionMore_003 start");

    MyFlag::hasPerm_ = false;
    EXPECT_FALSE(ExtensionPermissionsUtil::CheckSAPermissionMore(ExtensionAbilityType::REMOTE_NOTIFICATION));
    TAG_LOGI(AAFwkTag::TEST, "ExtensionPermissionsUtilTest CheckSAPermissionMore_003 end");
}

/*
 * Feature: ExtensionPermissionsUtil
 * Function: CheckSAPermission
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckSAPermissionMore
 */
HWTEST_F(ExtensionPermissionsUtilTest, CheckSAPermissionMore_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ExtensionPermissionsUtilTest CheckSAPermissionMore_004 start");

    MyFlag::hasPerm_ = false;
    EXPECT_FALSE(ExtensionPermissionsUtil::CheckSAPermissionMore(ExtensionAbilityType::REMOTE_LOCATION));
    TAG_LOGI(AAFwkTag::TEST, "ExtensionPermissionsUtilTest CheckSAPermissionMore_004 end");
}

/*
 * Feature: ExtensionPermissionsUtil
 * Function: CheckSAPermission
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckSAPermissionMore
 */
HWTEST_F(ExtensionPermissionsUtilTest, CheckSAPermissionMore_005, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ExtensionPermissionsUtilTest CheckSAPermissionMore_005 start");

    MyFlag::hasPerm_ = false;
    EXPECT_FALSE(ExtensionPermissionsUtil::CheckSAPermissionMore(ExtensionAbilityType::PUSH));
    TAG_LOGI(AAFwkTag::TEST, "ExtensionPermissionsUtilTest CheckSAPermissionMore_005 end");
}

/*
 * Feature: ExtensionPermissionsUtil
 * Function: CheckSAPermission
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckSAPermissionMore
 */
HWTEST_F(ExtensionPermissionsUtilTest, CheckSAPermissionMore_006, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ExtensionPermissionsUtilTest CheckSAPermissionMore_006 start");

    MyFlag::hasPerm_ = false;
    EXPECT_FALSE(ExtensionPermissionsUtil::CheckSAPermissionMore(ExtensionAbilityType::VOIP));
    TAG_LOGI(AAFwkTag::TEST, "ExtensionPermissionsUtilTest CheckSAPermissionMore_006 end");
}

/*
 * Feature: ExtensionPermissionsUtil
 * Function: CheckSAPermission
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckSAPermissionMore
 */
HWTEST_F(ExtensionPermissionsUtilTest, CheckSAPermissionMore_007, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ExtensionPermissionsUtilTest CheckSAPermissionMore_007 start");

    MyFlag::hasPerm_ = false;
    EXPECT_TRUE(ExtensionPermissionsUtil::CheckSAPermissionMore(ExtensionAbilityType::VPN));
    TAG_LOGI(AAFwkTag::TEST, "ExtensionPermissionsUtilTest CheckSAPermissionMore_007 end");
}
} // namespace AAFwk
} // namespace OHOS
