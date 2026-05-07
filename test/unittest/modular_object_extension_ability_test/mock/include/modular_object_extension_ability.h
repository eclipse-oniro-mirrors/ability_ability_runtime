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

#ifndef MOCK_MODULAR_OBJECT_EXTENSION_ABILITY_H
#define MOCK_MODULAR_OBJECT_EXTENSION_ABILITY_H

#include "ability_runtime_common.h"
#include "extension_ability.h"
#include "ipc_cparcel.h"
#include "modular_object_extension_context.h"
#include "want.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct OH_AbilityRuntime_ModularObjectExtensionInstance OH_AbilityRuntime_ModObjExtensionInstance;
typedef OH_AbilityRuntime_ModObjExtensionInstance *OH_AbilityRuntime_ModObjExtensionInstanceHandle;

typedef void (*OH_AbilityRuntime_ModObjExtensionAbility_OnCreateFunc)(
    OH_AbilityRuntime_ModObjExtensionInstanceHandle instance, AbilityBase_Want *want);
typedef void (*OH_AbilityRuntime_ModObjExtensionAbility_OnDestroyFunc)(
    OH_AbilityRuntime_ModObjExtensionInstanceHandle instance);
typedef OHIPCRemoteStub *(*OH_AbilityRuntime_ModObjExtensionAbility_OnConnectFunc)(
    OH_AbilityRuntime_ModObjExtensionInstanceHandle instance, AbilityBase_Want *want);
typedef void (*OH_AbilityRuntime_ModObjExtensionAbility_OnDisconnectFunc)(
    OH_AbilityRuntime_ModObjExtensionInstanceHandle instance);

AbilityRuntime_ErrorCode OH_AbilityRuntime_ModObjExtensionAbility_RegisterOnCreateFunc(
    OH_AbilityRuntime_ModObjExtensionInstanceHandle instance,
    OH_AbilityRuntime_ModObjExtensionAbility_OnCreateFunc onCreateFunc);
AbilityRuntime_ErrorCode OH_AbilityRuntime_ModObjExtensionAbility_RegisterOnDestroyFunc(
    OH_AbilityRuntime_ModObjExtensionInstanceHandle instance,
    OH_AbilityRuntime_ModObjExtensionAbility_OnDestroyFunc onDestroyFunc);
AbilityRuntime_ErrorCode OH_AbilityRuntime_ModObjExtensionAbility_RegisterOnConnectFunc(
    OH_AbilityRuntime_ModObjExtensionInstanceHandle instance,
    OH_AbilityRuntime_ModObjExtensionAbility_OnConnectFunc onConnectFunc);
AbilityRuntime_ErrorCode OH_AbilityRuntime_ModObjExtensionAbility_RegisterOnDisconnectFunc(
    OH_AbilityRuntime_ModObjExtensionInstanceHandle instance,
    OH_AbilityRuntime_ModObjExtensionAbility_OnDisconnectFunc onDisconnectFunc);
AbilityRuntime_ErrorCode OH_AbilityRuntime_ModObjExtensionAbility_GetContextFromInstance(
    OH_AbilityRuntime_ModObjExtensionInstanceHandle instance,
    OH_AbilityRuntime_ModObjExtensionContextHandle *context);
AbilityRuntime_ErrorCode OH_AbilityRuntime_ModObjExtensionAbility_GetInstanceFromBase(
    AbilityRuntime_ExtensionInstanceHandle baseExtensionInstance,
    OH_AbilityRuntime_ModObjExtensionInstanceHandle *modObjExtensionInstance);

#ifdef __cplusplus
}
#endif

#endif
