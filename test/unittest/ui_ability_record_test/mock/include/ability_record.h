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

#ifndef OHOS_ABILITY_RUNTIME_ABILITY_RECORD_H
#define OHOS_ABILITY_RUNTIME_ABILITY_RECORD_H

#include <atomic>
#include <memory>
#include <mutex>
#include <gmock/gmock.h>

#include "ability_info.h"
#include "ability_state.h"

namespace OHOS {
namespace AAFwk {
enum class AbilityRecordType {
    BASE_ABILITY,
    UI_ABILITY,
    MISSION_ABILITY,
};

class Want {};

struct AbilityRequest {
    int32_t requestCode = 0;
    Want want;
    AppExecFwk::AbilityInfo abilityInfo;
    AppExecFwk::ApplicationInfo appInfo;
};

class LifecycleDeal {
public:
    MOCK_METHOD(void, ScheduleCollaborate, (Want), ());
};

class AbilityRecord : public std::enable_shared_from_this<AbilityRecord> {
public:
    AbilityRecord(const Want &want, const AppExecFwk::AbilityInfo &abilityInfo,
        const AppExecFwk::ApplicationInfo &applicationInfo, int32_t requestCode)
        : abilityInfo_(abilityInfo) {}
    virtual ~AbilityRecord() = default;

    virtual void Init(const AbilityRequest &abilityRequest);
    virtual AbilityRecordType GetAbilityRecordType();

    inline void SetPendingState(AbilityState state)
    {
        pendingState_ = state;
    }
    inline AbilityState GetPendingState() const
    {
        return pendingState_;
    }
protected:
    bool isPrelaunch_ = false;
    bool isHook_ = false;
    AbilityState pendingState_ = AbilityState::INITIAL;
    std::atomic_bool isLastWantBackgroundDriven_ = false;
    std::mutex collaborateWantLock_;
    std::shared_ptr<LifecycleDeal> lifecycleDeal_;

    AppExecFwk::AbilityInfo abilityInfo_;
};
} // namespace AAFwk
} // namespace OHOS
#endif
