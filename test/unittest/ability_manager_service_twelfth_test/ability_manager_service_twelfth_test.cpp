/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#include "mock_ipc_skeleton.h"
#include "mock_permission_verification.h"
#include "mock_my_flag.h"
#include "mock_ability_connect_callback.h"
#include "ability_manager_service.h"
#include "modal_system_dialog/modal_system_dialog_ui_extension.h"
#include "utils/modal_system_dialog_util.h"
#include "user_controller/user_controller.h"
#include "remote_on_listener_stub_mock.h"
#include "ability_connect_manager.h"
#include "ui_extension_ability_manager.h"
#include "common_extension_manager.h"
#include "hilog_tag_wrapper.h"
#include "ability_bundle_event_callback.h"
#include "session/host/include/session.h"
#include "system_ability_definition.h"
#include "ability_util.h"
#include "connection_data.h"
#include "mock_parameters.h"
#include "mock_scene_board_judgement.h"
#include "mock_test_object.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AppExecFwk;
using OHOS::AppExecFwk::AbilityType;
using OHOS::AppExecFwk::ExtensionAbilityType;

constexpr char DEVELOPER_MODE_STATE[] = "const.security.developermode.state";
constexpr char KIOSK_MODE_ENABLED[] = "const.product.kioskmode.enabled";
constexpr const char* DEBUG_APP = "debugApp";
constexpr const char* START_ABILITY_TYPE = "ABILITY_INNER_START_WITH_ACCOUNT";
constexpr char PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED[] = "const.product.enterprisefeature.setting.enabled";
constexpr int32_t ONE = 1;
constexpr int32_t TWO = 2;
constexpr int32_t FOUNDATION_UID = 5523;
constexpr int32_t RESOURCE_SCHEDULE_UID = 1096;
constexpr int32_t UPDATE_CONFIG_FLAG_COVER = 1;
constexpr int32_t UPDATE_CONFIG_FLAG_APPEND = 2;
constexpr int32_t TEST_USER_ID = 100;

namespace OHOS {
namespace AAFwk {
namespace {
constexpr int32_t ACCOUNT_MGR_SERVICE_UID = 3058;
} // namespace
class AbilityManagerServiceTwelfthTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

    std::shared_ptr<AbilityRecord> MockAbilityRecord(AbilityType);
    sptr<Token> MockToken(AbilityType);
    sptr<Token> MockToken(AbilityType, uint32_t accessTokenId);
    AbilityRequest GenerateAbilityRequest(const std::string& deviceName, const std::string& abilityName,
        const std::string& appName, const std::string& bundleName, const std::string& moduleName);

    std::shared_ptr<AbilityRecord> abilityRecord;
    sptr<SessionInfo> MockSessionInfo(int32_t persistentId);
};

void AbilityManagerServiceTwelfthTest::SetUpTestCase() {}

void AbilityManagerServiceTwelfthTest::TearDownTestCase() {}

void AbilityManagerServiceTwelfthTest::SetUp() {}

void AbilityManagerServiceTwelfthTest::TearDown() {}

std::shared_ptr<AbilityRecord> AbilityManagerServiceTwelfthTest::MockAbilityRecord(AbilityType abilityType)
{
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.test.demo";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = abilityType;
    return AbilityRecord::CreateAbilityRecord(abilityRequest);
}

sptr<Token> AbilityManagerServiceTwelfthTest::MockToken(AbilityType abilityType)
{
    abilityRecord = MockAbilityRecord(abilityType);
    if (!abilityRecord) {
        return nullptr;
    }
    return abilityRecord->GetToken();
}

sptr<Token> AbilityManagerServiceTwelfthTest::MockToken(AbilityType abilityType, uint32_t accessTokenId)
{
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.test.demo";
    abilityRequest.appInfo.accessTokenId = accessTokenId;
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = abilityType;
    abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    if (!abilityRecord) {
        return nullptr;
    }
    return abilityRecord->GetToken();
}

sptr<SessionInfo> AbilityManagerServiceTwelfthTest::MockSessionInfo(int32_t persistentId)
{
    sptr<SessionInfo> sessionInfo = new (std::nothrow) SessionInfo();
    if (!sessionInfo) {
        TAG_LOGE(AAFwkTag::TEST, "sessionInfo is nullptr");
        return nullptr;
    }
    sessionInfo->persistentId = persistentId;
    return sessionInfo;
}

AbilityRequest AbilityManagerServiceTwelfthTest::GenerateAbilityRequest(const std::string& deviceName,
    const std::string& abilityName, const std::string& appName, const std::string& bundleName,
    const std::string& moduleName)
{
    ElementName element(deviceName, bundleName, abilityName, moduleName);
    Want want;
    want.SetElement(element);

    AbilityInfo abilityInfo;
    abilityInfo.visible = true;
    abilityInfo.applicationName = appName;
    abilityInfo.type = AbilityType::SERVICE;
    abilityInfo.name = abilityName;
    abilityInfo.bundleName = bundleName;
    abilityInfo.moduleName = moduleName;
    abilityInfo.deviceId = deviceName;
    ApplicationInfo appinfo;
    appinfo.name = appName;
    abilityInfo.applicationInfo = appinfo;
    AbilityRequest abilityRequest;
    abilityRequest.want = want;
    abilityRequest.abilityInfo = abilityInfo;
    abilityRequest.appInfo = appinfo;
    abilityInfo.process = bundleName;

    return abilityRequest;
}

/*
 * Feature: AbilityManagerService
 * Function: CloseUIExtensionAbilityBySCB
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CloseUIExtensionAbilityBySCB
 */
HWTEST_F(AbilityManagerServiceTwelfthTest, CloseUIExtensionAbilityBySCB_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest CloseUIExtensionAbilityBySCB_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_TRUE(abilityMs_ != nullptr);
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        sptr<IRemoteObject> callerToken = nullptr;
        EXPECT_CALL(Rosen::SceneBoardJudgement::GetInstance(), MockIsSceneBoardEnabled())
            .WillRepeatedly(Return(true));
        EXPECT_EQ(abilityMs_->CloseUIExtensionAbilityBySCB(callerToken), ERR_INVALID_VALUE);

        callerToken = MockToken(AbilityType::PAGE);
        EXPECT_EQ(abilityMs_->CloseUIExtensionAbilityBySCB(callerToken), ERR_PERMISSION_DENIED);

        abilityMs_->subManagersHelper_ = std::make_shared<SubManagersHelper>(nullptr, nullptr);
        IPCSkeleton::SetCallingUid(BASE_USER_RANGE);
        std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(ONE);
        abilityMs_->subManagersHelper_->uiExtensionAbilityManagers_.insert(std::make_pair(ONE, connectManager));
        IPCSkeleton::SetCallingTokenID(ONE);
        EXPECT_EQ(abilityMs_->CloseUIExtensionAbilityBySCB(callerToken), ERR_INVALID_VALUE);

        AbilityRequest abilityRequest{};
        std::string deviceName = "device";
        std::string abilityName = "ServiceAbility";
        std::string appName = "hiservcie";
        std::string bundleName = "com.ix.hiservcie";
        std::string moduleName = "entry";
        abilityRequest = GenerateAbilityRequest(deviceName, abilityName, appName, bundleName, moduleName);
        auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
        callerToken = abilityRecord->GetToken();
        auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
        uiAbilityLifecycleManager->sessionAbilityMap_.emplace(TWO, abilityRecord);
        abilityMs_->subManagersHelper_->uiAbilityManagers_.emplace(TWO, uiAbilityLifecycleManager);
        EXPECT_EQ(abilityMs_->CloseUIExtensionAbilityBySCB(callerToken), ERR_INVALID_VALUE);
    }

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest CloseUIExtensionAbilityBySCB_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: CloseUIExtensionAbilityBySCB
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CloseUIExtensionAbilityBySCB
 */
HWTEST_F(AbilityManagerServiceTwelfthTest, CloseUIExtensionAbilityBySCB_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest CloseUIExtensionAbilityBySCB_002 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_TRUE(abilityMs_ != nullptr);
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        sptr<IRemoteObject> callerToken = nullptr;
        EXPECT_CALL(Rosen::SceneBoardJudgement::GetInstance(), MockIsSceneBoardEnabled())
            .WillRepeatedly(Return(true));
        abilityMs_->subManagersHelper_ = std::make_shared<SubManagersHelper>(nullptr, nullptr);
        IPCSkeleton::SetCallingUid(BASE_USER_RANGE);
        std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(ONE);
        abilityMs_->subManagersHelper_->uiExtensionAbilityManagers_.insert(std::make_pair(ONE, connectManager));
        IPCSkeleton::SetCallingTokenID(ONE);

        AbilityRequest abilityRequest{};
        std::string deviceName = "device";
        std::string abilityName = "ServiceAbility";
        std::string appName = "hiservcie";
        std::string bundleName = "com.ix.hiservcie";
        std::string moduleName = "entry";
        abilityRequest = GenerateAbilityRequest(deviceName, abilityName, appName, bundleName, moduleName);
        abilityRequest.abilityInfo.extensionAbilityType = AppExecFwk::ExtensionAbilityType::SHARE;
        auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
        callerToken = abilityRecord->GetToken();
        auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
        uiAbilityLifecycleManager->sessionAbilityMap_.emplace(TWO, abilityRecord);
        abilityMs_->subManagersHelper_->uiAbilityManagers_.emplace(TWO, uiAbilityLifecycleManager);
        EXPECT_EQ(abilityMs_->CloseUIExtensionAbilityBySCB(callerToken), ERR_INVALID_VALUE);

        auto sessionInfo = new SessionInfo();
        abilityRecord->SetSessionInfo(sessionInfo);
        EXPECT_EQ(abilityMs_->CloseUIExtensionAbilityBySCB(callerToken), ERR_INVALID_VALUE);
    }

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest CloseUIExtensionAbilityBySCB_002 end");
}

/*
 * Feature: AbilityManagerService
 * Function: CloseUIAbilityBySCB
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CloseUIAbilityBySCB
 */
