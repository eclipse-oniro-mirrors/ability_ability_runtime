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
#include "mock_my_status.h"

#include "ability_manager_service.h"
#include "ability_manager_errors.h"
#include "hilog_tag_wrapper.h"
#include "rate_limiter.h"
#include "sub_managers_helper.h"
#include "mission_list_manager.h"
#include "scene_board_judgement.h"
#include "call_record.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AppExecFwk;
using OHOS::AppExecFwk::AbilityType;
using OHOS::AppExecFwk::ExtensionAbilityType;
namespace OHOS {
namespace AAFwk {
namespace {
constexpr int32_t EXTENSION_MAX_LIMIT = 50;
constexpr int32_t GAME_SA_UID = 7800;
}
class AbilityManagerServiceFourteenthTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    std::shared_ptr<AbilityRecord> MockAbilityRecord(AbilityType);
    std::shared_ptr<BaseExtensionRecord> MockExtensionRecordBase(AbilityType);
    sptr<Token> MockToken(AbilityType);
    sptr<SessionInfo> MockSessionInfo(int32_t persistentId);
    AbilityRequest GenerateAbilityRequest(const std::string& deviceName, const std::string& abilityName,
        const std::string& appName, const std::string& bundleName, const std::string& moduleName);

    AbilityRequest abilityRequest_{};
    Want want_{};
};

void AbilityManagerServiceFourteenthTest::SetUpTestCase() {}

void AbilityManagerServiceFourteenthTest::TearDownTestCase() {}

void AbilityManagerServiceFourteenthTest::SetUp() {}

void AbilityManagerServiceFourteenthTest::TearDown() {}

std::shared_ptr<AbilityRecord> AbilityManagerServiceFourteenthTest::MockAbilityRecord(AbilityType abilityType)
{
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.test.demo";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = abilityType;
    return AbilityRecord::CreateAbilityRecord(abilityRequest);
}

std::shared_ptr<BaseExtensionRecord> AbilityManagerServiceFourteenthTest::MockExtensionRecordBase(
    AbilityType abilityType)
{
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.test.demo";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = abilityType;
    return BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
}

sptr<Token> AbilityManagerServiceFourteenthTest::MockToken(AbilityType abilityType)
{
    std::shared_ptr<AbilityRecord> abilityRecord = MockAbilityRecord(abilityType);
    if (!abilityRecord) {
        return nullptr;
    }
    return abilityRecord->GetToken();
}

sptr<SessionInfo> AbilityManagerServiceFourteenthTest::MockSessionInfo(int32_t persistentId)
{
    sptr<SessionInfo> sessionInfo = new (std::nothrow) SessionInfo();
    if (!sessionInfo) {
        TAG_LOGE(AAFwkTag::TEST, "sessionInfo is nullptr");
        return nullptr;
    }
    sessionInfo->persistentId = persistentId;
    return sessionInfo;
}

AbilityRequest AbilityManagerServiceFourteenthTest::GenerateAbilityRequest(const std::string& deviceName,
    const std::string& abilityName, const std::string& appName, const std::string& bundleName,
    const std::string& moduleName)
{
    ElementName element(deviceName, bundleName, abilityName, moduleName);
    want_.SetElement(element);

    AbilityInfo abilityInfo;
    abilityInfo.visible = true;
    abilityInfo.applicationName = appName;
    abilityInfo.type = AbilityType::EXTENSION;
    abilityInfo.name = abilityName;
    abilityInfo.bundleName = bundleName;
    abilityInfo.moduleName = moduleName;
    abilityInfo.deviceId = deviceName;
    ApplicationInfo appinfo;
    appinfo.name = appName;
    appinfo.bundleName = bundleName;
    abilityInfo.applicationInfo = appinfo;
    AbilityRequest abilityRequest;
    abilityRequest.want = want_;
    abilityRequest.abilityInfo = abilityInfo;
    abilityRequest.appInfo = appinfo;

    return abilityRequest;
}

/*
 * Feature: AbilityManagerService
 * Name: NotifyDebugAssertResult_001
 * Function: NotifyDebugAssertResult
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService NotifyDebugAssertResult
 */
HWTEST_F(AbilityManagerServiceFourteenthTest, NotifyDebugAssertResult_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourteenthTest NotifyDebugAssertResult_001 start");
    MyStatus::GetInstance().paramGetBoolParameter_ = false;
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    int retCode = abilityMs_->NotifyDebugAssertResult(0, AAFwk::UserStatus::ASSERT_TERMINATE);
    EXPECT_EQ(retCode, ERR_NOT_SUPPORTED_PRODUCT_TYPE);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourteenthTest NotifyDebugAssertResult_001 end");
}

/*
 * Feature: AbilityManagerService
 * Name: NotifyDebugAssertResult_002
 * Function: NotifyDebugAssertResult
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService NotifyDebugAssertResult
 */
HWTEST_F(AbilityManagerServiceFourteenthTest, NotifyDebugAssertResult_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourteenthTest NotifyDebugAssertResult_002 start");
    MyStatus::GetInstance().paramGetBoolParameter_ = true;
    MyStatus::GetInstance().permPermission_ = 0;
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    int retCode = abilityMs_->NotifyDebugAssertResult(0, AAFwk::UserStatus::ASSERT_TERMINATE);
    EXPECT_EQ(retCode, ERR_PERMISSION_DENIED);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourteenthTest NotifyDebugAssertResult_002 end");
}

/*
 * Feature: AbilityManagerService
 * Name: NotifyDebugAssertResult_003
 * Function: NotifyDebugAssertResult
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService NotifyDebugAssertResult
 */
HWTEST_F(AbilityManagerServiceFourteenthTest, NotifyDebugAssertResult_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourteenthTest NotifyDebugAssertResult_003 start");
    MyStatus::GetInstance().paramGetBoolParameter_ = true;
    MyStatus::GetInstance().permPermission_ = 1;
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    int retCode = abilityMs_->NotifyDebugAssertResult(0, AAFwk::UserStatus::ASSERT_TERMINATE);
    EXPECT_EQ(retCode, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourteenthTest NotifyDebugAssertResult_003 end");
}

/*
 * Feature: AbilityManagerService
 * Name: NotifyDebugAssertResult_004
 * Function: NotifyDebugAssertResult
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService NotifyDebugAssertResult
 */
HWTEST_F(AbilityManagerServiceFourteenthTest, NotifyDebugAssertResult_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourteenthTest NotifyDebugAssertResult_004 start");
    MyStatus::GetInstance().paramGetBoolParameter_ = true;
    MyStatus::GetInstance().perJudgeCallerIsAllowedToUseSystemAPI_ = false;
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    int retCode = abilityMs_->NotifyDebugAssertResult(0, AAFwk::UserStatus::ASSERT_TERMINATE);
    EXPECT_EQ(retCode, ERR_NOT_SYSTEM_APP);
    MyStatus::GetInstance().perJudgeCallerIsAllowedToUseSystemAPI_ = true;
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourteenthTest NotifyDebugAssertResult_004 end");
}

/*
 * Feature: AbilityManagerService
 * Name: GetForegroundUIAbilities_001
 * Function: GetForegroundUIAbilities
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetForegroundUIAbilities
 */
HWTEST_F(AbilityManagerServiceFourteenthTest, GetForegroundUIAbilities_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourteenthTest GetForegroundUIAbilities_001 start");
    MyStatus::GetInstance().permPermission_ = false;
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    std::vector<AppExecFwk::AbilityStateData> list;
    int retCode = abilityMs_->GetForegroundUIAbilities(list);
    EXPECT_EQ(retCode, CHECK_PERMISSION_FAILED);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourteenthTest GetForegroundUIAbilities_001 end");
}

/*
 * Feature: AbilityManagerService
 * Name: GetForegroundUIAbilities_002
 * Function:GetForegroundUIAbilities
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetForegroundUIAbilities
 */
HWTEST_F(AbilityManagerServiceFourteenthTest, GetForegroundUIAbilities_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourteenthTest GetForegroundUIAbilities_002 start");
    MyStatus::GetInstance().permPermission_ = true;
    MyStatus::GetInstance().sbjIsSceneBoardEnabled_ = true;
    MyStatus::GetInstance().smhGetUIAbilityManagerByUserId_ = false;

    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    auto mockSubManagersHelper = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    EXPECT_NE(mockSubManagersHelper, nullptr);
    abilityMs_->subManagersHelper_ = mockSubManagersHelper;
    abilityMs_->subManagersHelper_->currentUIAbilityManager_ = nullptr;

    std::vector<AppExecFwk::AbilityStateData> list;
    int retCode = abilityMs_->GetForegroundUIAbilities(list);
    EXPECT_EQ(retCode, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourteenthTest GetForegroundUIAbilities_002 end");
}

/*
 * Feature: AbilityManagerService
 * Name: GetForegroundUIAbilities_003
 * Function:GetForegroundUIAbilities
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetForegroundUIAbilities
 */
HWTEST_F(AbilityManagerServiceFourteenthTest, GetForegroundUIAbilities_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourteenthTest GetForegroundUIAbilities_003 start");
    MyStatus::GetInstance().permPermission_ = true;
    MyStatus::GetInstance().sbjIsSceneBoardEnabled_ = true;
    MyStatus::GetInstance().smhGetUIAbilityManagerByUserId_ = true;

    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    auto mockSubManagersHelper = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    EXPECT_NE(mockSubManagersHelper, nullptr);
    auto mockCurrentUIAbilityManager = std::make_shared<UIAbilityLifecycleManager>(0);
    EXPECT_NE(mockCurrentUIAbilityManager, nullptr);
    abilityMs_->subManagersHelper_ = mockSubManagersHelper;
    abilityMs_->subManagersHelper_->currentUIAbilityManager_ = mockCurrentUIAbilityManager;

    std::vector<AppExecFwk::AbilityStateData> list;
    int retCode = abilityMs_->GetForegroundUIAbilities(list);
    EXPECT_EQ(retCode, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourteenthTest GetForegroundUIAbilities_003 end");
}

/*
 * Feature: AbilityManagerService
 * Name: GetForegroundUIAbilities_004
 * Function:GetForegroundUIAbilities
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetForegroundUIAbilities
 */
HWTEST_F(AbilityManagerServiceFourteenthTest, GetForegroundUIAbilities_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourteenthTest GetForegroundUIAbilities_004 start");
    MyStatus::GetInstance().permPermission_ = true;
    MyStatus::GetInstance().sbjIsSceneBoardEnabled_ = false;
    MyStatus::GetInstance().smhGetMissionListManagerByUserId_ = false;

    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    auto mockSubManagersHelper = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    EXPECT_NE(mockSubManagersHelper, nullptr);
    abilityMs_->subManagersHelper_ = mockSubManagersHelper;
    abilityMs_->subManagersHelper_->currentMissionListManager_ = nullptr;

    std::vector<AppExecFwk::AbilityStateData> list;
    int retCode = abilityMs_->GetForegroundUIAbilities(list);
    EXPECT_EQ(retCode, ERR_NULL_OBJECT);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourteenthTest GetForegroundUIAbilities_004 end");
}

/*
 * Feature: AbilityManagerService
 * Name: GetForegroundUIAbilities_005
 * Function:GetForegroundUIAbilities
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetForegroundUIAbilities
 */
HWTEST_F(AbilityManagerServiceFourteenthTest, GetForegroundUIAbilities_005, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourteenthTest GetForegroundUIAbilities_005 start");
    MyStatus::GetInstance().perJudgeCallerIsAllowedToUseSystemAPI_ = false;
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    std::vector<AppExecFwk::AbilityStateData> list;
    int retCode = abilityMs_->GetForegroundUIAbilities(list);
    EXPECT_EQ(retCode, ERR_NOT_SYSTEM_APP);
    MyStatus::GetInstance().perJudgeCallerIsAllowedToUseSystemAPI_ = true;
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourteenthTest GetForegroundUIAbilities_005 end");
}

/*
 * Feature: AbilityManagerService
 * Name: IsCallerSceneBoard_001
 * Function: IsCallerSceneBoard
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService IsCallerSceneBoard
 */
HWTEST_F(AbilityManagerServiceFourteenthTest, IsCallerSceneBoard_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourteenthTest IsCallerSceneBoard_001 start");
    MyStatus::GetInstance().smhGetConnectManagerByToken_ = false;
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    int retCode = abilityMs_->IsCallerSceneBoard();
    EXPECT_FALSE(retCode);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourteenthTest IsCallerSceneBoard_001 end");
}

/*
 * Feature: AbilityManagerService
 * Name: IsCallerSceneBoard_002
 * Function: IsCallerSceneBoard
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService IsCallerSceneBoard
 */
HWTEST_F(AbilityManagerServiceFourteenthTest, IsCallerSceneBoard_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourteenthTest IsCallerSceneBoard_002 start");
    MyStatus::GetInstance().smhGetConnectManagerByToken_ = true;
    
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    auto mockSubManagersHelper = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    EXPECT_NE(mockSubManagersHelper, nullptr);
    auto mockCurrentConnectManager = std::make_shared<CommonExtensionManager>(0);
    EXPECT_NE(mockCurrentConnectManager, nullptr);
    mockCurrentConnectManager->sceneBoardTokenId_ = 0;
    abilityMs_->subManagersHelper_ = mockSubManagersHelper;
    abilityMs_->subManagersHelper_->currentCommonExtensionManager_ = mockCurrentConnectManager;

    int retCode = abilityMs_->IsCallerSceneBoard();
    EXPECT_FALSE(retCode);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourteenthTest IsCallerSceneBoard_002 end");
}

/*
 * Feature: AbilityManagerService
 * Name: IsCallerSceneBoard_003
 * Function: IsCallerSceneBoard
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService IsCallerSceneBoard
 */
HWTEST_F(AbilityManagerServiceFourteenthTest, IsCallerSceneBoard_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourteenthTest IsCallerSceneBoard_003 start");
    MyStatus::GetInstance().smhGetConnectManagerByToken_ = true;
    MyStatus::GetInstance().ipcGetCallingTokenID_ = 0;

    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    auto mockSubManagersHelper = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    EXPECT_NE(mockSubManagersHelper, nullptr);
    auto mockCurrentConnectManager = std::make_shared<CommonExtensionManager>(0);
    EXPECT_NE(mockCurrentConnectManager, nullptr);
    mockCurrentConnectManager->sceneBoardTokenId_ = 1;
    abilityMs_->subManagersHelper_ = mockSubManagersHelper;
    abilityMs_->subManagersHelper_->currentCommonExtensionManager_ = mockCurrentConnectManager;

    int retCode = abilityMs_->IsCallerSceneBoard();
    EXPECT_FALSE(retCode);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourteenthTest IsCallerSceneBoard_003 end");
}

/*
 * Feature: AbilityManagerService
 * Name: IsCallerSceneBoard_004
 * Function: IsCallerSceneBoard
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService IsCallerSceneBoard
 */
HWTEST_F(AbilityManagerServiceFourteenthTest, IsCallerSceneBoard_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourteenthTest IsCallerSceneBoard_004 start");
    MyStatus::GetInstance().smhGetConnectManagerByToken_ = true;
    MyStatus::GetInstance().ipcGetCallingTokenID_ = 1;

    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    auto mockSubManagersHelper = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    EXPECT_NE(mockSubManagersHelper, nullptr);
    auto mockCurrentConnectManager = std::make_shared<CommonExtensionManager>(0);
    EXPECT_NE(mockCurrentConnectManager, nullptr);
    mockCurrentConnectManager->sceneBoardTokenId_ = 1;
    abilityMs_->subManagersHelper_ = mockSubManagersHelper;
    abilityMs_->subManagersHelper_->currentCommonExtensionManager_ = mockCurrentConnectManager;

    int retCode = abilityMs_->IsCallerSceneBoard();
    EXPECT_TRUE(retCode);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourteenthTest IsCallerSceneBoard_004 end");
}

/*
 * Feature: AbilityManagerService
 * Name: AcquireShareData_001
 * Function: AcquireShareData
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService AcquireShareData
 */
HWTEST_F(AbilityManagerServiceFourteenthTest, AcquireShareData_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourteenthTest AcquireShareData_001 start");
    MyStatus::GetInstance().perJudgeCallerIsAllowedToUseSystemAPI_ = false;
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    int retCode = abilityMs_->AcquireShareData(0, nullptr);
    EXPECT_EQ(retCode, ERR_NOT_SYSTEM_APP);
    MyStatus::GetInstance().perJudgeCallerIsAllowedToUseSystemAPI_ = true;
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourteenthTest AcquireShareData_001 end");
}

/*
 * Feature: AbilityManagerService
 * Name: AcquireShareData_002
 * Function: AcquireShareData
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService AcquireShareData
 */
HWTEST_F(AbilityManagerServiceFourteenthTest, AcquireShareData_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourteenthTest AcquireShareData_002 start");
    MyStatus::GetInstance().perJudgeCallerIsAllowedToUseSystemAPI_ = true;
    MyStatus::GetInstance().sbjIsSceneBoardEnabled_ = true;
    MyStatus::GetInstance().smhGetUIAbilityManagerByUserId_ = false;

    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    auto mockSubManagersHelper = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    EXPECT_NE(mockSubManagersHelper, nullptr);
    abilityMs_->subManagersHelper_ = mockSubManagersHelper;
    abilityMs_->subManagersHelper_->currentUIAbilityManager_ = nullptr;

    int retCode = abilityMs_->AcquireShareData(0, nullptr);
    EXPECT_EQ(retCode, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourteenthTest AcquireShareData_002 end");
}

/*
 * Feature: AbilityManagerService
 * Name: AcquireShareData_003
 * Function: AcquireShareData
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService AcquireShareData
 */
HWTEST_F(AbilityManagerServiceFourteenthTest, AcquireShareData_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourteenthTest AcquireShareData_003 start");
    MyStatus::GetInstance().perJudgeCallerIsAllowedToUseSystemAPI_ = true;
    MyStatus::GetInstance().sbjIsSceneBoardEnabled_ = true;
    MyStatus::GetInstance().smhGetUIAbilityManagerByUserId_ = true;

    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    auto mockSubManagersHelper = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    EXPECT_NE(mockSubManagersHelper, nullptr);
    auto mockCurrentUIAbilityManager = std::make_shared<UIAbilityLifecycleManager>(0);
    EXPECT_NE(mockCurrentUIAbilityManager, nullptr);
    abilityMs_->subManagersHelper_ = mockSubManagersHelper;
    abilityMs_->subManagersHelper_->currentUIAbilityManager_ = mockCurrentUIAbilityManager;

    int retCode = abilityMs_->AcquireShareData(0, nullptr);
    EXPECT_EQ(retCode, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourteenthTest AcquireShareData_003 end");
}

/*
 * Feature: AbilityManagerService
 * Name: AcquireShareData_004
 * Function: AcquireShareData
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService AcquireShareData
 */
HWTEST_F(AbilityManagerServiceFourteenthTest, AcquireShareData_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourteenthTest AcquireShareData_004 start");
    MyStatus::GetInstance().perJudgeCallerIsAllowedToUseSystemAPI_ = true;
    MyStatus::GetInstance().sbjIsSceneBoardEnabled_ = false;
    MyStatus::GetInstance().smhGetMissionListManagerByUserId_ = false;

    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    auto mockSubManagersHelper = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    EXPECT_NE(mockSubManagersHelper, nullptr);
    abilityMs_->subManagersHelper_ = mockSubManagersHelper;
    abilityMs_->subManagersHelper_->currentMissionListManager_ = nullptr;

    int retCode = abilityMs_->AcquireShareData(0, nullptr);
    EXPECT_EQ(retCode, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourteenthTest AcquireShareData_004 end");
}

/*
 * Feature: AbilityManagerService
 * Name: SetMissionIcon_001
 * Function: SetMissionIcon
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService SetMissionIcon
 */
HWTEST_F(AbilityManagerServiceFourteenthTest, SetMissionIcon_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourteenthTest SetMissionIcon_001 start");
    MyStatus::GetInstance().perJudgeCallerIsAllowedToUseSystemAPI_ = false;
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    int retCode = abilityMs_->SetMissionIcon(nullptr, nullptr);
    EXPECT_EQ(retCode, ERR_NOT_SYSTEM_APP);
    MyStatus::GetInstance().perJudgeCallerIsAllowedToUseSystemAPI_ = true;
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourteenthTest SetMissionIcon_001 end");
}

/*
 * Feature: AbilityManagerService
 * Name: SetMissionIcon_002
 * Function: SetMissionIcon
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService SetMissionIcon
 */
HWTEST_F(AbilityManagerServiceFourteenthTest, SetMissionIcon_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourteenthTest SetMissionIcon_002 start");
    MyStatus::GetInstance().perJudgeCallerIsAllowedToUseSystemAPI_ = true;
    MyStatus::GetInstance().arGetAbilityRecord_ = nullptr;
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    int retCode = abilityMs_->SetMissionIcon(nullptr, nullptr);
    EXPECT_EQ(retCode, -1);
    MyStatus::GetInstance().perJudgeCallerIsAllowedToUseSystemAPI_ = true;
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourteenthTest SetMissionIcon_002 end");
}

/*
 * Feature: AbilityManagerService
 * Name: SetMissionIcon_003
 * Function: SetMissionIcon
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService SetMissionIcon
 */
HWTEST_F(AbilityManagerServiceFourteenthTest, SetMissionIcon_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourteenthTest SetMissionIcon_003 start");
    MyStatus::GetInstance().perJudgeCallerIsAllowedToUseSystemAPI_ = true;
    MyStatus::GetInstance().arGetAbilityRecord_ = MockAbilityRecord(AbilityType::PAGE);
    MyStatus::GetInstance().arGetAbilityInfo_.applicationInfo.accessTokenId = 1;
    MyStatus::GetInstance().ipcGetCallingTokenID_ = 0;

    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    int retCode = abilityMs_->SetMissionIcon(nullptr, nullptr);
    EXPECT_EQ(retCode, -1);
    MyStatus::GetInstance().perJudgeCallerIsAllowedToUseSystemAPI_ = true;
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourteenthTest SetMissionIcon_003 end");
}

/*
 * Feature: AbilityManagerService
 * Name: SetMissionIcon_004
 * Function: SetMissionIcon
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService SetMissionIcon
 */
HWTEST_F(AbilityManagerServiceFourteenthTest, SetMissionIcon_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourteenthTest SetMissionIcon_004 start");
    MyStatus::GetInstance().perJudgeCallerIsAllowedToUseSystemAPI_ = true;
    MyStatus::GetInstance().arGetAbilityRecord_ = MockAbilityRecord(AbilityType::PAGE);
    MyStatus::GetInstance().arGetAbilityInfo_.applicationInfo.accessTokenId = 1;
    MyStatus::GetInstance().ipcGetCallingTokenID_ = 1;

    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    int retCode = abilityMs_->SetMissionIcon(nullptr, nullptr);
    EXPECT_EQ(retCode, -1);
    MyStatus::GetInstance().perJudgeCallerIsAllowedToUseSystemAPI_ = true;
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourteenthTest SetMissionIcon_004 end");
}

/*
 * Feature: AbilityManagerService
 * Name: GetTopAbility_001
 * Function: GetTopAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetTopAbility
 */
HWTEST_F(AbilityManagerServiceFourteenthTest, GetTopAbility_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourteenthTest GetTopAbility_001 start");
    MyStatus::GetInstance().permPermission_ = 0;
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    sptr<IRemoteObject> token = nullptr;
    int retCode = abilityMs_->GetTopAbility(token);
    EXPECT_EQ(retCode, CHECK_PERMISSION_FAILED);
    MyStatus::GetInstance().perJudgeCallerIsAllowedToUseSystemAPI_ = true;
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourteenthTest GetTopAbility_001 end");
}

/*
 * Feature: AbilityManagerService
 * Name: GetTopAbility_002
 * Function: GetTopAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetTopAbility
 */
HWTEST_F(AbilityManagerServiceFourteenthTest, GetTopAbility_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourteenthTest GetTopAbility_002 start");
    MyStatus::GetInstance().permPermission_ = 1;
    MyStatus::GetInstance().sbjIsSceneBoardEnabled_ = true;
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    sptr<IRemoteObject> token = nullptr;
    int retCode = abilityMs_->GetTopAbility(token);
    EXPECT_EQ(retCode, ERR_INVALID_VALUE);
    MyStatus::GetInstance().perJudgeCallerIsAllowedToUseSystemAPI_ = true;
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourteenthTest GetTopAbility_002 end");
}

/*
 * Feature: AbilityManagerService
 * Name: GetTopAbility_003
 * Function: GetTopAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetTopAbility
 */
HWTEST_F(AbilityManagerServiceFourteenthTest, GetTopAbility_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourteenthTest GetTopAbility_003 start");
    MyStatus::GetInstance().permPermission_ = 1;
    MyStatus::GetInstance().sbjIsSceneBoardEnabled_ = false;
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    abilityMs_->wmsHandler_ = nullptr;
    sptr<IRemoteObject> token = nullptr;
    int retCode = abilityMs_->GetTopAbility(token);
    EXPECT_EQ(retCode, ERR_INVALID_VALUE);
    MyStatus::GetInstance().perJudgeCallerIsAllowedToUseSystemAPI_ = true;
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourteenthTest GetTopAbility_003 end");
}

/*
 * Feature: AbilityManagerService
 * Name: TerminateAbilityWithFlag_001
 * Function: TerminateAbilityWithFlag
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService TerminateAbilityWithFlag
 */
HWTEST_F(AbilityManagerServiceFourteenthTest, TerminateAbilityWithFlag_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourteenthTest TerminateAbilityWithFlag_001 start");
    MyStatus::GetInstance().smhVerificationAllToken_ = false;
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);

    int retCode = abilityMs_->TerminateAbilityWithFlag(nullptr, 0, nullptr, true);
    EXPECT_EQ(retCode, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourteenthTest TerminateAbilityWithFlag_001 end");
}

/*
 * Feature: AbilityManagerService
 * Name: TerminateAbilityWithFlag_002
 * Function: TerminateAbilityWithFlag
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService TerminateAbilityWithFlag
 */
HWTEST_F(AbilityManagerServiceFourteenthTest, TerminateAbilityWithFlag_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourteenthTest TerminateAbilityWithFlag_002 start");
    MyStatus::GetInstance().smhVerificationAllToken_ = true;
    MyStatus::GetInstance().arGetAbilityRecord_ = nullptr;
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    auto mockSubManagersHelper = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    EXPECT_NE(mockSubManagersHelper, nullptr);
    abilityMs_->subManagersHelper_ = mockSubManagersHelper;
    int retCode = abilityMs_->TerminateAbilityWithFlag(nullptr, 0, nullptr, true);
    EXPECT_EQ(retCode, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourteenthTest TerminateAbilityWithFlag_002 end");
}

/*
 * Feature: AbilityManagerService
 * Name: TerminateAbilityWithFlag_003
 * Function: TerminateAbilityWithFlag
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService TerminateAbilityWithFlag
 */
HWTEST_F(AbilityManagerServiceFourteenthTest, TerminateAbilityWithFlag_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourteenthTest TerminateAbilityWithFlag_003 start");
    MyStatus::GetInstance().smhVerificationAllToken_ = true;
    MyStatus::GetInstance().arGetAbilityRecord_ = MockAbilityRecord(AbilityType::SERVICE);
    MyStatus::GetInstance().ipcGetCallingUid_ = 1;
    MyStatus::GetInstance().arGetAbilityInfo_.bundleName = AbilityConfig::SYSTEM_UI_BUNDLE_NAME;
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    auto mockSubManagersHelper = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    EXPECT_NE(mockSubManagersHelper, nullptr);
    abilityMs_->subManagersHelper_ = mockSubManagersHelper;
    int retCode = abilityMs_->TerminateAbilityWithFlag(nullptr, 0, nullptr, true);
    EXPECT_EQ(retCode, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourteenthTest TerminateAbilityWithFlag_003 end");
}

/*
 * Feature: AbilityManagerService
 * Name: TerminateAbilityWithFlag_004
 * Function: TerminateAbilityWithFlag
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService TerminateAbilityWithFlag
 */
HWTEST_F(AbilityManagerServiceFourteenthTest, TerminateAbilityWithFlag_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourteenthTest TerminateAbilityWithFlag_004 start");
    MyStatus::GetInstance().smhVerificationAllToken_ = true;
    MyStatus::GetInstance().arGetAbilityRecord_ = MockAbilityRecord(AbilityType::SERVICE);
    MyStatus::GetInstance().ipcGetCallingUid_ = 1;
    MyStatus::GetInstance().arGetAbilityInfo_.bundleName = AbilityConfig::SYSTEM_UI_NAVIGATION_BAR;
    MyStatus::GetInstance().arGetAbilityInfo_.type = AbilityType::SERVICE;
    MyStatus::GetInstance().smhGetConnectManagerByToken_ = false;
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    auto mockSubManagersHelper = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    EXPECT_NE(mockSubManagersHelper, nullptr);
    abilityMs_->subManagersHelper_ = mockSubManagersHelper;
    int retCode = abilityMs_->TerminateAbilityWithFlag(nullptr, 0, nullptr, true);
    EXPECT_EQ(retCode, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourteenthTest TerminateAbilityWithFlag_004 end");
}

/*
 * Feature: AbilityManagerService
 * Name: TerminateAbilityWithFlag_005
 * Function: TerminateAbilityWithFlag
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService TerminateAbilityWithFlag
 */
HWTEST_F(AbilityManagerServiceFourteenthTest, TerminateAbilityWithFlag_005, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourteenthTest TerminateAbilityWithFlag_005 start");
    MyStatus::GetInstance().smhVerificationAllToken_ = true;
    MyStatus::GetInstance().arGetAbilityRecord_ = MockAbilityRecord(AbilityType::SERVICE);
    MyStatus::GetInstance().ipcGetCallingUid_ = 1;
    MyStatus::GetInstance().arGetAbilityInfo_.bundleName = AbilityConfig::SYSTEM_UI_NAVIGATION_BAR;
    MyStatus::GetInstance().arGetAbilityInfo_.type = AbilityType::SERVICE;
    MyStatus::GetInstance().smhGetConnectManagerByToken_ = true;
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);

    auto mockSubManagersHelper = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    EXPECT_NE(mockSubManagersHelper, nullptr);
    auto mockCurrentConnectManager = std::make_shared<CommonExtensionManager>(0);
    EXPECT_NE(mockCurrentConnectManager, nullptr);
    abilityMs_->subManagersHelper_ = mockSubManagersHelper;
    abilityMs_->subManagersHelper_->currentCommonExtensionManager_ = mockCurrentConnectManager;

    int retCode = abilityMs_->TerminateAbilityWithFlag(nullptr, 0, nullptr, true);
    EXPECT_EQ(retCode, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourteenthTest TerminateAbilityWithFlag_005 end");
}

/*
 * Feature: AbilityManagerService
 * Name: TerminateAbilityWithFlag_006
 * Function: TerminateAbilityWithFlag
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService TerminateAbilityWithFlag
 */
HWTEST_F(AbilityManagerServiceFourteenthTest, TerminateAbilityWithFlag_006, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourteenthTest TerminateAbilityWithFlag_006 start");
    MyStatus::GetInstance().smhVerificationAllToken_ = true;
    MyStatus::GetInstance().arGetAbilityRecord_ = MockAbilityRecord(AbilityType::SERVICE);
    MyStatus::GetInstance().ipcGetCallingUid_ = 1;
    MyStatus::GetInstance().arGetAbilityInfo_.bundleName = AbilityConfig::SYSTEM_UI_NAVIGATION_BAR;
    MyStatus::GetInstance().arGetAbilityInfo_.type = AbilityType::DATA;
    MyStatus::GetInstance().smhGetConnectManagerByToken_ = false;
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    auto mockSubManagersHelper = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    EXPECT_NE(mockSubManagersHelper, nullptr);
    abilityMs_->subManagersHelper_ = mockSubManagersHelper;
    int retCode = abilityMs_->TerminateAbilityWithFlag(nullptr, 0, nullptr, true);
    EXPECT_EQ(retCode, ERR_WRONG_INTERFACE_CALL);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourteenthTest TerminateAbilityWithFlag_006 end");
}

/*
 * Feature: AbilityManagerService
 * Name: GetConnectManagerAndUIExtensionBySessionInfo_001
 * Function: GetConnectManagerAndUIExtensionBySessionInfo
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetConnectManagerAndUIExtensionBySessionInfo
 */
HWTEST_F(AbilityManagerServiceFourteenthTest, GetConnectManagerAndUIExtensionBySessionInfo_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST,
             "AbilityManagerServiceFourteenthTest GetConnectManagerAndUIExtensionBySessionInfo_001 start");
    MyStatus::GetInstance().smhGetConnectManagerByToken_ = false;
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    std::shared_ptr<UIExtensionAbilityManager> connectManager = nullptr;
    std::shared_ptr<BaseExtensionRecord> targetAbility = nullptr;
    abilityMs_->GetConnectManagerAndUIExtensionBySessionInfo(MockSessionInfo(0), connectManager, targetAbility, true);
    EXPECT_EQ(connectManager, nullptr);
    TAG_LOGI(AAFwkTag::TEST,
             "AbilityManagerServiceFourteenthTest GetConnectManagerAndUIExtensionBySessionInfo_001 end");
}

/*
 * Feature: AbilityManagerService
 * Name: GetConnectManagerAndUIExtensionBySessionInfo_002
 * Function: GetConnectManagerAndUIExtensionBySessionInfo
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetConnectManagerAndUIExtensionBySessionInfo
 */
HWTEST_F(AbilityManagerServiceFourteenthTest, GetConnectManagerAndUIExtensionBySessionInfo_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST,
             "AbilityManagerServiceFourteenthTest GetConnectManagerAndUIExtensionBySessionInfo_002 start");
    MyStatus::GetInstance().smhGetConnectManagerByToken_ = true;
    MyStatus::GetInstance().acmGetUIExtensionBySessionInfo_ = nullptr;
    MyStatus::GetInstance().acmGetUIExtensionBySessionFromServiceMap_ = MockExtensionRecordBase(AbilityType::SERVICE);

    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);

    auto mockSubManagersHelper = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    EXPECT_NE(mockSubManagersHelper, nullptr);
    auto mockCurrentConnectManager = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(mockCurrentConnectManager, nullptr);
    abilityMs_->subManagersHelper_ = mockSubManagersHelper;
    abilityMs_->subManagersHelper_->currentUIExtensionAbilityManager_ = mockCurrentConnectManager;

    std::shared_ptr<UIExtensionAbilityManager> connectManager = nullptr;
    std::shared_ptr<BaseExtensionRecord> targetAbility = nullptr;
    abilityMs_->GetConnectManagerAndUIExtensionBySessionInfo(MockSessionInfo(0), connectManager, targetAbility, true);
    EXPECT_NE(connectManager, nullptr);
    EXPECT_NE(targetAbility, nullptr);
    TAG_LOGI(AAFwkTag::TEST,
             "AbilityManagerServiceFourteenthTest GetConnectManagerAndUIExtensionBySessionInfo_002 end");
}

/*
 * Feature: AbilityManagerService
 * Name: GetConnectManagerAndUIExtensionBySessionInfo_003
 * Function: GetConnectManagerAndUIExtensionBySessionInfo
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetConnectManagerAndUIExtensionBySessionInfo
 */
HWTEST_F(AbilityManagerServiceFourteenthTest, GetConnectManagerAndUIExtensionBySessionInfo_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST,
             "AbilityManagerServiceFourteenthTest GetConnectManagerAndUIExtensionBySessionInfo_003 start");
    MyStatus::GetInstance().smhGetConnectManagerByToken_ = true;
    MyStatus::GetInstance().acmGetUIExtensionBySessionInfo_ = nullptr;
    MyStatus::GetInstance().ipcGetCallingUid_ = U0_USER_ID;
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);

    auto mockSubManagersHelper = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    EXPECT_NE(mockSubManagersHelper, nullptr);
    auto mockCurrentConnectManager = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(mockCurrentConnectManager, nullptr);
    abilityMs_->subManagersHelper_ = mockSubManagersHelper;
    abilityMs_->subManagersHelper_->currentUIExtensionAbilityManager_ = mockCurrentConnectManager;

    std::shared_ptr<UIExtensionAbilityManager> connectManager = nullptr;
    std::shared_ptr<BaseExtensionRecord> targetAbility = nullptr;
    abilityMs_->GetConnectManagerAndUIExtensionBySessionInfo(
        MockSessionInfo(0), connectManager, targetAbility, false);
    EXPECT_NE(connectManager, nullptr);
    EXPECT_EQ(targetAbility, nullptr);
    TAG_LOGI(AAFwkTag::TEST,
             "AbilityManagerServiceFourteenthTest GetConnectManagerAndUIExtensionBySessionInfo_003 end");
}

/*
 * Feature: AbilityManagerService
 * Name: GetConnectManagerAndUIExtensionBySessionInfo_004
 * Function: GetConnectManagerAndUIExtensionBySessionInfo
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetConnectManagerAndUIExtensionBySessionInfo
 */
HWTEST_F(AbilityManagerServiceFourteenthTest, GetConnectManagerAndUIExtensionBySessionInfo_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST,
             "AbilityManagerServiceFourteenthTest GetConnectManagerAndUIExtensionBySessionInfo_004 start");
    MyStatus::GetInstance().smhGetConnectManagerByToken_ = true;
    MyStatus::GetInstance().acmGetUIExtensionBySessionInfo_ = nullptr;
    MyStatus::GetInstance().ipcGetCallingUid_ = -1;
    MyStatus::GetInstance().acmGetUIExtensionBySessionFromServiceMap_ = nullptr;
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);

    auto mockSubManagersHelper = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    EXPECT_NE(mockSubManagersHelper, nullptr);
    auto mockCurrentConnectManager = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(mockCurrentConnectManager, nullptr);
    abilityMs_->subManagersHelper_ = mockSubManagersHelper;
    abilityMs_->subManagersHelper_->currentUIExtensionAbilityManager_ = mockCurrentConnectManager;

    std::shared_ptr<UIExtensionAbilityManager> connectManager = nullptr;
    std::shared_ptr<BaseExtensionRecord> targetAbility = nullptr;
    abilityMs_->GetConnectManagerAndUIExtensionBySessionInfo(MockSessionInfo(0), connectManager, targetAbility, true);
    EXPECT_NE(connectManager, nullptr);
    EXPECT_EQ(targetAbility, nullptr);
    TAG_LOGI(AAFwkTag::TEST,
             "AbilityManagerServiceFourteenthTest GetConnectManagerAndUIExtensionBySessionInfo_004 end");
}

/*
 * Feature: AbilityManagerService
 * Name: GetConnectManagerAndUIExtensionBySessionInfo_005
 * Function: GetConnectManagerAndUIExtensionBySessionInfo
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetConnectManagerAndUIExtensionBySessionInfo
 */
HWTEST_F(AbilityManagerServiceFourteenthTest, GetConnectManagerAndUIExtensionBySessionInfo_005, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST,
             "AbilityManagerServiceFourteenthTest GetConnectManagerAndUIExtensionBySessionInfo_005 start");
    MyStatus::GetInstance().smhGetConnectManagerByToken_ = true;
    MyStatus::GetInstance().acmGetUIExtensionBySessionInfo_ = nullptr;
    MyStatus::GetInstance().ipcGetCallingUid_ = -1;
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);

    auto mockSubManagersHelper = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    EXPECT_NE(mockSubManagersHelper, nullptr);
    auto mockCurrentConnectManager = std::make_shared<UIExtensionAbilityManager>(0);
    EXPECT_NE(mockCurrentConnectManager, nullptr);
    abilityMs_->subManagersHelper_ = mockSubManagersHelper;
    abilityMs_->subManagersHelper_->currentUIExtensionAbilityManager_ = mockCurrentConnectManager;

    std::shared_ptr<UIExtensionAbilityManager> connectManager = nullptr;
    std::shared_ptr<BaseExtensionRecord> targetAbility = nullptr;
    abilityMs_->GetConnectManagerAndUIExtensionBySessionInfo(MockSessionInfo(0), connectManager, targetAbility, false);
    EXPECT_NE(connectManager, nullptr);
    EXPECT_EQ(targetAbility, nullptr);
    TAG_LOGI(AAFwkTag::TEST,
             "AbilityManagerServiceFourteenthTest GetConnectManagerAndUIExtensionBySessionInfo_005 end");
}

/*
 * Feature: AbilityManagerService
 * Name: GetConnectManagerAndUIExtensionBySessionInfo_006
 * Function: GetConnectManagerAndUIExtensionBySessionInfo
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetConnectManagerAndUIExtensionBySessionInfo
 */
HWTEST_F(AbilityManagerServiceFourteenthTest, GetConnectManagerAndUIExtensionBySessionInfo_006, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST,
             "AbilityManagerServiceFourteenthTest GetConnectManagerAndUIExtensionBySessionInfo_006 start");
    MyStatus::GetInstance().smhGetConnectManagerByToken_ = false;
    MyStatus::GetInstance().acmGetUIExtensionBySessionInfo_ = nullptr;
    MyStatus::GetInstance().ipcGetCallingUid_ = -1;
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    std::shared_ptr<UIExtensionAbilityManager> connectManager = nullptr;
    std::shared_ptr<BaseExtensionRecord> targetAbility = nullptr;
    abilityMs_->GetConnectManagerAndUIExtensionBySessionInfo(MockSessionInfo(0), connectManager, targetAbility, false);
    EXPECT_EQ(connectManager, nullptr);
    EXPECT_EQ(targetAbility, nullptr);
    TAG_LOGI(AAFwkTag::TEST,
             "AbilityManagerServiceFourteenthTest GetConnectManagerAndUIExtensionBySessionInfo_006 end");
}

