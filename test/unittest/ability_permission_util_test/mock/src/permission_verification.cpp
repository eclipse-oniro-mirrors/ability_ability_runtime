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

#include "permission_verification.h"

#include "accesstoken_kit.h"
#include "mock_my_flag.h"

namespace OHOS {
namespace AAFwk {
bool PermissionVerification::retJudgeCallerIsAllowedToUseSystemAPI = false;
bool PermissionVerification::retVerifyStartRecentAbilityPermission = false;
bool PermissionVerification::retVerifyPrepareTerminatePermission = false;
bool PermissionVerification::retVerifyStartSelfUIAbility = false;

PermissionVerification::PermissionVerification() {}

PermissionVerification::~PermissionVerification() {}

std::shared_ptr<PermissionVerification> PermissionVerification::GetInstance()
{
    static std::shared_ptr<PermissionVerification> instance = std::make_shared<PermissionVerification>();
    return instance;
}

bool PermissionVerification::IsSACall() const
{
    return (MyFlag::flag_ & MyFlag::FLAG::IS_SA_CALL);
}

bool PermissionVerification::IsShellCall() const
{
    return (MyFlag::flag_ & MyFlag::FLAG::IS_SHELL_CALL);
}

bool PermissionVerification::JudgeCallerIsAllowedToUseSystemAPI() const
{
    return retJudgeCallerIsAllowedToUseSystemAPI;
}

bool PermissionVerification::JudgeCallerIsAllowedToUseSystemAPIByTokenId(uint64_t specifiedFullTokenId) const
{
    return true;
}

bool PermissionVerification::VerifyStartRecentAbilityPermission() const
{
    return retVerifyStartRecentAbilityPermission;
}

bool PermissionVerification::VerifyPrepareTerminatePermission(int32_t tokenId) const
{
    return retVerifyPrepareTerminatePermission;
}

bool PermissionVerification::VerifyStartSelfUIAbility(int32_t tokenId) const
{
    return retVerifyStartSelfUIAbility;
}

bool PermissionVerification::VerifyPermissionByTokenId(const int &tokenId, const std::string &permissionName) const
{
    return Security::AccessToken::AccessTokenKit::VerifyAccessToken(tokenId, permissionName, false) ==
        Security::AccessToken::PermissionState::PERMISSION_GRANTED;
}
}  // namespace AAFwk
}  // namespace OHOS
