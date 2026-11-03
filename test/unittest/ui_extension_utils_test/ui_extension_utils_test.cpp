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

#include "ui_extension_utils.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AAFwk {
class UIExtensionUtilsTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void UIExtensionUtilsTest::SetUpTestCase()
{}

void UIExtensionUtilsTest::TearDownTestCase()
{}

void UIExtensionUtilsTest::SetUp()
{}

void UIExtensionUtilsTest::TearDown()
{}

/**
 * @tc.name: IsUIExtension_0100
 * @tc.desc: IsUIExtension Test
 * @tc.type: FUNC
 * @tc.require: issueI7HOM3
 */
HWTEST_F(UIExtensionUtilsTest, IsUIExtension_0100, TestSize.Level0)
{
    AppExecFwk::ExtensionAbilityType extensionAbilityType = AppExecFwk::ExtensionAbilityType::UI;
    bool result = UIExtensionUtils::IsUIExtension(extensionAbilityType);
    EXPECT_TRUE(result);

    extensionAbilityType = AppExecFwk::ExtensionAbilityType::SYSPICKER_MEDIACONTROL;
    result = UIExtensionUtils::IsUIExtension(extensionAbilityType);
    EXPECT_TRUE(result);

    extensionAbilityType = AppExecFwk::ExtensionAbilityType::SYSDIALOG_USERAUTH;
    result = UIExtensionUtils::IsUIExtension(extensionAbilityType);
    EXPECT_TRUE(result);

    extensionAbilityType = AppExecFwk::ExtensionAbilityType::UKEY_AUTH;
    result = UIExtensionUtils::IsUIExtension(extensionAbilityType);
    EXPECT_TRUE(result);

    extensionAbilityType = AppExecFwk::ExtensionAbilityType::WINDOW;
    result = UIExtensionUtils::IsUIExtension(extensionAbilityType);
    EXPECT_FALSE(result);

    extensionAbilityType = AppExecFwk::ExtensionAbilityType::ACTION;
    result = UIExtensionUtils::IsUIExtension(extensionAbilityType);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: IsWindowExtension_0100
 * @tc.desc: IsWindowExtension Test
 * @tc.type: FUNC
 * @tc.require: issueI7HOM3
 */
HWTEST_F(UIExtensionUtilsTest, IsWindowExtension_0100, TestSize.Level0)
{
    AppExecFwk::ExtensionAbilityType extensionAbilityType = AppExecFwk::ExtensionAbilityType::UI;
    bool result = UIExtensionUtils::IsWindowExtension(extensionAbilityType);
    EXPECT_FALSE(result);

    extensionAbilityType = AppExecFwk::ExtensionAbilityType::WINDOW;
    result = UIExtensionUtils::IsWindowExtension(extensionAbilityType);
    EXPECT_TRUE(result);

    extensionAbilityType = AppExecFwk::ExtensionAbilityType::SHARE;
    result = UIExtensionUtils::IsUIExtension(extensionAbilityType);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: IsSystemUIExtension_0100
 * @tc.desc: IsWindowExtension Test
 * @tc.type: FUNC
 */
HWTEST_F(UIExtensionUtilsTest, IsSystemUIExtension_0100, TestSize.Level0)
{
    AppExecFwk::ExtensionAbilityType extensionAbilityType = AppExecFwk::ExtensionAbilityType::SYSDIALOG_COMMON;
    bool result = UIExtensionUtils::IsUIExtension(extensionAbilityType);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: IsProcessUdkeyExtension_0100
 * @tc.desc: IsProcessUdkeyExtension Test
 * @tc.type: FUNC
 */
HWTEST_F(UIExtensionUtilsTest, IsProcessUdkeyExtension_0100, TestSize.Level0)
{
    AppExecFwk::ExtensionAbilityType extensionAbilityType = AppExecFwk::ExtensionAbilityType::SYSDIALOG_COMMON;
    bool result = UIExtensionUtils::IsProcessUdkeyExtension(extensionAbilityType);
    EXPECT_FALSE(result);

    extensionAbilityType = AppExecFwk::ExtensionAbilityType::SYSPICKER_FILEPICKER;
    result = UIExtensionUtils::IsProcessUdkeyExtension(extensionAbilityType);
    EXPECT_TRUE(result);
}
}  // namespace AAFwk
}  // namespace OHOS
