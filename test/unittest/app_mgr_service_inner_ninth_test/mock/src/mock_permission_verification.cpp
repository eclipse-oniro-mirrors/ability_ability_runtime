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

#include "hilog_tag_wrapper.h"
#include "permission_verification.h"
#include "mock_my_status.h"

namespace OHOS {
namespace AAFwk {

bool PermissionVerification::VerifyCallingPermission(const std::string &permissionName,
    const uint32_t specifyTokenId) const
{
    return AAFwk::MyStatus::GetInstance().verifyCallingPermission_;
}
bool PermissionVerification::IsSACall() const
{
    return AAFwk::MyStatus::GetInstance().isSACall_;
}
bool PermissionVerification::IsShellCall() const
{
    return AAFwk::MyStatus::GetInstance().isShellCall_;
}
bool PermissionVerification::CheckSpecificSystemAbilityAccessPermission(const std::string &processName) const
{
    return AAFwk::MyStatus::GetInstance().checkSpecific_;
}
bool PermissionVerification::VerifyRunningInfoPerm() const
{
    return AAFwk::MyStatus::GetInstance().verifyRunningInfoPerm_;
}
bool PermissionVerification::VerifyControllerPerm() const
{
    return false;
}
#ifdef WITH_DLP
bool PermissionVerification::VerifyDlpPermission(Want &want) const
{
    return false;
}
#endif // WITH_DLP
int PermissionVerification::VerifyAccountPermission() const
{
    return false;
}
bool PermissionVerification::VerifyMissionPermission() const
{
    return false;
}
int PermissionVerification::VerifyAppStateObserverPermission() const
{
    return false;
}
int32_t PermissionVerification::VerifyUpdateConfigurationPerm() const
{
    return false;
}
bool PermissionVerification::VerifyInstallBundlePermission() const
{
    return false;
}
bool PermissionVerification::VerifyGetBundleInfoPrivilegedPermission() const
{
    return false;
}
int PermissionVerification::CheckCallDataAbilityPermission(const VerificationInfo &verificationInfo,
    bool isShell) const
{
    return 0;
}
int PermissionVerification::CheckCallServiceAbilityPermission(
    const VerificationInfo &verificationInfo, uint32_t specifyTokenId) const
{
    return 0;
}
int PermissionVerification::CheckCallAbilityPermission(const VerificationInfo &verificationInfo,
    bool isCallByShortcut) const
{
    return 0;
}
int PermissionVerification::CheckCallServiceExtensionPermission(const VerificationInfo &verificationInfo) const
{
    return false;
}
int PermissionVerification::CheckStartByCallPermission(const VerificationInfo &verificationInfo) const
{
    return false;
}
unsigned int PermissionVerification::GetCallingTokenID() const
{
    return false;
}
bool PermissionVerification::JudgeStartInvisibleAbility(const uint32_t accessTokenId, const bool visible,
    const uint32_t specifyTokenId) const
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
int PermissionVerification::JudgeInvisibleAndBackground(const VerificationInfo &verificationInfo,
    bool isCallByShortcut) const
{
    return 0;
}
bool PermissionVerification::JudgeCallerIsAllowedToUseSystemAPI() const
{
    return AAFwk::MyStatus::GetInstance().judgeCallerIsAllowed_;
}
bool PermissionVerification::JudgeCallerIsAllowedToUseSystemAPIByTokenId(uint64_t specifiedFullTokenId) const
{
    return true;
}
bool PermissionVerification::IsSystemAppCall() const
{
    return true;
}
int32_t PermissionVerification::VerifyUpdateAPPConfigurationPerm() const
{
    return AAFwk::MyStatus::GetInstance().verifyUpdateAPPConfigurationPerm_;
}
bool PermissionVerification::VerifySuperviseKiaServicePermission() const
{
    return AAFwk::MyStatus::GetInstance().verifySuperviseKiaServicePermission_;
}
}  // namespace AAFwk
}  // namespace OHOS