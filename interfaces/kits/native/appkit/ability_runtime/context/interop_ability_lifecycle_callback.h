/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_CONTEXT_INTEROP_ABILITY_LIFECYCLE_CALLBACK_H
#define OHOS_ABILITY_RUNTIME_CONTEXT_INTEROP_ABILITY_LIFECYCLE_CALLBACK_H

#include <memory>

#include "interop_object.h"

namespace OHOS {
namespace AbilityRuntime {
class InteropAbilityLifecycleCallback {
public:
    virtual ~InteropAbilityLifecycleCallback() {}
    virtual void OnAbilityCreate(std::shared_ptr<InteropObject> ability) {}
    virtual void OnWindowStageCreate(std::shared_ptr<InteropObject> ability,
        std::shared_ptr<InteropObject> windowStage) {}
    virtual void OnWindowStageDestroy(std::shared_ptr<InteropObject> ability,
        std::shared_ptr<InteropObject> windowStage) {}
    virtual void OnAbilityDestroy(std::shared_ptr<InteropObject> ability) {}
    virtual void OnAbilityForeground(std::shared_ptr<InteropObject> ability) {}
    virtual void OnAbilityBackground(std::shared_ptr<InteropObject> ability) {}
};
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_CONTEXT_INTEROP_ABILITY_LIFECYCLE_CALLBACK_H