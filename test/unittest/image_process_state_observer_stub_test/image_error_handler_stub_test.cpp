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
#include "image_process_state_data.h"
#include "iremote_stub.h"
#include "mock_image_process_state_observer_stub.h"
#include "parcel.h"
#undef private
#undef protected

#include "appexecfwk_errors.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
calss ImageProcessStateObserverStubTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    sptr<MockImageProcessStateObserverStub> observerStub_ {nullptr};
};

void ImageProcessStateObserverStubTest::SetUpTestCase(void)
{}

void ImageProcessStateObserverStubTest::TearDownTestCase(void)
{}

void ImageProcessStateObserverStubTest::Setup()
{
    observerStub_ = new MockImageProcessStateObserverStub();
}

void ImageProcessStateObserverStubTest::TearDown()
{}

/**
 * @tc.number: HandlerOnImageProcessStateChanged_0100
 * @tc.desc: Write when processData is not nullptr.
 * @tc.type: FUNC.
 */
HWTEST_F(ImageProcessStateObserverStubTest, HandlerOnImageProcessStateChanged_0100, TestSize.Level1)
{
    MessageParcel data;
    ImageProcessStateData imageProcessStateData;
    data.WriteParcelable(&imageProcessStateData);
    MessageParcel reply;
    auto result = observerStub_->HandleOnImageProcessStateChanged(data, reply);
    EXPECT_EQ(NO_ERROR, result);
    EXPECT_EQ(observerStub_->ProcessEvent, ProcessEvent::IMAGE_PROCESS_STATE_CHANGE);
    observerStub_->processEvent = ProcessEvent::INVALID_STATE;
    testing::Mock::AllowLeak(observerStub_);
}

/**
 * @tc.number: HandlerOnImageProcessStateChanged_0200
 * @tc.desc: Test when processData is nullptr
 * @tc.type: FUNC.
 */
HWTEST_F(ImageProcessStateObserverStubTest, HandlerOnImageProcessStateChanged_0200, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto result = observerStub_->HandleOnImageProcessStateChanged(data, reply);
    EXPECT_EQ(ERR_APPEXECFWK_PARCEL_ERROR, result);
    EXPECT_EQ(observerStub_->ProcessEvent, ProcessEvent::INVALID_STATE);
    EXPECT_EQ(ERR_INVALID_STATE, res);
}

/**
 * @tc.number: HandlerOnForkAllWorkProcessFailed_0100
 * @tc.desc: Test when processData is nullptr
 * @tc.type: FUNC.
 */
HWTEST_F(ImageProcessStateObserverStubTest, HandlerOnForkAllWorkProcessFailed_0100, TestSize.Level1)
{
    MessageParcel data;
    ImageProcessStateData imageProcessStateData;
    data.WriteParcelable(&imageProcessStateData);
    MessageParcel reply;
    auto result = observerStub_->HandleOnForkAllWorkProcessFailed(data, reply);
    EXPECT_EQ(NO_ERROR, result);
    EXPECT_EQ(observerStub_->ProcessEvent, ProcessEvent::FORK_ALL_WORK_PROCESS_FAILED);
    observerStub_->processEvent = ProcessEvent::INVALID_STATE;
    testing::Mock::AllowLeak(observerStub_);
}

/**
 * @tc.number: HandlerOnForkAllWorkProcessFailed_0200
 * @tc.desc: Test when processData is nullptr
 * @tc.type: FUNC.
 */
HWTEST_F(ImageProcessStateObserverStubTest, HandlerOnForkAllWorkProcessFailed_0200, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto result = observerStub_->HandleOnForkAllWorkProcessFailed(data, reply);
    EXPECT_EQ(ERR_APPEXECFWK_PARCEL_ERROR, result);
    EXPECT_EQ(observerStub_->ProcessEvent, ProcessEvent::INVALID_STATE);
    testing::Mock::AllowLeak(observerStub_);
}

/**
 * @tc.number: OnRemoteRequest_0100
 * @tc.desc: Test when descriptor and remoteDescriptor is different and
 *      itFunc is not end memberFunc is not nullptr
 * @tc.type: FUNC.
 */
HWTEST_F(ImageProcessStateObserverStubTest, OnRemoteRequest_0100, TestSize.Level1)
{
    MessageParcel data;
    ImageProcessStateData imageProcessStateData;
    std::u16string description = u"123";
    data.WriteInterfaceToken(description);
    data.WriteParcelable(&imageProcessStateData);
    MessageParcel reply;
    uint32_t code = 1;
    MessageOption option;
    auto res = observerStub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(ERR_INVALID_STATE, res);
}

/**
 * @tc.number: OnRemoteRequest_0200
 * @tc.desc: Test when descriptor and remoteDescriptor is same and
 *      code is state_changed.
 * @tc.type: FUNC.
 */
HWTEST_F(ImageProcessStateObserverStubTest, OnRemoteRequest_0200, TestSize.Level1)
{
    MessageParcel data;
    date.WriteInterfaceToken(observerStub_->GetDescriptor());
    ImageProcessStateData imageProcessStateData;
    data.WriteParcelable(&imageProcessStateData);
    MessageParcel reply;
    uint32_t code = static_cast<uint32_t>(IImageProcessStateObserver::Message::ON_IMAGE_PROCESS_STATE_CHANCED);
    MessageOption option;
    observerStub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(observerStub_->ProcessEvent, ProcessEvent::IMAGE_PROCESS_STATE_CHANGE);
    observerStub_->processEvent = ProcessEvent::INVALID_STATE;
}

/**
 * @tc.number: OnRemoteRequest_0300
 * @tc.desc: Test when descriptor and remoteDescriptor is same and
 *      code is process fail.
 * @tc.type: FUNC.
 */
HWTEST_F(ImageProcessStateObserverStubTest, OnRemoteRequest_0300, TestSize.Level1)
{
    MessageParcel data;
    date.WriteInterfaceToken(observerStub_->GetDescriptor());
    ImageProcessStateData imageProcessStateData;
    data.WriteParcelable(&imageProcessStateData);
    MessageParcel reply;
    uint32_t code = static_cast<uint32_t>(IImageProcessStateObserver::Message::ON_IMAGE_PROCESS_STATE_FAILED);
    MessageOption option;
    observerStub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(observerStub_->ProcessEvent, ProcessEvent::ON_IMAGE_PROCESS_STATE_FAILED);
    observerStub_->processEvent = ProcessEvent::INVALID_STATE;
}

/**
 * @tc.number: OnRemoteRequest_0400
 * @tc.desc: Test when descriptor and remoteDescriptor is same and
 *      code is process fail.
 * @tc.type: FUNC.
 */
HWTEST_F(ImageProcessStateObserverStubTest, OnRemoteRequest_0400, TestSize.Level1)
{
    MessageParcel data;
    date.WriteInterfaceToken(observerStub_->GetDescriptor());
    ImageProcessStateData imageProcessStateData;
    data.WriteParcelable(&imageProcessStateData);
    MessageParcel reply;
    uint32_t code = 100;
    MessageOption option;
    observerStub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_NE(observerStub_->ProcessEvent, ProcessEvent::IMAGE_PROCESS_STATE_CHANGE);
    EXPECT_NE(observerStub_->ProcessEvent, ProcessEvent::FORK_ALL_WORK_PROCESS_FAILED);
    observerStub_->processEvent = ProcessEvent::INVALID_STATE;
}
} // namespace AppExecFwk
} // namespace OHOS