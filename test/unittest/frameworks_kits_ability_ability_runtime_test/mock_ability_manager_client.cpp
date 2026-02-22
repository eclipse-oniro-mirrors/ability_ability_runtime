/*
 * Copyright (c) 2023-2026 Huawei Device Co., Ltd.
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
#include "mock_my_flag.h"
#include "start_params_by_SCB.h"
#undef private
#undef protected

namespace OHOS {
namespace AAFwk {
using namespace AppExecFwk;
namespace {
constexpr const int32_t ERR_OK = 0;
}
std::shared_ptr<AbilityManagerClient> AbilityManagerClient::instance_ = nullptr;
std::once_flag AbilityManagerClient::singletonFlag_;

std::shared_ptr<AbilityManagerClient> AbilityManagerClient::GetInstance()
{
    std::call_once(singletonFlag_, [] () {
        instance_ = std::make_shared<AbilityManagerClient>();
    });
    return instance_;
}

AbilityManagerClient::AbilityManagerClient()
{}

AbilityManagerClient::~AbilityManagerClient()
{}

ErrCode AbilityManagerClient::AttachAbilityThread(
    sptr<IAbilityScheduler> scheduler, sptr<IRemoteObject> token)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::AbilityTransitionDone(sptr<IRemoteObject> token, int state, const PacMap &saveData)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::AbilityWindowConfigTransitionDone(
    sptr<IRemoteObject> token, const WindowConfig &windowConfig)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::ScheduleConnectAbilityDone(
    sptr<IRemoteObject> token, sptr<IRemoteObject> remoteObject)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::ScheduleDisconnectAbilityDone(sptr<IRemoteObject> token)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::ScheduleCommandAbilityDone(sptr<IRemoteObject> token)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::ScheduleCommandAbilityWindowDone(
    sptr<IRemoteObject> token,
    sptr<SessionInfo> sessionInfo,
    WindowCommand winCmd,
    AbilityCommand abilityCmd)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::StartAbility(
    const Want &want, int requestCode, int32_t userId, uint64_t specifiedFullTokenId)
{
    return MyFlag::GetInstance()->GetStartAbility();
}

ErrCode AbilityManagerClient::StartAbility(
    const Want &want, sptr<IRemoteObject> callerToken, int requestCode, int32_t userId, uint64_t specifiedFullTokenId)
{
    return MyFlag::GetInstance()->GetStartAbility();
}

ErrCode AbilityManagerClient::StartAbilityByInsightIntent(
    const Want &want, sptr<IRemoteObject> callerToken, uint64_t intentId, int32_t userId)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::StartAbility(const Want &want, const AbilityStartSetting &abilityStartSetting,
    sptr<IRemoteObject> callerToken, int requestCode, int32_t userId)
{
    return MyFlag::GetInstance()->GetStartAbility();
}

ErrCode AbilityManagerClient::StartAbility(const Want &want, const StartOptions &startOptions,
    sptr<IRemoteObject> callerToken, int requestCode, int32_t userId)
{
    return MyFlag::GetInstance()->GetStartAbility();
}

ErrCode AbilityManagerClient::StartAbilityAsCaller(
    const Want &want, sptr<IRemoteObject> callerToken,
    sptr<IRemoteObject> asCallerSourceToken, int requestCode, int32_t userId)
{
    return MyFlag::GetInstance()->GetStartAbilityAsCaller();
}

ErrCode AbilityManagerClient::StartAbilityAsCaller(const Want &want, const StartOptions &startOptions,
    sptr<IRemoteObject> callerToken, sptr<IRemoteObject> asCallerSourceToken,
    int requestCode, int32_t userId)
{
    return MyFlag::GetInstance()->GetStartAbilityAsCaller();
}

ErrCode AbilityManagerClient::StartAbilityForResultAsCaller(
    const Want &want, sptr<IRemoteObject> callerToken, int requestCode, int32_t userId)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::StartAbilityForResultAsCaller(const Want &want, const StartOptions &startOptions,
    sptr<IRemoteObject> callerToken, int requestCode, int32_t userId)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::StartAbilityByUIContentSession(const Want &want, const StartOptions &startOptions,
    sptr<IRemoteObject> callerToken, sptr<AAFwk::SessionInfo> sessionInfo,
    int requestCode, int32_t userId)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::StartAbilityByUIContentSession(const Want &want, sptr<IRemoteObject> callerToken,
    sptr<AAFwk::SessionInfo> sessionInfo, int requestCode, int32_t userId)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::StartAbilityOnlyUIAbility(const Want &want, sptr<IRemoteObject> callerToken,
    uint32_t specifyTokenId)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::SendResultToAbility(int requestCode, int resultCode, Want& resultWant)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::StartExtensionAbility(const Want &want, sptr<IRemoteObject> callerToken,
    int32_t userId, AppExecFwk::ExtensionAbilityType extensionType)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::RequestModalUIExtension(const Want &want)
{
    return MyFlag::GetInstance()->GetRequestModalUIExtension();
}

ErrCode AbilityManagerClient::PreloadUIExtensionAbility(const Want &want, std::string &hostBundleName,
    int32_t userId, int32_t hostPid, int32_t requestCode)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::ChangeAbilityVisibility(sptr<IRemoteObject> token, bool isShow)
{
    return MyFlag::GetInstance()->GetChangeAbilityVisibility();
}

ErrCode AbilityManagerClient::ChangeUIAbilityVisibilityBySCB(sptr<SessionInfo> sessionInfo, bool isShow)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::StartUIExtensionAbility(sptr<SessionInfo> extensionSessionInfo, int32_t userId)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::StartUIAbilityBySCB(sptr<SessionInfo> sessionInfo,
    AbilityRuntime::StartParamsBySCB &params, bool &isColdStart)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::StopExtensionAbility(const Want &want, sptr<IRemoteObject> callerToken,
    int32_t userId, AppExecFwk::ExtensionAbilityType extensionType)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::TerminateAbility(sptr<IRemoteObject> token, int resultCode, const Want *resultWant)
{
    return MyFlag::GetInstance()->GetTerminateAbility();
}

ErrCode AbilityManagerClient::BackToCallerAbilityWithResult(const sptr<IRemoteObject> &token, int resultCode,
    const Want *resultWant, int64_t callerRequestCode)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::TerminateUIServiceExtensionAbility(sptr<IRemoteObject> token)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::TerminateUIExtensionAbility(sptr<SessionInfo> extensionSessionInfo,
    int resultCode, const Want *resultWant)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::CloseUIExtensionAbilityBySCB(const sptr<IRemoteObject> token)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::MoveAbilityToBackground(sptr<IRemoteObject> token)
{
    return MyFlag::GetInstance()->GetMoveAbilityToBackground();
}

ErrCode AbilityManagerClient::MoveUIAbilityToBackground(const sptr<IRemoteObject> token)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::CloseAbility(sptr<IRemoteObject> token, int resultCode, const Want *resultWant)
{
    return MyFlag::GetInstance()->GetCloseAbility();
}

ErrCode AbilityManagerClient::CloseUIAbilityBySCB(sptr<SessionInfo> sessionInfo,
    bool isUserRequestedExit, uint32_t sceneFlag)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::MinimizeAbility(sptr<IRemoteObject> token, bool fromUser)
{
    return MyFlag::GetInstance()->GetMinimizeAbility();
}

ErrCode AbilityManagerClient::MinimizeUIExtensionAbility(sptr<SessionInfo> extensionSessionInfo, bool fromUser)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::MinimizeUIAbilityBySCB(sptr<SessionInfo> sessionInfo, bool fromUser, uint32_t sceneFlag)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::ConnectAbility(const Want &want, sptr<IAbilityConnection> connect, int32_t userId)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::ConnectAbility(const Want &want, sptr<IAbilityConnection> connect, int32_t userId,
    int32_t loadTimeout)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::ConnectAbility(const Want &want, sptr<IAbilityConnection> connect,
    sptr<IRemoteObject> callerToken, int32_t userId, uint64_t specifiedFullTokenId)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::ConnectUIServiceExtesnionAbility(
    const Want &want, sptr<IAbilityConnection> connect, sptr<IRemoteObject> callerToken, int32_t userId)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::ConnectDataShareExtensionAbility(const Want &want,
    sptr<IAbilityConnection> connect, int32_t userId)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::ConnectExtensionAbility(const Want &want, sptr<IAbilityConnection> connect,
    int32_t userId, int32_t loadTimeout)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::ConnectUIExtensionAbility(const Want &want, sptr<IAbilityConnection> connect,
    sptr<SessionInfo> sessionInfo, int32_t userId, sptr<UIExtensionAbilityConnectInfo> connectInfo)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::DisconnectAbility(sptr<IAbilityConnection> connect)
{
    return ERR_OK;
}

sptr<IAbilityScheduler> AbilityManagerClient::AcquireDataAbility(
    const Uri &uri, bool tryBind, sptr<IRemoteObject> callerToken)
{
    return nullptr;
}

ErrCode AbilityManagerClient::ReleaseDataAbility(
    sptr<IAbilityScheduler> dataAbilityScheduler, sptr<IRemoteObject> callerToken)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::DumpState(const std::string &args, std::vector<std::string> &state)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::DumpSysState(
    const std::string& args, std::vector<std::string>& state, bool isClient, bool isUserID, int UserID)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::Connect()
{
    return ERR_OK;
}

void AbilityManagerClient::RemoveDeathRecipient()
{}

ErrCode AbilityManagerClient::StopServiceAbility(const Want &want, sptr<IRemoteObject> token)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::KillProcess(const std::string &bundleName, bool clearPageStack, int32_t appIndex,
    const std::string& reason)
{
    return ERR_OK;
}

#ifdef ABILITY_COMMAND_FOR_TEST
ErrCode AbilityManagerClient::ForceTimeoutForTest(const std::string &abilityName, const std::string &state)
{
    return ERR_OK;
}
#endif

ErrCode AbilityManagerClient::ContinueMission(const std::string &srcDeviceId, const std::string &dstDeviceId,
    int32_t missionId, sptr<IRemoteObject> callback, AAFwk::WantParams &wantParams)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::ContinueMission(AAFwk::ContinueMissionInfo continueMissionInfo,
    const sptr<IRemoteObject> &callback)

{
    return ERR_OK;
}

ErrCode AbilityManagerClient::StartContinuation(const Want &want, sptr<IRemoteObject> abilityToken,
    int32_t status)
{
    return ERR_OK;
}

void AbilityManagerClient::NotifyCompleteContinuation(const std::string &deviceId,
    int32_t sessionId, bool isSuccess)
{}

ErrCode AbilityManagerClient::ContinueAbility(const std::string &deviceId, int32_t missionId, uint32_t versionCode)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::NotifyContinuationResult(int32_t missionId, int32_t result)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::LockMissionForCleanup(int32_t missionId)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::UnlockMissionForCleanup(int32_t missionId)
{
    return ERR_OK;
}

void AbilityManagerClient::SetLockedState(int32_t sessionId, bool lockedState)
{}

ErrCode AbilityManagerClient::RegisterMissionListener(sptr<IMissionListener> listener)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::UnRegisterMissionListener(sptr<IMissionListener> listener)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::RegisterMissionListener(const std::string &deviceId,
    sptr<IRemoteMissionListener> listener)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::RegisterOnListener(const std::string &type,
    sptr<IRemoteOnListener> listener)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::RegisterOffListener(const std::string &type,
    sptr<IRemoteOnListener> listener)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::UnRegisterMissionListener(const std::string &deviceId,
    sptr<IRemoteMissionListener> listener)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::GetMissionInfos(const std::string& deviceId, int32_t numMax,
    std::vector<MissionInfo> &missionInfos)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::GetMissionInfo(const std::string& deviceId, int32_t missionId,
    MissionInfo &missionInfo)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::CleanMission(int32_t missionId)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::CleanAllMissions()
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::MoveMissionToFront(int32_t missionId)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::MoveMissionToFront(int32_t missionId, const StartOptions &startOptions)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::MoveMissionsToForeground(const std::vector<int32_t>& missionIds, int32_t topMissionId)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::MoveMissionsToBackground(const std::vector<int32_t>& missionIds,
    std::vector<int32_t>& result)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::GetMissionIdByToken(sptr<IRemoteObject> token, int32_t &missionId)
{
    missionId = MyFlag::GetInstance()->GetMissionId();
    return MyFlag::GetInstance()->GetMissionIdByToken();
}

ErrCode AbilityManagerClient::StartAbilityByCall(const Want &want, sptr<IAbilityConnection> connect, bool isSilent,
    bool isVisible)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::StartAbilityByCall(const Want &want, sptr<IAbilityConnection> connect,
    sptr<IRemoteObject> callToken, int32_t accountId, bool isSilent, bool promotePriority, bool isVisible)
{
    return ERR_OK;
}

ErrCode StartAbilityForPrelaunch(const Want &want, const int32_t frameNum)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::StartAbilityByCallWithErrMsg(const Want &want, sptr<IAbilityConnection> connect,
    sptr<IRemoteObject> callToken, int32_t accountId, std::string &errMsg, bool isSilent, bool promotePriority,
    bool isVisible)
{
    return ERR_OK;
}

void AbilityManagerClient::CallRequestDone(sptr<IRemoteObject> token, sptr<IRemoteObject> callStub)
{}

void AbilityManagerClient::GetAbilityTokenByCalleeObj(sptr<IRemoteObject> callStub, sptr<IRemoteObject> &token)
{}

ErrCode AbilityManagerClient::ReleaseCall(
    sptr<IAbilityConnection> connect, const AppExecFwk::ElementName &element)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::GetAbilityRunningInfos(std::vector<AbilityRunningInfo> &info)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::GetExtensionRunningInfos(int upperLimit, std::vector<ExtensionRunningInfo> &info)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::GetProcessRunningInfos(std::vector<AppExecFwk::RunningProcessInfo> &info)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::GetAllIntentExemptionInfo(std::vector<AppExecFwk::IntentExemptionInfo> &info)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::RequestDialogService(
    const Want &want, sptr<IRemoteObject> callerToken)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::ReportDrawnCompleted(sptr<IRemoteObject> callerToken)
{
    return ERR_OK;
}

/**
 * Start synchronizing remote device mission
 * @param devId, deviceId.
 * @param fixConflict, resolve synchronizing conflicts flag.
 * @param tag, call tag.
 * @return Returns ERR_OK on success, others on failure.
 */
