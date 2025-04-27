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

#include "uri_permission_manager_proxy.h"

#include "ability_manager_errors.h"
#include "hilog_tag_wrapper.h"
#include "parcel.h"

namespace OHOS {
namespace AAFwk {
namespace {
const int MAX_URI_COUNT = 500;
const uint32_t CYCLE_LIMIT = 1000;
}

UriPermissionManagerProxy::UriPermissionManagerProxy(const sptr<IRemoteObject> &impl)
    : IRemoteProxy<IUriPermissionManager>(impl) {}

bool UriPermissionManagerProxy::WriteBatchUris(MessageParcel &data, const std::vector<Uri> &uriVec)
{
    std::vector<std::string> uriStrVec;
    for (auto &uri : uriVec) {
        uriStrVec.emplace_back(uri.ToString());
    }
    if (!data.WriteUint32(uriStrVec.size())) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Write uri size failed");
        return false;
    }
    if (!data.WriteStringVector(uriStrVec)) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Write uris failed");
        return false;
    }
    return true;
}

int UriPermissionManagerProxy::GrantUriPermission(const Uri &uri, unsigned int flag,
    const std::string targetBundleName, int32_t appIndex, uint32_t initiatorTokenId)
{
    TAG_LOGD(AAFwkTag::URIPERMMGR, "call");
    MessageParcel data;
    if (!data.WriteInterfaceToken(IUriPermissionManager::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Write interfaceToken failed");
        return INNER_ERR;
    }
    if (!data.WriteParcelable(&uri)) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Write uri failed");
        return INNER_ERR;
    }
    if (!data.WriteUint32(flag)) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Write flag failed");
        return INNER_ERR;
    }
    if (!data.WriteString(targetBundleName)) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Write targetBundleName failed");
        return INNER_ERR;
    }
    if (!data.WriteInt32(appIndex)) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Write appIndex failed");
        return INNER_ERR;
    }
    if (!data.WriteUint32(initiatorTokenId)) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Write initiatorTokenId failed");
        return INNER_ERR;
    }
    MessageParcel reply;
    MessageOption option;
    int error = SendTransactCmd(UriPermMgrCmd::ON_GRANT_URI_PERMISSION, data, reply, option);
    if (error != ERR_OK) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "SendRequest failed, error:%{public}d", error);
        return INNER_ERR;
    }
    return reply.ReadInt32();
}

int UriPermissionManagerProxy::GrantUriPermission(const std::vector<Uri> &uriVec, unsigned int flag,
    const std::string targetBundleName, int32_t appIndex, uint32_t initiatorTokenId)
{
    TAG_LOGD(AAFwkTag::URIPERMMGR, "call");
    if (uriVec.empty() || uriVec.size() > MAX_URI_COUNT) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "uriVec empty or exceed maxSize %{public}d", MAX_URI_COUNT);
        return ERR_URI_LIST_OUT_OF_RANGE;
    }
    MessageParcel data;
    if (!data.WriteInterfaceToken(IUriPermissionManager::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Write interfaceToken failed");
        return INNER_ERR;
    }
    if (!WriteBatchUris(data, uriVec)) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Write Batch uris failed");
        return INNER_ERR;
    }
    if (!data.WriteUint32(flag)) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Write flag failed");
        return INNER_ERR;
    }
    if (!data.WriteString(targetBundleName)) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Write targetBundleName failed");
        return INNER_ERR;
    }
    if (!data.WriteInt32(appIndex)) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Write appIndex failed");
        return INNER_ERR;
    }
    if (!data.WriteUint32(initiatorTokenId)) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Write initiatorTokenId failed");
        return INNER_ERR;
    }
    MessageParcel reply;
    MessageOption option;
    int error = SendTransactCmd(UriPermMgrCmd::ON_BATCH_GRANT_URI_PERMISSION, data, reply, option);
    if (error != ERR_OK) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "SendRequest failed, error:%{public}d", error);
        return INNER_ERR;
    }
    return reply.ReadInt32();
}

