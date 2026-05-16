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

#include "ability_manager_errors.h"
#include "mock_my_status.h"
#include "hilog_tag_wrapper.h"
#include "preload_manager_service.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AAFwk {
class PreloadManagerServiceTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void PreloadManagerServiceTest::SetUpTestCase() {}

void PreloadManagerServiceTest::TearDownTestCase() {}

void PreloadManagerServiceTest::SetUp() {}

void PreloadManagerServiceTest::TearDown() {}

/*
 * Feature: PreloadManagerService
 * Name: PreloadApplication_001
 * Function: PreloadApplication
 * SubFunction: NA
 * FunctionPoints: PreloadManagerService PreloadApplication
 */
HWTEST_F(PreloadManagerServiceTest, PreloadApplication_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "PreloadManagerServiceTest PreloadApplication_001 start");
    
    MyStatus::GetInstance().isPreloadApplicationEnabled_ = false;
    std::string bundleName = "bundleName";
    int32_t userId = -1;
    int32_t appIndex = 0;
    auto result = PreloadManagerService::GetInstance().PreloadApplication(bundleName, userId, appIndex);
    EXPECT_EQ(result, ERR_CAPABILITY_NOT_SUPPORT);

    MyStatus::GetInstance().isPreloadApplicationEnabled_ = true;
    appIndex = -1;
    result = PreloadManagerService::GetInstance().PreloadApplication(bundleName, userId, appIndex);
    EXPECT_EQ(result, ERR_CAPABILITY_NOT_SUPPORT);

    TAG_LOGI(AAFwkTag::TEST, "PreloadManagerServiceTest PreloadApplication_001 end");
}

/*
 * Feature: PreloadManagerService
 * Name: PreloadApplication_002
 * Function: PreloadApplication
 * SubFunction: NA
 * FunctionPoints: PreloadManagerService PreloadApplication
 */
HWTEST_F(PreloadManagerServiceTest, PreloadApplication_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "PreloadManagerServiceTest PreloadApplication_002 start");
    
    MyStatus::GetInstance().isPreloadApplicationEnabled_ = true;
    MyStatus::GetInstance().retVerifyPreloadApplicationPermission_ = false;
    std::string bundleName = "bundleName";
    int32_t userId = -1;
    int32_t appIndex = 0;
    auto result = PreloadManagerService::GetInstance().PreloadApplication(bundleName, userId, appIndex);
    EXPECT_EQ(result, ERR_PERMISSION_DENIED);

    TAG_LOGI(AAFwkTag::TEST, "PreloadManagerServiceTest PreloadApplication_002 end");
}

/*
 * Feature: PreloadManagerService
 * Name: PreloadApplication_003
 * Function: PreloadApplication
 * SubFunction: NA
 * FunctionPoints: PreloadManagerService PreloadApplication
 */
HWTEST_F(PreloadManagerServiceTest, PreloadApplication_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "PreloadManagerServiceTest PreloadApplication_003 start");
    
    MyStatus::GetInstance().isPreloadApplicationEnabled_ = true;
    MyStatus::GetInstance().retVerifyPreloadApplicationPermission_ = true;
    MyStatus::GetInstance().isMultiUserConcurrency_ = false;
    std::string bundleName = "bundleName";
    int32_t userId = 102;
    int32_t appIndex = 0;
    auto result = PreloadManagerService::GetInstance().PreloadApplication(bundleName, userId, appIndex);
    EXPECT_EQ(result, ERR_CROSS_USER);

    TAG_LOGI(AAFwkTag::TEST, "PreloadManagerServiceTest PreloadApplication_003 end");
}

/*
 * Feature: PreloadManagerService
 * Name: PreloadApplication_004
 * Function: PreloadApplication
 * SubFunction: NA
 * FunctionPoints: PreloadManagerService PreloadApplication
 */
