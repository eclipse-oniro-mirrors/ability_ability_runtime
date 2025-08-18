/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include "file_permission_manager.h"

#include "accesstoken_kit.h"
#include "file_uri.h"
#include "hilog_tag_wrapper.h"
#include "ipc_skeleton.h"
#include "permission_constants.h"
#include "permission_verification.h"
#include "tokenid_kit.h"
#include "uri.h"

namespace OHOS {
namespace AAFwk {
constexpr const uint32_t SANDBOX_MANAGER_OK = 0;
const std::string FILE_MANAGER_AUTHORITY = "docs";
const std::string CURRENTUSER_URI = "file://docs/storage/Users/currentUser";
const std::string APPDATA_URI = "file://docs/storage/Users/currentUser/appdata";
const std::string DOWNLOAD_PATH = "/storage/Users/currentUser/Download";
const std::string DESKTOP_PATH = "/storage/Users/currentUser/Desktop";
const std::string DOCUMENTS_PATH = "/storage/Users/currentUser/Documents";
const std::string CURRENTUSER = "currentUser";
const std::string BACKFLASH = "/";

static bool CheckPermission(uint64_t tokenCaller, const std::string &permission)
{
    return PermissionVerification::GetInstance()->VerifyPermissionByTokenId(tokenCaller, permission);
}
static bool CheckDocsUriPermission(uint64_t callerTokenId, const std::string &uri)
{
    if (CheckPermission(callerTokenId, PermissionConstants::PERMISSION_FILE_ACCESS_MANAGER)) {
        if (uri.find(CURRENTUSER_URI) == 0 && uri.find(APPDATA_URI) != 0) {
            return true;
        }
    }
    if (uri.find(APPDATA_URI) == 0 &&
        CheckPermission(
            callerTokenId,
            PermissionConstants::PERMISSION_SANDBOX_ACCESS_MANAGER)) {
        return true;
    }
    return false;
}

static bool CheckFileManagerUriPermission(uint64_t providerTokenId,
                                          const std::string &filePath,
                                          const std::string &bundleName)
{
    std::string path = filePath;
    if (path.find(DOWNLOAD_PATH) == 0) {
        path = path.substr(DOWNLOAD_PATH.size());
        if (path.find(BACKFLASH) == 0) {
            path = path.substr(1);
        }
        std::string dirname = "";
        if (path.find(BACKFLASH) != std::string::npos) {
            size_t pos = path.find(BACKFLASH);
            dirname = path.substr(0, pos);
        } else {
            dirname = path;
        }
        if (dirname == bundleName) {
            return true;
        }
        return CheckPermission(providerTokenId, PermissionConstants::PERMISSION_READ_WRITE_DOWNLOAD);
    }
    if (path.find(DESKTOP_PATH) == 0) {
        return CheckPermission(providerTokenId, PermissionConstants::PERMISSION_READ_WRITE_DESKTON);
    }
    if (path.find(DOCUMENTS_PATH) == 0) {
        return CheckPermission(providerTokenId, PermissionConstants::PERMISSION_READ_WRITE_DOCUMENTS);
    }
    return false;
}

PolicyInfo FilePermissionManager::GetPathPolicyInfoFromUri(Uri &uri, uint32_t flag, const std::string &bundleName)
{
    AppFileService::ModuleFileUri::FileUri fileUri(uri.ToString());
    std::string path = fileUri.GetRealPathBySA(bundleName);
    PolicyInfo policyInfo;
    policyInfo.path = path;
    policyInfo.mode = (flag & (OperationMode::READ_MODE | OperationMode::WRITE_MODE));
    return policyInfo;
}

std::vector<bool> FilePermissionManager::CheckUriPersistentPermission(std::vector<Uri> &uriVec,
    uint32_t callerTokenId, uint32_t flag, std::vector<PolicyInfo> &pathPolicies, const std::string &bundleName)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR,
        "CheckUriPersistentPermission call, size of uri is %{public}zu", uriVec.size());
    std::vector<bool> resultCodes(uriVec.size(), false);
    pathPolicies.clear();
    std::vector<int32_t> resultIndex;
    std::vector<PolicyInfo> persistPolicys;
    for (size_t i = 0; i < uriVec.size(); i++) {
        PolicyInfo policyInfo = GetPathPolicyInfoFromUri(uriVec[i], flag);
        pathPolicies.emplace_back(policyInfo);
        if ((uriVec[i].GetAuthority() == FILE_MANAGER_AUTHORITY) &&
            (CheckFileManagerUriPermission(callerTokenId, policyInfo.path,
                                           bundleName) ||
             CheckDocsUriPermission(callerTokenId, uriVec[i].ToString()))) {
            resultCodes[i] = true;
            continue;
        }
        resultIndex.emplace_back(i);
        persistPolicys.emplace_back(policyInfo);
    }
#ifdef ABILITY_RUNTIME_FEATURE_SANDBOXMANAGER
    std::vector<bool> persistResultCodes;
    int32_t ret = SandboxManagerKit::CheckPersistPolicy(callerTokenId, persistPolicys, persistResultCodes);
    if (ret == SANDBOX_MANAGER_OK && persistResultCodes.size() == resultIndex.size()) {
        for (size_t i = 0; i < persistResultCodes.size(); i++) {
            auto index = resultIndex[i];
            resultCodes[index] = persistResultCodes[i];
        }
    }
#endif
    return resultCodes;
}
}
}
