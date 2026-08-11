/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "ability_manager_errors.h"
#include "global_constant.h"
#include "hilog_tag_wrapper.h"
#include "insight_intent_execute_result.h"
#include "ipc_skeleton.h"
#include "want.h"
#include "insight_intent_execute_callback_interface.h"
#include "insight_intent_execute_callback_stub.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AAFwk {
namespace {
const std::u16string APPMGR_INTERFACE_TOKEN = u"ohos.AAFwk.IntentExecuteCallback";
// Foundation process uid used for trust dispatch; references the shared codebase
// constant so the test does not carry an independent literal that can drift.
constexpr int32_t FOUNDATION_UID = OHOS::AbilityRuntime::GlobalConstant::FOUNDATION_UID;
}  // namespace

// Concrete subclass that satisfies the pure-virtual OnExecuteDone so the stub can
// be instantiated. Trust dispatch is driven entirely by mock IPCSkeleton state in
// each test case (SetCallingUid), not by subclassing. OnExecuteDone captures the
// forwarded (key, resultCode, executeResult.code) so success-path cases can verify
// the stub actually forwards the deserialized payload to the callback.
class InsightIntentExecuteCallbackStubTests : public InsightIntentExecuteCallbackStub {
public:
    InsightIntentExecuteCallbackStubTests() = default;
    virtual ~InsightIntentExecuteCallbackStubTests() = default;
    void OnExecuteDone(uint64_t key, int32_t resultCode,
        const AppExecFwk::InsightIntentExecuteResult &executeResult) override
    {
        onExecuteDoneCalled_ = true;
        lastKey_ = key;
        lastResultCode_ = resultCode;
        lastExecuteResultCode_ = executeResult.code;
    }

    bool IsOnExecuteDoneCalled() const { return onExecuteDoneCalled_; }
    uint64_t GetLastKey() const { return lastKey_; }
    int32_t GetLastResultCode() const { return lastResultCode_; }
    int32_t GetLastExecuteResultCode() const { return lastExecuteResultCode_; }

private:
    bool onExecuteDoneCalled_ = false;
    uint64_t lastKey_ = 0;
    int32_t lastResultCode_ = 0;
    int32_t lastExecuteResultCode_ = 0;
};

class InsightIntentExecuteCallbackStubTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void InsightIntentExecuteCallbackStubTest::SetUpTestCase(void)
{}

void InsightIntentExecuteCallbackStubTest::TearDownTestCase(void)
{}

void InsightIntentExecuteCallbackStubTest::SetUp()
{
    // Default to a trusted (foundation) caller; individual cases that exercise the
    // rejection path override this with a non-foundation uid.
    IPCSkeleton::SetCallingUid(FOUNDATION_UID);
}

void InsightIntentExecuteCallbackStubTest::TearDown()
{
    IPCSkeleton::SetCallingUid(0);
}

/**
 * @tc.name: OnRemoteRequest_0100
 * @tc.name: OnRemoteRequest
 * @tc.desc: Test OnRemoteRequest.
 */
HWTEST_F(InsightIntentExecuteCallbackStubTest, OnRemoteRequest_0100, TestSize.Level1)
{
    TAG_LOGE(AAFwkTag::TEST, "OnRemoteRequest_0100 begin.");
    std::shared_ptr<InsightIntentExecuteCallbackStub> backStub
        = std::make_shared<InsightIntentExecuteCallbackStubTests>();
    uint32_t code = IInsightIntentExecuteCallback::ON_INSIGHT_INTENT_EXECUTE_DONE;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(APPMGR_INTERFACE_TOKEN);
    int32_t result = backStub->OnRemoteRequest(code, data, reply, option);
    EXPECT_NE(result, ERR_OK);
    code = 0;
    MessageParcel data1;
    result = backStub->OnRemoteRequest(code, data1, reply, option);
    EXPECT_EQ(result, ERR_INVALID_STATE);
    TAG_LOGE(AAFwkTag::TEST, "OnRemoteRequest_0100 end.");
}

/**
 * @tc.name: OnRemoteRequest_0200
 * @tc.desc: Reject OnRemoteRequest when caller is not the foundation process.
 */
