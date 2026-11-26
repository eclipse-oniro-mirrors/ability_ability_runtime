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

#include <atomic>
#include <thread>

#include "ability_manager_errors.h"
#include "ability_util.h"
#include "ipc_skeleton.h"
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
#include "start_options.h"
#include "start_window_option.h"
#include "wm_animation_common.h"
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
#define private public
#define protected public
#include "app_exit_reason_data_manager.h"
#include "mock_single_kv_store.h"
#undef protected
#undef private

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

class PrelaunchTimeoutConnection : public IRemoteStub<IAbilityConnection> {
public:
    static int disconnectDoneCount;
    PrelaunchTimeoutConnection() = default;
    virtual ~PrelaunchTimeoutConnection() = default;
    virtual int OnRemoteRequest(
        uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option)
    {
        return 0;
    };
    virtual void OnAbilityConnectDone(
        const AppExecFwk::ElementName& element, const sptr<IRemoteObject>& remoteObject, int resultCode) {};
    virtual void OnAbilityDisconnectDone(const AppExecFwk::ElementName& element, int resultCode)
    {
        disconnectDoneCount++;
    };
};
int PrelaunchTimeoutConnection::disconnectDoneCount = 0;

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
namespace {
// Local session capturing the SessionInfo passed by MoveMissionToFront so tests can verify
// that a private clone (instead of the shared sessionInfo owned by the record) is sent to SCB.
class PendingActivationTestSession : public Rosen::Session {
public:
    explicit PendingActivationTestSession(const Rosen::SessionInfo &info) : Rosen::Session(info) {}
    ~PendingActivationTestSession() override = default;

    Rosen::WSError PendingSessionActivation(const sptr<AAFwk::SessionInfo> abilitySessionInfo) override
    {
        lastSessionInfo_ = abilitySessionInfo;
        return Rosen::WSError::WS_OK;
    }

    sptr<AAFwk::SessionInfo> lastSessionInfo_ = nullptr;
};
} // namespace
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
 * @tc.name: UIAbilityLifecycleManager_MoveMissionToFront_0500
 * @tc.desc: MoveMissionToFront mutates the shared sessionInfo under sessionLock_, then sends a
 *           private deep-copied clone to PendingSessionActivation outside the lock
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerThirdTest, MoveMissionToFront_005, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(uiAbilityLifecycleManager, nullptr);

    int32_t sessionId = 1;
    auto startOptions = std::make_shared<StartOptions>();
    startOptions->windowMode_ = AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_PRIMARY;
    startOptions->displayId_ = 5;
    startOptions->windowCreateParams_ = std::make_shared<Rosen::WindowCreateParams>();

    Rosen::SessionInfo info;
    auto testSession = sptr<PendingActivationTestSession>::MakeSptr(info);
    ASSERT_NE(testSession, nullptr);
    uiAbilityLifecycleManager->rootSceneSession_ = testSession->AsObject();

    AbilityRequest abilityRequest;
    auto abilityRecord = std::make_shared<UIAbilityRecord>(
        abilityRequest.want, abilityRequest.abilityInfo, abilityRequest.appInfo, abilityRequest.requestCode);
    auto sessionInfo = sptr<AAFwk::SessionInfo>::MakeSptr();
    auto processOptions = std::make_shared<ProcessOptions>();
    sessionInfo->processOptions = processOptions;
    sessionInfo->requestId = 777;
    abilityRecord->sessionInfo_ = sessionInfo;

    uiAbilityLifecycleManager->sessionAbilityMap_ = {
        {sessionId, abilityRecord}
    };

    EXPECT_EQ(uiAbilityLifecycleManager->MoveMissionToFront(sessionId, startOptions), ERR_OK);

    // The session must receive a private clone, not the shared sessionInfo of the record.
    ASSERT_NE(testSession->lastSessionInfo_, nullptr);
    EXPECT_NE(testSession->lastSessionInfo_.GetRefPtr(), sessionInfo.GetRefPtr());
    EXPECT_EQ(testSession->lastSessionInfo_->requestId, 777);
    // The in-lock mutations are visible on both the shared original and the clone.
    EXPECT_EQ(sessionInfo->want.GetIntParam(Want::PARAM_RESV_DISPLAY_ID, -1), 5);
    EXPECT_EQ(testSession->lastSessionInfo_->want.GetIntParam(Want::PARAM_RESV_DISPLAY_ID, -1), 5);
    EXPECT_EQ(sessionInfo->scenarios, ServerConstant::SCENARIO_MOVE_MISSION_TO_FRONT);
    EXPECT_EQ(testSession->lastSessionInfo_->scenarios, ServerConstant::SCENARIO_MOVE_MISSION_TO_FRONT);
    EXPECT_EQ(sessionInfo->processOptions, nullptr);
    EXPECT_EQ(testSession->lastSessionInfo_->processOptions, nullptr);
    // windowCreateParams is deep-copied by the clone: the session sent to SCB owns a distinct
    // pointee with equal content. The shared sessionInfo of the record legitimately keeps
    // aliasing the caller-provided startOptions pointee after the in-lock assignment, which
    // is safe because startOptions is only read within this call.
    ASSERT_NE(testSession->lastSessionInfo_->windowCreateParams, nullptr);
    EXPECT_NE(testSession->lastSessionInfo_->windowCreateParams.get(),
        startOptions->windowCreateParams_.get());
    EXPECT_NE(testSession->lastSessionInfo_->windowCreateParams.get(),
        sessionInfo->windowCreateParams.get());
    ASSERT_NE(sessionInfo->windowCreateParams, nullptr);
    EXPECT_EQ(sessionInfo->windowCreateParams.get(), startOptions->windowCreateParams_.get());
}

