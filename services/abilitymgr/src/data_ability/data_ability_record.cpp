/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "data_ability_record.h"

#include "ability_util.h"
#include "connection_state_manager.h"

namespace OHOS {
namespace AAFwk {
DataAbilityRecord::DataAbilityRecord(const AbilityRequest &req) : request_(req)
{
    TAG_LOGD(AAFwkTag::DATA_ABILITY, "called");

    if (request_.abilityInfo.type != AppExecFwk::AbilityType::DATA) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "wrong ability type");
    }
}

DataAbilityRecord::~DataAbilityRecord()
{
    TAG_LOGD(AAFwkTag::DATA_ABILITY, "called");
}

int DataAbilityRecord::StartLoading()
{
    TAG_LOGI(AAFwkTag::DATA_ABILITY, "called");

    if (ability_ || scheduler_) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "already started");
        return ERR_ALREADY_EXISTS;
    }

    if (request_.abilityInfo.type != AppExecFwk::AbilityType::DATA) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "wrong ability type");
        return ERR_INVALID_VALUE;
    }

    auto ability = AbilityRecord::CreateAbilityRecord(request_);
    if (!ability) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "allocate failed");
        return ERR_NO_MEMORY;
    }

    int ret = ability->LoadAbility();
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "loadAbility failed");
        return ret;
    }

    ability_ = ability;

    // Ability state is 'INITIAL' now.

    return ERR_OK;
}

int DataAbilityRecord::WaitForLoaded(ffrt::mutex &mutex, const std::chrono::system_clock::duration &timeout)
{
    CHECK_POINTER_AND_RETURN(ability_, ERR_INVALID_STATE);

    // Data ability uses 'ACTIVATE' as loaded state.
    if (ability_->GetAbilityState() == ACTIVE) {
        return ERR_OK;
    }

    std::unique_lock<ffrt::mutex> lock(mutex, std::adopt_lock);
    auto ret = loadedCond_.wait_for(lock, timeout, [this] { return ability_->GetAbilityState() == ACTIVE; });
    if (!ret) {
        return ERR_TIMED_OUT;
    }

    if (!scheduler_ || ability_->GetAbilityState() != ACTIVE) {
        return ERR_INVALID_STATE;
    }

    return ERR_OK;
}

sptr<IAbilityScheduler> DataAbilityRecord::GetScheduler()
{
    // Check if data ability is attached.
    CHECK_POINTER_AND_RETURN(ability_, nullptr);
    CHECK_POINTER_AND_RETURN(scheduler_, nullptr);

    // Check if data ability is loaded.
    if (ability_->GetAbilityState() != ACTIVE) {
        return nullptr;
    }

    return scheduler_;
}

int DataAbilityRecord::Attach(const sptr<IAbilityScheduler> &scheduler)
{
    TAG_LOGD(AAFwkTag::DATA_ABILITY, "called");

    if (!scheduler) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "invalid scheduler");
        return ERR_INVALID_DATA;
    }

    if (!ability_) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "not startloading");
        return ERR_INVALID_STATE;
    }

    if (scheduler_) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "already attached");
        return ERR_INVALID_STATE;
    }

    // INITIAL => ACTIVATING

    if (ability_->GetAbilityState() != INITIAL) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "not in 'INITIAL' state");
        return ERR_INVALID_STATE;
    }

    TAG_LOGD(AAFwkTag::DATA_ABILITY, "Attaching");
    ability_->SetScheduler(scheduler);
    scheduler_ = scheduler;

    TAG_LOGI(AAFwkTag::DATA_ABILITY, "Scheduling 'OnStart' for '%{public}s|%{public}s'",
        ability_->GetApplicationInfo().bundleName.c_str(),
        ability_->GetAbilityInfo().name.c_str());

    ability_->SetAbilityState(ACTIVATING);

    LifeCycleStateInfo state;
    state.state = AbilityLifeCycleState::ABILITY_STATE_ACTIVE;

    scheduler->ScheduleAbilityTransaction(ability_->GetWant(), state);

    return ERR_OK;
}

