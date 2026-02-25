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

#include "app_state_callback_host.h"

#include "appexecfwk_errors.h"
#include "configuration.h"
#include "hitrace_meter.h"
#include "hilog_tag_wrapper.h"
#include "ipc_types.h"
#include "iremote_object.h"

#include "app_state_callback_proxy.h"

namespace OHOS {
namespace AppExecFwk {
constexpr int32_t CYCLE_LIMIT = 1000;
AppStateCallbackHost::AppStateCallbackHost() {}

AppStateCallbackHost::~AppStateCallbackHost() {}

int AppStateCallbackHost::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    TAG_LOGD(AAFwkTag::APPMGR, "AppStateCallbackHost::OnReceived, code = %{public}u, flags= %{public}d.", code,
        option.GetFlags());
    std::u16string descriptor = AppStateCallbackHost::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (descriptor != remoteDescriptor) {
        TAG_LOGE(AAFwkTag::APPMGR, "local descriptor is not equal to remote");
        return ERR_INVALID_STATE;
    }

    switch (code) {
        case static_cast<uint32_t>(IAppStateCallback::Message::TRANSACT_ON_APP_STATE_CHANGED):
            return HandleOnAppStateChanged(data, reply);
        case static_cast<uint32_t>(IAppStateCallback::Message::TRANSACT_ON_ABILITY_REQUEST_DONE):
            return HandleOnAbilityRequestDone(data, reply);
        case static_cast<uint32_t>(IAppStateCallback::Message::TRANSACT_ON_NOTIFY_CONFIG_CHANGE):
            return HandleNotifyConfigurationChange(data, reply);
        case static_cast<uint32_t>(IAppStateCallback::Message::TRANSACT_ON_NOTIFY_START_RESIDENT_PROCESS):
            return HandleNotifyStartResidentProcess(data, reply);
        case static_cast<uint32_t>(IAppStateCallback::Message::TRANSACT_ON_APP_REMOTE_DIED):
            return HandleOnAppRemoteDied(data, reply);
        case static_cast<uint32_t>(IAppStateCallback::Message::TRANSACT_ON_APP_PRE_CACHE):
            return HandleNotifyAppPreCache(data, reply);
        case static_cast<uint32_t>(IAppStateCallback::Message::TRANSACT_ON_NOTIFY_START_KEEP_ALIVE_PROCESS):
            return HandleNotifyStartKeepAliveProcess(data, reply);
        case static_cast<uint32_t>(IAppStateCallback::Message::TRANSACT_ON_START_PROCESS_FAILED):
            return HandleOnStartProcessFailed(data, reply);
        case static_cast<uint32_t>(IAppStateCallback::Message::TRANSACT_ON_CACHE_EXIT_INFO):
            return HandleOnCacheExitInfo(data, reply);
        case static_cast<uint32_t>(IAppStateCallback::Message::TRANSACT_ON_RECORD_APP_EXIT_SIGNAL_REASON):
            return HandleRecordAppExitSignalReason(data, reply);
    }

    TAG_LOGD(AAFwkTag::APPMGR, "AppStateCallbackHost::OnRemoteRequest end");
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

void AppStateCallbackHost::OnAbilityRequestDone(const sptr<IRemoteObject> &, const AbilityState)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
}

void AppStateCallbackHost::OnAppStateChanged(const AppProcessData &)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
}

void AppStateCallbackHost::NotifyAppPreCache(int32_t pid, int32_t userId)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
}

void AppStateCallbackHost::NotifyConfigurationChange(const AppExecFwk::Configuration &config, int32_t userId)
{
}

void AppStateCallbackHost::NotifyStartResidentProcess(std::vector<AppExecFwk::BundleInfo> &bundleInfos)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
}

void AppStateCallbackHost::NotifyStartKeepAliveProcess(std::vector<AppExecFwk::BundleInfo> &bundleInfos,
    pid_t diedPid)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
}

void AppStateCallbackHost::OnAppRemoteDied(const std::vector<sptr<IRemoteObject>> &abilityTokens)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
}

