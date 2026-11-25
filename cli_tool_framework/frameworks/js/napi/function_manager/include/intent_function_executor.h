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

#ifndef OHOS_CLI_TOOL_INTENT_FUNCTION_EXECUTOR_H
#define OHOS_CLI_TOOL_INTENT_FUNCTION_EXECUTOR_H

#include "function_executor.h"
#include "intent_client.h"

namespace OHOS {
namespace CliTool {

/**
 * @brief Executor for IntentFunction: runs the before-invoke
 *        hook, assembles the ExecuteIntentParam (trace ids + caller token
 *        envelope + callback client) and dispatches via IntentClient.
 *
 * Both entries (system / public) share this implementation: the public
 * differences live in the query path (invoker) and the consent gate, and the
 * pre-registered record id rides session_.intentRecordId (0 = system path).
 *
 * The Invoke() pipeline is decomposed into static member helpers so the
 * internal stages (hook preparation, param assembly, dispatch + error
 * mapping) are directly unit-testable; they are internal implementation
 * helpers, not part of any stable surface.
 */
class IntentFunctionExecutor final : public FunctionExecutor {
public:
    /**
     * @param consentGate Authorization strategy for the public entry (null
     *                    for the system entry); the factory selects it from
     *                    the entry and the function type.
     */
    IntentFunctionExecutor(InvokeSession session, FunctionInfo functionInfo,
        std::unique_ptr<ConsentGate> consentGate);

    /**
     * @brief Execution identifiers re-read after the before-invoke hook has
     *        run: the two trace ids and the task attribution id.
     */
    struct ExecutionIds {
        std::string toolCallId;
        std::string dmSessionId;
        std::string taskId;
    };

    /**
     * @brief Inputs assembled after the before-invoke hook: the hook may
     *        rewrite the param (and its args).
     */
    struct PreparedInvoke {
        InvokeFunctionParam hookParam;
        AAFwk::WantParams hookParams;
        ExecutionIds executionIds;
    };

    /**
     * @brief Run the before-invoke hook and capture the post-hook trace ids;
     *        invalid hook-modified ids are dropped.
     */
    static PreparedInvoke PrepareInvoke(const InvokeFunctionParam &param);

    /**
     * @brief Build the callback that stamps the actual execution identifiers
     *        onto the after-hook Wrap and forwards the outcome. The callback
     *        client has already won the completed-flag race when it runs, so
     *        the delivery is a plain forward (no second settle attempt).
     */
    static InvokeResultCallback BuildWrappedCallback(const InvokeResultCallback &finalCallback,
        const ExecutionIds &executionIds);

    /**
     * @brief Assemble the ExecuteIntentParam: post-hook namespace/name/args,
     *        authoritative trace-id injection (ADR-8), the caller-token
     *        envelope, the pre-registered record id and the callback client.
     */
    static void BuildExecuteIntentParam(const PreparedInvoke &prepared,
        const sptr<IRemoteObject> &callerAbilityToken, int32_t intentRecordId,
        const std::shared_ptr<AbilityRuntime::InsightIntentExecuteCallbackInterface> &client,
        AAFwk::ExecuteIntentParam &execParam);

    /**
     * @brief Dispatch the execution and map a synchronous failure to
     *        the settle error code: ERR_PERMISSION_DENIED /
     *        AAFwk::ERR_NOT_SYSTEM_APP pass through, anything else collapses
     *        into ERR_FUNCTION_EXECUTE_FAILED.
     */
    static ErrCode ExecuteIntent(const AAFwk::ExecuteIntentParam &execParam);

protected:
    /**
     * @brief Worker-thread execution: hook preparation, callback client
     *        wiring, param assembly and dispatch. The synchronous failure
     *        return is already mapped; success (ERR_OK) settles
     *        asynchronously via the callback client.
     */
    ErrCode Invoke() override;
};
} // namespace CliTool
} // namespace OHOS

#endif // OHOS_CLI_TOOL_INTENT_FUNCTION_EXECUTOR_H
