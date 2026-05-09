/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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
#include "ams_mgr_scheduler.h"
#undef private

#include "app_state_callback_host.h"
#include "hilog_tag_wrapper.h"
#include "mock_ability_token.h"
#include "mock_app_mgr_service_inner.h"
#include "mock_bundle_manager.h"
#include "mock_my_flag.h"
#include "mock_sa_call.h"
#include "application_state_observer_stub.h"
#include "param.h"

using namespace testing;
using namespace testing::ext;
using testing::_;
using testing::Return;

namespace OHOS {
namespace AppExecFwk {
class AmsMgrSchedulerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
public:
protected:
    static const std::string GetTestAppName()
    {
        return "test_app_name";
    }
    static const std::string GetTestAbilityName()
    {
        return "test_ability_name";
    }

    std::shared_ptr<MockAppMgrServiceInner> GetMockAppMgrServiceInner();
    std::shared_ptr<AAFwk::TaskHandlerWrap> GetAmsTaskHandler();

private:
    std::shared_ptr<MockAppMgrServiceInner> mockAppMgrServiceInner_;
    std::shared_ptr<AAFwk::TaskHandlerWrap> amsTaskHandler_;
};

void AmsMgrSchedulerTest::SetUpTestCase()
{}

void AmsMgrSchedulerTest::TearDownTestCase()
{}

void AmsMgrSchedulerTest::SetUp()
{}

void AmsMgrSchedulerTest::TearDown()
{
    amsTaskHandler_.reset();
    mockAppMgrServiceInner_.reset();
}

std::shared_ptr<MockAppMgrServiceInner> AmsMgrSchedulerTest::GetMockAppMgrServiceInner()
{
    if (!mockAppMgrServiceInner_) {
        mockAppMgrServiceInner_ = std::make_shared<MockAppMgrServiceInner>();
    }
    return mockAppMgrServiceInner_;
}

std::shared_ptr<AAFwk::TaskHandlerWrap> AmsMgrSchedulerTest::GetAmsTaskHandler()
{
    if (!amsTaskHandler_) {
        amsTaskHandler_ = AAFwk::TaskHandlerWrap::CreateQueueHandler("AmsMgrSchedulerTest");
    }
    return amsTaskHandler_;
}

/*
 * Feature: AMS
 * Function: AmsMgrScheduler
 * SubFunction: LoadAbility
 * FunctionPoints: Act normal
 * EnvConditions: Mobile that can run ohos test framework.
 * CaseDescription: Verify the function LoadAbility can works.
 */
HWTEST_F(AmsMgrSchedulerTest, AmsMgrScheduler_001, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "AmsMgrScheduler_001 start.");

    auto mockAppMgrServiceInner = GetMockAppMgrServiceInner();
    std::unique_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_unique<AmsMgrScheduler>(mockAppMgrServiceInner, GetAmsTaskHandler());
    ASSERT_NE(amsMgrScheduler, nullptr);

    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = GetTestAbilityName();
    std::shared_ptr<ApplicationInfo> applicationInfo = std::make_shared<ApplicationInfo>();
    applicationInfo->name = GetTestAppName();

    EXPECT_CALL(*mockAppMgrServiceInner, LoadAbility(_, _, _, _))
        .WillOnce(InvokeWithoutArgs(mockAppMgrServiceInner.get(), &MockAppMgrServiceInner::Post));
    AbilityRuntime::LoadParam loadParam;
    loadParam.token = new MockAbilityToken();
    loadParam.preToken = new MockAbilityToken();
    auto loadParamPtr = std::make_shared<AbilityRuntime::LoadParam>(loadParam);
    amsMgrScheduler->LoadAbility(abilityInfo, applicationInfo, nullptr, loadParamPtr);
    mockAppMgrServiceInner->Wait();

    TAG_LOGD(AAFwkTag::TEST, "AmsMgrScheduler_001 end.");
}

/*
 * Feature: AMS
 * Function: AmsMgrScheduler
 * SubFunction: LoadAbility
 * FunctionPoints: Check params
 * EnvConditions: Mobile that can run ohos test framework.
 * CaseDescription: Verify the function LoadAbility can check appInfo and abilityInfo.
 */
HWTEST_F(AmsMgrSchedulerTest, AmsMgrScheduler_002, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "AmsMgrScheduler_002 start.");

    auto mockAppMgrServiceInner = std::make_shared<MockAppMgrServiceInner>();
    auto taskHandler = AAFwk::TaskHandlerWrap::CreateQueueHandler("AmsMgrSchedulerTest");
    std::unique_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_unique<AmsMgrScheduler>(mockAppMgrServiceInner, taskHandler);
    ASSERT_NE(amsMgrScheduler, nullptr);

    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = GetTestAbilityName();
    std::shared_ptr<ApplicationInfo> applicationInfo = std::make_shared<ApplicationInfo>();
    applicationInfo->name = GetTestAppName();

    // check token parameter
    EXPECT_CALL(*mockAppMgrServiceInner, LoadAbility(_, _, _, _)).Times(0);
    AbilityRuntime::LoadParam loadParam;
    loadParam.token = new MockAbilityToken();
    loadParam.preToken = new MockAbilityToken();
    auto loadParamPtr = std::make_shared<AbilityRuntime::LoadParam>(loadParam);
    amsMgrScheduler->LoadAbility(nullptr, applicationInfo, nullptr, loadParamPtr);

    // check pretoken parameter
    EXPECT_CALL(*mockAppMgrServiceInner, LoadAbility(_, _, _, _)).Times(0);
    amsMgrScheduler->LoadAbility(abilityInfo, nullptr, nullptr, loadParamPtr);

    TAG_LOGD(AAFwkTag::TEST, "AmsMgrScheduler_002 end.");
}

/*
 * Feature: AMS
 * Function: AmsMgrScheduler
 * SubFunction: UpdateAbilityState
 * FunctionPoints: Act normal
 * EnvConditions: Mobile that can run ohos test framework.
 * CaseDescription: Verify the function UpdateAbilityState can works.
 */
HWTEST_F(AmsMgrSchedulerTest, AmsMgrScheduler_003, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "AmsMgrScheduler_003 start.");

    auto mockAppMgrServiceInner = GetMockAppMgrServiceInner();
    std::unique_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_unique<AmsMgrScheduler>(mockAppMgrServiceInner, GetAmsTaskHandler());
    ASSERT_NE(amsMgrScheduler, nullptr);

    sptr<IRemoteObject> token = new MockAbilityToken();
    AbilityState abilityState = AbilityState::ABILITY_STATE_CREATE;

    EXPECT_CALL(*mockAppMgrServiceInner, UpdateAbilityState(_, _, _))
        .WillOnce(InvokeWithoutArgs(mockAppMgrServiceInner.get(), &MockAppMgrServiceInner::Post));
    amsMgrScheduler->UpdateAbilityState(token, abilityState);
    mockAppMgrServiceInner->Wait();

    TAG_LOGD(AAFwkTag::TEST, "AmsMgrScheduler_003 end.");
}

/*
 * Feature: AMS
 * Function: AmsMgrScheduler
 * SubFunction: TerminateAbility
 * FunctionPoints: Act normal
 * EnvConditions: Mobile that can run ohos test framework.
 * CaseDescription: Verify the function TerminateAbility can works.
 */
HWTEST_F(AmsMgrSchedulerTest, AmsMgrScheduler_004, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "AmsMgrScheduler_004 start.");

    auto mockAppMgrServiceInner = GetMockAppMgrServiceInner();
    std::unique_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_unique<AmsMgrScheduler>(mockAppMgrServiceInner, GetAmsTaskHandler());
    ASSERT_NE(amsMgrScheduler, nullptr);
    sptr<IRemoteObject> token = new MockAbilityToken();
    bool clearMissionFlag = true;
    EXPECT_CALL(*mockAppMgrServiceInner, TerminateAbility(_, _))
        .WillOnce(InvokeWithoutArgs(mockAppMgrServiceInner.get(), &MockAppMgrServiceInner::Post));
    amsMgrScheduler->TerminateAbility(token, clearMissionFlag);
    mockAppMgrServiceInner->Wait();

    TAG_LOGD(AAFwkTag::TEST, "AmsMgrScheduler_004 end.");
}

/*
 * Feature: AMS
 * Function: AmsMgrScheduler
 * SubFunction: RegisterAppStateCallback
 * FunctionPoints: Act normal
 * EnvConditions: Mobile that can run ohos test framework.
 * CaseDescription: Verify the function RegisterAppStateCallback can works.
 */
