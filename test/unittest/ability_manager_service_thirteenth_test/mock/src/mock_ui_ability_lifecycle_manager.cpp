/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#include "mock_ui_ability_lifecycle_manager.h"
#include "mock_my_status.h"

constexpr int32_t LOW_MEMORY_KILL_WHILE_STARTING = 1111;
namespace OHOS {
namespace AAFwk {

UIAbilityLifecycleManager::UIAbilityLifecycleManager(int32_t userId): userId_(userId) {}

bool UIAbilityLifecycleManager::ProcessColdStartBranch(AbilityRequest &abilityRequest, sptr<SessionInfo> sessionInfo,
    UIAbilityRecordPtr uiAbilityRecord, bool isColdStart)
{
    return true;
}

bool UIAbilityLifecycleManager::IsBundleStarting(pid_t pid)
{
    if (pid == LOW_MEMORY_KILL_WHILE_STARTING) {
        return true;
    }
    return false;
}

void UIAbilityLifecycleManager::AddStartingPid(pid_t pid)
{
}

void UIAbilityLifecycleManager::RemoveStartingPid(pid_t pid)
{
}

void UIAbilityLifecycleManager::RecordPidKilling(pid_t pid, const std::string &reason, bool isKillPrecedeStart)
{
}

void UIAbilityLifecycleManager::MarkStartingFlag(const AbilityRequest &abilityRequest)
{
}

int UIAbilityLifecycleManager::StartUIAbility(AbilityRequest &abilityRequest, sptr<SessionInfo> sessionInfo,
    AbilityRuntime::StartParamsBySCB &params, bool &isColdStart)
{
    return ERR_OK;
}

UIAbilityRecordPtr UIAbilityLifecycleManager::GenerateAbilityRecord(AbilityRequest &abilityRequest,
    sptr<SessionInfo> sessionInfo, bool &isColdStart)
{
    return nullptr;
}

UIAbilityRecordPtr UIAbilityLifecycleManager::FindRecordFromTmpMap(
    const AbilityRequest &abilityRequest)
{
    return nullptr;
}

bool UIAbilityLifecycleManager::CheckSessionInfo(sptr<SessionInfo> sessionInfo) const
{
    return true;
}

UIAbilityRecordPtr UIAbilityLifecycleManager::CreateAbilityRecord(AbilityRequest &abilityRequest,
    sptr<SessionInfo> sessionInfo) const
{
    return nullptr;
}

void UIAbilityLifecycleManager::AddCallerRecord(AbilityRequest &abilityRequest, sptr<SessionInfo> sessionInfo,
    UIAbilityRecordPtr uiAbilityRecord) const
{
}

void UIAbilityLifecycleManager::SendKeyEvent(const AbilityRequest &abilityRequest) const
{
}

int UIAbilityLifecycleManager::AttachAbilityThread(const sptr<IAbilityScheduler> &scheduler,
    const sptr<IRemoteObject> &token)
{
    return ERR_OK;
}

void UIAbilityLifecycleManager::OnAbilityRequestDone(const sptr<IRemoteObject> &token, int32_t state)
{
}

int UIAbilityLifecycleManager::AbilityTransactionDone(const sptr<IRemoteObject> &token, int state,
    const PacMap &saveData)
{
    return 0;
}

int UIAbilityLifecycleManager::AbilityWindowConfigTransactionDone(const sptr<IRemoteObject> &token,
    const WindowConfig &windowConfig)
{
    return ERR_OK;
}

bool UIAbilityLifecycleManager::AddStartCallerTimestamp(int32_t callerUid)
{
    return true;
}

int UIAbilityLifecycleManager::NotifySCBToStartUIAbility(AbilityRequest &abilityRequest)
{
    return ERR_OK;
}

int32_t UIAbilityLifecycleManager::NotifySCBToStartUIAbilities(std::vector<AbilityRequest> &abilityRequestList,
    const std::string &requestKey)
{
    return ERR_OK;
}

int32_t UIAbilityLifecycleManager::NotifySCBToRecoveryAfterInterception(const AbilityRequest &abilityRequest)
{
    return ERR_OK;
}

int UIAbilityLifecycleManager::NotifySCBToPreStartUIAbility(const AbilityRequest &abilityRequest,
    sptr<SessionInfo> &sessionInfo)
{
    return 0;
}

int UIAbilityLifecycleManager::DispatchState(const UIAbilityRecordPtr &abilityRecord, int state)
{
    return 0;
}

int UIAbilityLifecycleManager::DispatchForeground(const UIAbilityRecordPtr &abilityRecord, bool success,
    AbilityState state)
{
    return ERR_OK;
}

int UIAbilityLifecycleManager::DispatchBackground(const UIAbilityRecordPtr &abilityRecord)
{
    return ERR_OK;
}

int UIAbilityLifecycleManager::DispatchTerminate(const UIAbilityRecordPtr &abilityRecord)
{
    return ERR_OK;
}

void UIAbilityLifecycleManager::CompleteForegroundSuccess(const UIAbilityRecordPtr &abilityRecord)
{
}

void UIAbilityLifecycleManager::HandleForegroundFailed(const UIAbilityRecordPtr &ability,
    AbilityState state)
{
}

UIAbilityRecordPtr UIAbilityLifecycleManager::GetAbilityRecordByToken(const sptr<IRemoteObject> &token)
    const
{
    return MyStatus::GetInstance().ualmGetAbilityRecordByToken_;
}

#ifdef SUPPORT_SCREEN
void UIAbilityLifecycleManager::CompleteFirstFrameDrawing(const sptr<IRemoteObject> &token)
{
}
#endif

bool UIAbilityLifecycleManager::IsContainsAbility(const sptr<IRemoteObject> &token) const
{
    return true;
}

bool UIAbilityLifecycleManager::IsContainsAbilityInner(const sptr<IRemoteObject> &token) const
{
    return false;
}

void UIAbilityLifecycleManager::EraseAbilityRecord(const UIAbilityRecordPtr &abilityRecord)
{
}

std::string UIAbilityLifecycleManager::GenerateProcessNameForNewProcessMode(const AppExecFwk::AbilityInfo& abilityInfo)
{
    return "";
}

void UIAbilityLifecycleManager::PreCreateProcessName(AbilityRequest &abilityRequest)
{
}

void UIAbilityLifecycleManager::UpdateProcessName(const AbilityRequest &abilityRequest,
    UIAbilityRecordPtr &abilityRecord)
{
}

void UIAbilityLifecycleManager::UpdateAbilityRecordLaunchReason(
    const AbilityRequest &abilityRequest, UIAbilityRecordPtr &abilityRecord) const
{
}

UIAbilityRecordPtr UIAbilityLifecycleManager::GetUIAbilityRecordBySessionInfo(
    const sptr<SessionInfo> &sessionInfo)
{
    return nullptr;
}

int32_t UIAbilityLifecycleManager::NotifySCBToMinimizeUIAbility(const sptr<IRemoteObject> token)
{
    return 0;
}

int UIAbilityLifecycleManager::MinimizeUIAbility(const UIAbilityRecordPtr &abilityRecord, bool fromUser,
    uint32_t sceneFlag)
{
    return ERR_OK;
}

void UIAbilityLifecycleManager::MoveToBackground(const UIAbilityRecordPtr &abilityRecord)
{
}

int UIAbilityLifecycleManager::ResolveLocked(const AbilityRequest &abilityRequest, std::string &errMsg)
{
    return 0;
}

int UIAbilityLifecycleManager::PrelaunchAbilityLocked(const AbilityRequest &abilityRequest, const int32_t frameNum)
{
    return 0;
}

bool UIAbilityLifecycleManager::IsAbilityStarted(AbilityRequest &abilityRequest,
    std::shared_ptr<AbilityRecord> &targetRecord)
{
    return false;
}

int UIAbilityLifecycleManager::CallAbilityLocked(const AbilityRequest &abilityRequest, std::string &errMsg)
{
    return 0;
}

void UIAbilityLifecycleManager::CallUIAbilityBySCB(const sptr<SessionInfo> &sessionInfo,
    AbilityRuntime::StartParamsBySCB &params, bool &isColdStart)
{
}

sptr<SessionInfo> UIAbilityLifecycleManager::CreateSessionInfo(const AbilityRequest &abilityRequest,
    int32_t requestId) const
{
    return nullptr;
}

int UIAbilityLifecycleManager::NotifySCBPendingActivation(sptr<SessionInfo> &sessionInfo,
    const AbilityRequest &abilityRequest, std::string &errMsg)
{
    return 0;
}

bool UIAbilityLifecycleManager::IsHookModule(const AbilityRequest &abilityRequest) const
{
    return false;
}

int UIAbilityLifecycleManager::ResolveAbility(
    UIAbilityRecordPtr targetAbility, const AbilityRequest &abilityRequest) const
{
    return 0;
}

void UIAbilityLifecycleManager::NotifyAbilityToken(const sptr<IRemoteObject> &token,
    const AbilityRequest &abilityRequest) const
{
}

void UIAbilityLifecycleManager::PrintTimeOutLog(UIAbilityRecordPtr ability, uint32_t msgId, bool isHalf)
{
}

bool UIAbilityLifecycleManager::GetContentAndTypeId(uint32_t msgId, std::string &msgContent, int &typeId) const
{
    return true;
}

void UIAbilityLifecycleManager::CompleteBackground(const UIAbilityRecordPtr &abilityRecord)
{
}

int32_t UIAbilityLifecycleManager::BackToCallerAbilityWithResult(std::shared_ptr<AbilityRecord> abilityRecord,
    int resultCode, const Want *resultWant, int64_t callerRequestCode)
{
    return ERR_OK;
}

int32_t UIAbilityLifecycleManager::BackToCallerAbilityWithResultLocked(sptr<SessionInfo> currentSessionInfo,
    std::shared_ptr<AbilityRecord> callerAbilityRecord)
{
    return ERR_OK;
}

int UIAbilityLifecycleManager::CloseUIAbility(const UIAbilityRecordPtr &abilityRecord,
    int resultCode, const Want *resultWant, bool isClearSession, bool isIndependentRecovery)
{
    return ERR_OK;
}

void UIAbilityLifecycleManager::PrepareCloseUIAbility(UIAbilityRecordPtr abilityRecord,
    int resultCode, const Want *resultWant, bool isClearSession)
{
}

int UIAbilityLifecycleManager::CloseUIAbilityInner(UIAbilityRecordPtr abilityRecord)
{
    return ERR_OK;
}

void UIAbilityLifecycleManager::DelayCompleteTerminate(const UIAbilityRecordPtr &abilityRecord)
{
}

void UIAbilityLifecycleManager::CompleteTerminate(const UIAbilityRecordPtr &abilityRecord)
{
}

int32_t UIAbilityLifecycleManager::GetPersistentIdByAbilityRequest(const AbilityRequest &abilityRequest,
    bool &reuse) const
{
    return 0;
}

int32_t UIAbilityLifecycleManager::GetReusedSpecifiedPersistentId(const AbilityRequest &abilityRequest,
    bool &reuse) const
{
    return 0;
}

int32_t UIAbilityLifecycleManager::GetReusedStandardPersistentId(const AbilityRequest &abilityRequest,
    bool &reuse) const
{
    return 0;
}

int32_t UIAbilityLifecycleManager::GetReusedCollaboratorPersistentId(const AbilityRequest &abilityRequest,
    bool &reuse) const
{
    return 0;
}

bool UIAbilityLifecycleManager::CheckProperties(const UIAbilityRecordPtr &abilityRecord,
    const AbilityRequest &abilityRequest, AppExecFwk::LaunchMode launchMode) const
{
    return true;
}

void UIAbilityLifecycleManager::OnTimeOut(uint32_t msgId, int64_t abilityRecordId, bool isHalf)
{
}

void UIAbilityLifecycleManager::SetRootSceneSession(const sptr<IRemoteObject> &rootSceneSession)
{
}

void UIAbilityLifecycleManager::NotifySCBToHandleException(const UIAbilityRecordPtr &abilityRecord,
    int32_t errorCode, const std::string& errorReason, bool needClearCallerLink)
{
}

void UIAbilityLifecycleManager::NotifySCBToHandleAtomicServiceException(sptr<SessionInfo> sessionInfo,
    int32_t errorCode, const std::string& errorReason)
{
}

void UIAbilityLifecycleManager::HandleLoadTimeout(const UIAbilityRecordPtr &abilityRecord)
{
}

void UIAbilityLifecycleManager::HandleForegroundTimeout(const UIAbilityRecordPtr &abilityRecord)
{
}

void UIAbilityLifecycleManager::OnAbilityDied(UIAbilityRecordPtr abilityRecord)
{
}

void UIAbilityLifecycleManager::OnAcceptWantResponse(const AAFwk::Want &want, const std::string &flag,
    int32_t requestId)
{
}

void UIAbilityLifecycleManager::HandleLegacyAcceptWantDone(SpecifiedRequest &specifiedRequest,
    const std::string &flag, const AAFwk::Want &want)
{
}

void UIAbilityLifecycleManager::OnStartSpecifiedAbilityTimeoutResponse(int32_t requestId)
{
}

void UIAbilityLifecycleManager::OnStartSpecifiedFailed(int32_t requestId)
{
}

void UIAbilityLifecycleManager::OnStartSpecifiedProcessResponse(const std::string &flag, int32_t requestId,
    const std::string &callerProcessName, int32_t recordId)
{
}

void UIAbilityLifecycleManager::OnStartSpecifiedProcessTimeoutResponse(int32_t requestId)
{
}

int UIAbilityLifecycleManager::MoveAbilityToFront(const SpecifiedRequest &specifiedRequest,
    const UIAbilityRecordPtr abilityRecord, UIAbilityRecordPtr callerAbility)
{
    return ERR_OK;
}

int UIAbilityLifecycleManager::SendSessionInfoToSCB(UIAbilityRecordPtr &callerAbility,
    sptr<SessionInfo> &sessionInfo)
{
    return ERR_OK;
}

int UIAbilityLifecycleManager::StartAbilityBySpecified(const SpecifiedRequest &specifiedRequest,
    UIAbilityRecordPtr callerAbility)
{
    return ERR_OK;
}

void UIAbilityLifecycleManager::CallRequestDone(const UIAbilityRecordPtr &abilityRecord,
    const sptr<IRemoteObject> &callStub)
{
}

int UIAbilityLifecycleManager::ReleaseCallLocked(
    const sptr<IAbilityConnection> &connect, const AppExecFwk::ElementName &element)
{
    return ERR_OK;
}

void UIAbilityLifecycleManager::OnCallConnectDied(const std::shared_ptr<CallRecord> &callRecord)
{
}

std::vector<UIAbilityRecordPtr> UIAbilityLifecycleManager::GetAbilityRecordsByName(
    const AppExecFwk::ElementName &element, int32_t appIndex)
{
    return {};
}

std::vector<UIAbilityRecordPtr> UIAbilityLifecycleManager::GetAbilityRecordsByNameInner(
    const AppExecFwk::ElementName &element, int32_t appIndex)
{
    return {};
}

int32_t UIAbilityLifecycleManager::GetSessionIdByAbilityToken(const sptr<IRemoteObject> &token)
{
    return MyStatus::GetInstance().ualmGetSessionIdByAbilityToken_;
}

void UIAbilityLifecycleManager::SetReceiverInfo(const AbilityRequest &abilityRequest,
    UIAbilityRecordPtr abilityRecord) const
{
}

void UIAbilityLifecycleManager::SetLastExitReason(UIAbilityRecordPtr abilityRecord) const
{
}

bool UIAbilityLifecycleManager::PrepareTerminateAbility(const std::shared_ptr<AbilityRecord> &abilityRecord,
    bool isSCBCall)
{
    return true;
}

void UIAbilityLifecycleManager::PrepareTerminateAbilityDone(UIAbilityRecordPtr abilityRecord,
    bool isTerminate)
{
}

void UIAbilityLifecycleManager::SetSessionHandler(const sptr<ISessionHandler> &handler)
{
    handler_ = handler;
}

UIAbilityRecordPtr UIAbilityLifecycleManager::GetAbilityRecordsById(int32_t sessionId) const
{
    return nullptr;
}

void UIAbilityLifecycleManager::GetActiveAbilityList(int32_t uid, std::vector<std::string> &abilityList, int32_t pid)
{
}

bool UIAbilityLifecycleManager::CheckPid(const UIAbilityRecordPtr abilityRecord, const int32_t pid) const
{
    return true;
}

int32_t UIAbilityLifecycleManager::CheckAbilityNumber(
    const std::string &bundleName, const std::string &abilityName, const std::string &moduleName) const
{
    return 0;
}

void UIAbilityLifecycleManager::MoreAbilityNumbersSendEventInfo(
    int32_t userId, const std::string &bundleName, const std::string &abilityName, const std::string &moduleName)
{
}

void UIAbilityLifecycleManager::OnAppStateChanged(const AppInfo &info)
{
}

void UIAbilityLifecycleManager::UninstallApp(const std::string &bundleName, int32_t uid)
{
}

void UIAbilityLifecycleManager::GetAbilityRunningInfos(std::vector<AbilityRunningInfo> &info, bool isPerm) const
{
}

void UIAbilityLifecycleManager::Dump(std::vector<std::string> &info)
{
}

void UIAbilityLifecycleManager::DumpMissionList(
    std::vector<std::string> &info, bool isClient, const std::string &args)
{
}

void UIAbilityLifecycleManager::DumpMissionListByRecordId(std::vector<std::string> &info, bool isClient,
    int32_t abilityRecordId, const std::vector<std::string> &params)
{
}

int UIAbilityLifecycleManager::MoveMissionToFront(int32_t sessionId, std::shared_ptr<StartOptions> startOptions)
{
    return 0;
}

std::shared_ptr<StatusBarDelegateManager> UIAbilityLifecycleManager::GetStatusBarDelegateManager()
{
    return statusBarDelegateManager_;
}

int32_t UIAbilityLifecycleManager::RegisterStatusBarDelegate(sptr<AbilityRuntime::IStatusBarDelegate> delegate)
{
    return 0;
}

bool UIAbilityLifecycleManager::IsCallerInStatusBar(const std::string &instanceKey)
{
    return true;
}

bool UIAbilityLifecycleManager::IsInStatusBar(uint32_t accessTokenId, bool isMultiInstance)
{
    return true;
}

bool UIAbilityLifecycleManager::IsSupportStatusBar()
{
    return true;
}

int32_t UIAbilityLifecycleManager::DoProcessAttachment(UIAbilityRecordPtr abilityRecord)
{
    return 0;
}

int32_t UIAbilityLifecycleManager::DoCallerProcessAttachment(UIAbilityRecordPtr abilityRecord)
{
    return 0;
}

int32_t UIAbilityLifecycleManager::DoCallerProcessDetachment(UIAbilityRecordPtr abilityRecord)
{
    return 0;
}

bool UIAbilityLifecycleManager::CheckPrepareTerminateTokens(const std::vector<sptr<IRemoteObject>> &tokens,
    uint32_t &tokenId, std::map<std::string, std::vector<sptr<IRemoteObject>>> &tokensPerModuleName)
{
    return true;
}

void UIAbilityLifecycleManager::HandleAbilityStageOnPrepareTerminationTimeout(
    int32_t pid, const std::string &moduleName, const std::vector<sptr<IRemoteObject>> &tokens)
{
}

std::vector<sptr<IRemoteObject>> UIAbilityLifecycleManager::PrepareTerminateAppAndGetRemainingInner(
    int32_t pid, const std::string &moduleName, const std::vector<sptr<IRemoteObject>> &tokens)
{
    return {};
}

std::vector<sptr<IRemoteObject>> UIAbilityLifecycleManager::PrepareTerminateAppAndGetRemaining(
    int32_t pid, const std::vector<sptr<IRemoteObject>> &tokens)
{
    return {};
}

int32_t UIAbilityLifecycleManager::TryPrepareTerminateByPids(const std::vector<int32_t>& pids, bool clear)
{
    return ERR_OK;
}

void UIAbilityLifecycleManager::TryPrepareTerminateByPidsDone(const std::string &moduleName,
    int32_t prepareTermination, bool isExist)
{
}

void UIAbilityLifecycleManager::CancelPrepareTerminate(UIAbilityRecordPtr abilityRecord)
{
}

void UIAbilityLifecycleManager::BatchCloseUIAbility(
    const std::unordered_set<UIAbilityRecordPtr>& abilitySet)
{
}

void UIAbilityLifecycleManager::TerminateSession(std::shared_ptr<AbilityRecord> abilityRecord)
{
}

int UIAbilityLifecycleManager::ChangeAbilityVisibility(sptr<IRemoteObject> token, bool isShow)
{
    return 0;
}

int UIAbilityLifecycleManager::ChangeUIAbilityVisibilityBySCB(sptr<SessionInfo> sessionInfo, bool isShow)
{
    return ERR_OK;
}

int32_t UIAbilityLifecycleManager::UpdateSessionInfoBySCB(std::list<SessionInfo> &sessionInfos,
    std::vector<int32_t> &sessionIds)
{
    return ERR_OK;
}

void UIAbilityLifecycleManager::SignRestartAppFlag(int32_t uid, const std::string &instanceKey, bool isAppRecovery)
{
}

void UIAbilityLifecycleManager::CompleteFirstFrameDrawing(int32_t sessionId) const
{
}

int UIAbilityLifecycleManager::StartWithPersistentIdByDistributed(const AbilityRequest &abilityRequest,
    int32_t persistentId)
{
    return ERR_OK;
}

int32_t UIAbilityLifecycleManager::GetAbilityStateByPersistentId(int32_t persistentId, bool &state)
{
    return ERR_OK;
}

int32_t UIAbilityLifecycleManager::CleanUIAbility(
    const UIAbilityRecordPtr &abilityRecord)
{
    return 0;
}

void UIAbilityLifecycleManager::CheckCallerFromBackground(
    UIAbilityRecordPtr callerAbility, sptr<SessionInfo> &sessionInfo)
{
}

void UIAbilityLifecycleManager::EnableListForSCBRecovery()
{
}

UIAbilityRecordPtr UIAbilityLifecycleManager::FindRecordFromSessionMap(
    const AbilityRequest &abilityRequest)
{
    return nullptr;
}

void UIAbilityLifecycleManager::AddSpecifiedRequest(std::shared_ptr<SpecifiedRequest> request)
{
}

bool UIAbilityLifecycleManager::TryProcessHookModule(SpecifiedRequest &specifiedRequest, bool isHookModule)
{
    return true;
}

void UIAbilityLifecycleManager::StartSpecifiedRequest(SpecifiedRequest &specifiedRequest)
{
}

std::shared_ptr<SpecifiedRequest> UIAbilityLifecycleManager::PopAndGetNextSpecified(int32_t requestId)
{
    return nullptr;
}

bool UIAbilityLifecycleManager::IsSpecifiedModuleLoaded(const AbilityRequest &abilityRequest,
    bool isSpecifiedProcess, bool &isDebug)
{
    return true;
}

bool UIAbilityLifecycleManager::HandleStartSpecifiedCold(const AbilityRequest &abilityRequest,
    sptr<SessionInfo> sessionInfo, AbilityRuntime::StartParamsBySCB &params)
{
    return true;
}

bool UIAbilityLifecycleManager::HandleColdAcceptWantDone(const AAFwk::Want &want, const std::string &flag,
    const SpecifiedRequest &specifiedRequest)
{
    return true;
}

std::shared_ptr<SpecifiedRequest> UIAbilityLifecycleManager::GetSpecifiedRequest(int32_t requestId)
{
    return nullptr;
}

void UIAbilityLifecycleManager::SetKillForPermissionUpdateFlag(uint32_t accessTokenId)
{
}

void UIAbilityLifecycleManager::HandleForegroundCollaborate(
    const AbilityRequest &abilityRequest, UIAbilityRecordPtr abilityRecord)
{
}

bool UIAbilityLifecycleManager::UpdateSpecifiedFlag(UIAbilityRecordPtr uiAbilityRecord,
    const std::string &flag)
{
    return true;
}

int32_t UIAbilityLifecycleManager::RevokeDelegator(sptr<IRemoteObject> token)
{
    return ERR_OK;
}

int32_t UIAbilityLifecycleManager::NotifyStartupExceptionBySCB(int32_t requestId, const std::string &reason)
{
    return ERR_OK;
}

ErrCode UIAbilityLifecycleManager::IsUIAbilityAlreadyExist(const Want &want,
    const std::string &specifiedFlag, int32_t appIndex,
    const std::string &instanceKey, AppExecFwk::LaunchMode launchMode)
{
    return ERR_OK;
}

void UIAbilityLifecycleManager::HandleUIAbilityDiedByPid(pid_t pid)
{
    return;
}
}  // namespace AAFwk
}  // namespace OHOS