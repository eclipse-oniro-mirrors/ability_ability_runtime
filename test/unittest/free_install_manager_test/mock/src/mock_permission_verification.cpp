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

#include "permission_constants.h"

namespace OHOS {
namespace AAFwk {
bool PermissionVerification::VerifyCallingPermission(const std::string &permissionName,
    const uint32_t specifyTokenId) const
{
    if (permissionName == PermissionConstants::PERMISSION_CONNECT_AGENT) {
        return true;
    }
    return false;
}

bool PermissionVerification::IsSACall() const
{
    return false;
}

bool PermissionVerification::CheckSpecificSystemAbilityAccessPermission(const std::string &processName) const
{
    return false;
}

bool PermissionVerification::JudgeCallerIsAllowedToUseSystemAPI() const
{
    return false;
}

bool PermissionVerification::IsSystemAppCall() const
{
    return false;
}

bool PermissionVerification::IsSystemAppCallByTokenId(uint32_t callerTokenId) const
{
    return false;
}
}  // namespace AAFwk
}  // namespace OHOS
