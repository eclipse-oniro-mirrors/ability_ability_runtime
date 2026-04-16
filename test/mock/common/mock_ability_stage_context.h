/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_MOCK_ABILITY_STAGE_CONTEXT_H
#define OHOS_ABILITY_RUNTIME_MOCK_ABILITY_STAGE_CONTEXT_H

#include "ability_stage_context.h"

namespace OHOS {
namespace AbilityRuntime {
class MockAbilityStageContext : public AbilityStageContext {
public:
    MockAbilityStageContext() = default;
    virtual ~MockAbilityStageContext() = default;

    MOCK_METHOD0(OnCreate, void());
    MOCK_METHOD0(OnAcceptWant, void());
    MOCK_METHOD0(OnConfigurationUpdated, void());
    MOCK_METHOD1(OnMemoryLevel, void(int));
    MOCK_METHOD0(OnNewWant, void());
    MOCK_METHOD0(OnBackground, void());
    MOCK_METHOD0(OnForeground, void());
};
} // namespace AbilityRuntime
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_MOCK_ABILITY_STAGE_CONTEXT_H
