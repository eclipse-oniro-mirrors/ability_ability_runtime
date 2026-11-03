/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
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
bool PermissionVerification::verifyCallingPermissionRet_ = false;
bool PermissionVerification::verifyBackgroundCallPermissionRet_ = true;
int PermissionVerification::verifyCallingPermissionCount_ = 0;
std::string PermissionVerification::lastPermissionName_;
uint32_t PermissionVerification::lastSpecifyTokenId_ = 0;

bool PermissionVerification::VerifyCallingPermission(
    const std::string &permissionName, const uint32_t specifyTokenId) const
{
    verifyCallingPermissionCount_++;
    lastPermissionName_ = permissionName;
    lastSpecifyTokenId_ = specifyTokenId;
    return verifyCallingPermissionRet_;
}

bool PermissionVerification::VerifyBackgroundCallPermission(const bool isBackgroundCall) const
{
    return verifyBackgroundCallPermissionRet_;
}

bool PermissionVerification::IsSystemAppCall() const
{
    return false;
}

bool PermissionVerification::IsSACall() const
{
    return false;
}

bool PermissionVerification::VerifyDlpPermission(Want &want) const
{
    return false;
}
} // namespace AAFwk
} // namespace OHOS