HWTEST_F(AmsMgrSchedulerTest, AmsMgrScheduler_005, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "AmsMgrScheduler_005 start.");

    auto mockAppMgrServiceInner = GetMockAppMgrServiceInner();
    std::unique_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_unique<AmsMgrScheduler>(mockAppMgrServiceInner, GetAmsTaskHandler());
    ASSERT_NE(amsMgrScheduler, nullptr);

    sptr<AppStateCallbackHost> appStateCallbackHost = new AppStateCallbackHost();
    EXPECT_CALL(*mockAppMgrServiceInner, RegisterAppStateCallback(_))
        .WillOnce(InvokeWithoutArgs(mockAppMgrServiceInner.get(), &MockAppMgrServiceInner::Post));
    amsMgrScheduler->RegisterAppStateCallback(appStateCallbackHost);
    mockAppMgrServiceInner->Wait();

    TAG_LOGD(AAFwkTag::TEST, "AmsMgrScheduler_005 end.");
}

/*
 * Feature: AMS
 * Function: AmsMgrScheduler
 * SubFunction: IsReady
 * FunctionPoints: Check Params
 * EnvConditions: Mobile that can run ohos test framework.
 * CaseDescription: Verify the function IsReady can check params.
 */
HWTEST_F(AmsMgrSchedulerTest, AmsMgrScheduler_007, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "AmsMgrScheduler_007 start.");

    auto mockAppMgrServiceInner = GetMockAppMgrServiceInner();
    auto amsTaskHandler = GetAmsTaskHandler();

    // act normal
    std::unique_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_unique<AmsMgrScheduler>(mockAppMgrServiceInner, amsTaskHandler);
    EXPECT_EQ(true, amsMgrScheduler->IsReady());

    // check params AppMgrServiceInner
    std::unique_ptr<AmsMgrScheduler> amsMgrScheduler2 = std::make_unique<AmsMgrScheduler>(nullptr, amsTaskHandler);
    EXPECT_EQ(false, amsMgrScheduler2->IsReady());

    // check params amsTaskHandler
    std::unique_ptr<AmsMgrScheduler> amsMgrScheduler3 =
        std::make_unique<AmsMgrScheduler>(mockAppMgrServiceInner, nullptr);
    EXPECT_EQ(false, amsMgrScheduler3->IsReady());

    TAG_LOGD(AAFwkTag::TEST, "AmsMgrScheduler_007 end.");
}

/*
 * Feature: AMS
 * Function: KillApplication
 * SubFunction: IsReady
 * FunctionPoints: Check Params
 * EnvConditions: Mobile that can run ohos test framework.
 * CaseDescription: Kill apps by name
 */
HWTEST_F(AmsMgrSchedulerTest, AmsMgrScheduler_008, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "AmsMgrScheduler_008 start.");

    auto mockAppMgrServiceInner = GetMockAppMgrServiceInner();
    auto amsTaskHandler = GetAmsTaskHandler();

    EXPECT_CALL(*mockAppMgrServiceInner, KillApplication(_, _, _, _)).Times(1).WillOnce(Return(ERR_OK));

    // check params AppMgrServiceInner
    std::unique_ptr<AmsMgrScheduler> amsMgrScheduler2 = std::make_unique<AmsMgrScheduler>(nullptr, amsTaskHandler);
    EXPECT_EQ(false, amsMgrScheduler2->IsReady());

    EXPECT_EQ(ERR_INVALID_OPERATION, amsMgrScheduler2->KillApplication(GetTestAppName()));

    // check params amsTaskHandler
    std::unique_ptr<AmsMgrScheduler> amsMgrScheduler3 =
        std::make_unique<AmsMgrScheduler>(mockAppMgrServiceInner, nullptr);
    EXPECT_EQ(false, amsMgrScheduler3->IsReady());

    EXPECT_EQ(ERR_INVALID_OPERATION, amsMgrScheduler3->KillApplication(GetTestAppName()));

    // act normal
    std::unique_ptr<AmsMgrScheduler> amsMgrScheduler4 =
        std::make_unique<AmsMgrScheduler>(mockAppMgrServiceInner, amsTaskHandler);
    EXPECT_EQ(true, amsMgrScheduler4->IsReady());

    EXPECT_EQ(ERR_OK, amsMgrScheduler4->KillApplication(GetTestAppName()));

    TAG_LOGD(AAFwkTag::TEST, "AmsMgrScheduler_008 end.");
}

/*
 * Feature: AMS
 * Function: IPC
 * SubFunction: appmgr interface
 * FunctionPoints: KillApplication interface
 * CaseDescription: test IPC can transact data
 */
HWTEST_F(AmsMgrSchedulerTest, UnregisterApplicationStateObserver_001, TestSize.Level0)
{
    TAG_LOGD(AAFwkTag::TEST, "UnregisterApplicationStateObserver_001 start");
    sptr<IApplicationStateObserver> observer = new ApplicationStateObserverStub();
    auto mockAppMgrServiceInner = std::make_shared<MockAppMgrServiceInner>();
    int32_t err1 = mockAppMgrServiceInner->UnregisterApplicationStateObserver(observer);
    // unregister not exist return ERR_INVALID_VALUE
    EXPECT_EQ(1, err1);
    int32_t err2 = mockAppMgrServiceInner->UnregisterApplicationStateObserver(nullptr);
    // unregister null return ERR_INVALID_VALUE
    EXPECT_EQ(1, err2);
    TAG_LOGD(AAFwkTag::TEST, "UnregisterApplicationStateObserver_001 end");
}

/*
 * Feature: AmsMgrScheduler
 * Function: LoadAbility
 * SubFunction: NA
 * FunctionPoints: AmsMgrScheduler LoadAbility
 * EnvConditions: NA
 * CaseDescription: Verify LoadAbility
 */
HWTEST_F(AmsMgrSchedulerTest, LoadAbility_001, TestSize.Level0)
{
    auto amsMgrScheduler = std::make_shared<AmsMgrScheduler>(nullptr, nullptr);
    ASSERT_NE(amsMgrScheduler, nullptr);
    sptr<IRemoteObject> token = nullptr;
    sptr<IRemoteObject> preToken = nullptr;
    std::shared_ptr<AbilityInfo> abilityInfo = nullptr;
    std::shared_ptr<ApplicationInfo> appInfo = nullptr;
    std::shared_ptr<Want> want = nullptr;
    AbilityRuntime::LoadParam loadParam;
    loadParam.token = token;
    loadParam.preToken = preToken;
    auto loadParamPtr = std::make_shared<AbilityRuntime::LoadParam>(loadParam);
    amsMgrScheduler->LoadAbility(abilityInfo, appInfo, want, loadParamPtr);
}

/*
 * Feature: AmsMgrScheduler
 * Function: LoadAbility
 * SubFunction: NA
 * FunctionPoints: AmsMgrScheduler LoadAbility
 * EnvConditions: NA
 * CaseDescription: Verify LoadAbility
 */
HWTEST_F(AmsMgrSchedulerTest, LoadAbility_002, TestSize.Level0)
{
    auto amsMgrScheduler = std::make_shared<AmsMgrScheduler>(nullptr, nullptr);
    ASSERT_NE(amsMgrScheduler, nullptr);
    sptr<IRemoteObject> token = nullptr;
    sptr<IRemoteObject> preToken = nullptr;
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    std::shared_ptr<ApplicationInfo> appInfo = nullptr;
    std::shared_ptr<Want> want = nullptr;
    AbilityRuntime::LoadParam loadParam;
    loadParam.token = token;
    loadParam.preToken = preToken;
    auto loadParamPtr = std::make_shared<AbilityRuntime::LoadParam>(loadParam);
    amsMgrScheduler->LoadAbility(abilityInfo, appInfo, want, loadParamPtr);
}

/*
 * Feature: AmsMgrScheduler
 * Function: LoadAbility
 * SubFunction: NA
 * FunctionPoints: AmsMgrScheduler LoadAbility
 * EnvConditions: NA
 * CaseDescription: Verify LoadAbility
 */
HWTEST_F(AmsMgrSchedulerTest, LoadAbility_003, TestSize.Level0)
{
    auto amsMgrScheduler = std::make_shared<AmsMgrScheduler>(nullptr, nullptr);
    ASSERT_NE(amsMgrScheduler, nullptr);
    sptr<IRemoteObject> token = nullptr;
    sptr<IRemoteObject> preToken = nullptr;
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    std::shared_ptr<Want> want = nullptr;
    AbilityRuntime::LoadParam loadParam;
    loadParam.token = token;
    loadParam.preToken = preToken;
    auto loadParamPtr = std::make_shared<AbilityRuntime::LoadParam>(loadParam);
    amsMgrScheduler->LoadAbility(abilityInfo, appInfo, want, loadParamPtr);
}

