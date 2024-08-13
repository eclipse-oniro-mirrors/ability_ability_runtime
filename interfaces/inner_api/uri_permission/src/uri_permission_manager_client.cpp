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

#include "uri_permission_manager_client.h"

#include "ability_manager_errors.h"
#include "hilog_tag_wrapper.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "uri_permission_load_callback.h"

namespace OHOS {
namespace AAFwk {
namespace {
const int LOAD_SA_TIMEOUT_MS = 4 * 1000;
const int MAX_URI_COUNT = 500;
} // namespace
UriPermissionManagerClient& UriPermissionManagerClient::GetInstance()
{
    static UriPermissionManagerClient instance;
    return instance;
}

int UriPermissionManagerClient::GrantUriPermission(const Uri &uri, unsigned int flag,
    const std::string targetBundleName, int32_t appIndex, uint32_t initiatorTokenId, int32_t abilityId)
{
    TAG_LOGD(AAFwkTag::URIPERMMGR, "targetBundleName:%{public}s", targetBundleName.c_str());
    auto uriPermMgr = ConnectUriPermService();
    if (uriPermMgr) {
        return uriPermMgr->GrantUriPermission(uri, flag, targetBundleName, appIndex, initiatorTokenId, abilityId);
    }
    return INNER_ERR;
}

int UriPermissionManagerClient::GrantUriPermission(const std::vector<Uri> &uriVec, unsigned int flag,
    const std::string targetBundleName, int32_t appIndex, uint32_t initiatorTokenId, int32_t abilityId)
{
    TAG_LOGD(AAFwkTag::URIPERMMGR, "targetBundleName:%{public}s, uriVecSize:%{public}zu", targetBundleName.c_str(),
        uriVec.size());
    if (uriVec.empty() || uriVec.size() > MAX_URI_COUNT) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "uriVec empty or exceed maxSize %{public}d", MAX_URI_COUNT);
        return ERR_URI_LIST_OUT_OF_RANGE;
    }
    auto uriPermMgr = ConnectUriPermService();
    if (uriPermMgr) {
        return uriPermMgr->GrantUriPermission(uriVec, flag, targetBundleName, appIndex, initiatorTokenId, abilityId);
    }
    return INNER_ERR;
}

int32_t UriPermissionManagerClient::GrantUriPermissionPrivileged(const std::vector<Uri> &uriVec, uint32_t flag,
    const std::string &targetBundleName, int32_t appIndex)
{
    TAG_LOGD(AAFwkTag::URIPERMMGR, "targetBundleName:%{public}s, uriVecSize:%{public}zu",
        targetBundleName.c_str(), uriVec.size());
    if (uriVec.empty() || uriVec.size() > MAX_URI_COUNT) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "uriVec empty or exceed maxSize %{public}d", MAX_URI_COUNT);
        return ERR_URI_LIST_OUT_OF_RANGE;
    }
    auto uriPermMgr = ConnectUriPermService();
    if (uriPermMgr) {
        return uriPermMgr->GrantUriPermissionPrivileged(uriVec, flag, targetBundleName, appIndex);
    }
    return INNER_ERR;
}


void UriPermissionManagerClient::RevokeUriPermission(const uint32_t tokenId, int32_t abilityId)
{
    TAG_LOGD(AAFwkTag::URIPERMMGR, "call");
    auto uriPermMgr = ConnectUriPermService();
    if (uriPermMgr) {
        return uriPermMgr->RevokeUriPermission(tokenId, abilityId);
    }
}

int UriPermissionManagerClient::RevokeAllUriPermissions(const uint32_t tokenId)
{
    TAG_LOGD(AAFwkTag::URIPERMMGR, "call");
    auto uriPermMgr = ConnectUriPermService();
    if (uriPermMgr) {
        return uriPermMgr->RevokeAllUriPermissions(tokenId);
    }
    return INNER_ERR;
}

int UriPermissionManagerClient::RevokeUriPermissionManually(const Uri &uri, const std::string bundleName,
    int32_t appIndex)
{
    TAG_LOGD(AAFwkTag::URIPERMMGR, "call");
    auto uriPermMgr = ConnectUriPermService();
    if (uriPermMgr) {
        return uriPermMgr->RevokeUriPermissionManually(uri, bundleName, appIndex);
    }
    return INNER_ERR;
}

bool UriPermissionManagerClient::VerifyUriPermission(const Uri& uri, uint32_t flag, uint32_t tokenId)
{
    auto uriPermMgr = ConnectUriPermService();
    if (uriPermMgr) {
        return uriPermMgr->VerifyUriPermission(uri, flag, tokenId);
    }
    return false;
}

