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

#ifndef MOCK_MY_FLAG_H
#define MOCK_MY_FLAG_H

#include <string>
#include <sys/types.h>
#include <unordered_map>

#include "access_token.h"

namespace OHOS {
namespace AAFwk {
struct TokenInfo;
using MyATokenTypeEnum = Security::AccessToken::ATokenTypeEnum;
using TokenInfoMap = std::unordered_map<uint32_t, TokenInfo>;

struct TokenInfo {
    uint32_t tokenId = 0;
    std::string processName = "";
    std::string bundleName = "";
    int32_t userID = 0;
    MyATokenTypeEnum tokenType = MyATokenTypeEnum::TOKEN_INVALID;
    
    TokenInfo() {}

    TokenInfo(uint32_t tokenId, MyATokenTypeEnum tokenType, std::string processName = "", std::string bundleName = "",
        int32_t userID = 0)
    {
        this->tokenId = tokenId;
        this->tokenType = tokenType;
        this->processName = processName;
        this->bundleName = bundleName;
        this->userID = userID;
    }
};

class MyFlag {
public:
    static TokenInfoMap tokenInfos_;
    static int retNativeSuccValue_;
    static int retNativeFailValue_;
    static int retHapSuccValue_;
    static int retHapFailValue_;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif // MOCK_MY_FLAG_H