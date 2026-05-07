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

#include "modular_object_extension_context.h"

#include "ability_business_error_utils.h"
#include "ability_manager_client.h"
#include "hilog_tag_wrapper.h"
#include "modular_object_extension_context_impl.h"
#include "modular_object_extension_types.h"
#include "start_options_impl.h"
#include "want_manager.h"
#include "want_utils.h"

using namespace OHOS;
using namespace OHOS::AAFwk;
using namespace OHOS::AbilityRuntime;

namespace {
AbilityRuntime_ErrorCode CheckMoeContext(OH_AbilityRuntime_ModObjExtensionContextHandle context,
    std::shared_ptr<OHOS::AbilityRuntime::Context> &contextPtr)
{
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null context");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    if (context->type != AppExecFwk::ExtensionAbilityType::MODULAR_OBJECT) {
        TAG_LOGE(AAFwkTag::APPKIT, "invalid extension type");
        return ABILITY_RUNTIME_ERROR_CODE_INCORRECT_ABILITY_TYPE;
    }
    contextPtr = context->context.lock();
    if (contextPtr == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "context not exist");
        return ABILITY_RUNTIME_ERROR_CODE_CONTEXT_NOT_EXIST;
    }
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode TransformWant(const AbilityBase_Want *want, Want &abilityWant)
{
    auto ret = CheckWant(const_cast<AbilityBase_Want *>(want));
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        TAG_LOGE(AAFwkTag::APPKIT, "invalid want");
        return ret;
    }
    auto errCode = CWantManager::TransformToWant(*want, false, abilityWant);
    if (errCode != ABILITY_BASE_ERROR_CODE_NO_ERROR) {
        TAG_LOGE(AAFwkTag::APPKIT, "transform want failed: %{public}d", errCode);
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}
} // namespace

#ifdef __cplusplus
extern "C" {
#endif

AbilityRuntime_ErrorCode OH_AbilityRuntime_ModObjExtensionContext_GetBaseContext(
    OH_AbilityRuntime_ModObjExtensionContextHandle modObjExtensionContext, AbilityRuntime_ContextHandle* baseContext)
{
    if (modObjExtensionContext == nullptr || baseContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "invalid params");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    if (modObjExtensionContext->type != AppExecFwk::ExtensionAbilityType::MODULAR_OBJECT) {
        TAG_LOGE(AAFwkTag::APPKIT, "invalid extension type");
        return ABILITY_RUNTIME_ERROR_CODE_INCORRECT_ABILITY_TYPE;
    }
    *baseContext = static_cast<AbilityRuntime_ContextHandle>(modObjExtensionContext);
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_ModObjExtensionContext_StartSelfUIAbility(
    OH_AbilityRuntime_ModObjExtensionContextHandle context, const AbilityBase_Want *want)
{
    std::shared_ptr<OHOS::AbilityRuntime::Context> contextPtr;
    auto ret = CheckMoeContext(context, contextPtr);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        return ret;
    }
    Want abilityWant;
    ret = TransformWant(want, abilityWant);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        return ret;
    }
    auto moeContext = std::static_pointer_cast<OHOS::AbilityRuntime::ModularObjectExtensionContext>(contextPtr);
    auto err = moeContext->StartSelfUIAbility(abilityWant);
    return ConvertToAPI17BusinessErrorCode(err);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_ModObjExtensionContext_StartSelfUIAbilityWithStartOptions(
    OH_AbilityRuntime_ModObjExtensionContextHandle context, const AbilityBase_Want *want,
    const AbilityRuntime_StartOptions *options)
{
    if (options == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null options");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    std::shared_ptr<OHOS::AbilityRuntime::Context> contextPtr;
    auto ret = CheckMoeContext(context, contextPtr);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        return ret;
    }
    Want abilityWant;
    ret = TransformWant(want, abilityWant);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        return ret;
    }
    auto startOptions = const_cast<AbilityRuntime_StartOptions *>(options)->GetInnerStartOptions();
    auto moeContext = std::static_pointer_cast<OHOS::AbilityRuntime::ModularObjectExtensionContext>(contextPtr);
    auto err = moeContext->StartSelfUIAbilityWithStartOptions(abilityWant, startOptions);
    return ConvertToAPI17BusinessErrorCode(err);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_ModObjExtensionContext_TerminateSelf(
    OH_AbilityRuntime_ModObjExtensionContextHandle context)
{
    std::shared_ptr<OHOS::AbilityRuntime::Context> contextPtr;
    auto ret = CheckMoeContext(context, contextPtr);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        return ret;
    }
    auto moeContext = std::static_pointer_cast<OHOS::AbilityRuntime::ModularObjectExtensionContext>(contextPtr);
    auto err = moeContext->TerminateSelf();
    return ConvertToCommonBusinessErrorCode(err);
}

#ifdef __cplusplus
} // extern "C"
#endif
