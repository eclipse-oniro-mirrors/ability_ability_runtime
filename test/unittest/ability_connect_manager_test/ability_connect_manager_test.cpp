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
#include <memory>

#define private public
#define protected public
#include "ability_connect_manager.h"
#include "common_extension_manager.h"
#include "ui_extension_ability_manager.h"
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
#include "param.h"
#include "sa_mgr_client.h"
#include "system_ability_definition.h"
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
    const int32_t SLEEP_TIME = 10000;
}

namespace OHOS {
namespace AAFwk {
template<typename F>
static void WaitUntilTaskCalled(const F& f, const std::shared_ptr<TaskHandlerWrap>& handler,
    std::atomic<bool>& taskCalled)
{
    const uint32_t maxRetryCount = 1000;
    const uint32_t sleepTime = 1000;
    uint32_t count = 0;
    if (handler->SubmitTask(f)) {
        while (!taskCalled.load()) {
            ++count;
            // if delay more than 1 second, break
            if (count >= maxRetryCount) {
                break;
            }
            usleep(sleepTime);
        }
    }
}

static void WaitUntilTaskDone(const std::shared_ptr<TaskHandlerWrap>& handler)
{
    std::atomic<bool> taskCalled(false);
    auto f = [&taskCalled]() { taskCalled.store(true); };
    WaitUntilTaskCalled(f, handler, taskCalled);
}

class AbilityConnectManagerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    AbilityConnectManager* ConnectManager() const;
    CommonExtensionManager* GetCommonExtensionManager() const;
    std::shared_ptr<MockTaskHandlerWrap> TaskHandler() const;
    std::shared_ptr<EventHandlerWrap> EventHandler() const;

    AbilityRequest GenerateAbilityRequest(const std::string& deviceName, const std::string& abilityName,
        const std::string& appName, const std::string& bundleName, const std::string& moduleName);

    static constexpr int TEST_WAIT_TIME = 1000000;

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
    std::shared_ptr<CommonExtensionManager> commonExtensionManager_;
    std::shared_ptr<EventHandlerWrap> eventHandler_;
};

class TestAbilityConnectManager : public AbilityConnectManager {
public:
    explicit TestAbilityConnectManager(int userId) : AbilityConnectManager(userId) {}

    int32_t UnRegisterPreloadUIExtensionHostClient(int32_t callerPid) override
    {
        lastUnregisterPid_ = callerPid;
        return ERR_OK;
    }

    int32_t lastUnregisterPid_ = -1;
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
{
    std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_TIME));
}

