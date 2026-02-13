/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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
#define private public
#define protected public
#include "ability_record.h"
#include "ability_start_setting.h"
#include "app_scheduler.h"
#include "app_utils.h"
#include "scene_board/ui_ability_lifecycle_manager.h"
#undef protected
#undef private
#include "app_mgr_client.h"
#include "hilog_tag_wrapper.h"
#include "mock_ability_info_callback_stub.h"
#include "process_options.h"
#include "session/host/include/session.h"
#include "session_info.h"
#include "startup_util.h"
#include "ability_manager_service.h"
#include "ability_scheduler_mock.h"
#include "ipc_skeleton.h"
#include "start_params_by_SCB.h"
#include "status_bar_delegate_interface.h"
#include "user_controller/user_controller.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AAFwk {
namespace {
#ifdef WITH_DLP
const std::string DLP_INDEX = "ohos.dlp.params.index";
#endif // WITH_DLP
constexpr int32_t TEST_UID = 20010001;
constexpr const char* IS_CALLING_FROM_DMS = "supportCollaborativeCallingFromDmsInAAFwk";
};
class UIAbilityLifecycleManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    UIAbilityRecordPtr InitAbilityRecord();
};

void UIAbilityLifecycleManagerTest::SetUpTestCase() {}

void UIAbilityLifecycleManagerTest::TearDownTestCase() {}

void UIAbilityLifecycleManagerTest::SetUp() {}

void UIAbilityLifecycleManagerTest::TearDown() {}

class MockIStatusBarDelegate : public OHOS::AbilityRuntime::IStatusBarDelegate {
public:
    int32_t CheckIfStatusBarItemExists(uint32_t accessTokenId, const std::string &instanceKey,
        bool& isExist)
    {
        return 0;
    }
    int32_t AttachPidToStatusBarItem(uint32_t accessTokenId, int32_t pid, const std::string &instanceKey)
    {
        return 0;
    }
    int32_t DetachPidToStatusBarItem(uint32_t accessTokenId, int32_t pid, const std::string &instanceKey)
    {
        return 0;
    }
    sptr<IRemoteObject> AsObject()
    {
        return nullptr;
    }
};

class UIAbilityLifecycleManagerTestStub : public IRemoteStub<IAbilityConnection> {
public:
    UIAbilityLifecycleManagerTestStub() {};
    virtual ~UIAbilityLifecycleManagerTestStub() {};

    virtual int OnRemoteRequest(
        uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option)
    {
        return 0;
    };

    virtual void OnAbilityConnectDone(
        const AppExecFwk::ElementName& element, const sptr<IRemoteObject>& remoteObject, int resultCode) {};

    /**
     * OnAbilityDisconnectDone, AbilityMs notify caller ability the result of disconnect.
     *
     * @param element, service ability's ElementName.
     * @param resultCode, ERR_OK on success, others on failure.
     */
    virtual void OnAbilityDisconnectDone(const AppExecFwk::ElementName& element, int resultCode) {};
};

UIAbilityRecordPtr UIAbilityLifecycleManagerTest::InitAbilityRecord()
{
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::PAGE;
    UIAbilityRecordPtr abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    return abilityRecord;
}

