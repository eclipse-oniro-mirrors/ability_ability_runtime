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
 * @brief Provides the definition of the C interface for the modular object extension manager.
 *
 * @since 26.0.0
 */

/**
 * @file connect_options.h
 *
 * @brief Declares the connection options.
 *
 * @library libability_runtime.so
 * @kit AbilityKit
 * @syscap SystemCapability.Ability.AbilityRuntime.Core
 * @since 26.0.0
 */

#ifndef ABILITY_RUNTIME_CONNECT_OPTIONS_H
#define ABILITY_RUNTIME_CONNECT_OPTIONS_H

#include <stdint.h>
#include "ability_runtime_common.h"
#include "ipc_cparcel.h"
#include "want.h"

#ifdef __cplusplus
extern "C" {
#endif

struct AbilityBase_Want;
typedef struct AbilityBase_Want AbilityBase_Want;

struct AbilityBase_Element;
typedef struct AbilityBase_Element AbilityBase_Element;

/**
 * @brief Defines the OH_AbilityRuntime_ConnectOptions structure type.
 *
 * @since 26.0.0
 */
typedef struct OH_AbilityRuntime_ConnectOptions OH_AbilityRuntime_ConnectOptions;

/**
 * @brief The callback interface is invoked when the connection succeeds.
 *
 * @param connectOptions Represents a pointer to an {@link
 * OH_AbilityRuntime_ConnectOptions} instance.
 * @param element Represents the element name of the modular object extension ability.
 * @param proxy Represents the remote object instance.
 *
 * @since 26.0.0
 */
typedef void (*OH_AbilityRuntime_ConnectOptions_OnConnectCallback)(
    OH_AbilityRuntime_ConnectOptions *connectOptions, AbilityBase_Element *element, OHIPCRemoteProxy *proxy);

/**
 * @brief The callback interface is invoked when the disconnection occurs.
 *
 * @param connectOptions Represents a pointer to an {@link OH_AbilityRuntime_ConnectOptions} instance.
 * @param element Represents the element name of the modular object extension ability.
 *
 * @since 26.0.0
 */
typedef void (*OH_AbilityRuntime_ConnectOptions_OnDisconnectCallback)(
    OH_AbilityRuntime_ConnectOptions *connectOptions, AbilityBase_Element *element);

/**
 * @brief The callback interface is invoked when the connection fails.
 *
 * @param connectOptions Represents a pointer to an {@link OH_AbilityRuntime_ConnectOptions} instance.
 * @param code Represents the error code of the failure.
 *
 * @since 26.0.0
 */
typedef void (*OH_AbilityRuntime_ConnectOptions_OnFailedCallback)(
    OH_AbilityRuntime_ConnectOptions *connectOptions, AbilityRuntime_ErrorCode code);

/**
 * @brief Creates a ConnectOptions object.
 *
 *
 * @return Returns a newly created OH_AbilityRuntime_ConnectOptions object.
 * The caller is responsible for destroying the returned object by calling
 * {@link OH_AbilityRuntime_DestroyConnectOptions} to avoid memory leaks.
 * @since 26.0.0
 */
OH_AbilityRuntime_ConnectOptions* OH_AbilityRuntime_CreateConnectOptions();

/**
 * @brief Destroys the specified ConnectOptions.
 *
 * @param connectOptions The ConnectOptions object to be destroyed.
 * @return The error code.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if the operation is successful.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if the connectOptions is invalid.
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_DestroyConnectOptions(OH_AbilityRuntime_ConnectOptions *connectOptions);

/**
 * @brief Set the callback {@link OH_AbilityRuntime_ConnectOptions_OnConnectCallback} in
 * {@link OH_AbilityRuntime_ConnectOptions}.
 *
 * @param connectOptions Pointer to an {@link OH_AbilityRuntime_ConnectOptions} instance to be set.
 * @param onConnectCallback Represents {@link OH_AbilityRuntime_ConnectOptions_OnConnectCallback} instance
 * which will be set in.
 * @return Returns a specific error code.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} success.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} parameter check failed.
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_ConnectOptions_SetOnConnectCallback(
    OH_AbilityRuntime_ConnectOptions *connectOptions,
    OH_AbilityRuntime_ConnectOptions_OnConnectCallback onConnectCallback);

/**
 * @brief Set the callback {@link OH_AbilityRuntime_ConnectOptions_OnDisconnectCallback} in
 * {@link OH_AbilityRuntime_ConnectOptions}.
 *
 * @param connectOptions Pointer to an {@link OH_AbilityRuntime_ConnectOptions} instance to be set.
 * @param onDisconnectCallback Represents {@link OH_AbilityRuntime_ConnectOptions_OnDisconnectCallback} instance
 * which will be set in.
 * @return Returns a specific error code.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} success.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} parameter check failed.
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_ConnectOptions_SetOnDisconnectCallback(
    OH_AbilityRuntime_ConnectOptions *connectOptions,
    OH_AbilityRuntime_ConnectOptions_OnDisconnectCallback onDisconnectCallback);

/**
 * @brief Set the callback {@link OH_AbilityRuntime_ConnectOptions_OnFailedCallback} in
 * {@link OH_AbilityRuntime_ConnectOptions}.
 *
 * @param connectOptions Pointer to an {@link OH_AbilityRuntime_ConnectOptions} instance to be set.
 * @param onFailedCallback Represents {@link OH_AbilityRuntime_ConnectOptions_OnFailedCallback} instance
 * which will be set in.
 * @return Returns a specific error code.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} success.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} parameter check failed.
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_ConnectOptions_SetOnFailedCallback(
    OH_AbilityRuntime_ConnectOptions *connectOptions,
    OH_AbilityRuntime_ConnectOptions_OnFailedCallback onFailedCallback);

#ifdef __cplusplus
}
#endif

/** @} */
#endif // ABILITY_RUNTIME_CONNECT_OPTIONS_H