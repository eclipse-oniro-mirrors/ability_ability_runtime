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

#include "ability_manager_service.h"
#include "global_constant.h"
#include "hilog_tag_wrapper.h"
#include "insight_intent_db_cache.h"
#include "insight_intent_execute_manager.h"
#include "mission_list_manager.h"
#include "mock_my_status.h"
#include "sub_managers_helper.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AppExecFwk;
using OHOS::AppExecFwk::AbilityType;
using OHOS::AppExecFwk::ExtensionAbilityType;

constexpr char DEVELOPER_MODE_STATE[] = "const.security.developermode.state";
constexpr const char* DEBUG_APP = "debugApp";
constexpr const char* DLP_PARAMS_SANDBOX = "ohos.dlp.params.sandbox";
constexpr const char* DLP_INDEX = "ohos.dlp.params.index";
constexpr const char* START_ABILITY_TYPE = "ABILITY_INNER_START_WITH_ACCOUNT";
constexpr const char* AUTO_STARTUP_READY = "persist.sys.abilityms.if_startup_ready";
constexpr int32_t ONE = 1;
constexpr int32_t TWO = 2;
constexpr int32_t FOUNDATION_UID = 5523;
constexpr int32_t DMS_UID = 5522;
constexpr int32_t LOW_MEMORY_KILL_WHILE_STARTING = 1111;
constexpr int32_t DEFAULT_INVAL_VALUE = -1;
constexpr int32_t PENG_LAI_UID = 7655;
constexpr int32_t BROKER_UID = 5557;

namespace OHOS {
namespace AAFwk {
class AbilityManagerServiceThirteenthTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    std::shared_ptr<AbilityRecord> MockAbilityRecord(AbilityType);
    sptr<Token> MockToken(AbilityType);
};

class MockISAInterceptor : public AbilityRuntime::ISAInterceptor {
public:
    int32_t OnCheckStarting(const std::string& params, Rule& rule) override { return 0; };
    sptr<IRemoteObject> AsObject() override
    {
        return nullptr;
    }
};

void AbilityManagerServiceThirteenthTest::SetUpTestCase() {}

void AbilityManagerServiceThirteenthTest::TearDownTestCase() {}

void AbilityManagerServiceThirteenthTest::SetUp() {}

void AbilityManagerServiceThirteenthTest::TearDown() {}
std::shared_ptr<AbilityRecord> AbilityManagerServiceThirteenthTest::MockAbilityRecord(AbilityType abilityType)
{
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.test.demo";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = abilityType;
    return AbilityRecord::CreateAbilityRecord(abilityRequest);
}

sptr<Token> AbilityManagerServiceThirteenthTest::MockToken(AbilityType abilityType)
{
    std::shared_ptr<AbilityRecord> abilityRecord = MockAbilityRecord(abilityType);
    if (!abilityRecord) {
        return nullptr;
    }
    return abilityRecord->GetToken();
}

#ifdef SUPPORT_RECORDER_DSOFTBUS
/*
 * Feature: AbilityManagerService
 * Name: ConnectAbilityCommon_001
 * Function: ConnectAbilityCommon
 * SubFunction: NA
 * FunctionPoints: Verify ConnectAbilityCommon forwards AGENT extension type to ConnectFreeInstall
 */
HWTEST_F(AbilityManagerServiceThirteenthTest, ConnectAbilityCommon_001, TestSize.Level1)
{
    auto &status = MyStatus::GetInstance();
    status.fimConnectFreeInstallCalled_ = false;
    status.fimConnectFreeInstall_ = CHECK_PERMISSION_FAILED;
    status.fimConnectExtensionType_ = -1;
    status.fimConnectLocalDeviceId_.clear();
    status.softbusGetLocalNodeDeviceInfo_ = ERR_OK;

    auto abilityMs = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs, nullptr);
    abilityMs->interceptorExecuter_ = std::make_shared<AbilityInterceptorExecuter>();
    abilityMs->freeInstallManager_ = std::make_shared<FreeInstallManager>();
    ASSERT_NE(abilityMs->freeInstallManager_, nullptr);

    Want want;
    ElementName element("", "com.test.demo", "MainAbility");
    want.SetElement(element);
    want.AddFlags(Want::FLAG_INSTALL_ON_DEMAND);
    auto connect = sptr<InsightIntentExecuteConnection>::MakeSptr();
    auto callerToken = MockToken(AbilityType::PAGE);

    auto ret = abilityMs->ConnectAbilityCommon(
        want, connect, callerToken, ExtensionAbilityType::AGENT, DEFAULT_INVAL_VALUE, false);
    EXPECT_EQ(ret, CHECK_PERMISSION_FAILED);
    EXPECT_TRUE(status.fimConnectFreeInstallCalled_);
    EXPECT_EQ(status.fimConnectExtensionType_, static_cast<int32_t>(ExtensionAbilityType::AGENT));
    EXPECT_EQ(status.fimConnectLocalDeviceId_, "0");
}
#endif // SUPPORT_RECORDER_DSOFTBUS

/*
 * Feature: AbilityManagerService
 * Name: FreeInstallAbilityFromRemote_001
 * Function: FreeInstallAbilityFromRemote
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService FreeInstallAbilityFromRemote
 */
HWTEST_F(AbilityManagerServiceThirteenthTest, FreeInstallAbilityFromRemote_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest FreeInstallAbilityFromRemote_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    MyStatus::GetInstance().ipcGetCallingUid_ = FOUNDATION_UID;
    Want want;
    sptr<IRemoteObject> callback = nullptr;
    int retCode = abilityMs_->FreeInstallAbilityFromRemote(want, callback, 0, 0);
    EXPECT_EQ(retCode, CHECK_PERMISSION_FAILED);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest FreeInstallAbilityFromRemote_001 end");
}

/*
 * Feature: AbilityManagerService
 * Name: FreeInstallAbilityFromRemote_002
 * Function: FreeInstallAbilityFromRemote
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService FreeInstallAbilityFromRemote
 */
