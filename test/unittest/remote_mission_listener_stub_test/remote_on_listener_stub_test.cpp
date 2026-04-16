/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#include "mock_remote_on_listener_stub.h"

using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace AAFwk {
class RemoteOnListenerStubTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    void WriteInterfaceToken(MessageParcel& data);
    sptr<MockRemoteOnListenerStub> stub_{ nullptr };
};

void RemoteOnListenerStubTest::SetUpTestCase(void)
{}
void RemoteOnListenerStubTest::TearDownTestCase(void)
{}
void RemoteOnListenerStubTest::SetUp()
{
    stub_ = new MockRemoteOnListenerStub();
}
void RemoteOnListenerStubTest::TearDown()
{}

void RemoteOnListenerStubTest::WriteInterfaceToken(MessageParcel& data)
{
    data.WriteInterfaceToken(MockRemoteOnListenerStub::GetDescriptor());
}

/*
 * Feature: RemoteOnListenerStub
 * Function: OnRemoteRequest
 * SubFunction: NA
 * FunctionPoints: RemoteOnListenerStub OnRemoteRequest
 * EnvConditions: The code which not exist
 * CaseDescription: Verify that on remote request is abnormal
 */
HWTEST_F(RemoteOnListenerStubTest, RemoteOnListenerStubTest_OnRemoteRequest_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    WriteInterfaceToken(data);
    int res = stub_->OnRemoteRequest(10000, data, reply, option);
    EXPECT_EQ(res, IPC_STUB_UNKNOW_TRANS_ERR);
}

/*
 * Feature: RemoteOnListenerStub
 * Function: OnRemoteRequest
 * SubFunction: NA
 * FunctionPoints: RemoteOnListenerStub OnRemoteRequest
 * EnvConditions: Description abnormal
 * CaseDescription: Verify that on remote request is abnormal
 */
HWTEST_F(RemoteOnListenerStubTest, RemoteOnListenerStubTest_OnRemoteRequest_002, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    int res = stub_->OnRemoteRequest(IRemoteOnListener::ON_CALLBACK, data, reply, option);
    EXPECT_EQ(res, ERR_INVALID_STATE);
}

/*
 * Feature: RemoteOnListenerStub
 * Function: OnRemoteRequest
 * SubFunction: NA
 * FunctionPoints: RemoteOnListenerStub OnRemoteRequest
 * EnvConditions: Code is NOTIFY_On_CHANGED
 * CaseDescription: Verify that on remote request is normal
 */
HWTEST_F(RemoteOnListenerStubTest, RemoteOnListenerStubTest_OnRemoteRequest_003, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    WriteInterfaceToken(data);
    EXPECT_CALL(*stub_, OnCallback(_)).Times(1).WillOnce(Return());
    int res = stub_->OnRemoteRequest(IRemoteOnListener::ON_CALLBACK, data, reply, option);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: RemoteOnListenerStub
 * Function: OnCallbackInner
 * SubFunction: NA
 * FunctionPoints: RemoteOnListenerStub OnCallbackInner
 * EnvConditions: Description normal
 * CaseDescription: Verify that on callback inner is normal
 */
HWTEST_F(RemoteOnListenerStubTest, RemoteOnListenerStubTest_OnCallbackInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    EXPECT_CALL(*stub_, OnCallback(_)).Times(1).WillOnce(Return());
    int res = stub_->OnCallbackInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}
}  // namespace AAFwk
}  // namespace OHOS
