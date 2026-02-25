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
#include "ability_util.h"
#define private public
#define protected public
#include "ability_record.h"
#include "app_mgr_util.h"
#include "scene_board/ui_ability_lifecycle_manager.h"
#undef protected
#undef private
#include "ability_start_setting.h"
#include "app_scheduler.h"
#include "app_utils.h"
#include "app_mgr_client.h"
#include "mock_ability_info_callback_stub.h"
#include "process_options.h"
#include "session/host/include/session.h"
#include "session_info.h"
#include "startup_util.h"
#define private public
#define protected public
#include "ability_manager_service.h"
#undef protected
#undef private
#include "ability_scheduler_mock.h"
#include "hilog_tag_wrapper.h"
#include "mock_my_flag.h"
#include "mock_permission_verification.h"

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
class UIAbilityLifecycleManagerSecondTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    UIAbilityRecordPtr InitAbilityRecord();
};

void UIAbilityLifecycleManagerSecondTest::SetUpTestCase() {}

void UIAbilityLifecycleManagerSecondTest::TearDownTestCase() {}

void UIAbilityLifecycleManagerSecondTest::SetUp() {}

void UIAbilityLifecycleManagerSecondTest::TearDown() {}

class UIAbilityLifecycleManagerSecondTestStub : public IRemoteStub<IAbilityConnection> {
public:
    UIAbilityLifecycleManagerSecondTestStub() {};
    virtual ~UIAbilityLifecycleManagerSecondTestStub() {};

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

UIAbilityRecordPtr UIAbilityLifecycleManagerSecondTest::InitAbilityRecord()
{
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::PAGE;
    UIAbilityRecordPtr abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    return abilityRecord;
}

/**
 * @tc.name: UIAbilityLifecycleManager_HandleLegacyAcceptWantDone_0100
 * @tc.desc: HandleLegacyAcceptWantDone
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerSecondTest, HandleLegacyAcceptWantDone_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "HandleLegacyAcceptWantDone_001 begin.");
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    Want want;
    AbilityRequest abilityRequest;
    want.SetParam("ohos.anco.param.missionAffinity", false);
    abilityRequest.want = want;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->persistentId = 100;
    abilityRequest.sessionInfo = sessionInfo;
    UIAbilityRecordPtr abilityRecord = InitAbilityRecord();
    abilityRecord->collaboratorType_ = CollaboratorType::RESERVE_TYPE;
    mgr->sessionAbilityMap_.emplace(sessionInfo->persistentId, abilityRecord);
    bool reuse = false;
    std::string flag = "";
    int32_t requestId = 1;
    SpecifiedRequest specifiedRequest(requestId, abilityRequest);
    mgr->HandleLegacyAcceptWantDone(specifiedRequest, flag, want);
    EXPECT_EQ(specifiedRequest.abilityRequest.specifiedFlag, "");
    flag = "specified";
    mgr->HandleLegacyAcceptWantDone(specifiedRequest, flag, want);
    EXPECT_EQ(specifiedRequest.abilityRequest.specifiedFlag, flag);
    TAG_LOGI(AAFwkTag::TEST, "HandleLegacyAcceptWantDone_001 end.");
}

/**
 * @tc.name: UIAbilityLifecycleManager_FindRecordFromSessionMap_001
 * @tc.desc: FindRecordFromSessionMap
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerSecondTest, FindRecordFromSessionMap_001, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    EXPECT_NE(mgr, nullptr);

    AbilityRequest abilityRequest;
    int32_t appIndex = 0;
    const int32_t sessionId = 100;
    int32_t dlpIndex = 1001;
    abilityRequest.want.SetParam(DLP_INDEX, dlpIndex);

    auto ret = mgr->FindRecordFromSessionMap(abilityRequest);
    EXPECT_EQ(ret, nullptr);
    abilityRequest.want.RemoveParam(DLP_INDEX);

    (void)AbilityRuntime::StartupUtil::GetAppIndex(abilityRequest.want, appIndex);
    auto instanceKey = abilityRequest.want.GetStringParam(Want::APP_INSTANCE_KEY);
    std::unordered_map<int32_t, UIAbilityRecordPtr> sessionAbilityMap_;

    abilityRequest.appInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::PAGE;
    UIAbilityRecordPtr abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    mgr->sessionAbilityMap_[sessionId] = abilityRecord;

    const auto info = abilityRecord->GetAbilityInfo();

    ret = mgr->FindRecordFromSessionMap(abilityRequest);
    EXPECT_NE(ret, nullptr);
    EXPECT_EQ(ret, abilityRecord);
    EXPECT_EQ(info.bundleName, abilityRequest.abilityInfo.bundleName);
    EXPECT_EQ(info.moduleName, abilityRequest.abilityInfo.moduleName);
    EXPECT_EQ(appIndex, abilityRecord->GetAppIndex());
    EXPECT_EQ(instanceKey, abilityRecord->GetInstanceKey());
}

/**
 * @tc.name: UIAbilityLifecycleManager_IsSpecifiedModuleLoaded_0100
 * @tc.desc: IsSpecifiedModuleLoaded PrepareTerminateAppAndGetRemainingInner
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerSecondTest, IsSpecifiedModuleLoaded_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "IsSpecifiedModuleLoaded_001 begin.");
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    AbilityRequest abilityRequest;
    bool isDebug = false;
    EXPECT_FALSE(mgr->IsSpecifiedModuleLoaded(abilityRequest, false, isDebug));
    TAG_LOGI(AAFwkTag::TEST, "IsSpecifiedModuleLoaded_001 end.");
}

/**
 * @tc.name: UIAbilityLifecycleManager_IsSpecifiedModuleLoaded_003
 * @tc.desc: IsSpecifiedModuleLoaded
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerSecondTest, IsSpecifiedModuleLoaded_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "IsSpecifiedModuleLoaded_003 begin.");
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    AbilityRequest abilityRequest;
    bool isDebug = false;
    auto ret = mgr->IsSpecifiedModuleLoaded(abilityRequest, true, isDebug);
    EXPECT_EQ(ret, false);

    mgr->sessionAbilityMap_.emplace(0, nullptr);

    int32_t uid = 100;
    std::string instanceKey = "testInstanceKey";
    abilityRequest.abilityInfo.uid = uid;
    abilityRequest.want.SetParam(Want::APP_INSTANCE_KEY, instanceKey);
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    mgr->sessionAbilityMap_.emplace(1, abilityRecord);

    ret = mgr->IsSpecifiedModuleLoaded(abilityRequest, true, isDebug);
    EXPECT_EQ(ret, false);

    abilityRecord->SetInstanceKey(instanceKey);
    ret = mgr->IsSpecifiedModuleLoaded(abilityRequest, true, isDebug);
    EXPECT_EQ(ret, true);
    TAG_LOGI(AAFwkTag::TEST, "IsSpecifiedModuleLoaded_003 end.");
}

/**
 * @tc.name: UIAbilityLifecycleManager_PrepareTerminateAppAndGetRemainingInner_0100
 * @tc.desc: PrepareTerminateAppAndGetRemainingInner
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerSecondTest, PrepareTerminateAppAndGetRemainingInner_001, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);

    int32_t pid = 1;
    std::string moduleName = "testModuleName";
    std::vector<sptr<IRemoteObject>> tokens;
    std::vector<sptr<IRemoteObject>> remainingTokens;

    auto ret = uiAbilityLifecycleManager->PrepareTerminateAppAndGetRemainingInner(pid, moduleName, tokens);
    EXPECT_EQ(ret, remainingTokens);

    std::shared_ptr<UIAbilityLifecycleManager::PrepareTerminateByPidRecord> record =
        std::make_shared<UIAbilityLifecycleManager::PrepareTerminateByPidRecord>(
        pid, moduleName, false, 0, false);
    uiAbilityLifecycleManager->prepareTerminateByPidRecords_.push_back(record);
    ret = uiAbilityLifecycleManager->PrepareTerminateAppAndGetRemainingInner(pid, moduleName, tokens);
    EXPECT_EQ(ret, remainingTokens);
}

/**
 * @tc.name: UIAbilityLifecycleManager_PrepareTerminateAppAndGetRemaining_001
 * @tc.desc: PrepareTerminateAppAndGetRemaining
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerSecondTest, PrepareTerminateAppAndGetRemaining_001, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);

    int32_t pid = 1;
    std::vector<sptr<IRemoteObject>> tokens;
    std::vector<sptr<IRemoteObject>> remainingTokens;
    auto ret = uiAbilityLifecycleManager->PrepareTerminateAppAndGetRemaining(pid, tokens);
    EXPECT_EQ(ret, remainingTokens);

    AbilityRequest abilityRequest;
    abilityRequest.appInfo.accessTokenId = IPCSkeleton::GetCallingTokenID();
    abilityRequest.sessionInfo = nullptr;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    abilityRecord->abilityInfo_.applicationInfo.accessTokenId = 1;
    uiAbilityLifecycleManager->sessionAbilityMap_.emplace(0, abilityRecord);
    sptr<IRemoteObject> token = abilityRecord->GetToken()->AsObject();
    tokens.push_back(token);
    AAFwk::MyFlag::flag_ = 1;
    AppUtils::isStartOptionsWithAnimation_ = true;
    ret = uiAbilityLifecycleManager->PrepareTerminateAppAndGetRemaining(pid, tokens);
    EXPECT_EQ(ret, remainingTokens);
}

/**
 * @tc.name: UIAbilityLifecycleManager_ProcessColdStartBranch_001
 * @tc.desc: ProcessColdStartBranch
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerSecondTest, ProcessColdStartBranch_001, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    AbilityRequest abilityRequest;
    bool isColdStart = false;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    auto sessionInfo = sptr<SessionInfo>(new SessionInfo());
    auto ret = uiAbilityLifecycleManager->ProcessColdStartBranch(abilityRequest, sessionInfo,
        abilityRecord, isColdStart);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: UIAbilityLifecycleManager_ProcessColdStartBranch_002
 * @tc.desc: ProcessColdStartBranch
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerSecondTest, ProcessColdStartBranch_002, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    AbilityRequest abilityRequest;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->requestId = 100;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    abilityRecord->SetIsHook(true);
    bool isColdStart = true;
    auto ret = uiAbilityLifecycleManager->ProcessColdStartBranch(abilityRequest, sessionInfo, abilityRecord,
        isColdStart);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: UIAbilityLifecycleManager_TryProcessHookModule_001
 * @tc.desc: TryProcessHookModule
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerSecondTest, TryProcessHookModule_001, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    AbilityRequest abilityRequest;
    auto specifiedRequest = std::make_shared<AAFwk::SpecifiedRequest>(0, abilityRequest);
    auto ret = uiAbilityLifecycleManager->TryProcessHookModule(*specifiedRequest, false);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: UIAbilityLifecycleManager_TryProcessHookModule_002
 * @tc.desc: TryProcessHookModule
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerSecondTest, TryProcessHookModule_002, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    AbilityRequest abilityRequest;
    int32_t appIndex = 0;
    const int32_t sessionId = 100;
    (void)AbilityRuntime::StartupUtil::GetAppIndex(abilityRequest.want, appIndex);
    auto instanceKey = abilityRequest.want.GetStringParam(Want::APP_INSTANCE_KEY);

    abilityRequest.appInfo.bundleName = "com.example.unittest";
    UIAbilityRecordPtr abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    abilityRecord->SetIsHook(true);
    abilityRecord->SetHookOff(true);
    uiAbilityLifecycleManager->sessionAbilityMap_[sessionId] = abilityRecord;

    auto specifiedRequest = std::make_shared<AAFwk::SpecifiedRequest>(0, abilityRequest);
    auto ret = uiAbilityLifecycleManager->TryProcessHookModule(*specifiedRequest, true);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: UIAbilityLifecycleManager_TryProcessHookModule_003
 * @tc.desc: TryProcessHookModule
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerSecondTest, TryProcessHookModule_003, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    AbilityRequest abilityRequest;
    int32_t appIndex = 0;
    const int32_t sessionId = 100;
    (void)AbilityRuntime::StartupUtil::GetAppIndex(abilityRequest.want, appIndex);
    auto instanceKey = abilityRequest.want.GetStringParam(Want::APP_INSTANCE_KEY);

    abilityRequest.appInfo.bundleName = "com.example.unittest";
    UIAbilityRecordPtr abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    abilityRecord->SetIsHook(true);
    uiAbilityLifecycleManager->sessionAbilityMap_[sessionId] = abilityRecord;
    auto specifiedRequest = std::make_shared<AAFwk::SpecifiedRequest>(100, abilityRequest);
    auto ret = uiAbilityLifecycleManager->TryProcessHookModule(*specifiedRequest, true);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: UIAbilityLifecycleManager_RevokeDelegator_001
 * @tc.desc: RevokeDelegator
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerSecondTest, RevokeDelegator_001, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    sptr<IRemoteObject> token = nullptr;
    auto ret = uiAbilityLifecycleManager->RevokeDelegator(token);
    EXPECT_EQ(ret, ERR_INVALID_CONTEXT);
}

/**
 * @tc.name: UIAbilityLifecycleManager_RevokeDelegator_002
 * @tc.desc: RevokeDelegator
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerSecondTest, RevokeDelegator_002, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    Rosen::SessionInfo info;
    sptr<Rosen::ISession> session = new Rosen::Session(info);
    EXPECT_NE(session, nullptr);
    auto ret = uiAbilityLifecycleManager->RevokeDelegator(session->AsObject());
    EXPECT_EQ(ret, ERR_INVALID_CONTEXT);
}

/**
 * @tc.name: UIAbilityLifecycleManager_RevokeDelegator_003
 * @tc.desc: RevokeDelegator
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerSecondTest, RevokeDelegator_003, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    auto abilityRecord = InitAbilityRecord();
    auto ret = uiAbilityLifecycleManager->RevokeDelegator(abilityRecord->GetToken());
    EXPECT_EQ(ret, ERR_INVALID_CONTEXT);
}

/**
 * @tc.name: UIAbilityLifecycleManager_RevokeDelegator_004
 * @tc.desc: RevokeDelegator
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerSecondTest, RevokeDelegator_004, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.accessTokenId = IPCSkeleton::GetCallingTokenID();
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    abilityRecord->currentState_ = AbilityState::BACKGROUND;
    abilityRecord->isAbilityForegrounding_ = false;
    auto ret = uiAbilityLifecycleManager->RevokeDelegator(abilityRecord->GetToken());
    EXPECT_EQ(ret, ERR_ABILITY_NOT_FOREGROUND);
}

/**
 * @tc.name: UIAbilityLifecycleManager_RevokeDelegator_005
 * @tc.desc: RevokeDelegator
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerSecondTest, RevokeDelegator_005, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.accessTokenId = IPCSkeleton::GetCallingTokenID();
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    abilityRecord->isAbilityForegrounding_ = true;
    auto ret = uiAbilityLifecycleManager->RevokeDelegator(abilityRecord->GetToken());
    EXPECT_EQ(ret, ERR_NOT_HOOK);
}

/**
 * @tc.name: UIAbilityLifecycleManager_RevokeDelegator_006
 * @tc.desc: RevokeDelegator
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerSecondTest, RevokeDelegator_006, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.accessTokenId = IPCSkeleton::GetCallingTokenID();
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    abilityRecord->isAbilityForegrounding_ = true;
    abilityRecord->isHook_ = true;
    abilityRecord->abilityInfo_.launchMode = AppExecFwk::LaunchMode::SPECIFIED;
    auto ret = uiAbilityLifecycleManager->RevokeDelegator(abilityRecord->GetToken());
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: UIAbilityLifecycleManager_RevokeDelegator_007
 * @tc.desc: RevokeDelegator
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerSecondTest, RevokeDelegator_007, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.accessTokenId = IPCSkeleton::GetCallingTokenID();
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    abilityRecord->isAbilityForegrounding_ = true;
    abilityRecord->isHook_ = true;
    auto ret = uiAbilityLifecycleManager->RevokeDelegator(abilityRecord->GetToken());
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: UIAbilityLifecycleManager_RevokeDelegator_008
 * @tc.desc: RevokeDelegator
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerSecondTest, RevokeDelegator_008, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    AbilityRequest abilityRequest;
    abilityRequest.sessionInfo = new SessionInfo();
    abilityRequest.appInfo.accessTokenId = IPCSkeleton::GetCallingTokenID();
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    abilityRecord->isAbilityForegrounding_ = true;
    abilityRecord->isHook_ = true;
    auto ret = uiAbilityLifecycleManager->RevokeDelegator(abilityRecord->GetToken());
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: UIAbilityLifecycleManager_RevokeDelegator_009
 * @tc.desc: RevokeDelegator
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerSecondTest, RevokeDelegator_009, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    AbilityRequest abilityRequest;
    abilityRequest.sessionInfo = new SessionInfo();
    abilityRequest.appInfo.accessTokenId = IPCSkeleton::GetCallingTokenID();
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    abilityRecord->isAbilityForegrounding_ = true;
    abilityRecord->isHook_ = true;
    abilityRecord->SetLaunchWant(std::make_shared<Want>());
    auto ret = uiAbilityLifecycleManager->RevokeDelegator(abilityRecord->GetToken());
    EXPECT_EQ(ret, ERR_FROM_WINDOW);
}

/**
 * @tc.name: UIAbilityLifecycleManager_AbilityWindowConfigTransactionDone_0100
 * @tc.desc: AbilityWindowConfigTransactionDone
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerSecondTest, AbilityWindowConfigTransactionDone_001, TestSize.Level1)
{
    auto mgr = std::make_unique<UIAbilityLifecycleManager>();
    WindowConfig windowConfig;

    auto ret = mgr->AbilityWindowConfigTransactionDone(nullptr, windowConfig);

    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: UIAbilityLifecycleManager_AddStartCallerTimestamp_0100
 * @tc.desc: AddStartCallerTimestamp
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerSecondTest, AddStartCallerTimestamp_001, TestSize.Level1)
{
    auto mgr = std::make_unique<UIAbilityLifecycleManager>();
    std::map<int32_t, std::vector<int64_t>> startUIAbilityCallerTimestamps;
    std::vector<int64_t> callerTimestamps = {};
    constexpr int32_t START_UI_ABILITY_PER_SECOND_UPPER_LIMIT = 20;
    auto curTimeNs = AbilityUtil::GetSysTimeNs();
    for (int i = 0; i < START_UI_ABILITY_PER_SECOND_UPPER_LIMIT + 2; i++) {
        callerTimestamps.emplace_back(curTimeNs);
    }
    startUIAbilityCallerTimestamps.emplace(2, callerTimestamps);
    mgr->startUIAbilityCallerTimestamps_ = startUIAbilityCallerTimestamps;

    auto ret = mgr->AddStartCallerTimestamp(2);

    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: UIAbilityLifecycleManager_AddStartCallerTimestamp_0200
 * @tc.desc: AddStartCallerTimestamp
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerSecondTest, AddStartCallerTimestamp_002, TestSize.Level1)
{
    auto mgr = std::make_unique<UIAbilityLifecycleManager>();
    std::map<int32_t, std::vector<int64_t>> startUIAbilityCallerTimestamps_;
    constexpr int32_t START_UI_ABILITY_PER_SECOND_UPPER_LIMIT = 20;
    for (int i = 0; i < START_UI_ABILITY_PER_SECOND_UPPER_LIMIT - 3; i++) {
        startUIAbilityCallerTimestamps_.emplace(i, std::vector<int64_t>{1, 2});
    }

    auto ret = mgr->AddStartCallerTimestamp(2);

    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: UIAbilityLifecycleManager_NotifySCBToRecoveryAfterInterception_0100
 * @tc.desc: NotifySCBToRecoveryAfterInterception
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerSecondTest, NotifySCBToRecoveryAfterInterception_001, TestSize.Level1)
{
    auto mgr = std::make_unique<UIAbilityLifecycleManager>();
    AbilityRequest abilityRequest = {};

    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::PAGE;
    abilityRequest.abilityInfo.isStageBasedModel = false;
    abilityRequest.abilityInfo.launchMode == AppExecFwk::LaunchMode::SPECIFIED;

    auto ret = mgr->NotifySCBToRecoveryAfterInterception(abilityRequest);

    EXPECT_EQ(ret, 22);
}

/**
 * @tc.name: UIAbilityLifecycleManager_NotifySCBToRecoveryAfterInterception_0200
 * @tc.desc: NotifySCBToRecoveryAfterInterception
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerSecondTest, NotifySCBToRecoveryAfterInterception_002, TestSize.Level1)
{
    auto mgr = std::make_unique<UIAbilityLifecycleManager>();
    AbilityRequest abilityRequest ={};
    bool isNewProcessMode = abilityRequest.processOptions &&
    ProcessOptions::IsNewProcessMode(abilityRequest.processOptions->processMode);
    abilityRequest.processOptions = nullptr;
    abilityRequest.abilityInfo.isolationProcess = true;
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::PAGE;
    abilityRequest.abilityInfo.isStageBasedModel = true;
    AppUtils::GetInstance().isStartOptionsWithAnimation_ = true;

    auto ret = mgr->NotifySCBToRecoveryAfterInterception(abilityRequest);

    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: UIAbilityLifecycleManager_DispatchBackground_0100
 * @tc.desc: DispatchBackground
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerSecondTest, DispatchBackground_001, TestSize.Level1)
{
    auto mgr = std::make_unique<UIAbilityLifecycleManager>();
    AbilityRequest abilityRequest;
    auto abilityRecord = std::make_shared<UIAbilityRecord>(
        abilityRequest.want, abilityRequest.abilityInfo, abilityRequest.appInfo, abilityRequest.requestCode);
    abilityRecord->currentState_ = AbilityState::INITIAL;

    auto ret = mgr->DispatchBackground(abilityRecord);

    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: UIAbilityLifecycleManager_PreCreateProcessName_0100
 * @tc.desc: PreCreateProcessName
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerSecondTest, PreCreateProcessName_001, TestSize.Level1)
{
    auto mgr = std::make_unique<UIAbilityLifecycleManager>();
    AbilityRequest abilityRequest;
    abilityRequest.processOptions = std::make_shared<ProcessOptions>();
    abilityRequest.processOptions->processMode = ProcessMode::UNSPECIFIED;
    abilityRequest.processOptions->processName = "fffAAABBBCCCggg";

    mgr->PreCreateProcessName(abilityRequest);

    EXPECT_EQ(abilityRequest.processOptions->processName, "fffAAABBBCCCggg");
}

/**
 * @tc.name: UIAbilityLifecycleManager_PreCreateProcessName_0200
 * @tc.desc: PreCreateProcessName
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerSecondTest, PreCreateProcessName_002, TestSize.Level1)
{
    auto mgr = std::make_unique<UIAbilityLifecycleManager>();
    AbilityRequest abilityRequest;
    abilityRequest.processOptions = std::make_shared<ProcessOptions>();
    abilityRequest.processOptions->processMode = ProcessMode::NEW_PROCESS_ATTACH_TO_PARENT;
    abilityRequest.processOptions->processName = "fffAAABBBCCCggg";
    abilityRequest.abilityInfo.bundleName = "BHi";
    abilityRequest.abilityInfo.moduleName = "MHi";
    abilityRequest.abilityInfo.name = "NHi";

    mgr->PreCreateProcessName(abilityRequest);

    auto processName = abilityRequest.processOptions->processName;
    auto processNameSub = processName.substr(0, processName.find_last_of(':'));
    EXPECT_EQ(processNameSub, "BHi:MHi:NHi");
}

/**
 * @tc.name: UIAbilityLifecycleManager_BackToCallerAbilityWithResult_0700
 * @tc.desc: BackToCallerAbilityWithResult
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerSecondTest, BackToCallerAbilityWithResult_007, TestSize.Level1)
{
    auto mgr = std::make_unique<UIAbilityLifecycleManager>();

    auto ret = mgr->BackToCallerAbilityWithResult(nullptr, 1, nullptr, 0);

    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: UIAbilityLifecycleManager_BackToCallerAbilityWithResult_0800
 * @tc.desc: BackToCallerAbilityWithResult
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerSecondTest, BackToCallerAbilityWithResult_008, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();

    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.name = "TestAbility";
    abilityRequest.abilityInfo.applicationInfo.accessTokenId = 12345;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);

    int64_t callerRequestCode = 123456789;
    CallerRequestInfo requestInfo;
    requestInfo.requestCode = 1;
    requestInfo.pid = 100;
    requestInfo.backFlag = false;

    auto ret = mgr->BackToCallerAbilityWithResult(abilityRecord, 0, nullptr, callerRequestCode);

    EXPECT_EQ(ret, ERR_CALLER_NOT_EXISTS);
}

/**
 * @tc.name: UIAbilityLifecycleManager_SetLastExitReason_0100
 * @tc.desc: SetLastExitReason
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerSecondTest, SetLastExitReason_001, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    UIAbilityRecordPtr abilityRecord = InitAbilityRecord();
    abilityRecord->abilityInfo_.bundleName = "not empty";
    abilityRecord->sessionInfo_ = nullptr;

    mgr->SetLastExitReason(abilityRecord);

    EXPECT_NE(abilityRecord, nullptr);
    EXPECT_NE(abilityRecord->GetAbilityInfo().bundleName.empty(), true);
    EXPECT_EQ(abilityRecord->GetSessionInfo(), nullptr);
}

/**
 * @tc.name: UIAbilityLifecycleManager_PrepareCloseUIAbility_0100
 * @tc.desc: PrepareCloseUIAbility
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerSecondTest, PrepareCloseUIAbility_001, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    mgr->terminateAbilityList_.clear();

    mgr->PrepareCloseUIAbility(nullptr, 0, nullptr, true);

    auto listSize = mgr->terminateAbilityList_.size();
    EXPECT_EQ(listSize, 0);
}

/**
 * @tc.name: UIAbilityLifecycleManager_NotifySCBToHandleAtomicServiceException_0100
 * @tc.desc: NotifySCBToHandleAtomicServiceException
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerSecondTest, NotifySCBToHandleAtomicServiceException_001, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    auto sessionInfo = sptr<SessionInfo>::MakeSptr();
    sessionInfo->sessionToken = nullptr;
    sessionInfo->errorCode = 0;
    std::string errorReason = "";

    mgr->NotifySCBToHandleAtomicServiceException(sessionInfo, 5, errorReason);

    EXPECT_EQ(sessionInfo->errorCode, 0);
}

/**
 * @tc.name: UIAbilityLifecycleManager_PrepareTerminateAbilityDone_0100
 * @tc.desc: PrepareTerminateAbilityDone
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerSecondTest, PrepareTerminateAbilityDone_001, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    AbilityRequest abilityRequest;
    auto abilityRecord = std::make_shared<UIAbilityRecord>(
        abilityRequest.want, abilityRequest.abilityInfo, abilityRequest.appInfo, abilityRequest.requestCode);
    abilityRecord->isPrepareTerminate_ = true;

    mgr->PrepareTerminateAbilityDone(abilityRecord, false);
    EXPECT_EQ(abilityRecord->isPrepareTerminate_, true);
}

/**
 * @tc.name: UIAbilityLifecycleManager_PrepareTerminateAbilityDone_0200
 * @tc.desc: PrepareTerminateAbilityDone
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerSecondTest, PrepareTerminateAbilityDone_002, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    AbilityRequest abilityRequest;
    auto abilityRecord = std::make_shared<UIAbilityRecord>(
        abilityRequest.want, abilityRequest.abilityInfo, abilityRequest.appInfo, abilityRequest.requestCode);
    abilityRecord->isPrepareTerminate_ = true;
    abilityRecord->isPrepareTerminateAbilityCalled_.store(true);

    mgr->PrepareTerminateAbilityDone(abilityRecord, false);
    EXPECT_EQ(abilityRecord->isPrepareTerminate_, false);
}

/**
 * @tc.name: UIAbilityLifecycleManager_CheckPrepareTerminateTokens_0100
 * @tc.desc: CheckPrepareTerminateTokens
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerSecondTest, CheckPrepareTerminateTokens_001, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    std::vector<sptr<IRemoteObject>> tokens = {};
    uint32_t tokenId = 0;
    std::map<std::string, std::vector<sptr<IRemoteObject>>> tokensPerModuleName = {};
    AppUtils::isStartOptionsWithAnimation_ = true;

    auto ret = mgr->CheckPrepareTerminateTokens(tokens, tokenId, tokensPerModuleName);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: UIAbilityLifecycleManager_CancelPrepareTerminate_0100
 * @tc.desc: CancelPrepareTerminate
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerSecondTest, CancelPrepareTerminate_001, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    auto prepareTerminateByPidRecords1 = std::make_shared<UIAbilityLifecycleManager::PrepareTerminateByPidRecord>(
        1,
        "HelloWorld",
        false,
        0,
        false
    );
    auto prepareTerminateByPidRecords2 = std::make_shared<UIAbilityLifecycleManager::PrepareTerminateByPidRecord>(
        2,
        "HiWorld",
        false,
        0,
        false
    );
    mgr->prepareTerminateByPidRecords_.emplace_back(prepareTerminateByPidRecords1);
    mgr->prepareTerminateByPidRecords_.emplace_back(prepareTerminateByPidRecords2);
    AbilityRequest abilityRequest;
    auto abilityRecord = std::make_shared<UIAbilityRecord>(
        abilityRequest.want, abilityRequest.abilityInfo, abilityRequest.appInfo, abilityRequest.requestCode);
    abilityRecord->pid_ = 1;
    abilityRecord->abilityInfo_.moduleName = "HelloWorld";

    mgr->CancelPrepareTerminate(abilityRecord);
    auto recordSize = mgr->prepareTerminateByPidRecords_.size();
    EXPECT_EQ(recordSize, 1);
}

/**
 * @tc.name: UIAbilityLifecycleManager_CleanUIAbility_0100
 * @tc.desc: CleanUIAbility
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerSecondTest, CleanUIAbility_001, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();

    auto ret = mgr->CleanUIAbility(nullptr);
    EXPECT_EQ(ret, ERR_UI_ABILITY_MANAGER_NULL_ABILITY_RECORD);
}

/**
 * @tc.name: UIAbilityLifecycleManager_TryProcessHookModule_0400
 * @tc.desc: TryProcessHookModule
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerSecondTest, TryProcessHookModule_004, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    AbilityRequest abilityRequest;
    auto specifiedRequest = std::make_shared<AAFwk::SpecifiedRequest>(100, abilityRequest);

    auto ret = mgr->TryProcessHookModule(*specifiedRequest, false);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: UIAbilityLifecycleManager_TryProcessHookModule_0500
 * @tc.desc: TryProcessHookModule
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerSecondTest, TryProcessHookModule_005, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    mgr->sessionAbilityMap_.clear();
    AbilityRequest abilityRequest;
    auto specifiedRequest = std::make_shared<AAFwk::SpecifiedRequest>(100, abilityRequest);

    auto ret = mgr->TryProcessHookModule(*specifiedRequest, true);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: UIAbilityLifecycleManager_TryProcessHookModule_0600
 * @tc.desc: TryProcessHookModule
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerSecondTest, TryProcessHookModule_006, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();

    int32_t appIndex = 0;
    AbilityRequest abilityRequest;
    auto abilityRecord = std::make_shared<UIAbilityRecord>(
        abilityRequest.want, abilityRequest.abilityInfo, abilityRequest.appInfo, abilityRequest.requestCode);
    abilityRecord->SetIsHook(true);
    abilityRecord->SetHookOff(false);
    abilityRecord->abilityInfo_.bundleName = "HelloWorld";
    abilityRecord->abilityInfo_.moduleName = "HelloWorld";
    abilityRecord->SetInstanceKey("HelloWorld");

    mgr->sessionAbilityMap_.clear();
    mgr->sessionAbilityMap_.emplace(1, abilityRecord);
    
    AbilityRequest abilityRequest2;
    abilityRequest2.abilityInfo.bundleName = "HelloWorld";
    abilityRequest2.abilityInfo.moduleName = "HelloWorld";
    abilityRequest2.want.SetParam(Want::APP_INSTANCE_KEY, std::string("HelloWorld"));
    AbilityRuntime::StartupUtil::GetAppIndex(abilityRequest2.want, appIndex);
    abilityRecord->SetAppIndex(appIndex);

    auto specifiedRequest = std::make_shared<AAFwk::SpecifiedRequest>(100, abilityRequest2);

    auto ret = mgr->TryProcessHookModule(*specifiedRequest, true);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: UIAbilityLifecycleManager_AddSpecifiedRequest_0100
 * @tc.desc: AddSpecifiedRequest
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerSecondTest, AddSpecifiedRequest_001, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    mgr->specifiedRequestList_.clear();

    mgr->AddSpecifiedRequest(nullptr);
    EXPECT_EQ(mgr->specifiedRequestList_.size(), 0);
}

/**
 * @tc.name: UIAbilityLifecycleManager_AddSpecifiedRequest_0200
 * @tc.desc: AddSpecifiedRequest
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerSecondTest, AddSpecifiedRequest_002, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    mgr->specifiedRequestList_.clear();

    AbilityRequest abilityRequest;
    auto request = std::make_shared<SpecifiedRequest>(1, abilityRequest);

    mgr->AddSpecifiedRequest(request);
    EXPECT_EQ(mgr->specifiedRequestList_.size(), 1);
}

/**
 * @tc.name: UIAbilityLifecycleManager_HandleColdAcceptWantDone_0100
 * @tc.desc: HandleColdAcceptWantDone
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerSecondTest, HandleColdAcceptWantDone_001, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    AAFwk::Want want;
    std::string flag = "";
    AbilityRequest abilityRequest;
    SpecifiedRequest specifiedRequest(1, abilityRequest);

    mgr->sessionAbilityMap_.clear();

    auto ret = mgr->HandleColdAcceptWantDone(want, flag, specifiedRequest);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: UIAbilityLifecycleManager_HandleColdAcceptWantDone_0200
 * @tc.desc: HandleColdAcceptWantDone
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerSecondTest, HandleColdAcceptWantDone_002, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    AAFwk::Want want;
    std::string flag = "";
    AbilityRequest abilityRequest;
    SpecifiedRequest specifiedRequest(1, abilityRequest);
    specifiedRequest.persistentId = 1;

    auto abilityRecord = std::make_shared<UIAbilityRecord>(
        abilityRequest.want, abilityRequest.abilityInfo, abilityRequest.appInfo, abilityRequest.requestCode);
    abilityRecord->specifiedFlag_ = "";

    mgr->sessionAbilityMap_.clear();
    mgr->sessionAbilityMap_.emplace(1, abilityRecord);

    auto ret = mgr->HandleColdAcceptWantDone(want, flag, specifiedRequest);
    EXPECT_EQ(ret, true);
}


/**
 * @tc.name: UIAbilityLifecycleManager_IsBundleStarting_0100
 * @tc.desc: IsBundleStarting
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerSecondTest, IsBundleStarting_001, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    mgr->startingPids_ = {1, 2, 3};
    pid_t pid = 1;

    auto ret = mgr->IsBundleStarting(pid);

    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: UIAbilityLifecycleManager_IsBundleStarting_0200
 * @tc.desc: IsBundleStarting
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerSecondTest, IsBundleStarting_002, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    mgr->startingPids_ = {2, 3};
    pid_t pid = 1;

    auto ret = mgr->IsBundleStarting(pid);

    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: UIAbilityLifecycleManager_AddStartingPid_0100
 * @tc.desc: AddStartingPid
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerSecondTest, AddStartingPid_001, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    mgr->startingPids_ = {1, 2, 3};
    pid_t pid = 3;

    mgr->AddStartingPid(pid);

    EXPECT_NE(mgr->startingPids_.size(), 4);
}

/**
 * @tc.name: UIAbilityLifecycleManager_AddStartingPid_0200
 * @tc.desc: AddStartingPid
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerSecondTest, AddStartingPid_002, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    mgr->startingPids_ = {1, 2, 3};
    pid_t pid = 4;

    mgr->AddStartingPid(pid);

    EXPECT_EQ(mgr->startingPids_.size(), 4);
}

/**
 * @tc.name: UIAbilityLifecycleManager_RemoveStartingPid_0100
 * @tc.desc: RemoveStartingPid
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerSecondTest, RemoveStartingPid_001, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    mgr->startingPids_ = {1, 2, 3};
    pid_t pid = 4;

    mgr->RemoveStartingPid(pid);

    EXPECT_EQ(mgr->startingPids_.size(), 3);
}

/**
 * @tc.name: UIAbilityLifecycleManager_RemoveStartingPid_0200
 * @tc.desc: RemoveStartingPid
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerSecondTest, RemoveStartingPid_002, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    mgr->startingPids_ = {1, 2, 3};
    pid_t pid = 3;

    mgr->RemoveStartingPid(pid);

    EXPECT_EQ(mgr->startingPids_.size(), 2);
}

/**
 * @tc.name: UIAbilityLifecycleManager_RecordPidKilling_0100
 * @tc.desc: RecordPidKilling
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerSecondTest, RecordPidKilling_001, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    AbilityRequest abilityRequest;
    auto abilityRecord = std::make_shared<UIAbilityRecord>(
        abilityRequest.want, abilityRequest.abilityInfo, abilityRequest.appInfo, abilityRequest.requestCode);
    abilityRecord->pid_ = 1;
    std::unordered_map<int32_t, UIAbilityRecordPtr> sessionAbilityMap =
    {
        {1, abilityRecord},
    };
    mgr->sessionAbilityMap_ = sessionAbilityMap;
    pid_t pid = 1;
    std::string reason = "HelloWorld";
    bool isKillPrecedeStart = true;
    
    mgr->RecordPidKilling(pid, reason, isKillPrecedeStart);
    EXPECT_EQ(mgr->sessionAbilityMap_[1]->killReason_, "HelloWorld");
    EXPECT_EQ(mgr->sessionAbilityMap_[1]->isKillPrecedeStart_, true);
}

/**
 * @tc.name: UIAbilityLifecycleManager_AbilityWindowConfigTransactionDone_0100
 * @tc.desc: AbilityWindowConfigTransactionDone
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerSecondTest, AbilityWindowConfigTransactionDone_002, TestSize.Level1)
{
    auto mgr = std::make_unique<UIAbilityLifecycleManager>();

    AbilityRequest abilityRequest;
    auto abilityRecord = std::make_shared<UIAbilityRecord>(
        abilityRequest.want, abilityRequest.abilityInfo, abilityRequest.appInfo, abilityRequest.requestCode);

    auto token = sptr<Token>::MakeSptr(abilityRecord);
    abilityRecord->token_ = token;
    
    std::list<UIAbilityRecordPtr> terminateAbilityList_ =
    {
        abilityRecord,
    };
    
    WindowConfig windowConfig;

    auto ret = mgr->AbilityWindowConfigTransactionDone(token, windowConfig);

    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: UIAbilityLifecycleManager_AddStartCallerTimestamp_0300
 * @tc.desc: AddStartCallerTimestamp
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerSecondTest, AddStartCallerTimestamp_003, TestSize.Level1)
{
    auto mgr = std::make_unique<UIAbilityLifecycleManager>();
    mgr->startUIAbilityCallerTimestamps_.clear();
    mgr->startUIAbilityCallerTimestamps_.emplace(1, std::vector<int64_t>{100000, 300000});

    auto ret = mgr->AddStartCallerTimestamp(1);

    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: BackToCallerAbilityWithResultLocked_001
 * @tc.desc: BackToCallerAbilityWithResultLocked
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerSecondTest, BackToCallerAbilityWithResultLocked_001, TestSize.Level1)
{
    auto mgr = std::make_unique<UIAbilityLifecycleManager>();
    auto currentSessionInfo = sptr<AAFwk::SessionInfo>::MakeSptr();
    Rosen::SessionInfo info;
    sptr<Rosen::ISession> session = sptr<Rosen::Session>::MakeSptr(info);
    currentSessionInfo->sessionToken = session->AsObject();

    AbilityRequest abilityRequest;
    auto callerAbilityRecord = std::make_shared<UIAbilityRecord>(
        abilityRequest.want, abilityRequest.abilityInfo, abilityRequest.appInfo, abilityRequest.requestCode);
    callerAbilityRecord->sessionInfo_ = nullptr;

    auto ret = mgr->BackToCallerAbilityWithResultLocked(currentSessionInfo, callerAbilityRecord);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: QueryCallerTokenIdForAnco_001
 * @tc.desc: Test QueryCallerTokenIdForAnco with empty callerInfoMap_
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerSecondTest, QueryCallerTokenIdForAnco_001, TestSize.Level1)
{
    auto mgr = std::make_unique<UIAbilityLifecycleManager>();
    const std::string sessionId = "non_existent_session_id";
    uint32_t callerTokenId = 0;

    auto ret = mgr->QueryCallerTokenIdForAnco(sessionId, callerTokenId);

    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: QueryCallerTokenIdForAnco_002
 * @tc.desc: Test QueryCallerTokenIdForAnco with non-existent session ID
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerSecondTest, QueryCallerTokenIdForAnco_002, TestSize.Level1)
{
    auto mgr = std::make_unique<UIAbilityLifecycleManager>();

    // Populate callerInfoMap_ with test data
    UIAbilityLifecycleManager::CallerInfo callerInfo;
    callerInfo.callerTokenId = 12345;
    callerInfo.targetWant.SetParam("testKey", std::string("testValue"));
    mgr->callerInfoMap_["existing_session_id"] = callerInfo;

    const std::string nonExistentSessionId = "non_existent_session_id";
    uint32_t callerTokenId = 0;

    auto ret = mgr->QueryCallerTokenIdForAnco(nonExistentSessionId, callerTokenId);

    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: QueryCallerTokenIdForAnco_003
 * @tc.desc: Test QueryCallerTokenIdForAnco successfully
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerSecondTest, QueryCallerTokenIdForAnco_003, TestSize.Level1)
{
    auto mgr = std::make_unique<UIAbilityLifecycleManager>();

    // Populate callerInfoMap_ with test data
    const std::string sessionId = "test_session_id";
    UIAbilityLifecycleManager::CallerInfo callerInfo;
    callerInfo.callerTokenId = 67890;
    callerInfo.targetWant.SetParam(Want::PARAM_RESV_CALLER_TOKEN, 11111);
    callerInfo.targetWant.SetParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME, std::string("com.test.bundle"));
    mgr->callerInfoMap_[sessionId] = callerInfo;

    uint32_t callerTokenId = 0;

    auto ret = mgr->QueryCallerTokenIdForAnco(sessionId, callerTokenId);

    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(callerTokenId, 67890);
}

/**
 * @tc.name: QueryCallerTokenIdForAnco_004
 * @tc.desc: Test QueryCallerTokenIdForAnco with multiple entries in callerInfoMap_
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerSecondTest, QueryCallerTokenIdForAnco_004, TestSize.Level1)
{
    auto mgr = std::make_unique<UIAbilityLifecycleManager>();

    // Populate callerInfoMap_ with multiple entries
    UIAbilityLifecycleManager::CallerInfo callerInfo1;
    callerInfo1.callerTokenId = 11111;
    mgr->callerInfoMap_["session_id_1"] = callerInfo1;

    UIAbilityLifecycleManager::CallerInfo callerInfo2;
    callerInfo2.callerTokenId = 22222;
    mgr->callerInfoMap_["session_id_2"] = callerInfo2;

    UIAbilityLifecycleManager::CallerInfo callerInfo3;
    callerInfo3.callerTokenId = 33333;
    mgr->callerInfoMap_["session_id_3"] = callerInfo3;

    uint32_t callerTokenId = 0;

    // Query the second entry
    auto ret = mgr->QueryCallerTokenIdForAnco("session_id_2", callerTokenId);

    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(callerTokenId, 22222);
}

/**
 * @tc.name: UpdateTokenIdAndWantWithRealCallerInfo_001
 * @tc.desc: Test UpdateTokenIdAndWantWithRealCallerInfo with empty session ID in want
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerSecondTest, UpdateTokenIdAndWantWithRealCallerInfo_001, TestSize.Level1)
{
    auto mgr = std::make_unique<UIAbilityLifecycleManager>();

    Want want;
    uint32_t realCallerTokenId = 0;

    mgr->UpdateTokenIdAndWantWithRealCallerInfo(want, realCallerTokenId);

    // Should return early without updating anything
    EXPECT_EQ(realCallerTokenId, 0);
}

/**
 * @tc.name: UpdateTokenIdAndWantWithRealCallerInfo_002
 * @tc.desc: Test UpdateTokenIdAndWantWithRealCallerInfo with non-existent session ID
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerSecondTest, UpdateTokenIdAndWantWithRealCallerInfo_002, TestSize.Level1)
{
    auto mgr = std::make_unique<UIAbilityLifecycleManager>();

    const std::string nonExistentSessionId = "non_existent_session_id";
    Want want;
    want.SetParam("ohos.ability.param.sessionId", nonExistentSessionId);

    uint32_t realCallerTokenId = 0;

    mgr->UpdateTokenIdAndWantWithRealCallerInfo(want, realCallerTokenId);

    // Should return early without updating anything
    EXPECT_EQ(realCallerTokenId, 0);
}

/**
 * @tc.name: UpdateTokenIdAndWantWithRealCallerInfo_003
 * @tc.desc: Test UpdateTokenIdAndWantWithRealCallerInfo successfully
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerSecondTest, UpdateTokenIdAndWantWithRealCallerInfo_003, TestSize.Level1)
{
    auto mgr = std::make_unique<UIAbilityLifecycleManager>();

    // Populate callerInfoMap_ with test data
    const std::string sessionId = "test_update_session_id";
    UIAbilityLifecycleManager::CallerInfo callerInfo;
    callerInfo.callerTokenId = 99999;
    callerInfo.targetWant.SetParam(Want::PARAM_RESV_CALLER_TOKEN, 11111);
    callerInfo.targetWant.SetParam(Want::PARAM_RESV_CALLER_UID, 22222);
    callerInfo.targetWant.SetParam(Want::PARAM_RESV_CALLER_PID, 33333);
    callerInfo.targetWant.SetParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME, std::string("com.update.test"));
    callerInfo.targetWant.SetParam(Want::PARAM_RESV_CALLER_ABILITY_NAME, std::string("UpdateAbility"));
    callerInfo.targetWant.SetParam(Want::PARAM_RESV_CALLER_APP_CLONE_INDEX, 44444);
    callerInfo.targetWant.SetParam(Want::PARAM_RESV_CALLER_NATIVE_NAME, std::string("nativeName"));
    callerInfo.targetWant.SetParam(Want::PARAM_RESV_CALLER_APP_ID, std::string("appId"));
    callerInfo.targetWant.SetParam(Want::PARAM_RESV_CALLER_APP_IDENTIFIER, std::string("appIdentifier"));
    mgr->callerInfoMap_[sessionId] = callerInfo;

    Want want;
    want.SetParam("ohos.ability.param.sessionId", sessionId);

    uint32_t realCallerTokenId = 0;

    mgr->UpdateTokenIdAndWantWithRealCallerInfo(want, realCallerTokenId);

    // Verify realCallerTokenId was updated
    EXPECT_EQ(realCallerTokenId, 99999);

    // Verify want parameters were updated
    EXPECT_EQ(want.GetIntParam(Want::PARAM_RESV_CALLER_TOKEN, 0), 11111);
    EXPECT_EQ(want.GetIntParam(Want::PARAM_RESV_CALLER_UID, 0), 22222);
    EXPECT_EQ(want.GetIntParam(Want::PARAM_RESV_CALLER_PID, 0), 33333);
    EXPECT_EQ(want.GetStringParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME), "com.update.test");
    EXPECT_EQ(want.GetStringParam(Want::PARAM_RESV_CALLER_ABILITY_NAME), "UpdateAbility");
    EXPECT_EQ(want.GetIntParam(Want::PARAM_RESV_CALLER_APP_CLONE_INDEX, 0), 44444);
    EXPECT_EQ(want.GetStringParam(Want::PARAM_RESV_CALLER_NATIVE_NAME), "nativeName");
    EXPECT_EQ(want.GetStringParam(Want::PARAM_RESV_CALLER_APP_ID), "appId");
    EXPECT_EQ(want.GetStringParam(Want::PARAM_RESV_CALLER_APP_IDENTIFIER), "appIdentifier");

    // Verify the entry was removed from callerInfoMap_
    EXPECT_TRUE(mgr->callerInfoMap_.empty());
}

/**
 * @tc.name: UpdateTokenIdAndWantWithRealCallerInfo_004
 * @tc.desc: Test UpdateTokenIdAndWantWithRealCallerInfo with partial caller info
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerSecondTest, UpdateTokenIdAndWantWithRealCallerInfo_004, TestSize.Level1)
{
    auto mgr = std::make_unique<UIAbilityLifecycleManager>();

    // Populate callerInfoMap_ with partial data
    const std::string sessionId = "partial_session_id";
    UIAbilityLifecycleManager::CallerInfo callerInfo;
    callerInfo.callerTokenId = 55555;
    callerInfo.targetWant.SetParam(Want::PARAM_RESV_CALLER_TOKEN, 66666);
    // Only set some parameters
    callerInfo.targetWant.SetParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME, std::string("com.partial.test"));
    mgr->callerInfoMap_[sessionId] = callerInfo;

    Want want;
    want.SetParam("ohos.ability.param.sessionId", sessionId);

    uint32_t realCallerTokenId = 0;

    mgr->UpdateTokenIdAndWantWithRealCallerInfo(want, realCallerTokenId);

    // Verify realCallerTokenId was updated
    EXPECT_EQ(realCallerTokenId, 55555);

    // Verify set parameters were updated
    EXPECT_EQ(want.GetIntParam(Want::PARAM_RESV_CALLER_TOKEN, 0), 66666);
    EXPECT_EQ(want.GetStringParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME), "com.partial.test");

    // Verify unset parameters use default values
    EXPECT_EQ(want.GetIntParam(Want::PARAM_RESV_CALLER_UID, 0), 0);
    EXPECT_EQ(want.GetIntParam(Want::PARAM_RESV_CALLER_PID, 0), 0);

    // Verify the entry was removed from callerInfoMap_
    EXPECT_TRUE(mgr->callerInfoMap_.empty());
}

/**
 * @tc.name: UpdateTokenIdAndWantWithRealCallerInfo_005
 * @tc.desc: Test UpdateTokenIdAndWantWithRealCallerInfo with multiple entries
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerSecondTest, UpdateTokenIdAndWantWithRealCallerInfo_005, TestSize.Level1)
{
    auto mgr = std::make_unique<UIAbilityLifecycleManager>();

    // Populate callerInfoMap_ with multiple entries
    UIAbilityLifecycleManager::CallerInfo callerInfo1;
    callerInfo1.callerTokenId = 11111;
    mgr->callerInfoMap_["session_1"] = callerInfo1;

    UIAbilityLifecycleManager::CallerInfo callerInfo2;
    callerInfo2.callerTokenId = 22222;
    callerInfo2.targetWant.SetParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME, std::string("com.session2.test"));
    mgr->callerInfoMap_["session_2"] = callerInfo2;

    UIAbilityLifecycleManager::CallerInfo callerInfo3;
    callerInfo3.callerTokenId = 33333;
    mgr->callerInfoMap_["session_3"] = callerInfo3;

    Want want;
    want.SetParam("ohos.ability.param.sessionId", std::string("session_2"));

    uint32_t realCallerTokenId = 0;

    mgr->UpdateTokenIdAndWantWithRealCallerInfo(want, realCallerTokenId);

    // Verify correct entry was processed
    EXPECT_EQ(realCallerTokenId, 22222);
    EXPECT_EQ(want.GetStringParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME), "com.session2.test");

    // Verify only one entry was removed
    EXPECT_EQ(mgr->callerInfoMap_.size(), 2);
    EXPECT_TRUE(mgr->callerInfoMap_.find("session_1") != mgr->callerInfoMap_.end());
    EXPECT_TRUE(mgr->callerInfoMap_.find("session_3") != mgr->callerInfoMap_.end());
    EXPECT_TRUE(mgr->callerInfoMap_.find("session_2") == mgr->callerInfoMap_.end());
}
}  // namespace AAFwk
}  // namespace OHOS