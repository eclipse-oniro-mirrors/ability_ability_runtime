/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#include "ability_record.h"
#include "start_options_utils.h"
#undef private

#include "ability_manager_errors.h"
#include "hilog_tag_wrapper.h"
#include "mock_my_flag.h"
#include "process_options.h"
#include "start_options.h"
#include "ui_ability_lifecycle_manager.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AAFwk {
class StartOptionsUtilsTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void StartOptionsUtilsTest::SetUpTestCase() {}

void StartOptionsUtilsTest::TearDownTestCase() {}

void StartOptionsUtilsTest::SetUp() {}

void StartOptionsUtilsTest::TearDown() {}

/*
 * Feature: StartOptionsUtils
 * Function: CheckProcessOptions
 * SubFunction: NA
 * FunctionPoints: StartOptionsUtils CheckProcessOptions
 */
HWTEST_F(StartOptionsUtilsTest, CheckProcessOptions_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartOptionsUtilsTest CheckProcessOptions_001 start");

    MyFlag::GetInstance().isHiddenStart_ = true;
    MyFlag::GetInstance().retHiddenStartSupported_ = ERR_OK;

    Want want;
    StartOptions startOptions;
    auto ret = StartOptionsUtils::CheckProcessOptions(want, startOptions, nullptr, -1);
    EXPECT_EQ(ret, ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "StartOptionsUtilsTest CheckProcessOptions_001 end");
}

/*
 * Feature: StartOptionsUtils
 * Function: CheckProcessOptions
 * SubFunction: NA
 * FunctionPoints: StartOptionsUtils CheckProcessOptions
 */
HWTEST_F(StartOptionsUtilsTest, CheckProcessOptions_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartOptionsUtilsTest CheckProcessOptions_002 start");

    MyFlag::GetInstance().isHiddenStart_ = false;
    MyFlag::GetInstance().isStartSelfUIAbility_ = true;

    Want want;
    StartOptions startOptions;
    startOptions.processOptions = std::make_shared<ProcessOptions>();
    startOptions.processOptions->isStartFromNDK = true;
    auto ret = StartOptionsUtils::CheckProcessOptions(want, startOptions, nullptr, -1);
    EXPECT_EQ(ret, StartOptionsUtils::CheckStartSelfUIAbilityStartOptions(want, startOptions));

    TAG_LOGI(AAFwkTag::TEST, "StartOptionsUtilsTest CheckProcessOptions_002 end");
}

/*
 * Feature: StartOptionsUtils
 * Function: CheckProcessOptions
 * SubFunction: NA
 * FunctionPoints: StartOptionsUtils CheckProcessOptions
 */
HWTEST_F(StartOptionsUtilsTest, CheckProcessOptions_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartOptionsUtilsTest CheckProcessOptions_003 start");

    MyFlag::GetInstance().isHiddenStart_ = false;
    MyFlag::GetInstance().isStartSelfUIAbility_ = false;

    Want want;
    StartOptions startOptions;
    auto ret = StartOptionsUtils::CheckProcessOptions(want, startOptions, nullptr, -1);
    EXPECT_EQ(ret, ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "StartOptionsUtilsTest CheckProcessOptions_003 end");
}

/*
 * Feature: StartOptionsUtils
 * Function: CheckStartSelfUIAbilityStartOptions
 * SubFunction: NA
 * FunctionPoints: StartOptionsUtils CheckStartSelfUIAbilityStartOptions
 */
HWTEST_F(StartOptionsUtilsTest, CheckStartSelfUIAbilityStartOptions_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartOptionsUtilsTest CheckStartSelfUIAbilityStartOptions_001 start");

    Want want;
    StartOptions startOptions;
    auto ret = StartOptionsUtils::CheckStartSelfUIAbilityStartOptions(want, startOptions);
    EXPECT_EQ(ret, ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "StartOptionsUtilsTest CheckStartSelfUIAbilityStartOptions_001 end");
}

/*
 * Feature: StartOptionsUtils
 * Function: CheckStartSelfUIAbilityStartOptions
 * SubFunction: NA
 * FunctionPoints: StartOptionsUtils CheckStartSelfUIAbilityStartOptions
 */
