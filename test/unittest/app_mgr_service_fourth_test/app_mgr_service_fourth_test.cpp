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

#define private public
#include "accesstoken_kit.h"
#include "ability_manager_errors.h"
#include "app_mgr_service.h"
#include "app_running_record.h"
#include "mock_ability_token.h"
#include "mock_app_mgr_service_inner.h"
#include "mock_ipc_skeleton.h"
#include "mock_my_status.h"
#include "mock_permission_verification.h"
#include "task_handler_wrap.h"
#undef private
#include "hilog_tag_wrapper.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AAFwk;

namespace OHOS {
namespace AppExecFwk {
namespace {
const std::string BUNDLE_NAME = "com.example.test";
constexpr int UID = 1000;
constexpr pid_t PID = 1000;
constexpr pid_t RSS_UID = 1096;
}

class MockTaskHandlerWrap : public TaskHandlerWrap {
public:
    static std::shared_ptr<MockTaskHandlerWrap> CreateQueueHandler(const std::string &queueName,
        TaskQoS queueQos = TaskQoS::DEFAULT)
    {
        return std::make_shared<MockTaskHandlerWrap>();
    }
    static std::shared_ptr<MockTaskHandlerWrap> GetFfrtHandler()
    {
        return std::make_shared<MockTaskHandlerWrap>();
    }
    MockTaskHandlerWrap() : TaskHandlerWrap("MockTaskHandlerWrap") {}
    virtual ~MockTaskHandlerWrap() {}

