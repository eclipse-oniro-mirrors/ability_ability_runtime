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
class ImageProcessStateObserverStubTest : public testing::Test {
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

void ImageProcessStateObserverStubTest::SetUp()
{
    observerStub_ = new MockImageProcessStateObserverStub();
}

void ImageProcessStateObserverStubTest::TearDown()
{}

/**
 * @tc.number: HandleOnImageProcessStateChanged_0100
 * @tc.desc: Test when processData is not nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(ImageProcessStateObserverStubTest, HandleOnImageProcessStateChanged_0100, TestSize.Level1)
{
    MessageParcel data;
    ImageProcessStateData imageProcessStateData;
    data.WriteParcelable(&imageProcessStateData);
    MessageParcel reply;
    auto result = observerStub_->HandleOnImageProcessStateChanged(data, reply);
    EXPECT_EQ(NO_ERROR, result);
    EXPECT_EQ(observerStub_->processEvent, ProcessEvent::IMAGE_PROCESS_STATE_CHANGE);
    observerStub_->processEvent = ProcessEvent::INVALID_STATE;
    testing::Mock::AllowLeak(observerStub_);
}

/**
 * @tc.number: HandleOnImageProcessStateChanged_0200
 * @tc.desc: Test when processData is nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(ImageProcessStateObserverStubTest, HandleOnImageProcessStateChanged_0200, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto result = observerStub_->HandleOnImageProcessStateChanged(data, reply);
    EXPECT_EQ(ERR_APPEXECFWK_PARCEL_ERROR, result);
    EXPECT_EQ(observerStub_->processEvent, ProcessEvent::INVALID_STATE);
    testing::Mock::AllowLeak(observerStub_);
}

/**
 * @tc.number: HandleOnForkAllWorkProcessFailed_0100
 * @tc.desc: Test when processData is not nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(ImageProcessStateObserverStubTest, HandleOnForkAllWorkProcessFailed_0100, TestSize.Level1)
{
    MessageParcel data;
    ImageProcessStateData imageProcessStateData;
    data.WriteParcelable(&imageProcessStateData);
    MessageParcel reply;
    auto result = observerStub_->HandleOnForkAllWorkProcessFailed(data, reply);
    EXPECT_EQ(NO_ERROR, result);
    EXPECT_EQ(observerStub_->processEvent, ProcessEvent::FORK_ALL_WORK_PROCESS_FAILED);
    observerStub_->processEvent = ProcessEvent::INVALID_STATE;
    testing::Mock::AllowLeak(observerStub_);
}

/**
 * @tc.number: HandleOnForkAllWorkProcessFailed_0200
 * @tc.desc: Test when processData is nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(ImageProcessStateObserverStubTest, HandleOnForkAllWorkProcessFailed_0200, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto result = observerStub_->HandleOnForkAllWorkProcessFailed(data, reply);
    EXPECT_EQ(ERR_APPEXECFWK_PARCEL_ERROR, result);
    EXPECT_EQ(observerStub_->processEvent, ProcessEvent::INVALID_STATE);
    testing::Mock::AllowLeak(observerStub_);
}

/**
 * @tc.number: OnRemoteRequest_0100
 * @tc.desc: Test when descriptor and remoteDescriptor is different and
 *      itFunc is not end memberFunc is not nullptr.
 * @tc.type: FUNC
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
 * @tc.type: FUNC
 */
HWTEST_F(ImageProcessStateObserverStubTest, OnRemoteRequest_0200, TestSize.Level1)
{
    MessageParcel data;
    data.WriteInterfaceToken(observerStub_->GetDescriptor());
    ImageProcessStateData imageProcessStateData;
    data.WriteParcelable(&imageProcessStateData);
    MessageParcel reply;
    uint32_t code = static_cast<uint32_t>(IImageProcessStateObserver::Message::ON_IMAGE_PROCESS_STATE_CHANGED);
    MessageOption option;
    observerStub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(observerStub_->processEvent, ProcessEvent::IMAGE_PROCESS_STATE_CHANGE);
    observerStub_->processEvent = ProcessEvent::INVALID_STATE;
}

/**
 * @tc.number: OnRemoteRequest_0300
 * @tc.desc: Test when descriptor and remoteDescriptor is same and
 *      code is process fail.
 * @tc.type: FUNC
 */
HWTEST_F(ImageProcessStateObserverStubTest, OnRemoteRequest_0300, TestSize.Level1)
{
    MessageParcel data;
    data.WriteInterfaceToken(observerStub_->GetDescriptor());
    ImageProcessStateData imageProcessStateData;
    data.WriteParcelable(&imageProcessStateData);
    MessageParcel reply;
    uint32_t code = static_cast<uint32_t>(IImageProcessStateObserver::Message::ON_FORKALL_WORK_PROCESS_FAILED);
    MessageOption option;
    observerStub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(observerStub_->processEvent, ProcessEvent::FORK_ALL_WORK_PROCESS_FAILED);
    observerStub_->processEvent = ProcessEvent::INVALID_STATE;
}

/**
 * @tc.number: OnRemoteRequest_0400
 * @tc.desc: Test when descriptor and remoteDescriptor is same and
 *      code is invalid.
 * @tc.type: FUNC
 */
HWTEST_F(ImageProcessStateObserverStubTest, OnRemoteRequest_0400, TestSize.Level1)
{
    MessageParcel data;
    data.WriteInterfaceToken(observerStub_->GetDescriptor());
    ImageProcessStateData imageProcessStateData;
    data.WriteParcelable(&imageProcessStateData);
    MessageParcel reply;
    uint32_t code = 100;
    MessageOption option;
    observerStub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_NE(observerStub_->processEvent, ProcessEvent::IMAGE_PROCESS_STATE_CHANGE);
    EXPECT_NE(observerStub_->processEvent, ProcessEvent::FORK_ALL_WORK_PROCESS_FAILED);
    observerStub_->processEvent = ProcessEvent::INVALID_STATE;
}
} // namespace AppExecFwk
} // namespace OHOS