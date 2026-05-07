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

#include "event_dispatcher.h"

#include <chrono>

#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace CliTool {
namespace {

constexpr int32_t ERR_OK = 0;

int64_t GetCurrentTimeMs()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
}

class CallbackDeathRecipient final : public IRemoteObject::DeathRecipient {
public:
    using Handler = std::function<void(const wptr<IRemoteObject> &)>;

    explicit CallbackDeathRecipient(Handler handler) : handler_(std::move(handler))
    {}

    ~CallbackDeathRecipient() override = default;

    void OnRemoteDied(const wptr<IRemoteObject> &remote) override
    {
        if (handler_) {
            handler_(remote);
        }
    }

private:
    Handler handler_;
};
} // namespace

EventDispatcher &EventDispatcher::GetInstance()
{
    static EventDispatcher instance;
    return instance;
}

bool EventDispatcher::RegisterScheduler(int32_t callerPid, const sptr<ICliToolManagerScheduler> &scheduler)
{
    if (callerPid <= 0 || scheduler == nullptr || scheduler->AsObject() == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL,
            "RegisterScheduler failed: invalid callerPid=%{public}d or scheduler remote is null", callerPid);
        return false;
    }
    auto remote = scheduler->AsObject();
    auto deathRecipient = sptr<IRemoteObject::DeathRecipient>(
        new (std::nothrow) CallbackDeathRecipient([callerPid](const wptr<IRemoteObject> &) {
            EventDispatcher::GetInstance().UnregisterScheduler(callerPid);
        }));
    if (deathRecipient == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "RegisterScheduler failed: alloc death recipient for pid %{public}d",
            callerPid);
        return false;
    }
    if (!remote->AddDeathRecipient(deathRecipient)) {
        TAG_LOGW(AAFwkTag::CLI_TOOL, "AddDeathRecipient failed for scheduler pid %{public}d", callerPid);
    }

    sptr<IRemoteObject> oldRemote;
    sptr<IRemoteObject::DeathRecipient> oldDeathRecipient;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto schedulerIt = schedulers_.find(callerPid);
        if (schedulerIt != schedulers_.end()) {
            oldRemote = schedulerIt->second.remote;
            oldDeathRecipient = schedulerIt->second.deathRecipient;
            RemoveSubscribersForPidLocked(callerPid);
        }
        schedulers_[callerPid] = SchedulerState {scheduler, remote, deathRecipient};
    }

    if (oldRemote != nullptr && oldDeathRecipient != nullptr) {
        oldRemote->RemoveDeathRecipient(oldDeathRecipient);
    }
    return true;
}

void EventDispatcher::UnregisterScheduler(int32_t callerPid)
{
    sptr<IRemoteObject> remote;
    sptr<IRemoteObject::DeathRecipient> deathRecipient;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = schedulers_.find(callerPid);
        if (it != schedulers_.end()) {
            remote = it->second.remote;
            deathRecipient = it->second.deathRecipient;
            schedulers_.erase(it);
        }
        RemoveSubscribersForPidLocked(callerPid);
    }
    if (remote != nullptr && deathRecipient != nullptr) {
        remote->RemoveDeathRecipient(deathRecipient);
    }
}

bool EventDispatcher::RegisterSubscriber(const std::string &sessionId,
    const std::string &subscriptionId, int32_t callerPid)
{
    if (sessionId.empty() || subscriptionId.empty() || callerPid <= 0) {
        TAG_LOGE(AAFwkTag::CLI_TOOL,
            "RegisterSubscriber failed: invalid args sessionId=%{public}s, subscriptionId=%{public}s, "
            "callerPid=%{public}d", sessionId.c_str(), subscriptionId.c_str(), callerPid);
        return false;
    }

    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto schedulerIt = schedulers_.find(callerPid);
        if (schedulerIt == schedulers_.end() || schedulerIt->second.scheduler == nullptr) {
            TAG_LOGE(AAFwkTag::CLI_TOOL,
                "RegisterSubscriber failed: scheduler not found for pid %{public}d, sessionId=%{public}s, "
                "subscriptionId=%{public}s", callerPid, sessionId.c_str(), subscriptionId.c_str());
            return false;
        }
        auto &subscribers = sessionSubscribers_[sessionId];
        SubscriberKey key {callerPid, subscriptionId};
        subscribers[key] = SubscriberState {key, schedulerIt->second.scheduler};
    }
    return true;
}

bool EventDispatcher::UnregisterSubscriber(const std::string &sessionId,
    const std::string &subscriptionId, int32_t callerPid)
{
    if (sessionId.empty() || subscriptionId.empty() || callerPid <= 0) {
        TAG_LOGE(AAFwkTag::CLI_TOOL,
            "UnregisterSubscriber failed: invalid args sessionId=%{public}s, subscriptionId=%{public}s, "
            "callerPid=%{public}d", sessionId.c_str(), subscriptionId.c_str(), callerPid);
        return false;
    }

    std::lock_guard<std::mutex> lock(mutex_);
    auto sessionIt = sessionSubscribers_.find(sessionId);
    if (sessionIt == sessionSubscribers_.end()) {
        TAG_LOGW(AAFwkTag::CLI_TOOL,
            "UnregisterSubscriber skipped: no subscribers for sessionId=%{public}s", sessionId.c_str());
        return true;
    }

    sessionIt->second.erase(SubscriberKey {callerPid, subscriptionId});
    if (sessionIt->second.empty()) {
        sessionSubscribers_.erase(sessionIt);
    }
    return true;
}

