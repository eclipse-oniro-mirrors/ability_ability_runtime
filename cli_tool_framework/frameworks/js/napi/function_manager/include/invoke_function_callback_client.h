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

#ifndef OHOS_CLI_TOOL_INVOKE_FUNCTION_CALLBACK_CLIENT_H
#define OHOS_CLI_TOOL_INVOKE_FUNCTION_CALLBACK_CLIENT_H

#include <atomic>
#include <memory>

#include "insight_intent_callback_interface.h"
#include "insight_intent_execute_result.h"

#include "invoke_function_executor.h"

namespace OHOS {
namespace CliTool {

/**
 * @brief Execution callback client.
 *
 * Receives the function reply on a binder thread, wins (or yields) the shared
 * completed flag against the timeout/failure paths, and delivers a pure-C++
 * InvokeFunctionResult via the injected callback. Does NOT depend on napi or on
 * InvokeFunctionExecutor, so it can be unit-tested by injecting a callback and
 * an atomic flag directly.
 *
 * Race-guard contract: the shared `completed` flag is the single arbiter across
 * this callback, the executor's ReportError path, and the timeout task; only the
 * first compare_exchange_strong winner may deliver a result.
 */
class InvokeFunctionCallbackClient final
    : public AbilityRuntime::InsightIntentExecuteCallbackInterface {
public:
    using CompletedFlag = std::atomic<bool>;

    /**
     * @param completed Shared race-guard flag (owned by the executor, shared with
     *                  the timeout/failure paths).
     * @param callback  Invoked exactly once if this client wins the race.
     */
    InvokeFunctionCallbackClient(std::shared_ptr<CompletedFlag> completed, InvokeResultCallback callback);

    void ProcessInsightIntentExecute(int32_t resultCode,
        AppExecFwk::InsightIntentExecuteResult executeResult) override;

private:
    std::shared_ptr<CompletedFlag> completed_;
    InvokeResultCallback callback_;
};

} // namespace CliTool
} // namespace OHOS

#endif // OHOS_CLI_TOOL_INVOKE_FUNCTION_CALLBACK_CLIENT_H
