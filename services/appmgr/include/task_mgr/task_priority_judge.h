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

#ifndef OHOS_APP_EXECFWK_TASK_MGR_TASK_PRIORITY_JUDGE_H
#define OHOS_APP_EXECFWK_TASK_MGR_TASK_PRIORITY_JUDGE_H

#include <cstdint>
#include <functional>
#include <map>
#include <mutex>
#include <set>
#include <string>
#include <sys/types.h>
#include <vector>

#include "task_mgr/system_load_provider.h"

namespace OHOS {
namespace AppExecFwk {

/**
 * @brief Judges the task priority level from the caller foreground state and the system
 *        load fusion level. The judge only caches its two inputs (the per-uid foreground
 *        ability counts and the fusion level); the wiring of the inputs (application state
 *        subscription, load listener registration) is done by TaskManagerService when the
 *        task manager assembles. All members are protected by judgeMutex_, a leaf lock that
 *        is only ever taken while (or without) the task record mutex and never calls out,
 *        so the lock order tasksMutex_ -> judgeMutex_ is respected everywhere.
 */
class TaskPriorityJudge {
public:
    static TaskPriorityJudge &GetInstance();

    /**
     * @brief Compile-time default mapping matrix:
     *        foreground (the caller process hosts a foreground ability) -> HIGH;
     *        background & fusion level 5~7 (high load) -> LOW;
     *        background & fusion level 0~4 or the load unavailable -> MEDIUM.
     * @param foreground Indicates whether the caller process hosts a foreground ability.
     * @param loadLevel Indicates the fusion level (0~7); a negative value means unavailable.
     * @return Returns one of TASK_PRIORITY_HIGH / TASK_PRIORITY_MEDIUM / TASK_PRIORITY_LOW.
     */
    static int32_t DetermineLevel(bool foreground, int32_t loadLevel);

    /**
     * @brief Starts recording the foreground abilities of a process and reconciles the
     *        initial state in the same call: the ability observer only delivers
     *        incremental events, so a pid that was already foreground before its
     *        tracking started would otherwise stay invisible until its next transition.
     *        When the tracked pid has no foreground entry yet and the foreground seeder
     *        is wired, the current foreground ability tokens are queried through the
     *        seeder and seeded. Only tracked processes are recorded: ability state
     *        events of untracked pids are ignored, so the recorded state stays bounded
     *        by the live task creators.
     * @param pid Indicates the caller process id.
     */
    void TrackForeground(pid_t pid);

    /**
     * @brief Stops recording a process and drops its foreground entries.
     * @param pid Indicates the caller process id.
     */
    void UntrackForeground(pid_t pid);

    /**
     * @brief Updates the foreground ability set of a tracked process; idempotent so that
     *        AMS double-dispatch paths (ChangeAbilityToForegroundIfNeeded +
     *        PopForegroundingAbilityTokens, app_running_record.cpp:1016,1203) collapse
     *        onto one entry. Uses the ability's IRObject token address as the identity
     *        key; a foreground event is inserted, background/terminated/ended is erased.
     * @param pid Indicates the caller process id.
     * @param token Indicates the ability's IRObject token address.
     * @param foreground Indicates true when an ability entered the foreground; false when
     *        it left the foreground (including termination).
     */
    void OnForegroundAbilityChanged(pid_t pid, uintptr_t token, bool foreground);

    /**
     * @brief Updates the cached fusion level; called from the load listener callback and
     *        once right after a successful listener registration.
     * @param loadLevel Indicates the new fusion level (0~7); values outside the range are
     *        treated as unavailable (fallback to the default level).
     */
    void OnSystemLoadLevelChanged(int32_t loadLevel);

    /**
     * @brief Judges the level for a caller process from the cached inputs.
     * @param pid Indicates the caller process id.
     * @return Returns the judged task level (HIGH/MEDIUM/LOW); falls back to the default
     *         level when the inputs are unavailable.
     */
    int32_t JudgeLevel(pid_t pid) const;

    /**
     * @brief Queries whether the caller process currently hosts any foreground ability.
     * @param pid Indicates the caller process id.
     * @return Returns true when at least one foreground ability identity exists.
     */
    bool IsForeground(pid_t pid) const;

