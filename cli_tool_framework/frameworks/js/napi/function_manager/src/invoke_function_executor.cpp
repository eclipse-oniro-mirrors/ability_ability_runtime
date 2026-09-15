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
#include "function_result_wrap.h"
#include "hilog_tag_wrapper.h"
#include "intent_client.h"
#include "invoke_function_callback_client.h"
#include "invoke_function_param.h"

namespace OHOS {
namespace CliTool {
namespace {
constexpr int32_t INVOKE_FUNCTION_TIMEOUT_US = 30000000;
} // namespace

std::shared_ptr<InvokeFunctionExecutor> InvokeFunctionExecutor::Create()
{
    return std::make_shared<InvokeFunctionExecutor>();
}

void InvokeFunctionExecutor::Execute(const InvokeFunctionParam &param, InvokeResultCallback callback)
{
    callback_ = std::move(callback);
    auto self = shared_from_this();
    SetupTimeout();
    ffrt::submit([self, param]() {
        self->DoExecute(param);
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
    FunctionResultHolder holder;
    holder.innerError = errorCode;
    if (callback_) {
        callback_(holder);
    }
}

void InvokeFunctionExecutor::DoExecute(const InvokeFunctionParam &param)
{
    FunctionInfo functionInfo;
    ErrCode queryErr = CliToolMGRClient::GetInstance().GetFunctionInfo(
        param.functionNamespace, param.functionName, functionInfo);
    if (queryErr != ERR_OK) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "GetFunctionInfo failed: %{public}d", queryErr);
        ReportError(queryErr);
        return;
    }

    if (functionInfo.functionType != FunctionType::INTENT_FUNCTION) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Function type not supported: %{public}d",
            static_cast<int32_t>(functionInfo.functionType));
        ReportError(ERR_INNER_PARAM_INVALID);
        return;
    }

    InvokeFunctionParam hookParam = param;
    CliToolMGRClient::GetInstance().BeforeInvokeFunction(hookParam);

    AAFwk::WantParams hookParams = hookParam.args;
    auto self = shared_from_this();
    InvokeResultCallback wrappedCallback = [self](
        const FunctionResultHolder &holder) {
        FunctionResultWrap functionResultWrap;
        functionResultWrap.result = holder.result;
        CliToolMGRClient::GetInstance().AfterInvokeFunction(functionResultWrap);
        FunctionResultHolder out = holder;
        out.result = functionResultWrap.result;
        if (self->callback_) {
            self->callback_(out);
        }
    };
    auto client = std::make_shared<InvokeFunctionCallbackClient>(completed_, std::move(wrappedCallback));

    AAFwk::ExecuteIntentParam execParam;
    execParam.bundleName = hookParam.functionNamespace;
    execParam.intentName = hookParam.functionName;
    execParam.wantParam = hookParams;
    execParam.callback = client;

    auto err = AAFwk::IntentClient::GetInstance().ExecuteIntentByFunctionCall(execParam);
    if (err != ERR_OK) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "ExecuteIntentByFunctionCall failed: %{public}d", err);
        int32_t reportErr = ERR_FUNCTION_EXECUTE_FAILED;
        if (err == OHOS::ERR_PERMISSION_DENIED || err == OHOS::AAFwk::ERR_NOT_SYSTEM_APP) {
            reportErr = err;
        }
        ReportError(reportErr);
    }
}

} // namespace CliTool
} // namespace OHOS
