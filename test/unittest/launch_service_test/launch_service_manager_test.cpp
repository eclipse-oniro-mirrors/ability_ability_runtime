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
#include <chrono>
#include <functional>
#include <memory>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <vector>
#include "app_mgr_service_inner.h"
#include "launch_service_observer.h"
#include "task_handler_wrap.h"
#define private public
#include "launch_service_manager.h"
#undef private
#include "ability_manager_errors.h"
#include "errors.h"

using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
class LaunchServiceManagerTest : public testing::Test {
public:
    std::shared_ptr<LaunchServiceManager> manager_ =
        std::make_shared<LaunchServiceManager>(std::weak_ptr<AppMgrServiceInner>{});
    LaunchServiceManager::LaunchServiceKey key_{"com.example.daemon", "entry", "worker", 100, 0};
};

/**
 * @tc.name: StartLaunchService_001
 * @tc.desc: Verify a missing owner prevents launch service startup.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceManagerTest, StartLaunchService_001, TestSize.Level1)
{
    LaunchService service;
    EXPECT_EQ(manager_->StartLaunchService(key_, service), AAFwk::ABILITY_SERVICE_NOT_CONNECTED);
}

/**
 * @tc.name: StartLaunchServiceLocked_001
 * @tc.desc: Verify missing or stale generations do not spawn a process.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceManagerTest, StartLaunchServiceLocked_001, TestSize.Level1)
{
    LaunchService service;
    EXPECT_EQ(manager_->StartLaunchServiceLocked(key_, service, 0, 1), ERR_OK);
    LaunchServiceManager::LaunchServiceRuntimeRecord record;
    record.generation = 2;
    record.service.keepAlive = true;
    manager_->launchServiceRuntimeRecords_[key_] = record;
    EXPECT_EQ(manager_->StartLaunchServiceLocked(key_, service, 0, 1), ERR_OK);
    EXPECT_EQ(manager_->launchServiceRuntimeRecords_.at(key_).generation, 2);
}

/**
 * @tc.name: HandleLaunchServiceProcessDied_001
 * @tc.desc: Verify a non-keepalive service does not schedule a restart after exit.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceManagerTest, HandleLaunchServiceProcessDied_001, TestSize.Level1)
{
    LaunchServiceManager::LaunchServiceRuntimeRecord record;
    record.pid = 123;
    record.generation = 1;
    record.service.keepAlive = false;
    manager_->launchServiceRuntimeRecords_[key_] = record;
    manager_->HandleLaunchServiceProcessDied(123);
    EXPECT_EQ(manager_->launchServiceRuntimeRecords_.at(key_).pid, 0);
    EXPECT_EQ(manager_->launchServiceRuntimeRecords_.at(key_).restartCount, 0);
}

/**
 * @tc.name: HandleLaunchServiceProcessDied_002
 * @tc.desc: Verify repeated exit notifications increment the restart count only once.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceManagerTest, HandleLaunchServiceProcessDied_002, TestSize.Level1)
{
    LaunchServiceManager::LaunchServiceRuntimeRecord record;
    record.pid = 123;
    record.generation = 1;
    record.service.keepAlive = true;
    manager_->launchServiceRuntimeRecords_[key_] = record;
    manager_->HandleLaunchServiceProcessDied(123);
    manager_->HandleLaunchServiceProcessDied(123);
    EXPECT_EQ(manager_->launchServiceRuntimeRecords_.at(key_).pid, 0);
    EXPECT_EQ(manager_->launchServiceRuntimeRecords_.at(key_).restartCount, 1);
}

/**
 * @tc.name: HandleLaunchServiceProcessDied_003
 * @tc.desc: Verify generation zero prevents a keepalive restart.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceManagerTest, HandleLaunchServiceProcessDied_003, TestSize.Level1)
{
    LaunchServiceManager::LaunchServiceRuntimeRecord record;
    record.pid = 123;
    record.generation = 0;
    record.service.keepAlive = true;
    manager_->launchServiceRuntimeRecords_[key_] = record;
    manager_->HandleLaunchServiceProcessDied(123);
    EXPECT_EQ(manager_->launchServiceRuntimeRecords_.at(key_).restartCount, 0);
}

/**
 * @tc.name: StopLaunchServices_001
 * @tc.desc: Verify stopping the original app preserves records for another app index.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceManagerTest, StopLaunchServices_001, TestSize.Level1)
{
    LaunchServiceManager::LaunchServiceRuntimeRecord record;
    record.pid = 0;
    record.generation = 1;
    manager_->launchServiceRuntimeRecords_[key_] = record;
    auto other = key_;
    other.appIndex = 1000;
    manager_->launchServiceRuntimeRecords_[other] = record;
    manager_->StopLaunchServices(key_.bundleName, key_.userId, key_.appIndex);
    EXPECT_EQ(manager_->launchServiceRuntimeRecords_.count(key_), 0);
    EXPECT_EQ(manager_->launchServiceRuntimeRecords_.count(other), 1);
}

/**
 * @tc.name: StopLaunchService_001
 * @tc.desc: Verify a mismatched expected PID preserves the current process record.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceManagerTest, StopLaunchService_001, TestSize.Level1)
{
    LaunchServiceManager::LaunchServiceRuntimeRecord record;
    record.pid = 123;
    manager_->launchServiceRuntimeRecords_[key_] = record;
    EXPECT_EQ(manager_->StopLaunchService(key_, 456), AAFwk::ERR_LAUNCH_SERVICE_NOT_RUNNING);
    EXPECT_EQ(manager_->launchServiceRuntimeRecords_.at(key_).pid, 123);
}
} // namespace AppExecFwk
} // namespace OHOS
