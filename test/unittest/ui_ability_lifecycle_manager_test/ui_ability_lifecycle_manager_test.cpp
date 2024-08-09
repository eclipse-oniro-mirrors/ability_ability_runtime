/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#include "ability_manager_service.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AAFwk {
namespace {
#ifdef WITH_DLP
const std::string DLP_INDEX = "ohos.dlp.params.index";
#endif // WITH_DLP
constexpr int32_t TEST_UID = 20010001;
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
    abilityRequest.sessionInfo = sessionInfo;
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
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
    abilityRequest.sessionInfo = sessionInfo;
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    abilityRecord->SetPendingState(AbilityState::FOREGROUND);
    mgr->sessionAbilityMap_.emplace(sessionInfo->persistentId, abilityRecord);
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
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    AbilityRequest abilityRequest;
    Rosen::SessionInfo info;
    sptr<SessionInfo> sessionInfo = new (std::nothrow) SessionInfo();
    sessionInfo->sessionToken = new (std::nothrow) Rosen::Session(info);
    sessionInfo->persistentId = 1;
    abilityRequest.abilityInfo.bundleName = "com.example.test";
    abilityRequest.abilityInfo.name = "Entry";
    abilityRequest.abilityInfo.moduleName = "EntryModule";
    std::shared_ptr<AbilityRecord>  abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    mgr->sessionAbilityMap_.emplace(2, abilityRecord);
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
    EXPECT_NE(mgr, nullptr);
    AbilityRequest abilityRequest;
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    abilityRecord->SetPendingState(AbilityState::BACKGROUND);
    mgr->CompleteForegroundSuccess(abilityRecord);
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
    EXPECT_NE(mgr, nullptr);
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
    EXPECT_NE(mgr, nullptr);
    AbilityRequest abilityRequest;
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    abilityRecord->SetStartedByCall(true);
    abilityRecord->SetStartToForeground(true);
    abilityRecord->isReady_ = true;
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
    uiAbilityLifecycleManager->NotifySCBPendingActivation(sessionInfo, abilityRequest);
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
    EXPECT_EQ(uiAbilityLifecycleManager->ResolveLocked(abilityRequest), RESOLVE_CALL_ABILITY_INNER_ERR);
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
    EXPECT_EQ(uiAbilityLifecycleManager->ResolveLocked(abilityRequest), RESOLVE_CALL_ABILITY_INNER_ERR);
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

    uiAbilityLifecycleManager->CallAbilityLocked(abilityRequest);
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
    uiAbilityLifecycleManager->CallAbilityLocked(abilityRequest);
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
 * @tc.name: UIAbilityLifecycleManager_EraseSpecifiedAbilityRecord_0100
 * @tc.desc: EraseSpecifiedAbilityRecord
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, EraseSpecifiedAbilityRecord_001, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    std::shared_ptr<AbilityRecord> abilityRecord1 = InitAbilityRecord();
    std::shared_ptr<AbilityRecord> abilityRecord2 = InitAbilityRecord();
    UIAbilityLifecycleManager::SpecifiedInfo specifiedInfo1;
    UIAbilityLifecycleManager::SpecifiedInfo specifiedInfo2;
    UIAbilityLifecycleManager::SpecifiedInfo specifiedInfo3;
    UIAbilityLifecycleManager::SpecifiedInfo specifiedInfo4;
    specifiedInfo1.abilityName = "MainAbility";
    specifiedInfo1.bundleName = "com.example.unittest";
    specifiedInfo1.flag = "flag";
    specifiedInfo2.abilityName = "MainAbility2";
    specifiedInfo2.bundleName = "com.example.unittest";
    specifiedInfo2.flag = "flag";
    specifiedInfo3.abilityName = "MainAbility";
    specifiedInfo3.bundleName = "com.example.unittest2";
    specifiedInfo3.flag = "flag";
    specifiedInfo4.abilityName = "MainAbility";
    specifiedInfo4.bundleName = "com.example.unittest2";
    specifiedInfo4.flag = "flag2";
    uiAbilityLifecycleManager->specifiedAbilityMap_.emplace(specifiedInfo1, abilityRecord1);
    uiAbilityLifecycleManager->specifiedAbilityMap_.emplace(specifiedInfo1, nullptr);
    uiAbilityLifecycleManager->specifiedAbilityMap_.emplace(specifiedInfo1, abilityRecord2);
    uiAbilityLifecycleManager->specifiedAbilityMap_.emplace(specifiedInfo2, abilityRecord1);
    uiAbilityLifecycleManager->specifiedAbilityMap_.emplace(specifiedInfo3, abilityRecord1);
    uiAbilityLifecycleManager->specifiedAbilityMap_.emplace(specifiedInfo4, abilityRecord1);
    uiAbilityLifecycleManager->EraseSpecifiedAbilityRecord(abilityRecord1);
    uiAbilityLifecycleManager.reset();
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
    uiAbilityLifecycleManager->OnAcceptWantResponse(want, flag);

