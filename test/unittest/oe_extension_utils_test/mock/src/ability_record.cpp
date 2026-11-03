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

#include "ability_record.h"

#include "start_ability_utils.h"

namespace OHOS {
namespace AAFwk {
std::shared_ptr<AbilityRecord> AbilityRecord::CreateAbilityRecord(
    const AppExecFwk::AbilityInfo &abilityInfo, int32_t userId)
{
    auto abilityRecord = std::make_shared<AbilityRecord>(abilityInfo, userId);
    abilityRecord->token_ = sptr<Token>::MakeSptr(abilityRecord);
    return abilityRecord;
}


bool StartAbilityUtils::mockStatus = true;
int32_t StartAbilityUtils::mockAppIndex = 0;

bool StartAbilityUtils::GetAppIndex(const Want &want, sptr<IRemoteObject> callerToken,
    int userId, int32_t &appIndex)
{
    appIndex = mockAppIndex;
    return mockStatus;
}
}
}