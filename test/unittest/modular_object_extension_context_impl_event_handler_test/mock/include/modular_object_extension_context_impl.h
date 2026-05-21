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

#ifndef MOCK_MODULAR_OBJECT_EXTENSION_CONTEXT_IMPL_H
#define MOCK_MODULAR_OBJECT_EXTENSION_CONTEXT_IMPL_H

#include <cstddef>
#include <memory>

#include "extension_context.h"

namespace OHOS {
namespace AbilityRuntime {

class ModularObjectExtensionContext : public ExtensionContext {
public:
    static const size_t CONTEXT_TYPE_ID;

    // Pure declarations — definitions come from real .cpp (linked via BUILD.gn)
    void SetEventHandler(const std::shared_ptr<AppExecFwk::EventHandler> &handler);

    std::shared_ptr<AppExecFwk::EventHandler> GetEventHandler() const;

    // Pure declarations — definitions come from real .cpp (linked via BUILD.gn)
    ErrCode StartSelfUIAbility(const AAFwk::Want &want) const;

    ErrCode StartSelfUIAbilityWithStartOptions(const AAFwk::Want &want,
        const AAFwk::StartOptions &startOptions) const;

    ErrCode TerminateSelf();

    bool IsContext(size_t contextTypeId) { return contextTypeId == CONTEXT_TYPE_ID; }

private:
    std::weak_ptr<AppExecFwk::EventHandler> handler_;
};

} // namespace AbilityRuntime
} // namespace OHOS

#endif // MOCK_MODULAR_OBJECT_EXTENSION_CONTEXT_IMPL_H
