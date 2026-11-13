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

#include "task_mgr/task_priority_judge.h"

#include "hilog_tag_wrapper.h"
#include "task_mgr/common.h"

namespace OHOS {
namespace AppExecFwk {

namespace {
constexpr int32_t FUSION_LEVEL_HIGH_LOAD_MIN = 5;
constexpr int32_t FUSION_LEVEL_HIGH_LOAD_MAX = 7;
}

TaskPriorityJudge &TaskPriorityJudge::GetInstance()
{
    static TaskPriorityJudge instance;
    return instance;
}

bool TaskPriorityJudge::IsHighLoad(int32_t loadLevel)
{
    return loadLevel >= FUSION_LEVEL_HIGH_LOAD_MIN && loadLevel <= FUSION_LEVEL_HIGH_LOAD_MAX;
}

int32_t TaskPriorityJudge::DetermineLevel(bool foreground, int32_t loadLevel)
{
    if (foreground) {
        return TASK_PRIORITY_HIGH;
    }
    if (IsHighLoad(loadLevel)) {
        return TASK_PRIORITY_LOW;
    }
    return TASK_PRIORITY_MEDIUM;
}

void TaskPriorityJudge::TrackForeground(pid_t pid)
{
    if (pid <= 0) {
        return;
    }
    std::function<void(pid_t, std::vector<uintptr_t>&)> seeder;
    {
        std::lock_guard<std::mutex> lock(judgeMutex_);
        trackedPids_.insert(pid);
        auto it = fgAbilities_.find(pid);
        if (it != fgAbilities_.end() && !it->second.empty()) {
            return;
        }
        seeder = foregroundSeeder_;
    }
    // The seeder enters the appmgr running-record locks, so it must run WITHOUT
    // judgeMutex_ (lock order: no judge lock while holding appmgr locks). SeedForeground
    // re-takes the judge lock and is idempotent per token, so a concurrent ability
    // event landing in between collapses onto the same entry.
    if (seeder == nullptr) {
        return;
    }
    std::vector<uintptr_t> tokens;
    seeder(pid, tokens);
    if (!tokens.empty()) {
        SeedForeground(pid, tokens);
        TAG_LOGI(AAFwkTag::APPMGR, "fg abilities reconciled, pid: %{public}d count: %{public}zu",
            pid, tokens.size());
    }
}

void TaskPriorityJudge::UntrackForeground(pid_t pid)
{
    std::lock_guard<std::mutex> lock(judgeMutex_);
    trackedPids_.erase(pid);
    fgAbilities_.erase(pid);
}

void TaskPriorityJudge::OnForegroundAbilityChanged(pid_t pid, uintptr_t token, bool foreground)
{
    if (pid <= 0) {
        return;
    }
    std::lock_guard<std::mutex> lock(judgeMutex_);
    if (trackedPids_.count(pid) == 0) {
        return;
    }
    auto &abilities = fgAbilities_[pid];
    if (foreground) {
        abilities.insert(token);
        TAG_LOGI(AAFwkTag::APPMGR, "fg ability added, pid: %{public}d count: %{public}zu",
            pid, abilities.size());
    } else {
        abilities.erase(token);
        if (abilities.empty()) {
            fgAbilities_.erase(pid);
        }
        TAG_LOGI(AAFwkTag::APPMGR, "fg ability removed, pid: %{public}d count: %{public}zu",
            pid, fgAbilities_.count(pid) ? fgAbilities_[pid].size() : 0);
    }
}

void TaskPriorityJudge::SeedForeground(pid_t pid, const std::vector<uintptr_t> &tokens)
{
    if (pid <= 0) {
        return;
    }
    std::lock_guard<std::mutex> lock(judgeMutex_);
    auto &abilities = fgAbilities_[pid];
    for (uintptr_t token : tokens) {
        abilities.insert(token);
    }
    TAG_LOGI(AAFwkTag::APPMGR, "fg abilities seeded, pid: %{public}d count: %{public}zu", pid, tokens.size());
}

void TaskPriorityJudge::OnProcessDied(pid_t pid)
{
    if (pid <= 0) {
        return;
    }
    std::lock_guard<std::mutex> lock(judgeMutex_);
    trackedPids_.erase(pid);
    auto it = fgAbilities_.find(pid);
    if (it != fgAbilities_.end()) {
        size_t dropped = it->second.size();
        fgAbilities_.erase(it);
        TAG_LOGI(AAFwkTag::APPMGR, "fg abilities cleared on process died, pid: %{public}d dropped: %{public}zu",
            pid, dropped);
    }
}

void TaskPriorityJudge::OnSystemLoadLevelChanged(int32_t loadLevel)
{
    {
        std::lock_guard<std::mutex> lock(judgeMutex_);
        TAG_LOGD(AAFwkTag::APPMGR, "systemload level changed: %{public}d", loadLevel);
        if (loadLevel < 0 || loadLevel > FUSION_LEVEL_HIGH_LOAD_MAX) {
            TAG_LOGW(AAFwkTag::APPMGR, "systemload level out of range: %{public}d", loadLevel);
            loadLevel_ = -1;
        } else {
            loadLevel_ = loadLevel;
        }
    }
    // Notify OUTSIDE the judge lock: the hook re-enters the judge (JudgeLevel) and must
    // not deadlock. The notify lives inside this entry point so every cache update drives
    // the re-judgment, matching the provider-callback wiring semantics one-to-one.
    NotifyLevelChanged(loadLevel);
}

bool TaskPriorityJudge::IsForeground(pid_t pid) const
{
    std::lock_guard<std::mutex> lock(judgeMutex_);
    auto it = fgAbilities_.find(pid);
    return it != fgAbilities_.end() && !it->second.empty();
}

int32_t TaskPriorityJudge::CurrentLoadLevel() const
{
    std::lock_guard<std::mutex> lock(judgeMutex_);
    return loadLevel_;
}

int32_t TaskPriorityJudge::JudgeLevel(pid_t pid) const
{
    std::lock_guard<std::mutex> lock(judgeMutex_);
    auto it = fgAbilities_.find(pid);
    bool foreground = (it != fgAbilities_.end() && !it->second.empty());
    return DetermineLevel(foreground, loadLevel_);
}

void TaskPriorityJudge::SetLoadProvider(const std::shared_ptr<ISystemLoadProvider> &provider)
{
    std::shared_ptr<ISystemLoadProvider> oldProvider;
    bool wasRegistered;
    {
        std::lock_guard<std::mutex> lock(judgeMutex_);
        oldProvider = loadProvider_;
        wasRegistered = loadListenerRegistered_;
        loadProvider_ = provider;
        loadListenerRegistered_ = false;
    }
    // switch providers cleanly: the old listener must not keep delivering levels after the
    // swap (it would fight the new provider's cache); the unregister runs outside the judge
    // lock because it may call into the provider's own lock.
    if (wasRegistered && oldProvider != nullptr) {
        oldProvider->UnregisterListener();
    }
}

bool TaskPriorityJudge::RegisterSystemLoadListener()
{
    std::shared_ptr<ISystemLoadProvider> provider;
    {
        std::lock_guard<std::mutex> lock(judgeMutex_);
        if (loadListenerRegistered_) {
            return true;
        }
        if (loadProvider_ == nullptr) {
            loadProvider_ = CreateDefaultSystemLoadProvider();
        }
        provider = loadProvider_;
    }
    bool ok = provider != nullptr &&
        provider->RegisterListener([](int32_t level) {
            // the entry point updates the cache and notifies the hook itself
            TaskPriorityJudge::GetInstance().OnSystemLoadLevelChanged(level);
        });
    std::lock_guard<std::mutex> lock(judgeMutex_);
    loadListenerRegistered_ = ok;
    if (!ok) {
        TAG_LOGE(AAFwkTag::APPMGR, "register systemload listener failed, judge falls back");
    }
    return ok;
}

void TaskPriorityJudge::SetLevelChangedHook(std::function<void(int32_t)> hook)
{
    std::lock_guard<std::mutex> lock(judgeMutex_);
    levelChangedHook_ = std::move(hook);
}

void TaskPriorityJudge::SetForegroundSeeder(std::function<void(pid_t, std::vector<uintptr_t>&)> seeder)
{
    std::lock_guard<std::mutex> lock(judgeMutex_);
    foregroundSeeder_ = std::move(seeder);
}

void TaskPriorityJudge::NotifyLevelChanged(int32_t level)
{
    std::function<void(int32_t)> hook;
    {
        std::lock_guard<std::mutex> lock(judgeMutex_);
        hook = levelChangedHook_;
    }
    if (hook != nullptr) {
        hook(level);
    }
}

void TaskPriorityJudge::UnregisterSystemLoadListener()
{
    std::shared_ptr<ISystemLoadProvider> provider;
    {
        std::lock_guard<std::mutex> lock(judgeMutex_);
        if (!loadListenerRegistered_) {
            return;
        }
        loadListenerRegistered_ = false;
        provider = loadProvider_;
    }
    if (provider != nullptr) {
        provider->UnregisterListener();
    }
}

std::string TaskPriorityJudge::DumpJudgeState() const
{
    std::lock_guard<std::mutex> lock(judgeMutex_);
    std::string result = "Judge state:\n";
    result += "      foreground ability counts [";
    bool first = true;
    for (const auto &item : fgAbilities_) {
        if (item.second.empty()) {
            continue;
        }
        if (!first) {
            result += " ";
        }
        result += "pid:" + std::to_string(item.first) + " count:" + std::to_string(item.second.size());
        first = false;
    }
    if (first) {
        result += "empty";
    }
    result += "]\n";
    result += "      cached systemload level [" + std::to_string(loadLevel_) + "]\n";
    result += std::string("      load listener [") + (loadListenerRegistered_ ? "registered" : "not registered") +
        "]\n";
    return result;
}

}  // namespace AppExecFwk
}  // namespace OHOS
