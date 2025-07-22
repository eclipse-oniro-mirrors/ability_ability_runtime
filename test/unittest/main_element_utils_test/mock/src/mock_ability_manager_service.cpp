/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "ability_manager_service.h"

#include "mock_my_status.h"

namespace OHOS {
namespace AAFwk {
AbilityManagerService::AbilityManagerService()
{}

AbilityManagerService::~AbilityManagerService()
{}

int32_t AbilityManagerService::StartAbility(const Want &want, const StartOptions &startOptions,
    sptr<IRemoteObject> callerToken, int32_t userId, int requestCode)
{
    return ERR_OK;
}

int32_t AbilityManagerService::UpdateKeepAliveEnableState(const std::string &bundleName,
    const std::string &moduleName, const std::string &mainElement, bool updateEnable, int32_t userId)
{
    return ERR_OK;
}

bool AbilityManagerService::GetDataAbilityUri(const std::vector<AppExecFwk::AbilityInfo> &abilityInfos,
    const std::string &mainAbility, std::string &uri)
{
    return true;
}
}  // namespace AAFwk
}  // namespace OHOS
