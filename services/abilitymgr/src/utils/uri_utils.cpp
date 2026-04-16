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

#include "uri_utils.h"

#include "ability_manager_client.h"
#include "ability_config.h"
#include "ability_record.h"
#include "ability_manager_errors.h"
#include "ability_util.h"
#include "accesstoken_kit.h"
#include "app_utils.h"
#include "common_event_manager.h"
#include "event_report.h"
#include "extension_ability_info.h"
#include "global_constant.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "in_process_call_wrapper.h"
#include "ipc_skeleton.h"
#include "tokenid_kit.h"
#include "ui_extension_wrapper.h"
#ifdef SUPPORT_UPMS
#include "fud_constants.h"
#include "uri_permission_manager_client.h"
#endif // SUPPORT_UPMS

namespace OHOS {
namespace AAFwk {
namespace {
const std::string PARAMS_URI = "ability.verify.uri";
const std::string DISTRIBUTED_FILES_PATH = "/data/storage/el2/distributedfiles/";
const std::string HIDE_SENSITIVE_TYPE = "ohos.media.params.hideSensitiveType";
const std::string DMS_PROCESS_NAME = "distributedsched";
const std::string ERASE_URI = "eraseUri";
const std::string ERASE_PARAM_STREAM = "eraseParamStream";
const std::string SEPARATOR = "/";
const std::string SCHEME_SEPARATOR = "://";
const std::string FILE_SCHEME = "file";
const int32_t MAX_URI_COUNT = 500;
constexpr int32_t API20 = 20;
constexpr int32_t API_VERSION_MOD = 100;
constexpr uint32_t TOKEN_ID_BIT_SIZE = 32;
}

UriUtils::UriUtils() {}

UriUtils::~UriUtils() {}

UriUtils &UriUtils::GetInstance()
{
    static UriUtils utils;
    return utils;
}

bool UriUtils::IsInAncoAppIdentifier(const std::string &bundleName)
{
    auto identifier = AppUtils::GetInstance().GetAncoAppIdentifiers();
    return CheckIsInAncoAppIdentifier(identifier, bundleName);
}

bool UriUtils::CheckIsInAncoAppIdentifier(const std::string &identifier, const std::string &bundleName)
{
    if (identifier.empty() || bundleName.empty()) {
        return false;
    }
    TAG_LOGI(AAFwkTag::ABILITYMGR, "identifier:%{public}s", identifier.c_str());
    std::stringstream ss(identifier);
    std::string item;
    while (getline(ss, item, '|')) {
        if (item == bundleName) {
            return true;
        }
    }
    return false;
}

std::vector<Uri> UriUtils::GetUriListFromWantDms(Want &want)
{
    std::vector<std::string> uriStrVec = want.GetStringArrayParam(PARAMS_URI);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "uriVec size: %{public}zu", uriStrVec.size());
    if (uriStrVec.size() > MAX_URI_COUNT) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "uri list size is more than %{public}u", MAX_URI_COUNT);
        uriStrVec.resize(MAX_URI_COUNT);
    }
    std::vector<Uri> validUriVec;
    for (auto &&str : uriStrVec) {
        Uri uri(str);
        auto &&scheme = uri.GetScheme();
        TAG_LOGI(AAFwkTag::ABILITYMGR, "uri scheme: %{public}s", scheme.c_str());
        // only support file scheme
        if (scheme != "file") {
            TAG_LOGW(AAFwkTag::ABILITYMGR, "only support file uri");
            continue;
        }
        std::string srcPath = uri.GetPath();
        if (std::filesystem::exists(srcPath) && std::filesystem::is_symlink(srcPath)) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "soft links not allowed");
            continue;
        }
        std::string absolutePath;
        if (uri.IsRelative()) {
            char path[PATH_MAX] = {0};
            if (realpath(srcPath.c_str(), path) == nullptr) {
                TAG_LOGE(AAFwkTag::ABILITYMGR, "fail, errno :%{public}d", errno);
                continue;
            }
            absolutePath = path;
        } else {
            absolutePath = srcPath;
        }
        if (absolutePath.compare(0, DISTRIBUTED_FILES_PATH.size(), DISTRIBUTED_FILES_PATH) != 0) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "uri not distributed path");
            continue;
        }
        validUriVec.emplace_back(uri);
    }
    uriStrVec.clear();
    want.SetParam(PARAMS_URI, uriStrVec);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "size of vaid uri is %{public}zu", validUriVec.size());
    return validUriVec;
}

