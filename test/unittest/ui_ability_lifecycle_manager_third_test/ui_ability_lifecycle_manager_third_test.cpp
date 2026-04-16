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
#include "app_utils.h"
#include "scene_board/ui_ability_lifecycle_manager.h"
#undef protected
#undef private
#include "ability_start_setting.h"
#include "app_scheduler.h"
#include "app_mgr_client.h"
#include "display_util.h"
#include "mock_ability_info_callback_stub.h"
#include "mock_scene_session_manager_lite.h"
#include "process_options.h"
#include "session/host/include/session.h"
#include "session_info.h"
#include "session_manager_lite.h"
#include "startup_util.h"
#include "start_params_by_SCB.h"
#include "status_bar_delegate_interface.h"
#include "scene_board/status_bar_delegate_manager.h"
#include "server_constant.h"
#define private public
#define protected public
#include "ability_manager_service.h"
#undef protected
#undef private
#include "ability_scheduler_mock.h"
#include "hilog_tag_wrapper.h"
#include "mock_app_mgr_service.h"
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
constexpr int32_t TIMEOUT_VALUE = 4000;
};
class UIAbilityLifecycleManagerThirdTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    std::shared_ptr<UIAbilityRecord> InitAbilityRecord();
};

void UIAbilityLifecycleManagerThirdTest::SetUpTestCase() {}

void UIAbilityLifecycleManagerThirdTest::TearDownTestCase() {}

void UIAbilityLifecycleManagerThirdTest::SetUp() {}

void UIAbilityLifecycleManagerThirdTest::TearDown() {}

class UIAbilityLifecycleManagerThirdTestStub : public IRemoteStub<IAbilityConnection> {
public:
    UIAbilityLifecycleManagerThirdTestStub() {};
    virtual ~UIAbilityLifecycleManagerThirdTestStub() {};

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

std::shared_ptr<UIAbilityRecord> UIAbilityLifecycleManagerThirdTest::InitAbilityRecord()
{
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::PAGE;
    return UIAbilityRecord::CreateAbilityRecord(abilityRequest);
}

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

class MockIRemoteObject : public IRemoteObject {
public:
    MockIRemoteObject() : IRemoteObject(u"mock_i_remote_object") {}

    ~MockIRemoteObject() {}

    int32_t GetObjectRefCount() override
    {
        return 0;
    }

    int SendRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override
    {
        return 0;
    }

    bool IsProxyObject() const override
    {
        return true;
    }

    bool CheckObjectLegality() const override
    {
        return true;
    }

    bool AddDeathRecipient(const sptr<DeathRecipient> &recipient) override
    {
        return true;
    }

    bool RemoveDeathRecipient(const sptr<DeathRecipient> &recipient) override
    {
        return true;
    }

    bool AddRefreshRecipient(const sptr<RefreshRecipient> &recipient) override
    {
        return true;
    }
    bool RemoveRefreshRecipient(const sptr<RefreshRecipient> &recipient) override
    {
        return true;
    }

    bool Marshalling(Parcel &parcel) const override
    {
        return true;
    }

    sptr<IRemoteBroker> AsInterface() override
    {
        return nullptr;
    }

    int Dump(int fd, const std::vector<std::u16string> &args) override
    {
        return 0;
    }

    std::u16string GetObjectDescriptor() const
    {
        std::u16string descriptor = std::u16string();
        return descriptor;
    }
};

class SysMrgClient {
public:
    static SysMrgClient* instance_;

