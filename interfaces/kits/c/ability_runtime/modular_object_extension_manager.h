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
 * @file modular_object_extension_manager.h
 *
 * @brief Declares the modular object extension manager.
 *
 * @library libability_runtime.so
 * @kit AbilityKit
 * @syscap SystemCapability.Ability.AbilityRuntime.Core
 * @since 26.0.0
 */

#ifndef ABILITY_RUNTIME_MODULAR_OBJECT_EXTENSION_MANAGER_H
#define ABILITY_RUNTIME_MODULAR_OBJECT_EXTENSION_MANAGER_H

#include <stddef.h>

#include "ability_runtime_common.h"
#include "connect_options.h"
#include "context.h"
#include "want_manager.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Defines the pointer to OH_AbilityRuntime_ModularObjectExtensionInfo.
 *
 * @since 26.0.0
 */
typedef struct OH_AbilityRuntime_ModularObjectExtensionInfo* OH_AbilityRuntime_ModObjExtensionInfoHandle;

/**
 * @brief Defines the pointer to OH_AbilityRuntime_AllModularObjectExtensionInfos.
 *
 * @since 26.0.0
 */
typedef struct OH_AbilityRuntime_AllModularObjectExtensionInfos* OH_AbilityRuntime_AllModObjExtensionInfosHandle;

/**
 * @brief The launch mode of a modular object extension.
 * @since 26.0.0
 */
typedef enum OH_AbilityRuntime_LaunchMode {
    /**
     * @brief All extensions under the same bundle share a single process.
     * @since 26.0.0
     */
    OH_ABILITY_RUNTIME_LAUNCH_MODE_IN_PROCESS = 0,

    /**
     * @brief Allow modular object extension abilities to be started across processes.
     * @since 26.0.0
     */
    OH_ABILITY_RUNTIME_LAUNCH_MODE_CROSS_PROCESS = 1,
} OH_AbilityRuntime_LaunchMode;

/**
 * @brief The process mode of a modular object extension.
 * @since 26.0.0
 */
typedef enum OH_AbilityRuntime_ProcessMode {
    /**
     * @brief All modular object extensions under the same bundle share a single process.
     * @since 26.0.0
     */
    OH_ABILITY_RUNTIME_PROCESS_MODE_BUNDLE = 0,

    /**
     * @brief Modular object extensions with the same name share a single process.
     * @since 26.0.0
     */
    OH_ABILITY_RUNTIME_PROCESS_MODE_TYPE = 1,

    /**
     * @brief Each modular object extension instance is a process.
     * @since 26.0.0
     */
    OH_ABILITY_RUNTIME_PROCESS_MODE_INSTANCE = 2,
} OH_AbilityRuntime_ProcessMode;

/**
 * @brief The thread mode of a modular object extension.
 * @since 26.0.0
 */
typedef enum OH_AbilityRuntime_ThreadMode {
    /**
     * @brief All modular object extensions under the same bundle share a single thread.
     * @since 26.0.0
     */
    OH_ABILITY_RUNTIME_THREAD_MODE_BUNDLE = 0,

    /**
     * @brief Modular object extensions with the same name share a single thread.
     * @since 26.0.0
     */
    OH_ABILITY_RUNTIME_THREAD_MODE_TYPE = 1,

    /**
     * @brief Each modular object extension instance is a thread.
     * @since 26.0.0
     */
    OH_ABILITY_RUNTIME_THREAD_MODE_INSTANCE = 2,
} OH_AbilityRuntime_ThreadMode;