/*
 * Feature: AmsMgrScheduler
 * Function: LoadAbility
 * SubFunction: NA
 * FunctionPoints: AmsMgrScheduler LoadAbility
 * EnvConditions: NA
 * CaseDescription: Verify LoadAbility
 */
HWTEST_F(AmsMgrSchedulerTest, LoadAbility_004, TestSize.Level0)
{
    auto mockAppMgrServiceInner = GetMockAppMgrServiceInner();
    std::unique_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_unique<AmsMgrScheduler>(mockAppMgrServiceInner, GetAmsTaskHandler());
    ASSERT_NE(amsMgrScheduler, nullptr);
    sptr<IRemoteObject> token = nullptr;
    sptr<IRemoteObject> preToken = nullptr;
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    std::shared_ptr<Want> want = nullptr;
    AbilityRuntime::LoadParam loadParam;
    loadParam.token = token;
    loadParam.preToken = preToken;
    auto loadParamPtr = std::make_shared<AbilityRuntime::LoadParam>(loadParam);
    amsMgrScheduler->LoadAbility(abilityInfo, appInfo, want, loadParamPtr);
}

/*
 * Feature: AmsMgrScheduler
 * Function: UpdateAbilityState
 * SubFunction: NA
 * FunctionPoints: AmsMgrScheduler UpdateAbilityState
 * EnvConditions: NA
 * CaseDescription: Verify UpdateAbilityState
 */
HWTEST_F(AmsMgrSchedulerTest, UpdateAbilityState_001, TestSize.Level2)
{
    auto amsMgrScheduler = std::make_unique<AmsMgrScheduler>(nullptr, nullptr);
    ASSERT_NE(amsMgrScheduler, nullptr);
    sptr<IRemoteObject> token = nullptr;
    AbilityState state = AbilityState::ABILITY_STATE_READY;
    amsMgrScheduler->UpdateAbilityState(token, state);
    amsMgrScheduler->amsMgrServiceInner_ = GetMockAppMgrServiceInner();
    amsMgrScheduler->amsHandler_ = GetAmsTaskHandler();
    amsMgrScheduler->UpdateAbilityState(token, state);
}

/*
 * Feature: AmsMgrScheduler
 * Function: UpdateExtensionState
 * SubFunction: NA
 * FunctionPoints: AmsMgrScheduler UpdateExtensionState
 * EnvConditions: NA
 * CaseDescription: Verify UpdateExtensionState
 */
HWTEST_F(AmsMgrSchedulerTest, UpdateExtensionState_001, TestSize.Level2)
{
    auto amsMgrScheduler = std::make_unique<AmsMgrScheduler>(nullptr, nullptr);
    ASSERT_NE(amsMgrScheduler, nullptr);
    sptr<IRemoteObject> token = nullptr;
    ExtensionState state = ExtensionState::EXTENSION_STATE_READY;
    amsMgrScheduler->UpdateExtensionState(token, state);
    amsMgrScheduler->amsMgrServiceInner_ = GetMockAppMgrServiceInner();
    amsMgrScheduler->amsHandler_ = GetAmsTaskHandler();
    amsMgrScheduler->UpdateExtensionState(token, state);
}

/*
 * Feature: AmsMgrScheduler
 * Function: TerminateAbility
 * SubFunction: NA
 * FunctionPoints: AmsMgrScheduler TerminateAbility
 * EnvConditions: NA
 * CaseDescription: Verify TerminateAbility
 */
HWTEST_F(AmsMgrSchedulerTest, TerminateAbility_001, TestSize.Level2)
{
    auto amsMgrScheduler = std::make_unique<AmsMgrScheduler>(nullptr, nullptr);
    ASSERT_NE(amsMgrScheduler, nullptr);
    sptr<IRemoteObject> token = nullptr;
    bool clearMissionFlag = true;
    amsMgrScheduler->TerminateAbility(token, clearMissionFlag);
    amsMgrScheduler->amsMgrServiceInner_ = GetMockAppMgrServiceInner();
    amsMgrScheduler->amsHandler_ = GetAmsTaskHandler();
    amsMgrScheduler->TerminateAbility(token, clearMissionFlag);
}

/*
 * Feature: AmsMgrScheduler
 * Function: RegisterAppStateCallback
 * SubFunction: NA
 * FunctionPoints: AmsMgrScheduler RegisterAppStateCallback
 * EnvConditions: NA
 * CaseDescription: Verify RegisterAppStateCallback
 */
HWTEST_F(AmsMgrSchedulerTest, RegisterAppStateCallback_001, TestSize.Level2)
{
    auto amsMgrScheduler = std::make_unique<AmsMgrScheduler>(nullptr, nullptr);
    ASSERT_NE(amsMgrScheduler, nullptr);
    sptr<IAppStateCallback> callback = nullptr;
    amsMgrScheduler->RegisterAppStateCallback(callback);
    amsMgrScheduler->amsMgrServiceInner_ = GetMockAppMgrServiceInner();
    amsMgrScheduler->amsHandler_ = GetAmsTaskHandler();
    amsMgrScheduler->RegisterAppStateCallback(callback);
}

/*
 * Feature: AmsMgrScheduler
 * Function: KillProcessesByUserId
 * SubFunction: NA
 * FunctionPoints: AmsMgrScheduler KillProcessesByUserId
 * EnvConditions: NA
 * CaseDescription: Verify KillProcessesByUserId
 */
HWTEST_F(AmsMgrSchedulerTest, KillProcessesByUserId_001, TestSize.Level2)
{
    auto amsMgrScheduler = std::make_unique<AmsMgrScheduler>(nullptr, nullptr);
    ASSERT_NE(amsMgrScheduler, nullptr);
    int32_t userId = 0;
    amsMgrScheduler->KillProcessesByUserId(userId, false, nullptr);
    amsMgrScheduler->KillProcessesByUserId(userId, true, nullptr);
}

/*
 * Feature: AmsMgrScheduler
 * Function: KillProcessesByUserId
 * SubFunction: NA
 * FunctionPoints: AmsMgrScheduler KillProcessesByUserId
 * EnvConditions: NA
 * CaseDescription: The caller is not system-app, can not use system-api
 */
HWTEST_F(AmsMgrSchedulerTest, KillProcessesByUserId_002, TestSize.Level2)
{
    auto amsMgrScheduler = std::make_unique<AmsMgrScheduler>(nullptr, nullptr);
    amsMgrScheduler->amsMgrServiceInner_ = GetMockAppMgrServiceInner();
    amsMgrScheduler->amsHandler_ = GetAmsTaskHandler();
    ASSERT_NE(amsMgrScheduler, nullptr);
    int32_t userId = 0;
    AAFwk::MyFlag::flag_ = 0;
    amsMgrScheduler->KillProcessesByUserId(userId, false, nullptr);
    AAFwk::MyFlag::flag_ = 1;
}

/*
 * Feature: AmsMgrScheduler
 * Function: KillProcessesByUserId
 * SubFunction: NA
 * FunctionPoints: AmsMgrScheduler KillProcessesByUserId
 * EnvConditions: NA
 * CaseDescription: SubmitTask
 */
HWTEST_F(AmsMgrSchedulerTest, KillProcessesByUserId_003, TestSize.Level2)
{
    auto amsMgrScheduler = std::make_unique<AmsMgrScheduler>(nullptr, nullptr);
    amsMgrScheduler->amsMgrServiceInner_ = GetMockAppMgrServiceInner();
    amsMgrScheduler->amsHandler_ = GetAmsTaskHandler();
    ASSERT_NE(amsMgrScheduler, nullptr);
    int32_t userId = 0;
    AAFwk::MyFlag::flag_ = 1;
    amsMgrScheduler->KillProcessesByUserId(userId, false, nullptr);
}

/*
 * Feature: AmsMgrScheduler
 * Function: KillProcessWithAccount
 * SubFunction: NA
 * FunctionPoints: AmsMgrScheduler KillProcessWithAccount
 * EnvConditions: NA
 * CaseDescription: Verify KillProcessWithAccount
 */
