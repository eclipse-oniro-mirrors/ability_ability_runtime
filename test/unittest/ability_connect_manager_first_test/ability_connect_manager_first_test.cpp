/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include <memory>

#define private public
#define protected public
#include "ability_connect_manager.h"
#include "common_extension_manager.h"
#undef private
#undef protected

#include "ability_config.h"
#include "ability_manager_errors.h"
#include "ability_scheduler.h"
#include "ability_util.h"
#include "agent_card.h"
#include "agent_extension_connection_constants.h"
#include "bundlemgr/mock_bundle_manager.h"
#include "hilog_tag_wrapper.h"
#include "mock_ability_connect_callback.h"
#include "mock_sa_call.h"
#include "mock_task_handler_wrap.h"
#include "sa_mgr_client.h"
#include "system_ability_definition.h"
#include "session/host/include/session.h"
#include <thread>
#include <chrono>

using namespace testing::ext;
using namespace OHOS::AppExecFwk;
using testing::_;
using testing::Invoke;
using testing::Return;
using testing::SetArgReferee;
using ::testing::DoAll;

namespace {
const std::string PARAM_RESV_CALLER_APP_ID("ohos.aafwk.param.callerAppId");
constexpr uint32_t FAKE_TOKENID = 111;
}

namespace OHOS {
namespace AAFwk {
class AbilityConnectManagerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    AbilityConnectManager* ConnectManager() const;
    std::shared_ptr<MockTaskHandlerWrap> TaskHandler() const;
    std::shared_ptr<EventHandlerWrap> EventHandler() const;

    AbilityRequest GenerateAbilityRequest(const std::string& deviceName, const std::string& abilityName,
        const std::string& appName, const std::string& bundleName, const std::string& moduleName);

    sptr<SessionInfo> MockSessionInfo(int32_t persistentId);
    std::shared_ptr<BaseExtensionRecord> InitAbilityRecord();
    std::shared_ptr<MockTaskHandlerWrap> taskHandler_;
protected:
    AbilityRequest abilityRequest_{};
    AbilityRequest abilityRequest1_{};
    AbilityRequest abilityRequest2_{};
    std::shared_ptr<BaseExtensionRecord> serviceRecord_{ nullptr };
    std::shared_ptr<BaseExtensionRecord> serviceRecord1_{ nullptr };
    std::shared_ptr<BaseExtensionRecord> serviceRecord2_{ nullptr };
    OHOS::sptr<Token> serviceToken_{ nullptr };
    OHOS::sptr<Token> serviceToken1_{ nullptr };
    OHOS::sptr<Token> serviceToken2_{ nullptr };
    OHOS::sptr<IAbilityConnection> callbackA_{ nullptr };
    OHOS::sptr<IAbilityConnection> callbackB_{ nullptr };

private:
    std::shared_ptr<AbilityConnectManager> connectManager_;
    std::shared_ptr<EventHandlerWrap> eventHandler_;
};

AbilityRequest AbilityConnectManagerTest::GenerateAbilityRequest(const std::string& deviceName,
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

sptr<SessionInfo> AbilityConnectManagerTest::MockSessionInfo(int32_t persistentId)
{
    sptr<SessionInfo> sessionInfo = new (std::nothrow) SessionInfo();
    if (!sessionInfo) {
        TAG_LOGE(AAFwkTag::TEST, "sessionInfo is nullptr");
        return nullptr;
    }
    sessionInfo->persistentId = persistentId;
    return sessionInfo;
}

std::shared_ptr<BaseExtensionRecord> AbilityConnectManagerTest::InitAbilityRecord()
{
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = AbilityType::PAGE;
    std::shared_ptr<BaseExtensionRecord> abilityRecord = BaseExtensionRecord::CreateBaseExtensionRecord(
        abilityRequest);
    return abilityRecord;
}

void AbilityConnectManagerTest::SetUpTestCase(void)
{}

void AbilityConnectManagerTest::TearDownTestCase(void)
{}

void AbilityConnectManagerTest::SetUp(void)
{
    connectManager_ = std::make_unique<AbilityConnectManager>(0);
    taskHandler_ = MockTaskHandlerWrap::CreateQueueHandler("AbilityConnectManagerTest");
    eventHandler_ = std::make_shared<EventHandlerWrap>(taskHandler_);
    // generate ability request
    std::string deviceName = "device";
    std::string abilityName = "ServiceAbility";
    std::string appName = "hiservcie";
    std::string bundleName = "com.ix.hiservcie";
    std::string moduleName = "entry";
    abilityRequest_ = GenerateAbilityRequest(deviceName, abilityName, appName, bundleName, moduleName);
    serviceRecord_ = BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest_);
    serviceToken_ = serviceRecord_->GetToken();
    std::string deviceName1 = "device";
    std::string abilityName1 = "musicServiceAbility";
    std::string appName1 = "musicservcie";
    std::string bundleName1 = "com.ix.musicservcie";
    std::string moduleName1 = "entry";
    abilityRequest1_ = GenerateAbilityRequest(deviceName1, abilityName1, appName1, bundleName1, moduleName1);
    serviceRecord1_ = BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest1_);
    std::string deviceName2 = "device";
    std::string abilityName2 = "residentServiceAbility";
    std::string appName2 = "residentservcie";
    std::string bundleName2 = "com.ix.residentservcie";
    std::string moduleName2 = "entry";
    abilityRequest2_ = GenerateAbilityRequest(deviceName2, abilityName2, appName2, bundleName2, moduleName2);
    serviceRecord2_ = BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest2_);
    serviceToken2_ = serviceRecord_->GetToken();
    serviceToken1_ = serviceRecord_->GetToken();
    callbackA_ = new AbilityConnectCallback();
    callbackB_ = new AbilityConnectCallback();
}

