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

#ifndef OHOS_ABILITY_RUNTIME_UI_ABILITY_LIFECYCLE_MANAGER_H
#define OHOS_ABILITY_RUNTIME_UI_ABILITY_LIFECYCLE_MANAGER_H

#include <list>
#include <map>
#include <memory>
#include <queue>
#include <unordered_map>
#include "cpp/mutex.h"

#include "ability_manager_constants.h"
#include "isession_handler_interface.h"
#include "scene_board/ui_ability_record.h"

namespace OHOS {
namespace AAFwk {
class SessionInfo;
class StatusBarDelegateManager;
struct AbilityRunningInfo;
struct MissionValidResult;

enum class SpecifiedProcessState: u_int8_t {
    STATE_NONE = 0,
    STATE_PROCESS = 1,
    STATE_ABILITY = 2
};

struct SpecifiedRequest {
    bool preCreateProcessName = false;
    bool isCold = false;
    SpecifiedProcessState specifiedProcessState = SpecifiedProcessState::STATE_NONE;
    int32_t requestId = 0;
    int32_t persistentId = 0;
    uint32_t sceneFlag = 0;
    uint32_t callingTokenId = 0;
    AbilityRequest abilityRequest;

    SpecifiedRequest(int32_t requestId, AbilityRequest request) : requestId(requestId), abilityRequest(request) {}
};

class UIAbilityLifecycleManager : public std::enable_shared_from_this<UIAbilityLifecycleManager> {
public:
    UIAbilityLifecycleManager() = default;
    explicit UIAbilityLifecycleManager(int32_t userId);
    virtual ~UIAbilityLifecycleManager() = default;

    void SignRestartAppFlag(int32_t uid, const std::string &instanceKey, bool isAppRecovery = false);

    /**
     * StartUIAbility with request.
     *
     * @param abilityRequest the request of the service ability to start.
     * @param sessionInfo the info of scene session
     * @param params start parameters.
     * @param isColdStart the session info of the ability is or not cold start.
     * @return Returns ERR_OK on success, others on failure.
     */
    int StartUIAbility(AbilityRequest &abilityRequest, sptr<SessionInfo> sessionInfo,
        AbilityRuntime::StartParamsBySCB &params, bool &isColdStart);

    /**
     * @brief execute after the ability schedule the lifecycle
     *
     * @param token the ability token
     * @param state the ability state
     * @param saveData the saved data
     * @return execute error code
     */
    int AbilityTransactionDone(const sptr<IRemoteObject> &token, int state, const AppExecFwk::PacMap &saveData);

    /**
     * @brief execute after the ability schedule the lifecycle
     *
     * @param token the ability token
     * @param windowConfig the windowconfig
     * @return execute error code
     */
    int AbilityWindowConfigTransactionDone(
        const sptr<IRemoteObject> &token, const WindowConfig &windowConfig);

    /**
     * attach ability thread ipc object.
     *
     * @param scheduler ability thread ipc object.
     * @param token the token of ability.
     * @return Returns ERR_OK on success, others on failure.
     */
    int AttachAbilityThread(const sptr<IAbilityScheduler> &scheduler, const sptr<IRemoteObject> &token);

    /**
     * app manager service call this interface after ability request done.
     *
     * @param token ability's token.
     * @param state the state of ability lift cycle.
     */
    void OnAbilityRequestDone(const sptr<IRemoteObject> &token, int32_t state);

    /**
     * Check whether the UIAbility is alive.
     *
     * @param token ability's token.
     * @return Returns true on alive.
     */
    bool IsContainsAbility(const sptr<IRemoteObject> &token) const;

    /**
     * Notify SCB to minimize UIAbility
     *
     * @param token ability's token
     */
    int32_t NotifySCBToMinimizeUIAbility(const sptr<IRemoteObject> token);

    /**
     * MinimizeUIAbility, minimize the special ability by scb.
     *
     * @param abilityRecord, the ability to minimize.
     * @param fromUser, Whether form user.
     * @return Returns ERR_OK on success, others on failure.
     */
    int MinimizeUIAbility(const UIAbilityRecordPtr &abilityRecord, bool fromUser, uint32_t sceneFlag);

    /**
     * GetUIAbilityRecordBySessionInfo.
     *
     * @param sessionToken, service ability's session token.
     * @return Returns UIAbilityRecord shared_ptr.
     */
    UIAbilityRecordPtr GetUIAbilityRecordBySessionInfo(const sptr<SessionInfo> &sessionInfo);

