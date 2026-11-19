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

#include "ability_manager_errors.h"
#include "ability_record.h"
#include "app_scheduler.h"
#include "hilog_tag_wrapper.h"
#include "start_ability_utils.h"
#include "startup_util.h"
#include "utils/oe_extension_utils.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AAFwk {
namespace {
constexpr int32_t TEST_USER_ID = 100;
constexpr int32_t TEST_HOST_PID = 12345;
constexpr int32_t OESA_UID = 7061;
constexpr int32_t INVALID_UID = 9999;
const std::string TEST_BUNDLE_NAME = "com.example.test";
const std::string TEST_ABILITY_NAME = "TestAbility";
const std::string TEST_SPECIFIED_FLAG = "specified_flag";
}

class OEExtensionUtilsTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

protected:
    std::shared_ptr<AbilityRecord> CreateOEExtensionAbilityRecord();
    Want CreateValidOEExtWant();
};

void OEExtensionUtilsTest::SetUpTestCase() {}

void OEExtensionUtilsTest::TearDownTestCase() {}

void OEExtensionUtilsTest::SetUp() {}

void OEExtensionUtilsTest::TearDown() {}

std::shared_ptr<AbilityRecord> OEExtensionUtilsTest::CreateOEExtensionAbilityRecord()
{
    AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.bundleName = TEST_BUNDLE_NAME;
    abilityInfo.name = "OEExtension";
    abilityInfo.type = AbilityType::EXTENSION;
    abilityInfo.extensionAbilityType = ExtensionAbilityType::CONTENT_EMBED;
    return AbilityRecord::CreateAbilityRecord(abilityInfo, TEST_USER_ID);
}

Want OEExtensionUtilsTest::CreateValidOEExtWant()
{
    Want want;
    want.SetElementName(TEST_BUNDLE_NAME, TEST_ABILITY_NAME);
    return want;
}

/*
 * Feature: OEExtensionUtils
 * Function: ValidateCaller
 * SubFunction: NA
 * FunctionPoints: Validate caller with null token
 */
HWTEST_F(OEExtensionUtilsTest, ValidateCaller_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OEExtensionUtilsTest ValidateCaller_001 start");

    Want want = CreateValidOEExtWant();
    std::string hostBundleName;
    int32_t userId = -1;

    int32_t result = OEExtensionUtils::GetInstance().ValidateCaller(
        OESA_UID, want, nullptr, TEST_HOST_PID, hostBundleName, userId);

    EXPECT_EQ(result, ERR_INVALID_CALLER);
    EXPECT_TRUE(hostBundleName.empty());
    EXPECT_EQ(userId, -1);

    TAG_LOGI(AAFwkTag::TEST, "OEExtensionUtilsTest ValidateCaller_001 end");
}

/*
 * Feature: OEExtensionUtils
 * Function: ValidateCaller
 * SubFunction: NA
 * FunctionPoints: Validate caller with PAGE type ability
 */
HWTEST_F(OEExtensionUtilsTest, ValidateCaller_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OEExtensionUtilsTest ValidateCaller_002 start");

    AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.bundleName = TEST_BUNDLE_NAME;
    abilityInfo.name = "MainAbility";
    abilityInfo.type = AbilityType::PAGE;
    abilityInfo.extensionAbilityType = ExtensionAbilityType::UNSPECIFIED;
    auto abilityRecord = std::make_shared<AbilityRecord>(abilityInfo, TEST_USER_ID);

    Want want = CreateValidOEExtWant();
    std::string hostBundleName;
    int32_t userId = -1;

    int32_t result = OEExtensionUtils::GetInstance().ValidateCaller(
        OESA_UID, want, abilityRecord->GetToken(), TEST_HOST_PID, hostBundleName, userId);

    EXPECT_EQ(result, ERR_INVALID_CALLER);

    TAG_LOGI(AAFwkTag::TEST, "OEExtensionUtilsTest ValidateCaller_002 end");
}