void AppStateCallbackHost::OnStartProcessFailed(const std::vector<sptr<IRemoteObject>> &abilityTokens)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
}

void AppStateCallbackHost::OnCacheExitInfo(uint32_t accessTokenId, const RunningProcessInfo &exitInfo,
    const std::string &bundleName, const std::vector<std::string> &abilityNames,
    const std::vector<std::string> &uiExtensionNames)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
}

void AppStateCallbackHost::RecordAppExitSignalReason(int32_t pid, int32_t uid, int32_t signal, std::string &bundleName)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
}

int32_t AppStateCallbackHost::HandleOnAppStateChanged(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    std::unique_ptr<AppProcessData> processData(data.ReadParcelable<AppProcessData>());
    if (!processData) {
        TAG_LOGE(AAFwkTag::APPMGR, "ReadParcelable<AppProcessData> failed");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }

    OnAppStateChanged(*processData);
    return NO_ERROR;
}

int32_t AppStateCallbackHost::HandleOnAbilityRequestDone(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    sptr<IRemoteObject> obj = nullptr;
    if (data.ReadBool()) {
        obj = data.ReadRemoteObject();
    }
    int32_t state = data.ReadInt32();
    OnAbilityRequestDone(obj, static_cast<AbilityState>(state));
    return NO_ERROR;
}

int32_t AppStateCallbackHost::HandleNotifyConfigurationChange(MessageParcel &data, MessageParcel &reply)
{
    std::unique_ptr<AppExecFwk::Configuration> config(data.ReadParcelable<AppExecFwk::Configuration>());
    if (config == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "To read config failed.");
        return ERR_DEAD_OBJECT;
    }
    auto userId = data.ReadInt32();
    NotifyConfigurationChange(*config, userId);
    return NO_ERROR;
}

int32_t AppStateCallbackHost::HandleNotifyStartResidentProcess(MessageParcel &data, MessageParcel &reply)
{
    std::vector<AppExecFwk::BundleInfo> bundleInfos;
    int32_t infoSize = data.ReadInt32();
    if (infoSize > CYCLE_LIMIT) {
        TAG_LOGE(AAFwkTag::APPMGR, "infoSize is too large");
        return ERR_INVALID_VALUE;
    }
    for (int32_t i = 0; i < infoSize; i++) {
        std::unique_ptr<AppExecFwk::BundleInfo> bundleInfo(data.ReadParcelable<AppExecFwk::BundleInfo>());
        if (!bundleInfo) {
            TAG_LOGE(AAFwkTag::APPMGR, "Read Parcelable infos failed.");
            return ERR_INVALID_VALUE;
        }
        bundleInfos.emplace_back(*bundleInfo);
    }
    NotifyStartResidentProcess(bundleInfos);
    return NO_ERROR;
}

int32_t AppStateCallbackHost::HandleNotifyStartKeepAliveProcess(MessageParcel &data, MessageParcel &reply)
{
    std::vector<AppExecFwk::BundleInfo> bundleInfos;
    int32_t infoSize = data.ReadInt32();
    if (infoSize > CYCLE_LIMIT) {
        TAG_LOGE(AAFwkTag::APPMGR, "infoSize is too large");
        return ERR_INVALID_VALUE;
    }
    for (int32_t i = 0; i < infoSize; i++) {
        std::unique_ptr<AppExecFwk::BundleInfo> bundleInfo(data.ReadParcelable<AppExecFwk::BundleInfo>());
        if (!bundleInfo) {
            TAG_LOGE(AAFwkTag::APPMGR, "Read Parcelable infos failed.");
            return ERR_INVALID_VALUE;
        }
        bundleInfos.emplace_back(*bundleInfo);
    }
    pid_t diedPid = static_cast<pid_t>(data.ReadInt32());
    NotifyStartKeepAliveProcess(bundleInfos, diedPid);
    return NO_ERROR;
}