HWTEST_F(PreloadManagerServiceTest, PreloadApplication_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "PreloadManagerServiceTest PreloadApplication_004 start");
    
    MyStatus::GetInstance().isPreloadApplicationEnabled_ = true;
    MyStatus::GetInstance().retVerifyPreloadApplicationPermission_ = true;
    MyStatus::GetInstance().isMultiUserConcurrency_ = true;
    MyStatus::GetInstance().retCheckPreloadAppRecordExist_ = -1;
    std::string bundleName = "bundleName";
    int32_t userId = -1;
    int32_t appIndex = 0;
    auto result = PreloadManagerService::GetInstance().PreloadApplication(bundleName, userId, appIndex);
    EXPECT_EQ(result, -1);

    MyStatus::GetInstance().retCheckPreloadAppRecordExist_ = ERR_OK;
    MyStatus::GetInstance().isPreloadApplicationRecordExist_ = true;
    result = PreloadManagerService::GetInstance().PreloadApplication(bundleName, userId, appIndex);
    EXPECT_EQ(result, ERR_PRELOAD_APP_RECORD_ALREADY_EXIST);

    TAG_LOGI(AAFwkTag::TEST, "PreloadManagerServiceTest PreloadApplication_004 end");
}

/*
 * Feature: PreloadManagerService
 * Name: PreloadApplication_005
 * Function: PreloadApplication
 * SubFunction: NA
 * FunctionPoints: PreloadManagerService PreloadApplication
 */
HWTEST_F(PreloadManagerServiceTest, PreloadApplication_005, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "PreloadManagerServiceTest PreloadApplication_005 start");
    
    MyStatus::GetInstance().isPreloadApplicationEnabled_ = true;
    MyStatus::GetInstance().retVerifyPreloadApplicationPermission_ = true;
    MyStatus::GetInstance().isMultiUserConcurrency_ = true;
    MyStatus::GetInstance().retCheckPreloadAppRecordExist_ = ERR_OK;
    MyStatus::GetInstance().isPreloadApplicationRecordExist_ = false;
    MyStatus::GetInstance().bundleMgrHelper_ = nullptr;
    std::string bundleName = "bundleName";
    int32_t userId = -1;
    int32_t appIndex = 0;
    auto result = PreloadManagerService::GetInstance().PreloadApplication(bundleName, userId, appIndex);
    EXPECT_EQ(result, INNER_ERR);

    TAG_LOGI(AAFwkTag::TEST, "PreloadManagerServiceTest PreloadApplication_005 end");
}

/*
 * Feature: PreloadManagerService
 * Name: PreloadApplication_006
 * Function: PreloadApplication
 * SubFunction: NA
 * FunctionPoints: PreloadManagerService PreloadApplication
 */
HWTEST_F(PreloadManagerServiceTest, PreloadApplication_006, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "PreloadManagerServiceTest PreloadApplication_006 start");
    
    MyStatus::GetInstance().isPreloadApplicationEnabled_ = true;
    MyStatus::GetInstance().retVerifyPreloadApplicationPermission_ = true;
    MyStatus::GetInstance().isMultiUserConcurrency_ = true;
    MyStatus::GetInstance().retCheckPreloadAppRecordExist_ = ERR_OK;
    MyStatus::GetInstance().isPreloadApplicationRecordExist_ = false;
    MyStatus::GetInstance().bundleMgrHelper_ = DelayedSingleton<BundleMgrHelper>::GetInstance();
    MyStatus::GetInstance().retGetLaunchWantForBundle_ = -1;
    std::string bundleName = "bundleName";
    int32_t userId = -1;
    int32_t appIndex = 0;
    auto result = PreloadManagerService::GetInstance().PreloadApplication(bundleName, userId, appIndex);
    EXPECT_EQ(result, -1);

    TAG_LOGI(AAFwkTag::TEST, "PreloadManagerServiceTest PreloadApplication_006 end");
}

/*
 * Feature: PreloadManagerService
 * Name: PreloadApplication_007
 * Function: PreloadApplication
 * SubFunction: NA
 * FunctionPoints: PreloadManagerService PreloadApplication
 */
