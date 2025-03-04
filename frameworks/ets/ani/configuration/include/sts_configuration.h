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

#ifndef OHOS_ABILITY_RUNTIME_STS_ABILITY_DELEGATOR_UTILS_H
#define OHOS_ABILITY_RUNTIME_STS_ABILITY_DELEGATOR_UTILS_H

#include "configuration.h"
#include "sts_runtime.h"

namespace OHOS {
namespace ConfigurationSts {
ani_object CreateStsConfiguration(ani_env *env,
    const std::shared_ptr<AppExecFwk::Configuration> configuration);
void StsConfigurationInit(ani_env *env);
} // namespace ConfigurationSts
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_STS_ABILITY_DELEGATOR_UTILS_H
