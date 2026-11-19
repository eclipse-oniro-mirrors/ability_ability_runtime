/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_ABILITY_DELEGATOR_H
#define OHOS_ABILITY_RUNTIME_ABILITY_DELEGATOR_H

#include <functional>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#ifndef SUPPORT_GRAPHICS
#include "inttypes.h"
#endif

#include "ability_delegator_infos.h"
#include "ability_lifecycle_executor.h"
#include "context.h"
#include "delegator_thread.h"
#include "iability_monitor.h"
#include "iinterop_ability_monitor.h"
#include "iability_stage_monitor.h"
#include "shell_cmd_result.h"
#include "test_runner.h"
#include "want.h"
#include "iability_delegator.h"

namespace OHOS {
namespace AppExecFwk {
class AbilityDelegator : public std::enable_shared_from_this<AbilityDelegator>, public IAbilityDelegator {
public:
    enum class AbilityState : uint8_t {
        /**
         * Indicates that the ability has not been initialized.
         */
        UNINITIALIZED = 0,
        /**
         * Indicates that the ability is in the started state.
         */
        STARTED,
        /**
         * Indicates that the ability is in the foreground state.
         */
        FOREGROUND,
        /**
         * Indicates that the ability is in the background state.
         */
        BACKGROUND,
        /**
         * Indicates that the ability is in the stopped state.
         */
        STOPPED
    };

    /**
     * Definition of cleanup function.
     */
    using ClearFunc = std::function<void(const std::shared_ptr<BaseDelegatorAbilityProperty> &)>;

public:
    static std::shared_ptr<AbilityDelegator> Create(const std::shared_ptr<AbilityRuntime::Context>& context,
        std::unique_ptr<TestRunner> runner, const sptr<IRemoteObject>& observer);

    /**
     * A constructor used to create a AbilityDelegator instance with the input parameter passed.
     *
     * @param context Indicates the ability runtime context.
     * @param runner Indicates the TestRunner object.
     * @param observer Indicates the TestObserver object.
     */
    AbilityDelegator(const std::shared_ptr<AbilityRuntime::Context> &context, std::unique_ptr<TestRunner> runner,
        const sptr<IRemoteObject> &observer);

    /**
     * Default deconstructor used to deconstruct.
     */
    ~AbilityDelegator();

    /**
     * Adds monitor for monitoring the lifecycle state changes of the ability.
     *
     * @param monitor, Indicates the monitor object.
     */
    void AddAbilityMonitor(const std::shared_ptr<IAbilityMonitor> &monitor);

    /**
     * Adds interop monitor for monitoring the lifecycle state changes of the ability.
     *
     * @param monitor, Indicates the interop monitor object.
     */
    void AddInteropAbilityMonitor(const std::shared_ptr<IInteropAbilityMonitor> &monitor);

    /**
     * Adds monitor for monitoring the lifecycle state changes of the abilityStage.
     *
     * @param monitor, Indicates the stage monitor object.
     */
    void AddAbilityStageMonitor(const std::shared_ptr<IAbilityStageMonitor> &monitor);

    /**
     * Removes ability monitor.
     *
     * @param monitor, Indicates the specified monitor object.
     */
    void RemoveAbilityMonitor(const std::shared_ptr<IAbilityMonitor> &monitor);

    /**
     * Removes interop ability monitor.
     *
     * @param monitor, Indicates the specified interop monitor object.
     */
    void RemoveInteropAbilityMonitor(const std::shared_ptr<IInteropAbilityMonitor> &monitor);

    /**
     * Removes abilityStage monitor.
     *
     * @param monitor, Indicates the specified stage monitor object.
     */
    void RemoveAbilityStageMonitor(const std::shared_ptr<IAbilityStageMonitor> &monitor);

    /**
     * Clears all monitors.
     */
    void ClearAllMonitors();

    /**
     * Obtains the number of monitors.
     *
     * @return the number of monitors.
     */
    size_t GetMonitorsNum();

    /**
     * Obtains the number of stage monitors.
     *
     * @return the number of stage monitors.
     */
    size_t GetStageMonitorsNum();

