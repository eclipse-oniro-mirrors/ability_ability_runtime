/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#include "ability_local_record.h"

#include "ability_thread.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
constexpr const char* IS_HOOK = "ohos.ability_runtime.is_hook";
}
AbilityLocalRecord::AbilityLocalRecord(const std::shared_ptr<AbilityInfo> &info, const sptr<IRemoteObject> &token,
    const std::shared_ptr<AAFwk::Want> &want, int32_t abilityRecordId)
    : abilityInfo_(info), token_(token), want_(want), abilityRecordId_(abilityRecordId) {}

AbilityLocalRecord::~AbilityLocalRecord() {}

const std::shared_ptr<AbilityInfo> &AbilityLocalRecord::GetAbilityInfo()
{
    return abilityInfo_;
}

const sptr<IRemoteObject> &AbilityLocalRecord::GetToken()
{
    return token_;
}

int32_t AbilityLocalRecord::GetAbilityRecordId() const
{
    return abilityRecordId_;
}

const sptr<AbilityThread> &AbilityLocalRecord::GetAbilityThread()
{
    return abilityThread_;
}

void AbilityLocalRecord::SetAbilityThread(const sptr<AbilityThread> &abilityThread)
{
    abilityThread_ = abilityThread;
}

const std::shared_ptr<AAFwk::Want> &AbilityLocalRecord::GetWant()
{
    return want_;
}

bool AbilityLocalRecord::IsHook() const
{
    return want_ && want_->GetBoolParam(IS_HOOK, false);
}

void AbilityLocalRecord::SetSkipAbilityStageLifecycle(bool skipAbilityStageLifecycle)
{
    skipAbilityStageLifecycle_ = skipAbilityStageLifecycle;
}

bool AbilityLocalRecord::IsSkipAbilityStageLifecycle() const
{
    return skipAbilityStageLifecycle_;
}
}  // namespace AppExecFwk
}  // namespace OHOS