void AbilityConnectManagerTest::TearDown(void)
{
    // reset the callback count
    AbilityConnectCallback::onAbilityConnectDoneCount = 0;
    AbilityConnectCallback::onAbilityDisconnectDoneCount = 0;
    serviceRecord_ = nullptr;
}

AbilityConnectManager* AbilityConnectManagerTest::ConnectManager() const
{
    return connectManager_.get();
}

std::shared_ptr<MockTaskHandlerWrap> AbilityConnectManagerTest::TaskHandler() const
{
    return taskHandler_;
}

std::shared_ptr<EventHandlerWrap> AbilityConnectManagerTest::EventHandler() const
{
    return eventHandler_;
}

/*
 * Feature: AbilityConnectManager
 * Function: HandleActiveAbility
 */
HWTEST_F(AbilityConnectManagerTest, HandleActiveAbility_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "HandleActiveAbility_001 start");
    std::shared_ptr<AbilityConnectManager> connectManager = std::make_shared<AbilityConnectManager>(0);
    std::shared_ptr<BaseExtensionRecord> abilityRecord = serviceRecord_;
    AbilityRequest abilityRequest;
    sptr<IRemoteObject> callerToken = abilityRecord->GetToken();
    OHOS::sptr<IAbilityConnection> callback1 = new AbilityConnectCallback();
    std::shared_ptr<ConnectionRecord> connectRecord =
        std::make_shared<ConnectionRecord>(callerToken, abilityRecord, callback1, nullptr);
    std::shared_ptr<BaseExtensionRecord> targetService = nullptr;
    connectManager->HandleActiveAbility(targetService, connectRecord);
    abilityRecord->abilityInfo_.extensionAbilityType = AppExecFwk::ExtensionAbilityType::UI_SERVICE;
    Want want;
    want.SetParam(PARAM_RESV_CALLER_APP_ID, std::string("app"));
    abilityRecord->SetWant(want);
    EXPECT_EQ(abilityRecord->GetWant().GetStringParam(PARAM_RESV_CALLER_APP_ID), "app");
    TAG_LOGI(AAFwkTag::TEST, "HandleActiveAbility_001 end");
}

/*
 * Feature: AbilityConnectManager
 * Function: HandleActiveAbility
 */
HWTEST_F(AbilityConnectManagerTest, HandleActiveAbility_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "HandleActiveAbility_002 start");
    std::shared_ptr<CommonExtensionManager> connectManager = std::make_shared<CommonExtensionManager>(0);
    EXPECT_NE(connectManager, nullptr);
    auto result = connectManager->ConnectAbilityLocked(abilityRequest_, callbackA_, nullptr);
    EXPECT_EQ(0, result);
    auto connectMap = connectManager->connectMap_;
    auto connectRecordList = connectMap.at(callbackA_->AsObject());
    EXPECT_EQ(1, static_cast<int>(connectRecordList.size()));
    auto elementName = abilityRequest_.want.GetElement();
    auto elementNameUri = elementName.GetURI();
    auto serviceMap = connectManager->GetServiceMap();
    std::shared_ptr<BaseExtensionRecord> abilityRecord = serviceMap.at(elementNameUri);
    //AbilityRequest abilityRequest;
    sptr<IRemoteObject> callerToken = abilityRecord->GetToken();
    Want want;
    want.SetParam(PARAM_RESV_CALLER_APP_ID, std::string("app"));
    abilityRecord->SetWant(want);
    OHOS::sptr<IAbilityConnection> callback1 = new AbilityConnectCallback();
    auto connectRecord = std::make_shared<ConnectionRecord>(callerToken, abilityRecord, callback1, nullptr);
    abilityRecord->abilityInfo_.extensionAbilityType = AppExecFwk::ExtensionAbilityType::UI;
    connectManager->HandleActiveAbility(abilityRecord, connectRecord);
    EXPECT_EQ(abilityRecord->GetWant().GetStringParam(PARAM_RESV_CALLER_APP_ID), "app"); // no remove signatureInfo
    TAG_LOGI(AAFwkTag::TEST, "HandleActiveAbility_002 end");
}

/*
 * Feature: AbilityConnectManager
 * Function: HandleActiveAbility
 */
