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
#include "mock_my_flag.h"

namespace OHOS {
namespace Security {
namespace AccessToken {
using MyFlag = OHOS::AAFwk::MyFlag;

ATokenTypeEnum AccessTokenKit::GetTokenTypeFlag(AccessTokenID tokenID)
{
    if (MyFlag::tokenInfos.find(tokenID) != MyFlag::tokenInfos.end()) {
        return MyFlag::tokenInfos[tokenID].tokenType;
    }
    return ATokenTypeEnum::TOKEN_INVALID;
}

int AccessTokenKit::GetNativeTokenInfo(AccessTokenID tokenID, NativeTokenInfo &nativeTokenInfoRes)
{
    if (MyFlag::tokenInfos.find(tokenID) != MyFlag::tokenInfos.end()) {
        nativeTokenInfoRes.processName = MyFlag::tokenInfos[tokenID].processName;
        return 0;
    }
    return -1;
}

int AccessTokenKit::GetHapTokenInfo(AccessTokenID tokenID, HapTokenInfo &hapInfo)
{
    if (MyFlag::tokenInfos.find(tokenID) != MyFlag::tokenInfos.end()) {
        hapInfo.bundleName = MyFlag::tokenInfos[tokenID].bundleName;
        return 0;
    }
    return -1;
}

int AccessTokenKit::GetReqPermissions(AccessTokenID tokenID, std::vector<PermissionStateFull> &reqPermList,
    bool isSystemGrant)
{
    if (MyFlag::permissionAgentFileAccessDeclared_) {
        PermissionStateFull perm;
        perm.permissionName = "ohos.permission.AGENT_FILE_ACCESS";
        reqPermList.push_back(perm);
    }
    return 0;
}
} // namespace AccessToken
} // namespace Security
} // namespace OHOS