ErrCode AbilityManagerClient::StartSyncRemoteMissions(const std::string &devId, bool fixConflict, int64_t tag)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::StopSyncRemoteMissions(const std::string &devId)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::StartUser(int accountId, uint64_t displayId, sptr<IUserCallback> callback,
    bool isAppRecovery)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::StopUser(int accountId, sptr<IUserCallback> callback)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::LogoutUser(int32_t accountId, sptr<IUserCallback> callback)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::RegisterSnapshotHandler(sptr<ISnapshotHandler> handler)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::GetMissionSnapshot(const std::string& deviceId, int32_t missionId,
    MissionSnapshot& snapshot, bool isLowResolution)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::StartUserTest(const Want &want, sptr<IRemoteObject> observer)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::FinishUserTest(
    const std::string &msg, const int64_t &resultCode, const std::string &bundleName)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::GetTopAbility(sptr<IRemoteObject> &token)
{
    return ERR_OK;
}

AppExecFwk::ElementName AbilityManagerClient::GetElementNameByToken(sptr<IRemoteObject> token,
    bool isNeedLocalDeviceId)
{
    return {};
}

ErrCode AbilityManagerClient::CheckUIExtensionIsFocused(uint32_t uiExtensionTokenId, bool& isFocused,
    uint64_t displayId)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::DelegatorDoAbilityForeground(sptr<IRemoteObject> token)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::DelegatorDoAbilityBackground(sptr<IRemoteObject> token)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::SetMissionContinueState(sptr<IRemoteObject> token,
    const AAFwk::ContinueState &state, sptr<IRemoteObject> sessionToken)
{
    return ERR_OK;
}