    AbilityRequest abilityRequest;
    uiAbilityLifecycleManager->OnAcceptWantResponse(want, flag);

    abilityRequest.abilityInfo.launchMode = AppExecFwk::LaunchMode::SPECIFIED;
    uiAbilityLifecycleManager->OnAcceptWantResponse(want, flag);

    uiAbilityLifecycleManager->OnAcceptWantResponse(want, "");
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
    uiAbilityLifecycleManager->OnAcceptWantResponse(want, flag);

    UIAbilityLifecycleManager::SpecifiedInfo specifiedInfo;
    specifiedInfo.abilityName = "MainAbility";
    specifiedInfo.bundleName = "com.example.unittest";
    specifiedInfo.flag = "flag";
    uiAbilityLifecycleManager->specifiedAbilityMap_.emplace(specifiedInfo, abilityRecord);
    uiAbilityLifecycleManager->OnAcceptWantResponse(want, flag);

    std::shared_ptr<AbilityRecord> callerAbility = InitAbilityRecord();
    abilityRequest.callerToken = callerAbility->GetToken()->AsObject();
    uiAbilityLifecycleManager->OnAcceptWantResponse(want, flag);
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
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    Want want;
    uiAbilityLifecycleManager->StartSpecifiedAbilityBySCB(want);
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_GetReusedSpecifiedAbility_0100
 * @tc.desc: GetReusedSpecifiedAbility
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, GetReusedSpecifiedAbility_001, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    Want want;
    std::string flag = "flag";
    UIAbilityLifecycleManager::SpecifiedInfo specifiedInfo;
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    specifiedInfo.flag = "flag2";
    want.SetElementName(specifiedInfo.abilityName, specifiedInfo.bundleName);
    uiAbilityLifecycleManager->specifiedAbilityMap_.emplace(specifiedInfo, abilityRecord);
    specifiedInfo.abilityName = "MainAbility2";
    specifiedInfo.flag = "flag";
    uiAbilityLifecycleManager->specifiedAbilityMap_.emplace(specifiedInfo, abilityRecord);
    specifiedInfo.abilityName = "MainAbility";
    specifiedInfo.bundleName = "com.example.unittest2";
    uiAbilityLifecycleManager->specifiedAbilityMap_.emplace(specifiedInfo, abilityRecord);
    uiAbilityLifecycleManager->GetReusedSpecifiedAbility(want, flag);
    uiAbilityLifecycleManager.reset();
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
    int res = uiAbilityLifecycleManager->MoveAbilityToFront(abilityRequest, nullptr, nullptr, nullptr);
    EXPECT_EQ(res, ERR_INVALID_VALUE);

    abilityRequest.sessionInfo = new SessionInfo();
    abilityRequest.appInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::PAGE;
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    res = uiAbilityLifecycleManager->MoveAbilityToFront(abilityRequest, abilityRecord, nullptr, nullptr);
    EXPECT_EQ(res, ERR_OK);