/**
 * @brief Gets the launch mode from modular object extension info.
 *
 * @param extensionInfo The modular object extension info from which to get the launch mode.
 * @param launchMode The launch mode obtained from the extension info.
 * @return Returns a specific error code.
 *          {@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if the operation is successful.
 *          {@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if the arguments provided are invalid.
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_GetModularObjectExtensionInfoLaunchMode(
    OH_AbilityRuntime_ModObjExtensionInfoHandle extensionInfo, OH_AbilityRuntime_LaunchMode *launchMode);

/**
 * @brief Gets the process mode from modular object extension info.
 *
 * @param extensionInfo The modular object extension info to get the process mode from.
 * @param processMode The process mode obtained from the extension info.
 * @return Returns a specific error code.
 *          {@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if the operation is successful.
 *          {@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if the arguments provided are invalid.
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_GetModularObjectExtensionInfoProcessMode(
    OH_AbilityRuntime_ModObjExtensionInfoHandle extensionInfo, OH_AbilityRuntime_ProcessMode *processMode);

/**
 * @brief Gets the thread mode from modular object extension info.
 *
 * @param extensionInfo The modular object extension info to get the thread mode from.
 * @param threadMode The thread mode obtained from the extension info.
 * @return Returns a specific error code.
 *          {@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if the operation is successful.
 *          {@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if the arguments provided are invalid.
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_GetModularObjectExtensionInfoThreadMode(
    OH_AbilityRuntime_ModObjExtensionInfoHandle extensionInfo, OH_AbilityRuntime_ThreadMode *threadMode);

/**
 * @brief Gets elementName from modular object extension info.
 *
 * @param extensionInfo The modular object extension info to get the element name from.
 * @param element The element name obtained from the extension info.
 * @return Returns a specific error code.
 *          {@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if the operation is successful.
 *          {@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if the arguments provided are invalid.
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_GetModularObjectExtensionInfoElementName(
    OH_AbilityRuntime_ModObjExtensionInfoHandle extensionInfo, AbilityBase_Element *element);

/**
 * @brief Gets the disable state of modular object extension.
 *
 * @param extensionInfo The modular object extension info.
 * @param isDisabled Whether the extension is disabled.
 * @return Returns a specific error code.
 *          {@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if the operation is successful.
 *          {@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if the arguments provided are invalid.
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_GetModularObjectExtensionInfoDisableState(
    OH_AbilityRuntime_ModObjExtensionInfoHandle extensionInfo, bool *isDisabled);

/**
 * @brief Acquires all modular object extension infos within the self application.
 *
 * @param outOwnedAllExtensionInfos Information about all extensions within the self application.
 * @return Returns a specific error code.
 *          {@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if the operation is successful.
 *          {@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if the arguments provided are invalid.
 *          {@link ABILITY_RUNTIME_ERROR_CODE_NOT_SUPPORTED} if the device does not support.
 * @release OH_AbilityRuntime_ReleaseAllExtensionInfos {outOwnedAllExtensionInfos}
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_AcquireSelfModularObjectExtensionInfos(
    OH_AbilityRuntime_AllModObjExtensionInfosHandle *outOwnedAllExtensionInfos);

/**
 * @brief Releases the specified all modular object extension infos.
 *
 * @param allExtensionInfos The all modular object extension infos to be released.
 * @return Returns a specific error code.
 *          {@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if the operation is successful.
 *          {@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if the arguments provided are invalid.
 * @see OH_AbilityRuntime_AcquireSelfModularObjectExtensionInfos
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_ReleaseAllExtensionInfos(
    OH_AbilityRuntime_AllModObjExtensionInfosHandle *allExtensionInfos);

/**
 * @brief Gets the exact count of modular object extension infos present in the collection.
 *
 * @param allExtensionInfos The handle representing the collection of all modular object extension infos.
 * @param count Return value to receive the count of extensions in the collection.
 * @return Returns a specific error code.
 *          {@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if the operation is successful.
 *          {@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if the arguments provided are invalid.
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_GetCountFromAllModObjExtensionInfos(
    OH_AbilityRuntime_AllModObjExtensionInfosHandle allExtensionInfos, size_t *count);

/**
 * @brief Retrieves a specific modular object extension info handle from the collection by its index.
 *
 * @param allExtensionInfos Information about all extensions within the self application.
 * @param index The index of the extension to retrieve. Must be strictly less than the count.
 * @param extensionInfo The retrieved single modular object extension info handle for the specified index.
 * @return Returns a specific error code.
 *          {@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if the operation is successful.
 *          {@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if the arguments provided are invalid.
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_GetModObjExtensionInfoByIndex(
    OH_AbilityRuntime_AllModObjExtensionInfosHandle allExtensionInfos, size_t index,
    OH_AbilityRuntime_ModObjExtensionInfoHandle *extensionInfo);

/**
 * @brief Connect to a modular object extension ability
 *
 * @param want Indicates the service extension to connect.
 * For details, see {@link AbilityBase_Want}.
 * @param connectOptions Indicates the connection options.
 * @param connectionId Indicates the connection id that is a output param.
 * @return Returns a specific error code.
 *          {@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if the call is successful.
 *          {@link ABILITY_RUNTIME_ERROR_CODE_PERMISSION_DENIED} if the caller has no correct permission.
 *          {@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if the arguments provided are invalid.
 *          {@link ABILITY_RUNTIME_ERROR_CODE_NOT_SUPPORTED} if the device does not support connecting modular
 *          object extension ability.
 *          {@link ABILITY_RUNTIME_ERROR_CODE_NO_SUCH_ABILITY} if the target ability does not exist.
 *          {@link ABILITY_RUNTIME_ERROR_CODE_INCORRECT_ABILITY_TYPE} if the ability type is incorrect.
 *          {@link ABILITY_RUNTIME_ERROR_CODE_VISIBILITY_VERIFICATION_FAILED} - Cannot start an invisible component.
 *          {@link ABILITY_RUNTIME_ERROR_CODE_STATIC_CFG_PERMISSION} The specified process does not have
            the permission.
 *          {@link ABILITY_RUNTIME_ERROR_CODE_CROSS_USER_OPERATION} Cross-user operations are not allowed.
 *          {@link ABILITY_RUNTIME_ERROR_CODE_CROWDTEST_EXPIRED} if the crowdtesting application expires.
 *          {@link ABILITY_RUNTIME_ERROR_CODE_INTERNAL} if an internal error occurs.
 *          {@link ABILITY_RUNTIME_ERROR_CODE_NOT_TOP_ABILITY} if the caller is not a foreground process.
 *          {@lin ABILITY_RUNTIME_ERROR_CODE_UPPER_LIMIT_REACHED} The number of ability instances is
 *          more than twenty.
 *          {@link ABILITY_RUNTIME_ERROR_CODE_NO_RUNNING_ABILITIES_WITH_UI} if the target application does not have
 *          running abilities with UI.
 *          {@link ABILITY_RUNTIME_ERROR_CODE_UPPER_RATE_LIMIT} The API call frequency is too high and
 *          exceeds 20 times per second.
 *          {@link ABILITY_RUNTIME_ERROR_CODE_UPPER_CONNECTION_NUMBER_LIMIT} The number of connections exceeds five.
 *          {@link ABILITY_RUNTIME_ERROR_CODE_INVALID_DISTRIBUTION_TYPE} The distribution type of
 *          application is invalid.
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_ConnectModularObjectExtensionAbility(AbilityBase_Want *want,
    OH_AbilityRuntime_ConnectOptions *connectOptions, int64_t *connectionId);

/**
 * @brief Disconnect the modular object extension ability
 *
 * @param connectionId Indicates the connection ID.
 * @return Returns a specific error code.
 *          {@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if the call is successful.
 *          {@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if the arguments provided are invalid.
 *          {@link ABILITY_RUNTIME_ERROR_CODE_INTERNAL} if an internal error occurs.
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_DisconnectModularObjectExtensionAbility(int64_t connectionId);

#ifdef __cplusplus
}
#endif

/** @} */
#endif // ABILITY_RUNTIME_MODULAR_OBJECT_EXTENSION_MANAGER_H
