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

#include "cli_tool_mgr_client.h"

#include "errors.h"

namespace OHOS {
namespace CliTool {

CliToolMGRClient &CliToolMGRClient::GetInstance()
{
    static CliToolMGRClient instance;
    return instance;
}

ErrCode CliToolMGRClient::GetFunctionInfo(const std::string &bundleName, const std::string &functionName,
    FunctionInfo &function)
{
    if (mockStatus_ != ERR_OK) {
        return static_cast<ErrCode>(mockStatus_);  // simulate query failure
    }
    // On success, surface the configured function type so tests can drive the
    // executor's type-validation branch (Step 2 of DoExecute).
    function.functionType = mockFunctionType_;
    return ERR_OK;
}

} // namespace CliTool
} // namespace OHOS
