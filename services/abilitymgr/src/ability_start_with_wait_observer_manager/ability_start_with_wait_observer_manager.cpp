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

#include "ability_start_with_wait_observer_manager.h"

#include <chrono>

#include "ability_start_with_wait_observer_data.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"

namespace OHOS {
namespace AAFwk {

AbilityStartWithWaitObserverManager& AbilityStartWithWaitObserverManager::GetInstance()
{
    static AbilityStartWithWaitObserverManager instance;
    return instance;
}

int32_t AbilityStartWithWaitObserverManager::RegisterObserver(Want &want, sptr<IAbilityStartWithWaitObserver> observer)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "AbilityStartWithWaitObserverManager::RegisterObserver called");
    if (!observer || !observer->AsObject()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null observer");
        return ERR_NULL_OBJECT;
    }

    std::lock_guard<std::mutex> guard(oberverMutex_);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "observer list size:%{public}zu", observerList_.size());
    for (auto& ob : observerList_) {
        if (!ob.first) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "null observer in observerList_");
            continue;
        }
        if (observer->AsObject() == ob.first->AsObject()) {
            TAG_LOGI(AAFwkTag::ABILITYMGR, "observer already register");
            return ERR_OK;
        }
    }
    AbilityForegroundInfo abilityForegroundInfo;
    observerId_ = (observerId_ == INT_MAX) ? 0 : (observerId_ + 1);
    abilityForegroundInfo.observerId = observerId_;
    abilityForegroundInfo.startTime = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    // 1.save deathRecipient_
    abilityForegroundInfo.deathRecipient = GenerateDeathRecipient(observer);
    observerList_.emplace(observer, abilityForegroundInfo);
    want.SetParam(Want::START_ABILITY_WITH_WAIT_OBSERVER_ID_KEY, observerId_);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "observer list size:%{public}zu", observerList_.size());
    return ERR_OK;
}

void AbilityStartWithWaitObserverManager::UnregisterObserver(sptr<IAbilityStartWithWaitObserver> observer)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "AbilityStartWithWaitObserverManager::UnregisterObserver called");
    if (!observer || !observer->AsObject()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null observer");
        return;
    }
    std::lock_guard<std::mutex> guard(oberverMutex_);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "observer list size:%{public}zu", observerList_.size());
    for (auto& ob : observerList_) {
        if (!ob.first) {
            continue;
        }
        if (ob.first->AsObject() == observer->AsObject()) {
            // 1.remove deathRecipient for proxy
            observer->AsObject()->RemoveDeathRecipient(ob.second.deathRecipient);
            observerList_.erase(ob.first);
            break;
        }
    }
    TAG_LOGD(AAFwkTag::ABILITYMGR, "observer list size:%{public}zu", observerList_.size());
}

void AbilityStartWithWaitObserverManager::NotifyAATerminateWait(Want& want, TerminateReason reason)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "AbilityStartWithWaitObserverManager::NotifyAATerminateWait called");
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto observerId = want.GetIntParam(Want::START_ABILITY_WITH_WAIT_OBSERVER_ID_KEY, -1);
    if (observerId == -1) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "invaid observerId");
        return;
    }
    sptr<IAbilityStartWithWaitObserver> abilityStartWithWaitObserver = nullptr;
    {
        std::lock_guard<std::mutex> guard(oberverMutex_);
        for (auto& observer : observerList_) {
            if (!observer.first) {
                TAG_LOGE(AAFwkTag::ABILITYMGR, "null observer");
                continue;
            }
            if (observerId == observer.second.observerId) {
                abilityStartWithWaitObserver = observer.first;
                // 1.remove deathRecipient for abilityStartWithWaitObserver
                if (observer.first->AsObject()) {
                    observer.first->AsObject()->RemoveDeathRecipient(observer.second.deathRecipient);
                }
                observerList_.erase(observer.first);
                TAG_LOGD(AAFwkTag::ABILITYMGR, "observer list size:%{public}zu", observerList_.size());
                break;
            }
        }
    }
    if (abilityStartWithWaitObserver) {
        AbilityStartWithWaitObserverData data;
        data.reason = static_cast<uint32_t>(reason);
        abilityStartWithWaitObserver->NotifyAATerminateWait(data);
    } else {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "abilityStartWithWaitObserver is nullptr");
    }
    want.RemoveParam(Want::START_ABILITY_WITH_WAIT_OBSERVER_ID_KEY);
}

