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
#define protected public
#include "ability_auto_startup_service.h"
#include "ability_manager_errors.h"
#include "distributed_kv_data_manager.h"
#include "hilog_tag_wrapper.h"
#include "mock_ability_auto_startup_data_manager.h"
#include "mock_ability_manager_service.h"
#include "mock_bundle_mgr_helper.h"
#include "mock_my_flag.h"
#include "mock_parameters.h"
#include "mock_sa_call.h"
#include "nativetoken_kit.h"
#include "parameters.h"
#include "token_setproc.h"
#undef private
#undef protected

namespace {
const bool AUTO_STARTUP_SERVICE_TRUE = true;
const bool AUTO_STARTUP_SERVICE_FALSE = false;
constexpr int32_t BASE_USER_RANGE = 200000;
} // namespace

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AbilityRuntime;
namespace OHOS {
namespace AAFwk {
class AbilityAutoStartupServiceThirdTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void AbilityAutoStartupServiceThirdTest::SetUpTestCase()
{
    auto abilityMs = OHOS::DelayedSingleton<AbilityManagerService>::GetInstance();
    EXPECT_NE(abilityMs, nullptr);
}

void AbilityAutoStartupServiceThirdTest::TearDownTestCase()
{
    OHOS::DelayedSingleton<AbilityManagerService>::DestroyInstance();
}

void AbilityAutoStartupServiceThirdTest::SetUp() {}

void AbilityAutoStartupServiceThirdTest::TearDown() {}

/*
 * Feature: AbilityAutoStartupService
 * Function: InnerSetApplicationAutoStartup
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService InnerSetApplicationAutoStartup
 */
HWTEST_F(AbilityAutoStartupServiceThirdTest, InnerSetApplicationAutoStartup_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceThirdTest InnerSetApplicationAutoStartup_001 start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);
    AutoStartupInfo info;
    info.bundleName = BUNDLENAME_FONUD;
    info.canUserModify = AUTO_STARTUP_SERVICE_FALSE;
    auto result = abilityAutoStartupService->InnerSetApplicationAutoStartup(info);
    EXPECT_EQ(result, ERR_EDM_APP_CONTROLLED);
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceThirdTest InnerSetApplicationAutoStartup_001 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: InnerSetApplicationAutoStartup
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService InnerSetApplicationAutoStartup
 */
HWTEST_F(AbilityAutoStartupServiceThirdTest, InnerSetApplicationAutoStartup_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceThirdTest InnerSetApplicationAutoStartup_002 start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);
    AutoStartupInfo info;
    info.bundleName = BUNDLENAME_NO_FONUD;
    info.userId = -1;
    auto result = abilityAutoStartupService->InnerSetApplicationAutoStartup(info);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceThirdTest InnerSetApplicationAutoStartup_002 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: InnerSetApplicationAutoStartup
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService InnerSetApplicationAutoStartup
 */
HWTEST_F(AbilityAutoStartupServiceThirdTest, InnerSetApplicationAutoStartup_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceThirdTest InnerSetApplicationAutoStartup_003 start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);
    AutoStartupInfo info;
    info.bundleName = BUNDLENAME_FONUD;
    info.abilityName = ABILITYNAME_AUTO_START;
    info.accessTokenId = "123";
    info.setterUserId = 100;
    info.userId = 100;
    info.setterType = AutoStartupSetterType::USER;
    info.canUserModify = AUTO_STARTUP_SERVICE_TRUE;
    auto result = abilityAutoStartupService->InnerSetApplicationAutoStartup(info);
    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceThirdTest InnerSetApplicationAutoStartup_003 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: InnerSetApplicationAutoStartup
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService InnerSetApplicationAutoStartup
 */
HWTEST_F(AbilityAutoStartupServiceThirdTest, InnerSetApplicationAutoStartup_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceThirdTest InnerSetApplicationAutoStartup_004 start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);
    AutoStartupInfo info;
    info.bundleName = BUNDLENAME_FONUD;
    info.abilityName = ABILITYNAME_NOT_AUTO_START;
    info.canUserModify = AUTO_STARTUP_SERVICE_TRUE;
    auto result = abilityAutoStartupService->InnerSetApplicationAutoStartup(info);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceThirdTest InnerSetApplicationAutoStartup_004 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: InnerSetApplicationAutoStartup
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService InnerSetApplicationAutoStartup
 */
HWTEST_F(AbilityAutoStartupServiceThirdTest, InnerSetApplicationAutoStartup_005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceThirdTest InnerSetApplicationAutoStartup_005 start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);
    AutoStartupInfo info;
    info.bundleName = BUNDLENAME_FONUD;
    info.abilityName = ABILITYNAME_NOT_AUTO_START;
    info.accessTokenId = "123";
    info.setterUserId = 100;
    info.userId = 100;
    info.setterType = AutoStartupSetterType::USER;
    info.canUserModify = AUTO_STARTUP_SERVICE_TRUE;
    auto result = abilityAutoStartupService->InnerSetApplicationAutoStartup(info);
    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceThirdTest InnerSetApplicationAutoStartup_005 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: InnerSetApplicationAutoStartup
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService InnerSetApplicationAutoStartup
 */
HWTEST_F(AbilityAutoStartupServiceThirdTest, InnerSetApplicationAutoStartup_006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceThirdTest InnerSetApplicationAutoStartup_006 start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);
    AutoStartupInfo info;
    info.bundleName = BUNDLENAME_NO_FONUD;
    info.abilityName = ABILITYNAME_NOT_AUTO_START;
    info.accessTokenId = "123";
    info.setterUserId = 100;
    info.userId = 100;
    info.setterType = AutoStartupSetterType::USER;
    auto result = abilityAutoStartupService->InnerSetApplicationAutoStartup(info);
    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceThirdTest InnerSetApplicationAutoStartup_006 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: InnerCancelApplicationAutoStartup
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService InnerCancelApplicationAutoStartup
 */
HWTEST_F(AbilityAutoStartupServiceThirdTest, InnerCancelApplicationAutoStartup_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceThirdTest InnerCancelApplicationAutoStartup_001 start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);
    AutoStartupInfo info;
    info.bundleName = BUNDLENAME_FONUD;
    info.canUserModify = AUTO_STARTUP_SERVICE_FALSE;
    auto result = abilityAutoStartupService->InnerCancelApplicationAutoStartup(info);
    EXPECT_EQ(result, ERR_EDM_APP_CONTROLLED);
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceThirdTest InnerCancelApplicationAutoStartup_001 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: InnerCancelApplicationAutoStartup
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService InnerCancelApplicationAutoStartup
 */
HWTEST_F(AbilityAutoStartupServiceThirdTest, InnerCancelApplicationAutoStartup_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceThirdTest InnerCancelApplicationAutoStartup_002 start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);
    AutoStartupInfo info;
    info.bundleName = BUNDLENAME_NO_FONUD;
    info.userId = -1;
    auto result = abilityAutoStartupService->InnerCancelApplicationAutoStartup(info);
    EXPECT_EQ(result, ERR_NAME_NOT_FOUND);
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceThirdTest InnerCancelApplicationAutoStartup_002 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: InnerCancelApplicationAutoStartup
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService InnerCancelApplicationAutoStartup
 */
HWTEST_F(AbilityAutoStartupServiceThirdTest, InnerCancelApplicationAutoStartup_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceThirdTest InnerCancelApplicationAutoStartup_003 start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);
    AutoStartupInfo info;
    info.bundleName = BUNDLENAME_FONUD;
    info.abilityName = ABILITYNAME_AUTO_START;
    info.accessTokenId = "123";
    info.setterUserId = 0;
    info.userId = 100;
    info.setterType = AutoStartupSetterType::USER;
    info.canUserModify = AUTO_STARTUP_SERVICE_TRUE;
    auto result = abilityAutoStartupService->InnerCancelApplicationAutoStartup(info);
    EXPECT_EQ(result, ERR_INVALID_OPERATION);
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceThirdTest InnerCancelApplicationAutoStartup_003 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: InnerCancelApplicationAutoStartup
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService InnerCancelApplicationAutoStartup
 */
HWTEST_F(AbilityAutoStartupServiceThirdTest, InnerCancelApplicationAutoStartup_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceThirdTest InnerCancelApplicationAutoStartup_004 start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);
    AutoStartupInfo info;
    info.bundleName = BUNDLENAME_FONUD;
    info.abilityName = ABILITYNAME_NOT_AUTO_START;
    info.canUserModify = AUTO_STARTUP_SERVICE_TRUE;
    auto result = abilityAutoStartupService->InnerCancelApplicationAutoStartup(info);
    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceThirdTest InnerCancelApplicationAutoStartup_004 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: InnerCancelApplicationAutoStartup
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService InnerCancelApplicationAutoStartup
 */
HWTEST_F(AbilityAutoStartupServiceThirdTest, InnerCancelApplicationAutoStartup_005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceThirdTest InnerCancelApplicationAutoStartup_005 start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);
    AutoStartupInfo info;
    info.bundleName = BUNDLENAME_FONUD;
    info.abilityName = ABILITYNAME_AUTO_START_BY_EDM;
    info.accessTokenId = "123";
    info.setterUserId = 100;
    info.userId = 100;
    info.setterType = AutoStartupSetterType::SYSTEM;
    info.canUserModify = AUTO_STARTUP_SERVICE_TRUE;
    auto result = abilityAutoStartupService->InnerCancelApplicationAutoStartup(info);
    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceThirdTest InnerCancelApplicationAutoStartup_005 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: InnerCancelApplicationAutoStartup
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService InnerCancelApplicationAutoStartup
 */
HWTEST_F(AbilityAutoStartupServiceThirdTest, InnerCancelApplicationAutoStartup_006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceThirdTest InnerCancelApplicationAutoStartup_006 start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);
    AutoStartupInfo info;
    info.bundleName = BUNDLENAME_FONUD;
    info.abilityName = ABILITYNAME_AUTO_START_BY_EDM;
    info.accessTokenId = "123";
    info.setterUserId = 100;
    info.userId = -1;
    info.setterType = AutoStartupSetterType::SYSTEM;
    info.canUserModify = AUTO_STARTUP_SERVICE_TRUE;
    auto result = abilityAutoStartupService->InnerCancelApplicationAutoStartup(info);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceThirdTest InnerCancelApplicationAutoStartup_006 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: InnerCancelApplicationAutoStartup
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService InnerCancelApplicationAutoStartup
 */
HWTEST_F(AbilityAutoStartupServiceThirdTest, InnerCancelApplicationAutoStartup_007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceThirdTest InnerCancelApplicationAutoStartup_007 start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);
    AutoStartupInfo info;
    info.bundleName = BUNDLENAME_FONUD;
    info.abilityName = ABILITYNAME_AUTO_START;
    info.accessTokenId = "123";
    info.setterUserId = 101;
    info.userId = 101;
    info.setterType = AutoStartupSetterType::USER;
    info.canUserModify = AUTO_STARTUP_SERVICE_TRUE;
    auto result = abilityAutoStartupService->InnerCancelApplicationAutoStartup(info);
    EXPECT_EQ(result, ERR_INVALID_OPERATION);
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceThirdTest InnerCancelApplicationAutoStartup_007 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: InnerApplicationAutoStartupByEDM
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService InnerApplicationAutoStartupByEDM
 */
HWTEST_F(AbilityAutoStartupServiceThirdTest, InnerApplicationAutoStartupByEDM_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceThirdTest InnerApplicationAutoStartupByEDM_001 start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);
    AutoStartupInfo info;
    info.bundleName = BUNDLENAME_FONUD;
    info.abilityName = ABILITYNAME_AUTO_START;
    auto result = abilityAutoStartupService->InnerApplicationAutoStartupByEDM(info, true, true);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceThirdTest InnerApplicationAutoStartupByEDM_001 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: InnerApplicationAutoStartupByEDM
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService InnerApplicationAutoStartupByEDM
 */
HWTEST_F(AbilityAutoStartupServiceThirdTest, InnerApplicationAutoStartupByEDM_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceThirdTest InnerApplicationAutoStartupByEDM_002 start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);
    AutoStartupInfo info;
    info.bundleName = BUNDLENAME_FONUD;
    info.abilityName = ABILITYNAME_AUTO_START;
    info.accessTokenId = "123";
    info.setterUserId = 100;
    info.userId = 100;
    info.setterType = AutoStartupSetterType::USER;
    auto result = abilityAutoStartupService->InnerApplicationAutoStartupByEDM(info, true, true);
    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceThirdTest InnerApplicationAutoStartupByEDM_002 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: InnerApplicationAutoStartupByEDM
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService InnerApplicationAutoStartupByEDM
 */
HWTEST_F(AbilityAutoStartupServiceThirdTest, InnerApplicationAutoStartupByEDM_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceThirdTest InnerApplicationAutoStartupByEDM_003 start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);
    AutoStartupInfo info;
    info.bundleName = BUNDLENAME_FONUD;
    info.abilityName = ABILITYNAME_NOT_AUTO_START;
    info.canUserModify = AUTO_STARTUP_SERVICE_TRUE;
    info.userId = 100;
    auto result = abilityAutoStartupService->InnerApplicationAutoStartupByEDM(info, false, false);
    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceThirdTest InnerApplicationAutoStartupByEDM_003 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: InnerSetApplicationAutoStartup
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService InnerSetApplicationAutoStartup
 */
HWTEST_F(AbilityAutoStartupServiceThirdTest, InnerApplicationAutoStartupByEDM_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceThirdTest InnerApplicationAutoStartupByEDM_004 start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);
    AutoStartupInfo info;
    info.bundleName = BUNDLENAME_NO_FONUD;
    info.userId = -1;
    auto result = abilityAutoStartupService->InnerApplicationAutoStartupByEDM(info, false, false);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceThirdTest InnerApplicationAutoStartupByEDM_004 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: InnerSetApplicationAutoStartup
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService InnerSetApplicationAutoStartup
 */
HWTEST_F(AbilityAutoStartupServiceThirdTest, InnerApplicationAutoStartupByEDM_005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceThirdTest InnerApplicationAutoStartupByEDM_005 start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);
    AutoStartupInfo info;
    info.bundleName = BUNDLENAME_NO_FONUD;
    info.abilityName = ABILITYNAME_NOT_AUTO_START;
    info.accessTokenId = "123";
    info.setterUserId = 100;
    info.userId = 100;
    info.setterType = AutoStartupSetterType::USER;
    auto result = abilityAutoStartupService->InnerApplicationAutoStartupByEDM(info, true, false);
    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceThirdTest InnerApplicationAutoStartupByEDM_005 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: InnerSetApplicationAutoStartup
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService InnerSetApplicationAutoStartup
 */
HWTEST_F(AbilityAutoStartupServiceThirdTest, InnerApplicationAutoStartupByEDM_006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceThirdTest InnerApplicationAutoStartupByEDM_006 start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);
    AutoStartupInfo info;
    info.bundleName = BUNDLENAME_NO_FONUD;
    info.abilityName = ABILITYNAME_NOT_AUTO_START;
    info.accessTokenId = "123";
    info.setterUserId = 100;
    info.userId = 100;
    info.setterType = AutoStartupSetterType::USER;
    auto result = abilityAutoStartupService->InnerApplicationAutoStartupByEDM(info, false, false);
    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceThirdTest InnerApplicationAutoStartupByEDM_006 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: InnerApplicationAutoStartupByEDM
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService InnerApplicationAutoStartupByEDM
 */
HWTEST_F(AbilityAutoStartupServiceThirdTest, InnerApplicationAutoStartupByEDM_007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceThirdTest InnerApplicationAutoStartupByEDM_007 start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);
    AutoStartupInfo info;
    info.bundleName = BUNDLENAME_FONUD;
    info.abilityName = ABILITYNAME_AUTO_START;
    info.canUserModify = AUTO_STARTUP_SERVICE_TRUE;
    auto result = abilityAutoStartupService->InnerApplicationAutoStartupByEDM(info, false, false);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceThirdTest InnerApplicationAutoStartupByEDM_007 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: InnerApplicationAutoStartupByEDM
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService InnerApplicationAutoStartupByEDM
 */
HWTEST_F(AbilityAutoStartupServiceThirdTest, InnerApplicationAutoStartupByEDM_008, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceThirdTest InnerApplicationAutoStartupByEDM_008 start";
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);
    AutoStartupInfo info;
    info.bundleName = BUNDLENAME_FONUD;
    info.abilityName = ABILITYNAME_AUTO_START;
    info.accessTokenId = "123";
    info.setterUserId = 100;
    info.userId = 100;
    info.setterType = AutoStartupSetterType::USER;
    info.canUserModify = AUTO_STARTUP_SERVICE_TRUE;
    auto result = abilityAutoStartupService->InnerApplicationAutoStartupByEDM(info, false, false);
    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceThirdTest InnerApplicationAutoStartupByEDM_008 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: CheckAutoStartupData
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService CheckAutoStartupData
 */
HWTEST_F(AbilityAutoStartupServiceThirdTest, CheckAutoStartupData_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceThirdTest CheckAutoStartupData_001 start";

    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);

