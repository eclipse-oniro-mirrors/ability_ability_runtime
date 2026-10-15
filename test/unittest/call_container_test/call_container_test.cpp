/*
 * Copyright (c) 2022-2026 Huawei Device Co., Ltd.
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
#include "call_container.h"
#include "ability_record.h"
#include "ability_manager_service.h"
#include "mission_list_manager.h"
#undef private
#undef protected
#include "ability_scheduler_mock.h"
#include "mock_ability_connect_callback.h"

using namespace testing::ext;

namespace OHOS {
namespace AAFwk {
class CallContainerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    std::shared_ptr<CallContainer> get() const;
    MissionAbilityRecordPtr abilityRecord_{ nullptr };
private:
    std::shared_ptr<CallContainer> callContainer_{ nullptr };

    int MOCK_MAIN_USER_ID = 100;
};

void CallContainerTest::SetUpTestCase(void) {}
void CallContainerTest::TearDownTestCase(void) {}
void CallContainerTest::TearDown() {}

void CallContainerTest::SetUp()
{
    callContainer_ = std::make_shared<CallContainer>();
    AbilityRequest abilityRequest;
    abilityRecord_ = MissionAbilityRecord::CreateAbilityRecord(abilityRequest);
}

std::shared_ptr<CallContainer> CallContainerTest::get() const
{
    return callContainer_;
}

/*
 * Feature: CallContainer
 * Function: AddCallRecord
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions:NA
 * CaseDescription: Verify funtion call called
 */
HWTEST_F(CallContainerTest, Call_Container_Add_Call_Record_001, TestSize.Level1)
{
    std::shared_ptr<CallContainer> callContainer = get();
    AbilityRequest abilityRequest;
    abilityRequest.callerUid = 1;
    abilityRequest.callType = AbilityCallType::CALL_REQUEST_TYPE;
    abilityRequest.connect = new AbilityConnectCallback();
    std::shared_ptr<CallRecord> callRecord = CallRecord::CreateCallRecord(
        abilityRequest.callerUid, abilityRecord_->shared_from_this(),
        abilityRequest.connect, abilityRequest.callerToken);
    callContainer->AddCallRecord(abilityRequest.connect, callRecord);

    std::shared_ptr<CallRecord> getCallRecord = callContainer->GetCallRecord(abilityRequest.connect);
    EXPECT_EQ(callRecord, getCallRecord);
}

/*
 * Feature: CallContainer
 * Function: GetCallRecord
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions:NA
 * CaseDescription: Verify funtion call called
 */
HWTEST_F(CallContainerTest, Call_Container_Get_Call_Record_001, TestSize.Level1)
{
    sptr<IAbilityConnection> connect = new AbilityConnectCallback();
    std::shared_ptr<CallContainer> callContainer = get();
    std::shared_ptr<CallRecord> getCallRecord = callContainer->GetCallRecord(connect);
    EXPECT_EQ(nullptr, getCallRecord);
}

/*
 * Feature: CallContainer
 * Function: RemoveCallRecord
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions:NA
 * CaseDescription: Verify funtion call called
 */
HWTEST_F(CallContainerTest, Call_Container_Remove_Call_Record_001, TestSize.Level1)
{
    sptr<IAbilityConnection> connect = new AbilityConnectCallback();
    std::shared_ptr<CallContainer> callContainer = get();
    bool result = callContainer->RemoveCallRecord(connect);
    EXPECT_EQ(result, false);
}

/*
 * Feature: CallContainer
 * Function: RemoveCallRecord
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions:NA
 * CaseDescription: Verify funtion call called
 */
HWTEST_F(CallContainerTest, Call_Container_Remove_Call_Record_002, TestSize.Level1)
{
    std::shared_ptr<CallContainer> callContainer = get();
    AbilityRequest abilityRequest;
    abilityRequest.callerUid = 1;
    abilityRequest.callType = AbilityCallType::CALL_REQUEST_TYPE;
    abilityRequest.connect = new AbilityConnectCallback();
    std::shared_ptr<CallRecord> callRecord = CallRecord::CreateCallRecord(
        abilityRequest.callerUid, abilityRecord_->shared_from_this(),
        abilityRequest.connect, abilityRequest.callerToken);
    callContainer->AddCallRecord(abilityRequest.connect, callRecord);

    bool result = callContainer->RemoveCallRecord(abilityRequest.connect);
    EXPECT_EQ(result, true);
}

/*
 * Feature: CallContainer
 * Function: CallRequestDone
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions:NA
 * CaseDescription: Verify funtion call called
 */
