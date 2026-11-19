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
#include <csignal>
#include <functional>
#include <future>
#include <memory>
#include <cerrno>
#include <fcntl.h>
#include <poll.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include <unistd.h>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <vector>
#define private public
#include "app_mgr_service_inner.h"
#undef private
#include "remote_client_manager.h"
#include "app_spawn_client.h"
#include "launch_service_observer.h"
#include "task_handler_wrap.h"
#define private public
#include "launch_service_manager.h"
#include "app_pidfd_manager.h"
#undef private
#include "ability_manager_errors.h"
#include "errors.h"
#include "mock/launch_service_iam_mock.h"
#include "mock/launch_service_bundle_mock.h"

using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
class ControlledDaemonSpawnClient : public AppSpawnClient {
public:
    int32_t result = ERR_OK;
    int32_t outputPid = 123;
    int32_t StartProcess(const AppSpawnStartMsg &, pid_t &pid) override
    {
        pid = outputPid;
        return result;
    }
};
class ManagerRecordingObserver : public AbilityRuntime::LaunchServiceObserverStub {
public:
    int32_t statusCalls = 0;
    int32_t processCalls = 0;
    AbilityRuntime::LaunchServiceStateData status;
    AbilityRuntime::LaunchServiceProcessStateData process;
    void OnLaunchServiceStatusChanged(const AbilityRuntime::LaunchServiceStateData &value) override
    {
        status = value;
        ++statusCalls;
    }
    void OnLaunchServiceProcessStateChanged(const AbilityRuntime::LaunchServiceProcessStateData &value) override
    {
        process = value;
        ++processCalls;
    }
};
class LaunchServiceManagerTest : public testing::Test {
public:
    void SetUp() override
    {
        UserIam::UserAuth::LaunchServiceIAMMock::Instance().Reset();
        LaunchServiceBundleMock::Instance().Reset();
    }
    void TearDown() override { UserIam::UserAuth::LaunchServiceIAMMock::Instance().Reset(); }
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
/**
 * @tc.name: StopLaunchService_WithoutWatcher_001
 * @tc.desc: Explicit stop opens a temporary pidfd when no watcher is registered.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceManagerTest, StopLaunchService_WithoutWatcher_001, TestSize.Level1)
{
    const pid_t child = fork();
    ASSERT_GE(child, 0);
    if (child == 0) {
        for (;;) {
            pause();
        }
    }
    const int fd = static_cast<int>(syscall(SYS_pidfd_open, child, 0));
    if (fd < 0) {
        kill(child, SIGKILL);
        while (waitpid(child, nullptr, 0) < 0 && errno == EINTR) {}
        FAIL() << "pidfd_open failed for test child";
    }
    auto &record = manager_->launchServiceRuntimeRecords_[key_];
    record.pid = child;
    EXPECT_EQ(manager_->StopLaunchService(key_), ERR_OK);
    pollfd event {fd, POLLIN, 0};
    const int result = poll(&event, 1, 3000);
    EXPECT_EQ(result, 1);
    EXPECT_NE(event.revents & POLLIN, 0);
    if (result <= 0) {
        kill(child, SIGKILL);
    }
    while (waitpid(child, nullptr, 0) < 0 && errno == EINTR) {}
    close(fd);
    EXPECT_EQ(manager_->launchServiceRuntimeRecords_.count(key_), 0u);
}

/**
 * @tc.name: RegisterDaemon_InvalidIdentity_001
 * @tc.desc: Verify invalid identity is rejected before contacting external services.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceManagerTest, RegisterDaemon_InvalidIdentity_001, TestSize.Level1)
{
    AAFwk::LaunchServiceDescriptor descriptor;
    EXPECT_EQ(manager_->RegisterDaemon(descriptor, 20000000), ERR_INVALID_VALUE);
    descriptor.bundleName = "com.example.daemon";
    descriptor.moduleName = "entry";
    descriptor.serviceName = "worker";
    descriptor.appIndex = -1;
    EXPECT_EQ(manager_->RegisterDaemon(descriptor, 20000000), ERR_INVALID_VALUE);
}

/**
 * @tc.name: UnregisterDaemon_InvalidIdentity_001
 * @tc.desc: Verify invalid identity is rejected before contacting external services.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceManagerTest, UnregisterDaemon_InvalidIdentity_001, TestSize.Level1)
{
    AAFwk::LaunchServiceDescriptor descriptor;
    EXPECT_EQ(manager_->UnregisterDaemon(descriptor, 20000000), ERR_INVALID_VALUE);
    descriptor.bundleName = "com.example.daemon";
    descriptor.moduleName = "entry";
    descriptor.serviceName = "worker";
    descriptor.appIndex = -1;
    EXPECT_EQ(manager_->UnregisterDaemon(descriptor, 20000000), ERR_INVALID_VALUE);
}

/**
 * @tc.name: StartDaemon_InvalidIdentity_001
 * @tc.desc: Verify invalid identity is rejected before contacting external services.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceManagerTest, StartDaemon_InvalidIdentity_001, TestSize.Level1)
{
    AAFwk::LaunchServiceDescriptor descriptor;
    EXPECT_EQ(manager_->StartDaemon(descriptor, 20000000), ERR_INVALID_VALUE);
    descriptor.bundleName = "com.example.daemon";
    descriptor.moduleName = "entry";
    descriptor.serviceName = "worker";
    descriptor.appIndex = -1;
    EXPECT_EQ(manager_->StartDaemon(descriptor, 20000000), ERR_INVALID_VALUE);
}

/**
 * @tc.name: StopDaemon_InvalidIdentity_001
 * @tc.desc: Verify invalid identity is rejected before contacting external services.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceManagerTest, StopDaemon_InvalidIdentity_001, TestSize.Level1)
{
    AAFwk::LaunchServiceDescriptor descriptor;
    EXPECT_EQ(manager_->StopDaemon(descriptor, 20000000), ERR_INVALID_VALUE);
    descriptor.bundleName = "com.example.daemon";
    descriptor.moduleName = "entry";
    descriptor.serviceName = "worker";
    descriptor.appIndex = -1;
    EXPECT_EQ(manager_->StopDaemon(descriptor, 20000000), ERR_INVALID_VALUE);
}

/**
 * @tc.name: GetAllRegisteredInformation_IAM_001
 * @tc.desc: Verify IAM success clears stale output and IAM failure is propagated.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceManagerTest, GetAllRegisteredInformation_IAM_001, TestSize.Level1)
{
    auto &launchServiceMock = UserIam::UserAuth::LaunchServiceIAMMock::Instance();
    std::vector<AbilityRuntime::LaunchServiceInformation> output(1);
    EXPECT_EQ(manager_->GetAllRegisteredInformation(output, 100, 100), ERR_OK);
    EXPECT_TRUE(output.empty());
    EXPECT_EQ(launchServiceMock.listCalls, 1);
    launchServiceMock.result = UserIam::UserAuth::GENERAL_ERROR;
    EXPECT_EQ(manager_->GetAllRegisteredInformation(output, 100, 100), AAFwk::ABILITY_SERVICE_NOT_CONNECTED);
    EXPECT_EQ(launchServiceMock.listCalls, 2);
}

/**
 * @tc.name: SpawnDaemonProcess_NoOwner_001
 * @tc.desc: Verify unavailable owner resets outputs and prevents spawn.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceManagerTest, SpawnDaemonProcess_NoOwner_001, TestSize.Level1)
{
    AAFwk::LaunchServiceDescriptor descriptor;
    int32_t pid = 123;
    int32_t uid = 456;
    EXPECT_EQ(manager_->SpawnDaemonProcess(descriptor, pid, uid), AAFwk::ABILITY_SERVICE_NOT_CONNECTED);
    EXPECT_EQ(pid, 0);
    EXPECT_EQ(uid, -1);
    EXPECT_EQ(manager_->GetTaskHandler(), nullptr);
}

/**
 * @tc.name: LaunchServiceKey_Identity_001
 * @tc.desc: Verify equality includes all identity fields and equal keys have equal hashes.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceManagerTest, LaunchServiceKey_Identity_001, TestSize.Level1)
{
    auto other = key_;
    LaunchServiceManager::LaunchServiceKey::Hash hash;
    EXPECT_TRUE(key_ == other);
    EXPECT_EQ(hash(key_), hash(other));
    other.bundleName += "other";
    EXPECT_FALSE(key_ == other);
    other = key_; other.moduleName += "other";
    EXPECT_FALSE(key_ == other);
    other = key_; other.serviceName += "other";
    EXPECT_FALSE(key_ == other);
    other = key_; ++other.userId;
    EXPECT_FALSE(key_ == other);
    other = key_; ++other.appIndex;
    EXPECT_FALSE(key_ == other);
    EXPECT_NE(manager_->GetLaunchServiceRestartTaskName(key_, 1), manager_->GetLaunchServiceRestartTaskName(key_, 2));
}

/**
 * @tc.name: CreateLaunchServicesEnabledCompletion_Error_001
 * @tc.desc: Verify approval rejection and transport errors are mapped without changing process state.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceManagerTest, CreateLaunchServicesEnabledCompletion_Error_001, TestSize.Level1)
{
    int received = 0;
    int calls = 0;
    auto completion = manager_->CreateLaunchServicesEnabledCompletion(key_.bundleName, 100, 0, true,
        [&received, &calls](int value) { received = value; ++calls; });
    completion(UserIam::UserAuth::AUTH_TOKEN_CHECK_FAILED);
    EXPECT_EQ(received, AAFwk::ERR_LAUNCH_SERVICE_APPROVAL_REJECTED);
    completion(UserIam::UserAuth::CANCELED);
    EXPECT_EQ(received, AAFwk::ERR_LAUNCH_SERVICE_APPROVAL_REJECTED);
    completion(UserIam::UserAuth::GENERAL_ERROR);
    EXPECT_EQ(received, AAFwk::ABILITY_SERVICE_NOT_CONNECTED);
    EXPECT_EQ(calls, 3);
    EXPECT_TRUE(manager_->launchServiceRuntimeRecords_.empty());
}

/**
 * @tc.name: StartLaunchServices_EmptyIAM_001
 * @tc.desc: Verify empty and failed IAM queries do not create runtime records.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceManagerTest, StartLaunchServices_EmptyIAM_001, TestSize.Level1)
{
    auto &launchServiceMock = UserIam::UserAuth::LaunchServiceIAMMock::Instance();
    manager_->StartLaunchServices(key_.bundleName, 100, 0, true);
    EXPECT_EQ(launchServiceMock.listCalls, 1);
    EXPECT_TRUE(manager_->launchServiceRuntimeRecords_.empty());
    launchServiceMock.result = UserIam::UserAuth::GENERAL_ERROR;
    manager_->StartLaunchServices(key_.bundleName, 100, 0, false);
    EXPECT_EQ(launchServiceMock.listCalls, 2);
    EXPECT_TRUE(manager_->launchServiceRuntimeRecords_.empty());
}


/**
 * @tc.name: RegisterDaemon_IAM_001
 * @tc.desc: Verify registration success, duplicate registration, and IAM failure mapping.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceManagerTest, RegisterDaemon_IAM_001, TestSize.Level1)
{
    AAFwk::LaunchServiceDescriptor descriptor {key_.bundleName, key_.moduleName, key_.serviceName, 100, 0};
    auto &launchServiceMock = UserIam::UserAuth::LaunchServiceIAMMock::Instance();
    EXPECT_EQ(manager_->RegisterDaemon(descriptor, 20000001), ERR_OK);
    launchServiceMock.result = UserIam::UserAuth::REPLICATE_REGISTER;
    EXPECT_EQ(manager_->RegisterDaemon(descriptor, 20000001), AAFwk::ERR_LAUNCH_SERVICE_ALREADY_REGISTERED);
    launchServiceMock.result = UserIam::UserAuth::GENERAL_ERROR;
    EXPECT_EQ(manager_->RegisterDaemon(descriptor, 20000001), AAFwk::INNER_ERR);
    EXPECT_EQ(manager_->RegisterDaemon(descriptor, 20000002), AAFwk::ERR_LAUNCH_SERVICE_NOT_FOUND);
    LaunchServiceBundleMock::Instance().services.launchServices.clear();
    EXPECT_EQ(manager_->RegisterDaemon(descriptor, 20000001), AAFwk::ERR_LAUNCH_SERVICE_NOT_FOUND);
}

/**
 * @tc.name: UnregisterDaemon_IAM_001
 * @tc.desc: Successful unregistration removes runtime state; IAM rejection preserves it.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceManagerTest, UnregisterDaemon_IAM_001, TestSize.Level1)
{
    AAFwk::LaunchServiceDescriptor descriptor {key_.bundleName, key_.moduleName, key_.serviceName, 100, 0};
    auto &launchServiceMock = UserIam::UserAuth::LaunchServiceIAMMock::Instance();
    manager_->launchServiceRuntimeRecords_[key_].uid = 20000001;
    launchServiceMock.result = UserIam::UserAuth::NOT_ENROLLED;
    EXPECT_EQ(manager_->UnregisterDaemon(descriptor, 20000001), AAFwk::ERR_LAUNCH_SERVICE_NOT_REGISTERED);
    EXPECT_EQ(manager_->launchServiceRuntimeRecords_.count(key_), 1);
    launchServiceMock.result = UserIam::UserAuth::SUCCESS;
    EXPECT_EQ(manager_->UnregisterDaemon(descriptor, 20000001), ERR_OK);
    EXPECT_EQ(manager_->launchServiceRuntimeRecords_.count(key_), 0);
    EXPECT_EQ(launchServiceMock.unregisterCalls, 2);
}

/**
 * @tc.name: StartStopDaemon_Developer_001
 * @tc.desc: Mismatched developers cannot start or stop another application service.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceManagerTest, StartStopDaemon_Developer_001, TestSize.Level1)
{
    AAFwk::LaunchServiceDescriptor descriptor {key_.bundleName, key_.moduleName, key_.serviceName, 100, 0};
    auto &bundle = LaunchServiceBundleMock::Instance();
    bundle.developerId = "other";
    EXPECT_EQ(manager_->StartDaemon(descriptor, 20000001), AAFwk::ERR_LAUNCH_SERVICE_NOT_FOUND);
    bundle.provisionResult = ERR_INVALID_VALUE;
    EXPECT_EQ(manager_->StopDaemon(descriptor, 20000001), AAFwk::ERR_LAUNCH_SERVICE_NOT_FOUND);
    bundle.Reset();
    EXPECT_EQ(manager_->StartDaemon(descriptor, 20000001), AAFwk::ABILITY_SERVICE_NOT_CONNECTED);
    EXPECT_EQ(manager_->StopDaemon(descriptor, 20000001), AAFwk::ERR_LAUNCH_SERVICE_NOT_RUNNING);
}

/**
 * @tc.name: CheckLaunchServiceStartLocked_IAM_001
 * @tc.desc: Missing, disabled, forbidden, and upgrading registrations cannot start.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceManagerTest, CheckLaunchServiceStartLocked_IAM_001, TestSize.Level1)
{
    auto inner = std::make_shared<AppMgrServiceInner>();
    auto manager = std::make_shared<LaunchServiceManager>(inner);
    AAFwk::LaunchServiceDescriptor descriptor;
    auto &launchServiceMock = UserIam::UserAuth::LaunchServiceIAMMock::Instance();
    EXPECT_EQ(manager->CheckLaunchServiceStartLocked(key_, descriptor), AAFwk::ERR_LAUNCH_SERVICE_NOT_REGISTERED);
    UserIam::UserAuth::LaunchServiceFlatInfo record;
    record.uid = 20000001;
    record.serviceKey = {key_.serviceName, key_.moduleName};
    record.activeState = UserIam::UserAuth::LaunchServiceState::DEACTIVATE;
    launchServiceMock.records = {record};
    EXPECT_EQ(manager->CheckLaunchServiceStartLocked(key_, descriptor), AAFwk::ERR_LAUNCH_SERVICE_DISABLED);
    launchServiceMock.records[0].activeState = UserIam::UserAuth::LaunchServiceState::ACTIVATE;
    launchServiceMock.records[0].isForbidden = true;
    EXPECT_EQ(manager->CheckLaunchServiceStartLocked(key_, descriptor), AAFwk::ERR_LAUNCH_SERVICE_DISABLED);
    launchServiceMock.records[0].isForbidden = false;
    EXPECT_EQ(manager->CheckLaunchServiceStartLocked(key_, descriptor), ERR_OK);
    EXPECT_EQ(descriptor.bundleName, key_.bundleName);
    manager->upgradingLaunchServiceKeys_[record.uid] = {key_};
    EXPECT_EQ(manager->CheckLaunchServiceStartLocked(key_, descriptor), AAFwk::ERR_UNINSTALLING_OR_UPGRADING_APP);
}

/**
 * @tc.name: StartLaunchServiceLocked_WatchedProcess_001
 * @tc.desc: A live watcher or non-ESRCH query error preserves the running record; no watcher retries spawn.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceManagerTest, StartLaunchServiceLocked_WatchedProcess_001, TestSize.Level1)
{
    auto inner = std::make_shared<AppMgrServiceInner>();
    auto manager = std::make_shared<LaunchServiceManager>(inner);
    UserIam::UserAuth::LaunchServiceFlatInfo registration;
    registration.uid = 20000001;
    registration.serviceKey = {key_.serviceName, key_.moduleName};
    registration.activeState = UserIam::UserAuth::LaunchServiceState::ACTIVATE;
    UserIam::UserAuth::LaunchServiceIAMMock::Instance().records = {registration};
    const pid_t pid = getpid();
    auto &pidfdManager = AppPidFdManager::GetInstance();
    ASSERT_EQ(pidfdManager.pidfdMap_.count(pid), 0u);
    const int fd = pidfdManager.OpenPidFd(pid, 0);
    ASSERT_GE(fd, 0);
    // Do not register with the poller: this test owns and closes the descriptor.
    pidfdManager.pidfdMap_.emplace(pid, AppPidFdManager::PidFdEntry{fd, pid});
    LaunchService service;
    auto &record = manager->launchServiceRuntimeRecords_[key_];
    record.pid = pid;
    record.generation = 7;
    EXPECT_EQ(manager->StartLaunchServiceLocked(key_, service, 0, 0), ERR_OK);
    EXPECT_EQ(manager->launchServiceRuntimeRecords_.count(key_), 1u);
    if (manager->launchServiceRuntimeRecords_.count(key_) != 0) {
        EXPECT_EQ(manager->launchServiceRuntimeRecords_.at(key_).generation, 7u);
    }
    pidfdManager.pidfdMap_.at(pid).pidfd = -1;
    EXPECT_EQ(manager->StartLaunchServiceLocked(key_, service, 0, 0), ERR_OK);
    EXPECT_EQ(manager->launchServiceRuntimeRecords_.count(key_), 1u);
    if (manager->launchServiceRuntimeRecords_.count(key_) != 0) {
        EXPECT_EQ(manager->launchServiceRuntimeRecords_.at(key_).pid, pid);
    }
    pidfdManager.pidfdMap_.erase(pid);
    EXPECT_EQ(close(fd), 0);
    // Missing bundle makes a new spawn fail deterministically, without spawning a real process.
    EXPECT_EQ(manager->StartLaunchServiceLocked(key_, service, 0, 0), AAFwk::ERR_LAUNCH_SERVICE_NOT_FOUND);
    EXPECT_TRUE(manager->launchServiceRuntimeRecords_.empty());
}

/**
 * @tc.name: GetInformation_IAM_001
 * @tc.desc: Query maps IAM states and returns runtime PID without losing descriptor.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceManagerTest, GetInformation_IAM_001, TestSize.Level1)
{
    AbilityRuntime::LaunchServiceDescriptor descriptor;
    descriptor.bundleName = key_.bundleName;
    descriptor.moduleName = key_.moduleName;
    descriptor.serviceName = key_.serviceName;
    descriptor.userId = 100;
    AbilityRuntime::LaunchServiceInformation information;
    auto &launchServiceMock = UserIam::UserAuth::LaunchServiceIAMMock::Instance();
    EXPECT_EQ(manager_->GetLaunchServiceInformation(descriptor, information, 20000001), ERR_OK);
    EXPECT_EQ(information.state, AbilityRuntime::LaunchServiceState::NOT_REGISTERED);
    UserIam::UserAuth::LaunchServiceFlatInfo record;
    record.uid = 20000001;
    record.serviceKey = {key_.serviceName, key_.moduleName};
    record.activeState = UserIam::UserAuth::LaunchServiceState::ACTIVATE;
    launchServiceMock.records = {record};
    manager_->launchServiceRuntimeRecords_[key_].pid = 123;
    EXPECT_EQ(manager_->GetLaunchServiceInformation(descriptor, information, 20000001), ERR_OK);
    EXPECT_EQ(information.state, AbilityRuntime::LaunchServiceState::ENABLED);
    EXPECT_EQ(information.pid, 123);
    launchServiceMock.records[0].activeState = UserIam::UserAuth::LaunchServiceState::DEACTIVATE;
    EXPECT_EQ(manager_->GetLaunchServiceInformation(descriptor, information, 20000001), ERR_OK);
    EXPECT_EQ(information.state, AbilityRuntime::LaunchServiceState::DISABLED);
    launchServiceMock.result = UserIam::UserAuth::GENERAL_ERROR;
    EXPECT_EQ(manager_->GetLaunchServiceInformation(descriptor, information, 20000001),
        AAFwk::ABILITY_SERVICE_NOT_CONNECTED);
    LaunchServiceBundleMock::Instance().result = ERR_INVALID_VALUE;
    EXPECT_EQ(manager_->GetLaunchServiceInformation(descriptor, information, 20000001),
        AAFwk::ERR_LAUNCH_SERVICE_NOT_FOUND);
}

/**
 * @tc.name: SetEnabled_WaitsForIAM_001
 * @tc.desc: Missing token keeps completion pending; IAM rejection completes exactly once.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceManagerTest, SetEnabled_WaitsForIAM_001, TestSize.Level1)
{
    auto calls = std::make_shared<int32_t>(0);
    auto result = std::make_shared<int32_t>(ERR_OK);
    manager_->SetLaunchServicesEnabled(key_.bundleName, 100, 0, {}, true, 20000001,
        [calls, result](int32_t value) { ++*calls; *result = value; });
    auto &launchServiceMock = UserIam::UserAuth::LaunchServiceIAMMock::Instance();
    ASSERT_NE(launchServiceMock.pending, nullptr);
    EXPECT_EQ(*calls, 0);
    EXPECT_TRUE(launchServiceMock.lastToken.empty());
    EXPECT_EQ(launchServiceMock.lastUid, 20000001);
    EXPECT_EQ(launchServiceMock.lastState, UserIam::UserAuth::LaunchServiceState::ACTIVATE);
    launchServiceMock.pending->OnResult(UserIam::UserAuth::CANCELED);
    EXPECT_EQ(*calls, 1);
    EXPECT_EQ(*result, AAFwk::ERR_LAUNCH_SERVICE_APPROVAL_REJECTED);
    launchServiceMock.pending->OnResult(UserIam::UserAuth::SUCCESS);
    launchServiceMock.pending.reset();
    EXPECT_EQ(*calls, 1);
}

HWTEST_F(LaunchServiceManagerTest, UsesLaunchServiceTaskHandler, TestSize.Level1)
{
    auto inner = std::make_shared<AppMgrServiceInner>();
    auto generalHandler = AAFwk::TaskHandlerWrap::CreateConcurrentQueueHandler("launch_general_test", 1);
    auto launchHandler = AAFwk::TaskHandlerWrap::CreateConcurrentQueueHandler("launch_dedicated_test", 1);
    ASSERT_NE(generalHandler, nullptr);
    ASSERT_NE(launchHandler, nullptr);
    inner->SetTaskHandler(generalHandler);
    manager_ = inner->GetLaunchServiceManager();
    EXPECT_EQ(manager_->GetTaskHandler(), nullptr);
    inner->SetLaunchServiceTaskHandler(launchHandler);
    EXPECT_EQ(manager_->GetTaskHandler(), launchHandler);
    EXPECT_NE(manager_->GetTaskHandler(), generalHandler);
    inner->SetLaunchServiceTaskHandler(nullptr);
    EXPECT_EQ(manager_->GetTaskHandler(), nullptr);
    inner->SetLaunchServiceTaskHandler(launchHandler);
    launchHandler.reset();
    EXPECT_EQ(manager_->GetTaskHandler(), nullptr);
}

/**
 * @tc.name: SetEnabled_Success_001
 * @tc.desc: Successful IAM callback completes asynchronous enable and disable operations.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceManagerTest, SetEnabled_Success_001, TestSize.Level1)
{
    auto inner = std::make_shared<AppMgrServiceInner>();
    inner->SetTaskHandler(AAFwk::TaskHandlerWrap::CreateConcurrentQueueHandler(
        "launch_service_test_queue", 1, AAFwk::TaskQoS::USER_INTERACTIVE));
    ASSERT_NE(inner->GetTaskHandler(), nullptr);
    inner->SetLaunchServiceTaskHandler(inner->GetTaskHandler());
    manager_ = inner->GetLaunchServiceManager();
    auto &launchServiceMock = UserIam::UserAuth::LaunchServiceIAMMock::Instance();
    for (bool enabled : {true, false}) {
        auto promise = std::make_shared<std::promise<int32_t>>();
        auto future = promise->get_future();
        manager_->SetLaunchServicesEnabled(key_.bundleName, 100, 0, {1}, enabled, 20000001,
            [promise](int32_t result) { promise->set_value(result); });
        ASSERT_NE(launchServiceMock.pending, nullptr);
        EXPECT_EQ(launchServiceMock.lastToken, std::vector<uint8_t>({1}));
        EXPECT_EQ(launchServiceMock.lastState, enabled ? UserIam::UserAuth::LaunchServiceState::ACTIVATE :
            UserIam::UserAuth::LaunchServiceState::DEACTIVATE);
        EXPECT_EQ(future.wait_for(std::chrono::milliseconds(0)), std::future_status::timeout);
        launchServiceMock.pending->OnResult(UserIam::UserAuth::SUCCESS);
        ASSERT_EQ(future.wait_for(std::chrono::seconds(3)), std::future_status::ready);
        EXPECT_EQ(future.get(), ERR_OK);
        launchServiceMock.pending.reset();
    }
}

/**
 * @tc.name: SetEnabled_MissingBundle_001
 * @tc.desc: Missing bundle reports target error without starting IAM approval.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceManagerTest, SetEnabled_MissingBundle_001, TestSize.Level1)
{
    LaunchServiceBundleMock::Instance().uid = -1;
    int32_t calls = 0;
    int32_t result = ERR_OK;
    manager_->SetLaunchServicesEnabled(key_.bundleName, 100, 0, {}, true, 20000001,
        [&calls, &result](int32_t value) { ++calls; result = value; });
    EXPECT_EQ(calls, 1);
    EXPECT_EQ(result, AAFwk::ERR_TARGET_BUNDLE_NOT_EXIST);
    EXPECT_EQ(UserIam::UserAuth::LaunchServiceIAMMock::Instance().pending, nullptr);
}

/**
 * @tc.name: CleanupByUid_001
 * @tc.desc: UID cleanup preserves unrelated apps and rejects invalid UID.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceManagerTest, CleanupByUid_001, TestSize.Level1)
{
    manager_->launchServiceRuntimeRecords_[key_].uid = 20000001;
    auto other = key_;
    other.appIndex = 1;
    manager_->launchServiceRuntimeRecords_[other].uid = 20000002;
    manager_->CleanupLaunchServicesByUid(-1);
    EXPECT_EQ(manager_->launchServiceRuntimeRecords_.size(), 2);
    manager_->UnregisterLaunchServicesByUid(20000001);
    EXPECT_EQ(manager_->launchServiceRuntimeRecords_.count(key_), 0);
    EXPECT_EQ(manager_->launchServiceRuntimeRecords_.count(other), 1);
    EXPECT_EQ(UserIam::UserAuth::LaunchServiceIAMMock::Instance().unregisterCalls, 1);
}

/**
 * @tc.name: UnregisterRemovedLaunchService_001
 * @tc.desc: Removed declaration cleanup is skipped on IAM failure and performed on success.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceManagerTest, UnregisterRemovedLaunchService_001, TestSize.Level1)
{
    AAFwk::LaunchServiceDescriptor descriptor {key_.bundleName, key_.moduleName, key_.serviceName, 100, 0};
    auto &launchServiceMock = UserIam::UserAuth::LaunchServiceIAMMock::Instance();
    manager_->launchServiceRuntimeRecords_[key_].uid = 20000001;
    launchServiceMock.result = UserIam::UserAuth::GENERAL_ERROR;
    manager_->UnregisterRemovedLaunchService(descriptor, 20000001);
    EXPECT_EQ(manager_->launchServiceRuntimeRecords_.count(key_), 1);
    launchServiceMock.result = UserIam::UserAuth::SUCCESS;
    manager_->UnregisterRemovedLaunchService(descriptor, 20000001);
    EXPECT_EQ(manager_->launchServiceRuntimeRecords_.count(key_), 0);
}

/**
 * @tc.name: UpgradeTracking_001
 * @tc.desc: Upgrade defers IAM unregistration; completion clears the pending UID entry.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceManagerTest, UpgradeTracking_001, TestSize.Level1)
{
    manager_->launchServiceRuntimeRecords_[key_].uid = 20000001;
    manager_->HandleLaunchServiceUninstallOrUpgrade(key_.bundleName, 20000001, true);
    EXPECT_EQ(manager_->upgradingLaunchServiceKeys_.count(20000001), 1);
    EXPECT_EQ(UserIam::UserAuth::LaunchServiceIAMMock::Instance().unregisterCalls, 0);
    manager_->HandleLaunchServiceUpgradeEnd(20000001);
    EXPECT_EQ(manager_->upgradingLaunchServiceKeys_.count(20000001), 0);
    manager_->HandleLaunchServiceUninstallOrUpgrade(key_.bundleName, 20000001, false);
    EXPECT_EQ(UserIam::UserAuth::LaunchServiceIAMMock::Instance().unregisterCalls, 1);
}

/**
 * @tc.name: StartDaemonProcess_001
 * @tc.desc: AppSpawn failures reset PID; positive PID is required for success.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceManagerTest, StartDaemonProcess_001, TestSize.Level1)
{
    auto inner = std::make_shared<AppMgrServiceInner>();
    inner->remoteClientManager_ = std::make_shared<RemoteClientManager>();
    auto spawn = std::make_shared<ControlledDaemonSpawnClient>();
    inner->remoteClientManager_->SetSpawnClient(spawn);
    AppSpawnStartMsg message {};
    int32_t pid = 0;
    EXPECT_EQ(manager_->StartDaemonProcess(inner, message, pid), ERR_OK);
    EXPECT_EQ(pid, 123);
    spawn->outputPid = 0;
    EXPECT_EQ(manager_->StartDaemonProcess(inner, message, pid), AAFwk::ABILITY_SERVICE_NOT_CONNECTED);
    EXPECT_EQ(pid, 0);
    spawn->outputPid = 123;
    spawn->result = ERR_INVALID_VALUE;
    EXPECT_EQ(manager_->StartDaemonProcess(inner, message, pid), ERR_INVALID_VALUE);
    EXPECT_EQ(pid, 0);
}

/**
 * @tc.name: SpawnDaemonProcess_MissingBundle_001
 * @tc.desc: Missing bundle rejects spawn and resets PID and UID.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceManagerTest, SpawnDaemonProcess_MissingBundle_001, TestSize.Level1)
{
    auto inner = std::make_shared<AppMgrServiceInner>();
    auto manager = std::make_shared<LaunchServiceManager>(inner);
    AAFwk::LaunchServiceDescriptor descriptor {key_.bundleName, key_.moduleName, key_.serviceName, 100, 0};
    int32_t pid = 123;
    int32_t uid = 20000001;
    EXPECT_EQ(manager->SpawnDaemonProcess(descriptor, pid, uid), AAFwk::ERR_LAUNCH_SERVICE_NOT_FOUND);
    EXPECT_EQ(pid, 0);
    EXPECT_EQ(uid, -1);
}

/**
 * @tc.name: ObserverForwarding_001
 * @tc.desc: Duplicate registration delivers once and a single unregister stops manager notifications.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceManagerTest, ObserverForwarding_001, TestSize.Level1)
{
    auto observer = sptr<ManagerRecordingObserver>::MakeSptr();
    ASSERT_EQ(manager_->RegisterLaunchServiceObserver(observer, 20000001), ERR_OK);
    ASSERT_EQ(manager_->RegisterLaunchServiceObserver(observer, 20000001), ERR_OK);
    AAFwk::LaunchServiceDescriptor descriptor {key_.bundleName, key_.moduleName, key_.serviceName, 100, 0};
    manager_->NotifyLaunchServiceStateChanged(descriptor, AbilityRuntime::LaunchServiceState::ENABLED);
    manager_->NotifyLaunchServiceProcessStateChanged(key_, 123, AbilityRuntime::LaunchServiceProcessState::CREATED);
    EXPECT_EQ(observer->statusCalls, 1);
    EXPECT_EQ(observer->status.descriptor.bundleName, key_.bundleName);
    EXPECT_EQ(observer->processCalls, 1);
    EXPECT_EQ(observer->process.pid, 123);
    EXPECT_EQ(manager_->UnregisterLaunchServiceObserver(observer), ERR_OK);
    manager_->NotifyLaunchServiceStateChanged(descriptor, AbilityRuntime::LaunchServiceState::DISABLED);
    EXPECT_EQ(observer->statusCalls, 1);
}

/**
 * @tc.name: GetAllRegisteredInformation_Records_001
 * @tc.desc: Convert matching IAM records, filter missing declarations and merge runtime PID.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceManagerTest, GetAllRegisteredInformation_Records_001, TestSize.Level1)
{
    auto &launchServiceMock = UserIam::UserAuth::LaunchServiceIAMMock::Instance();
    UserIam::UserAuth::LaunchServiceFlatInfo record;
    record.uid = 20000001;
    record.bundleInfo.ownerBundleName = key_.bundleName;
    record.bundleInfo.appIndex = 0;
    record.serviceKey = {key_.serviceName, key_.moduleName};
    record.activeState = UserIam::UserAuth::LaunchServiceState::ACTIVATE;
    launchServiceMock.records = {record};
    manager_->launchServiceRuntimeRecords_[key_].pid = 123;
    std::vector<AbilityRuntime::LaunchServiceInformation> information;
    ASSERT_EQ(manager_->GetAllRegisteredInformation(information, 100, 100), ERR_OK);
    ASSERT_EQ(information.size(), 1);
    EXPECT_EQ(information[0].pid, 123);
    EXPECT_EQ(information[0].state, AbilityRuntime::LaunchServiceState::ENABLED);
    EXPECT_EQ(information[0].descriptor.bundleName, key_.bundleName);
    LaunchServiceBundleMock::Instance().services.launchServices.clear();
    EXPECT_EQ(manager_->GetAllRegisteredInformation(information, 100, 100), ERR_OK);
    EXPECT_TRUE(information.empty());
}

/**
 * @tc.name: PrepareDaemonStartMsg_001
 * @tc.desc: Preserve application identity and data groups when shared-bundle lookup succeeds or fails.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceManagerTest, PrepareDaemonStartMsg_001, TestSize.Level1)
{
    auto inner = std::make_shared<AppMgrServiceInner>();
    inner->remoteClientManager_.reset();
    auto helper = DelayedSingleton<BundleMgrHelper>::GetInstance();
    BundleInfo bundle;
    bundle.name = key_.bundleName;
    bundle.uid = 20000001;
    bundle.gid = 20000002;
    bundle.applicationInfo.appSignType = "enterprise";
    bundle.reqPermissions = {"ohos.permission.PROTECT_SCREEN_LOCK_DATA"};
    AppSpawnStartMsg message {};
    manager_->PrepareDaemonStartMsg(inner, helper, bundle, message);
    EXPECT_EQ(message.uid, bundle.uid);
    EXPECT_EQ(message.gid, bundle.gid);
    EXPECT_EQ(message.appSignType, "enterprise");
    EXPECT_EQ(message.hspList.size(), 1);
    ASSERT_EQ(message.dataGroupInfoList.size(), 1);
    EXPECT_EQ(message.dataGroupInfoList[0].gid, 123);
    EXPECT_TRUE(message.isScreenLockDataProtect);
    LaunchServiceBundleMock::Instance().hspResult = ERR_INVALID_VALUE;
    AppSpawnStartMsg failedHsp {};
    bundle.reqPermissions.clear();
    manager_->PrepareDaemonStartMsg(inner, helper, bundle, failedHsp);
    EXPECT_TRUE(failedHsp.hspList.empty());
    EXPECT_EQ(failedHsp.dataGroupInfoList.size(), 1);
    EXPECT_FALSE(failedHsp.isScreenLockDataProtect);
}

/**
 * @tc.name: CleanupRemovedLaunchServices_001
 * @tc.desc: Unregister only IAM records no longer declared by the updated bundle.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceManagerTest, CleanupRemovedLaunchServices_001, TestSize.Level1)
{
    auto &bundle = LaunchServiceBundleMock::Instance();
    bundle.bundleAvailable = true;
    auto &launchServiceMock = UserIam::UserAuth::LaunchServiceIAMMock::Instance();
    UserIam::UserAuth::LaunchServiceFlatInfo record;
    record.uid = 20000001;
    record.bundleInfo.ownerBundleName = key_.bundleName;
    record.serviceKey = {key_.serviceName, key_.moduleName};
    record.serviceInfo.serviceType = UserIam::UserAuth::LaunchServiceType::PRIVILEGED_DAEMON;
    launchServiceMock.records = {record};
    HapModuleInfo module;
    module.moduleName = key_.moduleName;
    module.launchServices = bundle.services.launchServices;
    bundle.bundle.hapModuleInfos = {module};
    manager_->CleanupRemovedLaunchServices(record.uid);
    EXPECT_EQ(launchServiceMock.unregisterCalls, 0);
    bundle.bundle.hapModuleInfos.clear();
    manager_->launchServiceRuntimeRecords_[key_].uid = record.uid;
    manager_->CleanupRemovedLaunchServices(record.uid);
    EXPECT_EQ(launchServiceMock.unregisterCalls, 1);
    EXPECT_EQ(manager_->launchServiceRuntimeRecords_.count(key_), 0);
}

/**
 * @tc.name: RestartLaunchService_001
 * @tc.desc: Stale generation, running PID, and disabled keepalive preserve the runtime record.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceManagerTest, RestartLaunchService_001, TestSize.Level1)
{
    manager_->RestartLaunchService(key_, 1);
    EXPECT_TRUE(manager_->launchServiceRuntimeRecords_.empty());
    auto &record = manager_->launchServiceRuntimeRecords_[key_];
    record.generation = 2;
    record.pid = 123;
    record.service.keepAlive = true;
    manager_->RestartLaunchService(key_, 0);
    manager_->RestartLaunchService(key_, 1);
    manager_->RestartLaunchService(key_, 2);
    EXPECT_EQ(record.pid, 123);
    record.pid = 0;
    record.service.keepAlive = false;
    manager_->RestartLaunchService(key_, 2);
    EXPECT_EQ(record.generation, 2);
    EXPECT_EQ(record.restartCount, 0);
}
} // namespace AppExecFwk
} // namespace OHOS
