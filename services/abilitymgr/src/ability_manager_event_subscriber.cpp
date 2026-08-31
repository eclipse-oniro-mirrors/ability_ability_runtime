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

#include "ability_manager_event_subscriber.h"

#include "ability_util.h"
#include "common_event_support.h"
#include "hilog_tag_wrapper.h"
#include "user_controller/user_controller.h"

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

bool AbilityEventMapManager::CheckUserUnlocked(int32_t userId)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto iter = eventMap_.find(userId);
    if (iter != eventMap_.end()) {
        return iter->second.userUnlock_;
    }
    return false;
}

size_t AbilityEventMapManager::GetUserCount()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return eventMap_.size();
}

void AbilityEventMapManager::AddPendingUserId(int32_t userId)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (pendingUserIds_.count(userId) > 0) {
        // One unlock chain authorizes one trigger, a duplicate record means the chain
        // passed twice without consumption in between.
        TAG_LOGW(AAFwkTag::ABILITYMGR, "AddPendingUserId duplicate: %{public}d.", userId);
        return;
    }
    TAG_LOGI(AAFwkTag::ABILITYMGR, "AddPendingUserId: %{public}d.", userId);
    pendingUserIds_.insert(userId);
}

bool AbilityEventMapManager::IsPendingUser(int32_t userId)
{
    std::lock_guard<std::mutex> lock(mutex_);
    return pendingUserIds_.find(userId) != pendingUserIds_.end();
}

void AbilityEventMapManager::RemovePendingUserId(int32_t userId)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (pendingUserIds_.erase(userId) == 0) {
        // The caller expected the user to be pending, but the record is already gone.
        TAG_LOGD(AAFwkTag::ABILITYMGR, "RemovePendingUserId not found: %{public}d.", userId);
        return;
    }
    TAG_LOGI(AAFwkTag::ABILITYMGR, "RemovePendingUserId: %{public}d.", userId);
}

AbilityScreenUnlockEventSubscriber::AbilityScreenUnlockEventSubscriber(
    const EventFwk::CommonEventSubscribeInfo &subscribeInfo, const std::function<void(int32_t)> &screenUnlockCallback)
    : EventFwk::CommonEventSubscriber(subscribeInfo), screenUnlockCallback_(screenUnlockCallback)
{}

void AbilityScreenUnlockEventSubscriber::OnReceiveEvent(const EventFwk::CommonEventData &data)
{
    const AAFwk::Want &want = data.GetWant();
    const auto &action = want.GetActionRef();
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
        // Remove the finished user before notifying, otherwise it is still counted by
        // UnSubscribeScreenUnlockedEvent as a pending user and the subscriber never goes away.
        AbilityEventMapManager::GetInstance().RemoveUser(userId);
        // Record the passed user for the auto startup trigger, consumed after the
        // trigger is done no matter whether the launches succeed.
        AbilityEventMapManager::GetInstance().AddPendingUserId(userId);
        screenUnlockCallback_(userId);
    }
}

AbilityUserUnlockEventSubscriber::AbilityUserUnlockEventSubscriber(
    const EventFwk::CommonEventSubscribeInfo &subscribeInfo, const std::function<void(int32_t)> &screenUnlockCallback,
    const std::function<void(int32_t)> &userScreenUnlockCallback)
    : EventFwk::CommonEventSubscriber(subscribeInfo), screenUnlockCallback_(screenUnlockCallback),
    userScreenUnlockCallback_(userScreenUnlockCallback)
{}

void AbilityUserUnlockEventSubscriber::OnReceiveEvent(const EventFwk::CommonEventData &data)
{
    const AAFwk::Want &want = data.GetWant();
    const auto &action = want.GetActionRef();
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
    UserController::GetInstance().SetUserLockStatus(userId, UserController::UserLockStatus::USER_UNLOCKED);
    if (AbilityEventMapManager::GetInstance().CheckAllUnlocked(userId)) {
        AbilityEventMapManager::GetInstance().RemoveUser(userId);
        AbilityEventMapManager::GetInstance().AddPendingUserId(userId);
        screenUnlockCallback_(userId);
        return;
    }
    userScreenUnlockCallback_(userId);
}
} // namespace AbilityRuntime
} // namespace OHOS