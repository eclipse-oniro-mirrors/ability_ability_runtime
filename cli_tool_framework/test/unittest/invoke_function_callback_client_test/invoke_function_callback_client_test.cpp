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

#include <atomic>
#include <memory>
#include <optional>

#include "array_wrapper.h"
#include "cli_error_code.h"
#include "insight_intent_execute_result.h"
#include "invoke_function_callback_client.h"
#include "string_wrapper.h"

using namespace testing::ext;

namespace OHOS {
namespace CliTool {
namespace {
constexpr int32_t APP_CODE_42 = 42;
}

class InvokeFunctionCallbackClientTest : public testing::Test {
protected:
    void SetUp() override
    {
        completed_ = std::make_shared<std::atomic<bool>>(false);
        callCount_ = 0;
        captured_.reset();
        client_ = std::make_shared<InvokeFunctionCallbackClient>(completed_,
            [this](const InvokeFunctionResult &result) {
                captured_ = result;
                ++callCount_;
            });
    }

    // Build an execute result carrying the given app-level code and optional data.
    AppExecFwk::InsightIntentExecuteResult BuildIntentResult(int32_t code,
        std::shared_ptr<AAFwk::WantParams> wantParam = nullptr)
    {
        AppExecFwk::InsightIntentExecuteResult result;
        result.code = code;
        result.result = wantParam;
        return result;
    }

