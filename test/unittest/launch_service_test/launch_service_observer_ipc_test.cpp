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
#include "launch_service_observer.h"
#include "ipc_types.h"
#include "message_parcel.h"
using namespace testing::ext;
namespace OHOS::AbilityRuntime {
class RecordingIPCObserver : public LaunchServiceObserverStub {
public:
    int statusCalls = 0;
    int processCalls = 0;
    LaunchServiceStateData status;
    LaunchServiceProcessStateData process;
    void OnLaunchServiceStatusChanged(const LaunchServiceStateData &value) override
    {
        ++statusCalls;
        status = value;
    }
    void OnLaunchServiceProcessStateChanged(const LaunchServiceProcessStateData &value) override
    {
        ++processCalls;
        process = value;
    }
};
class LaunchServiceObserverIPCTest : public testing::Test {};

/**
 * @tc.name: ProxyRoundTrip_001
 * @tc.desc: Both proxy notifications preserve payload and dispatch to their respective callbacks.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceObserverIPCTest, ProxyRoundTrip_001, TestSize.Level1)
{
    auto observer = sptr<RecordingIPCObserver>::MakeSptr();
    LaunchServiceObserverProxy proxy(observer->AsObject());
    LaunchServiceStateData status;
    status.descriptor.bundleName = "com.example.daemon";
    status.descriptor.moduleName = "entry";
    status.descriptor.serviceName = "worker";
    status.descriptor.userId = 100;
    status.descriptor.appIndex = 2;
    status.state = LaunchServiceState::DISABLED;
    proxy.OnLaunchServiceStatusChanged(status);
    EXPECT_EQ(observer->statusCalls, 1);
    EXPECT_EQ(observer->processCalls, 0);
    EXPECT_EQ(observer->status.state, status.state);
    EXPECT_EQ(observer->status.descriptor.bundleName, status.descriptor.bundleName);
    EXPECT_EQ(observer->status.descriptor.userId, 100);
    EXPECT_EQ(observer->status.descriptor.appIndex, 2);
    LaunchServiceProcessStateData process;
    process.descriptor = status.descriptor;
    process.pid = 123;
    process.state = LaunchServiceProcessState::TERMINATED;
    proxy.OnLaunchServiceProcessStateChanged(process);
    EXPECT_EQ(observer->processCalls, 1);
    EXPECT_EQ(observer->statusCalls, 1);
    EXPECT_EQ(observer->process.pid, 123);
    EXPECT_EQ(observer->process.state, process.state);
    EXPECT_EQ(observer->process.descriptor.serviceName, "worker");
}

/**
 * @tc.name: OnRemoteRequest_Invalid_001
 * @tc.desc: Wrong interface tokens and truncated payloads never invoke observers.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceObserverIPCTest, OnRemoteRequest_Invalid_001, TestSize.Level1)
{
    auto observer = sptr<RecordingIPCObserver>::MakeSptr();
    for (uint32_t code : {ILaunchServiceObserver::ON_STATUS_CHANGED,
        ILaunchServiceObserver::ON_PROCESS_STATE_CHANGED}) {
        for (bool validToken : {false, true}) {
            MessageParcel data;
            MessageParcel reply;
            MessageOption option;
            ASSERT_TRUE(data.WriteInterfaceToken(validToken ? ILaunchServiceObserver::GetDescriptor() : u"invalid"));
            EXPECT_EQ(observer->OnRemoteRequest(code, data, reply, option),
                validToken ? ERR_INVALID_VALUE : ERR_INVALID_STATE);
        }
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    ASSERT_TRUE(data.WriteInterfaceToken(ILaunchServiceObserver::GetDescriptor()));
    EXPECT_NE(observer->OnRemoteRequest(UINT32_MAX, data, reply, option), ERR_OK);
    EXPECT_EQ(observer->statusCalls, 0);
    EXPECT_EQ(observer->processCalls, 0);
}
} // namespace OHOS::AbilityRuntime