/*
 * Feature: AbilityManagerService
 * Name: TerminateUIExtensionAbility_001
 * Function: TerminateUIExtensionAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService TerminateUIExtensionAbility
 */
HWTEST_F(AbilityManagerServiceFourteenthTest, TerminateUIExtensionAbility_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourteenthTest TerminateUIExtensionAbility_001 start");
    MyStatus::GetInstance().smhGetConnectManagerByToken_ = false;
    MyStatus::GetInstance().acmGetUIExtensionBySessionInfo_ = nullptr;
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);

    int retCode = abilityMs_->TerminateUIExtensionAbility(MockSessionInfo(0), 0, nullptr);
    EXPECT_EQ(retCode, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourteenthTest TerminateUIExtensionAbility_001 end");
}

/*
 * Feature: AbilityManagerService
 * Name: IsDelegatorCall_001
 * Function: IsDelegatorCall
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService IsDelegatorCall
 */
HWTEST_F(AbilityManagerServiceFourteenthTest, IsDelegatorCall_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourteenthTest IsDelegatorCall_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    RunningProcessInfo processInfo;
    processInfo.isTestProcess = false;
    AbilityRequest abilityRequest;
    bool retCode = abilityMs_->IsDelegatorCall(processInfo, abilityRequest);
    EXPECT_FALSE(retCode);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourteenthTest IsDelegatorCall_001 end");
}

