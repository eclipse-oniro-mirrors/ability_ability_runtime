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

#ifndef OHOS_FUNCTION_JS_FUNCTION_MANAGER_H
#define OHOS_FUNCTION_JS_FUNCTION_MANAGER_H

#include "native_engine/native_engine.h"

namespace OHOS {
namespace CliTool {

/**
 * @class JSFunctionManager
 * @brief JS API wrapper for function management functionality.
 *
 * Provides native methods for querying functions.
 */
class JSFunctionManager final {
public:
    JSFunctionManager() {}
    ~JSFunctionManager() {}

    /**
     * @brief Finalizer for the JSFunctionManager object.
     *
     * @param env The N-API environment.
     * @param data The pointer to the JSFunctionManager instance.
     * @param hint The hint data.
     */
    static void Finalizer(napi_env env, void *data, void *hint);

    /**
     * @brief Native method for querying all functions.
     *
     * @param env The N-API environment.
     * @param info The N-API callback info.
     * @return Returns the N-API value.
     */
    static napi_value QueryFunctions(napi_env env, napi_callback_info info);

private:
    /**
     * @brief Implementation for querying all functions.
     *
     * @param env The N-API environment.
     * @param argc The argument count.
     * @param argv The argument values.
     * @return Returns the N-API value.
     */
    napi_value OnQueryFunctions(napi_env env, size_t argc, napi_value *argv);
};

/**
 * @brief Initialize the JSFunctionManager module.
 *
 * @param env The N-API environment.
 * @param exportObj The export object.
 * @return Returns the N-API value.
 */
napi_value JSFunctionManagerInit(napi_env env, napi_value exportObj);

} // namespace CliTool
} // namespace OHOS

#endif // OHOS_FUNCTION_JS_FUNCTION_MANAGER_H
