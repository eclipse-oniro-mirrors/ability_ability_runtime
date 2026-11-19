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

#ifndef OHOS_ABILITY_RUNTIME_ETS_INTEROP_ABILITY_MONITOR_H
#define OHOS_ABILITY_RUNTIME_ETS_INTEROP_ABILITY_MONITOR_H

#include <memory>
#include <string>
#include "ani.h"
#include "ets_native_reference.h"
#include "iinterop_ability_monitor.h"
#include <node_api.h>

namespace OHOS {
namespace AbilityDelegatorEts {
class EtsInteropAbilityMonitor : public AppExecFwk::IInteropAbilityMonitor {
public:
    explicit EtsInteropAbilityMonitor(const std::string &abilityName);
    explicit EtsInteropAbilityMonitor(const std::string &abilityName, const std::string &moduleName);
    ~EtsInteropAbilityMonitor() = default;

    void OnAbilityStart(const std::weak_ptr<AppExecFwk::BaseDelegatorAbilityProperty> &abilityObj) override;
    void OnAbilityForeground(const std::weak_ptr<AppExecFwk::BaseDelegatorAbilityProperty> &abilityObj) override;
    void OnAbilityBackground(const std::weak_ptr<AppExecFwk::BaseDelegatorAbilityProperty> &abilityObj) override;
    void OnAbilityStop(const std::weak_ptr<AppExecFwk::BaseDelegatorAbilityProperty> &abilityObj) override;
    void OnWindowStageCreate(const std::weak_ptr<AppExecFwk::BaseDelegatorAbilityProperty> &abilityObj) override;
    void OnWindowStageRestore(const std::weak_ptr<AppExecFwk::BaseDelegatorAbilityProperty> &abilityObj) override;
    void OnWindowStageDestroy(const std::weak_ptr<AppExecFwk::BaseDelegatorAbilityProperty> &abilityObj) override;

    void SetEtsInteropAbilityMonitor(ani_env *env, ani_object &monitorObj);
    void SetNapiEnv(napi_env env);

    std::unique_ptr<AppExecFwk::ETSNativeReference> &GetEtsInteropAbilityMonitor()
    {
        return etsInteropMonitor_;
    }

private:
    void CallLifecycleCBFunction(const std::string &functionName,
        const std::weak_ptr<AppExecFwk::BaseDelegatorAbilityProperty> &abilityObj);
    ani_object ConvertAbilityToAniRef(const std::weak_ptr<AppExecFwk::BaseDelegatorAbilityProperty> &abilityObj);
    ani_env *GetAniEnv();

    ani_vm *vm_ = nullptr;
    napi_env napiEnv_ = nullptr;
    std::string abilityName_;
    std::string moduleName_;
    std::unique_ptr<AppExecFwk::ETSNativeReference> etsInteropMonitor_;
};
}  // namespace AbilityDelegatorEts
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_ETS_INTEROP_ABILITY_MONITOR_H