/*
 * Feature: AbilityManagerService
 * Name: IsTargetPermission_001
 * Function: IsTargetPermission
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService IsTargetPermission
 */
HWTEST_F(AbilityManagerServiceFourteenthTest, IsTargetPermission_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourteenthTest IsTargetPermission_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    Want want;
    ElementName element("", "com.ix.hiAccount", "AccountTest");
    want.SetElement(element);
    bool retCode = abilityMs_->IsTargetPermission(want);
    EXPECT_FALSE(retCode);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourteenthTest IsTargetPermission_001 end");
}

/*
 * Feature: AbilityManagerService
 * Name: CheckNewRuleSwitchState_001
 * Function: CheckNewRuleSwitchState
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckNewRuleSwitchState
 */
HWTEST_F(AbilityManagerServiceFourteenthTest, CheckNewRuleSwitchState_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourteenthTest CheckNewRuleSwitchState_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    bool retCode = abilityMs_->CheckNewRuleSwitchState("component.startup.newRules");
    EXPECT_TRUE(retCode);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourteenthTest CheckNewRuleSwitchState_001 end");
}

/*
 * Feature: AbilityManagerService
 * Name: CheckNewRuleSwitchState_002
 * Function: CheckNewRuleSwitchState
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckNewRuleSwitchState
 */