/*
 * Feature: OEExtensionUtils
 * Function: ValidateCaller
 * SubFunction: NA
 * FunctionPoints: Validate caller with non-CONTENT_EMBED extension
 */
HWTEST_F(OEExtensionUtilsTest, ValidateCaller_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OEExtensionUtilsTest ValidateCaller_003 start");

    AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.bundleName = TEST_BUNDLE_NAME;
    abilityInfo.name = "DataExtension";
    abilityInfo.type = AbilityType::EXTENSION;
    abilityInfo.extensionAbilityType = ExtensionAbilityType::FORM;
    auto abilityRecord = std::make_shared<AbilityRecord>(abilityInfo, TEST_USER_ID);

    Want want = CreateValidOEExtWant();
    std::string hostBundleName;
    int32_t userId = -1;

    int32_t result = OEExtensionUtils::GetInstance().ValidateCaller(
        OESA_UID, want, abilityRecord->GetToken(), TEST_HOST_PID, hostBundleName, userId);

    EXPECT_EQ(result, ERR_INVALID_CALLER);

    TAG_LOGI(AAFwkTag::TEST, "OEExtensionUtilsTest ValidateCaller_003 end");
}

/*
 * Feature: OEExtensionUtils
 * Function: ValidateCaller
 * SubFunction: NA
 * FunctionPoints: Validate caller with bundle name mismatch
 */
HWTEST_F(OEExtensionUtilsTest, ValidateCaller_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OEExtensionUtilsTest ValidateCaller_004 start");

    auto abilityRecord = CreateOEExtensionAbilityRecord();

    Want want;
    want.SetElementName("com.other.bundle", TEST_ABILITY_NAME);

    std::string hostBundleName;
    int32_t userId = -1;

    int32_t result = OEExtensionUtils::GetInstance().ValidateCaller(
        OESA_UID, want, abilityRecord->GetToken(), TEST_HOST_PID, hostBundleName, userId);

    EXPECT_EQ(result, INVALID_PARAMETERS_ERR);

    TAG_LOGI(AAFwkTag::TEST, "OEExtensionUtilsTest ValidateCaller_004 end");
}

/*
 * Feature: OEExtensionUtils
 * Function: ValidateCaller
 * SubFunction: NA
 * FunctionPoints: Validate caller with empty abilityName
 */
HWTEST_F(OEExtensionUtilsTest, ValidateCaller_005, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OEExtensionUtilsTest ValidateCaller_005 start");

    auto abilityRecord = CreateOEExtensionAbilityRecord();

    Want want;
    want.SetElementName(TEST_BUNDLE_NAME, "");

    std::string hostBundleName;
    int32_t userId = -1;

    int32_t result = OEExtensionUtils::GetInstance().ValidateCaller(
        OESA_UID, want, abilityRecord->GetToken(), TEST_HOST_PID, hostBundleName, userId);

    EXPECT_EQ(result, INVALID_PARAMETERS_ERR);

    TAG_LOGI(AAFwkTag::TEST, "OEExtensionUtilsTest ValidateCaller_005 end");
}

/*
 * Feature: OEExtensionUtils
 * Function: ValidateCaller
 * SubFunction: NA
 * FunctionPoints: Validate caller with host not in foreground
 */
HWTEST_F(OEExtensionUtilsTest, ValidateCaller_006, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OEExtensionUtilsTest ValidateCaller_006 start");

    auto abilityRecord = CreateOEExtensionAbilityRecord();
    Want want = CreateValidOEExtWant();
    std::string hostBundleName;
    int32_t userId = -1;

    DelayedSingleton<AppScheduler>::GetInstance()->state_ = AppExecFwk::AppProcessState::NONE;
    int32_t result = OEExtensionUtils::GetInstance().ValidateCaller(
        OESA_UID, want, abilityRecord->GetToken(), TEST_HOST_PID, hostBundleName, userId);

    EXPECT_EQ(result, NOT_TOP_ABILITY);

    TAG_LOGI(AAFwkTag::TEST, "OEExtensionUtilsTest ValidateCaller_006 end");
}