    MOCK_METHOD2(SubmitTaskInner,
        std::shared_ptr<InnerTaskHandle>(std::function<void()> &&task, const TaskAttribute &));
    bool CancelTaskInner(const std::shared_ptr<InnerTaskHandle> &taskHandle) override
    {
        return true;
    }
    void WaitTaskInner(const std::shared_ptr<InnerTaskHandle> &taskHandle) override {}
    bool RemoveTask(const std::string &name, const TaskHandle &taskHandle)
    {
        return true;
    }
    std::shared_ptr<InnerTaskHandle> MockTaskHandler(const std::function<void()> &&, const TaskAttribute &);
};

class AppMgrServiceFourthTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    static std::shared_ptr<InnerTaskHandle> IgnoreTask(std::function<void()> task, const TaskAttribute& attr);
    inline static std::shared_ptr<AppMgrService> appMgrService_{ nullptr };
    inline static std::shared_ptr<MockAppMgrServiceInner> mockAppMgrServiceInner_{ nullptr };
    inline static std::shared_ptr<AMSEventHandler> eventHandler_{ nullptr };
    inline static std::shared_ptr<MockTaskHandlerWrap> mockTaskHandler_{ nullptr };
    inline static std::shared_ptr<AppRunningManager> appRunningManager_{ nullptr };
};

void AppMgrServiceFourthTest::SetUpTestCase(void)
{
    appMgrService_ = std::make_shared<AppMgrService>();
    mockAppMgrServiceInner_ = std::make_shared<MockAppMgrServiceInner>();
    appMgrService_->appMgrServiceInner_ = mockAppMgrServiceInner_;
    appMgrService_->OnStart();
    mockTaskHandler_ = MockTaskHandlerWrap::CreateQueueHandler(Constants::APP_MGR_SERVICE_NAME);
    appMgrService_->taskHandler_ = mockTaskHandler_;
    eventHandler_= std::make_shared<AMSEventHandler>(appMgrService_->taskHandler_,
        appMgrService_->appMgrServiceInner_);
    appMgrService_->eventHandler_ = eventHandler_;
    appRunningManager_ = std::make_shared<AppRunningManager>();
    appMgrService_->appMgrServiceInner_->appRunningManager_ = appRunningManager_;
    auto appRecord = std::make_shared<AppRunningRecord>(nullptr, 0, "");
    appRecord->appInfo_ = std::make_shared<ApplicationInfo>();
    appRecord->appInfo_->accessTokenId = IPCSkeleton::GetCallingTokenID();
    appMgrService_->appMgrServiceInner_->appRunningManager_->appRunningRecordMap_.emplace(0, appRecord);
}

void AppMgrServiceFourthTest::TearDownTestCase(void)
{
    if (appRunningManager_) {
        appRunningManager_.reset();
    }

    if (mockAppMgrServiceInner_) {
        mockAppMgrServiceInner_.reset();
    }

    if (mockTaskHandler_) {
        mockTaskHandler_.reset();
    }

    if (eventHandler_) {
        eventHandler_.reset();
    }

    if (appMgrService_) {
        appMgrService_->OnStop();
        int sleepTime = 1;
        sleep(sleepTime);

        if (appMgrService_->appMgrServiceInner_->appRunningManager_) {
            appMgrService_->appMgrServiceInner_->appRunningManager_.reset();
        }

        if (appMgrService_->appMgrServiceInner_) {
            appMgrService_->appMgrServiceInner_.reset();
        }

        if (appMgrService_->taskHandler_) {
            appMgrService_->taskHandler_.reset();
        }

        if (appMgrService_->eventHandler_) {
            appMgrService_->eventHandler_.reset();
        }
        appMgrService_.reset();
    }
}

void AppMgrServiceFourthTest::SetUp() {}

void AppMgrServiceFourthTest::TearDown() {}

std::shared_ptr<InnerTaskHandle> AppMgrServiceFourthTest::IgnoreTask(std::function<void()> task,
    const TaskAttribute& attr)
{
    TAG_LOGI(AAFwkTag::TEST, "SubmitTaskInner called but task not executed");
    return nullptr;
}

/**
 * @tc.name: ApplicationForegrounded_0100
 * @tc.desc: ApplicationForegrounded.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceFourthTest, ApplicationForegrounded_0100, TestSize.Level1)
{
    // return after JudgeAppSelfCalled failed
    TAG_LOGI(AAFwkTag::TEST, "ApplicationForegrounded_0100 start");
    appMgrService_->eventHandler_ = nullptr;
    EXPECT_CALL(*mockTaskHandler_, SubmitTaskInner(_, _)).Times(0).WillOnce(Invoke(IgnoreTask));
    appMgrService_->ApplicationForegrounded(0);
    appMgrService_->eventHandler_ = eventHandler_;
    TAG_LOGI(AAFwkTag::TEST, "ApplicationForegrounded_0100 end");
}

/**
 * @tc.name: ApplicationForegrounded_0200
 * @tc.desc: ApplicationForegrounded.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceFourthTest, ApplicationForegrounded_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ApplicationForegrounded_0200 start");
    EXPECT_CALL(*mockTaskHandler_, SubmitTaskInner(_, _)).Times(1).WillOnce(Invoke(IgnoreTask));
    appMgrService_->ApplicationForegrounded(0);
    TAG_LOGI(AAFwkTag::TEST, "ApplicationForegrounded_0200 end");
}

/**
 * @tc.name: ApplicationBackgrounded_0100
 * @tc.desc: ApplicationBackgrounded.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceFourthTest, ApplicationBackgrounded_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ApplicationBackgrounded_0100 start");
    // return after JudgeAppSelfCalled failed
    appMgrService_->eventHandler_ = nullptr;
    EXPECT_CALL(*mockTaskHandler_, SubmitTaskInner(_, _)).Times(0).WillOnce(Invoke(IgnoreTask));
    appMgrService_->ApplicationBackgrounded(0);
    appMgrService_->eventHandler_ = eventHandler_;
    TAG_LOGI(AAFwkTag::TEST, "ApplicationBackgrounded_0100 end");
}

/**
 * @tc.name: ApplicationBackgrounded_0200
 * @tc.desc: ApplicationBackgrounded.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceFourthTest, ApplicationBackgrounded_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ApplicationBackgrounded_0200 start");
    EXPECT_CALL(*mockTaskHandler_, SubmitTaskInner(_, _)).Times(1).WillOnce(Invoke(IgnoreTask));
    appMgrService_->ApplicationBackgrounded(0);
    TAG_LOGI(AAFwkTag::TEST, "ApplicationBackgrounded_0200 end");
}

/**
 * @tc.name: ApplicationTerminated_0100
 * @tc.desc: ApplicationTerminated.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceFourthTest, ApplicationTerminated_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ApplicationTerminated_0100 start");
    // return after JudgeAppSelfCalled failed
    appMgrService_->eventHandler_ = nullptr;
    EXPECT_CALL(*mockTaskHandler_, SubmitTaskInner(_, _)).Times(0).WillOnce(Invoke(IgnoreTask));
    appMgrService_->ApplicationTerminated(0);
    appMgrService_->eventHandler_ = eventHandler_;
    TAG_LOGI(AAFwkTag::TEST, "ApplicationTerminated_0100 end");
}

/**
 * @tc.name: ApplicationTerminated_0200
 * @tc.desc: ApplicationTerminated.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceFourthTest, ApplicationTerminated_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ApplicationTerminated_0200 start");
    EXPECT_CALL(*mockTaskHandler_, SubmitTaskInner(_, _)).Times(1).WillOnce(Invoke(IgnoreTask));
    appMgrService_->ApplicationTerminated(0);
    TAG_LOGI(AAFwkTag::TEST, "ApplicationTerminated_0200 end");
}

/**
 * @tc.name: StartupResidentProcess_0100
 * @tc.desc: StartupResidentProcess.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceFourthTest, StartupResidentProcess_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartupResidentProcess_0100 start");
    IPCSkeleton::SetCallingPid(getprocpid());
    EXPECT_CALL(*mockTaskHandler_, SubmitTaskInner(_, _)).Times(1).WillOnce(Invoke(IgnoreTask));
    std::vector<AppExecFwk::BundleInfo> bundleInfos;
    appMgrService_->StartupResidentProcess(bundleInfos);
    TAG_LOGI(AAFwkTag::TEST, "StartupResidentProcess_0100 end");
}

/**
 * @tc.name: GetAmsMgr_0100
 * @tc.desc: GetAmsMgr.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceFourthTest, GetAmsMgr_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetAmsMgr_0100 start");
    auto amsMgr = appMgrService_->GetAmsMgr();
    EXPECT_EQ(amsMgr, appMgrService_->amsMgrScheduler_);
    TAG_LOGI(AAFwkTag::TEST, "GetAmsMgr_0100 end");
}

/**
 * @tc.name: NotifyProcMemoryLevel_0100
 * @tc.desc: NotifyProcMemoryLevel.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceFourthTest, NotifyProcMemoryLevel_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "NotifyProcMemoryLevel_0100 start");
    appMgrService_->eventHandler_ = nullptr;
    std::map<pid_t, MemoryLevel> procLevelMap;
    auto result = appMgrService_->NotifyProcMemoryLevel(procLevelMap);
    EXPECT_EQ(result, ERR_INVALID_OPERATION);
    appMgrService_->eventHandler_ = eventHandler_;
    TAG_LOGI(AAFwkTag::TEST, "NotifyProcMemoryLevel_0100 end");
}

HWTEST_F(AppMgrServiceFourthTest, HasDumpPermission_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "HasDumpPermission_0100 start");
    AAFwk::MyStatus::GetInstance().isVerifyAccessToken_ = Security::AccessToken::PermissionState::PERMISSION_GRANTED;
    AAFwk::MyStatus::GetInstance().isVerifyAccessToken_ += 1;
    auto result = appMgrService_->HasDumpPermission();
    EXPECT_EQ(result, false);
    TAG_LOGI(AAFwkTag::TEST, "HasDumpPermission_0100 end");
}

/**
 * @tc.name: AddAbilityStageDone_0100
 * @tc.desc: AddAbilityStageDone.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceFourthTest, AddAbilityStageDone_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AddAbilityStageDone_0100 start");
    EXPECT_CALL(*mockTaskHandler_, SubmitTaskInner(_, _)).Times(1).WillOnce(Invoke(IgnoreTask));
    appMgrService_->AddAbilityStageDone(0);
    TAG_LOGI(AAFwkTag::TEST, "AddAbilityStageDone_0100 end");
}

/**
 * @tc.name: StartUserTestProcess_0100
 * @tc.desc: StartUserTestProcess.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceFourthTest, StartUserTestProcess_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartUserTestProcess_0100 start");
    AAFwk::MyStatus::GetInstance().isShellCall_ = false;
    AAFwk::Want want;
    sptr<IRemoteObject> observer;
    AppExecFwk::BundleInfo bundleInfo;
    int32_t result = appMgrService_->StartUserTestProcess(want, observer, bundleInfo, 0);
    EXPECT_EQ(result, ERR_INVALID_OPERATION);
    TAG_LOGI(AAFwkTag::TEST, "StartUserTestProcess_0100 end");
}

/**
 * @tc.name: GetAllAbilityInfos_001
 * @tc.desc: verify GetAllAbilityInfos works when inner service is null
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceFourthTest, GetAllAbilityInfos_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetAllAbilityInfos_001 start");
    appMgrService_->appMgrServiceInner_ = nullptr;
    AAFwk::MyStatus::GetInstance().isCheckSpecificSystemAbilityAccessPermission_ = true;
    IPCSkeleton::SetCallingUid(RSS_UID);
    std::vector<AppExecFwk::AbilityStateData> infos;
    int32_t result = appMgrService_->GetAllAbilityInfos(-1, infos);
    EXPECT_EQ(result, AAFwk::ERR_APP_MGR_SERVICE_NOT_READY);
    AAFwk::MyStatus::GetInstance().isCheckSpecificSystemAbilityAccessPermission_ = false;
    appMgrService_->appMgrServiceInner_ = mockAppMgrServiceInner_;
    TAG_LOGI(AAFwkTag::TEST, "GetAllAbilityInfos_001 end");
}

/**
 * @tc.name: GetAllAbilityInfos_002
 * @tc.desc: verify GetAllAbilityInfos works when permission denied
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceFourthTest, GetAllAbilityInfos_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetAllAbilityInfos_002 start");
    std::vector<AppExecFwk::AbilityStateData> infos;
    int32_t result = appMgrService_->GetAllAbilityInfos(-1, infos);
    EXPECT_EQ(result, ERR_PERMISSION_DENIED);
    TAG_LOGI(AAFwkTag::TEST, "GetAllAbilityInfos_002 end");
}
} // namespace AppExecFwk
} // namespace OHOS