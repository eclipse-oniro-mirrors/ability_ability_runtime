/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "ability_manager_event_subscriber.h"

#include "ability_util.h"
#include "common_event_support.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr int32_t INVALID_USERID = -1;
}
AbilityEventMapManager &AbilityEventMapManager::GetInstance()
{
    static AbilityEventMapManager instance;
    return instance;
}
AbilityEventMapManager::AbilityEventMapManager() {}
AbilityEventMapManager::~AbilityEventMapManager() {}

void AbilityEventMapManager::AddEvent(int32_t userId, const std::string &event)
{
    std::lock_guard<std::mutex> lock(mutex_);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "SU life, AddEvent userId: %{public}d, event: %{public}s.", userId, event.c_str());
    auto iter = eventMap_.find(userId);
    if (iter != eventMap_.end()) {
        if (event == EventFwk::CommonEventSupport::COMMON_EVENT_USER_UNLOCKED) {
            iter->second.userUnlock_ = true;
            return;
        }
        iter->second.screenUnlock_ = true;
        return;
    }
    if (event == EventFwk::CommonEventSupport::COMMON_EVENT_USER_UNLOCKED) {
        eventMap_[userId] = { true, false };
        return;
    }
    eventMap_[userId] = { false, true };
}

void AbilityEventMapManager::RemoveUser(int32_t userId)
{
    std::lock_guard<std::mutex> lock(mutex_);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "RemoveUser userId: %{public}d.", userId);
    eventMap_.erase(userId);
}

bool AbilityEventMapManager::CheckAllUnlocked(int32_t userId)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto iter = eventMap_.find(userId);
    if (iter != eventMap_.end()) {
        return iter->second.userUnlock_ && iter->second.screenUnlock_;
    }
    return false;
}

AbilityScreenUnlockEventSubscriber::AbilityScreenUnlockEventSubscriber(
    const EventFwk::CommonEventSubscribeInfo &subscribeInfo, const std::function<void(int32_t)> &screenUnlockCallback)
    : EventFwk::CommonEventSubscriber(subscribeInfo), screenUnlockCallback_(screenUnlockCallback)
{}

void AbilityScreenUnlockEventSubscriber::OnReceiveEvent(const EventFwk::CommonEventData &data)
{
    const AAFwk::Want &want = data.GetWant();
    std::string action = want.GetAction();
    TAG_LOGD(AAFwkTag::ABILITYMGR, "The action: %{public}s.", action.c_str());
    if (action != EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_UNLOCKED) {
        return;
    }
    if (screenUnlockCallback_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "screenUnlockCallback_ nullptr");
        return;
    }
    int32_t userId = want.GetIntParam("userId", INVALID_USERID);
    if (userId == INVALID_USERID) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "userId is invalid");
        return;
    }
    AbilityEventMapManager::GetInstance().AddEvent(userId, action);
    if (AbilityEventMapManager::GetInstance().CheckAllUnlocked(userId)) {
        screenUnlockCallback_(userId);
        AbilityEventMapManager::GetInstance().RemoveUser(userId);
    }
}

AbilityUserUnlockEventSubscriber::AbilityUserUnlockEventSubscriber(
    const EventFwk::CommonEventSubscribeInfo &subscribeInfo, const std::function<void(int32_t)> &screenUnlockCallback,
    const std::function<void()> &userScreenUnlockCallback)
    : EventFwk::CommonEventSubscriber(subscribeInfo), screenUnlockCallback_(screenUnlockCallback),
    userScreenUnlockCallback_(userScreenUnlockCallback)
{}

void AbilityUserUnlockEventSubscriber::OnReceiveEvent(const EventFwk::CommonEventData &data)
{
    const AAFwk::Want &want = data.GetWant();
    std::string action = want.GetAction();
    TAG_LOGD(AAFwkTag::ABILITYMGR, "The action: %{public}s.", action.c_str());
    if (action != EventFwk::CommonEventSupport::COMMON_EVENT_USER_UNLOCKED) {
        return;
    }
    if (userScreenUnlockCallback_ == nullptr || screenUnlockCallback_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "nullptr callback");
        return;
    }
    int32_t userId = data.GetCode();
    AbilityEventMapManager::GetInstance().AddEvent(userId, action);
    if (AbilityEventMapManager::GetInstance().CheckAllUnlocked(userId)) {
        screenUnlockCallback_(userId);
        AbilityEventMapManager::GetInstance().RemoveUser(userId);
        return;
    }
    userScreenUnlockCallback_();
}
} // namespace AbilityRuntime
} // namespace OHOS