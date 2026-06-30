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

#include "invoke_function_callback_client.h"

#include "cli_error_code.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace CliTool {

InvokeFunctionCallbackClient::InvokeFunctionCallbackClient(
    std::shared_ptr<CompletedFlag> completed, InvokeResultCallback callback)
    : completed_(std::move(completed)), callback_(std::move(callback)) {}

void InvokeFunctionCallbackClient::ProcessInsightIntentExecute(int32_t resultCode,
    AppExecFwk::InsightIntentExecuteResult executeResult)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "invokeFunction result callback, resultCode=%{public}d", resultCode);
    if (completed_ == nullptr) {
        return;
    }

    bool expected = false;
    if (!completed_->compare_exchange_strong(expected, true)) {
        TAG_LOGW(AAFwkTag::CLI_TOOL, "invokeFunction already completed");
        return;
    }
    InvokeFunctionResult out;
    out.invokeSuccess = (resultCode == 0);
    out.errorCode = (resultCode == 0) ? 0 : ERR_FUNCTION_EXECUTE_FAILED;
    out.resultCode = executeResult.code;  // app-level code drives InvokeResult
    out.result = executeResult.BuildFunctionResult();
    if (callback_) {
        callback_(out);
    }
}

} // namespace CliTool
} // namespace OHOS
