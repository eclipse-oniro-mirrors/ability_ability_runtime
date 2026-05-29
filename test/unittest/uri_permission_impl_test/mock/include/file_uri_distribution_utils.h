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

#ifndef OHOS_ABILITY_RUNTIME_FILE_URI_DISTRIBUTION_UTILS_H
#define OHOS_ABILITY_RUNTIME_FILE_URI_DISTRIBUTION_UTILS_H

#include "check_result.h"
#include "uri.h"

namespace OHOS {
namespace AAFwk {
struct FUDAppInfo {
    uint32_t tokenId = 0;
    std::string bundleName;
    std::string alterBundleName;
    int32_t userId = -1;
};
class FUDUtils {
public:
    static bool SendShareUnPrivilegeUriEvent(uint32_t callTokenId, uint32_t targetTokenId);
    static bool SendSystemAppGrantUriPermissionEvent(uint32_t callerTokenId, uint32_t targetTokenId,
        const std::vector<std::string> &uriVec, const std::vector<CheckResult> &resVec);
    static int32_t GetCurrentAccountId();
    static bool IsFoundationCall();
    static bool IsSAOrSystemAppCall();
    static bool IsSystemAppCall();
    static bool IsPrivilegedSACall();
    static bool CheckIsSystemAppByTokenId(uint32_t tokenId);
    static bool GetDirByBundleNameAndAppIndex(const std::string &bundleName, int32_t appIndex, std::string &dirName);
    static bool GetBundleNameByTokenId(uint32_t tokenId, std::string &bundleName);
    static bool GetAlterableBundleNameByTokenId(uint32_t tokenId, std::string &bundleName);
    static int32_t GetAppIdByBundleName(const std::string &bundleName, std::string &appId);
    static int32_t GetTokenIdByBundleName(const std::string &bundleName, int32_t appIndex, int32_t userId,
                                          uint32_t &tokenId);
    static bool CheckUriTypeIsValid(Uri &uri);
    static bool GetBundleApiTargetVersion(const std::string &bundleName, int32_t &targetApiVersion);
    static bool IsDocsCloudUri(Uri &uri);
    static bool GenerateFUDAppInfo(FUDAppInfo &info);
    static bool IsUdmfOrPasteboardCall();
    static bool IsDFSCall();
    static bool IsSandboxApp(uint32_t tokenId);
    static bool IsBrokerCaller();
};
} // OHOS
} // AAFwk
#endif  // OHOS_ABILITY_RUNTIME_FILE_URI_DISTRIBUTION_UTILS_H