    /**
     * Waits for the specified monitor and return the obtained ability.
     *
     * @param monitor, Indicates the specified monitor object.
     * @return the obtained ability.
     */
    std::shared_ptr<BaseDelegatorAbilityProperty> WaitAbilityMonitor(const std::shared_ptr<IAbilityMonitor> &monitor);

    /**
     * Waits for the specified monitor and return the obtained abilityStage.
     *
     * @param monitor, Indicates the specified stage monitor object.
     * @return the obtained abilityStage.
     */
    std::shared_ptr<BaseDelegatorAbilityStageProperty> WaitAbilityStageMonitor(
        const std::shared_ptr<IAbilityStageMonitor> &monitor);

    /**
     * Waits for the specified monitor within the timeout time and return the obtained ability.
     *
     * @param monitor, Indicates the specified monitor object.
     * @param timeoutMs, Indicates the specified time out time, in milliseconds.
     * @return the obtained ability.
     */
    std::shared_ptr<BaseDelegatorAbilityProperty> WaitAbilityMonitor(
        const std::shared_ptr<IAbilityMonitor> &monitor, const int64_t timeoutMs);

    /**
     * Waits for the specified monitor within the timeout time and return the obtained abilityStage.
     *
     * @param monitor, Indicates the specified stage monitor object.
     * @param timeoutMs, Indicates the specified time out time, in milliseconds.
     * @return the obtained abilityStage.
     */
    std::shared_ptr<BaseDelegatorAbilityStageProperty> WaitAbilityStageMonitor(
        const std::shared_ptr<IAbilityStageMonitor> &monitor, const int64_t timeoutMs);

    /**
     * Obtains the application context.
     *
     * @return the application context.
     */
    std::shared_ptr<AbilityRuntime::Context> GetAppContext() const;

    /**
     * Obtains the lifecycle state of the specified ability.
     *
     * @param token, Indicates the specified ability.
     * @return the lifecycle state of the specified ability.
     */
    AbilityDelegator::AbilityState GetAbilityState(const sptr<IRemoteObject> &token);

    /**
     * Obtains the ability that is currently being displayed.
     *
     * @return the ability that is currently being displayed.
     */
    std::shared_ptr<BaseDelegatorAbilityProperty> GetCurrentTopAbility();

    /**
     * Obtains the name of the thread.
     *
     * @return the name of the thread.
     */
    std::string GetThreadName() const;

    /**
     * Notifies TestRunner to prepare.
     */
    void Prepare();

    /**
     * Notifies TestRunner to run.
     */
    void OnRun();

    /**
     * Starts an ability based on the given Want.
     *
     * @param want, Indicates the Want for starting the ability.
     * @return the result code.
     */
    ErrCode StartAbility(const AAFwk::Want &want);

    /**
     * Transits the specified ability to foreground.
     *
     * @param token, Indicates the specified ability.
     * @return true if succeed; returns false otherwise.
     */
    bool DoAbilityForeground(const sptr<IRemoteObject> &token);

    /**
     * Transits the specified ability to background.
     *
     * @param token, Indicates the specified ability.
     * @return true if succeed; returns false otherwise.
     */
    bool DoAbilityBackground(const sptr<IRemoteObject> &token);

    /**
     * Executes the specified shell command.
     *
     * @param cmd, Indicates the specified shell command.
     * @param timeoutSec, Indicates the specified time out time, in seconds.
     * @return the result of the specified shell command.
     */
    std::unique_ptr<ShellCmdResult> ExecuteShellCommand(const std::string &cmd, const int64_t timeoutSec);

    /**
     * Prints log information to the console.
     * The total length of the log information to be printed cannot exceed 1000 characters.
     *
     * @param msg, Indicates the log information to print.
     */
    void Print(const std::string &msg);

    /**
     * Saves ability properties when ability is started and notify monitors of state changes.
     *
     * @param ability, Indicates the ability properties.
     */
    void PostPerformStart(const std::shared_ptr<BaseDelegatorAbilityProperty> &ability);

    /**
     * Saves abilityStage properties when abilityStage is started and notify monitors.
     *
     * @param abilityStage , Indicates the abilityStage properties.
     */
    void PostPerformStageStart(const std::shared_ptr<BaseDelegatorAbilityStageProperty> &abilityStage);

