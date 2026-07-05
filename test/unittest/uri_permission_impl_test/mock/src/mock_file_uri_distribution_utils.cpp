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

#include "file_uri_distribution_utils.h"

#include "ability_manager_errors.h"
#include "mock_my_flag.h"

namespace OHOS {
namespace AAFwk {
using MyFlag = OHOS::AAFwk::MyFlag;

bool FUDUtils::SendShareUnPrivilegeUriEvent(uint32_t callerTokenId, uint32_t targetTokenId)
{
    return true;
}

bool FUDUtils::SendSystemAppGrantUriPermissionEvent(uint32_t callerTokenId, uint32_t targetTokenId,
    const std::vector<std::string> &uriVec, const std::vector<CheckResult> &resVec)
{
    return true;
}

int32_t FUDUtils::GetCurrentAccountId()
{
    return 1;
}

bool FUDUtils::IsFoundationCall()
{
    return MyFlag::upmsUtilsIsFoundationCallRet_;
}

bool FUDUtils::IsSAOrSystemAppCall()
{
    return MyFlag::isSAOrSystemAppCall_ || (MyFlag::IS_SA_CALL & MyFlag::flag_) != 0;
}

bool FUDUtils::IsSystemAppCall()
{
    return MyFlag::isSystemAppCall_;
}

bool FUDUtils::IsPrivilegedSACall()
{
    return MyFlag::isPrivilegedSACall_;
}

bool FUDUtils::GetBundleApiTargetVersion(const std::string &bundleName, int32_t &targetApiVersion)
{
    return true;
}

bool FUDUtils::CheckIsSystemAppByTokenId(uint32_t tokenId)
{
    return MyFlag::upmsUtilsCheckIsSystemAppByTokenIdRet_;
}

bool FUDUtils::GetDirByBundleNameAndAppIndex(const std::string &bundleName, int32_t appIndex, std::string &dirName)
{
    dirName = MyFlag::upmsUtilsAlterBundleName_;
    return MyFlag::upmsUtilsGetDirByBundleNameAndAppIndexRet_;
}

bool FUDUtils::GetAlterableBundleNameByTokenId(uint32_t tokenId, std::string &bundleName)
{
    bundleName = MyFlag::upmsUtilsAlterBundleName_;
    return MyFlag::upmsUtilsGetAlterBundleNameByTokenIdRet_;
}

bool FUDUtils::GetBundleNameByTokenId(uint32_t tokenId, std::string &bundleName)
{
    bundleName = MyFlag::upmsUtilsBundleName_;
    return MyFlag::upmsUtilsGetBundleNameByTokenIdRet_;
}

int32_t FUDUtils::GetAppIdByBundleName(const std::string &bundleName, std::string &appId)
{
    appId = MyFlag::upmsUtilsAppId_;
    return MyFlag::upmsUtilsGetAppIdByBundleNameRet_;
}

int32_t FUDUtils::GetTokenIdByBundleName(const std::string &bundleName, int32_t appIndex, int32_t userId,
                                         uint32_t &tokenId)
{
    tokenId = MyFlag::upmsUtilsTokenId_;
    return MyFlag::getTokenIdByBundleNameStatus_;
}

bool FUDUtils::CheckUriTypeIsValid(Uri &uri)
{
    return MyFlag::isUriTypeValid_;
}

bool FUDUtils::IsDocsCloudUri(Uri &uri)
{
    return MyFlag::isDocsCloudUri_;
}

bool FUDUtils::GenerateFUDAppInfo(FUDAppInfo &info, bool supportSA)
{
    if (!MyFlag::generateFUDAppInfoResults_.empty()) {
        auto result = MyFlag::generateFUDAppInfoResults_.front();
        MyFlag::generateFUDAppInfoResults_.erase(MyFlag::generateFUDAppInfoResults_.begin());

        info.alterBundleName = result.alterBundleName.empty() ?
            MyFlag::upmsUtilsAlterBundleName_ : result.alterBundleName;
        info.bundleName = result.bundleName.empty() ?
            MyFlag::upmsUtilsBundleName_ : result.bundleName;
        info.userId = result.userId;
        info.isSA = result.isSA;
        return result.success;
    }

    info.alterBundleName = MyFlag::upmsUtilsAlterBundleName_;
    info.bundleName = MyFlag::upmsUtilsBundleName_;
    info.userId = MyFlag::fudAppInfoUserId_;
    info.isSA = false;
    return MyFlag::fudUtilsGenerateFUDAppInfoRet_;
}

bool FUDUtils::IsUdmfOrPasteboardCall()
{
    return MyFlag::isUdmfOrPasteboardCallRet_;
}

bool FUDUtils::IsDFSCall()
{
    return MyFlag::isDFSCallRet_;
}

bool FUDUtils::IsSandboxApp(uint32_t tokenId)
{
    return MyFlag::isSandboxAppRet_;
}

bool FUDUtils::IsBrokerCaller()
{
    return false;
}
}  // namespace AAFwk
}  // namespace OHOS
