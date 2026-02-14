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

#ifndef OHOS_ABILITY_RUNTIME_ABILITY_MANAGER_STUB_H
#define OHOS_ABILITY_RUNTIME_ABILITY_MANAGER_STUB_H

#include "ability_manager_interface.h"

#include <iremote_object.h>
#include <iremote_stub.h>
#ifdef WITH_DLP
#include "dlp_connection_info.h"
#endif // WITH_DLP
#include "iconnection_observer.h"
#include "iforeground_app_connection.h"

namespace OHOS {
namespace AAFwk {
using AutoStartupInfo = AbilityRuntime::AutoStartupInfo;
/**
 * @class AbilityManagerStub
 * AbilityManagerStub.
 */
class AbilityManagerStub : public IRemoteStub<IAbilityManager> {
public:
    AbilityManagerStub();
    ~AbilityManagerStub();
    virtual int OnRemoteRequest(
        uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;

    /**
     * Calls this interface to move the ability to the foreground.
     *
     * @param token, ability's token.
     * @param flag, use for lock or unlock flag and so on.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int DoAbilityForeground(const sptr<IRemoteObject> &token, uint32_t flag) override;

    /**
     * Calls this interface to move the ability to the background.
     *
     * @param token, ability's token.
     * @param flag, use for lock or unlock flag and so on.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int DoAbilityBackground(const sptr<IRemoteObject> &token, uint32_t flag) override;

    virtual int RegisterObserver(const sptr<AbilityRuntime::IConnectionObserver> &observer);

    virtual int UnregisterObserver(const sptr<AbilityRuntime::IConnectionObserver> &observer);

    virtual int RegisterForegroundAppObserver(sptr<AbilityRuntime::IForegroundAppConnection> observer);

    virtual int UnregisterForegroundAppObserver(sptr<AbilityRuntime::IForegroundAppConnection> observer);

#ifdef WITH_DLP
    virtual int GetDlpConnectionInfos(std::vector<AbilityRuntime::DlpConnectionInfo> &infos);
#endif // WITH_DLP

    virtual int GetConnectionData(std::vector<AbilityRuntime::ConnectionData> &connectionData);

    virtual void CancelWantSenderByFlags(const sptr<IWantSender> &sender, uint32_t flags);

private:
    int TerminateAbilityInner(MessageParcel &data, MessageParcel &reply);
    int BackToCallerInner(MessageParcel &data, MessageParcel &reply);
    int32_t TerminateUIServiceExtensionAbilityInner(MessageParcel &data, MessageParcel &reply);
    int TerminateUIExtensionAbilityInner(MessageParcel &data, MessageParcel &reply);
    int CloseUIExtensionAbilityBySCBInner(MessageParcel &data, MessageParcel &reply);
    int CloseUIAbilityBySCBInner(MessageParcel &data, MessageParcel &reply);
    int SendResultToAbilityInner(MessageParcel &data, MessageParcel &reply);
    int MinimizeAbilityInner(MessageParcel &data, MessageParcel &reply);
    int MinimizeUIExtensionAbilityInner(MessageParcel &data, MessageParcel &reply);
    int MinimizeUIAbilityBySCBInner(MessageParcel &data, MessageParcel &reply);
    int AttachAbilityThreadInner(MessageParcel &data, MessageParcel &reply);
    int NotifyAttachAbilityThreadDoneInner(MessageParcel &data, MessageParcel &reply);
    int AbilityTransitionDoneInner(MessageParcel &data, MessageParcel &reply);
    int AbilityWindowConfigTransitionDoneInner(MessageParcel &data, MessageParcel &reply);
    int ScheduleConnectAbilityDoneInner(MessageParcel &data, MessageParcel &reply);
    int ScheduleDisconnectAbilityDoneInner(MessageParcel &data, MessageParcel &reply);
    int ScheduleCommandAbilityDoneInner(MessageParcel &data, MessageParcel &reply);
    int ScheduleCommandAbilityWindowDoneInner(MessageParcel &data, MessageParcel &reply);
    int GetMissionSnapshotInner(MessageParcel &data, MessageParcel &reply);
    int AcquireDataAbilityInner(MessageParcel &data, MessageParcel &reply);
    int ReleaseDataAbilityInner(MessageParcel &data, MessageParcel &reply);
    int KillProcessInner(MessageParcel &data, MessageParcel &reply);
    int UninstallAppInner(MessageParcel &data, MessageParcel &reply);
    int32_t UpgradeAppInner(MessageParcel &data, MessageParcel &reply);
    int StartSelfUIAbilityInner(MessageParcel &data, MessageParcel &reply);
    int StartSelfUIAbilityWithStartOptionsInner(MessageParcel &data, MessageParcel &reply);
    int StartSelfUIAbilityWithPidResultInner(MessageParcel &data, MessageParcel &reply);
    int StartAbilityInner(MessageParcel &data, MessageParcel &reply);
    int StartAbilityInnerSpecifyTokenId(MessageParcel &data, MessageParcel &reply);
    int StartAbilityByUIContentSessionAddCallerInner(MessageParcel &data, MessageParcel &reply);
    int StartAbilityByUIContentSessionForOptionsInner(MessageParcel &data, MessageParcel &reply);
    int StartAbilityAsCallerByTokenInner(MessageParcel &data, MessageParcel &reply);
    int StartAbilityAsCallerForOptionInner(MessageParcel &data, MessageParcel &reply);
    int StartExtensionAbilityInner(MessageParcel &data, MessageParcel &reply);
    int StartUIExtensionAbilityInner(MessageParcel &data, MessageParcel &reply);
    int StartUIExtensionAbilityEmbeddedInner(MessageParcel &data, MessageParcel &reply);
    int StartUIExtensionConstrainedEmbeddedInner(MessageParcel &data, MessageParcel &reply);
    int StartUIExtensionPreViewEmbeddedInner(MessageParcel &data, MessageParcel &reply);
    int StartUIAbilityBySCBInner(MessageParcel &data, MessageParcel &reply);
    int StopExtensionAbilityInner(MessageParcel& data, MessageParcel& reply);
    int StartAbilityAddCallerInner(MessageParcel &data, MessageParcel &reply);
    int ConnectAbilityInner(MessageParcel &data, MessageParcel &reply);
    int ConnectAbilityWithTypeInner(MessageParcel &data, MessageParcel &reply);
    int ConnectUIExtensionAbilityInner(MessageParcel &data, MessageParcel &reply);
    int DisconnectAbilityInner(MessageParcel &data, MessageParcel &reply);
    int StopServiceAbilityInner(MessageParcel &data, MessageParcel &reply);
    int DumpStateInner(MessageParcel &data, MessageParcel &reply);
    int DumpSysStateInner(MessageParcel &data, MessageParcel &reply);
    int StartAbilityForSettingsInner(MessageParcel &data, MessageParcel &reply);
    int StartAbilityForOptionsInner(MessageParcel &data, MessageParcel &reply);
    int RequestModalUIExtensionInner(MessageParcel &data, MessageParcel &reply);
    int ChangeAbilityVisibilityInner(MessageParcel &data, MessageParcel &reply);
    int ChangeUIAbilityVisibilityBySCBInner(MessageParcel &data, MessageParcel &reply);
    int PreloadUIExtensionAbilityInner(MessageParcel &data, MessageParcel &reply);

    int GetWantSenderInner(MessageParcel &data, MessageParcel &reply);
    int SendWantSenderInner(MessageParcel &data, MessageParcel &reply);
    int SendLocalWantSenderInner(MessageParcel &data, MessageParcel &reply);
    int CancelWantSenderInner(MessageParcel &data, MessageParcel &reply);

    int GetPendingWantUidInner(MessageParcel &data, MessageParcel &reply);
    int GetPendingWantUserIdInner(MessageParcel &data, MessageParcel &reply);
    int GetPendingWantBundleNameInner(MessageParcel &data, MessageParcel &reply);
    int GetPendingWantCodeInner(MessageParcel &data, MessageParcel &reply);
    int GetPendingWantTypeInner(MessageParcel &data, MessageParcel &reply);

    int RegisterCancelListenerInner(MessageParcel &data, MessageParcel &reply);
    int UnregisterCancelListenerInner(MessageParcel &data, MessageParcel &reply);

    int GetPendingRequestWantInner(MessageParcel &data, MessageParcel &reply);
    int GetWantSenderInfoInner(MessageParcel &data, MessageParcel &reply);

    int GetAppMemorySizeInner(MessageParcel &data, MessageParcel &reply);
    int IsRamConstrainedDeviceInner(MessageParcel &data, MessageParcel &reply);

    int ContinueMissionInner(MessageParcel &data, MessageParcel &reply);
    int ContinueMissionOfBundleNameInner(MessageParcel &data, MessageParcel &reply);
    int ContinueAbilityInner(MessageParcel &data, MessageParcel &reply);
    int StartContinuationInner(MessageParcel &data, MessageParcel &reply);
    int NotifyCompleteContinuationInner(MessageParcel &data, MessageParcel &reply);
    int NotifyContinuationResultInner(MessageParcel &data, MessageParcel &reply);
    int RegisterRemoteMissionListenerInner(MessageParcel &data, MessageParcel &reply);
    int RegisterRemoteOnListenerInner(MessageParcel &data, MessageParcel &reply);
    int RegisterRemoteOffListenerInner(MessageParcel &data, MessageParcel &reply);
    int UnRegisterRemoteMissionListenerInner(MessageParcel &data, MessageParcel &reply);

    int LockMissionForCleanupInner(MessageParcel &data, MessageParcel &reply);
    int UnlockMissionForCleanupInner(MessageParcel &data, MessageParcel &reply);
    int SetLockedStateInner(MessageParcel &data, MessageParcel &reply);
    int RegisterMissionListenerInner(MessageParcel &data, MessageParcel &reply);
    int UnRegisterMissionListenerInner(MessageParcel &data, MessageParcel &reply);
    int GetMissionInfosInner(MessageParcel &data, MessageParcel &reply);
    int GetMissionInfoInner(MessageParcel &data, MessageParcel &reply);
    int CleanMissionInner(MessageParcel &data, MessageParcel &reply);
    int CleanAllMissionsInner(MessageParcel &data, MessageParcel &reply);
    int MoveMissionToFrontInner(MessageParcel &data, MessageParcel &reply);
    int MoveMissionsToForegroundInner(MessageParcel &data, MessageParcel &reply);
    int MoveMissionsToBackgroundInner(MessageParcel &data, MessageParcel &reply);
    int GetMissionIdByTokenInner(MessageParcel &data, MessageParcel &reply);

    // for new version ability (call ability)
    int StartAbilityByCallInner(MessageParcel &data, MessageParcel &reply);
    int StartAbilityForPrelaunchInner(MessageParcel &data, MessageParcel &reply);
    int CallRequestDoneInner(MessageParcel &data, MessageParcel &reply);
    int ReleaseCallInner(MessageParcel &data, MessageParcel &reply);
    int StartUserInner(MessageParcel &data, MessageParcel &reply);
    int StopUserInner(MessageParcel &data, MessageParcel &reply);
    int LogoutUserInner(MessageParcel &data, MessageParcel &reply);
    int GetAbilityRunningInfosInner(MessageParcel &data, MessageParcel &reply);
    int GetExtensionRunningInfosInner(MessageParcel &data, MessageParcel &reply);
    int GetProcessRunningInfosInner(MessageParcel &data, MessageParcel &reply);
    int GetAllIntentExemptionInfoInner(MessageParcel &data, MessageParcel &reply);

    int StartSyncRemoteMissionsInner(MessageParcel &data, MessageParcel &reply);
    int StopSyncRemoteMissionsInner(MessageParcel &data, MessageParcel &reply);
    int RegisterSnapshotHandlerInner(MessageParcel &data, MessageParcel &reply);
    int GetMissionSnapshotInfoInner(MessageParcel &data, MessageParcel &reply);

    int SetAbilityControllerInner(MessageParcel &data, MessageParcel &reply);

    int StartUserTestInner(MessageParcel &data, MessageParcel &reply);
    int FinishUserTestInner(MessageParcel &data, MessageParcel &reply);
    int GetTopAbilityTokenInner(MessageParcel &data, MessageParcel &reply);
    int CheckUIExtensionIsFocusedInner(MessageParcel &data, MessageParcel &reply);
    int DelegatorDoAbilityForegroundInner(MessageParcel &data, MessageParcel &reply);
    int DelegatorDoAbilityBackgroundInner(MessageParcel &data, MessageParcel &reply);
    int DoAbilityForegroundInner(MessageParcel &data, MessageParcel &reply);
    int DoAbilityBackgroundInner(MessageParcel &data, MessageParcel &reply);

    int IsRunningInStabilityTestInner(MessageParcel &data, MessageParcel &reply);
    int MoveMissionToFrontByOptionsInner(MessageParcel &data, MessageParcel &reply);

    int UpdateMissionSnapShotFromWMSInner(MessageParcel &data, MessageParcel &reply);
    int RegisterConnectionObserverInner(MessageParcel &data, MessageParcel &reply);
    int UnregisterConnectionObserverInner(MessageParcel &data, MessageParcel &reply);
#ifdef WITH_DLP
    int GetDlpConnectionInfosInner(MessageParcel &data, MessageParcel &reply);
#endif // WITH_DLP
    int GetConnectionDataInner(MessageParcel &data, MessageParcel &reply);
    int MoveAbilityToBackgroundInner(MessageParcel &data, MessageParcel &reply);
    int32_t MoveUIAbilityToBackgroundInner(MessageParcel &data, MessageParcel &reply);
    int SetMissionContinueStateInner(MessageParcel &data, MessageParcel &reply);
#ifdef SUPPORT_GRAPHICS
    int SetMissionLabelInner(MessageParcel &data, MessageParcel &reply);
    int SetMissionIconInner(MessageParcel &data, MessageParcel &reply);
    int RegisterWindowManagerServiceHandlerInner(MessageParcel &data, MessageParcel &reply);
    int CompleteFirstFrameDrawingInner(MessageParcel &data, MessageParcel &reply);
    int PrepareTerminateAbilityInner(MessageParcel &data, MessageParcel &reply);
    int GetDialogSessionInfoInner(MessageParcel &data, MessageParcel &reply);
    int SendDialogResultInner(MessageParcel &data, MessageParcel &reply);
    int RegisterAbilityFirstFrameStateObserverInner(MessageParcel &data, MessageParcel &reply);
    int UnregisterAbilityFirstFrameStateObserverInner(MessageParcel &data, MessageParcel &reply);
    int CompleteFirstFrameDrawingBySCBInner(MessageParcel &data, MessageParcel &reply);
#endif

    #ifdef ABILITY_COMMAND_FOR_TEST
    int ForceTimeoutForTestInner(MessageParcel &data, MessageParcel &reply);
    #endif

    int FreeInstallAbilityFromRemoteInner(MessageParcel &data, MessageParcel &reply);
    int AddFreeInstallObserverInner(MessageParcel &data, MessageParcel &reply);

    int32_t RegisterForegroundAppObserverInner(MessageParcel &data, MessageParcel &reply);
    int32_t UnregisterForegroundAppObserverInner(MessageParcel &data, MessageParcel &reply);

    int EnableRecoverAbilityInner(MessageParcel &data, MessageParcel &reply);
    int SubmitSaveRecoveryInfoInner(MessageParcel &data, MessageParcel &reply);
    int ScheduleRecoverAbilityInner(MessageParcel &data, MessageParcel &reply);
    int ScheduleClearRecoveryPageStackInner(MessageParcel &data, MessageParcel &reply);
    int GetTopAbilityInner(MessageParcel &data, MessageParcel &reply);
    int GetElementNameByTokenInner(MessageParcel &data, MessageParcel &reply);
    int DumpAbilityInfoDoneInner(MessageParcel &data, MessageParcel &reply);
    int32_t IsValidMissionIdsInner(MessageParcel &data, MessageParcel &reply);

    int VerifyPermissionInner(MessageParcel &data, MessageParcel &reply);

    int HandleRequestDialogService(MessageParcel &data, MessageParcel &reply);
    int32_t HandleReportDrawnCompleted(MessageParcel &data, MessageParcel &reply);

    int AcquireShareDataInner(MessageParcel &data, MessageParcel &reply);
    int ShareDataDoneInner(MessageParcel &data, MessageParcel &reply);
    int GetAbilityTokenByCalleeObjInner(MessageParcel &data, MessageParcel &reply);

    int32_t ForceExitAppInner(MessageParcel &data, MessageParcel &reply);
    int32_t RecordAppExitReasonInner(MessageParcel &data, MessageParcel &reply);
    int32_t RecordProcessExitReasonInner(MessageParcel &data, MessageParcel &reply);
    int32_t RecordProcessExitReasonPlusInner(MessageParcel &data, MessageParcel &reply);
    int32_t SetResidentProcessEnableInner(MessageParcel &data, MessageParcel &reply);

    int SetRootSceneSessionInner(MessageParcel &data, MessageParcel &reply);
    int CallUIAbilityBySCBInner(MessageParcel &data, MessageParcel &reply);
    int StartSpecifiedAbilityBySCBInner(MessageParcel &data, MessageParcel &reply);
    int NotifySaveAsResultInner(MessageParcel &data, MessageParcel &reply);

    int SetSessionManagerServiceInner(MessageParcel &data, MessageParcel &reply);

    int32_t RegisterIAbilityManagerCollaboratorInner(MessageParcel &data, MessageParcel &reply);
    int32_t UnregisterIAbilityManagerCollaboratorInner(MessageParcel &data, MessageParcel &reply);
    int32_t GetAbilityManagerCollaboratorInner(MessageParcel &data, MessageParcel &reply);

    int32_t RegisterStatusBarDelegateInner(MessageParcel &data, MessageParcel &reply);
    int32_t KillProcessWithPrepareTerminateInner(MessageParcel &data, MessageParcel &reply);

    int32_t KillProcessWithReasonInner(MessageParcel &data, MessageParcel &reply);

    int32_t RegisterAutoStartupSystemCallbackInner(MessageParcel &data, MessageParcel &reply);
    int32_t UnregisterAutoStartupSystemCallbackInner(MessageParcel &data, MessageParcel &reply);
    int32_t SetApplicationAutoStartupInner(MessageParcel &data, MessageParcel &reply);
    int32_t CancelApplicationAutoStartupInner(MessageParcel &data, MessageParcel &reply);
    int32_t QueryAllAutoStartupApplicationsInner(MessageParcel &data, MessageParcel &reply);
    int32_t GetAutoStartupStatusForSelfInner(MessageParcel &data, MessageParcel &reply);
    int32_t ManualStartAutoStartupAppsInner(MessageParcel &data, MessageParcel &reply);

    int PrepareTerminateAbilityBySCBInner(MessageParcel &data, MessageParcel &reply);
    int RegisterSessionHandlerInner(MessageParcel &data, MessageParcel &reply);
    int32_t UpdateSessionInfoBySCBInner(MessageParcel &data, MessageParcel &reply);

    int32_t RegisterAppDebugListenerInner(MessageParcel &data, MessageParcel &reply);
    int32_t UnregisterAppDebugListenerInner(MessageParcel &data, MessageParcel &reply);
    int32_t AttachAppDebugInner(MessageParcel &data, MessageParcel &reply);
    int32_t DetachAppDebugInner(MessageParcel &data, MessageParcel &reply);
    int32_t ExecuteIntentInner(MessageParcel &data, MessageParcel &reply);

    int32_t SetApplicationAutoStartupByEDMInner(MessageParcel &data, MessageParcel &reply);
    int32_t CancelApplicationAutoStartupByEDMInner(MessageParcel &data, MessageParcel &reply);

    int32_t IsAbilityControllerStartInner(MessageParcel &data, MessageParcel &reply);
    int32_t OpenFileInner(MessageParcel &data, MessageParcel &reply);

    int32_t OpenAtomicServiceInner(MessageParcel &data, MessageParcel &reply);
    int32_t IsEmbeddedOpenAllowedInner(MessageParcel &data, MessageParcel &reply);

    int StartAbilityForResultAsCallerInner(MessageParcel &data, MessageParcel &reply);
    int StartAbilityForResultAsCallerForOptionsInner(MessageParcel &data, MessageParcel &reply);
    int32_t StartUIAbilitiesInSplitWindowModeInner(MessageParcel &data, MessageParcel &reply);
    int32_t StartUIAbilitiesInner(MessageParcel &data, MessageParcel &reply);

    int32_t StartAbilityOnlyUIAbilityInner(MessageParcel &data, MessageParcel &reply);

    //insight intent related
    int32_t StartAbilityByInsightIntentInner(MessageParcel &data, MessageParcel &reply);
    int32_t ExecuteInsightIntentDoneInner(MessageParcel &data, MessageParcel &reply);
    int32_t GetForegroundUIAbilitiesInner(MessageParcel &data, MessageParcel &reply);
    int32_t RestartAppInner(MessageParcel &data, MessageParcel &reply);

    int32_t GetUIExtensionRootHostInfoInner(MessageParcel &data, MessageParcel &reply);
    int32_t GetUIExtensionSessionInfoInner(MessageParcel &data, MessageParcel &reply);
    int32_t RequestAssertFaultDialogInner(MessageParcel &data, MessageParcel &reply);
    int32_t NotifyDebugAssertResultInner(MessageParcel &data, MessageParcel &reply);
    int32_t StartShortcutInner(MessageParcel &data, MessageParcel &reply);
    int32_t GetAbilityStateByPersistentIdInner(MessageParcel &data, MessageParcel &reply);
    int32_t TransferAbilityResultForExtensionInner(MessageParcel &data, MessageParcel &reply);
    int32_t NotifyFrozenProcessByRSSInner(MessageParcel &data, MessageParcel &reply);
    int32_t CleanUIAbilityBySCBInner(MessageParcel &data, MessageParcel &reply);
    int32_t PreStartMissionInner(MessageParcel &data, MessageParcel &reply);
    int32_t SetApplicationKeepAliveInner(MessageParcel &data, MessageParcel &reply);
    int32_t QueryKeepAliveApplicationsInner(MessageParcel &data, MessageParcel &reply);
    int32_t SetApplicationKeepAliveByEDMInner(MessageParcel &data, MessageParcel &reply);
    int32_t QueryKeepAliveApplicationsByEDMInner(MessageParcel &data, MessageParcel &reply);
    int32_t SetAppServiceExtensionKeepAliveInner(MessageParcel &data, MessageParcel &reply);
    int32_t QueryKeepAliveAppServiceExtensionsInner(MessageParcel &data, MessageParcel &reply);
    int32_t AddQueryERMSObserverInner(MessageParcel &data, MessageParcel &reply);
    int32_t QueryAtomicServiceStartupRuleInner(MessageParcel &data, MessageParcel &reply);
    int32_t PrepareTerminateAbilityDoneInner(MessageParcel &data, MessageParcel &reply);
    int32_t KillProcessWithPrepareTerminateDoneInner(MessageParcel &data, MessageParcel &reply);
    int32_t KillProcessForPermissionUpdateInner(MessageParcel &data, MessageParcel &reply);
    int32_t RegisterHiddenStartObserverInner(MessageParcel &data, MessageParcel &reply);
    int32_t UnregisterHiddenStartObserverInner(MessageParcel &data, MessageParcel &reply);
    int32_t QueryPreLoadUIExtensionRecordInner(MessageParcel &data, MessageParcel &reply);
    int32_t RevokeDelegatorInner(MessageParcel &data, MessageParcel &reply);
    int32_t GetAllInsightIntentInfoInner(MessageParcel &data, MessageParcel &reply);
    int32_t GetInsightIntentInfoByBundleNameInner(MessageParcel &data, MessageParcel &reply);
    int32_t GetInsightIntentInfoByIntentNameInner(MessageParcel &data, MessageParcel &reply);
    int32_t StartAbilityWithWaitInner(MessageParcel &data, MessageParcel &reply);
    int32_t RestartSelfAtomicServiceInner(MessageParcel &data, MessageParcel &reply);
    int32_t RegisterSAInterceptorInner(MessageParcel &data, MessageParcel &reply);
    int32_t SuspendExtensionAbilityInner(MessageParcel &data, MessageParcel &reply);
    int32_t ResumeExtensionAbilityInner(MessageParcel &data, MessageParcel &reply);
    int32_t SetOnNewWantSkipScenariosInner(MessageParcel &data, MessageParcel &reply);
    int32_t NotifyStartupExceptionBySCBInner(MessageParcel &data, MessageParcel &reply);
    int32_t PreloadApplicationInner(MessageParcel &data, MessageParcel &reply);
    int32_t UnPreloadUIExtensionAbilityInner(MessageParcel &data, MessageParcel &reply);
    int32_t ClearAllPreloadUIExtensionAbilityInner(MessageParcel &data, MessageParcel &reply);
    int32_t RegisterPreloadUIExtensionHostClientInner(MessageParcel &data, MessageParcel &reply);
    int32_t UnRegisterPreloadUIExtensionHostClientInner(MessageParcel &data, MessageParcel &reply);

    int OnRemoteRequestInnerFirst(uint32_t code, MessageParcel &data,
        MessageParcel &reply, MessageOption &option);
    int OnRemoteRequestInnerSecond(uint32_t code, MessageParcel &data,
        MessageParcel &reply, MessageOption &option);
    int OnRemoteRequestInnerThird(uint32_t code, MessageParcel &data,
        MessageParcel &reply, MessageOption &option);
    int OnRemoteRequestInnerFourth(uint32_t code, MessageParcel &data,
        MessageParcel &reply, MessageOption &option);
    int OnRemoteRequestInnerFifth(uint32_t code, MessageParcel &data,
        MessageParcel &reply, MessageOption &option);
    int OnRemoteRequestInnerSixth(uint32_t code, MessageParcel &data,
        MessageParcel &reply, MessageOption &option);
    int OnRemoteRequestInnerSeventh(uint32_t code, MessageParcel &data,
        MessageParcel &reply, MessageOption &option);
    int OnRemoteRequestInnerEighth(uint32_t code, MessageParcel &data,
        MessageParcel &reply, MessageOption &option);
    int OnRemoteRequestInnerNinth(uint32_t code, MessageParcel &data,
        MessageParcel &reply, MessageOption &option);
    int OnRemoteRequestInnerTenth(uint32_t code, MessageParcel &data,
        MessageParcel &reply, MessageOption &option);
    int OnRemoteRequestInnerEleventh(uint32_t code, MessageParcel &data,
        MessageParcel &reply, MessageOption &option);
    int OnRemoteRequestInner(uint32_t code, MessageParcel &data,
        MessageParcel &reply, MessageOption &option);
    int OnRemoteRequestInnerTwelveth(uint32_t code, MessageParcel &data,
        MessageParcel &reply, MessageOption &option);
    int OnRemoteRequestInnerThirteenth(uint32_t code, MessageParcel &data,
        MessageParcel &reply, MessageOption &option);
    int OnRemoteRequestInnerFourteenth(uint32_t code, MessageParcel &data,
        MessageParcel &reply, MessageOption &option);
    int OnRemoteRequestInnerFifteenth(uint32_t code, MessageParcel &data,
        MessageParcel &reply, MessageOption &option);
    int OnRemoteRequestInnerSixteenth(uint32_t code, MessageParcel &data,
        MessageParcel &reply, MessageOption &option);
    int OnRemoteRequestInnerSeventeenth(uint32_t code, MessageParcel &data,
        MessageParcel &reply, MessageOption &option);
    int OnRemoteRequestInnerEighteenth(uint32_t code, MessageParcel &data,
        MessageParcel &reply, MessageOption &option);
    int OnRemoteRequestInnerNineteenth(uint32_t code, MessageParcel &data,
        MessageParcel &reply, MessageOption &option);
    int OnRemoteRequestInnerTwentieth(uint32_t code, MessageParcel &data,
        MessageParcel &reply, MessageOption &option);
    int OnRemoteRequestInnerTwentyFirst(uint32_t code, MessageParcel &data,
        MessageParcel &reply, MessageOption &option);
    int OnRemoteRequestInnerTwentySecond(uint32_t code, MessageParcel &data,
        MessageParcel &reply, MessageOption &option);
    int HandleOnRemoteRequestInnerFirst(uint32_t code, MessageParcel &data,
        MessageParcel &reply, MessageOption &option);
    int HandleOnRemoteRequestInnerSecond(uint32_t code, MessageParcel &data,
        MessageParcel &reply, MessageOption &option);
    int HandleOnRemoteRequestInnerThird(uint32_t code, MessageParcel &data,
        MessageParcel &reply, MessageOption &option);
    int32_t OpenLinkInner(MessageParcel &data, MessageParcel &reply);
    int32_t TerminateMissionInner(MessageParcel &data, MessageParcel &reply);
    int32_t BlockAllAppStartInner(MessageParcel &data, MessageParcel &reply);
    int32_t UpdateAssociateConfigListInner(MessageParcel &data, MessageParcel &reply);
    int32_t UpdateKioskApplicationListInner(MessageParcel &data, MessageParcel &reply);
    int32_t EnterKioskModeInner(MessageParcel &data, MessageParcel &reply);
    int32_t ExitKioskModeInner(MessageParcel &data, MessageParcel &reply);
    int32_t GetKioskStatusInner(MessageParcel &data, MessageParcel &reply);
    int32_t StartSelfUIAbilityInCurrentProcessInner(MessageParcel &data, MessageParcel &reply);
    int32_t IsRestartAppLimitInner(MessageParcel &data, MessageParcel &reply);
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_ABILITY_MANAGER_STUB_H
