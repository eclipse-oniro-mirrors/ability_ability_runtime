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

#ifndef OHOS_ABILITY_RUNTIME_CLI_SESSION_SUBSCRIPTION_MANAGER_H
#define OHOS_ABILITY_RUNTIME_CLI_SESSION_SUBSCRIPTION_MANAGER_H

#include <atomic>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "cli_tool_event.h"

namespace OHOS {
namespace CliTool {

using SessionToolEventCallback = std::function<void(const std::string &, const std::string &, const CliToolEvent &)>;

class CliSessionSubscriptionManager final {
public:
    static CliSessionSubscriptionManager &GetInstance();

    std::string AddProvisionalSubscription(const std::string &sessionId, const SessionToolEventCallback &callback);
    void ActivateSubscription(const std::string &subscriptionId);
    void RemoveSubscription(const std::string &subscriptionId);
    void ClearAllSubscriptions();
    int32_t HandleSessionEvent(const std::string &sessionId, const std::string &subscriptionId,
        const CliToolEvent &event);

private:
    struct SubscriptionRecord {
        std::string sessionId;
        SessionToolEventCallback callback;
        bool isActive = false;
        std::vector<CliToolEvent> pendingEvents;
    };

    CliSessionSubscriptionManager() = default;
    ~CliSessionSubscriptionManager() = default;

    std::string GenerateSubscriptionId(const std::string &sessionId);

private:
    std::mutex mutex_;
    std::unordered_map<std::string, SubscriptionRecord> subscriptions_;
    std::atomic<uint64_t> nextSubscriptionId_ {0};
};

} // namespace CliTool
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_CLI_SESSION_SUBSCRIPTION_MANAGER_H