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

#ifndef MOCK_MODULAR_OBJECT_EXTENSION_TYPES_H
#define MOCK_MODULAR_OBJECT_EXTENSION_TYPES_H

#include <memory>
#include <string>
#include "connect_options_impl.h"

namespace OHOS {
namespace AbilityRuntime {

struct AbilityRuntime_Context {
    int type = 0;
    std::weak_ptr<void> context;
};

struct OH_AbilityRuntime_ModularObjectExtensionContext : public AbilityRuntime_Context {};

struct OH_AbilityRuntime_ExtensionInstance {
    int type = 0;
    std::weak_ptr<void> extension;
    std::shared_ptr<void> context;
};

struct OH_AbilityRuntime_ModularObjectExtensionInstance : public OH_AbilityRuntime_ExtensionInstance {
    void *onCreateFunc = nullptr;
    void *onDestroyFunc = nullptr;
    void *(*onConnectFunc)(void *, void *) = nullptr;
    void *onDisconnectFunc = nullptr;
};

} // namespace AbilityRuntime
} // namespace OHOS

#endif // MOCK_MODULAR_OBJECT_EXTENSION_TYPES_H
