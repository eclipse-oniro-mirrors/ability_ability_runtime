/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

#include "uri_permission_manager_stub_impl.h"

#include "ability_manager_errors.h"
#include "ability_manager_client.h"
#include "accesstoken_kit.h"
#include "app_utils.h"
#include "file_uri_distribution_utils.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "file_permission_manager.h"
#include "if_system_ability_manager.h"
#include "in_process_call_wrapper.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#ifdef ABILITY_RUNTIME_MEDIA_LIBRARY_ENABLE
#include "media_permission_manager.h"
#endif // ABILITY_RUNTIME_MEDIA_LIBRARY_ENABLE
#include "parameter.h"
#include "permission_constants.h"
#include "permission_verification.h"
#ifdef ABILITY_RUNTIME_FEATURE_SANDBOXMANAGER
#include "sandbox_manager_kit.h"
#endif // ABILITY_RUNTIME_FEATURE_SANDBOXMANAGER
#include "system_ability_definition.h"
#include "tokenid_kit.h"
#include "fud_constants.h"
#ifdef ABILITY_RUNTIME_UDMF_ENABLE
#include "upms_udmf_utils.h"
#endif // ABILITY_RUNTIME_UDMF_ENABLE
#include "want.h"

namespace OHOS {
namespace AAFwk {
namespace {
constexpr int32_t ERR_OK = 0;
constexpr uint32_t FLAG_READ_WRITE_URI = Want::FLAG_AUTH_READ_URI_PERMISSION | Want::FLAG_AUTH_WRITE_URI_PERMISSION;
constexpr uint32_t FLAG_WRITE_URI = Want::FLAG_AUTH_WRITE_URI_PERMISSION;
constexpr uint32_t FLAG_READ_URI = Want::FLAG_AUTH_READ_URI_PERMISSION;
constexpr uint32_t FLAG_PERSIST_URI = Want::FLAG_AUTH_PERSISTABLE_URI_PERMISSION;
constexpr const char* DISTRIBUTED_DOCS_URI_MARK = "?networkid=";
constexpr size_t MAX_IPC_RAW_DATA_SIZE = 128 * 1024 * 1024; // 128M
const int MAX_URI_COUNT = 200000;
#ifndef ABILITY_RUNTIME_MEDIA_LIBRARY_ENABLE
constexpr int32_t CAPABILITY_NOT_SUPPORT = 801;
#endif // ABILITY_RUNTIME_MEDIA_LIBRARY_ENABLE
#ifdef ABILITY_RUNTIME_FEATURE_SANDBOXMANAGER
constexpr int32_t SANDBOX_MANAGER_PERMISSION_DENIED = 1;
#endif // ABILITY_RUNTIME_FEATURE_SANDBOXMANAGER
}

inline int32_t UriPermissionManagerStubImpl::WrapErrorCode(int32_t errorCode, int32_t &funcRet)
{
    funcRet = errorCode;
    return ERR_OK;
}

ErrCode UriPermissionManagerStubImpl::VerifyUriPermission(const Uri& uri, uint32_t flag, uint32_t tokenId,
    bool& funcResult)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    // verify if tokenId have uri permission record
    TAG_LOGI(AAFwkTag::URIPERMMGR, "uri:%{private}s, flag:%{public}u, tokenId:%{public}u",
        uri.ToString().c_str(), flag, tokenId);
    if (!FUDUtils::IsDFSCall()) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "no permission call");
        funcResult = false;
        return ERR_OK;
    }
    funcResult = VerifyUriPermissionInner(uri, flag, tokenId);
    return ERR_OK;
}

bool UriPermissionManagerStubImpl::VerifyUriPermissionInner(const Uri& uri, uint32_t flag, uint32_t tokenId)
{
    if ((flag & FLAG_READ_WRITE_URI) == 0) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Flag invalid");
        return false;
    }
    // only reserve read and write file flag
    flag &= FLAG_READ_WRITE_URI;
    auto uriInner = uri;
    if (uriInner.GetScheme() != FUDConstants::FILE_SCHEME) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "type of uri is valid");
        return false;
    }
    if (uriInner.GetAuthority() == FUDConstants::MEDIA_AUTHORITY) {
        TAG_LOGW(AAFwkTag::URIPERMMGR, "not support media uri");
        return false;
    }
    std::vector<Uri> uriVec = { uriInner };
    if (FUDUtils::IsDocsCloudUri(uriInner)) {
        auto result = VerifyUriPermissionByMap(uriVec, flag, tokenId);
        if (!result[0]) {
            TAG_LOGI(AAFwkTag::URIPERMMGR, "uri permission not exists.");
        }
        return result[0];
    }
    auto policyInfo = FilePermissionManager::GetPathPolicyInfoFromUri(uriInner, flag);
    std::vector<PolicyInfo> policyVec = { policyInfo };
    auto result = VerifyUriPermissionByPolicy(policyVec, flag, tokenId);
    if (!result[0]) {
        TAG_LOGI(AAFwkTag::URIPERMMGR, "uri permission not exists, path:%{private}s", policyInfo.path.c_str());
    }
    return result[0];
}

std::vector<bool> UriPermissionManagerStubImpl::VerifyUriPermissionByPolicy(std::vector<PolicyInfo> &policyVec,
    uint32_t flag, uint32_t tokenId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGI(AAFwkTag::URIPERMMGR, "VerifyUriPermissionByPolicy called, %{public}zu policyVec", policyVec.size());
    std::vector<bool> result;
#ifdef ABILITY_RUNTIME_FEATURE_SANDBOXMANAGER
    auto ret = SandboxManagerKit::CheckPolicy(tokenId, policyVec, result);
    if (ret != ERR_OK || result.size() != policyVec.size()) {
        TAG_LOGW(AAFwkTag::URIPERMMGR, "Check policy failed: %{public}d.", ret);
        result = std::vector<bool>(policyVec.size(), false);
        return result;
    }
    for (size_t i = 0; i < result.size(); i++) {
        if (!result[i]) {
            TAG_LOGI(AAFwkTag::URIPERMMGR, "no policy record.");
            break;
        }
    }
#else
    result = std::vector<bool>(policyVec.size(), true);
#endif // ABILITY_RUNTIME_FEATURE_SANDBOXMANAGER
    return result;
}

std::vector<bool> UriPermissionManagerStubImpl::VerifyUriPermissionByMap(std::vector<Uri> &uriVec,
    uint32_t flag, uint32_t tokenId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    uint32_t newFlag = FLAG_READ_URI;
    if ((flag & FLAG_WRITE_URI) != 0) {
        newFlag = FLAG_WRITE_URI;
    }
    std::vector<bool> result(uriVec.size(), false);
    for (size_t i = 0; i < uriVec.size(); i++) {
        auto uriStr = uriVec[i].ToString();
        result[i] = VerifySingleUriPermissionByMap(uriStr, newFlag, tokenId);
    }
    return result;
}

bool UriPermissionManagerStubImpl::VerifySingleUriPermissionByMap(const std::string &uri,
    uint32_t flag, uint32_t tokenId)
{
    std::lock_guard<std::mutex> guard(mutex_);
    auto search = uriMap_.find(uri);
    if (search != uriMap_.end()) {
        auto& list = search->second;
        for (auto it = list.begin(); it != list.end(); it++) {
            if ((it->targetTokenId == tokenId) && ((it->flag | FLAG_READ_URI) & flag) != 0) {
                TAG_LOGD(AAFwkTag::URIPERMMGR, "have uri permission");
                return true;
            }
        }
    }
    return VerifySubDirUriPermission(uri, flag, tokenId);
}

bool UriPermissionManagerStubImpl::VerifySubDirUriPermission(const std::string &uriStr,
                                                             uint32_t newFlag, uint32_t tokenId)
{
    auto iPos = uriStr.find(DISTRIBUTED_DOCS_URI_MARK);
    if (iPos == std::string::npos) {
        TAG_LOGI(AAFwkTag::URIPERMMGR, "Local uri not support to verify sub directory uri permission");
        return false;
    }

    for (auto search = uriMap_.rbegin(); search != uriMap_.rend(); ++search) {
        if (!IsDistributedSubDirUri(uriStr, search->first)) {
            continue;
        }
        auto& list = search->second;
        for (auto it = list.begin(); it != list.end(); it++) {
            if ((it->targetTokenId == tokenId) && ((it->flag | FLAG_READ_URI) & newFlag) != 0) {
                TAG_LOGD(AAFwkTag::URIPERMMGR, "have uri permission");
                return true;
            }
        }
        break;
    }
    TAG_LOGI(AAFwkTag::URIPERMMGR, "Uri permission not exists");
    return false;
}

bool UriPermissionManagerStubImpl::IsDistributedSubDirUri(const std::string &inputUri, const std::string &cachedUri)
{
    auto iPos = inputUri.find(DISTRIBUTED_DOCS_URI_MARK);
    auto cPos = cachedUri.find(DISTRIBUTED_DOCS_URI_MARK);
    if ((iPos == std::string::npos) || (cPos == std::string::npos)) {
        TAG_LOGI(AAFwkTag::URIPERMMGR, "not distributed file uri");
        return false;
    }
    std::string iTempUri = inputUri.substr(0, iPos);
    std::string cTempUri = cachedUri.substr(0, cPos);
    std::string iTempNetworkId = inputUri.substr(iPos);
    std::string cTempNetworkId = cachedUri.substr(cPos);
    return (iTempUri.find(cTempUri + "/") == 0 && iTempNetworkId.compare(cTempNetworkId) == 0);
}

ErrCode UriPermissionManagerStubImpl::GrantUriPermission(const Uri& uri, uint32_t flag,
    const std::string& targetBundleName, int32_t appIndex, uint32_t initiatorTokenId, int32_t& funcResult)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "Uri:%{private}s", uri.ToString().c_str());
    Uri tempUri = uri;
    if (FUDUtils::IsSystemAppCall() && tempUri.GetScheme() != FUDConstants::FILE_SCHEME) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Only support file uri");
        return WrapErrorCode(ERR_CODE_INVALID_URI_TYPE, funcResult);
    }
    std::vector<std::string> uriVec = { uri.ToString() };
    GrantUriPermission(uriVec, flag, targetBundleName, appIndex, initiatorTokenId, funcResult);
    return ERR_OK;
}

