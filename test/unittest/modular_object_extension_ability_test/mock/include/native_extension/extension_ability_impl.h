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

#ifndef MOCK_NATIVE_EXTENSION_ABILITY_IMPL_H
#define MOCK_NATIVE_EXTENSION_ABILITY_IMPL_H

#include <memory>
#include "extension_ability_info.h"

namespace OHOS {
namespace AbilityRuntime {
class Extension {};
} // namespace AbilityRuntime
} // namespace OHOS

#ifdef __cplusplus
extern "C" {
#endif

struct AbilityRuntime_ExtensionInstance {
    OHOS::AppExecFwk::ExtensionAbilityType type;
    std::weak_ptr<OHOS::AbilityRuntime::Extension> extension;
};

typedef struct AbilityRuntime_ExtensionInstance *AbilityRuntime_ExtensionInstanceHandle;

#ifdef __cplusplus
}
#endif

#endif
