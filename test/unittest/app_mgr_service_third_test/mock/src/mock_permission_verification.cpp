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
bool PermissionVerification::CheckSpecificSystemAbilityAccessPermission(const std::string &processName) const
{
    return !!(MyFlag::flag_);
}

bool PermissionVerification::VerifyRunningInfoPerm() const
{
    return MyFlag::perm;
}

bool PermissionVerification::JudgeCallerIsAllowedToUseSystemAPI() const
{
    return !!(MyFlag::flag_);
}
bool PermissionVerification::JudgeCallerIsAllowedToUseSystemAPIByTokenId(uint64_t specifiedFullTokenId) const
{
    return true;
}
bool PermissionVerification::IsSACall() const
{
    return (MyFlag::flag_ & MyFlag::FLAG::IS_SA_CALL);
}

bool PermissionVerification::VerifyPreloadApplicationPermission() const
{
    return (MyFlag::flag_ & MyFlag::FLAG::IS_PRELOAD_APPLICATION_PERMISSION);
}
}  // namespace AAFwk
}  // namespace OHOS