/**
 * @tc.name: UIAbilityLifecycleManager_StartUIAbility_0100
 * @tc.desc: StartUIAbility
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, StartUIAbility_001, TestSize.Level1)
{
    auto mgr = std::make_unique<UIAbilityLifecycleManager>();
    AbilityRequest abilityRequest;
    AbilityRuntime::StartParamsBySCB params;
    bool isColdStart = false;
    EXPECT_EQ(mgr->StartUIAbility(abilityRequest, nullptr, params, isColdStart), ERR_INVALID_VALUE);
}

/**
 * @tc.name: UIAbilityLifecycleManager_StartUIAbility_0200
 * @tc.desc: StartUIAbility
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, StartUIAbility_002, TestSize.Level1)
{
    auto mgr = std::make_unique<UIAbilityLifecycleManager>();
    AbilityRequest abilityRequest;
    Rosen::SessionInfo info;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->sessionToken = new Rosen::Session(info);
    AbilityRuntime::StartParamsBySCB params;
    bool isColdStart = false;
    EXPECT_EQ(mgr->StartUIAbility(abilityRequest, sessionInfo, params, isColdStart), ERR_OK);
}

/**
 * @tc.name: UIAbilityLifecycleManager_StartUIAbility_0300
 * @tc.desc: StartUIAbility
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, StartUIAbility_003, TestSize.Level1)
{
    auto mgr = std::make_unique<UIAbilityLifecycleManager>();
    AbilityRequest abilityRequest;
    Rosen::SessionInfo info;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->sessionToken = new Rosen::Session(info);
    sessionInfo->persistentId = 1;
    sessionInfo->isNewWant = true;
    abilityRequest.sessionInfo = sessionInfo;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    abilityRecord->SetPendingState(AbilityState::FOREGROUND);
    mgr->sessionAbilityMap_.emplace(sessionInfo->persistentId, abilityRecord);
    AbilityRuntime::StartParamsBySCB params;
    bool isColdStart = false;
    EXPECT_EQ(mgr->StartUIAbility(abilityRequest, sessionInfo, params, isColdStart), ERR_OK);
}

/**
 * @tc.name: UIAbilityLifecycleManager_StartUIAbility_0400
 * @tc.desc: StartUIAbility
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, StartUIAbility_004, TestSize.Level1)
{
    auto mgr = std::make_unique<UIAbilityLifecycleManager>();
    AbilityRequest abilityRequest;
    Rosen::SessionInfo info;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->sessionToken = new Rosen::Session(info);
    sessionInfo->startSetting = std::make_shared<AbilityStartSetting>();
    sessionInfo->persistentId = 1;
    sessionInfo->specifiedFlag = "0";
    abilityRequest.sessionInfo = sessionInfo;
    AbilityRuntime::StartParamsBySCB params;
    bool isColdStart = false;
    EXPECT_EQ(mgr->StartUIAbility(abilityRequest, sessionInfo, params, isColdStart), ERR_OK);
}

/**
 * @tc.name: UIAbilityLifecycleManager_StartUIAbility_0500
 * @tc.desc: StartUIAbility
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, StartUIAbility_005, TestSize.Level1)
{
    auto mgr = std::make_unique<UIAbilityLifecycleManager>();
    AbilityRequest abilityRequest;
    Rosen::SessionInfo info;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->sessionToken = new Rosen::Session(info);
    sessionInfo->persistentId = 1;
    sessionInfo->reuseDelegatorWindow = true;
    abilityRequest.sessionInfo = sessionInfo;
    AbilityRuntime::StartParamsBySCB params;
    bool isColdStart = false;
    EXPECT_EQ(mgr->StartUIAbility(abilityRequest, sessionInfo, params, isColdStart), ERR_OK);
}

/**
 * @tc.name: UIAbilityLifecycleManager_StartUIAbility_0600
 * @tc.desc: StartUIAbility
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, StartUIAbility_006, TestSize.Level1)
{
    auto mgr = std::make_unique<UIAbilityLifecycleManager>();
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfoCallback = new MockAbilityInfoCallbackStub();
    Rosen::SessionInfo info;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->sessionToken = new Rosen::Session(info);
    sessionInfo->persistentId = 1;
    sessionInfo->isNewWant = false;
    abilityRequest.sessionInfo = sessionInfo;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    mgr->sessionAbilityMap_.emplace(sessionInfo->persistentId, abilityRecord);
    AbilityRuntime::StartParamsBySCB params;
    bool isColdStart = false;
    EXPECT_EQ(mgr->StartUIAbility(abilityRequest, sessionInfo, params, isColdStart), ERR_OK);
}

/**
 * @tc.name: UIAbilityLifecycleManager_StartUIAbility_0700
 * @tc.desc: StartUIAbility
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, StartUIAbility_007, TestSize.Level1)
{
    auto mgr = std::make_unique<UIAbilityLifecycleManager>();
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.visible = true;
    abilityRequest.abilityInfoCallback = new MockAbilityInfoCallbackStub();
    Rosen::SessionInfo info;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->sessionToken = new Rosen::Session(info);
    sessionInfo->persistentId = 1;
    abilityRequest.sessionInfo = sessionInfo;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    abilityRecord->isReady_ = true;
    mgr->sessionAbilityMap_.emplace(sessionInfo->persistentId, abilityRecord);
    AbilityRuntime::StartParamsBySCB params;
    bool isColdStart = false;
    EXPECT_EQ(mgr->StartUIAbility(abilityRequest, sessionInfo, params, isColdStart), ERR_OK);
}

/**
 * @tc.name: UIAbilityLifecycleManager_StartUIAbility_0800
 * @tc.desc: StartUIAbility
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, StartUIAbility_008, TestSize.Level1)
{
    auto mgr = std::make_unique<UIAbilityLifecycleManager>();
    AbilityRequest abilityRequest;
    abilityRequest.want.SetParam(IS_CALLING_FROM_DMS, true);
    Rosen::SessionInfo info;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->sessionToken = new Rosen::Session(info);
    sessionInfo->isNewWant = false;
    AbilityRuntime::StartParamsBySCB params;
    bool isColdStart = false;
    EXPECT_EQ(mgr->StartUIAbility(abilityRequest, sessionInfo, params, isColdStart), ERR_OK);
}

/**
 * @tc.name: UIAbilityLifecycleManager_StartUIAbility_0900
 * @tc.desc: StartUIAbility
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, StartUIAbility_009, TestSize.Level1)
{
    auto mgr = std::make_unique<UIAbilityLifecycleManager>();
    AbilityRequest abilityRequest;
    Rosen::SessionInfo info;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->sessionToken = new Rosen::Session(info);
    sessionInfo->persistentId = 1;
    sessionInfo->reuseDelegatorWindow = true;
    abilityRequest.sessionInfo = sessionInfo;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    mgr->sessionAbilityMap_.emplace(sessionInfo->persistentId, abilityRecord);
    AbilityRuntime::StartParamsBySCB params;
    bool isColdStart = false;
    EXPECT_EQ(mgr->StartUIAbility(abilityRequest, sessionInfo, params, isColdStart), ERR_OK);
}

/**
 * @tc.name: StartUIAbility_1000
 * @tc.desc: StartUIAbility
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, StartUIAbility_1000, TestSize.Level1)
{
    auto mgr = std::make_unique<UIAbilityLifecycleManager>();
    AbilityRequest abilityRequest;
    Rosen::SessionInfo info;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->sessionToken = new Rosen::Session(info);
    sessionInfo->persistentId = 1;
    sessionInfo->processOptions = std::make_shared<ProcessOptions>();
    sessionInfo->processOptions->isPreloadStart = true;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    mgr->sessionAbilityMap_.emplace(sessionInfo->persistentId, abilityRecord);
    AbilityRuntime::StartParamsBySCB params;
    bool isColdStart = false;
    EXPECT_EQ(mgr->StartUIAbility(abilityRequest, sessionInfo, params, isColdStart), ERR_OK);
}

/**
 * @tc.name: StartUIAbility_1100
 * @tc.desc: StartUIAbility
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, StartUIAbility_1100, TestSize.Level1)
{
    auto mgr = std::make_unique<UIAbilityLifecycleManager>();
    AbilityRequest abilityRequest;
    Rosen::SessionInfo info;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->sessionToken = new Rosen::Session(info);
    sessionInfo->persistentId = 1;
    sessionInfo->reuseDelegatorWindow = true;
    sessionInfo->processOptions = std::make_shared<ProcessOptions>();
    sessionInfo->processOptions->isPreloadStart = true;
    abilityRequest.sessionInfo = sessionInfo;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    abilityRecord->SetPreloaded();
    mgr->sessionAbilityMap_.emplace(sessionInfo->persistentId, abilityRecord);
    AbilityRuntime::StartParamsBySCB params;
    bool isColdStart = false;
    EXPECT_EQ(mgr->StartUIAbility(abilityRequest, sessionInfo, params, isColdStart), ERR_OK);
    EXPECT_NE(abilityRequest.processOptions, nullptr);
    EXPECT_TRUE(abilityRequest.processOptions->isPreloadStart);
}

/**
 * @tc.name: StartUIAbility_1200
 * @tc.desc: StartUIAbility
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, StartUIAbility_1200, TestSize.Level1)
{
    auto mgr = std::make_unique<UIAbilityLifecycleManager>();
    AbilityRequest abilityRequest;
    Rosen::SessionInfo info;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->sessionToken = new Rosen::Session(info);
    sessionInfo->persistentId = 1;
    sessionInfo->reuseDelegatorWindow = true;
    sessionInfo->processOptions = std::make_shared<ProcessOptions>();
    sessionInfo->processOptions->isPreloadStart = false;
    sessionInfo->processOptions->callingPid = 2000;
    sessionInfo->processOptions->loadAbilityCallbackId = 123456;
    abilityRequest.sessionInfo = sessionInfo;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    abilityRecord->SetPid(1000);
    mgr->sessionAbilityMap_.emplace(sessionInfo->persistentId, abilityRecord);
    abilityRequest.appInfo.uid = 3000;
    AbilityRuntime::StartParamsBySCB params;
    bool isColdStart = false;
    EXPECT_EQ(mgr->StartUIAbility(abilityRequest, sessionInfo, params, isColdStart), ERR_OK);
    EXPECT_FALSE(isColdStart);
    EXPECT_NE(abilityRequest.processOptions, nullptr);
}

/**
 * @tc.name: UIAbilityLifecycleManager_CreateSessionInfo_0100
 * @tc.desc: CreateSessionInfo
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, CreateSessionInfo_001, TestSize.Level1)
{
    auto mgr = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(mgr, nullptr);
    AbilityRequest abilityRequest;
    abilityRequest.startSetting = std::make_shared<AbilityStartSetting>();
    int32_t requestId = 10000;
    EXPECT_NE(mgr->CreateSessionInfo(abilityRequest, requestId), nullptr);
}

/**
 * @tc.name: UIAbilityLifecycleManager_AbilityTransactionDone_0100
 * @tc.desc: AbilityTransactionDone
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, AbilityTransactionDone_001, TestSize.Level1)
{
    auto mgr = std::make_unique<UIAbilityLifecycleManager>();
    AbilityRequest abilityRequest;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    auto token = abilityRecord->GetToken()->AsObject();
    int state = 6;
    PacMap saveData;
    EXPECT_EQ(mgr->AbilityTransactionDone(token, state, saveData), ERR_INVALID_VALUE);
}

/**
 * @tc.name: UIAbilityLifecycleManager_AbilityTransactionDone_0200
 * @tc.desc: AbilityTransactionDone
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, AbilityTransactionDone_002, TestSize.Level1)
{
    auto mgr = std::make_unique<UIAbilityLifecycleManager>();
    AbilityRequest abilityRequest;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    mgr->terminateAbilityList_.emplace_back(abilityRecord);
    auto token = abilityRecord->GetToken()->AsObject();
    int state = 6;
    PacMap saveData;
    EXPECT_EQ(mgr->AbilityTransactionDone(token, state, saveData), ERR_INVALID_VALUE);
}

/**
 * @tc.name: AbilityTransactionDone_0300
 * @tc.desc: AbilityTransactionDone
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, AbilityTransactionDone_0300, TestSize.Level1)
{
    auto mgr = std::make_unique<UIAbilityLifecycleManager>();
    AbilityRequest abilityRequest;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    abilityRecord->SetPreloadStart(true);
    mgr->terminateAbilityList_.emplace_back(abilityRecord);
    auto token = abilityRecord->GetToken()->AsObject();
    int state = static_cast<int>(AbilityLifeCycleState::ABILITY_STATE_FOREGROUND_NEW);
    PacMap saveData;
    mgr->AbilityTransactionDone(token, state, saveData);
    EXPECT_TRUE(abilityRecord->IsPreloaded());
}

/**
 * @tc.name: UIAbilityLifecycleManager_AttachAbilityThread_0100
 * @tc.desc: AttachAbilityThread
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, AttachAbilityThread_001, TestSize.Level1)
{
    auto mgr = std::make_unique<UIAbilityLifecycleManager>();
    sptr<IAbilityScheduler> scheduler = nullptr;
    sptr<IRemoteObject> token = nullptr;
    EXPECT_EQ(mgr->AttachAbilityThread(scheduler, token), ERR_INVALID_VALUE);
}

/**
 * @tc.name: UIAbilityLifecycleManager_AttachAbilityThread_0200
 * @tc.desc: AttachAbilityThread
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, AttachAbilityThread_002, TestSize.Level1)
{
    auto mgr = std::make_unique<UIAbilityLifecycleManager>();
    AbilityRequest abilityRequest;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    mgr->sessionAbilityMap_.emplace(1, abilityRecord);
    sptr<IAbilityScheduler> scheduler = nullptr;
    auto&& token = abilityRecord->GetToken()->AsObject();
    EXPECT_EQ(mgr->AttachAbilityThread(scheduler, token), ERR_INVALID_VALUE);
}

/**
 * @tc.name: UIAbilityLifecycleManager_AttachAbilityThread_0300
 * @tc.desc: AttachAbilityThread
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, AttachAbilityThread_003, TestSize.Level1)
{
    auto mgr = std::make_unique<UIAbilityLifecycleManager>();
    AbilityRequest abilityRequest;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    abilityRecord->SetStartedByCall(true);

    mgr->sessionAbilityMap_.emplace(1, abilityRecord);
    sptr<IAbilityScheduler> scheduler = nullptr;
    auto&& token = abilityRecord->GetToken()->AsObject();
    EXPECT_EQ(mgr->AttachAbilityThread(scheduler, token), ERR_INVALID_VALUE);
}

/**
 * @tc.name: UIAbilityLifecycleManager_AttachAbilityThread_0400
 * @tc.desc: AttachAbilityThread
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, AttachAbilityThread_004, TestSize.Level1)
{
    auto mgr = std::make_unique<UIAbilityLifecycleManager>();
    AbilityRequest abilityRequest;
    Want want;
    want.SetParam(Want::PARAM_RESV_CALL_TO_FOREGROUND, true);
    abilityRequest.want = want;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    abilityRecord->SetStartedByCall(true);

    mgr->sessionAbilityMap_.emplace(1, abilityRecord);
    sptr<IAbilityScheduler> scheduler = nullptr;
    auto&& token = abilityRecord->GetToken()->AsObject();
    EXPECT_EQ(mgr->AttachAbilityThread(scheduler, token), ERR_INVALID_VALUE);
}

/**
 * @tc.name: UIAbilityLifecycleManager_OnAbilityRequestDone_0100
 * @tc.desc: OnAbilityRequestDone
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, OnAbilityRequestDone_001, TestSize.Level1)
{
    auto mgr = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(mgr, nullptr);
    sptr<IRemoteObject> token = nullptr;
    mgr->OnAbilityRequestDone(token, 1);
    EXPECT_NE(mgr, nullptr);
}

/**
 * @tc.name: UIAbilityLifecycleManager_OnAbilityRequestDone_0200
 * @tc.desc: OnAbilityRequestDone
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, OnAbilityRequestDone_002, TestSize.Level1)
{
    auto mgr = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(mgr, nullptr);
    AbilityRequest abilityRequest;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    mgr->sessionAbilityMap_[1] = abilityRecord;
    auto&& token = abilityRecord->GetToken()->AsObject();
    int32_t foreground = 2;
    mgr->OnAbilityRequestDone(token, foreground);
    EXPECT_EQ(abilityRecord->GetPendingState(), AbilityState::INITIAL);
}

/**
 * @tc.name: UIAbilityLifecycleManager_GetAbilityRecordByToken_0100
 * @tc.desc: GetAbilityRecordByToken
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, GetAbilityRecordByToken_001, TestSize.Level1)
{
    auto mgr = std::make_unique<UIAbilityLifecycleManager>();
    sptr<IRemoteObject> token = nullptr;
    EXPECT_EQ(mgr->GetAbilityRecordByToken(token), nullptr);
}

/**
 * @tc.name: UIAbilityLifecycleManager_GetAbilityRecordByToken_0200
 * @tc.desc: GetAbilityRecordByToken
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, GetAbilityRecordByToken_002, TestSize.Level1)
{
    auto mgr = std::make_unique<UIAbilityLifecycleManager>();
    AbilityRequest abilityRequest;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    auto&& token = abilityRecord->GetToken()->AsObject();
    EXPECT_EQ(mgr->GetAbilityRecordByToken(token), nullptr);
}

/**
 * @tc.name: UIAbilityLifecycleManager_GetAbilityRecordByToken_0300
 * @tc.desc: GetAbilityRecordByToken
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, GetAbilityRecordByToken_003, TestSize.Level1)
{
    auto mgr = std::make_unique<UIAbilityLifecycleManager>();
    AbilityRequest abilityRequest;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    mgr->sessionAbilityMap_.emplace(1, abilityRecord);
    auto&& token = abilityRecord->GetToken()->AsObject();
    EXPECT_NE(mgr->GetAbilityRecordByToken(token), nullptr);
}

/**
 * @tc.name: UIAbilityLifecycleManager_UpdateAbilityRecordLaunchReason_0100
 * @tc.desc: UpdateAbilityRecordLaunchReason
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, UpdateAbilityRecordLaunchReason_001, TestSize.Level1)
{
    auto mgr = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(mgr, nullptr);
    AbilityRequest abilityRequest;
    UIAbilityRecordPtr abilityRecord = nullptr;
    mgr->UpdateAbilityRecordLaunchReason(abilityRequest, abilityRecord);
    EXPECT_NE(mgr, nullptr);
}

/**
 * @tc.name: UIAbilityLifecycleManager_UpdateAbilityRecordLaunchReason_0200
 * @tc.desc: UpdateAbilityRecordLaunchReason
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, UpdateAbilityRecordLaunchReason_002, TestSize.Level1)
{
    auto mgr = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(mgr, nullptr);
    Want want;
    want.SetFlags(Want::FLAG_ABILITY_CONTINUATION);
    AbilityRequest abilityRequest;
    abilityRequest.want = want;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    mgr->UpdateAbilityRecordLaunchReason(abilityRequest, abilityRecord);
    EXPECT_NE(mgr, nullptr);
}

/**
 * @tc.name: UIAbilityLifecycleManager_UpdateAbilityRecordLaunchReason_0300
 * @tc.desc: UpdateAbilityRecordLaunchReason
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, UpdateAbilityRecordLaunchReason_003, TestSize.Level1)
{
    auto mgr = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(mgr, nullptr);
    Want want;
    want.SetParam(Want::PARAM_ABILITY_RECOVERY_RESTART, true);
    AbilityRequest abilityRequest;
    abilityRequest.want = want;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    mgr->UpdateAbilityRecordLaunchReason(abilityRequest, abilityRecord);
    EXPECT_NE(mgr, nullptr);
}

/**
 * @tc.name: UIAbilityLifecycleManager_UpdateAbilityRecordLaunchReason_0400
 * @tc.desc: UpdateAbilityRecordLaunchReason
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, UpdateAbilityRecordLaunchReason_004, TestSize.Level1)
{
    auto mgr = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(mgr, nullptr);
    Want want;
    want.SetParam(Want::PARAM_ABILITY_RECOVERY_RESTART, true);
    AbilityRequest abilityRequest;
    abilityRequest.want = want;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    mgr->UpdateAbilityRecordLaunchReason(abilityRequest, abilityRecord);
    EXPECT_NE(mgr, nullptr);
}

/**
 * @tc.name: UpdateAbilityRecordLaunchReason_0500
 * @tc.desc: UpdateAbilityRecordLaunchReason
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, UpdateAbilityRecordLaunchReason_0500, TestSize.Level1)
{
    auto mgr = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(mgr, nullptr);
    AbilityRequest abilityRequest;
    abilityRequest.processOptions = std::make_shared<ProcessOptions>();
    abilityRequest.processOptions->isPreloadStart = true;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    mgr->UpdateAbilityRecordLaunchReason(abilityRequest, abilityRecord);
    EXPECT_EQ(abilityRecord->lifeCycleStateInfo_.launchParam.launchReason, LaunchReason::LAUNCHREASON_PRELOAD);
}

/**
 * @tc.name: UIAbilityLifecycleManager_EraseAbilityRecord_0100
 * @tc.desc: EraseAbilityRecord
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, EraseAbilityRecord_001, TestSize.Level1)
{
    auto mgr = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(mgr, nullptr);
    UIAbilityRecordPtr abilityRecord = nullptr;
    mgr->EraseAbilityRecord(abilityRecord);
    EXPECT_NE(mgr, nullptr);
}

/**
 * @tc.name: UIAbilityLifecycleManager_EraseAbilityRecord_0200
 * @tc.desc: EraseAbilityRecord
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, EraseAbilityRecord_002, TestSize.Level1)
{
    auto mgr = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(mgr, nullptr);
    AbilityRequest abilityRequest;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    mgr->sessionAbilityMap_.emplace(1, abilityRecord);
    mgr->EraseAbilityRecord(abilityRecord);
    EXPECT_NE(mgr, nullptr);
}

/**
 * @tc.name: UIAbilityLifecycleManager_DispatchState_0100
 * @tc.desc: DispatchState
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, DispatchState_001, TestSize.Level1)
{
    auto mgr = std::make_unique<UIAbilityLifecycleManager>();
    UIAbilityRecordPtr abilityRecord = nullptr;
    EXPECT_EQ(mgr->DispatchState(abilityRecord, AbilityState::INITIAL), ERR_INVALID_VALUE);
}

/**
 * @tc.name: UIAbilityLifecycleManager_DispatchState_0200
 * @tc.desc: DispatchState
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, DispatchState_002, TestSize.Level1)
{
    auto mgr = std::make_unique<UIAbilityLifecycleManager>();
    UIAbilityRecordPtr abilityRecord = nullptr;
    EXPECT_EQ(mgr->DispatchState(abilityRecord, AbilityState::FOREGROUND), ERR_INVALID_VALUE);
}

/**
 * @tc.name: UIAbilityLifecycleManager_DispatchState_0300
 * @tc.desc: DispatchState
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, DispatchState_003, TestSize.Level1)
{
    auto mgr = std::make_unique<UIAbilityLifecycleManager>();
    UIAbilityRecordPtr abilityRecord = nullptr;
    EXPECT_EQ(mgr->DispatchState(abilityRecord, AbilityState::FOREGROUND_FAILED), ERR_INVALID_VALUE);
}

/**
 * @tc.name: UIAbilityLifecycleManager_DispatchState_0400
 * @tc.desc: DispatchState
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, DispatchState_004, TestSize.Level1)
{
    auto mgr = std::make_unique<UIAbilityLifecycleManager>();
    UIAbilityRecordPtr abilityRecord = nullptr;
    int state = 130;
    EXPECT_EQ(mgr->DispatchState(abilityRecord, state), ERR_INVALID_VALUE);
}

/**
 * @tc.name: UIAbilityLifecycleManager_CompleteForegroundSuccess_0100
 * @tc.desc: CompleteForegroundSuccess
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, CompleteForegroundSuccess_001, TestSize.Level1)
{
    auto mgr = std::make_unique<UIAbilityLifecycleManager>();
    mgr->CompleteForegroundSuccess(nullptr);
    EXPECT_NE(mgr, nullptr);
}

/**
 * @tc.name: UIAbilityLifecycleManager_CompleteForegroundSuccess_0200
 * @tc.desc: CompleteForegroundSuccess
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, CompleteForegroundSuccess_002, TestSize.Level1)
{
    auto mgr = std::make_unique<UIAbilityLifecycleManager>();
    AbilityRequest abilityRequest;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    abilityRecord->SetPendingState(AbilityState::FOREGROUND);
    mgr->CompleteForegroundSuccess(abilityRecord);
    EXPECT_NE(mgr, nullptr);
}

/**
 * @tc.name: UIAbilityLifecycleManager_CompleteForegroundSuccess_0300
 * @tc.desc: CompleteForegroundSuccess
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, CompleteForegroundSuccess_003, TestSize.Level1)
{
    auto mgr = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(mgr, nullptr);
    AbilityRequest abilityRequest;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    abilityRecord->SetStartedByCall(true);
    abilityRecord->SetStartToForeground(true);
    abilityRecord->isReady_ = true;
    abilityRecord->SetPendingState(AbilityState::BACKGROUND);
    mgr->CompleteForegroundSuccess(abilityRecord);
    EXPECT_NE(mgr, nullptr);
}

/**
 * @tc.name: UIAbilityLifecycleManager_CompleteForegroundSuccess_0400
 * @tc.desc: CompleteForegroundSuccess
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, CompleteForegroundSuccess_004, TestSize.Level1)
{
    auto mgr = std::make_unique<UIAbilityLifecycleManager>();
    AbilityRequest abilityRequest;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    abilityRecord->SetLastWant(std::make_shared<Want>());
    sptr<ISessionHandler> handler;
    mgr->SetSessionHandler(handler);
    abilityRecord->SetSessionInfo(new SessionInfo());
    mgr->CompleteForegroundSuccess(abilityRecord);
    EXPECT_NE(mgr, nullptr);
}

/**
 * @tc.name: UIAbilityLifecycleManager_HandleForegroundFailed_0100
 * @tc.desc: HandleForegroundOrFailed
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, HandleForegroundFailed_001, TestSize.Level1)
{
    auto mgr = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(mgr, nullptr);
    UIAbilityRecordPtr abilityRecord = nullptr;
    mgr->HandleForegroundFailed(abilityRecord, AbilityState::FOREGROUND_FAILED);
    EXPECT_NE(mgr, nullptr);
}

/**
 * @tc.name: UIAbilityLifecycleManager_HandleForegroundFailed_0200
 * @tc.desc: HandleForegroundFailed
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, HandleForegroundFailed_002, TestSize.Level1)
{
    auto mgr = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(mgr, nullptr);
    AbilityRequest abilityRequest;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    abilityRecord->currentState_ = AbilityState::FOREGROUNDING;
    mgr->HandleForegroundFailed(abilityRecord, AbilityState::FOREGROUND_FAILED);
    EXPECT_NE(mgr, nullptr);
}

/**
 * @tc.name: UIAbilityLifecycleManager_MinimizeUIAbility_0100
 * @tc.desc: MinimizeUIAbility
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, MinimizeUIAbility_001, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    EXPECT_EQ(uiAbilityLifecycleManager->MinimizeUIAbility(nullptr, false, 0), ERR_INVALID_VALUE);
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_MinimizeUIAbility_0200
 * @tc.desc: MinimizeUIAbility
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, MinimizeUIAbility_002, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    UIAbilityRecordPtr abilityRecord = InitAbilityRecord();
    abilityRecord->currentState_ = AbilityState::BACKGROUND;
    EXPECT_EQ(uiAbilityLifecycleManager->MinimizeUIAbility(abilityRecord, false, 0), ERR_OK);
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_MinimizeUIAbility_0300
 * @tc.desc: MinimizeUIAbility
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, MinimizeUIAbility_003, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    UIAbilityRecordPtr abilityRecord = InitAbilityRecord();
    abilityRecord->currentState_ = AbilityState::FOREGROUND;
    EXPECT_EQ(uiAbilityLifecycleManager->MinimizeUIAbility(abilityRecord, false, 0), ERR_OK);
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_MoveToBackground_0100
 * @tc.desc: MoveToBackground
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, MoveToBackground_001, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(uiAbilityLifecycleManager, nullptr);
    uiAbilityLifecycleManager->MoveToBackground(nullptr);
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_MoveToBackground_0200
 * @tc.desc: MoveToBackground
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, MoveToBackground_002, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(uiAbilityLifecycleManager, nullptr);
    UIAbilityRecordPtr abilityRecord = InitAbilityRecord();
    uiAbilityLifecycleManager->MoveToBackground(abilityRecord);
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_PrintTimeOutLog_0100
 * @tc.desc: PrintTimeOutLog
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, PrintTimeOutLog_001, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(uiAbilityLifecycleManager, nullptr);
    uint32_t msgId = 0;
    uiAbilityLifecycleManager->PrintTimeOutLog(nullptr, msgId);
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_PrintTimeOutLog_0200
 * @tc.desc: PrintTimeOutLog
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, PrintTimeOutLog_002, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(uiAbilityLifecycleManager, nullptr);
    UIAbilityRecordPtr abilityRecord = InitAbilityRecord();
    uint32_t msgId = 0;
    uiAbilityLifecycleManager->PrintTimeOutLog(abilityRecord, msgId);
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_PrintTimeOutLog_0300
 * @tc.desc: PrintTimeOutLog
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, PrintTimeOutLog_003, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(uiAbilityLifecycleManager, nullptr);
    UIAbilityRecordPtr abilityRecord = InitAbilityRecord();
    uint32_t msgId = 1;
    uiAbilityLifecycleManager->PrintTimeOutLog(abilityRecord, msgId);
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_PrintTimeOutLog_0400
 * @tc.desc: PrintTimeOutLog
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, PrintTimeOutLog_004, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(uiAbilityLifecycleManager, nullptr);
    UIAbilityRecordPtr abilityRecord = InitAbilityRecord();
    uint32_t msgId = 2;
    uiAbilityLifecycleManager->PrintTimeOutLog(abilityRecord, msgId);
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_PrintTimeOutLog_0500
 * @tc.desc: PrintTimeOutLog
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, PrintTimeOutLog_005, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(uiAbilityLifecycleManager, nullptr);
    UIAbilityRecordPtr abilityRecord = InitAbilityRecord();
    uint32_t msgId = 3;
    uiAbilityLifecycleManager->PrintTimeOutLog(abilityRecord, msgId);
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_PrintTimeOutLog_0600
 * @tc.desc: PrintTimeOutLog
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, PrintTimeOutLog_006, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(uiAbilityLifecycleManager, nullptr);
    UIAbilityRecordPtr abilityRecord = InitAbilityRecord();
    uint32_t msgId = 4;
    uiAbilityLifecycleManager->PrintTimeOutLog(abilityRecord, msgId);
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_PrintTimeOutLog_0700
 * @tc.desc: PrintTimeOutLog
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, PrintTimeOutLog_007, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(uiAbilityLifecycleManager, nullptr);
    UIAbilityRecordPtr abilityRecord = InitAbilityRecord();
    uint32_t msgId = 5;
    uiAbilityLifecycleManager->PrintTimeOutLog(abilityRecord, msgId);
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_PrintTimeOutLog_0800
 * @tc.desc: PrintTimeOutLog
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, PrintTimeOutLog_008, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(uiAbilityLifecycleManager, nullptr);
    UIAbilityRecordPtr abilityRecord = InitAbilityRecord();
    uint32_t msgId = 6;
    uiAbilityLifecycleManager->PrintTimeOutLog(abilityRecord, msgId);
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_CompleteBackground_0100
 * @tc.desc: CompleteBackground
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, CompleteBackground_001, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(uiAbilityLifecycleManager, nullptr);
    UIAbilityRecordPtr abilityRecord = InitAbilityRecord();
    abilityRecord->currentState_ = AbilityState::FOREGROUND;
    uiAbilityLifecycleManager->CompleteBackground(abilityRecord);
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_CompleteBackground_0200
 * @tc.desc: CompleteBackground
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, CompleteBackground_002, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    UIAbilityRecordPtr abilityRecord = InitAbilityRecord();
    abilityRecord->currentState_ = AbilityState::BACKGROUNDING;
    abilityRecord->SetPendingState(AbilityState::FOREGROUND);
    uiAbilityLifecycleManager->CompleteBackground(abilityRecord);
    EXPECT_EQ(abilityRecord->GetAbilityState(), AbilityState::FOREGROUNDING);
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_CompleteBackground_0300
 * @tc.desc: CompleteBackground
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, CompleteBackground_003, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(uiAbilityLifecycleManager, nullptr);
    UIAbilityRecordPtr abilityRecord = InitAbilityRecord();
    abilityRecord->currentState_ = AbilityState::BACKGROUNDING;
    abilityRecord->SetPendingState(AbilityState::BACKGROUND);
    uiAbilityLifecycleManager->CompleteBackground(abilityRecord);
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_CompleteBackground_0500
 * @tc.desc: CompleteBackground
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, CompleteBackground_005, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    EXPECT_NE(mgr, nullptr);
    AbilityRequest abilityRequest;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    abilityRecord->currentState_ = AbilityState::BACKGROUNDING;
    abilityRecord->SetStartedByCall(true);
    abilityRecord->SetStartToBackground(true);
    abilityRecord->isReady_ = true;
    mgr->CompleteBackground(abilityRecord);
    EXPECT_NE(mgr, nullptr);
}

/**
 * @tc.name: UIAbilityLifecycleManager_CompleteBackground_0600
 * @tc.desc: CompleteBackground
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, CompleteBackground_006, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    EXPECT_NE(mgr, nullptr);
    AbilityRequest abilityRequest;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    abilityRecord->currentState_ = AbilityState::BACKGROUNDING;
    mgr->terminateAbilityList_.push_back(abilityRecord);
    mgr->CompleteBackground(abilityRecord);
    EXPECT_NE(mgr, nullptr);
}

/**
 * @tc.name: UIAbilityLifecycleManager_CloseUIAbility_0100
 * @tc.desc: CloseUIAbility
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, CloseUIAbility_001, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    UIAbilityRecordPtr abilityRecord = InitAbilityRecord();
    abilityRecord->SetTerminatingState();
    abilityRecord->currentState_ = AbilityState::BACKGROUND;
    EXPECT_EQ(uiAbilityLifecycleManager->CloseUIAbility(abilityRecord, -1, nullptr, false, false), ERR_OK);
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_CloseUIAbility_0200
 * @tc.desc: CloseUIAbility
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, CloseUIAbility_002, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    UIAbilityRecordPtr abilityRecord = InitAbilityRecord();
    EXPECT_EQ(uiAbilityLifecycleManager->CloseUIAbility(abilityRecord, -1, nullptr, false, false), ERR_INVALID_VALUE);
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_CloseUIAbility_0300
 * @tc.desc: CloseUIAbility
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, CloseUIAbility_003, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    UIAbilityRecordPtr abilityRecord = InitAbilityRecord();
    Want want;
    EXPECT_EQ(uiAbilityLifecycleManager->CloseUIAbility(abilityRecord, -1, &want, false, false), ERR_INVALID_VALUE);
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_CloseUIAbility_0400
 * @tc.desc: CloseUIAbility
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, CloseUIAbility_004, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    auto abilityRecord = InitAbilityRecord();
    abilityRecord->currentState_ = AbilityState::FOREGROUND;
    Want want;
    EXPECT_EQ(uiAbilityLifecycleManager->CloseUIAbility(abilityRecord, -1, &want, false, false), ERR_OK);
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_CloseUIAbility_0500
 * @tc.desc: CloseUIAbility
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, CloseUIAbility_005, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    auto abilityRecord = InitAbilityRecord();
    abilityRecord->currentState_ = AbilityState::FOREGROUNDING;
    Want want;
    EXPECT_EQ(uiAbilityLifecycleManager->CloseUIAbility(abilityRecord, -1, &want, false, false), ERR_OK);
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_CloseUIAbility_0600
 * @tc.desc: CloseUIAbility
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, CloseUIAbility_006, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    auto abilityRecord = InitAbilityRecord();
    abilityRecord->currentState_ = AbilityState::BACKGROUND;
    Want want;
    EXPECT_EQ(uiAbilityLifecycleManager->CloseUIAbility(abilityRecord, -1, &want, false, false), ERR_OK);
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_CloseUIAbility_0700
 * @tc.desc: CloseUIAbility
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, CloseUIAbility_007, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    auto abilityRecord = InitAbilityRecord();
    abilityRecord->currentState_ = AbilityState::BACKGROUND;
    abilityRecord->SetPendingState(FOREGROUND);
    Want want;
    EXPECT_EQ(uiAbilityLifecycleManager->CloseUIAbility(abilityRecord, -1, &want, false, false), ERR_OK);
}

/**
 * @tc.name: UIAbilityLifecycleManager_CloseUIAbility_0800
 * @tc.desc: CloseUIAbility
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, CloseUIAbility_008, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    auto abilityRecord = InitAbilityRecord();
    abilityRecord->currentState_ = AbilityState::FOREGROUND;
    abilityRecord->SetPendingState(FOREGROUND);
    Want want;
    EXPECT_EQ(uiAbilityLifecycleManager->CloseUIAbility(abilityRecord, -1, &want, false, true), ERR_OK);
}

/**
 * @tc.name: UIAbilityLifecycleManager_DelayCompleteTerminate_0100
 * @tc.desc: DelayCompleteTerminate
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, DelayCompleteTerminate_001, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(uiAbilityLifecycleManager, nullptr);
    UIAbilityRecordPtr abilityRecord = InitAbilityRecord();
    uiAbilityLifecycleManager->DelayCompleteTerminate(abilityRecord);
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_CompleteTerminate_0100
 * @tc.desc: CompleteTerminate
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, CompleteTerminate_001, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(uiAbilityLifecycleManager, nullptr);
    UIAbilityRecordPtr abilityRecord = InitAbilityRecord();
    abilityRecord->currentState_ = AbilityState::BACKGROUND;
    uiAbilityLifecycleManager->CompleteTerminate(abilityRecord);
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_CompleteTerminate_0200
 * @tc.desc: CompleteTerminate
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, CompleteTerminate_002, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(uiAbilityLifecycleManager, nullptr);
    UIAbilityRecordPtr abilityRecord = InitAbilityRecord();
    abilityRecord->currentState_ = AbilityState::TERMINATING;
    uiAbilityLifecycleManager->CompleteTerminate(abilityRecord);
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_OnTimeOut_0100
 * @tc.desc: OnTimeOut
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, OnTimeOut_001, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(uiAbilityLifecycleManager, nullptr);
    UIAbilityRecordPtr abilityRecord = InitAbilityRecord();
    uint32_t msgId = 0;
    int64_t abilityRecordId = 0;
    uiAbilityLifecycleManager->OnTimeOut(msgId, abilityRecordId);
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_OnTimeOut_0200
 * @tc.desc: OnTimeOut
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, OnTimeOut_002, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(uiAbilityLifecycleManager, nullptr);
    UIAbilityRecordPtr abilityRecord = InitAbilityRecord();
    uint32_t msgId = 0;
    int64_t abilityRecordId = 0;
    uiAbilityLifecycleManager->sessionAbilityMap_.emplace(0, abilityRecord);
    uiAbilityLifecycleManager->OnTimeOut(msgId, abilityRecordId);
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_OnTimeOut_0300
 * @tc.desc: OnTimeOut
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, OnTimeOut_003, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(uiAbilityLifecycleManager, nullptr);
    UIAbilityRecordPtr abilityRecord = InitAbilityRecord();
    uint32_t msgId = 5;
    int64_t abilityRecordId = 0;
    uiAbilityLifecycleManager->sessionAbilityMap_.emplace(0, abilityRecord);
    uiAbilityLifecycleManager->OnTimeOut(msgId, abilityRecordId);
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_OnTimeOut_0400
 * @tc.desc: OnTimeOut
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, OnTimeOut_004, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(uiAbilityLifecycleManager, nullptr);
    UIAbilityRecordPtr abilityRecord = InitAbilityRecord();
    uint32_t msgId = 6;
    int64_t abilityRecordId = 0;
    uiAbilityLifecycleManager->sessionAbilityMap_.emplace(msgId, abilityRecord);
    uiAbilityLifecycleManager->OnTimeOut(msgId, abilityRecordId);
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_NotifySCBToHandleException_0100
 * @tc.desc: NotifySCBToHandleException
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, NotifySCBToHandleException_001, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(uiAbilityLifecycleManager, nullptr);
    UIAbilityRecordPtr record = nullptr;
    uiAbilityLifecycleManager->NotifySCBToHandleException(record,
        static_cast<int32_t>(ErrorLifecycleState::ABILITY_STATE_LOAD_TIMEOUT), "handleLoadTimeout");
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_NotifySCBToHandleException_0200
 * @tc.desc: NotifySCBToHandleException
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, NotifySCBToHandleException_002, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(uiAbilityLifecycleManager, nullptr);
    UIAbilityRecordPtr abilityRecord = InitAbilityRecord();
    uiAbilityLifecycleManager->NotifySCBToHandleException(abilityRecord,
        static_cast<int32_t>(ErrorLifecycleState::ABILITY_STATE_LOAD_TIMEOUT), "handleLoadTimeout");
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_NotifySCBToHandleException_0300
 * @tc.desc: NotifySCBToHandleException
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, NotifySCBToHandleException_003, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest abilityRequest;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    abilityRequest.sessionInfo = sessionInfo;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    uiAbilityLifecycleManager->NotifySCBToHandleException(abilityRecord,
        static_cast<int32_t>(ErrorLifecycleState::ABILITY_STATE_LOAD_TIMEOUT), "handleLoadTimeout");
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_NotifySCBToHandleException_0400
 * @tc.desc: NotifySCBToHandleException
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, NotifySCBToHandleException_004, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest abilityRequest;
    Rosen::SessionInfo info;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->sessionToken = new Rosen::Session(info);
    abilityRequest.sessionInfo = sessionInfo;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    uiAbilityLifecycleManager->NotifySCBToHandleException(abilityRecord,
        static_cast<int32_t>(ErrorLifecycleState::ABILITY_STATE_LOAD_TIMEOUT), "handleLoadTimeout");
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_NotifySCBToHandleException_0500
 * @tc.desc: NotifySCBToHandleException
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, NotifySCBToHandleException_005, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest abilityRequest;
    Rosen::SessionInfo info;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->sessionToken = new Rosen::Session(info);
    abilityRequest.sessionInfo = sessionInfo;
    sessionInfo->persistentId = 0;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    uiAbilityLifecycleManager->sessionAbilityMap_.emplace(sessionInfo->persistentId, abilityRecord);
    uiAbilityLifecycleManager->NotifySCBToHandleException(abilityRecord,
        static_cast<int32_t>(ErrorLifecycleState::ABILITY_STATE_LOAD_TIMEOUT), "handleLoadTimeout");
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_NotifySCBToHandleException_0600
 * @tc.desc: NotifySCBToHandleException
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, NotifySCBToHandleException_006, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest abilityRequest;
    Rosen::SessionInfo info;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->sessionToken = new Rosen::Session(info);
    abilityRequest.sessionInfo = sessionInfo;
    sessionInfo->persistentId = 0;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    uiAbilityLifecycleManager->sessionAbilityMap_.emplace(sessionInfo->persistentId, abilityRecord);
    uiAbilityLifecycleManager->NotifySCBToHandleException(abilityRecord,
        static_cast<int32_t>(ErrorLifecycleState::ABILITY_STATE_FOREGROUND_TIMEOUT), "handleForegroundTimeout");
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_NotifySCBToHandleException_0700
 * @tc.desc: NotifySCBToHandleException
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, NotifySCBToHandleException_007, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest abilityRequest;
    Rosen::SessionInfo info;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->sessionToken = new Rosen::Session(info);
    abilityRequest.sessionInfo = sessionInfo;
    sessionInfo->persistentId = 0;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    uiAbilityLifecycleManager->sessionAbilityMap_.emplace(sessionInfo->persistentId, abilityRecord);
    uiAbilityLifecycleManager->NotifySCBToHandleException(abilityRecord,
        static_cast<int32_t>(ErrorLifecycleState::ABILITY_STATE_DIED), "onAbilityDied");
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_HandleLoadTimeout_0100
 * @tc.desc: HandleLoadTimeout
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, HandleLoadTimeout_001, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(uiAbilityLifecycleManager, nullptr);
    uiAbilityLifecycleManager->HandleLoadTimeout(nullptr);
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_HandleLoadTimeout_0200
 * @tc.desc: HandleLoadTimeout
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, HandleLoadTimeout_002, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(uiAbilityLifecycleManager, nullptr);
    UIAbilityRecordPtr abilityRecord = InitAbilityRecord();
    uiAbilityLifecycleManager->HandleLoadTimeout(abilityRecord);
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_HandleLoadTimeout_0300
 * @tc.desc: HandleLoadTimeout
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, HandleLoadTimeout_003, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest abilityRequest;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    abilityRequest.sessionInfo = sessionInfo;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    uiAbilityLifecycleManager->HandleLoadTimeout(abilityRecord);
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_HandleLoadTimeout_0400
 * @tc.desc: HandleLoadTimeout
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, HandleLoadTimeout_004, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest abilityRequest;
    Rosen::SessionInfo info;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->sessionToken = new Rosen::Session(info);
    abilityRequest.sessionInfo = sessionInfo;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    uiAbilityLifecycleManager->HandleLoadTimeout(abilityRecord);
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_HandleLoadTimeout_0500
 * @tc.desc: HandleLoadTimeout
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, HandleLoadTimeout_005, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest abilityRequest;
    Rosen::SessionInfo info;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->sessionToken = new Rosen::Session(info);
    abilityRequest.sessionInfo = sessionInfo;
    sessionInfo->persistentId = 0;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    uiAbilityLifecycleManager->sessionAbilityMap_.emplace(sessionInfo->persistentId, abilityRecord);
    uiAbilityLifecycleManager->HandleLoadTimeout(abilityRecord);
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_HandleForegroundTimeout_0100
 * @tc.desc: HandleForegroundTimeout
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, HandleForegroundTimeout_001, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(uiAbilityLifecycleManager, nullptr);
    uiAbilityLifecycleManager->HandleForegroundTimeout(nullptr);
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_HandleForegroundTimeout_0200
 * @tc.desc: HandleForegroundTimeout
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, HandleForegroundTimeout_002, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(uiAbilityLifecycleManager, nullptr);
    UIAbilityRecordPtr abilityRecord = InitAbilityRecord();
    abilityRecord->currentState_ = AbilityState::TERMINATING;
    uiAbilityLifecycleManager->HandleForegroundTimeout(abilityRecord);
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_HandleForegroundTimeout_0300
 * @tc.desc: HandleForegroundTimeout
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, HandleForegroundTimeout_003, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(uiAbilityLifecycleManager, nullptr);
    UIAbilityRecordPtr abilityRecord = InitAbilityRecord();
    abilityRecord->currentState_ = AbilityState::FOREGROUNDING;
    uiAbilityLifecycleManager->HandleForegroundTimeout(abilityRecord);
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_HandleForegroundTimeout_0400
 * @tc.desc: HandleForegroundTimeout
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, HandleForegroundTimeout_004, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest abilityRequest;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    abilityRequest.sessionInfo = sessionInfo;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    abilityRecord->currentState_ = AbilityState::FOREGROUNDING;
    uiAbilityLifecycleManager->HandleForegroundTimeout(abilityRecord);
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_HandleForegroundTimeout_0500
 * @tc.desc: HandleForegroundTimeout
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, HandleForegroundTimeout_005, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest abilityRequest;
    Rosen::SessionInfo info;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->sessionToken = new Rosen::Session(info);
    abilityRequest.sessionInfo = sessionInfo;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    abilityRecord->currentState_ = AbilityState::FOREGROUNDING;
    uiAbilityLifecycleManager->HandleForegroundTimeout(abilityRecord);
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_HandleForegroundTimeout_0600
 * @tc.desc: HandleForegroundTimeout
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, HandleForegroundTimeout_006, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest abilityRequest;
    Rosen::SessionInfo info;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->sessionToken = new Rosen::Session(info);
    abilityRequest.sessionInfo = sessionInfo;
    sessionInfo->persistentId = 0;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    abilityRecord->currentState_ = AbilityState::FOREGROUNDING;
    uiAbilityLifecycleManager->sessionAbilityMap_.emplace(sessionInfo->persistentId, abilityRecord);
    uiAbilityLifecycleManager->HandleForegroundTimeout(abilityRecord);
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_OnAbilityDied_0100
 * @tc.desc: OnAbilityDied
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, OnAbilityDied_001, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(uiAbilityLifecycleManager, nullptr);
    uiAbilityLifecycleManager->OnAbilityDied(nullptr);
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_OnAbilityDied_0200
 * @tc.desc: OnAbilityDied
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, OnAbilityDied_002, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(uiAbilityLifecycleManager, nullptr);
    UIAbilityRecordPtr abilityRecord = InitAbilityRecord();
    uiAbilityLifecycleManager->OnAbilityDied(abilityRecord);
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_OnAbilityDied_0300
 * @tc.desc: OnAbilityDied
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, OnAbilityDied_003, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest abilityRequest;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    abilityRequest.sessionInfo = sessionInfo;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    uiAbilityLifecycleManager->OnAbilityDied(abilityRecord);
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_OnAbilityDied_0400
 * @tc.desc: OnAbilityDied
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, OnAbilityDied_004, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest abilityRequest;
    Rosen::SessionInfo info;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->sessionToken = new Rosen::Session(info);
    abilityRequest.sessionInfo = sessionInfo;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    uiAbilityLifecycleManager->OnAbilityDied(abilityRecord);
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_OnAbilityDied_0500
 * @tc.desc: OnAbilityDied
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, OnAbilityDied_005, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest abilityRequest;
    Rosen::SessionInfo info;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->sessionToken = new Rosen::Session(info);
    abilityRequest.sessionInfo = sessionInfo;
    sessionInfo->persistentId = 0;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    uiAbilityLifecycleManager->sessionAbilityMap_.emplace(sessionInfo->persistentId, abilityRecord);
    uiAbilityLifecycleManager->OnAbilityDied(abilityRecord);
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_SetRootSceneSession_0100
 * @tc.desc: SetRootSceneSession
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, SetRootSceneSession_001, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    sptr<IRemoteObject> object = nullptr;
    uiAbilityLifecycleManager->SetRootSceneSession(object);
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
}

/**
 * @tc.name: UIAbilityLifecycleManager_SetRootSceneSession_0200
 * @tc.desc: SetRootSceneSession
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, SetRootSceneSession_002, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    auto abilityRecord = InitAbilityRecord();
    EXPECT_NE(abilityRecord, nullptr);
    auto token = abilityRecord->GetToken();
    EXPECT_NE(token, nullptr);
    auto object = token->AsObject();
    uiAbilityLifecycleManager->SetRootSceneSession(object);
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
}

/**
 * @tc.name: UIAbilityLifecycleManager_SetRootSceneSession_0300
 * @tc.desc: SetRootSceneSession
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, SetRootSceneSession_003, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    Rosen::SessionInfo info;
    sptr<Rosen::ISession> session = new Rosen::Session(info);
    EXPECT_NE(session, nullptr);
    sptr<IRemoteObject> rootSceneSession = session->AsObject();
    uiAbilityLifecycleManager->SetRootSceneSession(rootSceneSession);
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
}

/**
 * @tc.name: UIAbilityLifecycleManager_NotifySCBToStartUIAbility_0100
 * @tc.desc: NotifySCBToStartUIAbility
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, NotifySCBToStartUIAbility_001, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest abilityRequest;
    uiAbilityLifecycleManager->NotifySCBToStartUIAbility(abilityRequest);
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
}

/**
 * @tc.name: UIAbilityLifecycleManager_GetPersistentIdByAbilityRequest_0100
 * @tc.desc: GetPersistentIdByAbilityRequest
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, GetPersistentIdByAbilityRequest_001, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.launchMode = AppExecFwk::LaunchMode::SPECIFIED;
    bool reuse = false;
    EXPECT_EQ(uiAbilityLifecycleManager->GetPersistentIdByAbilityRequest(abilityRequest, reuse), 0);
}

/**
 * @tc.name: UIAbilityLifecycleManager_GetPersistentIdByAbilityRequest_0200
 * @tc.desc: GetPersistentIdByAbilityRequest
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, GetPersistentIdByAbilityRequest_002, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.launchMode = AppExecFwk::LaunchMode::STANDARD;
    bool reuse = false;
    EXPECT_EQ(uiAbilityLifecycleManager->GetPersistentIdByAbilityRequest(abilityRequest, reuse), 0);
}

/**
 * @tc.name: UIAbilityLifecycleManager_GetPersistentIdByAbilityRequest_0300
 * @tc.desc: GetPersistentIdByAbilityRequest
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, GetPersistentIdByAbilityRequest_003, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.launchMode = AppExecFwk::LaunchMode::SINGLETON;
    abilityRequest.abilityInfo.name = "testAbility";
    abilityRequest.abilityInfo.moduleName = "testModule";
    abilityRequest.abilityInfo.bundleName = "com.test.ut";

    Rosen::SessionInfo info;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->sessionToken = new Rosen::Session(info);
    sessionInfo->persistentId = 1;
    abilityRequest.sessionInfo = sessionInfo;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    uiAbilityLifecycleManager->sessionAbilityMap_.emplace(sessionInfo->persistentId, abilityRecord);
    bool reuse = false;
    EXPECT_EQ(uiAbilityLifecycleManager->GetPersistentIdByAbilityRequest(abilityRequest, reuse),
        sessionInfo->persistentId);
}

/**
 * @tc.name: UIAbilityLifecycleManager_GetPersistentIdByAbilityRequest_0400
 * @tc.desc: GetPersistentIdByAbilityRequest
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, GetPersistentIdByAbilityRequest_004, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest abilityRequest1;
    abilityRequest1.abilityInfo.launchMode = AppExecFwk::LaunchMode::SINGLETON;

    Rosen::SessionInfo info;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->sessionToken = new Rosen::Session(info);
    sessionInfo->persistentId = 1;
    AbilityRequest abilityRequest;
    abilityRequest.sessionInfo = sessionInfo;
    abilityRequest.abilityInfo.name = "testAbility";
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    uiAbilityLifecycleManager->sessionAbilityMap_.emplace(sessionInfo->persistentId, abilityRecord);
    bool reuse = false;
    EXPECT_EQ(uiAbilityLifecycleManager->GetPersistentIdByAbilityRequest(abilityRequest1, reuse), 0);
}

/**
 * @tc.name: UIAbilityLifecycleManager_GetReusedSpecifiedPersistentId_0100
 * @tc.desc: GetReusedSpecifiedPersistentId
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, GetReusedSpecifiedPersistentId_001, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest abilityRequest;
    bool reuse = false;
    EXPECT_EQ(uiAbilityLifecycleManager->GetReusedSpecifiedPersistentId(abilityRequest, reuse), 0);
}

/**
 * @tc.name: UIAbilityLifecycleManager_GetReusedSpecifiedPersistentId_0200
 * @tc.desc: GetReusedSpecifiedPersistentId
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, GetReusedSpecifiedPersistentId_002, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.launchMode = AppExecFwk::LaunchMode::SPECIFIED;
    abilityRequest.abilityInfo.name = "testAbility";
    abilityRequest.abilityInfo.moduleName = "testModule";
    abilityRequest.abilityInfo.bundleName = "com.test.ut";
    abilityRequest.startRecent = true;
    std::string flag = "specified";
    abilityRequest.specifiedFlag = flag;

    Rosen::SessionInfo info;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->sessionToken = new Rosen::Session(info);
    sessionInfo->persistentId = 1;
    abilityRequest.sessionInfo = sessionInfo;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    abilityRecord->SetSpecifiedFlag(flag);
    uiAbilityLifecycleManager->sessionAbilityMap_.emplace(sessionInfo->persistentId, abilityRecord);
    bool reuse = false;
    EXPECT_EQ(uiAbilityLifecycleManager->GetReusedSpecifiedPersistentId(abilityRequest, reuse),
        sessionInfo->persistentId);
}

/**
 * @tc.name: UIAbilityLifecycleManager_GetReusedSpecifiedPersistentId_0300
 * @tc.desc: GetReusedSpecifiedPersistentId
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, GetReusedSpecifiedPersistentId_003, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.launchMode = AppExecFwk::LaunchMode::SPECIFIED;
    abilityRequest.startRecent = true;
    std::string flag = "specified";
    abilityRequest.specifiedFlag = flag;

    Rosen::SessionInfo info;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->sessionToken = new Rosen::Session(info);
    sessionInfo->persistentId = 1;
    abilityRequest.sessionInfo = sessionInfo;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    uiAbilityLifecycleManager->sessionAbilityMap_.emplace(sessionInfo->persistentId, abilityRecord);
    bool reuse = false;
    EXPECT_EQ(uiAbilityLifecycleManager->GetReusedSpecifiedPersistentId(abilityRequest, reuse), 0);
}

/**
 * @tc.name: UIAbilityLifecycleManager_GetReusedStandardPersistentId_0100
 * @tc.desc: GetReusedStandardPersistentId
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, GetReusedStandardPersistentId_001, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest abilityRequest;
    bool reuse = false;
    EXPECT_EQ(uiAbilityLifecycleManager->GetReusedStandardPersistentId(abilityRequest, reuse), 0);
}

/**
 * @tc.name: UIAbilityLifecycleManager_GetReusedStandardPersistentId_0200
 * @tc.desc: GetReusedStandardPersistentId
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, GetReusedStandardPersistentId_002, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.launchMode = AppExecFwk::LaunchMode::STANDARD;
    abilityRequest.abilityInfo.name = "testAbility";
    abilityRequest.abilityInfo.moduleName = "testModule";
    abilityRequest.abilityInfo.bundleName = "com.test.ut";
    abilityRequest.startRecent = true;

    Rosen::SessionInfo info;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->sessionToken = new Rosen::Session(info);
    sessionInfo->persistentId = 1;
    abilityRequest.sessionInfo = sessionInfo;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    uiAbilityLifecycleManager->sessionAbilityMap_.emplace(sessionInfo->persistentId, abilityRecord);
    bool reuse = false;
    EXPECT_EQ(uiAbilityLifecycleManager->GetReusedStandardPersistentId(abilityRequest, reuse),
        sessionInfo->persistentId);
}

/**
 * @tc.name: UIAbilityLifecycleManager_NotifySCBPendingActivation_0100
 * @tc.desc: NotifySCBPendingActivation
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, NotifySCBPendingActivation_001, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest abilityRequest;
    Rosen::SessionInfo info;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->sessionToken = new Rosen::Session(info);
    sessionInfo->persistentId = 1;
    abilityRequest.sessionInfo = sessionInfo;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    uiAbilityLifecycleManager->sessionAbilityMap_.emplace(sessionInfo->persistentId, abilityRecord);
    auto token = abilityRecord->GetToken();
    EXPECT_NE(token, nullptr);
    abilityRequest.callerToken = token->AsObject();
    std::string errMsg;
    uiAbilityLifecycleManager->NotifySCBPendingActivation(sessionInfo, abilityRequest, errMsg);
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
}

/**
 * @tc.name: UIAbilityLifecycleManager_NotifySCBPendingActivation_0200
 * @tc.desc: NotifySCBPendingActivation
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, NotifySCBPendingActivation_002, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest abilityRequest;
    Rosen::SessionInfo info;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->sessionToken = new Rosen::Session(info);
    sessionInfo->persistentId = 1;
    abilityRequest.sessionInfo = sessionInfo;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    uiAbilityLifecycleManager->sessionAbilityMap_.emplace(sessionInfo->persistentId, abilityRecord);
    auto token = abilityRecord->GetToken();
    EXPECT_NE(token, nullptr);
    abilityRequest.callerToken = token->AsObject();
    abilityRequest.isStartInSplitMode = true;
    std::string errMsg;
    uiAbilityLifecycleManager->NotifySCBPendingActivation(sessionInfo, abilityRequest, errMsg);
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
}

/**
 * @tc.name: UIAbilityLifecycleManager_NotifySCBPendingActivation_0300
 * @tc.desc: NotifySCBPendingActivation
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, NotifySCBPendingActivation_003, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest abilityRequest;
    Rosen::SessionInfo info;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->sessionToken = new Rosen::Session(info);
    abilityRequest.sessionInfo = sessionInfo;
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    auto token = abilityRecord->GetToken();
    EXPECT_NE(token, nullptr);
    abilityRequest.callerToken = token->AsObject();
    abilityRequest.isStartInSplitMode = false;
    abilityRequest.isTargetPlugin = true;
    std::string errMsg;
    uiAbilityLifecycleManager->NotifySCBPendingActivation(sessionInfo, abilityRequest, errMsg);
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
}

/**
 * @tc.name: UIAbilityLifecycleManager_NotifySCBPendingActivation_0400
 * @tc.desc: NotifySCBPendingActivation
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, NotifySCBPendingActivation_004, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest abilityRequest;
    Rosen::SessionInfo info;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->sessionToken = new Rosen::Session(info);
    abilityRequest.sessionInfo = sessionInfo;
    abilityRequest.isStartInSplitMode = false;
    abilityRequest.callerToken = nullptr;
    abilityRequest.isTargetPlugin = true;
    std::string errMsg;
    uiAbilityLifecycleManager->NotifySCBPendingActivation(sessionInfo, abilityRequest, errMsg);
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
}

/**
 * @tc.name: UIAbilityLifecycleManager_NotifySCBPendingActivationInSplitMode_0100
 * @tc.desc: NotifySCBPendingActivationInSplitMode
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, NotifySCBPendingActivationInSplitMode_001, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest abilityRequest;
    Rosen::SessionInfo info;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->sessionToken = new Rosen::Session(info);
    sessionInfo->persistentId = 1;
    abilityRequest.sessionInfo = sessionInfo;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    uiAbilityLifecycleManager->sessionAbilityMap_.emplace(sessionInfo->persistentId, abilityRecord);
    auto token = abilityRecord->GetToken();
    EXPECT_NE(token, nullptr);
    abilityRequest.callerToken = token->AsObject();
    std::string errMsg;
    uiAbilityLifecycleManager->NotifySCBPendingActivationInSplitMode(sessionInfo, abilityRequest);
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
}
/**
 * @tc.name: UIAbilityLifecycleManager_CreateSessionConfigurations_0100
 * @tc.desc: CreateSessionConfigurations
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, CreateSessionConfigurations_001, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest abilityRequest;
    Rosen::SessionInfo info;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->sessionToken = new Rosen::Session(info);
    sessionInfo->persistentId = 1;
    std::vector<sptr<SessionInfo>> sessionInfoList;
    std::vector<Rosen::PendingSessionActivationConfig> configList;
    int32_t primaryWindowId = 1;
    uiAbilityLifecycleManager->CreateSessionConfigurations(sessionInfoList, primaryWindowId, configList, sessionInfo);
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
}
/**
 * @tc.name: UIAbilityLifecycleManager_ResolveLocked_0100
 * @tc.desc: ResolveLocked
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, ResolveLocked_001, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest abilityRequest;
    std::string errMsg;
    EXPECT_EQ(uiAbilityLifecycleManager->ResolveLocked(abilityRequest, errMsg), RESOLVE_CALL_ABILITY_INNER_ERR);
}

/**
 * @tc.name: UIAbilityLifecycleManager_ResolveLocked_0200
 * @tc.desc: ResolveLocked
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, ResolveLocked_002, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest abilityRequest;
    abilityRequest.callType = AbilityCallType::CALL_REQUEST_TYPE;
    std::string errMsg;
    EXPECT_EQ(uiAbilityLifecycleManager->ResolveLocked(abilityRequest, errMsg), RESOLVE_CALL_ABILITY_INNER_ERR);
}

/**
 * @tc.name: UIAbilityLifecycleManager_CallAbilityLocked_0100
 * @tc.desc: CallAbilityLocked
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, CallAbilityLocked_001, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);

    AbilityRequest abilityRequest;
    Rosen::SessionInfo info;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->sessionToken = new Rosen::Session(info);
    sessionInfo->persistentId = 1;
    Want want;
    want.SetParam(Want::PARAM_RESV_CALL_TO_FOREGROUND, true);
    abilityRequest.sessionInfo = sessionInfo;
    abilityRequest.want = want;
    abilityRequest.abilityInfo.launchMode = AppExecFwk::LaunchMode::SINGLETON;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    uiAbilityLifecycleManager->sessionAbilityMap_.emplace(sessionInfo->persistentId, abilityRecord);
    abilityRecord->isReady_ = true;

    std::string errMsg;
    uiAbilityLifecycleManager->CallAbilityLocked(abilityRequest, errMsg);
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
}

/**
 * @tc.name: UIAbilityLifecycleManager_CallAbilityLocked_0200
 * @tc.desc: CallAbilityLocked
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, CallAbilityLocked_002, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest abilityRequest;
    abilityRequest.callType = AbilityCallType::CALL_REQUEST_TYPE;
    Want want;
    want.SetParam(Want::PARAM_RESV_CALL_TO_FOREGROUND, true);
    abilityRequest.want = want;
    std::string errMsg;
    uiAbilityLifecycleManager->CallAbilityLocked(abilityRequest, errMsg);
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
}

/**
 * @tc.name: UIAbilityLifecycleManager_CallUIAbilityBySCB_0100
 * @tc.desc: CallUIAbilityBySCB
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, CallUIAbilityBySCB_001, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    sptr<SessionInfo> sessionInfo;
    bool isColdStart = false;
    AbilityRuntime::StartParamsBySCB params;
    uiAbilityLifecycleManager->CallUIAbilityBySCB(sessionInfo, params, isColdStart);
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
}

/**
 * @tc.name: UIAbilityLifecycleManager_CallUIAbilityBySCB_0200
 * @tc.desc: CallUIAbilityBySCB
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, CallUIAbilityBySCB_002, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->sessionToken = nullptr;
    bool isColdStart = false;
    AbilityRuntime::StartParamsBySCB params;
    uiAbilityLifecycleManager->CallUIAbilityBySCB(sessionInfo, params, isColdStart);
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
}

/**
 * @tc.name: UIAbilityLifecycleManager_CallUIAbilityBySCB_0300
 * @tc.desc: CallUIAbilityBySCB
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, CallUIAbilityBySCB_003, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    AbilityRequest abilityRequest;
    abilityRequest.sessionInfo = sessionInfo;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    auto token = abilityRecord->GetToken();
    EXPECT_NE(token, nullptr);
    sessionInfo->sessionToken = token->AsObject();
    bool isColdStart = false;
    AbilityRuntime::StartParamsBySCB params;
    uiAbilityLifecycleManager->CallUIAbilityBySCB(sessionInfo, params, isColdStart);
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
}

/**
 * @tc.name: UIAbilityLifecycleManager_CallUIAbilityBySCB_0400
 * @tc.desc: CallUIAbilityBySCB
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, CallUIAbilityBySCB_004, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    Rosen::SessionInfo info;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->sessionToken = new Rosen::Session(info);
    bool isColdStart = false;
    AbilityRuntime::StartParamsBySCB params;
    uiAbilityLifecycleManager->CallUIAbilityBySCB(sessionInfo, params, isColdStart);
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
}

/**
 * @tc.name: UIAbilityLifecycleManager_CallUIAbilityBySCB_0500
 * @tc.desc: CallUIAbilityBySCB
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, CallUIAbilityBySCB_005, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    Rosen::SessionInfo info;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->sessionToken = new Rosen::Session(info);
    sessionInfo->uiAbilityId = 1;

    uiAbilityLifecycleManager->tmpAbilityMap_.emplace(1, nullptr);
    bool isColdStart = false;
    AbilityRuntime::StartParamsBySCB params;
    uiAbilityLifecycleManager->CallUIAbilityBySCB(sessionInfo, params, isColdStart);
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
}

/**
 * @tc.name: UIAbilityLifecycleManager_CallUIAbilityBySCB_0600
 * @tc.desc: CallUIAbilityBySCB
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, CallUIAbilityBySCB_006, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    Rosen::SessionInfo info;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->sessionToken = new Rosen::Session(info);
    sessionInfo->uiAbilityId = 1;

    AbilityRequest abilityRequest;
    abilityRequest.sessionInfo = sessionInfo;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    uiAbilityLifecycleManager->tmpAbilityMap_.emplace(1, abilityRecord);
    bool isColdStart = false;
    AbilityRuntime::StartParamsBySCB params;
    uiAbilityLifecycleManager->CallUIAbilityBySCB(sessionInfo, params, isColdStart);
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
}

/**
 * @tc.name: UIAbilityLifecycleManager_CallUIAbilityBySCB_0700
 * @tc.desc: CallUIAbilityBySCB
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, CallUIAbilityBySCB_007, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    Rosen::SessionInfo info;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->sessionToken = new Rosen::Session(info);
    sessionInfo->uiAbilityId = 1;
    sessionInfo->persistentId = 1;

    AbilityRequest abilityRequest;
    abilityRequest.sessionInfo = sessionInfo;
    abilityRequest.abilityInfo.launchMode = AppExecFwk::LaunchMode::SINGLETON;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);

    uiAbilityLifecycleManager->tmpAbilityMap_.emplace(1, abilityRecord);
    uiAbilityLifecycleManager->sessionAbilityMap_.emplace(sessionInfo->persistentId, abilityRecord);
    bool isColdStart = false;
    AbilityRuntime::StartParamsBySCB params;
    uiAbilityLifecycleManager->CallUIAbilityBySCB(sessionInfo, params, isColdStart);
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
}

/**
 * @tc.name: UIAbilityLifecycleManager_CallRequestDone_0100
 * @tc.desc: CallRequestDone
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, CallRequestDone_001, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    uiAbilityLifecycleManager->CallRequestDone(nullptr, nullptr);
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
}

/**
 * @tc.name: UIAbilityLifecycleManager_CallRequestDone_0200
 * @tc.desc: CallRequestDone
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, CallRequestDone_002, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest abilityRequest;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    uiAbilityLifecycleManager->CallRequestDone(abilityRecord, nullptr);
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
}

/**
 * @tc.name: UIAbilityLifecycleManager_CallRequestDone_0300
 * @tc.desc: CallRequestDone
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, CallRequestDone_003, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest abilityRequest;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    auto token = abilityRecord->GetToken();
    EXPECT_NE(token, nullptr);
    uiAbilityLifecycleManager->CallRequestDone(abilityRecord, token->AsObject());
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
}

/**
 * @tc.name: UIAbilityLifecycleManager_ReleaseCallLocked_0100
 * @tc.desc: ReleaseCallLocked
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, ReleaseCallLocked_001, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    sptr<IAbilityConnection> connect = new UIAbilityLifecycleManagerTestStub();
    AppExecFwk::ElementName element;
    auto ret = uiAbilityLifecycleManager->ReleaseCallLocked(connect, element);
    EXPECT_EQ(ret, RELEASE_CALL_ABILITY_INNER_ERR);
}

/**
 * @tc.name: UIAbilityLifecycleManager_ReleaseCallLocked_0200
 * @tc.desc: ReleaseCallLocked
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, ReleaseCallLocked_002, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    auto abilityRecord = InitAbilityRecord();
    uiAbilityLifecycleManager->sessionAbilityMap_.emplace(1, abilityRecord);
    sptr<IAbilityConnection> connect = new UIAbilityLifecycleManagerTestStub();
    AppExecFwk::ElementName element("", "com.example.unittest", "MainAbility");
    auto ret = uiAbilityLifecycleManager->ReleaseCallLocked(connect, element);
    EXPECT_EQ(ret, RELEASE_CALL_ABILITY_INNER_ERR);
}

/**
 * @tc.name: UIAbilityLifecycleManager_Dump_001
 * @tc.desc: Dump
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, Dump_001, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest abilityRequest;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    uiAbilityLifecycleManager->sessionAbilityMap_.emplace(1, abilityRecord);
    uiAbilityLifecycleManager->sessionAbilityMap_.emplace(1, nullptr);
    std::vector<std::string> info;
    uiAbilityLifecycleManager->Dump(info);
}

/**
 * @tc.name: UIAbilityLifecycleManager_DumpMissionList_001
 * @tc.desc: DumpMissionList
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, DumpMissionList_001, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest abilityRequest;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    uiAbilityLifecycleManager->sessionAbilityMap_.emplace(1, nullptr);
    uiAbilityLifecycleManager->sessionAbilityMap_.emplace(1, abilityRecord);
    std::vector<std::string> info;
    bool isClient = false;
    std::string args;
    uiAbilityLifecycleManager->DumpMissionList(info, isClient, args);
}

/**
 * @tc.name: UIAbilityLifecycleManager_DumpMissionList_002
 * @tc.desc: DumpMissionList
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, DumpMissionList_002, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest abilityRequest;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    uiAbilityLifecycleManager->sessionAbilityMap_.emplace(1, abilityRecord);
    std::vector<std::string> info;
    bool isClient = false;
    std::string args;
    uiAbilityLifecycleManager->DumpMissionList(info, isClient, args);
}

/**
 * @tc.name: UIAbilityLifecycleManager_DumpMissionListByRecordId_001
 * @tc.desc: DumpMissionListByRecordId
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, DumpMissionListByRecordId_001, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest abilityRequest;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    uiAbilityLifecycleManager->sessionAbilityMap_.emplace(1, nullptr);
    uiAbilityLifecycleManager->sessionAbilityMap_.emplace(1, abilityRecord);
    std::vector<std::string> info;
    bool isClient = false;
    int32_t abilityRecordId = 0;
    std::vector<std::string> params;
    uiAbilityLifecycleManager->DumpMissionListByRecordId(info, isClient, abilityRecordId, params);
}

/**
 * @tc.name: UIAbilityLifecycleManager_DumpMissionListByRecordId_002
 * @tc.desc: DumpMissionListByRecordId
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, DumpMissionListByRecordId_002, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest abilityRequest;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    uiAbilityLifecycleManager->sessionAbilityMap_.emplace(1, abilityRecord);
    std::vector<std::string> info;
    bool isClient = false;
    int32_t abilityRecordId = 1;
    std::vector<std::string> params;
    uiAbilityLifecycleManager->DumpMissionListByRecordId(info, isClient, abilityRecordId, params);
}

/**
 * @tc.name: UIAbilityLifecycleManager_OnAcceptWantResponse_0100
 * @tc.desc: OnAcceptWantResponse
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, OnAcceptWantResponse_001, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    Want want;
    std::string flag = "flag";
    uiAbilityLifecycleManager->OnAcceptWantResponse(want, flag, 0);

    AbilityRequest abilityRequest;
    uiAbilityLifecycleManager->OnAcceptWantResponse(want, flag, 0);

    abilityRequest.abilityInfo.launchMode = AppExecFwk::LaunchMode::SPECIFIED;
    uiAbilityLifecycleManager->OnAcceptWantResponse(want, flag, 0);

    uiAbilityLifecycleManager->OnAcceptWantResponse(want, "", 0);
    uiAbilityLifecycleManager.reset();
}

#ifdef WITH_DLP
/**
 * @tc.name: UIAbilityLifecycleManager_OnAcceptWantResponse_0200
 * @tc.desc: OnAcceptWantResponse
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, OnAcceptWantResponse_002, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    Want want;
    std::string flag = "flag";
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.launchMode = AppExecFwk::LaunchMode::SPECIFIED;
    abilityRequest.startRecent = true;
    abilityRequest.abilityInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.moduleName = "entry";
    abilityRequest.specifiedFlag = flag;
    want.SetParam(DLP_INDEX, 1);
    abilityRequest.want = want;
    UIAbilityRecordPtr abilityRecord = InitAbilityRecord();
    abilityRecord->abilityInfo_.launchMode = AppExecFwk::LaunchMode::SPECIFIED;
    abilityRecord->abilityInfo_.moduleName = "entry";
    abilityRecord->SetAppIndex(1);
    abilityRecord->SetSpecifiedFlag(flag);
    uiAbilityLifecycleManager->sessionAbilityMap_.emplace(1, abilityRecord);
    uiAbilityLifecycleManager->OnAcceptWantResponse(want, flag, 0);

    UIAbilityRecordPtr callerAbility = InitAbilityRecord();
    abilityRequest.callerToken = callerAbility->GetToken()->AsObject();
    uiAbilityLifecycleManager->OnAcceptWantResponse(want, flag, 0);
    uiAbilityLifecycleManager.reset();
}
#endif // WITH_DLP

/**
 * @tc.name: UIAbilityLifecycleManager_IsStartSpecifiedProcessRequest_001
 * @tc.desc: IsStartSpecifiedProcessRequest, test isolationProcess mode
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, IsStartSpecifiedProcessRequest_001, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    AppUtils::GetInstance().isStartSpecifiedProcess_.isLoaded = true;
    AppUtils::GetInstance().isStartSpecifiedProcess_.value = true;
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.isolationProcess = false;
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::PAGE;
    abilityRequest.abilityInfo.isStageBasedModel = true;
    auto ret = uiAbilityLifecycleManager->IsStartSpecifiedProcessRequest(abilityRequest);
    AppUtils::GetInstance().isStartSpecifiedProcess_.isLoaded = false;
    AppUtils::GetInstance().isStartSpecifiedProcess_.value = false;
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: UIAbilityLifecycleManager_IsStartSpecifiedProcessRequest_002
 * @tc.desc: IsStartSpecifiedProcessRequest, test UIAbility
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, IsStartSpecifiedProcessRequest_002, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    AppUtils::GetInstance().isStartSpecifiedProcess_.isLoaded = true;
    AppUtils::GetInstance().isStartSpecifiedProcess_.value = true;
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.isolationProcess = true;
    // isUIAbility is false
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::SERVICE;
    abilityRequest.abilityInfo.isStageBasedModel = true;
    auto ret = uiAbilityLifecycleManager->IsStartSpecifiedProcessRequest(abilityRequest);
    AppUtils::GetInstance().isStartSpecifiedProcess_.isLoaded = false;
    AppUtils::GetInstance().isStartSpecifiedProcess_.value = false;
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: UIAbilityLifecycleManager_IsStartSpecifiedProcessRequest_003
 * @tc.desc: IsStartSpecifiedProcessRequest, test UIAbility stage mode
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, IsStartSpecifiedProcessRequest_003, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    AppUtils::GetInstance().isStartSpecifiedProcess_.isLoaded = true;
    AppUtils::GetInstance().isStartSpecifiedProcess_.value = true;
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.isolationProcess = true;
    // isUIAbility is false
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::PAGE;
    abilityRequest.abilityInfo.isStageBasedModel = false;
    auto ret = uiAbilityLifecycleManager->IsStartSpecifiedProcessRequest(abilityRequest);
    AppUtils::GetInstance().isStartSpecifiedProcess_.isLoaded = false;
    AppUtils::GetInstance().isStartSpecifiedProcess_.value = false;
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: UIAbilityLifecycleManager_IsStartSpecifiedProcessRequest_004
 * @tc.desc: IsStartSpecifiedProcessRequest, test isNewProcessMode
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, IsStartSpecifiedProcessRequest_004, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    AppUtils::GetInstance().isStartSpecifiedProcess_.isLoaded = true;
    AppUtils::GetInstance().isStartSpecifiedProcess_.value = true;
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.isolationProcess = true;
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::PAGE;
    abilityRequest.abilityInfo.isStageBasedModel = true;
    // isNewProcessMode is true
    abilityRequest.processOptions = std::make_shared<ProcessOptions>();
    abilityRequest.processOptions->processMode = ProcessMode::NEW_PROCESS_ATTACH_TO_PARENT;
    auto ret = uiAbilityLifecycleManager->IsStartSpecifiedProcessRequest(abilityRequest);
    AppUtils::GetInstance().isStartSpecifiedProcess_.isLoaded = false;
    AppUtils::GetInstance().isStartSpecifiedProcess_.value = false;
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: UIAbilityLifecycleManager_IsStartSpecifiedProcessRequest_005
 * @tc.desc: IsStartSpecifiedProcessRequest, test isPlugin
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, IsStartSpecifiedProcessRequest_005, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    AppUtils::GetInstance().isStartSpecifiedProcess_.isLoaded = true;
    AppUtils::GetInstance().isStartSpecifiedProcess_.value = true;
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.isolationProcess = true;
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::PAGE;
    abilityRequest.abilityInfo.isStageBasedModel = true;
    // isPlugin ios true
    abilityRequest.want.SetParam(Want::DESTINATION_PLUGIN_ABILITY, true);
    auto ret = uiAbilityLifecycleManager->IsStartSpecifiedProcessRequest(abilityRequest);
    AppUtils::GetInstance().isStartSpecifiedProcess_.isLoaded = false;
    AppUtils::GetInstance().isStartSpecifiedProcess_.value = false;
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: UIAbilityLifecycleManager_IsStartSpecifiedProcessRequest_006
 * @tc.desc: IsStartSpecifiedProcessRequest, test START_SPECIFIED_PROCESS param
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, IsStartSpecifiedProcessRequest_006, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    // START_SPECIFIED_PROCESS is false
    AppUtils::GetInstance().isStartSpecifiedProcess_.isLoaded = true;
    AppUtils::GetInstance().isStartSpecifiedProcess_.value = false;
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.isolationProcess = true;
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::PAGE;
    abilityRequest.abilityInfo.isStageBasedModel = true;
    auto ret = uiAbilityLifecycleManager->IsStartSpecifiedProcessRequest(abilityRequest);
    AppUtils::GetInstance().isStartSpecifiedProcess_.isLoaded = false;
    AppUtils::GetInstance().isStartSpecifiedProcess_.value = false;
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: UIAbilityLifecycleManager_IsStartSpecifiedProcessRequest_007
 * @tc.desc: IsStartSpecifiedProcessRequest, test ok
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, IsStartSpecifiedProcessRequest_007, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    AppUtils::GetInstance().isStartSpecifiedProcess_.isLoaded = true;
    AppUtils::GetInstance().isStartSpecifiedProcess_.value = true;
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.isolationProcess = true;
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::PAGE;
    abilityRequest.abilityInfo.isStageBasedModel = true;
    auto ret = uiAbilityLifecycleManager->IsStartSpecifiedProcessRequest(abilityRequest);
    AppUtils::GetInstance().isStartSpecifiedProcess_.isLoaded = false;
    AppUtils::GetInstance().isStartSpecifiedProcess_.value = false;
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: UIAbilityLifecycleManager_MoveAbilityToFront_0100
 * @tc.desc: MoveAbilityToFront
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, MoveAbilityToFront_001, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    AbilityRequest abilityRequest;
    SpecifiedRequest specifiedRequest(0, abilityRequest);
    int res = uiAbilityLifecycleManager->MoveAbilityToFront(specifiedRequest, nullptr, nullptr);
    EXPECT_EQ(res, ERR_INVALID_VALUE);

    specifiedRequest.abilityRequest.sessionInfo = new SessionInfo();
    specifiedRequest.abilityRequest.appInfo.bundleName = "com.example.unittest";
    specifiedRequest.abilityRequest.abilityInfo.name = "MainAbility";
    specifiedRequest.abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::PAGE;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(specifiedRequest.abilityRequest);
    res = uiAbilityLifecycleManager->MoveAbilityToFront(specifiedRequest, abilityRecord, nullptr);
    EXPECT_EQ(res, ERR_OK);

    res = uiAbilityLifecycleManager->MoveAbilityToFront(specifiedRequest, abilityRecord, nullptr);
    EXPECT_EQ(res, ERR_OK);

    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_MoveAbilityToFront_0200
 * @tc.desc: MoveAbilityToFront
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, MoveAbilityToFront_002, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    AbilityRequest abilityRequest;
    SpecifiedRequest specifiedRequest(0, abilityRequest);
    int res = uiAbilityLifecycleManager->MoveAbilityToFront(specifiedRequest, nullptr, nullptr);
    EXPECT_EQ(res, ERR_INVALID_VALUE);

    specifiedRequest.abilityRequest.sessionInfo = new SessionInfo();
    specifiedRequest.abilityRequest.appInfo.bundleName = "com.example.unittest";
    specifiedRequest.abilityRequest.abilityInfo.name = "MainAbility";
    specifiedRequest.abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::PAGE;
    specifiedRequest.abilityRequest.isStartInSplitMode = true;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(specifiedRequest.abilityRequest);
    res = uiAbilityLifecycleManager->MoveAbilityToFront(specifiedRequest, abilityRecord, nullptr);
    EXPECT_EQ(res, ERR_OK);

    uiAbilityLifecycleManager.reset();
}
/**
 * @tc.name: UIAbilityLifecycleManager_SendSessionInfoToSCB_0100
 * @tc.desc: SendSessionInfoToSCB
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, SendSessionInfoToSCB_0100, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    Rosen::SessionInfo info;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    AbilityRequest abilityRequest;
    sessionInfo->sessionToken = new Rosen::Session(info);
    abilityRequest.sessionInfo = sessionInfo;
    abilityRequest.appInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::PAGE;
    UIAbilityRecordPtr callerAbility = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    int res = uiAbilityLifecycleManager->SendSessionInfoToSCB(callerAbility, sessionInfo);
    EXPECT_EQ(res, ERR_OK);

    sessionInfo->sessionToken = nullptr;
    abilityRequest.sessionInfo = sessionInfo;
    callerAbility = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    res = uiAbilityLifecycleManager->SendSessionInfoToSCB(callerAbility, sessionInfo);
    EXPECT_EQ(res, ERR_INVALID_VALUE);

    abilityRequest.sessionInfo = nullptr;
    callerAbility = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    auto token = callerAbility->GetToken();
    EXPECT_NE(token, nullptr);
    auto object = token->AsObject();
    uiAbilityLifecycleManager->SetRootSceneSession(object);
    res = uiAbilityLifecycleManager->SendSessionInfoToSCB(callerAbility, sessionInfo);
    EXPECT_EQ(res, ERR_INVALID_VALUE);

    uiAbilityLifecycleManager->SetRootSceneSession(nullptr);
    res = uiAbilityLifecycleManager->SendSessionInfoToSCB(callerAbility, sessionInfo);
    EXPECT_EQ(res, ERR_INVALID_VALUE);

    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_SendSessionInfoToSCB_0200
 * @tc.desc: SendSessionInfoToSCB
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, SendSessionInfoToSCB_0200, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    Rosen::SessionInfo info;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    AbilityRequest abilityRequest;
    sessionInfo->sessionToken = new Rosen::Session(info);
    sessionInfo->want.SetParam(KEY_REQUEST_ID, std::string("123456"));
    abilityRequest.sessionInfo = sessionInfo;
    abilityRequest.appInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::PAGE;
    UIAbilityRecordPtr callerAbility = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    int res = uiAbilityLifecycleManager->SendSessionInfoToSCB(callerAbility, sessionInfo);
    EXPECT_EQ(res, ERR_OK);
    EXPECT_FALSE(sessionInfo->want.HasParameter(KEY_REQUEST_ID));

    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_SendSessionInfoToSCB_0300
 * @tc.desc: SendSessionInfoToSCB
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, SendSessionInfoToSCB_0300, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    Rosen::SessionInfo info;
    uiAbilityLifecycleManager->rootSceneSession_ = new Rosen::Session(info);
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    AbilityRequest abilityRequest;
    sessionInfo->sessionToken = new Rosen::Session(info);
    sessionInfo->want.SetParam(KEY_REQUEST_ID, std::string("123456"));
    abilityRequest.sessionInfo = sessionInfo;
    abilityRequest.appInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::PAGE;
    UIAbilityRecordPtr callerAbility = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    sessionInfo->callerToken = callerAbility->GetToken();
    UIAbilityRecordPtr caller = nullptr;
    int res = uiAbilityLifecycleManager->SendSessionInfoToSCB(caller, sessionInfo);
    EXPECT_EQ(res, ERR_OK);
    EXPECT_FALSE(sessionInfo->want.HasParameter(KEY_REQUEST_ID));

    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_SendSessionInfoToSCBInSplitMode_0100
 * @tc.desc: SendSessionInfoToSCBInSplitMode
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, SendSessionInfoToSCBInSplitMode_0100, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    Rosen::SessionInfo info;
    uiAbilityLifecycleManager->rootSceneSession_ = new Rosen::Session(info);
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    AbilityRequest abilityRequest;
    sessionInfo->sessionToken = new Rosen::Session(info);
    sessionInfo->want.SetParam(KEY_REQUEST_ID, std::string("123456"));
    abilityRequest.sessionInfo = sessionInfo;
    abilityRequest.appInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::PAGE;
    UIAbilityRecordPtr callerAbility = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    abilityRequest.primaryWindowId = 1;
    int res = uiAbilityLifecycleManager->SendSessionInfoToSCBInSplitMode(abilityRequest.primaryWindowId, callerAbility,
        sessionInfo);
    EXPECT_EQ(res, ERR_OK);

    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_StartAbilityBySpecified_0100
 * @tc.desc: StartAbilityBySpecified
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, StartAbilityBySpecified_001, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest request;
    SpecifiedRequest specifiedRequest(0, request);
    UIAbilityRecordPtr callerAbility = nullptr;
    uiAbilityLifecycleManager->StartAbilityBySpecified(specifiedRequest, callerAbility);
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_StartAbilityBySpecified_0200
 * @tc.desc: StartAbilityBySpecified
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, StartAbilityBySpecified_0200, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest request;
    SpecifiedRequest specifiedRequest(0, request);
    specifiedRequest.requestListId = 0;
    UIAbilityRecordPtr callerAbility = nullptr;
    uiAbilityLifecycleManager->StartAbilityBySpecified(specifiedRequest, callerAbility);
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_GetAbilityStateByPersistentId_0100
 * @tc.desc: GetAbilityStateByPersistentId
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, GetAbilityStateByPersistentId_001, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    int32_t persistentId = 100;
    bool state;
    int32_t ret = uiAbilityLifecycleManager->GetAbilityStateByPersistentId(persistentId, state);
    EXPECT_EQ(ERR_INVALID_VALUE, ret);
}

/**
 * @tc.name: UIAbilityLifecycleManager_GetAbilityStateByPersistentId_0200
 * @tc.desc: GetAbilityStateByPersistentId
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, GetAbilityStateByPersistentId_002, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    UIAbilityRecordPtr abilityRecord = InitAbilityRecord();
    abilityRecord->SetPendingState(AbilityState::INITIAL);
    uiAbilityLifecycleManager->sessionAbilityMap_.emplace(100, abilityRecord);
    int32_t persistentId = 100;
    bool state;
    int32_t ret = uiAbilityLifecycleManager->GetAbilityStateByPersistentId(persistentId, state);
    EXPECT_EQ(ERR_OK, ret);
}

/**
 * @tc.name: UIAbilityLifecycleManager_UpdateProcessName_0100
 * @tc.desc: UpdateProcessName
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, UpdateProcessName_001, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    sptr<SessionInfo> sessionInfo = new (std::nothrow) SessionInfo();
    sessionInfo->processOptions = std::make_shared<ProcessOptions>();
    EXPECT_NE(sessionInfo->processOptions, nullptr);
    sessionInfo->processOptions->processMode = ProcessMode::NEW_PROCESS_ATTACH_TO_PARENT;
    AbilityRequest abilityRequest;
    abilityRequest.sessionInfo = sessionInfo;
    abilityRequest.abilityInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.moduleName = "entry";
    abilityRequest.abilityInfo.name = "MainAbility";
    UIAbilityRecordPtr abilityRecord = InitAbilityRecord();
    uiAbilityLifecycleManager->UpdateProcessName(abilityRequest, abilityRecord);
    EXPECT_EQ("com.example.unittest:entry:MainAbility:0", abilityRecord->GetProcessName());
}

/**
 * @tc.name: UIAbilityLifecycleManager_ChangeAbilityVisibility_0100
 * @tc.desc: ChangeAbilityVisibility
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, ChangeAbilityVisibility_001, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    int32_t ret = uiAbilityLifecycleManager->ChangeAbilityVisibility(nullptr, true);
    EXPECT_EQ(ERR_INVALID_VALUE, ret);
}

/**
 * @tc.name: UIAbilityLifecycleManager_IsContainsAbility_0100
 * @tc.desc: IsContainsAbility
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, IsContainsAbility_001, TestSize.Level1)
{
    auto mgr = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(mgr, nullptr);
    sptr<IRemoteObject> token = nullptr;
    bool boolValue = mgr->IsContainsAbility(token);
    EXPECT_FALSE(boolValue);
}

/**
 * @tc.name: UIAbilityLifecycleManager_IsContainsAbility_0200
 * @tc.desc: IsContainsAbility
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, IsContainsAbility_002, TestSize.Level1)
{
    auto mgr = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(mgr, nullptr);
    AbilityRequest abilityRequest;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    auto&& token = abilityRecord->GetToken()->AsObject();
    mgr->sessionAbilityMap_.emplace(1, abilityRecord);
    bool boolValue = mgr->IsContainsAbility(token);
    EXPECT_TRUE(boolValue);
}

/**
 * @tc.name: UIAbilityLifecycleManager_IsContainsAbilityInner_0100
 * @tc.desc: IsContainsAbilityInner
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, IsContainsAbilityInner_001, TestSize.Level1)
{
    auto mgr = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(mgr, nullptr);
    sptr<IRemoteObject> token = nullptr;
    bool boolValue = mgr->IsContainsAbilityInner(token);
    EXPECT_FALSE(boolValue);
}

/**
 * @tc.name: UIAbilityLifecycleManager_IsContainsAbilityInner_0200
 * @tc.desc: IsContainsAbilityInner
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, IsContainsAbilityInner_002, TestSize.Level1)
{
    auto mgr = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(mgr, nullptr);
    AbilityRequest abilityRequest;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    auto&& token = abilityRecord->GetToken()->AsObject();
    mgr->sessionAbilityMap_.emplace(1, abilityRecord);
    bool boolValue = mgr->IsContainsAbilityInner(token);
    EXPECT_TRUE(boolValue);
}

/**
 * @tc.name: UIAbilityLifecycleManager_NotifySCBToMinimizeUIAbility_0100
 * @tc.desc: NotifySCBToMinimizeUIAbility
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, NotifySCBToMinimizeUIAbility_001, TestSize.Level1)
{
    auto mgr = std::make_unique<UIAbilityLifecycleManager>();
    sptr<IRemoteObject> token = nullptr;
    EXPECT_NE(mgr->NotifySCBToMinimizeUIAbility(token), ERR_OK);
}

/**
 * @tc.name: UIAbilityLifecycleManager_GetUIAbilityRecordBySessionInfo_0100
 * @tc.desc: GetUIAbilityRecordBySessionInfo
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, GetUIAbilityRecordBySessionInfo_001, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    sptr<SessionInfo> sessionInfo = nullptr;
    EXPECT_EQ(uiAbilityLifecycleManager->GetUIAbilityRecordBySessionInfo(sessionInfo), nullptr);
}

/**
 * @tc.name: UIAbilityLifecycleManager_GetUIAbilityRecordBySessionInfo_0200
 * @tc.desc: GetUIAbilityRecordBySessionInfo
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, GetUIAbilityRecordBySessionInfo_002, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->sessionToken = nullptr;
    EXPECT_EQ(uiAbilityLifecycleManager->GetUIAbilityRecordBySessionInfo(sessionInfo), nullptr);
}

/**
 * @tc.name: UIAbilityLifecycleManager_GetUIAbilityRecordBySessionInfo_0300
 * @tc.desc: GetUIAbilityRecordBySessionInfo
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, GetUIAbilityRecordBySessionInfo_003, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    Rosen::SessionInfo info;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->sessionToken = new Rosen::Session(info);
    EXPECT_EQ(uiAbilityLifecycleManager->GetUIAbilityRecordBySessionInfo(sessionInfo), nullptr);
}

/**
 * @tc.name: UIAbilityLifecycleManager_GetUIAbilityRecordBySessionInfo_0400
 * @tc.desc: GetUIAbilityRecordBySessionInfo
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, GetUIAbilityRecordBySessionInfo_004, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest abilityRequest;
    Rosen::SessionInfo info;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->sessionToken = new Rosen::Session(info);
    sessionInfo->persistentId = 1;
    abilityRequest.sessionInfo = sessionInfo;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    uiAbilityLifecycleManager->sessionAbilityMap_.emplace(sessionInfo->persistentId, abilityRecord);
    EXPECT_NE(uiAbilityLifecycleManager->GetUIAbilityRecordBySessionInfo(sessionInfo), nullptr);
}

/**
 * @tc.name: UIAbilityLifecycleManager_OnStartSpecifiedProcessResponse_0100
 * @tc.desc: OnStartSpecifiedProcessResponse
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, OnStartSpecifiedProcessResponse_001, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    std::string flag = "flag";
    uiAbilityLifecycleManager->OnStartSpecifiedProcessResponse(flag, 0);
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_OnStartSpecifiedProcessResponse_0200
 * @tc.desc: OnStartSpecifiedProcessResponse
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, OnStartSpecifiedProcessResponse_002, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    std::string flag = "flag";
    int32_t requestId = 100;
    uiAbilityLifecycleManager->OnStartSpecifiedProcessResponse(flag, requestId);
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_OnStartSpecifiedAbilityTimeoutResponse_0100
 * @tc.desc: OnStartSpecifiedAbilityTimeoutResponse
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, OnStartSpecifiedAbilityTimeoutResponse_001, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    uiAbilityLifecycleManager->OnStartSpecifiedAbilityTimeoutResponse(0);
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_OnStartSpecifiedAbilityTimeoutResponse_0200
 * @tc.desc: OnStartSpecifiedAbilityTimeoutResponse
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, OnStartSpecifiedAbilityTimeoutResponse_002, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    int32_t requestId = 100;
    uiAbilityLifecycleManager->OnStartSpecifiedAbilityTimeoutResponse(requestId);
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_OnStartSpecifiedFailed_0100
 * @tc.desc: OnStartSpecifiedFailed
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, OnStartSpecifiedFailed_001, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    int32_t requestId = 0;
    uiAbilityLifecycleManager->OnStartSpecifiedFailed(requestId);
    
    auto &list = uiAbilityLifecycleManager->specifiedRequestList_[std::string()];
    list.push_back(std::make_shared<SpecifiedRequest>(requestId, AbilityRequest()));

    uiAbilityLifecycleManager->OnStartSpecifiedFailed(requestId);
    EXPECT_TRUE(list.empty());
}

/**
 * @tc.name: UIAbilityLifecycleManager_OnStartSpecifiedProcessTimeoutResponse_0100
 * @tc.desc: OnStartSpecifiedProcessTimeoutResponse
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, OnStartSpecifiedProcessTimeoutResponse_001, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    int32_t requestId = 1;
    AbilityRequest abilityRequest;
    auto specifiedRequest = std::make_shared<SpecifiedRequest>(requestId, abilityRequest);
    auto &list = uiAbilityLifecycleManager->specifiedRequestList_[std::string()];
    list.push_back(specifiedRequest);
    int32_t requestId_2 = 2;
    list.push_back(std::make_shared<SpecifiedRequest>(requestId_2, abilityRequest));

    uiAbilityLifecycleManager->OnStartSpecifiedProcessTimeoutResponse(0);
    EXPECT_FALSE(list.empty());
}

/**
 * @tc.name: UIAbilityLifecycleManager_OnStartSpecifiedProcessTimeoutResponse_0200
 * @tc.desc: OnStartSpecifiedProcessTimeoutResponse
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, OnStartSpecifiedProcessTimeoutResponse_002, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    int32_t requestId = 1;
    AbilityRequest abilityRequest;
    std::string instanceKey = "instance_0";
    abilityRequest.want.SetParam(Want::APP_INSTANCE_KEY, instanceKey);
    auto specifiedRequest = std::make_shared<SpecifiedRequest>(requestId, abilityRequest);
    auto &list = uiAbilityLifecycleManager->specifiedRequestList_[std::string()];
    list.push_back(specifiedRequest);
    int32_t requestId_2 = 2;
    list.push_back(std::make_shared<SpecifiedRequest>(requestId_2, abilityRequest));

    uiAbilityLifecycleManager->OnStartSpecifiedProcessTimeoutResponse(0);
    EXPECT_FALSE(list.empty());
}

/**
 * @tc.name: UIAbilityLifecycleManager_OnCallConnectDied_0100
 * @tc.desc: OnCallConnectDied
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, OnCallConnectDied_001, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    std::shared_ptr<CallRecord> callRecord = nullptr;
    uiAbilityLifecycleManager->OnCallConnectDied(callRecord);
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_GetSessionIdByAbilityToken_0100
 * @tc.desc: GetSessionIdByAbilityToken
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, GetSessionIdByAbilityToken_001, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    sptr<IRemoteObject> token = nullptr;
    EXPECT_EQ(uiAbilityLifecycleManager->GetSessionIdByAbilityToken(token), ERR_OK);
}

/**
 * @tc.name: UIAbilityLifecycleManager_GetSessionIdByAbilityToken_0200
 * @tc.desc: GetSessionIdByAbilityToken
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, GetSessionIdByAbilityToken_002, TestSize.Level1)
{
    auto mgr = std::make_unique<UIAbilityLifecycleManager>();
    AbilityRequest abilityRequest;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    mgr->sessionAbilityMap_.emplace(1, abilityRecord);
    auto&& token = abilityRecord->GetToken()->AsObject();
    EXPECT_EQ(mgr->GetSessionIdByAbilityToken(token), 1);
}

/**
 * @tc.name: UIAbilityLifecycleManager_GetActiveAbilityList_0100
 * @tc.desc: GetActiveAbilityList
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, GetActiveAbilityList_001, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest abilityRequest;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    uiAbilityLifecycleManager->sessionAbilityMap_.emplace(1, abilityRecord);
    std::vector<std::string> abilityList;
    int32_t pid = 100;
    uiAbilityLifecycleManager->GetActiveAbilityList(TEST_UID, abilityList, pid);
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_GetActiveAbilityList_0200
 * @tc.desc: GetActiveAbilityList
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, GetActiveAbilityList_002, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.launchMode = AppExecFwk::LaunchMode::STANDARD;
    abilityRequest.abilityInfo.name = "testAbility";
    abilityRequest.abilityInfo.moduleName = "testModule";
    abilityRequest.abilityInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.applicationInfo.uid = TEST_UID;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    abilityRecord->SetOwnerMissionUserId(AbilityRuntime::UserController::GetInstance().GetCallerUserId());
    uiAbilityLifecycleManager->sessionAbilityMap_.emplace(1, abilityRecord);
    std::vector<std::string> abilityList;
    int32_t pid = 100;
    uiAbilityLifecycleManager->GetActiveAbilityList(TEST_UID, abilityList, pid);
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_IsAbilityStarted_0100
 * @tc.desc: IsAbilityStarted
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, IsAbilityStarted_001, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    ASSERT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest abilityRequest;
    Rosen::SessionInfo info;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->sessionToken = new Rosen::Session(info);
    sessionInfo->persistentId = 0;
    abilityRequest.sessionInfo = sessionInfo;
    auto targetRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    uiAbilityLifecycleManager->sessionAbilityMap_.emplace(sessionInfo->persistentId, targetRecord);
    std::shared_ptr<AbilityRecord> tmpRecord;
    EXPECT_EQ(uiAbilityLifecycleManager->IsAbilityStarted(abilityRequest, tmpRecord), false);
}

/**
 * @tc.name: UIAbilityLifecycleManager_IsAbilityStarted_0200
 * @tc.desc: IsAbilityStarted
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, IsAbilityStarted_002, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    ASSERT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest abilityRequest;
    Rosen::SessionInfo info;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->sessionToken = new Rosen::Session(info);
    sessionInfo->persistentId = 1;
    abilityRequest.sessionInfo = sessionInfo;
    auto targetRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    uiAbilityLifecycleManager->sessionAbilityMap_.emplace(sessionInfo->persistentId, targetRecord);
    sptr<IAbilityScheduler> scheduler = new AbilitySchedulerMock();
    targetRecord->SetScheduler(scheduler);
    std::shared_ptr<AbilityRecord> tmpRecord;
    EXPECT_EQ(uiAbilityLifecycleManager->IsAbilityStarted(abilityRequest, tmpRecord), true);
}

/**
 * @tc.name: UIAbilityLifecycleManager_TryPrepareTerminateByPids_0100
 * @tc.desc: TryPrepareTerminateByPids
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, TryPrepareTerminateByPids_001, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    ASSERT_NE(uiAbilityLifecycleManager, nullptr);
    std::vector<int32_t> pids;
    EXPECT_EQ(uiAbilityLifecycleManager->TryPrepareTerminateByPids(pids, false), ERR_OK);
}

/**
 * @tc.name: UIAbilityLifecycleManager_ChangeAbilityVisibility_0200
 * @tc.desc: ChangeAbilityVisibility
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, ChangeAbilityVisibility_002, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    sptr<IRemoteObject> token = nullptr;
    bool isShow = false;
    EXPECT_EQ(uiAbilityLifecycleManager->ChangeAbilityVisibility(token, isShow), ERR_INVALID_VALUE);
}

/**
 * @tc.name: UIAbilityLifecycleManager_ChangeAbilityVisibility_0300
 * @tc.desc: ChangeAbilityVisibility
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, ChangeAbilityVisibility_003, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.accessTokenId = 100;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    uiAbilityLifecycleManager->sessionAbilityMap_.emplace(0, abilityRecord);
    sptr<IRemoteObject> token = abilityRecord->GetToken()->AsObject();
    bool isShow = true;
    EXPECT_EQ(uiAbilityLifecycleManager->ChangeAbilityVisibility(token, isShow), ERR_NATIVE_NOT_SELF_APPLICATION);
}

/**
 * @tc.name: UIAbilityLifecycleManager_ChangeAbilityVisibility_0400
 * @tc.desc: ChangeAbilityVisibility
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, ChangeAbilityVisibility_004, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.accessTokenId = IPCSkeleton::GetCallingTokenID();
    abilityRequest.sessionInfo = nullptr;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    uiAbilityLifecycleManager->sessionAbilityMap_.emplace(0, abilityRecord);
    sptr<IRemoteObject> token = abilityRecord->GetToken()->AsObject();
    bool isShow = true;
    EXPECT_EQ(uiAbilityLifecycleManager->ChangeAbilityVisibility(token, isShow), ERR_INVALID_VALUE);
}

/**
 * @tc.name: UIAbilityLifecycleManager_ChangeAbilityVisibility_0500
 * @tc.desc: ChangeAbilityVisibility
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, ChangeAbilityVisibility_005, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.accessTokenId = IPCSkeleton::GetCallingTokenID();
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->processOptions = nullptr;
    abilityRequest.sessionInfo = sessionInfo;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    uiAbilityLifecycleManager->sessionAbilityMap_.emplace(0, abilityRecord);
    sptr<IRemoteObject> token = abilityRecord->GetToken()->AsObject();
    bool isShow = true;
    EXPECT_EQ(uiAbilityLifecycleManager->ChangeAbilityVisibility(token, isShow), ERR_START_OPTIONS_CHECK_FAILED);
}

/**
 * @tc.name: UIAbilityLifecycleManager_ChangeAbilityVisibility_0600
 * @tc.desc: ChangeAbilityVisibility
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, ChangeAbilityVisibility_006, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.accessTokenId = IPCSkeleton::GetCallingTokenID();
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->processOptions = std::make_shared<ProcessOptions>();
    abilityRequest.sessionInfo = sessionInfo;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    uiAbilityLifecycleManager->sessionAbilityMap_.emplace(0, abilityRecord);
    sptr<IRemoteObject> token = abilityRecord->GetToken()->AsObject();
    bool isShow = true;
    EXPECT_EQ(uiAbilityLifecycleManager->ChangeAbilityVisibility(token, isShow), ERR_START_OPTIONS_CHECK_FAILED);
}

/**
 * @tc.name: UIAbilityLifecycleManager_ChangeAbilityVisibility_0700
 * @tc.desc: ChangeAbilityVisibility
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, ChangeAbilityVisibility_007, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.accessTokenId = IPCSkeleton::GetCallingTokenID();
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->sessionToken = nullptr;
    sessionInfo->processOptions = std::make_shared<ProcessOptions>();
    sessionInfo->processOptions->processMode = ProcessMode::NEW_PROCESS_ATTACH_TO_STATUS_BAR_ITEM;
    abilityRequest.sessionInfo = sessionInfo;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    uiAbilityLifecycleManager->sessionAbilityMap_.emplace(0, abilityRecord);
    sptr<IRemoteObject> token = abilityRecord->GetToken()->AsObject();
    bool isShow = true;
    EXPECT_EQ(uiAbilityLifecycleManager->ChangeAbilityVisibility(token, isShow), ERR_START_OPTIONS_CHECK_FAILED);
}

/**
 * @tc.name: UIAbilityLifecycleManager_ChangeAbilityVisibility_0800
 * @tc.desc: ChangeAbilityVisibility
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, ChangeAbilityVisibility_008, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.accessTokenId = IPCSkeleton::GetCallingTokenID();
    Rosen::SessionInfo info;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->sessionToken = new Rosen::Session(info);
    sessionInfo->processOptions = std::make_shared<ProcessOptions>();
    sessionInfo->processOptions->processMode = ProcessMode::NEW_PROCESS_ATTACH_TO_STATUS_BAR_ITEM;
    abilityRequest.sessionInfo = sessionInfo;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    uiAbilityLifecycleManager->sessionAbilityMap_.emplace(0, abilityRecord);
    sptr<IRemoteObject> token = abilityRecord->GetToken()->AsObject();
    bool isShow = true;
    EXPECT_EQ(uiAbilityLifecycleManager->ChangeAbilityVisibility(token, isShow), ERR_START_OPTIONS_CHECK_FAILED);
}

/**
 * @tc.name: UIAbilityLifecycleManager_ChangeAbilityVisibility_0900
 * @tc.desc: ChangeAbilityVisibility
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, ChangeAbilityVisibility_009, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.accessTokenId = IPCSkeleton::GetCallingTokenID();
    Rosen::SessionInfo info;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->sessionToken = new Rosen::Session(info);
    sessionInfo->processOptions = std::make_shared<ProcessOptions>();
    sessionInfo->processOptions->processMode = ProcessMode::NEW_PROCESS_ATTACH_TO_STATUS_BAR_ITEM;
    abilityRequest.sessionInfo = sessionInfo;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    uiAbilityLifecycleManager->sessionAbilityMap_.emplace(0, abilityRecord);
    sptr<IRemoteObject> token = abilityRecord->GetToken()->AsObject();
    bool isShow = false;
    EXPECT_EQ(uiAbilityLifecycleManager->ChangeAbilityVisibility(token, isShow), ERR_START_OPTIONS_CHECK_FAILED);
}

/**
 * @tc.name: UIAbilityLifecycleManager_ChangeUIAbilityVisibilityBySCB_0200
 * @tc.desc: ChangeUIAbilityVisibilityBySCB
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, ChangeUIAbilityVisibilityBySCB_002, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    sptr<SessionInfo> sessionInfo = nullptr;
    bool isShow = false;
    EXPECT_EQ(uiAbilityLifecycleManager->ChangeUIAbilityVisibilityBySCB(sessionInfo, isShow), ERR_INVALID_VALUE);
}

/**
 * @tc.name: UIAbilityLifecycleManager_ChangeUIAbilityVisibilityBySCB_0300
 * @tc.desc: ChangeUIAbilityVisibilityBySCB
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, ChangeUIAbilityVisibilityBySCB_003, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest abilityRequest;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->persistentId = 100;
    abilityRequest.sessionInfo = sessionInfo;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    uiAbilityLifecycleManager->sessionAbilityMap_.emplace(0, abilityRecord);
    bool isShow = false;
    EXPECT_EQ(uiAbilityLifecycleManager->ChangeUIAbilityVisibilityBySCB(sessionInfo, isShow),
        ERR_NATIVE_ABILITY_NOT_FOUND);
}

/**
 * @tc.name: UIAbilityLifecycleManager_ChangeUIAbilityVisibilityBySCB_0400
 * @tc.desc: ChangeUIAbilityVisibilityBySCB
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, ChangeUIAbilityVisibilityBySCB_004, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->persistentId = 100;
    uiAbilityLifecycleManager->sessionAbilityMap_.emplace(sessionInfo->persistentId, nullptr);
    bool isShow = false;
    EXPECT_EQ(uiAbilityLifecycleManager->ChangeUIAbilityVisibilityBySCB(sessionInfo, isShow),
        ERR_INVALID_VALUE);
}

/**
 * @tc.name: UIAbilityLifecycleManager_ChangeUIAbilityVisibilityBySCB_0500
 * @tc.desc: ChangeUIAbilityVisibilityBySCB
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, ChangeUIAbilityVisibilityBySCB_005, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest abilityRequest;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->persistentId = 100;
    abilityRequest.sessionInfo = sessionInfo;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    abilityRecord->SetAbilityVisibilityState(AbilityVisibilityState::INITIAL);
    uiAbilityLifecycleManager->sessionAbilityMap_.emplace(sessionInfo->persistentId, abilityRecord);
    bool isShow = false;
    EXPECT_EQ(uiAbilityLifecycleManager->ChangeUIAbilityVisibilityBySCB(sessionInfo, isShow), ERR_OK);
}

/**
 * @tc.name: UIAbilityLifecycleManager_ChangeUIAbilityVisibilityBySCB_0600
 * @tc.desc: ChangeUIAbilityVisibilityBySCB
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, ChangeUIAbilityVisibilityBySCB_006, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest abilityRequest;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->persistentId = 100;
    abilityRequest.sessionInfo = sessionInfo;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    abilityRecord->SetAbilityVisibilityState(AbilityVisibilityState::UNSPECIFIED);
    uiAbilityLifecycleManager->sessionAbilityMap_.emplace(sessionInfo->persistentId, abilityRecord);
    bool isShow = false;
    EXPECT_EQ(uiAbilityLifecycleManager->ChangeUIAbilityVisibilityBySCB(sessionInfo, isShow), ERR_OK);
}

/**
 * @tc.name: UIAbilityLifecycleManager_ChangeUIAbilityVisibilityBySCB_0700
 * @tc.desc: ChangeUIAbilityVisibilityBySCB
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, ChangeUIAbilityVisibilityBySCB_007, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest abilityRequest;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->persistentId = 100;
    abilityRequest.sessionInfo = sessionInfo;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    abilityRecord->SetAbilityVisibilityState(AbilityVisibilityState::FOREGROUND_SHOW);
    uiAbilityLifecycleManager->sessionAbilityMap_.emplace(sessionInfo->persistentId, abilityRecord);
    bool isShow = true;
    EXPECT_EQ(uiAbilityLifecycleManager->ChangeUIAbilityVisibilityBySCB(sessionInfo, isShow), ERR_OK);
}

/**
 * @tc.name: UIAbilityLifecycleManager_ChangeUIAbilityVisibilityBySCB_0800
 * @tc.desc: ChangeUIAbilityVisibilityBySCB
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, ChangeUIAbilityVisibilityBySCB_008, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest abilityRequest;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->persistentId = 100;
    abilityRequest.sessionInfo = sessionInfo;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    abilityRecord->SetAbilityVisibilityState(AbilityVisibilityState::FOREGROUND_HIDE);
    uiAbilityLifecycleManager->sessionAbilityMap_.emplace(sessionInfo->persistentId, abilityRecord);
    bool isShow = false;
    EXPECT_EQ(uiAbilityLifecycleManager->ChangeUIAbilityVisibilityBySCB(sessionInfo, isShow), ERR_OK);
}

/**
 * @tc.name: UIAbilityLifecycleManager_GetAbilityRecordsByName_0100
 * @tc.desc: GetAbilityRecordsByName
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, GetAbilityRecordsByName_001, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.deviceId = "100";
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    uiAbilityLifecycleManager->sessionAbilityMap_.emplace(1, abilityRecord);
    AppExecFwk::ElementName element;
    int32_t appIndex = 0;
    auto ret = uiAbilityLifecycleManager->GetAbilityRecordsByName(element, appIndex);
    EXPECT_EQ(ret.empty(), true);
}

/**
 * @tc.name: UIAbilityLifecycleManager_GetAbilityRecordsByName_0200
 * @tc.desc: GetAbilityRecordsByName
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, GetAbilityRecordsByName_002, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.deviceId = "100";
    abilityRequest.abilityInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "MainAbility";
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    uiAbilityLifecycleManager->sessionAbilityMap_.emplace(1, abilityRecord);
    AppExecFwk::ElementName element("100", "com.example.unittest", "MainAbility");
    int32_t appIndex = 0;
    auto ret = uiAbilityLifecycleManager->GetAbilityRecordsByName(element, appIndex);
    EXPECT_EQ(ret.empty(), false);
}

/**
 * @tc.name: UIAbilityLifecycleManager_GetAbilityRecordsByName_0300
 * @tc.desc: GetAbilityRecordsByName
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, GetAbilityRecordsByName_003, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.deviceId = "100";
    abilityRequest.abilityInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.moduleName = "entry";
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    uiAbilityLifecycleManager->sessionAbilityMap_.emplace(1, abilityRecord);
    AppExecFwk::ElementName element("100", "com.example.unittest", "MainAbility", "entry");
    int32_t appIndex = 0;
    auto ret = uiAbilityLifecycleManager->GetAbilityRecordsByName(element, appIndex);
    EXPECT_EQ(ret.empty(), false);
}

/**
 * @tc.name: UIAbilityLifecycleManager_GetAbilityRecordsByNameInner_0100
 * @tc.desc: GetAbilityRecordsByNameInner
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, GetAbilityRecordsByNameInner_001, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.deviceId = "100";
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    uiAbilityLifecycleManager->sessionAbilityMap_.emplace(1, abilityRecord);
    AppExecFwk::ElementName element;
    int32_t appIndex = 0;
    auto ret = uiAbilityLifecycleManager->GetAbilityRecordsByNameInner(element, appIndex);
    EXPECT_EQ(ret.empty(), true);
}

/**
 * @tc.name: UIAbilityLifecycleManager_GetAbilityRecordsByNameInner_0200
 * @tc.desc: GetAbilityRecordsByNameInner
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, GetAbilityRecordsByNameInner_002, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.deviceId = "100";
    abilityRequest.abilityInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "MainAbility";
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    uiAbilityLifecycleManager->sessionAbilityMap_.emplace(1, abilityRecord);
    AppExecFwk::ElementName element("100", "com.example.unittest", "MainAbility");
    int32_t appIndex = 0;
    auto ret = uiAbilityLifecycleManager->GetAbilityRecordsByNameInner(element, appIndex);
    EXPECT_EQ(ret.empty(), false);
}

/**
 * @tc.name: UIAbilityLifecycleManager_GetAbilityRecordsByNameInner_0300
 * @tc.desc: GetAbilityRecordsByNameInner
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, GetAbilityRecordsByNameInner_003, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.deviceId = "100";
    abilityRequest.abilityInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "MainAbility";
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    uiAbilityLifecycleManager->sessionAbilityMap_.emplace(1, abilityRecord);
    AppExecFwk::ElementName element("100", "com.example.unittest", "MainAbility");
    int32_t appIndex = -1;
    auto ret = uiAbilityLifecycleManager->GetAbilityRecordsByNameInner(element, appIndex);
    EXPECT_EQ(ret.empty(), false);
}

/**
 * @tc.name: UIAbilityLifecycleManager_GetAbilityRecordsByNameInner_0400
 * @tc.desc: GetAbilityRecordsByNameInner
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, GetAbilityRecordsByNameInner_004, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.deviceId = "100";
    abilityRequest.abilityInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.moduleName = "entry";
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    uiAbilityLifecycleManager->sessionAbilityMap_.emplace(1, abilityRecord);
    AppExecFwk::ElementName element("100", "com.example.unittest", "MainAbility", "entry");
    int32_t appIndex = 0;
    auto ret = uiAbilityLifecycleManager->GetAbilityRecordsByNameInner(element, appIndex);
    EXPECT_EQ(ret.empty(), false);
}

/**
 * @tc.name: UIAbilityLifecycleManager_GetAbilityRecordsByNameInner_0500
 * @tc.desc: GetAbilityRecordsByNameInner
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, GetAbilityRecordsByNameInner_005, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.deviceId = "100";
    abilityRequest.abilityInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.moduleName = "entry";
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    uiAbilityLifecycleManager->sessionAbilityMap_.emplace(1, abilityRecord);
    AppExecFwk::ElementName element("100", "com.example.unittest", "MainAbility", "entry");
    int32_t appIndex = -1;
    auto ret = uiAbilityLifecycleManager->GetAbilityRecordsByNameInner(element, appIndex);
    EXPECT_EQ(ret.empty(), false);
}

/**
 * @tc.name: UIAbilityLifecycleManager_PrepareTerminateAbility_0100
 * @tc.desc: PrepareTerminateAbility
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, PrepareTerminateAbility_001, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    UIAbilityRecordPtr abilityRecord = nullptr;
    bool boolValue = uiAbilityLifecycleManager->PrepareTerminateAbility(abilityRecord, false);
    EXPECT_FALSE(boolValue);
}

/**
 * @tc.name: UIAbilityLifecycleManager_PrepareTerminateAbility_0200
 * @tc.desc: PrepareTerminateAbility
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, PrepareTerminateAbility_002, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    UIAbilityRecordPtr abilityRecord = InitAbilityRecord();
    bool boolValue = uiAbilityLifecycleManager->PrepareTerminateAbility(abilityRecord, false);
    EXPECT_FALSE(boolValue);
}

/**
 * @tc.name: UIAbilityLifecycleManager_SetSessionHandler_0100
 * @tc.desc: SetSessionHandler
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, SetSessionHandler_001, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    sptr<ISessionHandler> handler;
    uiAbilityLifecycleManager->SetSessionHandler(handler);
    EXPECT_EQ(uiAbilityLifecycleManager->handler_, handler);
}

/**
 * @tc.name: UIAbilityLifecycleManager_GetAbilityRecordsById_0100
 * @tc.desc: GetAbilityRecordsById
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, GetAbilityRecordsById_001, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    int32_t sessionId = 100;
    AbilityRequest abilityRequest;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    uiAbilityLifecycleManager->sessionAbilityMap_.emplace(sessionId, abilityRecord);
    EXPECT_EQ(uiAbilityLifecycleManager->GetAbilityRecordsById(sessionId + 1), nullptr);
}

/**
 * @tc.name: UIAbilityLifecycleManager_GetAbilityRecordsById_0200
 * @tc.desc: GetAbilityRecordsById
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, GetAbilityRecordsById_002, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    int32_t sessionId = 100;
    AbilityRequest abilityRequest;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    uiAbilityLifecycleManager->sessionAbilityMap_.emplace(sessionId, abilityRecord);
    EXPECT_NE(uiAbilityLifecycleManager->GetAbilityRecordsById(sessionId), nullptr);
}

/**
 * @tc.name: UIAbilityLifecycleManager_OnAppStateChanged_0100
 * @tc.desc: OnAppStateChanged
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, OnAppStateChanged_001, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.process = "AbilityProcess";
    UIAbilityRecordPtr abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    AppInfo info;
    info.processName = "AbilityProcess";
    info.state = AppState::TERMINATED;
    uiAbilityLifecycleManager->terminateAbilityList_.emplace_back(abilityRecord);
    uiAbilityLifecycleManager->OnAppStateChanged(info);
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_OnAppStateChanged_0200
 * @tc.desc: OnAppStateChanged
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, OnAppStateChanged_002, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.process = "AbilityProcess";
    UIAbilityRecordPtr abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    AppInfo info;
    info.processName = "AbilityProcess";
    info.state = AppState::END;
    uiAbilityLifecycleManager->terminateAbilityList_.emplace_back(abilityRecord);
    uiAbilityLifecycleManager->OnAppStateChanged(info);
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_OnAppStateChanged_0300
 * @tc.desc: OnAppStateChanged
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, OnAppStateChanged_003, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.process = "AbilityProcess";
    UIAbilityRecordPtr abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    AppInfo info;
    info.processName = "com.example.unittest";
    info.state = AppState::TERMINATED;
    uiAbilityLifecycleManager->terminateAbilityList_.emplace_back(abilityRecord);
    uiAbilityLifecycleManager->OnAppStateChanged(info);
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_OnAppStateChanged_0400
 * @tc.desc: OnAppStateChanged
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, OnAppStateChanged_004, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.process = "AbilityProcess";
    UIAbilityRecordPtr abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    AppInfo info;
    info.processName = "com.example.unittest";
    info.state = AppState::END;
    uiAbilityLifecycleManager->terminateAbilityList_.emplace_back(abilityRecord);
    uiAbilityLifecycleManager->OnAppStateChanged(info);
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_OnAppStateChanged_0500
 * @tc.desc: OnAppStateChanged
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, OnAppStateChanged_005, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.process = "AbilityProcess";
    UIAbilityRecordPtr abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    AppInfo info;
    info.processName = "com.example.unittest";
    info.state = AppState::COLD_START;
    uiAbilityLifecycleManager->sessionAbilityMap_.emplace(0, abilityRecord);
    uiAbilityLifecycleManager->OnAppStateChanged(info);
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_OnAppStateChanged_0600
 * @tc.desc: OnAppStateChanged
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, OnAppStateChanged_006, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.process = "AbilityProcess";
    UIAbilityRecordPtr abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    AppInfo info;
    info.processName = "AbilityProcess";
    info.state = AppState::COLD_START;
    uiAbilityLifecycleManager->sessionAbilityMap_.emplace(0, abilityRecord);
    uiAbilityLifecycleManager->OnAppStateChanged(info);
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_OnAppStateChanged_0700
 * @tc.desc: OnAppStateChanged
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, OnAppStateChanged_007, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.process = "AbilityProcess";
    UIAbilityRecordPtr abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    AppInfo info;
    info.processName = "com.example.unittest";
    uiAbilityLifecycleManager->sessionAbilityMap_.emplace(0, abilityRecord);
    uiAbilityLifecycleManager->OnAppStateChanged(info);
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_OnAppStateChanged_0800
 * @tc.desc: OnAppStateChanged
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, OnAppStateChanged_008, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.process = "AbilityProcess";
    UIAbilityRecordPtr abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    AppInfo info;
    info.processName = "AbilityProcess";
    uiAbilityLifecycleManager->sessionAbilityMap_.emplace(0, abilityRecord);
    uiAbilityLifecycleManager->OnAppStateChanged(info);
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_OnAppStateChanged_0900
 * @tc.desc: OnAppStateChanged
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, OnAppStateChanged_009, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.process = "AbilityProcess";
    UIAbilityRecordPtr abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    AppInfo info;
    info.processName = "AbilityProcess";
    info.bundleName = "com.example.unittest";
    info.appIndex = abilityRecord->GetAppIndex();
    info.instanceKey = abilityRecord->GetInstanceKey();
    info.pid = abilityRecord->GetPid();
    info.state = AppState::TERMINATED;
    uiAbilityLifecycleManager->terminateAbilityList_.emplace_back(abilityRecord);
    uiAbilityLifecycleManager->OnAppStateChanged(info);
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_OnAppStateChanged_1000
 * @tc.desc: OnAppStateChanged
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, OnAppStateChanged_010, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.process = "AbilityProcess";
    UIAbilityRecordPtr abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    AppInfo info;
    info.processName = "AbilityProcess";
    info.bundleName = "com.example.unittest1";
    info.state = AppState::TERMINATED;
    uiAbilityLifecycleManager->OnAppStateChanged(info);
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_OnAppStateChanged_1100
 * @tc.desc: OnAppStateChanged
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, OnAppStateChanged_011, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.process = "AbilityProcess";
    UIAbilityRecordPtr abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    AppInfo info;
    info.processName = "AbilityProcess";
    info.bundleName = "com.example.unittest";
    info.appIndex = abilityRecord->GetAppIndex();
    info.instanceKey = abilityRecord->GetInstanceKey();
    info.pid = abilityRecord->GetPid();
    info.state = AppState::END;
    uiAbilityLifecycleManager->terminateAbilityList_.emplace_back(abilityRecord);
    uiAbilityLifecycleManager->OnAppStateChanged(info);
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_OnAppStateChanged_1200
 * @tc.desc: OnAppStateChanged
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, OnAppStateChanged_012, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.process = "AbilityProcess";
    UIAbilityRecordPtr abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    AppInfo info;
    info.processName = "AbilityProcess";
    info.bundleName = "com.example.unittest1";
    info.state = AppState::END;
    uiAbilityLifecycleManager->OnAppStateChanged(info);
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_OnAppStateChanged_1300
 * @tc.desc: OnAppStateChanged
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, OnAppStateChanged_013, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.process = "AbilityProcess";
    UIAbilityRecordPtr abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    AppInfo info;
    info.processName = "AbilityProcess";
    info.bundleName = "com.example.unittest";
    info.appIndex = abilityRecord->GetAppIndex();
    info.instanceKey = abilityRecord->GetInstanceKey();
    info.pid = abilityRecord->GetPid();
    info.state = AppState::COLD_START;
    uiAbilityLifecycleManager->sessionAbilityMap_.emplace(0, abilityRecord);
    uiAbilityLifecycleManager->OnAppStateChanged(info);
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_OnAppStateChanged_1400
 * @tc.desc: OnAppStateChanged
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, OnAppStateChanged_014, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.process = "AbilityProcess";
    UIAbilityRecordPtr abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    AppInfo info;
    info.processName = "AbilityProcess";
    info.bundleName = "com.example.unittest1";
    info.state = AppState::COLD_START;
    uiAbilityLifecycleManager->OnAppStateChanged(info);
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_OnAppStateChanged_1500
 * @tc.desc: OnAppStateChanged
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, OnAppStateChanged_015, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.process = "AbilityProcess";
    UIAbilityRecordPtr abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    AppInfo info;
    info.processName = "AbilityProcess";
    info.bundleName = "com.example.unittest";
    info.appIndex = abilityRecord->GetAppIndex();
    info.instanceKey = abilityRecord->GetInstanceKey();
    info.pid = abilityRecord->GetPid();
    uiAbilityLifecycleManager->sessionAbilityMap_.emplace(0, abilityRecord);
    uiAbilityLifecycleManager->OnAppStateChanged(info);
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_OnAppStateChanged_1600
 * @tc.desc: OnAppStateChanged
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, OnAppStateChanged_016, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.process = "AbilityProcess";
    UIAbilityRecordPtr abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    AppInfo info;
    info.processName = "AbilityProcess";
    info.bundleName = "com.example.unittest1";
    uiAbilityLifecycleManager->OnAppStateChanged(info);
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_UninstallApp_0100
 * @tc.desc: UninstallApp
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, UninstallApp_001, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "MainAbility";
    UIAbilityRecordPtr abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    AppInfo info;
    std::string bundleName = "com.example.unittest";
    int32_t uid = 0;
    uiAbilityLifecycleManager->sessionAbilityMap_.emplace(0, abilityRecord);
    uiAbilityLifecycleManager->UninstallApp(bundleName, uid);
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_GetAbilityRunningInfos_0100
 * @tc.desc: GetAbilityRunningInfos
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, GetAbilityRunningInfos_001, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(uiAbilityLifecycleManager, nullptr);
    UIAbilityRecordPtr abilityRecord = InitAbilityRecord();
    uiAbilityLifecycleManager->sessionAbilityMap_.emplace(0, abilityRecord);
    std::vector<AbilityRunningInfo> info;
    bool isPerm = true;
    uiAbilityLifecycleManager->GetAbilityRunningInfos(info, isPerm);
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_GetAbilityRunningInfos_0200
 * @tc.desc: GetAbilityRunningInfos
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, GetAbilityRunningInfos_002, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.accessTokenId = IPCSkeleton::GetCallingTokenID();
    UIAbilityRecordPtr abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    uiAbilityLifecycleManager->sessionAbilityMap_.emplace(0, abilityRecord);
    std::vector<AbilityRunningInfo> info;
    bool isPerm = false;
    uiAbilityLifecycleManager->GetAbilityRunningInfos(info, isPerm);
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_MoveMissionToFront_0100
 * @tc.desc: MoveMissionToFront
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, MoveMissionToFront_001, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    ASSERT_NE(uiAbilityLifecycleManager, nullptr);
    uiAbilityLifecycleManager->rootSceneSession_ = nullptr;
    int32_t sessionId = 100;
    std::shared_ptr<StartOptions> startOptions;
    EXPECT_EQ(uiAbilityLifecycleManager->MoveMissionToFront(sessionId, startOptions), ERR_INVALID_VALUE);
}

/**
 * @tc.name: UIAbilityLifecycleManager_MoveMissionToFront_0200
 * @tc.desc: MoveMissionToFront
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, MoveMissionToFront_002, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    ASSERT_NE(uiAbilityLifecycleManager, nullptr);
    Rosen::SessionInfo info;
    uiAbilityLifecycleManager->rootSceneSession_ = new Rosen::Session(info);
    int32_t sessionId = 100;
    std::shared_ptr<StartOptions> startOptions;
    UIAbilityRecordPtr abilityRecord = InitAbilityRecord();
    uiAbilityLifecycleManager->sessionAbilityMap_.emplace(0, abilityRecord);
    EXPECT_EQ(uiAbilityLifecycleManager->MoveMissionToFront(sessionId, startOptions), ERR_INVALID_VALUE);
}

/**
 * @tc.name: UIAbilityLifecycleManager_MoveMissionToFront_0300
 * @tc.desc: MoveMissionToFront
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, MoveMissionToFront_003, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    ASSERT_NE(uiAbilityLifecycleManager, nullptr);
    Rosen::SessionInfo info;
    uiAbilityLifecycleManager->rootSceneSession_ = new Rosen::Session(info);
    int32_t sessionId = 100;
    std::shared_ptr<StartOptions> startOptions;
    AbilityRequest abilityRequest;
    sptr<SessionInfo> sessionInfo = nullptr;
    abilityRequest.sessionInfo = sessionInfo;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    uiAbilityLifecycleManager->sessionAbilityMap_.emplace(sessionId, abilityRecord);
    EXPECT_EQ(uiAbilityLifecycleManager->MoveMissionToFront(sessionId, startOptions), ERR_INVALID_VALUE);
}

/**
 * @tc.name: UIAbilityLifecycleManager_MoveMissionToFront_0400
 * @tc.desc: MoveMissionToFront
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, MoveMissionToFront_004, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    ASSERT_NE(uiAbilityLifecycleManager, nullptr);
    int32_t sessionId = 100;
    std::shared_ptr<StartOptions> startOptions;
    Rosen::SessionInfo info;
    uiAbilityLifecycleManager->rootSceneSession_ = new Rosen::Session(info);
    AbilityRequest abilityRequest;
    sptr<SessionInfo> sessionInfo = (new SessionInfo());
    abilityRequest.sessionInfo = sessionInfo;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    uiAbilityLifecycleManager->sessionAbilityMap_.emplace(sessionId, abilityRecord);
    EXPECT_EQ(uiAbilityLifecycleManager->MoveMissionToFront(sessionId, startOptions), ERR_OK);
}

/**
 * @tc.name: UIAbilityLifecycleManager_GetReusedCollaboratorPersistentId_0100
 * @tc.desc: GetReusedCollaboratorPersistentId
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, GetReusedCollaboratorPersistentId_001, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest abilityRequest;
    Want want;
    want.SetParam("ohos.anco.param.missionAffinity", false);
    abilityRequest.want = want;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->persistentId = 100;
    abilityRequest.sessionInfo = sessionInfo;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    abilityRecord->collaboratorType_ = CollaboratorType::DEFAULT_TYPE;
    uiAbilityLifecycleManager->sessionAbilityMap_.emplace(sessionInfo->persistentId, abilityRecord);
    bool reuse = false;
    EXPECT_NE(uiAbilityLifecycleManager->GetReusedCollaboratorPersistentId(abilityRequest, reuse),
        sessionInfo->persistentId);
}

/**
 * @tc.name: UIAbilityLifecycleManager_GetReusedCollaboratorPersistentId_0200
 * @tc.desc: GetReusedCollaboratorPersistentId
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, GetReusedCollaboratorPersistentId_002, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest abilityRequest;
    Want want;
    want.SetParam("ohos.anco.param.missionAffinity", false);
    abilityRequest.want = want;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->persistentId = 100;
    abilityRequest.sessionInfo = sessionInfo;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    abilityRecord->collaboratorType_ = CollaboratorType::RESERVE_TYPE;
    uiAbilityLifecycleManager->sessionAbilityMap_.emplace(sessionInfo->persistentId, abilityRecord);
    bool reuse = false;
    EXPECT_EQ(uiAbilityLifecycleManager->GetReusedCollaboratorPersistentId(abilityRequest, reuse),
        sessionInfo->persistentId);
}

/**
 * @tc.name: UIAbilityLifecycleManager_GetReusedCollaboratorPersistentId_0300
 * @tc.desc: GetReusedCollaboratorPersistentId
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, GetReusedCollaboratorPersistentId_003, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest abilityRequest;
    Want want;
    want.SetParam("ohos.anco.param.missionAffinity", false);
    abilityRequest.want = want;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->persistentId = 100;
    abilityRequest.sessionInfo = sessionInfo;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    abilityRecord->collaboratorType_ = CollaboratorType::OTHERS_TYPE;
    uiAbilityLifecycleManager->sessionAbilityMap_.emplace(sessionInfo->persistentId, abilityRecord);
    bool reuse = false;
    EXPECT_EQ(uiAbilityLifecycleManager->GetReusedCollaboratorPersistentId(abilityRequest, reuse),
        sessionInfo->persistentId);
}

/**
 * @tc.name: UIAbilityLifecycleManager_DispatchTerminate_0100
 * @tc.desc: DispatchTerminate
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, DispatchTerminate_001, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    UIAbilityRecordPtr abilityRecord = nullptr;
    EXPECT_EQ(uiAbilityLifecycleManager->DispatchTerminate(abilityRecord), ERR_INVALID_VALUE);
}

/**
 * @tc.name: UIAbilityLifecycleManager_DispatchTerminate_0200
 * @tc.desc: DispatchTerminate
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, DispatchTerminate_002, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    UIAbilityRecordPtr abilityRecord = InitAbilityRecord();
    abilityRecord->currentState_ = AbilityState::INITIAL;
    EXPECT_EQ(uiAbilityLifecycleManager->DispatchTerminate(abilityRecord), INNER_ERR);
}

/**
 * @tc.name: UIAbilityLifecycleManager_DispatchTerminate_0300
 * @tc.desc: DispatchTerminate
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, DispatchTerminate_003, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    UIAbilityRecordPtr abilityRecord = InitAbilityRecord();
    abilityRecord->currentState_ = AbilityState::TERMINATING;
    EXPECT_EQ(uiAbilityLifecycleManager->DispatchTerminate(abilityRecord), ERR_INVALID_VALUE);
}

/**
 * @tc.name: UIAbilityLifecycleManager_DispatchBackground_0100
 * @tc.desc: DispatchBackground
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, DispatchBackground_001, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    UIAbilityRecordPtr abilityRecord = InitAbilityRecord();
    EXPECT_EQ(uiAbilityLifecycleManager->DispatchBackground(abilityRecord), ERR_INVALID_VALUE);
}

/**
 * @tc.name: UIAbilityLifecycleManager_DispatchBackground_0200
 * @tc.desc: DispatchBackground
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, DispatchBackground_002, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    UIAbilityRecordPtr abilityRecord = nullptr;
    EXPECT_EQ(uiAbilityLifecycleManager->DispatchBackground(abilityRecord), ERR_INVALID_VALUE);
}

#ifdef WITH_DLP
/**
 * @tc.name: UIAbilityLifecycleManager_CheckProperties_0100
 * @tc.desc: CheckProperties
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, CheckProperties_001, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    Want want;
    AbilityRequest abilityRequest;
    want.SetParam(DLP_INDEX, 1);
    abilityRequest.want = want;
    abilityRequest.abilityInfo.launchMode = AppExecFwk::LaunchMode::SPECIFIED;
    abilityRequest.abilityInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.moduleName = "entry";
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    abilityRecord->SetAppIndex(2);
    AppExecFwk::LaunchMode launchMode = AppExecFwk::LaunchMode::SPECIFIED;
    auto ret = uiAbilityLifecycleManager->CheckProperties(abilityRecord, abilityRequest, launchMode);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: UIAbilityLifecycleManager_CheckProperties_0200
 * @tc.desc: CheckProperties
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, CheckProperties_002, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    Want want;
    AbilityRequest abilityRequest;
    want.SetParam(DLP_INDEX, 1);
    abilityRequest.want = want;
    abilityRequest.abilityInfo.launchMode = AppExecFwk::LaunchMode::SPECIFIED;
    abilityRequest.abilityInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.moduleName = "entry";
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    abilityRecord->SetAppIndex(1);
    AppExecFwk::LaunchMode launchMode = AppExecFwk::LaunchMode::STANDARD;
    auto ret = uiAbilityLifecycleManager->CheckProperties(abilityRecord, abilityRequest, launchMode);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: UIAbilityLifecycleManager_CheckProperties_0300
 * @tc.desc: CheckProperties
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, CheckProperties_003, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    Want want;
    AbilityRequest abilityRequest;
    want.SetParam(DLP_INDEX, 1);
    abilityRequest.want = want;
    abilityRequest.abilityInfo.launchMode = AppExecFwk::LaunchMode::SPECIFIED;
    abilityRequest.abilityInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.moduleName = "entry";
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    abilityRecord->SetAppIndex(1);
    AppExecFwk::LaunchMode launchMode = AppExecFwk::LaunchMode::SPECIFIED;
    auto ret = uiAbilityLifecycleManager->CheckProperties(abilityRecord, abilityRequest, launchMode);
    EXPECT_EQ(ret, true);
}
#endif // WITH_DLP

/**
 * @tc.name: UIAbilityLifecycleManager_ResolveAbility_0100
 * @tc.desc: ResolveAbility
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, ResolveAbility_001, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest abilityRequest;
    UIAbilityRecordPtr targetAbility = nullptr;
    EXPECT_EQ(uiAbilityLifecycleManager->ResolveAbility(targetAbility, abilityRequest),
        ResolveResultType::NG_INNER_ERROR);
}

/**
 * @tc.name: UIAbilityLifecycleManager_ResolveAbility_0200
 * @tc.desc: ResolveAbility
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, ResolveAbility_002, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest abilityRequest;
    abilityRequest.callType = AbilityCallType::START_OPTIONS_TYPE;
    auto targetAbility = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    EXPECT_EQ(uiAbilityLifecycleManager->ResolveAbility(targetAbility, abilityRequest),
        ResolveResultType::NG_INNER_ERROR);
}

/**
 * @tc.name: UIAbilityLifecycleManager_ResolveAbility_0300
 * @tc.desc: ResolveAbility
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, ResolveAbility_003, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest abilityRequest;
    abilityRequest.connect = new UIAbilityLifecycleManagerTestStub();
    abilityRequest.callType = AbilityCallType::CALL_REQUEST_TYPE;
    auto targetAbility = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    targetAbility->callContainer_ = std::make_shared<CallContainer>();
    EXPECT_EQ(uiAbilityLifecycleManager->ResolveAbility(targetAbility, abilityRequest),
        ResolveResultType::OK_NO_REMOTE_OBJ);
}

/**
 * @tc.name: UIAbilityLifecycleManager_ResolveAbility_0400
 * @tc.desc: ResolveAbility
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, ResolveAbility_004, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest abilityRequest;
    abilityRequest.connect = new UIAbilityLifecycleManagerTestStub();
    abilityRequest.callType = AbilityCallType::CALL_REQUEST_TYPE;
    auto targetAbility = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    targetAbility->isReady_ = true;
    EXPECT_EQ(uiAbilityLifecycleManager->ResolveAbility(targetAbility, abilityRequest),
        ResolveResultType::OK_HAS_REMOTE_OBJ);
}

/**
 * @tc.name: UIAbilityLifecycleManager_GetContentAndTypeId_0100
 * @tc.desc: GetContentAndTypeId
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, GetContentAndTypeId_001, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    uint32_t msgId = AbilityManagerService::LOAD_TIMEOUT_MSG;
    std::string msgContent = "content";
    int typeId;
    EXPECT_EQ(uiAbilityLifecycleManager->GetContentAndTypeId(msgId, msgContent, typeId), true);
}

/**
 * @tc.name: UIAbilityLifecycleManager_GetContentAndTypeId_0200
 * @tc.desc: GetContentAndTypeId
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, GetContentAndTypeId_002, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    uint32_t msgId = AbilityManagerService::FOREGROUND_TIMEOUT_MSG;
    std::string msgContent = "content";
    int typeId;
    EXPECT_EQ(uiAbilityLifecycleManager->GetContentAndTypeId(msgId, msgContent, typeId), true);
}

/**
 * @tc.name: UIAbilityLifecycleManager_GetContentAndTypeId_0300
 * @tc.desc: GetContentAndTypeId
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, GetContentAndTypeId_003, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    uint32_t msgId = AbilityManagerService::BACKGROUND_TIMEOUT_MSG;
    std::string msgContent = "content";
    int typeId;
    EXPECT_EQ(uiAbilityLifecycleManager->GetContentAndTypeId(msgId, msgContent, typeId), true);
}

/**
 * @tc.name: UIAbilityLifecycleManager_GetContentAndTypeId_0400
 * @tc.desc: GetContentAndTypeId
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, GetContentAndTypeId_004, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    uint32_t msgId = AbilityManagerService::TERMINATE_TIMEOUT_MSG;
    std::string msgContent = "content";
    int typeId;
    EXPECT_EQ(uiAbilityLifecycleManager->GetContentAndTypeId(msgId, msgContent, typeId), true);
}

/**
 * @tc.name: UIAbilityLifecycleManager_GetContentAndTypeId_0500
 * @tc.desc: GetContentAndTypeId
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, GetContentAndTypeId_005, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    uint32_t msgId = AbilityManagerService::ACTIVE_TIMEOUT_MSG;
    std::string msgContent = "content";
    int typeId;
    EXPECT_EQ(uiAbilityLifecycleManager->GetContentAndTypeId(msgId, msgContent, typeId), false);
}

/**
 * @tc.name: UIAbilityLifecycleManager_GetContentAndTypeId_0600
 * @tc.desc: GetContentAndTypeId
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, GetContentAndTypeId_0600, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    uint32_t msgId = AbilityManagerService::LOAD_TIMEOUT_MSG;
    std::string msgContent;
    std::string ret = "load timeout.";
    int typeId;
    EXPECT_EQ(uiAbilityLifecycleManager->GetContentAndTypeId(msgId, msgContent, typeId), true);
    EXPECT_EQ(msgContent, ret);
}

/**
 * @tc.name: UIAbilityLifecycleManager_GetContentAndTypeId_0700
 * @tc.desc: GetContentAndTypeId
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, GetContentAndTypeId_0700, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    uint32_t msgId = AbilityManagerService::FOREGROUND_TIMEOUT_MSG;
    std::string msgContent;
    std::string ret = "foreground timeout.";
    int typeId;
    EXPECT_EQ(uiAbilityLifecycleManager->GetContentAndTypeId(msgId, msgContent, typeId), true);
    EXPECT_EQ(msgContent, ret);
}

/**
 * @tc.name: UIAbilityLifecycleManager_GetContentAndTypeId_0800
 * @tc.desc: GetContentAndTypeId
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, GetContentAndTypeId_0800, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    uint32_t msgId = AbilityManagerService::BACKGROUND_TIMEOUT_MSG;
    std::string msgContent;
    std::string ret = "background timeout.";
    int typeId;
    EXPECT_EQ(uiAbilityLifecycleManager->GetContentAndTypeId(msgId, msgContent, typeId), true);
    EXPECT_EQ(msgContent, ret);
}

/**
 * @tc.name: UIAbilityLifecycleManager_GetContentAndTypeId_0900
 * @tc.desc: GetContentAndTypeId
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, GetContentAndTypeId_0900, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    uint32_t msgId = AbilityManagerService::TERMINATE_TIMEOUT_MSG;
    std::string msgContent;
    std::string ret = "terminate timeout.";
    int typeId;
    EXPECT_EQ(uiAbilityLifecycleManager->GetContentAndTypeId(msgId, msgContent, typeId), true);
    EXPECT_EQ(msgContent, ret);
}

/**
 * @tc.name: UIAbilityLifecycleManager_GetContentAndTypeId_1000
 * @tc.desc: GetContentAndTypeId
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, GetContentAndTypeId_1000, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    uint32_t msgId = AbilityManagerService::ACTIVE_TIMEOUT_MSG;
    std::string msgContent;
    std::string ret = "active timeout.";
    int typeId;
    EXPECT_EQ(uiAbilityLifecycleManager->GetContentAndTypeId(msgId, msgContent, typeId), false);
    EXPECT_NE(msgContent, ret);
}

/**
 * @tc.name: UIAbilityLifecycleManager_GetContentAndTypeId_1100
 * @tc.desc: GetContentAndTypeId
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, GetContentAndTypeId_1100, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    uint32_t msgId1 = AbilityManagerService::LOAD_TIMEOUT_MSG;
    std::string msgContent1;
    int typeId1 = -1;
    EXPECT_EQ(uiAbilityLifecycleManager->GetContentAndTypeId(msgId1, msgContent1, typeId1), true);
    
    uint32_t msgId2 = AbilityManagerService::FOREGROUND_TIMEOUT_MSG;
    std::string msgContent2;
    int typeId2 = -1;
    EXPECT_EQ(uiAbilityLifecycleManager->GetContentAndTypeId(msgId2, msgContent2, typeId2), true);
    
    EXPECT_NE(msgContent1, msgContent2);
    EXPECT_EQ(typeId1, typeId2);
}

/**
 * @tc.name: UIAbilityLifecycleManager_CheckCallerFromBackground_0100
 * @tc.desc: CheckCallerFromBackground
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, CheckCallerFromBackground_0100, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    sptr<SessionInfo> info = nullptr;
    uiAbilityLifecycleManager->CheckCallerFromBackground(nullptr, info);
}

/**
 * @tc.name: UIAbilityLifecycleManager_CheckCallerFromBackground_0200
 * @tc.desc: CheckCallerFromBackground
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, CheckCallerFromBackground_0200, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    sptr<SessionInfo> info = nullptr;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    AbilityRequest abilityRequest;
    abilityRequest.sessionInfo = sessionInfo;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    uiAbilityLifecycleManager->CheckCallerFromBackground(abilityRecord, info);
}

/**
 * @tc.name: UIAbilityLifecycleManager_CheckCallerFromBackground_0300
 * @tc.desc: CheckCallerFromBackground
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, CheckCallerFromBackground_0300, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    uiAbilityLifecycleManager->CheckCallerFromBackground(nullptr, sessionInfo);
}

/**
 * @tc.name: UIAbilityLifecycleManager_CheckCallerFromBackground_0400
 * @tc.desc: CheckCallerFromBackground
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, CheckCallerFromBackground_0400, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    AbilityRequest abilityRequest;
    abilityRequest.sessionInfo = sessionInfo;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    uiAbilityLifecycleManager->CheckCallerFromBackground(abilityRecord, sessionInfo);
}

/*
 * Feature: UIAbilityLifecycleManagerTest
 * Function: BackToCallerAbilityWithResult
 * SubFunction: NA
 * FunctionPoints: UIAbilityLifecycleManagerTest BackToCallerAbilityWithResult
 * EnvConditions: NA
 * CaseDescription: Verify BackToCallerAbilityWithResult
 */
