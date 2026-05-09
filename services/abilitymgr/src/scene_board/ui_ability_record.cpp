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

#include "ui_ability_record.h"

#include "ability_util.h"
#include "native_ability_util.h"

namespace OHOS {
namespace AAFwk {
UIAbilityRecord::UIAbilityRecord(const Want &want, const AppExecFwk::AbilityInfo &abilityInfo,
    const AppExecFwk::ApplicationInfo &applicationInfo, int requestCode)
    : AbilityRecord(want, abilityInfo, applicationInfo, requestCode) {}

std::shared_ptr<UIAbilityRecord> UIAbilityRecord::CreateAbilityRecord(const AbilityRequest &abilityRequest)
{
    auto abilityRecord = std::make_shared<UIAbilityRecord>(
        abilityRequest.want, abilityRequest.abilityInfo, abilityRequest.appInfo, abilityRequest.requestCode);
    abilityRecord->Init(abilityRequest);
    NativeAbilityMetaData metaData;
    NativeAbilityMetaData::InitData(abilityRecord->abilityInfo_, metaData);
    if (metaData.withNativeModule) {
        if (metaData.startupPhase == AAFwk::StartupPhase::PRE_WINDOW ||
            metaData.startupPhase == AAFwk::StartupPhase::PRE_FOREGROUND) {
            abilityRecord->abilityNativeState_ = AbilityNativeState::INIT;
        } else {
            abilityRecord->abilityNativeState_ = AbilityNativeState::NORMAL;
        }
    }
    return abilityRecord;
}

AbilityRecordType UIAbilityRecord::GetAbilityRecordType()
{
    return AbilityRecordType::UI_ABILITY;
}

void UIAbilityRecord::ScheduleCollaborate(const Want &want)
{
    std::lock_guard guard(collaborateWantLock_);
    CHECK_POINTER(lifecycleDeal_);
    lifecycleDeal_->ScheduleCollaborate(want);
}

void UIAbilityRecord::AttachNative()
{
    if (GetNativeState() == AbilityNativeState::INIT) {
        SetNativeState(AbilityNativeState::ATTACHED);
    }
}
}  // namespace AAFwk
}  // namespace OHOS