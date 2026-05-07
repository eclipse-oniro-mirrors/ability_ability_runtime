/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include "ability_delegator.h"

#include "hilog_tag_wrapper.h"
#include "ohos_application.h"
#include "ability_manager_client.h"
#include "ability_delegator_registry.h"
#include "itest_observer.h"

namespace OHOS {
namespace AppExecFwk {
std::shared_ptr<AbilityDelegator> AbilityDelegator::Create(const std::shared_ptr<AbilityRuntime::Context>& context,
    std::unique_ptr<TestRunner> runner, const sptr<IRemoteObject>& observer)
{
    return std::make_shared<AbilityDelegator>(context, std::move(runner), observer);
}

AbilityDelegator::AbilityDelegator(const std::shared_ptr<AbilityRuntime::Context> &context,
    std::unique_ptr<TestRunner> runner, const sptr<IRemoteObject> &observer)
    : appContext_(context), testRunner_(std::move(runner)), observer_(observer)
{}

AbilityDelegator::~AbilityDelegator()
{}

void AbilityDelegator::AddAbilityMonitor(const std::shared_ptr<IAbilityMonitor> &monitor)
{
    if (!monitor) {
        TAG_LOGW(AAFwkTag::DELEGATOR, "invalid params");
        return;
    }

    std::unique_lock<std::mutex> lck(mutexMonitor_);
    auto pos = std::find(abilityMonitors_.begin(), abilityMonitors_.end(), monitor);
    if (pos != abilityMonitors_.end()) {
        TAG_LOGW(AAFwkTag::DELEGATOR, "monitor added");
        return;
    }

    abilityMonitors_.emplace_back(monitor);
}

void AbilityDelegator::AddInteropAbilityMonitor(const std::shared_ptr<IInteropAbilityMonitor> &monitor)
{
    if (!monitor) {
        TAG_LOGW(AAFwkTag::DELEGATOR, "invalid params");
        return;
    }

    std::unique_lock<std::mutex> lck(mutexMonitor_);
    auto pos = std::find(interopAbilityMonitors_.begin(), interopAbilityMonitors_.end(), monitor);
    if (pos != interopAbilityMonitors_.end()) {
        TAG_LOGW(AAFwkTag::DELEGATOR, "interop monitor added");
        return;
    }

    interopAbilityMonitors_.emplace_back(monitor);
}

void AbilityDelegator::AddAbilityStageMonitor(const std::shared_ptr<IAbilityStageMonitor> &monitor)
{
    if (!monitor) {
        TAG_LOGW(AAFwkTag::DELEGATOR, "invalid params");
        return;
    }
    std::unique_lock<std::mutex> lck(mutexStageMonitor_);
    auto pos = std::find(abilityStageMonitors_.begin(), abilityStageMonitors_.end(), monitor);
    if (pos != abilityStageMonitors_.end()) {
        TAG_LOGW(AAFwkTag::DELEGATOR, "stage monitor added");
        return;
    }
    abilityStageMonitors_.emplace_back(monitor);
}

void AbilityDelegator::RemoveAbilityMonitor(const std::shared_ptr<IAbilityMonitor> &monitor)
{
    if (!monitor) {
        TAG_LOGW(AAFwkTag::DELEGATOR, "invalid params");
        return;
    }

    std::unique_lock<std::mutex> lck(mutexMonitor_);
    auto pos = std::find(abilityMonitors_.begin(), abilityMonitors_.end(), monitor);
    if (pos != abilityMonitors_.end()) {
        abilityMonitors_.erase(pos);
    }
}

void AbilityDelegator::RemoveInteropAbilityMonitor(const std::shared_ptr<IInteropAbilityMonitor> &monitor)
{
    if (!monitor) {
        TAG_LOGW(AAFwkTag::DELEGATOR, "invalid params");
        return;
    }

    std::unique_lock<std::mutex> lck(mutexMonitor_);
    auto pos = std::find(interopAbilityMonitors_.begin(), interopAbilityMonitors_.end(), monitor);
    if (pos != interopAbilityMonitors_.end()) {
        interopAbilityMonitors_.erase(pos);
    }
}

void AbilityDelegator::RemoveAbilityStageMonitor(const std::shared_ptr<IAbilityStageMonitor> &monitor)
{
    if (!monitor) {
        TAG_LOGW(AAFwkTag::DELEGATOR, "invalid params");
        return;
    }
    std::unique_lock<std::mutex> lck(mutexStageMonitor_);
    auto pos = std::find(abilityStageMonitors_.begin(), abilityStageMonitors_.end(), monitor);
    if (pos != abilityStageMonitors_.end()) {
        abilityStageMonitors_.erase(pos);
    }
}

void AbilityDelegator::ClearAllMonitors()
{
    std::unique_lock<std::mutex> lck(mutexMonitor_);
    abilityMonitors_.clear();
    interopAbilityMonitors_.clear();
}

size_t AbilityDelegator::GetMonitorsNum()
{
    std::unique_lock<std::mutex> lck(mutexMonitor_);
    return abilityMonitors_.size();
}

size_t AbilityDelegator::GetStageMonitorsNum()
{
    std::unique_lock<std::mutex> lck(mutexStageMonitor_);
    return abilityStageMonitors_.size();
}


std::shared_ptr<BaseDelegatorAbilityProperty> AbilityDelegator::WaitAbilityMonitor(
    const std::shared_ptr<IAbilityMonitor> &monitor)
{
    if (!monitor) {
        TAG_LOGW(AAFwkTag::DELEGATOR, "invalid params");
        return {};
    }

    AddAbilityMonitor(monitor);

    auto obtainedAbility = monitor->WaitForAbility();
    if (!obtainedAbility) {
        TAG_LOGW(AAFwkTag::DELEGATOR, "invalid obtained ability");
        return {};
    }

    return obtainedAbility;
}

std::shared_ptr<BaseDelegatorAbilityStageProperty> AbilityDelegator::WaitAbilityStageMonitor(
    const std::shared_ptr<IAbilityStageMonitor> &monitor)
{
    if (!monitor) {
        TAG_LOGW(AAFwkTag::DELEGATOR, "invalid monitor");
        return nullptr;
    }

    AddAbilityStageMonitor(monitor);
    auto stage = monitor->WaitForAbilityStage();
    if (!stage) {
        TAG_LOGW(AAFwkTag::DELEGATOR, "invalid abilityStage");
        return nullptr;
    }
    return stage;
}

std::shared_ptr<BaseDelegatorAbilityProperty> AbilityDelegator::WaitAbilityMonitor(
    const std::shared_ptr<IAbilityMonitor> &monitor, const int64_t timeoutMs)
{
    if (!monitor) {
        TAG_LOGW(AAFwkTag::DELEGATOR, "invalid params");
        return {};
    }

    AddAbilityMonitor(monitor);

    auto obtainedAbility = monitor->WaitForAbility(timeoutMs);
    if (!obtainedAbility) {
        TAG_LOGW(AAFwkTag::DELEGATOR, "invalid obtained ability");
        return {};
    }

    return obtainedAbility;
}

std::shared_ptr<BaseDelegatorAbilityStageProperty> AbilityDelegator::WaitAbilityStageMonitor(
    const std::shared_ptr<IAbilityStageMonitor> &monitor, const int64_t timeoutMs)
{
    if (!monitor) {
        TAG_LOGW(AAFwkTag::DELEGATOR, "invalid monitor");
        return nullptr;
    }
    AddAbilityStageMonitor(monitor);
    auto stage = monitor->WaitForAbilityStage(timeoutMs);
    if (!stage) {
        TAG_LOGW(AAFwkTag::DELEGATOR, "invalid obtained abilityStage");
        return nullptr;
    }
    return stage;
}

std::shared_ptr<AbilityRuntime::Context> AbilityDelegator::GetAppContext() const
{
    return appContext_;
}

AbilityDelegator::AbilityState AbilityDelegator::GetAbilityState(const sptr<IRemoteObject> &token)
{
    if (!token) {
        TAG_LOGW(AAFwkTag::DELEGATOR, "invalid params");
        return AbilityDelegator::AbilityState::UNINITIALIZED;
    }

    std::unique_lock<std::mutex> lck(mutexAbilityProperties_);
    auto existedProperty = FindPropertyByToken(token);
    if (!existedProperty) {
        TAG_LOGW(AAFwkTag::DELEGATOR, "unknown ability token");
        return AbilityDelegator::AbilityState::UNINITIALIZED;
    }

    return ConvertAbilityState(existedProperty->lifecycleState_);
}

std::shared_ptr<BaseDelegatorAbilityProperty> AbilityDelegator::GetCurrentTopAbility()
{
    AppExecFwk::ElementName elementName = AAFwk::AbilityManagerClient::GetInstance()->GetTopAbility();
    std::string bundleName = elementName.GetBundleName();
    std::string abilityName = elementName.GetAbilityName();
    if (abilityName.empty()) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "get topAbility failed");
        return {};
    }

