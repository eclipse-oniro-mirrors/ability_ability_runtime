/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#include "sub_managers_helper.h"

#include <dlfcn.h>

#include "ability_cache_manager.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "os_account_manager_wrapper.h"
#include "scene_board_judgement.h"
#include "server_constant.h"
#include "user_controller/user_controller.h"

namespace OHOS {
namespace AAFwk {
constexpr int32_t INVALID_USER_ID = -1;

SubManagersHelper::SubManagersHelper(
    std::shared_ptr<TaskHandlerWrap> taskHandler, std::shared_ptr<AbilityEventHandler> eventHandler)
    : taskHandler_(taskHandler), eventHandler_(eventHandler) {}

SubManagersHelper::~SubManagersHelper()
{
    if (missionLibHandle_ != nullptr) {
        missionListWrap_ = nullptr;
        dlclose(missionLibHandle_);
        missionLibHandle_ = nullptr;
    }
}

void SubManagersHelper::InitSubManagers(int userId, bool switchUser)
{
    if (userId == U1_USER_ID) {
        InitUIExtensionAbilityManager(userId, false);
        InitCommonExtensionManager(userId, false);
        TAG_LOGI(AAFwkTag::ABILITYMGR, "Init U1");
        return;
    }
    InitUIExtensionAbilityManager(userId, switchUser);
    InitCommonExtensionManager(userId, switchUser);
    InitDataAbilityManager(userId, switchUser);
    InitPendWantManager(userId, switchUser);
    if (userId != U0_USER_ID) {
        if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
            InitUIAbilityManager(userId, switchUser);
        } else {
            InitMissionListManager(userId, switchUser);
        }
    }
}

void SubManagersHelper::InitUIExtensionAbilityManager(int32_t userId, bool switchUser)
{
    std::lock_guard<ffrt::mutex> lock(managersMutex_);
    auto it = uiExtensionAbilityManagers_.find(userId);
    if (it != uiExtensionAbilityManagers_.end()) {
        if (switchUser) {
            currentUIExtensionAbilityManager_ = it->second;
        }
        return;
    }
    auto manager = std::make_shared<UIExtensionAbilityManager>(userId);
    manager->SetTaskHandler(taskHandler_);
    manager->SetEventHandler(eventHandler_);
    uiExtensionAbilityManagers_.emplace(userId, manager);
    if (switchUser) {
        currentUIExtensionAbilityManager_ = manager;
    }
}

void SubManagersHelper::InitCommonExtensionManager(int32_t userId, bool switchUser)
{
    std::lock_guard<ffrt::mutex> lock(managersMutex_);
    auto it = commonExtensionManagers_.find(userId);
    if (it != commonExtensionManagers_.end()) {
        if (switchUser) {
            currentCommonExtensionManager_ = it->second;
        }
        return;
    }
    auto manager = std::make_shared<CommonExtensionManager>(userId);
    manager->SetTaskHandler(taskHandler_);
    manager->SetEventHandler(eventHandler_);
    commonExtensionManagers_.emplace(userId, manager);
    if (switchUser) {
        currentCommonExtensionManager_ = manager;
    }
}

void SubManagersHelper::InitDataAbilityManager(int32_t userId, bool switchUser)
{
    std::lock_guard<ffrt::mutex> lock(managersMutex_);
    auto it = dataAbilityManagers_.find(userId);
    if (it != dataAbilityManagers_.end()) {
        if (switchUser) {
            currentDataAbilityManager_ = it->second;
        }
        return;
    }
    auto manager = std::make_shared<DataAbilityManager>();
    dataAbilityManagers_.emplace(userId, manager);
    if (switchUser) {
        currentDataAbilityManager_ = manager;
    }
}

void SubManagersHelper::InitPendWantManager(int32_t userId, bool switchUser)
{
    std::lock_guard<ffrt::mutex> lock(managersMutex_);
    auto it = pendingWantManagers_.find(userId);
    if (it != pendingWantManagers_.end()) {
        if (switchUser) {
            currentPendingWantManager_ = it->second;
        }
        return;
    }
    auto manager = std::make_shared<PendingWantManager>();
    pendingWantManagers_.emplace(userId, manager);
    if (switchUser) {
        currentPendingWantManager_ = manager;
    }
}

void SubManagersHelper::InitMissionListManager(int userId, bool switchUser)
{
    std::lock_guard<ffrt::mutex> lock(managersMutex_);
    auto it = missionListManagers_.find(userId);
    if (it != missionListManagers_.end()) {
        if (switchUser) {
            auto missionListWrap = GetMissionListWrap();
            if (missionListWrap) {
                missionListWrap->InitMissionInfoMgr(userId);
            }
            currentMissionListManager_ = it->second;
        }
        return;
    }
    auto manager = CreateMissionListMgr(userId);
    if (manager == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "manager empty");
        return;
    }
    manager->Init();
    missionListManagers_.emplace(userId, manager);
    if (switchUser) {
        currentMissionListManager_ = manager;
    }
}