bool UriUtils::ProcessWantUri(bool checkResult, int32_t apiVersion, Want &want, std::vector<Uri> &permissionedUris)
{
    if (want.GetUriString().empty()) {
        return true;
    }
    if (checkResult) {
        permissionedUris.emplace_back(want.GetUri());
        return true;
    }
    auto scheme = want.GetUri().GetScheme();
    if (scheme == FILE_SCHEME || (scheme.empty() && apiVersion >= API20)) {
        want.SetUri("");
        TAG_LOGI(AAFwkTag::ABILITYMGR, "erase uri param.");
    }
    return false;
}

bool UriUtils::GetCallerNameAndApiVersion(uint32_t tokenId, std::string &callerName, int32_t &apiVersion)
{
    auto tokenType = Security::AccessToken::AccessTokenKit::GetTokenTypeFlag(tokenId);
    if (tokenType == Security::AccessToken::ATokenTypeEnum::TOKEN_NATIVE) {
        // for SA, caller name is process name
        Security::AccessToken::NativeTokenInfo nativeInfo;
        auto result = Security::AccessToken::AccessTokenKit::GetNativeTokenInfo(tokenId, nativeInfo);
        if (result != ERR_OK) {
            TAG_LOGE(AAFwkTag::URIPERMMGR, "GetNativeTokenInfo failed, ret:%{public}d.", result);
            return false;
        }
        callerName = nativeInfo.processName;
        return true;
    }
    if (tokenType == Security::AccessToken::ATokenTypeEnum::TOKEN_HAP) {
        // for application, caller name is bundle name
        Security::AccessToken::HapTokenInfo hapInfo;
        auto ret = Security::AccessToken::AccessTokenKit::GetHapTokenInfo(tokenId, hapInfo);
        if (ret != Security::AccessToken::AccessTokenKitRet::RET_SUCCESS) {
            TAG_LOGE(AAFwkTag::URIPERMMGR, "GetHapTokenInfo failed, ret is %{public}d", ret);
            return false;
        }
        apiVersion = hapInfo.apiVersion % API_VERSION_MOD;
        callerName = hapInfo.bundleName;
        return true;
    }
    TAG_LOGE(AAFwkTag::URIPERMMGR, "invalid tokenType: %{public}d", static_cast<int32_t>(tokenType));
    return false;
}

std::vector<Uri> UriUtils::GetPermissionedUriList(const std::vector<std::string> &uriVec,
    const std::vector<CheckResult> &checkResults, uint32_t callerTokenId,
    const std::string &targetBundleName, Want &want)
{
    std::vector<Uri> permissionedUris;
    if (uriVec.size() != checkResults.size()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Invalid param: %{public}zu : %{public}zu",
            uriVec.size(), checkResults.size());
        return permissionedUris;
    }
    std::string callerBundleName;
    int32_t apiVersion = 0;
    GetCallerNameAndApiVersion(callerTokenId, callerBundleName, apiVersion);
    // process uri
    size_t startIndex = want.GetUriString().empty() ? 0 : 1;
    if (!ProcessWantUri(checkResults[0].result, apiVersion, want, permissionedUris)) {
        SendGrantUriPermissionEvent(callerBundleName, targetBundleName, uriVec[0], apiVersion, ERASE_URI);
    }
    // process param stream
    bool eraseParamStreamEventSent = false;
    std::vector<std::string> paramStreamUris;
    for (size_t index = startIndex; index < checkResults.size(); index++) {
        // only reserve privileged file uri
        auto uri = Uri(uriVec[index]);
        if (checkResults[index].result) {
            permissionedUris.emplace_back(uri);
            paramStreamUris.emplace_back(uriVec[index]);
            continue;
        }
        if (uri.GetScheme() != FILE_SCHEME && apiVersion < API20) {
            paramStreamUris.emplace_back(uriVec[index]);
        }
        if (eraseParamStreamEventSent) {
            continue;
        }
        eraseParamStreamEventSent = true;
        SendGrantUriPermissionEvent(callerBundleName, targetBundleName, uriVec[index], apiVersion, ERASE_PARAM_STREAM);
    }
    if (paramStreamUris.size() != (checkResults.size() - startIndex)) {
        // erase old param stream and set new param stream
        want.RemoveParam(AbilityConfig::PARAMS_STREAM);
        want.SetParam(AbilityConfig::PARAMS_STREAM, paramStreamUris);
        TAG_LOGI(AAFwkTag::ABILITYMGR, "startIndex: %{public}zu, uriVec: %{public}zu, paramStreamUris: %{public}zu",
            startIndex, uriVec.size(), paramStreamUris.size());
    }
    return permissionedUris;
}

