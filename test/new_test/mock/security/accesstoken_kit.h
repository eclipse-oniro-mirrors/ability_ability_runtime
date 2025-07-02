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

#ifndef INTERFACES_INNER_KITS_ACCESSTOKEN_KIT_H
#define INTERFACES_INNER_KITS_ACCESSTOKEN_KIT_H

#include "access_token.h"
#include "native_token_info.h"
#include "hap_token_info.h"

namespace OHOS {
namespace Security {
namespace AccessToken {

class AccessTokenKit {
public:
    static ATokenTypeEnum GetTokenTypeFlag(AccessTokenID tokenID)
    {
        return getTokenTypeFlagRet;
    }

    static int32_t GetNativeTokenInfo(AccessTokenID tokenID, NativeTokenInfo &nativeTokenInfoRes)
    {
        nativeTokenInfoRes = nativeTokenInfo;
        return getNativeTokenInfoRet;
    }

    static int GetHapTokenInfo(AccessTokenID tokenID, HapTokenInfo &hapInfoRes)
    {
        hapInfoRes = hapInfo;
        return getHapTokenInfoRet;
    }

    static void InitMockResult()
    {
        getNativeTokenInfoRet = 0;
        getHapTokenInfoRet = 0;
        nativeTokenInfo = NativeTokenInfo();
        hapInfo = HapTokenInfo();
    }

    static NativeTokenInfo nativeTokenInfo;
    static HapTokenInfo hapInfo;
    static int32_t getNativeTokenInfoRet;
    static int32_t getHapTokenInfoRet;
    static ATokenTypeEnum getTokenTypeFlagRet;
};
} // namespace AccessToken
} // namespace Security
} // namespace OHOS
#endif