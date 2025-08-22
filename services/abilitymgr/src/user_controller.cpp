/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#include "user_controller.h"

#include "ability_manager_service.h"
#include "hilog_tag_wrapper.h"
#include "mock_session_manager_service.h"
#include "os_account_manager_wrapper.h"
#include "scene_board_judgement.h"

namespace OHOS {
namespace AAFwk {
using namespace OHOS::AppExecFwk;
namespace {
const int64_t USER_SWITCH_TIMEOUT = 3 * 1000; // 3s
constexpr const char* DEVELOPER_MODE_STATE = "const.security.developermode.state";
}

UserItem::UserItem(int32_t id) : userId_(id)
{}

UserItem::~UserItem() {}

int32_t UserItem::GetUserId()
{
    return userId_;
}

void UserItem::SetState(const UserState &state)
{
    if (curState_ == state) {
        return;
    }
    lastState_ = curState_;
    curState_ = state;
}

UserState UserItem::GetState()
{
    return curState_;
}

UserController::UserController()
{
}

UserController::~UserController()
{
}

void UserController::Init()
{
    auto handler = DelayedSingleton<AbilityManagerService>::GetInstance()->GetTaskHandler();
    if (!handler) {
        return;
    }

    if (eventHandler_) {
        return;
    }
    eventHandler_ = std::make_shared<UserEventHandler>(handler, shared_from_this());
}

void UserController::ClearAbilityUserItems(int32_t userId)
{
    std::lock_guard<ffrt::mutex> guard(userLock_);
    if (userItems_.count(userId)) {
        userItems_.erase(userId);
    }
}

int UserController::StartUser(int32_t userId, sptr<IUserCallback> callback, bool isAppRecovery)
{
    if (userId == U1_USER_ID) {
        return StartNoHeadUser(userId, callback);
    }

    if (userId < 0 || userId == USER_ID_NO_HEAD) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "StartUserId invalid:%{public}d", userId);
        if (callback != nullptr) {
            callback->OnStartUserDone(userId, INVALID_USERID_VALUE);
        }
        return INVALID_USERID_VALUE;
    }

    if (IsCurrentUser(userId)) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "StartUser current:%{public}d", userId);
        if (callback != nullptr) {
            callback->OnStartUserDone(userId, ERR_OK);
        }
        return ERR_OK;
    }

    auto appScheduler = DelayedSingleton<AppScheduler>::GetInstance();
    if (!appScheduler) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null appScheduler");
        return ABILITY_SERVICE_NOT_CONNECTED;
    }
    appScheduler->SetEnableStartProcessFlagByUserId(userId, true);

    if (!IsExistOsAccount(userId)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null StartUser account:%{public}d", userId);
        if (callback != nullptr) {
            callback->OnStartUserDone(userId, INVALID_USERID_VALUE);
        }
        return INVALID_USERID_VALUE;
    }

    if (GetCurrentUserId() != USER_ID_NO_HEAD && !Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        // start freezing screen
        SetFreezingNewUserId(userId);
        DelayedSingleton<AbilityManagerService>::GetInstance()->StartFreezingScreen();
    }

    auto oldUserId = GetCurrentUserId();
    auto userItem = GetOrCreateUserItem(userId);
    auto state = userItem->GetState();
    if (state == STATE_STOPPING || state == STATE_SHUTDOWN) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "StartUser user stop, userId:%{public}d", userId);
        if (callback != nullptr) {
            callback->OnStartUserDone(userId, ERR_DEAD_OBJECT);
        }
        return ERR_DEAD_OBJECT;
    }

    SetCurrentUserId(userId);
    if (state == STATE_BOOTING) {
        // send user start msg.
        SendSystemUserStart(userId);
    }

    SendSystemUserCurrent(oldUserId, userId);
    SendReportUserSwitch(oldUserId, userId, userItem);
    SendUserSwitchTimeout(oldUserId, userId, userItem);
    return MoveUserToForeground(oldUserId, userId, callback, isAppRecovery);
}

int32_t UserController::StartNoHeadUser(int32_t userId, sptr<IUserCallback> callback) const
{
    if (!IsExistOsAccount(userId)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "U1 not exist");
        callback->OnStartUserDone(userId, INVALID_USERID_VALUE);
        return INVALID_USERID_VALUE;
    }

    auto abilityManagerService = DelayedSingleton<AbilityManagerService>::GetInstance();
    if (!abilityManagerService) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "abilityManagerService is nullptr");
        callback->OnLogoutUserDone(userId, GET_ABILITY_SERVICE_FAILED);
        return GET_ABILITY_SERVICE_FAILED;
    }
    abilityManagerService->UserStarted(userId);
    callback->OnStartUserDone(userId, ERR_OK);
    return ERR_OK;
}

