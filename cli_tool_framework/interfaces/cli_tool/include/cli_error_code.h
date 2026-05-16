/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License"),
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

#ifndef OHOS_ABILITY_RUNTIME_CLI_ERROR_CODE_H
#define OHOS_ABILITY_RUNTIME_CLI_ERROR_CODE_H

#include "errors.h"

namespace OHOS {
namespace CliTool {

enum {
    /*
     * Result (35700000) for Connect: An error of the get cli tool mgr service.
     */
    GET_CLI_TOOL_MGR_SERVICE_FAILED = 35700000,

    /*
     * Result (35700001): Session limit.
     */
    ERR_SESSION_LIMIT_EXCEEDED = 35700001,

    /*
     * Result (35700002): Invliad param.
     */
    ERR_INVALID_PARAM = 35700002,

    /*
     * Result (35700003): The caller is not hap.
     */
    ERR_NOT_HAP = 35700003,

    /*
     * Result (35700004): Fail to init.
     */
    ERR_NO_INIT = 35700004,

    /*
     * Result (35700005): Tool not exist.
     */
    ERR_TOOL_NOT_EXIST = 35700005,

    /*
     * Result (35700006): Fail to crate js CliSessionInfo.
     */
    ERR_INNER_PARAM_INVALID = 35700006,

    /*
     * Result (35700007): Permission denied.
     */
    ERR_PERMISSION_DENIED = 35700007,

    /*
     * Result (35700008): The caller is not system hap.
     */
    ERR_NOT_SYSTEM_APP = 35700008,

    ERR_CLI_SESSION_NOT_FOUND = 35700009,

    ERR_CLI_SEND_MESSAGE = 35700010,

    /*
     * Result (35700011): The caller is not SA.
     */
    ERR_NOT_SA_CALLER = 35700011,

    /*
     * Result (35700012): fail to kill.
     */
    ERR_NOT_KILL = 35700012,
};

} // namespace CliTool
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_CLI_ERROR_CODE_H