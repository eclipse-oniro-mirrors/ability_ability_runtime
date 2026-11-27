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
#include "consent_gate.h"
#include "errors.h"
#include "function_executor.h"
#include "function_executor_factory.h"
#include "function_info.h"
#include "insight_intent_execute_param.h"
#include "intent_client.h"
#include "intent_preprocessor.h"
#include "function_executor.h"
#include "function_invoker.h"
#include "intent_function_executor.h"
#include "invoke_function_callback_client.h"
#include "invoke_session.h"
#include "long_wrapper.h"
#include "ipc_object_stub.h"
#include "iremote_object.h"
#include "remote_object_wrapper.h"
#include "string_wrapper.h"
#include "want_params.h"
#include "want_params_wrapper.h"

using namespace testing::ext;

namespace OHOS {
namespace CliTool {
namespace {

// Upper bound for waiting on the async pipeline. The ffrt worker runs
// query -> factory -> executor within milliseconds; this only bounds the
// failure mode (a real hang).
constexpr int32_t WAIT_TIMEOUT_MS = 2000;
constexpr int32_t SLEEP_MS = 5;

constexpr const char *BUNDLE_NAME = "com.example.app";
constexpr const char *FUNCTION_NAME = "QueryWeather";

// Read a string-typed want parameter; returns an empty string when the key is
// absent or holds a non-string value (same semantics as the executor's reader).
std::string GetWantParamString(const AAFwk::WantParams &wantParams, const std::string &key)
{
    sptr<AAFwk::IInterface> value = wantParams.GetParam(key);
    if (value == nullptr) {
        return "";
    }
    auto *iStr = AAFwk::IString::Query(value);
    if (iStr == nullptr) {
        return "";
    }
    return AAFwk::String::Unbox(iStr);
}

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
    FunctionResultHolder result;
};

class FunctionInvokerTest : public testing::Test {
protected:
    void SetUp() override
    {
        // The clients are singletons and persist across tests; reset every knob
        // to a clean success baseline before each case overrides what it needs.
        auto &mgrClient = CliToolMGRClient::GetInstance();
        mgrClient.mockStatus_ = ERR_OK;
        mgrClient.mockPublicStatus_ = ERR_OK;
        mgrClient.mockFunctionType_ = FunctionType::INTENT_FUNCTION;
        mgrClient.mockHookModify_ = false;
        mgrClient.mockHookToolCallId_.clear();
        mgrClient.mockHookDmSessionId_.clear();
        mgrClient.mockHookTaskId_.clear();
        mgrClient.afterHookCalled_ = false;
        mgrClient.lastWrap_ = FunctionResultWrap();
        auto &intentClient = AAFwk::IntentClient::GetInstance();
        intentClient.mockStatus_ = ERR_OK;
        intentClient.executeCalled_ = false;
        intentClient.lastWantParam_ = AAFwk::WantParams();
        AbilityRuntime::IntentPreprocessor::mockStatus_ = ERR_OK;
        AbilityRuntime::IntentPreprocessor::mockGranted_ = false;
        AbilityRuntime::IntentPreprocessor::mockIntentId_ = 0;
        AbilityRuntime::IntentPreprocessor::preprocessorCalled_ = false;
        AbilityRuntime::IntentPreprocessor::lastBundleName_.clear();
        AbilityRuntime::IntentPreprocessor::lastIntentName_.clear();
    }

