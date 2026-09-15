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

#ifndef OHOS_CLI_TOOL_INVOKE_FUNCTION_EXECUTOR_H
#define OHOS_CLI_TOOL_INVOKE_FUNCTION_EXECUTOR_H

#include <atomic>
#include <functional>
#include <memory>
#include <string>

#include "invoke_function_param.h"
#include "invoke_function_result.h"

namespace OHOS {
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

class InvokeFunctionExecutor
    : public std::enable_shared_from_this<InvokeFunctionExecutor> {
public:
    InvokeFunctionExecutor() : completed_(std::make_shared<std::atomic<bool>>(false)) {}

    static std::shared_ptr<InvokeFunctionExecutor> Create();

    void Execute(const InvokeFunctionParam &param, InvokeResultCallback callback);

private:
    void DoExecute(const InvokeFunctionParam &param);
    void ReportError(int32_t errorCode);
    void SetupTimeout();

    std::shared_ptr<std::atomic<bool>> completed_;
    InvokeResultCallback callback_;
};

} // namespace CliTool
} // namespace OHOS

#endif // OHOS_CLI_TOOL_INVOKE_FUNCTION_EXECUTOR_H
