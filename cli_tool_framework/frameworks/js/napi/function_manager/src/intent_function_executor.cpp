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

#include "intent_function_executor.h"

#include "ability_manager_errors.h"
#include "cli_error_code.h"
#include "cli_tool_mgr_client.h"
#include "errors.h"
#include "exec_options.h"
#include "function_result_wrap.h"
#include "hilog_tag_wrapper.h"
#include "insight_intent/insight_intent_execute_param.h"
#include "intent_client.h"
#include "invoke_function_callback_client.h"
#include "invoke_function_param.h"
#include "long_wrapper.h"
#include "remote_object_wrapper.h"
#include "string_wrapper.h"
#include "want_params_wrapper.h"

namespace OHOS {
namespace CliTool {
namespace {
// Read a string-typed want parameter; returns an empty string when the key is
// absent or holds a non-string value.
std::string GetWantParamString(const AAFwk::WantParams &wantParams, const std::string &key)
{
    const auto &params = wantParams.GetParams();
    auto it = params.find(key);
    if (it == params.end() || it->second == nullptr) {
        return "";
    }
    auto *iStr = AAFwk::IString::Query(it->second);
    if (iStr == nullptr) {
        return "";
    }
    return AAFwk::String::Unbox(iStr);
}
} // namespace

IntentFunctionExecutor::IntentFunctionExecutor(InvokeSession session, FunctionInfo functionInfo,
    std::unique_ptr<ConsentGate> consentGate)
    : FunctionExecutor(std::move(session), std::move(functionInfo), std::move(consentGate))
{
}

// Run the before-invoke hook and capture the post-hook trace ids; invalid
// hook-modified ids are dropped.
IntentFunctionExecutor::PreparedInvoke IntentFunctionExecutor::PrepareInvoke(const InvokeFunctionParam &param)
{
    PreparedInvoke prepared;
    prepared.hookParam = param;
    CliToolMGRClient::GetInstance().BeforeInvokeFunction(prepared.hookParam);
    prepared.hookParams = prepared.hookParam.args;
    ExecutionIds &ids = prepared.executionIds;
    ids.toolCallId = GetWantParamString(prepared.hookParams, "ohos.insightIntent.toolCallId");
    ids.dmSessionId = GetWantParamString(prepared.hookParams, "ohos.insightIntent.dmSessionId");
    ids.taskId = GetWantParamString(prepared.hookParams, "ohos.task.id");
    if (!IsValidTraceId(ids.toolCallId) || !IsValidTraceId(ids.dmSessionId)) {
        TAG_LOGW(AAFwkTag::CLI_TOOL,
            "invokeFunction execute: invalid hook-modified trace id dropped, length: %{public}zu/%{public}zu",
            ids.toolCallId.length(), ids.dmSessionId.length());
        if (!IsValidTraceId(ids.toolCallId)) {
            ids.toolCallId.clear();
        }
        if (!IsValidTraceId(ids.dmSessionId)) {
            ids.dmSessionId.clear();
        }
    }
    // dmSessionId is stamped on the Wrap only, never logged here.
    if (!ids.toolCallId.empty()) {
        TAG_LOGI(AAFwkTag::CLI_TOOL, "invokeFunction execute, toolCallId=%{public}s",
            ids.toolCallId.c_str());
    }
    return prepared;
}

// Stamp the actual execution identifiers onto the after-hook Wrap. The
// callback is captured by value: it owns the settlement bridge, which keeps
// the delivery chain alive without pinning the executor itself.
InvokeResultCallback IntentFunctionExecutor::BuildWrappedCallback(const InvokeResultCallback &finalCallback,
    const ExecutionIds &executionIds)
{
    return [finalCallback, toolCallId = executionIds.toolCallId, dmSessionId = executionIds.dmSessionId](
        const FunctionResultHolder &holder) {
        FunctionResultWrap functionResultWrap;
        functionResultWrap.result = holder.result;
        functionResultWrap.toolCallId = toolCallId;
        functionResultWrap.dmSessionId = dmSessionId;
        CliToolMGRClient::GetInstance().AfterInvokeFunction(functionResultWrap);
        FunctionResultHolder out = holder;
        out.result = functionResultWrap.result;
        if (finalCallback != nullptr) {
            finalCallback(out);
        }
    };
}

void IntentFunctionExecutor::BuildExecuteIntentParam(const PreparedInvoke &prepared,
    const sptr<IRemoteObject> &callerAbilityToken, int32_t intentRecordId,
    const std::shared_ptr<AbilityRuntime::InsightIntentExecuteCallbackInterface> &client,
    AAFwk::ExecuteIntentParam &execParam)
{
    execParam.bundleName = prepared.hookParam.functionNamespace;
    execParam.intentName = prepared.hookParam.functionName;
    execParam.wantParam = prepared.hookParams;
    if (intentRecordId != 0) {
        execParam.wantParam.SetParam(AppExecFwk::INSIGHT_INTENT_EXECUTE_RECORD_ID,
            AAFwk::Long::Box64(static_cast<int64_t>(intentRecordId)));
    }
    if (!prepared.executionIds.toolCallId.empty()) {
        execParam.wantParam.SetParam("ohos.insightIntent.toolCallId",
            AAFwk::String::Box(prepared.executionIds.toolCallId));
    }
    if (!prepared.executionIds.dmSessionId.empty()) {
        execParam.wantParam.SetParam("ohos.insightIntent.dmSessionId",
            AAFwk::String::Box(prepared.executionIds.dmSessionId));
    }
    if (!prepared.executionIds.taskId.empty()) {
        execParam.wantParam.SetParam("ohos.task.id", AAFwk::String::Box(prepared.executionIds.taskId));
    }
    if (callerAbilityToken != nullptr) {
        // Serialize the caller token through the WantParamWrapper envelope. A bare
        // RemoteObjectWrap would be silently dropped by WantParams marshalling
        // (its WriteMarshalling has no IRemoteObjectWrap branch), so the remote
        // object must be nested under TYPE_PROPERTY / VALUE_PROPERTY.
        AAFwk::WantParams tokenWrap;
        tokenWrap.SetParam(AAFwk::TYPE_PROPERTY, AAFwk::String::Box(AAFwk::REMOTE_OBJECT));
        tokenWrap.SetParam(AAFwk::VALUE_PROPERTY, AAFwk::RemoteObjectWrap::Box(callerAbilityToken));
        execParam.wantParam.SetParam(AppExecFwk::INSIGHT_INTENT_CALLER_TOKEN,
            AAFwk::WantParamWrapper::Box(tokenWrap));
    }
    execParam.callback = client;
}

// Dispatch the intentExecution and map a synchronous failure to the settle
// error code: the two specific codes pass through, anything else collapses
// into ERR_FUNCTION_EXECUTE_FAILED.
ErrCode IntentFunctionExecutor::ExecuteIntent(const AAFwk::ExecuteIntentParam &execParam)
{
    auto err = AAFwk::IntentClient::GetInstance().ExecuteIntentByFunctionCall(execParam);
    if (err != ERR_OK) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "ExecuteIntentByFunctionCall failed: %{public}d", err);
        int32_t reportErr = ERR_FUNCTION_EXECUTE_FAILED;
        if (err == OHOS::ERR_PERMISSION_DENIED || err == OHOS::AAFwk::ERR_NOT_SYSTEM_APP) {
            reportErr = err;
        }
        return reportErr;
    }
    return ERR_OK;
}

ErrCode IntentFunctionExecutor::Invoke()
{
    // Worker-thread execution: hook preparation, callback client wiring, param
    // assembly and dispatch. The synchronous failure return is already mapped;
    // success (ERR_OK) settles asynchronously via the callback client.
    PreparedInvoke prepared = PrepareInvoke(session_.param);

    auto client = std::make_shared<InvokeFunctionCallbackClient>(session_.completed,
        BuildWrappedCallback(session_.callback, prepared.executionIds));

    AAFwk::ExecuteIntentParam execParam;
    // Box the caller token from the ORIGINAL param (pre-hook copy): the token rides
    // invokeOptions and is resolved once at the NAPI layer; the before-hook
    // round-trip only rewrites its own param copy, leaving this one intact.
    BuildExecuteIntentParam(prepared, session_.param.invokeOptions.callerAbilityToken,
        session_.intentRecordId, client, execParam);

    return ExecuteIntent(execParam);
}
} // namespace CliTool
} // namespace OHOS
