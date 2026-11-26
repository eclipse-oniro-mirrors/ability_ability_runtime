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

#define private public
#include "mock_process_lifecycle_observer_stub.h"
#include "process_lifecycle_observer_proxy.h"
#undef private

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
namespace {
ProcessData BuildProcessData()
{
    ProcessData processData;
    processData.bundleName = "com.example.lifecycle";
    processData.pid = 1234;
    processData.uid = 5678;
    processData.hostPid = 100;
    processData.gpuPid = 200;
    processData.renderUid = 300;
    processData.state = AppProcessState::APP_STATE_READY;
    processData.isContinuousTask = true;
    processData.isKeepAlive = true;
    processData.isFocused = true;
    processData.requestProcCode = 1;
    processData.processChangeReason = 2;
    processData.processName = "com.example.lifecycle:p1";
    processData.processType = ProcessType::NORMAL;
    processData.extensionType = ExtensionAbilityType::UNSPECIFIED;
    processData.accessTokenId = 400;
    processData.isTestMode = false;
    processData.exitReason = 5;
    processData.exitMsg = "exit msg";
    processData.childUid = -1;
    processData.isPreload = true;
    processData.isPreloadModule = false;
    processData.callerPid = 101;
    processData.callerUid = 102;
    processData.killReason = "kill reason";
    processData.isFromWindowFocusChanged = true;
    processData.preloadMode = 1;
    processData.imageProcessType = 2;
    processData.isPreloadUIExtension = false;
    processData.isFromScreenOffBackground = true;
    processData.byCallStatus = 3;
    processData.callerBundleName = "com.example.caller";
    return processData;
}

void VerifyProcessData(const ProcessData &actual, const ProcessData &expected)
{
    EXPECT_EQ(actual.bundleName, expected.bundleName);
    EXPECT_EQ(actual.pid, expected.pid);
    EXPECT_EQ(actual.uid, expected.uid);
    EXPECT_EQ(actual.hostPid, expected.hostPid);
    EXPECT_EQ(actual.gpuPid, expected.gpuPid);
    EXPECT_EQ(actual.renderUid, expected.renderUid);
    EXPECT_EQ(actual.state, expected.state);
    EXPECT_EQ(actual.isContinuousTask, expected.isContinuousTask);
    EXPECT_EQ(actual.isKeepAlive, expected.isKeepAlive);
    EXPECT_EQ(actual.isFocused, expected.isFocused);
    EXPECT_EQ(actual.requestProcCode, expected.requestProcCode);
    EXPECT_EQ(actual.processChangeReason, expected.processChangeReason);
    EXPECT_EQ(actual.processName, expected.processName);
    EXPECT_EQ(actual.processType, expected.processType);
    EXPECT_EQ(actual.extensionType, expected.extensionType);
    EXPECT_EQ(actual.accessTokenId, expected.accessTokenId);
    EXPECT_EQ(actual.isTestMode, expected.isTestMode);
    EXPECT_EQ(actual.exitReason, expected.exitReason);
    EXPECT_EQ(actual.exitMsg, expected.exitMsg);
    EXPECT_EQ(actual.childUid, expected.childUid);
    EXPECT_EQ(actual.isPreload, expected.isPreload);
    EXPECT_EQ(actual.isPreloadModule, expected.isPreloadModule);
    EXPECT_EQ(actual.callerPid, expected.callerPid);
    EXPECT_EQ(actual.callerUid, expected.callerUid);
    EXPECT_EQ(actual.killReason, expected.killReason);
    EXPECT_EQ(actual.isFromWindowFocusChanged, expected.isFromWindowFocusChanged);
    EXPECT_EQ(actual.preloadMode, expected.preloadMode);
    EXPECT_EQ(actual.imageProcessType, expected.imageProcessType);
    EXPECT_EQ(actual.isPreloadUIExtension, expected.isPreloadUIExtension);
    EXPECT_EQ(actual.isFromScreenOffBackground, expected.isFromScreenOffBackground);
    EXPECT_EQ(actual.byCallStatus, expected.byCallStatus);
    EXPECT_EQ(actual.callerBundleName, expected.callerBundleName);
}
} // namespace

class ProcessLifecycleObserverProxyTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    sptr<ProcessLifecycleObserverProxy> observerProxy_;
    sptr<MockProcessLifecycleObserverStub> mock_;
};

void ProcessLifecycleObserverProxyTest::SetUpTestCase(void) {}

void ProcessLifecycleObserverProxyTest::TearDownTestCase(void) {}

void ProcessLifecycleObserverProxyTest::SetUp()
{
    mock_ = new (std::nothrow) MockProcessLifecycleObserverStub();
    observerProxy_ = new (std::nothrow) ProcessLifecycleObserverProxy(mock_);
}

void ProcessLifecycleObserverProxyTest::TearDown()
{
    observerProxy_ = nullptr;
    mock_ = nullptr;
}

/**
 * @tc.name: WriteInterfaceToken_0100
 * @tc.desc: Write token into parcel data.
 * @tc.type: FUNC
 */
HWTEST_F(ProcessLifecycleObserverProxyTest, WriteInterfaceToken_0100, TestSize.Level1)
{
    MessageParcel data;
    EXPECT_TRUE(observerProxy_->WriteInterfaceToken(data));
}