HWTEST_F(StartOptionsUtilsTest, CheckStartSelfUIAbilityStartOptions_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartOptionsUtilsTest CheckStartSelfUIAbilityStartOptions_002 start");

    MyFlag::GetInstance().isScbEnabled_ = false;

    Want want;
    StartOptions startOptions;
    startOptions.processOptions = std::make_shared<ProcessOptions>();
    auto ret = StartOptionsUtils::CheckStartSelfUIAbilityStartOptions(want, startOptions);
    EXPECT_EQ(ret, ERR_CAPABILITY_NOT_SUPPORT);
    TAG_LOGI(AAFwkTag::TEST, "StartOptionsUtilsTest CheckStartSelfUIAbilityStartOptions_002 end");
}

/*
 * Feature: StartOptionsUtils
 * Function: CheckStartSelfUIAbilityStartOptions
 * SubFunction: NA
 * FunctionPoints: StartOptionsUtils CheckStartSelfUIAbilityStartOptions
 */
HWTEST_F(StartOptionsUtilsTest, CheckStartSelfUIAbilityStartOptions_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartOptionsUtilsTest CheckStartSelfUIAbilityStartOptions_003 start");

    MyFlag::GetInstance().isScbEnabled_ = true;
    MyFlag::GetInstance().isStartOptionsWithProcessOptions_ = false;

    Want want;
    StartOptions startOptions;
    startOptions.processOptions = std::make_shared<ProcessOptions>();
    auto ret = StartOptionsUtils::CheckStartSelfUIAbilityStartOptions(want, startOptions);
    EXPECT_EQ(ret, ERR_CAPABILITY_NOT_SUPPORT);
    TAG_LOGI(AAFwkTag::TEST, "StartOptionsUtilsTest CheckStartSelfUIAbilityStartOptions_003 end");
}

/*
 * Feature: StartOptionsUtils
 * Function: CheckStartSelfUIAbilityStartOptions
 * SubFunction: NA
 * FunctionPoints: StartOptionsUtils CheckStartSelfUIAbilityStartOptions
 */
HWTEST_F(StartOptionsUtilsTest, CheckStartSelfUIAbilityStartOptions_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartOptionsUtilsTest CheckStartSelfUIAbilityStartOptions_004 start");

    MyFlag::GetInstance().isScbEnabled_ = true;
    MyFlag::GetInstance().isStartOptionsWithProcessOptions_ = true;
    MyFlag::GetInstance().uiManager_ = nullptr;

    Want want;
    StartOptions startOptions;
    startOptions.processOptions = std::make_shared<ProcessOptions>();
    auto ret = StartOptionsUtils::CheckStartSelfUIAbilityStartOptions(want, startOptions);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);

    TAG_LOGI(AAFwkTag::TEST, "StartOptionsUtilsTest CheckStartSelfUIAbilityStartOptions_004 end");
}

/*
 * Feature: StartOptionsUtils
 * Function: CheckStartSelfUIAbilityStartOptions
 * SubFunction: NA
 * FunctionPoints: StartOptionsUtils CheckStartSelfUIAbilityStartOptions
 */
HWTEST_F(StartOptionsUtilsTest, CheckStartSelfUIAbilityStartOptions_005, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartOptionsUtilsTest CheckStartSelfUIAbilityStartOptions_005 start");

    MyFlag::GetInstance().isScbEnabled_ = true;
    MyFlag::GetInstance().isStartOptionsWithProcessOptions_ = true;
    MyFlag::GetInstance().uiManager_ = std::make_shared<UIAbilityLifecycleManager>();
    MyFlag::GetInstance().isCallerInStatusBar_ = false;

    Want want;
    StartOptions startOptions;
    startOptions.processOptions = std::make_shared<ProcessOptions>();
    auto ret = StartOptionsUtils::CheckStartSelfUIAbilityStartOptions(want, startOptions);
    EXPECT_EQ(ret, ERR_START_OPTIONS_CHECK_FAILED);

    TAG_LOGI(AAFwkTag::TEST, "StartOptionsUtilsTest CheckStartSelfUIAbilityStartOptions_005 end");
}