HWTEST_F(CallContainerTest, Call_Container_Call_Request_Done_001, TestSize.Level1)
{
    std::shared_ptr<CallContainer> callContainer = get();
    OHOS::sptr<IAbilityScheduler> scheduler = new AbilitySchedulerMock();
    abilityRecord_->SetScheduler(scheduler);
    scheduler->CallRequest();
    bool result = callContainer->CallRequestDone(nullptr);
    EXPECT_EQ(result, false);
}

/*
 * Feature: CallContainer
 * Function: CallRequestDone
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions:NA
 * CaseDescription: Verify funtion call called
 */
HWTEST_F(CallContainerTest, Call_Container_Call_Request_Done_002, TestSize.Level1)
{
    class AbilitySchedulerMockFunction : public AbilitySchedulerMock {
    public:
        sptr<IRemoteObject> CallRequestModify() { return this; }
    };

    std::shared_ptr<CallContainer> callContainer = get();
    auto scheduler = new AbilitySchedulerMockFunction();
    sptr<IRemoteObject> object = scheduler->CallRequestModify();
    bool result = callContainer->CallRequestDone(object);
    EXPECT_EQ(result, true);
}

/*
 * Feature: CallContainer
 * Function: Dump
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions:NA
 * CaseDescription: Verify funtion call called
 */
HWTEST_F(CallContainerTest, Call_Container_Dump_001, TestSize.Level1)
{
    std::shared_ptr<CallContainer> callContainer = get();
    std::vector<std::string> dumpInfo;
    callContainer->Dump(dumpInfo);
    EXPECT_EQ(dumpInfo.size(), 0);
}

/*
 * Feature: CallContainer
 * Function: Dump
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions:NA
 * CaseDescription: Verify funtion call called
 */
HWTEST_F(CallContainerTest, Call_Container_Dump_002, TestSize.Level1)
{
    std::shared_ptr<CallContainer> callContainer = get();
    AbilityRequest abilityRequest;
    abilityRequest.callerUid = 1;
    abilityRequest.callType = AbilityCallType::CALL_REQUEST_TYPE;
    abilityRequest.connect = new AbilityConnectCallback();
    std::shared_ptr<CallRecord> callRecord = CallRecord::CreateCallRecord(
        abilityRequest.callerUid, abilityRecord_->shared_from_this(),
        abilityRequest.connect, abilityRequest.callerToken);
    callContainer->AddCallRecord(abilityRequest.connect, callRecord);

    std::vector<std::string> dumpInfo;
    callContainer->Dump(dumpInfo);
    EXPECT_NE(dumpInfo.size(), 0);
}

/*
 * Feature: CallContainer
 * Function: IsNeedToCallRequest
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions:NA
 * CaseDescription: Verify funtion call called
 */
HWTEST_F(CallContainerTest, Call_Container_Is_Need_To_Call_Request_001, TestSize.Level1)
{
    std::shared_ptr<CallContainer> callContainer = get();
    EXPECT_EQ(callContainer->IsNeedToCallRequest(), false);
}

/*
 * Feature: CallContainer
 * Function: IsNeedToCallRequest
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions:NA
 * CaseDescription: Verify funtion call called
 */
HWTEST_F(CallContainerTest, Call_Container_Is_Need_To_Call_Request_002, TestSize.Level1)
{
    std::shared_ptr<CallContainer> callContainer = get();
    AbilityRequest abilityRequest;
    abilityRequest.callerUid = 1;
    abilityRequest.callType = AbilityCallType::CALL_REQUEST_TYPE;
    abilityRequest.connect = new AbilityConnectCallback();
    std::shared_ptr<CallRecord> callRecord = CallRecord::CreateCallRecord(
        abilityRequest.callerUid, abilityRecord_->shared_from_this(),
        abilityRequest.connect, abilityRequest.callerToken);
    callRecord->SetCallState(CallState::INIT);
    callContainer->AddCallRecord(abilityRequest.connect, callRecord);
    EXPECT_EQ(callContainer->IsNeedToCallRequest(), true);
}

/*
 * Feature: CallContainer
 * Function: IsNeedToCallRequest
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions:NA
 * CaseDescription: Verify funtion call called
 */
HWTEST_F(CallContainerTest, Call_Container_Is_Need_To_Call_Request_003, TestSize.Level1)
{
    std::shared_ptr<CallContainer> callContainer = get();
    AbilityRequest abilityRequest;
    abilityRequest.callerUid = 1;
    abilityRequest.callType = AbilityCallType::CALL_REQUEST_TYPE;
    abilityRequest.connect = new AbilityConnectCallback();
    std::shared_ptr<CallRecord> callRecord = CallRecord::CreateCallRecord(
        abilityRequest.callerUid, abilityRecord_->shared_from_this(),
        abilityRequest.connect, abilityRequest.callerToken);
    callRecord->SetCallState(CallState::REQUESTING);
    callContainer->AddCallRecord(abilityRequest.connect, callRecord);
    EXPECT_EQ(callContainer->IsNeedToCallRequest(), true);
}