HWTEST_F(InsightIntentExecuteCallbackStubTest, OnRemoteRequest_0200, TestSize.Level1)
{
    TAG_LOGE(AAFwkTag::TEST, "OnRemoteRequest_0200 begin.");
    IPCSkeleton::SetCallingUid(0);  // simulate a non-foundation caller
    std::shared_ptr<InsightIntentExecuteCallbackStub> backStub
        = std::make_shared<InsightIntentExecuteCallbackStubTests>();
    uint32_t code = IInsightIntentExecuteCallback::ON_INSIGHT_INTENT_EXECUTE_DONE;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(APPMGR_INTERFACE_TOKEN);
    int32_t result = backStub->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(result, CHECK_PERMISSION_FAILED);
    TAG_LOGE(AAFwkTag::TEST, "OnRemoteRequest_0200 end.");
}

/**
 * @tc.name: OnExecuteDoneInner_0100
 * @tc.name: OnExecuteDoneInner
 * @tc.desc: Test OnExecuteDoneInner.
 */
HWTEST_F(InsightIntentExecuteCallbackStubTest, OnExecuteDoneInner_0100, TestSize.Level1)
{
    TAG_LOGE(AAFwkTag::TEST, "OnExecuteDoneInner_0100 begin.");
    std::shared_ptr<InsightIntentExecuteCallbackStub> backStub
        = std::make_shared<InsightIntentExecuteCallbackStubTests>();
    MessageParcel data;
    MessageParcel reply;
    int32_t result = backStub->OnExecuteDoneInner(data, reply);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    TAG_LOGE(AAFwkTag::TEST, "OnExecuteDoneInner_0100 end.");
}

/**
 * @tc.name: OnExecuteDoneInner_0200
 * @tc.desc: Test OnExecuteDoneInner success path with a complete parcel (key + resultCode + result).
 */
HWTEST_F(InsightIntentExecuteCallbackStubTest, OnExecuteDoneInner_0200, TestSize.Level1)
{
    TAG_LOGE(AAFwkTag::TEST, "OnExecuteDoneInner_0200 begin.");
    auto backStub = std::make_shared<InsightIntentExecuteCallbackStubTests>();
    MessageParcel data;
    MessageParcel reply;
    data.WriteUint64(456);
    data.WriteInt32(0);
    AppExecFwk::InsightIntentExecuteResult executeResult;
    executeResult.code = 4567;  // distinct value to verify executeResult round-trips into the callback
    data.WriteParcelable(&executeResult);
    int32_t result = backStub->OnExecuteDoneInner(data, reply);
    EXPECT_EQ(result, ERR_OK);
    // Verify the stub forwarded (key, resultCode, executeResult.code) to OnExecuteDone.
    EXPECT_TRUE(backStub->IsOnExecuteDoneCalled());
    EXPECT_EQ(backStub->GetLastKey(), static_cast<uint64_t>(456));
    EXPECT_EQ(backStub->GetLastResultCode(), 0);
    EXPECT_EQ(backStub->GetLastExecuteResultCode(), 4567);
    TAG_LOGE(AAFwkTag::TEST, "OnExecuteDoneInner_0200 end.");
}

/**
 * @tc.name: OnRemoteRequest_0300
 * @tc.desc: Test OnRemoteRequest success path (valid token + foundation uid + target code + complete parcel).
 */
HWTEST_F(InsightIntentExecuteCallbackStubTest, OnRemoteRequest_0300, TestSize.Level1)
{
    TAG_LOGE(AAFwkTag::TEST, "OnRemoteRequest_0300 begin.");
    IPCSkeleton::SetCallingUid(FOUNDATION_UID);
    auto backStub = std::make_shared<InsightIntentExecuteCallbackStubTests>();
    uint32_t code = IInsightIntentExecuteCallback::ON_INSIGHT_INTENT_EXECUTE_DONE;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(APPMGR_INTERFACE_TOKEN);
    data.WriteUint64(789);
    data.WriteInt32(0);
    AppExecFwk::InsightIntentExecuteResult executeResult;
    executeResult.code = 7890;  // distinct value to verify executeResult round-trips into the callback
    data.WriteParcelable(&executeResult);
    int32_t result = backStub->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(result, ERR_OK);
    // Verify the full dispatch path forwarded (key, resultCode, executeResult.code) to OnExecuteDone.
    EXPECT_TRUE(backStub->IsOnExecuteDoneCalled());
    EXPECT_EQ(backStub->GetLastKey(), static_cast<uint64_t>(789));
    EXPECT_EQ(backStub->GetLastResultCode(), 0);
    EXPECT_EQ(backStub->GetLastExecuteResultCode(), 7890);
    TAG_LOGE(AAFwkTag::TEST, "OnRemoteRequest_0300 end.");
}

