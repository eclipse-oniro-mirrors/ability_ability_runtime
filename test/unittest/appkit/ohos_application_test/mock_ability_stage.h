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

#ifndef OHOS_ABILITY_RUNTIME_MOCK_ABILITY_STAGE_H
#define OHOS_ABILITY_RUNTIME_MOCK_ABILITY_STAGE_H

#include "ability_stage.h"

#include "gmock/gmock.h"

namespace OHOS {
namespace AbilityRuntime {
class MockAbilityStage : public AbilityStage {
public:
    MockAbilityStage() {}
    ~MockAbilityStage() {}

    MOCK_CONST_METHOD0(OnDestroy, void());
    MOCK_METHOD1(OnMemoryLevel, void(int));
    MOCK_METHOD5(RunAutoStartupTask, int32_t(const std::function<void()>&, std::shared_ptr<AAFwk::Want>, bool&,
        const std::shared_ptr<Context>&, bool));
    MOCK_METHOD0(OnAboutToCreateAbility, void());
    MOCK_METHOD0(OnLaunchFromHyperSnap, void());
};
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_MOCK_ABILITY_STAGE_H
