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

#include "cli_session_subscription_manager.h"

#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace CliTool {

namespace {
constexpr int32_t ERROR_CODE = -1;
constexpr int32_t ERR_OK = 0;
} // namespace

CliSessionSubscriptionManager &CliSessionSubscriptionManager::GetInstance()
{
    static CliSessionSubscriptionManager instance;
    return instance;
}

std::string CliSessionSubscriptionManager::GenerateSubscriptionId(const std::string &sessionId)
{
    uint64_t suffix = nextSubscriptionId_.fetch_add(1, std::memory_order_relaxed) + 1;
    return sessionId + "-" + std::to_string(suffix);
}

std::string CliSessionSubscriptionManager::AddProvisionalSubscription(const std::string &sessionId,
    const SessionToolEventCallback &callback)
{
    if (callback == nullptr || sessionId.empty()) {
        return "";
    }

    std::lock_guard<std::mutex> lock(mutex_);
    std::string subscriptionId = GenerateSubscriptionId(sessionId);
    subscriptions_[subscriptionId] = SubscriptionRecord {sessionId, std::move(callback), false};
    return subscriptionId;
}

void CliSessionSubscriptionManager::ActivateSubscription(const std::string &subscriptionId)
{
    SessionToolEventCallback callback;
    std::vector<CliToolEvent> pendingEvents;
    std::string sessionId;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = subscriptions_.find(subscriptionId);
        if (it == subscriptions_.end()) {
            return;
        }
        it->second.isActive = true;
        callback = it->second.callback;
        sessionId = it->second.sessionId;
        pendingEvents.swap(it->second.pendingEvents);
    }

    if (callback == nullptr) {
        RemoveSubscription(subscriptionId);
        return;
    }

    bool sawExit = false;
    for (const auto &event : pendingEvents) {
        callback(sessionId, subscriptionId, event);
        if (event.type == "exit") {
            sawExit = true;
        }
    }

    if (sawExit) {
        RemoveSubscription(subscriptionId);
    }
}

void CliSessionSubscriptionManager::RemoveSubscription(const std::string &subscriptionId)
{
    std::lock_guard<std::mutex> lock(mutex_);
    subscriptions_.erase(subscriptionId);
}

void CliSessionSubscriptionManager::ClearAllSubscriptions()
{
    std::lock_guard<std::mutex> lock(mutex_);
    subscriptions_.clear();
}

int32_t CliSessionSubscriptionManager::HandleSessionEvent(const std::string &sessionId,
    const std::string &subscriptionId, const CliToolEvent &event)
{
    SessionToolEventCallback callback;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = subscriptions_.find(subscriptionId);
        if (it == subscriptions_.end() || it->second.sessionId != sessionId) {
            return ERROR_CODE;
        }
        if (!it->second.isActive) {
            it->second.pendingEvents.push_back(event);
            return ERR_OK;
        }
        callback = it->second.callback;
        if (event.type == "exit") {
            subscriptions_.erase(it);
        }
    }

    if (callback == nullptr) {
        return ERROR_CODE;
    }
    callback(sessionId, subscriptionId, event);
    return ERR_OK;
}

} // namespace CliTool
} // namespace OHOS