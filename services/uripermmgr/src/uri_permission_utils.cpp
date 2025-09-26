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
#include "accesstoken_kit.h"
#include "bundle_mgr_client.h"
#include "global_constant.h"
#include "hilog_tag_wrapper.h"
#include "in_process_call_wrapper.h"
#include "ipc_skeleton.h"
#include "os_account_manager_wrapper.h"
#include "permission_verification.h"
#include "tokenid_kit.h"

namespace OHOS {
namespace AAFwk {
namespace {
constexpr int32_t DEFAULT_USER_ID = 0;
constexpr int32_t API_VERSION_MOD = 100;
constexpr int32_t FOUNDATION_UID = 5523;
constexpr int32_t UDMF_UID = 3012;
constexpr int32_t PASTEBOARD_UID = 3816;
constexpr int32_t BROKER_PASTEBOARD_UID = 5557;
constexpr const char* NET_WORK_ID_MARK = "?networkid=";
}

std::shared_ptr<AppExecFwk::BundleMgrHelper> UPMSUtils::ConnectManagerHelper()
{
    if (bundleMgrHelper_ == nullptr) {
        bundleMgrHelper_ = DelayedSingleton<AppExecFwk::BundleMgrHelper>::GetInstance();
    }
    return bundleMgrHelper_;
}

bool UPMSUtils::SendShareUnPrivilegeUriEvent(uint32_t callerTokenId, uint32_t targetTokenId)
{
    std::string callerBundleName;
    if (!GetBundleNameByTokenId(callerTokenId, callerBundleName)) {
        return false;
    }
    std::string targetBundleName;
    if (!GetBundleNameByTokenId(targetTokenId, targetBundleName)) {
        return false;
    }
    AAFwk::EventInfo eventInfo;
    eventInfo.callerBundleName = callerBundleName;
    eventInfo.bundleName = targetBundleName;
    TAG_LOGD(AAFwkTag::URIPERMMGR, "Send SHARE_UNPRIVILEGED_FILE_URI Event");
    AAFwk::EventReport::SendGrantUriPermissionEvent(AAFwk::EventName::SHARE_UNPRIVILEGED_FILE_URI, eventInfo);
    return true;
}

bool UPMSUtils::SendSystemAppGrantUriPermissionEvent(uint32_t callerTokenId, uint32_t targetTokenId,
    const std::vector<std::string> &uriVec, const std::vector<bool> &resVec)
{
    TAG_LOGD(AAFwkTag::URIPERMMGR, "send grant uri permission event start.");
    EventInfo eventInfo;
    if (!CheckAndCreateEventInfo(callerTokenId, targetTokenId, eventInfo)) {
        return false;
    }
    for (size_t i = 0; i < resVec.size(); i++) {
        if (resVec[i]) {
            Uri uri(uriVec[i]);
            eventInfo.uri = uri.GetScheme() + ":" + uri.GetAuthority();
            EventReport::SendGrantUriPermissionEvent(EventName::GRANT_URI_PERMISSION, eventInfo);
            return true;
        }
    }
    TAG_LOGD(AAFwkTag::URIPERMMGR, "send grant uri permission event end.");
    return false;
}

bool UPMSUtils::SendAutoPersistEvent(uint32_t callerTokenId, uint32_t targetTokenId)
{
    EventInfo eventInfo;
    std::string callerBundleName;
    GetBundleNameByTokenId(callerTokenId, callerBundleName);
    eventInfo.callerBundleName = callerBundleName;
    std::string targetBundleName;
    GetBundleNameByTokenId(targetTokenId, targetBundleName);
    eventInfo.bundleName = targetBundleName;
    eventInfo.uri = "autoPersist";
    EventReport::SendGrantUriPermissionEvent(EventName::GRANT_URI_PERMISSION, eventInfo);
    return true;
}

bool UPMSUtils::CheckAndCreateEventInfo(uint32_t callerTokenId, uint32_t targetTokenId,
    EventInfo &eventInfo)
{
    std::string callerBundleName;
    if (!GetBundleNameByTokenId(callerTokenId, callerBundleName)) {
        TAG_LOGD(AAFwkTag::URIPERMMGR, "get callerBundleName failed");
        return false;
    }
    if (!CheckIsSystemAppByBundleName(callerBundleName)) {
        TAG_LOGD(AAFwkTag::URIPERMMGR, "caller not system");
        return false;
    }
    std::string targetBundleName;
    if (!GetBundleNameByTokenId(targetTokenId, targetBundleName)) {
        TAG_LOGD(AAFwkTag::URIPERMMGR, "get targetBundleName failed");
        return false;
    }
    if (CheckIsSystemAppByBundleName(targetBundleName)) {
        TAG_LOGD(AAFwkTag::URIPERMMGR, "target is systemApp");
        return false;
    }
    eventInfo.callerBundleName = callerBundleName;
    eventInfo.bundleName = targetBundleName;
    return true;
}

int32_t UPMSUtils::GetCurrentAccountId()
{
    std::vector<int32_t> osActiveAccountIds;
    auto ret = DelayedSingleton<AppExecFwk::OsAccountManagerWrapper>::GetInstance()->
        QueryActiveOsAccountIds(osActiveAccountIds);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "QueryActiveOsAccountIds error. ret: %{public}d", ret);
        return DEFAULT_USER_ID;
    }
    if (osActiveAccountIds.empty()) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "QueryActiveOsAccountIds empty");
        return DEFAULT_USER_ID;
    }
    TAG_LOGD(AAFwkTag::URIPERMMGR, "osActiveAccountId: %{public}d", osActiveAccountIds.front());
    return osActiveAccountIds.front();
}