void SubManagersHelper::InitUIAbilityManager(int userId, bool switchUser)
{
    std::lock_guard<ffrt::mutex> lock(managersMutex_);
    auto it = uiAbilityManagers_.find(userId);
    if (it != uiAbilityManagers_.end()) {
        if (switchUser) {
            currentUIAbilityManager_ = it->second;
        }
        return;
    }
    auto manager = std::make_shared<UIAbilityLifecycleManager>(userId);
    uiAbilityManagers_.emplace(userId, manager);
    if (switchUser) {
        currentUIAbilityManager_ = manager;
    }
}

void SubManagersHelper::ClearSubManagers(int userId)
{
    std::lock_guard<ffrt::mutex> lock(managersMutex_);
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        uiAbilityManagers_.erase(userId);
    } else {
        missionListManagers_.erase(userId);
    }
    dataAbilityManagers_.erase(userId);
    pendingWantManagers_.erase(userId);
    uiExtensionAbilityManagers_.erase(userId);
    commonExtensionManagers_.erase(userId);
}

std::shared_ptr<DataAbilityManager> SubManagersHelper::GetCurrentDataAbilityManager()
{
    std::lock_guard<ffrt::mutex> lock(managersMutex_);
    return currentDataAbilityManager_;
}

std::shared_ptr<DataAbilityManager> SubManagersHelper::GetDataAbilityManager(const sptr<IAbilityScheduler> &scheduler)
{
    if (scheduler == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null scheduler");
        return nullptr;
    }

    std::lock_guard<ffrt::mutex> lock(managersMutex_);
    for (auto& item: dataAbilityManagers_) {
        if (item.second && item.second->ContainsDataAbility(scheduler)) {
            return item.second;
        }
    }

    return nullptr;
}

std::unordered_map<int, std::shared_ptr<DataAbilityManager>> SubManagersHelper::GetDataAbilityManagers()
{
    std::lock_guard<ffrt::mutex> lock(managersMutex_);
    return dataAbilityManagers_;
}

std::shared_ptr<DataAbilityManager> SubManagersHelper::GetDataAbilityManagerByUserId(int32_t userId)
{
    std::lock_guard<ffrt::mutex> lock(managersMutex_);
    auto it = dataAbilityManagers_.find(userId);
    if (it != dataAbilityManagers_.end()) {
        return it->second;
    }
    TAG_LOGE(AAFwkTag::ABILITYMGR, "failed. UserId: %{public}d", userId);
    return nullptr;
}

std::shared_ptr<DataAbilityManager> SubManagersHelper::GetDataAbilityManagerByToken(const sptr<IRemoteObject> &token)
{
    std::lock_guard<ffrt::mutex> lock(managersMutex_);
    for (auto& item: dataAbilityManagers_) {
        if (item.second && item.second->GetAbilityRecordByToken(token)) {
            return item.second;
        }
    }

    return nullptr;
}

std::vector<std::shared_ptr<AbilityConnectManager>> SubManagersHelper::GetConnectManagers()
{
    std::lock_guard<ffrt::mutex> lock(managersMutex_);
    
    size_t totalSize = uiExtensionAbilityManagers_.size() +
                      commonExtensionManagers_.size();
    
    std::vector<std::shared_ptr<AbilityConnectManager>> result;
    result.reserve(totalSize);
    
    for (const auto& [key, value] : commonExtensionManagers_) {
        result.push_back(value);
    }

    for (const auto& [key, value] : uiExtensionAbilityManagers_) {
        result.push_back(value);
    }
    
    return result;
}

std::unordered_map<int, std::shared_ptr<UIExtensionAbilityManager>> SubManagersHelper::GetUIExtensionAbilityManagers()
{
    std::lock_guard<ffrt::mutex> lock(managersMutex_);
    return uiExtensionAbilityManagers_;
}

std::shared_ptr<UIExtensionAbilityManager> SubManagersHelper::GetCurrentUIExtensionAbilityManager()
{
    std::lock_guard<ffrt::mutex> lock(managersMutex_);
    return currentUIExtensionAbilityManager_;
}