    std::string bundleName = "bundleNameTest";
    int32_t result = abilityAutoStartupService->CheckAutoStartupData(bundleName, BASE_USER_RANGE);
    EXPECT_EQ(result, ERR_OK);

    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceThirdTest CheckAutoStartupData_001 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: CheckAutoStartupData
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService CheckAutoStartupData
 */
HWTEST_F(AbilityAutoStartupServiceThirdTest, CheckAutoStartupData_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceThirdTest CheckAutoStartupData_002 start";

    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);

    std::string bundleName = "hapAbilityInfoVisible";
    int32_t result = abilityAutoStartupService->CheckAutoStartupData(bundleName, BASE_USER_RANGE);
    EXPECT_EQ(result, ERR_NO_INIT);

    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceThirdTest CheckAutoStartupData_002 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: CheckAutoStartupData
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService CheckAutoStartupData
 */
HWTEST_F(AbilityAutoStartupServiceThirdTest, CheckAutoStartupData_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceThirdTest CheckAutoStartupData_003 start";

    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);

    std::string bundleName = "hapModuleInfosModuleNameIsEmpty";
    int32_t result = abilityAutoStartupService->CheckAutoStartupData(bundleName, BASE_USER_RANGE);
    EXPECT_EQ(result, ERR_OK);