    int32_t BackToCallerAbilityWithResult(std::shared_ptr<AbilityRecord> abilityRecord,
        int resultCode, const Want *resultWant, int64_t callerRequestCode);

    /**
     * CloseUIAbility, close the special ability by scb.
     *
     * @param abilityRecord, the ability to close.
     * @param resultCode, the resultCode of the ability to terminate.
     * @param resultWant, the Want of the ability to return.
     * @param isClearSession Indicates whether to close UIAbility because the session is cleared.
     * @return Returns ERR_OK on success, others on failure.
     */
    int CloseUIAbility(const UIAbilityRecordPtr &abilityRecord,
        int resultCode, const Want *resultWant, bool isClearSession, bool isIndependentRecovery);

    /**
     * Set rootSceneSession by SCB.
     *
     * @param rootSceneSession Indicates root scene session of SCB.
     */
    void SetRootSceneSession(const sptr<IRemoteObject> &rootSceneSession);

    int NotifySCBToStartUIAbility(AbilityRequest &abilityRequest);
    int32_t NotifySCBToStartUIAbilities(std::vector<AbilityRequest> &abilityRequestList,
        const std::string &requestKey);

    int NotifySCBToPreStartUIAbility(const AbilityRequest &abilityRequest,
        sptr<SessionInfo> &sessionInfo);

    int32_t NotifySCBToRecoveryAfterInterception(const AbilityRequest &abilityRequest);

    /**
     * @brief handle time out event
     *
     * @param msgId the msg id in ability record
     * @param abilityRecordId the id of ability record
     * @param isHalf is half
     */
    void OnTimeOut(uint32_t msgId, int64_t abilityRecordId, bool isHalf = false);

    /**
     * @brief handle when ability died
     *
     * @param abilityRecord the died ability
     */
    void OnAbilityDied(UIAbilityRecordPtr abilityRecord);

    /**
     * resolve the call ipc of ability for scheduling oncall.
     *
     * @param abilityRequest target ability request.
     */
    int ResolveLocked(const AbilityRequest &abilityRequest, std::string &errMsg);

    /**
     * resolve the call ipc of ability for prelaunch.
     *
     * @param abilityRequest target ability request.
     * @return Returns ERR_OK on success, others on failure.
     */
    int PrelaunchAbilityLocked(const AbilityRequest &abilityRequest, const int32_t frameNum);

    /**
     * Call UIAbility by SCB.
     *
     * @param sessionInfo the session info of the ability to be called.
     * @param params start parameters.
     * @param isColdStart the session of the ability is or not cold start.
     */
    void CallUIAbilityBySCB(const sptr<SessionInfo> &sessionInfo, AbilityRuntime::StartParamsBySCB &params,
        bool &isColdStart);

    /**
     * OnAcceptWantResponse.
     *
     * @param want the want of the ability to start.
     * @param abilityRequest the flag of the ability to start.
     * @return Returns ERR_OK on success, others on failure.
     */
    void OnAcceptWantResponse(const AAFwk::Want &want, const std::string &flag, int32_t requestId);

    /**
     * OnStartSpecifiedProcessResponse.
     *
     * @param requestId target ability requestId.
     */
    void OnStartSpecifiedProcessResponse(const std::string &flag, int32_t requestId,
        const std::string &callerProcessName = "", int32_t recordId = 0);

    /**
     * OnStartSpecifiedAbilityTimeoutResponse.
     *
     * @param requestId the requestId of the ability to start.
     */
    void OnStartSpecifiedAbilityTimeoutResponse(int32_t requestId);

    /**
     * OnStartSpecifiedProcessTimeoutResponse.
     *
     * @param requestId the requestId of the ability to start.
     */
    void OnStartSpecifiedProcessTimeoutResponse(int32_t requestId);

    /**
     * OnStartSpecifiedFailed.
     *
     * @param requestId request id of the failed request.
     * @return Returns ERR_OK on success, others on failure.
     */
    void OnStartSpecifiedFailed(int32_t requestId);

    /**
     * CallRequestDone, after invoke callRequest, ability will call this interface to return callee.
     *
     * @param abilityRecord ability's record.
     * @param callStub ability's callee.
     */
    void CallRequestDone(const UIAbilityRecordPtr &abilityRecord, const sptr<IRemoteObject> &callStub);