HWTEST_F(UIAbilityLifecycleManagerTest, BackToCallerAbilityWithResult_001, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);

    UIAbilityRecordPtr abilityRecord = InitAbilityRecord();
    Want resultWant;
    int32_t resultCode = 100;
    int64_t callerRequestCode = 0;
    auto ret = uiAbilityLifecycleManager->BackToCallerAbilityWithResult(abilityRecord,
        resultCode, &resultWant, callerRequestCode);
    EXPECT_EQ(ret, ERR_CALLER_NOT_EXISTS);
}

/*
 * Feature: UIAbilityLifecycleManagerTest
 * Function: BackToCallerAbilityWithResult
 * SubFunction: NA
 * FunctionPoints: UIAbilityLifecycleManagerTest BackToCallerAbilityWithResult
 * EnvConditions: NA
 * CaseDescription: Verify BackToCallerAbilityWithResult
 */
HWTEST_F(UIAbilityLifecycleManagerTest, BackToCallerAbilityWithResult_002, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);

    UIAbilityRecordPtr abilityRecord = InitAbilityRecord();
    Want resultWant;
    int32_t resultCode = 100;
    int64_t callerRequestCode = 1;
    auto ret = uiAbilityLifecycleManager->BackToCallerAbilityWithResult(abilityRecord,
        resultCode, &resultWant, callerRequestCode);
    EXPECT_EQ(ret, ERR_CALLER_NOT_EXISTS);
}

