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

#include "mock_permission_verification.h"

namespace OHOS {
namespace AAFwk {

bool PermissionVerification::VerifyCallingPermission(const std::string &permissionName,
    const uint32_t specifyTokenId) const
{
    return true;
}
bool PermissionVerification::IsSACall() const
{
    return true;
}
bool PermissionVerification::IsShellCall() const
{
    return true;
}
bool PermissionVerification::CheckSpecificSystemAbilityAccessPermission() const
{
    return true;
}
bool PermissionVerification::VerifyRunningInfoPerm() const
{
    return true;
}
bool PermissionVerification::VerifyControllerPerm() const
{
    return true;
}
bool PermissionVerification::VerifyDlpPermission(Want &want) const
{
    return true;
}
int PermissionVerification::VerifyAccountPermission() const
{
    return 0;
}
bool PermissionVerification::VerifyMissionPermission() const
{
    return true;
}
int PermissionVerification::VerifyAppStateObserverPermission() const
{
    return 0;
}
int32_t PermissionVerification::VerifyUpdateConfigurationPerm() const
{
    return 0;
}
bool PermissionVerification::VerifyInstallBundlePermission() const
{
    return true;
}
bool PermissionVerification::VerifyGetBundleInfoPrivilegedPermission() const
{
    return true;
}
int PermissionVerification::CheckCallDataAbilityPermission(const VerificationInfo &verificationInfo, bool isShell) const
{
    return 0;
}
int PermissionVerification::CheckCallServiceAbilityPermission(const VerificationInfo &verificationInfo) const
{
    return 0;
}
int PermissionVerification::CheckCallAbilityPermission(const VerificationInfo &verificationInfo) const
{
    return 0;
}
int PermissionVerification::CheckCallServiceExtensionPermission(const VerificationInfo &verificationInfo) const
{
    return 0;
}
int PermissionVerification::CheckStartByCallPermission(const VerificationInfo &verificationInfo) const
{
    return 0;
}
unsigned int PermissionVerification::GetCallingTokenID() const
{
    return 0;
}
bool PermissionVerification::JudgeStartInvisibleAbility(const uint32_t accessTokenId, const bool visible) const
{
    return true;
}
bool PermissionVerification::JudgeStartAbilityFromBackground(const bool isBackgroundCall,
    const uint32_t specifyTokenId) const
{
    return true;
}
bool PermissionVerification::JudgeAssociatedWakeUp(const uint32_t accessTokenId, const bool associatedWakeUp) const
{
    return true;
}
int PermissionVerification::JudgeInvisibleAndBackground(const VerificationInfo &verificationInfo) const
{
    return 0;
}
bool PermissionVerification::JudgeCallerIsAllowedToUseSystemAPI() const
{
    return true;
}
bool PermissionVerification::JudgeCallerIsAllowedToUseSystemAPIByTokenId(uint64_t specifiedFullTokenId) const
{
    return true;
}
bool PermissionVerification::IsSystemAppCall() const
{
    return MyFlag::isSystemAppCallRet;
}
} // namespace AAFwk
} // namespace OHOS
