/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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
#include "iremote_proxy.h"
#include "want_sender_stub_impl_mock.h"

using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace AAFwk {
class WantSenderStubTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    void WriteInterfaceToken(MessageParcel& data);
    sptr<WantSenderStubImplMock> stub_{ nullptr };
};

void WantSenderStubTest::SetUpTestCase(void)
{}
void WantSenderStubTest::TearDownTestCase(void)
{}
void WantSenderStubTest::TearDown()
{}

void WantSenderStubTest::SetUp()
{
    stub_ = new WantSenderStubImplMock();
}

void WantSenderStubTest::WriteInterfaceToken(MessageParcel& data)
{
    data.WriteInterfaceToken(WantSenderStubImplMock::GetDescriptor());
}

/*
 * Feature: WantSenderStub
 * Function: OnRemoteRequest
 * SubFunction: NA
 * FunctionPoints: WantSenderStub OnRemoteRequest
 * EnvConditions: The code which not exist
 * CaseDescription: Verify that on remote request is abnormal
 */
HWTEST_F(WantSenderStubTest, WantSenderStubTest_OnRemoteRequest_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    WriteInterfaceToken(data);
    int res = stub_->OnRemoteRequest(10000, data, reply, option);

    EXPECT_EQ(res, IPC_STUB_UNKNOW_TRANS_ERR);
}

/*
 * Feature: WantSenderStub
 * Function: OnRemoteRequest
 * SubFunction: NA
 * FunctionPoints: WantSenderStub OnRemoteRequest
 * EnvConditions: code = 0 (legacy WANT_SENDER_SEND, removed)
 * CaseDescription: Regression lock-in. code 0 was the legacy WANT_SENDER_SEND
 *                 trans code that allowed bypassing
 *                 PendingWantManager::CheckCallerPermission via direct
 *                 IWantSender::Send IPC. The trans code enum was deleted along
 *                 with WantSenderStub::OnRemoteRequest override, so code 0 now
 *                 falls through to IPCObjectStub which returns
 *                 IPC_STUB_UNKNOW_TRANS_ERR. This test prevents reintroduction
 *                 of WANT_SENDER_SEND handling without an explicit security
 *                 review.
 */
HWTEST_F(WantSenderStubTest, WantSenderStubTest_OnRemoteRequest_Code0_Rejected_0100, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    WriteInterfaceToken(data);
    int res = stub_->OnRemoteRequest(0, data, reply, option);

    EXPECT_EQ(res, IPC_STUB_UNKNOW_TRANS_ERR);
}

/*
 * Feature: WantSenderStub
 * Function: OnRemoteRequest
 * SubFunction: NA
 * FunctionPoints: WantSenderStub OnRemoteRequest
 * EnvConditions: code = 0, valid interface token + valid SenderInfo payload
 * CaseDescription: Verify code 0 is rejected even when a valid-looking
 *                 SenderInfo payload is attached. Guards against partial
 *                 reintroduction (e.g., re-adding SendInner dispatch on code 0
 *                 based on payload shape rather than trans code definition).
 */
HWTEST_F(WantSenderStubTest, WantSenderStubTest_OnRemoteRequest_Code0_WithPayload_Rejected_0100, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    WriteInterfaceToken(data);
    SenderInfo info;
    data.WriteParcelable(&info);

    int res = stub_->OnRemoteRequest(0, data, reply, option);

    EXPECT_EQ(res, IPC_STUB_UNKNOW_TRANS_ERR);
}
}  // namespace AAFwk
}  // namespace OHOS
