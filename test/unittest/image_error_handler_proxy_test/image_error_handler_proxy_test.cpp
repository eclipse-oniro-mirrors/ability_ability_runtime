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
#include "image_error_handler_proxy.h"
#include "mock_image_error_handler_stub.h"
#undef private

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
class ImageErrorHandlerProxyTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    sptr<ImageErrorHandlerProxy> handlerProxy_;
    sptr<MockImageErrorHandlerStub> mock_;
};

void ImageErrorHandlerProxyTest::SetUpTestCase(void) {}

void ImageErrorHandlerProxyTest::TearDownTestCase(void) {}

void ImageErrorHandlerProxyTest::SetUp()
{
    mock_ = new (std::nothrow) MockImageErrorHandlerStub();
    handlerProxy_ = new (std::nothrow) ImageErrorHandlerProxy(mock_);
}

void ImageErrorHandlerProxyTest::TearDown() {}

/**
 * @tc.name: WriteInterfaceToken_0100
 * @tc.desc: Write token into parcel data.
 * @tc.type: FUNC
 */
HWTEST_F(ImageErrorHandlerProxyTest, WriteInterfaceToken_0100, TestSize.Level1)
{
    MessageParcel data;
    EXPECT_TRUE(handlerProxy_->WriteInterfaceToken(data));
}

/**
 * @tc.name: OnError_0100
 * @tc.desc: Test when the return of WriteInterfaeToken and
 *      WriteParcelable is true and remote is not nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(ImageErrorHandlerProxyTest, OnError_0100, TestSize.Level1)
{
    EXPECT_CALL(*mock_, OnError(_)).Times(1);
    int32_t errCode = 0;
    handlerProxy_->OnError(errCode);
}
} // namespace AppExecFwk
} // namespace OHOS