HWTEST_F(AbilityManagerServiceFourteenthTest, CheckNewRuleSwitchState_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourteenthTest CheckNewRuleSwitchState_002 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    bool retCode = abilityMs_->CheckNewRuleSwitchState("");
    EXPECT_FALSE(retCode);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourteenthTest CheckNewRuleSwitchState_002 end");
}

/*
 * Feature: AbilityManagerService
 * Name: CheckDlpForExtension_001
 * Function: CheckDlpForExtension
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckDlpForExtension
 */
HWTEST_F(AbilityManagerServiceFourteenthTest, CheckDlpForExtension_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourteenthTest CheckDlpForExtension_001 start");
    MyStatus::GetInstance().ipcGetCallingUid_ = AppExecFwk::Constants::FOUNDATION_UID;
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    Want want;
    want.SetBundle("com.ohos.formrenderservice");
    sptr<IRemoteObject> token = nullptr;
    EventInfo eventInfo;
    EventName eventName = static_cast<EventName>(-1);
    int retCode = abilityMs_->CheckDlpForExtension(want, token, 0, eventInfo, eventName);
    EXPECT_EQ(retCode, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourteenthTest CheckDlpForExtension_001 end");
}

/*
 * Feature: AbilityManagerService
 * Name: CheckDlpForExtension_002
 * Function: CheckDlpForExtension
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckDlpForExtension
 */
HWTEST_F(AbilityManagerServiceFourteenthTest, CheckDlpForExtension_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourteenthTest CheckDlpForExtension_002 start");
    MyStatus::GetInstance().ipcGetCallingUid_ = -1;
    MyStatus::GetInstance().permPermission_ = false;
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    Want want;
    want.SetBundle("com.example.abilityManagerServiceTest");
    sptr<IRemoteObject> token = nullptr;
    EventInfo eventInfo;
    EventName eventName = static_cast<EventName>(-1);
    int retCode = abilityMs_->CheckDlpForExtension(want, token, -1, eventInfo, eventName);
    EXPECT_EQ(retCode, CHECK_PERMISSION_FAILED);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourteenthTest CheckDlpForExtension_002 end");
}

/*
 * Feature: AbilityManagerService
 * Name: CheckDlpForExtension_003
 * Function: CheckDlpForExtension
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckDlpForExtension
 */
HWTEST_F(AbilityManagerServiceFourteenthTest, CheckDlpForExtension_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourteenthTest CheckDlpForExtension_003 start");
    MyStatus::GetInstance().ipcGetCallingUid_ = -1;
    MyStatus::GetInstance().permPermission_ = true;
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    Want want;
    want.SetBundle("com.example.abilityManagerServiceTest");
    sptr<IRemoteObject> token = nullptr;
    EventInfo eventInfo;
    EventName eventName = static_cast<EventName>(-1);
    int retCode = abilityMs_->CheckDlpForExtension(want, token, 0, eventInfo, eventName);
    EXPECT_EQ(retCode, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourteenthTest CheckDlpForExtension_003 end");
}

/*
 * Feature: AbilityManagerService
 * Name: GetFocusAbility_001
 * Function: GetFocusAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetFocusAbility
 */
HWTEST_F(AbilityManagerServiceFourteenthTest, GetFocusAbility_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourteenthTest GetFocusAbility_001 start");
    MyStatus::GetInstance().ipcGetCallingUid_ = AppExecFwk::Constants::FOUNDATION_UID;
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    abilityMs_->wmsHandler_ = nullptr;
    EXPECT_EQ(abilityMs_->GetFocusAbility(), nullptr);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourteenthTest GetFocusAbility_001 end");
}

/*
 * Feature: AbilityManagerService
 * Name: GetFocusAbility_002
 * Function: GetFocusAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetFocusAbility
 */
HWTEST_F(AbilityManagerServiceFourteenthTest, GetFocusAbility_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourteenthTest GetFocusAbility_002 start");
    MyStatus::GetInstance().ipcGetCallingUid_ = AppExecFwk::Constants::FOUNDATION_UID;
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    abilityMs_->wmsHandler_ = nullptr;
    EXPECT_EQ(abilityMs_->GetFocusAbility(), nullptr);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourteenthTest GetFocusAbility_002 end");
}

/*
 * Feature: AbilityManagerService
 * Name: CheckUIExtensionIsFocused_001
 * Function: CheckUIExtensionIsFocused
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckUIExtensionIsFocused
 */
HWTEST_F(AbilityManagerServiceFourteenthTest, CheckUIExtensionIsFocused_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourteenthTest CheckUIExtensionIsFocused_001 start");
    MyStatus::GetInstance().permPermission_ = 0;
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    sptr<IRemoteObject> token = nullptr;
    bool isFocused = false;
    int retCode = abilityMs_->CheckUIExtensionIsFocused(0, isFocused);
    EXPECT_EQ(retCode, CHECK_PERMISSION_FAILED);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourteenthTest CheckUIExtensionIsFocused_001 end");
}

/*
 * Feature: AbilityManagerService
 * Name: CheckExtensionRateLimit_001
 * Function: CheckExtensionRateLimit
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckExtensionRateLimit
 */
HWTEST_F(AbilityManagerServiceFourteenthTest, CheckExtensionRateLimit_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourteenthTest CheckExtensionRateLimit_001 start");
    
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    auto uid = 20010001;
    MyStatus::GetInstance().permPermission_ = 1;
    MyStatus::GetInstance().ipcGetCallingUid_ = uid;
    Want want;
    ElementName element("", "com.acts.testCheckExtensionRateLimit", "TestAbility");
    want.SetElement(element);
    for (int i = 0; i < EXTENSION_MAX_LIMIT + 1; i++) {
        abilityMs_->CheckExtensionRateLimit(want);
    }

    auto &rateLimiter = RateLimiter::GetInstance();
    auto isLimit = rateLimiter.CheckExtensionLimit(uid);
    EXPECT_EQ(isLimit.limited, false);
    
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourteenthTest CheckExtensionRateLimit_001 end");
}

/*
 * Feature: AbilityManagerService
 * Name: CheckExtensionRateLimit_002
 * Function: CheckExtensionRateLimit
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckExtensionRateLimit
 */
HWTEST_F(AbilityManagerServiceFourteenthTest, CheckExtensionRateLimit_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourteenthTest CheckExtensionRateLimit_002 start");
    
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    auto uid = 20010001;
    MyStatus::GetInstance().permPermission_ = 0;
    MyStatus::GetInstance().ipcGetCallingUid_ = uid;
    Want want;
    ElementName element("", "com.acts.testCheckExtensionRateLimit", "TestAbility");
    want.SetElement(element);
    for (int i = 0; i < EXTENSION_MAX_LIMIT + 1; i++) {
        abilityMs_->CheckExtensionRateLimit(want);
    }

    auto &rateLimiter = RateLimiter::GetInstance();
    auto isLimit = rateLimiter.CheckExtensionLimit(uid);
    EXPECT_EQ(isLimit.limited, true);

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourteenthTest CheckExtensionRateLimit_002 end");
}

/*
 * Feature: AbilityManagerService
 * Name: CheckExtensionRateLimit_003
 * Function: CheckExtensionRateLimit
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckExtensionRateLimit
 */
HWTEST_F(AbilityManagerServiceFourteenthTest, CheckExtensionRateLimit_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourteenthTest CheckExtensionRateLimit_003 start");
    
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    auto uid = 20010001;
    MyStatus::GetInstance().permPermission_ = 0;
    MyStatus::GetInstance().ipcGetCallingUid_ = uid;
    Want want;
    ElementName element("", "com.acts.testCheckExtensionRateLimit", "TestAbility");
    want.SetElement(element);
    for (int i = 0; i < EXTENSION_MAX_LIMIT + 1; i++) {
        abilityMs_->CheckExtensionRateLimit(want);
    }

    auto &rateLimiter = RateLimiter::GetInstance();
    auto isLimit = rateLimiter.CheckReportLimit(uid, 50);
    EXPECT_EQ(isLimit, true);

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourteenthTest CheckExtensionRateLimit_003 end");
}

/*
 * Feature: AbilityManagerService
 * Function: EnterKioskMode
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService EnterKioskMode
 */
HWTEST_F(AbilityManagerServiceFourteenthTest, EnterKioskMode_004, TestSize.Level1) {
    MyStatus::GetInstance().arGetAbilityRecord_ = nullptr;
    auto abilityManagerService = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityManagerService, nullptr);
    auto callerToken = MockToken(AbilityType::PAGE);
    ASSERT_NE(callerToken, nullptr);
    auto result = abilityManagerService->EnterKioskMode(callerToken);
    EXPECT_EQ(result, INVALID_PARAMETERS_ERR);
}

/*
 * Feature: AbilityManagerService
 * Function: ExitKioskMode
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ExitKioskMode
 */
HWTEST_F(AbilityManagerServiceFourteenthTest, ExitKioskMode_004, TestSize.Level1) {
    MyStatus::GetInstance().arGetAbilityRecord_ = nullptr;
    auto abilityManagerService = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityManagerService, nullptr);
    auto callerToken = MockToken(AbilityType::PAGE);
    ASSERT_NE(callerToken, nullptr);
    auto result = abilityManagerService->ExitKioskMode(callerToken);
    EXPECT_EQ(result, INVALID_PARAMETERS_ERR);
}