bool UPMSUtils::IsFoundationCall()
{
    return IPCSkeleton::GetCallingUid() == FOUNDATION_UID;
}

bool UPMSUtils::IsSAOrSystemAppCall()
{
    return PermissionVerification::GetInstance()->IsSystemAppCall() ||
        PermissionVerification::GetInstance()->IsSACall();
}

bool UPMSUtils::IsSystemAppCall()
{
    return PermissionVerification::GetInstance()->IsSystemAppCall();
}

bool UPMSUtils::CheckIsSystemAppByBundleName(std::string &bundleName)
{
    auto bundleMgrHelper = ConnectManagerHelper();
    if (bundleMgrHelper == nullptr) {
        TAG_LOGW(AAFwkTag::URIPERMMGR, "bundleMgrHelper null");
        return false;
    }
    AppExecFwk::ApplicationInfo appInfo;
    if (!IN_PROCESS_CALL(bundleMgrHelper->GetApplicationInfo(bundleName,
        AppExecFwk::BundleFlag::GET_BUNDLE_DEFAULT, GetCurrentAccountId(), appInfo))) {
        TAG_LOGW(AAFwkTag::URIPERMMGR, "GetApplicationInfo failed");
        return false;
    }
    auto isSystemApp = Security::AccessToken::TokenIdKit::IsSystemAppByFullTokenID(appInfo.accessTokenIdEx);
    TAG_LOGD(AAFwkTag::URIPERMMGR, "BundleName:%{public}s, isSystemApp:%{public}d", bundleName.c_str(),
        static_cast<int32_t>(isSystemApp));
    return isSystemApp;
}

bool UPMSUtils::GetBundleApiTargetVersion(const std::string &bundleName, int32_t &targetApiVersion)
{
    auto bundleMgrHelper = ConnectManagerHelper();
    if (bundleMgrHelper == nullptr) {
        TAG_LOGW(AAFwkTag::URIPERMMGR, "The bundleMgrHelper is nullptr.");
        return false;
    }
    AppExecFwk::ApplicationInfo appInfo;
    if (!IN_PROCESS_CALL(bundleMgrHelper->GetApplicationInfo(bundleName,
        AppExecFwk::BundleFlag::GET_BUNDLE_DEFAULT, GetCurrentAccountId(), appInfo))) {
        TAG_LOGI(AAFwkTag::URIPERMMGR, "Get application info failed.");
        return false;
    }
    targetApiVersion = (appInfo.apiTargetVersion % API_VERSION_MOD);
    return true;
}

bool UPMSUtils::CheckIsSystemAppByTokenId(uint32_t tokenId)
{
    std::string bundleName;
    if (GetBundleNameByTokenId(tokenId, bundleName)) {
        return CheckIsSystemAppByBundleName(bundleName);
    }
    return false;
}

bool UPMSUtils::GetDirByBundleNameAndAppIndex(const std::string &bundleName, int32_t appIndex, std::string &dirName)
{
    auto bmsClient = DelayedSingleton<AppExecFwk::BundleMgrClient>::GetInstance();
    if (bmsClient == nullptr) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "bundleMgrClient is nullptr.");
        return false;
    }
    auto bmsRet = IN_PROCESS_CALL(bmsClient->GetDirByBundleNameAndAppIndex(bundleName, appIndex, dirName));
    if (bmsRet != ERR_OK) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "GetDirByBundleNameAndAppIndex failed, ret:%{public}d", bmsRet);
        return false;
    }
    return true;
}

bool UPMSUtils::GetAlterableBundleNameByTokenId(uint32_t tokenId, std::string &bundleName)
{
    auto tokenType = Security::AccessToken::AccessTokenKit::GetTokenTypeFlag(tokenId);
    if (tokenType == Security::AccessToken::ATokenTypeEnum::TOKEN_HAP) {
        Security::AccessToken::HapTokenInfo hapInfo;
        auto ret = Security::AccessToken::AccessTokenKit::GetHapTokenInfo(tokenId, hapInfo);
        if (ret != Security::AccessToken::AccessTokenKitRet::RET_SUCCESS) {
            TAG_LOGE(AAFwkTag::URIPERMMGR, "GetHapTokenInfo failed, ret:%{public}d", ret);
            return false;
        }
        return GetDirByBundleNameAndAppIndex(hapInfo.bundleName, hapInfo.instIndex, bundleName);
    }
    return false;
}

