/*
 * Copyright (c) 2022-2026 Huawei Device Co., Ltd.
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

#define private public
#define protected public
#include "ability_manager_client.h"
#undef private
#undef protected
#include "ability_manager_interface.h"
#include "string_ex.h"
#include "hilog_tag_wrapper.h"
#include "ipc_skeleton.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "sys_mgr_client.h"

namespace OHOS {
namespace AAFwk {
std::mutex g_mockMutex;
sptr<IRemoteObject> g_remoteObject = nullptr;
std::shared_ptr<AbilityManagerClient> g_mockInstance = nullptr;

std::shared_ptr<AbilityManagerClient> AbilityManagerClient::GetInstance()
{
    if (g_mockInstance == nullptr) {
        std::lock_guard<std::mutex> lock_l(g_mockMutex);
        if (g_mockInstance == nullptr) {
            g_mockInstance = std::make_shared<AbilityManagerClient>();
        }
    }
    return g_mockInstance;
}

AbilityManagerClient::AbilityManagerClient()
{}

AbilityManagerClient::~AbilityManagerClient()
{}

ErrCode AbilityManagerClient::AttachAbilityThread(
    sptr<IAbilityScheduler> scheduler, sptr<IRemoteObject> token)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerClient::AttachAbilityThread start");
    ErrCode err = Connect();
    if (err != ERR_OK) {
        return ABILITY_SERVICE_NOT_CONNECTED;
    }

    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(g_remoteObject);
    return abms->AttachAbilityThread(scheduler, token);
}

ErrCode AbilityManagerClient::AbilityTransitionDone(sptr<IRemoteObject> token, int state, const PacMap& saveData)
{
    if (g_remoteObject == nullptr) {
        return ABILITY_SERVICE_NOT_CONNECTED;
    }
    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(g_remoteObject);
    return abms->AbilityTransitionDone(token, state, saveData);
}

ErrCode AbilityManagerClient::ScheduleConnectAbilityDone(
    sptr<IRemoteObject> token, sptr<IRemoteObject> remoteObject)
{
    if (g_remoteObject == nullptr) {
        return ABILITY_SERVICE_NOT_CONNECTED;
    }
    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(g_remoteObject);
    return abms->ScheduleConnectAbilityDone(token, remoteObject);
}

ErrCode AbilityManagerClient::ScheduleDisconnectAbilityDone(sptr<IRemoteObject> token)
{
    if (g_remoteObject == nullptr) {
        return ABILITY_SERVICE_NOT_CONNECTED;
    }
    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(g_remoteObject);
    return abms->ScheduleDisconnectAbilityDone(token);
}

ErrCode AbilityManagerClient::ScheduleCommandAbilityDone(sptr<IRemoteObject> token)
{
    if (g_remoteObject == nullptr) {
        TAG_LOGE(AAFwkTag::TEST, "%{private}s:ability service not command", __func__);
        return ABILITY_SERVICE_NOT_CONNECTED;
    }
    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(g_remoteObject);
    return abms->ScheduleCommandAbilityDone(token);
}

ErrCode AbilityManagerClient::ScheduleCommandAbilityWindowDone(
    sptr<IRemoteObject> token,
    sptr<SessionInfo> sessionInfo,
    WindowCommand winCmd,
    AbilityCommand abilityCmd)
{
    if (g_remoteObject == nullptr) {
        return ABILITY_SERVICE_NOT_CONNECTED;
    }
    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(g_remoteObject);
    return abms->ScheduleCommandAbilityWindowDone(token, sessionInfo, winCmd, abilityCmd);
}

ErrCode AbilityManagerClient::StartAbility(const Want& want, int32_t userId, int requestCode,
    uint64_t specifiedFullTokenId)
{
    if (g_remoteObject == nullptr) {
        return ABILITY_SERVICE_NOT_CONNECTED;
    }
    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(g_remoteObject);
    return abms->StartAbility(want, userId, requestCode);
}

ErrCode AbilityManagerClient::StartAbility(
    const Want& want, sptr<IRemoteObject> callerToken, int32_t userId, int requestCode, uint64_t specifiedFullTokenId)
{
    if (g_remoteObject == nullptr) {
        return ABILITY_SERVICE_NOT_CONNECTED;
    }
    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(g_remoteObject);
    return abms->StartAbility(want, callerToken, userId, requestCode);
}

ErrCode AbilityManagerClient::StartAbilityByCall(const Want& want, sptr<IAbilityConnection> connect,
    sptr<IRemoteObject> callerToken, int32_t accountId, bool isSilent, bool promotePriority, bool isVisible)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerClient::StartAbilityByCall start");
    if (g_remoteObject == nullptr) {
        TAG_LOGI(AAFwkTag::TEST, "AbilityManagerClient::StartAbilityByCall fail because remoteObject is null");
        g_remoteObject =
            OHOS::DelayedSingleton<AppExecFwk::SysMrgClient>::GetInstance()->GetSystemAbility(ABILITY_MGR_SERVICE_ID);
    }
    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(g_remoteObject);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerClient::StartAbilityByCall end");
    return abms->StartAbilityByCall(want, connect, callerToken);
}

ErrCode AbilityManagerClient::StartAbilityForPrelaunch(const Want& want, const int32_t frameNum,
    const sptr<IAbilityConnection> &connect)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerClient::StartAbilityForPrelaunch start");
    if (g_remoteObject == nullptr) {
        TAG_LOGI(AAFwkTag::TEST, "AbilityManagerClient::StartAbilityForPrelaunch fail because remoteObject is null");
        g_remoteObject =
            OHOS::DelayedSingleton<AppExecFwk::SysMrgClient>::GetInstance()->GetSystemAbility(ABILITY_MGR_SERVICE_ID);
    }
    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(g_remoteObject);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerClient::StartAbilityForPrelaunch end");
    return abms->StartAbilityForPrelaunch(want, frameNum, connect);
}

ErrCode AbilityManagerClient::ReleaseCall(
    sptr<IAbilityConnection> connect, const AppExecFwk::ElementName& element)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerClient::ReleaseCall start");
    if (g_remoteObject == nullptr) {
        TAG_LOGI(AAFwkTag::TEST, "AbilityManagerClient::ReleaseCall fail because remoteObject is null");
        g_remoteObject =
            OHOS::DelayedSingleton<AppExecFwk::SysMrgClient>::GetInstance()->GetSystemAbility(ABILITY_MGR_SERVICE_ID);
    }
    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(g_remoteObject);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerClient::ReleaseCall end");
    return abms->ReleaseCall(connect, element);
}

ErrCode AbilityManagerClient::TerminateAbility(
    sptr<IRemoteObject> token, int resultCode, const Want* resultWant)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerClient::TerminateAbility start");
    if (g_remoteObject == nullptr) {
        g_remoteObject =
            OHOS::DelayedSingleton<AppExecFwk::SysMrgClient>::GetInstance()->GetSystemAbility(ABILITY_MGR_SERVICE_ID);
    }
    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(g_remoteObject);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerClient::TerminateAbility end");
    return abms->TerminateAbility(token, resultCode, resultWant);
}

ErrCode AbilityManagerClient::ConnectAbility(const Want& want, sptr<IAbilityConnection> connect,
    sptr<IRemoteObject> callerToken, int32_t userId, uint64_t specifiedFullTokenId)
{
    if (g_remoteObject == nullptr) {
        g_remoteObject =
            OHOS::DelayedSingleton<AppExecFwk::SysMrgClient>::GetInstance()->GetSystemAbility(ABILITY_MGR_SERVICE_ID);
    }
    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(g_remoteObject);
    return abms->ConnectAbility(want, connect, callerToken, userId);
}

ErrCode AbilityManagerClient::DisconnectAbility(sptr<IAbilityConnection> connect)
{
    if (g_remoteObject == nullptr) {
        g_remoteObject =
            OHOS::DelayedSingleton<AppExecFwk::SysMrgClient>::GetInstance()->GetSystemAbility(ABILITY_MGR_SERVICE_ID);
    }
    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(g_remoteObject);
    return abms->DisconnectAbility(connect);
}

ErrCode AbilityManagerClient::SuspendExtensionAbility(sptr<IAbilityConnection> connect)
{
    if (g_remoteObject == nullptr) {
        g_remoteObject =
            OHOS::DelayedSingleton<AppExecFwk::SysMrgClient>::GetInstance()->GetSystemAbility(ABILITY_MGR_SERVICE_ID);
    }
    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(g_remoteObject);
    return abms->SuspendExtensionAbility(connect);
}

ErrCode AbilityManagerClient::ResumeExtensionAbility(sptr<IAbilityConnection> connect)
{
    if (g_remoteObject == nullptr) {
        g_remoteObject =
            OHOS::DelayedSingleton<AppExecFwk::SysMrgClient>::GetInstance()->GetSystemAbility(ABILITY_MGR_SERVICE_ID);
    }
    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(g_remoteObject);
    return abms->ResumeExtensionAbility(connect);
}

ErrCode AbilityManagerClient::DumpState(const std::string& args, std::vector<std::string>& state)
{
    if (g_remoteObject == nullptr) {
        return ABILITY_SERVICE_NOT_CONNECTED;
    }
    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(g_remoteObject);
    abms->DumpState(args, state);
    return ERR_OK;
}

ErrCode AbilityManagerClient::Connect()
{
    std::lock_guard<std::mutex> lock(g_mockMutex);
    g_remoteObject =
        OHOS::DelayedSingleton<AppExecFwk::SysMrgClient>::GetInstance()->GetSystemAbility(ABILITY_MGR_SERVICE_ID);
    if (g_remoteObject == nullptr) {
        TAG_LOGE(AAFwkTag::TEST, "AbilityManagerClient::Connect g_remoteObject == nullptr");
        return ERR_NO_MEMORY;
    }

    return ERR_OK;
}

ErrCode AbilityManagerClient::StopServiceAbility(const Want& want, sptr<IRemoteObject> token)
{
    if (g_remoteObject == nullptr) {
        g_remoteObject =
            OHOS::DelayedSingleton<AppExecFwk::SysMrgClient>::GetInstance()->GetSystemAbility(ABILITY_MGR_SERVICE_ID);
    }
    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(g_remoteObject);
    return abms->StopServiceAbility(want);
}

sptr<IAbilityScheduler> AbilityManagerClient::AcquireDataAbility(
    const Uri& uri, bool tryBind, sptr<IRemoteObject> callerToken)
{
    g_remoteObject =
        OHOS::DelayedSingleton<AppExecFwk::SysMrgClient>::GetInstance()->GetSystemAbility(ABILITY_MGR_SERVICE_ID);
    if (g_remoteObject == nullptr) {
        return nullptr;
    }

    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(g_remoteObject);
    return abms->AcquireDataAbility(uri, tryBind, callerToken);
}

ErrCode AbilityManagerClient::ReleaseDataAbility(
    sptr<IAbilityScheduler> dataAbilityScheduler, sptr<IRemoteObject> callerToken)
{
    g_remoteObject =
        OHOS::DelayedSingleton<AppExecFwk::SysMrgClient>::GetInstance()->GetSystemAbility(ABILITY_MGR_SERVICE_ID);
    if (g_remoteObject == nullptr) {
        return -1;
    }

    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(g_remoteObject);
    return abms->ReleaseDataAbility(dataAbilityScheduler, callerToken);
}

ErrCode AbilityManagerClient::ContinueMission(const std::string& srcDeviceId, const std::string& dstDeviceId,
    int32_t missionId, sptr<IRemoteObject> callback, AAFwk::WantParams& wantParams)
{
    if (g_remoteObject == nullptr) {
        return ABILITY_SERVICE_NOT_CONNECTED;
    }

    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(g_remoteObject);
    return abms->ContinueMission(srcDeviceId, dstDeviceId, missionId, callback, wantParams);
}

ErrCode AbilityManagerClient::StartContinuation(const Want& want, sptr<IRemoteObject> abilityToken,
    int32_t status)
{
    if (g_remoteObject == nullptr) {
        return ABILITY_SERVICE_NOT_CONNECTED;
    }

    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(g_remoteObject);
    return abms->StartContinuation(want, abilityToken, status);
}

void AbilityManagerClient::NotifyCompleteContinuation(const std::string& deviceId, int32_t sessionId, bool isSuccess)
{
    if (g_remoteObject == nullptr) {
        return;
    }

    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(g_remoteObject);
    abms->NotifyCompleteContinuation(deviceId, sessionId, isSuccess);
}

ErrCode AbilityManagerClient::StartSyncRemoteMissions(const std::string& devId, bool fixConflict, int64_t tag)
{
    if (g_remoteObject == nullptr) {
        return ABILITY_SERVICE_NOT_CONNECTED;
    }

    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(g_remoteObject);
    return abms->StartSyncRemoteMissions(devId, fixConflict, tag);
}

ErrCode AbilityManagerClient::StopSyncRemoteMissions(const std::string& devId)
{
    if (g_remoteObject == nullptr) {
        return ABILITY_SERVICE_NOT_CONNECTED;
    }

    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(g_remoteObject);
    return abms->StopSyncRemoteMissions(devId);
}

ErrCode AbilityManagerClient::StartUser(int accountId, uint64_t displayId, sptr<IUserCallback> callback,
    bool isAppRecovery)
{
    if (g_remoteObject == nullptr) {
        return ABILITY_SERVICE_NOT_CONNECTED;
    }

    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(g_remoteObject);
    return abms->StartUser(accountId, displayId, callback);
}

ErrCode AbilityManagerClient::StopUser(int accountId, sptr<IUserCallback> callback)
{
    if (g_remoteObject == nullptr) {
        return ABILITY_SERVICE_NOT_CONNECTED;
    }

    sptr<IAbilityManager> abms = iface_cast<IAbilityManager>(g_remoteObject);
    return abms->StopUser(accountId, callback);
}
} // namespace AAFwk
} // namespace OHOS