HWTEST_F(AbilityConnectManagerTest, HandleActiveAbility_006, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "HandleActiveAbility_006 start");
    std::shared_ptr<AbilityConnectManager> connectManager = std::make_shared<AbilityConnectManager>(0);
    std::shared_ptr<BaseExtensionRecord> abilityRecord = serviceRecord_;
    ASSERT_NE(abilityRecord, nullptr);
    abilityRecord->abilityInfo_.type = AbilityType::EXTENSION;
    abilityRecord->abilityInfo_.extensionAbilityType = AppExecFwk::ExtensionAbilityType::AGENT;
    abilityRecord->SetAbilityState(AbilityState::ACTIVE);
    abilityRecord->isConnected = false;

    Want want = abilityRecord->GetWant();
    want.SetParam(AgentRuntime::AGENT_CARD_TYPE_KEY, static_cast<int32_t>(AgentRuntime::AgentCardType::LOW_CODE));
    want.SetParam(AgentRuntime::AGENTID_KEY, std::string("lowCodeAgent"));
    OHOS::sptr<IAbilityConnection> callback = new AbilityConnectCallback();
    auto connectRecord =
        std::make_shared<ConnectionRecord>(abilityRecord->GetToken(), abilityRecord, callback, nullptr);
    connectRecord->SetConnectWant(want);
    connectRecord->SetConnectState(ConnectionState::CONNECTING);

    connectManager->HandleActiveAbility(abilityRecord, connectRecord);
    EXPECT_TRUE(abilityRecord->isConnected);
    EXPECT_EQ(connectRecord->GetConnectState(), ConnectionState::CONNECTING);
    TAG_LOGI(AAFwkTag::TEST, "HandleActiveAbility_006 end");
}

/*
 * Feature: AbilityConnectManager
 * Function: HandleActiveAbility
 */
HWTEST_F(AbilityConnectManagerTest, HandleActiveAbility_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "HandleActiveAbility_003 start");
    std::shared_ptr<CommonExtensionManager> connectManager = std::make_shared<CommonExtensionManager>(0);
    EXPECT_NE(connectManager, nullptr);

    auto result = connectManager->ConnectAbilityLocked(abilityRequest_, callbackA_, nullptr);
    EXPECT_EQ(0, result);

    result = connectManager->ConnectAbilityLocked(abilityRequest_, callbackB_, nullptr);
    EXPECT_EQ(0, result);

    auto connectMap = connectManager->connectMap_;
    auto connectRecordList = connectMap.at(callbackA_->AsObject());
    EXPECT_EQ(1, static_cast<int>(connectRecordList.size()));

    connectRecordList = connectMap.at(callbackB_->AsObject());
    EXPECT_EQ(1, static_cast<int>(connectRecordList.size()));

    auto elementName = abilityRequest_.want.GetElement();
    std::string elementNameUri = elementName.GetURI();
    auto serviceMap = connectManager->GetServiceMap();
    auto abilityRecord = serviceMap.at(elementNameUri);
    connectRecordList = abilityRecord->GetConnectRecordList();
    EXPECT_EQ(2, static_cast<int>(connectRecordList.size()));
    //AbilityRequest abilityRequest;
    sptr<IRemoteObject> callerToken = abilityRecord->GetToken();
    OHOS::sptr<IAbilityConnection> callback1 = new AbilityConnectCallback();
    auto connectRecord = std::make_shared<ConnectionRecord>(callerToken, abilityRecord, callback1, nullptr);
    auto mockTaskHandler = MockTaskHandlerWrap::CreateQueueHandler("HandleActiveAbility003");
    EXPECT_CALL(*mockTaskHandler, SubmitTaskInner(_, _)).Times(0);
    connectManager->taskHandler_ = mockTaskHandler;
    abilityRecord->connRemoteObject_ = abilityRecord->GetToken();
    connectManager->HandleActiveAbility(abilityRecord, connectRecord);
    TAG_LOGI(AAFwkTag::TEST, "HandleActiveAbility_003 end");
}

/*
 * Feature: AbilityConnectManager
 * Function: HandleRestartResidentTask
 */
HWTEST_F(AbilityConnectManagerTest, HandleRestartResidentTask_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "HandleRestartResidentTask_001 start");
    std::shared_ptr<AbilityConnectManager> connectManager = std::make_shared<AbilityConnectManager>(0);
    EXPECT_NE(connectManager, nullptr);

    std::string deviceName = "device";
    std::string abilityName = "TestAbility";
    std::string appName = "hiservcie";
    std::string bundleName = "com.example.bundle";
    std::string moduleName = "entry";
    abilityRequest_ = GenerateAbilityRequest(deviceName, abilityName, appName, bundleName, moduleName);
    serviceRecord_ = BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest_);

    connectManager->HandleRestartResidentTask(abilityRequest_);
    ASSERT_TRUE(connectManager->restartResidentTaskList_.empty());
    TAG_LOGI(AAFwkTag::TEST, "HandleRestartResidentTask_001 end");
}

/*
 * Feature: AbilityConnectManager
 * Function: HandleRestartResidentTask
 */
HWTEST_F(AbilityConnectManagerTest, HandleRestartResidentTask_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "HandleRestartResidentTask_002 start");
    std::shared_ptr<AbilityConnectManager> connectManager = std::make_shared<AbilityConnectManager>(0);
    EXPECT_NE(connectManager, nullptr);

    connectManager->restartResidentTaskList_.clear();
    std::string deviceName = "device";
    std::string abilityName = "TestAbility";
    std::string appName = "hiservcie";
    std::string bundleName = "com.example.bundle";
    std::string moduleName = "entry";
    abilityRequest_ = GenerateAbilityRequest(deviceName, abilityName, appName, bundleName, moduleName);
    serviceRecord_ = BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest_);

    connectManager->HandleRestartResidentTask(abilityRequest_);
    ASSERT_EQ(connectManager->restartResidentTaskList_.size(), 0);
    TAG_LOGI(AAFwkTag::TEST, "HandleRestartResidentTask_002 end");
}