    if (!bundleName.empty()) {
        std::string::size_type pos = abilityName.find(bundleName);
        if (pos == std::string::npos || pos != 0) {
            abilityName = bundleName + "." + abilityName;
        }
    }

    std::unique_lock<std::mutex> lck(mutexAbilityProperties_);
    auto existedProperty = FindPropertyByName(abilityName);
    if (!existedProperty) {
        TAG_LOGW(AAFwkTag::DELEGATOR, "unknown ability name");
        return {};
    }

    return existedProperty;
}

std::string AbilityDelegator::GetThreadName() const
{
    return {};
}

void AbilityDelegator::Prepare()
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "Enter");
    if (!testRunner_) {
        TAG_LOGW(AAFwkTag::DELEGATOR, "invalid TestRunner");
        return;
    }

    TAG_LOGD(AAFwkTag::DELEGATOR, "call js onPrepare()");
    testRunner_->Prepare();

    if (!delegatorThread_) {
        delegatorThread_ = std::make_unique<DelegatorThread>(true);
        if (!delegatorThread_) {
            TAG_LOGE(AAFwkTag::DELEGATOR, "create delegatorThread failed");
            return;
        }
    }

    auto runTask = [this]() { this->OnRun(); };
    if (!delegatorThread_->Run(runTask)) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "run task on delegatorThread failed");
    }
}

