/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_ETS_ABILITY_STAGE_CONTEXT_H
#define OHOS_ABILITY_RUNTIME_ETS_ABILITY_STAGE_CONTEXT_H

#include "ani.h"
#include "configuration.h"
#include "ets_runtime.h"

namespace OHOS {
namespace AppExecFwk {
class OHOSApplication;
}
namespace AbilityRuntime {
constexpr const char* ETS_ABILITY_STAGE_CONTEXT_CLASS_NAME = "application.AbilityStageContext.AbilityStageContext";
constexpr const char* ETS_ABILITY_STAGE_CLASS_NAME = "@ohos.app.ability.AbilityStage.AbilityStage";
constexpr const char* ETS_HAPMODULEINFO_CLASS_NAME = "bundleManager.HapModuleInfoInner.HapModuleInfoInner";

class Context;
class ETSAbilityStageContext final {
public:
    explicit ETSAbilityStageContext(const std::shared_ptr<Context> &context) : context_(context) {}
    ~ETSAbilityStageContext() = default;

    static void ConfigurationUpdated(ani_env *env, const std::shared_ptr<AppExecFwk::Configuration> &config);

    std::shared_ptr<Context> GetContext()
    {
        return context_.lock();
    }
    static ani_object CreateEtsAbilityStageContext(ani_env *env, std::shared_ptr<Context> context);
private:
    static void SetConfiguration(ani_env *env, ani_class stageCls, ani_object stageCtxObj,
        std::shared_ptr<Context> &context);
    static void SetLaunchElement(ani_env *env, ani_class stageCls, ani_object stageCtxObj,
        std::shared_ptr<Context> &context);
    static ani_object CreateHapModuleInfo(ani_env* env, const std::shared_ptr<Context> &context);
private:
    std::weak_ptr<Context> context_;
    static ani_ref etsAbilityStageContextObj_;
};

}  // namespace AbilityRuntime
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_ETS_ABILITY_STAGE_CONTEXT_H
