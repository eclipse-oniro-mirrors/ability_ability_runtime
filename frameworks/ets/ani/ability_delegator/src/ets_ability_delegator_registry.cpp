/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "ets_ability_delegator_registry.h"

#include <memory>
#include "ability_delegator.h"
#include "ability_delegator_registry.h"
#include "ets_ability_delegator.h"
#include "ets_ability_delegator_utils.h"
#include "ets_native_reference.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AbilityDelegatorEts {
std::unique_ptr<AppExecFwk::ETSNativeReference> etsReference;
std::mutex etsReferenceMutex;

namespace {
constexpr const char* ETS_DELEGATOR_REGISTRY_NAMESPACE =
    "L@ohos/app/ability/abilityDelegatorRegistry/abilityDelegatorRegistry;";
constexpr const char* ETS_DELEGATOR_REGISTRY_SIGNATURE_DELEAGTOR = ":Lapplication/AbilityDelegator/AbilityDelegator;";
constexpr const char* ETS_DELEGATOR_REGISTRY_SIGNATURE_ATGS =
    ":Lapplication/abilityDelegatorArgs/AbilityDelegatorArgs;";;
}
static ani_object GetAbilityDelegator(ani_env *env)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "null env");
        return {};
    }

    std::lock_guard<std::mutex> lock(etsReferenceMutex);
    auto delegator = AppExecFwk::AbilityDelegatorRegistry::GetAbilityDelegator(AbilityRuntime::Runtime::Language::ETS);
    if (delegator == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "null delegator");
        return {};
    }

    if (etsReference == nullptr) {
        ani_object value = CreateEtsAbilityDelegator(env);
        if (value == nullptr) {
            TAG_LOGE(AAFwkTag::DELEGATOR, "value is nullptr");
            return {};
        }
        ani_boolean isValue = false;
        env->Reference_IsNullishValue(value, &isValue);
        if (isValue) {
            TAG_LOGE(AAFwkTag::DELEGATOR, "Reference_IsNullishValue");
            return {};
        }
        etsReference = std::make_unique<AppExecFwk::ETSNativeReference>();
        ani_ref result = nullptr;
        auto status = env->GlobalReference_Create(value, &(result));
        if (status != ANI_OK) {
            TAG_LOGE(AAFwkTag::DELEGATOR, "Create Gloabl ref for delegator failed %{public}d", status);
            return {};
        }
        etsReference->aniObj = static_cast<ani_object>(result);
        return etsReference->aniObj;
    } else {
        return etsReference->aniObj;
    }
}

static ani_object GetArguments(ani_env *env)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "null env");
        return {};
    }

    auto abilityDelegatorArgs = AppExecFwk::AbilityDelegatorRegistry::GetArguments();
    if (abilityDelegatorArgs == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "get argument failed");
        return {};
    }

    return CreateEtsAbilityDelegatorArguments(env, abilityDelegatorArgs);
}

void EtsAbilityDelegatorRegistryInit(ani_env *env)
{
    TAG_LOGD(AAFwkTag::DELEGATOR, "EtsAbilityDelegatorRegistryInit call");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "null env");
        return;
    }
    ani_status status = ANI_ERROR;
    if (env->ResetError() != ANI_OK) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "ResetError failed");
    }

    ani_namespace ns = nullptr;
    status = env->FindNamespace(ETS_DELEGATOR_REGISTRY_NAMESPACE, &ns);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "FindNamespace abilityDelegatorRegistry failed status: %{public}d", status);
        return;
    }

    std::array kitFunctions = {
        ani_native_function {"getAbilityDelegator", ETS_DELEGATOR_REGISTRY_SIGNATURE_DELEAGTOR,
            reinterpret_cast<void *>(GetAbilityDelegator)},
        ani_native_function {"getArguments", ETS_DELEGATOR_REGISTRY_SIGNATURE_ATGS,
            reinterpret_cast<void *>(GetArguments)},
    };

    status = env->Namespace_BindNativeFunctions(ns, kitFunctions.data(), kitFunctions.size());
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "Namespace_BindNativeFunctions failed status: %{public}d", status);
    }

    if (env->ResetError() != ANI_OK) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "ResetError failed");
    }
    TAG_LOGD(AAFwkTag::DELEGATOR, "EtsAbilityDelegatorRegistryInit end");
}

extern "C" {
ANI_EXPORT ani_status ANI_Constructor(ani_vm *vm, uint32_t *result)
{
    TAG_LOGD(AAFwkTag::DELEGATOR, "ANI_Constructor");
    ani_env *env = nullptr;
    ani_status status = ANI_ERROR;
    if (vm == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "null vm");
        return ANI_ERROR;
    }
    status = vm->GetEnv(ANI_VERSION_1, &env);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "GetEnv failed status: %{public}d", status);
        return ANI_NOT_FOUND;
    }

    EtsAbilityDelegatorRegistryInit(env);
    *result = ANI_VERSION_1;
    TAG_LOGD(AAFwkTag::DELEGATOR, "ANI_Constructor finish");
    return ANI_OK;
}
}
} // namespace AbilityDelegatorEts
} // namespace OHOS