HWTEST_F(AmsMgrSchedulerTest, KillProcessWithAccount_001, TestSize.Level2)
{
    auto amsMgrScheduler = std::make_unique<AmsMgrScheduler>(nullptr, nullptr);
    ASSERT_NE(amsMgrScheduler, nullptr);
    std::string bundleName = "bundleName";
    int accountId = 0;
    int32_t res1 = amsMgrScheduler->KillProcessWithAccount(bundleName, accountId);
    EXPECT_EQ(res1, ERR_INVALID_OPERATION);
    amsMgrScheduler->amsMgrServiceInner_ = GetMockAppMgrServiceInner();
    amsMgrScheduler->amsHandler_ = GetAmsTaskHandler();
    int32_t res2 = amsMgrScheduler->KillProcessWithAccount(bundleName, accountId);
    EXPECT_NE(res2, ERR_INVALID_OPERATION);
}

/*
 * Feature: AmsMgrScheduler
 * Function: AbilityAttachTimeOut
 * SubFunction: NA
 * FunctionPoints: AmsMgrScheduler AbilityAttachTimeOut
 * EnvConditions: NA
 * CaseDescription: Verify AbilityAttachTimeOut
 */
HWTEST_F(AmsMgrSchedulerTest, AbilityAttachTimeOut_001, TestSize.Level2)
{
    auto amsMgrScheduler = std::make_unique<AmsMgrScheduler>(nullptr, nullptr);
    ASSERT_NE(amsMgrScheduler, nullptr);
    sptr<IRemoteObject> token = nullptr;
    amsMgrScheduler->AbilityAttachTimeOut(token);
    amsMgrScheduler->amsMgrServiceInner_ = GetMockAppMgrServiceInner();
    amsMgrScheduler->amsHandler_ = GetAmsTaskHandler();
    amsMgrScheduler->AbilityAttachTimeOut(token);
}

/*
 * Feature: AmsMgrScheduler
 * Function: PrepareTerminate
 * SubFunction: NA
 * FunctionPoints: AmsMgrScheduler PrepareTerminate
 * EnvConditions: NA
 * CaseDescription: Verify PrepareTerminate
 */
HWTEST_F(AmsMgrSchedulerTest, PrepareTerminate_001, TestSize.Level2)
{
    auto amsMgrScheduler = std::make_unique<AmsMgrScheduler>(nullptr, nullptr);
    ASSERT_NE(amsMgrScheduler, nullptr);
    sptr<IRemoteObject> token = nullptr;
    amsMgrScheduler->PrepareTerminate(token);
    amsMgrScheduler->amsMgrServiceInner_ = GetMockAppMgrServiceInner();
    amsMgrScheduler->amsHandler_ = GetAmsTaskHandler();
    amsMgrScheduler->PrepareTerminate(token);
}

/*
 * Feature: AmsMgrScheduler
 * Function: KillApplication
 * SubFunction: NA
 * FunctionPoints: AmsMgrScheduler KillApplication
 * EnvConditions: NA
 * CaseDescription: Verify KillApplication
 */
HWTEST_F(AmsMgrSchedulerTest, KillApplication_001, TestSize.Level2)
{
    auto amsMgrScheduler = std::make_unique<AmsMgrScheduler>(nullptr, nullptr);
    ASSERT_NE(amsMgrScheduler, nullptr);
    std::string bundleName = "bundleName";
    amsMgrScheduler->KillApplication(bundleName);
    amsMgrScheduler->amsMgrServiceInner_ = GetMockAppMgrServiceInner();
    amsMgrScheduler->amsHandler_ = GetAmsTaskHandler();
    amsMgrScheduler->KillApplication(bundleName);
}

/*
 * Feature: AmsMgrScheduler
 * Function: KillApplicationByUid
 * SubFunction: NA
 * FunctionPoints: AmsMgrScheduler KillApplicationByUid
 * EnvConditions: NA
 * CaseDescription: Verify KillApplicationByUid
 */
HWTEST_F(AmsMgrSchedulerTest, KillApplicationByUid_001, TestSize.Level2)
{
    auto amsMgrScheduler = std::make_unique<AmsMgrScheduler>(nullptr, nullptr);
    ASSERT_NE(amsMgrScheduler, nullptr);
    std::string bundleName = "bundleName";
    int uid = 0;
    amsMgrScheduler->KillApplicationByUid(bundleName, uid);
    amsMgrScheduler->amsMgrServiceInner_ = GetMockAppMgrServiceInner();
    amsMgrScheduler->amsHandler_ = GetAmsTaskHandler();
    amsMgrScheduler->KillApplicationByUid(bundleName, uid);
}

/*
 * Feature: AmsMgrScheduler
 * Function: KillApplicationSelf
 * SubFunction: NA
 * FunctionPoints: AmsMgrScheduler KillApplicationSelf
 * EnvConditions: NA
 * CaseDescription: Verify KillApplicationSelf
 */
HWTEST_F(AmsMgrSchedulerTest, KillApplicationSelf_001, TestSize.Level2)
{
    auto amsMgrScheduler = std::make_unique<AmsMgrScheduler>(nullptr, nullptr);
    ASSERT_NE(amsMgrScheduler, nullptr);
    amsMgrScheduler->KillApplicationSelf();
    amsMgrScheduler->amsMgrServiceInner_ = GetMockAppMgrServiceInner();
    amsMgrScheduler->amsHandler_ = GetAmsTaskHandler();
    amsMgrScheduler->KillApplicationSelf();
}

/*
 * Feature: AmsMgrScheduler
 * Function: GetRunningProcessInfoByToken
 * SubFunction: NA
 * FunctionPoints: AmsMgrScheduler GetRunningProcessInfoByToken
 * EnvConditions: NA
 * CaseDescription: Verify GetRunningProcessInfoByToken
 */
HWTEST_F(AmsMgrSchedulerTest, GetRunningProcessInfoByToken_001, TestSize.Level2)
{
    auto amsMgrScheduler = std::make_unique<AmsMgrScheduler>(nullptr, nullptr);
    ASSERT_NE(amsMgrScheduler, nullptr);
    sptr<IRemoteObject> token = nullptr;
    RunningProcessInfo info;
    amsMgrScheduler->GetRunningProcessInfoByToken(token, info);
    amsMgrScheduler->amsMgrServiceInner_ = GetMockAppMgrServiceInner();
    amsMgrScheduler->amsHandler_ = GetAmsTaskHandler();
    amsMgrScheduler->GetRunningProcessInfoByToken(token, info);
}

/*
 * Feature: AmsMgrScheduler
 * Function: IsMemorySizeSufficient
 * SubFunction: NA
 * FunctionPoints: AmsMgrScheduler IsMemorySizeSufficient
 * EnvConditions: NA
 * CaseDescription: Verify IsMemorySizeSufficient
 */
HWTEST_F(AmsMgrSchedulerTest, IsMemorySizeSufficient_001, TestSize.Level2)
{
    auto amsMgrScheduler = std::make_unique<AmsMgrScheduler>(nullptr, nullptr);
    ASSERT_NE(amsMgrScheduler, nullptr);
    bool res = amsMgrScheduler->IsMemorySizeSufficient();
    EXPECT_EQ(res, true);
    amsMgrScheduler->amsMgrServiceInner_ = GetMockAppMgrServiceInner();
    amsMgrScheduler->amsHandler_ = GetAmsTaskHandler();
    res = amsMgrScheduler->IsMemorySizeSufficient();
    EXPECT_EQ(res, true);
}

/*
 * Feature: AmsMgrScheduler
 * Function: VerifyKillProcessPermission
 * SubFunction: NA
 * FunctionPoints: AmsMgrScheduler VerifyKillProcessPermission
 * EnvConditions: NA
 * CaseDescription: Verify VerifyKillProcessPermission
 */
HWTEST_F(AmsMgrSchedulerTest, VerifyKillProcessPermission_001, TestSize.Level2)
{
    auto amsMgrScheduler = std::make_unique<AmsMgrScheduler>(nullptr, nullptr);
    ASSERT_NE(amsMgrScheduler, nullptr);
    auto res = amsMgrScheduler->VerifyKillProcessPermission("test");
    EXPECT_NE(res, ERR_OK);
    amsMgrScheduler->amsMgrServiceInner_ = GetMockAppMgrServiceInner();
    amsMgrScheduler->amsHandler_ = GetAmsTaskHandler();
    res = amsMgrScheduler->VerifyKillProcessPermission("test");
    EXPECT_EQ(res, ERR_OK);
}

