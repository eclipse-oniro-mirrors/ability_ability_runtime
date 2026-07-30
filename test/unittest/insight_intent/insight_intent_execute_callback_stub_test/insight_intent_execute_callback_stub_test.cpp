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
#include "hilog_tag_wrapper.h"
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
// Must mirror the constant in services/abilitymgr/src/insight_intent/insight_intent_execute_callback_stub.cpp.
constexpr int32_t FOUNDATION_UID = 5523;
}  // namespace

// Concrete subclass that satisfies the pure-virtual OnExecuteDone so the stub can
// be instantiated. Trust dispatch is driven entirely by mock IPCSkeleton state in
// each test case (SetCallingUid), not by subclassing.
class InsightIntentExecuteCallbackStubTests : public InsightIntentExecuteCallbackStub {
public:
    InsightIntentExecuteCallbackStubTests() = default;
    virtual ~InsightIntentExecuteCallbackStubTests()
    {}
    void OnExecuteDone(uint64_t key, int32_t resultCode,
        const AppExecFwk::InsightIntentExecuteResult &executeResult) override {}
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

} // namespace AAFwk
} // namespace OHOS
