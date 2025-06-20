/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_STARTUP_UTILS_H
#define OHOS_ABILITY_RUNTIME_STARTUP_UTILS_H

#include <string>

#include "cJSON.h"
#include "errors.h"

namespace OHOS {
namespace AbilityRuntime {
enum {
    ERR_STARTUP_INVALID_VALUE = 401,
    ERR_STARTUP_INTERNAL_ERROR = 16000050,
    ERR_STARTUP_DEPENDENCY_NOT_FOUND = 28800001,
    ERR_STARTUP_CIRCULAR_DEPENDENCY = 28800002,
    ERR_STARTUP_FAILED_TO_EXECUTE_STARTUP = 28800003,
    ERR_STARTUP_TIMEOUT = 28800004,
    ERR_STARTUP_CONFIG_NOT_FOUND = 28800005,
    ERR_STARTUP_CONFIG_PATH_ERROR = 28800006,
    ERR_STARTUP_CONFIG_PARSE_ERROR = 28800007,
    ERR_NATIVE_MODULE_MANAGER_CONSTRUCTION = 28800008,
    ERR_LOAD_NATIVE_MODULE = 28800009,
    NAPI_CREATE_OBJECT_FAILED = 28800010,
};

class StartupUtils {
public:
    static std::string GetErrorMessage(int32_t errCode);
    static bool ParseJsonStringArray(const cJSON *json, const std::string key, std::vector<std::string> &array);
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_STARTUP_UTILS_H