ErrCode UriPermissionManagerStubImpl::CheckGrantUriPermission(const std::vector<std::string>& uriVec, uint32_t flag,
    const std::string& targetBundleName, int32_t appIndex)
{
    if (!FUDUtils::IsSAOrSystemAppCall()) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "not SA/SystemApp");
        return ERR_NOT_SYSTEM_APP;
    }
    auto checkResult = CheckCalledBySandBox();
    if (checkResult != ERR_OK) {
        return checkResult;
    }
    if (uriVec.empty() || uriVec.size() > MAX_URI_COUNT) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "out of range: %{public}zu", uriVec.size());
        return ERR_URI_LIST_OUT_OF_RANGE;
    }
    if ((flag & FLAG_READ_WRITE_URI) == 0) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "invalid flag: %{public}u", flag);
        return ERR_CODE_INVALID_URI_FLAG;
    }
    return ERR_OK;
}

ErrCode UriPermissionManagerStubImpl::GrantUriPermission(const std::vector<std::string>& uriVec, uint32_t flag,
    const std::string& targetBundleName, int32_t appIndex, uint32_t initiatorTokenId, int32_t& funcResult)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGI(AAFwkTag::URIPERMMGR, "BundleName:%{public}s, appIndex:%{public}d, flag:%{public}u, uris:%{public}zu",
        targetBundleName.c_str(), appIndex, flag, uriVec.size());
    auto ret = CheckGrantUriPermission(uriVec, flag, targetBundleName, appIndex);
    if (ret != ERR_OK) {
        return WrapErrorCode(ret, funcResult);
    }
    uint32_t targetTokenId = 0;
    ret = FUDUtils::GetTokenIdByBundleName(targetBundleName, appIndex, targetTokenId);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "get tokenId by bundle name failed");
        return WrapErrorCode(ret, funcResult);
    }
    uint32_t callerTokenId = initiatorTokenId;
    if (!FUDUtils::IsPrivilegedSACall()) {
        callerTokenId = IPCSkeleton::GetCallingTokenID();
    }
    // split uris by uri authority
    BatchUri batchUri;
    // Compatible grant write permission can read file for sandboxManager.
    auto rwMode = (flag | FLAG_READ_URI) & (FLAG_READ_WRITE_URI | FLAG_PERSIST_URI);
    std::string callerAlterableBundleName = "";
    FUDUtils::GetAlterableBundleNameByTokenId(callerTokenId, callerAlterableBundleName);
    std::string targetAlterableBundleName = "";
    FUDUtils::GetDirByBundleNameAndAppIndex(targetBundleName, appIndex, targetAlterableBundleName);
    bool haveSandboxAccessPermission = PermissionVerification::GetInstance()->VerifyPermissionByTokenId(callerTokenId,
        PermissionConstants::PERMISSION_SANDBOX_ACCESS_MANAGER);
    if (batchUri.Init(uriVec, rwMode, callerAlterableBundleName, targetAlterableBundleName,
        haveSandboxAccessPermission) == 0) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "All uri is invalid.");
        return WrapErrorCode(ERR_CODE_INVALID_URI_TYPE, funcResult);
    }
    auto checkRet = CheckUriPermission(batchUri, flag, callerTokenId, callerAlterableBundleName, targetTokenId);
    if (checkRet != ERR_OK) {
        // udmf and pasteboard support content and dfs docs uri
        if (!FUDUtils::IsUdmfOrPasteboardCall() || batchUri.contentUris.empty()) {
            return WrapErrorCode(checkRet, funcResult);
        }
    }
    FUDAppInfo callerInfo = { .tokenId = callerTokenId, .alterBundleName = callerAlterableBundleName };
    FUDAppInfo targetInfo = { .tokenId = targetTokenId, .bundleName = targetBundleName,
        .alterBundleName = targetAlterableBundleName };
    auto grantRet = GrantUriPermissionInner(batchUri, uriVec, flag, callerInfo, targetInfo);
    TAG_LOGI(AAFwkTag::URIPERMMGR, "Grant UriPermission finished.");
    return WrapErrorCode(grantRet, funcResult);
}

ErrCode UriPermissionManagerStubImpl::GrantUriPermission(const UriPermissionRawData& rawData, uint32_t flag,
    const std::string& targetBundleName, int32_t appIndex, uint32_t initiatorTokenId, int32_t& funcResult)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::vector<std::string> uriVec;
    auto res = RawDataToStringVec(rawData, uriVec);
    if (res != ERR_OK) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "raw data to vec failed");
        funcResult = res;
        return ERR_OK;
    }
    auto errCode = GrantUriPermission(uriVec, flag, targetBundleName, appIndex, initiatorTokenId, funcResult);
    if (errCode != ERR_OK) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "GrantUriPermission failed, errCode:%{public}d", errCode);
        return errCode;
    }
    return ERR_OK;
}

int32_t UriPermissionManagerStubImpl::GrantUriPermissionInner(BatchUri &batchUri,
    const std::vector<std::string> &uriVec, uint32_t flag, const FUDAppInfo &callerInfo, const FUDAppInfo &targetInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGI(AAFwkTag::URIPERMMGR, "callerTokenId:%{public}u, targetTokenId:%{public}u",
        callerInfo.tokenId, targetInfo.tokenId);
    int32_t grantRet = INNER_ERR;
    if (batchUri.targetBundleUriCount > 0) {
        grantRet = ERR_OK;
    }
    // media
    std::vector<std::string> mediaUriVec;
    if (batchUri.GetMediaUriToGrant(mediaUriVec) > 0) {
        if (GrantBatchMediaUriPermissionImpl(mediaUriVec, flag, callerInfo.tokenId, targetInfo.tokenId, 0) == ERR_OK) {
            grantRet = ERR_OK;
        }
    }
    std::vector<PolicyInfo> docspolicyVec;
    std::vector<PolicyInfo> bundlepolicyVec;
    batchUri.GetUriToGrantByPolicy(docspolicyVec, bundlepolicyVec);
    TAG_LOGI(AAFwkTag::URIPERMMGR, "bundle policy:%{public}zu, docs policy:%{public}zu, content uris:%{public}zu",
        bundlepolicyVec.size(), docspolicyVec.size(), batchUri.contentUris.size());
    // bundle
    if (GrantBatchUriPermissionImplByPolicy(bundlepolicyVec, flag, callerInfo, targetInfo) == ERR_OK) {
        grantRet = ERR_OK;
    }
    // docs
    if (GrantBatchUriPermissionImplByPolicy(docspolicyVec, flag, callerInfo, targetInfo) == ERR_OK) {
        grantRet = ERR_OK;
    }
    if (!FUDUtils::IsUdmfOrPasteboardCall()) {
        FUDUtils::SendSystemAppGrantUriPermissionEvent(callerInfo.tokenId, targetInfo.tokenId, uriVec,
            batchUri.checkResult);
        return grantRet;
    }
    if (GrantBatchContentUriPermissionImpl(batchUri.contentUris, flag, targetInfo.tokenId,
        targetInfo.bundleName) == ERR_OK) {
        grantRet = ERR_OK;
    }
    return grantRet;
}

int32_t UriPermissionManagerStubImpl::GrantBatchUriPermissionImplByPolicy(const std::vector<PolicyInfo> &policyInfoVec,
    uint32_t flag, const FUDAppInfo &callerInfo, const FUDAppInfo &targetInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (policyInfoVec.empty()) {
        return INNER_ERR;
    }
#ifdef ABILITY_RUNTIME_FEATURE_SANDBOXMANAGER
    std::vector<uint32_t> result;
    auto ret = SandboxManagerSetPolicy(policyInfoVec, flag, callerInfo, targetInfo, result);
    if (ret != ERR_OK) {
        return ret;
    }
    std::lock_guard<std::mutex> lock(ptMapMutex_);
    permissionTokenMap_.insert(targetInfo.tokenId);
    int successCount = 0;
    // for revoke uri permission manually
    bool needCachePolicyPermissionInfo = !FUDUtils::IsFoundationCall();
    for (size_t i = 0; i < result.size(); i++) {
        if (result[i] != ERR_OK) {
            ret = result[i];
            continue;
        }
        TAG_LOGD(AAFwkTag::URIPERMMGR, "Insert an uri policy info, path is %{private}s",
            policyInfoVec[i].path.c_str());
        if (needCachePolicyPermissionInfo) {
            AddPolicyRecordCache(callerInfo.tokenId, targetInfo.tokenId, policyInfoVec[i].path);
        }
        successCount++;
    }
    if (successCount != policyInfoVec.size()) {
        TAG_LOGW(AAFwkTag::URIPERMMGR, "SetPolicy failed:%{public}d, ret:%{public}d", successCount, ret);
    } else {
        TAG_LOGI(AAFwkTag::URIPERMMGR, "Total %{public}d path policy added.", successCount);
    }
    if (successCount == 0) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Grant uri policy failed.");
        return INNER_ERR;
    }
#endif // ABILITY_RUNTIME_FEATURE_SANDBOXMANAGER
    return ERR_OK;
}