/**
 * @tc.name: UIAbilityLifecycleManager_MoveMissionToFront_00600
 * @tc.desc: MoveMissionToFront with null startOptions takes the else branch: the want display id
 *           is reset to -1 and processOptions/startWindowOption are cleared
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerThirdTest, MoveMissionToFront_006, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(uiAbilityLifecycleManager, nullptr);

    int32_t sessionId = 1;

    Rosen::SessionInfo info;
    auto testSession = sptr<PendingActivationTestSession>::MakeSptr(info);
    ASSERT_NE(testSession, nullptr);
    uiAbilityLifecycleManager->rootSceneSession_ = testSession->AsObject();

    AbilityRequest abilityRequest;
    auto abilityRecord = std::make_shared<UIAbilityRecord>(
        abilityRequest.want, abilityRequest.abilityInfo, abilityRequest.appInfo, abilityRequest.requestCode);
    auto sessionInfo = sptr<AAFwk::SessionInfo>::MakeSptr();
    // Non-null options: the else branch must clear them on the shared info and on the clone.
    auto processOptions = std::make_shared<ProcessOptions>();
    sessionInfo->processOptions = processOptions;
    sessionInfo->startWindowOption = std::make_shared<AAFwk::StartWindowOption>();
    abilityRecord->sessionInfo_ = sessionInfo;

    uiAbilityLifecycleManager->sessionAbilityMap_ = {
        {sessionId, abilityRecord}
    };

    EXPECT_EQ(uiAbilityLifecycleManager->MoveMissionToFront(sessionId, nullptr), ERR_OK);

    ASSERT_NE(testSession->lastSessionInfo_, nullptr);
    // startOptions == nullptr: the display id is explicitly reset to -1.
    EXPECT_EQ(sessionInfo->want.GetIntParam(Want::PARAM_RESV_DISPLAY_ID, -999), -1);
    EXPECT_EQ(testSession->lastSessionInfo_->want.GetIntParam(Want::PARAM_RESV_DISPLAY_ID, -999), -1);
    // No startOptions fields are copied into the want.
    EXPECT_EQ(testSession->lastSessionInfo_->want.GetIntParam(Want::PARAM_RESV_WINDOW_MODE, -1), -1);
    // processOptions / startWindowOption are cleared on both the shared info and the clone.
    EXPECT_EQ(sessionInfo->processOptions, nullptr);
    EXPECT_EQ(sessionInfo->startWindowOption, nullptr);
    EXPECT_EQ(testSession->lastSessionInfo_->processOptions, nullptr);
    EXPECT_EQ(testSession->lastSessionInfo_->startWindowOption, nullptr);
    EXPECT_TRUE(testSession->lastSessionInfo_->canStartAbilityFromBackground);
    EXPECT_EQ(testSession->lastSessionInfo_->scenarios, ServerConstant::SCENARIO_MOVE_MISSION_TO_FRONT);
}

/**
 * @tc.name: UIAbilityLifecycleManager_MoveMissionToFront_00700
 * @tc.desc: MoveMissionToFront substitutes displayId == 0 with the default display id (resolved
 *           before the lock) and propagates the other startOptions fields into the want
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerThirdTest, MoveMissionToFront_007, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(uiAbilityLifecycleManager, nullptr);

    int32_t sessionId = 1;
    auto startOptions = std::make_shared<StartOptions>();
    startOptions->windowMode_ = AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_PRIMARY;
    // displayId 0 asks the manager to substitute the default display id.
    startOptions->displayId_ = 0;
    startOptions->SetSplitRatioPreference(7);

    Rosen::SessionInfo info;
    auto testSession = sptr<PendingActivationTestSession>::MakeSptr(info);
    ASSERT_NE(testSession, nullptr);
    uiAbilityLifecycleManager->rootSceneSession_ = testSession->AsObject();

    AbilityRequest abilityRequest;
    auto abilityRecord = std::make_shared<UIAbilityRecord>(
        abilityRequest.want, abilityRequest.abilityInfo, abilityRequest.appInfo, abilityRequest.requestCode);
    auto sessionInfo = sptr<AAFwk::SessionInfo>::MakeSptr();
    sessionInfo->want.RemoveParam(Want::PARAM_RESV_DISPLAY_ID);
    abilityRecord->sessionInfo_ = sessionInfo;

    uiAbilityLifecycleManager->sessionAbilityMap_ = {
        {sessionId, abilityRecord}
    };

    EXPECT_EQ(uiAbilityLifecycleManager->MoveMissionToFront(sessionId, startOptions), ERR_OK);

    ASSERT_NE(testSession->lastSessionInfo_, nullptr);
    // Self-consistent: the substitution equals whatever DisplayUtil resolves in this UT
    // environment (the value itself may legitimately be -1 without a real display).
    int32_t defaultDisplayId = DisplayUtil::GetDefaultDisplayId();
    EXPECT_EQ(sessionInfo->want.GetIntParam(Want::PARAM_RESV_DISPLAY_ID, -999), defaultDisplayId);
    EXPECT_EQ(testSession->lastSessionInfo_->want.GetIntParam(Want::PARAM_RESV_DISPLAY_ID, -999),
        defaultDisplayId);
    // Other startOptions fields are propagated into the want / the sessionInfo.
    EXPECT_EQ(testSession->lastSessionInfo_->want.GetIntParam(Want::PARAM_RESV_WINDOW_MODE, -1),
        AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_PRIMARY);
    EXPECT_EQ(testSession->lastSessionInfo_->splitRatioPreference, 7);
}

/**
 * @tc.name: UIAbilityLifecycleManager_MoveMissionToFront_00800
 * @tc.desc: MoveMissionToFront keeps a non-zero startOptions display id instead of substituting
 *           the default display id
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerThirdTest, MoveMissionToFront_008, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(uiAbilityLifecycleManager, nullptr);

    int32_t sessionId = 1;
    auto startOptions = std::make_shared<StartOptions>();
    startOptions->windowMode_ = AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_PRIMARY;
    startOptions->displayId_ = 123;

    Rosen::SessionInfo info;
    auto testSession = sptr<PendingActivationTestSession>::MakeSptr(info);
    ASSERT_NE(testSession, nullptr);
    uiAbilityLifecycleManager->rootSceneSession_ = testSession->AsObject();

    AbilityRequest abilityRequest;
    auto abilityRecord = std::make_shared<UIAbilityRecord>(
        abilityRequest.want, abilityRequest.abilityInfo, abilityRequest.appInfo, abilityRequest.requestCode);
    auto sessionInfo = sptr<AAFwk::SessionInfo>::MakeSptr();
    sessionInfo->want.RemoveParam(Want::PARAM_RESV_DISPLAY_ID);
    abilityRecord->sessionInfo_ = sessionInfo;

    uiAbilityLifecycleManager->sessionAbilityMap_ = {
        {sessionId, abilityRecord}
    };

    EXPECT_EQ(uiAbilityLifecycleManager->MoveMissionToFront(sessionId, startOptions), ERR_OK);

    ASSERT_NE(testSession->lastSessionInfo_, nullptr);
    // A non-zero display id is kept as-is, not overwritten with the default display id.
    EXPECT_EQ(sessionInfo->want.GetIntParam(Want::PARAM_RESV_DISPLAY_ID, -999), 123);
    EXPECT_EQ(testSession->lastSessionInfo_->want.GetIntParam(Want::PARAM_RESV_DISPLAY_ID, -999), 123);
    EXPECT_NE(DisplayUtil::GetDefaultDisplayId(), 123);
}

/**
 * @tc.name: UIAbilityLifecycleManager_MoveMissionToFront_00900
 * @tc.desc: MoveMissionToFront with a session id absent from the map returns ERR_INVALID_VALUE
 *           without touching the record or activating any session
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerThirdTest, MoveMissionToFront_009, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(uiAbilityLifecycleManager, nullptr);

    int32_t sessionId = 1;
    int32_t invalidSessionId = 999;
    auto startOptions = std::make_shared<StartOptions>();
    startOptions->windowMode_ = AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_PRIMARY;
    startOptions->displayId_ = 123;

    Rosen::SessionInfo info;
    auto testSession = sptr<PendingActivationTestSession>::MakeSptr(info);
    ASSERT_NE(testSession, nullptr);
    uiAbilityLifecycleManager->rootSceneSession_ = testSession->AsObject();

    AbilityRequest abilityRequest;
    auto abilityRecord = std::make_shared<UIAbilityRecord>(
        abilityRequest.want, abilityRequest.abilityInfo, abilityRequest.appInfo, abilityRequest.requestCode);
    auto sessionInfo = sptr<AAFwk::SessionInfo>::MakeSptr();
    abilityRecord->sessionInfo_ = sessionInfo;

    uiAbilityLifecycleManager->sessionAbilityMap_ = {
        {sessionId, abilityRecord}
    };

    // No record matches invalidSessionId: rejected before any want mutation or activation.
    EXPECT_EQ(uiAbilityLifecycleManager->MoveMissionToFront(invalidSessionId, startOptions),
        ERR_INVALID_VALUE);
    EXPECT_EQ(testSession->lastSessionInfo_, nullptr);
    EXPECT_EQ(sessionInfo->want.GetIntParam(Want::PARAM_RESV_DISPLAY_ID, -999), -999);
    EXPECT_EQ(sessionInfo->want.GetIntParam(Want::PARAM_RESV_WINDOW_MODE, -1), -1);
}

/**
 * @tc.name: UIAbilityLifecycleManager_MoveMissionToFront_01000
 * @tc.desc: MoveMissionToFront from a non-system caller removes PARAM_RESV_WINDOW_FOCUSED from
 *           the want instead of overwriting it with the startOptions value
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerThirdTest, MoveMissionToFront_010, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(uiAbilityLifecycleManager, nullptr);

    int32_t sessionId = 1;
    auto startOptions = std::make_shared<StartOptions>();
    startOptions->windowMode_ = AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_PRIMARY;
    startOptions->displayId_ = 123;
    startOptions->SetWindowFocused(false);

    Rosen::SessionInfo info;
    auto testSession = sptr<PendingActivationTestSession>::MakeSptr(info);
    ASSERT_NE(testSession, nullptr);
    uiAbilityLifecycleManager->rootSceneSession_ = testSession->AsObject();

    AbilityRequest abilityRequest;
    auto abilityRecord = std::make_shared<UIAbilityRecord>(
        abilityRequest.want, abilityRequest.abilityInfo, abilityRequest.appInfo, abilityRequest.requestCode);
    auto sessionInfo = sptr<AAFwk::SessionInfo>::MakeSptr();
    // A pre-existing focused param distinguishes the removal done for a non-system caller from
    // the overwrite with startOptions->GetWindowFocused() done for a system caller.
    sessionInfo->want.SetParam(Want::PARAM_RESV_WINDOW_FOCUSED, true);
    abilityRecord->sessionInfo_ = sessionInfo;

    uiAbilityLifecycleManager->sessionAbilityMap_ = {
        {sessionId, abilityRecord}
    };

    // The UT caller carries no IPC calling token, so it is not treated as a system app.
    ASSERT_FALSE(PermissionVerification::GetInstance()->IsSystemAppCall());

    EXPECT_EQ(uiAbilityLifecycleManager->MoveMissionToFront(sessionId, startOptions), ERR_OK);

    ASSERT_NE(testSession->lastSessionInfo_, nullptr);
    // Non-system caller: PARAM_RESV_WINDOW_FOCUSED is removed from both the shared info and
    // the clone delivered to PendingSessionActivation.
    EXPECT_FALSE(sessionInfo->want.HasParameter(Want::PARAM_RESV_WINDOW_FOCUSED));
    EXPECT_FALSE(testSession->lastSessionInfo_->want.HasParameter(Want::PARAM_RESV_WINDOW_FOCUSED));
    EXPECT_FALSE(testSession->lastSessionInfo_->want.GetBoolParam(Want::PARAM_RESV_WINDOW_FOCUSED, false));
    // The caller-identity branch does not affect the display id handling.
    EXPECT_EQ(testSession->lastSessionInfo_->want.GetIntParam(Want::PARAM_RESV_DISPLAY_ID, -999), 123);
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
 * @tc.name: ExtendGamePreLaunchTimeout_001
 * @tc.desc: No game SA prelaunch ability found, return ERR_NOT_GAME_PRELOAD_STATE.
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerThirdTest, ExtendGamePreLaunchTimeout_001, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    // sessionAbilityMap_ is empty — no prelaunch ability
    int32_t pid = 1234;
    int64_t extendTime = 60 * 1000 * 1000; // 60s in microseconds
    auto ret = mgr->ExtendGamePreLaunchTimeout(pid, extendTime);
    EXPECT_EQ(ret, ERR_NOT_GAME_PRELOAD_STATE);
    EXPECT_TRUE(mgr->extendTaskMap_.empty());
}

/**
 * @tc.name: ExtendGamePreLaunchTimeout_002
 * @tc.desc: Valid prelaunch ability and extendTime, countdown reset, return ERR_OK.
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerThirdTest, ExtendGamePreLaunchTimeout_002, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "MainAbility";
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    ASSERT_NE(abilityRecord, nullptr);
    abilityRecord->SetGameSAPreLaunch(true);
    abilityRecord->SetPid(1234);
    abilityRecord->sessionInfo_ = sptr<SessionInfo>::MakeSptr();
    mgr->sessionAbilityMap_ = { {1, abilityRecord} };

    MyFlag::ffrtSubmitFlag_ = 0;
    int32_t pid = 1234;
    int64_t extendTime = 60 * 1000 * 1000; // 60s in microseconds
    auto ret = mgr->ExtendGamePreLaunchTimeout(pid, extendTime);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_GE(MyFlag::ffrtSubmitFlag_, 1);
    EXPECT_EQ(mgr->extendTaskMap_.size(), 1);
    // Clear flag so detached task body skips NotifyCompleteGamePreLaunch
    abilityRecord->SetGameSAPreLaunch(false);
    usleep(TIMEOUT_VALUE);
}

/**
 * @tc.name: ExtendGamePreLaunchTimeout_003
 * @tc.desc: Multiple calls (reentrant), only last countdown effective.
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerThirdTest, ExtendGamePreLaunchTimeout_003, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "MainAbility";
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    ASSERT_NE(abilityRecord, nullptr);
    abilityRecord->SetGameSAPreLaunch(true);
    abilityRecord->SetPid(1234);
    abilityRecord->sessionInfo_ = sptr<SessionInfo>::MakeSptr();
    mgr->sessionAbilityMap_ = { {1, abilityRecord} };

    MyFlag::ffrtSubmitFlag_ = 0;
    int32_t pid = 1234;
    int64_t extendTime1 = 60 * 1000 * 1000;    // 60s
    int64_t extendTime2 = 120 * 1000 * 1000;   // 120s
    auto ret1 = mgr->ExtendGamePreLaunchTimeout(pid, extendTime1);
    EXPECT_EQ(ret1, ERR_OK);
    auto ret2 = mgr->ExtendGamePreLaunchTimeout(pid, extendTime2);
    EXPECT_EQ(ret2, ERR_OK);
    // Two submissions but map holds only one entry (old handle replaced)
    EXPECT_GE(MyFlag::ffrtSubmitFlag_, 2);
    EXPECT_EQ(mgr->extendTaskMap_.size(), 1);
    // Clear flag so detached task bodies skip NotifyCompleteGamePreLaunch
    abilityRecord->SetGameSAPreLaunch(false);
    usleep(TIMEOUT_VALUE);
}

/**
 * @tc.name: ExtendGamePreLaunchTimeout_004
 * @tc.desc: Ability exists but IsGameSAPreLaunch is false, skip it and return ERR_NOT_GAME_PRELOAD_STATE.
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerThirdTest, ExtendGamePreLaunchTimeout_004, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "MainAbility";
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    ASSERT_NE(abilityRecord, nullptr);
    abilityRecord->SetGameSAPreLaunch(false);
    abilityRecord->SetPid(1234);
    abilityRecord->sessionInfo_ = sptr<SessionInfo>::MakeSptr();
    mgr->sessionAbilityMap_ = { {1, abilityRecord} };

    int32_t pid = 1234;
    int64_t extendTime = 60 * 1000 * 1000; // 60s in microseconds
    auto ret = mgr->ExtendGamePreLaunchTimeout(pid, extendTime);
    EXPECT_EQ(ret, ERR_NOT_GAME_PRELOAD_STATE);
    EXPECT_TRUE(mgr->extendTaskMap_.empty());
}

/**
 * @tc.name: ExtendGamePreLaunchTimeout_005
 * @tc.desc: Ability is prelaunch but pid does not match, skip it and return ERR_NOT_GAME_PRELOAD_STATE.
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerThirdTest, ExtendGamePreLaunchTimeout_005, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "MainAbility";
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    ASSERT_NE(abilityRecord, nullptr);
    abilityRecord->SetGameSAPreLaunch(true);
    abilityRecord->SetPid(1234);
    abilityRecord->sessionInfo_ = sptr<SessionInfo>::MakeSptr();
    mgr->sessionAbilityMap_ = { {1, abilityRecord} };

    int32_t pid = 5678; // mismatched pid
    int64_t extendTime = 60 * 1000 * 1000; // 60s in microseconds
    auto ret = mgr->ExtendGamePreLaunchTimeout(pid, extendTime);
    EXPECT_EQ(ret, ERR_NOT_GAME_PRELOAD_STATE);
    EXPECT_TRUE(mgr->extendTaskMap_.empty());
    // Clear flag so no detached task body triggers
    abilityRecord->SetGameSAPreLaunch(false);
}

/**
 * @tc.name: ExtendGamePreLaunchTimeout_006
 * @tc.desc: sessionAbilityMap_ contains a nullptr entry followed by a valid prelaunch ability;
 *           the nullptr entry is skipped and the valid one is selected, returning ERR_OK.
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerThirdTest, ExtendGamePreLaunchTimeout_006, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "MainAbility";
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    ASSERT_NE(abilityRecord, nullptr);
    abilityRecord->SetGameSAPreLaunch(true);
    abilityRecord->SetPid(1234);
    abilityRecord->sessionInfo_ = sptr<SessionInfo>::MakeSptr();
    // First entry is nullptr (skipped), second entry is the valid prelaunch ability
    mgr->sessionAbilityMap_ = { {1, nullptr}, {2, abilityRecord} };

    MyFlag::ffrtSubmitFlag_ = 0;
    int32_t pid = 1234;
    int64_t extendTime = 60 * 1000 * 1000; // 60s in microseconds
    auto ret = mgr->ExtendGamePreLaunchTimeout(pid, extendTime);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_GE(MyFlag::ffrtSubmitFlag_, 1);
    EXPECT_EQ(mgr->extendTaskMap_.size(), 1);
    // Clear flag so detached task body skips NotifyCompleteGamePreLaunch
    abilityRecord->SetGameSAPreLaunch(false);
    usleep(TIMEOUT_VALUE);
}

/**
 * @tc.name: CleanupExtendTask_001
 * @tc.desc: CleanupExtendTask clears the handle from extendTaskMap_.
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerThirdTest, CleanupExtendTask_001, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    int32_t recordId = 42;
    mgr->extendTaskMap_[recordId] = ffrt::task_handle{};
    EXPECT_EQ(mgr->extendTaskMap_.size(), 1);
    mgr->CleanupExtendTask(recordId);
    EXPECT_TRUE(mgr->extendTaskMap_.empty());
}

/**
 * @tc.name: CleanupExtendTask_002
 * @tc.desc: CleanupExtendTask on non-existent record is a safe no-op.
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerThirdTest, CleanupExtendTask_002, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    // No entry for recordId 999
    mgr->CleanupExtendTask(999);
    EXPECT_TRUE(mgr->extendTaskMap_.empty());
}

/**
 * @tc.name: HandleAbilityRecordReused_GameSAPrelaunch_ExtendTaskCleanup
 * @tc.desc: User launch (HandleAbilityRecordReused) with GameSAPreLaunch cleans up extend task.
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerThirdTest, HandleAbilityRecordReused_GameSAPrelaunch_ExtendTaskCleanup,
    TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "MainAbility";
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    ASSERT_NE(abilityRecord, nullptr);
    abilityRecord->SetGameSAPreLaunch(true);

    // Set up matching session tokens
    auto session = sptr<Rosen::Session>::MakeSptr(Rosen::SessionInfo());
    auto recordSessionInfo = sptr<SessionInfo>::MakeSptr();
    recordSessionInfo->sessionToken = session->AsObject();
    abilityRecord->sessionInfo_ = recordSessionInfo;
    mgr->sessionAbilityMap_ = { {1, abilityRecord} };

    // Pre-populate extendTaskMap_ to simulate a pending delayed task
    mgr->extendTaskMap_[abilityRecord->GetRecordId()] = ffrt::task_handle{};
    EXPECT_EQ(mgr->extendTaskMap_.size(), 1);

    // Mock app mgr
    auto originAppMgr = AppMgrUtil::appMgr_;
    auto appmgr = sptr<AppExecFwk::MockAppMgrService>::MakeSptr();
    AppMgrUtil::appMgr_ = appmgr;

    SessionInfo sessionInfo;
    sessionInfo.sessionToken = session->AsObject();
    sessionInfo.persistentId = 1;
    sessionInfo.reuseDelegatorWindow = false;

    // HandleAbilityRecordReused may return nullptr if SetGameSAPrelaunch fails in test env,
    // but CleanupExtendTask runs before SetGameSAPrelaunch
    auto result = mgr->HandleAbilityRecordReused(abilityRecord, sessionInfo, abilityRequest);

    // Verify extend task was cleaned up (CleanupExtendTask ran before SetGameSAPrelaunch)
    EXPECT_TRUE(mgr->extendTaskMap_.empty());
    // Flag should be cleared regardless of SetGameSAPrelaunch result
    EXPECT_FALSE(abilityRecord->IsGameSAPreLaunch());

    AppMgrUtil::appMgr_ = originAppMgr;
}

/**
 * @tc.name: EraseAbilityRecord_ExtendTaskCleanup
 * @tc.desc: EraseAbilityRecord cleans up extend task as safety net.
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerThirdTest, EraseAbilityRecord_ExtendTaskCleanup, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "MainAbility";
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    ASSERT_NE(abilityRecord, nullptr);
    abilityRecord->sessionInfo_ = sptr<SessionInfo>::MakeSptr();
    mgr->sessionAbilityMap_ = { {1, abilityRecord} };

    // Pre-populate extendTaskMap_
    mgr->extendTaskMap_[abilityRecord->GetRecordId()] = ffrt::task_handle{};
    EXPECT_EQ(mgr->extendTaskMap_.size(), 1);

    mgr->EraseAbilityRecord(abilityRecord);

    // Verify extend task was cleaned up
    EXPECT_TRUE(mgr->extendTaskMap_.empty());
    // Verify ability record was removed from sessionAbilityMap_
    EXPECT_TRUE(mgr->sessionAbilityMap_.empty());
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

/**
 * @tc.name: UIAbilityLifecycleManager_HandleStartSelfTimeout_0100
 * @tc.desc: HandleStartSelfTimeout with null abilityRecord
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerThirdTest, HandleStartSelfTimeout_001, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    MyFlag::ffrtSubmitFlag_ = 0;
    uiAbilityLifecycleManager->HandleStartSelfTimeout(nullptr, false);
    EXPECT_EQ(MyFlag::ffrtSubmitFlag_, 0);
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_HandleStartSelfTimeout_0200
 * @tc.desc: HandleStartSelfTimeout with native state not CREATED
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerThirdTest, HandleStartSelfTimeout_002, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    UIAbilityRecordPtr abilityRecord = InitAbilityRecord();
    abilityRecord->SetNativeState(AbilityNativeState::ATTACHED);
    MyFlag::ffrtSubmitFlag_ = 0;
    uiAbilityLifecycleManager->HandleStartSelfTimeout(abilityRecord, false);
    EXPECT_EQ(MyFlag::ffrtSubmitFlag_, 0);
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_HandleStartSelfTimeout_0300
 * @tc.desc: HandleStartSelfTimeout with isHalf=true, should not call ffrt::submit
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerThirdTest, HandleStartSelfTimeout_003, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    UIAbilityRecordPtr abilityRecord = InitAbilityRecord();
    abilityRecord->SetNativeState(AbilityNativeState::CREATED);
    MyFlag::ffrtSubmitFlag_ = 0;
    uiAbilityLifecycleManager->HandleStartSelfTimeout(abilityRecord, true);
    EXPECT_LE(MyFlag::ffrtSubmitFlag_, 1);
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_HandleStartSelfTimeout_0400
 * @tc.desc: HandleStartSelfTimeout with isHalf=false and native state CREATED, should call ffrt::submit for kill
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerThirdTest, HandleStartSelfTimeout_004, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    UIAbilityRecordPtr abilityRecord = InitAbilityRecord();
    abilityRecord->SetNativeState(AbilityNativeState::CREATED);
    MyFlag::ffrtSubmitFlag_ = 0;
    uiAbilityLifecycleManager->HandleStartSelfTimeout(abilityRecord, false);
    EXPECT_GE(MyFlag::ffrtSubmitFlag_, 1);
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_PostStartSelfTimeoutEvent_0100
 * @tc.desc: PostStartSelfTimeoutEvent should call ffrt::submit twice for half and full timeout
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerThirdTest, PostStartSelfTimeoutEvent_001, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    UIAbilityRecordPtr abilityRecord = InitAbilityRecord();
    abilityRecord->SetNativeState(AbilityNativeState::NORMAL);
    MyFlag::ffrtSubmitFlag_ = 0;
    uiAbilityLifecycleManager->PostStartSelfTimeoutEvent(abilityRecord);
    EXPECT_GE(MyFlag::ffrtSubmitFlag_, 2);
    uiAbilityLifecycleManager.reset();
}
/**
 * @tc.name: UIAbilityLifecycleManager_SubmitSpecifiedFailTask_001
 * @tc.desc: SubmitSpecifiedFailTask with isDelay=false, should submit task immediately
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerThirdTest, SubmitSpecifiedFailTask_001, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    int32_t requestId = 1;
    AbilityRequest abilityRequest;
    auto abilityRecord = std::make_shared<UIAbilityRecord>(
        abilityRequest.want, abilityRequest.abilityInfo, abilityRequest.appInfo, abilityRequest.requestCode);
    mgr->hookSpecifiedMap_ = { { requestId, abilityRecord } };

    MyFlag::ffrtSubmitFlag_ = 0;
    mgr->SubmitSpecifiedFailTask(requestId, false);
    EXPECT_GE(MyFlag::ffrtSubmitFlag_, 1);
    usleep(TIMEOUT_VALUE);
    EXPECT_EQ(mgr->hookSpecifiedMap_.size(), 0);
}

/**
 * @tc.name: UIAbilityLifecycleManager_SubmitSpecifiedFailTask_002
 * @tc.desc: SubmitSpecifiedFailTask with isDelay=true, should submit delayed task
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerThirdTest, SubmitSpecifiedFailTask_002, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    int32_t requestId = 2;
    AbilityRequest abilityRequest;
    auto abilityRecord = std::make_shared<UIAbilityRecord>(
        abilityRequest.want, abilityRequest.abilityInfo, abilityRequest.appInfo, abilityRequest.requestCode);
    mgr->hookSpecifiedMap_ = { { requestId, abilityRecord } };

    MyFlag::ffrtSubmitFlag_ = 0;
    mgr->SubmitSpecifiedFailTask(requestId, true);
    EXPECT_GE(MyFlag::ffrtSubmitFlag_, 1);
    usleep(TIMEOUT_VALUE);
    EXPECT_EQ(mgr->hookSpecifiedMap_.size(), 0);
}

/*
 * @tc.name: PrelaunchAbilityLocked_WithConnect_001
 * @tc.desc: PrelaunchAbilityLocked with connect != nullptr creates CallRecord
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerThirdTest, PrelaunchAbilityLocked_WithConnect_001, TestSize.Level1)
{
    auto mgr = std::make_unique<UIAbilityLifecycleManager>();
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.bundleName = "com.test.bundle";
    abilityRequest.abilityInfo.moduleName = "entry";
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::PAGE;
    abilityRequest.abilityInfo.isStageBasedModel = true;
    abilityRequest.callType = AbilityCallType::CALL_REQUEST_TYPE;
    abilityRequest.connect = sptr<IAbilityConnection>(new UIAbilityLifecycleManagerThirdTestStub());

    auto result = mgr->PrelaunchAbilityLocked(abilityRequest, 0);
    auto record = mgr->FindRecordFromTmpMap(abilityRequest);
    ASSERT_NE(record, nullptr);
    EXPECT_NE(record->callContainer_, nullptr);
}

/*
 * @tc.name: PrelaunchAbilityLocked_WithConnect_002
 * @tc.desc: PrelaunchAbilityLocked with connect == nullptr does NOT create CallRecord
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerThirdTest, PrelaunchAbilityLocked_WithConnect_002, TestSize.Level1)
{
    auto mgr = std::make_unique<UIAbilityLifecycleManager>();
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.bundleName = "com.test.bundle";
    abilityRequest.abilityInfo.moduleName = "entry";
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::PAGE;
    abilityRequest.abilityInfo.isStageBasedModel = true;
    abilityRequest.callType = AbilityCallType::CALL_REQUEST_TYPE;

    auto result = mgr->PrelaunchAbilityLocked(abilityRequest, 0);
    auto record = mgr->FindRecordFromTmpMap(abilityRequest);
    ASSERT_NE(record, nullptr);
    EXPECT_EQ(record->callContainer_, nullptr);
}

/*
 * @tc.name: PrelaunchAbilityLocked_WithConnect_ExistingAbility_001
 * @tc.desc: PrelaunchAbilityLocked with connect for existing ability processes connect (LOG-01)
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerThirdTest, PrelaunchAbilityLocked_WithConnect_ExistingAbility_001, TestSize.Level1)
{
    auto mgr = std::make_unique<UIAbilityLifecycleManager>();
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.bundleName = "com.test.bundle";
    abilityRequest.abilityInfo.moduleName = "entry";
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::PAGE;
    abilityRequest.abilityInfo.isStageBasedModel = true;
    abilityRequest.callType = AbilityCallType::CALL_REQUEST_TYPE;

    auto existingRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    ASSERT_NE(existingRecord, nullptr);
    int32_t persistentId = 100;
    mgr->sessionAbilityMap_[persistentId] = existingRecord;

    abilityRequest.connect = sptr<IAbilityConnection>(new UIAbilityLifecycleManagerThirdTestStub());
    auto result = mgr->PrelaunchAbilityLocked(abilityRequest, 0);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    EXPECT_NE(existingRecord->callContainer_, nullptr);
}

/*
 * @tc.name: NotifyStartupExceptionBySCB_PrelaunchTimeout_001
 * @tc.desc: NotifyStartupExceptionBySCB with prelaunch flag=true triggers NotifyCallersOnPrelaunchTimeout
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerThirdTest, NotifyStartupExceptionBySCB_PrelaunchTimeout_001, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(mgr, nullptr);

    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.bundleName = "com.test.bundle";
    abilityRequest.abilityInfo.moduleName = "entry";

    auto record = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    ASSERT_NE(record, nullptr);
    record->SetPrelaunchFlag(true);

    auto connect = sptr<IAbilityConnection>(new PrelaunchTimeoutConnection());
    ASSERT_NE(connect, nullptr);
    auto callRecord = CallRecord::CreateCallRecord(1, record->shared_from_this(), connect, nullptr);
    ASSERT_NE(callRecord, nullptr);
    callRecord->SetCallState(CallState::REQUESTING);
    callRecord->SetPrelaunchFlag(true);

    record->callContainer_ = std::make_shared<CallContainer>();
    ASSERT_NE(record->callContainer_, nullptr);
    record->callContainer_->AddCallRecord(connect, callRecord);

    int32_t requestId = 42;
    mgr->tmpAbilityMap_[requestId] = record;

    PrelaunchTimeoutConnection::disconnectDoneCount = 0;
    std::string reason = "prelaunch timeout";
    auto ret = mgr->NotifyStartupExceptionBySCB(requestId, reason);
    EXPECT_EQ(ret, ERR_OK);
    usleep(100000);
    EXPECT_EQ(PrelaunchTimeoutConnection::disconnectDoneCount, 1);
    EXPECT_EQ(mgr->tmpAbilityMap_.size(), 0);
}

namespace {
constexpr uint32_t NOTIFY_TEST_ACCESS_TOKEN_ID = 123456;
const std::string NOTIFY_TEST_BUNDLE_NAME = "com.example.notifytest";
const std::string NOTIFY_TEST_MODULE_NAME = "entry";
const std::string NOTIFY_TEST_ABILITY_NAME = "MainAbility";

UIAbilityRecordPtr MakeDiedAbilityRecord(int32_t persistentId, AppExecFwk::LaunchMode launchMode)
{
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.bundleName = NOTIFY_TEST_BUNDLE_NAME;
    abilityRequest.abilityInfo.name = NOTIFY_TEST_ABILITY_NAME;
    abilityRequest.abilityInfo.moduleName = NOTIFY_TEST_MODULE_NAME;
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::PAGE;
    abilityRequest.abilityInfo.launchMode = launchMode;
    abilityRequest.abilityInfo.applicationInfo.accessTokenId = NOTIFY_TEST_ACCESS_TOKEN_ID;
    abilityRequest.appInfo.accessTokenId = NOTIFY_TEST_ACCESS_TOKEN_ID;
    abilityRequest.sessionInfo = new SessionInfo();
    abilityRequest.sessionInfo->persistentId = persistentId;
    return UIAbilityRecord::CreateAbilityRecord(abilityRequest);
}

std::shared_ptr<MockSingleKvStore> MakeKvStoreWithExitReason(AAFwk::Reason reason,
    const std::vector<std::string> &abilityList, const std::string &recoverInfo, bool hasRecoverInfo)
{
    auto kvStore = std::make_shared<MockSingleKvStore>();
    nlohmann::json exitReasonJson = nlohmann::json {
        { "reason", reason },
        { "exit_msg", "test kill" },
        { "kill_msg", "" },
        { "time_stamp", static_cast<int64_t>(1) },
        { "ability_list", abilityList },
    };
    DistributedKv::Entry exitReasonEntry;
    exitReasonEntry.key = std::to_string(NOTIFY_TEST_ACCESS_TOKEN_ID);
    exitReasonEntry.value = DistributedKv::Value(exitReasonJson.dump());
    std::vector<DistributedKv::Entry> allEntries;
    allEntries.push_back(exitReasonEntry);
    EXPECT_CALL(*kvStore, GetEntries(_, _)).WillRepeatedly(
        DoAll(SetArgReferee<1>(allEntries), Return(DistributedKv::Status::SUCCESS)));

    if (hasRecoverInfo) {
        nlohmann::json recoverInfoJson = nlohmann::json {
            { "recover_info_list", { recoverInfo } },
            { "session_id_list", { 1 } },
        };
        DistributedKv::Value recoverValue(recoverInfoJson.dump());
        EXPECT_CALL(*kvStore, Get(_, _)).WillRepeatedly(
            DoAll(SetArgReferee<1>(recoverValue), Return(DistributedKv::Status::SUCCESS)));
    } else {
        EXPECT_CALL(*kvStore, Get(_, _)).WillRepeatedly(Return(DistributedKv::Status::KEY_NOT_FOUND));
    }
    return kvStore;
}
} // namespace

/**
 * @tc.name: UIAbilityLifecycleManager_NotifyStackRecoverFromDied_0100
 * @tc.desc: resource control kill with recover info, notify window with persistentId and isStackRecover true
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerThirdTest, NotifyStackRecoverFromDied_0100, TestSize.Level1)
{
    auto mockSceneSessionManagerLite = new (std::nothrow) Rosen::MockSceneSessionManagerLite();
    Rosen::SessionManagerLite::GetInstance().sceneSessionManagerLiteProxy_ = mockSceneSessionManagerLite;
    EXPECT_CALL(*mockSceneSessionManagerLite, NotifyStackRecoverFromAMS(1, true)).Times(1)
        .WillOnce(Return(Rosen::WSError::WS_OK));

    auto kvStore = MakeKvStoreWithExitReason(AAFwk::REASON_RESOURCE_CONTROL,
        { NOTIFY_TEST_ABILITY_NAME }, NOTIFY_TEST_MODULE_NAME + NOTIFY_TEST_ABILITY_NAME, true);
    DelayedSingleton<AbilityRuntime::AppExitReasonDataManager>::GetInstance()->kvStorePtr_ = kvStore;

    auto mgr = std::make_unique<UIAbilityLifecycleManager>();
    auto abilityRecord = MakeDiedAbilityRecord(1, AppExecFwk::LaunchMode::SINGLETON);
    ASSERT_NE(abilityRecord, nullptr);
    mgr->NotifyStackRecoverFromDied(abilityRecord);
    DelayedSingleton<AbilityRuntime::AppExitReasonDataManager>::GetInstance()->kvStorePtr_ = nullptr;
}

/**
 * @tc.name: UIAbilityLifecycleManager_NotifyStackRecoverFromDied_0200
 * @tc.desc: resource control kill without recover info, no notify
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerThirdTest, NotifyStackRecoverFromDied_0200, TestSize.Level1)
{
    auto mockSceneSessionManagerLite = new (std::nothrow) Rosen::MockSceneSessionManagerLite();
    Rosen::SessionManagerLite::GetInstance().sceneSessionManagerLiteProxy_ = mockSceneSessionManagerLite;
    EXPECT_CALL(*mockSceneSessionManagerLite, NotifyStackRecoverFromAMS(_, _)).Times(0);

    auto kvStore = MakeKvStoreWithExitReason(AAFwk::REASON_RESOURCE_CONTROL,
        { NOTIFY_TEST_ABILITY_NAME }, "", false);
    DelayedSingleton<AbilityRuntime::AppExitReasonDataManager>::GetInstance()->kvStorePtr_ = kvStore;

    auto mgr = std::make_unique<UIAbilityLifecycleManager>();
    auto abilityRecord = MakeDiedAbilityRecord(1, AppExecFwk::LaunchMode::SINGLETON);
    ASSERT_NE(abilityRecord, nullptr);
    mgr->NotifyStackRecoverFromDied(abilityRecord);
    DelayedSingleton<AbilityRuntime::AppExitReasonDataManager>::GetInstance()->kvStorePtr_ = nullptr;
}

/**
 * @tc.name: UIAbilityLifecycleManager_NotifyStackRecoverFromDied_0300
 * @tc.desc: non control kill reason (JS_ERROR) with recover info, no notify
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerThirdTest, NotifyStackRecoverFromDied_0300, TestSize.Level1)
{
    auto mockSceneSessionManagerLite = new (std::nothrow) Rosen::MockSceneSessionManagerLite();
    Rosen::SessionManagerLite::GetInstance().sceneSessionManagerLiteProxy_ = mockSceneSessionManagerLite;
    EXPECT_CALL(*mockSceneSessionManagerLite, NotifyStackRecoverFromAMS(_, _)).Times(0);

    auto kvStore = MakeKvStoreWithExitReason(AAFwk::REASON_JS_ERROR,
        { NOTIFY_TEST_ABILITY_NAME }, NOTIFY_TEST_MODULE_NAME + NOTIFY_TEST_ABILITY_NAME, true);
    DelayedSingleton<AbilityRuntime::AppExitReasonDataManager>::GetInstance()->kvStorePtr_ = kvStore;

    auto mgr = std::make_unique<UIAbilityLifecycleManager>();
    auto abilityRecord = MakeDiedAbilityRecord(1, AppExecFwk::LaunchMode::SINGLETON);
    ASSERT_NE(abilityRecord, nullptr);
    mgr->NotifyStackRecoverFromDied(abilityRecord);
    DelayedSingleton<AbilityRuntime::AppExitReasonDataManager>::GetInstance()->kvStorePtr_ = nullptr;
}

/**
 * @tc.name: UIAbilityLifecycleManager_NotifyStackRecoverFromDied_0400
 * @tc.desc: exit reason record exists but the died ability is not in the ability list, no notify
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerThirdTest, NotifyStackRecoverFromDied_0400, TestSize.Level1)
{
    auto mockSceneSessionManagerLite = new (std::nothrow) Rosen::MockSceneSessionManagerLite();
    Rosen::SessionManagerLite::GetInstance().sceneSessionManagerLiteProxy_ = mockSceneSessionManagerLite;
    EXPECT_CALL(*mockSceneSessionManagerLite, NotifyStackRecoverFromAMS(_, _)).Times(0);

    auto kvStore = MakeKvStoreWithExitReason(AAFwk::REASON_RESOURCE_CONTROL,
        { "OtherAbility" }, NOTIFY_TEST_MODULE_NAME + NOTIFY_TEST_ABILITY_NAME, true);
    DelayedSingleton<AbilityRuntime::AppExitReasonDataManager>::GetInstance()->kvStorePtr_ = kvStore;

    auto mgr = std::make_unique<UIAbilityLifecycleManager>();
    auto abilityRecord = MakeDiedAbilityRecord(1, AppExecFwk::LaunchMode::SINGLETON);
    ASSERT_NE(abilityRecord, nullptr);
    mgr->NotifyStackRecoverFromDied(abilityRecord);
    DelayedSingleton<AbilityRuntime::AppExitReasonDataManager>::GetInstance()->kvStorePtr_ = nullptr;
}

/**
 * @tc.name: UIAbilityLifecycleManager_NotifyStackRecoverFromDied_0500
 * @tc.desc: performance control kill with recover info, notify window
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerThirdTest, NotifyStackRecoverFromDied_0500, TestSize.Level1)
{
    auto mockSceneSessionManagerLite = new (std::nothrow) Rosen::MockSceneSessionManagerLite();
    Rosen::SessionManagerLite::GetInstance().sceneSessionManagerLiteProxy_ = mockSceneSessionManagerLite;
    EXPECT_CALL(*mockSceneSessionManagerLite, NotifyStackRecoverFromAMS(1, true)).Times(1)
        .WillOnce(Return(Rosen::WSError::WS_OK));

    auto kvStore = MakeKvStoreWithExitReason(AAFwk::REASON_PERFORMANCE_CONTROL,
        { NOTIFY_TEST_ABILITY_NAME }, NOTIFY_TEST_MODULE_NAME + NOTIFY_TEST_ABILITY_NAME, true);
    DelayedSingleton<AbilityRuntime::AppExitReasonDataManager>::GetInstance()->kvStorePtr_ = kvStore;

    auto mgr = std::make_unique<UIAbilityLifecycleManager>();
    auto abilityRecord = MakeDiedAbilityRecord(1, AppExecFwk::LaunchMode::SINGLETON);
    ASSERT_NE(abilityRecord, nullptr);
    mgr->NotifyStackRecoverFromDied(abilityRecord);
    DelayedSingleton<AbilityRuntime::AppExitReasonDataManager>::GetInstance()->kvStorePtr_ = nullptr;
}

/**
 * @tc.name: UIAbilityLifecycleManager_NotifyStackRecoverFromDied_0600
 * @tc.desc: standard launch mode queries exit reason and recover info with persistentId suffix
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerThirdTest, NotifyStackRecoverFromDied_0600, TestSize.Level1)
{
    auto mockSceneSessionManagerLite = new (std::nothrow) Rosen::MockSceneSessionManagerLite();
    Rosen::SessionManagerLite::GetInstance().sceneSessionManagerLiteProxy_ = mockSceneSessionManagerLite;
    EXPECT_CALL(*mockSceneSessionManagerLite, NotifyStackRecoverFromAMS(2, true)).Times(1)
        .WillOnce(Return(Rosen::WSError::WS_OK));

    std::string suffixedAbilityName = NOTIFY_TEST_ABILITY_NAME + std::to_string(2);
    auto kvStore = MakeKvStoreWithExitReason(AAFwk::REASON_RESOURCE_CONTROL,
        { suffixedAbilityName }, NOTIFY_TEST_MODULE_NAME + suffixedAbilityName, true);
    DelayedSingleton<AbilityRuntime::AppExitReasonDataManager>::GetInstance()->kvStorePtr_ = kvStore;

    auto mgr = std::make_unique<UIAbilityLifecycleManager>();
    auto abilityRecord = MakeDiedAbilityRecord(2, AppExecFwk::LaunchMode::STANDARD);
    ASSERT_NE(abilityRecord, nullptr);
    mgr->NotifyStackRecoverFromDied(abilityRecord);
    DelayedSingleton<AbilityRuntime::AppExitReasonDataManager>::GetInstance()->kvStorePtr_ = nullptr;
}

/**
 * @tc.name: UIAbilityLifecycleManager_NotifyStackRecoverFromDied_0700
 * @tc.desc: died record without session info, no notify and no crash
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerThirdTest, NotifyStackRecoverFromDied_0700, TestSize.Level1)
{
    auto mockSceneSessionManagerLite = new (std::nothrow) Rosen::MockSceneSessionManagerLite();
    Rosen::SessionManagerLite::GetInstance().sceneSessionManagerLiteProxy_ = mockSceneSessionManagerLite;
    EXPECT_CALL(*mockSceneSessionManagerLite, NotifyStackRecoverFromAMS(_, _)).Times(0);

    auto mgr = std::make_unique<UIAbilityLifecycleManager>();
    Want want;
    AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.bundleName = NOTIFY_TEST_BUNDLE_NAME;
    AppExecFwk::ApplicationInfo appInfo;
    auto abilityRecord = std::make_shared<UIAbilityRecord>(want, abilityInfo, appInfo, 0);
    ASSERT_NE(abilityRecord, nullptr);
    mgr->NotifyStackRecoverFromDied(abilityRecord);
}

/**
 * @tc.name: UIAbilityLifecycleManager_NotifyStackRecoverFromDied_0800
 * @tc.desc: null died record, no notify and no crash
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerThirdTest, NotifyStackRecoverFromDied_0800, TestSize.Level1)
{
    auto mockSceneSessionManagerLite = new (std::nothrow) Rosen::MockSceneSessionManagerLite();
    Rosen::SessionManagerLite::GetInstance().sceneSessionManagerLiteProxy_ = mockSceneSessionManagerLite;
    EXPECT_CALL(*mockSceneSessionManagerLite, NotifyStackRecoverFromAMS(_, _)).Times(0);

    auto mgr = std::make_unique<UIAbilityLifecycleManager>();
    mgr->NotifyStackRecoverFromDied(nullptr);
}

/**
 * @tc.name: UIAbilityLifecycleManager_NotifyStackRecoverFromDied_0900
 * @tc.desc: kv store unavailable, no notify and no crash
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerThirdTest, NotifyStackRecoverFromDied_0900, TestSize.Level1)
{
    auto mockSceneSessionManagerLite = new (std::nothrow) Rosen::MockSceneSessionManagerLite();
    Rosen::SessionManagerLite::GetInstance().sceneSessionManagerLiteProxy_ = mockSceneSessionManagerLite;
    EXPECT_CALL(*mockSceneSessionManagerLite, NotifyStackRecoverFromAMS(_, _)).Times(0);

    DelayedSingleton<AbilityRuntime::AppExitReasonDataManager>::GetInstance()->kvStorePtr_ = nullptr;

    auto mgr = std::make_unique<UIAbilityLifecycleManager>();
    auto abilityRecord = MakeDiedAbilityRecord(1, AppExecFwk::LaunchMode::SINGLETON);
    ASSERT_NE(abilityRecord, nullptr);
    mgr->NotifyStackRecoverFromDied(abilityRecord);
    DelayedSingleton<AbilityRuntime::AppExitReasonDataManager>::GetInstance()->kvStorePtr_ = nullptr;
}

/**
 * @tc.name: UIAbilityLifecycleManager_HandleAppDataCleared_0100
 * @tc.desc: recover info exists, notify window to clear flag for each session and remove records
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerThirdTest, HandleAppDataCleared_0100, TestSize.Level1)
{
    auto mockSceneSessionManagerLite = new (std::nothrow) Rosen::MockSceneSessionManagerLite();
    Rosen::SessionManagerLite::GetInstance().sceneSessionManagerLiteProxy_ = mockSceneSessionManagerLite;
    EXPECT_CALL(*mockSceneSessionManagerLite, NotifyStackRecoverFromAMS(1, false)).Times(1)
        .WillOnce(Return(Rosen::WSError::WS_OK));
    EXPECT_CALL(*mockSceneSessionManagerLite, NotifyStackRecoverFromAMS(2, false)).Times(1)
        .WillOnce(Return(Rosen::WSError::WS_OK));

    auto kvStore = std::make_shared<MockSingleKvStore>();
    nlohmann::json recoverInfoJson = nlohmann::json {
        { "recover_info_list", { NOTIFY_TEST_MODULE_NAME + NOTIFY_TEST_ABILITY_NAME } },
        { "session_id_list", { 1, 2 } },
    };
    DistributedKv::Value recoverValue(recoverInfoJson.dump());
    EXPECT_CALL(*kvStore, Get(_, _)).WillRepeatedly(
        DoAll(SetArgReferee<1>(recoverValue), Return(DistributedKv::Status::SUCCESS)));
    std::vector<DistributedKv::Entry> allEntries;
    EXPECT_CALL(*kvStore, GetEntries(_, _)).WillRepeatedly(
        DoAll(SetArgReferee<1>(allEntries), Return(DistributedKv::Status::SUCCESS)));
    EXPECT_CALL(*kvStore, Delete(_)).Times(AtLeast(1))
        .WillRepeatedly(Return(DistributedKv::Status::SUCCESS));
    DelayedSingleton<AbilityRuntime::AppExitReasonDataManager>::GetInstance()->kvStorePtr_ = kvStore;

    auto mgr = std::make_unique<UIAbilityLifecycleManager>();
    mgr->HandleAppDataCleared(NOTIFY_TEST_BUNDLE_NAME, NOTIFY_TEST_ACCESS_TOKEN_ID);
    DelayedSingleton<AbilityRuntime::AppExitReasonDataManager>::GetInstance()->kvStorePtr_ = nullptr;
}

/**
 * @tc.name: UIAbilityLifecycleManager_HandleAppDataCleared_0200
 * @tc.desc: no recover info record, no notify and no crash
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerThirdTest, HandleAppDataCleared_0200, TestSize.Level1)
{
    auto mockSceneSessionManagerLite = new (std::nothrow) Rosen::MockSceneSessionManagerLite();
    Rosen::SessionManagerLite::GetInstance().sceneSessionManagerLiteProxy_ = mockSceneSessionManagerLite;
    EXPECT_CALL(*mockSceneSessionManagerLite, NotifyStackRecoverFromAMS(_, _)).Times(0);

    auto kvStore = std::make_shared<MockSingleKvStore>();
    EXPECT_CALL(*kvStore, Get(_, _)).WillRepeatedly(Return(DistributedKv::Status::KEY_NOT_FOUND));
    std::vector<DistributedKv::Entry> allEntries;
    EXPECT_CALL(*kvStore, GetEntries(_, _)).WillRepeatedly(
        DoAll(SetArgReferee<1>(allEntries), Return(DistributedKv::Status::SUCCESS)));
    DelayedSingleton<AbilityRuntime::AppExitReasonDataManager>::GetInstance()->kvStorePtr_ = kvStore;

    auto mgr = std::make_unique<UIAbilityLifecycleManager>();
    mgr->HandleAppDataCleared(NOTIFY_TEST_BUNDLE_NAME, NOTIFY_TEST_ACCESS_TOKEN_ID);
    DelayedSingleton<AbilityRuntime::AppExitReasonDataManager>::GetInstance()->kvStorePtr_ = nullptr;
}

/**
 * @tc.name: UIAbilityLifecycleManager_HandleAppDataCleared_0300
 * @tc.desc: invalid bundleName or accessTokenId, no notify and no crash
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerThirdTest, HandleAppDataCleared_0300, TestSize.Level1)
{
    auto mockSceneSessionManagerLite = new (std::nothrow) Rosen::MockSceneSessionManagerLite();
    Rosen::SessionManagerLite::GetInstance().sceneSessionManagerLiteProxy_ = mockSceneSessionManagerLite;
    EXPECT_CALL(*mockSceneSessionManagerLite, NotifyStackRecoverFromAMS(_, _)).Times(0);

    auto mgr = std::make_unique<UIAbilityLifecycleManager>();
    mgr->HandleAppDataCleared("", NOTIFY_TEST_ACCESS_TOKEN_ID);
    mgr->HandleAppDataCleared(NOTIFY_TEST_BUNDLE_NAME, 0);
    SUCCEED();
}


namespace {
// Local session capturing the SessionInfo passed by ChangeAbilityVisibility so tests can verify
// that a private clone (instead of the shared sessionInfo owned by the record) is sent to SCB.
class VisibilityTestSession : public Rosen::Session {
public:
    explicit VisibilityTestSession(const Rosen::SessionInfo &info) : Rosen::Session(info) {}
    ~VisibilityTestSession() override = default;

    Rosen::WSError ChangeSessionVisibilityWithStatusBar(const sptr<AAFwk::SessionInfo> abilitySessionInfo,
        bool visible) override
    {
        lastSessionInfo_ = abilitySessionInfo;
        lastVisible_ = visible;
        return Rosen::WSError::WS_OK;
    }

    sptr<AAFwk::SessionInfo> lastSessionInfo_ = nullptr;
    bool lastVisible_ = false;
};
} // namespace

/**
 * @tc.name: UIAbilityLifecycleManager_ChangeAbilityVisibility_0100
 * @tc.desc: ChangeAbilityVisibility sends a private sessionInfo snapshot to the session and
 *           keeps the shared sessionInfo owned by the record untouched
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerThirdTest, ChangeAbilityVisibility_001, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(mgr, nullptr);

    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.bundleName = "com.example.unittest";
    abilityRequest.appInfo.accessTokenId = IPCSkeleton::GetCallingTokenID();
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    ASSERT_NE(abilityRecord, nullptr);

    auto sessionInfo = sptr<AAFwk::SessionInfo>::MakeSptr();
    auto testSession = sptr<VisibilityTestSession>::MakeSptr(Rosen::SessionInfo());
    ASSERT_NE(testSession, nullptr);
    sessionInfo->sessionToken = testSession->AsObject();
    sessionInfo->want.SetParam("visibilityTestKey", 100);
    auto processOptions = std::make_shared<ProcessOptions>();
    processOptions->processMode = ProcessMode::NO_ATTACHMENT;
    sessionInfo->processOptions = processOptions;
    // Every pointee that is marshalled field by field must be deep-copied by Clone.
    auto startSetting = AbilityStartSetting::GetEmptySetting();
    sessionInfo->startSetting = startSetting;
    auto startWindowOption = std::make_shared<AAFwk::StartWindowOption>();
    startWindowOption->startWindowBackgroundColor = "#FFFFFF";
    sessionInfo->startWindowOption = startWindowOption;
    auto windowCreateParams = std::make_shared<Rosen::WindowCreateParams>();
    sessionInfo->windowCreateParams = windowCreateParams;
    abilityRecord->sessionInfo_ = sessionInfo;

    int32_t sessionId = 1;
    mgr->sessionAbilityMap_.emplace(sessionId, abilityRecord);
    auto token = abilityRecord->GetToken()->AsObject();
    ASSERT_NE(token, nullptr);

    auto showParams = std::make_shared<Rosen::WindowShowParams>();
    showParams->windowFocused = false;
    showParams->raiseTop = false;
    EXPECT_EQ(mgr->ChangeAbilityVisibility(token, true, showParams), ERR_OK);
    // The session must receive a private clone, not the shared sessionInfo of the record.
    ASSERT_NE(testSession->lastSessionInfo_, nullptr);
    EXPECT_NE(testSession->lastSessionInfo_.GetRefPtr(), sessionInfo.GetRefPtr());
    EXPECT_TRUE(testSession->lastVisible_);
    EXPECT_EQ(testSession->lastSessionInfo_->want.GetIntParam("visibilityTestKey", -1), 100);
    // showParams delivered by showAbility are set on the private clone only; the shared
    // sessionInfo owned by the record keeps its own showParams untouched.
    ASSERT_NE(testSession->lastSessionInfo_->showParams, nullptr);
    EXPECT_EQ(testSession->lastSessionInfo_->showParams.get(), showParams.get());
    EXPECT_FALSE(testSession->lastSessionInfo_->showParams->windowFocused);
    EXPECT_EQ(sessionInfo->showParams, nullptr);
    // processOptions is deep-copied by the clone: distinct pointee, equal values.
    ASSERT_NE(testSession->lastSessionInfo_->processOptions, nullptr);
    EXPECT_NE(testSession->lastSessionInfo_->processOptions.get(), processOptions.get());
    EXPECT_EQ(testSession->lastSessionInfo_->processOptions->processMode, ProcessMode::NO_ATTACHMENT);
    EXPECT_EQ(testSession->lastSessionInfo_->processOptions->selfPid, -1);
    // In-place mutation of the original options must not leak into the already-taken clone.
    processOptions->selfPid = 4321;
    EXPECT_EQ(testSession->lastSessionInfo_->processOptions->selfPid, -1);
    // startSetting / startWindowOption / windowCreateParams are deep-copied as well.
    ASSERT_NE(testSession->lastSessionInfo_->startSetting, nullptr);
    EXPECT_NE(testSession->lastSessionInfo_->startSetting.get(), startSetting.get());
    ASSERT_NE(testSession->lastSessionInfo_->startWindowOption, nullptr);
    EXPECT_NE(testSession->lastSessionInfo_->startWindowOption.get(), startWindowOption.get());
    EXPECT_EQ(testSession->lastSessionInfo_->startWindowOption->startWindowBackgroundColor, "#FFFFFF");
    startWindowOption->startWindowBackgroundColor = "#000000";
    EXPECT_EQ(testSession->lastSessionInfo_->startWindowOption->startWindowBackgroundColor, "#FFFFFF");
    ASSERT_NE(testSession->lastSessionInfo_->windowCreateParams, nullptr);
    EXPECT_NE(testSession->lastSessionInfo_->windowCreateParams.get(), windowCreateParams.get());
    // The shared sessionInfo must stay untouched.
    EXPECT_EQ(sessionInfo->want.GetIntParam("visibilityTestKey", -1), 100);
    EXPECT_NE(sessionInfo->processOptions, nullptr);
    EXPECT_EQ(sessionInfo->startSetting, startSetting);
    EXPECT_EQ(sessionInfo->startWindowOption, startWindowOption);
    EXPECT_EQ(sessionInfo->windowCreateParams, windowCreateParams);
}

/**
 * @tc.name: UIAbilityLifecycleManager_ChangeAbilityVisibility_0200
 * @tc.desc: ChangeAbilityVisibility rejects the call when the caller process has not been
 *           attached to the status bar with attachment-required processOptions
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerThirdTest, ChangeAbilityVisibility_002, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(mgr, nullptr);

    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.bundleName = "com.example.unittest";
    abilityRequest.appInfo.accessTokenId = IPCSkeleton::GetCallingTokenID();
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    ASSERT_NE(abilityRecord, nullptr);

    auto sessionInfo = sptr<AAFwk::SessionInfo>::MakeSptr();
    auto processOptions = std::make_shared<ProcessOptions>();
    processOptions->processMode = ProcessMode::NEW_PROCESS_ATTACH_TO_PARENT;
    sessionInfo->processOptions = processOptions;
    abilityRecord->sessionInfo_ = sessionInfo;

    int32_t sessionId = 1;
    mgr->sessionAbilityMap_.emplace(sessionId, abilityRecord);
    auto token = abilityRecord->GetToken()->AsObject();
    ASSERT_NE(token, nullptr);

    EXPECT_EQ(mgr->ChangeAbilityVisibility(token, true, nullptr), ERR_START_OPTIONS_CHECK_FAILED);
}

/**
 * @tc.name: UIAbilityLifecycleManager_ChangeAbilityVisibility_0300
 * @tc.desc: ChangeAbilityVisibility rejects the call from a different application
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerThirdTest, ChangeAbilityVisibility_003, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(mgr, nullptr);

    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.bundleName = "com.example.unittest";
    abilityRequest.appInfo.accessTokenId = IPCSkeleton::GetCallingTokenID() + 1;
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    ASSERT_NE(abilityRecord, nullptr);
    abilityRecord->sessionInfo_ = sptr<AAFwk::SessionInfo>::MakeSptr();

    int32_t sessionId = 1;
    mgr->sessionAbilityMap_.emplace(sessionId, abilityRecord);
    auto token = abilityRecord->GetToken()->AsObject();
    ASSERT_NE(token, nullptr);

    EXPECT_EQ(mgr->ChangeAbilityVisibility(token, true, nullptr), ERR_NATIVE_NOT_SELF_APPLICATION);
}

/**
 * @tc.name: UIAbilityLifecycleManager_ChangeAbilityVisibility_0400
 * @tc.desc: ChangeAbilityVisibility with an unknown token returns ERR_INVALID_VALUE
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerThirdTest, ChangeAbilityVisibility_004, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(mgr, nullptr);

    auto token = sptr<MockIRemoteObject>::MakeSptr();
    ASSERT_NE(token, nullptr);
    EXPECT_EQ(mgr->ChangeAbilityVisibility(token, true, nullptr), ERR_INVALID_VALUE);
}

/**
 * @tc.name: UIAbilityLifecycleManager_ChangeAbilityVisibility_0500
 * @tc.desc: ChangeAbilityVisibility runs concurrently with in-lock sessionInfo writers without
 *           crash or deadlock; run with ASAN/TSAN to catch want corruption regressions
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerThirdTest, ChangeAbilityVisibility_Stress_001, TestSize.Level3)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(mgr, nullptr);

    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.bundleName = "com.example.unittest";
    abilityRequest.appInfo.accessTokenId = IPCSkeleton::GetCallingTokenID();
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    ASSERT_NE(abilityRecord, nullptr);

    auto sessionInfo = sptr<AAFwk::SessionInfo>::MakeSptr();
    sessionInfo->sessionToken = sptr<Rosen::Session>::MakeSptr(Rosen::SessionInfo())->AsObject();
    ASSERT_NE(sessionInfo->sessionToken, nullptr);
    auto processOptions = std::make_shared<ProcessOptions>();
    processOptions->processMode = ProcessMode::NO_ATTACHMENT;
    sessionInfo->processOptions = processOptions;
    abilityRecord->sessionInfo_ = sessionInfo;

    int32_t sessionId = 1;
    mgr->sessionAbilityMap_.emplace(sessionId, abilityRecord);
    auto token = abilityRecord->GetToken()->AsObject();
    ASSERT_NE(token, nullptr);

    const int32_t loopCount = 200;
    std::atomic<bool> writerRunning = true;
    // Emulates in-lock writers (e.g. StartUIAbility, MoveMissionToFront) mutating the shared
    // want while ChangeAbilityVisibility copies its snapshot under the same sessionLock_.
    std::thread writerThread([&mgr, &abilityRecord, &writerRunning]() {
        int32_t value = 0;
        while (writerRunning.load()) {
            std::lock_guard<ffrt::mutex> guard(mgr->sessionLock_);
            auto sharedInfo = abilityRecord->GetSessionInfo();
            if (sharedInfo != nullptr) {
                sharedInfo->want.SetParam("stressKey", ++value);
                sharedInfo->want.RemoveParam("stressKey2");
                sharedInfo->want.SetParam("stressKey2", value);
            }
        }
    });
    for (int32_t i = 0; i < loopCount; ++i) {
        EXPECT_EQ(mgr->ChangeAbilityVisibility(token, i % 2 == 0, nullptr), ERR_OK);
    }
    writerRunning.store(false);
    writerThread.join();
}

/**
 * @tc.name: UIAbilityLifecycleManager_ChangeAbilityVisibility_0600
 * @tc.desc: ChangeAbilityVisibility returns ERR_INVALID_VALUE when the record exists but carries
 *           no sessionInfo (sessionInfo null-check inside the lock)
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerThirdTest, ChangeAbilityVisibility_005, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(mgr, nullptr);

    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.bundleName = "com.example.unittest";
    // Keep the caller-self check passing so the sessionInfo null-check branch is reached.
    abilityRequest.appInfo.accessTokenId = IPCSkeleton::GetCallingTokenID();
    auto abilityRecord = UIAbilityRecord::CreateAbilityRecord(abilityRequest);
    ASSERT_NE(abilityRecord, nullptr);
    // The record is registered, but no SessionInfo is attached to it.
    abilityRecord->sessionInfo_ = nullptr;

    int32_t sessionId = 1;
    mgr->sessionAbilityMap_.emplace(sessionId, abilityRecord);
    auto token = abilityRecord->GetToken()->AsObject();
    ASSERT_NE(token, nullptr);

    EXPECT_EQ(mgr->ChangeAbilityVisibility(token, true, nullptr), ERR_INVALID_VALUE);
}
}  // namespace AAFwk
}  // namespace OHOS