    // Build a callback that records the single outcome into a shared capture.
    static InvokeResultCallback MakeCallback(std::shared_ptr<ResultCapture> capture)
    {
        return [capture](const FunctionResultHolder &holder) {
            std::lock_guard<std::mutex> lock(capture->mutex);
            capture->fired = true;
            capture->callCount++;
            capture->result = holder;
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

    // Spin-wait until an atomic flag flips true (DispatchInvoke posts to ffrt).
    static bool WaitForFlag(const std::atomic<bool> &flag, int32_t timeoutMs = WAIT_TIMEOUT_MS)
    {
        auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(timeoutMs);
        while (!flag.load()) {
            if (std::chrono::steady_clock::now() > deadline) {
                return false;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_MS));
        }
        return true;
    }

    // Build a param mirroring the NAPI entry contract: non-empty invokeOptions
    // trace identifiers ride the reserved wantParam keys (transport leg).
    static InvokeFunctionParam MakeParam(const std::string &toolCallId = "",
        const std::string &dmSessionId = "", const std::string &taskId = "")
    {
        InvokeFunctionParam param;
        param.functionNamespace = BUNDLE_NAME;
        param.functionName = FUNCTION_NAME;
        param.invokeOptions.toolCallId = toolCallId;
        param.invokeOptions.dmSessionId = dmSessionId;
        param.invokeOptions.taskId = taskId;
        if (!toolCallId.empty()) {
            param.args.SetParam(RESERVED_KEY_TOOL_CALL_ID, AAFwk::String::Box(toolCallId));
        }
        if (!dmSessionId.empty()) {
            param.args.SetParam(RESERVED_KEY_DM_SESSION_ID, AAFwk::String::Box(dmSessionId));
        }
        if (!taskId.empty()) {
            param.args.SetParam(RESERVED_KEY_TASK_ID, AAFwk::String::Box(taskId));
        }
        return param;
    }

    // Kick off an invocation with a caller-built param and return the capture
    // the callback will report into.
    static std::shared_ptr<ResultCapture> RunParam(const InvokeFunctionParam &param)
    {
        auto capture = std::make_shared<ResultCapture>();
        auto invoker = FunctionInvoker::Create(param, false, nullptr, nullptr);
        invoker->Execute(MakeCallback(capture));
        return capture;
    }

    // Public (third-party) invocation path: the invoker queries via
    // GetPublicFunctionInfo and runs the public consent gate.
    static std::shared_ptr<ResultCapture> RunPublic()
    {
        auto capture = std::make_shared<ResultCapture>();
        auto invoker = FunctionInvoker::Create(MakeParam(), true, nullptr, nullptr);
        invoker->Execute(MakeCallback(capture));
        return capture;
    }

    // Kick off an executor run without trace identifiers and return the capture
    // the callback will report into.
    static std::shared_ptr<ResultCapture> Run()
    {
        return RunParam(MakeParam());
    }

    // Session for executor-level stage tests (custom subclass + injected gate).
    static InvokeSession MakeExecutorSession(const std::shared_ptr<ResultCapture> &capture)
    {
        InvokeSession session;
        session.param = MakeParam();
        session.completed = std::make_shared<std::atomic<bool>>(false);
        session.callback = MakeCallback(capture);
        return session;
    }

    static FunctionInfo MakeFunctionInfo(FunctionType type)
    {
        FunctionInfo info;
        info.functionNamespace = BUNDLE_NAME;
        info.functionName = FUNCTION_NAME;
        info.functionType = type;
        return info;
    }
};

/**
 * @tc.name: FunctionInvoker_Create_0100
 * @tc.desc: Create() returns a non-null invoker instance carrying the
 *           parameters (incl. isPublic) as members.
 * @tc.type: FUNC
 */
HWTEST_F(FunctionInvokerTest, FunctionInvoker_Create_0100, TestSize.Level1)
{
    auto invoker = FunctionInvoker::Create(MakeParam(), true, nullptr, nullptr);
    EXPECT_NE(invoker, nullptr);
}

/**
 * @tc.name: FunctionInvoker_QueryFailedGeneric_0200
 * @tc.desc: Step 1 query returns a generic error (neither permission nor
 *           not-exist) -> inner error (original error code passed through).
 * @tc.type: FUNC
 */
HWTEST_F(FunctionInvokerTest, FunctionInvoker_QueryFailedGeneric_0200, TestSize.Level1)
{
    CliToolMGRClient::GetInstance().mockStatus_ = ERR_KVSTORE_NOT_READY;
    auto capture = Run();
    ASSERT_TRUE(WaitForResult(capture));
    EXPECT_FALSE(capture->result.result.success);
    EXPECT_EQ(capture->result.innerError, ERR_KVSTORE_NOT_READY);
}

/**
 * @tc.name: FunctionInvoker_QueryPermissionDenied_0300
 * @tc.desc: Step 1 query returns ERR_PERMISSION_DENIED -> preserved verbatim
 *           (specific semantics, not collapsed into QUERY_FAILED).
 * @tc.type: FUNC
 */
HWTEST_F(FunctionInvokerTest, FunctionInvoker_QueryPermissionDenied_0300, TestSize.Level1)
{
    CliToolMGRClient::GetInstance().mockStatus_ = ERR_PERMISSION_DENIED;
    auto capture = Run();
    ASSERT_TRUE(WaitForResult(capture));
    EXPECT_FALSE(capture->result.result.success);
    EXPECT_EQ(capture->result.innerError, ERR_PERMISSION_DENIED);
}

/**
 * @tc.name: FunctionInvoker_QueryFunctionNotExist_0400
 * @tc.desc: Step 1 query returns ERR_FUNCTION_NOT_EXIST -> preserved verbatim.
 * @tc.type: FUNC
 */
HWTEST_F(FunctionInvokerTest, FunctionInvoker_QueryFunctionNotExist_0400, TestSize.Level1)
{
    CliToolMGRClient::GetInstance().mockStatus_ = ERR_FUNCTION_NOT_EXIST;
    auto capture = Run();
    ASSERT_TRUE(WaitForResult(capture));
    EXPECT_FALSE(capture->result.result.success);
    EXPECT_EQ(capture->result.innerError, ERR_FUNCTION_NOT_EXIST);
}

/**
 * @tc.name: InvokePublicFunctionExecutor_QueryFailed_1100
 * @tc.desc: Public path: GetPublicFunctionInfo failure is propagated verbatim
 *           (permission/visible checks happen on the service side).
 * @tc.type: FUNC
 */
HWTEST_F(FunctionInvokerTest, InvokePublicFunctionExecutor_QueryFailed_1100, TestSize.Level1)
{
    // The system-path mock stays ERR_OK: the public path must NOT fall back to it.
    CliToolMGRClient::GetInstance().mockStatus_ = ERR_OK;
    CliToolMGRClient::GetInstance().mockPublicStatus_ = ERR_PERMISSION_DENIED;
    auto capture = RunPublic();
    ASSERT_TRUE(WaitForResult(capture));
    EXPECT_FALSE(capture->result.result.success);
    EXPECT_EQ(capture->result.innerError, ERR_PERMISSION_DENIED);
}

/**
 * @tc.name: InvokePublicFunctionExecutor_QueryNotExist_1200
 * @tc.desc: Public path: ERR_FUNCTION_NOT_EXIST (function not open to third-party apps)
 *           is preserved verbatim.
 * @tc.type: FUNC
 */
HWTEST_F(FunctionInvokerTest, InvokePublicFunctionExecutor_QueryNotExist_1200, TestSize.Level1)
{
    CliToolMGRClient::GetInstance().mockPublicStatus_ = ERR_FUNCTION_NOT_EXIST;
    auto capture = RunPublic();
    ASSERT_TRUE(WaitForResult(capture));
    EXPECT_FALSE(capture->result.result.success);
    EXPECT_EQ(capture->result.innerError, ERR_FUNCTION_NOT_EXIST);
}

/**
 * @tc.name: InvokePublicFunctionExecutor_Success_1300
 * @tc.desc: Public path with no UIAbilityContext: the query succeeds and the
 *           consent gate settles fail-closed (the NAPI layer rejects this
 *           input up front; this exercises the gate's defense-in-depth), so
 *           the invocation ends with ERR_PERMISSION_DENIED and nothing is
 *           dispatched.
 * @tc.type: FUNC
 */
HWTEST_F(FunctionInvokerTest, InvokePublicFunctionExecutor_Success_1300, TestSize.Level1)
{
    CliToolMGRClient::GetInstance().mockPublicStatus_ = ERR_OK;
    auto capture = RunPublic();
    ASSERT_TRUE(WaitForResult(capture));
    EXPECT_FALSE(capture->result.result.success);
    EXPECT_EQ(capture->result.innerError, OHOS::ERR_PERMISSION_DENIED);
    // The consent gate stopped the flow before any intent dispatch.
    EXPECT_FALSE(AAFwk::IntentClient::GetInstance().executeCalled_);
}

/**
 * @tc.name: FunctionInvoker_TypeNotSupported_0500
 * @tc.desc: Step 2 query succeeds but the function type is not ok
 *           function -> inner error (ERR_INNER_PARAM_INVALID).
 * @tc.type: FUNC
 */
HWTEST_F(FunctionInvokerTest, FunctionInvoker_TypeNotSupported_0500, TestSize.Level1)
{
    CliToolMGRClient::GetInstance().mockFunctionType_ = static_cast<FunctionType>(1);
    auto capture = Run();
    ASSERT_TRUE(WaitForResult(capture));
    EXPECT_FALSE(capture->result.result.success);
    EXPECT_EQ(capture->result.innerError, ERR_INNER_PARAM_INVALID);
}

/**
 * @tc.name: FunctionInvoker_IntentPermissionDenied_0600
 * @tc.desc: Step 3 ExecuteIntentByFunctionCall fails with the common
 *           OHOS::ERR_PERMISSION_DENIED
 * @tc.type: FUNC
 */
HWTEST_F(FunctionInvokerTest, FunctionInvoker_IntentPermissionDenied_0600, TestSize.Level1)
{
    AAFwk::IntentClient::GetInstance().mockStatus_ = OHOS::ERR_PERMISSION_DENIED;
    auto capture = Run();
    ASSERT_TRUE(WaitForResult(capture));
    EXPECT_FALSE(capture->result.result.success);
    EXPECT_EQ(capture->result.innerError, OHOS::ERR_PERMISSION_DENIED);
}

/**
 * @tc.name: FunctionInvoker_IntentFailed_0700
 * @tc.desc: Step 3 ExecuteIntentByFunctionCall fails with any other error ->
 *           collapsed into ERR_FUNCTION_EXECUTE_FAILED.
 * @tc.type: FUNC
 */
HWTEST_F(FunctionInvokerTest, FunctionInvoker_IntentFailed_0700, TestSize.Level1)
{
    AAFwk::IntentClient::GetInstance().mockStatus_ = ERR_INVALID_OPERATION;
    auto capture = Run();
    ASSERT_TRUE(WaitForResult(capture));
    EXPECT_FALSE(capture->result.result.success);
    EXPECT_EQ(capture->result.innerError, ERR_FUNCTION_EXECUTE_FAILED);
}

/**
 * @tc.name: FunctionInvoker_Success_0800
 * @tc.desc: Query ok, type supported, execute ok and the execution reply is delivered
 *           through the callback client -> success outcome reaches the caller.
 *           Exercises the executor + callback client wiring end-to-end.
 * @tc.type: FUNC
 */
HWTEST_F(FunctionInvokerTest, FunctionInvoker_Success_0800, TestSize.Level1)
{
    auto capture = Run();
    ASSERT_TRUE(WaitForResult(capture));
    EXPECT_TRUE(capture->result.result.success);
    EXPECT_EQ(capture->result.innerError, 0);
    EXPECT_EQ(capture->result.result.errorCode, 0);
}

/**
 * @tc.name: FunctionInvoker_ReportedExactlyOnce_0900
 * @tc.desc: On the success path the outcome is delivered exactly once even
 *           though both the normal callback and the armed 30s timeout race on
 *           the same completed flag.
 * @tc.type: FUNC
 */
HWTEST_F(FunctionInvokerTest, FunctionInvoker_ReportedExactlyOnce_0900, TestSize.Level1)
{
    auto capture = Run();
    ASSERT_TRUE(WaitForResult(capture));
    // Give any duplicate / late delivery a chance to land, then confirm it never did.
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    std::lock_guard<std::mutex> lock(capture->mutex);
    EXPECT_EQ(capture->callCount, 1);
    EXPECT_TRUE(capture->result.result.success);  // the first (winning) outcome is retained
}

/**
 * @tc.name: FunctionInvoker_1000
 * @tc.desc: Step 3 ExecuteIntentByFunctionCall fails with the common
 *           AAFwk::ERR_NOT_SYSTEM_APP
 * @tc.type: FUNC
 */
HWTEST_F(FunctionInvokerTest, FunctionInvoker_1000, TestSize.Level1)
{
    AAFwk::IntentClient::GetInstance().mockStatus_ = OHOS::AAFwk::ERR_NOT_SYSTEM_APP;
    auto capture = Run();
    ASSERT_TRUE(WaitForResult(capture));
    EXPECT_FALSE(capture->result.result.success);
    EXPECT_EQ(capture->result.innerError, OHOS::AAFwk::ERR_NOT_SYSTEM_APP);
}

/**
 * @tc.name: FunctionInvoker_TraceIdInjected_1100
 * @tc.desc: Valid trace identifiers provided at entry ride the reserved wantParam
 *           keys into the intent execution with their values intact
 *           (authoritative injection point, ADR-8).
 * @tc.type: FUNC
 */
HWTEST_F(FunctionInvokerTest, FunctionInvoker_TraceIdInjected_1100, TestSize.Level1)
{
    auto capture = RunParam(MakeParam("tcid-Unit_0100", "dmsid-Unit-0200"));
    ASSERT_TRUE(WaitForResult(capture));
    EXPECT_TRUE(capture->result.result.success);

    auto &intentClient = AAFwk::IntentClient::GetInstance();
    ASSERT_TRUE(intentClient.executeCalled_);
    EXPECT_EQ(GetWantParamString(intentClient.lastWantParam_, RESERVED_KEY_TOOL_CALL_ID), "tcid-Unit_0100");
    EXPECT_EQ(GetWantParamString(intentClient.lastWantParam_, RESERVED_KEY_DM_SESSION_ID), "dmsid-Unit-0200");
}

/**
 * @tc.name: FunctionInvoker_TraceIdNotProvided_1200
 * @tc.desc: When no trace identifiers are provided the reserved keys are absent
 *           from the wantParam handed to the intent execution.
 * @tc.type: FUNC
 */
HWTEST_F(FunctionInvokerTest, FunctionInvoker_TraceIdNotProvided_1200, TestSize.Level1)
{
    auto capture = RunParam(MakeParam());
    ASSERT_TRUE(WaitForResult(capture));
    EXPECT_TRUE(capture->result.result.success);

    auto &intentClient = AAFwk::IntentClient::GetInstance();
    ASSERT_TRUE(intentClient.executeCalled_);
    EXPECT_FALSE(intentClient.lastWantParam_.HasParam(RESERVED_KEY_TOOL_CALL_ID));
    EXPECT_FALSE(intentClient.lastWantParam_.HasParam(RESERVED_KEY_DM_SESSION_ID));
}

/**
 * @tc.name: FunctionInvoker_HookRewrittenTraceId_1300
 * @tc.desc: A before-hook rewriting the toolCallId wins: the execution runs with
 *           the post-hook value while the untouched dmSessionId keeps its entry
 *           value (post-hook capture, R-3).
 * @tc.type: FUNC
 */
HWTEST_F(FunctionInvokerTest, FunctionInvoker_HookRewrittenTraceId_1300, TestSize.Level1)
{
    auto &mgrClient = CliToolMGRClient::GetInstance();
    mgrClient.mockHookModify_ = true;
    mgrClient.mockHookToolCallId_ = "hook-modified-id";

    auto capture = RunParam(MakeParam("orig-tool-call-id", "dmsid-keep-0300"));
    ASSERT_TRUE(WaitForResult(capture));
    EXPECT_TRUE(capture->result.result.success);

    auto &intentClient = AAFwk::IntentClient::GetInstance();
    ASSERT_TRUE(intentClient.executeCalled_);
    EXPECT_EQ(GetWantParamString(intentClient.lastWantParam_, RESERVED_KEY_TOOL_CALL_ID), "hook-modified-id");
    EXPECT_EQ(GetWantParamString(intentClient.lastWantParam_, RESERVED_KEY_DM_SESSION_ID), "dmsid-keep-0300");
}

/**
 * @tc.name: FunctionInvoker_HookInvalidTraceIdDropped_1400
 * @tc.desc: A before-hook rewriting the toolCallId to an invalid value (illegal
 *           characters) has the identifier dropped from the execution
 *           identifiers: the after-hook Wrap is stamped with an empty
 *           toolCallId while the valid dmSessionId survives and is re-injected.
 * @tc.type: FUNC
 */
HWTEST_F(FunctionInvokerTest, FunctionInvoker_HookInvalidTraceIdDropped_1400, TestSize.Level1)
{
    auto &mgrClient = CliToolMGRClient::GetInstance();
    mgrClient.mockHookModify_ = true;
    mgrClient.mockHookToolCallId_ = "bad id!";  // space/'!' fall outside [A-Za-z0-9_-]

    auto capture = RunParam(MakeParam("", "dmsid-valid-0400"));
    ASSERT_TRUE(WaitForResult(capture));
    EXPECT_TRUE(capture->result.result.success);

    auto &intentClient = AAFwk::IntentClient::GetInstance();
    ASSERT_TRUE(intentClient.executeCalled_);
    // The invalid hook value is dropped from the actual execution identifiers.
    ASSERT_TRUE(mgrClient.afterHookCalled_);
    EXPECT_EQ(mgrClient.lastWrap_.toolCallId, "");
    // The valid dmSessionId is still stamped and takes part in the injection.
    EXPECT_EQ(mgrClient.lastWrap_.dmSessionId, "dmsid-valid-0400");
    EXPECT_EQ(GetWantParamString(intentClient.lastWantParam_, RESERVED_KEY_DM_SESSION_ID), "dmsid-valid-0400");
}

/**
 * @tc.name: FunctionInvoker_StampExecutionIds_1500
 * @tc.desc: On the success path the after-hook FunctionResultWrap is stamped
 *           with the identifiers the execution actually ran with: the entry
 *           toolCallId and the hook-rewritten dmSessionId (post-hook capture,
 *           R-3).
 * @tc.type: FUNC
 */
HWTEST_F(FunctionInvokerTest, FunctionInvoker_StampExecutionIds_1500, TestSize.Level1)
{
    auto &mgrClient = CliToolMGRClient::GetInstance();
    mgrClient.mockHookModify_ = true;
    mgrClient.mockHookDmSessionId_ = "dmsid-hook-0500";

    auto capture = RunParam(MakeParam("tcid-stamp-0500", "dmsid-orig-0500"));
    ASSERT_TRUE(WaitForResult(capture));
    EXPECT_TRUE(capture->result.result.success);

    ASSERT_TRUE(mgrClient.afterHookCalled_);
    EXPECT_EQ(mgrClient.lastWrap_.toolCallId, "tcid-stamp-0500");
    EXPECT_EQ(mgrClient.lastWrap_.dmSessionId, "dmsid-hook-0500");
}

/**
 * @tc.name: FunctionInvoker_TaskIdInjected_1600
 * @tc.desc: A valid taskId provided at entry rides the reserved "ohos.task.id"
 *           wantParam key into the intent execution with its value intact
 *           (authoritative injection point, ADR-8).
 * @tc.type: FUNC
 */
HWTEST_F(FunctionInvokerTest, FunctionInvoker_TaskIdInjected_1600, TestSize.Level1)
{
    auto param = MakeParam("", "", "task-uuid-0600");
    param.args.SetParam("city", AAFwk::String::Box("Shanghai"));
    auto capture = RunParam(param);
    ASSERT_TRUE(WaitForResult(capture));
    EXPECT_TRUE(capture->result.result.success);

    auto &intentClient = AAFwk::IntentClient::GetInstance();
    ASSERT_TRUE(intentClient.executeCalled_);
    EXPECT_EQ(GetWantParamString(intentClient.lastWantParam_, RESERVED_KEY_TASK_ID), "task-uuid-0600");
    EXPECT_EQ(GetWantParamString(intentClient.lastWantParam_, "city"), "Shanghai");
}

/**
 * @tc.name: FunctionInvoker_TaskIdNotProvided_1700
 * @tc.desc: When no taskId is provided the reserved "ohos.task.id" key is absent
 *           from the wantParam handed to the intent execution (EX-4: missing
 *           key keeps the current behavior).
 * @tc.type: FUNC
 */
HWTEST_F(FunctionInvokerTest, FunctionInvoker_TaskIdNotProvided_1700, TestSize.Level1)
{
    auto capture = RunParam(MakeParam("tcid-only-0700"));
    ASSERT_TRUE(WaitForResult(capture));
    EXPECT_TRUE(capture->result.result.success);

    auto &intentClient = AAFwk::IntentClient::GetInstance();
    ASSERT_TRUE(intentClient.executeCalled_);
    EXPECT_FALSE(intentClient.lastWantParam_.HasParam(RESERVED_KEY_TASK_ID));
    EXPECT_EQ(GetWantParamString(intentClient.lastWantParam_, RESERVED_KEY_TOOL_CALL_ID), "tcid-only-0700");
}

/**
 * @tc.name: FunctionInvoker_ToolCallIdPassthrough_1100
 * @tc.desc: toolCallId in the reserved WantParams key reaches IntentClient with business args intact.
 * @tc.type: FUNC
 */
HWTEST_F(FunctionInvokerTest, FunctionInvoker_ToolCallIdPassthrough_1100, TestSize.Level1)
{
    auto param = MakeParam("tc-003");
    param.args.SetParam("city", AAFwk::String::Box("Shanghai"));
    auto capture = RunParam(param);
    ASSERT_TRUE(WaitForResult(capture));
    EXPECT_TRUE(capture->result.result.success);
    const auto &wantParam = AAFwk::IntentClient::GetInstance().lastWantParam_;
    EXPECT_EQ(wantParam.GetStringParam(AppExecFwk::INSIGHT_INTENT_TOOL_CALL_ID), "tc-003");
    EXPECT_EQ(wantParam.GetStringParam("city"), "Shanghai");
}

/**
 * @tc.name: FunctionInvoker_ToolCallIdEmptyCompat_1200
 * @tc.desc: an absent toolCallId does not introduce the reserved key into WantParams.
 * @tc.type: FUNC
 */
HWTEST_F(FunctionInvokerTest, FunctionInvoker_ToolCallIdEmptyCompat_1200, TestSize.Level1)
{
    auto capture = Run();
    ASSERT_TRUE(WaitForResult(capture));
    EXPECT_TRUE(capture->result.result.success);
    EXPECT_FALSE(AAFwk::IntentClient::GetInstance().lastWantParam_.HasParam(
        AppExecFwk::INSIGHT_INTENT_TOOL_CALL_ID));
}

/**
 * @tc.name: FunctionInvoker_CallerTokenBoxed_1800
 * @tc.desc: The caller ability token (context->GetToken(), passed through
 *           Execute) rides the reserved "ohos.insightIntent.callerToken" key in
 *           the WantParamWrapper envelope (TYPE_PROPERTY/VALUE_PROPERTY) into
 *           the intentExecution, round-tripping the same remote object.
 * @tc.type: FUNC
 */
HWTEST_F(FunctionInvokerTest, FunctionInvoker_CallerTokenBoxed_1800, TestSize.Level1)
{
    sptr<IRemoteObject> callerToken = new IPCObjectStub(u"FunctionInvokerTestCaller");
    ASSERT_NE(callerToken, nullptr);

    auto param = MakeParam();
    param.invokeOptions.callerAbilityToken = callerToken;
    auto capture = std::make_shared<ResultCapture>();
    auto invoker = FunctionInvoker::Create(param, false, nullptr, nullptr);
    invoker->Execute(MakeCallback(capture));
    ASSERT_TRUE(WaitForResult(capture));
    EXPECT_TRUE(capture->result.result.success);

    auto &intentClient = AAFwk::IntentClient::GetInstance();
    ASSERT_TRUE(intentClient.executeCalled_);
    // The mock mirrors the real IntentClient: it strips the token envelope out
    // of the wantParam and decodes it — the decoded remote object round-trips
    // the same token. (The envelope structure itself is asserted directly by
    // IntentExecutor_BuildExecuteIntentParam_3100.)
    EXPECT_EQ(intentClient.lastCallerAbilityToken_, callerToken);
    EXPECT_FALSE(intentClient.lastWantParam_.HasParam(AppExecFwk::INSIGHT_INTENT_CALLER_TOKEN));
}

/**
 * @tc.name: FunctionInvoker_CallerTokenAbsent_1900
 * @tc.desc: When no caller ability token is supplied (system entry / no
 *           UIAbilityContext) the reserved "ohos.insightIntent.callerToken" key
 *           is absent from the wantParam handed to the intentExecution.
 * @tc.type: FUNC
 */
HWTEST_F(FunctionInvokerTest, FunctionInvoker_CallerTokenAbsent_1900, TestSize.Level1)
{
    auto capture = Run();
    ASSERT_TRUE(WaitForResult(capture));
    EXPECT_TRUE(capture->result.result.success);

    auto &intentClient = AAFwk::IntentClient::GetInstance();
    ASSERT_TRUE(intentClient.executeCalled_);
    EXPECT_FALSE(intentClient.lastWantParam_.HasParam(AppExecFwk::INSIGHT_INTENT_CALLER_TOKEN));
}

// ---- Executor-level stage tests (custom subclass + injected gate) ----

// Minimal executor recording whether Invoke() ran.
class StubExecutor final : public FunctionExecutor {
public:
    StubExecutor(InvokeSession session, FunctionInfo functionInfo, std::unique_ptr<ConsentGate> gate)
        : FunctionExecutor(std::move(session), std::move(functionInfo), std::move(gate)) {}

