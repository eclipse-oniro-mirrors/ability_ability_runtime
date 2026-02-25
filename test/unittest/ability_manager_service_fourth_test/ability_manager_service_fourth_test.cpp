/*
 * Copyright (c) 2023-2026 Huawei Device Co., Ltd.
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
#include "mock_ipc_skeleton.h"
#include "mock_permission_verification.h"
#include "mock_my_flag.h"
#include "mock_parameters.h"
#include "ability_manager_service.h"
#include "insight_intent_execute_manager.h"
#include "modal_system_dialog/modal_system_dialog_ui_extension.h"
#include "user_controller/user_controller.h"
#include "utils/modal_system_dialog_util.h"
#undef private
#undef protected
#include "hilog_tag_wrapper.h"
#include "mock_ability_token.h"
#include "ability_bundle_event_callback.h"
#include "session/host/include/session.h"
#include "start_ability_utils.h"
#include "start_params_by_SCB.h"
#include "system_ability_definition.h"
#include "ability_util.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AppExecFwk;
using OHOS::AppExecFwk::AbilityType;
using OHOS::AppExecFwk::ExtensionAbilityType;

constexpr char DEVELOPER_MODE_STATE[] = "const.security.developermode.state";
constexpr const char* DEBUG_APP = "debugApp";
constexpr const char* START_ABILITY_TYPE = "ABILITY_INNER_START_WITH_ACCOUNT";

constexpr int32_t FOUNDATION_UID = 5523;
constexpr int32_t TEST_VALID_USER_ID = 100;

namespace OHOS {
namespace AAFwk {
class AbilityManagerServiceFourthTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

    std::shared_ptr<AbilityRecord> MockAbilityRecord(AbilityType);
    sptr<Token> MockToken(AbilityType);
    sptr<SessionInfo> MockSessionInfo(int32_t persistentId);

    std::shared_ptr<AbilityRecord> abilityRecord;
};

void AbilityManagerServiceFourthTest::SetUpTestCase() {}

void AbilityManagerServiceFourthTest::TearDownTestCase() {}

void AbilityManagerServiceFourthTest::SetUp() {}

void AbilityManagerServiceFourthTest::TearDown() {}

sptr<SessionInfo> AbilityManagerServiceFourthTest::MockSessionInfo(int32_t persistentId)
{
    sptr<SessionInfo> sessionInfo = new (std::nothrow) SessionInfo();
    if (!sessionInfo) {
        TAG_LOGE(AAFwkTag::TEST, "sessionInfo is nullptr");
        return nullptr;
    }
    sessionInfo->persistentId = persistentId;
    return sessionInfo;
}

std::shared_ptr<AbilityRecord> AbilityManagerServiceFourthTest::MockAbilityRecord(AbilityType abilityType)
{
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.test.demo";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = abilityType;
    return AbilityRecord::CreateAbilityRecord(abilityRequest);
}

sptr<Token> AbilityManagerServiceFourthTest::MockToken(AbilityType abilityType)
{
    abilityRecord = MockAbilityRecord(abilityType);
    if (!abilityRecord) {
        return nullptr;
    }
    return abilityRecord->GetToken();
}

class IRemoteObjectMocker : public IRemoteObject {
public:
    IRemoteObjectMocker() : IRemoteObject { u"IRemoteObjectMocker" } {}

    ~IRemoteObjectMocker() {}

    int32_t GetObjectRefCount()
    {
        return 0;
    }

    int SendRequest(uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option)
    {
        int32_t key = data.ReadInt32();
        std::string parameters = Str16ToStr8(data.ReadString16());
        std::string command = Str16ToStr8(data.ReadString16());
        if (command.compare("test") == 0) {
            isSuccess_ = true;
            return 0;
        }
        isSuccess_ = false;
        return -1;
    }

    bool IsProxyObject() const
    {
        return true;
    }

    bool CheckObjectLegality() const
    {
        return true;
    }

    bool AddDeathRecipient(const sptr<DeathRecipient>& recipient)
    {
        return true;
    }

    bool RemoveDeathRecipient(const sptr<DeathRecipient>& recipient)
    {
        return true;
    }

    sptr<IRemoteBroker> AsInterface()
    {
        return nullptr;
    }

    int Dump(int fd, const std::vector<std::u16string>& args)
    {
        return 0;
    }

public:
    bool isSuccess_ = false;
};

/*
 * Feature: AbilityManagerService
 * Function: AddFreeInstallObserver
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService AddFreeInstallObserver
 */