/*
 * Feature: AmsMgrScheduler
 * Function: StartSpecifiedAbility
 * SubFunction: NA
 * FunctionPoints: AmsMgrScheduler StartSpecifiedAbility
 * EnvConditions: NA
 * CaseDescription: Verify StartSpecifiedAbility
 */
HWTEST_F(AmsMgrSchedulerTest, StartSpecifiedAbility_001, TestSize.Level2)
{
    auto amsMgrScheduler = std::make_unique<AmsMgrScheduler>(nullptr, nullptr);
    ASSERT_NE(amsMgrScheduler, nullptr);
    Want want;
    AbilityInfo abilityInfo;
    amsMgrScheduler->StartSpecifiedAbility(want, abilityInfo);
    amsMgrScheduler->amsMgrServiceInner_ = GetMockAppMgrServiceInner();
    amsMgrScheduler->amsHandler_ = GetAmsTaskHandler();
    amsMgrScheduler->StartSpecifiedAbility(want, abilityInfo);
}

/*
 * Feature: AmsMgrScheduler
 * Function: RegisterStartSpecifiedAbilityResponse
 * SubFunction: NA
 * FunctionPoints: AmsMgrScheduler RegisterStartSpecifiedAbilityResponse
 * EnvConditions: NA
 * CaseDescription: Verify RegisterStartSpecifiedAbilityResponse
 */
HWTEST_F(AmsMgrSchedulerTest, RegisterStartSpecifiedAbilityResponse_001, TestSize.Level2)
{
    auto amsMgrScheduler = std::make_unique<AmsMgrScheduler>(nullptr, nullptr);
    ASSERT_NE(amsMgrScheduler, nullptr);
    sptr<IStartSpecifiedAbilityResponse> response = nullptr;
    amsMgrScheduler->RegisterStartSpecifiedAbilityResponse(response);
    amsMgrScheduler->amsMgrServiceInner_ = GetMockAppMgrServiceInner();
    amsMgrScheduler->amsHandler_ = GetAmsTaskHandler();
    amsMgrScheduler->RegisterStartSpecifiedAbilityResponse(response);
}

/*
 * Feature: AmsMgrScheduler
 * Function: GetApplicationInfoByProcessID
 * SubFunction: NA
 * FunctionPoints: AmsMgrScheduler GetApplicationInfoByProcessID
 * EnvConditions: NA
 * CaseDescription: Verify GetApplicationInfoByProcessID
 */
HWTEST_F(AmsMgrSchedulerTest, GetApplicationInfoByProcessID_001, TestSize.Level2)
{
    auto amsMgrScheduler = std::make_unique<AmsMgrScheduler>(nullptr, nullptr);
    int pid = 0;
    ApplicationInfo application;
    bool debug = true;
    int res1 = amsMgrScheduler->GetApplicationInfoByProcessID(pid, application, debug);
    EXPECT_EQ(res1, ERR_INVALID_OPERATION);
    amsMgrScheduler->amsMgrServiceInner_ = GetMockAppMgrServiceInner();
    amsMgrScheduler->amsHandler_ = GetAmsTaskHandler();
    int res2 = amsMgrScheduler->GetApplicationInfoByProcessID(pid, application, debug);
    EXPECT_NE(res2, ERR_INVALID_OPERATION);
}

/*
 * Feature: AmsMgrScheduler
 * Function: NotifyAppMgrRecordExitReason
 * SubFunction: NA
 * FunctionPoints: AmsMgrScheduler NotifyAppMgrRecordExitReason
 * EnvConditions: NA
 * CaseDescription: Verify NotifyAppMgrRecordExitReason
 */
HWTEST_F(AmsMgrSchedulerTest, NotifyAppMgrRecordExitReason_001, TestSize.Level2)
{
    auto amsMgrScheduler = std::make_unique<AmsMgrScheduler>(nullptr, nullptr);
    int32_t pid = 0;
    int32_t reason = 1;
    std::string exitMsg = "JsError";
    int res1 = amsMgrScheduler->NotifyAppMgrRecordExitReason(pid, reason, exitMsg);
    EXPECT_EQ(res1, ERR_INVALID_OPERATION);

    amsMgrScheduler->amsMgrServiceInner_ = GetMockAppMgrServiceInner();
    amsMgrScheduler->amsHandler_ = GetAmsTaskHandler();
    int res2 = amsMgrScheduler->NotifyAppMgrRecordExitReason(pid, reason, exitMsg);
    EXPECT_NE(res2, ERR_INVALID_OPERATION);
}

/**
 * @tc.name: RegisterAppDebugListener_001
 * @tc.desc: Test the state of RegisterAppDebugListener
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrSchedulerTest, RegisterAppDebugListener_001, TestSize.Level2)
{
    auto amsMgrScheduler = std::make_unique<AmsMgrScheduler>(nullptr, nullptr);
    EXPECT_NE(amsMgrScheduler, nullptr);
    sptr<IAppDebugListener> listener = nullptr;
    int32_t res = amsMgrScheduler->RegisterAppDebugListener(listener);
    EXPECT_EQ(res, ERR_INVALID_OPERATION);

    amsMgrScheduler->amsMgrServiceInner_ = GetMockAppMgrServiceInner();
    amsMgrScheduler->amsHandler_ = GetAmsTaskHandler();
    res = amsMgrScheduler->RegisterAppDebugListener(listener);
    EXPECT_NE(res, ERR_INVALID_OPERATION);
}

/**
 * @tc.name: UnregisterAppDebugListener_001
 * @tc.desc: Test the state of UnregisterAppDebugListener
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrSchedulerTest, UnregisterAppDebugListener_001, TestSize.Level2)
{
    auto amsMgrScheduler = std::make_unique<AmsMgrScheduler>(nullptr, nullptr);
    EXPECT_NE(amsMgrScheduler, nullptr);
    sptr<IAppDebugListener> listener = nullptr;
    int32_t res = amsMgrScheduler->UnregisterAppDebugListener(listener);
    EXPECT_EQ(res, ERR_INVALID_OPERATION);

    amsMgrScheduler->amsMgrServiceInner_ = GetMockAppMgrServiceInner();
    amsMgrScheduler->amsHandler_ = GetAmsTaskHandler();
    res = amsMgrScheduler->UnregisterAppDebugListener(listener);
    EXPECT_NE(res, ERR_INVALID_OPERATION);
}

/**
 * @tc.name: AttachAppDebug_001
 * @tc.desc: Test the state of AttachAppDebug
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrSchedulerTest, AttachAppDebug_001, TestSize.Level2)
{
    AAFwk::IsMockSaCall::IsMockSaCallWithPermission();
    auto amsMgrScheduler = std::make_unique<AmsMgrScheduler>(nullptr, nullptr);
    EXPECT_NE(amsMgrScheduler, nullptr);
    std::string bundleName = "";
    int32_t res = amsMgrScheduler->AttachAppDebug(bundleName, false);
    EXPECT_EQ(res, ERR_INVALID_OPERATION);

    amsMgrScheduler->amsMgrServiceInner_ = GetMockAppMgrServiceInner();
    amsMgrScheduler->amsHandler_ = GetAmsTaskHandler();
    res = amsMgrScheduler->AttachAppDebug(bundleName, false);
    EXPECT_EQ(res, ERR_INVALID_OPERATION);
}

/**
 * @tc.name: DetachAppDebug_001
 * @tc.desc: Test the state of DetachAppDebug
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrSchedulerTest, DetachAppDebug_001, TestSize.Level2)
{
    auto amsMgrScheduler = std::make_unique<AmsMgrScheduler>(nullptr, nullptr);
    EXPECT_NE(amsMgrScheduler, nullptr);
    std::string bundleName = "";
    int32_t res = amsMgrScheduler->DetachAppDebug(bundleName);
    EXPECT_EQ(res, ERR_INVALID_OPERATION);

    amsMgrScheduler->amsMgrServiceInner_ = GetMockAppMgrServiceInner();
    amsMgrScheduler->amsHandler_ = GetAmsTaskHandler();
    res = amsMgrScheduler->DetachAppDebug(bundleName);
    EXPECT_EQ(res, ERR_INVALID_OPERATION);
}

/**
 * @tc.name: RegisterAbilityDebugResponse_001
 * @tc.desc: Test the state of RegisterAbilityDebugResponse
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrSchedulerTest, RegisterAbilityDebugResponse_001, TestSize.Level2)
{
    auto amsMgrScheduler = std::make_unique<AmsMgrScheduler>(nullptr, nullptr);
    EXPECT_NE(amsMgrScheduler, nullptr);
    sptr<IAbilityDebugResponse> response = nullptr;
    int32_t res = amsMgrScheduler->RegisterAbilityDebugResponse(response);
    EXPECT_EQ(res, ERR_INVALID_OPERATION);

    amsMgrScheduler->amsMgrServiceInner_ = GetMockAppMgrServiceInner();
    amsMgrScheduler->amsHandler_ = GetAmsTaskHandler();
    res = amsMgrScheduler->RegisterAbilityDebugResponse(response);
    EXPECT_NE(res, ERR_INVALID_OPERATION);
}

/*
 * Feature: AmsMgrScheduler
 * Function: KillProcessesByPids
 * SubFunction: NA
 * FunctionPoints: AmsMgrScheduler KillProcessesByPids
 * EnvConditions: NA
 * CaseDescription: not initial scheduler
 */