/*
 * Feature: CallContainer
 * Function: IsNeedToCallRequest
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions:NA
 * CaseDescription: Verify funtion call called
 */
HWTEST_F(CallContainerTest, Call_Container_Is_Need_To_Call_Request_004, TestSize.Level1)
{
    std::shared_ptr<CallContainer> callContainer = get();
    AbilityRequest abilityRequest;
    abilityRequest.callerUid = 1;
    abilityRequest.callType = AbilityCallType::CALL_REQUEST_TYPE;
    abilityRequest.connect = new AbilityConnectCallback();
    std::shared_ptr<CallRecord> callRecord = CallRecord::CreateCallRecord(
        abilityRequest.callerUid, abilityRecord_->shared_from_this(),
        abilityRequest.connect, abilityRequest.callerToken);
    callRecord->SetCallState(CallState::REQUESTED);
    callContainer->AddCallRecord(abilityRequest.connect, callRecord);
    EXPECT_EQ(callContainer->IsNeedToCallRequest(), false);
}

/*
 * Feature: CallContainer
 * Function: AddConnectDeathRecipient
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions:NA
 * CaseDescription: Verify funtion call called
 */
HWTEST_F(CallContainerTest, Call_Container_Add_Connect_Death_Recipient_001, TestSize.Level1)
{
    std::shared_ptr<CallContainer> callContainer = get();
    sptr<IAbilityConnection> connect = new AbilityConnectCallback();
    callContainer->AddConnectDeathRecipient(connect);
    EXPECT_EQ(callContainer->deathRecipientMap_.size(), 1);
}

/*
 * Feature: CallContainer
 * Function: RemoveConnectDeathRecipient
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions:NA
 * CaseDescription: Verify funtion call called
 */
HWTEST_F(CallContainerTest, Call_Container_Remove_Connect_Death_Recipient_001, TestSize.Level1)
{
    std::shared_ptr<CallContainer> callContainer = get();
    sptr<IAbilityConnection> connect = new AbilityConnectCallback();
    callContainer->AddConnectDeathRecipient(connect);
    callContainer->RemoveConnectDeathRecipient(connect);
    EXPECT_EQ(callContainer->deathRecipientMap_.size(), 0);
}

/*
 * Feature: CallContainer
 * Function: OnConnectionDied
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions:NA
 * CaseDescription: Verify funtion call called
 */
HWTEST_F(CallContainerTest, Call_Container_On_Connect_Died_001, TestSize.Level1)
{
    std::shared_ptr<CallContainer> callContainer = get();
    EXPECT_EQ(callContainer->callRecordMap_.size(), 0);

    AbilityRequest abilityRequest;
    abilityRequest.callerUid = 1;
    abilityRequest.callType = AbilityCallType::CALL_REQUEST_TYPE;
    abilityRequest.connect = new AbilityConnectCallback();
    std::shared_ptr<CallRecord> callRecord = CallRecord::CreateCallRecord(
        abilityRequest.callerUid, abilityRecord_->shared_from_this(),
        abilityRequest.connect, abilityRequest.callerToken);
    callRecord->SetCallState(CallState::REQUESTED);
    callContainer->AddCallRecord(abilityRequest.connect, callRecord);
    EXPECT_EQ(callContainer->callRecordMap_.size(), 1);

    auto mission = std::make_shared<Mission>(0, abilityRecord_, "launcher");
    auto missionList = std::make_shared<MissionList>();
    missionList->AddMissionToTop(mission);
    abilityRecord_->callContainer_ = callContainer;

    std::shared_ptr<MissionListManager> missionListMgr = std::make_shared<MissionListManager>(0);
    missionListMgr->currentMissionLists_.push_front(missionList);
    DelayedSingleton<AbilityManagerService>::GetInstance()->subManagersHelper_ =
        std::make_shared<SubManagersHelper>(nullptr, nullptr);
    DelayedSingleton<AbilityManagerService>::GetInstance()->subManagersHelper_->currentMissionListManager_ =
        missionListMgr;
    callContainer->OnConnectionDied(abilityRequest.connect->AsObject());

    EXPECT_EQ(callContainer->callRecordMap_.size(), 1);
}

