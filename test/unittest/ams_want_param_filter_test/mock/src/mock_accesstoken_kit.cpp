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

#include "mock_accesstoken_kit.h"

namespace OHOS {
namespace Security {
namespace AccessToken {
namespace {
// default INVALID keeps the third-party strip path of the existing cases
MockTokenType g_mockTokenType = MockTokenType::INVALID;
} // namespace

void SetMockTokenType(MockTokenType type)
{
    g_mockTokenType = type;
}

MockTokenType GetMockTokenType()
{
    return g_mockTokenType;
}

ATokenTypeEnum AccessTokenKit::GetTokenTypeFlag(AccessTokenID tokenID)
{
    switch (g_mockTokenType) {
        case MockTokenType::HAP:
            return ATokenTypeEnum::TOKEN_HAP;
        case MockTokenType::NATIVE:
            return ATokenTypeEnum::TOKEN_NATIVE;
        default:
            return ATokenTypeEnum::TOKEN_INVALID;
    }
}

int AccessTokenKit::GetNativeTokenInfo(AccessTokenID tokenID, NativeTokenInfo &nativeTokenInfoRes)
{
    return -1;
}

int AccessTokenKit::GetHapTokenInfo(AccessTokenID tokenID, HapTokenInfo &hapInfo)
{
    return -1;
}
} // namespace AccessToken
} // namespace Security
} // namespace OHOS
