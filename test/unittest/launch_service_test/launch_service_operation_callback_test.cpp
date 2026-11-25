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
#include <atomic>
#include <thread>
#include "ability_manager_errors.h"
#include <functional>
#include <memory>
#include <mutex>
#define private public
#include "launch_service_operation_callback.h"
#undef private
#include "message_parcel.h"

using namespace testing::ext;

namespace OHOS {
namespace AbilityRuntime {
class LaunchServiceOperationCallbackTest : public testing::Test {
};

/**
 * @tc.name: Destructor_001
 * @tc.desc: Verify destruction completes a pending request with a service connection error.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceOperationCallbackTest, Destructor_001, TestSize.Level1)
{
    int count = 0;
    int result = 0;
    {
        LaunchServiceOperationCallback callback([&count, &result](int value) {
            ++count;
            result = value;
        });
    }
    EXPECT_EQ(count, 1);
    EXPECT_EQ(result, AAFwk::ABILITY_SERVICE_NOT_CONNECTED);
}

/**
 * @tc.name: WatchRemote_001
 * @tc.desc: Verify a null remote object is rejected.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceOperationCallbackTest, WatchRemote_001, TestSize.Level1)
{
    LaunchServiceOperationCallback callback([](int) {});
    EXPECT_FALSE(callback.WatchRemote(nullptr));
}

/**
 * @tc.name: OnRemoteRequest_001
 * @tc.desc: Verify an invalid interface token does not complete the callback.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceOperationCallbackTest, OnRemoteRequest_001, TestSize.Level1)
{
    int count = 0;
    LaunchServiceOperationCallback callback([&count](int) { ++count; });
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    ASSERT_TRUE(data.WriteInterfaceToken(u"invalid"));
    ASSERT_TRUE(data.WriteInt32(0));
    EXPECT_NE(callback.OnRemoteRequest(0, data, reply, option), 0);
    EXPECT_EQ(count, 0);
}

/**
 * @tc.name: OnRemoteRequest_002
 * @tc.desc: Verify a truncated result does not complete the callback.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceOperationCallbackTest, OnRemoteRequest_002, TestSize.Level1)
{
    int count = 0;
    LaunchServiceOperationCallback callback([&count](int) { ++count; });
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    ASSERT_TRUE(data.WriteInterfaceToken(ILaunchServiceOperationCallback::GetDescriptor()));
    EXPECT_NE(callback.OnRemoteRequest(0, data, reply, option), 0);
    EXPECT_EQ(count, 0);
}
/**
 * @tc.name: OnResult_Concurrent_001
 * @tc.desc: Verify concurrent result delivery and destruction complete only once.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceOperationCallbackTest, OnResult_Concurrent_001, TestSize.Level1)
{
    std::atomic<int> calls{0};
    std::atomic<int> result{-1};
    {
        auto callback = sptr<LaunchServiceOperationCallback>::MakeSptr([&calls, &result](int value) {
            ++calls;
            result = value;
        });
        std::thread first([callback]() { callback->OnResult(0); });
        std::thread second([callback]() { callback->OnResult(123); });
        first.join();
        second.join();
        EXPECT_EQ(calls.load(), 1);
        EXPECT_TRUE(result.load() == 0 || result.load() == 123);
    }
    EXPECT_EQ(calls.load(), 1);
}

/**
 * @tc.name: OnRemoteRequest_Success_003
 * @tc.desc: Verify a valid IPC result is delivered once and unknown codes do not complete.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceOperationCallbackTest, OnRemoteRequest_Success_003, TestSize.Level1)
{
    int calls = 0;
    int result = 0;
    auto callback =
        sptr<LaunchServiceOperationCallback>::MakeSptr([&calls, &result](int value) { ++calls; result = value; });
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    ASSERT_TRUE(data.WriteInterfaceToken(ILaunchServiceOperationCallback::GetDescriptor()));
    ASSERT_TRUE(data.WriteInt32(123));
    EXPECT_EQ(callback->OnRemoteRequest(0, data, reply, option), ERR_OK);
    EXPECT_EQ(calls, 1);
    EXPECT_EQ(result, 123);
    callback->OnResult(456);
    EXPECT_EQ(calls, 1);
}

/**
 * @tc.name: ProxyOnResult_001
 * @tc.desc: Verify callback proxy marshals the result into the local stub.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceOperationCallbackTest, ProxyOnResult_001, TestSize.Level1)
{
    int calls = 0;
    auto callback = sptr<LaunchServiceOperationCallback>::MakeSptr([&calls](int result) {
        ++calls;
        EXPECT_EQ(result, 123);
    });
    LaunchServiceOperationCallbackProxy proxy(callback->AsObject());
    proxy.OnResult(123);
    EXPECT_EQ(calls, 1);
    EXPECT_TRUE(callback->WatchRemote(callback->AsObject()));
}


/**
 * @tc.name: RemoteDied_001
 * @tc.desc: Service death completes a pending operation once and ignores late results.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceOperationCallbackTest, RemoteDied_001, TestSize.Level1)
{
    int32_t calls = 0;
    int32_t result = ERR_OK;
    auto state = std::make_shared<LaunchServiceOperationCallback::ResultState>(
        [&calls, &result](int32_t value) { ++calls; result = value; });
    auto recipient = sptr<LaunchServiceOperationCallback::DeathRecipient>::MakeSptr(state);
    recipient->OnRemoteDied({});
    EXPECT_EQ(calls, 1);
    EXPECT_EQ(result, AAFwk::ABILITY_SERVICE_NOT_CONNECTED);
    state->Complete(ERR_OK);
    recipient->OnRemoteDied({});
    EXPECT_EQ(calls, 1);
}
} // namespace AbilityRuntime
} // namespace OHOS
