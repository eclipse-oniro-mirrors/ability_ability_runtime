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
#include "image_process_state_observer_proxy.h"
#include "mock_image_process_state_observer_stub.h"
#undef private

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
class ImageProcessStateObserverProxyTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    sptr<ImageProcessStateObserverProxy> observerProxy_;
    sptr<MockImageProcessStateObserverStub> mock_;
};

void ImageProcessStateObserverProxyTest::SetUpTestCase(void) {}

void ImageProcessStateObserverProxyTest::TearDownTestCase(void) {}

void ImageProcessStateObserverProxyTest::SetUp()
{
    mock_ = new (std::nothrow) MockImageProcessStateObserverStub();
    observerProxy_ = new (std::nothrow) ImageProcessStateObserverProxy(mock_);
}

void ImageProcessStateObserverProxyTest::TearDown() {}

/**
 * @tc.name: WriteInterfaceToken_0100
 * @tc.desc: Write token into parcel data.
 * @tc.type: FUNC
 */
HWTEST_F(ImageProcessStateObserverProxyTest, WriteInterfaceToken_0100, TestSize.Level1)
{
    MessageParcel data;
    EXPECT_TRUE(observerProxy_->WriteInterfaceToken(data));
}

/**
 * @tc.name: OnImageProcessStateChanged_0100
 * @tc.desc: Test when the return of WriteInterfaceToken and
 *      WriteParcelable is true and remote is not nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(ImageProcessStateObserverProxyTest, OnImageProcessStateChanged_0100, TestSize.Level1)
{
    EXPECT_CALL(*mock_, OnImageProcessStateChanged(_)).Times(1);
    ImageProcessStateData imageProcessStateData;
    observerProxy_->OnImageProcessStateChanged(imageProcessStateData);
}

/**
 * @tc.name: OnForkAllWorkProcessFailed_0100
 * @tc.desc: Test when the return of WriteInterfaeToken and
 *      WriteParcelable is true and remote is not nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(ImageProcessStateObserverProxyTest, OnForkAllWorkProcessFailed_0100, TestSize.Level1)
{
    EXPECT_CALL(*mock_, OnForkAllWorkProcessFailed(_, _)).Times(1);
    ImageProcessStateData imageProcessStateData;
    int32_t errCode = 0;
    observerProxy_->OnForkAllWorkProcessFailed(imageProcessStateData, errCode);
}
} // namespace AppExecFwk
} // namespace OHOS