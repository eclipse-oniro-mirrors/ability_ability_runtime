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

#include "system_ability.h"

#include "mock_my_flag.h"

namespace OHOS {
bool AgentRuntime::MyFlag::retAddSystemAbilityListener = false;
sptr<IRemoteObject> AgentRuntime::MyFlag::systemAbility = nullptr;
bool AgentRuntime::MyFlag::retPublish = false;
bool AgentRuntime::MyFlag::isAddSystemAbilityListenerCalled = false;
int32_t AgentRuntime::MyFlag::backfillPreInstallCardsCallCount = 0;

bool SystemAbility::AddSystemAbilityListener(int32_t systemAbilityId)
{
    AgentRuntime::MyFlag::isAddSystemAbilityListenerCalled = true;
    return AgentRuntime::MyFlag::retAddSystemAbilityListener;
}

sptr<IRemoteObject> SystemAbility::GetSystemAbility(int32_t systemAbilityId)
{
    return AgentRuntime::MyFlag::systemAbility;
}

bool SystemAbility::Publish(sptr<IRemoteObject> systemAbility)
{
    return AgentRuntime::MyFlag::retPublish;
}
}