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

#ifndef MOCK_ABILITY_MANAGER_SERVICE_H
#define MOCK_ABILITY_MANAGER_SERVICE_H

#include <string>
#include <vector>
#include <memory>
#include "mock_flag.h"

namespace OHOS {
namespace AAFwk {

// Forward declare
class UIAbilityLifecycleManager;
class MissionListManagerInterface;
class UIExtensionAbilityManager;

class UIAbilityLifecycleManager {
public:
    void GetActiveAbilityList(int32_t uid, std::vector<std::string> &abilityList)
    {
        if (MockFlag::hasRunningUIAbility) {
            abilityList.push_back("TestAbility");
        }
    }
};

class MissionListManagerInterface {
public:
    void GetActiveAbilityList(int32_t uid, std::vector<std::string> &abilityList)
    {
        if (MockFlag::hasRunningUIAbility) {
            abilityList.push_back("TestAbility");
        }
    }
};

class UIExtensionAbilityManager {
public:
    void GetActiveUIExtensionListByUid(int32_t uid, std::vector<std::string> &extensionList)
    {
        if (MockFlag::hasRunningUIExtension) {
            extensionList.push_back("TestExtension");
        }
    }
};

class AbilityManagerService {
    DECLARE_DELAYED_SINGLETON(AbilityManagerService);
public:
    std::shared_ptr<UIAbilityLifecycleManager> GetUIAbilityManagerByUserId(int32_t userId)
    {
        if (MockFlag::uiAbilityMgrNull) {
            return nullptr;
        }
        static auto mgr = std::make_shared<UIAbilityLifecycleManager>();
        return mgr;
    }
    std::shared_ptr<MissionListManagerInterface> GetMissionListManagerByUserId(int32_t userId)
    {
        if (MockFlag::missionListMgrNull) {
            return nullptr;
        }
        static auto mgr = std::make_shared<MissionListManagerInterface>();
        return mgr;
    }
    std::shared_ptr<UIExtensionAbilityManager> GetUIExtensionAbilityManagerByUserId(int32_t userId)
    {
        if (MockFlag::uiExtMgrNull) {
            return nullptr;
        }
        static auto mgr = std::make_shared<UIExtensionAbilityManager>();
        return mgr;
    }
};

} // namespace AAFwk
} // namespace OHOS

#endif // MOCK_ABILITY_MANAGER_SERVICE_H
