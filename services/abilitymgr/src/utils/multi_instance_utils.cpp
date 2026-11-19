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

#include "multi_instance_utils.h"

#include <vector>
#include <unordered_map>

#include "ability_util.h"
#include "app_mgr_util.h"
#include "scene_board_judgement.h"

namespace OHOS {
namespace AAFwk {
namespace {
constexpr const char* APP_INSTANCE_KEY_0 = "app_instance_0";
}
std::string MultiInstanceUtils::GetInstanceKey(const Want& want)
{
    return want.GetStringParam(Want::APP_INSTANCE_KEY);
}

std::string MultiInstanceUtils::GetValidExtensionInstanceKey(const AbilityRequest &abilityRequest)
{
    if (!IsSupportedExtensionType(abilityRequest.abilityInfo.extensionAbilityType)) {
        return APP_INSTANCE_KEY_0;
    }
    auto instanceKey = abilityRequest.want.GetStringParam(Want::APP_INSTANCE_KEY);
    if (instanceKey.empty()) {
        instanceKey = GetSelfCallerInstanceKey(abilityRequest);
        if (instanceKey.empty()) {
            return APP_INSTANCE_KEY_0;
        }
    }
    return instanceKey;
}

std::string MultiInstanceUtils::GetSelfCallerInstanceKey(const AbilityRequest &abilityRequest)
{
    auto callerRecord = Token::GetAbilityRecordByToken(abilityRequest.callerToken);
    if (callerRecord && callerRecord->GetAbilityInfo().bundleName == abilityRequest.want.GetBundleNameRef()) {
        return callerRecord->GetInstanceKey();
    }
    return "";
}

bool MultiInstanceUtils::IsDefaultInstanceKey(const std::string& key)
{
    return key == APP_INSTANCE_KEY_0;
}

bool MultiInstanceUtils::IsMultiInstanceApp(AppExecFwk::ApplicationInfo appInfo)
{
    if (appInfo.multiAppMode.multiAppModeType != AppExecFwk::MultiAppModeType::MULTI_INSTANCE) {
        return false;
    }
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        return false;
    }
    return true;
}

bool MultiInstanceUtils::IsSupportedExtensionType(AppExecFwk::ExtensionAbilityType type)
{
    static const std::unordered_set<AppExecFwk::ExtensionAbilityType> supportMultiInstanceExtensionSet {
        AppExecFwk::ExtensionAbilityType::WORK_SCHEDULER,
        AppExecFwk::ExtensionAbilityType::BACKUP,
        AppExecFwk::ExtensionAbilityType::SHARE
    };
    return supportMultiInstanceExtensionSet.find(type) != supportMultiInstanceExtensionSet.end();
}

bool MultiInstanceUtils::IsInstanceKeyExist(const std::string& bundleName, const std::string& key)
{
    auto appMgr = AppMgrUtil::GetAppMgr();
    CHECK_POINTER_AND_RETURN(appMgr, false);
    std::vector<std::string> instanceKeyArray;
    auto result = IN_PROCESS_CALL(appMgr->GetAllRunningInstanceKeysByBundleName(bundleName, instanceKeyArray));
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Failed to get instance key");
        return false;
    }
    for (const auto& item: instanceKeyArray) {
        if (item == key) {
            return true;
        }
    }
    return false;
}
}  // namespace AAFwk
}  // namespace OHOS