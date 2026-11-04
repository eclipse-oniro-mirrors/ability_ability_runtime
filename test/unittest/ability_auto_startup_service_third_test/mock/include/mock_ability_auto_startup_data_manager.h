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

#ifndef OHOS_ABILITY_RUNTIME_ABILITY_AUTO_STARTUP_DATA_MANAGER_H
#define OHOS_ABILITY_RUNTIME_ABILITY_AUTO_STARTUP_DATA_MANAGER_H

#include <mutex>
#include <vector>

#include "auto_startup_info.h"
#include "singleton.h"
#include "types.h"

namespace {
const std::string ABILITYNAME_ = "";
const std::string BUNDLENAME_NO_FONUD = "bundleName_no_found";
const std::string BUNDLENAME_FONUD = "bundleName_found";
const std::string ABILITYNAME_AUTO_START = "abilityName_auto_start";
const std::string ABILITYNAME_AUTO_START_BY_EDM = "abilityName_auto_start_by_edm";
const std::string ABILITYNAME_NOT_AUTO_START = "abilityName_not_auto_start";
const int32_t DEFAULT_USERID = 100;
} // namespace

namespace OHOS {
namespace AbilityRuntime {
class AbilityAutoStartupDataManager : public DelayedSingleton<AbilityAutoStartupDataManager> {
public:
    AbilityAutoStartupDataManager();

    virtual ~AbilityAutoStartupDataManager();

    int32_t InsertAutoStartupData(const AutoStartupInfo &info, bool isAutoStartup, bool isEdmForce);

    int32_t UpdateAutoStartupData(const AutoStartupInfo &info, DistributedKv::Key &originKey,
        bool isAutoStartup, bool isEdmForce);

    int32_t DeleteAutoStartupData(const AutoStartupInfo &info, DistributedKv::Key &originKey);

    int32_t DeleteAutoStartupData(const std::string &bundleName, int32_t accessTokenId);

    AutoStartupStatus QueryAutoStartupData(const AutoStartupInfo &info, DistributedKv::Key &originKey);

    int32_t QueryAllAutoStartupApplications(std::vector<AutoStartupInfo> &infoList, int32_t userId, bool isCalledByEDM);

    int32_t GetCurrentAppAutoStartupData(const std::string &bundleName,
        std::vector<AutoStartupInfo> &infoList, const std::string &accessTokenId);

    int32_t ResetAutoStartupDirectBoot(const AutoStartupInfo &info);
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_ABILITY_AUTO_STARTUP_DATA_MANAGER_H