/*
 * Feature: OEExtensionUtils
 * Function: ValidateCaller
 * SubFunction: NA
 * FunctionPoints: Validate caller with valid CONTENT_EMBED extension
 */
HWTEST_F(OEExtensionUtilsTest, ValidateCaller_007, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OEExtensionUtilsTest ValidateCaller_007 start");

    auto abilityRecord = CreateOEExtensionAbilityRecord();
    Want want = CreateValidOEExtWant();
    std::string hostBundleName;
    int32_t userId = -1;

    DelayedSingleton<AppScheduler>::GetInstance()->state_ = AppExecFwk::AppProcessState::APP_STATE_FOREGROUND;
    DelayedSingleton<AppScheduler>::GetInstance()->bundleNames_.push_back(TEST_BUNDLE_NAME);
    int32_t result = OEExtensionUtils::GetInstance().ValidateCaller(
        OESA_UID, want, abilityRecord->GetToken(), TEST_HOST_PID, hostBundleName, userId);

    EXPECT_EQ(result, ERR_OK);
    EXPECT_FALSE(hostBundleName.empty());

    TAG_LOGI(AAFwkTag::TEST, "OEExtensionUtilsTest ValidateCaller_007 end");
}

/*
 * Feature: OEExtensionUtils
 * Function: ValidateCaller
 * SubFunction: NA
 * FunctionPoints: Validate caller with invalid UID
 */
HWTEST_F(OEExtensionUtilsTest, ValidateCaller_008, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OEExtensionUtilsTest ValidateCaller_008 start");

    auto abilityRecord = CreateOEExtensionAbilityRecord();
    Want want = CreateValidOEExtWant();
    std::string hostBundleName;
    int32_t userId = -1;

    int32_t result = OEExtensionUtils::GetInstance().ValidateCaller(
        INVALID_UID, want, abilityRecord->GetToken(), TEST_HOST_PID, hostBundleName, userId);

    EXPECT_EQ(result, CHECK_PERMISSION_FAILED);

    TAG_LOGI(AAFwkTag::TEST, "OEExtensionUtilsTest ValidateCaller_008 end");
}

/*
 * Feature: OEExtensionUtils
 * Function: ValidateCaller
 * SubFunction: NA
 * FunctionPoints: Validate caller with invalid UID
 */
HWTEST_F(OEExtensionUtilsTest, ValidateCaller_009, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OEExtensionUtilsTest ValidateCaller_009 start");

    auto abilityRecord = CreateOEExtensionAbilityRecord();
    Want want = CreateValidOEExtWant();
    std::string hostBundleName;
    int32_t userId = -1;

    AAFwk::StartAbilityUtils::mockStatus = false;
    AAFwk::StartAbilityUtils::mockAppIndex = 1;
    int32_t result = OEExtensionUtils::GetInstance().ValidateCaller(
        OESA_UID, want, abilityRecord->GetToken(), TEST_HOST_PID, hostBundleName, userId);
    EXPECT_EQ(result, ERR_APP_CLONE_INDEX_INVALID);

    AAFwk::StartAbilityUtils::mockStatus = true;
    AAFwk::StartAbilityUtils::mockAppIndex = 1;
    result = OEExtensionUtils::GetInstance().ValidateCaller(
        OESA_UID, want, abilityRecord->GetToken(), TEST_HOST_PID, hostBundleName, userId);
    EXPECT_EQ(result, ERR_APP_CLONE_INDEX_INVALID);

    AAFwk::StartAbilityUtils::mockStatus = false;
    AAFwk::StartAbilityUtils::mockAppIndex = 0;
    result = OEExtensionUtils::GetInstance().ValidateCaller(
        OESA_UID, want, abilityRecord->GetToken(), TEST_HOST_PID, hostBundleName, userId);
    EXPECT_EQ(result, ERR_APP_CLONE_INDEX_INVALID);

    AAFwk::StartAbilityUtils::mockStatus = true;
    AAFwk::StartAbilityUtils::mockAppIndex = 0;
    result = OEExtensionUtils::GetInstance().ValidateCaller(
        OESA_UID, want, abilityRecord->GetToken(), TEST_HOST_PID, hostBundleName, userId);
    EXPECT_EQ(result, ERR_OK);

    AAFwk::StartAbilityUtils::mockStatus = true;
    AAFwk::StartAbilityUtils::mockAppIndex = 1;
    abilityRecord->appIndex_ = 1;
    result = OEExtensionUtils::GetInstance().ValidateCaller(
        OESA_UID, want, abilityRecord->GetToken(), TEST_HOST_PID, hostBundleName, userId);
    EXPECT_EQ(result, ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "OEExtensionUtilsTest ValidateCaller_009 end");
}