HWTEST_F(AmsMgrSchedulerTest, KillProcessesByPids_001, TestSize.Level2)
{
    auto amsMgrScheduler = std::make_unique<AmsMgrScheduler>(nullptr, nullptr);
    ASSERT_NE(amsMgrScheduler, nullptr);
    int32_t userId = 0;
    std::vector<int32_t> pids = {1};
    amsMgrScheduler->KillProcessesByPids(pids);
}

/*
 * Feature: AmsMgrScheduler
 * Function: KillProcessesByPids
 * SubFunction: NA
 * FunctionPoints: AmsMgrScheduler KillProcessesByPids
 * EnvConditions: NA
 * CaseDescription: SubmitTask
 */
HWTEST_F(AmsMgrSchedulerTest, KillProcessesByPids_002, TestSize.Level2)
{
    auto amsMgrScheduler = std::make_unique<AmsMgrScheduler>(nullptr, nullptr);
    amsMgrScheduler->amsMgrServiceInner_ = GetMockAppMgrServiceInner();
    amsMgrScheduler->amsHandler_ = GetAmsTaskHandler();
    ASSERT_NE(amsMgrScheduler, nullptr);
    int32_t userId = 0;
    std::vector<int32_t> pids = {1};
    amsMgrScheduler->KillProcessesByPids(pids);
}

/*
 * Feature: AmsMgrScheduler
 * Function: AttachPidToParent
 * SubFunction: NA
 * FunctionPoints: AmsMgrScheduler AttachPidToParent
 * EnvConditions: NA
 * CaseDescription: not initial scheduler
 */
HWTEST_F(AmsMgrSchedulerTest, AttachPidToParent_001, TestSize.Level2)
{
    auto amsMgrScheduler = std::make_unique<AmsMgrScheduler>(nullptr, nullptr);
    ASSERT_NE(amsMgrScheduler, nullptr);
    const sptr<IRemoteObject> token;
    const sptr<IRemoteObject> callerToken;
    amsMgrScheduler->AttachPidToParent(token, callerToken);
}

/*
 * Feature: AmsMgrScheduler
 * Function: AttachPidToParent
 * SubFunction: NA
 * FunctionPoints: AmsMgrScheduler AttachPidToParent
 * EnvConditions: NA
 * CaseDescription: SubmitTask
 */
HWTEST_F(AmsMgrSchedulerTest, AttachPidToParent_002, TestSize.Level2)
{
    auto amsMgrScheduler = std::make_unique<AmsMgrScheduler>(nullptr, nullptr);
    amsMgrScheduler->amsMgrServiceInner_ = GetMockAppMgrServiceInner();
    amsMgrScheduler->amsHandler_ = GetAmsTaskHandler();
    ASSERT_NE(amsMgrScheduler, nullptr);
    const sptr<IRemoteObject> token;
    const sptr<IRemoteObject> callerToken;
    amsMgrScheduler->AttachPidToParent(token, callerToken);
}

/*
 * Feature: AmsMgrScheduler
 * Function: UpdateApplicationInfoInstalled
 * SubFunction: NA
 * FunctionPoints: AmsMgrScheduler UpdateApplicationInfoInstalled
 * EnvConditions: NA
 * CaseDescription: not initial scheduler
 */
HWTEST_F(AmsMgrSchedulerTest, UpdateApplicationInfoInstalled_001, TestSize.Level2)
{
    auto amsMgrScheduler = std::make_unique<AmsMgrScheduler>(nullptr, nullptr);
    ASSERT_NE(amsMgrScheduler, nullptr);
    const std::string bundleName = "";
    const int uid = 0;
    const std::string moduleName = "";
    auto iret = amsMgrScheduler->UpdateApplicationInfoInstalled(bundleName, uid, moduleName, false);
    ASSERT_EQ(iret, 38);
}

/*
 * Feature: AmsMgrScheduler
 * Function: UpdateApplicationInfoInstalled
 * SubFunction: NA
 * FunctionPoints: AmsMgrScheduler UpdateApplicationInfoInstalled
 * EnvConditions: NA
 * CaseDescription: UpdateApplicationInfoInstalled
 */
HWTEST_F(AmsMgrSchedulerTest, UpdateApplicationInfoInstalled_002, TestSize.Level2)
{
    auto amsMgrScheduler = std::make_unique<AmsMgrScheduler>(nullptr, nullptr);
    amsMgrScheduler->amsMgrServiceInner_ = GetMockAppMgrServiceInner();
    amsMgrScheduler->amsHandler_ = GetAmsTaskHandler();
    ASSERT_NE(amsMgrScheduler, nullptr);
    const std::string bundleName = "";
    const int uid = 0;
    const std::string moduleName = "";
    auto iret = amsMgrScheduler->UpdateApplicationInfoInstalled(bundleName, uid, moduleName, false);
    ASSERT_EQ(iret, 0);
}

/*
 * Feature: AmsMgrScheduler
 * Function: SetAbilityForegroundingFlagToAppRecord
 * SubFunction: NA
 * FunctionPoints: AmsMgrScheduler SetAbilityForegroundingFlagToAppRecord
 * EnvConditions: NA
 * CaseDescription: not initial scheduler
 */
HWTEST_F(AmsMgrSchedulerTest, SetAbilityForegroundingFlagToAppRecord_001, TestSize.Level2)
{
    auto amsMgrScheduler = std::make_unique<AmsMgrScheduler>(nullptr, nullptr);
    ASSERT_NE(amsMgrScheduler, nullptr);
    const pid_t pid = 1;
    amsMgrScheduler->SetAbilityForegroundingFlagToAppRecord(pid);
}

/*
 * Feature: AmsMgrScheduler
 * Function: SetAbilityForegroundingFlagToAppRecord
 * SubFunction: NA
 * FunctionPoints: AmsMgrScheduler SetAbilityForegroundingFlagToAppRecord
 * EnvConditions: NA
 * CaseDescription: SetAbilityForegroundingFlagToAppRecord
 */
HWTEST_F(AmsMgrSchedulerTest, SetAbilityForegroundingFlagToAppRecord_002, TestSize.Level2)
{
    auto amsMgrScheduler = std::make_unique<AmsMgrScheduler>(nullptr, nullptr);
    amsMgrScheduler->amsMgrServiceInner_ = GetMockAppMgrServiceInner();
    amsMgrScheduler->amsHandler_ = GetAmsTaskHandler();
    ASSERT_NE(amsMgrScheduler, nullptr);
    const pid_t pid = 1;
    amsMgrScheduler->SetAbilityForegroundingFlagToAppRecord(pid);
}

/*
 * Feature: AmsMgrScheduler
 * Function: StartSpecifiedProcess
 * SubFunction: NA
 * FunctionPoints: AmsMgrScheduler StartSpecifiedProcess
 * EnvConditions: NA
 * CaseDescription: not initial scheduler
 */
HWTEST_F(AmsMgrSchedulerTest, StartSpecifiedProcess_001, TestSize.Level2)
{
    auto amsMgrScheduler = std::make_unique<AmsMgrScheduler>(nullptr, nullptr);
    ASSERT_NE(amsMgrScheduler, nullptr);
    const AAFwk::Want want;
    const AppExecFwk::AbilityInfo abilityInfo;
    int32_t requestId = 1;
    amsMgrScheduler->StartSpecifiedProcess(want, abilityInfo, requestId);
}

/*
 * Feature: AmsMgrScheduler
 * Function: StartSpecifiedProcess
 * SubFunction: NA
 * FunctionPoints: AmsMgrScheduler StartSpecifiedProcess
 * EnvConditions: NA
 * CaseDescription: StartSpecifiedProcess
 */
