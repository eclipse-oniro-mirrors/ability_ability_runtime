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

#include "res_sched_util.h"

#include <string>

#include "ability_info.h"
#include "ui_extension_wrapper.h"
#include "hilog_tag_wrapper.h"
#include "mock_my_status.h"
#ifdef RESOURCE_SCHEDULE_SERVICE_ENABLE
#include "res_sched_client.h"
#include "res_type.h"
#endif

namespace OHOS {
namespace AAFwk {
using AssociatedStartType = ResourceSchedule::ResType::AssociatedStartType;
ResSchedUtil &ResSchedUtil::GetInstance()
{
    static ResSchedUtil instance;
    return instance;
}

int64_t ResSchedUtil::convertType(int64_t resSchedType)
{
    return -1;
}

void ResSchedUtil::ReportAbilityAssociatedStartInfoToRSS(
    const AbilityInfo &abilityInfo, int64_t resSchedType, int32_t callerUid, int32_t callerPid)
{
}

void ResSchedUtil::ReportPreloadApplicationToRSS(const std::shared_ptr<AbilityInfo>& abilityInfo, int32_t preloadMode)
{
}

void ResSchedUtil::ReportUIExtensionProcColdStartToRss(
    int32_t extensionAbilityType, int hostPid, const std::string& hostBundleName, const std::string& bundleName,
    const std::string& abilityName, const std::string& moduleName, bool isPreloadUIExtension)
{
}

std::string ResSchedUtil::GetThawReasonByAbilityType(const AbilityInfo &abilityInfo)
{
    return "";
}

void ResSchedUtil::ReportAbilityIntentExemptionInfoToRSS(int32_t callerUid, int32_t callerPid)
{
}

void ResSchedUtil::ReportEventToRSS(const int32_t uid, const std::string &bundleName, const std::string &reason,
    const int32_t pid, const int32_t callerPid, bool isCreateFormImage)
{
}

void ResSchedUtil::GetAllFrozenPidsFromRSS(std::unordered_set<int32_t> &frozenPids)
{
}

bool ResSchedUtil::CheckShouldForceKillProcess(int32_t pid, const std::string& bundleName)
{
    return AAFwk::MyStatus::GetInstance().isShouldKillProcess_;
}

void ResSchedUtil::ReportLoadingEventToRss(LoadingStage stage, int32_t pid, int32_t uid,
    int64_t timeDuration, int64_t abilityRecordId)
{
}

std::unordered_set<std::string> ResSchedUtil::GetNWebPreloadSet() const
{
    return std::unordered_set<std::string>();
}
} // namespace AAFwk
} // namespace OHOS