void AbilityDelegator::OnRun()
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "Enter");
    if (!testRunner_) {
        TAG_LOGW(AAFwkTag::DELEGATOR, "invalid TestRunner");
        return;
    }

    TAG_LOGD(AAFwkTag::DELEGATOR, "call js onRun()");
    testRunner_->Run();
}

ErrCode AbilityDelegator::StartAbility(const AAFwk::Want &want)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "Enter");

    auto realWant(want);
    auto delegatorArgs = AbilityDelegatorRegistry::GetArguments();
    if (delegatorArgs && delegatorArgs->FindDebugFlag()) {
        TAG_LOGI(AAFwkTag::DELEGATOR, "start with debug");
        realWant.SetParam("debugApp", true);
    }
    realWant.SetParam(IS_DELEGATOR_CALL, true);

    return AbilityManagerClient::GetInstance()->StartAbility(realWant);
}

bool AbilityDelegator::DoAbilityForeground(const sptr<IRemoteObject> &token)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "called");

    if (!token) {
        TAG_LOGW(AAFwkTag::DELEGATOR, "invalid params");
        return false;
    }

    auto ret = AAFwk::AbilityManagerClient::GetInstance()->DelegatorDoAbilityForeground(token);
    if (ret) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "ret: %{public}d", ret);
        return false;
    }

    return true;
}

bool AbilityDelegator::DoAbilityBackground(const sptr<IRemoteObject> &token)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "Enter");

    if (!token) {
        TAG_LOGW(AAFwkTag::DELEGATOR, "invalid params");
        return false;
    }

    auto ret = AAFwk::AbilityManagerClient::GetInstance()->DelegatorDoAbilityBackground(token);
    if (ret) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "ret: %{public}d", ret);
        return false;
    }

    return true;
}

std::unique_ptr<ShellCmdResult> AbilityDelegator::ExecuteShellCommand(const std::string &cmd, const int64_t timeoutSec)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "command : %{public}s, timeout : %{public}" PRId64, cmd.data(), timeoutSec);

    if (cmd.empty()) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "invalid cmd");
        return {};
    }

    auto testObserver = iface_cast<ITestObserver>(observer_);
    if (!testObserver) {
        TAG_LOGW(AAFwkTag::DELEGATOR, "invalid testObserver");
        return {};
    }

    auto result = testObserver->ExecuteShellCommand(cmd, timeoutSec);
    return std::make_unique<ShellCmdResult>(result);
}

void AbilityDelegator::Print(const std::string &msg)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "Enter");

    auto testObserver = iface_cast<ITestObserver>(observer_);
    if (!testObserver) {
        TAG_LOGW(AAFwkTag::DELEGATOR, "invalid testObserver");
        return;
    }

    auto realMsg(msg);
    if (realMsg.length() > DELEGATOR_PRINT_MAX_LENGTH) {
        TAG_LOGW(AAFwkTag::DELEGATOR, "too long message");
        realMsg.resize(DELEGATOR_PRINT_MAX_LENGTH);
    }
    TAG_LOGI(AAFwkTag::DELEGATOR, "message: %{public}s", realMsg.data());

    testObserver->TestStatus(realMsg, 0);
}