#ifdef SUPPORT_SCREEN
ErrCode AbilityManagerClient::SetMissionLabel(sptr<IRemoteObject> token, const std::string& label)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::SetMissionIcon(
    sptr<IRemoteObject> abilityToken, std::shared_ptr<OHOS::Media::PixelMap> icon)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::RegisterWindowManagerServiceHandler(sptr<IWindowManagerServiceHandler> handler,
    bool animationEnabled)
{
    return ERR_OK;
}

void AbilityManagerClient::CompleteFirstFrameDrawing(sptr<IRemoteObject> abilityToken)
{}

void AbilityManagerClient::CompleteFirstFrameDrawing(int32_t sessionId)
{}

ErrCode AbilityManagerClient::PrepareTerminateAbility(sptr<IRemoteObject> token,
    sptr<IPrepareTerminateCallback> callback)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::GetDialogSessionInfo(const std::string &dialogSessionId, sptr<DialogSessionInfo> &info)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::SendDialogResult(const Want &want, const std::string &dialogSessionId, const bool isAllow)
{
    return ERR_OK;
}
#endif

ErrCode AbilityManagerClient::DoAbilityForeground(sptr<IRemoteObject> token, uint32_t flag)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::DoAbilityBackground(sptr<IRemoteObject> token, uint32_t flag)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::SetAbilityController(sptr<AppExecFwk::IAbilityController> abilityController,
    bool imAStabilityTest)
{
    return ERR_OK;
}
#ifdef SUPPORT_SCREEN
void AbilityManagerClient::UpdateMissionSnapShot(sptr<IRemoteObject> token,
    std::shared_ptr<Media::PixelMap> pixelMap)
{}
#endif // SUPPORT_SCREEN
void AbilityManagerClient::EnableRecoverAbility(sptr<IRemoteObject> token)
{}

