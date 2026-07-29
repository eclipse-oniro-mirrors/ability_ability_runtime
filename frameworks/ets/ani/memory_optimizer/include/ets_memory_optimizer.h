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

#ifndef OHOS_ABILITY_RUNTIME_ETS_MEMORY_OPTIMIZER_H
#define OHOS_ABILITY_RUNTIME_ETS_MEMORY_OPTIMIZER_H

#include "ani.h"

namespace OHOS {
namespace AbilityRuntime {

class EtsMemoryOptimizer {
public:
    static void EvictFilePagesCheck(ani_env *env, ani_object fileNames);
    static void EvictModuleFilePagesCheck(ani_env *env, ani_object moduleNames);
    static void EvictFilePages(ani_env *env, ani_object fileNames, ani_object callback);
    static void EvictModuleFilePages(ani_env *env, ani_object moduleNames, ani_object callback);
};

void EtsMemoryOptimizerInit(ani_env *env);
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_ETS_MEMORY_OPTIMIZER_H
