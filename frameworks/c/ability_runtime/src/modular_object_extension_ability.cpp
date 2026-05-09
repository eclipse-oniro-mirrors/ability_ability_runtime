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

#include "modular_object_extension_ability.h"

#include "hilog_tag_wrapper.h"
#include "modular_object_extension_types.h"

namespace {

AbilityRuntime_ErrorCode CheckMoeInstance(OH_AbilityRuntime_ModObjExtensionInstanceHandle instance,
    OH_AbilityRuntime_ModularObjectExtensionInstance **moeInstance)
{
    if (instance == nullptr || moeInstance == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "invalid params");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    auto *inner = reinterpret_cast<OH_AbilityRuntime_ModularObjectExtensionInstance *>(instance);
    if (inner->type != OHOS::AppExecFwk::ExtensionAbilityType::MODULAR_OBJECT) {
        TAG_LOGE(AAFwkTag::APPKIT, "invalid extension type");
        return ABILITY_RUNTIME_ERROR_CODE_INCORRECT_ABILITY_TYPE;
    }
    *moeInstance = inner;
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}
} // namespace

#ifdef __cplusplus
extern "C" {
#endif

AbilityRuntime_ErrorCode OH_AbilityRuntime_ModObjExtensionAbility_RegisterOnCreateFunc(
    OH_AbilityRuntime_ModObjExtensionInstanceHandle instance,
    OH_AbilityRuntime_ModObjExtensionAbility_OnCreateFunc onCreateFunc)
{
    OH_AbilityRuntime_ModularObjectExtensionInstance *inner = nullptr;
    auto ret = CheckMoeInstance(instance, &inner);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        return ret;
    }
    inner->onCreateFunc = onCreateFunc;
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_ModObjExtensionAbility_RegisterOnDestroyFunc(
    OH_AbilityRuntime_ModObjExtensionInstanceHandle instance,
    OH_AbilityRuntime_ModObjExtensionAbility_OnDestroyFunc onDestroyFunc)
{
    OH_AbilityRuntime_ModularObjectExtensionInstance *inner = nullptr;
    auto ret = CheckMoeInstance(instance, &inner);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        return ret;
    }
    inner->onDestroyFunc = onDestroyFunc;
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_ModObjExtensionAbility_RegisterOnConnectFunc(
    OH_AbilityRuntime_ModObjExtensionInstanceHandle instance,
    OH_AbilityRuntime_ModObjExtensionAbility_OnConnectFunc onConnectFunc)
{
    OH_AbilityRuntime_ModularObjectExtensionInstance *inner = nullptr;
    auto ret = CheckMoeInstance(instance, &inner);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        return ret;
    }
    inner->onConnectFunc = onConnectFunc;
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_ModObjExtensionAbility_RegisterOnDisconnectFunc(
    OH_AbilityRuntime_ModObjExtensionInstanceHandle instance,
    OH_AbilityRuntime_ModObjExtensionAbility_OnDisconnectFunc onDisconnectFunc)
{
    OH_AbilityRuntime_ModularObjectExtensionInstance *inner = nullptr;
    auto ret = CheckMoeInstance(instance, &inner);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        return ret;
    }
    inner->onDisconnectFunc = onDisconnectFunc;
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_ModObjExtensionAbility_GetContextFromInstance(
    OH_AbilityRuntime_ModObjExtensionInstanceHandle instance, OH_AbilityRuntime_ModObjExtensionContextHandle *context)
{
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null context");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    OH_AbilityRuntime_ModularObjectExtensionInstance *inner = nullptr;
    auto ret = CheckMoeInstance(instance, &inner);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        return ret;
    }
    if (inner->context == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null inner context");
        return ABILITY_RUNTIME_ERROR_CODE_CONTEXT_NOT_EXIST;
    }
    *context = inner->context.get();
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_ModObjExtensionAbility_GetInstanceFromBase(
    AbilityRuntime_ExtensionInstanceHandle baseExtensionInstance,
    OH_AbilityRuntime_ModObjExtensionInstanceHandle* modObjExtensionInstance)
{
    if (baseExtensionInstance == nullptr || modObjExtensionInstance == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "invalid params");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    if (baseExtensionInstance->type != OHOS::AppExecFwk::ExtensionAbilityType::MODULAR_OBJECT) {
        TAG_LOGE(AAFwkTag::APPKIT, "invalid extension type");
        return ABILITY_RUNTIME_ERROR_CODE_INCORRECT_ABILITY_TYPE;
    }
    *modObjExtensionInstance = reinterpret_cast<OH_AbilityRuntime_ModObjExtensionInstanceHandle>(baseExtensionInstance);
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

#ifdef __cplusplus
} // extern "C"
#endif