int32_t AppStateCallbackHost::HandleOnAppRemoteDied(MessageParcel &data, MessageParcel &reply)
{
    std::vector<sptr<IRemoteObject>> abilityTokens;
    int32_t infoSize = data.ReadInt32();
    if (infoSize > CYCLE_LIMIT) {
        TAG_LOGE(AAFwkTag::APPMGR, "infoSize is too large");
        return ERR_INVALID_VALUE;
    }
    for (int32_t i = 0; i < infoSize; i++) {
        sptr<IRemoteObject> obj = data.ReadRemoteObject();
        if (!obj) {
            TAG_LOGE(AAFwkTag::APPMGR, "Read token failed.");
            return ERR_INVALID_VALUE;
        }
        abilityTokens.emplace_back(obj);
    }
    OnAppRemoteDied(abilityTokens);
    return NO_ERROR;
}

int32_t AppStateCallbackHost::HandleOnStartProcessFailed(MessageParcel &data, MessageParcel &reply)
{
    std::vector<sptr<IRemoteObject>> abilityTokens;
    int32_t infoSize = data.ReadInt32();
    if (infoSize > CYCLE_LIMIT) {
        TAG_LOGE(AAFwkTag::APPMGR, "infoSize is too large");
        return ERR_INVALID_VALUE;
    }
    for (int32_t i = 0; i < infoSize; i++) {
        sptr<IRemoteObject> obj = data.ReadRemoteObject();
        if (!obj) {
            TAG_LOGE(AAFwkTag::APPMGR, "Read token failed.");
            return ERR_INVALID_VALUE;
        }
        abilityTokens.emplace_back(obj);
    }
    OnStartProcessFailed(abilityTokens);
    return NO_ERROR;
}

int32_t AppStateCallbackHost::HandleNotifyAppPreCache(MessageParcel &data, MessageParcel &reply)
{
    int32_t pid = data.ReadInt32();
    if (pid <= 0) {
        TAG_LOGE(AAFwkTag::APPMGR, "pid is illegal");
        return ERR_INVALID_VALUE;
    }
    int32_t userId = data.ReadInt32();
    if (userId < 0) {
        TAG_LOGE(AAFwkTag::APPMGR, "userId is illegal");
        return ERR_INVALID_VALUE;
    }
    NotifyAppPreCache(pid, userId);
    return NO_ERROR;
}

int32_t AppStateCallbackHost::HandleOnCacheExitInfo(MessageParcel &data, MessageParcel &reply)
{
    uint32_t accessTokenId = data.ReadUint32();
    std::unique_ptr<RunningProcessInfo> exitInfo(data.ReadParcelable<RunningProcessInfo>());
    if (exitInfo == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "To read exitInfo failed");
        return ERR_DEAD_OBJECT;
    }
    std::string bundleName = data.ReadString();
    std::vector<std::string> abilityNames;
    data.ReadStringVector(&abilityNames);
    std::vector<std::string> uiExtensionNames;
    data.ReadStringVector(&uiExtensionNames);
    OnCacheExitInfo(accessTokenId, *exitInfo, bundleName, abilityNames, uiExtensionNames);

    return NO_ERROR;
}

int32_t AppStateCallbackHost::HandleRecordAppExitSignalReason(MessageParcel &data, MessageParcel &reply)
{
    int32_t pid = data.ReadInt32();
    if (pid <= 0) {
        TAG_LOGE(AAFwkTag::APPMGR, "pid is illegal");
        return ERR_INVALID_VALUE;
    }
    int32_t uid = data.ReadInt32();
    if (uid < 0) {
        TAG_LOGE(AAFwkTag::APPMGR, "uid is illegal");
        return ERR_INVALID_VALUE;
    }
    int32_t signal = data.ReadInt32();
    if (signal < 0) {
        TAG_LOGE(AAFwkTag::APPMGR, "signal is illegal");
        return ERR_INVALID_VALUE;
    }
    std::string bundleName = data.ReadString();
    RecordAppExitSignalReason(pid, uid, signal, bundleName);
    return NO_ERROR;
}
}  // namespace AppExecFwk
}  // namespace OHOS