bool UriUtils::GetUriListFromWant(Want &want, std::vector<std::string> &uriVec)
{
    // remove uris out of 500
    auto uriStr = want.GetUri().ToString();
    uriVec = want.GetStringArrayParam(AbilityConfig::PARAMS_STREAM);
    if (uriVec.empty() && uriStr.empty()) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "uriVec empty.");
        return false;
    }
    // process param stream
    auto paramStreamUriCount = uriVec.size();
    if (uriStr.empty() && paramStreamUriCount > MAX_URI_COUNT) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "uri empty, paream stream counts: %{public}zu", paramStreamUriCount);
        uriVec.resize(MAX_URI_COUNT);
        want.RemoveParam(AbilityConfig::PARAMS_STREAM);
        want.SetParam(AbilityConfig::PARAMS_STREAM, uriVec);
    }
    if (!uriStr.empty() && paramStreamUriCount > MAX_URI_COUNT - 1) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "paream stream counts: %{public}zu", paramStreamUriCount);
        uriVec.resize(MAX_URI_COUNT - 1);
        want.RemoveParam(AbilityConfig::PARAMS_STREAM);
        want.SetParam(AbilityConfig::PARAMS_STREAM, uriVec);
    }
    // process uri
    if (!uriStr.empty()) {
        uriVec.insert(uriVec.begin(), uriStr);
    }
    return true;
}

#ifdef SUPPORT_UPMS
bool UriUtils::IsGrantUriPermissionFlag(uint32_t flag)
{
    return ((flag & (Want::FLAG_AUTH_READ_URI_PERMISSION | Want::FLAG_AUTH_WRITE_URI_PERMISSION)) != 0);
}
#endif // SUPPORT_UPMS

bool UriUtils::IsServiceExtensionType(AppExecFwk::ExtensionAbilityType extensionAbilityType)
{
    return extensionAbilityType == AppExecFwk::ExtensionAbilityType::SERVICE ||
        extensionAbilityType == AppExecFwk::ExtensionAbilityType::UI_SERVICE;
}

bool UriUtils::IsDmsCall(uint32_t fromTokenId)
{
    auto tokenType = Security::AccessToken::AccessTokenKit::GetTokenTypeFlag(fromTokenId);
    bool isNativeCall = tokenType == Security::AccessToken::ATokenTypeEnum::TOKEN_NATIVE;
    if (!isNativeCall) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "not native call");
        return false;
    }
    Security::AccessToken::NativeTokenInfo nativeTokenInfo;
    int32_t result = Security::AccessToken::AccessTokenKit::GetNativeTokenInfo(fromTokenId, nativeTokenInfo);
    if (result == ERR_OK && nativeTokenInfo.processName == DMS_PROCESS_NAME) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "dms ability call");
        return true;
    }
    return false;
}