    GTEST_LOG_(INFO) << "AbilityAutoStartupServiceThirdTest CheckAutoStartupData_003 end";
}

/*
 * Feature: AbilityAutoStartupService
 * Function: VerifyCrossUserAccountPermission
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService VerifyCrossUserAccountPermission with setterUserId = U0
 * Description: Test VerifyCrossUserAccountPermission when setterUserId is U0, should allow directly
 */
HWTEST_F(AbilityAutoStartupServiceThirdTest, VerifyCrossUserAccountPermission_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityAutoStartupServiceThirdTest VerifyCrossUserAccountPermission_001 start");
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);

    AutoStartupAbilityData abilityData;
    abilityData.setterUserId = 0; // U0_USER_ID
    abilityData.userId = 100;
    auto result = abilityAutoStartupService->VerifyCrossUserAccountPermission(abilityData);
    EXPECT_EQ(result, ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "AbilityAutoStartupServiceThirdTest VerifyCrossUserAccountPermission_001 end");
}

/*
 * Feature: AbilityAutoStartupService
 * Function: VerifyCrossUserAccountPermission
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService VerifyCrossUserAccountPermission with setterUserId = U1
 * Description: Test VerifyCrossUserAccountPermission when setterUserId is U1, should allow directly
 */
HWTEST_F(AbilityAutoStartupServiceThirdTest, VerifyCrossUserAccountPermission_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityAutoStartupServiceThirdTest VerifyCrossUserAccountPermission_002 start");
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);

    AutoStartupAbilityData abilityData;
    abilityData.setterUserId = 1; // U1_USER_ID
    abilityData.userId = 100;
    auto result = abilityAutoStartupService->VerifyCrossUserAccountPermission(abilityData);
    EXPECT_EQ(result, ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "AbilityAutoStartupServiceThirdTest VerifyCrossUserAccountPermission_002 end");
}

