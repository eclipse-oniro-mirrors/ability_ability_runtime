/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "mock_accesstoken_kit.h"
#include "mock_my_status.h"

namespace OHOS {
namespace Security {
namespace AccessToken {

ATokenTypeEnum AccessTokenKit::GetTokenTypeFlag(AccessTokenID tokenID)
{
    return static_cast<ATokenTypeEnum>(AAFwk::MyStatus::GetInstance().atkGetTokenTypeFlag_);
}

int AccessTokenKit::GetNativeTokenInfo(AccessTokenID tokenID, NativeTokenInfo &nativeTokenInfoRes)
{
    return -1;
}

int AccessTokenKit::GetHapTokenInfo(AccessTokenID tokenID, HapTokenInfo &hapInfo)
{
    // Configurable for tests that exercise the app-notify path (e.g. skill launch
    // completion); the default keeps the legacy "lookup fails" behavior.
    auto &status = AAFwk::MyStatus::GetInstance();
    if (status.atkGetHapTokenInfoRet_ == 0) {
        hapInfo.bundleName = status.atkHapTokenInfoBundleName_;
    }
    return status.atkGetHapTokenInfoRet_;
}

int AccessTokenKit::VerifyAccessToken(AccessTokenID tokenID, const std::string &permissionName, bool crossIpc)
{
    return -1;
}

void AccessTokenKit::GetPermissionManagerInfo(PermissionGrantInfo &info)
{
}

uint32_t AccessTokenKit::GetHapTokenID(int32_t userId, const std::string &bundleName, int32_t appIndex)
{
    return -1;
}

} // namespace AccessToken
} // namespace Security
} // namespace OHOS