#ifdef SUPPORT_UPMS
bool UriUtils::GrantDmsUriPermission(Want &want, uint32_t callerTokenId,
    std::string targetBundleName, int32_t appIndex)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto validUriVec = GetUriListFromWantDms(want);
    auto hideSensitiveType = want.GetIntParam(HIDE_SENSITIVE_TYPE, 0);
    auto ret = IN_PROCESS_CALL(UriPermissionManagerClient::GetInstance().GrantUriPermissionPrivileged(validUriVec,
        want.GetFlags(), targetBundleName, appIndex, callerTokenId, hideSensitiveType));
    if (ret != ERR_OK) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "ret is %{public}d.", ret);
        return false;
    }
    return true;
}

bool UriUtils::GrantShellUriPermission(const std::vector<std::string> &strUriVec, uint32_t flag,
    const std::string &targetPkg, int32_t appIndex)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Grant uri permission to shell.");
    std::vector<Uri> uriVec;
    for (auto&& str : strUriVec) {
        Uri uri(str);
        auto&& scheme = uri.GetScheme();
        if (scheme != FUDConstants::ANCO_SCHEME) {
            TAG_LOGI(AAFwkTag::ABILITYMGR, "Non-anco uri found in broker call: %{public}s", scheme.c_str());
            return false;
        }
        uriVec.emplace_back(uri);
    }
    uint32_t callerTokenId = IPCSkeleton::GetCallingTokenID();
    auto ret = IN_PROCESS_CALL(UriPermissionManagerClient::GetInstance().GrantUriPermissionPrivileged(
        uriVec, flag, targetPkg, appIndex, callerTokenId));
    if (ret != ERR_OK) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "grant uri to shell failed: %{public}d", ret);
    }
    return true;
}

void UriUtils::CheckUriPermission(uint32_t callerTokenId, Want &want)
{
    // Check and clear no-permission uris in want, not support content uri
    uint32_t flag = want.GetFlags();
    if (!IsGrantUriPermissionFlag(flag)) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "No grant uri flag: %{public}u.", flag);
        return;
    }
    std::vector<std::string> uriVec;
    if (!UriUtils::GetUriListFromWant(want, uriVec)) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "No file uri neet grant.");
        return;
    }
    auto checkResults = IN_PROCESS_CALL(UriPermissionManagerClient::GetInstance().CheckUriAuthorizationWithType(
        uriVec, flag, callerTokenId));
    auto permissionUris = GetPermissionedUriList(uriVec, checkResults, callerTokenId, "", want);
    if (permissionUris.empty()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "all uris not permissioned.");
        return;
    }
}

void UriUtils::GrantUriPermission(const std::vector<std::string> &uriVec, int32_t flag,
    const std::string &targetBundleName, int32_t appIndex, uint32_t initiatorTokenId)
{
    std::vector<Uri> permissionUris;
    for (auto &uriStr: uriVec) {
        Uri uri(uriStr);
        permissionUris.emplace_back(uri);
    }
    if (permissionUris.empty()) {
        return;
    }
    auto ret = IN_PROCESS_CALL(UriPermissionManagerClient::GetInstance().GrantUriPermission(permissionUris,
        flag, targetBundleName, appIndex, initiatorTokenId));
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "failed, err:%{public}d", ret);
    }

    return;
}
#endif // SUPPORT_UPMS

bool UriUtils::IsSandboxApp(uint32_t tokenId)
{
    auto tokenType = Security::AccessToken::AccessTokenKit::GetTokenTypeFlag(tokenId);
    if (tokenType == Security::AccessToken::ATokenTypeEnum::TOKEN_HAP) {
        Security::AccessToken::HapTokenInfo hapInfo;
        auto ret = Security::AccessToken::AccessTokenKit::GetHapTokenInfo(tokenId, hapInfo);
        if (ret != Security::AccessToken::AccessTokenKitRet::RET_SUCCESS) {
            TAG_LOGE(AAFwkTag::URIPERMMGR, "GetHapTokenInfo failed, ret:%{public}d", ret);
            return false;
        }
        return hapInfo.instIndex > AbilityRuntime::GlobalConstant::MAX_APP_CLONE_INDEX;
    }
    return false;
}

