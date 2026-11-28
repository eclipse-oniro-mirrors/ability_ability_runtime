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
#include "app_state_data.h"
#include "application_state_observer_proxy.h"
#include "mock_application_state_observer_stub.h"
#include "peer_holder.h"
#undef private

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
class ApplicationStateObserverProxyTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    sptr<ApplicationStateObserverProxy> observerProxy_;
    sptr<MockApplicationStateObserverStub> mock_;
};

void ApplicationStateObserverProxyTest::SetUpTestCase(void) {}

void ApplicationStateObserverProxyTest::TearDownTestCase(void) {}

void ApplicationStateObserverProxyTest::SetUp()
{
    mock_ = new (std::nothrow) MockApplicationStateObserverStub();
    observerProxy_ = new (std::nothrow) ApplicationStateObserverProxy(mock_);
}

void ApplicationStateObserverProxyTest::TearDown() {}

/**
 * @tc.name: WriteInterfaceToken_0100
 * @tc.desc: Write token into parcel data.
 * @tc.type: FUNC
 */
HWTEST_F(ApplicationStateObserverProxyTest, WriteInterfaceToken_0100, TestSize.Level1)
{
    MessageParcel data;
    EXPECT_TRUE(observerProxy_->WriteInterfaceToken(data));
}

/**
 * @tc.name: OnGamePrelaunchForegroundStateChanged_0100
 * @tc.desc: Test when the return of WriteInterfaceToken and
 *      WriteParcelable is true and remote is not nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(ApplicationStateObserverProxyTest, OnGamePrelaunchForegroundStateChanged_0100, TestSize.Level1)
{
    EXPECT_CALL(*mock_, OnGamePrelaunchForegroundStateChanged(_)).Times(1);
    AppStateData appStateData;
    observerProxy_->OnGamePrelaunchForegroundStateChanged(appStateData);
}
} // namespace AppExecFwk
} // namespace OHOS
