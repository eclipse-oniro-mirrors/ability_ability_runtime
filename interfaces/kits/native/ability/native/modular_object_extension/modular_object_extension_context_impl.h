/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_MODULAR_OBJECT_EXTENSION_CONTEXT_IMPL_H
#define OHOS_ABILITY_RUNTIME_MODULAR_OBJECT_EXTENSION_CONTEXT_IMPL_H

#include "extension_context.h"
#include "start_options.h"
#include "want.h"

namespace OHOS {
namespace AbilityRuntime {
class ModularObjectExtensionContext : public ExtensionContext {
public:
    ModularObjectExtensionContext() = default;
    ~ModularObjectExtensionContext() override = default;

    ErrCode StartSelfUIAbility(const AAFwk::Want &want) const;

    ErrCode StartSelfUIAbilityWithStartOptions(const AAFwk::Want &want, const AAFwk::StartOptions &startOptions) const;

    ErrCode TerminateSelf();

    static const size_t CONTEXT_TYPE_ID;

protected:
    bool IsContext(size_t contextTypeId) override
    {
        return contextTypeId == CONTEXT_TYPE_ID || ExtensionContext::IsContext(contextTypeId);
    }
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_MODULAR_OBJECT_EXTENSION_CONTEXT_IMPL_H