#ifdef SUPPORT_UPMS
bool UriUtils::GrantUriPermission(Want &want, const GrantUriPermissionInfo &grantInfo)
{
    if (!IsGrantUriPermissionFlag(grantInfo.flag)) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "No grant uri flag: %{public}u.", grantInfo.flag);
        return false;
    }
    if (grantInfo.targetBundleName.empty()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "targetBundleName empty");
        return false;
    }
    if (grantInfo.targetBundleName == AppUtils::GetInstance().GetBrokerDelegateBundleName() &&
        grantInfo.collaboratorType == CollaboratorType::OTHERS_TYPE) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "reject shell application to grant uri permission");
        return false;
    }
    if (grantInfo.callerTokenId == 0) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "callerTokenId is invalid.");
        return false;
    }
    if (grantInfo.isSandboxApp || IsSandboxApp(grantInfo.callerTokenId)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "sandbox can not grant UriPermission");
        return false;
    }
    ProcessUDMFKey(want);

    if (IsDmsCall(grantInfo.callerTokenId)) {
        return GrantDmsUriPermission(want, grantInfo.callerTokenId, grantInfo.targetBundleName, grantInfo.appIndex);
    }

    std::vector<std::string> uriVec;
    if (!UriUtils::GetUriListFromWant(want, uriVec)) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "No file uri need grant.");
        return false;
    }

    auto callerPkg = want.GetStringParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME);
    bool isBrokerCall = (IsInAncoAppIdentifier(callerPkg) ||
        IPCSkeleton::GetCallingUid() == AppUtils::GetInstance().GetCollaboratorBrokerUID());
    if (isBrokerCall && GrantShellUriPermission(uriVec, grantInfo.flag, grantInfo.targetBundleName,
        grantInfo.appIndex)) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "permission to shell");
        return true;
    }
    if (!GrantUriPermissionInner(uriVec, grantInfo, want, isBrokerCall)) {
        return false;
    }
    // report open file event
    PublishFileOpenEvent(want, grantInfo);
    return true;
}

void UriUtils::MarkContentUriAuthorizedForBroker(const std::vector<std::string> &uriVec,
    std::vector<CheckResult> &checkResults, bool isBrokerCall)
{
    if (!isBrokerCall) {
        return;
    }
    for (size_t i = 0; i < uriVec.size(); i++) {
        Uri uri(uriVec[i]);
        if (uri.GetScheme() == FUDConstants::ANCO_SCHEME) {
            checkResults[i].result = true;
            checkResults[i].permissionType = 0;
        }
    }
}

void UriUtils::ProcessUDMFKey(Want &want)
{
    // PARAMS_STREAM and UDMF_DATA_KEY is conflict
    if (!want.GetStringArrayParam(AbilityConfig::PARAMS_STREAM).empty()) {
        want.RemoveParam(Want::PARAM_ABILITY_UNIFIED_DATA_KEY);
    }
}