HWTEST_F(AbilityManagerServiceFourthTest, AddFreeInstallObserver_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest AddFreeInstallObserver_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    sptr<AbilityRuntime::IFreeInstallObserver> observer;
    EXPECT_EQ(abilityMs_->AddFreeInstallObserver(nullptr, observer), ERR_INVALID_VALUE);

    abilityMs_->freeInstallManager_ = std::make_shared<FreeInstallManager>();
    EXPECT_EQ(abilityMs_->AddFreeInstallObserver(nullptr, observer), ERR_INVALID_VALUE);

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest AddFreeInstallObserver_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: VerifyPermission
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService VerifyPermission
 */
HWTEST_F(AbilityManagerServiceFourthTest, VerifyPermission_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest VerifyPermission_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();

    std::string permission = "test_permission";
    int pid = 0;
    int uid = 0;
    EXPECT_EQ(abilityMs_->VerifyPermission(permission, pid, uid), CHECK_PERMISSION_FAILED);

    std::string permission2 = "";
    EXPECT_EQ(abilityMs_->VerifyPermission(permission2, pid, uid), CHECK_PERMISSION_FAILED);

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest VerifyPermission_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: AcquireShareData
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService AcquireShareData
 */
HWTEST_F(AbilityManagerServiceFourthTest, AcquireShareData_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest AcquireShareData_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();

    int32_t missionId = 1;
    sptr<IAcquireShareDataCallback> shareData = nullptr;
    EXPECT_EQ(abilityMs_->AcquireShareData(missionId, shareData), ERR_INVALID_VALUE);

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest AcquireShareData_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: ShareDataDone
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ShareDataDone
 */
HWTEST_F(AbilityManagerServiceFourthTest, ShareDataDone_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest ShareDataDone_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();

    sptr<IRemoteObject> token = nullptr;
    int32_t resultCode = 1;
    int32_t uniqueId = 1;
    WantParams wantParam;
    EXPECT_EQ(abilityMs_->ShareDataDone(token, resultCode, uniqueId, wantParam), ERR_INVALID_VALUE);

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest ShareDataDone_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: NotifySaveAsResult
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService NotifySaveAsResult
 */
HWTEST_F(AbilityManagerServiceFourthTest, NotifySaveAsResult_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest NotifySaveAsResult_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();

    Want want;
    auto result = abilityMs_->NotifySaveAsResult(want, 0, 0);
    EXPECT_EQ(result, CHECK_PERMISSION_FAILED);

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest NotifySaveAsResult_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: OnStop
 * FunctionPoints: AbilityManagerService OnStop
 */
HWTEST_F(AbilityManagerServiceFourthTest, OnStop_001, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    abilityMs->abilityBundleEventCallback_ = new (std::nothrow) AbilityBundleEventCallback(nullptr, nullptr);
    abilityMs->OnStop();
    EXPECT_NE(abilityMs->abilityBundleEventCallback_, nullptr);
}

/*
 * Feature: AbilityManagerService
 * Function: OnStop
 * FunctionPoints: AbilityManagerService OnStop
 */
HWTEST_F(AbilityManagerServiceFourthTest, OnStop_002, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    abilityMs->OnStop();
    EXPECT_EQ(abilityMs->abilityBundleEventCallback_, nullptr);
}

/*
 * Feature: AbilityManagerService
 * Function: GetConfiguration
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetConfiguration
 */
HWTEST_F(AbilityManagerServiceFourthTest, GetConfiguration_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest GetConfiguration_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    AppExecFwk::Configuration config;
    abilityMs_->SubscribeBackgroundTask();
    EXPECT_EQ(abilityMs_->GetConfiguration(config), 0);
    abilityMs_->UnSubscribeBackgroundTask();
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest GetConfiguration_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: ReportAbilityStartInfoToRSS
 * FunctionPoints: AbilityManagerService ReportAbilityStartInfoToRSS
 */
HWTEST_F(AbilityManagerServiceFourthTest, ReportAbilityStartInfoToRSS_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest ReportAbilityStartInfoToRSS start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.type = AppExecFwk::AbilityType::PAGE;
    abilityMs->ReportAbilityStartInfoToRSS(abilityInfo);
    EXPECT_EQ(abilityInfo.type, AppExecFwk::AbilityType::PAGE);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest ReportAbilityStartInfoToRSS end");
}

/*
 * Feature: AbilityManagerService
 * Function: ReportAbilityAssociatedStartInfoToRSS
 * FunctionPoints: AbilityManagerService ReportAbilityAssociatedStartInfoToRSS
 */
HWTEST_F(AbilityManagerServiceFourthTest, ReportAbilityAssociatedStartInfoToRSS_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest ReportAbilityAssociatedStartInfoToRSS start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.type = AppExecFwk::AbilityType::PAGE;
    sptr<IRemoteObject> callerToken = nullptr;
    int64_t type = 0;
    abilityMs->ReportAbilityAssociatedStartInfoToRSS(abilityInfo, type, callerToken);
    callerToken = MockToken(AbilityType::PAGE);
    abilityMs->ReportAbilityAssociatedStartInfoToRSS(abilityInfo, type, callerToken);
    EXPECT_EQ(abilityInfo.type, AppExecFwk::AbilityType::PAGE);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest ReportAbilityAssociatedStartInfoToRSS end");
}

/*
 * Feature: AbilityManagerService
 * Function: StartExtensionAbilityInner
 * FunctionPoints: AbilityManagerService StartExtensionAbilityInner
 */
HWTEST_F(AbilityManagerServiceFourthTest, StartExtensionAbilityInner_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest StartExtensionAbilityInner_004 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    Want want;
    sptr<IRemoteObject> callerToken = nullptr;
    int32_t userId = 0;
    AppExecFwk::ExtensionAbilityType extensionType = AppExecFwk::ExtensionAbilityType::VPN;
    bool checkSystemCaller = true;
    bool isImplicit = true;
    bool isDlp = true;
    abilityMs->interceptorExecuter_ = std::make_shared<AbilityInterceptorExecuter>();
    abilityMs->subManagersHelper_ = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    auto result = abilityMs->StartExtensionAbilityInner(want, callerToken, userId, extensionType, checkSystemCaller,
        isImplicit, isDlp);
    EXPECT_EQ(result, ERR_IMPLICIT_START_ABILITY_FAIL);

    want.SetElementName("test.bundleName", "test.abilityName");
    abilityMs-> implicitStartProcessor_ = std::make_shared<ImplicitStartProcessor>();
    result = abilityMs->StartExtensionAbilityInner(want, callerToken, userId, extensionType, checkSystemCaller,
        isImplicit, isDlp);
    EXPECT_NE(result, ERR_IMPLICIT_START_ABILITY_FAIL);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest StartExtensionAbilityInner_004 end");
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityWithSpecifyTokenId
 * FunctionPoints: AbilityManagerService StartAbilityWithSpecifyTokenId
 */
HWTEST_F(AbilityManagerServiceFourthTest, StartAbilityWithSpecifyTokenId_002, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    Want want;
    sptr<IRemoteObject> callerToken;
    uint32_t specifyTokenId = 0;
    int32_t userId = 0;
    int32_t requestCode = 0;
    IPCSkeleton::SetCallingUid(FOUNDATION_UID);
    auto result = abilityMs->StartAbilityWithSpecifyTokenId(want, callerToken, specifyTokenId, userId, requestCode);
    EXPECT_NE(result, ERR_INVALID_CONTINUATION_FLAG);
}

/*
 * Feature: AbilityManagerService
 * Function: StartUIAbilityBySCB
 * FunctionPoints: AbilityManagerService StartUIAbilityBySCB
 */
HWTEST_F(AbilityManagerServiceFourthTest, StartUIAbilityBySCB_003, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    Rosen::SessionInfo info;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->sessionToken = new Rosen::Session(info);
    bool isColdStart = true;
    AbilityRuntime::StartParamsBySCB params;
    auto result = abilityMs->StartUIAbilityBySCB(sessionInfo, params, isColdStart);
    EXPECT_EQ(result, ERR_WRONG_INTERFACE_CALL);
    abilityMs->subManagersHelper_ = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    result = abilityMs->StartUIAbilityBySCB(sessionInfo, params, isColdStart);
    EXPECT_EQ(result, ERR_WRONG_INTERFACE_CALL);
}

/*
 * Feature: AbilityManagerService
 * Function: RequestDialogServiceInner
 * FunctionPoints: AbilityManagerService RequestDialogServiceInner
 */
HWTEST_F(AbilityManagerServiceFourthTest, RequestDialogServiceInner_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest RequestDialogServiceInner_001 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    Want want;
    const sptr<IRemoteObject> callerToken;
    int32_t userId = 0;
    int requestCode = 0;
    auto result = abilityMs->RequestDialogServiceInner(want, callerToken, requestCode, userId);
    EXPECT_EQ(result, ERR_INVALID_CALLER);

    abilityMs->subManagersHelper_ = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    abilityMs->subManagersHelper_->currentUIAbilityManager_ = std::make_shared<UIAbilityLifecycleManager>();
    std::shared_ptr<AbilityRecord> abilityRecord = MockAbilityRecord(AbilityType::PAGE);
    sptr<IRemoteObject> callerToken2 = abilityRecord->GetToken();

    result = abilityMs->RequestDialogServiceInner(want, callerToken2, requestCode, userId);
    EXPECT_EQ(result, ERR_INVALID_CALLER);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceThirdTest RequestDialogServiceInner_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityForOptionInner
 * FunctionPoints: AbilityManagerService StartAbilityForOptionInner
 */
HWTEST_F(AbilityManagerServiceFourthTest, StartAbilityForOptionInner_001, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    Want want;
    StartOptions startOptions;
    const sptr<IRemoteObject> callerToken;
    int32_t userId = 0;
    int requestCode = 0;
    bool isStartAsCaller = true;
    uint32_t specifyTokenId = 0;
    bool isImplicit = true;
    auto result = abilityMs->StartAbilityForOptionInner(want, startOptions, callerToken, false, userId, requestCode,
        isStartAsCaller, specifyTokenId, isImplicit);
    EXPECT_EQ(result, ERR_NULL_INTERCEPTOR_EXECUTER);
    abilityMs->interceptorExecuter_ = std::make_shared<AbilityInterceptorExecuter>();
    result = abilityMs->StartAbilityForOptionInner(want, startOptions, callerToken, false, userId, requestCode,
        isStartAsCaller, specifyTokenId, isImplicit);
    EXPECT_EQ(result, ERR_IMPLICIT_START_ABILITY_FAIL);
    want.SetElementName("test.bundleName", "test.abilityName");
    abilityMs-> implicitStartProcessor_ = std::make_shared<ImplicitStartProcessor>();
    result = abilityMs->StartAbilityForOptionInner(want, startOptions, callerToken, false, userId, requestCode,
       isStartAsCaller, specifyTokenId, isImplicit);
    EXPECT_NE(result, ERR_INVALID_VALUE);
}


/*
 * Feature: AbilityManagerService
 * Function: InitDeepLinkReserve
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService InitDeepLinkReserve
 */
HWTEST_F(AbilityManagerServiceFourthTest, InitDeepLinkReserve_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest InitDeepLinkReserve_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->InitDeepLinkReserve();
    EXPECT_TRUE(abilityMs_ != nullptr);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest InitDeepLinkReserve_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: InitInterceptor
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService InitInterceptor
 */
HWTEST_F(AbilityManagerServiceFourthTest, InitInterceptor_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest InitInterceptor_001 start");
    OHOS::system::SetBoolParameter(OHOS::AppExecFwk::PARAMETER_APP_JUMP_INTERCEPTOR_ENABLE, false);
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->InitInterceptor();
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest InitInterceptor_001 mid");
    OHOS::system::SetBoolParameter(OHOS::AppExecFwk::PARAMETER_APP_JUMP_INTERCEPTOR_ENABLE, true);
    abilityMs_->InitInterceptor();
    EXPECT_TRUE(abilityMs_ != nullptr);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest InitInterceptor_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: InitStartupFlag
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService InitStartupFlag
 */
HWTEST_F(AbilityManagerServiceFourthTest, InitStartupFlag_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest InitStartupFlag_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->InitStartupFlag();
    EXPECT_TRUE(abilityMs_ != nullptr);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest InitStartupFlag_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: QueryServiceState
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService QueryServiceState
 */
HWTEST_F(AbilityManagerServiceFourthTest, QueryServiceState_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest QueryServiceState_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->QueryServiceState();
    EXPECT_TRUE(abilityMs_ != nullptr);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest QueryServiceState_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbility
 */
HWTEST_F(AbilityManagerServiceFourthTest, StartAbility_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest StartAbility_001 start");
    Want want;
    int32_t userId{0};
    int requestCode{0};
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    auto ret = abilityMs_->StartAbility(want, userId, requestCode);
    EXPECT_EQ(ret, ERR_NULL_INTERCEPTOR_EXECUTER);

    want.SetParam(DEBUG_APP, true);
    system::SetBoolParameter(DEVELOPER_MODE_STATE, false);
    auto ret1 = abilityMs_->StartAbility(want, userId, requestCode);
    EXPECT_EQ(ret1, ERR_NOT_DEVELOPER_MODE);

    want.SetParam(DEBUG_APP, false);
    want.SetParam(START_ABILITY_TYPE, true);
    auto ret2 = abilityMs_->StartAbility(want, userId, requestCode);
    EXPECT_EQ(ret2, ERR_NULL_INTERCEPTOR_EXECUTER);

    want.SetParam(DEBUG_APP, false);
    want.SetParam(START_ABILITY_TYPE, false);
    want.SetParam(Want::PARAM_RESV_WINDOW_LEFT, 1);
    system::SetBoolParameter(DEVELOPER_MODE_STATE, true);
    auto ret3 = abilityMs_->StartAbility(want, userId, requestCode);
    EXPECT_EQ(ret2, ERR_NULL_INTERCEPTOR_EXECUTER);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest StartAbility_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbility
 */
HWTEST_F(AbilityManagerServiceFourthTest, StartAbility_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest StartAbility_002 start");
    Want want;
    auto callerToken = MockToken(AbilityType::PAGE);
    int32_t userId{0};
    int requestCode{0};
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    auto ret = abilityMs_->StartAbility(want, callerToken, userId, requestCode);
    EXPECT_EQ(ret, ERR_INVALID_CALLER);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest StartAbility_002 end");
}

/*
* Feature: AbilityManagerService
* Function: StartAbilityWithRemoveIntentFlag
* SubFunction: NA
* FunctionPoints: AbilityManagerService StartAbilityWithRemoveIntentFlag
*/
HWTEST_F(AbilityManagerServiceFourthTest, StartAbilityWithRemoveIntentFlag_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest StartAbilityWithRemoveIntentFlag_003 start");
    Want want;
    auto callerToken = MockToken(AbilityType::PAGE);
    int32_t userId{0};
    int requestCode{0};
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    StartAbilityWrapParam param = {
        .want = want,
        .callerToken = callerToken,
        .requestCode = requestCode,
        .userId = userId,
        .removeInsightIntentFlag = true,
    };
    auto ret = abilityMs_->StartAbilityWithRemoveIntentFlag(param);
    EXPECT_EQ(ret, ERR_INVALID_CALLER);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest StartAbilityWithRemoveIntentFlag_003 end");
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityByFreeInstall
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbilityByFreeInstall
 */
HWTEST_F(AbilityManagerServiceFourthTest, StartAbilityByFreeInstall_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest StartAbilityByFreeInstall_001 start");
    Want want;
    int32_t userId{0};
    int requestCode{0};
    want.SetParam(START_ABILITY_TYPE, true);
    auto callerToken = MockToken(AbilityType::PAGE);
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    StartAbilityWrapParam param = {
        .want = want,
        .callerToken = callerToken,
        .requestCode = requestCode,
        .userId = userId,
    };
    auto ret = abilityMs_->StartAbilityByFreeInstall(param);
    EXPECT_EQ(ret, ERR_INVALID_CALLER);

    param.want.SetParam(START_ABILITY_TYPE, false);
    param.want.AddFlags(Want::FLAG_ABILITY_CONTINUATION);
    auto ret1 = abilityMs_->StartAbilityByFreeInstall(param);
    EXPECT_EQ(ret1, ERR_INVALID_CONTINUATION_FLAG);

    param.want.SetParam(START_ABILITY_TYPE, false);
    param.want.RemoveFlags(Want::FLAG_ABILITY_CONTINUATION);
    auto ret2 = abilityMs_->StartAbilityByFreeInstall(param);
    EXPECT_EQ(ret2, ERR_INVALID_CALLER);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest StartAbilityByFreeInstall_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityWithSpecifyTokenIdInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbilityWithSpecifyTokenIdInner
 */
HWTEST_F(AbilityManagerServiceFourthTest, StartAbilityWithSpecifyTokenIdInner_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest StartAbilityWithSpecifyTokenIdInner_001 start");
    Want want;
    uint32_t specifyTokenId{0};
    int32_t userId{0};
    int requestCode{0};
    auto callerToken = MockToken(AbilityType::PAGE);
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    want.AddFlags(Want::FLAG_ABILITY_CONTINUATION);
    auto ret = abilityMs_->StartAbilityWithSpecifyTokenIdInner(
        want, callerToken, specifyTokenId, false, userId, requestCode);
    EXPECT_EQ(ret, ERR_INVALID_CONTINUATION_FLAG);

    want.RemoveFlags(Want::FLAG_ABILITY_CONTINUATION);
    auto ret1 = abilityMs_->StartAbilityWithSpecifyTokenIdInner(
        want, callerToken, specifyTokenId, false, userId, requestCode);
    EXPECT_EQ(ret1, ERR_INVALID_CALLER);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest StartAbilityWithSpecifyTokenIdInner_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityWithSpecifyTokenIdInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbilityWithSpecifyTokenIdInner
 */
HWTEST_F(AbilityManagerServiceFourthTest, StartAbilityWithSpecifyTokenIdInner_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest StartAbilityWithSpecifyTokenIdInner_002 start");
    Want want;
    StartOptions startOptions;
    auto callerToken = MockToken(AbilityType::PAGE);
    int32_t userId{0};
    int requestCode{0};
    uint32_t callerTokenId{0};
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    auto ret = abilityMs_->StartAbilityWithSpecifyTokenIdInner(
        want, startOptions, callerToken, false, userId, requestCode, callerTokenId);
    EXPECT_EQ(ret, ERR_INVALID_CALLER);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest StartAbilityWithSpecifyTokenIdInner_002 end");
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityByInsightIntent
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbilityByInsightIntent
 */
HWTEST_F(AbilityManagerServiceFourthTest, StartAbilityByInsightIntent_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest StartAbilityByInsightIntent_001 start");
    Want want;
    auto callerToken = MockToken(AbilityType::PAGE);
    uint64_t key{0};
    std::string bundleName{""};
    uint64_t intentId{1};
    int32_t userId{0};
    std::string callerBundleName = "com.example.TDDTest";
    DelayedSingleton<InsightIntentExecuteManager>::GetInstance()->AddRecord(key, callerToken, bundleName, intentId,
        callerBundleName);
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    auto ret = abilityMs_->StartAbilityByInsightIntent(want, callerToken, intentId, userId);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest StartAbilityByInsightIntent_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityByUIContentSession
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbilityByUIContentSession
 */
HWTEST_F(AbilityManagerServiceFourthTest, StartAbilityByUIContentSession_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest StartAbilityByUIContentSession_001 start");

    Want want;
    auto callerToken = MockToken(AbilityType::PAGE);
    sptr<SessionInfo> sessionInfo{nullptr};
    int32_t userId{0};
    int requestCode{0};
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    auto ret = abilityMs_->StartAbilityByUIContentSession(want, callerToken, sessionInfo, userId, requestCode);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);

    sessionInfo = MockSessionInfo(0);
    auto ret1 = abilityMs_->StartAbilityByUIContentSession(want, callerToken, sessionInfo, userId, requestCode);
    EXPECT_EQ(ret1, ERR_INVALID_VALUE);

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest StartAbilityByUIContentSession_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityByUIContentSession
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbilityByUIContentSession
 */
HWTEST_F(AbilityManagerServiceFourthTest, StartAbilityByUIContentSession_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest StartAbilityByUIContentSession_002 start");
    Want want;
    StartOptions startOptions;
    auto callerToken = MockToken(AbilityType::PAGE);
    sptr<SessionInfo> sessionInfo{nullptr};
    int32_t userId{0};
    int requestCode{0};
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    auto ret = abilityMs_->StartAbilityByUIContentSession(
        want, startOptions, callerToken, sessionInfo, userId, requestCode);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);

    sessionInfo = MockSessionInfo(0);
    auto ret1 = abilityMs_->StartAbilityByUIContentSession(
        want, startOptions, callerToken, sessionInfo, userId, requestCode);
    EXPECT_EQ(ret1, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest StartAbilityByUIContentSession_002 end");
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityAsCaller
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbilityAsCaller
 */
HWTEST_F(AbilityManagerServiceFourthTest, StartAbilityAsCaller_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest StartAbilityAsCaller_001 start");
    Want want;
    auto callerToken = MockToken(AbilityType::PAGE);
    int32_t userId{0};
    int requestCode{0};
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    auto ret = abilityMs_->StartAbilityAsCaller(want, callerToken, callerToken, userId, requestCode);
    EXPECT_EQ(ret, ERR_INVALID_CALLER);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest StartAbilityAsCaller_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: ImplicitStartAbilityAsCaller
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ImplicitStartAbilityAsCaller
 */
HWTEST_F(AbilityManagerServiceFourthTest, ImplicitStartAbilityAsCaller_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest ImplicitStartAbilityAsCaller_001 start");
    Want want;
    auto callerToken = MockToken(AbilityType::PAGE);
    int32_t userId{0};
    int requestCode{0};
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    auto ret = abilityMs_->ImplicitStartAbilityAsCaller(want, callerToken, callerToken, userId, requestCode);
    EXPECT_EQ(ret, ERR_INVALID_CALLER);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest ImplicitStartAbilityAsCaller_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityAsCallerDetails
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbilityAsCallerDetails
 */
HWTEST_F(AbilityManagerServiceFourthTest, StartAbilityAsCallerDetails_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest StartAbilityAsCallerDetails_001 start");
    Want want;
    want.AddFlags(Want::FLAG_ABILITY_CONTINUATION);
    auto callerToken = MockToken(AbilityType::PAGE);
    auto asCallerSourceToken = MockToken(AbilityType::PAGE);
    int32_t userId{0};
    int requestCode{0};
    bool isImplicit{true};
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    auto ret = abilityMs_->StartAbilityAsCallerDetails(
        want, callerToken, asCallerSourceToken, userId, requestCode, isImplicit);
    EXPECT_EQ(ret, ERR_INVALID_CONTINUATION_FLAG);

    want.RemoveFlags(Want::FLAG_ABILITY_CONTINUATION);
    auto ret1 = abilityMs_->StartAbilityAsCallerDetails(
        want, callerToken, asCallerSourceToken, userId, requestCode, isImplicit);
    EXPECT_EQ(ret1, ERR_INVALID_CALLER);

    string callerPkg{"test"};
    want.SetParam(AbilityUtil::JUMP_INTERCEPTOR_DIALOG_CALLER_PKG, callerPkg);
    want.SetElementName("com.ohos.sceneboard", "com.ohos.sceneboard.systemdialog");
    auto ret2 = abilityMs_->StartAbilityAsCallerDetails(
        want, callerToken, asCallerSourceToken, userId, requestCode, isImplicit);
    EXPECT_EQ(ret2, ERR_INVALID_CALLER);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest StartAbilityAsCallerDetails_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityPublicPrechainCheck
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbilityPublicPrechainCheck
 */
HWTEST_F(AbilityManagerServiceFourthTest, StartAbilityPublicPrechainCheck_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest StartAbilityPublicPrechainCheck_001 start");
    Want want;
    StartAbilityParams startAbilityParams(want);
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    auto ret = abilityMs_->StartAbilityPublicPrechainCheck(startAbilityParams);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest StartAbilityPublicPrechainCheck_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityPrechainInterceptor
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbilityPrechainInterceptor
 */
HWTEST_F(AbilityManagerServiceFourthTest, StartAbilityPrechainInterceptor_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest StartAbilityPrechainInterceptor_001 start");
    Want want;
    StartAbilityParams startAbilityParams(want);
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    auto ret = abilityMs_->StartAbilityPrechainInterceptor(startAbilityParams);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest StartAbilityPrechainInterceptor_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: SetReserveInfo
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService SetReserveInfo
 */
HWTEST_F(AbilityManagerServiceFourthTest, SetReserveInfo_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest SetReserveInfo_001 start");
    std::string linkString{""};
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    AbilityRequest abilityRequest;
    abilityMs_->SetReserveInfo(linkString, abilityRequest);
    EXPECT_TRUE(abilityMs_ != nullptr);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest SetReserveInfo_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: CheckExtensionCallPermission
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckExtensionCallPermission
 */
HWTEST_F(AbilityManagerServiceFourthTest, CheckExtensionCallPermission_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest CheckExtensionCallPermission_001 start");
    Want want;
    AbilityRequest abilityRequest;
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    auto ret = abilityMs_->CheckExtensionCallPermission(want, abilityRequest, 0);
    EXPECT_NE(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest CheckExtensionCallPermission_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: CheckServiceCallPermission
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckServiceCallPermission
 */
HWTEST_F(AbilityManagerServiceFourthTest, CheckServiceCallPermission_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest CheckServiceCallPermission_001 start");
    Want want;
    AbilityRequest abilityRequest;
    AppExecFwk::AbilityInfo abilityInfo;
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    auto ret = abilityMs_->CheckServiceCallPermission(abilityRequest, abilityInfo);
    EXPECT_NE(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest CheckServiceCallPermission_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: CheckBrokerCallPermission
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckBrokerCallPermission
 */
HWTEST_F(AbilityManagerServiceFourthTest, CheckBrokerCallPermission_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest CheckBrokerCallPermission_001 start");
    Want want;
    AbilityRequest abilityRequest;
    AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.visible = false;
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    auto ret = abilityMs_->CheckBrokerCallPermission(abilityRequest, abilityInfo);
    EXPECT_EQ(ret, CHECK_PERMISSION_FAILED);

    abilityInfo.visible = true;
    auto ret1 = abilityMs_->CheckBrokerCallPermission(abilityRequest, abilityInfo);
    EXPECT_EQ(ret1, CHECK_PERMISSION_FAILED);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest CheckBrokerCallPermission_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: CheckAbilityCallPermission
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckAbilityCallPermission
 */
HWTEST_F(AbilityManagerServiceFourthTest, CheckAbilityCallPermission_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest CheckAbilityCallPermission_001 start");
    AbilityRequest abilityRequest;
    AppExecFwk::AbilityInfo abilityInfo;
    uint32_t specifyTokenId{0};
    MyFlag::errCode_ = ERR_INVALID_MISSION_ID;
    MyFlag::flag_ = 0;
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    auto ret = abilityMs_->CheckAbilityCallPermission(abilityRequest, abilityInfo, specifyTokenId, false);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    MyFlag::flag_ = 1;
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest CheckAbilityCallPermission_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: CheckCallPermission
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckCallPermission
 */
HWTEST_F(AbilityManagerServiceFourthTest, CheckCallPermission_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest CheckCallPermission_001 start");
    Want want;
    AbilityRequest abilityRequest;
    AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.type = AppExecFwk::AbilityType::DATA;
    bool isForegroundToRestartApp{true};
    bool isSendDialogResult{true};
    uint32_t specifyTokenId{0};
    std::string callerBundleName{""};
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    auto ret = abilityMs_->CheckCallPermission(
        want, abilityInfo, abilityRequest, isForegroundToRestartApp, isSendDialogResult, specifyTokenId,
        callerBundleName, false);
    EXPECT_EQ(ret, ERR_WRONG_INTERFACE_CALL);

    abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    auto ret1 = abilityMs_->CheckCallPermission(
        want, abilityInfo, abilityRequest, isForegroundToRestartApp, isSendDialogResult, specifyTokenId,
        callerBundleName, false);
    EXPECT_EQ(ret1, ERR_WRONG_INTERFACE_CALL);

    MyFlag::flag_ = 0;
    abilityInfo.type = AppExecFwk::AbilityType::SERVICE;
    auto ret2 = abilityMs_->CheckCallPermission(
        want, abilityInfo, abilityRequest, isForegroundToRestartApp, isSendDialogResult, specifyTokenId,
        callerBundleName, false);
    EXPECT_EQ(ret2, ERR_INVALID_VALUE);

    MyFlag::flag_ = 1;
    abilityInfo.type = AppExecFwk::AbilityType::UNKNOWN;
    constexpr int32_t BROKER_UID = 5557;
    IPCSkeleton::SetCallingUid(BROKER_UID);
    auto ret3 = abilityMs_->CheckCallPermission(
        want, abilityInfo, abilityRequest, isForegroundToRestartApp, isSendDialogResult, specifyTokenId,
        callerBundleName, false);
    EXPECT_EQ(ret3, CHECK_PERMISSION_FAILED);

    abilityInfo.type = AppExecFwk::AbilityType::UNKNOWN;
    IPCSkeleton::SetCallingUid(0);
    auto ret4 = abilityMs_->CheckCallPermission(
        want, abilityInfo, abilityRequest, isForegroundToRestartApp, isSendDialogResult, specifyTokenId,
        callerBundleName, false);
    EXPECT_EQ(ret4, CHECK_PERMISSION_FAILED);

    auto ret5 = abilityMs_->CheckCallPermission(
        want, abilityInfo, abilityRequest, false, false, specifyTokenId, callerBundleName, false);
    EXPECT_NE(ret5, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest CheckCallPermission_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: PreStartFreeInstall
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService PreStartFreeInstall
 */
HWTEST_F(AbilityManagerServiceFourthTest, PreStartFreeInstall_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest PreStartFreeInstall_001 start");
    Want want, localWant;
    auto callerToken = MockToken(AbilityType::PAGE);
    uint32_t specifyTokenId{0};
    bool isStartAsCaller{true};
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    auto ret = abilityMs_->PreStartFreeInstall(want, callerToken, specifyTokenId, isStartAsCaller, localWant);
    EXPECT_EQ(ret, ERR_NULL_FREE_INSTALL_MANAGER);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest PreStartFreeInstall_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityByConnectManager
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbilityByConnectManager
 */
HWTEST_F(AbilityManagerServiceFourthTest, StartAbilityByConnectManager_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest StartAbilityByConnectManager_001 start");
    Want want;
    AbilityRequest abilityRequest;
    AppExecFwk::AbilityInfo abilityInfo;
    int validUserId{0};
    auto callerToken = MockToken(AbilityType::PAGE);
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    auto ret = abilityMs_->StartAbilityByConnectManager(want, abilityRequest, abilityInfo, validUserId, callerToken);
    EXPECT_EQ(ret, CONNECT_MAMAGER_NOT_FIND_BY_USERID);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest StartAbilityByConnectManager_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbility
 */
HWTEST_F(AbilityManagerServiceFourthTest, StartAbility_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest StartAbility_003 start");
    Want want;
    AbilityStartSetting abilityStartSetting;
    auto callerToken = MockToken(AbilityType::PAGE);
    int32_t userId{0};
    int requestCode{0};
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    auto ret = abilityMs_->StartAbility(want, abilityStartSetting, callerToken, userId, requestCode);
    EXPECT_EQ(ret, ERR_INVALID_CALLER);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest StartAbility_003 end");
}


/*
 * Feature: AbilityManagerService
 * Function: ImplicitStartAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ImplicitStartAbility
 */
HWTEST_F(AbilityManagerServiceFourthTest, ImplicitStartAbility_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest ImplicitStartAbility_001 start");
    Want want;
    AbilityStartSetting abilityStartSetting;
    auto callerToken = MockToken(AbilityType::PAGE);
    int32_t userId{0};
    int requestCode{0};
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    auto ret = abilityMs_->ImplicitStartAbility(want, abilityStartSetting, callerToken, userId, requestCode);
    EXPECT_EQ(ret, ERR_INVALID_CALLER);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest ImplicitStartAbility_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityAsCaller
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbilityAsCaller
 */
HWTEST_F(AbilityManagerServiceFourthTest, StartAbilityAsCaller_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest StartAbilityAsCaller_002 start");
    Want want;
    StartOptions startOptions;
    auto callerToken = MockToken(AbilityType::PAGE);
    auto asCallerSourceToken = MockToken(AbilityType::PAGE);
    int32_t userId{0};
    int requestCode{0};
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    auto ret = abilityMs_->StartAbilityAsCaller(
        want, startOptions, callerToken, asCallerSourceToken, userId, requestCode);
    EXPECT_EQ(ret, ERR_INVALID_CALLER);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest StartAbilityAsCaller_002 end");
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityForResultAsCaller
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbilityForResultAsCaller
 */
HWTEST_F(AbilityManagerServiceFourthTest, StartAbilityForResultAsCaller_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest StartAbilityForResultAsCaller_001 start");
    Want want;
    auto callerToken = MockToken(AbilityType::PAGE);
    int requestCode{0};
    int32_t userId{0};
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    auto ret = abilityMs_->StartAbilityForResultAsCaller(want, callerToken, requestCode, userId);
    EXPECT_EQ(ret, ERR_NO_INIT);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest StartAbilityForResultAsCaller_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityForResultAsCaller
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbilityForResultAsCaller
 */
HWTEST_F(AbilityManagerServiceFourthTest, StartAbilityAsCaller_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest StartAbilityAsCaller_003 start");
    Want want;
    StartOptions startOptions;
    auto callerToken = MockToken(AbilityType::PAGE);
    int requestCode{0};
    int32_t userId{0};
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    auto ret = abilityMs_->StartAbilityForResultAsCaller(want, startOptions, callerToken, requestCode, userId);
    EXPECT_EQ(ret, ERR_NO_INIT);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest StartAbilityAsCaller_003 end");
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityForResultAsCaller
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbilityForResultAsCaller
 */
HWTEST_F(AbilityManagerServiceFourthTest, StartAbilityForResultAsCaller_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest StartAbilityForResultAsCaller_004 start");
    Want want;
    auto callerToken = MockToken(AbilityType::PAGE);
    int requestCode{0};
    int32_t userId{1};
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    auto ret = abilityMs_->StartAbilityForResultAsCaller(want, callerToken, requestCode, userId);
    EXPECT_EQ(ret, ERR_NO_INIT);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest StartAbilityForResultAsCaller_004 end");
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityForResultAsCaller
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbilityForResultAsCaller
 */
HWTEST_F(AbilityManagerServiceFourthTest, StartAbilityForResultAsCaller_005, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest StartAbilityForResultAsCaller_005 start");
    Want want;
    StartOptions startOptions;
    auto callerToken = MockToken(AbilityType::PAGE);
    int requestCode{0};
    int32_t userId{1};
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    auto ret = abilityMs_->StartAbilityForResultAsCaller(want, startOptions, callerToken, requestCode, userId);
    EXPECT_EQ(ret, ERR_NO_INIT);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest StartAbilityForResultAsCaller_005 end");
}

/*
 * Feature: AbilityManagerService
 * Function: RequestDialogService
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService RequestDialogService
 */
HWTEST_F(AbilityManagerServiceFourthTest, RequestDialogService_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest RequestDialogService_001 start");
    Want want;
    auto callerToken = MockToken(AbilityType::PAGE);
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    auto ret = abilityMs_->RequestDialogService(want, callerToken);
    EXPECT_EQ(ret, ERR_INVALID_CALLER);

    want.AddFlags(want.FLAG_ABILITY_CONTINUATION);
    auto ret1 = abilityMs_->RequestDialogService(want, callerToken);
    EXPECT_EQ(ret1, ERR_INVALID_CONTINUATION_FLAG);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest RequestDialogService_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: ReportDrawnCompleted
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ReportDrawnCompleted
 */
HWTEST_F(AbilityManagerServiceFourthTest, ReportDrawnCompleted_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest ReportDrawnCompleted_001 start");
    sptr<IRemoteObject> callerToken = nullptr;
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    auto ret = abilityMs_->ReportDrawnCompleted(callerToken);
    EXPECT_EQ(ret, INNER_ERR);

    callerToken = MockToken(AbilityType::PAGE);
    auto ret1 = abilityMs_->ReportDrawnCompleted(callerToken);
    EXPECT_EQ(ret1, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest ReportDrawnCompleted_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: StartUIAbilityBySCB
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartUIAbilityBySCB
 */
HWTEST_F(AbilityManagerServiceFourthTest, StartUIAbilityBySCB_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest StartUIAbilityBySCB_001 start");
    bool isColdStart;
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    AbilityRuntime::StartParamsBySCB params;
    auto ret = abilityMs_->StartUIAbilityBySCB(nullptr, params, isColdStart);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest StartUIAbilityBySCB_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: IsDmsAlive
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService IsDmsAlive
 */
HWTEST_F(AbilityManagerServiceFourthTest, IsDmsAlive_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest IsDmsAlive_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    auto ret = abilityMs_->IsDmsAlive();
    EXPECT_EQ(ret, false);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest IsDmsAlive_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: RecordAppExitReason
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService RecordAppExitReason
 */
HWTEST_F(AbilityManagerServiceFourthTest, RecordAppExitReason_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest RecordAppExitReason_001 start");
    AAFwk::ExitReason exitReason;
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    auto ret = abilityMs_->RecordAppExitReason(exitReason);
    EXPECT_EQ(ret, ERR_NULL_OBJECT);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest RecordAppExitReason_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: OnAddSystemAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService OnAddSystemAbility
 */
HWTEST_F(AbilityManagerServiceFourthTest, OnAddSystemAbility_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest OnAddSystemAbility_001 start");
    int32_t systemAbilityId{BACKGROUND_TASK_MANAGER_SERVICE_ID};
    std::string deviceId{"deviceId"};
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->OnAddSystemAbility(systemAbilityId, deviceId);

    systemAbilityId = DISTRIBUTED_SCHED_SA_ID;
    abilityMs_->OnAddSystemAbility(systemAbilityId, deviceId);

    systemAbilityId = BUNDLE_MGR_SERVICE_SYS_ABILITY_ID;
    abilityMs_->OnAddSystemAbility(systemAbilityId, deviceId);

    systemAbilityId = MULTIMODAL_INPUT_SERVICE_ID;
    abilityMs_->OnAddSystemAbility(systemAbilityId, deviceId);

    systemAbilityId = WINDOW_MANAGER_SERVICE_ID;
    abilityMs_->OnAddSystemAbility(systemAbilityId, deviceId);
    EXPECT_TRUE(abilityMs_ != nullptr);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest OnAddSystemAbility_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: OnRemoveSystemAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService OnRemoveSystemAbility
 */
HWTEST_F(AbilityManagerServiceFourthTest, OnRemoveSystemAbility_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest OnRemoveSystemAbility_001 start");
    int32_t systemAbilityId{BACKGROUND_TASK_MANAGER_SERVICE_ID};
    std::string deviceId{"deviceId"};
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->OnRemoveSystemAbility(systemAbilityId, deviceId);

    systemAbilityId = DISTRIBUTED_SCHED_SA_ID;
    abilityMs_->OnRemoveSystemAbility(systemAbilityId, deviceId);

    systemAbilityId = BUNDLE_MGR_SERVICE_SYS_ABILITY_ID;
    abilityMs_->OnRemoveSystemAbility(systemAbilityId, deviceId);

    systemAbilityId = WINDOW_MANAGER_SERVICE_ID;
    abilityMs_->OnRemoveSystemAbility(systemAbilityId, deviceId);

    systemAbilityId = RENDER_SERVICE;
    abilityMs_->OnRemoveSystemAbility(systemAbilityId, deviceId);
    EXPECT_TRUE(abilityMs_ != nullptr);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest OnRemoveSystemAbility_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: SubscribeBackgroundTask
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService SubscribeBackgroundTask
 */
HWTEST_F(AbilityManagerServiceFourthTest, SubscribeBackgroundTask_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest SubscribeBackgroundTask_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->SubscribeBackgroundTask();
    EXPECT_TRUE(abilityMs_ != nullptr);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest SubscribeBackgroundTask_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: UnSubscribeBackgroundTask
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService UnSubscribeBackgroundTask
 */
HWTEST_F(AbilityManagerServiceFourthTest, UnSubscribeBackgroundTask_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest UnSubscribeBackgroundTask_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->SubscribeBackgroundTask();
    abilityMs_->UnSubscribeBackgroundTask();
    EXPECT_TRUE(abilityMs_ != nullptr);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest UnSubscribeBackgroundTask_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: SubscribeBundleEventCallback
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService SubscribeBundleEventCallback
 */
HWTEST_F(AbilityManagerServiceFourthTest, SubscribeBundleEventCallback_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest SubscribeBundleEventCallback_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->SubscribeBundleEventCallback();
    EXPECT_TRUE(abilityMs_ != nullptr);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest SubscribeBundleEventCallback_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: UnsubscribeBundleEventCallback
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService UnsubscribeBundleEventCallback
 */
HWTEST_F(AbilityManagerServiceFourthTest, UnsubscribeBundleEventCallback_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest UnsubscribeBundleEventCallback_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->SubscribeBundleEventCallback();
    abilityMs_->UnsubscribeBundleEventCallback();
    EXPECT_TRUE(abilityMs_ != nullptr);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest UnsubscribeBundleEventCallback_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: ReportEventToRSS
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ReportEventToRSS
 */
HWTEST_F(AbilityManagerServiceFourthTest, ReportEventToRSS_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest ReportEventToRSS_001 start");
    AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.type = AppExecFwk::AbilityType::PAGE;
    auto callerToken = MockToken(AbilityType::PAGE);
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->ReportEventToRSS(abilityInfo, callerToken);

    abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    abilityInfo.extensionAbilityType = AppExecFwk::ExtensionAbilityType::SERVICE;
    abilityMs_->ReportEventToRSS(abilityInfo, callerToken);

    abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    abilityInfo.extensionAbilityType = AppExecFwk::ExtensionAbilityType::SYSDIALOG_ATOMICSERVICEPANEL;
    abilityMs_->ReportEventToRSS(abilityInfo, callerToken);

    abilityInfo.type = AppExecFwk::AbilityType::UNKNOWN;
    abilityMs_->ReportEventToRSS(abilityInfo, callerToken);
    EXPECT_TRUE(abilityMs_ != nullptr);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest ReportEventToRSS_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: RequestModalUIExtension
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService RequestModalUIExtension
 */
HWTEST_F(AbilityManagerServiceFourthTest, RequestModalUIExtension_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest RequestModalUIExtension_001 start");
    Want want;
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    auto ret = abilityMs_->RequestModalUIExtension(want);
    EXPECT_EQ(ret, INNER_ERR);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest RequestModalUIExtension_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: ChangeAbilityVisibility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ChangeAbilityVisibility
 */
HWTEST_F(AbilityManagerServiceFourthTest, ChangeAbilityVisibility_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest ChangeAbilityVisibility_001 start");
    auto callerToken = MockToken(AbilityType::PAGE);
    bool isShow{true};
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->ChangeAbilityVisibility(callerToken, isShow);
    EXPECT_TRUE(abilityMs_ != nullptr);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest ChangeAbilityVisibility_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: ChangeUIAbilityVisibilityBySCB
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ChangeUIAbilityVisibilityBySCB
 */
HWTEST_F(AbilityManagerServiceFourthTest, ChangeUIAbilityVisibilityBySCB_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest ChangeUIAbilityVisibilityBySCB_001 start");
    bool isShow{true};
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->ChangeUIAbilityVisibilityBySCB(MockSessionInfo(0), isShow);
    EXPECT_TRUE(abilityMs_ != nullptr);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest ChangeUIAbilityVisibilityBySCB_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: SetAbilityRequestSessionInfo
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService SetAbilityRequestSessionInfo
 */
HWTEST_F(AbilityManagerServiceFourthTest, SetAbilityRequestSessionInfo_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest SetAbilityRequestSessionInfo_001 start");
    AbilityRequest abilityRequest;
    AppExecFwk::ExtensionAbilityType extensionType{AppExecFwk::ExtensionAbilityType::VPN};

    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->SetAbilityRequestSessionInfo(abilityRequest, extensionType);
    EXPECT_TRUE(abilityMs_ != nullptr);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest SetAbilityRequestSessionInfo_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: SetAbilityRequestSessionInfo
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService SetAbilityRequestSessionInfo
 */
HWTEST_F(AbilityManagerServiceFourthTest, SetAbilityRequestSessionInfo_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest SetAbilityRequestSessionInfo_002 start");
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.extensionAbilityType = AppExecFwk::ExtensionAbilityType::UI_SERVICE;
    abilityRequest.callerToken = MockToken(AbilityType::PAGE);
    AppExecFwk::ExtensionAbilityType extensionType{AppExecFwk::ExtensionAbilityType::UI_SERVICE};

    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->SetAbilityRequestSessionInfo(abilityRequest, extensionType);
    EXPECT_TRUE(abilityMs_ != nullptr);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest SetAbilityRequestSessionInfo_002 end");
}

/*
 * Feature: AbilityManagerService
 * Function: SetAbilityRequestSessionInfo
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService SetAbilityRequestSessionInfo
 */
HWTEST_F(AbilityManagerServiceFourthTest, SetAbilityRequestSessionInfo_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest SetAbilityRequestSessionInfo_003 start");
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.extensionAbilityType = AppExecFwk::ExtensionAbilityType::VPN;
    abilityRequest.callerToken = MockToken(AbilityType::PAGE);
    abilityRecord = nullptr;
    AppExecFwk::ExtensionAbilityType extensionType{AppExecFwk::ExtensionAbilityType::UI_SERVICE};

    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->SetAbilityRequestSessionInfo(abilityRequest, extensionType);
    EXPECT_TRUE(abilityMs_ != nullptr);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest SetAbilityRequestSessionInfo_003 end");
}

/*
 * Feature: AbilityManagerService
 * Function: CheckDebugAppNotInDeveloperMode
 * SubFunction: NA
 * FunctionPoints: ModalSystemDialogUtil CheckDebugAppNotInDeveloperMode
 */
HWTEST_F(AbilityManagerServiceFourthTest, CheckDebugAppNotInDeveloperMode_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest CheckDebugAppNotInDeveloperMode start");
    bool srcDeveloperMode = OHOS::system::GetBoolParameter(DEVELOPER_MODE_STATE, false);

    OHOS::system::SetBoolParameter(DEVELOPER_MODE_STATE, false);
    ApplicationInfo applicationInfo;
    applicationInfo.appProvisionType = "release";
    EXPECT_FALSE(ModalSystemDialogUtil::CheckDebugAppNotInDeveloperMode(applicationInfo));

    applicationInfo.appProvisionType = "debug";
    EXPECT_TRUE(ModalSystemDialogUtil::CheckDebugAppNotInDeveloperMode(applicationInfo));

    OHOS::system::SetBoolParameter(DEVELOPER_MODE_STATE, true);
    applicationInfo.appProvisionType = "release";
    EXPECT_FALSE(ModalSystemDialogUtil::CheckDebugAppNotInDeveloperMode(applicationInfo));

    applicationInfo.appProvisionType = "debug";
    EXPECT_FALSE(ModalSystemDialogUtil::CheckDebugAppNotInDeveloperMode(applicationInfo));

    OHOS::system::SetBoolParameter(DEVELOPER_MODE_STATE, srcDeveloperMode);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest CheckDebugAppNotInDeveloperMode end");
}

/*
 * Feature: AbilityManagerService
 * Function: CreateModalUIExtension
 * SubFunction: NA
 * FunctionPoints: ModalSystemDialogUIExtension CreateModalUIExtension
 */
HWTEST_F(AbilityManagerServiceFourthTest, ModalSystemDialogUIExtension_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest ModalSystemDialogUIExtension_001 start");
    auto modalSystemDialog = std::make_shared<ModalSystemDialogUIExtension>();
    ASSERT_NE(modalSystemDialog, nullptr);
    std::string commandStr = "test";
    auto result = modalSystemDialog->CreateModalUIExtension(commandStr);
    EXPECT_FALSE(result);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest ModalSystemDialogUIExtension_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: OnAbilityConnectDone
 * SubFunction: NA
 * FunctionPoints: ModalSystemDialogUIExtension OnAbilityConnectDone
 */
HWTEST_F(AbilityManagerServiceFourthTest, OnAbilityConnectDone_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest OnAbilityConnectDone_001 start");
    sptr<ModalSystemDialogUIExtension::DialogConnection> dialogConnection(
        new (std::nothrow) ModalSystemDialogUIExtension::DialogConnection("test"));
    ASSERT_NE(dialogConnection, nullptr);
    ElementName element("", "", "ability", "");
    dialogConnection->OnAbilityConnectDone(element, nullptr, 0);
    sptr<IRemoteObjectMocker> iRemoteObject = new IRemoteObjectMocker();
    dialogConnection->OnAbilityConnectDone(element, iRemoteObject, 0);
    EXPECT_TRUE(iRemoteObject->isSuccess_);
    dialogConnection->commandStr_ = "";
    dialogConnection->OnAbilityConnectDone(element, iRemoteObject, 0);
    EXPECT_FALSE(iRemoteObject->isSuccess_);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest OnAbilityConnectDone_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: StartUIExtensionAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartUIExtensionAbility
 */
HWTEST_F(AbilityManagerServiceFourthTest, StartUIExtensionAbilityTset_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest StartUIExtensionAbilityTset_001 start");

    auto abilityManagerService = std::make_shared<AbilityManagerService>();
    sptr<SessionInfo> extensionSessionInfoTest = new (std::nothrow) SessionInfo();
    EXPECT_NE(extensionSessionInfoTest, nullptr);
    extensionSessionInfoTest->uiExtensionUsage = UIExtensionUsage::CONSTRAINED_EMBEDDED;
    EXPECT_EQ(abilityManagerService->StartUIExtensionAbility(extensionSessionInfoTest, 1), ERR_INVALID_VALUE);

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest StartUIExtensionAbilityTset_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetUserLockedBundleList
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetUserLockedBundleList
 */
HWTEST_F(AbilityManagerServiceFourthTest, GetUserLockedBundleList_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest GetUserLockedBundleList_001 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs, nullptr);
    AbilityRuntime::UserController::GetInstance().userLockedBundleMap_.clear();
    IPCSkeleton::SetCallingUid(FOUNDATION_UID);
    auto userId = TEST_VALID_USER_ID;
    std::unordered_set<std::string> userLockedBundleList;
    auto result = abilityMs->GetUserLockedBundleList(userId, userLockedBundleList);
    EXPECT_EQ(result, ERR_OK);
    AbilityRuntime::UserController::GetInstance().userLockedBundleMap_[TEST_VALID_USER_ID].insert("com.ohos.test");
    result = abilityMs->GetUserLockedBundleList(userId, userLockedBundleList);
    EXPECT_EQ(result, ERR_OK);
    IPCSkeleton::SetCallingUid(0);
    result = abilityMs->GetUserLockedBundleList(userId, userLockedBundleList);
    EXPECT_EQ(result, CHECK_PERMISSION_FAILED);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest GetUserLockedBundleList_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: AddToUserLockedBundleList
 * SubFunction: NA
 * FunctionPoints: UserController AddToUserLockedBundleList
 */
HWTEST_F(AbilityManagerServiceFourthTest, AddToUserLockedBundleList_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest AddToUserLockedBundleList_001 start");
    std::string bundleName = "";
    int32_t userId = DEFAULT_INVAL_VALUE;
    AbilityRuntime::UserController::GetInstance().userLockedBundleMap_.clear();
    AbilityRuntime::UserController::GetInstance().AddToUserLockedBundleList(bundleName, userId);
    EXPECT_TRUE(AbilityRuntime::UserController::GetInstance().userLockedBundleMap_.empty());

    bundleName = "com.ohos.test";
    AbilityRuntime::UserController::GetInstance().AddToUserLockedBundleList(bundleName, userId);
    EXPECT_TRUE(AbilityRuntime::UserController::GetInstance().userLockedBundleMap_.empty());

    userId = U0_USER_ID;
    AbilityRuntime::UserController::GetInstance().AddToUserLockedBundleList(bundleName, userId);
    EXPECT_TRUE(AbilityRuntime::UserController::GetInstance().userLockedBundleMap_.empty());

    userId = U1_USER_ID;
    AbilityRuntime::UserController::GetInstance().AddToUserLockedBundleList(bundleName, userId);
    EXPECT_TRUE(AbilityRuntime::UserController::GetInstance().userLockedBundleMap_.empty());

    userId = TEST_VALID_USER_ID;
    AbilityRuntime::UserController::GetInstance().AddToUserLockedBundleList(bundleName, userId);
    EXPECT_TRUE(AbilityRuntime::UserController::GetInstance().userLockedBundleMap_.empty());

    AbilityRuntime::UserController::GetInstance().SetUserLockStatus(userId,
        AbilityRuntime::UserController::UserLockStatus::USER_LOCKED);
    AbilityRuntime::UserController::GetInstance().AddToUserLockedBundleList(bundleName, userId);
    EXPECT_FALSE(AbilityRuntime::UserController::GetInstance().userLockedBundleMap_.empty());
    AbilityRuntime::UserController::GetInstance().SetUserLockStatus(userId,
        AbilityRuntime::UserController::UserLockStatus::USER_LOCKED);
    AbilityRuntime::UserController::GetInstance().DeleteUserLockedBundleListByUserId(userId);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceFourthTest AddToUserLockedBundleList_001 end");
}
} // namespace AAFwk
} // namespace OHOS