void AbilityStartWithWaitObserverManager::NotifyAATerminateWait(
    std::shared_ptr<AbilityRecord> abilityRecord, TerminateReason reason)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "AbilityStartWithWaitObserverManager::NotifyAATerminateWait called");
    if (!abilityRecord) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null abilityRecord");
        return;
    }
    auto observerId = abilityRecord->GetIntParam(Want::START_ABILITY_WITH_WAIT_OBSERVER_ID_KEY, -1);
    if (observerId == -1) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "invaid observerId");
        return;
    }
    TAG_LOGD(AAFwkTag::ABILITYMGR, "observerId:%{public}d", observerId);
    abilityRecord->RemoveSpecifiedWantParam(Want::START_ABILITY_WITH_WAIT_OBSERVER_ID_KEY);
    sptr<IAbilityStartWithWaitObserver> proxy = nullptr;
    AbilityStartWithWaitObserverData data;
    {
        std::lock_guard<std::mutex> guard(oberverMutex_);
        for (auto& observer : observerList_) {
            if (!observer.first) {
                TAG_LOGE(AAFwkTag::ABILITYMGR, "null observer");
                continue;
            }
            TAG_LOGD(AAFwkTag::ABILITYMGR, "observerId = %{public}d", observer.second.observerId);
            if (observerId == observer.second.observerId) {
                data.reason = static_cast<uint32_t>(reason);
                data.coldStart = observer.second.coldStart;
                data.startTime = observer.second.startTime;
                data.foregroundTime = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::system_clock::now().time_since_epoch()).count();
                data.bundleName = abilityRecord->GetAbilityInfo().bundleName;
                data.abilityName = abilityRecord->GetAbilityInfo().name;
                TAG_LOGI(AAFwkTag::ABILITYMGR, "notify aa, time:%{public}" PRId64 " foregroundTime:%{public}" PRId64
                    " ,bundleName:%{public}s ability:%{public}s", data.startTime, data.foregroundTime,
                    data.bundleName.c_str(), data.abilityName.c_str());
                proxy = observer.first;
                // 1.remove deathRecipient for proxy
                if (observer.first->AsObject()) {
                    observer.first->AsObject()->RemoveDeathRecipient(observer.second.deathRecipient);
                }
                observerList_.erase(observer.first);
                TAG_LOGD(AAFwkTag::ABILITYMGR, "observer list size:%{public}zu", observerList_.size());
                break;
            }
        }
    }

    if (proxy) {
        proxy->NotifyAATerminateWait(data);
    } else {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "proxy not find");
    }
}

void AbilityStartWithWaitObserverManager::SetColdStartForShellCall(std::shared_ptr<AbilityRecord> abilityRecord)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "AbilityStartWithWaitObserverManager::SetColdStartForShellCall called");
    if (!abilityRecord) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null abilityRecord");
        return;
    }
    auto observerId = abilityRecord->GetIntParam(Want::START_ABILITY_WITH_WAIT_OBSERVER_ID_KEY, -1);
    if (observerId == -1) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "invaid observerId");
        return;
    }
    std::lock_guard<std::mutex> guard(oberverMutex_);
    for (auto& observer : observerList_) {
        if (!observer.first) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "null observer");
            continue;
        }
        if (observerId == observer.second.observerId) {
            TAG_LOGI(AAFwkTag::ABILITYMGR, "set cold start, bundleName:%{public}s ability:%{public}s",
                abilityRecord->GetAbilityInfo().bundleName.c_str(), abilityRecord->GetAbilityInfo().name.c_str());
            observer.second.coldStart = true;
        }
    }
}

sptr<AbilityStartWithWaitObserverRecipient> AbilityStartWithWaitObserverManager::GenerateDeathRecipient(
    sptr<IAbilityStartWithWaitObserver> observer)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "AbilityStartWithWaitObserverManager::GenerateDeathRecipient called");
    if (!observer || !observer->AsObject()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null observer");
        return nullptr;
    }
    auto deathRecipientFunc = [](const wptr<IRemoteObject> &remote) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "death recipient call");
        auto object = remote.promote();
        if (object == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "object null");
            return;
        }
        auto observer = iface_cast<IAbilityStartWithWaitObserver>(object);
        AbilityStartWithWaitObserverManager::GetInstance().UnregisterObserver(observer);
    };
    auto deathRecipient = sptr<AbilityStartWithWaitObserverRecipient>::MakeSptr(deathRecipientFunc);
    if (deathRecipient == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null deathRecipient");
        return nullptr;
    }
    if (!observer->AsObject()->AddDeathRecipient(deathRecipient)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "AddDeathRecipient fail");
    }
    return deathRecipient;
}
} // namespace AAFwk
} // namespace OHOS