std::shared_ptr<UIExtensionAbilityManager> SubManagersHelper::GetUIExtensionAbilityManagerByUserId(int32_t userId)
{
    std::lock_guard<ffrt::mutex> lock(managersMutex_);
    auto it = uiExtensionAbilityManagers_.find(userId);
    if (it != uiExtensionAbilityManagers_.end()) {
        return it->second;
    }
    TAG_LOGE(AAFwkTag::ABILITYMGR, "failed. UserId: %{public}d", userId);
    return nullptr;
}

std::shared_ptr<UIExtensionAbilityManager> SubManagersHelper::GetUIExtensionAbilityManagerByToken(
    const sptr<IRemoteObject> &token)
{
    std::lock_guard<ffrt::mutex> lock(managersMutex_);
    for (auto& item: uiExtensionAbilityManagers_) {
        if (item.second && item.second->GetExtensionByTokenFromServiceMap(token)) {
            return item.second;
        }
        if (item.second && item.second->GetExtensionByTokenFromTerminatingMap(token)) {
            return item.second;
        }
    }
    auto abilityRecord = AbilityCacheManager::GetInstance().FindRecordByToken(token);
    if (abilityRecord == nullptr) {
        return nullptr;
    }
    auto iter = uiExtensionAbilityManagers_.find(abilityRecord->GetOwnerMissionUserId());
    if (iter == uiExtensionAbilityManagers_.end()) {
        return nullptr;
    }
    return iter->second;
}

std::shared_ptr<UIExtensionAbilityManager> SubManagersHelper::GetUIExtensionAbilityManagerByAbilityRecordId(
    const int64_t &abilityRecordId)
{
    std::lock_guard<ffrt::mutex> lock(managersMutex_);
    for (auto& item: uiExtensionAbilityManagers_) {
        if (item.second == nullptr) {
            continue;
        }
        if (item.second->GetExtensionByIdFromServiceMap(abilityRecordId)) {
            return item.second;
        }
        if (item.second->GetExtensionByIdFromTerminatingMap(abilityRecordId)) {
            return item.second;
        }
    }

    return nullptr;
}

std::unordered_map<int, std::shared_ptr<CommonExtensionManager>> SubManagersHelper::GetCommonExtensionManagers()
{
    std::lock_guard<ffrt::mutex> lock(managersMutex_);
    return commonExtensionManagers_;
}

std::shared_ptr<CommonExtensionManager> SubManagersHelper::GetCurrentCommonExtensionManager()
{
    std::lock_guard<ffrt::mutex> lock(managersMutex_);
    return currentCommonExtensionManager_;
}

std::shared_ptr<CommonExtensionManager> SubManagersHelper::GetCommonExtensionManagerByUserId(int32_t userId)
{
    std::lock_guard<ffrt::mutex> lock(managersMutex_);
    auto it = commonExtensionManagers_.find(userId);
    if (it != commonExtensionManagers_.end()) {
        return it->second;
    }
    TAG_LOGE(AAFwkTag::ABILITYMGR, "failed. UserId: %{public}d", userId);
    return nullptr;
}

std::shared_ptr<CommonExtensionManager> SubManagersHelper::GetCommonExtensionManagerByToken(
    const sptr<IRemoteObject> &token)
{
    std::lock_guard<ffrt::mutex> lock(managersMutex_);
    for (auto& item: commonExtensionManagers_) {
        if (item.second && item.second->GetExtensionByTokenFromServiceMap(token)) {
            return item.second;
        }
        if (item.second && item.second->GetExtensionByTokenFromTerminatingMap(token)) {
            return item.second;
        }
    }
    auto abilityRecord = AbilityCacheManager::GetInstance().FindRecordByToken(token);
    if (abilityRecord == nullptr) {
        return nullptr;
    }
    auto iter = commonExtensionManagers_.find(abilityRecord->GetOwnerMissionUserId());
    if (iter == commonExtensionManagers_.end()) {
        return nullptr;
    }
    return iter->second;
}

std::shared_ptr<CommonExtensionManager> SubManagersHelper::GetCommonExtensionManagerByAbilityRecordId(
    const int64_t &abilityRecordId)
{
    std::lock_guard<ffrt::mutex> lock(managersMutex_);
    for (auto& item: commonExtensionManagers_) {
        if (item.second == nullptr) {
            continue;
        }
        if (item.second->GetExtensionByIdFromServiceMap(abilityRecordId)) {
            return item.second;
        }
        if (item.second->GetExtensionByIdFromTerminatingMap(abilityRecordId)) {
            return item.second;
        }
    }

    return nullptr;
}

std::shared_ptr<PendingWantManager> SubManagersHelper::GetCurrentPendingWantManager()
{
    std::lock_guard<ffrt::mutex> lock(managersMutex_);
    return currentPendingWantManager_;
}

