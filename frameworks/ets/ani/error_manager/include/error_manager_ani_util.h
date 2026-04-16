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

#ifndef OHOS_ABILITY_RUNTIME_ERROR_MANAGER_UTIL_H
#define OHOS_ABILITY_RUNTIME_ERROR_MANAGER_UTIL_H

#include <string>
#include "ani.h"

namespace OHOS {
namespace AbilityRuntime {
bool IsRefUndefined(ani_env *env, ani_ref ref);
bool IsNull(ani_env *env, ani_ref ref);
ani_env* GetAniEnv(ani_vm *vm);
ani_vm* GetAniVm(ani_env *env);
ani_object CreateErrorObject(ani_env *env, const std::string &name,
    const std::string &message, const std::string &stack);
bool SetPropertyByName(ani_env *env, ani_object &error, const std::string &value,
    const char *name);
} // namespace AbilityRuntime
} // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_ERROR_MANAGER_UTIL_H
