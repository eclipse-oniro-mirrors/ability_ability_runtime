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

#ifndef OHOS_CLI_TOOL_FUNCTION_EXECUTOR_H
#define OHOS_CLI_TOOL_FUNCTION_EXECUTOR_H

#include <memory>

#include "consent_gate.h"
#include "function_info.h"
#include "invoke_session.h"

namespace OHOS {
namespace CliTool {

/**
 * @brief Type-specific execution stage of a function invocation.
 *
 * The executor is created by FunctionExecutorFactory AFTER the invoker has
 * queried the function info (the type decides which executor to build), and
 * runs a fixed two-stage chain via the non-virtual Execute template method:
 * RequestConsent (virtual, defaulting to the consent gate decision), then
 * Invoke (pure virtual).
 *
 * Threading contract:
 *  - Execute() is called on a worker thread (by the invoker) and the consent
 *    stage starts there: all IPC stays off the JS/UI main thread.
 *  - Consent may hop to the main thread (dialog); Invoke is always re-posted
 *    to a worker thread before running, so subclasses can assume a non-main
 *    thread inside Invoke.
 *  - Outcomes settle via session_.Fail (error) or the execution callback
 *    client (success), both racing on the shared completed flag so exactly
 *    one delivery reaches the caller.
 */
class FunctionExecutor : public std::enable_shared_from_this<FunctionExecutor> {
public:
    /**
     * @param session     Invocation parameters (incl. isPublic) + settlement.
     * @param functionInfo Queried function info (type already validated).
     * @param consentGate Authorization strategy for the public entry, selected
     *                    by the factory from the entry and the function type;
     *                    null means no consent stage at all (system entry) and
     *                    the consent stage default passes straight through.
     */
    FunctionExecutor(InvokeSession session, FunctionInfo functionInfo,
        std::unique_ptr<ConsentGate> consentGate);
    virtual ~FunctionExecutor();

    /**
     * @brief Run the two-stage chain. Call exactly once, on a worker thread.
     */
    void Execute();

protected:
    /**
     * @brief Stage 1: user consent (may hop to the main thread for UI).
     *        Default delegates to the consent gate's RequestConsent; the
     *        continuation carries the pre-registered record id on grant.
     */
    virtual void RequestConsent(const ConsentContinuation &cont);

    /**
     * @brief Stage 2: the actual execution (worker thread; IPC allowed).
     * @return The settle error code when the synchronous part fails (already
     *         mapped to a reportable code); ERR_OK means the outcome will be
     *         delivered asynchronously through the callback client.
     */
    virtual ErrCode Invoke() = 0;

    /**
     * @brief Settle with an error (CAS-guarded, see InvokeSession::Fail).
     */
    void Fail(int32_t errorCode);

    /**
     * @brief Post a task back to a worker thread (ffrt).
     */
    void PostToWorker(std::function<void()> task);

    /**
     * @brief Post a UI task to the JS/UI main thread via the injected poster.
     * @return Whether the task was actually posted to the main thread: false
     *         when the task is null, no main-thread channel exists, or the
     *         post failed.
     */
    bool PostToMain(std::function<void()> task);

    // Call parameters (incl. isPublic) + settlement channel.
    InvokeSession session_;
    // Queried function info; the functionType decided this executor's class.
    FunctionInfo functionInfo_;

private:
    void DispatchInvoke();

    std::unique_ptr<ConsentGate> consentGate_;
};
} // namespace CliTool
} // namespace OHOS

#endif // OHOS_CLI_TOOL_FUNCTION_EXECUTOR_H