int DataAbilityRecord::OnTransitionDone(int state)
{
    CHECK_POINTER_AND_RETURN(ability_, ERR_INVALID_STATE);
    CHECK_POINTER_AND_RETURN(scheduler_, ERR_INVALID_STATE);

    if (ability_->GetAbilityState() != ACTIVATING) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "not in 'ACTIVATING' state");
        return ERR_INVALID_STATE;
    }

    if (state != AbilityLifeCycleState::ABILITY_STATE_ACTIVE) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "not ACTIVE");
        ability_->SetAbilityState(INITIAL);
        loadedCond_.notify_all();
        return ERR_INVALID_STATE;
    }

    // ACTIVATING => ACTIVE(loaded):
    // Set loaded state, data ability uses 'ACTIVE' as loaded state.

    ability_->SetAbilityState(ACTIVE);
    loadedCond_.notify_all();

    TAG_LOGI(AAFwkTag::DATA_ABILITY, "'%{public}s|%{public}s' loaded",
        ability_->GetApplicationInfo().bundleName.c_str(),
        ability_->GetAbilityInfo().name.c_str());

    return ERR_OK;
}

int DataAbilityRecord::AddClient(const sptr<IRemoteObject> &client, bool tryBind, bool isNotHap)
{
    if (!client) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "invalid param");
        return ERR_INVALID_STATE;
    }

    if (!ability_ || !scheduler_) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "not attached");
        return ERR_INVALID_STATE;
    }

    if (ability_->GetAbilityState() != ACTIVE) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "not loaded");
        return ERR_INVALID_STATE;
    }

    auto appScheduler = DelayedSingleton<AppScheduler>::GetInstance();
    if (!appScheduler) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "get appScheduler failed");
        return ERR_NULL_OBJECT;
    }

    TAG_LOGI(AAFwkTag::DATA_ABILITY, "add death monitoring");
    if (client != nullptr && callerDeathRecipient_ != nullptr) {
        client->RemoveDeathRecipient(callerDeathRecipient_);
    }
    if (callerDeathRecipient_ == nullptr) {
        std::weak_ptr<DataAbilityRecord> thisWeakPtr(weak_from_this());
        callerDeathRecipient_ = new DataAbilityCallerRecipient([thisWeakPtr](const wptr<IRemoteObject> &remote) {
            auto dataAbilityRecord = thisWeakPtr.lock();
            if (dataAbilityRecord) {
                dataAbilityRecord->OnSchedulerDied(remote);
            }
        });
    }
    if (client != nullptr) {
        client->AddDeathRecipient(callerDeathRecipient_);
    }

    // One client can be added multi-times, so 'RemoveClient()' must be called in corresponding times.
    auto &clientInfo = clients_.emplace_back();
    clientInfo.client = client;
    clientInfo.tryBind = tryBind;
    clientInfo.isNotHap = isNotHap;
    clientInfo.clientPid = IPCSkeleton::GetCallingPid();

    return ERR_OK;
}

int DataAbilityRecord::RemoveClient(const sptr<IRemoteObject> &client, bool isNotHap)
{
    TAG_LOGI(AAFwkTag::DATA_ABILITY, "called");

    if (!client) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "invalid client");
        return ERR_INVALID_STATE;
    }

    if (!ability_ || !scheduler_) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "not attached");
        return ERR_INVALID_STATE;
    }

    if (ability_->GetAbilityState() != ACTIVE) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "not loaded");
        return ERR_INVALID_STATE;
    }

    if (clients_.empty()) {
        TAG_LOGD(AAFwkTag::DATA_ABILITY, "no clients");
        return ERR_OK;
    }

    for (auto it(clients_.begin()); it != clients_.end(); ++it) {
        if (it->client == client) {
            clients_.erase(it);
            TAG_LOGI(AAFwkTag::DATA_ABILITY, "dataability '%{public}s|%{public}s'",
                ability_->GetApplicationInfo().bundleName.c_str(),
                ability_->GetAbilityInfo().name.c_str());
            break;
        }
    }

    return ERR_OK;
}