/*
 * Feature: StartOptionsUtils
 * Function: CheckStartSelfUIAbilityStartOptions
 * SubFunction: NA
 * FunctionPoints: StartOptionsUtils CheckStartSelfUIAbilityStartOptions
 */
HWTEST_F(StartOptionsUtilsTest, CheckStartSelfUIAbilityStartOptions_006, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartOptionsUtilsTest CheckStartSelfUIAbilityStartOptions_006 start");

    MyFlag::GetInstance().isScbEnabled_ = true;
    MyFlag::GetInstance().isStartOptionsWithProcessOptions_ = true;
    MyFlag::GetInstance().uiManager_ = std::make_shared<UIAbilityLifecycleManager>();
    MyFlag::GetInstance().isCallerInStatusBar_ = true;

    Want want;
    StartOptions startOptions;
    startOptions.processOptions = std::make_shared<ProcessOptions>();
    auto ret = StartOptionsUtils::CheckStartSelfUIAbilityStartOptions(want, startOptions);
    EXPECT_EQ(ret, ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "StartOptionsUtilsTest CheckStartSelfUIAbilityStartOptions_006 end");
}

/*
 * Feature: StartOptionsUtils
 * Function: CheckStartSelfUIAbilityStartOptions
 * SubFunction: NA
 * FunctionPoints: StartOptionsUtils CheckStartSelfUIAbilityStartOptions
 */
HWTEST_F(StartOptionsUtilsTest, CheckStartSelfUIAbilityStartOptions_007, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartOptionsUtilsTest CheckStartSelfUIAbilityStartOptions_007 start");

    MyFlag::GetInstance().isScbEnabled_ = true;
    MyFlag::GetInstance().isStartOptionsWithProcessOptions_ = true;
    MyFlag::GetInstance().uiManager_ = nullptr;

    Want want;
    StartOptions startOptions;
    startOptions.processOptions = std::make_shared<ProcessOptions>();
    startOptions.processOptions->startupVisibility = StartupVisibility::UNSPECIFIED;
    startOptions.processOptions->loadAbilityCallbackId = 12345;
    auto ret = StartOptionsUtils::CheckStartSelfUIAbilityStartOptions(want, startOptions);
    EXPECT_EQ(ret, ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "StartOptionsUtilsTest CheckStartSelfUIAbilityStartOptions_007 end");
}

/*
 * Feature: StartOptionsUtils
 * Name: CheckProcessOptionsInner_001
 * Function: CheckProcessOptionsInner
 * SubFunction: NA
 * FunctionPoints: StartOptionsUtils CheckProcessOptionsInner
 */
HWTEST_F(StartOptionsUtilsTest, CheckProcessOptionsInner_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartOptionsUtilsTest CheckProcessOptionsInner_001 start");
    Want want;
    StartOptions startOptions;
    auto retCode = StartOptionsUtils::CheckProcessOptionsInner(want, startOptions, nullptr, -1);
    EXPECT_EQ(retCode, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "StartOptionsUtilsTest CheckProcessOptionsInner_001 end");
}


/*
 * Feature: StartOptionsUtils
 * Name: CheckProcessOptionsInner_002
 * Function: CheckProcessOptionsInner
 * SubFunction: NA
 * FunctionPoints: StartOptionsUtils CheckProcessOptionsInner
 */
HWTEST_F(StartOptionsUtilsTest, CheckProcessOptionsInner_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartOptionsUtilsTest CheckProcessOptionsInner_002 start");
    Want want;
    StartOptions startOptions;
    startOptions.processOptions = std::make_shared<ProcessOptions>();
    startOptions.processOptions->processMode = ProcessMode::UNSPECIFIED;
    auto retCode = StartOptionsUtils::CheckProcessOptionsInner(want, startOptions, nullptr, -1);
    EXPECT_EQ(retCode, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "StartOptionsUtilsTest CheckProcessOptionsInner_002 end");
}

/*
 * Feature: StartOptionsUtils
 * Name: CheckProcessOptionsInner_003
 * Function: CheckProcessOptionsInner
 * SubFunction: NA
 * FunctionPoints: StartOptionsUtils CheckProcessOptionsInner
 */