bool UPMSUtils::GetBundleNameByTokenId(uint32_t tokenId, std::string &bundleName)
{
    auto tokenType = Security::AccessToken::AccessTokenKit::GetTokenTypeFlag(tokenId);
    if (tokenType == Security::AccessToken::ATokenTypeEnum::TOKEN_HAP) {
        Security::AccessToken::HapTokenInfo hapInfo;
        auto ret = Security::AccessToken::AccessTokenKit::GetHapTokenInfo(tokenId, hapInfo);
        if (ret != Security::AccessToken::AccessTokenKitRet::RET_SUCCESS) {
            TAG_LOGE(AAFwkTag::URIPERMMGR, "GetHapTokenInfo failed, ret:%{public}d", ret);
            return false;
        }
        bundleName = hapInfo.bundleName;
        return true;
    }
    return false;
}

int32_t UPMSUtils::GetAppIdByBundleName(const std::string &bundleName, std::string &appId)
{
    TAG_LOGD(AAFwkTag::URIPERMMGR, "BundleName is %{public}s.", bundleName.c_str());
    auto bms = ConnectManagerHelper();
    if (bms == nullptr) {
        TAG_LOGW(AAFwkTag::URIPERMMGR, "The bundleMgrHelper is nullptr.");
        return GET_BUNDLE_MANAGER_SERVICE_FAILED;
    }
    auto userId = GetCurrentAccountId();
    appId = IN_PROCESS_CALL(bms->GetAppIdByBundleName(bundleName, userId));
    if (appId.empty()) {
        TAG_LOGW(AAFwkTag::URIPERMMGR, "Get appId by bundle name failed, userId is %{private}d", userId);
        return INNER_ERR;
    }
    return ERR_OK;
}

int32_t UPMSUtils::GetTokenIdByBundleName(const std::string &bundleName, int32_t appIndex, uint32_t &tokenId)
{
    TAG_LOGD(AAFwkTag::URIPERMMGR, "BundleName:%{public}s, appIndex:%{public}d", bundleName.c_str(), appIndex);
    auto bms = ConnectManagerHelper();
    if (bms == nullptr) {
        TAG_LOGW(AAFwkTag::URIPERMMGR, "null bms");
        return GET_BUNDLE_MANAGER_SERVICE_FAILED;
    }
    AppExecFwk::BundleInfo bundleInfo;
    auto userId = GetCurrentAccountId();
    if (appIndex == 0) {
        auto bundleFlag = AppExecFwk::BundleFlag::GET_BUNDLE_WITH_EXTENSION_INFO;
        if (!IN_PROCESS_CALL(bms->GetBundleInfo(bundleName, bundleFlag, bundleInfo, userId))) {
            TAG_LOGW(AAFwkTag::URIPERMMGR, "Failed GetBundleInfo");
            return ERR_GET_TARGET_BUNDLE_INFO_FAILED;
        }
        tokenId = bundleInfo.applicationInfo.accessTokenId;
        return ERR_OK;
    }
    if (appIndex <= AbilityRuntime::GlobalConstant::MAX_APP_CLONE_INDEX) {
        auto bundleFlag = static_cast<int32_t>(AppExecFwk::GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_APPLICATION);
        if (IN_PROCESS_CALL(bms->GetCloneBundleInfo(bundleName, bundleFlag, appIndex, bundleInfo, userId)) != ERR_OK) {
            TAG_LOGW(AAFwkTag::URIPERMMGR, "Failed GetCloneBundleInfo");
            return ERR_GET_TARGET_BUNDLE_INFO_FAILED;
        }
        tokenId = bundleInfo.applicationInfo.accessTokenId;
        return ERR_OK;
    }
    if (IN_PROCESS_CALL(bms->GetSandboxBundleInfo(bundleName, appIndex, userId, bundleInfo) != ERR_OK)) {
        TAG_LOGW(AAFwkTag::URIPERMMGR, "Failed GetSandboxBundleInfo");
        return ERR_GET_TARGET_BUNDLE_INFO_FAILED;
    }
    tokenId = bundleInfo.applicationInfo.accessTokenId;
    return ERR_OK;
}

bool UPMSUtils::CheckUriTypeIsValid(Uri &uri)
{
    auto &&scheme = uri.GetScheme();
    if (scheme != "file" && scheme != "content") {
        TAG_LOGW(AAFwkTag::URIPERMMGR, "uri invalid: %{public}s-%{private}s", scheme.c_str(), uri.ToString().c_str());
        return false;
    }
    return true;
}

bool UPMSUtils::IsDocsCloudUri(Uri &uri)
{
    return (uri.GetAuthority() == "docs" && uri.ToString().find(NET_WORK_ID_MARK) != std::string::npos);
}

bool UPMSUtils::IsUdmfOrPasteboardCall()
{
    auto uid = IPCSkeleton::GetCallingUid();
    return uid == UDMF_UID || uid == PASTEBOARD_UID || uid == BROKER_PASTEBOARD_UID;
}

std::shared_ptr<AppExecFwk::BundleMgrHelper> UPMSUtils::bundleMgrHelper_ = nullptr;
}  // namespace AAFwk
}  // namespace OHOS
