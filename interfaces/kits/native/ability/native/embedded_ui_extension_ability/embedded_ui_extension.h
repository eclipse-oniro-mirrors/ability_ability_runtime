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

#ifndef OHOS_ABILITY_RUNTIME_EMBEDDED_UI_EXTENSION_H
#define OHOS_ABILITY_RUNTIME_EMBEDDED_UI_EXTENSION_H

#include "ui_extension_base.h"

namespace OHOS {
namespace AbilityRuntime {
class UIExtensionContext;
class Runtime;
/**
 * @brief embedded UI extension components.
 */
class EmbeddedUIExtension
    : public UIExtensionBase<UIExtensionContext>, public std::enable_shared_from_this<EmbeddedUIExtension> {
public:
    EmbeddedUIExtension() = default;

    ~EmbeddedUIExtension() override = default;

    /**
     * @brief Create embedded UI extension.
     *
     * @param runtime The runtime.
     * @return The embedded UI extension instance.
     */
    static EmbeddedUIExtension *Create(const std::unique_ptr<Runtime> &runtime);
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_EMBEDDED_UI_EXTENSION_H