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

#ifndef OHOS_ABILITY_RUNTIME_SUB_MANAGERS_HELPER_H
#define OHOS_ABILITY_RUNTIME_SUB_MANAGERS_HELPER_H

#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "ability_connect_manager.h"
#include "ui_extension_ability_manager.h"
#include "common_extension_manager.h"
#include "ability_event_handler.h"
#include "cpp/mutex.h"
#include "data_ability_manager.h"
#include "mission_list_manager_interface.h"
#include "nocopyable.h"
#include "pending_want_manager.h"
#include "scene_board/ui_ability_lifecycle_manager.h"
#include "task_handler_wrap.h"

namespace OHOS {
namespace AAFwk {
using CreateMissionListMgrFunc = MissionListWrap*(*)();
class SubManagersHelper {
public:
    SubManagersHelper(std::shared_ptr<TaskHandlerWrap> taskHandler, std::shared_ptr<AbilityEventHandler> eventHandler);
    virtual ~SubManagersHelper();

    void InitSubManagers(int userId, bool switchUser);
    void InitMissionListManager(int userId, bool switchUser);
    void InitUIAbilityManager(int userId, bool switchUser);
    void InitDataAbilityManager(int32_t userId, bool switchUser);
    void InitPendWantManager(int32_t userId, bool switchUser);
    void InitUIExtensionAbilityManager(int32_t userId, bool switchUser);
    void InitCommonExtensionManager(int32_t userId, bool switchUser);

    void ClearSubManagers(int userId);

    std::shared_ptr<DataAbilityManager> GetCurrentDataAbilityManager();
    std::shared_ptr<DataAbilityManager> GetDataAbilityManager(const sptr<IAbilityScheduler> &scheduler);
    std::unordered_map<int, std::shared_ptr<DataAbilityManager>> GetDataAbilityManagers();
    std::shared_ptr<DataAbilityManager> GetDataAbilityManagerByUserId(int32_t userId);
    std::shared_ptr<DataAbilityManager> GetDataAbilityManagerByToken(const sptr<IRemoteObject> &token);

    std::vector<std::shared_ptr<AbilityConnectManager>> GetConnectManagers();

    std::shared_ptr<PendingWantManager> GetCurrentPendingWantManager();
    std::shared_ptr<PendingWantManager> GetPendingWantManagerByUserId(int32_t userId);

    std::unordered_map<int, std::shared_ptr<MissionListManagerInterface>> GetMissionListManagers();
    std::shared_ptr<MissionListManagerInterface> GetCurrentMissionListManager();
    std::shared_ptr<MissionListManagerInterface> GetMissionListManagerByUserId(int32_t userId);
    std::shared_ptr<MissionListManagerInterface> GetMissionListManagerByUid(int32_t uid);

    std::unordered_map<int, std::shared_ptr<UIAbilityLifecycleManager>> GetUIAbilityManagers();
    std::shared_ptr<UIAbilityLifecycleManager> GetCurrentUIAbilityManager();
    std::shared_ptr<UIAbilityLifecycleManager> GetUIAbilityManagerByUserId(int32_t userId);
    std::shared_ptr<UIAbilityLifecycleManager> GetUIAbilityManagerByUid(int32_t uid);

    std::unordered_map<int, std::shared_ptr<UIExtensionAbilityManager>> GetUIExtensionAbilityManagers();
    std::shared_ptr<UIExtensionAbilityManager> GetCurrentUIExtensionAbilityManager();
    std::shared_ptr<UIExtensionAbilityManager> GetUIExtensionAbilityManagerByUserId(int32_t userId);
    std::shared_ptr<UIExtensionAbilityManager> GetUIExtensionAbilityManagerByToken(const sptr<IRemoteObject> &token);
    std::shared_ptr<UIExtensionAbilityManager> GetUIExtensionAbilityManagerByAbilityRecordId(
        const int64_t &abilityRecordId);

    std::unordered_map<int, std::shared_ptr<CommonExtensionManager>> GetCommonExtensionManagers();
    std::shared_ptr<CommonExtensionManager> GetCurrentCommonExtensionManager();
    std::shared_ptr<CommonExtensionManager> GetCommonExtensionManagerByUserId(int32_t userId);
    std::shared_ptr<CommonExtensionManager> GetCommonExtensionManagerByToken(const sptr<IRemoteObject> &token);
    std::shared_ptr<CommonExtensionManager> GetCommonExtensionManagerByAbilityRecordId(const int64_t &abilityRecordId);

    void UninstallApp(const std::string &bundleName, int32_t uid);
    void UninstallAppInUIAbilityManagers(int32_t userId, const std::string &bundleName, int32_t uid);
    void UninstallAppInMissionListManagers(int32_t userId, const std::string &bundleName, int32_t uid);
    bool VerificationAllToken(const sptr<IRemoteObject> &token);
    bool VerificationAllTokenForConnectManagers(const sptr<IRemoteObject> &token);

    void HandlePendingWantDeathCleanup(const std::string &bundleName, int32_t pid);

    std::shared_ptr<MissionListWrap> GetMissionListWrap();
    std::shared_ptr<MissionListManagerInterface> CreateMissionListMgr(int32_t userId);
private:
    DISALLOW_COPY_AND_MOVE(SubManagersHelper);

    std::shared_ptr<TaskHandlerWrap> taskHandler_;
    std::shared_ptr<AbilityEventHandler> eventHandler_;
    std::shared_ptr<DataAbilityManager> currentDataAbilityManager_;
    std::shared_ptr<PendingWantManager> currentPendingWantManager_;
    std::shared_ptr<MissionListManagerInterface> currentMissionListManager_;
    std::shared_ptr<UIAbilityLifecycleManager> currentUIAbilityManager_;
    std::shared_ptr<UIExtensionAbilityManager> currentUIExtensionAbilityManager_;
    std::shared_ptr<CommonExtensionManager> currentCommonExtensionManager_;
    std::shared_ptr<MissionListWrap> missionListWrap_;
    std::unordered_map<int, std::shared_ptr<DataAbilityManager>> dataAbilityManagers_;
    std::unordered_map<int, std::shared_ptr<PendingWantManager>> pendingWantManagers_;
    std::unordered_map<int, std::shared_ptr<MissionListManagerInterface>> missionListManagers_;
    std::unordered_map<int, std::shared_ptr<UIAbilityLifecycleManager>> uiAbilityManagers_;
    std::unordered_map<int, std::shared_ptr<UIExtensionAbilityManager>> uiExtensionAbilityManagers_;
    std::unordered_map<int, std::shared_ptr<CommonExtensionManager>> commonExtensionManagers_;

    std::mutex missionListWrapMutex_;
    ffrt::mutex managersMutex_;
    void* missionLibHandle_ = nullptr;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_SUB_MANAGERS_HELPER_H