/*
 * Feature: AbilityConnectManager
 * Function: IsNeedToRestart
 */
HWTEST_F(AbilityConnectManagerTest, IsNeedToRestart_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "IsNeedToRestart_001 start");
    std::shared_ptr<AbilityConnectManager> connectManager = std::make_shared<AbilityConnectManager>(0);
    EXPECT_NE(connectManager, nullptr);

    std::shared_ptr<BaseExtensionRecord> abilityRecord = serviceRecord_;
    bool result = connectManager->IsNeedToRestart(abilityRecord, "com.example.bundle", "TestAbility");
    ASSERT_TRUE(result);
    TAG_LOGI(AAFwkTag::TEST, "IsNeedToRestart_001 end");
}

/*
 * Feature: AbilityConnectManager
 * Function: IsStrictMode
 */
HWTEST_F(AbilityConnectManagerTest, IsStrictMode_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "IsStrictMode_001 start");
    std::shared_ptr<AbilityConnectManager> connectManager = std::make_shared<AbilityConnectManager>(0);
    EXPECT_NE(connectManager, nullptr);

    bool result = connectManager->IsStrictMode(nullptr);
    ASSERT_FALSE(result);
    TAG_LOGI(AAFwkTag::TEST, "IsStrictMode_001 end");
}

/*
 * Feature: AbilityConnectManager
 * Function: IsStrictMode
 */
HWTEST_F(AbilityConnectManagerTest, IsStrictMode_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "IsStrictMode_002 start");
    std::shared_ptr<AbilityConnectManager> connectManager = std::make_shared<AbilityConnectManager>(0);
    EXPECT_NE(connectManager, nullptr);

    std::shared_ptr<BaseExtensionRecord> abilityRecord = serviceRecord_;
    abilityRecord->abilityInfo_.extensionAbilityType = AppExecFwk::ExtensionAbilityType::INPUTMETHOD;
    Want want;
    want.SetParam("strictMode", true);
    abilityRecord->SetWant(want);

    bool result = connectManager->IsStrictMode(abilityRecord);
    ASSERT_TRUE(result);
    TAG_LOGI(AAFwkTag::TEST, "IsStrictMode_002 end");
}

/*
 * Feature: AbilityConnectManager
 * Function: TerminateAbilityInner
 */
HWTEST_F(AbilityConnectManagerTest, TerminateAbilityInner_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "TerminateAbilityInner_001 start");
    std::shared_ptr<AbilityConnectManager> connectManager = std::make_shared<AbilityConnectManager>(0);
    EXPECT_NE(connectManager, nullptr);

    std::shared_ptr<BaseExtensionRecord> abilityRecord = BaseExtensionRecord::CreateBaseExtensionRecord(
        abilityRequest_);
    auto abilityInfo = abilityRecord->GetAbilityInfo();
    abilityRecord->abilityInfo_.extensionAbilityType = AppExecFwk::ExtensionAbilityType::UI;
    auto result = ConnectManager()->TerminateAbilityInner(abilityRecord->GetToken());
    EXPECT_EQ(ERR_CONNECT_MANAGER_NULL_ABILITY_RECORD, result);
    TAG_LOGI(AAFwkTag::TEST, "TerminateAbilityInner_001 end");
}

/*
 * Feature: AbilityConnectManager
 * Function: TerminateAbilityInner
 */
HWTEST_F(AbilityConnectManagerTest, TerminateAbilityInner_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "TerminateAbilityInner_002 start");
    std::shared_ptr<AbilityConnectManager> connectManager = std::make_shared<AbilityConnectManager>(0);
    EXPECT_NE(connectManager, nullptr);

    std::shared_ptr<BaseExtensionRecord> abilityRecord = serviceRecord_;
    auto abilityInfo = abilityRecord->GetAbilityInfo();
    abilityRecord->abilityInfo_.extensionAbilityType = AppExecFwk::ExtensionAbilityType::UI;
    OHOS::sptr<IAbilityConnection> callback = new AbilityConnectCallback();
    std::shared_ptr<ConnectionRecord> connectionRe =
        std::make_shared<ConnectionRecord>(abilityRecord->GetToken(), abilityRecord, callback, nullptr);
    EXPECT_NE(connectionRe, nullptr);

    abilityRecord->currentState_ = AbilityState::FOREGROUND;
    abilityRecord->connRecordList_.push_back(connectionRe);
    auto result = connectManager->TerminateAbilityInner(abilityRecord->GetToken());
    EXPECT_EQ(ERR_CONNECT_MANAGER_NULL_ABILITY_RECORD, result);
    TAG_LOGI(AAFwkTag::TEST, "TerminateAbilityInner_002 end");
}

