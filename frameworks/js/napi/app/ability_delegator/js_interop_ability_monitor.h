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

#ifndef OHOS_ABILITY_RUNTIME_JS_INTEROP_ABILITY_MONITOR_H
#define OHOS_ABILITY_RUNTIME_JS_INTEROP_ABILITY_MONITOR_H

#include <memory>
#include <string>
#include "ability_delegator_infos.h"
#include "js_interop_object.h"
#include "native_engine/native_reference.h"

namespace OHOS {
namespace AbilityDelegatorJs {
class JsInteropAbilityMonitor {
public:
    explicit JsInteropAbilityMonitor(const std::string &abilityName);
    explicit JsInteropAbilityMonitor(const std::string &abilityName, const std::string &moduleName);
    ~JsInteropAbilityMonitor() = default;

    void OnAbilityCreate(const std::weak_ptr<AppExecFwk::BaseDelegatorAbilityProperty> &abilityObj);
    void OnAbilityForeground(const std::weak_ptr<AppExecFwk::BaseDelegatorAbilityProperty> &abilityObj);
    void OnAbilityBackground(const std::weak_ptr<AppExecFwk::BaseDelegatorAbilityProperty> &abilityObj);
    void OnAbilityDestroy(const std::weak_ptr<AppExecFwk::BaseDelegatorAbilityProperty> &abilityObj);
    void OnWindowStageCreate(const std::weak_ptr<AppExecFwk::BaseDelegatorAbilityProperty> &abilityObj);
    void OnWindowStageRestore(const std::weak_ptr<AppExecFwk::BaseDelegatorAbilityProperty> &abilityObj);
    void OnWindowStageDestroy(const std::weak_ptr<AppExecFwk::BaseDelegatorAbilityProperty> &abilityObj);

    void SetJsInteropAbilityMonitor(napi_env env, napi_value monitor);
    void SetAniEnv(void *aniEnv);

    std::unique_ptr<NativeReference> &GetJsInteropAbilityMonitor()
    {
        return jsInteropMonitor_;
    }

private:
    napi_value CallLifecycleCBFunction(const std::string &functionName,
        const std::weak_ptr<AppExecFwk::BaseDelegatorAbilityProperty> &abilityObj);
    napi_value ConvertAbilityToNapiValue(const std::weak_ptr<AppExecFwk::BaseDelegatorAbilityProperty> &abilityObj);

private:
    std::string abilityName_ = "";
    std::string moduleName_ = "";
    napi_env env_ = nullptr;
    void *aniEnvVoid_ = nullptr;
    std::unique_ptr<NativeReference> jsInteropMonitor_ = nullptr;
};
}  // namespace AbilityDelegatorJs
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_JS_INTEROP_ABILITY_MONITOR_H
