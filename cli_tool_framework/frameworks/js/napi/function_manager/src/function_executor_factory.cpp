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

#include "function_executor_factory.h"

#include "consent_gate.h"
#include "hilog_tag_wrapper.h"
#include "intent_function_executor.h"

namespace OHOS {
namespace CliTool {

std::shared_ptr<FunctionExecutor> FunctionExecutorFactory::Create(const FunctionInfo &functionInfo,
    InvokeSession session)
{
    switch (functionInfo.functionType) {
        case FunctionType::INTENT_FUNCTION:
            return std::make_shared<IntentFunctionExecutor>(std::move(session), functionInfo,
                CreateConsentGate(functionInfo.functionType, session.isPublic));
        default:
            TAG_LOGE(AAFwkTag::CLI_TOOL, "function type not supported: %{public}d",
                static_cast<int32_t>(functionInfo.functionType));
            return nullptr;
    }
}

std::unique_ptr<ConsentGate> FunctionExecutorFactory::CreateConsentGate(FunctionType functionType,
    bool isPublic)
{
    // The consent stage applies to the public entry only; the system entry's
    // authorization happens on the service side, so it gets no gate at all
    // and the executor's consent default passes straight through.
    if (!isPublic) {
        return nullptr;
    }
    // One consent gate per public function type. INTENT_FUNCTION currently
    // uses the dialog-based gate; a future type with a different consent UX
    // branches here on the functionType.
    return std::make_unique<PublicConsentGate>();
}
} // namespace CliTool
} // namespace OHOS