#ifdef ABILITY_RUNTIME_FEATURE_SANDBOXMANAGER
int32_t UriPermissionManagerStubImpl::SandboxManagerSetPolicy(const std::vector<PolicyInfo> &policyInfoVec,
    uint32_t flag, const FUDAppInfo &callerInfo, const FUDAppInfo &targetInfo, std::vector<uint32_t> &result)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (policyInfoVec.empty()) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Invalid policyInfoVec");
        return INNER_ERR;
    }
    uint32_t policyFlag = (flag & FLAG_PERSIST_URI) == 0 ? 0 : IS_POLICY_ALLOWED_TO_BE_PRESISTED;
    SetInfo setInfo;
    setInfo.bundleName = callerInfo.alterBundleName;
    setInfo.timestamp = static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(
    std::chrono::high_resolution_clock::now().time_since_epoch()).count());
    TAG_LOGI(AAFwkTag::URIPERMMGR, "SetPolicy: %{public}zu,%{public}u,%{public}s,%{public}d",
        policyInfoVec.size(), policyFlag, setInfo.bundleName.c_str(), policyInfoVec[0].type);
    auto ret = IN_PROCESS_CALL(SandboxManagerKit::SetPolicy(targetInfo.tokenId, policyInfoVec,
        policyFlag, result, setInfo));
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "SetPolicy failed,ret:%{public}d.", ret);
        return INNER_ERR;
    }
    if (result.size() != policyInfoVec.size()) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "SetPolicy failed,invalid result:%{public}zu,%{public}zu",
            result.size(), policyInfoVec.size());
        return INNER_ERR;
    }
    return ERR_OK;
}
#endif

void UriPermissionManagerStubImpl::AddPolicyRecordCache(uint32_t callerTokenId, uint32_t targetTokenId,
    const std::string &path)
{
    if (FUDUtils::IsUdmfOrPasteboardCall()) {
        callerTokenId = IPCSkeleton::GetCallingTokenID();
    }
    GrantPolicyInfo grantPolicyInfo = { callerTokenId, targetTokenId };
    std::lock_guard<std::mutex> guard(policyMapMutex_);
    auto it = policyMap_.find(path);
    if (it == policyMap_.end()) {
        std::list<GrantPolicyInfo> grantPolicyInfoList = { grantPolicyInfo };
        policyMap_.emplace(path, grantPolicyInfoList);
        return;
    }
    auto &grantPolicyInfoList = it->second;
    for (auto it = grantPolicyInfoList.begin(); it != grantPolicyInfoList.end(); it++) {
        if (it->Equal(callerTokenId, targetTokenId)) {
            return;
        }
    }
    grantPolicyInfoList.emplace_back(grantPolicyInfo);
    return;
}

int32_t UriPermissionManagerStubImpl::GrantBatchMediaUriPermissionImpl(const std::vector<std::string> &mediaUris,
    uint32_t flag, uint32_t callerTokenId, uint32_t targetTokenId, int32_t hideSensitiveType)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
#ifdef ABILITY_RUNTIME_MEDIA_LIBRARY_ENABLE
    if (mediaUris.empty()) {
        return INNER_ERR;
    }
    auto ret = MediaPermissionManager::GetInstance().GrantUriPermission(mediaUris, flag, callerTokenId, targetTokenId,
        hideSensitiveType);
    if (ret != ERR_OK) {
        TAG_LOGW(AAFwkTag::URIPERMMGR, "Grant media uri permission failed, ret:%{public}d", ret);
        return ret;
    }
    TAG_LOGD(AAFwkTag::URIPERMMGR, "Grant media uri permission success");
    return ERR_OK;
#else
    return CAPABILITY_NOT_SUPPORT;
#endif // ABILITY_RUNTIME_MEDIA_LIBRARY_ENABLE
}

int32_t UriPermissionManagerStubImpl::GrantBatchContentUriPermissionImpl(const std::vector<std::string> &contentUris,
    uint32_t flag, uint32_t targetTokenId, const std::string &targetBundleName)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (contentUris.empty()) {
        return INNER_ERR;
    }
    TAG_LOGI(AAFwkTag::URIPERMMGR, "contentUris: %{public}zu", contentUris.size());
    auto abilityClient = AAFwk::AbilityManagerClient::GetInstance();
    if (abilityClient == nullptr) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "abilityClient null");
        return INNER_ERR;
    }
    auto collaborator = IN_PROCESS_CALL(abilityClient->GetAbilityManagerCollaborator());
    if (collaborator == nullptr) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "collaborator null");
        return INNER_ERR;
    }
    auto ret = collaborator->GrantUriPermission(contentUris, flag, targetTokenId, targetBundleName);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "GrantUriPermission failed:%{public}d", ret);
        return ret;
    }
    AddContentTokenIdRecord(targetTokenId);
    return ERR_OK;
}

int32_t UriPermissionManagerStubImpl::RevokeContentUriPermission(uint32_t tokenId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::URIPERMMGR, "RevokeContentUriPermission called");
    if (!IsContentUriGranted(tokenId)) {
        return ERR_OK;
    }
    auto abilityClient = AAFwk::AbilityManagerClient::GetInstance();
    if (abilityClient == nullptr) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "abilityClient null");
        return INNER_ERR;
    }
    auto collaborator = IN_PROCESS_CALL(abilityClient->GetAbilityManagerCollaborator());
    if (collaborator == nullptr) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "collaborator null");
        return INNER_ERR;
    }
    auto ret = collaborator->RevokeUriPermission(tokenId);
    if (ret != ERR_OK) {
        TAG_LOGW(AAFwkTag::URIPERMMGR, "RevokeUriPermission failed:%{public}d", ret);
    }
    RemoveContentTokenIdRecord(tokenId);
    return ret;
}

bool UriPermissionManagerStubImpl::IsContentUriGranted(uint32_t tokenId)
{
    std::lock_guard<std::mutex> lock(contentTokenIdSetMutex_);
    return contentTokenIdSet_.find(tokenId) != contentTokenIdSet_.end();
}

void UriPermissionManagerStubImpl::AddContentTokenIdRecord(uint32_t tokenId)
{
    std::lock_guard<std::mutex> lock(contentTokenIdSetMutex_);
    contentTokenIdSet_.insert(tokenId);
}

void UriPermissionManagerStubImpl::RemoveContentTokenIdRecord(uint32_t tokenId)
{
    std::lock_guard<std::mutex> lock(contentTokenIdSetMutex_);
    contentTokenIdSet_.erase(tokenId);
}

void UriPermissionManagerStubImpl::StringVecToRawData(const std::vector<std::string> &stringVec,
    StorageFileRawData &rawData)
{
    std::stringstream ss;
    uint32_t stringCount = stringVec.size();
    ss.write(reinterpret_cast<const char*>(&stringCount), sizeof(stringCount));

    for (uint32_t i = 0; i < stringCount; ++i) {
        uint32_t strLen = stringVec[i].length();
        ss.write(reinterpret_cast<const char*>(&strLen), sizeof(strLen));
        ss.write(stringVec[i].c_str(), strLen);
    }
    std::string result = ss.str();
    rawData.ownedData = std::move(result);
    rawData.data = rawData.ownedData.data();
    rawData.size = rawData.ownedData.size();
}

int32_t UriPermissionManagerStubImpl::GrantBatchUriPermissionImpl(const std::vector<std::string> &uriVec,
    uint32_t flag, TokenId callerTokenId, TokenId targetTokenId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (uriVec.empty()) {
        return INNER_ERR;
    }
    TAG_LOGI(AAFwkTag::URIPERMMGR, "privileged uris: %{public}zu", uriVec.size());
    // only reserve read and write file flag
    flag &= FLAG_READ_WRITE_URI;
    ConnectManager(storageManager_, STORAGE_MANAGER_MANAGER_ID);
    if (storageManager_ == nullptr) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "null ConnectManager");
        return INNER_ERR;
    }
    std::vector<int32_t> resVec;
    StorageFileRawData uriRawData;
    StringVecToRawData(uriVec, uriRawData);
    storageManager_->CreateShareFile(uriRawData, targetTokenId, flag, resVec);
    if (resVec.size() == 0) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "CreateShareFile failed, storageManager resVec empty");
        return INNER_ERR;
    }
    if (resVec.size() != uriVec.size()) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "CreateShareFile failed, invalid resVec, ret:%{public}u", resVec[0]);
        return resVec[0];
    }
    int successCount = 0;
    for (size_t i = 0; i < uriVec.size(); i++) {
        auto ret = resVec[i];
        if (ret != 0 && ret != -EEXIST && successCount == 0) {
            TAG_LOGE(AAFwkTag::URIPERMMGR, "CreateShareFile failed, ret:%{public}d", ret);
            continue;
        }
        AddTempUriPermission(uriVec[i], flag, callerTokenId, targetTokenId);
        successCount++;
    }
    TAG_LOGI(AAFwkTag::URIPERMMGR, "total %{public}d uri permissions added", successCount);
    if (successCount == 0) {
        return INNER_ERR;
    }
    // index that targetTokenId is granted
    std::lock_guard<std::mutex> lock(ptMapMutex_);
    permissionTokenMap_.insert(targetTokenId);
    return ERR_OK;
}

int32_t UriPermissionManagerStubImpl::AddTempUriPermission(const std::string &uri, uint32_t flag,
    TokenId fromTokenId, TokenId targetTokenId)
{
    std::lock_guard<std::mutex> guard(mutex_);
    auto search = uriMap_.find(uri);
    GrantInfo info = { flag, fromTokenId, targetTokenId };
    if (search == uriMap_.end()) {
        std::list<GrantInfo> infoList = { info };
        uriMap_.emplace(uri, infoList);
        return ERR_OK;
    }
    auto& infoList = search->second;
    for (auto& item : infoList) {
        if (item.fromTokenId == fromTokenId && item.targetTokenId == targetTokenId) {
            TAG_LOGD(AAFwkTag::URIPERMMGR, "Item: flag:%{public}u", item.flag);
            if ((item.flag & flag) != flag) {
                item.flag |= flag;
                return ERR_OK;
            }
            return ERR_OK;
        }
    }
    infoList.emplace_back(info);
    return ERR_OK;
}

ErrCode UriPermissionManagerStubImpl::CheckGrantUriPermissionPrivileged(uint32_t callerTokenId, uint32_t flag)
{
    auto permissionName = PermissionConstants::PERMISSION_GRANT_URI_PERMISSION_PRIVILEGED;
    if (!PermissionVerification::GetInstance()->VerifyPermissionByTokenId(callerTokenId, permissionName)) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "No permission to call");
        return CHECK_PERMISSION_FAILED;
    }
    if ((flag & FLAG_READ_WRITE_URI) == 0) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "invalid flag:%{public}u", flag);
        return ERR_CODE_INVALID_URI_FLAG;
    }
    return ERR_OK;
}