HWTEST_F(StartOptionsUtilsTest, CheckProcessOptionsInner_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartOptionsUtilsTest CheckProcessOptionsInner_003 start");

    MyFlag::GetInstance().isScbEnabled_ = false;

    Want want;
    StartOptions startOptions;
    startOptions.processOptions = std::make_shared<ProcessOptions>();
    startOptions.processOptions->processMode = ProcessMode::NEW_PROCESS_ATTACH_TO_PARENT;
    auto retCode = StartOptionsUtils::CheckProcessOptionsInner(want, startOptions, nullptr, -1);
    EXPECT_EQ(retCode, ERR_CAPABILITY_NOT_SUPPORT);
    TAG_LOGI(AAFwkTag::TEST, "StartOptionsUtilsTest CheckProcessOptionsInner_003 end");
}

/*
 * Feature: StartOptionsUtils
 * Name: CheckProcessOptionsInner_004
 * Function: CheckProcessOptionsInner
 * SubFunction: NA
 * FunctionPoints: StartOptionsUtils CheckProcessOptionsInner
 */
HWTEST_F(StartOptionsUtilsTest, CheckProcessOptionsInner_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartOptionsUtilsTest CheckProcessOptionsInner_004 start");

    MyFlag::GetInstance().isScbEnabled_ = true;
    MyFlag::GetInstance().isStartOptionsWithProcessOptions_ = false;

    Want want;
    StartOptions startOptions;
    startOptions.processOptions = std::make_shared<ProcessOptions>();
    startOptions.processOptions->processMode = ProcessMode::NEW_PROCESS_ATTACH_TO_PARENT;
    auto retCode = StartOptionsUtils::CheckProcessOptionsInner(want, startOptions, nullptr, -1);
    EXPECT_EQ(retCode, ERR_CAPABILITY_NOT_SUPPORT);

    TAG_LOGI(AAFwkTag::TEST, "StartOptionsUtilsTest CheckProcessOptionsInner_004 end");
}

/*
 * Feature: StartOptionsUtils
 * Name: CheckProcessOptionsInner_005
 * Function: CheckProcessOptionsInner
 * SubFunction: NA
 * FunctionPoints: StartOptionsUtils CheckProcessOptionsInner
 */
HWTEST_F(StartOptionsUtilsTest, CheckProcessOptionsInner_005, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartOptionsUtilsTest CheckProcessOptionsInner_005 start");

    MyFlag::GetInstance().isScbEnabled_ = true;
    MyFlag::GetInstance().isStartOptionsWithProcessOptions_ = true;

    Want want;
    StartOptions startOptions;
    startOptions.processOptions = std::make_shared<ProcessOptions>();
    startOptions.processOptions->processMode = ProcessMode::NEW_PROCESS_ATTACH_TO_PARENT;
    auto retCode = StartOptionsUtils::CheckProcessOptionsInner(want, startOptions, nullptr, -1);
    EXPECT_EQ(retCode, ERR_NOT_ALLOW_IMPLICIT_START);

    TAG_LOGI(AAFwkTag::TEST, "StartOptionsUtilsTest CheckProcessOptionsInner_005 end");
}

/*
 * Feature: StartOptionsUtils
 * Name: CheckProcessOptionsInner_006
 * Function: CheckProcessOptionsInner
 * SubFunction: NA
 * FunctionPoints: StartOptionsUtils CheckProcessOptionsInner
 */
HWTEST_F(StartOptionsUtilsTest, CheckProcessOptionsInner_006, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartOptionsUtilsTest CheckProcessOptionsInner_006 start");

    MyFlag::GetInstance().isScbEnabled_ = true;
    MyFlag::GetInstance().isStartOptionsWithProcessOptions_ = true;
    MyFlag::GetInstance().retCheckSpecificSystemAbilityAccessPermission_ = true;

    Want want;
    want.SetElementName("bundle", "ability");
    StartOptions startOptions;
    startOptions.processOptions = std::make_shared<ProcessOptions>();
    startOptions.processOptions->isRestartKeepAlive = true;
    auto retCode = StartOptionsUtils::CheckProcessOptionsInner(want, startOptions, nullptr, -1);
    EXPECT_EQ(retCode, ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "StartOptionsUtilsTest CheckProcessOptionsInner_006 end");
}

