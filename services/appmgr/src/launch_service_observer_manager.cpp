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

#include "launch_service_observer_manager.h"

#include <limits>
#include <new>
#include <vector>

#include "ipc_types.h"
#include "ability_manager_errors.h"
#include "parameters.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
constexpr char ENTERPRISE_SPACE_ENABLE[] = "persist.space_mgr_service.enterprise_space_enable";
constexpr uint32_t MAX_OBSERVERS_PER_APP = 10;

bool ShouldNotifyObserver(
    int32_t observerUserId, const AbilityRuntime::LaunchServiceDescriptor &descriptor, bool enterpriseSpaceEnabled)
{
    if (!enterpriseSpaceEnabled || observerUserId == 0) {
        return true;
    }
    return descriptor.userId == observerUserId;
}
} // namespace

LaunchServiceObserverManager &LaunchServiceObserverManager::GetInstance()
{
    static LaunchServiceObserverManager instance;
    return instance;
}

int32_t LaunchServiceObserverManager::AllocateObserverIdLocked()
{
    for (size_t count = 0; count <= observers_.size(); ++count) {
        if (nextObserverId_ <= 0) {
            nextObserverId_ = 1;
        }
        const int32_t candidate = nextObserverId_;
        nextObserverId_ = nextObserverId_ == std::numeric_limits<int32_t>::max() ? 1 : nextObserverId_ + 1;
        if (observers_.find(candidate) == observers_.end()) {
            return candidate;
        }
    }
    return -1;
}

uint32_t LaunchServiceObserverManager::CountObserversLocked(int32_t callerUid) const
{
    uint32_t count = 0;
    for (const auto &[id, record] : observers_) {
        if (record->callerUid == callerUid) {
            ++count;
        }
    }
    return count;
}

int32_t LaunchServiceObserverManager::RegisterObserver(
    const sptr<AbilityRuntime::ILaunchServiceObserver> &observer, int32_t userId, int32_t callerUid)
{
    if (observer == nullptr || observer->AsObject() == nullptr) {
        return ERR_INVALID_VALUE;
    }
    auto remote = observer->AsObject();
    std::lock_guard<std::mutex> lock(mutex_);
    for (const auto &item : observers_) {
        if (item.second->observer->AsObject() == remote) {
            if (item.second->callerUid != callerUid) {
                return ERR_INVALID_VALUE;
            }
            return ERR_OK;
        }
    }
    if (CountObserversLocked(callerUid) >= MAX_OBSERVERS_PER_APP) {
        return AAFwk::ERR_LAUNCH_SERVICE_OBSERVER_LIMIT;
    }
    const int32_t id = AllocateObserverIdLocked();
    if (id <= 0) {
        return ERR_NO_MEMORY;
    }
    sptr<IRemoteObject::DeathRecipient> recipient = new (std::nothrow) ObserverDeathRecipient(id);
    if (recipient == nullptr) {
        return ERR_NO_MEMORY;
    }
    auto record = std::make_shared<ObserverRecord>();
    record->observer = observer;
    record->deathRecipient = recipient;
    record->userId = userId;
    record->callerUid = callerUid;
    observers_.emplace(id, record);
    if (remote->IsProxyObject() && !remote->AddDeathRecipient(recipient)) {
        observers_.erase(id);
        return ERR_INVALID_STATE;
    }
    return ERR_OK;
}

int32_t LaunchServiceObserverManager::UnregisterObserver(const sptr<AbilityRuntime::ILaunchServiceObserver> &observer)
{
    if (observer == nullptr || observer->AsObject() == nullptr) {
        return ERR_INVALID_VALUE;
    }
    std::shared_ptr<ObserverRecord> record;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        for (auto iter = observers_.begin(); iter != observers_.end(); ++iter) {
            if (iter->second->observer->AsObject() == observer->AsObject()) {
                record = iter->second;
                observers_.erase(iter);
                break;
            }
        }
    }
    if (record == nullptr) {
        return ERR_INVALID_VALUE;
    }
    {
        std::lock_guard<std::mutex> lock(record->mutex);
        record->active = false;
    }
    auto remote = record->observer->AsObject();
    if (remote != nullptr && remote->IsProxyObject()) {
        remote->RemoveDeathRecipient(record->deathRecipient);
    }
    return ERR_OK;
}

void LaunchServiceObserverManager::RemoveObserver(int32_t observerId)
{
    std::shared_ptr<ObserverRecord> record;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto iter = observers_.find(observerId);
        if (iter == observers_.end()) {
            return;
        }
        record = iter->second;
        observers_.erase(iter);
    }
    {
        std::lock_guard<std::mutex> lock(record->mutex);
        record->active = false;
    }
    auto remote = record->observer->AsObject();
    if (remote != nullptr && remote->IsProxyObject()) {
        remote->RemoveDeathRecipient(record->deathRecipient);
    }
}

void LaunchServiceObserverManager::NotifyStatusChanged(const AbilityRuntime::LaunchServiceStateData &stateData)
{
    const bool enterpriseSpaceEnabled = system::GetBoolParameter(ENTERPRISE_SPACE_ENABLE, false);
    std::vector<std::shared_ptr<ObserverRecord>> observers;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        for (const auto &item : observers_) {
            observers.emplace_back(item.second);
        }
    }
    for (const auto &record : observers) {
        std::lock_guard<std::mutex> lock(record->mutex);
        if (record->active && ShouldNotifyObserver(record->userId, stateData.descriptor, enterpriseSpaceEnabled)) {
            record->observer->OnLaunchServiceStatusChanged(stateData);
        }
    }
}

void LaunchServiceObserverManager::NotifyProcessStateChanged(
    const AbilityRuntime::LaunchServiceProcessStateData &processStateData)
{
    const bool enterpriseSpaceEnabled = system::GetBoolParameter(ENTERPRISE_SPACE_ENABLE, false);
    std::vector<std::shared_ptr<ObserverRecord>> observers;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        for (const auto &item : observers_) {
            observers.emplace_back(item.second);
        }
    }
    for (const auto &record : observers) {
        std::lock_guard<std::mutex> lock(record->mutex);
        if (record->active &&
            ShouldNotifyObserver(record->userId, processStateData.descriptor, enterpriseSpaceEnabled)) {
            record->observer->OnLaunchServiceProcessStateChanged(processStateData);
        }
    }
}

void LaunchServiceObserverManager::ObserverDeathRecipient::OnRemoteDied(
    const wptr<IRemoteObject> &)
{
    LaunchServiceObserverManager::GetInstance().RemoveObserver(observerId_);
}
} // namespace AppExecFwk
} // namespace OHOS