    int ReleaseCallLocked(const sptr<IAbilityConnection> &connect, const AppExecFwk::ElementName &element);

    /**
     * @brief handle when call connection died
     *
     * @param callRecord the died call connection
     */
    void OnCallConnectDied(const std::shared_ptr<CallRecord> &callRecord);

    /**
     * Get sessionId by ability token.
     *
     * @param token the ability token.
     * @return Returns sessionId on success, zero on failure.
     */
    int32_t GetSessionIdByAbilityToken(const sptr<IRemoteObject> &token);

    void GetActiveAbilityList(int32_t uid, std::vector<std::string> &abilityList, int32_t pid = NO_PID);

    bool PrepareTerminateAbility(const std::shared_ptr<AbilityRecord> &abilityRecord, bool isSCBCall);
    void SetSessionHandler(const sptr<ISessionHandler> &handler);

    /**
     * Get abilityRecord by session id.
     *
     * @param sessionId the session id.
     * @return Returns abilityRecord on success, nullptr on failure.
     */
    UIAbilityRecordPtr GetAbilityRecordsById(int32_t sessionId) const;

    /**
     * Get check ability number.
     *
     * @param bundleName record ability info bundle name.
     * @param abilityName record ability info ability name.
     * @param moduleName recode ability info module name.
     * @return Return find ability number.
     */
    int32_t CheckAbilityNumber(
        const std::string &bundleName, const std::string &abilityName, const std::string &moduleName) const;

    /**
     * If ability number more then one, send event info.
     *
     * @param userId record ability info user id.
     * @param bundleName record ability info bundle name.
     * @param abilityName record ability info ability name.
     * @param moduleName recode ability info module name.
     */
    void MoreAbilityNumbersSendEventInfo(
        int32_t userId, const std::string &bundleName, const std::string &abilityName, const std::string &moduleName);

    void OnAppStateChanged(const AppInfo &info);

    void UninstallApp(const std::string &bundleName, int32_t uid);

    void GetAbilityRunningInfos(std::vector<AbilityRunningInfo> &info, bool isPerm) const;

    /**
     * @brief dump all abilities
     *
     * @param info dump result.
     */
    void Dump(std::vector<std::string>& info);

    /**
     * @brief dump mission list
     *
     * @param info dump result.
     */
    void DumpMissionList(std::vector<std::string> &info, bool isClient, const std::string &args = "");

    /**
     * @brief dump mission list by id with params
     *
     * @param info dump result.
     * @param params dump params.
     */
    void DumpMissionListByRecordId(std::vector<std::string>& info, bool isClient, int32_t abilityRecordId,
        const std::vector<std::string>& params);

    int MoveMissionToFront(int32_t sessionId, std::shared_ptr<StartOptions> startOptions = nullptr);

    bool IsAbilityStarted(AbilityRequest &abilityRequest, std::shared_ptr<AbilityRecord> &targetRecord);

    /**
     * @brief Update session info.
     * @param sessionInfos The vector of session info.
     */
    int32_t UpdateSessionInfoBySCB(std::list<SessionInfo> &sessionInfos, std::vector<int32_t> &sessionIds);

    int32_t RegisterStatusBarDelegate(sptr<AbilityRuntime::IStatusBarDelegate> delegate);

    bool IsCallerInStatusBar(const std::string &instanceKey);

    bool IsInStatusBar(uint32_t accessTokenId, bool isMultiInstance);

    bool IsSupportStatusBar();

    int32_t TryPrepareTerminateByPids(const std::vector<int32_t>& pids, bool clear);

    int ChangeAbilityVisibility(sptr<IRemoteObject> token, bool isShow);

    int ChangeUIAbilityVisibilityBySCB(sptr<SessionInfo> sessionInfo, bool isShow);

    std::vector<UIAbilityRecordPtr> GetAbilityRecordsByName(const AppExecFwk::ElementName &element,
        int32_t appIndex);

    UIAbilityRecordPtr GetAbilityRecordByToken(const sptr<IRemoteObject> &token) const;

#ifdef SUPPORT_GRAPHICS
    void CompleteFirstFrameDrawing(const sptr<IRemoteObject> &token);
#endif
    void CompleteFirstFrameDrawing(int32_t sessionId) const;

    int32_t GetAbilityStateByPersistentId(int32_t persistentId, bool &state);

    void NotifySCBToHandleAtomicServiceException(sptr<SessionInfo> sessionInfo, int32_t errorCode,
        const std::string& errorReason);