/*
 * Feature: AbilityConnectManager
 * Function: TerminateAbilityInner
 */
HWTEST_F(AbilityConnectManagerTest, TerminateAbilityInner_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "TerminateAbilityInner_003 start");
    std::shared_ptr<AbilityConnectManager> connectManager = std::make_shared<AbilityConnectManager>(0);
    EXPECT_NE(connectManager, nullptr);

    std::shared_ptr<BaseExtensionRecord> abilityRecord = serviceRecord_;
    auto abilityInfo = abilityRecord->GetAbilityInfo();
    abilityRecord->abilityInfo_.extensionAbilityType = AppExecFwk::ExtensionAbilityType::UI;
    abilityRecord->connRecordList_.clear();
    auto result = connectManager->TerminateAbilityInner(abilityRecord->GetToken());
    EXPECT_EQ(ERR_CONNECT_MANAGER_NULL_ABILITY_RECORD, result);
    TAG_LOGI(AAFwkTag::TEST, "TerminateAbilityInner_003 end");
}

/*
 * Feature: AbilityConnectManager
 * Function: IsWindowExtensionFocused
 */
HWTEST_F(AbilityConnectManagerTest, IsWindowExtensionFocused_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "IsWindowExtensionFocused_001 start");
    std::shared_ptr<AbilityConnectManager> connectManager = std::make_shared<AbilityConnectManager>(0);
    ASSERT_NE(connectManager, nullptr);

    uint32_t extensionTokenId = 12345;
    std::shared_ptr<BaseExtensionRecord> abilityRecord = serviceRecord_;
    sptr<IRemoteObject> focusToken = abilityRecord->GetToken();
    ASSERT_NE(focusToken, nullptr);
    
    sptr<SessionInfo> sessionInfo = MockSessionInfo(1);
    sessionInfo->callerToken = focusToken;
    connectManager->windowExtensionMap_.emplace(focusToken, std::make_pair(extensionTokenId, sessionInfo));

    bool result = connectManager->IsWindowExtensionFocused(extensionTokenId, focusToken);
    EXPECT_TRUE(result);

    uint32_t invalidTokenId = 54321;
    result = connectManager->IsWindowExtensionFocused(invalidTokenId, focusToken);
    EXPECT_FALSE(result);
    TAG_LOGI(AAFwkTag::TEST, "IsWindowExtensionFocused_001 end");
}

/*
 * Feature: AbilityConnectManager
 * Function: IsWindowExtensionFocused
 */
HWTEST_F(AbilityConnectManagerTest, IsWindowExtensionFocused_0002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "IsWindowExtensionFocused_002 start");
    std::shared_ptr<AbilityConnectManager> connectManager = std::make_shared<AbilityConnectManager>(0);
    ASSERT_NE(connectManager, nullptr);

    uint32_t extensionTokenId = 12345;
    std::shared_ptr<BaseExtensionRecord> abilityRecord = serviceRecord_;
    sptr<IRemoteObject> focusToken = abilityRecord->GetToken();
    ASSERT_NE(focusToken, nullptr);
    
    sptr<SessionInfo> sessionInfo = MockSessionInfo(1);
    sessionInfo->callerToken = focusToken;
    connectManager->windowExtensionMap_.emplace(focusToken, std::make_pair(extensionTokenId, nullptr));
    
    bool result = connectManager->IsWindowExtensionFocused(extensionTokenId, focusToken);
    EXPECT_FALSE(result);
    TAG_LOGI(AAFwkTag::TEST, "IsWindowExtensionFocused_002 end");
}

/*
 * Feature: AbilityConnectManager
 * Function: NeedExtensionControl
 */
HWTEST_F(AbilityConnectManagerTest, NeedExtensionControl_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "NeedExtensionControl_001 start");
    std::shared_ptr<AbilityConnectManager> connectManager = std::make_shared<AbilityConnectManager>(0);
    EXPECT_NE(connectManager, nullptr);
    
    std::shared_ptr<BaseExtensionRecord> nullRecord = nullptr;
    bool result = connectManager->NeedExtensionControl(nullRecord);
    EXPECT_FALSE(result);

    std::shared_ptr<BaseExtensionRecord> abilityRecord = serviceRecord_;
    abilityRecord->abilityInfo_.extensionAbilityType = AppExecFwk::ExtensionAbilityType::SERVICE;
    result = connectManager->NeedExtensionControl(abilityRecord);
    EXPECT_FALSE(result);

    abilityRecord->abilityInfo_.extensionAbilityType = AppExecFwk::ExtensionAbilityType::DATASHARE;
    result = connectManager->NeedExtensionControl(abilityRecord);
    EXPECT_FALSE(result);
    TAG_LOGI(AAFwkTag::TEST, "NeedExtensionControl_001 end");
}

/*
 * Feature: AbilityConnectManager
 * Function: NeedExtensionControl
 */
