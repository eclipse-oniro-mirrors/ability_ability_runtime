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

#include "invoke_function_executor.h"

#include "ability_manager_errors.h"
#include "cli_error_code.h"
#include "cli_tool_mgr_client.h"
#include "ffrt.h"
#include "function_info.h"
#include "hilog_tag_wrapper.h"
#include "intent_client.h"
#include "invoke_function_callback_client.h"

namespace OHOS {
namespace CliTool {
namespace {
constexpr int32_t INVOKE_FUNCTION_TIMEOUT_US = 30000000;
} // namespace

std::shared_ptr<InvokeFunctionExecutor> InvokeFunctionExecutor::Create()
{
    return std::make_shared<InvokeFunctionExecutor>();
}

void InvokeFunctionExecutor::Execute(const std::string &funcNamespace, const std::string &functionName,
    const AAFwk::WantParams &wantParams, InvokeResultCallback callback)
{
    callback_ = std::move(callback);
    auto self = shared_from_this();
    SetupTimeout();
    ffrt::submit([self, funcNamespace, functionName, wantParams]() {
        self->DoExecute(funcNamespace, functionName, wantParams);
    });
}

void InvokeFunctionExecutor::SetupTimeout()
{
    auto self = shared_from_this();
    ffrt::submit([self]() {
        self->ReportError(ERR_FUNCTION_EXECUTE_TIMEOUT);
        }, ffrt::task_attr().delay(INVOKE_FUNCTION_TIMEOUT_US));
}

void InvokeFunctionExecutor::ReportError(int32_t errorCode)
{
    bool expected = false;
    if (completed_ == nullptr || !completed_->compare_exchange_strong(expected, true)) {
        return;  // already settled by the normal callback or another failure
    }
    InvokeFunctionResult out;
    out.success = false;
    out.errorCode = errorCode;
    if (callback_) {
        callback_(out);
    }
}

void InvokeFunctionExecutor::DoExecute(const std::string &funcNamespace, const std::string &functionName,
    const AAFwk::WantParams &wantParams)
{
    // Step 1: Query function info
    FunctionInfo functionInfo;
    ErrCode queryErr = CliToolMGRClient::GetInstance().GetFunctionInfo(funcNamespace, functionName, functionInfo);
    if (queryErr != ERR_OK) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "GetFunctionInfo failed: %{public}d", queryErr);
        // Preserve specific error semantics (permission / not-exist / not-system-app);
        // other errors fall through as inner errors.
        ReportError(queryErr);
        return;
    }

    // Step 2: Validate function type
    if (functionInfo.functionType != FunctionType::INTENT_FUNCTION) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Function type not supported: %{public}d",
            static_cast<int32_t>(functionInfo.functionType));
        ReportError(ERR_INNER_PARAM_INVALID);
        return;
    }

    // Step 3: Execute
    auto client = std::make_shared<InvokeFunctionCallbackClient>(completed_, callback_);

    AAFwk::ExecuteIntentParam param;
    param.bundleName = funcNamespace;
    param.intentName = functionName;
    param.wantParam = wantParams;
    param.callback = client;

    auto err = AAFwk::IntentClient::GetInstance().ExecuteIntentByFunctionCall(param);
    if (err != ERR_OK) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "ExecuteIntentByFunctionCall failed: %{public}d", err);
        int32_t reportErr = ERR_FUNCTION_EXECUTE_FAILED;
        if (err == OHOS::ERR_PERMISSION_DENIED) {
            reportErr = ERR_PERMISSION_DENIED;
        } else if (err == OHOS::AAFwk::ERR_NOT_SYSTEM_APP) {
            reportErr = ERR_NOT_SYSTEM_APP;
        }
        ReportError(reportErr);
    }
    // Success: result will come via InvokeFunctionCallbackClient::ProcessInsightIntentExecute
}

} // namespace CliTool
} // namespace OHOS