    auto startOptions = std::make_shared<StartOptions>();
    res = uiAbilityLifecycleManager->MoveAbilityToFront(abilityRequest, abilityRecord, nullptr, nullptr);
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
    uiAbilityLifecycleManager->StartAbilityBySpecifed(request, callerAbility);
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
    std::shared_ptr<AbilityRecord> abilityRecord = nullptr;
    sptr<IRemoteObject> token = nullptr;
    EXPECT_NE(mgr->NotifySCBToMinimizeUIAbility(abilityRecord, token), ERR_OK);
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
    Want want;
    std::string flag = "flag";
    uiAbilityLifecycleManager->OnStartSpecifiedProcessResponse(want, flag);
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
    Want want;
    std::string flag = "flag";
    int32_t requestId = 100;
    uiAbilityLifecycleManager->OnStartSpecifiedProcessResponse(want, flag, requestId);
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
    Want want;
    uiAbilityLifecycleManager->OnStartSpecifiedAbilityTimeoutResponse(want);
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
    Want want;
    int32_t requestId = 100;
    uiAbilityLifecycleManager->OnStartSpecifiedAbilityTimeoutResponse(want, requestId);
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_OnStartSpecifiedProcessTimeoutResponse_0100
 * @tc.desc: OnStartSpecifiedProcessTimeoutResponse
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, OnStartSpecifiedProcessTimeoutResponse_001, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    Want want;
    uiAbilityLifecycleManager->OnStartSpecifiedProcessTimeoutResponse(want);
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_OnStartSpecifiedProcessTimeoutResponse_0200
 * @tc.desc: OnStartSpecifiedProcessTimeoutResponse
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, OnStartSpecifiedProcessTimeoutResponse_002, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    Want want;
    int32_t requestId = 100;
    uiAbilityLifecycleManager->OnStartSpecifiedProcessTimeoutResponse(want, requestId);
    uiAbilityLifecycleManager.reset();
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
    EXPECT_EQ(uiAbilityLifecycleManager->IsAbilityStarted(abilityRequest, targetRecord), true);
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
    bool boolValue = uiAbilityLifecycleManager->PrepareTerminateAbility(abilityRecord);
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
    bool boolValue = uiAbilityLifecycleManager->PrepareTerminateAbility(abilityRecord);
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
 * @tc.name: UIAbilityLifecycleManager_KillProcessWithPrepareTerminate_0100
 * @tc.desc: KillProcessWithPrepareTerminate
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, KillProcessWithPrepareTerminate_001, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    ASSERT_NE(uiAbilityLifecycleManager, nullptr);
    std::vector<int32_t> pids;
    EXPECT_EQ(uiAbilityLifecycleManager->KillProcessWithPrepareTerminate(pids), ERR_OK);
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
    EXPECT_EQ(uiAbilityLifecycleManager->ChangeAbilityVisibility(token, isShow), ERR_INVALID_VALUE);
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
    EXPECT_EQ(uiAbilityLifecycleManager->ChangeAbilityVisibility(token, isShow), ERR_OK);
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
    EXPECT_EQ(uiAbilityLifecycleManager->ChangeAbilityVisibility(token, isShow), ERR_OK);
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
    EXPECT_EQ(uiAbilityLifecycleManager->ChangeUIAbilityVisibilityBySCB(sessionInfo, isShow),
        ERR_NATIVE_ABILITY_STATE_CHECK_FAILED);
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
    EXPECT_EQ(uiAbilityLifecycleManager->ChangeUIAbilityVisibilityBySCB(sessionInfo, isShow),
        ERR_NATIVE_ABILITY_STATE_CHECK_FAILED);
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
 * @tc.name: UIAbilityLifecycleManager_CheckPrepareTerminateEnable_0100
 * @tc.desc: CheckPrepareTerminateEnable
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, CheckPrepareTerminateEnable_001, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    std::shared_ptr<AbilityRecord> abilityRecord = nullptr;
    EXPECT_EQ(uiAbilityLifecycleManager->CheckPrepareTerminateEnable(abilityRecord), false);
}

/**
 * @tc.name: UIAbilityLifecycleManager_CheckPrepareTerminateEnable_0200
 * @tc.desc: CheckPrepareTerminateEnable
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, CheckPrepareTerminateEnable_002, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    abilityRecord->isTerminating_ = true;
    EXPECT_EQ(uiAbilityLifecycleManager->CheckPrepareTerminateEnable(abilityRecord), false);
}

/**
 * @tc.name: UIAbilityLifecycleManager_CheckPrepareTerminateEnable_0300
 * @tc.desc: CheckPrepareTerminateEnable
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, CheckPrepareTerminateEnable_003, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::PAGE;
    abilityRequest.abilityInfo.isStageBasedModel = false;
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    abilityRecord->isTerminating_ = false;
    EXPECT_EQ(uiAbilityLifecycleManager->CheckPrepareTerminateEnable(abilityRecord), false);
}

/**
 * @tc.name: UIAbilityLifecycleManager_CheckPrepareTerminateEnable_0400
 * @tc.desc: CheckPrepareTerminateEnable
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, CheckPrepareTerminateEnable_004, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::SERVICE;
    abilityRequest.abilityInfo.isStageBasedModel = true;
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    abilityRecord->isTerminating_ = false;
    EXPECT_EQ(uiAbilityLifecycleManager->CheckPrepareTerminateEnable(abilityRecord), false);
}

/**
 * @tc.name: UIAbilityLifecycleManager_CheckPrepareTerminateEnable_0500
 * @tc.desc: CheckPrepareTerminateEnable
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, CheckPrepareTerminateEnable_005, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::PAGE;
    abilityRequest.abilityInfo.isStageBasedModel = true;
    abilityRequest.appInfo.accessTokenId = IPCSkeleton::GetCallingTokenID();
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    abilityRecord->isTerminating_ = false;
    EXPECT_EQ(uiAbilityLifecycleManager->CheckPrepareTerminateEnable(abilityRecord), false);
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
}  // namespace AAFwk
}  // namespace OHOS