/**
 * @tc.name: OnProcessCreated_0100
 * @tc.desc: Test OnProcessCreated when the return of WriteInterfaceToken and
 *      WriteParcelable is true and remote is not nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(ProcessLifecycleObserverProxyTest, OnProcessCreated_0100, TestSize.Level1)
{
    EXPECT_CALL(*mock_, OnProcessCreated(_)).Times(1);
    ProcessData processData;
    observerProxy_->OnProcessCreated(processData);
}

/**
 * @tc.name: OnProcessCreated_0200
 * @tc.desc: Test OnProcessCreated when remote is nullptr, no callback and no crash.
 * @tc.type: FUNC
 */
HWTEST_F(ProcessLifecycleObserverProxyTest, OnProcessCreated_0200, TestSize.Level1)
{
    EXPECT_CALL(*mock_, OnProcessCreated(_)).Times(0);
    sptr<ProcessLifecycleObserverProxy> proxy(new (std::nothrow) ProcessLifecycleObserverProxy(nullptr));
    ASSERT_NE(proxy, nullptr);
    ProcessData processData;
    proxy->OnProcessCreated(processData);
}

/**
 * @tc.name: OnProcessCreated_0300
 * @tc.desc: Test OnProcessCreated transfers ProcessData to the stub completely.
 * @tc.type: FUNC
 */
HWTEST_F(ProcessLifecycleObserverProxyTest, OnProcessCreated_0300, TestSize.Level1)
{
    ProcessData expected = BuildProcessData();
    ProcessData received;
    EXPECT_CALL(*mock_, OnProcessCreated(_)).WillOnce(SaveArg<0>(&received));
    observerProxy_->OnProcessCreated(expected);
    VerifyProcessData(received, expected);
}

/**
 * @tc.name: OnProcessDied_0100
 * @tc.desc: Test OnProcessDied when the return of WriteInterfaceToken and
 *      WriteParcelable is true and remote is not nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(ProcessLifecycleObserverProxyTest, OnProcessDied_0100, TestSize.Level1)
{
    EXPECT_CALL(*mock_, OnProcessDied(_)).Times(1);
    ProcessData processData;
    observerProxy_->OnProcessDied(processData);
}

/**
 * @tc.name: OnProcessDied_0200
 * @tc.desc: Test OnProcessDied when remote is nullptr, no callback and no crash.
 * @tc.type: FUNC
 */
HWTEST_F(ProcessLifecycleObserverProxyTest, OnProcessDied_0200, TestSize.Level1)
{
    EXPECT_CALL(*mock_, OnProcessDied(_)).Times(0);
    sptr<ProcessLifecycleObserverProxy> proxy(new (std::nothrow) ProcessLifecycleObserverProxy(nullptr));
    ASSERT_NE(proxy, nullptr);
    ProcessData processData;
    proxy->OnProcessDied(processData);
}

/**
 * @tc.name: OnProcessDied_0300
 * @tc.desc: Test OnProcessDied transfers ProcessData to the stub completely.
 * @tc.type: FUNC
 */
HWTEST_F(ProcessLifecycleObserverProxyTest, OnProcessDied_0300, TestSize.Level1)
{
    ProcessData expected = BuildProcessData();
    ProcessData received;
    EXPECT_CALL(*mock_, OnProcessDied(_)).WillOnce(SaveArg<0>(&received));
    observerProxy_->OnProcessDied(expected);
    VerifyProcessData(received, expected);
}

/**
 * @tc.name: SendRequestInternal_0100
 * @tc.desc: Test SendRequestInternal dispatches ON_PROCESS_CREATED code to the stub.
 * @tc.type: FUNC
 */
HWTEST_F(ProcessLifecycleObserverProxyTest, SendRequestInternal_0100, TestSize.Level1)
{
    EXPECT_CALL(*mock_, OnProcessCreated(_)).Times(1);
    ProcessData processData;
    observerProxy_->SendRequestInternal(
        static_cast<uint32_t>(IProcessLifecycleObserver::Message::ON_PROCESS_CREATED), processData);
}

/**
 * @tc.name: SendRequestInternal_0200
 * @tc.desc: Test SendRequestInternal dispatches ON_PROCESS_DIED code to the stub.
 * @tc.type: FUNC
 */
HWTEST_F(ProcessLifecycleObserverProxyTest, SendRequestInternal_0200, TestSize.Level1)
{
    EXPECT_CALL(*mock_, OnProcessDied(_)).Times(1);
    ProcessData processData;
    observerProxy_->SendRequestInternal(
        static_cast<uint32_t>(IProcessLifecycleObserver::Message::ON_PROCESS_DIED), processData);
}

/**
 * @tc.name: SendRequestInternal_0300
 * @tc.desc: Test SendRequestInternal with an unknown code, no callback and no crash.
 * @tc.type: FUNC
 */
HWTEST_F(ProcessLifecycleObserverProxyTest, SendRequestInternal_0300, TestSize.Level1)
{
    EXPECT_CALL(*mock_, OnProcessCreated(_)).Times(0);
    EXPECT_CALL(*mock_, OnProcessDied(_)).Times(0);
    ProcessData processData;
    observerProxy_->SendRequestInternal(static_cast<uint32_t>(999), processData);
}
} // namespace AppExecFwk
} // namespace OHOS