/*
 * Feature: OEExtensionUtils
 * Function: AddOEExtRequest
 * SubFunction: NA
 * FunctionPoints: Add single request ID to tracking set
 */
HWTEST_F(OEExtensionUtilsTest, AddOEExtRequest_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OEExtensionUtilsTest AddOEExtRequest_001 start");

    int32_t requestId = 1001;
    OEExtensionUtils::GetInstance().AddOEExtRequest(requestId);

    bool removed = OEExtensionUtils::GetInstance().RemoveOEExtRequest(requestId);
    EXPECT_TRUE(removed);

    TAG_LOGI(AAFwkTag::TEST, "OEExtensionUtilsTest AddOEExtRequest_001 end");
}

/*
 * Feature: OEExtensionUtils
 * Function: AddOEExtRequest
 * SubFunction: NA
 * FunctionPoints: Add duplicate request ID
 */
HWTEST_F(OEExtensionUtilsTest, AddOEExtRequest_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OEExtensionUtilsTest AddOEExtRequest_002 start");

    int32_t requestId = 1002;
    OEExtensionUtils::GetInstance().AddOEExtRequest(requestId);
    OEExtensionUtils::GetInstance().AddOEExtRequest(requestId);

    bool removed = OEExtensionUtils::GetInstance().RemoveOEExtRequest(requestId);
    EXPECT_TRUE(removed);

    removed = OEExtensionUtils::GetInstance().RemoveOEExtRequest(requestId);
    EXPECT_FALSE(removed);

    TAG_LOGI(AAFwkTag::TEST, "OEExtensionUtilsTest AddOEExtRequest_002 end");
}

/*
 * Feature: OEExtensionUtils
 * Function: RemoveOEExtRequest
 * SubFunction: NA
 * FunctionPoints: Remove non-existent request ID
 */
HWTEST_F(OEExtensionUtilsTest, RemoveOEExtRequest_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OEExtensionUtilsTest RemoveOEExtRequest_002 start");

    int32_t requestId = 1004;
    bool removed = OEExtensionUtils::GetInstance().RemoveOEExtRequest(requestId);
    EXPECT_FALSE(removed);

    TAG_LOGI(AAFwkTag::TEST, "OEExtensionUtilsTest RemoveOEExtRequest_002 end");
}

/*
 * Feature: OEExtensionUtils
 * Function: GetInstance
 */
HWTEST_F(OEExtensionUtilsTest, GetInstance_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OEExtensionUtilsTest GetInstance_001 start");

    OEExtensionUtils &instance = OEExtensionUtils::GetInstance();
    EXPECT_EQ(&instance, &OEExtensionUtils::GetInstance());

    OEExtensionUtils &instance2 = OEExtensionUtils::GetInstance();
    EXPECT_EQ(&instance, &instance2);
    TAG_LOGI(AAFwkTag::TEST, "OEExtensionUtilsTest GetInstance_001 end");
}
}  // namespace AAFwk
}  // namespace OHOS