/*
 * Feature: StartOptionsUtils
 * Name: CheckProcessOptionsInner_007
 * Function: CheckProcessOptionsInner
 * SubFunction: NA
 * FunctionPoints: StartOptionsUtils CheckProcessOptionsInner
 */
HWTEST_F(StartOptionsUtilsTest, CheckProcessOptionsInner_007, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartOptionsUtilsTest CheckProcessOptionsInner_007 start");

    MyFlag::GetInstance().isScbEnabled_ = true;
    MyFlag::GetInstance().isStartOptionsWithProcessOptions_ = true;
    MyFlag::GetInstance().retCheckSpecificSystemAbilityAccessPermission_ = false;
    MyFlag::GetInstance().retCheckCallingTokenId_ = false;
    MyFlag::GetInstance().setTargetCloneIndexCallCount_ = 0;
    MyFlag::GetInstance().resolveTargetAppCloneIndexCallCount_ = 0;
    MyFlag::GetInstance().resolveTargetAppCloneIndexUserId_ = -1;

    Want want;
    want.SetElementName("bundle", "ability");
    StartOptions startOptions;
    startOptions.processOptions = std::make_shared<ProcessOptions>();
    startOptions.processOptions->processMode = ProcessMode::NEW_PROCESS_ATTACH_TO_PARENT;
    constexpr int32_t userId = 100;
    auto retCode = StartOptionsUtils::CheckProcessOptionsInner(want, startOptions, nullptr, userId);
    EXPECT_EQ(retCode, ERR_NOT_SELF_APPLICATION);
    EXPECT_EQ(MyFlag::GetInstance().resolveTargetAppCloneIndexCallCount_, 1);
    EXPECT_EQ(MyFlag::GetInstance().resolveTargetAppCloneIndexUserId_, userId);
    EXPECT_EQ(MyFlag::GetInstance().setTargetCloneIndexCallCount_, 0);

    TAG_LOGI(AAFwkTag::TEST, "StartOptionsUtilsTest CheckProcessOptionsInner_007 end");
}

/*
 * Feature: StartOptionsUtils
 * Name: CheckProcessOptionsInner_008
 * Function: CheckProcessOptionsInner
 * SubFunction: NA
 * FunctionPoints: StartOptionsUtils CheckProcessOptionsInner
 */
HWTEST_F(StartOptionsUtilsTest, CheckProcessOptionsInner_008, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartOptionsUtilsTest CheckProcessOptionsInner_008 start");

    MyFlag::GetInstance().isScbEnabled_ = true;
    MyFlag::GetInstance().isStartOptionsWithProcessOptions_ = true;
    MyFlag::GetInstance().retCheckSpecificSystemAbilityAccessPermission_ = false;
    MyFlag::GetInstance().retCheckCallingTokenId_ = true;
    MyFlag::GetInstance().uiManager_ = nullptr;

    Want want;
    want.SetElementName("bundle", "ability");
    StartOptions startOptions;
    startOptions.processOptions = std::make_shared<ProcessOptions>();
    startOptions.processOptions->processMode = ProcessMode::NEW_PROCESS_ATTACH_TO_PARENT;
    auto retCode = StartOptionsUtils::CheckProcessOptionsInner(want, startOptions, nullptr, -1);
    EXPECT_EQ(retCode, ERR_INVALID_VALUE);

    TAG_LOGI(AAFwkTag::TEST, "StartOptionsUtilsTest CheckProcessOptionsInner_008 end");
}

/*
 * Feature: StartOptionsUtils
 * Name: CheckProcessOptionsInner_009
 * Function: CheckProcessOptionsInner
 * SubFunction: NA
 * FunctionPoints: StartOptionsUtils CheckProcessOptionsInner
 */
