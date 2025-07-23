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

#ifndef OHOS_ABILITY_RUNTIME_ETS_ABILITY_STAGE_MONITOR_H
#define OHOS_ABILITY_RUNTIME_ETS_ABILITY_STAGE_MONITOR_H

#include "iability_stage_monitor.h"

namespace OHOS {
namespace AbilityDelegatorEts {
using namespace OHOS::AppExecFwk;
class EtsAbilityStageMonitor : public IAbilityStageMonitor {
public:
    /**
     * A constructor used to create a AbilityStageMonitor instance with the input parameter passed.
     *
     * @param moduleName Indicates the specified module name.
     * @param srcEntrance Indicates the abilityStage source path.
     */
    EtsAbilityStageMonitor(const std::string &moduleName, const std::string &srcEntrance)
        : IAbilityStageMonitor(moduleName, srcEntrance) {}

    /**
     * Default deconstructor used to deconstruct.
     */
    ~EtsAbilityStageMonitor() = default;
};
} // namespace AbilityDelegatorEts
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_ETS_ABILITY_STAGE_MONITOR_H
