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
#include "launch_service_operation_callback.h"
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
        LaunchServiceOperationCallback callback([&](int value) {
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
    LaunchServiceOperationCallback callback([&](int) { ++count; });
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
    LaunchServiceOperationCallback callback([&](int) { ++count; });
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    ASSERT_TRUE(data.WriteInterfaceToken(ILaunchServiceOperationCallback::GetDescriptor()));
    EXPECT_NE(callback.OnRemoteRequest(0, data, reply, option), 0);
    EXPECT_EQ(count, 0);
}
} // namespace AbilityRuntime
} // namespace OHOS