int32_t UserController::StopUser(int32_t userId)
{
    if (userId < 0 || userId == USER_ID_NO_HEAD || userId == USER_ID_DEFAULT) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "userId invalid:%{public}d", userId);
        return -1;
    }

    if (IsCurrentUser(userId)) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "user current:%{public}d", userId);
        return 0;
    }

    if (!IsExistOsAccount(userId)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null account:%{public}d", userId);
        return -1;
    }

    BroadcastUserStopping(userId);

    auto appScheduler = DelayedSingleton<AppScheduler>::GetInstance();
    if (!appScheduler) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null appScheduler");
        return -1;
    }
    appScheduler->KillProcessesByUserId(userId);

    auto abilityManagerService = DelayedSingleton<AbilityManagerService>::GetInstance();
    if (!abilityManagerService) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "abilityManagerService null");
        return -1;
    }

    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        auto missionListWrap = abilityManagerService->GetMissionListWrap();
        if (!missionListWrap) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "missionListWrap null");
            return -1;
        }
        missionListWrap->RemoveUserDir(userId);
    }

    abilityManagerService->ClearUserData(userId);

    BroadcastUserStopped(userId);
    return 0;
}

int32_t UserController::LogoutUser(int32_t userId, sptr<IUserCallback> callback)
{
    if (userId < 0 || userId == USER_ID_NO_HEAD) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "userId invalid:%{public}d", userId);
        if (callback) {
            callback->OnLogoutUserDone(userId, INVALID_USERID_VALUE);
        }
        return INVALID_USERID_VALUE;
    }
    if (!IsExistOsAccount(userId)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null account:%{public}d", userId);
        if (callback) {
            callback->OnLogoutUserDone(userId, INVALID_USERID_VALUE);
        }
        return INVALID_USERID_VALUE;
    }
    auto abilityManagerService = DelayedSingleton<AbilityManagerService>::GetInstance();
    if (!abilityManagerService) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null abilityManagerService");
        if (callback) {
            callback->OnLogoutUserDone(userId, -1);
        }
        return -1;
    }
    abilityManagerService->RemoveLauncherDeathRecipient(userId);
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "SceneBoard exit normally.");
        Rosen::MockSessionManagerService::GetInstance().NotifyNotKillService();
    }
    auto appScheduler = DelayedSingleton<AppScheduler>::GetInstance();
    if (!appScheduler) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null appScheduler");
        if (callback) {
            callback->OnLogoutUserDone(userId, INVALID_USERID_VALUE);
        }
        return INVALID_USERID_VALUE;
    }
    abilityManagerService->ClearUserData(userId);
    appScheduler->SetEnableStartProcessFlagByUserId(userId, false);
    if (IsCurrentUser(userId)) {
        SetCurrentUserId(0);
    }
    appScheduler->KillProcessesByUserId(userId, system::GetBoolParameter(DEVELOPER_MODE_STATE, false), callback);
    ClearAbilityUserItems(userId);
    return 0;
}

int32_t UserController::GetCurrentUserId()
{
    std::lock_guard<ffrt::mutex> guard(userLock_);
    return currentUserId_;
}

std::shared_ptr<UserItem> UserController::GetUserItem(int32_t userId)
{
    std::lock_guard<ffrt::mutex> guard(userLock_);
    auto it = userItems_.find(userId);
    if (it != userItems_.end()) {
        return it->second;
    }

    return nullptr;
}

bool UserController::IsCurrentUser(int32_t userId)
{
    int32_t oldUserId = GetCurrentUserId();
    if (oldUserId == userId) {
        auto userItem = GetUserItem(userId);
        if (userItem) {
            TAG_LOGW(AAFwkTag::ABILITYMGR, "IsCurrentUserId current:%{public}d", userId);
            return true;
        }
    }
    return false;
}

bool UserController::IsExistOsAccount(int32_t userId) const
{
    bool isExist = false;
    auto errCode = DelayedSingleton<OsAccountManagerWrapper>::GetInstance()->IsOsAccountExists(userId, isExist);
    return (errCode == 0) && isExist;
}