std::vector<bool> UriPermissionManagerClient::CheckUriAuthorization(const std::vector<std::string> &uriVec,
    uint32_t flag, uint32_t tokenId)
{
    uint32_t size = uriVec.size();
    TAG_LOGD(AAFwkTag::URIPERMMGR, "flag:%{public}u, tokenId:%{public}u", flag, tokenId);
    std::vector<bool> errorRes(size, false);
    if (uriVec.empty() || uriVec.size() > MAX_URI_COUNT) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "uriVec empty or exceed maxSize %{public}d", MAX_URI_COUNT);
        return errorRes;
    }
    auto uriPermMgr = ConnectUriPermService();
    if (uriPermMgr) {
        return uriPermMgr->CheckUriAuthorization(uriVec, flag, tokenId);
    }
    return errorRes;
}

sptr<IUriPermissionManager> UriPermissionManagerClient::ConnectUriPermService()
{
    TAG_LOGD(AAFwkTag::URIPERMMGR, "call");
    auto uriPermMgr = GetUriPermMgr();
    if (uriPermMgr == nullptr) {
        if (!LoadUriPermService()) {
            TAG_LOGE(AAFwkTag::URIPERMMGR, "LoadUriPermService failed");
            return nullptr;
        }
        uriPermMgr = GetUriPermMgr();
        if (uriPermMgr == nullptr || uriPermMgr->AsObject() == nullptr) {
            TAG_LOGE(AAFwkTag::URIPERMMGR, "GetUriPermMgr failed");
            return nullptr;
        }
        const auto& onClearProxyCallback = [] {
            UriPermissionManagerClient::GetInstance().ClearProxy();
        };
        sptr<UpmsDeathRecipient> recipient(new UpmsDeathRecipient(onClearProxyCallback));
        uriPermMgr->AsObject()->AddDeathRecipient(recipient);
    }
    TAG_LOGD(AAFwkTag::URIPERMMGR, "End");
    return uriPermMgr;
}

bool UriPermissionManagerClient::LoadUriPermService()
{
    TAG_LOGD(AAFwkTag::URIPERMMGR, "call");
    auto systemAbilityMgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemAbilityMgr == nullptr) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "GetSystemAbilityManager failed");
        return false;
    }

    sptr<UriPermissionLoadCallback> loadCallback = new (std::nothrow) UriPermissionLoadCallback();
    if (loadCallback == nullptr) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Create loadCallback failed");
        return false;
    }

    auto ret = systemAbilityMgr->LoadSystemAbility(URI_PERMISSION_MGR_SERVICE_ID, loadCallback);
    if (ret != 0) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "LoadSystemAbility %{public}d failed:%{public}d",
            URI_PERMISSION_MGR_SERVICE_ID, ret);
        return false;
    }

    {
        std::unique_lock<std::mutex> lock(saLoadMutex_);
        auto waitStatus = loadSaVariable_.wait_for(lock, std::chrono::milliseconds(LOAD_SA_TIMEOUT_MS),
            [this]() {
                return saLoadFinished_;
            });
        if (!waitStatus) {
            TAG_LOGE(AAFwkTag::URIPERMMGR, "Wait for load sa timeout");
            return false;
        }
    }
    return true;
}

sptr<IUriPermissionManager> UriPermissionManagerClient::GetUriPermMgr()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return uriPermMgr_;
}

void UriPermissionManagerClient::SetUriPermMgr(const sptr<IRemoteObject> &remoteObject)
{
    TAG_LOGD(AAFwkTag::URIPERMMGR, "call");
    std::lock_guard<std::mutex> lock(mutex_);
    uriPermMgr_ = iface_cast<IUriPermissionManager>(remoteObject);
}

void UriPermissionManagerClient::OnLoadSystemAbilitySuccess(const sptr<IRemoteObject> &remoteObject)
{
    TAG_LOGD(AAFwkTag::URIPERMMGR, "call");
    SetUriPermMgr(remoteObject);
    std::unique_lock<std::mutex> lock(saLoadMutex_);
    saLoadFinished_ = true;
    loadSaVariable_.notify_one();
}

void UriPermissionManagerClient::OnLoadSystemAbilityFail()
{
    TAG_LOGD(AAFwkTag::URIPERMMGR, "call");
    SetUriPermMgr(nullptr);
    std::unique_lock<std::mutex> lock(saLoadMutex_);
    saLoadFinished_ = true;
    loadSaVariable_.notify_one();
}

void UriPermissionManagerClient::ClearProxy()
{
    TAG_LOGD(AAFwkTag::URIPERMMGR, "call");
    {
        std::lock_guard<std::mutex> lock(mutex_);
        uriPermMgr_ = nullptr;
    }
    std::unique_lock<std::mutex> lock(saLoadMutex_);
    saLoadFinished_ = false;
}

void UriPermissionManagerClient::UpmsDeathRecipient::OnRemoteDied([[maybe_unused]] const wptr<IRemoteObject>& remote)
{
    TAG_LOGE(AAFwkTag::URIPERMMGR, "call");
    proxy_();
}
}  // namespace AAFwk
}  // namespace OHOS
