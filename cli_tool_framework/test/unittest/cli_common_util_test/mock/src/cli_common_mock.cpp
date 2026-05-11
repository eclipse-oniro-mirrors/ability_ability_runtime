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

#include "cli_common_mock.h"

#include "accesstoken_kit.h"

namespace OHOS {
namespace CliTool {
int32_t CliCommonMock::intParameterValue = 8;
int32_t CliCommonMock::vectorPermissionResult = Security::AccessToken::PermissionState::PERMISSION_GRANTED;
int32_t CliCommonMock::singlePermissionResult = Security::AccessToken::PermissionState::PERMISSION_GRANTED;
std::vector<int32_t> CliCommonMock::permissionStateList;

void CliCommonMock::Reset()
{
    intParameterValue = 8;
    vectorPermissionResult = Security::AccessToken::PermissionState::PERMISSION_GRANTED;
    singlePermissionResult = Security::AccessToken::PermissionState::PERMISSION_GRANTED;
    permissionStateList.clear();
}
} // namespace CliTool

namespace Security {
namespace AccessToken {
int32_t AccessTokenKit::VerifyAccessToken(AccessTokenID, const std::vector<std::string> &permissions,
    std::vector<int32_t> &permStateList)
{
    if (!CliTool::CliCommonMock::permissionStateList.empty()) {
        permStateList = CliTool::CliCommonMock::permissionStateList;
    } else {
        permStateList.assign(permissions.size(), CliTool::CliCommonMock::vectorPermissionResult);
    }
    return CliTool::CliCommonMock::vectorPermissionResult;
}

int32_t AccessTokenKit::VerifyAccessToken(AccessTokenID, const std::string &, bool)
{
    return CliTool::CliCommonMock::singlePermissionResult;
}
} // namespace AccessToken
} // namespace Security
} // namespace OHOS