    int32_t CleanUIAbility(const UIAbilityRecordPtr &abilityRecord);

    void EnableListForSCBRecovery();

    void SetKillForPermissionUpdateFlag(uint32_t accessTokenId);

    void PrepareTerminateAbilityDone(UIAbilityRecordPtr abilityRecord, bool isTerminate);

    void TryPrepareTerminateByPidsDone(const std::string &moduleName, int32_t prepareTermination, bool isExist);
    
    int32_t RevokeDelegator(sptr<IRemoteObject> token);

    bool IsBundleStarting(pid_t pid);

    void RecordPidKilling(pid_t pid, const std::string &reason, bool isKillPrecedeStart);

    int32_t NotifyStartupExceptionBySCB(int32_t requestId, const std::string &reason);

    ErrCode IsUIAbilityAlreadyExist(const Want &want, const std::string &specifiedFlag,
        int32_t appIndex, const std::string &instanceKey, AppExecFwk::LaunchMode launchMode);
    void HandleUIAbilityDiedByPid(pid_t pid);

private:
    void AddStartingPid(pid_t pid);
    void RemoveStartingPid(pid_t pid);
    void MarkStartingFlag(const AbilityRequest &abilityRequest);
    int32_t GetPersistentIdByAbilityRequest(const AbilityRequest &abilityRequest, bool &reuse) const;
    int32_t GetReusedSpecifiedPersistentId(const AbilityRequest &abilityRequest, bool &reuse) const;
    int32_t GetReusedStandardPersistentId(const AbilityRequest &abilityRequest, bool &reuse) const;
    int32_t GetReusedCollaboratorPersistentId(const AbilityRequest &abilityRequest, bool &reuse) const;
    std::string GenerateProcessNameForNewProcessMode(const AppExecFwk::AbilityInfo& abilityInfo);
    void PreCreateProcessName(AbilityRequest &abilityRequest);
    void UpdateProcessName(const AbilityRequest &abilityRequest, UIAbilityRecordPtr &abilityRecord);
    void UpdateAbilityRecordLaunchReason(const AbilityRequest &abilityRequest,
        UIAbilityRecordPtr &abilityRecord) const;
    void EraseAbilityRecord(const UIAbilityRecordPtr &abilityRecord);
    int DispatchState(const UIAbilityRecordPtr &abilityRecord, int state);
    int DispatchTerminate(const UIAbilityRecordPtr &abilityRecord);
    int DispatchBackground(const UIAbilityRecordPtr &abilityRecord);
    int DispatchForeground(const UIAbilityRecordPtr &abilityRecord, bool success,
        AbilityState state = AbilityState::INITIAL);
    void CompleteForegroundSuccess(const UIAbilityRecordPtr &abilityRecord);
    void HandleLoadTimeout(const UIAbilityRecordPtr &ability);
    void HandleForegroundFailed(const UIAbilityRecordPtr &ability,
        AbilityState state = AbilityState::INITIAL);
    void HandleForegroundTimeout(const UIAbilityRecordPtr &ability);
    void NotifySCBToHandleException(const UIAbilityRecordPtr &ability, int32_t errorCode,
        const std::string& errorReason, bool needClearCallerLink = true);
    void MoveToBackground(const UIAbilityRecordPtr &abilityRecord);
    void CompleteBackground(const UIAbilityRecordPtr &abilityRecord);
    void PrintTimeOutLog(UIAbilityRecordPtr ability, uint32_t msgId, bool isHalf = false);
    void DelayCompleteTerminate(const UIAbilityRecordPtr &abilityRecord);
    void CompleteTerminate(const UIAbilityRecordPtr &abilityRecord);
    bool IsContainsAbilityInner(const sptr<IRemoteObject> &token) const;
    bool CheckProperties(const UIAbilityRecordPtr &abilityRecord, const AbilityRequest &abilityRequest,
        AppExecFwk::LaunchMode launchMode) const;
    void NotifyAbilityToken(const sptr<IRemoteObject> &token, const AbilityRequest &abilityRequest) const;
    void PrepareCloseUIAbility(UIAbilityRecordPtr abilityRecord,
        int resultCode, const Want *resultWant, bool isClearSession);
    int CloseUIAbilityInner(UIAbilityRecordPtr abilityRecord);
    int32_t BackToCallerAbilityWithResultLocked(sptr<SessionInfo> currentSessionInfo,
        std::shared_ptr<AbilityRecord> callerAbilityRecord);