int32_t UriPermissionManagerProxy::GrantUriPermissionPrivileged(const std::vector<Uri> &uriVec, uint32_t flag,
    const std::string &targetBundleName, int32_t appIndex, uint32_t initiatorTokenId, int32_t hideSensitiveType)
{
    TAG_LOGD(AAFwkTag::URIPERMMGR, "call");
    if (uriVec.empty() || uriVec.size() > MAX_URI_COUNT) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "uriVec empty or exceed maxSize %{public}d", MAX_URI_COUNT);
        return ERR_URI_LIST_OUT_OF_RANGE;
    }
    MessageParcel data;
    if (!data.WriteInterfaceToken(IUriPermissionManager::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Write interfaceToken failed");
        return INNER_ERR;
    }
    if (!WriteBatchUris(data, uriVec)) {
        return INNER_ERR;
    }
    if (!data.WriteUint32(flag)) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Write flag failed");
        return INNER_ERR;
    }
    if (!data.WriteString(targetBundleName)) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Write targetBundleName failed");
        return INNER_ERR;
    }
    if (!data.WriteInt32(appIndex)) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Write appIndex failed");
        return INNER_ERR;
    }
    if (!data.WriteUint32(initiatorTokenId)) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Write initiatorTokenId failed");
        return INNER_ERR;
    }
    if (!data.WriteInt32(hideSensitiveType)) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Write hideSensitiveType failed");
        return INNER_ERR;
    }
    MessageParcel reply;
    MessageOption option;
    int error = SendTransactCmd(UriPermMgrCmd::ON_GRANT_URI_PERMISSION_PRIVILEGED, data, reply, option);
    if (error != ERR_OK) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "SendRequest failed, error:%{public}d", error);
        return INNER_ERR;
    }
    return reply.ReadInt32();
}

int UriPermissionManagerProxy::RevokeAllUriPermissions(const uint32_t tokenId)
{
    TAG_LOGD(AAFwkTag::URIPERMMGR, "call");
    MessageParcel data;
    if (!data.WriteInterfaceToken(IUriPermissionManager::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Write interfaceToken failed");
        return INNER_ERR;
    }
    if (!data.WriteUint32(tokenId)) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Write AccessTokenID failed");
        return INNER_ERR;
    }
    MessageParcel reply;
    MessageOption option;
    int error = SendTransactCmd(UriPermMgrCmd::ON_REVOKE_ALL_URI_PERMISSION, data, reply, option);
    if (error != ERR_OK) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "SendRequest fail, error:%{public}d", error);
        return INNER_ERR;
    }
    return reply.ReadInt32();
}

int UriPermissionManagerProxy::RevokeUriPermissionManually(const Uri &uri, const std::string bundleName,
    int32_t appIndex)
{
    TAG_LOGD(AAFwkTag::URIPERMMGR, "call");
    MessageParcel data;
    if (!data.WriteInterfaceToken(IUriPermissionManager::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Write interfaceToken failed");
        return INNER_ERR;
    }
    if (!data.WriteParcelable(&uri)) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Write uri failed");
        return INNER_ERR;
    }
    if (!data.WriteString(bundleName)) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Write bundleName failed");
        return INNER_ERR;
    }
    if (!data.WriteInt32(appIndex)) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Write appIndex failed");
        return INNER_ERR;
    }
    MessageParcel reply;
    MessageOption option;
    int error = SendTransactCmd(UriPermMgrCmd::ON_REVOKE_URI_PERMISSION_MANUALLY, data, reply, option);
    if (error != ERR_OK) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "SendRequest fail, error:%{public}d", error);
        return INNER_ERR;
    }
    return reply.ReadInt32();
}

bool UriPermissionManagerProxy::VerifyUriPermission(const Uri& uri, uint32_t flag, uint32_t tokenId)
{
    TAG_LOGD(AAFwkTag::URIPERMMGR, "call");
    MessageParcel data;
    if (!data.WriteInterfaceToken(IUriPermissionManager::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Write interfaceToken failed");
        return false;
    }
    if (!data.WriteParcelable(&uri)) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Write uri failed");
        return false;
    }
    if (!data.WriteUint32(flag)) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Write flag failed");
        return false;
    }
    if (!data.WriteUint32(tokenId)) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Write tokenId failed");
        return false;
    }
    MessageParcel reply;
    MessageOption option;
    int error = SendTransactCmd(UriPermMgrCmd::ON_VERIFY_URI_PERMISSION, data, reply, option);
    if (error != ERR_OK) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "SendRequest fail, error:%{public}d", error);
        return false;
    }
    return reply.ReadBool();
}