HWTEST_F(StartOptionsUtilsTest, CheckProcessOptionsInner_009, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartOptionsUtilsTest CheckProcessOptionsInner_009 start");

    MyFlag::GetInstance().isScbEnabled_ = true;
    MyFlag::GetInstance().isStartOptionsWithProcessOptions_ = true;
    MyFlag::GetInstance().retCheckSpecificSystemAbilityAccessPermission_ = false;
    MyFlag::GetInstance().retCheckCallingTokenId_ = true;
    MyFlag::GetInstance().uiManager_ = std::make_shared<UIAbilityLifecycleManager>();
    MyFlag::GetInstance().isCallerInStatusBar_ = false;

    Want want;
    want.SetElementName("bundle", "ability");
    StartOptions startOptions;
    startOptions.processOptions = std::make_shared<ProcessOptions>();
    startOptions.processOptions->processMode = ProcessMode::ATTACH_TO_STATUS_BAR_ITEM;
    auto retCode = StartOptionsUtils::CheckProcessOptionsInner(want, startOptions, nullptr, -1);
    EXPECT_EQ(retCode, ERR_START_OPTIONS_CHECK_FAILED);

    TAG_LOGI(AAFwkTag::TEST, "StartOptionsUtilsTest CheckProcessOptionsInner_009 end");
}

/*
 * Feature: StartOptionsUtils
 * Name: CheckProcessOptionsInner_010
 * Function: CheckProcessOptionsInner
 * SubFunction: NA
 * FunctionPoints: StartOptionsUtils CheckProcessOptionsInner
 */
HWTEST_F(StartOptionsUtilsTest, CheckProcessOptionsInner_010, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartOptionsUtilsTest CheckProcessOptionsInner_010 start");

    MyFlag::GetInstance().isScbEnabled_ = true;
    MyFlag::GetInstance().isStartOptionsWithProcessOptions_ = true;
    MyFlag::GetInstance().retCheckSpecificSystemAbilityAccessPermission_ = false;
    MyFlag::GetInstance().retCheckCallingTokenId_ = true;
    MyFlag::GetInstance().uiManager_ = std::make_shared<UIAbilityLifecycleManager>();
    MyFlag::GetInstance().isCallerInStatusBar_ = true;
    auto abilityRecord = std::make_shared<AbilityRecord>();
    abilityRecord->abilityInfo.launchMode = AppExecFwk::LaunchMode::SINGLETON;
    MyFlag::GetInstance().abilityRecords_ = { abilityRecord };

    Want want;
    want.SetElementName("bundle", "ability");
    StartOptions startOptions;
    startOptions.processOptions = std::make_shared<ProcessOptions>();
    startOptions.processOptions->processMode = ProcessMode::ATTACH_TO_STATUS_BAR_ITEM;
    auto retCode = StartOptionsUtils::CheckProcessOptionsInner(want, startOptions, nullptr, -1);
    EXPECT_EQ(retCode, ERR_ABILITY_ALREADY_RUNNING);

    TAG_LOGI(AAFwkTag::TEST, "StartOptionsUtilsTest CheckProcessOptionsInner_010 end");
}

/*
 * Feature: StartOptionsUtils
 * Name: CheckProcessOptionsInner_011
 * Function: CheckProcessOptionsInner
 * SubFunction: NA
 * FunctionPoints: StartOptionsUtils CheckProcessOptionsInner
 */
HWTEST_F(StartOptionsUtilsTest, CheckProcessOptionsInner_011, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartOptionsUtilsTest CheckProcessOptionsInner_011 start");

    MyFlag::GetInstance().isScbEnabled_ = true;
    MyFlag::GetInstance().isStartOptionsWithProcessOptions_ = true;
    MyFlag::GetInstance().retCheckSpecificSystemAbilityAccessPermission_ = false;
    MyFlag::GetInstance().retCheckCallingTokenId_ = true;
    MyFlag::GetInstance().uiManager_ = std::make_shared<UIAbilityLifecycleManager>();
    MyFlag::GetInstance().isCallerInStatusBar_ = true;
    MyFlag::GetInstance().abilityRecords_.clear();

    Want want;
    want.SetElementName("bundle", "ability");
    StartOptions startOptions;
    startOptions.processOptions = std::make_shared<ProcessOptions>();
    startOptions.processOptions->processMode = ProcessMode::ATTACH_TO_STATUS_BAR_ITEM;
    auto retCode = StartOptionsUtils::CheckProcessOptionsInner(want, startOptions, nullptr, -1);
    EXPECT_EQ(retCode, ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "StartOptionsUtilsTest CheckProcessOptionsInner_011 end");
}
}  // namespace AAFwk
}  // namespace OHOS
