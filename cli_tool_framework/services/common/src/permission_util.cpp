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

#include "permission_util.h"

#include "accesstoken_kit.h"
#include "cli_error_code.h"
#include "hilog_tag_wrapper.h"
#include "ipc_skeleton.h"
#include "tokenid_kit.h"

namespace OHOS {
namespace CliTool {
bool PermissionUtil::VerifyAccessToken(AccessToken::AccessTokenID tokenId, const std::string &requirePermission)
{
    int32_t ret = Security::AccessToken::AccessTokenKit::VerifyAccessToken(tokenId, requirePermission, false);
    if (ret != Security::AccessToken::PermissionState::PERMISSION_GRANTED) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "%{public}d not has %{public}s", tokenId, requirePermission.c_str());
        return false;
    }
    return true;
}

bool PermissionUtil::IsSystemApp()
{
    auto fullTokenId = IPCSkeleton::GetCallingFullTokenID();
    return AccessToken::TokenIdKit::IsSystemAppByFullTokenID(fullTokenId);
}

bool PermissionUtil::IsSystemSA()
{
    auto callerToken = IPCSkeleton::GetCallingTokenID();
    return Security::AccessToken::AccessTokenKit::GetTokenTypeFlag(callerToken) ==
        Security::AccessToken::ATokenTypeEnum::TOKEN_NATIVE;
}

int32_t PermissionUtil::CheckSystemAndPermission(const std::string &permissionName)
{
    if (!IsSystemApp() && !IsSystemSA()) {
        TAG_LOGW(AAFwkTag::CLI_TOOL, "CheckSystemAndPermission: not a system app nor SA");
        return ERR_NOT_SYSTEM_APP;
    }
    auto callerToken = IPCSkeleton::GetCallingTokenID();
    if (!VerifyAccessToken(callerToken, permissionName)) {
        TAG_LOGW(AAFwkTag::CLI_TOOL, "CheckSystemAndPermission: permission denied");
        return ERR_PERMISSION_DENIED;
    }
    return ERR_OK;
}

int32_t PermissionUtil::CheckSystemAppAndPermission(const std::string &permissionName)
{
    if (!IsSystemApp()) {
        TAG_LOGW(AAFwkTag::CLI_TOOL, "CheckSystemAppAndPermission: not a system app (SA not allowed)");
        return ERR_NOT_SYSTEM_APP;
    }
    auto callerToken = IPCSkeleton::GetCallingTokenID();
    if (!VerifyAccessToken(callerToken, permissionName)) {
        TAG_LOGW(AAFwkTag::CLI_TOOL, "CheckSystemAppAndPermission: permission denied");
        return ERR_PERMISSION_DENIED;
    }
    return ERR_OK;
}
} // namespace CliTool
} // namespace OHOS