HWTEST_F(AbilityConnectManagerTest, NeedExtensionControl_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "NeedExtensionControl_002 start");
    std::shared_ptr<AbilityConnectManager> connectManager = std::make_shared<AbilityConnectManager>(0);
    EXPECT_NE(connectManager, nullptr);

    std::shared_ptr<BaseExtensionRecord> abilityRecord = serviceRecord_;
    abilityRecord->abilityInfo_.extensionAbilityType = AppExecFwk::ExtensionAbilityType::FORM;
    abilityRecord->SetCustomProcessFlag("");
    bool result = connectManager->NeedExtensionControl(abilityRecord);
    EXPECT_TRUE(result);

    abilityRecord->SetCustomProcessFlag("Test customProces");
    uint32_t extensionProcessMode = 0;
    abilityRecord->SetExtensionProcessMode(extensionProcessMode);
    result = connectManager->NeedExtensionControl(abilityRecord);
    EXPECT_FALSE(result);
    TAG_LOGI(AAFwkTag::TEST, "NeedExtensionControl_002 end");
}

/*
 * Feature: AbilityConnectManager
 * Function: StartAbilityLocked
 */
HWTEST_F(AbilityConnectManagerTest, StartAbilityLocked_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartAbilityLocked_001 start");
    std::shared_ptr<AbilityConnectManager> connectManager = std::make_shared<AbilityConnectManager>(0);
    EXPECT_NE(connectManager, nullptr);

    std::shared_ptr<BaseExtensionRecord> abilityRecord = serviceRecord_;
    sptr<IRemoteObject> focusToken = abilityRecord->GetToken();
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.deviceId = "id";
    abilityRequest.abilityInfo.bundleName = "bundle";
    abilityRequest.abilityInfo.name = "name";
    abilityRequest.abilityInfo.moduleName = "module";
    abilityRequest.abilityInfo.extensionAbilityType = ExtensionAbilityType::UI;
    std::string stringUri = "id/bundle/module/name";
    abilityRequest.callerToken = abilityRecord->GetToken();
    AppExecFwk::ElementName element(abilityRequest.abilityInfo.deviceId, abilityRequest.abilityInfo.bundleName,
        abilityRequest.abilityInfo.name, abilityRequest.abilityInfo.moduleName);
    EXPECT_EQ(element.GetURI(), stringUri);

    int result = connectManager->StartAbilityLocked(abilityRequest);
    EXPECT_EQ(result, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "StartAbilityLocked_001 end");
}

/*
 * Feature: AbilityConnectManager
 * Function: HandleActiveAbility
 */
HWTEST_F(AbilityConnectManagerTest, HandleActiveAbility_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "HandleActiveAbility_004 start");
    std::shared_ptr<CommonExtensionManager> connectManager = std::make_shared<CommonExtensionManager>(0);
    EXPECT_NE(connectManager, nullptr);

    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.extensionAbilityType = AppExecFwk::ExtensionAbilityType::WORK_SCHEDULER;
    abilityRequest.abilityInfo.bundleName = "com.example.test";
    abilityRequest.abilityInfo.name = "TestAbility";
    std::shared_ptr<BaseExtensionRecord> targetService  = BaseExtensionRecord::CreateBaseExtensionRecord(
        abilityRequest);
    Want want;
    want.SetParam(PARAM_RESV_CALLER_APP_ID, std::string("app"));
    targetService->SetWant(want);
    targetService->abilityInfo_.extensionAbilityType = ExtensionAbilityType::UI_SERVICE;
    std::shared_ptr<ConnectionRecord> connectRecord = nullptr;

    connectManager->HandleActiveAbility(targetService, connectRecord);
    EXPECT_NE(targetService->GetWant().GetStringParam(PARAM_RESV_CALLER_APP_ID), "app");
    TAG_LOGI(AAFwkTag::TEST, "HandleActiveAbility_004 end");
}

/*
 * Feature: AbilityConnectManager
 * Function: HandleActiveAbility
 */
HWTEST_F(AbilityConnectManagerTest, HandleActiveAbility_005, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "HandleActiveAbility_005 start");
    std::shared_ptr<CommonExtensionManager> connectManager = std::make_shared<CommonExtensionManager>(0);
    EXPECT_NE(connectManager, nullptr);

    std::shared_ptr<BaseExtensionRecord> abilityRecord = serviceRecord_;
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.extensionAbilityType = AppExecFwk::ExtensionAbilityType::UI_SERVICE;
    abilityRequest.abilityInfo.bundleName = "com.example.test";
    abilityRequest.abilityInfo.name = "TestAbility";
    std::shared_ptr<BaseExtensionRecord> targetService  = BaseExtensionRecord::CreateBaseExtensionRecord(
        abilityRequest);
    Want want;
    want.SetParam(PARAM_RESV_CALLER_APP_ID, std::string("app"));
    targetService->SetWant(want);
    sptr<IRemoteObject> callerToken = abilityRecord->GetToken();
    OHOS::sptr<IAbilityConnection> callback1 = new AbilityConnectCallback();
    std::shared_ptr<ConnectionRecord> connectRecord =
        std::make_shared<ConnectionRecord>(callerToken, abilityRecord, callback1, nullptr);

    connectManager->HandleActiveAbility(targetService, connectRecord);
    EXPECT_NE(targetService->GetWant().GetStringParam(PARAM_RESV_CALLER_APP_ID), "app");
    TAG_LOGI(AAFwkTag::TEST, "HandleActiveAbility_005 end");
}

