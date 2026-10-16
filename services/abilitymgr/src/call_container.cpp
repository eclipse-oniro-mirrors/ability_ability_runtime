/*
 * Copyright (c) 2022-2026 Huawei Device Co., Ltd.
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

#include "call_container.h"

#include "ability_connect_callback_stub.h"
#include "ability_util.h"
#include "ability_manager_service.h"

namespace OHOS {
namespace AAFwk {
constexpr int32_t REASON_CALLEE_TERMINATE = 1;
constexpr int32_t REASON_PRELAUNCH_TIMEOUT = 2;
CallContainer::CallContainer()
{}

CallContainer::~CallContainer()
{
    std::for_each(deathRecipientMap_.begin(),
        deathRecipientMap_.end(),
        [&](RecipientMapType::reference recipient) {
            recipient.first->RemoveDeathRecipient(recipient.second);
        });

    deathRecipientMap_.clear();
    callRecordMap_.clear();
}

std::shared_ptr<CallRecord> CallContainer::FindCallRecordMap(sptr<IRemoteObject> object) const
{
    std::lock_guard lock(callRecordMapLock_);
    auto record = callRecordMap_.find(object);
    if (record != callRecordMap_.end()) {
        return record->second;
    }
    return nullptr;
}

std::shared_ptr<CallRecord> CallContainer::RemoveCallRecordMap(sptr<IRemoteObject> object)
{
    std::shared_ptr<CallRecord> result;
    std::lock_guard lock(callRecordMapLock_);
    auto record = callRecordMap_.find(object);
    if (record != callRecordMap_.end()) {
        result = record->second;
        callRecordMap_.erase(record);
    }
    return result;
}

void CallContainer::InsertCallRecordMap(sptr<IRemoteObject> object, std::shared_ptr<CallRecord> callRecord)
{
    std::lock_guard lock(callRecordMapLock_);
    callRecordMap_.emplace(object, callRecord);
}

CallContainer::CallMapType CallContainer::CopyCallRecordMap() const
{
    std::lock_guard lock(callRecordMapLock_);
    return callRecordMap_;
}

bool CallContainer::EmptyCallRecordMap()
{
    std::lock_guard lock(callRecordMapLock_);
    return callRecordMap_.empty();
}

void CallContainer::AddCallRecord(const sptr<IAbilityConnection> & connect,
    const std::shared_ptr<CallRecord>& callRecord)
{
    CHECK_POINTER(callRecord);
    CHECK_POINTER(connect);
    CHECK_POINTER(connect->AsObject());

    auto record = RemoveCallRecordMap(connect->AsObject());
    if (record != nullptr) {
        RemoveConnectDeathRecipient(connect);
    }

    AddConnectDeathRecipient(connect);
    callRecord->SetConCallBack(connect);
    InsertCallRecordMap(connect->AsObject(), callRecord);

    TAG_LOGD(AAFwkTag::ABILITYMGR, "Add call record to callcontainer, target: %{public}s",
        callRecord->GetTargetServiceName().GetURI().c_str());
}

std::shared_ptr<CallRecord> CallContainer::GetCallRecord(const sptr<IAbilityConnection> & connect) const
{
    CHECK_POINTER_AND_RETURN(connect, nullptr);
    CHECK_POINTER_AND_RETURN(connect->AsObject(), nullptr);

    auto record = FindCallRecordMap(connect->AsObject());
    return record;
}

bool CallContainer::RemoveCallRecord(const sptr<IAbilityConnection> & connect)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call container release call record by callback.");
    CHECK_POINTER_AND_RETURN(connect, false);
    CHECK_POINTER_AND_RETURN(connect->AsObject(), false);

    auto record = RemoveCallRecordMap(connect->AsObject());
    if (record != nullptr) {
        record->SchedulerDisconnectDone();
        RemoveConnectDeathRecipient(connect);
        TAG_LOGD(AAFwkTag::ABILITYMGR, "remove call record is success.");
        return true;
    }

    if (EmptyCallRecordMap()) {
        // notify soft resouce service.
        TAG_LOGD(AAFwkTag::ABILITYMGR, "this ability has no callrecord.");
    }

    TAG_LOGW(AAFwkTag::ABILITYMGR, "remove call record not exist");
    return false;
}

void CallContainer::OnConnectionDied(const wptr<IRemoteObject> &remote)
{
    TAG_LOGW(AAFwkTag::ABILITYMGR, "call back died");
    auto object = remote.promote();
    CHECK_POINTER(object);

    std::shared_ptr<CallRecord> callRecord = FindCallRecordMap(object);
    auto abilityManagerService = DelayedSingleton<AbilityManagerService>::GetInstance();
    CHECK_POINTER(abilityManagerService);
    auto handler = abilityManagerService->GetTaskHandler();
    CHECK_POINTER(handler);
    auto task = [abilityManagerService, callRecord]() {
        abilityManagerService->OnCallConnectDied(callRecord);
    };
    handler->SubmitTask(task);
}

bool CallContainer::CallRequestDone(const sptr<IRemoteObject> &callStub)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "start");
    CHECK_POINTER_AND_RETURN(callStub, false);
    CallMapType copyCallRecordMap = CopyCallRecordMap();
    for (const auto &iter : copyCallRecordMap) {
        std::shared_ptr<CallRecord> callRecord = iter.second;
        if (callRecord && callRecord->IsCallState(CallState::REQUESTING)) {
            callRecord->SetCallStub(callStub);
            callRecord->SchedulerConnectDone();
        }
    }
    TAG_LOGD(AAFwkTag::ABILITYMGR, "end");
    return true;
}

void CallContainer::Dump(std::vector<std::string> &info) const
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "dump call records");
    CallMapType copyCallRecordMap = CopyCallRecordMap();
    for (const auto &iter : copyCallRecordMap) {
        auto callRecord = iter.second;
        if (callRecord) {
            callRecord->Dump(info);
        }
    }
}

bool CallContainer::IsNeedToCallRequest() const
{
    CallMapType copyCallRecordMap = CopyCallRecordMap();
    for (const auto &iter : copyCallRecordMap) {
        auto callRecord = iter.second;
        if (callRecord && !callRecord->IsCallState(CallState::REQUESTED)) {
            return true;
        }
    }
    return false;
}

void CallContainer::AddConnectDeathRecipient(const sptr<IAbilityConnection> &connect)
{
    CHECK_POINTER(connect);
    CHECK_POINTER(connect->AsObject());
    auto it = deathRecipientMap_.find(connect->AsObject());
    if (it != deathRecipientMap_.end()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "death recipient been added");
        return;
    } else {
        std::weak_ptr<CallContainer> thisWeakPtr(shared_from_this());
        sptr<IRemoteObject::DeathRecipient> deathRecipient =
            new AbilityConnectCallbackRecipient([thisWeakPtr](const wptr<IRemoteObject> &remote) {
                auto callContainer = thisWeakPtr.lock();
                if (callContainer) {
                    callContainer->OnConnectionDied(remote);
                }
            });
        if (!connect->AsObject()->AddDeathRecipient(deathRecipient)) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "addDeathRecipient failed");
        }
        deathRecipientMap_.emplace(connect->AsObject(), deathRecipient);
    }
}

void CallContainer::RemoveConnectDeathRecipient(const sptr<IAbilityConnection> &connect)
{
    CHECK_POINTER(connect);
    CHECK_POINTER(connect->AsObject());
    auto it = deathRecipientMap_.find(connect->AsObject());
    if (it != deathRecipientMap_.end()) {
        it->first->RemoveDeathRecipient(it->second);
        deathRecipientMap_.erase(it);
        return;
    }
}

bool CallContainer::IsExistConnection(const sptr<IAbilityConnection> &connect)
{
    return FindCallRecordMap(connect->AsObject()) != nullptr;
}

void CallContainer::NotifyAllCallDisconnect(const AppExecFwk::ElementName &element)
{
    std::lock_guard lock(callRecordMapLock_);
    if (callRecordMap_.empty()) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "No call records to notify");
        return;
    }

    for (const auto &[_, callRecord] : callRecordMap_) {
        if (callRecord == nullptr || !callRecord->IsCallState(CallState::REQUESTED)) {
            continue;
        }
        auto callback = callRecord->GetConCallBack();
        if (callback) {
            // Notify caller that callee is disconnecting
            ffrt::submit([callback, element]() {
                callback->OnAbilityDisconnectDone(element, REASON_CALLEE_TERMINATE);
            });
        }
    }

    // Clear all call records after notification
    callRecordMap_.clear();
}

void CallContainer::NotifyDisconnectOnTimeout(const AppExecFwk::ElementName &element)
{
    std::lock_guard lock(callRecordMapLock_);
    if (callRecordMap_.empty()) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "No call records to notify");
        return;
    }

    for (auto it = callRecordMap_.begin(); it != callRecordMap_.end();) {
        auto &callRecord = it->second;
        if (callRecord == nullptr || !callRecord->IsPrelaunch()) {
            ++it;
            continue;
        }
        TAG_LOGI(AAFwkTag::ABILITYMGR, "Notify prelaunch timeout disconnect, recordId:%{public}d, reason:%{public}d",
            callRecord->GetCallRecordId(), REASON_PRELAUNCH_TIMEOUT);
        auto callback = callRecord->GetConCallBack();
        if (callback) {
            ffrt::submit([callback, element]() {
                callback->OnAbilityDisconnectDone(element, REASON_PRELAUNCH_TIMEOUT);
            });
        }
        it = callRecordMap_.erase(it);
    }
}
}  // namespace AAFwk
}  // namesspace OHOS
