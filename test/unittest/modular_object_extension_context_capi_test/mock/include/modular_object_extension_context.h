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

#ifndef MOCK_MODULAR_OBJECT_EXTENSION_CONTEXT_H
#define MOCK_MODULAR_OBJECT_EXTENSION_CONTEXT_H

#include "ability_runtime_common.h"

struct AbilityBase_Want;
typedef struct AbilityBase_Want AbilityBase_Want;
struct AbilityRuntime_StartOptions;
typedef struct AbilityRuntime_StartOptions AbilityRuntime_StartOptions;

typedef struct OH_AbilityRuntime_ModularObjectExtensionContext *OH_AbilityRuntime_ModObjExtensionContextHandle;

#ifdef __cplusplus
extern "C" {
#endif

AbilityRuntime_ErrorCode OH_AbilityRuntime_ModObjExtensionContext_GetBaseContext(
    OH_AbilityRuntime_ModObjExtensionContextHandle modObjExtensionContext, AbilityRuntime_ContextHandle* baseContext);

AbilityRuntime_ErrorCode OH_AbilityRuntime_ModObjExtensionContext_StartSelfUIAbility(
    OH_AbilityRuntime_ModObjExtensionContextHandle context, const AbilityBase_Want *want);

AbilityRuntime_ErrorCode OH_AbilityRuntime_ModObjExtensionContext_StartSelfUIAbilityWithStartOptions(
    OH_AbilityRuntime_ModObjExtensionContextHandle context, const AbilityBase_Want *want,
    const AbilityRuntime_StartOptions *options);

AbilityRuntime_ErrorCode OH_AbilityRuntime_ModObjExtensionContext_TerminateSelf(
    OH_AbilityRuntime_ModObjExtensionContextHandle context);

#ifdef __cplusplus
}
#endif

#endif // MOCK_MODULAR_OBJECT_EXTENSION_CONTEXT_H
