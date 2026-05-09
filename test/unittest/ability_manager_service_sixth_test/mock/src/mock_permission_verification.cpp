/*
 * Copyright (c) 2024 - 2025 Huawei Device Co., Ltd.
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

#include "hilog_tag_wrapper.h"
#include "mock_my_flag.h"

namespace OHOS {
namespace AAFwk {

bool PermissionVerification::VerifyPermissionByTokenId(const int &tokenId, const std::string &permissionName) const
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s enter", __func__);
    return !!(MyFlag::flag_);
}

bool PermissionVerification::VerifyCallingPermission(
    const std::string &permissionName, const uint32_t specifyTokenId) const
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s enter", __func__);
    return !!(MyFlag::flag_);
}

bool PermissionVerification::IsSACall() const
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s enter", __func__);
    return (MyFlag::flag_ & MyFlag::FLAG::IS_SA_CALL);
}

bool PermissionVerification::IsSACallByTokenId(uint32_t callerTokenId) const
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s enter", __func__);
    return !!(MyFlag::flag_);
}

bool PermissionVerification::IsShellCall() const
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s enter", __func__);
    return (MyFlag::flag_ & MyFlag::FLAG::IS_SHELL_CALL);
}

bool PermissionVerification::IsShellCallByTokenId(uint32_t callerTokenId) const
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s enter", __func__);
    return !!(MyFlag::flag_);
}

bool PermissionVerification::CheckSpecificSystemAbilityAccessPermission(const std::string &processName) const
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s enter", __func__);
    return !!(MyFlag::flag_);
}

bool PermissionVerification::CheckObserverCallerPermission() const
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s enter", __func__);
    return !!(MyFlag::flag_);
}
bool PermissionVerification::VerifyRunningInfoPerm() const
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s enter", __func__);
    return !!(MyFlag::flag_);
}

bool PermissionVerification::VerifyCustomSandbox(uint32_t accessTokenId) const
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s enter", __func__);
    return !!(MyFlag::flag_);
}

bool PermissionVerification::VerifyControllerPerm() const
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s enter", __func__);
    return !!(MyFlag::flag_);
}

#ifdef WITH_DLP
bool PermissionVerification::VerifyDlpPermission(Want &want) const
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s enter", __func__);
    return !!(MyFlag::flag_);
}
#endif // WITH_DLP
int PermissionVerification::VerifyAccountPermission() const
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s enter", __func__);
    return MyFlag::flag_;
}

bool PermissionVerification::VerifyMissionPermission() const
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s enter", __func__);
    return !!(MyFlag::flag_);
}

bool PermissionVerification::VerifyContinuationNotifyPermission() const
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s enter", __func__);
    return !!(MyFlag::flag_);
}

int PermissionVerification::VerifyAppStateObserverPermission() const
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s enter", __func__);
    return MyFlag::flag_;
}

int32_t PermissionVerification::VerifyUpdateConfigurationPerm() const
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s enter", __func__);
    return static_cast<int32_t>(MyFlag::flag_);
}

bool PermissionVerification::VerifyInstallBundlePermission() const
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s enter", __func__);
    return !!(MyFlag::flag_);
}

bool PermissionVerification::VerifyGetBundleInfoPrivilegedPermission() const
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s enter", __func__);
    return !!(MyFlag::flag_);
}

bool PermissionVerification::VerifyStartRecentAbilityPermission() const
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s enter", __func__);
    return !!(MyFlag::flag_);
}

int PermissionVerification::CheckCallDataAbilityPermission(const VerificationInfo &verificationInfo, bool isShell) const
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s enter", __func__);
    return MyFlag::flag_;
}

int PermissionVerification::CheckCallServiceAbilityPermission(
    const VerificationInfo &verificationInfo, uint32_t specifyTokenId) const
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s enter", __func__);
    return MyFlag::flag_;
}

int PermissionVerification::CheckCallAbilityPermission(const VerificationInfo &verificationInfo,
    bool isCallByShortcut) const
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s enter", __func__);
    return MyFlag::flag_;
}

int PermissionVerification::CheckCallServiceExtensionPermission(const VerificationInfo &verificationInfo) const
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s enter", __func__);
    return MyFlag::flag_;
}

int PermissionVerification::CheckStartByCallPermission(const VerificationInfo &verificationInfo) const
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s enter", __func__);
    return MyFlag::flag_;
}

unsigned int PermissionVerification::GetCallingTokenID() const
{
    TAG_LOGI(AAFwkTag::TEST, "PermissionVerification::GetCallingTokenID");
    return static_cast<unsigned int>(MyFlag::flag_);
}

bool PermissionVerification::JudgeStartInvisibleAbility(const uint32_t accessTokenId, const bool visible,
    const uint32_t specifyTokenId) const
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s enter", __func__);
    return !!(MyFlag::flag_);
}

bool PermissionVerification::JudgeStartAbilityFromBackground(const bool isBackgroundCall) const
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s enter", __func__);
    return !!(MyFlag::flag_);
}

bool PermissionVerification::JudgeAssociatedWakeUp(const uint32_t accessTokenId, const bool associatedWakeUp) const
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s enter", __func__);
    return !!(MyFlag::flag_);
}

int PermissionVerification::JudgeInvisibleAndBackground(const VerificationInfo &verificationInfo,
    bool isCallByShortcut) const
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s enter", __func__);
    return MyFlag::flag_;
}

bool PermissionVerification::JudgeCallerIsAllowedToUseSystemAPI() const
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s enter", __func__);
    return MyFlag::systemAppFlag_;
}

bool PermissionVerification::JudgeCallerIsAllowedToUseSystemAPIByTokenId(uint64_t specifiedFullTokenId) const
{
    return true;
}

bool PermissionVerification::VerifyPrepareTerminatePermission() const
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s enter", __func__);
    return true;
}

bool PermissionVerification::VerifyPrepareTerminatePermission(const int &tokenId) const
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s enter", __func__);
    return true;
}

bool PermissionVerification::VerifyShellStartExtensionType(int32_t type, uint32_t specifyTokenId) const
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s enter", __func__);
    return MyFlag::flag_;
}

bool PermissionVerification::VerifyPreloadApplicationPermission() const
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s enter", __func__);
    return MyFlag::flag_;
}

bool PermissionVerification::VerifyPreStartAtomicServicePermission() const
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s enter", __func__);
    return MyFlag::flag_;
}

bool PermissionVerification::IsSystemAppCall() const
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s enter", __func__);
    return true;
}

bool PermissionVerification::IsSystemAppCallByTokenId(uint32_t callerTokenId) const
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s enter", __func__);
    return true;
}

bool PermissionVerification::VerifyBackgroundCallPermission(const bool isBackgroundCall) const
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s enter", __func__);
    return MyFlag::flag_;
}

bool PermissionVerification::VerifyKillProcessDependedOnWebPermission() const
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s enter", __func__);
    return true;
}

bool PermissionVerification::VerifyBlockAllAppStartPermission() const
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s enter", __func__);
    return true;
}

bool PermissionVerification::VerifyStartUIAbilityToHiddenPermission() const
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s enter", __func__);
    return true;
}

bool PermissionVerification::VerifySuperviseKiaServicePermission() const
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s enter", __func__);
    return true;
}

bool PermissionVerification::VerifyStartLocalDebug(int32_t tokenId) const
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s enter", __func__);
    return true;
}

bool PermissionVerification::VerifyStartSelfUIAbility(int tokenId) const
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s enter", __func__);
    return true;
}

bool PermissionVerification::VerifyFusionAccessPermission() const
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s enter", __func__);
    return true;
}

int PermissionVerification::CheckCallModularObjectExtensionPermission(
    const VerificationInfo &verificationInfo) const
{
    return ERR_OK;
}
}  // namespace AAFwk
}  // namespace OHOS