std::shared_ptr<UserItem> UserController::GetOrCreateUserItem(int32_t userId)
{
    std::lock_guard<ffrt::mutex> guard(userLock_);
    auto it = userItems_.find(userId);
    if (it != userItems_.end()) {
        return it->second;
    }

    auto userItem = std::make_shared<UserItem>(userId);
    userItems_.emplace(userId, userItem);
    return userItem;
}

void UserController::SetCurrentUserId(int32_t userId)
{
    std::lock_guard<ffrt::mutex> guard(userLock_);
    currentUserId_ = userId;
    TAG_LOGD(AAFwkTag::ABILITYMGR, "set current userId: %{public}d", userId);
    DelayedSingleton<AppScheduler>::GetInstance()->SetCurrentUserId(userId);
}

int UserController::MoveUserToForeground(int32_t oldUserId, int32_t newUserId, sptr<IUserCallback> callback,
    bool isAppRecovery)
{
    auto manager = DelayedSingleton<AbilityManagerService>::GetInstance();
    if (!manager) {
        return ABILITY_SERVICE_NOT_CONNECTED;
    }
    auto ret = manager->SwitchToUser(oldUserId, newUserId, callback, isAppRecovery);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "SwitchToUser failed: %{public}d", ret);
    }
    BroadcastUserBackground(oldUserId);
    BroadcastUserForeground(newUserId);
    return ret;
}

void UserController::UserBootDone(std::shared_ptr<UserItem> &item)
{
    if (!item) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null item");
        return;
    }
    int32_t userId = item->GetUserId();

    std::lock_guard<ffrt::mutex> guard(userLock_);
    auto it = userItems_.find(userId);
    if (it == userItems_.end()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "invalid userId");
        return;
    }

    if (item != it->second) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "invalid item");
        return;
    }
    item->SetState(UserState::STATE_STARTED);
    auto manager = DelayedSingleton<AbilityManagerService>::GetInstance();
    if (!manager) {
        return;
    }
    manager->UserStarted(userId);
}

void UserController::BroadcastUserBackground(int32_t userId)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "user background");
}

void UserController::BroadcastUserForeground(int32_t userId)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "user foreground");
}

void UserController::BroadcastUserStopping(int32_t userId)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "user stopping");
}

void UserController::BroadcastUserStopped(int32_t userId)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "user stopped");
}

void UserController::SendSystemUserStart(int32_t userId)
{
    auto handler = eventHandler_;
    if (!handler) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null handler");
        return;
    }

    auto eventData = std::make_shared<UserEvent>();
    eventData->newUserId = userId;
    handler->SendEvent(EventWrap(UserEventHandler::EVENT_SYSTEM_USER_START, eventData));
    TAG_LOGD(AAFwkTag::ABILITYMGR, "SendEvent(EVENT_SYSTEM_USER_START)");
}

void UserController::ProcessEvent(const EventWrap &event)
{
    auto eventId = event.GetEventId();
    auto eventData = static_cast<UserEvent*>(event.GetEventData().get());
    if (!eventData) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "no event data, event id: %{public}u.", eventId);
        return;
    }

    TAG_LOGD(AAFwkTag::ABILITYMGR, "Event id obtained: %{public}u.", eventId);
    switch (eventId) {
        case UserEventHandler::EVENT_SYSTEM_USER_START: {
            HandleSystemUserStart(eventData->newUserId);
            break;
        }
        case UserEventHandler::EVENT_SYSTEM_USER_CURRENT: {
            HandleSystemUserCurrent(eventData->oldUserId, eventData->newUserId);
            break;
        }
        case UserEventHandler::EVENT_REPORT_USER_SWITCH: {
            HandleReportUserSwitch(eventData->oldUserId, eventData->newUserId, eventData->userItem);
            break;
        }
        case UserEventHandler::EVENT_CONTINUE_USER_SWITCH: {
            HandleContinueUserSwitch(eventData->oldUserId, eventData->newUserId, eventData->userItem);
            break;
        }
        case UserEventHandler::EVENT_USER_SWITCH_TIMEOUT: {
            HandleUserSwitchTimeout(eventData->oldUserId, eventData->newUserId, eventData->userItem);
            break;
        }
        case UserEventHandler::EVENT_REPORT_USER_SWITCH_DONE: {
            HandleUserSwitchDone(eventData->newUserId);
            break;
        }
        default: {
            TAG_LOGW(AAFwkTag::ABILITYMGR, "Unsupported  event.");
            break;
        }
    }
}

