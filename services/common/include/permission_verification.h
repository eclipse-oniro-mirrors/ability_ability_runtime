/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_PERMISSION_VERIFICATION_H
#define OHOS_ABILITY_RUNTIME_PERMISSION_VERIFICATION_H

#include "ipc_skeleton.h"
#include "singleton.h"
#include "want.h"

namespace OHOS {
namespace AAFwk {
class PermissionVerification : public DelayedSingleton<PermissionVerification> {
public:
struct VerificationInfo {
    bool visible = false;
    bool isBackgroundCall = true;
    bool associatedWakeUp = false;
    bool withContinuousTask = false;
    uint32_t accessTokenId = 0;
    int32_t apiTargetVersion = 0;
    uint32_t specifyTokenId = 0;
};

    PermissionVerification() = default;
    ~PermissionVerification() = default;

    bool VerifyPermissionByTokenId(const int &tokenId, const std::string &permissionName) const;

    bool VerifyCallingPermission(const std::string &permissionName, const uint32_t specifyTokenId = 0) const;

    bool IsSACall() const;

    bool IsSACallByTokenId(uint32_t callerTokenId) const;

    bool IsShellCall() const;

    bool IsShellCallByTokenId(uint32_t callerTokenId) const;

    bool CheckSpecificSystemAbilityAccessPermission(const std::string &processName) const;

    bool CheckObserverCallerPermission() const;

    bool VerifyRunningInfoPerm() const;

    bool VerifyControllerPerm() const;

    bool VerifyDlpPermission(Want &want) const;

    int VerifyAccountPermission() const;

    bool VerifyMissionPermission() const;

    int VerifyAppStateObserverPermission() const;

    int32_t VerifyUpdateConfigurationPerm() const;

    int32_t VerifyUpdateAPPConfigurationPerm() const;

    bool VerifyInstallBundlePermission() const;

    bool VerifyGetBundleInfoPrivilegedPermission() const;

    bool VerifyStartRecentAbilityPermission() const;

    int CheckCallDataAbilityPermission(const VerificationInfo &verificationInfo, bool isShell) const;

    int CheckCallServiceAbilityPermission(const VerificationInfo &verificationInfo, uint32_t specifyTokenId = 0) const;

    int CheckCallAbilityPermission(const VerificationInfo &verificationInfo, bool isCallByShortcut = false) const;

    /**
     * Check if Caller is allowed to start ServiceExtension(Stage) or DataShareExtension(Stage)
     *
     * @param verificationInfo, verificationInfo.
     * @return Returns ERR_OK on check success, others on check failure.
     */
    int CheckCallServiceExtensionPermission(const VerificationInfo &verificationInfo) const;

    int CheckStartByCallPermission(const VerificationInfo &verificationInfo) const;

    bool JudgeCallerIsAllowedToUseSystemAPI() const;

    bool JudgeCallerIsAllowedToUseSystemAPIByTokenId(uint64_t specifiedFullTokenId) const;

    bool IsSystemAppCall() const;

    bool IsSystemAppCallByTokenId(uint32_t callerTokenId) const;

    bool VerifyPrepareTerminatePermission() const;

    bool VerifyPrepareTerminatePermission(const int &tokenId) const;

    bool VerifyShellStartExtensionType(int32_t type, uint32_t specifyTokenId = 0) const;

    bool VerifyPreloadApplicationPermission() const;

    bool VerifyPreStartAtomicServicePermission() const;

    bool VerifyKillProcessDependedOnWebPermission() const;

    bool VerifyBackgroundCallPermission(const bool isBackgroundCall) const;

    bool VerifyBlockAllAppStartPermission() const;

    bool VerifyStartUIAbilityToHiddenPermission() const;

    bool VerifySuperviseKiaServicePermission() const;

    bool VerifyStartLocalDebug(int32_t tokenId) const;

    bool VerifyStartSelfUIAbility(int tokenId) const;

    bool VerifyFusionAccessPermission() const;

    bool VerifyCustomSandbox(uint32_t accessTokenId) const;

private:
    DISALLOW_COPY_AND_MOVE(PermissionVerification);

    constexpr static int32_t API8 = 8;

    unsigned int GetCallingTokenID() const;

    bool JudgeStartInvisibleAbility(const uint32_t accessTokenId, const bool visible,
        const uint32_t specifyTokenId = 0) const;

    bool JudgeStartAbilityFromBackground(const bool isBackgroundCall) const;

    bool JudgeAssociatedWakeUp(const uint32_t accessTokenId, const bool associatedWakeUp) const;

    int JudgeInvisibleAndBackground(const VerificationInfo &verificationInfo, bool isCallByShortcut = false) const;

    inline bool IsCallFromSameAccessToken(const uint32_t accessTokenId) const
    {
        return IPCSkeleton::GetCallingTokenID() == accessTokenId;
    }
};
}  // namespace AAFwk
}  // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_PERMISSION_VERIFICATION_H