void AbilityDelegator::PostPerformStart(const std::shared_ptr<BaseDelegatorAbilityProperty> &ability)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "called");

    if (!ability) {
        TAG_LOGW(AAFwkTag::DELEGATOR, "invalid params");
        return;
    }

    ProcessAbilityProperties(ability);

    std::unique_lock<std::mutex> lck(mutexMonitor_);
    for (auto &monitor : abilityMonitors_) {
        if (!monitor) {
            continue;
        }
        if (monitor->Match(ability, true)) {
            monitor->OnAbilityStart(ability);
        }
    }

    for (auto &monitor : interopAbilityMonitors_) {
        if (!monitor) {
            continue;
        }
        if (IsFromOtherRuntime(monitor, ability) && monitor->Match(ability, true)) {
            monitor->OnAbilityStart(ability);
        }
    }
}

void AbilityDelegator::PostPerformStageStart(const std::shared_ptr<BaseDelegatorAbilityStageProperty> &abilityStage)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "called");
    if (!abilityStage) {
        TAG_LOGW(AAFwkTag::DELEGATOR, "invalid params");
        return;
    }

    std::unique_lock<std::mutex> lck(mutexStageMonitor_);
    if (abilityStageMonitors_.empty()) {
        TAG_LOGW(AAFwkTag::DELEGATOR, "null abilityStageMonitors");
        return;
    }

    for (auto &monitor : abilityStageMonitors_) {
        if (!monitor) {
            continue;
        }
        monitor->Match(abilityStage, true);
    }
}

void AbilityDelegator::PostPerformScenceCreated(const std::shared_ptr<BaseDelegatorAbilityProperty> &ability)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "called");

    if (!ability) {
        TAG_LOGW(AAFwkTag::DELEGATOR, "invalid params");
        return;
    }

    ProcessAbilityProperties(ability);

    std::unique_lock<std::mutex> lck(mutexMonitor_);
    for (auto &monitor : abilityMonitors_) {
        if (!monitor) {
            continue;
        }
        if (monitor->Match(ability, true)) {
            monitor->OnWindowStageCreate(ability);
        }
    }

    for (auto &monitor : interopAbilityMonitors_) {
        if (!monitor) {
            continue;
        }
        if (IsFromOtherRuntime(monitor, ability) && monitor->Match(ability, true)) {
            monitor->OnWindowStageCreate(ability);
        }
    }
}

void AbilityDelegator::PostPerformScenceRestored(const std::shared_ptr<BaseDelegatorAbilityProperty> &ability)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "called");

    if (!ability) {
        TAG_LOGW(AAFwkTag::DELEGATOR, "invalid params");
        return;
    }

    ProcessAbilityProperties(ability);

    std::unique_lock<std::mutex> lck(mutexMonitor_);
    for (auto &monitor : abilityMonitors_) {
        if (!monitor) {
            continue;
        }
        if (monitor->Match(ability, true)) {
            monitor->OnWindowStageRestore(ability);
        }
    }

    for (auto &monitor : interopAbilityMonitors_) {
        if (!monitor) {
            continue;
        }
        if (IsFromOtherRuntime(monitor, ability) && monitor->Match(ability, true)) {
            monitor->OnWindowStageRestore(ability);
        }
    }
}

void AbilityDelegator::PostPerformScenceDestroyed(const std::shared_ptr<BaseDelegatorAbilityProperty> &ability)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "called");

    if (!ability) {
        TAG_LOGW(AAFwkTag::DELEGATOR, "invalid params");
        return;
    }

    ProcessAbilityProperties(ability);

    std::unique_lock<std::mutex> lck(mutexMonitor_);
    for (auto &monitor : abilityMonitors_) {
        if (!monitor) {
            continue;
        }
        if (monitor->Match(ability, true)) {
            monitor->OnWindowStageDestroy(ability);
        }
    }

    for (auto &monitor : interopAbilityMonitors_) {
        if (!monitor) {
            continue;
        }
        if (IsFromOtherRuntime(monitor, ability) && monitor->Match(ability, true)) {
            monitor->OnWindowStageDestroy(ability);
        }
    }
}

