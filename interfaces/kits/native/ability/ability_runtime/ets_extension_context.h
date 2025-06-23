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

#ifndef OHOS_ABILITY_RUNTIME_STS_EXTENSION_CONTEXT_H
#define OHOS_ABILITY_RUNTIME_STS_EXTENSION_CONTEXT_H

#include "ani.h"
#include "extension_context.h"

namespace OHOS {
namespace AbilityRuntime {
struct STSNativeReference;
class EtsExtensionContext final {
public:
    static void ConfigurationUpdated(ani_env *env, const std::shared_ptr<STSNativeReference> &stsContext,
        const std::shared_ptr<AppExecFwk::Configuration> &config);
};

void CreatEtsExtensionContext(ani_env* aniEnv, ani_class contextClass, ani_object contextObj,
    std::shared_ptr<OHOS::AbilityRuntime::ExtensionContext> context,
    std::shared_ptr<OHOS::AppExecFwk::AbilityInfo> abilityInfo);
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_STS_EXTENSION_CONTEXT_H