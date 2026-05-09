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

#include "cli_manager_error_utils.h"

#include "cli_error_code.h"
#include "hilog_tag_wrapper.h"
#include "napi_common_util.h"

namespace OHOS {
namespace CliTool {
namespace {
static const std::map<CliManagerErrorCode, std::string> ERROR_MSG_MAP = {
    {CliManagerErrorCode::ERROR_PERMISSION_DENIED, "Permission denied."},
    {CliManagerErrorCode::ERROR_NOT_SYSTEM_APP, "Not system application."},
    {CliManagerErrorCode::ERROR_INVALID_PARAM, "Invalid input parameter."},
    {CliManagerErrorCode::ERROR_TOOL_NOT_FOUND, "The tool does not exist."},
    {CliManagerErrorCode::ERROR_REACH_LIMIT, "Maximum number of processes has been reached."},
    {CliManagerErrorCode::ERROR_SESSION_NOT_FOUND, "The session not found."},
    {CliManagerErrorCode::ERROR_SEND_MESSAGE, "Sending Message to failure."},
    {CliManagerErrorCode::ERROR_INNER, "Internal error."},
};

static const std::map<int32_t, CliManagerErrorCode> NATIVE_TO_BUSINESS_ERROR_MAP = {
    {ERR_PERMISSION_DENIED, CliManagerErrorCode::ERROR_PERMISSION_DENIED},
    {ERR_NOT_SYSTEM_APP, CliManagerErrorCode::ERROR_NOT_SYSTEM_APP},
    {ERR_INVALID_PARAM, CliManagerErrorCode::ERROR_INVALID_PARAM},
    {ERR_TOOL_NOT_EXIST, CliManagerErrorCode::ERROR_TOOL_NOT_FOUND},
    {ERR_SESSION_LIMIT_EXCEEDED, CliManagerErrorCode::ERROR_REACH_LIMIT},
    {ERR_CLI_SESSION_NOT_FOUND, CliManagerErrorCode::ERROR_SESSION_NOT_FOUND},
    {ERR_CLI_SEND_MESSAGE, CliManagerErrorCode::ERROR_SEND_MESSAGE},
};

std::string GetErrorMsg(CliManagerErrorCode errCode)
{
    auto it = ERROR_MSG_MAP.find(errCode);
    if (it != ERROR_MSG_MAP.end()) {
        return it->second;
    }
    return "Unknown error.";
}

CliManagerErrorCode GetBusinessErrorCode(int32_t nativeErr)
{
    auto it = NATIVE_TO_BUSINESS_ERROR_MAP.find(nativeErr);
    if (it != NATIVE_TO_BUSINESS_ERROR_MAP.end()) {
        return it->second;
    }
    return CliManagerErrorCode::ERROR_INNER;
}
}  // namespace

napi_value CreateCliManagerError(napi_env env, int32_t errCode, const std::string& errMsg)
{
    AbilityRuntime::HandleEscape handleEscape(env);
    napi_value result = nullptr;
    napi_value codeValue = AbilityRuntime::CreateJsValue(env, errCode);
    napi_value msgValue = AbilityRuntime::CreateJsValue(env, errMsg);
    napi_status status = napi_create_error(env, codeValue, msgValue, &result);
    TAG_LOGI(AAFwkTag::CLI_TOOL, "napi_create_error returned: %{public}d", status);
    return handleEscape.Escape(result);
}

napi_value CreateCliJsErrorByNativeErr(napi_env env, int32_t nativeErr)
{
    auto businessErrCode = GetBusinessErrorCode(nativeErr);
    auto errMsg = GetErrorMsg(businessErrCode);
    return CreateCliManagerError(env, static_cast<int32_t>(businessErrCode), errMsg);
}

} // namespace CliTool
} // namespace OHOS
