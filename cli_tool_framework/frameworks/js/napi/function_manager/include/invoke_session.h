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

#ifndef OHOS_CLI_TOOL_INVOKE_SESSION_H
#define OHOS_CLI_TOOL_INVOKE_SESSION_H

#include <atomic>
#include <functional>
#include <memory>

#include "errors.h"
#include "invoke_function_param.h"
#include "invoke_function_result.h"

namespace OHOS {
namespace AbilityRuntime {
class AbilityContext;
} // namespace AbilityRuntime
namespace CliTool {

/**
 * @brief Carries InvokeFunctionResult (JS-visible) + innerError (framework reject code).
 *
 * result is exposed to the hook via FunctionResultWrap and may be modified.
 * innerError is NOT exposed to the hook; it drives the promise rejection path.
 */
struct FunctionResultHolder {
    int32_t innerError = 0;
    InvokeFunctionResult result;
};

/**
 * @brief Invoked EXACTLY ONCE on completion / failure / timeout.
 */
using InvokeResultCallback = std::function<void(const FunctionResultHolder &)>;

/**
 * @brief Hops a task onto the JS/UI main thread (consent dialog and other UI
 *        actions). Returns whether the task was actually posted: false means
 *        the delivery failed and the task will never run, so the caller must
 *        fail-closed instead of waiting.
 *
 * Injected by the NAPI layer (built on the threadsafe-function bridge); null when
 * no main-thread channel exists, in which case UI-dependent steps are unavailable.
 */
using MainThreadPoster = std::function<bool(std::function<void()>)>;

/**
 * @brief Continuation for the async consent stage: granted = true with the
 *        pre-registered execute record id to reuse (0 when none), or granted
 *        = false to settle with ERR_PERMISSION_DENIED.
 */
using ConsentContinuation = std::function<void(bool, int32_t)>;

/**
 * @brief Shared context of one function invocation, threaded from the invoker
 *        (which owns the parameters and the settlement channel) through the
 *        consent gate and the type-specific executor.
 *
 * All call parameters live here as members, including the entry selector
 * isPublic: the entry (invokeFunction / invokePublicFunction) — not the caller
 * identity — decides the query path and the authorization path.
 *
 * intentRecordId is the slot for the consent-side pre-registered execute
 * record id: written by the executor chain's consent continuation when the
 * user grants the invocation and consumed by the intent executor when
 * building the execution param (0 = system path, no record).
 */
struct InvokeSession {
    InvokeFunctionParam param;
    bool isPublic = false;
    // Caller UIAbilityContext hosting the consent dialog (public path only).
    // (The caller token rides param.invokeOptions.callerAbilityToken, resolved
    // once at the NAPI layer from options.context.)
    std::shared_ptr<AbilityRuntime::AbilityContext> abilityContext;
    // Main-thread hop for UI actions (public path only; may be null).
    MainThreadPoster mainPoster;
    // Pre-registered intent execute record id slot (0 = system path).
    int32_t intentRecordId = 0;

    // Settlement channel, created by the invoker and shared with the executor
    // and the execution callback client: the completed flag is the single
    // arbiter across the normal callback, failure paths and the timeout task;
    // only the first compare_exchange_strong winner may deliver a result.
    std::shared_ptr<std::atomic<bool>> completed;
    InvokeResultCallback callback;

    /**
     * @brief Whether the invocation has already been settled (by the timeout
     *        task, a failure path, or the callback client). Dispatch sites
     *        (query continuation, invoke stage) check this to skip work whose
     *        outcome the caller can no longer receive.
     */
    bool Settled() const
    {
        return completed != nullptr && completed->load();
    }

    /**
     * @brief Settle the invocation with an error through an explicit channel
     *        (completed + callback). CAS-guarded (see the completed field
     *        above): at most one delivery per call.
     */
    static void Settle(const std::shared_ptr<std::atomic<bool>> &completed,
        const InvokeResultCallback &callback, int32_t innerError)
    {
        if (completed == nullptr) {
            return;
        }
        bool expected = false;
        if (!completed->compare_exchange_strong(expected, true)) {
            return; // already settled by the normal callback or another failure
        }
        if (callback) {
            FunctionResultHolder holder;
            holder.innerError = innerError;
            callback(holder);
        }
    }

    /**
     * @brief Settle the invocation with an error. CAS-guarded (see the
     *        completed field above): at most one delivery per call.
     */
    void Fail(int32_t innerError) const
    {
        Settle(completed, callback, innerError);
    }
};
} // namespace CliTool
} // namespace OHOS

#endif // OHOS_CLI_TOOL_INVOKE_SESSION_H
