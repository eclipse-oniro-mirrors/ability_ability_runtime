/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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

struct FUDAppInfoMockResult {
    bool success;
    int32_t userId;
    std::string alterBundleName;
    std::string bundleName;
    bool isSA = false;

    FUDAppInfoMockResult() : success(false), userId(-1), alterBundleName(""), bundleName(""), isSA(false) {}

    FUDAppInfoMockResult(bool s, int32_t uid, const std::string& alterName = "", const std::string& bName = "",
                         bool sa = false)
        : success(s), userId(uid), alterBundleName(alterName), bundleName(bName), isSA(sa) {}
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
        permissionAgentFileAccess_ = false;
        permissionAgentFileAccessDeclared_ = false;
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
        permissionSandboxAccessManager_ = false;
        IsSystemAppCall_ = false;
        getTokenIdByBundleNameStatus_ = 0;
        isSAOrSystemAppCall_ = false;
        isSystemAppCall_ = false;
        isPrivilegedSACall_ = false;
        isUriTypeValid_ = false;
        isDocsCloudUri_ = false;
        permissionGrantUriPermissionAsCaller_ = false;
        processUdmfKeyRet_ = 0;
        udmfUtilsUris_ = {};
        upmsUtilsCheckIsSystemAppByBundleNameRet_ = true;
        upmsUtilsCheckIsSystemAppByTokenIdRet_ = false;
        upmsUtilsGetDirByBundleNameAndAppIndexRet_ = true;
        upmsUtilsAlterBundleName_ = "";
        upmsUtilsGetAlterBundleNameByTokenIdRet_ = true;
        upmsUtilsBundleName_ = "";
        upmsUtilsGetBundleNameByTokenIdRet_ = false;
        upmsUtilsAppId_ = "";
        upmsUtilsGetAppIdByBundleNameRet_ = 0;
        upmsUtilsTokenId_ = 0;
        upmsUtilsIsFoundationCallRet_ = false;
        fudUtilsGenerateFUDAppInfoRet_ = true;
        fudAppInfoUserId_ = -1;
        generateFUDAppInfoResults_.clear();
        tokenInfos = {};
        isUdmfOrPasteboardCallRet_ = false;
        isDFSCallRet_ = false;
        isSandboxAppRet_ = false;
        isSACall_ = false;
    }

    static void PushGenerateFUDAppInfoResult(bool success, int32_t userId,
                                               const std::string& alterBundleName = "",
                                               const std::string& bundleName = "")
    {
        FUDAppInfoMockResult result;
        result.success = success;
        result.userId = userId;
        result.alterBundleName = alterBundleName;
        result.bundleName = bundleName;
        generateFUDAppInfoResults_.push_back(result);
    }

    static int flag_;
    static bool permissionFileAccessManager_;
    static bool permissionAgentFileAccess_;
    static bool permissionAgentFileAccessDeclared_;
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
    static bool permissionSandboxAccessManager_;
    static bool IsSystemAppCall_;
    static bool permissionFileAccessPersist_;
    static bool permissionGrantUriPermissionAsCaller_;
    static bool isSAOrSystemAppCall_;
    static bool isUriTypeValid_;
    static bool isSystemAppCall_;
    static bool isPrivilegedSACall_;
    static bool isDocsCloudUri_;
    static int32_t getTokenIdByBundleNameStatus_;
    static int32_t processUdmfKeyRet_;
    static std::vector<std::string> udmfUtilsUris_;
    static bool upmsUtilsCheckIsSystemAppByBundleNameRet_;
    static bool upmsUtilsCheckIsSystemAppByTokenIdRet_;
    static bool upmsUtilsGetDirByBundleNameAndAppIndexRet_;
    static std::string upmsUtilsAlterBundleName_ ;
    static bool upmsUtilsGetAlterBundleNameByTokenIdRet_;
    static std::string upmsUtilsBundleName_;
    static bool upmsUtilsGetBundleNameByTokenIdRet_;
    static std::string upmsUtilsAppId_;
    static int32_t upmsUtilsGetAppIdByBundleNameRet_;
    static uint32_t upmsUtilsTokenId_;
    static bool upmsUtilsIsFoundationCallRet_;
    static bool fudUtilsGenerateFUDAppInfoRet_;
    static int32_t fudAppInfoUserId_;
    static std::string bundleName_;
    static std::vector<FUDAppInfoMockResult> generateFUDAppInfoResults_;
    static TokenInfoMap tokenInfos;
    static bool isUdmfOrPasteboardCallRet_;
    static bool isDFSCallRet_;
    static bool isSandboxAppRet_;
    static bool isSACall_;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif // MOCK_MY_FLAG_H