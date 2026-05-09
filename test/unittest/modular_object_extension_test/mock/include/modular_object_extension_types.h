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
#include "extension_ability_info.h"
#include "modular_object_extension_ability.h"
#include "native_extension/context_impl.h"
#include "native_extension/extension_ability_impl.h"

struct OH_AbilityRuntime_ModularObjectExtensionContext : public AbilityRuntime_Context {};

struct OH_AbilityRuntime_ModularObjectExtensionInstance : public AbilityRuntime_ExtensionInstance {
    std::shared_ptr<OH_AbilityRuntime_ModularObjectExtensionContext> context;
    OH_AbilityRuntime_ModObjExtensionAbility_OnCreateFunc onCreateFunc = nullptr;
    OH_AbilityRuntime_ModObjExtensionAbility_OnDestroyFunc onDestroyFunc = nullptr;
    OH_AbilityRuntime_ModObjExtensionAbility_OnConnectFunc onConnectFunc = nullptr;
    OH_AbilityRuntime_ModObjExtensionAbility_OnDisconnectFunc onDisconnectFunc = nullptr;
};

#endif // MOCK_MODULAR_OBJECT_EXTENSION_TYPES_H
