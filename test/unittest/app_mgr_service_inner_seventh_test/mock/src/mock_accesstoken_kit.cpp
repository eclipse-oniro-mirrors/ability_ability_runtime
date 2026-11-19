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

#include "accesstoken_kit.h"
#include "mock_my_status.h"

namespace OHOS {
namespace Security {
namespace AccessToken {

ATokenTypeEnum AccessTokenKit::GetTokenTypeFlag(AccessTokenID tokenID)
{
    return ATokenTypeEnum::TOKEN_INVALID;
}

int AccessTokenKit::GetNativeTokenInfo(AccessTokenID tokenID, NativeTokenInfo &nativeTokenInfoRes)
{
    return 0;
}

int AccessTokenKit::GetHapTokenInfo(AccessTokenID tokenID, HapTokenInfo &hapInfo)
{
    return 0;
}

int AccessTokenKit::ClearUserGrantedPermissionState(AccessTokenID tokenID)
{
    return AAFwk::MyStatus::GetInstance().clearUserGranted_;
}

bool AccessTokenKit::IsCliToolToken(FullTokenID tokenID)
{
    return AAFwk::MyStatus::GetInstance().isCliToolToken_;
}

} // namespace AccessToken
} // namespace Security
} // namespace OHOS
