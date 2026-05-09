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

#ifndef OHOS_ABILITY_RUNTIME_INTEROP_ABILITY_MONITOR_H
#define OHOS_ABILITY_RUNTIME_INTEROP_ABILITY_MONITOR_H

#include <memory>
#include "iinterop_ability_monitor.h"
#include "js_interop_ability_monitor.h"
#include "native_engine/native_reference.h"

namespace OHOS {
namespace AbilityDelegatorJs {
using namespace OHOS::AppExecFwk;
class InteropAbilityMonitor : public IInteropAbilityMonitor {
public:
    InteropAbilityMonitor(const std::string &name,
        const std::shared_ptr<JsInteropAbilityMonitor> &jsInteropAbilityMonitor);

    InteropAbilityMonitor(const std::string &name, const std::string &moduleName,
        const std::shared_ptr<JsInteropAbilityMonitor> &jsInteropAbilityMonitor);

    ~InteropAbilityMonitor() = default;

    void OnAbilityStart(const std::weak_ptr<BaseDelegatorAbilityProperty> &abilityObj) override;
    void OnAbilityForeground(const std::weak_ptr<BaseDelegatorAbilityProperty> &abilityObj) override;
    void OnAbilityBackground(const std::weak_ptr<BaseDelegatorAbilityProperty> &abilityObj) override;
    void OnAbilityStop(const std::weak_ptr<BaseDelegatorAbilityProperty> &abilityObj) override;
    void OnWindowStageCreate(const std::weak_ptr<BaseDelegatorAbilityProperty> &abilityObj) override;
    void OnWindowStageRestore(const std::weak_ptr<BaseDelegatorAbilityProperty> &abilityObj) override;
    void OnWindowStageDestroy(const std::weak_ptr<BaseDelegatorAbilityProperty> &abilityObj) override;

private:
    std::shared_ptr<JsInteropAbilityMonitor> jsInteropMonitor_;
};
}  // namespace AbilityDelegatorJs
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_INTEROP_ABILITY_MONITOR_H
