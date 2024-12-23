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

#ifndef OHOS_ABILITY_RUNTIME_CJ_ABILITY_DELEGATOR_INFOS_H
#define OHOS_ABILITY_RUNTIME_CJ_ABILITY_DELEGATOR_INFOS_H

#include <cstdint>
#include <string>

#include "ability_lifecycle_executor.h"
#include "iremote_object.h"

namespace OHOS {
namespace AppExecFwk {
struct ACJDelegatorAbilityProperty {
    // token of ability
    sptr<IRemoteObject> token_;
    // name of ability
    std::string name_;
    // name of modulename
    std::string moduleName_;
    // name of bundle + ability
    std::string fullName_;
    // lifecycle state of ability
    AbilityLifecycleExecutor::LifecycleState lifecycleState_ {
        AbilityLifecycleExecutor::LifecycleState::UNINITIALIZED
    };
    // ability object in jsruntime
    int64_t cjObject_;
};

struct CJDelegatorAbilityStageProperty {
    std::string moduleName_;
    std::string srcEntrance_;
    int64_t cjStageObject_;
};
} // namespace AppExecFwk
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_CJ_ABILITY_DELEGATOR_INFOS_H