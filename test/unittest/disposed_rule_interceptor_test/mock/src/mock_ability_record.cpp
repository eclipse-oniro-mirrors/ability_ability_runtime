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

#include "ability_record.h"

#include "mock_my_flag.h"

namespace OHOS {
namespace AAFwk {
Token::Token()
{}

Token::~Token()
{}

std::shared_ptr<AbilityRecord> Token::GetAbilityRecordByToken(const sptr<IRemoteObject> &token)
{
    return MyFlag::abilityRecord_;
}

AbilityRecord::AbilityRecord()
{}

AbilityRecord::~AbilityRecord()
{}

const AppExecFwk::AbilityInfo &AbilityRecord::GetAbilityInfo() const
{
    return abilityInfo;
}

const Want& AbilityRecord::GetWant() const
{
    return want_;
}

sptr<SessionInfo> AbilityRecord::GetSessionInfo() const
{
    return sessionInfo_;
}

int32_t AbilityRecord::GetMissionId() const
{
    return missionId_;
}

int32_t AbilityRecord::CreateModalUIExtension(const Want &want)
{
    return MyFlag::retAbilityRecordCreateModalUIExtension_;
}
}  // namespace AAFwk
}  // namespace OHOS
