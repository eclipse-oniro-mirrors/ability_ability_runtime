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

#ifndef OHOS_ABILITY_RUNTIME_CLI_MANAGER_ERROR_UTILS_H
#define OHOS_ABILITY_RUNTIME_CLI_MANAGER_ERROR_UTILS_H

#include <native_engine/native_engine.h>
#include <string>

namespace OHOS {
namespace CliTool {

enum class CliManagerErrorCode {
    ERROR_PERMISSION_DENIED = 201,
    ERROR_NOT_SYSTEM_APP = 202,
    ERROR_INVALID_PARAM = 401,

    ERROR_TOOL_NOT_FOUND = 35600030,
    ERROR_REACH_LIMIT = 35600031,
    ERROR_SESSION_NOT_FOUND = 35600032,
    ERROR_SEND_MESSAGE = 35600033,

    ERROR_INNER = 35600050,
};

napi_value CreateCliManagerError(napi_env env, int32_t errCode, const std::string& errMsg = "");

napi_value CreateCliJsErrorByNativeErr(napi_env env, int32_t nativeErr);

}  // namespace CliTool
}  // namespace OHOS

#endif  // OHOS_ABILITY_RUNTIME_CLI_MANAGER_ERROR_UTILS_H