    /**
     * @brief Clears all foreground ability entries for a dead process (called from
     *        OnProcessDied). After this call the pid is absent from the map so the
     *        count cannot leak.
     * @param pid Indicates the dead process id.
     */
    void OnProcessDied(pid_t pid);

    /**
     * @brief Seeds the foreground entries of a tracked pid with the given ability tokens,
     *        the same identity the ability state observer reports; idempotent per token.
     *        Used to reconcile a pid that was already foreground before its tracking
     *        started (the observer is incremental-only).
     * @param pid Indicates the process id to seed.
     * @param tokens Indicates the token addresses of the pid's foreground abilities.
     */
    void SeedForeground(pid_t pid, const std::vector<uintptr_t> &tokens);

    /**
     * @brief Queries the cached fusion level.
     * @return Returns the fusion level (0~7); -1 when unavailable.
     */
    int32_t CurrentLoadLevel() const;

    /**
     * @brief Injects the load provider; used by unit tests to stub the load source. When
     *        never called, the default ressched-backed provider is created lazily on the
     *        first RegisterSystemLoadListener.
     * @param provider Indicates the provider, or nullptr to restore the default.
     */
    void SetLoadProvider(const std::shared_ptr<ISystemLoadProvider> &provider);

    /**
     * @brief Registers the fusion level listener through the load provider; on failure the
     *        load input stays unavailable and every background task is judged with the
     *        fallback level.
     * @return Returns true when the listener is active.
     */
    bool RegisterSystemLoadListener();

    /**
     * @brief Sets the hook invoked (on the provider callback thread) after every fusion
     *        level cache update; TaskManagerService uses it to re-judge the running tasks.
     * @param hook Indicates the hook, or nullptr to clear.
     */
    void SetLevelChangedHook(std::function<void(int32_t)> hook);

    /**
     * @brief Wires the per-pid foreground ability token query used by TrackForeground to
     *        reconcile the initial state of a newly tracked pid; the tokens must be the
     *        same identity the ability state observer reports, so a seeded entry and an
     *        observed entry are indistinguishable. Wired by the app manager service,
     *        which owns the running-application records.
     * @param seeder Indicates the query collecting the foreground ability tokens of one
     *        process, or null to unwire (the reconcile is then skipped).
     */
    void SetForegroundSeeder(std::function<void(pid_t, std::vector<uintptr_t>&)> seeder);

    /**
     * @brief Unregisters the fusion level listener.
     */
    void UnregisterSystemLoadListener();

    /**
     * @brief Renders the judge state section for the --psched dump (cached fusion level,
     *        per-uid foreground ability counts and the listener registration state).
     * @return Returns the dump text.
     */
    std::string DumpJudgeState() const;

private:
    TaskPriorityJudge() = default;
    ~TaskPriorityJudge() = default;
    TaskPriorityJudge(const TaskPriorityJudge &) = delete;
    TaskPriorityJudge &operator=(const TaskPriorityJudge &) = delete;

    static bool IsHighLoad(int32_t loadLevel);

    // Invoked right after a fusion level cache update to drive the task re-judgment.
    void NotifyLevelChanged(int32_t level);

    std::shared_ptr<ISystemLoadProvider> loadProvider_;
    std::function<void(int32_t)> levelChangedHook_;
    // Per-pid foreground ability token query wired by the app manager service; invoked
    // WITHOUT judgeMutex_ held (it enters the appmgr running-record locks) and only for
    // a tracked pid whose foreground set is still empty.
    std::function<void(pid_t, std::vector<uintptr_t>&)> foregroundSeeder_;
    mutable std::mutex judgeMutex_;
    // Foreground ability set per process: the set stores the ability's IRObject token
    // address (uintptr_t). A sentinel value 0 represents the seeder "some ability is
    // foreground but we don't know its identity"; the first real ability event for that
    // pid clears the sentinel. The set's non-emptiness drives IsForeground / JudgeLevel.
    // Using a set collapses the AMS double-dispatch paths (FG sent twice for the same
    // ability) onto a single entry and makes the model order-independent.
    std::set<pid_t> trackedPids_;
    std::map<pid_t, std::set<uintptr_t>> fgAbilities_;
    int32_t loadLevel_ = -1;
    bool loadListenerRegistered_ = false;
};

}  // namespace AppExecFwk
}  // namespace OHOS

#endif  // OHOS_APP_EXECFWK_TASK_MGR_TASK_PRIORITY_JUDGE_H
