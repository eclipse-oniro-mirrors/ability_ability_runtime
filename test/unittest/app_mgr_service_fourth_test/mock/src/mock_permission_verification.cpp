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
#include "mock_my_status.h"

namespace OHOS {
namespace AAFwk {

bool PermissionVerification::IsSACall() const
{
    return MyStatus::GetInstance().isSACall_;
}

bool PermissionVerification::CheckSpecificSystemAbilityAccessPermission(const std::string &processName) const
{
    return MyStatus::GetInstance().isCheckSpecificSystemAbilityAccessPermission_;
}

bool PermissionVerification::VerifyRunningInfoPerm() const
{
    return MyStatus::GetInstance().isVerifyRunningInfoPerm_;
}

bool PermissionVerification::IsShellCall() const
{
    return MyStatus::GetInstance().isShellCall_;
}

bool PermissionVerification::IsAllowedToUseSystemAPI(const std::string &permissionName) const
{
    return MyStatus::GetInstance().isAllowedToUseSystemAPI_;
}

bool PermissionVerification::VerifyCallingPermission(const std::string &permissionName) const
{
    return MyStatus::GetInstance().isVerifyCallingPermission_;
}

bool PermissionVerification::VerifyCallingPermission(
    const std::string &permissionName, const uint32_t specifyTokenId) const
{
    return MyStatus::GetInstance().isVerifyCallingPermission_;
}

bool PermissionVerification::IsAllowLocalDebugOtherApps(bool isDebugFromLocal) const
{
    return false;
}
} // namespace AAFwk
} // namespace OHOS
