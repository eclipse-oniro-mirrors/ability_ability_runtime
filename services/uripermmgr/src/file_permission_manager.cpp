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

#include "file_permission_manager.h"

#include <dlfcn.h>

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
namespace {
constexpr int32_t PERMISSION_GRANTED = 1;
constexpr int32_t PERMISSION_DENIED = 2;
constexpr const char* URI_CHECK_SO_NAME = "libams_broker_ext.z.so";
constexpr const char* URI_CHECK_FUNC_NAME = "CheckCollaboratorUriPermission";
}
const std::string FILE_MANAGER_AUTHORITY = "docs";
const std::string STORAGE_URI = "/storage";
const std::string APPDATA_URI = "/storage/Users/currentUser/appdata/";
const std::string DOWNLOAD_PATH = "/storage/Users/currentUser/Download";
const std::string DESKTOP_PATH = "/storage/Users/currentUser/Desktop";
const std::string DOCUMENTS_PATH = "/storage/Users/currentUser/Documents";
const std::string CURRENTUSER = "currentUser";
const std::string BACKFLASH = "/";

DllWrapper::~DllWrapper()
{
    if (handle_ != nullptr) {
        dlclose(handle_);
    }
    func_ = nullptr;
}

bool DllWrapper::InitDlSymbol(const char* name, const char* funcName)
{
    std::lock_guard<std::mutex> lock(funcLock_);
    if (func_ != nullptr) {
        TAG_LOGD(AAFwkTag::URIPERMMGR, "func is valid.");
        return true;
    }
    if (handle_ == nullptr) {
        if (name == nullptr) {
            TAG_LOGE(AAFwkTag::URIPERMMGR, "dlopen failed: name is nullptr");
            return false;
        }
        handle_ = dlopen(name, RTLD_NOW);
        if (handle_ == nullptr) {
            TAG_LOGE(AAFwkTag::URIPERMMGR, "dlopen failed %{public}s, %{public}s", name, dlerror());
            return false;
        }
    }
    func_ = reinterpret_cast<CheckUriFunc>(dlsym(handle_, funcName));
    if (func_ == nullptr) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "dlsym failed %{public}s, %{public}s", funcName, dlerror());
        dlclose(handle_);
        handle_ = nullptr;
        return false;
    }
    return true;
}

CheckUriFunc DllWrapper::GetFunc()
{
    std::lock_guard<std::mutex> lock(funcLock_);
    return func_;
}

DllWrapper& FilePermissionManager::GetDllWrapper()
{
    static DllWrapper dll;
    return dll;
}

bool FilePermissionManager::CheckDocsUriPermission(TokenIdPermission &tokenPermission, const std::string &path)
{
    if (path.find(APPDATA_URI) == 0) {
        return tokenPermission.VerifySandboxAccessPermission();
    }

    auto& dll = GetDllWrapper();
    CheckUriFunc func = dll.GetFunc();
    if (func != nullptr) {
        int32_t ret = func(path, tokenPermission.GetTokenId());
        if (ret == PERMISSION_GRANTED) {
            return true;
        }
        if (ret == PERMISSION_DENIED) {
            return false;
        }
    }

    if (path.find(STORAGE_URI) == 0 && path.find(APPDATA_URI) != 0) {
        return tokenPermission.VerifyFileAccessManagerPermission();
    }
    return false;
}

static bool CheckFileManagerUriPermission(TokenIdPermission &tokenPermission,
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
        return tokenPermission.VerifyRWDownloadPermission();
    }
    if (path.find(DESKTOP_PATH) == 0) {
        return tokenPermission.VerifyRWDeskTopPermission();
    }
    if (path.find(DOCUMENTS_PATH) == 0) {
        return tokenPermission.VerifyRWDocumentsPermission();
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
    uint32_t callerTokenId, uint32_t flag, const std::string &bundleName, std::vector<PolicyInfo> &pathPolicies)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR,
        "CheckUriPersistentPermission call, size of uri is %{public}zu", uriVec.size());
    std::vector<bool> resultCodes(uriVec.size(), false);
    pathPolicies.clear();
    std::vector<int32_t> resultIndex;
    std::vector<PolicyInfo> persistPolicys;
    TokenIdPermission tokenPermission(callerTokenId);
    auto& dll = GetDllWrapper();
    (void)dll.InitDlSymbol(URI_CHECK_SO_NAME, URI_CHECK_FUNC_NAME);
    for (size_t i = 0; i < uriVec.size(); i++) {
        PolicyInfo policyInfo = GetPathPolicyInfoFromUri(uriVec[i], flag);
        pathPolicies.emplace_back(policyInfo);
        if ((uriVec[i].GetAuthority() == FILE_MANAGER_AUTHORITY) &&
            (CheckFileManagerUriPermission(tokenPermission, policyInfo.path, bundleName) ||
            CheckDocsUriPermission(tokenPermission, policyInfo.path))) {
            resultCodes[i] = true;
            continue;
        }
        resultIndex.emplace_back(i);
        persistPolicys.emplace_back(policyInfo);
    }

#ifdef ABILITY_RUNTIME_FEATURE_SANDBOXMANAGER
    if (persistPolicys.empty()) {
        return resultCodes;
    }
    std::vector<bool> persistResultCodes;
    int32_t ret = SandboxManagerKit::CheckPersistPolicy(callerTokenId, persistPolicys, persistResultCodes);
    if (ret == 0 && persistResultCodes.size() == resultIndex.size()) {
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