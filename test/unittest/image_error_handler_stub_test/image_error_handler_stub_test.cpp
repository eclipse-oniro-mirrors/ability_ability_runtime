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
#include "iremote_stub.h"
#include "mock_image_error_handler_stub.h"
#include "parcel.h"
#undef private
#undef protected

#include "appexecfwk_errors.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
class ImageErrorHandlerStubTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    sptr<MockImageErrorHandlerStub> handlerStub_ {nullptr};
};

void ImageErrorHandlerStubTest::SetUpTestCase(void)
{}

void ImageErrorHandlerStubTest::TearDownTestCase(void)
{}

void ImageErrorHandlerStubTest::SetUp()
{
    handlerStub_ = new MockImageErrorHandlerStub();
}

void ImageErrorHandlerStubTest::TearDown()
{}

/**
 * @tc.number: HandleOnError_0100
 * @tc.desc: Test when processData is not nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(ImageErrorHandlerStubTest, HandleOnError_0100, TestSize.Level1)
{
    MessageParcel data;
    int32_t errCode = 10;
    data.WriteInt32(errCode);
    MessageParcel reply;
    auto result = handlerStub_->HandleOnError(data, reply);
    EXPECT_EQ(NO_ERROR, result);
    EXPECT_EQ(handlerStub_->errCode_, errCode);
    testing::Mock::AllowLeak(handlerStub_);
}

/**
 * @tc.number: OnRemoteRequest_0100
 * @tc.desc: Test when descriptor and remoteDescriptor is different and
 *      itFunc is not end memberFunc is not nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(ImageErrorHandlerStubTest, OnRemoteRequest_0100, TestSize.Level1)
{
    MessageParcel data;
    std::u16string description = u"123";
    data.WriteInterfaceToken(description);
    MessageParcel reply;
    uint32_t code = 1;
    MessageOption option;
    auto res = handlerStub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(ERR_INVALID_STATE, res);
}

/**
 * @tc.number: OnRemoteRequest_0200
 * @tc.desc: Test when descriptor and remoteDescriptor is same and
 *      code is valid.
 * @tc.type: FUNC
 */
HWTEST_F(ImageErrorHandlerStubTest, OnRemoteRequest_0200, TestSize.Level1)
{
    int32_t errCode = 1;
    MessageParcel data;
    data.WriteInterfaceToken(handlerStub_->GetDescriptor());
    data.WriteInt32(errCode);
    MessageParcel reply;
    uint32_t code = static_cast<uint32_t>(IImageErrorHandler::Message::ON_ERROR);
    MessageOption option;
    auto res = handlerStub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(handlerStub_->errCode_, errCode);
    EXPECT_EQ(NO_ERROR, res);
}

/**
 * @tc.number: OnRemoteRequest_0300
 * @tc.desc: Test when descriptor and remoteDescriptor is same and
 *      code is invalid.
 * @tc.type: FUNC
 */
HWTEST_F(ImageErrorHandlerStubTest, OnRemoteRequest_0300, TestSize.Level1)
{
    int32_t errCode = 0;
    MessageParcel data;
    data.WriteInterfaceToken(handlerStub_->GetDescriptor());
    data.WriteInt32(errCode);
    MessageParcel reply;
    uint32_t code = 100;
    MessageOption option;
    handlerStub_->errCode_ = -1;
    handlerStub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(handlerStub_->errCode_, -1);
    EXPECT_NE(handlerStub_->errCode_, errCode);
}
} // namespace AppExecFwk
} // namespace OHOS