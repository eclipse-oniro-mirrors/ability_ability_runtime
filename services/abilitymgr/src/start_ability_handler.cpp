/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#include "start_ability_handler.h"

#include "permission_verification.h"
#ifdef WITH_DLP
#include "dlp_utils.h"
#endif // WITH_DLP
#include "global_constant.h"

namespace OHOS {
namespace AAFwk {
bool StartAbilityParams::IsCallerSandboxApp()
{
    return AbilityRuntime::GlobalConstant::IsDlpIndex(GetCallerAppIndex());
}

#ifdef WITH_DLP
bool StartAbilityParams::OtherAppsAccessDlp()
{
    if (otherAppsAccessDlp.has_value()) {
        return otherAppsAccessDlp.value();
    }
    otherAppsAccessDlp = DlpUtils::OtherAppsAccessDlpCheck(callerToken, want);
    return otherAppsAccessDlp.value();
}

bool StartAbilityParams::DlpAccessOtherApps()
{
    if (dlpAccessOtherApps.has_value()) {
        return dlpAccessOtherApps.value();
    }
    dlpAccessOtherApps = DlpUtils::DlpAccessOtherAppsCheck(callerToken, want);
    return dlpAccessOtherApps.value();
}

bool StartAbilityParams::SandboxExternalAuth()
{
    if (sandboxExternalAuth.has_value()) {
        return sandboxExternalAuth.value();
    }
    auto record = GetCallerRecord();
    if (!record) {
        sandboxExternalAuth = false;
        return false;
    }
    sandboxExternalAuth = DlpUtils::SandboxAuthCheck(*record, want);
    return sandboxExternalAuth.value();
}
#endif // WITH_DLP

bool StartAbilityParams::IsCallerSysApp()
{
    if (isCallerSysApp.has_value()) {
        return isCallerSysApp.value();
    }
    isCallerSysApp = PermissionVerification::GetInstance()->IsSystemAppCall();
    return isCallerSysApp.value();
}

std::shared_ptr<AbilityRecord> StartAbilityParams::GetCallerRecord()
{
    if (callerRecord.has_value()) {
        return callerRecord.value();
    }

    if (callerToken) {
        callerRecord = Token::GetAbilityRecordByToken(callerToken);
    } else {
        callerRecord = nullptr;
    }
    return callerRecord.value();
}

int32_t StartAbilityParams::GetCallerAppIndex()
{
    if (callerAppIndex.has_value()) {
        return callerAppIndex.value();
    }
    auto record = GetCallerRecord();
    callerAppIndex = record ? record->GetAppIndex() : 0;
    return callerAppIndex.value();
}

EventInfo StartAbilityParams::BuildEventInfo()
{
    EventInfo eventInfo;
    eventInfo.userId = userId;
    eventInfo.bundleName = want.GetBundleNameRef();
    eventInfo.moduleName = want.GetModuleNameRef();
    eventInfo.abilityName = want.GetAbilityNameRef();
    return eventInfo;
}
} // namespace AAFwk
} // namespace OHOS