    // byCall
    int CallAbilityLocked(const AbilityRequest &abilityRequest, std::string &errMsg);
    sptr<SessionInfo> CreateSessionInfo(const AbilityRequest &abilityRequest, int32_t requestId) const;
    int NotifySCBPendingActivation(sptr<SessionInfo> &sessionInfo,
        const AbilityRequest &abilityRequest, std::string &errMsg);
    bool IsHookModule(const AbilityRequest &abilityRequest) const;
    int ResolveAbility(UIAbilityRecordPtr targetAbility, const AbilityRequest &abilityRequest) const;
    std::vector<UIAbilityRecordPtr> GetAbilityRecordsByNameInner(const AppExecFwk::ElementName &element,
        int32_t appIndex);
    void HandleForegroundCollaborate(const AbilityRequest &abilityRequest,
        UIAbilityRecordPtr abilityRecord);

    void NotifyStartSpecifiedAbility(AbilityRequest &request, const AAFwk::Want &want);
    void NotifyRestartSpecifiedAbility(const AbilityRequest &request, const sptr<IRemoteObject> &token);
    int MoveAbilityToFront(const SpecifiedRequest &specifiedRequest,
        const UIAbilityRecordPtr abilityRecord, UIAbilityRecordPtr callerAbility);
    int SendSessionInfoToSCB(UIAbilityRecordPtr &callerAbility, sptr<SessionInfo> &sessionInfo);
    int StartAbilityBySpecified(const SpecifiedRequest &specifiedRequest,
        UIAbilityRecordPtr callerAbility);

    void SetLastExitReason(UIAbilityRecordPtr abilityRecord) const;
    void SetReceiverInfo(const AbilityRequest &abilityRequest, UIAbilityRecordPtr abilityRecord) const;

    /**
     * @brief Execute PrepareTerminateApp when it is implemented
     *
     * @param pid process id
     * @param tokens the tokens of ability records
     * @return Returns the tokens that still need to execute PrepareTerminate.
     */
    std::vector<sptr<IRemoteObject>> PrepareTerminateAppAndGetRemaining(
        int32_t pid, const std::vector<sptr<IRemoteObject>> &tokens);

    bool GetContentAndTypeId(uint32_t msgId, std::string &msgContent, int &typeId) const;

    bool CheckSessionInfo(sptr<SessionInfo> sessionInfo) const;
    UIAbilityRecordPtr CreateAbilityRecord(AbilityRequest &abilityRequest,
        sptr<SessionInfo> sessionInfo) const;
    void AddCallerRecord(AbilityRequest &abilityRequest, sptr<SessionInfo> sessionInfo,
        UIAbilityRecordPtr uiAbilityRecord) const;
    void SendKeyEvent(const AbilityRequest &abilityRequest) const;
    bool CheckPid(const UIAbilityRecordPtr abilityRecord, const int32_t pid) const;
    std::shared_ptr<StatusBarDelegateManager> GetStatusBarDelegateManager();
    int32_t DoProcessAttachment(UIAbilityRecordPtr abilityRecord);
    void BatchCloseUIAbility(const std::unordered_set<UIAbilityRecordPtr>& abilitySet);
    void TerminateSession(std::shared_ptr<AbilityRecord> abilityRecord);
    int StartWithPersistentIdByDistributed(const AbilityRequest &abilityRequest, int32_t persistentId);
    void CheckCallerFromBackground(UIAbilityRecordPtr callerAbility, sptr<SessionInfo> &sessionInfo);
    int32_t DoCallerProcessAttachment(UIAbilityRecordPtr abilityRecord);
    int32_t DoCallerProcessDetachment(UIAbilityRecordPtr abilityRecord);
    UIAbilityRecordPtr GenerateAbilityRecord(AbilityRequest &abilityRequest,
        sptr<SessionInfo> sessionInfo, bool &isColdStart);
    UIAbilityRecordPtr FindRecordFromTmpMap(const AbilityRequest &abilityRequest);
    bool AddStartCallerTimestamp(int32_t callerUid);
    UIAbilityRecordPtr FindRecordFromSessionMap(const AbilityRequest &abilityRequest);
    inline int32_t GetRequestId()
    {
        if (requestId_ == 0 || requestId_ == INT32_MAX) {
            requestId_ = 1;
        }
        return requestId_++;
    }