HWTEST_F(PreloadManagerServiceTest, PreloadApplication_007, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "PreloadManagerServiceTest PreloadApplication_007 start");
    
    MyStatus::GetInstance().isPreloadApplicationEnabled_ = true;
    MyStatus::GetInstance().retVerifyPreloadApplicationPermission_ = true;
    MyStatus::GetInstance().isMultiUserConcurrency_ = true;
    MyStatus::GetInstance().retCheckPreloadAppRecordExist_ = ERR_OK;
    MyStatus::GetInstance().isPreloadApplicationRecordExist_ = false;
    MyStatus::GetInstance().bundleMgrHelper_ = DelayedSingleton<BundleMgrHelper>::GetInstance();
    MyStatus::GetInstance().retGetLaunchWantForBundle_ = 0;
    MyStatus::GetInstance().retQueryAbilityInfo_ = false;
    std::string bundleName = "bundleName";
    int32_t userId = -1;
    int32_t appIndex = 0;
    auto result = PreloadManagerService::GetInstance().PreloadApplication(bundleName, userId, appIndex);
    EXPECT_EQ(result, RESOLVE_ABILITY_ERR);

    TAG_LOGI(AAFwkTag::TEST, "PreloadManagerServiceTest PreloadApplication_007 end");
}

/*
 * Feature: PreloadManagerService
 * Name: PreloadApplication_008
 * Function: PreloadApplication
 * SubFunction: NA
 * FunctionPoints: PreloadManagerService PreloadApplication
 */
HWTEST_F(PreloadManagerServiceTest, PreloadApplication_008, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "PreloadManagerServiceTest PreloadApplication_008 start");
    
    MyStatus::GetInstance().isPreloadApplicationEnabled_ = true;
    MyStatus::GetInstance().retVerifyPreloadApplicationPermission_ = true;
    MyStatus::GetInstance().isMultiUserConcurrency_ = true;
    MyStatus::GetInstance().retCheckPreloadAppRecordExist_ = ERR_OK;
    MyStatus::GetInstance().isPreloadApplicationRecordExist_ = false;
    MyStatus::GetInstance().bundleMgrHelper_ = DelayedSingleton<BundleMgrHelper>::GetInstance();
    MyStatus::GetInstance().retGetLaunchWantForBundle_ = 0;
    MyStatus::GetInstance().retQueryAbilityInfo_ = true;
    MyStatus::GetInstance().queryAbilityInfo_.applicationInfo.appPreloadPhase =
        AppExecFwk::AppPreloadPhase::ABILITY_STAGE_CREATED;
    MyStatus::GetInstance().retPreloadApplicationByPhase_ = 100;
    std::string bundleName = "bundleName";
    int32_t userId = -1;
    int32_t appIndex = 0;
    auto result = PreloadManagerService::GetInstance().PreloadApplication(bundleName, userId, appIndex);
    EXPECT_EQ(result, 100);

    TAG_LOGI(AAFwkTag::TEST, "PreloadManagerServiceTest PreloadApplication_008 end");
}

/*
 * Feature: PreloadManagerService
 * Name: PreloadApplication_009
 * Function: PreloadApplication
 * SubFunction: NA
 * FunctionPoints: PreloadManagerService PreloadApplication
 */
HWTEST_F(PreloadManagerServiceTest, PreloadApplication_009, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "PreloadManagerServiceTest PreloadApplication_009 start");
    
    MyStatus::GetInstance().isPreloadApplicationEnabled_ = true;
    MyStatus::GetInstance().retVerifyPreloadApplicationPermission_ = true;
    MyStatus::GetInstance().isMultiUserConcurrency_ = true;
    MyStatus::GetInstance().retCheckPreloadAppRecordExist_ = ERR_OK;
    MyStatus::GetInstance().isPreloadApplicationRecordExist_ = false;
    MyStatus::GetInstance().bundleMgrHelper_ = DelayedSingleton<BundleMgrHelper>::GetInstance();
    MyStatus::GetInstance().retGetLaunchWantForBundle_ = 0;
    MyStatus::GetInstance().retQueryAbilityInfo_ = true;
    MyStatus::GetInstance().queryAbilityInfo_.applicationInfo.appPreloadPhase =
        AppExecFwk::AppPreloadPhase::WINDOW_STAGE_CREATED;
    MyStatus::GetInstance().retStartAbility_ = 100;
    std::string bundleName = "bundleName";
    int32_t userId = -1;
    int32_t appIndex = 0;
    auto result = PreloadManagerService::GetInstance().PreloadApplication(bundleName, userId, appIndex);
    EXPECT_EQ(result, 100);

    TAG_LOGI(AAFwkTag::TEST, "PreloadManagerServiceTest PreloadApplication_009 end");
}

/*
 * Feature: PreloadManagerService
 * Name: PreloadApplication_010
 * Function: PreloadApplication
 * SubFunction: NA
 * FunctionPoints: PreloadManagerService PreloadApplication
 */
