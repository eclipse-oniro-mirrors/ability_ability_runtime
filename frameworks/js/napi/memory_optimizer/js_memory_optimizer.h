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

#ifndef OHOS_ABILITY_RUNTIME_JS_MEMORY_OPTIMIZER_H
#define OHOS_ABILITY_RUNTIME_JS_MEMORY_OPTIMIZER_H

#include "js_runtime_utils.h"
#include "native_engine/native_engine.h"

namespace OHOS {
namespace AbilityRuntime {

class JsMemoryOptimizer {
public:
    JsMemoryOptimizer() = default;
    ~JsMemoryOptimizer() = default;

    static void Finalizer(napi_env env, void *data, void *hint);
    static napi_value EvictFilePages(napi_env env, napi_callback_info info);
    static napi_value EvictModuleFilePages(napi_env env, napi_callback_info info);

private:
    static bool CheckCallerIsSystemApp();

    napi_value OnEvictFilePages(napi_env env, NapiCallbackInfo &info);
    napi_value OnEvictModuleFilePages(napi_env env, NapiCallbackInfo &info);
};

napi_value JsMemoryOptimizerInit(napi_env env, napi_value exportObj);
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_JS_MEMORY_OPTIMIZER_H
