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

#include "permission_verification.h"

namespace OHOS {
namespace AAFwk {

bool g_mockGrantFoundationPermission = false;

bool PermissionVerification::VerifyCallingPermission(const std::string &permissionName,
    const uint32_t specifyTokenId) const
{
    return false;
}
bool PermissionVerification::IsSACall() const { return false; }
bool PermissionVerification::IsSACallByTokenId(uint32_t callerTokenId) const { return false; }
bool PermissionVerification::IsShellCall() const { return false; }
bool PermissionVerification::IsShellCallByTokenId(uint32_t callerTokenId) const { return false; }

bool PermissionVerification::CheckSpecificSystemAbilityAccessPermission(const std::string &processName) const
{
    return g_mockGrantFoundationPermission;
}

bool PermissionVerification::VerifyRunningInfoPerm() const { return false; }
bool PermissionVerification::VerifyControllerPerm() const { return false; }
bool PermissionVerification::VerifyDlpPermission(Want &want) const { return false; }
int PermissionVerification::VerifyAccountPermission() const { return 0; }
bool PermissionVerification::VerifyMissionPermission() const { return false; }
int PermissionVerification::VerifyAppStateObserverPermission() const { return 0; }
int32_t PermissionVerification::VerifyUpdateConfigurationPerm() const { return 0; }
bool PermissionVerification::VerifyInstallBundlePermission() const { return false; }
bool PermissionVerification::VerifyGetBundleInfoPrivilegedPermission() const { return false; }
int PermissionVerification::CheckCallDataAbilityPermission(const VerificationInfo &verificationInfo, bool isShell) const
{
    return 0;
}
int PermissionVerification::CheckCallServiceAbilityPermission(const VerificationInfo &verificationInfo) const
{
    return 0;
}
int PermissionVerification::CheckCallAbilityPermission(const VerificationInfo &verificationInfo) const { return 0; }
int PermissionVerification::CheckCallServiceExtensionPermission(const VerificationInfo &verificationInfo) const
{
    return 0;
}
int PermissionVerification::CheckStartByCallPermission(const VerificationInfo &verificationInfo) const { return 0; }
unsigned int PermissionVerification::GetCallingTokenID() const { return 0; }
bool PermissionVerification::JudgeStartInvisibleAbility(const uint32_t accessTokenId, const bool visible) const
{
    return false;
}
bool PermissionVerification::JudgeStartAbilityFromBackground(const bool isBackgroundCall,
    const uint32_t specifyTokenId) const
{
    return false;
}
bool PermissionVerification::JudgeAssociatedWakeUp(const uint32_t accessTokenId, const bool associatedWakeUp) const
{
    return false;
}
int PermissionVerification::JudgeInvisibleAndBackground(const VerificationInfo &verificationInfo) const { return 0; }
bool PermissionVerification::JudgeCallerIsAllowedToUseSystemAPI() const { return false; }
bool PermissionVerification::JudgeCallerIsAllowedToUseSystemAPIByTokenId(uint64_t specifiedFullTokenId) const
{
    return true;
}
bool PermissionVerification::IsSystemAppCall() const { return false; }

}  // namespace AAFwk
}  // namespace OHOS
