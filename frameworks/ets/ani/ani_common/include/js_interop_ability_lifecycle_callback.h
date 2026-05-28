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

#ifndef OHOS_ABILITY_RUNTIME_JS_INTEROP_ABILITY_LIFECYCLE_CALLBACK_H
#define OHOS_ABILITY_RUNTIME_JS_INTEROP_ABILITY_LIFECYCLE_CALLBACK_H

#include <node_api.h>
#include <vector>

#include "interop_ability_lifecycle_callback.h"

class NativeReference;

namespace OHOS {
namespace AbilityRuntime {
class JsInteropAbilityLifecycleCallback : public InteropAbilityLifecycleCallback,
    public std::enable_shared_from_this<JsInteropAbilityLifecycleCallback> {
public:
    JsInteropAbilityLifecycleCallback(napi_env env);

    void OnAbilityCreate(std::shared_ptr<InteropObject> ability) override;
    void OnWindowStageCreate(std::shared_ptr<InteropObject> ability,
        std::shared_ptr<InteropObject> windowStage) override;
    void OnWindowStageDestroy(std::shared_ptr<InteropObject> ability,
        std::shared_ptr<InteropObject> windowStage) override;
    void OnAbilityDestroy(std::shared_ptr<InteropObject> ability) override;
    void OnAbilityForeground(std::shared_ptr<InteropObject> ability) override;
    void OnAbilityBackground(std::shared_ptr<InteropObject> ability) override;
    void OnAbilityContinue(std::shared_ptr<InteropObject> ability) override;
    void OnWindowStageInactive(std::shared_ptr<InteropObject> ability,
        std::shared_ptr<InteropObject> windowStage) override;
    void OnWindowStageActive(std::shared_ptr<InteropObject> ability,
        std::shared_ptr<InteropObject> windowStage) override;
    void OnNewWant(std::shared_ptr<InteropObject> ability) override;
    void OnAbilityWillBackground(std::shared_ptr<InteropObject> ability) override;
    void OnAbilityWillForeground(std::shared_ptr<InteropObject> ability) override;
    void OnAbilityWillCreate(std::shared_ptr<InteropObject> ability) override;
    void OnWindowStageWillDestroy(std::shared_ptr<InteropObject> ability,
        std::shared_ptr<InteropObject> windowStage) override;
    void OnAbilityWillDestroy(std::shared_ptr<InteropObject> ability) override;
    void OnWindowStageRestore(std::shared_ptr<InteropObject> ability,
        std::shared_ptr<InteropObject> windowStage) override;
    void OnAbilitySaveState(std::shared_ptr<InteropObject> ability) override;
    void OnAbilityWillContinue(std::shared_ptr<InteropObject> ability) override;
    void OnAbilityWillSaveState(std::shared_ptr<InteropObject> ability) override;
    void OnWillNewWant(std::shared_ptr<InteropObject> ability) override;
    void OnWindowStageWillCreate(std::shared_ptr<InteropObject> ability,
        std::shared_ptr<InteropObject> windowStage) override;
    void OnWindowStageWillRestore(std::shared_ptr<InteropObject> ability,
        std::shared_ptr<InteropObject> windowStage) override;

    int32_t Register(void *callback) override;
    bool Unregister(void *jsVallback = nullptr) override;
    bool Empty() override;

private:
    void CallObjectMethod(const char *methodName, std::shared_ptr<InteropObject> ability,
        std::shared_ptr<InteropObject> windowStage = nullptr);

private:
    napi_env env_ = nullptr;
    std::vector<std::shared_ptr<NativeReference>> callbacks_;
};
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_JS_INTEROP_ABILITY_LIFECYCLE_CALLBACK_H