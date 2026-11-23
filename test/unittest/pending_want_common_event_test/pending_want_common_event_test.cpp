/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include <atomic>
#include <thread>

#include <gtest/gtest.h>
#define private public
#include "pending_want_common_event.h"
#undef private
#include "want.h"
#include "int_wrapper.h"
#include "wants_info.h"
#include "hilog_tag_wrapper.h"

using namespace testing::ext;
using namespace OHOS::AppExecFwk;
namespace OHOS {
namespace AAFwk {
class PendingWantCommonEventTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void PendingWantCommonEventTest::SetUpTestCase(void)
{}
void PendingWantCommonEventTest::TearDownTestCase(void)
{}
void PendingWantCommonEventTest::SetUp(void)
{}
void PendingWantCommonEventTest::TearDown(void)
{}
class TestWantReceiver : public IWantReceiver {
    virtual void Send(const int32_t resultCode) {}
    virtual void PerformReceive(const Want &want, int resultCode, const std::string &data, const WantParams &extras,
        bool serialized, bool sticky, int sendingUser) {}
    virtual sptr<IRemoteObject> AsObject()
    {
        return nullptr;
    }
};

// Records every PerformReceive invocation so the test can verify the extras
// actually forwarded by PendingWantCommonEvent::OnReceiveEvent.
class RecordingWantReceiver : public IWantReceiver {
public:
    void Send(const int32_t resultCode) override {}
    void PerformReceive(const Want &want, int resultCode, const std::string &data, const WantParams &extras,
        bool serialized, bool sticky, int sendingUser) override
    {
        callCount_++;
        lastExtras_ = extras;
        lastResultCode_ = resultCode;
    }
    sptr<IRemoteObject> AsObject() override
    {
        return nullptr;
    }

    int callCount_ = 0;
    WantParams lastExtras_;
    int lastResultCode_ = -1;
};

// Thread-safe receiver used by the concurrency smoke test: only counts calls.
class CountingWantReceiver : public IWantReceiver {
public:
    void Send(const int32_t resultCode) override {}
    void PerformReceive(const Want &want, int resultCode, const std::string &data, const WantParams &extras,
        bool serialized, bool sticky, int sendingUser) override
    {
        callCount_.fetch_add(1);
    }
    sptr<IRemoteObject> AsObject() override
    {
        return nullptr;
    }

    std::atomic<int> callCount_{0};
};
/*
 * @tc.number: OnReceiveEvent_0100
 * @tc.name: set type
 * @tc.desc: Set RequestWant, use GetRequestWant to verify whether the RequestWant  is set successfully
 */
HWTEST_F(PendingWantCommonEventTest, OnReceiveEvent_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "PendingWantCommonEventTest OnReceiveEvent_0100 start");
    PendingWantCommonEvent PendingWant;
    EventFwk::CommonEventData data;
    PendingWant.OnReceiveEvent(data);
    EXPECT_EQ(PendingWant.finishedReceiver_, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "PendingWantCommonEventTest OnReceiveEvent_0100 end");
}

/*
 * @tc.number: OnReceiveEvent_0200
 * @tc.name: set type
 * @tc.desc: Set RequestWant, use GetRequestWant to verify whether the RequestWant  is set successfully
 */
HWTEST_F(PendingWantCommonEventTest, OnReceiveEvent_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "PendingWantCommonEventTest OnReceiveEvent_0200 start");
    PendingWantCommonEvent PendingWant;
    EventFwk::CommonEventData data;
    PendingWant.finishedReceiver_ = new TestWantReceiver();
    PendingWant.OnReceiveEvent(data);
    EXPECT_NE(PendingWant.finishedReceiver_, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "PendingWantCommonEventTest OnReceiveEvent_0200 end");
}

/*
 * @tc.number: OnReceiveEvent_0300
 * @tc.name: OnReceiveEvent passes the latest wantParams set via SetWantParams
 * @tc.desc: SetWantParams then OnReceiveEvent, verify receiver's extras equal the set params
 */