HWTEST_F(AbilityManagerServiceThirteenthTest, FreeInstallAbilityFromRemote_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest FreeInstallAbilityFromRemote_002 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    MyStatus::GetInstance().ipcGetCallingUid_ = DMS_UID;
    abilityMs_->freeInstallManager_ = nullptr;
    Want want;
    sptr<IRemoteObject> callback = nullptr;
    int retCode = abilityMs_->FreeInstallAbilityFromRemote(want, callback, 0, 0);
    EXPECT_EQ(retCode, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest FreeInstallAbilityFromRemote_002 end");
}

/*
 * Feature: AbilityManagerService
 * Name: FreeInstallAbilityFromRemote_003
 * Function: FreeInstallAbilityFromRemote
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService FreeInstallAbilityFromRemote
 */
HWTEST_F(AbilityManagerServiceThirteenthTest, FreeInstallAbilityFromRemote_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest FreeInstallAbilityFromRemote_003 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    MyStatus::GetInstance().ipcGetCallingUid_ = DMS_UID;
    Want want;
    sptr<IRemoteObject> callback = nullptr;
    int retCode = abilityMs_->FreeInstallAbilityFromRemote(want, callback, 0, 0);
    EXPECT_EQ(retCode, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest FreeInstallAbilityFromRemote_003 end");
}

/*
 * Feature: AbilityManagerService
 * Name: SetMissionContinueState_001
 * Function: SetMissionContinueState
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService SetMissionContinueState
 */
HWTEST_F(AbilityManagerServiceThirteenthTest, SetMissionContinueState_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest SetMissionContinueState_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);

    sptr<IRemoteObject> token = nullptr;
    AAFwk::ContinueState state = AAFwk::ContinueState::CONTINUESTATE_UNKNOWN;
    int result = abilityMs_->SetMissionContinueState(token, state);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest SetMissionContinueState_001 end");
}

/*
 * Feature: AbilityManagerService
 * Name: SetMissionContinueState_002
 * Function: SetMissionContinueState
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService SetMissionContinueState
 */
HWTEST_F(AbilityManagerServiceThirteenthTest, SetMissionContinueState_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest SetMissionContinueState_002 start");
    
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    
    sptr<IRemoteObject> token = MockToken(AbilityType::PAGE);
    AAFwk::ContinueState state = AAFwk::ContinueState::CONTINUESTATE_UNKNOWN;
    int result = abilityMs_->SetMissionContinueState(token, state);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest SetMissionContinueState_002 end");
}

/*
 * Feature: AbilityManagerService
 * Name: CheckFileAccessExtensionPermission_001
 * Function: CheckFileAccessExtensionPermission
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckFileAccessExtensionPermission
 */
HWTEST_F(AbilityManagerServiceThirteenthTest, CheckFileAccessExtensionPermission_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest CheckFileAccessExtensionPermission_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    MyStatus::GetInstance().permPermission_ = 0;
    AbilityRequest abilityRequest;
    int result = abilityMs_->CheckFileAccessExtensionPermission(abilityRequest);
    EXPECT_EQ(result, CHECK_PERMISSION_FAILED);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest CheckFileAccessExtensionPermission_001 end");
}

/*
 * Feature: AbilityManagerService
 * Name: CheckFileAccessExtensionPermission_002
 * Function: CheckFileAccessExtensionPermission
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckFileAccessExtensionPermission
 */
HWTEST_F(AbilityManagerServiceThirteenthTest, CheckFileAccessExtensionPermission_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest CheckFileAccessExtensionPermission_002 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    MyStatus::GetInstance().permPermission_ = CHECK_PERMISSION_FAILED;
    AbilityRequest abilityRequest;
    int result = abilityMs_->CheckFileAccessExtensionPermission(abilityRequest);
    EXPECT_EQ(result, CHECK_PERMISSION_FAILED);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest CheckFileAccessExtensionPermission_002 end");
}

/*
 * Feature: AbilityManagerService
 * Name: CheckCallerInfoQueryExtensionPermission_001
 * Function: CheckCallerInfoQueryExtensionPermission
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckCallerInfoQueryExtensionPermission
 */
HWTEST_F(AbilityManagerServiceThirteenthTest, CheckCallerInfoQueryExtensionPermission_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest CheckCallerInfoQueryExtensionPermission_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    MyStatus::GetInstance().permPermission_ = 0;
    AbilityRequest abilityRequest;
    int result = abilityMs_->CheckCallerInfoQueryExtensionPermission(abilityRequest);
    EXPECT_EQ(result, CHECK_PERMISSION_FAILED);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest CheckCallerInfoQueryExtensionPermission_001 end");
}

/*
 * Feature: AbilityManagerService
 * Name: CheckCallerInfoQueryExtensionPermission_002
 * Function: CheckCallerInfoQueryExtensionPermission
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckCallerInfoQueryExtensionPermission
 */
HWTEST_F(AbilityManagerServiceThirteenthTest, CheckCallerInfoQueryExtensionPermission_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest CheckCallerInfoQueryExtensionPermission_002 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    MyStatus::GetInstance().permPermission_ = 1;
    AbilityRequest abilityRequest;
    int result = abilityMs_->CheckCallerInfoQueryExtensionPermission(abilityRequest);
    EXPECT_EQ(result, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest CheckCallerInfoQueryExtensionPermission_002 end");
}

/*
 * Feature: AbilityManagerService
 * Name: CheckUIExtensionCallerIsForeground_001
 * Function: CheckUIExtensionCallerIsForeground
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckUIExtensionCallerIsForeground
 */
HWTEST_F(AbilityManagerServiceThirteenthTest, CheckUIExtensionCallerIsForeground_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest CheckUIExtensionCallerIsForeground_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    AbilityRequest abilityRequest;
    int result = abilityMs_->CheckUIExtensionCallerIsForeground(abilityRequest);
    EXPECT_TRUE(result);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest CheckUIExtensionCallerIsForeground_001 end");
}

/*
 * Feature: AbilityManagerService
 * Name: CheckUIExtensionCallerIsForeground_002
 * Function: CheckUIExtensionCallerIsForeground
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckUIExtensionCallerIsForeground
 */
HWTEST_F(AbilityManagerServiceThirteenthTest, CheckUIExtensionCallerIsForeground_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest CheckUIExtensionCallerIsForeground_002 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.test.demo";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = AbilityType::PAGE;
    MyStatus::GetInstance().permPermission_ = 0;
    int result = abilityMs_->CheckUIExtensionCallerIsForeground(abilityRequest);
    EXPECT_TRUE(result);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest CheckUIExtensionCallerIsForeground_002 end");
}

/*
 * Feature: AbilityManagerService
 * Name: CheckUIExtensionCallerIsForeground_003
 * Function: CheckUIExtensionCallerIsForeground
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckUIExtensionCallerIsForeground
 */
HWTEST_F(AbilityManagerServiceThirteenthTest, CheckUIExtensionCallerIsForeground_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest CheckUIExtensionCallerIsForeground_003 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.test.demo";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = AbilityType::PAGE;
    MyStatus::GetInstance().permPermission_ = 1;
    int result = abilityMs_->CheckUIExtensionCallerIsForeground(abilityRequest);
    EXPECT_TRUE(result);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest CheckUIExtensionCallerIsForeground_003 end");
}

/*
 * Feature: AbilityManagerService
 * Name: CheckDebugAssertPermission_001
 * Function: CheckDebugAssertPermission
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckDebugAssertPermission
 */
HWTEST_F(AbilityManagerServiceThirteenthTest, CheckDebugAssertPermission_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest CheckDebugAssertPermission_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    MyStatus::GetInstance().paramGetBoolParameter_ = false;
    int32_t result = abilityMs_->CheckDebugAssertPermission();
    EXPECT_EQ(result, ERR_NOT_SUPPORTED_PRODUCT_TYPE);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest CheckDebugAssertPermission_001 end");
}

/*
 * Feature: AbilityManagerService
 * Name: CheckDebugAssertPermission_002
 * Function: CheckDebugAssertPermission
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckDebugAssertPermission
 */
HWTEST_F(AbilityManagerServiceThirteenthTest, CheckDebugAssertPermission_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest CheckDebugAssertPermission_002 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    MyStatus::GetInstance().paramGetBoolParameter_ = true;
    int32_t result = abilityMs_->CheckDebugAssertPermission();
    EXPECT_NE(result, ERR_NOT_SUPPORTED_PRODUCT_TYPE);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest CheckDebugAssertPermission_002 end");
}

/*
 * Feature: AbilityManagerService
 * Name: IsEmbeddedOpenAllowed_001
 * Function: IsEmbeddedOpenAllowed
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService IsEmbeddedOpenAllowed
 */
HWTEST_F(AbilityManagerServiceThirteenthTest, IsEmbeddedOpenAllowed_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest IsEmbeddedOpenAllowed_001 start");
    MyStatus::GetInstance().auIsLaunchEmbededUIAbility_ = false;

    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    sptr<IRemoteObject> callerToken;
    std::string appId = "";
    EXPECT_FALSE(abilityMs_->IsEmbeddedOpenAllowed(callerToken, appId));
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest IsEmbeddedOpenAllowed_001 end");
}

/*
 * Feature: AbilityManagerService
 * Name: IsEmbeddedOpenAllowed_002
 * Function: IsEmbeddedOpenAllowed
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService IsEmbeddedOpenAllowed
 */
HWTEST_F(AbilityManagerServiceThirteenthTest, IsEmbeddedOpenAllowed_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest IsEmbeddedOpenAllowed_002 start");
    MyStatus::GetInstance().auIsLaunchEmbededUIAbility_ = true;
    MyStatus::GetInstance().atkGetTokenTypeFlag_ = -1;

    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    sptr<IRemoteObject> callerToken;
    std::string appId = "";
    EXPECT_FALSE(abilityMs_->IsEmbeddedOpenAllowed(callerToken, appId));
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest IsEmbeddedOpenAllowed_002 end");
}

/*
 * Feature: AbilityManagerService
 * Name: IsEmbeddedOpenAllowed_003
 * Function: IsEmbeddedOpenAllowed
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService IsEmbeddedOpenAllowed
 */
HWTEST_F(AbilityManagerServiceThirteenthTest, IsEmbeddedOpenAllowed_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest IsEmbeddedOpenAllowed_003 start");
    MyStatus::GetInstance().auIsLaunchEmbededUIAbility_ = true;
    MyStatus::GetInstance().atkGetTokenTypeFlag_ = 0;
    MyStatus::GetInstance().smhGetUIAbilityManagerByUid_ = true;
    MyStatus::GetInstance().fimStartFreeInstall_ = ERR_OK;
    MyStatus::GetInstance().arGetAbilityInfo_.applicationInfo.accessTokenId = 1;
    Want want;
    OHOS::AppExecFwk::AbilityInfo abilityInfo;
    OHOS::AppExecFwk::ApplicationInfo applicationInfo;
    auto record = std::make_shared<UIAbilityRecord>(want, abilityInfo, applicationInfo, -1);
    EXPECT_NE(record, nullptr);
    record->currentState_ = AbilityState::FOREGROUND;
    record->isAbilityForegrounding_ = true;
    MyStatus::GetInstance().ualmGetAbilityRecordByToken_ = record;

    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    auto mockSubManagersHelper = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    EXPECT_NE(mockSubManagersHelper, nullptr);
    auto mockCurrentUIAbilityManager = std::make_shared<UIAbilityLifecycleManager>(0);
    EXPECT_NE(mockCurrentUIAbilityManager, nullptr);
    auto mockFreeInstallManager = std::make_shared<FreeInstallManager>();
    EXPECT_NE(mockFreeInstallManager, nullptr);
    abilityMs_->subManagersHelper_ = mockSubManagersHelper;
    abilityMs_->subManagersHelper_->currentUIAbilityManager_ = mockCurrentUIAbilityManager;
    abilityMs_->freeInstallManager_ = mockFreeInstallManager;

    sptr<IRemoteObject> callerToken;
    std::string appId = "";
    EXPECT_TRUE(abilityMs_->IsEmbeddedOpenAllowed(callerToken, appId));
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest IsEmbeddedOpenAllowed_003 end");
}

/*
 * Feature: AbilityManagerService
 * Name: IsEmbeddedOpenAllowed_004
 * Function: IsEmbeddedOpenAllowed
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService IsEmbeddedOpenAllowed
 */
HWTEST_F(AbilityManagerServiceThirteenthTest, IsEmbeddedOpenAllowed_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest IsEmbeddedOpenAllowed_004 start");
    MyStatus::GetInstance().auIsLaunchEmbededUIAbility_ = true;
    MyStatus::GetInstance().atkGetTokenTypeFlag_ = 0;
    MyStatus::GetInstance().smhGetUIAbilityManagerByUid_ = true;
    MyStatus::GetInstance().ualmGetAbilityRecordByToken_ = nullptr;

    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    auto mockSubManagersHelper = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    EXPECT_NE(mockSubManagersHelper, nullptr);
    auto mockCurrentUIAbilityManager = std::make_shared<UIAbilityLifecycleManager>(0);
    EXPECT_NE(mockCurrentUIAbilityManager, nullptr);
    abilityMs_->subManagersHelper_ = mockSubManagersHelper;
    abilityMs_->subManagersHelper_->currentUIAbilityManager_ = mockCurrentUIAbilityManager;

    sptr<IRemoteObject> callerToken;
    std::string appId = "";
    EXPECT_FALSE(abilityMs_->IsEmbeddedOpenAllowed(callerToken, appId));
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest IsEmbeddedOpenAllowed_004 end");
}

/*
 * Feature: AbilityManagerService
 * Name: IsEmbeddedOpenAllowed_005
 * Function: IsEmbeddedOpenAllowed
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService IsEmbeddedOpenAllowed
 */
HWTEST_F(AbilityManagerServiceThirteenthTest, IsEmbeddedOpenAllowed_005, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest IsEmbeddedOpenAllowed_005 start");
    MyStatus::GetInstance().auIsLaunchEmbededUIAbility_ = true;
    MyStatus::GetInstance().atkGetTokenTypeFlag_ = 0;
    MyStatus::GetInstance().smhGetUIAbilityManagerByUid_ = true;
    MyStatus::GetInstance().arGetAbilityInfo_.applicationInfo.accessTokenId = -1;
    Want want;
    OHOS::AppExecFwk::AbilityInfo abilityInfo;
    OHOS::AppExecFwk::ApplicationInfo applicationInfo;
    auto record = std::make_shared<UIAbilityRecord>(want, abilityInfo, applicationInfo, -1);
    EXPECT_NE(record, nullptr);
    MyStatus::GetInstance().ualmGetAbilityRecordByToken_ = record;

    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    auto mockSubManagersHelper = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    EXPECT_NE(mockSubManagersHelper, nullptr);
    auto mockCurrentUIAbilityManager = std::make_shared<UIAbilityLifecycleManager>(0);
    EXPECT_NE(mockCurrentUIAbilityManager, nullptr);
    abilityMs_->subManagersHelper_ = mockSubManagersHelper;
    abilityMs_->subManagersHelper_->currentUIAbilityManager_ = mockCurrentUIAbilityManager;

    sptr<IRemoteObject> callerToken;
    std::string appId = "";
    EXPECT_FALSE(abilityMs_->IsEmbeddedOpenAllowed(callerToken, appId));
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest IsEmbeddedOpenAllowed_005 end");
}

/*
 * Feature: AbilityManagerService
 * Name: IsEmbeddedOpenAllowed_006
 * Function: IsEmbeddedOpenAllowed
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService IsEmbeddedOpenAllowed
 */
HWTEST_F(AbilityManagerServiceThirteenthTest, IsEmbeddedOpenAllowed_006, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest IsEmbeddedOpenAllowed_006 start");
    MyStatus::GetInstance().auIsLaunchEmbededUIAbility_ = true;
    MyStatus::GetInstance().atkGetTokenTypeFlag_ = 0;
    MyStatus::GetInstance().smhGetUIAbilityManagerByUid_ = true;
    MyStatus::GetInstance().arGetAbilityInfo_.applicationInfo.accessTokenId = 1;
    Want want;
    OHOS::AppExecFwk::AbilityInfo abilityInfo;
    OHOS::AppExecFwk::ApplicationInfo applicationInfo;
    auto record = std::make_shared<UIAbilityRecord>(want, abilityInfo, applicationInfo, -1);
    EXPECT_NE(record, nullptr);
    record->currentState_ = AbilityState::BACKGROUND;
    record->isAbilityForegrounding_ = false;
    MyStatus::GetInstance().ualmGetAbilityRecordByToken_ = record;

    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    auto mockSubManagersHelper = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    EXPECT_NE(mockSubManagersHelper, nullptr);
    auto mockCurrentUIAbilityManager = std::make_shared<UIAbilityLifecycleManager>(0);
    EXPECT_NE(mockCurrentUIAbilityManager, nullptr);
    abilityMs_->subManagersHelper_ = mockSubManagersHelper;
    abilityMs_->subManagersHelper_->currentUIAbilityManager_ = mockCurrentUIAbilityManager;

    sptr<IRemoteObject> callerToken;
    std::string appId = "";
    EXPECT_FALSE(abilityMs_->IsEmbeddedOpenAllowed(callerToken, appId));
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest IsEmbeddedOpenAllowed_006 end");
}

/*
 * Feature: AbilityManagerService
 * Name: IsEmbeddedOpenAllowed_007
 * Function: IsEmbeddedOpenAllowed
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService IsEmbeddedOpenAllowed
 */
HWTEST_F(AbilityManagerServiceThirteenthTest, IsEmbeddedOpenAllowed_007, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest IsEmbeddedOpenAllowed_007 start");
    MyStatus::GetInstance().auIsLaunchEmbededUIAbility_ = true;
    MyStatus::GetInstance().atkGetTokenTypeFlag_ = 0;
    MyStatus::GetInstance().smhGetUIAbilityManagerByUid_ = true;
    MyStatus::GetInstance().arGetAbilityInfo_.applicationInfo.accessTokenId = 1;
    Want want;
    OHOS::AppExecFwk::AbilityInfo abilityInfo;
    OHOS::AppExecFwk::ApplicationInfo applicationInfo;
    auto record = std::make_shared<UIAbilityRecord>(want, abilityInfo, applicationInfo, -1);
    EXPECT_NE(record, nullptr);
    record->currentState_ = AbilityState::FOREGROUND;
    record->isAbilityForegrounding_ = false;
    MyStatus::GetInstance().ualmGetAbilityRecordByToken_ = record;

    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    auto mockSubManagersHelper = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    EXPECT_NE(mockSubManagersHelper, nullptr);
    auto mockCurrentUIAbilityManager = std::make_shared<UIAbilityLifecycleManager>(0);
    EXPECT_NE(mockCurrentUIAbilityManager, nullptr);
    abilityMs_->subManagersHelper_ = mockSubManagersHelper;
    abilityMs_->subManagersHelper_->currentUIAbilityManager_ = mockCurrentUIAbilityManager;

    sptr<IRemoteObject> callerToken;
    std::string appId = "";
    EXPECT_FALSE(abilityMs_->IsEmbeddedOpenAllowed(callerToken, appId));
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest IsEmbeddedOpenAllowed_007 end");
}

/*
 * Feature: AbilityManagerService
 * Name: IsEmbeddedOpenAllowed_008
 * Function: IsEmbeddedOpenAllowed
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService IsEmbeddedOpenAllowed
 */
HWTEST_F(AbilityManagerServiceThirteenthTest, IsEmbeddedOpenAllowed_008, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest IsEmbeddedOpenAllowed_008 start");
    MyStatus::GetInstance().auIsLaunchEmbededUIAbility_ = true;
    MyStatus::GetInstance().atkGetTokenTypeFlag_ = 0;
    MyStatus::GetInstance().smhGetUIAbilityManagerByUid_ = true;
    MyStatus::GetInstance().arGetAbilityInfo_.applicationInfo.accessTokenId = 1;
    Want want;
    OHOS::AppExecFwk::AbilityInfo abilityInfo;
    OHOS::AppExecFwk::ApplicationInfo applicationInfo;
    auto record = std::make_shared<UIAbilityRecord>(want, abilityInfo, applicationInfo, -1);
    EXPECT_NE(record, nullptr);
    record->currentState_ = AbilityState::FOREGROUND;
    record->isAbilityForegrounding_ = true;
    MyStatus::GetInstance().ualmGetAbilityRecordByToken_ = record;

    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    auto mockSubManagersHelper = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    EXPECT_NE(mockSubManagersHelper, nullptr);
    auto mockCurrentUIAbilityManager = std::make_shared<UIAbilityLifecycleManager>(0);
    EXPECT_NE(mockCurrentUIAbilityManager, nullptr);
    abilityMs_->subManagersHelper_ = mockSubManagersHelper;
    abilityMs_->subManagersHelper_->currentUIAbilityManager_ = mockCurrentUIAbilityManager;
    abilityMs_->freeInstallManager_ = nullptr;
    sptr<IRemoteObject> callerToken;
    std::string appId = "";
    EXPECT_FALSE(abilityMs_->IsEmbeddedOpenAllowed(callerToken, appId));
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest IsEmbeddedOpenAllowed_008 end");
}

/*
 * Feature: AbilityManagerService
 * Name: IsEmbeddedOpenAllowed_009
 * Function: IsEmbeddedOpenAllowed
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService IsEmbeddedOpenAllowed
 */
HWTEST_F(AbilityManagerServiceThirteenthTest, IsEmbeddedOpenAllowed_009, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest IsEmbeddedOpenAllowed_009 start");
    MyStatus::GetInstance().auIsLaunchEmbededUIAbility_ = true;
    MyStatus::GetInstance().atkGetTokenTypeFlag_ = 0;
    MyStatus::GetInstance().smhGetUIAbilityManagerByUid_ = true;
    MyStatus::GetInstance().fimStartFreeInstall_ = ERR_INVALID_VALUE;
    MyStatus::GetInstance().arGetAbilityInfo_.applicationInfo.accessTokenId = 1;
    Want want;
    OHOS::AppExecFwk::AbilityInfo abilityInfo;
    OHOS::AppExecFwk::ApplicationInfo applicationInfo;
    auto record = std::make_shared<UIAbilityRecord>(want, abilityInfo, applicationInfo, -1);
    EXPECT_NE(record, nullptr);
    record->currentState_ = AbilityState::FOREGROUND;
    record->isAbilityForegrounding_ = true;
    MyStatus::GetInstance().ualmGetAbilityRecordByToken_ = record;

    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    auto mockSubManagersHelper = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    EXPECT_NE(mockSubManagersHelper, nullptr);
    auto mockCurrentUIAbilityManager = std::make_shared<UIAbilityLifecycleManager>(0);
    EXPECT_NE(mockCurrentUIAbilityManager, nullptr);
    auto mockFreeInstallManager = std::make_shared<FreeInstallManager>();
    EXPECT_NE(mockFreeInstallManager, nullptr);
    abilityMs_->subManagersHelper_ = mockSubManagersHelper;
    abilityMs_->subManagersHelper_->currentUIAbilityManager_ = mockCurrentUIAbilityManager;
    abilityMs_->freeInstallManager_ = mockFreeInstallManager;

    sptr<IRemoteObject> callerToken;
    std::string appId = "";
    EXPECT_FALSE(abilityMs_->IsEmbeddedOpenAllowed(callerToken, appId));
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest IsEmbeddedOpenAllowed_009 end");
}

/*
 * Feature: AbilityManagerService
 * Name: QueryAtomicServiceStartupRule_001
 * Function: QueryAtomicServiceStartupRule
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService QueryAtomicServiceStartupRule
 */
HWTEST_F(AbilityManagerServiceThirteenthTest, QueryAtomicServiceStartupRule_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest QueryAtomicServiceStartupRule_001 start");

    MyStatus::GetInstance().auIsLaunchEmbededUIAbility_ = false;
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);

    sptr<IRemoteObject> callerToken;
    std::string appId = "";
    const std::string startTime = "";
    AtomicServiceStartupRule rule;
    EXPECT_EQ(abilityMs_->QueryAtomicServiceStartupRule(callerToken, appId, startTime, rule),
              ERR_CAPABILITY_NOT_SUPPORT);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest QueryAtomicServiceStartupRule_001 end");
}

/*
 * Feature: AbilityManagerService
 * Name: QueryAtomicServiceStartupRule_002
 * Function: QueryAtomicServiceStartupRule
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService QueryAtomicServiceStartupRule
 */
HWTEST_F(AbilityManagerServiceThirteenthTest, QueryAtomicServiceStartupRule_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest QueryAtomicServiceStartupRule_002 start");

    MyStatus::GetInstance().auIsLaunchEmbededUIAbility_ = true;
    MyStatus::GetInstance().atkGetTokenTypeFlag_ = -1; // 0: TOKEN_HAP

    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);

    sptr<IRemoteObject> callerToken;
    std::string appId = "";
    const std::string startTime = "";
    AtomicServiceStartupRule rule;
    EXPECT_EQ(abilityMs_->QueryAtomicServiceStartupRule(callerToken, appId, startTime, rule), INNER_ERR);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest QueryAtomicServiceStartupRule_002 end");
}

/*
 * Feature: AbilityManagerService
 * Name: QueryAtomicServiceStartupRule_003
 * Function: QueryAtomicServiceStartupRule
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService QueryAtomicServiceStartupRule
 */
HWTEST_F(AbilityManagerServiceThirteenthTest, QueryAtomicServiceStartupRule_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest QueryAtomicServiceStartupRule_003 start");

    MyStatus::GetInstance().auIsLaunchEmbededUIAbility_ = true;
    MyStatus::GetInstance().atkGetTokenTypeFlag_ = 0; // 0: TOKEN_HAP
    MyStatus::GetInstance().smhGetUIAbilityManagerByUid_ = false;

    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);

    sptr<IRemoteObject> callerToken;
    std::string appId = "";
    const std::string startTime = "";
    AtomicServiceStartupRule rule;
    EXPECT_EQ(abilityMs_->QueryAtomicServiceStartupRule(callerToken, appId, startTime, rule), INNER_ERR);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest QueryAtomicServiceStartupRule_003 end");
}

/*
 * Feature: AbilityManagerService
 * Name: QueryAtomicServiceStartupRule_004
 * Function: QueryAtomicServiceStartupRule
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService QueryAtomicServiceStartupRule
 */
HWTEST_F(AbilityManagerServiceThirteenthTest, QueryAtomicServiceStartupRule_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest QueryAtomicServiceStartupRule_004 start");

    MyStatus::GetInstance().auIsLaunchEmbededUIAbility_ = true;
    MyStatus::GetInstance().atkGetTokenTypeFlag_ = 0; // 0: TOKEN_HAP
    MyStatus::GetInstance().smhGetUIAbilityManagerByUid_ = true;
    MyStatus::GetInstance().ualmGetAbilityRecordByToken_ = nullptr;

    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    auto mockSubManagersHelper = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    EXPECT_NE(mockSubManagersHelper, nullptr);
    auto mockCurrentUIAbilityManager = std::make_shared<UIAbilityLifecycleManager>(0);
    EXPECT_NE(mockCurrentUIAbilityManager, nullptr);
    abilityMs_->subManagersHelper_ = mockSubManagersHelper;
    abilityMs_->subManagersHelper_->currentUIAbilityManager_ = mockCurrentUIAbilityManager;

    sptr<IRemoteObject> callerToken;
    std::string appId = "";
    const std::string startTime = "";
    AtomicServiceStartupRule rule;
    EXPECT_EQ(abilityMs_->QueryAtomicServiceStartupRule(callerToken, appId, startTime, rule), INNER_ERR);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest QueryAtomicServiceStartupRule_004 end");
}

/*
 * Feature: AbilityManagerService
 * Name: QueryAtomicServiceStartupRule_005
 * Function: QueryAtomicServiceStartupRule
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService QueryAtomicServiceStartupRule
 */