void AbilityManagerClient::ScheduleRecoverAbility(sptr<IRemoteObject> token, int32_t reason, const Want *want)
{}

void AbilityManagerClient::SubmitSaveRecoveryInfo(sptr<IRemoteObject> token)
{}

void AbilityManagerClient::ScheduleClearRecoveryPageStack()
{}

sptr<IAbilityManager> AbilityManagerClient::GetAbilityManager()
{
    return proxy_;
}

void AbilityManagerClient::ResetProxy(wptr<IRemoteObject> remote)
{}

void AbilityManagerClient::AbilityMgrDeathRecipient::OnRemoteDied(const wptr<IRemoteObject>& remote)
{}

ErrCode AbilityManagerClient::FreeInstallAbilityFromRemote(const Want &want, sptr<IRemoteObject> callback,
    int32_t userId, int requestCode)
{
    return ERR_OK;
}

AppExecFwk::ElementName AbilityManagerClient::GetTopAbility(bool isNeedLocalDeviceId)
{
    return {};
}

ErrCode AbilityManagerClient::DumpAbilityInfoDone(std::vector<std::string> &infos,
    sptr<IRemoteObject> callerToken)
{
    return ERR_OK;
}

void AbilityManagerClient::HandleDlpApp(Want &want)
{}

ErrCode AbilityManagerClient::AddFreeInstallObserver(const sptr<IRemoteObject> callerToken,
    const sptr<AbilityRuntime::IFreeInstallObserver> observer)
{
    return MyFlag::GetInstance()->GetAddFreeInstallObserver();
}