/**
 * @tc.name: OnRemoteRequest_0400
 * @tc.desc: Test OnRemoteRequest with valid token + foundation uid but non-target code (falls back to base).
 */
HWTEST_F(InsightIntentExecuteCallbackStubTest, OnRemoteRequest_0400, TestSize.Level1)
{
    TAG_LOGE(AAFwkTag::TEST, "OnRemoteRequest_0400 begin.");
    IPCSkeleton::SetCallingUid(FOUNDATION_UID);
    std::shared_ptr<InsightIntentExecuteCallbackStub> backStub
        = std::make_shared<InsightIntentExecuteCallbackStubTests>();
    uint32_t code = 9999;  // not ON_INSIGHT_INTENT_EXECUTE_DONE
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(APPMGR_INTERFACE_TOKEN);
    int32_t result = backStub->OnRemoteRequest(code, data, reply, option);
    EXPECT_NE(result, ERR_INVALID_STATE);
    EXPECT_NE(result, CHECK_PERMISSION_FAILED);
    TAG_LOGE(AAFwkTag::TEST, "OnRemoteRequest_0400 end.");
}

/**
 * @tc.name: OnRemoteRequest_0500
 * @tc.desc: Test OnRemoteRequest with wrong interface token (mismatch → ERR_INVALID_STATE).
 */
HWTEST_F(InsightIntentExecuteCallbackStubTest, OnRemoteRequest_0500, TestSize.Level1)
{
    TAG_LOGE(AAFwkTag::TEST, "OnRemoteRequest_0500 begin.");
    IPCSkeleton::SetCallingUid(FOUNDATION_UID);
    std::shared_ptr<InsightIntentExecuteCallbackStub> backStub
        = std::make_shared<InsightIntentExecuteCallbackStubTests>();
    uint32_t code = IInsightIntentExecuteCallback::ON_INSIGHT_INTENT_EXECUTE_DONE;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(u"ohos.AAFwk.WrongToken");
    int32_t result = backStub->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(result, ERR_INVALID_STATE);
    TAG_LOGE(AAFwkTag::TEST, "OnRemoteRequest_0500 end.");
}

/**
 * @tc.name: OnRemoteRequest_0600
 * @tc.desc: Test OnRemoteRequest with empty parcel (no interface token written).
 */
HWTEST_F(InsightIntentExecuteCallbackStubTest, OnRemoteRequest_0600, TestSize.Level1)
{
    TAG_LOGE(AAFwkTag::TEST, "OnRemoteRequest_0600 begin.");
    IPCSkeleton::SetCallingUid(FOUNDATION_UID);
    std::shared_ptr<InsightIntentExecuteCallbackStub> backStub
        = std::make_shared<InsightIntentExecuteCallbackStubTests>();
    uint32_t code = IInsightIntentExecuteCallback::ON_INSIGHT_INTENT_EXECUTE_DONE;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    int32_t result = backStub->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(result, ERR_INVALID_STATE);
    TAG_LOGE(AAFwkTag::TEST, "OnRemoteRequest_0600 end.");
}

/**
 * @tc.name: OnExecuteDoneInner_0300
 * @tc.desc: Test OnExecuteDoneInner with only key written (no resultCode, no result → ERR_INVALID_VALUE).
 */