bool UriUtils::GrantUriPermissionInner(const std::vector<std::string> &uriVec,
    const GrantUriPermissionInfo &grantInfo, Want &want, bool isBrokerCall)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    NotifyGrantUriPermissionStart(grantInfo.isNotifyCollaborator, uriVec, grantInfo.flag, grantInfo.userId);
    auto checkResults = IN_PROCESS_CALL(UriPermissionManagerClient::GetInstance().CheckUriAuthorizationWithType(
        uriVec, grantInfo.flag, grantInfo.callerTokenId));
    MarkContentUriAuthorizedForBroker(uriVec, checkResults, isBrokerCall);
    auto permissionUris = GetPermissionedUriList(uriVec, checkResults, grantInfo.callerTokenId,
        grantInfo.targetBundleName, want);
    std::vector<bool> boolResults(checkResults.size(), false);
    if (permissionUris.empty()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "uris not permissioned.");
        NotifyGrantUriPermissionEnd(grantInfo.isNotifyCollaborator, uriVec, grantInfo.flag,
            grantInfo.userId, boolResults);
        return false;
    }
    std::vector<int32_t> permissionTypes;
    for (size_t i = 0; i < checkResults.size(); i++) {
        boolResults[i] = checkResults[i].result;
        if (checkResults[i].result) {
            permissionTypes.emplace_back(checkResults[i].permissionType);
        }
    }
    auto hideSensitiveType = want.GetIntParam(HIDE_SENSITIVE_TYPE, 0);
    auto ret = IN_PROCESS_CALL(UriPermissionManagerClient::GetInstance().GrantUriPermissionWithType(
        permissionUris, grantInfo.flag, grantInfo.targetBundleName, grantInfo.appIndex, grantInfo.callerTokenId,
        hideSensitiveType, permissionTypes));
    NotifyGrantUriPermissionEnd(grantInfo.isNotifyCollaborator, uriVec, grantInfo.flag, grantInfo.userId, boolResults);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "failed, err:%{public}d", ret);
        return false;
    }
    return true;
}
#endif // SUPPORT_UPMS

void UriUtils::PublishFileOpenEvent(const Want &want, const GrantUriPermissionInfo &grantInfo)
{
    auto wangUri = want.GetUri();
    std::string uriStr = wangUri.ToString();
    if (!uriStr.empty() && wangUri.GetScheme() == "file") {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "ability record:%{private}s,ability:%{public}s_%{public}s,userId:%{public}d",
            uriStr.c_str(), grantInfo.targetBundleName.c_str(), grantInfo.targetAbilityName.c_str(), grantInfo.userId);
        Want msgWant;
        msgWant.SetAction("file.event.OPEN_TIME");
        msgWant.SetParam("uri", uriStr);
        msgWant.SetParam("bundleName", grantInfo.targetBundleName);
        msgWant.SetParam("abilityName", grantInfo.targetAbilityName);
        auto timeNow = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        std::string currentTime = std::to_string(timeNow);
        msgWant.SetParam("viewTime", currentTime);
        msgWant.SetParam("targetUid", grantInfo.targetUid);
        msgWant.SetParam("callerUid", grantInfo.callerUid);
        EventFwk::CommonEventData commonData{msgWant};
        EventFwk::CommonEventPublishInfo commonEventPublishInfo;
        std::vector<std::string> subscriberPermissions = {"ohos.permission.MANAGE_LOCAL_ACCOUNTS"};
        commonEventPublishInfo.SetSubscriberPermissions(subscriberPermissions);
        IN_PROCESS_CALL(EventFwk::CommonEventManager::PublishCommonEventAsUser(commonData, commonEventPublishInfo,
            grantInfo.userId));
    }
}

#ifdef SUPPORT_UPMS
bool UriUtils::GrantUriPermissionForServiceExtension(const AbilityRequest &abilityRequest)
{
    if (IsServiceExtensionType(abilityRequest.abilityInfo.extensionAbilityType)) {
        auto &abilityInfo = abilityRequest.abilityInfo;
        auto &want = const_cast<Want &>(abilityRequest.want);
        auto callerTokenId = abilityRequest.specifyTokenId > 0 ? abilityRequest.specifyTokenId :
            static_cast<uint32_t>(want.GetIntParam(Want::PARAM_RESV_CALLER_TOKEN, 0));
        GrantUriPermissionInfo grantInfo;
        grantInfo.isNotifyCollaborator = false;
        grantInfo.targetBundleName = abilityInfo.bundleName;
        grantInfo.targetAbilityName = abilityInfo.name;
        grantInfo.appIndex = abilityInfo.applicationInfo.appIndex;
        grantInfo.isSandboxApp = false;
        grantInfo.callerTokenId = callerTokenId;
        grantInfo.collaboratorType = abilityRequest.collaboratorType;
        grantInfo.flag = want.GetFlags();
        grantInfo.userId = abilityRequest.userId;
        GrantUriPermission(want, grantInfo);
        return true;
    }
    return false;
}

