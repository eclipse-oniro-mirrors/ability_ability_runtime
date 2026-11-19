/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "ability_delegator_registry.h"

namespace OHOS {
namespace AppExecFwk {
std::map<AbilityRuntime::Runtime::Language, std::shared_ptr<IAbilityDelegator>>
    AbilityDelegatorRegistry::abilityDelegator_ {};
std::shared_ptr<AbilityDelegatorArgs> AbilityDelegatorRegistry::abilityDelegatorArgs_ {};
void *AbilityDelegatorRegistry::napiEnv_ = nullptr;
void *AbilityDelegatorRegistry::aniEnv_ = nullptr;

std::shared_ptr<AbilityDelegator> AbilityDelegatorRegistry::GetAbilityDelegator(
    const AbilityRuntime::Runtime::Language &language)
{
    auto it = abilityDelegator_.find(language);
    if (it != abilityDelegator_.end()) {
        auto p = reinterpret_cast<AbilityDelegator *>(it->second.get());
        return std::shared_ptr<AbilityDelegator>(it->second, p);
    }
    return nullptr;
}

#ifdef CJ_FRONTEND
std::shared_ptr<CJAbilityDelegatorImpl> AbilityDelegatorRegistry::GetCJAbilityDelegator()
{
    auto it = abilityDelegator_.find(AbilityRuntime::Runtime::Language::CJ);
    if (it != abilityDelegator_.end()) {
        auto p = reinterpret_cast<CJAbilityDelegatorImpl *>(it->second.get());
        return std::shared_ptr<CJAbilityDelegatorImpl>(it->second, p);
    }
    return nullptr;
}
#endif

std::shared_ptr<AbilityDelegatorArgs> AbilityDelegatorRegistry::GetArguments()
{
    return abilityDelegatorArgs_;
}

void AbilityDelegatorRegistry::RegisterInstance(const std::shared_ptr<IAbilityDelegator> &delegator,
    const std::shared_ptr<AbilityDelegatorArgs> &args, const AbilityRuntime::Runtime::Language &language)
{
    abilityDelegatorArgs_ = args;
    abilityDelegator_.insert_or_assign(language, delegator);
}

void AbilityDelegatorRegistry::SetRuntimeEnvs(void *napiEnv, void *aniEnv)
{
    napiEnv_ = napiEnv;
    aniEnv_ = aniEnv;
}

void *AbilityDelegatorRegistry::GetNapiEnv()
{
    return napiEnv_;
}

void *AbilityDelegatorRegistry::GetAniEnv()
{
    return aniEnv_;
}
} // namespace AppExecFwk
} // namespace OHOS
