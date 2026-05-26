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
    virtual int32_t Register(void *callback) = 0;
    virtual bool Unregister(void *callback = nullptr) = 0;
    virtual bool Empty() = 0;
    virtual void OnAbilityCreate(std::shared_ptr<InteropObject> ability) {}
    virtual void OnWindowStageCreate(std::shared_ptr<InteropObject> ability,
        std::shared_ptr<InteropObject> windowStage) {}
    virtual void OnWindowStageDestroy(std::shared_ptr<InteropObject> ability,
        std::shared_ptr<InteropObject> windowStage) {}
    virtual void OnAbilityDestroy(std::shared_ptr<InteropObject> ability) {}
    virtual void OnAbilityForeground(std::shared_ptr<InteropObject> ability) {}
    virtual void OnAbilityBackground(std::shared_ptr<InteropObject> ability) {}
    virtual void OnAbilityContinue(std::shared_ptr<InteropObject> ability) {}
    virtual void OnWindowStageInactive(std::shared_ptr<InteropObject> ability,
        std::shared_ptr<InteropObject> windowStage) {}
    virtual void OnWindowStageActive(std::shared_ptr<InteropObject> ability,
        std::shared_ptr<InteropObject> windowStage) {}
    virtual void OnNewWant(std::shared_ptr<InteropObject> ability) {}
    virtual void OnAbilityWillBackground(std::shared_ptr<InteropObject> ability) {}
    virtual void OnAbilityWillForeground(std::shared_ptr<InteropObject> ability) {}
    virtual void OnAbilityWillCreate(std::shared_ptr<InteropObject> ability) {}
    virtual void OnWindowStageWillDestroy(std::shared_ptr<InteropObject> ability,
        std::shared_ptr<InteropObject> windowStage) {}
    virtual void OnAbilityWillDestroy(std::shared_ptr<InteropObject> ability) {}
    virtual void OnWindowStageRestore(std::shared_ptr<InteropObject> ability,
        std::shared_ptr<InteropObject> windowStage) {}
    virtual void OnAbilitySaveState(std::shared_ptr<InteropObject> ability) {}
    virtual void OnAbilityWillContinue(std::shared_ptr<InteropObject> ability) {}
    virtual void OnAbilityWillSaveState(std::shared_ptr<InteropObject> ability) {}
    virtual void OnWillNewWant(std::shared_ptr<InteropObject> ability) {}
    virtual void OnWindowStageWillCreate(std::shared_ptr<InteropObject> ability,
        std::shared_ptr<InteropObject> windowStage) {}
    virtual void OnWindowStageWillRestore(std::shared_ptr<InteropObject> ability,
        std::shared_ptr<InteropObject> windowStage) {}
};
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_CONTEXT_INTEROP_ABILITY_LIFECYCLE_CALLBACK_H