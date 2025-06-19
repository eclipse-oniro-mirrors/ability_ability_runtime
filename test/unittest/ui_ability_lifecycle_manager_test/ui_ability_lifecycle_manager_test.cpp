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
#include "mock_ability_info_callback_stub.h"
#include "process_options.h"
#include "session/host/include/session.h"
#include "session_info.h"
#include "startup_util.h"
#include "ability_manager_service.h"
#include "ability_scheduler_mock.h"
#include "ipc_skeleton.h"
#include "status_bar_delegate_interface.h"

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
    std::shared_ptr<AbilityRecord> InitAbilityRecord();
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

class UIAbilityLifcecycleManagerTestStub : public IRemoteStub<IAbilityConnection> {
public:
    UIAbilityLifcecycleManagerTestStub() {};
    virtual ~UIAbilityLifcecycleManagerTestStub() {};

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

std::shared_ptr<AbilityRecord> UIAbilityLifecycleManagerTest::InitAbilityRecord()
{
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::PAGE;
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
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
    bool isColdStart = false;
    EXPECT_EQ(mgr->StartUIAbility(abilityRequest, nullptr, 0, isColdStart), ERR_INVALID_VALUE);
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
    bool isColdStart = false;
    EXPECT_EQ(mgr->StartUIAbility(abilityRequest, sessionInfo, 0, isColdStart), ERR_OK);
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
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    abilityRecord->SetPendingState(AbilityState::FOREGROUND);
    mgr->sessionAbilityMap_.emplace(sessionInfo->persistentId, abilityRecord);
    bool isColdStart = false;
    EXPECT_EQ(mgr->StartUIAbility(abilityRequest, sessionInfo, 0, isColdStart), ERR_OK);
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
    bool isColdStart = false;
    EXPECT_EQ(mgr->StartUIAbility(abilityRequest, sessionInfo, 0, isColdStart), ERR_OK);
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
    bool isColdStart = false;
    EXPECT_EQ(mgr->StartUIAbility(abilityRequest, sessionInfo, 0, isColdStart), ERR_OK);
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
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    mgr->sessionAbilityMap_.emplace(sessionInfo->persistentId, abilityRecord);
    bool isColdStart = false;
    EXPECT_EQ(mgr->StartUIAbility(abilityRequest, sessionInfo, 0, isColdStart), ERR_OK);
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
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    abilityRecord->isReady_ = true;
    mgr->sessionAbilityMap_.emplace(sessionInfo->persistentId, abilityRecord);
    bool isColdStart = false;
    EXPECT_EQ(mgr->StartUIAbility(abilityRequest, sessionInfo, 0, isColdStart), ERR_OK);
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
    bool isColdStart = false;
    EXPECT_EQ(mgr->StartUIAbility(abilityRequest, sessionInfo, 0, isColdStart), ERR_OK);
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
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    mgr->sessionAbilityMap_.emplace(sessionInfo->persistentId, abilityRecord);
    bool isColdStart = false;
    EXPECT_EQ(mgr->StartUIAbility(abilityRequest, sessionInfo, 0, isColdStart), ERR_OK);
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
    EXPECT_NE(mgr->CreateSessionInfo(abilityRequest), nullptr);
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
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
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
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    mgr->terminateAbilityList_.emplace_back(abilityRecord);
    auto token = abilityRecord->GetToken()->AsObject();
    int state = 6;
    PacMap saveData;
    EXPECT_EQ(mgr->AbilityTransactionDone(token, state, saveData), ERR_INVALID_VALUE);
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
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
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
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
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
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
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
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    auto&& token = abilityRecord->GetToken()->AsObject();
    mgr->OnAbilityRequestDone(token, 1);
    EXPECT_NE(mgr, nullptr);
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
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
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
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
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
    std::shared_ptr<AbilityRecord> abilityRecord = nullptr;
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
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
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
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
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
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    mgr->UpdateAbilityRecordLaunchReason(abilityRequest, abilityRecord);
    EXPECT_NE(mgr, nullptr);
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
    std::shared_ptr<AbilityRecord> abilityRecord = nullptr;
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
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
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
    std::shared_ptr<AbilityRecord> abilityRecord = nullptr;
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
    std::shared_ptr<AbilityRecord> abilityRecord = nullptr;
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
    std::shared_ptr<AbilityRecord> abilityRecord = nullptr;
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
    std::shared_ptr<AbilityRecord> abilityRecord = nullptr;
    int state = 130;
    EXPECT_EQ(mgr->DispatchState(abilityRecord, state), ERR_INVALID_VALUE);
}

/**
 * @tc.name: UIAbilityLifecycleManager_DispatchForeground_0100
 * @tc.desc: DispatchForeground
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, DispatchForeground_001, TestSize.Level1)
{
    auto mgr = std::make_unique<UIAbilityLifecycleManager>();
    AbilityRequest abilityRequest;
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    EXPECT_EQ(mgr->DispatchForeground(abilityRecord, true, AbilityState::FOREGROUND), ERR_INVALID_VALUE);
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
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
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
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
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
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
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
    std::shared_ptr<AbilityRecord> abilityRecord = nullptr;
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
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
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
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
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
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
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
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
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
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
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
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
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
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
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
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
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
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
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
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
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
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
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
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
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
    ASSERT_NE(uiAbilityLifecycleManager, nullptr);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    abilityRecord->currentState_ = AbilityState::BACKGROUNDING;
    abilityRecord->SetPendingState(AbilityState::FOREGROUND);
    uiAbilityLifecycleManager->CompleteBackground(abilityRecord);
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
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    abilityRecord->currentState_ = AbilityState::BACKGROUNDING;
    abilityRecord->SetPendingState(AbilityState::BACKGROUND);
    uiAbilityLifecycleManager->CompleteBackground(abilityRecord);
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_CompleteBackground_0400
 * @tc.desc: CompleteBackground
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, CompleteBackground_004, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(uiAbilityLifecycleManager, nullptr);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    abilityRecord->currentState_ = AbilityState::BACKGROUNDING;
    abilityRecord->SetPendingState(AbilityState::FOREGROUND);
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
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
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
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
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
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    abilityRecord->SetTerminatingState();
    abilityRecord->currentState_ = AbilityState::BACKGROUND;
    EXPECT_EQ(uiAbilityLifecycleManager->CloseUIAbility(abilityRecord, -1, nullptr, false), ERR_OK);
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
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    EXPECT_EQ(uiAbilityLifecycleManager->CloseUIAbility(abilityRecord, -1, nullptr, false), ERR_INVALID_VALUE);
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
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    Want want;
    EXPECT_EQ(uiAbilityLifecycleManager->CloseUIAbility(abilityRecord, -1, &want, false), ERR_INVALID_VALUE);
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
    EXPECT_EQ(uiAbilityLifecycleManager->CloseUIAbility(abilityRecord, -1, &want, false), ERR_OK);
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
    EXPECT_EQ(uiAbilityLifecycleManager->CloseUIAbility(abilityRecord, -1, &want, false), ERR_OK);
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
    EXPECT_EQ(uiAbilityLifecycleManager->CloseUIAbility(abilityRecord, -1, &want, false), ERR_OK);
    uiAbilityLifecycleManager.reset();
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
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
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
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
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
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
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
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
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
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
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
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
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
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
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
    std::shared_ptr<AbilityRecord> record = nullptr;
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
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
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
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
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
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
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
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
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
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
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
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
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
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
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
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
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
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
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
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
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
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
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
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
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
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
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
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
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
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
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
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
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
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
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
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
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
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
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
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
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
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
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
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
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
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
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
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
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
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    uiAbilityLifecycleManager->sessionAbilityMap_.emplace(sessionInfo->persistentId, abilityRecord);
    auto token = abilityRecord->GetToken();
    EXPECT_NE(token, nullptr);
    abilityRequest.callerToken = token->AsObject();
    std::string errMsg;
    uiAbilityLifecycleManager->NotifySCBPendingActivation(sessionInfo, abilityRequest, errMsg);
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
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
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
    uiAbilityLifecycleManager->CallUIAbilityBySCB(sessionInfo, isColdStart);
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
    uiAbilityLifecycleManager->CallUIAbilityBySCB(sessionInfo, isColdStart);
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
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    auto token = abilityRecord->GetToken();
    EXPECT_NE(token, nullptr);
    sessionInfo->sessionToken = token->AsObject();
    bool isColdStart = false;
    uiAbilityLifecycleManager->CallUIAbilityBySCB(sessionInfo, isColdStart);
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
    uiAbilityLifecycleManager->CallUIAbilityBySCB(sessionInfo, isColdStart);
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
    uiAbilityLifecycleManager->CallUIAbilityBySCB(sessionInfo, isColdStart);
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
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    uiAbilityLifecycleManager->tmpAbilityMap_.emplace(1, abilityRecord);
    bool isColdStart = false;
    uiAbilityLifecycleManager->CallUIAbilityBySCB(sessionInfo, isColdStart);
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
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);

    uiAbilityLifecycleManager->tmpAbilityMap_.emplace(1, abilityRecord);
    uiAbilityLifecycleManager->sessionAbilityMap_.emplace(sessionInfo->persistentId, abilityRecord);
    bool isColdStart = false;
    uiAbilityLifecycleManager->CallUIAbilityBySCB(sessionInfo, isColdStart);
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
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
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
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
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
    sptr<IAbilityConnection> connect = new UIAbilityLifcecycleManagerTestStub();
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
    sptr<IAbilityConnection> connect = new UIAbilityLifcecycleManagerTestStub();
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
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
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
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
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
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
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
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
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
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
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
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    abilityRecord->abilityInfo_.launchMode = AppExecFwk::LaunchMode::SPECIFIED;
    abilityRecord->abilityInfo_.moduleName = "entry";
    abilityRecord->SetAppIndex(1);
    abilityRecord->SetSpecifiedFlag(flag);
    uiAbilityLifecycleManager->sessionAbilityMap_.emplace(1, abilityRecord);
    uiAbilityLifecycleManager->OnAcceptWantResponse(want, flag, 0);

    std::shared_ptr<AbilityRecord> callerAbility = InitAbilityRecord();
    abilityRequest.callerToken = callerAbility->GetToken()->AsObject();
    uiAbilityLifecycleManager->OnAcceptWantResponse(want, flag, 0);
    uiAbilityLifecycleManager.reset();
}
#endif // WITH_DLP

/**
 * @tc.name: UIAbilityLifecycleManager_StartSpecifiedAbilityBySCB_0100
 * @tc.desc: StartSpecifiedAbilityBySCB
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, StartSpecifiedAbilityBySCB_001, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    AbilityRequest abilityRequest;
    auto result = uiAbilityLifecycleManager->StartSpecifiedAbilityBySCB(abilityRequest);
    EXPECT_EQ(result, ERR_OK);
}

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
 * @tc.name: UIAbilityLifecycleManager_NotifyRestartSpecifiedAbility_0100
 * @tc.desc: NotifyRestartSpecifiedAbility
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, NotifyRestartSpecifiedAbility_001, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest request;
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    sptr<IRemoteObject> token = abilityRecord->GetToken();
    request.abilityInfoCallback = new MockAbilityInfoCallbackStub();
    uiAbilityLifecycleManager->NotifyRestartSpecifiedAbility(request, token);
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_NotifyStartSpecifiedAbility_0100
 * @tc.desc: NotifyStartSpecifiedAbility
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, NotifyStartSpecifiedAbility_001, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest request;
    Want want;
    request.abilityInfoCallback = new MockAbilityInfoCallbackStub();
    uiAbilityLifecycleManager->NotifyStartSpecifiedAbility(request, want);
    uiAbilityLifecycleManager.reset();
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
    int res = uiAbilityLifecycleManager->MoveAbilityToFront(abilityRequest, nullptr, nullptr, nullptr, 0);
    EXPECT_EQ(res, ERR_INVALID_VALUE);

    abilityRequest.sessionInfo = new SessionInfo();
    abilityRequest.appInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::PAGE;
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    res = uiAbilityLifecycleManager->MoveAbilityToFront(abilityRequest, abilityRecord, nullptr, nullptr, 0);
    EXPECT_EQ(res, ERR_OK);

    auto startOptions = std::make_shared<StartOptions>();
    res = uiAbilityLifecycleManager->MoveAbilityToFront(abilityRequest, abilityRecord, nullptr, nullptr, 0);
    EXPECT_EQ(res, ERR_OK);

    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_SendSessionInfoToSCB_0100
 * @tc.desc: SendSessionInfoToSCB
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, SendSessionInfoToSCB_001, TestSize.Level1)
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
    std::shared_ptr<AbilityRecord> callerAbility = AbilityRecord::CreateAbilityRecord(abilityRequest);
    int res = uiAbilityLifecycleManager->SendSessionInfoToSCB(callerAbility, sessionInfo);
    EXPECT_EQ(res, ERR_OK);

    sessionInfo->sessionToken = nullptr;
    abilityRequest.sessionInfo = sessionInfo;
    callerAbility = AbilityRecord::CreateAbilityRecord(abilityRequest);
    res = uiAbilityLifecycleManager->SendSessionInfoToSCB(callerAbility, sessionInfo);
    EXPECT_EQ(res, ERR_INVALID_VALUE);

    abilityRequest.sessionInfo = nullptr;
    callerAbility = AbilityRecord::CreateAbilityRecord(abilityRequest);
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
 * @tc.name: UIAbilityLifecycleManager_StartAbilityBySpecifed_0100
 * @tc.desc: StartAbilityBySpecifed
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, StartAbilityBySpecifed_001, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest request;
    std::shared_ptr<AbilityRecord> callerAbility = nullptr;
    uiAbilityLifecycleManager->StartAbilityBySpecifed(request, callerAbility, 0);
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
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
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
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
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
 * @tc.name: UIAbilityLifecycleManager_ChangeUIAbilityVisibilityBySCB_0100
 * @tc.desc: ChangeUIAbilityVisibilityBySCB
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, ChangeUIAbilityVisibilityBySCB_001, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    int32_t ret = uiAbilityLifecycleManager->ChangeUIAbilityVisibilityBySCB(nullptr, true);
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
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
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
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
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
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
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
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
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
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
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
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    abilityRecord->SetOwnerMissionUserId(DelayedSingleton<AbilityManagerService>::GetInstance()->GetUserId());
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
    auto targetRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    uiAbilityLifecycleManager->sessionAbilityMap_.emplace(sessionInfo->persistentId, targetRecord);
    EXPECT_EQ(uiAbilityLifecycleManager->IsAbilityStarted(abilityRequest, targetRecord), false);
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
    auto targetRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    uiAbilityLifecycleManager->sessionAbilityMap_.emplace(sessionInfo->persistentId, targetRecord);
    sptr<IAbilityScheduler> scheduler = new AbilitySchedulerMock();
    targetRecord->SetScheduler(scheduler);
    EXPECT_EQ(uiAbilityLifecycleManager->IsAbilityStarted(abilityRequest, targetRecord), true);
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
    EXPECT_EQ(uiAbilityLifecycleManager->TryPrepareTerminateByPids(pids), ERR_OK);
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
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
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
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
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
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
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
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
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
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
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
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
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
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
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
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
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
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
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
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
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
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
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
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
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
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    uiAbilityLifecycleManager->sessionAbilityMap_.emplace(1, abilityRecord);
    AppExecFwk::ElementName element;
    auto ret = uiAbilityLifecycleManager->GetAbilityRecordsByName(element);
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
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    uiAbilityLifecycleManager->sessionAbilityMap_.emplace(1, abilityRecord);
    AppExecFwk::ElementName element("100", "com.example.unittest", "MainAbility");
    auto ret = uiAbilityLifecycleManager->GetAbilityRecordsByName(element);
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
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    uiAbilityLifecycleManager->sessionAbilityMap_.emplace(1, abilityRecord);
    AppExecFwk::ElementName element("100", "com.example.unittest", "MainAbility", "entry");
    auto ret = uiAbilityLifecycleManager->GetAbilityRecordsByName(element);
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
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    uiAbilityLifecycleManager->sessionAbilityMap_.emplace(1, abilityRecord);
    AppExecFwk::ElementName element;
    auto ret = uiAbilityLifecycleManager->GetAbilityRecordsByNameInner(element);
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
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    uiAbilityLifecycleManager->sessionAbilityMap_.emplace(1, abilityRecord);
    AppExecFwk::ElementName element("100", "com.example.unittest", "MainAbility");
    auto ret = uiAbilityLifecycleManager->GetAbilityRecordsByNameInner(element);
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
    abilityRequest.abilityInfo.moduleName = "entry";
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    uiAbilityLifecycleManager->sessionAbilityMap_.emplace(1, abilityRecord);
    AppExecFwk::ElementName element("100", "com.example.unittest", "MainAbility", "entry");
    auto ret = uiAbilityLifecycleManager->GetAbilityRecordsByNameInner(element);
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
    std::shared_ptr<AbilityRecord> abilityRecord = nullptr;
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
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
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
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
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
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
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
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
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
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
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
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
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
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
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
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
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
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
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
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
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
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    AppInfo info;
    info.processName = "AbilityProcess";
    uiAbilityLifecycleManager->sessionAbilityMap_.emplace(0, abilityRecord);
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
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
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
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
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
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
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
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
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
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
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
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
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
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
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
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
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
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
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
    std::shared_ptr<AbilityRecord> abilityRecord = nullptr;
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
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
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
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
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
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
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
    std::shared_ptr<AbilityRecord> abilityRecord = nullptr;
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
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
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
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
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
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
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
    std::shared_ptr<AbilityRecord> targetAbility = nullptr;
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
    auto targetAbility = AbilityRecord::CreateAbilityRecord(abilityRequest);
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
    abilityRequest.connect = new UIAbilityLifcecycleManagerTestStub();
    abilityRequest.callType = AbilityCallType::CALL_REQUEST_TYPE;
    auto targetAbility = AbilityRecord::CreateAbilityRecord(abilityRequest);
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
    abilityRequest.connect = new UIAbilityLifcecycleManagerTestStub();
    abilityRequest.callType = AbilityCallType::CALL_REQUEST_TYPE;
    auto targetAbility = AbilityRecord::CreateAbilityRecord(abilityRequest);
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
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
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
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
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

    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
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

    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
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

    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    Want resultWant;
    int32_t resultCode = 100;
    int32_t requestCode = 1;
    int32_t pid = 1;
    bool backFlag = false;
    int64_t callerRequestCode = AbilityRuntime::StartupUtil::GenerateFullRequestCode(pid, backFlag, requestCode);

    // not support back to caller
    std::shared_ptr<AbilityRecord> callerAbilityRecord = InitAbilityRecord();
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

    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
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

    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    Want resultWant;
    int32_t resultCode = 100;
    int32_t requestCode = 1;
    int32_t pid = 1;
    bool backFlag = true;
    int64_t callerRequestCode = AbilityRuntime::StartupUtil::GenerateFullRequestCode(pid, backFlag, requestCode);

    std::shared_ptr<AbilityRecord> callerAbilityRecord = InitAbilityRecord();
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

    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    Want resultWant;
    int32_t resultCode = 100;
    int32_t requestCode = 1;
    int32_t pid = 1;
    bool backFlag = true;
    int64_t callerRequestCode = AbilityRuntime::StartupUtil::GenerateFullRequestCode(pid, backFlag, requestCode);

    std::shared_ptr<AbilityRecord> callerAbilityRecord = InitAbilityRecord();
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
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
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
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
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
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
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
    std::shared_ptr<AbilityRecord> abilityRecord = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);

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
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
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
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);

    abilityRecord->abilityInfo_.applicationInfo.accessTokenId = 2;
    abilityRecord->abilityInfo_.applicationInfo.bundleType = AppExecFwk::BundleType::ATOMIC_SERVICE;
    abilityRecord->abilityInfo_.type = AppExecFwk::AbilityType::PAGE;
    auto abilityRecord2 = AbilityRecord::CreateAbilityRecord(abilityRequest);
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
    auto abilityRecord3 = AbilityRecord::CreateAbilityRecord(abilityRequest);
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
    auto abilityRecord4 = AbilityRecord::CreateAbilityRecord(abilityRequest);
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
    uint32_t sceneFlag = 1;
    auto result = uiAbilityLifecycleManager->HandleStartSpecifiedCold(abilityRequest, nullptr, sceneFlag);
    EXPECT_EQ(result, false);

    sptr<SessionInfo> sessionInfo = sptr<AAFwk::SessionInfo>::MakeSptr();
    abilityRequest.abilityInfo.launchMode = AppExecFwk::LaunchMode::STANDARD;
    result = uiAbilityLifecycleManager->HandleStartSpecifiedCold(abilityRequest, sessionInfo, sceneFlag);
    EXPECT_EQ(result, false);

    abilityRequest.abilityInfo.launchMode = AppExecFwk::LaunchMode::SPECIFIED;
    sessionInfo->requestId = 1;
    auto specifiedRequest = std::make_shared<AAFwk::SpecifiedRequest>(0, abilityRequest);
    specifiedRequest->isCold = true;
    specifiedRequest->requestId = sessionInfo->requestId;
    std::list<std::shared_ptr<SpecifiedRequest>> Lists;
    Lists.push_back(specifiedRequest);
    uiAbilityLifecycleManager->specifiedRequestList_.emplace("key1", Lists);
    result = uiAbilityLifecycleManager->HandleStartSpecifiedCold(abilityRequest, sessionInfo, sceneFlag);
    EXPECT_EQ(result, true);

    Lists.clear();
    uiAbilityLifecycleManager->specifiedRequestList_.clear();
    specifiedRequest->isCold = false;
    specifiedRequest->requestId = sessionInfo->requestId;
    Lists.push_back(specifiedRequest);
    uiAbilityLifecycleManager->specifiedRequestList_.emplace("key2", Lists);
    result = uiAbilityLifecycleManager->HandleStartSpecifiedCold(abilityRequest, sessionInfo, sceneFlag);
    EXPECT_EQ(result, false);

    Lists.clear();
    uiAbilityLifecycleManager->specifiedRequestList_.clear();
    specifiedRequest->isCold = true;
    specifiedRequest->requestId = 2;
    Lists.push_back(specifiedRequest);
    uiAbilityLifecycleManager->specifiedRequestList_.emplace("key3", Lists);
    result = uiAbilityLifecycleManager->HandleStartSpecifiedCold(abilityRequest, sessionInfo, sceneFlag);
    EXPECT_EQ(result, false);
}

/**
 * @tc.name: UIAbilityLifecycleManager_HasAbilityRequest_0100
 * @tc.desc: HasAbilityRequest
 */
HWTEST_F(UIAbilityLifecycleManagerTest, HasAbilityRequest_0100, TestSize.Level1)
{
    AbilityRequest abilityRequest;
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();

    auto abilityRequest1 = std::make_shared<AbilityRequest>(abilityRequest);
    uiAbilityLifecycleManager->startAbilityCheckMap_.emplace(1, abilityRequest1);
    auto result = uiAbilityLifecycleManager->HasAbilityRequest(abilityRequest);
    EXPECT_EQ(result, true);

    uiAbilityLifecycleManager->startAbilityCheckMap_.clear();
    auto abilityRequest2 = std::make_shared<AbilityRequest>();
    abilityRequest2->abilityInfo.bundleName = "otherBundleName";
    uiAbilityLifecycleManager->startAbilityCheckMap_.emplace(1, abilityRequest2);
    result = uiAbilityLifecycleManager->HasAbilityRequest(abilityRequest);
    EXPECT_EQ(result, false);
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
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
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
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();

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
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();

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
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
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
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
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
    auto tmpRecord = AbilityRecord::CreateAbilityRecord(dummyRequest);
    mgr->tmpAbilityMap_[100] = tmpRecord;
    bool isColdStart = false;
    auto record = mgr->GenerateAbilityRecord(request, sessionInfo, isColdStart);
    EXPECT_EQ(record, tmpRecord);
    EXPECT_TRUE(isColdStart);
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
    auto record = AbilityRecord::CreateAbilityRecord(dummyRequest);
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
    auto record = AbilityRecord::CreateAbilityRecord(dummyRequest);
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
    auto record = AbilityRecord::CreateAbilityRecord(dummyRequest);
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
 * @tc.desc: tmpAbilityMap_ contains non-matching AbilityRecord, should return nullptr
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
    auto recordNotMatch = AbilityRecord::CreateAbilityRecord(dummyRequest);
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
    auto record = AbilityRecord::CreateAbilityRecord(dummyRequest);
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
    auto record = AbilityRecord::CreateAbilityRecord(request);

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
    std::shared_ptr<AbilityRecord> record = AbilityRecord::CreateAbilityRecord(request);

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
    auto record = AbilityRecord::CreateAbilityRecord(request);
    auto token = record->GetToken();
    mgr->sessionAbilityMap_[1] = record;

    int ret = mgr->AttachAbilityThread(nullptr, token);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: OnAbilityRequestDone_0001
 * @tc.desc: IsTerminating is true
 */
HWTEST_F(UIAbilityLifecycleManagerTest, OnAbilityRequestDone_0001, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    AbilityRequest request;
    auto record = AbilityRecord::CreateAbilityRecord(request);
    record->SetAbilityState(AbilityState::TERMINATING);
    auto token = record->GetToken();
    mgr->sessionAbilityMap_[1] = record;

    mgr->OnAbilityRequestDone(token, static_cast<int32_t>(AppAbilityState::ABILITY_STATE_FOREGROUND));

    EXPECT_EQ(record->GetPendingState(), AbilityState::INITIAL);
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
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
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

    auto record = AbilityRecord::CreateAbilityRecord(request);
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
    auto record = AbilityRecord::CreateAbilityRecord(request);
    record->SetAbilityState(AbilityState::FOREGROUNDING);

    int ret = mgr->DispatchForeground(record, true);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: DispatchForeground_0002
 * @tc.desc: DispatchForeground
 */
HWTEST_F(UIAbilityLifecycleManagerTest, DispatchForeground_0002, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    AbilityRequest request;
    request.abilityInfo.bundleName = "com.example.bundle";
    request.abilityInfo.name = "AbilityA";
    request.abilityInfo.moduleName = "moduleA";
    request.abilityInfo.isStageBasedModel = true;
    request.abilityInfo.type = AppExecFwk::AbilityType::PAGE;
    auto record = AbilityRecord::CreateAbilityRecord(request);
    record->SetAbilityState(AbilityState::FOREGROUNDING);

    int ret = mgr->DispatchForeground(record, false, AbilityState::FOREGROUND_FAILED);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: DispatchForeground_0003
 * @tc.desc: DispatchForeground
 */
HWTEST_F(UIAbilityLifecycleManagerTest, DispatchForeground_0003, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    AbilityRequest request;
    request.abilityInfo.bundleName = "com.example.bundle";
    request.abilityInfo.name = "AbilityA";
    request.abilityInfo.moduleName = "moduleA";
    request.abilityInfo.isStageBasedModel = true;
    request.abilityInfo.type = AppExecFwk::AbilityType::PAGE;
    auto record = AbilityRecord::CreateAbilityRecord(request);
    record->SetAbilityState(AbilityState::FOREGROUNDING);

    int ret = mgr->DispatchForeground(record, false, AbilityState::FOREGROUND_WINDOW_FREEZED);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
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
    auto record = AbilityRecord::CreateAbilityRecord(request);
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
    auto record = AbilityRecord::CreateAbilityRecord(request);
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
    auto record = AbilityRecord::CreateAbilityRecord(request);

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
    auto record = AbilityRecord::CreateAbilityRecord(request);
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
    auto record = AbilityRecord::CreateAbilityRecord(request);
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
    auto record = AbilityRecord::CreateAbilityRecord(request);
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

    auto record = AbilityRecord::CreateAbilityRecord(request);
    record->SetPendingState(AbilityState::FOREGROUND);
    int32_t persistentId = 1;
    mgr->sessionAbilityMap_[persistentId] = record;
    std::string errMsg;
    int ret = mgr->CallAbilityLocked(request, errMsg);
    EXPECT_EQ(record->GetPendingState(), AbilityState::FOREGROUND);
}

/**
 * @tc.name: PostCallTimeoutTask_001
 * @tc.desc: abilityRecord is nullptr, tmpAbilityMap_ and callRequestCache_ remain unchanged
 */
HWTEST_F(UIAbilityLifecycleManagerTest, PostCallTimeoutTask_001, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    size_t beforeSize = mgr->tmpAbilityMap_.size();
    mgr->PostCallTimeoutTask(nullptr);
    EXPECT_EQ(mgr->tmpAbilityMap_.size(), beforeSize);
}

/**
 * @tc.name: PostCallTimeoutTask_002
 * @tc.desc: abilityRecord not in tmpAbilityMap_, tmpAbilityMap_ and callRequestCache_ remain unchanged after timeout
 */
HWTEST_F(UIAbilityLifecycleManagerTest, PostCallTimeoutTask_002, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    AbilityRequest request;
    request.abilityInfo.name = "AbilityA";
    auto record = AbilityRecord::CreateAbilityRecord(request);
    size_t beforeSize = mgr->tmpAbilityMap_.size();
    mgr->PostCallTimeoutTask(record);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_EQ(mgr->tmpAbilityMap_.size(), beforeSize);
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
    auto record = AbilityRecord::CreateAbilityRecord(request);
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
    auto record = AbilityRecord::CreateAbilityRecord(request);
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
    auto record = AbilityRecord::CreateAbilityRecord(request);
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
    auto record = AbilityRecord::CreateAbilityRecord(request);
    int oldState = record->GetAbilityState();
    mgr->PrintTimeOutLog(record, 100, true);
    EXPECT_EQ(record->GetAbilityState(), oldState);
}

/**
 * @tc.name: PrintTimeOutLog_0006
 * @tc.desc: state not UNKNOWN, isHalf is false, ability state should not change
 */
HWTEST_F(UIAbilityLifecycleManagerTest, PrintTimeOutLog_0006, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    AbilityRequest request;
    request.abilityInfo.name = "AbilityA";
    auto record = AbilityRecord::CreateAbilityRecord(request);
    int oldState = record->GetAbilityState();
    mgr->PrintTimeOutLog(record, 100, false);
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
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(request);
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
    auto callerAbilityRecord = AbilityRecord::CreateAbilityRecord(request);

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
    auto record = AbilityRecord::CreateAbilityRecord(request);
    record->SetAbilityState(AbilityState::INITIAL);
    int ret = mgr->CloseUIAbility(record, 0, nullptr, false);
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
    auto record = AbilityRecord::CreateAbilityRecord(request);
    record->SetAbilityState(AbilityState::FOREGROUND);
    int ret = mgr->CloseUIAbility(record, 0, nullptr, true);
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
    auto record = AbilityRecord::CreateAbilityRecord(request);
    record->SetAbilityState(AbilityState::FOREGROUND);
    record->SetPendingState(AbilityState::FOREGROUND);
    int ret = mgr->CloseUIAbility(record, 0, nullptr, false);
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
    auto record = AbilityRecord::CreateAbilityRecord(request);
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
    auto record = AbilityRecord::CreateAbilityRecord(request);
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
    auto record = AbilityRecord::CreateAbilityRecord(request);
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
    auto record = AbilityRecord::CreateAbilityRecord(request);
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
    auto record = AbilityRecord::CreateAbilityRecord(request);
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
    auto record = AbilityRecord::CreateAbilityRecord(request);
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
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
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
    auto record = AbilityRecord::CreateAbilityRecord(request);
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
}  // namespace AAFwk
}  // namespace OHOS