/*
 * Feature: CallContainer
 * Function: IsExistConnection
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions:NA
 * CaseDescription: Verify IsExistConnection funtion call called
 */
HWTEST_F(CallContainerTest, Call_Container_Is_Exist_Connection_001, TestSize.Level1)
{
    std::shared_ptr<CallContainer> callContainer = get();
    sptr<IAbilityConnection> connect = new AbilityConnectCallback();
    EXPECT_FALSE(callContainer->IsExistConnection(connect));
}

/*
 * Feature: CallContainer
 * Function: NotifyAllCallDisconnect
 * SubFunction: NA
 * FunctionPoints: Notify all callers when callee ability is terminating
 * EnvConditions:NA
 * CaseDescription: Verify NotifyAllCallDisconnect notifies all registered callers
 */
HWTEST_F(CallContainerTest, Call_Container_Notify_All_Call_Disconnect_001, TestSize.Level1)
{
    std::shared_ptr<CallContainer> callContainer = get();

    // Create multiple call records
    AbilityRequest abilityRequest1;
    abilityRequest1.callerUid = 1;
    abilityRequest1.callType = AbilityCallType::CALL_REQUEST_TYPE;
    auto connect1 = sptr<AbilityConnectCallback>::MakeSptr();
    std::shared_ptr<CallRecord> callRecord1 = CallRecord::CreateCallRecord(
        abilityRequest1.callerUid, abilityRecord_->shared_from_this(),
        connect1, abilityRequest1.callerToken);
    callRecord1->SetCallState(CallState::REQUESTED);
    callContainer->AddCallRecord(connect1, callRecord1);

    AbilityRequest abilityRequest2;
    abilityRequest2.callerUid = 2;
    abilityRequest2.callType = AbilityCallType::CALL_REQUEST_TYPE;
    auto connect2 = sptr<AbilityConnectCallback>::MakeSptr();
    std::shared_ptr<CallRecord> callRecord2 = CallRecord::CreateCallRecord(
        abilityRequest2.callerUid, abilityRecord_->shared_from_this(),
        connect2, abilityRequest2.callerToken);
    callRecord2->SetCallState(CallState::REQUESTED);
    callContainer->AddCallRecord(connect2, callRecord2);

    // Verify records are added
    EXPECT_EQ(callContainer->GetCallRecord(connect1), callRecord1);
    EXPECT_EQ(callContainer->GetCallRecord(connect2), callRecord2);

    // Create element name for callee
    AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.bundleName = "com.test.callee";
    abilityInfo.name = "CalleeAbility";
    abilityInfo.moduleName = "entry";
    AppExecFwk::ElementName element("deviceId", abilityInfo.bundleName,
        abilityInfo.name, abilityInfo.moduleName);

    // Notify all disconnect
    callContainer->NotifyAllCallDisconnect(element);

    // Verify all records are cleared
    EXPECT_EQ(callContainer->GetCallRecord(connect1), nullptr);
    EXPECT_EQ(callContainer->GetCallRecord(connect2), nullptr);
}

/*
 * Feature: CallContainer
 * Function: NotifyAllCallDisconnect
 * SubFunction: NA
 * FunctionPoints: Notify all callers when callee ability is terminating
 * EnvConditions:NA
 * CaseDescription: Verify NotifyAllCallDisconnect handles empty call records
 */
HWTEST_F(CallContainerTest, Call_Container_Notify_All_Call_Disconnect_002, TestSize.Level1)
{
    std::shared_ptr<CallContainer> callContainer = get();

    AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.bundleName = "com.test.callee";
    abilityInfo.name = "CalleeAbility";
    abilityInfo.moduleName = "entry";
    AppExecFwk::ElementName element("deviceId", abilityInfo.bundleName,
        abilityInfo.name, abilityInfo.moduleName);

    // Should not crash with empty records
    callContainer->NotifyAllCallDisconnect(element);

    // Verify no records exist
    EXPECT_TRUE(callContainer->EmptyCallRecordMap());
}

/*
 * Feature: CallContainer
 * Function: NotifyAllCallDisconnect
 * SubFunction: NA
 * FunctionPoints: Notify all callers when callee ability is terminating
 * EnvConditions:NA
 * CaseDescription: Verify NotifyAllCallDisconnect only notifies REQUESTED state calls
 */
