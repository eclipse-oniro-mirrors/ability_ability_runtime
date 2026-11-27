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

#include <map>

#include "ability_record.h"

namespace OHOS {
namespace AAFwk {
class UIAbilityRecord;

// Caller identity stashed for the attach-time attribution report, keyed by task id.
// A task id maps to every stashed activation (a list): repeated key-carrying starts
// before the ability thread attached must each report once, so they are appended
// instead of displacing each other.
struct PendingTaskAttribution {
    int32_t callerUid = -1;
    int32_t callerPid = -1;
};
using UIAbilityRecordPtr = std::shared_ptr<UIAbilityRecord>;

class UIAbilityRecord : public AbilityRecord {
public:
    UIAbilityRecord(const Want &want, const AppExecFwk::AbilityInfo &abilityInfo,
        const AppExecFwk::ApplicationInfo &applicationInfo, int requestCode);
    ~UIAbilityRecord() override;

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

    void NotifyCallersOnPrelaunchTimeout();

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

    void UpdatePendingStateByCall(CallToState state);

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

    // All handles per (record, task id): every genuine key-carrying startAbility
    // reports again (refreshing the task-flow keep-alive and open counts), so a
    // task id maps to every handle it produced. Duplicates of SCB-replayed wants
    // never reach here (key stripped upstream). Removed by task-finished cleanup
    // or the destructor (sessionLock_).
    inline void AddTaskAttribution(const std::string &taskId, int64_t attributionId)
    {
        taskAttributions_[taskId].emplace_back(attributionId);
    }

    // Task-finished cleanup: drops the finished task's handles so a reused id
    // starts with clean bookkeeping (sessionLock_). Returns handles removed.
    // Bookkeeping queries observe GetTaskAttributions() directly (every entry
    // means the record reported for that task id at least once).
    inline size_t RemoveTaskAttributionByTaskId(const std::string &taskId)
    {
        auto it = taskAttributions_.find(taskId);
        if (it == taskAttributions_.end()) {
            return 0;
        }
        size_t removed = it->second.size();
        taskAttributions_.erase(it);
        return removed;
    }

    inline const std::map<std::string, std::vector<int64_t>> &GetTaskAttributions() const
    {
        return taskAttributions_;
    }

    // Cold-start attribution contexts keyed by task id: every key-carrying start
    // before the ability thread attached stashes its own caller identity (the
    // same task id appends — parallel tasks and repeated activations do not
    // displace each other), reported at ability-thread attach. Guard with
    // sessionLock_.
    inline void AddPendingTaskAttribution(const std::string &taskId, int32_t callerUid, int32_t callerPid)
    {
        PendingTaskAttribution pending;
        pending.callerUid = callerUid;
        pending.callerPid = callerPid;
        pendingTaskAttributions_[taskId].emplace_back(std::move(pending));
    }

    inline const std::map<std::string, std::vector<PendingTaskAttribution>> &GetPendingTaskAttributions() const
    {
        return pendingTaskAttributions_;
    }

    inline void ClearPendingTaskAttributions()
    {
        pendingTaskAttributions_.clear();
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
    std::map<std::string, std::vector<int64_t>> taskAttributions_;
    std::map<std::string, std::vector<PendingTaskAttribution>> pendingTaskAttributions_;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_UI_ABILITY_RECORD_H