ErrCode UriPermissionManagerStubImpl::GrantUriPermissionPrivileged(const std::vector<std::string>& uriVec,
    uint32_t flag, const std::string& targetBundleName, int32_t appIndex, uint32_t initiatorTokenId,
    int32_t hideSensitiveType, int32_t& funcResult)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (uriVec.size() == 0 || uriVec.size() > MAX_URI_COUNT) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "out of range: %{public}zu", uriVec.size());
        funcResult = ERR_URI_LIST_OUT_OF_RANGE;
        return ERR_URI_LIST_OUT_OF_RANGE;
    }
    TAG_LOGI(AAFwkTag::URIPERMMGR, "BundleName:%{public}s, appIndex:%{public}d, flag:%{public}u, uris:%{public}zu",
        targetBundleName.c_str(), appIndex, flag, uriVec.size());
    uint32_t callerTokenId = IPCSkeleton::GetCallingTokenID();
    auto checkRes = CheckGrantUriPermissionPrivileged(callerTokenId, flag);
    if (checkRes != ERR_OK) {
        return WrapErrorCode(checkRes, funcResult);
    }
    uint32_t targetTokenId = 0;
    auto ret = FUDUtils::GetTokenIdByBundleName(targetBundleName, appIndex, targetTokenId);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "get tokenId failed, bundleName:%{public}s", targetBundleName.c_str());
        return WrapErrorCode(ret, funcResult);
    }
    if (FUDUtils::IsFoundationCall()) {
        callerTokenId = initiatorTokenId;
    } else {
        // hideSensitiveType is only support for foundation
        hideSensitiveType = 0;
    }
    std::string targetAlterBundleName = targetBundleName;
    FUDUtils::GetDirByBundleNameAndAppIndex(targetBundleName, appIndex, targetAlterBundleName);
    std::vector<Uri> uriVecInner;
    for (auto& uri : uriVec) {
        uriVecInner.emplace_back(uri);
    }
    std::string callerAlterableBundleName = "";
    FUDUtils::GetAlterableBundleNameByTokenId(callerTokenId, callerAlterableBundleName);
    FUDAppInfo callerAppInfo = { .tokenId = callerTokenId, .alterBundleName = callerAlterableBundleName };
    FUDAppInfo targetAppInfo = {targetTokenId, targetBundleName, targetAlterBundleName};
    std::vector<int32_t> permissionTypes(uriVec.size(), 0);
    ret = GrantUriPermissionPrivilegedInner(uriVecInner, flag, callerAppInfo, targetAppInfo,
        hideSensitiveType, permissionTypes);
    TAG_LOGI(AAFwkTag::URIPERMMGR, "GrantUriPermissionPrivileged finished.");
    return WrapErrorCode(ret, funcResult);
}

ErrCode UriPermissionManagerStubImpl::GrantUriPermissionPrivileged(const UriPermissionRawData& rawData, uint32_t flag,
    const std::string& targetBundleName, int32_t appIndex, uint32_t initiatorTokenId, int32_t hideSensitiveType,
    int32_t& funcResult)
{
    std::vector<std::string> uriStrVec;
    auto res = RawDataToStringVec(rawData, uriStrVec);
    if (res != ERR_OK) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "raw data to vec failed");
        funcResult = res;
        return res;
    }
    auto errCode = GrantUriPermissionPrivileged(uriStrVec, flag, targetBundleName, appIndex, initiatorTokenId,
        hideSensitiveType, funcResult);
    if (errCode != ERR_OK) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "GrantUriPermissionPrivileged failed, errCode:%{public}d", errCode);
        return errCode;
    }
    return ERR_OK;
}

ErrCode UriPermissionManagerStubImpl::GrantUriPermissionWithType(const std::vector<Uri> &uriVec, uint32_t flag,
    const std::string &targetBundleName, int32_t appIndex, uint32_t initiatorTokenId, int32_t hideSensitiveType,
    const std::vector<int32_t> &permissionTypes, int32_t &funcResult)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (!FUDUtils::IsFoundationCall()) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Not foundation call");
        return WrapErrorCode(CHECK_PERMISSION_FAILED, funcResult);
    }
    TAG_LOGI(AAFwkTag::URIPERMMGR, "BundleName:%{public}s, appIndex:%{public}d, flag:%{public}u, uris:%{public}zu",
        targetBundleName.c_str(), appIndex, flag, uriVec.size());
    if (uriVec.empty() || uriVec.size() > MAX_URI_COUNT || uriVec.size() != permissionTypes.size()) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Out of range: %{public}zu,%{public}zu", uriVec.size(), permissionTypes.size());
        return WrapErrorCode(ERR_URI_LIST_OUT_OF_RANGE, funcResult);
    }
    if ((flag & FLAG_READ_WRITE_URI) == 0) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Invalid flag:%{public}u", flag);
        return WrapErrorCode(ERR_CODE_INVALID_URI_FLAG, funcResult);
    }
    if (initiatorTokenId == 0) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Invalid initiatorTokenId");
        return WrapErrorCode(ERR_UPMS_INVALID_CALLER_TOKENID, funcResult);
    }
    uint32_t targetTokenId = 0;
    auto ret = FUDUtils::GetTokenIdByBundleName(targetBundleName, appIndex, targetTokenId);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Get tokenId failed, bundleName:%{public}s", targetBundleName.c_str());
        return WrapErrorCode(ret, funcResult);
    }

    std::string targetAlterBundleName = targetBundleName;
    FUDUtils::GetDirByBundleNameAndAppIndex(targetBundleName, appIndex, targetAlterBundleName);
    FUDAppInfo targetAppInfo = {targetTokenId, targetBundleName, targetAlterBundleName};

    std::string callerAlterableBundleName = "";
    FUDUtils::GetAlterableBundleNameByTokenId(initiatorTokenId, callerAlterableBundleName);
    FUDAppInfo callerAppInfo = { .tokenId = initiatorTokenId, .alterBundleName = callerAlterableBundleName };

    ret = GrantUriPermissionPrivilegedInner(uriVec, flag, callerAppInfo, targetAppInfo, hideSensitiveType,
        permissionTypes);
    TAG_LOGI(AAFwkTag::URIPERMMGR, "GrantUriPermissionWithType finished.");
    return WrapErrorCode(ret, funcResult);
}

int32_t UriPermissionManagerStubImpl::GrantUriPermissionPrivilegedInner(const std::vector<Uri> &uriVec, uint32_t flag,
    const FUDAppInfo &callerInfo, const FUDAppInfo &targetAppInfo, int32_t hideSensitiveType,
    const std::vector<int32_t> &permissionTypes)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGI(AAFwkTag::URIPERMMGR, "caller:%{public}u, target:%{public}u", callerInfo.tokenId, targetAppInfo.tokenId);
    BatchStringUri batchUris;
    int32_t validUriCount = 0;
    int32_t grantRet = INNER_ERR;
    // Compatible grant write flag can read file.
    auto rwMode = (flag | FLAG_READ_URI) & (FLAG_READ_WRITE_URI | FLAG_PERSIST_URI);
    for (size_t i = 0; i < uriVec.size(); i++) {
        auto uriInner = uriVec[i];
        if (!FUDUtils::CheckUriTypeIsValid(uriInner)) {
            continue;
        }
        validUriCount++;
        // content and distributed docs uri
        if (uriInner.GetScheme() == FUDConstants::CONTENT_SCHEME) {
            batchUris.contentUris.emplace_back(uriInner.ToString());
            continue;
        }
        if (FUDUtils::IsDocsCloudUri(uriInner)) {
            batchUris.uriStrVec.emplace_back(uriInner.ToString());
            continue;
        }
        if (uriInner.GetAuthority() == targetAppInfo.alterBundleName) {
            grantRet = ERR_OK;
            continue;
        }
        // media
        if (uriInner.GetAuthority() == FUDConstants::MEDIA_AUTHORITY) {
            batchUris.mediaUriVec.emplace_back(uriInner.ToString());
            continue;
        }
        // docs and bundle
        auto policyInfo = FilePermissionManager::GetPathPolicyInfoFromUri(uriInner, rwMode);
        // add permission type
        policyInfo.type = static_cast<PolicyType>(permissionTypes[i]);
        batchUris.policyVec.emplace_back(policyInfo);
    }
    TAG_LOGI(AAFwkTag::URIPERMMGR, "valid uris: %{public}d", validUriCount);
    if (validUriCount == 0) {
        return ERR_CODE_INVALID_URI_TYPE;
    }
    if (GrantUriPermissionPrivilegedImpl(batchUris, flag, callerInfo, targetAppInfo,
        hideSensitiveType) == ERR_OK) {
        return ERR_OK;
    }
    return grantRet;
}

int32_t UriPermissionManagerStubImpl::GrantUriPermissionPrivilegedImpl(BatchStringUri &batchUris, uint32_t flag,
    const FUDAppInfo &callerInfo, const FUDAppInfo &targetAppInfo, int32_t hideSensitiveType)
{
    int32_t result = INNER_ERR;
    if (GrantBatchUriPermissionImpl(batchUris.uriStrVec, flag, callerInfo.tokenId, targetAppInfo.tokenId) == ERR_OK) {
        result = ERR_OK;
    }
    if (GrantBatchMediaUriPermissionImpl(batchUris.mediaUriVec, flag, callerInfo.tokenId, targetAppInfo.tokenId,
        hideSensitiveType) == ERR_OK) {
        result = ERR_OK;
    }
    if (GrantBatchUriPermissionImplByPolicy(batchUris.policyVec, flag, callerInfo, targetAppInfo) == ERR_OK) {
        result = ERR_OK;
    }
    if (GrantBatchContentUriPermissionImpl(batchUris.contentUris, flag, targetAppInfo.tokenId,
        targetAppInfo.bundleName) == ERR_OK) {
        result = ERR_OK;
    }
    return result;
}