int32_t AbilityManagerClient::IsValidMissionIds(
    const std::vector<int32_t> &missionIds, std::vector<MissionValidResult> &results)
{
    return 0;
}

ErrCode AbilityManagerClient::VerifyPermission(const std::string &permission, int pid, int uid)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::AcquireShareData(
    int32_t missionId, sptr<IAcquireShareDataCallback> shareData)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::ShareDataDone(
    sptr<IRemoteObject> token, int32_t resultCode, int32_t uniqueId, WantParams &wantParam)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::ForceExitApp(const int32_t pid, const ExitReason &exitReason)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::RecordAppExitReason(const ExitReason &exitReason)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::RecordProcessExitReason(const int32_t pid, const ExitReason &exitReason)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::RecordProcessExitReason(int32_t pid, int32_t uid, const ExitReason &exitReason)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::KillAppWithReason(int32_t pid, const ExitReasonCompability &exitReason)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::KillBundleWithReason(
    const std::string &bundleName, int32_t userId, int32_t appIndex, const ExitReasonCompability &exitReason)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::RecordAppWithReason(int32_t pid, int32_t uid, const ExitReasonCompability &exitReason)
{
    return ERR_OK;
}

void AbilityManagerClient::SetRootSceneSession(sptr<IRemoteObject> rootSceneSession)
{}

