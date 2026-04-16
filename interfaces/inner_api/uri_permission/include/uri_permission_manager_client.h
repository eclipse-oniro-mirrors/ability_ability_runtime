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

#ifndef OHOS_ABILITY_RUNTIME_URI_PERMISSION_MANAGER_CLIENT_H
#define OHOS_ABILITY_RUNTIME_URI_PERMISSION_MANAGER_CLIENT_H

#include <functional>
#include <sstream>

#include "check_result.h"
#include "iuri_permission_manager.h"
#include "uri.h"
#include "uri_permission_raw_data.h"
#ifdef ABILITY_RUNTIME_FEATURE_SANDBOXMANAGER
#include "policy_info.h"
#endif // ABILITY_RUNTIME_FEATURE_SANDBOXMANAGER

namespace OHOS {
namespace AAFwk {
using ProxyClearProxyCallback = std::function<void()>;
#ifdef ABILITY_RUNTIME_FEATURE_SANDBOXMANAGER
using namespace AccessControl::SandboxManager;
#endif // ABILITY_RUNTIME_FEATURE_SANDBOXMANAGER
class UriPermissionManagerClient {
public:
    static UriPermissionManagerClient& GetInstance();
    ~UriPermissionManagerClient() = default;

    /**
     * @brief Authorize the uri permission of to targetBundleName.
     * @param uri The file uri, not support content uri.
     * @param flag Want::FLAG_AUTH_READ_URI_PERMISSION or Want::FLAG_AUTH_WRITE_URI_PERMISSION.
     * @param targetBundleName The user of uri.
     */
    int GrantUriPermission(const Uri &uri, uint32_t flag, const std::string targetBundleName, int32_t appIndex = 0,
        uint32_t initiatorTokenId = 0);

    /**
     * @brief Authorize the uri permission of to targetBundleName.
     * @param uriVec The file uri list.
     * @param flag Want::FLAG_AUTH_READ_URI_PERMISSION or Want::FLAG_AUTH_WRITE_URI_PERMISSION.
     * @param targetBundleName The user of uri.
     */
    int GrantUriPermission(const std::vector<Uri> &uriVec, uint32_t flag, const std::string targetBundleName,
        int32_t appIndex = 0, uint32_t initiatorTokenId = 0);

    /**
     * @brief Authorize the uri permission to targetBundleName.
     * @param uriVec The file urilist.
     * @param flag Want::FLAG_AUTH_READ_URI_PERMISSION or Want::FLAG_AUTH_WRITE_URI_PERMISSION.
     * @param targetBundleName The user of uri.
     * @param appIndex The index of application in sandbox.
     * @param initiatorTokenId The initial caller tokenId, only for foundation.
     * @param hideSensitiveType The hide sensitive type, only for foundation.
     * @return Returns ERR_OK if the authorization is successful, otherwise returns error code.
     */
    int32_t GrantUriPermissionPrivileged(const std::vector<Uri> &uriVec, uint32_t flag,
        const std::string &targetBundleName, int32_t appIndex = 0, uint32_t initiatorTokenId = 0,
        int32_t hideSensitiveType = 0);
    
    /**
     * @brief Authorize the uri permission to targetBundleName with permission type.Only for foundation.
     * @param uriVec The file urilist.
     * @param flag Want::FLAG_AUTH_READ_URI_PERMISSION or Want::FLAG_AUTH_WRITE_URI_PERMISSION.
     * @param targetBundleName The user of uri.
     * @param appIndex The index of application in sandbox.
     * @param initiatorTokenId The initial caller tokenId.
     * @param hideSensitiveType The hide sensitive type.
     * @param permisionTypes The uri permission type.
     * @return Returns ERR_OK if the authorization is successful, otherwise returns error code.
     */
    int32_t GrantUriPermissionWithType(const std::vector<Uri> &uriVec, uint32_t flag,
        const std::string &targetBundleName, int32_t appIndex, uint32_t initiatorTokenId,
        int32_t hideSensitiveType, const std::vector<int32_t> &permissionTypes);

    /**
     * @brief Clear user's all uri authorization record with auto remove flag.
     *
     * @param tokenId A tokenId of an application.
     */
    int RevokeAllUriPermissions(const uint32_t tokenId);