ErrCode UriPermissionManagerStubImpl::GrantUriPermissionByKeyAsCaller(const std::string &key, uint32_t flag,
    uint32_t callerTokenId, uint32_t targetTokenId, int32_t &funcResult)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
#ifdef ABILITY_RUNTIME_UDMF_ENABLE
    auto ret = CheckGrantUriPermissionByKeyAsCaller();
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "check fail:%{public}d", ret);
        return WrapErrorCode(ret, funcResult);
    }
    ret = GrantUriPermissionByKeyInner(key, flag, callerTokenId, targetTokenId);
    return WrapErrorCode(ret, funcResult);
#else
    TAG_LOGE(AAFwkTag::URIPERMMGR, "do not support udmf");
    return WrapErrorCode(ERR_CAPABILITY_NOT_SUPPORT, funcResult);
#endif // ABILITY_RUNTIME_UDMF_ENABLE
}

ErrCode UriPermissionManagerStubImpl::GrantUriPermissionByKey(const std::string &key, uint32_t flag,
    uint32_t targetTokenId, int32_t &funcResult)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
#ifdef ABILITY_RUNTIME_UDMF_ENABLE
    auto ret = CheckGrantUriPermissionByKey();
    if (ret != ERR_OK) {
        return WrapErrorCode(ret, funcResult);
    }
    uint32_t callerTokenId = IPCSkeleton::GetCallingTokenID();
    ret = GrantUriPermissionByKeyInner(key, flag, callerTokenId, targetTokenId);
    if (ret == ERR_UPMS_INVALID_CALLER_TOKENID) {
        return WrapErrorCode(INNER_ERR, funcResult);
    }
    return WrapErrorCode(ret, funcResult);
#else
    TAG_LOGE(AAFwkTag::URIPERMMGR, "do not support udmf");
    return WrapErrorCode(ERR_CAPABILITY_NOT_SUPPORT, funcResult);
#endif // ABILITY_RUNTIME_UDMF_ENABLE
}

#ifdef ABILITY_RUNTIME_UDMF_ENABLE
int32_t UriPermissionManagerStubImpl::GrantUriPermissionByKeyInner(const std::string &key, uint32_t flag,
    uint32_t callerTokenId, uint32_t targetTokenId)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "key:%{public}s, flag:%{public}u, caller:%{public}u, target:%{public}u",
        key.c_str(), flag, callerTokenId, targetTokenId);
    FUDAppInfo callerAppInfo = { .tokenId = callerTokenId };
    FUDAppInfo targetAppInfo = { .tokenId = targetTokenId };
    std::vector<std::string> uris;
    auto ret = CheckGrantUriPermissionByKeyParams(key, flag, callerAppInfo, targetAppInfo, uris);
    if (ret != ERR_OK) {
        return ret;
    }
    // check uri permission
    BatchUri batchUri;
    auto rwMode = (flag | FLAG_READ_URI) & (FLAG_READ_WRITE_URI | FLAG_PERSIST_URI);
    bool haveSandboxAccessPermission = PermissionVerification::GetInstance()->VerifyPermissionByTokenId(callerTokenId,
        PermissionConstants::PERMISSION_SANDBOX_ACCESS_MANAGER);
    batchUri.Init(uris, rwMode, callerAppInfo.alterBundleName, targetAppInfo.alterBundleName,
        haveSandboxAccessPermission);
    if (!batchUri.IsAllUriValid()) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Not all uri is valid");
        return ERR_UPMS_GET_FILE_URIS_BY_KEY_FAILED;
    }
    CheckUriPermission(batchUri, flag, callerTokenId, callerAppInfo.alterBundleName, targetTokenId);
    if (!batchUri.IsAllUriPermissioned()) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Not all uri is permissioned");
        return ERR_UPMS_NO_PERMISSION_GRANT_URI;
    }
    // grant uri permission
    std::vector<PolicyInfo> policyVec;
    batchUri.GetUriToGrantByPolicy(policyVec);
    if (!policyVec.empty() &&
        GrantBatchUriPermissionImplByPolicyWithoutCache(policyVec, flag, callerAppInfo, targetAppInfo) != ERR_OK) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "grant batch uri permission failed");
        return ERR_UPMS_GRANT_URI_PERMISSION_FAILED;
    }
    std::vector<std::string> mediaUriVec;
    if (batchUri.GetMediaUriToGrant(mediaUriVec) > 0 &&
        GrantBatchMediaUriPermissionImpl(mediaUriVec, flag, callerTokenId, targetTokenId, 0) != ERR_OK) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "grant media uri permission failed");
        return ERR_UPMS_GRANT_URI_PERMISSION_FAILED;
    }
    return ERR_OK;
}

int32_t UriPermissionManagerStubImpl::CheckGrantUriPermissionByKeyAsCaller()
{
    if (!AppUtils::GetInstance().IsSupportGrantUriPermission()) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "not support grant uri permission");
        return ERR_CAPABILITY_NOT_SUPPORT;
    }
    if (!FUDUtils::IsSystemAppCall()) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "caller not system");
        return ERR_NOT_SYSTEM_APP;
    }
    uint32_t callingTokenId = IPCSkeleton::GetCallingTokenID();
    auto permissionName = PermissionConstants::PERMISSION_GRANT_URI_PERMISSION_AS_CALLER;
    if (!PermissionVerification::GetInstance()->VerifyPermissionByTokenId(callingTokenId, permissionName)) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "No permission to call");
        return CHECK_PERMISSION_FAILED;
    }
    auto ret = CheckCalledBySandBox();
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "sandbox not support grant uripermission");
        return ret;
    }
    return ERR_OK;
}

int32_t UriPermissionManagerStubImpl::CheckGrantUriPermissionByKey()
{
    if (!AppUtils::GetInstance().IsSupportGrantUriPermission()) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "not support grant uri permission");
        return ERR_CAPABILITY_NOT_SUPPORT;
    }
    if (!FUDUtils::IsSystemAppCall()) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "caller not system");
        return ERR_NOT_SYSTEM_APP;
    }
    auto ret = CheckCalledBySandBox();
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "sandbox not support grant uripermission");
        return ret;
    }
    return ERR_OK;
}

int32_t UriPermissionManagerStubImpl::CheckGrantUriPermissionByKeyParams(const std::string &key, uint32_t flag,
    FUDAppInfo &callerAppInfo, FUDAppInfo &targetAppInfo, std::vector<std::string> &uris)
{
    if ((flag & FLAG_READ_WRITE_URI) == 0) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Flag invalid");
        return ERR_CODE_INVALID_URI_FLAG;
    }
    if (callerAppInfo.tokenId == targetAppInfo.tokenId) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "caller equal to target");
        return ERR_UPMS_INVALID_TARGET_TOKENID;
    }
    if (!FUDUtils::GenerateFUDAppInfo(callerAppInfo)) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "callerTokenId invalid");
        return ERR_UPMS_INVALID_CALLER_TOKENID;
    }
    if (!FUDUtils::GenerateFUDAppInfo(targetAppInfo)) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "targetTokenId invalid");
        return ERR_UPMS_INVALID_TARGET_TOKENID;
    }
    if (callerAppInfo.userId != targetAppInfo.userId) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "not support cross user");
        return ERR_UPMS_INVALID_TARGET_TOKENID;
    }
    auto ret = UDMFUtils::ProcessUdmfKey(key, callerAppInfo.tokenId, targetAppInfo.tokenId, uris);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "ProcessUdmfKey failed:%{public}d", ret);
        return ERR_UPMS_GET_FILE_URIS_BY_KEY_FAILED;
    }
    return ERR_OK;
}

int32_t UriPermissionManagerStubImpl::GrantBatchUriPermissionImplByPolicyWithoutCache(
    const std::vector<PolicyInfo> &policyInfos, uint32_t flag,
    const FUDAppInfo &callerInfo, const FUDAppInfo &targetInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    // if all policy grant success, return OK, else failed
#ifdef ABILITY_RUNTIME_FEATURE_SANDBOXMANAGER
    std::vector<uint32_t> result;
    auto ret = SandboxManagerSetPolicy(policyInfos, flag, callerInfo, targetInfo, result);
    if (ret != ERR_OK) {
        return ret;
    }
    std::lock_guard<std::mutex> lock(ptMapMutex_);
    permissionTokenMap_.insert(targetInfo.tokenId);
    for (size_t i = 0; i < result.size(); i++) {
        if (result[i] != ERR_OK) {
            TAG_LOGW(AAFwkTag::URIPERMMGR, "Failed to set policy, ret:%{public}d", result[i]);
            return result[i];
        }
    }
    return ERR_OK;
#else
    TAG_LOGE(AAFwkTag::URIPERMMGR, "sandbox manager not support");
    return ERR_CAPABILITY_NOT_SUPPORT;
#endif // ABILITY_RUNTIME_FEATURE_SANDBOXMANAGER
}
#endif // ABILITY_RUNTIME_UDMF_ENABLE

