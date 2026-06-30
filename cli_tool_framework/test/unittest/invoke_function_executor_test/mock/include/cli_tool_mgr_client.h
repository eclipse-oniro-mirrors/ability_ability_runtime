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

#ifndef OHOS_ABILITY_RUNTIME_CLI_SA_CLIENT_H
#define OHOS_ABILITY_RUNTIME_CLI_SA_CLIENT_H

#include "errors.h"
#include "function_info.h"

namespace OHOS {
namespace CliTool {

/**
 * @class CliToolMGRClient
 * CliToolMGRClient provides client access to the CliSaService.
 * This is a singleton class that manages connection to the service.
 */
class CliToolMGRClient {
public:
    /**
     * @brief Get the singleton instance of CliToolMGRClient.
     * @return Reference to the CliToolMGRClient instance.
     */
    static CliToolMGRClient& GetInstance();

    /**
     * @brief Get function information by bundleName and functionName
     * @param bundleName Bundle name
     * @param functionName Function name
     * @param function Output FunctionInfo
     * @return ErrCode ERR_OK on success
     */
    ErrCode GetFunctionInfo(const std::string &bundleName, const std::string &functionName,
        FunctionInfo &function);

    // Test knobs (the singleton persists across tests; reset in SetUp).
    //   mockStatus_        — value returned by GetFunctionInfo (ERR_OK == success).
    //   mockFunctionType_  — function type surfaced on the success path, so tests can
    //                        drive the executor's type-validation branch (Step 2).
    int32_t mockStatus_ = 0;
    FunctionType mockFunctionType_ = FunctionType::INTENT_FUNCTION;
};

} // namespace CliTool
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_CLI_SA_CLIENT_H
