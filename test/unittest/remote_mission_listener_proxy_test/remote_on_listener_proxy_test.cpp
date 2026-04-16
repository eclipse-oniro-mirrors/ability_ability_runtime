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

#define private public
#include "remote_on_listener_proxy.h"
#undef private
#include "remote_on_listener_stub_mock.h"
#include "ipc_types.h"

using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace AAFwk {
class RemoteOnListenerProxyTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    std::shared_ptr<RemoteOnListenerProxy> proxy_ {nullptr};
    sptr<RemoteOnListenerStubMock> mock_ {nullptr};
};

void RemoteOnListenerProxyTest::SetUpTestCase(void)
{}
void RemoteOnListenerProxyTest::TearDownTestCase(void)
{}
void RemoteOnListenerProxyTest::TearDown(void)
{}
void RemoteOnListenerProxyTest::SetUp()
{
    mock_ = new RemoteOnListenerStubMock();
    proxy_ = std::make_shared<RemoteOnListenerProxy>(mock_);
}

/*
 * Feature: RemoteOnListenerProxy
 * Function: OnCallback
 * SubFunction: NA
 * FunctionPoints: RemoteOnListenerProxy OnCallback
 * EnvConditions: NA
 * CaseDescription: Verify OnCallback
 */
HWTEST_F(RemoteOnListenerProxyTest, OnCallback_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &RemoteOnListenerStubMock::InvokeSendRequest));
    OnCallbackInfo info;
    info.continueState = 0;
    info.srcDeviceId = "";
    info.bundleName = "bundleName";
    info.continueType = "continueType";
    info.srcBundleName = "srcBundleName";
    info.appIdentifiers.push_back("appIdentifier");
    proxy_->OnCallback(info);
    EXPECT_EQ(IRemoteOnListener::ON_CALLBACK, mock_->code_);
}

/*
 * Feature: RemoteOnListenerProxyTest
 * Function: OnCallback
 * SubFunction: NA
 * FunctionPoints: RemoteOnListenerProxyTest OnCallback
 * EnvConditions: NA
 * CaseDescription: Verify OnCallback
 */
HWTEST_F(RemoteOnListenerProxyTest, OnCallback_002, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &RemoteOnListenerStubMock::InvokeErrorSendRequest));
    OnCallbackInfo info;
    info.continueState = 0;
    info.srcDeviceId = "test";
    info.bundleName = "bundleName";
    info.continueType = "continueType";
    info.srcBundleName = "srcBundleName";
    info.appIdentifiers.push_back("appIdentifier");
    proxy_->OnCallback(info);
    EXPECT_EQ(IRemoteOnListener::ON_CALLBACK, mock_->code_);
}
}  // namespace AAFwk
}  // namespace OHOS