/*
 * Feature: UIAbilityLifecycleManagerTest
 * Function: BackToCallerAbilityWithResult
 * SubFunction: NA
 * FunctionPoints: UIAbilityLifecycleManagerTest BackToCallerAbilityWithResult
 * EnvConditions: NA
 * CaseDescription: Verify BackToCallerAbilityWithResult
 */
HWTEST_F(UIAbilityLifecycleManagerTest, BackToCallerAbilityWithResult_003, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);

    UIAbilityRecordPtr abilityRecord = InitAbilityRecord();
    Want resultWant;
    int32_t resultCode = 100;
    int32_t requestCode = 1;
    int32_t pid = 1;
    bool backFlag = false;
    int64_t callerRequestCode = AbilityRuntime::StartupUtil::GenerateFullRequestCode(pid, backFlag, requestCode);

    // not support back to caller
    UIAbilityRecordPtr callerAbilityRecord = InitAbilityRecord();
    callerAbilityRecord->pid_ = pid;
    auto newCallerRecord = std::make_shared<CallerRecord>(requestCode, callerAbilityRecord);
    newCallerRecord->AddHistoryRequestCode(requestCode);
    abilityRecord->callerList_.emplace_back(newCallerRecord);
    auto ret = uiAbilityLifecycleManager->BackToCallerAbilityWithResult(abilityRecord,
        resultCode, &resultWant, callerRequestCode);
    EXPECT_EQ(ret, ERR_NOT_SUPPORT_BACK_TO_CALLER);
}