    std::atomic<bool> invoked_ { false };

    // Expose the protected main-thread hop for contract tests.
    bool CallPostToMain(std::function<void()> task)
    {
        return PostToMain(std::move(task));
    }

protected:
    ErrCode Invoke() override
    {
        invoked_ = true;
        return ERR_OK; // success outcome would arrive via the callback client
    }
};

// Gate knob carrier: pre-check error and consent decision are both settable.
class StubGate final : public ConsentGate {
public:
    void RequestConsent(InvokeSession &session, const ConsentContinuation &cont) override
    {
        cont(granted, 0);
    }

    bool granted = true;
};

/**
 * @tc.name: FunctionExecutor_ConsentDenied_2200
 * @tc.desc: A consent denial settles the invocation with ERR_PERMISSION_DENIED
 *           and Invoke() never runs.
 * @tc.type: FUNC
 */
HWTEST_F(FunctionInvokerTest, FunctionExecutor_ConsentDenied_2200, TestSize.Level1)
{
    auto capture = std::make_shared<ResultCapture>();
    auto gate = std::make_unique<StubGate>();
    gate->granted = false;
    auto executor = std::make_shared<StubExecutor>(MakeExecutorSession(capture), MakeFunctionInfo(
        FunctionType::INTENT_FUNCTION), std::move(gate));
    executor->Execute();
    ASSERT_TRUE(WaitForResult(capture));
    EXPECT_EQ(capture->result.innerError, OHOS::ERR_PERMISSION_DENIED);
    EXPECT_FALSE(executor->invoked_);
}

/**
 * @tc.name: FunctionExecutor_ConsentGranted_2300
 * @tc.desc: A granted consent reaches Invoke() on a worker thread; with a
 *           stub Invoke returning ERR_OK and no callback client the promise
 *           stays pending (settlement belongs to the callback client race).
 * @tc.type: FUNC
 */
HWTEST_F(FunctionInvokerTest, FunctionExecutor_ConsentGranted_2300, TestSize.Level1)
{
    auto capture = std::make_shared<ResultCapture>();
    auto executor = std::make_shared<StubExecutor>(MakeExecutorSession(capture), MakeFunctionInfo(
        FunctionType::INTENT_FUNCTION), std::make_unique<StubGate>());
    executor->Execute();
    ASSERT_TRUE(WaitForFlag(executor->invoked_));
    EXPECT_FALSE(capture->fired);
}

/**
 * @tc.name: FunctionExecutorFactory_UnknownType_2400
 * @tc.desc: The factory returns nullptr for an unsupported functionType; the
 *           invoker then settles with ERR_INNER_PARAM_INVALID (covered by
 *           FunctionInvoker_TypeNotSupported_0500 end-to-end).
 * @tc.type: FUNC
 */
HWTEST_F(FunctionInvokerTest, FunctionExecutorFactory_UnknownType_2400, TestSize.Level1)
{
    InvokeSession session;
    EXPECT_EQ(FunctionExecutorFactory::Create(MakeFunctionInfo(static_cast<FunctionType>(99)), session),
        nullptr);
}

/**
 * @tc.name: FunctionExecutorFactory_IntentType_2500
 * @tc.desc: The factory returns an executor for FunctionType::INTENT_FUNCTION.
 * @tc.type: FUNC
 */
HWTEST_F(FunctionInvokerTest, FunctionExecutorFactory_IntentType_2500, TestSize.Level1)
{
    InvokeSession session;
    EXPECT_NE(FunctionExecutorFactory::Create(MakeFunctionInfo(FunctionType::INTENT_FUNCTION), session),
        nullptr);
}

/**
 * @tc.name: FunctionExecutor_SettledBeforeDispatch_2700
 * @tc.desc: A session already settled (timeout won, or a late consent
 *           decision racing a decline) never reaches Invoke(): server-side
 *           execution is not dispatched after settlement.
 * @tc.type: FUNC
 */
HWTEST_F(FunctionInvokerTest, FunctionExecutor_SettledBeforeDispatch_2700, TestSize.Level1)
{
    auto capture = std::make_shared<ResultCapture>();
    auto session = MakeExecutorSession(capture);
    ASSERT_NE(session.completed, nullptr);
    session.completed->store(true);  // pre-settle: e.g. the 30s timeout won

    auto executor = std::make_shared<StubExecutor>(session, MakeFunctionInfo(
        FunctionType::INTENT_FUNCTION), std::make_unique<StubGate>());
    executor->Execute();
    // Let the ffrt worker run the (guarded) dispatch task.
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_FALSE(executor->invoked_);
    EXPECT_FALSE(capture->fired);  // already settled: no new delivery either
}

/**
 * @tc.name: FunctionExecutor_PostToMain_NoChannel_3300
 * @tc.desc: PostToMain fails closed when no main-thread channel exists: it
 *           returns false and never executes the task inline — UI work must
 *           stay on the host UI thread.
 * @tc.type: FUNC
 */
HWTEST_F(FunctionInvokerTest, FunctionExecutor_PostToMain_NoChannel_3300, TestSize.Level1)
{
    auto capture = std::make_shared<ResultCapture>();
    auto session = MakeExecutorSession(capture);
    session.mainPoster = nullptr;  // no main-thread channel

    auto executor = std::make_shared<StubExecutor>(session, MakeFunctionInfo(
        FunctionType::INTENT_FUNCTION), nullptr);
    auto ran = std::make_shared<std::atomic<bool>>(false);
    bool posted = executor->CallPostToMain([ran]() { ran->store(true); });
    EXPECT_FALSE(posted);
    EXPECT_FALSE(ran->load());
}

/**
 * @tc.name: FunctionExecutor_PostToMain_PostFailed_3400
 * @tc.desc: PostToMain fails closed when the poster reports a failed
 *           delivery: it returns false and the task never runs — the caller
 *           must settle/skip instead of waiting.
 * @tc.type: FUNC
 */
HWTEST_F(FunctionInvokerTest, FunctionExecutor_PostToMain_PostFailed_3400, TestSize.Level1)
{
    auto capture = std::make_shared<ResultCapture>();
    auto session = MakeExecutorSession(capture);
    session.mainPoster = [](std::function<void()>) { return false; };  // delivery failed

    auto executor = std::make_shared<StubExecutor>(session, MakeFunctionInfo(
        FunctionType::INTENT_FUNCTION), nullptr);
    auto ran = std::make_shared<std::atomic<bool>>(false);
    bool posted = executor->CallPostToMain([ran]() { ran->store(true); });
    EXPECT_FALSE(posted);
    EXPECT_FALSE(ran->load());
}

/**
 * @tc.name: FunctionExecutor_PostToMain_Posted_3450
 * @tc.desc: PostToMain returns true only when the task was actually handed
 *           to the main-thread channel (and only then may it run).
 * @tc.type: FUNC
 */
HWTEST_F(FunctionInvokerTest, FunctionExecutor_PostToMain_Posted_3450, TestSize.Level1)
{
    auto capture = std::make_shared<ResultCapture>();
    auto session = MakeExecutorSession(capture);
    auto ran = std::make_shared<std::atomic<bool>>(false);
    auto ranRef = ran;
    session.mainPoster = [ranRef](std::function<void()> task) {
        task();  // simulate the main thread picking the task up
        return true;
    };

    auto executor = std::make_shared<StubExecutor>(session, MakeFunctionInfo(
        FunctionType::INTENT_FUNCTION), nullptr);
    bool posted = executor->CallPostToMain([ranRef]() { ranRef->store(true); });
    EXPECT_TRUE(posted);
    EXPECT_TRUE(ran->load());
}

/**
 * @tc.name: InvokeSession_Settle_Once_3500
 * @tc.desc: The explicit-channel Settle (used by the timeout task, which no
 *           longer captures the invoker) delivers exactly once: the first
 *           call wins the CAS, a second call on the same channel is a
 *           no-op, and a null channel never crashes.
 * @tc.type: FUNC
 */
HWTEST_F(FunctionInvokerTest, InvokeSession_Settle_Once_3500, TestSize.Level1)
{
    auto completed = std::make_shared<std::atomic<bool>>(false);
    auto capture = std::make_shared<ResultCapture>();
    auto callback = MakeCallback(capture);

    InvokeSession::Settle(completed, callback, ERR_FUNCTION_EXECUTE_TIMEOUT);
    ASSERT_TRUE(WaitForResult(capture));
    EXPECT_EQ(capture->result.innerError, ERR_FUNCTION_EXECUTE_TIMEOUT);
    EXPECT_EQ(capture->callCount, 1);

    // A second settle on the same channel loses the CAS: no new delivery.
    InvokeSession::Settle(completed, callback, OHOS::ERR_PERMISSION_DENIED);
    std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_MS * 10));
    EXPECT_EQ(capture->callCount, 1);