bool UriUtils::GrantUriPermissionForUIOrServiceExtension(const AbilityRequest &abilityRequest)
{
    auto extensionType = abilityRequest.abilityInfo.extensionAbilityType;
    if (UIExtensionWrapper::IsUIExtension(extensionType) || IsServiceExtensionType(extensionType)) {
        auto &abilityInfo = abilityRequest.abilityInfo;
        auto &want = const_cast<Want &>(abilityRequest.want);
        auto callerTokenId = abilityRequest.specifyTokenId > 0 ? abilityRequest.specifyTokenId :
            static_cast<uint32_t>(want.GetIntParam(Want::PARAM_RESV_CALLER_TOKEN, 0));
        GrantUriPermissionInfo grantInfo;
        grantInfo.isNotifyCollaborator = false;
        grantInfo.targetBundleName = abilityInfo.bundleName;
        grantInfo.targetAbilityName = abilityInfo.name;
        grantInfo.appIndex = abilityInfo.applicationInfo.appIndex;
        grantInfo.isSandboxApp = false;
        grantInfo.callerTokenId = callerTokenId;
        grantInfo.collaboratorType = abilityRequest.collaboratorType;
        grantInfo.flag = want.GetFlags();
        grantInfo.userId = abilityRequest.userId;
        GrantUriPermission(want, grantInfo);
        return true;
    }
    return false;
}
#endif // SUPPORT_UPMS

bool UriUtils::SendGrantUriPermissionEvent(const std::string &callerBundleName, const std::string &targetBundleName,
    const std::string &oriUri, int32_t apiVersion, const std::string &eventType)
{
    EventInfo eventInfo;
    eventInfo.callerBundleName = callerBundleName;
    eventInfo.bundleName = targetBundleName;
    Uri uri = Uri(oriUri);
    auto scheme = uri.GetScheme();
    auto authority = uri.GetAuthority();
    eventInfo.uri = eventType + SCHEME_SEPARATOR + scheme + SEPARATOR + authority +
        SEPARATOR + std::to_string(apiVersion);
    TAG_LOGI(AAFwkTag::URIPERMMGR, "event: %{public}s", eventInfo.uri.c_str());
    EventReport::SendGrantUriPermissionEvent(EventName::GRANT_URI_PERMISSION, eventInfo);
    return true;
}

bool UriUtils::NotifyGrantUriPermissionStart(bool isNotifyCollaborator, const std::vector<std::string> &uris,
    uint32_t flag, int32_t userId)
{
    if (!isNotifyCollaborator) {
        return true;
    }
    auto abilityClient = AbilityManagerClient::GetInstance();
    if (abilityClient == nullptr) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "abilityClient null");
        return false;
    }
    auto collaborator = IN_PROCESS_CALL(abilityClient->GetAbilityManagerCollaborator());
    if (collaborator == nullptr) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "collaborator null");
        return false;
    }
    auto ret = collaborator->NotifyGrantUriPermissionStart(uris, flag, userId);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "NotifyGrantUriPermissionStart failed: %{public}d", ret);
        return false;
    }
    return true;
}

bool UriUtils::NotifyGrantUriPermissionEnd(bool isNotifyCollaborator, const std::vector<std::string> &uris,
    uint32_t flag, int32_t userId, const std::vector<bool> &checkResults)
{
    if (!isNotifyCollaborator) {
        return true;
    }
    auto abilityClient = AbilityManagerClient::GetInstance();
    if (abilityClient == nullptr) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "abilityClient null");
        return false;
    }
    auto collaborator = IN_PROCESS_CALL(abilityClient->GetAbilityManagerCollaborator());
    if (collaborator == nullptr) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "collaborator null");
        return false;
    }
    auto ret = collaborator->NotifyGrantUriPermissionEnd(uris, flag, userId, checkResults);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "NotifyGrantUriPermissionEnd failed: %{public}d", ret);
        return false;
    }
    return true;
}
} // AAFwk
} // OHOS