    void AddSpecifiedRequest(std::shared_ptr<SpecifiedRequest> request);
    void StartSpecifiedRequest(SpecifiedRequest &specifiedRequest);
    std::shared_ptr<SpecifiedRequest> PopAndGetNextSpecified(int32_t requestId);
    bool IsSpecifiedModuleLoaded(const AbilityRequest &abilityRequest, bool isSpecifiedProcess, bool &isDebug);
    bool HandleStartSpecifiedCold(const AbilityRequest &abilityRequest, sptr<SessionInfo> sessionInfo,
        AbilityRuntime::StartParamsBySCB &params);
    bool HandleColdAcceptWantDone(const AAFwk::Want &want, const std::string &flag,
        const SpecifiedRequest &specifiedRequest);
    void HandleLegacyAcceptWantDone(SpecifiedRequest &specifiedRequest,
        const std::string &flag, const AAFwk::Want &want);
    std::shared_ptr<SpecifiedRequest> GetSpecifiedRequest(int32_t requestId);
    bool CheckPrepareTerminateTokens(const std::vector<sptr<IRemoteObject>> &tokens,
        uint32_t &tokenId, std::map<std::string, std::vector<sptr<IRemoteObject>>> &tokensPerModuleName);
    void HandleAbilityStageOnPrepareTerminationTimeout(int32_t pid, const std::string &moduleName,
        const std::vector<sptr<IRemoteObject>> &tokens);
    std::vector<sptr<IRemoteObject>> PrepareTerminateAppAndGetRemainingInner(int32_t pid, const std::string &moduleName,
        const std::vector<sptr<IRemoteObject>> &tokens);
    void CancelPrepareTerminate(UIAbilityRecordPtr abilityRecord);
    bool UpdateSpecifiedFlag(UIAbilityRecordPtr abilityRequest, const std::string &flag);
    bool ProcessColdStartBranch(AbilityRequest &abilityRequest, sptr<SessionInfo> sessionInfo,
        UIAbilityRecordPtr uiAbilityRecord, bool isColdStart);
    bool TryProcessHookModule(SpecifiedRequest &specifiedRequest, bool isHookModule);

    int32_t userId_ = -1;
    mutable ffrt::mutex sessionLock_;
    std::unordered_map<int32_t, UIAbilityRecordPtr> sessionAbilityMap_;
    std::unordered_map<int64_t, UIAbilityRecordPtr> tmpAbilityMap_;
    std::unordered_map<UIAbilityRecordPtr, std::list<AbilityRequest>> callRequestCache_;
    std::list<UIAbilityRecordPtr> terminateAbilityList_;
    sptr<IRemoteObject> rootSceneSession_;
    int32_t requestId_ = 0;
    sptr<ISessionHandler> handler_;
    ffrt::mutex statusBarDelegateManagerLock_;
    std::shared_ptr<StatusBarDelegateManager> statusBarDelegateManager_;
    bool isSCBRecovery_ = false;
    std::unordered_set<int32_t> coldStartInSCBRecovery_;

    ffrt::mutex startUIAbilityCallerTimestampsLock_;
    std::map<int32_t, std::vector<int64_t>> startUIAbilityCallerTimestamps_;

    std::map<std::string, std::list<std::shared_ptr<SpecifiedRequest>>> specifiedRequestList_;

    struct PrepareTerminateByPidRecord {
        pid_t pid_;
        std::string moduleName_;
        std::atomic_bool isTryPrepareTerminateByPidsDone_;
        int32_t prepareTermination_;
        bool isExist_;

        PrepareTerminateByPidRecord(pid_t pid, const std::string &moduleName, bool done,
            int32_t prepareTermination, bool isExist) : pid_(pid), moduleName_(moduleName),
            isTryPrepareTerminateByPidsDone_(done), prepareTermination_(prepareTermination),
            isExist_(isExist) {}
    };
    std::mutex isTryPrepareTerminateByPidsDoneMutex_;
    std::condition_variable isTryPrepareTerminateByPidsCv_;
    std::vector<std::shared_ptr<PrepareTerminateByPidRecord>> prepareTerminateByPidRecords_;
    std::unordered_map<int32_t, UIAbilityRecordPtr> hookSpecifiedMap_;

    std::mutex startingPidsMutex_;
    std::vector<pid_t> startingPids_;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_UI_ABILITY_LIFECYCLE_MANAGER_H