HWTEST_F(PreloadManagerServiceTest, PreloadApplication_010, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "PreloadManagerServiceTest PreloadApplication_010 start");
    
    MyStatus::GetInstance().isPreloadApplicationEnabled_ = true;
    MyStatus::GetInstance().retVerifyPreloadApplicationPermission_ = true;
    MyStatus::GetInstance().isMultiUserConcurrency_ = true;
    MyStatus::GetInstance().retCheckPreloadAppRecordExist_ = ERR_OK;
    MyStatus::GetInstance().isPreloadApplicationRecordExist_ = false;
    MyStatus::GetInstance().bundleMgrHelper_ = DelayedSingleton<BundleMgrHelper>::GetInstance();
    MyStatus::GetInstance().retGetLaunchWantForBundle_ = 0;
    MyStatus::GetInstance().retQueryAbilityInfo_ = true;
    MyStatus::GetInstance().queryAbilityInfo_.applicationInfo.appPreloadPhase =
        AppExecFwk::AppPreloadPhase::DEFAULT;
    std::string bundleName = "bundleName";
    int32_t userId = -1;
    int32_t appIndex = 0;
    auto result = PreloadManagerService::GetInstance().PreloadApplication(bundleName, userId, appIndex);
    EXPECT_EQ(result, ERR_INVALID_APP_PRELOAD_PHASE);

    MyStatus::GetInstance().queryAbilityInfo_.applicationInfo.appPreloadPhase = AppExecFwk::AppPreloadPhase(100);
    result = PreloadManagerService::GetInstance().PreloadApplication(bundleName, userId, appIndex);
    EXPECT_EQ(result, ERR_INVALID_APP_PRELOAD_PHASE);

    TAG_LOGI(AAFwkTag::TEST, "PreloadManagerServiceTest PreloadApplication_010 end");
}

/*
 * Feature: PreloadManagerService
 * Name: PreloadApplication_011
 * Function: PreloadApplication
 * SubFunction: NA
 * FunctionPoints: PreloadManagerService PreloadApplication
 */
HWTEST_F(PreloadManagerServiceTest, PreloadApplication_011, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "PreloadManagerServiceTest PreloadApplication_011 start");
    
    MyStatus::GetInstance().isPreloadApplicationEnabled_ = true;
    MyStatus::GetInstance().retVerifyPreloadApplicationPermission_ = true;
    MyStatus::GetInstance().isMultiUserConcurrency_ = true;
    MyStatus::GetInstance().retCheckPreloadAppRecordExist_ = ERR_OK;
    MyStatus::GetInstance().isPreloadApplicationRecordExist_ = false;
    MyStatus::GetInstance().bundleMgrHelper_ = DelayedSingleton<BundleMgrHelper>::GetInstance();
    MyStatus::GetInstance().retGetLaunchWantForBundle_ = 0;
    MyStatus::GetInstance().retQueryAbilityInfo_ = true;
    MyStatus::GetInstance().queryAbilityInfo_.applicationInfo.appPreloadPhase =
        AppExecFwk::AppPreloadPhase::WINDOW_STAGE_CREATED;
    MyStatus::GetInstance().queryAbilityInfo_.launchMode = AppExecFwk::LaunchMode::STANDARD;
    std::string bundleName = "bundleName";
    int32_t userId = -1;
    int32_t appIndex = 0;
    auto result = PreloadManagerService::GetInstance().PreloadApplication(bundleName, userId, appIndex);
    EXPECT_EQ(result, ERR_CAPABILITY_NOT_SUPPORT);

    TAG_LOGI(AAFwkTag::TEST, "PreloadManagerServiceTest PreloadApplication_011 end");
}

/*
 * Feature: PreloadManagerService
 * Name: LaunchGameCustomized_001
 * Function: LaunchGameCustomized
 * SubFunction: NA
 * FunctionPoints: PreloadManagerService LaunchGameCustomized verification failed
 */