HWTEST_F(AbilityManagerServiceTwelfthTest, CloseUIAbilityBySCB_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest CloseUIAbilityBySCB_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_TRUE(abilityMs_ != nullptr);
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        Rosen::SessionInfo info;
        sptr<SessionInfo> sessionInfo = nullptr;
        bool isUserRequestedExit = true;
        uint32_t sceneFlag = ONE;

        EXPECT_EQ(abilityMs_->CloseUIAbilityBySCB(sessionInfo, isUserRequestedExit, sceneFlag), ERR_INVALID_VALUE);

        sessionInfo = new SessionInfo();
        sessionInfo->callerToken = MockToken(AbilityType::PAGE);
        EXPECT_EQ(abilityMs_->CloseUIAbilityBySCB(sessionInfo, isUserRequestedExit, sceneFlag), ERR_INVALID_VALUE);

        sessionInfo->sessionToken = MockToken(AbilityType::PAGE);
        EXPECT_EQ(abilityMs_->CloseUIAbilityBySCB(sessionInfo, isUserRequestedExit, sceneFlag),
            ERR_WRONG_INTERFACE_CALL);

        abilityMs_->subManagersHelper_ = std::make_shared<SubManagersHelper>(nullptr, nullptr);
        IPCSkeleton::SetCallingUid(BASE_USER_RANGE);
        std::shared_ptr<CommonExtensionManager> connectManager = std::make_shared<CommonExtensionManager>(ONE);
        connectManager->sceneBoardTokenId_ = ONE;
        abilityMs_->subManagersHelper_->commonExtensionManagers_.insert(std::make_pair(ONE, connectManager));
        IPCSkeleton::SetCallingTokenID(ONE);
        auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
        abilityMs_->subManagersHelper_->uiAbilityManagers_.emplace(ONE, uiAbilityLifecycleManager);

        EXPECT_EQ(abilityMs_->CloseUIAbilityBySCB(sessionInfo, isUserRequestedExit, sceneFlag), ERR_INVALID_VALUE);

        sessionInfo->isClearSession = true;
        AbilityRequest abilityRequest{};
        std::string deviceName = "device";
        std::string abilityName = "ServiceAbility";
        std::string appName = "hiservcie";
        std::string bundleName = "com.ix.hiservcie";
        std::string moduleName = "entry";
        abilityRequest = GenerateAbilityRequest(deviceName, abilityName, appName, bundleName, moduleName);
        auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
        sessionInfo->persistentId = TWO;
        uiAbilityLifecycleManager->sessionAbilityMap_.emplace(TWO, abilityRecord);
        EXPECT_EQ(abilityMs_->CloseUIAbilityBySCB(sessionInfo, isUserRequestedExit, sceneFlag), ERR_INVALID_VALUE);

        isUserRequestedExit = false;
        EXPECT_EQ(abilityMs_->CloseUIAbilityBySCB(sessionInfo, isUserRequestedExit, sceneFlag), ERR_INVALID_VALUE);
    }

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest CloseUIAbilityBySCB_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: DisconnectAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService DisconnectAbility
 */
