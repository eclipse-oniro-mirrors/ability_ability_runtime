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

#include <chrono>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <thread>

#include "cli_error_code.h"
#include "cli_tool_mgr_client.h"
#include "errors.h"
#include "function_info.h"
#include "intent_client.h"
#include "invoke_function_executor.h"
#include "want_params.h"

using namespace testing::ext;

namespace OHOS {
namespace CliTool {
namespace {

// Upper bound for waiting on the async executor. The ffrt worker runs DoExecute
// within milliseconds; this only bounds the failure mode (a real hang).
constexpr int32_t WAIT_TIMEOUT_MS = 2000;

constexpr const char *BUNDLE_NAME = "com.example.app";
constexpr const char *FUNCTION_NAME = "QueryWeather";

/**
 * @brief Ref-counted result sink for the executor callback.
 *
 * Execute() runs its flow on an ffrt worker thread and may invoke the callback
 * from there (or, for an unhanded success, never within the test window). The
 * callback therefore must not reference transient fixture state. It captures a
 * shared_ptr to this struct, which is also kept alive by the executor's pending
 * closures, so a late (e.g. 30s timeout) delivery can never touch freed memory.
 */
struct ResultCapture {
    std::mutex mutex;
    std::condition_variable cv;
    bool fired = false;
    int32_t callCount = 0;
    InvokeFunctionResult result;
};

class InvokeFunctionExecutorTest : public testing::Test {
protected:
    void SetUp() override
    {
        // The clients are singletons and persist across tests; reset every knob
        // to a clean success baseline before each case overrides what it needs.
        CliToolMGRClient::GetInstance().mockStatus_ = ERR_OK;
        CliToolMGRClient::GetInstance().mockFunctionType_ = FunctionType::INTENT_FUNCTION;
        AAFwk::IntentClient::GetInstance().mockStatus_ = ERR_OK;
    }

    // Build a callback that records the single outcome into a shared capture.
    static InvokeResultCallback MakeCallback(std::shared_ptr<ResultCapture> capture)
    {
        return [capture](const InvokeFunctionResult &result) {
            std::lock_guard<std::mutex> lock(capture->mutex);
            capture->fired = true;
            capture->callCount++;
            capture->result = result;
            capture->cv.notify_all();
        };
    }

    // Block until the executor reports an outcome, or the deadline elapses.
    static bool WaitForResult(const std::shared_ptr<ResultCapture> &capture,
        int32_t timeoutMs = WAIT_TIMEOUT_MS)
    {
        std::unique_lock<std::mutex> lock(capture->mutex);
        return capture->cv.wait_for(lock, std::chrono::milliseconds(timeoutMs),
            [&capture]() { return capture->fired; });
    }

