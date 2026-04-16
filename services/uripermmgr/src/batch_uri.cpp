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

#include "batch_uri.h"

#include "file_permission_manager.h"
#include "file_uri_distribution_utils.h"
#include "hilog_tag_wrapper.h"
#include "fud_constants.h"
namespace OHOS {
namespace AAFwk {

int32_t BatchUri::Init(const std::vector<std::string> &uriVec, uint32_t mode, const std::string &callerAlterBundleName,
                       const std::string &targetAlterBundleName, bool haveSandboxAccessPermission)
{
    if (uriVec.empty()) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "uriVec is empty.");
        return 0;
    }
    totalUriCount = static_cast<int32_t>(uriVec.size());
    validUriCount = 0;
    checkResult = std::vector<CheckResult>(totalUriCount, CheckResult());
    isDocsUriVec = std::vector<bool>(totalUriCount, false);
    isTargetBundleUri = std::vector<bool>(totalUriCount, false);
    for (size_t index = 0; index < uriVec.size(); index++) {
        Uri uriInner = Uri(uriVec[index]);
        auto &&scheme = uriInner.GetScheme();
        if (index == 0) {
            TAG_LOGI(AAFwkTag::URIPERMMGR, "uri type: %{public}s.", uriInner.GetAuthority().c_str());
        }
        if (scheme != FUDConstants::FILE_SCHEME && scheme != FUDConstants::ANCO_SCHEME) {
            TAG_LOGW(AAFwkTag::URIPERMMGR, "uri is invalid: %{private}s.", uriInner.ToString().c_str());
            continue;
        }
        validUriCount++;
        // content uri
        if (scheme == FUDConstants::ANCO_SCHEME) {
            contentUris.emplace_back(uriInner.ToString());
            contentIndexes.emplace_back(index);
            continue;
        }
        InitFileUriInfo(uriInner, index, mode, callerAlterBundleName, targetAlterBundleName,
                        haveSandboxAccessPermission);
    }
    TAG_LOGI(AAFwkTag::URIPERMMGR, "count of uri is %{public}d, count of valid uri is %{public}d.", totalUriCount,
             validUriCount);
    return validUriCount;
}

void BatchUri::InitFileUriInfo(Uri &uriInner, uint32_t index, const uint32_t mode,
                               const std::string &callerAlterBundleName, const std::string &targetAlterBundleName,
                               bool haveSandboxAccessPermission)
{
    auto &&authority = uriInner.GetAuthority();
    // media uri
    if (authority == FUDConstants::MEDIA_AUTHORITY) {
        mediaUris.emplace_back(uriInner.ToString());
        mediaIndexes.emplace_back(index);
        return;
    }
    // docs uri
    if (authority == FUDConstants::DOCS_AUTHORITY) {
        isDocsUriVec[index] = true;
        // need to check uri permission
        otherUris.emplace_back(uriInner);
        otherIndexes.emplace_back(index);
        return;
    }
    // bundle uri
    isTargetBundleUri[index] = (!targetAlterBundleName.empty() && authority == targetAlterBundleName);
    if (!authority.empty() && (haveSandboxAccessPermission || authority == callerAlterBundleName)) {
        checkResult[index].result = true;
        if (authority == callerAlterBundleName) {
            checkResult[index].permissionType = PolicyType::SELF_PATH;
        }
        if (isTargetBundleUri[index]) {
            TAG_LOGI(AAFwkTag::URIPERMMGR, "uri belong to targetBundle.");
            targetBundleUriCount++;
            return;
        }
        if (mode > 0) {
            // need set policy
            auto policyInfo = FilePermissionManager::GetPathPolicyInfoFromUri(uriInner, mode);
            policyInfo.type = static_cast<PolicyType>(checkResult[index].permissionType);
            selfBundlePolicyInfos.emplace_back(policyInfo);
        }
        return;
    }
    // bundle uri, need to check uri permission
    otherUris.emplace_back(uriInner);
    otherIndexes.emplace_back(index);
}

void BatchUri::SetMediaUriCheckResult(const std::vector<bool> &mediaUriResult)
{
    for (size_t i = 0; i < mediaUriResult.size(); i++) {
        auto index = mediaIndexes[i];
        checkResult[index].result = mediaUriResult[i];
    }
}

void BatchUri::SetOtherUriCheckResult(const std::vector<bool> &otherUriResult)
{
    for (size_t i = 0; i < otherUriResult.size(); i++) {
        auto index = otherIndexes[i];
        checkResult[index].result = otherUriResult[i];
        if (checkResult[index].result && isTargetBundleUri[index]) {
            targetBundleUriCount++;
        }
    }
}

int32_t BatchUri::GetMediaUriToGrant(std::vector<std::string> &uriVec)
{
    for (size_t i = 0; i < mediaIndexes.size(); i++) {
        if (checkResult[mediaIndexes[i]].result) {
            uriVec.emplace_back(mediaUris[i]);
        }
    }
    return uriVec.size();
}

void BatchUri::GetNeedCheckProxyPermissionURI(std::vector<PolicyInfo> &proxyUrisByPolicy)
{
    // docs uri and bundle uri
    for (size_t i = 0; i < otherIndexes.size(); i++) {
        auto index = otherIndexes[i];
        if (!checkResult[index].result) {
            proxyIndexesByPolicy.emplace_back(index);
            proxyUrisByPolicy.emplace_back(otherPolicyInfos[i]);
        }
    }
}

