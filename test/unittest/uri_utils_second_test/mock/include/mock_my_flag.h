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
    MyATokenTypeEnum tokenType = MyATokenTypeEnum::TOKEN_INVALID;
    
    TokenInfo() {}

    TokenInfo(uint32_t tokenId, MyATokenTypeEnum tokenType, std::string processName = "", std::string bundleName = "")
    {
        this->tokenId = tokenId;
        this->tokenType = tokenType;
        this->processName = processName;
        this->bundleName = bundleName;
    }
};

class MyFlag {
public:
    enum FLAG {
        IS_SA_CALL = 1,
        IS_SHELL_CALL,
        IS_SA_AND_SHELL_CALL
    };

    static void Init()
    {
        flag_ = 0;
        permissionFileAccessManager_ = false;
        permissionWriteImageVideo_ = false;
        permissionReadImageVideo_ = false;
        permissionAllMedia_ = false;
        permissionWriteAudio_ = false;
        permissionReadAudio_ = false;
        permissionProxyAuthorization_ = false;
        permissionAll_ = false;
        permissionPrivileged_ = false;
        permissionReadWriteDownload_ = false;
        permissionReadWriteDesktop_ = false;
        permissionReadWriteDocuments_ = false;
        IsSystempAppCall_ = false;
        tokenInfos = {};
    }

    static int flag_;
    static bool permissionFileAccessManager_;
    static bool permissionWriteImageVideo_;
    static bool permissionReadImageVideo_;
    static bool permissionWriteAudio_;
    static bool permissionReadAudio_;
    static bool permissionAllMedia_;
    static bool permissionProxyAuthorization_;
    static bool permissionAll_;
    static bool permissionPrivileged_;
    static bool permissionReadWriteDownload_;
    static bool permissionReadWriteDesktop_;
    static bool permissionReadWriteDocuments_;
    static bool IsSystempAppCall_;
    static bool permissionFileAccessPersist_;
    static bool isSAOrSystemAppCall_;
    static bool isUriTypeValid_;
    static bool isSystemAppCall_;
    static bool isDocsCloudUri_;
    static std::string bundleName_;
    static int32_t getTokenIdByBundleNameStatus_;

    static TokenInfoMap tokenInfos;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif // MOCK_MY_FLAG_H