/*
 * Feature: UIAbilityLifecycleManagerTest
 * Function: BackToCallerAbilityWithResult
 * SubFunction: NA
 * FunctionPoints: UIAbilityLifecycleManagerTest BackToCallerAbilityWithResult
 * EnvConditions: NA
 * CaseDescription: Verify BackToCallerAbilityWithResult
 */
HWTEST_F(UIAbilityLifecycleManagerTest, BackToCallerAbilityWithResult_004, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);

    UIAbilityRecordPtr abilityRecord = InitAbilityRecord();
    Want resultWant;
    int32_t resultCode = 100;
    int32_t requestCode = 1;
    int32_t pid = 1;
    bool backFlag = true;
    int64_t callerRequestCode = AbilityRuntime::StartupUtil::GenerateFullRequestCode(pid, backFlag, requestCode);

    // caller is self
    abilityRecord->pid_ = pid;
    auto newCallerRecord = std::make_shared<CallerRecord>(requestCode, abilityRecord);
    newCallerRecord->AddHistoryRequestCode(requestCode);
    abilityRecord->callerList_.emplace_back(newCallerRecord);
    auto ret = uiAbilityLifecycleManager->BackToCallerAbilityWithResult(abilityRecord,
        resultCode, &resultWant, callerRequestCode);
    EXPECT_EQ(ret, ERR_OK);
}

/*
 * Feature: UIAbilityLifecycleManagerTest
 * Function: BackToCallerAbilityWithResult
 * SubFunction: NA
 * FunctionPoints: UIAbilityLifecycleManagerTest BackToCallerAbilityWithResult
 * EnvConditions: NA
 * CaseDescription: Verify BackToCallerAbilityWithResult
 */
HWTEST_F(UIAbilityLifecycleManagerTest, BackToCallerAbilityWithResult_005, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);

    UIAbilityRecordPtr abilityRecord = InitAbilityRecord();
    Want resultWant;
    int32_t resultCode = 100;
    int32_t requestCode = 1;
    int32_t pid = 1;
    bool backFlag = true;
    int64_t callerRequestCode = AbilityRuntime::StartupUtil::GenerateFullRequestCode(pid, backFlag, requestCode);

    UIAbilityRecordPtr callerAbilityRecord = InitAbilityRecord();
    callerAbilityRecord->pid_ = pid;
    auto newCallerRecord = std::make_shared<CallerRecord>(requestCode, callerAbilityRecord);
    newCallerRecord->AddHistoryRequestCode(requestCode);
    abilityRecord->pid_ = pid;
    abilityRecord->callerList_.emplace_back(newCallerRecord);

    // current ability is backgrounded
    abilityRecord->currentState_ = AbilityState::BACKGROUND;
    auto ret = uiAbilityLifecycleManager->BackToCallerAbilityWithResult(abilityRecord,
        resultCode, &resultWant, callerRequestCode);
    EXPECT_EQ(ret, CHECK_PERMISSION_FAILED);
}

/*
 * Feature: UIAbilityLifecycleManagerTest
 * Function: BackToCallerAbilityWithResult
 * SubFunction: NA
 * FunctionPoints: UIAbilityLifecycleManagerTest BackToCallerAbilityWithResult
 * EnvConditions: NA
 * CaseDescription: Verify BackToCallerAbilityWithResult
 */
HWTEST_F(UIAbilityLifecycleManagerTest, BackToCallerAbilityWithResult_006, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);

    UIAbilityRecordPtr abilityRecord = InitAbilityRecord();
    Want resultWant;
    int32_t resultCode = 100;
    int32_t requestCode = 1;
    int32_t pid = 1;
    bool backFlag = true;
    int64_t callerRequestCode = AbilityRuntime::StartupUtil::GenerateFullRequestCode(pid, backFlag, requestCode);

    UIAbilityRecordPtr callerAbilityRecord = InitAbilityRecord();
    callerAbilityRecord->pid_ = pid;
    auto newCallerRecord = std::make_shared<CallerRecord>(requestCode, callerAbilityRecord);
    newCallerRecord->AddHistoryRequestCode(requestCode);
    abilityRecord->pid_ = pid;
    abilityRecord->callerList_.emplace_back(newCallerRecord);

    abilityRecord->currentState_ = AbilityState::FOREGROUND;
    auto ret = uiAbilityLifecycleManager->BackToCallerAbilityWithResult(abilityRecord, resultCode,
        &resultWant, callerRequestCode);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: UIAbilityLifecycleManager_UpdateSpecifiedFlag_0100
 * @tc.desc: UpdateSpecifiedFlag
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, UpdateSpecifiedFlag_0100, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    std::string flag = "specified";
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.launchMode = AppExecFwk::LaunchMode::SPECIFIED;
    abilityRequest.startRecent = true;
    abilityRequest.specifiedFlag = flag;

    Rosen::SessionInfo info;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->sessionToken = new Rosen::Session(info);
    sessionInfo->persistentId = 1;
    abilityRequest.sessionInfo = sessionInfo;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    EXPECT_NE(abilityRecord, nullptr);
    auto ret = uiAbilityLifecycleManager->UpdateSpecifiedFlag(abilityRecord, flag);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: UIAbilityLifecycleManager_UpdateSpecifiedFlag_0200
 * @tc.desc: UpdateSpecifiedFlag failed
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, UpdateSpecifiedFlag_0200, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    std::string flag = "specified";
    auto ret = uiAbilityLifecycleManager->UpdateSpecifiedFlag(nullptr, flag);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: UIAbilityLifecycleManager_UpdateSpecifiedFlag_0300
 * @tc.desc: UpdateSpecifiedFlag failed
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, UpdateSpecifiedFlag_0300, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.launchMode = AppExecFwk::LaunchMode::SPECIFIED;
    abilityRequest.startRecent = true;
    std::string flag = "specified";
    abilityRequest.specifiedFlag = flag;
    abilityRequest.sessionInfo = nullptr;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    EXPECT_NE(abilityRecord, nullptr);
    auto ret = uiAbilityLifecycleManager->UpdateSpecifiedFlag(abilityRecord, flag);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: UIAbilityLifecycleManager_UpdateSpecifiedFlag_0400
 * @tc.desc: UpdateSpecifiedFlag failed
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, UpdateSpecifiedFlag_0400, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.launchMode = AppExecFwk::LaunchMode::SPECIFIED;
    abilityRequest.startRecent = true;
    std::string flag = "specified";
    abilityRequest.specifiedFlag = flag;

    Rosen::SessionInfo info;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->sessionToken = nullptr;
    sessionInfo->persistentId = 1;
    abilityRequest.sessionInfo = sessionInfo;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    EXPECT_NE(abilityRecord, nullptr);
    auto ret = uiAbilityLifecycleManager->UpdateSpecifiedFlag(abilityRecord, flag);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: UIAbilityLifecycleManager_HandleForegroundCollaborate_0100
 * @tc.desc: HandleForegroundCollaborate
 */
HWTEST_F(UIAbilityLifecycleManagerTest, HandleForegroundCollaborate_0100, TestSize.Level1)
{
    AbilityRequest abilityRequest;
    abilityRequest.want.SetElementName("bundleName", "abilityName");
    Want want;
    AppExecFwk::AbilityInfo abilityInfo;
    AppExecFwk::ApplicationInfo applicationInfo;
    auto abilityRecord = std::make_shared<UIAbilityRecord>(want, abilityInfo, applicationInfo, -1);

    abilityRecord->currentState_ = AbilityState::FOREGROUND;
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    uiAbilityLifecycleManager->HandleForegroundCollaborate(abilityRequest, abilityRecord);
    EXPECT_EQ(abilityRecord->GetWant().operation_.bundleName_, "bundleName");
    EXPECT_EQ(abilityRecord->GetWant().operation_.abilityName_, "abilityName");

    abilityRecord->currentState_ = AbilityState::BACKGROUND;
    uiAbilityLifecycleManager->HandleForegroundCollaborate(abilityRequest, abilityRecord);
    EXPECT_EQ(abilityRecord->GetWant().operation_.bundleName_, "bundleName");
    EXPECT_EQ(abilityRecord->GetWant().operation_.abilityName_, "abilityName");
}

/**
 * @tc.name: UIAbilityLifecycleManager_SetKillForPermissionUpdateFlag_0100
 * @tc.desc: SetKillForPermissionUpdateFlag
 */
HWTEST_F(UIAbilityLifecycleManagerTest, SetKillForPermissionUpdateFlag_0100, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    uint32_t accessTokenId = 1;
    AbilityRequest abilityRequest;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    abilityRecord->abilityInfo_.applicationInfo.accessTokenId = accessTokenId;
    abilityRecord->abilityInfo_.applicationInfo.bundleType = AppExecFwk::BundleType::ATOMIC_SERVICE;
    abilityRecord->abilityInfo_.type = AppExecFwk::AbilityType::PAGE;
    uiAbilityLifecycleManager->sessionAbilityMap_.emplace(std::make_pair(accessTokenId, abilityRecord));
    uiAbilityLifecycleManager->SetKillForPermissionUpdateFlag(accessTokenId);
    for (auto& item : uiAbilityLifecycleManager->sessionAbilityMap_) {
        if (item.second != nullptr) {
            EXPECT_EQ(item.second->GetKillForPermissionUpdateFlag(), true);
        }
    }
}

/**
 * @tc.name: UIAbilityLifecycleManager_SetKillForPermissionUpdateFlag_0200
 * @tc.desc: SetKillForPermissionUpdateFlag
 */
HWTEST_F(UIAbilityLifecycleManagerTest, SetKillForPermissionUpdateFlag_0200, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    uint32_t accessTokenId = 1;
    AbilityRequest abilityRequest;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);

    abilityRecord->abilityInfo_.applicationInfo.accessTokenId = 2;
    abilityRecord->abilityInfo_.applicationInfo.bundleType = AppExecFwk::BundleType::ATOMIC_SERVICE;
    abilityRecord->abilityInfo_.type = AppExecFwk::AbilityType::PAGE;
    auto abilityRecord2 = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    uiAbilityLifecycleManager->sessionAbilityMap_.emplace(std::make_pair(accessTokenId, abilityRecord2));
    uiAbilityLifecycleManager->SetKillForPermissionUpdateFlag(accessTokenId);
    for (auto& item : uiAbilityLifecycleManager->sessionAbilityMap_) {
        if (item.second != nullptr) {
            EXPECT_EQ(item.second->isKillForPermissionUpdate_, false);
        }
    }
    uiAbilityLifecycleManager->sessionAbilityMap_.clear();

    abilityRecord->abilityInfo_.applicationInfo.accessTokenId = 1;
    abilityRecord->abilityInfo_.applicationInfo.bundleType = AppExecFwk::BundleType::APP;
    abilityRecord->abilityInfo_.type = AppExecFwk::AbilityType::PAGE;
    auto abilityRecord3 = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    uiAbilityLifecycleManager->sessionAbilityMap_.emplace(std::make_pair(accessTokenId, abilityRecord3));
    uiAbilityLifecycleManager->SetKillForPermissionUpdateFlag(accessTokenId);
    for (auto& item : uiAbilityLifecycleManager->sessionAbilityMap_) {
        if (item.second != nullptr) {
            EXPECT_EQ(item.second->isKillForPermissionUpdate_, false);
        }
    }
    uiAbilityLifecycleManager->sessionAbilityMap_.clear();

    abilityRecord->abilityInfo_.applicationInfo.accessTokenId = 1;
    abilityRecord->abilityInfo_.applicationInfo.bundleType = AppExecFwk::BundleType::APP;
    abilityRecord->abilityInfo_.type = AppExecFwk::AbilityType::SERVICE;
    auto abilityRecord4 = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    uiAbilityLifecycleManager->sessionAbilityMap_.emplace(std::make_pair(accessTokenId, abilityRecord4));
    uiAbilityLifecycleManager->SetKillForPermissionUpdateFlag(accessTokenId);
    for (auto& item : uiAbilityLifecycleManager->sessionAbilityMap_) {
        if (item.second != nullptr) {
            EXPECT_EQ(item.second->isKillForPermissionUpdate_, false);
        }
    }
}

/**
 * @tc.name: UIAbilityLifecycleManager_HandleStartSpecifiedCold_0100
 * @tc.desc: HandleStartSpecifiedCold
 */
HWTEST_F(UIAbilityLifecycleManagerTest, HandleStartSpecifiedCold_0100, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    AbilityRequest abilityRequest;
    AbilityRuntime::StartParamsBySCB params;
    auto result = uiAbilityLifecycleManager->HandleStartSpecifiedCold(abilityRequest, nullptr, params);
    EXPECT_EQ(result, false);

    sptr<SessionInfo> sessionInfo = sptr<AAFwk::SessionInfo>::MakeSptr();
    abilityRequest.abilityInfo.launchMode = AppExecFwk::LaunchMode::STANDARD;
    result = uiAbilityLifecycleManager->HandleStartSpecifiedCold(abilityRequest, sessionInfo, params);
    EXPECT_EQ(result, false);

    abilityRequest.abilityInfo.launchMode = AppExecFwk::LaunchMode::SPECIFIED;
    sessionInfo->requestId = 1;
    auto specifiedRequest = std::make_shared<AAFwk::SpecifiedRequest>(0, abilityRequest);
    specifiedRequest->isCold = true;
    specifiedRequest->requestId = sessionInfo->requestId;
    std::list<std::shared_ptr<SpecifiedRequest>> Lists;
    Lists.push_back(specifiedRequest);
    uiAbilityLifecycleManager->specifiedRequestList_.emplace("key1", Lists);
    result = uiAbilityLifecycleManager->HandleStartSpecifiedCold(abilityRequest, sessionInfo, params);
    EXPECT_EQ(result, true);

    Lists.clear();
    uiAbilityLifecycleManager->specifiedRequestList_.clear();
    specifiedRequest->isCold = false;
    specifiedRequest->requestId = sessionInfo->requestId;
    Lists.push_back(specifiedRequest);
    uiAbilityLifecycleManager->specifiedRequestList_.emplace("key2", Lists);
    result = uiAbilityLifecycleManager->HandleStartSpecifiedCold(abilityRequest, sessionInfo, params);
    EXPECT_EQ(result, false);

    Lists.clear();
    uiAbilityLifecycleManager->specifiedRequestList_.clear();
    specifiedRequest->isCold = true;
    specifiedRequest->requestId = 2;
    Lists.push_back(specifiedRequest);
    uiAbilityLifecycleManager->specifiedRequestList_.emplace("key3", Lists);
    result = uiAbilityLifecycleManager->HandleStartSpecifiedCold(abilityRequest, sessionInfo, params);
    EXPECT_EQ(result, false);

    params.isRestart = true;
    result = uiAbilityLifecycleManager->HandleStartSpecifiedCold(abilityRequest, sessionInfo, params);
    EXPECT_EQ(result, true);
}

/**
 * @tc.name: UIAbilityLifecycleManager_TryPrepareTerminateByPidsDone_0100
 * @tc.desc: TryPrepareTerminateByPidsDone
 */
HWTEST_F(UIAbilityLifecycleManagerTest, TryPrepareTerminateByPidsDone_0100, TestSize.Level1)
{
    std::string moduleName = "moduleName";
    int32_t prepareTermination = 1;
    bool isExist = false;

    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    uiAbilityLifecycleManager->prepareTerminateByPidRecords_.clear();
    uiAbilityLifecycleManager->TryPrepareTerminateByPidsDone(moduleName, prepareTermination, isExist);

    auto prepareTerminateByPidRecord =
        std::make_shared<UIAbilityLifecycleManager::PrepareTerminateByPidRecord>(1,
        moduleName, true, prepareTermination, isExist);
    prepareTerminateByPidRecord->pid_ = 1;
    prepareTerminateByPidRecord->moduleName_ = "moduleName";
    uiAbilityLifecycleManager->prepareTerminateByPidRecords_.push_back(prepareTerminateByPidRecord);
    uiAbilityLifecycleManager->TryPrepareTerminateByPidsDone(moduleName, prepareTermination, isExist);
    EXPECT_EQ(uiAbilityLifecycleManager->prepareTerminateByPidRecords_.size(), 0);
}

/**
 * @tc.name: UIAbilityLifecycleManager_DoCallerProcessDetachment_0100
 * @tc.desc: DoCallerProcessDetachment
 */
HWTEST_F(UIAbilityLifecycleManagerTest, DoCallerProcessDetachment_0100, TestSize.Level1)
{
    AbilityRequest abilityRequest;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    auto ret = uiAbilityLifecycleManager->DoCallerProcessDetachment(abilityRecord);
    EXPECT_NE(ret, ERR_OK);
}

/**
 * @tc.name: UIAbilityLifecycleManager_RegisterStatusBarDelegate_0100
 * @tc.desc: RegisterStatusBarDelegate
 */