void AbilityManagerClient::CallUIAbilityBySCB(sptr<SessionInfo> sessionInfo, AbilityRuntime::StartParamsBySCB &params,
    bool &isColdStart)
{}

int32_t AbilityManagerClient::StartSpecifiedAbilityBySCB(const Want &want, const StartSpecifiedAbilityParams &params)
{
    return 0;
}

ErrCode AbilityManagerClient::NotifySaveAsResult(const Want &want, int resultCode, int requestCode)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::SetSessionManagerService(sptr<IRemoteObject> sessionManagerService)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::RegisterIAbilityManagerCollaborator(
    int32_t type, sptr<IAbilityManagerCollaborator> impl)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::UnregisterIAbilityManagerCollaborator(int32_t type)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::RegisterStatusBarDelegate(sptr<AbilityRuntime::IStatusBarDelegate> delegate)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::KillProcessWithPrepareTerminate(const std::vector<int32_t>& pids, bool clear)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::KillProcessWithReason(int32_t pid, const ExitReason &reason)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::RegisterAutoStartupSystemCallback(sptr<IRemoteObject> callback)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::UnregisterAutoStartupSystemCallback(sptr<IRemoteObject> callback)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::SetApplicationAutoStartup(const AutoStartupInfo &info)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::CancelApplicationAutoStartup(const AutoStartupInfo &info)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::QueryAllAutoStartupApplications(std::vector<AutoStartupInfo> &infoList)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::PrepareTerminateAbilityBySCB(sptr<SessionInfo> sessionInfo,
    bool &isPrepareTerminate)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::RegisterSessionHandler(sptr<IRemoteObject> object)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::RegisterAppDebugListener(sptr<AppExecFwk::IAppDebugListener> listener)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::UnregisterAppDebugListener(sptr<AppExecFwk::IAppDebugListener> listener)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::AttachAppDebug(const std::string &bundleName, bool isDebugFromLocal)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::DetachAppDebug(const std::string &bundleName, bool isDebugFromLocal)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::ExecuteIntent(uint64_t key, sptr<IRemoteObject> callerToken,
    const InsightIntentExecuteParam &param)
{
    return ERR_OK;
}

bool AbilityManagerClient::IsAbilityControllerStart(const Want &want)
{
    return true;
}

ErrCode AbilityManagerClient::ExecuteInsightIntentDone(sptr<IRemoteObject> token, uint64_t intentId,
    const InsightIntentExecuteResult &result)
{
    return ERR_OK;
}

int32_t AbilityManagerClient::GetForegroundUIAbilities(std::vector<AppExecFwk::AbilityStateData> &list)
{
    return 0;
}

int32_t AbilityManagerClient::OpenFile(const Uri& uri, uint32_t flag)
{
    return 0;
}

int32_t AbilityManagerClient::RequestAssertFaultDialog(
    const sptr<IRemoteObject> &callback, const AAFwk::WantParams &wantParams)
{
    return 0;
}

int32_t AbilityManagerClient::NotifyDebugAssertResult(uint64_t assertFaultSessionId, AAFwk::UserStatus userStatus)
{
    return 0;
}

int32_t AbilityManagerClient::UpdateSessionInfoBySCB(std::list<SessionInfo> &sessionInfos, int32_t userId,
    std::vector<int32_t> &sessionIds)
{
    return 0;
}