HWTEST_F(AbilityManagerServiceThirteenthTest, QueryAtomicServiceStartupRule_005, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest QueryAtomicServiceStartupRule_005 start");
    MyStatus::GetInstance().auIsLaunchEmbededUIAbility_ = true;
    MyStatus::GetInstance().atkGetTokenTypeFlag_ = 0; // 0: TOKEN_HAP
    MyStatus::GetInstance().smhGetUIAbilityManagerByUid_ = true;
    MyStatus::GetInstance().ipcGetCallingTokenID_ = 1;
    MyStatus::GetInstance().arGetAbilityInfo_.applicationInfo.accessTokenId = -1;
    Want want;
    OHOS::AppExecFwk::AbilityInfo abilityInfo;
    OHOS::AppExecFwk::ApplicationInfo applicationInfo;
    auto record = std::make_shared<UIAbilityRecord>(want, abilityInfo, applicationInfo, -1);
    EXPECT_NE(record, nullptr);
    MyStatus::GetInstance().ualmGetAbilityRecordByToken_ = record;

    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    auto mockSubManagersHelper = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    EXPECT_NE(mockSubManagersHelper, nullptr);
    auto mockCurrentUIAbilityManager = std::make_shared<UIAbilityLifecycleManager>(0);
    EXPECT_NE(mockCurrentUIAbilityManager, nullptr);
    abilityMs_->subManagersHelper_ = mockSubManagersHelper;
    abilityMs_->subManagersHelper_->currentUIAbilityManager_ = mockCurrentUIAbilityManager;

    sptr<IRemoteObject> callerToken;
    std::string appId = "";
    const std::string startTime = "";
    AtomicServiceStartupRule rule;
    EXPECT_EQ(abilityMs_->QueryAtomicServiceStartupRule(callerToken, appId, startTime, rule), INNER_ERR);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest QueryAtomicServiceStartupRule_005 end");
}

/*
 * Feature: AbilityManagerService
 * Name: QueryAtomicServiceStartupRule_006
 * Function: QueryAtomicServiceStartupRule
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService QueryAtomicServiceStartupRule
 */
HWTEST_F(AbilityManagerServiceThirteenthTest, QueryAtomicServiceStartupRule_006, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest QueryAtomicServiceStartupRule_006 start");
    MyStatus::GetInstance().auIsLaunchEmbededUIAbility_ = true;
    MyStatus::GetInstance().atkGetTokenTypeFlag_ = 0;
    MyStatus::GetInstance().smhGetUIAbilityManagerByUid_ = true;
    MyStatus::GetInstance().ipcGetCallingTokenID_ = 1;
    MyStatus::GetInstance().arGetAbilityInfo_.applicationInfo.accessTokenId = 1;
    Want want;
    OHOS::AppExecFwk::AbilityInfo abilityInfo;
    OHOS::AppExecFwk::ApplicationInfo applicationInfo;
    auto record = std::make_shared<UIAbilityRecord>(want, abilityInfo, applicationInfo, -1);
    EXPECT_NE(record, nullptr);
    record->currentState_ = AbilityState::BACKGROUND;
    record->isAbilityForegrounding_ = false;
    MyStatus::GetInstance().ualmGetAbilityRecordByToken_ = record;

    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    auto mockSubManagersHelper = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    EXPECT_NE(mockSubManagersHelper, nullptr);
    auto mockCurrentUIAbilityManager = std::make_shared<UIAbilityLifecycleManager>(0);
    EXPECT_NE(mockCurrentUIAbilityManager, nullptr);
    abilityMs_->subManagersHelper_ = mockSubManagersHelper;
    abilityMs_->subManagersHelper_->currentUIAbilityManager_ = mockCurrentUIAbilityManager;

    sptr<IRemoteObject> callerToken;
    std::string appId = "";
    const std::string startTime = "";
    AtomicServiceStartupRule rule;
    EXPECT_EQ(abilityMs_->QueryAtomicServiceStartupRule(callerToken, appId, startTime, rule), INNER_ERR);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest QueryAtomicServiceStartupRule_006 end");
}

/*
 * Feature: AbilityManagerService
 * Name: QueryAtomicServiceStartupRule_007
 * Function: QueryAtomicServiceStartupRule
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService QueryAtomicServiceStartupRule
 */
HWTEST_F(AbilityManagerServiceThirteenthTest, QueryAtomicServiceStartupRule_007, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest QueryAtomicServiceStartupRule_007 start");
    MyStatus::GetInstance().auIsLaunchEmbededUIAbility_ = true;
    MyStatus::GetInstance().atkGetTokenTypeFlag_ = 0;
    MyStatus::GetInstance().smhGetUIAbilityManagerByUid_ = true;
    MyStatus::GetInstance().ipcGetCallingTokenID_ = 1;
    MyStatus::GetInstance().arGetAbilityInfo_.applicationInfo.accessTokenId = 1;
    MyStatus::GetInstance().fimStartFreeInstall_ = ERR_INVALID_VALUE;
    Want want;
    OHOS::AppExecFwk::AbilityInfo abilityInfo;
    OHOS::AppExecFwk::ApplicationInfo applicationInfo;
    auto record = std::make_shared<UIAbilityRecord>(want, abilityInfo, applicationInfo, -1);
    EXPECT_NE(record, nullptr);
    record->currentState_ = AbilityState::FOREGROUND;
    record->isAbilityForegrounding_ = false;
    MyStatus::GetInstance().ualmGetAbilityRecordByToken_ = record;

    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    auto mockSubManagersHelper = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    EXPECT_NE(mockSubManagersHelper, nullptr);
    auto mockCurrentUIAbilityManager = std::make_shared<UIAbilityLifecycleManager>(0);
    EXPECT_NE(mockCurrentUIAbilityManager, nullptr);
    auto mockFreeInstallManager = std::make_shared<FreeInstallManager>();
    EXPECT_NE(mockFreeInstallManager, nullptr);
    abilityMs_->subManagersHelper_ = mockSubManagersHelper;
    abilityMs_->subManagersHelper_->currentUIAbilityManager_ = mockCurrentUIAbilityManager;
    abilityMs_->freeInstallManager_ = mockFreeInstallManager;

    sptr<IRemoteObject> callerToken;
    std::string appId = "";
    const std::string startTime = "";
    AtomicServiceStartupRule rule;
    EXPECT_EQ(abilityMs_->QueryAtomicServiceStartupRule(callerToken, appId, startTime, rule), ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest QueryAtomicServiceStartupRule_007 end");
}

/*
 * Feature: AbilityManagerService
 * Name: QueryAtomicServiceStartupRule_008
 * Function: QueryAtomicServiceStartupRule
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService QueryAtomicServiceStartupRule
 */
HWTEST_F(AbilityManagerServiceThirteenthTest, QueryAtomicServiceStartupRule_008, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest QueryAtomicServiceStartupRule_008 start");
    MyStatus::GetInstance().auIsLaunchEmbededUIAbility_ = true;
    MyStatus::GetInstance().atkGetTokenTypeFlag_ = 0;
    MyStatus::GetInstance().smhGetUIAbilityManagerByUid_ = true;
    MyStatus::GetInstance().ipcGetCallingTokenID_ = 1;
    MyStatus::GetInstance().arGetAbilityInfo_.applicationInfo.accessTokenId = 1;
    MyStatus::GetInstance().fimStartFreeInstall_ = ERR_OK;
    MyStatus::GetInstance().eriQueryAtomicServiceStartupRule_ = ERR_OK;
    Want want;
    OHOS::AppExecFwk::AbilityInfo abilityInfo;
    OHOS::AppExecFwk::ApplicationInfo applicationInfo;
    auto record = std::make_shared<UIAbilityRecord>(want, abilityInfo, applicationInfo, -1);
    EXPECT_NE(record, nullptr);
    record->currentState_ = AbilityState::FOREGROUND;
    record->isAbilityForegrounding_ = true;
    MyStatus::GetInstance().ualmGetAbilityRecordByToken_ = record;

    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    auto mockSubManagersHelper = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    EXPECT_NE(mockSubManagersHelper, nullptr);
    auto mockCurrentUIAbilityManager = std::make_shared<UIAbilityLifecycleManager>(0);
    EXPECT_NE(mockCurrentUIAbilityManager, nullptr);
    auto mockFreeInstallManager = std::make_shared<FreeInstallManager>();
    EXPECT_NE(mockFreeInstallManager, nullptr);
    abilityMs_->subManagersHelper_ = mockSubManagersHelper;
    abilityMs_->subManagersHelper_->currentUIAbilityManager_ = mockCurrentUIAbilityManager;
    abilityMs_->freeInstallManager_ = mockFreeInstallManager;

    sptr<IRemoteObject> callerToken;
    std::string appId = "";
    const std::string startTime = "";
    AtomicServiceStartupRule rule;
    EXPECT_EQ(abilityMs_->QueryAtomicServiceStartupRule(callerToken, appId, startTime, rule), ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest QueryAtomicServiceStartupRule_008 end");
}

/*
 * Feature: AbilityManagerService
 * Name: QueryAtomicServiceStartupRule_009
 * Function: QueryAtomicServiceStartupRule
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService QueryAtomicServiceStartupRule
 */
HWTEST_F(AbilityManagerServiceThirteenthTest, QueryAtomicServiceStartupRule_009, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest QueryAtomicServiceStartupRule_009 start");
    MyStatus::GetInstance().auIsLaunchEmbededUIAbility_ = true;
    MyStatus::GetInstance().atkGetTokenTypeFlag_ = 0;
    MyStatus::GetInstance().smhGetUIAbilityManagerByUid_ = true;
    MyStatus::GetInstance().ipcGetCallingTokenID_ = 1;
    MyStatus::GetInstance().arGetAbilityInfo_.applicationInfo.accessTokenId = 1;
    MyStatus::GetInstance().fimStartFreeInstall_ = ERR_OK;
    MyStatus::GetInstance().eriQueryAtomicServiceStartupRule_ = ERR_CAPABILITY_NOT_SUPPORT;
    Want want;
    OHOS::AppExecFwk::AbilityInfo abilityInfo;
    OHOS::AppExecFwk::ApplicationInfo applicationInfo;
    auto record = std::make_shared<UIAbilityRecord>(want, abilityInfo, applicationInfo, -1);
    EXPECT_NE(record, nullptr);
    record->currentState_ = AbilityState::FOREGROUND;
    record->isAbilityForegrounding_ = true;
    MyStatus::GetInstance().ualmGetAbilityRecordByToken_ = record;

    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    auto mockSubManagersHelper = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    EXPECT_NE(mockSubManagersHelper, nullptr);
    auto mockCurrentUIAbilityManager = std::make_shared<UIAbilityLifecycleManager>(0);
    EXPECT_NE(mockCurrentUIAbilityManager, nullptr);
    auto mockFreeInstallManager = std::make_shared<FreeInstallManager>();
    EXPECT_NE(mockFreeInstallManager, nullptr);
    abilityMs_->subManagersHelper_ = mockSubManagersHelper;
    abilityMs_->subManagersHelper_->currentUIAbilityManager_ = mockCurrentUIAbilityManager;
    abilityMs_->freeInstallManager_ = mockFreeInstallManager;

    sptr<IRemoteObject> callerToken;
    std::string appId = "";
    const std::string startTime = "";
    AtomicServiceStartupRule rule;
    EXPECT_EQ(abilityMs_->QueryAtomicServiceStartupRule(callerToken, appId, startTime, rule),
              ERR_CAPABILITY_NOT_SUPPORT);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest QueryAtomicServiceStartupRule_009 end");
}

/*
 * Feature: AbilityManagerService
 * Name: QueryAtomicServiceStartupRule_010
 * Function: QueryAtomicServiceStartupRule
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService QueryAtomicServiceStartupRule
 */
HWTEST_F(AbilityManagerServiceThirteenthTest, QueryAtomicServiceStartupRule_010, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest QueryAtomicServiceStartupRule_010 start");
    MyStatus::GetInstance().auIsLaunchEmbededUIAbility_ = true;
    MyStatus::GetInstance().atkGetTokenTypeFlag_ = 0;
    MyStatus::GetInstance().smhGetUIAbilityManagerByUid_ = true;
    MyStatus::GetInstance().ipcGetCallingTokenID_ = 1;
    MyStatus::GetInstance().arGetAbilityInfo_.applicationInfo.accessTokenId = 1;
    MyStatus::GetInstance().fimStartFreeInstall_ = ERR_OK;
    MyStatus::GetInstance().eriQueryAtomicServiceStartupRule_ = ERR_INVALID_VALUE;
    Want want;
    OHOS::AppExecFwk::AbilityInfo abilityInfo;
    OHOS::AppExecFwk::ApplicationInfo applicationInfo;
    auto record = std::make_shared<UIAbilityRecord>(want, abilityInfo, applicationInfo, -1);
    EXPECT_NE(record, nullptr);
    record->currentState_ = AbilityState::FOREGROUND;
    record->isAbilityForegrounding_ = true;
    MyStatus::GetInstance().ualmGetAbilityRecordByToken_ = record;

    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    auto mockSubManagersHelper = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    EXPECT_NE(mockSubManagersHelper, nullptr);
    auto mockCurrentUIAbilityManager = std::make_shared<UIAbilityLifecycleManager>(0);
    EXPECT_NE(mockCurrentUIAbilityManager, nullptr);
    auto mockFreeInstallManager = std::make_shared<FreeInstallManager>();
    EXPECT_NE(mockFreeInstallManager, nullptr);
    abilityMs_->subManagersHelper_ = mockSubManagersHelper;
    abilityMs_->subManagersHelper_->currentUIAbilityManager_ = mockCurrentUIAbilityManager;
    abilityMs_->freeInstallManager_ = mockFreeInstallManager;

    sptr<IRemoteObject> callerToken;
    std::string appId = "";
    const std::string startTime = "";
    AtomicServiceStartupRule rule;
    EXPECT_EQ(abilityMs_->QueryAtomicServiceStartupRule(callerToken, appId, startTime, rule), INNER_ERR);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest QueryAtomicServiceStartupRule_010 end");
}

/*
 * Feature: AbilityManagerService
 * Name: QueryAtomicServiceStartupRule_011
 * Function: QueryAtomicServiceStartupRule
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService QueryAtomicServiceStartupRule
 */
HWTEST_F(AbilityManagerServiceThirteenthTest, QueryAtomicServiceStartupRule_011, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest QueryAtomicServiceStartupRule_011 start");
    MyStatus::GetInstance().auIsLaunchEmbededUIAbility_ = true;
    MyStatus::GetInstance().atkGetTokenTypeFlag_ = 0;
    MyStatus::GetInstance().smhGetUIAbilityManagerByUid_ = true;
    MyStatus::GetInstance().ipcGetCallingTokenID_ = 1;
    MyStatus::GetInstance().arGetAbilityInfo_.applicationInfo.accessTokenId = 1;
    MyStatus::GetInstance().fimStartFreeInstall_ = ERR_OK;
    MyStatus::GetInstance().eriQueryAtomicServiceStartupRule_ = ERR_ECOLOGICAL_CONTROL_STATUS;
    Want want;
    OHOS::AppExecFwk::AbilityInfo abilityInfo;
    OHOS::AppExecFwk::ApplicationInfo applicationInfo;
    auto record = std::make_shared<UIAbilityRecord>(want, abilityInfo, applicationInfo, -1);
    EXPECT_NE(record, nullptr);
    record->currentState_ = AbilityState::FOREGROUND;
    record->isAbilityForegrounding_ = true;
    MyStatus::GetInstance().ualmGetAbilityRecordByToken_ = record;

    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    auto mockSubManagersHelper = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    EXPECT_NE(mockSubManagersHelper, nullptr);
    auto mockCurrentUIAbilityManager = std::make_shared<UIAbilityLifecycleManager>(0);
    EXPECT_NE(mockCurrentUIAbilityManager, nullptr);
    auto mockFreeInstallManager = std::make_shared<FreeInstallManager>();
    EXPECT_NE(mockFreeInstallManager, nullptr);
    abilityMs_->subManagersHelper_ = mockSubManagersHelper;
    abilityMs_->subManagersHelper_->currentUIAbilityManager_ = mockCurrentUIAbilityManager;
    abilityMs_->freeInstallManager_ = mockFreeInstallManager;

    sptr<IRemoteObject> callerToken;
    std::string appId = "";
    const std::string startTime = "";
    AtomicServiceStartupRule rule;
    EXPECT_EQ(abilityMs_->QueryAtomicServiceStartupRule(callerToken, appId, startTime, rule), INNER_ERR);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest QueryAtomicServiceStartupRule_011 end");
}

/*
 * Feature: AbilityManagerService
 * Name: GetMissionIdByAbilityTokenInner_001
 * Function: GetMissionIdByAbilityTokenInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetMissionIdByAbilityTokenInner
 */
HWTEST_F(AbilityManagerServiceThirteenthTest, GetMissionIdByAbilityTokenInner_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest GetMissionIdByAbilityTokenInner_001 start");
    MyStatus::GetInstance().arGetAbilityRecord_ = nullptr;

    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);

    sptr<IRemoteObject> token = MockToken(AbilityType::PAGE);
    EXPECT_EQ(abilityMs_->GetMissionIdByAbilityTokenInner(token), -1);

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest GetMissionIdByAbilityTokenInner_001 end");
}

/*
 * Feature: AbilityManagerService
 * Name: GetMissionIdByAbilityTokenInner_002
 * Function: GetMissionIdByAbilityTokenInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetMissionIdByAbilityTokenInner
 */
HWTEST_F(AbilityManagerServiceThirteenthTest, GetMissionIdByAbilityTokenInner_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest GetMissionIdByAbilityTokenInner_002 start");
    Want want;
    OHOS::AppExecFwk::AbilityInfo abilityInfo;
    OHOS::AppExecFwk::ApplicationInfo applicationInfo;
    std::shared_ptr<AbilityRecord> record = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    EXPECT_NE(record, nullptr);
    record->ownerMissionUserId_ = 1;
    MyStatus::GetInstance().arGetAbilityRecord_ = record;
    MyStatus::GetInstance().sbjIsSceneBoardEnabled_ = true;
    MyStatus::GetInstance().smhGetUIAbilityManagerByUserId_ = false;

    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);

    sptr<IRemoteObject> token = MockToken(AbilityType::PAGE);
    EXPECT_EQ(abilityMs_->GetMissionIdByAbilityTokenInner(token), ERR_INVALID_VALUE);

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest GetMissionIdByAbilityTokenInner_002 end");
}