    // Kick off an executor run and return the capture the callback will report into.
    static std::shared_ptr<ResultCapture> Run()
    {
        auto capture = std::make_shared<ResultCapture>();
        auto executor = InvokeFunctionExecutor::Create();
        AAFwk::WantParams params;
        executor->Execute(BUNDLE_NAME, FUNCTION_NAME, params, MakeCallback(capture));
        return capture;
    }
};

/**
 * @tc.name: InvokeFunctionExecutor_Create_0100
 * @tc.desc: Create() returns a non-null executor instance.
 * @tc.type: FUNC
 */
HWTEST_F(InvokeFunctionExecutorTest, InvokeFunctionExecutor_Create_0100, TestSize.Level1)
{
    auto executor = InvokeFunctionExecutor::Create();
    ASSERT_NE(executor, nullptr);
}

/**
 * @tc.name: InvokeFunctionExecutor_QueryFailedGeneric_0200
 * @tc.desc: Step 1 query returns a generic error (neither permission nor
 *           not-exist) -> inner error (original error code passed through).
 * @tc.type: FUNC
 */
HWTEST_F(InvokeFunctionExecutorTest, InvokeFunctionExecutor_QueryFailedGeneric_0200, TestSize.Level1)
{
    CliToolMGRClient::GetInstance().mockStatus_ = ERR_KVSTORE_NOT_READY;
    auto capture = Run();
    ASSERT_TRUE(WaitForResult(capture));
    EXPECT_FALSE(capture->result.invokeSuccess);
    EXPECT_EQ(capture->result.errorCode, ERR_KVSTORE_NOT_READY);
}

/**
 * @tc.name: InvokeFunctionExecutor_QueryPermissionDenied_0300
 * @tc.desc: Step 1 query returns ERR_PERMISSION_DENIED -> preserved verbatim
 *           (specific semantics, not collapsed into QUERY_FAILED).
 * @tc.type: FUNC
 */
HWTEST_F(InvokeFunctionExecutorTest, InvokeFunctionExecutor_QueryPermissionDenied_0300, TestSize.Level1)
{
    CliToolMGRClient::GetInstance().mockStatus_ = ERR_PERMISSION_DENIED;
    auto capture = Run();
    ASSERT_TRUE(WaitForResult(capture));
    EXPECT_FALSE(capture->result.invokeSuccess);
    EXPECT_EQ(capture->result.errorCode, ERR_PERMISSION_DENIED);
}

/**
 * @tc.name: InvokeFunctionExecutor_QueryFunctionNotExist_0400
 * @tc.desc: Step 1 query returns ERR_FUNCTION_NOT_EXIST -> preserved verbatim.
 * @tc.type: FUNC
 */
HWTEST_F(InvokeFunctionExecutorTest, InvokeFunctionExecutor_QueryFunctionNotExist_0400, TestSize.Level1)
{
    CliToolMGRClient::GetInstance().mockStatus_ = ERR_FUNCTION_NOT_EXIST;
    auto capture = Run();
    ASSERT_TRUE(WaitForResult(capture));
    EXPECT_FALSE(capture->result.invokeSuccess);
    EXPECT_EQ(capture->result.errorCode, ERR_FUNCTION_NOT_EXIST);
}

/**
 * @tc.name: InvokeFunctionExecutor_TypeNotSupported_0500
 * @tc.desc: Step 2 query succeeds but the function type is not ok
 *           function -> inner error (ERR_INNER_PARAM_INVALID).
 * @tc.type: FUNC
 */
HWTEST_F(InvokeFunctionExecutorTest, InvokeFunctionExecutor_TypeNotSupported_0500, TestSize.Level1)
{
    CliToolMGRClient::GetInstance().mockFunctionType_ = static_cast<FunctionType>(1);
    auto capture = Run();
    ASSERT_TRUE(WaitForResult(capture));
    EXPECT_FALSE(capture->result.invokeSuccess);
    EXPECT_EQ(capture->result.errorCode, ERR_INNER_PARAM_INVALID);
}

/**
 * @tc.name: InvokeFunctionExecutor_IntentPermissionDenied_0600
 * @tc.desc: Step 3 ExecuteIntentByFunctionCall fails with the common
 *           OHOS::ERR_PERMISSION_DENIED
 * @tc.type: FUNC
 */
HWTEST_F(InvokeFunctionExecutorTest, InvokeFunctionExecutor_IntentPermissionDenied_0600, TestSize.Level1)
{
    AAFwk::IntentClient::GetInstance().mockStatus_ = OHOS::ERR_PERMISSION_DENIED;
    auto capture = Run();
    ASSERT_TRUE(WaitForResult(capture));
    EXPECT_FALSE(capture->result.invokeSuccess);
    EXPECT_EQ(capture->result.errorCode, OHOS::ERR_PERMISSION_DENIED);
}

/**
 * @tc.name: InvokeFunctionExecutor_IntentFailed_0700
 * @tc.desc: Step 3 ExecuteIntentByFunctionCall fails with any other error ->
 *           collapsed into ERR_FUNCTION_EXECUTE_FAILED.
 * @tc.type: FUNC
 */
HWTEST_F(InvokeFunctionExecutorTest, InvokeFunctionExecutor_IntentFailed_0700, TestSize.Level1)
{
    AAFwk::IntentClient::GetInstance().mockStatus_ = ERR_INVALID_OPERATION;
    auto capture = Run();
    ASSERT_TRUE(WaitForResult(capture));
    EXPECT_FALSE(capture->result.invokeSuccess);
    EXPECT_EQ(capture->result.errorCode, ERR_FUNCTION_EXECUTE_FAILED);
}

/**
 * @tc.name: InvokeFunctionExecutor_Success_0800
 * @tc.desc: Query ok, type supported, execute ok and the execution reply is delivered
 *           through the callback client -> success outcome reaches the caller.
 *           Exercises the executor + callback client wiring end-to-end.
 * @tc.type: FUNC
 */
HWTEST_F(InvokeFunctionExecutorTest, InvokeFunctionExecutor_Success_0800, TestSize.Level1)
{
    auto capture = Run();
    ASSERT_TRUE(WaitForResult(capture));
    EXPECT_TRUE(capture->result.invokeSuccess);
    EXPECT_EQ(capture->result.errorCode, 0);
    EXPECT_EQ(capture->result.resultCode, 0);
}

/**
 * @tc.name: InvokeFunctionExecutor_ReportedExactlyOnce_0900
 * @tc.desc: On the success path the outcome is delivered exactly once even
 *           though both the normal callback and the armed 30s timeout race on
 *           the same completed flag.
 * @tc.type: FUNC
 */
HWTEST_F(InvokeFunctionExecutorTest, InvokeFunctionExecutor_ReportedExactlyOnce_0900, TestSize.Level1)
{
    auto capture = Run();
    ASSERT_TRUE(WaitForResult(capture));
    // Give any duplicate / late delivery a chance to land, then confirm it never did.
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    std::lock_guard<std::mutex> lock(capture->mutex);
    EXPECT_EQ(capture->callCount, 1);
    EXPECT_TRUE(capture->result.invokeSuccess);  // the first (winning) outcome is retained
}

/**
 * @tc.name: InvokeFunctionExecutor_1000
 * @tc.desc: Step 3 ExecuteIntentByFunctionCall fails with the common
 *           AAFwk::ERR_NOT_SYSTEM_APP
 * @tc.type: FUNC
 */
HWTEST_F(InvokeFunctionExecutorTest, InvokeFunctionExecutor_1000, TestSize.Level1)
{
    AAFwk::IntentClient::GetInstance().mockStatus_ = OHOS::AAFwk::ERR_NOT_SYSTEM_APP;
    auto capture = Run();
    ASSERT_TRUE(WaitForResult(capture));
    EXPECT_FALSE(capture->result.invokeSuccess);
    EXPECT_EQ(capture->result.errorCode, OHOS::AAFwk::ERR_NOT_SYSTEM_APP);
}
} // namespace
} // namespace CliTool
} // namespace OHOS
