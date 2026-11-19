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

#ifndef MOCK_SYSTEM_ABILITY_MANAGER_H
#define MOCK_SYSTEM_ABILITY_MANAGER_H

#include "if_system_ability_manager.h"
#include "iremote_stub.h"

namespace OHOS {
namespace AAFwk {
class MockSystemAbilityManager : public IRemoteStub<ISystemAbilityManager> {
public:
    MockSystemAbilityManager() = default;
    ~MockSystemAbilityManager() = default;

    int32_t OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override
    {
        return 0;
    }

    int LoadSystemAbility(int32_t systemAbilityId, const sptr<ISystemAbilityLoadCallback> &callback) override;

    sptr<IRemoteObject> GetSystemAbility(int32_t systemAbilityId) override;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif // MOCK_SYSTEM_ABILITY_MANAGER_H