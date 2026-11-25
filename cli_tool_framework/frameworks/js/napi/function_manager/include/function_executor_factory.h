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

#ifndef OHOS_CLI_TOOL_FUNCTION_EXECUTOR_FACTORY_H
#define OHOS_CLI_TOOL_FUNCTION_EXECUTOR_FACTORY_H

#include <memory>

#include "function_executor.h"
#include "function_info.h"
#include "invoke_session.h"

namespace OHOS {
namespace CliTool {

/**
 * @brief Creates the type-specific executor from the QUERIED function info.
 *
 * The factory is the single composition point of the (entry, type) pair: the
 * functionType decides the executor class, and the entry decides whether a
 * consent gate exists at all (public entry only) and which concrete gate the
 * type uses. A new function type plugs in as one new FunctionExecutor
 * subclass plus one branch (or registry entry) here — the invoker, the
 * two-stage consent chain and the system entry are reused unchanged.
 */
class FunctionExecutorFactory {
public:
    /**
     * @param functionInfo Queried function info (functionType selects the class).
     * @param session      Invocation parameters (incl. isPublic) + settlement.
     * @return The executor, or nullptr for an unsupported function type (the
     *         caller settles with ERR_INNER_PARAM_INVALID).
     */
    static std::shared_ptr<FunctionExecutor> Create(const FunctionInfo &functionInfo,
        InvokeSession session);

private:
    /**
     * @brief Selects the consent gate for one invocation. The consent stage
     *        applies to the public entry only: the system entry gets NO gate
     *        (its authorization happens on the service side), and the public
     *        gate may vary by functionType.
     * @param functionType The queried function type.
     * @param isPublic     Entry selector (invokePublicFunction / invokeFunction).
     * @return The gate, or null when the entry needs no consent stage.
     */
    static std::unique_ptr<ConsentGate> CreateConsentGate(FunctionType functionType, bool isPublic);
};
} // namespace CliTool
} // namespace OHOS

#endif // OHOS_CLI_TOOL_FUNCTION_EXECUTOR_FACTORY_H
