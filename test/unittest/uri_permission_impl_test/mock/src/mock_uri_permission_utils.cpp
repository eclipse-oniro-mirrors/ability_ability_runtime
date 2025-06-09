/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#include "uri_permission_utils.h"

#include "ability_manager_errors.h"
#include "mock_my_flag.h"

namespace OHOS {
namespace AAFwk {
using MyFlag = OHOS::AAFwk::MyFlag;

bool UPMSUtils::SendShareUnPrivilegeUriEvent(uint32_t callerTokenId, uint32_t targetTokenId)
{
    return true;
}

bool UPMSUtils::SendSystemAppGrantUriPermissionEvent(uint32_t callerTokenId, uint32_t targetTokenId,
    const std::vector<std::string> &uriVec, const std::vector<bool> &resVec)
{
    return true;
}

int32_t UPMSUtils::GetCurrentAccountId()
{
    return 1;
}

bool UPMSUtils::IsFoundationCall()
{
    return MyFlag::upmsUtilsIsFoundationCallRet_;
}

bool UPMSUtils::IsSAOrSystemAppCall()
{
    return MyFlag::isSAOrSystemAppCall_ || (MyFlag::IS_SA_CALL & MyFlag::flag_) != 0;
}

bool UPMSUtils::IsSystemAppCall()
{
    return MyFlag::isSystemAppCall_;
}

bool UPMSUtils::GetBundleApiTargetVersion(const std::string &bundleName, int32_t &targetApiVersion)
{
    return true;
}

bool UPMSUtils::CheckIsSystemAppByTokenId(uint32_t tokenId)
{
    return MyFlag::upmsUtilsCheckIsSystemAppByTokenIdRet_;
}

bool UPMSUtils::GetDirByBundleNameAndAppIndex(const std::string &bundleName, int32_t appIndex, std::string &dirName)
{
    dirName = MyFlag::upmsUtilsAlterBundleName_;
    return MyFlag::upmsUtilsGetDirByBundleNameAndAppIndexRet_;
}

bool UPMSUtils::GetAlterableBundleNameByTokenId(uint32_t tokenId, std::string &bundleName)
{
    bundleName = MyFlag::upmsUtilsAlterBundleName_;
    return MyFlag::upmsUtilsGetAlterBundleNameByTokenIdRet_;
}

bool UPMSUtils::GetBundleNameByTokenId(uint32_t tokenId, std::string &bundleName)
{
    bundleName = MyFlag::upmsUtilsBundleName_;
    return MyFlag::upmsUtilsGetBundleNameByTokenIdRet_;
}

int32_t UPMSUtils::GetAppIdByBundleName(const std::string &bundleName, std::string &appId)
{
    appId = MyFlag::upmsUtilsAppId_;
    return MyFlag::upmsUtilsGetAppIdByBundleNameRet_;
}

int32_t UPMSUtils::GetTokenIdByBundleName(const std::string &bundleName, int32_t appIndex, uint32_t &tokenId)
{
    tokenId = MyFlag::upmsUtilsTokenId_;
    return MyFlag::getTokenIdByBundleNameStatus_;
}

bool UPMSUtils::CheckUriTypeIsValid(Uri &uri)
{
    return MyFlag::isUriTypeValid_;
}

bool UPMSUtils::IsDocsCloudUri(Uri &uri)
{
    return MyFlag::isDocsCloudUri_;
}
}  // namespace AAFwk
}  // namespace OHOS
