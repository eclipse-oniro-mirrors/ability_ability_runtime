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

#ifndef OHOS_ABILITY_RUNTIME_LAUNCH_SERVICE_OBSERVER_MANAGER_H
#define OHOS_ABILITY_RUNTIME_LAUNCH_SERVICE_OBSERVER_MANAGER_H

#include <map>
#include <memory>
#include <mutex>

#include "launch_service_observer.h"

namespace OHOS {
namespace AppExecFwk {
class LaunchServiceObserverManager final {
public:
    static LaunchServiceObserverManager &GetInstance();

    int32_t RegisterObserver(
        const sptr<AbilityRuntime::ILaunchServiceObserver> &observer, int32_t userId, int32_t callerUid);
    int32_t UnregisterObserver(const sptr<AbilityRuntime::ILaunchServiceObserver> &observer);
    void NotifyStatusChanged(const AbilityRuntime::LaunchServiceStateData &stateData);
    void NotifyProcessStateChanged(const AbilityRuntime::LaunchServiceProcessStateData &processStateData);

private:
    static constexpr int32_t INVALID_CALLER_UID = -1;

    class ObserverDeathRecipient final : public IRemoteObject::DeathRecipient {
    public:
        explicit ObserverDeathRecipient(int32_t observerId) : observerId_(observerId)
        {
        }
        void OnRemoteDied(const wptr<IRemoteObject> &remote) override;

    private:
        int32_t observerId_;
    };

    struct ObserverRecord {
        sptr<AbilityRuntime::ILaunchServiceObserver> observer;
        sptr<IRemoteObject::DeathRecipient> deathRecipient;
        int32_t userId = 0;
        int32_t callerUid = INVALID_CALLER_UID;
        std::mutex mutex;
        bool active = true;
    };

    LaunchServiceObserverManager() = default;
    int32_t AllocateObserverIdLocked();
    uint32_t CountObserversLocked(int32_t callerUid) const;
    void RemoveObserver(int32_t observerId);

    std::mutex mutex_;
    std::map<int32_t, std::shared_ptr<ObserverRecord>> observers_;
    int32_t nextObserverId_ = 1;
};
} // namespace AppExecFwk
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_LAUNCH_SERVICE_OBSERVER_MANAGER_H
