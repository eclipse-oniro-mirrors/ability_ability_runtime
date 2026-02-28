/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
#include "extension_config_mgr.h"
#include "app_module_checker.h"
#include "mock/mock_runtime.h"
#undef private
#undef protected

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AbilityRuntime {
namespace {
    constexpr int32_t EXTENSION_TYPE_FORM = 0;
    constexpr int32_t EXTENSION_TYPE_WORK_SCHEDULER = 1;
    constexpr int32_t EXTENSION_TYPE_INPUTMETHOD = 2;
    constexpr int32_t EXTENSION_TYPE_SERVICE = 3;
    constexpr int32_t EXTENSION_TYPE_ACCESSIBILITY = 4;
    constexpr int32_t EXTENSION_TYPE_DATASHARE = 5;
    constexpr int32_t EXTENSION_TYPE_STATICSUBSCRIBER = 7;
    constexpr int32_t EXTENSION_TYPE_WALLPAPER = 8;
    constexpr int32_t EXTENSION_TYPE_BACKUP = 9;
    constexpr int32_t EXTENSION_TYPE_WINDOW = 10;
    constexpr int32_t EXTENSION_TYPE_ENTERPRISE_ADMIN = 11;
    constexpr int32_t EXTENSION_TYPE_FILE_ACCESS = 12;
    constexpr int32_t EXTENSION_TYPE_DRIVER = 18;
    constexpr char BLOCK_LIST_ITEM_SERVICE_EXTENSION[] = "ServiceExtension";
    constexpr char BLOCK_LIST_ITEM_FORM_EXTENSION[] = "FormExtension";
    constexpr char BLOCK_LIST_ITEM_FILE_ACCESS_EXTENSION[] = "FileAccessExtension";
    constexpr char BLOCK_LIST_ITEM_BACKUP_EXTENSION[] = "BackupExtension";
    constexpr char BLOCK_LIST_ITEM_ENTERPRISE_ADMIN_EXTENSION[] = "EnterpriseAdminExtension";
    constexpr char BLOCK_LIST_ITEM_WINDOW_EXTENSION_EXTENSION[] = "WindowExtensionExtension";
    constexpr char BLOCK_LIST_ITEM_WALLPAPER_EXTENSION[] = "WallpaperExtension";
    constexpr char BLOCK_LIST_ITEM_STATIC_SUBSCRIBER_EXTENSION[] = "StaticSubscriberExtension";
    constexpr char BLOCK_LIST_ITEM_ACCESSIBILITY_EXTENSION[] = "AccessibilityExtension";
    constexpr char BLOCK_LIST_ITEM_INPUT_METHOD_EXTENSION_ABILITY[] = "InputMethodExtensionAbility";
    constexpr char BLOCK_LIST_ITEM_WORK_SCHEDULER_EXTENSION[] = "WorkSchedulerExtension";
    constexpr char BLOCK_LIST_ITEM_DATA_SHARE_EXTENSION[] = "DataShareExtension";
    constexpr char BLOCK_LIST_ITEM_DRIVER_EXTENSION[] = "DriverExtension";
    constexpr char INVAILD_BLOCK_LIST_ITEM[] = "InvaildExtension";
}

class ExtensionConfigMgrTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void ExtensionConfigMgrTest::SetUpTestCase()
{}

void ExtensionConfigMgrTest::TearDownTestCase()
{}

void ExtensionConfigMgrTest::SetUp()
{}

void ExtensionConfigMgrTest::TearDown()
{}

/**
 * @tc.name: LoadExtensionBlockList_0100
 * @tc.desc: Test LoadExtensionBlockList with multiple extensions
 * @tc.type: FUNC
 * @tc.require: issueI581T3
 */
HWTEST_F(ExtensionConfigMgrTest, LoadExtensionBlockList_0100, TestSize.Level0)
{
    ExtensionConfigMgr mgr;
    // Load multiple extension blocklists
    mgr.LoadExtensionBlockList(BLOCK_LIST_ITEM_SERVICE_EXTENSION, EXTENSION_TYPE_SERVICE);
    bool result = (mgr.extensionBlocklist_.find(EXTENSION_TYPE_SERVICE) != mgr.extensionBlocklist_.end());
    EXPECT_TRUE(result);

    mgr.LoadExtensionBlockList(BLOCK_LIST_ITEM_FORM_EXTENSION, EXTENSION_TYPE_FORM);
    result = (mgr.extensionBlocklist_.find(EXTENSION_TYPE_FORM) != mgr.extensionBlocklist_.end());
    EXPECT_TRUE(result);

    mgr.LoadExtensionBlockList(BLOCK_LIST_ITEM_FILE_ACCESS_EXTENSION, EXTENSION_TYPE_FILE_ACCESS);
    result = (mgr.extensionBlocklist_.find(EXTENSION_TYPE_FILE_ACCESS) != mgr.extensionBlocklist_.end());
    EXPECT_TRUE(result);

    mgr.LoadExtensionBlockList(BLOCK_LIST_ITEM_BACKUP_EXTENSION, EXTENSION_TYPE_BACKUP);
    result = (mgr.extensionBlocklist_.find(EXTENSION_TYPE_BACKUP) != mgr.extensionBlocklist_.end());
    EXPECT_TRUE(result);

    mgr.LoadExtensionBlockList(BLOCK_LIST_ITEM_ENTERPRISE_ADMIN_EXTENSION, EXTENSION_TYPE_ENTERPRISE_ADMIN);
    result = (mgr.extensionBlocklist_.find(EXTENSION_TYPE_ENTERPRISE_ADMIN) != mgr.extensionBlocklist_.end());
    EXPECT_TRUE(result);

    mgr.LoadExtensionBlockList(BLOCK_LIST_ITEM_WINDOW_EXTENSION_EXTENSION, EXTENSION_TYPE_WINDOW);
    result = (mgr.extensionBlocklist_.find(EXTENSION_TYPE_WINDOW) != mgr.extensionBlocklist_.end());
    EXPECT_TRUE(result);

    mgr.LoadExtensionBlockList(BLOCK_LIST_ITEM_WALLPAPER_EXTENSION, EXTENSION_TYPE_WALLPAPER);
    result = (mgr.extensionBlocklist_.find(EXTENSION_TYPE_WALLPAPER) != mgr.extensionBlocklist_.end());
    EXPECT_TRUE(result);

    mgr.LoadExtensionBlockList(BLOCK_LIST_ITEM_STATIC_SUBSCRIBER_EXTENSION, EXTENSION_TYPE_STATICSUBSCRIBER);
    result = (mgr.extensionBlocklist_.find(EXTENSION_TYPE_STATICSUBSCRIBER) != mgr.extensionBlocklist_.end());
    EXPECT_TRUE(result);

    mgr.LoadExtensionBlockList(BLOCK_LIST_ITEM_ACCESSIBILITY_EXTENSION, EXTENSION_TYPE_ACCESSIBILITY);
    result = (mgr.extensionBlocklist_.find(EXTENSION_TYPE_ACCESSIBILITY) != mgr.extensionBlocklist_.end());
    EXPECT_TRUE(result);

    mgr.LoadExtensionBlockList(BLOCK_LIST_ITEM_INPUT_METHOD_EXTENSION_ABILITY, EXTENSION_TYPE_INPUTMETHOD);
    result = (mgr.extensionBlocklist_.find(EXTENSION_TYPE_INPUTMETHOD) != mgr.extensionBlocklist_.end());
    EXPECT_TRUE(result);

    mgr.LoadExtensionBlockList(BLOCK_LIST_ITEM_WORK_SCHEDULER_EXTENSION, EXTENSION_TYPE_WORK_SCHEDULER);
    result = (mgr.extensionBlocklist_.find(EXTENSION_TYPE_WORK_SCHEDULER) != mgr.extensionBlocklist_.end());
    EXPECT_TRUE(result);

    mgr.LoadExtensionBlockList(BLOCK_LIST_ITEM_DATA_SHARE_EXTENSION, EXTENSION_TYPE_DATASHARE);
    result = (mgr.extensionBlocklist_.find(EXTENSION_TYPE_DATASHARE) != mgr.extensionBlocklist_.end());
    EXPECT_TRUE(result);
}

/**
 * @tc.name: LoadExtensionBlockList_0200
 * @tc.desc: Test LoadExtensionBlockList and verify blocklist content
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionConfigMgrTest, LoadExtensionBlockList_0200, TestSize.Level0)
{
    ExtensionConfigMgr mgr;
    // Load extension blocklist
    mgr.LoadExtensionBlockList(BLOCK_LIST_ITEM_SERVICE_EXTENSION, EXTENSION_TYPE_SERVICE);
    bool result = (mgr.extensionBlocklist_.find(EXTENSION_TYPE_SERVICE) != mgr.extensionBlocklist_.end());
    EXPECT_TRUE(result);

    mgr.LoadExtensionBlockList(BLOCK_LIST_ITEM_FORM_EXTENSION, EXTENSION_TYPE_FORM);
    result = (mgr.extensionBlocklist_.find(EXTENSION_TYPE_FORM) != mgr.extensionBlocklist_.end());
    EXPECT_TRUE(result);

    // Verify blocklist content is not empty
    auto iter = mgr.extensionBlocklist_.find(EXTENSION_TYPE_FORM);
    EXPECT_TRUE(iter != mgr.extensionBlocklist_.end());
    if (iter != mgr.extensionBlocklist_.end()) {
        EXPECT_FALSE(iter->second.empty());
    }
}

/**
 * @tc.name: AddBlockListItem_0100
 * @tc.desc: Test LoadExtensionBlockList with multiple extensions
 * @tc.type: FUNC
 * @tc.require: issueI581T3
 */
HWTEST_F(ExtensionConfigMgrTest, AddBlockListItem_0100, TestSize.Level1)
{
    ExtensionConfigMgr mgr;
    bool result = false;

    // Load extension blocklists using new API
    mgr.LoadExtensionBlockList(BLOCK_LIST_ITEM_FORM_EXTENSION, EXTENSION_TYPE_FORM);
    result = (mgr.extensionBlocklist_.find(EXTENSION_TYPE_FORM) != mgr.extensionBlocklist_.end());
    EXPECT_TRUE(result);

    mgr.LoadExtensionBlockList(BLOCK_LIST_ITEM_WORK_SCHEDULER_EXTENSION, EXTENSION_TYPE_WORK_SCHEDULER);
    result = (mgr.extensionBlocklist_.find(EXTENSION_TYPE_WORK_SCHEDULER) != mgr.extensionBlocklist_.end());
    EXPECT_TRUE(result);

    mgr.LoadExtensionBlockList(BLOCK_LIST_ITEM_INPUT_METHOD_EXTENSION_ABILITY, EXTENSION_TYPE_INPUTMETHOD);
    result = (mgr.extensionBlocklist_.find(EXTENSION_TYPE_INPUTMETHOD) != mgr.extensionBlocklist_.end());
    EXPECT_TRUE(result);

    mgr.LoadExtensionBlockList(BLOCK_LIST_ITEM_SERVICE_EXTENSION, EXTENSION_TYPE_SERVICE);
    result = (mgr.extensionBlocklist_.find(EXTENSION_TYPE_SERVICE) != mgr.extensionBlocklist_.end());
    EXPECT_TRUE(result);

    mgr.LoadExtensionBlockList(BLOCK_LIST_ITEM_ACCESSIBILITY_EXTENSION, EXTENSION_TYPE_ACCESSIBILITY);
    result = (mgr.extensionBlocklist_.find(EXTENSION_TYPE_ACCESSIBILITY) != mgr.extensionBlocklist_.end());
    EXPECT_TRUE(result);

    mgr.LoadExtensionBlockList(BLOCK_LIST_ITEM_DATA_SHARE_EXTENSION, EXTENSION_TYPE_DATASHARE);
    result = (mgr.extensionBlocklist_.find(EXTENSION_TYPE_DATASHARE) != mgr.extensionBlocklist_.end());
    EXPECT_TRUE(result);

    mgr.LoadExtensionBlockList(BLOCK_LIST_ITEM_STATIC_SUBSCRIBER_EXTENSION, EXTENSION_TYPE_STATICSUBSCRIBER);
    result = (mgr.extensionBlocklist_.find(EXTENSION_TYPE_STATICSUBSCRIBER) != mgr.extensionBlocklist_.end());
    EXPECT_TRUE(result);

    mgr.LoadExtensionBlockList(BLOCK_LIST_ITEM_WALLPAPER_EXTENSION, EXTENSION_TYPE_WALLPAPER);
    result = (mgr.extensionBlocklist_.find(EXTENSION_TYPE_WALLPAPER) != mgr.extensionBlocklist_.end());
    EXPECT_TRUE(result);

    mgr.LoadExtensionBlockList(BLOCK_LIST_ITEM_BACKUP_EXTENSION, EXTENSION_TYPE_BACKUP);
    result = (mgr.extensionBlocklist_.find(EXTENSION_TYPE_BACKUP) != mgr.extensionBlocklist_.end());
    EXPECT_TRUE(result);

    mgr.LoadExtensionBlockList(BLOCK_LIST_ITEM_WINDOW_EXTENSION_EXTENSION, EXTENSION_TYPE_WINDOW);
    result = (mgr.extensionBlocklist_.find(EXTENSION_TYPE_WINDOW) != mgr.extensionBlocklist_.end());
    EXPECT_TRUE(result);

    mgr.LoadExtensionBlockList(BLOCK_LIST_ITEM_ENTERPRISE_ADMIN_EXTENSION, EXTENSION_TYPE_ENTERPRISE_ADMIN);
    result = (mgr.extensionBlocklist_.find(EXTENSION_TYPE_ENTERPRISE_ADMIN) != mgr.extensionBlocklist_.end());
    EXPECT_TRUE(result);

    mgr.LoadExtensionBlockList(BLOCK_LIST_ITEM_FILE_ACCESS_EXTENSION, EXTENSION_TYPE_FILE_ACCESS);
    result = (mgr.extensionBlocklist_.find(EXTENSION_TYPE_FILE_ACCESS) != mgr.extensionBlocklist_.end());
    EXPECT_TRUE(result);
}

/**
 * @tc.name: AddBlockListItem_0200
 * @tc.desc: Test LoadExtensionBlockList with invalid extension name
 * @tc.type: FUNC
 * @tc.require: issueI5825N
 */
HWTEST_F(ExtensionConfigMgrTest, AddBlockListItem_0200, TestSize.Level1)
{
    ExtensionConfigMgr mgr;
    // Try to load blocklist with invalid extension name
    mgr.LoadExtensionBlockList(INVAILD_BLOCK_LIST_ITEM, EXTENSION_TYPE_FORM);
    bool result = (mgr.extensionBlocklist_.find(EXTENSION_TYPE_FORM) != mgr.extensionBlocklist_.end());
    EXPECT_FALSE(result);
}

/**
 * @tc.name: GenerateExtensionEtsBlocklists_0100
 * @tc.desc: Generate Extension ets block list Test
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionConfigMgrTest, GenerateExtensionEtsBlocklists_0100, TestSize.Level1)
{
    ExtensionConfigMgr mgr;
    std::unordered_set<std::string> set1 = { "111", "222", "333" };
    mgr.extensionBlocklist_.clear();
    mgr.extensionEtsBlocklist_.clear();
    mgr.extensionBlocklist_.emplace(EXTENSION_TYPE_FORM, set1);
    mgr.extensionType_ = EXTENSION_TYPE_WORK_SCHEDULER;
    mgr.GenerateExtensionEtsBlocklists();
    EXPECT_TRUE(mgr.extensionEtsBlocklist_.empty());

    std::unordered_set<std::string> set2;
    mgr.extensionBlocklist_.clear();
    mgr.extensionEtsBlocklist_.clear();
    mgr.extensionBlocklist_.emplace(EXTENSION_TYPE_FORM, set2);
    mgr.extensionType_ = EXTENSION_TYPE_FORM;
    mgr.GenerateExtensionEtsBlocklists();
    EXPECT_TRUE(mgr.extensionEtsBlocklist_.empty());

    mgr.extensionBlocklist_.clear();
    mgr.extensionEtsBlocklist_.clear();
    mgr.extensionBlocklist_.emplace(EXTENSION_TYPE_FORM, set1);
    mgr.extensionType_ = EXTENSION_TYPE_FORM;
    mgr.GenerateExtensionEtsBlocklists();
    EXPECT_FALSE(mgr.extensionEtsBlocklist_.empty());
    for (const auto& ele: set1) {
        EXPECT_TRUE(mgr.extensionEtsBlocklist_.find(ele) != mgr.extensionEtsBlocklist_.end());
    }
}

/**
 * @tc.name: GenerateExtensionEtsBlocklists_ShouldDoNothing_WhenExtensionEtsBlocklistIsNotEmpty
 * @tc.desc: func should do nothing when extensionEtsBlocklist is not empty.
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionConfigMgrTest, GenerateExtensionEtsBlocklists_ShouldDoNothing_WhenExtensionEtsBlocklistIsNotEmpty,
    TestSize.Level1)
{
    ExtensionConfigMgr mgr;
    mgr.extensionEtsBlocklist_ = { "module1", "module2" };
    mgr.GenerateExtensionEtsBlocklists();

    EXPECT_EQ(mgr.extensionEtsBlocklist_.size(), 2);
    EXPECT_NE(mgr.extensionEtsBlocklist_.find("module1"), mgr.extensionEtsBlocklist_.end());
    EXPECT_NE(mgr.extensionEtsBlocklist_.find("module2"), mgr.extensionEtsBlocklist_.end());
}

/**
 * @tc.name: GetStringAfterRemovePreFix_ShouldRemoveOHOSPrefix_WhenNameStartsWithOHOS
 * @tc.desc: func should remove prefix when input name startswith @ohos.
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionConfigMgrTest, GetStringAfterRemovePreFix_ShouldRemoveOHOSPrefix_WhenNameStartsWithOHOS,
    TestSize.Level1)
{
    ExtensionConfigMgr mgr;
    std::string input = "@ohos.example";
    std::string expectOutput = "example";
    EXPECT_EQ(mgr.GetStringAfterRemovePreFix(input), expectOutput);
}

/**
 * @tc.name: GetStringAfterRemovePreFix_ShouldRemoveOtherPrefix_WhenNameStartsWithOther
 * @tc.desc: func should remove prefix when input name startswith @xxx.
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionConfigMgrTest, GetStringAfterRemovePreFix_ShouldRemoveOtherPrefix_WhenNameStartsWithOther,
    TestSize.Level1)
{
    ExtensionConfigMgr mgr;
    std::string input = "@xxx.example";
    std::string expectOutput = "example";
    EXPECT_EQ(mgr.GetStringAfterRemovePreFix(input), expectOutput);

    input = "@xx.example";
    expectOutput = "example";
    EXPECT_EQ(mgr.GetStringAfterRemovePreFix(input), expectOutput);
}

/**
 * @tc.name: GetStringAfterRemovePreFix_ShouldNotRemovePrefix_WhenNameDoesNotStartsWithPrefix
 * @tc.desc: func should not remove prefix when input name does not startswith any prefix.
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionConfigMgrTest, GetStringAfterRemovePreFix_ShouldNotRemovePrefix_WhenNameDoesNotStartsWithPrefix,
    TestSize.Level1)
{
    ExtensionConfigMgr mgr;
    std::string input = "example";
    std::string expectOutput = "example";
    EXPECT_EQ(mgr.GetStringAfterRemovePreFix(input), expectOutput);
}

/**
 * @tc.name: GetStringAfterRemovePreFix_ShouldHandleEmptyInput_WhenNameIsEmpty
 * @tc.desc: func should handle empty input when input name is empty.
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionConfigMgrTest, GetStringAfterRemovePreFix_ShouldHandleEmptyInput_WhenNameIsEmpty, TestSize.Level1)
{
    ExtensionConfigMgr mgr;
    std::string input = "";
    std::string expectOutput = "";
    EXPECT_EQ(mgr.GetStringAfterRemovePreFix(input), expectOutput);
}

/**
 * @tc.name: GetStringAfterRemovePreFix_ShouldHandleShortInput_WhenNameIsShorterThanPrefix
 * @tc.desc: func should handle short input when input name is shorter than prefix.
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionConfigMgrTest, GetStringAfterRemovePreFix_ShouldHandleShortInput_WhenNameIsShorterThanPrefix,
    TestSize.Level1)
{
    ExtensionConfigMgr mgr;
    std::string input = "11";
    std::string expectOutput = "11";
    EXPECT_EQ(mgr.GetStringAfterRemovePreFix(input), expectOutput);
}

/**
 * @tc.name: GetStringAfterRemovePreFix_0100
 * @tc.desc: GetStringAfterRemovePreFix Test
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionConfigMgrTest, GetStringAfterRemovePreFix_0100, TestSize.Level1)
{
    ExtensionConfigMgr mgr;
    EXPECT_EQ(mgr.GetStringAfterRemovePreFix("@ohos.aaa.bbb"), "aaa.bbb");
    EXPECT_EQ(mgr.GetStringAfterRemovePreFix("aaaaaa"), "aaaaaa");
    EXPECT_EQ(mgr.GetStringAfterRemovePreFix("aaa"), "aaa");
    EXPECT_EQ(mgr.GetStringAfterRemovePreFix("@ohos."), "");
    EXPECT_EQ(mgr.GetStringAfterRemovePreFix("@xxx."), "");
    EXPECT_EQ(mgr.GetStringAfterRemovePreFix("@xxx.aaa"), "aaa");
    EXPECT_EQ(mgr.GetStringAfterRemovePreFix("@xxx"), "@xxx");
}

/**
 * @tc.name: CheckEtsModuleLoadable_ShouldReturnTrue_WhenFileNameIsPrefixAndNotInBlocklist
 * @tc.desc: func should return true when file is prefix and not in blocklist.
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionConfigMgrTest, CheckEtsModuleLoadable_ShouldReturnTrue_WhenFileNameIsPrefixAndNotInBlocklist,
    TestSize.Level1)
{
    ExtensionConfigMgr mgr;
    std::string className = "file.class";
    std::string fileName = "file";
    EXPECT_TRUE(mgr.CheckEtsModuleLoadable(className, fileName));
}

/**
 * @tc.name: CheckEtsModuleLoadable_ShouldReturnFalse_WhenFileNameIsNotPrefix
 * @tc.desc: func should return false when file is not prefix.(note: class name should be "std::string(filename) + '.'")
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionConfigMgrTest, CheckEtsModuleLoadable_ShouldReturnFalse_WhenFileNameIsNotPrefix, TestSize.Level1)
{
    ExtensionConfigMgr mgr;
    std::string className = "file.class";
    std::string fileName = "fileName";
    EXPECT_FALSE(mgr.CheckEtsModuleLoadable(className, fileName));
    EXPECT_FALSE(mgr.CheckEtsModuleLoadable("bundle.bundleApplication", "bundle.bundleApp"));
}

/**
 * @tc.name: CheckEtsModuleLoadable_ShouldReturnFalse_WhenFileNameIsLongerThanClassName
 * @tc.desc: func should return false when file name is longer than class name.
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionConfigMgrTest, CheckEtsModuleLoadable_ShouldReturnFalse_WhenFileNameIsLongerThanClassName,
    TestSize.Level1)
{
    ExtensionConfigMgr mgr;
    std::string className = "file";
    std::string fileName = "fileName";
    EXPECT_FALSE(mgr.CheckEtsModuleLoadable(className, fileName));
}

/**
 * @tc.name: CheckEtsModuleLoadable_ShouldReturnFalse_WhenFileNameInBlockListAndStartsWithOHOS
 * @tc.desc: func should return false when file name in blocklist and startswith ohos.
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionConfigMgrTest, CheckEtsModuleLoadable_ShouldReturnFalse_WhenFileNameInBlockListAndStartsWithOHOS,
    TestSize.Level1)
{
    ExtensionConfigMgr mgr;
    mgr.LoadExtensionBlockList(BLOCK_LIST_ITEM_DRIVER_EXTENSION, EXTENSION_TYPE_DRIVER);
    mgr.extensionType_ = EXTENSION_TYPE_DRIVER;
    mgr.GenerateExtensionEtsBlocklists();
    EXPECT_FALSE(mgr.CheckEtsModuleLoadable("@ohos.abilityAccessCtrl.abilityAccessCtrl", "@ohos.abilityAccessCtrl"));
}

/**
 * @tc.name: CheckEtsModuleLoadable_ShouldReturnFalse_WhenFileNameInBlockListAndStartsWithXXX
 * @tc.desc: func should return false when file name in blocklist and startswith xxx.
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionConfigMgrTest, CheckEtsModuleLoadable_ShouldReturnFalse_WhenFileNameInBlockListAndStartsWithXXX,
    TestSize.Level1)
{
    ExtensionConfigMgr mgr;
    mgr.LoadExtensionBlockList(BLOCK_LIST_ITEM_DRIVER_EXTENSION, EXTENSION_TYPE_DRIVER);
    mgr.extensionType_ = EXTENSION_TYPE_DRIVER;
    mgr.GenerateExtensionEtsBlocklists();
    EXPECT_FALSE(mgr.CheckEtsModuleLoadable("@xxx.app.ability.appManager", "@xxx.app.ability.appManager"));
}

/**
 * @tc.name: CheckEtsModuleLoadable_ShouldReturnFalse_WhenFileNameInBlockListAndNotStartsWithPrefix
 * @tc.desc: func should return false when file name in blocklist and not startswith prefix.
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionConfigMgrTest,
    CheckEtsModuleLoadable_ShouldReturnFalse_WhenFileNameInBlockListAndNotStartsWithPrefix, TestSize.Level1)
{
    ExtensionConfigMgr mgr;
    mgr.LoadExtensionBlockList(BLOCK_LIST_ITEM_DRIVER_EXTENSION, EXTENSION_TYPE_DRIVER);
    mgr.extensionType_ = EXTENSION_TYPE_DRIVER;
    mgr.GenerateExtensionEtsBlocklists();

    EXPECT_FALSE(mgr.CheckEtsModuleLoadable("abilityAccessCtrl.XXX", "abilityAccessCtrl"));
    EXPECT_FALSE(mgr.CheckEtsModuleLoadable("bundle.bundle", "bundle"));
    EXPECT_FALSE(mgr.CheckEtsModuleLoadable("bundle.bundleManager.bundleManager", "bundle.bundleManager"));
}

/**
 * @tc.name: CheckEtsModuleLoadable_ShouldReturnTrue_WhenFileNameNotInBlockListAndFileNameLengthIsSameWithClassName
 * @tc.desc: func should return true when file name is not in blocklist and its length is same with class name.
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionConfigMgrTest,
    CheckEtsModuleLoadable_ShouldReturnTrue_WhenFileNameNotInBlockListAndFileNameLengthIsSameWithClassName,
    TestSize.Level1)
{
    ExtensionConfigMgr mgr;
    mgr.LoadExtensionBlockList(BLOCK_LIST_ITEM_DRIVER_EXTENSION, EXTENSION_TYPE_DRIVER);
    mgr.extensionType_ = EXTENSION_TYPE_DRIVER;
    mgr.GenerateExtensionEtsBlocklists();

    EXPECT_TRUE(mgr.CheckEtsModuleLoadable("@xxx.111.111", "@xxx.111.111"));
    EXPECT_TRUE(mgr.CheckEtsModuleLoadable("111.111", "111.111"));
}

/**
 * @tc.name: CheckEtsModuleLoadable_ShouldReturnFalse_WhenFileNameInBlockListAndFileNameLengthIsSameWithClassName
 * @tc.desc: func should return false when file name is in blocklist and its length is same with class name.
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionConfigMgrTest,
    CheckEtsModuleLoadable_ShouldReturnFalse_WhenFileNameInBlockListAndFileNameLengthIsSameWithClassName,
    TestSize.Level1)
{
    ExtensionConfigMgr mgr;
    mgr.LoadExtensionBlockList(BLOCK_LIST_ITEM_DRIVER_EXTENSION, EXTENSION_TYPE_DRIVER);
    mgr.extensionType_ = EXTENSION_TYPE_DRIVER;
    mgr.GenerateExtensionEtsBlocklists();

    EXPECT_FALSE(mgr.CheckEtsModuleLoadable("bundle.appControl", "bundle.appControl"));
}

/**
 * @tc.name: CheckEtsModuleLoadable_ShouldReturnFalse_WhenFileNameIsNotEqualWithClassNameButLengthIsSame
 * @tc.desc: func should return false when file name is in blocklist and the length is same.
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionConfigMgrTest,
    CheckEtsModuleLoadable_ShouldReturnFalse_WhenFileNameIsNotEqualWithClassNameButLengthIsSame, TestSize.Level1)
{
    ExtensionConfigMgr mgr;
    mgr.GenerateExtensionEtsBlocklists();

    EXPECT_FALSE(mgr.CheckEtsModuleLoadable("appControl", "appContron"));
}

/**
 * @tc.name: CheckEtsModuleLoadable_ShouldReturnTrue_WhenFileNameNotInBlockListAndStartsWithPrefix
 * @tc.desc: func should return true when file name not in blocklist and startswith prefix.
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionConfigMgrTest,
    CheckEtsModuleLoadable_ShouldReturnTrue_WhenFileNameNotInBlockListAndStartsWithPrefix, TestSize.Level1)
{
    ExtensionConfigMgr mgr;
    mgr.LoadExtensionBlockList(BLOCK_LIST_ITEM_DRIVER_EXTENSION, EXTENSION_TYPE_DRIVER);
    mgr.extensionType_ = EXTENSION_TYPE_DRIVER;
    mgr.GenerateExtensionEtsBlocklists();

    EXPECT_TRUE(mgr.CheckEtsModuleLoadable("@xxx.app.app.appManager.appManager", "@xxx.app.app.appManager"));
    EXPECT_TRUE(mgr.CheckEtsModuleLoadable("@ohos.app.app.appManager.appManager", "@ohos.app.app.appManager"));
}

/**
 * @tc.name: UpdateRuntimeModuleChecker_ShouldHandleEtsRuntime_WhenLanguageIsEts
 * @tc.desc: func should handle ets runtime when language is EtsRuntime.
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionConfigMgrTest, UpdateRuntimeModuleChecker_ShouldHandleEtsRuntime_WhenLanguageIsEts, TestSize.Level1)
{
    ExtensionConfigMgr mgr;
    mgr.UpdateRuntimeModuleChecker(nullptr);
    auto mockRuntimePtr = std::make_unique<MockRuntime>();
    EXPECT_NE(mockRuntimePtr, nullptr);
    std::unique_ptr<Runtime> runtime = std::move(mockRuntimePtr);
    mgr.UpdateRuntimeModuleChecker(runtime);
    MockRuntime &mockRuntime = static_cast<MockRuntime&>(*runtime);
    EXPECT_TRUE(mockRuntime.extensionApiCheckerFlag_);
    EXPECT_TRUE(mockRuntime.loadCheckerFlag_);
    auto checkFunc = mockRuntime.GetExtensionApiCheckCallback();
    EXPECT_NE(checkFunc, nullptr);
    EXPECT_FALSE(checkFunc("111", "222"));
}

/**
 * @tc.name: UpdateRuntimeModuleChecker_ShouldHandleNotEtsRuntime_WhenLanguageIsJs
 * @tc.desc: func should handle not ets runtime when language is JsRuntime.
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionConfigMgrTest, UpdateRuntimeModuleChecker_ShouldHandleNotEtsRuntime_WhenLanguageIsJs,
    TestSize.Level1)
{
    ExtensionConfigMgr mgr;
    auto mockRuntimePtr = std::make_unique<MockRuntime>();
    EXPECT_NE(mockRuntimePtr, nullptr);
    mockRuntimePtr->SetLanguage(Runtime::Language::JS);
    std::unique_ptr<Runtime> runtime = std::move(mockRuntimePtr);
    mgr.UpdateRuntimeModuleChecker(runtime);
    MockRuntime &mockRuntime = static_cast<MockRuntime&>(*runtime);
    EXPECT_TRUE(mockRuntime.loadCheckerFlag_);
    EXPECT_FALSE(mockRuntime.extensionApiCheckerFlag_);
}

/**
 * @tc.name: LoadExtensionBlockList_ShouldReturnEarly_WhenAlreadyLoaded
 * @tc.desc: Test LoadExtensionBlockList when the same type is already loaded.
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionConfigMgrTest, LoadExtensionBlockList_ShouldReturnEarly_WhenAlreadyLoaded, TestSize.Level1)
{
    ExtensionConfigMgr mgr;
    // First load
    mgr.LoadExtensionBlockList(BLOCK_LIST_ITEM_SERVICE_EXTENSION, EXTENSION_TYPE_SERVICE);
    bool result = (mgr.extensionBlocklist_.find(EXTENSION_TYPE_SERVICE) != mgr.extensionBlocklist_.end());
    EXPECT_TRUE(result);

    // Get the size after first load
    auto iter = mgr.extensionBlocklist_.find(EXTENSION_TYPE_SERVICE);
    size_t firstSize = iter->second.size();

    // Second load with same type - should return early and not modify
    mgr.LoadExtensionBlockList(BLOCK_LIST_ITEM_SERVICE_EXTENSION, EXTENSION_TYPE_SERVICE);
    result = (mgr.extensionBlocklist_.find(EXTENSION_TYPE_SERVICE) != mgr.extensionBlocklist_.end());
    EXPECT_TRUE(result);

    // Verify size hasn't changed
    iter = mgr.extensionBlocklist_.find(EXTENSION_TYPE_SERVICE);
    EXPECT_EQ(iter->second.size(), firstSize);
}

/**
 * @tc.name: GetStringAfterRemovePreFix_ShouldReturnOriginal_WhenNameStartsWithAtButNoDot
 * @tc.desc: func should return original name when it starts with @ but has no dot.
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionConfigMgrTest, GetStringAfterRemovePreFix_ShouldReturnOriginal_WhenNameStartsWithAtButNoDot,
    TestSize.Level1)
{
    ExtensionConfigMgr mgr;
    std::string input = "@ohos";
    std::string expectOutput = "@ohos";
    EXPECT_EQ(mgr.GetStringAfterRemovePreFix(input), expectOutput);

    input = "@xxx";
    expectOutput = "@xxx";
    EXPECT_EQ(mgr.GetStringAfterRemovePreFix(input), expectOutput);

    input = "@";
    expectOutput = "@";
    EXPECT_EQ(mgr.GetStringAfterRemovePreFix(input), expectOutput);
}

/**
 * @tc.name: UpdateRuntimeModuleChecker_ShouldPassEmptyBlocklist_WhenCalledSecondTime
 * @tc.desc: func should pass empty localBlocklist to AppModuleChecker when called second time.
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionConfigMgrTest, UpdateRuntimeModuleChecker_ShouldPassEmptyBlocklist_WhenCalledSecondTime,
    TestSize.Level1)
{
    ExtensionConfigMgr mgr;
    mgr.LoadExtensionBlockList(BLOCK_LIST_ITEM_FORM_EXTENSION, EXTENSION_TYPE_FORM);
    mgr.extensionType_ = EXTENSION_TYPE_FORM;

    auto mockRuntimePtr = std::make_unique<MockRuntime>();
    mockRuntimePtr->SetLanguage(Runtime::Language::JS);
    std::unique_ptr<Runtime> runtime = std::move(mockRuntimePtr);

    mgr.UpdateRuntimeModuleChecker(runtime);
    MockRuntime &mockRuntime = static_cast<MockRuntime&>(*runtime);
    EXPECT_TRUE(mockRuntime.loadCheckerFlag_);

    auto firstChecker = mockRuntime.GetModuleChecker();
    EXPECT_NE(firstChecker, nullptr);

    std::unique_ptr<ApiAllowListChecker> apiAllowListChecker;
    bool firstCheckResult = firstChecker->CheckModuleLoadable("multimedia.camera", apiAllowListChecker, false);
    EXPECT_FALSE(firstCheckResult);

    mockRuntime.ClearModuleChecker();
    mgr.UpdateRuntimeModuleChecker(runtime);

    auto secondChecker = mockRuntime.GetModuleChecker();
    EXPECT_NE(secondChecker, nullptr);
    EXPECT_TRUE(mockRuntime.loadCheckerFlag_);

    std::unique_ptr<ApiAllowListChecker> apiAllowListChecker2;
    bool secondCheckResult = secondChecker->CheckModuleLoadable("multimedia.camera", apiAllowListChecker2, false);
    EXPECT_TRUE(secondCheckResult);
}
}  // namespace AbilityRuntime
}  // namespace OHOS