/*
 * Feature: AbilityManagerService
 * Name: GetMissionIdByAbilityTokenInner_003
 * Function: GetMissionIdByAbilityTokenInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetMissionIdByAbilityTokenInner
 */
HWTEST_F(AbilityManagerServiceThirteenthTest, GetMissionIdByAbilityTokenInner_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest GetMissionIdByAbilityTokenInner_003 start");
    Want want;
    OHOS::AppExecFwk::AbilityInfo abilityInfo;
    OHOS::AppExecFwk::ApplicationInfo applicationInfo;
    std::shared_ptr<AbilityRecord> record = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    EXPECT_NE(record, nullptr);
    record->ownerMissionUserId_ = 1;
    MyStatus::GetInstance().arGetAbilityRecord_ = record;
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

    sptr<IRemoteObject> token = MockToken(AbilityType::PAGE);
    EXPECT_EQ(abilityMs_->GetMissionIdByAbilityTokenInner(token), ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest GetMissionIdByAbilityTokenInner_003 end");
}

/*
 * Feature: AbilityManagerService
 * Name: GetMissionIdByAbilityTokenInner_004
 * Function: GetMissionIdByAbilityTokenInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetMissionIdByAbilityTokenInner
 */
HWTEST_F(AbilityManagerServiceThirteenthTest, GetMissionIdByAbilityTokenInner_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest GetMissionIdByAbilityTokenInner_004 start");
    Want want;
    OHOS::AppExecFwk::AbilityInfo abilityInfo;
    OHOS::AppExecFwk::ApplicationInfo applicationInfo;
    std::shared_ptr<AbilityRecord> record = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    EXPECT_NE(record, nullptr);
    record->ownerMissionUserId_ = 1;
    MyStatus::GetInstance().arGetAbilityRecord_ = record;
    MyStatus::GetInstance().sbjIsSceneBoardEnabled_ = false;
    MyStatus::GetInstance().smhGetUIAbilityManagerByUserId_ = true;
    MyStatus::GetInstance().smhGetMissionListManagerByUserId_ = false;
    
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);

    auto mockSubManagersHelper = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    EXPECT_NE(mockSubManagersHelper, nullptr);
    auto mockCurrentUIAbilityManager = std::make_shared<UIAbilityLifecycleManager>(0);
    EXPECT_NE(mockCurrentUIAbilityManager, nullptr);
    abilityMs_->subManagersHelper_ = mockSubManagersHelper;
    abilityMs_->subManagersHelper_->currentUIAbilityManager_ = mockCurrentUIAbilityManager;

    sptr<IRemoteObject> token = MockToken(AbilityType::PAGE);
    EXPECT_EQ(abilityMs_->GetMissionIdByAbilityTokenInner(token), -1);

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest GetMissionIdByAbilityTokenInner_004 end");
}

/*
 * Feature: AbilityManagerService
 * Name: KillProcessWithReason_002
 * Function: KillProcessWithReason
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService KillProcessWithReason
 */
HWTEST_F(AbilityManagerServiceThirteenthTest, KillProcessWithReason_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest KillProcessWithReason_002 start");

    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);

    auto mockSubManagersHelper = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    EXPECT_NE(mockSubManagersHelper, nullptr);
    auto mockCurrentUIAbilityManager = std::make_shared<UIAbilityLifecycleManager>(0);
    EXPECT_NE(mockCurrentUIAbilityManager, nullptr);
    abilityMs_->subManagersHelper_ = mockSubManagersHelper;
    abilityMs_->subManagersHelper_->currentUIAbilityManager_ = mockCurrentUIAbilityManager;

    int32_t pid = LOW_MEMORY_KILL_WHILE_STARTING;
    MyStatus::GetInstance().permPermission_ = 1;
    ExitReason reason;
    reason.exitMsg = AbilityRuntime::GlobalConstant::LOW_MEMORY_KILL;
    reason.reason = Reason::REASON_RESOURCE_CONTROL;
    MyStatus::GetInstance().smhGetUIAbilityManagerByUid_ = true;

    EXPECT_EQ(abilityMs_->KillProcessWithReason(pid, reason), ERR_KILL_APP_WHILE_STARTING);

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest KillProcessWithReason_002 end");
}

/*
 * Feature: AbilityManagerService
 * Name: KillProcessWithReason_003
 * Function: InitFocusListener
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService KillProcessWithReason
 */
HWTEST_F(AbilityManagerServiceThirteenthTest, KillProcessWithReason_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest KillProcessWithReason_003 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    int32_t pid = 1;
    ExitReason exitReason;
    exitReason.reason = Reason::REASON_RESOURCE_CONTROL;
    exitReason.shouldKillForeground = false;
    auto result = abilityMs_->KillProcessWithReason(pid, exitReason);
    EXPECT_EQ(result, ERR_KILL_APP_WHILE_FOREGROUND);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest KillProcessWithReason_003 end");
}

/*
 * Feature: AbilityManagerService
 * Name: StartUIAbilitiesInSplitWindowMode_001
 * Function: StartUIAbilitiesInSplitWindowMode
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartUIAbilitiesInSplitWindowMode SCB false
 */
HWTEST_F(AbilityManagerServiceThirteenthTest, StartUIAbilitiesInSplitWindowMode_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest StartUIAbilitiesInSplitWindowMode_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    Want want;
    want.SetElementName("com.ohos.test", "MainAbility");

    int32_t windowId = 1;
    sptr<IRemoteObject> callerToken = nullptr;
    MyStatus::GetInstance().sbjIsSceneBoardEnabled_ = false;

    EXPECT_EQ(abilityMs_->StartUIAbilitiesInSplitWindowMode(windowId, want, callerToken),
        ERR_CAPABILITY_NOT_SUPPORT);

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest StartUIAbilitiesInSplitWindowMode_001 end");
}

/*
 * Feature: AbilityManagerService
 * Name: StartUIAbilitiesInSplitWindowMode_002
 * Function: StartUIAbilitiesInSplitWindowMode
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartUIAbilitiesInSplitWindowMode SUPPORT_LINKAGE_SCENE false
 */
HWTEST_F(AbilityManagerServiceThirteenthTest, StartUIAbilitiesInSplitWindowMode_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest StartUIAbilitiesInSplitWindowMode_002 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    Want want;
    want.SetElementName("com.ohos.test", "MainAbility");

    int32_t windowId = 1;
    sptr<IRemoteObject> callerToken = nullptr;
    MyStatus::GetInstance().sbjIsSceneBoardEnabled_ = true;
    MyStatus::GetInstance().paramGetBoolParameter_ = false;

    EXPECT_EQ(abilityMs_->StartUIAbilitiesInSplitWindowMode(windowId, want, callerToken),
        ERR_CAPABILITY_NOT_SUPPORT);

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest StartUIAbilitiesInSplitWindowMode_002 end");
}

/*
 * Feature: AbilityManagerService
 * Name: StartUIAbilitiesInSplitWindowMode_003
 * Function: StartUIAbilitiesInSplitWindowMode
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartUIAbilitiesInSplitWindowMode SUPPORT_LINKAGE_SCENE false
 */
HWTEST_F(AbilityManagerServiceThirteenthTest, StartUIAbilitiesInSplitWindowMode_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest StartUIAbilitiesInSplitWindowMode_003 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    Want want;
    int32_t windowId = 1;
    sptr<IRemoteObject> callerToken = nullptr;
    MyStatus::GetInstance().sbjIsSceneBoardEnabled_ = true;
    MyStatus::GetInstance().paramGetBoolParameter_ = true;
    MyStatus::GetInstance().isSystemAppCall_ = false;
    EXPECT_EQ(abilityMs_->StartUIAbilitiesInSplitWindowMode(windowId, want, callerToken),
        ERR_NOT_SYSTEM_APP);
    MyStatus::GetInstance().isSystemAppCall_ = true;
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest StartUIAbilitiesInSplitWindowMode_003 end");
}

/*
 * Feature: AbilityManagerService
 * Name: StartUIAbilitiesInSplitWindowMode_004
 * Function: StartUIAbilitiesInSplitWindowMode
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartUIAbilitiesInSplitWindowMode callerToken is null
 */
HWTEST_F(AbilityManagerServiceThirteenthTest, StartUIAbilitiesInSplitWindowMode_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest StartUIAbilitiesInSplitWindowMode_004 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);

    Want want;
    want.SetElementName("com.ohos.test", "MainAbility");

    int32_t windowId = 1;
    sptr<IRemoteObject> callerToken = nullptr;
    MyStatus::GetInstance().sbjIsSceneBoardEnabled_ = true;
    MyStatus::GetInstance().paramGetBoolParameter_ = true;
    EXPECT_EQ(abilityMs_->StartUIAbilitiesInSplitWindowMode(windowId, want, callerToken),
        ERR_INVALID_CALLER);

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest StartUIAbilitiesInSplitWindowMode_004 end");
}

/*
 * Feature: AbilityManagerService
 * Name: StartUIAbilitiesInSplitWindowMode_005
 * Function: StartUIAbilitiesInSplitWindowMode
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartUIAbilitiesInSplitWindowMode verifyToken failed
 */
HWTEST_F(AbilityManagerServiceThirteenthTest, StartUIAbilitiesInSplitWindowMode_005, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest StartUIAbilitiesInSplitWindowMode_005 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    auto mockSubManagersHelper = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    EXPECT_NE(mockSubManagersHelper, nullptr);
    auto mockCurrentUIAbilityManager = std::make_shared<UIAbilityLifecycleManager>(0);
    EXPECT_NE(mockCurrentUIAbilityManager, nullptr);
    abilityMs_->subManagersHelper_ = mockSubManagersHelper;
    abilityMs_->subManagersHelper_->currentUIAbilityManager_ = mockCurrentUIAbilityManager;

    Want want;
    want.SetElementName("com.ohos.test", "MainAbility");

    int32_t windowId = 1;
    sptr<IRemoteObject> callerToken = MockToken(AbilityType::EXTENSION);
    MyStatus::GetInstance().sbjIsSceneBoardEnabled_ = true;
    MyStatus::GetInstance().paramGetBoolParameter_ = true;
    MyStatus::GetInstance().smhVerificationAllToken_ = false;
    EXPECT_EQ(abilityMs_->StartUIAbilitiesInSplitWindowMode(windowId, want, callerToken),
        ERR_INVALID_CALLER);

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest StartUIAbilitiesInSplitWindowMode_005 end");
}

/*
 * Feature: AbilityManagerService
 * Name: StartUIAbilitiesInSplitWindowMode_006
 * Function: StartUIAbilitiesInSplitWindowMode
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartUIAbilitiesInSplitWindowModeHandleWant failed
 */
HWTEST_F(AbilityManagerServiceThirteenthTest, StartUIAbilitiesInSplitWindowMode_006, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest StartUIAbilitiesInSplitWindowMode_006 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    auto mockSubManagersHelper = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    EXPECT_NE(mockSubManagersHelper, nullptr);
    auto mockCurrentUIAbilityManager = std::make_shared<UIAbilityLifecycleManager>(0);
    EXPECT_NE(mockCurrentUIAbilityManager, nullptr);
    abilityMs_->subManagersHelper_ = mockSubManagersHelper;
    abilityMs_->subManagersHelper_->currentUIAbilityManager_ = mockCurrentUIAbilityManager;

    Want want;
    int32_t windowId = 1;
    sptr<IRemoteObject> callerToken = MockToken(AbilityType::EXTENSION);
    MyStatus::GetInstance().sbjIsSceneBoardEnabled_ = true;
    MyStatus::GetInstance().paramGetBoolParameter_ = true;
    MyStatus::GetInstance().smhVerificationAllToken_ = true;
    EXPECT_NE(abilityMs_->StartUIAbilitiesInSplitWindowMode(windowId, want, callerToken),
        ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest StartUIAbilitiesInSplitWindowMode_006 end");
}

/*
 * Feature: AbilityManagerService
 * Name: StartUIAbilitiesInSplitWindowModeHandleWant_001
 * Function: StartUIAbilitiesInSplitWindowModeHandleWant
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartUIAbilitiesInSplitWindowModeHandleWant failed
 */
HWTEST_F(AbilityManagerServiceThirteenthTest, StartUIAbilitiesInSplitWindowModeHandleWant_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST,
        "AbilityManagerServiceThirteenthTest StartUIAbilitiesInSplitWindowModeHandleWant_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);

    Want want;
    AbilityRequest abilityRequest;
    int32_t windowId = 1;
    int32_t validUserId = abilityMs_->GetValidUserId(DEFAULT_INVAL_VALUE);
    int32_t appIndex = 0;
    sptr<IRemoteObject> callerToken = MockToken(AbilityType::EXTENSION);

    EXPECT_NE(abilityMs_->StartUIAbilitiesInSplitWindowModeHandleWant(want, callerToken, abilityRequest),
        ERR_OK);

    TAG_LOGI(AAFwkTag::TEST,
        "AbilityManagerServiceThirteenthTest StartUIAbilitiesInSplitWindowModeHandleWant_001 end");
}

/*
 * Feature: AbilityManagerService
 * Name: StartUIAbilitiesInSplitWindowModeHandleWant_002
 * Function: StartUIAbilitiesInSplitWindowModeHandleWant
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartUIAbilitiesInSplitWindowModeHandleWant generateRequest
 */
HWTEST_F(AbilityManagerServiceThirteenthTest, StartUIAbilitiesInSplitWindowModeHandleWant_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST,
        "AbilityManagerServiceThirteenthTest StartUIAbilitiesInSplitWindowModeHandleWant_002 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();

    Want want;
    want.SetElementName("com.ohos.test", "MainAbility");
    AbilityRequest abilityRequest;
    int32_t windowId = 1;
    int32_t validUserId = abilityMs_->GetValidUserId(DEFAULT_INVAL_VALUE);
    int32_t appIndex = 0;
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    EXPECT_NE(abilityRecord, nullptr);
    sptr<IRemoteObject> callerToken = abilityRecord->GetToken();

    EXPECT_EQ(abilityMs_->StartUIAbilitiesInSplitWindowModeHandleWant(want, callerToken, abilityRequest),
        RESOLVE_ABILITY_ERR);

    TAG_LOGI(AAFwkTag::TEST,
        "AbilityManagerServiceThirteenthTest StartUIAbilitiesInSplitWindowModeHandleWant_002 end");
}
/*
 * Feature: AbilityManagerService
 * Name: CheckWantForSplitMode_001
 * Function: CheckWantForSplitMode
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckWantForSplitMode verifyToken failed
 */
HWTEST_F(AbilityManagerServiceThirteenthTest, CheckWantForSplitMode_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest CheckWantForSplitMode_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    auto mockSubManagersHelper = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    EXPECT_NE(mockSubManagersHelper, nullptr);
    auto mockCurrentUIAbilityManager = std::make_shared<UIAbilityLifecycleManager>(0);
    EXPECT_NE(mockCurrentUIAbilityManager, nullptr);
    abilityMs_->subManagersHelper_ = mockSubManagersHelper;
    abilityMs_->subManagersHelper_->currentUIAbilityManager_ = mockCurrentUIAbilityManager;

    Want want;
    want.SetElementName("com.ohos.test", "MainAbility");
    want.SetParam(AAFwk::Want::DESTINATION_PLUGIN_ABILITY, true);

    int32_t windowId = 1;
    int32_t validUserId = abilityMs_->GetValidUserId(DEFAULT_INVAL_VALUE);
    int32_t appIndex = 0;
    sptr<IRemoteObject> callerToken = MockToken(AbilityType::EXTENSION);

    EXPECT_EQ(abilityMs_->CheckWantForSplitMode(want, callerToken, validUserId, appIndex),
        START_UI_ABILITIES_NOT_SUPPORT_START_PLUGIN);

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest CheckWantForSplitMode_001 end");
}

/*
 * Feature: AbilityManagerService
 * Name: CheckWantForSplitMode_003
 * Function: CheckWantForSplitMode
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckWantForSplitMode dlp failed with dlp
 */
HWTEST_F(AbilityManagerServiceThirteenthTest, CheckWantForSplitMode_002, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    auto mockSubManagersHelper = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    EXPECT_NE(mockSubManagersHelper, nullptr);
    auto mockCurrentUIAbilityManager = std::make_shared<UIAbilityLifecycleManager>(0);
    EXPECT_NE(mockCurrentUIAbilityManager, nullptr);
    abilityMs_->subManagersHelper_ = mockSubManagersHelper;
    abilityMs_->subManagersHelper_->currentUIAbilityManager_ = mockCurrentUIAbilityManager;

    Want want;
    want.SetElementName("com.ohos.test", "MainAbility");
    want.SetParam(DLP_PARAMS_SANDBOX, true);
    int32_t windowId = 1;
    int32_t validUserId = abilityMs_->GetValidUserId(DEFAULT_INVAL_VALUE);
    int32_t appIndex = 0;
    sptr<IRemoteObject> callerToken = MockToken(AbilityType::EXTENSION);

    EXPECT_EQ(abilityMs_->CheckWantForSplitMode(want, callerToken, validUserId, appIndex),
        START_UI_ABILITIES_NOT_SUPPORT_DLP);

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest CheckWantForSplitMode_002 end");
}

/*
 * Feature: AbilityManagerService
 * Name: CheckWantForSplitMode_003
 * Function: CheckWantForSplitMode
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckWantForSplitMode appCloneIndexError
 */
HWTEST_F(AbilityManagerServiceThirteenthTest, CheckWantForSplitMode_003, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);

    Want want;
    AbilityRequest abilityRequest;
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    EXPECT_NE(abilityRecord, nullptr);
    sptr<IRemoteObject> callerToken = abilityRecord->GetToken();
    int32_t windowId = 1;
    int32_t validUserId = abilityMs_->GetValidUserId(DEFAULT_INVAL_VALUE);
    int32_t appIndex = 0;

    want.SetParam(Want::PARAM_APP_CLONE_INDEX_KEY, -5);

    EXPECT_EQ(abilityMs_->CheckWantForSplitMode(want, callerToken, validUserId, appIndex),
        ERR_APP_CLONE_INDEX_INVALID);

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest CheckWantForSplitMode_003 end");
}