HWTEST_F(AmsMgrSchedulerTest, StartSpecifiedProcess_002, TestSize.Level2)
{
    auto amsMgrScheduler = std::make_unique<AmsMgrScheduler>(nullptr, nullptr);
    amsMgrScheduler->amsMgrServiceInner_ = GetMockAppMgrServiceInner();
    amsMgrScheduler->amsHandler_ = GetAmsTaskHandler();
    ASSERT_NE(amsMgrScheduler, nullptr);
    const AAFwk::Want want;
    const AppExecFwk::AbilityInfo abilityInfo;
    int32_t requestId = 1;
    amsMgrScheduler->StartSpecifiedProcess(want, abilityInfo, requestId);
}

/*
 * Feature: AmsMgrScheduler
 * Function: GetBundleNameByPid
 * SubFunction: NA
 * FunctionPoints: AmsMgrScheduler GetBundleNameByPid
 * EnvConditions: NA
 * CaseDescription: not initial scheduler
 */
HWTEST_F(AmsMgrSchedulerTest, GetBundleNameByPid_001, TestSize.Level2)
{
    auto amsMgrScheduler = std::make_unique<AmsMgrScheduler>(nullptr, nullptr);
    ASSERT_NE(amsMgrScheduler, nullptr);
    const int pid = 1;
    std::string bundleName;
    int32_t uid = 1;
    auto iret = amsMgrScheduler->GetBundleNameByPid(pid, bundleName, uid);
    ASSERT_EQ(iret, 38);
}

/*
 * Feature: AmsMgrScheduler
 * Function: GetBundleNameByPid
 * SubFunction: NA
 * FunctionPoints: AmsMgrScheduler GetBundleNameByPid
 * EnvConditions: NA
 * CaseDescription: GetBundleNameByPid
 */
HWTEST_F(AmsMgrSchedulerTest, GetBundleNameByPid_002, TestSize.Level2)
{
    auto amsMgrScheduler = std::make_unique<AmsMgrScheduler>(nullptr, nullptr);
    amsMgrScheduler->amsMgrServiceInner_ = GetMockAppMgrServiceInner();
    amsMgrScheduler->amsHandler_ = GetAmsTaskHandler();
    ASSERT_NE(amsMgrScheduler, nullptr);
    const int pid = 1;
    std::string bundleName;
    int32_t uid = 1;
    auto iret = amsMgrScheduler->GetBundleNameByPid(pid, bundleName, uid);
    ASSERT_EQ(iret, 38);
}

/*
 * Feature: AmsMgrScheduler
 * Function: SetAppWaitingDebug
 * SubFunction: NA
 * FunctionPoints: AmsMgrScheduler SetAppWaitingDebug
 * EnvConditions: NA
 * CaseDescription: not initial scheduler
 */
HWTEST_F(AmsMgrSchedulerTest, SetAppWaitingDebug_001, TestSize.Level2)
{
    auto amsMgrScheduler = std::make_unique<AmsMgrScheduler>(nullptr, nullptr);
    ASSERT_NE(amsMgrScheduler, nullptr);
    const std::string bundleName;
    bool isPersist = true;
    auto iret = amsMgrScheduler->SetAppWaitingDebug(bundleName, isPersist);
    ASSERT_EQ(iret, 38);
}

/*
 * Feature: AmsMgrScheduler
 * Function: SetAppWaitingDebug
 * SubFunction: NA
 * FunctionPoints: AmsMgrScheduler SetAppWaitingDebug
 * EnvConditions: NA
 * CaseDescription: SetAppWaitingDebug
 */
HWTEST_F(AmsMgrSchedulerTest, SetAppWaitingDebug_002, TestSize.Level2)
{
    auto amsMgrScheduler = std::make_unique<AmsMgrScheduler>(nullptr, nullptr);
    amsMgrScheduler->amsMgrServiceInner_ = GetMockAppMgrServiceInner();
    amsMgrScheduler->amsHandler_ = GetAmsTaskHandler();
    ASSERT_NE(amsMgrScheduler, nullptr);
    const std::string bundleName;
    bool isPersist = true;
    auto iret = amsMgrScheduler->SetAppWaitingDebug(bundleName, isPersist);
    ASSERT_EQ(iret, ERR_PERMISSION_DENIED);
}

/*
 * Feature: AmsMgrScheduler
 * Function: CancelAppWaitingDebug
 * SubFunction: NA
 * FunctionPoints: AmsMgrScheduler CancelAppWaitingDebug
 * EnvConditions: NA
 * CaseDescription: not initial scheduler
 */
HWTEST_F(AmsMgrSchedulerTest, CancelAppWaitingDebug_001, TestSize.Level2)
{
    auto amsMgrScheduler = std::make_unique<AmsMgrScheduler>(nullptr, nullptr);
    ASSERT_NE(amsMgrScheduler, nullptr);
    amsMgrScheduler->CancelAppWaitingDebug();
}

/*
 * Feature: AmsMgrScheduler
 * Function: CancelAppWaitingDebug
 * SubFunction: NA
 * FunctionPoints: AmsMgrScheduler CancelAppWaitingDebug
 * EnvConditions: NA
 * CaseDescription: CancelAppWaitingDebug
 */
HWTEST_F(AmsMgrSchedulerTest, CancelAppWaitingDebug_002, TestSize.Level2)
{
    auto amsMgrScheduler = std::make_unique<AmsMgrScheduler>(nullptr, nullptr);
    amsMgrScheduler->amsMgrServiceInner_ = GetMockAppMgrServiceInner();
    amsMgrScheduler->amsHandler_ = GetAmsTaskHandler();
    ASSERT_NE(amsMgrScheduler, nullptr);
    amsMgrScheduler->CancelAppWaitingDebug();
}

/*
 * Feature: AmsMgrScheduler
 * Function: GetWaitingDebugApp
 * SubFunction: NA
 * FunctionPoints: AmsMgrScheduler GetWaitingDebugApp
 * EnvConditions: NA
 * CaseDescription: not initial scheduler
 */
HWTEST_F(AmsMgrSchedulerTest, GetWaitingDebugApp_001, TestSize.Level2)
{
    auto amsMgrScheduler = std::make_unique<AmsMgrScheduler>(nullptr, nullptr);
    ASSERT_NE(amsMgrScheduler, nullptr);
    std::vector<std::string> debugInfoList;
    auto iret = amsMgrScheduler->GetWaitingDebugApp(debugInfoList);
    ASSERT_EQ(iret, 38);
}

/*
 * Feature: AmsMgrScheduler
 * Function: GetWaitingDebugApp
 * SubFunction: NA
 * FunctionPoints: AmsMgrScheduler GetWaitingDebugApp
 * EnvConditions: NA
 * CaseDescription: not initial scheduler
 */
HWTEST_F(AmsMgrSchedulerTest, GetWaitingDebugApp_002, TestSize.Level2)
{
    auto amsMgrScheduler = std::make_unique<AmsMgrScheduler>(nullptr, nullptr);
    amsMgrScheduler->amsMgrServiceInner_ = GetMockAppMgrServiceInner();
    amsMgrScheduler->amsHandler_ = GetAmsTaskHandler();
    ASSERT_NE(amsMgrScheduler, nullptr);
    std::vector<std::string> debugInfoList;
    auto iret = amsMgrScheduler->GetWaitingDebugApp(debugInfoList);
    ASSERT_EQ(iret, ERR_PERMISSION_DENIED);
}

/*
 * Feature: AmsMgrScheduler
 * Function: IsWaitingDebugApp
 * SubFunction: NA
 * FunctionPoints: AmsMgrScheduler IsWaitingDebugApp
 * EnvConditions: NA
 * CaseDescription: not initial scheduler
 */
HWTEST_F(AmsMgrSchedulerTest, IsWaitingDebugApp_001, TestSize.Level2)
{
    auto amsMgrScheduler = std::make_unique<AmsMgrScheduler>(nullptr, nullptr);
    ASSERT_NE(amsMgrScheduler, nullptr);
    const std::string bundleName;
    auto iret = amsMgrScheduler->IsWaitingDebugApp(bundleName);
    ASSERT_EQ(iret, false);
}

/*
 * Feature: AmsMgrScheduler
 * Function: IsWaitingDebugApp
 * SubFunction: NA
 * FunctionPoints: AmsMgrScheduler IsWaitingDebugApp
 * EnvConditions: NA
 * CaseDescription: IsWaitingDebugApp
 */
