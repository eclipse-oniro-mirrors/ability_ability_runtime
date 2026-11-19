/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_ABILITY_MANAGER_EVENT_SUBSCRIBER_H
#define OHOS_ABILITY_RUNTIME_ABILITY_MANAGER_EVENT_SUBSCRIBER_H

#include <mutex>
#include <set>

#include "common_event_data.h"
#include "common_event_subscribe_info.h"
#include "common_event_subscriber.h"
#include "nocopyable.h"

namespace OHOS {
namespace AbilityRuntime {
struct LockEventData {
    bool userUnlock_ = false;
    bool screenUnlock_ = false;
};

class AbilityEventMapManager {
public:
    static AbilityEventMapManager &GetInstance();
    void AddEvent(int32_t userId, const std::string &event);
    void RemoveUser(int32_t userId);
    bool CheckAllUnlocked(int32_t userId);
    bool CheckUserUnlocked(int32_t userId);
    size_t GetUserCount();
    void AddPendingUserId(int32_t userId);
    bool IsPendingUser(int32_t userId);
    void RemovePendingUserId(int32_t userId);

private:
    AbilityEventMapManager();
    ~AbilityEventMapManager();
    std::unordered_map<int32_t, LockEventData> eventMap_;
    std::set<int32_t> pendingUserIds_;
    std::mutex mutex_;

    DISALLOW_COPY_AND_MOVE(AbilityEventMapManager);
};

class AbilityScreenUnlockEventSubscriber : public EventFwk::CommonEventSubscriber {
public:
    explicit AbilityScreenUnlockEventSubscriber(
        const EventFwk::CommonEventSubscribeInfo &subscribeInfo,
        const std::function<void(int32_t)> &screenUnlockCallback);

    ~AbilityScreenUnlockEventSubscriber() override = default;

    void OnReceiveEvent(const EventFwk::CommonEventData &data) override;

private:
    std::function<void(int32_t)> screenUnlockCallback_;
};

class AbilityUserUnlockEventSubscriber : public EventFwk::CommonEventSubscriber {
public:
    explicit AbilityUserUnlockEventSubscriber(
        const EventFwk::CommonEventSubscribeInfo &subscribeInfo,
        const std::function<void(int32_t)> &screenUnlockCallback,
        const std::function<void(int32_t)> &userScreenUnlockCallback);

    ~AbilityUserUnlockEventSubscriber() override = default;

    void OnReceiveEvent(const EventFwk::CommonEventData &data) override;

private:
    std::function<void(int32_t)> screenUnlockCallback_;
    std::function<void(int32_t)> userScreenUnlockCallback_;
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_ABILITY_MANAGER_EVENT_SUBSCRIBER_H