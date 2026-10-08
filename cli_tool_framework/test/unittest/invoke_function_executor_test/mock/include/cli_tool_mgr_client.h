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

#ifndef OHOS_ABILITY_RUNTIME_CLI_SA_CLIENT_H
#define OHOS_ABILITY_RUNTIME_CLI_SA_CLIENT_H

#include <string>

#include "errors.h"
#include "function_info.h"
#include "invoke_function_param.h"
#include "function_result_wrap.h"

namespace OHOS {
namespace CliTool {

// Reserved wantParam keys that carry the trace identifiers into the executor
// (the transport leg the NAPI layer injects from invokeOptions).
constexpr const char *RESERVED_KEY_TOOL_CALL_ID = "ohos.insightIntent.toolCallId";
constexpr const char *RESERVED_KEY_DM_SESSION_ID = "ohos.insightIntent.dmSessionId";

/**
 * @class CliToolMGRClient
 * CliToolMGRClient provides client access to the CliSaService.
 * This is a singleton class that manages connection to the service.
 */
class CliToolMGRClient {
public:
    static CliToolMGRClient& GetInstance();

    ErrCode GetFunctionInfo(const std::string &bundleName, const std::string &functionName,
        FunctionInfo &function);

    // Simulates a registered before-hook rewriting the trace identifiers. The
    // rewrite covers both hook surfaces: the echo field (invokeOptions) and the
    // reserved transport key in args -- the channel the executor captures the
    // post-hook identifiers from.
    ErrCode BeforeInvokeFunction(InvokeFunctionParam &param);

    // Captures the FunctionResultWrap stamped by the executor with the actual
    // execution identifiers.
    ErrCode AfterInvokeFunction(FunctionResultWrap &functionResultWrap);

    int32_t mockStatus_ = 0;
    FunctionType mockFunctionType_ = FunctionType::INTENT_FUNCTION;

    // Before-hook knobs (reset by the test fixture SetUp).
    bool mockHookModify_ = false;
    std::string mockHookToolCallId_;
    std::string mockHookDmSessionId_;

    // After-hook capture (reset by the test fixture SetUp).
    FunctionResultWrap lastWrap_;
    bool afterHookCalled_ = false;
};

} // namespace CliTool
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_CLI_SA_CLIENT_H
