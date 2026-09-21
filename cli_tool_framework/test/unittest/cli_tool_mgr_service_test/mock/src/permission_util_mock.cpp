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

#include "cli_error_code.h"
#include "ipc_skeleton.h"
#include "permission_util_mock.h"

namespace OHOS {
namespace CliTool {
bool PermissionUtilMock::execCliToolPermitted = true;
bool PermissionUtilMock::execPublicCliToolPermitted = true;

void PermissionUtilMock::Reset()
{
    execCliToolPermitted = true;
    execPublicCliToolPermitted = true;
}

bool PermissionUtil::VerifyAccessToken(Security::AccessToken::AccessTokenID, const std::string &perm)
{
    if (perm == "ohos.permission.EXEC_CLI_TOOL") {
        return PermissionUtilMock::execCliToolPermitted;
    }
    if (perm == "ohos.permission.EXEC_PUBLIC_CLI_TOOL") {
        return PermissionUtilMock::execPublicCliToolPermitted;
    }
    return true;
}

bool PermissionUtil::IsSystemApp()
{
    return IPCSkeleton::GetCallingFullTokenID() == 0;
}

bool PermissionUtil::IsSystemSA()
{
    return IPCSkeleton::GetCallingTokenID() == TOKEN_NATIVE;
}

int32_t PermissionUtil::CheckSystemAndPermission(const std::string &)
{
    if (!IsSystemApp() && !IsSystemSA()) {
        return ERR_NOT_SYSTEM_APP;
    }
    return ERR_OK;
}

} // namespace CliTool
} // namespace OHOS