    SysMrgClient* GetInstance()
    {
        return instance_;
    }
};
SysMrgClient* SysMrgClient::instance_ = nullptr;

/**
 * @tc.name: FindRecordFromTmpMap_001
 * @tc.desc: FindRecordFromTmpMap
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerThirdTest, FindRecordFromTmpMap_001, TestSize.Level1)
{
    auto mgr = std::make_unique<UIAbilityLifecycleManager>();
    
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.name = "Name";
    abilityRequest.abilityInfo.bundleName = "BundleName";
    abilityRequest.abilityInfo.moduleName = "ModuleName";
    abilityRequest.abilityInfo.appIndex = 5;
    abilityRequest.want.SetParam(Want::APP_INSTANCE_KEY, std::string("InstanceKey"));
    
    auto callerAbilityRecord = std::make_shared<UIAbilityRecord>(
        abilityRequest.want, abilityRequest.abilityInfo, abilityRequest.appInfo, abilityRequest.requestCode);
    callerAbilityRecord->sessionInfo_ = nullptr;
    callerAbilityRecord->SetAppIndex(5);
    callerAbilityRecord->SetInstanceKey("InstanceKey");

    mgr->tmpAbilityMap_ = {{1, callerAbilityRecord}};

    auto ret = mgr->FindRecordFromTmpMap(abilityRequest);
    EXPECT_NE(ret, nullptr);
}

/**
 * @tc.name: FindRecordFromTmpMap_002
 * @tc.desc: FindRecordFromTmpMap
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerThirdTest, FindRecordFromTmpMap_002, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    
    AbilityRequest abilityRequest;
    int32_t dlpIndex = 1001;
    abilityRequest.want.SetParam(DLP_INDEX, dlpIndex);
    auto ret = mgr->FindRecordFromTmpMap(abilityRequest);
    EXPECT_EQ(ret, nullptr);
}

/**
 * @tc.name: FindRecordFromTmpMap_003
 * @tc.desc: FindRecordFromTmpMap
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerThirdTest, FindRecordFromTmpMap_003, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    
    AbilityRequest abilityRequest;
    mgr->tmpAbilityMap_.emplace(0, nullptr);
    auto ret = mgr->FindRecordFromTmpMap(abilityRequest);
    EXPECT_EQ(ret, nullptr);
}

/**
 * @tc.name: CheckSessionInfo_001
 * @tc.desc: CheckSessionInfo
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerThirdTest, CheckSessionInfo_001, TestSize.Level1)
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

    auto ret = mgr->CheckSessionInfo(currentSessionInfo);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: UIAbilityLifecycleManager_AbilityWindowConfigTransactionDone_0300
 * @tc.desc: AbilityWindowConfigTransactionDone
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerThirdTest, AbilityWindowConfigTransactionDone_003, TestSize.Level1)
{
    auto mgr = std::make_unique<UIAbilityLifecycleManager>();

    AbilityRequest abilityRequest;
    auto abilityRecord = std::make_shared<UIAbilityRecord>(
        abilityRequest.want, abilityRequest.abilityInfo, abilityRequest.appInfo, abilityRequest.requestCode);

    auto token = sptr<Token>::MakeSptr(abilityRecord);
    abilityRecord->token_ = token;

    mgr->terminateAbilityList_ = { abilityRecord };
    
    WindowConfig windowConfig;

    auto ret = mgr->AbilityWindowConfigTransactionDone(token, windowConfig);

    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: UIAbilityLifecycleManager_NotifySCBToStartUIAbility_0300
 * @tc.desc: NotifySCBToStartUIAbility
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerThirdTest, NotifySCBToStartUIAbility_003, TestSize.Level1)
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

    AbilityRequest abilityRequest;
    abilityRequest.want.SetParam(Want::PARAM_RESV_CALLER_UID, 2);

    auto ret = mgr->NotifySCBToStartUIAbility(abilityRequest);

    EXPECT_EQ(ret, ERR_FREQ_START_ABILITY);
}

/**
 * @tc.name: UIAbilityLifecycleManager_NotifySCBToStartUIAbility_004
 * @tc.desc: NotifySCBToStartUIAbility
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerThirdTest, NotifySCBToStartUIAbility_004, TestSize.Level1)
{
    auto mgr = std::make_unique<UIAbilityLifecycleManager>();
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.launchMode = AppExecFwk::LaunchMode::SPECIFIED;

    auto ret = mgr->NotifySCBToStartUIAbility(abilityRequest);

    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: UIAbilityLifecycleManager_PostCallTimeoutTask_001
 * @tc.desc: PostCallTimeoutTask
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerThirdTest, PostCallTimeoutTask_001, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    int32_t requestId = 1;
    mgr->PostCallTimeoutTask(requestId);
    EXPECT_NE(mgr, nullptr);
}

/**
 * @tc.name: UIAbilityLifecycleManager_StartSpecifiedRequest_001
 * @tc.desc: StartSpecifiedRequest
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerThirdTest, StartSpecifiedRequest_001, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    SpecifiedRequest specifiedRequest(0, AbilityRequest());
    mgr->StartSpecifiedRequest(specifiedRequest);
    EXPECT_TRUE(specifiedRequest.isCold);
}

/**
 * @tc.name: UIAbilityLifecycleManager_StartSpecifiedRequest_002
 * @tc.desc: StartSpecifiedRequest
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerThirdTest, StartSpecifiedRequest_002, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    SpecifiedRequest specifiedRequest(0, AbilityRequest());
    specifiedRequest.specifiedProcessState = SpecifiedProcessState::STATE_PROCESS;
    mgr->StartSpecifiedRequest(specifiedRequest);
    EXPECT_FALSE(specifiedRequest.isCold);
}

/**
 * @tc.name: UIAbilityLifecycleManager_StartSpecifiedRequest_003
 * @tc.desc: StartSpecifiedRequest
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerThirdTest, StartSpecifiedRequest_003, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    SpecifiedRequest specifiedRequest(0, AbilityRequest());
    specifiedRequest.specifiedProcessState = SpecifiedProcessState::STATE_ABILITY;
    mgr->StartSpecifiedRequest(specifiedRequest);
    EXPECT_TRUE(specifiedRequest.isCold);
}

/**
 * @tc.name: UIAbilityLifecycleManager_StartSpecifiedRequest_004
 * @tc.desc: StartSpecifiedRequest
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerThirdTest, StartSpecifiedRequest_004, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    SpecifiedRequest specifiedRequest(0, AbilityRequest());
    specifiedRequest.preCreateProcessName = true;
    mgr->StartSpecifiedRequest(specifiedRequest);
    EXPECT_TRUE(specifiedRequest.isCold);
}

/**
 * @tc.name: UIAbilityLifecycleManager_StartSpecifiedRequest_005
 * @tc.desc: StartSpecifiedRequest
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerThirdTest, StartSpecifiedRequest_005, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    SpecifiedRequest specifiedRequest(0, AbilityRequest());
    auto originAppMgr = AppMgrUtil::appMgr_;
    auto appmgr = sptr<AppExecFwk::MockAppMgrService>::MakeSptr();
    EXPECT_CALL(*appmgr, IsSpecifiedModuleLoaded)
        .WillOnce([](const Want &, const AppExecFwk::AbilityInfo &, bool &result, bool &) {
            result = true;
            return 0;
        });
    AppMgrUtil::appMgr_ = appmgr;
    mgr->StartSpecifiedRequest(specifiedRequest);
    EXPECT_FALSE(specifiedRequest.isCold);
    AppMgrUtil::appMgr_ = originAppMgr;
}

/**
 * @tc.name: UIAbilityLifecycleManager_StartSpecifiedRequest_006
 * @tc.desc: StartSpecifiedRequest
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerThirdTest, StartSpecifiedRequest_006, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    SpecifiedRequest specifiedRequest(0, AbilityRequest());
    specifiedRequest.abilityRequest.want.SetParam("debugApp", true);
    mgr->StartSpecifiedRequest(specifiedRequest);
    usleep(TIMEOUT_VALUE);
    EXPECT_TRUE(specifiedRequest.isCold);
}

/**
 * @tc.name: UIAbilityLifecycleManager_StartSpecifiedRequest_007
 * @tc.desc: StartSpecifiedRequest
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerThirdTest, StartSpecifiedRequest_007, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    SpecifiedRequest specifiedRequest(0, AbilityRequest());
    specifiedRequest.abilityRequest.want.SetParam("nativeDebug", true);
    mgr->StartSpecifiedRequest(specifiedRequest);
    usleep(TIMEOUT_VALUE);
    EXPECT_TRUE(specifiedRequest.isCold);
}

/**
 * @tc.name: UIAbilityLifecycleManager_StartSpecifiedRequest_008
 * @tc.desc: StartSpecifiedRequest
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerThirdTest, StartSpecifiedRequest_008, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    SpecifiedRequest specifiedRequest(0, AbilityRequest());
    specifiedRequest.abilityRequest.want.SetParam("perfCmd", std::string("perfCmd"));
    mgr->StartSpecifiedRequest(specifiedRequest);
    usleep(TIMEOUT_VALUE);
    EXPECT_TRUE(specifiedRequest.isCold);
}

/**
 * @tc.name: UIAbilityLifecycleManager_StartSpecifiedRequest_009
 * @tc.desc: StartSpecifiedRequest
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerThirdTest, StartSpecifiedRequest_009, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    SpecifiedRequest specifiedRequest(0, AbilityRequest());
    auto originAppMgr = AppMgrUtil::appMgr_;
    auto appmgr = sptr<AppExecFwk::MockAppMgrService>::MakeSptr();
    AppMgrUtil::appMgr_ = appmgr;
    EXPECT_CALL(*appmgr, IsSpecifiedModuleLoaded)
        .WillOnce([](const Want &, const AppExecFwk::AbilityInfo &, bool &result, bool &isDebug) {
            result = false;
            isDebug = true;
            return 0;
        });
    mgr->StartSpecifiedRequest(specifiedRequest);
    usleep(TIMEOUT_VALUE);
    EXPECT_TRUE(specifiedRequest.isCold);
    AppMgrUtil::appMgr_ = originAppMgr;
}

/**
 * @tc.name: UIAbilityLifecycleManager_DispatchBackground_0200
 * @tc.desc: DispatchBackground
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerThirdTest, DispatchBackground_002, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    
    auto taskHandler = TaskHandlerWrap::CreateQueueHandler("HelloWorld");
    DelayedSingleton<AbilityManagerService>::GetInstance()->taskHandler_ = taskHandler;

    AbilityRequest abilityRequest;
    auto abilityRecord = std::make_shared<UIAbilityRecord>(
        abilityRequest.want, abilityRequest.abilityInfo, abilityRequest.appInfo, abilityRequest.requestCode);
    abilityRecord->currentState_ = AbilityState::BACKGROUNDING;

    auto ret = mgr->DispatchBackground(abilityRecord);

    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: UIAbilityLifecycleManager_UpdateProcessName_0200
 * @tc.desc: UpdateProcessName
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerThirdTest, UpdateProcessName_002, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    sptr<SessionInfo> sessionInfo = new (std::nothrow) SessionInfo();
    sessionInfo->processOptions = std::make_shared<ProcessOptions>();
    EXPECT_NE(sessionInfo->processOptions, nullptr);
    sessionInfo->processOptions->processMode = ProcessMode::NEW_PROCESS_ATTACH_TO_PARENT;
    sessionInfo->processOptions->processName = "HeavenlyMe";
    AbilityRequest abilityRequest;
    abilityRequest.sessionInfo = sessionInfo;
    abilityRequest.abilityInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.moduleName = "entry";
    abilityRequest.abilityInfo.name = "MainAbility";
    auto abilityRecord = InitAbilityRecord();
    uiAbilityLifecycleManager->UpdateProcessName(abilityRequest, abilityRecord);
    EXPECT_EQ("HeavenlyMe", abilityRecord->GetProcessName());
}

/**
 * @tc.name: UIAbilityLifecycleManager_UpdateAbilityRecordLaunchReason_0500
 * @tc.desc: UpdateAbilityRecordLaunchReason
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerThirdTest, UpdateAbilityRecordLaunchReason_005, TestSize.Level1)
{
    auto mgr = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(mgr, nullptr);

    AbilityRequest abilityRequest;
    abilityRequest.want.SetParam(Want::PARM_LAUNCH_REASON_MESSAGE, std::string("HeavenlyMe"));
    auto abilityRecord = std::make_shared<UIAbilityRecord>(
        abilityRequest.want, abilityRequest.abilityInfo, abilityRequest.appInfo, abilityRequest.requestCode);
    abilityRecord->lifeCycleStateInfo_.launchParam.launchReasonMessage = "HelloWorld";

    mgr->UpdateAbilityRecordLaunchReason(abilityRequest, abilityRecord);
    auto message = abilityRecord->lifeCycleStateInfo_.launchParam.launchReasonMessage;
    EXPECT_EQ(message, "HeavenlyMe");
}

/**
 * @tc.name: UIAbilityLifecycleManager_UpdateAbilityRecordLaunchReason_0600
 * @tc.desc: UpdateAbilityRecordLaunchReason
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerThirdTest, UpdateAbilityRecordLaunchReason_006, TestSize.Level1)
{
    auto mgr = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(mgr, nullptr);
    Want want;
    want.SetParam(Want::PARAM_ABILITY_RECOVERY_RESTART, true);

    AbilityRequest abilityRequest;
    abilityRequest.want.SetParam(Want::PARM_LAUNCH_REASON_MESSAGE, std::string(""));
    auto abilityRecord = std::make_shared<UIAbilityRecord>(
        abilityRequest.want, abilityRequest.abilityInfo, abilityRequest.appInfo, abilityRequest.requestCode);
    abilityRecord->lifeCycleStateInfo_.launchParam.launchReasonMessage = "HelloWorld";

    mgr->UpdateAbilityRecordLaunchReason(abilityRequest, abilityRecord);
    auto message = abilityRecord->lifeCycleStateInfo_.launchParam.launchReasonMessage;
    EXPECT_NE(message, "");
}

/**
 * @tc.name: UIAbilityLifecycleManager_MinimizeUIAbility_0300
 * @tc.desc: MinimizeUIAbility
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerThirdTest, MinimizeUIAbility_003, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    
    AbilityRequest abilityRequest;
    auto abilityRecord = std::make_shared<UIAbilityRecord>(
        abilityRequest.want, abilityRequest.abilityInfo, abilityRequest.appInfo, abilityRequest.requestCode);
    abilityRecord->SetPendingState(AbilityState::FOREGROUND);

    EXPECT_EQ(uiAbilityLifecycleManager->MinimizeUIAbility(abilityRecord, false, 0), ERR_OK);
}

/**
 * @tc.name: UIAbilityLifecycleManager_GetPersistentIdByAbilityRequest_0300
 * @tc.desc: GetPersistentIdByAbilityRequest
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerThirdTest, GetPersistentIdByAbilityRequest_003, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest abilityRequest;
    abilityRequest.collaboratorType = CollaboratorType::OTHERS_TYPE;
    uiAbilityLifecycleManager->sessionAbilityMap_.clear();
    bool reuse = false;
    EXPECT_EQ(uiAbilityLifecycleManager->GetPersistentIdByAbilityRequest(abilityRequest, reuse), 0);
}

/**
 * @tc.name: UIAbilityLifecycleManager_OnStartSpecifiedFailed_0200
 * @tc.desc: OnStartSpecifiedFailed
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerThirdTest, OnStartSpecifiedFailed_002, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    int32_t requestId = 1;
    AbilityRequest abilityRequest;
    auto abilityRecord = std::make_shared<UIAbilityRecord>(
        abilityRequest.want, abilityRequest.abilityInfo, abilityRequest.appInfo, abilityRequest.requestCode);

    uiAbilityLifecycleManager->hookSpecifiedMap_ = {
        {requestId, abilityRecord}
    };

    uiAbilityLifecycleManager->OnStartSpecifiedFailed(requestId);
    EXPECT_EQ(uiAbilityLifecycleManager->hookSpecifiedMap_.size(), 0);
}

/**
 * @tc.name: UIAbilityLifecycleManager_OnStartSpecifiedFailed_0300
 * @tc.desc: OnStartSpecifiedFailed
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerThirdTest, OnStartSpecifiedFailed_003, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    int32_t requestId = 1;
    AbilityRequest abilityRequest;
    auto specifiedRequest = std::make_shared<SpecifiedRequest>(requestId, abilityRequest);
    specifiedRequest->persistentId = 1;
    auto &list = uiAbilityLifecycleManager->specifiedRequestList_[std::string()];
    list.push_back(specifiedRequest);

    int32_t requestId_2 = 2;
    list.push_back(std::make_shared<SpecifiedRequest>(requestId_2, abilityRequest));

    auto abilityRecord = std::make_shared<UIAbilityRecord>(
        abilityRequest.want, abilityRequest.abilityInfo, abilityRequest.appInfo, abilityRequest.requestCode);
    uiAbilityLifecycleManager->sessionAbilityMap_.emplace(specifiedRequest->persistentId, abilityRecord);

    uiAbilityLifecycleManager->OnStartSpecifiedFailed(requestId);
    EXPECT_FALSE(list.empty());
}

/**
 * @tc.name: UIAbilityLifecycleManager_OnStartSpecifiedProcessResponse_0300
 * @tc.desc: OnStartSpecifiedProcessResponse
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerThirdTest, OnStartSpecifiedProcessResponse_003, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);

    int32_t requestId = 10;
    AbilityRequest abilityRequest;
    auto abilityRecord = std::make_shared<UIAbilityRecord>(
        abilityRequest.want, abilityRequest.abilityInfo, abilityRequest.appInfo, abilityRequest.requestCode);

    abilityRequest.abilityInfo.launchMode == AppExecFwk::LaunchMode::SPECIFIED;
    auto specifiedRequestPtr = std::make_shared<SpecifiedRequest>(requestId, abilityRequest);
    specifiedRequestPtr->specifiedProcessState = SpecifiedProcessState::STATE_PROCESS;
    uiAbilityLifecycleManager->specifiedRequestList_ = {
        { "NewKawasaki", { specifiedRequestPtr } }
    };

    uiAbilityLifecycleManager->OnStartSpecifiedProcessResponse("HeavenlyMe", requestId);

    auto listMap = uiAbilityLifecycleManager->specifiedRequestList_.find("NewKawasaki");
    EXPECT_EQ(listMap, uiAbilityLifecycleManager->specifiedRequestList_.end());
}

/**
 * @tc.name: UIAbilityLifecycleManager_OnStartSpecifiedProcessResponse_0400
 * @tc.desc: OnStartSpecifiedProcessResponse
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerThirdTest, OnStartSpecifiedProcessResponse_004, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);

    int32_t requestId = 10;
    AbilityRequest abilityRequest;
    auto abilityRecord = std::make_shared<UIAbilityRecord>(
        abilityRequest.want, abilityRequest.abilityInfo, abilityRequest.appInfo, abilityRequest.requestCode);

    abilityRequest.abilityInfo.launchMode = AppExecFwk::LaunchMode::SINGLETON;
    auto specifiedRequestPtr = std::make_shared<SpecifiedRequest>(requestId, abilityRequest);
    specifiedRequestPtr->specifiedProcessState = SpecifiedProcessState::STATE_PROCESS;
    uiAbilityLifecycleManager->specifiedRequestList_ = {
        {"NewKawasaki", {specifiedRequestPtr}}
    };

    uiAbilityLifecycleManager->OnStartSpecifiedProcessResponse("HeavenlyMe", requestId);

    auto listMap = uiAbilityLifecycleManager->specifiedRequestList_.find("NewKawasaki");
    EXPECT_EQ(listMap, uiAbilityLifecycleManager->specifiedRequestList_.end());
}

/**
 * @tc.name: UIAbilityLifecycleManager_IsSupportStatusBar_0400
 * @tc.desc: IsSupportStatusBar
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerThirdTest, IsSupportStatusBar_004, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    
    uiAbilityLifecycleManager->statusBarDelegateManager_ = uiAbilityLifecycleManager->GetStatusBarDelegateManager();
    auto delegate = sptr<MockIStatusBarDelegate>::MakeSptr();
    uiAbilityLifecycleManager->statusBarDelegateManager_->RegisterStatusBarDelegate(delegate);

    auto ret = uiAbilityLifecycleManager->IsSupportStatusBar();

    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: UIAbilityLifecycleManager_OnAcceptWantResponse_0100
 * @tc.desc: OnAcceptWantResponse
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerThirdTest, OnAcceptWantResponse_001, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    
    AAFwk::Want want;
    std::string flag = "";
    int32_t requestId = 1;

    AbilityRequest abilityRequest;
    auto abilityRecord = std::make_shared<UIAbilityRecord>(
        abilityRequest.want, abilityRequest.abilityInfo, abilityRequest.appInfo, abilityRequest.requestCode);
    abilityRecord->sessionInfo_ = sptr<AAFwk::SessionInfo>::MakeSptr();

    uiAbilityLifecycleManager->hookSpecifiedMap_ = {
        {requestId, abilityRecord}
    };

    uiAbilityLifecycleManager->OnAcceptWantResponse(want, flag, requestId);

    auto size = uiAbilityLifecycleManager->hookSpecifiedMap_.size();
    EXPECT_EQ(size, 0);
}

/**
 * @tc.name: UIAbilityLifecycleManager_OnAcceptWantResponse_0200
 * @tc.desc: OnAcceptWantResponse
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerThirdTest, OnAcceptWantResponse_002, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    
    AAFwk::Want want;
    std::string flag = "";
    int32_t requestId = 1;

    AbilityRequest abilityRequest;
    auto abilityRecord = std::make_shared<UIAbilityRecord>(
        abilityRequest.want, abilityRequest.abilityInfo, abilityRequest.appInfo, abilityRequest.requestCode);

    uiAbilityLifecycleManager->hookSpecifiedMap_ = {
        {requestId, abilityRecord}
    };

    uiAbilityLifecycleManager->OnAcceptWantResponse(want, flag, 2);

    auto size = uiAbilityLifecycleManager->hookSpecifiedMap_.size();
    EXPECT_EQ(size, 1);
}

/**
 * @tc.name: UIAbilityLifecycleManager_MoveAbilityToFront_0100
 * @tc.desc: MoveAbilityToFront
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerThirdTest, MoveAbilityToFront_001, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);

    AbilityRequest abilityRequest;
    abilityRequest.processOptions = std::make_shared<ProcessOptions>();
    auto abilityRecord = std::make_shared<UIAbilityRecord>(
        abilityRequest.want, abilityRequest.abilityInfo, abilityRequest.appInfo, abilityRequest.requestCode);
    abilityRecord->sessionInfo_ = sptr<AAFwk::SessionInfo>::MakeSptr();
    abilityRequest.abilityInfo.launchMode = AppExecFwk::LaunchMode::SPECIFIED;

    AppUtils::isStartOptionsWithAnimation_ = true;
    SpecifiedRequest specifiedRequest(0, abilityRequest);
    uiAbilityLifecycleManager->MoveAbilityToFront(specifiedRequest, abilityRecord, abilityRecord);

    auto sessionInfo = abilityRecord->GetSessionInfo();
    EXPECT_EQ(sessionInfo->processOptions, nullptr);
}

/**
 * @tc.name: UIAbilityLifecycleManager_MoveAbilityToFront_0200
 * @tc.desc: MoveAbilityToFront
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerThirdTest, MoveAbilityToFront_002, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);

    AbilityRequest abilityRequest;
    abilityRequest.processOptions = std::make_shared<ProcessOptions>();
    auto abilityRecord = std::make_shared<UIAbilityRecord>(
        abilityRequest.want, abilityRequest.abilityInfo, abilityRequest.appInfo, abilityRequest.requestCode);
    abilityRecord->sessionInfo_ = sptr<AAFwk::SessionInfo>::MakeSptr();
    abilityRequest.abilityInfo.launchMode = AppExecFwk::LaunchMode::SPECIFIED;

    AppUtils::isStartOptionsWithAnimation_ = false;
    SpecifiedRequest specifiedRequest(0, abilityRequest);
    uiAbilityLifecycleManager->MoveAbilityToFront(specifiedRequest, abilityRecord, abilityRecord);

    auto sessionInfo = abilityRecord->GetSessionInfo();
    EXPECT_EQ(sessionInfo->processOptions, nullptr);
}

/**
 * @tc.name: UIAbilityLifecycleManager_MoveAbilityToFront_0300
 * @tc.desc: MoveAbilityToFront
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerThirdTest, MoveAbilityToFront_003, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);

    AbilityRequest abilityRequest;
    abilityRequest.processOptions = std::make_shared<ProcessOptions>();
    abilityRequest.abilityInfo.launchMode = AppExecFwk::LaunchMode::SPECIFIED;
    auto abilityRecord = std::make_shared<UIAbilityRecord>(
        abilityRequest.want, abilityRequest.abilityInfo, abilityRequest.appInfo, abilityRequest.requestCode);
    abilityRecord->sessionInfo_ = sptr<AAFwk::SessionInfo>::MakeSptr();
    abilityRecord->want_.SetParam(Want::PARAM_RESV_WINDOW_MODE,
        AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_UNDEFINED);

    AppUtils::isStartOptionsWithAnimation_ = false;

    SpecifiedRequest specifiedRequest(0, abilityRequest);
    specifiedRequest.requestListId = 0;
    uiAbilityLifecycleManager->MoveAbilityToFront(specifiedRequest, abilityRecord, abilityRecord);

    auto ret = abilityRecord->want_.GetIntParam(Want::PARAM_RESV_WINDOW_MODE, -1);
    EXPECT_EQ(ret, AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_UNDEFINED);
}

/**
 * @tc.name: UIAbilityLifecycleManager_MoveAbilityToFront_0400
 * @tc.desc: MoveAbilityToFront
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerThirdTest, MoveAbilityToFront_004, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);

    AbilityRequest abilityRequest;
    abilityRequest.processOptions = std::make_shared<ProcessOptions>();
    abilityRequest.abilityInfo.launchMode = AppExecFwk::LaunchMode::SPECIFIED;
    auto abilityRecord = std::make_shared<UIAbilityRecord>(
        abilityRequest.want, abilityRequest.abilityInfo, abilityRequest.appInfo, abilityRequest.requestCode);
    abilityRecord->sessionInfo_ = sptr<AAFwk::SessionInfo>::MakeSptr();
    abilityRecord->want_.SetParam(Want::PARAM_RESV_WINDOW_MODE,
        AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_UNDEFINED);

    AppUtils::isStartOptionsWithAnimation_ = false;

    SpecifiedRequest specifiedRequest(0, abilityRequest);
    uiAbilityLifecycleManager->MoveAbilityToFront(specifiedRequest, abilityRecord, abilityRecord);

    auto ret = abilityRecord->want_.GetIntParam(Want::PARAM_RESV_WINDOW_MODE, -1);
    EXPECT_EQ(ret, -1);
}

/**
 * @tc.name: UIAbilityLifecycleManager_CallRequestDone_0100
 * @tc.desc: CallRequestDone
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerThirdTest, CallRequestDone_001, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);

    AbilityRequest abilityRequest;
    auto abilityRecord = std::make_shared<UIAbilityRecord>(
        abilityRequest.want, abilityRequest.abilityInfo, abilityRequest.appInfo, abilityRequest.requestCode);

    auto mockCallStub = sptr<MockIRemoteObject>::MakeSptr();

    uiAbilityLifecycleManager->callRequestCache_ = {
        {abilityRecord, {abilityRequest}}
    };

    uiAbilityLifecycleManager->CallRequestDone(abilityRecord, mockCallStub);

    EXPECT_EQ(uiAbilityLifecycleManager->callRequestCache_.size(), 0);
}

/**
 * @tc.name: UIAbilityLifecycleManager_CallRequestDone_0200
 * @tc.desc: CallRequestDone
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerThirdTest, CallRequestDone_002, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);

    AbilityRequest abilityRequest;
    auto abilityRecord1 = std::make_shared<UIAbilityRecord>(
        abilityRequest.want, abilityRequest.abilityInfo, abilityRequest.appInfo, abilityRequest.requestCode);
    
    auto abilityRecord2 = std::make_shared<UIAbilityRecord>(
        abilityRequest.want, abilityRequest.abilityInfo, abilityRequest.appInfo, abilityRequest.requestCode);


    auto mockCallStub = sptr<MockIRemoteObject>::MakeSptr();

    uiAbilityLifecycleManager->callRequestCache_ = {
        {abilityRecord1, {abilityRequest}}
    };

    uiAbilityLifecycleManager->CallRequestDone(abilityRecord2, mockCallStub);

    EXPECT_NE(uiAbilityLifecycleManager->callRequestCache_.size(), 0);
}

/**
 * @tc.name: UIAbilityLifecycleManager_MoveMissionToFront_0100
 * @tc.desc: MoveMissionToFront
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerThirdTest, MoveMissionToFront_001, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);

    int32_t sessionId = 1;
    auto startOptions = std::make_shared<StartOptions>();

    startOptions->windowMode_ = AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_PRIMARY;

    Rosen::SessionInfo info;
    sptr<Rosen::ISession> session = sptr<Rosen::Session>::MakeSptr(info);
    EXPECT_NE(session, nullptr);
    sptr<IRemoteObject> rootSceneSession = session->AsObject();
    uiAbilityLifecycleManager->rootSceneSession_ = rootSceneSession;

    AbilityRequest abilityRequest;
    auto abilityRecord = std::make_shared<UIAbilityRecord>(
        abilityRequest.want, abilityRequest.abilityInfo, abilityRequest.appInfo, abilityRequest.requestCode);
    abilityRecord->sessionInfo_ = sptr<AAFwk::SessionInfo>::MakeSptr();

    uiAbilityLifecycleManager->sessionAbilityMap_ = {
        {sessionId, abilityRecord}
    };

    uiAbilityLifecycleManager->MoveMissionToFront(sessionId, startOptions);
    
    auto ret = abilityRecord->want_.GetIntParam(Want::PARAM_RESV_WINDOW_MODE, -1);
    
    EXPECT_EQ(ret, AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_PRIMARY);
}

/**
 * @tc.name: UIAbilityLifecycleManager_MoveMissionToFront_0200
 * @tc.desc: MoveMissionToFront
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerThirdTest, MoveMissionToFront_002, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);

    int32_t sessionId = 1;

    Rosen::SessionInfo info;
    sptr<Rosen::ISession> session = sptr<Rosen::Session>::MakeSptr(info);
    EXPECT_NE(session, nullptr);
    sptr<IRemoteObject> rootSceneSession = session->AsObject();
    uiAbilityLifecycleManager->rootSceneSession_ = rootSceneSession;

    AbilityRequest abilityRequest;
    auto abilityRecord = std::make_shared<UIAbilityRecord>(
        abilityRequest.want, abilityRequest.abilityInfo, abilityRequest.appInfo, abilityRequest.requestCode);
    abilityRecord->sessionInfo_ = sptr<AAFwk::SessionInfo>::MakeSptr();

    uiAbilityLifecycleManager->sessionAbilityMap_ = {
        {sessionId, abilityRecord}
    };

    uiAbilityLifecycleManager->MoveMissionToFront(sessionId, nullptr);
    
    auto ret = abilityRecord->want_.GetIntParam(Want::PARAM_RESV_WINDOW_MODE, -1);
    
    EXPECT_EQ(ret, -1);
}

/**
 * @tc.name: UIAbilityLifecycleManager_MoveMissionToFront_0300
 * @tc.desc: MoveMissionToFront
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerThirdTest, MoveMissionToFront_003, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);

    int32_t sessionId = 1;
    auto startOptions = std::make_shared<StartOptions>();

    startOptions->windowMode_ = AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_PRIMARY;
    startOptions->displayId_ = 0;

    Rosen::SessionInfo info;
    sptr<Rosen::ISession> session = sptr<Rosen::Session>::MakeSptr(info);
    EXPECT_NE(session, nullptr);
    sptr<IRemoteObject> rootSceneSession = session->AsObject();
    uiAbilityLifecycleManager->rootSceneSession_ = rootSceneSession;

    AbilityRequest abilityRequest;
    auto abilityRecord = std::make_shared<UIAbilityRecord>(
        abilityRequest.want, abilityRequest.abilityInfo, abilityRequest.appInfo, abilityRequest.requestCode);
    abilityRecord->sessionInfo_ = sptr<AAFwk::SessionInfo>::MakeSptr();
    abilityRecord->sessionInfo_->want.RemoveParam(Want::PARAM_RESV_DISPLAY_ID);

    uiAbilityLifecycleManager->sessionAbilityMap_ = {
        {sessionId, abilityRecord}
    };

    uiAbilityLifecycleManager->MoveMissionToFront(sessionId, startOptions);
    
    auto ret = abilityRecord->sessionInfo_->want.GetIntParam(Want::PARAM_RESV_DISPLAY_ID, -1);
    
    EXPECT_EQ(ret, DisplayUtil::GetDefaultDisplayId());
}

/**
 * @tc.name: UIAbilityLifecycleManager_MoveMissionToFront_0400
 * @tc.desc: MoveMissionToFront
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerThirdTest, MoveMissionToFront_004, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);

    int32_t sessionId = 1;
    auto startOptions = std::make_shared<StartOptions>();

    startOptions->windowMode_ = AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_PRIMARY;
    startOptions->displayId_ = 1;

    Rosen::SessionInfo info;
    sptr<Rosen::ISession> session = sptr<Rosen::Session>::MakeSptr(info);
    EXPECT_NE(session, nullptr);
    sptr<IRemoteObject> rootSceneSession = session->AsObject();
    uiAbilityLifecycleManager->rootSceneSession_ = rootSceneSession;

    AbilityRequest abilityRequest;
    auto abilityRecord = std::make_shared<UIAbilityRecord>(
        abilityRequest.want, abilityRequest.abilityInfo, abilityRequest.appInfo, abilityRequest.requestCode);
    abilityRecord->sessionInfo_ = sptr<AAFwk::SessionInfo>::MakeSptr();
    abilityRecord->sessionInfo_->want.RemoveParam(Want::PARAM_RESV_DISPLAY_ID);

    uiAbilityLifecycleManager->sessionAbilityMap_ = {
        {sessionId, abilityRecord}
    };

    uiAbilityLifecycleManager->MoveMissionToFront(sessionId, startOptions);
    
    auto ret = abilityRecord->sessionInfo_->want.GetIntParam(Want::PARAM_RESV_DISPLAY_ID, -1);
    
    EXPECT_EQ(ret, 1);
    EXPECT_NE(ret, DisplayUtil::GetDefaultDisplayId());
}

/**
 * @tc.name: UIAbilityLifecycleManager_CheckPrepareTerminateTokens_0100
 * @tc.desc: CheckPrepareTerminateTokens
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerThirdTest, CheckPrepareTerminateTokens_001, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);

    AppUtils::isStartOptionsWithAnimation_ = true;
    std::vector<sptr<IRemoteObject>> tokens = {};
    uint32_t tokenId = 0;
    std::map<std::string, std::vector<sptr<IRemoteObject>>> tokensPerModuleName = {};

    auto ret = uiAbilityLifecycleManager->CheckPrepareTerminateTokens(tokens, tokenId, tokensPerModuleName);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: UIAbilityLifecycleManager_UpdateSessionInfoBySCB_0100
 * @tc.desc: UpdateSessionInfoBySCB
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerThirdTest, UpdateSessionInfoBySCB_001, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);

    SessionInfo sessionInfo;
    sessionInfo.persistentId = 1;
    std::list<SessionInfo> sessionInfos = {
        sessionInfo
    };
    std::vector<int32_t> sessionIds = {};

    auto ret = uiAbilityLifecycleManager->UpdateSessionInfoBySCB(sessionInfos, sessionIds);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_NE(sessionIds.size(), 0);
    EXPECT_EQ(*sessionIds.rbegin(), 1);
}

/**
 * @tc.name: UIAbilityLifecycleManager_UpdateSessionInfoBySCB_0200
 * @tc.desc: UpdateSessionInfoBySCB
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerThirdTest, UpdateSessionInfoBySCB_002, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);

    SessionInfo sessionInfo;
    sessionInfo.persistentId = 1;
    std::list<SessionInfo> sessionInfos = {
        sessionInfo
    };
    std::vector<int32_t> sessionIds = {};

    AbilityRequest abilityRequest;
    auto abilityRecord = std::make_shared<UIAbilityRecord>(
        abilityRequest.want, abilityRequest.abilityInfo, abilityRequest.appInfo, abilityRequest.requestCode);
    abilityRecord->sessionInfo_ = sptr<AAFwk::SessionInfo>::MakeSptr();
    uiAbilityLifecycleManager->sessionAbilityMap_ = {
        {1, abilityRecord}
    };

    auto ret = uiAbilityLifecycleManager->UpdateSessionInfoBySCB(sessionInfos, sessionIds);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(sessionIds.size(), 0);
}

/**
 * @tc.name: UIAbilityLifecycleManager_AddSpecifiedRequest_0200
 * @tc.desc: AddSpecifiedRequest
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerThirdTest, AddSpecifiedRequest_002, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);

    uiAbilityLifecycleManager->specifiedRequestList_.clear();

    uiAbilityLifecycleManager->AddSpecifiedRequest(nullptr);

    EXPECT_EQ(uiAbilityLifecycleManager->specifiedRequestList_.size(), 0);
}

/**
 * @tc.name: UIAbilityLifecycleManager_PopAndGetNextSpecified_0100
 * @tc.desc: PopAndGetNextSpecified
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerThirdTest, PopAndGetNextSpecified_001, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);

    int32_t requestId1 = 1;
    int32_t requestId2 = 2;

    AbilityRequest abilityRequest;
    auto abilityRecord = std::make_shared<UIAbilityRecord>(
        abilityRequest.want, abilityRequest.abilityInfo, abilityRequest.appInfo, abilityRequest.requestCode);

    auto specifiedRequestPtr = std::make_shared<SpecifiedRequest>(requestId1, abilityRequest);
    specifiedRequestPtr->requestId = 1;
    auto specifiedRequest2Ptr = std::make_shared<SpecifiedRequest>(requestId2, abilityRequest);
    specifiedRequest2Ptr->requestId = 2;

    uiAbilityLifecycleManager->specifiedRequestList_ = {
        {"NewKawasaki", {specifiedRequestPtr, specifiedRequest2Ptr}}
    };

    auto ret = uiAbilityLifecycleManager->PopAndGetNextSpecified(1);

    EXPECT_EQ(ret, specifiedRequest2Ptr);
}

/**
 * @tc.name: UIAbilityLifecycleManager_IsSpecifiedModuleLoaded_0100
 * @tc.desc: IsSpecifiedModuleLoaded
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerThirdTest, IsSpecifiedModuleLoaded_001, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);

    AbilityRequest abilityRequest;

    AppMgrUtil::appMgr_ = nullptr;
    SysMrgClient::instance_ = nullptr;
    bool isDebug = false;
    auto ret = uiAbilityLifecycleManager->IsSpecifiedModuleLoaded(abilityRequest, false, isDebug);

    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: UIAbilityLifecycleManager_HandleColdAcceptWantDone_0100
 * @tc.desc: HandleColdAcceptWantDone
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerThirdTest, HandleColdAcceptWantDone_001, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);

    AAFwk::Want want;

    int32_t requestId1 = 1;
    AbilityRequest abilityRequest;
    auto abilityRecord = std::make_shared<UIAbilityRecord>(
        abilityRequest.want, abilityRequest.abilityInfo, abilityRequest.appInfo, abilityRequest.requestCode);

    auto specifiedRequestPtr = std::make_shared<SpecifiedRequest>(requestId1, abilityRequest);

    uiAbilityLifecycleManager->sessionAbilityMap_.clear();

    auto ret = uiAbilityLifecycleManager->HandleColdAcceptWantDone(want, "", *specifiedRequestPtr);

    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: UIAbilityLifecycleManager_HandleColdAcceptWantDone_0200
 * @tc.desc: HandleColdAcceptWantDone
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerThirdTest, HandleColdAcceptWantDone_002, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);

    AAFwk::Want want;

    int32_t requestId1 = 1;
    AbilityRequest abilityRequest;

    auto specifiedRequestPtr = std::make_shared<SpecifiedRequest>(requestId1, abilityRequest);
    specifiedRequestPtr->persistentId = 1;

    uiAbilityLifecycleManager->sessionAbilityMap_ = {
        {1, nullptr}
    };

    auto ret = uiAbilityLifecycleManager->HandleColdAcceptWantDone(want, "", *specifiedRequestPtr);

    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: UIAbilityLifecycleManager_HandleColdAcceptWantDone_0300
 * @tc.desc: HandleColdAcceptWantDone
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerThirdTest, HandleColdAcceptWantDone_003, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);

    AAFwk::Want want;

    int32_t requestId1 = 1;
    AbilityRequest abilityRequest;
    auto abilityRecord = std::make_shared<UIAbilityRecord>(
        abilityRequest.want, abilityRequest.abilityInfo, abilityRequest.appInfo, abilityRequest.requestCode);
    abilityRecord->specifiedFlag_ = "HeavenlyMe";

    auto specifiedRequestPtr = std::make_shared<SpecifiedRequest>(requestId1, abilityRequest);
    specifiedRequestPtr->persistentId = 1;

    uiAbilityLifecycleManager->sessionAbilityMap_ = {
        {1, abilityRecord}
    };

    auto ret = uiAbilityLifecycleManager->HandleColdAcceptWantDone(want, "", *specifiedRequestPtr);

    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: UIAbilityLifecycleManager_HandleColdAcceptWantDone_0400
 * @tc.desc: HandleColdAcceptWantDone
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerThirdTest, HandleColdAcceptWantDone_004, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);

    AAFwk::Want want;

    int32_t requestId1 = 1;
    AbilityRequest abilityRequest;
    auto abilityRecord = std::make_shared<UIAbilityRecord>(
        abilityRequest.want, abilityRequest.abilityInfo, abilityRequest.appInfo, abilityRequest.requestCode);
    abilityRecord->specifiedFlag_.clear();

    auto specifiedRequestPtr = std::make_shared<SpecifiedRequest>(requestId1, abilityRequest);
    specifiedRequestPtr->persistentId = 1;

    uiAbilityLifecycleManager->sessionAbilityMap_ = {
        {1, abilityRecord}
    };

    auto ret = uiAbilityLifecycleManager->HandleColdAcceptWantDone(want, "", *specifiedRequestPtr);

    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: UIAbilityLifecycleManager_StartSpecifiedProcessRequest_0100
 * @tc.desc: StartSpecifiedProcessRequest
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerThirdTest, StartSpecifiedProcessRequest_0100, TestSize.Level1)
{
    auto mockSceneSessionManagerLite = new (std::nothrow) Rosen::MockSceneSessionManagerLite();
    Rosen::SessionManagerLite::GetInstance().sceneSessionManagerLiteProxy_ = mockSceneSessionManagerLite;
    EXPECT_CALL(*mockSceneSessionManagerLite, CreateNewInstanceKey(_, _))
        .Times(1)
        .WillOnce(Return(Rosen::WMError(-1)));

    AppUtils::isInOnNewProcessEnableList_ = true;
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.applicationInfo.multiAppMode.multiAppModeType =
        AppExecFwk::MultiAppModeType::MULTI_INSTANCE;
    abilityRequest.want.SetParam(Want::CREATE_APP_INSTANCE_KEY, true);
    auto result = uiAbilityLifecycleManager->StartSpecifiedProcessRequest(abilityRequest, nullptr);
    EXPECT_EQ(result, ERR_CREATE_INSTANCE_KEY_FAILED);
}

/**
 * @tc.name: UIAbilityLifecycleManager_StartSpecifiedProcessRequest_0200
 * @tc.desc: StartSpecifiedProcessRequest
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerThirdTest, StartSpecifiedProcessRequest_0200, TestSize.Level1)
{
    auto mockSceneSessionManagerLite = new (std::nothrow) Rosen::MockSceneSessionManagerLite();
    Rosen::SessionManagerLite::GetInstance().sceneSessionManagerLiteProxy_ = mockSceneSessionManagerLite;
    EXPECT_CALL(*mockSceneSessionManagerLite, CreateNewInstanceKey(_, _))
        .Times(1)
        .WillOnce(Return(Rosen::WMError::WM_OK));

    AppUtils::isInOnNewProcessEnableList_ = true;
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.applicationInfo.multiAppMode.multiAppModeType =
        AppExecFwk::MultiAppModeType::MULTI_INSTANCE;
    abilityRequest.want.SetParam(Want::CREATE_APP_INSTANCE_KEY, true);
    auto result = uiAbilityLifecycleManager->StartSpecifiedProcessRequest(abilityRequest, nullptr);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: UIAbilityLifecycleManager_StartSpecifiedProcessRequest_0300
 * @tc.desc: StartSpecifiedProcessRequest
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerThirdTest, StartSpecifiedProcessRequest_0300, TestSize.Level1)
{
    auto mockSceneSessionManagerLite = new (std::nothrow) Rosen::MockSceneSessionManagerLite();
    Rosen::SessionManagerLite::GetInstance().sceneSessionManagerLiteProxy_ = mockSceneSessionManagerLite;
    EXPECT_CALL(*mockSceneSessionManagerLite, CreateNewInstanceKey(_, _))
        .Times(1)
        .WillOnce(Return(Rosen::WMError::WM_OK));

    AppUtils::isInOnNewProcessEnableList_ = true;
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.applicationInfo.multiAppMode.multiAppModeType =
        AppExecFwk::MultiAppModeType::MULTI_INSTANCE;
    abilityRequest.want.SetParam(Want::CREATE_APP_INSTANCE_KEY, true);
    auto abilitiesRequest = std::make_shared<AbilitiesRequest>();
    EXPECT_NE(abilitiesRequest, nullptr);
    auto result = uiAbilityLifecycleManager->StartSpecifiedProcessRequest(abilityRequest, abilitiesRequest);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: UIAbilityLifecycleManager_NotifyStartupExceptionBySCB_001
 * @tc.desc: NotifyStartupExceptionBySCB
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerThirdTest, NotifyStartupExceptionBySCB_001, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    EXPECT_NE(mgr, nullptr);

    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.name = "Name";
    abilityRequest.abilityInfo.bundleName = "BundleName";
    abilityRequest.abilityInfo.moduleName = "ModuleName";
    
    auto callerAbilityRecord = std::make_shared<UIAbilityRecord>(
        abilityRequest.want, abilityRequest.abilityInfo, abilityRequest.appInfo, abilityRequest.requestCode);
    int32_t requestId = 1;
    mgr->tmpAbilityMap_ = {{requestId, callerAbilityRecord}};

    std::string reason = "SCB intercepted this startup attempt";
    auto ret = mgr->NotifyStartupExceptionBySCB(requestId, reason);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(mgr->tmpAbilityMap_.size(), 0);
}

/**
 * @tc.name: UIAbilityLifecycleManager_NotifyStartupExceptionBySCB_002
 * @tc.desc: NotifyStartupExceptionBySCB
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerThirdTest, NotifyStartupExceptionBySCB_002, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);

    int32_t requestId1 = 1;
    int32_t requestId2 = 2;

    AbilityRequest abilityRequest;
    auto specifiedRequestPtr = std::make_shared<SpecifiedRequest>(requestId1, abilityRequest);
    specifiedRequestPtr->requestId = 1;
    auto specifiedRequest2Ptr = std::make_shared<SpecifiedRequest>(requestId2, abilityRequest);
    specifiedRequest2Ptr->requestId = 2;

    uiAbilityLifecycleManager->specifiedRequestList_ = {
        {"NewKawasaki", {specifiedRequestPtr, specifiedRequest2Ptr}}
    };
    int32_t requestId = 1;
    std::string reason = "SCB intercepted this startup attempt";
    auto ret = uiAbilityLifecycleManager->NotifyStartupExceptionBySCB(requestId, reason);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: StartUIAbility_1200
 * @tc.desc: StartUIAbility
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerThirdTest, StartUIAbility_1200, TestSize.Level1)
{
    auto mgr = std::make_unique<UIAbilityLifecycleManager>();
    AbilityRequest abilityRequest;
    AbilityRuntime::StartParamsBySCB params;
    params.isRestart = true;
    bool isColdStart = false;
    EXPECT_EQ(mgr->StartUIAbility(abilityRequest, nullptr, params, isColdStart), ERR_INVALID_VALUE);
}

/**
 * @tc.name: HandleRestartUIAbility_0100
 * @tc.desc: HandleRestartUIAbility
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerThirdTest, HandleRestartUIAbility_0100, TestSize.Level1)
{
    auto mgr = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_FALSE(mgr->HandleRestartUIAbility(nullptr));

    auto sessionInfo = sptr<SessionInfo>::MakeSptr();
    EXPECT_FALSE(mgr->HandleRestartUIAbility(sessionInfo));

    auto session = sptr<Rosen::Session>::MakeSptr(Rosen::SessionInfo());
    sessionInfo->sessionToken = session->AsObject();
    EXPECT_FALSE(mgr->HandleRestartUIAbility(sessionInfo));

    auto session2 = sptr<Rosen::Session>::MakeSptr(Rosen::SessionInfo());
    sessionInfo->callerSession = session2->AsObject();
    EXPECT_FALSE(mgr->HandleRestartUIAbility(sessionInfo));

    mgr->sessionAbilityMap_.emplace(0, nullptr);
    AbilityRequest abilityRequest;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    mgr->sessionAbilityMap_.emplace(1, abilityRecord);
    auto sessionInfo2 = sptr<SessionInfo>::MakeSptr();
    sessionInfo2->sessionToken = session2->AsObject();
    abilityRequest.sessionInfo = sessionInfo2;
    auto abilityRecord2 = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    mgr->sessionAbilityMap_.emplace(2, abilityRecord2);
    EXPECT_FALSE(mgr->HandleRestartUIAbility(sessionInfo));

    sessionInfo->callerSession = sessionInfo->sessionToken;
    sessionInfo2->sessionToken = sessionInfo->sessionToken;
    EXPECT_FALSE(mgr->HandleRestartUIAbility(sessionInfo));
}

/**
 * @tc.name: SignRestartProcess_001
 * @tc.desc: Put a single ability record into map and sign restart app flag
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(UIAbilityLifecycleManagerThirdTest, SignRestartProcess_001, TestSize.Level2)
{
    auto mgr = std::make_unique<UIAbilityLifecycleManager>();
    auto pid = 100;
    mgr->sessionAbilityMap_.emplace(0, nullptr);
    mgr->SignRestartProcess(pid);
    EXPECT_FALSE(mgr->sessionAbilityMap_.empty());

    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "MainAbility";
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    mgr->sessionAbilityMap_.emplace(pid, abilityRecord);
    mgr->SignRestartProcess(pid);
    EXPECT_FALSE(abilityRecord->GetRestartAppFlag());

    abilityRecord->SetPid(pid);
    mgr->SignRestartProcess(pid);
    EXPECT_TRUE(abilityRecord->GetRestartAppFlag());
}

/**
 * @tc.name: HandleAbilityRecordReused_001
 * @tc.desc: HandleAbilityRecordReused null abilityRecord
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(UIAbilityLifecycleManagerThirdTest, HandleAbilityRecordReused_001, TestSize.Level2)
{
    auto mgr = std::make_unique<UIAbilityLifecycleManager>();
    AbilityRequest abilityRequest;
    SessionInfo sessionInfo;
    EXPECT_EQ(mgr->HandleAbilityRecordReused(nullptr, sessionInfo, abilityRequest), nullptr);
}

/**
 * @tc.name: HandleAbilityRecordReused_002
 * @tc.desc: HandleAbilityRecordReused null sessionInfo
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(UIAbilityLifecycleManagerThirdTest, HandleAbilityRecordReused_002, TestSize.Level2)
{
    auto mgr = std::make_unique<UIAbilityLifecycleManager>();
    AbilityRequest abilityRequest;
    SessionInfo sessionInfo;
    abilityRequest.appInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "MainAbility";
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    abilityRecord->sessionInfo_ = nullptr;
    EXPECT_EQ(mgr->HandleAbilityRecordReused(abilityRecord, sessionInfo, abilityRequest), nullptr);
}

/**
 * @tc.name: HandleAbilityRecordReused_003
 * @tc.desc: HandleAbilityRecordReused sessionToken not same
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(UIAbilityLifecycleManagerThirdTest, HandleAbilityRecordReused_003, TestSize.Level2)
{
    auto mgr = std::make_unique<UIAbilityLifecycleManager>();
    AbilityRequest abilityRequest;
    SessionInfo sessionInfo;
    auto session = sptr<Rosen::Session>::MakeSptr(Rosen::SessionInfo());
    sessionInfo.sessionToken = session->AsObject();
    abilityRequest.appInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "MainAbility";
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    auto sessionInfo2 = sptr<SessionInfo>::MakeSptr();
    auto session2 = sptr<Rosen::Session>::MakeSptr(Rosen::SessionInfo());
    sessionInfo2->sessionToken = session2->AsObject();
    abilityRecord->sessionInfo_ = sessionInfo2;
    EXPECT_EQ(mgr->HandleAbilityRecordReused(abilityRecord, sessionInfo, abilityRequest), nullptr);
}

/**
 * @tc.name: HandleAbilityRecordReused_004
 * @tc.desc: HandleAbilityRecordReused ok
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(UIAbilityLifecycleManagerThirdTest, HandleAbilityRecordReused_004, TestSize.Level2)
{
    auto mgr = std::make_unique<UIAbilityLifecycleManager>();
    AbilityRequest abilityRequest;
    SessionInfo sessionInfo;
    auto session = sptr<Rosen::Session>::MakeSptr(Rosen::SessionInfo());
    sessionInfo.sessionToken = session->AsObject();
    abilityRequest.appInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "MainAbility";
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    auto sessionInfo2 = sptr<SessionInfo>::MakeSptr();
    sessionInfo2->sessionToken = session->AsObject();
    abilityRecord->sessionInfo_ = sessionInfo2;
    sessionInfo.reuseDelegatorWindow = false;

    EXPECT_EQ(mgr->HandleAbilityRecordReused(abilityRecord, sessionInfo, abilityRequest), abilityRecord);
}

/**
 * @tc.name: HandleAbilityRecordReused_005
 * @tc.desc: HandleAbilityRecordReused reuseDelegatorWindow true
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(UIAbilityLifecycleManagerThirdTest, HandleAbilityRecordReused_005, TestSize.Level2)
{
    auto mgr = std::make_unique<UIAbilityLifecycleManager>();
    AbilityRequest abilityRequest;
    SessionInfo sessionInfo;
    auto session = sptr<Rosen::Session>::MakeSptr(Rosen::SessionInfo());
    sessionInfo.sessionToken = session->AsObject();
    abilityRequest.appInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "MainAbility";
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    auto sessionInfo2 = sptr<SessionInfo>::MakeSptr();
    sessionInfo2->sessionToken = session->AsObject();
    abilityRecord->sessionInfo_ = sessionInfo2;

    auto originAppMgr = AppMgrUtil::appMgr_;
    auto appmgr = sptr<AppExecFwk::MockAppMgrService>::MakeSptr();
    EXPECT_CALL(*appmgr, LaunchAbility).Times(1).WillOnce(Return(ERR_OK));
    AppMgrUtil::appMgr_ = appmgr;

    sessionInfo.reuseDelegatorWindow = true;
    EXPECT_EQ(mgr->HandleAbilityRecordReused(abilityRecord, sessionInfo, abilityRequest), abilityRecord);

    EXPECT_CALL(*appmgr, LaunchAbility).Times(1).WillOnce(Return(-1));
    EXPECT_EQ(mgr->HandleAbilityRecordReused(abilityRecord, sessionInfo, abilityRequest), nullptr);

    AppMgrUtil::appMgr_ = originAppMgr;
}

/**
 * @tc.name: HandleAbilityRecordReused_006
 * @tc.desc: HandleAbilityRecordReused with GameSAPrelaunch flag but SetGameSAPrelaunch fails
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerThirdTest, HandleAbilityRecordReused_006, TestSize.Level2)
{
    auto mgr = std::make_unique<UIAbilityLifecycleManager>();
    AbilityRequest abilityRequest;
    SessionInfo sessionInfo;
    auto session = sptr<Rosen::Session>::MakeSptr(Rosen::SessionInfo());
    sessionInfo.sessionToken = session->AsObject();
    sessionInfo.persistentId = 1;
    sessionInfo.reuseDelegatorWindow = false;

    abilityRequest.appInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "MainAbility";
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    abilityRecord->SetGameSAPreLaunch(true);

    auto originAppMgr = AppMgrUtil::appMgr_;
    auto appmgr = sptr<AppExecFwk::MockAppMgrService>::MakeSptr();
    // SetGameSAPrelaunch will fail before reaching LaunchAbility, so LaunchAbility won't be called
    EXPECT_CALL(*appmgr, LaunchAbility).Times(0);
    AppMgrUtil::appMgr_ = appmgr;

    auto result = mgr->HandleAbilityRecordReused(abilityRecord, sessionInfo, abilityRequest);

    EXPECT_EQ(result, nullptr);

    AppMgrUtil::appMgr_ = originAppMgr;
}

/**
 * @tc.name: HandleAbilityRecordReused_007
 * @tc.desc: HandleAbilityRecordReused without GameSAPrelaunch flag
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerThirdTest, HandleAbilityRecordReused_007, TestSize.Level2)
{
    auto mgr = std::make_unique<UIAbilityLifecycleManager>();
    AbilityRequest abilityRequest;
    SessionInfo sessionInfo;
    auto session = sptr<Rosen::Session>::MakeSptr(Rosen::SessionInfo());
    sessionInfo.sessionToken = session->AsObject();
    sessionInfo.persistentId = 1;
    sessionInfo.reuseDelegatorWindow = false;

    abilityRequest.appInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "MainAbility";
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    abilityRecord->SetGameSAPreLaunch(false);

    auto originAppMgr = AppMgrUtil::appMgr_;
    auto appmgr = sptr<AppExecFwk::MockAppMgrService>::MakeSptr();
    // reuseDelegatorWindow = false, so LaunchAbility will not be called
    EXPECT_CALL(*appmgr, LaunchAbility).Times(0);
    AppMgrUtil::appMgr_ = appmgr;

    auto result = mgr->HandleAbilityRecordReused(abilityRecord, sessionInfo, abilityRequest);

    EXPECT_EQ(result, nullptr);
    EXPECT_FALSE(abilityRecord->IsGameSAPreLaunch());

    AppMgrUtil::appMgr_ = originAppMgr;
}

/**
 * @tc.name: SetProcessPrepareExit_001
 * @tc.desc: SetProcessPrepareExit
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(UIAbilityLifecycleManagerThirdTest, SetProcessPrepareExit_001, TestSize.Level2)
{
    auto mgr = std::make_unique<UIAbilityLifecycleManager>();
    auto appMgr = sptr<AppExecFwk::MockAppMgrService>::MakeSptr();

    auto originAppMgr = AppMgrUtil::appMgr_;
    AppMgrUtil::appMgr_ = appMgr;
    EXPECT_CALL(*appMgr, SetProcessPrepareExit).Times(1);
    mgr->SetProcessPrepareExit(0, 1u, 1u, true);
    EXPECT_CALL(*appMgr, SetProcessPrepareExit).Times(0);
    mgr->SetProcessPrepareExit(0, 1u, 0u, true);
    EXPECT_CALL(*appMgr, SetProcessPrepareExit).Times(0);
    mgr->SetProcessPrepareExit(0, 0u, 0u, true);
    EXPECT_CALL(*appMgr, SetProcessPrepareExit).Times(0);
    mgr->SetProcessPrepareExit(0, 0u, 1u, true);
    EXPECT_CALL(*appMgr, SetProcessPrepareExit).Times(0);
    mgr->SetProcessPrepareExit(0, 0u, 0u, false);
    EXPECT_CALL(*appMgr, SetProcessPrepareExit).Times(0);
    mgr->SetProcessPrepareExit(0, 0u, 1u, false);
    EXPECT_CALL(*appMgr, SetProcessPrepareExit).Times(0);
    mgr->SetProcessPrepareExit(0, 1u, 1u, false);
    AppMgrUtil::appMgr_ = originAppMgr;
}

/**
 * @tc.name: SetLastExitReasonAsync_001
 * @tc.desc: SetLastExitReasonAsync
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(UIAbilityLifecycleManagerThirdTest, SetLastExitReasonAsync_001, TestSize.Level2)
{
    auto mgr = std::make_unique<UIAbilityLifecycleManager>();
    mgr->SetLastExitReasonAsync(nullptr);

    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "MainAbility";
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    abilityRecord->exitReasonLoaded_ = false;
    mgr->SetLastExitReasonAsync(abilityRecord);
    EXPECT_TRUE(abilityRecord->exitReasonLoaded_);

    abilityRecord->exitReasonLoaded_ = true;
    mgr->SetLastExitReasonAsync(abilityRecord);
    EXPECT_TRUE(abilityRecord->exitReasonLoaded_);
}

/**
 * @tc.name: SyncLoadExitReasonTask_001
 * @tc.desc: SyncLoadExitReasonTask
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(UIAbilityLifecycleManagerThirdTest, SyncLoadExitReasonTask_001, TestSize.Level2)
{
    auto mgr = std::make_unique<UIAbilityLifecycleManager>();
    mgr->exitReasonTasks_.clear();
    mgr->SyncLoadExitReasonTask(0);

    mgr->exitReasonTasks_.emplace(0, ffrt::task_handle());
    mgr->SyncLoadExitReasonTask(0);

    EXPECT_TRUE(mgr->exitReasonTasks_.empty());
}
}  // namespace AAFwk
}  // namespace OHOS