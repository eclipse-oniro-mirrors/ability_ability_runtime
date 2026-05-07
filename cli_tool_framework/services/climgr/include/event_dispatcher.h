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

#include <memory>
#include <mutex>
#include <sys/types.h>
#include <unordered_map>
#include <utility>

#include "icli_tool_manager_scheduler.h"
#include "iremote_object.h"

namespace OHOS {
namespace CliTool {

class EventDispatcher final {
public:
    struct SubscriberKey {
        int32_t callerPid = 0;
        std::string subscriptionId;

        bool operator==(const SubscriberKey &other) const
        {
            return callerPid == other.callerPid && subscriptionId == other.subscriptionId;
        }
    };

    struct SubscriberKeyHash {
        size_t operator()(const SubscriberKey &key) const
        {
            return std::hash<int32_t>()(key.callerPid) ^ (std::hash<std::string>()(key.subscriptionId) << 1);
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

    bool RegisterScheduler(int32_t callerPid, const sptr<ICliToolManagerScheduler> &remote);

    void UnregisterScheduler(int32_t callerPid);

    bool RegisterSubscriber(const std::string &sessionId, const std::string &subscriptionId, int32_t callerPid);

    bool UnregisterSubscriber(const std::string &sessionId, const std::string &subscriptionId, int32_t callerPid);

    void DispatchIOEvent(const std::string &sessionId, const std::string &eventType, const std::string &data);
    void DispatchErrorEvent(const std::string &sessionId, const std::string &error);
    void DispatchExitEvent(const std::string &sessionId, int32_t exitCode);

    bool DispatchInputReplyEvent(int32_t callerPid, const std::string &eventId, int32_t result);

    bool DispatchExecToolReplyEvent(int32_t callerPid, const std::string &eventId,
        int32_t result, const CliSessionInfo &session);

    void ClearSessionSubscribers(const std::string &sessionId);

    void ClearAll();

private:
    void DispatchEvent(const std::string &sessionId, const CliToolEvent &event);

    void RemoveSubscribersForPidLocked(int32_t callerPid);

private:
    std::unordered_map<int32_t, SchedulerState> schedulers_;
    std::unordered_map<std::string, std::unordered_map<SubscriberKey, SubscriberState, SubscriberKeyHash>>
        sessionSubscribers_;
    std::mutex mutex_;
};

} // namespace CliTool
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_CLI_TOOL_MGR_EVENT_DISPATCHER_H