    std::shared_ptr<std::atomic<bool>> completed_;
    std::shared_ptr<InvokeFunctionCallbackClient> client_;
    int32_t callCount_ = 0;
    std::optional<InvokeFunctionResult> captured_;
};

/**
 * @tc.name: InvokeFunctionCallbackClient_Success_0100
 * @tc.desc: framework resultCode == 0 and app code == 0 -> success path.
 *           With a null input result, the reply is wrapped into a non-empty
 *           WantParams carrying only flags (no result/uris keys).
 * @tc.type: FUNC
 */
HWTEST_F(InvokeFunctionCallbackClientTest, InvokeFunctionCallbackClient_Success_0100, TestSize.Level1)
{
    client_->ProcessInsightIntentExecute(0, BuildIntentResult(0));
    ASSERT_EQ(callCount_, 1);
    ASSERT_TRUE(captured_.has_value());
    EXPECT_TRUE(captured_->invokeSuccess);
    EXPECT_EQ(captured_->errorCode, 0);
    EXPECT_EQ(captured_->resultCode, 0);
    ASSERT_NE(captured_->result, nullptr);
    EXPECT_TRUE(captured_->result->HasParam("flags"));
    EXPECT_EQ(captured_->result->GetIntParam("flags", -1), 0);
    EXPECT_FALSE(captured_->result->HasParam("uris"));
    EXPECT_FALSE(captured_->result->HasParam("result"));
}

/**
 * @tc.name: InvokeFunctionCallbackClient_AppBusinessFailure_0200
 * @tc.desc: framework delivered ok (resultCode == 0) but app business code != 0:
 *           success stays true while resultCode surfaces the app-level code
 *           (framework vs app dual-authority contract).
 * @tc.type: FUNC
 */
HWTEST_F(InvokeFunctionCallbackClientTest, InvokeFunctionCallbackClient_AppBusinessFailure_0200,
    TestSize.Level1)
{
    client_->ProcessInsightIntentExecute(0, BuildIntentResult(APP_CODE_42));
    ASSERT_EQ(callCount_, 1);
    ASSERT_TRUE(captured_.has_value());
    EXPECT_TRUE(captured_->invokeSuccess);                  // framework delivery succeeded
    EXPECT_EQ(captured_->errorCode, 0);
    EXPECT_EQ(captured_->resultCode, APP_CODE_42);    // app business-level code passes through
}

/**
 * @tc.name: InvokeFunctionCallbackClient_FrameworkFailure_0300
 * @tc.desc: framework resultCode != 0 -> success=false, errorCode=EXECUTE_FAILED
 * @tc.type: FUNC
 */
HWTEST_F(InvokeFunctionCallbackClientTest, InvokeFunctionCallbackClient_FrameworkFailure_0300,
    TestSize.Level1)
{
    client_->ProcessInsightIntentExecute(1, BuildIntentResult(0));
    ASSERT_EQ(callCount_, 1);
    ASSERT_TRUE(captured_.has_value());
    EXPECT_FALSE(captured_->invokeSuccess);
    EXPECT_EQ(captured_->errorCode, ERR_FUNCTION_EXECUTE_FAILED);
    EXPECT_EQ(captured_->resultCode, 0);  // app code untouched on the framework-failure path
}

/**
 * @tc.name: InvokeFunctionCallbackClient_ResultPassthrough_0400
 * @tc.desc: the input result is wrapped into a fresh WantParams: the original
 *           WantParams is nested under the "result" key (not pointer-shared),
 *           alongside the "flags" key.
 * @tc.type: FUNC
 */
HWTEST_F(InvokeFunctionCallbackClientTest, InvokeFunctionCallbackClient_ResultPassthrough_0400,
    TestSize.Level1)
{
    auto want = std::make_shared<AAFwk::WantParams>();
    want->SetParam("k", AAFwk::String::Box("v"));
    client_->ProcessInsightIntentExecute(0, BuildIntentResult(0, want));
    ASSERT_EQ(callCount_, 1);
    ASSERT_TRUE(captured_.has_value());
    ASSERT_NE(captured_->result, nullptr);
    EXPECT_NE(captured_->result.get(), want.get());  // wrapped, not the same pointer
    EXPECT_TRUE(captured_->result->HasParam("flags"));
    EXPECT_TRUE(captured_->result->HasParam("result"));
    EXPECT_TRUE(captured_->result->GetWantParams("result") == *want);
    EXPECT_EQ(captured_->result->GetWantParams("result").GetStringParam("k"), "v");
}

/**
 * @tc.name: InvokeFunctionCallbackClient_AlreadyCompleted_0500
 * @tc.desc: when completed flag is already set (timeout/failure won first), the
 *           callback is NOT invoked.
 * @tc.type: FUNC
 */
HWTEST_F(InvokeFunctionCallbackClientTest, InvokeFunctionCallbackClient_AlreadyCompleted_0500,
    TestSize.Level1)
{
    completed_->store(true);  // simulate timeout/failure settling first
    client_->ProcessInsightIntentExecute(0, BuildIntentResult(0));
    EXPECT_EQ(callCount_, 0);
}

/**
 * @tc.name: InvokeFunctionCallbackClient_OnlyOnce_0600
 * @tc.desc: a second invocation after the first settle yields (CAS guard).
 * @tc.type: FUNC
 */
HWTEST_F(InvokeFunctionCallbackClientTest, InvokeFunctionCallbackClient_OnlyOnce_0600, TestSize.Level1)
{
    client_->ProcessInsightIntentExecute(0, BuildIntentResult(0));
    client_->ProcessInsightIntentExecute(1, BuildIntentResult(0));
    EXPECT_EQ(callCount_, 1);
    ASSERT_TRUE(captured_.has_value());
    EXPECT_TRUE(captured_->invokeSuccess);  // first winner's result is retained
}

/**
 * @tc.name: InvokeFunctionCallbackClient_NullCompleted_0700
 * @tc.desc: a null completed flag must not crash and must not invoke the callback.
 * @tc.type: FUNC
 */
HWTEST_F(InvokeFunctionCallbackClientTest, InvokeFunctionCallbackClient_NullCompleted_0700,
    TestSize.Level1)
{
    auto client = std::make_shared<InvokeFunctionCallbackClient>(nullptr,
        [](const InvokeFunctionResult &) { FAIL(); });
    client->ProcessInsightIntentExecute(0, BuildIntentResult(0));  // no crash, no callback
    SUCCEED();
}

/**
 * @tc.name: InvokeFunctionCallbackClient_ResultWrapsFlagsAndUris_0800
 * @tc.desc: flags value and the uris string array are packed into the result
 *           WantParams alongside the nested result (full three-field wrap).
 * @tc.type: FUNC
 */
HWTEST_F(InvokeFunctionCallbackClientTest, InvokeFunctionCallbackClient_ResultWrapsFlagsAndUris_0800,
    TestSize.Level1)
{
    auto want = std::make_shared<AAFwk::WantParams>();
    AppExecFwk::InsightIntentExecuteResult intentResult;
    intentResult.code = 0;
    intentResult.flags = 7;
    intentResult.uris = { "uri1", "uri2" };
    intentResult.result = want;
    client_->ProcessInsightIntentExecute(0, intentResult);
    ASSERT_EQ(callCount_, 1);
    ASSERT_TRUE(captured_.has_value());
    ASSERT_NE(captured_->result, nullptr);
    EXPECT_EQ(captured_->result->GetIntParam("flags", -1), 7);
    EXPECT_TRUE(captured_->result->HasParam("result"));
    EXPECT_TRUE(captured_->result->GetWantParams("result") == *want);
    // uris packed as a 2-element string array
    sptr<AAFwk::IInterface> urisVal = captured_->result->GetParam("uris");
    ASSERT_NE(urisVal, nullptr);
    auto *urisArr = AAFwk::IArray::Query(urisVal);
    ASSERT_NE(urisArr, nullptr);
    EXPECT_TRUE(AAFwk::Array::IsStringArray(urisArr));
    long len = 0;
    EXPECT_EQ(urisArr->GetLength(len), 0);
    EXPECT_EQ(len, 2);
}
} // namespace CliTool
} // namespace OHOS
