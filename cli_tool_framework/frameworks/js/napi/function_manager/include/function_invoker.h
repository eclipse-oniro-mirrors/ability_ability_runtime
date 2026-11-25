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

#ifndef OHOS_CLI_TOOL_FUNCTION_INVOKER_H
#define OHOS_CLI_TOOL_FUNCTION_INVOKER_H

#include <memory>

#include "function_info.h"
#include "invoke_session.h"

namespace OHOS {
namespace AbilityRuntime {
class AbilityContext;
} // namespace AbilityRuntime
namespace CliTool {

/**
 * @brief Orchestrator of one function invocation, created by the NAPI layer.
 *
 * Fixed pipeline on a worker thread (all stages but the UI hop, so no IPC
 * blocks the JS/UI main thread): query the function info, where isPublic
 * selects the public query (ACCESS_PUBLIC_FUNCTION + open-to-third-party
 * check on the service side) or the system query (system app +
 * ACCESS_FUNCTION); create the executor from the QUERIED functionType
 * (unsupported types settle with ERR_INNER_PARAM_INVALID); then hand off to
 * the executor's two-stage chain — consent (which may hop to the main
 * thread for the dialog), then invoke.
 *
 * The query runs BEFORE any executor exists (the type decides the executor),
 * so it lives here rather than in the executor base. The settlement channel
 * (completed flag + callback + timeout) is owned here and shared with the
 * executor chain and the callback client: exactly one delivery per call.
 */
class FunctionInvoker : public std::enable_shared_from_this<FunctionInvoker> {
public:
    /**
     * @param param            Parsed invocation parameters (namespace/name/args/options).
     * @param isPublic         Entry selector: invokePublicFunction (true) runs the
     *                         public query + consent path; invokeFunction (false)
     *                         the system path.
     * @param abilityContext   Caller UIAbilityContext hosting the consent dialog
     *                         (public path); may be null.
     * @param mainPoster       Main-thread hop for UI actions; may be null.
     */
    static std::shared_ptr<FunctionInvoker> Create(InvokeFunctionParam param, bool isPublic,
        const std::shared_ptr<AbilityRuntime::AbilityContext> &abilityContext,
        MainThreadPoster mainPoster);

    /**
     * @brief Start the invocation. Call exactly once, on the JS thread; the
     *        whole pipeline is posted to a worker thread. The timeout covers
     *        query + consent + execution from this moment on.
     * @param callback Invoked exactly once with the final outcome.
     */
    void Execute(InvokeResultCallback callback);

    // Public for std::make_shared inside Create(); the shared-from-this state
    // requires construction via std::shared_ptr (use FunctionInvoker::Create).
    FunctionInvoker(InvokeFunctionParam param, bool isPublic,
        std::shared_ptr<AbilityRuntime::AbilityContext> abilityContext, MainThreadPoster mainPoster);

private:
    void Run();
    void SetupTimeout();
    ErrCode QueryFunctionInfo(FunctionInfo &functionInfo) const;

    // All call parameters (incl. isPublic) + the settlement channel.
    InvokeSession session_;
};
} // namespace CliTool
} // namespace OHOS

#endif // OHOS_CLI_TOOL_FUNCTION_INVOKER_H