void UserController::SendSystemUserCurrent(int32_t oldUserId, int32_t newUserId)
{
    auto handler = eventHandler_;
    if (!handler) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null handler");
        return;
    }

    auto eventData = std::make_shared<UserEvent>();
    eventData->oldUserId = oldUserId;
    eventData->newUserId = newUserId;
    handler->SendEvent(EventWrap(UserEventHandler::EVENT_SYSTEM_USER_CURRENT, eventData));
    TAG_LOGD(AAFwkTag::ABILITYMGR, "SendEvent(EVENT_SYSTEM_USER_CURRENT)");
}

void UserController::SendReportUserSwitch(int32_t oldUserId, int32_t newUserId,
    std::shared_ptr<UserItem> &usrItem)
{
    auto handler = eventHandler_;
    if (!handler) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null handler");
        return;
    }

    auto eventData = std::make_shared<UserEvent>();
    eventData->oldUserId = oldUserId;
    eventData->newUserId = newUserId;
    eventData->userItem = usrItem;
    handler->SendEvent(EventWrap(UserEventHandler::EVENT_REPORT_USER_SWITCH, eventData));
    TAG_LOGD(AAFwkTag::ABILITYMGR, "SendEvent(EVENT_REPORT_USER_SWITCH)");
}

void UserController::SendUserSwitchTimeout(int32_t oldUserId, int32_t newUserId,
    std::shared_ptr<UserItem> &usrItem)
{
    auto handler = eventHandler_;
    if (!handler) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null handler");
        return;
    }

    auto eventData = std::make_shared<UserEvent>();
    eventData->oldUserId = oldUserId;
    eventData->newUserId = newUserId;
    eventData->userItem = usrItem;
    handler->SendEvent(EventWrap(UserEventHandler::EVENT_USER_SWITCH_TIMEOUT,
        eventData), USER_SWITCH_TIMEOUT);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "SendEvent(EVENT_USER_SWITCH_TIMEOUT)");
}

void UserController::SendContinueUserSwitch(int32_t oldUserId, int32_t newUserId,
    std::shared_ptr<UserItem> &usrItem)
{
    auto handler = eventHandler_;
    if (!handler) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null handler");
        return;
    }

    auto eventData = std::make_shared<UserEvent>();
    eventData->oldUserId = oldUserId;
    eventData->newUserId = newUserId;
    eventData->userItem = usrItem;
    handler->SendEvent(EventWrap(UserEventHandler::EVENT_CONTINUE_USER_SWITCH, eventData));
    TAG_LOGD(AAFwkTag::ABILITYMGR, "SendEvent(EVENT_CONTINUE_USER_SWITCH)");
}

void UserController::SendUserSwitchDone(int32_t userId)
{
    auto handler = eventHandler_;
    if (!handler) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null handler");
        return;
    }

    auto eventData = std::make_shared<UserEvent>();
    eventData->newUserId = userId;
    handler->SendEvent(EventWrap(UserEventHandler::EVENT_REPORT_USER_SWITCH_DONE,
        eventData));
    TAG_LOGD(AAFwkTag::ABILITYMGR, "SendEvent(EVENT_REPORT_USER_SWITCH_DONE)");
}

void UserController::HandleSystemUserStart(int32_t userId)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "notify system user start.");
}

void UserController::HandleSystemUserCurrent(int32_t oldUserId, int32_t newUserId)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "notify system user current.");
}

void UserController::HandleReportUserSwitch(int32_t oldUserId, int32_t newUserId,
    std::shared_ptr<UserItem> &usrItem)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "notify report user switch.");
}

void UserController::HandleUserSwitchTimeout(int32_t oldUserId, int32_t newUserId,
    std::shared_ptr<UserItem> &usrItem)
{
    SendContinueUserSwitch(oldUserId, newUserId, usrItem);
}

void UserController::HandleContinueUserSwitch(int32_t oldUserId, int32_t newUserId,
    std::shared_ptr<UserItem> &usrItem)
{
    auto manager = DelayedSingleton<AbilityManagerService>::GetInstance();
    if (manager && !Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        manager->StopFreezingScreen();
    }
    SendUserSwitchDone(newUserId);
}

void UserController::HandleUserSwitchDone(int32_t userId)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "handle user switch done.");
}

int32_t UserController::GetFreezingNewUserId() const
{
    return freezingNewUserId_;
}

void UserController::SetFreezingNewUserId(int32_t userId)
{
    freezingNewUserId_ = userId;
}
}
}