HWTEST_F(InsightIntentExecuteCallbackStubTest, OnExecuteDoneInner_0300, TestSize.Level1)
{
    TAG_LOGE(AAFwkTag::TEST, "OnExecuteDoneInner_0300 begin.");
    std::shared_ptr<InsightIntentExecuteCallbackStub> backStub
        = std::make_shared<InsightIntentExecuteCallbackStubTests>();
    MessageParcel data;
    MessageParcel reply;
    data.WriteUint64(100);
    int32_t result = backStub->OnExecuteDoneInner(data, reply);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    TAG_LOGE(AAFwkTag::TEST, "OnExecuteDoneInner_0300 end.");
}

/**
 * @tc.name: OnExecuteDoneInner_0400
 * @tc.desc: Test OnExecuteDoneInner with key + resultCode but null executeResult → ERR_INVALID_VALUE.
 */
HWTEST_F(InsightIntentExecuteCallbackStubTest, OnExecuteDoneInner_0400, TestSize.Level1)
{
    TAG_LOGE(AAFwkTag::TEST, "OnExecuteDoneInner_0400 begin.");
    std::shared_ptr<InsightIntentExecuteCallbackStub> backStub
        = std::make_shared<InsightIntentExecuteCallbackStubTests>();
    MessageParcel data;
    MessageParcel reply;
    data.WriteUint64(200);
    data.WriteInt32(0);
    int32_t result = backStub->OnExecuteDoneInner(data, reply);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    TAG_LOGE(AAFwkTag::TEST, "OnExecuteDoneInner_0400 end.");
}

/**
 * @tc.name: OnRemoteRequest_0700
 * @tc.desc: Test OnRemoteRequest success path with non-zero resultCode (error result forwarded).
 */
HWTEST_F(InsightIntentExecuteCallbackStubTest, OnRemoteRequest_0700, TestSize.Level1)
{
    TAG_LOGE(AAFwkTag::TEST, "OnRemoteRequest_0700 begin.");
    IPCSkeleton::SetCallingUid(FOUNDATION_UID);
    auto backStub = std::make_shared<InsightIntentExecuteCallbackStubTests>();
    uint32_t code = IInsightIntentExecuteCallback::ON_INSIGHT_INTENT_EXECUTE_DONE;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(APPMGR_INTERFACE_TOKEN);
    data.WriteUint64(999);
    data.WriteInt32(ERR_INVALID_VALUE);
    AppExecFwk::InsightIntentExecuteResult executeResult;
    executeResult.code = 9991;  // distinct value to verify executeResult round-trips into the callback
    data.WriteParcelable(&executeResult);
    int32_t result = backStub->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(result, ERR_OK);
    // Verify the error-result payload (non-zero resultCode) is still forwarded verbatim.
    EXPECT_TRUE(backStub->IsOnExecuteDoneCalled());
    EXPECT_EQ(backStub->GetLastKey(), static_cast<uint64_t>(999));
    EXPECT_EQ(backStub->GetLastResultCode(), ERR_INVALID_VALUE);
    EXPECT_EQ(backStub->GetLastExecuteResultCode(), 9991);
    TAG_LOGE(AAFwkTag::TEST, "OnRemoteRequest_0700 end.");
}

/**
 * @tc.name: OnRemoteRequest_0800
 * @tc.desc: Test OnRemoteRequest with target code + foundation uid but only token (no payload) → ERR_INVALID_VALUE.
 */
HWTEST_F(InsightIntentExecuteCallbackStubTest, OnRemoteRequest_0800, TestSize.Level1)
{
    TAG_LOGE(AAFwkTag::TEST, "OnRemoteRequest_0800 begin.");
    IPCSkeleton::SetCallingUid(FOUNDATION_UID);
    std::shared_ptr<InsightIntentExecuteCallbackStub> backStub
        = std::make_shared<InsightIntentExecuteCallbackStubTests>();
    uint32_t code = IInsightIntentExecuteCallback::ON_INSIGHT_INTENT_EXECUTE_DONE;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(APPMGR_INTERFACE_TOKEN);
    int32_t result = backStub->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    TAG_LOGE(AAFwkTag::TEST, "OnRemoteRequest_0800 end.");
}
} // namespace AAFwk
} // namespace OHOS
