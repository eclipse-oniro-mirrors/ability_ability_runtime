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

#ifndef OHOS_ABILITY_RUNTIME_UI_ABILITY_RECORD_H
#define OHOS_ABILITY_RUNTIME_UI_ABILITY_RECORD_H

#include "ability_record.h"

namespace OHOS {
namespace AAFwk {
class UIAbilityRecord;
using UIAbilityRecordPtr = std::shared_ptr<UIAbilityRecord>;

class UIAbilityRecord : public AbilityRecord {
public:
    UIAbilityRecord(const Want &want, const AppExecFwk::AbilityInfo &abilityInfo,
        const AppExecFwk::ApplicationInfo &applicationInfo, int requestCode);
    
    static std::shared_ptr<UIAbilityRecord> CreateAbilityRecord(const AbilityRequest &abilityRequest);

    AbilityRecordType GetAbilityRecordType() override;

    inline void SetExitReasonLoaded()
    {
        exitReasonLoaded_ = true;
    }
    inline bool IsExitReasonLoaded() const
    {
        return exitReasonLoaded_;
    }

    inline void SetIsKillPrecedeStart(bool isKillPrecedeStart)
    {
        isKillPrecedeStart_.store(isKillPrecedeStart);
    }
    inline bool IsKillPrecedeStart()
    {
        return isKillPrecedeStart_.load();
    }

    inline bool GetPrelaunchFlag() const
    {
        return isPrelaunch_;
    }
    inline void SetPrelaunchFlag(bool isPrelaunch)
    {
        isPrelaunch_ = isPrelaunch;
    }

    void ScheduleCollaborate(const Want &want);

    inline bool IsHook () const
    {
        return isHook_;
    }
    inline void SetIsHook(bool isHook)
    {
        isHook_ = isHook;
    }

    inline bool GetHookOff () const
    {
        return hookOff_;
    }
    inline void SetHookOff(bool hookOff)
    {
        hookOff_ = hookOff;
    }

    inline void SetShouldUpdateWant(bool shouldUpdateWant)
    {
        shouldUpdateWant_.store(shouldUpdateWant);
    }

    inline bool ShouldUpdateWant() const
    {
        return shouldUpdateWant_.load();
    }

    inline void SetLastWant(std::shared_ptr<Want> lastWant)
    {
        std::lock_guard lock(wantLock_);
        lastWant_ = lastWant;
    }

    inline bool HasLastWant() const
    {
        std::lock_guard lock(wantLock_);
        return lastWant_ != nullptr;
    }

    bool UpdateWantByLastWant();

    inline void SetOnNewWantSkipScenarios(int32_t scenarios)
    {
        scenarios_.store(scenarios);
    }

    inline int32_t GetOnNewWantSkipScenarios() const
    {
        return scenarios_.load();
    }

    inline void SetNativeState(AbilityNativeState newState)
    {
        abilityNativeState_ = newState;
    }

    inline AbilityNativeState GetNativeState() const
    {
        return abilityNativeState_;
    }

    void AttachNative();

    inline bool CheckStartPendingState(int32_t requestId) const
    {
        auto pendingState = GetPendingState();
        return pendingState == AbilityState::INITIAL || (pendingState == AbilityState::FOREGROUND &&
            GetNativeState() == AbilityNativeState::ON_FOREGROUND && requestId == startSelfRequestId_);
    }

    inline void SetStartSelfRequestId(int32_t startSelfRequestId)
    {
        startSelfRequestId_ = startSelfRequestId;
    }

    inline void SetLaunchWant(std::shared_ptr<Want> launchWant)
    {
        launchWant_ = launchWant;
    }

    inline std::shared_ptr<Want> GetLaunchWant() const
    {
        return launchWant_;
    }

    inline void SetByOeExt(bool isByOeExt)
    {
        isByOeExt_ = isByOeExt;
    }

    inline bool IsByOeExt() const
    {
        return isByOeExt_;
    }

private:
    bool exitReasonLoaded_ = false;
    bool hookOff_ = false;
    bool isByOeExt_ = false;
    int32_t startSelfRequestId_ = 0;
    std::atomic_bool shouldUpdateWant_ = false;
    std::atomic_bool isKillPrecedeStart_ = false;
    std::atomic<AbilityNativeState> abilityNativeState_ = AbilityNativeState::NONE;
    std::atomic_int32_t scenarios_ = 0;
    std::shared_ptr<Want> launchWant_;
    std::shared_ptr<Want> lastWant_;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_UI_ABILITY_RECORD_H