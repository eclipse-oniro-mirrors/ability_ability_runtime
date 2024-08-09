/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_URI_PERMISSION_MANAGER_STUB_IMPL_H
#define OHOS_ABILITY_RUNTIME_URI_PERMISSION_MANAGER_STUB_IMPL_H

#include <functional>
#include <map>
#include <vector>
#include <unordered_set>

#include "app_mgr_interface.h"
#include "istorage_manager.h"
#include "tokenid_permission.h"
#include "uri.h"
#include "uri_permission_manager_stub.h"

namespace OHOS::AAFwk {
namespace {
using ClearProxyCallback = std::function<void(const wptr<IRemoteObject>&)>;
using TokenId = Security::AccessToken::AccessTokenID;
constexpr int32_t DEFAULT_ABILITY_ID = -1;
}

struct GrantInfo {
    unsigned int flag;
    const uint32_t fromTokenId;
    const uint32_t targetTokenId;
    bool autoRemove;
    std::unordered_set<int32_t> abilityIds;

    void AddAbilityId(int32_t abilityId)
    {
        if (abilityId != DEFAULT_ABILITY_ID) {
            abilityIds.insert(abilityId);
        }
    }

    bool RemoveAbilityId(int32_t abilityId)
    {
        return abilityIds.erase(abilityId) > 0;
    }

    bool IsEmptyAbilityId()
    {
        return abilityIds.empty();
    }

    void ClearAbilityIds()
    {
        abilityIds.clear();
    }
};

struct PolicyInfo final {
public:
    std::string path;
    uint64_t mode;
};

class UriPermissionManagerStubImpl : public UriPermissionManagerStub,
                                     public std::enable_shared_from_this<UriPermissionManagerStubImpl> {
public:
    UriPermissionManagerStubImpl() = default;
    virtual ~UriPermissionManagerStubImpl() = default;

    int GrantUriPermission(const Uri &uri, unsigned int flag, const std::string targetBundleName,
        int32_t appIndex = 0, uint32_t initiatorTokenId = 0, int32_t abilityId = -1) override;
    int GrantUriPermission(const std::vector<Uri> &uriVec, unsigned int flag,
        const std::string targetBundleName, int32_t appIndex = 0, uint32_t initiatorTokenId = 0,
        int32_t abilityId = -1) override;
    int32_t GrantUriPermissionPrivileged(const std::vector<Uri> &uriVec, uint32_t flag,
        const std::string &targetBundleName, int32_t appIndex = 0) override;
    
    std::vector<bool> CheckUriAuthorization(const std::vector<std::string> &uriVec, uint32_t flag,
        uint32_t tokenId) override;

    // only for foundation calling
    void RevokeUriPermission(const TokenId tokenId, int32_t abilityId = -1) override;
    int RevokeAllUriPermissions(uint32_t tokenId) override;
    int RevokeUriPermissionManually(const Uri &uri, const std::string bundleName,
        int32_t appIndex = 0) override;

    bool VerifyUriPermission(const Uri &uri, uint32_t flag, uint32_t tokenId) override;

private:
    template<typename T>
    void ConnectManager(sptr<T> &mgr, int32_t serviceId);
    int GrantUriPermissionImpl(const Uri &uri, unsigned int flag,
        TokenId fromTokenId, TokenId targetTokenId, int32_t abilityId);
    int AddTempUriPermission(const std::string &uri, unsigned int flag, TokenId fromTokenId,
        TokenId targetTokenId, int32_t abilityId);

    int GrantBatchUriPermissionImpl(const std::vector<std::string> &uriVec, unsigned int flag,
        TokenId initiatorTokenId, TokenId targetTokenId, int32_t abilityId);
    int GrantBatchUriPermission(const std::vector<Uri> &uriVec, unsigned int flag, uint32_t initiatorTokenId,
        uint32_t targetTokenId, int32_t abilityId);
    
    int32_t GrantBatchUriPermissionPrivileged(const std::vector<Uri> &uriVec, uint32_t flag,
        uint32_t callerTokenId, uint32_t targetTokenId, int32_t abilityId = -1);
    
    int32_t GrantBatchUriPermissionFor2In1Privileged(const std::vector<Uri> &uriVec, uint32_t flag,
        uint32_t callerTokenId, uint32_t targetTokenId, int32_t abilityId = -1);

    int GrantSingleUriPermission(const Uri &uri, unsigned int flag, uint32_t callerTokenId, uint32_t targetTokenId,
        int32_t abilityId);

    int32_t CheckCalledBySandBox();

    bool CheckUriPermission(Uri uri, uint32_t flag, TokenIdPermission &tokenIdPermission);

    bool CheckUriTypeIsValid(Uri uri);

    int GrantUriPermissionInner(const std::vector<Uri> &uriVec, unsigned int flag, const std::string targetBundleName,
        int32_t appIndex, uint32_t initiatorTokenId, int32_t abilityId = -1);

    int GrantUriPermissionFor2In1Inner(const std::vector<Uri> &uriVec, unsigned int flag,
        const std::string &targetBundleName, int32_t appIndex, bool isSystemAppCall, uint32_t initiatorTokenId = 0,
        int32_t abilityId = -1);

    void HandleUriPermission(
        uint64_t tokenId, unsigned int flag, std::vector<PolicyInfo> &docsVec, bool isSystemAppCall);

    int32_t CheckProxyUriPermission(TokenIdPermission &tokenIdPermission, const Uri &uri, uint32_t flag);

    bool AccessMediaUriPermission(TokenIdPermission &tokenIdPermission, const Uri &uri, uint32_t flag);

    bool AccessDocsUriPermission(TokenIdPermission &tokenIdPermission, const Uri &uri, uint32_t flag);
    
    int32_t DeleteShareFile(uint32_t targetTokenId, const std::vector<std::string> &uriVec);

    void RemoveUriRecord(std::vector<std::string> &uriList, const TokenId tokenId, int32_t abilityId);

    bool VerifySubDirUriPermission(const std::string &uriStr, uint32_t newFlag, uint32_t tokenId);

    bool IsDistributedSubDirUri(const std::string &inputUri, const std::string &cachedUri);

    class ProxyDeathRecipient : public IRemoteObject::DeathRecipient {
    public:
        explicit ProxyDeathRecipient(ClearProxyCallback&& proxy) : proxy_(proxy) {}
        ~ProxyDeathRecipient() = default;
        virtual void OnRemoteDied([[maybe_unused]] const wptr<IRemoteObject>& remote) override;

    private:
        ClearProxyCallback proxy_;
    };

private:
    std::map<std::string, std::list<GrantInfo>> uriMap_;
    std::mutex mutex_;
    std::mutex mgrMutex_;
    sptr<AppExecFwk::IAppMgr> appMgr_ = nullptr;
    sptr<StorageManager::IStorageManager> storageManager_ = nullptr;
};
}  // namespace OHOS::AAFwk
#endif  // OHOS_ABILITY_RUNTIME_URI_PERMISSION_MANAGER_STUB_IMPL_H