std::vector<bool> UriPermissionManagerProxy::CheckUriAuthorization(const std::vector<std::string> &uriVec,
    uint32_t flag, uint32_t tokenId)
{
    TAG_LOGD(AAFwkTag::URIPERMMGR, "call");
    std::vector<bool> result(uriVec.size(), false);
    if (uriVec.empty() || uriVec.size() > MAX_URI_COUNT) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "uriVec empty or exceed maxSize %{public}d", MAX_URI_COUNT);
        return result;
    }
    MessageParcel data;
    if (!data.WriteInterfaceToken(IUriPermissionManager::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Write interfaceToken failed");
        return result;
    }
    if (!data.WriteUint32(uriVec.size())) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Write uris size failed");
        return result;
    }
    if (!data.WriteStringVector(uriVec)) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Write uris failed");
        return result;
    }
    if (!data.WriteUint32(flag)) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Write flag failed");
        return result;
    }
    if (!data.WriteUint32(tokenId)) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Write tokenId failed");
        return result;
    }
    MessageParcel reply;
    MessageOption option;
    int error = SendTransactCmd(UriPermMgrCmd::ON_CHECK_URI_AUTHORIZATION, data, reply, option);
    if (error != ERR_OK) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "SendRequest error:%{public}d", error);
        return result;
    }
    auto size = reply.ReadUint32();
    if (size > CYCLE_LIMIT) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Reply size too large");
        return result;
    }
    for (auto i = 0; i < static_cast<int32_t>(size); i++) {
        result[i] = reply.ReadBool();
    }
    return result;
}

int32_t UriPermissionManagerProxy::SendTransactCmd(uint32_t code, MessageParcel &data,
    MessageParcel &reply, MessageOption &option)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "remoteObject null");
        return ERR_NULL_OBJECT;
    }

    int32_t ret = remote->SendRequest(code, data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "SendRequest failed. code:%{public}d, ret:%{public}d", code, ret);
        return ret;
    }
    return NO_ERROR;
}

int UriPermissionManagerProxy::ClearPermissionTokenByMap(const uint32_t tokenId)
{
    TAG_LOGD(AAFwkTag::URIPERMMGR, "call");
    MessageParcel data;
    if (!data.WriteInterfaceToken(IUriPermissionManager::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Write interfaceToken failed");
        return INNER_ERR;
    }
    if (!data.WriteUint32(tokenId)) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Write AccessTokenID failed");
        return INNER_ERR;
    }
    MessageParcel reply;
    MessageOption option;
    int error = SendTransactCmd(UriPermMgrCmd::ON_CLEAR_PERMISSION_TOKEN_BY_MAP, data, reply, option);
    if (error != ERR_OK) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "SendRequest fail, error:%{public}d", error);
        return INNER_ERR;
    }
    return reply.ReadInt32();
}

#ifdef ABILITY_RUNTIME_FEATURE_SANDBOXMANAGER
int UriPermissionManagerProxy::Active(const std::vector<PolicyInfo> &policy, std::vector<uint32_t> &result)
{
    TAG_LOGD(AAFwkTag::URIPERMMGR, "call");
    if (policy.empty() || policy.size() > MAX_URI_COUNT) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "uriVec empty or exceed maxSize %{public}d", MAX_URI_COUNT);
        return ERR_URI_LIST_OUT_OF_RANGE;
    }
    MessageParcel data;
    if (!data.WriteInterfaceToken(IUriPermissionManager::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Write interfaceToken failed");
        return INNER_ERR;
    }
    if (!data.WriteUint32(policy.size())) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Write size of policy failed");
        return INNER_ERR;
    }
    for (const auto &policyInfo : policy) {
        if (!data.WriteString(policyInfo.path)) {
            TAG_LOGE(AAFwkTag::URIPERMMGR, "Write policy path failed");
            return INNER_ERR;
        }
        if (!data.WriteUint64(policyInfo.mode)) {
            TAG_LOGE(AAFwkTag::URIPERMMGR, "Write policy mode failed");
            return INNER_ERR;
        }
    }
    MessageParcel reply;
    MessageOption option;
    int error = SendTransactCmd(UriPermMgrCmd::ON_ACTIVE, data, reply, option);
    if (error != ERR_OK) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "SendRequest fail, error:%{public}d", error);
        return INNER_ERR;
    }
    if (!reply.ReadUInt32Vector(&result)) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "ReadUInt32Vector failed");
        return INNER_ERR;
    }
    return reply.ReadInt32();
}
#endif // ABILITY_RUNTIME_FEATURE_SANDBOXMANAGER
}  // namespace AAFwk
}  // namespace OHOS