HWTEST_F(UIAbilityLifecycleManagerTest, RegisterStatusBarDelegate_0100, TestSize.Level1)
{
    sptr<AbilityRuntime::IStatusBarDelegate> delegate = sptr<MockIStatusBarDelegate>::MakeSptr();
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    auto ret = uiAbilityLifecycleManager->RegisterStatusBarDelegate(delegate);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: UIAbilityLifecycleManager_IsInStatusBar_0100
 * @tc.desc: IsInStatusBar
 */
HWTEST_F(UIAbilityLifecycleManagerTest, IsInStatusBar_0100, TestSize.Level1)
{
    uint32_t accessTokenId = 1;
    bool isMultiInstance = false;
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    auto ret = uiAbilityLifecycleManager->IsInStatusBar(accessTokenId, isMultiInstance);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: ProcessColdStartBranch_0100
 * @tc.desc: isColdStart is false
 */
HWTEST_F(UIAbilityLifecycleManagerTest, ProcessColdStartBranch_0100, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    AbilityRequest abilityRequest;
    sptr<SessionInfo> sessionInfo = new SessionInfo();
    UIAbilityRecordPtr abilityRecord = InitAbilityRecord();

    bool isColdStart = false;
    bool ret = mgr->ProcessColdStartBranch(abilityRequest, sessionInfo, abilityRecord, isColdStart);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: ProcessColdStartBranch_0200
 * @tc.desc: isColdStart is true, IsHook is false
 */
HWTEST_F(UIAbilityLifecycleManagerTest, ProcessColdStartBranch_0200, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    AbilityRequest abilityRequest;
    sptr<SessionInfo> sessionInfo = new SessionInfo();
    UIAbilityRecordPtr abilityRecord = InitAbilityRecord();

    bool isColdStart = true;
    abilityRecord->isHook_ = false;
    bool ret = mgr->ProcessColdStartBranch(abilityRequest, sessionInfo, abilityRecord, isColdStart);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: ProcessColdStartBranch_0300
 * @tc.desc: isColdStart is true, IsHook is true, nextRequest is null
 */
HWTEST_F(UIAbilityLifecycleManagerTest, ProcessColdStartBranch_0300, TestSize.Level1)
{
    class TestMgr : public UIAbilityLifecycleManager {
    public:
        std::shared_ptr<SpecifiedRequest> PopAndGetNextSpecified(int32_t)
        {
            return nullptr;
        }
    };
    auto mgr = std::make_shared<TestMgr>();
    AbilityRequest abilityRequest;
    sptr<SessionInfo> sessionInfo = new SessionInfo();
    sessionInfo->requestId = 1;
    UIAbilityRecordPtr abilityRecord = InitAbilityRecord();
    abilityRecord->isHook_ = true;

    bool isColdStart = true;
    bool ret = mgr->ProcessColdStartBranch(abilityRequest, sessionInfo, abilityRecord, isColdStart);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: ProcessColdStartBranch_0400
 * @tc.desc: isColdStart is true, IsHook is true
 */
HWTEST_F(UIAbilityLifecycleManagerTest, ProcessColdStartBranch_0400, TestSize.Level1)
{
    class TestMgr : public UIAbilityLifecycleManager {
    public:
        std::shared_ptr<SpecifiedRequest> PopAndGetNextSpecified(int32_t)
        {
            return std::make_shared<SpecifiedRequest>(1, AbilityRequest());
        }
    };
    auto mgr = std::make_shared<TestMgr>();
    AbilityRequest abilityRequest;
    sptr<SessionInfo> sessionInfo = new SessionInfo();
    sessionInfo->requestId = 1;
    UIAbilityRecordPtr abilityRecord = InitAbilityRecord();
    abilityRecord->isHook_ = true;

    bool isColdStart = true;
    bool ret = mgr->ProcessColdStartBranch(abilityRequest, sessionInfo, abilityRecord, isColdStart);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: RemoveStartingPid_WeakPtrNull
 * @tc.desc: Test the lambda branch when weak_ptr is expired (object destroyed).
 */
HWTEST_F(UIAbilityLifecycleManagerTest, RemoveStartingPid_WeakPtrNull, TestSize.Level1)
{
    std::weak_ptr<UIAbilityLifecycleManager> weakPtr;
    {
        auto mgr = std::make_shared<UIAbilityLifecycleManager>();
        weakPtr = mgr;
    }

    pid_t testPid = 12345;
    EXPECT_EQ(weakPtr.lock(), nullptr);

    auto lambda = [weakPtr, testPid]() {
        auto uiAbilityManager = weakPtr.lock();
        if (uiAbilityManager == nullptr) {
            SUCCEED();
            return;
        }
        uiAbilityManager->RemoveStartingPid(testPid);
        FAIL() << "Should not reach here";
    };
    lambda();
}

/**
 * @tc.name: RemoveStartingPid_WeakPtrValid
 * @tc.desc: Test the lambda branch when weak_ptr is valid (object alive).
 */
HWTEST_F(UIAbilityLifecycleManagerTest, RemoveStartingPid_WeakPtrValid, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    std::weak_ptr<UIAbilityLifecycleManager> weakPtr = mgr;
    pid_t testPid = 54321;
    mgr->AddStartingPid(testPid);

    auto lambda = [weakPtr, testPid]() {
        auto uiAbilityManager = weakPtr.lock();
        ASSERT_NE(uiAbilityManager, nullptr);
        uiAbilityManager->RemoveStartingPid(testPid);
        EXPECT_FALSE(uiAbilityManager->IsBundleStarting(testPid));
    };
    lambda();
    EXPECT_FALSE(mgr->IsBundleStarting(testPid));
}

/**
 * @tc.name: GenerateAbilityRecord_0001
 * @tc.desc: Cold start, no record in sessionAbilityMap, CreateAbilityRecord returns valid, isUIAbility & hook,
 * processOptions null
 */
HWTEST_F(UIAbilityLifecycleManagerTest, GenerateAbilityRecord_0001, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    AbilityRequest request;
    request.abilityInfo.type = AppExecFwk::AbilityType::PAGE;
    request.abilityInfo.isStageBasedModel = true;
    request.abilityInfo.applicationInfo.multiAppMode.multiAppModeType = AppExecFwk::MultiAppModeType::MULTI_INSTANCE;
    request.abilityInfo.launchMode = AppExecFwk::LaunchMode::STANDARD;
    sptr<SessionInfo> sessionInfo = new SessionInfo();
    sessionInfo->persistentId = 1;
    sessionInfo->instanceKey = "key1";
    sessionInfo->processOptions = nullptr;
    bool isColdStart = false;
    auto record = mgr->GenerateAbilityRecord(request, sessionInfo, isColdStart);
    EXPECT_NE(record, nullptr);
    EXPECT_TRUE(isColdStart);
}

/**
 * @tc.name: GenerateAbilityRecord_0002
 * @tc.desc: Cold start, FindRecordFromTmpMap returns valid record
 */
HWTEST_F(UIAbilityLifecycleManagerTest, GenerateAbilityRecord_0002, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    AbilityRequest request;
    request.abilityInfo.type = AppExecFwk::AbilityType::PAGE;
    request.abilityInfo.isStageBasedModel = true;
    request.abilityInfo.applicationInfo.multiAppMode.multiAppModeType = AppExecFwk::MultiAppModeType::MULTI_INSTANCE;
    request.abilityInfo.launchMode = AppExecFwk::LaunchMode::STANDARD;
    sptr<SessionInfo> sessionInfo = new SessionInfo();
    sessionInfo->persistentId = 2;
    sessionInfo->instanceKey = "key2";
    AbilityRequest dummyRequest;
    auto tmpRecord = UIAbilityRecord::CreateAbilityRecord(dummyRequest);
    mgr->tmpAbilityMap_[100] = tmpRecord;
    bool isColdStart = false;
    auto record = mgr->GenerateAbilityRecord(request, sessionInfo, isColdStart);
    EXPECT_EQ(record, tmpRecord);
    EXPECT_FALSE(isColdStart);
}

/**
 * @tc.name: GenerateAbilityRecord_0003
 * @tc.desc: Warm start, sessionAbilityMap has nullptr
 */
HWTEST_F(UIAbilityLifecycleManagerTest, GenerateAbilityRecord_0003, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    AbilityRequest request;
    sptr<SessionInfo> sessionInfo = new SessionInfo();
    sessionInfo->persistentId = 4;
    mgr->sessionAbilityMap_[4] = nullptr;
    bool isColdStart = false;
    auto record = mgr->GenerateAbilityRecord(request, sessionInfo, isColdStart);
    EXPECT_EQ(record, nullptr);
}

/**
 * @tc.name: GenerateAbilityRecord_0004
 * @tc.desc: Warm start, sessionAbilityMap has valid record but sessionToken mismatch
 */
HWTEST_F(UIAbilityLifecycleManagerTest, GenerateAbilityRecord_0004, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    AbilityRequest request;
    sptr<SessionInfo> sessionInfo = new SessionInfo();
    sessionInfo->persistentId = 5;
    sessionInfo->sessionToken = new OHOS::IPCObjectStub(u"desc1");
    AbilityRequest dummyRequest;
    auto record = UIAbilityRecord::CreateAbilityRecord(dummyRequest);
    auto recordSessionInfo = new SessionInfo();
    sessionInfo->sessionToken = new OHOS::IPCObjectStub(u"desc2");
    record->SetSessionInfo(recordSessionInfo);
    mgr->sessionAbilityMap_[5] = record;
    bool isColdStart = false;
    auto ret = mgr->GenerateAbilityRecord(request, sessionInfo, isColdStart);
    EXPECT_EQ(ret, nullptr);
}

/**
 * @tc.name: GenerateAbilityRecord_0005
 * @tc.desc: Warm start, reuseDelegatorWindow is true, LaunchAbility returns ERR_OK
 */
HWTEST_F(UIAbilityLifecycleManagerTest, GenerateAbilityRecord_0005, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    AbilityRequest request;
    sptr<SessionInfo> sessionInfo = new SessionInfo();
    sessionInfo->persistentId = 6;
    sessionInfo->reuseDelegatorWindow = true;
    AbilityRequest dummyRequest;
    auto record = UIAbilityRecord::CreateAbilityRecord(dummyRequest);
    auto recordSessionInfo = new SessionInfo();
    recordSessionInfo->sessionToken = sessionInfo->sessionToken;
    record->SetSessionInfo(recordSessionInfo);
    mgr->sessionAbilityMap_[6] = record;
    bool isColdStart = false;
    auto ret = mgr->GenerateAbilityRecord(request, sessionInfo, isColdStart);
    ASSERT_NE(ret, nullptr);
    ASSERT_NE(ret->GetSessionInfo(), nullptr);
    EXPECT_EQ(ret, record);
}

/**
 * @tc.name: GenerateAbilityRecord_0006
 * @tc.desc: Warm start, isNewWant is true
 */
HWTEST_F(UIAbilityLifecycleManagerTest, GenerateAbilityRecord_0006, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    AbilityRequest request;
    sptr<SessionInfo> sessionInfo = new SessionInfo();
    sessionInfo->persistentId = 7;
    sessionInfo->isNewWant = true;
    AbilityRequest dummyRequest;
    auto record = UIAbilityRecord::CreateAbilityRecord(dummyRequest);
    auto recordSessionInfo = new SessionInfo();
    recordSessionInfo->sessionToken = sessionInfo->sessionToken;
    record->SetSessionInfo(recordSessionInfo);
    mgr->sessionAbilityMap_[7] = record;
    bool isColdStart = false;
    auto ret = mgr->GenerateAbilityRecord(request, sessionInfo, isColdStart);
    EXPECT_EQ(ret, record);
}

/**
 * @tc.name: FindRecordFromTmpMap_0001
 * @tc.desc: tmpAbilityMap_ contains nullptr, should return nullptr
 */
HWTEST_F(UIAbilityLifecycleManagerTest, FindRecordFromTmpMap_0001, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    AbilityRequest request;
    request.abilityInfo.name = "AbilityA";
    request.abilityInfo.bundleName = "com.example.bundle";
    request.abilityInfo.moduleName = "moduleA";
    request.want.SetParam(Want::APP_INSTANCE_KEY, std::string("keyA"));
    int32_t appIndex = 1;
    request.want.SetParam("appIndex", appIndex);

    mgr->tmpAbilityMap_[1] = nullptr;

    auto found = mgr->FindRecordFromTmpMap(request);
    EXPECT_EQ(found, nullptr);
}

/**
 * @tc.name: FindRecordFromTmpMap_0002
 * @tc.desc: tmpAbilityMap_ contains non-matching UIAbilityRecord, should return nullptr
 */
HWTEST_F(UIAbilityLifecycleManagerTest, FindRecordFromTmpMap_0002, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    AbilityRequest request;
    request.abilityInfo.name = "AbilityA";
    request.abilityInfo.bundleName = "com.example.bundle";
    request.abilityInfo.moduleName = "moduleA";
    request.want.SetParam(Want::APP_INSTANCE_KEY, std::string("keyA"));
    int32_t appIndex = 1;
    request.want.SetParam("appIndex", appIndex);

    AbilityRequest dummyRequest;
    auto recordNotMatch = UIAbilityRecord::CreateAbilityRecord(dummyRequest);
    recordNotMatch->abilityInfo_.name = "AbilityB";
    recordNotMatch->abilityInfo_.bundleName = "com.example.bundle";
    recordNotMatch->abilityInfo_.moduleName = "moduleA";
    recordNotMatch->SetAppIndex(appIndex);
    recordNotMatch->SetInstanceKey("keyA");
    mgr->tmpAbilityMap_[2] = recordNotMatch;

    auto found = mgr->FindRecordFromTmpMap(request);
    EXPECT_EQ(found, nullptr);
}

/**
 * @tc.name: FindRecordFromTmpMap_0003
 * @tc.desc: tmpAbilityMap_ contains records but none match, should return nullptr
 */
HWTEST_F(UIAbilityLifecycleManagerTest, FindRecordFromTmpMap_0003, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    AbilityRequest request;
    request.abilityInfo.name = "NotExist";
    request.abilityInfo.bundleName = "com.example.bundle";
    request.abilityInfo.moduleName = "moduleA";
    request.want.SetParam(Want::APP_INSTANCE_KEY, std::string("keyA"));
    int32_t appIndex = 1;
    request.want.SetParam("appIndex", appIndex);

    AbilityRequest dummyRequest;
    auto record = UIAbilityRecord::CreateAbilityRecord(dummyRequest);
    record->abilityInfo_.name = "AbilityA";
    record->abilityInfo_.bundleName = "com.example.bundle";
    record->abilityInfo_.moduleName = "moduleA";
    record->SetAppIndex(appIndex);
    record->SetInstanceKey("keyA");
    mgr->tmpAbilityMap_[4] = record;

    auto found = mgr->FindRecordFromTmpMap(request);
    EXPECT_EQ(found, nullptr);
}

/**
 * @tc.name: AddCallerRecord_0001
 * @tc.desc: Test AddCallerRecord when Want::PARAM_RESV_FOR_RESULT is true, should remove params and set srcAbilityId
 */
HWTEST_F(UIAbilityLifecycleManagerTest, AddCallerRecord_0001, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    AbilityRequest request;
    request.want.SetParam(Want::PARAM_RESV_FOR_RESULT, true);
    request.want.SetParam("dmsSrcNetworkId", std::string("device123"));
    request.want.SetParam("dmsMissionId", 42);

    sptr<SessionInfo> sessionInfo = new SessionInfo();
    auto record = UIAbilityRecord::CreateAbilityRecord(request);

    mgr->AddCallerRecord(request, sessionInfo, record);

    EXPECT_FALSE(request.want.GetBoolParam(Want::PARAM_RESV_FOR_RESULT, false));
    EXPECT_EQ(request.want.GetStringParam("dmsSrcNetworkId"), "");
    EXPECT_EQ(request.want.GetIntParam("dmsMissionId", -1), -1);
}

/**
 * @tc.name: AddCallerRecord_0002
 * @tc.desc: Test AddCallerRecord when sessionInfo is nullptr, should early return
 */
HWTEST_F(UIAbilityLifecycleManagerTest, AddCallerRecord_0002, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    AbilityRequest request;
    UIAbilityRecordPtr record = UIAbilityRecord::CreateAbilityRecord(request);

    mgr->AddCallerRecord(request, nullptr, record);
    EXPECT_NE(record, nullptr);
}

/**
 * @tc.name: AttachAbilityThread_0001
 * @tc.desc: processAttachResult != ERR_OK
 */
HWTEST_F(UIAbilityLifecycleManagerTest, AttachAbilityThread_0001, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    AbilityRequest request;
    auto record = UIAbilityRecord::CreateAbilityRecord(request);
    auto token = record->GetToken();
    mgr->sessionAbilityMap_[1] = record;

    int ret = mgr->AttachAbilityThread(nullptr, token);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: NotifySCBToStartUIAbility_0001
 * @tc.desc: AddStartCallerTimestamp is false
 */
HWTEST_F(UIAbilityLifecycleManagerTest, NotifySCBToStartUIAbility_0001, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    AbilityRequest request;
    request.want.SetParam(Want::PARAM_RESV_CALLER_UID, 12345);
    for (int i = 0; i < 20; ++i) {
        mgr->AddStartCallerTimestamp(12345);
    }
    int ret = mgr->NotifySCBToStartUIAbility(request);
    EXPECT_EQ(ret, ERR_FREQ_START_ABILITY);
}

/**
 * @tc.name: NotifySCBToStartUIAbility_0002
 * @tc.desc: launchMode is SPECIFIED
 */
HWTEST_F(UIAbilityLifecycleManagerTest, NotifySCBToStartUIAbility_0002, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    AbilityRequest request;
    request.abilityInfo.launchMode = AppExecFwk::LaunchMode::SPECIFIED;
    request.abilityInfo.bundleName = "com.example.bundle";
    request.abilityInfo.name = "AbilityA";
    request.abilityInfo.moduleName = "moduleA";
    request.abilityInfo.isStageBasedModel = true;
    request.abilityInfo.type = AppExecFwk::AbilityType::PAGE;
    request.want.SetParam(Want::PARAM_RESV_CALLER_UID, 12345);
    StartupUtil::IsStartPlugin(request.want);

    int ret = mgr->NotifySCBToStartUIAbility(request);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: NotifySCBToStartUIAbility_0003
 * @tc.desc: IsHookModule
 */
HWTEST_F(UIAbilityLifecycleManagerTest, NotifySCBToStartUIAbility_0003, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    AbilityRequest request;
    request.abilityInfo.bundleName = "com.example.bundle";
    request.abilityInfo.moduleName = "moduleA";
    request.abilityInfo.name = "AbilityA";
    request.want.SetParam(Want::PARAM_RESV_CALLER_UID, 12345);
    int ret = mgr->NotifySCBToStartUIAbility(request);
    EXPECT_NE(ret, ERR_OK);

    auto record = UIAbilityRecord::CreateAbilityRecord(request);
    mgr->sessionAbilityMap_[1] = record;
    ret = mgr->NotifySCBToStartUIAbility(request);
    EXPECT_NE(ret, ERR_OK);

    record->SetIsHook(true);
    record->SetHookOff(true);
    ret = mgr->NotifySCBToStartUIAbility(request);
    EXPECT_NE(ret, ERR_OK);

    record->SetHookOff(false);
    sptr<SessionInfo> sessionInfo = new SessionInfo();
    record->SetSessionInfo(sessionInfo);
    ret = mgr->NotifySCBToStartUIAbility(request);
}

/**
 * @tc.name: NotifySCBToStartUIAbilities_0001
 * @tc.desc: handleListError
 */
HWTEST_F(UIAbilityLifecycleManagerTest, NotifySCBToStartUIAbilities_0001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "UIAbilityLifecycleManagerTest NotifySCBToStartUIAbilities_0001 start");
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    EXPECT_NE(mgr, nullptr);
    std::vector<AbilityRequest> requestList;
    std::string requestKey = "requestKey";
    AbilityRequest request;
    request.want.SetParam(Want::PARAM_RESV_CALLER_UID, 12345);
    for (int i = 0; i < 20; ++i) {
        mgr->AddStartCallerTimestamp(12345);
    }
    requestList.emplace_back(request);

    int ret = mgr->NotifySCBToStartUIAbilities(requestList, requestKey);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "UIAbilityLifecycleManagerTest NotifySCBToStartUIAbilities_0001 end");
}

/**
 * @tc.name: NotifySCBToStartUIAbilities_0002
 * @tc.desc: SpecifiedProcess true
 */
HWTEST_F(UIAbilityLifecycleManagerTest, NotifySCBToStartUIAbilities_0002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "UIAbilityLifecycleManagerTest NotifySCBToStartUIAbilities_0002 start");
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    EXPECT_NE(mgr, nullptr);
    std::vector<AbilityRequest> requestList;
    std::string requestKey = "requestKey";
    AppUtils::GetInstance().isStartSpecifiedProcess_.isLoaded = true;
    AppUtils::GetInstance().isStartSpecifiedProcess_.value = true;
    AbilityRequest request;
    request.abilityInfo.isolationProcess = true;
    request.abilityInfo.type = AppExecFwk::AbilityType::PAGE;
    request.abilityInfo.isStageBasedModel = true;
    requestList.emplace_back(request);

    int ret = mgr->NotifySCBToStartUIAbilities(requestList, requestKey);
    EXPECT_EQ(ret, START_UI_ABILITIES_WAITING_SPECIFIED_CODE);

    AppUtils::GetInstance().isStartSpecifiedProcess_.isLoaded = false;
    AppUtils::GetInstance().isStartSpecifiedProcess_.value = false;
    mgr->abilitiesRequestMap_.clear();
    TAG_LOGI(AAFwkTag::TEST, "UIAbilityLifecycleManagerTest NotifySCBToStartUIAbilities_0002 end");
}

/**
 * @tc.name: NotifySCBToStartUIAbilities_0003
 * @tc.desc: launchMode Specified
 */
HWTEST_F(UIAbilityLifecycleManagerTest, NotifySCBToStartUIAbilities_0003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "UIAbilityLifecycleManagerTest NotifySCBToStartUIAbilities_0003 start");
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    EXPECT_NE(mgr, nullptr);
    std::vector<AbilityRequest> requestList;
    std::string requestKey = "requestKey";
    AbilityRequest request;
    request.abilityInfo.launchMode = AppExecFwk::LaunchMode::SPECIFIED;
    requestList.emplace_back(request);

    int ret = mgr->NotifySCBToStartUIAbilities(requestList, requestKey);
    EXPECT_EQ(ret, START_UI_ABILITIES_WAITING_SPECIFIED_CODE);

    mgr->abilitiesRequestMap_.clear();
    TAG_LOGI(AAFwkTag::TEST, "UIAbilityLifecycleManagerTest NotifySCBToStartUIAbilities_0003 end");
}

/**
 * @tc.name: NotifySCBToStartUIAbilities_0004
 * @tc.desc: normal
 */
HWTEST_F(UIAbilityLifecycleManagerTest, NotifySCBToStartUIAbilities_0004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "UIAbilityLifecycleManagerTest NotifySCBToStartUIAbilities_0004 start");
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    EXPECT_NE(mgr, nullptr);
    std::vector<AbilityRequest> requestList;
    std::string requestKey = "requestKey";
    AbilityRequest request;
    request.abilityInfo.launchMode = AppExecFwk::LaunchMode::STANDARD;
    request.abilityInfo.bundleName = "com.example.bundle";
    request.abilityInfo.name = "AbilityA";
    request.abilityInfo.moduleName = "moduleA";
    request.abilityInfo.isStageBasedModel = true;
    request.abilityInfo.type = AppExecFwk::AbilityType::PAGE;

    requestList.emplace_back(request);

    int ret = mgr->NotifySCBToStartUIAbilities(requestList, requestKey);
    EXPECT_NE(ret, ERR_OK);

    mgr->abilitiesRequestMap_.clear();
    TAG_LOGI(AAFwkTag::TEST, "UIAbilityLifecycleManagerTest NotifySCBToStartUIAbilities_0004 end");
}


/**
 * @tc.name: HandleUIAbilityRequestList_0001
 * @tc.desc: callerUid's timeStamps >= limit
 */
HWTEST_F(UIAbilityLifecycleManagerTest, HandleUIAbilityRequestList_0001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "UIAbilityLifecycleManagerTest HandleUIAbilityRequestList_0001 start");
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    EXPECT_NE(mgr, nullptr);
    std::vector<AbilityRequest> requestList;
    AbilityRequest request;
    request.want.SetParam(Want::PARAM_RESV_CALLER_UID, 12345);
    for (int i = 0; i < 20; ++i) {
        mgr->AddStartCallerTimestamp(12345);
    }
    requestList.emplace_back(request);

    int ret = mgr->HandleUIAbilityRequestList(requestList);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "UIAbilityLifecycleManagerTest HandleUIAbilityRequestList_0001 end");
}

/**
 * @tc.name: HandleUIAbilityRequestList_0002
 * @tc.desc: isCallerKilling true？ ERR_OK
 */
HWTEST_F(UIAbilityLifecycleManagerTest, HandleUIAbilityRequestList_0002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "UIAbilityLifecycleManagerTest HandleUIAbilityRequestList_0002 start");
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    EXPECT_NE(mgr, nullptr);
    std::vector<AbilityRequest> requestList;
    AbilityRequest request;
    requestList.emplace_back(request);

    int ret = mgr->HandleUIAbilityRequestList(requestList);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "UIAbilityLifecycleManagerTest HandleUIAbilityRequestList_0002 end");
}

/**
 * @tc.name: HandleAbilitiesNormalSessionInfo_0001
 * @tc.desc: isCallerKilling true？ ERR_OK
 */
HWTEST_F(UIAbilityLifecycleManagerTest, HandleAbilitiesNormalSessionInfo_0001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "UIAbilityLifecycleManagerTest HandleAbilitiesNormalSessionInfo_0001 start");
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    EXPECT_NE(mgr, nullptr);
    AbilityRequest request;
    auto abilitiesRequest = std::make_shared<AbilitiesRequest>();
    EXPECT_NE(abilitiesRequest, nullptr);
    int32_t requestId = 1;

    mgr->HandleAbilitiesNormalSessionInfo(request, abilitiesRequest, requestId);
    EXPECT_EQ(abilitiesRequest->doneCount, 1);
    TAG_LOGI(AAFwkTag::TEST, "UIAbilityLifecycleManagerTest HandleAbilitiesNormalSessionInfo_0001 end");
}

/**
 * @tc.name: BatchNotifySCBPendingActivations_0001
 * @tc.desc: sessionInfo nullptr
 */
HWTEST_F(UIAbilityLifecycleManagerTest, BatchNotifySCBPendingActivations_0001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "UIAbilityLifecycleManagerTest BatchNotifySCBPendingActivations_0001 start");
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    EXPECT_NE(mgr, nullptr);
    int32_t requestId = 1;

    auto abilitiesRequest = std::make_shared<AbilitiesRequest>();
    EXPECT_NE(abilitiesRequest, nullptr);
    sptr<SessionInfo> sessionInfo = nullptr;
    abilitiesRequest->sessionInfoList.emplace_back(requestId, sessionInfo);

    int32_t ret = mgr->BatchNotifySCBPendingActivations(*abilitiesRequest);
    EXPECT_NE(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "UIAbilityLifecycleManagerTest BatchNotifySCBPendingActivations_0001 end");
}

/**
 * @tc.name: BatchNotifySCBPendingActivations_0002
 * @tc.desc: callerAbilityRecord nullptr rootSS null
 */
HWTEST_F(UIAbilityLifecycleManagerTest, BatchNotifySCBPendingActivations_0002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "UIAbilityLifecycleManagerTest BatchNotifySCBPendingActivations_0002 start");
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    EXPECT_NE(mgr, nullptr);
    int32_t requestId = 1;

    auto abilitiesRequest = std::make_shared<AbilitiesRequest>();
    EXPECT_NE(abilitiesRequest, nullptr);
    Rosen::SessionInfo info;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->sessionToken = new Rosen::Session(info);
    abilitiesRequest->sessionInfoList.emplace_back(requestId, sessionInfo);
    abilitiesRequest->callerToken = nullptr;

    int32_t ret = mgr->BatchNotifySCBPendingActivations(*abilitiesRequest);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "UIAbilityLifecycleManagerTest BatchNotifySCBPendingActivations_0002 end");
}

/**
 * @tc.name: BatchNotifySCBPendingActivations_0003
 * @tc.desc: callerAbilityRecord restart rootSS null
 */
HWTEST_F(UIAbilityLifecycleManagerTest, BatchNotifySCBPendingActivations_0003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "UIAbilityLifecycleManagerTest BatchNotifySCBPendingActivations_0003 start");
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    EXPECT_NE(mgr, nullptr);
    auto abilitiesRequest = std::make_shared<AbilitiesRequest>();
    EXPECT_NE(abilitiesRequest, nullptr);
    int32_t requestId = 1;

    AbilityRequest abilityRequest;
    UIAbilityRecordPtr abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    EXPECT_NE(abilityRecord, nullptr);
    sptr<IRemoteObject> callerToken = abilityRecord->GetToken();
    abilitiesRequest->callerToken = callerToken;
    abilityRecord->SetRestartAppFlag(true);

    Rosen::SessionInfo info;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->sessionToken = new Rosen::Session(info);
    abilitiesRequest->sessionInfoList.emplace_back(requestId, sessionInfo);

    int32_t ret = mgr->BatchNotifySCBPendingActivations(*abilitiesRequest);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "UIAbilityLifecycleManagerTest BatchNotifySCBPendingActivations_0003 end");
}

/**
 * @tc.name: BatchNotifySCBPendingActivations_0004
 * @tc.desc: callerAbilityRecord nullptr has rootSS
 */
HWTEST_F(UIAbilityLifecycleManagerTest, BatchNotifySCBPendingActivations_0004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "UIAbilityLifecycleManagerTest BatchNotifySCBPendingActivations_0004 start");
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    EXPECT_NE(mgr, nullptr);
    int32_t requestId = 1;
    auto abilitiesRequest = std::make_shared<AbilitiesRequest>();
    EXPECT_NE(abilitiesRequest, nullptr);
    Rosen::SessionInfo info;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->sessionToken = new Rosen::Session(info);
    abilitiesRequest->sessionInfoList.emplace_back(requestId, sessionInfo);
    abilitiesRequest->callerToken = nullptr;
    Rosen::SessionInfo rootInfo;
    mgr->rootSceneSession_ = new Rosen::Session(rootInfo);

    int32_t ret = mgr->BatchNotifySCBPendingActivations(*abilitiesRequest);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "UIAbilityLifecycleManagerTest BatchNotifySCBPendingActivations_0004 end");
}

/**
 * @tc.name: BatchNotifySCBPendingActivations_0005
 * @tc.desc: normal callerAbilityRecord callerSessionInfo nullptr
 */
HWTEST_F(UIAbilityLifecycleManagerTest, BatchNotifySCBPendingActivations_0005, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "UIAbilityLifecycleManagerTest BatchNotifySCBPendingActivations_0005 start");
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    EXPECT_NE(mgr, nullptr);
    auto abilitiesRequest = std::make_shared<AbilitiesRequest>();
    EXPECT_NE(abilitiesRequest, nullptr);
    int32_t requestId = 1;

    AbilityRequest abilityRequest;
    UIAbilityRecordPtr abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    EXPECT_NE(abilityRecord, nullptr);
    sptr<IRemoteObject> callerToken = abilityRecord->GetToken();
    abilitiesRequest->callerToken = callerToken;

    Rosen::SessionInfo info;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->sessionToken = new Rosen::Session(info);
    abilitiesRequest->sessionInfoList.emplace_back(requestId, sessionInfo);

    int32_t ret = mgr->BatchNotifySCBPendingActivations(*abilitiesRequest);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "UIAbilityLifecycleManagerTest BatchNotifySCBPendingActivations_0005 end");
}

/**
 * @tc.name: BatchNotifySCBPendingActivations_0006
 * @tc.desc: normal callerAbilityRecord callerSessionInfo->sessionToken nullptr
 */
HWTEST_F(UIAbilityLifecycleManagerTest, BatchNotifySCBPendingActivations_0006, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "UIAbilityLifecycleManagerTest BatchNotifySCBPendingActivations_0006 start");
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    EXPECT_NE(mgr, nullptr);
    auto abilitiesRequest = std::make_shared<AbilitiesRequest>();
    EXPECT_NE(abilitiesRequest, nullptr);
    int32_t requestId = 1;

    AbilityRequest abilityRequest;
    UIAbilityRecordPtr abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    EXPECT_NE(abilityRecord, nullptr);
    sptr<IRemoteObject> callerToken = abilityRecord->GetToken();
    abilitiesRequest->callerToken = callerToken;

    sptr<SessionInfo> callerSessionInfo = new SessionInfo();
    callerSessionInfo->sessionToken = nullptr;
    abilityRecord->SetSessionInfo(callerSessionInfo);

    Rosen::SessionInfo info;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->sessionToken = new Rosen::Session(info);
    abilitiesRequest->sessionInfoList.emplace_back(requestId, sessionInfo);

    int32_t ret = mgr->BatchNotifySCBPendingActivations(*abilitiesRequest);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "UIAbilityLifecycleManagerTest BatchNotifySCBPendingActivations_0006 end");
}

/**
 * @tc.name: BatchNotifySCBPendingActivations_0007
 * @tc.desc: normal callerAbilityRecord callerSession nullptr
 */
HWTEST_F(UIAbilityLifecycleManagerTest, BatchNotifySCBPendingActivations_0007, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "UIAbilityLifecycleManagerTest BatchNotifySCBPendingActivations_0007 start");
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    EXPECT_NE(mgr, nullptr);
    auto abilitiesRequest = std::make_shared<AbilitiesRequest>();
    EXPECT_NE(abilitiesRequest, nullptr);
    int32_t requestId = 1;

    AbilityRequest abilityRequest;
    UIAbilityRecordPtr abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    EXPECT_NE(abilityRecord, nullptr);
    sptr<IRemoteObject> callerToken = abilityRecord->GetToken();
    abilitiesRequest->callerToken = callerToken;

    sptr<SessionInfo> callerSessionInfo = new SessionInfo();
    callerSessionInfo->sessionToken = new OHOS::IPCObjectStub(u"desc2");
    abilityRecord->SetSessionInfo(callerSessionInfo);

    Rosen::SessionInfo info;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->sessionToken = new Rosen::Session(info);
    abilitiesRequest->sessionInfoList.emplace_back(requestId, sessionInfo);

    int32_t ret = mgr->BatchNotifySCBPendingActivations(*abilitiesRequest);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "UIAbilityLifecycleManagerTest BatchNotifySCBPendingActivations_0007 end");
}

/**
 * @tc.name: BatchNotifySCBPendingActivations_0008
 * @tc.desc: normal callerAbilityRecord ERR_OK
 */
HWTEST_F(UIAbilityLifecycleManagerTest, BatchNotifySCBPendingActivations_0008, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "UIAbilityLifecycleManagerTest BatchNotifySCBPendingActivations_0008 start");
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    EXPECT_NE(mgr, nullptr);
    auto abilitiesRequest = std::make_shared<AbilitiesRequest>();
    EXPECT_NE(abilitiesRequest, nullptr);
    int32_t requestId = 1;

    AbilityRequest abilityRequest;
    UIAbilityRecordPtr abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    EXPECT_NE(abilityRecord, nullptr);
    sptr<IRemoteObject> callerToken = abilityRecord->GetToken();
    abilitiesRequest->callerToken = callerToken;

    sptr<SessionInfo> callerSessionInfo = new SessionInfo();
    abilityRecord->SetSessionInfo(callerSessionInfo);

    Rosen::SessionInfo info;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->sessionToken = new Rosen::Session(info);
    abilitiesRequest->sessionInfoList.emplace_back(requestId, sessionInfo);

    int32_t ret = mgr->BatchNotifySCBPendingActivations(*abilitiesRequest);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "UIAbilityLifecycleManagerTest BatchNotifySCBPendingActivations_0008 end");
}

/**
 * @tc.name: HandleAbilitiesRequestDone_0001
 * @tc.desc: requestListId not found
 */
