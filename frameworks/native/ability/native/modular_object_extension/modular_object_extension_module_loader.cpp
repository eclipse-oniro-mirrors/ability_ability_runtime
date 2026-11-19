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

#include "modular_object_extension_module_loader.h"

#include <string>

#include "modular_object_extension.h"

namespace OHOS::AbilityRuntime {
ModularObjectExtensionModuleLoader::ModularObjectExtensionModuleLoader() = default;
ModularObjectExtensionModuleLoader::~ModularObjectExtensionModuleLoader() = default;

Extension *ModularObjectExtensionModuleLoader::Create(const std::unique_ptr<Runtime> &runtime) const
{
    return ModularObjectExtension::Create();
}

std::map<std::string, std::string> ModularObjectExtensionModuleLoader::GetParams()
{
    std::map<std::string, std::string> params;
    params.insert(std::pair<std::string, std::string>("type", "39"));
    params.insert(std::pair<std::string, std::string>("name", "modularObject"));
    return params;
}

extern "C" __attribute__((visibility("default"))) void* OHOS_EXTENSION_GetExtensionModule()
{
    return &ModularObjectExtensionModuleLoader::GetInstance();
}
} // namespace OHOS::AbilityRuntime