/*
 * Feature: AbilityManagerService
 * Function: NotifyStartupExceptionBySCB
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService NotifyStartupExceptionBySCB
 */
HWTEST_F(AbilityManagerServiceFourteenthTest, NotifyStartupExceptionBySCB_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourteenthTest NotifyStartupExceptionBySCB_001 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    int32_t requestId = 0;
    int32_t result = abilityMs->NotifyStartupExceptionBySCB(requestId);
    EXPECT_EQ(result, ERR_PERMISSION_DENIED);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourteenthTest NotifyStartupExceptionBySCB_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: NotifyStartupExceptionBySCB
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService NotifyStartupExceptionBySCB
 */
HWTEST_F(AbilityManagerServiceFourteenthTest, NotifyStartupExceptionBySCB_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourteenthTest NotifyStartupExceptionBySCB_002 start");
    MyStatus::GetInstance().smhGetConnectManagerByToken_ = true;
    MyStatus::GetInstance().ipcGetCallingTokenID_ = 1;

    auto abilityMs = std::make_shared<AbilityManagerService>();
    auto mockSubManagersHelper = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    EXPECT_NE(mockSubManagersHelper, nullptr);
    auto mockCurrentConnectManager = std::make_shared<CommonExtensionManager>(0);
    EXPECT_NE(mockCurrentConnectManager, nullptr);
    abilityMs->subManagersHelper_ = mockSubManagersHelper;
    mockCurrentConnectManager->sceneBoardTokenId_ = 1;
    abilityMs->subManagersHelper_->currentCommonExtensionManager_ = mockCurrentConnectManager;
    auto mockCurrentUIAbilityManager = std::make_shared<UIAbilityLifecycleManager>(0);
    abilityMs->subManagersHelper_->currentUIAbilityManager_ = mockCurrentUIAbilityManager;
    int32_t requestId = 0;
    int32_t result = abilityMs->NotifyStartupExceptionBySCB(requestId);
    EXPECT_EQ(result, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourteenthTest NotifyStartupExceptionBySCB_002 end");
}

/*
 * Feature: AbilityManagerService
 * Function: ProcessUdmfKey
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ProcessUdmfKey
 */
HWTEST_F(AbilityManagerServiceFourteenthTest, ProcessUdmfKey_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourteenthTest ProcessUdmfKey_001 start");
    Want want;
    want.SetParam("ability.want.params.udKey", std::string("udkey"));
    uint32_t targetTokenId = 1000001;
    AppExecFwk::ExtensionAbilityType extensionType = AppExecFwk::ExtensionAbilityType::SYSPICKER_FILEPICKER;
    auto abilityMs = std::make_shared<AbilityManagerService>();
    int32_t result = abilityMs->ProcessUdmfKey(want, targetTokenId, extensionType);
    EXPECT_EQ(result, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourteenthTest ProcessUdmfKey_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: ProcessUdmfKey
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ProcessUdmfKey
 */
HWTEST_F(AbilityManagerServiceFourteenthTest, ProcessUdmfKey_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourteenthTest ProcessUdmfKey_002 start");
    Want want;
    want.SetParam("ability.want.params.udKey", std::string("udkey"));
    uint32_t targetTokenId = 1000001;
    AppExecFwk::ExtensionAbilityType extensionType = AppExecFwk::ExtensionAbilityType::EMBEDDED_UI;
    auto abilityMs = std::make_shared<AbilityManagerService>();
    int32_t result = abilityMs->ProcessUdmfKey(want, targetTokenId, extensionType);
    EXPECT_EQ(result, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourteenthTest ProcessUdmfKey_002 end");
}

/*
 * Feature: AbilityManagerService
 * Function: ProcessUdmfKey
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ProcessUdmfKey
 */
HWTEST_F(AbilityManagerServiceFourteenthTest, ProcessUdmfKey_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourteenthTest ProcessUdmfKey_003 start");
    Want want;
    uint32_t targetTokenId = 1000001;
    AppExecFwk::ExtensionAbilityType extensionType = AppExecFwk::ExtensionAbilityType::SYSPICKER_FILEPICKER;
    auto abilityMs = std::make_shared<AbilityManagerService>();
    int32_t result = abilityMs->ProcessUdmfKey(want, targetTokenId, extensionType);
    EXPECT_EQ(result, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourteenthTest ProcessUdmfKey_003 end");
}

/*
 * Feature: AbilityManagerService
 * Function: ProcessUdmfKey
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ProcessUdmfKey
 */
HWTEST_F(AbilityManagerServiceFourteenthTest, ProcessUdmfKey_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourteenthTest ProcessUdmfKey_004 start");
    Want want;
    uint32_t targetTokenId = 1000001;
    AppExecFwk::ExtensionAbilityType extensionType = AppExecFwk::ExtensionAbilityType::EMBEDDED_UI;
    auto abilityMs = std::make_shared<AbilityManagerService>();
    int32_t result = abilityMs->ProcessUdmfKey(want, targetTokenId, extensionType);
    EXPECT_EQ(result, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourteenthTest ProcessUdmfKey_004 end");
}

/*
 * Feature: AbilityManagerService
 * Function: JudgeSystemParamsForPicker
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService JudgeSystemParamsForPicker
 */
HWTEST_F(AbilityManagerServiceFourteenthTest, JudgeSystemParamsForPicker_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourteenthTest JudgeSystemParamsForPicker_001 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    // 创建空的WantParams参数
    WantParams parameters;

    // 测试场景：参数中不包含SCREENCONFIG_SCREENMODE时，应返回true
    EXPECT_TRUE(abilityMs->JudgeSystemParamsForPicker(parameters));

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourteenthTest JudgeSystemParamsForPicker_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: JudgeSystemParamsForPicker
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService JudgeSystemParamsForPicker
 */
HWTEST_F(AbilityManagerServiceFourteenthTest, JudgeSystemParamsForPicker_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourteenthTest JudgeSystemParamsForPicker_002 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();

    WantParams parameters;

    // 模拟参数中包含SCREENCONFIG_SCREENMODE
    parameters.SetParam("ohos.verticalpanel.screenconfig.screenmode", nullptr);

    // Mock场景：模拟JudgeCallerIsAllowedToUseSystemAPI返回true
    // 测试场景：当调用者有权限使用系统API时，应返回true
    MyStatus::GetInstance().perJudgeCallerIsAllowedToUseSystemAPI_ = true;
    EXPECT_TRUE(abilityMs->JudgeSystemParamsForPicker(parameters));

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourteenthTest JudgeSystemParamsForPicker_002 end");
}

/*
 * Feature: AbilityManagerService
 * Function: JudgeSystemParamsForPicker
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService JudgeSystemParamsForPicker
 */
HWTEST_F(AbilityManagerServiceFourteenthTest, JudgeSystemParamsForPicker_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourteenthTest JudgeSystemParamsForPicker_003 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();

    WantParams parameters;

    // 模拟参数中包含SCREENCONFIG_SCREENMODE
    parameters.SetParam("ohos.verticalpanel.screenconfig.screenmode", nullptr);

    // Mock场景：模拟JudgeCallerIsAllowedToUseSystemAPI返回false
    // 测试场景：当调用者没有权限使用系统API时，应返回false
    MyStatus::GetInstance().perJudgeCallerIsAllowedToUseSystemAPI_ = false;
    EXPECT_FALSE(abilityMs->JudgeSystemParamsForPicker(parameters));
    MyStatus::GetInstance().perJudgeCallerIsAllowedToUseSystemAPI_ = true;

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourteenthTest JudgeSystemParamsForPicker_003 end");
}

/*
 * Feature: HandleShareDataTimeOut_001
 * Function: HandleShareDataTimeOut
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService HandleShareDataTimeOut
 * EnvConditions: no share data callbacks registered
 * CaseDescription: Expect ERR_INVALID_VALUE when share-data timeout cleanup is invoked without registered callbacks
 */
HWTEST_F(AbilityManagerServiceFourteenthTest, HandleShareDataTimeOut_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourteenthTest HandleShareDataTimeOut_001 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs, nullptr);
    abilityMs->HandleShareDataTimeOut(100);
    WantParams wantParams;
    int retCode = abilityMs->GetShareDataPairAndReturnData(nullptr, ERR_TIMED_OUT, 100, wantParams);
    EXPECT_EQ(retCode, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourteenthTest HandleShareDataTimeOut_001 end");
}

/*
 * Feature: OnAbilityDied_001
 * Function: OnAbilityDied
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService OnAbilityDied
 * EnvConditions: basic ability record creation
 * CaseDescription: Trigger OnAbilityDied to ensure the death path handles a valid ability record
 */
HWTEST_F(AbilityManagerServiceFourteenthTest, OnAbilityDied_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourteenthTest OnAbilityDied_001 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs, nullptr);
    auto abilityRecord = MockAbilityRecord(AbilityType::PAGE);
    ASSERT_NE(abilityRecord, nullptr);
    abilityMs->OnAbilityDied(abilityRecord);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourteenthTest OnAbilityDied_001 end");
}

/*
 * Feature: OnCallConnectDied_001
 * Function: OnCallConnectDied
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService OnCallConnectDied
 * EnvConditions: call record creation with null callback
 * CaseDescription: Verify the call-record death handler schedules OnCallConnectDied without crashing
 */
HWTEST_F(AbilityManagerServiceFourteenthTest, OnCallConnectDied_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourteenthTest OnCallConnectDied_001 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs, nullptr);
    auto abilityRecord = MockAbilityRecord(AbilityType::SERVICE);
    ASSERT_NE(abilityRecord, nullptr);
    auto callRecord = CallRecord::CreateCallRecord(0, abilityRecord, nullptr, nullptr);
    ASSERT_NE(callRecord, nullptr);
    abilityMs->OnCallConnectDied(callRecord);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourteenthTest OnCallConnectDied_001 end");
}

/*
 * Feature: GetConnectManagerByToken_001
 * Function: GetConnectManagerByToken
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetConnectManagerByToken
 * EnvConditions: null token and helper
 * CaseDescription: Expect nullptr when retrieving a connect manager without a helper or sub-managers
 */
HWTEST_F(AbilityManagerServiceFourteenthTest, GetConnectManagerByToken_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourteenthTest GetConnectManagerByToken_001 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs, nullptr);
    EXPECT_EQ(abilityMs->GetConnectManagerByToken(nullptr, ExtensionAbilityType::APP_SERVICE), nullptr);
    abilityMs->subManagersHelper_ = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    ASSERT_NE(abilityMs->subManagersHelper_, nullptr);
    EXPECT_EQ(abilityMs->GetConnectManagerByToken(nullptr, ExtensionAbilityType::EMBEDDED_UI), nullptr);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourteenthTest GetConnectManagerByToken_001 end");
}

/*
 * Feature: GetUIExtensionAbilityManagers_001
 * Function: GetUIExtensionAbilityManagers
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetUIExtensionAbilityManagers
 * EnvConditions: helper not initialized
 * CaseDescription: Ensure GetUIExtensionAbilityManagers returns an empty map
 */
HWTEST_F(AbilityManagerServiceFourteenthTest, GetUIExtensionAbilityManagers_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourteenthTest GetUIExtensionAbilityManagers_001 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs, nullptr);
    auto managers = abilityMs->GetUIExtensionAbilityManagers();
    EXPECT_TRUE(managers.empty());
    abilityMs->subManagersHelper_ = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    ASSERT_NE(abilityMs->subManagersHelper_, nullptr);
    auto managersWithHelper = abilityMs->GetUIExtensionAbilityManagers();
    EXPECT_TRUE(managersWithHelper.empty());
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourteenthTest GetUIExtensionAbilityManagers_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: CheckUIExtensionCallerPidByHostWindowId
 * FunctionPoints: AbilityManagerService CheckUIExtensionCallerPidByHostWindowId
 */
HWTEST_F(AbilityManagerServiceFourteenthTest, CheckUIExtensionCallerPidByHostWindowId_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourteenthTest CheckUIExtensionCallerPidByHostWindowId_001 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs, nullptr);
    AbilityRequest callerRequest = GenerateAbilityRequest("0", "abilityName", "appName", "bundleName", "moduleName");
    callerRequest.abilityInfo.extensionAbilityType = AppExecFwk::ExtensionAbilityType::SYS_COMMON_UI;
    auto callerRecord = AbilityRecord::CreateAbilityRecord(callerRequest);
    ASSERT_NE(callerRecord, nullptr);
    AppInfo info;
    info.userId = 0;
    abilityMs->OnAppStateChanged(info);
    info.userId = 1;
    abilityMs->OnAppStateChanged(info);
    AbilityRequest abilityRequest = GenerateAbilityRequest("0", "abilityName", "appName", "bundleName", "moduleName");
    abilityRequest.callerToken = callerRecord->GetToken();
    auto sessionInfo = sptr<SessionInfo>::MakeSptr();
    ASSERT_NE(sessionInfo, nullptr);
    sessionInfo->hostWindowId = 1;
    abilityRequest.sessionInfo = sessionInfo;
    auto result = abilityMs->CheckUIExtensionCallerPidByHostWindowId(abilityRequest, ERR_INVALID_VALUE);
    sleep(1);
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_TRUE(result);
    } else {
        EXPECT_FALSE(result);
    }
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourteenthTest CheckUIExtensionCallerPidByHostWindowId_001 end");
}

/**
 * @tc.name: LaunchGameCustomized_001
 * @tc.desc: Test LaunchGameCustomized with valid parameters.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceFourteenthTest, LaunchGameCustomized_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourteenthTest LaunchGameCustomized_001 start");
    std::string bundleName = "com.test.game";
    int32_t userId = 100;
    int32_t appIndex = 0;
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    auto result = abilityMs_->LaunchGameCustomized(bundleName, userId, appIndex);
    EXPECT_NE(result, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourteenthTest LaunchGameCustomized_001 end");
}

/**
 * @tc.name: SetGamePreLaunchCompleteTime_001
 * @tc.desc: Test SetGamePreLaunchCompleteTime with invalid permission (wrong UID).
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceFourteenthTest, SetGamePreLaunchCompleteTime_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourteenthTest SetGamePreLaunchCompleteTime_001 start");
    int32_t userId = 100;
    int64_t completeTime = 1000;
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    auto result = abilityMs_->SetGamePreLaunchCompleteTime(userId, completeTime);
    // Should fail due to permission check (unless running as GAME_SA_UID)
    if (IPCSkeleton::GetCallingUid() != GAME_SA_UID) {
        EXPECT_EQ(result, CHECK_PERMISSION_FAILED);
    } else {
        EXPECT_EQ(result, ERR_OK);
    }
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourteenthTest SetGamePreLaunchCompleteTime_001 end");
}

/**
 * @tc.name: NotifyCancelGamePreLaunch_001
 * @tc.desc: Test NotifyCancelGamePreLaunch when scene board is not enabled.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceFourteenthTest, NotifyCancelGamePreLaunch_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourteenthTest NotifyCancelGamePreLaunch_001 start");
    sptr<IRemoteObject> callerToken = nullptr;
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    auto result = abilityMs_->NotifyCancelGamePreLaunch(callerToken);
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_EQ(result, ERR_CAPABILITY_NOT_SUPPORT);
    } else {
        EXPECT_EQ(result, ERR_INVALID_VALUE);
    }
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourteenthTest NotifyCancelGamePreLaunch_001 end");
}

/**
 * @tc.name: NotifyCompleteGamePreLaunch_001
 * @tc.desc: Test NotifyCompleteGamePreLaunch when scene board is not enabled.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceFourteenthTest, NotifyCompleteGamePreLaunch_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourteenthTest NotifyCompleteGamePreLaunch_001 start");
    sptr<IRemoteObject> callerToken = nullptr;
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    auto result = abilityMs_->NotifyCompleteGamePreLaunch(callerToken);
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_EQ(result, ERR_CAPABILITY_NOT_SUPPORT);
    } else {
        EXPECT_EQ(result, ERR_INVALID_VALUE);
    }
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourteenthTest NotifyCompleteGamePreLaunch_001 end");
}
} // namespace AAFwk
} // namespace OHOS
