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

#include <gtest/gtest.h>
#include <memory>
#include <limits>
#include <future>
#include <chrono>
#include <algorithm>
#include <shared_mutex>
#include <unordered_map>
#define private public
#include "app_mgr_service_inner.h"
#include "app_pidfd_manager.h"
#include "launch_service_manager.h"
#undef private
#include "ability_manager_errors.h"
#include "launch_service_operation_callback.h"
#include "mock/launch_service_iam_mock.h"
#include "mock/launch_service_bundle_mock.h"
using namespace testing::ext;
namespace OHOS::AppExecFwk {
class LaunchServiceInnerTest : public testing::Test {
public:
    void SetUp() override
    {
        UserIam::UserAuth::LaunchServiceIAMMock::Instance().Reset();
        LaunchServiceBundleMock::Instance().Reset();
        inner_->SetTaskHandler(AAFwk::TaskHandlerWrap::CreateConcurrentQueueHandler(
            "launch_service_test_queue", 1, AAFwk::TaskQoS::USER_INTERACTIVE));
        ASSERT_NE(inner_->GetTaskHandler(), nullptr);
        launchHandler_ = AAFwk::TaskHandlerWrap::CreateQueueHandler("launch_inner_dedicated_test");
        ASSERT_NE(launchHandler_, nullptr);
        inner_->SetLaunchServiceTaskHandler(launchHandler_);
    }
    void TearDown() override
    {
        UserIam::UserAuth::LaunchServiceIAMMock::Instance().Reset();
    }
    std::shared_ptr<AAFwk::TaskHandlerWrap> launchHandler_;
    std::shared_ptr<AppMgrServiceInner> inner_ = std::make_shared<AppMgrServiceInner>();
};

// This test inspects manager records and must link the manager implementation locally.
class LaunchServicePidfdRecordTest : public testing::Test {
public:
    void SetUp() override
    {
        manager_.Init({}, {});
    }
    void TearDown() override
    {
        while (!manager_.pidfdMap_.empty()) {
            manager_.OnPidfdFired(manager_.pidfdMap_.begin()->first, PidFdType::LAUNCH_SERVICE);
        }
        manager_.Init({}, {});
    }
    AppPidFdManager &manager_ = AppPidFdManager::GetInstance();
};

/**
 * @tc.name: AddWatcher_ExitedProcessDispatchesCleanup_001
 * @tc.desc: An already exited process dispatches cleanup and clears the recorded pid.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServicePidfdRecordTest, AddWatcher_ExitedProcessDispatchesCleanup_001, TestSize.Level1)
{
    auto inner = std::make_shared<AppMgrServiceInner>();
    auto handler = AAFwk::TaskHandlerWrap::CreateQueueHandler("launch_watch_failure_test");
    ASSERT_NE(handler, nullptr);
    inner->SetLaunchServiceTaskHandler(handler);
    manager_.Init(inner, {});
    auto launchManager = inner->GetLaunchServiceManager();
    LaunchServiceManager::LaunchServiceKey key {"com.example.daemon", "entry", "worker", 100, 0};
    const pid_t pid = std::numeric_limits<pid_t>::max();
    launchManager->launchServiceRuntimeRecords_[key].pid = pid;
    launchManager->launchServiceRuntimeRecords_[key].generation = 1;
    EXPECT_TRUE(manager_.AddWatcher(pid, PidFdType::LAUNCH_SERVICE));
    auto completed = std::make_shared<std::promise<void>>();
    auto future = completed->get_future();
    auto task = handler->SubmitTask([completed]() { completed->set_value(); });
    ASSERT_TRUE(task);
    ASSERT_EQ(future.wait_for(std::chrono::seconds(3)), std::future_status::ready);
    ASSERT_EQ(launchManager->launchServiceRuntimeRecords_.count(key), 1u);
    EXPECT_EQ(launchManager->launchServiceRuntimeRecords_.at(key).pid, 0);
    EXPECT_EQ(launchManager->launchServiceRuntimeRecords_.at(key).generation, 1u);
}

/**
 * @tc.name: ManagerAndBoot_001
 * @tc.desc: Manager is reused and boot launches query IAM only once.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceInnerTest, ManagerAndBoot_001, TestSize.Level1)
{
    auto first = inner_->GetLaunchServiceManager();
    ASSERT_NE(first, nullptr);
    EXPECT_EQ(first, inner_->GetLaunchServiceManager());
    inner_->StartLaunchServicesAfterBoot();
    inner_->StartLaunchServicesAfterBoot();
    EXPECT_EQ(UserIam::UserAuth::LaunchServiceIAMMock::Instance().listCalls, 1);
}

/**
 * @tc.name: RegisterDaemon_001
 * @tc.desc: Forward invalid identity to the manager and preserve its result.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceInnerTest, RegisterDaemon_001, TestSize.Level1)
{
    AAFwk::LaunchServiceDescriptor descriptor;
    EXPECT_EQ(inner_->RegisterDaemon(descriptor, 20000001), ERR_INVALID_VALUE);
}

/**
 * @tc.name: UnregisterDaemon_001
 * @tc.desc: Forward invalid identity to the manager and preserve its result.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceInnerTest, UnregisterDaemon_001, TestSize.Level1)
{
    AAFwk::LaunchServiceDescriptor descriptor;
    EXPECT_EQ(inner_->UnregisterDaemon(descriptor, 20000001), ERR_INVALID_VALUE);
}

/**
 * @tc.name: StartDaemon_001
 * @tc.desc: Forward invalid identity to the manager and preserve its result.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceInnerTest, StartDaemon_001, TestSize.Level1)
{
    AAFwk::LaunchServiceDescriptor descriptor;
    EXPECT_EQ(inner_->StartDaemon(descriptor, 20000001), ERR_INVALID_VALUE);
}

/**
 * @tc.name: StopDaemon_001
 * @tc.desc: Forward invalid identity to the manager and preserve its result.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceInnerTest, StopDaemon_001, TestSize.Level1)
{
    AAFwk::LaunchServiceDescriptor descriptor;
    EXPECT_EQ(inner_->StopDaemon(descriptor, 20000001), ERR_INVALID_VALUE);
}

/**
 * @tc.name: GetInformation_001
 * @tc.desc: Forward query failures and list success without replacing the manager.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceInnerTest, GetInformation_001, TestSize.Level1)
{
    AbilityRuntime::LaunchServiceDescriptor descriptor;
    AbilityRuntime::LaunchServiceInformation information;
    LaunchServiceBundleMock::Instance().result = ERR_INVALID_VALUE;
    EXPECT_EQ(inner_->GetLaunchServiceInformation(descriptor, information, 20000001),
        AAFwk::ERR_LAUNCH_SERVICE_NOT_FOUND);
    std::vector<AbilityRuntime::LaunchServiceInformation> list(1);
    EXPECT_EQ(inner_->GetAllRegisteredInformation(list, 100, 100), ERR_OK);
    EXPECT_TRUE(list.empty());
}

/**
 * @tc.name: SetEnabled_001
 * @tc.desc: Preserve asynchronous approval and callback errors across the inner layer.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceInnerTest, SetEnabled_001, TestSize.Level1)
{
    auto result = std::make_shared<int32_t>(ERR_OK);
    auto callback = [result](int32_t value) { *result = value; };
    inner_->SetLaunchServicesEnabled("com.example.daemon", 100, 0, {}, true, 20000001, callback);
    auto &launchServiceMock = UserIam::UserAuth::LaunchServiceIAMMock::Instance();
    ASSERT_NE(launchServiceMock.pending, nullptr);
    launchServiceMock.pending->OnResult(UserIam::UserAuth::CANCELED);
    EXPECT_EQ(*result, AAFwk::ERR_LAUNCH_SERVICE_APPROVAL_REJECTED);
    launchServiceMock.pending.reset();
}

/**
 * @tc.name: Observer_001
 * @tc.desc: Reject null observers through the inner forwarding layer.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceInnerTest, Observer_001, TestSize.Level1)
{
    EXPECT_EQ(inner_->RegisterLaunchServiceObserver(nullptr, 20000001), ERR_INVALID_VALUE);
    EXPECT_EQ(inner_->UnregisterLaunchServiceObserver(nullptr), ERR_INVALID_VALUE);
}

/**
 * @tc.name: ProcessExited_001
 * @tc.desc: Process exit clears PID on the same manager runtime record.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceInnerTest, ProcessExited_001, TestSize.Level1)
{
    auto manager = inner_->GetLaunchServiceManager();
    LaunchServiceManager::LaunchServiceKey key {"com.example.daemon", "entry", "worker", 100, 0};
    manager->launchServiceRuntimeRecords_[key].pid = 123;
    manager->launchServiceRuntimeRecords_[key].generation = 1;
    inner_->OnLaunchServiceProcessExited(123);
    EXPECT_EQ(manager->launchServiceRuntimeRecords_.at(key).pid, 0);
    EXPECT_EQ(manager->launchServiceRuntimeRecords_.at(key).generation, 1u);
}

/**
 * @tc.name: DispatchLaunchServiceProcessExitCleanup_001
 * @tc.desc: Exit cleanup uses the dedicated queue even when the general queue is absent.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceInnerTest, DispatchLaunchServiceProcessExitCleanup_001, TestSize.Level1)
{
    inner_->SetTaskHandler(nullptr);
    auto manager = inner_->GetLaunchServiceManager();
    LaunchServiceManager::LaunchServiceKey key {"com.example.daemon", "entry", "worker", 100, 0};
    manager->launchServiceRuntimeRecords_[key].pid = 123;
    manager->launchServiceRuntimeRecords_[key].generation = 1;
    inner_->DispatchLaunchServiceProcessExitCleanup(123);
    auto completed = std::make_shared<std::promise<void>>();
    auto future = completed->get_future();
    auto handle = launchHandler_->SubmitTask([completed]() { completed->set_value(); });
    ASSERT_TRUE(handle);
    ASSERT_EQ(future.wait_for(std::chrono::seconds(3)), std::future_status::ready);
    EXPECT_EQ(manager->launchServiceRuntimeRecords_.at(key).pid, 0);
    EXPECT_EQ(manager->launchServiceRuntimeRecords_.at(key).generation, 1u);
}

/**
 * @tc.name: DispatchLaunchServiceProcessExitCleanup_NoHandler_001
 * @tc.desc: Missing dedicated queue does not fall back to the general queue.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceInnerTest, DispatchLaunchServiceProcessExitCleanup_NoHandler_001, TestSize.Level1)
{
    inner_->SetLaunchServiceTaskHandler(nullptr);
    auto manager = inner_->GetLaunchServiceManager();
    LaunchServiceManager::LaunchServiceKey key {"com.example.daemon", "entry", "worker", 100, 0};
    manager->launchServiceRuntimeRecords_[key].pid = 123;
    inner_->DispatchLaunchServiceProcessExitCleanup(123);
    EXPECT_EQ(manager->launchServiceRuntimeRecords_.at(key).pid, 123);
}

/**
 * @tc.name: SetAppPermissionInfo_001
 * @tc.desc: Daemon identity includes its app UID and owning GID once; ordinary processes keep their app UID.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceInnerTest, SetAppPermissionInfo_001, TestSize.Level1)
{
    BundleInfo bundle;
    bundle.uid = 20000001;
    bundle.gid = 20000002;
    AppSpawnStartMsg daemon {};
    daemon.code = MSG_SPAWN_SERVICE_BIN;
    inner_->SetAppPermissionInfo(bundle, daemon);
    EXPECT_EQ(daemon.appUid, bundle.uid);
    EXPECT_EQ(daemon.allowInternet, 0);
    EXPECT_EQ(daemon.setAllowInternet, 1);
    EXPECT_EQ(std::count(daemon.gids.begin(), daemon.gids.end(), bundle.gid), 1);
    inner_->SetAppPermissionInfo(bundle, daemon);
    EXPECT_EQ(std::count(daemon.gids.begin(), daemon.gids.end(), bundle.gid), 1);
    LaunchServiceBundleMock::Instance().permissionGranted = true;
    LaunchServiceBundleMock::Instance().networkResult = ERR_INVALID_VALUE;
    inner_->SetAppPermissionInfo(bundle, daemon);
    EXPECT_EQ(daemon.allowInternet, 1);
    EXPECT_EQ(daemon.setAllowInternet, 0);
    AppSpawnStartMsg ordinary {};
    ordinary.appUid = 123;
    inner_->SetAppPermissionInfo(bundle, ordinary);
    EXPECT_EQ(ordinary.appUid, 123);
}
} // namespace OHOS::AppExecFwk