/*
 * Feature: AbilityManagerService
 * Name: CheckWantForSplitMode_004
 * Function: CheckWantForSplitMode
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckWantForSplitMode ImplicitStartError
 */
HWTEST_F(AbilityManagerServiceThirteenthTest, CheckWantForSplitMode_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest CheckWantForSplitMode_004 start");

    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);

    Want want;
    AbilityRequest abilityRequest;
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    EXPECT_NE(abilityRecord, nullptr);
    sptr<IRemoteObject> callerToken = abilityRecord->GetToken();
    int32_t windowId = 1;
    int32_t validUserId = abilityMs_->GetValidUserId(DEFAULT_INVAL_VALUE);
    int32_t appIndex = 0;

    EXPECT_EQ(abilityMs_->CheckWantForSplitMode(want, callerToken, validUserId, appIndex),
        START_UI_ABILITIES_NOT_SUPPORT_IMPLICIT_START);

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest CheckWantForSplitMode_004 end");
}

/*
 * Feature: AbilityManagerService
 * Name: CheckWantForSplitMode_005
 * Function: CheckWantForSplitMode
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckWantForSplitMode createInstance
 */
HWTEST_F(AbilityManagerServiceThirteenthTest, CheckWantForSplitMode_005, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest CheckWantForSplitMode_005 start");

    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);

    Want want;
    AbilityRequest abilityRequest;
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    EXPECT_NE(abilityRecord, nullptr);
    sptr<IRemoteObject> callerToken = abilityRecord->GetToken();
    int32_t windowId = 1;
    int32_t validUserId = abilityMs_->GetValidUserId(DEFAULT_INVAL_VALUE);
    int32_t appIndex = 0;

    want.SetParam(AAFwk::Want::CREATE_APP_INSTANCE_KEY, true);
    EXPECT_EQ(abilityMs_->CheckWantForSplitMode(want, callerToken, validUserId, appIndex),
        START_UI_ABILITIES_NOT_SUPPORT_CREATE_APP_INSTANCE_KEY);

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest CheckWantForSplitMode_005 end");
}

/*
 * Feature: AbilityManagerService
 * Name: CheckWantForSplitMode_006
 * Function: CheckWantForSplitMode
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckWantForSplitMode multiInstanceError
 */
HWTEST_F(AbilityManagerServiceThirteenthTest, CheckWantForSplitMode_006, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest CheckWantForSplitMode_006 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);

    Want want;
    want.SetElementName("com.ohos.test", "MainAbility");
    AbilityRequest abilityRequest;
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    EXPECT_NE(abilityRecord, nullptr);
    sptr<IRemoteObject> callerToken = abilityRecord->GetToken();
    MyStatus::GetInstance().isSupportMultiInstance_ = false;
    std::string instanceKey = "MainAbility";
    want.SetParam(AAFwk::Want::APP_INSTANCE_KEY, instanceKey);
    int32_t windowId = 1;
    int32_t validUserId = abilityMs_->GetValidUserId(DEFAULT_INVAL_VALUE);
    int32_t appIndex = 0;
    EXPECT_NE(abilityMs_->CheckWantForSplitMode(want, callerToken, validUserId, appIndex),
        ERR_OK);
    MyStatus::GetInstance().isSupportMultiInstance_ = true;
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest CheckWantForSplitMode_006 end");
}

/*
 * Feature: AbilityManagerService
 * Name: CheckWantForSplitMode_007
 * Function: CheckWantForSplitMode
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckWantForSplitMode return ok
 */
HWTEST_F(AbilityManagerServiceThirteenthTest, CheckWantForSplitMode_007, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest CheckWantForSplitMode_007 start");

    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);

    Want want;
    want.SetElementName("com.ohos.test", "MainAbility");
    AbilityRequest abilityRequest;
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    EXPECT_NE(abilityRecord, nullptr);
    sptr<IRemoteObject> callerToken = abilityRecord->GetToken();
    int32_t windowId = 1;
    int32_t validUserId = abilityMs_->GetValidUserId(DEFAULT_INVAL_VALUE);
    int32_t appIndex = 0;

    EXPECT_EQ(abilityMs_->CheckWantForSplitMode(want, callerToken, validUserId, appIndex),
        ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest CheckWantForSplitMode_007 end");
}

/*
 * Feature: AbilityManagerService
 * Name: StartUIAbilities_001
 * Function: StartUIAbilities
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartUIAbilities CCM false SCB false
 */
HWTEST_F(AbilityManagerServiceThirteenthTest, StartUIAbilities_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest StartUIAbilities_001 start");

    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);

    std::vector<Want> wantList;
    std::string requestKey;
    sptr<IRemoteObject> callerToken = nullptr;

    MyStatus::GetInstance().isSupportStartAbilities_ = false;
    MyStatus::GetInstance().sbjIsSceneBoardEnabled_ = false;

    EXPECT_EQ(abilityMs_->StartUIAbilities(wantList, requestKey, callerToken), ERR_CAPABILITY_NOT_SUPPORT);

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest StartUIAbilities_001 end");
}

/*
 * Feature: AbilityManagerService
 * Name: StartUIAbilities_002
 * Function: StartUIAbilities
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartUIAbilities CCM true SCB false
 */
HWTEST_F(AbilityManagerServiceThirteenthTest, StartUIAbilities_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest StartUIAbilities_002 start");

    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);

    std::vector<Want> wantList;
    std::string requestKey;
    sptr<IRemoteObject> callerToken = nullptr;

    MyStatus::GetInstance().isSupportStartAbilities_ = true;
    MyStatus::GetInstance().sbjIsSceneBoardEnabled_ = false;

    EXPECT_EQ(abilityMs_->StartUIAbilities(wantList, requestKey, callerToken), ERR_CAPABILITY_NOT_SUPPORT);

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest StartUIAbilities_002 end");
}

/*
 * Feature: AbilityManagerService
 * Name: StartUIAbilities_003
 * Function: StartUIAbilities
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartUIAbilities CCM false SCB true
 */
HWTEST_F(AbilityManagerServiceThirteenthTest, StartUIAbilities_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest StartUIAbilities_003 start");

    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);

    std::vector<Want> wantList;
    std::string requestKey;
    sptr<IRemoteObject> callerToken = nullptr;

    MyStatus::GetInstance().isSupportStartAbilities_ = false;
    MyStatus::GetInstance().sbjIsSceneBoardEnabled_ = true;

    EXPECT_EQ(abilityMs_->StartUIAbilities(wantList, requestKey, callerToken), ERR_CAPABILITY_NOT_SUPPORT);

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest StartUIAbilities_003 end");
}

/*
 * Feature: AbilityManagerService
 * Name: StartUIAbilities_004
 * Function: StartUIAbilities
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartUIAbilities CCM true SCB true listSize = 0
 */
HWTEST_F(AbilityManagerServiceThirteenthTest, StartUIAbilities_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest StartUIAbilities_004 start");

    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);

    std::vector<Want> wantList(6);
    std::string requestKey;
    sptr<IRemoteObject> callerToken = nullptr;

    MyStatus::GetInstance().isSupportStartAbilities_ = true;
    MyStatus::GetInstance().sbjIsSceneBoardEnabled_ = true;

    EXPECT_EQ(abilityMs_->StartUIAbilities(wantList, requestKey, callerToken), START_UI_ABILITIES_WANT_LIST_SIZE_ERROR);

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest StartUIAbilities_004 end");
}

/*
 * Feature: AbilityManagerService
 * Name: StartUIAbilities_005
 * Function: StartUIAbilities
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartUIAbilities listSize > 4
 */
HWTEST_F(AbilityManagerServiceThirteenthTest, StartUIAbilities_005, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest StartUIAbilities_005 start");

    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);

    std::vector<Want> wantList(5);
    std::string requestKey;
    sptr<IRemoteObject> callerToken = nullptr;
    MyStatus::GetInstance().isSupportStartAbilities_ = true;
    MyStatus::GetInstance().sbjIsSceneBoardEnabled_ = true;

    EXPECT_EQ(abilityMs_->StartUIAbilities(wantList, requestKey, callerToken), START_UI_ABILITIES_WANT_LIST_SIZE_ERROR);

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest StartUIAbilities_005 end");
}

/*
 * Feature: AbilityManagerService
 * Name: StartUIAbilities_006
 * Function: StartUIAbilities
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartUIAbilities callerToken = nullptr
 */
HWTEST_F(AbilityManagerServiceThirteenthTest, StartUIAbilities_006, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest StartUIAbilities_006 start");

    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    auto mockSubManagersHelper = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    EXPECT_NE(mockSubManagersHelper, nullptr);
    auto mockCurrentUIAbilityManager = std::make_shared<UIAbilityLifecycleManager>(0);
    EXPECT_NE(mockCurrentUIAbilityManager, nullptr);
    abilityMs_->subManagersHelper_ = mockSubManagersHelper;
    abilityMs_->subManagersHelper_->currentUIAbilityManager_ = mockCurrentUIAbilityManager;

    std::vector<Want> wantList(2);
    std::string requestKey;
    sptr<IRemoteObject> callerToken = nullptr;
    MyStatus::GetInstance().isSupportStartAbilities_ = true;
    MyStatus::GetInstance().sbjIsSceneBoardEnabled_ = true;

    EXPECT_EQ(abilityMs_->StartUIAbilities(wantList, requestKey, callerToken), ERR_INVALID_CALLER);

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest StartUIAbilities_006 end");
}

/*
 * Feature: AbilityManagerService
 * Name: StartUIAbilities_007
 * Function: StartUIAbilities
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartUIAbilities verifyToken failed
 */
HWTEST_F(AbilityManagerServiceThirteenthTest, StartUIAbilities_007, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest StartUIAbilities_007 start");

    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    auto mockSubManagersHelper = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    EXPECT_NE(mockSubManagersHelper, nullptr);
    auto mockCurrentUIAbilityManager = std::make_shared<UIAbilityLifecycleManager>(0);
    EXPECT_NE(mockCurrentUIAbilityManager, nullptr);
    abilityMs_->subManagersHelper_ = mockSubManagersHelper;
    abilityMs_->subManagersHelper_->currentUIAbilityManager_ = mockCurrentUIAbilityManager;

    std::vector<Want> wantList(2);
    std::string requestKey;
    MyStatus::GetInstance().isSupportStartAbilities_ = true;
    MyStatus::GetInstance().sbjIsSceneBoardEnabled_ = true;

    sptr<IRemoteObject> callerToken = MockToken(AbilityType::EXTENSION);
    MyStatus::GetInstance().smhVerificationAllToken_ = false;

    EXPECT_EQ(abilityMs_->StartUIAbilities(wantList, requestKey, callerToken), ERR_INVALID_CALLER);

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest StartUIAbilities_007 end");
}

/*
 * Feature: AbilityManagerService
 * Name: StartUIAbilities_008
 * Function: StartUIAbilities
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartUIAbilities handleWant failed
 */
HWTEST_F(AbilityManagerServiceThirteenthTest, StartUIAbilities_008, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest StartUIAbilities_008 start");

    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    auto mockSubManagersHelper = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    EXPECT_NE(mockSubManagersHelper, nullptr);
    auto mockCurrentUIAbilityManager = std::make_shared<UIAbilityLifecycleManager>(0);
    EXPECT_NE(mockCurrentUIAbilityManager, nullptr);
    abilityMs_->subManagersHelper_ = mockSubManagersHelper;
    abilityMs_->subManagersHelper_->currentUIAbilityManager_ = mockCurrentUIAbilityManager;

    std::vector<Want> wantList(2);
    std::string requestKey;
    sptr<IRemoteObject> callerToken = MockToken(AbilityType::EXTENSION);
    MyStatus::GetInstance().isSupportStartAbilities_ = true;
    MyStatus::GetInstance().sbjIsSceneBoardEnabled_ = true;
    MyStatus::GetInstance().smhVerificationAllToken_ = true;

    EXPECT_NE(abilityMs_->StartUIAbilities(wantList, requestKey, callerToken), ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest StartUIAbilities_008 end");
}

/*
 * Feature: AbilityManagerService
 * Name: StartUIAbilitiesHandleWant_001
 * Function: StartUIAbilitiesHandleWant
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartUIAbilitiesHandleWant startPlugin
 */
HWTEST_F(AbilityManagerServiceThirteenthTest, StartUIAbilitiesHandleWant_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest StartUIAbilitiesHandleWant_001 start");

    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);

    std::vector<AbilityRequest> abilityRequestList;
    std::string requestKey;
    sptr<IRemoteObject> callerToken = MockToken(AbilityType::EXTENSION);
    MyStatus::GetInstance().isSupportStartAbilities_ = true;
    MyStatus::GetInstance().sbjIsSceneBoardEnabled_ = true;
    MyStatus::GetInstance().smhVerificationAllToken_ = true;

    Want want;
    want.SetParam(AAFwk::Want::DESTINATION_PLUGIN_ABILITY, true);

    EXPECT_EQ(abilityMs_->StartUIAbilitiesHandleWant(want, callerToken, abilityRequestList),
        START_UI_ABILITIES_NOT_SUPPORT_START_PLUGIN);

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest StartUIAbilitiesHandleWant_001 end");
}

/*
 * Feature: AbilityManagerService
 * Name: StartUIAbilitiesHandleWant_002
 * Function: StartUIAbilitiesHandleWant
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartUIAbilitiesHandleWant createInstance
 */
HWTEST_F(AbilityManagerServiceThirteenthTest, StartUIAbilitiesHandleWant_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest StartUIAbilitiesHandleWant_002 start");

    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);

    std::vector<AbilityRequest> abilityRequestList;
    std::string requestKey;
    sptr<IRemoteObject> callerToken = MockToken(AbilityType::EXTENSION);
    MyStatus::GetInstance().isSupportStartAbilities_ = true;
    MyStatus::GetInstance().sbjIsSceneBoardEnabled_ = true;
    MyStatus::GetInstance().smhVerificationAllToken_ = true;

    Want want;
    want.SetParam(AAFwk::Want::CREATE_APP_INSTANCE_KEY, true);

    EXPECT_EQ(abilityMs_->StartUIAbilitiesHandleWant(want, callerToken, abilityRequestList),
        START_UI_ABILITIES_NOT_SUPPORT_CREATE_APP_INSTANCE_KEY);

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest StartUIAbilitiesHandleWant_002 end");
}

/*
 * Feature: AbilityManagerService
 * Name: StartUIAbilitiesHandleWant_003
 * Function: StartUIAbilitiesHandleWant
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartUIAbilitiesHandleWant dlp failed with dlp
 */