std::shared_ptr<PendingWantManager> SubManagersHelper::GetPendingWantManagerByUserId(int32_t userId)
{
    std::lock_guard<ffrt::mutex> lock(managersMutex_);
    auto it = pendingWantManagers_.find(userId);
    if (it != pendingWantManagers_.end()) {
        return it->second;
    }
    TAG_LOGE(AAFwkTag::ABILITYMGR, "failed.UserId: %{public}d", userId);
    return nullptr;
}

std::unordered_map<int, std::shared_ptr<MissionListManagerInterface>> SubManagersHelper::GetMissionListManagers()
{
    std::lock_guard<ffrt::mutex> lock(managersMutex_);
    return missionListManagers_;
}

std::shared_ptr<MissionListManagerInterface> SubManagersHelper::GetCurrentMissionListManager()
{
    std::lock_guard<ffrt::mutex> lock(managersMutex_);
    return currentMissionListManager_;
}

std::shared_ptr<MissionListManagerInterface> SubManagersHelper::GetMissionListManagerByUserId(int32_t userId)
{
    std::lock_guard<ffrt::mutex> lock(managersMutex_);
    auto it = missionListManagers_.find(userId);
    if (it != missionListManagers_.end()) {
        return it->second;
    }
    TAG_LOGE(AAFwkTag::ABILITYMGR, "failed UserId: %{public}d", userId);
    return nullptr;
}

std::shared_ptr<MissionListManagerInterface> SubManagersHelper::GetMissionListManagerByUid(int32_t uid)
{
    int32_t userId = INVALID_USER_ID;
    int32_t getOsAccountRet =
        AppExecFwk::OsAccountManagerWrapper::GetOsAccountLocalIdFromUid(uid, userId);
    if (getOsAccountRet != 0) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "GetOsAccountLocalIdFromUid() failed. ret: %{public}d", getOsAccountRet);
        return nullptr;
    }
    TAG_LOGD(AAFwkTag::ABILITYMGR, "userId: %{public}d", userId);
    if (userId == U0_USER_ID) {
        std::lock_guard<ffrt::mutex> lock(managersMutex_);
        return currentMissionListManager_;
    }
    return GetMissionListManagerByUserId(userId);
}

std::unordered_map<int, std::shared_ptr<UIAbilityLifecycleManager>> SubManagersHelper::GetUIAbilityManagers()
{
    std::lock_guard<ffrt::mutex> lock(managersMutex_);
    return uiAbilityManagers_;
}

std::shared_ptr<UIAbilityLifecycleManager> SubManagersHelper::GetCurrentUIAbilityManager()
{
    std::lock_guard<ffrt::mutex> lock(managersMutex_);
    return currentUIAbilityManager_;
}

std::shared_ptr<UIAbilityLifecycleManager> SubManagersHelper::GetUIAbilityManagerByUserId(int32_t userId)
{
    std::lock_guard<ffrt::mutex> lock(managersMutex_);
    auto it = uiAbilityManagers_.find(userId);
    if (it != uiAbilityManagers_.end()) {
        return it->second;
    }
    TAG_LOGE(AAFwkTag::ABILITYMGR, "fail UserId: %{public}d", userId);
    return nullptr;
}

std::shared_ptr<UIAbilityLifecycleManager> SubManagersHelper::GetUIAbilityManagerByUid(int32_t uid)
{
    int32_t userId = INVALID_USER_ID;
    int32_t getOsAccountRet =
        AppExecFwk::OsAccountManagerWrapper::GetOsAccountLocalIdFromUid(uid, userId);
    if (getOsAccountRet != 0) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "GetOsAccountLocalIdFromUid() failed. ret: %{public}d", getOsAccountRet);
        return nullptr;
    }
    TAG_LOGD(AAFwkTag::ABILITYMGR, "userId: %{public}d", userId);
    if (userId == U0_USER_ID) {
        userId = AbilityRuntime::UserController::GetInstance().GetForegroundUserId(
            AbilityRuntime::ServerConstant::DEFAULT_DISPLAY_ID);
    }
    return GetUIAbilityManagerByUserId(userId);
}

void SubManagersHelper::UninstallApp(const std::string &bundleName, int32_t uid)
{
    int32_t userId = INVALID_USER_ID;
    int32_t getOsAccountRet =
        AppExecFwk::OsAccountManagerWrapper::GetOsAccountLocalIdFromUid(uid, userId);
    if (getOsAccountRet != 0) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "GetOsAccountLocalIdFromUid() failed. ret: %{public}d", getOsAccountRet);
        return;
    }
    TAG_LOGD(AAFwkTag::ABILITYMGR, "userId: %{public}d", userId);
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        UninstallAppInUIAbilityManagers(userId, bundleName, uid);
    } else {
        UninstallAppInMissionListManagers(userId, bundleName, uid);
    }

    auto currentPendingWantManager = GetCurrentPendingWantManager();
    if (currentPendingWantManager) {
        currentPendingWantManager->ClearPendingWantRecord(bundleName, uid);
    }
}