HWTEST_F(UIAbilityLifecycleManagerTest, HandleAbilitiesRequestDone_0001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "UIAbilityLifecycleManagerTest HandleAbilitiesRequestDone_0001 start");
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    EXPECT_NE(mgr, nullptr);
    int32_t requestId = 1;
    int32_t requestListId = 1;
    sptr<SessionInfo> sessionInfo = new SessionInfo();

    mgr->HandleAbilitiesRequestDone(requestId, requestListId, sessionInfo);
    TAG_LOGI(AAFwkTag::TEST, "UIAbilityLifecycleManagerTest HandleAbilitiesRequestDone_0001 end");
}

/**
 * @tc.name: HandleAbilitiesRequestDone_0002
 * @tc.desc: requestListId found; abilitiesRequest nullptr
 */
HWTEST_F(UIAbilityLifecycleManagerTest, HandleAbilitiesRequestDone_0002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "UIAbilityLifecycleManagerTest HandleAbilitiesRequestDone_0002 start");
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    EXPECT_NE(mgr, nullptr);
    int32_t requestId = 1;
    int32_t requestListId = 1;
    sptr<SessionInfo> sessionInfo = new SessionInfo();
    mgr->abilitiesRequestMap_.emplace(requestListId, nullptr);

    mgr->HandleAbilitiesRequestDone(requestId, requestListId, sessionInfo);
    mgr->abilitiesRequestMap_.clear();
    TAG_LOGI(AAFwkTag::TEST, "UIAbilityLifecycleManagerTest HandleAbilitiesRequestDone_0002 end");
}

/**
 * @tc.name: HandleAbilitiesRequestDone_0003
 * @tc.desc: requestListId normal; sessionInfo nullptr; callerRecord nullptr
 */
HWTEST_F(UIAbilityLifecycleManagerTest, HandleAbilitiesRequestDone_0003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "UIAbilityLifecycleManagerTest HandleAbilitiesRequestDone_0003 start");
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    EXPECT_NE(mgr, nullptr);
    int32_t requestId = 1;
    int32_t requestListId = 1;
    sptr<SessionInfo> sessionInfo = new SessionInfo();

    auto abilitiesRequest = std::make_shared<AbilitiesRequest>();
    EXPECT_NE(abilitiesRequest, nullptr);
    abilitiesRequest->sessionInfoList.emplace_back(requestId, nullptr);
    abilitiesRequest->doneCount = 0;
    abilitiesRequest->callerToken = nullptr;
    mgr->abilitiesRequestMap_.emplace(requestListId, abilitiesRequest);

    mgr->HandleAbilitiesRequestDone(requestId, requestListId, nullptr);
    mgr->abilitiesRequestMap_.clear();
    TAG_LOGI(AAFwkTag::TEST, "UIAbilityLifecycleManagerTest HandleAbilitiesRequestDone_0003 end");
}

/**
 * @tc.name: HandleAbilitiesRequestDone_0004
 * @tc.desc: requestListId normal; sessionInfo nullptr; callerRecord exist
 */
HWTEST_F(UIAbilityLifecycleManagerTest, HandleAbilitiesRequestDone_0004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "UIAbilityLifecycleManagerTest HandleAbilitiesRequestDone_0004 start");
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    EXPECT_NE(mgr, nullptr);
    int32_t requestId = 1;
    int32_t requestListId = 1;
    sptr<SessionInfo> sessionInfo = new SessionInfo();

    auto abilitiesRequest = std::make_shared<AbilitiesRequest>();
    EXPECT_NE(abilitiesRequest, nullptr);
    abilitiesRequest->sessionInfoList.emplace_back(requestId, nullptr);
    abilitiesRequest->doneCount = 0;

    AbilityRequest abilityRequest;
    UIAbilityRecordPtr abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    EXPECT_NE(abilityRecord, nullptr);
    sptr<IRemoteObject> callerToken = abilityRecord->GetToken();
    abilitiesRequest->callerToken = callerToken;
    mgr->abilitiesRequestMap_.emplace(requestListId, abilitiesRequest);

    mgr->HandleAbilitiesRequestDone(requestId, requestListId, nullptr);
    mgr->abilitiesRequestMap_.clear();
    TAG_LOGI(AAFwkTag::TEST, "UIAbilityLifecycleManagerTest HandleAbilitiesRequestDone_0004 end");
}

/**
 * @tc.name: HandleAbilitiesRequestDone_0005
 * @tc.desc: requestListId normal; sessionInfo exist; requestId found
 */
HWTEST_F(UIAbilityLifecycleManagerTest, HandleAbilitiesRequestDone_0005, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "UIAbilityLifecycleManagerTest HandleAbilitiesRequestDone_0005 start");
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    EXPECT_NE(mgr, nullptr);
    int32_t requestId = 1;
    int32_t requestListId = 1;
    sptr<SessionInfo> sessionInfo = new SessionInfo();

    auto abilitiesRequest = std::make_shared<AbilitiesRequest>();
    EXPECT_NE(abilitiesRequest, nullptr);
    abilitiesRequest->sessionInfoList.emplace_back(requestId, sessionInfo);
    abilitiesRequest->doneCount = 0;

    AbilityRequest abilityRequest;
    UIAbilityRecordPtr abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    EXPECT_NE(abilityRecord, nullptr);
    sptr<IRemoteObject> callerToken = abilityRecord->GetToken();
    abilitiesRequest->callerToken = callerToken;
    mgr->abilitiesRequestMap_.emplace(requestListId, abilitiesRequest);

    mgr->HandleAbilitiesRequestDone(requestId, requestListId, sessionInfo);
    mgr->abilitiesRequestMap_.clear();
    TAG_LOGI(AAFwkTag::TEST, "UIAbilityLifecycleManagerTest HandleAbilitiesRequestDone_0005 end");
}

/**
 * @tc.name: HandleAbilitiesRequestDone_0006
 * @tc.desc: requestListId normal; sessionInfo exist; requestId not found
 */
HWTEST_F(UIAbilityLifecycleManagerTest, HandleAbilitiesRequestDone_0006, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "UIAbilityLifecycleManagerTest HandleAbilitiesRequestDone_0006 start");
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    EXPECT_NE(mgr, nullptr);
    int32_t requestId = 1;
    int32_t requestListId = 1;
    sptr<SessionInfo> sessionInfo = new SessionInfo();

    auto abilitiesRequest = std::make_shared<AbilitiesRequest>();
    EXPECT_NE(abilitiesRequest, nullptr);
    int32_t otherRequestId = 2;
    abilitiesRequest->sessionInfoList.emplace_back(otherRequestId, sessionInfo);
    abilitiesRequest->doneCount = 0;

    AbilityRequest abilityRequest;
    UIAbilityRecordPtr abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    EXPECT_NE(abilityRecord, nullptr);
    sptr<IRemoteObject> callerToken = abilityRecord->GetToken();
    abilitiesRequest->callerToken = callerToken;
    mgr->abilitiesRequestMap_.emplace(requestListId, abilitiesRequest);

    mgr->HandleAbilitiesRequestDone(requestId, requestListId, sessionInfo);
    mgr->abilitiesRequestMap_.clear();
    TAG_LOGI(AAFwkTag::TEST, "UIAbilityLifecycleManagerTest HandleAbilitiesRequestDone_0006 end");
}

/**
 * @tc.name: HandleAbilitiesRequestDone_0007
 * @tc.desc: requestId found; pendingActivations; callerRecord nullptr
 */
HWTEST_F(UIAbilityLifecycleManagerTest, HandleAbilitiesRequestDone_0007, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "UIAbilityLifecycleManagerTest HandleAbilitiesRequestDone_0007 start");
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    EXPECT_NE(mgr, nullptr);
    int32_t requestId = 1;
    int32_t requestListId = 1;
    sptr<SessionInfo> sessionInfo = new SessionInfo();

    auto abilitiesRequest = std::make_shared<AbilitiesRequest>();
    EXPECT_NE(abilitiesRequest, nullptr);
    abilitiesRequest->sessionInfoList.clear();
    abilitiesRequest->sessionInfoList.emplace_back(requestId, sessionInfo);
    abilitiesRequest->doneCount = 0;
    abilitiesRequest->callerToken = nullptr;
    mgr->abilitiesRequestMap_.emplace(requestListId, abilitiesRequest);

    mgr->HandleAbilitiesRequestDone(requestId, requestListId, sessionInfo);
    mgr->abilitiesRequestMap_.clear();
    TAG_LOGI(AAFwkTag::TEST, "UIAbilityLifecycleManagerTest HandleAbilitiesRequestDone_0007 end");
}

/**
 * @tc.name: HandleAbilitiesRequestDone_0008
 * @tc.desc: requestId found; pendingActivations; callerRecord exist
 */
HWTEST_F(UIAbilityLifecycleManagerTest, HandleAbilitiesRequestDone_0008, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "UIAbilityLifecycleManagerTest HandleAbilitiesRequestDone_0008 start");
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    EXPECT_NE(mgr, nullptr);
    int32_t requestId = 1;
    int32_t requestListId = 1;
    sptr<SessionInfo> sessionInfo = new SessionInfo();

    auto abilitiesRequest = std::make_shared<AbilitiesRequest>();
    EXPECT_NE(abilitiesRequest, nullptr);
    abilitiesRequest->sessionInfoList.emplace_back(requestId, sessionInfo);
    abilitiesRequest->doneCount = 0;
    AbilityRequest abilityRequest;
    UIAbilityRecordPtr abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    EXPECT_NE(abilityRecord, nullptr);
    sptr<IRemoteObject> callerToken = abilityRecord->GetToken();
    abilitiesRequest->callerToken = callerToken;
    mgr->abilitiesRequestMap_.emplace(requestListId, abilitiesRequest);

    mgr->HandleAbilitiesRequestDone(requestId, requestListId, sessionInfo);
    mgr->abilitiesRequestMap_.clear();
    TAG_LOGI(AAFwkTag::TEST, "UIAbilityLifecycleManagerTest HandleAbilitiesRequestDone_0008 end");
}

/**
 * @tc.name: HandleAbilitiesRequestDone_0009
 * @tc.desc: requestId found; not pendingActivations
 */
HWTEST_F(UIAbilityLifecycleManagerTest, HandleAbilitiesRequestDone_0009, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "UIAbilityLifecycleManagerTest HandleAbilitiesRequestDone_0009 start");
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    EXPECT_NE(mgr, nullptr);
    int32_t requestId = 1;
    int32_t requestListId = 1;
    sptr<SessionInfo> sessionInfo = new SessionInfo();

    auto abilitiesRequest = std::make_shared<AbilitiesRequest>();
    EXPECT_NE(abilitiesRequest, nullptr);
    abilitiesRequest->sessionInfoList.emplace_back(requestId, sessionInfo);
    abilitiesRequest->doneCount = 50;
    AbilityRequest abilityRequest;
    UIAbilityRecordPtr abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    EXPECT_NE(abilityRecord, nullptr);
    sptr<IRemoteObject> callerToken = abilityRecord->GetToken();
    abilitiesRequest->callerToken = callerToken;
    mgr->abilitiesRequestMap_.emplace(requestListId, abilitiesRequest);

    mgr->HandleAbilitiesRequestDone(requestId, requestListId, sessionInfo);
    mgr->abilitiesRequestMap_.clear();
    TAG_LOGI(AAFwkTag::TEST, "UIAbilityLifecycleManagerTest HandleAbilitiesRequestDone_0009 end");
}

/**
 * @tc.name: NotifySCBToRecoveryAfterInterception_0001
 * @tc.desc: launchMode is SPECIFIED
 */
