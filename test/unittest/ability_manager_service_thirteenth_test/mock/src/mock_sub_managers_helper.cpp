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

#include "mock_sub_managers_helper.h"
#include "mock_my_status.h"

namespace OHOS {
namespace AAFwk {

SubManagersHelper::SubManagersHelper(
    std::shared_ptr<TaskHandlerWrap> taskHandler, std::shared_ptr<AbilityEventHandler> eventHandler)
    : taskHandler_(taskHandler), eventHandler_(eventHandler) {}

SubManagersHelper::~SubManagersHelper()
{
}

void SubManagersHelper::InitSubManagers(int userId, bool switchUser)
{
}

void SubManagersHelper::InitUIExtensionAbilityManager(int32_t userId, bool switchUser)
{
}

void SubManagersHelper::InitCommonExtensionManager(int32_t userId, bool switchUser)
{
}

void SubManagersHelper::InitDataAbilityManager(int32_t userId, bool switchUser)
{
}

void SubManagersHelper::InitPendWantManager(int32_t userId, bool switchUser)
{
}

void SubManagersHelper::InitMissionListManager(int userId, bool switchUser)
{
}

void SubManagersHelper::InitUIAbilityManager(int userId, bool switchUser)
{
}

void SubManagersHelper::ClearSubManagers(int userId)
{
}

std::shared_ptr<DataAbilityManager> SubManagersHelper::GetCurrentDataAbilityManager()
{
    if (MyStatus::GetInstance().smhGetCurrentDataAbilityManager_) {
        return currentDataAbilityManager_;
    }
    return nullptr;
}

std::shared_ptr<DataAbilityManager> SubManagersHelper::GetDataAbilityManager(const sptr<IAbilityScheduler> &scheduler)
{
    return nullptr;
}

std::unordered_map<int, std::shared_ptr<DataAbilityManager>> SubManagersHelper::GetDataAbilityManagers()
{
    return dataAbilityManagers_;
}

std::shared_ptr<DataAbilityManager> SubManagersHelper::GetDataAbilityManagerByUserId(int32_t userId)
{
    return nullptr;
}

std::shared_ptr<DataAbilityManager> SubManagersHelper::GetDataAbilityManagerByToken(const sptr<IRemoteObject> &token)
{
    return nullptr;
}

std::vector<std::shared_ptr<AbilityConnectManager>> SubManagersHelper::GetConnectManagers()
{
    return std::vector<std::shared_ptr<AbilityConnectManager>>{};
}

std::unordered_map<int, std::shared_ptr<UIExtensionAbilityManager>> SubManagersHelper::GetUIExtensionAbilityManagers()
{
    return uiExtensionAbilityManagers_;
}

std::shared_ptr<UIExtensionAbilityManager> SubManagersHelper::GetCurrentUIExtensionAbilityManager()
{
    return currentUIExtensionAbilityManager_;
}

std::shared_ptr<UIExtensionAbilityManager> SubManagersHelper::GetUIExtensionAbilityManagerByUserId(int32_t userId)
{
    if (MyStatus::GetInstance().smhGetConnectManagerByToken_) {
        return currentUIExtensionAbilityManager_;
    }
    return nullptr;
}

std::shared_ptr<UIExtensionAbilityManager> SubManagersHelper::GetUIExtensionAbilityManagerByToken(
    const sptr<IRemoteObject> &token)
{
    return nullptr;
}

std::shared_ptr<UIExtensionAbilityManager> SubManagersHelper::GetUIExtensionAbilityManagerByAbilityRecordId(
    const int64_t &abilityRecordId)
{
    return nullptr;
}

std::unordered_map<int, std::shared_ptr<CommonExtensionManager>> SubManagersHelper::GetCommonExtensionManagers()
{
    return commonExtensionManagers_;
}

std::shared_ptr<CommonExtensionManager> SubManagersHelper::GetCurrentCommonExtensionManager()
{
    return currentCommonExtensionManager_;
}

std::shared_ptr<CommonExtensionManager> SubManagersHelper::GetCommonExtensionManagerByUserId(int32_t userId)
{
    if (MyStatus::GetInstance().smhGetConnectManagerByToken_) {
        return currentCommonExtensionManager_;
    }
    return nullptr;
}

std::shared_ptr<CommonExtensionManager> SubManagersHelper::GetCommonExtensionManagerByToken(
    const sptr<IRemoteObject> &token)
{
    return nullptr;
}

std::shared_ptr<CommonExtensionManager> SubManagersHelper::GetCommonExtensionManagerByAbilityRecordId(
    const int64_t &abilityRecordId)
{
    return nullptr;
}

std::shared_ptr<PendingWantManager> SubManagersHelper::GetCurrentPendingWantManager()
{
    return currentPendingWantManager_;
}

std::shared_ptr<PendingWantManager> SubManagersHelper::GetPendingWantManagerByUserId(int32_t userId)
{
    return nullptr;
}

std::unordered_map<int, std::shared_ptr<MissionListManagerInterface>> SubManagersHelper::GetMissionListManagers()
{
    return missionListManagers_;
}

std::shared_ptr<MissionListManagerInterface> SubManagersHelper::GetCurrentMissionListManager()
{
    return currentMissionListManager_;
}

std::shared_ptr<MissionListManagerInterface> SubManagersHelper::GetMissionListManagerByUserId(int32_t userId)
{
    if (MyStatus::GetInstance().smhGetMissionListManagerByUserId_) {
        return currentMissionListManager_;
    }
    return nullptr;
}

std::shared_ptr<MissionListManagerInterface> SubManagersHelper::GetMissionListManagerByUid(int32_t uid)
{
    return nullptr;
}

std::unordered_map<int, std::shared_ptr<UIAbilityLifecycleManager>> SubManagersHelper::GetUIAbilityManagers()
{
    return uiAbilityManagers_;
}

std::shared_ptr<UIAbilityLifecycleManager> SubManagersHelper::GetCurrentUIAbilityManager()
{
    return currentUIAbilityManager_;
}

std::shared_ptr<UIAbilityLifecycleManager> SubManagersHelper::GetUIAbilityManagerByUserId(int32_t userId)
{
    if (MyStatus::GetInstance().smhGetUIAbilityManagerByUserId_) {
        return currentUIAbilityManager_;
    }
    return nullptr;
}

std::shared_ptr<UIAbilityLifecycleManager> SubManagersHelper::GetUIAbilityManagerByUid(int32_t uid)
{
    if (MyStatus::GetInstance().smhGetUIAbilityManagerByUid_) {
        return currentUIAbilityManager_;
    }
    return nullptr;
}

void SubManagersHelper::UninstallApp(const std::string &bundleName, int32_t uid)
{
}

void SubManagersHelper::UninstallAppInUIAbilityManagers(int32_t userId, const std::string &bundleName, int32_t uid)
{
}

void SubManagersHelper::UninstallAppInMissionListManagers(int32_t userId, const std::string &bundleName, int32_t uid)
{
}

bool SubManagersHelper::VerificationAllTokenForConnectManagers(const sptr<IRemoteObject> &token)
{
    return false;
}

bool SubManagersHelper::VerificationAllToken(const sptr<IRemoteObject> &token)
{
    return MyStatus::GetInstance().smhVerificationAllToken_;
}

std::shared_ptr<MissionListWrap> SubManagersHelper::GetMissionListWrap()
{
    return missionListWrap_;
}

std::shared_ptr<MissionListManagerInterface> SubManagersHelper::CreateMissionListMgr(int32_t userId)
{
    return nullptr;
}

void SubManagersHelper::HandlePendingWantDeathCleanup(const std::string &bundleName, int32_t pid)
{
}
}  // namespace AAFwk
}  // namespace OHOS