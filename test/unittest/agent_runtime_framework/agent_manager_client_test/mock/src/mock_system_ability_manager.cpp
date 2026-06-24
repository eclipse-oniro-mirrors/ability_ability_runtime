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

#include "mock_system_ability_manager.h"

#include "mock_my_flag.h"

namespace OHOS {
namespace AgentRuntime {
    sptr<IRemoteObject> MyFlag::agentMgr = nullptr;
}
namespace AAFwk {
int32_t MockSystemAbilityManager::LoadSystemAbility(int32_t systemAbilityId,
    const sptr<ISystemAbilityLoadCallback> &callback)
{
    return 0;
}

sptr<IRemoteObject> MockSystemAbilityManager::GetSystemAbility(int32_t systemAbilityId)
{
    if (OHOS::AgentRuntime::MyFlag::nullSystemAbility) {
        return nullptr;
    }
    return OHOS::AgentRuntime::MyFlag::agentMgr;
}
} // namespace AAFwk
}  // namespace OHOS
