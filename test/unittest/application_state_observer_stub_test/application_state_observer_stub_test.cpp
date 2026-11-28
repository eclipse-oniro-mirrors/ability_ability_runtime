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

// NOLINTNEXTLINE(g.nam.03-cpp): Redefining C++ keyword 'private' for unit testing
#define private public
// NOLINTNEXTLINE(g.nam.03-cpp): Redefining C++ keyword 'protected' for unit testing
#define protected public
#include "app_state_data.h"
#include "iremote_stub.h"
#include "mock_application_state_observer_stub.h"
#include "parcel.h"
#undef private
#undef protected

#include "appexecfwk_errors.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
class ApplicationStateObserverStubTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    sptr<MockApplicationStateObserverStub> observerStub_ {nullptr};
};

void ApplicationStateObserverStubTest::SetUpTestCase(void)
{}

void ApplicationStateObserverStubTest::TearDownTestCase(void)
{}

void ApplicationStateObserverStubTest::SetUp()
{
    observerStub_ = new (std::nothrow) MockApplicationStateObserverStub();
}

void ApplicationStateObserverStubTest::TearDown()
{}

/**
 * @tc.number: HandleOnGamePrelaunchForegroundStateChanged_0100
 * @tc.desc: Test when appStateData is not nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(ApplicationStateObserverStubTest, HandleOnGamePrelaunchForegroundStateChanged_0100, TestSize.Level1)
{
    EXPECT_NE(observerStub_, nullptr);
    EXPECT_CALL(*observerStub_, OnGamePrelaunchForegroundStateChanged(_)).Times(1);
    MessageParcel data;
    AppStateData appStateData;
    data.WriteParcelable(&appStateData);
    MessageParcel reply;
    auto result = observerStub_->HandleOnGamePrelaunchForegroundStateChanged(data, reply);
    EXPECT_EQ(NO_ERROR, result);
    testing::Mock::AllowLeak(observerStub_);
}

/**
 * @tc.number: HandleOnGamePrelaunchForegroundStateChanged_0200
 * @tc.desc: Test when appStateData is nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(ApplicationStateObserverStubTest, HandleOnGamePrelaunchForegroundStateChanged_0200, TestSize.Level1)
{
    EXPECT_NE(observerStub_, nullptr);
    EXPECT_CALL(*observerStub_, OnGamePrelaunchForegroundStateChanged(_)).Times(0);
    MessageParcel data;
    MessageParcel reply;
    auto result = observerStub_->HandleOnGamePrelaunchForegroundStateChanged(data, reply);
    EXPECT_EQ(ERR_APPEXECFWK_PARCEL_ERROR, result);
    testing::Mock::AllowLeak(observerStub_);
}

/**
 * @tc.number: OnRemoteRequest_OnGamePrelaunchForegroundStateChanged_0100
 * @tc.desc: Test OnRemoteRequest dispatches to HandleOnGamePrelaunchForegroundStateChanged.
 * @tc.type: FUNC
 */
HWTEST_F(ApplicationStateObserverStubTest, OnRemoteRequest_OnGamePrelaunchForegroundStateChanged_0100, TestSize.Level1)
{
    EXPECT_NE(observerStub_, nullptr);
    EXPECT_CALL(*observerStub_, OnGamePrelaunchForegroundStateChanged(_)).Times(1);
    MessageParcel data;
    data.WriteInterfaceToken(ApplicationStateObserverStub::GetDescriptor());
    AppStateData appStateData;
    data.WriteParcelable(&appStateData);
    MessageParcel reply;
    MessageOption option;
    auto result = observerStub_->OnRemoteRequest(
        static_cast<uint32_t>(IApplicationStateObserver::Message::TRANSACT_ON_GAME_PRELAUNCH_FOREGROUND_STATE_CHANGED),
        data, reply, option);
    EXPECT_EQ(NO_ERROR, result);
    testing::Mock::AllowLeak(observerStub_);
}
} // namespace AppExecFwk
} // namespace OHOS