ErrCode AbilityManagerClient::GetUIExtensionRootHostInfo(const sptr<IRemoteObject> token,
    UIExtensionHostInfo &hostInfo, int32_t userId)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::GetUIExtensionSessionInfo(const sptr<IRemoteObject> token,
    UIExtensionSessionInfo &uiExtensionSessionInfo, int32_t userId)
{
    return ERR_OK;
}

int32_t AbilityManagerClient::RestartApp(const AAFwk::Want &want)
{
    return 0;
}

int32_t AbilityManagerClient::OpenAtomicService(Want& want, const StartOptions &options,
    sptr<IRemoteObject> callerToken, int32_t requestCode, int32_t userId)
{
    return MyFlag::GetInstance()->GetOpenAtomicService();
}

int32_t AbilityManagerClient::SetResidentProcessEnabled(const std::string &bundleName, bool enable)
{
    return 0;
}

bool AbilityManagerClient::IsEmbeddedOpenAllowed(sptr<IRemoteObject> callerToken, const std::string &appId)
{
    return true;
}

int32_t AbilityManagerClient::StartShortcut(const Want &want, const StartOptions &startOptions)
{
    return 0;
}

int32_t AbilityManagerClient::GetAbilityStateByPersistentId(int32_t persistentId, bool &state)
{
    return 0;
}

int32_t AbilityManagerClient::TransferAbilityResultForExtension(const sptr<IRemoteObject> &callerToken,
    int32_t resultCode, const Want &want)
{
    return 0;
}

void AbilityManagerClient::NotifyFrozenProcessByRSS(const std::vector<int32_t> &pidList, int32_t uid)
{}

ErrCode AbilityManagerClient::CleanUIAbilityBySCB(sptr<SessionInfo> sessionInfo,
    bool isUserRequestedExit, uint32_t sceneFlag)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::PreStartMission(const std::string& bundleName, const std::string& moduleName,
    const std::string& abilityName, const std::string& startTime)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::OpenLink(const Want& want, sptr<IRemoteObject> callerToken,
    int32_t userId, int requestCode, bool hideFailureTipDialog)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::TerminateMission(int32_t missionId)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::BlockAllAppStart(bool flag)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::UpdateAssociateConfigList(const std::map<std::string, std::list<std::string>>& configs,
    const std::list<std::string>& exportConfigs, int32_t flag)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::AddQueryERMSObserver(sptr<IRemoteObject> callerToken,
    sptr<AbilityRuntime::IQueryERMSObserver> observer)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::QueryAtomicServiceStartupRule(sptr<IRemoteObject> callerToken,
    const std::string &appId, const std::string &startTime, AtomicServiceStartupRule &rule)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::StartSelfUIAbility(const Want &want)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::StartSelfUIAbilityWithStartOptions(const Want &want,
    const StartOptions &options)
{
    return ERR_OK;
}

int AbilityManagerClient::StartSelfUIAbilityWithPidResult(const Want &want, StartOptions &options,
    uint64_t callbackId)
{
    return ERR_OK;
}

void AbilityManagerClient::PrepareTerminateAbilityDone(sptr<IRemoteObject> token, bool isTerminate)
{}

void AbilityManagerClient::KillProcessWithPrepareTerminateDone(const std::string &moduleName,
    int32_t prepareTermination, bool isExist)
{}

ErrCode AbilityManagerClient::KillProcessForPermissionUpdate(uint32_t accessTokenId)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::RegisterHiddenStartObserver(const sptr<IHiddenStartObserver> &observer)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::UnregisterHiddenStartObserver(const sptr<IHiddenStartObserver> &observer)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::QueryPreLoadUIExtensionRecord(const AppExecFwk::ElementName &element,
                                                            const std::string &moduleName,
                                                            const int32_t hostPid,
                                                            int32_t &recordNum,
                                                            int32_t userId)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::RevokeDelegator(sptr<IRemoteObject> token)
{
    return ERR_OK;
}

ErrCode GetUserLockedBundleList(int32_t userId, std::unordered_set<std::string> &userLockedBundleList)
{
    return ERR_OK;
}
} // namespace AAFwk
} // namespace OHOS
