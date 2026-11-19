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

#include "extension_query_event_util.h"

#include <string>

#include "ability_util.h"
#include "event_report.h"
#include "extension_ability_info.h"
#include "in_process_call_wrapper.h"
#include "ipc_skeleton.h"

namespace OHOS {
namespace AAFwk {
void ExtensionQueryEventUtil::ReportExtensionQueryMultiResult(
    const std::vector<AppExecFwk::ExtensionAbilityInfo> &extensionInfos, bool isSandbox)
{
    if (extensionInfos.size() <= 1) {
        return;
    }
    if (extensionInfos[0].applicationInfo.isSystemApp) {
        return;
    }
    bool hasSystem = false;
    std::string bundleNames;
    std::string abilityNames;
    for (const auto &info : extensionInfos) {
        if (!bundleNames.empty()) {
            bundleNames += "-";
            abilityNames += "-";
        }
        bundleNames += info.bundleName;
        abilityNames += info.name;
        if (info.applicationInfo.isSystemApp) {
            hasSystem = true;
        }
    }
    if (!hasSystem) {
        return;
    }
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    std::string callerBundleName;
    auto bms = AbilityUtil::GetBundleManagerHelper();
    if (bms != nullptr) {
        IN_PROCESS_CALL_WITHOUT_RET(bms->GetNameForUid(callingUid, callerBundleName));
    }
    EventInfo eventInfo;
    eventInfo.moduleName = isSandbox ? "GetSandboxExtAbilityInfos" : "QueryExtensionAbilityInfos";
    eventInfo.bundleName = bundleNames;
    eventInfo.abilityName = abilityNames;
    eventInfo.extensionType = static_cast<int32_t>(extensionInfos[0].type);
    eventInfo.callerBundleName = callerBundleName.empty() ? std::to_string(callingUid) : callerBundleName;
    EventReport::SendStartAbilityOtherExtensionEvent(EventName::START_ABILITY_OTHER_EXTENSION, eventInfo);
}
}
}