HWTEST_F(AbilityManagerServiceThirteenthTest, StartUIAbilitiesHandleWant_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest StartUIAbilitiesHandleWant_003 start");

    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);

    std::vector<AbilityRequest> abilityRequestList;
    std::string requestKey;
    sptr<IRemoteObject> callerToken = MockToken(AbilityType::EXTENSION);
    MyStatus::GetInstance().isSupportStartAbilities_ = true;
    MyStatus::GetInstance().sbjIsSceneBoardEnabled_ = true;
    MyStatus::GetInstance().smhVerificationAllToken_ = true;

    Want want;
    want.SetParam(DLP_PARAMS_SANDBOX, true);
    want.SetElementName("com.ohos.test", "MainAbility");

    EXPECT_EQ(abilityMs_->StartUIAbilitiesHandleWant(want, callerToken, abilityRequestList),
        START_UI_ABILITIES_NOT_SUPPORT_DLP);

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest StartUIAbilitiesHandleWant_003 end");
}

/*
 * Feature: AbilityManagerService
 * Name: StartUIAbilitiesHandleWant_004
 * Function: StartUIAbilitiesHandleWant
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartUIAbilitiesHandleWant appCloneIndexError
 */
HWTEST_F(AbilityManagerServiceThirteenthTest, StartUIAbilitiesHandleWant_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest StartUIAbilitiesHandleWant_004 start");

    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);

    std::vector<AbilityRequest> abilityRequestList;
    std::string requestKey;
    MyStatus::GetInstance().isSupportStartAbilities_ = true;
    MyStatus::GetInstance().sbjIsSceneBoardEnabled_ = true;
    MyStatus::GetInstance().smhVerificationAllToken_ = true;

    Want want;
    AbilityRequest abilityRequest;
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    EXPECT_NE(abilityRecord, nullptr);
    sptr<IRemoteObject> callerToken = abilityRecord->GetToken();
    int32_t userId = DEFAULT_INVAL_VALUE;
    MyStatus::GetInstance().permPermission_ = true;

    want.SetParam(Want::PARAM_APP_CLONE_INDEX_KEY, -5);

    EXPECT_EQ(abilityMs_->StartUIAbilitiesHandleWant(want, callerToken, abilityRequestList),
        ERR_APP_CLONE_INDEX_INVALID);

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest StartUIAbilitiesHandleWant_004 end");
}

/*
 * Feature: AbilityManagerService
 * Name: StartUIAbilitiesHandleWant_005
 * Function: StartUIAbilitiesHandleWant
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartUIAbilitiesHandleWant multiInstanceError
 */
HWTEST_F(AbilityManagerServiceThirteenthTest, StartUIAbilitiesHandleWant_005, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest StartUIAbilitiesHandleWant_005 start");

    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);

    std::vector<AbilityRequest> abilityRequestList;
    std::string requestKey;
    MyStatus::GetInstance().isSupportStartAbilities_ = true;
    MyStatus::GetInstance().sbjIsSceneBoardEnabled_ = true;
    MyStatus::GetInstance().smhVerificationAllToken_ = true;

    Want want;
    want.SetElementName("com.ohos.test", "MainAbility");
    AbilityRequest abilityRequest;
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    EXPECT_NE(abilityRecord, nullptr);
    sptr<IRemoteObject> callerToken = abilityRecord->GetToken();
    int32_t userId = DEFAULT_INVAL_VALUE;
    MyStatus::GetInstance().permPermission_ = true;

    want.SetParam(Want::APP_INSTANCE_KEY, 2);
    MyStatus::GetInstance().isSupportMultiInstance_ = false;

    EXPECT_NE(abilityMs_->StartUIAbilitiesHandleWant(want, callerToken, abilityRequestList),
        ERR_OK);

    MyStatus::GetInstance().isSupportMultiInstance_ = true;

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest StartUIAbilitiesHandleWant_005 end");
}

/*
 * Feature: AbilityManagerService
 * Name: StartUIAbilitiesHandleWant_006
 * Function: StartUIAbilitiesHandleWant
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartUIAbilitiesHandleWant OperateRemoteError
 */
HWTEST_F(AbilityManagerServiceThirteenthTest, StartUIAbilitiesHandleWant_006, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest StartUIAbilitiesHandleWant_006 start");

    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);

    std::vector<AbilityRequest> abilityRequestList;
    std::string requestKey;
    MyStatus::GetInstance().isSupportStartAbilities_ = true;
    MyStatus::GetInstance().sbjIsSceneBoardEnabled_ = true;
    MyStatus::GetInstance().smhVerificationAllToken_ = true;

    Want want;
    want.SetElementName("com.ohos.test", "MainAbility");
    AbilityRequest abilityRequest;
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    EXPECT_NE(abilityRecord, nullptr);
    sptr<IRemoteObject> callerToken = abilityRecord->GetToken();
    int32_t userId = DEFAULT_INVAL_VALUE;
    MyStatus::GetInstance().permPermission_ = true;

    want.SetDeviceId("deviceId");

    EXPECT_NE(abilityMs_->StartUIAbilitiesHandleWant(want, callerToken, abilityRequestList),
        ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest StartUIAbilitiesHandleWant_006 end");
}

/*
 * Feature: AbilityManagerService
 * Name: StartUIAbilitiesHandleWant_007
 * Function: StartUIAbilitiesHandleWant
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartUIAbilitiesHandleWant CrossUserError?
 */
HWTEST_F(AbilityManagerServiceThirteenthTest, StartUIAbilitiesHandleWant_007, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest StartUIAbilitiesHandleWant_007 start");

    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);

    std::vector<AbilityRequest> abilityRequestList;
    std::string requestKey;
    MyStatus::GetInstance().isSupportStartAbilities_ = true;
    MyStatus::GetInstance().sbjIsSceneBoardEnabled_ = true;
    MyStatus::GetInstance().smhVerificationAllToken_ = true;

    Want want;
    want.SetElementName("com.ohos.test", "MainAbility");
    AbilityRequest abilityRequest;
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    EXPECT_NE(abilityRecord, nullptr);
    sptr<IRemoteObject> callerToken = abilityRecord->GetToken();
    int32_t userId = DEFAULT_INVAL_VALUE;
    MyStatus::GetInstance().permPermission_ = true;

    want.SetDeviceId("deviceId");

    EXPECT_NE(abilityMs_->StartUIAbilitiesHandleWant(want, callerToken, abilityRequestList),
        ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest StartUIAbilitiesHandleWant_007 end");
}

/*
 * Feature: AbilityManagerService
 * Name: StartUIAbilitiesHandleWant_008
 * Function: StartUIAbilitiesHandleWant
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartUIAbilitiesHandleWant ImplicitStartError
 */
HWTEST_F(AbilityManagerServiceThirteenthTest, StartUIAbilitiesHandleWant_008, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest StartUIAbilitiesHandleWant_008 start");

    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);

    std::vector<AbilityRequest> abilityRequestList;
    std::string requestKey;
    MyStatus::GetInstance().isSupportStartAbilities_ = true;
    MyStatus::GetInstance().sbjIsSceneBoardEnabled_ = true;
    MyStatus::GetInstance().smhVerificationAllToken_ = true;

    Want want;
    AbilityRequest abilityRequest;
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    EXPECT_NE(abilityRecord, nullptr);
    sptr<IRemoteObject> callerToken = abilityRecord->GetToken();
    int32_t userId = DEFAULT_INVAL_VALUE;
    MyStatus::GetInstance().permPermission_ = true;

    EXPECT_EQ(abilityMs_->StartUIAbilitiesHandleWant(want, callerToken, abilityRequestList),
        START_UI_ABILITIES_NOT_SUPPORT_IMPLICIT_START);

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest StartUIAbilitiesHandleWant_008 end");
}

/*
 * Feature: AbilityManagerService
 * Name: StartUIAbilitiesHandleWant_009
 * Function: StartUIAbilitiesHandleWant
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartUIAbilitiesHandleWant generateRequestPass
 */
HWTEST_F(AbilityManagerServiceThirteenthTest, StartUIAbilitiesHandleWant_009, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest StartUIAbilitiesHandleWant_009 start");

    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);

    std::vector<AbilityRequest> abilityRequestList;
    std::string requestKey;
    MyStatus::GetInstance().isSupportStartAbilities_ = true;
    MyStatus::GetInstance().sbjIsSceneBoardEnabled_ = true;
    MyStatus::GetInstance().smhVerificationAllToken_ = true;

    Want want;
    AbilityRequest abilityRequest;
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    EXPECT_NE(abilityRecord, nullptr);
    sptr<IRemoteObject> callerToken = abilityRecord->GetToken();
    int32_t userId = DEFAULT_INVAL_VALUE;
    MyStatus::GetInstance().permPermission_ = true;
    want.SetElementName("com.ohos.test", "MainAbility");

    EXPECT_EQ(abilityMs_->StartUIAbilitiesHandleWant(want, callerToken, abilityRequestList),
        RESOLVE_ABILITY_ERR);

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest StartUIAbilitiesHandleWant_009 end");
}

#ifdef WITH_DLP
/*
 * Feature: AbilityManagerService
 * Name: StartUIAbilitiesCheckDlp_001
 * Function: StartUIAbilitiesCheckDlp
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartUIAbilitiesCheckDlp handleDLP == true
 */
HWTEST_F(AbilityManagerServiceThirteenthTest, StartUIAbilitiesCheckDlp_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest StartUIAbilitiesCheckDlp_001 start");

    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);

    std::vector<AbilityRequest> abilityRequestList;
    std::string requestKey;
    sptr<IRemoteObject> callerToken = MockToken(AbilityType::EXTENSION);
    MyStatus::GetInstance().isSupportStartAbilities_ = true;
    MyStatus::GetInstance().sbjIsSceneBoardEnabled_ = true;
    MyStatus::GetInstance().smhVerificationAllToken_ = true;

    Want want;
    want.SetParam(DLP_PARAMS_SANDBOX, true);
    want.SetElementName("com.ohos.test", "MainAbility");
    int32_t userId = DEFAULT_INVAL_VALUE;


    EXPECT_EQ(abilityMs_->StartUIAbilitiesCheckDlp(want, callerToken, userId),
        START_UI_ABILITIES_NOT_SUPPORT_DLP);

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest StartUIAbilitiesCheckDlp_001 end");
}

/*
 * Feature: AbilityManagerService
 * Name: StartUIAbilitiesCheckDlp_002
 * Function: StartUIAbilitiesCheckDlp
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartUIAbilitiesCheckDlp false false false
 */
HWTEST_F(AbilityManagerServiceThirteenthTest, StartUIAbilitiesCheckDlp_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest StartUIAbilitiesCheckDlp_002 start");

    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);

    std::vector<AbilityRequest> abilityRequestList;
    std::string requestKey;
    MyStatus::GetInstance().isSupportStartAbilities_ = true;
    MyStatus::GetInstance().sbjIsSceneBoardEnabled_ = true;
    MyStatus::GetInstance().smhVerificationAllToken_ = true;

    Want want;
    AbilityRequest abilityRequest;
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    EXPECT_NE(abilityRecord, nullptr);
    sptr<IRemoteObject> callerToken = abilityRecord->GetToken();
    int32_t userId = DEFAULT_INVAL_VALUE;
    MyStatus::GetInstance().permPermission_ = true;

    EXPECT_EQ(abilityMs_->StartUIAbilitiesCheckDlp(want, callerToken, userId),
        ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest StartUIAbilitiesCheckDlp_002 end");
}

/*
 * Feature: AbilityManagerService
 * Name: StartUIAbilitiesCheckDlp_003
 * Function: StartUIAbilitiesCheckDlp
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartUIAbilitiesCheckDlp true false false
 */
HWTEST_F(AbilityManagerServiceThirteenthTest, StartUIAbilitiesCheckDlp_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest StartUIAbilitiesCheckDlp_003 start");

    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);

    std::vector<AbilityRequest> abilityRequestList;
    std::string requestKey;
    MyStatus::GetInstance().isSupportStartAbilities_ = true;
    MyStatus::GetInstance().sbjIsSceneBoardEnabled_ = true;
    MyStatus::GetInstance().smhVerificationAllToken_ = true;

    Want want;
    want.SetParam(DLP_INDEX, 1);
    MyStatus::GetInstance().permPermission_ = false;
    int32_t userId = DEFAULT_INVAL_VALUE;
    sptr<IRemoteObject> callerToken = MockToken(AbilityType::EXTENSION);

    EXPECT_EQ(abilityMs_->StartUIAbilitiesCheckDlp(want, callerToken, userId),
        CHECK_PERMISSION_FAILED);

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest StartUIAbilitiesCheckDlp_003 end");
}

/*
 * Feature: AbilityManagerService
 * Name: StartUIAbilitiesCheckDlp_004
 * Function: StartUIAbilitiesCheckDlp
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartUIAbilitiesCheckDlp true true false
 */
HWTEST_F(AbilityManagerServiceThirteenthTest, StartUIAbilitiesCheckDlp_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest StartUIAbilitiesCheckDlp_004 start");

    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);

    std::vector<AbilityRequest> abilityRequestList;
    std::string requestKey;
    MyStatus::GetInstance().isSupportStartAbilities_ = true;
    MyStatus::GetInstance().sbjIsSceneBoardEnabled_ = true;
    MyStatus::GetInstance().smhVerificationAllToken_ = true;

    Want want;
    want.SetParam(DLP_INDEX, 1);
    MyStatus::GetInstance().permPermission_ = false;
    int32_t userId = 0;
    sptr<IRemoteObject> callerToken = MockToken(AbilityType::EXTENSION);

    EXPECT_EQ(abilityMs_->StartUIAbilitiesCheckDlp(want, callerToken, userId),
        CHECK_PERMISSION_FAILED);

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest StartUIAbilitiesCheckDlp_004 end");
}
#endif // WITH_DLP

/*
 * Feature: AbilityManagerService
 * Name: StartUIAbilitiesInterceptorCheck_001
 * Function: StartUIAbilitiesInterceptorCheck
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartUIAbilitiesInterceptorCheck generateRequestPass
 */
HWTEST_F(AbilityManagerServiceThirteenthTest, StartUIAbilitiesInterceptorCheck_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest StartUIAbilitiesInterceptorCheck_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);

    MyStatus::GetInstance().isSupportStartAbilities_ = true;
    MyStatus::GetInstance().sbjIsSceneBoardEnabled_ = true;
    MyStatus::GetInstance().smhVerificationAllToken_ = true;

    int32_t appIndex = 0;
    Want want;
    AbilityRequest abilityRequest;
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    EXPECT_NE(abilityRecord, nullptr);
    sptr<IRemoteObject> callerToken = abilityRecord->GetToken();
    int32_t userId = DEFAULT_INVAL_VALUE;
    MyStatus::GetInstance().permPermission_ = true;
    want.SetElementName("com.ohos.test", "MainAbility");

    EXPECT_EQ(abilityMs_->StartUIAbilitiesInterceptorCheck(want, abilityRequest, callerToken, appIndex, userId),
        START_UI_ABILITIES_INTERCEPTOR_CHECK_FAILED);

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest StartUIAbilitiesInterceptorCheck_001 end");
}

/*
 * Feature: AbilityManagerService
 * Name: DelegatorDoAbilityForeground_001
 * Function: DelegatorDoAbilityForeground
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService DelegatorDoAbilityForeground
 */
HWTEST_F(AbilityManagerServiceThirteenthTest, DelegatorDoAbilityForeground_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest DelegatorDoAbilityForeground_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);

    sptr<IRemoteObject> token = nullptr;
    EXPECT_EQ(abilityMs_->DelegatorDoAbilityForeground(token), ERR_INVALID_VALUE);

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest DelegatorDoAbilityForeground_001 end");
}

/*
 * Feature: AbilityManagerService
 * Name: DelegatorDoAbilityForeground_002
 * Function: DelegatorDoAbilityForeground
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService DelegatorDoAbilityForeground
 */
HWTEST_F(AbilityManagerServiceThirteenthTest, DelegatorDoAbilityForeground_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest DelegatorDoAbilityForeground_002 start");
    MyStatus::GetInstance().arGetAbilityRecord_ = nullptr;

    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);

    sptr<IRemoteObject> token = MockToken(AbilityType::PAGE);
    EXPECT_EQ(abilityMs_->DelegatorDoAbilityForeground(token), ERR_INVALID_VALUE);

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest DelegatorDoAbilityForeground_002 end");
}

/*
 * Feature: AbilityManagerService
 * Name: DelegatorDoAbilityForeground_003
 * Function: DelegatorDoAbilityForeground
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService DelegatorDoAbilityForeground
 */
HWTEST_F(AbilityManagerServiceThirteenthTest, DelegatorDoAbilityForeground_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest DelegatorDoAbilityForeground_003 start");

    Want want;
    OHOS::AppExecFwk::AbilityInfo abilityInfo;
    OHOS::AppExecFwk::ApplicationInfo applicationInfo;
    std::shared_ptr<AbilityRecord> record = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    EXPECT_NE(record, nullptr);
    record->pid_ = 0;
    MyStatus::GetInstance().arGetAbilityRecord_ = record;

    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);

    sptr<IRemoteObject> token = MockToken(AbilityType::PAGE);
    EXPECT_EQ(abilityMs_->DelegatorDoAbilityForeground(token), ERR_INVALID_VALUE);

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest DelegatorDoAbilityForeground_003 end");
}

