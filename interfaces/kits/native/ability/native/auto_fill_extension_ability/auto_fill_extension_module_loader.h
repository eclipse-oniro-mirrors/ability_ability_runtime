/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_AUTO_FILL_EXTENSION_MODULE_LOADER_H
#define OHOS_ABILITY_RUNTIME_AUTO_FILL_EXTENSION_MODULE_LOADER_H

#include "extension_module_loader.h"

namespace OHOS {
namespace AbilityRuntime {
class AutoFillExtensionModuleLoader : public ExtensionModuleLoader, public Singleton<AutoFillExtensionModuleLoader> {
    DECLARE_SINGLETON(AutoFillExtensionModuleLoader);

public:
    /**
     * @brief Create Extension.
     *
     * @param runtime The runtime.
     * @return The Extension instance.
     */
    Extension *Create(const std::unique_ptr<Runtime> &runtime) const override;

    /**
     * @brief Get the Params object
     *
     * @return std::map<std::string, std::string> The map of extension type and extension name.
     */
    std::map<std::string, std::string> GetParams() override;
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_AUTO_FILL_EXTENSION_MODULE_LOADER_H
