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
#define protected public
#include "focused_application_info.h"
#include "focused_application_observer_interface.h"
#include "focused_application_observer_stub.h"
#include "mock_focused_application_observer_stub.h"
#include "parcel.h"
#undef private
#undef protected

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
class FocusedApplicationObserverStubTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void FocusedApplicationObserverStubTest::SetUpTestCase(void) {}
void FocusedApplicationObserverStubTest::TearDownTestCase(void) {}
void FocusedApplicationObserverStubTest::SetUp() {}
void FocusedApplicationObserverStubTest::TearDown() {}

/**
 * @tc.number: HandleOnFocusedApplicationChanged_0100
 * @tc.desc: Test HandleOnFocusedApplicationChanged with valid FocusedApplicationInfo.
 * @tc.type: FUNC
 */
HWTEST_F(FocusedApplicationObserverStubTest, HandleOnFocusedApplicationChanged_0100, TestSize.Level1)
{
    auto stub = new (std::nothrow) MockFocusedApplicationObserverStub();
    ASSERT_NE(stub, nullptr);
    EXPECT_CALL(*stub, OnFocusedApplicationChanged(_)).Times(1);
    MessageParcel data;
    FocusedApplicationInfo focusedApplicationInfo;
    focusedApplicationInfo.focusedUid = 10001;
    focusedApplicationInfo.focusedBundleName = "com.example.focused";
    data.WriteParcelable(&focusedApplicationInfo);
    MessageParcel reply;
    auto result = stub->HandleOnFocusedApplicationChanged(data, reply);
    EXPECT_EQ(NO_ERROR, result);
    testing::Mock::AllowLeak(stub);
}

/**
 * @tc.number: OnRemoteRequest_0100
 * @tc.desc: Test OnRemoteRequest dispatches ON_FOCUSED_APPLICATION_CHANGED.
 * @tc.type: FUNC
 */
HWTEST_F(FocusedApplicationObserverStubTest, OnRemoteRequest_0100, TestSize.Level1)
{
    auto stub = new (std::nothrow) MockFocusedApplicationObserverStub();
    ASSERT_NE(stub, nullptr);
    EXPECT_CALL(*stub, OnFocusedApplicationChanged(_)).Times(1);
    MessageParcel data;
    data.WriteInterfaceToken(IFocusedApplicationObserver::GetDescriptor());
    FocusedApplicationInfo focusedApplicationInfo;
    focusedApplicationInfo.focusedUid = 10001;
    focusedApplicationInfo.focusedBundleName = "com.example.first";
    data.WriteParcelable(&focusedApplicationInfo);
    MessageParcel reply;
    MessageOption option;
    auto result = stub->OnRemoteRequest(
        static_cast<uint32_t>(IFocusedApplicationObserver::Message::ON_FOCUSED_APPLICATION_CHANGED),
        data, reply, option);
    EXPECT_EQ(NO_ERROR, result);
    testing::Mock::AllowLeak(stub);
}
}  // namespace AppExecFwk
}  // namespace OHOS