void AbilityConnectManagerTest::SetUp(void)
{
    connectManager_ = std::make_unique<AbilityConnectManager>(0);
    commonExtensionManager_ = std::make_unique<CommonExtensionManager>(0);
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
    // mock bms
    OHOS::DelayedSingleton<SaMgrClient>::GetInstance()->RegisterSystemAbility(
        OHOS::BUNDLE_MGR_SERVICE_SYS_ABILITY_ID, new BundleMgrService());
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

CommonExtensionManager* AbilityConnectManagerTest::GetCommonExtensionManager() const
{
    return commonExtensionManager_.get();
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
 * Function: StartAbility
 * SubFunction: NA
 * FunctionPoints: StartAbility
 * EnvConditions:NA
 * CaseDescription: Verify the normal process of startability
 */
HWTEST_F(AbilityConnectManagerTest, AAFWK_Connect_Service_001, TestSize.Level1)
{
    GetCommonExtensionManager()->SetTaskHandler(TaskHandler());
    GetCommonExtensionManager()->SetEventHandler(EventHandler());

    auto result = GetCommonExtensionManager()->StartAbility(abilityRequest_);
    EXPECT_EQ(OHOS::ERR_OK, result);
    WaitUntilTaskDone(TaskHandler());

    auto elementName = abilityRequest_.want.GetElement().GetURI();
    auto service = GetCommonExtensionManager()->GetServiceRecordByElementName(elementName);
    EXPECT_NE(service, nullptr);
    EXPECT_EQ(static_cast<int>(GetCommonExtensionManager()->GetServiceMap().size()), 1);

    service->SetAbilityState(OHOS::AAFwk::AbilityState::ACTIVE);

    auto result1 = GetCommonExtensionManager()->StartAbility(abilityRequest_);
    WaitUntilTaskDone(TaskHandler());
    EXPECT_EQ(OHOS::ERR_OK, result1);
    EXPECT_EQ(static_cast<int>(GetCommonExtensionManager()->GetServiceMap().size()), 1);

    service->SetAbilityState(OHOS::AAFwk::AbilityState::ACTIVATING);
    auto result2 = GetCommonExtensionManager()->StartAbility(abilityRequest_);
    WaitUntilTaskDone(TaskHandler());
    EXPECT_EQ(OHOS::ERR_OK, result2);
    EXPECT_EQ(static_cast<int>(GetCommonExtensionManager()->GetServiceMap().size()), 1);
}

/*
 * Feature: AbilityConnectManager
 * Function: TerminateAbility
 * SubFunction: NA
 * FunctionPoints: StartAbility and TerminateAbility
 * EnvConditions:NA
 * CaseDescription: Verify the following:
 * 1.token is nullptr, terminate ability failed
 * 2.token is not nullptr, terminate ability success, and verify the status
 */
HWTEST_F(AbilityConnectManagerTest, AAFWK_Connect_Service_002, TestSize.Level1)
{
    GetCommonExtensionManager()->SetTaskHandler(TaskHandler());
    GetCommonExtensionManager()->SetEventHandler(EventHandler());

    auto result = GetCommonExtensionManager()->StartAbility(abilityRequest_);
    EXPECT_EQ(OHOS::ERR_OK, result);
    WaitUntilTaskDone(TaskHandler());

    OHOS::sptr<OHOS::IRemoteObject> nullToken = nullptr;
    auto result1 = GetCommonExtensionManager()->TerminateAbility(nullToken);
    EXPECT_EQ(ERR_CONNECT_MANAGER_NULL_ABILITY_RECORD, result1);

    auto elementName = abilityRequest_.want.GetElement().GetURI();
    auto service = GetCommonExtensionManager()->GetServiceRecordByElementName(elementName);
    EXPECT_NE(service, nullptr);

    auto result2 = GetCommonExtensionManager()->TerminateAbility(service->GetToken());
    WaitUntilTaskDone(TaskHandler());
    EXPECT_EQ(OHOS::ERR_OK, result2);
    EXPECT_EQ(service->GetAbilityState(), TERMINATING);
}

/*
 * Feature: AbilityConnectManager
 * Function: TerminateAbility
 * SubFunction: NA
 * FunctionPoints: StartAbility and TerminateAbility
 * EnvConditions:NA
 * CaseDescription: Verify ability is terminating, terminate ability success
 */
HWTEST_F(AbilityConnectManagerTest, AAFWK_Connect_Service_003, TestSize.Level1)
{
    GetCommonExtensionManager()->SetTaskHandler(TaskHandler());
    GetCommonExtensionManager()->SetEventHandler(EventHandler());

    auto result = GetCommonExtensionManager()->StartAbility(abilityRequest_);
    EXPECT_EQ(OHOS::ERR_OK, result);
    WaitUntilTaskDone(TaskHandler());

    auto elementName = abilityRequest_.want.GetElement().GetURI();
    auto service = GetCommonExtensionManager()->GetServiceRecordByElementName(elementName);
    EXPECT_NE(service, nullptr);

    service->SetTerminatingState();
    auto result1 = GetCommonExtensionManager()->TerminateAbility(service->GetToken());
    WaitUntilTaskDone(TaskHandler());
    EXPECT_EQ(OHOS::ERR_OK, result1);
    EXPECT_NE(service->GetAbilityState(), TERMINATING);
}

/*
 * Feature: AbilityConnectManager
 * Function: TerminateAbility
 * SubFunction: NA
 * FunctionPoints: StartAbility and TerminateAbility
 * EnvConditions: NA
 * CaseDescription: Verify service is connected, terminate ability failed
 */
HWTEST_F(AbilityConnectManagerTest, AAFWK_Connect_Service_004, TestSize.Level1)
{
    GetCommonExtensionManager()->SetTaskHandler(TaskHandler());
    GetCommonExtensionManager()->SetEventHandler(EventHandler());

    auto result = GetCommonExtensionManager()->StartAbility(abilityRequest_);
    EXPECT_EQ(OHOS::ERR_OK, result);
    WaitUntilTaskDone(TaskHandler());

    auto result1 = GetCommonExtensionManager()->ConnectAbilityLocked(abilityRequest_, callbackA_, nullptr);
    EXPECT_EQ(0, result1);

    auto elementName = abilityRequest_.want.GetElement().GetURI();
    auto service = GetCommonExtensionManager()->GetServiceRecordByElementName(elementName);
    EXPECT_NE(service, nullptr);

    auto result2 = GetCommonExtensionManager()->TerminateAbility(service->GetToken());
    WaitUntilTaskDone(TaskHandler());
    EXPECT_EQ(0, result2);
    EXPECT_EQ(service->GetAbilityState(), TERMINATING);
}

/*
 * Feature: AbilityConnectManager
 * Function: StopServiceAbility
 * SubFunction: NA
 * FunctionPoints: StartAbility and StopServiceAbility
 * EnvConditions: NA
 * CaseDescription: Verify the following:
 * 1.token is nullptr, stop service ability failed
 * 2.token is not nullptr, stop service ability success, and verify the status
 */
HWTEST_F(AbilityConnectManagerTest, AAFWK_Connect_Service_005, TestSize.Level1)
{
    GetCommonExtensionManager()->SetTaskHandler(TaskHandler());
    GetCommonExtensionManager()->SetEventHandler(EventHandler());

    auto result = GetCommonExtensionManager()->StartAbility(abilityRequest_);
    EXPECT_EQ(OHOS::ERR_OK, result);
    WaitUntilTaskDone(TaskHandler());

    auto elementName = abilityRequest_.want.GetElement().GetURI();
    auto service = GetCommonExtensionManager()->GetServiceRecordByElementName(elementName);
    EXPECT_NE(service, nullptr);

    AbilityRequest otherRequest;
    auto result1 = GetCommonExtensionManager()->StopServiceAbility(otherRequest);
    WaitUntilTaskDone(TaskHandler());
    EXPECT_EQ(OHOS::ERR_INVALID_VALUE, result1);

    auto result2 = GetCommonExtensionManager()->StopServiceAbility(abilityRequest_);
    WaitUntilTaskDone(TaskHandler());
    EXPECT_EQ(OHOS::ERR_OK, result2);
    EXPECT_EQ(service->GetAbilityState(), TERMINATING);
}

/*
 * Feature: AbilityConnectManager
 * Function: StopServiceAbility
 * SubFunction: NA
 * FunctionPoints: StartAbility and StopServiceAbility
 * EnvConditions:NA
 * CaseDescription: Verify ability is terminating, stop service ability success
 */
HWTEST_F(AbilityConnectManagerTest, AAFWK_Connect_Service_006, TestSize.Level1)
{
    GetCommonExtensionManager()->SetTaskHandler(TaskHandler());
    GetCommonExtensionManager()->SetEventHandler(EventHandler());

    auto result = GetCommonExtensionManager()->StartAbility(abilityRequest_);
    EXPECT_EQ(OHOS::ERR_OK, result);
    WaitUntilTaskDone(TaskHandler());

    auto elementName = abilityRequest_.want.GetElement().GetURI();
    auto service = GetCommonExtensionManager()->GetServiceRecordByElementName(elementName);
    EXPECT_NE(service, nullptr);

    service->SetTerminatingState();
    auto result1 = GetCommonExtensionManager()->StopServiceAbility(abilityRequest_);
    WaitUntilTaskDone(TaskHandler());
    EXPECT_EQ(OHOS::ERR_OK, result1);
    EXPECT_NE(service->GetAbilityState(), TERMINATING);
}

/*
 * Feature: AbilityConnectManager
 * Function: StopServiceAbility
 * SubFunction: NA
 * FunctionPoints: StartAbility and StopServiceAbility
 * EnvConditions: NA
 * CaseDescription: Verify service is connected, stop service ability failed
 */
HWTEST_F(AbilityConnectManagerTest, AAFWK_Connect_Service_007, TestSize.Level1)
{
    GetCommonExtensionManager()->SetTaskHandler(TaskHandler());
    GetCommonExtensionManager()->SetEventHandler(EventHandler());

    auto result = GetCommonExtensionManager()->StartAbility(abilityRequest_);
    EXPECT_EQ(OHOS::ERR_OK, result);
    WaitUntilTaskDone(TaskHandler());

    auto result1 = GetCommonExtensionManager()->ConnectAbilityLocked(abilityRequest_, callbackA_, nullptr);
    EXPECT_EQ(0, result1);

    auto elementName = abilityRequest_.want.GetElement().GetURI();
    auto service = GetCommonExtensionManager()->GetServiceRecordByElementName(elementName);
    EXPECT_NE(service, nullptr);

    auto result2 = GetCommonExtensionManager()->StopServiceAbility(abilityRequest_);
    WaitUntilTaskDone(TaskHandler());
    EXPECT_EQ(0, result2);
    EXPECT_EQ(service->GetAbilityState(), TERMINATING);
}

/*
 * Feature: AbilityConnectManager
 * Function: ConnectAbilityLocked
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions:NA
 * CaseDescription: verify the scene of service not loaded and callback not bound.
 */
HWTEST_F(AbilityConnectManagerTest, AAFWK_Connect_Service_008, TestSize.Level1)
{
    int result = GetCommonExtensionManager()->ConnectAbilityLocked(abilityRequest_, callbackA_, nullptr);
    EXPECT_EQ(0, result);

    auto connectMap = GetCommonExtensionManager()->connectMap_;
    auto connectRecordList = connectMap.at(callbackA_->AsObject());
    EXPECT_EQ(1, static_cast<int>(connectRecordList.size()));

    auto elementName = abilityRequest_.want.GetElement();
    auto elementNameUri = elementName.GetURI();
    auto serviceMap = GetCommonExtensionManager()->GetServiceMap();
    auto abilityRecord = serviceMap.at(elementNameUri);
    connectRecordList = abilityRecord->GetConnectRecordList();
    EXPECT_EQ(1, static_cast<int>(connectRecordList.size()));
}

/*
 * Feature: AbilityConnectManager
 * Function: ConnectAbilityLocked
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions:NA
 * CaseDescription: verify the scene of service load ability's timeout.
 */
HWTEST_F(AbilityConnectManagerTest, AAFWK_Connect_Service_009, TestSize.Level1)
{
    GetCommonExtensionManager()->SetTaskHandler(TaskHandler());
    GetCommonExtensionManager()->SetEventHandler(EventHandler());
    int result = GetCommonExtensionManager()->ConnectAbilityLocked(abilityRequest_, callbackA_, nullptr);
    EXPECT_EQ(0, result);

    auto connectMap = GetCommonExtensionManager()->connectMap_;
    EXPECT_EQ(1, static_cast<int>(connectMap.size()));
    WaitUntilTaskDone(TaskHandler());
    usleep(TEST_WAIT_TIME);

    connectMap = GetCommonExtensionManager()->connectMap_;
    EXPECT_EQ(1, static_cast<int>(connectMap.size()));
}

/*
 * Feature: AbilityConnectManager
 * Function: ConnectAbilityLocked
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions:NA
 * CaseDescription: verify the scene of service loaded and callback not bound.
 */
HWTEST_F(AbilityConnectManagerTest, AAFWK_Connect_Service_010, TestSize.Level1)
{
    auto result = GetCommonExtensionManager()->ConnectAbilityLocked(abilityRequest_, callbackA_, nullptr);
    EXPECT_EQ(0, result);

    result = GetCommonExtensionManager()->ConnectAbilityLocked(abilityRequest_, callbackB_, nullptr);
    EXPECT_EQ(0, result);

    auto connectMap = GetCommonExtensionManager()->connectMap_;
    auto connectRecordList = connectMap.at(callbackA_->AsObject());
    EXPECT_EQ(1, static_cast<int>(connectRecordList.size()));

    connectRecordList = connectMap.at(callbackB_->AsObject());
    EXPECT_EQ(1, static_cast<int>(connectRecordList.size()));

    auto elementName = abilityRequest_.want.GetElement();
    std::string elementNameUri = elementName.GetURI();
    auto serviceMap = GetCommonExtensionManager()->GetServiceMap();
    auto abilityRecord = serviceMap.at(elementNameUri);
    connectRecordList = abilityRecord->GetConnectRecordList();
    EXPECT_EQ(2, static_cast<int>(connectRecordList.size()));
}

/*
 * Feature: AbilityConnectManager
 * Function: ConnectAbilityLocked
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions:NA
 * CaseDescription: verify the scene of service connect ability's timeout.
 */
HWTEST_F(AbilityConnectManagerTest, AAFWK_Connect_Service_011, TestSize.Level1)
{
    GetCommonExtensionManager()->SetTaskHandler(TaskHandler());
    GetCommonExtensionManager()->SetEventHandler(EventHandler());

    int result = GetCommonExtensionManager()->ConnectAbilityLocked(abilityRequest_, callbackA_, nullptr);
    auto elementName = abilityRequest_.want.GetElement();
    std::string elementNameUri = elementName.GetURI();
    auto serviceMap = GetCommonExtensionManager()->GetServiceMap();
    auto abilityRecord = serviceMap.at(elementNameUri);
    auto token = abilityRecord->GetToken();

    auto connectMap = GetCommonExtensionManager()->connectMap_;
    EXPECT_EQ(1, static_cast<int>(connectMap.size()));

    auto scheduler = new AbilityScheduler();
    GetCommonExtensionManager()->AttachAbilityThreadLocked(scheduler, token->AsObject());
    GetCommonExtensionManager()->AbilityTransitionDone(token->AsObject(), OHOS::AAFwk::AbilityState::INACTIVE);

    WaitUntilTaskDone(TaskHandler());
    usleep(TEST_WAIT_TIME);
    connectMap = GetCommonExtensionManager()->connectMap_;
    EXPECT_EQ(0, result);
    EXPECT_EQ(1, static_cast<int>(connectMap.size()));
}

/*
 * Feature: AbilityConnectManager
 * Function: ConnectAbilityLocked
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions:NA
 * CaseDescription: verify the scene of service loaded and callback bound.
 */
HWTEST_F(AbilityConnectManagerTest, AAFWK_Connect_Service_012, TestSize.Level1)
{
    auto result = GetCommonExtensionManager()->ConnectAbilityLocked(abilityRequest_, callbackA_, nullptr);
    EXPECT_EQ(0, result);

    result = GetCommonExtensionManager()->ConnectAbilityLocked(abilityRequest_, callbackA_, nullptr);
    EXPECT_EQ(0, result);

    auto connectMap = GetCommonExtensionManager()->connectMap_;
    auto connectRecordList = connectMap.at(callbackA_->AsObject());
    EXPECT_EQ(1, static_cast<int>(connectRecordList.size()));

    auto elementName = abilityRequest_.want.GetElement();
    std::string elementNameUri = elementName.GetURI();
    auto serviceMap = GetCommonExtensionManager()->GetServiceMap();
    auto abilityRecord = serviceMap.at(elementNameUri);
    connectRecordList = abilityRecord->GetConnectRecordList();
    EXPECT_EQ(1, static_cast<int>(connectRecordList.size()));
}

/*
 * Feature: AbilityConnectManager
 * Function: ConnectAbilityLocked
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions:NA
 * CaseDescription: verify the scene of service not loaded and callback bound.
 */
HWTEST_F(AbilityConnectManagerTest, AAFWK_Connect_Service_013, TestSize.Level1)
{
    int result = GetCommonExtensionManager()->ConnectAbilityLocked(abilityRequest_, callbackA_, nullptr);
    EXPECT_EQ(0, result);

    std::string deviceNameB = "device";
    std::string abilityNameB = "ServiceAbilityB";
    std::string appNameB = "hiservcieB";
    std::string bundleNameB = "com.ix.hiservcieB";
    std::string moduleNameB = "entry";
    auto abilityRequestB = GenerateAbilityRequest(deviceNameB, abilityNameB, appNameB, bundleNameB, moduleNameB);
    result = GetCommonExtensionManager()->ConnectAbilityLocked(abilityRequestB, callbackA_, nullptr);
    EXPECT_EQ(0, result);

    auto connectMap = GetCommonExtensionManager()->connectMap_;
    auto connectRecordList = connectMap.at(callbackA_->AsObject());
    EXPECT_EQ(2, static_cast<int>(connectRecordList.size()));

    auto elementName = abilityRequest_.want.GetElement();
    std::string elementNameUri = elementName.GetURI();
    auto serviceMap = GetCommonExtensionManager()->GetServiceMap();
    auto abilityRecord = serviceMap.at(elementNameUri);
    connectRecordList = abilityRecord->GetConnectRecordList();
    EXPECT_EQ(1, static_cast<int>(connectRecordList.size()));

    auto elementNameB = abilityRequest_.want.GetElement();
    std::string elementNameUriB = elementNameB.GetURI();
    abilityRecord = serviceMap.at(elementNameUriB);
    connectRecordList = abilityRecord->GetConnectRecordList();
    EXPECT_EQ(1, static_cast<int>(connectRecordList.size()));
}

/*
 * Feature: AbilityConnectManager
 * Function: ConnectAbilityLocked
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions:NA
 * CaseDescription: verify the scene of service loaded and callback bound, but service and callback was not associated.
 */
HWTEST_F(AbilityConnectManagerTest, AAFWK_Connect_Service_014, TestSize.Level1)
{
    auto result = GetCommonExtensionManager()->ConnectAbilityLocked(abilityRequest_, callbackA_, nullptr);
    EXPECT_EQ(0, result);

    std::string deviceNameB = "device";
    std::string abilityNameB = "ServiceAbilityB";
    std::string appNameB = "hiservcieB";
    std::string bundleNameB = "com.ix.hiservcieB";
    std::string moduleNameB = "entry";
    auto abilityRequestB = GenerateAbilityRequest(deviceNameB, abilityNameB, appNameB, bundleNameB, moduleNameB);
    result = GetCommonExtensionManager()->ConnectAbilityLocked(abilityRequestB, callbackB_, nullptr);
    EXPECT_EQ(0, result);

    GetCommonExtensionManager()->ConnectAbilityLocked(abilityRequest_, callbackB_, nullptr);
    auto connectMap = GetCommonExtensionManager()->connectMap_;
    auto connectRecordList = connectMap.at(callbackB_->AsObject());
    EXPECT_EQ(2, static_cast<int>(connectRecordList.size()));

    connectRecordList = connectMap.at(callbackA_->AsObject());
    EXPECT_EQ(1, static_cast<int>(connectRecordList.size()));

    auto elementName = abilityRequest_.want.GetElement();
    std::string elementNameUri = elementName.GetURI();
    auto serviceMap = GetCommonExtensionManager()->GetServiceMap();
    auto abilityRecord = serviceMap.at(elementNameUri);
    connectRecordList = abilityRecord->GetConnectRecordList();
    EXPECT_EQ(2, static_cast<int>(connectRecordList.size()));
}

/*
 * Feature: AbilityConnectManager
 * Function: AttachAbilityThreadLocked
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions:NA
 * CaseDescription: verify the AttachAbilityThreadLocked function when the parameter is null.
 */
HWTEST_F(AbilityConnectManagerTest, AAFWK_Connect_Service_015, TestSize.Level1)
{
    auto result = GetCommonExtensionManager()->AttachAbilityThreadLocked(nullptr, nullptr);
    EXPECT_EQ(OHOS::ERR_INVALID_VALUE, result);
}

/*
 * Feature: AbilityConnectManager
 * Function: ScheduleConnectAbilityDoneLocked
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions:NA
 * CaseDescription: verify the ScheduleConnectAbilityDoneLocked function when the parameter is null.
 */
HWTEST_F(AbilityConnectManagerTest, AAFWK_Connect_Service_016, TestSize.Level1)
{
    auto callback = new AbilityConnectCallback();
    auto result = GetCommonExtensionManager()->ScheduleConnectAbilityDoneLocked(nullptr, callback);
    EXPECT_EQ(OHOS::ERR_INVALID_VALUE, result);
}

/*
 * Feature: AbilityConnectManager
 * Function: ScheduleConnectAbilityDoneLocked
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions:NA
 * CaseDescription: verify the ScheduleConnectAbilityDoneLocked function when the state is CONNECTED.
 */
HWTEST_F(AbilityConnectManagerTest, AAFWK_Connect_Service_017, TestSize.Level1)
{
    auto callback = new AbilityConnectCallback();
    GetCommonExtensionManager()->ConnectAbilityLocked(abilityRequest_, callback, nullptr);

    auto elementName = abilityRequest_.want.GetElement();
    std::string elementNameUri = elementName.GetURI();
    auto serviceMap = GetCommonExtensionManager()->GetServiceMap();
    auto abilityRecord = serviceMap.at(elementNameUri);
    auto token = abilityRecord->GetToken();

    abilityRecord->SetAbilityState(OHOS::AAFwk::AbilityState::ACTIVE);
    GetCommonExtensionManager()->ScheduleConnectAbilityDoneLocked(token, callback);
    auto abilityRecordB = std::static_pointer_cast<BaseExtensionRecord>(Token::GetAbilityRecordByToken(token));
    EXPECT_TRUE(abilityRecordB);
    auto connectRecordList = abilityRecordB->GetConnectRecordList();
    int size = connectRecordList.size();
    EXPECT_EQ(1, size);
    if (size != 0) {
        auto connState = (*(connectRecordList.begin()))->GetConnectState();
        EXPECT_EQ(ConnectionState::CONNECTED, connState);
    }
}

/*
 * Feature: AbilityConnectManager
 * Function: ScheduleConnectAbilityDoneLocked
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions:NA
 * CaseDescription: verify the input parameters.
 */
HWTEST_F(AbilityConnectManagerTest, AAFWK_Kit_Connect_001, TestSize.Level1)
{
    // start test
    // test1 for serviceToken is null but remoteObject is valid
    OHOS::sptr<OHOS::IRemoteObject> object = new AbilityConnectCallback();
    int ret = GetCommonExtensionManager()->ScheduleConnectAbilityDoneLocked(nullptr, object);
    EXPECT_EQ(OHOS::ERR_INVALID_VALUE, ret);

    // test2 for both of serviceToken and remoteObject are null
    ret = GetCommonExtensionManager()->ScheduleConnectAbilityDoneLocked(nullptr, nullptr);
    EXPECT_EQ(OHOS::ERR_INVALID_VALUE, ret);
}

/*
 * Feature: AbilityConnectManager
 * Function: ScheduleConnectAbilityDoneLocked
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions:NA
 * CaseDescription: verify the input serviceToken which corresponding ability record doesn't exist.
 */
HWTEST_F(AbilityConnectManagerTest, AAFWK_Kit_Connect_002, TestSize.Level1)
{
    // test for serviceToken's abilityRecord is null
    serviceRecord_ = nullptr;
    int ret = GetCommonExtensionManager()->ScheduleConnectAbilityDoneLocked(serviceToken_, nullptr);
    EXPECT_EQ(OHOS::ERR_INVALID_VALUE, ret);
}

/*
 * Feature: AbilityConnectManager
 * Function: ScheduleConnectAbilityDoneLocked
 * SubFunction: OnAbilityConnectDone
 * FunctionPoints: NA
 * EnvConditions:NA
 * CaseDescription: verify the input serviceToken which corresponding connection list is empty.
 */
HWTEST_F(AbilityConnectManagerTest, AAFWK_Kit_Connect_003, TestSize.Level1)
{
    // test for serviceToken's connection list is null
    // start test
    auto callback = new AbilityConnectCallback();
    int ret = GetCommonExtensionManager()->ScheduleConnectAbilityDoneLocked(serviceToken_, callback);
    EXPECT_EQ(OHOS::AAFwk::INVALID_CONNECTION_STATE, ret);
    auto connList = serviceRecord_->GetConnectRecordList();
    EXPECT_EQ(true, connList.empty());  // the connection list size should be empty
}

/*
 * Feature: AbilityConnectManager
 * Function: ScheduleConnectAbilityDoneLocked
 * SubFunction: OnAbilityConnectDone
 * FunctionPoints: NA
 * EnvConditions:NA
 * CaseDescription: verify the input serviceToken which corresponding connection list members' state
 * is not CONNECTING or CONNECTED.
 */
HWTEST_F(AbilityConnectManagerTest, AAFWK_Kit_Connect_004, TestSize.Level1)
{
    // test for schedule the service connected done but the corresponding connection state is not CONNECTING
    // generate the first connection record of callbackA_
    auto newConnRecord1 = ConnectionRecord::CreateConnectionRecord(
        serviceToken_, serviceRecord_, callbackA_, nullptr);  // newConnRecord1's default state is INIT
    serviceRecord_->AddConnectRecordToList(newConnRecord1);
    // generate the second connection record of callbackB_
    auto newConnRecord2 = ConnectionRecord::CreateConnectionRecord(serviceToken_, serviceRecord_, callbackB_, nullptr);
    newConnRecord2->SetConnectState(ConnectionState::DISCONNECTING);  // newConnRecord2's state is DISCONNECTING
    serviceRecord_->AddConnectRecordToList(newConnRecord2);
    auto connList = serviceRecord_->GetConnectRecordList();
    EXPECT_EQ(2, static_cast<int>(connList.size()));  // the connection list members should be two
    // start test
    auto callback = new AbilityConnectCallback();
    serviceRecord_->SetAbilityState(OHOS::AAFwk::AbilityState::ACTIVE);
    int ret = GetCommonExtensionManager()->ScheduleConnectAbilityDoneLocked(serviceToken_, callback);
    EXPECT_EQ(OHOS::ERR_OK, ret);  // the result should be OK
    // connection callback should not be called, so check the count
    EXPECT_EQ(0, AbilityConnectCallback::onAbilityConnectDoneCount);
    EXPECT_EQ(0, AbilityConnectCallback::onAbilityDisconnectDoneCount);
}

/*
 * Feature: AbilityConnectManager
 * Function: ScheduleConnectAbilityDoneLocked
 * SubFunction: OnAbilityConnectDone
 * FunctionPoints: NA
 * EnvConditions:NA
 * CaseDescription: verify the scene : 1.serviceToken's corresponding connection list member's state is CONNECTING.
 * 2.But the connection callback is null.
 */
HWTEST_F(AbilityConnectManagerTest, AAFWK_Kit_Connect_005, TestSize.Level1)
{
    // test for schedule the service connected done but the corresponding connection state is not CONNECTING
    // generate the first connection record of null
    auto newConnRecord1 = ConnectionRecord::CreateConnectionRecord(
        serviceToken_, serviceRecord_, nullptr, nullptr);  // newConnRecord1's default state is INIT
    serviceRecord_->AddConnectRecordToList(newConnRecord1);
    newConnRecord1->SetConnectState(ConnectionState::CONNECTING);  // newConnRecord1's state is CONNECTING
    auto connList = serviceRecord_->GetConnectRecordList();
    EXPECT_EQ(1, static_cast<int>(connList.size()));  // the connection list members should be zero
    // start test
    auto callback = new AbilityConnectCallback();
    serviceRecord_->SetAbilityState(OHOS::AAFwk::AbilityState::ACTIVE);
    int ret = GetCommonExtensionManager()->ScheduleConnectAbilityDoneLocked(serviceToken_, callback);
    EXPECT_EQ(OHOS::ERR_OK, ret);  // the result should be OK
    // connection callback should not be called, so check the count
    EXPECT_EQ(0, AbilityConnectCallback::onAbilityConnectDoneCount);
}

/*
 * Feature: AbilityConnectManager
 * Function: ScheduleConnectAbilityDoneLocked
 * SubFunction: OnAbilityConnectDone
 * FunctionPoints: NA
 * EnvConditions:NA
 * CaseDescription: verify the scene : 1.serviceToken's corresponding connection list member's state is CONNECTED.
 * 2.But the connection callback is null.
 */
HWTEST_F(AbilityConnectManagerTest, AAFWK_Kit_Connect_006, TestSize.Level1)
{
    // test for schedule the service connected done but the corresponding connection state is not CONNECTING
    // generate the first connection record of null
    auto newConnRecord1 = ConnectionRecord::CreateConnectionRecord(
        serviceToken_, serviceRecord_, nullptr, nullptr);  // newConnRecord1's default state is INIT
    serviceRecord_->AddConnectRecordToList(newConnRecord1);
    newConnRecord1->SetConnectState(ConnectionState::CONNECTED);  // newConnRecord1's state is CONNECTED
    auto connList = serviceRecord_->GetConnectRecordList();
    EXPECT_EQ(1, static_cast<int>(connList.size()));  // the connection list members should be zero
    // start test
    auto callback = new AbilityConnectCallback();
    serviceRecord_->SetAbilityState(OHOS::AAFwk::AbilityState::ACTIVE);
    int ret = GetCommonExtensionManager()->ScheduleConnectAbilityDoneLocked(serviceToken_, callback);
    EXPECT_EQ(OHOS::ERR_OK, ret);  // the result should be OK
    // connection callback should not be called, so check the count
    EXPECT_EQ(0, AbilityConnectCallback::onAbilityConnectDoneCount);
}

/*
 * Feature: AbilityConnectManager
 * Function: ScheduleConnectAbilityDoneLocked
 * SubFunction: OnAbilityConnectDone
 * FunctionPoints: NA
 * EnvConditions:NA
 * CaseDescription: verify the scene : 1.serviceToken's corresponding connection list member's state is CONNECTING.
 * 2.But the connection callback is valid.
 */
HWTEST_F(AbilityConnectManagerTest, AAFWK_Kit_Connect_007, TestSize.Level1)
{
    // test for schedule the service connected done but the corresponding connection state is not CONNECTING
    // generate the first connection record of callbackA_
    auto newConnRecord1 = ConnectionRecord::CreateConnectionRecord(
        serviceToken_, serviceRecord_, callbackA_, nullptr);  // newConnRecord1's default state is INIT
    serviceRecord_->AddConnectRecordToList(newConnRecord1);
    // generate the second connection record of callbackB_
    auto newConnRecord2 = ConnectionRecord::CreateConnectionRecord(serviceToken_, serviceRecord_, callbackB_, nullptr);
    newConnRecord2->SetConnectState(ConnectionState::CONNECTING);  // newConnRecord2's state is CONNECTING
    serviceRecord_->AddConnectRecordToList(newConnRecord2);
    auto connList = serviceRecord_->GetConnectRecordList();
    EXPECT_EQ(2, static_cast<int>(connList.size()));  // the connection list members should be two
    // start test
    auto callback = new AbilityConnectCallback();
    serviceRecord_->SetAbilityState(OHOS::AAFwk::AbilityState::ACTIVE);
    int ret = GetCommonExtensionManager()->ScheduleConnectAbilityDoneLocked(serviceToken_, callback);
    EXPECT_EQ(OHOS::ERR_OK, ret);  // the result should be OK
    // connection callback should not be called, so check the count
    EXPECT_EQ(0, AbilityConnectCallback::onAbilityConnectDoneCount);
    EXPECT_EQ(0, AbilityConnectCallback::onAbilityDisconnectDoneCount);
}

/*
 * Feature: AbilityConnectManager
 * Function: ScheduleConnectAbilityDoneLocked
 * SubFunction: OnAbilityConnectDone
 * FunctionPoints: NA
 * EnvConditions:NA
 * CaseDescription: verify the scene : 1.serviceToken's corresponding connection list member's state is CONNECTED.
 * 2.But the connection callback is valid.
 */
HWTEST_F(AbilityConnectManagerTest, AAFWK_Kit_Connect_008, TestSize.Level1)
{
    // test for schedule the service connected done but the corresponding connection state is not CONNECTING
    // generate the first connection record of callbackA_
    auto newConnRecord1 = ConnectionRecord::CreateConnectionRecord(
        serviceToken_, serviceRecord_, callbackA_, nullptr);               // newConnRecord1's default state is INIT
    newConnRecord1->SetConnectState(ConnectionState::CONNECTED);  // newConnRecord1's state is CONNECTED
    serviceRecord_->AddConnectRecordToList(newConnRecord1);
    // generate the second connection record of callbackB_
    auto newConnRecord2 = ConnectionRecord::CreateConnectionRecord(serviceToken_, serviceRecord_, callbackB_, nullptr);
    newConnRecord2->SetConnectState(ConnectionState::CONNECTING);  // newConnRecord2's state is CONNECTING
    serviceRecord_->AddConnectRecordToList(newConnRecord2);
    // generate the third connection record of callbackC
    auto callbackC = new AbilityConnectCallback();
    auto newConnRecord3 = ConnectionRecord::CreateConnectionRecord(serviceToken_, serviceRecord_, callbackC, nullptr);
    newConnRecord3->SetConnectState(ConnectionState::CONNECTING);  // newConnRecord3's state is CONNECTING
    serviceRecord_->AddConnectRecordToList(newConnRecord3);
    auto connList = serviceRecord_->GetConnectRecordList();
    EXPECT_EQ(3, static_cast<int>(connList.size()));  // the connection list members should be three
    // start test
    auto callback = new AbilityConnectCallback();
    serviceRecord_->SetAbilityState(OHOS::AAFwk::AbilityState::ACTIVE);
    int ret = GetCommonExtensionManager()->ScheduleConnectAbilityDoneLocked(serviceToken_, callback);
    EXPECT_EQ(OHOS::ERR_OK, ret);  // the result should be OK
    // connection callback should not be called, so check the count
    EXPECT_EQ(0, AbilityConnectCallback::onAbilityConnectDoneCount);
    EXPECT_EQ(0, AbilityConnectCallback::onAbilityDisconnectDoneCount);
}

/*
 * Feature: AbilityConnectManager
 * Function: DisconnectAbilityLocked
 * SubFunction:
 * FunctionPoints: DisconnectAbilityLocked and ConnectAbilityLocked
 * EnvConditions:NA
 * CaseDescription:Verify the following:
 * 1. Disconnect ability a nonexistent connect, disconnect failed
 * 2. If the current connect ability state is not connected, disconnect fails
 * 3. Verify the success of disconnect ability
 */
HWTEST_F(AbilityConnectManagerTest, AAFWK_Kit_Disconnect_001, TestSize.Level1)
{
    GetCommonExtensionManager()->SetTaskHandler(TaskHandler());
    GetCommonExtensionManager()->SetEventHandler(EventHandler());

    auto callback = new AbilityConnectCallback();
    auto result = GetCommonExtensionManager()->DisconnectAbilityLocked(callback);
    EXPECT_EQ(result, OHOS::AAFwk::CONNECTION_NOT_EXIST);

    auto result1 = GetCommonExtensionManager()->ConnectAbilityLocked(abilityRequest_, callbackA_, nullptr);
    EXPECT_EQ(0, result1);

    auto result2 = GetCommonExtensionManager()->DisconnectAbilityLocked(callbackA_);
    EXPECT_EQ(result2, OHOS::AAFwk::INVALID_CONNECTION_STATE);

    auto list = GetCommonExtensionManager()->GetConnectRecordListByCallback(callbackA_);
    EXPECT_EQ(static_cast<int>(list.size()), 1);

    for (auto& it : list) {
        it->SetConnectState(ConnectionState::CONNECTED);
    }

    auto result3 = GetCommonExtensionManager()->DisconnectAbilityLocked(callbackA_);
    EXPECT_EQ(result3, OHOS::ERR_OK);
}

/*
 * Feature: AbilityConnectManager
 * Function: DisconnectAbilityLocked
 * SubFunction:
 * FunctionPoints: DisconnectAbilityLocked and ConnectAbilityLocked
 * EnvConditions:NA
 * CaseDescription: Results after verifying the disconnect ability
 */
HWTEST_F(AbilityConnectManagerTest, AAFWK_Kit_Disconnect_002, TestSize.Level1)
{
    GetCommonExtensionManager()->SetTaskHandler(TaskHandler());
    GetCommonExtensionManager()->SetEventHandler(EventHandler());

    auto result = GetCommonExtensionManager()->ConnectAbilityLocked(abilityRequest_, callbackA_, nullptr);
    EXPECT_EQ(0, result);

    auto result1 = GetCommonExtensionManager()->ConnectAbilityLocked(abilityRequest_, callbackB_, nullptr);
    EXPECT_EQ(0, result1);

    auto result2 = GetCommonExtensionManager()->ConnectAbilityLocked(abilityRequest1_, callbackA_, nullptr);
    EXPECT_EQ(0, result2);

    auto result3 = GetCommonExtensionManager()->ConnectAbilityLocked(abilityRequest1_, callbackB_, nullptr);
    EXPECT_EQ(0, result3);

    auto listA = GetCommonExtensionManager()->GetConnectRecordListByCallback(callbackA_);
    EXPECT_EQ(static_cast<int>(listA.size()), 2);

    for (auto& it : listA) {
        it->SetConnectState(ConnectionState::CONNECTED);
    }

    auto listB = GetCommonExtensionManager()->GetConnectRecordListByCallback(callbackB_);
    EXPECT_EQ(static_cast<int>(listB.size()), 2);

    for (auto& it : listB) {
        it->SetConnectState(ConnectionState::CONNECTED);
    }

    auto result5 = GetCommonExtensionManager()->DisconnectAbilityLocked(callbackA_);
    WaitUntilTaskDone(TaskHandler());
    EXPECT_EQ(result5, OHOS::ERR_OK);
    auto serviceMap = GetCommonExtensionManager()->GetServiceMap();
    EXPECT_EQ(static_cast<int>(serviceMap.size()), 2);

    auto connectMap = GetCommonExtensionManager()->connectMap_;
    EXPECT_EQ(static_cast<int>(connectMap.size()), 1);
    for (auto& it : connectMap) {
        EXPECT_EQ(static_cast<int>(it.second.size()), 2);
    }
}

/*
 * Feature: AbilityConnectManager
 * Function: DisconnectAbilityLocked
 * SubFunction:
 * FunctionPoints: DisconnectAbilityLocked and ConnectAbilityLocked
 * EnvConditions:NA
 * CaseDescription:Verify the following:
 * 1. Disconnect ability a nonexistent connect, disconnect failed
 * 2. If the current connect ability state is not connected, disconnect fails
 * 3. Verify the success of disconnect ability
 */
HWTEST_F(AbilityConnectManagerTest, AAFWK_Kit_Disconnect_003, TestSize.Level1)
{
    GetCommonExtensionManager()->SetTaskHandler(TaskHandler());
    GetCommonExtensionManager()->SetEventHandler(EventHandler());

    auto callback = new AbilityConnectCallback();
    auto result = GetCommonExtensionManager()->DisconnectAbilityLocked(callback);
    EXPECT_EQ(result, OHOS::AAFwk::CONNECTION_NOT_EXIST);

    auto result1 = GetCommonExtensionManager()->ConnectAbilityLocked(abilityRequest_, callbackA_, nullptr);
    EXPECT_EQ(0, result1);

    auto result2 = GetCommonExtensionManager()->DisconnectAbilityLocked(callbackA_);
    EXPECT_EQ(result2, OHOS::AAFwk::INVALID_CONNECTION_STATE);

    auto list = GetCommonExtensionManager()->GetConnectRecordListByCallback(callbackA_);
    EXPECT_EQ(static_cast<int>(list.size()), 1);

    for (auto& it : list) {
        it->SetConnectState(ConnectionState::CONNECTED);
    }

    auto result3 = GetCommonExtensionManager()->DisconnectAbilityLocked(callbackA_);
    EXPECT_EQ(result3, OHOS::ERR_OK);
}

/*
 * Feature: AbilityConnectManager
 * Function: DisconnectAbilityLocked
 * SubFunction:
 * FunctionPoints: DisconnectAbilityLocked and ConnectAbilityLocked
 * EnvConditions:NA
 * CaseDescription: Results after verifying the disconnect ability
 */
HWTEST_F(AbilityConnectManagerTest, AAFWK_Kit_Disconnect_004, TestSize.Level1)
{
    GetCommonExtensionManager()->SetTaskHandler(TaskHandler());
    GetCommonExtensionManager()->SetEventHandler(EventHandler());

    auto result = GetCommonExtensionManager()->ConnectAbilityLocked(abilityRequest_, callbackA_, nullptr);
    EXPECT_EQ(0, result);

    auto result1 = GetCommonExtensionManager()->ConnectAbilityLocked(abilityRequest_, callbackB_, nullptr);
    EXPECT_EQ(0, result1);

    auto result2 = GetCommonExtensionManager()->ConnectAbilityLocked(abilityRequest1_, callbackA_, nullptr);
    EXPECT_EQ(0, result2);

    auto result3 = GetCommonExtensionManager()->ConnectAbilityLocked(abilityRequest1_, callbackB_, nullptr);
    EXPECT_EQ(0, result3);

    auto listA = GetCommonExtensionManager()->GetConnectRecordListByCallback(callbackA_);
    EXPECT_EQ(static_cast<int>(listA.size()), 2);

    for (auto& it : listA) {
        it->SetConnectState(ConnectionState::CONNECTED);
    }

    auto listB = GetCommonExtensionManager()->GetConnectRecordListByCallback(callbackB_);
    EXPECT_EQ(static_cast<int>(listB.size()), 2);

    for (auto& it : listB) {
        it->SetConnectState(ConnectionState::CONNECTED);
    }

    auto result5 = GetCommonExtensionManager()->DisconnectAbilityLocked(callbackA_);
    WaitUntilTaskDone(TaskHandler());
    EXPECT_EQ(result5, OHOS::ERR_OK);
    auto serviceMap = GetCommonExtensionManager()->GetServiceMap();
    EXPECT_EQ(static_cast<int>(serviceMap.size()), 2);

    auto connectMap = GetCommonExtensionManager()->connectMap_;
    EXPECT_EQ(static_cast<int>(connectMap.size()), 1);
    for (auto& it : connectMap) {
        EXPECT_EQ(static_cast<int>(it.second.size()), 2);
    }
}

/*
 * Feature: AbilityConnectManager
 * Function: AbilityTransitionDone
 * SubFunction: NA
 * FunctionPoints: AbilityTransitionDone
 * EnvConditions:NA
 * CaseDescription: Verify the abilitytransitiondone process
 */
HWTEST_F(AbilityConnectManagerTest, AAFWK_Connect_Service_019, TestSize.Level1)
{
    OHOS::sptr<OHOS::IRemoteObject> nullToken = nullptr;
    auto result = GetCommonExtensionManager()->AbilityTransitionDone(nullToken, OHOS::AAFwk::AbilityState::INACTIVE);
    EXPECT_EQ(result, OHOS::ERR_INVALID_VALUE);

    GetCommonExtensionManager()->ConnectAbilityLocked(abilityRequest_, callbackA_, nullptr);
    auto elementName = abilityRequest_.want.GetElement();
    std::string elementNameUri = elementName.GetURI();
    auto serviceMap = GetCommonExtensionManager()->GetServiceMap();
    auto abilityRecord = serviceMap.at(elementNameUri);
    auto token = abilityRecord->GetToken();

    auto result1 = GetCommonExtensionManager()->AbilityTransitionDone(token, OHOS::AAFwk::AbilityState::INACTIVE);
    EXPECT_EQ(result1, OHOS::ERR_INVALID_VALUE);

    GetCommonExtensionManager()->MoveToTerminatingMap(abilityRecord);
    auto result2 = GetCommonExtensionManager()->AbilityTransitionDone(token, OHOS::AAFwk::AbilityState::INITIAL);
    EXPECT_EQ(result2, OHOS::ERR_OK);

    auto result3 = GetCommonExtensionManager()->AbilityTransitionDone(token, OHOS::AAFwk::AbilityState::TERMINATING);
    EXPECT_EQ(result3, OHOS::ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityConnectManager
 * Function: ScheduleDisconnectAbilityDoneLocked
 * SubFunction: NA
 * FunctionPoints: ScheduleDisconnectAbilityDoneLocked
 * EnvConditions:NA
 * CaseDescription: Verify the ScheduleDisconnectAbilityDoneLocked process
 */
HWTEST_F(AbilityConnectManagerTest, AAFWK_Connect_Service_020, TestSize.Level1)
{
    GetCommonExtensionManager()->SetTaskHandler(TaskHandler());
    GetCommonExtensionManager()->SetEventHandler(EventHandler());

    OHOS::sptr<OHOS::IRemoteObject> nullToken = nullptr;
    auto result = GetCommonExtensionManager()->ScheduleDisconnectAbilityDoneLocked(nullToken);
    EXPECT_EQ(result, OHOS::AAFwk::CONNECTION_NOT_EXIST);

    std::shared_ptr<BaseExtensionRecord> ability = nullptr;
    OHOS::sptr<OHOS::IRemoteObject> token1 = new OHOS::AAFwk::Token(ability);
    auto result1 = GetCommonExtensionManager()->ScheduleDisconnectAbilityDoneLocked(token1);
    EXPECT_EQ(result1, OHOS::AAFwk::CONNECTION_NOT_EXIST);

    GetCommonExtensionManager()->ConnectAbilityLocked(abilityRequest_, callbackA_, nullptr);
    auto elementName = abilityRequest_.want.GetElement();
    std::string elementNameUri = elementName.GetURI();
    auto serviceMap = GetCommonExtensionManager()->GetServiceMap();
    auto abilityRecord = serviceMap.at(elementNameUri);
    auto token = abilityRecord->GetToken();

    auto listA = GetCommonExtensionManager()->GetConnectRecordListByCallback(callbackA_);
    for (auto& it : listA) {
        it->SetConnectState(ConnectionState::CONNECTED);
    }

    auto result2 = GetCommonExtensionManager()->DisconnectAbilityLocked(callbackA_);
    WaitUntilTaskDone(TaskHandler());
    EXPECT_EQ(result2, OHOS::ERR_OK);

    auto result3 = GetCommonExtensionManager()->ScheduleDisconnectAbilityDoneLocked(token);
    EXPECT_EQ(result3, OHOS::AAFwk::INVALID_CONNECTION_STATE);

    abilityRecord->SetAbilityState(OHOS::AAFwk::AbilityState::ACTIVE);

    auto result4 = GetCommonExtensionManager()->ScheduleDisconnectAbilityDoneLocked(token);
    EXPECT_EQ(result4, OHOS::ERR_OK);
}

/*
 * Feature: AbilityConnectManager
 * Function: ScheduleCommandAbilityDoneLocked
 * SubFunction: NA
 * FunctionPoints: ScheduleCommandAbilityDoneLocked
 * EnvConditions:NA
 * CaseDescription: Verify the ScheduleCommandAbilityDoneLocked process
 */
HWTEST_F(AbilityConnectManagerTest, AAFWK_Connect_Service_021, TestSize.Level1)
{
    GetCommonExtensionManager()->SetTaskHandler(TaskHandler());
    GetCommonExtensionManager()->SetEventHandler(EventHandler());

    OHOS::sptr<OHOS::IRemoteObject> nullToken = nullptr;
    auto result = GetCommonExtensionManager()->ScheduleCommandAbilityDoneLocked(nullToken);
    EXPECT_EQ(result, OHOS::ERR_INVALID_VALUE);

    std::shared_ptr<BaseExtensionRecord> ability = nullptr;
    OHOS::sptr<OHOS::IRemoteObject> token1 = new OHOS::AAFwk::Token(ability);
    auto result1 = GetCommonExtensionManager()->ScheduleCommandAbilityDoneLocked(token1);
    EXPECT_EQ(result1, OHOS::ERR_INVALID_VALUE);

    GetCommonExtensionManager()->ConnectAbilityLocked(abilityRequest_, callbackA_, nullptr);
    auto elementName = abilityRequest_.want.GetElement();
    std::string elementNameUri = elementName.GetURI();
    auto serviceMap = GetCommonExtensionManager()->GetServiceMap();
    auto abilityRecord = serviceMap.at(elementNameUri);
    auto token = abilityRecord->GetToken();

    auto result2 = GetCommonExtensionManager()->ScheduleCommandAbilityDoneLocked(token);
    EXPECT_EQ(result2, OHOS::AAFwk::INVALID_CONNECTION_STATE);

    abilityRecord->SetAbilityState(OHOS::AAFwk::AbilityState::ACTIVE);
    auto result3 = GetCommonExtensionManager()->ScheduleCommandAbilityDoneLocked(token);
    EXPECT_EQ(result3, OHOS::ERR_OK);
}

/*
 * Feature: AbilityConnectManager
 * Function: GetExtensionByTokenFromServiceMap
 * SubFunction: NA
 * FunctionPoints: GetExtensionByTokenFromServiceMap
 * EnvConditions:NA
 * CaseDescription: Verify the GetExtensionByTokenFromServiceMap process
 */
HWTEST_F(AbilityConnectManagerTest, AAFWK_Connect_Service_022, TestSize.Level1)
{
    GetCommonExtensionManager()->ConnectAbilityLocked(abilityRequest_, callbackA_, nullptr);
    auto elementName = abilityRequest_.want.GetElement();
    std::string elementNameUri = elementName.GetURI();
    auto serviceMap = GetCommonExtensionManager()->GetServiceMap();
    auto abilityRecord = serviceMap.at(elementNameUri);
    auto token = abilityRecord->GetToken();

    auto ability = GetCommonExtensionManager()->GetExtensionByTokenFromServiceMap(token);
    EXPECT_EQ(abilityRecord, ability);

    OHOS::sptr<OHOS::IRemoteObject> nullToken = nullptr;
    auto ability1 = GetCommonExtensionManager()->GetExtensionByTokenFromServiceMap(nullToken);
    EXPECT_EQ(nullptr, ability1);

    auto recordId = abilityRecord->GetAbilityRecordId();
    EXPECT_EQ(GetCommonExtensionManager()->GetExtensionByIdFromServiceMap(recordId), abilityRecord);
    EXPECT_EQ(GetCommonExtensionManager()->GetExtensionByIdFromServiceMap(0), nullptr);
}

/*
 * Feature: AbilityConnectManager
 * Function: OnAbilityDied
 * SubFunction:
 * FunctionPoints: OnAbilityDied
 * EnvConditions:NA
 * CaseDescription: Verify the OnAbilityDied process
 */
HWTEST_F(AbilityConnectManagerTest, AAFWK_Connect_Service_024, TestSize.Level1)
{
    GetCommonExtensionManager()->SetTaskHandler(TaskHandler());
    GetCommonExtensionManager()->SetEventHandler(EventHandler());

    auto result = GetCommonExtensionManager()->ConnectAbilityLocked(abilityRequest_, callbackA_, nullptr);
    EXPECT_EQ(0, result);

    auto result1 = GetCommonExtensionManager()->ConnectAbilityLocked(abilityRequest_, callbackB_, nullptr);
    EXPECT_EQ(0, result1);

    auto result2 = GetCommonExtensionManager()->ConnectAbilityLocked(abilityRequest1_, callbackA_, nullptr);
    EXPECT_EQ(0, result2);

    auto result3 = GetCommonExtensionManager()->ConnectAbilityLocked(abilityRequest1_, callbackB_, nullptr);
    EXPECT_EQ(0, result3);

    auto listA = GetCommonExtensionManager()->GetConnectRecordListByCallback(callbackA_);
    EXPECT_EQ(static_cast<int>(listA.size()), 2);

    for (auto& it : listA) {
        it->SetConnectState(ConnectionState::CONNECTED);
    }

    auto listB = GetCommonExtensionManager()->GetConnectRecordListByCallback(callbackB_);
    EXPECT_EQ(static_cast<int>(listB.size()), 2);

    for (auto& it : listB) {
        it->SetConnectState(ConnectionState::CONNECTED);
    }

    auto elementName = abilityRequest_.want.GetElement();
    std::string elementNameUri = elementName.GetURI();
    auto serviceMap = GetCommonExtensionManager()->GetServiceMap();
    auto abilityRecord = serviceMap.at(elementNameUri);
    auto token = abilityRecord->GetToken();

    auto task = [abilityRecord, connectManager = GetCommonExtensionManager()]() {
        connectManager->HandleAbilityDiedTask(abilityRecord);
    };
    EXPECT_CALL(*taskHandler_, SubmitTaskInner(_, _)).WillRepeatedly(DoAll(SetArgReferee<0>(task),
        testing::Invoke(taskHandler_.get(), &MockTaskHandlerWrap::MockTaskHandler)));
    GetCommonExtensionManager()->OnAbilityDied(abilityRecord);
    auto list = abilityRecord->GetConnectRecordList();
    EXPECT_EQ(static_cast<int>(list.size()), 0);

    auto elementName1 = abilityRequest1_.want.GetElement();
    std::string elementNameUri1 = elementName1.GetURI();
    serviceMap = GetCommonExtensionManager()->GetServiceMap();
    auto abilityRecord1 = serviceMap.at(elementNameUri1);
    auto token1 = abilityRecord1->GetToken();
    auto task1 = [abilityRecord1, connectManager = GetCommonExtensionManager()]() {
        connectManager->HandleAbilityDiedTask(abilityRecord1);
    };
    EXPECT_CALL(*taskHandler_, SubmitTaskInner(_, _)).WillRepeatedly(DoAll(SetArgReferee<0>(task1),
        testing::Invoke(taskHandler_.get(), &MockTaskHandlerWrap::MockTaskHandler)));
    GetCommonExtensionManager()->OnAbilityDied(abilityRecord1);
    auto list1 = abilityRecord1->GetConnectRecordList();
    EXPECT_EQ(static_cast<int>(list1.size()), 0);
}

/*
 * Feature: AbilityConnectManager
 * Function: DispatchInactive
 * SubFunction:
 * FunctionPoints: DispatchInactive
 * EnvConditions:NA
 * CaseDescription: Verify the DispatchInactive process
 */
HWTEST_F(AbilityConnectManagerTest, AAFWK_Connect_Service_025, TestSize.Level1)
{
    std::shared_ptr<BaseExtensionRecord> ability = nullptr;
    auto result = GetCommonExtensionManager()->DispatchInactive(
        ability, OHOS::AAFwk::AbilityState::INACTIVATING, nullptr);
    EXPECT_EQ(result, OHOS::ERR_INVALID_VALUE);

    GetCommonExtensionManager()->SetTaskHandler(TaskHandler());
    GetCommonExtensionManager()->SetEventHandler(EventHandler());

    auto result3 = GetCommonExtensionManager()->ConnectAbilityLocked(abilityRequest_, callbackA_, nullptr);
    EXPECT_EQ(0, result3);

    auto elementName = abilityRequest_.want.GetElement();
    std::string elementNameUri = elementName.GetURI();
    auto serviceMap = GetCommonExtensionManager()->GetServiceMap();
    auto abilityRecord = serviceMap.at(elementNameUri);
    auto token = abilityRecord->GetToken();

    abilityRecord->SetAbilityState(OHOS::AAFwk::AbilityState::ACTIVE);
    auto result1 = GetCommonExtensionManager()->DispatchInactive(
        abilityRecord, OHOS::AAFwk::AbilityState::INACTIVATING, abilityRecord->GetToken());
    EXPECT_EQ(result1, OHOS::ERR_INVALID_VALUE);

    abilityRecord->SetAbilityState(OHOS::AAFwk::AbilityState::INACTIVATING);
    auto result2 = GetCommonExtensionManager()->DispatchInactive(
        abilityRecord, OHOS::AAFwk::AbilityState::INACTIVATING, abilityRecord->GetToken());
    EXPECT_EQ(result2, OHOS::ERR_OK);
    EXPECT_EQ(abilityRecord->GetAbilityState(), OHOS::AAFwk::AbilityState::INACTIVE);
}

/*
 * Feature: AbilityConnectManager
 * Function: DispatchInactive
 * SubFunction:
 * FunctionPoints: DispatchInactive
 * EnvConditions:NA
 * CaseDescription: Verify the DispatchInactive process
 */
HWTEST_F(AbilityConnectManagerTest, AAFWK_Connect_Service_026, TestSize.Level1)
{
    std::shared_ptr<BaseExtensionRecord> ability = nullptr;
    auto result = GetCommonExtensionManager()->DispatchInactive(
        ability, OHOS::AAFwk::AbilityState::INACTIVATING, nullptr);
    EXPECT_EQ(result, OHOS::ERR_INVALID_VALUE);

    GetCommonExtensionManager()->SetTaskHandler(TaskHandler());
    GetCommonExtensionManager()->SetEventHandler(EventHandler());

    auto result3 = GetCommonExtensionManager()->ConnectAbilityLocked(abilityRequest_, callbackA_, nullptr);
    EXPECT_EQ(0, result3);

    auto elementName = abilityRequest_.want.GetElement();
    std::string elementNameUri = elementName.GetURI();
    auto serviceMap = GetCommonExtensionManager()->GetServiceMap();
    auto abilityRecord = serviceMap.at(elementNameUri);
    auto token = abilityRecord->GetToken();

    abilityRecord->SetAbilityState(OHOS::AAFwk::AbilityState::ACTIVE);
    auto result1 = GetCommonExtensionManager()->DispatchInactive(
        abilityRecord, OHOS::AAFwk::AbilityState::INACTIVATING, abilityRecord->GetToken());
    EXPECT_EQ(result1, OHOS::ERR_INVALID_VALUE);

    abilityRecord->SetAbilityState(OHOS::AAFwk::AbilityState::INACTIVATING);
    auto result2 = GetCommonExtensionManager()->DispatchInactive(
        abilityRecord, OHOS::AAFwk::AbilityState::INACTIVATING, abilityRecord->GetToken());
    EXPECT_EQ(result2, OHOS::ERR_OK);
    EXPECT_EQ(abilityRecord->GetAbilityState(), OHOS::AAFwk::AbilityState::INACTIVE);
}

/*
 * Feature: AbilityConnectManager
 * Function: AddConnectDeathRecipient
 * SubFunction:
 * FunctionPoints: AddConnectDeathRecipient
 * EnvConditions:NA
 * CaseDescription: Verify the AddConnectDeathRecipient process
 */
HWTEST_F(AbilityConnectManagerTest, AAFWK_Connect_Service_027, TestSize.Level1)
{
    GetCommonExtensionManager()->SetTaskHandler(TaskHandler());
    GetCommonExtensionManager()->SetEventHandler(EventHandler());

    GetCommonExtensionManager()->AddConnectDeathRecipient(nullptr);
    EXPECT_TRUE(GetCommonExtensionManager()->recipientMap_.empty());
}

/*
 * Feature: AbilityConnectManager
 * Function: RemoveConnectDeathRecipient
 * SubFunction:
 * FunctionPoints: RemoveConnectDeathRecipient
 * EnvConditions:NA
 * CaseDescription: Verify the RemoveConnectDeathRecipient process
 */
HWTEST_F(AbilityConnectManagerTest, AAFWK_Connect_Service_028, TestSize.Level1)
{
    GetCommonExtensionManager()->SetTaskHandler(TaskHandler());
    GetCommonExtensionManager()->SetEventHandler(EventHandler());

    GetCommonExtensionManager()->AddConnectDeathRecipient(nullptr);
    EXPECT_TRUE(GetCommonExtensionManager()->recipientMap_.empty());

    GetCommonExtensionManager()->RemoveConnectDeathRecipient(nullptr);
    EXPECT_TRUE(GetCommonExtensionManager()->recipientMap_.empty());
}

/*
 * Feature: AbilityConnectManager
 * Function: OnCallBackDied
 * SubFunction:
 * FunctionPoints: OnCallBackDied
 * EnvConditions:NA
 * CaseDescription: Verify the OnCallBackDied process
 */
HWTEST_F(AbilityConnectManagerTest, AAFWK_Connect_Service_029, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityConnectManagerTest::AAFWK_Connect_Service_029 called.");
    GetCommonExtensionManager()->SetTaskHandler(TaskHandler());
    GetCommonExtensionManager()->SetEventHandler(EventHandler());


    auto result = GetCommonExtensionManager()->ConnectAbilityLocked(abilityRequest_, callbackA_, nullptr);
    EXPECT_EQ(0, result);

    const sptr<IRemoteObject> remoteObject1 = nullptr;
    auto task1 = [remoteObject1, connectManager = GetCommonExtensionManager()]() {
        connectManager->HandleCallBackDiedTask(remoteObject1);
    };
    EXPECT_CALL(*taskHandler_, SubmitTaskInner(_, _)).WillRepeatedly(DoAll(SetArgReferee<0>(task1),
        testing::Invoke(taskHandler_.get(), &MockTaskHandlerWrap::MockTaskHandler)));
    GetCommonExtensionManager()->OnCallBackDied(nullptr);
    auto connectMap = GetCommonExtensionManager()->connectMap_;
    auto connectRecordList = connectMap.at(callbackA_->AsObject());
    EXPECT_EQ(1, static_cast<int>(connectRecordList.size()));
    for (auto& it : connectRecordList) {
        EXPECT_NE(it->GetAbilityConnectCallback(), nullptr);
    }


    const sptr<IRemoteObject> remoteObject2 = callbackA_->AsObject();
    auto task2 = [remoteObject2, connectManager = GetCommonExtensionManager()]() {
        connectManager->HandleCallBackDiedTask(remoteObject2);
    };
    EXPECT_CALL(*taskHandler_, SubmitTaskInner(_, _)).WillRepeatedly(DoAll(SetArgReferee<0>(task2),
        testing::Invoke(taskHandler_.get(), &MockTaskHandlerWrap::MockTaskHandler)));
    GetCommonExtensionManager()->OnCallBackDied(callbackA_->AsObject());
    connectRecordList = connectMap.at(callbackA_->AsObject());
    EXPECT_EQ(1, static_cast<int>(connectMap.size()));
    for (auto& it : connectRecordList) {
        EXPECT_EQ(it->GetAbilityConnectCallback(), nullptr);
    }
    TAG_LOGI(AAFwkTag::TEST, "AbilityConnectManagerTest::AAFWK_Connect_Service_029 end.");
}

/*
 * Feature: AbilityConnectManager
 * Function: StartAbilityLocked
 * SubFunction: StartAbilityLocked
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityConnectManager StartAbilityLocked
 */
HWTEST_F(AbilityConnectManagerTest, AAFwk_AbilityMS_StartAbilityLocked_001, TestSize.Level1)
{
    std::shared_ptr<CommonExtensionManager> connectManager = std::make_shared<CommonExtensionManager>(0);
    std::shared_ptr<BaseExtensionRecord> abilityRecord = serviceRecord_;
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.deviceId = "id";
    abilityRequest.abilityInfo.bundleName = "bundle";
    abilityRequest.abilityInfo.name = "name";
    abilityRequest.abilityInfo.moduleName = "module";
    std::string stringUri = "id/bundle/module/name";
    AppExecFwk::ElementName element(abilityRequest.abilityInfo.deviceId, abilityRequest.abilityInfo.bundleName,
        abilityRequest.abilityInfo.name, abilityRequest.abilityInfo.moduleName);
    EXPECT_EQ(element.GetURI(), stringUri);
    abilityRecord->currentState_ = AbilityState::ACTIVE;
    connectManager->serviceMap_.emplace(stringUri, abilityRecord);
    int res = connectManager->StartAbilityLocked(abilityRequest);
    EXPECT_EQ(res, ERR_OK);
}

/*
 * Feature: AbilityConnectManager
 * Function: ConnectAbilityLocked
 * SubFunction: ConnectAbilityLocked
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityConnectManager ConnectAbilityLocked
 */
HWTEST_F(AbilityConnectManagerTest, AAFwk_AbilityMS_ConnectAbilityLocked_001, TestSize.Level1)
{
    std::shared_ptr<CommonExtensionManager> connectManager = std::make_shared<CommonExtensionManager>(0);
    ASSERT_NE(connectManager, nullptr);
    GetCommonExtensionManager()->SetTaskHandler(TaskHandler());
    GetCommonExtensionManager()->SetEventHandler(EventHandler());
    std::shared_ptr<BaseExtensionRecord> abilityRecord = serviceRecord_;
    AbilityRequest abilityRequest;
    sptr<IRemoteObject> callerToken = abilityRecord->GetToken();
    OHOS::sptr<IAbilityConnection> connect = new AbilityConnectCallback();
    OHOS::sptr<IAbilityConnection> callback1 = new AbilityConnectCallback();
    OHOS::sptr<IAbilityConnection> callback2 = new AbilityConnectCallback();
    std::shared_ptr<ConnectionRecord> connection1 =
        std::make_shared<ConnectionRecord>(callerToken, abilityRecord, callback1, nullptr);
    std::shared_ptr<ConnectionRecord> connection2 =
        std::make_shared<ConnectionRecord>(callerToken, abilityRecord, callback2, nullptr);
    abilityRequest.abilityInfo.deviceId = "id";
    abilityRequest.abilityInfo.bundleName = "bundle";
    abilityRequest.abilityInfo.name = "name";
    abilityRequest.abilityInfo.moduleName = "module";
    std::string stringUri = "id/bundle/name/module";
    abilityRecord->currentState_ = AbilityState::ACTIVE;
    abilityRecord->AddConnectRecordToList(connection1);
    connectManager->serviceMap_.emplace(stringUri, abilityRecord);
    connectManager->connectMap_.clear();
    connectManager->ConnectAbilityLocked(abilityRequest, connect, callerToken);
    abilityRecord->AddConnectRecordToList(connection2);
    connectManager->ConnectAbilityLocked(abilityRequest, connect, callerToken);
    connectManager->SetEventHandler(nullptr);
    connectManager->ConnectAbilityLocked(abilityRequest, connect, callerToken);
}

/*
 * Feature: AbilityConnectManager
 * Function: AttachAbilityThreadLocked
 * SubFunction: AttachAbilityThreadLocked
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityConnectManager AttachAbilityThreadLocked
 */
HWTEST_F(AbilityConnectManagerTest, AAFwk_AbilityMS_AttachAbilityThreadLocked_001, TestSize.Level1)
{
    std::shared_ptr<AbilityConnectManager> connectManager = std::make_shared<AbilityConnectManager>(0);
    std::shared_ptr<BaseExtensionRecord> abilityRecord = serviceRecord_;
    sptr<IAbilityScheduler> scheduler = nullptr;
    sptr<IRemoteObject> token = abilityRecord->GetToken();
    connectManager->serviceMap_.emplace("first", abilityRecord);
    connectManager->eventHandler_ = nullptr;
    connectManager->taskHandler_ = nullptr;
    int res = connectManager->AttachAbilityThreadLocked(scheduler, token);
    EXPECT_EQ(res, ERR_OK);
}

/*
 * Feature: AbilityConnectManager
 * Function: OnAppStateChanged
 * SubFunction: OnAppStateChanged
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityConnectManager OnAppStateChanged
 */
HWTEST_F(AbilityConnectManagerTest, AAFwk_AbilityMS_OnAppStateChanged_001, TestSize.Level1)
{
    std::shared_ptr<AbilityConnectManager> connectManager = std::make_shared<AbilityConnectManager>(0);
    ASSERT_NE(connectManager, nullptr);
    std::shared_ptr<BaseExtensionRecord> abilityRecord = serviceRecord_;
    AppInfo info;
    std::string bundleName = "bundleName";
    std::string name = "name";
    int32_t uid = 0;
    info.processName = bundleName;
    abilityRecord->abilityInfo_.applicationInfo.bundleName = bundleName;
    abilityRecord->abilityInfo_.applicationInfo.name = name;
    abilityRecord->abilityInfo_.uid = uid;
    info.appData.push_back({name, uid});
    connectManager->serviceMap_.emplace("first", abilityRecord);
    connectManager->OnAppStateChanged(info);
}

/*
 * Feature: AbilityConnectManager
 * Function: OnAppStateChanged
 * SubFunction: OnAppStateChanged
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityConnectManager OnAppStateChanged
 */
HWTEST_F(AbilityConnectManagerTest, AAFwk_AbilityMS_OnAppStateChanged_002, TestSize.Level1)
{
    std::shared_ptr<AbilityConnectManager> connectManager = std::make_shared<AbilityConnectManager>(0);
    ASSERT_NE(connectManager, nullptr);
    std::shared_ptr<BaseExtensionRecord> abilityRecord = serviceRecord_;
    AppInfo info;
    std::string bundleName = "bundleName";
    std::string name = "name";
    int32_t uid = 0;
    info.processName = "";
    abilityRecord->abilityInfo_.applicationInfo.bundleName = bundleName;
    abilityRecord->abilityInfo_.applicationInfo.name = name;
    abilityRecord->abilityInfo_.uid = uid;
    info.appData.push_back({name, uid});
    connectManager->serviceMap_.emplace("first", abilityRecord);
    connectManager->serviceMap_.emplace("first", nullptr);
    connectManager->OnAppStateChanged(info);
}

/*
 * Feature: AbilityConnectManager
 * Function: AbilityTransitionDone
 * SubFunction: AbilityTransitionDone
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityConnectManager AbilityTransitionDone
 */
HWTEST_F(AbilityConnectManagerTest, AAFwk_AbilityMS_AbilityTransitionDone_001, TestSize.Level1)
{
    std::shared_ptr<AbilityConnectManager> connectManager = std::make_shared<AbilityConnectManager>(0);
    std::shared_ptr<BaseExtensionRecord> abilityRecord = serviceRecord_;
    sptr<IRemoteObject> token = abilityRecord->GetToken();
    int state = AbilityState::INACTIVE;
    abilityRecord->abilityInfo_.type = AbilityType::PAGE;
    abilityRecord->SetAbilityState(AbilityState::ACTIVE);
    connectManager->serviceMap_.emplace("first", abilityRecord);
    int res1 = connectManager->AbilityTransitionDone(token, state);
    EXPECT_EQ(res1, ERR_INVALID_VALUE);
    state = AbilityState::INITIAL;
    connectManager->MoveToTerminatingMap(abilityRecord);
    int res2 = connectManager->AbilityTransitionDone(token, state);
    EXPECT_EQ(res2, ERR_OK);
}

/*
 * Feature: AbilityConnectManager
 * Function: HandleCommandDestroy
 * SubFunction: HandleCommandDestroy
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify HandleCommandDestroy removes session mapping.
 */
HWTEST_F(AbilityConnectManagerTest, AAFwk_AbilityMS_HandleCommandDestroy_001, TestSize.Level1)
{
    auto connectManager = std::make_shared<AbilityConnectManager>(0);
    ASSERT_NE(connectManager, nullptr);

    sptr<SessionInfo> sessionInfo = new (std::nothrow) SessionInfo();
    ASSERT_NE(sessionInfo, nullptr);
    sptr<IRemoteObject> token = serviceRecord_->GetToken();
    sessionInfo->sessionToken = token;

    sptr<SessionInfo> storedInfo = new (std::nothrow) SessionInfo();
    ASSERT_NE(storedInfo, nullptr);
    storedInfo->callerToken = token;

    connectManager->windowExtensionMap_.emplace(token, AbilityConnectManager::WindowExtMapValType(0, storedInfo));
    EXPECT_EQ(connectManager->windowExtensionMap_.size(), 1u);

    connectManager->HandleCommandDestroy(sessionInfo);
    EXPECT_EQ(connectManager->windowExtensionMap_.size(), 0u);

    connectManager->HandleCommandDestroy(nullptr);
}

/*
 * Feature: AbilityConnectManager
 * Function: OnStartSpecifiedProcessResponse
 * SubFunction: OnStartSpecifiedProcessResponse
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify OnStartSpecifiedProcessResponse pops queue and sets flag.
 */
HWTEST_F(AbilityConnectManagerTest, AAFwk_AbilityMS_OnStartSpecifiedProcessResponse_001, TestSize.Level1)
{
    auto connectManager = std::make_shared<AbilityConnectManager>(0);
    ASSERT_NE(connectManager, nullptr);

    AbilityConnectManager::LoadAbilityContext context;
    context.loadParam = std::make_shared<AbilityRuntime::LoadParam>();
    context.abilityInfo = std::make_shared<AppExecFwk::AbilityInfo>();
    context.appInfo = std::make_shared<AppExecFwk::ApplicationInfo>();
    context.want = std::make_shared<Want>();

    const int32_t requestId = 1;
    connectManager->loadAbilityQueue_.push_back({ { requestId, context } });
    connectManager->OnStartSpecifiedProcessResponse("flag", requestId);

    EXPECT_TRUE(connectManager->loadAbilityQueue_.empty());
    EXPECT_EQ(context.want->GetStringParam("ohoSpecifiedProcessFlag"), "flag");
}

/*
 * Feature: AbilityConnectManager
 * Function: OnStartSpecifiedProcessTimeoutResponse
 * SubFunction: OnStartSpecifiedProcessTimeoutResponse
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify OnStartSpecifiedProcessTimeoutResponse pops queue.
 */
HWTEST_F(AbilityConnectManagerTest, AAFwk_AbilityMS_OnStartSpecifiedProcessTimeoutResponse_001, TestSize.Level1)
{
    auto connectManager = std::make_shared<AbilityConnectManager>(0);
    ASSERT_NE(connectManager, nullptr);

    AbilityConnectManager::LoadAbilityContext context;
    context.loadParam = std::make_shared<AbilityRuntime::LoadParam>();
    context.abilityInfo = std::make_shared<AppExecFwk::AbilityInfo>();
    context.appInfo = std::make_shared<AppExecFwk::ApplicationInfo>();
    context.want = std::make_shared<Want>();

    const int32_t requestId = 2;
    connectManager->loadAbilityQueue_.push_back({ { requestId, context } });
    connectManager->OnStartSpecifiedProcessTimeoutResponse(requestId);

    EXPECT_TRUE(connectManager->loadAbilityQueue_.empty());
}

/*
 * Feature: AbilityConnectManager
 * Function: HandleNotifyAssertFaultDialogDied
 * SubFunction: HandleNotifyAssertFaultDialogDied
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify HandleNotifyAssertFaultDialogDied invalid branches.
 */
HWTEST_F(AbilityConnectManagerTest, AAFwk_AbilityMS_HandleNotifyAssertFaultDialogDied_001, TestSize.Level1)
{
    auto connectManager = std::make_shared<AbilityConnectManager>(0);
    ASSERT_NE(connectManager, nullptr);
    auto abilityRecord = serviceRecord_;

    abilityRecord->abilityInfo_.bundleName = "other.bundle";
    abilityRecord->abilityInfo_.name = "other.name";
    connectManager->HandleNotifyAssertFaultDialogDied(abilityRecord);

    abilityRecord->abilityInfo_.bundleName = "com.ohos.amsdialog";
    abilityRecord->abilityInfo_.name = "AssertFaultDialog";
    Want want;
    want.SetParam(Want::PARAM_ASSERT_FAULT_SESSION_ID, std::string("not_number"));
    abilityRecord->SetWant(want);
    connectManager->HandleNotifyAssertFaultDialogDied(abilityRecord);
}

/*
 * Feature: AbilityConnectManager
 * Function: TerminateOrCacheAbility
 * SubFunction: TerminateOrCacheAbility
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify TerminateOrCacheAbility early return for scene board ability.
 */
HWTEST_F(AbilityConnectManagerTest, AAFwk_AbilityMS_TerminateOrCacheAbility_001, TestSize.Level1)
{
    auto connectManager = std::make_shared<AbilityConnectManager>(0);
    ASSERT_NE(connectManager, nullptr);
    auto abilityRecord = serviceRecord_;
    abilityRecord->abilityInfo_.bundleName = AbilityConfig::SCENEBOARD_BUNDLE_NAME;
    abilityRecord->abilityInfo_.name = AbilityConfig::SCENEBOARD_ABILITY_NAME;

    connectManager->serviceMap_.emplace("sceneBoard", abilityRecord);
    connectManager->TerminateOrCacheAbility(abilityRecord);
    EXPECT_EQ(connectManager->serviceMap_.count("sceneBoard"), 1u);
}

/*
 * Feature: AbilityConnectManager
 * Function: ScheduleConnectAbilityDoneLocked
 * SubFunction: ScheduleConnectAbilityDoneLocked
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityConnectManager ScheduleConnectAbilityDoneLocked
 */
HWTEST_F(AbilityConnectManagerTest, AAFwk_AbilityMS_ScheduleConnectAbilityDoneLocked_001, TestSize.Level1)
{
    std::shared_ptr<AbilityConnectManager> connectManager = std::make_shared<AbilityConnectManager>(0);
    std::shared_ptr<BaseExtensionRecord> abilityRecord = serviceRecord_;
    sptr<IRemoteObject> token = abilityRecord->GetToken();
    sptr<IRemoteObject> remoteObject = nullptr;
    abilityRecord->abilityInfo_.type = AbilityType::PAGE;
    abilityRecord->SetAbilityState(AbilityState::INACTIVE);
    connectManager->serviceMap_.emplace("first", abilityRecord);
    int res = connectManager->ScheduleConnectAbilityDoneLocked(token, remoteObject);
    EXPECT_EQ(res, ERR_OK);
}

/*
 * Feature: AbilityConnectManager
 * Function: ScheduleDisconnectAbilityDoneLocked
 * SubFunction: ScheduleDisconnectAbilityDoneLocked
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityConnectManager ScheduleDisconnectAbilityDoneLocked
 */
HWTEST_F(AbilityConnectManagerTest, AAFwk_AbilityMS_ScheduleDisconnectAbilityDoneLocked_001, TestSize.Level1)
{
    std::shared_ptr<AbilityConnectManager> connectManager = std::make_shared<AbilityConnectManager>(0);
    std::shared_ptr<BaseExtensionRecord> abilityRecord = serviceRecord_;
    OHOS::sptr<IAbilityConnection> callback = new AbilityConnectCallback();
    std::shared_ptr<ConnectionRecord> connection =
        std::make_shared<ConnectionRecord>(abilityRecord->GetToken(), abilityRecord, callback, nullptr);
    sptr<IRemoteObject> token = abilityRecord->GetToken();
    connection->SetConnectState(ConnectionState::DISCONNECTING);
    abilityRecord->abilityInfo_.type = AbilityType::PAGE;
    abilityRecord->SetAbilityState(AbilityState::INACTIVE);
    abilityRecord->connRecordList_.push_back(connection);
    connectManager->serviceMap_.emplace("first", abilityRecord);
    int res = connectManager->ScheduleDisconnectAbilityDoneLocked(token);
    EXPECT_EQ(res, INVALID_CONNECTION_STATE);
    abilityRecord->AddStartId();
    abilityRecord->SetAbilityState(AbilityState::ACTIVE);
    connectManager->serviceMap_.emplace("first", abilityRecord);
    res = connectManager->ScheduleDisconnectAbilityDoneLocked(token);
    EXPECT_EQ(res, ERR_OK);
}

/*
 * Feature: AbilityConnectManager
 * Function: ScheduleCommandAbilityDoneLocked
 * SubFunction: ScheduleCommandAbilityDoneLocked
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityConnectManager ScheduleCommandAbilityDoneLocked
 */
HWTEST_F(AbilityConnectManagerTest, AAFwk_AbilityMS_ScheduleCommandAbilityDoneLocked_001, TestSize.Level1)
{
    std::shared_ptr<AbilityConnectManager> connectManager = std::make_shared<AbilityConnectManager>(0);
    std::shared_ptr<BaseExtensionRecord> abilityRecord = serviceRecord_;
    sptr<IRemoteObject> token = abilityRecord->GetToken();
    abilityRecord->abilityInfo_.type = AbilityType::PAGE;
    abilityRecord->SetAbilityState(AbilityState::INACTIVE);
    abilityRecord->connRecordList_.clear();
    connectManager->serviceMap_.emplace("first", abilityRecord);
    int res = connectManager->ScheduleCommandAbilityDoneLocked(token);
    EXPECT_EQ(res, ERR_OK);
}

/*
 * Feature: AbilityConnectManager
 * Function: CompleteCommandAbility
 * SubFunction: CompleteCommandAbility
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityConnectManager CompleteCommandAbility
 */
HWTEST_F(AbilityConnectManagerTest, AAFwk_AbilityMS_CompleteCommandAbility_001, TestSize.Level1)
{
    std::shared_ptr<AbilityConnectManager> connectManager = std::make_shared<AbilityConnectManager>(0);
    std::shared_ptr<BaseExtensionRecord> abilityRecord = serviceRecord_;
    connectManager->taskHandler_ = nullptr;
    connectManager->CompleteCommandAbility(abilityRecord);
    EXPECT_TRUE(abilityRecord->IsAbilityState(AbilityState::ACTIVE));
}

/*
 * Feature: AbilityConnectManager
 * Function: GetServiceRecordByElementName
 * SubFunction: GetServiceRecordByElementName
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityConnectManager GetServiceRecordByElementName
 */
HWTEST_F(AbilityConnectManagerTest, AAFwk_AbilityMS_GetServiceRecordByElementName_001, TestSize.Level1)
{
    std::shared_ptr<AbilityConnectManager> connectManager = std::make_shared<AbilityConnectManager>(0);
    std::shared_ptr<BaseExtensionRecord> abilityRecord = serviceRecord_;
    std::string element = "first";
    connectManager->serviceMap_.emplace(element, abilityRecord);
    auto res = connectManager->GetServiceRecordByElementName(element);
    EXPECT_NE(res, nullptr);
}

/*
 * Feature: AbilityConnectManager
 * Function: GetExtensionByTokenFromServiceMap
 * SubFunction: GetExtensionByTokenFromServiceMap
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityConnectManager GetExtensionByTokenFromServiceMap
 */
HWTEST_F(AbilityConnectManagerTest, AAFwk_AbilityMS_GetExtensionByTokenFromServiceMap_001, TestSize.Level1)
{
    std::shared_ptr<AbilityConnectManager> connectManager = std::make_shared<AbilityConnectManager>(0);
    std::shared_ptr<BaseExtensionRecord> abilityRecord = serviceRecord_;
    sptr<IRemoteObject> token = abilityRecord->GetToken();
    connectManager->serviceMap_.emplace("first", nullptr);
    auto res = connectManager->GetExtensionByTokenFromServiceMap(token);
    EXPECT_EQ(res, nullptr);
}

/*
 * Feature: AbilityConnectManager
 * Function: GetConnectRecordListByCallback
 * SubFunction: GetConnectRecordListByCallback
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityConnectManager GetConnectRecordListByCallback
 */
HWTEST_F(AbilityConnectManagerTest, AAFwk_AbilityMS_GetConnectRecordListByCallback_001, TestSize.Level1)
{
    std::shared_ptr<AbilityConnectManager> connectManager = std::make_shared<AbilityConnectManager>(0);
    std::shared_ptr<BaseExtensionRecord> abilityRecord = serviceRecord_;
    sptr<IAbilityConnection> callback = new AbilityConnectCallback();
    connectManager->connectMap_.clear();
    auto res = connectManager->GetConnectRecordListByCallback(callback);
    EXPECT_EQ(res.size(), 0u);
}

/*
 * Feature: AbilityConnectManager
 * Function: GetExtensionByIdFromServiceMap
 * SubFunction: GetExtensionByIdFromServiceMap
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityConnectManager GetExtensionByIdFromServiceMap
 */
HWTEST_F(AbilityConnectManagerTest, AAFwk_AbilityMS_GetAbilityRecordById_001, TestSize.Level1)
{
    std::shared_ptr<AbilityConnectManager> connectManager = std::make_shared<AbilityConnectManager>(0);
    std::shared_ptr<BaseExtensionRecord> abilityRecord = serviceRecord_;
    int64_t abilityRecordId = abilityRecord->GetRecordId();
    connectManager->serviceMap_.emplace("first", abilityRecord);
    connectManager->serviceMap_.emplace("second", nullptr);
    auto res = connectManager->GetExtensionByIdFromServiceMap(abilityRecordId);
    EXPECT_NE(res, nullptr);
}

/*
 * Feature: AbilityConnectManager
 * Function: LoadAbility
 * SubFunction: LoadAbility
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityConnectManager LoadAbility
 */
HWTEST_F(AbilityConnectManagerTest, AAFwk_AbilityMS_LoadAbility_001, TestSize.Level1)
{
    std::shared_ptr<AbilityConnectManager> connectManager = std::make_shared<AbilityConnectManager>(0);
    std::shared_ptr<BaseExtensionRecord> abilityRecord = serviceRecord_;
    abilityRecord->isLauncherRoot_ = true;
    abilityRecord->isRestarting_ = true;
    abilityRecord->isLauncherAbility_ = true;
    abilityRecord->restartCount_ = -1;
    EXPECT_FALSE(abilityRecord->CanRestartRootLauncher());
    connectManager->LoadAbility(abilityRecord);
}

/*
 * Feature: AbilityConnectManager
 * Function: LoadAbility
 * SubFunction: LoadAbility
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityConnectManager LoadAbility
 */
HWTEST_F(AbilityConnectManagerTest, AAFwk_AbilityMS_LoadAbility_002, TestSize.Level1)
{
    std::shared_ptr<AbilityConnectManager> connectManager = std::make_shared<AbilityConnectManager>(0);
    std::shared_ptr<BaseExtensionRecord> abilityRecord = serviceRecord_;
    std::shared_ptr<CallerRecord> caller1 = std::make_shared<CallerRecord>(0, abilityRecord);
    std::shared_ptr<CallerRecord> caller2 = std::make_shared<CallerRecord>();
    abilityRecord->isLauncherRoot_ = false;
    abilityRecord->isCreateByConnect_ = false;
    abilityRecord->callerList_.push_back(caller1);
    EXPECT_TRUE(abilityRecord->CanRestartRootLauncher());
    connectManager->LoadAbility(abilityRecord);
    abilityRecord->callerList_.push_back(caller2);
    connectManager->LoadAbility(abilityRecord);
    abilityRecord->callerList_.push_back(nullptr);
    connectManager->LoadAbility(abilityRecord);
}

/*
 * Feature: AbilityConnectManager
 * Function: PostTimeOutTask
 * SubFunction: PostTimeOutTask
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityConnectManager PostTimeOutTask
 */
HWTEST_F(AbilityConnectManagerTest, AAFwk_AbilityMS_PostTimeOutTask_001, TestSize.Level1)
{
    std::shared_ptr<AbilityConnectManager> connectManager = std::make_shared<AbilityConnectManager>(0);
    ASSERT_NE(connectManager, nullptr);
    std::shared_ptr<BaseExtensionRecord> abilityRecord = serviceRecord_;
    uint32_t messageId = 2;
    connectManager->PostTimeOutTask(abilityRecord, messageId);
}

/*
 * Feature: AbilityConnectManager
 * Function: HandleStartTimeoutTask
 * SubFunction: HandleStartTimeoutTask
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityConnectManager HandleStartTimeoutTask
 */
HWTEST_F(AbilityConnectManagerTest, AAFwk_AbilityMS_HandleStartTimeoutTask_001, TestSize.Level1)
{
    std::shared_ptr<AbilityConnectManager> connectManager = std::make_shared<AbilityConnectManager>(0);
    ASSERT_NE(connectManager, nullptr);
    std::shared_ptr<BaseExtensionRecord> abilityRecord = serviceRecord_;
    abilityRecord->abilityInfo_.name = "abilityName";
    connectManager->HandleStartTimeoutTask(abilityRecord);
}

/*
 * Feature: AbilityConnectManager
 * Function: HandleStartTimeoutTask
 * SubFunction: HandleStartTimeoutTask
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityConnectManager HandleStartTimeoutTask
 */
HWTEST_F(AbilityConnectManagerTest, AAFwk_AbilityMS_HandleStartTimeoutTask_002, TestSize.Level1)
{
    std::shared_ptr<AbilityConnectManager> connectManager = std::make_shared<AbilityConnectManager>(0);
    ASSERT_NE(connectManager, nullptr);
    std::shared_ptr<BaseExtensionRecord> abilityRecord = serviceRecord_;
    abilityRecord->abilityInfo_.name = AbilityConfig::LAUNCHER_ABILITY_NAME;
    connectManager->HandleStartTimeoutTask(abilityRecord);
}

/*
 * Feature: AbilityConnectManager
 * Function: HandleConnectTimeoutTask
 * SubFunction: HandleConnectTimeoutTask
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityConnectManager HandleConnectTimeoutTask
 */
HWTEST_F(AbilityConnectManagerTest, AAFwk_AbilityMS_HandleConnectTimeoutTask_001, TestSize.Level1)
{
    std::shared_ptr<AbilityConnectManager> connectManager = std::make_shared<AbilityConnectManager>(0);
    ASSERT_NE(connectManager, nullptr);
    std::shared_ptr<BaseExtensionRecord> abilityRecord = serviceRecord_;
    abilityRecord->abilityInfo_.name = AbilityConfig::LAUNCHER_ABILITY_NAME;
    connectManager->HandleConnectTimeoutTask(abilityRecord);
}

/*
 * Feature: AbilityConnectManager
 * Function: HandleCommandTimeoutTask
 * SubFunction: HandleCommandTimeoutTask
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityConnectManager HandleCommandTimeoutTask
 */
HWTEST_F(AbilityConnectManagerTest, AAFwk_AbilityMS_HandleCommandTimeoutTask_001, TestSize.Level1)
{
    std::shared_ptr<AbilityConnectManager> connectManager = std::make_shared<AbilityConnectManager>(0);
    ASSERT_NE(connectManager, nullptr);
    std::shared_ptr<BaseExtensionRecord> abilityRecord = serviceRecord_;
    abilityRecord->abilityInfo_.name = AbilityConfig::LAUNCHER_ABILITY_NAME;
    connectManager->HandleCommandTimeoutTask(abilityRecord);
    abilityRecord->abilityInfo_.name = "abilityName";
    connectManager->HandleCommandTimeoutTask(abilityRecord);
}

/*
 * Feature: AbilityConnectManager
 * Function: HandleTerminateDisconnectTask
 * SubFunction: HandleTerminateDisconnectTask
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityConnectManager HandleTerminateDisconnectTask
 */
HWTEST_F(AbilityConnectManagerTest, AAFwk_AbilityMS_HandleTerminateDisconnectTask_001, TestSize.Level1)
{
    std::shared_ptr<AbilityConnectManager> connectManager = std::make_shared<AbilityConnectManager>(0);
    ASSERT_NE(connectManager, nullptr);
    OHOS::sptr<IAbilityConnection> callback = new AbilityConnectCallback();
    std::shared_ptr<ConnectionRecord> connection =
        std::make_shared<ConnectionRecord>(nullptr, nullptr, callback, nullptr);
    AbilityConnectManager::ConnectListType connectlist;
    connectlist.push_back(nullptr);
    connectlist.push_back(connection);
    connectManager->HandleTerminateDisconnectTask(connectlist);
}

/*
 * Feature: AbilityConnectManager
 * Function: DispatchInactive
 * SubFunction: DispatchInactive
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityConnectManager DispatchInactive
 */
HWTEST_F(AbilityConnectManagerTest, AAFwk_AbilityMS_DispatchInactive_001, TestSize.Level1)
{
    std::shared_ptr<AbilityConnectManager> connectManager = std::make_shared<AbilityConnectManager>(0);
    std::shared_ptr<BaseExtensionRecord> abilityRecord = serviceRecord_;
    int state = 0;
    abilityRecord->SetAbilityState(AbilityState::INACTIVATING);
    abilityRecord->isCreateByConnect_ = false;
    connectManager->SetTaskHandler(TaskHandler());
    connectManager->SetEventHandler(EventHandler());
    int res = connectManager->DispatchInactive(abilityRecord, state, abilityRecord->GetToken());
    EXPECT_EQ(res, ERR_OK);
}

/*
 * Feature: AbilityConnectManager
 * Function: DispatchTerminate
 * SubFunction: DispatchTerminate
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityConnectManager DispatchTerminate
 */
HWTEST_F(AbilityConnectManagerTest, AAFwk_AbilityMS_DispatchTerminate_001, TestSize.Level1)
{
    std::shared_ptr<AbilityConnectManager> connectManager = std::make_shared<AbilityConnectManager>(0);
    std::shared_ptr<BaseExtensionRecord> abilityRecord = serviceRecord_;
    connectManager->SetTaskHandler(TaskHandler());
    connectManager->SetEventHandler(EventHandler());
    int res = connectManager->DispatchTerminate(abilityRecord, abilityRecord->GetToken());
    EXPECT_EQ(res, ERR_OK);
}

/*
 * Feature: AbilityConnectManager
 * Function: CommandAbility
 * SubFunction: CommandAbility
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityConnectManager CommandAbility
 */
HWTEST_F(AbilityConnectManagerTest, AAFwk_AbilityMS_CommandAbility_001, TestSize.Level1)
{
    std::shared_ptr<AbilityConnectManager> connectManager = std::make_shared<AbilityConnectManager>(0);
    ASSERT_NE(connectManager, nullptr);
    std::shared_ptr<BaseExtensionRecord> abilityRecord = serviceRecord_;
    connectManager->SetEventHandler(nullptr);
    connectManager->CommandAbility(abilityRecord);
}

/*
 * Feature: AbilityConnectManager
 * Function: TerminateDone
 * SubFunction: TerminateDone
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityConnectManager TerminateDone
 */
HWTEST_F(AbilityConnectManagerTest, AAFwk_AbilityMS_TerminateDone_001, TestSize.Level1)
{
    std::shared_ptr<AbilityConnectManager> connectManager = std::make_shared<AbilityConnectManager>(0);
    ASSERT_NE(connectManager, nullptr);
    std::shared_ptr<BaseExtensionRecord> abilityRecord = serviceRecord_;
    abilityRecord->SetAbilityState(AbilityState::TERMINATING);
    connectManager->TerminateDone(abilityRecord);
}

/*
 * Feature: AbilityConnectManager
 * Function: GetAbilityConnectedRecordFromRecordList
 * SubFunction: GetAbilityConnectedRecordFromRecordList
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityConnectManager GetAbilityConnectedRecordFromRecordList
 */
HWTEST_F(AbilityConnectManagerTest, AAFwk_AbilityMS_GetAbilityConnectedRecordFromRecordList_001, TestSize.Level1)
{
    std::shared_ptr<AbilityConnectManager> connectManager = std::make_shared<AbilityConnectManager>(0);
    std::shared_ptr<BaseExtensionRecord> abilityRecord = serviceRecord_;
    std::list<std::shared_ptr<ConnectionRecord>> connectRecordList;
    OHOS::sptr<IAbilityConnection> callback = new AbilityConnectCallback();
    std::shared_ptr<ConnectionRecord> connection =
        std::make_shared<ConnectionRecord>(abilityRecord->GetToken(), abilityRecord, callback, nullptr);
    connectRecordList.push_back(connection);
    auto res1 = connectManager->GetAbilityConnectedRecordFromRecordList(nullptr, connectRecordList);
    EXPECT_EQ(res1, nullptr);
    connectRecordList.push_back(nullptr);
    auto res2 = connectManager->GetAbilityConnectedRecordFromRecordList(abilityRecord, connectRecordList);
    EXPECT_EQ(res2, connection);
}

/*
 * Feature: AbilityConnectManager
 * Function: RemoveServiceAbility
 * SubFunction: RemoveServiceAbility
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityConnectManager RemoveServiceAbility
 */
HWTEST_F(AbilityConnectManagerTest, AAFwk_AbilityMS_RemoveServiceAbility_001, TestSize.Level1)
{
    std::shared_ptr<AbilityConnectManager> connectManager = std::make_shared<AbilityConnectManager>(0);
    ASSERT_NE(connectManager, nullptr);
    std::shared_ptr<BaseExtensionRecord> abilityRecord = serviceRecord_;
    AbilityInfo abilityInfo;
    abilityRecord->abilityInfo_ = abilityInfo;
    connectManager->serviceMap_.clear();
    connectManager->RemoveServiceAbility(abilityRecord);
}

/*
 * Feature: AbilityConnectManager
 * Function: OnCallBackDied
 * SubFunction: OnCallBackDied
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityConnectManager OnCallBackDied
 */
HWTEST_F(AbilityConnectManagerTest, AAFwk_AbilityMS_OnCallBackDied_001, TestSize.Level1)
{
    std::shared_ptr<AbilityConnectManager> connectManager = std::make_shared<AbilityConnectManager>(0);
    ASSERT_NE(connectManager, nullptr);
    std::shared_ptr<BaseExtensionRecord> abilityRecord = serviceRecord_;
    wptr<IRemoteObject> remote{ abilityRecord->GetToken() };
    connectManager->SetEventHandler(nullptr);
    connectManager->OnCallBackDied(remote);
}

/*
 * Feature: AbilityConnectManager
 * Function: HandleCallBackDiedTask
 * SubFunction: HandleCallBackDiedTask
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityConnectManager HandleCallBackDiedTask
 */
HWTEST_F(AbilityConnectManagerTest, AAFwk_AbilityMS_HandleCallBackDiedTask_001, TestSize.Level1)
{
    std::shared_ptr<AbilityConnectManager> connectManager = std::make_shared<AbilityConnectManager>(0);
    ASSERT_NE(connectManager, nullptr);
    std::shared_ptr<BaseExtensionRecord> abilityRecord = serviceRecord_;
    sptr<IRemoteObject> connect = abilityRecord->GetToken();
    connectManager->connectMap_.clear();
    connectManager->HandleCallBackDiedTask(connect);
}

/*
 * Feature: AbilityConnectManager
 * Function: OnAbilityDied
 * SubFunction: OnAbilityDied
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityConnectManager OnAbilityDied
 */
HWTEST_F(AbilityConnectManagerTest, AAFwk_AbilityMS_OnAbilityDied_001, TestSize.Level1)
{
    std::shared_ptr<AbilityConnectManager> connectManager = std::make_shared<AbilityConnectManager>(0);
    ASSERT_NE(connectManager, nullptr);
    std::shared_ptr<BaseExtensionRecord> abilityRecord = serviceRecord_;
    abilityRecord->abilityInfo_.type = AbilityType::PAGE;
    connectManager->SetEventHandler(nullptr);
    connectManager->OnAbilityDied(abilityRecord);
    abilityRecord->abilityInfo_.type = AbilityType::EXTENSION;
    connectManager->OnAbilityDied(abilityRecord);
}

/*
 * Feature: AbilityConnectManager
 * Function: OnTimeOut
 * SubFunction: OnTimeOut
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityConnectManager OnTimeOut
 */
HWTEST_F(AbilityConnectManagerTest, AAFwk_AbilityMS_OnTimeOut_001, TestSize.Level1)
{
    std::shared_ptr<AbilityConnectManager> connectManager = std::make_shared<AbilityConnectManager>(0);
    ASSERT_NE(connectManager, nullptr);
    std::shared_ptr<BaseExtensionRecord> abilityRecord = serviceRecord_;
    uint32_t msgId = 2;
    connectManager->serviceMap_.emplace("first", abilityRecord);
    int64_t abilityRecordId = 1;
    connectManager->OnTimeOut(msgId, abilityRecordId);
    msgId = 0;
    connectManager->OnTimeOut(msgId, abilityRecordId);
}

/*
 * Feature: AbilityConnectManager
 * Function: HandleInactiveTimeout
 * SubFunction: HandleInactiveTimeout
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityConnectManager HandleInactiveTimeout
 */
HWTEST_F(AbilityConnectManagerTest, AAFwk_AbilityMS_HandleInactiveTimeout_001, TestSize.Level1)
{
    std::shared_ptr<AbilityConnectManager> connectManager = std::make_shared<AbilityConnectManager>(0);
    ASSERT_NE(connectManager, nullptr);
    std::shared_ptr<BaseExtensionRecord> abilityRecord = serviceRecord_;
    abilityRecord->abilityInfo_.name = AbilityConfig::LAUNCHER_ABILITY_NAME;
    connectManager->HandleInactiveTimeout(abilityRecord);
    abilityRecord->abilityInfo_.name = "abilityName";
    connectManager->HandleInactiveTimeout(abilityRecord);
}

/*
 * Feature: AbilityConnectManager
 * Function: HandleAbilityDiedTask
 * SubFunction: HandleAbilityDiedTask
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityConnectManager HandleAbilityDiedTask
 */
HWTEST_F(AbilityConnectManagerTest, AAFwk_AbilityMS_HandleAbilityDiedTask_001, TestSize.Level1)
{
    std::shared_ptr<AbilityConnectManager> connectManager = std::make_shared<AbilityConnectManager>(0);
    ASSERT_NE(connectManager, nullptr);
    std::shared_ptr<BaseExtensionRecord> abilityRecord = serviceRecord_;
    connectManager->serviceMap_.clear();
    connectManager->HandleAbilityDiedTask(abilityRecord);
}

/*
 * Feature: AbilityConnectManager
 * Function: DisconnectBeforeCleanup
 * SubFunction: DisconnectBeforeCleanup
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityConnectManager DisconnectBeforeCleanup
 */
HWTEST_F(AbilityConnectManagerTest, AAFwk_AbilityMS_DisconnectBeforeCleanup_001, TestSize.Level1)
{
    std::shared_ptr<AbilityConnectManager> connectManager = std::make_shared<AbilityConnectManager>(100);
    ASSERT_NE(connectManager, nullptr);
    GetCommonExtensionManager()->SetTaskHandler(TaskHandler());
    GetCommonExtensionManager()->SetEventHandler(EventHandler());
    serviceRecord1_->SetUid(102 * 200000);
    std::shared_ptr<BaseExtensionRecord> abilityRecord = serviceRecord_;
    AbilityRequest abilityRequest;
    sptr<IRemoteObject> callerToken = abilityRecord->GetToken();
    OHOS::sptr<IAbilityConnection> callback1 = new AbilityConnectCallback();
    std::shared_ptr<ConnectionRecord> connection1 =
        std::make_shared<ConnectionRecord>(callerToken, abilityRecord, callback1, nullptr);
    connection1->AttachCallerInfo();
    abilityRequest.abilityInfo.deviceId = "id";
    abilityRequest.abilityInfo.bundleName = "bundle";
    abilityRequest.abilityInfo.name = "name";
    abilityRequest.abilityInfo.moduleName = "module";
    std::string stringUri = "id/bundle/name/module";
    abilityRecord->currentState_ = AbilityState::ACTIVE;
    abilityRecord->AddConnectRecordToList(connection1);
    connectManager->AddConnectObjectToMap(callback1->AsObject(), abilityRecord->GetConnectRecordList(), false);
    connectManager->serviceMap_.emplace(stringUri, abilityRecord);
    connectManager->DisconnectBeforeCleanup();
    ASSERT_EQ(abilityRecord->GetConnectRecordList().empty(), true);
    ASSERT_EQ(connectManager->GetConnectRecordListByCallback(callback1).empty(), true);
}

/*
 * Feature: AbilityConnectManager
 * Function: RestartAbility
 * SubFunction: RestartAbility
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityConnectManager RestartAbility
 */
HWTEST_F(AbilityConnectManagerTest, AAFwk_AbilityMS_RestartAbility_001, TestSize.Level1)
{
    std::shared_ptr<AbilityConnectManager> connectManager = std::make_shared<AbilityConnectManager>(0);
    ASSERT_NE(connectManager, nullptr);
    std::shared_ptr<BaseExtensionRecord> abilityRecord = serviceRecord_;
    int32_t currentUserId = 0;
    connectManager->userId_ = 1;
    abilityRecord->abilityInfo_.name = AbilityConfig::LAUNCHER_ABILITY_NAME;
    connectManager->RestartAbility(abilityRecord, currentUserId);
    connectManager->userId_ = currentUserId;
    connectManager->RestartAbility(abilityRecord, currentUserId);
}

/*
 * Feature: AbilityConnectManager
 * Function: RestartAbility
 * SubFunction: RestartAbility
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityConnectManager RestartAbility
 */
HWTEST_F(AbilityConnectManagerTest, AAFwk_AbilityMS_RestartAbility_002, TestSize.Level1)
{
    std::shared_ptr<AbilityConnectManager> connectManager = std::make_shared<AbilityConnectManager>(0);
    ASSERT_NE(connectManager, nullptr);
    std::shared_ptr<BaseExtensionRecord> abilityRecord = serviceRecord_;
    int32_t currentUserId = 0;
    connectManager->userId_ = currentUserId;
    abilityRecord->abilityInfo_.name = "abilityName";
    abilityRecord->SetRestartCount(-1);
    connectManager->RestartAbility(abilityRecord, currentUserId);
}

/*
 * Feature: AbilityConnectManager
 * Function: RestartAbility
 * SubFunction: RestartAbility
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityConnectManager RestartAbility
 */
HWTEST_F(AbilityConnectManagerTest, AAFwk_AbilityMS_RestartAbility_003, TestSize.Level1)
{
    std::shared_ptr<AbilityConnectManager> connectManager = std::make_shared<AbilityConnectManager>(0);
    ASSERT_NE(connectManager, nullptr);
    std::shared_ptr<BaseExtensionRecord> abilityRecord = serviceRecord_;
    connectManager->userId_ = 1;
    abilityRecord->abilityInfo_.bundleName = AbilityConfig::SCENEBOARD_BUNDLE_NAME;
    abilityRecord->abilityInfo_.name = AbilityConfig::SCENEBOARD_ABILITY_NAME;
    connectManager->HandleAbilityDiedTask(abilityRecord);
    EXPECT_EQ(static_cast<int>(GetCommonExtensionManager()->GetServiceMap().size()), 0);
}

/*
 * Feature: AbilityConnectManager
 * Function: IsAbilityNeedKeepAlive
 * SubFunction:
 * FunctionPoints: IsAbilityNeedKeepAlive
 * EnvConditions:NA
 * CaseDescription: Verify the IsAbilityNeedKeepAlive need keep alive.
 * @tc.require: issueI6588V
 */
HWTEST_F(AbilityConnectManagerTest, AAFWK_IsAbilityNeedKeepAlive_001, TestSize.Level1)
{
    GetCommonExtensionManager()->SetTaskHandler(TaskHandler());
    GetCommonExtensionManager()->SetEventHandler(EventHandler());

    serviceRecord2_->SetKeepAliveBundle(true);
    // mock bms return
    EXPECT_TRUE(GetCommonExtensionManager()->IsAbilityNeedKeepAlive(serviceRecord2_));
}

/*
 * Feature: AbilityConnectManager
 * Function: RestartAbility
 * SubFunction:
 * FunctionPoints: RestartAbility
 * EnvConditions:NA
 * CaseDescription: Verify ability not restart the normal ability died.
 * @tc.require: issueI6588V
 */
HWTEST_F(AbilityConnectManagerTest, AAFWK_RestartAbility_001, TestSize.Level1)
{
    GetCommonExtensionManager()->SetTaskHandler(TaskHandler());
    GetCommonExtensionManager()->SetEventHandler(EventHandler());

    auto result = GetCommonExtensionManager()->StartAbility(abilityRequest_);
    EXPECT_EQ(OHOS::ERR_OK, result);

    auto elementName = abilityRequest_.want.GetElement().GetURI();
    std::shared_ptr<BaseExtensionRecord> service =
        GetCommonExtensionManager()->GetServiceRecordByElementName(elementName);
    EXPECT_NE(service, nullptr);
    EXPECT_EQ(static_cast<int>(GetCommonExtensionManager()->GetServiceMap().size()), 1);

    // HandleTerminate
    auto task = [service, connectManager = GetCommonExtensionManager()]() {
        connectManager->HandleAbilityDiedTask(service);
    };
    EXPECT_CALL(*taskHandler_, SubmitTaskInner(_, _)).WillRepeatedly(DoAll(SetArgReferee<0>(task),
        testing::Invoke(taskHandler_.get(), &MockTaskHandlerWrap::MockTaskHandler)));
    GetCommonExtensionManager()->OnAbilityDied(service);
    EXPECT_EQ(static_cast<int>(GetCommonExtensionManager()->GetServiceMap().size()), 0);
}

/*
 * Feature: AbilityConnectManager
 * Function: RestartAbility
 * SubFunction:
 * FunctionPoints: RestartAbility
 * EnvConditions:NA
 * CaseDescription: Verify ability restart when the resident ability died.
 * @tc.require: issueI6588V
 */
HWTEST_F(AbilityConnectManagerTest, AAFWK_RestartAbility_002, TestSize.Level1)
{
    GetCommonExtensionManager()->SetTaskHandler(TaskHandler());
    GetCommonExtensionManager()->SetEventHandler(EventHandler());

    auto result = GetCommonExtensionManager()->StartAbility(abilityRequest2_);
    EXPECT_EQ(OHOS::ERR_OK, result);
    WaitUntilTaskDone(TaskHandler());

    auto elementName = abilityRequest2_.want.GetElement().GetURI();
    auto service = GetCommonExtensionManager()->GetServiceRecordByElementName(elementName);
    EXPECT_NE(service, nullptr);
    EXPECT_EQ(static_cast<int>(GetCommonExtensionManager()->GetServiceMap().size()), 1);

    // HandleTerminate
    GetCommonExtensionManager()->HandleAbilityDiedTask(service);
    EXPECT_EQ(static_cast<int>(GetCommonExtensionManager()->GetServiceMap().size()), 0);
}

/*
 * Feature: AbilityConnectManager
 * Function: RestartAbility
 * SubFunction:
 * FunctionPoints: RestartAbility
 * EnvConditions:NA
 * CaseDescription: Verify ability restart when the resident ability died and restart out of max times.
 * @tc.require: issueI6588V
 */
HWTEST_F(AbilityConnectManagerTest, AAFWK_RestartAbility_003, TestSize.Level1)
{
    GetCommonExtensionManager()->SetTaskHandler(TaskHandler());
    GetCommonExtensionManager()->SetEventHandler(EventHandler());

    auto result = GetCommonExtensionManager()->StartAbility(abilityRequest2_);
    EXPECT_EQ(OHOS::ERR_OK, result);
    WaitUntilTaskDone(TaskHandler());

    auto elementName = abilityRequest2_.want.GetElement().GetURI();
    std::shared_ptr<BaseExtensionRecord> service =
        GetCommonExtensionManager()->GetServiceRecordByElementName(elementName);
    EXPECT_NE(service, nullptr);
    EXPECT_EQ(static_cast<int>(GetCommonExtensionManager()->GetServiceMap().size()), 1);
    // set the over interval time according the config; without init config, the interval time is 0.
    // ensure now - restartTime < intervalTime
    service->SetRestartTime(AbilityUtil::SystemTimeMillis() + 1000);

    // HandleTerminate
    GetCommonExtensionManager()->HandleAbilityDiedTask(service);
    EXPECT_EQ(static_cast<int>(GetCommonExtensionManager()->GetServiceMap().size()), 0);
}

/*
 * Feature: AbilityConnectManager
 * Function: PostRestartResidentTask
 * SubFunction:
 * FunctionPoints: PostRestartResidentTask
 * EnvConditions:NA
 * CaseDescription: Verify the PostRestartResidentTask process.
 * @tc.require: issueI6588V
 */
HWTEST_F(AbilityConnectManagerTest, AAFWK_PostRestartResidentTask_001, TestSize.Level1)
{
    GetCommonExtensionManager()->SetTaskHandler(TaskHandler());
    GetCommonExtensionManager()->SetEventHandler(EventHandler());

    GetCommonExtensionManager()->PostRestartResidentTask(abilityRequest2_);
    WaitUntilTaskDone(TaskHandler());
    EXPECT_EQ(static_cast<int>(GetCommonExtensionManager()->GetServiceMap().size()), 0);
}

/*
 * Feature: AbilityConnectManager
 * Function: StartAbility
 * SubFunction: NA
 * FunctionPoints: StartAbility
 * EnvConditions:NA
 * CaseDescription: Verify the normal process of startability with session info
 */
HWTEST_F(AbilityConnectManagerTest, AAFWK_Start_Service_With_SessionInfo_001, TestSize.Level1)
{
    GetCommonExtensionManager()->SetTaskHandler(TaskHandler());
    GetCommonExtensionManager()->SetEventHandler(EventHandler());

    auto sessionInfo = MockSessionInfo(0);
    abilityRequest_.sessionInfo = sessionInfo;
    auto result = GetCommonExtensionManager()->StartAbility(abilityRequest_);
    EXPECT_EQ(OHOS::ERR_OK, result);
    abilityRequest_.sessionInfo = nullptr;
    WaitUntilTaskDone(TaskHandler());
}

/*
 * Feature: AbilityConnectManager
 * Function: StartAbilityLocked
 * SubFunction: StartAbilityLocked
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityConnectManager StartAbilityLocked with session info
 */
HWTEST_F(AbilityConnectManagerTest, AAFwk_AbilityMS_StartAbilityLocked_With_SessionInfo_001, TestSize.Level1)
{
    std::shared_ptr<CommonExtensionManager> connectManager = std::make_shared<CommonExtensionManager>(0);
    std::shared_ptr<BaseExtensionRecord> abilityRecord = serviceRecord_;
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.deviceId = "id";
    abilityRequest.abilityInfo.bundleName = "bundle";
    abilityRequest.abilityInfo.name = "name";
    abilityRequest.abilityInfo.moduleName = "module";
    std::string stringUri = "id/bundle/module/name";
    AppExecFwk::ElementName element(abilityRequest.abilityInfo.deviceId, abilityRequest.abilityInfo.bundleName,
        abilityRequest.abilityInfo.name, abilityRequest.abilityInfo.moduleName);
    EXPECT_EQ(element.GetURI(), stringUri);
    abilityRecord->currentState_ = AbilityState::ACTIVE;
    connectManager->serviceMap_.emplace(stringUri, abilityRecord);
    abilityRequest.sessionInfo = MockSessionInfo(0);
    int res = connectManager->StartAbilityLocked(abilityRequest);
    EXPECT_EQ(res, ERR_OK);
}

/*
 * Feature: MissionListManager
 * Function: MoveToBackground
 * SubFunction: NA
 * FunctionPoints: MissionListManager MoveToBackground
 * EnvConditions: NA
 * CaseDescription: Verify MoveToBackground
 */
HWTEST_F(AbilityConnectManagerTest, MoveToBackground_001, TestSize.Level1)
{
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(3);
    ASSERT_NE(connectManager, nullptr);
    std::shared_ptr<BaseExtensionRecord> abilityRecord;
    connectManager->MoveToBackground(abilityRecord);
    connectManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: MoveToBackground
 * SubFunction: NA
 * FunctionPoints: MissionListManager MoveToBackground
 * EnvConditions: NA
 * CaseDescription: Verify MoveToBackground
 */
HWTEST_F(AbilityConnectManagerTest, MoveToBackground_002, TestSize.Level1)
{
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(3);
    ASSERT_NE(connectManager, nullptr);
    std::shared_ptr<BaseExtensionRecord> abilityRecord = InitAbilityRecord();
    abilityRecord->lifeCycleStateInfo_.sceneFlag = 1;
    connectManager->MoveToBackground(abilityRecord);
    connectManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: MoveToBackground
 * SubFunction: NA
 * FunctionPoints: MissionListManager MoveToBackground
 * EnvConditions: NA
 * CaseDescription: Verify MoveToBackground
 */
HWTEST_F(AbilityConnectManagerTest, MoveToBackground_003, TestSize.Level1)
{
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(3);
    ASSERT_NE(connectManager, nullptr);
    std::shared_ptr<BaseExtensionRecord> abilityRecord = InitAbilityRecord();
    abilityRecord->lifeCycleStateInfo_.sceneFlag = 2;
    abilityRecord->SetClearMissionFlag(true);
    connectManager->MoveToBackground(abilityRecord);
    connectManager.reset();
}


/*
 * Feature: MissionListManager
 * Function: CompleteBackground
 * SubFunction: NA
 * FunctionPoints: MissionListManager CompleteBackground
 * EnvConditions: NA
 * CaseDescription: Verify CompleteBackground
 */
HWTEST_F(AbilityConnectManagerTest, CompleteBackground_001, TestSize.Level1)
{
    std::shared_ptr<AbilityConnectManager> connectManager = std::make_shared<AbilityConnectManager>(3);
    ASSERT_NE(connectManager, nullptr);
    std::shared_ptr<BaseExtensionRecord> abilityRecord = InitAbilityRecord();
    abilityRecord->currentState_ = AbilityState::FOREGROUND;
    connectManager->CompleteBackground(abilityRecord);
    connectManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: CompleteBackground
 * SubFunction: NA
 * FunctionPoints: MissionListManager CompleteBackground
 * EnvConditions: NA
 * CaseDescription: Verify CompleteBackground
 */
HWTEST_F(AbilityConnectManagerTest, CompleteBackground_002, TestSize.Level1)
{
    std::shared_ptr<AbilityConnectManager> connectManager = std::make_shared<AbilityConnectManager>(3);
    ASSERT_NE(connectManager, nullptr);
    std::shared_ptr<BaseExtensionRecord> abilityRecord = InitAbilityRecord();
    abilityRecord->currentState_ = AbilityState::BACKGROUNDING;
    abilityRecord->SetPendingState(AbilityState::FOREGROUND);
    abilityRecord->SetSwitchingPause(true);
    connectManager->CompleteBackground(abilityRecord);
    connectManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: CompleteBackground
 * SubFunction: NA
 * FunctionPoints: MissionListManager CompleteBackground
 * EnvConditions: NA
 * CaseDescription: Verify CompleteBackground
 */
HWTEST_F(AbilityConnectManagerTest, CompleteBackground_003, TestSize.Level1)
{
    std::shared_ptr<AbilityConnectManager> connectManager = std::make_shared<AbilityConnectManager>(3);
    ASSERT_NE(connectManager, nullptr);
    std::shared_ptr<BaseExtensionRecord> abilityRecord = InitAbilityRecord();
    abilityRecord->currentState_ = AbilityState::BACKGROUNDING;
    abilityRecord->SetPendingState(AbilityState::BACKGROUND);
    abilityRecord->SetSwitchingPause(false);
    abilityRecord->SetStartedByCall(true);
    abilityRecord->SetStartToBackground(true);
    abilityRecord->isReady_ = true;
    connectManager->CompleteBackground(abilityRecord);
    connectManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: CompleteBackground
 * SubFunction: NA
 * FunctionPoints: MissionListManager CompleteBackground
 * EnvConditions: NA
 * CaseDescription: Verify CompleteBackground
 */
HWTEST_F(AbilityConnectManagerTest, CompleteBackground_004, TestSize.Level1)
{
    std::shared_ptr<AbilityConnectManager> connectManager = std::make_shared<AbilityConnectManager>(3);
    ASSERT_NE(connectManager, nullptr);
    std::shared_ptr<BaseExtensionRecord> abilityRecord = InitAbilityRecord();
    std::shared_ptr<BaseExtensionRecord> abilityRecord2 = InitAbilityRecord();
    abilityRecord->currentState_ = AbilityState::BACKGROUNDING;
    abilityRecord->SetPendingState(AbilityState::BACKGROUND);
    abilityRecord->SetSwitchingPause(false);
    abilityRecord->SetStartedByCall(false);
    abilityRecord->SetStartToBackground(true);
    abilityRecord->isReady_ = true;
    abilityRecord2->currentState_ = AbilityState::BACKGROUND;
    connectManager->CompleteBackground(abilityRecord);
    connectManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: CompleteBackground
 * SubFunction: NA
 * FunctionPoints: MissionListManager CompleteBackground
 * EnvConditions: NA
 * CaseDescription: Verify CompleteBackground
 */
HWTEST_F(AbilityConnectManagerTest, CompleteBackground_005, TestSize.Level1)
{
    std::shared_ptr<AbilityConnectManager> connectManager = std::make_shared<AbilityConnectManager>(3);
    ASSERT_NE(connectManager, nullptr);
    std::shared_ptr<BaseExtensionRecord> abilityRecord = InitAbilityRecord();
    std::shared_ptr<BaseExtensionRecord> abilityRecord2 = InitAbilityRecord();
    abilityRecord->currentState_ = AbilityState::BACKGROUNDING;
    abilityRecord->SetPendingState(AbilityState::BACKGROUND);
    abilityRecord->SetSwitchingPause(false);
    abilityRecord->SetStartedByCall(true);
    abilityRecord->SetStartToBackground(false);
    abilityRecord->isReady_ = true;
    abilityRecord2->currentState_ = AbilityState::BACKGROUND;
    connectManager->CompleteBackground(abilityRecord);
    connectManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: CompleteBackground
 * SubFunction: NA
 * FunctionPoints: MissionListManager CompleteBackground
 * EnvConditions: NA
 * CaseDescription: Verify CompleteBackground
 */
HWTEST_F(AbilityConnectManagerTest, CompleteBackground_006, TestSize.Level1)
{
    std::shared_ptr<AbilityConnectManager> connectManager = std::make_shared<AbilityConnectManager>(3);
    ASSERT_NE(connectManager, nullptr);
    std::shared_ptr<BaseExtensionRecord> abilityRecord = InitAbilityRecord();
    std::shared_ptr<BaseExtensionRecord> abilityRecord2 = InitAbilityRecord();
    abilityRecord->currentState_ = AbilityState::BACKGROUNDING;
    abilityRecord->SetPendingState(AbilityState::BACKGROUND);
    abilityRecord->SetSwitchingPause(false);
    abilityRecord->SetStartedByCall(true);
    abilityRecord->SetStartToBackground(true);
    abilityRecord->isReady_ = false;
    abilityRecord2->currentState_ = AbilityState::FOREGROUND;
    connectManager->CompleteBackground(abilityRecord);
    connectManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: PrintTimeOutLog
 * SubFunction: NA
 * FunctionPoints: MissionListManager PrintTimeOutLog
 * EnvConditions: NA
 * CaseDescription: Verify PrintTimeOutLog
 */
HWTEST_F(AbilityConnectManagerTest, PrintTimeOutLog_001, TestSize.Level1)
{
    std::shared_ptr<AbilityConnectManager> connectManager = std::make_shared<AbilityConnectManager>(3);
    ASSERT_NE(connectManager, nullptr);
    uint32_t msgId = 0;
    connectManager->PrintTimeOutLog(nullptr, msgId);
    connectManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: PrintTimeOutLog
 * SubFunction: NA
 * FunctionPoints: MissionListManager PrintTimeOutLog
 * EnvConditions: NA
 * CaseDescription: Verify PrintTimeOutLog
 */
HWTEST_F(AbilityConnectManagerTest, PrintTimeOutLog_002, TestSize.Level1)
{
    std::shared_ptr<AbilityConnectManager> connectManager = std::make_shared<AbilityConnectManager>(3);
    ASSERT_NE(connectManager, nullptr);
    std::shared_ptr<BaseExtensionRecord> abilityRecord = InitAbilityRecord();
    uint32_t msgId = 0;
    connectManager->PrintTimeOutLog(abilityRecord, msgId);
    connectManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: PrintTimeOutLog
 * SubFunction: NA
 * FunctionPoints: MissionListManager PrintTimeOutLog
 * EnvConditions: NA
 * CaseDescription: Verify PrintTimeOutLog
 */
HWTEST_F(AbilityConnectManagerTest, PrintTimeOutLog_003, TestSize.Level1)
{
    std::shared_ptr<AbilityConnectManager> connectManager = std::make_shared<AbilityConnectManager>(3);
    ASSERT_NE(connectManager, nullptr);
    std::shared_ptr<BaseExtensionRecord> abilityRecord = InitAbilityRecord();
    uint32_t msgId = 1;
    connectManager->PrintTimeOutLog(abilityRecord, msgId);
    connectManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: PrintTimeOutLog
 * SubFunction: NA
 * FunctionPoints: MissionListManager PrintTimeOutLog
 * EnvConditions: NA
 * CaseDescription: Verify PrintTimeOutLog
 */
HWTEST_F(AbilityConnectManagerTest, PrintTimeOutLog_004, TestSize.Level1)
{
    std::shared_ptr<AbilityConnectManager> connectManager = std::make_shared<AbilityConnectManager>(3);
    ASSERT_NE(connectManager, nullptr);
    std::shared_ptr<BaseExtensionRecord> abilityRecord = InitAbilityRecord();
    uint32_t msgId = 2;
    connectManager->PrintTimeOutLog(abilityRecord, msgId);
    connectManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: PrintTimeOutLog
 * SubFunction: NA
 * FunctionPoints: MissionListManager PrintTimeOutLog
 * EnvConditions: NA
 * CaseDescription: Verify PrintTimeOutLog
 */
HWTEST_F(AbilityConnectManagerTest, PrintTimeOutLog_005, TestSize.Level1)
{
    std::shared_ptr<AbilityConnectManager> connectManager = std::make_shared<AbilityConnectManager>(3);
    ASSERT_NE(connectManager, nullptr);
    std::shared_ptr<BaseExtensionRecord> abilityRecord = InitAbilityRecord();
    uint32_t msgId = 4;
    connectManager->PrintTimeOutLog(abilityRecord, msgId);
    connectManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: PrintTimeOutLog
 * SubFunction: NA
 * FunctionPoints: MissionListManager PrintTimeOutLog
 * EnvConditions: NA
 * CaseDescription: Verify PrintTimeOutLog
 */
HWTEST_F(AbilityConnectManagerTest, PrintTimeOutLog_006, TestSize.Level1)
{
    std::shared_ptr<AbilityConnectManager> connectManager = std::make_shared<AbilityConnectManager>(3);
    ASSERT_NE(connectManager, nullptr);
    std::shared_ptr<BaseExtensionRecord> abilityRecord = InitAbilityRecord();
    uint32_t msgId = 5;
    connectManager->PrintTimeOutLog(abilityRecord, msgId);
    connectManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: PrintTimeOutLog
 * SubFunction: NA
 * FunctionPoints: MissionListManager PrintTimeOutLog
 * EnvConditions: NA
 * CaseDescription: Verify PrintTimeOutLog
 */
HWTEST_F(AbilityConnectManagerTest, PrintTimeOutLog_007, TestSize.Level1)
{
    std::shared_ptr<AbilityConnectManager> connectManager = std::make_shared<AbilityConnectManager>(3);
    ASSERT_NE(connectManager, nullptr);
    std::shared_ptr<BaseExtensionRecord> abilityRecord = InitAbilityRecord();
    uint32_t msgId = 6;
    connectManager->PrintTimeOutLog(abilityRecord, msgId);
    connectManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: PrintTimeOutLog
 * SubFunction: NA
 * FunctionPoints: MissionListManager PrintTimeOutLog
 * EnvConditions: NA
 * CaseDescription: Verify PrintTimeOutLog
 */
HWTEST_F(AbilityConnectManagerTest, PrintTimeOutLog_008, TestSize.Level1)
{
    std::shared_ptr<AbilityConnectManager> connectManager = std::make_shared<AbilityConnectManager>(3);
    ASSERT_NE(connectManager, nullptr);
    std::shared_ptr<BaseExtensionRecord> abilityRecord = InitAbilityRecord();
    uint32_t msgId = 3;
    connectManager->PrintTimeOutLog(abilityRecord, msgId);
    connectManager.reset();
}

/*
 * Feature: AbilityConnectManager
 * Function: ScheduleCommandAbilityWindowDone
 * SubFunction: NA
 * FunctionPoints: AbilityConnectManager ScheduleCommandAbilityWindowDone
 * EnvConditions: NA
 * CaseDescription: Verify ScheduleCommandAbilityWindowDone
 * @tc.require: AR000I8B26
 */
HWTEST_F(AbilityConnectManagerTest, ScheduleCommandAbilityWindowDone_001, TestSize.Level1)
{
    GetCommonExtensionManager()->SetTaskHandler(TaskHandler());
    GetCommonExtensionManager()->SetEventHandler(EventHandler());
    auto sessionInfo = MockSessionInfo(0);

    sptr<IRemoteObject> nullToken = nullptr;
    auto result = GetCommonExtensionManager()->ScheduleCommandAbilityWindowDone(
        nullToken, sessionInfo, WIN_CMD_FOREGROUND, ABILITY_CMD_FOREGROUND);
    EXPECT_EQ(result, OHOS::ERR_INVALID_VALUE);

    std::shared_ptr<BaseExtensionRecord> ability = nullptr;
    OHOS::sptr<OHOS::IRemoteObject> token1 = new OHOS::AAFwk::Token(ability);
    auto result1 = GetCommonExtensionManager()->ScheduleCommandAbilityWindowDone(
        token1, sessionInfo, WIN_CMD_FOREGROUND, ABILITY_CMD_FOREGROUND);
    EXPECT_EQ(result1, OHOS::ERR_INVALID_VALUE);

    sptr<SessionInfo> nullSession = nullptr;
    auto result2 = GetCommonExtensionManager()->ScheduleCommandAbilityWindowDone(
        serviceToken_, nullSession, WIN_CMD_FOREGROUND, ABILITY_CMD_FOREGROUND);
    EXPECT_EQ(result2, OHOS::ERR_INVALID_VALUE);

    auto result3 = GetCommonExtensionManager()->ScheduleCommandAbilityWindowDone(
        serviceToken_, sessionInfo, WIN_CMD_FOREGROUND, ABILITY_CMD_FOREGROUND);
    EXPECT_EQ(result3, OHOS::ERR_OK);
}

/*
 * Feature: AbilityConnectManager
 * Function: ForegroundUIExtensionAbility
 * SubFunction: NA
 * FunctionPoints: MissionListManager ForegroundUIExtensionAbility
 * EnvConditions: NA
 * CaseDescription: Verify ForegroundUIExtensionAbility
 * @tc.require: AR000I8B26
 */
HWTEST_F(AbilityConnectManagerTest, MoveToForeground_001, TestSize.Level1)
{
    serviceRecord_->ForegroundUIExtensionAbility();
    EXPECT_EQ(serviceRecord_->GetAbilityState(), AbilityState::FOREGROUNDING);
    serviceRecord_->SetAbilityState(AbilityState::INITIAL);
}

/*
 * Feature: AbilityConnectManager
 * Function: DispatchForeground
 * SubFunction:
 * FunctionPoints: DispatchForeground
 * EnvConditions:NA
 * CaseDescription: Verify the DispatchForeground process
 * @tc.require: AR000I8B26
 */
HWTEST_F(AbilityConnectManagerTest, DispatchForeground_001, TestSize.Level1)
{
    std::shared_ptr<AbilityConnectManager> connectManager = std::make_shared<AbilityConnectManager>(3);
    ASSERT_NE(connectManager, nullptr);
    std::shared_ptr<BaseExtensionRecord> ability = nullptr;
    auto result = connectManager->DispatchForeground(ability, nullptr);
    EXPECT_EQ(result, OHOS::ERR_INVALID_VALUE);

    result = connectManager->DispatchForeground(serviceRecord_, serviceRecord_->GetToken());
    EXPECT_EQ(result, OHOS::ERR_INVALID_VALUE);

    connectManager->SetTaskHandler(TaskHandler());
    connectManager->SetEventHandler(EventHandler());
    result = connectManager->DispatchForeground(serviceRecord_, serviceRecord_->GetToken());
    EXPECT_EQ(result, OHOS::ERR_OK);
}

/*
 * Feature: AbilityConnectManager
 * Function: DispatchBackground
 * SubFunction:
 * FunctionPoints: DispatchBackground
 * EnvConditions:NA
 * CaseDescription: Verify the DispatchBackground process
 * @tc.require: AR000I8B26
 */
HWTEST_F(AbilityConnectManagerTest, DispatchBackground_001, TestSize.Level1)
{
    std::shared_ptr<AbilityConnectManager> connectManager = std::make_shared<AbilityConnectManager>(3);
    ASSERT_NE(connectManager, nullptr);
    std::shared_ptr<BaseExtensionRecord> ability = nullptr;
    auto result = connectManager->DispatchBackground(ability, nullptr);
    EXPECT_EQ(result, OHOS::ERR_INVALID_VALUE);

    result = connectManager->DispatchBackground(serviceRecord_, serviceRecord_->GetToken());
    EXPECT_EQ(result, OHOS::ERR_INVALID_VALUE);

    connectManager->SetTaskHandler(TaskHandler());
    connectManager->SetEventHandler(EventHandler());
    result = connectManager->DispatchBackground(serviceRecord_, serviceRecord_->GetToken());
    EXPECT_EQ(result, OHOS::ERR_OK);
}

/*
 * Feature: AbilityConnectManager
 * Function: HandleCommandWindowTimeoutTask
 * SubFunction: HandleCommandWindowTimeoutTask
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityConnectManager HandleCommandWindowTimeoutTask
 * @tc.require: AR000I8B26
 */
HWTEST_F(AbilityConnectManagerTest, HandleCommandWindowTimeoutTask_001, TestSize.Level1)
{
    std::shared_ptr<AbilityConnectManager> connectManager = std::make_shared<AbilityConnectManager>(0);
    ASSERT_NE(connectManager, nullptr);
    connectManager->HandleCommandWindowTimeoutTask(serviceRecord_, MockSessionInfo(0), WIN_CMD_FOREGROUND);
}

/*
 * Feature: AbilityConnectManager
 * Function: CommandAbilityWindow
 * SubFunction: CommandAbilityWindow
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityConnectManager CommandAbilityWindow
 * @tc.require: AR000I8B26
 */
HWTEST_F(AbilityConnectManagerTest, CommandAbilityWindow_001, TestSize.Level1)
{
    std::shared_ptr<AbilityConnectManager> connectManager = std::make_shared<AbilityConnectManager>(0);
    ASSERT_NE(connectManager, nullptr);
    connectManager->SetTaskHandler(TaskHandler());
    connectManager->SetEventHandler(EventHandler());
    connectManager->CommandAbilityWindow(serviceRecord_, MockSessionInfo(0), WIN_CMD_FOREGROUND);
}

/*
 * Feature: AbilityConnectManager
 * Function: CompleteForeground
 * SubFunction: CompleteForeground
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityConnectManager CompleteForeground
 * @tc.require: AR000I8B26
 */
HWTEST_F(AbilityConnectManagerTest, CompleteForeground_001, TestSize.Level1)
{
    std::shared_ptr<AbilityConnectManager> connectManager = std::make_shared<AbilityConnectManager>(3);
    ASSERT_NE(connectManager, nullptr);
    std::shared_ptr<BaseExtensionRecord> abilityRecord = InitAbilityRecord();
    abilityRecord->currentState_ = AbilityState::BACKGROUND;
    connectManager->CompleteForeground(abilityRecord);
    EXPECT_EQ(abilityRecord->GetAbilityState(), AbilityState::BACKGROUND);

    abilityRecord->currentState_ = AbilityState::FOREGROUNDING;
    connectManager->CompleteForeground(abilityRecord);
    EXPECT_EQ(abilityRecord->GetAbilityState(), AbilityState::FOREGROUND);
    connectManager.reset();
}

/**
 * @tc.name: AbilityWindowConfigTransactionDone_0100
 * @tc.desc: AbilityWindowConfigTransactionDone
 * @tc.type: FUNC
 */
HWTEST_F(AbilityConnectManagerTest, AbilityWindowConfigTransactionDone_0100, TestSize.Level1)
{
    std::shared_ptr<AbilityConnectManager> connectManager = std::make_shared<AbilityConnectManager>(0);
    ASSERT_NE(connectManager, nullptr);

    WindowConfig windowConfig;
    auto ret = connectManager->AbilityWindowConfigTransactionDone(serviceToken_, windowConfig);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: UpdateKeepAliveEnableState_0100
 * @tc.desc: UpdateKeepAliveEnableState
 * @tc.type: FUNC
 */
HWTEST_F(AbilityConnectManagerTest, UpdateKeepAliveEnableState_0100, TestSize.Level1)
{
    std::shared_ptr<AbilityConnectManager> connectManager = std::make_shared<AbilityConnectManager>(0);
    ASSERT_NE(connectManager, nullptr);

    auto ret = connectManager->UpdateKeepAliveEnableState("bundle", "entry", "mainAbility", true);
    EXPECT_EQ(ret, ERR_OK);
}

/*
 * Feature: AbilityConnectManager
 * Function: ScheduleConnectAbilityDoneLocked
 * SubFunction: ScheduleConnectAbilityDoneLocked
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityConnectManager ScheduleConnectAbilityDoneLocked
 */
HWTEST_F(AbilityConnectManagerTest, AAFwk_AbilityMS_ScheduleConnectAbilityDoneLocked_002, TestSize.Level1)
{
    std::shared_ptr<AbilityConnectManager> connectManager = std::make_shared<AbilityConnectManager>(0);
    std::shared_ptr<BaseExtensionRecord> abilityRecord = serviceRecord_;
    sptr<IRemoteObject> token = abilityRecord->GetToken();
    sptr<IRemoteObject> remoteObject = nullptr;
    abilityRecord->abilityInfo_.type = AbilityType::EXTENSION;
    abilityRecord->SetAbilityState(AbilityState::INACTIVE);
    connectManager->serviceMap_.emplace("first", abilityRecord);
    int res = connectManager->ScheduleConnectAbilityDoneLocked(token, remoteObject);
    EXPECT_EQ(res, ERR_OK);
}

/*
 * Feature: AbilityConnectManager
 * Function: ScheduleConnectAbilityDoneLocked
 * SubFunction: ScheduleConnectAbilityDoneLocked
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityConnectManager ScheduleConnectAbilityDoneLocked
 */
HWTEST_F(AbilityConnectManagerTest, AAFwk_AbilityMS_ScheduleConnectAbilityDoneLocked_003, TestSize.Level1)
{
    std::shared_ptr<AbilityConnectManager> connectManager = std::make_shared<AbilityConnectManager>(0);
    std::shared_ptr<BaseExtensionRecord> abilityRecord = serviceRecord_;
    sptr<IRemoteObject> token = abilityRecord->GetToken();
    sptr<IRemoteObject> remoteObject = nullptr;
    abilityRecord->abilityInfo_.type = AbilityType::SERVICE;
    abilityRecord->SetAbilityState(AbilityState::INACTIVE);
    connectManager->serviceMap_.emplace("first", abilityRecord);
    int res = connectManager->ScheduleConnectAbilityDoneLocked(token, remoteObject);
    EXPECT_EQ(res, ERR_OK);
}

/*
 * Feature: AbilityConnectManager
 * Function: ScheduleConnectAbilityDoneLocked
 * SubFunction: ScheduleConnectAbilityDoneLocked
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityConnectManager ScheduleConnectAbilityDoneLocked
 */
HWTEST_F(AbilityConnectManagerTest, AAFwk_AbilityMS_ScheduleConnectAbilityDoneLocked_004, TestSize.Level1)
{
    std::shared_ptr<AbilityConnectManager> connectManager = std::make_shared<AbilityConnectManager>(0);
    std::shared_ptr<BaseExtensionRecord> abilityRecord = serviceRecord_;
    sptr<IRemoteObject> token = abilityRecord->GetToken();
    sptr<IRemoteObject> remoteObject = nullptr;
    abilityRecord->abilityInfo_.type = AbilityType::DATA;
    abilityRecord->SetAbilityState(AbilityState::INACTIVE);
    connectManager->serviceMap_.emplace("first", abilityRecord);
    int res = connectManager->ScheduleConnectAbilityDoneLocked(token, remoteObject);
    EXPECT_EQ(res, ERR_OK);
}

/*
 * Feature: AbilityConnectManager
 * Function: ScheduleConnectAbilityDoneLocked
 * SubFunction: ScheduleConnectAbilityDoneLocked
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityConnectManager ScheduleConnectAbilityDoneLocked
 */
HWTEST_F(AbilityConnectManagerTest, AAFwk_AbilityMS_ScheduleConnectAbilityDoneLocked_005, TestSize.Level1)
{
    std::shared_ptr<AbilityConnectManager> connectManager = std::make_shared<AbilityConnectManager>(0);
    std::shared_ptr<BaseExtensionRecord> abilityRecord = serviceRecord_;
    sptr<IRemoteObject> token = abilityRecord->GetToken();
    sptr<IRemoteObject> remoteObject = nullptr;
    abilityRecord->abilityInfo_.type = AbilityType::FORM;
    abilityRecord->SetAbilityState(AbilityState::INACTIVE);
    connectManager->serviceMap_.emplace("first", abilityRecord);
    int res = connectManager->ScheduleConnectAbilityDoneLocked(token, remoteObject);
    EXPECT_EQ(res, ERR_OK);
}

/*
 * Feature: AbilityConnectManager
 * Function: ScheduleConnectAbilityDoneLocked
 * SubFunction: ScheduleConnectAbilityDoneLocked
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityConnectManager ScheduleConnectAbilityDoneLocked
 */
HWTEST_F(AbilityConnectManagerTest, AAFwk_AbilityMS_ScheduleConnectAbilityDoneLocked_006, TestSize.Level1)
{
    std::shared_ptr<AbilityConnectManager> connectManager = std::make_shared<AbilityConnectManager>(0);
    std::shared_ptr<BaseExtensionRecord> abilityRecord = serviceRecord_;
    sptr<IRemoteObject> token = abilityRecord->GetToken();
    sptr<IRemoteObject> remoteObject = nullptr;
    abilityRecord->abilityInfo_.type = AbilityType::UNKNOWN;
    abilityRecord->SetAbilityState(AbilityState::INACTIVE);
    connectManager->serviceMap_.emplace("first", abilityRecord);
    int res = connectManager->ScheduleConnectAbilityDoneLocked(token, remoteObject);
    EXPECT_EQ(res, ERR_OK);
}

/*
 * Feature: AbilityConnectManager
 * Function: ScheduleConnectAbilityDoneLocked
 * SubFunction: ScheduleConnectAbilityDoneLocked
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityConnectManager ScheduleConnectAbilityDoneLocked
 */
HWTEST_F(AbilityConnectManagerTest, AAFwk_AbilityMS_ScheduleConnectAbilityDoneLocked_007, TestSize.Level1)
{
    std::shared_ptr<AbilityConnectManager> connectManager = std::make_shared<AbilityConnectManager>(0);
    std::shared_ptr<BaseExtensionRecord> abilityRecord = serviceRecord_;
    sptr<IRemoteObject> token = abilityRecord->GetToken();
    sptr<IRemoteObject> remoteObject = nullptr;
    abilityRecord->abilityInfo_.type = AbilityType::PAGE;
    abilityRecord->SetAbilityState(AbilityState::INACTIVE);
    connectManager->serviceMap_.emplace("first", abilityRecord);
    int res = connectManager->ScheduleConnectAbilityDoneLocked(nullptr, remoteObject);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityConnectManager
 * Function: ScheduleConnectAbilityDoneLocked
 * SubFunction: ScheduleConnectAbilityDoneLocked
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityConnectManager ScheduleConnectAbilityDoneLocked
 */
HWTEST_F(AbilityConnectManagerTest, AAFwk_AbilityMS_ScheduleConnectAbilityDoneLocked_008, TestSize.Level1)
{
    std::shared_ptr<AbilityConnectManager> connectManager = std::make_shared<AbilityConnectManager>(0);
    std::shared_ptr<BaseExtensionRecord> abilityRecord = serviceRecord_;
    sptr<IRemoteObject> token = abilityRecord->GetToken();
    sptr<IRemoteObject> remoteObject = nullptr;
    abilityRecord->abilityInfo_.type = AbilityType::EXTENSION;
    abilityRecord->SetAbilityState(AbilityState::INACTIVE);
    connectManager->serviceMap_.emplace("first", abilityRecord);
    int res = connectManager->ScheduleConnectAbilityDoneLocked(nullptr, remoteObject);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityConnectManager
 * Function: ScheduleConnectAbilityDoneLocked
 * SubFunction: ScheduleConnectAbilityDoneLocked
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityConnectManager ScheduleConnectAbilityDoneLocked
 */
HWTEST_F(AbilityConnectManagerTest, AAFwk_AbilityMS_ScheduleConnectAbilityDoneLocked_009, TestSize.Level1)
{
    std::shared_ptr<AbilityConnectManager> connectManager = std::make_shared<AbilityConnectManager>(0);
    std::shared_ptr<BaseExtensionRecord> abilityRecord = serviceRecord_;
    sptr<IRemoteObject> token = abilityRecord->GetToken();
    sptr<IRemoteObject> remoteObject = nullptr;
    abilityRecord->abilityInfo_.type = AbilityType::SERVICE;
    abilityRecord->SetAbilityState(AbilityState::INACTIVE);
    connectManager->serviceMap_.emplace("first", abilityRecord);
    int res = connectManager->ScheduleConnectAbilityDoneLocked(nullptr, remoteObject);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityConnectManager
 * Function: ScheduleConnectAbilityDoneLocked
 * SubFunction: ScheduleConnectAbilityDoneLocked
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityConnectManager ScheduleConnectAbilityDoneLocked
 */
HWTEST_F(AbilityConnectManagerTest, AAFwk_AbilityMS_ScheduleConnectAbilityDoneLocked_010, TestSize.Level1)
{
    std::shared_ptr<AbilityConnectManager> connectManager = std::make_shared<AbilityConnectManager>(0);
    std::shared_ptr<BaseExtensionRecord> abilityRecord = serviceRecord_;
    sptr<IRemoteObject> token = abilityRecord->GetToken();
    sptr<IRemoteObject> remoteObject = nullptr;
    abilityRecord->abilityInfo_.type = AbilityType::DATA;
    abilityRecord->SetAbilityState(AbilityState::INACTIVE);
    connectManager->serviceMap_.emplace("first", abilityRecord);
    int res = connectManager->ScheduleConnectAbilityDoneLocked(nullptr, remoteObject);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityConnectManager
 * Function: ScheduleConnectAbilityDoneLocked
 * SubFunction: ScheduleConnectAbilityDoneLocked
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityConnectManager ScheduleConnectAbilityDoneLocked
 */
HWTEST_F(AbilityConnectManagerTest, AAFwk_AbilityMS_ScheduleConnectAbilityDoneLocked_011, TestSize.Level1)
{
    std::shared_ptr<AbilityConnectManager> connectManager = std::make_shared<AbilityConnectManager>(0);
    std::shared_ptr<BaseExtensionRecord> abilityRecord = serviceRecord_;
    sptr<IRemoteObject> token = abilityRecord->GetToken();
    sptr<IRemoteObject> remoteObject = nullptr;
    abilityRecord->abilityInfo_.type = AbilityType::FORM;
    abilityRecord->SetAbilityState(AbilityState::INACTIVE);
    connectManager->serviceMap_.emplace("first", abilityRecord);
    int res = connectManager->ScheduleConnectAbilityDoneLocked(nullptr, remoteObject);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityConnectManager
 * Function: ScheduleConnectAbilityDoneLocked
 * SubFunction: ScheduleConnectAbilityDoneLocked
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityConnectManager ScheduleConnectAbilityDoneLocked
 */
HWTEST_F(AbilityConnectManagerTest, AAFwk_AbilityMS_ScheduleConnectAbilityDoneLocked_012, TestSize.Level1)
{
    std::shared_ptr<AbilityConnectManager> connectManager = std::make_shared<AbilityConnectManager>(0);
    std::shared_ptr<BaseExtensionRecord> abilityRecord = serviceRecord_;
    sptr<IRemoteObject> token = abilityRecord->GetToken();
    sptr<IRemoteObject> remoteObject = nullptr;
    abilityRecord->abilityInfo_.type = AbilityType::UNKNOWN;
    abilityRecord->SetAbilityState(AbilityState::INACTIVE);
    connectManager->serviceMap_.emplace("first", abilityRecord);
    int res = connectManager->ScheduleConnectAbilityDoneLocked(nullptr, remoteObject);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityConnectManager
 * Function: ScheduleDisconnectAbilityDoneLocked
 * SubFunction: ScheduleDisconnectAbilityDoneLocked
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityConnectManager ScheduleDisconnectAbilityDoneLocked
 */
HWTEST_F(AbilityConnectManagerTest, AAFwk_AbilityMS_ScheduleDisconnectAbilityDoneLocked_002, TestSize.Level1)
{
    std::shared_ptr<AbilityConnectManager> connectManager = std::make_shared<AbilityConnectManager>(0);
    std::shared_ptr<BaseExtensionRecord> abilityRecord = serviceRecord_;
    OHOS::sptr<IAbilityConnection> callback = new AbilityConnectCallback();
    std::shared_ptr<ConnectionRecord> connection =
        std::make_shared<ConnectionRecord>(abilityRecord->GetToken(), abilityRecord, callback, nullptr);
    sptr<IRemoteObject> token = abilityRecord->GetToken();
    connection->SetConnectState(ConnectionState::DISCONNECTING);
    abilityRecord->abilityInfo_.type = AbilityType::EXTENSION;
    abilityRecord->SetAbilityState(AbilityState::INACTIVE);
    abilityRecord->connRecordList_.push_back(connection);
    connectManager->serviceMap_.emplace("first", abilityRecord);
    int res = connectManager->ScheduleDisconnectAbilityDoneLocked(token);
    EXPECT_EQ(res, INVALID_CONNECTION_STATE);
    abilityRecord->AddStartId();
    abilityRecord->SetAbilityState(AbilityState::ACTIVE);
    connectManager->serviceMap_.emplace("first", abilityRecord);
    res = connectManager->ScheduleDisconnectAbilityDoneLocked(token);
    EXPECT_EQ(res, ERR_OK);
}

/*
 * Feature: AbilityConnectManager
 * Function: ScheduleDisconnectAbilityDoneLocked
 * SubFunction: ScheduleDisconnectAbilityDoneLocked
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityConnectManager ScheduleDisconnectAbilityDoneLocked
 */
HWTEST_F(AbilityConnectManagerTest, AAFwk_AbilityMS_ScheduleDisconnectAbilityDoneLocked_003, TestSize.Level1)
{
    std::shared_ptr<AbilityConnectManager> connectManager = std::make_shared<AbilityConnectManager>(0);
    std::shared_ptr<BaseExtensionRecord> abilityRecord = serviceRecord_;
    OHOS::sptr<IAbilityConnection> callback = new AbilityConnectCallback();
    std::shared_ptr<ConnectionRecord> connection =
        std::make_shared<ConnectionRecord>(abilityRecord->GetToken(), abilityRecord, callback, nullptr);
    sptr<IRemoteObject> token = abilityRecord->GetToken();
    connection->SetConnectState(ConnectionState::DISCONNECTING);
    abilityRecord->abilityInfo_.type = AbilityType::SERVICE;
    abilityRecord->SetAbilityState(AbilityState::INACTIVE);
    abilityRecord->connRecordList_.push_back(connection);
    connectManager->serviceMap_.emplace("first", abilityRecord);
    int res = connectManager->ScheduleDisconnectAbilityDoneLocked(token);
    EXPECT_EQ(res, INVALID_CONNECTION_STATE);
    abilityRecord->AddStartId();
    abilityRecord->SetAbilityState(AbilityState::ACTIVE);
    connectManager->serviceMap_.emplace("first", abilityRecord);
    res = connectManager->ScheduleDisconnectAbilityDoneLocked(token);
    EXPECT_EQ(res, ERR_OK);
}

/*
 * Feature: AbilityConnectManager
 * Function: ScheduleDisconnectAbilityDoneLocked
 * SubFunction: ScheduleDisconnectAbilityDoneLocked
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityConnectManager ScheduleDisconnectAbilityDoneLocked
 */
HWTEST_F(AbilityConnectManagerTest, AAFwk_AbilityMS_ScheduleDisconnectAbilityDoneLocked_004, TestSize.Level1)
{
    std::shared_ptr<AbilityConnectManager> connectManager = std::make_shared<AbilityConnectManager>(0);
    std::shared_ptr<BaseExtensionRecord> abilityRecord = serviceRecord_;
    OHOS::sptr<IAbilityConnection> callback = new AbilityConnectCallback();
    std::shared_ptr<ConnectionRecord> connection =
        std::make_shared<ConnectionRecord>(abilityRecord->GetToken(), abilityRecord, callback, nullptr);
    sptr<IRemoteObject> token = abilityRecord->GetToken();
    connection->SetConnectState(ConnectionState::DISCONNECTING);
    abilityRecord->abilityInfo_.type = AbilityType::DATA;
    abilityRecord->SetAbilityState(AbilityState::INACTIVE);
    abilityRecord->connRecordList_.push_back(connection);
    connectManager->serviceMap_.emplace("first", abilityRecord);
    int res = connectManager->ScheduleDisconnectAbilityDoneLocked(token);
    EXPECT_EQ(res, INVALID_CONNECTION_STATE);
    abilityRecord->AddStartId();
    abilityRecord->SetAbilityState(AbilityState::ACTIVE);
    connectManager->serviceMap_.emplace("first", abilityRecord);
    res = connectManager->ScheduleDisconnectAbilityDoneLocked(token);
    EXPECT_EQ(res, ERR_OK);
}

/*
 * Feature: AbilityConnectManager
 * Function: ScheduleDisconnectAbilityDoneLocked
 * SubFunction: ScheduleDisconnectAbilityDoneLocked
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityConnectManager ScheduleDisconnectAbilityDoneLocked
 */
HWTEST_F(AbilityConnectManagerTest, AAFwk_AbilityMS_ScheduleDisconnectAbilityDoneLocked_005, TestSize.Level1)
{
    std::shared_ptr<AbilityConnectManager> connectManager = std::make_shared<AbilityConnectManager>(0);
    std::shared_ptr<BaseExtensionRecord> abilityRecord = serviceRecord_;
    OHOS::sptr<IAbilityConnection> callback = new AbilityConnectCallback();
    std::shared_ptr<ConnectionRecord> connection =
        std::make_shared<ConnectionRecord>(abilityRecord->GetToken(), abilityRecord, callback, nullptr);
    sptr<IRemoteObject> token = abilityRecord->GetToken();
    connection->SetConnectState(ConnectionState::DISCONNECTING);
    abilityRecord->abilityInfo_.type = AbilityType::FORM;
    abilityRecord->SetAbilityState(AbilityState::INACTIVE);
    abilityRecord->connRecordList_.push_back(connection);
    connectManager->serviceMap_.emplace("first", abilityRecord);
    int res = connectManager->ScheduleDisconnectAbilityDoneLocked(token);
    EXPECT_EQ(res, INVALID_CONNECTION_STATE);
    abilityRecord->AddStartId();
    abilityRecord->SetAbilityState(AbilityState::ACTIVE);
    connectManager->serviceMap_.emplace("first", abilityRecord);
    res = connectManager->ScheduleDisconnectAbilityDoneLocked(token);
    EXPECT_EQ(res, ERR_OK);
}

/*
 * Feature: AbilityConnectManager
 * Function: ScheduleDisconnectAbilityDoneLocked
 * SubFunction: ScheduleDisconnectAbilityDoneLocked
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityConnectManager ScheduleDisconnectAbilityDoneLocked
 */
HWTEST_F(AbilityConnectManagerTest, AAFwk_AbilityMS_ScheduleDisconnectAbilityDoneLocked_006, TestSize.Level1)
{
    std::shared_ptr<AbilityConnectManager> connectManager = std::make_shared<AbilityConnectManager>(0);
    std::shared_ptr<BaseExtensionRecord> abilityRecord = serviceRecord_;
    OHOS::sptr<IAbilityConnection> callback = new AbilityConnectCallback();
    std::shared_ptr<ConnectionRecord> connection =
        std::make_shared<ConnectionRecord>(abilityRecord->GetToken(), abilityRecord, callback, nullptr);
    sptr<IRemoteObject> token = abilityRecord->GetToken();
    connection->SetConnectState(ConnectionState::DISCONNECTING);
    abilityRecord->abilityInfo_.type = AbilityType::UNKNOWN;
    abilityRecord->SetAbilityState(AbilityState::INACTIVE);
    abilityRecord->connRecordList_.push_back(connection);
    connectManager->serviceMap_.emplace("first", abilityRecord);
    int res = connectManager->ScheduleDisconnectAbilityDoneLocked(token);
    EXPECT_EQ(res, INVALID_CONNECTION_STATE);
    abilityRecord->AddStartId();
    abilityRecord->SetAbilityState(AbilityState::ACTIVE);
    connectManager->serviceMap_.emplace("first", abilityRecord);
    res = connectManager->ScheduleDisconnectAbilityDoneLocked(token);
    EXPECT_EQ(res, ERR_OK);
}

/*
 * Feature: AbilityConnectManager
 * Function: ScheduleDisconnectAbilityDoneLocked
 * SubFunction: ScheduleDisconnectAbilityDoneLocked
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityConnectManager ScheduleDisconnectAbilityDoneLocked
 */
HWTEST_F(AbilityConnectManagerTest, AAFwk_AbilityMS_ScheduleDisconnectAbilityDoneLocked_007, TestSize.Level1)
{
    std::shared_ptr<AbilityConnectManager> connectManager = std::make_shared<AbilityConnectManager>(0);
    std::shared_ptr<BaseExtensionRecord> abilityRecord = serviceRecord_;
    OHOS::sptr<IAbilityConnection> callback = new AbilityConnectCallback();
    std::shared_ptr<ConnectionRecord> connection =
        std::make_shared<ConnectionRecord>(abilityRecord->GetToken(), abilityRecord, callback, nullptr);
    sptr<IRemoteObject> token = abilityRecord->GetToken();
    connection->SetConnectState(ConnectionState::DISCONNECTING);
    abilityRecord->abilityInfo_.type = AbilityType::PAGE;
    abilityRecord->SetAbilityState(AbilityState::INACTIVE);
    abilityRecord->connRecordList_.push_back(connection);
    connectManager->serviceMap_.emplace("first", abilityRecord);
    int res = connectManager->ScheduleDisconnectAbilityDoneLocked(token);
    EXPECT_EQ(res, INVALID_CONNECTION_STATE);
    abilityRecord->AddStartId();
    abilityRecord->SetAbilityState(AbilityState::ACTIVE);
    connectManager->serviceMap_.emplace("first", abilityRecord);
    res = connectManager->ScheduleDisconnectAbilityDoneLocked(nullptr);
    EXPECT_EQ(res, CONNECTION_NOT_EXIST);
}

/*
 * Feature: AbilityConnectManager
 * Function: ScheduleDisconnectAbilityDoneLocked
 * SubFunction: ScheduleDisconnectAbilityDoneLocked
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityConnectManager ScheduleDisconnectAbilityDoneLocked
 */
HWTEST_F(AbilityConnectManagerTest, AAFwk_AbilityMS_ScheduleDisconnectAbilityDoneLocked_008, TestSize.Level1)
{
    std::shared_ptr<AbilityConnectManager> connectManager = std::make_shared<AbilityConnectManager>(0);
    std::shared_ptr<BaseExtensionRecord> abilityRecord = serviceRecord_;
    OHOS::sptr<IAbilityConnection> callback = new AbilityConnectCallback();
    std::shared_ptr<ConnectionRecord> connection =
        std::make_shared<ConnectionRecord>(abilityRecord->GetToken(), abilityRecord, callback, nullptr);
    sptr<IRemoteObject> token = abilityRecord->GetToken();
    connection->SetConnectState(ConnectionState::DISCONNECTING);
    abilityRecord->abilityInfo_.type = AbilityType::EXTENSION;
    abilityRecord->SetAbilityState(AbilityState::INACTIVE);
    abilityRecord->connRecordList_.push_back(connection);
    connectManager->serviceMap_.emplace("first", abilityRecord);
    int res = connectManager->ScheduleDisconnectAbilityDoneLocked(token);
    EXPECT_EQ(res, INVALID_CONNECTION_STATE);
    abilityRecord->AddStartId();
    abilityRecord->SetAbilityState(AbilityState::ACTIVE);
    connectManager->serviceMap_.emplace("first", abilityRecord);
    res = connectManager->ScheduleDisconnectAbilityDoneLocked(nullptr);
    EXPECT_EQ(res, CONNECTION_NOT_EXIST);
}

/*
 * Feature: AbilityConnectManager
 * Function: ScheduleDisconnectAbilityDoneLocked
 * SubFunction: ScheduleDisconnectAbilityDoneLocked
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityConnectManager ScheduleDisconnectAbilityDoneLocked
 */
HWTEST_F(AbilityConnectManagerTest, AAFwk_AbilityMS_ScheduleDisconnectAbilityDoneLocked_009, TestSize.Level1)
{
    std::shared_ptr<AbilityConnectManager> connectManager = std::make_shared<AbilityConnectManager>(0);
    std::shared_ptr<BaseExtensionRecord> abilityRecord = serviceRecord_;
    OHOS::sptr<IAbilityConnection> callback = new AbilityConnectCallback();
    std::shared_ptr<ConnectionRecord> connection =
        std::make_shared<ConnectionRecord>(abilityRecord->GetToken(), abilityRecord, callback, nullptr);
    sptr<IRemoteObject> token = abilityRecord->GetToken();
    connection->SetConnectState(ConnectionState::DISCONNECTING);
    abilityRecord->abilityInfo_.type = AbilityType::SERVICE;
    abilityRecord->SetAbilityState(AbilityState::INACTIVE);
    abilityRecord->connRecordList_.push_back(connection);
    connectManager->serviceMap_.emplace("first", abilityRecord);
    int res = connectManager->ScheduleDisconnectAbilityDoneLocked(token);
    EXPECT_EQ(res, INVALID_CONNECTION_STATE);
    abilityRecord->AddStartId();
    abilityRecord->SetAbilityState(AbilityState::ACTIVE);
    connectManager->serviceMap_.emplace("first", abilityRecord);
    res = connectManager->ScheduleDisconnectAbilityDoneLocked(nullptr);
    EXPECT_EQ(res, CONNECTION_NOT_EXIST);
}

/*
 * Feature: AbilityConnectManager
 * Function: ScheduleDisconnectAbilityDoneLocked
 * SubFunction: ScheduleDisconnectAbilityDoneLocked
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityConnectManager ScheduleDisconnectAbilityDoneLocked
 */
HWTEST_F(AbilityConnectManagerTest, AAFwk_AbilityMS_ScheduleDisconnectAbilityDoneLocked_010, TestSize.Level1)
{
    std::shared_ptr<AbilityConnectManager> connectManager = std::make_shared<AbilityConnectManager>(0);
    std::shared_ptr<BaseExtensionRecord> abilityRecord = serviceRecord_;
    OHOS::sptr<IAbilityConnection> callback = new AbilityConnectCallback();
    std::shared_ptr<ConnectionRecord> connection =
        std::make_shared<ConnectionRecord>(abilityRecord->GetToken(), abilityRecord, callback, nullptr);
    sptr<IRemoteObject> token = abilityRecord->GetToken();
    connection->SetConnectState(ConnectionState::DISCONNECTING);
    abilityRecord->abilityInfo_.type = AbilityType::DATA;
    abilityRecord->SetAbilityState(AbilityState::INACTIVE);
    abilityRecord->connRecordList_.push_back(connection);
    connectManager->serviceMap_.emplace("first", abilityRecord);
    int res = connectManager->ScheduleDisconnectAbilityDoneLocked(token);
    EXPECT_EQ(res, INVALID_CONNECTION_STATE);
    abilityRecord->AddStartId();
    abilityRecord->SetAbilityState(AbilityState::ACTIVE);
    connectManager->serviceMap_.emplace("first", abilityRecord);
    res = connectManager->ScheduleDisconnectAbilityDoneLocked(nullptr);
    EXPECT_EQ(res, CONNECTION_NOT_EXIST);
}

/*
 * Feature: AbilityConnectManager
 * Function: ScheduleDisconnectAbilityDoneLocked
 * SubFunction: ScheduleDisconnectAbilityDoneLocked
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityConnectManager ScheduleDisconnectAbilityDoneLocked
 */
HWTEST_F(AbilityConnectManagerTest, AAFwk_AbilityMS_ScheduleDisconnectAbilityDoneLocked_011, TestSize.Level1)
{
    std::shared_ptr<AbilityConnectManager> connectManager = std::make_shared<AbilityConnectManager>(0);
    std::shared_ptr<BaseExtensionRecord> abilityRecord = serviceRecord_;
    OHOS::sptr<IAbilityConnection> callback = new AbilityConnectCallback();
    std::shared_ptr<ConnectionRecord> connection =
        std::make_shared<ConnectionRecord>(abilityRecord->GetToken(), abilityRecord, callback, nullptr);
    sptr<IRemoteObject> token = abilityRecord->GetToken();
    connection->SetConnectState(ConnectionState::DISCONNECTING);
    abilityRecord->abilityInfo_.type = AbilityType::FORM;
    abilityRecord->SetAbilityState(AbilityState::INACTIVE);
    abilityRecord->connRecordList_.push_back(connection);
    connectManager->serviceMap_.emplace("first", abilityRecord);
    int res = connectManager->ScheduleDisconnectAbilityDoneLocked(token);
    EXPECT_EQ(res, INVALID_CONNECTION_STATE);
    abilityRecord->AddStartId();
    abilityRecord->SetAbilityState(AbilityState::ACTIVE);
    connectManager->serviceMap_.emplace("first", abilityRecord);
    res = connectManager->ScheduleDisconnectAbilityDoneLocked(nullptr);
    EXPECT_EQ(res, CONNECTION_NOT_EXIST);
}

/*
 * Feature: AbilityConnectManager
 * Function: ScheduleDisconnectAbilityDoneLocked
 * SubFunction: ScheduleDisconnectAbilityDoneLocked
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityConnectManager ScheduleDisconnectAbilityDoneLocked
 */
HWTEST_F(AbilityConnectManagerTest, AAFwk_AbilityMS_ScheduleDisconnectAbilityDoneLocked_012, TestSize.Level1)
{
    std::shared_ptr<AbilityConnectManager> connectManager = std::make_shared<AbilityConnectManager>(0);
    std::shared_ptr<BaseExtensionRecord> abilityRecord = serviceRecord_;
    OHOS::sptr<IAbilityConnection> callback = new AbilityConnectCallback();
    std::shared_ptr<ConnectionRecord> connection =
        std::make_shared<ConnectionRecord>(abilityRecord->GetToken(), abilityRecord, callback, nullptr);
    sptr<IRemoteObject> token = abilityRecord->GetToken();
    connection->SetConnectState(ConnectionState::DISCONNECTING);
    abilityRecord->abilityInfo_.type = AbilityType::UNKNOWN;
    abilityRecord->SetAbilityState(AbilityState::INACTIVE);
    abilityRecord->connRecordList_.push_back(connection);
    connectManager->serviceMap_.emplace("first", abilityRecord);
    int res = connectManager->ScheduleDisconnectAbilityDoneLocked(token);
    EXPECT_EQ(res, INVALID_CONNECTION_STATE);
    abilityRecord->AddStartId();
    abilityRecord->SetAbilityState(AbilityState::ACTIVE);
    connectManager->serviceMap_.emplace("first", abilityRecord);
    res = connectManager->ScheduleDisconnectAbilityDoneLocked(nullptr);
    EXPECT_EQ(res, CONNECTION_NOT_EXIST);
}

/*
 * Feature: AbilityConnectManager
 * Function: ScheduleDisconnectAbilityDoneLocked
 * SubFunction: ScheduleDisconnectAbilityDoneLocked
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AgentExtension can finish disconnect while inactive
 */
HWTEST_F(AbilityConnectManagerTest, AAFwk_AbilityMS_ScheduleDisconnectAbilityDoneLocked_013, TestSize.Level1)
{
    std::shared_ptr<AbilityConnectManager> connectManager = std::make_shared<AbilityConnectManager>(0);
    std::shared_ptr<BaseExtensionRecord> abilityRecord = serviceRecord_;
    OHOS::sptr<IAbilityConnection> callback = new AbilityConnectCallback();
    std::shared_ptr<ConnectionRecord> connection =
        std::make_shared<ConnectionRecord>(abilityRecord->GetToken(), abilityRecord, callback, nullptr);
    sptr<IRemoteObject> token = abilityRecord->GetToken();
    connection->SetConnectState(ConnectionState::DISCONNECTING);
    abilityRecord->abilityInfo_.type = AbilityType::EXTENSION;
    abilityRecord->abilityInfo_.extensionAbilityType = ExtensionAbilityType::AGENT;
    abilityRecord->SetAbilityState(AbilityState::INACTIVE);
    abilityRecord->connRecordList_.push_back(connection);
    connectManager->serviceMap_.emplace(AbilityConnectManager::GetServiceKey(abilityRecord), abilityRecord);

    int res = connectManager->ScheduleDisconnectAbilityDoneLocked(token);
    EXPECT_EQ(res, ERR_OK);
    EXPECT_TRUE(abilityRecord->GetConnectRecordList().empty());
}

/**
 * @tc.name: SetExtensionLoadParam_ShouldSetReusePidWhenClientPidValidAndInProcess
 * @tc.desc: Test reusePid is set when clientPid > 0 and isInProcess
 * @tc.type: FUNC
 */
HWTEST_F(AbilityConnectManagerTest,
    SetExtensionLoadParam_ShouldSetReusePidWhenClientPidValidAndInProcess, TestSize.Level1)
{
    auto connectManager = std::make_shared<TestAbilityConnectManager>(0);
    auto record = InitAbilityRecord();
    ASSERT_NE(record, nullptr);
    record->SetClientPid(1234);
    record->SetIsInProcess(true);

    AbilityRuntime::LoadParam loadParam;
    connectManager->SetExtensionLoadParam(loadParam, record);
    EXPECT_EQ(loadParam.reusePid, 1234);
}

/**
 * @tc.name: SetExtensionLoadParam_ShouldNotSetReusePidWhenNotInProcess
 * @tc.desc: Test reusePid is not set when clientPid > 0 but not isInProcess
 * @tc.type: FUNC
 */
HWTEST_F(AbilityConnectManagerTest,
    SetExtensionLoadParam_ShouldNotSetReusePidWhenNotInProcess, TestSize.Level1)
{
    auto connectManager = std::make_shared<TestAbilityConnectManager>(0);
    auto record = InitAbilityRecord();
    ASSERT_NE(record, nullptr);
    record->SetClientPid(1234);
    // isInProcess defaults to false

    AbilityRuntime::LoadParam loadParam;
    connectManager->SetExtensionLoadParam(loadParam, record);
    EXPECT_EQ(loadParam.reusePid, -1);
}

/**
 * @tc.name: SetExtensionLoadParam_ShouldNotSetReusePidWhenClientPidInvalid
 * @tc.desc: Test reusePid is not set when clientPid <= 0
 * @tc.type: FUNC
 */
HWTEST_F(AbilityConnectManagerTest,
    SetExtensionLoadParam_ShouldNotSetReusePidWhenClientPidInvalid, TestSize.Level1)
{
    auto connectManager = std::make_shared<TestAbilityConnectManager>(0);
    auto record = InitAbilityRecord();
    ASSERT_NE(record, nullptr);
    record->SetIsInProcess(true);
    // clientPid defaults to -1

    AbilityRuntime::LoadParam loadParam;
    connectManager->SetExtensionLoadParam(loadParam, record);
    EXPECT_EQ(loadParam.reusePid, -1);
}

/**
 * @tc.name: SetExtensionLoadParam_ShouldNotSetReusePidWhenBothInvalid
 * @tc.desc: Test reusePid is not set when both clientPid <= 0 and not isInProcess
 * @tc.type: FUNC
 */
HWTEST_F(AbilityConnectManagerTest,
    SetExtensionLoadParam_ShouldNotSetReusePidWhenBothInvalid, TestSize.Level1)
{
    auto connectManager = std::make_shared<TestAbilityConnectManager>(0);
    auto record = InitAbilityRecord();
    ASSERT_NE(record, nullptr);
    // both clientPid and isInProcess are default (invalid)

    AbilityRuntime::LoadParam loadParam;
    connectManager->SetExtensionLoadParam(loadParam, record);
    EXPECT_EQ(loadParam.reusePid, -1);
}

/**
 * @tc.name: Agent_DisconnectingRecord_ShouldNotBeDuplicateEligible
 * @tc.desc: Test AGENT reconnect does not treat a disconnecting old record as duplicate
 * @tc.type: FUNC
 */
HWTEST_F(AbilityConnectManagerTest, Agent_DisconnectingRecord_ShouldNotBeDuplicateEligible, TestSize.Level1)
{
    auto connectManager = std::make_shared<TestAbilityConnectManager>(0);
    auto abilityRecord = InitAbilityRecord();
    ASSERT_NE(abilityRecord, nullptr);
    abilityRecord->abilityInfo_.type = AbilityType::EXTENSION;
    abilityRecord->abilityInfo_.extensionAbilityType = ExtensionAbilityType::AGENT;
    auto callback = new AbilityConnectCallback();
    auto disconnectingRecord = ConnectionRecord::CreateConnectionRecord(
        abilityRecord->GetToken(), abilityRecord, callback, connectManager);
    ASSERT_NE(disconnectingRecord, nullptr);
    disconnectingRecord->SetConnectState(ConnectionState::DISCONNECTING);
    AbilityConnectManager::ConnectListType connectRecordList;
    connectRecordList.push_back(disconnectingRecord);

    auto duplicate = connectManager->GetAbilityConnectedRecordFromRecordList(abilityRecord, connectRecordList);

    EXPECT_EQ(duplicate, nullptr);
}

/**
 * @tc.name: Agent_TerminatingRecord_ShouldBeRecreatedForConnect
 * @tc.desc: Test AGENT connect recreates record instead of reusing a terminating stale record
 * @tc.type: FUNC
 */
HWTEST_F(AbilityConnectManagerTest, Agent_TerminatingRecord_ShouldBeRecreatedForConnect, TestSize.Level1)
{
    auto connectManager = std::make_shared<TestAbilityConnectManager>(0);
    auto abilityRequest = GenerateAbilityRequest("device", "AgentExtAbility", "agentapp", "agent.bundle", "entry");
    abilityRequest.abilityInfo.type = AbilityType::EXTENSION;
    abilityRequest.abilityInfo.extensionAbilityType = ExtensionAbilityType::AGENT;
    abilityRequest.want.SetParam(AgentRuntime::AGENTID_KEY, std::string("agentA"));
    abilityRequest.want.SetParam(AgentRuntime::AGENT_CARD_TYPE_KEY,
        static_cast<int32_t>(AgentRuntime::AgentCardType::APP));

    auto staleRecord = BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    ASSERT_NE(staleRecord, nullptr);
    staleRecord->SetAbilityState(AbilityState::TERMINATING);
    auto serviceKey = AbilityConnectManager::GetServiceKey(abilityRequest);
    connectManager->serviceMap_.emplace(serviceKey, staleRecord);

    std::shared_ptr<BaseExtensionRecord> targetService = nullptr;
    bool isLoadedAbility = true;
    connectManager->GetOrCreateServiceRecord(abilityRequest, true, targetService, isLoadedAbility);

    ASSERT_NE(targetService, nullptr);
    EXPECT_NE(targetService, staleRecord);
    EXPECT_FALSE(isLoadedAbility);
    EXPECT_EQ(connectManager->serviceMap_[serviceKey], targetService);
}

/**
 * @tc.name: Agent_GetServiceKey_ShouldUseHostKeyForLowCode
 * @tc.desc: Test standard Agent service key includes AgentId while low-code service key uses host identity only
 * @tc.type: FUNC
 */
HWTEST_F(AbilityConnectManagerTest, Agent_GetServiceKey_ShouldUseHostKeyForLowCode, TestSize.Level1)
{
    auto standardA = GenerateAbilityRequest("device", "AgentExtAbility", "agentapp", "agent.bundle", "entry");
    standardA.abilityInfo.type = AbilityType::EXTENSION;
    standardA.abilityInfo.extensionAbilityType = ExtensionAbilityType::AGENT;
    standardA.want.SetParam(AgentRuntime::AGENTID_KEY, std::string("agentA"));
    standardA.want.SetParam(AgentRuntime::AGENT_CARD_TYPE_KEY, static_cast<int32_t>(AgentRuntime::AgentCardType::APP));
    auto standardB = standardA;
    standardB.want.SetParam(AgentRuntime::AGENTID_KEY, std::string("agentB"));

    auto lowCodeA = standardA;
    lowCodeA.want.SetParam(AgentRuntime::AGENT_CARD_TYPE_KEY,
        static_cast<int32_t>(AgentRuntime::AgentCardType::LOW_CODE));
    auto lowCodeB = lowCodeA;
    lowCodeB.want.SetParam(AgentRuntime::AGENTID_KEY, std::string("agentB"));

    auto standardKeyA = AbilityConnectManager::GetServiceKey(standardA);
    auto standardKeyB = AbilityConnectManager::GetServiceKey(standardB);
    auto lowCodeKeyA = AbilityConnectManager::GetServiceKey(lowCodeA);
    auto lowCodeKeyB = AbilityConnectManager::GetServiceKey(lowCodeB);

    EXPECT_NE(standardKeyA, standardKeyB);
    EXPECT_EQ(lowCodeKeyA, lowCodeKeyB);
    EXPECT_EQ(lowCodeKeyA, lowCodeA.want.GetElement().GetURI());
}

}  // namespace AAFwk
}  // namespace OHOS