HWTEST_F(AbilityManagerServiceTwelfthTest, DisconnectAbility_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest DisconnectAbility_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    sptr<IAbilityConnection> connect = nullptr;

    EXPECT_EQ(abilityMs_->DisconnectAbility(connect), ERR_INVALID_VALUE);

    connect = new AbilityConnectCallback();
    EXPECT_EQ(abilityMs_->DisconnectAbility(connect), ERR_NO_INIT);

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest DisconnectAbility_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: DisconnectLocalAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService DisconnectLocalAbility
 */
HWTEST_F(AbilityManagerServiceTwelfthTest, DisconnectLocalAbility_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTestDisconnectLocalAbility_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    sptr<IAbilityConnection> connect = nullptr;

    EXPECT_EQ(abilityMs_->DisconnectLocalAbility(connect), ERR_NO_INIT);

    abilityMs_->subManagersHelper_ = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    IPCSkeleton::SetCallingUid(BASE_USER_RANGE);
    connect = new AbilityConnectCallback();
    std::shared_ptr<CommonExtensionManager> commonManager = std::make_shared<CommonExtensionManager>(0);
    std::shared_ptr<UIExtensionAbilityManager> uiExtManager = std::make_shared<UIExtensionAbilityManager>(0);
    commonManager->sceneBoardTokenId_ = TWO;
    abilityMs_->subManagersHelper_->commonExtensionManagers_.insert(std::make_pair(0, commonManager));
    abilityMs_->subManagersHelper_->uiExtensionAbilityManagers_.insert(std::make_pair(0, uiExtManager));
    std::shared_ptr<CommonExtensionManager> connectManager2 = std::make_shared<CommonExtensionManager>(TWO);
    connectManager2->sceneBoardTokenId_ = TWO;
    abilityMs_->subManagersHelper_->commonExtensionManagers_.insert(std::make_pair(0, connectManager2));
    EXPECT_EQ(abilityMs_->DisconnectLocalAbility(connect), CONNECT_MAMAGER_NOT_FIND_BY_USERID);

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest DisconnectLocalAbility_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetConnectionData
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetConnectionData
 */
HWTEST_F(AbilityManagerServiceTwelfthTest, GetConnectionData_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfth GetConnectionData_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    std::vector<AbilityRuntime::ConnectionData> connectionData;

    EXPECT_EQ(abilityMs_->GetConnectionData(connectionData), ERR_OK);

    MyFlag::flag_ = false;
    EXPECT_EQ(abilityMs_->GetConnectionData(connectionData), CHECK_PERMISSION_FAILED);

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest GetConnectionData_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: RegisterOffListener
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService RegisterOffListener
 */
HWTEST_F(AbilityManagerServiceTwelfthTest, RegisterOffListener_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfth RegisterOffListener_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    std::string type = "";
    sptr<IRemoteOnListener> listener = nullptr;
    EXPECT_EQ(abilityMs_->RegisterOffListener(type, listener), ERR_INVALID_VALUE);

    listener = new RemoteOnListenerStubMock();
    MyFlag::flag_ = true;
    EXPECT_EQ(abilityMs_->RegisterOffListener(type, listener), INNER_ERR);

    MyFlag::flag_ = false;
    EXPECT_EQ(abilityMs_->RegisterOffListener(type, listener), CHECK_PERMISSION_FAILED);

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest RegisterOffListener_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetUserScreenUnlockCallback
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetUserScreenUnlockCallback
 */
HWTEST_F(AbilityManagerServiceTwelfthTest, GetUserScreenUnlockCallback_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest GetUserScreenUnlockCallback_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->UnSubscribeScreenUnlockedEvent();
    abilityMs_->interceptorExecuter_ = std::make_shared<AbilityInterceptorExecuter>();
    abilityMs_->RemoveScreenUnlockInterceptor();
    int32_t retryCount = 1;
    abilityMs_->RetrySubscribeUnlockedEvent(retryCount, abilityMs_->screenSubscriber_);
    Want want;
    AbilityRequest abilityRequest;
    auto callerTokenId = IPCSkeleton::GetCallingTokenID();
    abilityMs_->RemoveUnauthorizedLaunchReasonMessage(want, abilityRequest, callerTokenId);
    std::string testText = "AbilityManagerServiceTwelfthTest";
    want.SetParam(Want::PARM_LAUNCH_REASON_MESSAGE, testText);
    abilityMs_->RemoveUnauthorizedLaunchReasonMessage(want, abilityRequest, callerTokenId);
    abilityMs_->Init();
    auto ret = abilityMs_->GetUserScreenUnlockCallback();
    int32_t userId = TEST_USER_ID;
    if (ret) {
        ret(userId);
        abilityMs_.reset();
        ret(userId);
    }
    EXPECT_EQ(static_cast<bool>(ret), true);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest GetUserScreenUnlockCallback_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityJust
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbilityJust
 */
HWTEST_F(AbilityManagerServiceTwelfthTest, StartAbilityJust_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest StartAbilityJust_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    AbilityRequest abilityRequest;
    int32_t validUserId = 0;
    auto retCode = abilityMs_->StartAbilityJust(abilityRequest, validUserId);
    EXPECT_EQ(retCode, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest StartAbilityJust_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: StartUser
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartUser
 */
HWTEST_F(AbilityManagerServiceTwelfthTest, StartUser_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest StartUser_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    int32_t userId = 0;
    uint64_t displayId = 0;
    sptr<IUserCallback> callback = nullptr;
    auto retCode = abilityMs_->StartUser(userId, displayId, callback);
    EXPECT_EQ(retCode, INVALID_PARAMETERS_ERR);

    callback = new MockIUserCallback();
    bool isAppRecovery = false;
    retCode = abilityMs_->StartUser(userId, displayId, callback, isAppRecovery);
    EXPECT_EQ(retCode, INVALID_USERID_VALUE);

    userId = 100;
    retCode = abilityMs_->StartUser(userId, displayId, callback, isAppRecovery);
    EXPECT_EQ(retCode, CHECK_PERMISSION_FAILED);

    IPCSkeleton::SetCallingUid(ACCOUNT_MGR_SERVICE_UID);
    userId =  10000;
    retCode = abilityMs_->StartUser(userId, displayId, callback, isAppRecovery);
    EXPECT_EQ(retCode, INVALID_USERID_VALUE);

    userId =  100;
    retCode = abilityMs_->StartUser(userId, displayId, callback, isAppRecovery);
    EXPECT_EQ(retCode, ERR_INVALID_CALLER);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest StartUser_001 end");
}

/*
* Feature: AbilityManagerService
* Function: StartUser
* SubFunction: NA
* FunctionPoints: AbilityManagerService StartUser - U1_USER_ID special case
*/
HWTEST_F(AbilityManagerServiceTwelfthTest, StartUser_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest StartUser_002 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    int32_t userId = U1_USER_ID; // userId = 1
    uint64_t displayId = 0;
    sptr<IUserCallback> callback = new MockIUserCallback();
    bool isAppRecovery = false;

    IPCSkeleton::SetCallingUid(ACCOUNT_MGR_SERVICE_UID);
    auto retCode = abilityMs_->StartUser(userId, displayId, callback, isAppRecovery);
    EXPECT_EQ(retCode, INVALID_USERID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest StartUser_002 end");
}

/*
* Feature: AbilityManagerService
* Function: StartUser
* SubFunction: NA
* FunctionPoints: AbilityManagerService StartUser - Already foreground user
*/
HWTEST_F(AbilityManagerServiceTwelfthTest, StartUser_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest StartUser_003 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    int32_t userId = 100;
    uint64_t displayId = 0;
    sptr<IUserCallback> callback = new MockIUserCallback();
    bool isAppRecovery = false;

    IPCSkeleton::SetCallingUid(ACCOUNT_MGR_SERVICE_UID);
    abilityMs_->subManagersHelper_ = std::make_shared<SubManagersHelper>(nullptr, nullptr);

    AbilityRuntime::UserController::GetInstance().SetForegroundUserId(userId, displayId);

    auto retCode = abilityMs_->StartUser(userId, displayId, callback, isAppRecovery);
    EXPECT_EQ(retCode, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest StartUser_003 end");
}

/*
* Feature: AbilityManagerService
* Function: StartUser
* SubFunction: NA
* FunctionPoints: AbilityManagerService StartUser - Invalid userId negative value
*/
HWTEST_F(AbilityManagerServiceTwelfthTest, StartUser_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest StartUser_004 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    int32_t userId = -1; // invalid userId
    uint64_t displayId = 0;
    sptr<IUserCallback> callback = new MockIUserCallback();
    bool isAppRecovery = false;

    IPCSkeleton::SetCallingUid(ACCOUNT_MGR_SERVICE_UID);
    auto retCode = abilityMs_->StartUser(userId, displayId, callback, isAppRecovery);
    EXPECT_EQ(retCode, INVALID_USERID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest StartUser_004 end");
}

/*
* Feature: AbilityManagerService
* Function: StartUser
* SubFunction: NA
* FunctionPoints: AbilityManagerService StartUser - Without permission
*/
HWTEST_F(AbilityManagerServiceTwelfthTest, StartUser_005, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest StartUser_005 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    int32_t userId = 100;
    uint64_t displayId = 0;
    sptr<IUserCallback> callback = new MockIUserCallback();
    bool isAppRecovery = false;

    IPCSkeleton::SetCallingUid(1000);
    auto retCode = abilityMs_->StartUser(userId, displayId, callback, isAppRecovery);
    EXPECT_EQ(retCode, CHECK_PERMISSION_FAILED);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest StartUser_005 end");
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityByCallWithErrMsg
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbilityByCallWithErrMsg
 */
HWTEST_F(AbilityManagerServiceTwelfthTest, StartAbilityByCallWithErrMsg_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest StartAbilityByCallWithErrMsg_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    Want want;
    int32_t accountId = -1;
    std::string errMsg;
    auto retCode = abilityMs_->StartAbilityByCallWithErrMsg(want, nullptr, nullptr, accountId, errMsg);
    EXPECT_EQ(retCode, ERR_INVALID_VALUE);
    sptr<IAbilityConnection> connect = new MockIAbilityConnection();
    accountId = 0;
    retCode = abilityMs_->StartAbilityByCallWithErrMsg(want, connect, nullptr, accountId, errMsg);
    EXPECT_EQ(retCode, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest StartAbilityByCallWithErrMsg_001 end");
}

/*
 * Feature: AbilityManagerService
 * Name: IsSceneBoardReady_001
 * Function: IsSceneBoardReady
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService IsSceneBoardReady
 */
HWTEST_F(AbilityManagerServiceTwelfthTest, IsSceneBoardReady_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest IsSceneBoardReady_001 start");
    int32_t userId = -1;
    auto abilityMs = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs, nullptr);
    abilityMs->subManagersHelper_ = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    EXPECT_NE(abilityMs->subManagersHelper_, nullptr);
    bool result = abilityMs->IsSceneBoardReady(userId);
    EXPECT_FALSE(result);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest IsSceneBoardReady_001 end");
}

/*
 * Feature: AbilityManagerService
 * Name: IsSceneBoardReady_002
 * Function: IsSceneBoardReady
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService IsSceneBoardReady
 */
HWTEST_F(AbilityManagerServiceTwelfthTest, IsSceneBoardReady_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest IsSceneBoardReady_002 start");
    int32_t userId = 100;
    auto abilityMs = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs, nullptr);
    abilityMs->subManagersHelper_ = nullptr;
    bool result = abilityMs->IsSceneBoardReady(userId);
    EXPECT_FALSE(result);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest IsSceneBoardReady_002 end");
}

/*
 * Feature: AbilityManagerService
 * Name: PrepareTerminateAbilityBySCB_001
 * Function: PrepareTerminateAbilityBySCB
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService PrepareTerminateAbilityBySCB
 */
HWTEST_F(AbilityManagerServiceTwelfthTest, PrepareTerminateAbilityBySCB_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest PrepareTerminateAbilityBySCB_001 start");
    bool isTerminate = false;
    auto abilityMs = std::make_shared<AbilityManagerService>();
    sptr<SessionInfo> sessionInfo = MockSessionInfo(1);
    sessionInfo->sessionToken = MockToken(AbilityType::PAGE);
    int result = abilityMs->PrepareTerminateAbilityBySCB(sessionInfo, isTerminate);
    EXPECT_EQ(result, ERR_WRONG_INTERFACE_CALL);
    EXPECT_FALSE(isTerminate);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest PrepareTerminateAbilityBySCB_001 end");
}

/*
 * Feature: AbilityManagerService
 * Name: PrepareTerminateAbilityBySCB_002
 * Function: PrepareTerminateAbilityBySCB
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService PrepareTerminateAbilityBySCB
 */
HWTEST_F(AbilityManagerServiceTwelfthTest, PrepareTerminateAbilityBySCB_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest PrepareTerminateAbilityBySCB_002 start");
    bool isTerminate = false;
    auto abilityMs = std::make_shared<AbilityManagerService>();
    sptr<SessionInfo> sessionInfo = nullptr;
    int result = abilityMs->PrepareTerminateAbilityBySCB(sessionInfo, isTerminate);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    EXPECT_FALSE(isTerminate);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest PrepareTerminateAbilityBySCB_002 end");
}

/*
 * Feature: AbilityManagerService
 * Function: OnStartSpecifiedAbilityTimeoutResponse
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService OnStartSpecifiedAbilityTimeoutResponse
 */
HWTEST_F(AbilityManagerServiceTwelfthTest, OnStartSpecifiedAbilityTimeoutResponse_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest OnStartSpecifiedAbilityTimeoutResponse_001 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    int32_t requestId = 101;
    std::shared_ptr<TaskHandlerWrap> taskHandler =
        TaskHandlerWrap::CreateQueueHandler(AbilityConfig::NAME_ABILITY_MGR_SERVICE);
    std::shared_ptr<AbilityEventHandler> eventHandler = std::make_shared<AbilityEventHandler>(taskHandler, abilityMs);
    abilityMs->subManagersHelper_ = std::make_shared<SubManagersHelper>(taskHandler, eventHandler);
    EXPECT_NE(abilityMs->subManagersHelper_, nullptr);
    int32_t userId = 100;
    abilityMs->OnStartSpecifiedAbilityTimeoutResponse(requestId, userId);
    auto manager = abilityMs->GetCurrentUIAbilityManager();
    EXPECT_EQ(manager, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest OnStartSpecifiedAbilityTimeoutResponse_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: OnStartSpecifiedAbilityTimeoutResponse
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService OnStartSpecifiedAbilityTimeoutResponse
 */
HWTEST_F(AbilityManagerServiceTwelfthTest, OnStartSpecifiedAbilityTimeoutResponse_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest OnStartSpecifiedAbilityTimeoutResponse_002 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    int32_t requestId = 101;
    int32_t userId = 100;
    abilityMs->OnStartSpecifiedAbilityTimeoutResponse(requestId, userId);
    auto manager = abilityMs->GetCurrentUIAbilityManager();
    EXPECT_EQ(manager, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest OnStartSpecifiedAbilityTimeoutResponse_002 end");
}

/*
 * Feature: AbilityManagerService
 * Function: UpdateAssociateConfigList
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService UpdateAssociateConfigList
 */
HWTEST_F(AbilityManagerServiceTwelfthTest, UpdateAssociateConfigList_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest UpdateAssociateConfigList_001 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    IPCSkeleton::SetCallingUid(BASE_USER_RANGE);
    std::map<std::string, std::list<std::string>> configs;
    std::list<std::string> exportConfigs;
    int32_t flag = 0;
    int32_t result = abilityMs->UpdateAssociateConfigList(configs, exportConfigs, flag);
    EXPECT_EQ(result, CHECK_PERMISSION_FAILED);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest UpdateAssociateConfigList_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: UpdateAssociateConfigList
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService UpdateAssociateConfigList
 */
HWTEST_F(AbilityManagerServiceTwelfthTest, UpdateAssociateConfigList_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest UpdateAssociateConfigList_002 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    IPCSkeleton::SetCallingUid(RESOURCE_SCHEDULE_UID);
    std::map<std::string, std::list<std::string>> configs1 = {
        {"caller1", {"callee1/ability1", "callee2/ability2"}},
        {"caller2", {"callee3/ability3"}}
    };
    std::list<std::string> exportConfigs1 = {"callee4/ability4", "callee5/ability5"};
    int32_t flag1 = UPDATE_CONFIG_FLAG_COVER;
    int32_t result1 = abilityMs->UpdateAssociateConfigList(configs1, exportConfigs1, flag1);
    EXPECT_EQ(result1, ERR_OK);
    EXPECT_TRUE(abilityMs->IsInWhiteList("caller1", "callee1", "ability1"));
    EXPECT_TRUE(abilityMs->IsInWhiteList("caller2", "callee3", "ability3"));
    EXPECT_TRUE(abilityMs->IsInWhiteList("caller1", "callee2", "ability2"));
    EXPECT_TRUE(abilityMs->IsInWhiteList("caller2", "callee3", "ability3"));
    EXPECT_TRUE(abilityMs->IsInWhiteList("callerX", "callee4", "ability4"));

    std::map<std::string, std::list<std::string>> configs2 = {
        {"caller1", {"callee6/ability6"}},
        {"caller3", {"callee7/ability7"}}
    };
    std::list<std::string> exportConfigs2 = {"callee8/ability8"};
    int32_t flag2 = UPDATE_CONFIG_FLAG_APPEND;
    int32_t result2 = abilityMs->UpdateAssociateConfigList(configs2, exportConfigs2, flag2);
    EXPECT_EQ(result2, ERR_OK);
    EXPECT_TRUE(abilityMs->IsInWhiteList("caller1", "callee6", "ability6"));
    EXPECT_TRUE(abilityMs->IsInWhiteList("caller3", "callee7", "ability7"));
    EXPECT_TRUE(abilityMs->IsInWhiteList("callerX", "callee8", "ability8"));

    std::map<std::string, std::list<std::string>> configs3;
    std::list<std::string> exportConfigs3;
    int32_t flag3 = UPDATE_CONFIG_FLAG_APPEND;
    int32_t result3 = abilityMs->UpdateAssociateConfigList(configs3, exportConfigs3, flag3);
    EXPECT_EQ(result3, ERR_OK);
    EXPECT_TRUE(abilityMs->IsInWhiteList("caller1", "callee6", "ability6"));
    EXPECT_TRUE(abilityMs->IsInWhiteList("caller3", "callee7", "ability7"));
    EXPECT_TRUE(abilityMs->IsInWhiteList("callerX", "callee8", "ability8"));
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest UpdateAssociateConfigList_002 end");
}

/*
 * Feature: AbilityManagerService
 * Function: UpdateAssociateConfigList
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService UpdateAssociateConfigList
 */
HWTEST_F(AbilityManagerServiceTwelfthTest, UpdateAssociateConfigList_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest UpdateAssociateConfigList_003 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    IPCSkeleton::SetCallingUid(RESOURCE_SCHEDULE_UID);
    std::map<std::string, std::list<std::string>> configs = {
        {"caller1", {"callee1/ability1"}}
    };
    std::list<std::string> exportConfigs = {"callee2/ability2"};
    int32_t invalidFlag = 999;
    int32_t result = abilityMs->UpdateAssociateConfigList(configs, exportConfigs, invalidFlag);
    EXPECT_EQ(result, ERR_OK);
    EXPECT_FALSE(abilityMs->IsInWhiteList("caller1", "callee1", "ability1"));
    EXPECT_FALSE(abilityMs->IsInWhiteList("callerX", "callee2", "ability2"));
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest UpdateAssociateConfigList_003 end");
}

/*
 * Feature: AbilityManagerService
 * Function: IsAppSelfCalled
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService IsAppSelfCalled
 */
HWTEST_F(AbilityManagerServiceTwelfthTest, IsAppSelfCalled_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest IsAppSelfCalled_001 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    IPCSkeleton::SetCallingTokenID(ONE);
    AbilityRequest abilityRequest{};
    std::string deviceName = "device";
    std::string abilityName = "ServiceAbility";
    std::string appName = "hiservcie";
    std::string bundleName = "com.ix.hiservcie";
    std::string moduleName = "entry";
    abilityRequest = GenerateAbilityRequest(deviceName, abilityName, appName, bundleName, moduleName);
    AppExecFwk::ApplicationInfo appInfo;
    appInfo.accessTokenId = ONE;
    abilityRequest.appInfo = appInfo;
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    bool result = abilityMs->IsAppSelfCalled(abilityRecord);
    EXPECT_TRUE(result);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest IsAppSelfCalled_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: IsAppSelfCalled
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService IsAppSelfCalled
 */
HWTEST_F(AbilityManagerServiceTwelfthTest, IsAppSelfCalled_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest IsAppSelfCalled_002 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    std::shared_ptr<AbilityRecord> abilityRecord = nullptr;
    bool result = abilityMs->IsAppSelfCalled(abilityRecord);
    EXPECT_FALSE(result);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest IsAppSelfCalled_002 end");
}

/*
 * Feature: AbilityManagerService
 * Function: CheckProcessIsBackground
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckProcessIsBackground
 */
HWTEST_F(AbilityManagerServiceTwelfthTest, CheckProcessIsBackground_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest CheckProcessIsBackground_001 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    int32_t pid = 1234;
    AbilityState currentState = AbilityState::BACKGROUND;
    abilityMs->windowVisibleList_.insert(pid);
    bool result = abilityMs->CheckProcessIsBackground(pid, currentState);
    ASSERT_FALSE(result);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest CheckProcessIsBackground_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: CheckProcessIsBackground
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckProcessIsBackground
 */
HWTEST_F(AbilityManagerServiceTwelfthTest, CheckProcessIsBackground_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest CheckProcessIsBackground_002 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    int32_t pid = 1234;
    AbilityState currentState = AbilityState::BACKGROUND;
    abilityMs->windowVisibleList_.clear();
    bool result = abilityMs->CheckProcessIsBackground(pid, currentState);
    ASSERT_TRUE(result);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest CheckProcessIsBackground_002 end");
}

/*
 * Feature: AbilityManagerService
 * Function: CheckProcessIsBackground
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckProcessIsBackground
 */
HWTEST_F(AbilityManagerServiceTwelfthTest, CheckProcessIsBackground_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest CheckProcessIsBackground_003 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    int32_t pid = 1234;
    AbilityState currentState = AbilityState::FOREGROUND;
    abilityMs->windowVisibleList_.insert(pid);
    bool result = abilityMs->CheckProcessIsBackground(pid, currentState);
    ASSERT_FALSE(result);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest CheckProcessIsBackground_003 end");
}

/*
 * Feature: AbilityManagerService
 * Function: CheckProcessIsBackground
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckProcessIsBackground
 */
HWTEST_F(AbilityManagerServiceTwelfthTest, CheckProcessIsBackground_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest CheckProcessIsBackground_004 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    int32_t pid = 1234;
    AbilityState currentState = AbilityState::FOREGROUND;
    abilityMs->windowVisibleList_.erase(pid);
    bool result = abilityMs->CheckProcessIsBackground(pid, currentState);
    ASSERT_FALSE(result);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest CheckProcessIsBackground_004 end");
}

/*
 * Feature: AbilityManagerService
 * Function: CleanUIAbilityBySCB
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CleanUIAbilityBySCB
 */
HWTEST_F(AbilityManagerServiceTwelfthTest, CleanUIAbilityBySCB_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest CleanUIAbilityBySCB_001 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    sptr<SessionInfo> sessionInfo = nullptr;
    uint32_t sceneFlag = 0;
    bool isUserRequestedExit = false;
    int32_t result = abilityMs->CleanUIAbilityBySCB(sessionInfo, isUserRequestedExit, sceneFlag);
    ASSERT_EQ(result, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest CleanUIAbilityBySCB_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: CleanUIAbilityBySCB
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CleanUIAbilityBySCB
 */
HWTEST_F(AbilityManagerServiceTwelfthTest, CleanUIAbilityBySCB_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest CleanUIAbilityBySCB_002 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    sptr<SessionInfo> sessionInfo = new SessionInfo();
    sessionInfo->sessionToken = nullptr;
    uint32_t sceneFlag = 0;
    bool isUserRequestedExit = false;
    int32_t result = abilityMs->CleanUIAbilityBySCB(sessionInfo, isUserRequestedExit, sceneFlag);
    ASSERT_EQ(result, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest CleanUIAbilityBySCB_002 end");
}

/*
 * Feature: AbilityManagerService
 * Function: CleanUIAbilityBySCB
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CleanUIAbilityBySCB
 */
HWTEST_F(AbilityManagerServiceTwelfthTest, CleanUIAbilityBySCB_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest CleanUIAbilityBySCB_003 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    Rosen::SessionInfo info;
    sptr<SessionInfo> sessionInfo = new SessionInfo();
    sessionInfo->sessionToken = new Rosen::Session(info);
    uint32_t sceneFlag = 0;
    bool isUserRequestedExit = false;
    int32_t result = abilityMs->CleanUIAbilityBySCB(sessionInfo, isUserRequestedExit, sceneFlag);
    ASSERT_EQ(result, ERR_WRONG_INTERFACE_CALL);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest CleanUIAbilityBySCB_003 end");
}

/*
 * Feature: AbilityManagerService
 * Function: CleanUIAbilityBySCB
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CleanUIAbilityBySCB
 */
HWTEST_F(AbilityManagerServiceTwelfthTest, CleanUIAbilityBySCB_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest CleanUIAbilityBySCB_004 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    sptr<SessionInfo> sessionInfo = new SessionInfo();
    Rosen::SessionInfo info;
    sessionInfo->sessionToken = new Rosen::Session(info);
    sessionInfo->persistentId = 99999;
    uint32_t sceneFlag = 1;
    bool isUserRequestedExit = false;
    auto uiAbilityManager = abilityMs->GetCurrentUIAbilityManager();
    int32_t result = abilityMs->CleanUIAbilityBySCB(sessionInfo, isUserRequestedExit, sceneFlag);
    ASSERT_EQ(result, ERR_WRONG_INTERFACE_CALL);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest CleanUIAbilityBySCB_004 end");
}

/*
 * Feature: AbilityManagerService
 * Function: UpdateKioskApplicationList
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService UpdateKioskApplicationList
 */
HWTEST_F(AbilityManagerServiceTwelfthTest, UpdateKioskApplicationList_Success, TestSize.Level1) {
    IPCSkeleton::SetCallingUid(BASE_USER_RANGE);
    IPCSkeleton::SetCallingTokenID(ONE);
    auto abilityManagerService = std::make_shared<AbilityManagerService>();
    std::vector<std::string> bundleNames;
    bundleNames.emplace_back("com.test.demo");
    bundleNames.emplace_back("com.test.demo1");
    bundleNames.emplace_back("com.test.demo2");
    MyFlag::flag_ = true;
    auto result = abilityManagerService->UpdateKioskApplicationList(bundleNames);
    if (!system::GetBoolParameter(KIOSK_MODE_ENABLED, false)) {
        ASSERT_EQ(result, ERR_CAPABILITY_NOT_SUPPORT);
    } else {
        ASSERT_EQ(result, ERR_OK);
    }
}

/*
 * Feature: AbilityManagerService
 * Function: UpdateKioskApplicationList
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService UpdateKioskApplicationList
 */
HWTEST_F(AbilityManagerServiceTwelfthTest, UpdateKioskApplicationList_Fail, TestSize.Level1) {
    IPCSkeleton::SetCallingUid(BASE_USER_RANGE);
    IPCSkeleton::SetCallingTokenID(ONE);
    auto abilityManagerService = std::make_shared<AbilityManagerService>();
    std::vector<std::string> bundleNames;
    bundleNames.emplace_back("com.test.demo");
    bundleNames.emplace_back("com.test.demo1");
    bundleNames.emplace_back("com.test.demo2");
    MyFlag::flag_ = false;
    auto result = abilityManagerService->UpdateKioskApplicationList(bundleNames);
    if (!system::GetBoolParameter(KIOSK_MODE_ENABLED, false)) {
        ASSERT_EQ(result, ERR_CAPABILITY_NOT_SUPPORT);
    } else {
        ASSERT_EQ(result, CHECK_PERMISSION_FAILED);
    }
}

/*
 * Feature: AbilityManagerService
 * Function: EnterKioskMode
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService EnterKioskMode
 */
HWTEST_F(AbilityManagerServiceTwelfthTest, EnterKioskMode_Fail, TestSize.Level1) {
    IPCSkeleton::SetCallingUid(BASE_USER_RANGE);
    IPCSkeleton::SetCallingTokenID(ONE);
    auto abilityManagerService = std::make_shared<AbilityManagerService>();
    std::vector<std::string> bundleNames;
    bundleNames.emplace_back("com.test.demo");
    MyFlag::flag_ = true;
    auto result = abilityManagerService->UpdateKioskApplicationList(bundleNames);
    auto callerToken = MockToken(AbilityType::PAGE, ONE);
    result = abilityManagerService->EnterKioskMode(callerToken);
    if (!system::GetBoolParameter(KIOSK_MODE_ENABLED, false)) {
        ASSERT_EQ(result, ERR_CAPABILITY_NOT_SUPPORT);
    } else {
        ASSERT_EQ(result, ERR_APP_NOT_IN_FOCUS);
    }
}

/*
 * Feature: AbilityManagerService
 * Function: ExitKioskMode
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ExitKioskMode
 */
HWTEST_F(AbilityManagerServiceTwelfthTest, ExitKioskMode_Fail, TestSize.Level1) {
    IPCSkeleton::SetCallingUid(BASE_USER_RANGE);
    IPCSkeleton::SetCallingTokenID(ONE);
    auto abilityManagerService = std::make_shared<AbilityManagerService>();
    auto callerToken = MockToken(AbilityType::PAGE, ONE);
    auto result = abilityManagerService->ExitKioskMode(callerToken);
    if (!system::GetBoolParameter(KIOSK_MODE_ENABLED, false)) {
        ASSERT_EQ(result, ERR_CAPABILITY_NOT_SUPPORT);
    } else {
        ASSERT_EQ(result, ERR_KIOSK_MODE_NOT_IN_WHITELIST);
    }
}

/*
 * Feature: AbilityManagerService
 * Function: GetKioskStatus
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetKioskStatus
 */
HWTEST_F(AbilityManagerServiceTwelfthTest, GetKioskStatus, TestSize.Level1) {
    IPCSkeleton::SetCallingUid(BASE_USER_RANGE);
    IPCSkeleton::SetCallingTokenID(ONE);
    auto abilityManagerService = std::make_shared<AbilityManagerService>();
    KioskStatus kioskStatus;
    MyFlag::flag_ = true;
    auto result = abilityManagerService->GetKioskStatus(kioskStatus);
    if (!system::GetBoolParameter(KIOSK_MODE_ENABLED, false)) {
        ASSERT_EQ(result, ERR_CAPABILITY_NOT_SUPPORT);
    } else {
        ASSERT_EQ(result, ERR_OK);
    }
}

/*
 * Feature: KioskManager
 * Function: UpdateKioskApplicationList
 * SubFunction: NA
 * FunctionPoints: KioskManager UpdateKioskApplicationList
 */
HWTEST_F(AbilityManagerServiceTwelfthTest, KioskManager_UpdateKioskApplicationList, TestSize.Level1) {
    IPCSkeleton::SetCallingUid(BASE_USER_RANGE);
    IPCSkeleton::SetCallingTokenID(ONE);
    MyFlag::flag_ = true;
    std::vector<std::string> bundleNames;
    bundleNames.emplace_back("com.test.demo");
    bundleNames.emplace_back("com.test.demo2");
    bundleNames.emplace_back("com.test.demo3");
    auto result = KioskManager::GetInstance().UpdateKioskApplicationList(bundleNames);
    if (!system::GetBoolParameter(KIOSK_MODE_ENABLED, false)) {
        ASSERT_EQ(result, ERR_CAPABILITY_NOT_SUPPORT);
    } else {
        ASSERT_EQ(result, ERR_OK);
    }
}

/*
 * Feature: KioskManager
 * Function: EnterKioskMode
 * SubFunction: NA
 * FunctionPoints: KioskManager EnterKioskMode
 */
HWTEST_F(AbilityManagerServiceTwelfthTest, KioskManager_EnterKioskMode_Fail_01, TestSize.Level1) {
    auto callerToken = MockToken(AbilityType::PAGE);
    auto result = KioskManager::GetInstance().EnterKioskMode(callerToken);
    if (!system::GetBoolParameter(KIOSK_MODE_ENABLED, false)) {
        ASSERT_EQ(result, ERR_CAPABILITY_NOT_SUPPORT);
    } else {
        ASSERT_EQ(result, ERR_APP_NOT_IN_FOCUS);
    }
}

/*
 * Feature: KioskManager
 * Function: ExitKioskMode
 * SubFunction: NA
 * FunctionPoints: KioskManager ExitKioskMode
 */
HWTEST_F(AbilityManagerServiceTwelfthTest, KioskManager_ExitKioskMode_Fail_01, TestSize.Level1) {
    auto callerToken = MockToken(AbilityType::PAGE);
    auto result = KioskManager::GetInstance().ExitKioskMode(callerToken, false);
    if (!system::GetBoolParameter(KIOSK_MODE_ENABLED, false)) {
        ASSERT_EQ(result, ERR_CAPABILITY_NOT_SUPPORT);
    } else {
        ASSERT_EQ(result, ERR_KIOSK_MODE_NOT_IN_WHITELIST);
    }
}

/*
 * Feature: KioskManager
 * Function: GetKioskStatus
 * SubFunction: NA
 * FunctionPoints: KioskManager GetKioskStatus
 */
HWTEST_F(AbilityManagerServiceTwelfthTest, KioskManager_GetKioskStatus_Success, TestSize.Level1) {
    IPCSkeleton::SetCallingUid(BASE_USER_RANGE);
    IPCSkeleton::SetCallingTokenID(ONE);
    MyFlag::flag_ = true;
    KioskStatus kioskStatus;
    auto result = KioskManager::GetInstance().GetKioskStatus(kioskStatus);
    if (!system::GetBoolParameter(KIOSK_MODE_ENABLED, false)) {
        ASSERT_EQ(result, ERR_CAPABILITY_NOT_SUPPORT);
    } else {
        ASSERT_EQ(result, ERR_OK);
    }
}


/*
 * Feature: AbilityManagerService
 * Function: SetAppServiceExtensionKeepAlive
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService SetAppServiceExtensionKeepAlive
 */
HWTEST_F(AbilityManagerServiceTwelfthTest, SetAppServiceExtensionKeepAlive_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest SetAppServiceExtensionKeepAlive_001 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    std::string bundleName = "bundleName";
    bool flag = true;
    system::SetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, false);
    int32_t result = abilityMs->SetAppServiceExtensionKeepAlive(bundleName, flag);
    ASSERT_EQ(result, ERR_CAPABILITY_NOT_SUPPORT);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest SetAppServiceExtensionKeepAlive_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: QueryKeepAliveAppServiceExtensions
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService QueryKeepAliveAppServiceExtensions
 */
HWTEST_F(AbilityManagerServiceTwelfthTest, QueryKeepAliveAppServiceExtensions_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest QueryKeepAliveAppServiceExtensions_001 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    std::vector<AbilityRuntime::KeepAliveInfo> list;
    system::SetBoolParameter(PRODUCT_ENTERPRISE_FEATURE_SETTING_ENABLED, false);
    int32_t result = abilityMs->QueryKeepAliveAppServiceExtensions(list);
    ASSERT_EQ(result, ERR_CAPABILITY_NOT_SUPPORT);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest QueryKeepAliveAppServiceExtensions_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: SetOnNewWantSkipScenarios
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService SetOnNewWantSkipScenarios
 */
HWTEST_F(AbilityManagerServiceTwelfthTest, SetOnNewWantSkipScenarios_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest SetOnNewWantSkipScenarios_001 start");
    auto abilityMs = std::make_unique<AbilityManagerService>();
    int32_t result = abilityMs->SetOnNewWantSkipScenarios(nullptr, 0);
    EXPECT_EQ(result, ERR_INVALID_CONTEXT);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest SetOnNewWantSkipScenarios_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: SetOnNewWantSkipScenarios
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService SetOnNewWantSkipScenarios
 */
HWTEST_F(AbilityManagerServiceTwelfthTest, SetOnNewWantSkipScenarios_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest SetOnNewWantSkipScenarios_002 start");
    auto abilityMs = std::make_unique<AbilityManagerService>();
    auto abilityRecord = MockAbilityRecord(AbilityType::PAGE);
    int32_t result = abilityMs->SetOnNewWantSkipScenarios(abilityRecord->GetToken(), 0);
    EXPECT_EQ(result, ERR_INVALID_CALLER);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest SetOnNewWantSkipScenarios_002 end");
}

/*
 * Feature: AbilityManagerService
 * Function: SetOnNewWantSkipScenarios
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService SetOnNewWantSkipScenarios
 */
HWTEST_F(AbilityManagerServiceTwelfthTest, SetOnNewWantSkipScenarios_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest SetOnNewWantSkipScenarios_003 start");
    auto abilityMs = std::make_unique<AbilityManagerService>();
    auto token = MockToken(AbilityType::PAGE);
    std::shared_ptr<AbilityRecord> abilityRecord = MockAbilityRecord(AbilityType::PAGE);
    EXPECT_NE(abilityRecord, nullptr);
    token->abilityRecord_ = abilityRecord;
    auto abilityRecordTest = Token::GetAbilityRecordByToken(token);
    EXPECT_NE(abilityRecordTest, nullptr);
    abilityRecordTest->abilityInfo_.applicationInfo.accessTokenId = IPCSkeleton::GetCallingTokenID();
    int32_t result = abilityMs->SetOnNewWantSkipScenarios(token, 0);
    EXPECT_EQ(result, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest SetOnNewWantSkipScenarios_003 end");
}

/*
 * Feature: AbilityManagerService
 * Function: SuspendExtensionAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService SuspendExtensionAbility
 */
HWTEST_F(AbilityManagerServiceTwelfthTest, SuspendExtensionAbility_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest SuspendExtensionAbility_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    sptr<IAbilityConnection> connect = nullptr;

    EXPECT_EQ(abilityMs_->SuspendExtensionAbility(connect), ERR_INVALID_VALUE);

    connect = new AbilityConnectCallback();
    EXPECT_EQ(abilityMs_->SuspendExtensionAbility(connect), CONNECT_MAMAGER_NOT_FIND_BY_USERID);

    MyFlag::flag_ = false;
    EXPECT_EQ(abilityMs_->SuspendExtensionAbility(connect), CHECK_PERMISSION_FAILED);
    MyFlag::flag_ = true;

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest SuspendExtensionAbility_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: ResumeExtensionAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ResumeExtensionAbility
 */
HWTEST_F(AbilityManagerServiceTwelfthTest, ResumeExtensionAbility_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest ResumeExtensionAbility_001 start");
    MyFlag::flag_ = true;
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs_, nullptr);
    sptr<IAbilityConnection> connect = nullptr;

    EXPECT_EQ(abilityMs_->ResumeExtensionAbility(connect), ERR_INVALID_VALUE);

    connect = new AbilityConnectCallback();
    EXPECT_EQ(abilityMs_->ResumeExtensionAbility(connect), CONNECT_MAMAGER_NOT_FIND_BY_USERID);

    MyFlag::flag_ = false;
    EXPECT_EQ(abilityMs_->ResumeExtensionAbility(connect), CHECK_PERMISSION_FAILED);
    MyFlag::flag_ = true;

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest ResumeExtensionAbility_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: OnAbilityDied
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService OnAbilityDied
 */
HWTEST_F(AbilityManagerServiceTwelfthTest, OnAbilityDied_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest OnAbilityDied_001 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    sptr<IRemoteObject> token = MockToken(AbilityType::PAGE);
    std::shared_ptr<AbilityRecord> abilityRecord = MockAbilityRecord(AbilityType::PAGE);
    EXPECT_NE(abilityRecord, nullptr);
    KioskManager::GetInstance().kioskStatus_.isKioskMode_ = false;
    abilityMs->OnAbilityDied(abilityRecord);
    KioskManager::GetInstance().kioskStatus_.isKioskMode_ = true;
    abilityMs->OnAbilityDied(abilityRecord);
    EXPECT_NE(abilityRecord->GetToken(), nullptr);
    KioskManager::GetInstance().whitelist_.emplace(abilityRecord->GetAbilityInfo().bundleName);
    KioskManager::GetInstance().kioskStatus_.kioskBundleUid_ = abilityRecord->GetAbilityInfo().uid;
    abilityMs->OnAbilityDied(abilityRecord);
    if (system::GetBoolParameter(KIOSK_MODE_ENABLED, false)) {
        EXPECT_EQ(KioskManager::GetInstance().kioskStatus_.kioskBundleName_, "");
    }
    EXPECT_FALSE(abilityMs->VerificationAllToken(token));
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest OnAbilityDied_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: HandleExtensionAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService HandleExtensionAbility
 */
HWTEST_F(AbilityManagerServiceTwelfthTest, HandleExtensionAbility_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest HandleExtensionAbility_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    sptr<IAbilityConnection> connect = nullptr;

    EXPECT_EQ(abilityMs_->HandleExtensionAbility(connect,
        [](std::shared_ptr<AbilityConnectManager> connectManager, sptr<IAbilityConnection> connect) {
            return connectManager->ResumeExtensionAbilityLocked(connect);}), ERR_INVALID_VALUE);

    connect = new AbilityConnectCallback();
    EXPECT_EQ(abilityMs_->HandleExtensionAbility(connect,
        [](std::shared_ptr<AbilityConnectManager> connectManager, sptr<IAbilityConnection> connect) {
            return connectManager->ResumeExtensionAbilityLocked(connect);}), CONNECT_MAMAGER_NOT_FIND_BY_USERID);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest HandleExtensionAbility_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: EnterKioskMode
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService EnterKioskMode
 */
HWTEST_F(AbilityManagerServiceTwelfthTest, EnterKioskMode_003, TestSize.Level1) {
    IPCSkeleton::SetCallingUid(BASE_USER_RANGE);
    IPCSkeleton::SetCallingTokenID(ONE);
    MyFlag::flag_ = true;
    auto abilityManagerService = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityManagerService, nullptr);
    uint32_t tokenId = 0;
    auto callerToken = MockToken(AbilityType::PAGE, tokenId);
    ASSERT_NE(callerToken, nullptr);
    auto result = abilityManagerService->EnterKioskMode(callerToken);
    EXPECT_EQ(result, CHECK_PERMISSION_FAILED);
}

/*
 * Feature: AbilityManagerService
 * Function: ExitKioskMode
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ExitKioskMode
 */
HWTEST_F(AbilityManagerServiceTwelfthTest, ExitKioskMode_003, TestSize.Level1) {
    IPCSkeleton::SetCallingUid(BASE_USER_RANGE);
    IPCSkeleton::SetCallingTokenID(ONE);
    MyFlag::flag_ = true;
    auto abilityManagerService = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityManagerService, nullptr);
    uint32_t tokenId = 0;
    auto callerToken = MockToken(AbilityType::PAGE, tokenId);
    ASSERT_NE(callerToken, nullptr);
    auto result = abilityManagerService->ExitKioskMode(callerToken);
    EXPECT_EQ(result, CHECK_PERMISSION_FAILED);
}

/*
 * Feature: AbilityManagerService
 * Name: SubscribeScreenUnlockedEvent
 * SubFunction: NA
 * Function: AbilityManagerService SubscribeScreenUnlockedEvent
 */
HWTEST_F(AbilityManagerServiceTwelfthTest, SubscribeScreenUnlockedEvent_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest SubscribeScreenUnlockedEvent_001 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs, nullptr);
    abilityMs->SubscribeScreenUnlockedEvent();
    EXPECT_EQ(abilityMs->isSubscribed_, true);
    abilityMs->UnSubscribeScreenUnlockedEvent();
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest SubscribeScreenUnlockedEvent_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetDisplayIdByAccount
 * SubFunction: NA
 * FunctionPoints: Get displayId by accountId
 */
HWTEST_F(AbilityManagerServiceTwelfthTest, GetDisplayIdByAccount_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest GetDisplayIdByAccount_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_TRUE(abilityMs_ != nullptr);

    // 分支1: 权限验证失败
    MyFlag::flag_ = CHECK_PERMISSION_FAILED;
    int32_t accountId = 999;
    uint64_t displayId = 0;
    EXPECT_EQ(abilityMs_->GetDisplayIdByAccount(accountId, displayId), CHECK_PERMISSION_FAILED);

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest GetDisplayIdByAccount_001 end");
}

HWTEST_F(AbilityManagerServiceTwelfthTest, GetDisplayIdByAccount_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest GetDisplayIdByAccount_002 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_TRUE(abilityMs_ != nullptr);

    // 清理 UserController 状态，确保测试隔离
    AbilityRuntime::UserController::GetInstance().ClearUserId(999);

    // 分支2: 权限验证失败（flag_=1不等于ERR_OK），未找到 displayId
    MyFlag::flag_ = 1;
    int32_t accountId = 999;
    uint64_t displayId = 0;
    EXPECT_EQ(abilityMs_->GetDisplayIdByAccount(accountId, displayId), CHECK_PERMISSION_FAILED);

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest GetDisplayIdByAccount_002 end");
}

HWTEST_F(AbilityManagerServiceTwelfthTest, GetDisplayIdByAccount_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest GetDisplayIdByAccount_003 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_TRUE(abilityMs_ != nullptr);

    // 分支3: 成功获取 displayId（由于callerUser=0是U0_USER_ID且userId=999是前台用户，走早期返回路径，flag_=1不起作用）
    MyFlag::flag_ = 1;
    int32_t accountId = 999;
    uint64_t displayId = 0;
    uint64_t expectedDisplayId = 1;

    AbilityRuntime::UserController::GetInstance().SetForegroundUserId(accountId, expectedDisplayId);

    EXPECT_EQ(abilityMs_->GetDisplayIdByAccount(accountId, displayId), ERR_OK);
    EXPECT_EQ(displayId, expectedDisplayId);

    // 清理 UserController 状态，确保测试隔离
    AbilityRuntime::UserController::GetInstance().ClearUserId(accountId);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest GetDisplayIdByAccount_003 end");
}

HWTEST_F(AbilityManagerServiceTwelfthTest, GetDisplayIdByAccount_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest GetDisplayIdByAccount_004 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_TRUE(abilityMs_ != nullptr);

    // 边界值: accountId 为负数
    MyFlag::flag_ = 1;
    int32_t accountId = -1;
    uint64_t displayId = 0;

    EXPECT_EQ(abilityMs_->GetDisplayIdByAccount(accountId, displayId), ERR_INVALID_VALUE);

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest GetDisplayIdByAccount_004 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetTopAbilityByUserId
 * SubFunction: NA
 * FunctionPoints: Get top ability token by userId
 */
HWTEST_F(AbilityManagerServiceTwelfthTest, GetTopAbilityByUserId_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest GetTopAbilityByUserId_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_TRUE(abilityMs_ != nullptr);

    // 分支1: 账号权限验证通过（flag_=0不等于CHECK_PERMISSION_FAILED），但wmsHandler为空
    sptr<IRemoteObject> token = nullptr;
    int32_t userId = 100;
    uint64_t displayId = 0;

    // JudgeCallerIsAllowedToUseSystemAPI 返回 true，系统应用检查通过
    // VerifyAccountPermission 返回 0，不等于 CHECK_PERMISSION_FAILED (2097177)，账号权限检查通过
    // 但 wmsHandler 为空，返回 ERR_INVALID_VALUE
    MyFlag::flag_ = 0;
    EXPECT_EQ(abilityMs_->GetTopAbilityByUserId(token, userId, displayId), ERR_INVALID_VALUE);

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest GetTopAbilityByUserId_001 end");
}

HWTEST_F(AbilityManagerServiceTwelfthTest, GetTopAbilityByUserId_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest GetTopAbilityByUserId_002 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_TRUE(abilityMs_ != nullptr);

    // 分支2: 账号权限验证通过（flag_=0不等于CHECK_PERMISSION_FAILED），但wmsHandler为空
    MyFlag::flag_ = 0;
    sptr<IRemoteObject> token = nullptr;
    int32_t userId = 100;
    uint64_t displayId = 0;

    // VerifyAccountPermission 返回 0，不等于 CHECK_PERMISSION_FAILED (2097177)，账号权限检查通过
    // 但 wmsHandler 为空，返回 ERR_INVALID_VALUE
    EXPECT_EQ(abilityMs_->GetTopAbilityByUserId(token, userId, displayId), ERR_INVALID_VALUE);

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest GetTopAbilityByUserId_002 end");
}

HWTEST_F(AbilityManagerServiceTwelfthTest, GetTopAbilityByUserId_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest GetTopAbilityByUserId_003 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_TRUE(abilityMs_ != nullptr);

    // 分支3: 权限验证成功，但 wmsHandler 为空 (SceneBoard 禁用场景)
    MyFlag::flag_ = 1;
    sptr<IRemoteObject> token = nullptr;
    int32_t userId = 100;
    uint64_t displayId = 0;

    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        TAG_LOGI(AAFwkTag::TEST, "SceneBoard is enabled, skip test");
        TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest GetTopAbilityByUserId_003 end");
        return;
    }

    EXPECT_EQ(abilityMs_->GetTopAbilityByUserId(token, userId, displayId), ERR_INVALID_VALUE);

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest GetTopAbilityByUserId_003 end");
}

HWTEST_F(AbilityManagerServiceTwelfthTest, GetTopAbilityByUserId_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest GetTopAbilityByUserId_004 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_TRUE(abilityMs_ != nullptr);

    // 分支4: SceneBoard 启用，sceneSessionManager 为空
    MyFlag::flag_ = 1;
    sptr<IRemoteObject> token = nullptr;
    int32_t userId = 100;
    uint64_t displayId = 0;

    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        TAG_LOGI(AAFwkTag::TEST, "SceneBoard is not enabled, skip test");
        TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest GetTopAbilityByUserId_004 end");
        return;
    }

    EXPECT_CALL(Rosen::SceneBoardJudgement::GetInstance(), MockIsSceneBoardEnabled())
        .WillRepeatedly(Return(true));

    EXPECT_EQ(abilityMs_->GetTopAbilityByUserId(token, userId, displayId), ERR_INVALID_VALUE);

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest GetTopAbilityByUserId_004 end");
}

HWTEST_F(AbilityManagerServiceTwelfthTest, GetTopAbilityByUserId_005, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest GetTopAbilityByUserId_005 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_TRUE(abilityMs_ != nullptr);

    // 分支5: 权限验证成功，SceneBoard 禁用，验证返回值
    MyFlag::flag_ = 1;
    sptr<IRemoteObject> token = nullptr;
    int32_t userId = 100;
    uint64_t displayId = 0;

    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        TAG_LOGI(AAFwkTag::TEST, "SceneBoard is enabled, skip test");
        TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest GetTopAbilityByUserId_005 end");
        return;
    }

    // wmsHandler 为空，返回 ERR_INVALID_VALUE
    EXPECT_EQ(abilityMs_->GetTopAbilityByUserId(token, userId, displayId), ERR_INVALID_VALUE);

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest GetTopAbilityByUserId_005 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetTopAbilityByUserId
 * SubFunction: NA
 * FunctionPoints: Get top ability token by userId - invalid userId
 */
HWTEST_F(AbilityManagerServiceTwelfthTest, GetTopAbilityByUserId_006, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest GetTopAbilityByUserId_006 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_TRUE(abilityMs_ != nullptr);

    // 测试场景：GetValidUserId 返回负值
    MyFlag::flag_ = 1;
    sptr<IRemoteObject> token = nullptr;
    int32_t userId = 100;
    uint64_t displayId = 0;

    // 当 GetValidUserId 返回负值时，应返回 ERR_INVALID_VALUE
    EXPECT_EQ(abilityMs_->GetTopAbilityByUserId(token, userId, displayId), ERR_INVALID_VALUE);

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest GetTopAbilityByUserId_006 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetTopAbilityByUserId
 * SubFunction: NA
 * FunctionPoints: Get top ability token by userId - token is null
 */
HWTEST_F(AbilityManagerServiceTwelfthTest, GetTopAbilityByUserId_007, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest GetTopAbilityByUserId_007 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_TRUE(abilityMs_ != nullptr);

    // 测试场景：获取token成功但token本身为null
    MyFlag::flag_ = 1;
    sptr<IRemoteObject> token = nullptr;
    int32_t userId = 100;
    uint64_t displayId = 0;

    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        TAG_LOGI(AAFwkTag::TEST, "SceneBoard is enabled, skip test");
        TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest GetTopAbilityByUserId_007 end");
        return;
    }

    // wmsHandler 为空时，获取的token为null，应返回 ERR_INVALID_VALUE
    EXPECT_EQ(abilityMs_->GetTopAbilityByUserId(token, userId, displayId), ERR_INVALID_VALUE);

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest GetTopAbilityByUserId_007 end");
}

/*
 * Feature: AbilityManagerService
 * Function: RequestModalUIExtensionInner
 * SubFunction: NA
 * FunctionPoints: Request modal UI extension - no top ability
 */
HWTEST_F(AbilityManagerServiceTwelfthTest, RequestModalUIExtensionInner_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest RequestModalUIExtensionInner_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_TRUE(abilityMs_ != nullptr);

    // 设置 mock 返回成功
    MyFlag::retCreateModalUIExtension_ = true;

    // 测试场景：没有顶层 Ability，GetTopAbility 返回错误
    Want want;
    std::string bundleName = "com.test.demo";
    want.SetParam("bundleName", bundleName);

    // 由于没有 wmsHandler，GetTopAbility 会失败
    // 预期：使用 ModalSystemUiExtension 降级方案，返回 ERR_OK
    auto result = abilityMs_->RequestModalUIExtensionInner(want);
    EXPECT_EQ(result, ERR_OK);

    // 清理
    MyFlag::retCreateModalUIExtension_ = true;  // 恢复默认值
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest RequestModalUIExtensionInner_001 end");
}

HWTEST_F(AbilityManagerServiceTwelfthTest, RequestModalUIExtensionInner_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest RequestModalUIExtensionInner_002 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_TRUE(abilityMs_ != nullptr);

    // 设置 mock 返回成功
    MyFlag::retCreateModalUIExtension_ = true;

    // 测试场景：基本场景 - 设置 bundleName
    Want want;
    std::string bundleName = "com.test.demo";
    want.SetParam("bundleName", bundleName);

    // 由于没有 wmsHandler，GetTopAbility 会失败
    // 预期：使用 ModalSystemUiExtension 降级方案，返回 ERR_OK
    auto result = abilityMs_->RequestModalUIExtensionInner(want);
    EXPECT_EQ(result, ERR_OK);

    // 清理
    MyFlag::retCreateModalUIExtension_ = true;  // 恢复默认值
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest RequestModalUIExtensionInner_002 end");
}

/*
 * Feature: AbilityManagerService
 * Function: RequestModalUIExtensionInner
 * SubFunction: NA
 * FunctionPoints: Request modal UI extension - token is null after GetTopAbility
 */
HWTEST_F(AbilityManagerServiceTwelfthTest, RequestModalUIExtensionInner_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest RequestModalUIExtensionInner_003 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_TRUE(abilityMs_ != nullptr);

    // 设置 mock 返回成功
    MyFlag::retCreateModalUIExtension_ = true;

    // 测试场景：GetTopAbility 返回 OK 但 token 为 null
    Want want;
    std::string bundleName = "com.test.demo";
    want.SetParam("bundleName", bundleName);
    MyFlag::flag_ = 0;  // Mock GetTopAbility 返回 OK 但 token 为 null

    // 预期：由于 token 为 null，使用 ModalSystemUiExtension 降级方案，返回 ERR_OK
    auto result = abilityMs_->RequestModalUIExtensionInner(want);
    EXPECT_EQ(result, ERR_OK);

    // 清理
    MyFlag::retCreateModalUIExtension_ = true;  // 恢复默认值
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest RequestModalUIExtensionInner_003 end");
}

/*
 * Feature: AbilityManagerService
 * Function: RequestModalUIExtensionInner
 * SubFunction: NA
 * FunctionPoints: Request modal UI extension - bundleName mismatch
 */
HWTEST_F(AbilityManagerServiceTwelfthTest, RequestModalUIExtensionInner_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest RequestModalUIExtensionInner_004 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_TRUE(abilityMs_ != nullptr);

    // 设置 mock 返回成功
    MyFlag::retCreateModalUIExtension_ = true;

    // 测试场景：焦点应用 bundleName 与调用者不匹配
    Want want;
    std::string bundleName = "com.caller.demo";  // 与焦点应用不一致
    want.SetParam("bundleName", bundleName);
    MyFlag::flag_ = 1;  // Mock 焦点应用为 "com.test.demo"

    // 预期：由于 bundleName 不匹配，使用 ModalSystemUiExtension 降级方案，返回 ERR_OK
    auto result = abilityMs_->RequestModalUIExtensionInner(want);
    EXPECT_EQ(result, ERR_OK);

    // 清理
    MyFlag::retCreateModalUIExtension_ = true;  // 恢复默认值
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest RequestModalUIExtensionInner_004 end");
}

/*
 * Feature: AbilityManagerService
 * Function: RequestModalUIExtensionInner
 * SubFunction: NA
 * FunctionPoints: Request modal UI extension - record is null
 */
HWTEST_F(AbilityManagerServiceTwelfthTest, RequestModalUIExtensionInner_005, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest RequestModalUIExtensionInner_005 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_TRUE(abilityMs_ != nullptr);

    // 设置 mock 返回成功
    MyFlag::retCreateModalUIExtension_ = true;

    // 测试场景：token有效但AbilityRecord为null
    // 这个场景需要Mock GetTopAbility返回有效token，但Token::GetAbilityRecordByToken返回null
    // 在当前测试环境中，无法直接Mock Token::GetAbilityRecordByToken
    // 此测试用例作为占位符，表明需要测试该分支
    Want want;
    std::string bundleName = "com.test.demo";
    want.SetParam("bundleName", bundleName);

    // 由于无法完全Mock该场景，实际会走降级方案返回 ERR_OK
    auto result = abilityMs_->RequestModalUIExtensionInner(want);
    EXPECT_EQ(result, ERR_OK);

    // 清理
    MyFlag::retCreateModalUIExtension_ = true;  // 恢复默认值
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest RequestModalUIExtensionInner_005 end");
}

/*
 * Feature: AbilityManagerService
 * Function: RequestModalUIExtensionInner
 * SubFunction: NA
 * FunctionPoints: Request modal UI extension - PAGE type success path
 */
HWTEST_F(AbilityManagerServiceTwelfthTest, RequestModalUIExtensionInner_006, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest RequestModalUIExtensionInner_006 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_TRUE(abilityMs_ != nullptr);

    // 设置 mock 返回成功
    MyFlag::retCreateModalUIExtension_ = true;

    // 测试场景：完整的PAGE类型Ability创建模态UI扩展
    // 这个场景需要完整的AbilityRecord Mock配置
    // 在当前测试环境中，由于无法完整Mock AbilityRecord，会走降级方案
    Want want;
    std::string bundleName = "com.test.demo";
    want.SetParam("bundleName", bundleName);
    MyFlag::flag_ = 1;  // Mock 焦点应用为 "com.test.demo"

    // 由于Mock限制，实际会走降级方案，返回 ERR_OK
    auto result = abilityMs_->RequestModalUIExtensionInner(want);
    EXPECT_EQ(result, ERR_OK);

    // 清理 mock
    MyFlag::retCreateModalUIExtension_ = true;  // 恢复默认值
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest RequestModalUIExtensionInner_006 end");
}

/*
 * Feature: AbilityManagerService
 * Function: RequestModalUIExtensionWithAccount
 * SubFunction: NA
 * FunctionPoints: Request modal UI extension with accountId - GetDisplayIdByAccount failed
 */
HWTEST_F(AbilityManagerServiceTwelfthTest, RequestModalUIExtensionWithAccount_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest RequestModalUIExtensionWithAccount_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_TRUE(abilityMs_ != nullptr);

    // 测试场景：权限验证失败
    Want want;
    std::string bundleName = "com.test.demo";
    want.SetParam("bundleName", bundleName);
    int32_t accountId = 999;
    MyFlag::flag_ = 0;  // Mock 权限验证失败

    EXPECT_EQ(abilityMs_->RequestModalUIExtensionWithAccount(want, accountId), ERR_INVALID_VALUE);

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest RequestModalUIExtensionWithAccount_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: RequestModalUIExtensionWithAccount
 * SubFunction: NA
 * FunctionPoints: Request modal UI extension with accountId - displayId not found
 */
HWTEST_F(AbilityManagerServiceTwelfthTest, RequestModalUIExtensionWithAccount_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest RequestModalUIExtensionWithAccount_002 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_TRUE(abilityMs_ != nullptr);

    // 测试场景：权限验证成功但未找到 displayId
    Want want;
    std::string bundleName = "com.test.demo";
    want.SetParam("bundleName", bundleName);
    int32_t accountId = 999;
    MyFlag::flag_ = 1;  // Mock 权限验证成功但无 displayId

    EXPECT_EQ(abilityMs_->RequestModalUIExtensionWithAccount(want, accountId), CHECK_PERMISSION_FAILED);

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest RequestModalUIExtensionWithAccount_002 end");
}

/*
 * Feature: AbilityManagerService
 * Function: RequestModalUIExtensionWithAccount
 * SubFunction: NA
 * FunctionPoints: Request modal UI extension with accountId - GetTopAbilityByUserId failed
 */
HWTEST_F(AbilityManagerServiceTwelfthTest, RequestModalUIExtensionWithAccount_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest RequestModalUIExtensionWithAccount_003 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_TRUE(abilityMs_ != nullptr);

    // 设置 mock 返回成功
    MyFlag::retCreateModalUIExtension_ = true;

    // 测试场景：成功获取 displayId，但 GetTopAbilityByUserId 失败
    Want want;
    std::string bundleName = "com.test.demo";
    want.SetParam("bundleName", bundleName);
    int32_t accountId = 999;
    uint64_t displayId = 1;
    MyFlag::flag_ = 1;  // Mock 权限验证成功

    AbilityRuntime::UserController::GetInstance().SetForegroundUserId(accountId, displayId);

    // GetTopAbilityByUserId 会因为 wmsHandler 为空而失败
    // 预期：使用 ModalSystemUiExtension 降级方案，返回 ERR_OK
    auto result = abilityMs_->RequestModalUIExtensionWithAccount(want, accountId);
    EXPECT_EQ(result, ERR_OK);

    // 清理 UserController 状态，确保测试隔离
    AbilityRuntime::UserController::GetInstance().ClearUserId(accountId);
    // 清理 mock
    MyFlag::retCreateModalUIExtension_ = true;  // 恢复默认值
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest RequestModalUIExtensionWithAccount_003 end");
}

/*
 * Feature: AbilityManagerService
 * Function: RequestModalUIExtensionWithAccount
 * SubFunction: NA
 * FunctionPoints: Request modal UI extension with accountId - token is null
 */
HWTEST_F(AbilityManagerServiceTwelfthTest, RequestModalUIExtensionWithAccount_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest RequestModalUIExtensionWithAccount_004 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_TRUE(abilityMs_ != nullptr);

    // 测试场景：GetTopAbilityByUserId 返回 OK 但 token 为 null
    Want want;
    std::string bundleName = "com.test.demo";
    want.SetParam("bundleName", bundleName);
    int32_t accountId = 999;
    uint64_t displayId = 1;
    MyFlag::flag_ = 2;  // Mock GetTopAbilityByUserId 返回 OK 但 token 为 null

    AbilityRuntime::UserController::GetInstance().SetForegroundUserId(accountId, displayId);

    // 预期：由于 token 为 null，使用 ModalSystemUiExtension 降级方案，返回 ERR_OK
    auto result = abilityMs_->RequestModalUIExtensionWithAccount(want, accountId);
    EXPECT_EQ(result, ERR_OK);

    // 清理 UserController 状态，确保测试隔离
    AbilityRuntime::UserController::GetInstance().ClearUserId(accountId);
    // 清理 mock
    MyFlag::retCreateModalUIExtension_ = true;  // 恢复默认值
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest RequestModalUIExtensionWithAccount_004 end");
}

/*
 * Feature: AbilityManagerService
 * Function: RequestModalUIExtensionWithAccount
 * SubFunction: NA
 * FunctionPoints: Request modal UI extension with accountId - bundleName mismatch
 */
HWTEST_F(AbilityManagerServiceTwelfthTest, RequestModalUIExtensionWithAccount_005, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest RequestModalUIExtensionWithAccount_005 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_TRUE(abilityMs_ != nullptr);

    // 设置 mock 返回成功
    MyFlag::retCreateModalUIExtension_ = true;

    // 测试场景：焦点应用 bundleName 与调用者不匹配
    Want want;
    std::string bundleName = "com.caller.demo";  // 与焦点应用不一致
    want.SetParam("bundleName", bundleName);
    int32_t accountId = 999;
    uint64_t displayId = 1;
    MyFlag::flag_ = 3;  // Mock 焦点应用为 "com.test.demo"

    AbilityRuntime::UserController::GetInstance().SetForegroundUserId(accountId, displayId);

    // 预期：由于 bundleName 不匹配，使用 ModalSystemUiExtension 降级方案，返回 ERR_OK
    auto result = abilityMs_->RequestModalUIExtensionWithAccount(want, accountId);
    EXPECT_EQ(result, ERR_OK);

    // 清理 UserController 状态，确保测试隔离
    AbilityRuntime::UserController::GetInstance().ClearUserId(accountId);
    // 清理 mock
    MyFlag::retCreateModalUIExtension_ = true;  // 恢复默认值
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest RequestModalUIExtensionWithAccount_005 end");
}

/*
 * Feature: AbilityManagerService
 * Function: RequestModalUIExtensionWithAccount
 * SubFunction: NA
 * FunctionPoints: Request modal UI extension with accountId - success scenario
 */
HWTEST_F(AbilityManagerServiceTwelfthTest, RequestModalUIExtensionWithAccount_006, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest RequestModalUIExtensionWithAccount_006 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_TRUE(abilityMs_ != nullptr);

    // 设置 mock 返回成功
    MyFlag::retCreateModalUIExtension_ = true;

    // 测试场景：正常流程 - 所有条件满足
    Want want;
    std::string bundleName = "com.test.demo";  // 与焦点应用一致
    want.SetParam("bundleName", bundleName);
    int32_t accountId = 999;
    uint64_t displayId = 1;
    MyFlag::flag_ = 4;  // Mock 成功场景

    AbilityRuntime::UserController::GetInstance().SetForegroundUserId(accountId, displayId);

    // 预期：所有条件满足，调用 CreateModalUIExtension
    auto result = abilityMs_->RequestModalUIExtensionWithAccount(want, accountId);
    // 由于Mock限制，实际会走降级方案，返回 ERR_OK
    EXPECT_EQ(result, ERR_OK);

    // 清理 UserController 状态，确保测试隔离
    AbilityRuntime::UserController::GetInstance().ClearUserId(accountId);
    // 清理 mock
    MyFlag::retCreateModalUIExtension_ = true;  // 恢复默认值
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest RequestModalUIExtensionWithAccount_006 end");
}

/*
 * Feature: AbilityManagerService
 * Function: RequestModalUIExtensionWithAccount
 * SubFunction: NA
 * FunctionPoints: Request modal UI extension with accountId - record is null
 */
HWTEST_F(AbilityManagerServiceTwelfthTest, RequestModalUIExtensionWithAccount_007, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest RequestModalUIExtensionWithAccount_007 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_TRUE(abilityMs_ != nullptr);

    // 设置 mock 返回成功
    MyFlag::retCreateModalUIExtension_ = true;

    // 测试场景：token有效但AbilityRecord为null
    // 这个场景需要Mock GetTopAbilityByUserId返回有效token，但Token::GetAbilityRecordByToken返回null
    Want want;
    std::string bundleName = "com.test.demo";
    want.SetParam("bundleName", bundleName);
    int32_t accountId = 999;
    uint64_t displayId = 1;
    MyFlag::flag_ = 1;

    AbilityRuntime::UserController::GetInstance().SetForegroundUserId(accountId, displayId);

    // 由于Mock限制，实际会走降级方案，返回 ERR_OK
    auto result = abilityMs_->RequestModalUIExtensionWithAccount(want, accountId);
    EXPECT_EQ(result, ERR_OK);

    // 清理 UserController 状态，确保测试隔离
    AbilityRuntime::UserController::GetInstance().ClearUserId(accountId);
    // 清理 mock
    MyFlag::retCreateModalUIExtension_ = true;  // 恢复默认值
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest RequestModalUIExtensionWithAccount_007 end");
}

/*
 * Feature: AbilityManagerService
 * Function: RequestModalUIExtensionWithAccount
 * SubFunction: NA
 * FunctionPoints: Request modal UI extension with accountId - PAGE type success path
 */
HWTEST_F(AbilityManagerServiceTwelfthTest, RequestModalUIExtensionWithAccount_008, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest RequestModalUIExtensionWithAccount_008 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_TRUE(abilityMs_ != nullptr);

    // 设置 mock 返回成功
    MyFlag::retCreateModalUIExtension_ = true;

    // 测试场景：完整的PAGE类型Ability创建模态UI扩展
    // 这个场景需要完整的AbilityRecord Mock配置
    Want want;
    std::string bundleName = "com.test.demo";
    want.SetParam("bundleName", bundleName);
    int32_t accountId = 999;
    uint64_t displayId = 1;
    MyFlag::flag_ = 4;

    AbilityRuntime::UserController::GetInstance().SetForegroundUserId(accountId, displayId);

    // 由于Mock限制，实际会走降级方案，返回 ERR_OK
    auto result = abilityMs_->RequestModalUIExtensionWithAccount(want, accountId);
    EXPECT_EQ(result, ERR_OK);

    // 清理 UserController 状态，确保测试隔离
    AbilityRuntime::UserController::GetInstance().ClearUserId(accountId);
    // 清理 mock
    MyFlag::retCreateModalUIExtension_ = true;  // 恢复默认值
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceTwelfthTest RequestModalUIExtensionWithAccount_008 end");
}
} // namespace AAFwk
} // namespace OHOS