HWTEST_F(AmsMgrSchedulerTest, IsWaitingDebugApp_002, TestSize.Level2)
{
    auto amsMgrScheduler = std::make_unique<AmsMgrScheduler>(nullptr, nullptr);
    amsMgrScheduler->amsMgrServiceInner_ = GetMockAppMgrServiceInner();
    amsMgrScheduler->amsHandler_ = GetAmsTaskHandler();
    ASSERT_NE(amsMgrScheduler, nullptr);
    const std::string bundleName;
    auto iret = amsMgrScheduler->IsWaitingDebugApp(bundleName);
    ASSERT_EQ(iret, false);
}

/*
 * Feature: AmsMgrScheduler
 * Function: ClearNonPersistWaitingDebugFlag
 * SubFunction: NA
 * FunctionPoints: AmsMgrScheduler ClearNonPersistWaitingDebugFlag
 * EnvConditions: NA
 * CaseDescription: not initial scheduler
 */
HWTEST_F(AmsMgrSchedulerTest, ClearNonPersistWaitingDebugFlag_001, TestSize.Level2)
{
    auto amsMgrScheduler = std::make_unique<AmsMgrScheduler>(nullptr, nullptr);
    ASSERT_NE(amsMgrScheduler, nullptr);
    amsMgrScheduler->ClearNonPersistWaitingDebugFlag();
}

/*
 * Feature: AmsMgrScheduler
 * Function: ClearNonPersistWaitingDebugFlag
 * SubFunction: NA
 * FunctionPoints: AmsMgrScheduler ClearNonPersistWaitingDebugFlag
 * EnvConditions: NA
 * CaseDescription: ClearNonPersistWaitingDebugFlag
 */
HWTEST_F(AmsMgrSchedulerTest, ClearNonPersistWaitingDebugFlag_002, TestSize.Level2)
{
    auto amsMgrScheduler = std::make_unique<AmsMgrScheduler>(nullptr, nullptr);
    amsMgrScheduler->amsMgrServiceInner_ = GetMockAppMgrServiceInner();
    amsMgrScheduler->amsHandler_ = GetAmsTaskHandler();
    ASSERT_NE(amsMgrScheduler, nullptr);
    amsMgrScheduler->ClearNonPersistWaitingDebugFlag();
}

/*
 * Feature: AmsMgrScheduler
 * Function: IsAttachDebug
 * SubFunction: NA
 * FunctionPoints: AmsMgrScheduler IsAttachDebug
 * EnvConditions: NA
 * CaseDescription: not initial scheduler
 */
HWTEST_F(AmsMgrSchedulerTest, IsAttachDebug_001, TestSize.Level2)
{
    auto amsMgrScheduler = std::make_unique<AmsMgrScheduler>(nullptr, nullptr);
    ASSERT_NE(amsMgrScheduler, nullptr);
    const std::string bundleName;
    auto iret = amsMgrScheduler->IsAttachDebug(bundleName);
    ASSERT_EQ(iret, false);
}

/*
 * Feature: AmsMgrScheduler
 * Function: IsAttachDebug
 * SubFunction: NA
 * FunctionPoints: AmsMgrScheduler IsAttachDebug
 * EnvConditions: NA
 * CaseDescription: IsAttachDebug
 */
HWTEST_F(AmsMgrSchedulerTest, IsAttachDebug_002, TestSize.Level2)
{
    auto amsMgrScheduler = std::make_unique<AmsMgrScheduler>(nullptr, nullptr);
    amsMgrScheduler->amsMgrServiceInner_ = GetMockAppMgrServiceInner();
    amsMgrScheduler->amsHandler_ = GetAmsTaskHandler();
    ASSERT_NE(amsMgrScheduler, nullptr);
    const std::string bundleName;
    auto iret = amsMgrScheduler->IsAttachDebug(bundleName);
    ASSERT_EQ(iret, false);
}

/*
 * Feature: AmsMgrScheduler
 * Function: ClearProcessByToken
 * SubFunction: NA
 * FunctionPoints: AmsMgrScheduler ClearProcessByToken
 * EnvConditions: NA
 * CaseDescription: not initial scheduler
 */
HWTEST_F(AmsMgrSchedulerTest, ClearProcessByToken_001, TestSize.Level2)
{
    auto amsMgrScheduler = std::make_unique<AmsMgrScheduler>(nullptr, nullptr);
    ASSERT_NE(amsMgrScheduler, nullptr);
    sptr<IRemoteObject> token;
    amsMgrScheduler->ClearProcessByToken(token);
}

/*
 * Feature: AmsMgrScheduler
 * Function: ClearProcessByToken
 * SubFunction: NA
 * FunctionPoints: AmsMgrScheduler ClearProcessByToken
 * EnvConditions: NA
 * CaseDescription: caller is not foundation
 */
HWTEST_F(AmsMgrSchedulerTest, ClearProcessByToken_002, TestSize.Level2)
{
    auto amsMgrScheduler = std::make_unique<AmsMgrScheduler>(nullptr, nullptr);
    amsMgrScheduler->amsMgrServiceInner_ = GetMockAppMgrServiceInner();
    amsMgrScheduler->amsHandler_ = GetAmsTaskHandler();
    ASSERT_NE(amsMgrScheduler, nullptr);
    sptr<IRemoteObject> token;
    amsMgrScheduler->ClearProcessByToken(token);
}

/*
 * Feature: AmsMgrScheduler
 * Function: BlockProcessCacheByPids
 * SubFunction: NA
 * FunctionPoints: AmsMgrScheduler BlockProcessCacheByPids
 * EnvConditions: NA
 * CaseDescription: not initial scheduler
 */
HWTEST_F(AmsMgrSchedulerTest, BlockProcessCacheByPids_001, TestSize.Level2)
{
    auto amsMgrScheduler = std::make_unique<AmsMgrScheduler>(nullptr, nullptr);
    ASSERT_NE(amsMgrScheduler, nullptr);
    int32_t userId = 0;
    std::vector<int32_t> pids = {1};
    amsMgrScheduler->BlockProcessCacheByPids(pids);
}

/*
 * Feature: AmsMgrScheduler
 * Function: BlockProcessCacheByPids
 * SubFunction: NA
 * FunctionPoints: AmsMgrScheduler BlockProcessCacheByPids
 * EnvConditions: NA
 * CaseDescription: SubmitTask
 */
HWTEST_F(AmsMgrSchedulerTest, BlockProcessCacheByPids_002, TestSize.Level2)
{
    auto amsMgrScheduler = std::make_unique<AmsMgrScheduler>(nullptr, nullptr);
    amsMgrScheduler->amsMgrServiceInner_ = GetMockAppMgrServiceInner();
    amsMgrScheduler->amsHandler_ = GetAmsTaskHandler();
    ASSERT_NE(amsMgrScheduler, nullptr);
    int32_t userId = 0;
    std::vector<int32_t> pids = {1};
    amsMgrScheduler->BlockProcessCacheByPids(pids);
}

/*
 * Feature: AmsMgrScheduler
 * Function: AttachedToStatusBar
 * SubFunction: NA
 * FunctionPoints: AmsMgrScheduler AttachedToStatusBar
 * EnvConditions: NA
 * CaseDescription: not initial scheduler
 */
HWTEST_F(AmsMgrSchedulerTest, AttachedToStatusBar_001, TestSize.Level2)
{
    auto amsMgrScheduler = std::make_unique<AmsMgrScheduler>(nullptr, nullptr);
    ASSERT_NE(amsMgrScheduler, nullptr);
    sptr<IRemoteObject> token;
    amsMgrScheduler->AttachedToStatusBar(token);
}

/*
 * Feature: AmsMgrScheduler
 * Function: AttachedToStatusBar
 * SubFunction: NA
 * FunctionPoints: AmsMgrScheduler AttachedToStatusBar
 * EnvConditions: NA
 * CaseDescription: SubmitTask
 */
HWTEST_F(AmsMgrSchedulerTest, AttachedToStatusBar_002, TestSize.Level2)
{
    auto amsMgrScheduler = std::make_unique<AmsMgrScheduler>(nullptr, nullptr);
    amsMgrScheduler->amsMgrServiceInner_ = GetMockAppMgrServiceInner();
    amsMgrScheduler->amsHandler_ = GetAmsTaskHandler();
    ASSERT_NE(amsMgrScheduler, nullptr);
    sptr<IRemoteObject> token;
    amsMgrScheduler->AttachedToStatusBar(token);
}
}  // namespace AppExecFwk
}  // namespace OHOS
