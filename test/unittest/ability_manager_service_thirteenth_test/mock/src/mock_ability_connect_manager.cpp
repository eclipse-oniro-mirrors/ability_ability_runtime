/*
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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

#include "ability_connect_manager.h"
#include "mock_my_status.h"

namespace OHOS {
namespace AAFwk {
AbilityConnectManager::AbilityConnectManager(int userId) : userId_(userId)
{
}

AbilityConnectManager::~AbilityConnectManager()
{}

bool AbilityConnectManager::HasRequestIdInLoadAbilityQueue(int32_t requestId)
{
    return false;
}

void AbilityConnectManager::OnStartSpecifiedProcessResponse(const std::string &flag, int32_t requestId)
{}

void AbilityConnectManager::OnStartSpecifiedProcessTimeoutResponse(int32_t requestId)
{}

void AbilityConnectManager::StartSpecifiedProcess(
    const LoadAbilityContext &context, const std::shared_ptr<BaseExtensionRecord> &abilityRecord)
{}


int AbilityConnectManager::StartAbility(const AbilityRequest &abilityRequest)
{
    return 0;
}

int AbilityConnectManager::TerminateAbility(const sptr<IRemoteObject> &token)
{
    return 0;
}

int AbilityConnectManager::TerminateAbilityInner(const sptr<IRemoteObject> &token)
{
    return 0;
}

int AbilityConnectManager::StopServiceAbility(const AbilityRequest &abilityRequest)
{
    return 0;
}

int AbilityConnectManager::StartAbilityLocked(const AbilityRequest &abilityRequest)
{
    return ERR_OK;
}

void AbilityConnectManager::EnqueueStartServiceReq(const AbilityRequest &abilityRequest, const std::string &serviceUri)
{
}

int AbilityConnectManager::TerminateAbilityLocked(const sptr<IRemoteObject> &token)
{
    return ERR_OK;
}

int AbilityConnectManager::StopServiceAbilityLocked(const AbilityRequest &abilityRequest)
{
    return ERR_OK;
}

void AbilityConnectManager::RemoveServiceFromMapSafe(const std::string &serviceKey)
{
}


void AbilityConnectManager::GetConnectRecordListFromMap(
    const sptr<IAbilityConnection> &connect, std::list<std::shared_ptr<ConnectionRecord>> &connectRecordList)
{
}

void AbilityConnectManager::ReportEventToRSS(const AppExecFwk::AbilityInfo &abilityInfo,
    const std::shared_ptr<BaseExtensionRecord> abilityRecord, sptr<IRemoteObject> callerToken)
{
}

int AbilityConnectManager::ConnectAbilityLocked(const AbilityRequest &abilityRequest,
    const sptr<IAbilityConnection> &connect, const sptr<IRemoteObject> &callerToken, sptr<SessionInfo> sessionInfo,
    std::shared_ptr<IndirectCallerInfo> indirectCallerInfo)
{
    return ERR_OK;
}

void AbilityConnectManager::HandleActiveAbility(std::shared_ptr<BaseExtensionRecord> &targetService,
    std::shared_ptr<ConnectionRecord> &connectRecord)
{
}

std::shared_ptr<ConnectionRecord> AbilityConnectManager::GetAbilityConnectedRecordFromRecordList(
    const std::shared_ptr<BaseExtensionRecord> &targetService,
    std::list<std::shared_ptr<ConnectionRecord>> &connectRecordList)
{
    return nullptr;
}

int AbilityConnectManager::DisconnectAbilityLocked(const sptr<IAbilityConnection> &connect)
{
    return ERR_OK;
}

int AbilityConnectManager::DisconnectAbilityLocked(const sptr<IAbilityConnection> &connect, bool callerDied)
{
    return ERR_OK;
}

int32_t AbilityConnectManager::SuspendExtensionAbilityLocked(const sptr<IAbilityConnection> &connect)
{
    return ERR_OK;
}

int32_t AbilityConnectManager::ResumeExtensionAbilityLocked(const sptr<IAbilityConnection> &connect)
{
    return ERR_OK;
}

void AbilityConnectManager::TerminateRecord(std::shared_ptr<BaseExtensionRecord> abilityRecord)
{
}

int AbilityConnectManager::DisconnectRecordNormal(ConnectListType &list,
    std::shared_ptr<ConnectionRecord> connectRecord, bool callerDied) const
{
    return ERR_OK;
}

void AbilityConnectManager::DisconnectRecordForce(ConnectListType &list,
    std::shared_ptr<ConnectionRecord> connectRecord)
{
}

int AbilityConnectManager::AttachAbilityThreadLocked(
    const sptr<IAbilityScheduler> &scheduler, const sptr<IRemoteObject> &token)
{
    return ERR_OK;
}

void AbilityConnectManager::OnAppStateChanged(const AppInfo &info)
{
}

int AbilityConnectManager::AbilityTransitionDone(const sptr<IRemoteObject> &token, int state)
{
    return ERR_OK;
}

int AbilityConnectManager::AbilityWindowConfigTransactionDone(const sptr<IRemoteObject> &token,
    const WindowConfig &windowConfig)
{
    return ERR_OK;
}

void AbilityConnectManager::ProcessPreload(const std::shared_ptr<BaseExtensionRecord> &record) const
{
}

int AbilityConnectManager::ScheduleConnectAbilityDoneLocked(
    const sptr<IRemoteObject> &token, const sptr<IRemoteObject> &remoteObject)
{
    return ERR_OK;
}

void AbilityConnectManager::ProcessEliminateAbilityRecord(std::shared_ptr<BaseExtensionRecord> eliminateRecord)
{
}

void AbilityConnectManager::TerminateOrCacheAbility(std::shared_ptr<BaseExtensionRecord> abilityRecord)
{
}

int AbilityConnectManager::ScheduleDisconnectAbilityDoneLocked(const sptr<IRemoteObject> &token)
{
    return ERR_OK;
}

int AbilityConnectManager::ScheduleCommandAbilityDoneLocked(const sptr<IRemoteObject> &token)
{
    return ERR_OK;
}

int AbilityConnectManager::ScheduleCommandAbilityWindowDone(
    const sptr<IRemoteObject> &token,
    const sptr<SessionInfo> &sessionInfo,
    WindowCommand winCmd,
    AbilityCommand abilityCmd)
{
    return ERR_OK;
}

void AbilityConnectManager::HandleCommandDestroy(const sptr<SessionInfo> &sessionInfo)
{
}

void AbilityConnectManager::CompleteCommandAbility(std::shared_ptr<BaseExtensionRecord> abilityRecord)
{
}

void AbilityConnectManager::CompleteStartServiceReq(const std::string &serviceUri)
{
}

std::shared_ptr<BaseExtensionRecord> AbilityConnectManager::GetServiceRecordByAbilityRequest(
    const AbilityRequest &abilityRequest)
{
    return nullptr;
}

std::shared_ptr<BaseExtensionRecord> AbilityConnectManager::GetServiceRecordByElementName(const std::string &element)
{
    return nullptr;
}

std::shared_ptr<BaseExtensionRecord> AbilityConnectManager::GetExtensionByTokenFromServiceMap(
    const sptr<IRemoteObject> &token)
{
    return nullptr;
}

std::shared_ptr<BaseExtensionRecord> AbilityConnectManager::GetExtensionByIdFromServiceMap(
    const int64_t &abilityRecordId)
{
    return nullptr;
}

std::shared_ptr<BaseExtensionRecord> AbilityConnectManager::GetExtensionByIdFromTerminatingMap(
    const int64_t &abilityRecordId)
{
    return nullptr;
}

std::shared_ptr<BaseExtensionRecord> AbilityConnectManager::GetExtensionByTokenFromTerminatingMap(
    const sptr<IRemoteObject> &token)
{
    return nullptr;
}

void AbilityConnectManager::LoadAbility(const std::shared_ptr<BaseExtensionRecord> &abilityRecord,
    std::function<void(const std::shared_ptr<BaseExtensionRecord>&)> updateRecordCallback,
    bool isPreloadUIExtension, int32_t loadTimeout)
{
}

void AbilityConnectManager::SetExtensionLoadParam(AbilityRuntime::LoadParam &loadParam,
    std::shared_ptr<BaseExtensionRecord> abilityRecord)
{
}

bool AbilityConnectManager::IsStrictMode(std::shared_ptr<BaseExtensionRecord> abilityRecord)
{
    return true;
}

bool AbilityConnectManager::NeedExtensionControl(std::shared_ptr<BaseExtensionRecord> abilityRecord)
{
    return true;
}

void AbilityConnectManager::PostRestartResidentTask(const AbilityRequest &abilityRequest)
{
}

void AbilityConnectManager::HandleRestartResidentTask(const AbilityRequest &abilityRequest)
{
}

void AbilityConnectManager::PostTimeOutTask(const std::shared_ptr<BaseExtensionRecord> &abilityRecord,
    uint32_t messageId)
{
}

void AbilityConnectManager::PostTimeOutTask(const std::shared_ptr<BaseExtensionRecord> &abilityRecord,
    int connectRecordId, uint32_t messageId)
{
}

void AbilityConnectManager::HandleStartTimeoutTask(const std::shared_ptr<BaseExtensionRecord> &abilityRecord)
{
}

void AbilityConnectManager::HandleCommandTimeoutTask(const std::shared_ptr<BaseExtensionRecord> &abilityRecord)
{
}

void AbilityConnectManager::HandleConnectTimeoutTask(std::shared_ptr<BaseExtensionRecord> abilityRecord)
{
}

void AbilityConnectManager::HandleCommandWindowTimeoutTask(const std::shared_ptr<BaseExtensionRecord> &abilityRecord,
    const sptr<SessionInfo> &sessionInfo, WindowCommand winCmd)
{
}

void AbilityConnectManager::HandleStopTimeoutTask(const std::shared_ptr<BaseExtensionRecord> &abilityRecord)
{
}

void AbilityConnectManager::HandleTerminateDisconnectTask(const ConnectListType& connectlist)
{
}

int AbilityConnectManager::DispatchInactive(const std::shared_ptr<BaseExtensionRecord> &abilityRecord, int state,
    const sptr<IRemoteObject> &token)
{
    return ERR_OK;
}

int AbilityConnectManager::DispatchForeground(const std::shared_ptr<BaseExtensionRecord> &abilityRecord,
    const sptr<IRemoteObject> &token)
{
    return ERR_OK;
}

int AbilityConnectManager::DispatchBackground(const std::shared_ptr<BaseExtensionRecord> &abilityRecord,
    const sptr<IRemoteObject> &token)
{
    return ERR_OK;
}

int AbilityConnectManager::DispatchTerminate(const std::shared_ptr<BaseExtensionRecord> &abilityRecord,
    const sptr<IRemoteObject> &token)
{
    return ERR_OK;
}

void AbilityConnectManager::ConnectAbility(const std::shared_ptr<BaseExtensionRecord> &abilityRecord)
{
}

void AbilityConnectManager::ConnectPerConnectionTypeExtension(
    const std::shared_ptr<BaseExtensionRecord> &abilityRecord, int connectRecordId, const Want &want)
{
}

void AbilityConnectManager::ResumeConnectAbility(const std::shared_ptr<BaseExtensionRecord> &abilityRecord)
{
}

void AbilityConnectManager::CommandAbility(const std::shared_ptr<BaseExtensionRecord> &abilityRecord)
{
}

void AbilityConnectManager::CommandAbilityWindow(const std::shared_ptr<BaseExtensionRecord> &abilityRecord,
    const sptr<SessionInfo> &sessionInfo, WindowCommand winCmd)
{
}

void AbilityConnectManager::TerminateAbilityWindowLocked(const std::shared_ptr<BaseExtensionRecord> &abilityRecord,
    const sptr<SessionInfo> &sessionInfo)
{
}

void AbilityConnectManager::TerminateDone(const std::shared_ptr<BaseExtensionRecord> &abilityRecord)
{
}

void AbilityConnectManager::RemoveConnectionRecordFromMap(std::shared_ptr<ConnectionRecord> connection)
{
}

void AbilityConnectManager::RemoveServiceAbility(const std::shared_ptr<BaseExtensionRecord> &abilityRecord)
{
}

void AbilityConnectManager::AddConnectDeathRecipient(sptr<IRemoteObject> connectObject)
{
}

void AbilityConnectManager::RemoveConnectDeathRecipient(sptr<IRemoteObject> connectObject)
{
}

void AbilityConnectManager::OnCallBackDied(const wptr<IRemoteObject> &remote)
{
}

void AbilityConnectManager::HandleCallBackDiedTask(const sptr<IRemoteObject> &connect)
{
}

void AbilityConnectManager::OnLoadAbilityFailed(std::shared_ptr<BaseExtensionRecord> abilityRecord)
{
}

void AbilityConnectManager::OnAbilityDied(const std::shared_ptr<BaseExtensionRecord> abilityRecord)
{
}

void AbilityConnectManager::OnTimeOut(uint32_t msgId, int64_t abilityRecordId, bool isHalf)
{
}

void AbilityConnectManager::HandleInactiveTimeout(const std::shared_ptr<BaseExtensionRecord> &ability)
{
}

void AbilityConnectManager::CleanActivatingTimeoutAbility(std::shared_ptr<BaseExtensionRecord> abilityRecord)
{
}

bool AbilityConnectManager::IsAbilityNeedKeepAlive(const std::shared_ptr<BaseExtensionRecord> &abilityRecord)
{
    return true;
}

void AbilityConnectManager::KeepAbilityAlive(const std::shared_ptr<BaseExtensionRecord> &abilityRecord)
{
}

bool AbilityConnectManager::IsNeedToRestart(const std::shared_ptr<BaseExtensionRecord> &abilityRecord,
    const std::string &bundleName, const std::string &abilityName)
{
    return true;
}

void AbilityConnectManager::DisconnectBeforeCleanup()
{
}

void AbilityConnectManager::HandleAbilityDiedTask(
    const std::shared_ptr<BaseExtensionRecord> &abilityRecord)
{
}

static bool CheckIsNumString(const std::string &numStr)
{
    return true;
}

void AbilityConnectManager::HandleNotifyAssertFaultDialogDied(const std::shared_ptr<BaseExtensionRecord> &abilityRecord)
{
}

void AbilityConnectManager::CloseAssertDialog(const std::string &assertSessionId)
{
}

void AbilityConnectManager::RestartAbility(const std::shared_ptr<BaseExtensionRecord> &abilityRecord,
    int32_t currentUserId)
{
}

std::string AbilityConnectManager::GetServiceKey(const std::shared_ptr<BaseExtensionRecord> &service)
{
    return "";
}

void AbilityConnectManager::PauseExtensions()
{
}

void AbilityConnectManager::RemoveLauncherDeathRecipient()
{
}

bool AbilityConnectManager::IsLauncher(std::shared_ptr<BaseExtensionRecord> serviceExtension) const
{
    return true;
}

void AbilityConnectManager::KillProcessesByUserId() const
{
}

void AbilityConnectManager::CompleteForeground(const std::shared_ptr<BaseExtensionRecord> &abilityRecord)
{
}

void AbilityConnectManager::HandleForegroundTimeoutTask(const std::shared_ptr<BaseExtensionRecord> &abilityRecord)
{
}

void AbilityConnectManager::CompleteBackground(const std::shared_ptr<BaseExtensionRecord> &abilityRecord)
{
}

void AbilityConnectManager::PrintTimeOutLog(const std::shared_ptr<BaseExtensionRecord> &ability,
    uint32_t msgId, bool isHalf)
{
}

bool AbilityConnectManager::GetTimeoutMsgContent(uint32_t msgId, std::string &msgContent, int &typeId)
{
    return true;
}

void AbilityConnectManager::MoveToTerminatingMap(const std::shared_ptr<BaseExtensionRecord>& abilityRecord)
{
}

bool AbilityConnectManager::IsWindowExtensionFocused(uint32_t extensionTokenId, const sptr<IRemoteObject>& focusToken)
{
    return true;
}

void AbilityConnectManager::HandleProcessFrozen(const std::vector<int32_t> &pidList, int32_t uid)
{
}

void AbilityConnectManager::PostExtensionDelayDisconnectTask(const std::shared_ptr<ConnectionRecord> &connectRecord)
{
}

void AbilityConnectManager::RemoveExtensionDelayDisconnectTask(const std::shared_ptr<ConnectionRecord> &connectRecord)
{
}

void AbilityConnectManager::HandleExtensionDisconnectTask(const std::shared_ptr<ConnectionRecord> &connectRecord)
{
}

bool AbilityConnectManager::IsCacheExtensionAbilityByInfo(const AppExecFwk::AbilityInfo &abilityInfo)
{
    return true;
}

bool AbilityConnectManager::IsCacheExtensionAbility(const std::shared_ptr<BaseExtensionRecord> &abilityRecord)
{
    return true;
}

void AbilityConnectManager::SignRestartAppFlag(int32_t uid, const std::string &instanceKey)
{
}

bool AbilityConnectManager::AddToServiceMap(const std::string &key, std::shared_ptr<BaseExtensionRecord> abilityRecord)
{
    return true;
}

AbilityConnectManager::ServiceMapType AbilityConnectManager::GetServiceMap()
{
    std::lock_guard lock(serviceMapMutex_);
    return serviceMap_;
}

void AbilityConnectManager::AddConnectObjectToMap(sptr<IRemoteObject> connectObject,
    const ConnectListType &connectRecordList, bool updateOnly)
{
}

EventInfo AbilityConnectManager::BuildEventInfo(const std::shared_ptr<BaseExtensionRecord> &abilityRecord)
{
    EventInfo eventInfo;
    return eventInfo;
}

std::string AbilityConnectManager::GenerateBundleName(const AbilityRequest &abilityRequest)
{
    return "";
}

int32_t AbilityConnectManager::ReportXiaoYiToRSSIfNeeded(const AppExecFwk::AbilityInfo &abilityInfo)
{
    return ERR_OK;
}

int32_t AbilityConnectManager::ReportAbilityStartInfoToRSS(const AppExecFwk::AbilityInfo &abilityInfo)
{
    return ERR_OK;
}

void AbilityConnectManager::UninstallApp(const std::string &bundleName, int32_t uid)
{
}

int32_t AbilityConnectManager::UpdateKeepAliveEnableState(const std::string &bundleName,
    const std::string &moduleName, const std::string &mainElement, bool updateEnable)
{
    return ERR_OK;
}

void AbilityConnectManager::DumpState(
    ServiceMapType &serviceMapBack, std::vector<std::string> &info, bool isClient, const std::string &args)
{
}

void AbilityConnectManager::DumpStateByUri(ServiceMapType &serviceMapBack, std::vector<std::string> &info,
    bool isClient, const std::string &args, std::vector<std::string> &params)
{
}

void AbilityConnectManager::GetExtensionRunningInfos(ServiceMapType &serviceMapBack, int upperLimit,
    std::vector<ExtensionRunningInfo> &info, const int32_t userId, bool isPerm)
{
}

void AbilityConnectManager::GetExtensionRunningInfo(
    std::shared_ptr<BaseExtensionRecord> &abilityRecord, const int32_t userId, std::vector<ExtensionRunningInfo> &info)
{
}

std::shared_ptr<BaseExtensionRecord> AbilityConnectManager::GetUIExtensionBySessionFromServiceMap(
    const sptr<SessionInfo> &sessionInfo)
{
    return MyStatus::GetInstance().acmGetUIExtensionBySessionFromServiceMap_;
}

void AbilityConnectManager::HandleLoadAbilityOrStartSpecifiedProcess(
    const AbilityRuntime::LoadParam &loadParam, const std::shared_ptr<BaseExtensionRecord> &abilityRecord)
{
}

bool AbilityConnectManager::IsSpecialAbility(const AppExecFwk::AbilityInfo &abilityInfo)
{
    return false;
}

int AbilityConnectManager::UpdateStateAndCompleteDisconnect(const std::shared_ptr<BaseExtensionRecord> &abilityRecord,
    const std::shared_ptr<ConnectionRecord> &connect, const sptr<IRemoteObject> &token)
{
    return 0;
}

void AbilityConnectManager::HandleConnectRecordOnAbilityDied(const std::shared_ptr<BaseExtensionRecord> &abilityRecord)
{
}

void AbilityConnectManager::HandleAfterServiceRemoved(
    const std::shared_ptr<BaseExtensionRecord> &abilityRecord, bool isRemove)
{
}

void AbilityConnectManager::HandlePostConnectTimeout(const std::shared_ptr<BaseExtensionRecord> &abilityRecord,
    int connectRecordId, int64_t recordId)
{
}

void AbilityConnectManager::HandleStartTimeoutTaskInner(const std::shared_ptr<BaseExtensionRecord> &abilityRecord)
{
}

void AbilityConnectManager::HandleForegroundTimeoutTaskInner(const std::shared_ptr<BaseExtensionRecord> &abilityRecord)
{
}

void AbilityConnectManager::HandleStopTimeoutTaskInner(const std::shared_ptr<BaseExtensionRecord> &abilityRecord)
{
}

void AbilityConnectManager::CleanActivatingTimeoutAbilityInner(std::shared_ptr<BaseExtensionRecord> abilityRecord)
{
}

void AbilityConnectManager::HandleAbilityDiedTaskInner(const std::shared_ptr<BaseExtensionRecord> &abilityRecord)
{
}

bool AbilityConnectManager::HandleExtensionAbilityRemove(const std::shared_ptr<BaseExtensionRecord> &abilityRecord)
{
    return false;
}

void AbilityConnectManager::PostLoadTimeoutTask(
    const std::shared_ptr<BaseExtensionRecord> &abilityRecord, int32_t loadTimeout)
{
}


int AbilityConnectManager::CheckAbilityStateForDisconnect(const std::shared_ptr<BaseExtensionRecord> &abilityRecord)
{
    return 0;
}

int AbilityConnectManager::CleanupConnectionAndTerminateIfNeeded(std::shared_ptr<BaseExtensionRecord> &abilityRecord)
{
    return 0;
}

void AbilityConnectManager::CompleteForegroundInner(const std::shared_ptr<BaseExtensionRecord> &abilityRecord)
{
}

int32_t AbilityConnectManager::ConnectAbilityLockedInner(bool isLoadedAbility,
    std::shared_ptr<BaseExtensionRecord>& targetService, const AbilityRequest& abilityRequest,
    std::shared_ptr<ConnectionRecord>& connectRecord)
{
    return 0;
}
}  // namespace AAFwk
}  // namespace OHOS