    // Null channel: no-op, no crash.
    InvokeSession::Settle(nullptr, callback, ERR_FUNCTION_EXECUTE_TIMEOUT);
    EXPECT_EQ(capture->callCount, 1);
}

/**
 * @tc.name: FunctionInvoker_ChannelOutlivesInvoker_3600
 * @tc.desc: The settlement channel is independent of the invoker's lifetime:
 *           the invoker's last strong reference drops when Run() returns (the
 *           NAPI layer holds none), yet the outcome — delivered afterwards
 *           by the callback client on a later worker task — still arrives
 *           exactly once. The timeout task captures only the channel
 *           (completed + callback), not the invoker.
 * @tc.type: FUNC
 */
HWTEST_F(FunctionInvokerTest, FunctionInvoker_ChannelOutlivesInvoker_3600, TestSize.Level1)
{
    // Default mocks (SetUp): query OK, gate null (system entry), execute OK —
    // the settlement is delivered by the callback client on the DispatchInvoke
    // worker task, after Run() has already released the invoker.
    std::shared_ptr<ResultCapture> capture;
    {
        auto invoker = FunctionInvoker::Create(MakeParam(), false, nullptr, nullptr);
        capture = std::make_shared<ResultCapture>();
        invoker->Execute(MakeCallback(capture));
    }
    ASSERT_TRUE(WaitForResult(capture));
    EXPECT_TRUE(capture->result.result.success);
    EXPECT_EQ(capture->result.innerError, 0);
    EXPECT_EQ(capture->callCount, 1);
}

// ---- Direct static-helper tests: the IntentFunctionExecutor stages are
// static member functions, so the internal logic is unit-testable directly
// (no anonymous-namespace internal linkage).

/**
 * @tc.name: PublicConsentGate_GrantFlow_2650
 * @tc.desc: The grant path: the gate hops to the main-thread poster, calls the
 *           consent preprocessor with the session's target, and forwards the
 *           granted decision together with the pre-registered record id.
 * @tc.type: FUNC
 */
HWTEST_F(FunctionInvokerTest, PublicConsentGate_GrantFlow_2650, TestSize.Level1)
{
    InvokeSession session;
    session.param = MakeParam();
    session.abilityContext = std::make_shared<AbilityRuntime::AbilityContext>();
    bool posterCalled = false;
    session.mainPoster = [&posterCalled](std::function<void()> task) {
        posterCalled = true;
        task();  // run inline: the automation-shortcut timing of the real pipeline
        return true;
    };
    using Pre = AbilityRuntime::IntentPreprocessor;
    Pre::mockGranted_ = true;
    Pre::mockIntentId_ = 42;

    bool decided = false;
    bool granted = false;
    int32_t recordId = -1;
    PublicConsentGate publicGate;
    publicGate.RequestConsent(session, [&](bool g, int32_t id) {
        decided = true;
        granted = g;
        recordId = id;
    });

    EXPECT_TRUE(posterCalled);
    EXPECT_TRUE(Pre::preprocessorCalled_);
    EXPECT_EQ(Pre::lastBundleName_, BUNDLE_NAME);
    EXPECT_EQ(Pre::lastIntentName_, FUNCTION_NAME);
    EXPECT_TRUE(decided);
    EXPECT_TRUE(granted);
    EXPECT_EQ(recordId, 42);
}

/**
 * @tc.name: PublicConsentGate_PipelineFailure_2660
 * @tc.desc: A synchronous preprocessor failure (pre-registration or dialog
 *           mount) settles fail-closed: declined with no record id, the
 *           decide callback never fires.
 * @tc.type: FUNC
 */
HWTEST_F(FunctionInvokerTest, PublicConsentGate_PipelineFailure_2660, TestSize.Level1)
{
    InvokeSession session;
    session.param = MakeParam();
    session.abilityContext = std::make_shared<AbilityRuntime::AbilityContext>();
    session.mainPoster = [](std::function<void()> task) {
        task();
        return true;
    };
    using Pre = AbilityRuntime::IntentPreprocessor;
    Pre::mockStatus_ = ERR_INVALID_OPERATION;
    Pre::mockGranted_ = true;  // would grant; the failure must win

    bool decided = false;
    bool granted = true;
    int32_t recordId = -1;
    PublicConsentGate publicGate;
    publicGate.RequestConsent(session, [&](bool g, int32_t id) {
        decided = true;
        granted = g;
        recordId = id;
    });

    EXPECT_TRUE(Pre::preprocessorCalled_);
    EXPECT_TRUE(decided);
    EXPECT_FALSE(granted);
    EXPECT_EQ(recordId, 0);
}

/**
 * @tc.name: PublicConsentGate_PostFailed_2670
 * @tc.desc: A failed main-thread post means the consent task never runs:
 *           the gate settles fail-closed immediately (no 30s timeout wait)
 *           and the preprocessor is never invoked.
 * @tc.type: FUNC
 */
HWTEST_F(FunctionInvokerTest, PublicConsentGate_PostFailed_2670, TestSize.Level1)
{
    InvokeSession session;
    session.param = MakeParam();
    session.abilityContext = std::make_shared<AbilityRuntime::AbilityContext>();
    session.mainPoster = [](std::function<void()>) { return false; };  // delivery failed

    bool decided = false;
    bool granted = true;
    int32_t recordId = -1;
    PublicConsentGate publicGate;
    publicGate.RequestConsent(session, [&](bool g, int32_t id) {
        decided = true;
        granted = g;
        recordId = id;
    });

    EXPECT_TRUE(decided);
    EXPECT_FALSE(granted);
    EXPECT_EQ(recordId, 0);
    // The consent task never ran, so the preprocessor was never reached.
    EXPECT_FALSE(AbilityRuntime::IntentPreprocessor::preprocessorCalled_);
}

/**
 * @tc.name: IntentExecutor_PrepareInvoke_2800
 * @tc.desc: PrepareInvoke runs the before-invoke hook and captures the
 *           post-hook trace identifiers (hook rewrite wins over entry values).
 * @tc.type: FUNC
 */
HWTEST_F(FunctionInvokerTest, IntentExecutor_PrepareInvoke_2800, TestSize.Level1)
{
    auto &mgrClient = CliToolMGRClient::GetInstance();
    mgrClient.mockHookModify_ = true;
    mgrClient.mockHookToolCallId_ = "hook-id-2800";

    auto prepared = IntentFunctionExecutor::PrepareInvoke(MakeParam("orig-2800", "dmsid-2800"));
    EXPECT_EQ(prepared.executionIds.toolCallId, "hook-id-2800");
    EXPECT_EQ(prepared.executionIds.dmSessionId, "dmsid-2800");
    EXPECT_EQ(prepared.hookParam.functionNamespace, BUNDLE_NAME);
    EXPECT_EQ(prepared.hookParam.functionName, FUNCTION_NAME);
}

/**
 * @tc.name: IntentExecutor_PrepareInvoke_DropsInvalid_2900
 * @tc.desc: An invalid hook-modified trace identifier is dropped from the
 *           prepared identifiers while the valid one survives.
 * @tc.type: FUNC
 */
HWTEST_F(FunctionInvokerTest, IntentExecutor_PrepareInvoke_DropsInvalid_2900, TestSize.Level1)
{
    auto &mgrClient = CliToolMGRClient::GetInstance();
    mgrClient.mockHookModify_ = true;
    mgrClient.mockHookToolCallId_ = "bad id!";  // space/'!' fall outside [A-Za-z0-9_-]

    auto prepared = IntentFunctionExecutor::PrepareInvoke(MakeParam("", "dmsid-2900"));
    EXPECT_TRUE(prepared.executionIds.toolCallId.empty());
    EXPECT_EQ(prepared.executionIds.dmSessionId, "dmsid-2900");
}

/**
 * @tc.name: IntentExecutor_BuildWrappedCallback_3000
 * @tc.desc: The wrapped callback stamps the execution identifiers onto the
 *           after-hook Wrap and forwards the outcome to the final callback.
 * @tc.type: FUNC
 */
HWTEST_F(FunctionInvokerTest, IntentExecutor_BuildWrappedCallback_3000, TestSize.Level1)
{
    auto &mgrClient = CliToolMGRClient::GetInstance();
    auto capture = std::make_shared<ResultCapture>();
    IntentFunctionExecutor::ExecutionIds ids = { "tcid-3000", "dmsid-3000", "" };

    auto wrapped = IntentFunctionExecutor::BuildWrappedCallback(MakeCallback(capture), ids);
    FunctionResultHolder holder;
    wrapped(holder);

    ASSERT_TRUE(mgrClient.afterHookCalled_);
    EXPECT_EQ(mgrClient.lastWrap_.toolCallId, "tcid-3000");
    EXPECT_EQ(mgrClient.lastWrap_.dmSessionId, "dmsid-3000");
    std::lock_guard<std::mutex> lock(capture->mutex);
    EXPECT_TRUE(capture->fired);
    EXPECT_EQ(capture->callCount, 1);
}

/**
 * @tc.name: IntentExecutor_BuildExecuteIntentParam_3100
 * @tc.desc: Param assembly: post-hook namespace/name/args, authoritative
 *           trace-id injection, the pre-registered record id boxing, the
 *           caller-token envelope and the callback client wiring.
 * @tc.type: FUNC
 */
HWTEST_F(FunctionInvokerTest, IntentExecutor_BuildExecuteIntentParam_3100, TestSize.Level1)
{
    sptr<IRemoteObject> callerToken = new IPCObjectStub(u"IntentExecutorHelperCaller");
    ASSERT_NE(callerToken, nullptr);

    IntentFunctionExecutor::PreparedInvoke prepared;
    prepared.hookParam = MakeParam("tcid-3100", "dmsid-3100", "task-3100");
    prepared.hookParams = prepared.hookParam.args;
    prepared.executionIds = { "tcid-3100", "dmsid-3100", "task-3100" };

    auto completed = std::make_shared<std::atomic<bool>>(false);
    auto client = std::make_shared<InvokeFunctionCallbackClient>(completed, nullptr);

    AAFwk::ExecuteIntentParam execParam;
    IntentFunctionExecutor::BuildExecuteIntentParam(prepared, callerToken, 0, client, execParam);

    EXPECT_EQ(execParam.bundleName, BUNDLE_NAME);
    EXPECT_EQ(execParam.intentName, FUNCTION_NAME);
    EXPECT_EQ(GetWantParamString(execParam.wantParam, RESERVED_KEY_TOOL_CALL_ID), "tcid-3100");
    EXPECT_EQ(GetWantParamString(execParam.wantParam, RESERVED_KEY_DM_SESSION_ID), "dmsid-3100");
    EXPECT_EQ(GetWantParamString(execParam.wantParam, RESERVED_KEY_TASK_ID), "task-3100");
    EXPECT_EQ(execParam.callback, client);
    // Zero record id (system path): the reserved key stays absent.
    EXPECT_FALSE(execParam.wantParam.HasParam(AppExecFwk::INSIGHT_INTENT_EXECUTE_RECORD_ID));

    // Non-zero record id (granted public path): boxed as a Long for the
    // IntentClient to strip and forward as a dedicated IPC field.
    AAFwk::ExecuteIntentParam publicParam;
    IntentFunctionExecutor::BuildExecuteIntentParam(prepared, callerToken, 42, client, publicParam);
    sptr<AAFwk::IInterface> recordValue = publicParam.wantParam.GetParam(
        AppExecFwk::INSIGHT_INTENT_EXECUTE_RECORD_ID);
    ASSERT_NE(recordValue, nullptr);
    auto *iLong = AAFwk::ILong::Query(recordValue);
    ASSERT_NE(iLong, nullptr);
    EXPECT_EQ(AAFwk::Long::Unbox64(iLong), 42);

    // The caller token rides the WantParamWrapper envelope (a bare
    // RemoteObjectWrap would be dropped by WantParams marshalling).
    sptr<AAFwk::IInterface> tokenValue = execParam.wantParam.GetParam(
        AppExecFwk::INSIGHT_INTENT_CALLER_TOKEN);
    ASSERT_NE(tokenValue, nullptr);
    auto *iwp = AAFwk::IWantParams::Query(tokenValue);
    ASSERT_NE(iwp, nullptr);
    AAFwk::WantParams tokenWrap = AAFwk::WantParamWrapper::Unbox(iwp);
    EXPECT_EQ(GetWantParamString(tokenWrap, AAFwk::TYPE_PROPERTY), AAFwk::REMOTE_OBJECT);
    sptr<AAFwk::IInterface> remoteObjValue = tokenWrap.GetParam(AAFwk::VALUE_PROPERTY);
    ASSERT_NE(remoteObjValue, nullptr);
    auto *iRemoteObj = AAFwk::IRemoteObjectWrap::Query(remoteObjValue);
    ASSERT_NE(iRemoteObj, nullptr);
    EXPECT_EQ(AAFwk::RemoteObjectWrap::UnBox(iRemoteObj), callerToken);
}

/**
 * @tc.name: IntentExecutor_ExecuteIntent_MapsErrors_3200
 * @tc.desc: The dispatch maps ERR_PERMISSION_DENIED verbatim and collapses
 *           any other synchronous failure into ERR_FUNCTION_EXECUTE_FAILED.
 * @tc.type: FUNC
 */
HWTEST_F(FunctionInvokerTest, IntentExecutor_ExecuteIntent_MapsErrors_3200, TestSize.Level1)
{
    AAFwk::ExecuteIntentParam execParam;
    auto &intentClient = AAFwk::IntentClient::GetInstance();

    intentClient.mockStatus_ = OHOS::ERR_PERMISSION_DENIED;
    EXPECT_EQ(IntentFunctionExecutor::ExecuteIntent(execParam), OHOS::ERR_PERMISSION_DENIED);

    intentClient.mockStatus_ = ERR_INVALID_OPERATION;
    EXPECT_EQ(IntentFunctionExecutor::ExecuteIntent(execParam), ERR_FUNCTION_EXECUTE_FAILED);

    intentClient.mockStatus_ = ERR_OK;
    EXPECT_EQ(IntentFunctionExecutor::ExecuteIntent(execParam), ERR_OK);
}

/**
 * @tc.name: PublicConsentGate_MissingContext_2600
 * @tc.desc: The public gate settles fail-closed (declined) when invoked
 *           without a UIAbilityContext or a main-thread poster: the consent
 *           dialog cannot be mounted. (The NAPI layer rejects a context-less
 *           public invocation up front; this is the defense-in-depth path.)
 *           The system entry gets NO gate at all (the factory passes null
 *           and the executor consent default passes through).
 * @tc.type: FUNC
 */
HWTEST_F(FunctionInvokerTest, PublicConsentGate_MissingContext_2600, TestSize.Level1)
{
    InvokeSession session;
    PublicConsentGate publicGate;

    bool decided = false;
    bool granted = true;
    publicGate.RequestConsent(session, [&decided, &granted](bool g, int32_t) {
        decided = true;
        granted = g;
    });
    EXPECT_TRUE(decided);
    EXPECT_FALSE(granted);
}

} // namespace
} // namespace CliTool
} // namespace OHOS
