/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include <functional>
#include <memory>
#include <string>
#include <ctime>
#include <vector>

#include <gtest/gtest.h>

#define private public
#define protected public
#include "ui_extension_ability_manager.h"
#undef private
#undef protected

#include "ability_manager_errors.h"
#include "app_scheduler.h"
#include "errors.h"
#include "extension_record.h"
#include "ipc_skeleton.h"
#include "iremote_object.h"
#include "message_option.h"
#include "message_parcel.h"
#include "mock_task_handler_wrap.h"

using namespace testing::ext;

namespace OHOS {
namespace AAFwk {
namespace {
const std::string UIEXTENSION_LAUNCH_TIMESTAMP_HIGH = "ohos.ability.params.uiExtensionLaunchTimestampHigh";

sptr<SessionInfo> MockSessionInfo(int32_t persistentId)
{
    auto sessionInfo = sptr<SessionInfo>::MakeSptr();
    if (sessionInfo != nullptr) {
        sessionInfo->persistentId = persistentId;
    }
    return sessionInfo;
}
}

class MockPreloadHostClient final : public IRemoteObject {
public:
    explicit MockPreloadHostClient(bool addDeathRecipientResult)
        : IRemoteObject(u"mock_preload_host_client"), addDeathRecipientResult_(addDeathRecipientResult)
    {}

    ~MockPreloadHostClient() override = default;

    int32_t GetObjectRefCount() override
    {
        return 0;
    }

    int SendRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override
    {
        (void)code;
        (void)data;
        (void)reply;
        (void)option;
        return ERR_OK;
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
        addDeathRecipientCount_++;
        deathRecipient_ = recipient;
        return addDeathRecipientResult_;
    }

    bool RemoveDeathRecipient(const sptr<DeathRecipient> &recipient) override
    {
        removeDeathRecipientCount_++;
        return recipient == deathRecipient_;
    }

    bool Marshalling(Parcel &parcel) const override
    {
        (void)parcel;
        return true;
    }

    sptr<IRemoteBroker> AsInterface() override
    {
        return nullptr;
    }

    int Dump(int fd, const std::vector<std::u16string> &args) override
    {
        (void)fd;
        (void)args;
        return ERR_OK;
    }

    bool addDeathRecipientResult_ = true;
    int32_t addDeathRecipientCount_ = 0;
    int32_t removeDeathRecipientCount_ = 0;
    sptr<DeathRecipient> deathRecipient_ = nullptr;
};

class MockUIExtensionAbilityManager final : public UIExtensionAbilityManager {
public:
    explicit MockUIExtensionAbilityManager(int userId) : UIExtensionAbilityManager(userId) {}
    ~MockUIExtensionAbilityManager() override = default;

    int32_t StartAbilityLocked(const AbilityRequest &abilityRequest) override
    {
        completedRequestCodes_.emplace_back(abilityRequest.requestCode);
        return ERR_OK;
    }

