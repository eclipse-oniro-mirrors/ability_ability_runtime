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

/**
 * @addtogroup AbilityRuntime
 * @{
 *
 * @brief Provides the definition of the C interface for the modular object extension ability.
 *
 * @since 26.0.0
 */

/**
 * @file modular_object_extension_ability.h
 *
 * @brief Declares the modular object extension ability.
 *
 * @library libability_runtime.so
 * @kit AbilityKit
 * @syscap SystemCapability.Ability.AbilityRuntime.Core
 * @since 26.0.0
 */

#ifndef ABILITY_RUNTIME_MODULAR_OBJECT_EXTENSION_ABILITY_H
#define ABILITY_RUNTIME_MODULAR_OBJECT_EXTENSION_ABILITY_H

#include "ability_runtime_common.h"
#include "extension_ability.h"
#include "ipc_cparcel.h"
#include "modular_object_extension_context.h"
#include "want.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Defines the struct for OH_AbilityRuntime_ModObjExtensionInstance.
 *
 * @since 26.0.0
 */
typedef struct OH_AbilityRuntime_ModularObjectExtensionInstance OH_AbilityRuntime_ModObjExtensionInstance;

/**
 * @brief Defines the pointer to OH_AbilityRuntime_ModObjExtensionInstance.
 *
 * @since 26.0.0
 */
typedef OH_AbilityRuntime_ModObjExtensionInstance* OH_AbilityRuntime_ModObjExtensionInstanceHandle;

/**
 * @brief Callback invoked when a modular object extension is started for initialization.
 *
 * @param instance Points to an {@link OH_AbilityRuntime_ModObjExtensionInstance} instance.
 * @param want Indicates the want of created modular object extension.
 * For details, see {@link AbilityBase_Want}.
 *
 * @since 26.0.0
 */
typedef void (*OH_AbilityRuntime_ModObjExtensionAbility_OnCreateFunc)(
    OH_AbilityRuntime_ModObjExtensionInstanceHandle instance, AbilityBase_Want *want);

/**
 * @brief Callback invoked before a modular object extension is destroyed.
 *
 * @param instance Points to an {@link OH_AbilityRuntime_ModObjExtensionInstance} instance.
 *
 * @since 26.0.0
 */
typedef void (*OH_AbilityRuntime_ModObjExtensionAbility_OnDestroyFunc)(
    OH_AbilityRuntime_ModObjExtensionInstanceHandle instance);

/**
 * @brief Callback invoked when a modular object extension is connected to an ability.
 *
 * @param instance Points to an {@link OH_AbilityRuntime_ModObjExtensionInstance} instance.
 * @param want Indicates the want of created modular object extension.
 *
 * @since 26.0.0
 */
typedef OHIPCRemoteStub* (*OH_AbilityRuntime_ModObjExtensionAbility_OnConnectFunc)(
    OH_AbilityRuntime_ModObjExtensionInstanceHandle instance, AbilityBase_Want *want);

/**
 * @brief Callback invoked when all abilities connected to a modular object extension are
 * disconnected.
 *
 * @param instance Points to an {@link OH_AbilityRuntime_ModObjExtensionInstance} instance.
 *
 * @since 26.0.0
 */
typedef void (*OH_AbilityRuntime_ModObjExtensionAbility_OnDisconnectFunc)(
    OH_AbilityRuntime_ModObjExtensionInstanceHandle instance);

/**
 * @brief Registers the function {@link OH_AbilityRuntime_ModObjExtensionAbility_OnCreateFunc} with
 * {@link OH_AbilityRuntime_ModObjExtensionInstance}.
 *
 * @param instance Points to an {@link OH_AbilityRuntime_ModObjExtensionInstance} instance.
 * @param onCreateFunc Represents the onCreate callback function.
 * @return Returns a specific error code.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} success.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} parameter check failed.
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_ModObjExtensionAbility_RegisterOnCreateFunc(
    OH_AbilityRuntime_ModObjExtensionInstanceHandle instance,
    OH_AbilityRuntime_ModObjExtensionAbility_OnCreateFunc onCreateFunc);

/**
 * @brief Registers the function {@link OH_AbilityRuntime_ModObjExtensionAbility_OnDestroyFunc} with
 * {@link OH_AbilityRuntime_ModObjExtensionInstance}.
 *
 * @param instance Points to an {@link OH_AbilityRuntime_ModObjExtensionInstance} instance.
 * @param onDestroyFunc Represents the onDestroy callback function.
 * @return Returns a specific error code.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} success.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} parameter check failed.
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_ModObjExtensionAbility_RegisterOnDestroyFunc(
    OH_AbilityRuntime_ModObjExtensionInstanceHandle instance,
    OH_AbilityRuntime_ModObjExtensionAbility_OnDestroyFunc onDestroyFunc);

/**
 * @brief Registers the function {@link OH_AbilityRuntime_ModObjExtensionAbility_OnConnectFunc} with
 * {@link OH_AbilityRuntime_ModObjExtensionInstance}.
 *
 * @param instance Points to an {@link OH_AbilityRuntime_ModObjExtensionInstance} instance.
 * @param onConnectFunc Represents the onConnect callback function.
 * @return Returns a specific error code.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} success.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} parameter check failed.
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_ModObjExtensionAbility_RegisterOnConnectFunc(
    OH_AbilityRuntime_ModObjExtensionInstanceHandle instance,
    OH_AbilityRuntime_ModObjExtensionAbility_OnConnectFunc onConnectFunc);

/**
 * @brief Registers the function {@link OH_AbilityRuntime_ModObjExtensionAbility_OnDisconnectFunc} with
 * {@link OH_AbilityRuntime_ModObjExtensionInstance}.
 *
 * @param instance Points to an {@link OH_AbilityRuntime_ModObjExtensionInstance} instance.
 * @param onDisconnectFunc Represents the onDisconnect callback function.
 * @return Returns a specific error code.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} success.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} parameter check failed.
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_ModObjExtensionAbility_RegisterOnDisconnectFunc(
    OH_AbilityRuntime_ModObjExtensionInstanceHandle instance,
    OH_AbilityRuntime_ModObjExtensionAbility_OnDisconnectFunc onDisconnectFunc);


/**
 * @brief Gets the extension context from the modular object extension instance.
 *
 * @param instance Points to an {@link OH_AbilityRuntime_ModObjExtensionInstance} instance.
 * @param context Represents a pointer to the modular object extension ability context.
 * @return Returns a specific error code.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} success.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} parameter check failed.
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_ModObjExtensionAbility_GetContextFromInstance(
    OH_AbilityRuntime_ModObjExtensionInstanceHandle instance, OH_AbilityRuntime_ModObjExtensionContextHandle* context);

/**
 * @brief Gets the modular object extension instance from a base extension instance.
 *
 * @param baseExtensionInstance Represents a pointer to a {@link
 * AbilityRuntime_ExtensionInstance} base extension instance.
 * @param modObjExtensionInstance Represents a pointer to an {@link
 * OH_AbilityRuntime_ModObjExtensionInstanceHandle} instance that is an output parameter.
 * @return Returns a specific error code.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} success.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} parameter check failed.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_INCORRECT_ABILITY_TYPE} if the ability instance is not
 *         a modular object extension.
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_ModObjExtensionAbility_GetInstanceFromBase(
    AbilityRuntime_ExtensionInstanceHandle baseExtensionInstance,
    OH_AbilityRuntime_ModObjExtensionInstanceHandle* modObjExtensionInstance);

#ifdef __cplusplus
}
#endif

/** @} */
#endif // ABILITY_RUNTIME_MODULAR_OBJECT_EXTENSION_ABILITY_H