HWTEST_F(PreloadManagerServiceTest, LaunchGameCustomized_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "PreloadManagerServiceTest LaunchGameCustomized_001 start");

    // Set up for verification failure
    MyStatus::GetInstance().isMultiUserConcurrency_ = false;
    std::string bundleName = "com.test.game";
    int32_t userId = 100;
    int32_t appIndex = 0;

    // PreloadApplicationVerification will fail due to isMultiUserConcurrency_ = false
    auto result = PreloadManagerService::GetInstance().LaunchGameCustomized(bundleName, userId, appIndex);
    EXPECT_EQ(result, ERR_CROSS_USER);

    TAG_LOGI(AAFwkTag::TEST, "PreloadManagerServiceTest LaunchGameCustomized_001 end");
}

/*
 * Feature: PreloadManagerService
 * Name: LaunchGameCustomized_002
 * Function: LaunchGameCustomized
 * SubFunction: NA
 * FunctionPoints: PreloadManagerService LaunchGameCustomized record already exist
 */
HWTEST_F(PreloadManagerServiceTest, LaunchGameCustomized_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "PreloadManagerServiceTest LaunchGameCustomized_002 start");

    // Set up for record already exist
    MyStatus::GetInstance().isMultiUserConcurrency_ = true;
    MyStatus::GetInstance().retCheckPreloadAppRecordExist_ = ERR_OK;
    MyStatus::GetInstance().isPreloadApplicationRecordExist_ = true;
    std::string bundleName = "com.test.game";
    int32_t userId = 100;
    int32_t appIndex = 0;

    auto result = PreloadManagerService::GetInstance().LaunchGameCustomized(bundleName, userId, appIndex);
    EXPECT_EQ(result, ERR_PRELOAD_APP_RECORD_ALREADY_EXIST);

    TAG_LOGI(AAFwkTag::TEST, "PreloadManagerServiceTest LaunchGameCustomized_002 end");
}

/*
 * Feature: PreloadManagerService
 * Name: LaunchGameCustomized_003
 * Function: LaunchGameCustomized
 * SubFunction: NA
 * FunctionPoints: PreloadManagerService LaunchGameCustomized bundle mgr helper is null
 */
HWTEST_F(PreloadManagerServiceTest, LaunchGameCustomized_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "PreloadManagerServiceTest LaunchGameCustomized_003 start");

    // Set up for bundle mgr helper null
    MyStatus::GetInstance().isMultiUserConcurrency_ = true;
    MyStatus::GetInstance().retCheckPreloadAppRecordExist_ = ERR_OK;
    MyStatus::GetInstance().isPreloadApplicationRecordExist_ = false;
    MyStatus::GetInstance().bundleMgrHelper_ = nullptr;
    std::string bundleName = "com.test.game";
    int32_t userId = 100;
    int32_t appIndex = 0;

    auto result = PreloadManagerService::GetInstance().LaunchGameCustomized(bundleName, userId, appIndex);
    EXPECT_EQ(result, INNER_ERR);

    TAG_LOGI(AAFwkTag::TEST, "PreloadManagerServiceTest LaunchGameCustomized_003 end");
}

/*
 * Feature: PreloadManagerService
 * Name: LaunchGameCustomized_004
 * Function: LaunchGameCustomized
 * SubFunction: NA
 * FunctionPoints: PreloadManagerService LaunchGameCustomized get launch want failed
 */
HWTEST_F(PreloadManagerServiceTest, LaunchGameCustomized_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "PreloadManagerServiceTest LaunchGameCustomized_004 start");

    // Set up for GetLaunchWantForBundle failure
    MyStatus::GetInstance().isMultiUserConcurrency_ = true;
    MyStatus::GetInstance().retCheckPreloadAppRecordExist_ = ERR_OK;
    MyStatus::GetInstance().isPreloadApplicationRecordExist_ = false;
    MyStatus::GetInstance().bundleMgrHelper_ = DelayedSingleton<BundleMgrHelper>::GetInstance();
    MyStatus::GetInstance().retGetLaunchWantForBundle_ = -1;
    std::string bundleName = "com.test.game";
    int32_t userId = 100;
    int32_t appIndex = 0;

    auto result = PreloadManagerService::GetInstance().LaunchGameCustomized(bundleName, userId, appIndex);
    EXPECT_EQ(result, -1);

    TAG_LOGI(AAFwkTag::TEST, "PreloadManagerServiceTest LaunchGameCustomized_004 end");
}
} // namespace AAFwk
} // namespace OHOS