bool BatchUri::SetCheckProxyByPolicyResult(const std::vector<bool> &proxyResultByPolicy)
{
    if (proxyResultByPolicy.size() != proxyIndexesByPolicy.size()) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Invalid proxyResult:%{public}zu, %{public}zu", proxyResultByPolicy.size(),
                 proxyIndexesByPolicy.size());
        return false;
    }
    for (size_t i = 0; i < proxyResultByPolicy.size(); i++) {
        auto index = proxyIndexesByPolicy[i];
        checkResult[index].result = proxyResultByPolicy[i];
        if (checkResult[index].result) {
            checkResult[index].permissionType = PolicyType::AUTHORIZATION_PATH;
            if (isTargetBundleUri[index]) {
                targetBundleUriCount++;
            }
        }
    }
    proxyIndexesByPolicy.clear();
    return true;
}

int32_t BatchUri::GetUriToGrantByMap(std::vector<std::string> &uriVec)
{
    return uriVec.size();
}

void BatchUri::SelectPermissionedUri(std::vector<Uri> &uris, std::vector<int32_t> &indexs,
                                     std::vector<std::string> &uriVec)
{
    for (size_t i = 0; i < indexs.size(); i++) {
        if (checkResult[indexs[i]].result) {
            auto uriStr = uris[i].ToString();
            uriVec.emplace_back(uriStr);
        }
    }
}

int32_t BatchUri::GetUriToGrantByPolicy(std::vector<PolicyInfo> &docsPolicyInfoVec,
                                        std::vector<PolicyInfo> &bundlePolicyInfoVec)
{
    // bundleName + docs
    int32_t count = 0;
    for (auto &selfBundleUriPolicy : selfBundlePolicyInfos) {
        bundlePolicyInfoVec.emplace_back(selfBundleUriPolicy);
        count++;
    }
    for (size_t i = 0; i < otherPolicyInfos.size(); i++) {
        auto index = otherIndexes[i];
        if (!checkResult[index].result) {
            continue;
        }
        // the uri belong to target app.
        if (isTargetBundleUri[index]) {
            continue;
        }
        TAG_LOGD(AAFwkTag::URIPERMMGR, "Add policy: path is %{private}s, mode is %{public}u.",
                 otherPolicyInfos[i].path.c_str(), static_cast<uint32_t>(otherPolicyInfos[i].mode));
        otherPolicyInfos[i].type = static_cast<PolicyType>(checkResult[index].permissionType);
        if (isDocsUriVec[index]) {
            docsPolicyInfoVec.emplace_back(otherPolicyInfos[i]);
        } else {
            bundlePolicyInfoVec.emplace_back(otherPolicyInfos[i]);
        }
        count++;
    }
    return count;
}

bool BatchUri::GetUriToGrantByPolicy(std::vector<PolicyInfo> &policyVec)
{
    // self bundle uris
    for (const auto &selfBundleUriPolicy : selfBundlePolicyInfos) {
        policyVec.emplace_back(selfBundleUriPolicy);
    }
    for (size_t i = 0; i < otherPolicyInfos.size(); i++) {
        auto index = otherIndexes[i];
        if (!checkResult[index].result) {
            return false;
        }
        // the uri belong to target app.
        if (isTargetBundleUri[index]) {
            continue;
        }
        otherPolicyInfos[i].type = static_cast<PolicyType>(checkResult[index].permissionType);
        policyVec.emplace_back(otherPolicyInfos[i]);
    }
    return true;
}

bool BatchUri::SetCheckUriAuthorizationResult(std::vector<bool> &funcResult)
{
    if (checkResult.size() != funcResult.size()) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Invalid funcResult:%{public}zu, %{public}zu", checkResult.size(),
                 funcResult.size());
        return false;
    }
    for (size_t i = 0; i < checkResult.size(); i++) {
        funcResult[i] = checkResult[i].result;
    }
    return true;
}

int32_t BatchUri::GetPermissionedUriCount()
{
    int32_t permissionedUriCount = 0;
    for (auto &checkRes : checkResult) {
        if (checkRes.result) {
            permissionedUriCount++;
        }
    }
    return permissionedUriCount;
}

bool BatchUri::IsAllUriValid()
{
    return validUriCount == totalUriCount;
}

bool BatchUri::IsAllUriPermissioned()
{
    for (auto &checkRes : checkResult) {
        if (!checkRes.result) {
            return false;
        }
    }
    return true;
}

bool BatchUri::SetCheckProxyByContentUriResult(const std::vector<bool> &contentUriResult)
{
    if (contentUriResult.size() != contentIndexes.size()) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Invalid contentUriResult:%{public}zu, %{public}zu", contentUriResult.size(),
                 contentIndexes.size());
        return false;
    }
    for (size_t i = 0; i < contentUriResult.size(); i++) {
        auto index = contentIndexes[i];
        checkResult[index].result = contentUriResult[i];
        if (checkResult[index].result) {
            checkResult[index].permissionType = PolicyType::AUTHORIZATION_PATH;
        }
    }
    return true;
}
}  // OHOS
}  // AAFwk