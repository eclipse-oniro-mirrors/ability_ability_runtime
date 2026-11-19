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
#include <functional>
#include "ability_manager_errors.h"
#include "app_mgr_interface.h"
#include "app_mgr_ipc_interface_code.h"
#include "app_service_manager.h"
#include "daemon_service_manager_client.h"
#include "ipc_object_stub.h"
#include "launch_service_operation_callback.h"
#include "message_parcel.h"

using namespace testing::ext;

namespace OHOS {
namespace {
constexpr int32_t TEST_USER_ID = 100;
constexpr int32_t TEST_APP_INDEX = 2;
sptr<IRemoteObject> g_testAppMgrRemote;
class LaunchServiceRemoteMock : public IPCObjectStub {
public:
    std::function<int(uint32_t, MessageParcel &, MessageParcel &)> replyHandler;
    int OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &) override
    {
        EXPECT_EQ(data.ReadInterfaceToken(), AppExecFwk::IAppMgr::GetDescriptor());
        return replyHandler(code, data, reply);
    }
};
class TestLaunchObserver : public AbilityRuntime::LaunchServiceObserverStub {
public:
    void OnLaunchServiceStatusChanged(const AbilityRuntime::LaunchServiceStateData &) override {}
    void OnLaunchServiceProcessStateChanged(const AbilityRuntime::LaunchServiceProcessStateData &) override {}
};
}
namespace AppExecFwk {
sptr<IRemoteObject> AppServiceManager::GetAppMgrService() const
{
    return g_testAppMgrRemote;
}
}
namespace AbilityRuntime {
class LaunchServiceClientTest : public testing::Test {
public:
    void SetUp() override
    {
        remote_ = sptr<LaunchServiceRemoteMock>::MakeSptr();
        g_testAppMgrRemote = remote_;
        descriptor_.bundleName = "com.example.daemon";
        descriptor_.moduleName = "entry";
        descriptor_.serviceName = "worker";
        descriptor_.userId = TEST_USER_ID;
        descriptor_.appIndex = TEST_APP_INDEX;
    }
    void TearDown() override
    {
        g_testAppMgrRemote = nullptr;
        remote_ = nullptr;
    }
    sptr<LaunchServiceRemoteMock> remote_;
    LaunchServiceDescriptor descriptor_;
    DaemonServiceManagerClient &client_ = DaemonServiceManagerClient::GetInstance();
};

/**
 * @tc.name: Register_Results_001
 * @tc.desc: Verify IPC identity fields and success, service error and transport error propagation.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceClientTest, Register_Results_001, TestSize.Level1)
{
    for (int32_t result : std::vector<int32_t>{ERR_OK, AAFwk::ERR_LAUNCH_SERVICE_NOT_FOUND}) {
        remote_->replyHandler = [this, result](uint32_t code, MessageParcel &data, MessageParcel &reply) {
            EXPECT_EQ(code, static_cast<uint32_t>(AppExecFwk::AppMgrInterfaceCode::REGISTER_LAUNCH_SERVICE));
            EXPECT_EQ(data.ReadString(), descriptor_.bundleName);
            EXPECT_EQ(data.ReadString(), descriptor_.moduleName);
            EXPECT_EQ(data.ReadString(), descriptor_.serviceName);
            EXPECT_EQ(data.ReadInt32(), descriptor_.userId);
            EXPECT_EQ(data.ReadInt32(), descriptor_.appIndex);
            EXPECT_TRUE(reply.WriteInt32(result));
            return ERR_OK;
        };
        EXPECT_EQ(client_.Register(descriptor_), result);
    }
    remote_->replyHandler = [](uint32_t, MessageParcel &, MessageParcel &) { return ERR_INVALID_STATE; };
    EXPECT_EQ(client_.Register(descriptor_), ERR_INVALID_STATE);
    g_testAppMgrRemote = nullptr;
    EXPECT_EQ(client_.Register(descriptor_), AAFwk::ABILITY_SERVICE_NOT_CONNECTED);
}

/**
 * @tc.name: Unregister_Results_001
 * @tc.desc: Verify IPC identity fields and success, service error and transport error propagation.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceClientTest, Unregister_Results_001, TestSize.Level1)
{
    for (int32_t result : std::vector<int32_t>{ERR_OK, AAFwk::ERR_LAUNCH_SERVICE_NOT_FOUND}) {
        remote_->replyHandler = [this, result](uint32_t code, MessageParcel &data, MessageParcel &reply) {
            EXPECT_EQ(code, static_cast<uint32_t>(AppExecFwk::AppMgrInterfaceCode::UNREGISTER_LAUNCH_SERVICE));
            EXPECT_EQ(data.ReadString(), descriptor_.bundleName);
            EXPECT_EQ(data.ReadString(), descriptor_.moduleName);
            EXPECT_EQ(data.ReadString(), descriptor_.serviceName);
            EXPECT_EQ(data.ReadInt32(), descriptor_.userId);
            EXPECT_EQ(data.ReadInt32(), descriptor_.appIndex);
            EXPECT_TRUE(reply.WriteInt32(result));
            return ERR_OK;
        };
        EXPECT_EQ(client_.Unregister(descriptor_), result);
    }
    remote_->replyHandler = [](uint32_t, MessageParcel &, MessageParcel &) { return ERR_INVALID_STATE; };
    EXPECT_EQ(client_.Unregister(descriptor_), ERR_INVALID_STATE);
    g_testAppMgrRemote = nullptr;
    EXPECT_EQ(client_.Unregister(descriptor_), AAFwk::ABILITY_SERVICE_NOT_CONNECTED);
}

/**
 * @tc.name: Start_Results_001
 * @tc.desc: Verify IPC identity fields and success, service error and transport error propagation.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceClientTest, Start_Results_001, TestSize.Level1)
{
    for (int32_t result : std::vector<int32_t>{ERR_OK, AAFwk::ERR_LAUNCH_SERVICE_NOT_FOUND}) {
        remote_->replyHandler = [this, result](uint32_t code, MessageParcel &data, MessageParcel &reply) {
            EXPECT_EQ(code, static_cast<uint32_t>(AppExecFwk::AppMgrInterfaceCode::START_LAUNCH_SERVICE));
            EXPECT_EQ(data.ReadString(), descriptor_.bundleName);
            EXPECT_EQ(data.ReadString(), descriptor_.moduleName);
            EXPECT_EQ(data.ReadString(), descriptor_.serviceName);
            EXPECT_EQ(data.ReadInt32(), descriptor_.userId);
            EXPECT_EQ(data.ReadInt32(), descriptor_.appIndex);
            EXPECT_TRUE(reply.WriteInt32(result));
            return ERR_OK;
        };
        EXPECT_EQ(client_.Start(descriptor_), result);
    }
    remote_->replyHandler = [](uint32_t, MessageParcel &, MessageParcel &) { return ERR_INVALID_STATE; };
    EXPECT_EQ(client_.Start(descriptor_), ERR_INVALID_STATE);
    g_testAppMgrRemote = nullptr;
    EXPECT_EQ(client_.Start(descriptor_), AAFwk::ABILITY_SERVICE_NOT_CONNECTED);
}

/**
 * @tc.name: Stop_Results_001
 * @tc.desc: Verify IPC identity fields and success, service error and transport error propagation.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceClientTest, Stop_Results_001, TestSize.Level1)
{
    for (int32_t result : std::vector<int32_t>{ERR_OK, AAFwk::ERR_LAUNCH_SERVICE_NOT_FOUND}) {
        remote_->replyHandler = [this, result](uint32_t code, MessageParcel &data, MessageParcel &reply) {
            EXPECT_EQ(code, static_cast<uint32_t>(AppExecFwk::AppMgrInterfaceCode::STOP_LAUNCH_SERVICE));
            EXPECT_EQ(data.ReadString(), descriptor_.bundleName);
            EXPECT_EQ(data.ReadString(), descriptor_.moduleName);
            EXPECT_EQ(data.ReadString(), descriptor_.serviceName);
            EXPECT_EQ(data.ReadInt32(), descriptor_.userId);
            EXPECT_EQ(data.ReadInt32(), descriptor_.appIndex);
            EXPECT_TRUE(reply.WriteInt32(result));
            return ERR_OK;
        };
        EXPECT_EQ(client_.Stop(descriptor_), result);
    }
    remote_->replyHandler = [](uint32_t, MessageParcel &, MessageParcel &) { return ERR_INVALID_STATE; };
    EXPECT_EQ(client_.Stop(descriptor_), ERR_INVALID_STATE);
    g_testAppMgrRemote = nullptr;
    EXPECT_EQ(client_.Stop(descriptor_), AAFwk::ABILITY_SERVICE_NOT_CONNECTED);
}

/**
 * @tc.name: SetLaunchServicesEnabled_Deferred_001
 * @tc.desc: Verify omitted token waits for a retained callback and completes exactly once.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceClientTest, SetLaunchServicesEnabled_Deferred_001, TestSize.Level1)
{
    for (int32_t result : std::vector<int32_t>{ERR_OK, AAFwk::ERR_LAUNCH_SERVICE_APPROVAL_REJECTED}) {
        sptr<ILaunchServiceOperationCallback> pending;
        remote_->replyHandler = [this, &pending](uint32_t code, MessageParcel &data, MessageParcel &reply) {
            EXPECT_EQ(code, static_cast<uint32_t>(AppExecFwk::AppMgrInterfaceCode::SET_LAUNCH_SERVICES_ENABLED));
            EXPECT_EQ(data.ReadString(), descriptor_.bundleName);
            EXPECT_EQ(data.ReadInt32(), TEST_USER_ID);
            EXPECT_EQ(data.ReadInt32(), TEST_APP_INDEX);
            std::vector<uint8_t> token;
            EXPECT_TRUE(data.ReadUInt8Vector(&token));
            EXPECT_TRUE(token.empty());
            EXPECT_TRUE(data.ReadBool());
            pending = iface_cast<ILaunchServiceOperationCallback>(data.ReadRemoteObject());
            EXPECT_TRUE(reply.WriteInt32(ERR_OK));
            return ERR_OK;
        };
        int calls = 0;
        int received = -1;
        client_.SetLaunchServicesEnabled(descriptor_.bundleName, TEST_USER_ID, TEST_APP_INDEX, {}, true,
            [&calls, &received](int value) { ++calls; received = value; });
        ASSERT_NE(pending, nullptr);
        EXPECT_EQ(calls, 0);
        pending->OnResult(result);
        EXPECT_EQ(calls, 1);
        EXPECT_EQ(received, result);
        pending->OnResult(ERR_INVALID_VALUE);
        pending = nullptr;
        EXPECT_EQ(calls, 1);
    }
}

/**
 * @tc.name: SetLaunchServicesEnabled_Token_001
 * @tc.desc: Verify explicit token and disabled state are serialized and service rejection completes once.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceClientTest, SetLaunchServicesEnabled_Token_001, TestSize.Level1)
{
    remote_->replyHandler = [this](uint32_t, MessageParcel &data, MessageParcel &reply) {
        EXPECT_EQ(data.ReadString(), descriptor_.bundleName);
        EXPECT_EQ(data.ReadInt32(), TEST_USER_ID);
        EXPECT_EQ(data.ReadInt32(), TEST_APP_INDEX);
        std::vector<uint8_t> token;
        EXPECT_TRUE(data.ReadUInt8Vector(&token));
        EXPECT_EQ(token, (std::vector<uint8_t>{1, 2, 255}));
        EXPECT_FALSE(data.ReadBool());
        EXPECT_NE(data.ReadRemoteObject(), nullptr);
        EXPECT_TRUE(reply.WriteInt32(AAFwk::CHECK_PERMISSION_FAILED));
        return ERR_OK;
    };
    int count = 0;
    client_.SetLaunchServicesEnabled(descriptor_.bundleName, TEST_USER_ID, TEST_APP_INDEX, {1, 2, 255}, false,
        [&count](int result) { ++count; EXPECT_EQ(result, AAFwk::CHECK_PERMISSION_FAILED); });
    EXPECT_EQ(count, 1);
}

/**
 * @tc.name: SetLaunchServicesEnabled_Disconnected_001
 * @tc.desc: Verify unavailable service completes with connection error.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceClientTest, SetLaunchServicesEnabled_Disconnected_001, TestSize.Level1)
{
    g_testAppMgrRemote = nullptr;
    int count = 0;
    client_.SetLaunchServicesEnabled("com.example.daemon", TEST_USER_ID, 0, {}, true,
        [&count](int result) { ++count; EXPECT_EQ(result, AAFwk::ABILITY_SERVICE_NOT_CONNECTED); });
    EXPECT_EQ(count, 1);
    client_.SetLaunchServicesEnabled("com.example.daemon", TEST_USER_ID, 0, {}, true, {});
}

/**
 * @tc.name: RegisterObserver_Results_001
 * @tc.desc: Verify IPC routing and service error propagation.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceClientTest, RegisterObserver_Results_001, TestSize.Level1)
{
    auto observer = sptr<TestLaunchObserver>::MakeSptr();
    for (int result : std::vector<int32_t>{ERR_OK, AAFwk::CHECK_PERMISSION_FAILED}) {
        remote_->replyHandler = [observer, result](uint32_t code, MessageParcel &data, MessageParcel &reply) {
            EXPECT_EQ(code, static_cast<uint32_t>(AppExecFwk::AppMgrInterfaceCode::REGISTER_LAUNCH_SERVICE_OBSERVER));
            EXPECT_EQ(data.ReadRemoteObject(), observer->AsObject());
            EXPECT_TRUE(reply.WriteInt32(result));
            return ERR_OK;
        };
        EXPECT_EQ(client_.RegisterObserver(observer), result);
    }
    g_testAppMgrRemote = nullptr;
    EXPECT_EQ(client_.RegisterObserver(observer), AAFwk::ABILITY_SERVICE_NOT_CONNECTED);
}

/**
 * @tc.name: UnregisterObserver_Results_001
 * @tc.desc: Verify IPC routing and service error propagation.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceClientTest, UnregisterObserver_Results_001, TestSize.Level1)
{
    auto observer = sptr<TestLaunchObserver>::MakeSptr();
    for (int result : std::vector<int32_t>{ERR_OK, AAFwk::CHECK_PERMISSION_FAILED}) {
        remote_->replyHandler = [observer, result](uint32_t code, MessageParcel &data, MessageParcel &reply) {
            EXPECT_EQ(code, static_cast<uint32_t>(AppExecFwk::AppMgrInterfaceCode::UNREGISTER_LAUNCH_SERVICE_OBSERVER));
            EXPECT_EQ(data.ReadRemoteObject(), observer->AsObject());
            EXPECT_TRUE(reply.WriteInt32(result));
            return ERR_OK;
        };
        EXPECT_EQ(client_.UnregisterObserver(observer), result);
    }
    g_testAppMgrRemote = nullptr;
    EXPECT_EQ(client_.UnregisterObserver(observer), AAFwk::ABILITY_SERVICE_NOT_CONNECTED);
}

/**
 * @tc.name: GetInformation_Results_001
 * @tc.desc: Verify information is decoded and failed replies preserve the output.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceClientTest, GetInformation_Results_001, TestSize.Level1)
{
    LaunchServiceInformation expected;
    expected.descriptor = descriptor_;
    expected.state = LaunchServiceState::ENABLED;
    expected.pid = 123;
    for (int result : std::vector<int32_t>{ERR_OK, AAFwk::ERR_LAUNCH_SERVICE_NOT_FOUND}) {
        remote_->replyHandler = [this, result, expected](uint32_t code, MessageParcel &data, MessageParcel &reply) {
            EXPECT_EQ(code, static_cast<uint32_t>(AppExecFwk::AppMgrInterfaceCode::GET_LAUNCH_SERVICE_INFORMATION));
            std::unique_ptr<LaunchServiceDescriptor> input(data.ReadParcelable<LaunchServiceDescriptor>());
            EXPECT_NE(input, nullptr);
            if (input) { EXPECT_EQ(input->bundleName, descriptor_.bundleName); }
            EXPECT_TRUE(reply.WriteInt32(result));
            if (result == ERR_OK) { EXPECT_TRUE(reply.WriteParcelable(&expected)); }
            return ERR_OK;
        };
        LaunchServiceInformation output;
        output.pid = 999;
        EXPECT_EQ(client_.GetInformation(descriptor_, output), result);
        EXPECT_EQ(output.pid, result == ERR_OK ? 123 : 999);
    }
    remote_->replyHandler = [](uint32_t, MessageParcel &, MessageParcel &reply) {
        reply.WriteInt32(ERR_OK);
        return ERR_OK;
    };
    LaunchServiceInformation output;
    EXPECT_EQ(client_.GetInformation(descriptor_, output), ERR_INVALID_VALUE);
    g_testAppMgrRemote = nullptr;
    EXPECT_EQ(client_.GetInformation(descriptor_, output), AAFwk::ABILITY_SERVICE_NOT_CONNECTED);
}

/**
 * @tc.name: GetAllRegisteredInformation_Results_001
 * @tc.desc: Verify list decoding, invalid counts, service errors and disconnect.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceClientTest, GetAllRegisteredInformation_Results_001, TestSize.Level1)
{
    LaunchServiceInformation item;
    item.descriptor = descriptor_;
    for (int count : {0, 1, -1, 2147483647}) {
        remote_->replyHandler = [count, item](uint32_t code, MessageParcel &data, MessageParcel &reply) {
            EXPECT_EQ(code, static_cast<uint32_t>(AppExecFwk::AppMgrInterfaceCode::GET_ALL_REGISTERED_INFORMATION));
            EXPECT_EQ(data.ReadInt32(), TEST_USER_ID);
            reply.WriteInt32(ERR_OK);
            reply.WriteInt32(count);
            if (count == 1) { EXPECT_TRUE(reply.WriteParcelable(&item)); }
            return ERR_OK;
        };
        std::vector<LaunchServiceInformation> output;
        EXPECT_EQ(client_.GetAllRegisteredInformation(TEST_USER_ID, output),
            count == 0 || count == 1 ? ERR_OK : ERR_INVALID_VALUE);
        EXPECT_EQ(output.size(), count == 1 ? 1u : 0u);
    }
    remote_->replyHandler = [](uint32_t, MessageParcel &, MessageParcel &reply) {
        reply.WriteInt32(AAFwk::CHECK_PERMISSION_FAILED);
        return ERR_OK;
    };
    std::vector<LaunchServiceInformation> output;
    EXPECT_EQ(client_.GetAllRegisteredInformation(TEST_USER_ID, output), AAFwk::CHECK_PERMISSION_FAILED);
    g_testAppMgrRemote = nullptr;
    EXPECT_EQ(client_.GetAllRegisteredInformation(TEST_USER_ID, output), AAFwk::ABILITY_SERVICE_NOT_CONNECTED);
}

} // namespace AbilityRuntime
} // namespace OHOS
