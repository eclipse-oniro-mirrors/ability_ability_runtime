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
 * @brief Provides the definition of the C interface for the modular object extension context.
 *
 * @since 26.0.0
 */

/**
 * @file modular_object_extension_context.h
 *
 * @brief Declares the modular object extension context.
 *
 * @library libability_runtime.so
 * @kit AbilityKit
 * @syscap SystemCapability.Ability.AbilityRuntime.Core
 * @since 26.0.0
 */

#ifndef ABILITY_RUNTIME_MODULAR_OBJECT_EXTENSION_CONTEXT_H
#define ABILITY_RUNTIME_MODULAR_OBJECT_EXTENSION_CONTEXT_H

#include "ability_runtime_common.h"
#include "context.h"
#include "ipc_cremote_object.h"
#include "start_options.h"
#include "want.h"

#ifdef __cplusplus
extern "C" {
#endif
struct AbilityBase_Want;
typedef struct AbilityBase_Want AbilityBase_Want;

/**
 * @brief Defines a pointer type to OH_AbilityRuntime_ModObjExtensionContextHandle.
 *
 * @since 26.0.0
 */
typedef struct OH_AbilityRuntime_ModularObjectExtensionContext* OH_AbilityRuntime_ModObjExtensionContextHandle;

/**
 * @brief Gets the base context from the modular object extension context.
 *
 * @param modObjExtensionContext Represents a pointer to a modular object extension ability context.
 * @param baseContext Represents a pointer to a {@link AbilityRuntime_ContextHandle} base extension ability context.
 * @return Returns a specific error code.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} success.
 *         {@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} parameter check failed.
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_ModObjExtensionContext_GetBaseContext(
    OH_AbilityRuntime_ModObjExtensionContextHandle modObjExtensionContext, AbilityRuntime_ContextHandle* baseContext);

/**
 * @brief Starts the self UIAbility.
 *
 * @permission ohos.permission.NDK_START_SELF_UI_ABILITY
 * @param context Represents a pointer to a modular object extension ability context.
 * @param want The arguments passed to start the self UIAbility.
 * For details, see {@link AbilityBase_Want}.
 * @return Returns a specific error code.
 *          {@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if the call is successful.
 *          {@link ABILITY_RUNTIME_ERROR_CODE_PERMISSION_DENIED} if the caller has no correct permission.
 *          {@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if the arguments provided is invalid.
 *          {@link ABILITY_RUNTIME_ERROR_CODE_NOT_SUPPORTED} if the device does not support starting self UIAbility.
 *          {@link ABILITY_RUNTIME_ERROR_CODE_NO_SUCH_ABILITY} if the target ability does not exist.
 *          {@link ABILITY_RUNTIME_ERROR_CODE_INCORRECT_ABILITY_TYPE} if the ability type is incorrect.
 *          {@link ABILITY_RUNTIME_ERROR_CODE_CROWDTEST_EXPIRED} if the crowdtesting application expires.
 *          {@link ABILITY_RUNTIME_ERROR_CODE_WUKONG_MODE} if the ability cannot be started in Wukong mode.
 *          {@link ABILITY_RUNTIME_ERROR_CODE_CONTROLLED} if the app is controlled.
 *          {@link ABILITY_RUNTIME_ERROR_CODE_EDM_CONTROLLED} if the app is controlled by EDM.
 *          {@link ABILITY_RUNTIME_ERROR_CODE_CROSS_APP} if the caller tries to start a different application.
 *          {@link ABILITY_RUNTIME_ERROR_CODE_INTERNAL} if an internal error occurs.
 *          {@link ABILITY_RUNTIME_ERROR_CODE_NOT_TOP_ABILITY} if the caller is not top ability.
 *          {@link ABILITY_RUNTIME_ERROR_CODE_MULTI_APP_NOT_SUPPORTED} if the app clone or multi-instance is
            not supported.
 *          {@link ABILITY_RUNTIME_ERROR_CODE_INVALID_APP_INSTANCE_KEY} if the app instance key is invalid.
 *          {@link ABILITY_RUNTIME_ERROR_CODE_UPPER_LIMIT_REACHED} if the number of app instances has reached
            the limit.
 *          {@link ABILITY_RUNTIME_ERROR_MULTI_INSTANCE_NOT_SUPPORTED} if multi-instance is not supported.
 *          {@link ABILITY_RUNTIME_ERROR_CODE_APP_INSTANCE_KEY_NOT_SUPPORTED} if the APP_INSTANCE_KEY cannot
            be specified.
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_ModObjExtensionContext_StartSelfUIAbility(
    OH_AbilityRuntime_ModObjExtensionContextHandle context, const AbilityBase_Want *want);

/**
 * @brief Starts the self UIAbility with start options.
 *
 * @permission ohos.permission.NDK_START_SELF_UI_ABILITY
 * @param context Represents a pointer to a modular object extension ability context.
 * @param want The arguments passed to start the self UIAbility.
 * For details, see {@link AbilityBase_Want}.
 * @param options The start options passed to start the self UIAbility.
 * For details, see {@link AbilityRuntime_StartOptions}.
 * @return Returns a specific error code.
 *          {@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if the call is successful.
 *          {@link ABILITY_RUNTIME_ERROR_CODE_PERMISSION_DENIED} if the caller has no correct permission.
 *          {@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if the arguments provided are invalid.
 *          {@link ABILITY_RUNTIME_ERROR_CODE_NOT_SUPPORTED} if the device does not support starting self UIAbility.
 *          {@link ABILITY_RUNTIME_ERROR_CODE_NO_SUCH_ABILITY} if the target ability does not exist.
 *          {@link ABILITY_RUNTIME_ERROR_CODE_INCORRECT_ABILITY_TYPE} if the ability type is incorrect.
 *          {@link ABILITY_RUNTIME_ERROR_CODE_CROWDTEST_EXPIRED} if the crowdtesting application expires.
 *          {@link ABILITY_RUNTIME_ERROR_CODE_WUKONG_MODE} if the ability cannot be started in Wukong mode.
 *          {@link ABILITY_RUNTIME_ERROR_CODE_CONTROLLED} if the app is controlled.
 *          {@link ABILITY_RUNTIME_ERROR_CODE_EDM_CONTROLLED} if the app is controlled by EDM.
 *          {@link ABILITY_RUNTIME_ERROR_CODE_CROSS_APP} if the caller tries to start a different application.
 *          {@link ABILITY_RUNTIME_ERROR_CODE_INTERNAL} if an internal error occurs.
 *          {@link ABILITY_RUNTIME_ERROR_CODE_NOT_TOP_ABILITY} if the caller is not a foreground process.
 *          {@link ABILITY_RUNTIME_ERROR_VISIBILITY_SETTING_DISABLED} if setting visibility is disabled.
 *          {@link ABILITY_RUNTIME_ERROR_CODE_MULTI_APP_NOT_SUPPORTED} if the app clone or multi-instance is
            not supported.
 *          {@link ABILITY_RUNTIME_ERROR_CODE_INVALID_APP_INSTANCE_KEY} if the app instance key is invalid.
 *          {@link ABILITY_RUNTIME_ERROR_CODE_UPPER_LIMIT_REACHED} if the number of app instances has reached
            the limit.
 *          {@link ABILITY_RUNTIME_ERROR_MULTI_INSTANCE_NOT_SUPPORTED} if multi-instance is not supported.
 *          {@link ABILITY_RUNTIME_ERROR_CODE_APP_INSTANCE_KEY_NOT_SUPPORTED} if the APP_INSTANCE_KEY cannot
            be specified.
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_ModObjExtensionContext_StartSelfUIAbilityWithStartOptions(
    OH_AbilityRuntime_ModObjExtensionContextHandle context, const AbilityBase_Want *want,
    const AbilityRuntime_StartOptions *options);

/**
 * @brief Destroys the modular object extension.
 *
 * @param context Represents a pointer to a modular object extension ability context.
 * @return Returns a specific error code.
 *          {@link ABILITY_RUNTIME_ERROR_CODE_NO_ERROR} if the call is successful.
 *          {@link ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID} if the arguments provided are invalid.
 *          {@link ABILITY_RUNTIME_ERROR_CODE_WUKONG_MODE} if the ability cannot be started in Wukong mode.
 *          {@link ABILITY_RUNTIME_ERROR_CODE_CONTEXT_NOT_EXIST} if the context does not exist.
 *          {@link ABILITY_RUNTIME_ERROR_CODE_INTERNAL} if an internal error occurs.
 * @since 26.0.0
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_ModObjExtensionContext_TerminateSelf(
    OH_AbilityRuntime_ModObjExtensionContextHandle context);

#ifdef __cplusplus
}
#endif

/** @} */
#endif // ABILITY_RUNTIME_MODULAR_OBJECT_EXTENSION_CONTEXT_H