HWTEST_F(PendingWantCommonEventTest, OnReceiveEvent_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "PendingWantCommonEventTest OnReceiveEvent_0300 start");
    PendingWantCommonEvent pendingWant;
    WantParams wantParams;
    wantParams.SetParam("intKey", AAFwk::Integer::Box(123));
    pendingWant.SetWantParams(wantParams);

    sptr<RecordingWantReceiver> receiver = new RecordingWantReceiver();
    pendingWant.SetFinishedReceiver(receiver);

    EventFwk::CommonEventData data;
    pendingWant.OnReceiveEvent(data);

    EXPECT_EQ(receiver->callCount_, 1);
    EXPECT_EQ(receiver->lastExtras_.GetIntParam("intKey", -1), 123);
    EXPECT_TRUE(receiver->lastExtras_ == wantParams);
    TAG_LOGI(AAFwkTag::TEST, "PendingWantCommonEventTest OnReceiveEvent_0300 end");
}

/*
 * @tc.number: OnReceiveEvent_0400
 * @tc.name: OnReceiveEvent does not call PerformReceive when finished receiver is nullptr
 * @tc.desc: SetFinishedReceiver(nullptr) then OnReceiveEvent, verify PerformReceive not invoked
 */
HWTEST_F(PendingWantCommonEventTest, OnReceiveEvent_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "PendingWantCommonEventTest OnReceiveEvent_0400 start");
    PendingWantCommonEvent pendingWant;
    sptr<RecordingWantReceiver> receiver = new RecordingWantReceiver();
    pendingWant.SetFinishedReceiver(receiver);

    EventFwk::CommonEventData data;
    pendingWant.OnReceiveEvent(data);
    EXPECT_EQ(receiver->callCount_, 1);

    pendingWant.SetFinishedReceiver(nullptr);
    pendingWant.OnReceiveEvent(data);
    EXPECT_EQ(receiver->callCount_, 1);
    TAG_LOGI(AAFwkTag::TEST, "PendingWantCommonEventTest OnReceiveEvent_0400 end");
}

/*
 * @tc.number: OnReceiveEvent_0500
 * @tc.name: concurrent SetWantParams/SetFinishedReceiver/OnReceiveEvent keeps object consistent
 * @tc.desc: Stress setters and OnReceiveEvent in parallel, then verify the object still forwards
 *            the deterministically-set params correctly after the stress
 */
HWTEST_F(PendingWantCommonEventTest, OnReceiveEvent_0500, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "PendingWantCommonEventTest OnReceiveEvent_0500 start");
    PendingWantCommonEvent pendingWant;
    sptr<CountingWantReceiver> countingReceiver = new CountingWantReceiver();
    pendingWant.SetFinishedReceiver(countingReceiver);

    constexpr int loopCount = 200;
    std::thread paramsSetter([&pendingWant]() {
        for (int i = 0; i < loopCount; i++) {
            WantParams params;
            params.SetParam("k", AAFwk::Integer::Box(i));
            pendingWant.SetWantParams(params);
        }
    });
    std::thread receiverSetter([&pendingWant, &countingReceiver]() {
        for (int i = 0; i < loopCount; i++) {
            if (i % 2 == 0) {
                pendingWant.SetFinishedReceiver(countingReceiver);
            } else {
                pendingWant.SetFinishedReceiver(nullptr);
            }
        }
    });
    std::thread eventThread([&pendingWant]() {
        EventFwk::CommonEventData data;
        for (int i = 0; i < loopCount; i++) {
            pendingWant.OnReceiveEvent(data);
        }
    });
    paramsSetter.join();
    receiverSetter.join();
    eventThread.join();

    WantParams finalParams;
    finalParams.SetParam("finalKey", AAFwk::Integer::Box(777));
    pendingWant.SetWantParams(finalParams);
    sptr<RecordingWantReceiver> finalReceiver = new RecordingWantReceiver();
    pendingWant.SetFinishedReceiver(finalReceiver);
    EventFwk::CommonEventData finalData;
    pendingWant.OnReceiveEvent(finalData);

    EXPECT_EQ(finalReceiver->callCount_, 1);
    EXPECT_EQ(finalReceiver->lastExtras_.GetIntParam("finalKey", -1), 777);
    EXPECT_TRUE(finalReceiver->lastExtras_ == finalParams);
    TAG_LOGI(AAFwkTag::TEST, "PendingWantCommonEventTest OnReceiveEvent_0500 end");
}
}  // namespace AAFwk
}  // namespace OHOS