ErrCode UriPermissionManagerStubImpl::CheckUriAuthorizationWithType(const std::vector<std::string> &uriVec,
    uint32_t flag, uint32_t tokenId, std::vector<CheckResult> &funcResult)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    funcResult = std::vector<CheckResult>(uriVec.size(), CheckResult());
    if (!FUDUtils::IsFoundationCall()) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Not foundation call");
        return CHECK_PERMISSION_FAILED;
    }
    if (uriVec.empty() || uriVec.size() > MAX_URI_COUNT) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Invalid uriVec: %{public}zu", uriVec.size());
        return ERR_URI_LIST_OUT_OF_RANGE;
    }
    if ((flag & FLAG_READ_WRITE_URI) == 0) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Invalid Flag:%{public}u", flag);
        return ERR_CODE_INVALID_URI_FLAG;
    }
    if (tokenId == 0) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Invalid tokenId");
        return ERR_UPMS_INVALID_CALLER_TOKENID;
    }
    std::string tokenAlterableBundleName;
    FUDUtils::GetAlterableBundleNameByTokenId(tokenId, tokenAlterableBundleName);
    TAG_LOGI(AAFwkTag::URIPERMMGR, "tokenId:%{public}u,tokenName:%{public}s,flag:%{public}u,uris:%{public}zu",
        tokenId, tokenAlterableBundleName.c_str(), flag, uriVec.size());
    // split uri by uri authority
    BatchUri batchUri;
    bool haveSandboxAccessPermission = PermissionVerification::GetInstance()->VerifyPermissionByTokenId(tokenId,
        PermissionConstants::PERMISSION_SANDBOX_ACCESS_MANAGER);
    if (batchUri.Init(uriVec, 0, tokenAlterableBundleName, "", haveSandboxAccessPermission) == 0) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "All uri is invalid.");
        return ERR_OK;
    }
    CheckUriPermission(batchUri, flag, tokenId, tokenAlterableBundleName);
    funcResult = batchUri.checkResult;
    TAG_LOGI(AAFwkTag::URIPERMMGR, "CheckUriAuthorizationWithType finished");
    return ERR_OK;
}

ErrCode UriPermissionManagerStubImpl::CheckUriAuthorization(const std::vector<std::string>& uriStrVec,
    uint32_t flag, uint32_t tokenId, std::vector<bool>& funcResult)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    funcResult = std::vector<bool>(uriStrVec.size(), false);
    if (uriStrVec.size() == 0 || uriStrVec.size() > MAX_URI_COUNT) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "uriVec empty or exceed maxSize %{public}d uriStrVec size: %{public}zu",
            MAX_URI_COUNT, uriStrVec.size());
        return ERR_URI_LIST_OUT_OF_RANGE;
    }
    if (!FUDUtils::IsPrivilegedSACall()) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "no permission call");
        return ERR_OK;
    }
    if ((flag & FLAG_READ_WRITE_URI) == 0) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Flag is invalid:%{public}u", flag);
        return ERR_OK;
    }
    std::string tokenAlterableBundleName;
    FUDUtils::GetAlterableBundleNameByTokenId(tokenId, tokenAlterableBundleName);
    TAG_LOGI(AAFwkTag::URIPERMMGR, "tokenId:%{private}u,tokenName:%{public}s,flag:%{public}u, uris:%{public}zu",
        tokenId, tokenAlterableBundleName.c_str(), flag, uriStrVec.size());
    // split uri by uri authority
    BatchUri batchUri;
    bool haveSandboxAccessPermission = PermissionVerification::GetInstance()->VerifyPermissionByTokenId(tokenId,
        PermissionConstants::PERMISSION_SANDBOX_ACCESS_MANAGER);
    if (batchUri.Init(uriStrVec, 0, tokenAlterableBundleName, "", haveSandboxAccessPermission) == 0) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "All uri is invalid.");
        return ERR_OK;
    }
    CheckUriPermission(batchUri, flag, tokenId, tokenAlterableBundleName);
    batchUri.SetCheckUriAuthorizationResult(funcResult);
    TAG_LOGI(AAFwkTag::URIPERMMGR, "CheckUriAuthorization finished");
    return ERR_OK;
}

ErrCode UriPermissionManagerStubImpl::CheckUriAuthorization(const UriPermissionRawData& rawData, uint32_t flag,
    uint32_t tokenId, UriPermissionRawData& funcResult)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::vector<std::string> uriStringVec;
    std::vector<char> resultCharVec;
    auto res = RawDataToStringVec(rawData, uriStringVec);
    if (res != ERR_OK) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "raw data to vec failed");
        std::vector<bool> defaultFalseResult(1, false);
        BoolVecToRawData(defaultFalseResult, funcResult, resultCharVec);
        return res;
    }
    std::vector<bool> resultBoolVec(uriStringVec.size(), false);
    auto errCode = CheckUriAuthorization(uriStringVec, flag, tokenId, resultBoolVec);
    if (errCode != ERR_OK) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "CheckUriAuthorization failed, errCode:%{public}d", errCode);
        return errCode;
    }
    BoolVecToRawData(resultBoolVec, funcResult, resultCharVec);
    if (funcResult.size > MAX_IPC_RAW_DATA_SIZE) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "funcResultSize is too large");
        std::vector<bool> defaultFalseResult(1, false);
        BoolVecToRawData(defaultFalseResult, funcResult, resultCharVec);
        return ERR_DEAD_OBJECT;
    }
    return ERR_OK;
}

int32_t UriPermissionManagerStubImpl::CheckUriPermission(BatchUri &batchUri, uint32_t flag,
    uint32_t callerTokenId, const std::string &callerAlterableBundleName, uint32_t targetTokenId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGI(AAFwkTag::URIPERMMGR, "callerTokenId:%{public}u,flag:%{public}u", callerTokenId, flag);
    // only need reserve read and write file flag
    flag &= FLAG_READ_WRITE_URI;
#ifdef ABILITY_RUNTIME_MEDIA_LIBRARY_ENABLE
    if (!batchUri.mediaUris.empty()) {
        auto mediaUriResult = MediaPermissionManager::GetInstance().CheckUriPermission(batchUri.mediaUris,
            callerTokenId, flag);
        batchUri.SetMediaUriCheckResult(mediaUriResult);
    }
#endif // ABILITY_RUNTIME_MEDIA_LIBRARY_ENABLE
    if (!batchUri.otherUris.empty()) {
        auto otherUriResult = FilePermissionManager::CheckUriPersistentPermission(batchUri.otherUris, callerTokenId,
            flag, callerAlterableBundleName, batchUri.otherPolicyInfos);
        TAG_LOGI(AAFwkTag::URIPERMMGR, "otherPolicyInfos:%{public}zu", batchUri.otherPolicyInfos.size());
        batchUri.SetOtherUriCheckResult(otherUriResult);
    }
    auto permissionedUriCount = batchUri.GetPermissionedUriCount();
    if (permissionedUriCount != batchUri.validUriCount) {
        TAG_LOGI(AAFwkTag::URIPERMMGR, "check proxy permission, permissioned uris:%{public}d", permissionedUriCount);
        CheckProxyUriPermission(batchUri, callerTokenId, flag);
    }
    permissionedUriCount = batchUri.GetPermissionedUriCount();
    if (targetTokenId != 0 && permissionedUriCount != batchUri.totalUriCount) {
        TAG_LOGW(AAFwkTag::URIPERMMGR, "total permissioned uris:%{public}d", permissionedUriCount);
        FUDUtils::SendShareUnPrivilegeUriEvent(callerTokenId, targetTokenId);
    }
    if (permissionedUriCount == 0) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Check uri permission failed.");
        return CHECK_PERMISSION_FAILED;
    }
    return ERR_OK;
}

int32_t UriPermissionManagerStubImpl::CheckProxyUriPermission(BatchUri &batchUri, uint32_t callerTokenId,
    uint32_t flag)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TokenIdPermission tokenIdPermission(callerTokenId);
    if (!tokenIdPermission.VerifyProxyAuthorizationUriPermission()) {
        TAG_LOGW(AAFwkTag::URIPERMMGR, "No proxy authorization uri permission.");
        return CHECK_PERMISSION_FAILED;
    }
    std::vector<PolicyInfo> proxyUrisByPolicy;
    batchUri.GetNeedCheckProxyPermissionURI(proxyUrisByPolicy);
    if (!proxyUrisByPolicy.empty()) {
        auto proxyResultByPolicy = VerifyUriPermissionByPolicy(proxyUrisByPolicy, flag, callerTokenId);
        batchUri.SetCheckProxyByPolicyResult(proxyResultByPolicy);
    }
    return ERR_OK;
}

void UriPermissionManagerStubImpl::RevokeMapUriPermission(uint32_t tokenId)
{
    // revoke uri permission record cache of tokenId when application exit
    TAG_LOGD(AAFwkTag::URIPERMMGR, "RevokeMapUriPermission call");
    std::lock_guard<std::mutex> guard(mutex_);
    std::vector<std::string> uriList;
    int32_t deleteCount = 0;
    for (auto iter = uriMap_.begin(); iter != uriMap_.end();) {
        auto& list = iter->second;
        bool findUriRecord = false;
        for (auto it = list.begin(); it != list.end();) {
            if (it->targetTokenId == tokenId) {
                deleteCount++;
                it = list.erase(it);
                findUriRecord = true;
                continue;
            }
            it++;
        }
        if (findUriRecord) {
            uriList.emplace_back(iter->first);
        }
        if (list.empty()) {
            iter = uriMap_.erase(iter);
            continue;
        }
        ++iter;
    }
    if (!uriList.empty()) {
        DeleteShareFile(tokenId, uriList);
    }
    if (deleteCount > 0) {
        TAG_LOGI(AAFwkTag::URIPERMMGR, "revoke map: %{public}d", deleteCount);
    }
}

void UriPermissionManagerStubImpl::RevokePolicyUriPermission(uint32_t tokenId)
{
    // revoke uri permission record cache of tokenId when application exit
    TAG_LOGD(AAFwkTag::URIPERMMGR, "call");
    std::lock_guard<std::mutex> guard(policyMapMutex_);
    int32_t deleteCount = 0;
    for (auto iter = policyMap_.begin(); iter != policyMap_.end();) {
        auto &grantPolicyInfoList = iter->second;
        for (auto it = grantPolicyInfoList.begin(); it != grantPolicyInfoList.end();) {
            if (it->targetTokenId == tokenId) {
                deleteCount++;
                it = grantPolicyInfoList.erase(it);
                continue;
            }
            it++;
        }
        if (grantPolicyInfoList.empty()) {
            iter = policyMap_.erase(iter);
            continue;
        }
        iter++;
    }
    if (deleteCount > 0) {
        TAG_LOGI(AAFwkTag::URIPERMMGR, "revoke policy: %{public}d", deleteCount);
    }
}

