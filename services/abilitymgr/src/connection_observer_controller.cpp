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

#include "connection_observer_controller.h"

#include "connection_observer_errors.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AAFwk {
using namespace OHOS::AbilityRuntime;
int ConnectionObserverController::AddObserver(const sptr<AbilityRuntime::IConnectionObserver> &observer)
{
    if (!observer) {
        TAG_LOGE(AAFwkTag::CONNECTION, "invalid observer");
        return AbilityRuntime::ERR_INVALID_OBSERVER;
    }

    std::lock_guard<ffrt::mutex> guard(observerLock_);
    auto it = std::find_if(observers_.begin(), observers_.end(), [&observer](const sptr<IConnectionObserver> &item) {
        return (item && item->AsObject() == observer->AsObject());
    });
    if (it != observers_.end()) {
        TAG_LOGW(AAFwkTag::CONNECTION, "observer already added");
        return 0;
    }

    if (!observerDeathRecipient_) {
        std::weak_ptr<ConnectionObserverController> thisWeakPtr(shared_from_this());
        observerDeathRecipient_ =
            new ObserverDeathRecipient([thisWeakPtr](const wptr<IRemoteObject> &remote) {
                auto controller = thisWeakPtr.lock();
                if (controller) {
                    controller->HandleRemoteDied(remote);
                }
            });
    }
    auto observerObj = observer->AsObject();
    if (!observerObj || !observerObj->AddDeathRecipient(observerDeathRecipient_)) {
        TAG_LOGE(AAFwkTag::CONNECTION, "AddDeathRecipient failed");
    }
    observers_.emplace_back(observer);

    return 0;
}

void ConnectionObserverController::RemoveObserver(const sptr<AbilityRuntime::IConnectionObserver> &observer)
{
    if (!observer) {
        TAG_LOGE(AAFwkTag::CONNECTION, "observer invalid");
        return;
    }

    std::lock_guard<ffrt::mutex> guard(observerLock_);
    auto it = std::find_if(observers_.begin(), observers_.end(), [&observer](const sptr<IConnectionObserver> item) {
        return (item && item->AsObject() == observer->AsObject());
    });
    if (it != observers_.end()) {
        observers_.erase(it);
    }
}

void ConnectionObserverController::NotifyExtensionConnected(const AbilityRuntime::ConnectionData& data)
{
    CallObservers(&AbilityRuntime::IConnectionObserver::OnExtensionConnected, data);
}

void ConnectionObserverController::NotifyExtensionDisconnected(const AbilityRuntime::ConnectionData& data)
{
    CallObservers(&AbilityRuntime::IConnectionObserver::OnExtensionDisconnected, data);
}


void ConnectionObserverController::NotifyExtensionSuspended(const AbilityRuntime::ConnectionData& data)
{
    CallObservers(&AbilityRuntime::IConnectionObserver::OnExtensionSuspended, data);
}


void ConnectionObserverController::NotifyExtensionResumed(const AbilityRuntime::ConnectionData& data)
{
    CallObservers(&AbilityRuntime::IConnectionObserver::OnExtensionResumed, data);
}

#ifdef WITH_DLP
void ConnectionObserverController::NotifyDlpAbilityOpened(const AbilityRuntime::DlpStateData& data)
{
    CallObservers(&AbilityRuntime::IConnectionObserver::OnDlpAbilityOpened, data);
}

void ConnectionObserverController::NotifyDlpAbilityClosed(const AbilityRuntime::DlpStateData& data)
{
    CallObservers(&AbilityRuntime::IConnectionObserver::OnDlpAbilityClosed, data);
}
#endif // WITH_DLP

std::vector<sptr<AbilityRuntime::IConnectionObserver>> ConnectionObserverController::GetObservers()
{
    std::lock_guard<ffrt::mutex> guard(observerLock_);
    return observers_;
}

void ConnectionObserverController::HandleRemoteDied(const wptr<IRemoteObject> &remote)
{
    TAG_LOGD(AAFwkTag::CONNECTION, "remote connection oberver died");
    auto remoteObj = remote.promote();
    if (!remoteObj) {
        TAG_LOGD(AAFwkTag::CONNECTION, "invalid remoteObj");
        return;
    }
    remoteObj->RemoveDeathRecipient(observerDeathRecipient_);

    std::lock_guard<ffrt::mutex> guard(observerLock_);
    auto it = std::find_if(observers_.begin(), observers_.end(), [&remoteObj](const sptr<IConnectionObserver> item) {
        return (item && item->AsObject() == remoteObj);
    });
    if (it != observers_.end()) {
        observers_.erase(it);
    }
}

ConnectionObserverController::ObserverDeathRecipient::ObserverDeathRecipient(ObserverDeathHandler handler)
    : deathHandler_(handler)
{}

void ConnectionObserverController::ObserverDeathRecipient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    if (deathHandler_) {
        deathHandler_(remote);
    }
}
} // namespace AAFwk
} // namespace OHOS