/*
 * Feature: AbilityConnectManager
 * Function: SuspendExtensionAbilityLocked
 */
HWTEST_F(AbilityConnectManagerTest, SuspendExtensionAbilityLocked_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SuspendExtensionAbilityLocked start");
    std::shared_ptr<CommonExtensionManager> connectManager = std::make_shared<CommonExtensionManager>(0);
    EXPECT_NE(connectManager, nullptr);

    auto result = connectManager->SuspendExtensionAbilityLocked(callbackA_);
    EXPECT_EQ(result, OHOS::AAFwk::CONNECTION_NOT_EXIST);

    result = connectManager->ConnectAbilityLocked(abilityRequest_, callbackA_, nullptr);
    EXPECT_EQ(0, result);

    result = connectManager->SuspendExtensionAbilityLocked(callbackA_);
    EXPECT_EQ(result, OHOS::AAFwk::INVALID_CONNECTION_STATE);

    auto list = connectManager->GetConnectRecordListByCallback(callbackA_);
    EXPECT_EQ(static_cast<int>(list.size()), 1);

    for (auto& it : list) {
        it->SetConnectState(ConnectionState::CONNECTED);
    }

    auto result3 = connectManager->SuspendExtensionAbilityLocked(callbackA_);
    EXPECT_EQ(result3, OHOS::ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "SuspendExtensionAbilityLocked end");
}

/*
 * Feature: AbilityConnectManager
 * Function: ResumeExtensionAbilityLocked
 */
HWTEST_F(AbilityConnectManagerTest, ResumeExtensionAbilityLocked_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ResumeExtensionAbilityLocked start");
    std::shared_ptr<CommonExtensionManager> connectManager = std::make_shared<CommonExtensionManager>(0);
    EXPECT_NE(connectManager, nullptr);

    auto result = connectManager->ConnectAbilityLocked(abilityRequest_, callbackA_, nullptr);
    EXPECT_EQ(0, result);

    result = connectManager->ResumeExtensionAbilityLocked(callbackA_);
    EXPECT_EQ(result, OHOS::AAFwk::INVALID_CONNECTION_STATE);

    auto list = connectManager->GetConnectRecordListByCallback(callbackA_);
    EXPECT_EQ(static_cast<int>(list.size()), 1);

    for (auto& it : list) {
        it->SetConnectState(ConnectionState::CONNECTED);
    }
    
    connectManager->SuspendExtensionAbilityLocked(callbackA_);
    result = connectManager->ResumeExtensionAbilityLocked(callbackA_);
    EXPECT_EQ(result, OHOS::ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "ResumeExtensionAbilityLocked end");
}

/*
 * Feature: AbilityConnectManager
 * Function: GetLoadTimeout
 */
HWTEST_F(AbilityConnectManagerTest, GetLoadTimeout_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetLoadTimeout_001 start");
    std::shared_ptr<CommonExtensionManager> connectManager = std::make_shared<CommonExtensionManager>(0);
    EXPECT_NE(connectManager, nullptr);

    auto timeout = connectManager->GetLoadTimeout(0);
    EXPECT_NE(timeout, 0);
    TAG_LOGI(AAFwkTag::TEST, "GetLoadTimeout_001 end");
}

/*
 * Feature: AbilityConnectManager
 * Function: GetLoadTimeout
 */
HWTEST_F(AbilityConnectManagerTest, GetLoadTimeout_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetLoadTimeout_002 start");
    std::shared_ptr<CommonExtensionManager> connectManager = std::make_shared<CommonExtensionManager>(0);
    EXPECT_NE(connectManager, nullptr);

    auto timeout = connectManager->GetLoadTimeout(-1);
    EXPECT_NE(timeout, -1);

    timeout = connectManager->GetLoadTimeout(100);
    EXPECT_NE(timeout, 100);
    TAG_LOGI(AAFwkTag::TEST, "GetLoadTimeout_002 end");
}

/*
 * Feature: AbilityConnectManager
 * Function: GetLoadTimeout
 */
HWTEST_F(AbilityConnectManagerTest, GetLoadTimeout_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetLoadTimeout_003 start");
    std::shared_ptr<CommonExtensionManager> connectManager = std::make_shared<CommonExtensionManager>(0);
    EXPECT_NE(connectManager, nullptr);

    auto timeout = connectManager->GetLoadTimeout(10);
    EXPECT_EQ(timeout, 10);
    TAG_LOGI(AAFwkTag::TEST, "GetLoadTimeout_003 end");
}

/*
* Feature: AbilityConnectManager
* Function: GetOrCreateServiceRecord
*/
HWTEST_F(AbilityConnectManagerTest, GetOrCreateServiceRecord_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetOrCreateServiceRecord_001 start");
    auto connectManager = std::make_shared<AbilityConnectManager>(0);
    EXPECT_NE(connectManager, nullptr);

    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.extensionAbilityType = AppExecFwk::ExtensionAbilityType::UI_SERVICE;
    abilityRequest.abilityInfo.bundleName = "com.example.test";
    abilityRequest.abilityInfo.name = "TestAbility";

    std::shared_ptr<BaseExtensionRecord> serviceRecord = nullptr;
    bool isLoadedAbility = false;
    connectManager->GetOrCreateServiceRecord(abilityRequest, false, serviceRecord, isLoadedAbility);

    ASSERT_NE(serviceRecord, nullptr);
    EXPECT_EQ(serviceRecord->GetAbilityInfo().name, "TestAbility");
    TAG_LOGI(AAFwkTag::TEST, "GetOrCreateServiceRecord_001 end");
}

