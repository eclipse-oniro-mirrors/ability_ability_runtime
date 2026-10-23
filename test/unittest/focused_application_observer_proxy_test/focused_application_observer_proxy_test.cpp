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
#include "focused_application_observer_proxy.h"
#include "focused_application_observer_stub.h"
#include "parcel.h"
#undef private
#undef protected

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
class FocusedApplicationObserverProxyTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    sptr<FocusedApplicationObserverStub> stub_ {nullptr};
    sptr<FocusedApplicationObserverProxy> proxy_ {nullptr};
};

void FocusedApplicationObserverProxyTest::SetUpTestCase(void) {}
void FocusedApplicationObserverProxyTest::TearDownTestCase(void) {}

class TestFocusedApplicationObserverStub : public FocusedApplicationObserverStub {
public:
    void OnFocusedApplicationChanged(const FocusedApplicationInfo &focusedApplicationInfo) override
    {
        receivedData_ = focusedApplicationInfo;
    }
    FocusedApplicationInfo receivedData_;
};

void FocusedApplicationObserverProxyTest::SetUp()
{
    stub_ = new (std::nothrow) TestFocusedApplicationObserverStub();
    ASSERT_NE(stub_, nullptr);
    proxy_ = new (std::nothrow) FocusedApplicationObserverProxy(stub_);
    ASSERT_NE(proxy_, nullptr);
}

void FocusedApplicationObserverProxyTest::TearDown()
{
    if (proxy_ != nullptr) {
        proxy_ = nullptr;
    }
    if (stub_ != nullptr) {
        stub_ = nullptr;
    }
}

/**
 * @tc.number: OnFocusedApplicationChanged_0100
 * @tc.desc: Test proxy OnFocusedApplicationChanged sends data correctly.
 * @tc.type: FUNC
 */
HWTEST_F(FocusedApplicationObserverProxyTest, OnFocusedApplicationChanged_0100, TestSize.Level1)
{
    FocusedApplicationInfo data;
    data.focusedUid = 10001;
    data.focusedBundleName = "com.example.focused";
    proxy_->OnFocusedApplicationChanged(data);
    auto stub = iface_cast<TestFocusedApplicationObserverStub>(stub_);
    ASSERT_NE(stub, nullptr);
    EXPECT_EQ(stub->receivedData_.focusedUid, 10001);
    EXPECT_EQ(stub->receivedData_.focusedBundleName, "com.example.focused");
}

/**
 * @tc.number: OnFocusedApplicationChanged_0200
 * @tc.desc: Test proxy OnFocusedApplicationChanged with empty focused bundle name.
 * @tc.type: FUNC
 */
HWTEST_F(FocusedApplicationObserverProxyTest, OnFocusedApplicationChanged_0200, TestSize.Level1)
{
    FocusedApplicationInfo data;
    data.focusedUid = 10001;
    data.focusedBundleName = "com.example.first";
    proxy_->OnFocusedApplicationChanged(data);
    auto stub = iface_cast<TestFocusedApplicationObserverStub>(stub_);
    ASSERT_NE(stub, nullptr);
    EXPECT_EQ(stub->receivedData_.focusedUid, 10001);
    EXPECT_EQ(stub->receivedData_.focusedBundleName, "com.example.first");
}
}  // namespace AppExecFwk
}  // namespace OHOS