    std::vector<int32_t> completedRequestCodes_;
};

class UIExtensionAbilityManagerThirdTest : public testing::Test {};

/*
 * Feature: UIExtensionAbilityManager
 * Function: OnAbilityRequestDone
 * CaseDescription: Verify a foregrounding UIExtension retains its modal launch timestamp
 */
HWTEST_F(UIExtensionAbilityManagerThirdTest, OnAbilityRequestDone_001, TestSize.Level1)
{
    auto connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    ASSERT_NE(connectManager, nullptr);

    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.extensionAbilityType = AppExecFwk::ExtensionAbilityType::SYS_COMMON_UI;
    abilityRequest.abilityInfo.bundleName = "com.test.uiextension";
    abilityRequest.abilityInfo.name = "TestUIExtension";
    auto abilityRecord = BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    ASSERT_NE(abilityRecord, nullptr);
    abilityRecord->SetAbilityState(AbilityState::FOREGROUNDING);
    auto sessionInfo = sptr<SessionInfo>::MakeSptr();
    ASSERT_NE(sessionInfo, nullptr);
    sessionInfo->persistentId = 1;
    sessionInfo->uiExtensionUsage = AppExecFwk::UIExtensionUsage::MODAL;
    abilityRecord->SetSessionInfo(sessionInfo);
    abilityRecord->AddUIExtensionLaunchTimestamp();
    EXPECT_NE(abilityRecord->GetWant().GetIntParam(UIEXTENSION_LAUNCH_TIMESTAMP_HIGH, -1), -1);
    connectManager->CallAddToServiceMap("uiextension", abilityRecord);

    connectManager->OnAbilityRequestDone(abilityRecord->GetToken(),
        static_cast<int32_t>(AppAbilityState::ABILITY_STATE_FOREGROUND));
    EXPECT_NE(abilityRecord->GetWant().GetIntParam(UIEXTENSION_LAUNCH_TIMESTAMP_HIGH, -1), -1);
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: OnAbilityRequestDone
 * CaseDescription: Verify an already foreground UIExtension skips foreground and retains timestamp
 */
HWTEST_F(UIExtensionAbilityManagerThirdTest, OnAbilityRequestDone_002, TestSize.Level1)
{
    auto connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    ASSERT_NE(connectManager, nullptr);

    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.extensionAbilityType = AppExecFwk::ExtensionAbilityType::SYS_COMMON_UI;
    abilityRequest.abilityInfo.bundleName = "com.test.uiextension";
    abilityRequest.abilityInfo.name = "TestUIExtension";
    auto abilityRecord = BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    ASSERT_NE(abilityRecord, nullptr);
    abilityRecord->SetAbilityState(AbilityState::FOREGROUND);
    auto sessionInfo = sptr<SessionInfo>::MakeSptr();
    ASSERT_NE(sessionInfo, nullptr);
    sessionInfo->persistentId = 1;
    sessionInfo->uiExtensionUsage = AppExecFwk::UIExtensionUsage::MODAL;
    abilityRecord->SetSessionInfo(sessionInfo);
    abilityRecord->AddUIExtensionLaunchTimestamp();
    EXPECT_NE(abilityRecord->GetWant().GetIntParam(UIEXTENSION_LAUNCH_TIMESTAMP_HIGH, -1), -1);
    connectManager->CallAddToServiceMap("uiextension", abilityRecord);

    connectManager->OnAbilityRequestDone(abilityRecord->GetToken(),
        static_cast<int32_t>(AppAbilityState::ABILITY_STATE_FOREGROUND));
    EXPECT_NE(abilityRecord->GetWant().GetIntParam(UIEXTENSION_LAUNCH_TIMESTAMP_HIGH, -1), -1);
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: BackgroundAbilityWindowLocked
 * CaseDescription: Verify a newer background request removes queued foreground requests for the same record
 */
HWTEST_F(UIExtensionAbilityManagerThirdTest, BackgroundAbilityWindowLocked_002, TestSize.Level1)
{
    auto connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    auto taskHandler = AppExecFwk::MockTaskHandlerWrap::CreateQueueHandler(
        "DoBackgroundAbilityWindow_002");
    ASSERT_NE(taskHandler, nullptr);
    connectManager->SetTaskHandler(taskHandler);
    EXPECT_CALL(*taskHandler, SubmitTaskInner(testing::_, testing::_))
        .Times(testing::AnyNumber()).WillRepeatedly(testing::Return(nullptr));
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.extensionAbilityType = AppExecFwk::ExtensionAbilityType::SYS_COMMON_UI;
    abilityRequest.sessionInfo = MockSessionInfo(1);
    auto abilityRecord = BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    ASSERT_NE(abilityRecord, nullptr);
    const std::string recordUri = "ui_extension_record_1";
    abilityRecord->SetURI(recordUri);
    abilityRecord->SetAbilityState(AbilityState::BACKGROUNDING);

    connectManager->DoForegroundUIExtension(abilityRecord, abilityRequest);
    auto requestIt = connectManager->uiExtensionForegroundRequestMap_.find(recordUri);
    ASSERT_NE(requestIt, connectManager->uiExtensionForegroundRequestMap_.end());
    ASSERT_NE(requestIt->second, nullptr);
    EXPECT_EQ(requestIt->second->requests.size(), 1);
    EXPECT_EQ(connectManager->startServiceReqList_.count(recordUri), 0);
    const std::string timeoutTaskName = "ui_extension_foreground_request_timeout:" + recordUri;
    EXPECT_EQ(taskHandler->tasks_.count(timeoutTaskName), 1);

    connectManager->BackgroundAbilityWindowLocked(abilityRecord, abilityRequest.sessionInfo);

    EXPECT_EQ(connectManager->uiExtensionForegroundRequestMap_.count(recordUri), 0);
    EXPECT_EQ(taskHandler->tasks_.count(timeoutTaskName), 0);
    EXPECT_EQ(abilityRecord->GetPendingState(), AbilityState::BACKGROUND);
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: BackgroundAbilityWindowLocked
 * CaseDescription: Verify cancellation is record-scoped and a later foreground request can be queued again
 */
HWTEST_F(UIExtensionAbilityManagerThirdTest, BackgroundAbilityWindowLocked_003, TestSize.Level1)
{
    auto connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    auto taskHandler = AppExecFwk::MockTaskHandlerWrap::CreateQueueHandler(
        "DoBackgroundAbilityWindow_003");
    ASSERT_NE(taskHandler, nullptr);
    connectManager->SetTaskHandler(taskHandler);
    EXPECT_CALL(*taskHandler, SubmitTaskInner(testing::_, testing::_))
        .Times(testing::AnyNumber()).WillRepeatedly(testing::Return(nullptr));
    AbilityRequest oldForegroundRequest;
    oldForegroundRequest.abilityInfo.extensionAbilityType = AppExecFwk::ExtensionAbilityType::SYS_COMMON_UI;
    oldForegroundRequest.sessionInfo = MockSessionInfo(1);
    oldForegroundRequest.requestCode = 1;
    auto abilityRecord = BaseExtensionRecord::CreateBaseExtensionRecord(oldForegroundRequest);
    ASSERT_NE(abilityRecord, nullptr);
    const std::string recordUri = "ui_extension_record_1";
    const std::string otherRecordUri = "ui_extension_record_2";
    abilityRecord->SetURI(recordUri);
    abilityRecord->SetAbilityState(AbilityState::BACKGROUNDING);

    connectManager->DoForegroundUIExtension(abilityRecord, oldForegroundRequest);
    AbilityRequest repeatedForegroundRequest = oldForegroundRequest;
    repeatedForegroundRequest.requestCode = 11;
    connectManager->DoForegroundUIExtension(abilityRecord, repeatedForegroundRequest);
    connectManager->EnqueueStartServiceReq(oldForegroundRequest, otherRecordUri);
    auto oldRequestIt = connectManager->uiExtensionForegroundRequestMap_.find(recordUri);
    ASSERT_NE(oldRequestIt, connectManager->uiExtensionForegroundRequestMap_.end());
    ASSERT_NE(oldRequestIt->second, nullptr);
    ASSERT_EQ(oldRequestIt->second->requests.size(), 2);
    connectManager->BackgroundAbilityWindowLocked(abilityRecord, oldForegroundRequest.sessionInfo);

    EXPECT_EQ(connectManager->uiExtensionForegroundRequestMap_.count(recordUri), 0);
    EXPECT_EQ(connectManager->uiExtensionForegroundRequestMap_.count(otherRecordUri), 1);
    EXPECT_EQ(taskHandler->tasks_.count("ui_extension_foreground_request_timeout:" + recordUri), 0);
    EXPECT_EQ(taskHandler->tasks_.count("ui_extension_foreground_request_timeout:" + otherRecordUri), 1);

    AbilityRequest newForegroundRequest = oldForegroundRequest;
    newForegroundRequest.requestCode = 2;
    connectManager->DoForegroundUIExtension(abilityRecord, newForegroundRequest);

    auto requestIt = connectManager->uiExtensionForegroundRequestMap_.find(recordUri);
    ASSERT_NE(requestIt, connectManager->uiExtensionForegroundRequestMap_.end());
    ASSERT_NE(requestIt->second, nullptr);
    ASSERT_EQ(requestIt->second->requests.size(), 1);
    EXPECT_EQ(requestIt->second->requests.front().abilityRequest.requestCode, 2);
    EXPECT_EQ(taskHandler->tasks_.count("ui_extension_foreground_request_timeout:" + recordUri), 1);
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: DoBackgroundAbilityWindow
 * CaseDescription: Verify delayed background reentry does not remove a later foreground request
 */
HWTEST_F(UIExtensionAbilityManagerThirdTest, DoBackgroundAbilityWindow_004, TestSize.Level1)
{
    auto connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    auto taskHandler = AppExecFwk::MockTaskHandlerWrap::CreateQueueHandler(
        "DoBackgroundAbilityWindow_004");
    ASSERT_NE(taskHandler, nullptr);
    connectManager->SetTaskHandler(taskHandler);
    EXPECT_CALL(*taskHandler, SubmitTaskInner(testing::_, testing::_))
        .Times(testing::AnyNumber()).WillRepeatedly(testing::Return(nullptr));

    AbilityRequest oldForegroundRequest;
    oldForegroundRequest.abilityInfo.extensionAbilityType = AppExecFwk::ExtensionAbilityType::SYS_COMMON_UI;
    oldForegroundRequest.sessionInfo = MockSessionInfo(1);
    oldForegroundRequest.requestCode = 1;
    auto abilityRecord = BaseExtensionRecord::CreateBaseExtensionRecord(oldForegroundRequest);
    ASSERT_NE(abilityRecord, nullptr);
    const std::string recordUri = "ui_extension_record_1";
    abilityRecord->SetURI(recordUri);
    abilityRecord->SetAbilityState(AbilityState::FOREGROUNDING);

    connectManager->DoForegroundUIExtension(abilityRecord, oldForegroundRequest);
    connectManager->BackgroundAbilityWindowLocked(abilityRecord, oldForegroundRequest.sessionInfo);
    EXPECT_EQ(connectManager->uiExtensionForegroundRequestMap_.count(recordUri), 0);

    AbilityRequest laterForegroundRequest = oldForegroundRequest;
    laterForegroundRequest.requestCode = 2;
    connectManager->DoForegroundUIExtension(abilityRecord, laterForegroundRequest);

    // Simulate the delayed background lifecycle reentry. It must not clear the later request.
    connectManager->DoBackgroundAbilityWindow(abilityRecord, laterForegroundRequest.sessionInfo);

    auto requestIt = connectManager->uiExtensionForegroundRequestMap_.find(recordUri);
    ASSERT_NE(requestIt, connectManager->uiExtensionForegroundRequestMap_.end());
    ASSERT_NE(requestIt->second, nullptr);
    ASSERT_EQ(requestIt->second->requests.size(), 1);
    EXPECT_EQ(requestIt->second->requests.front().abilityRequest.requestCode, laterForegroundRequest.requestCode);
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: EnqueueStartServiceReq
 * CaseDescription: Verify an executing timeout task cannot remove a recreated request queue
 */
HWTEST_F(UIExtensionAbilityManagerThirdTest, UIExtensionForegroundRequest_001, TestSize.Level1)
{
    auto connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    auto taskHandler = AppExecFwk::MockTaskHandlerWrap::CreateQueueHandler("UIExtensionForegroundRequest_001");
    ASSERT_NE(taskHandler, nullptr);
    connectManager->SetTaskHandler(taskHandler);

    std::vector<std::function<void()>> timeoutTasks;
    EXPECT_CALL(*taskHandler, SubmitTaskInner(testing::_, testing::_))
        .Times(testing::AnyNumber())
        .WillRepeatedly(testing::Invoke([&timeoutTasks](std::function<void()> &&task,
            const TaskAttribute &taskAttr) -> std::shared_ptr<InnerTaskHandle> {
            if (taskAttr.taskName_.find("ui_extension_foreground_request_timeout:") == 0) {
                timeoutTasks.emplace_back(std::move(task));
            }
            return nullptr;
        }));

    const std::string serviceUri = "ui_extension_record_timeout";
    AbilityRequest firstRequest;
    firstRequest.requestCode = 1;
    connectManager->EnqueueStartServiceReq(firstRequest, serviceUri);
    ASSERT_EQ(timeoutTasks.size(), 1);
    // Keep Q1 alive so its timeout reaches the map identity check after Q2 is created.
    auto oldRequestQueue = connectManager->uiExtensionForegroundRequestMap_[serviceUri];
    ASSERT_NE(oldRequestQueue, nullptr);

    struct timespec cutoffTime = {};
    ASSERT_EQ(clock_gettime(CLOCK_MONOTONIC, &cutoffTime), 0);
    connectManager->RemoveUIExtensionForegroundRequest(serviceUri, cutoffTime, true);

    AbilityRequest laterRequest;
    laterRequest.requestCode = 2;
    connectManager->EnqueueStartServiceReq(laterRequest, serviceUri);
    ASSERT_EQ(timeoutTasks.size(), 2);

    timeoutTasks.front()();

    auto requestIt = connectManager->uiExtensionForegroundRequestMap_.find(serviceUri);
    ASSERT_NE(requestIt, connectManager->uiExtensionForegroundRequestMap_.end());
    ASSERT_NE(requestIt->second, nullptr);
    ASSERT_EQ(requestIt->second->requests.size(), 1);
    EXPECT_EQ(requestIt->second->requests.front().abilityRequest.requestCode, laterRequest.requestCode);
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: EnqueueStartServiceReq
 * CaseDescription: Verify a queue timeout removes all requests waiting for the same start operation
 */
HWTEST_F(UIExtensionAbilityManagerThirdTest, UIExtensionForegroundRequest_002, TestSize.Level1)
{
    auto connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    auto taskHandler = AppExecFwk::MockTaskHandlerWrap::CreateQueueHandler("UIExtensionForegroundRequest_002");
    ASSERT_NE(taskHandler, nullptr);
    connectManager->SetTaskHandler(taskHandler);

    std::vector<std::function<void()>> timeoutTasks;
    EXPECT_CALL(*taskHandler, SubmitTaskInner(testing::_, testing::_))
        .Times(testing::AnyNumber())
        .WillRepeatedly(testing::Invoke([&timeoutTasks](std::function<void()> &&task,
            const TaskAttribute &taskAttr) -> std::shared_ptr<InnerTaskHandle> {
            if (taskAttr.taskName_.find("ui_extension_foreground_request_timeout:") == 0) {
                timeoutTasks.emplace_back(std::move(task));
            }
            return nullptr;
        }));

    const std::string serviceUri = "ui_extension_record_timeout";
    AbilityRequest firstRequest;
    firstRequest.requestCode = 1;
    connectManager->EnqueueStartServiceReq(firstRequest, serviceUri);
    ASSERT_EQ(timeoutTasks.size(), 1);
    auto requestQueue = connectManager->uiExtensionForegroundRequestMap_[serviceUri];
    ASSERT_NE(requestQueue, nullptr);
    ASSERT_EQ(requestQueue->requests.size(), 1);

    AbilityRequest laterRequest;
    laterRequest.requestCode = 2;
    connectManager->EnqueueStartServiceReq(laterRequest, serviceUri);
    ASSERT_EQ(requestQueue->requests.size(), 2);

    auto firstTimeoutTask = timeoutTasks.front();
    firstTimeoutTask();

    EXPECT_EQ(connectManager->uiExtensionForegroundRequestMap_.count(serviceUri), 0);
    EXPECT_EQ(timeoutTasks.size(), 1);
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: RemoveUIExtensionForegroundRequest
 * CaseDescription: Verify lifecycle cancellation retains a request queued after its cutoff time
 */
HWTEST_F(UIExtensionAbilityManagerThirdTest, UIExtensionForegroundRequest_003, TestSize.Level1)
{
    auto connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    auto taskHandler = AppExecFwk::MockTaskHandlerWrap::CreateQueueHandler("UIExtensionForegroundRequest_003");
    ASSERT_NE(taskHandler, nullptr);
    connectManager->SetTaskHandler(taskHandler);
    EXPECT_CALL(*taskHandler, SubmitTaskInner(testing::_, testing::_))
        .Times(testing::AnyNumber()).WillRepeatedly(testing::Return(nullptr));

    const std::string serviceUri = "ui_extension_record_remove";
    AbilityRequest previousRequest;
    previousRequest.requestCode = 1;
    connectManager->EnqueueStartServiceReq(previousRequest, serviceUri);

    AbilityRequest laterRequest;
    laterRequest.requestCode = 2;
    connectManager->EnqueueStartServiceReq(laterRequest, serviceUri);
    auto requestQueue = connectManager->uiExtensionForegroundRequestMap_[serviceUri];
    ASSERT_NE(requestQueue, nullptr);
    ASSERT_EQ(requestQueue->requests.size(), 2);

    struct timespec now = {};
    ASSERT_EQ(clock_gettime(CLOCK_MONOTONIC, &now), 0);
    // Model an enqueue that completed after the lifecycle IPC sampled its cutoff time.
    requestQueue->requests.back().enqueueTime = now;
    requestQueue->requests.back().enqueueTime.tv_sec += 3600;

    connectManager->RemoveUIExtensionForegroundRequest(serviceUri, now, true);

    auto requestIt = connectManager->uiExtensionForegroundRequestMap_.find(serviceUri);
    ASSERT_NE(requestIt, connectManager->uiExtensionForegroundRequestMap_.end());
    ASSERT_EQ(requestIt->second, requestQueue);
    ASSERT_EQ(requestIt->second->requests.size(), 1);
    EXPECT_EQ(requestIt->second->requests.front().abilityRequest.requestCode, laterRequest.requestCode);
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: CompleteStartServiceReq
 * CaseDescription: Verify generic service lifecycle entry dispatches to the UI extension request queue
 */
HWTEST_F(UIExtensionAbilityManagerThirdTest, UIExtensionForegroundRequest_004, TestSize.Level1)
{
    auto uiExtensionManager = std::make_shared<MockUIExtensionAbilityManager>(0);
    ASSERT_NE(uiExtensionManager, nullptr);
    auto taskHandler = AppExecFwk::MockTaskHandlerWrap::CreateQueueHandler("UIExtensionForegroundRequest_004");
    ASSERT_NE(taskHandler, nullptr);
    uiExtensionManager->SetTaskHandler(taskHandler);
    EXPECT_CALL(*taskHandler, SubmitTaskInner(testing::_, testing::_))
        .Times(testing::AnyNumber()).WillRepeatedly(testing::Return(nullptr));

    AbilityRequest abilityRequest;
    abilityRequest.requestCode = 1;
    abilityRequest.want.SetElementName("device", "bundle", "ability", "module");
    const std::string serviceUri = abilityRequest.want.GetElement().GetURI();
    std::shared_ptr<AbilityConnectManager> connectManager = uiExtensionManager;
    connectManager->EnqueueStartServiceReq(abilityRequest);
    ASSERT_EQ(uiExtensionManager->uiExtensionForegroundRequestMap_.count(serviceUri), 1);
    EXPECT_EQ(uiExtensionManager->uiExtensionForegroundRequestMap_.count(""), 0);
    EXPECT_EQ(uiExtensionManager->startServiceReqList_.count(serviceUri), 0);

    connectManager->CompleteStartServiceReq(serviceUri);

    EXPECT_EQ(uiExtensionManager->uiExtensionForegroundRequestMap_.count(serviceUri), 0);
    ASSERT_EQ(uiExtensionManager->completedRequestCodes_.size(), 1);
    EXPECT_EQ(uiExtensionManager->completedRequestCodes_.front(), abilityRequest.requestCode);
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: EnqueueStartServiceReq
 * CaseDescription: Verify an unavailable task handler keeps a foreground request for lifecycle completion
 */
HWTEST_F(UIExtensionAbilityManagerThirdTest, UIExtensionForegroundRequest_005, TestSize.Level1)
{
    auto uiExtensionManager = std::make_shared<MockUIExtensionAbilityManager>(0);
    ASSERT_NE(uiExtensionManager, nullptr);
    AbilityRequest abilityRequest;
    abilityRequest.requestCode = 1;
    const std::string serviceUri = "ui_extension_record_without_task_handler";

    uiExtensionManager->EnqueueStartServiceReq(abilityRequest, serviceUri);

    ASSERT_EQ(uiExtensionManager->uiExtensionForegroundRequestMap_.count(serviceUri), 1);
    uiExtensionManager->CompleteStartServiceReq(serviceUri);

    EXPECT_EQ(uiExtensionManager->uiExtensionForegroundRequestMap_.count(serviceUri), 0);
    ASSERT_EQ(uiExtensionManager->completedRequestCodes_.size(), 1);
    EXPECT_EQ(uiExtensionManager->completedRequestCodes_.front(), abilityRequest.requestCode);
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: RemoveUIExtensionForegroundRequest
 * CaseDescription: Verify lifecycle cancellation removes all requests when timestamp ordering is unavailable
 */
HWTEST_F(UIExtensionAbilityManagerThirdTest, UIExtensionForegroundRequest_006, TestSize.Level1)
{
    auto connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    auto taskHandler = AppExecFwk::MockTaskHandlerWrap::CreateQueueHandler("UIExtensionForegroundRequest_006");
    ASSERT_NE(taskHandler, nullptr);
    connectManager->SetTaskHandler(taskHandler);
    EXPECT_CALL(*taskHandler, SubmitTaskInner(testing::_, testing::_))
        .Times(testing::AnyNumber()).WillRepeatedly(testing::Return(nullptr));

    const std::string serviceUri = "ui_extension_record_without_timestamp";
    AbilityRequest firstRequest;
    firstRequest.requestCode = 1;
    AbilityRequest secondRequest;
    secondRequest.requestCode = 2;
    connectManager->EnqueueStartServiceReq(firstRequest, serviceUri);
    connectManager->EnqueueStartServiceReq(secondRequest, serviceUri);
    ASSERT_EQ(connectManager->uiExtensionForegroundRequestMap_.count(serviceUri), 1);

    struct timespec cutoffTime = { 0, 0 };
    connectManager->RemoveUIExtensionForegroundRequest(serviceUri, cutoffTime, false);

    EXPECT_EQ(connectManager->uiExtensionForegroundRequestMap_.count(serviceUri), 0);
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: CompleteBackground
 * CaseDescription: Verify pending foreground waits for request-done before entering FOREGROUNDING
 */
HWTEST_F(UIExtensionAbilityManagerThirdTest, CompleteBackground_001, TestSize.Level1)
{
    auto connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.extensionAbilityType = AppExecFwk::ExtensionAbilityType::SYS_COMMON_UI;
    abilityRequest.sessionInfo = MockSessionInfo(1);
    auto abilityRecord = BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    ASSERT_NE(abilityRecord, nullptr);
    abilityRecord->SetSessionInfo(abilityRequest.sessionInfo);
    abilityRecord->SetDebugApp(true);
    abilityRecord->SetAbilityState(AbilityState::BACKGROUNDING);
    abilityRecord->SetPendingState(AbilityState::FOREGROUND);

    connectManager->CompleteBackground(abilityRecord);

    EXPECT_EQ(abilityRecord->GetAbilityState(), AbilityState::BACKGROUND);
    EXPECT_EQ(abilityRecord->GetPendingState(), AbilityState::FOREGROUND);
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: RegisterPreloadUIExtensionHostClient
 * CaseDescription: Verify successful registration and unregister cleanup
 */
HWTEST_F(UIExtensionAbilityManagerThirdTest, RegisterPreloadUIExtensionHostClient_006, TestSize.Level1)
{
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    sptr<MockPreloadHostClient> callerToken = new MockPreloadHostClient(true);
    const int32_t callerPid = IPCSkeleton::GetCallingPid();

    int32_t res = connectManager->RegisterPreloadUIExtensionHostClient(callerToken);

    EXPECT_EQ(res, ERR_OK);
    EXPECT_EQ(callerToken->addDeathRecipientCount_, 1);
    EXPECT_EQ(connectManager->preloadUIExtensionHostClientDeathRecipients_.count(callerPid), 1);
    EXPECT_EQ(connectManager->uiExtensionAbilityRecordMgr_->preloadUIExtensionHostClientCallerTokens_.count(
        callerPid), 1);

    res = connectManager->UnRegisterPreloadUIExtensionHostClient(callerPid + 1);
    EXPECT_EQ(res, ERR_OK);
    EXPECT_EQ(callerToken->removeDeathRecipientCount_, 0);
    EXPECT_EQ(connectManager->preloadUIExtensionHostClientDeathRecipients_.count(callerPid), 1);
    EXPECT_EQ(connectManager->uiExtensionAbilityRecordMgr_->preloadUIExtensionHostClientCallerTokens_.count(
        callerPid), 1);

    res = connectManager->UnRegisterPreloadUIExtensionHostClient(callerPid);
    EXPECT_EQ(res, ERR_OK);
    EXPECT_EQ(callerToken->removeDeathRecipientCount_, 1);
    EXPECT_TRUE(connectManager->preloadUIExtensionHostClientDeathRecipients_.empty());
    EXPECT_TRUE(connectManager->uiExtensionAbilityRecordMgr_->preloadUIExtensionHostClientCallerTokens_.empty());
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: RegisterPreloadUIExtensionHostClient
 * CaseDescription: Verify registration rollback when adding a death recipient fails
 */
HWTEST_F(UIExtensionAbilityManagerThirdTest, RegisterPreloadUIExtensionHostClient_007, TestSize.Level1)
{
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    sptr<MockPreloadHostClient> callerToken = new MockPreloadHostClient(false);

    int32_t res = connectManager->RegisterPreloadUIExtensionHostClient(callerToken);

    EXPECT_EQ(res, INNER_ERR);
    EXPECT_EQ(callerToken->addDeathRecipientCount_, 1);
    EXPECT_EQ(callerToken->removeDeathRecipientCount_, 0);
    EXPECT_TRUE(connectManager->preloadUIExtensionHostClientDeathRecipients_.empty());
    EXPECT_TRUE(connectManager->uiExtensionAbilityRecordMgr_->preloadUIExtensionHostClientCallerTokens_.empty());
}

/*
 * Feature: UIExtensionAbilityManager
 * Function: RegisterPreloadUIExtensionHostClient
 * CaseDescription: Verify duplicate registration for the same process is idempotent
 */
HWTEST_F(UIExtensionAbilityManagerThirdTest, RegisterPreloadUIExtensionHostClient_008, TestSize.Level1)
{
    std::shared_ptr<UIExtensionAbilityManager> connectManager = std::make_shared<UIExtensionAbilityManager>(0);
    sptr<MockPreloadHostClient> firstCallerToken = new MockPreloadHostClient(true);
    sptr<MockPreloadHostClient> secondCallerToken = new MockPreloadHostClient(true);
    const int32_t callerPid = IPCSkeleton::GetCallingPid();

    EXPECT_EQ(connectManager->RegisterPreloadUIExtensionHostClient(firstCallerToken), ERR_OK);
    EXPECT_EQ(connectManager->RegisterPreloadUIExtensionHostClient(secondCallerToken), ERR_OK);

    EXPECT_EQ(firstCallerToken->addDeathRecipientCount_, 1);
    EXPECT_EQ(secondCallerToken->addDeathRecipientCount_, 0);
    EXPECT_EQ(connectManager->preloadUIExtensionHostClientDeathRecipients_.size(), 1);
    auto tokenIter = connectManager->uiExtensionAbilityRecordMgr_->preloadUIExtensionHostClientCallerTokens_.find(
        callerPid);
    ASSERT_NE(tokenIter,
        connectManager->uiExtensionAbilityRecordMgr_->preloadUIExtensionHostClientCallerTokens_.end());
    sptr<IRemoteObject> expectedCallerToken = firstCallerToken;
    EXPECT_EQ(tokenIter->second, expectedCallerToken);

    EXPECT_EQ(connectManager->UnRegisterPreloadUIExtensionHostClient(callerPid), ERR_OK);
}
}  // namespace AAFwk
}  // namespace OHOS