void SubManagersHelper::UninstallAppInUIAbilityManagers(int32_t userId, const std::string &bundleName, int32_t uid)
{
    if (userId == U0_USER_ID) {
        auto uiAbilityManagers = GetUIAbilityManagers();
        for (auto& item : uiAbilityManagers) {
            if (item.second) {
                item.second->UninstallApp(bundleName, uid);
            }
        }
    } else {
        auto manager = GetUIAbilityManagerByUserId(userId);
        if (manager) {
            manager->UninstallApp(bundleName, uid);
        }
    }
}

void SubManagersHelper::UninstallAppInMissionListManagers(int32_t userId, const std::string &bundleName, int32_t uid)
{
    if (userId == U0_USER_ID) {
        auto missionListManagers = GetMissionListManagers();
        for (auto& item : missionListManagers) {
            if (item.second) {
                item.second->UninstallApp(bundleName, uid);
            }
        }
    } else {
        auto listManager = GetMissionListManagerByUserId(userId);
        if (listManager) {
            listManager->UninstallApp(bundleName, uid);
        }
    }
}

bool SubManagersHelper::VerificationAllTokenForConnectManagers(const sptr<IRemoteObject> &token)
{
    auto connectManagers = GetConnectManagers();
    for (auto& item: connectManagers) {
        if (item && item->GetExtensionByTokenFromServiceMap(token)) {
            return true;
        }
        if (item && item->GetExtensionByTokenFromTerminatingMap(token)) {
            return true;
        }
    }
    if (AbilityCacheManager::GetInstance().FindRecordByToken(token)) {
        return true;
    }
    return false;
}

bool SubManagersHelper::VerificationAllToken(const sptr<IRemoteObject> &token)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "VerificationAllToken.");
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        auto uiAbilityManagers = GetUIAbilityManagers();
        for (auto& item: uiAbilityManagers) {
            if (item.second && item.second->IsContainsAbility(token)) {
                return true;
            }
        }
    } else {
        HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, "VerificationAllToken::SearchMissionListManagers");
        auto missionListManagers = GetMissionListManagers();
        for (auto& item: missionListManagers) {
            if (item.second && item.second->GetAbilityRecordByToken(token)) {
                return true;
            }
            if (item.second && item.second->GetAbilityFromTerminateList(token)) {
                return true;
            }
        }
    }
    {
        HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, "VerificationAllToken::SearchDataAbilityManagers_");
        auto dataAbilityManagers = GetDataAbilityManagers();
        for (auto& item: dataAbilityManagers) {
            if (item.second && item.second->GetAbilityRecordByToken(token)) {
                return true;
            }
        }
    }
    {
        HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, "VerificationAllToken::SearchConnectManagers_");
        if (VerificationAllTokenForConnectManagers(token)) {
            return true;
        }
    }
    TAG_LOGE(AAFwkTag::ABILITYMGR, "fail");
    return false;
}

std::shared_ptr<MissionListWrap> SubManagersHelper::GetMissionListWrap()
{
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        return nullptr;
    }

    std::lock_guard lock(missionListWrapMutex_);
    if (missionListWrap_) {
        return missionListWrap_;
    }

    if (missionLibHandle_ == nullptr) {
        missionLibHandle_ = dlopen("libmission_list.z.so", RTLD_NOW | RTLD_GLOBAL);
        if (missionLibHandle_ == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "open mission_list library failed");
            return nullptr;
        }
    }

    auto createMissionListWrapFunc = reinterpret_cast<CreateMissionListMgrFunc>(dlsym(missionLibHandle_,
        "CreateMissionListWrap"));
    if (createMissionListWrapFunc == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "createFunc empty");
        dlclose(missionLibHandle_);
        missionLibHandle_ = nullptr;
        return nullptr;
    }

    missionListWrap_ = std::shared_ptr<MissionListWrap>(createMissionListWrapFunc());
    return missionListWrap_;
}

std::shared_ptr<MissionListManagerInterface> SubManagersHelper::CreateMissionListMgr(int32_t userId)
{
    auto missionListWrap = GetMissionListWrap();
    if (missionListWrap != nullptr) {
        return missionListWrap->CreateMissionListManager(userId);
    }

    return nullptr;
}
}  // namespace AAFwk
}  // namespace OHOS