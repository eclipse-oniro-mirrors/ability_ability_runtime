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
#define private public
#define protected public
#include "app_mgr_stub.h"
#undef private
#undef protected
#include "mock_app_mgr_service.h"
#include "app_mgr_ipc_interface_code.h"
#include "launch_service_operation_callback.h"
#include "launch_service_observer.h"
#include "ipc_types.h"

using namespace testing;
using namespace testing::ext;
namespace OHOS::AppExecFwk {
class LaunchServiceStubMock : public MockAppMgrService {
public:
    MOCK_METHOD1(RegisterDaemon, int32_t(const AAFwk::LaunchServiceDescriptor &));
    MOCK_METHOD1(UnregisterDaemon, int32_t(const AAFwk::LaunchServiceDescriptor &));
    MOCK_METHOD1(StartDaemon, int32_t(const AAFwk::LaunchServiceDescriptor &));
    MOCK_METHOD1(StopDaemon, int32_t(const AAFwk::LaunchServiceDescriptor &));
    MOCK_METHOD2(GetLaunchServiceInformation, int32_t(const AbilityRuntime::LaunchServiceDescriptor &,
        AbilityRuntime::LaunchServiceInformation &));
    MOCK_METHOD2(GetAllRegisteredInformation, int32_t(int32_t,
        std::vector<AbilityRuntime::LaunchServiceInformation> &));
    MOCK_METHOD6(SetLaunchServicesEnabled, int32_t(const std::string &, int32_t, int32_t,
        const std::vector<uint8_t> &, bool, const sptr<AbilityRuntime::ILaunchServiceOperationCallback> &));
    MOCK_METHOD1(RegisterLaunchServiceObserver, int32_t(const sptr<AbilityRuntime::ILaunchServiceObserver> &));
    MOCK_METHOD1(UnregisterLaunchServiceObserver, int32_t(const sptr<AbilityRuntime::ILaunchServiceObserver> &));
};
class StubLaunchObserver : public AbilityRuntime::LaunchServiceObserverStub {
public:
    void OnLaunchServiceStatusChanged(const AbilityRuntime::LaunchServiceStateData &) override {}
    void OnLaunchServiceProcessStateChanged(const AbilityRuntime::LaunchServiceProcessStateData &) override {}
};
class LaunchServiceStubTest : public testing::Test {
public:
    sptr<LaunchServiceStubMock> stub_ = sptr<LaunchServiceStubMock>::MakeSptr();
};

/**
 * @tc.name: RegisterDaemon_Dispatch_001
 * @tc.desc: Dispatch a valid descriptor and preserve the service result.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceStubTest, RegisterDaemon_Dispatch_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    ASSERT_TRUE(data.WriteString("com.example.daemon"));
    ASSERT_TRUE(data.WriteString("entry"));
    ASSERT_TRUE(data.WriteString("worker"));
    ASSERT_TRUE(data.WriteInt32(100));
    ASSERT_TRUE(data.WriteInt32(2));
    EXPECT_CALL(*stub_, RegisterDaemon(_)).WillOnce(Invoke([](const auto &descriptor) {
        EXPECT_EQ(descriptor.bundleName, "com.example.daemon");
        EXPECT_EQ(descriptor.moduleName, "entry");
        EXPECT_EQ(descriptor.serviceName, "worker");
        EXPECT_EQ(descriptor.userId, 100);
        EXPECT_EQ(descriptor.appIndex, 2);
        return ERR_INVALID_STATE;
    }));
    EXPECT_EQ(stub_->OnRemoteRequestLaunchService(
        static_cast<uint32_t>(AppMgrInterfaceCode::REGISTER_LAUNCH_SERVICE), data, reply), ERR_OK);
    EXPECT_EQ(reply.ReadInt32(), ERR_INVALID_STATE);
}

/**
 * @tc.name: RegisterDaemon_Truncated_001
 * @tc.desc: Reject truncated descriptors without calling the service.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceStubTest, RegisterDaemon_Truncated_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    EXPECT_CALL(*stub_, RegisterDaemon(_)).Times(0);
    EXPECT_EQ(stub_->HandleRegisterDaemon(data, reply), ERR_OK);
    EXPECT_EQ(reply.ReadInt32(), ERR_INVALID_VALUE);
}

/**
 * @tc.name: UnregisterDaemon_Dispatch_001
 * @tc.desc: Dispatch a valid descriptor and preserve the service result.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceStubTest, UnregisterDaemon_Dispatch_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    ASSERT_TRUE(data.WriteString("com.example.daemon"));
    ASSERT_TRUE(data.WriteString("entry"));
    ASSERT_TRUE(data.WriteString("worker"));
    ASSERT_TRUE(data.WriteInt32(100));
    ASSERT_TRUE(data.WriteInt32(2));
    EXPECT_CALL(*stub_, UnregisterDaemon(_)).WillOnce(Invoke([](const auto &descriptor) {
        EXPECT_EQ(descriptor.bundleName, "com.example.daemon");
        EXPECT_EQ(descriptor.moduleName, "entry");
        EXPECT_EQ(descriptor.serviceName, "worker");
        EXPECT_EQ(descriptor.userId, 100);
        EXPECT_EQ(descriptor.appIndex, 2);
        return ERR_INVALID_STATE;
    }));
    EXPECT_EQ(stub_->OnRemoteRequestLaunchService(
        static_cast<uint32_t>(AppMgrInterfaceCode::UNREGISTER_LAUNCH_SERVICE), data, reply), ERR_OK);
    EXPECT_EQ(reply.ReadInt32(), ERR_INVALID_STATE);
}

/**
 * @tc.name: UnregisterDaemon_Truncated_001
 * @tc.desc: Reject truncated descriptors without calling the service.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceStubTest, UnregisterDaemon_Truncated_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    EXPECT_CALL(*stub_, UnregisterDaemon(_)).Times(0);
    EXPECT_EQ(stub_->HandleUnregisterDaemon(data, reply), ERR_OK);
    EXPECT_EQ(reply.ReadInt32(), ERR_INVALID_VALUE);
}

/**
 * @tc.name: StartDaemon_Dispatch_001
 * @tc.desc: Dispatch a valid descriptor and preserve the service result.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceStubTest, StartDaemon_Dispatch_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    ASSERT_TRUE(data.WriteString("com.example.daemon"));
    ASSERT_TRUE(data.WriteString("entry"));
    ASSERT_TRUE(data.WriteString("worker"));
    ASSERT_TRUE(data.WriteInt32(100));
    ASSERT_TRUE(data.WriteInt32(2));
    EXPECT_CALL(*stub_, StartDaemon(_)).WillOnce(Invoke([](const auto &descriptor) {
        EXPECT_EQ(descriptor.bundleName, "com.example.daemon");
        EXPECT_EQ(descriptor.moduleName, "entry");
        EXPECT_EQ(descriptor.serviceName, "worker");
        EXPECT_EQ(descriptor.userId, 100);
        EXPECT_EQ(descriptor.appIndex, 2);
        return ERR_INVALID_STATE;
    }));
    EXPECT_EQ(stub_->OnRemoteRequestLaunchService(
        static_cast<uint32_t>(AppMgrInterfaceCode::START_LAUNCH_SERVICE), data, reply), ERR_OK);
    EXPECT_EQ(reply.ReadInt32(), ERR_INVALID_STATE);
}

/**
 * @tc.name: StartDaemon_Truncated_001
 * @tc.desc: Reject truncated descriptors without calling the service.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceStubTest, StartDaemon_Truncated_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    EXPECT_CALL(*stub_, StartDaemon(_)).Times(0);
    EXPECT_EQ(stub_->HandleStartDaemon(data, reply), ERR_OK);
    EXPECT_EQ(reply.ReadInt32(), ERR_INVALID_VALUE);
}

/**
 * @tc.name: StopDaemon_Dispatch_001
 * @tc.desc: Dispatch a valid descriptor and preserve the service result.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceStubTest, StopDaemon_Dispatch_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    ASSERT_TRUE(data.WriteString("com.example.daemon"));
    ASSERT_TRUE(data.WriteString("entry"));
    ASSERT_TRUE(data.WriteString("worker"));
    ASSERT_TRUE(data.WriteInt32(100));
    ASSERT_TRUE(data.WriteInt32(2));
    EXPECT_CALL(*stub_, StopDaemon(_)).WillOnce(Invoke([](const auto &descriptor) {
        EXPECT_EQ(descriptor.bundleName, "com.example.daemon");
        EXPECT_EQ(descriptor.moduleName, "entry");
        EXPECT_EQ(descriptor.serviceName, "worker");
        EXPECT_EQ(descriptor.userId, 100);
        EXPECT_EQ(descriptor.appIndex, 2);
        return ERR_INVALID_STATE;
    }));
    EXPECT_EQ(stub_->OnRemoteRequestLaunchService(
        static_cast<uint32_t>(AppMgrInterfaceCode::STOP_LAUNCH_SERVICE), data, reply), ERR_OK);
    EXPECT_EQ(reply.ReadInt32(), ERR_INVALID_STATE);
}

/**
 * @tc.name: StopDaemon_Truncated_001
 * @tc.desc: Reject truncated descriptors without calling the service.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceStubTest, StopDaemon_Truncated_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    EXPECT_CALL(*stub_, StopDaemon(_)).Times(0);
    EXPECT_EQ(stub_->HandleStopDaemon(data, reply), ERR_OK);
    EXPECT_EQ(reply.ReadInt32(), ERR_INVALID_VALUE);
}

/**
 * @tc.name: GetInformation_001
 * @tc.desc: Serialize success information and preserve service errors.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceStubTest, GetInformation_001, TestSize.Level1)
{
    for (int32_t result : std::vector<int32_t>{ERR_OK, ERR_INVALID_STATE}) {
        MessageParcel data;
        MessageParcel reply;
        AbilityRuntime::LaunchServiceDescriptor descriptor;
        descriptor.bundleName = "com.example.daemon";
        descriptor.moduleName = "entry";
        descriptor.serviceName = "worker";
        descriptor.userId = 100;
        ASSERT_TRUE(data.WriteParcelable(&descriptor));
        EXPECT_CALL(*stub_, GetLaunchServiceInformation(_, _)).WillOnce(Invoke(
            [descriptor, result](const auto &input, auto &output) {
                EXPECT_EQ(input.bundleName, descriptor.bundleName);
                output.descriptor = input;
                return result;
            }));
        ASSERT_EQ(stub_->HandleGetLaunchServiceInformation(data, reply), ERR_OK);
        ASSERT_EQ(reply.ReadInt32(), result);
        if (result == ERR_OK) {
            std::unique_ptr<AbilityRuntime::LaunchServiceInformation> value(
                reply.ReadParcelable<AbilityRuntime::LaunchServiceInformation>());
            ASSERT_NE(value, nullptr);
            EXPECT_EQ(value->descriptor.bundleName, descriptor.bundleName);
        }
    }
    MessageParcel empty;
    MessageParcel reply;
    EXPECT_EQ(stub_->HandleGetLaunchServiceInformation(empty, reply), ERR_INVALID_VALUE);
}

/**
 * @tc.name: GetAllInformation_001
 * @tc.desc: Serialize list results and reject missing user ids.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceStubTest, GetAllInformation_001, TestSize.Level1)
{
    for (int32_t result : std::vector<int32_t>{ERR_OK, ERR_INVALID_STATE}) {
        MessageParcel data;
        MessageParcel reply;
        ASSERT_TRUE(data.WriteInt32(100));
        EXPECT_CALL(*stub_, GetAllRegisteredInformation(100, _)).WillOnce(Return(result));
        ASSERT_EQ(stub_->HandleGetAllRegisteredInformation(data, reply), ERR_OK);
        EXPECT_EQ(reply.ReadInt32(), result);
        if (result == ERR_OK) {
            EXPECT_EQ(reply.ReadInt32(), 0);
        }
    }
    MessageParcel empty;
    MessageParcel reply;
    EXPECT_EQ(stub_->HandleGetAllRegisteredInformation(empty, reply), ERR_INVALID_VALUE);
}

/**
 * @tc.name: SetEnabled_001
 * @tc.desc: Forward token and callback; reject requests with no callback.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceStubTest, SetEnabled_001, TestSize.Level1)
{
    auto callback = sptr<AbilityRuntime::LaunchServiceOperationCallback>::MakeSptr([](int32_t) {});
    for (bool withCallback : {false, true}) {
        MessageParcel data;
        MessageParcel reply;
        ASSERT_TRUE(data.WriteString("com.example.daemon"));
        ASSERT_TRUE(data.WriteInt32(100));
        ASSERT_TRUE(data.WriteInt32(2));
        ASSERT_TRUE(data.WriteUInt8Vector({1, 2, 3}));
        ASSERT_TRUE(data.WriteBool(true));
        if (withCallback) {
            ASSERT_TRUE(data.WriteRemoteObject(callback->AsObject()));
            EXPECT_CALL(*stub_, SetLaunchServicesEnabled("com.example.daemon", 100, 2,
                std::vector<uint8_t>({1, 2, 3}), true, _)).WillOnce(Return(ERR_OK));
        }
        EXPECT_EQ(stub_->HandleSetLaunchServicesEnabled(data, reply),
            withCallback ? ERR_OK : ERR_INVALID_VALUE);
    }
    MessageParcel empty;
    MessageParcel reply;
    EXPECT_EQ(stub_->HandleSetLaunchServicesEnabled(empty, reply), ERR_INVALID_VALUE);
}

/**
 * @tc.name: ObserverMissing_001
 * @tc.desc: Reject missing observers before dispatch.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceStubTest, ObserverMissing_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    EXPECT_CALL(*stub_, RegisterLaunchServiceObserver(_)).Times(0);
    EXPECT_CALL(*stub_, UnregisterLaunchServiceObserver(_)).Times(0);
    EXPECT_EQ(stub_->HandleRegisterLaunchServiceObserver(data, reply), ERR_INVALID_VALUE);
    EXPECT_EQ(stub_->HandleUnregisterLaunchServiceObserver(data, reply), ERR_INVALID_VALUE);
}

/**
 * @tc.name: InterfaceDefaults_001
 * @tc.desc: Base interface retains default return values and rejects unsupported asynchronous enable.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceStubTest, InterfaceDefaults_001, TestSize.Level1)
{
    AAFwk::LaunchServiceDescriptor daemon;
    AbilityRuntime::LaunchServiceDescriptor descriptor;
    AbilityRuntime::LaunchServiceInformation information;
    std::vector<AbilityRuntime::LaunchServiceInformation> list;
    EXPECT_EQ(stub_->IAppMgr::RegisterDaemon(daemon), ERR_OK);
    EXPECT_EQ(stub_->IAppMgr::UnregisterDaemon(daemon), ERR_OK);
    EXPECT_EQ(stub_->IAppMgr::StartDaemon(daemon), ERR_OK);
    EXPECT_EQ(stub_->IAppMgr::StopDaemon(daemon), ERR_OK);
    EXPECT_EQ(stub_->IAppMgr::GetLaunchServiceInformation(descriptor, information), ERR_OK);
    EXPECT_EQ(stub_->IAppMgr::GetAllRegisteredInformation(100, list), ERR_OK);
    EXPECT_EQ(stub_->IAppMgr::SetLaunchServicesEnabled("bundle", 100, 0, {}, true, nullptr), ERR_INVALID_OPERATION);
    EXPECT_EQ(stub_->IAppMgr::RegisterLaunchServiceObserver(nullptr), ERR_OK);
    EXPECT_EQ(stub_->IAppMgr::UnregisterLaunchServiceObserver(nullptr), ERR_OK);
}

/**
 * @tc.name: ObserverDispatch_001
 * @tc.desc: Deserialize observers and preserve registration and unregistration results.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceStubTest, ObserverDispatch_001, TestSize.Level1)
{
    auto observer = sptr<StubLaunchObserver>::MakeSptr();
    MessageParcel registerData;
    MessageParcel registerReply;
    ASSERT_TRUE(registerData.WriteRemoteObject(observer->AsObject()));
    EXPECT_CALL(*stub_, RegisterLaunchServiceObserver(_)).WillOnce(Return(ERR_INVALID_VALUE));
    ASSERT_EQ(stub_->HandleRegisterLaunchServiceObserver(registerData, registerReply), ERR_OK);
    EXPECT_EQ(registerReply.ReadInt32(), ERR_INVALID_VALUE);
    MessageParcel unregisterData;
    MessageParcel unregisterReply;
    ASSERT_TRUE(unregisterData.WriteRemoteObject(observer->AsObject()));
    EXPECT_CALL(*stub_, UnregisterLaunchServiceObserver(_)).WillOnce(Return(ERR_OK));
    ASSERT_EQ(stub_->HandleUnregisterLaunchServiceObserver(unregisterData, unregisterReply), ERR_OK);
    EXPECT_EQ(unregisterReply.ReadInt32(), ERR_OK);
}
} // namespace OHOS::AppExecFwk