void EventDispatcher::DispatchIOEvent(const std::string &sessionId,
    const std::string &eventType, const std::string &data)
{
    CliToolEvent event;
    event.type = eventType;
    event.eventData = data;
    event.exitCode = 0;
    event.timestamp = GetCurrentTimeMs();
    DispatchEvent(sessionId, event);
}

void EventDispatcher::DispatchErrorEvent(const std::string &sessionId, const std::string &error)
{
    DispatchIOEvent(sessionId, "error", error);
}

void EventDispatcher::DispatchExitEvent(const std::string &sessionId, int32_t exitCode)
{
    CliToolEvent event;
    event.type = "exit";
    event.eventData = "";
    event.exitCode = exitCode;
    event.timestamp = GetCurrentTimeMs();
    DispatchEvent(sessionId, event);
}

bool EventDispatcher::DispatchInputReplyEvent(int32_t callerPid, const std::string &eventId, int32_t result)
{
    sptr<ICliToolManagerScheduler> scheduler;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto schedulerIt = schedulers_.find(callerPid);
        if (schedulerIt == schedulers_.end() || schedulerIt->second.scheduler == nullptr) {
            TAG_LOGE(AAFwkTag::CLI_TOOL,
                "DispatchInputReplyEvent failed: scheduler not found for pid %{public}d, eventId=%{public}s",
                callerPid, eventId.c_str());
            return false;
        }
        scheduler = schedulerIt->second.scheduler;
    }

    return (scheduler->SchedulerInputReplyEvent(eventId, result) == ERR_OK);
}

bool EventDispatcher::DispatchExecToolReplyEvent(int32_t callerPid, const std::string &eventId,
    int32_t result, const CliSessionInfo &session)
{
    sptr<ICliToolManagerScheduler> scheduler;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto schedulerIt = schedulers_.find(callerPid);
        if (schedulerIt == schedulers_.end() || schedulerIt->second.scheduler == nullptr) {
            TAG_LOGE(AAFwkTag::CLI_TOOL,
                "DispatchExecToolReplyEvent failed: scheduler not found for pid %{public}d, eventId=%{public}s",
                callerPid, eventId.c_str());
            return false;
        }
        scheduler = schedulerIt->second.scheduler;
    }

    return (scheduler->SchedulerExecToolReplyEvent(eventId, result, session) == ERR_OK);
}

void EventDispatcher::ClearSessionSubscribers(const std::string &sessionId)
{
    std::lock_guard<std::mutex> lock(mutex_);
    sessionSubscribers_.erase(sessionId);
}

void EventDispatcher::ClearAll()
{
    std::vector<std::pair<sptr<IRemoteObject>, sptr<IRemoteObject::DeathRecipient>>> cleanupList;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        for (auto &[pid, schedulerState] : schedulers_) {
            cleanupList.emplace_back(schedulerState.remote, schedulerState.deathRecipient);
        }
        schedulers_.clear();
        sessionSubscribers_.clear();
    }
    for (const auto &[remote, deathRecipient] : cleanupList) {
        if (remote != nullptr && deathRecipient != nullptr) {
            remote->RemoveDeathRecipient(deathRecipient);
        }
    }
}

void EventDispatcher::DispatchEvent(const std::string &sessionId, const CliToolEvent &event)
{
    std::vector<SubscriberState> subscribers;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto sessionIt = sessionSubscribers_.find(sessionId);
        if (sessionIt == sessionSubscribers_.end()) {
            TAG_LOGW(AAFwkTag::CLI_TOOL,
                "DispatchEvent skipped: no subscribers for sessionId=%{public}s, eventType=%{public}s",
                sessionId.c_str(), event.type.c_str());
            return;
        }
        for (const auto &[key, state] : sessionIt->second) {
            subscribers.push_back(state);
        }
    }

    std::vector<SubscriberKey> failedSubscribers;
    for (const auto &subscriber : subscribers) {
        if (subscriber.scheduler == nullptr ||
            subscriber.scheduler->SchedulerSessionEvent(sessionId, subscriber.key.subscriptionId, event) != ERR_OK) {
            TAG_LOGW(AAFwkTag::CLI_TOOL,
                "DispatchEvent failed for pid %{public}d, sessionId='%{public}s', subscriptionId='%{public}s', "
                "eventType='%{public}s'", subscriber.key.callerPid, sessionId.c_str(),
                subscriber.key.subscriptionId.c_str(), event.type.c_str());
            failedSubscribers.push_back(subscriber.key);
        }
    }

    if (!failedSubscribers.empty()) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto sessionIt = sessionSubscribers_.find(sessionId);
        if (sessionIt != sessionSubscribers_.end()) {
            for (const auto &key : failedSubscribers) {
                sessionIt->second.erase(key);
            }
            if (sessionIt->second.empty()) {
                sessionSubscribers_.erase(sessionIt);
            }
        }
    }
}

void EventDispatcher::RemoveSubscribersForPidLocked(int32_t callerPid)
{
    for (auto sessionIt = sessionSubscribers_.begin(); sessionIt != sessionSubscribers_.end();) {
        auto &subscribers = sessionIt->second;
        for (auto it = subscribers.begin(); it != subscribers.end();) {
            if (it->first.callerPid == callerPid) {
                it = subscribers.erase(it);
            } else {
                ++it;
            }
        }
        if (subscribers.empty()) {
            sessionIt = sessionSubscribers_.erase(sessionIt);
        } else {
            ++sessionIt;
        }
    }
}

} // namespace CliTool
} // namespace OHOS