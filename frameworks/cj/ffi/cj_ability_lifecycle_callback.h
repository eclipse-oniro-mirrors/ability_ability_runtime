/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_CJ_CONTEXT_ABILITY_LIFECYCLE_CALLBACK_H
#define OHOS_ABILITY_RUNTIME_CJ_CONTEXT_ABILITY_LIFECYCLE_CALLBACK_H

#include <map>
#include <memory>
#include "cj_common_ffi.h"
#include "ability_lifecycle_callback.h"

using WindowStagePtr = void*;

namespace OHOS {
namespace AbilityRuntime {

class CjAbilityLifecycleCallback : public std::enable_shared_from_this<CjAbilityLifecycleCallback> {
public:
    explicit CjAbilityLifecycleCallback();
    void OnAbilityCreate(const int64_t &ability);
    void OnWindowStageCreate(const int64_t &ability, WindowStagePtr windowStage);
    void OnWindowStageActive(const int64_t &ability, WindowStagePtr windowStage);
    void OnWindowStageInactive(const int64_t &ability, WindowStagePtr windowStage);
    void OnWindowStageDestroy(const int64_t &ability, WindowStagePtr windowStage);
    void OnAbilityDestroy(const int64_t &ability);
    void OnAbilityForeground(const int64_t &ability);
    void OnAbilityBackground(const int64_t &ability);
    void OnAbilityContinue(const int64_t &ability);
    int32_t Register(CArrI64 cFuncIds, bool isSync = false);
    bool UnRegister(int32_t callbackId, bool isSync = false);
    bool IsEmpty() const;
    static int32_t serialNumber_;

private:
    std::map<int32_t, std::function<void(int64_t)>> onAbilityCreatecallbacks_;
    std::map<int32_t, std::function<void(int64_t, WindowStagePtr)>> onWindowStageCreatecallbacks_;
    std::map<int32_t, std::function<void(int64_t, WindowStagePtr)>> onWindowStageActivecallbacks_;
    std::map<int32_t, std::function<void(int64_t, WindowStagePtr)>> onWindowStageInactivecallbacks_;
    std::map<int32_t, std::function<void(int64_t, WindowStagePtr)>> onWindowStageDestroycallbacks_;
    std::map<int32_t, std::function<void(int64_t)>> onAbilityDestroycallbacks_;
    std::map<int32_t, std::function<void(int64_t)>> onAbilityForegroundcallbacks_;
    std::map<int32_t, std::function<void(int64_t)>> onAbilityBackgroundcallbacks_;
    std::map<int32_t, std::function<void(int64_t)>> onAbilityContinuecallbacks_;
};
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_CJ_CONTEXT_ABILITY_LIFECYCLE_CALLBACK_H