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

#ifndef OHOS_ABILITY_RUNTIME_BATCH_URI_H
#define OHOS_ABILITY_RUNTIME_BATCH_URI_H

#include <sys/types.h>
#include <vector>

#include "check_result.h"
#ifdef ABILITY_RUNTIME_FEATURE_SANDBOXMANAGER
#include "policy_info.h"
#else
#include "upms_policy_info.h"
#endif

#include "uri.h"

namespace OHOS {
namespace AAFwk {
namespace {
#ifdef ABILITY_RUNTIME_FEATURE_SANDBOXMANAGER
using PolicyInfo = AccessControl::SandboxManager::PolicyInfo;
using PolicyType = AccessControl::SandboxManager::PolicyType;
#endif
}

class BatchUri {
public:
    BatchUri() {}

    int32_t Init(const std::vector<std::string> &uriVec, uint32_t mode = 0,
                 const std::string &callerAlterBundleName = "", const std::string &targetAlterBundleName = "",
                 bool haveSandboxAccessPermission = false);

    void InitFileUriInfo(Uri &uriInner, uint32_t index, const uint32_t mode = 0,
                         const std::string &callerBundleName = "", const std::string &targetBundleName = "",
                         bool haveSandboxAccessPermission = false);

    void SetMediaUriCheckResult(const std::vector<bool> &mediaUriResult);

    void SetOtherUriCheckResult(const std::vector<bool> &otherUriResult);

    void GetNeedCheckProxyPermissionURI(std::vector<PolicyInfo> &proxyUrisByPolicy);

    bool SetCheckProxyByPolicyResult(const std::vector<bool> &proxyResultByPolicy);

    bool SetCheckProxyByContentUriResult(const std::vector<bool> &contentUriResult);

    int32_t GetUriToGrantByMap(std::vector<std::string> &uriVec);

    void SelectPermissionedUri(std::vector<Uri> &uris, std::vector<int32_t> &indexs, std::vector<std::string> &uriVec);

    int32_t GetUriToGrantByPolicy(std::vector<PolicyInfo> &docsPolicyInfoVec,
                                  std::vector<PolicyInfo> &bundlePolicyInfoVec);

    bool GetUriToGrantByPolicy(std::vector<PolicyInfo> &policyVec);

    int32_t GetPermissionedUriCount();

    // media
    int32_t GetMediaUriToGrant(std::vector<std::string> &uriVec);

    bool SetCheckUriAuthorizationResult(std::vector<bool> &funcResult);

    bool IsAllUriValid();

    bool IsAllUriPermissioned();

    // media uri
    std::vector<std::string> mediaUris;
    std::vector<int32_t> mediaIndexes;

    // docs and bundle uri
    std::vector<Uri> otherUris;
    std::vector<int32_t> otherIndexes;
    std::vector<PolicyInfo> otherPolicyInfos;

    // content uri
    std::vector<std::string> contentUris;
    std::vector<int32_t> contentIndexes;

    // caller's uri
    std::vector<PolicyInfo> selfBundlePolicyInfos;

    // for check proxy uri permission
    std::vector<int32_t> proxyIndexesByMap;
    std::vector<int32_t> proxyIndexesByPolicy;

    // result of CheckUriPermission
    std::vector<CheckResult> checkResult;
    std::vector<bool> isDocsUriVec;

    // target's uri
    int32_t targetBundleUriCount = 0;
    std::vector<bool> isTargetBundleUri;

    int32_t validUriCount = 0;
    int32_t totalUriCount = 0;
};

struct BatchStringUri {
    std::vector<std::string> uriStrVec;
    std::vector<std::string> contentUris;
    std::vector<std::string> mediaUriVec;
    std::vector<PolicyInfo> policyVec;
};
}  // OHOS
}  // AAFwk
#endif  // OHOS_ABILITY_RUNTIME_BATCH_URI_H