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

#include "launch_service_bundle_mock.h"
#include "accesstoken_kit.h"
#include "net_conn_client.h"
namespace OHOS::Security::AccessToken {
int AccessTokenKit::VerifyAccessToken(AccessTokenID, const std::string &)
{
    return AppExecFwk::LaunchServiceBundleMock::Instance().permissionGranted ? PERMISSION_GRANTED : PERMISSION_DENIED;
}
int AccessTokenKit::VerifyAccessToken(AccessTokenID token, const std::string &permission, bool)
{
    return VerifyAccessToken(token, permission);
}
int32_t AccessTokenKit::GetKernelPermissions(AccessTokenID, std::vector<PermissionWithValue> &permissions)
{
    permissions.clear();
    return 0;
}
} // namespace OHOS::Security::AccessToken
namespace OHOS::NetManagerStandard {
int32_t NetConnClient::SetInternetPermission(uint32_t uid, uint8_t allow)
{
    auto &mock = AppExecFwk::LaunchServiceBundleMock::Instance();
    mock.internetUid = uid;
    mock.internetAllowed = allow;
    return mock.networkResult;
}
} // namespace OHOS::NetManagerStandard
