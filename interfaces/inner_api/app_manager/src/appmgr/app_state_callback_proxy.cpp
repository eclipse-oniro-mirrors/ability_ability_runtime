/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "app_state_callback_proxy.h"

#include "configuration.h"
#include "ipc_types.h"

#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
AppStateCallbackProxy::AppStateCallbackProxy(const sptr<IRemoteObject> &impl) : IRemoteProxy<IAppStateCallback>(impl)
{}

bool AppStateCallbackProxy::WriteInterfaceToken(MessageParcel &data)
{
    if (!data.WriteInterfaceToken(AppStateCallbackProxy::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::APPMGR, "write interface token failed");
        return false;
    }
    return true;
}

void AppStateCallbackProxy::OnAbilityRequestDone(const sptr<IRemoteObject> &token, const AbilityState state)
{
    TAG_LOGD(AAFwkTag::APPMGR, "begin");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }

    if (token) {
        if (!data.WriteBool(true) || !data.WriteRemoteObject(token.GetRefPtr())) {
            TAG_LOGE(AAFwkTag::APPMGR, "Failed to write flag and token");
            return;
        }
    } else {
        if (!data.WriteBool(false)) {
            TAG_LOGE(AAFwkTag::APPMGR, "Failed to write flag");
            return;
        }
    }

    int32_t abilityState = static_cast<int32_t>(state);
    data.WriteInt32(abilityState);
    int32_t ret = SendTransactCmd(
        static_cast<uint32_t>(IAppStateCallback::Message::TRANSACT_ON_ABILITY_REQUEST_DONE), data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest is failed, error code: %{public}d", ret);
    }
    TAG_LOGD(AAFwkTag::APPMGR, "end");
}

void AppStateCallbackProxy::OnAppStateChanged(const AppProcessData &appProcessData)
{
    TAG_LOGD(AAFwkTag::APPMGR, "begin");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }
    data.WriteParcelable(&appProcessData);
    int32_t ret = SendTransactCmd(
        static_cast<uint32_t>(IAppStateCallback::Message::TRANSACT_ON_APP_STATE_CHANGED), data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest is failed, error code: %{public}d", ret);
    }
    TAG_LOGD(AAFwkTag::APPMGR, "end");
}

void AppStateCallbackProxy::NotifyConfigurationChange(const AppExecFwk::Configuration &config, int32_t userId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write interface token failed.");
        return;
    }
    if (!data.WriteParcelable(&config)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write config failed.");
        return;
    }
    if (!data.WriteInt32(userId)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write usr failed.");
        return;
    }
    auto error = SendTransactCmd(
        static_cast<uint32_t>(IAppStateCallback::Message::TRANSACT_ON_NOTIFY_CONFIG_CHANGE), data, reply, option);
    if (error != NO_ERROR) {
        TAG_LOGE(AAFwkTag::APPMGR, "Send config error: %{public}d", error);
    }
}

void AppStateCallbackProxy::NotifyStartResidentProcess(std::vector<AppExecFwk::BundleInfo> &bundleInfos)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "WriteInterfaceToken failed");
        return;
    }

    if (!data.WriteInt32(bundleInfos.size())) {
        TAG_LOGE(AAFwkTag::APPMGR, "write bundle info size failed.");
        return;
    }

    for (auto &bundleInfo : bundleInfos) {
        if (!data.WriteParcelable(&bundleInfo)) {
            TAG_LOGE(AAFwkTag::APPMGR, "write bundle info failed");
            return;
        }
    }
    auto ret = SendTransactCmd(
        static_cast<uint32_t>(IAppStateCallback::Message::TRANSACT_ON_NOTIFY_START_RESIDENT_PROCESS),
        data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest is failed, error code: %{public}d", ret);
    }
}

void AppStateCallbackProxy::NotifyStartKeepAliveProcess(std::vector<AppExecFwk::BundleInfo> &bundleInfos,
    pid_t diedPid)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "WriteInterfaceToken failed");
        return;
    }

    if (!data.WriteInt32(bundleInfos.size())) {
        TAG_LOGE(AAFwkTag::APPMGR, "write bundle info size failed.");
        return;
    }

    for (auto &bundleInfo : bundleInfos) {
        if (!data.WriteParcelable(&bundleInfo)) {
            TAG_LOGE(AAFwkTag::APPMGR, "write bundle info failed");
            return;
        }
    }
    if (!data.WriteInt32(static_cast<int32_t>(diedPid))) {
        TAG_LOGE(AAFwkTag::APPMGR, "write diedPid failed.");
        return;
    }
    auto ret = SendTransactCmd(
        static_cast<uint32_t>(IAppStateCallback::Message::TRANSACT_ON_NOTIFY_START_KEEP_ALIVE_PROCESS),
        data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest is failed, error code: %{public}d", ret);
    }
}