/*
 * Feature: AbilityAutoStartupService
 * Function: VerifyCrossUserAccountPermission
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService VerifyCrossUserAccountPermission with setterUserId == userId
 * Description: Test VerifyCrossUserAccountPermission when setterUserId equals userId, should allow directly
 */
HWTEST_F(AbilityAutoStartupServiceThirdTest, VerifyCrossUserAccountPermission_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityAutoStartupServiceThirdTest VerifyCrossUserAccountPermission_003 start");
    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);

    AutoStartupAbilityData abilityData;
    abilityData.setterUserId = 100;
    abilityData.userId = 100;
    auto result = abilityAutoStartupService->VerifyCrossUserAccountPermission(abilityData);
    EXPECT_EQ(result, ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "AbilityAutoStartupServiceThirdTest VerifyCrossUserAccountPermission_003 end");
}

/*
 * Feature: AbilityAutoStartupService
 * Function: VerifyCrossUserAccountPermission
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService VerifyCrossUserAccountPermission without cross-user permission
 * Description: Test VerifyCrossUserAccountPermission when setterUserId != userId and no cross-user permission
 */
HWTEST_F(AbilityAutoStartupServiceThirdTest, VerifyCrossUserAccountPermission_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityAutoStartupServiceThirdTest VerifyCrossUserAccountPermission_004 start");
    system::SetBoolParameter("", true);
    AAFwk::IsMockSaCall::IsMockSaCallWithPermission();
    MyFlag::flag_ = 0; // No VerifyAccountPermission permission

    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);

    AutoStartupAbilityData abilityData;
    abilityData.setterUserId = 100; // Not U0/U1
    abilityData.userId = 101; // Different from setterUserId
    auto result = abilityAutoStartupService->VerifyCrossUserAccountPermission(abilityData);
    EXPECT_EQ(result, CHECK_PERMISSION_FAILED);

    AAFwk::IsMockSaCall::IsMockResetPermission();
    MyFlag::flag_ = 0;
    system::SetBoolParameter("", false);
    TAG_LOGI(AAFwkTag::TEST, "AbilityAutoStartupServiceThirdTest VerifyCrossUserAccountPermission_004 end");
}

