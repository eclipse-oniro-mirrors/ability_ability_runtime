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

#include "function_invoker.h"

#include "cli_error_code.h"
#include "cli_tool_mgr_client.h"
#include "errors.h"
#include "ffrt.h"
#include "function_executor_factory.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace CliTool {
namespace {
constexpr int32_t INVOKE_FUNCTION_TIMEOUT_US = 30000000;
} // namespace

std::shared_ptr<FunctionInvoker> FunctionInvoker::Create(InvokeFunctionParam param, bool isPublic,
    const std::shared_ptr<AbilityRuntime::AbilityContext> &abilityContext,
    MainThreadPoster mainPoster)
{
    return std::make_shared<FunctionInvoker>(std::move(param), isPublic, abilityContext,
        std::move(mainPoster));
}

FunctionInvoker::FunctionInvoker(InvokeFunctionParam param, bool isPublic,
    std::shared_ptr<AbilityRuntime::AbilityContext> abilityContext, MainThreadPoster mainPoster)
{
    session_.param = std::move(param);
    session_.isPublic = isPublic;
    session_.abilityContext = abilityContext;
    session_.mainPoster = std::move(mainPoster);
}

void FunctionInvoker::Execute(InvokeResultCallback callback)
{
    session_.completed = std::make_shared<std::atomic<bool>>(false);
    session_.callback = std::move(callback);
    SetupTimeout();
    auto self = shared_from_this();
    ffrt::submit([self]() {
        self->Run();
    });
}

void FunctionInvoker::SetupTimeout()
{
    auto completed = session_.completed;
    auto callback = session_.callback;
    ffrt::submit([completed, callback]() {
        InvokeSession::Settle(completed, callback, ERR_FUNCTION_EXECUTE_TIMEOUT);
        }, ffrt::task_attr().delay(INVOKE_FUNCTION_TIMEOUT_US));
}

ErrCode FunctionInvoker::QueryFunctionInfo(FunctionInfo &functionInfo) const
{
    // Public path: GetPublicFunctionInfo enforces the ACCESS_PUBLIC_FUNCTION
    // permission and the open-to-third-party (visible) check on the service
    // side; the system path keeps the original GetFunctionInfo (system app +
    // ACCESS_FUNCTION) semantics.
    if (session_.isPublic) {
        return CliToolMGRClient::GetInstance().GetPublicFunctionInfo(
            session_.param.functionNamespace, session_.param.functionName, functionInfo);
    }
    return CliToolMGRClient::GetInstance().GetFunctionInfo(
        session_.param.functionNamespace, session_.param.functionName, functionInfo);
}

void FunctionInvoker::Run()
{
    FunctionInfo functionInfo;
    ErrCode queryErr = QueryFunctionInfo(functionInfo);
    if (queryErr != ERR_OK) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "query function info failed: %{public}d", queryErr);
        session_.Fail(queryErr);
        return;
    }

    // Guard against dispatch-after-settle: a slow query (or ffrt backlog) may
    // have let the 30s timeout settle first — do not build an executor and
    // dispatch execution whose outcome the caller can no longer receive.
    if (session_.Settled()) {
        TAG_LOGW(AAFwkTag::CLI_TOOL, "invoke skipped: invocation already settled");
        return;
    }

    // The queried functionType decides the executor class; the type check
    // lives in the factory (unknown types return nullptr).
    auto executor = FunctionExecutorFactory::Create(functionInfo, session_);
    if (executor == nullptr) {
        session_.Fail(ERR_INNER_PARAM_INVALID);
        return;
    }
    executor->Execute();
}
} // namespace CliTool
} // namespace OHOS