int DataAbilityRecord::RemoveClients(const std::shared_ptr<AbilityRecord> &client)
{
    TAG_LOGD(AAFwkTag::DATA_ABILITY, "called");

    if (!ability_ || !scheduler_) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "not attached");
        return ERR_INVALID_STATE;
    }

    if (ability_->GetAbilityState() != ACTIVE) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "not loaded");
        return ERR_INVALID_STATE;
    }

    if (clients_.empty()) {
        TAG_LOGD(AAFwkTag::DATA_ABILITY, "no clients");
        return ERR_OK;
    }

    auto appScheduler = DelayedSingleton<AppScheduler>::GetInstance();
    if (!appScheduler) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "invalid app scheduler");
        return ERR_NULL_OBJECT;
    }

    if (client) {
        TAG_LOGD(AAFwkTag::DATA_ABILITY, "Removing with filter");
        auto it = clients_.begin();
        while (it != clients_.end()) {
            if (!it->isNotHap) {
                auto clientAbilityRecord = Token::GetAbilityRecordByToken(it->client);
                if (!clientAbilityRecord) {
                    TAG_LOGE(AAFwkTag::DATA_ABILITY, "null clientAbilityRecord");
                    ++it;
                    continue;
                }
                if (clientAbilityRecord == client) {
                    it = clients_.erase(it);
                    TAG_LOGI(AAFwkTag::DATA_ABILITY,
                        "Ability '%{public}s|%{public}s' --X-> Data ability '%{public}s|%{public}s'",
                        client->GetApplicationInfo().bundleName.c_str(),
                        client->GetAbilityInfo().name.c_str(),
                        ability_->GetApplicationInfo().bundleName.c_str(),
                        ability_->GetAbilityInfo().name.c_str());
                } else {
                    ++it;
                }
            } else {
                ++it;
            }
        }
        return ERR_OK;
    }
    auto it = clients_.begin();
    while (it != clients_.end()) {
        if (!it->isNotHap) {
            auto clientAbilityRecord = Token::GetAbilityRecordByToken(it->client);
            if (!clientAbilityRecord) {
                TAG_LOGD(AAFwkTag::DATA_ABILITY, "null clientAbilityRecord");
                it = clients_.erase(it);
                continue;
            }
            it = clients_.erase(it);
        } else {
            ++it;
        }
    }

    return ERR_OK;
}

size_t DataAbilityRecord::GetClientCount(const sptr<IRemoteObject> &client) const
{
    CHECK_POINTER_AND_RETURN(ability_, 0);
    CHECK_POINTER_AND_RETURN(scheduler_, 0);

    if (ability_->GetAbilityState() != ACTIVE) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "not loaded");
        return 0;
    }

    if (client) {
        return std::count_if(
            clients_.begin(), clients_.end(), [client](const ClientInfo &ci) { return ci.client == client; });
    }

    return clients_.size();
}

int DataAbilityRecord::KillBoundClientProcesses()
{
    CHECK_POINTER_AND_RETURN(ability_, ERR_INVALID_STATE);
    CHECK_POINTER_AND_RETURN(scheduler_, ERR_INVALID_STATE);

    if (ability_->GetAbilityState() != ACTIVE) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "not loaded");
        return ERR_INVALID_STATE;
    }

    auto appScheduler = DelayedSingleton<AppScheduler>::GetInstance();
    if (!appScheduler) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "invalid app scheduler");
        return ERR_INVALID_STATE;
    }

    for (auto it = clients_.begin(); it != clients_.end(); ++it) {
        if (it->tryBind && !it->isNotHap) {
            auto clientAbilityRecord = Token::GetAbilityRecordByToken(it->client);
            CHECK_POINTER_CONTINUE(clientAbilityRecord);
            TAG_LOGI(AAFwkTag::DATA_ABILITY,
                "Killing bound client '%{public}s|%{public}s' of data ability '%{public}s|%{public}s'",
                clientAbilityRecord->GetApplicationInfo().bundleName.c_str(),
                clientAbilityRecord->GetAbilityInfo().name.c_str(),
                ability_->GetApplicationInfo().bundleName.c_str(),
                ability_->GetAbilityInfo().name.c_str());
            appScheduler->KillProcessByAbilityToken(clientAbilityRecord->GetToken());
        }
    }

    return ERR_OK;
}

const AbilityRequest &DataAbilityRecord::GetRequest() const
{
    return request_;
}

std::shared_ptr<AbilityRecord> DataAbilityRecord::GetAbilityRecord()
{
    return ability_;
}

sptr<IRemoteObject> DataAbilityRecord::GetToken()
{
    if (!ability_) {
        return nullptr;
    }

    return ability_->GetToken();
}