void AbilityDelegator::PostPerformForeground(const std::shared_ptr<BaseDelegatorAbilityProperty> &ability)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "called");

    if (!ability) {
        TAG_LOGW(AAFwkTag::DELEGATOR, "invalid params");
        return;
    }

    ProcessAbilityProperties(ability);

    std::unique_lock<std::mutex> lck(mutexMonitor_);
    for (auto &monitor : abilityMonitors_) {
        if (!monitor) {
            continue;
        }
        if (monitor->Match(ability, true)) {
            monitor->OnAbilityForeground(ability);
        }
    }

    for (auto &monitor : interopAbilityMonitors_) {
        if (!monitor) {
            continue;
        }
        if (IsFromOtherRuntime(monitor, ability) && monitor->Match(ability, true)) {
            monitor->OnAbilityForeground(ability);
        }
    }
}

void AbilityDelegator::PostPerformBackground(const std::shared_ptr<BaseDelegatorAbilityProperty> &ability)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "called");

    if (!ability) {
        TAG_LOGW(AAFwkTag::DELEGATOR, "invalid params");
        return;
    }

    ProcessAbilityProperties(ability);

    std::unique_lock<std::mutex> lck(mutexMonitor_);
    for (auto &monitor : abilityMonitors_) {
        if (!monitor) {
            continue;
        }
        if (monitor->Match(ability, true)) {
            monitor->OnAbilityBackground(ability);
        }
    }

    for (auto &monitor : interopAbilityMonitors_) {
        if (!monitor) {
            continue;
        }
        if (IsFromOtherRuntime(monitor, ability) && monitor->Match(ability, true)) {
            monitor->OnAbilityBackground(ability);
        }
    }
}

void AbilityDelegator::PostPerformStop(const std::shared_ptr<BaseDelegatorAbilityProperty> &ability)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "Enter");

    if (!ability) {
        TAG_LOGW(AAFwkTag::DELEGATOR, "invalid params");
        return;
    }

    ProcessAbilityProperties(ability);

    std::unique_lock<std::mutex> lck(mutexMonitor_);
    for (auto &monitor : abilityMonitors_) {
        if (!monitor) {
            continue;
        }
        if (monitor->Match(ability, true)) {
            monitor->OnAbilityStop(ability);
        }
    }

    for (auto &monitor : interopAbilityMonitors_) {
        if (!monitor) {
            continue;
        }
        if (IsFromOtherRuntime(monitor, ability) && monitor->Match(ability, true)) {
            monitor->OnAbilityStop(ability);
        }
    }

    RemoveAbilityProperty(ability);
    CallClearFunc(ability);
}

AbilityDelegator::AbilityState AbilityDelegator::ConvertAbilityState(
    const AbilityLifecycleExecutor::LifecycleState lifecycleState)
{
    AbilityDelegator::AbilityState abilityState {AbilityDelegator::AbilityState::UNINITIALIZED};
    switch (lifecycleState) {
        case AbilityLifecycleExecutor::LifecycleState::STARTED_NEW:
            abilityState = AbilityDelegator::AbilityState::STARTED;
            break;
        case AbilityLifecycleExecutor::LifecycleState::FOREGROUND_NEW:
            abilityState = AbilityDelegator::AbilityState::FOREGROUND;
            break;
        case AbilityLifecycleExecutor::LifecycleState::BACKGROUND_NEW:
            abilityState = AbilityDelegator::AbilityState::BACKGROUND;
            break;
        case AbilityLifecycleExecutor::LifecycleState::STOPED_NEW:
            abilityState = AbilityDelegator::AbilityState::STOPPED;
            break;
        default:
            TAG_LOGE(AAFwkTag::DELEGATOR, "Unknown lifecycleState");
            break;
    }

    return abilityState;
}

void AbilityDelegator::ProcessAbilityProperties(const std::shared_ptr<BaseDelegatorAbilityProperty> &ability)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "Enter");

    if (!ability) {
        TAG_LOGW(AAFwkTag::DELEGATOR, "invalid ability property");
        return;
    }

    TAG_LOGW(AAFwkTag::DELEGATOR, "ability property : name : %{public}s, state : %{public}d",
        ability->name_.data(), ability->lifecycleState_);

    std::unique_lock<std::mutex> lck(mutexAbilityProperties_);
    auto existedProperty = FindPropertyByToken(ability->token_);
    if (existedProperty) {
        // update
        existedProperty->lifecycleState_ = ability->lifecycleState_;
        return;
    }

    abilityProperties_.emplace_back(ability);
}