/*
 * Feature: AbilityAutoStartupService
 * Function: VerifyCrossUserAccountPermission
 * SubFunction: NA
 * FunctionPoints: AbilityAutoStartupService VerifyCrossUserAccountPermission with cross-user permission
 * Description: Test VerifyCrossUserAccountPermission when setterUserId != userId but has cross-user permission
 */
HWTEST_F(AbilityAutoStartupServiceThirdTest, VerifyCrossUserAccountPermission_005, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityAutoStartupServiceThirdTest VerifyCrossUserAccountPermission_005 start");
    system::SetBoolParameter("", true);
    AAFwk::IsMockSaCall::IsMockSaCallWithPermission();
    MyFlag::flag_ = 1; // Has VerifyAccountPermission permission

    auto abilityAutoStartupService = std::make_shared<AbilityAutoStartupService>();
    EXPECT_NE(abilityAutoStartupService, nullptr);

    AutoStartupAbilityData abilityData;
    abilityData.setterUserId = 100; // Not U0/U1
    abilityData.userId = 101; // Different from setterUserId
    auto result = abilityAutoStartupService->VerifyCrossUserAccountPermission(abilityData);
    EXPECT_EQ(result, ERR_OK);

    AAFwk::IsMockSaCall::IsMockResetPermission();
    MyFlag::flag_ = 0;
    system::SetBoolParameter("", false);
    TAG_LOGI(AAFwkTag::TEST, "AbilityAutoStartupServiceThirdTest VerifyCrossUserAccountPermission_005 end");
}
} // namespace AAFwk
} // namespace OHOS
