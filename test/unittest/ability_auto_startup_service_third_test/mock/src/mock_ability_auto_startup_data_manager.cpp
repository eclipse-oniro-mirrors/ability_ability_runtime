/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#include "mock_ability_auto_startup_data_manager.h"

namespace OHOS {
namespace AbilityRuntime {
AbilityAutoStartupDataManager::AbilityAutoStartupDataManager() {}

AbilityAutoStartupDataManager::~AbilityAutoStartupDataManager() {}

int32_t AbilityAutoStartupDataManager::InsertAutoStartupData(
    const AutoStartupInfo &info, bool isAutoStartup, bool isEdmForce)
{
    if (info.userId == -1) {
        return ERR_INVALID_VALUE;
    }
    return ERR_OK;
}

int32_t AbilityAutoStartupDataManager::UpdateAutoStartupData(
    const AutoStartupInfo &info, DistributedKv::Key &originKey, bool isAutoStartup, bool isEdmForce)
{
    if (info.userId == -1) {
        return ERR_INVALID_VALUE;
    }
    return ERR_OK;
}

int32_t AbilityAutoStartupDataManager::DeleteAutoStartupData(const AutoStartupInfo &info,
    DistributedKv::Key &originKey)
{
    if (info.userId == -1) {
        return ERR_INVALID_VALUE;
    }
    return ERR_OK;
}

int32_t AbilityAutoStartupDataManager::DeleteAutoStartupData(const std::string &bundleName, int32_t accessTokenId)
{
    return ERR_OK;
}

AutoStartupStatus AbilityAutoStartupDataManager::QueryAutoStartupData(const AutoStartupInfo &info,
    DistributedKv::Key &originKey)
{
    AutoStartupStatus startupStatus;
    if (info.bundleName == BUNDLENAME_NO_FONUD) {
        startupStatus.code = ERR_NAME_NOT_FOUND;
    } else if (info.bundleName == BUNDLENAME_FONUD && info.canUserModify == false) {
        startupStatus.isEdmForce = true;
        startupStatus.code = ERR_OK;
    } else if (info.bundleName == BUNDLENAME_FONUD && info.abilityName == ABILITYNAME_AUTO_START) {
        startupStatus.isAutoStartup = true;
        startupStatus.setterUserId = DEFAULT_USERID;
        startupStatus.setterType = AutoStartupSetterType::USER;
        startupStatus.code = ERR_OK;
    } else if (info.bundleName == BUNDLENAME_FONUD && info.abilityName == ABILITYNAME_NOT_AUTO_START) {
        startupStatus.isAutoStartup = false;
        startupStatus.setterUserId = DEFAULT_USERID;
        startupStatus.code = ERR_OK;
    } else if (info.bundleName == BUNDLENAME_FONUD && info.abilityName == ABILITYNAME_AUTO_START_BY_EDM) {
        startupStatus.isAutoStartup = true;
        startupStatus.setterType = AutoStartupSetterType::SYSTEM;
        startupStatus.code = ERR_OK;
    }
    return startupStatus;
}

int32_t AbilityAutoStartupDataManager::QueryAllAutoStartupApplications(std::vector<AutoStartupInfo> &infoList,
    int32_t userId, bool isCalledByEDM)
{
    return ERR_OK;
}

int32_t AbilityAutoStartupDataManager::ResetAutoStartupDirectBoot(const AutoStartupInfo &info)
{
    return info.userId == -1 ? ERR_INVALID_VALUE : ERR_OK;
}

int32_t AbilityAutoStartupDataManager::GetCurrentAppAutoStartupData(
    const std::string &bundleName, std::vector<AutoStartupInfo> &infoList, const std::string &accessTokenId)
{
    if (bundleName == "bundleNameTest" || bundleName == "hapModuleInfosModuleNameIsEmpty") {
        AutoStartupInfo info;
        info.bundleName = "bundleNameTest";
        info.abilityName = "nameTest";
        infoList.emplace_back(info);
        return ERR_OK;
    }
    return ERR_NO_INIT;
}
} // namespace AbilityRuntime
} // namespace OHOS
