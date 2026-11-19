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
#include <future>
#include <chrono>
#include <map>
#include <mutex>
#define private public
#include "app_mgr_service.h"
#undef private
#include "ability_manager_errors.h"
#include "cache_process_manager.h"
#include "permission_verification.h"
#include "parameters.h"
#include "launch_service_operation_callback.h"
using namespace testing::ext;
namespace {
constexpr const char *ALLOW_NON_DUAL_MODE = "const.sys.appms.allow_launch_service_on_non_dual_mode_device";
constexpr const char *PC_MODE_SWITCH = "const.window.support_window_pcmode_switch";
constexpr const char *MAIN_MODE = "const.sceneboard.mainmode";
constexpr const char *PC_MODE = "persist.sceneboard.ispcmode";
std::map<std::string, std::string> g_parameterValues;
int32_t g_deviceTypeReads = 0;
bool g_systemCaller = true;
bool g_permissionGranted = true;
}
namespace OHOS::system {
std::string GetParameter(const std::string &key, const std::string &def)
{
    if (key == "const.product.devicetype") {
        ++g_deviceTypeReads;
    }
    auto iter = g_parameterValues.find(key);
    return iter == g_parameterValues.end() ? def : iter->second;
}
bool GetBoolParameter(const std::string &key, bool def)
{
    auto value = GetParameter(key, "");
    if (value == "true" || value == "1") {
        return true;
    }
    if (value == "false" || value == "0") {
        return false;
    }
    return def;
}
}
namespace OHOS::AAFwk {
bool PermissionVerification::JudgeCallerIsAllowedToUseSystemAPI() const { return g_systemCaller; }
bool PermissionVerification::VerifyCallingPermission(const std::string &, const uint32_t) const
{
    return g_permissionGranted;
}
}
namespace OHOS::AppExecFwk {
CacheProcessManager::CacheProcessManager() = default;
CacheProcessManager::~CacheProcessManager() = default;

bool CacheProcessManager::QueryEnableProcessCache()
{
    ADD_FAILURE() << "Launch service entry tests must not query process cache capability";
    return false;
}

class LaunchServiceServiceTest : public testing::Test {
public:
    void SetUp() override
    {
        g_parameterValues.clear();
        g_deviceTypeReads = 0;
        g_systemCaller = true;
        g_permissionGranted = true;
    }
    void TearDown() override { SetUp(); }
    std::shared_ptr<AppMgrService> service_ = std::make_shared<AppMgrService>();
};

/**
 * @tc.name: RunLaunchServiceTask_001
 * @tc.desc: Absent handlers reject tasks; available queue preserves task result and inner instance.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceServiceTest, RunLaunchServiceTask_001, TestSize.Level1)
{
    service_->appMgrServiceInner_ = std::make_shared<AppMgrServiceInner>();
    service_->launchServiceTaskHandler_ = AAFwk::TaskHandlerWrap::CreateConcurrentQueueHandler(
        "launch_service_test_queue", 1, AAFwk::TaskQoS::USER_INTERACTIVE);
    ASSERT_NE(service_->launchServiceTaskHandler_, nullptr);
    int32_t calls = 0;
    service_->launchServiceTaskHandler_.reset();
    EXPECT_EQ(service_->RunLaunchServiceTask([&calls](const auto &) { ++calls; return ERR_OK; }),
        AAFwk::ABILITY_SERVICE_NOT_CONNECTED);
    EXPECT_EQ(calls, 0);
    service_->launchServiceTaskHandler_ = AAFwk::TaskHandlerWrap::CreateConcurrentQueueHandler(
        "launch_service_test_queue", 1, AAFwk::TaskQoS::USER_INTERACTIVE);
    ASSERT_NE(service_->launchServiceTaskHandler_, nullptr);
    EXPECT_EQ(service_->RunLaunchServiceTask([this, &calls](const auto &inner) {
        EXPECT_EQ(inner, service_->appMgrServiceInner_);
        ++calls;
        return ERR_INVALID_VALUE;
    }), ERR_INVALID_VALUE);
    EXPECT_EQ(calls, 1);
}

/**
 * @tc.name: RunLaunchServiceTask_Independent_001
 * @tc.desc: Independent service requests execute while sharing the launch-service queue.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceServiceTest, RunLaunchServiceTask_Independent_001, TestSize.Level1)
{
    service_->launchServiceTaskHandler_ = AAFwk::TaskHandlerWrap::CreateConcurrentQueueHandler(
        "launch_service_test_queue", 2, AAFwk::TaskQoS::USER_INTERACTIVE);
    ASSERT_NE(service_->launchServiceTaskHandler_, nullptr);
    auto release = std::make_shared<std::promise<void>>();
    auto gate = release->get_future().share();
    auto entered = std::make_shared<std::promise<void>>();
    auto enteredFuture = entered->get_future();
    auto first = std::async(std::launch::async, [service = service_, gate, entered]() {
        return service->RunLaunchServiceTask([gate, entered](const auto &) {
            entered->set_value();
            gate.wait();
            return ERR_OK;
        });
    });
    auto status = enteredFuture.wait_for(std::chrono::seconds(3));
    auto second = std::async(std::launch::async, [service = service_]() {
        return service->RunLaunchServiceTask([](const auto &) { return ERR_OK; });
    });
    auto secondStatus = second.wait_for(std::chrono::seconds(3));
    release->set_value();
    EXPECT_EQ(status, std::future_status::ready);
    EXPECT_EQ(secondStatus, std::future_status::ready);
    EXPECT_EQ(first.get(), ERR_OK);
    EXPECT_EQ(second.get(), ERR_OK);
}

/**
 * @tc.name: RegisterDaemon_DeviceAndReadiness_001
 * @tc.desc: Reject unsupported devices and unavailable service before scheduling.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceServiceTest, RegisterDaemon_DeviceAndReadiness_001, TestSize.Level1)
{
    AAFwk::LaunchServiceDescriptor descriptor;
    EXPECT_EQ(service_->RegisterDaemon(descriptor), AAFwk::ERR_CAPABILITY_NOT_SUPPORT);
    g_parameterValues[ALLOW_NON_DUAL_MODE] = "true";
    service_->appMgrServiceInner_.reset();
    EXPECT_EQ(service_->RegisterDaemon(descriptor), AAFwk::ABILITY_SERVICE_NOT_CONNECTED);
}

/**
 * @tc.name: UnregisterDaemon_DeviceAndReadiness_001
 * @tc.desc: Reject unsupported devices and unavailable service before scheduling.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceServiceTest, UnregisterDaemon_DeviceAndReadiness_001, TestSize.Level1)
{
    AAFwk::LaunchServiceDescriptor descriptor;
    EXPECT_EQ(service_->UnregisterDaemon(descriptor), AAFwk::ERR_CAPABILITY_NOT_SUPPORT);
    g_parameterValues[ALLOW_NON_DUAL_MODE] = "true";
    service_->appMgrServiceInner_.reset();
    EXPECT_EQ(service_->UnregisterDaemon(descriptor), AAFwk::ABILITY_SERVICE_NOT_CONNECTED);
}

/**
 * @tc.name: StartDaemon_DeviceAndReadiness_001
 * @tc.desc: Reject unsupported devices and unavailable service before scheduling.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceServiceTest, StartDaemon_DeviceAndReadiness_001, TestSize.Level1)
{
    AAFwk::LaunchServiceDescriptor descriptor;
    EXPECT_EQ(service_->StartDaemon(descriptor), AAFwk::ERR_CAPABILITY_NOT_SUPPORT);
    g_parameterValues[ALLOW_NON_DUAL_MODE] = "true";
    service_->appMgrServiceInner_.reset();
    EXPECT_EQ(service_->StartDaemon(descriptor), AAFwk::ABILITY_SERVICE_NOT_CONNECTED);
}

/**
 * @tc.name: StopDaemon_DeviceAndReadiness_001
 * @tc.desc: Reject unsupported devices and unavailable service before scheduling.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceServiceTest, StopDaemon_DeviceAndReadiness_001, TestSize.Level1)
{
    AAFwk::LaunchServiceDescriptor descriptor;
    EXPECT_EQ(service_->StopDaemon(descriptor), AAFwk::ERR_CAPABILITY_NOT_SUPPORT);
    g_parameterValues[ALLOW_NON_DUAL_MODE] = "true";
    service_->appMgrServiceInner_.reset();
    EXPECT_EQ(service_->StopDaemon(descriptor), AAFwk::ABILITY_SERVICE_NOT_CONNECTED);
}

/**
 * @tc.name: GetInformation_Guards_001
 * @tc.desc: Device, permission, and descriptor validation return distinct errors.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceServiceTest, GetInformation_Guards_001, TestSize.Level1)
{
    AbilityRuntime::LaunchServiceDescriptor descriptor;
    AbilityRuntime::LaunchServiceInformation information;
    EXPECT_EQ(service_->GetLaunchServiceInformation(descriptor, information), AAFwk::ERR_CAPABILITY_NOT_SUPPORT);
    g_parameterValues[ALLOW_NON_DUAL_MODE] = "true";
    g_permissionGranted = false;
    EXPECT_EQ(service_->GetLaunchServiceInformation(descriptor, information), AAFwk::CHECK_PERMISSION_FAILED);
    g_permissionGranted = true;
    EXPECT_EQ(service_->GetLaunchServiceInformation(descriptor, information), ERR_INVALID_VALUE);
}

/**
 * @tc.name: GetAllInformation_Guards_001
 * @tc.desc: System API and permission checks precede user validation.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceServiceTest, GetAllInformation_Guards_001, TestSize.Level1)
{
    std::vector<AbilityRuntime::LaunchServiceInformation> output;
    EXPECT_EQ(service_->GetAllRegisteredInformation(100, output), AAFwk::ERR_CAPABILITY_NOT_SUPPORT);
    g_parameterValues[ALLOW_NON_DUAL_MODE] = "true";
    g_systemCaller = false;
    EXPECT_EQ(service_->GetAllRegisteredInformation(100, output), AAFwk::ERR_NOT_SYSTEM_APP);
    g_systemCaller = true;
    g_permissionGranted = false;
    EXPECT_EQ(service_->GetAllRegisteredInformation(100, output), AAFwk::CHECK_PERMISSION_FAILED);
    g_permissionGranted = true;
    EXPECT_EQ(service_->GetAllRegisteredInformation(-2, output), ERR_INVALID_VALUE);
}

/**
 * @tc.name: RegisterLaunchServiceObserver_Guards_001
 * @tc.desc: Reject unsupported devices and callers lacking system status or permission.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceServiceTest, RegisterLaunchServiceObserver_Guards_001, TestSize.Level1)
{
    EXPECT_EQ(service_->RegisterLaunchServiceObserver(nullptr), AAFwk::ERR_CAPABILITY_NOT_SUPPORT);
    g_parameterValues[ALLOW_NON_DUAL_MODE] = "true";
    g_systemCaller = false;
    EXPECT_EQ(service_->RegisterLaunchServiceObserver(nullptr), AAFwk::ERR_NOT_SYSTEM_APP);
    g_systemCaller = true;
    g_permissionGranted = false;
    EXPECT_EQ(service_->RegisterLaunchServiceObserver(nullptr), AAFwk::CHECK_PERMISSION_FAILED);
    g_permissionGranted = true;
    service_->appMgrServiceInner_.reset();
    EXPECT_EQ(service_->RegisterLaunchServiceObserver(nullptr), AAFwk::ABILITY_SERVICE_NOT_CONNECTED);
}

/**
 * @tc.name: UnregisterLaunchServiceObserver_Guards_001
 * @tc.desc: Reject unsupported devices and callers lacking system status or permission.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceServiceTest, UnregisterLaunchServiceObserver_Guards_001, TestSize.Level1)
{
    EXPECT_EQ(service_->UnregisterLaunchServiceObserver(nullptr), AAFwk::ERR_CAPABILITY_NOT_SUPPORT);
    g_parameterValues[ALLOW_NON_DUAL_MODE] = "true";
    g_systemCaller = false;
    EXPECT_EQ(service_->UnregisterLaunchServiceObserver(nullptr), AAFwk::ERR_NOT_SYSTEM_APP);
    g_systemCaller = true;
    g_permissionGranted = false;
    EXPECT_EQ(service_->UnregisterLaunchServiceObserver(nullptr), AAFwk::CHECK_PERMISSION_FAILED);
    g_permissionGranted = true;
    service_->appMgrServiceInner_.reset();
    EXPECT_EQ(service_->UnregisterLaunchServiceObserver(nullptr), AAFwk::ABILITY_SERVICE_NOT_CONNECTED);
}

/**
 * @tc.name: SetEnabled_Guards_001
 * @tc.desc: Validate permission, callback, bundle name, user and app index before scheduling.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceServiceTest, SetEnabled_Guards_001, TestSize.Level1)
{
    auto callback = sptr<AbilityRuntime::LaunchServiceOperationCallback>::MakeSptr([](int32_t) {});
    auto invoke = [this, callback]() { return service_->
        SetLaunchServicesEnabled("com.example.daemon", 100, 0, {}, true, callback); };
    EXPECT_EQ(invoke(), AAFwk::ERR_CAPABILITY_NOT_SUPPORT);
    g_parameterValues[ALLOW_NON_DUAL_MODE] = "true";
    g_systemCaller = false;
    EXPECT_EQ(invoke(), AAFwk::ERR_NOT_SYSTEM_APP);
    g_systemCaller = true;
    g_permissionGranted = false;
    EXPECT_EQ(invoke(), AAFwk::CHECK_PERMISSION_FAILED);
    g_permissionGranted = true;
    EXPECT_EQ(service_->SetLaunchServicesEnabled("com.example.daemon", 100, 0, {}, true, nullptr), ERR_INVALID_VALUE);
    EXPECT_EQ(service_->SetLaunchServicesEnabled("", 100, 0, {}, true, callback), ERR_INVALID_VALUE);
    EXPECT_EQ(service_->SetLaunchServicesEnabled("com.example.daemon", -1, 0, {}, true, callback), ERR_INVALID_VALUE);
    EXPECT_EQ(service_->SetLaunchServicesEnabled("com.example.daemon", 100, -1, {}, true, callback), ERR_INVALID_VALUE);
    service_->appMgrServiceInner_.reset();
    EXPECT_EQ(invoke(), AAFwk::ABILITY_SERVICE_NOT_CONNECTED);
}

/**
 * @tc.name: ScheduleLaunchServicesAfterIAMReady_001
 * @tc.desc: An unready service cannot schedule startup after IAM becomes available.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceServiceTest, ScheduleLaunchServicesAfterIAMReady_001, TestSize.Level1)
{
    service_->taskHandler_.reset();
    EXPECT_EQ(service_->ScheduleLaunchServicesAfterIAMReady(), AAFwk::ABILITY_SERVICE_NOT_CONNECTED);
}

/**
 * @tc.name: ScheduleLaunchServicesAfterIAMReady_NoLaunchHandler_001
 * @tc.desc: IAM startup scheduling requires the dedicated launch-service queue.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceServiceTest, ScheduleLaunchServicesAfterIAMReady_NoLaunchHandler_001, TestSize.Level1)
{
    service_->taskHandler_ = AAFwk::TaskHandlerWrap::CreateQueueHandler("launch_boot_general_test");
    ASSERT_NE(service_->taskHandler_, nullptr);
    service_->eventHandler_ = std::make_shared<AMSEventHandler>(service_->taskHandler_, service_->appMgrServiceInner_);
    service_->launchServiceTaskHandler_.reset();
    ASSERT_TRUE(service_->IsReady());
    EXPECT_EQ(service_->ScheduleLaunchServicesAfterIAMReady(), AAFwk::ABILITY_SERVICE_NOT_CONNECTED);
}

/**
 * @tc.name: NonDualModeCapability_001
 * @tc.desc: Non-dual-mode devices use the opt-in switch, independent of product device type.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceServiceTest, NonDualModeCapability_001, TestSize.Level1)
{
    AAFwk::LaunchServiceDescriptor descriptor;
    service_->appMgrServiceInner_.reset();
    g_parameterValues[PC_MODE_SWITCH] = "false";
    g_parameterValues[MAIN_MODE] = "2";
    g_parameterValues[PC_MODE] = "false";
    g_parameterValues["const.product.devicetype"] = "2in1";
    EXPECT_EQ(service_->RegisterDaemon(descriptor), AAFwk::ERR_CAPABILITY_NOT_SUPPORT);
    g_parameterValues[ALLOW_NON_DUAL_MODE] = "false";
    EXPECT_EQ(service_->RegisterDaemon(descriptor), AAFwk::ERR_CAPABILITY_NOT_SUPPORT);
    g_parameterValues[ALLOW_NON_DUAL_MODE] = "true";
    g_parameterValues["const.product.devicetype"] = "phone";
    EXPECT_EQ(service_->RegisterDaemon(descriptor), AAFwk::ABILITY_SERVICE_NOT_CONNECTED);
    EXPECT_EQ(g_deviceTypeReads, 0);
}

/**
 * @tc.name: DualModeCapability_001
 * @tc.desc: Dual-mode devices follow PC mode regardless of the non-dual-mode switch.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceServiceTest, DualModeCapability_001, TestSize.Level1)
{
    AAFwk::LaunchServiceDescriptor descriptor;
    service_->appMgrServiceInner_.reset();
    g_parameterValues[PC_MODE_SWITCH] = "true";
    for (const char *mode : {"0", "1"}) {
        g_parameterValues[MAIN_MODE] = mode;
        for (const char *allowed : {"true", "false"}) {
            g_parameterValues[ALLOW_NON_DUAL_MODE] = allowed;
            g_parameterValues[PC_MODE] = "true";
            EXPECT_EQ(service_->RegisterDaemon(descriptor), AAFwk::ABILITY_SERVICE_NOT_CONNECTED);
            g_parameterValues[PC_MODE] = "false";
            EXPECT_EQ(service_->RegisterDaemon(descriptor), AAFwk::ERR_CAPABILITY_NOT_SUPPORT);
        }
    }
    EXPECT_EQ(g_deviceTypeReads, 0);
}

/**
 * @tc.name: PartialModeParameters_001
 * @tc.desc: Switch-capable devices reject invalid main mode and missing PC mode.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceServiceTest, PartialModeParameters_001, TestSize.Level1)
{
    AAFwk::LaunchServiceDescriptor descriptor;
    service_->appMgrServiceInner_.reset();
    g_parameterValues[PC_MODE_SWITCH] = "true";
    g_parameterValues[ALLOW_NON_DUAL_MODE] = "true";
    g_parameterValues[PC_MODE] = "true";
    for (const char *mode : {"", "2", "tablet"}) {
        g_parameterValues[MAIN_MODE] = mode;
        EXPECT_EQ(service_->RegisterDaemon(descriptor), AAFwk::ERR_CAPABILITY_NOT_SUPPORT);
    }
    g_parameterValues.erase(MAIN_MODE);
    EXPECT_EQ(service_->RegisterDaemon(descriptor), AAFwk::ERR_CAPABILITY_NOT_SUPPORT);
    g_parameterValues[MAIN_MODE] = "0";
    g_parameterValues.erase(PC_MODE);
    EXPECT_EQ(service_->RegisterDaemon(descriptor), AAFwk::ERR_CAPABILITY_NOT_SUPPORT);
    EXPECT_EQ(g_deviceTypeReads, 0);
}
} // namespace OHOS::AppExecFwk
