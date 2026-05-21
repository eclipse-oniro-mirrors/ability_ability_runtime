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

#ifndef OHOS_ABILITY_RUNTIME_CLI_TOOL_MGR_EVENT_DISPATCHER_H
#define OHOS_ABILITY_RUNTIME_CLI_TOOL_MGR_EVENT_DISPATCHER_H

#include <cstddef>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <sys/types.h>
#include <unordered_map>
#include <utility>

#include "icli_tool_manager_scheduler.h"
#include "iremote_object.h"

namespace OHOS {
namespace CliTool {

class EventDispatcher final {
public:
    static constexpr size_t CALLER_UID_HASH_SHIFT = 1;
    static constexpr size_t SUBSCRIPTION_ID_HASH_SHIFT = 2;

    struct SchedulerKey {
        int32_t callerPid = 0;
        int32_t callerUid = 0;

        bool operator==(const SchedulerKey &other) const
        {
            return callerPid == other.callerPid && callerUid == other.callerUid;
        }
    };

    struct SchedulerKeyHash {
        size_t operator()(const SchedulerKey &key) const
        {
            return std::hash<int32_t>()(key.callerPid) ^
                (std::hash<int32_t>()(key.callerUid) << CALLER_UID_HASH_SHIFT);
        }
    };

    struct SubscriberKey {
        int32_t callerPid = 0;
        int32_t callerUid = 0;
        std::string subscriptionId;

        bool operator==(const SubscriberKey &other) const
        {
            return callerPid == other.callerPid && callerUid == other.callerUid &&
                subscriptionId == other.subscriptionId;
        }
    };

    struct SubscriberKeyHash {
        size_t operator()(const SubscriberKey &key) const
        {
            return std::hash<int32_t>()(key.callerPid) ^
                (std::hash<int32_t>()(key.callerUid) << CALLER_UID_HASH_SHIFT) ^
                (std::hash<std::string>()(key.subscriptionId) << SUBSCRIPTION_ID_HASH_SHIFT);
        }
    };

    struct SchedulerState {
        sptr<ICliToolManagerScheduler> scheduler;
        sptr<IRemoteObject> remote;
        sptr<IRemoteObject::DeathRecipient> deathRecipient;
    };

    struct SubscriberState {
        SubscriberKey key;
        sptr<ICliToolManagerScheduler> scheduler;
    };

    static EventDispatcher &GetInstance();

    bool SetScheduler(int32_t callerPid, int32_t callerUid, const sptr<ICliToolManagerScheduler> &remote);

    void ClearScheduler(int32_t callerPid, int32_t callerUid);

    bool RegisterSubscriber(const std::string &sessionId, const std::string &subscriptionId,
        int32_t callerPid, int32_t callerUid);

    bool UnregisterSubscriber(const std::string &sessionId, const std::string &subscriptionId,
        int32_t callerPid, int32_t callerUid);

    void DispatchIOEvent(const std::string &sessionId, const std::string &eventType, const std::string &data);
    void DispatchErrorEvent(const std::string &sessionId, const std::string &error);
    void DispatchExitEvent(const std::string &sessionId, int32_t exitCode);

    bool DispatchInputReplyEvent(int32_t callerPid, int32_t callerUid, const std::string &eventId, int32_t result);

    bool DispatchExecToolReplyEvent(int32_t callerPid, int32_t callerUid, const std::string &eventId,
        int32_t result, const CliSessionInfo &session);

    void ClearSessionSubscribers(const std::string &sessionId);

    void ClearAll();

private:
    void DispatchEvent(const std::string &sessionId, const CliToolEvent &event);

    void RemoveSubscribersForCallerLocked(const SchedulerKey &caller);
    bool HasSameScheduler(const SchedulerKey &caller, const sptr<IRemoteObject> &remote);
    sptr<IRemoteObject::DeathRecipient> CreateDeathRecipient(int32_t callerPid, int32_t callerUid);
    bool SaveScheduler(const SchedulerKey &caller, const sptr<ICliToolManagerScheduler> &scheduler,
        const sptr<IRemoteObject> &remote, const sptr<IRemoteObject::DeathRecipient> &deathRecipient,
        sptr<IRemoteObject> &oldRemote, sptr<IRemoteObject::DeathRecipient> &oldDeathRecipient);

private:
    std::unordered_map<SchedulerKey, SchedulerState, SchedulerKeyHash> schedulers_;
    std::unordered_map<std::string, std::unordered_map<SubscriberKey, SubscriberState, SubscriberKeyHash>>
        sessionSubscribers_;
    std::mutex mutex_;
};

} // namespace CliTool
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_CLI_TOOL_MGR_EVENT_DISPATCHER_H