HWTEST_F(CallContainerTest, Call_Container_Notify_All_Call_Disconnect_003, TestSize.Level1)
{
    std::shared_ptr<CallContainer> callContainer = get();

    // Add call record in REQUESTING state (should not be notified)
    AbilityRequest abilityRequest;
    abilityRequest.callerUid = 1;
    abilityRequest.callType = AbilityCallType::CALL_REQUEST_TYPE;
    auto connect = sptr<AbilityConnectCallback>::MakeSptr();
    std::shared_ptr<CallRecord> callRecord = CallRecord::CreateCallRecord(
        abilityRequest.callerUid, abilityRecord_->shared_from_this(),
        connect, abilityRequest.callerToken);
    callRecord->SetCallState(CallState::REQUESTING);
    callContainer->AddCallRecord(connect, callRecord);

    AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.bundleName = "com.test.callee";
    abilityInfo.name = "CalleeAbility";
    abilityInfo.moduleName = "entry";
    AppExecFwk::ElementName element("deviceId", abilityInfo.bundleName,
        abilityInfo.name, abilityInfo.moduleName);

    // Notify all disconnect
    callContainer->NotifyAllCallDisconnect(element);

    // Verify records are cleared even if not in REQUESTED state
    EXPECT_EQ(callContainer->GetCallRecord(connect), nullptr);
}

/*
 * Feature: CallContainer
 * Function: NotifyDisconnectOnTimeout
 * SubFunction: NA
 * FunctionPoints: REQUESTING state CallRecord is notified (unlike NotifyAllCallDisconnect)
 * EnvConditions: NA
 * CaseDescription: Verify NotifyDisconnectOnTimeout notifies REQUESTING state records
 */
HWTEST_F(CallContainerTest, Call_Container_Notify_Disconnect_On_Prelaunch_Timeout_Requesting_001, TestSize.Level1)
{
    std::shared_ptr<CallContainer> callContainer = get();
    AppExecFwk::ElementName element("deviceId", "com.test.callee", "CalleeAbility", "entry");
    EXPECT_TRUE(callContainer->callRecordMap_.empty());
    callContainer->NotifyDisconnectOnTimeout(element);

    AbilityRequest abilityRequest;
    abilityRequest.callerUid = 1;
    abilityRequest.callType = AbilityCallType::CALL_REQUEST_TYPE;
    auto connect = sptr<AbilityConnectCallback>::MakeSptr();
    std::shared_ptr<CallRecord> callRecord = CallRecord::CreateCallRecord(
        abilityRequest.callerUid, abilityRecord_->shared_from_this(),
        connect, abilityRequest.callerToken);
    callRecord->SetCallState(CallState::REQUESTING);
    callRecord->SetPrelaunchFlag(true);
    callContainer->AddCallRecord(connect, callRecord);

    AbilityConnectCallback::onAbilityDisconnectDoneCount = 0;
    callContainer->NotifyDisconnectOnTimeout(element);
    usleep(100000); // ffrt::submit is async — wait for callback to complete
    EXPECT_EQ(AbilityConnectCallback::onAbilityDisconnectDoneCount, 1);
    EXPECT_EQ(callContainer->GetCallRecord(connect), nullptr);
}

/*
 * Feature: CallContainer
 * Function: NotifyDisconnectOnTimeout
 * SubFunction: NA
 * FunctionPoints: non-prelaunch CallRecord is skipped
 * EnvConditions: NA
 * CaseDescription: Verify NotifyDisconnectOnTimeout does not notify non-prelaunch records
 */
HWTEST_F(CallContainerTest, Call_Container_Notify_Disconnect_On_Prelaunch_Timeout_NonPrelaunch_001, TestSize.Level1)
{
    std::shared_ptr<CallContainer> callContainer = get();
    AppExecFwk::ElementName element("deviceId", "com.test.callee", "CalleeAbility", "entry");

    AbilityRequest abilityRequest;
    abilityRequest.callerUid = 1;
    abilityRequest.callType = AbilityCallType::CALL_REQUEST_TYPE;
    auto connect = sptr<AbilityConnectCallback>::MakeSptr();
    std::shared_ptr<CallRecord> callRecord = CallRecord::CreateCallRecord(
        abilityRequest.callerUid, abilityRecord_->shared_from_this(),
        connect, abilityRequest.callerToken);
    callRecord->SetCallState(CallState::REQUESTING);
    // isPrelaunch defaults to false — do NOT set prelaunch flag
    callContainer->AddCallRecord(connect, callRecord);

    AbilityConnectCallback::onAbilityDisconnectDoneCount = 0;
    callContainer->NotifyDisconnectOnTimeout(element);
    usleep(100000); // ffrt::submit is async — wait for callback to complete
    EXPECT_EQ(AbilityConnectCallback::onAbilityDisconnectDoneCount, 0);
    // non-prelaunch record should still be in the map
    EXPECT_NE(callContainer->GetCallRecord(connect), nullptr);
}
}  // namespace AAFwk
}  // namespace OHOS
