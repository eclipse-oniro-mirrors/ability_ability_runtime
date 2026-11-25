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

#include "application_state_observer_stub.h"
#include "appexecfwk_errors.h"
#include "hilog_tag_wrapper.h"
#include "ipc_types.h"
#include "iremote_object.h"

namespace OHOS {
namespace AppExecFwk {
std::mutex ApplicationStateObserverStub::callbackMutex_;
int ApplicationStateObserverStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    std::u16string descriptor = ApplicationStateObserverStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (descriptor != remoteDescriptor) {
        TAG_LOGE(AAFwkTag::APPMGR, "local descriptor is not equal to remote.");
        return ERR_INVALID_STATE;
    }

    switch (static_cast<Message>(code)) {
        case Message::TRANSACT_ON_FOREGROUND_APPLICATION_CHANGED:
            return HandleOnForegroundApplicationChanged(data, reply);
        case Message::TRANSACT_ON_ABILITY_STATE_CHANGED:
            return HandleOnAbilityStateChanged(data, reply);
        case Message::TRANSACT_ON_EXTENSION_STATE_CHANGED:
            return HandleOnExtensionStateChanged(data, reply);
        case Message::TRANSACT_ON_PROCESS_CREATED:
            return HandleOnProcessCreated(data, reply);
        case Message::TRANSACT_ON_PROCESS_STATE_CHANGED:
            return HandleOnProcessStateChanged(data, reply);
        case Message::TRANSACT_ON_PROCESS_DIED:
            return HandleOnProcessDied(data, reply);
        case Message::TRANSACT_ON_APPLICATION_STATE_CHANGED:
            return HandleOnApplicationStateChanged(data, reply);
        case Message::TRANSACT_ON_APP_STATE_CHANGED:
            return HandleOnAppStateChanged(data, reply);
        case Message::TRANSACT_ON_PROCESS_REUSED:
            return HandleOnProcessReused(data, reply);
        case Message::TRANSACT_ON_APP_STARTED:
            return HandleOnAppStarted(data, reply);
        case Message::TRANSACT_ON_APP_STOPPED:
            return HandleOnAppStopped(data, reply);
        case Message::TRANSACT_ON_PAGE_SHOW:
            return HandleOnPageShow(data, reply);
        case Message::TRANSACT_ON_PAGE_HIDE:
            return HandleOnPageHide(data, reply);
        case Message::TRANSACT_ON_APP_CACHE_STATE_CHANGED:
            return HandleOnAppCacheStateChanged(data, reply);
        case Message::TRANSACT_ON_WINDOW_SHOW:
            return HandleOnWindowShow(data, reply);
        case Message::TRANSACT_ON_WINDOW_HIDDEN:
            return HandleOnWindowHidden(data, reply);
        case Message::TRANSACT_ON_PROCESS_BINDINGRELATION_CHANGED:
            return HandleOnProcessBindingRelationChanged(data, reply);
        case Message::TRANSACT_ON_KEEP_ALIVE_STATE_CHANGED:
            return HandleOnKeepAliveStateChanged(data, reply);
        case Message::TRANSACT_ON_PRELOAD_PROCESS_STATE_CHANGED:
            return HandleOnProcessPreForegroundChanged(data, reply);
        case Message::TRANSACT_ON_PROCESS_TYPE_CHANGED:
            return HandleOnProcessTypeChanged(data, reply);
        case Message::TRANSACT_ON_GAME_PRELAUNCH_FOREGROUND_STATE_CHANGED:
            return HandleOnGamePrelaunchForegroundStateChanged(data, reply);
    }
    TAG_LOGW(AAFwkTag::APPMGR, "ApplicationStateObserverStub::OnRemoteRequest, default case, need check");
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

void ApplicationStateObserverStub::OnForegroundApplicationChanged(const AppStateData &appStateData)
{}

void ApplicationStateObserverStub::OnAbilityStateChanged(const AbilityStateData &abilityStateData)
{}

void ApplicationStateObserverStub::OnExtensionStateChanged(const AbilityStateData &abilityStateData)
{}

void ApplicationStateObserverStub::OnProcessCreated(const ProcessData &processData)
{}

void ApplicationStateObserverStub::OnProcessStateChanged(const ProcessData &processData)
{}

void ApplicationStateObserverStub::OnWindowShow(const ProcessData &processData)
{
    TAG_LOGD(AAFwkTag::APPMGR, "ApplicationStateObserverStub::OnWindowShow called, bundleName:%{public}s,"
        "pid:%{public}d, uid:%{public}d.", processData.bundleName.c_str(), processData.pid, processData.uid);
}

void ApplicationStateObserverStub::OnWindowHidden(const ProcessData &processData)
{
    TAG_LOGD(AAFwkTag::APPMGR, "ApplicationStateObserverStub::OnWindowHidden called, bundleName:%{public}s,"
        "pid:%{public}d, uid:%{public}d.", processData.bundleName.c_str(), processData.pid, processData.uid);
}

void ApplicationStateObserverStub::OnProcessDied(const ProcessData &processData)
{}

void ApplicationStateObserverStub::OnApplicationStateChanged(const AppStateData &appStateData)
{}

void ApplicationStateObserverStub::OnAppStateChanged(const AppStateData &appStateData)
{}

void ApplicationStateObserverStub::OnProcessReused(const ProcessData &processData)
{}

void ApplicationStateObserverStub::OnAppStarted(const AppStateData &appStateData)
{}

void ApplicationStateObserverStub::OnAppStopped(const AppStateData &appStateData)
{}

void ApplicationStateObserverStub::OnPageShow(const PageStateData &pageStateData)
{}

void ApplicationStateObserverStub::OnPageHide(const PageStateData &pageStateData)
{}

void ApplicationStateObserverStub::OnAppCacheStateChanged(const AppStateData &appStateData)
{}

void ApplicationStateObserverStub::OnProcessBindingRelationChanged(const ProcessBindData &processBindData)
{}

void ApplicationStateObserverStub::OnKeepAliveStateChanged(const ProcessData &processData)
{}

void ApplicationStateObserverStub::OnProcessPreForegroundChanged(const PreloadProcessData &preloadProcessData)
{}

void ApplicationStateObserverStub::OnProcessTypeChanged(const ProcessData &processData)
{}

void ApplicationStateObserverStub::OnGamePrelaunchForegroundStateChanged(const AppStateData &appStateData)
{}

int32_t ApplicationStateObserverStub::HandleOnForegroundApplicationChanged(MessageParcel &data, MessageParcel &reply)
{
    std::unique_ptr<AppStateData> processData(data.ReadParcelable<AppStateData>());
    if (!processData) {
        TAG_LOGE(AAFwkTag::APPMGR, "ReadParcelable<AppStateData> failed");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }

    OnForegroundApplicationChanged(*processData);
    return NO_ERROR;
}

int32_t ApplicationStateObserverStub::HandleOnAbilityStateChanged(MessageParcel &data, MessageParcel &reply)
{
    AbilityStateData* abilityStateData = nullptr;
    {
        std::unique_lock<std::mutex> lock(callbackMutex_);
        abilityStateData = data.ReadParcelable<AbilityStateData>();
        if (!abilityStateData) {
            TAG_LOGE(AAFwkTag::APPMGR, "ReadParcelable<AbilityStateData> failed");
            return ERR_APPEXECFWK_PARCEL_ERROR;
        }
    }
    OnAbilityStateChanged(*abilityStateData);
    {
        // Protect Multi Thread Deconstruct IRemoteObject
        std::unique_lock<std::mutex> lock(callbackMutex_);
        delete abilityStateData;
    }
    return NO_ERROR;
}

int32_t ApplicationStateObserverStub::HandleOnExtensionStateChanged(MessageParcel &data, MessageParcel &reply)
{
    AbilityStateData* abilityStateData = nullptr;
    {
        std::unique_lock<std::mutex> lock(callbackMutex_);
        abilityStateData = data.ReadParcelable<AbilityStateData>();
        if (!abilityStateData) {
            TAG_LOGE(AAFwkTag::APPMGR, "ReadParcelable<AbilityStateData> failed");
            return ERR_APPEXECFWK_PARCEL_ERROR;
        }
    }
    OnExtensionStateChanged(*abilityStateData);
    {
        // Protect Multi Thread Deconstruct IRemoteObject
        std::unique_lock<std::mutex> lock(callbackMutex_);
        delete abilityStateData;
    }
    return NO_ERROR;
}

int32_t ApplicationStateObserverStub::HandleOnProcessCreated(MessageParcel &data, MessageParcel &reply)
{
    std::unique_ptr<ProcessData> processData(data.ReadParcelable<ProcessData>());
    if (!processData) {
        TAG_LOGE(AAFwkTag::APPMGR, "ReadParcelable<ProcessData> failed");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }

    OnProcessCreated(*processData);
    return NO_ERROR;
}

int32_t ApplicationStateObserverStub::HandleOnProcessStateChanged(MessageParcel &data, MessageParcel &reply)
{
    std::unique_ptr<ProcessData> processData(data.ReadParcelable<ProcessData>());
    if (!processData) {
        TAG_LOGE(AAFwkTag::APPMGR, "ReadParcelable<ProcessData> failed");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }

    OnProcessStateChanged(*processData);
    return NO_ERROR;
}

int32_t ApplicationStateObserverStub::HandleOnProcessTypeChanged(MessageParcel &data, MessageParcel &reply)
{
    std::unique_ptr<ProcessData> processData(data.ReadParcelable<ProcessData>());
    if (!processData) {
        TAG_LOGE(AAFwkTag::APPMGR, "ReadParcelable<ProcessData> failed");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }

    OnProcessTypeChanged(*processData);
    return NO_ERROR;
}

int32_t ApplicationStateObserverStub::HandleOnWindowShow(MessageParcel &data, MessageParcel &reply)
{
    std::unique_ptr<ProcessData> processData(data.ReadParcelable<ProcessData>());
    if (!processData) {
        TAG_LOGE(AAFwkTag::APPMGR, "ReadParcelable<ProcessData> failed");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }

    OnWindowShow(*processData);
    return NO_ERROR;
}

int32_t ApplicationStateObserverStub::HandleOnWindowHidden(MessageParcel &data, MessageParcel &reply)
{
    std::unique_ptr<ProcessData> processData(data.ReadParcelable<ProcessData>());
    if (!processData) {
        TAG_LOGE(AAFwkTag::APPMGR, "ReadParcelable<ProcessData> failed");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }

    OnWindowHidden(*processData);
    return NO_ERROR;
}

int32_t ApplicationStateObserverStub::HandleOnProcessDied(MessageParcel &data, MessageParcel &reply)
{
    std::unique_ptr<ProcessData> processData(data.ReadParcelable<ProcessData>());
    if (!processData) {
        TAG_LOGE(AAFwkTag::APPMGR, "ReadParcelable<ProcessData> failed");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }

    OnProcessDied(*processData);
    return NO_ERROR;
}

int32_t ApplicationStateObserverStub::HandleOnApplicationStateChanged(MessageParcel &data, MessageParcel &reply)
{
    std::unique_ptr<AppStateData> processData(data.ReadParcelable<AppStateData>());
    if (!processData) {
        TAG_LOGE(AAFwkTag::APPMGR, "ReadParcelable<AppStateData> failed");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }

    OnApplicationStateChanged(*processData);
    return NO_ERROR;
}

int32_t ApplicationStateObserverStub::HandleOnAppStateChanged(MessageParcel &data, MessageParcel &reply)
{
    std::unique_ptr<AppStateData> processData(data.ReadParcelable<AppStateData>());
    if (!processData) {
        TAG_LOGE(AAFwkTag::APPMGR, "ReadParcelable<AppStateData> failed");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }

    OnAppStateChanged(*processData);
    return NO_ERROR;
}

void ApplicationStateObserverRecipient::OnRemoteDied(const wptr<IRemoteObject> &__attribute__((unused)) remote)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    if (handler_) {
        handler_(remote);
    }
}

int32_t ApplicationStateObserverStub::HandleOnProcessReused(MessageParcel &data, MessageParcel &reply)
{
    std::unique_ptr<ProcessData> processData(data.ReadParcelable<ProcessData>());
    if (!processData) {
        TAG_LOGE(AAFwkTag::APPMGR, "ReadParcelable<ProcessData> failed");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }

    OnProcessReused(*processData);
    return NO_ERROR;
}

int32_t ApplicationStateObserverStub::HandleOnAppStarted(MessageParcel &data, MessageParcel &reply)
{
    std::unique_ptr<AppStateData> processData(data.ReadParcelable<AppStateData>());
    if (!processData) {
        TAG_LOGE(AAFwkTag::APPMGR, "ReadParcelable<AppStateData> failed");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }

    OnAppStarted(*processData);
    return NO_ERROR;
}

int32_t ApplicationStateObserverStub::HandleOnAppStopped(MessageParcel &data, MessageParcel &reply)
{
    std::unique_ptr<AppStateData> processData(data.ReadParcelable<AppStateData>());
    if (!processData) {
        TAG_LOGE(AAFwkTag::APPMGR, "ReadParcelable<AppStateData> failed");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }

    OnAppStopped(*processData);
    return NO_ERROR;
}

int32_t ApplicationStateObserverStub::HandleOnPageShow(MessageParcel &data, MessageParcel &reply)
{
    std::unique_ptr<PageStateData> pageStateData(data.ReadParcelable<PageStateData>());
    if (!pageStateData) {
        TAG_LOGE(AAFwkTag::APPMGR, "ReadParcelable<PageStateData> failed");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }

    OnPageShow(*pageStateData);
    return NO_ERROR;
}

int32_t ApplicationStateObserverStub::HandleOnPageHide(MessageParcel &data, MessageParcel &reply)
{
    std::unique_ptr<PageStateData> pageStateData(data.ReadParcelable<PageStateData>());
    if (!pageStateData) {
        TAG_LOGE(AAFwkTag::APPMGR, "ReadParcelable<PageStateData> failed");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }

    OnPageHide(*pageStateData);
    return NO_ERROR;
}

int32_t ApplicationStateObserverStub::HandleOnAppCacheStateChanged(MessageParcel &data, MessageParcel &reply)
{
    std::unique_ptr<AppStateData> processData(data.ReadParcelable<AppStateData>());
    if (!processData) {
        TAG_LOGE(AAFwkTag::APPMGR, "ReadParcelable<AppStateData> failed");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }

    OnAppCacheStateChanged(*processData);
    return NO_ERROR;
}

int32_t ApplicationStateObserverStub::HandleOnProcessBindingRelationChanged(MessageParcel &data, MessageParcel &reply)
{
    std::unique_ptr<ProcessBindData> processBindData(data.ReadParcelable<ProcessBindData>());
    if (!processBindData) {
        TAG_LOGE(AAFwkTag::APPMGR, "ReadParcelable<ProcessBindData> failed");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }

    OnProcessBindingRelationChanged(*processBindData);
    return NO_ERROR;
}

int32_t ApplicationStateObserverStub::HandleOnKeepAliveStateChanged(MessageParcel &data, MessageParcel &reply)
{
    std::unique_ptr<ProcessData> processData(data.ReadParcelable<ProcessData>());
    if (!processData) {
        TAG_LOGE(AAFwkTag::APPMGR, "Handle OnKeepAliveStateChanged read ProcessData failed");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }

    OnKeepAliveStateChanged(*processData);
    return NO_ERROR;
}

int32_t ApplicationStateObserverStub::HandleOnProcessPreForegroundChanged(MessageParcel &data, MessageParcel &reply)
{
    std::unique_ptr<PreloadProcessData> preloadProcessData(data.ReadParcelable<PreloadProcessData>());
    if (!preloadProcessData) {
        TAG_LOGE(AAFwkTag::APPMGR, "ReadParcelable<PreloadProcessData> failed");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }

    OnProcessPreForegroundChanged(*preloadProcessData);
    return NO_ERROR;
}

int32_t ApplicationStateObserverStub::HandleOnGamePrelaunchForegroundStateChanged(
    MessageParcel &data, MessageParcel &reply)
{
    std::unique_ptr<AppStateData> processData(data.ReadParcelable<AppStateData>());
    if (!processData) {
        TAG_LOGE(AAFwkTag::APPMGR, "ReadParcelable<AppStateData> failed");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }

    OnGamePrelaunchForegroundStateChanged(*processData);
    return NO_ERROR;
}

ApplicationStateObserverRecipient::ApplicationStateObserverRecipient(RemoteDiedHandler handler) : handler_(handler)
{}

ApplicationStateObserverRecipient::~ApplicationStateObserverRecipient()
{}
}  // namespace AppExecFwk
}  // namespace OHOS
