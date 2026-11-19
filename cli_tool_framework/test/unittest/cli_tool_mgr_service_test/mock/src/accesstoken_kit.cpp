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

#include "accesstoken_kit.h"
#include "ipc_skeleton.h"

namespace OHOS {
namespace Security {
namespace AccessToken {

ATokenTypeEnum AccessTokenKit::GetTokenTypeFlag(uint64_t tokenID)
{
    // Return TOKEN_NATIVE for native tokens, TOKEN_HAP for others
    if (tokenID == TOKEN_NATIVE) {
        return ATokenTypeEnum::TOKEN_NATIVE;
    }
    return ATokenTypeEnum::TOKEN_HAP;
}

int32_t AccessTokenKit::VerifyAccessToken(AccessTokenID tokenID, const std::string &permissionName, bool crossIpc)
{
    (void)tokenID;
    (void)permissionName;
    (void)crossIpc;
    return PermissionState::PERMISSION_GRANTED;
}

int32_t AccessTokenKit::DeleteToolTokenByPid(int32_t pid)
{
    (void)pid;
    return 0;  // Mock implementation - return success
}

} // namespace AccessToken
} // namespace Security
} // namespace OHOS