void AppStateCallbackProxy::OnAppRemoteDied(const std::vector<sptr<IRemoteObject>> &abilityTokens)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "WriteInterfaceToken failed");
        return;
    }

    if (!data.WriteInt32(abilityTokens.size())) {
        TAG_LOGE(AAFwkTag::APPMGR, "write token size failed.");
        return;
    }

    for (auto &token : abilityTokens) {
        if (!data.WriteRemoteObject(token.GetRefPtr())) {
            TAG_LOGE(AAFwkTag::APPMGR, "write token failed");
            return;
        }
    }
    auto ret = SendTransactCmd(
        static_cast<uint32_t>(IAppStateCallback::Message::TRANSACT_ON_APP_REMOTE_DIED),
        data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest is failed, error code: %{public}d", ret);
    }
}

void AppStateCallbackProxy::OnStartProcessFailed(const std::vector<sptr<IRemoteObject>> &abilityTokens)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "WriteInterfaceToken failed");
        return;
    }

    if (!data.WriteInt32(abilityTokens.size())) {
        TAG_LOGE(AAFwkTag::APPMGR, "write token size failed.");
        return;
    }
    for (auto &token : abilityTokens) {
        if (!data.WriteRemoteObject(token.GetRefPtr())) {
            TAG_LOGE(AAFwkTag::APPMGR, "write token failed");
            return;
        }
    }
    auto ret = SendTransactCmd(
        static_cast<uint32_t>(IAppStateCallback::Message::TRANSACT_ON_START_PROCESS_FAILED),
        data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest is failed, error code: %{public}d", ret);
    }
}

void AppStateCallbackProxy::OnCacheExitInfo(uint32_t accessTokenId, const RunningProcessInfo &exitInfo,
    const std::string &bundleName, const std::vector<std::string> &abilityNames,
    const std::vector<std::string> &uiExtensionNames)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "WriteInterfaceToken failed");
        return;
    }
    if (!data.WriteUint32(accessTokenId)) {
        TAG_LOGE(AAFwkTag::APPMGR, "accessTokenId write failed");
        return;
    }
    if (!data.WriteParcelable(&exitInfo)) {
        TAG_LOGE(AAFwkTag::APPMGR, "exitInfo write failed");
        return;
    }
    if (!data.WriteString(bundleName)) {
        TAG_LOGE(AAFwkTag::APPMGR, "bundleName write failed");
        return;
    }
    if (!data.WriteStringVector(abilityNames)) {
        TAG_LOGE(AAFwkTag::APPMGR, "abilityNames write failed");
        return;
    }
    if (!data.WriteStringVector(uiExtensionNames)) {
        TAG_LOGE(AAFwkTag::APPMGR, "uiExtensionNames write failed");
        return;
    }
    auto ret = SendTransactCmd(
        static_cast<uint32_t>(IAppStateCallback::Message::TRANSACT_ON_CACHE_EXIT_INFO),
        data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest is failed, error code: %{public}d", ret);
    }

}

void AppStateCallbackProxy::NotifyAppPreCache(int32_t pid, int32_t userId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "WriteInterfaceToken failed");
        return;
    }

    if (!data.WriteInt32(pid)) {
        TAG_LOGE(AAFwkTag::APPMGR, "write pid failed.");
        return;
    }

    if (!data.WriteInt32(userId)) {
        TAG_LOGE(AAFwkTag::APPMGR, "write userId failed.");
        return;
    }

    auto ret = SendTransactCmd(
        static_cast<uint32_t>(IAppStateCallback::Message::TRANSACT_ON_APP_PRE_CACHE),
        data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest is failed, error code: %{public}d", ret);
    }
}

void AppStateCallbackProxy::RecordAppExitSignalReason(int32_t pid, int32_t uid, int32_t signal,
    std::string &bundleName)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "WriteInterfaceToken failed");
        return;
    }

    if (!data.WriteInt32(pid)) {
        TAG_LOGE(AAFwkTag::APPMGR, "write pid failed.");
        return;
    }

    if (!data.WriteInt32(uid)) {
        TAG_LOGE(AAFwkTag::APPMGR, "write uid failed.");
        return;
    }

    if (!data.WriteInt32(signal)) {
        TAG_LOGE(AAFwkTag::APPMGR, "write signal failed.");
        return;
    }

    if (!data.WriteString(bundleName)) {
        TAG_LOGE(AAFwkTag::APPMGR, "write bundleName failed");
        return;
    }

    auto ret = SendTransactCmd(
        static_cast<uint32_t>(IAppStateCallback::Message::TRANSACT_ON_RECORD_APP_EXIT_SIGNAL_REASON),
        data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::APPMGR, "SendRequest is failed, error code: %{public}d", ret);
    }
}

int32_t AppStateCallbackProxy::SendTransactCmd(uint32_t code, MessageParcel &data,
    MessageParcel &reply, MessageOption &option)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "Remote is nullptr.");
        return ERR_NULL_OBJECT;
    }

    auto ret = remote->SendRequest(code, data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::APPMGR, "Send request failed with error code: %{public}d", ret);
        return ret;
    }
    return ret;
}
}  // namespace AppExecFwk
}  // namespace OHOS