ErrCode UriPermissionManagerStubImpl::RevokeAllUriPermissions(uint32_t tokenId, int32_t& funcResult)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "RevokeAllUriPermissions, tokenId:%{public}u", tokenId);
    if (!FUDUtils::IsFoundationCall()) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "No permission to revoke all uri permission");
        funcResult = CHECK_PERMISSION_FAILED;
        return ERR_OK;
    }
    RevokeAllPolicyUriPermissions(tokenId);
    RevokeAllMapUriPermissions(tokenId);
    RevokeContentUriPermission(tokenId);
    funcResult = ERR_OK;
    return ERR_OK;
}

int32_t UriPermissionManagerStubImpl::RevokeAllMapUriPermissions(uint32_t tokenId)
{
    std::string callerAuthority = "";
    FUDUtils::GetAlterableBundleNameByTokenId(tokenId, callerAuthority);
    std::map<uint32_t, std::vector<std::string>> uriLists;
    {
        std::lock_guard<std::mutex> guard(mutex_);
        for (auto iter = uriMap_.begin(); iter != uriMap_.end();) {
            auto uriAuthority = Uri(iter->first).GetAuthority();
            // uri belong to target tokenId.
            if (callerAuthority == uriAuthority) {
                for (const auto &record : iter->second) {
                    uriLists[record.targetTokenId].emplace_back(iter->first);
                }
                iter = uriMap_.erase(iter);
                continue;
            }
            auto& list = iter->second;
            for (auto it = list.begin(); it != list.end();) {
                if (it->targetTokenId == tokenId || it->fromTokenId == tokenId) {
                    TAG_LOGI(AAFwkTag::URIPERMMGR, "Erase an uri permission record");
                    uriLists[it->targetTokenId].emplace_back(iter->first);
                    it = list.erase(it);
                    continue;
                }
                it++;
            }
            if (list.empty()) {
                iter = uriMap_.erase(iter);
                continue;
            }
            iter++;
        }
    }

    for (auto iter = uriLists.begin(); iter != uriLists.end(); iter++) {
        if (DeleteShareFile(iter->first, iter->second) != ERR_OK) {
            return INNER_ERR;
        }
    }
    return ERR_OK;
}

int32_t UriPermissionManagerStubImpl::RevokeAllPolicyUriPermissions(uint32_t tokenId)
{
    // only clear policy cache
    std::lock_guard<std::mutex> guard(policyMapMutex_);
    int32_t deleteCount = 0;
    for (auto iter = policyMap_.begin(); iter != policyMap_.end();) {
        auto &grantPolicyInfoList = iter->second;
        for (auto it = grantPolicyInfoList.begin(); it != grantPolicyInfoList.end();) {
            if (it->callerTokenId == tokenId || it->targetTokenId == tokenId) {
                deleteCount++;
                it = grantPolicyInfoList.erase(it);
                continue;
            }
            it++;
        }
        if (grantPolicyInfoList.empty()) {
            iter = policyMap_.erase(iter);
            continue;
        }
        iter++;
    }
    TAG_LOGI(AAFwkTag::URIPERMMGR, "revoke all: %{public}d", deleteCount);
    return ERR_OK;
}

ErrCode UriPermissionManagerStubImpl::RevokeUriPermissionManually(const Uri& uri, const std::string& bundleName,
    int32_t appIndex, int32_t& funcResult)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGI(AAFwkTag::URIPERMMGR, "uri:%{private}s, bundleName:%{public}s, appIndex:%{public}d",
        uri.ToString().c_str(), bundleName.c_str(), appIndex);
    if (!FUDUtils::IsSystemAppCall() && !FUDUtils::IsBrokerCaller()) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "not SystemApp call");
        funcResult = CHECK_PERMISSION_FAILED;
        return ERR_OK;
    }
    auto uriInner = uri;
    if (!FUDUtils::CheckUriTypeIsValid(uriInner)) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "CheckUriType failed, uri:%{private}s", uri.ToString().c_str());
        funcResult = ERR_CODE_INVALID_URI_TYPE;
        return ERR_OK;
    }
    uint32_t targetTokenId = 0;
    auto ret = FUDUtils::GetTokenIdByBundleName(bundleName, appIndex, targetTokenId);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "get tokenId by bundleName fail");
        funcResult = ret;
        return ERR_OK;
    }
    funcResult = RevokeUriPermissionManuallyInner(uriInner, targetTokenId);
    return ERR_OK;
}

int32_t UriPermissionManagerStubImpl::RevokeUriPermissionManuallyInner(Uri &uri, uint32_t targetTokenId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto callerTokenId = IPCSkeleton::GetCallingTokenID();
    TAG_LOGI(AAFwkTag::URIPERMMGR, "callerTokenId: %{public}u, targetTokenId:%{public}u",
        callerTokenId, targetTokenId);

    if (FUDUtils::IsDocsCloudUri(uri)) {
        return RevokeMapUriPermissionManually(callerTokenId, targetTokenId, uri);
    }
    if (uri.GetAuthority() == FUDConstants::MEDIA_AUTHORITY) {
        return RevokeMediaUriPermissionManually(callerTokenId, targetTokenId, uri);
    }
    // docs and bundle uri
    return RevokePolicyUriPermissionManually(callerTokenId, targetTokenId, uri);
}

int32_t UriPermissionManagerStubImpl::RevokeMapUriPermissionManually(uint32_t callerTokenId,
    uint32_t targetTokenId, Uri &uri)
{
    auto uriStr = uri.ToString();
    auto uriAuthority = uri.GetAuthority();
    // uri belong to caller or caller is target.
    std::string callerAuthority = "";
    FUDUtils::GetAlterableBundleNameByTokenId(callerTokenId, callerAuthority);
    bool isRevokeSelfUri = (callerTokenId == targetTokenId || callerAuthority == uriAuthority);
    std::vector<std::string> uriList;
    {
        std::lock_guard<std::mutex> guard(mutex_);
        auto search = uriMap_.find(uriStr);
        if (search == uriMap_.end()) {
            TAG_LOGI(AAFwkTag::URIPERMMGR, "URI not exist on uri map");
            return ERR_OK;
        }
        auto& list = search->second;
        for (auto it = list.begin(); it != list.end(); it++) {
            if (it->targetTokenId == targetTokenId && (callerTokenId == it->fromTokenId || isRevokeSelfUri)) {
                uriList.emplace_back(search->first);
                TAG_LOGD(AAFwkTag::URIPERMMGR, "revoke uri permission record");
                list.erase(it);
                break;
            }
        }
        if (list.empty()) {
            uriMap_.erase(search);
        }
    }
    return DeleteShareFile(targetTokenId, uriList);
}

int32_t UriPermissionManagerStubImpl::DeleteShareFile(uint32_t targetTokenId, const std::vector<std::string> &uriVec)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    ConnectManager(storageManager_, STORAGE_MANAGER_MANAGER_ID);
    if (storageManager_ == nullptr) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "null StorageManager");
        return INNER_ERR;
    }
    StorageFileRawData uriRawData;
    StringVecToRawData(uriVec, uriRawData);
    auto ret = storageManager_->DeleteShareFile(targetTokenId, uriRawData);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "DeleteShareFile failed:%{public}d", ret);
    }
    return ret;
}

int32_t UriPermissionManagerStubImpl::RevokePolicyUriPermissionManually(uint32_t callerTokenId, uint32_t targetTokenId,
    Uri &uri)
{
#ifdef ABILITY_RUNTIME_FEATURE_SANDBOXMANAGER
    auto policyInfo = FilePermissionManager::GetPathPolicyInfoFromUri(uri, FLAG_READ_WRITE_URI);
    TAG_LOGD(AAFwkTag::URIPERMMGR, "path is %{private}s.", policyInfo.path.c_str());
    std::lock_guard<std::mutex> guard(policyMapMutex_);
    auto searchPathIter = policyMap_.find(policyInfo.path);
    if (searchPathIter == policyMap_.end()) {
        TAG_LOGD(AAFwkTag::URIPERMMGR, "Do not found policy info record by path.");
        return ERR_OK;
    }
    bool ispolicyVectorByCaller = false;
    auto &grantPolicyInfoList = searchPathIter->second;
    for (auto it = grantPolicyInfoList.begin(); it != grantPolicyInfoList.end();) {
        if ((targetTokenId == it->targetTokenId) &&
            (callerTokenId == it->targetTokenId || callerTokenId == it->callerTokenId)) {
            it = grantPolicyInfoList.erase(it);
            ispolicyVectorByCaller = true;
            continue;
        }
        it++;
    }
    if (grantPolicyInfoList.empty()) {
        policyMap_.erase(searchPathIter);
    }
    if (ispolicyVectorByCaller) {
        TAG_LOGI(AAFwkTag::URIPERMMGR, "Start to unSetPolicy, path is %{private}s.", policyInfo.path.c_str());
        auto ret = IN_PROCESS_CALL(SandboxManagerKit::UnSetPolicy(targetTokenId, policyInfo));
        if (ret != ERR_OK) {
            TAG_LOGI(AAFwkTag::URIPERMMGR, "UnSetPolicy failed, ret is %{public}d", ret);
        }
        return ret;
    }
    TAG_LOGI(AAFwkTag::URIPERMMGR, "No grant uri permission record.");
#endif
    return ERR_OK;
}

int32_t UriPermissionManagerStubImpl::RevokeMediaUriPermissionManually(uint32_t callerTokenId, uint32_t targetTokenId,
    Uri &uri)
{
#ifdef ABILITY_RUNTIME_MEDIA_LIBRARY_ENABLE
    std::string uriStr = uri.ToString();
    return MediaPermissionManager::GetInstance().RevokeUriPermission(callerTokenId, targetTokenId, uriStr);
#else
    return CAPABILITY_NOT_SUPPORT;
#endif // ABILITY_RUNTIME_MEDIA_LIBRARY_ENABLE
}