    /**
     * Saves ability properties when scence is created and notify monitors of state changes.
     *
     * @param ability, Indicates the ability properties.
     */
    void PostPerformScenceCreated(const std::shared_ptr<BaseDelegatorAbilityProperty> &ability);

    /**
     * Saves ability properties when scence is restored and notify monitors of state changes.
     *
     * @param ability, Indicates the ability properties.
     */
    void PostPerformScenceRestored(const std::shared_ptr<BaseDelegatorAbilityProperty> &ability);

    /**
     * Saves ability properties when scence is destroyed and notify monitors of state changes.
     *
     * @param ability, Indicates the ability properties.
     */
    void PostPerformScenceDestroyed(const std::shared_ptr<BaseDelegatorAbilityProperty> &ability);

    /**
     * Saves ability properties when ability is in the foreground and notify monitors of state changes.
     *
     * @param ability, Indicates the ability properties.
     */
    void PostPerformForeground(const std::shared_ptr<BaseDelegatorAbilityProperty> &ability);

    /**
     * Saves ability properties when ability is in the background and notify monitors of state changes.
     *
     * @param ability, Indicates the ability properties.
     */
    void PostPerformBackground(const std::shared_ptr<BaseDelegatorAbilityProperty> &ability);

    /**
     * Saves ability properties when ability is stopped and notify monitors of state changes.
     *
     * @param ability, Indicates the ability properties.
     */
    void PostPerformStop(const std::shared_ptr<BaseDelegatorAbilityProperty> &ability);

    /**
     * Finishes user test.
     *
     * @param msg, Indicates the status information.The total length of the status information cannot
     * exceed 1000 characters.
     * @param resultCode, Indicates the result code.
     */
    void FinishUserTest(const std::string &msg, const int64_t resultCode);

    /**
     * Registers a function for cleanup.
     *
     * @param func, Indicates the cleanup function, called when the ability is stopped.
     */
    void RegisterClearFunc(ClearFunc func);

private:
    AbilityDelegator::AbilityState ConvertAbilityState(const AbilityLifecycleExecutor::LifecycleState lifecycleState);
    void ProcessAbilityProperties(const std::shared_ptr<BaseDelegatorAbilityProperty> &ability);
    void RemoveAbilityProperty(const std::shared_ptr<BaseDelegatorAbilityProperty> &ability);
    std::shared_ptr<BaseDelegatorAbilityProperty> FindPropertyByToken(const sptr<IRemoteObject> &token);
    std::shared_ptr<BaseDelegatorAbilityProperty> FindPropertyByName(const std::string &name);
    inline void CallClearFunc(const std::shared_ptr<BaseDelegatorAbilityProperty> &ability);
    bool IsFromOtherRuntime(const std::shared_ptr<IInteropAbilityMonitor> &monitor,
        const std::shared_ptr<BaseDelegatorAbilityProperty> &ability);

private:
    static constexpr size_t INFORMATION_MAX_LENGTH {1000};
    static constexpr size_t DELEGATOR_PRINT_MAX_LENGTH {10000};
    const std::string IS_DELEGATOR_CALL = "isDelegatorCall";

private:
    std::shared_ptr<AbilityRuntime::Context> appContext_;
    std::unique_ptr<TestRunner> testRunner_;
    sptr<IRemoteObject> observer_;

    std::unique_ptr<DelegatorThread> delegatorThread_;
    std::list<std::shared_ptr<BaseDelegatorAbilityProperty>> abilityProperties_;
    std::vector<std::shared_ptr<IAbilityMonitor>> abilityMonitors_;
    std::vector<std::shared_ptr<IInteropAbilityMonitor>> interopAbilityMonitors_;
    std::vector<std::shared_ptr<IAbilityStageMonitor>> abilityStageMonitors_;

    ClearFunc clearFunc_;

    std::mutex mutexMonitor_;
    std::mutex mutexAbilityProperties_;
    std::mutex mutexStageMonitor_;
    std::mutex mutexClearMonitor_;
};
}  // namespace AppExecFwk
}  // namespace OHOS

#endif  // OHOS_ABILITY_RUNTIME_ABILITY_DELEGATOR_H