    /**
     * @brief Clear user's uri authorization record.
     *
     * @param uri The file uri.
     * @param BundleName A BundleName of an application.
     * @param appIndex The index of application in sandbox.
     */
    int RevokeUriPermissionManually(const Uri &uri, const std::string bundleName, int32_t appIndex = 0);

    /**
     * @brief verify if tokenId have uri permission of flag, including temporary permission and persistable permission
     *
     * @param uri The file uri.
     * @param flag Want::FLAG_AUTH_READ_URI_PERMISSION or Want::FLAG_AUTH_WRITE_URI_PERMISSION.
     * @param tokenId A tokenId of an application.
     */
    bool VerifyUriPermission(const Uri& uri, uint32_t flag, uint32_t tokenId);

    /**
     * @brief verify if tokenId have uri permission of flag.
     * @param uri The file uri, not support content uri.
     * @param flag Want::FLAG_AUTH_READ_URI_PERMISSION or Want::FLAG_AUTH_WRITE_URI_PERMISSION.
     * @param tokenId A tokenId of an application.
     */
    std::vector<bool> CheckUriAuthorization(const std::vector<std::string> &uriVec, uint32_t flag, uint32_t tokenId);

    /**
     * @brief verify if tokenId have uri permission of flag.Only for foundation.
     * @param uri The file uri, not support content uri.
     * @param flag Want::FLAG_AUTH_READ_URI_PERMISSION or Want::FLAG_AUTH_WRITE_URI_PERMISSION.
     * @param tokenId A tokenId of an application.
     */
    std::vector<CheckResult> CheckUriAuthorizationWithType(const std::vector<std::string> &uriVec,
        uint32_t flag, uint32_t tokenId);

    int32_t ClearPermissionTokenByMap(uint32_t tokenId);

    int32_t GrantUriPermissionByKey(const std::string &key, uint32_t flag, uint32_t targetTokenId);

    int32_t GrantUriPermissionByKeyAsCaller(const std::string &key, uint32_t flag, uint32_t callerTokenId,
        uint32_t targetTokenId);

    void SetUriPermServiceStarted();

    bool IsUriPermServiceStarted();

#ifdef ABILITY_RUNTIME_FEATURE_SANDBOXMANAGER
    int32_t Active(const std::vector<PolicyInfo> &policy, std::vector<uint32_t> &result);
#endif // ABILITY_RUNTIME_FEATURE_SANDBOXMANAGER

    void OnLoadSystemAbilitySuccess(const sptr<IRemoteObject> &remoteObject);
    void OnLoadSystemAbilityFail();
private:
    UriPermissionManagerClient() = default;
    sptr<IUriPermissionManager> ConnectUriPermService();
    void ClearProxy();
    bool LoadUriPermService();
    void SetUriPermMgr(const sptr<IRemoteObject> &remoteObject);
    sptr<IUriPermissionManager> GetUriPermMgr();
    DISALLOW_COPY_AND_MOVE(UriPermissionManagerClient);
    bool RawDataToBoolVec(const UriPermissionRawData& rawData, std::vector<bool>& boolVec);
    void StringVecToRawData(const std::vector<std::string>& stringVec, UriPermissionRawData& rawData);
#ifdef ABILITY_RUNTIME_FEATURE_SANDBOXMANAGER
    void PolicyInfoToRawData(const std::vector<PolicyInfo> &policy, UriPermissionRawData &policyRawData);
#endif // ABILITY_RUNTIME_FEATURE_SANDBOXMANAGER
    class UpmsDeathRecipient : public IRemoteObject::DeathRecipient {
    public:
        explicit UpmsDeathRecipient(const ProxyClearProxyCallback& proxy) : proxy_(proxy) {}
        ~UpmsDeathRecipient() = default;
        virtual void OnRemoteDied([[maybe_unused]] const wptr<IRemoteObject>& remote) override;

    private:
        ProxyClearProxyCallback proxy_;
    };

private:
    std::mutex mutex_;
    std::mutex saLoadMutex_;
    std::condition_variable loadSaVariable_;
    bool saLoadFinished_ = false;
    sptr<IUriPermissionManager> uriPermMgr_ = nullptr;
    std::atomic_bool isUriPermServiceStarted_ = false;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_URI_PERMISSION_MANAGER_CLIENT_H
