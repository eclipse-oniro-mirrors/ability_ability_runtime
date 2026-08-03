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
#include "mock_permission_verification.h"
#define private public
#include "utils/ability_permission_util.h"
#include "ability_info.h"
#include "ability_record.h"
#include "ability_util.h"
#include "accesstoken_kit.h"
#include "app_utils.h"
#include "ipc_skeleton.h"
#include "running_process_info.h"
#include "permission_constants.h"
#include "permission_verification.h"
#undef private
#include "hilog_tag_wrapper.h"
#include "parameters.h"
#include "mock_my_flag.h"
#include "start_ability_utils.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AppExecFwk;
using AbilityRequest = OHOS::AAFwk::AbilityRequest;
using OHOS::AppExecFwk::AbilityType;
using OHOS::AAFwk::AbilityPermissionUtil;

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr int32_t ERR_INVALID_VALUE = 22;
} // namespace
class AbilityPermissionUtilSecondTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void AbilityPermissionUtilSecondTest::SetUpTestCase(void) {}
void AbilityPermissionUtilSecondTest::TearDownTestCase(void) {}
void AbilityPermissionUtilSecondTest::SetUp() {}
void AbilityPermissionUtilSecondTest::TearDown() {}

/**
 * @tc.name: AbilityPermissionUtil_CheckMultiInstanceAndAppClone_0100
 * @tc.desc: IsDominateScreen
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(AbilityPermissionUtilSecondTest, AbilityPermissionUtil_IsDominateScreen_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityPermissionUtil_IsDominateScreen_0100 start");
    Want want;
    auto result = AbilityPermissionUtil::GetInstance().IsDominateScreen(want, false);
    EXPECT_TRUE(result);
    TAG_LOGI(AAFwkTag::TEST, "AbilityPermissionUtil_IsDominateScreen_0100 end");
}

/**
 * @tc.name: AbilityPermissionUtil_IsDominateScreen_0200
 * @tc.desc: IsDominateScreen
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(AbilityPermissionUtilSecondTest, AbilityPermissionUtil_IsDominateScreen_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityPermissionUtil_IsDominateScreen_0200 start");
    Want want;
    auto result = AbilityPermissionUtil::GetInstance().IsDominateScreen(want, false);
    EXPECT_TRUE(result);
    TAG_LOGI(AAFwkTag::TEST, "AbilityPermissionUtil_IsDominateScreen_0200 end");
}

/**
 * @tc.name: AbilityPermissionUtil_IsDominateScreen_0300
 * @tc.desc: IsDominateScreen
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(AbilityPermissionUtilSecondTest, AbilityPermissionUtil_IsDominateScreen_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityPermissionUtil_IsDominateScreen_0300 start");
    Want want;
    auto result = AbilityPermissionUtil::GetInstance().IsDominateScreen(want, false);
    EXPECT_TRUE(result);
    TAG_LOGI(AAFwkTag::TEST, "AbilityPermissionUtil_IsDominateScreen_0300 end");
}

/**
 * @tc.name: AbilityPermissionUtil_IsDominateScreen_0400
 * @tc.desc: IsDominateScreen
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(AbilityPermissionUtilSecondTest, AbilityPermissionUtil_IsDominateScreen_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityPermissionUtil_IsDominateScreen_0400 start");
    Want want;
    auto result = AbilityPermissionUtil::GetInstance().IsDominateScreen(want, false);
    EXPECT_TRUE(result);
    TAG_LOGI(AAFwkTag::TEST, "AbilityPermissionUtil_IsDominateScreen_0400 end");
}

/**
 * @tc.name: AbilityPermissionUtil_CheckMultiInstance_0100
 * @tc.desc: CheckMultiInstance
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(AbilityPermissionUtilSecondTest, AbilityPermissionUtil_CheckMultiInstance_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityPermissionUtil_CheckMultiInstance_0100 start");
    Want want;
    MyFlag::mockFlag_ = 0;
    auto result = AbilityPermissionUtil::GetInstance().CheckMultiInstance(want, nullptr, 0, false);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AbilityPermissionUtil_CheckMultiInstance_0100 end");
}

/**
 * @tc.name: AbilityPermissionUtil_CheckMultiInstance_0200
 * @tc.desc: CheckMultiInstance
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(AbilityPermissionUtilSecondTest, AbilityPermissionUtil_CheckMultiInstance_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityPermissionUtil_CheckMultiInstance_0200 start");
    Want want;
    MyFlag::mockFlag_ = 1;
    auto result = AbilityPermissionUtil::GetInstance().CheckMultiInstance(want, nullptr, 0, false);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AbilityPermissionUtil_CheckMultiInstance_0200 end");
}

/**
 * @tc.name: AbilityPermissionUtil_CheckMultiInstanceAndAppClone0100
 * @tc.desc: CheckMultiInstanceAndAppClone
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(AbilityPermissionUtilSecondTest, AbilityPermissionUtil_CheckMultiInstanceAndAppClone0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityPermissionUtil_CheckMultiInstanceAndAppClone0100 start");
    Want want;
    StartAbilityUtils::startAbilityInfo = std::make_shared<StartAbilityInfo>();
    StartAbilityUtils::startAbilityInfo->abilityInfo.bundleName = "bundleName";
    StartAbilityUtils::startAbilityInfo->abilityInfo.name = "name";
    StartAbilityUtils::startAbilityInfo->abilityInfo.applicationInfo.bundleType =
        static_cast<AppExecFwk::BundleType>(AppExecFwk::BundleType::APP);
    StartAbilityUtils::startAbilityInfo->abilityInfo.type = AppExecFwk::AbilityType::PAGE;
    StartAbilityUtils::startAbilityInfo->abilityInfo.extensionAbilityType = ExtensionAbilityType::SERVICE;
    StartAbilityUtils::startAbilityInfo->abilityInfo.applicationInfo.multiAppMode.multiAppModeType =
        AppExecFwk::MultiAppModeType::UNSPECIFIED;
    int32_t result = AbilityPermissionUtil::GetInstance().CheckMultiInstanceAndAppClone(want, 1, 1, nullptr, false);
    EXPECT_EQ(result, ERR_MULTI_APP_NOT_SUPPORTED);
    TAG_LOGI(AAFwkTag::TEST, "AbilityPermissionUtil_CheckMultiInstanceAndAppClone0100 end");
}

/**
 * @tc.name: AbilityPermissionUtil_CheckMultiInstanceAndAppClone0200
 * @tc.desc: CheckMultiInstanceAndAppClone
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(AbilityPermissionUtilSecondTest, AbilityPermissionUtil_CheckMultiInstanceAndAppClone0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityPermissionUtil_CheckMultiInstanceAndAppClone0200 start");
    Want want;
    StartAbilityUtils::startAbilityInfo = std::make_shared<StartAbilityInfo>();
    StartAbilityUtils::startAbilityInfo->abilityInfo.bundleName = "bundleName";
    StartAbilityUtils::startAbilityInfo->abilityInfo.name = "name";
    StartAbilityUtils::startAbilityInfo->abilityInfo.applicationInfo.bundleType =
        static_cast<AppExecFwk::BundleType>(AppExecFwk::BundleType::APP);
    StartAbilityUtils::startAbilityInfo->abilityInfo.type = AppExecFwk::AbilityType::PAGE;
    StartAbilityUtils::startAbilityInfo->abilityInfo.extensionAbilityType = ExtensionAbilityType::SERVICE;
    StartAbilityUtils::startAbilityInfo->abilityInfo.applicationInfo.multiAppMode.multiAppModeType =
       AppExecFwk::MultiAppModeType::MULTI_INSTANCE;
    int32_t result = AbilityPermissionUtil::GetInstance().CheckMultiInstanceAndAppClone(want, 1, 1, nullptr, false);
    EXPECT_EQ(result, ERR_NOT_SUPPORT_APP_CLONE);
    TAG_LOGI(AAFwkTag::TEST, "AbilityPermissionUtil_CheckMultiInstanceAndAppClone0200 end");
}

/**
 * @tc.name: AbilityPermissionUtil_CheckMultiInstanceAndAppClone0300
 * @tc.desc: CheckMultiInstanceAndAppClone
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(AbilityPermissionUtilSecondTest, AbilityPermissionUtil_CheckMultiInstanceAndAppClone0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityPermissionUtil_CheckMultiInstanceAndAppClone0300 start");
    Want want;
    StartAbilityUtils::startAbilityInfo = std::make_shared<StartAbilityInfo>();
    StartAbilityUtils::startAbilityInfo->abilityInfo.bundleName = "bundleName";
    StartAbilityUtils::startAbilityInfo->abilityInfo.name = "name";
    StartAbilityUtils::startAbilityInfo->abilityInfo.applicationInfo.bundleType =
        static_cast<AppExecFwk::BundleType>(AppExecFwk::BundleType::APP);
    StartAbilityUtils::startAbilityInfo->abilityInfo.type = AppExecFwk::AbilityType::PAGE;
    StartAbilityUtils::startAbilityInfo->abilityInfo.extensionAbilityType = ExtensionAbilityType::SERVICE;
    StartAbilityUtils::startAbilityInfo->abilityInfo.applicationInfo.multiAppMode.multiAppModeType =
       AppExecFwk::MultiAppModeType::MULTI_INSTANCE;
    int32_t result = AbilityPermissionUtil::GetInstance().CheckMultiInstanceAndAppClone(want, 1, 0, nullptr, false);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AbilityPermissionUtil_CheckMultiInstanceAndAppClone0300 end");
}

/**
 * @tc.name: AbilityPermissionUtil_CheckPrepareTerminateEnable0100
 * @tc.desc: CheckPrepareTerminateEnable
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(AbilityPermissionUtilSecondTest, AbilityPermissionUtil_CheckPrepareTerminateEnable0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityPermissionUtil_CheckPrepareTerminateEnable0100 start");
    std::shared_ptr<AbilityRecord> abilityRecord{ nullptr };
    int32_t result = AbilityPermissionUtil::GetInstance().CheckPrepareTerminateEnable(abilityRecord);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AbilityPermissionUtil_CheckPrepareTerminateEnable0100 end");
}
}  // namespace AAFwk
}  // namespace OHOS