/*
 * Feature: AbilityManagerService
 * Name: DelegatorDoAbilityForeground_004
 * Function: DelegatorDoAbilityForeground
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService DelegatorDoAbilityForeground
 */
HWTEST_F(AbilityManagerServiceThirteenthTest, DelegatorDoAbilityForeground_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest DelegatorDoAbilityForeground_004 start");
    Want want;
    OHOS::AppExecFwk::AbilityInfo abilityInfo;
    OHOS::AppExecFwk::ApplicationInfo applicationInfo;
    std::shared_ptr<AbilityRecord> record = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    EXPECT_NE(record, nullptr);
    record->pid_ = 1;
    MyStatus::GetInstance().arGetAbilityRecord_ = record;
    MyStatus::GetInstance().sbjIsSceneBoardEnabled_ = true;
    MyStatus::GetInstance().ualmGetSessionIdByAbilityToken_ = 0;

    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);

    sptr<IRemoteObject> token = MockToken(AbilityType::PAGE);
    EXPECT_EQ(abilityMs_->DelegatorDoAbilityForeground(token), ERR_INVALID_VALUE);

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest DelegatorDoAbilityForeground_004 end");
}

/*
 * Feature: AbilityManagerService
 * Name: DelegatorDoAbilityForeground_005
 * Function: DelegatorDoAbilityForeground
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService DelegatorDoAbilityForeground
 */
HWTEST_F(AbilityManagerServiceThirteenthTest, DelegatorDoAbilityForeground_005, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest DelegatorDoAbilityForeground_005 start");
    Want want;
    OHOS::AppExecFwk::AbilityInfo abilityInfo;
    OHOS::AppExecFwk::ApplicationInfo applicationInfo;
    std::shared_ptr<AbilityRecord> record = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    record->ownerMissionUserId_ = 1;
    EXPECT_NE(record, nullptr);
    record->pid_ = 1;
    MyStatus::GetInstance().arGetAbilityRecord_ = record;
    MyStatus::GetInstance().sbjIsSceneBoardEnabled_ = true;
    MyStatus::GetInstance().smhGetUIAbilityManagerByUserId_ = true;
    MyStatus::GetInstance().ualmGetSessionIdByAbilityToken_ = 1;

    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);

    sptr<IRemoteObject> token = MockToken(AbilityType::PAGE);
    EXPECT_EQ(abilityMs_->DelegatorDoAbilityForeground(token), ERR_INVALID_VALUE);

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest DelegatorDoAbilityForeground_005 end");
}

/*
 * Feature: AbilityManagerService
 * Name: DelegatorDoAbilityForeground_006
 * Function: DelegatorDoAbilityForeground
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService DelegatorDoAbilityForeground
 */
HWTEST_F(AbilityManagerServiceThirteenthTest, DelegatorDoAbilityForeground_006, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest DelegatorDoAbilityForeground_006 start");
    Want want;
    OHOS::AppExecFwk::AbilityInfo abilityInfo;
    OHOS::AppExecFwk::ApplicationInfo applicationInfo;
    std::shared_ptr<AbilityRecord> record = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    record->ownerMissionUserId_ = 1;
    EXPECT_NE(record, nullptr);
    record->pid_ = 1;
    MyStatus::GetInstance().arGetAbilityRecord_ = record;
    MyStatus::GetInstance().sbjIsSceneBoardEnabled_ = false;
    MyStatus::GetInstance().smhGetUIAbilityManagerByUserId_ = true;
    MyStatus::GetInstance().ualmGetSessionIdByAbilityToken_ = 1;

    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);

    sptr<IRemoteObject> token = MockToken(AbilityType::PAGE);
    EXPECT_EQ(abilityMs_->DelegatorDoAbilityForeground(token), ERR_INVALID_VALUE);

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest DelegatorDoAbilityForeground_006 end");
}

/*
 * Feature: AbilityManagerService
 * Name: AddWatchParameters_001
 * Function: AddWatchParameters
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService AddWatchParameters
 */
HWTEST_F(AbilityManagerServiceThirteenthTest, AddWatchParameters_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest AddWatchParameters_001 start");

    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    abilityMs_->AddWatchParameters();

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest AddWatchParameters_001 end");
}

/*
 * Feature: AbilityManagerService
 * Name: DoAbilityForeground_001
 * Function: DoAbilityForeground
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService DoAbilityForeground
 */
HWTEST_F(AbilityManagerServiceThirteenthTest, DoAbilityForeground_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest DoAbilityForeground_001 start");

    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);

    sptr<IRemoteObject> token = nullptr;
    uint32_t flag = 0;
    EXPECT_EQ(abilityMs_->DoAbilityForeground(token, flag), ERR_INVALID_VALUE);

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest DoAbilityForeground_001 end");
}

/*
 * Feature: AbilityManagerService
 * Name: DoAbilityForeground_002
 * Function: DoAbilityForeground
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService DoAbilityForeground
 */
HWTEST_F(AbilityManagerServiceThirteenthTest, DoAbilityForeground_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest DoAbilityForeground_002 start");
    MyStatus::GetInstance().arGetAbilityInfo_.type = AbilityType::UNKNOWN;
    MyStatus::GetInstance().smhVerificationAllToken_ = false;

    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);

    auto mockSubManagersHelper = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    EXPECT_NE(mockSubManagersHelper, nullptr);
    auto mockCurrentUIAbilityManager = std::make_shared<UIAbilityLifecycleManager>(0);
    EXPECT_NE(mockCurrentUIAbilityManager, nullptr);
    abilityMs_->subManagersHelper_ = mockSubManagersHelper;
    abilityMs_->subManagersHelper_->currentUIAbilityManager_ = mockCurrentUIAbilityManager;

    sptr<IRemoteObject> token = MockToken(AbilityType::PAGE);
    uint32_t flag = 0;
    EXPECT_EQ(abilityMs_->DoAbilityForeground(token, flag), ERR_INVALID_VALUE);

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest DoAbilityForeground_002 end");
}

/*
 * Feature: AbilityManagerService
 * Name: DoAbilityForeground_003
 * Function: DoAbilityForeground
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService DoAbilityForeground
 */
HWTEST_F(AbilityManagerServiceThirteenthTest, DoAbilityForeground_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest DoAbilityForeground_003 start");
    MyStatus::GetInstance().arGetAbilityInfo_.type = AbilityType::UNKNOWN;
    MyStatus::GetInstance().smhVerificationAllToken_ = true;
    MyStatus::GetInstance().arGetAbilityRecord_ = nullptr;
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);

    auto mockSubManagersHelper = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    EXPECT_NE(mockSubManagersHelper, nullptr);
    auto mockCurrentUIAbilityManager = std::make_shared<UIAbilityLifecycleManager>(0);
    EXPECT_NE(mockCurrentUIAbilityManager, nullptr);
    abilityMs_->subManagersHelper_ = mockSubManagersHelper;
    abilityMs_->subManagersHelper_->currentUIAbilityManager_ = mockCurrentUIAbilityManager;

    sptr<IRemoteObject> token = MockToken(AbilityType::PAGE);
    uint32_t flag = 0;
    EXPECT_EQ(abilityMs_->DoAbilityForeground(token, flag), ERR_INVALID_VALUE);

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest DoAbilityForeground_003 end");
}

/*
 * Feature: AbilityManagerService
 * Name: DoAbilityForeground_004
 * Function: DoAbilityForeground
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService DoAbilityForeground
 */
HWTEST_F(AbilityManagerServiceThirteenthTest, DoAbilityForeground_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest DoAbilityForeground_004 start");
    MyStatus::GetInstance().arGetAbilityInfo_.type = AbilityType::UNKNOWN;
    MyStatus::GetInstance().smhVerificationAllToken_ = true;
    Want want;
    OHOS::AppExecFwk::AbilityInfo abilityInfo;
    OHOS::AppExecFwk::ApplicationInfo applicationInfo;
    std::shared_ptr<AbilityRecord> record = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    EXPECT_NE(record, nullptr);
    record->ownerMissionUserId_ = 1;
    MyStatus::GetInstance().arGetAbilityRecord_ = record;

    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);

    auto mockSubManagersHelper = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    EXPECT_NE(mockSubManagersHelper, nullptr);
    auto mockCurrentUIAbilityManager = std::make_shared<UIAbilityLifecycleManager>(0);
    EXPECT_NE(mockCurrentUIAbilityManager, nullptr);
    abilityMs_->subManagersHelper_ = mockSubManagersHelper;
    abilityMs_->subManagersHelper_->currentUIAbilityManager_ = mockCurrentUIAbilityManager;

    sptr<IRemoteObject> token = MockToken(AbilityType::PAGE);
    uint32_t flag = 0;
    EXPECT_EQ(abilityMs_->DoAbilityForeground(token, flag), ERR_WRONG_INTERFACE_CALL);

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest DoAbilityForeground_004 end");
}

/*
 * Feature: AbilityManagerService
 * Name: DoAbilityForeground_005
 * Function: DoAbilityForeground
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService DoAbilityForeground
 */
HWTEST_F(AbilityManagerServiceThirteenthTest, DoAbilityForeground_005, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest DoAbilityForeground_005 start");
    MyStatus::GetInstance().arGetAbilityInfo_.type = AbilityType::PAGE;
    MyStatus::GetInstance().smhVerificationAllToken_ = true;
    Want want;
    OHOS::AppExecFwk::AbilityInfo abilityInfo;
    OHOS::AppExecFwk::ApplicationInfo applicationInfo;
    auto record = std::make_shared<UIAbilityRecord>(want, abilityInfo, applicationInfo, -1);
    EXPECT_NE(record, nullptr);
    MyStatus::GetInstance().ualmGetAbilityRecordByToken_ = record;
    MyStatus::GetInstance().smhGetUIAbilityManagerByUserId_ = true;
    MyStatus::GetInstance().smhGetMissionListManagerByUserId_ = true;

    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    auto mockSubManagersHelper = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    EXPECT_NE(mockSubManagersHelper, nullptr);
    auto mockCurrentUIAbilityManager = std::make_shared<UIAbilityLifecycleManager>(0);
    EXPECT_NE(mockCurrentUIAbilityManager, nullptr);
    auto currentMissionListManager = std::make_shared<MissionListManager>(0);
    EXPECT_NE(currentMissionListManager, nullptr);
    abilityMs_->subManagersHelper_ = mockSubManagersHelper;
    abilityMs_->subManagersHelper_->currentUIAbilityManager_ = mockCurrentUIAbilityManager;
    abilityMs_->subManagersHelper_->currentMissionListManager_ = currentMissionListManager;
    
    sptr<IRemoteObject> token = MockToken(AbilityType::PAGE);
    uint32_t flag = 0;
    EXPECT_EQ(abilityMs_->DoAbilityForeground(token, flag), ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest DoAbilityForeground_005 end");
}

/*
 * Feature: AbilityManagerService
 * Name: RequestDialogServiceInner_001
 * Function: RequestDialogServiceInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService RequestDialogServiceInner
 */
HWTEST_F(AbilityManagerServiceThirteenthTest, RequestDialogServiceInner_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest RequestDialogServiceInner_001 start");

    Want want;
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    sptr<IRemoteObject> token = nullptr;
    uint32_t flag = 0;
    EXPECT_EQ(abilityMs_->RequestDialogServiceInner(want, token, 0, 0), ERR_INVALID_CALLER);

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest RequestDialogServiceInner_001 end");
}

/*
 * Feature: AbilityManagerService
 * Name: RequestDialogServiceInner_002
 * Function: RequestDialogServiceInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService RequestDialogServiceInner
 */
HWTEST_F(AbilityManagerServiceThirteenthTest, RequestDialogServiceInner_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest RequestDialogServiceInner_002 start");

    MyStatus::GetInstance().smhVerificationAllToken_ = false;
    Want want;
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    sptr<IRemoteObject> token = MockToken(AbilityType::PAGE);
    uint32_t flag = 0;
    EXPECT_EQ(abilityMs_->RequestDialogServiceInner(want, token, 0, 0), ERR_INVALID_CALLER);

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest RequestDialogServiceInner_002 end");
}

/*
 * Feature: AbilityManagerService
 * Name: RequestDialogServiceInner_003
 * Function: RequestDialogServiceInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService RequestDialogServiceInner
 */
HWTEST_F(AbilityManagerServiceThirteenthTest, RequestDialogServiceInner_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest RequestDialogServiceInner_003 start");

    MyStatus::GetInstance().smhVerificationAllToken_ = true;
    Want want;
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    auto mockSubManagersHelper = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    EXPECT_NE(mockSubManagersHelper, nullptr);
    abilityMs_->subManagersHelper_ = mockSubManagersHelper;
    sptr<IRemoteObject> token = MockToken(AbilityType::PAGE);
    uint32_t flag = 0;
    EXPECT_EQ(abilityMs_->RequestDialogServiceInner(want, token, 0, 0), ERR_INVALID_VALUE);

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest RequestDialogServiceInner_003 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetAllInsightIntentInfo
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetAllInsightIntentInfo
 */
HWTEST_F(AbilityManagerServiceThirteenthTest, GetAllInsightIntentInfo_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest GetAllInsightIntentInfo_001 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs, nullptr);
    auto flag = AbilityRuntime::GetInsightIntentFlag::GET_FULL_INSIGHT_INTENT;
    std::vector<InsightIntentInfoForQuery> infos;
    int32_t userId = 1;
    auto ret = abilityMs->GetAllInsightIntentInfo(flag, infos, userId);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest GetAllInsightIntentInfo_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetAllInsightIntentInfo
 * SubFunction: flag is GET_FULL_INSIGHT_INTENT or GET_ENTITY_INFO
 * FunctionPoints: AbilityManagerService GetAllInsightIntentInfo
 */
HWTEST_F(AbilityManagerServiceThirteenthTest, GetAllInsightIntentInfo_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest GetAllInsightIntentInfo_002 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs, nullptr);
    auto flag = static_cast<GetInsightIntentFlag>(AbilityRuntime::GetInsightIntentFlag::GET_FULL_INSIGHT_INTENT |
                AbilityRuntime::GetInsightIntentFlag::GET_ENTITY_INFO);
    std::vector<InsightIntentInfoForQuery> infos;
    int32_t userId = 1;
    auto ret = abilityMs->GetAllInsightIntentInfo(flag, infos, userId);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest GetAllInsightIntentInfo_002 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetAllInsightIntentInfo
 * SubFunction: flag is GET_SUMMARY_INSIGHT_INTENT or GET_ENTITY_INFO
 * FunctionPoints: AbilityManagerService GetAllInsightIntentInfo
 */
HWTEST_F(AbilityManagerServiceThirteenthTest, GetAllInsightIntentInfo_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest GetAllInsightIntentInfo_003 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs, nullptr);
    auto flag = static_cast<GetInsightIntentFlag>(AbilityRuntime::GetInsightIntentFlag::GET_SUMMARY_INSIGHT_INTENT |
                AbilityRuntime::GetInsightIntentFlag::GET_ENTITY_INFO);
    std::vector<InsightIntentInfoForQuery> infos;
    int32_t userId = 1;
    auto ret = abilityMs->GetAllInsightIntentInfo(flag, infos, userId);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest GetAllInsightIntentInfo_003 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetAllInsightIntentInfo
 * SubFunction: invalid flag
 * FunctionPoints: AbilityManagerService GetAllInsightIntentInfo
 */
HWTEST_F(AbilityManagerServiceThirteenthTest, GetAllInsightIntentInfo_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest GetAllInsightIntentInfo_004 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs, nullptr);
    auto flag = AbilityRuntime::GetInsightIntentFlag::GET_ENTITY_INFO;
    std::vector<InsightIntentInfoForQuery> infos;
    int32_t userId = 1;
    auto ret = abilityMs->GetAllInsightIntentInfo(flag, infos, userId);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest GetAllInsightIntentInfo_004 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetInsightIntentInfoByBundleName
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetInsightIntentInfoByBundleName
 */
HWTEST_F(AbilityManagerServiceThirteenthTest, GetInsightIntentInfoByBundleName_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest GetInsightIntentInfoByBundleName start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs, nullptr);
    auto flag = AbilityRuntime::GetInsightIntentFlag::GET_FULL_INSIGHT_INTENT;
    std::string bundleName = "com.example.bundleName";
    std::vector<InsightIntentInfoForQuery> infos;
    int32_t userId = 1;
    auto ret = abilityMs->GetInsightIntentInfoByBundleName(flag, bundleName, infos, userId);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest GetInsightIntentInfoByBundleName end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetInsightIntentInfoByBundleName
 * SubFunction: flag is GET_FULL_INSIGHT_INTENT or GET_ENTITY_INFO
 * FunctionPoints: AbilityManagerService GetInsightIntentInfoByBundleName
 */
HWTEST_F(AbilityManagerServiceThirteenthTest, GetInsightIntentInfoByBundleName_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest GetInsightIntentInfoByBundleName_002 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs, nullptr);
    auto flag = static_cast<GetInsightIntentFlag>(AbilityRuntime::GetInsightIntentFlag::GET_FULL_INSIGHT_INTENT |
                AbilityRuntime::GetInsightIntentFlag::GET_ENTITY_INFO);
    std::string bundleName = "com.example.bundleName";
    std::vector<InsightIntentInfoForQuery> infos;
    int32_t userId = 1;
    auto ret = abilityMs->GetInsightIntentInfoByBundleName(flag, bundleName, infos, userId);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest GetInsightIntentInfoByBundleName_002 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetInsightIntentInfoByBundleName
 * SubFunction: flag is GET_SUMMARY_INSIGHT_INTENT or GET_ENTITY_INFO
 * FunctionPoints: AbilityManagerService GetInsightIntentInfoByBundleName
 */
HWTEST_F(AbilityManagerServiceThirteenthTest, GetInsightIntentInfoByBundleName_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest GetInsightIntentInfoByBundleName_003 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs, nullptr);
    auto flag = static_cast<GetInsightIntentFlag>(AbilityRuntime::GetInsightIntentFlag::GET_SUMMARY_INSIGHT_INTENT |
                AbilityRuntime::GetInsightIntentFlag::GET_ENTITY_INFO);
    std::string bundleName = "com.example.bundleName";
    std::vector<InsightIntentInfoForQuery> infos;
    int32_t userId = 1;
    auto ret = abilityMs->GetInsightIntentInfoByBundleName(flag, bundleName, infos, userId);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest GetInsightIntentInfoByBundleName_003 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetInsightIntentInfoByBundleName
 * SubFunction: invalid flag
 * FunctionPoints: AbilityManagerService GetInsightIntentInfoByBundleName
 */
HWTEST_F(AbilityManagerServiceThirteenthTest, GetInsightIntentInfoByBundleName_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest GetInsightIntentInfoByBundleName_004 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs, nullptr);
    auto flag = AbilityRuntime::GetInsightIntentFlag::GET_ENTITY_INFO;
    std::string bundleName = "com.example.bundleName";
    std::vector<InsightIntentInfoForQuery> infos;
    int32_t userId = 1;
    auto ret = abilityMs->GetInsightIntentInfoByBundleName(flag, bundleName, infos, userId);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest GetInsightIntentInfoByBundleName_004 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetInsightIntentInfoByIntentName
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetInsightIntentInfoByIntentName
 */
HWTEST_F(AbilityManagerServiceThirteenthTest, GetInsightIntentInfoByIntentName_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest GetInsightIntentInfoByIntentName_001 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs, nullptr);
    auto flag = AbilityRuntime::GetInsightIntentFlag::GET_FULL_INSIGHT_INTENT;
    std::string bundleName = "com.example.bundleName";
    std::string moduleName = "entry";
    std::string intentName = "test";
    InsightIntentInfoForQuery info;
    int32_t userId = 1;
    auto ret = abilityMs->GetInsightIntentInfoByIntentName(flag, bundleName, moduleName, intentName, info, userId);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest GetInsightIntentInfoByIntentName_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetInsightIntentInfoByIntentName
 * SubFunction: flag is GET_FULL_INSIGHT_INTENT or GET_ENTITY_INFO
 * FunctionPoints: AbilityManagerService GetInsightIntentInfoByIntentName
 */
HWTEST_F(AbilityManagerServiceThirteenthTest, GetInsightIntentInfoByIntentName_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest GetInsightIntentInfoByIntentName_002 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs, nullptr);
    auto flag = static_cast<GetInsightIntentFlag>(AbilityRuntime::GetInsightIntentFlag::GET_FULL_INSIGHT_INTENT |
                AbilityRuntime::GetInsightIntentFlag::GET_ENTITY_INFO);
    std::string bundleName = "com.example.bundleName";
    std::string moduleName = "entry";
    std::string intentName = "test";
    InsightIntentInfoForQuery info;
    int32_t userId = 1;
    auto ret = abilityMs->GetInsightIntentInfoByIntentName(flag, bundleName, moduleName, intentName, info, userId);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest GetInsightIntentInfoByIntentName_002 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetInsightIntentInfoByIntentName
 * SubFunction: flag is GET_SUMMARY_INSIGHT_INTENT or GET_ENTITY_INFO
 * FunctionPoints: AbilityManagerService GetInsightIntentInfoByIntentName
 */
HWTEST_F(AbilityManagerServiceThirteenthTest, GetInsightIntentInfoByIntentName_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest GetInsightIntentInfoByIntentName_003 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs, nullptr);
    auto flag = static_cast<GetInsightIntentFlag>(AbilityRuntime::GetInsightIntentFlag::GET_SUMMARY_INSIGHT_INTENT |
                AbilityRuntime::GetInsightIntentFlag::GET_ENTITY_INFO);
    std::string bundleName = "com.example.bundleName";
    std::string moduleName = "entry";
    std::string intentName = "test";
    InsightIntentInfoForQuery info;
    int32_t userId = 1;
    auto ret = abilityMs->GetInsightIntentInfoByIntentName(flag, bundleName, moduleName, intentName, info, userId);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest GetInsightIntentInfoByIntentName_003 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetInsightIntentInfoByIntentName
 * SubFunction: invalid flag
 * FunctionPoints: AbilityManagerService GetInsightIntentInfoByIntentName
 */
HWTEST_F(AbilityManagerServiceThirteenthTest, GetInsightIntentInfoByIntentName_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest GetInsightIntentInfoByIntentName_004 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs, nullptr);
    auto flag = AbilityRuntime::GetInsightIntentFlag::GET_ENTITY_INFO;
    std::string bundleName = "com.example.bundleName";
    std::string moduleName = "entry";
    std::string intentName = "test";
    InsightIntentInfoForQuery info;
    int32_t userId = 1;
    auto ret = abilityMs->GetInsightIntentInfoByIntentName(flag, bundleName, moduleName, intentName, info, userId);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest GetInsightIntentInfoByIntentName_004 end");
}

/*
* Feature: AbilityManagerService
* Function: IntentOpenLinkInner
* FunctionPoints: AbilityManagerService IntentOpenLinkInner
*/
HWTEST_F(AbilityManagerServiceThirteenthTest, AbilityManagerServiceTest_IntentOpenLinkInner_001, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs, nullptr);
    AppExecFwk::InsightIntentExecuteParam param{};
    param.insightIntentParam_ = std::make_shared<WantParams>();
    param.uris_.push_back("");
    AbilityRuntime::ExtractInsightIntentGenericInfo info{};
    auto paramPtr = std::make_shared<InsightIntentExecuteParam>(param);
    int res = abilityMs->IntentOpenLinkInner(paramPtr, info, -1);
    EXPECT_NE(res, ERR_OK);
}