/*
  * Feature: AbilityConnectManager
  * Function: SetServiceAfterNewCreate
  */
 HWTEST_F(AbilityConnectManagerTest, SetServiceAfterNewCreate_001, TestSize.Level1)
 {
     TAG_LOGI(AAFwkTag::TEST, "SetServiceAfterNewCreate_001 start");
     auto connectManager = std::make_shared<AbilityConnectManager>(0);
     std::string deviceName = "device";
     std::string abilityName = AbilityConfig::LAUNCHER_ABILITY_NAME;
     std::string appName = "hiservcie";
     std::string bundleName = "com.ix.hiservcie";
     std::string moduleName = "entry";
     auto abilityRequest = GenerateAbilityRequest(deviceName, abilityName, appName, bundleName, moduleName);
     auto targetService = BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);


     connectManager->SetServiceAfterNewCreate(abilityRequest, *targetService);
     EXPECT_TRUE(targetService->IsLauncherRoot());
 }


/*
* Feature: AbilityConnectManager
* Function: SetServiceAfterNewCreate
*/
HWTEST_F(AbilityConnectManagerTest, SetServiceAfterNewCreate_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SetServiceAfterNewCreate_002 start");
    auto connectManager = std::make_shared<AbilityConnectManager>(0);
    std::string deviceName = "device";
    std::string abilityName = "ServiceAbility";
    std::string appName = "hiservcie";
    std::string bundleName = "com.ix.hiservcie";
    std::string moduleName = "entry";
    auto abilityRequest = GenerateAbilityRequest(deviceName, abilityName, appName, bundleName, moduleName);
    auto targetService = BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    targetService->SetKeepAliveBundle(true);


    connectManager->SetServiceAfterNewCreate(abilityRequest, *targetService);
    EXPECT_FALSE(targetService->IsLauncherRoot());
}


/*
* Feature: AbilityConnectManager
* Function: SetServiceAfterNewCreate
*/
HWTEST_F(AbilityConnectManagerTest, SetServiceAfterNewCreate_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SetServiceAfterNewCreate_003 start");
    auto connectManager = std::make_shared<AbilityConnectManager>(0);
    std::string deviceName = "device";
    std::string abilityName = AbilityConfig::SCENEBOARD_ABILITY_NAME;
    std::string appName = "hiservcie";
    std::string bundleName = AbilityConfig::SCENEBOARD_BUNDLE_NAME;
    std::string moduleName = "entry";
    auto abilityRequest = GenerateAbilityRequest(deviceName, abilityName, appName, bundleName, moduleName);
    auto targetService = BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    abilityRequest.appInfo.accessTokenId = FAKE_TOKENID;
    connectManager->SetServiceAfterNewCreate(abilityRequest, *targetService);
    EXPECT_EQ(connectManager->sceneBoardTokenId_, FAKE_TOKENID);
}

/*
* Feature: AbilityConnectManager
* Function: SetServiceAfterNewCreate
*/
HWTEST_F(AbilityConnectManagerTest, SetServiceAfterNewCreate_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SetServiceAfterNewCreate_004 start");
    auto connectManager = std::make_shared<AbilityConnectManager>(0);
    std::string deviceName = "device";
    std::string abilityName = AbilityConfig::SCENEBOARD_ABILITY_NAME;
    std::string appName = "hiservcie";
    std::string bundleName = AbilityConfig::SCENEBOARD_BUNDLE_NAME;
    std::string moduleName = "entry";
    auto abilityRequest = GenerateAbilityRequest(deviceName, abilityName, appName, bundleName, moduleName);
    abilityRequest.restart = true;
    auto targetService = BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    abilityRequest.appInfo.accessTokenId = FAKE_TOKENID;
    connectManager->SetServiceAfterNewCreate(abilityRequest, *targetService);
    EXPECT_EQ(connectManager->sceneBoardTokenId_, FAKE_TOKENID);
}

/*
* Feature: AbilityConnectManager
* Function: SetServiceAfterNewCreate
*/
HWTEST_F(AbilityConnectManagerTest, SetServiceAfterNewCreate_005, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SetServiceAfterNewCreate_005 start");
    auto connectManager = std::make_shared<AbilityConnectManager>(0);
    std::string deviceName = "device";
    std::string abilityName = "ServiceAbility";
    std::string appName = "hiservcie";
    std::string bundleName = "com.ix.hiservcie";
    std::string moduleName = "entry";
    auto abilityRequest = GenerateAbilityRequest(deviceName, abilityName, appName, bundleName, moduleName);
    abilityRequest.restart = true;
    auto targetService = BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    targetService->SetKeepAliveBundle(true);


    connectManager->SetServiceAfterNewCreate(abilityRequest, *targetService);
    EXPECT_FALSE(targetService->IsLauncherRoot());
}
}  // namespace AAFwk
}  // namespace OHOS