void DataAbilityRecord::Dump() const
{
    CHECK_POINTER(ability_);

    TAG_LOGI(AAFwkTag::DATA_ABILITY,
        "attached: %{public}s, clients: %{public}zu, refcnt: %{public}d, state: %{public}s",
        scheduler_ ? "true" : "false",
        clients_.size(),
        scheduler_ ? scheduler_->GetSptrRefCount() : 0,
        AbilityRecord::ConvertAbilityState(ability_->GetAbilityState()).c_str());

    int i = 0;

    for (auto it = clients_.begin(); it != clients_.end(); ++it) {
        if (!it->isNotHap) {
            auto clientAbilityRecord = Token::GetAbilityRecordByToken(it->client);
            CHECK_POINTER_CONTINUE(clientAbilityRecord);
            TAG_LOGI(AAFwkTag::DATA_ABILITY, "  %{public}2d '%{public}s|%{public}s' - tryBind: %{public}s",
                i++,
                clientAbilityRecord->GetApplicationInfo().bundleName.c_str(),
                clientAbilityRecord->GetAbilityInfo().name.c_str(),
                it->tryBind ? "true" : "false");
        } else {
            TAG_LOGI(AAFwkTag::DATA_ABILITY, "  %{public}2d '%{public}s' - tryBind: %{public}s",
                i++,
                "caller is system",
                it->tryBind ? "true" : "false");
        }
    }
}

void DataAbilityRecord::Dump(std::vector<std::string> &info) const
{
    CHECK_POINTER(ability_);
    info.emplace_back("    AbilityRecord ID #" + std::to_string(ability_->GetRecordId()) + "   state #" +
                      AbilityRecord::ConvertAbilityState(ability_->GetAbilityState()) + "   start time [" +
                      std::to_string(ability_->GetStartTime()) + "]");
    info.emplace_back("    main name [" + ability_->GetAbilityInfo().name + "]");
    info.emplace_back("    bundle name [" + ability_->GetAbilityInfo().bundleName + "]");
    info.emplace_back("    ability type [DATA]");
    info.emplace_back("    app state #" + AbilityRecord::ConvertAppState(ability_->GetAppState()));
    info.emplace_back("    Clients: " + std::to_string(clients_.size()));

    for (auto &&client : clients_) {
        if (!client.isNotHap) {
            auto clientAbilityRecord = Token::GetAbilityRecordByToken(client.client);
            CHECK_POINTER_CONTINUE(clientAbilityRecord);
            info.emplace_back("     > " + clientAbilityRecord->GetAbilityInfo().bundleName + "/" +
                              clientAbilityRecord->GetAbilityInfo().name + "  tryBind #" +
                              (client.tryBind ? "true" : "false") + "  isNotHap  # " +
                              (client.isNotHap ? "true" : "false"));
        } else {
            info.emplace_back(std::string("     > Caller is System /  tryBind # ") +
                              (client.tryBind ? "true" : "false") + "  isNotHap  # " +
                              (client.isNotHap ? "true" : "false"));
        }
    }
}

void DataAbilityRecord::OnSchedulerDied(const wptr<IRemoteObject> &remote)
{
    TAG_LOGI(AAFwkTag::DATA_ABILITY, "called");
    auto object = remote.promote();
    DelayedSingleton<ConnectionStateManager>::GetInstance()->HandleDataAbilityCallerDied(GetDiedCallerPid(object));

    if (clients_.empty()) {
        TAG_LOGD(AAFwkTag::DATA_ABILITY, "no clients");
        return;
    }

    auto appScheduler = DelayedSingleton<AppScheduler>::GetInstance();
    if (!appScheduler) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "invalid app scheduler");
        return;
    }

    if (object) {
        auto it = clients_.begin();
        while (it != clients_.end()) {
            if (it->client == object) {
                TAG_LOGD(AAFwkTag::DATA_ABILITY, "remove system caller");
                it = clients_.erase(it);
                TAG_LOGI(AAFwkTag::DATA_ABILITY, "dataability '%{public}s|%{public}s'",
                    ability_->GetApplicationInfo().bundleName.c_str(),
                    ability_->GetAbilityInfo().name.c_str());
            } else {
                ++it;
            }
        }
        return;
    }
    auto it = clients_.begin();
    while (it != clients_.end()) {
        if (it->isNotHap) {
            it = clients_.erase(it);
            TAG_LOGI(AAFwkTag::DATA_ABILITY, "dataability '%{public}s|%{public}s'",
                ability_->GetApplicationInfo().bundleName.c_str(),
                ability_->GetAbilityInfo().name.c_str());
        } else {
            ++it;
        }
    }
}

int32_t DataAbilityRecord::GetDiedCallerPid(const sptr<IRemoteObject> &remote)
{
    if (!remote) {
        return 0;
    }

    int32_t result = 0;
    for (auto it = clients_.begin(); it != clients_.end(); it++) {
        if (it->client == remote) {
            result = it->clientPid;
            break;
        }
    }

    return result;
}
}  // namespace AAFwk
}  // namespace OHOS