template<typename T>
void UriPermissionManagerStubImpl::ConnectManager(sptr<T> &mgr, int32_t serviceId)
{
    TAG_LOGD(AAFwkTag::URIPERMMGR, "Call");
    std::lock_guard<std::mutex> lock(mgrMutex_);
    if (mgr == nullptr) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "mgr null");
        auto systemAbilityMgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        if (systemAbilityMgr == nullptr) {
            TAG_LOGE(AAFwkTag::URIPERMMGR, "null systemAbilityMgr");
            return;
        }

        auto remoteObj = systemAbilityMgr->GetSystemAbility(serviceId);
        if (remoteObj == nullptr) {
            TAG_LOGE(AAFwkTag::URIPERMMGR, "null Obj");
            return;
        }
        TAG_LOGE(AAFwkTag::URIPERMMGR, "to cast");
        mgr = iface_cast<T>(remoteObj);
        if (mgr == nullptr || mgr->AsObject() == nullptr) {
            TAG_LOGE(AAFwkTag::URIPERMMGR, "null mgr");
            return;
        }
        wptr<T> manager = mgr;
        auto self = weak_from_this();
        auto onClearProxyCallback = [manager, self](const auto& remote) {
            auto mgrSptr = manager.promote();
            auto impl = self.lock();
            if (impl && mgrSptr && mgrSptr->AsObject() == remote.promote()) {
                std::lock_guard<std::mutex> lock(impl->mgrMutex_);
                mgrSptr.clear();
            }
        };
        sptr<ProxyDeathRecipient> recipient(new ProxyDeathRecipient(std::move(onClearProxyCallback)));
        if (!mgr->AsObject()->AddDeathRecipient(recipient)) {
            TAG_LOGE(AAFwkTag::URIPERMMGR, "AddDeathRecipient failed");
        }
    }
}

void UriPermissionManagerStubImpl::ProxyDeathRecipient::OnRemoteDied([[maybe_unused]]
    const wptr<IRemoteObject>& remote)
{
    if (proxy_) {
        TAG_LOGD(AAFwkTag::URIPERMMGR, "mgr stub died");
        proxy_(remote);
    }
}

int32_t UriPermissionManagerStubImpl::CheckCalledBySandBox()
{
    // reject sandbox to grant uri permission
    auto callerTokenId = IPCSkeleton::GetCallingTokenID();
    if (FUDUtils::IsSandboxApp(callerTokenId)) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "sandbox app not grant URI permission");
        return ERR_CODE_GRANT_URI_PERMISSION;
    }
    return ERR_OK;
}

ErrCode UriPermissionManagerStubImpl::ClearPermissionTokenByMap(uint32_t tokenId, int32_t& funcResult)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::URIPERMMGR, "call");
    if (!FUDUtils::IsFoundationCall()) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "not foundation call");
        return WrapErrorCode(ERR_PERMISSION_DENIED, funcResult);
    }
    RevokeContentUriPermission(tokenId);
    std::lock_guard<std::mutex> lock(ptMapMutex_);
    if (permissionTokenMap_.find(tokenId) == permissionTokenMap_.end()) {
        TAG_LOGD(AAFwkTag::URIPERMMGR, "permissionTokenMap_ empty");
        return WrapErrorCode(ERR_OK, funcResult);
    }
    RevokeMapUriPermission(tokenId);
#ifdef ABILITY_RUNTIME_FEATURE_SANDBOXMANAGER
    RevokePolicyUriPermission(tokenId);
    uint64_t timeNow = static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::high_resolution_clock::now().time_since_epoch()).count());
    TAG_LOGI(AAFwkTag::URIPERMMGR, "clear %{public}d permission", tokenId);
    auto ret = SandboxManagerKit::UnSetAllPolicyByToken(tokenId, timeNow);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "ClearPermission failed, ret is %{public}d", ret);
        return WrapErrorCode(ret, funcResult);
    }
#endif // ABILITY_RUNTIME_FEATURE_SANDBOXMANAGER
    permissionTokenMap_.erase(tokenId);
    return WrapErrorCode(ERR_OK, funcResult);
}

void UriPermissionManagerStubImpl::BoolVecToCharVec(const std::vector<bool>& boolVector, std::vector<char>& charVector)
{
    charVector.clear();
    if (boolVector.empty()) {
        return;
    }
    for (bool b : boolVector) {
        char value = b ? static_cast<char>(1) : static_cast<char>(0);
        charVector.push_back(value);
    }
}

void UriPermissionManagerStubImpl::BoolVecToRawData(const std::vector<bool> &boolVector, UriPermissionRawData &rawData,
    std::vector<char> &charVector)
{
    BoolVecToCharVec(boolVector, charVector);
    std::stringstream ss;
    uint32_t boolCount = boolVector.size();
    ss.write(reinterpret_cast<const char *>(&boolCount), sizeof(boolCount));
    for (uint32_t i = 0; i < boolCount; ++i) {
        ss.write(reinterpret_cast<const char *>(&charVector[i]), sizeof(charVector[i]));
    }
    std::string result = ss.str();
    rawData.ownedData = std::move(result);
    rawData.data = rawData.ownedData.data();
    rawData.size = rawData.ownedData.size();
}

ErrCode UriPermissionManagerStubImpl::RawDataToStringVec(const UriPermissionRawData &rawData,
    std::vector<std::string> &stringVec)
{
    if (rawData.data == nullptr) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "null data");
        return ERR_DEAD_OBJECT;
    }
    if (rawData.size == 0 || rawData.size > MAX_IPC_RAW_DATA_SIZE) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "size invalid: %{public}u", rawData.size);
        return ERR_DEAD_OBJECT;
    }
    std::stringstream ss;
    ss.write(reinterpret_cast<const char *>(rawData.data), rawData.size);
    uint32_t stringVecSize = 0;
    ss.read(reinterpret_cast<char *>(&stringVecSize), sizeof(stringVecSize));
    if (stringVecSize == 0 || stringVecSize > MAX_URI_COUNT) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "uriVec empty or exceed maxSize %{public}d, stringVecSize: %{public}d",
            MAX_URI_COUNT, stringVecSize);
        return ERR_URI_LIST_OUT_OF_RANGE;
    }
    uint32_t ssLength = static_cast<uint32_t>(ss.str().length());
    for (uint32_t i = 0; i < stringVecSize; ++i) {
        uint32_t strLen = 0;
        ss.read(reinterpret_cast<char *>(&strLen), sizeof(strLen));
        if (strLen > ssLength - static_cast<uint32_t>(ss.tellg())) {
            TAG_LOGE(AAFwkTag::URIPERMMGR, "string length:%{public}u is invalid", strLen);
            return ERR_DEAD_OBJECT;
        }
        std::string str;
        str.resize(strLen);
        ss.read(&str[0], strLen);
        stringVec.emplace_back(str);
    }
    return ERR_OK;
}

#ifdef ABILITY_RUNTIME_FEATURE_SANDBOXMANAGER
ErrCode UriPermissionManagerStubImpl::Active(const UriPermissionRawData& policyRawData, std::vector<uint32_t>& res,
    int32_t& funcResult)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto tokenId = IPCSkeleton::GetCallingTokenID();
    auto permissionName = PermissionConstants::PERMISSION_FILE_ACCESS_PERSIST;
    if (!PermissionVerification::GetInstance()->VerifyPermissionByTokenId(tokenId, permissionName)) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "No permission to call");
        funcResult = SANDBOX_MANAGER_PERMISSION_DENIED;
        return ERR_OK;
    }
    TAG_LOGI(AAFwkTag::URIPERMMGR, "active %{public}d permission", tokenId);
    std::vector<PolicyInfo> policy;
    auto result = RawDataToPolicyInfo(policyRawData, policy);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "RawDataToPolicyInfo failed");
        funcResult = result;
        return funcResult;
    }
    uint64_t timeNow = static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::high_resolution_clock::now().time_since_epoch()).count());
    auto ret = SandboxManagerKit::StartAccessingPolicy(policy, res, false, tokenId, timeNow);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "StartAccessingPolicy failed, ret is %{public}d", ret);
        funcResult = ret;
        return ERR_OK;
    }
    std::lock_guard<std::mutex> lock(ptMapMutex_);
    permissionTokenMap_.insert(tokenId);
    funcResult = ERR_OK;
    return ERR_OK;
}

ErrCode UriPermissionManagerStubImpl::RawDataToPolicyInfo(const UriPermissionRawData& policyRawData,
    std::vector<PolicyInfo>& policy)
{
    std::stringstream ss;
    ss.write(reinterpret_cast<const char *>(policyRawData.data), policyRawData.size);
    ss.seekg(0, std::ios::beg);
    uint32_t ssLength = static_cast<uint32_t>(ss.str().length());
    uint32_t policyInfoSize = 0;
    ss.read(reinterpret_cast<char *>(&policyInfoSize), sizeof(policyInfoSize));
    if (policyInfoSize == 0 || policyInfoSize > MAX_URI_COUNT) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "policy empty or exceed maxSize %{public}d, policyInfoSize: %{public}d",
            MAX_URI_COUNT, policyInfoSize);
        return ERR_URI_LIST_OUT_OF_RANGE;
    }
    for (uint32_t i = 0; i < policyInfoSize; ++i) {
        uint32_t pathLen = 0;
        ss.read(reinterpret_cast<char *>(&pathLen), sizeof(pathLen));
        if (pathLen > ssLength - static_cast<uint32_t>(ss.tellg())) {
            TAG_LOGE(AAFwkTag::URIPERMMGR, "path eln:%{public}u is invalid", pathLen);
            return INVALID_PARAMETERS_ERR;
        }
        PolicyInfo info;
        info.path.resize(pathLen);
        ss.read(info.path.data(), pathLen);
        ss.read(reinterpret_cast<char *>(&info.mode), sizeof(info.mode));
        policy.emplace_back(info);
    }
    return ERR_OK;
}
#endif // ABILITY_RUNTIME_FEATURE_SANDBOXMANAGER
}  // namespace AAFwk
}  // namespace OHOS