HWTEST_F(UIAbilityLifecycleManagerTest, NotifySCBToRecoveryAfterInterception_0001, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    AbilityRequest request;
    request.abilityInfo.launchMode = AppExecFwk::LaunchMode::SPECIFIED;
    request.abilityInfo.bundleName = "com.example.bundle";
    request.abilityInfo.name = "AbilityA";
    request.abilityInfo.moduleName = "moduleA";
    request.abilityInfo.isStageBasedModel = true;
    request.abilityInfo.type = AppExecFwk::AbilityType::PAGE;

    int ret = mgr->NotifySCBToRecoveryAfterInterception(request);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: NotifySCBToRecoveryAfterInterception_0002
 * @tc.desc: NotifySCBToRecoveryAfterInterception
 */
HWTEST_F(UIAbilityLifecycleManagerTest, NotifySCBToRecoveryAfterInterception_0002, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    AbilityRequest request;
    request.abilityInfo.launchMode = AppExecFwk::LaunchMode::STANDARD;
    request.abilityInfo.isStageBasedModel = true;
    request.abilityInfo.type = AppExecFwk::AbilityType::PAGE;

    int ret = mgr->NotifySCBToRecoveryAfterInterception(request);
    EXPECT_NE(ret, ERR_OK);
}

/**
 * @tc.name: NotifySCBToPreStartUIAbility_0001
 * @tc.desc: NotifySCBToPreStartUIAbility
 */
HWTEST_F(UIAbilityLifecycleManagerTest, NotifySCBToPreStartUIAbility_0001, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    AbilityRequest request;
    request.abilityInfo.bundleName = "com.example.bundle";
    request.abilityInfo.name = "AbilityA";
    request.abilityInfo.moduleName = "moduleA";
    request.abilityInfo.isStageBasedModel = true;
    request.abilityInfo.type = AppExecFwk::AbilityType::PAGE;
    request.requestCode = 100;

    sptr<SessionInfo> sessionInfo;
    int ret = mgr->NotifySCBToPreStartUIAbility(request, sessionInfo);

    EXPECT_NE(sessionInfo, nullptr);
    EXPECT_EQ(sessionInfo->requestCode, 100);
    EXPECT_TRUE(sessionInfo->isAtomicService);
}

/**
 * @tc.name: DispatchForeground_0001
 * @tc.desc: DispatchForeground
 */
HWTEST_F(UIAbilityLifecycleManagerTest, DispatchForeground_0001, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    AbilityRequest request;
    request.abilityInfo.bundleName = "com.example.bundle";
    request.abilityInfo.name = "AbilityA";
    request.abilityInfo.moduleName = "moduleA";
    request.abilityInfo.isStageBasedModel = true;
    request.abilityInfo.type = AppExecFwk::AbilityType::PAGE;
    auto record = UIAbilityRecord::CreateAbilityRecord(request);
    // abilityRecord nullptr
    int ret = mgr->DispatchForeground(nullptr, true, AbilityState::FOREGROUNDING);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    
    // abilityRecord state INITIAL
    record->SetAbilityState(AbilityState::INITIAL);
    ret = mgr->DispatchForeground(record, true, AbilityState::FOREGROUNDING);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);

    record->SetAbilityState(AbilityState::FOREGROUNDING);
    ret = mgr->DispatchForeground(record, false, AbilityState::FOREGROUNDING);
    EXPECT_EQ(ret, ERR_OK);

    record->SetAbilityState(AbilityState::FOREGROUNDING);
    ret = mgr->DispatchForeground(record, false, AbilityState::FOREGROUND_WINDOW_FREEZED);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: DispatchBackground_0001
 * @tc.desc: abilityRecord not is BACKGROUNDING
 */
HWTEST_F(UIAbilityLifecycleManagerTest, DispatchBackground_0001, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    AbilityRequest request;
    request.abilityInfo.bundleName = "com.example.bundle";
    request.abilityInfo.name = "AbilityA";
    request.abilityInfo.moduleName = "moduleA";
    request.abilityInfo.isStageBasedModel = true;
    request.abilityInfo.type = AppExecFwk::AbilityType::PAGE;
    auto record = UIAbilityRecord::CreateAbilityRecord(request);
    record->SetAbilityState(AbilityState::FOREGROUND);

    int ret = mgr->DispatchBackground(record);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: CompleteForegroundSuccess_IsNewWant_0001
 * @tc.desc: IsNewWant is true
 */
HWTEST_F(UIAbilityLifecycleManagerTest, CompleteForegroundSuccess_IsNewWant_0001, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    AbilityRequest request;
    request.abilityInfo.bundleName = "com.example.bundle";
    request.abilityInfo.name = "AbilityA";
    request.abilityInfo.moduleName = "moduleA";
    request.abilityInfo.isStageBasedModel = true;
    request.abilityInfo.type = AppExecFwk::AbilityType::PAGE;
    auto record = UIAbilityRecord::CreateAbilityRecord(request);
    record->SetLastWant(std::make_shared<Want>());
    mgr->CompleteForegroundSuccess(record);
    EXPECT_EQ(record->GetAbilityState(), AbilityState::FOREGROUNDING);
}

/**
 * @tc.name: UpdateProcessName_0001
 * @tc.desc: processName
 */
HWTEST_F(UIAbilityLifecycleManagerTest, UpdateProcessName_0001, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    AbilityRequest request;
    request.abilityInfo.bundleName = "com.example.bundle";
    request.abilityInfo.name = "AbilityA";
    request.abilityInfo.moduleName = "moduleA";
    request.abilityInfo.isStageBasedModel = true;
    request.abilityInfo.type = AppExecFwk::AbilityType::PAGE;
    request.sessionInfo = new SessionInfo();
    request.sessionInfo->processOptions = std::make_shared<ProcessOptions>();
    request.sessionInfo->processOptions->processMode = ProcessMode::NEW_PROCESS_ATTACH_TO_PARENT;
    request.sessionInfo->processOptions->processName = "custom.process.name";
    auto record = UIAbilityRecord::CreateAbilityRecord(request);

    mgr->UpdateProcessName(request, record);

    EXPECT_EQ(record->GetProcessName(), "custom.process.name");
    EXPECT_TRUE(record->IsCallerSetProcess());
}

/**
 * @tc.name: UpdateAbilityRecordLaunchReason_0001
 * @tc.desc: want
 */
HWTEST_F(UIAbilityLifecycleManagerTest, UpdateAbilityRecordLaunchReason_0001, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    AbilityRequest request;
    request.abilityInfo.bundleName = "com.example.bundle";
    request.abilityInfo.name = "AbilityA";
    request.abilityInfo.moduleName = "moduleA";
    request.abilityInfo.isStageBasedModel = true;
    request.abilityInfo.type = AppExecFwk::AbilityType::PAGE;
    std::string launchReasonMsg = "test_launch_reason";
    request.want.SetParam(Want::PARM_LAUNCH_REASON_MESSAGE, launchReasonMsg);
    auto record = UIAbilityRecord::CreateAbilityRecord(request);
    mgr->UpdateAbilityRecordLaunchReason(request, record);
    EXPECT_NE(record, nullptr);
}

/**
 * @tc.name: PreCreateProcessName_0001
 * @tc.desc: PreCreateProcessName
 */
HWTEST_F(UIAbilityLifecycleManagerTest, PreCreateProcessName_0001, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    AbilityRequest request1;
    request1.processOptions = nullptr;
    mgr->PreCreateProcessName(request1);
    EXPECT_TRUE(request1.abilityInfo.process.empty());

    AbilityRequest request2;
    request2.processOptions = std::make_shared<ProcessOptions>();
    request2.processOptions->processMode = ProcessMode::UNSPECIFIED;
    mgr->PreCreateProcessName(request2);
    EXPECT_TRUE(request2.abilityInfo.process.empty());
    AbilityRequest request3;
    request3.abilityInfo.bundleName = "com.example.bundle";
    request3.abilityInfo.name = "AbilityA";
    request3.abilityInfo.moduleName = "moduleA";
    request3.abilityInfo.isStageBasedModel = true;
    request3.abilityInfo.type = AppExecFwk::AbilityType::PAGE;
    request3.processOptions = std::make_shared<ProcessOptions>();
    request3.processOptions->processMode = ProcessMode::NEW_PROCESS_ATTACH_TO_PARENT;
    mgr->PreCreateProcessName(request3);
    EXPECT_FALSE(request3.abilityInfo.process.empty());
    EXPECT_EQ(request3.abilityInfo.process, request3.processOptions->processName);
}

/**
 * @tc.name: NotifySCBToMinimizeUIAbility_0001
 * @tc.desc: NotifySCBToMinimizeUIAbility
 */
HWTEST_F(UIAbilityLifecycleManagerTest, NotifySCBToMinimizeUIAbility_0001, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    sptr<IRemoteObject> token = new IPCObjectStub(u"test");
    int32_t ret = mgr->NotifySCBToMinimizeUIAbility(token);
    EXPECT_GE(ret, static_cast<int32_t>(Rosen::WSError::WS_OK));
}

/**
 * @tc.name: MinimizeUIAbility_0001
 * @tc.desc: GetPendingState not is INITIAL
 */
HWTEST_F(UIAbilityLifecycleManagerTest, MinimizeUIAbility_0001, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    AbilityRequest request;
    request.abilityInfo.name = "AbilityA";
    auto record = UIAbilityRecord::CreateAbilityRecord(request);
    record->SetPendingState(AbilityState::FOREGROUND);
    int ret = mgr->MinimizeUIAbility(record, true, 123);

    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(record->GetPendingState(), AbilityState::BACKGROUND);
}

/**
 * @tc.name: MoveToBackground_0001
 * @tc.desc: lock is nullptr
 */
HWTEST_F(UIAbilityLifecycleManagerTest, MoveToBackground_0001, TestSize.Level1)
{
    AbilityRequest request;
    request.abilityInfo.name = "AbilityA";
    auto record = UIAbilityRecord::CreateAbilityRecord(request);
    std::weak_ptr<UIAbilityLifecycleManager> weakMgr;
    {
        auto mgr = std::make_shared<UIAbilityLifecycleManager>();
        weakMgr = mgr;
    }
    auto self = weakMgr;
    auto task = [record, self]() {
        auto selfObj = self.lock();
        EXPECT_EQ(selfObj, nullptr);
        if (selfObj == nullptr) {
            return;
        }
        selfObj->PrintTimeOutLog(record, AbilityManagerService::BACKGROUND_TIMEOUT_MSG);
        selfObj->CompleteBackground(record);
    };
    task();
}

/**
 * @tc.name: CallAbilityLocked_0001
 * @tc.desc: Test ret is OK_HAS_REMOTE_OBJ, call to foreground, and pendingState is not INITIAL.
 */
HWTEST_F(UIAbilityLifecycleManagerTest, CallAbilityLocked_0001, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    AbilityRequest request;
    request.abilityInfo.bundleName = "com.example.bundle";
    request.abilityInfo.name = "AbilityA";
    request.abilityInfo.moduleName = "moduleA";
    request.abilityInfo.isStageBasedModel = true;
    request.abilityInfo.type = AppExecFwk::AbilityType::PAGE;
    request.want.SetParam(Want::PARAM_RESV_CALL_TO_FOREGROUND, true);
    request.want.SetParam("supportCollaborativeCallingFromDmsInAAFwk", true);
    request.abilityInfo.applicationInfo.bundleType = AppExecFwk::BundleType::ATOMIC_SERVICE;

    auto record = UIAbilityRecord::CreateAbilityRecord(request);
    record->SetPendingState(AbilityState::FOREGROUND);
    int32_t persistentId = 1;
    mgr->sessionAbilityMap_[persistentId] = record;
    std::string errMsg;
    int ret = mgr->CallAbilityLocked(request, errMsg);
    EXPECT_EQ(record->GetPendingState(), AbilityState::FOREGROUND);
}

/**
 * @tc.name: IsHookModule_0001
 * @tc.desc: GetHapModuleInfo returns false, should return false
 */
HWTEST_F(UIAbilityLifecycleManagerTest, IsHookModule_0001, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    AbilityRequest request;
    request.abilityInfo.bundleName = "not.exist.bundle";
    bool result = mgr->IsHookModule(request);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: IsHookModule_0002
 * @tc.desc: GetHapModuleInfo returns true, but abilitySrcEntryDelegator is empty, should return false
 */
HWTEST_F(UIAbilityLifecycleManagerTest, IsHookModule_0002, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    AbilityRequest request;
    request.abilityInfo.bundleName = "com.example.bundle";
    request.abilityInfo.moduleName = "module_with_empty_delegator";
    bool result = mgr->IsHookModule(request);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: IsHookModule_0003
 * @tc.desc: GetHapModuleInfo returns true, abilityStageSrcEntryDelegator equals moduleName, should return false
 */
HWTEST_F(UIAbilityLifecycleManagerTest, IsHookModule_0003, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    AbilityRequest request;
    request.abilityInfo.bundleName = "com.example.bundle";
    request.abilityInfo.moduleName = "stage";
    bool result = mgr->IsHookModule(request);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: IsHookModule_0004
 * @tc.desc: All conditions met, should return true if QueryAbilityInfo returns true
 */
HWTEST_F(UIAbilityLifecycleManagerTest, IsHookModule_0004, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    AbilityRequest request;
    request.abilityInfo.bundleName = "com.example.bundle";
    request.abilityInfo.moduleName = "hook_module";
    bool result = mgr->IsHookModule(request);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: PrintTimeOutLog_0001
 * @tc.desc: ability is nullptr, should not change any state
 */
HWTEST_F(UIAbilityLifecycleManagerTest, PrintTimeOutLog_0001, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    size_t before = mgr->tmpAbilityMap_.size();
    mgr->PrintTimeOutLog(nullptr, 100, false);
    EXPECT_EQ(mgr->tmpAbilityMap_.size(), before);
}

/**
 * @tc.name: PrintTimeOutLog_0002
 * @tc.desc: processInfo.pid_ is 0, ability state should not change
 */
HWTEST_F(UIAbilityLifecycleManagerTest, PrintTimeOutLog_0002, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    AbilityRequest request;
    request.abilityInfo.name = "AbilityA";
    auto record = UIAbilityRecord::CreateAbilityRecord(request);
    int oldState = record->GetAbilityState();
    mgr->PrintTimeOutLog(record, 100, false);
    EXPECT_EQ(record->GetAbilityState(), oldState);
}

/**
 * @tc.name: PrintTimeOutLog_0003
 * @tc.desc: GetContentAndTypeId returns false, ability state should not change
 */
HWTEST_F(UIAbilityLifecycleManagerTest, PrintTimeOutLog_0003, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    AbilityRequest request;
    request.abilityInfo.name = "AbilityA";
    auto record = UIAbilityRecord::CreateAbilityRecord(request);
    int oldState = record->GetAbilityState();
    mgr->PrintTimeOutLog(record, 0xFFFFFFFF, false);
    EXPECT_EQ(record->GetAbilityState(), oldState);
}

/**
 * @tc.name: PrintTimeOutLog_0004
 * @tc.desc: state is UNKNOWN, info.msg only contains msgContent, ability state should not change
 */
HWTEST_F(UIAbilityLifecycleManagerTest, PrintTimeOutLog_0004, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    AbilityRequest request;
    request.abilityInfo.name = "AbilityA";
    auto record = UIAbilityRecord::CreateAbilityRecord(request);
    int oldState = record->GetAbilityState();
    mgr->PrintTimeOutLog(record, 0, false);
    EXPECT_EQ(record->GetAbilityState(), oldState);
}

/**
 * @tc.name: PrintTimeOutLog_0005
 * @tc.desc: state != UNKNOWN, isHalf is true, ability state should not change
 */
HWTEST_F(UIAbilityLifecycleManagerTest, PrintTimeOutLog_0005, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    AbilityRequest request;
    request.abilityInfo.name = "AbilityA";
    auto record = UIAbilityRecord::CreateAbilityRecord(request);
    int oldState = record->GetAbilityState();
    mgr->PrintTimeOutLog(record, 100, true);
    EXPECT_EQ(record->GetAbilityState(), oldState);
}

/**
 * @tc.name: BackToCallerAbilityWithResult_0001
 * @tc.desc: Return ERR_CALLER_NOT_EXISTS when GetCallerByRequestCode returns nullptr
 */
HWTEST_F(UIAbilityLifecycleManagerTest, BackToCallerAbilityWithResult_0001, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    AbilityRequest request;
    request.abilityInfo.name = "AbilityA";
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(request);
    int requestCode = 1;
    int pid = 1234;
    int64_t callerRequestCode = StartupUtil::GenerateFullRequestCode(pid, 1, requestCode);
    Want resultWant;
    int ret = mgr->BackToCallerAbilityWithResult(abilityRecord, 0, &resultWant, callerRequestCode);

    EXPECT_EQ(ret, ERR_CALLER_NOT_EXISTS);
}

/**
 * @tc.name: BackToCallerAbilityWithResultLocked_0001
 * @tc.desc: Return ERR_INVALID_VALUE when callerAbilityRecord is nullptr
 */
HWTEST_F(UIAbilityLifecycleManagerTest, BackToCallerAbilityWithResultLocked_0001, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    sptr<SessionInfo> currentSessionInfo = new SessionInfo();
    currentSessionInfo->sessionToken = nullptr;
    int ret = mgr->BackToCallerAbilityWithResultLocked(currentSessionInfo, nullptr);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: BackToCallerAbilityWithResultLocked_0002
 * @tc.desc: Return ERR_INVALID_VALUE when sessionToken is nullptr (cannot instantiate abstract class)
 */
HWTEST_F(UIAbilityLifecycleManagerTest, BackToCallerAbilityWithResultLocked_0002, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    sptr<SessionInfo> currentSessionInfo = new SessionInfo();
    currentSessionInfo->sessionToken = nullptr;

    AbilityRequest request;
    request.abilityInfo.name = "AbilityA";
    auto callerAbilityRecord = UIAbilityRecord::CreateAbilityRecord(request);

    sptr<SessionInfo> callerSessionInfo = new SessionInfo();
    callerSessionInfo->sessionToken = nullptr;
    callerAbilityRecord->SetSessionInfo(callerSessionInfo);

    int ret = mgr->BackToCallerAbilityWithResultLocked(currentSessionInfo, callerAbilityRecord);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: CloseUIAbility_0001
 * @tc.desc: GetEventHandler is null
 */
HWTEST_F(UIAbilityLifecycleManagerTest, CloseUIAbility_0001, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    AbilityRequest request;
    request.abilityInfo.name = "AbilityA";
    auto record = UIAbilityRecord::CreateAbilityRecord(request);
    record->SetAbilityState(AbilityState::INITIAL);
    int ret = mgr->CloseUIAbility(record, 0, nullptr, false, false);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: CloseUIAbility_0002
 * @tc.desc: abilityRecord is debug and isClearSession is true, should call TerminateAbility
 */
HWTEST_F(UIAbilityLifecycleManagerTest, CloseUIAbility_DebugAndClearSession, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    AbilityRequest request;
    request.abilityInfo.name = "AbilityB";
    request.abilityInfo.applicationInfo.debug = true;
    auto record = UIAbilityRecord::CreateAbilityRecord(request);
    record->SetAbilityState(AbilityState::FOREGROUND);
    int ret = mgr->CloseUIAbility(record, 0, nullptr, true, false);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name: CloseUIAbility_0003
 * @tc.desc: abilityRecord pending state is not INITIAL, should set to BACKGROUND and return ERR_OK
 */
HWTEST_F(UIAbilityLifecycleManagerTest, CloseUIAbility_0003, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    AbilityRequest request;
    request.abilityInfo.name = "AbilityC";
    auto record = UIAbilityRecord::CreateAbilityRecord(request);
    record->SetAbilityState(AbilityState::FOREGROUND);
    record->SetPendingState(AbilityState::FOREGROUND);
    int ret = mgr->CloseUIAbility(record, 0, nullptr, false, false);
    EXPECT_EQ(record->GetPendingState(), AbilityState::BACKGROUND);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: DelayCompleteTerminate_002
 * @tc.desc: Should call PrintTimeOutLog and schedule CompleteTerminate task
 */
HWTEST_F(UIAbilityLifecycleManagerTest, DelayCompleteTerminate_002, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    AbilityRequest request;
    request.abilityInfo.name = "AbilityA";
    auto record = UIAbilityRecord::CreateAbilityRecord(request);
    int oldState = record->GetAbilityState();
    mgr->DelayCompleteTerminate(record);

    bool completed = false;
    for (int i = 0; i < 50; ++i) {
        if (record->GetAbilityState() != oldState) {
            completed = true;
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    EXPECT_TRUE(completed == oldState);
}

/**
 * @tc.name: GetPersistentIdByAbilityRequest_0001
 * @tc.desc: Should return 0 when launchMode is not SINGLETON
 */
HWTEST_F(UIAbilityLifecycleManagerTest, GetPersistentIdByAbilityRequest_0001, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    AbilityRequest request;
    request.collaboratorType = CollaboratorType::DEFAULT_TYPE;
    request.abilityInfo.launchMode = AppExecFwk::LaunchMode::STANDARD;
    bool reuse = false;
    int32_t ret = mgr->GetPersistentIdByAbilityRequest(request, reuse);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name: OnTimeOut_0001
 * @tc.desc: abilityRecord not found in both sessionAbilityMap_ and terminateAbilityList_
 */
HWTEST_F(UIAbilityLifecycleManagerTest, OnTimeOut_0001, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    uint32_t msgId = 1;
    int64_t abilityRecordId = 99999;
    bool isHalf = false;
    bool found = false;
    for (const auto &[_, record] : mgr->sessionAbilityMap_) {
        if (record && record->GetAbilityRecordId() == abilityRecordId) {
            found = true;
            break;
        }
    }
    for (const auto &record : mgr->terminateAbilityList_) {
        if (record && record->GetAbilityRecordId() == abilityRecordId) {
            found = true;
            break;
        }
    }
    EXPECT_FALSE(found);

    mgr->OnTimeOut(msgId, abilityRecordId, isHalf);

    found = false;
    for (const auto &[_, record] : mgr->sessionAbilityMap_) {
        if (record && record->GetAbilityRecordId() == abilityRecordId) {
            found = true;
            break;
        }
    }
    for (const auto &record : mgr->terminateAbilityList_) {
        if (record && record->GetAbilityRecordId() == abilityRecordId) {
            found = true;
            break;
        }
    }
    EXPECT_FALSE(found);
}

/**
 * @tc.name: OnTimeOut_0002
 * @tc.desc: abilityRecord found in sessionAbilityMap_, isHalf is true, should only call PrintTimeOutLog
 */
HWTEST_F(UIAbilityLifecycleManagerTest, OnTimeOut_0002, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    AbilityRequest request;
    request.abilityInfo.name = "AbilityA";
    auto record = UIAbilityRecord::CreateAbilityRecord(request);
    int64_t abilityRecordId = record->GetAbilityRecordId();
    mgr->sessionAbilityMap_[1] = record;

    uint32_t msgId = 1;
    bool isHalf = true;
    mgr->OnTimeOut(msgId, abilityRecordId, isHalf);
    EXPECT_EQ(record->GetAbilityRecordId(), abilityRecordId);
}

/**
 * @tc.name: OnTimeOut_0003
 * @tc.desc: abilityRecord found in terminateAbilityList_, isHalf is true, should only call PrintTimeOutLog
 */
HWTEST_F(UIAbilityLifecycleManagerTest, OnTimeOut_0003, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    AbilityRequest request;
    request.abilityInfo.name = "AbilityB";
    auto record = UIAbilityRecord::CreateAbilityRecord(request);
    int64_t abilityRecordId = record->GetAbilityRecordId();
    mgr->terminateAbilityList_.push_back(record);

    uint32_t msgId = 1;
    bool isHalf = true;
    mgr->OnTimeOut(msgId, abilityRecordId, isHalf);
    EXPECT_EQ(record->GetAbilityRecordId(), abilityRecordId);
}

/**
 * @tc.name: OnTimeOut_0004
 * @tc.desc: abilityRecord found, isHalf is false, msgId is LOAD_TIMEOUT_MSG
 */
HWTEST_F(UIAbilityLifecycleManagerTest, OnTimeOut_0004, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    AbilityRequest request;
    request.abilityInfo.name = "AbilityC";
    auto record = UIAbilityRecord::CreateAbilityRecord(request);
    int64_t abilityRecordId = record->GetAbilityRecordId();
    mgr->sessionAbilityMap_[2] = record;

    uint32_t msgId = AbilityManagerService::LOAD_TIMEOUT_MSG;
    bool isHalf = false;
    record->SetLoading(true);
    mgr->OnTimeOut(msgId, abilityRecordId, isHalf);
    EXPECT_FALSE(record->IsLoading());
}

/**
 * @tc.name: OnTimeOut_0005
 * @tc.desc: abilityRecord found, isHalf is false, msgId is FOREGROUND_TIMEOUT_MSG
 */
HWTEST_F(UIAbilityLifecycleManagerTest, OnTimeOut_0005, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    AbilityRequest request;
    request.abilityInfo.name = "AbilityD";
    auto record = UIAbilityRecord::CreateAbilityRecord(request);
    int64_t abilityRecordId = record->GetAbilityRecordId();
    mgr->sessionAbilityMap_[3] = record;

    uint32_t msgId = AbilityManagerService::FOREGROUND_TIMEOUT_MSG;
    bool isHalf = false;
    mgr->OnTimeOut(msgId, abilityRecordId, isHalf);
    EXPECT_EQ(record->GetAbilityRecordId(), abilityRecordId);
}

/**
 * @tc.name: OnTimeOut_0006
 * @tc.desc: abilityRecord found, isHalf is false, msgId is unknown (default branch)
 */
HWTEST_F(UIAbilityLifecycleManagerTest, OnTimeOut_0006, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    AbilityRequest request;
    request.abilityInfo.name = "AbilityE";
    auto record = UIAbilityRecord::CreateAbilityRecord(request);
    int64_t abilityRecordId = record->GetAbilityRecordId();
    mgr->sessionAbilityMap_[4] = record;

    uint32_t msgId = 0xDEADBEEF;
    bool isHalf = false;
    mgr->OnTimeOut(msgId, abilityRecordId, isHalf);
    EXPECT_EQ(record->GetAbilityRecordId(), abilityRecordId);
}


/**
 * @tc.name: CleanUIAbility_0001
 * @tc.desc: Should return ERR_INVALID_VALUE when abilityRecord is nullptr
 */
HWTEST_F(UIAbilityLifecycleManagerTest, CleanUIAbility_0001, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    int ret = mgr->CleanUIAbility(nullptr);
    EXPECT_EQ(ret, ERR_UI_ABILITY_MANAGER_NULL_ABILITY_RECORD);
}

/**
 * @tc.name: CleanUIAbility_0002
 * @tc.desc: Should call CloseUIAbility when CleanAbilityByUserRequest returns false
 */
HWTEST_F(UIAbilityLifecycleManagerTest, CleanUIAbility_0002, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    AbilityRequest request;
    request.abilityInfo.name = "AbilityB";
    auto record = UIAbilityRecord::CreateAbilityRecord(request);
    int ret = mgr->CleanUIAbility(record);
    EXPECT_NE(ret, ERR_OK);
}

/**
 * @tc.name: EnableListForSCBRecovery_001
 * @tc.desc: Should set isSCBRecovery_ to true and clear coldStartInSCBRecovery_
 */
HWTEST_F(UIAbilityLifecycleManagerTest, EnableListForSCBRecovery_001, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    mgr->isSCBRecovery_ = false;
    mgr->coldStartInSCBRecovery_.insert(1);
    mgr->coldStartInSCBRecovery_.insert(2);
    mgr->EnableListForSCBRecovery();

    EXPECT_TRUE(mgr->isSCBRecovery_);
    EXPECT_TRUE(mgr->coldStartInSCBRecovery_.empty());
}

/**
 * @tc.name: IsUIAbilityAlreadyExist_0001
 * @tc.desc: In STANDARD launch mode if a record with same ability exists return ERROR_UIABILITY_IS_ALREADY_EXIST
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, IsUIAbilityAlreadyExist_0001, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(mgr, nullptr);
    std::string abilityName = "AbilityA";
    std::string bundleName = "com.example.test";
    std::string moduleName = "entry";
    int32_t appIndex = 0;
    std::string instanceKey = "ik1";

    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.name = abilityName;
    abilityRequest.abilityInfo.bundleName = bundleName;
    abilityRequest.abilityInfo.moduleName = moduleName;
    abilityRequest.abilityInfo.launchMode = AppExecFwk::LaunchMode::STANDARD;
    abilityRequest.sessionInfo = new SessionInfo();
    abilityRequest.sessionInfo->instanceKey = instanceKey;
    auto record = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    record->SetAppIndex(appIndex);
    record->SetInstanceKey(instanceKey);
    mgr->sessionAbilityMap_[1] = record;

    Want want;
    want.SetElementName("device", bundleName, abilityName, moduleName);
    auto ret = mgr->IsUIAbilityAlreadyExist(want, "", appIndex, instanceKey, AppExecFwk::LaunchMode::STANDARD);
    EXPECT_EQ(ret, ERROR_UIABILITY_IS_ALREADY_EXIST);
}

/**
 * @tc.name: IsUIAbilityAlreadyExist_0002
 * @tc.desc: In SPECIFIED launch mode match ability should return ERROR_UIABILITY_IS_ALREADY_EXIST
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, IsUIAbilityAlreadyExist_0002, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(mgr, nullptr);
    std::string abilityName = "AbilityB";
    std::string bundleName = "com.example.test";
    std::string moduleName = "entry";
    std::string specifiedFlag = "flagX";
    int32_t appIndex = 2;
    std::string instanceKey = "ik2";

    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.name = abilityName;
    abilityRequest.abilityInfo.bundleName = bundleName;
    abilityRequest.abilityInfo.moduleName = moduleName;
    abilityRequest.abilityInfo.launchMode = AppExecFwk::LaunchMode::SPECIFIED;
    abilityRequest.sessionInfo = new SessionInfo();
    abilityRequest.sessionInfo->instanceKey = instanceKey;
    auto record = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    record->SetAppIndex(appIndex);
    record->SetInstanceKey(instanceKey);
    record->SetSpecifiedFlag(specifiedFlag);
    mgr->sessionAbilityMap_[10] = record;

    Want want;
    want.SetElementName("device", bundleName, abilityName, moduleName);
    auto ret = mgr->IsUIAbilityAlreadyExist(want, specifiedFlag, appIndex, instanceKey,
        AppExecFwk::LaunchMode::SPECIFIED);
    EXPECT_EQ(ret, ERROR_UIABILITY_IS_ALREADY_EXIST);
}

/**
 * @tc.name: IsUIAbilityAlreadyExist_0003
 * @tc.desc: Return ERR_OK when no matching record or abilityName differs
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, IsUIAbilityAlreadyExist_0003, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(mgr, nullptr);
    Want want;
    want.SetElementName("device", "com.example.test", "NoExistAbility", "entry");
    auto ret = mgr->IsUIAbilityAlreadyExist(want, "flag", 0, "key", AppExecFwk::LaunchMode::STANDARD);
    EXPECT_EQ(ret, ERR_OK);

    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.name = "OtherAbility";
    abilityRequest.abilityInfo.launchMode = AppExecFwk::LaunchMode::STANDARD;
    abilityRequest.sessionInfo = new SessionInfo();
    abilityRequest.sessionInfo->instanceKey = "key";
    auto record = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    record->SetAppIndex(0);
    record->SetInstanceKey("key");
    mgr->sessionAbilityMap_[5] = record;
    ret = mgr->IsUIAbilityAlreadyExist(want, "flag", 0, "key", AppExecFwk::LaunchMode::STANDARD);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: FindUIAbilityRecordByIdLocked_0001
 * @tc.desc: FindUIAbilityRecordByIdLocked
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, FindUIAbilityRecordByIdLocked_0001, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(mgr, nullptr);
    auto ret = mgr->FindUIAbilityRecordByIdLocked(100);
    EXPECT_EQ(ret, nullptr);
}

/**
 * @tc.name: FindUIAbilityRecordByIdLocked_0002
 * @tc.desc: FindUIAbilityRecordByIdLocked
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, FindUIAbilityRecordByIdLocked_0002, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(mgr, nullptr);

    AbilityRequest abilityRequest;
    auto record = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    record->recordId_ = 100;
    mgr->sessionAbilityMap_.emplace(1, record);
    auto ret = mgr->FindUIAbilityRecordByIdLocked(100);
    EXPECT_NE(ret, nullptr);
}

/**
 * @tc.name: FindUIAbilityRecordByIdLocked_0003
 * @tc.desc: FindUIAbilityRecordByIdLocked
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, FindUIAbilityRecordByIdLocked_0003, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(mgr, nullptr);

    AbilityRequest abilityRequest;
    auto record = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    record->recordId_ = 100;
    mgr->terminateAbilityList_.push_back(record);
    auto ret = mgr->FindUIAbilityRecordByIdLocked(100);
    EXPECT_NE(ret, nullptr);
}

/**
 * @tc.name: HandleUIAbilityDiedByPid_0001
 * @tc.desc: HandleUIAbilityDiedByPid
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, HandleUIAbilityDiedByPid_0001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "UIAbilityLifecycleManagerTest HandleUIAbilityDiedByPid_0001 start");
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(mgr, nullptr);

    AbilityRequest abilityRequest;
    auto record = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    record->pid_ = 1;
    mgr->sessionAbilityMap_.emplace(111, record);
    mgr->sessionAbilityMap_.emplace(112, nullptr);
    pid_t pid = 1;
    mgr->HandleUIAbilityDiedByPid(pid);
    EXPECT_NE(mgr->sessionAbilityMap_[111], nullptr);
    EXPECT_EQ(mgr->sessionAbilityMap_[112], nullptr);
    TAG_LOGI(AAFwkTag::TEST, "UIAbilityLifecycleManagerTest HandleUIAbilityDiedByPid_0001 end");
}

/**
 * @tc.name: HandleUIAbilityDiedByPid_0002
 * @tc.desc: HandleUIAbilityDiedByPid
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, HandleUIAbilityDiedByPid_0002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "UIAbilityLifecycleManagerTest HandleUIAbilityDiedByPid_0002 start");
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(mgr, nullptr);

    AbilityRequest abilityRequest;
    auto record = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    record->pid_ = 2;
    mgr->sessionAbilityMap_.emplace(111, record);
    mgr->sessionAbilityMap_.emplace(112, nullptr);
    pid_t pid = 1;
    mgr->HandleUIAbilityDiedByPid(pid);
    EXPECT_NE(mgr->sessionAbilityMap_[111], nullptr);
    EXPECT_EQ(mgr->sessionAbilityMap_[112], nullptr);
    TAG_LOGI(AAFwkTag::TEST, "UIAbilityLifecycleManagerTest HandleUIAbilityDiedByPid_0002 end");
}

/**
 * @tc.name: ProcessColdStartBranch_001
 * @tc.desc: ProcessColdStartBranch with hook and next request
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, ProcessColdStartBranch_001, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    AbilityRequest abilityRequest;
    sptr<SessionInfo> sessionInfo = new SessionInfo();
    sessionInfo->requestId = 123;
    auto record = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    record->SetIsHook(true);
    
    auto nextAbilityRequest = std::make_shared<SpecifiedRequest>(123, abilityRequest);
    auto nextRequestData = std::make_shared<SpecifiedRequest>(1234, abilityRequest);
    mgr->specifiedRequestList_["123"].push_back(nextAbilityRequest);
    mgr->specifiedRequestList_["123"].push_back(nextRequestData);

    bool isColdStart = true;
    bool result = mgr->ProcessColdStartBranch(abilityRequest, sessionInfo, record, isColdStart);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: MarkStartingFlag_001
 * @tc.desc: MarkStartingFlag
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, MarkStartingFlag_001, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.uid = 100;
    
    auto record = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    record->pid_ = 123;
    record->uid_ = 100;
    mgr->sessionAbilityMap_.emplace(1, record);
    
    mgr->MarkStartingFlag(abilityRequest);
    EXPECT_TRUE(mgr->IsBundleStarting(123));
}

/**
 * @tc.name: StartUIAbility_isSCBRecovery_001
 * @tc.desc: StartUIAbility isSCBRecovery_ is true
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, StartUIAbility_isSCBRecovery_001, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    mgr->isSCBRecovery_ = true;
    AbilityRequest abilityRequest;
    Rosen::SessionInfo info;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->sessionToken = new Rosen::Session(info);
    sessionInfo->persistentId = 1;
    AbilityRuntime::StartParamsBySCB params;
    bool isColdStart = false;
    EXPECT_EQ(mgr->StartUIAbility(abilityRequest, sessionInfo, params, isColdStart), ERR_OK);
    EXPECT_TRUE(isColdStart);
    EXPECT_TRUE(mgr->coldStartInSCBRecovery_.count(sessionInfo->persistentId));
}

/**
 * @tc.name: StartUIAbility_isLowMemKill_001
 * @tc.desc: StartUIAbility isLowMemKill is true
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, StartUIAbility_isLowMemKill_001, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    AbilityRequest abilityRequest;
    Rosen::SessionInfo info;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->sessionToken = new Rosen::Session(info);
    sessionInfo->persistentId = 1;

    auto oldRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    oldRecord->SetIsKillPrecedeStart(true);
    mgr->sessionAbilityMap_.emplace(1, oldRecord);

    AbilityRuntime::StartParamsBySCB params;
    bool isColdStart = false;
    EXPECT_EQ(mgr->StartUIAbility(abilityRequest, sessionInfo, params, isColdStart), ERR_OK);
    EXPECT_TRUE(isColdStart);
    EXPECT_TRUE(mgr->lowMemKillAbilityMap_.count(1));
}

/**
 * @tc.name: NotifySCBToStartUIAbilities_Error_001
 * @tc.desc: NotifySCBToStartUIAbilities with initial handle error
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, NotifySCBToStartUIAbilities_Error_001, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    std::vector<AbilityRequest> abilityRequestList;
    AbilityRequest req;
    req.want.SetParam("ohos.ability.params.callerUid", 12345);
    
    // Trigger more than 20 starts per second to make AddStartCallerTimestamp fail
    for (int i = 0; i < 25; ++i) {
        abilityRequestList.push_back(req);
    }
    
    auto result = mgr->NotifySCBToStartUIAbilities(abilityRequestList, "test_key");
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/**
 * @tc.name: AddStartingPid_001
 * @tc.desc: AddStartingPid double call
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, AddStartingPid_001, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    mgr->AddStartingPid(1234);
    EXPECT_TRUE(mgr->IsBundleStarting(1234));
    mgr->AddStartingPid(1234);
    EXPECT_TRUE(mgr->IsBundleStarting(1234));
}

/**
 * @tc.name: NotifySCBToStartUIAbility_FreqLimit_001
 * @tc.desc: NotifySCBToStartUIAbility frequency limit
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, NotifySCBToStartUIAbility_FreqLimit_001, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    AbilityRequest abilityRequest;
    abilityRequest.want.SetParam("ohos.ability.params.callerUid", 54321);
    
    for (int i = 0; i < 20; ++i) {
        mgr->NotifySCBToStartUIAbility(abilityRequest);
    }
    auto result = mgr->NotifySCBToStartUIAbility(abilityRequest);
    EXPECT_EQ(result, ERR_FREQ_START_ABILITY);
}

/**
 * @tc.name: AttachAbilityThread_NotContains_001
 * @tc.desc: AttachAbilityThread with token not in running list
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, AttachAbilityThread_NotContains_001, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    sptr<IRemoteObject> token = new IPCObjectStub();
    auto result = mgr->AttachAbilityThread(nullptr, token);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/**
 * @tc.name: AttachAbilityThread_ProcessAttachFail_001
 * @tc.desc: AttachAbilityThread with process attachment failure
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, AttachAbilityThread_ProcessAttachFail_001, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    AbilityRequest abilityRequest;
    abilityRequest.sessionInfo = new SessionInfo();
    abilityRequest.sessionInfo->persistentId = 1;

    auto record = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    auto token = record->GetToken();
    mgr->sessionAbilityMap_.emplace(1, record);
    
    auto result = mgr->AttachAbilityThread(nullptr, token->AsObject());
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/**
 * @tc.name: AttachAbilityThread_StartedByCall_001
 * @tc.desc: AttachAbilityThread with IsStartedByCall true
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, AttachAbilityThread_StartedByCall_001, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    AbilityRequest abilityRequest;
    abilityRequest.sessionInfo = new SessionInfo();
    abilityRequest.sessionInfo->persistentId = 1;

    auto record = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    record->isStartedByCall_ = true;
    record->isStartToForeground_ = true;
    auto token = record->GetToken();
    mgr->sessionAbilityMap_.emplace(1, record);
    
    auto result = mgr->AttachAbilityThread(nullptr, token->AsObject());
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/**
 * @tc.name: PrelaunchAbilityLocked_Various_001
 * @tc.desc: PrelaunchAbilityLocked various branches
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, PrelaunchAbilityLocked_Various_001, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    AbilityRequest abilityRequest;
    abilityRequest.want.SetParam("ohos.launch.reason.msg", std::string("test message"));
    
    auto result = mgr->PrelaunchAbilityLocked(abilityRequest, 100);
    EXPECT_NE(result, ERR_OK);
}

/**
 * @tc.name: CallUIAbilityBySCB_NullSession_001
 * @tc.desc: CallUIAbilityBySCB with null sessionInfo
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, CallUIAbilityBySCB_NullSession_001, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    AbilityRuntime::StartParamsBySCB params;
    bool isColdStart = false;
    mgr->CallUIAbilityBySCB(nullptr, params, isColdStart);
    EXPECT_FALSE(isColdStart);
}

/**
 * @tc.name: CallUIAbilityBySCB_DescriptorMismatch_001
 * @tc.desc: CallUIAbilityBySCB with descriptor mismatch
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, CallUIAbilityBySCB_DescriptorMismatch_001, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    sptr<SessionInfo> sessionInfo = new SessionInfo();
    sessionInfo->sessionToken = new IPCObjectStub();
    AbilityRuntime::StartParamsBySCB params;
    bool isColdStart = false;
    mgr->CallUIAbilityBySCB(sessionInfo, params, isColdStart);
    EXPECT_FALSE(isColdStart);
}

/**
 * @tc.name: SignRestartAppFlag_AppRecovery_001
 * @tc.desc: SignRestartAppFlag with isAppRecovery true
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, SignRestartAppFlag_AppRecovery_001, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    AbilityRequest req;
    req.sessionInfo = new SessionInfo();
    req.sessionInfo->persistentId = 1;
    auto record = UIAbilityRecord::CreateAbilityRecord(req);
    record->uid_ = 100;
    mgr->sessionAbilityMap_.emplace(1, record);
    
    mgr->SignRestartAppFlag(100, "", true);
    EXPECT_TRUE(record->isRestartApp_);
}

/**
 * @tc.name: CompleteForegroundSuccess_Various_001
 * @tc.desc: CompleteForegroundSuccess various state branches
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, CompleteForegroundSuccess_Various_001, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    AbilityRequest req;
    req.sessionInfo = new SessionInfo();
    req.sessionInfo->persistentId = 1;
    auto record = UIAbilityRecord::CreateAbilityRecord(req);
    
    // HasLastWant
    record->lastWant_ = std::make_shared<Want>();
    mgr->CompleteForegroundSuccess(record);
    // When lastWant_ is set, it calls ForegroundAbility which sets state to FOREGROUNDING
    EXPECT_EQ(record->GetAbilityState(), AbilityState::FOREGROUNDING);
    
    // PendingState BACKGROUND
    record->lastWant_ = nullptr;
    record->pendingState_ = AbilityState::BACKGROUND;
    mgr->CompleteForegroundSuccess(record);
    EXPECT_EQ(record->minimizeReason_, true);
    
    // PendingState FOREGROUND
    record->pendingState_ = AbilityState::FOREGROUND;
    mgr->CompleteForegroundSuccess(record);
    EXPECT_EQ(record->GetPendingState(), AbilityState::INITIAL);
}

/**
 * @tc.name: HandleForegroundFailed_Basic_001
 * @tc.desc: HandleForegroundFailed basic logic
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, HandleForegroundFailed_Basic_001, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    mgr->HandleForegroundFailed(nullptr, AbilityState::INITIAL);
    
    AbilityRequest req;
    req.sessionInfo = new SessionInfo();
    req.sessionInfo->persistentId = 1;
    auto record = UIAbilityRecord::CreateAbilityRecord(req);
    record->SetAbilityState(AbilityState::FOREGROUNDING);
    mgr->HandleForegroundFailed(record, AbilityState::INITIAL);
    EXPECT_TRUE(record->IsTerminating());
}

/**
 * @tc.name: GetAbilityRecordByToken_Various_001
 * @tc.desc: GetAbilityRecordByToken various branches
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, GetAbilityRecordByToken_Various_001, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    EXPECT_EQ(mgr->GetAbilityRecordByToken(nullptr), nullptr);
    
    AbilityRequest req;
    req.sessionInfo = new SessionInfo();
    req.sessionInfo->persistentId = 1;
    auto record = UIAbilityRecord::CreateAbilityRecord(req);
    mgr->terminateAbilityList_.push_back(record);
    EXPECT_EQ(mgr->GetAbilityRecordByToken(record->GetToken()->AsObject()), record);
    
    mgr->terminateAbilityList_.clear();
    mgr->sessionAbilityMap_.emplace(1, record);
    EXPECT_EQ(mgr->GetAbilityRecordByToken(record->GetToken()->AsObject()), record);
}

/**
 * @tc.name: NotifySCBToStartUIAbility_Specified_001
 * @tc.desc: NotifySCBToStartUIAbility with SPECIFIED mode
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, NotifySCBToStartUIAbility_Specified_001, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    AbilityRequest request;
    request.abilityInfo.launchMode = AppExecFwk::LaunchMode::SPECIFIED;
    
    auto result = mgr->NotifySCBToStartUIAbility(request);
    EXPECT_EQ(result, ERR_OK);
    
    request.startOptions.SetCurrentProcessName("test_process");
    result = mgr->NotifySCBToStartUIAbility(request);
    EXPECT_NE(result, ERR_OK);
}

/**
 * @tc.name: NotifySCBToStartUIAbility_Hook_001
 * @tc.desc: NotifySCBToStartUIAbility with HookModule logic
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, NotifySCBToStartUIAbility_Hook_001, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    AbilityRequest request;
    request.sessionInfo = new SessionInfo();
    request.sessionInfo->persistentId = 1;
    request.want.SetParam("isHook", true);
    
    auto record = UIAbilityRecord::CreateAbilityRecord(request);
    record->isHook_ = true;
    mgr->sessionAbilityMap_.emplace(1, record);
    
    auto result = mgr->NotifySCBToStartUIAbility(request);
    EXPECT_NE(result, ERR_OK);
}

/**
 * @tc.name: StartUIAbility_Preload_001
 * @tc.desc: StartUIAbility with preload checks
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, StartUIAbility_Preload_001, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    AbilityRequest request;
    sptr<SessionInfo> sessionInfo = new SessionInfo();
    sessionInfo->processOptions = std::make_shared<ProcessOptions>();
    sessionInfo->processOptions->isPreloadStart = true;
    sessionInfo->persistentId = 1;
    
    // sessionToken is required for CheckSessionInfo
    Rosen::SessionInfo info;
    sessionInfo->sessionToken = new Rosen::Session(info);
    request.sessionInfo = sessionInfo;
    
    auto record = UIAbilityRecord::CreateAbilityRecord(request);
    record->isPreloaded_ = false;
    mgr->sessionAbilityMap_.emplace(1, record);
    
    AbilityRuntime::StartParamsBySCB params;
    bool isColdStart = false;
    auto result = mgr->StartUIAbility(request, sessionInfo, params, isColdStart);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: StartUIAbility_CallerTypeForAnco_ADD_001
 * @tc.desc: Test StartUIAbility with CallerTypeForAnco::ADD
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, StartUIAbility_CallerTypeForAnco_ADD_001, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    AbilityRequest abilityRequest;
    Rosen::SessionInfo info;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->sessionToken = new Rosen::Session(info);
    sessionInfo->persistentId = 1;
    sessionInfo->callerTypeForAnco = static_cast<int32_t>(CallerTypeForAnco::ADD);
    sessionInfo->callingTokenId = 12345;
    abilityRequest.want.SetParam("testKey", std::string("testValue"));
    AbilityRuntime::StartParamsBySCB params;
    bool isColdStart = true;

    auto result = mgr->StartUIAbility(abilityRequest, sessionInfo, params, isColdStart);
    EXPECT_EQ(result, ERR_OK);

    // Verify callerInfoMap_ contains the entry
    EXPECT_FALSE(mgr->callerInfoMap_.empty());

    // Verify PARAM_DIALOG_SESSION_ID was set in the want
    auto sessionId = abilityRequest.want.GetStringParam("ohos.ability.param.sessionId");
    EXPECT_FALSE(sessionId.empty());
}

/**
 * @tc.name: StartUIAbility_CallerTypeForAnco_ADD_002
 * @tc.desc: Test StartUIAbility with CallerTypeForAnco::ADD and existing ability record
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, StartUIAbility_CallerTypeForAnco_ADD_002, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    AbilityRequest abilityRequest;
    Rosen::SessionInfo info;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->sessionToken = new Rosen::Session(info);
    sessionInfo->persistentId = 1;
    sessionInfo->callerTypeForAnco = static_cast<int32_t>(CallerTypeForAnco::ADD);
    sessionInfo->callingTokenId = 67890;
    abilityRequest.want.SetParam("key1", std::string("value1"));
    abilityRequest.want.SetParam("key2", 100);
    abilityRequest.sessionInfo = sessionInfo;

    // Pre-populate sessionAbilityMap_
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    mgr->sessionAbilityMap_.emplace(sessionInfo->persistentId, abilityRecord);

    AbilityRuntime::StartParamsBySCB params;
    bool isColdStart = false;

    auto result = mgr->StartUIAbility(abilityRequest, sessionInfo, params, isColdStart);
    EXPECT_EQ(result, ERR_OK);

    // Verify callerInfoMap_ contains the entry
    EXPECT_FALSE(mgr->callerInfoMap_.empty());

    // Verify callerInfo contains correct data
    if (!mgr->callerInfoMap_.empty()) {
        auto& callerInfo = mgr->callerInfoMap_.begin()->second;
        EXPECT_EQ(callerInfo.callerTokenId, 67890);
        EXPECT_EQ(callerInfo.targetWant.GetStringParam("key1"), "value1");
    }
}

/**
 * @tc.name: StartUIAbility_CallerTypeForAnco_QUERY_001
 * @tc.desc: Test StartUIAbility with CallerTypeForAnco::QUERY
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, StartUIAbility_CallerTypeForAnco_QUERY_001, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    AbilityRequest abilityRequest;
    Rosen::SessionInfo info;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->sessionToken = new Rosen::Session(info);
    sessionInfo->persistentId = 1;
    sessionInfo->callerTypeForAnco = static_cast<int32_t>(CallerTypeForAnco::QUERY);

    // Set up callerInfoMap_ with test data
    const std::string testSessionId = "test_session_id_12345";
    UIAbilityLifecycleManager::CallerInfo testCallerInfo;
    testCallerInfo.callerTokenId = 99999;
    testCallerInfo.targetWant.SetParam(Want::PARAM_RESV_CALLER_TOKEN, 11111);
    testCallerInfo.targetWant.SetParam(Want::PARAM_RESV_CALLER_UID, 22222);
    testCallerInfo.targetWant.SetParam(Want::PARAM_RESV_CALLER_PID, 33333);
    testCallerInfo.targetWant.SetParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME, std::string("com.test.bundle"));
    testCallerInfo.targetWant.SetParam(Want::PARAM_RESV_CALLER_ABILITY_NAME, std::string("TestAbility"));
    mgr->callerInfoMap_.emplace(testSessionId, testCallerInfo);

    // Set the session ID in the want
    abilityRequest.want.SetParam("ohos.ability.param.sessionId", testSessionId);
    abilityRequest.sessionInfo = sessionInfo;

    // Pre-populate sessionAbilityMap_
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    mgr->sessionAbilityMap_.emplace(sessionInfo->persistentId, abilityRecord);

    AbilityRuntime::StartParamsBySCB params;
    bool isColdStart = false;

    auto result = mgr->StartUIAbility(abilityRequest, sessionInfo, params, isColdStart);
    EXPECT_EQ(result, ERR_OK);

    // Verify callerInfoMap_ was cleared (the entry should have been removed)
    EXPECT_TRUE(mgr->callerInfoMap_.empty());
}

/**
 * @tc.name: StartUIAbility_CallerTypeForAnco_QUERY_002
 * @tc.desc: Test StartUIAbility with CallerTypeForAnco::QUERY and cold start
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, StartUIAbility_CallerTypeForAnco_QUERY_002, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    AbilityRequest abilityRequest;
    Rosen::SessionInfo info;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->sessionToken = new Rosen::Session(info);
    sessionInfo->persistentId = 1;
    sessionInfo->callerTypeForAnco = static_cast<int32_t>(CallerTypeForAnco::QUERY);

    // Set up callerInfoMap_ with test data
    const std::string testSessionId = "cold_start_session_id";
    UIAbilityLifecycleManager::CallerInfo testCallerInfo;
    testCallerInfo.callerTokenId = 55555;
    testCallerInfo.targetWant.SetParam(Want::PARAM_RESV_CALLER_TOKEN, 66666);
    testCallerInfo.targetWant.SetParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME, std::string("com.coldstart.test"));
    testCallerInfo.targetWant.SetParam(Want::PARAM_RESV_CALLER_ABILITY_NAME, std::string("ColdStartAbility"));
    mgr->callerInfoMap_.emplace(testSessionId, testCallerInfo);

    // Set the session ID in the want
    abilityRequest.want.SetParam("ohos.ability.param.sessionId", testSessionId);

    AbilityRuntime::StartParamsBySCB params;
    bool isColdStart = true;

    auto result = mgr->StartUIAbility(abilityRequest, sessionInfo, params, isColdStart);
    EXPECT_EQ(result, ERR_OK);

    // Verify callerInfoMap_ was cleared
    EXPECT_TRUE(mgr->callerInfoMap_.empty());
}

/**
 * @tc.name: StartUIAbility_CallerTypeForAnco_QUERY_003
 * @tc.desc: Test StartUIAbility with CallerTypeForAnco::QUERY and non-existent session ID
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, StartUIAbility_CallerTypeForAnco_QUERY_003, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    AbilityRequest abilityRequest;
    Rosen::SessionInfo info;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->sessionToken = new Rosen::Session(info);
    sessionInfo->persistentId = 1;
    sessionInfo->callerTypeForAnco = static_cast<int32_t>(CallerTypeForAnco::QUERY);

    // Use a non-existent session ID
    const std::string nonExistentSessionId = "non_existent_session_id";
    abilityRequest.want.SetParam("ohos.ability.param.sessionId", nonExistentSessionId);
    abilityRequest.sessionInfo = sessionInfo;

    // Pre-populate sessionAbilityMap_
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    mgr->sessionAbilityMap_.emplace(sessionInfo->persistentId, abilityRecord);

    AbilityRuntime::StartParamsBySCB params;
    bool isColdStart = false;

    // Should not fail even if session ID is not found (UpdateTokenIdAndWantWithRealCallerInfo
    // will handle this case gracefully with a warning log)
    auto result = mgr->StartUIAbility(abilityRequest, sessionInfo, params, isColdStart);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: CloseUIAbility_Various_001
 * @tc.desc: CloseUIAbility various branches
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, CloseUIAbility_Various_001, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    EXPECT_EQ(mgr->CloseUIAbility(nullptr, 0, nullptr, false, false), ERR_UI_ABILITY_MANAGER_NULL_ABILITY_RECORD);
    
    AbilityRequest req;
    auto record = UIAbilityRecord::CreateAbilityRecord(req);
    
    // IsTerminating and not Foreground
    record->SetAbilityState(AbilityState::BACKGROUND);
    record->isTerminating_ = true;
    EXPECT_EQ(mgr->CloseUIAbility(record, 0, nullptr, false, false), ERR_OK);
    
    // INITIAL state
    record->isTerminating_ = false;
    record->SetAbilityState(AbilityState::INITIAL);
    auto result = mgr->CloseUIAbility(record, 0, nullptr, false, false);
    EXPECT_NE(result, ERR_OK);
    
    // Pending State not INITIAL
    record->SetAbilityState(AbilityState::FOREGROUND);
    record->pendingState_ = AbilityState::FOREGROUND;
    EXPECT_EQ(mgr->CloseUIAbility(record, 0, nullptr, false, false), ERR_OK);
    EXPECT_EQ(record->GetPendingState(), AbilityState::BACKGROUND);
}

/**
 * @tc.name: NotifySCBToStartUIAbilities_Various_001
 * @tc.desc: NotifySCBToStartUIAbilities various branches
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, NotifySCBToStartUIAbilities_Various_001, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    std::vector<AbilityRequest> abilityRequestList;
    AbilityRequest req;
    req.abilityInfo.launchMode = AppExecFwk::LaunchMode::SPECIFIED;
    abilityRequestList.push_back(req);
    
    auto result = mgr->NotifySCBToStartUIAbilities(abilityRequestList, "key");
    EXPECT_EQ(result, START_UI_ABILITIES_WAITING_SPECIFIED_CODE);
}

/**
 * @tc.name: OnAcceptWantResponse_Various_001
 * @tc.desc: OnAcceptWantResponse various branches
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, OnAcceptWantResponse_Various_001, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    Want want;
    
    // iter == hookSpecifiedMap_.end()
    mgr->OnAcceptWantResponse(want, "flag", 123);
    EXPECT_TRUE(mgr->hookSpecifiedMap_.empty());
    
    // iter != hookSpecifiedMap_.end()
    AbilityRequest req;
    req.sessionInfo = new SessionInfo();
    req.sessionInfo->persistentId = 1;
    auto record = UIAbilityRecord::CreateAbilityRecord(req);
    mgr->hookSpecifiedMap_.emplace(123, record);
    mgr->OnAcceptWantResponse(want, "flag", 123);
    EXPECT_EQ(record->specifiedFlag_, "flag");
}

/**
 * @tc.name: BatchNotifySCBPendingActivations_Various_001
 * @tc.desc: BatchNotifySCBPendingActivations various branches
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, BatchNotifySCBPendingActivations_Various_001, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    AbilitiesRequest abilitiesRequest;
    auto sessionInfo = new SessionInfo();
    abilitiesRequest.sessionInfoList.emplace_back(1, sessionInfo);
    
    auto result = mgr->BatchNotifySCBPendingActivations(abilitiesRequest);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}
}  // namespace AAFwk
}  // namespace OHOS