/*
* Feature: AbilityManagerService
* Function: IntentOpenLinkInner
* FunctionPoints: AbilityManagerService IntentOpenLinkInner
*/
HWTEST_F(AbilityManagerServiceThirteenthTest, AbilityManagerServiceTest_IntentOpenLinkInner_002, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs, nullptr);
    AppExecFwk::InsightIntentExecuteParam param{};
    param.insightIntentParam_ = std::make_shared<WantParams>();
    param.uris_.push_back("123");
    AbilityRuntime::ExtractInsightIntentGenericInfo info{};
    auto paramPtr = std::make_shared<InsightIntentExecuteParam>(param);
    int res = abilityMs->IntentOpenLinkInner(paramPtr, info, -1);
    EXPECT_NE(res, ERR_OK);
}

/*
* Feature: AbilityManagerService
* Function: IntentOpenLinkInner
* FunctionPoints: AbilityManagerService IntentOpenLinkInner
*/
HWTEST_F(AbilityManagerServiceThirteenthTest, AbilityManagerServiceTest_IntentOpenLinkInner_003, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs, nullptr);
    AppExecFwk::InsightIntentExecuteParam param{};
    param.insightIntentParam_ = std::make_shared<WantParams>();
    param.uris_.push_back("123");
    AbilityRuntime::ExtractInsightIntentGenericInfo info{};
    auto refLinkInfo = info.get<InsightIntentLinkInfo>();
    refLinkInfo.uri = "123";
    auto paramPtr = std::make_shared<InsightIntentExecuteParam>(param);
    int res = abilityMs->IntentOpenLinkInner(paramPtr, info, -1);
    EXPECT_NE(res, ERR_OK);
}

/*
 * Feature: AbilityManagerService
 * Function: OnStartTest
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService OnStartTest
 */
HWTEST_F(AbilityManagerServiceThirteenthTest, OnStartTest_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest OnStartTest_001 start");

    auto abilityManagerService = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityManagerService, nullptr);
    abilityManagerService->OnStart();
    EXPECT_EQ(abilityManagerService->QueryServiceState(), ServiceRunningState::STATE_RUNNING);

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest OnStartTest_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: OnStartTest
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService OnStartTest
 */
HWTEST_F(AbilityManagerServiceThirteenthTest, OnStartTest_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest OnStartTest_002 start");

    auto abilityManagerService = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityManagerService, nullptr);
    auto taskHandler = TaskHandlerWrap::CreateQueueHandler(AbilityConfig::NAME_ABILITY_MGR_SERVICE);
    auto eventHandler = std::make_shared<AbilityEventHandler>(taskHandler, abilityManagerService);
    abilityManagerService->subManagersHelper_ = std::make_shared<SubManagersHelper>(taskHandler, eventHandler);
    EXPECT_NE(abilityManagerService->subManagersHelper_, nullptr);
    abilityManagerService->OnStart();
    EXPECT_EQ(abilityManagerService->QueryServiceState(), ServiceRunningState::STATE_RUNNING);

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest OnStartTest_002 end");
}

/*
 * Feature: AbilityManagerService
 * Function: RegisterSAInterceptor
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService RegisterSAInterceptor
 */
HWTEST_F(AbilityManagerServiceThirteenthTest, RegisterSAInterceptor_001, TestSize.Level1) {
    sptr<AbilityRuntime::ISAInterceptor> interceptor = new (std::nothrow) MockISAInterceptor();
    ASSERT_NE(interceptor, nullptr);
    auto abilityManagerService = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityManagerService, nullptr);
    auto result = abilityManagerService->RegisterSAInterceptor(interceptor);
    EXPECT_EQ(result, CHECK_PERMISSION_FAILED);
}

/*
 * Feature: AbilityManagerService
 * Function: RegisterSAInterceptor
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService RegisterSAInterceptor
 */
HWTEST_F(AbilityManagerServiceThirteenthTest, RegisterSAInterceptor_002, TestSize.Level1) {
    sptr<AbilityRuntime::ISAInterceptor> interceptor = new (std::nothrow) MockISAInterceptor();
    ASSERT_NE(interceptor, nullptr);
    auto abilityManagerService = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityManagerService, nullptr);
    MyStatus::GetInstance().ipcGetCallingUid_ = PENG_LAI_UID;
    auto result = abilityManagerService->RegisterSAInterceptor(interceptor);
    EXPECT_EQ(result, ERR_OK);
}

/*
 * Feature: ManualStartAutoStartupApps_001
 * Function: ManualStartAutoStartupApps
 * SubFunction: NA
 * FunctionPoints: CHECK_PERMISSION_FAILED
 */
HWTEST_F(AbilityManagerServiceThirteenthTest, ManualStartAutoStartupApps_001, TestSize.Level1) {
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest ManualStartAutoStartupApps_001 start");
    auto abilityManagerService = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityManagerService, nullptr);
    MyStatus::GetInstance().permPermission_ = 0;
    int32_t userId = -1;
    auto result = abilityManagerService->ManualStartAutoStartupApps(userId);
    EXPECT_EQ(result, CHECK_PERMISSION_FAILED);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest ManualStartAutoStartupApps_001 end");
}

/*
 * Feature: ManualStartAutoStartupApps_002
 * Function: ManualStartAutoStartupApps
 * SubFunction: NA
 * FunctionPoints: ERR_OK
 */
HWTEST_F(AbilityManagerServiceThirteenthTest, ManualStartAutoStartupApps_002, TestSize.Level1) {
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest ManualStartAutoStartupApps_002 start");
    auto abilityManagerService = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityManagerService, nullptr);
    MyStatus::GetInstance().permPermission_ = 1;
    int32_t userId = -1;
    auto result = abilityManagerService->ManualStartAutoStartupApps(userId);
    EXPECT_EQ(result, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest ManualStartAutoStartupApps_002 end");
}

/*
 * Feature: QueryCallerTokenIdForAnco_001
 * Function: QueryCallerTokenIdForAnco
 * SubFunction: NA
 * FunctionPoints: uiAbilityManager is null
 */
HWTEST_F(AbilityManagerServiceThirteenthTest, QueryCallerTokenIdForAnco_001, TestSize.Level1) {
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest QueryCallerTokenIdForAnco_001 start");
    auto abilityManagerService = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityManagerService, nullptr);
    MyStatus::GetInstance().ipcGetCallingUid_ = FOUNDATION_UID;
    int32_t userId = 101;
    std::string asCallerForAncoSessionId = "520";
    uint32_t callerTokenId = 0;
    auto result = abilityManagerService->QueryCallerTokenIdForAnco(userId, asCallerForAncoSessionId, callerTokenId);
    EXPECT_EQ(result, CHECK_PERMISSION_FAILED);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest QueryCallerTokenIdForAnco_001 end");
}

/*
 * Feature: QueryCallerTokenIdForAnco_002
 * Function: QueryCallerTokenIdForAnco
 * SubFunction: NA
 * FunctionPoints: negative dialogSessionId
 */
HWTEST_F(AbilityManagerServiceThirteenthTest, QueryCallerTokenIdForAnco_002, TestSize.Level1) {
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest QueryCallerTokenIdForAnco_002 start");
    auto abilityManagerService = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityManagerService, nullptr);
    MyStatus::GetInstance().ipcGetCallingUid_ = BROKER_UID;
    int32_t userId = -1;
    std::string asCallerForAncoSessionId = "5201314";
    uint32_t callerTokenId = 0;
    auto result = abilityManagerService->QueryCallerTokenIdForAnco(userId, asCallerForAncoSessionId, callerTokenId);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirteenthTest QueryCallerTokenIdForAnco_002 end");
}

/*
 * Feature: AbilityManagerService
 * Function: KillAppWithReason
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService KillAppWithReason
 */
HWTEST_F(AbilityManagerServiceThirteenthTest, KillAppWithReason_0100, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs, nullptr);
    int32_t pid = -1;
    MyStatus::GetInstance().permPermission_ = 2;
    abilityMs->appExitReasonHelper_ = std::make_shared<AppExitReasonHelper>(nullptr);
    ExitReasonCompability exitReason{};
    exitReason.reason = Reason::REASON_JS_ERROR;
    exitReason.exitMsg = "Js Error.";
    exitReason.subReason = 0;
    exitReason.shouldKillForeground = false;
    exitReason.killId = 1;
    auto res = abilityMs->KillAppWithReason(pid, exitReason);
    EXPECT_EQ(res, DEFAULT_INVAL_VALUE);
    MyStatus::GetInstance().permPermission_ = 0;
}

/*
 * Feature: AbilityManagerService
 * Function: KillAppWithReason
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService KillAppWithReason
 */
HWTEST_F(AbilityManagerServiceThirteenthTest, KillAppWithReason_0200, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs, nullptr);
    int32_t pid = 12345;
    MyStatus::GetInstance().permPermission_ = 2;
    abilityMs->appExitReasonHelper_ = std::make_shared<AppExitReasonHelper>(nullptr);
    ExitReasonCompability exitReason{};
    exitReason.reason = Reason::REASON_JS_ERROR;
    exitReason.exitMsg = "Js Error.";
    exitReason.subReason = 0;
    exitReason.shouldKillForeground = false;
    exitReason.killId = 1;
    auto res = abilityMs->KillAppWithReason(pid, exitReason);
    EXPECT_EQ(res, ERR_KILL_APP_WHILE_FOREGROUND);
    MyStatus::GetInstance().permPermission_ = 0;
}

/*
 * Feature: AbilityManagerService
 * Function: KillAppWithReason
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService KillAppWithReason
 */
HWTEST_F(AbilityManagerServiceThirteenthTest, KillAppWithReason_0300, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs, nullptr);
    int32_t pid = 654321;
    MyStatus::GetInstance().permPermission_ = 2;
    abilityMs->appExitReasonHelper_ = std::make_shared<AppExitReasonHelper>(nullptr);
    ExitReasonCompability exitReason{};
    exitReason.reason = Reason::REASON_JS_ERROR;
    exitReason.exitMsg = "Js Error.";
    exitReason.subReason = 0;
    exitReason.shouldKillForeground = false;
    exitReason.killId = 1;
    auto res = abilityMs->KillAppWithReason(pid, exitReason);
    EXPECT_EQ(res, ERR_OK);
    MyStatus::GetInstance().permPermission_ = 0;
}
} // namespace AAFwk
} // namespace OHOS
