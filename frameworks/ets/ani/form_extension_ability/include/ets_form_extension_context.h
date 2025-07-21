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

#ifndef OHOS_ABILITY_RUNTIME_ETS_FORM_EXTENSION_CONTEXT_H
#define OHOS_ABILITY_RUNTIME_ETS_FORM_EXTENSION_CONTEXT_H

#include "ani.h"
#include "form_extension_context.h"

namespace OHOS {
namespace AbilityRuntime {
ani_ref CreateEtsFormExtensionContext(ani_env *env, std::shared_ptr<FormExtensionContext> &context);

class ETSFormExtensionContext {
public:
    static ani_ref CreateEtsExtensionContext(ani_env *env, const std::shared_ptr<FormExtensionContext> &context,
        std::shared_ptr<OHOS::AppExecFwk::AbilityInfo> &abilityInfo);

    static ani_object SetFormExtensionContext(ani_env *env, const std::shared_ptr<FormExtensionContext> &context);
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_ETS_FORM_EXTENSION_CONTEXT_H