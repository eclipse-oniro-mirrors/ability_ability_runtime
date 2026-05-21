/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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
        uint32_t accessTokenId = 0;
        int32_t apiTargetVersion = 0;
    };
    static PermissionVerification &GetInstance()
    {
        static PermissionVerification instance;
        return instance;
    };
    PermissionVerification() = default;
    ~PermissionVerification() = default;

    bool VerifyCallingPermission(const std::string &permissionName, const uint32_t specifyTokenId = 0) const;

    bool IsSACall() const;

    bool IsShellCall() const;

    bool CheckSpecificSystemAbilityAccessPermission() const;

    bool VerifyRunningInfoPerm() const;

    bool VerifyControllerPerm() const;

    bool VerifyDlpPermission(Want &want) const;

    int VerifyAccountPermission() const;

    bool VerifyMissionPermission() const;

    int VerifyAppStateObserverPermission() const;

    int32_t VerifyUpdateConfigurationPerm() const;

    bool VerifyInstallBundlePermission() const;

    bool VerifyGetBundleInfoPrivilegedPermission() const;

    int CheckCallDataAbilityPermission(const VerificationInfo &verificationInfo, bool isShell) const;

    int CheckCallServiceAbilityPermission(const VerificationInfo &verificationInfo) const;

    int CheckCallAbilityPermission(const VerificationInfo &verificationInfo) const;

    int CheckCallServiceExtensionPermission(const VerificationInfo &verificationInfo) const;

    int CheckStartByCallPermission(const VerificationInfo &verificationInfo) const;

    unsigned int GetCallingTokenID() const;

    bool JudgeStartInvisibleAbility(const uint32_t accessTokenId, const bool visible) const;

    bool JudgeStartAbilityFromBackground(const bool isBackgroundCall, const uint32_t specifyTokenId = 0) const;

    bool JudgeAssociatedWakeUp(const uint32_t accessTokenId, const bool associatedWakeUp) const;

    int JudgeInvisibleAndBackground(const VerificationInfo &verificationInfo) const;

    inline bool IsCallFromSameAccessToken(const uint32_t accessTokenId) const
    {
        return IPCSkeleton::GetCallingTokenID() == accessTokenId;
    }

    bool JudgeCallerIsAllowedToUseSystemAPI() const;
    bool JudgeCallerIsAllowedToUseSystemAPIByTokenId(uint64_t specifiedFullTokenId) const;
    bool IsSystemAppCall() const;
};
} // namespace AAFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_PERMISSION_VERIFICATION_H