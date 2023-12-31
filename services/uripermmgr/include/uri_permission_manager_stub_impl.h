/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include "app_mgr_interface.h"
#include "bundlemgr/bundle_mgr_interface.h"
#include "istorage_manager.h"
#include "uri.h"
#include "uri_bundle_event_callback.h"
#include "uri_permission_manager_stub.h"

namespace OHOS::AAFwk {
namespace {
using ClearProxyCallback = std::function<void(const wptr<IRemoteObject>&)>;
using TokenId = Security::AccessToken::AccessTokenID;
}

struct GrantInfo {
    unsigned int flag;
    const uint32_t fromTokenId;
    const uint32_t targetTokenId;
    int autoremove;
};
class UriPermissionManagerStubImpl : public UriPermissionManagerStub,
                                     public std::enable_shared_from_this<UriPermissionManagerStubImpl> {
public:
    UriPermissionManagerStubImpl() = default;
    virtual ~UriPermissionManagerStubImpl() = default;
    void Init();
    void Stop() const;

    int GrantUriPermission(const Uri &uri, unsigned int flag,
        const std::string targetBundleName, int autoremove, int32_t appIndex = 0) override;

    void RevokeUriPermission(const TokenId tokenId) override;
    void RevokeAllUriPermissions(uint32_t tokenId);
    int RevokeUriPermissionManually(const Uri &uri, const std::string bundleName) override;

private:
    template<typename T>
    void ConnectManager(sptr<T> &mgr, int32_t serviceId);
    int32_t GetCurrentAccountId() const;
    int GrantUriPermissionImpl(const Uri &uri, unsigned int flag,
        TokenId fromTokenId, TokenId targetTokenId, int autoremove);
    uint32_t GetTokenIdByBundleName(const std::string bundleName, int32_t appIndex);

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
    sptr<AppExecFwk::IBundleMgr> bundleManager_ = nullptr;
    sptr<StorageManager::IStorageManager> storageManager_ = nullptr;
    sptr<AppExecFwk::IBundleEventCallback> uriBundleEventCallback_ = nullptr;
};
}  // namespace OHOS::AAFwk
#endif  // OHOS_ABILITY_RUNTIME_URI_PERMISSION_MANAGER_STUB_IMPL_H