void AbilityDelegator::RemoveAbilityProperty(const std::shared_ptr<BaseDelegatorAbilityProperty> &ability)
{
    TAG_LOGD(AAFwkTag::DELEGATOR, "called");

    if (!ability) {
        TAG_LOGW(AAFwkTag::DELEGATOR, "invalid ability property");
        return;
    }

    TAG_LOGI(AAFwkTag::DELEGATOR, "ability property { name : %{public}s, state : %{public}d }",
        ability->name_.data(), ability->lifecycleState_);

    std::unique_lock<std::mutex> lck(mutexAbilityProperties_);
    abilityProperties_.remove_if([ability](const auto &properties) {
        return ability->fullName_ == properties->fullName_;
    });
}

std::shared_ptr<BaseDelegatorAbilityProperty> AbilityDelegator::FindPropertyByToken(const sptr<IRemoteObject> &token)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "Enter");

    if (!token) {
        TAG_LOGW(AAFwkTag::DELEGATOR, "invalid params");
        return {};
    }

    for (const auto &it : abilityProperties_) {
        if (!it) {
            TAG_LOGW(AAFwkTag::DELEGATOR, "invalid ability property");
            continue;
        }

        if (token == it->token_) {
            TAG_LOGD(AAFwkTag::DELEGATOR, "property exist");
            return it;
        }
    }

    return {};
}

std::shared_ptr<BaseDelegatorAbilityProperty> AbilityDelegator::FindPropertyByName(const std::string &name)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "find property by %{public}s", name.c_str());

    if (name.empty()) {
        TAG_LOGW(AAFwkTag::DELEGATOR, "invalid params");
        return {};
    }

    for (const auto &it : abilityProperties_) {
        if (!it) {
            TAG_LOGW(AAFwkTag::DELEGATOR, "invalid ability property");
            continue;
        }

        if (name == it->fullName_) {
            TAG_LOGI(AAFwkTag::DELEGATOR, "property exist");
            return it;
        }
    }

    return {};
}

void AbilityDelegator::FinishUserTest(const std::string &msg, const int64_t resultCode)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "msg: %{public}s, code: %{public}" PRId64, msg.data(), resultCode);

    if (!observer_) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "invalid observer");
        return;
    }
    auto delegatorArgs = AbilityDelegatorRegistry::GetArguments();
    if (!delegatorArgs) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "invalid args");
        return;
    }
    auto realMsg(msg);
    if (realMsg.length() > INFORMATION_MAX_LENGTH) {
        TAG_LOGW(AAFwkTag::DELEGATOR, "too long message");
        realMsg.resize(INFORMATION_MAX_LENGTH);
    }
    if (!delegatorThread_) {
        delegatorThread_ = std::make_unique<DelegatorThread>(true);
        if (!delegatorThread_) {
            TAG_LOGE(AAFwkTag::DELEGATOR, "create delegatorThread failed");
            return;
        }
    }
    auto runTask = [weak = weak_from_this(), realMsg, resultCode, delegatorArgs]() {
        auto abilityDelegator = weak.lock();
        if (!abilityDelegator || !abilityDelegator->testRunner_) {
            TAG_LOGW(AAFwkTag::DELEGATOR, "invalid TestRunner");
            return;
        }

        TAG_LOGI(AAFwkTag::DELEGATOR, "call js onStop()");
        abilityDelegator->testRunner_->Stop();
        
        const auto &bundleName = delegatorArgs->GetTestBundleName();
        auto err = AAFwk::AbilityManagerClient::GetInstance()->FinishUserTest(realMsg, resultCode, bundleName);
        if (err) {
            TAG_LOGE(AAFwkTag::DELEGATOR, "call FinishUserTest failed: %{public}d", err);
        }
    };
    if (!delegatorThread_->Run(runTask)) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "run task on delegatorThread failed");
    }
}

void AbilityDelegator::RegisterClearFunc(ClearFunc func)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "Enter");
    if (!func) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "invalid func");
        return;
    }
    std::lock_guard<std::mutex> lck(mutexClearMonitor_);
    clearFunc_ = func;
}

inline void AbilityDelegator::CallClearFunc(const std::shared_ptr<BaseDelegatorAbilityProperty> &ability)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "Enter");
    std::lock_guard<std::mutex> lck(mutexClearMonitor_);
    if (clearFunc_) {
        clearFunc_(ability);
    }
}

bool AbilityDelegator::IsFromOtherRuntime(const std::shared_ptr<IInteropAbilityMonitor> &monitor,
    const std::shared_ptr<BaseDelegatorAbilityProperty> &ability)
{
    if (!monitor || !ability) {
        return false;
    }
    return monitor->GetLanguage() != ability->language_;
}
}  // namespace AppExecFwk
}  // namespace OHOS
